/* TODO: More stuff should be moved in here */
#ifndef ST_TICKS
# include <status.h>
#endif
# define AssessCost()       (status()[ST_TICKS])
# define GetCost(c)         (c-status()[ST_TICKS])
# define OutputCost(t, c)   (SYSLOG(ralign(t,16)+": "+ralign(pretty_number(c)+"\n", 12)))

