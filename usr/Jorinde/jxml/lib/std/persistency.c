/**
 * Description: For subscribing to changes of an XML node.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../include/xml.h"

private int persistent;

nomask int dsp_layer()
{
	return persistent;
}

nomask int ds_verify(string datasource)
{
	/* TODO: Check whether "I" can access the .xml file. */
	/* Does the datasource end with .xml */
	return 0;
}

int ds_update(string datasource)
{
	/* TODO: Do something -- the underlying XML file changed */
}

