/**
 * Description: Handles and prettifies errors for output on scripted
 *				web-pages (LSP's)
 * Version:     $Id: htmlerrord.c 172 2004-06-01 19:34:08Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include <kernel/kernel.h>
# include "../../../include/www.h"
#if 0
# include "../../../include/error.h"
#endif
# include "../include/lsp.h"
# include <trace.h>

inherit LIB_HTTP_STRING;

static object compiler, wwwaccess;
static string lsp_home;

void create(varargs int clone)
{
	wwwaccess = find_object( WWW_ACCESS );
}

void set_compiler(object o)
{
	lsp_home = o->get_lsp_home();
	compiler = o;
}

static string read_source(string filename, int line, int span)
{
	string *lines, code;
	int j;

	lines = explode(wwwaccess->_HTTPD_read_file(filename), "\n");

	code = "";
	if(sizeof(lines) > span && line > span) {
		j = (line - span);
	} else {
		j = 0;
	}

	for(; j < sizeof(lines); j++) {
		if((j + 1) == line) {
			code += "<span style=\"background-color:yellow\">";
		}
		code += (j + 1) + ": " + unentity(lines[j]) + "<br>";
		if(j > (line + span)) {
			break;
		}
		if((j + 1) == line) {
			code += "</span>";
		}
	}

	code = "<pre style=\"font-size: 8px\">" + code + "</pre>";
	return code;
}

string html_tr(string style, string *columnstyle, string *columns)
{
	string ret;
	int i;

	if(sizeof(columnstyle) != sizeof(columns)) {
		error("number of styles does not match number of columns\n");
	}

	ret = "<tr style=\"" + style + "\">\n";
	for(i = 0; i < sizeof(columns); i++) {
		ret += "\t<td style=\"" + columnstyle[i] + "\">\n"
			+  "\t\t" + columns[i] + "\n"
			+  "\t</td>\n";
	}
	ret += "</tr>\n";
	return ret;
}

static mixed **clarify_lsp_stack(mixed **trace);

void runtime_error(string error, int caught, mixed **trace, varargs mixed format)
{
	int i, line;
	string ret;
	string color, align;
	string func, prog, obj, code;

	trace = clarify_lsp_stack(trace);
	ret = "";
	for(i = 0; i < (sizeof(trace)-1); i++) {
		line = trace[i][TRACE_LINE];
		func = trace[i][TRACE_FUNCTION];
		prog = trace[i][TRACE_PROGNAME];
		obj  = trace[i][TRACE_OBJNAME];

		ret += html_tr(
			"font-size:10px;font-family: sans-serif;" +
			"border-bottom:solid 1px;" +
			"vertical-align:top;background-color:#"+(i%2?"ffffff":"f0f0f0"),
			({
				"text-align: right;", "",
				"", ""
			}),
			({
				 (string)(line?line:"nil"), func?func:"nil",
				 prog?prog:"nil", obj?obj:"nil"
			})
			);
	}

	/* leading "> to end any HTML tags we might have ended up in */
	ret = "\n\n\nERROR\">\n"
		+ "<table cellspacing=1 cellpadding=1 border=0 bgcolor=#cccccc>"
		+ "<tr>"
		+ "<th style=\"background-color:#a0a0ff;border:solid 1px\" "
		+ "align=left colspan=4>"
		+ "<h3>"
		+ "Error: " + (error?error:"?") + (caught?" [caught " + caught +"]":"")
		+ "</h3>"
		+ "</th>"
		+ "</tr>\n"
		+ "<tr>"
		+ "<td style=\"border-bottom:1px solid;\">Line</td>"
		+ "<td style=\"border-bottom:1px solid;\">Function</td>"
		+ "<td style=\"border-bottom:1px solid;\">Program</td>"
		+ "<td style=\"border-bottom:1px solid;\">Object</td>"
		+ "</tr>"
		+ ret
		+ "</table>\n";

	if(this_user()) {
		this_user()->set_error(ret);
	} else {
		SYSLOG("html errord, warning: No this_user(): " + ret + "\n");
	}
}

