# include "support/tests.h"
# include "../include/www.h"
# include "../include/object.h"

inherit test LUNIT+"lib/testcase";
inherit date JORINDE_SHARED+"lib/date";

object objectd;

static void create()
{
	test::create("objectd");
#ifdef __HAVE_OBJECTD__
	add_test("testObjectd");
#endif /* __HAVE_OBJECTD__ */
}


void setup()
{
#ifdef __HAVE_OBJECTD__
	objectd = find_object(OBJECTD_WRAP);
#endif /* __HAVE_OBJECTD__ */
}


void teardown()
{
}

/* The tests */

void testObjectd()
{
	/*
	 * This is really a bad unit-test now, but I want to ignore this test
	 * if, for some reason, __HAVE_OBJECTD__ was set and it does not exist.
	 * This is to simplify packaging.
	 */
	if(objectd == nil) {
		SYSLOG(	"Ignoring Unit-Test for objectd (could not find it), " + 
				"remove __HAVE_OBJECTD__ in your glue to get rid of " + 
				"this warning and the [possible] error above.\n");
		return;
	}
	assert_notnil(objectd->report_on_object( object_name(this_object()) ));
	assert_notnil(objectd->od_status( object_name(this_object()) )[6] );
	assert_notnil(objectd->destroyed_obj_list());
}
