# include "support/tests.h"
# include "../include/www.h"
# include "../include/object.h"

inherit test LUNIT+"lib/testcase";
inherit date JORINDE_SHARED+"lib/date";

static void create()
{
	test::create("xml");
	add_test("testDates");
}


void setup()
{
}


void teardown()
{
}


/* The tests */

void testDates()
{
	string date;

	date = "Sun Dec  1 20:10:25 1974";
	assert_equals(date, ctime(datetime_to_int(date)));
	
	date = "Tue Dec 31 20:10:25 2024";
	assert_equals(date, ctime(datetime_to_int(date)));
	
	date = "Thu Jan  1 20:10:25 2015";
	assert_equals(date, ctime(datetime_to_int(date)));
	
	date = "Sun Feb 29 20:10:25 2004";
	assert_equals(date, ctime(datetime_to_int(date)));
}
