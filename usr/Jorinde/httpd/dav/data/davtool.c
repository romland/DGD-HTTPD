/**
 * Description: Implements the WebDAV file-based methods supported by Jorinde.
 *				TODO: Lots and lots of code-cleanup can be done in here,
 *				this includes fixing plenty of duplicate code.
 * Version:     $Id$
 * License:     (c)2004 Joakim Romland, see doc/License
 */

/* --------------------------------------------------------------------------
 *	Status	Method		Functionality
 * --------------------------------------------------------------------------
 *	done	PROPFIND
 *	done				Retrieving Named Properties
 *	TODO				Using allprop to Retrieve All Properties
 *	done				Using propname to Retrieve all Property Names
 *	done				Retrieving named arbitrary properties
 *	done	PROPPATCH
 *	done				Setting arbitrary properties
 *	done	MKCOL
 *	done	GET
 *	done	HEAD
 *	done	POST
 *	n/a-				POST for Collections (not supported)
 *	done	DELETE
 *	done				DELETE for Non-Collection Resources
 *	done				DELETE for Collections
 *	done				DELETE for properties
 *	done	PUT
 *	done				PUT for Non-Collection Resources
 *	n/a-				PUT for Collections (not supported)
 *	done	COPY
 *	done				COPY for resources
 *	TODO				COPY for Properties
 *	done				COPY for Collections
 *	done	MOVE
 *	done				MOVE for resources
 *	TODO				MOVE for Properties
 *	done				MOVE for Collections
 *	n/a-	LOCK		Not supported initially.
 *	n/a-	UNLOCK		Not supported initially.
 */

# include <kernel/access.h>
# include <status.h>
# include "../include/dav.h"
# define USER() ((User)previous_object() == get_owner())

inherit tool	"../../lib/tool";
inherit str		LIB_HTTP_STRING;
inherit util	LIB_XML_UTIL;
inherit date	LIB_DATE;
inherit dav		"../lib/dav";
#ifdef __IGOR__
inherit "/lib/lwo";
#endif

#define DEBUG_DATA

private object	infod;
private mapping	methods, supported;

static void create(varargs int clone)
{
#ifdef __NO_CREATE_CLONE_ARG__
	if(!IS_CLONE(this_object())) { return; }
#else
	if(!clone) { return; }
#endif

	tool::create();
	dav::create();

	set_version("WebDAV",	DAV_VERSION);
	set_method("PROPFIND",	"cmd_propfind");
	set_method("PROPPATCH",	"cmd_proppatch");
	set_method("DELETE",	"cmd_delete");
/*	set_method("PUT",		"cmd_put");*/
	set_method("MKCOL",		"cmd_mkcol");
	set_method("COPY",		"cmd_copy");
	set_method("MOVE",		"cmd_move");

	if(!KERNEL() && !SYSTEM()) {
		error("illegal call");
	}

	supported = ([ ]);
	/* (Web)DAV properties */
	supported["DAV"] = ({
			DAVP_CREATIONDATE, DAVP_DISPLAYNAME, DAVP_GETCONTENTLANGUAGE,
			DAVP_GETCONTENTLENGTH, DAVP_GETCONTENTTYPE, /*DAVP_GETETAG,*/
			DAVP_GETLASTMODIFIED, /*DAVP_LOCKDISCOVERY,*/ DAVP_RESOURCETYPE,
			/*DAVP_SOURCE, DAVP_SUPPORTEDLOCK*/
	});

	/* JOR(inde) properties */
	supported["JOR"] = ({
			DAVJ_OBJECT, DAVJ_LOADED, DAVJ_OWNER, DAVJ_CLONES,
			DAVJ_COMPILETIME, DAVJ_PROGRAMSIZE, DAVJ_DATASIZE,
			DAVJ_CALLOUTS, DAVJ_INDEX
	});
	
	infod = find_object(DAV_INFOD);
}


private void debug_info(object request, object response)
{
	SYSLOG("Request-body:---|\n" + request->content_tostring() + "|---\n");
	SYSLOG("Response-body:---|\n" + response->content_tostring() + "|---\n");
}


/**
 * ([ namespace : ({ propertynames ... }) ... ])
 */
