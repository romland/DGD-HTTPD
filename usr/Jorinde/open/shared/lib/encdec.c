/**
 * Description: Library to url-encode/decode strings.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
#include "../include/shared.h"
inherit LIB_HTTP_STRING;

static string url_decode(string in)
{
	int i, pos;
	string ret;

	ret = "";
	i = 0;

	while((pos = index_of(i, in, "%")) >= 0) {
		if(pos) {
			ret += replace(in[(i?i+1:0)..pos-1], "+", " ");
		}
		ret += hex_to_char(in[pos+1..pos+2]);
		i = pos + 2;
	}

	if(i < strlen(in)) {
		ret += replace(in[(i?i+1:i)..], "+", " ");
	}

	return ret;
}
