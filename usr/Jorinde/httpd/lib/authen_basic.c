# include <kernel/kernel.h>
# include <kernel/access.h>
# include "../include/www.h"
# include "../include/access.h"

inherit	str	LIB_HTTP_STRING;

private object authend;

static string *create()
{
	return ({ "Basic" });
	authend = nil;
}


int xml_parameter(string key, mixed value)
{
	if(object_name(previous_object()) != HTTP_SERVER) {
		error("illegal call");
	}

	if(!(authend = find_object(value))) {
		authend = compile_object(value);
	}
	return TRUE;
}



private string *decode(string encoded)
{
	string *arr, tmp;

	tmp = base64_decode(encoded);
	arr = explode(tmp, ":");
	/* there can be ':' in password */
	arr = ({ arr[0], implode(arr[1..], ":") });
	return (sizeof(arr)==2) ? arr : nil;
}


static string authenticate(string authstr)
{
	string *tmp;

	if(previous_program() != HTTP_AUTHENTICATE) {
		error("illegal call");
	}

	if(!authstr || authstr == "") {
		return nil;
	}

	tmp = explode(authstr, " ");
	tmp = decode(tmp[1]);
	if(tmp != nil) {
		if(authend->match(tmp[0], tmp[1]) == TRUE) {
			return tmp[0];
		}
	}
	return nil;
}


static string initialize(object request)
{
	return "Basic realm=\"" + request->get_header("Host") + "\"";
}
