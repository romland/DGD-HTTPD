# include <kernel/kernel.h>
# include <kernel/user.h>

/*
 * This file is for gluing httpd to the kernel library, override by
 * changing jorinde.h
 */

/* TODO: We want to restrict usage! */

inherit LIB_USER;
inherit user API_USER;

nomask static void disconnect()
{
	::disconnect();
}

nomask static void connection(object conn)
{
	::connection(conn);
}

nomask static void redirect(object user, string str)
{
	::redirect(user, str);
}

static void login(string str)
{
	::login(str);
}

static void logout()
{
	::logout();
}

string query_name()
{
	return ::query_name();
}

int message(string str)
{
	return ::message(str);
}

int message_done()
{
	return ::message_done();
}

static void datagram_challenge(string str)
{
	::datagram_challenge(str);
}

int datagram(string str)
{
	return ::datagram(str);
}

