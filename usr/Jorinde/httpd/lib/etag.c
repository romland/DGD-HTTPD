/**
 * Description: Implements etag.
 * Version:     $Id: etag.c 255 2004-07-09 14:45:41Z romland $
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
