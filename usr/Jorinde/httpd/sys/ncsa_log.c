/**
 * Description: HTTP access-logging daemon.
 * Version:     $Id: ncsa_log.c 210 2004-06-15 08:27:28Z romland $
 * License:     see Jorinde/doc/License
 */

# include "../include/www.h"
inherit LIB_HTTP_STRING;

private string filename, logformat;

void set_logfile(string fn)
{
	filename = fn;
}

int xml_parameter(string key, mixed value)
{
	switch(key) {
	case "log-filename" :
		set_logfile(value);
		return 1;
	case "log-format" :
		logformat = value;
		return 1;
	}
	return 0;
}

/**
 * TODO: There is more to NCSA-logging than this, but it's easy enough
 * to implement for someone patient enough to read the entire document.
 * This is largely based on these two webpages:
 * http://as400bks.rochester.ibm.com/tividd/td/ITWSA/ITWSA_info45/en_US/HTML/
 *		  guide/c-logs.html
 * http://httpd.apache.org/docs/mod/mod_log_config.html
 *
 * NCSA extended/combined log format:
 *		"%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-agent}i\""
 */
static string format_log(string format, object request, object response)
{
	int i, x;
	string *fmt, entry, token, tmp;

	fmt = explode(format, "%");

	entry = "";
	for(i = 0; i < sizeof(fmt); i++) {
		token = fmt[i];
		if(token == "%") {
			entry += "%";
			continue;
		}

		switch(token[0]) {
		case 'h' :
			/* Remote host */
			entry += request->get_client_ip_name() + token[1..];
			break;
		case 'l' :
			/* TODO: Not supported. Remote logname (from inetd) */
			entry += "-" + token[1..];
			break;
		case 'u' :
			/* Remote httpd-user if authenticated */
			if(!(tmp = request->get_authenticated())) tmp = "-";
			entry += tmp + token[1..];
			break;
		case 'r' :
			/* Entire first line of request */
			entry += token[1..];
			break;
		case 't' :
			/* Time TODO: Should be other time format */
			entry += ctime(time()) + token[1..];
			break;
		case 'b' :
			/* Bytes sent, excluding headers */
			tmp = (response->get_header("Content-Length") ? 
					response->get_header("Content-Length") : "0");
			if(tmp == nil || tmp == "0") tmp = "-";
			entry += tmp + token[1..];
			break;
		case 's' :
			/* Status */
			tmp = "" + response->get_status();
			entry += tmp + token[1..];
			break;
		case '{' :
			/* Get any request-header */
			x = index_of(1, token, "}");
			if(x != -1) {
				tmp = request->get_header(token[1..x-1]);
				if(tmp) entry += tmp; else entry += "-";
				if(strlen(token) > x) {
					entry += token[x + 1..];
				}
			} else {
				/* ignore this */
			}
			break;
		default :
			break;
		}
	}
	return entry;
}

void log(object request, object response)
{
	write_file(filename, format_log(logformat, request, response) + "\n");
}

