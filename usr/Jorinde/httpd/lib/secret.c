/**
 * Description: Container for digest-secrets (right to exist: performance),
 *				the reason we don't use application properties for these is
 *				simply because they are ... secrets.
 * Version:     $Id: secret.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

#define SECURE() (object_name(previous_object()) == HTTP_AUTHENTICATE)

private mapping secrets;

static void create()
{
	secrets = ([ ]);
}

void set_secret(string user, string secret)
{
	if(!SECURE()) {
		error("illegal call");
	}
	secrets[user] = secret;
}

void clear_secret(string user)
{
	if(!SECURE()) {
		error("illegal call");
	}
	secrets[user] = nil;
}

int is_valid_secret(string user, string secret)
{
	if(!SECURE()) {
		error("illegal call");
	}
	if((s = secrets[user]) && s == secret) {
		return TRUE;
	}
	return FALSE;
}

