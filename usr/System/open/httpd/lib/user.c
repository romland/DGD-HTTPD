# include <kernel/kernel.h>
# include <kernel/user.h>

/*
 * This file is for gluing httpd to the kernel library, override by
 * changing jorinde.h
 */

inherit LIB_USER;
inherit user API_USER;

private int valid;

static create(varargs int clone)
{
	if((DRIVER->creator(object_name(this_object()))) == "Jorinde") {
		::create();
		valid = 1;
	} else {
		destruct_object(this_object());
		error("'" + DRIVER->creator(object_name(this_object())) + 
			  "' cannot extend this");
	}
}


nomask static void disconnect()
{
	if(!valid) error("not initialized");
	::disconnect();
}

nomask static void connection(object conn)
{
	if(!valid) error("not initialized");
	::connection(conn);
}

nomask static void redirect(object user, string str)
{
	if(!valid) error("not initialized");
	::redirect(user, str);
}

static void login(string str)
{
	if(!valid) error("not initialized");
	::login(str);
}

static void logout()
{
	if(!valid) error("not initialized");
	::logout();
}

string query_name()
{
	if(!valid) error("not initialized");
	return ::query_name();
}

int message(string str)
{
	if(!valid) error("not initialized");
	return ::message(str);
}

int message_done()
{
	if(!valid) error("not initialized");
	return ::message_done();
}

static void datagram_challenge(string str)
{
	if(!valid) error("not initialized");
	::datagram_challenge(str);
}

int datagram(string str)
{
	if(!valid) error("not initialized");
	return ::datagram(str);
}

