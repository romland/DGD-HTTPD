# include "../../open/jorinde.h"

private object accessd;

static void create()
{
	accessd = find_object("obj/secure/player/access");
}

int access(string name, string file, int type)
{
	int ret;

	ret = accessd->check_access(name, file, type);
	SYSLOG(type + "-access " + (ret ? "granted" : "denied") + 
			" for " + name + " to " + file + "\n");
	return ret;
}
