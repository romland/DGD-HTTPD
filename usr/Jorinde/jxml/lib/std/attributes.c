/**
 * Description: Attribute-collection for XML-elements.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../../include/xml.h"

private mapping attributes;


int has_attributes()
{
	return map_sizeof(attributes) ? 1 : 0;
}


void constructor()
{
	attributes = ([ ]);
}

int matchAttribute(string name, mixed value)
{
	int i;
	string *arr;

	if(!map_sizeof(attributes))
		return 0;
	
	arr = map_indices(attributes);
	for(i = 0; i < sizeof(arr); i++) {
		if(value == attributes[arr[i]])
			return 1;
	}
	return 0;
}

mixed getAttribute(string s)
{
	return attributes[s];
}


void setAttribute(string name, mixed value)
{
	attributes[name] = value;
}


void setAttributes(mapping map)
{
	attributes = map;
}


mapping getAttributes()
{
	return ([ ]) + attributes;
}


string attributesToString()
{
	string str;
	str = "";

	if(map_sizeof(attributes)) {
		mixed *keys;
		int j;
		keys = map_indices(attributes);
		for(j = 0; j < sizeof(keys); j++) {
			if(str != "") str += " ";
			str += keys[j] + "=\"" + attributes[keys[j]] + "\"";
		}
	}
	return str;
}

