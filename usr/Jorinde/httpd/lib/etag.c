/**
 * Description: Implements etag.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/www.h"

static void create()
{
}

static string get_etag(string filename, string length, string modified)
{
	return "" + hash_crc16(filename, length, modified);
}
