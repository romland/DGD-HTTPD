# include "../include/xml.h"

/* TODO: Security */
void syslog(mixed str) {
	if(str && strlen(str) && str[strlen(str)-1] != '\n') {
		str += "\n";
	}
#ifdef __IGOR__
	write("XML-LOG " + str + "");
#else
	(JORINDE_HTTPD+"sys/logd")->syslog(x)
#endif
}
