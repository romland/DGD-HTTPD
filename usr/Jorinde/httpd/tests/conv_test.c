# include "support/tests.h"
# include "../include/www.h"
# include "../include/object.h"

inherit test LUNIT+"lib/testcase";
inherit date JORINDE_SHARED+"lib/encdec";

static void create()
{
	test::create("conversion");
	add_test("testConvert");
}


void setup()
{
}


void teardown()
{
}

/* The tests */

void testConvert()
{
	string str;

	assert_equals( 255, hex_to_int("ff") );
	assert_equals(   1, hex_to_int("01") );
	assert_equals(   0, hex_to_int("00") );

	assert_equals( "0",  hex_to_char("30") );
	assert_equals( "A",  hex_to_char("41") );
	assert_equals( "Q",  hex_to_char("51") );
	assert_equals( "a",  hex_to_char("61") );
	assert_equals( "q",  hex_to_char("71") );
	assert_equals( "\n", hex_to_char("0a") );
	assert_equals( "\r", hex_to_char("0d") );

	assert_equals( 15, hex("f") );

	str = "%0AAn+overview+of+the+kernel+library+for+DGD%3Cbr%3E%3Cbr%3E%3Cbr%3E";
	assert_equals( "\nAn overview of the kernel library for DGD<br><br><br>", url_decode(str) );
}
