/**
 * Description: All test-suites should inherit this. A test_suite is a
 *				composite of tests.
 * Version:     $Id: testsuite.c 226 2004-06-20 21:58:43Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/path.h"
# include "../include/internal/path.h"
# define Secure() if(previous_program() != TEST_RUNNER) error("illegal call")

inherit output	LUNIT_OUTPUT_API;

private string	_name;
private object*	_suites;


static void create(varargs string name)
{
	output::create();

	_suites = ({ });

	if(name) {
		_name = name;
	} else {
		_name = object_name(this_object());
	}
}

int is_testsuite()
{
	return TRUE;
}


/* TODO: Catch errors in this function and return FALSE */
static int add_suite(string filename)
{
	COMPILE(filename);

	/* Change here to work with clones instead */
	_suites += ({ find_object(filename) });
	return TRUE;
}


static int remove_suite(string filename)
{
	error("not implemented");
}


int count_suites()
{
	return sizeof(_suites);
}


nomask object* __LUNIT_TEST_run()
{
	int i;
	object *tests;

	Secure();

	tests = ({ });
	for(i = 0; i < sizeof(_suites); i++) {
		_suites[i]->__LUNIT_TEST_setup();
		tests += _suites[i]->__LUNIT_TEST_run();
		_suites[i]->__LUNIT_TEST_teardown();
	}

	return tests;
}


nomask object* __LUNIT_TEST_result()
{
	error("deprecated");
}

