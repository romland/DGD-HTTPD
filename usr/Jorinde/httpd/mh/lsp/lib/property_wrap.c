/**
 * Description: Wrapper around properties in another object. To be used by
 *				LSP's and inherited by all lsp-wrappers.
 * Version:     $Id: session.c 226 2004-06-20 21:58:43Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/* The object we are wrapped around */
private object wrapped;

static set_wrapped(object ob)
{
	wrapped = ob;
}

int set_property(mixed key, mixed value)
{
	return wrapped->set_property(key, value);
}

mixed get_property(mixed key)
{
	return wrapped->get_property(key);
}

int remove_property(mixed key)
{
	return wrapped->remove_property(key);
}

mapping get_properties()
{
	return wrapped->get_properties();
}

