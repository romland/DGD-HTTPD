/*
 * THIS SOLUTION IS ONE GIANT HACK, I WILL FIX IN FUTURE VERSION,
 * IT'S JUST TOO MUCH RE-WRITING AND RE-PONDERING! :P
 */

# include "../include/httpd.h"
# include <kernel/kernel.h>

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

inherit tool USR_SYSTEM+"httpd/lib/tool";

#define Jorinde() (sscanf(object_name(previous_object()),USR+"/Jorinde/%*s")==1)
#define Illegal() error("illegal call")

static void create(varargs int clone)
{
	if(!KERNEL() && !SYSTEM()) {
		error("illegal call");
	}

	tool::create();
}

static void message(string str)
{ this_user()->set_error("Kernel message: " + str); }

mixed **get_dir(string file)
{ 
	if(!Jorinde()) Illegal(); 
#ifdef __IGOR__
	return ::get_dir_info(file);
#else
	return ::get_dir(file);
#endif
}

int make_dir(string dir)
{ if(!Jorinde()) Illegal(); return ::make_dir(dir); }

string read_file(string file, varargs int offset, int size)
{
	if(!Jorinde()) Illegal(); 
#ifdef __IGOR__
	return :: read_bytes(file, offset, size);
#else
	return :: read_file(file, offset, size);
#endif
}

int remove_dir(string dir)
{ if(!Jorinde()) Illegal(); return ::remove_dir(dir); }

int remove_file(string file)
{ if(!Jorinde()) Illegal(); return ::remove_file(file); }

int rename_file(string from, string to)
{ if(!Jorinde()) Illegal(); return ::rename_file(from, to); }

int write_file(string file, string str, varargs int offset)
{ 
	if(!Jorinde()) Illegal(); 
#ifdef __IGOR__
	return ::write_bytes(file, (offset ? offset : 0), str);
#else
	return ::write_file(file, str, offset);
#endif
}