private mapping supported_propertynames()
{
	return supported;
}


private int supported_property(string propname)
{
	return is_member(propname, (supported["DAV"] + supported["JOR"]));
}


/* returns nil if not applicable on the resource type (eg, not a .c file) */
private mixed get_prop_value(string prop, string resource, string xmlns,
		mixed *finfo)
{
	mixed v;
	v = nil;
	if(xmlns == "JOR:") {
		object ob;
		string prog;
		if(sscanf(resource, "%s.c", prog) != 1|| !(ob = find_object(prog))) {
			return nil;
		}
		switch(prop) {
		case DAVJ_OBJECT :		/* 1 for lib, what about others? */
			v = make_string(finfo[3]);
			break;
		case DAVJ_LOADED :		/* is object compiled */
			v = TRUE;
			break;
		case DAVJ_OWNER :		/* owner of master-object */
			v = ob->query_owner();
			break;
		case DAVJ_CLONES :		/* TODO: number of clones */
			v = -1;
			break;
		case DAVJ_COMPILETIME :	/* time of compilation */
			v = datetime_rfc1123(status(ob)[O_COMPILETIME]);
			break;
		case DAVJ_PROGRAMSIZE :	/* program size of object */
			v = status(ob)[O_PROGSIZE];
			break;
		case DAVJ_NSECTORS :	/* # sectors used by object */
			v = status(ob)[O_NSECTORS];
			break;
		case DAVJ_DATASIZE :	/* # variables in object */
			v = status(ob)[O_DATASIZE];
			break;
		case DAVJ_CALLOUTS :	/* # call-outs for object */
			v = make_string(status(ob)[O_CALLOUTS]);
			break;
		case DAVJ_INDEX :		/* unique id for master object */
			v = status(ob)[O_INDEX];
			break;
		}
	} else /*if(xmlns == "DAV:")*/ {
		string tmp;
		switch(prop) {
		case DAVP_CREATIONDATE :
			/*
			 * Windows XP's client demand this datetime format,
			 * how does it work for other clients?
			 */
			v = datetime_tz(0);	/* No support in DGD for creation */
			break;

		case DAVP_DISPLAYNAME :
			v = finfo[0];
			break;

		case DAVP_GETCONTENTLANGUAGE :
			v = "en";	/* TODO */
			break;
		
		case DAVP_GETCONTENTLENGTH :
			v = (finfo[1] >= 0) ? finfo[1] : "0";
			break;
		
		case DAVP_GETCONTENTTYPE :
			if(finfo[1] != -2) {
				sscanf(finfo[0], "%*s.%s", tmp);
				v = get_owner()->get_application()->get_contenttype(tmp);
			} else {
				v = nil;
			}
			break;
		
		case DAVP_GETETAG :
			/* TODO */
			break;
		
		case DAVP_GETLASTMODIFIED :
			v = datetime_rfc1123(finfo[2]);
			break;
		
		case DAVP_LOCKDISCOVERY :
			/* TODO */
			break;
		
		case DAVP_RESOURCETYPE :
#ifdef USE_DAV_ALIASES
			if(finfo[1] == -2) {
				v = new_node("collection");
				v->setNamespace(get_alias("DAV:"));
			} else {
				v = nil;
			}
#else
			v = ((finfo[1] == -2) ? new_node("collection") : nil);
#endif
			break;
		
		case DAVP_SOURCE :
			/* TODO */
			break;
		
		case DAVP_SUPPORTEDLOCK :
			/* TODO */
			break;
		
		default :
#if 0
			/*
			 * Bugs in a propname request (xmlns is nil), error is elsewhere!
			 * I think this was fixed though. Verify.
			 */
			SYSLOG("Unknown property name: " + make_string(prop) + 
					" (" + make_string(xmlns) + ")\n");
#endif
			break;
		}
	}
	return v;
}


/* TRUE on success, values are inserted into the passed in mapping props */
private int set_prop_values(mapping props, string resource, mixed *finfo)
{
	int i;
	string *arr, prop;
	arr = map_indices(props);

	for(i = 0; i < sizeof(arr); i++) {
		prop = arr[i];
		if(props[prop][DP_NAME_ONLY] != TRUE) {
			props[prop][DP_VALUE] = get_prop_value(prop,
												   resource,
												   props[prop][DP_NAMESPACE],
												   finfo);
		}
	}
	return TRUE;
}


