/**
 * Description: Big-string implementation.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/shared.h"

#ifdef __IGOR__
inherit "/lib/lwo";
#endif
inherit bstr LIB_BSTR;

static void create(varargs int clone)
{
	bstr::create();
}

/**
 * TODO: We want to be able to set size of the chunks returned by the 
 * iterator, or possibly, 'get_next_line()'.
 */
