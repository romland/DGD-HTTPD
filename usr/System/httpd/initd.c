/**
 * Description: Initialize http-related objects in ~System.
 * Version:     $Id$
 * License:     see Jorinde/doc/License
 */

# include <kernel/kernel.h>
# include <kernel/user.h>
# include <kernel/rsrc.h>
# include "include/httpd.h"
# include "../jorinde.h"

inherit rsrc API_RSRC;

static void create(varargs int clone)
{
	object errd;
	int val;

	rsrc::create();

	/* TODO: We need to be able to configure most of this file */
	rsrc::add_owner("Jorinde");
	rsrc::add_owner("www");
	rsrc::add_owner("wwwuser");

	COMPILE(USR_SYSTEM+"httpd/binaryd");
	COMPILE(USR_SYSTEM+"httpd/data/tool");
	COMPILE(USR_SYSTEM+"httpd/syslogd");
	COMPILE(USR_SYSTEM+"httpd/accessd");
	COMPILE(USR_SYSTEM+"httpd/authenticated");
	COMPILE(USR+"/www/access");
	COMPILE(JORINDE_ERRORD+"sys/errord");

	errd = find_object(JORINDE_ERRORD+"sys/errord");
	errd->add_hook("/", nil, nil);

	/* Initialize all Jorinde applications (incl. httpd) */
	COMPILE(JORINDE+"initd");

# if 0
	/* TODO: Fix error handler so it doesn't suck so incredibly much when
	 * it's not a web-error. It seems as if we don't get compile errors
	 * if they occur during startup of driver?
	 */
	find_object(DRIVER)->set_error_manager(errd);
# endif
}

