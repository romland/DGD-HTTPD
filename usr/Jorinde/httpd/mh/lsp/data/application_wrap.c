/**
 * Description: Wrapper class for application that's used by LSP's.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include "../../../include/www.h"
# include <type.h>
# include "../include/lsp.h"

inherit LIB_HTTP_STRING;
inherit LIB_LSP_PROPERTY_WRAP;
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static object request, response, application, server, session;

void set_related(object req, object res, object app, object ser, object ses) {
	request = req;
	response = res;
	application = app;
	server = ser;
	session = ses;

	::set_wrapped(app);
}

static void create(varargs int clone)
{
}

string	get_webroot()				{ return application->get_webroot(); }
string	get_timezone()				{ return application->get_timezone(); }
int		get_port()					{ return application->get_port(); }

string	get_hostname()
{
	return application->get_hostname();
}

string	get_status_string(int a)
{
	return application->http_status_string(a);
}

/*TODO: set/get properties */
