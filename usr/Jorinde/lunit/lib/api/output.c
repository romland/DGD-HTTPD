/**
 * Description: Used by the unit-testing to output info 'on screen'.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../../include/internal/path.h"

static void create()
{
}

static void output(string str)
{
	(LUNIT_WRITER)->write(str);
}
