/**
 * Description: Very minimal DOM implementation (all I need up until now).
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

inherit xpath "./xpath";

/*
 * TODO: Should check if node to be inserted appear elsewhere in doc;
 *		 if yes, error
 */

void constructor()
{
	/* ... */
	xpath::constructor();
}

/*
 * DOM manipulation, but this is not all, some DOM manipulation is available
 * in attributes.c which should be inherited by all nodes as well.
 */
/*
 * Do we want to be able to create an empty DOM and then load XML into it?
 * As it is now, you call the processor which will return a DOM for you,
 * either works for me, really. Not sure what the DOM-spec says on this
 * topic.
 */
object getByName(string name)
{
	error("deprecated\n");
}

object getById(int id)
{
	error("TODO");
}

int delete()
{
	error("TODO");
}

int insert(mixed node)
{
	this_object()->addContents(node);
	return TRUE;
}

int insertBefore(object node)
{
	error("TODO");
}

int insertAfter(object node)
{
	error("TODO");
}

/*
 * This is a bit of a vague function since it essentially deal with
 * a child of this node. Perhaps we shouldn't allow this? What the
 * idea is however: Append a node to the end of a childnodes content.
 * 
 * It's essentially the same as appendNode, but to one of this nodes
 * children.
 * TODO: Consider: Do we _really_ want this one? I say no right now.
 */
int insertNodeInto(object node)
{
	error("TODO");
}

