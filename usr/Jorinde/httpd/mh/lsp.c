/**
 * Description: Mediahandler for files ending with '.lsp'.
 * Version:     $Id: lsp.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include "../include/www.h"
#if 0
# include "../include/error.h"
#endif
# include "lsp/include/lsp.h"

inherit LIB_HTTP_STRING;
inherit API_HTTP_MEDIAHANDLER;
inherit lib LIB_HTTP_MEDIAHANDLER;

static object compiler;
static object htmlerrd;

void create(varargs int clone)
{
	if(!find_object(LSP_ERRORD))		compile_object(LSP_ERRORD);
	if(!find_object(LSP_COMPILER))		compile_object(LSP_COMPILER);
	if(!find_object(LSP_REQUEST_WRAP))	compile_object(LSP_REQUEST_WRAP);
    if(!find_object(LSP_RESPONSE_WRAP))	compile_object(LSP_RESPONSE_WRAP);
    if(!find_object(LSP_APPLICATION_WRAP)) compile_object(LSP_APPLICATION_WRAP);
    if(!find_object(LSP_SERVER_WRAP)) compile_object(LSP_SERVER_WRAP);
    if(!find_object(LSP_SESSION_WRAP)) compile_object(LSP_SESSION_WRAP);

	compiler = find_object(LSP_COMPILER);
    htmlerrd = find_object(LSP_ERRORD);
	
	lib::create();
}

int xml_parameter(string key, mixed value)
{
	switch(key) {
	case "lsp_home" :
		compiler->set_lsp_home(value);
		return 1;
	}

	return 0;
}


object get_errord()
{
	return htmlerrd;
}

object get_compiler()
{
	return compiler;
}

int get_supports(string filesuffix)
{
	::get_supports(filesuffix);
	return (filesuffix == "lsp") ? TRUE : FALSE;
}

int get_need_content(object request, object response)
{
	if(compiler->get_compile(request, response) == TRUE)
		return TRUE;
	return FALSE;
}

static object new_wrapper(string program)
{
	return new_object( program );
}

int handle(object request, object response)
{
	int i;
	string *arr, content;
	mapping interfaces;
	object page, tmp;

	if(!get_supports(request->get_uri()->get_filesuffix())) {
		return FALSE;
	}
	::handle(request, response);

	interfaces = ([ ]);
	interfaces["request"]     = new_wrapper(LSP_REQUEST_WRAP);
	interfaces["response"]    = new_wrapper(LSP_RESPONSE_WRAP);
	interfaces["application"] = new_wrapper(LSP_APPLICATION_WRAP);
	interfaces["server"]      = new_wrapper(LSP_SERVER_WRAP);
	interfaces["session"]     = new_wrapper(LSP_SESSION_WRAP);

	tmp = previous_object()->get_session_object(response->get_session_id());
	if(tmp == nil) {
		error("Could not obtain a session object!\n");
	}
#if 0
	interfaces["session"]     = tmp;
#endif

    arr = map_indices(interfaces);
    for(i = 0; i < sizeof(arr); i++) {
        call_other(interfaces[arr[i]], "set_related",
			request, response, previous_object(),
			previous_object()->get_server(), tmp
		);
    }

	if(compiler->get_compile(request, response) == TRUE) {
		/* This seems to be a modified source, re-parse */
		content = response->content_tostring();

		if((content = compiler->parse(content)) == nil) {
			return FALSE;
		}
		response->clear_content();
	} else {
		content = nil;
		response->clear_content();
	}

	page = compiler->compile( request, response, content );
	if(page) {
		compiler->run(page, interfaces);
	}

	response->set_header("Content-Length", response->get_content_length());
	response->set_header("Content-Type", "text/html");
	return TRUE;
}

