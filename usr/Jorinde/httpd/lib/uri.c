/**
 * Description: URI-helper.
 * Version:     $Id: uri.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include "../include/www.h"

inherit LIB_HTTP_STRING;

static void create()
{
}

static object create_uri(string str, string root)
{
	object ob;
	if(!str || !strlen(str)) {
		return nil;
	}
	ob = new_object(HTTP_URI);
	if(!(ob->set( str, root ))) {
		return nil;
	}
	return ob;
}

static string get_suffix_only(string filename)
{
	string *tmp;

	tmp = explode(filename, ".");
	if(sizeof(tmp) > 1) {
		return tmp[sizeof(tmp)-1];
	}
	return "";
}


static string get_dir_only(string path, string webroot)
{
	string *tree;

	tree = explode(path, "/");
	path = implode(tree[..sizeof(tree)-1], "/");
	path = normalize_path(path, webroot, "TODO");
	path = replace(path, "//", "/");

	return (path[strlen(path)-1] == '/') ? (path) : (path + "/");
}


static string get_file_only(string filename)
{
	string *a;
	a = explode(filename, "/");
	return a[sizeof(a)-1];
}

mixed *parse_request_uri(string str) { error("deprecated"); }

