# include <kernel/kernel.h>
# include "../include/www.h"

/* TODO: This is also used by urlencoded formdata to have one consistent
 * object for all form-data. A better solution to this problem would be 
 * to have two different objects (multipart and formdata) share the same 
 * base. But I think I want to rewrite all of this to use bstr before I
 * do that.
 */

inherit header  LIB_HTTP_HEADER;
inherit content LIB_HTTP_CONTENT;
inherit LIB_HTTP_STRING;
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

static string name, filename, disposition, type, encoding;
static int length;

/*
 * multipart/form-data:		http://www.faqs.org/rfcs/rfc1867.html
 * MIME (part II) RFC:		http://www.faqs.org/rfcs/rfc2046.html
 */
void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	header::create();
	content::create();
}

void	set_name(string a)			{ name = a; }
void	set_filename(string a)		{ filename = a; }
void	set_length(int x)			{ length = x; }
void	set_disposition(string a)	{ disposition = a; }

string  get_name()					{ return name; }
string  get_filename()				{ return filename; }
int		get_length()				{ return length; }
string  get_disposition()			{ return disposition; }

void set_header(mixed name, mixed val)
{
	if(name == "Content-Disposition") {
		int i;
		string *arr, *arr2, tmp;
		arr = explode(val, ";");
		for(i = 0; i < sizeof(arr); i++) {
			arr2 = explode(arr[i], "=");
			if(sizeof(arr2)) {
				switch(trim(arr2[0])) {
				case "form-data" :
					set_disposition( trim(arr2[0]) );
					break;
				case "name" :
					tmp = trim(arr2[1]);
					if(tmp[0] == '"' && tmp[strlen(tmp)-1] == '"') {
						set_name( tmp[1..strlen(tmp)-2] );
					} else {
						set_name( tmp );
					}
					break;
				case "filename" :
					set_filename( trim(arr2[1]) );
					break;
				default :
					SYSLOG("WARNING: Unknown meta-data '" + arr2[0] +
							"' of content-disposition.\n");
					break;
				}
			}
		}
	}

	/* Do this no matter what. Redundant data, but we'll live -- I like
	 * the fact that this is consistent with other headers.
	 */
	header::set_header(name, val);
}

