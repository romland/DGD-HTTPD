/**
 * Description: HTTP-cookie implementation.
 * Version:     $Id: cookie.c 226 2004-06-20 21:58:43Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

#include <kernel/kernel.h>
#include "../include/www.h"

inherit LIB_HTTP_STRING;
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static mapping values;
static string  name, expires, path, domain;
static int     max_age, secure;

void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	values = ([ ]);
}

void set_expires(mixed arg)		{ expires = arg; }

void set_value(string k, string v)
{
	values[replace(k, ";", "")] = replace(v, ";", "");
}

void set_secure(int boolean)	{ secure = (boolean ? 1 : 0); }
void set_name(string arg) 		{ name = replace(arg, ";", ""); }
void set_path(string arg) 		{ path = arg; }
void set_domain(string arg) 	{ domain = arg; }
void set_max_age(mixed arg)		{ max_age = (int)arg; }

int     get_secure()			{ return secure; }
mapping get_values()			{ return values + ([ ]); }
string  get_name()				{ return name; }
string  get_expires()			{ return expires; }
string  get_value(string key)	{ return values[key]; }
string  get_path()				{ return path; }
string  get_domain()			{ return domain; }
int     get_max_age()			{ return max_age; }

int fromstring(string s)
{
	values = ([ ]);

	if(s && s != "") {
		string *pairs, *pair;
		int i, x;

		i = index_of(0, s, "=");
		if(i < 1) {
			return FALSE;
		}

		set_name( trim_left(s[..i-1]) );
		pairs = explode(s[i+1..], ":");
		for(i = 0; i < sizeof(pairs); i++) {
			pair = explode(pairs[i], "=");
			x = sizeof(pair);
			if(!x) continue;

			if(x > 2) {
				set_value(pair[0], implode(pair[sizeof(pair)-1..], "="));
			} else if(x == 2) {
				set_value(pair[0], pair[1]);
			} else if(x == 1) {
				set_value(pair[0], "");
			}
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Note: Commas are stripped from the key/value pair.
 */
string tostring()
{
	string x;
	string *indices;

	if(!get_name())
		return "";

	if(get_name()[1] == '$') {
		error("Names that begin with $ are reserved for other uses and must " 
			+ "not be used by applications [RFC 2109].\n");
	}

	x = "";
	if(map_sizeof(values)) {
		int i;
		string *indices;
		indices = map_indices(values);
		for(i = 0; i < sizeof(indices); i++) {
			if(x != "") x += ":";
			x += indices[i] + "=" + values[indices[i]];
		}
	}
	x = get_name() + "=" + x + "; ";

	if(get_secure())		x += "Secure; ";
	if(get_expires())		x += "Expires=" +	get_expires()		+ "; ";
	if(get_path())			x += "Path=" +		get_path()			+ "; ";
	if(get_domain())		x += "Domain=" +	get_domain()		+ "; ";
	if(get_max_age())		x += "Max-Age=" +	get_max_age()		+ "; ";
/*
	x += "Version=1";
*/
	return x;
}

