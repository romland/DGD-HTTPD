/**
 * Description: Interface for handling pending incoming request-content. It
 *              will act as a collection if the data is of type multipart.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include <kernel/kernel.h>
# include "../include/www.h"

inherit header  LIB_HTTP_HEADER;
inherit content LIB_HTTP_CONTENT;
inherit LIB_HTTP_STRING;
inherit LIB_HTTP_ENCDEC;

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static User			owner;
static Application	app;
static Request		request;
static Response		response;
static int			expected_length;

static int		started, complete;
/*static string	boundary, type;
*/
static mixed *ct;

int add_part(string str);

void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	header::create();
	content::create();
	started  = FALSE;
	complete = FALSE;
}

/* Content-Type */
# define CT_TYPE		0			/* such as 'image' or 'multipart' */
# define CT_SUBTYPE		1			/* such as 'gif' or 'form-data' */
# define CT_AVP_SIZE	2			/* # avp's */
# define CT_AVP			3			/* such as boundary or charset */
# define CT_STRUCT_SIZE	4

/**
 * TODO: Move to another class -- we will want to use it elsewhere
 */
mixed *parse_content_type(string str)
{
	mixed	*ret;
	int		count;
	string	type, subtype, attribs, tmp;
	mapping	avp;

	avp = ([ ]);
	
	if(!str || str == "") {
		type = "application";
		subtype = "octet-stream";
	} else {
		str = trim(str);
		count = sscanf(str, "%s/%s;%s", type, subtype, attribs);
		if(count != 3) {
			count = sscanf(str, "%s/%s", type, subtype);
			if(count != 2) {
				error("Not valid. TODO: Caller should flag invalid request");
			}
		}

		if(count == 3) {
			avp = parse_avp(attribs, ";", "=");
		}
	}
	
	ret = allocate(CT_STRUCT_SIZE);
	ret[CT_TYPE] =		type;
	ret[CT_SUBTYPE] =	subtype;
	ret[CT_AVP_SIZE] =	map_sizeof(avp);
	ret[CT_AVP]	=		avp;

	return ret;
}

/* Content-Disposition */
# define CD_TYPE		0			/* such as 'form-data' */
# define CD_AVP_SIZE	1
# define CD_AVP			2			/* such as 'name' or 'filename' */
# define CD_STRUCT_SIZE	3

mixed *parse_content_disposition(string str)
{
	mixed   *ret;
	int     count;
	string  type, tmp, attribs;
	mapping avp;

	if(!str || str == "") {
		type    = "";
	} else {
		str = trim(str);
		count = sscanf(str, "%s;%s", type, attribs);
		/* TODO: Check count */
	}

	if(attribs) {
		avp = parse_avp(attribs, ";", "=");
	}
	
	ret = allocate(CD_STRUCT_SIZE);
	ret[CD_TYPE] = type;
	ret[CD_AVP_SIZE] = map_sizeof(avp);
	ret[CD_AVP] = avp;
	return ret;
}

/* Content-Range */
/* Content-Encoding */
/* Content-Language */
/* Content-Length */
/* Content-Location */
/* Content-MD5 */


int is_multipart()
{
	if(ct[CT_TYPE] == "multipart" && ct[CT_AVP]["boundary"]) {
		return TRUE;
	}
	return FALSE;
}

int is_form_urlencoded()
{
	return (ct[CT_SUBTYPE] == "x-www-form-urlencoded");
}

