/* Various external includes */
# include "../../include/www.h"

/* TODO: Hardcoded path -- we don't want to depend on package like this */
# include "../../../jxml/include/xml.h"

/* XXX TODO: The include above changes the SYSLOG() define, set it back! */
# undef SYSLOG
# define SYSLOG(x)	(HTTP_LOGD)->syslog(x)

#if 0
/* TODO: Hardcoded path -- we don't want to depend on package like this */
#ifndef SYSLOG
# define SYSLOG(x)	(HTTP_LOGD)->syslog(x)
/*# define SYSLOG(x)  ("/usr/Jorinde/sys/logd")->syslog(x)*/
#endif
#endif

/* My version */
# define DAV_VERSION				"0.1.0"

/* Security macros */
# define CMD_SECURE()			(object_name(previous_object()) == HTTP_USER)

# define DAV_INFOD				DAV_HOME + "sys/infod"
# define DAV_STORAGE			DAV_HOME + "obj/storage"
# define DAV_TOOL				DAV_HOME + "data/davtool"

# define XMLNS_DAV				"http://www.xs4all.nl/~jorindek/xmlns/dav/"
# define XMLNS_JOR				"http://www.xs4all.nl/~jorindek/xmlns/jor/"

/* WebDAV (D: / DAV:) namespace properties */
# define DAVP_CREATIONDATE          "creationdate"			/* ! = TODO */
# define DAVP_DISPLAYNAME           "displayname"
# define DAVP_GETCONTENTLANGUAGE    "getcontentlanguage"	/* ! */
# define DAVP_GETCONTENTLENGTH      "getcontentlength"
# define DAVP_GETCONTENTTYPE        "getcontenttype"		/* ! */
# define DAVP_GETETAG               "getetag"				/* ! */
# define DAVP_GETLASTMODIFIED       "getlastmodified"
# define DAVP_LOCKDISCOVERY         "lockdiscovery"			/* ! */
# define DAVP_RESOURCETYPE          "resourcetype"
# define DAVP_SOURCE                "source"				/* ! */
# define DAVP_SUPPORTEDLOCK         "supportedlock"			/* ! */

/* Jorinde's WebDAV (J: / JOR:) namespace properties */
# define DAVJ_OBJECT                "object"
# define DAVJ_LOADED                "loaded"
# define DAVJ_OWNER                 "owner"
# define DAVJ_CLONES                "clones"
# define DAVJ_COMPILETIME           "compiletime"
# define DAVJ_PROGRAMSIZE           "programsize"
# define DAVJ_NSECTORS				"sectors"
# define DAVJ_DATASIZE              "datasize"
# define DAVJ_CALLOUTS              "callouts"
# define DAVJ_INDEX                 "index"

/* used for mapping property: ([ name : ({ DP_* ... }) ... ]) */
# define DP_NAMESPACE       0
# define DP_VALUE           1       /* initially nil */
# define DP_STATUS          2       /* initially nil */
# define DP_NAME_ONLY       3
# define DP_SIZE            4

# define USE_DAV_ALIASES
