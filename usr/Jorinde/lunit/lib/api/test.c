/**
 * Description: API for tests by test_cases.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../../include/internal/path.h"

static void create()
{
}

static object add_test(string testfun, varargs string testname)
{
	object test;

	test = new_object(LUNIT_TEST_OBJ);

	test->set_function(testfun);
	test->set_name(testname);
	test->set_object(this_object());
	test->set_status(TRUE);
	return test;
}

static int run(object test)
{
	if(!test->is_test()) {
		return FALSE;
	}

	test->run();
	return TRUE;
}

