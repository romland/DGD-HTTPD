# include "support/tests.h"
# include "../include/www.h"
# include "../include/object.h"

inherit test LUNIT+"lib/testcase";

private object bstr;

static void create()
{
#ifndef __IGOR__ /* These fail on Igor due to out of ticks */
	test::create("bstr");
	add_test("testCreate");
	add_test("testIterate");
	add_test("testConcat");
	add_test("testInsert");
	add_test("testPrepend");
	add_test("testPrepend2");
	add_test("testSubstr");
	add_test("testClear");
	add_test("testInsertBig");
	add_test("testFind");
	add_test("testSubbstr");
	add_test("testReplace");
	add_test("testTostring");
	add_test("testStartsWith");
	add_test("testEndsWith");
#endif /* __IGOR__ */
}


void setup()
{
	bstr = new_object(BSTR);
}


void teardown()
{
	bstr = nil;
}


/* The tests */

void testCreate()
{
	assert_notnil(bstr);
	assert_true(bstr->is_bstr());
}


void testIterate()
{
	object iter;

	iter = bstr->iterator();
	assert_notnil(	iter				);
	assert_notnil(	iter->has_next()	);
	assert_equals(	"", iter->next()	);
}


void testConcat()
{
	object iter;
	string str;

	str = "";
	bstr->insert("abc");
	bstr->insert("def");
	
	iter = bstr->iterator();
	while(iter->has_next()) {
		str += iter->next();
	}

	assert_equals("abcdef", str);
}

void testInsert()
{
	object iter;
	string str;

	str = "";
	bstr->insert("123", 3);
	
	iter = bstr->iterator();
	while(iter->has_next()) {
		str += iter->next();
	}

	assert_equals("abc123def", str);
}


void testPrepend()
{
	object iter;
	string str;

	str = "";
	bstr->insert("Hi! ", 0);
	
	iter = bstr->iterator();
	while(iter->has_next()) {
		str += iter->next();
	}

	assert_equals("Hi! abc123def", str);
}


void testPrepend2()
{
	object iter;
	string str;

	str = "";
	bstr->insert("iii", 1);
	
	iter = bstr->iterator();
	while(iter->has_next()) {
		str += iter->next();
	}

	assert_equals("Hiiii! abc123def", str);
}


void testSubstr()
{
	string str;

	str = bstr->substr(0, 6);
	assert_equals("Hiiii!", str);

	str = bstr->substr(7, 6);
	assert_equals("abc123", str);

	str = bstr->substr(7, 5);
	assert_notequals("abc123", str);

	str = bstr->substr(7);
	assert_equals("abc123def", str);
}


void testSubbstr()
{
	object tmpbstr;

	tmpbstr = bstr->subbstr(100, 140000);

	assert_notnil( tmpbstr );
	assert_equals( 195000, bstr->length() );
	assert_equals( 140000, tmpbstr->length() );
}


void testReplace()
{
	/* replace equal size string */
	bstr->replace("MUST", "FOOB");
	
	/* TODO: replace with bigger string * /
	bstr->replace("MAY",  "FOOBAR");
	*/

	assert_equals("FOOB", bstr->substr(194364, 4));

	/* Should not find 'MUST' now */
	assert_equals(-1, bstr->find("MUST") );
}


void testTostring()
{
	object tmpbstr;

	/* Test to_string() */
	tmpbstr = bstr->subbstr(0, 27);
	assert_equals( "\n\n\n\n\n\nNetwork Working Group", tmpbstr->to_string() );
}


void testClear()
{
	object iter;
	string str;

	bstr->clear();
	str = "";
	
	iter = bstr->iterator();
	while(iter->has_next()) {
		str += iter->next();
	}

	assert_equals( "", str );
	assert_equals( 0, bstr->length() );
}


/* The section in data.txt looks like this:
   allows dynamically produced content to be transferred along with the
   information necessary for the recipient to verify that it has
*/
void testInsertBig()
{
	object iter;
	string str;

	/* fill bstr with a lot of data */
#ifdef __IGOR__
	str = read_bytes(JORINDE_HTTPD+"tests/support/data.txt", 0, 65000);
#else
	str = read_file(JORINDE_HTTPD+"tests/support/data.txt", 0, 65000);
#endif
	bstr->insert(str);

#ifdef __IGOR__
	str = read_bytes(JORINDE_HTTPD+"tests/support/data.txt", 65000, 65000);
#else
	str = read_file(JORINDE_HTTPD+"tests/support/data.txt", 65000, 65000);
#endif
	bstr->insert(str);

#ifdef __IGOR__
	str = read_bytes(JORINDE_HTTPD+"tests/support/data.txt", 130000, 65000);
#else
	str = read_file(JORINDE_HTTPD+"tests/support/data.txt", 130000, 65000);
#endif
	bstr->insert(str);

	/* length on a big string */
	assert_equals(195000, bstr->length());

	/* substr on a big string */
	str = bstr->substr(64990, 23);
	assert_equals("with the\n   information", str );
	assert_equals(23, strlen(str));

	/* substr on a big string over boundary */
	str = bstr->substr(65518, 22);
	assert_equals("chunk-ext-val  = token", str);
	assert_equals(22, strlen(str));
	
	/* substr on a big string at begging of chunk in middle */
	str = bstr->substr(65536, 10);
	assert_equals("oken | quo", str);
	assert_equals(10, strlen(str));
}


void testFind()
{
	int pos;

	/* test locate within a chunk */
	pos = bstr->find("with the\n   information necessary", 0);
	assert_equals(64990, pos);
	assert_equals("with the\n   information necessary", bstr->substr(pos, 33));

	/* test locate over a boundary */
	pos = bstr->find("chunk-ext-val  = token", 0);
	assert_equals(65518, pos);
	assert_equals("chunk-ext-val  = token", bstr->substr(pos, 22));
}


void testStartsWith()
{
	assert_true( bstr->startswith("\n\n\n\n\n\nNetwork") );
}


void testEndsWith()
{
	assert_true( bstr->endswith("While the orig") );
}

