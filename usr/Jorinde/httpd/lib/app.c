/**
 * Description: Library of functions an application must implement.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include "../include/www.h"
#if 0
# include "../include/error.h"
#endif
# include <type.h>

inherit str		LIB_HTTP_STRING;

private string	webroot, hostname, timezone;
private object	server;
private string	*default_docs;
private mapping	status;
private string	*allowed_methods;
private string	port;
private int		anon_acc;

nomask static void set_server(object arg);
nomask object get_server();

static void create(varargs int clone)
{
	set_server( find_object(HTTP_SERVER) );
	allowed_methods = ({ });
	
	status = get_server()->get_status_codes();
	if(status == nil) {
		error("status is nil, does previous_object() have get_server()?\n");
	}
	default_docs = ({ });
}

nomask static void set_default_documents(string *a)
{
	default_docs = a;
}

nomask static string *get_default_documents()
{
	return default_docs + ({ });
}

nomask static void set_server(object arg)
{
	server = arg;
}

nomask object get_server()
{
	return server;
}

nomask static void set_webroot(string arg)
{
	webroot = arg;
}

nomask string get_webroot()
{
	return webroot;
}

nomask static void set_allow_anonymous(string arg)
{
	if(arg == "allow") {
		anon_acc = TRUE;
	} else {
		anon_acc = FALSE;
	}
}

nomask int get_allow_anonymous()
{
	return anon_acc;
}

nomask string get_absolute_path(string r)
{
	return webroot + r;
}

nomask static void set_hostname(string a)
{
	hostname = a;
}

nomask string get_hostname()
{
	return hostname;
}

nomask string get_timezone()
{
	return timezone;
}

nomask void set_timezone(string arg)
{
	timezone = arg;
}

nomask string get_port()
{
	return port;
}

nomask void set_port(string arg)
{
	port = arg;
}

/* TODO: Ability to remove methods? */
nomask void set_allowed_method(string c)
{
	if(sizeof(allowed_methods & ({ c })) == 0) {
		allowed_methods += ({ c });
	}
}

nomask int get_allowed_method(string c)
{
	return (sizeof(allowed_methods & ({ c })) != 0) ? TRUE : FALSE;
}

nomask string *get_allowed_methods()
{
	return allowed_methods + ({ });
}

nomask string http_status_string(int val)
{
	return HTTP_PROTOCOL + " " + val + " "
		+ (status[val] ? status[val][HTTP_ST_MESSAGE] : "Unknown to Jorinde");
}

/**
 * locate the default document with the highest priority (lowest index)
 * for a directory.
 *
 * Leave maskable.
 * TODO: RENAME!
 */
string default_document(string rel_path)
{
	/* TODO: All. See excerpt of function
	 *
	 * What is expected of this function is to check the directory for 
	 * candidates of default documents and decide which one should have
	 * highest priority -- then return that.
	 *
	 * If NO default candidate could be found, return NULL. Callers of this
	 * function must deal with this NULL and -not- redirect client. What
	 * should happen is just a 404 (not found).
	 *
	 * All this is TODO.
	 */
	return default_docs[0];
}

/**
 * TODO: Consider: Do we need to take any arguments / return something?
 */
nomask static void set_error_pages(object handler)
{
	int i;
#if 0
	object ob;
	ob = find_object(HTTP_STATUSD_400_500);
#endif

	/* Set error handler for all 400 / 500 codes */
	status = get_server()->get_status_codes();
	for(i = 0; i < 999; i++) {
		if(status[i] && i >= 400 && i < 600 && i != 401) {
			status[i][HTTP_ST_HOOK] = handler;
		}
	}
}


/**
 * Leave maskable.
 */
int get_error_page(object response)
{
	int code;
	response->set_header("Content-Type", "text/html");
	response->clear_content();

	code = response->get_status();
#if 0
	SYSLOG("Fetching error-page for " + code + " from " + 
			object_name(status[code][HTTP_ST_HOOK]) + "\n");
#endif
	if(code && typeof(status[code][HTTP_ST_HOOK]) == T_OBJECT) {
		response->add_content(
					status[code][HTTP_ST_HOOK]->error_page(response)
		);
	} else {
		/* Default error message. */
		response->add_content(
				get_server()->get_server_string() + "<br>" +
				"You encountered an error or a TODO " +
				"(and status page was missing).<br><br>" +
				"<b>" + http_status_string(code) + "</b><br>"
		);
	}
	response->update_content_length();
	return TRUE;
}

