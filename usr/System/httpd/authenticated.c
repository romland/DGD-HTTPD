/**
 * Description: Verify a user's password.
 * Version:     $Id: authenticated.c 219 2004-06-16 08:44:39Z romland $
 * License:     see Jorinde/doc/License
 */

# include <kernel/kernel.h>
# include "include/httpd.h"

# define SECURE() (previous_program() == HTTP_HOME+"lib/authen_basic" || previous_program() == HTTP_HOME+"lib/authen_digest")

string password;


static void create(varargs int clone)
{
}


private int get_password(string user)
{
	return restore_object("/kernel/data/" + user + ".pwd");
}


nomask int match(string user, string pass)
{
	if(!SECURE()) {
		error("illegal call\n");
	}

	if(!get_password(user)) {
		return FALSE;
	}
	pass = crypt(pass, password);
	return (password == pass) ? TRUE : FALSE;
}
