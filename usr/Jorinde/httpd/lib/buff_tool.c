/**
 * Description: Implements buffering and partial outgoing data (response)
 *              for tools (such as http and dav methods).
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../include/www.h"
# include "../include/access.h"
# include "../include/events.h"

inherit f LIB_FILE_TOOL;
inherit etag "./etag";

private object	buffer;


private int lid;
nomask int lwoid() { return lid; }


static void create()
{
	/* Keep a unique ID of this LWO for future reference when comparing */
	lid = random(32768) + random(32768);
	buffer = nil;
	f::create();
}


nomask static void message(string str)
{
	/* TODO: Do something with this message? */
	SYSLOG("TODO! buff_tool->message(): " + str + "\n");
}


/**
 * Name: write_resource()
 * Desc: Writes a resource in a request's body to disk.
 * Note: See note on read_resource()
 * Args: request, response, file offset, data length
 * Rets: TRUE on success, FALSE otherwise.
 */
nomask static int write_resource(object request, object response,
						int offset, int length)
{
	int i;
	string fn;

	SYSLOG("TODO: buff_tool->write_resource(): WILL NOT WRITE TO DISK!\n");

	if(offset != -1) {
		i = offset;
	} else {
		i = 0;
	}

	if(offset != -1 || length != -1) {
		error("ranges not supported\n");
	}

	fn = request->get_absolute_filename();
	rlimits( 0; -1) {
		/* All of this is TODO */
		if(offset == -1 && length == -1) {
			/* ... */
		} else {
			error("writing range (" + offset+"-"+(offset+length)+") " +
				"is not supported\n");
		}
	}
	return TRUE;
}


/**
 * Name: read_resource()
 * Desc: Reads a resource (file) into the response.
 * Note: Reads a file from disk using rights of whichever user owns this
 *       tool. In the case of access to WEBROOT* for current application 
 *       we call WWW_ACCESS to read the file.
 *       If the authorization and authentication works as it should, this
 *       SHOULD not have any nasty surprises -- remember that auth* is
 *       done before getting here.
 * Args:
 * Rets: TRUE on success, FALSE otherwise.
 */
nomask static int read_resource(object request, object response,
						int offset, int length)
{
	int i, anonymous;
	string fn;
	string webroot;

	webroot = get_owner()->get_application()->get_webroot();

	if(offset != -1) {
		i = offset;
	} else {
		i = 0;
	}

	fn = request->get_uri()->get_absolute_filename();
/*	rlimits( 0; -1)*/
	{
		if(response->get_read_on_demand() == FALSE) {
			int size;
			size = (int)response->get_header("Content-Length");
			do {
				response->add_content(f::read_file(fn, i, HTTP_CHUNK_SIZE));
				i += HTTP_CHUNK_SIZE;
			} while(i < size);
		} else {
			int bytes_done, tmp_read;
			bytes_done = 0;
			do {
				if((length - bytes_done) > HTTP_CHUNK_SIZE) {
					tmp_read = HTTP_CHUNK_SIZE;
				} else {
					tmp_read = length;
				}
				/* TODO: Don't read past end of file.  */
				response->add_content(f::read_file(fn, i+bytes_done, tmp_read));
				bytes_done += tmp_read;
			} while(bytes_done < length);
		}
	}
	return TRUE;
}


/**
 * Name: send()
 * Desc:
 * Note: TODO: When buffering a response, clear request-content?
 *       See the two "buffer = response" below in this function.
 *
 *       Reasoning: The response keeps a pointer to the request which
 *       means we will keep that one in memory until the entire response
 *       is sent. Waste of memory if we're not using the content.
 *
 *       This is only a bad scenario when we recieved an entity body
 *       with the request
 *
 *       Another solution is of course to NOT keep a pointer to the
 *       request; then we'd need to move all data in request that we
 *       depend on to the response-object (not too much, I think?)
 * Args:
 * Rets:
 */
nomask static int send(object response, int send_headers, int send_content)
{
	int i, rod, csize;
	object owner;

	rod = response->get_read_on_demand();
	owner = get_owner();
	
	if(send_headers) {
		owner->send_headers(response);
	}

	if(!response->get_content_length() && !rod) {
		/* No (pending) content? Ignore in-argument. */
		send_content = FALSE;
	}
	if(!rod) {
		response->update_content_length();
		csize = response->get_content_size();
		for(i = 0; i < csize; i++) {
			if(!owner->send_content(response, response->get_content_chunk(i))) {
				if((i + 1) < csize) {
					response->content_chunks_sent(i + 1);
					buffer = response;
					owner->subscribe(EVT_MESSAGE_DONE);
				}
				return TRUE;
			}
		}
	} else {
		/* read on demand enabled */
		int	*range, offset, length, total;

		total  = (int)response->get_header("Content-Length");
		range  = response->get_last_read_range();
		length = HTTP_CHUNK_SIZE;
		offset = range[0] + range[1];

		do {
			i = response->get_content_size() - 1;
			if(i >= 0) {
				if( !(owner->send_content(response, 
							response->get_content_chunk(i))) ) {
					buffer = response;
					response->content_chunks_sent(i);
					owner->subscribe(EVT_MESSAGE_DONE);
					return TRUE;
				}
				response->content_chunks_sent(i);
			}
			read_resource(response->get_request(), response, offset, length);
			response->set_read_range(offset, length);
			offset = (offset) + length;
		} while((offset-length) < total);
	}
	return TRUE;
}


/*
 * EVT_MESSAGE_DONE
 */
nomask void lwo_evt_message_done(object user)
{
	user->unsubscribe(EVT_MESSAGE_DONE);
	if(buffer) {
		object response;
		response = buffer;
		buffer = nil;
		if(!send(response, FALSE, TRUE)) {
			SYSLOG("Warning: Could not send buffered response...\n");
			response->set_status(500);
		}
	}
}

