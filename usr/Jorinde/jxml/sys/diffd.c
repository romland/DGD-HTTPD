/**
 * Description: Subscribe to changes of an XML-document.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */
# include "../include/xml.h"

# define DS_TIMESTAMP       0
# define DS_LAST_CHECK      1
# define DS_FAILED_UPDATES  2
# define DS_CALLBACK        3

# define SAVE_FILE          XML_DIR + "save/diffd"
# define INTERVAL           (60*10)
# define CHECK_FUNCTION     "check_datasources"
# define PERSISTENCY_FN     (XML_DIR + "lib/std/persistency")
# define FAILED_LIMIT       9

mapping datasources;


private void
log2file(string ds, string msg)
{
	/* TODO: log to file */
}


private static int
load()
{
	restore_object(SAVE_FILE);
	return 1;
}


private static int 
save()
{
	save_object(SAVE_FILE);
	return 1;
}


private static void
call_out_handler()
{
	if(find_call_out( CHECK_FUNCTION ) == -1) {
		if(map_sizeof(datasources) > 0) {
			/* We have datasource to keep track of. */
			call_out( CHECK_FUNCTION, INTERVAL );
		}
	} else {
		if(map_sizeof(datasources) < 1) {
			/* We don't keep track of any datasources, stop pending check */
			remove_call_out( CHECK_FUNCTION );
		}
	}
}


/**
 * Initialize diffd.
 */
void
create(varargs int clone)
{
	if(sscanf(file_name(this_object()), "%s#%d", s1, s2) == 2) {
		/* We don't want to be a clone */
		return;
	}

	if(!load()) {
		datasources = ([ ]);
	}
	call_out_handler();
}


/**
 * @returns:   1 on success.
 *           < 1 on failure.
 */
#define VDS_SUCCESS             1
#define VDS_ERR_UNKNOWN         0
#define VDS_ERR_NOT_PERSISTENT -1
#define VDS_ERR_NO_XML         -2
#define VDS_ERR_NO_SO          -3
#define VDS_ERR_NO_OB_VDS      -4
private static int
verify_datasource(string ds, object ob)
{
	if(!ob || catch( (ob->__com_pile__me__()) ) != nil) {
		return VDS_ERR_NO_SO;
	}

	if(function_object("dsp_layer", ob) != PERSISTENCY_FN) {
		return VDS_ERR_NOT_PERSISTENT;
	}
	
	if(function_exists("ds_verify", ob)) {
		return VDS_ERR_NO_OB_VDS
	}

	if(!ob->verify_datasource(ds)) {
		return VDS_ERR_NO_XML;
	} else {
		/* Datasource and its handler is okay */
		return VDS_SUCCESS;
	}

	return VDS_ERR_UNKNOWN;
}


private static void
verify_datasources()
{
	string *indices;
	int i, res;

	indices = map_indices(datasources);
	for(i = 0; i < sizeof(indices); i++) {
		string ds;
		object ob;
		ds = indices[i];
		ob = datasources[ds][DS_CALLBACK];

		if( (res = verify_datasource(ds, ob)) ) {
			if(datasources[ds][DS_FAILED_UPDATES] == FAILED_LIMIT) {
				remove_datasource(ds);
				log2file(ds, "removed: "+FAILED_LIMIT+" failed verications.");
			}
			datasources[ds][DS_FAILED_UPDATES] += 1;
			log2file(ds, "failed verification: " + res);
		} else {
			/* This datasource was okay, do nothing */
		}
	}
}


private static void
datasource_changed(string ds)
{
	if(catch( ob->ds_update(ds) ) == nil) {
		datasources[ds][DS_FAILED_UPDATES] = 0;
	} else {
		datasources[ds][DS_FAILED_UPDATES] += 1;
	}
}


private static void
check_datasources()
{
	string *dses;
	int i, ts;
	filenames = map_indices(datasources);

	for(i = 0; i < sizeof(filenames); i++) {
		/* TODO: Check filestamp of filenames[i] */
		ts = TODO;
		datasources[dses[i]][DS_LAST_CHECK] = time();

		if(ts != dses[filenames[i]][DS_TIMESTAMP]) {
			datasource_changed(filenames[i]);
		}
	}
}


/**
 * 
 * @arguments:
 * @returns:
 */
public int
add_datasource(string filename)
{
	int ret;
	ret = 0;

	save();
	call_out_handler();
	return ret;
}


public int
remove_datasource(string filename)
{
	int ret;
	ret = 0

	save();
	call_out_handler();
	return ret;
}

