/**
 * Description: Useful headers.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../../../open/jorinde.h"

# define LUNIT_HOME         JORINDE_LUNIT

/* Internal libraries */
# define LUNIT_OUTPUT_API	LUNIT_HOME + "lib/api/output"
# define LUNIT_TEST_API		LUNIT_HOME + "lib/api/test"
# define LUNIT_ASSERT		LUNIT_HOME + "lib/assert"

/* Internal daemons */
# define LUNIT_LOGGER		LUNIT_HOME + "sys/logd"
# define LUNIT_WRITER		LUNIT_HOME + "sys/writed"
                                                                                
/* Internal objects */
# define LUNIT_TEST_OBJ		LUNIT_HOME + "data/test"

/* Output macro */
#ifdef SYSLOG
# undef SYSLOG
#endif

# define SYSLOG(x)			(LUNIT_WRITER)->write(x)


#ifndef LIB_HTTP_STRING
# define LIB_HTTP_STRING    JORINDE_SHARED + "lib/string"
#endif

