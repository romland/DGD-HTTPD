/**
 * Description: XML characterdata object.
 * Version:     $Id: characterdata.c 226 2004-06-20 21:58:43Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
#include "../include/xml.h"

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

inherit tag "../lib/std/tag";

void
constructor()
{
	tag::constructor();
}

int getType()
{
	return  XML_CHARACTER_DATA;
}

void setType()
{
	error("setting type is not allowed");
}

