#if 0
# include "../include/www.h"
# include "../include/xmlutil.h"
# include "../include/object.h"
#endif
# include "../include/shared.h"

/**
 * Will call xml_parameter in 'ob' with arguments string key, mixed value.
 * The node should look look something like this (the child-node parameter
 * is the relevant thing):
 * 
 *	<handler>
 *		<parameter key="lsp_home" value="/usr/www/lsp/" />
 *	</handler>
 * 
 * Returns 1 if all parameters was understood by ob->xml_parameter().
 *
 */
static int forward_xml_parameters(object ob, object node)
{
	int ret, known, i;
	object *params;
	
	if(node == nil || node->dummy()) {
		return 1;
	}

	known = 1;
	params = node->xpath("parameter");
	for(i = 0; i < sizeof(params); i++) {
		if(params[i]->dummy()) continue;
		ret = ob->xml_parameter(
				params[i]->getAttribute("key"),
				params[i]->getAttribute("value")
			);
		if(!ret) {
			SYSLOG("WARNING: " + object_name(ob) + "->" +
				" : xml_parameter(" +
					params[i]->getAttribute("key") + ", " +
					params[i]->getAttribute("value") +
				") returned 0 or nil!\n");
			known = 0;
		}
	}
	return known;
}

/**
 * Return object 'program' which should be a child-node of 'node'.
 */
static object get_program(object node)
{
	string filename;
	if(node == nil || node->dummy()) {
		return nil;
	}

	filename = node->xpath("program")[0]->getCleanValue();
	if(filename == nil) {
		return nil;
	}

	COMPILE(filename);
	return find_object(filename);
}

