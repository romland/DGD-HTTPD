/**
 * Description: Paths for lunit's Public API's.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../../open/jorinde.h"
# define LUNIT_HOME			JORINDE_LUNIT

/* Public libraries */
# define TEST_SUITE			LUNIT_HOME + "lib/testsuite"
# define TEST_CASE			LUNIT_HOME + "lib/testcase"
# define TEST_RUNNER		LUNIT_HOME + "data/runner"

# define TestSuite			object TEST_SUITE
# define TestCase			object TEST_CASE
# define TestRunner			object LIB_HTTP_STRING	/* No lib for this yet */