/**
 * Generates a compliant WebDAV collection for PROPFIND.
 */
private object *create_collection(string host, string rel_path, 
		string abs_path, mapping props, int maxdepth, varargs int depth)
{
	object	*ret, this;
	mixed	**files, *objects;
	string	*names;
	int		fsize, i, *sizes, *times;

	/* TODO: SECURITY! Need read access to this dir (this fn is recursive!) */
	if((maxdepth - depth) < 0) {
		return ({ });
	} else if(maxdepth > 0) {
		SYSLOG("get_dir (current+content-of): " + abs_path + "/*\n");
		files = ::get_dir(abs_path + "/*");
	} else {
		SYSLOG("get_dir (current): " + abs_path + "\n");
		files = ::get_dir(abs_path);
	}

	names   = files[0];
	sizes   = files[1];
	times   = files[2];
#ifdef __IGOR__
	objects = allocate(sizeof(sizes));
#else
	objects = files[3];
#endif
	fsize   = sizeof(sizes);

	if(fsize == 0 && maxdepth > 0) {
		/* (possibly) empty directory */
		SYSLOG("Case: empty dir\n");
		set_prop_values(props,
						abs_path,
						({ abs_path, -2, 0, nil }));
		this = create_response(host + rel_path, props, TRUE);
		return ({ this });
	} else if(fsize == 0 && maxdepth == 0) {
		/* non-existing file */
		SYSLOG("Case: non-existing file\n");
		set_prop_status(404, props);
		set_prop_values(props,
						abs_path,
						({ abs_path, 0, 0, nil }));
		this = create_response(host + rel_path, props, FALSE);
		return ({ this });
	} else if(maxdepth == 0) { 
		/* changed to check maxdepth == 0, 17jul2004 */
		/* get properties for -this- file/directory only (not content of) */
		mixed **tmp;
		SYSLOG("Case: normal behaviour\n");
		tmp = ::get_dir(abs_path);
#ifdef __IGOR__
		tmp += ({ allocate(sizeof(tmp[0])) });
#endif
		set_prop_values(props,
						abs_path,
						({ tmp[0][0], tmp[1][0], tmp[2][0], tmp[3][0] }));
		this = create_response(host + rel_path, props, FALSE);
		return ({ this }); /* changed to break out, 17jul2004 */
	}

	/* get content of a directory and its subdirectories */
	ret = allocate(fsize);
	for(i = 0; i < fsize; i++) {
		int     size, iscol;
		string	href;
#if 0
		SYSLOG("Fetching: " + names[i] + "\n");
#endif
		size  = sizes[i];
		set_prop_values(props,
						abs_path,
						({ names[i], size, times[i], objects[i] }));
		
		iscol = ((size == -2) ? TRUE : FALSE);
		href  = ending_slash(host + rel_path + "/" + names[i], iscol);
		ret[i] = create_response(href, props, iscol);

		if(iscol && (maxdepth-depth) < 0 ) {
			ret += create_collection(host, 
						rel_path,
						abs_path + "/" + names[i] + "/",
						props,
						maxdepth,
						depth + 1);
		}
	}

	return ({ this }) + ret;
}


private mapping get_dav_props(string path, object node)
{
	mapping value;
	object *props;
	string name;
	mapping ret;
	int i;

	ret = ([ ]);
	props = node->xpath("prop/*");
	if(sizeof(props) > 0 && props[0]->dummy() != TRUE) {
		for(i = 0; i < sizeof(props); i++) {
			name = lower_case(props[i]->getName());
			ret[name] = allocate(DP_SIZE);
			
			/*ret[name][DP_NAMESPACE] = props[i]->getAttribute("xmlns");*/
			ret[name][DP_NAMESPACE] = props[i]->getNamespace();

			/* Make namespace known by alias */
			set_alias( ret[name][DP_NAMESPACE] );

			ret[name][DP_VALUE] = nil;
			if(supported_property(name)) {
				ret[name][DP_STATUS] = 200;
			} else if((value = infod->get(path, name)) != nil) {
				/* TODO: Sometimes value is an XML node.
				 * (when a property contains more than one element 
				 * (eg. authors/author... author...) *crash* */
				ret[name][DP_VALUE] = value[name];
				if(value[name] == nil) {
					ret[name][DP_STATUS] = 501;
				} else {
					ret[name][DP_STATUS] = 200;
				}
			} else {
				ret[name][DP_STATUS] = 404;
			}
		}
	} 
	return ret;
}


