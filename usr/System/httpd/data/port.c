/**
 * Description: Incoming binary connections on specified port. The purpose
 *				of this objects existance is simply so that we can determine
 *				which port a connection arrived on prior to having a user
 *				object. Originally we had the binary-manager take care of
 *				all http-related ports and that works fine if all applications
 *				listen to the same port.
 * Version:     $Id: port.c 230 2004-06-21 09:38:41Z romland $
 * License:     see Jorinde/doc/License
 */

# include "../include/httpd.h"

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

private int port;
private object binaryd;

static create(varargs int clone)
{
	if(object_name(previous_object()) != USR_SYSTEM+"httpd/binaryd" 
			&& previous_object() != this_object()) {
		error("illegal call");
	}

	binaryd = find_object(USR_SYSTEM+"httpd/binaryd");
}

void set_port(int arg)
{
	if(previous_object() != binaryd) {
		error("illegal call");
	}
	port = arg;
}

int get_port()
{
	return port;
}

int query_timeout(object obj)
{
	return binaryd->query_timeout(obj);
}
                                                                                
string query_banner(object obj)
{
	return binaryd->query_banner(obj);
}

object select(string str)
{
	return binaryd->select(port, str);
}

