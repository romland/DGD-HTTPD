/**
 * Description: XML CDATA object.
 * Version:     $Id: cdata.c 167 2004-06-01 08:42:16Z romland $
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

