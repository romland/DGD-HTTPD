/**
 * Description: URI. Scheme // authority : port / path ? query # fragment
 *				Ex: http://host.com:81/blah.html?arg=value&arg2=value2#section
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/www.h"

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

inherit str LIB_HTTP_STRING;
inherit uri LIB_HTTP_URI;

private int		initialized, port;
private string	uri_string, scheme, authority;
private string	rel_path, filename, filesuffix, query_string, abs_path;
private string	fragment;
private mapping	query_items;

void debug_info();

static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif
	uri::create();
	query_items = ([ ]);
}

/* parsers */

private void set_query_item(string name, string value)
{
	query_items[name] = value;
}


/**
 * The query component is a string of information to be interpreted by
 * the resource. Within a query component, the characters ";", "/", "?",
 * ":", "@", "&", "=", "+", ",", and "$" are reserved. We, however, do
 * not filter characters at this level.
 */
private int set_query_string(string str)
{
	if(str && strlen(str)) {
		int i, x;
		string *pairs, *pair;

		pairs = explode(str, "&");
		for(i = 0; i < sizeof(pairs); i++) {
			pair = explode(pairs[i], "=");
			x = sizeof(pair);
			if(!x) continue;

			if(x > 2) {
				set_query_item(pair[0], implode(pair[sizeof(pair)-1..], "="));
			} else if(x == 2) {
				set_query_item(pair[0], pair[1]);
			} else if(x == 1) {
				set_query_item(pair[0], "");
			}
		}
	}
	return TRUE;
}


/* Public */
string	get_host()			{	error("deprecated. Use get_authority()");	}
string	get_protocol()		{	error("deprecated. Use get_scheme()");		}

int		get_port()					{	return port;				}
mapping	get_query_items()			{	return query_items + ([ ]);	}
string	get_query_item(string name)	{	return query_items[name];	}
string	get_query_string()			{	return query_string;		}
string	get_relative_path()			{	return rel_path;			}
string	get_scheme()				{	return scheme;				}
string	get_authority()				{	return authority;			}
string	get_filename()				{	return filename;			}
string	get_filesuffix()			{	return filesuffix;			}
string	get_absolute_path()			{	return abs_path;			}
string	get_absolute_filename()		{	return abs_path+filename;	}
string	get_fragment()				{	return fragment;			}


string to_href(varargs string hrefhost)
{
	string hrefscheme;

	if(!hrefhost) {
		hrefhost = (authority ? authority : "undefined");
	}

	hrefscheme = get_scheme();
	if(!hrefscheme || !strlen(hrefscheme)) {
		hrefscheme = "http";
	}

	/* TODO: Verify that it should be :// below when in WebDAV! */
	return	hrefscheme + "://" + hrefhost +
			((get_port() != 80) ? (":" + get_port()) : "") + "/" +
			get_relative_path() +
			get_filename();
}


/* TODO: This function should probably be in a public lib somewhere */
private string urldecode(string str)
{
	string pre, post;
	int val, bit0t3, bit5t8;

	if(!str || !strlen(str))
		return str;
#if 0
	SYSLOG("urldecode() org: '" + str + "'\n");
#endif
	str = replace(str, "+", " ");
	while( sscanf(str, "%s%%%c%c%s", pre, bit0t3, bit5t8, post) == 4 ||
		   sscanf(str, "%s%%%c%c", pre, bit0t3, bit5t8) == 3) {
		val = hex_to_int(chr(bit0t3)+chr(bit5t8));
		if(val > 255) continue;
		str = pre + chr(val) + (post ? post : "");
	}
#if 0
	SYSLOG("urldecode() new: '" + str + "'\n");
#endif
	return str;
}


/* Public setters */

int set(string uri, string webroot)
{
	string *tokens;
	string path;

	if(initialized) {
		error("already initialized");
	}
	
	uri = urldecode(uri);

	uri_string = uri;
	query_string = "";
	scheme = "";
	fragment = "";

	sscanf(uri, "%s://%s", scheme, uri);
	sscanf(uri, "%s#%s", uri, fragment);
	sscanf(uri, "%s?%s", uri, query_string);

	tokens = explode(uri, ":");
	/* fix for URI's like: /::%7B2227A280-3AEA-1069-A2DE-08002B30309D%7D */
	/*if(sizeof(tokens) > 1) {*/
	if(sizeof(tokens) > 1 && scheme) {
		/* we have a port in uri */
		authority = tokens[0];
		tokens = explode(tokens[1], "/");
		if(is_numeric(tokens[0])) {
			port = (int)tokens[0];
		} else {
			SYSLOG("Invalid port#");
			return FALSE;
		}
		path = implode(tokens[1..], "/");
	} else if(sizeof(tokens) == 1) {
		/* no port in uri */
		port = 80;
		tokens = explode(tokens[0], "/");

		/* if there is no scheme, ignore authority */
		if(!strlen(scheme)) {
			authority = "";
			path = implode(tokens[0..], "/");
		} else {
			authority = tokens[0];
			path = implode(tokens[1..], "/");
		}
	} else {
		SYSLOG("malformed uri (no uri or illegal colons)");
		return FALSE;
	}

	/* TODO: Hack! This if smells like missing logic in the lines above */
	if(!strlen(path)) {
		path = "/";
	}

#if 0
	/* Quick hack to support encoded space (%20), need more support
	 * for the rest of allowed escaped characters.
	 */
	path = replace(path, "%20", " ");
#endif
	
	path = replace(path, "*", "");
	if(path && path[strlen(path)-1] == '/') {
		filename = "";
		rel_path = path;
	} else {
		filename = get_file_only(path);
		rel_path = path[..(strlen(path)-strlen(filename)-1)];
	}

	if(strlen(rel_path) && rel_path[0] == '/') {
		rel_path = rel_path[1..];
	}

	filesuffix = get_suffix_only(filename);
	abs_path =   get_dir_only(rel_path, webroot);

	if(!set_query_string(query_string)) {
		return FALSE;
	}

	initialized = TRUE;
	return TRUE;
}


/* TODO: SECURITY! We want to get rid of this function, see HTTP_TOOL */
void set_filename(string arg)
{
	if(object_name(previous_object()) != HTTP_TOOL + "#-1") {
		error("illegal call");
	}
	filename = arg;
	filesuffix = get_suffix_only(filename);
}


void debug_info()
{
	SYSLOG("---------- URI -----------------------\n");
	SYSLOG("  uri_string: " + uri_string + "\n");
	SYSLOG(" initialized: " + initialized + "\n");
	SYSLOG("        port: " + port + "\n");
	SYSLOG("      scheme: " + scheme + "\n");
	SYSLOG("   authority: " + authority + "\n");
	SYSLOG("    rel_path: " + rel_path + "\n");
	SYSLOG("    filename: " + filename + "\n");
	SYSLOG("  filesuffix: " + filesuffix + "\n");
	SYSLOG("query_string: " + query_string + "\n");
	SYSLOG("    abs_path: " + abs_path + "\n");
	SYSLOG("    fragment: " + fragment + "\n");
	SYSLOG("abs_filename: " + get_absolute_filename() + "\n");
	SYSLOG("--------------------------------------\n");
}

