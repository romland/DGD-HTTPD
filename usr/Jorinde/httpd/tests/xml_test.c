# include "support/tests.h"
# include "../include/www.h"
# include "../include/object.h"

inherit test LUNIT+"lib/testcase";

private object doc;

static void create()
{
	test::create("xml");
	add_test("testInit");
	add_test("testInsert");
	add_test("testXpath");
	add_test("testDom");
}


void setup()
{
	doc = (JXMLROOTD)->new();
}


void teardown()
{
	doc = nil;
}


/* The tests */

void testInit()
{
	assert_true( 
			doc->parseXML(read_file(JORINDE_HTTPD+"tests/support/data.xml"))
		);
	assert_notnil(doc);
	assert_true( doc->isRoot() );
}

void testInsert()
{
	/* TODO */
}

void testXpath()
{
	object *nodes;

	/* Test with and without leading slash */
	assert_equals( "XML test.", doc->xpath("test/name")[0]->getCleanValue()  );
	assert_equals( "XML test.", doc->xpath("/test/name")[0]->getCleanValue() );

	/* Test wildcard */
	nodes = doc->xpath("/test/collection/*");
	assert_equals( 3, sizeof(nodes) );
	assert_equals( "config1.xml", nodes[0]->getAttribute("config-file") );

	/* Test getting node */
	nodes = doc->xpath("/test/collection/");
	assert_equals( 1, sizeof(nodes) );
	nodes = doc->xpath("/test/collection");
	assert_equals( 1, sizeof(nodes) );

}

void testDom()
{
	/* TODO */
}

