# include "support/tests.h"
# include "../include/www.h"

inherit LUNIT+"lib/testsuite";

/* Accept wild-card in filename and add them as suites */
private void add_many_suites(string dir, string fn)
{
	int i;
	mixed *files;

#ifdef __IGOR__
	files = get_dir_info(dir+fn)[0];
#else
	files = get_dir(dir+fn)[0];
#endif
	for(i = 0; i < sizeof(files); i++) {
		::add_suite(dir+files[i][0..strlen(files[i])-3]);
	}
}


static void create()
{
	::create("TestAll");
	add_many_suites(HTTP_HOME + "tests/", "*_test.c");
}

