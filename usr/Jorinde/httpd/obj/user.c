/**
 * Description: User-object for Jorinde (http/dav).
 * Version:	 $Id$
 * License:	 (c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <kernel/access.h>
# include <status.h>
# include <limits.h>
# include <trace.h>
# include "../include/www.h"
# include "../include/access.h"

#define VERBOSE_IO_DEBUG
/*#define LIMITED_IO_DEBUG*/


inherit user	LIB_HTTP_BUFF_USER;
inherit str		LIB_HTTP_STRING;
inherit 		LIB_DATE;
inherit cmds	LIB_HTTP_CMD_PLUGINS;

private	int		keep_alive;
private object  app, accessd, authend;
private	string	errors;

static	object	create_request(string str);


/**
 * Name: create()
 * Desc: Initialize this (cloned) user object.
 * Note: Kernel hook.
 * Args: Clone id
 * Rets: n/a
 */
static void create(int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif
	if(!sscanf(object_name(previous_object()), USR+"/System/%*s") 
			&& previous_object() != this_object()) {
		error("illegal call");
	}
	user::create();
	cmds::create();
	errors = nil;

	set_server( find_object(HTTP_SERVER) );
	authend = server->get_authentication_server();
	accessd = server->get_authorization_server();
}


/**
 * Name: set_application()
 * Desc: Set related application for this user.
 * Note: The application can change at any given time, objects should always
 *	   use get_application() to retrieve a user's application if they need
 *	   it.
 * Args: object application
 * Rets: n/a
 */
void set_application(object o)
{
	if(previous_program() != HTTP_USER && previous_object() != server) {
		error("illegal call");
	}
	app = o;
}


static int call_method(object request, object response)
{
	if(app->get_allowed_method( request->get_command() )) {
		return ::call_method(request, response);
	}
	return FALSE;
}


/**
 * Name: create_response()
 * Desc: Create a response object from the request.
 * Note: n/a
 * Args: request object
 * Rets: A response-object (or nil and no response is sent).
 */
static object create_response(object request)
{
	object	response;
	string	file, cmd;

	response = new_object(HTTP_RESPONSE);
	response->set_status(200);
	response->set_header("Date", datetime(time(), app->get_timezone()));
	response->set_header("Server", server->get_server_string());
	response->set_header("Keep-Alive", server->get_keep_alive_time());
	response->set_header("Cache-control", "private");
	/* TODO: Make expirytime configurable */
	response->set_header("Expires", 
							datetime(time(), app->get_timezone())
						);
	response->set_header("Content-Length", 0);
	response->set_header("Last-Modified", ctime(time()));
	response->set_request(request);

	if(app->get_session_tracking()) {
		response->set_cookie( app->get_session_cookie(request) );
	}

	if(request->get_badrequest()) {
		response->set_status(400);
		return response;
	}

	return response;
}


/**
 * Name: query_name()
 * Desc: Mask Kernel's query_name().
 * Note:
 * Args:
 * Rets:
 */
string query_name()
{
	if(::query_name()) {
		return ::query_name();
	}
	return HTTP_ANON_USER;
}


/**
 * Name: get_name()
 * Desc: Get name of this user.
 * Note: For consistency, we use get_* everywhere, kernel uses query_*
 * Args: n/a
 * Rets: Name of the user (or the anonymous user)
 */
string get_name()
{
	return query_name();
}


/**
 * Name: get_application()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
object get_application()
{
	return app;
}


/**
 * Name: set_error()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
void set_error(string s)
{
	if(errors == nil) {
		errors = s;
	} else {
		errors += s;
	}
}

string get_error()
{
	string ret;
	ret = errors;
	errors = nil;
	return ret;
}

int has_error()
{
	return (errors != nil);
}


/**
 * Name: send_headers()
 * Desc:
 * Note: 
 * Args:
 * Rets:
 */
