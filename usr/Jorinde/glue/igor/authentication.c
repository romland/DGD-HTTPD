# include "../../open/jorinde.h"

# define SECURE() (previous_program() == JORINDE_HTTPD+"lib/authen_basic" || previous_program() == JORINDE_HTTPD+"lib/authen_digest")

private object passd;

static void create()
{
	("/obj/secure/player/httpd/passwd")->__WAKEUP();
	passd = find_object("/obj/secure/player/httpd/passwd");
	if(!passd) error("no password restorer available!");
}

nomask int match(string user, string pass)
{
	string password;

	if(!SECURE()) {
		error("illegal call\n");
	}

	if(!(password = passd->get_password(user))) {
		return FALSE;
	}
	pass = crypt(pass, password);
SYSLOG("real password: " + password + "\n");
SYSLOG("entered  pass: " + pass + "\n");
	return (password == pass) ? TRUE : FALSE;
}
