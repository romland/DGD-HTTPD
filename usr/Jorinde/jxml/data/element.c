/**
 * Description: Implementation of an XML-element.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# define LOG_LEVEL_DEBUG
# define LOG_LEVEL_INFO
# define LOG_LEVEL_WARN
# define LOG_LEVEL_ERROR

# include <type.h>
# include "../include/log.h"
# include "../include/xml.h"

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

inherit tag "../lib/std/tag";
inherit att "../lib/std/attributes";
inherit ser "../lib/std/serialize";
inherit dom "../lib/std/dom";

private object	*contents;
private int		_type;

void constructor(int uid)
{
	tag::constructor();
	att::constructor();
	ser::constructor();
	dom::constructor();
	
	tag::setUid(uid);
	_type = XML_ELEMENT_NODE;

	contents = ({ });
}

int is_empty()
{
	if( sizeof(contents) == 0 || (sizeof(contents) == 1 &&
	   (contents[0]->getType() == XML_CDATA_NODE) ) ) {
		return 1;
	}
	return 0;
}

int getType()
{
	return  _type;
}

void setType(int type)
{
	WARN("Setting type is a bad thing to do.");
	_type = type;
}

int getContentSize()
{
	return sizeof(contents);
}

void setContents(object *i)
{
	contents = i;
}

object *getContents()
{
	if(!contents) {
		return ({ });
	}
	return contents + ({ });
}

void addContents(mixed o)
{
	if(typeof(o) == T_ARRAY) {
		contents += o;
	} else if(typeof(o) == T_OBJECT) {
		contents += ({ o });
	} else {
		error("not object or *object");
	}
}

