# include "./jorinde.h"

static void create(varargs int clone)
{
#ifdef __HAVE_OBJECTD__
	catch {
		COMPILE(OBJECTD);
		("/kernel/sys/driver")->set_object_manager(find_object(OBJECTD));
		find_object(OBJECTD)->do_initial_obj_setup();
		COMPILE(OBJECTD_WRAP);
	}
#endif /* __HAVE_OBJECTD__ */

#ifdef __HAVE_SSHD__
	if(!find_object("sshd/initd")) {
		COMPILE("sshd/initd");
	}
#endif

#ifdef JORINDE
	if(!find_object(USR_SYSTEM+"httpd/initd")) {
		COMPILE(USR_SYSTEM+"httpd/initd");
	}
#else
	("/kernel/sys/driver")->message("~System/initd is not starting httpd.\n");
#endif
}

