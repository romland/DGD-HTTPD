/**
 * Description: Initialize the unit-testing framework.
 * Version:     $Id: initd.c 246 2004-06-24 17:52:43Z romland $
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
