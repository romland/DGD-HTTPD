# include "../../open/jorinde.h"

#ifndef __IGOR_GLUE__
# include <kernel/kernel.h>
#endif

# define XML_DIR						JORINDE_XML
# define XML_LWO_DIR					XML_DIR + "data/"

# define XML_ROOT_NODE						1
# define XML_CHARACTER_DATA					2
# define XML_CDATA_NODE						3
# define XML_COMMENT_NODE					4
# define XML_ELEMENT_NODE					5
# define XML_ENTITY_REFERENCE_NODE			6
# define XML_FRAGMENT_NODE					7
# define XML_PROCESSING_INSTRUCTION_NODE	8

# define NODE	XML_LWO_DIR + "element"
# define DATA	XML_LWO_DIR + "characterdata"

#ifndef COMPILE
# define COMPILE(s) if(!find_object(s)) compile_object(s);
#endif

#ifdef SYSLOG	/* undef SYSLOG() for XML */
# undef SYSLOG
# define SYSLOG(x)	(JORINDE_XML+"sys/logd")->syslog(x)
#endif

#ifndef LIB_HTTP_STRING
# define LIB_HTTP_STRING	JORINDE_SHARED + "lib/string"
#endif

