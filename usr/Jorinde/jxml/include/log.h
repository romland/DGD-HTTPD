#ifdef LOG_LEVEL_DEBUG
	#define DEBUG(s)	_DEBUG(s)
#else
	#define DEBUG(s)	no_DEBUG(s)
#endif

#ifdef LOG_LEVEL_INFO
	#define INFO(s)		_INFO(s)
#else
	#define INFO(s)		no_INFO(s)
#endif

#ifdef LOG_LEVEL_WARN
	#define WARN(s)		_WARN(s)
#else
	#define WARN(s)		no_WARN(s)
#endif

#ifdef LOG_LEVEL_ERROR
	#define ERROR(s)	_ERROR(s)
#else
	#define ERROR(s)	no_ERROR(s)
#endif


