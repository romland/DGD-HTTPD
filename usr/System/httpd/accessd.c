/**
 * Description: Grants access to querying access in accessd for everyone.
 * Version:     $Id$
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
