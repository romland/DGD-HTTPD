/**
 * Description: Implementation of HTTP/1.1 digest authentication.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/*
realm
	A string to be displayed to users to inform them which username and
	password to use. For example "registered_users@gotham.news.com".

nonce
	A server-specified data string which should be uniquely generated each
	time a 401 response is made. Specifically, since the string is passed
	in the header lines as a quoted string, the double-quote character is
	not allowed. A nonce might, for example, be constructed as the base 64
	encoding of time-stamp H(time-stamp ":" ETag ":" private-key)

opaque
	A string of data, specified by the server, which should be returned by
	the client unchanged in the Authorization header of subsequent requests
	with URIs in the same protection space.

stale
	A flag, indicating that if true - the request was correct, 
	username-password also correct, nonce was incorrect; if false or any
	other value or if not exists - the username-password are incorrect

algorithm
	optional, MD5 = default

qop
	If present, it is a quoted string of one or more tokens indicating the
	"quality of protection" values supported by the server. The value
	"auth" indicates authentication; the value "auth-int" indicates
	authentication with integrity protection.

											Comments by Anatoly Lubarsky 
*/

# include <kernel/kernel.h>
# include <kernel/access.h>
# include "../include/www.h"
# include "../include/digest.h"
# include "../include/access.h"

inherit str	LIB_HTTP_STRING;

private string *methods;

static string *create()
{
	return ({ "Digest" });
}


nomask static string get_password(string user)
{
	object pwd;
	string password;

	pwd = new_object( HTTP_PASSWORD );

	/* returns nil on non-existing user */
	return pwd->get(user);
}

/**
 * nonce, adj: made only once or for a special occasion
 * TODO: SECURITY! make this stronger. :)
 */
static string generate_nonce()
{
/* TESTING TESTING */
	return "0";
	return (string)(time()+180);
}


/**
 * TODO: SECURITY! make this stronger. :)
 */
static string generate_opaque()
{
	return friendly_md5( "0" );
}

/* returns TRUE (bad) on stale nonce, FALSE (good) otherwise */
static int stale_nonce(string nonce)
{
/* TESTING TESTING */
	return FALSE;
	if(is_numeric(nonce)) {
		int n;
		n = (int)nonce;
		return (n <= (time() + 180) && n >= time()) ? FALSE : TRUE;
	}
	return TRUE;
}

private int get_au(Request request, mixed *au)
{
	string method, str, attribs;
	mapping avp;
	
	if(sizeof(au) != AU_STRUCT_SIZE) {
		return FALSE;
	}
	str = request->get_header(au[AU_HEADER]);
	if(str && sscanf(str, "%s %s", method, str) != 2) {
		SYSLOG("get_au() header not found\n");
		return FALSE;
	}
	avp = parse_avp(str, ",", "=");
	if(avp == nil) {
		SYSLOG("No avp?!\n");
	}
	
	au[AU_METHOD] = method;
	au[AU_AVP] = avp;
	return TRUE;
}


/*
 * 
 */
static string authenticate(Request request)
{
	int		stale;
	mixed	*au_req, *au_res;
	string	a1, ha1, a2, ha2;
	string	password, realm, result, md5_result;
	mapping	req_avp;

	au_req = allocate(AU_STRUCT_SIZE);
	au_req[AU_HEADER] = AU_REQ_HEADER;
	if(get_au(request, au_req) == FALSE) {
		SYSLOG("auth_request(): request: failed to parse header\n");
		return nil;
	}
	
	if(au_req[AU_METHOD] != "Digest" || !map_sizeof(au_req[AU_AVP]) ) {
		SYSLOG("auth_request(): request: wrong method or no attributes\n");
		return nil;
	}

	req_avp = au_req[AU_AVP];

	/* get password (crypt():ed) */
	if( (password = get_password(req_avp[AU_ATT_USER])) == nil ) {
		SYSLOG("auth_request(): could not fetch password\n");
		return nil;
	}

/* TESTING TESTING */
	password = "-";
/* END OF */

	/* TODO: SECURITY! Replace below with a real value */
	realm =	"jorinde@dgd"; /* + request->get_header("Host");*/

	a1 = req_avp[AU_ATT_USER]	+ ":" +
		realm					+ ":" +
		password;
	ha1 = friendly_md5(a1);

	a2 = request->get_command()	+ ":" +		/* This is GET, POST etc */
		req_avp[AU_ATT_URI];
	ha2 = friendly_md5(a2);

	/* Check nonce */
	stale = stale_nonce(req_avp[AU_ATT_NONCE]);
	/* TODO: SECURITY! store nonce in application! Verify against that!
	 * request->get_application()
	 */
	/* TODO: app.Context.Items["staleNonce"] = isNonceStale; */

	if(req_avp[AU_ATT_QOP]) {
		result = ha1					+	":" +
				req_avp[AU_ATT_NONCE]	+	":" +
				req_avp[AU_ATT_NC]		+	":" +
				req_avp[AU_ATT_CNONCE]	+	":" +
				req_avp[AU_ATT_QOP]		+	":" +
				ha2;
	} else {
		result = ha1					+	":" +
				req_avp[AU_ATT_NONCE]	+	":" +
				ha2;
	}

	md5_result = friendly_md5(result);

	SYSLOG("          AVP: " + make_string(req_avp) + "\n");
	SYSLOG("   Digest got: '" + req_avp[AU_ATT_RESPONSE] + "'\n");
	SYSLOG("Digest wanted: '" + md5_result + "'\n");
	SYSLOG("Want (nohash): " + result + "\n");
	SYSLOG("  Stale nonce: " + stale + "\n");

	if(req_avp[AU_ATT_RESPONSE] == md5_result && !stale) {
		return req_avp[AU_ATT_USER];
	} else {
		return nil;
	}

	/* Should not get here */
	return nil;
}

static string initialize(Reqest request)
{
	string realm, nonce, opaque, stale, algorithm, qop;

	/* TODO: SECURITY! Replace below with a real value */
	realm =		"jorinde@dgd"; /* + request->get_header("Host");*/
	nonce =		generate_nonce();
	opaque =	generate_opaque();
	/* TODO: SECURITY! store nonce in application! Verify against that!
	 * request->get_application()
	 */
	stale =		(stale_nonce(nonce) ? "true" : "false");
	algorithm =	"MD5";
	qop =		"auth";

	return	"Digest " +
			"realm=\"" +		realm		+ "\", " +
			"qop=\"" +			qop			+ "\", " +
			"nonce=\"" +		nonce		+ "\", " +
			"opaque=\"" +		opaque		+ "\", " +
			"algorithm=" +		algorithm	+ ", " +
			"stale=" +			stale		+ "";
}
#if 0

response:
WWW-Authenticate: Digest
		realm="roulette",
		qop="auth,auth-int",
		nonce="1084226453",
		opaque="4D6F6E204D61792031302032333A35373A35332032303034",
		algorithm=MD5,
		stale=false

request:
Authorization: Digest
		username="admin",
		realm="foo@roulette",
		nonce="1084226819",
		uri="/secure/",
		algorithm=MD5,
		response="9012e9184571b5ba40a0fcefa70a1cea",
		opaque="547565204D61792031312030303A30333A35392032303034",
		qop=auth,
		nc=00000001,
		cnonce="501cc2749d23426a"
#endif

