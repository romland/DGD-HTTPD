/**
 * Description: Sessions functionality support for HTTP-applications.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
#include <kernel/kernel.h>
#include <type.h>
#include "../include/www.h"

inherit LIB_HTTP_STRING;

static int     length, track, last_clean;
static string  key, name;
static object  app;

/* ([ string session-id : ({ int expiry time, object session }) ]) */
static mapping sessions;

int    get_session_tracking()			{ return track; }
int    get_session_length()				{ return length; }
string get_session_key()				{ return key; }
string get_session_name()				{ return name; }

void   set_session_tracking(int arg)	{ track = arg; }
void   set_session_length(int arg)		{ length = arg; }
void   set_session_key(string arg)		{ key = arg; }
void   set_session_name(string arg)		{ name = arg; }

int    remove_session(string id)		{ sessions[id] = nil; }

/**
 * TODO: This lib should probably have another name. app_session or so? 
 *       I'm tired.
 */
void create(varargs int clone)
{
	sessions = ([ ]);
	set_session_name("JORINDESESSIONID");
	set_session_key("JOOR");
	app = this_object();
	last_clean = time();
}

void set_session_object(string id, object ses)
{
	/* TODO */
}

object get_session_object(string id)
{
	if(sessions[id] && typeof(sessions[id]) == T_ARRAY) {
		return sessions[id][1];
	}
	return nil;
}

private string new_id()
{
	return hex_encode( hash_md5("" + random(0x7fffffff) + time()) );
}

private void clean()
{
	string *ids;
	string id;
	int i;

	ids = map_indices(sessions);
	for(i = 0; i < sizeof(ids); i++) {
		id = ids[i];
		if(time() > sessions[id][0]) {
			sessions[id] = nil;
			SYSLOG("\tremoved session: " + id + "\n");
		}
	}
}

/*
 * TODO: The breadcrumb-stuff should be moved elsewhere and make
 * a hook in the application if they want the functionality.
 */
object get_session_cookie(object request)
{
	int expires;
	string id;
	object cookie, old, session;

	if(time() > (last_clean + 5*60)) {
		clean();
		last_clean = time();
	}

	old = request->get_cookie( name );
	cookie = new_object(HTTP_COOKIE);
	expires = time() + get_session_length();

	if( !old || !sessions[old->get_value(key)] ) {
		id = new_id();
		session = new_object(HTTP_SESSION);
	} else {
		id = old->get_value(key);
		session = sessions[id][1];
	}

    /* Create a session for application */
    session->set_id( id );
    session->set_expires( expires );
    session->set_application( this_object() );
	sessions[id] = ({ expires, session });

	/* Create a cookie for user-agent */
	cookie->set_name(name);
	cookie->set_value( key, id );
/*	cookie->set_domain(	app->get_hostname() );
	cookie->set_max_age( get_session_length() );
	cookie->set_expires( datetime(expires, app->get_timezone()) );
*/
	cookie->set_path("/" );

	/* Add breadcrumb */
	session->set_breadcrumb(
		request->get_uri()->get_relative_path() + 
		request->get_uri()->get_filename() 
	);

	return cookie;
}

