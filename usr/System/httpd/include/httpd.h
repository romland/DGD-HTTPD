/**
 * Description: Defines on the ~System side of things.
 * Version:     $Id$
 * License:     see Jorinde/doc/License
 */
# include "../../jorinde.h"
# undef SYSLOG

# define HTTP_HOME              JORINDE_HTTPD
# define HTTP_ERRORD            JORINDE_ERRORD+"sys/errord"
# define HTTP_USER				HTTP_HOME+"obj/user"

# define SYSLOGD				USR_SYSTEM+"httpd/syslogd"
# define SYSLOG(x)				(SYSLOGD)->syslog(x)
