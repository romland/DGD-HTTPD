# include <local.h>

#define AUTO            "/kernel/lib/auto"
#define USER            "/std/user"
#define BINARY          "/std/binary"
#define CLOSED          "/std/closed"
#define DRIVER          "/kernel/sys/driver"
#define GLOBAL          "/kernel/sys/global"
#define HNAME           "/kernel/sys/hname"
#define SIGNALS         "/sys/signals"
#define EVENTS		"/sys/events"
#define MASTER          "/obj/master"
#define ACCESS          "/obj/secure/player/access"
#define PLAYER          "/obj/player"
#define LOGIN           "/obj/login"
#define EDITOR          "/std/editor"
#define CINDENT         "/std/cindent"
#define BANISH          "/room/banish"
#define INTERACTIVE     "/lib/interactive"
#define CONTROL         "/room/control"
#define FINGER          "/obj/secure/player/finger"
#define GUILD           "/room/adv_guild"
#define TOPLIST         "/obj/list/toplist"
#define RAISE           "/obj/raise"
#define SHUT            "/obj/shut"
#define AUTOLOAD        "/sys/autoload"
#define RTIME           "/lib/r_time"
#define WIZLINE         "/sys/wizline"
#define WIZSLINE        "/sys/wizSline"
#define MORTLINE	"/sys/mortline"
#define	IGOR_START_ROOM	"/room/church"

#define RESET_TIME      (130 + random (130) + random(130))
#define MAX_BITS        1200
#define FILE_CHUNK      5000
#define TAIL_CHUNK      1000
#define TAIL_LINES      20
#define CAT_LINES       40
#define READ_BYTES_MAX  50000

#define MAXDEPTH        50
#define MAXTICKS        2000000
#define SWAP_SHUT_PCT   98
#define OBJ_SHUT_RESV   250
#define SWAPINTERVAL    21600
#define SHUTINTERVAL    90

/*
 * Define this to restrict basic spells (missile/shock/fireball) to guildless
 * players only.
 */
#define GUILDLESS_SPELLS

/*
 * Define this to enable alternate characters (registered parallels).
 */
#define IGOR_ALT
#define IGOR_ALT_DELAY       5400

#define KERNEL_CLONE_LIMIT   3000
#define KERNEL_CLONE_DZ      (KERNEL_CLONE_LIMIT - 100)
#define KERNEL_CALLOUT_LIMIT 250
#define KERNEL_CALLOUT_DZ    (KERNEL_CALLOUT_LIMIT - 50)

#define FAILED_LOGIN "/obj/secure/player/failed_login"

#define POSSIBLE_INTERACTIVE(x) \
                (function_object("valid_interactive", (x)) == INTERACTIVE)
#define VALID_INTERACTIVE(x) \
                (POSSIBLE_INTERACTIVE(x) && (x)->valid_interactive())

/* Define to enable support for the HTTPD in the mudlib */
#define __HTTPD__
/* Levels specific defines */
#define IGOR_LEVELS_2001	1

