# include "support/tests.h"
# include "../include/www.h"
# include "../include/object.h"

inherit test LUNIT+"lib/testcase";

private BigString bstr;

static void create()
{
	test::create("bstr");
	add_test("testFunc");
}


void setup()
{
}


void teardown()
{
}


/* The tests */

void testFunc()
{
}

