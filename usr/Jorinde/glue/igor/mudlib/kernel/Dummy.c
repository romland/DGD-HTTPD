/*
 * This is a big big hack. The include files in this directory point to this
 * file.
 *
 * This file is just a placeholder for functions that are called by httpd's
 * default plugins and since it's designed for the Kernel Library by default
 * some libraries will not exist elsewhere. This object should probably never 
 * contain any functionality. You should be able to override everything in
 * the httpd-configuration files.
 */
void create(varargs mixed arg...) { }

void add_owner(varargs mixed arg...) { }

int access(string name, string file, int type)
{ 
	error("should not happen, change your httpd configuration");
}
