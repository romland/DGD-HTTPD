# include "../../open/jorinde.h"

/* HTTPD-core Directories */
# define HTTP_HOME              JORINDE_HTTPD
# define DAV_HOME				HTTP_HOME + "dav/"
# define HTTP_MH				HTTP_HOME + "mh/"
#if 0
# define HTTP_CONFIG_DIR		HTTP_HOME + "config/"
# define HTTP_ERROR_PAGES		HTTP_HOME + "errors/"
#endif

/* HTTPD-core Daemons */
# define HTTP_SERVER			HTTP_HOME + "sys/server"
# define HTTP_ERRORD			HTTP_HOME + "sys/errord" 
# define HTTP_AUTHORIZE			HTTP_HOME + "sys/authorize"
# define HTTP_AUTHENTICATE		HTTP_HOME + "sys/authenticate"
# define HTTP_NCSA_LOGGER		HTTP_HOME + "sys/ncsa_log"
#if 0
# define HTTP_STATUSD_400_500	HTTP_HOME + "sys/statusd_400_500"
#endif
# define HTTP_LOGD				HTTP_HOME + "sys/logd"
#if 0
# define HTTP_ERRORD            HTTP_HOME + "sys/errord"
#endif

/* HTTPD-core Objects */
# define HTTP_USER				HTTP_HOME + "obj/user"
# define HTTP_APP				HTTP_HOME + "obj/app"

/* HTTPD-core LWO's */
# define HTTP_COOKIE			HTTP_HOME + "data/cookie"
# define HTTP_SESSION			HTTP_HOME + "data/session"
# define HTTP_MIME				HTTP_HOME + "data/mime"
# define HTTP_RESPONSE			HTTP_HOME + "data/response"
# define HTTP_REQUEST			HTTP_HOME + "data/request"
# define HTTP_CONTENT			HTTP_HOME + "data/content"
# define HTTP_URI				HTTP_HOME + "data/uri"
# define HTTP_TOOL				HTTP_HOME + "data/httptool"

/* HTTPD-core Libraries */
# define LIB_HTTP_SERVER		HTTP_HOME + "lib/server"
# define LIB_HTTP_AUTH_BASIC    HTTP_HOME + "lib/authen_basic"
# define LIB_HTTP_AUTH_DIGEST   HTTP_HOME + "lib/authen_digest"
# define LIB_HTTP_AUTHORIZE     HTTP_HOME + "lib/authorize"
#if 0
# define LIB_HTTP_TEXT_ERROR	HTTP_HOME + "lib/text_error"
#endif
# define LIB_HTTP_URI			HTTP_HOME + "lib/uri"
# define LIB_HTTP_APP			HTTP_HOME + "lib/app"
#if 0
# define LIB_HTTP_STRING		HTTP_HOME + "lib/string"
#endif
# define LIB_HTTP_COOKIE		HTTP_HOME + "lib/cookie"
# define LIB_HTTP_HEADER		HTTP_HOME + "lib/header"
# define LIB_HTTP_CONTENT		HTTP_HOME + "lib/content"
# define LIB_HTTP_SESSION		HTTP_HOME + "lib/session" /* Used by app.c */
# define LIB_HTTP_CMD_PLUGINS	HTTP_HOME + "lib/command_plugins"
#if 0
# define LIB_HTTP_CONNECTION	HTTP_HOME + "lib/connection"
#endif
# define LIB_HTTP_MEDIAHANDLER	HTTP_HOME + "lib/mediahandler"

# define LIB_HTTP_BUFF_USER		HTTP_HOME + "lib/buff_user"
# define LIB_HTTP_BUFF_TOOL		HTTP_HOME + "lib/buff_tool"
# define LIB_FILE_TOOL			HTTP_HOME + "lib/file"

/* Confusing name seeing as we have a httptool.c and this is NOT it */
# define LIB_HTTP_TOOL			HTTP_HOME + "lib/tool"

/* Object types */
# define Uri				object LIB_HTTP_URI
# define Application		object LIB_HTTP_APP
# define Cookie				object LIB_HTTP_STRING	/* no lib for data/cookie*/
# define Content			object LIB_HTTP_CONTENT
# define Session			object JORINDE_SHARED+"lib/properties" /*no lib*/
# define Mediahandler		object LIB_HTTP_MEDIAHANDLER
# define BufferingUser		object LIB_HTTP_BUFF_USER
# define BufferingTool		object LIB_HTTP_BUFF_TOOL
# define Request			object LIB_HTTP_HEADER	/* no request lib */
# define Response			object LIB_HTTP_HEADER	/* no response lib */
# define User				object LIB_HTTP_BUFF_USER
# define Mime				object LIB_HTTP_CONTENT	/* no mime lib */
# define Httpd				object LIB_HTTP_SERVER

/* HTTPD-core Services */
#if 0
# define LIB_HTTP_CMD			HTTP_HOME + "lib/httpcmd"
#endif

/* HTTPD-core API's */
# define API_HTTP_MEDIAHANDLER	HTTP_HOME + "lib/api/mediahandler"

/* TODO: Following does not belong here */
# undef SYSLOG
# define SYSLOG(x)				(HTTP_LOGD)->syslog(x)
# define LOG(x)					(HTTP_LOGD)->log(x)

