# include "../../open/jorinde.h"
# include "../../httpd/include/access.h"

private string name;

static void create()
{
	/* TODO */
}

private void suicide()
{
	SYSLOG("glue/igor/user->suicide()\n");
	destruct(this_object());
}

static object query_conn()
{
	/* TODO */
	SYSLOG("glue/igor/user->query_conn()\n");
	return this_object();
}


nomask static void disconnect()
{
	SYSLOG("glue/igor/user->disconnect()\n");
	call_out("suicide", 0);
}

nomask static void connection(object conn)
{
	/* TODO */
	SYSLOG("glue/igor/user->connection() (TODO)\n");
}

static object *query_connections()
{
	/* TODO */
	SYSLOG("glue/igor/user->query_connections() (TODO)\n");
}

nomask static void redirect(object user, string str)
{
	/* TODO */
	SYSLOG("glue/igor/user->redirect() (TODO)\n");
}

static void set_name(string n)
{
	if(!n) {
		return;
	}

	SYSLOG("set_name(): " + n + "\n");

	if(name && strlen(name) && n != HTTP_ANON_USER) {
		error("illegal call");
	}

	name = n;
}

static void login(string str)
{
	SYSLOG("glue/igor/user->login()\n");
	set_name(str);
}

static void logout()
{
	/* TODO */
	SYSLOG("glue/igor/user->logout() (TODO)\n");
}

string query_name()
{
	SYSLOG("glue/igor/user->query_name(): " + name + "\n");
	return name;
}

static int message(string str)
{
	SYSLOG("glue/igor/user->message()\n");
	return send_message(str);
}

int message_done()
{
	/* TODO */
	SYSLOG("glue/igor/user->message_done() (TODO?)\n");
}

static void datagram_challenge(string str)
{
	/* TODO */
	SYSLOG("glue/igor/user->datagram_challenge() (TODO)\n");
}

int datagram(string str)
{
	/* TODO */
	SYSLOG("glue/igor/user->datagram() (TODO)\n");
}

