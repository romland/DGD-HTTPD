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
	add_test("testNameSpace");
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
			doc->loadXML(JORINDE_HTTPD+"tests/support/data.xml")
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

void testNameSpace()
{
	object davdoc, *nodes;
	string s;

	davdoc = new_object( DAV_HOME + "data/davxmlroot" );
	s = "<?xml version=\"1.0\"?><a:multistatus xmlns:b=\"urn:uuid:c2f41010-65b3-11d1-a29f-00aa00c14882/\" xmlns:c=\"xml:\" xmlns:a=\"DAV:\"><a:response><a:href>http://localhost/</a:href><a:propstat><a:status>HTTP/1.1 200 OK</a:status><a:prop><a:getcontentlength b:dt=\"int\">0</a:getcontentlength><a:creationdate b:dt=\"dateTime.tz\">2004-03-17T18:14:29.458Z</a:creationdate><a:displayname>/</a:displayname><a:getetag>\"29114e04bcc41:1520\"</a:getetag><a:getlastmodified b:dt=\"dateTime.rfc1123\">Wed, 17 Mar 2004 18:15:49 GMT</a:getlastmodified><a:resourcetype><a:collection/></a:resourcetype><a:supportedlock/><a:ishidden b:dt=\"boolean\">0</a:ishidden><a:iscollection b:dt=\"boolean\">1</a:iscollection><a:getcontenttype/></a:prop></a:propstat></a:response></a:multistatus>";

	davdoc->parseXML(s);
/*	SYSLOG("--|\n" + davdoc->xml() + "\n|--\n");
*/
	nodes = davdoc->xpath("/multistatus/response/propstat");
	assert_equals("DAV:", nodes[0]->getNamespace());
}


void testDom()
{
	/* TODO */
}

