/**
 * Description: Helper class for LSP's (handles compile and recompile etc).
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include "../../include/www.h"
# include "include/lsp.h"
# include <limits.h>
#if 0
# include "../../include/error.h"
#endif
# define __CATCH__


inherit LIB_HTTP_STRING;

static string  lsp_home;
static mapping compiled;
static mapping debug_parsed_src;
static object  wwwaccess;

void create(varargs int clone)
{
	wwwaccess = find_object( WWW_ACCESS );
	compiled = ([ ]);
	debug_parsed_src = ([ ]);
}

void set_lsp_home(string a)
{
	lsp_home = a;

	/* Add hook for errors in LSP's path (which is defined in configuration
	 * file) to be forwarded to LSP_ERRORD. */
#if 0
	SYSLOG("Registering " + LSP_ERRORD + " as errord for files in " +
			lsp_home + "\n");
#endif
	find_object(JORINDE_ERRORD+"sys/errord")->add_hook(lsp_home, 
				find_object(LSP_ERRORD), nil);
	find_object(LSP_ERRORD)->set_compiler(this_object());
}

string get_lsp_home()
{
	return lsp_home;
}

int get_compile(object request, object response)
{
	string time, size, name, hash;

	time = response->get_header("Last-Modified");
	size = response->get_header("Content-Length");
	name = request->get_uri()->get_absolute_filename();
	hash = hash_md5(time, size, name);

	if( compiled[name] && compiled[name] == hash) {
		return FALSE;
	}
	return TRUE;
}

string get_filename(string hash)
{
	int i;
	string *arr;
	arr = map_indices(compiled);
	for(i = 0; i < sizeof(arr); i++) {
		if(hex_encode(compiled[arr[i]]) == hash) {
			return arr[i];
		}
	}
	return nil;
}

string get_parsed_source(string file)
{
	return debug_parsed_src[file];
}

mapping get_compiled_objects()
{
	int i;
	mapping map;
	string *arr;

	arr = map_indices(compiled);
	map = ([ ]);
	for(i = 0; i < sizeof(arr); i++) {
		map[arr[i]] = lsp_home + "data/" + hex_encode(compiled[arr[i]])+".c";
	}
	return map + ([ ]);
}


string get_lsp_set_fn()		{ error("deprecated"); }
string get_lsp_init_fn()	{ error("deprecated"); }
string get_lsp_output_fn()	{ return "response->write"; }
string get_lsp_main_fn()	{ return "__LSP_body"; }
string get_lsp_global()
{
	return "" +
#ifdef __IGOR__
		"inherit \"/lib/lwo\";" +
#endif
		"object request, response, server, application, session;" +
		"void __LSP_setwrap(object *a)" +
		"{" +
			"request = a[0];" +
			"response = a[1];" +
			"server = a[2];" +
			"application = a[3];" +
			"session = a[4];" +
		"}";

}

/**
 * TODO: This is JUST A PROOF OF CONCEPT! It's a hack.
 *		 This is also a VERY unforgiving parser (buggy, even). 
 *		 See it as alpha.
 */