/* used by propname and allprop */
private mapping propname(string resource, string xmlns, string *props,
		int name_only)
{
	string prop;
	int i;
	mapping ret;

	ret = ([ ]);
	for(i = 0; i < sizeof(props); i++) {
		prop = props[i];
		ret[prop] = allocate(DP_SIZE);
		ret[prop][DP_STATUS] = 200;
		if(xmlns != "") {
			ret[prop][DP_NAMESPACE] = xmlns;
		} else {
			ret[prop][DP_NAMESPACE] = infod->get_xmlns(resource, prop);
		}
		ret[prop][DP_NAME_ONLY] = name_only;
	}
	return ret;
}


private mapping get_dav_propname(string path)
{
	string *props;
	mapping ret;

	ret = propname(path, /*XMLNS_DAV*/ "DAV:", supported["DAV"], TRUE);
	ret += propname(path, /*XMLNS_JOR*/ "JOR:", supported["JOR"], TRUE);
	ret += propname(path, "", infod->names(path), TRUE);
	return ret;
}


private mapping get_dav_allprop(string path)
{
	string *props;
	mapping ret;

	ret = propname(path, /*XMLNS_DAV*/ "DAV:", supported["DAV"], FALSE);
	ret += propname(path, /*XMLNS_JOR*/ "JOR:", supported["JOR"], FALSE);
	ret += propname(path, "", infod->names(path), FALSE);
	return ret;
}


private mapping get_dav_propfind(string path, object node)
{
	mapping ret;
	object *propfinds;
	int i;

	ret = ([ ]);
	propfinds = node->xpath("/propfind/*");
	if(sizeof(propfinds) > 0 && propfinds[0]->dummy() != TRUE) {
		for(i = 0; i < sizeof(propfinds); i++) {
			ret += get_dav_props(path, propfinds[i]);
		}
	} else {
		/* no body means allprop */
		return get_dav_allprop(path);
	}

	if(map_sizeof(ret) == 0 && sizeof(propfinds) == 1) {
		if(!(propfinds[0]->xpath("propname")[0]->dummy())) {
			ret = get_dav_propname(path);
		} else if(!(propfinds[0]->xpath("allprop")[0]->dummy())) {
			ret = get_dav_allprop(path);
		} else {
			error("TODO: Recover, unknown propfind request\n");
		}
	} else if(map_sizeof(ret) == 0) {
		error("TODO: Recover from this, we got a malformed propfind\n");
	}

	return ret;
}


/**
 * Name: cmd_propfind()
 * Desc:
 * Note:
 * Args:
 * Rets:
 */
