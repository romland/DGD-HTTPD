/**
 * Description: Implements a base used by all XML-tags.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# define  LOG_LEVEL_DEBUG
# include "../../include/log.h"
# include "../../include/xml.h"

inherit "../file";
inherit "../log";
inherit "../string";

private mixed _value;
private int _uid;
private string _name, _typename;

/* function declarations */
int getUid();

nomask int is_dom_element()
{
	return 1;
}

nomask int is_implemented(string name)
{
	/*
	 * TODO: Check whether method 'name' is implemented in the object
	 * that inherit us.
	 */
	return 1;
}

/**
 * Note: We will not allow elements to implement create(). Function 
 * 		 constructor() should always be called right after 'cloning'.
 * 		 I changed my mind, I want the ROOT node to define create().
 */
void create(varargs int clone)
{
	if(is_implemented("getType") &&  is_implemented("setType")) {
		/* Everything's cool */
	}
	else {
		error("lib/tag: one or more functions are not implemented.");
	}
	_uid = -1;
	_value = "";
}


void constructor()
{
	_typename = "";
	_value = nil;
}

mixed getValue()
{
	if(_value == nil) {
		object *contents;
		/* be nice and get [TODO: the?] character-data of the node */
		contents = this_object()->getContents();
		if(contents && sizeof(contents) == 1 &&
				contents[0]->getType() == XML_CHARACTER_DATA) {
			return contents[0]->getValue();
		} else {
			return nil;
		}
	}
	return _value;
}

/**
 * TODO: Do we really want to go here? Keep or not?
 */
mixed getCleanValue()
{
	mixed val;
	int i;
	val = getValue();
	if(val == nil) return val;
	val = trim(val);

	for(i = 0; i < strlen(val); i++)
		if(val[i] < '0' || val[i] > '9')
			return val;
	return (int)val;
}

void setValue(mixed v)
{
	_value = v;
}


string getTypeName()
{
	if(_typename == "") {
		_typename = _filename(object_name(this_object()));
	}
	return _typename;
}


void setTypeName(string s)
{
	_typename = s;
}

void setName(string s)
{
	int i;
	if((i = index_of(0, s, ":")) != -1) {
		/* TODO: We need to implement support for XML namespaces. It's just
		 * hacked together to get it to work with WebDAV now. If a name 
		 * contains : we split the string and set first part as xmlns in an
		 * attribute in the element and second part as name.
		 */
#if 0
		SYSLOG("attr: " + s[0..i] + " - s: " + s[i+1..] + "\n");
#endif
		this_object()->setAttribute("xmlns", s[0..i]);
		s = s[i+1..];
	}
	_name = s;
}

string getName()
{
	return _name;
}

int getUid()
{
	return _uid;
}

void setUid(int a)
{
	if(_uid != -1) {
		error("tag: UID already set. Mask setUid() to enable resetting.");
	}
	_uid = a;
}

