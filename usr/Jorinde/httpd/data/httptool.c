/**
 * Description: Implements the methods we support in HTTP specification.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/access.h>
# include <kernel/kernel.h>
# include "../include/www.h"

# define OWNER() (previous_object() == get_owner())

inherit tool LIB_HTTP_TOOL;
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	if(!SYSTEM() && !KERNEL()) {
		error("illegal call");
	}

	tool::create();

	set_method("GET",			"cmd_get");
	set_method("POST",			"cmd_post");
	set_method("HEAD",			"cmd_head");
	set_method("OPTIONS",		"cmd_options");
	set_method("PUT",			"cmd_put");
	set_method("TRACE",			"cmd_trace");
	set_version("HTTP-methods",	"0.1.0");
}


/**
 * Name: cmd_head()
 * Desc:
 * Note: TODO: Headers; ETag
 * Args:
 * Rets:
 */
int cmd_head(object request, object response)
{
	if(!OWNER()) {
		error("illegal call\n");
	}

	if(has_access(request, response, READ_ACCESS) == FALSE) {
		send(response, TRUE, FALSE);
		return FALSE;
	}

	response->update_content_length();
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: http_tool->cmd_head() failed to send\n");
	}
	return TRUE;
}


/**
 * Name: cmd_options()
 * Desc:
 * Note: TODO: I don't think OPTIONS * HTTP/1.1 will work yet.
 * Args:
 * Rets:
 */
int cmd_options(object request, object response)
{
	if(!OWNER()) {
		error("illegal call\n");
	}

	/* The 0 means that we demand NO access to serve this request */
	if(has_access(request, response, 0) == FALSE) {
		send(response, TRUE, FALSE);
		return FALSE;
	}

	response->set_status(200);
	response->set_header("Public", 
		implode(get_owner()->get_application()->get_allowed_methods(), ", "));
#if 0
	/* TODO: */
	response->set_header("Allow",  implode(app->get_allowed_methods(), ", "));
	response->set_header("Allow",  "PROPFIND");
#endif

	/* TODO: This does not belong here, it belongs in the DAV module! */
	response->set_header("DAV", "1");
	response->update_content_length();
	
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: http_tool->cmd_options() failed to send\n");
	}
	return TRUE;
}


/**
 * Name: cmd_put()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int cmd_put(object request, object response)
{
	int offset, length;

	if(!OWNER()) {
		error("illegal call\n");
	}

	if(has_access(request, response, WRITE_ACCESS) == FALSE) {
		send(response, TRUE, FALSE);
		return FALSE;
	}

	/* TODO: ranges */
	if(request->get_header("Content-Range")) {
		error("Content-Range not supported yet.");
	}

	/* write file to disk */
	offset = -1;
	length = -1;

	SYSLOG("TODO: http_tool->write_resource!\n");
	::write_resource(request, response, offset, length);
 
	/* if status up until now is okay, set it to 201 */
	if(response->get_status() == 200) {
		response->set_status(201);
	} else {
		/* Keep status unchanged */
	}

	response->update_content_length();
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: http_tool->cmd_put() failed to send\n");
	}
	return TRUE;
}


/**
 * Name: cmd_delete()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int cmd_delete(object request, object response)
{
	if(!OWNER()) {
		error("illegal call\n");
	}

	if(has_access(request, response, WRITE_ACCESS) == FALSE) {
		send(response, TRUE, FALSE);
		return FALSE;
	}

	/* TODO: Rest of (this function) cmd_delete */
	response->update_content_length();
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: http_tool->cmd_delete() failed to send\n");
	}
	return TRUE;
}


/**
 * Name: cmd_trace()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int cmd_trace(object request, object response)
{
	if(!OWNER()) {
		send(response, TRUE, FALSE);
		error("illegal call\n");
	}
	
	/* TODO: Rest of (this function) cmd_trace */
	response->set_status(501);

	response->update_content_length();
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: http_tool->cmd_head() failed to send\n");
	}
	return TRUE;
}


/**
 * Name: cmd_get()
 * Desc:
 * Note: TODO: Conditional and partial GETs.
 * Args:
 * Rets:
 */
