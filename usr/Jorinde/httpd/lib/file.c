/**
 * Description: Perform file operations using standard e/kfuns or forward
 *				the operations to the access-elevation layer in webroot. 
 * Version:     $Id: file.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/*
 * THIS FILE DOES NOT DO ANY ACCESS CHECKING FOR OPERATIONS IN WEBROOT, THAT
 * SHOULD BE DONE BEFORE CALLING ANYTHING IN HERE. THIS IS SUBJECT TO BE
 * FIXED AT SOME POINT. OR NOT.
 */

# include <kernel/kernel.h>
# include "../include/www.h"
# include <type.h>

#define IsWeb(x)	(sscanf(x, get_owner()->get_application()->get_webroot() + "%*s") == 1 && get_owner()->get_application()->get_allow_anonymous() == TRUE)

#define ELEVATOR	(get_owner()->get_application()->get_elevator())

private object tool, owner;


static void create()
{
/*	tool::create();*/
}


nomask void set_tool(object t)
{
	if(!SYSTEM()) {
		error("illegal call");
	}
	tool = t;
}


nomask void set_owner(object user)
{
	if(!SYSTEM()) {
		error("illegal call");
	}
	owner = user;
}
                                                                                
nomask object get_owner() { return owner; }


/**
 * Name: get_dir()
 * Desc: Perform requested file operation either as user or as 'anonymous'
 *		 user in application's 'webroot'.
 * Note: For full description see the kfun and/or afun/efun documentation.
 * Args:
 * Rets: 
 */
static mixed **get_dir(string file)
{
	if(IsWeb(file)) {
		return (ELEVATOR)->_HTTPD_get_dir(file);
	}

#ifdef __IGOR__
	return tool->get_dir_info(file);
#else
	return tool->get_dir(file);
#endif
}

#ifdef __IGOR__
static mixed **get_dir_info(string file)
{
	return get_dir(file);
}
#endif

/**
 * Name: make_dir()
 * Desc: Perform requested file operation either as user or as 'anonymous'
 *       user in application's 'webroot'.
 * Note: For full description see the kfun and/or afun/efun documentation.
 * Args:
 * Rets:
 */
static int make_dir(string dir)
{
	if(IsWeb(dir)) {
		return (ELEVATOR)->_HTTPD_make_dir(dir);
	}
	return tool->make_dir(dir);
}


/**
 * Name: read_file()
 * Desc: Perform requested file operation either as user or as 'anonymous'
 *       user in application's 'webroot'.
 * Note: For full description see the kfun and/or afun/efun documentation.
 * Args:
 * Rets:
 */
static string read_file(string file, varargs int offset, int size)
{
	if(IsWeb(file)) {
		return (ELEVATOR)->_HTTPD_read_file(file, offset, size);
	}
	return tool->read_file(file, offset, size);
}


/**
 * Name: remove_dir()
 * Desc: Perform requested file operation either as user or as 'anonymous'
 *       user in application's 'webroot'.
 * Note: For full description see the kfun and/or afun/efun documentation.
 * Args:
 * Rets:
 */
static int remove_dir(string dir)
{
	if(IsWeb(dir)) {
		return (ELEVATOR)->_HTTPD_remove_dir(dir);
	}
	return tool->remove_dir(dir);
}


/**
 * Name: remove_file()
 * Desc: Perform requested file operation either as user or as 'anonymous'
 *       user in application's 'webroot'.
 * Note: For full description see the kfun and/or afun/efun documentation.
 * Args:
 * Rets:
 */
static int remove_file(string file)
{
	if(IsWeb(file)) {
		return (ELEVATOR)->_HTTPD_remove_file(file);
	}
	return tool->remove_file(file);
}


/**
 * Name: rename_file()
 * Desc: Perform requested file operation either as user or as 'anonymous'
 *       user in application's 'webroot'.
 * Note: For full description see the kfun and/or afun/efun documentation.
 * Args:
 * Rets:
 */
static int rename_file(string from, string to)
{
	if(IsWeb(from) && IsWeb(to)) {
		return (ELEVATOR)->_HTTPD_rename_file(from, to);
	}
	return tool->rename_file(from, to);
}


/**
 * Name: write_file()
 * Desc: Perform requested file operation either as user or as 'anonymous'
 *       user in application's 'webroot'.
 * Note: For full description see the kfun and/or afun/efun documentation.
 * Args:
 * Rets:
 */
static int write_file(string file, string str, varargs int offset)
{
	if(IsWeb(file)) {
		return (ELEVATOR)->_HTTPD_write_file(file, str, offset);
	}
	return tool->write_file(file, str, offset);
}


/**
 * Largely stolen from the wiztool.c in DGD's Kernel Library.
 */
static int copy_file(string src, string dest)
{
	int offset, n, sz;
	string chunk;
	
	offset = 0;
	do {
		chunk = read_file(src, offset, 57344);
		if(typeof(chunk) != T_STRING) {
			return FALSE;
		}
		n = write_file(dest, chunk);
		if(n <= 0) {
			return FALSE;
		}
		offset += strlen(chunk);
		sz -= strlen(chunk);
	} while(sz > 0 && strlen(chunk) != 0);

	return TRUE;
}


static int file_type(string path)
{
	mixed **dir;
	dir = get_dir(path);
	if(sizeof(dir[0]) == 0) {
		return F_NONEXISTING;
	}

	if(dir[1][0] == -2) {
		return F_DIRECTORY;
	}
	return F_FILE;
}


static int file_exists(string path)
{
	return (file_type(path) != F_NONEXISTING);
}


static int is_dir(string path)
{
	return (file_type(path) == F_DIRECTORY);
}


static int is_file(string path)
{
	return (file_type(path) == F_FILE);
}

