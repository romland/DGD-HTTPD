/**
 * Description: XML Comment object.
 * Version:     $Id$
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
	return  XML_COMMENT_NODE;
}

void setType()
{
	error("setting type is not allowed");
}

