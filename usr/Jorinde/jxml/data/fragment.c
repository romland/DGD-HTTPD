/**
 * Description: Used for temporary storage by the XML-processor.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
#include "../include/xml.h"

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

inherit "../lib/std/tag";
inherit "../lib/log";

private string str;
private string end;
private object *fraggles;		/* I -like- the name fraggles! */


void constructor()
{
	str = "";
	end = "";
	fraggles = ({ });
}


int getType()
{
	return  XML_FRAGMENT_NODE;
}

void setType()
{
	error("setting type is not allowed");
}


string getString() {
	return str;
}
void setString(string arg) { 
	str = arg;
}

string getEnd() {
	return end;
}

void setEnd(string a) {
	end = a;
}

int getFragglesSize() {
	return sizeof(fraggles);
}


void setFraggle(int index, object fraggle) {
	int i;

	if(sizeof(fraggles) <= index) {
		int sof;
		sof = sizeof(fraggles);
		for(i = sof; i <= index; i++) {
			fraggles += ({ nil });
		}
	}
	fraggles[index] = fraggle;
}

object *getFraggles() {
	return fraggles + ({ });
}


object getFraggleByIndex(int index) {
	return fraggles[index];
}

