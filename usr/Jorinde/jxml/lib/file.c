/**
 * Description: IO helper library.
 * Version:     $Id: file.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
string _filename(string s)
{
	string *fn;

	fn = explode(s, "/");
	return explode(fn[sizeof(fn)-1], "#")[0];
}
