/**
 * Description: Grants access to querying access in accessd for everyone.
 * Version:     $Id: accessd.c 55 2004-04-05 08:33:49Z romland $
 * License:     see Jorinde/doc/License
 */

# include <kernel/kernel.h>
# include <kernel/access.h>

inherit access API_ACCESS;

static void create(varargs int clone)
{
	access::create();
}

int access(string name, string file, int type)
{
	return ::access(name, file, type);
}
