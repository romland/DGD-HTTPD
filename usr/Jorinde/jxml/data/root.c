/**
 * Description: XML Root object.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/* 
 * Note: Copy this file to your own directory so you have full control
 *       over the root object's read_file() functionality, this is also
 *       needed in order to support external entities.
 */
inherit root "../lib/std/root";

private string filename;

/* Can we get away with this? If not: External entities won't work in these */
int parseXML(string source)	{ error("deprecated; use loadXML()"); }
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


int loadXML(string fn)
{
	filename = fn;
	return ::parseXML(xml_readfile(fn));
}
