/**
 * Description: XML Root object.
 * Version:     $Id: root.c 226 2004-06-20 21:58:43Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# define LOG_LEVEL_DEBUG
# define LOG_LEVEL_INFO
# define LOG_LEVEL_WARN
# define LOG_LEVEL_ERROR
# include "../include/log.h"
# include "../include/xml.h"
# include <type.h>

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

inherit tag "../lib/std/tag";
inherit att "../lib/std/attributes";
inherit ser "../lib/std/serialize";
inherit par "../lib/std/parse";
inherit dom "../lib/std/dom";

private object	*index;
private object	*contents;
private int		_is_root;
private int		_type;


void constructor()
{
	tag::constructor();
	att::constructor();
	ser::constructor();
	dom::constructor();

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
}

void addContents(mixed o)
{
	if(typeof(o) == T_ARRAY) {
		error("root allowed one node only\n");
	} else if(typeof(o) == T_OBJECT) {
		contents += ({ o });
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
 * Basically the same as loadXML, except that you send the source of the
 * XML document as a string.
 */
int parseXML(string source)
{
	return parse(source);
}

/**
 *
 */
int loadXML(string filename)
{
	/* TODO: check if file exists */

	if(strlen(filename) > 4 && filename[strlen(filename)-4..] != ".xml") {
		ERROR("File does not have a .xml extension.");
	}

	parseXML( read_file(filename) );
	return 1;
}

