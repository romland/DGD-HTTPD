/**
 * Description: Start a test-run, this will call the suite and collect 
 *				the result from all tests and output failures (or 'all 
 *				tests successful').
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/internal/path.h"

# ifdef __IGOR__
inherit "/lib/lwo";
# endif
inherit LIB_HTTP_STRING;

private string _suite;
private object _suiteob;
private int _throw, _silent;

static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	_throw = TRUE;
}


void initialize(string suite)
{
	if(_suite) {
		error("runner is already initialized");
	}

	_suite = suite;
}

void error_on_failure(int val)
{
	_throw = val;
}

void silent_on_success(int val)
{
	_silent = val;
}


# define SEP  "-----------------------------------------------------------"
# define R(x) ralign(x, 5)

private int examine_result(object* tests)
{
	int szt, i, j, ret, f_tests, f_asserts, asserts, errors;
	string *msgs, msg;

	ret = TRUE;
	szt = sizeof(tests);
	msg = "\n";

	for(i = 0; i < szt; i++) {
		object test;

		test = tests[i];
		asserts   += test->get_assert_count();
		errors    += test->get_error_count();
		f_asserts += test->get_failed_assert_count();
		if(test->get_status() == TRUE) continue;

		/* A failed test */
		if(test->get_executed() == FALSE) {
			test->add_message("never executed");
		}

		f_tests++;
		ret = FALSE;
		msgs = test->get_messages();
		for(j = 0; j < sizeof(msgs); j++) {
			msg += msgs[j] + "\n\n";
		}
	}

	if(!ret) {
		msg +=  
"TEST STATS: " + _suite + ".c:\n" +
"\t       \t,---------------------------------------" + ".\n" +
"\t\t|\ttotal\t good\t  bad" + "\t\t|\n" +
"\t       \t+---------------------------------------" + "+\n" +
"\t  Tests\t|\t" + R(szt) + "\t" + R(szt-f_tests)+	"\t" + R(f_tests) + 
"\t\t|\n" +
"\tAsserts\t|\t"+R(asserts)+"\t"+R(asserts-f_asserts)+"\t"+R(f_asserts) +
"\t\t|\n";
		if(errors) {
			msg +=
" Runtime Errors\t|\t" + R("-") +	"\t" +R("-")+			"\t" + R(errors) +
"\t\t|\n";
		}
		msg += 
"\t       \t'---------------------------------------" + "'\n\n";
		SYSLOG(msg);
	} else if(!_silent) {
		SYSLOG("Lunit: all " + sizeof(tests) + " tests (" + asserts + 
				" asserts) successful\n");
	}
	
	return f_tests;
}


/* return TRUE if all tests succeeded, otherwise FALSE */
int run(varargs string suite)
{
	int failures;
	object *result;

	if(!_suite && !suite) {
		error("runner is not initialized");
	} else if(suite) {
		_suite = suite;
	}

	COMPILE(_suite);

	/* if we want to work with clones instead, change here */
	_suiteob = find_object(_suite);

	result = _suiteob->__LUNIT_TEST_run();

	if(failures = examine_result(result)) {
		string msg;

		msg = failures + " unit test" + (failures == 1 ? "" : "s") + " failed";
		if(_throw) {
			error(msg);
		}
		SYSLOG(msg + "\n");
	}

	return TRUE;
}

