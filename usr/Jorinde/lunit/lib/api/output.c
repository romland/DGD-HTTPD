/**
 * Description: Used by the unit-testing to output info 'on screen'.
 * Version:     $Id: output.c 226 2004-06-20 21:58:43Z romland $
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
