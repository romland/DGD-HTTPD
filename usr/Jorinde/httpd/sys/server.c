/**
 * Description: Server for keeping track of 'applications' and cloning users.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include <kernel/user.h>
# include <status.h>
# include <type.h>
# include "../include/www.h"
# include "../include/server.h"
# include "../include/object.h"

# define SECURE()	if(!SYSTEM()) error("illegal call\n")

inherit props JORINDE_SHARED+"lib/properties";
inherit LIB_HTTP_SERVER;
inherit LIB_HTTP_STRING;
inherit LIB_XML_UTIL;

static mapping status, applications, default_apps;
static string *plugins;
static int timeout, keep_alive_time, started;
static object accessd, authend, binaryd, logd;
string get_server_string();

static void create(varargs int clone)
{
	int i, j;
	object *elts, elt, config;
	mixed *ports;
	
	started = FALSE;
	props::create();
	default_apps = ([ ]);
	binaryd = find_object(SYS_BINARYD);

	/* Each application will maintain its own copy of this mapping */
	status = ([
		1   : ({ "!.!", "Internal: Default Status",         nil }),
		100 : ({ "1.1", "Continue",                         nil }),
		101 : ({ "1.1", "Switching Protocols",              nil }),
		102 : ({ "W.D", "Processing",						nil }),
		200 : ({ "1.0", "OK",                               nil }),
		201 : ({ "1.0", "Created",                          nil }),
		202 : ({ "1.0", "Accepted",                         nil }),
		203 : ({ "1.?", "Non-Authoritative Information",    nil }),
		204 : ({ "1.0", "No Content",                       nil }),
		205 : ({ "1.1", "Reset Content",                    nil }),
		206 : ({ "1.1", "Partial Content",                  nil }),
		207 : ({ "W.D", "Multi-Status",						nil }),
		300 : ({ "1.?", "Multiple Choices",                 nil }),
		301 : ({ "1.0", "Moved Permanently",                nil }),
		302 : ({ "1.0", "Found. Moved Temporarily",         nil }),
		303 : ({ "1.1", "See Other",                        nil }),
		304 : ({ "1.0", "Not Modified",                     nil }),
		305 : ({ "1.1", "Use Proxy",                        nil }),
		306 : ({ "?.?", "(Unused)",                         nil }),
		307 : ({ "1.1", "Temporary Redirect",               nil }),
		400 : ({ "1.0", "Bad Request",                      nil }),
		401 : ({ "1.0", "Unauthorized",                     nil }),
		402 : ({ "1.1", "Payment Required",                 nil }),
		403 : ({ "1.0", "Forbidden",                        nil }),
		404 : ({ "1.0", "Not Found",                        nil }),
		405 : ({ "1.1", "Method not allowed",               nil }),
		406 : ({ "1.1", "Not Acceptable",                   nil }),
		407 : ({ "1.1", "Proxy Authentication Required",    nil }),
		408 : ({ "1.1", "Request Timeout",                  nil }),
		409 : ({ "1.1", "Conflict",                         nil }),
		410 : ({ "1.1", "Gone",                             nil }),
		411 : ({ "1.1", "Length Required",                  nil }),
		412 : ({ "1.1", "Precondition Failed",              nil }),
		413 : ({ "1.1", "Request Entity Too Large",         nil }),
		414 : ({ "1.1", "Request-URI Too Long",             nil }),
		415 : ({ "1.1", "Unsupported Media Type",           nil }),
		416 : ({ "1.1", "Requested Range Not Satisfiable",  nil }),
		417 : ({ "1.1", "Expectation Failed",               nil }),
		422 : ({ "W.D", "Unprocessable Entity",				nil }),
		423 : ({ "W.D", "Locked",							nil }),
		424 : ({ "W.D", "Failed Dependency",				nil }),
		500 : ({ "1.0", "Internal Server Error",            nil }),
		501 : ({ "1.0", "Not Implemented",                  nil }),
		502 : ({ "1.0", "Bad Gateway",                      nil }),
		503 : ({ "1.0", "Service Unavailable",              nil }),
		504 : ({ "1.1", "Gateway Timeout",                  nil }),
		505 : ({ "1.1", "HTTP Version Not Supported",       nil }),
		507 : ({ "W.D", "Insufficient Storage",				nil }),
	]);

	ports = status()[ST_BINARYPORTS];
	if(ports == nil || !sizeof(ports)) {
		SYSLOG("No binary ports defined, will not start Jorinde");
		return;
	}

	/* Stringify elements of array (for comparing) */
	for(i = 0; i < sizeof(ports); i++) {
		ports[i] += "";
	}

	/* Load configuration */
	config = (JXMLROOTD)->new();
	config->loadXML(JORINDE_HTTPD_CONFIG_DIR + "server.xml");

	if(!config) {
		SYSLOG("Jorinde httpd server could not find configuration.\n");
		return;
	}

	/* Set misc properties */
	keep_alive_time = (int)config->xpath("server/timeout")[0]->getValue();
	timeout = (int)config->xpath("server/keep-alive")[0]->getValue();

	/* Get all applications from configuration */
	elts = config->xpath( "server/applications/application" );
	applications = ([ ]);
	for(j = 0; j < sizeof(elts); j++) {
		object app, appconfig;
		string hostname, tmp;

		elt = elts[j];

		/* Get index of port this application wants */
		for(i = 0; i < sizeof(ports); i++) {
			if(ports[i] == elt->getAttribute("port")) {
				break;
			}
		}

		if(i >= sizeof(ports)) {
			SYSLOG("Jorinde httpd could not bind port " + 
					elt->getAttribute("port") + "\n");
			continue;
		}

		/* Get application configuration */
		appconfig = (JXMLROOTD)->new();
		appconfig->loadXML(
				JORINDE_HTTPD_CONFIG_DIR + elt->getAttribute("config-file")
			);

		if(!sizeof(appconfig->xpath("application"))) {
			SYSLOG("Invalid configuration: " + JORINDE_HTTPD_CONFIG_DIR + 
					elt->getAttribute("config-file") + ", bailing.");
			/*
			 * Note that this is -fatal- for the server,
			 * we return without starting server.
			 */
			return;
		}

		hostname = appconfig->xpath("application/hostname")[0]->getValue() +
				":" + ports[i];

		/* Make server (me) aware of this application */
		tmp = appconfig->xpath("application/program/path")[0]->getValue();
		COMPILE(tmp);
		app = clone_object(tmp);
		applications[hostname] = allocate(HTAPP_SIZE);
		applications[hostname][HTAPP_INDEX] = i;
		applications[hostname][HTAPP_OBJECT] = app;
		applications[hostname][HTAPP_PORT] = ports[i];

		/* Initialize application */
		app->initialize(appconfig);
		app->set_port(ports[i]);

		if(elt->getAttribute("default") == "yes") {
			default_apps[(int)ports[i]] = hostname;
		}
	}

	/* Get plugins */
	/* All applications will get commands from the server. To enable or
	 * disable what can be executed; modify 'allowed methods' node in
	 * in each application configuration.
	 */
	plugins  = ({ });
	elts = config->xpath( "server/plugins/method/plugin" );
	for(i = 0; i < sizeof(elts); i++) {
		string program;

		if(elts[i]->dummy()) continue;
		program = elts[i]->xpath("program")[0]->getCleanValue();
		COMPILE(program);
		plugins += ({ program });
	}

	if(sizeof(plugins) == 0) {
		SYSLOG("no method plugins in Jorinde httpd's server.xml, bailing");
		return;
	}

	if(binaryd == nil) {
		SYSLOG("no binary manager for Jorinde httpd found, bailing");
		return;
	}

	binaryd->init_httpd(timeout, applications);

	authend = find_object( HTTP_AUTHENTICATE );
	elts = config->xpath( "server/plugins/authentication/plugin" );
	if(authend != nil && !elts[0]->dummy()) {
		authend->xml_parameter( "authentication", 
							elts[0]->xpath( "program" )[0]->getCleanValue() );
	} else {
		SYSLOG("no authentication daemon for Jorinde httpd in server.xml, " + 
			   "bailing");
		return;
	}

	accessd = find_object( HTTP_AUTHORIZE );
	elts = config->xpath( "server/plugins/authorization/plugin" );
	if(accessd != nil && !elts[0]->dummy()) {
		accessd->xml_parameter( "authorization", 
							elts[0]->xpath( "program" )[0]->getCleanValue() );
	} else {
		SYSLOG("no authorization daemon for Jorinde httpd in server.xml, " + 
			   "bailing");
		return;
	}

	logd = find_object( HTTP_LOGD );
	elts = config->xpath( "server/plugins/loghandler/plugin" );
	if(accessd != nil && !elts[0]->dummy()) {
		logd->xml_parameter( "logging", 
							elts[0]->xpath( "program" )[0]->getCleanValue() );
	} else {
		SYSLOG("no logger daemon for Jorinde httpd in server.xml, bailing");
		return;
	}

	started = TRUE;
	LOG(get_server_string() + " started");
}


