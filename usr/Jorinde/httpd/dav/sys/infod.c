/**
 * Description:	Daemon for handling all storage-objects for arbitrary WebDAV
 *				properties and possibly also LOCKs in the future, DGD will
 *				handle swapping out of single storage-objects (that's why 
 *				we're not using LWO's) and this daemon will only initialize
 *				storages as they are used.
 * Version:     $Id: infod.c 167 2004-06-01 08:42:16Z romland $
 * License:     (c)2004 Joakim Romland, see doc/License
 */

# include "../include/dav.h"

inherit LIB_HTTP_STRING;

# define IsAllowed() (object_name(previous_object()) == (DAV_TOOL+"#-1") || previous_object() == this_object())

private mapping storages;

static void create()
{
	storages = ([ ]);
}


/**
 * Name: file_type()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
static int file_type(string path)
{
	mixed **dir;
#ifdef __IGOR__
	dir = get_dir_info(path);
#else
	dir = get_dir(path);
#endif
	if(sizeof(dir[0]) == 0) {
		return F_NONEXISTING;
	}

	if(dir[1][0] == -2) {
		return F_DIRECTORY;
	}
	return F_FILE;
}
                                                                                
                                                                                
/**
 * Name: file_exists()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
static int file_exists(string path)
{
	return (file_type(path) != F_NONEXISTING);
}


/**
 * Name: is_dir()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
static int is_dir(string path)
{
	return (file_type(path) == F_DIRECTORY);
}


/**
 * Name: init_storage()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
static void init_storage(string store, mixed noinit)
{
	object ob;

	ob = clone_object(DAV_STORAGE);
	ob->set_owns(store);
	storages[store] = ob;
}


/**
 * Name: get_parent_path()
 * Desc: Get the parent path of resource if it's not a directory itself.
 * Note:
 * Args:
 * Rets:
 */
static string get_parent_path(string resource)
{
	string *arr;
	if(resource[strlen(resource)-1] == '/') {
		return resource;
	}
	arr = explode(resource, "/");
	return "/" + implode(arr[0..sizeof(arr)-2], "/") + "/";
}


static string get_storage_name(string resource)
{
	return DAV_HOME + "save/" + replace(resource, "/", "_") + ".o";
}


/**
 * Name: get_storage()
 * Desc:
 * Note: noinit should be set to TRUE if you do not want a NEW empty storage
 *		 to be created (eg, the operation is a fetch, not a write).
 * Args: 
 * Rets: storage-object or nil if non-existing and noinit was set.
 */
static object get_storage(string resource, varargs mixed noinit)
{
	string storage;
	storage = get_parent_path(resource);
	if(!storages[storage]) {
		string file;
		resource = get_parent_path(resource);
		file = get_storage_name(resource);
		if(file_exists(file) || noinit != TRUE) {
			init_storage(resource, noinit);
			storages[storage]->load();
		} 
	}
	return storages[storage];
}


/**
 * Name: get()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
mapping get(string resource, varargs string prop)
{
	object ob;
	if(!IsAllowed()) {
		error("illegal call");
	}

	ob = get_storage(resource, TRUE);
	if(!ob) {
		return ([ ]);
	}
	return ob->get(resource, prop);
}

string *names(string resource)
{
	if(!IsAllowed()) {
		error("illegal call");
	}
	return get_storage(resource)->names(resource);
}


string get_xmlns(string resource, string prop)
{
	if(!IsAllowed()) {
		error("illegal call");
	}
	return get_storage(resource)->get_xmlns(resource, prop);
}

/**
 * Name: set()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int set(string resource, string prop, mixed value, varargs string xmlns)
{
	if(!IsAllowed()) {
		error("illegal call");
	}
	return get_storage(resource)->set(resource, prop, value, xmlns);
}


/**
 * Name: copy()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int copy(string prop, string from, string to)
{
	if(!IsAllowed()) {
		error("illegal call");
	}
	return set(to, prop, get(from, prop));
}


/* TODO: Copy collection */
int copy_collection(string from, string to)
{
	if(!IsAllowed()) {
		error("illegal call");
	}
	SYSLOG("infod->copy_collection(): " + from + ", THIS IS A TODO!\n");
	return TRUE;
}

/* TODO: Copy resource */
int copy_resource(string from, string to)
{
	if(!IsAllowed()) {
		error("illegal call");
	}
	SYSLOG("infod->copy_resource(): " + from + ", THIS IS A TODO!\n");
	return TRUE;
}


/**
 * Name: delete()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int delete(string prop, string resource)
{
	if(!IsAllowed()) {
		error("illegal call");
	}
	return set(resource, prop, nil);
}


int delete_collection(string resource)
{
	object ob;
	string fn;
	
	if(!IsAllowed()) {
		error("illegal call");
	}
	if(file_exists((fn = get_storage_name(resource)))) {
		remove_file(fn);
	}

	if((ob = storages[resource])) {
		storages[resource] = nil;
		destruct_object(ob);
	}
	return TRUE;
}


int delete_resource(string resource)
{
	if(!IsAllowed()) {
		error("illegal call");
	}
	return set(resource, nil, nil);
}


/**
 * Name: move()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int move(string prop, string from, string to)
{
	int ret;
	if(!IsAllowed()) {
		error("illegal call");
	}

	ret = set(to, prop, get(from, prop));
	if(ret) {
		return delete(prop, from);
	}
	return ret;
}

/* TODO: Move collection */
int move_collection(string from, string to)
{
	mapping props;
	string *arr;
	int i;

	if(!IsAllowed()) {
		error("illegal call");
	}

	props = get(from);
	arr = map_indices(props);
	for(i = 0; i < sizeof(arr); i++) {
		SYSLOG("infod->move_collection(): " + from + ", THIS IS A TODO!\n");
	}

/*	delete_collection(from);*/
	return TRUE;
}


/* TODO: Move resource */
int move_resource(string from, string to)
{
	mapping props;
	string *arr;
	int i;

	if(!IsAllowed()) {
		error("illegal call");
	}

	props = get(from);
	arr = map_indices(props);
	for(i = 0; i < sizeof(arr); i++) {
		SYSLOG("infod->move_resource(): " + from + ", THIS IS A TODO!\n");
	}
	
/*	delete_resource(from);*/
	return TRUE;
}

