/**
 * Description: Implements simple events for a user's tools (LWO's!).
 *
 *              These events are not as sophisticated as the ones provided
 *              by the Kernel Library, so only allow trusted objects (that
 *              works) to subscribe (by masking can_subscribe()).
 *
 *              The bonus is that we don't have to rely on a (mud)lib to
 *              have events implemented in a way we need them to be.
 *
 *              This can be improved by catching/unsubscribing etc.
 * Version:     $Id: events.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/shared.h"
private mapping subscribers;

#define NO_LISTENERS(n) (!subscribers||!subscribers[n]||!sizeof(subscribers[n]))


static void call_subscribers(string name, object *obs, mixed *args)
{
	int i;

	args = ({ this_object() }) + args;
	for(i = 0; i < sizeof(obs); i++) {
		call_other(obs[i], "lwo_evt_" + name, args...);
	}
}


/**
 * Mask to allow (certain) objects to subscribe.
 *
 * IMPORTANT: Mapping subscribers hold references to LWO's. It's important
 *            that your can_subscribe() implementation only allow objects
 *            which you already keep references to. If you do not do this,
 *            the subscribing LWO's will be kept in memory until they
 *            unsubscribe or this object is destructed, which may or may
 *            not be something you want.
 *
 *            In the case of http-tools it works just fine because a user
 *            lets its tools listen to certain events and when the user is
 *            destructed, so are the tools.
 */
int can_subscribe(object ob, string name)
{
	error("Can't subscribe (aka, are sure you know what you're doing?)");
	return 0;
}


nomask void unsubscribe(string name)
{
	if(NO_LISTENERS(name)) {
		return;
	}

	subscribers[name] -= ({ previous_object() });
}


nomask int subscribe(string name)
{
	object ob;

	ob = previous_object();
	if(!subscribers || !subscribers[name]) {
		error("Event does not exist.");
	}

	if(this_object()->can_subscribe(ob, name)) {
		subscribers[name] += ({ ob });
		return TRUE;
	}
	return FALSE;
}


static nomask object *get_subscribers(string name)
{
	if(NO_LISTENERS(name)) {
		return ({ });
	}
	return subscribers[name] + ({ });
}


static nomask void init_event(string name)
{
	if(!subscribers) {
		subscribers = ([ ]);
	}

	if(!subscribers[name]) {
		subscribers[name] = ({ });
	}
}


static nomask void invoke_event(string name, mixed args...)
{
	object	*obs;
	int		i, ssize;

	if(NO_LISTENERS(name)) {
		return;
	}

	obs = subscribers[name];
	ssize = sizeof(obs);
	obs -= ({ nil });

	if(ssize != sizeof(obs)) {
		subscribers[name] = obs;
	}

	call_out("call_subscribers", 0, name, obs, args);
}


static nomask void stop_event(string name)
{
	if(NO_LISTENERS(name)) {
		return;
	}

	subscribers[name] = nil;
}

