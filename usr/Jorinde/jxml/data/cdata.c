/**
 * Description: XML CDATA object.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
#include "../include/xml.h"

inherit tag "../lib/std/tag";

void
constructor()
{
	tag::constructor();
}

int getType()
{
	return  XML_CDATA_NODE;
}

void setType()
{
	error("setting type is not allowed");
}

