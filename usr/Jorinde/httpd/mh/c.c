/**
 * Description: Mediahandler for files ending with '.c' (see as an example).
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include "../include/www.h"

inherit     LIB_HTTP_STRING;
inherit api API_HTTP_MEDIAHANDLER;
inherit lib LIB_HTTP_MEDIAHANDLER;


void create(varargs int clone)
{
	lib::create();
}


int get_supports(string filesuffix)
{
	::get_supports(filesuffix);
	return (filesuffix == "c") ? TRUE : FALSE;
}


int handle(Request request, Response response)
{
	if(!get_supports(request->get_uri()->get_filesuffix())) {
		return FALSE;
	}
	::handle(request, response);

	if(response->get_content_size()) {
		int i;
		string *content;

		content = response->get_content();
		for(i = 0; i < sizeof(content); i++) {
			content[i] = unentity(content[i]);
		}

		content[0] = "<PRE>\n" + content[0];
		content[sizeof(content)-1] += "</PRE>\n";

		response->set_header("Content-Length", response->get_content_length());
		response->set_header("Content-Type", "text/html");
	}
	return TRUE;
}