int cmd_get(object request, object response)
{
	object	app, uri;
	mixed	**list;
	string	str;
	
	if(!OWNER()) {
		error("illegal call\n");
	}
	
	app = get_owner()->get_application();
	uri = request->get_uri();

	/* if requested file ends with '/', use default page of dir */
	str = uri->get_filename();
	if(!str || str == "" || str == "/" || str[strlen(str)-1] == '/') {
		/* TODO: This is a hack. I guess what we should do is redirect
		 * the client
		 */
		uri->set_filename(
					app->default_document(uri->get_absolute_path())
				);
	}

	if(has_access(request, response, READ_ACCESS) == FALSE) {
		send(response, TRUE, FALSE);
		return FALSE;
	}

#if 1 /* TODO: This section should probably be reused in [some] other cmds */
	list = ::get_dir(uri->get_absolute_filename());
	if(sizeof(list[0]) == 0) {
		response->set_status(404);
		send(response, TRUE, TRUE);
		return FALSE;
	}

	/* redirect user-agent if requested _file_ is a directory */
	if(list[1][0] < 0) {
		SYSLOG("redirecting\n");
		response->set_status(302);
		response->set_header("Content-Length", 0);
		response->set_header("Location", 
				uri->get_relative_path() + uri->get_filename() + "/");
		send(response, TRUE, FALSE);
		return TRUE;
	}
	response->set_header("Content-Length", list[1][0]);
	response->set_header("Last-Modified", ctime(list[2][0]));
#endif

#ifdef USE_ETAGS /* TODO: DISABLED ON 04-AUG-2004 DUE TO BUGS, WILL FIX! */
	/*
	 * Verify set etag and other conditional gets only if plugged in 
	 * mediahandlers does not want to deal with this resource.
	 */
	if(app->get_mediahandler(uri->get_filesuffix()) == nil) {
		string etag;

		etag = get_etag(uri->get_relative_path() + uri->get_filename(),
					"" + list[1][0], ctime(list[2][0]));

		response->set_header("Etag", etag);

		/* Is their cache up to date? If yes, do not send content. */
		if(request->meets_condition(response->get_status(), etag, list[2][0], 
					list[1][0]) ) {
			response->set_status(304);
			response->set_header("Content-Length", 0);
			/* TODO: close connection header -- need call_out() in user? */
			SYSLOG("Telling client to use its cache...\n");
			send(response, TRUE, FALSE);
			return TRUE;
		}
	}
#endif /* USE_ETAGS */
	
	/*
	 * Check if the mediahandler wants us to read in the content for them
	 * before they deal with it; alternatively they read it themselves.
	 */
	if(app->get_need_content(request, response)) {
		int offset, length;

		/* read all if mediatype has a mediahandler or it is < CHUNK_SIZE */
		if(app->get_mediahandler(uri->get_filesuffix()) != nil ||
		  ((int)response->get_header("Content-Length")) <= HTTP_CHUNK_SIZE) {
			response->set_read_on_demand(FALSE);
			offset = -1;
			length = -1;
		} else {
			response->set_read_on_demand(TRUE);
			offset = 0;
			length = HTTP_CHUNK_SIZE;
		}

		if(!(read_resource(request, response, offset, length))) {
			LOG("WARNING: could not to read " + uri->get_filename() + "\n");
			response->set_status(500);
			response->update_content_length();
		}

		response->set_read_range(offset, length);
	}

	if(app->handle_mediatype(request, response) == FALSE) {
		response->set_status(500);
	}

	/*
	 * If any errors occured in the process of dealing with this mediatype,
	 * output them to user's webbrowser.
	 */
	if(this_user()->has_error()) {
		response->add_content( this_user()->get_error() );
		response->update_content_length();
	}

	if(response->get_content_length() == 0) {
		response->set_status(204);
	}

	if(!send(response, TRUE, FALSE)) {
		LOG("WARNING: failed to send response\n");
	}
	return TRUE;
}


/**
 * Name: cmd_post()
 * Desc:
 * Note: If the initial content of a request is greater than 65 kb we will
 *       bug. I'm not sure if this can happen.
 *       TODO: Need to do some sanity checking and possibly add more headers.
 * Args:
 * Rets:
 */
int cmd_post(object request, object response)
{
	if(!OWNER()) {
		error("illegal call\n");
	}

	if(has_access(request, response, READ_ACCESS) == FALSE) {
		send(response, TRUE, FALSE);
		return FALSE;
	}

	if(request == nil || response == nil) {
		SYSLOG("WARNING: Response or request is nil\n");
	}

	/* perform a normal GET with POSTed content in request */
	if(cmd_get(request, response) == FALSE) {
		SYSLOG("cmd_get failed after post-request\n");
	}
	return TRUE;
}

