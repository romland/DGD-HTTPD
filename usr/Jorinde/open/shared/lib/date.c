/**
 * Description: General purpose date and time library.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../include/shared.h"
#ifndef __IGOR_GLUE__
# include <kernel/kernel.h>
#endif
# include <type.h>

inherit LIB_HTTP_STRING;
inherit LIB_ARRAY;

/**
 * Accepts DGD's date format in string or integer.
 * Netscape:	Wdy, DD-Mon-YYYY HH:MM:SS GMT
 *				Sun, 17-Jan-2038 19:14:07 GMT
 *				01234567890123456789012345678
 * DGD:			Wed Jan 21 20:10:25 2004
 *				012345678901234567890123
 */
/* TODO: RFC 2068, date format */
static string datetime(mixed arg, varargs string zone)
{
	string x;

	if(typeof(arg) == T_STRING && strlen(arg) != 24) {
		return arg;
	}
	x = ( (typeof(arg)==T_INT) ? ctime(arg) : arg );
	x = x[0..2] + ", " + x[8..9] + "-" + x[4..6] + "-" + x[20..23] + " "
		+ x[11..18] + "" + (zone ? zone : "");
	return x;
}


/* TODO: -Almost- duplicate code, see if we can't re-use the function above
<a:getlastmodified b:dt="dateTime.rfc1123">
		Wed, 17 Mar 2004 18:15:49 GMT
</a:getlastmodified>
*/
static string datetime_rfc1123(mixed arg, varargs string zone)
{
	string x;

	if(typeof(arg) == T_STRING && strlen(arg) != 24) {
		return arg;
	}
	
	x = ( (typeof(arg)==T_INT) ? ctime(arg) : arg );
	x = x[0..2] + ", " + x[8..9] + " " + x[4..6] + " " + x[20..23] + " "
		+ x[11..18] + "" + (zone ? zone : "");
	return x;
}
																																							 
# define SHORTMONTHS	({ "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" })
# define SHORTDAYS		({ "sun", "mon", "tue", "wed", "thu", "fri", "sat" })
# define LONGDAYS		 ({ "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday" })

# define DAYSPERMONTH	 ({ 31,28,31,30,31,30,31,31,30,31,30,31 })

# define IsLeapYear(x)	( !((x) % 4) && ( ((x) % 100) || !((x) % 400) ) )

# define DT_WEEKDAY		0
# define DT_DATE		1
# define DT_MONTH		2
# define DT_YEAR		3
# define DT_HOUR		4
# define DT_MINUTE		5
# define DT_SECOND		6
# define DT_ZONE		7
# define DT_SIZE		8
# define NewDT()		allocate(DT_SIZE)


static int get_dayofyear(int year, int month, int date)
{
	int ret;
	
	ret = 0;
	if(IsLeapYear(year) && month > 1) {
		ret += 1;
	}
	
	while(month--) {
		ret += DAYSPERMONTH[month];
	}
	
	ret += date;
	return ret;
}


static int dt_to_int(mixed *dt)
{
	string strval;
	int *intdt;
	int ret, yearday, i;
	
	intdt = NewDT();
	intdt[DT_YEAR]	 = (int)dt[DT_YEAR];
	intdt[DT_DATE]	 = ((int)trim_left(dt[DT_DATE])) - 1;
	intdt[DT_MONTH]	 = (int)index_of_arr(dt[DT_MONTH], SHORTMONTHS);
	intdt[DT_HOUR]	 = ((int)dt[DT_HOUR]) - 1;
	intdt[DT_MINUTE] = (int)dt[DT_MINUTE];
	intdt[DT_SECOND] = (int)dt[DT_SECOND];
	
	yearday = get_dayofyear(intdt[DT_YEAR], intdt[DT_MONTH], intdt[DT_DATE]);
	
	ret = intdt[DT_SECOND] + 60 *
			(intdt[DT_MINUTE] + 60 * (intdt[DT_HOUR] + 24 * yearday));
	
	i = 1970;
	while(i < intdt[DT_YEAR]) {
		if(IsLeapYear(i)) {
			ret += 31622400;
		} else {
			ret += 31536000;
		}
		i++;
	}

	return ret;
}


static int datetime_rfc850_to_int(string val)
{
	error("TODO: datetime_rfc850_to_int()");
}


/* Wed, 17 Mar 2004 18:15:49 GMT */
static int datetime_rfc1123_to_int(string val)
{
	mixed *dt;
	dt = NewDT();
	
	val = lower_case(val);
	dt[DT_WEEKDAY] = val[0..2];
	dt[DT_DATE]		= trim(val[5..6]);
	dt[DT_MONTH]	 = val[8..10];
	dt[DT_YEAR]		= val[12..15];
	dt[DT_HOUR]		= val[17..18];
	dt[DT_MINUTE]	= val[20..21];
	dt[DT_SECOND]	= val[23..24];
	dt[DT_ZONE]		= val[26..27];
	return dt_to_int(dt);
}


/* Wed Jan 21 20:10:25 2004 */
static int ctime_to_int(string val)
{
	mixed *dt, *time;
	
	dt = NewDT();

	val = lower_case(val);
	dt[DT_WEEKDAY] = val[0..2];
	dt[DT_MONTH]	 = val[4..6];
	dt[DT_DATE]		= val[8..9];
	time = explode(val[11..18], ":");
	dt[DT_HOUR]		= time[0];
	dt[DT_MINUTE]	= time[1];
	dt[DT_SECOND]	= time[2];
	dt[DT_YEAR]		= val[20..23];
	return dt_to_int(dt);
}


static int datetime_to_int(string val)
{
	if(strlen(val) < 4) {
		error("invalid date");
	}
	
	if(val[3] == ',') {
		/* RFC 822, updated by RFC 1123 */
		return datetime_rfc1123_to_int(val);
	
	} else if(val[3] == ' ' && strlen(val) != 24) {
		/* RFC 850, obsoleted by RFC 1036 */
		return datetime_rfc850_to_int(val);
	
	} else {
		/* ANSI C's asctime() format */
		return ctime_to_int(val);
	}
}

