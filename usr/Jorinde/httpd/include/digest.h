# define AU_RES_HEADER      "WWW-Authenticate"
# define AU_REQ_HEADER      "Authorization"

/* Structure that holds an authentication header (response or request) */
# define AU_HEADER          0
# define AU_METHOD          1
# define AU_AVP             2
# define AU_STRUCT_SIZE     3
                                                                                
/* Authorization/WWW-Authenticate digest attributes (AVP) */
# define AU_ATT_USER        "username"
# define AU_ATT_REALM       "realm"
# define AU_ATT_NONCE       "nonce"
# define AU_ATT_URI         "uri"
# define AU_ATT_QOP         "qop"
# define AU_ATT_NC          "nc"
# define AU_ATT_CNONCE      "cnonce"
# define AU_ATT_RESPONSE    "response"
# define AU_ATT_OPAQUE      "opaque"
# define AU_ATT_STALE       "stale"
# define AU_ATT_ALGORITHM   "algorithm"

