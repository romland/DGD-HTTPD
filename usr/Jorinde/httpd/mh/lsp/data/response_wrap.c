/**
 * Description: Wrapper class for response that's used by LSP's.
 * Version:     $Id: response_wrap.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include "../../../include/www.h"
# include <type.h>
# include "../include/lsp.h"

inherit LIB_HTTP_STRING;
inherit LIB_DATE;
inherit LIB_LSP_PROPERTY_WRAP;
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static object request, response, application, server, session;
static int expires_in;

void set_related(object req, object res, object app, object ser, object ses) {
	request = req;
	response = res;
	application = app;
	server = ser;
	session = ses;

	::set_wrapped(res);
}

void create(varargs int clone)
{
	/* ... */
}

void end()
{
	error("TODO");
}

string  get_header(string a)			{ return response->get_header(a); }
int     get_status()					{ return response->get_status();  }
string  get_charset()					{ return response->get_charset(); }
int		get_expires_in()				{ return expires_in;              }

mapping get_headers()					{ return response->get_headers(); }
object  get_cookies()					{ return response->get_cookies(); }

void	set_header(string a, string b)	{ response->set_header(a, b); }
void	set_status(int stat)			{ response->set_status(stat); }
void	set_charset(string a)			{ response->set_charset(a);   }

void set_expires_in(int a)
{
    response->set_header("Expires", datetime(a, application->get_timezone()));
}

void set_expires_on(int a)
{
	error("TODO: set_expires_on()\n");
}

void write(mixed arg)
{
	switch(typeof(arg)) {
	case T_NIL :
		arg = "[nil]";
		break;
	case T_INT :
	case T_FLOAT :
		arg = "" + arg;
		break;
	case T_OBJECT :
		arg = "[OBJECT]" + object_name(arg);
		break;
	case T_ARRAY :
		error("arg is array\n");
	case T_MAPPING :
		error("arg is mapping\n");
	}
	response->add_content(arg);
}

object new_cookie()
{
	object cookie;
	cookie = new_object(HTTP_COOKIE);
	response->set_cookie(cookie);
	return cookie;
}

void redirect(string url)
{
	response->clear_content();
	response->set_status(301);
	response->set_header("Location", url);
}

void clear()
{
	response->clear_content();
}

