/**
 * Description:	Initialize httpd.
 * Version:		$Id$
 * License:		(c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include <status.h>
# include "include/www.h"
# include "include/object.h"

private object server;


static void create(varargs int clone)
{
	object runner;

	COMPILE(HTTP_LOGD);						/* Log daemon */
	COMPILE(DAV_HOME + "initd");			/* Initialize DAV objects */
	COMPILE(HTTP_AUTHENTICATE);				/* Authentication daemon */
	COMPILE(HTTP_AUTHORIZE);				/* Authorization daemon */
#if 0
	COMPILE(HTTP_STATUSD_400_500);			/* Error-page handler */
#endif
	COMPILE(HTTP_SERVER);					/* The web-server */
	COMPILE(HTTP_APP);						/* Web-application container */
	COMPILE(HTTP_MIME);						/* Mime container */
	COMPILE(HTTP_COOKIE);					/* Cookie container */
	COMPILE(HTTP_SESSION);					/* Session container */
	COMPILE(HTTP_USER);						/* http user object */
	COMPILE(HTTP_CONTENT);					/* Content container */
	COMPILE(HTTP_REQUEST);					/* Request object */
	COMPILE(HTTP_RESPONSE);					/* Response object */
	COMPILE(HTTP_URI);						/* URI object */

	/* Run the TestAll test-suite */
	runner = new_object(JORINDE_LUNIT + "data/runner");
	runner->initialize( HTTP_HOME + "tests/TestAll" );
	runner->silent_on_success(TRUE);
	runner->error_on_failure(TRUE);
	runner->run();

	/* Done initializing */
	server = find_object(HTTP_SERVER);		/* ... keep reference to this */
	if(server->is_started()) {
		SYSLOG(server->get_server_string() + " started.\n");
	} else {
		SYSLOG("WARNING: " + server->get_server_string() + 
			   " did not start, see preceeding messages.\n");
	}
}


object get_server()
{
	return server;
}

