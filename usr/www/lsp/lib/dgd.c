# include <kernel/kernel.h>
# include <kernel/user.h>
# include <type.h>
# include <status.h>
#ifdef status
# undef status
#endif

inherit "./string";

# define SPACE16	"                "

private object driver;		/* driver object */

static void create(int size)
{
	driver = find_object(DRIVER);
}

/*
 * NAME:    swapnum()
 * DESCRIPTION: return a swap average in X.XX format
 */
static string swapnum(int num, int div)
{
    string str;

    str = (string) ((float) num / (float) div);
    str += (sscanf(str, "%*s.") != 0) ? "00" : ".00";
    if (strlen(str) > 4) {
    str = (str[3] == '.') ? str[.. 2] : str[.. 3];
    }
    return str;
}

static string dgd_status()
{
    mixed *status;
    int uptime, hours, minutes, seconds;
    int short, long;
    int i;
	string str;
    mixed obj;

    if (!str) {
	status = status();
	str =
"                                          Server:       " +
  (string) status[ST_VERSION] + "\n" +
"------------ Swap device -------------\n" +
"sectors:  " + ralign(status[ST_SWAPUSED], 9) + " / " +
	       ralign(status[ST_SWAPSIZE], 9) + " (" +
  ralign((int) status[ST_SWAPUSED] * 100 / (int) status[ST_SWAPSIZE], 3) +
  "%)    Start time:   " + ctime(status[ST_STARTTIME])[4 ..] + "\n" +
"sector size:   " + (((float) status[ST_SECTORSIZE] / 1024.0) + "K" +
		     SPACE16)[..15];
	if ((int) status[ST_STARTTIME] != (int) status[ST_BOOTTIME]) {
	    str += "           Last reboot:  " +
		   ctime(status[ST_BOOTTIME])[4 ..];
	}

	uptime = status[ST_UPTIME];
	seconds = uptime % 60;
	uptime /= 60;
	minutes = uptime % 60;
	uptime /= 60;
	hours = uptime % 24;
	uptime /= 24;
	short = status[ST_NCOSHORT];
	long = status[ST_NCOLONG];
/*	i = sizeof(query_connections());
*/
	i = sizeof(this_user()->get_connections());
	str += "\n" +
"swap average:  " + (swapnum(status[ST_SWAPRATE1], 60) + ", " +
		     swapnum(status[ST_SWAPRATE5], 300) + SPACE16)[.. 15] +
  "           Uptime:       " +
  ((uptime == 0) ? "" : uptime + ((uptime == 1) ? " day, " : " days, ")) +
  ralign("00" + hours, 2) + ":" + ralign("00" + minutes, 2) + ":" +
  ralign("00" + seconds, 2) + "\n\n" +
"--------------- Memory ---------------" +
  "    ------------ Callouts ------------\n" +
"static:   " + ralign(status[ST_SMEMUSED], 9) + " / " +
	       ralign(status[ST_SMEMSIZE], 9) + " (" +
  ralign((int) ((float) status[ST_SMEMUSED] * 100.0 /
		(float) status[ST_SMEMSIZE]), 3) +
  "%)    short term:   " + ralign(short, 5) + "         (" +
  ((short + long == 0) ? "  0" : ralign(100 - long * 100 / (short + long), 3)) +
  "%)\n" +
"dynamic:  " + ralign(status[ST_DMEMUSED], 9) + " / " +
	       ralign(status[ST_DMEMSIZE], 9) + " (" +
  ralign((int) ((float) status[ST_DMEMUSED] * 100.0 /
	 (float) status[ST_DMEMSIZE]), 3) +
  "%) +  other:        " + ralign(long, 5) + "         (" +
  ((short + long == 0) ? "  0" : ralign(long * 100 / (short + long), 3)) +
  "%) +\n" +
"          " +
  ralign((int) status[ST_SMEMUSED] + (int) status[ST_DMEMUSED], 9) + " / " +
  ralign((int) status[ST_SMEMSIZE] + (int) status[ST_DMEMSIZE], 9) + " (" +
  ralign((int) (((float) status[ST_SMEMUSED] +
		 (float) status[ST_DMEMUSED]) * 100.0 /
		((float) status[ST_SMEMSIZE] +
		 (float) status[ST_DMEMSIZE])), 3) +
  "%)                  " + ralign(short + long, 5) + " / " +
			   ralign(status[ST_COTABSIZE], 5) + " (" +
  ralign((short + long) * 100 / (int) status[ST_COTABSIZE], 3) + "%)\n\n" +
"Objects:  " + ralign(status[ST_NOBJECTS], 9) + " / " +
	       ralign(status[ST_OTABSIZE], 9) + " (" +
  ralign((int) status[ST_NOBJECTS] * 100 / (int) status[ST_OTABSIZE], 3) +
  "%)    Connections:  " + ralign(i, 5) + " / " +
			   ralign(status[ST_UTABSIZE], 5) + " (" +
  ralign(i * 100 / (int) status[ST_UTABSIZE], 3) + "%)\n\n";
    }
/*
 else {
	i = -1;
	if (!str || (sscanf(str, "$%d%s", i, str) != 0 &&
		     (i < 0 || i >= hmax || str != ""))) {
	    message("Usage: " + cmd + " [<obj> | $<ident>]\n");
	    return;
	}

	if (i >= 0) {
	    obj = history[i];
	    if (typeof(obj) != T_OBJECT) {
		message("Not an object.\n");
		return;
	    }
	} else if (sscanf(str, "$%s", str) != 0) {
	    obj = ident(str);
	    if (!obj) {
		message("Unknown $ident.\n");
		return;
	    }
	} else {
	    obj = driver->normalize_path(str, directory, owner);
	}

	str = catch(status = status(obj));
	if (str) {
	    str += ".\n";
	} else if (!status) {
	    str = "No such object.\n";
	} else {
	    if (typeof(obj) == T_OBJECT) {
		obj = object_name(obj);
	    }
	    str = driver->creator(obj);
	    if (!str) {
		str = "Ecru";
	    }
	    str = "Object:      <" + obj + ">" +
"\nCompiled at: " + ctime(status[O_COMPILETIME])[4 ..] +
  "    Program size: " + (int) status[O_PROGSIZE] +
"\nCreator:     " + (str + SPACE16)[.. 16] +
  "       Variables:    " + (int) status[O_DATASIZE] +
"\nOwner:       " + (((obj=find_object(obj)) ?
		     (obj=obj->query_owner()) ? obj : "Ecru" :
		     str) + SPACE16)[.. 16] +
  "       Callouts:     " + sizeof(status[O_CALLOUTS]) +
"\nMaster ID:   " + ((int) status[O_INDEX] + SPACE16)[.. 16] +
  "       Sectors:      " + (int) status[O_NSECTORS] + "\n";
	}
    }
*/
    return "<pre>" + str + "</pre>";
}

