#ifndef __JORINDE_H__
# define __JORINDE_H__

/* Change this line to enable different compatiblity glues */
#define USE_GLUE_FOR_KERNEL

#if defined(USE_GLUE_FOR_KERNEL)
	#include "./glue/kernel_glue.h"
#elif defined(USE_GLUE_FOR_IGOR)
	#include "./glue/igor_glue.h"
#endif

/* TODO: This depends on HTTPD, which is not good */
#ifndef SYSLOG
# define SYSLOG(x)				(JORINDE_HTTPD+"sys/logd")->syslog(x)
# define DGDSYSLOG(x)			(JORINDE_HTTPD+"sys/logd")->dgdsyslog(x)
#endif

# include "./shared/include/shared.h"

#endif /* __JORINDE_H__ */
