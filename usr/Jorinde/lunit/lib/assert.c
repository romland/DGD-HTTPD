/**
 * Description: Asserts used by the unit-testing framework.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../include/internal/path.h"
# include <trace.h>
# include <type.h>

inherit "./string";
inherit output  LUNIT_OUTPUT_API;

# define SelfTest()	if(_current_test == nil) error("not properly initialized")

object _current_test;

static void create()
{
	output::create();
	_current_test = nil;
}


private void assert_failed(string operator, mixed expected, mixed actual)
{
	mixed **trace, *test;
	string warning, assert;

	trace = call_trace();
	test   = trace[ sizeof(trace) - 4];
	assert = trace[ sizeof(trace) - 3][TRACE_FUNCTION];

	_current_test->set_status(FALSE);
	
	warning = "TEST FAILED: " + test[TRACE_FUNCTION] + "()\n" +
		"\t" +
		"line " + test[TRACE_LINE] + " " + 
		"in " + test[TRACE_OBJNAME] + ".c " +
		(test[TRACE_PROGNAME] != test[TRACE_OBJNAME] ? 
				" (" + test[TRACE_PROGNAME] + ".c)" : 
				""
		) + 
		"\n" +
		"\t" + assert + " expected " + 
		(operator != "" ? operator + " " : "") +
		"" + make_string(expected) + " " + 
		"(got: " + 
		"" + make_string(actual) + ")";

	_current_test->add_message(warning);
	_current_test->add_failed_assert_count();	
}


static void assert_equals(mixed expected, mixed actual)
{
	SelfTest();

	_current_test->add_assert_count();

	if(expected != actual) {
		assert_failed("", expected, actual );
	}
}


static void assert_notequals(mixed expected, mixed actual)
{
	SelfTest();

	_current_test->add_assert_count();

	if(expected == actual) {
		assert_failed("not", expected, actual );
	}
}

static void assert_nil(mixed actual)
{
	SelfTest();

	_current_test->add_assert_count();

	if(actual != nil) {
		assert_failed("", "nil", actual );
	}
}

static void assert_notnil(mixed actual)
{
	SelfTest();

	_current_test->add_assert_count();

	if(actual == nil) {
		assert_failed("not", "nil", actual );
	}
}

static void assert_true(mixed actual)
{
	SelfTest();

	_current_test->add_assert_count();

	if(actual != TRUE) {
		assert_failed("", TRUE, actual );
	}
}

static void assert_false(mixed actual)
{
	SelfTest();

	_current_test->add_assert_count();

	if(actual != FALSE) {
		assert_failed("", FALSE, actual );
	}
}

static void assert_same(object expected, object actual)
{
	SelfTest();

	_current_test->add_assert_count();

	if(expected == actual) {
		assert_failed("", expected, actual );
	}
}

static void assert_notsame(object expected, object actual)
{
	SelfTest();

	_current_test->add_assert_count();

	if(expected != actual) {
		assert_failed("not", expected, actual );
	}
}

