/**
 * This class is supposed to handle WebDAV operations on a DGD mapping
 * which can contain an array or a non-collection. See it as a test-case
 * for file operations.
 */

static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif
}

/* These are the functions that we want implemented */
#if 0
	int is_collection() {}
	int cmd_propfind(object request, object response) {}
	cmd_proppatch() {}
	cmd_delete() {}
	cmd_put() {}
	cmd_mkcol() {}
	cmd_copy() {}
	cmd_move() {}
#endif
