/**
 * Description: Implements basic functionality needed by http (etc.) methods.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/www.h"

inherit tool LIB_HTTP_BUFF_TOOL;
inherit LIB_DATE;

private mapping methods;
private string plugin_name, plugin_version;
private object accessd, authend;

static void create()
{
	tool::create();
	accessd = find_object(HTTP_AUTHORIZE);
	authend = find_object(HTTP_AUTHENTICATE);
	methods = ([ ]);
}


static void set_method(string name, string function)
{
	methods[name] = ({ object_name(this_object()), function });
}


/**
 * Name: get_plugin_methods()
 * Desc: httpd-hook, called by server to fetch methods this plugin supports.
 * Note:
 * Args:
 * Rets: ([ method : ({ object-to-call, function-to-call }), ... ])
 */
mapping get_plugin_methods()
{
	if(!map_sizeof(methods)) {
		error("no methods set");
	}
	return methods + ([ ]);
}


nomask void set_version(string name, string ver)
{
	plugin_name = name;
	plugin_version = ver;
}

/**
 * Name: get_plugin_version()
 * Desc: httpd-hook, called by server to fetch name/version of plugin.
 * Note: Server wants it in form of 'name/x.x.x'.
 * Args:
 * Rets:
 */
nomask string get_plugin_version()
{
	return plugin_name + "/" + plugin_version;
}


static int access_denied(Request request, Response response)
{
	/* TODO: Huh? Clear name please? */
	authend->initialize(request, response);
	return TRUE;
}


static nomask int has_access(Request request, Response response, int type)
{
	if(accessd->authorize(request->get_uri()->get_absolute_filename(), type) == FALSE) {
		response->set_status(401);
		access_denied(request, response);
		return FALSE;
	}
	return TRUE;
}

static nomask int has_access_file(string path, int type)
{
	if(accessd->authorize(path, type) == FALSE) {
		return FALSE;
	}
	return TRUE;
}

