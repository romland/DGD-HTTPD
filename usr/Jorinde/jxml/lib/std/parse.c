/**
 * Description: XML 1.0 compliant (a few exceptions) parser/processor.
 * Version:     $Id: parse.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
/*
#define	LOG_LEVEL_DEBUG
#define	LOG_LEVEL_INFO
*/
#define	LOG_LEVEL_WARN
#define	LOG_LEVEL_ERROR

#include "../../include/log.h"
#include "../../include/xml.h"

inherit "../log";
inherit "../string";
inherit "../file";

/* Keep track of elements' unique identities. This will become ROOT index. */
private int    element_count;
private object *element_index;

private mixed process(object fragment);

private object new_lwo(string name)
{
	return new_object(XML_LWO_DIR + name);
}

/**
 *  Only used for internal purposes, the top fragment will really contain
 *  the entire DOM structure when we're done -- but only then.
 */
private object create_fragment()
{
	object ob;
	
	ob = new_lwo("fragment");
	ob->constructor();
	return ob;
}

/**
 *
 */
private object create_element()
{
	object ob;

	ob = new_lwo("element");
	ob->constructor( element_count );
	element_index += ({ nil });
	element_index[element_count] = ob;

	element_count++;
	return ob;
}


/**
 *
 */
private object create_characterdata()
{
	object ob;

	ob = new_lwo("characterdata");
	ob->constructor();
	return ob;
}


/**
 * CDATA != character data. 
 */
private object create_cdata()
{
	object ob;

	ob = new_lwo("cdata");
	ob->constructor();
	return ob;
}


/**
 * 
 */
private object create_pi()
{
	object ob;

	ob = new_lwo("pi");
	ob->constructor();
	return ob;
}


/**
 * 
 */
private object create_comment()
{
	object ob;

	ob = new_lwo("comment");
	ob->constructor();
	return ob;
}


/**
 * 
 */
private string remove_prolog(string src)
{
	int start, close, dp;

#if 0
	/* this check was added on 07-may-2004 */
	if(src == "") {
		return src;
	}
#endif

	start = index_of(0, src, "<");
	if(src[start..start+2] == "<?x" || src[start..start+2] == "<?X") {
		close = index_of(start, src, "?>");
		src = src[close+2..];
	}

	start = index_of(0, src, "<!DOCTYPE");
	if(start != -1) {
		close = index_of(start, src, ">") + 1;
		dp = index_of(start, src, "[");
		if(dp != -1 && dp < close) {
			close = index_of(start, src, "]>") + 2;
		}
		src = src[close..];
	}
	return src;
}


/**
 * 
 */
private mapping get_attributes(string str)
{
	mapping all;
	int equal_sign, id1, id2, idstart, nextid, tmp;
	string look_for, val, name;

	all = ([ ]);
	while(1) {
		equal_sign = index_of(0, str, "=");
		if(strlen(str) == 0 || equal_sign == -1) {
			return all;
		}

		id1 = index_of(0, str, "\'");
		id2 = index_of(0, str, "\"");

		if((id1 < id2 && id1 != -1) || id2 == -1) {
			idstart = (int)id1;
			look_for = "\'";
		}
		if((id2 < id1 || id1 == -1) && id2 != -1) {
			idstart = (int)id2;
			look_for = "\"";
		}

		/* ignore escaped quotes within attribute */
		tmp = idstart;
		while( (nextid = index_of((tmp+1), str, look_for) ) && nextid != -1 
				&& str[nextid-1] == '\\') {
			tmp = nextid;
		}

		/* TODO: If nextid = -1, missing quote? Nice warning? */
		val = str[idstart+1..nextid-1];
		val = replace(val, "\\", "");
		
		name = strip_whitechars( str[0..equal_sign-1] );
		all[name] = entity(val);
		str = str[nextid + 1..];
	}

	/* No attributes */
	return all;
}


/**
 * ...
 */
private object tag_element(object fragment)
{
	/* init temporary variables */
	int close, empty, nextspace, current;
	string starttag, attribs, name;
	string fragmentstr;
	object contents;

	fragmentstr = fragment->getString();
	close = index_of(0, fragmentstr, ">");
	if(close == -1) {
		WARN("parser.tag_element(): could not find: '>' in "
			 + fragmentstr[0..]);
	}
	empty = (fragmentstr[close-1..close-1] == "/");
	if(empty) {
		close--;
	}

	/* get attribute keys and values */
	starttag = normalize_whitechars(fragmentstr[1..close-1]);
	nextspace = index_of(0, starttag, " ");
	if(nextspace != -1) {
		name = starttag[0..nextspace-1];
		attribs = starttag[nextspace+1..];
	}
	else {
		name = starttag;
		attribs = "";
	}

	INFO("Found node " + name);
	
	current = fragment->getFragglesSize();
	fragment->setFraggle(current, create_element());
	fragment->getFraggleByIndex(current)->setName(strip_whitechars(name));

	if(strlen(attribs) > 0) {
		fragment->getFraggleByIndex(current)
				->setAttributes( get_attributes(attribs) );
	}