int initialize(string str)
{
	string tmp, headers, content;
	int count, fucking_content_type_in_body;

#if 0
	SYSLOG("Starting content receive.\n");
#endif

	count = sscanf(str, "%s" + CRLF + CRLF + "%s", headers, content);
	if(count < 2) {
		headers = "";
		content = str;
	} else {
		headers = headers;
	}

	if(!(tmp = request->get_header("Content-Type"))) {
		count = sscanf(CRLF+headers+CRLF, "%*sContent-Type: %s" + CRLF, tmp);
		if(count != 2) {
			SYSLOG("WARNING: This COULD be a bug! " +
					"Content-type not found; using default\n");
			tmp = "";
#if 0
			return FALSE;
#endif
		} else {
			SYSLOG("Content-Type in body!\n");
			fucking_content_type_in_body = TRUE;
		}
	}
	ct = parse_content_type(tmp);

	if(!(tmp = request->get_header("Content-Length"))) {
		count = sscanf(CRLF+headers+CRLF, "%*sContent-Length: %s" + CRLF, tmp);
		SYSLOG("count: " + count + ", tmp: " + tmp + "\n");
		if(count != 2) {
			SYSLOG("ERROR: content-length not found\n");
			SYSLOG("Headers: " + headers + "\n");
			return FALSE;
		}
	}
	SYSLOG("Found Content-Length:: " + tmp + "\n");
	expected_length = (int)tmp;

	if(is_multipart()) {
#if 0
		SYSLOG("8 first bytes of headers: --|" + headers[0..7] + "|---\n");
		SYSLOG("8 first bytes of content: --|" + content[0..7] + "|---\n");
		SYSLOG("8 first bytes of str:     --|" + str[0..7] + "|---\n");
#endif
		/* In the case where we got type from body, include CRLF */
		add_part(fucking_content_type_in_body ? (CRLF + content) : (CRLF + str) );
		/* In the case where we got it from header: * /
TODO: Make this consistent, what is calling this?
		add_part(str);
*/
		started = TRUE;
	} else if(is_form_urlencoded()) {
		/*SYSLOG("stripping up until crlf^2 (len: " + strlen(str) +")\n");*/
		sscanf(str, "%*s"+CRLF+CRLF+"%s", str);
		add_content(str);
		started = TRUE;
	} else {
#if 0
		SYSLOG("Adding _str_ (correct? not content?); add_content()...\n");
#endif
		add_content(str);
		started = TRUE;
	}

#ifdef DELETE_ME
	if(sscanf(str, 
			"Content-Type: %s; boundary=%s" + CRLF +
			"Content-Length: %d" + CRLF + CRLF + "%s", t, b, l, c) == 4) {
		type = t;
		boundary = b;
		expected_length = l;
		add_part(CRLF + c);
		started = TRUE;
	} else {
		string tmp;
		tmp = request->get_header("Content-Type");
		if(sscanf(tmp, "%s; boundary=%s", t, b) == 2) {
			type = t;
			boundary = b + "";
			started = TRUE;
			if(request->get_content_length()) {
				expected_length = (int)request->get_header("Content-Length");
				add_part(str);
			}
		} else {
			SYSLOG("WARNING: Could not obtain Content-Type from neither " +
					"request's content or its headers.\n");
			started = FALSE;
		}
	}
#endif
#if 0
	SYSLOG("Leaving data/content->initialize()\n");
#endif
	return started;
}


int set_part_headers(string str, Mime part)
{
	int i;
	string *lines, *line;

	lines = explode(str, CRLF);
	for(i = 0; i < sizeof(lines); i++) {
		if(lines[i] != "") {
			line = explode(lines[i], ": ");
			if(sizeof(line) > 1) {
				part->set_header(line[0], implode(line[1..], ": "));
			}
		}
	}
	return TRUE;
}

mapping get_parts_urlencoded()
{
	string *items, *item;
	int i;
	mapping parts;

	if(!is_form_urlencoded()) {
		error("not a form that is urlencoded");
	}

	parts = ([ ]);
	/*
	 * TODO: Must convert all of this file to use bstr, this WILL bug with
	 * data more than 64k.
	 */
	items = explode(content_tostring(), "&");
	for(i = 0; i < sizeof(items); i++) {
		Mime part;
		item = explode(items[i], "=");
		/* Note that the 'mime' is not REALLY made for this purpose,
		 * so we do not set any headers in it, just content.
		 */
		part = new_object(HTTP_MIME);
		part->set_name(item[0]);
		if(sizeof(item) > 1) {
			part->add_content( url_decode(item[1]) );
		}
		parts[item[0]] = part;
	}
	return parts;
}


/**
 * This function counts on add_part() to do its job. If 
 * it didn't, this function will misbehave badly. So look
 * in add_part() first.
 * 
 * NOTE: Common problem is that we didn't get two leading CRLF's. :-)
 */
