/**
 * TODO: SECURITY! This will give anyone access to xml-root's, which is ...
 * um, okay?
 */
# include "../include/xml.h"

object new()
{
	return new_object( JORINDE_XML + "data/root" );
}
