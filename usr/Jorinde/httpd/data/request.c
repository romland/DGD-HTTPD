/**
 * Description: Request.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include "../include/www.h"

inherit header  LIB_HTTP_HEADER;
inherit content LIB_HTTP_CONTENT;
inherit cookie  LIB_HTTP_COOKIE;
inherit uri		LIB_HTTP_URI;
inherit 		LIB_DATE;
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static object app, owner;
static string command, protocol, protocol_version;
static string host_name, host_ip;
static string authenticated;
static int    badrequest;

static int    port;
static mapping formdata;		/* posted form-items */

static string debug_ruri;
static object uri, dest_uri;


# define DEPRECATED	error("deprecated")
string	get_query_item(string name)					{ DEPRECATED; }
void	set_query_item(string name, string value)	{ DEPRECATED; }
mapping	get_query_items()							{ DEPRECATED; }
string	get_query_string()							{ DEPRECATED; }
int		parse_query_string(string str)				{ DEPRECATED; }
int		set_ruri(string ruri)						{ DEPRECATED; }
string	get_filename()								{ DEPRECATED; }
void	set_filename(string arg)					{ DEPRECATED; }
string	get_relative_path()							{ DEPRECATED; }
string	get_filesuffix()							{ DEPRECATED; }
string	get_absolute_path()							{ DEPRECATED; }
string	get_absolute_filename()						{ DEPRECATED; }
string	get_href()									{ DEPRECATED; }
void   set_keep_alive(int a)						{ DEPRECATED; }
int    get_keep_alive()								{ DEPRECATED; }


static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	header::create();
	cookie::create();
	content::create();
	app = previous_object(CREATE_PREV_OBJ)->get_application();
	protocol = "HTTP";
	badrequest = FALSE;
	owner = previous_object(CREATE_PREV_OBJ);
	formdata = ([ ]);
}


object get_application()
{
	return app;
}

object get_uri()
{
	return uri;
}

/* ob should be an uri-object */
private int is_below_webroot(object ob)
{
	string abs_root, abs_path;

	if(!ob) {
		return FALSE;
	}
	
	/* THIS is where you look for gaping SECURITY holes. */
	abs_root = app->get_webroot();
	abs_path = ob->get_absolute_path();
	if(abs_path != abs_root && (strlen(abs_path) < strlen(abs_root) ||
		abs_path[..strlen(abs_root)-1] != abs_root)) {
		SYSLOG("Illegal request.\n"
			+ "       www-root: " + abs_root + "\n"
			+ "       abs-path: " + abs_path + "\n"
			+ "       rel-path: " + ob->get_relative_path() + "\n");
		return FALSE;
	}
	return TRUE;
}


int set_uri(string str)
{
	if(previous_object() != owner) {
		error("illegal call");
	}

	uri = create_uri(str, app->get_webroot());
	if(uri == nil || is_below_webroot(uri) == FALSE) {
		uri = nil;
		return FALSE;
	}

	return TRUE;
}


object get_destination_uri()
{
	if(dest_uri == nil) {
		dest_uri = create_uri(get_header("Destination"), app->get_webroot());
	}

	if(is_below_webroot(uri) == FALSE) {
		return nil;
	}

	return dest_uri;
}


void set_authenticated(string user)
{
	if(previous_program() != HTTP_USER) {
		error("illegal call\n");
	}
	authenticated = user;
}


string get_authenticated()
{
	return authenticated;
}


int is_conditional()
{
	if((get_header("If-Modified-Since")) || 
			(get_header("If-Unmodified-Since")) ||
			(get_header("If-Match")) ||
			(get_header("If-None-Match")) ||
			(get_header("If-Range"))
			) {
		return TRUE;
	}
	return FALSE;
}

/*
 * Returns TRUE if we don't have to send the content; otherwise FALSE.
 *
 */
int meets_condition(int status, string etag, int modified, int length)
{
	int ret, cmp_date;
	string ifms, ifus, ifm, ifnm, ifr;

	if(!(is_conditional())) {
		return FALSE;
	}

	/* Assume we have to send content */
	ret  = FALSE;

	/* Get the conditional headers */
	ifms = get_header("If-Modified-Since");
	ifus = get_header("If-Unmodified-Since");
	ifm  = get_header("If-Match");
	ifnm = get_header("If-None-Match");
	ifr  = get_header("If-Range");

	/* If-Match: anything other than a 2xx or 412 status, then ignore */
	if(ifm == etag && (status == 412 || (status > 199 && status < 300))) {
		ret = TRUE;
	}
	
	/* If-None-Match: http://www.httpsniffer.com/http/1426.htm */
	if(ifnm == etag && (status == 412 || (status > 199 && status < 300))) {
		ret = TRUE;
	} else if(ifnm) {
		SYSLOG("TODO: If-None-Match - Not yet fully implmeneted\n");
		/* Not implemented yet, so send content */
		ret = FALSE;
	}

	/* If-Modified-Since */
	if(ifms) {
		cmp_date = datetime_to_int(ifms);
		if(modified > cmp_date) {
			/* It has been modified, send content (FALSE) */
			ret = FALSE;
		} else {
			ret = TRUE;
		}
	}

	/* If-Unmodified-Since */
	/* This just does the opposite of 'ifms'; is this correct behaviour? */
	if(ifus) {
		cmp_date = datetime_to_int(ifms);
		if(modified > cmp_date) {
			/* It's unmodified, don't send content (TRUE) */
			ret = TRUE;
		} else {
			ret = FALSE;
		}
	}

	/* If-Range */
	if(ifr) {
		SYSLOG("TODO: If-Range - Not yet implemented\n");
		/* Not implemented yet, so send content */
		ret = FALSE;
	}

	return ret;
}


void   set_badrequest(int a)			{ badrequest = a;			}
int    get_badrequest()					{ return badrequest;		}

void   set_port(int a)					{ port = a;					}
int    get_port()						{ return port;				}

string get_command()					{ return command;			}
string get_protocol()					{ return protocol;			}
string get_client_ip_name()				{ return host_name;			}
string get_client_ip_number()			{ return host_ip;			}

mapping get_formdata()					{ return formdata;			}
object get_post_item(string arg)		{ return formdata[arg];		}

void set_formdata(mapping m)			{ formdata = m;				}
void set_post_item(string a, object o)	{ formdata[a] = o;			}

void set_command(string arg)			{ command = arg;			}
void set_protocol(string arg)			{ protocol = arg;			}
void set_client_ip_name(string arg)		{ host_name = arg;			}
void set_client_ip_number(string arg)	{ host_ip = arg;			}

/* TODO: Treat version as major/minor integers (possibly a float)? */
string get_protocol_version()			{ return protocol_version; 	}
void set_protocol_version(string arg)	{ protocol_version = arg;	}

void debug_info()
{
	SYSLOG("------------ REQUEST INFORMATION ------------\n");
	SYSLOG("all req.: " + debug_ruri + "\n");
	SYSLOG("---\n");
	SYSLOG(" command: " + get_command() + "\n");
	SYSLOG("protocol: " + get_protocol() + "\n");
	SYSLOG(" version: " + get_protocol_version() + "\n");
	SYSLOG(" headers: " + headers_tostring() + "\n");
	SYSLOG(" cookies: " + cookies_tostring() + "\n");
	uri->debug_info();
	SYSLOG("---------------------------------------------\n");
}

