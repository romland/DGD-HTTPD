/**
 * Description: Implements WebDAV arbitrary properties.
 *				We have these in a persistent object because we want each
 *				to be swapped out of memory when they're not used.
 * Version:     $Id: storage.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/*
# define PERSISTENT
*/
# include "../include/dav.h"
# include <type.h>

# define IsInitialized() if(!owns) error("not initialized")
# define IsAllowed() (object_name(previous_object()) == DAV_INFOD)

# define PROP_VALUE		0
# define PROP_XMLNS		1
# define PROP_SIZE		2

inherit LIB_HTTP_STRING;

private string store;
string owns;

/* format: ([ resource : ([ property : ({ value, xmlns }), ... ]), ... ]) */ 
mapping props;


string set_xmlns() { error("deprecated: use set!\n"); }


static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif
	props = ([ ]);
}


/**
 * Name: load()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
void load()
{
	if(!IsAllowed()) {
		error("illegal call: " + object_name(previous_object()) + "\n");
	}

#ifndef PERSISTENT
	SYSLOG("loading saved storage: "+DAV_HOME+"save/"+store+".o"+"\n");
	restore_object(DAV_HOME + "save/" + store + ".o");
#endif
}


/**
 * Name: set_owns()
 * Desc: 
 * Note: 
 * Args: 
 * Rets: 
 */
void set_owns(string collection)
{
	if(!IsAllowed()) {
		error("illegal call: " + object_name(previous_object()) + "\n");
	}
	owns = collection;
	store = replace(owns, "/", "_");
}


/**
 * Name: get_owns()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
string get_owns()
{
	return owns;
}


/**
 * Name: get()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
mapping get(string resource, varargs string prop)
{
	if(!IsAllowed()) {
		error("illegal call: " + object_name(previous_object()) + "\n");
	}
	IsInitialized();

	if(!props[resource]) {
		return ([ ]);
	}

	if(!prop) {
		/*
		 * TODO: This will return a bit weird data, considering the fact
		 * that the caller might not expect an array with value, xmlns
		 */
		return props[resource];
	} else if(!props[resource][prop]) {
		return ([ ]);
	}
	return ([ prop : props[resource][prop][PROP_VALUE] ]);
}


string *names(string resource)
{
	if(!IsAllowed()) {
		error("illegal call: " + object_name(previous_object()) + "\n");
	}
	IsInitialized();

	if(!props[resource]) {
		return ({ });
	}
	return map_indices(props[resource]);
}


string get_xmlns(string resource, string prop)
{
	if(!IsAllowed()) {
		error("illegal call: " + object_name(previous_object()) + "\n");
	}
	IsInitialized();

	if(!props[resource]) {
		return nil;
	}
	return props[resource][prop][PROP_XMLNS];
}


/**
 * Name: set()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int set(string resource, string prop, mixed value, string xmlns)
{
	if(!IsAllowed()) {
		error("illegal call: " + object_name(previous_object()) + "\n");
	}

	IsInitialized();

	if(!props[resource]) {
		props[resource] = ([ ]);
	}

	if(prop == nil) {
		props[resource] = nil;
		save_object(DAV_HOME + "save/" + store + ".o");
		return TRUE;
	}

	props[resource][prop] = allocate(PROP_SIZE);
	props[resource][prop][PROP_VALUE] = value;
	if(xmlns) {
		props[resource][prop][PROP_XMLNS] = xmlns;
	}
#ifndef PERSISTENT /* hack-ish, but we want the xml saved, not the ob-ref */
	if(typeof(value) == T_OBJECT) {
		props[resource][prop][PROP_VALUE] = value->xml();
	}
	save_object(DAV_HOME + "save/" + store + ".o");
	props[resource][prop][PROP_VALUE] = value;
#endif
	return FALSE;
}