static void debug_parsed_src(string prog)
{
    string src, *arr;
    int i;

    arr = explode( compiler->get_parsed_source(prog), "\n");
    src = "";
    for(i = 0; i < sizeof(arr); i++) { src += (i+1) + "\t" + arr[i] + "\n"; }
    SYSLOG("htmlerrord:\n" + src);
}

void compile_error(string file, int line, string error, varargs mixed format)
{
	string ret, prog;

	if(!lsp_home) {
		SYSLOG("ERROR: lsp_home is not set, htmlerrord will fail (bailing)");
		return;
	}

	prog = file[strlen(lsp_home + "data/")..];
	if((prog = compiler->get_filename(prog)) == nil) {
		/*
		 * Compile-error was not in an LWO.
		 *
		 * TODO: SECURITY hole. This enables you to reference (by inherit for
		 * for instance) any file and get a snippet out of that code. This
		 * should be moved out of ~System and into ~www
		 */
		prog = file;
	}

	debug_parsed_src(prog);

	ret = "" 
		+ "<table width=\"60%\" cellspacing=1 cellpadding=1 border=0 bgcolor=#f0f0f0>\n"
		+ "<tr>\n"
		+ "<td style=\"background-color:#a0a0ff;border:solid 1px\" "
		+ "align=left colspan=2>\n"
		+ "<h3>"
		+ "Compile error: " + error
		+ "</h3>"
		+ "</td>\n"
		+ "</tr>\n"
		+ "<tr>\n"
		+ "<td style=\"border-bottom:1px solid;\">Line " + line + "</td>\n"
		+ "<td width=\"90%\"style=\"border-bottom:1px solid;\">in " + prog + "</td>\n"
		+ "</tr>\n"
		+ "<tr>\n"
		+ "<td style=\"border-bottom:1px solid;\">&nbsp;" + "</td>\n"
		+ "<td style=\"border-bottom:1px solid;\">" + prog + "<br>"
		+ read_source(prog, line, 5)
		+ "</td>\n"
		+ "</tr>\n"
		+ "</table><br>\n\n"
		+ "";

    if(this_user()) {
        this_user()->set_error(ret);
    } else {
        SYSLOG("html errord, warning: No this_user(): " + ret + "\n");
    }
}

static mixed **clarify_lsp_stack(mixed **trace)
{
	int i, lsp_error, line;
	string func, prog, obj, code;

	if(!compiler || !lsp_home) {
		SYSLOG("Warning: No compiler/lsp_home set for clarify_lsp_stack()\n");
		return trace;
	}

	for(i = 0; i < sizeof(trace); i++) {
		line = trace[i][TRACE_LINE];
		func = trace[i][TRACE_FUNCTION];
		prog = trace[i][TRACE_PROGNAME];
		obj  = trace[i][TRACE_OBJNAME];

		switch(prog) {
		case LSP_COMPILER :
			lsp_error = TRUE;
			break;
		}

		if(lsp_error && starts_with(prog, lsp_home + "data/")) {
			/* If it's an LWO, show the file's name instead of the object's */
			prog = compiler->get_filename(prog[strlen(lsp_home + "data/")..]);
			if(prog != nil) {
				obj = prog + "#-1";
				if(func == compiler->get_lsp_main_fn()) {
					func = "[none]";
				}
				prog = "<strong>" + prog + "</strong><br>" +
					read_source(prog, line, 5);
			} else {
				prog = "oddities!";
			}
			trace[i][TRACE_LINE]	 = line;
			trace[i][TRACE_FUNCTION] = func;
			trace[i][TRACE_PROGNAME] = prog;
			trace[i][TRACE_OBJNAME]  = obj;
		}
	}
	return trace;
}

