/**
 * Description: API to be used to talk to the testsuites (is it used?).
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
static int setup(object suite)
{
	if(!suite->is_testsuite()) {
		return FALSE;
	}

	test->setup();
	return TRUE;
}


static int teardown(object suite)
{
	if(!suite->is_testsuite()) {
		return FALSE;
	}

	suite->teardown();
	return TRUE;
}

