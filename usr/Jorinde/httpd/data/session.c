/**
 * Description: Implements 'persistent' http-sessions that's kept alive longer
 *              than an actual user object.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include "../include/www.h"

inherit props JORINDE_SHARED+"lib/properties";
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

#if 0
static mapping values;
#endif
static string  id;
static int     expires, remove;
static object  app;

void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	props::create();
}

void set_id(string arg)				{ id = arg;				}
void set(mixed key, mixed value)	{ set_property(key, value);	}
void set_expires(int when)			{ expires = when;		}
void set_remove(int arg)			{ remove = arg;			}
void set_application(object arg)	{ app = arg;			}

mixed   get(mixed key)				{ return get_property(key);	}
mapping get_all()					{ return get_properties();	}
string  get_id()					{ return id;			}
int     get_expires()				{ return expires;		}
int     get_remove()				{ return remove;		}
object  get_application()			{ return app;			}


#define BC			"$BREADCRUMBS"
#define BC_SIZE		50
void set_breadcrumb(string page)
{
	string *bc;

	bc = get_property(BC);
	if(!bc || !sizeof(bc)) {
		bc = ({ ({ page, time() }) });
	} else {
		bc += ({ ({ page, time() }) });
	}
	if(sizeof(bc) > BC_SIZE) {
		bc = bc[1..];
	}
	set_property(BC, bc);
}

string **get_breadcrumbs()
{
	return get_property(BC);
}

