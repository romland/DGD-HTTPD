/**
 * Description: DAV XML Root object.
 * Version:     $Id: root.c 28 2004-10-02 10:15:32Z trickster $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../include/dav.h"
# include <kernel/kernel.h>

# define CREATOR(o)	(DRIVER->creator(object_name(o)))

inherit root JORINDE_XML + "/lib/std/root";

private string filename;

string query_filename()		{ return filename; }

string query_directory()
{
	string *arr;
	arr = explode(filename, "/");
	return "/" + implode(arr[0..sizeof(arr)-2], "/");
}

/*
 * This function is also used by the parser to read in external entities.
 */
static string xml_readfile(string file)
{
	if(!strlen(file)) {
		error("no filename set, can't check authorization");
	}

	/* TODO: security checks! Is 'filename' is allowed to read 'file'? */
	/* TODO: check if file exists */

	if(strlen(file) > 4 && file[strlen(file)-4..] != ".xml") {
		error("File does not have a .xml extension.");
	}

	return read_file(file);
}


/*
 * Making this function public is the only difference from the original 
 * XML root object. Eg. we do want to be able to parse XML that does not
 * recide on disk.
 */
int parseXML(string source)
{
	if(CREATOR(previous_object()) != CREATOR(this_object())) {
		error("illegal call");
	}

	return ::parseXML(source);
}

int loadXML(string fn)
{
	filename = fn;
	return ::parseXML(xml_readfile(fn));
}
