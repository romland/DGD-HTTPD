/**
 * Description: Layer above actual access-daemon, deals with anonymous access.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include <kernel/access.h>
# include "../include/www.h"
# include "../include/access.h"

# define ACCTYPES ({ "none", "read", "write", "full" })

private object accessd;

static void create(varargs int clone)
{
/* Now configurable 
	accessd = find_object(SYS_ACCESSD);
*/
}


int xml_parameter(string key, mixed value)
{
	if(object_name(previous_object()) != HTTP_SERVER) {
		error("illegal call");
	}

	if(!(accessd = find_object(value))) {
		accessd = compile_object(value);
	}
	return TRUE;
}


/**
 * Right now it's very silly and simple:
 * HTTP_ANON_USER will have read-access to everything in wwwroot except
 * if the file path contains 'secure'. :-)
 *
 * Will ponder on how I want it to work in the end.
 */
nomask int authorize(string file, int type)
{
	User	user;
	string	name;
	int		granted;

	user = this_user();
	name = user->query_name();

	/* If user is not authenticated and we don't allow anonymous. Deny! */
	if(name == HTTP_ANON_USER && 
	   !(user->get_application()->get_allow_anonymous())) {
		granted = FALSE;
	} else {
		granted = accessd->access(name, file, type);
		/* type 0 means 'no access' */
		if(type == 0
		   || (name == HTTP_ANON_USER && sscanf(file, "%*ssecure%*s") != 2
		       && type == READ_ACCESS) ) {
			/* Override accessd's decision */
			granted = TRUE;
		}
	}
#if 1
	SYSLOG( name + " [" + ACCTYPES[type]  + " " + 
			(granted ? "granted" : "denied") + "]: " + file + "\n");
#endif
	return granted;
}


