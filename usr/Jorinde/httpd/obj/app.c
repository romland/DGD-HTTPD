/**
 * Description: Application. This keeps track of each 'website' and related.
 * Version:     $Id: app.c 259 2004-07-13 20:32:07Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include <kernel/rsrc.h>
# include <type.h>
# include "../include/www.h"
# include "../include/xmlutil.h"

# define BE_QUIET

inherit props JORINDE_SHARED+"lib/properties";
inherit session LIB_HTTP_SESSION;
inherit libapp	LIB_HTTP_APP;
inherit xmlutil LIB_XML_UTIL;
inherit LIB_HTTP_STRING;

static mapping media_handlers;
static object logger, elevator, errorpaged;


static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	props::create();
	libapp::create();
	session::create();
}


int initialize(object conf)
{
	int i;
	string media_handler, *tmp;
	object *nodes, node, program;
	
	conf = conf->xpath("application")[0];

#ifndef BE_QUIET
	SYSLOG("Initializing: " + conf->XPATH("name") + "\n");
#endif

	/* TODO: Make configurable */
	elevator = find_object(WWW_ACCESS);
	
	/* Set up the application */
	session::set_session_tracking(	conf->XPATH("session/enabled")			);
	session::set_session_length(	conf->XPATH("session/length")			);
	libapp::set_hostname(			conf->XPATH("hostname")					);
	libapp::set_webroot(			conf->XPATH("web-root/path")			);
	libapp::set_allow_anonymous(	conf->XPATH("web-root/anonymous-access"));
	libapp::set_timezone(	" " +	conf->XPATH("local-time/zone")			);

	/* Set up default pages to look for in / cases */
	nodes = conf->xpath("default-pages/page");
	tmp = allocate(sizeof(nodes));
	for(i = 0; i < sizeof(nodes); i++) {
		tmp[i] = trim(nodes[i]->getAttribute("name"));
	}
	libapp::set_default_documents(tmp);

	/* Set up media-types */
	/*
	 * TODO: All media-handlers should be clones since they can have 
	 * different parameters (parameter key/value pair in config.xml).
	 */
	nodes = conf->xpath("media-types/media-type");
	media_handlers = ([ ]);
	for(i = 0; i < sizeof(nodes); i++) {
		string suffix;

		node = nodes[i];
		suffix = node->XPATH("file-suffix");
		
		program = get_program(node->xpath("handler")[0]);
		media_handlers[suffix] = allocate(MT_SIZE);
		media_handlers[suffix][MT_CONTENTTYPE] = node->XPATH("content-type");
		media_handlers[suffix][MT_HANDLER] = program;
		
		if(program != nil) {
			forward_xml_parameters( program, node->xpath("handler")[0] );
		}
	}

	/* Set up allowed methods */
	nodes = conf->xpath("support/methods/public/method");
	for(i = 0; i < sizeof(nodes); i++) {
		node = nodes[i];
		/* TODO: We want to get rid of these XML-dummies */
		if(node->dummy()) continue;
		libapp::set_allowed_method( node->getValue() );
	}

	/* Setup errorpage handler and pass on parameters */
	node = conf->xpath("errorpagehandler")[0];
	errorpaged = get_program( node );
	if(errorpaged != nil) {
		forward_xml_parameters( errorpaged, node );
		libapp::set_error_pages(errorpaged);
	} else {
		SYSLOG("AppConfig warning: No <errorpagehandler>\n");
	}

	/* Set up logger. TODO: logger should be clone! */
	node = conf->xpath("accesslogger")[0];
	logger = get_program( node );
	if(logger != nil) {
		forward_xml_parameters( logger, node );
	} else {
		SYSLOG("AppConfig warning: No <accesslogger>\n");
	}

	LOG("Application " + conf->XPATH("name") + " started");
	
	return TRUE;
}


object get_elevator()
{
	return elevator;
}


object get_mediahandler(string filesuffix)
{
	if(media_handlers[filesuffix] && media_handlers[filesuffix][MT_HANDLER])
		return media_handlers[filesuffix][MT_HANDLER];
	return nil;
}


string get_contenttype(string filesuffix)
{
	if(filesuffix && media_handlers[filesuffix]) {
		return media_handlers[filesuffix][MT_CONTENTTYPE];
	}
	/* TODO: Is text/html really a good default datatype? No! */
	return "text/html";
}


int handle_mediatype(object request, object response)
{
	mixed *mediastruct;
	string filesuffix;

	filesuffix = lower_case(request->get_uri()->get_filesuffix());

	if(media_handlers[filesuffix]) {
		object handler;
		mediastruct = media_handlers[filesuffix];

		if((handler = mediastruct[MT_HANDLER]) && handler &&
		   handler->get_supports(filesuffix)) {
			if(!handler->handle(request, response)) {
				SYSLOG("Medihandler for " + filesuffix + " bailed out.\n");
				return FALSE;
			}
		}
	} else {
		SYSLOG("Ignoring mediatype for: '" + 
					request->get_uri()->get_filename() + "'\n");
	}
	return TRUE;
}


/**
 * This is called prior to reading any data from disk; does
 * media-handler need to read content of file? TRUE/FALSE.
 */
int get_need_content(object request, object response)
{
	object mh;
	string filesuffix;
																				
	filesuffix = request->get_uri()->get_filesuffix();

	if(mh = get_mediahandler(filesuffix)) {
		return mh->get_need_content(request, response);
	}
	return TRUE;
}


mixed *get_mediatype(string filesuffix)
{
	mixed *arr;
	arr = media_handlers[filesuffix];
	return (arr ? arr + ({ }) : allocate(MT_SIZE));
}


void set_mediatype(string filesuffix, object handler, string contenttype)
{
	if(get_contenttype(filesuffix)) {
		error("Cannot add mediahandler, delete it first.\n");
	}

	media_handlers[filesuffix] = ({ handler, contenttype });
}


void delete_mediatype(string filesuffix)
{
	if(get_contenttype(filesuffix)) {
		media_handlers[filesuffix] = 0;
	}
}


void http_log(object request, object response)
{
	if(request == nil)	{ SYSLOG("no request-ob, can't log\n"); return; }
	if(response == nil)	{ SYSLOG("no response-ob, can't log\n"); return; }

	if(logger) {
		logger->log(request, response);
	}
}
