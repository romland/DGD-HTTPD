/* Enable Kernel compatiblity */
#ifndef __KERNEL__
# define __KERNEL__

/*
 * Path to where creators's directories are (eg. /players, /home, /usr etc).
 */
# define USR						"/usr"

/*
 * The base path where all Jorinde projects are located 
 */
# define JORINDE					USR+"/Jorinde/"

/*
 * All JORINDE projects need to be defined here due to dependencies.
 */
# define JORINDE_HTTPD				JORINDE+"httpd/"
# define JORINDE_XML				JORINDE+"jxml/"
# define JORINDE_LUNIT				JORINDE+"lunit/"
# define JORINDE_ERRORD				JORINDE+"jerror/"
# define JORINDE_SHARED				JORINDE+"open/shared/"
# define JORINDE_INDEX				JORINDE+"jindex/"

/*
 * Where privileged daemons are located. Override with xml-config
 */
# define USR_SYSTEM					USR+"/System/"
# define HTTP_SYSTEM				USR_SYSTEM+"httpd/"
# define HTTP_SYSTEM_OPEN			USR_SYSTEM+"open/httpd/"

/* Underlying user-object you want to base the http-user on. */
# define SYS_LIB_USER				HTTP_SYSTEM_OPEN+"lib/user"

/* Specify where your httpd-configuration files are located */
# define JORINDE_HTTPD_CONFIG_DIR	JORINDE_HTTPD+"config/"

/* Macro to use to compile files */
# define COMPILE(s)					if(!find_object(s)) compile_object(s);

/*
 * Where in the chain of previous objects previous object REALLY is when
 * create() is called. (Kernel library it's 0)
 */
# define CREATE_PREV_OBJ			0

/*
 * Define this to enable tests on Phantasmal's objectd. I guess it could
 * easily be modified for other objectd's as well.
 */
# define __HAVE_OBJECTD__
# define OBJECTD					USR_SYSTEM+"objectd/sys/objectd"
# define OBJECTD_WRAP				USR_SYSTEM+"objectd/sys/objectdwrap"

/*
 * Define this if you want to test the indexing crap :)
 * /
# define __TEST_INDEXD__
*/

# include "./independent.h"

/*
 * Define if you have sshd installed.
 */
#if 0
# define __HAVE_SSHD__
#endif /* 0 */

#endif /* __KERNEL__ */

