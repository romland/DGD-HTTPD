# include "support/tests.h"
# include "../include/www.h"
# include "../include/object.h"

inherit test LUNIT+"lib/testcase";

private Httpd server;

static void create()
{
	test::create("xml");
	add_test("testProps");
}


void setup()
{
	server = find_object(HTTP_SERVER);
}


void teardown()
{
	server = nil;
}


/* The tests */

void testProps()
{
	assert_notnil( server );
	server->set_property("fnurt", "iblam");
	assert_equals( "iblam", server->get_property("fnurt") );
	assert_equals( 1, map_sizeof(server->get_properties()) );
	assert_false( server->remove_property("foo") );
	assert_true( server->remove_property("fnurt") );
	assert_nil(server->get_property("fnurt"));
}
