/**
 * Description: Implements functionality for arbitrary properties.
 * Version:     $Id: properties.c 226 2004-06-20 21:58:43Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/shared.h"
# include <type.h>

private mapping properties;

static void create()
{
	properties = ([ ]);
}


int set_property(mixed key, mixed value)
{
#if 0 /* TODO */
	if(typeof(key) == T_STRING && strlen(key) > 0 && key[0] == '$' && 
			previous_object() != this_object() ) {
		error("properties starting with $ are private");
	}
#endif 
	properties[key] = value;
	return TRUE;
}


mixed get_property(mixed key)
{
	return properties[key];
}


int remove_property(mixed key)
{
#if 0 TODO
	if(typeof(key) == T_STRING && strlen(key) > 0 && key[0] == '$' && 
			previous_object() != this_object() ) {
		error("properties starting with $ are private");
	}
#endif
	if(!properties[key]) {
		return FALSE;
	}

	properties[key] = nil;
	return TRUE;
}


mapping get_properties()
{
	return ( properties ? properties + ([ ]) : ([ ]) );
}