int cmd_propfind(object request, object response)
{
	object	doc, root, reqxml, uri;
	int		depth;
	string	host, rel_path, abs_path;
	mapping	props;

	if(!USER()) {
		error("illegal call");
	}
	
	depth = get_depth(request->get_header("Depth"));
	if(depth < 0) {
		SYSLOG("TODO: WARNING: Incorrect depth, what status do we need?\n");
		return FALSE;
	}

	/* Get XML and properties (...) of the request */
	reqxml = get_request_xml(request);
	uri = request->get_uri();
	
	doc = new_xmldoc();

#ifdef USE_DAV_ALIASES
	root = new_node("multistatus");
	root->setNamespace(get_alias("DAV:"));
#else
	root = new_node("multistatus");
#endif

	doc->insert(root);

#if 1 /* Ending slash of host belongs to HOST */
	host = "http://" +
				request->get_header("Host") + 
				((request->get_port() != 80) ? (":"+request->get_port()) : "")+
				"/";
#else /* Ending slash of host belongs to FILENAME */
	host = "http://" +
				request->get_header("Host") + 
				((request->get_port() != 80) ? (":"+request->get_port()) : "");
#endif
	rel_path = uri->get_relative_path();
	abs_path = uri->get_absolute_path();

	props = get_dav_propfind(abs_path + uri->get_filename(), reqxml);

	/* TODO: SECURITY issue! Should we demand that user is authenticated? */
	if(has_access(request, response, READ_ACCESS) == FALSE) {
		set_prop_status(403, props);
		root->insert( create_response(host + rel_path, props, TRUE) );
		response->set_status(401);
	} else {
		root->insert( create_collection(host,
								rel_path + uri->get_filename(),
								abs_path + uri->get_filename(),
								props, 
								depth, 0) );
		response->set_status(207);
	}

#ifdef USE_DAV_ALIASES
	set_aliases(root);
#endif

	response->set_header("Content-Type", "text/xml; charset=\"utf-8\"");
	response->add_content( doc->xml() );
	response->update_content_length();

#ifdef DEBUG_DATA
	debug_info(request, response);
#endif

	if(!send(response, TRUE, TRUE)) {
		SYSLOG("WARNING: davtool->cmd_propfind() failed to send\n");
	}
	return TRUE;
}


int cmd_mkcol(object request, object response)
{
	int		status;
	mixed	*file;

	if(!USER()) {
		error("illegal call");
	}

	if(has_access(request, response, WRITE_ACCESS) == FALSE) {
		status = 401;
	} else {
		int i;
		string abs_path, parent_path, mkdir, *tmp;

		status = 201;
		abs_path = request->get_uri()->get_absolute_filename();
		file = ::get_dir(abs_path);
#if 0
		SYSLOG("mkcol: " + abs_path + "\n");
#endif
		if(sizeof(file[0])) {
			/* a file or directory with name already exists */
			status = 405;
		}

		tmp = explode(abs_path, "/");
		i = sizeof(tmp);
		/* Get last component of the path */
		while(i >= 1 && tmp[--i] == "") { /* ... */ }

		parent_path = "/" + implode(tmp[0..i-1], "/");
		mkdir = tmp[i];
		file = ::get_dir(parent_path + "/");
#if 0
		SYSLOG("---\n" +
				"   abs_path: " + abs_path + "\n" +
				"parent_path: " + parent_path + "\n" +
				"      mkdir: " + mkdir + "\n");
#endif
	
		if(request->get_content_length()) {
			/* we don't know what body contains or means */
			status = 415;
		}

		if(status == 201) {
			if((::make_dir(abs_path)) == 1) {
				SYSLOG("created dir\n");
			} else {
				SYSLOG("failed to create dir\n");
				/*status = 507; / * which really means insufficient storage */
				status = 409; 
			}
		}
	}

	response->set_status(status);
	response->update_content_length();
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: davtool->cmd_mkcol() failed to send\n");
	}

	return TRUE;
}


/* return mapping with ([ name : status ]) */
private mapping verify_access_file(string path, int acctype)
{
	if(has_access_file(path, acctype) == FALSE) {
		return ([ path : 401 ]);
	}
	return ([ path : 200 ]);
}


/* return mapping with ([ name : status ]) */
private mapping verify_access_collection(string path, int acctype)
{
	int		i, *sizes;
	mixed	*files;
	string	*names, name;
	mapping	ret;

	files = ::get_dir(path + "*");
#if 0
	SYSLOG("Getting dir: " + path + "*\n");
#endif
	names = files[0];

	if(sizeof(names) == 1) {
		return ([ path + "/" : 200 ]);
	}

	sizes = files[1];
	ret = ([ path : 200 ]);
	for(i = 0; i < sizeof(sizes); i++) {
		name = names[i];
		if(sizes[i] < 0) {
			if(has_access_file(path + name, acctype)) {
				ret += verify_access_collection(path + name + "/", acctype);
			} else {
				ret += ([ path + name : 401 ]);
			}
		} else {
			ret += verify_access_file(path + name, acctype);
		}
	}
	return ret;
}


