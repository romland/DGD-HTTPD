/**
 * Description: A container for a test, it will keep information on what
 *				is being tested and what the result of the test was.
 * Version:     $Id: test.c 226 2004-06-20 21:58:43Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../include/path.h"
# include "../include/internal/path.h"

#ifdef __IGOR__
inherit "/lib/lwo";
#endif

private string	_name, _function;
private int		_success, _executed, _asserts, _failedasserts, _runtime;
private object	_object;
private string*	_history;

# define Secure() if(previous_program() != LUNIT_TEST_API && previous_program() != LUNIT_TEST_OBJ) error("illegal call")

static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	_history = ({ });
}

void	set_object(object ob)		{ Secure(); _object = ob; }
object	get_object()				{ return _object; }

void	set_name(string name)		{ Secure(); _name = name; }
string	get_name()					{ return (_name ? _name : "undefined"); }

void	set_function(string name)	{ Secure(); _function = name; }
string	get_function()				{ return _function; }

void	set_status(int val)			{ _success = val; }
int		get_status()				{ return _success; }

void	add_message(string str)		{ _history += ({ str }); }
string*	get_messages()				{ return _history; }

void	set_executed(int val)		{ Secure(); _executed = val; }
int		get_executed()				{ return _executed; }

void	add_assert_count()			{ _asserts++; }
int		get_assert_count()			{ return _asserts; }

void	add_failed_assert_count()	{ _failedasserts++; }
int		get_failed_assert_count()	{ return _failedasserts; }

void	add_error_count()			{ _runtime++; }
int		get_error_count()			{ return _runtime; }

string	to_string()					{ Secure(); return "not implemented"; }

int		is_test()					{ return TRUE; }

int run()
{
	string ret;
	object ob;

	Secure();

	ob = get_object();
	if(ob == nil) {
		add_message("failure: target object is nil");
		set_status(FALSE);
		set_executed(FALSE);
		return FALSE;
	}

#ifndef NO_CATCH
	ret = catch( call_other(ob, get_function()) );
	if(ret) {
		add_message("TEST FAILED: " + get_function() + "()\n" + 
				"\tin " + object_name(ob) + ".c\n" +
				"\terror: " + ret + " (see error log for details)"
		);
		add_error_count();
		set_status(FALSE);
	}
#else
	call_other( ob, get_function() );
#endif

	set_executed(TRUE);
	return TRUE;
}

