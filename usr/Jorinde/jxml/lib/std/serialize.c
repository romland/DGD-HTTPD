/**
 * Description: Class for serializing an XML-node to XML-string.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# define LOG_LEVEL_DEBUG
# define LOG_LEVEL_INFO
# define LOG_LEVEL_WARN
# define LOG_LEVEL_ERROR

# include "../../include/log.h"
# include "../../include/xml.h"

inherit "../log";

/*# define INDENT
*/
#ifdef INDENT
#define TABSIZE 4
/* TODO: We will crash on high indent-levels */
#define TABS "                                                                                                                                               "
#endif

void constructor() { }

/*
 * XML serialization.
 *
 * TODO: Clean up all this serialization shit. We do want that part 
 * configurable. Perhaps we should split it up in two functions even
 * though it would mean redundancy in code?
 *
 * TODO: On large trees DGD's max-string-size will be exceeded and 
 * throw an error, this should be fixed.
 */
string
xml(mixed args...)
{
	object *contents;
	int i, closed;
	string str, tag;
	mapping map;
#ifdef INDENT
	int indent;
#endif

	if(this_object()->isRoot()) {
		/* TODO: processing instructions */
		str = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
#ifdef INDENT
		str += "\n";
#endif
	} else {
		str = "";
	}
#ifdef INDENT
	if(!(sizeof(args)) || !(indent = args[0])) {
		indent = 0;
	}
#endif
	contents = this_object()->getContents();
	for(i = 0; i < sizeof(contents); i++) {
		object e;

		tag = "";
		closed = 0;
		e = contents[i];
		
		if(e == nil || !(e->is_dom_element())) {
			continue;
		}

		switch(e->getType()) {
		case XML_ELEMENT_NODE :
#ifdef INDENT
			tag += TABS[0..(indent*TABSIZE)];
#endif
			tag += "<" + e->getName();
			if(e->has_attributes()) {
				tag += " " + e->attributesToString();
			}

			if(e->is_empty()) {
				tag += "/";
				closed = 1;
			}
			tag += ">";
#ifdef INDENT
			if((sizeof(e->getContents()) > 0 && e->getContents()[0]->getType() != XML_CHARACTER_DATA) || e->is_empty()) {
				tag += "\n";
			}
#endif
			break;

		case XML_CHARACTER_DATA :
			if(e->getValue()) {
				tag += e->getValue();
			}
#ifdef INDENT
			tag += "\n";
#endif
			break;

		case XML_CDATA_NODE :
			WARN("TODO!");
			break;

		case XML_COMMENT_NODE :
#ifdef INDENT
			tag += TABS[0..(indent*TABSIZE)];
#endif
			tag += "<!--" + e->getValue() + "-->";
			break;

		default :
			tag += "[unknown element: " + e->getType() + "]";
			WARN("unknown element: " + e->getType() );
			break;
		}
		str += tag;
		/*
		 * Hmm, will we miss anything if we only call down the tree
		 * in 'element nodes'? Don't think so? Or should all element-
		 * types inherit code to serialize to XML?
		 */
		if(e->getType() == XML_ELEMENT_NODE) {
#ifdef INDENT
			str += e->xml(indent + 1);
#else
			str += e->xml(1);
#endif
			if(!closed) {
#ifdef INDENT
				str += TABS[0..(indent*TABSIZE)];
#endif
				str += "</" + e->getName() + ">\n";
			}
		}
	}
	return str;
}

