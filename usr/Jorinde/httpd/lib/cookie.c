#include <kernel/kernel.h>
#include "../include/www.h"

static mapping cookies;

void create(varargs int clone)
{
	cookies = ([ ]);
}

void    set_cookie(object c)		{	cookies[c->get_name()] = c;		}
object  get_cookie(string n)		{	return cookies[n];				}
mapping get_cookies()				{	return cookies + ([ ]);			}

/**
 * Takes a request's cookie string and generate cookie-object(s).
 * Return TRUE for success.
 */
int cookies_fromstring(string s)
{
	string *arr;
	int i;

	if(s && s != "") {
		arr = explode(s, ";");
		for(i = 0; i < sizeof(arr); i++) {
			object cookie;
			cookie = new_object(HTTP_COOKIE);
			if( cookie->fromstring(arr[i]) ) {
				set_cookie(cookie);
			} else {
				SYSLOG("Error onverting cookie: " + arr[i] + " to object.\n");
			}
		}
	}
	return TRUE;
}

string cookies_tostring()
{
	if(map_sizeof(cookies)) {
		int i;
		string x;
		string *indices;
		x = "";
		indices = map_indices(cookies);
		for(i = 0; i < sizeof(indices); i++) {
			x += "Set-Cookie: " + cookies[indices[i]]->tostring() + "\n";
		}
		return x;
	}
	return "";
}

