/**
 * Description: Response.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include <type.h>
# include "../include/www.h"

inherit header  LIB_HTTP_HEADER;
inherit content LIB_HTTP_CONTENT;
inherit cookie  LIB_HTTP_COOKIE;
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static int     status, rod_flag;
static object  app, request, server;

static int **ranges;

static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif
	
	header::create();
	content::create();
	cookie::create();
	app = previous_object(CREATE_PREV_OBJ)->get_application();
	server = previous_object(CREATE_PREV_OBJ)->get_server();
	if(server == nil) error("server is nil");

	status = 1;
	rod_flag = FALSE;
	ranges = ({ });
}


/**
 * The logic of when to read a data-range from disk and when not to.
 * Range Retrieval Requests: 
 *		http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.35.2
 */
void set_read_on_demand(int v)
{
	rod_flag = v;
}

/**
 * Name: get_read_on_demand()
 * Desc: get status on whether we should read on demand or not
 * Note: Will return FALSE if last entry in ranges is -1, -1 regardless of 
 *       what rod_flag says.
 * Args: -
 * Rets: TRUE / FALSE
 */
int get_read_on_demand()
{
	int rsize;

	rsize = sizeof(ranges);
	if(rod_flag && rsize > 0 && ranges[rsize-1][0] == -1 &&
			ranges[rsize-1][1] == -1) {
		return FALSE;
	}
	return rod_flag;
}


int *get_last_read_range()
{
	if(get_read_on_demand() == FALSE) {
		error("read_on_demand-flag is not set; don't call me");
	}

	return ranges[sizeof(ranges)-1];
}


/**
 * Name: set_read_range()
 * Desc: set a byte-range in a file as read
 * Note: -1, -1 as arguments marks all file as read.
 * Args: offset (start), length (from offset)
 * Rets: -
 */
void set_read_range(int offset, int length)
{
	if(offset == -1 && length == -1) {
		set_read_on_demand(FALSE);
	}
#if 0 
	/* TODO: Do we want the array to grow in _some_ scenarios? When you do
	 * get-ranges in the GET-request maybe? 
	 * In normal read-on-demand files we don't want this, it will make things
	 * barf on max-array-size.
	 */
	ranges += ({ ({ offset, length }) });
#else
	ranges = ({ ({ offset, length }) });
#endif
}

object get_application()			{	return app;		}

void   set_status(int arg)			{	status = arg;	}
int    get_status()					{	return status;	}

void   set_request(object arg)		{	request = arg;	}
object get_request()				{	return request;	}

string get_session_id()
{
	object c;
	c = get_cookie( app->get_session_name() );
	if(c) {
		return c->get_value( app->get_session_key() );
	}
	return nil;
}

/* Serialization */

string status_tostring()
{
	return (status ? 
			app->http_status_string(status) : app->http_status_string(503)
	) + "\r\n";
}

int get_status_group(mixed val)
{
	switch(val) {
	case 1			: return HTTP_STG_UNKNOWN;
	case 100..199	: return HTTP_STG_INFO;
	case 200..299	: return HTTP_STG_SUCCESS;
	case 300..399	: return HTTP_STG_REDIRECT;
	case 400..499	: return HTTP_STG_CLIENT_ERROR;
	case 500..599	: return HTTP_STG_SERVER_ERROR;
	default			: error("Unknown status-code.\n");
	}
}

void debug_info()
{
    SYSLOG("----------- RESPONSE INFORMATION ------------\n");
	SYSLOG("  status: "+status_tostring());			/* no lf 2 mimic reality */
    SYSLOG(" cookies: "+cookies_tostring() + "");   /* no lf 2 mimic reality */
    SYSLOG(" headers: "+headers_tostring() + "");	/* no lf 2 mimic reality */
    SYSLOG(" content: "+"<" + get_content_length() + " bytes>" + "\n");
    SYSLOG("---------------------------------------------\n");
}

