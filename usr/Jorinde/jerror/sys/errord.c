/**
 * Description: Error-handler which supports multiple error-handlers and
 *				forwards each error to multiple daemons depending on what
 *				they wanted to get (depending on path).
 * Version:     $Id: errord.c 285 2004-08-06 14:54:11Z romland $
 * License:     see Jorinde/doc/License
 */

# include <kernel/kernel.h>
# include <trace.h>
# include <type.h>
# include "../include/error.h"

inherit LIB_HTTP_STRING;
inherit text LIB_TEXT_ERROR;


static object  driver;
static mapping hooks;

void create(varargs int clone)
{
	text::create(clone);

	driver = find_object(DRIVER);
	hooks = ([ ]);
}

void add_hook(string filepath, object obj, varargs mixed extra)
{
	if(filepath == nil) {
		error("Filepath to add_hook cannot be nil\n");
	}

	if(obj == this_object()) {
		error("obj == myself <invoking clue-bat on you>");
	}

	hooks[filepath] = ({ obj, extra });
}


static mixed **get_hooks(mixed **trace)
{
	int i;
	string *tmp;
	mixed  *ret;

	ret = ({ });
	tmp = map_indices(hooks);
	for(i = 0; i < sizeof(tmp); i++) {
		int j;
		for(j = 0; j < sizeof(trace); j++) {
			if(starts_with(trace[j][TRACE_PROGNAME], tmp[i]) == TRUE) {
				ret |= ({ hooks[tmp[i]] });
			}
		}
	}
	return ret;
}


void compile_error(string file, int line, string error)
{
	mixed **tmphooks, **trace;
	int i;

	trace = ({ allocate(6) });
	trace[0][TRACE_PROGNAME] = file;
	tmphooks = get_hooks(trace);

	if(sizeof(tmphooks) == 0) {
		SYSLOG("jerror: Will not forward compile errors to any object.\n");
		return;
	}

	for(i = 0; i < sizeof(tmphooks); i++) {
		mixed *hook;
		hook = tmphooks[i];
		if(hook[ERR_DAEMON] == nil) {
			SYSLOG("compile_error: hook nil, sending to SYSLOG()\n");
			SYSLOG(text::compile_error_tostring(file, line, error));
			continue;
		}

		if(hook[ERR_EXTRA_ARG] == nil) {
#if ERRORD_DEBUG
			SYSLOG("Errord calling: " + object_name(hook[ERR_DAEMON]) + 
					"->compile_error(" + file + ", " + line + ", " + error +
					")\n");
#endif
			call_other(hook[ERR_DAEMON], "compile_error",
				file, line, error);
#if ERRORD_DEBUG
			SYSLOG("Errord got back from call.\n");
#endif
		} else {
			call_other(hook[ERR_DAEMON], "compile_error", 
				file, line, error, hook[ERR_EXTRA_ARG]);
		}
	}
}


/* TODO: Need handler for atomic errors */
void atomic_error(string error, int atom, mixed **trace)
{
	SYSLOG("TODO: Atomic error: "
		+ error + ", "
		+ atom + ": "
		+ make_string(trace)
		+ "\n");
}


void runtime_error(string error, int caught, mixed **trace)
{
	mixed **tmphooks;
	string output;
	int i;

	tmphooks = get_hooks(trace);
	if(sizeof(tmphooks) == 0) {
		SYSLOG("jerror: Will not forward runtime errors to any object.\n");
		return;
	}

	for(i = 0; i < sizeof(tmphooks); i++) {
		mixed *hook;
		hook = tmphooks[i];

		if(hook[ERR_DAEMON] == nil) {
			SYSLOG("runtime_error: hook nil, sending to SYSLOG()\n");
			SYSLOG(text::runtime_error_tostring(error, caught, trace));
			continue;
		}

		if(hook[ERR_EXTRA_ARG] == nil) {
#if ERRORD_DEBUG
			SYSLOG("Errord calling: " + object_name(hook[ERR_DAEMON]) + 
					"->runtime_error(" + error +
					")\n");
#endif
			call_other(hook[ERR_DAEMON], "runtime_error", 
					error, caught, trace);
#if ERRORD_DEBUG
			SYSLOG("Errord got back from call.\n");
#endif
		} else {
			call_other(hook[ERR_DAEMON], "runtime_error",
					error, caught, trace, hook[ERR_EXTRA_ARG]);
		}
	}
}

