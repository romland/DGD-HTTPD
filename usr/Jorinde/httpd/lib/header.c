#include <kernel/kernel.h>
#include "../include/www.h"
#include <limits.h>
#include <type.h>

/*
#define LOWER_CASE_HEADERS
*/

inherit LIB_HTTP_STRING;

static mapping headers;
static string charset;

void create(varargs int clone)
{
	headers = ([ ]);
}

/**
 * Whee, messy. :)
 */
void set_header(string key, mixed value, varargs int keepcurrent)
{
	value += "";

	if((keepcurrent == FALSE || !headers[key]) && typeof(value) != T_ARRAY) {
		headers[key] = value;
	} else {
		if(typeof(headers[key]) == T_ARRAY) {
			if(typeof(value) == T_ARRAY) {
				headers[key] += value;
			} else {
				headers[key] += ({ value });
			}
		} else {
			if(typeof(value) == T_ARRAY) {
				headers[key] = (headers[key] ?
					 ({ headers[key] }) + value : value);
			} else {
				headers[key] = (headers[key] ? 
					({ headers[key], value }) : ({ value }) );
			}
		}
	}
}

string set_charset(string arg)
{
	charset = arg;
}

string get_charset()
{
	return charset;
}

string  get_header(string key)
{
	return (key=="Content-Type" && charset!=nil) 
			? headers[key]+charset : headers[key];
}

mapping get_headers()
{
	return headers + ([ ]);
}

/**
 * @returns: false on malformed request-headers.
 */
int set_headers(string arg)
{
	int i;
	string *lines;
	string key, values;

	if(!arg || arg == "" || arg == CRLF || arg == "\n")
		return TRUE;

	lines = explode(arg, CRLF);

	for(i = 0; i < sizeof(lines); i++) {
		if(lines[i] != "" && sscanf(lines[i], "%s: %s", key, values) == 2) {
			if(key == "Cookie") {
				this_object()->cookies_fromstring( values );
			} else {
				set_header(key, values);
			}
		} else {
			return FALSE;
		}
	}
	return TRUE;
}

string headers_tostring()
{
    string x, h;

    x = "";
    if(map_sizeof(headers)) {
        int i;
        string *indices;
        indices = map_indices(headers);
        for(i = 0; i < sizeof(indices); i++) {
			h = indices[i];
			if(typeof(headers[h]) == T_ARRAY) {
				int j;
				for(j = 0; j < sizeof(headers[h]); j++) {
					x += h + ": " + headers[h][j] + CRLF;
				}
			} else {
	            x += h + ": " + headers[h] + CRLF;
			}
        }
    }
    return x;
}