int cmd_delete(object request, object response)
{
	int		status, c;
	mixed	*file;
	string	abs_path;

	if(!USER()) {
		error("illegal call");
	}

	if(has_access(request, response, WRITE_ACCESS) == FALSE) {
		status = 401;
	} else {
		int iscol, count;
		abs_path = request->get_uri()->get_absolute_filename();
		status = 200;

		file = ::get_dir(abs_path);
		count = sizeof(file[0]);
		c = abs_path[strlen(abs_path)-1];	/* last char */
		if(count) {
			iscol = (file[1][0] < 0) ? TRUE : FALSE;
		}

		/*if(count != 1 || (iscol && c != '/') || (!iscol && c == '/')) {*/
		if(count != 1) {
			/* - Non-existing file/directory or more than one match.
			 * - File/collection does/doesn't (respectively) end with slash.
			 */
			status = 404;
		}

		if(status == 200) {
			/* verify delete and then delete if all could be deleted */
			mapping rm;
			string *fns;
			int	*stats, i;

			rm = ([ ]);
			if(iscol) {
				rm = verify_access_collection(abs_path, WRITE_ACCESS);
			} else {
				rm = verify_access_file(abs_path, WRITE_ACCESS);
			}

			stats = map_values(rm);
			if(!is_member(401, stats)) {
				string fn;
				int ret;
				fns = map_indices(rm);

				for(i = sizeof(fns)-1; i >= 0; i--) {
					fn = fns[i];
					if(fn && fn[strlen(fn)-1] == '/') {
						infod->delete_collection(fn);
						ret = ::remove_dir(fn);
					} else {
						infod->delete_resource(fn);
						ret = ::remove_file(fn);
					}

					if(!ret) {
						SYSLOG("Failed to remove: " + fn + 
								", we can't recover atomically.\n");
					}
				}
			} else {
				/* TODO: Generate XML; we failed to delete (partially?) */
				error("TODO!\n");
			}
		}
	}

	response->set_status(status);
	response->update_content_length();

	/* TODO: In some cases we DO want to send body! */
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: davtool->cmd_delete() failed to send\n");
	}
	return TRUE;
}


int cmd_copy(object request, object response)
{
	int		status, overwrite, c;
	mixed	*file;
	string	src, dst;

	if(!USER()) {
		error("illegal call");
	}

	status = 204;
	src = request->get_uri()->get_absolute_filename();
	overwrite = get_overwrite(request);

	if(!(dst = get_absolute_destination(request))) {
		status = 400;
	} else if(has_access_file(src, READ_ACCESS) == FALSE ||
	   has_access_file(dst, WRITE_ACCESS) == FALSE) {
		status = 401;
		access_denied(request, response);
	} else if(status == 204) {
		int iscol, count;

		file = ::get_dir(src);
		count = sizeof(file[0]);
		c = src[strlen(src)-1];
		if(count) {
			iscol = (file[1][0] < 0) ? TRUE : FALSE;
		}

		if(count == 0 || (file_exists(dst) == FALSE && overwrite == FALSE)) {
			/* source doesn't exist or destination exists */
			SYSLOG("source or destination doesn't exist\n");
			status = 405;
		} else {
			/* do our thing (copy) */
			mapping cp;
			string *fns;
			int *stats, i;

			cp = ([ ]);
			src = ending_slash(src, iscol);

			if(iscol) {
				cp = verify_access_collection(src, READ_ACCESS);
				dst = ending_slash(dst, TRUE);
			} else {
				cp = verify_access_file(src, READ_ACCESS);
				dst = ending_slash(dst, FALSE);
			}

			stats = map_values(cp);
			if(!is_member(401, stats)) {
				string fn, curdst;
				int ret;
				fns = map_indices(cp);

				for(i = 0; i < sizeof(fns); i++) {
					fn = fns[i];
					curdst = dst + fn[strlen(src)..];

					if(fn && fn[strlen(fn)-1] == '/') {
						/* create dir */
						infod->copy_collection(fn, curdst);
						ret = ::make_dir(curdst);
					} else {
						/* copy file */
						infod->copy_resource(fn, curdst);
						ret = ::copy_file(fn, curdst);
					}

					if(!ret) {
						SYSLOG("Failed to copy: " + fn + 
								", we can't recover atomically.\n");
					}
				}
			} else {
				/* TODO: Generate XML; we failed to copy (partially?) */
				error("TODO!\n");
			}
		}
	}

	response->set_status(status);
	response->update_content_length();

	/* TODO: In some cases we DO want to send body! */
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: davtool->cmd_copy() failed to send\n");
	}
	return TRUE;		
}


