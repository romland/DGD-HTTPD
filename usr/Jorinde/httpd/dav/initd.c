# include "include/dav.h"
# include "../include/object.h"

private object server;

static void create(varargs int clone)
{
	server = find_object(HTTP_SERVER);

	COMPILE( DAV_INFOD );
	COMPILE( DAV_STORAGE );
}

object get_server()
{
	return server;
}

