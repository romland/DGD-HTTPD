/**
 * Description: Let the httpd log to stdout (mainly for debugging).
 * Version:     $Id: syslogd.c 240 2004-06-24 08:05:28Z romland $
 * License:     see Jorinde/doc/License
 */

# include <kernel/kernel.h>
# include "include/httpd.h"

private object driver;

static void create()
{
	driver = find_object(DRIVER);
}
                                                                                
void syslog(mixed msg)
{
	string owner;

#ifdef __IGOR__
	owner = creator(previous_object());
#else
	owner = previous_object()->query_owner();
#endif
	if(owner != "Jorinde" && owner != "www" && owner != "System") {
		error("illegal call\n");
	}

	if(msg == nil) msg = "[nil]";
	/*
	 * something like this: driver->message(msg + "");
	 * This macro can be changed in jorinde.h
	 */
#ifdef STDOUT
	STDOUT(""+msg);
#else
	driver->message(msg + "");
#endif
}