int cmd_move(object request, object response)
{
	int     status, overwrite, c;
	mixed   *file;
	string  src, dst;

	if(!USER()) {
		error("illegal call");
	}

	status = 204;
	overwrite = get_overwrite(request);
	src = request->get_uri()->get_absolute_filename();
	if(!(dst = get_absolute_destination(request))) {
		status = 400;
	} else if(has_access_file(src, WRITE_ACCESS) == FALSE ||
			  has_access_file(dst, WRITE_ACCESS) == FALSE) {
		status = 401;
		access_denied(request, response);
	} else if(status == 204) {
		/* rename */
		if(!(::rename_file(src, dst)) || !(infod->move_resource(src, dst)) ) {
			SYSLOG("TODO: Rename failed, cannot recover atomically, " + 
					"what status-code do we want?\n");
		}
	}

	response->set_status(status);
	response->update_content_length();

	/* TODO: In some cases we DO want to send body! */
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: davtool->cmd_copy() failed to send\n");
	}
	return TRUE;
}


private object propertyupdate(string resource, string action, object node)
{
	int i, rm;
	object ret, *prop;

	prop = node->xpath("propertyupdate/" + action + "/prop/*");
	if(!prop || (sizeof(prop) == 1 && prop[0]->dummy())) {
		SYSLOG("No 'propertyupdate/" + action + "' node in root-node\n");
		return nil;
	}

	if(action == "remove") {
		rm = TRUE;
	}

	for(i = 0; i < sizeof(prop); i++) {
		object tmp;
		string name;
		mixed value;

		tmp = prop[i];
		name = tmp->getName();
		
		if(supported_property(name)) {
			/* supported, but can't be changed  TODO: status for this */
			continue;
		}

		if(rm) {
			infod->delete(resource, name);
			continue;
		}

		/* TODO: Need to set the xmlns with the properties! 4th arg in set! */
		if(tmp->getContentSize() > 1) {
			/* it has children, throw the entire node in there */
			infod->set(resource, name, tmp);
		} else {
			value = tmp->xpath(name)[0]->getValue(); 
			infod->set(resource, name, value);
		}
	}
	return ret;
}


int cmd_proppatch(object request, object response)
{
	object doc, reqxml;
	string resource;
	int status, i;

	if(!USER()) {
		error("illegal call");
	}

	/* Must have write-access to the resource */
	if(has_access(request, response, WRITE_ACCESS) == FALSE) {
		status = 401;
	} else {
		object tmp;
		mapping props;
		string href;
		int iscol;

		status = 204;
		resource = request->get_uri()->get_absolute_filename();
		iscol    = is_dir(resource);
		resource = ending_slash(resource, iscol);
		reqxml   = get_request_xml(request);
		tmp = propertyupdate(resource, "set", reqxml);
		if(tmp == nil) {
			tmp = propertyupdate(resource, "remove", reqxml);
		}

		if(tmp != nil) {
			/* we have errors (TODO: propertyupdate returns nothing useful) */
			status = 207;
			props  = ([ ]);
			href   = ending_slash(request->get_href(), iscol);
			doc = create_response(resource, props, iscol);
			doc->insert(tmp);
			response->set_header("Content-Type",
								"application/xml; charset=\"utf-8\"");
			response->add_content( doc->xml() );
		}
	}

	response->set_status(status);
	response->update_content_length();

	debug_info(request, response);

	/* TODO: In some cases we DO want to send body! */
	if(!send(response, TRUE, FALSE)) {
		SYSLOG("WARNING: davtool->cmd_proppatch() failed to send\n");
	}
	return TRUE;
}

#if 0
int cmd_put(object request, object response)
{
	error("TODO: Do we need to re-implement this one?\n");
}
#endif
