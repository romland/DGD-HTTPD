/**
 * Description: Wrapper class for request that's used by LSP's.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include "../../../include/www.h"
# include <limits.h>
# include "../include/lsp.h"

inherit LIB_LSP_PROPERTY_WRAP;
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static Request request;
static Response response;
static Application application;
static Httpd server;
static Session session;
static Uri req_uri;

void set_related(Request req, Response res, Application app, Httpd ser, 
		Session ses) {
	if(req == nil) error("request is nil in wrapper.\n");
    if(res == nil) error("response is nil in wrapper.\n");
    if(app == nil) error("application is nil in wrapper.\n");
    if(ser == nil) error("server is nil in wrapper.\n");
    if(ses == nil) error("session is nil in wrapper.\n");

    request = req;
    response = res;
    application = app;
    server = ser;
    session = ses;
	req_uri = request->get_uri();

	::set_wrapped(req);
}

void create(varargs int clone)
{
	/* ... */
}

void initialize(Request ob)
{
	request = ob;
}

string   user()				{ return request->get_authenticated();		}
string   item(string a)		{ return req_uri->get_query_item(a);		}
mapping  items()			{ return req_uri->get_query_items();		}

/*
 * TODO: questionable wrapper-names, return values, but I just want to
 * test it now. NOTE: THIS WILL CHANGE!
 */
Content	form(string a)		{ return request->get_post_item(a);			}
mapping form_items()		{ return request->get_formdata();			}

string form_string(string a)
{
	Content ob;

	if((ob = form(a))) {
		return ob->content_tostring();
	}

	return "";
}

string   query()			{ return request->get_query_string();		}
string   path()				{ return req_uri->get_relative_path();		}
string   file()				{ return req_uri->get_filename();			}
string   abs_path()			{ return req_uri->get_absolute_path();		}
string   method()			{ return request->get_command();			}
string   ip_name()			{ return request->get_client_ip_name();		}
string   ip_number()		{ return request->get_client_ip_number();	}
mixed    header(string a)	{ return request->get_header(a);			}
mapping  headers()			{ return request->get_headers();			}
Cookie   cookie(string a)	{ return request->get_cookie(a);			}
Cookie   cookies()			{ return request->get_cookies();			}

string   abs_filename()		{ return req_uri->get_absolute_filename();	}

string content()
{
	if(request->get_content_length() > MAX_STRING_SIZE) {
		error("Content too large, use relay_content() as an alternative.\n");
	}
	return request->content_tostring();
}


int relay_content(Content obj, string func)
{
	int i;
	string str;

	if(!function_object(func, obj)) {
		error(func + " in " + object_name(obj) + " doesn't exist.\n");
	}

	for(i = 0; i < request->get_content_size(); i++) {
		str = request->get_content_chunk(i);
		call_other( obj, func, str );
	}
	return TRUE;
}


/**
 * TODO: Obtain an iterator over content-array in request.
 */
Iterator iterator()
{
	return request->iterator(1);
}