nomask int send_headers(object response)
{
	string data;
	int status;

	/* Only user's tools should be able to call this */
	if(!is_tool(previous_object())) {
		error("illegal call");
	}

	status = response->get_status();
	if(response->get_content_length() == 0) {
		switch(status) {
		case 0..199 :
		/* 200-299 is no error */
		case 300..303 :
		/* 304 is no error (Not Modified) */
		case 305..999 :
			if(app->get_error_page(response) == FALSE) {
				LOG("Can't obtain status-page: " + status + "\n");
			}
		}
	}

	data  = response->status_tostring();
	data += response->headers_tostring();
	data += response->cookies_tostring();

#ifdef VERBOSE_IO_DEBUG
	SYSLOG("-------- RESPONSE HEADERS --------\n");
	SYSLOG("\n" + data);
	SYSLOG("----------------------------------\n");
#endif

#ifdef LIMITED_IO_DEBUG
	SYSLOG("res: " +
				"[" + ip_number() + "] " + 
				app->http_status_string(response->get_status()) + ", " +
				"len: " + response->get_content_length() + 
				"\n\n"
			);
#endif

	if(!send_data(data + CRLF)) {
		SYSLOG("Could not send headers. Destructing user object.");
		disconnect();
		return FALSE;
	}
	return TRUE;
}


nomask int send_content(object response, string data)
{
	/* Only user's tools should be able to call this */
	if(!is_tool(previous_object())) {
		error("illegal call");
	}
	
	if(errors) {
		LOG("HTTP 503: " + errors);
		response->set_status(503);			/* or 200? */
		response->add_content(errors);
		response->update_content_length();
		errors = nil;
	}
#if 0
	SYSLOG("DATA: ---|\n" + data + "\n----|\n");
#endif
	return send_data(data);
}


/**
 * Masked event-function
 */
nomask int can_subscribe(object obj, string name)
{
	return is_tool(obj);
}


/**
 * Name: receive_request()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
static int receive_request(string request_string)
{
	int ret;
	object request, response;
	string hostname;

#ifdef VERBOSE_IO_DEBUG
	SYSLOG("--------------- REQUEST ---------------\n");
	SYSLOG("From: " + ip_name() + " (" + ip_number() + ")\n" + request_string);
	SYSLOG("------------ END OF REQUEST -----------\n");
#endif

	ret = MODE_NOCHANGE;

	/* Get which application to use (as early as possible) */
	if(sscanf(request_string, "%*sHost: %s\r\n%*s", hostname) == 3) {
		if(index_of(0, hostname, ":") != -1) {
			/* *sigh* Ugly fix. */
			hostname = explode(hostname, ":")[0];
		}
		set_application(server->get_application(hostname, get_port()));
	} else {
		set_application(server->get_application("", get_port()));
	}

	/* create a request object */
	request = create_request(request_string);
	if(request == nil) {
		SYSLOG("request is nil (but stored in pending), incoming data\n");
		return MODE_NOCHANGE;
	}

#ifdef LIMITED_IO_DEBUG
	SYSLOG("req: " + 
				"[" + ip_number() + "] " + 
				app->get_hostname() + ":" + 
				app->get_port() + ", " + 
				request->get_command() + " " +
				request->get_uri()->get_relative_path() + 
				request->get_uri()->get_filename() + ", " +
				"len: " + request->get_content_length() +
				"\n"
			);
#endif
	
	/* create a response object */
	response = create_response(request);
	app->http_log(request, response);
	if(response == nil) {
		/* we got a content body with request, dealing with that */
		SYSLOG("user->receive_request(), response is nil\n");
	} else {
		if(call_method(request, response) == FALSE) {
			ret = MODE_DISCONNECT;
			SYSLOG("call_method failed; disconnecting\n");
		}
	}

	if(!keep_alive && response->contents_buffered() == FALSE) {
		ret = MODE_DISCONNECT;
	}
	return ret;
}


/*
 * TODO: This method should know NOTHING about what HTTP-methods exist,
 * it now does some magic if if the command is a POST. How can we get 
 * around this?
 */