string parse(string source)
{
	string parsed, global, pre, code, post;
	string *lines, line;
	string *blocks;
	int *blockinfo, i, j, matches;

	parsed    = "";
	blocks    = ({ });
	blockinfo = ({ });
	if((matches = sscanf(source, "%s<%%+%s+%%>%s", pre, global, post)) == 3) {
		blocks    += ({ pre });
		blockinfo += ({ LSP_BL_HTML });
	} else {
		blocks    += ({ pre });
		blockinfo += ({ LSP_BL_HTML });
		post = source;
	}

	i = 0;
	j = 0;
	/*
	 * TODO, FIXME: This 'inherit ' search is VERY dangerous as it will
	 * find it in comments and sometimes not at all. :-)
	 */
	while((i = index_of(i, global, "inherit ")) != -1) {
		i = index_of(i, global, ";");
		j = i;
	}

    while((i = index_of(j, global, "#")) != -1) {
        i = index_of(i, global, "\n");
        j = i;
    }

	if(j) {
		global = global[0..j] + get_lsp_global() + global[j+1..];
	} else {
		global = get_lsp_global() + global;
	}

	blocks    += ({ global });
	blockinfo += ({ LSP_BL_GLOBAL });

	while(post && (matches=sscanf(post, "%s<%%%s%%>%s",pre,code,post)) >= 2) {
		blocks    += ({ pre });
		blockinfo += ({ LSP_BL_HTML });

		blocks    += ({ code });
		blockinfo += ({ LSP_BL_CODE });
	}

	if(matches == 0) {
		blocks    += ({ post });
		blockinfo += ({ LSP_BL_HTML });
	}

	parsed += "void " + get_lsp_main_fn() + "() {\n";
	for(i = 0; i < sizeof(blocks); i++) {
		code = blocks[i];
		if(!code) {
			continue;
		}

		switch(blockinfo[i]) {
		case LSP_BL_HTML :
			lines = explode(code, "\n");
			for(j = 0; j < sizeof(lines); j++) {
				if(j) {
					parsed += " +\n";
				} else {
					parsed += get_lsp_output_fn() + "(";
				}
				line = lines[j];
				line = replace(line, "\"", "\\\"") ;
				line = "\"" + line + "\""; 
				parsed += line;
			}
			if(j > 1) {
				parsed += ");\n"; /* Do we want this newline? */
			} else if(j) {
				 parsed += ");";
			}
			break;

		case LSP_BL_GLOBAL :
			parsed = code + parsed;
			break;

		case LSP_BL_CODE :
			if(strlen(code) && code[0..0] == "=") {
				code = get_lsp_output_fn() + "(" + code[1..] + ");\n";
			}
			parsed += code;
			break;

		default :
			error("Unknown block.\n");
			break;
		}
	}
	parsed += "}";

#if 0
	SYSLOG("Parsed: " + parsed);
#endif
	return parsed;
}


object compile(object request, object response, string source)
{
	string err, name, hash;

	name = request->get_uri()->get_absolute_filename();
	if(source != nil) {
		string time, size;

		/* TODO: remove this line when stable -- memory consumption */
		debug_parsed_src[name] = source;

		time = response->get_header("Last-Modified");
		size = response->get_header("Content-Length");
		hash = hash_md5(time, size, name);

        compiled[name] = hash;
		hash = hex_encode(hash);
		SYSLOG("(Re)compiling "+name+" as "+lsp_home+"data/" + hash + "\n");
#ifdef __CATCH__
		err = catch(
			wwwaccess->_HTTPD_compile_object(lsp_home + "data/" + hash, source)
		);
#else
		wwwaccess->_HTTPD_compile_object(lsp_home + "data/" + hash, source);
#endif
		if(err) {
			SYSLOG("LSP-compiler: failed to compile.\n");
			compiled[name] = "not compiled";
			return nil;
		}
	} else {
		hash = hex_encode(compiled[name]);
	}

	if(find_object(lsp_home + "data/" + hash)) {
		return new_object( lsp_home + "data/" + hash );
	} else {
		SYSLOG("object not found: " + lsp_home + "data/" + hash + "\n");
	}
	return nil;
}

int run(object program, mapping interfaces)
{
	string err;
/*
	if(!SYSTEM()) {
		error("illegal call\n");
		return FALSE;
	}
*/
	call_other(program, "__LSP_setwrap",
			({ 
				interfaces["request"],
				interfaces["response"],
				interfaces["server"],
				interfaces["application"],
				interfaces["session"]
			})
		);

#ifdef __CATCH__
	catch {
		call_other(program, get_lsp_main_fn());
	} : return FALSE;
#else
	call_other(program, get_lsp_main_fn());
#endif
#if 1
    /* This is a HUGE SECURITY hole (read as ~Jorinde (not as user)).
    SYSLOG("reading file (without checking access): "
        + interfaces["request"]->abs_filename() + "\n");
*/
    interfaces["response"]->write("\n\n\n\n\n\n"
        + "<DIV STYLE=\"position:absolute;top:1600;left:0px;"
        + "border:1px solid;height:300px;background-color:#d0d0f0;"
        + "overflow-y:hidden;font-size:10px;overflow: auto; "
        + "width: 300px; \">"
        + "<pre>"
        + unentity(
			wwwaccess->_HTTPD_read_file( interfaces["request"]->abs_filename() )
		  )
        + "</pre>"
        + "</div>"
        + "");
                                                                                
    interfaces["response"]->write(""
        + "<DIV STYLE=\"position:absolute;top:1600;left:300px;"
        + "border:1px solid;height:300px;background-color:#d0d0f0;"
        + "overflow-y:hidden;font-size:10px;overflow: auto; "
        + "width: 300px; \">"
        + "<pre>"
        + unentity(debug_parsed_src[interfaces["request"]->abs_filename()])
        + "</pre>"
        + "</div>"
        + "");
#endif

	return TRUE;
}

