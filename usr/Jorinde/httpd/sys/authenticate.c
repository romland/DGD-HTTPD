/**
 * Description: Authentication daemon.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/**
 * Functions to be implemented by authentication modules:
 * - create()
 *			Should return ({ authentication-methods }) it supports
 *
 * - initialize()
 *			Should returns the initial header requesting authentication
 *			information from the client
 *
 * - authenticate()
 *			Should return authenticated user if it was successfully
 *			authenticated, otherwise nil.
 */

/*
 * Holy shit:
 *      NTLM auth: http://www.innovation.ch/java/ntlm.html
 */

# include <kernel/kernel.h>
# include "../include/www.h"
# include "../include/digest.h"
# include "../include/access.h"

# define BASIC
/*# define DIGEST*/

#ifdef BASIC
inherit basic	LIB_HTTP_AUTH_BASIC;
#endif
#ifdef DIGEST
inherit digest	LIB_HTTP_AUTH_DIGEST;
#endif

private string *methods;


static void create()
{
	methods = ({ });
#ifdef BASIC
	methods += basic::create();
#endif
#ifdef DIGEST
	methods += digest::create();
#endif
}


int xml_parameter(string key, mixed value)
{
	return basic::xml_parameter(key, value);
}


/* returns authenticated user on success, nil otherwise */
nomask string authenticate(object request)
{
	string header, method;

	if(previous_program() != HTTP_USER) {
		error("illegal call");
	}
	
	header = request->get_header("Authorization");
	method = explode(header, " ")[0];

	if(!sizeof(methods & ({ method }))) {
		SYSLOG("Unknown authentication method: " + method + "\n");
		return nil;
	}
	
	switch(method) {
#ifdef BASIC
	case "Basic" :
		return basic::authenticate(header);
#endif
#ifdef DIGEST
	case "Digest" :
		return digest::authenticate(request);
#endif
	default :
		return nil;
	}
}


nomask int initialize(object request, object response)
{
#ifdef BASIC
	SYSLOG("authenticate.c: Accepting Basic...\n");
	response->set_header("WWW-Authenticate", basic::initialize(request),FALSE);
#endif
#ifdef DIGEST
	SYSLOG("authenticate.c: Accepting Digest...\n");
	response->set_header("WWW-Authenticate", digest::initialize(request),TRUE);
#endif
	return TRUE;
}