static object create_request(string str)
{
	object request;
	string	cmd, ruri, ver, head, file, authstr, tmp;

	request = new_object(HTTP_REQUEST);

	/* note no \r in sscanf */
	if(!str || sscanf(str, "%s %s HTTP/%s\n%s", cmd, ruri, ver, head) != 4) {
		request->set_badrequest(TRUE);
		return request;
	} 

	request->set_headers(head);
	request->set_command(cmd);
	request->set_protocol_version(ver);
	request->set_client_ip_name(ip_name());
	request->set_client_ip_number(ip_number());
	request->set_port(get_port());

	if(request->set_uri(ruri) == FALSE) {
		request->set_badrequest(TRUE);
		return request;
	}

	if(ver == "1.1" && request->get_header("Host") == nil) {
		request->set_badrequest(TRUE);
		return request;
	} else if(request->get_header("Host") == nil) {
		/* Cheating, but we always want a Host header. */
		request->set_header("Host", "");
	} else if(index_of(0, request->get_header("Host"), ":") != -1) {
		/* *sigh* Ugly fix. 
		 * TODO: This will not work under INET6
		 */
		request->set_header("Host", 
				explode(request->get_header("Host"), ":")[0]);
	}

	/* TODO: Must deal with more connection-types, this is a hack */
	if((tmp = request->get_header("Connection")) && 
			lower_case(tmp) == "keep-alive") {
		keep_alive = TRUE;
	} else {
		keep_alive = FALSE;
	}

	/* Authenticate user (if any) authenticate() returns user to log in */
	if(request->get_header("Authorization") && get_name() == HTTP_ANON_USER &&
				(authstr = authend->authenticate(request)) && strlen(str)) {
SYSLOG("logging in '" + authstr + "'...\n");
		user::login( authstr, 1 );
SYSLOG("logged in name is: " + query_name() + "\n");
		request->set_header("Authorization", "[secret]");
		request->set_authenticated( get_name() ? get_name() : "" );
		clone_tools();
	} else if(commandcount() == 0) {
		clone_tools();
	}

	/* POST requests don't need to set content-length in headers */
	if((request->get_header("Content-Length") 
			&& request->get_header("Content-Length") != "0") 
			|| cmd == "POST") {
		int len;
		string content;
		object ob;
	
		if(request->get_header("Content-Length")) {
			len = (int)request->get_header("Content-Length");
		} else {
			len = 0;
		}

		SYSLOG("Claimed tot: " + len + " bytes (got: "+strlen(str)+")\n");

		if(sscanf(str, "%*s" + CRLF + CRLF + "%s", content) == 2) {
			SYSLOG("adding some content\n");
		} else if(cmd == "POST") {
			SYSLOG("expecting content\n");
		} else {
			/* Headers did not end with 2*CRLF, bad request */
			SYSLOG("malformed request\n");
			request->set_badrequest(TRUE);
			return request;
		}

		SYSLOG("expecting more content\n");
		ob = set_incoming();
		ob->set_request(request);

		if(content != nil && strlen(content)) {
			request->add_content(content);
			receive_pending( request->content_tostring() );
		} else {
			SYSLOG("no content with request\n");
		}

		return nil;
	}

	return request;
}


int receive_message(string str)
{
	string initial_req;

	/* TODO: SECURITY: checking whether previous_program() != 0 is Igor compat,
	 * is this okay elsewhere and/or on Igor? Really?
	 */
	if(previous_program() && previous_program() != LIB_CONN) {
		error("illegal call");
	}

	initial_req = get_first_line();
	initial_req = (initial_req ? initial_req : "");
	if(pending_content() || sscanf(initial_req + str, "%*s\r\n\r\n") != 1) {
		return ::receive_message(str);
	}

	set_first_line("");
	return receive_request((initial_req=="" ? "" : initial_req+"\r\n") + str);
}
