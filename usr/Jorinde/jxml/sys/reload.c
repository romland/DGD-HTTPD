/**
 * Description: This will destruct all objects within XML_DIR, this 
 *				should only be used during development or testing. This 
 *				function is somewhat dangerous seeing as it destroys 
 *				master objects located in data/ as well -- those are 
 *				LWO:s -- if an LWO's master object is destroyed so are 
 *				all its 'clones'. Handle with care.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
#include "../include/xml.h"

void
dest(string f)
{
	if(catch( destruct_object(find_object( XML_DIR + f)) )) {
		SYSLOG("failed to destruct '" + XML_DIR + f + "'...\n");
		return;
	}
}


int
authorized()
{
	return 1;
}


void 
destruct_xml()
{
	if(authorized()) {
		dest("data/cdata");
		dest("data/characterdata");
		dest("data/element");
		dest("data/fragment");
		dest("data/comment");
		dest("data/root");
		dest("data/pi");
	}

	dest("lib/std/attributes");
	dest("lib/std/serialize");
	dest("lib/std/tag");
	dest("lib/std/parse");
	
	dest("lib/file");
	dest("lib/string");
	dest("lib/log");
	dest("lib/dom_info");

	/*dest("example/testing");*/
	
	/* This file */
	call_out("dest", 0, "lib/reload");
}