mapping get_status_codes()
{
	return status + ([ ]);
}


object get_application(string host, int port)
{
	if(host && applications[host + ":" + port]) {
		return applications[host + ":" + port][HTAPP_OBJECT];
	} else {
		return applications[default_apps[port]][HTAPP_OBJECT];
	}
}


int get_keep_alive_time()
{
	return keep_alive_time;
}


/**
 * Called by ~System/httpd/binarymgr.c
 */
int selected(int port, object user)
{
	if(previous_object() != binaryd) {
		error("illegal call");
	}

	/* Initially we always use the default application */
	user->set_application(
				applications[default_apps[port]][HTAPP_OBJECT]
			);
	user->set_port(port);
	return 1;
}


object *get_plugins()
{
	if(sscanf(object_name(previous_object()), HTTP_USER + "#%*d") != 1) {
		error("illegal call");
	}

	return binaryd->clone_tools(previous_object(), plugins);
}


string get_server_string()
{
	string dgdver;
	dgdver = status()[ST_VERSION];

	/* string-slicing below might break unexpectedly */
	return dgdver[0..2]		+ "/" +	dgdver[4..] + " "
		 + "Jorinde-httpd"	+ "/" +	HTTPD_VERSION;
}


object get_authorization_server()
{
	return accessd;
}

object get_authentication_server()
{
	return authend;
}

/*
 * The started flag indicates whether the server was successfully started
 * or not.
 */
int is_started()
{
	return started;
}

/* This is not used AFAIK */
object get_logging_server()
{
	return logd;
}
