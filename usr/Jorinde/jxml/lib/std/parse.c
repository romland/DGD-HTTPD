/**
 * Description: XML 1.0 compliant (a few exceptions) parser/processor.
 * Version:     $Id$
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
private mapping realnames;

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


/*
 * <!DOCTYPE feature [<!ENTITY ext1 SYSTEM "./feature.xml">]>
 */
private string process_entity(string entity)
{
	string path;

	if(sscanf(entity, "%*sENTITY%*sSYSTEM%*s\"%s\"%*s", path) == 5) {
		return this_object()->xml_readfile(path);
	}
	
	return "<!-- We do not understand this entity (yet?) -->";
}


/**
 * Added hack (hack because of the name of the function) to support 
 * external entities. It's up to the Root object to implement security
 * for reading files.
 */
private string remove_prolog(string src)
{
	int start, close, dp;
	string entity;

	start = index_of(0, src, "<");
	if(src[start..start+2] == "<?x" || src[start..start+2] == "<?X") {
		close = index_of(start, src, "?>");
		src = src[close+2..];
	}

	/* 
	 * TODO: This is dangerous, this can cause an infinite loop
	 * since it will parse included entities when they've beem
	 * read in. Need some kind of safety mechanism to detect this.
	 */
	while((start = index_of(0, src, "<!DOCTYPE")) != -1) {
		close = index_of(start, src, ">") + 1;
		dp = index_of(start, src, "[");

		if(dp != -1 && dp < close) {
			close = index_of(start, src, "]>") + 2;
		} else {
			error("missing [ after DOCTYPE");
		}

		/* strip tag and replace it with content of file */
		entity = process_entity(src[start..close]);
		src = src[0..start-1] + entity + src[close..];
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

		/*
		 * Ignore escaped quotes within attribute. 
		 * This is not according to RFC, I think :P
		 */
		tmp = idstart;
		while( (nextid = index_of((tmp+1), str, look_for) ) && nextid != -1 
				&& str[nextid-1] == '\\') {
			tmp = nextid;
		}

		/* TODO: If nextid = -1, missing quote? Nice warning? */
		val = str[idstart+1..nextid-1];
		val = replace(val, "\\", "");
		
		name = strip_whitechars( str[0..equal_sign-1] );

		/*
		 * Is there a namespace used in this attribute and not a declaration
		 * of an alias for a namespace? If yes, remove it, we have no support
		 * yet.
		 */
		if((tmp = index_of(0, name, ":")) != -1 && !sscanf(name,"xmlns:%*s")) {
			/*
			 * TODO: Attributes should really have support for namespaces
			 * as well as the tags. 
			 *
			 * But, for now, we just disregard them. This at least until
			 * attributes have their own objects; or we come up with some
			 * other way to retrieve the namespaces of an attribute.
			 */
			name = name[tmp+1..];
		}

		all[name] = entity(val);
		str = str[nextid + 1..];
	}

	/* No attributes */
	return all;
}


/**
 * TODO: This parser will act bad if the following occurs:
 *
 *	<a:foo xmls:a="jorinde.datatypes" xmlns:b="jorinde.actions">
 *		<b:bar xmlns:a="jorinde.somethingelse">
 *			<a:blib/>
 *		</b:bar>
 *	</a:foo>
 *	
 *	The alias for a will be 'jorinde.somethingelse' when it wants to close
 *	the <a:foo> tag. This could be solved by always passing local aliases
 *	to the parsing function. But I am lazy right now.
 *
 *  Note: There is also no support for getting the namespace of a single
 *  attribute. We need to create an attribute object for this to work, and
 *  eh. Not just now.
 */
private void setRealName(string alias, string name)
{
	realnames[alias] = name;
}


private void setRealNames(mapping map)
{
	string *keys, alias;
	int i;

	keys = map_indices(map);
	for(; i < sizeof(keys); i++) {
		if(sscanf(keys[i], "xmlns:%s", alias) == 1) {
			/*
			 * Yes, this is -bad form-, we add the alias even it might 
			 * already be set. Set comment at setRealName().
			 */
			setRealName(alias, map[keys[i]]);
		}
	}
}


private string getRealName(string alias)
{
	string rname;
	
	rname = realnames[alias];

	return (rname ? rname : alias);
}


/**
 * ...
 */
private object tag_element(object fragment)
{
	/* init temporary variables */
	int close, empty, nextspace, current;
	string starttag, attribs, name, namespacealias;
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

	namespacealias = strip_whitechars(name);/* This is namespace -and- name */
	if(sscanf(namespacealias, "%s:%s", namespacealias, name) == 2) {
		string namespace;
		namespace = getRealName(namespacealias);
		fragment->getFraggleByIndex(current)->setNamespace(namespace);
	} else {
		name = namespacealias;
		namespacealias = nil;
	}

	fragment->getFraggleByIndex(current)->setName(name);

	if(strlen(attribs) > 0) {
		mapping avps;

		avps = get_attributes(attribs);

		/*
		 * The following function call will grab all 'xmlns' aliases that
		 * are created and store them in the alias-translation mapping.
		 * I guess this could be optimized to actually parse the 'xmlns'
		 * at the same time as we parse the attributes. 
		 */
		setRealNames(avps);

		fragment->getFraggleByIndex(current)->setAttributes( avps );
	}

	if(!empty) {
		/* get contents of tag, parse these (recursive) */
		contents = create_fragment();
		contents->setString(fragmentstr[close+1..]);
		/*contents->setEnd(name);*/
		if(namespacealias) {
			contents->setEnd(namespacealias + ":" + name);
		} else {
			contents->setEnd(name);
		}
		contents = process(contents);
		fragment->getFraggleByIndex(current)
				->setContents( contents->getFraggles() );
		fragment->setString( contents->getString() );
		/* Note that fragmentstr SHOULD change here, but doesn't (because it
		 * isn't used more in this function. Just for future reference.
		 */
	} else {
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

						/* lame error-reporting, I know (TODO: HACK) */
						if(strlen(fragmentstr) < tmp) { 
							error("XML: " + this_object()->query_filename() +
									": missing close-tag"); 
						}
						
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
 * Only non-private function in this file. Called by child.
 */
static int parse(string src)
{
	object fragment, root;

	if(src == nil || src == "") {
		return 0;
		/*error("XML source is nil");*/
	}

	INFO("xml->parse()...");

	realnames = ([ ]);
	
	src = strip_crlf(src);
	src = remove_prolog(src);
	
	fragment = create_fragment();
	fragment->setString(src);

	/* Add ROOT (this ob) to index. Always the first entry. */
	element_index = ({ this_object() });
	element_count = 1;

	fragment = process(fragment);

	/* Pass the index of the DOM-tree to super */
	this_object()->setContents( fragment->getFraggles() );
	this_object()->setIndex( element_index );
	this_object()->setGlobalNamespaces(map_values(realnames));

	/* We don't need the information gathered by this class any more */
	reset_vars();
	return 1;
}