	if(!empty) {
		/* get contents of tag, parse these (recursive) */
		contents = create_fragment();
		contents->setString(fragmentstr[close+1..]);
		contents->setEnd(name);
		contents = process(contents);
		fragment->getFraggleByIndex(current)
				->setContents( contents->getFraggles() );
		fragment->setString( contents->getString() );
		/* Note that fragmentstr SHOULD change here, but doesn't (because it
		 * isn't used more in this function. Just for future reference.
		 */
	}
	else {
		/* we have nested tags */
		fragment->setString( fragmentstr[close+2..] );
	}
	return fragment;
}


/**
 * 
 */
private object tag_pi(object fragment)
{
	int close, current;
	string val;

	close = index_of(0, fragment->getString(), "?>");
	val = fragment->getString()[2..close-1];
	current = fragment->getFragglesSize();

	fragment->setFraggle( current, create_pi() );
	fragment->getFraggleByIndex(current)->setValue( val );
	fragment->setString(fragment->getString()[close+2..]);

	return fragment;
}


/**
 * 
 */
private object tag_comment(object fragment)
{
	int close, current;
	string val;

	close = index_of(0, fragment->getString(), "-->");
	val = fragment->getString()[4..close-1];
	current = fragment->getFragglesSize();

	fragment->setFraggle( current, create_comment() );
	fragment->getFraggleByIndex(current)->setValue( val );
	fragment->setString(fragment->getString()[close+3..]);

	return fragment;
}

private object characterdata(object fragment)
{
	ERROR("TODO");
}


/**
 * 
 */
private object tag_cdata(object fragment)
{
	int close, current;
	string val;

	INFO("tag_cdata()");

	close = index_of(0, fragment->getString(), "]]>");
	val = fragment->getString()[9..close];
	current = fragment->getSize();

	fragment->setFraggle(current, create_cdata());
	fragment->getFraggleByIndex(current)->setValue( val );
	fragment->setString(fragment->getString()[close+3..]);

	return fragment;
}


/**
 * 
 */
private mixed process(object fragment)
{
	while(1) {
		int tagstart;
		string fragmentstr;

		fragmentstr = fragment->getString();
/*		INFO("bytes left: " + strlen(fragmentstr) );
*/
		/* When fragment-string is empty, return fragment */
		if(strlen(fragmentstr) == 0) {
			return fragment;
		}

		tagstart = index_of(0, fragmentstr, "<");
		if(tagstart != 0) {
			/* We found some characters, grab them. */
			int current;
			current = fragment->getFragglesSize();
			fragment->setFraggle(current, create_characterdata());
			if(tagstart == -1) {
				fragment->getFraggleByIndex(current)->setValue( 
								entity(fragmentstr) 
							);
				fragmentstr = "";
				fragment->setString(fragmentstr);
			}
			else {
				fragment->getFraggleByIndex(current)->setValue(
									entity(fragmentstr[0..tagstart-1])
								);
				fragment->setString(fragmentstr[tagstart..]);
			}
		}
		else {
			/* deal with and get type of upcoming section */
			if(fragmentstr[1..1] == "?") {
				/* Processing Instructions (PIs) */
				fragment = tag_pi(fragment);
			}
			else {
				if(fragmentstr[1..3] == "!--") {
					/* comments */
					fragment = tag_comment(fragment);
				}
				else {
					if(fragmentstr[1..7] == "!CDATA[") {
						/* character data */
						INFO("cdata!");
						fragment = tag_cdata(fragment);
					}
					else {
						int tmp;
						string endstr;

						endstr = fragment->getEnd();
						tmp = strlen(endstr) + 3 - 1;
						if(fragmentstr[1..tmp] == ("/" + endstr + ">") ||
						   strip_whitechars(fragmentstr[1..tmp]) == 
						   ("/" + endstr)) {
							fragment->setString(fragmentstr[tmp+1..]);
							fragment->setEnd("");
							return fragment;
						}
						else {
							fragment = tag_element(fragment);
						}
					}
				}
			}
		}
	}

	/* Nothing to process? */
	ERROR("process(): Should not get here [insert good error msg].");
	return "";
}


/**
 * 
 */
private void reset_vars()
{
	element_count = 0;
	element_index = ({ });
}


/**
 * This is the entry point, and also the only non-private function in this
 * object, have fun. :-)
 */
static int parse(string src)
{
	object fragment, root;

	if(src == nil || src == "") {
		return 0;
		/*error("XML source is nil");*/
	}
	
	INFO("xml->parse()...");
	src = strip_crlf(src);
	src = remove_prolog(src);

	fragment = create_fragment();
	fragment->setString(src);

	/* Add ROOT (this ob) to index. Always the first entry. */
	element_index = ({ this_object() });
	element_count = 1;

	fragment = process(fragment);

	/* Pass the index of the DOM-tree 'up' to parent class */
	this_object()->setContents( fragment->getFraggles() );
	this_object()->setIndex( element_index );

	/* We don't need the information gathered by this class any more */
	reset_vars();
	return 1;
}

