/**
 * Description: Implements some WebDAV functionality.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <limits.h>
# include <type.h>
# include "../include/dav.h"

inherit str		LIB_HTTP_STRING;
inherit util	LIB_XML_UTIL;
inherit "./xmlutil";

#ifdef USE_DAV_ALIASES
private mapping aliases;
#endif

nomask User get_owner();


static void create()
{
#ifdef USE_DAV_ALIASES
	aliases = ([ ]);
	aliases["DAV:"] = "d";
	aliases["JOR:"] = "j";
#endif
}


#ifdef USE_DAV_ALIASES
static void set_aliases(object node)
{
	string *keys;
	int i;
	keys = map_indices(aliases);
	for(; i < sizeof(keys); i++) {
		node->setAttribute("xmlns:" + aliases[keys[i]], keys[i]);
	}
}


static string get_alias(string alias)
{
	return aliases[alias];
}
#endif


static string get_relative_destination(object request)
{
	string	tmp, proto, host, path;
	int port;

	tmp = request->get_header("Destination");

	if(!tmp) {
		return nil;
	}

	if(sscanf(tmp, "%s://%s:%d%s", proto, host, port, path) == 4) {
		tmp = proto + "://" + host + path;
	}
	host = request->get_header("Host");
	if(index_of(0, tmp, "http://" + host) == 0) {
		tmp = tmp[strlen("http://" + host)..];
	} else {
		tmp = nil;
	}
	return tmp;
}


static string get_absolute_destination(object request)
{
	object	uri;
	string	dst, tmp;

	error("TODO: Need to see to it that the uri can hand us information " + 
		"that matches get_relative_destination()");
	tmp = get_relative_destination(request);
	if(!tmp) {
		return nil;
	}
	uri = request->get_destination_uri();
	if(!uri) {
		return nil;
	}

	dst = uri->get_absolute_path() + uri->get_filename();
	return dst;
}


static string ending_slash(string path, int iscol)
{
	int len;
	
	len = strlen(path);
	
	if(iscol && (!len || path[len-1] != '/')) {
		path += "/";
	} else if(!iscol && path[len-1] == '/') {
		path = path[0..len-2];
	}

	return path;
}


static string get_status_string(int val)
{
	return get_owner()->get_application()->http_status_string(val);
}


static object create_dav_property(string key, mixed value, string xmlns,
		mixed name_only)
{
	object prop, tmp;

	if(!xmlns) {
		xmlns = "DAV:";
	}

#ifdef USE_DAV_ALIASES
	tmp = new_node(aliases[xmlns] + ":" + key);
	SYSLOG("Creating newstyle: " + tmp->getName() + "\n");
#else
	tmp = new_node(key);
#endif

	if(!name_only) {
		switch(typeof(value)) {
		case T_OBJECT :
			tmp->insert(value);
			break;
		case T_NIL :
			/* Do we want to insert something else if it's nil? */
			break;
		default :
			tmp->insert(new_data(value));
			break;
		}
	}

#ifndef USE_DAV_ALIASES
	tmp->setAttribute("xmlns", xmlns);
#endif

	return tmp;
}


static mapping get_propstat_group(int status, mapping props)
{
	mapping	ret;
	mixed	*tmp;
	int		i;

	ret = ([ ]);
	tmp = map_indices(props);	
	for(i = 0; i < sizeof(tmp); i++) {
		if(status == props[tmp[i]][DP_STATUS]) {
			ret[tmp[i]] = props[tmp[i]];
		}
	}
	return ret;
}


static int *get_propstat_groups(mapping props)
{
	int		*ret;
	mixed   *tmp;
	int     i, status;

	ret = ({ });
	tmp = map_indices(props);
	for(i = 0; i < sizeof(tmp); i++) {
		status = status = props[tmp[i]][DP_STATUS];
		if(!(is_member(status, ret))) {
			ret += ({ status });
		}
	}
	return ret;
}


static void set_prop_status(int status, mapping props)
{
	mixed	*tmp;
	int		i;

	tmp = map_indices(props);
	for(i = 0; i < sizeof(tmp); i++) {
		props[tmp[i]][DP_STATUS] = status;
	}
}


/**
 * TODO: Deal with access denied errors set in mapping props.
 */
static object *create_propstat(mapping props)
{
	object	tmp, *ret, prop;
	string	*propnames;
	int		*statuses;
	mapping	tmpprops;
	int		i, j;

	statuses = get_propstat_groups(props);
	ret = allocate(sizeof(statuses));
	for(j = 0; j < sizeof(statuses); j++) {
#ifdef USE_DAV_ALIASES
		ret[j] = new_node(aliases["DAV:"] + ":propstat");
		tmp = new_node(aliases["DAV:"] + ":status");
#else
		ret[j] = new_node("propstat");
		tmp = new_node("status");
#endif

		tmp->insert( new_data(get_status_string(statuses[j])) );
		ret[j]->insert( tmp );

		tmpprops = get_propstat_group(statuses[j], props);
		propnames = map_indices(tmpprops);
#ifdef USE_DAV_ALIASES
		prop = new_node(aliases["DAV:"] + ":prop");
#else
		prop = new_node("prop");
#endif
		ret[j]->insert(prop);
		for(i = 0; i < map_sizeof(tmpprops); i++) {
			prop->insert(create_dav_property(propnames[i], 
									tmpprops[propnames[i]][DP_VALUE],
									tmpprops[propnames[i]][DP_NAMESPACE],
									tmpprops[propnames[i]][DP_NAME_ONLY])
						);
		}
	}
	return ret;
}


static object create_response(string href, mapping props, int iscol)
{
	int		i;
	object	ret, tmp, *propstat, prop;

#ifdef USE_DAV_ALIASES
	ret = new_node(aliases["DAV:"] + ":response");
	tmp = new_node(aliases["DAV:"] + ":href");
#else
	ret = new_node("response");
	tmp = new_node("href");
#endif
	tmp->insert(new_data(href));
	ret->insert(tmp);

	propstat = create_propstat(props);
	for(i = 0; i < sizeof(propstat); i++) {
		ret->insert(propstat[i]);
	}

#if 0
#ifdef USE_DAV_ALIASES
	tmp = new_node(aliases["DAV:"] + ":status");
#else
	tmp = new_node("status");
#endif
	tmp->insert(new_data(get_status_string(status)));
	propstat->insert(tmp);
#endif

	return ret;
}


static int get_depth(mixed val)
{
	switch(typeof(val)) {
	case T_NIL :
		return INT_MAX;
	case T_INT :
		return (int)val;
	case T_STRING :
		if(is_numeric(val)) {
			return (int)val;
		} else if(val == "infinity") {
			return INT_MAX;
		}
	}
	return -1;
}


static int get_overwrite(object request)
{
	string tmp;
	tmp = request->get_header("Overwrite");
	if(tmp && tmp == "T") {
		return TRUE;
	}
	return FALSE;
}


/**
 * Name: get_request_xml()
 * Desc: Get request's XML root node.
 * Note: TODO: This will bug if there is more than 64kb in body. The 
 *		 root-cause of this bug is buried deep down in the XML-parser
 *		 and serialize.c (in this case).
 * Args: Request.
 * Rets: XML root-node.
 */
static object get_request_xml(object request)
{
	object root;
	root = new_xmldoc();
	root->parseXML(request->content_tostring());
	return root;
}