mapping get_parts()
{
	string  b, c;
	Mime	part;
	mapping parts;
	int     i, blen, clen, chsize;

	if(!is_multipart()) {
		error("not a multipart");
	}
	
	parts = ([ ]);
	chsize = get_content_size();
	b = CRLF + "--" + ct[CT_AVP]["boundary"];
	blen = strlen(b);
	for(i = 0; i < chsize; i++) {
		c = get_content_chunk(i);
		clen = strlen(c);
#if 0
		SYSLOG("clen for " + i + " is: " + clen + "\n");
#endif
		if(clen > blen && c[0..blen-1] == b) {
			part = new_object(HTTP_MIME);
			set_part_headers(c, part);
			parts[part->get_name()] = part;
		} else if(part) {
			part->add_content( c );
		} else {
#if 0
			SYSLOG("CHUNK " + i + " |--start--|" + c + "|--end--|\n");
#endif
			error("No part set at chunk " + i + "!\n");
		}
		set_content_chunk(i, nil);
	}

#if 0
	if(1) {
		int p;
		string *names;
		names = map_indices(parts);
		for(i = 0; i < map_sizeof(parts); i++) {
			SYSLOG("Part " + i + " is " + (names[i]?names[i]:"nil") + ": " +
				parts[names[i]]->get_content_length() + " bytes.\n");
		}
	}
#endif

	return parts;
}

/*
 * Recursive function. Should stack-depth be a concern?
 */
int add_part(string str)
{
	int cpos, csize, glen;
	string prev, temp;

	if(!is_multipart()) {
		error("not a multipart\n");
	}
#if 0
	if(get_content_size() == 0) {
		SYSLOG("|--- start ---|" + str + "|--- end ---|\n");
	}
#endif

	if((csize = get_content_size()) > 0) {
		/* Length of [s]tr, [p]rev, [g]rab, [b]oundary */
		int slen, plen, glen, blen;

		prev = get_content_chunk(csize - 1);
		slen = strlen(str);
		plen = strlen(prev);
		blen = strlen(ct[CT_AVP]["boundary"]);
		glen = blen + 8;

		prev += (slen > blen) ? str[..blen] : str;
		cpos = (plen < glen) ? 0 : (plen - glen);
		if((cpos = index_of(cpos, prev, CRLF + "--" + 
							ct[CT_AVP]["boundary"])) != -1) {
			/* This might be a split boundary; end of previous->current */
			str = prev[cpos..cpos+(strlen(prev)-plen)] + str;
			set_content_chunk(csize-1, prev[..cpos-1]);
		}
	}

	if((cpos = index_of(0, str, CRLF + "--" + ct[CT_AVP]["boundary"])) != -1) {
		if(cpos == 0) {
			cpos = index_of(cpos, str, CRLF + CRLF);
			if(cpos != -1) {
				cpos += CRLFLEN + CRLFLEN;
			}
		}
		
		if(cpos != -1) {
			add_content(str[0..cpos - 1]);
			return add_part(str[cpos..]);
		} else {
			/* this is last part, or something went VERY wrong */
			/* TODO: Do some checking to see if it's an error or not? */
			add_content(str);
#if 0 /* COMPLETE_FLAG_DOES_NOT_WORK */
			SYSLOG("Flagging complete in add_part()\n");
			complete = TRUE;
#endif
		}
	} else {
		add_content(str);
	}

	return TRUE;
}

/* TODO: This request shit does not belong here */
void	set_request(Request o)	{ request = o; }
Request	get_request()			{ return request; }

int		get_started()			{ return started; }


int get_complete()
{
	if(get_content_length()==0 || (expected_length && get_content_size()==0)) {
		return FALSE;
	}

	if(expected_length == get_content_length() || (is_multipart() && ends_with(
					get_content_chunk(get_content_size()-1), 
					ct[CT_AVP]["boundary"] + "--" + CRLF)
			)) {
		return TRUE;
	}

	if(get_content_length() > expected_length) {
		error("We have recieved TOO MUCH content (expected " + 
				expected_length + "), this is what we have: --|" +
				content_tostring() + "|--\n"
			  );
	}
	
	return FALSE;
}

