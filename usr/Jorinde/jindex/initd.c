/**
 * Description: Initialize Searching/indexing engine.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../open/jorinde.h"
# include "./include/index.h"

static void create(varargs int clone)
{
	COMPILE( INDEX_UNIQUE );
	COMPILE( INDEX_OBJECT );
	COMPILE( INDEXD );

#if 1
	(JORINDE_INDEX+"sys/indexd")->create_index("/doc/");
#if 0
	(JORINDE_INDEX+"sys/indexd")->visualize_index();
#endif
#endif
}
