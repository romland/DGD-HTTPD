/**
 * Description: Take care of incoming binary connections and clone user-object
 *				on behalf of data/port.c (which is assigned to be the manager
 *				out of the Kernel's perspective).
 * Version:     $Id$
 * License:     see Jorinde/doc/License
 */

# include <kernel/user.h>
# include <kernel/kernel.h>

# include "include/httpd.h"
# define TRUSTED	HTTP_HOME + "sys/server"
# define PORT_MGR	USR_SYSTEM+"httpd/data/port"

private object	trusted;
private int		timeout;
private mapping	portmanagers;

static void create()
{
	COMPILE(PORT_MGR);
	portmanagers = ([ ]);
}


/**
 * Called during initialization of (httpd) server.c
 */
int init_httpd(int seconds, mapping apps)
{
	int i, j;
	mixed *info;
	object portmgr;
	
	if(previous_object() != find_object(TRUSTED)) {
		error("illegal call");
	}
	trusted = find_object(TRUSTED);
	
	info = map_indices(apps);
	for(i = 0; i < sizeof(info); i++) {
		int port;
		port = (int)apps[info[i]][2];
		if(!portmanagers[port]) {
			portmgr = new_object(PORT_MGR);
			portmgr->set_port(port);
			portmanagers[port] = portmgr;
		} else {
			portmgr = portmanagers[port];
		}
		j = apps[info[i]][0];
#if 0
		SYSLOG("host: " + info[i] + " index: " + j + " port: " + port + "\n");
#endif
#ifdef __IGOR__
		("/kernel/sys/driver")->set_binary_manager(j, portmgr);
#else
		(USERD)->set_binary_manager(j, portmgr);
#endif
	}
	return 1;
}

int query_timeout(object obj)
{
	return timeout;
}
                                                                                
string query_banner(object obj)
{
	return "";
}

/**
 * This could pose a security threat if we have compromised data in
 * ~Jorinde/config/???.xml, objects will be cloned with creator set
 * to ~System. TODO: SECURITY! We might want to restrict this so that
 * only a certain set of objects are allowed.
 */
object *clone_tools(object user, string *plugins)
{
	int i;
	object *ret, wiztool;

	if(previous_object() != trusted) {
		error("illegal call");
	}

#ifdef __IGOR__
	/* Igor efun */
	wiztool = new_object(USR_SYSTEM+"httpd/data/tool");
#else
	/* Kernel library behaviour */
	wiztool = new_object(USR_SYSTEM+"httpd/data/tool", user->query_name());
#endif

	ret = allocate(sizeof(plugins));
	for(i = 0; i < sizeof(plugins); i++) {
#ifdef __IGOR__
		/* Igor efun */
		ret[i] = new_object(plugins[i]);
#else
		/* Kernel library behaviour */
		ret[i] = new_object(plugins[i], user->query_name() );
#endif
		ret[i]->set_owner(user);
		ret[i]->set_tool(wiztool);
	}
	return ret + ({ wiztool });
}

object select(int port, string str)
{
	object user;

	if(!KERNEL() && !SYSTEM()) {
		error("illegal call");
	}

	user = clone_object(HTTP_USER);

	/* Allow trusted object to initialize this user-object */
	if(!trusted || !(trusted->selected(port, user))) {
		/* TODO: What I want is the connection attempt to fail.
		 * Will this do the trick? */
		destruct_object(user);
		return nil;
	}
	return user;
}

