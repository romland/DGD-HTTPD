/**
 * Description: Functionality inherited by user-object, not sure how useful
 *				it would be to anything other than the httpd-user-ob, but
 *				it's a nice thought. :)
 * Version:  $Id: buff_user.c 190 2004-06-10 09:09:39Z romland $
 * License:  (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include <kernel/user.h>
# include "../include/www.h"
# include "../include/events.h"

inherit user	SYS_LIB_USER;
inherit events  LIB_EVENTS;

private object	pending;		/* pending incoming content */
static object	server;
private int     con_count, port;
private string  first_line;
private string  ip_name, ip_number;
private int     ttl_handle;

static  int     receive_pending(string str);

/* TODO: HACK: These two functions must be implemented in parent */
static	object	create_response(object request);
static  int     call_method(object request, object response);


static void create()
{
	pending = nil;
	events::init_event(EVT_MESSAGE_DONE);
	user::create();
}

static void set_server(object ob)	{ server = ob; } 
object get_server()					{ return server; }


void set_port(int a)
{
	if(object_name(previous_object()) != HTTP_SERVER) {
		error("illegal call");
	}
	port = a;
}
                                                                                
                                                                                
int get_port()
{
	return port;
}


/**
 * Name: ip_number()
 * Desc:
 * Note: TODO: Bad names. Rename to get_ip_*
 * Args:
 * Rets:
 */
string ip_number() { return ip_number; }
string ip_name() { return ip_name; }

static void clear_ttl()
{
	if(ttl_handle) {
#ifdef __RM_CO_TAKES_FN__
		remove_call_out("disconnect");
#else
		remove_call_out(ttl_handle);
#endif
		ttl_handle = 0;
	}
}

static void set_ttl(int seconds)
{
	clear_ttl();
#ifdef __CO_IS_VOID__
	call_out("disconnect", seconds);
	ttl_handle = 1;
#else
	ttl_handle = call_out("disconnect", seconds);
#endif
}



/**
 * Name: get_connections()
 * Desc:
 * Note: n/a
 * Args:
 * Rets:
 */
object *get_connections()
{
	return user::query_connections() + ({ });
}

/**
 * Name: login()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int login(string str, varargs mixed pass_on)
{
	/* __IGOR__ compat problem, need to check if we have a previos ob */
	if(previous_program() != LIB_CONN && previous_object()) {
		return MODE_DISCONNECT;
	}
# if 1
	if(pass_on) {
		SYSLOG("passing " + str + " on...\n");
		user::login( str );
		return MODE_NOCHANGE;
	}
#endif

	if(con_count == 0) {
		user::login( nil );
	}
	con_count++;
	first_line = str;

	connection(previous_object());

	ip_number = query_ip_number(query_conn());
	ip_name =   query_ip_name(query_conn());
	return MODE_RAW;
}

/**
 * Name: logout()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
void logout(int quit)
{
	SYSLOG("buff_user->logout(" + quit + "): " + ip_number + "\n");
	if(previous_program() == LIB_CONN && --con_count == 0) {
#if 0
		if (query_conn()) {
			string ip;
			ip = "[" + ip_name + " (" + ip_number + ")]";
			if (quit) {
				SYSLOG("logout() nicely " + ip + ".\n");
			} else {
				SYSLOG("logout() linkdeath " + ip + ".\n");
			}
		}
#endif
		::logout();
		destruct_object(this_object());
	}
}

/**
 * Name: send_data()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
nomask static int send_data(string str)
{
	int ret;
	string err;

	if(!str || str == nil || str == "") {
		return TRUE;
	}
	return message(str);
}

/**
 * Name: message_done()
 * Desc: Kernel hook.
 * Note:
 * Args:
 * Rets:
 */
nomask int message_done()
{
	clear_ttl();
	events::invoke_event(EVT_MESSAGE_DONE);

	if(sizeof(get_subscribers(EVT_MESSAGE_DONE)) == 0) {
#if 0 /* TODO: Is this handled in inheritor, it should be! VERIFY! */
		/* TODO: If request is HTTP/1.0, disconnect immediately? */
		if(!keep_alive) {
			return MODE_DISCONNECT;
		}
#endif
		set_ttl(server->get_keep_alive_time());
	}
	return MODE_NOCHANGE;
}


static int pending_content()
{
	return pending ? TRUE : FALSE;
}

/* TODO: Bad name, should be consistent and rename most of the things
 * related to 'pending' and 'incoming' and possibly also 'trailing'.
 */
static object set_incoming()
{
	pending = new_object(HTTP_CONTENT);
	return pending;
}


static string get_first_line()
{
	return first_line;
}


static void set_first_line(string arg)
{
	first_line = arg;
}

/**
 * Name: receive_message()
 * Desc: Called by parent.
 * Note:
 * Args:
 * Rets:
 */
static int receive_message(string str)
{
	if(pending_content()) {
		receive_pending( str );
	} else {
		first_line += str;
	}

	return MODE_NOCHANGE;
}


/**
 * Name: receive_pending()
 * Desc:
 * Note: TODO: I'm not happy with how pendings are dealt with now. The
 *     data is moved around too much; I made a bad design decision here. :)
 * Args:
 * Rets:
 */
static int receive_pending(string str)
{
	int err;

	if(!pending) {
		LOG("WARNING: receive_pending() called: Not expecting content.");
		return FALSE;
	}

	if(pending->get_started() == FALSE) {
		if(pending->initialize(str) == FALSE) {
			err = 1;
		}
	} else if(pending->get_complete() == FALSE) {
		if(pending->is_multipart() && pending->add_part(str) == FALSE) {
			err = 2;
		} else if(pending->add_content(str) == FALSE) {
			err = 3;
		}
	}

	if(err) {
		LOG("ERROR: Could not initialize or add pending content (" +err+ ").");
		error(err + ": Could not initialize or add pending content.\n");
	}

	if(pending->get_complete() == TRUE) {
		object request, response;

		request = pending->get_request();
		request->clear_content();
		if(pending->is_multipart()) {
			request->set_formdata( pending->get_parts() );
		} else if(pending->is_form_urlencoded()) {
			request->set_formdata( pending->get_parts_urlencoded() );
		} else {
			request->add_content( pending->content_tostring() );
		}
		pending = nil;

		/* This is a hack: It appeared when I did some refactoring and 
		 * separated the connection stuff from user, we call up to our
		 * inheritor here. Nasty.
		 */
		response = create_response(request);
		if(call_method(request, response) == FALSE) {
			LOG("WARNING: call_method returned false, cannot recover?");
			error("WARNING: call_method returned false, cannot recover?\n");
		}
	}

	return TRUE;
}

