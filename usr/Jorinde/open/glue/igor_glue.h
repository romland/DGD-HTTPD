/*
 * Enable Igor compatiblity
 */
#ifndef __IGOR_GLUE__
# define __IGOR_GLUE__

/*
 * This should be defined if we're on Igor, but make sure.
 */
#ifndef __IGOR__
# define __IGOR__
#endif

/*
 * Path to where creators's directories are (eg. /players, /home, /usr etc).
 */
# define USR				"/players"


/*
 * The base path where all Jorinde projects are located
 */
# define JORINDE			USR+"/Jorinde/"


/*
 * All JORINDE projects need to be defined here due to dependencies.
 */
# define JORINDE_HTTPD		JORINDE+"httpd/"
# define JORINDE_XML		JORINDE+"jxml/"
# define JORINDE_LUNIT		JORINDE+"lunit/"
# define JORINDE_ERRORD		JORINDE+"jerror/"
# define JORINDE_SHARED		JORINDE+"open/shared/"
# define JORINDE_INDEX		JORINDE+"jindex/"

/*
 * Where privileged daemons are located. Override with xml-config
 */
# define USR_SYSTEM			USR+"/System/"
# define HTTP_SYSTEM		USR_SYSTEM+"httpd/"
# define HTTP_SYSTEM_OPEN	USR_SYSTEM+"open/httpd/"


/*
 * Underlying user-object you want to base the http-user on.
 */
# define SYS_LIB_USER		JORINDE+"glue/igor/user"


/*
 * Specify where your httpd-configuration files are located
 */
# define JORINDE_HTTPD_CONFIG_DIR	JORINDE+"glue/igor/config/"


/*
 * Macro to use to compile files
 */
# define COMPILE(s) if(!find_object(s)) compile_object(s); (s)->____WAKEUP();


/*
 * Flag that create() does not get a clone-id when called
 */
# define __NO_CREATE_CLONE_ARG__


/*
 * Where in the chain of previous objects previous object REALLY is when
 * create() is called. (Kernel library it's 0)
 */
# define CREATE_PREV_OBJ	1

/*
 * Check whether an object is a clone when __NO_CREATE_CLONE_ARG__ is set
 */
# define IS_CLONE(o)		(sscanf(object_name(o), "%*s#%*d") == 2)


/*
 * Macro to output data on stdout
 */
# define STDOUT(s)			(("/kernel/sys/driver")->stderr(s))


/*
 * Boolean; since it's not defiend by default
 */
# define TRUE				1
# define FALSE				0


/*
 * We don't want 'status' to mean 'int'
 */
# undef status


/* 
 * Macro to check whether a call came from /players/System
 */
# define SYSTEM()			sscanf(previous_program(), USR + "/System/%*s")


/*
 * Macro to check whether a call came from /kernel/
 */
# define KERNEL()			sscanf(previous_program(), "/kernel/%*s")


/*
 * Flag to set if remove_call_out() takes function-name
 */
# define __RM_CO_TAKES_FN__


/*
 * Flag to set if call_out() returns nothing (void)
 */
# define __CO_IS_VOID__

/*
 * These modes are used by the Kernel Library and I don't think they have
 * any real functionality on Igor but they are here to make things compile
 * none the less
 */
# define MODE_DISCONNECT	0
# define MODE_NOECHO		1   /* telnet */
# define MODE_ECHO			2   /* telnet */
# define MODE_LINE			2   /* binary */
# define MODE_RAW			3   /* binary */
# define MODE_NOCHANGE		4   /* telnet + binary */
# define MODE_UNBLOCK		5   /* unblock to previous mode */
# define MODE_BLOCK			6   /* block input */

# include "./independent.h"

#endif /* __IGOR_GLUE__ */

