# include <kernel/kernel.h>
# include "../include/www.h"
# include <limits.h>
# include <type.h>

static string *content;
private int buffered;

/*
 * TODO: We want an iterator-implementation for users to talk to contents.
 */

void create(varargs int clone)
{
	content = ({ });
	buffered = FALSE;
}

int contents_buffered()
{
	return buffered;
}

void clear_content()
{
	content = ({ });
	buffered = FALSE;
}


/**
 * TODO: This should recide in data/response.c, not here.
 */
void content_chunks_sent(int arg)
{
	int i;
	if(arg >= sizeof(content)) {
		return;
	}
	buffered = TRUE;

	i = 0;
	/* We will be existing for a while, remove sent data to save memory. */
	do {
		content[i] = nil;
		i++;
	} while(arg > i);
	content -= ({ nil });
}

/**
 * Note: Returns number of characters
 */
int get_content_length()
{
	int i, x;
	for(i = 0; i < sizeof(content); i++) {
		x += strlen(content[i]);
	}
	return x;
}

void update_content_length()
{
	this_object()->set_header("Content-Length", get_content_length());
}

/**
 * Note: Returns number of indices in array
 */
int get_content_size()
{
	/* sizeof(content -= ({ nil })); -- but this should never contain nil */
	return sizeof(content);
}

void add_content(mixed str)
{
	if(str == nil) {
		SYSLOG("WARNING: trying to add content of nil; ignoring\n");
		return;
	}

	if(typeof(str) != T_ARRAY) {
		if(!sizeof(content)) {
			content = ({ str });
		} else {
#ifdef BE_CLEVER_AND_STUPID
// Don't fucking do this merging thing, it messes up partial-reading :)
			if((strlen(content[sizeof(content)-1])+strlen(str)) < 32768) {
				content[sizeof(content)-1] += str;
			} else {
				content += ({ str });
			}
#else
			content += ({ str });
#endif
		}
	} else {
		if(!sizeof(content)) {
			content = str;
		} else {
			content += str;
		}
	}
}

string *get_content()
{
	return content;
}

string get_content_chunk(int i)
{
	return content[i];
}

int set_content_chunk(int i, string chunk)
{
	if(i >= sizeof(content)) {
		return FALSE;
	}
	content[i] = chunk;
	return TRUE;
}

string content_tostring()
{
	int i;
	string x;

	if(get_content_length() > MAX_STRING_SIZE) {
		error("String too large, use get_content()\n");
	}

	x = "";
	for(i = 0; i < sizeof(content); i++) {
		x += content[i];
	}
	return x;
}

