/**
 * Description: Initialize the unit-testing framework.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../open/jorinde.h"

static void create()
{
	COMPILE(JORINDE_LUNIT+"data/runner");
	COMPILE(JORINDE_LUNIT+"data/test");
	COMPILE(JORINDE_LUNIT+"sys/logd");
	COMPILE(JORINDE_LUNIT+"sys/writed");
}
