/**
 * Description: Minimal xpath implemenation.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# define LOG_LEVEL_DEBUG
# define LOG_LEVEL_INFO
# define LOG_LEVEL_WARN
# define LOG_LEVEL_ERROR

# include <type.h>
# include "../../include/log.h"
# include "../../include/xml.h"

inherit "../string";

/* debug messages off */
#undef SYSLOG
#define SYSLOG 


private object *getXpathNodes(string name);

void constructor()
{
	/* ... */
}

private mixed *flatten_array(mixed *arr)
{
	int i;
	mixed *ret;
	ret = ({ });
	for(i = 0; i < sizeof(arr); i++) {
		if(typeof(arr[i]) == T_ARRAY) {
			ret += flatten_array(arr[i]);
		} else {
			ret += ({ arr[i] });
		}
	}

	ret -= ({ nil });
	return ret;
}

/*
 * xpath basics:
 * http://www.w3schools.com/xpath/xpath_syntax.asp
 */
object *xpath(string str, varargs mixed index)
{
	object *nodes, *ret, to;
	string *path;
	int i;
	
	if(sscanf(str, "%*s|%*s") > 1) {
		error("multiple questions not supported\n");
	}

	if(index == nil) {
		index = 0;
	}
	path = explode(str, "/");
	to = this_object();

	/* xpath: [ //name ], [ // ] and [ //* ] */
	if(index == 0 && sizeof(path) > 1 && path[0] == "" && path[1] == "") {
		if(!to->isRoot()) {
			error("I am not a root-node.\n");
		}
		nodes = to->getIndex();
		if(sizeof(path) == 2 || path[2] == "" || path[2] == "*") {
			return nodes;
		}

		/* Get all nodes with matching name in flat document structure */
		for(i = 0; i < sizeof(nodes); i++) {
			if(nodes->getName() == path[2]) {
				ret += ({ nodes[i] });
				SYSLOG("//name; adding :: " + make_string(ret) + "\n");
			}
		}
		return ret;
	}

	if(index == sizeof(path)) {
		SYSLOG("We're on last item, returning to.\n");
		return ({ to });
	} else if(index > sizeof(path)) {
		SYSLOG("array index too high, returning nil\n");
		return nil;
	}						
	SYSLOG( object_name(to) + "->xpath('"+path[index]+"', " +index+ ")\n" );

	nodes = getXpathNodes(path[index]);
	if(nodes == nil) {
		SYSLOG("nodes = nil\n");
		return nil;
	} else if(sizeof(nodes) == 0 && path[index] == to->getName() ) {
		SYSLOG("nodes = to\n");
		nodes = ({ to });
	} else if(sizeof(nodes) < 1) {
		SYSLOG("not found: " + path[index] + " (0) - adding dummy\n");
		/*
		 * HACK TODO FIXME: We always want to return at least
		 * one element. The alternative is dead-ugly code on the
		 * caller's side where they have to check sizeof on
		 * the returned array all the time. I don't know how I want
		 * to get around this, yet... I made a bad design-choice here.
		 * Sue me. :)
		 */
		ret = ({ new_object(XML_LWO_DIR + "dummy") });
		return ret;
	}

	/* Pass on xpath to matching children */
	ret = ({ });
	for(i = 0; i < sizeof(nodes); i++) {
		mixed *tmp;
		tmp = ({ nodes[i]->xpath( str, (index + 1) ) });
		if(tmp != nil) {
/*			SYSLOG("adding '" + nodes[i]->getName() + "' to ret...\n");*/
			ret += tmp;
		}
	}

	if(index == 0) {
		/* Hack */
		ret = flatten_array(ret);
	}

	SYSLOG("depth " + index + " :: " + make_string(ret) + "\n");
	return ret;
}


private object *getXpathNodes(string name) 
{
	int i;
	object *ret, *contents;
	string nodename, attrpair, attrname, attrvalue;

	if(sscanf(name, "%s[@%s]", nodename, attrpair) == 2) {
		/* Attribute matching */
		sscanf(attrpair, "%s=%s", attrname, attrvalue) == 2;
		SYSLOG("match attribute: " + attrname + " = " + attrvalue + "\n");
	}
/*	else if(sscanf(name, "%s(%s)%s", fun, arg, rest) > 1) {
		/ * Evaluate expression * /
		SYSLOG("getXpathNodes() expression: TODO!\n");
	}
*/
	else {
		/* Plain name matching */
		nodename = name;
		attrpair = nil;
	}

	ret = ({ });
	contents = this_object()->getContents();
	for(i = 0; i < sizeof(contents); i++) {
		if((nodename == "*" || contents[i]->getName() == nodename) &&
				(attrpair == nil  ||
				 contents[i]->matchAttribute(attrname, attrvalue) == TRUE)) {
			SYSLOG("found "+nodename+" ("+object_name(contents[i])+")\n");
			ret += ({ contents[i] });
		}
	}

	SYSLOG("getXpathNodes returning " + sizeof(ret) + " items\n");
	return ret;
}

