/**
 * Description: Daemon to output information 'on screen'.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/path.h"

void write(mixed str)
{
#ifdef STDOUT
	STDOUT(""+str);
#else
	SYSLOG(str+ "");
#endif
}

