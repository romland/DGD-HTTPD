/**
 * Description: Logging library.
 * Version:     $Id: log.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
#include "../include/xml.h"
#include <trace.h>

void no_DEBUG(string s) { }
void no_INFO(string s) { }
void no_WARN(string s) { }
void no_ERROR(string s) { }

string get_logging_item()
{
	string t, program;
	mixed *a;
	int i;

	a = explode(previous_program(1), "/");
	program = a[sizeof(a)-1] + ".c";
	i = sizeof(call_trace());
	a = call_trace()[i-4];
	t = program + "->" + a[TRACE_FUNCTION] + "() (" + a[TRACE_LINE] + ")";
	return t;
}

void _DEBUG(string s)
{
	SYSLOG("DEBUG: " + get_logging_item() + ": " + s + "\n");
}

void _WARN(string s)
{
	SYSLOG(" WARN: " + get_logging_item() + ": " + s + "\n");
}

void _INFO(string s)
{
	SYSLOG(" INFO: " + s + "\n");
}

void _ERROR(string s) {
	error("ERROR: " + get_logging_item() + ": " + s + "\n");
}

