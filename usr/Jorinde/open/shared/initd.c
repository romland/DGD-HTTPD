/**
 * Description: Initialize Jorinde shared libraries.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "include/shared.h"

static void create(varargs int clone)
{
	COMPILE(ITERATOR);
	COMPILE(BSTR);
}
