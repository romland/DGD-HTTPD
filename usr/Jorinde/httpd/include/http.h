/* HTTP status groups */
#define HTTP_STG_UNKNOWN        0
#define HTTP_STG_INFO           1
#define HTTP_STG_SUCCESS        2
#define HTTP_STG_REDIRECT       3
#define HTTP_STG_CLIENT_ERROR   4
#define HTTP_STG_SERVER_ERROR   5

/* Misc */
#define CRLF                    "\r\n"
#define CRLFLEN                 2

# define HTTP11

# ifdef HTTP10
# define HTTP_PROTOCOL      "HTTP/1.0"
# else
# define HTTP_PROTOCOL      "HTTP/1.1"
# endif

