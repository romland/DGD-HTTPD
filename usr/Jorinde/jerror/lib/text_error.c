# include <trace.h>
# include <kernel/kernel.h>
# include <kernel/objreg.h>

# include "../include/error.h"
inherit LIB_HTTP_STRING;

void create(varargs int clone)
{
	/* ... */
}

/**
 * Shamelessly stolen from driver.c in the Kernel Library.
 */
string runtime_error_tostring(string error, int caught, mixed **trace)
{
    string line, function, progname, objname;
    int i, sz, len;
    object obj;
	string str;

	str = error;

	if (caught != 0) {
		str += " [caught]";
	}
	str += "\n";

	sz = sizeof(trace) - 1;

	for (i = 0; i < sz; i++) {
		progname = trace[i][TRACE_PROGNAME];
		len = trace[i][TRACE_LINE];
		if (len == 0) {
			line = "    ";
		} else {
			line = "    " + len;
			line = line[strlen(line) - 4 ..];
		}
		function = trace[i][TRACE_FUNCTION];
		len = strlen(function);
		if (progname == AUTO && i != sz - 1 && len > 3) {
			switch (function[.. 2]) {
			case "bad":
			case "_F_":
			case "_Q_":
				continue;
			}
		}
		if (len < 17) {
			function += "                 "[len ..];
		}

		objname = trace[i][TRACE_OBJNAME];
		if (progname != objname) {
			len = strlen(progname);
			if (len < strlen(objname) && progname == objname[.. len - 1] &&
				objname[len] == '#') {
				objname = objname[len ..];
			}
			str += line + " " + function + " " + progname + " (" + objname +
			   ")\n";
		} else {
			str += line + " " + function + " " + progname + "\n";
		}
	}
	return str;
}

static string compile_error_tostring(string file, int line, string err)
{
	return "Compile error: " + err + ", " + line + ": " + file + "\n";
}

