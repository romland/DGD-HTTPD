/**
 * Description: All test-cases should inherit this.	A test case defines the
 *				fixture to run multiple tests.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/path.h"
# include "../include/internal/path.h"
# define Secure() if(previous_program()!=TEST_SUITE) error("illegal call");

inherit test	LUNIT_TEST_API;
inherit assert	LUNIT_ASSERT;

private string	_name;
private object*	_tests;

/* Inheritor must implement following */
static void setup()		{ /* Mask */ } 
static void teardown()	{ /* Mask */ } 


private int implements(string fn)
{
	return function_object(fn, this_object()) ? TRUE : FALSE;
}


static void create(varargs string name)
{
	assert::create();
	test::create();

	_tests = ({ });

	if(name)
		_name = name;
	else
		_name = object_name(this_object());

	if(!implements("setup") || !implements("teardown")) {
		error("testcase must implement setup() and teardown()");
	}
}


int is_testcase()
{
	return TRUE;
}


static int add_test(string testfun, varargs string testname)
{
	_tests += ({ test::add_test(testfun, testname) });

	return TRUE;
}


static int remove_test(string testfun)
{
	int i, removed;

	removed = 0;

	for(i = 0; i < sizeof(_tests); i++) {
		if(_tests[i] && _tests[i]->get_function() == testfun) {
			_tests[i] = nil;
			removed++;
		}
	}

	return removed;
}


static int count_tests()
{
	return sizeof(_tests);
}


/* Should never be called by this_object() */
nomask int __LUNIT_TEST_setup()
{
	Secure();

	_current_test = nil;
	this_object()->setup();

	return TRUE;
}


/* Should never be called by this_object() */
nomask int __LUNIT_TEST_teardown()
{
	Secure();

	this_object()->teardown();
	_current_test = nil;

	return TRUE;
}


/* Should never be called by this_object() */
nomask object* __LUNIT_TEST_run()
{
	int i, result;
	
	Secure();
	if(!_tests) {
		return ({ });
	}
	
	for(i = 0; i < sizeof(_tests); i++) {
		_current_test = _tests[i];
		result = run(_tests[i]);
		if(!result) {
			error( object_name(_tests[i]) + " is an invalid test" );
		}
	}

	return _tests;
}


nomask object* __LUNIT_TEST_result()
{
	error("deprecated");
}

