/**
 * Description: Initialize XML.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "include/xml.h"

static void create(varargs int clone)
{
	COMPILE(JORINDE_XML+"sys/logd");
	COMPILE(JORINDE_XML+"data/dummy");
	COMPILE(JORINDE_XML+"data/cdata");
	COMPILE(JORINDE_XML+"data/characterdata");
	COMPILE(JORINDE_XML+"data/element");
	COMPILE(JORINDE_XML+"data/fragment");
	COMPILE(JORINDE_XML+"data/comment");
	COMPILE(JORINDE_XML+"data/root");
	COMPILE(JORINDE_XML+"data/pi");
	COMPILE(JORINDE_XML+"sys/xmlroot");

#if 0	
	COMPILE("sys/diffd");			/* 'subscribe' to changes of a doc */
#endif
}

