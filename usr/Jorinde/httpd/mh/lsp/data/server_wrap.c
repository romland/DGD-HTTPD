/**
 * Description: Wrapper class for server that's used by LSP's.
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

static Request request;
static Response response;
static Application application;
static Httpd server;
static Session session;

void set_related(Request req, Response res, Application app, Httpd ser, 
		Session ses) {
	request = req;
	response = res;
	application = app;
	server = ser;
	session = ses;

	::set_wrapped(ser);
}

static void create(varargs int clone)
{
}

int		get_keep_alive_time()	{ return server->get_keep_alive_time(); }
string	get_server_string()		{ return server->get_server_string(); }

/* TODO: set/get properties */
