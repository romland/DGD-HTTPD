/**
 * Description: Inherited by XML Root objects.
 * Version:     $Id: root.c 23 2004-09-07 21:11:23Z trickster $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# define LOG_LEVEL_DEBUG
# define LOG_LEVEL_INFO
# define LOG_LEVEL_WARN
# define LOG_LEVEL_ERROR

# include <type.h>
# include "../../include/log.h"
# include "../../include/xml.h"

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

inherit tag "./tag";
inherit ser "./serialize";
inherit par "./parse";
inherit dom "./dom";

private object	*index;
private object	*contents;
private int		_is_root;
private int		_type;
private string	*_namespaces;


void constructor()
{
	tag::constructor();
	ser::constructor();
	dom::constructor();

	_namespaces = ({ });
	_type = XML_ROOT_NODE;
	setName("ROOT");
	_is_root = 1;
	
	index = ({ });
	contents = ({ });
}

void create(varargs int clone)
{
	constructor();
}

int getType()
{
	return  _type;
}

void setType(int type)
{
	_type = type;
}

object *getContents()
{
	return contents;
}

void setContents(object *o)
{
	contents = o;
	propagateNamespace( ({ o }) );
}

void addContents(mixed o)
{
	if(typeof(o) == T_ARRAY) {
		error("root allowed one node only\n");
	} else if(typeof(o) == T_OBJECT) {
		contents += ({ o });
		propagateNamespace( ({ o }) );
	} else {
		error("not an object");
	}
}

void setIndex(object *i)
{
	index = i;
}

object *getIndex()
{
	return index;
}

int isRoot()
{
	return _is_root;
}

void setIsRoot(int a)
{
	_is_root = a;
}

/**
 * New behaviour introduced: This function is now static.
 * Basically the same as loadXML, except that you send the source of the
 * XML document as a string.
 */
static int parseXML(string source)
{
	return parse(source);
}

#if 0
/**
 * New behaviour introduced: This function is now disabled and should be in 
 * parent class.
 */
static int loadXML(string filename)
{
	/* TODO: check if file exists */

	if(strlen(filename) > 4 && filename[strlen(filename)-4..] != ".xml") {
		ERROR("File does not have a .xml extension.");
	}

	parseXML( read_file(filename) );
	return 1;
}
#endif

object iterator()
{
	error("xmlroot does not implement an iterator, it only has one child");
}

/* 
 * This is merely for speed, it makes it easier to retrieve all namespaces
 * that are used by the document. These are set by the parser after the
 * entire document is parsed. (Eg, these functions are of no use if the
 * document was 'constructed' using insert in the DOM.
 */
void setGlobalNamespaces(string *ns)
{
	_namespaces = ns;
}

string* getGlobalNamespaces()
{
	return _namespaces;
}

/**
 * Overloaded: real functionality is in serialize.c
 */
#if 0
string xml(mixed args...)
{
	int i;

	if(sizeof(_namespaces) > 25) {
		error("No support for more than 25 namespaces yet");
	}

	for(i = 0; i < sizeof(_namespaces); i++) {
	}

	return ::xml(args...);
}
#endif
