/* TODO: Rename this file so that it's not confused with the shared lib.
 * TODO: Add (c) header
 */

# include "../include/dav.h"
#if 0
inherit "../../lib/xmlutil";
#endif

object new_xmldoc()
{
	return (JXMLROOTD)->new();
}
                                                                                
object new_node(string name, varargs string xmlns)
{
	object o;

	o = new_object(NODE);
	o->constructor(-1);
	o->setName(name);

	if(!xmlns) {
		xmlns = "DAV:";
	}

	o->setAttribute("xmlns", xmlns);
	return o;
}
                                                                                
object new_data(mixed val)
{
	object o;
	o = new_object(DATA);
	o->constructor();
	o->setValue(val);
	return o;
}

