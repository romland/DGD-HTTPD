/**
 * Description: Read the correct error-page for http-errors. This file might
 *				need quite a bit of work in order to be 'hookable' to alien
 *				environments.
 * Version:     $Id: statusd_400_500.c 220 2004-06-16 20:28:52Z romland $
 * License:     see Jorinde/doc/License
 */
# include <kernel/kernel.h>
# include "../include/www.h"

private string errordir;

/* TODO: This entire file */
void create()
{
	/* ... */
}


int xml_parameter(string key, mixed value)
{
	if(key ==  "directory") {
		if(!sscanf(value, "%*s/"))
			errordir =  value + "/";
		else
			errordir = value;
	}
	return TRUE;
}


string error_page(object response)
{
	int status;
	status = response->get_status();

	switch(status) {
	case 401 :
	case 404 :
		return read_file(errordir + status + ".html");
	default :
		return read_file(errordir + "missing.html");;
	}
}

