/**
 * Description: Initialize Jorinde shared libraries.
 * Version:     $Id: initd.c 210 2004-06-15 08:27:28Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "include/shared.h"

static void create(varargs int clone)
{
	COMPILE(ITERATOR);
	COMPILE(BSTR);
}
