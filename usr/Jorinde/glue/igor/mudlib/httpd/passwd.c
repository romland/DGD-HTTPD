/*
 * Used by httpd to authenticate users.
 */
inherit "/lib/files";

# define SERVER	"/players/Jorinde/glue/igor/authentication"

string password;

string get_password(string name)
{
	if(previous_program() != SERVER) {
		error("illegal call");
	}

	if(strlen(name) > 0 && restore_object(player_file(name))) {
		return password;
	}

	return nil;
}
