/**
 * Description: Handle logging to screen for Jorinde.
 * Version:     $Id: logd.c 226 2004-06-20 21:58:43Z romland $
 * License:     see Jorinde/doc/License
 */

# include "../include/www.h"
# include "../include/debug.h"
# include <type.h>

inherit LIB_HTTP_STRING;

private object syslogd;

static void create()
{
	/*
	 * Set this temporarily, until our server's set it to a correct value,
	 * we might want to log things before it's set.
	 */
	syslogd = find_object(SYSTEM_SYSLOGD);
}


int xml_parameter(string key, mixed value)
{
	if(object_name(previous_object()) != HTTP_SERVER) {
		error("illegal call");
	}

	if(!(syslogd = find_object(value))) {
		syslogd = compile_object(value);
	}
	return TRUE;
}


/* TODO: SECURITY! */
void syslog(mixed msg)
{
	switch(typeof(msg)) {
	case T_OBJECT :
	case T_ARRAY :
	case T_MAPPING :
		msg = make_string(msg) + "\n";
		break;
	}

	if(syslogd != nil) {
		/* if nil
		 * This is really a reason to worry since you are missing msgs,
		 * but better than an error I guess.
		 */
		syslogd->syslog(msg);
	}
}


/* Log to disk, will contain debugging and other 'important' information */
void log(mixed msg)
{
	if(!write_file(SERVER_LOG, ctime(time())[4 .. 18] + " " + msg + "\n")) {
		syslog("error: could not log: " + msg + "\n");
	}
}

