/**
 * Description: IO helper library.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
string _filename(string s)
{
	string *fn;

	fn = explode(s, "/");
	return explode(fn[sizeof(fn)-1], "#")[0];
}
