# include <config.h>
# include <levels.h>
# include <limits.h>
# include <privilege.h>
# include <masked_debug.h>
# undef status
# include <status.h>
# include <trace.h>
# include <net.h>
# include <object-db.h>

object *usr;		/* user array just before a swapout */

private int     querying_real_name; /* to avoid errors within errors within..*/
private mixed   *saved_error;       /* for re-throwing errors                */
private mapping compile_errors_map; /* Collecting errors per file, per line  */
private int do_statedump;

#ifdef __HTTPD__
private mapping binary;
#endif

/*
 * NAME:        simplify_path()
 * DESCRIPTION: simplify the path a bit.
 */
string
simplify_path(string path)
{
    string pre, post;

    path = "/" + path;
    while (sscanf(path, "%s//%s",  pre, post) == 2 ||
	   sscanf(path, "%s/./%s", pre, post) == 2) {
        path = pre  + "/" + post;
    }
    return path;
}

/*
 * NAME:        relative_path()
 * DESCRIPTION: determine the simplified path of a file relative to another.
 */
string
relative_path(string from, string to)
{
    int    i, len;
    string str;

    len = strlen(from);
    for (i = len - 1; i >= 0; i--) {
        if (from[i] == '/') {
	    from = from[..i - 1];
	    break;
	}
    }
    while (sscanf(to, "../%s", str)) {
        len = strlen(from);
	for (i = len - 1; i >= 0; i--) {
	    if (from[i] == '/') {
	        from = from[..i - 1];
		break;
	    }
	}
	to = str;
    }
    return simplify_path(from + "/" + to);
}

/*
 * NAME:        _F_compile_object()
 * DESCRIPTION: compile an object and register it if needed.
 */
object
_F_compile_object(string path, varargs string source)
{
    object obj, new;

    if (!PRIVILEGED() && previous_program() != DRIVER) {
	return 0;
    }
    obj = find_object(path);
    switch (path) {
    case AUTO:
    case DRIVER:
	/*
	 * Ok, nothing extra required.
	 */
        break;
    default:
	/*
	 * For any others we'll need the auto-object.
	 */
        if (!find_object(AUTO)) {
	    /*
	     * Otherwise it'll be compiled without being registered.
	     */
	    _F_compile_object(AUTO);
	}
	break;
    }
    rlimits (-1; -1) {
	if (!obj && find_object(path)) {
	    /*
	     * The _F_compile_object(AUTO) trigger-loaded the requested
	     * object, which was probably the global object, then.
	     */
	    new = find_object(path);
	} else {
	    if (source)
		new = ::compile_object(path, source);
	    else
		new = ::compile_object(path);
	    catch {
		SYS_OBJECTDB->add_object(new, !!obj);
	    }
	}
    }
    return new;
}

/*
 * NAME:        _F_clone_object()
 * DESCRIPTION: Notify driver-object that a clone has been made.
 */
void
_F_clone_object(object obj)
{
    if (!PRIVILEGED()) {
	return;
    }
    catch {
	SYS_OBJECTDB->add_clone(obj);
    }
}

/*
 * NAME:        _F_destruct_object()
 * DESCRIPTION: Notify driver-object that an object is about to be destructed
 */
void
_F_destruct_object(object obj)
{
    if (!PRIVILEGED()) {
	return;
    }
    if (sscanf(object_name(obj), "%*s#")) {
	catch {
	    SYS_OBJECTDB->del_clone(obj);
	}
    }
}

#ifdef __NET__
/*
 * NAME:	open_ports
 * DESCRIPTION:	open all ports listed in net.h
 */
void open_ports()
{
    /* It's not static, so that ports can be reopened from outside */
    int i;
    string *services;
    
    services = SERVICES;
    for (i = 0; i < sizeof(services); i++) {
	catch {
	    object service;

	    service = find_object(services[i]);
	    if (service == 0) {
	        service = _F_compile_object(services[i]);
	    }
	    service->port_up();
	}
    }
}
#endif

private void
add_timestamp(string file)
{
    write_file(file, ctime(time()) + " Timestamp [reboot]\n");
}

private void
add_timestamps()
{
    add_timestamp("/syslog/FORCE");
    add_timestamp("/syslog/NEWPLAYER");
    add_timestamp("/syslog/QUESTS");
    add_timestamp("/syslog/ENTER");
#ifdef __MASKED_DEBUG__
    add_timestamp("/syslog/MASKED_DEBUG");
#endif /* __MASKED_RESET_DEBUG__ */
}

/*
 * NAME:	initialize
 * DESCRIPTION:	called once at game startup
 */
static void initialize()
{
    int    i, sz, t, l;
    string *castles, castle, error;
    object typedb, *precomp;

    /*
     * Add markers in logfiles to indicate when we've rebooted.
     */
    add_timestamps();

    precomp = status()[ST_PRECOMPILED];
    for (i = 0; i < sizeof(precomp); i ++) {
       string obname;

       obname = object_name(precomp[i]);
       send_message("Precompiled: " + obname + "\n");
       switch (obname) {
       case DRIVER:
       case OBJ_OBJECTDB:
	   break;
       default:
	   catch {
	       SYS_OBJECTDB->add_object(precomp[i]);
	   }
       }
    }
    catch {
        SYS_OBJECTDB->add_object(this_object());
    }

    call_other(GLOBAL, "???");
    call_other(ACCESS, "???");

    send_message("\nLoading typecheck file config/loose_typed\n");
    catch("/sys/typedb"->load_list());

# ifndef TESTMUD
    send_message("\nLoading init file config/preload\n");
    castles = explode(read_file("/config/preload"), "\n");
# else
    send_message("\nLoading init file config/preload.testmud\n");
    castles = explode(read_file("/config/preload.testmud"), "\n");
# endif
    for (i = 0, sz = sizeof(castles); i < sz; i++) {
	if (strlen(castle = castles[i]) && castle[0] != '#') {
#ifdef __EPP__
	    if (sscanf(castle, "players/%*s")) {
		continue;
	    }
#endif
	    send_message("Preloading: " + castle);
	    t = time();

	    if ((l = strlen(castle)) > 1 && castle[l - 2 ..] == ".c") {
		castle = castle[.. l - 3];
	    }
	    rlimits (MAXDEPTH; MAXTICKS) {
		error = catch(call_other(castle, "???"));
	    }
	    if (error) {
		send_message(" - load error: " + error + "\n");
	    } else {
		send_message(" " + (time() - t) + ".0\n");
	    }
	}
    }
    send_message("Setting up ipc.\n");
#ifdef __NET__
    open_ports();
#endif

    call_out("status_check", SHUTINTERVAL);
    call_out("hourly_check", 3600);
#ifdef __EPP__
    shutdown();
#endif
}

void stderr(string str);

/*
 * NAME:        hourly_check()
 * DESCRIPTION: do hourly maintenance
 */
static void hourly_check()
{
    mixed *stat;
    int up_days, up_hours;

    call_out("hourly_check", 3600);

#ifdef __NET__
    open_ports();
#endif
    GLOBAL->regular_cleanup();
    
    stat = status();
    up_days = stat[ST_UPTIME] / (3600 * 24);
    up_hours = (stat[ST_UPTIME] / 3600) % 24;
    write_file("/syslog/STATUS", time() + ": Up " + up_days + "d " +
	       up_hours + "h: " + stat[ST_NOBJECTS] + " objects (" +
	       GLOBAL->query_all_objects() + " b / " +
	       GLOBAL->query_all_clones() + " c), " +
	       stat[ST_SWAPUSED] + " sectors.\n");

}

/*
 * NAME:        status_check()
 * DESCRIPTION: check whether we will need a reboot
 */
static void status_check()
{
    int i, j;
    mixed *stat;
    object shut, *u;

    stat = status();
    i = 100 * stat[ST_SWAPUSED] / stat[ST_SWAPSIZE];
    j = stat[ST_OTABSIZE] - stat[ST_NOBJECTS];

    if (i > SWAP_SHUT_PCT) {
	stderr("Over " + SWAP_SHUT_PCT + "% swap sectors in use.\n");
    } else if (j < OBJ_SHUT_RESV) {
	stderr("Less than " + OBJ_SHUT_RESV + " objects left.\n");
    } else {
	call_out("status_check", SHUTINTERVAL);
	return;
    }

    call_other(SHUT, "???");
    if (!(shut = find_object(SHUT))) {
	for (i = sizeof(u = users()); --i >= 0;) {
	    u[i]->catch_tell("*** Universe meltdown ***\n");
	}

	shutdown();
	return;
    }

    for (i = sizeof(u = users()); --i >= 0;) {
	u[i]->catch_tell("Armageddon shouts: The end of the universe is " +
			 "nigh!\n");
    }

    GLOBAL->set_this_player(shut);
    shut->shut(10);
}

/*
 * NAME:        set_users()
 * DESCRIPTION: keep a copy of the users array
 */
void set_users()
{
    if (PRIVILEGED()) {
	usr = users();
    }
}

/*
 * NAME:        restored()
 * DESCRIPTION: re-initialize the system after a restore
 */
static void restored()
{
    int i;

    do_statedump = 0;
    if (usr) {
	for (i = sizeof(usr); --i >= 0;) {
	    catch {
                usr[i]->disconnect_for_statedump();
            }
	}
	usr = 0;
    }

    send_message("\nState restored.\nSetting up ipc.\n");
#ifdef __NET__
    open_ports();
#endif
}
 
/*
 * NAME:	path_read()
 * DESCRIPTION:	handle an editor read path
 */
string path_read(string path)
{
    path = find_object(ACCESS)->valid_read(path, previous_object());
    return path && (this_user()->query_player()->query_wizard() ||
		    this_user()->verify_file(path)) ? path : 0;
}

/*
 * NAME:	path_write()
 * DESCRIPTION:	handle an editor write path
 */
string path_write(string path)
{
    path = find_object(ACCESS)->valid_write(path, previous_object());
    return path && (this_user()->query_player()->query_wizard() ||
		    this_user()->verify_file(path)) ? path : 0;
}

/*
 * NAME:	path_object()
 * DESCRIPTION:	translate an object path
 */
string path_object(string path)
{
    int l;
 
    l = strlen(path);
    if (l > 3 && path[l - 2..] == ".c") {
	return path[.. l - 3];
    }
    return simplify_path(path);
}

/*
 * NAME:        call_object()
 * DESCRIPTION: return the object to be called
 */
static object call_object(string path)
{
    int l;
    object obj;

    l = strlen(path);
    if (l > 3 && path[l - 2..] == ".c") {
	path = path[.. l - 3];
    }
    if (!(obj = find_object(path))) {
        return _F_compile_object(simplify_path(path));
    }
    return obj;
}

static object inherit_program(string file, string path, int private_flag)
{
    object ob;

    if (strlen(path) && path[0] == '.') {
	path = relative_path(file, path);
    }
    ob = call_object(path);
    if (ob && private_flag) {
	write_file("/syslog/PRIVATE",
		   ctime(time()) + ": private inherit " + object_name(ob) +
		   " from " + file + "\n");
    }
    return ob;
}

/*
 * NAME:	path_include()
 * DESCRIPTION:	translate an include path
 */
string path_include(string file, string path)
{
    return path[0] == '/' ? path : relative_path(file, path);
}

/*
 * NAME:        recompile()
 * DESCRIPTION: (not) used to recompile objects
 */
static void recompile(object ob)
{
/* Zellski 980713 */
# if 0
    GLOBAL->request_destruct(ob);
# endif
}

/*
 * NAME:	telnet_connect()
 * DESCRIPTION:	return a player object
 */
static object telnet_connect()
{
    object user;

    GLOBAL->set_this_player(0);
    user = find_object(USER);
    if (user == 0) {
        user = _F_compile_object(USER);
    }
    return clone_object(user);
}


#ifdef __HTTPD__
/* This does not belong in driver.c, but it works for now */
void set_binary_manager(int port, object obj)
{
	/* TODO: need SECURITY! NOW! */
	if(!binary) 
		binary = ([ ]);
	binary[port] = obj;
}
#endif

/*
 * NAME:	binary_connect()
 * DESCRIPTION:	return a player object
 */
#ifdef __HTTPD__
static object binary_connect(int port)
#else
static object binary_connect()
#endif
{
    object user;

    GLOBAL->set_this_player(0);
    user = find_object(BINARY);
    if (user == 0) {
        user = _F_compile_object(BINARY);
    }

#ifdef __HTTPD__
	/* Hack to get httpd to work */
	if(!binary || !binary[port]) {
	    return clone_object(user);
	} else {
		return binary[port]->select("");
	}
#else
    return clone_object(user);
#endif
}


/*
 * Align the int/string by appending spaces.
 */
private string
lalign(mixed s, int width)
{
    int len;
    string str;

    s = (string)s;
    width -= strlen(s);
    if (width <= 0) {
        return s;
    }
    str = "                                                                ";
    len = 64;
    while (width > len) {
        len <<= 1;
        str += str;
    }
    return s + str[..width - 1];
}

/*
 * Align the int/string by inserting spaces.
 */
private string
ralign(mixed s, int width)
{
    int len;
    string str;

    s = (string)s;
    width -= strlen(s);
    if (width <= 0) {
        return s;
    }
    str = "                                                                ";
    len = 64;
    while (width > len) {
        len <<= 1;
        str += str;
    }
    return str[..width - 1] + s;
}

private string
format_path(string path)
{
    return sscanf(path, "/players/%*s/") ? "~" + path[9..] : path;
}

/*
 * Add a file/line/error combination to the temporary storage.
 */
static void
add_compile_error(string file, int line, string err)
{
    mixed *data;

    if (!compile_errors_map) {
	compile_errors_map = ([ ]);
    }
    data = compile_errors_map[file];
    if (data) {
	string *list;

	list = data[1][line];
	if (list) {
	    data[1][line] = list + ({ err });
	} else {
	    data[1][line] = ({ err });
	}
    } else {
	compile_errors_map[file] = ({ time(), ([ line: ({ err }) ]) });
    }
}

/*
 * NAME:        compile_error()
 * DESCRIPTION: deal with a compilation error
 */
void compile_error(string file, int line, string err)
{
#if 1
    add_compile_error(file, line, err);
#else
    string wizard;

    err = file + ", " + line + ": " + err + "\n";
    send_message(err);
    if (sscanf(file, "/players/%s/", wizard) != 0) {
        write_file("/log/errors/" + wizard, err);
    } else {
        write_file("/syslog/Backbone.err", err);
    }
#endif
}

private string
format_compile_error(string file, int timestamp, mapping lines)
{
    int    i, sz, *linenrs;
    string str, **errors;

    str = ctime(timestamp) + ": Compile error(s) in " + file + "\n";
    sz = map_sizeof(lines);
    linenrs = map_indices(lines);
    errors = map_values(lines);
    for (i = 0; i < sz; i++) {
	string linenr;

	linenr = ralign(linenrs[i], 5) + "   ";
	str += linenr + implode(errors[i], "\n" + linenr) + "\n";
    }
    return str;
}

static void
flush_compile_errors()
{
    if (compile_errors_map) {
        int sz;

	sz = map_sizeof(compile_errors_map);
	if (sz) {
	    int    i;
	    string *files;
	    mixed  **data;

	    files = map_indices(compile_errors_map);
	    data  = map_values(compile_errors_map);
	    for (i = 0; i < sz; i++) {
		string file, owner, rest, text;

		file = files[i];
		if (sscanf(file, "/players/%s/%s", owner, rest) == 2) {
		    file = "~"+owner+"/"+rest;
		    owner = "/log/errors/"+owner;
		} else {
		    owner = "/syslog/Backbone.err";
		}
		text = format_compile_error(file,
					    data[i][0],
					    data[i][1]) + "\n";
		write_file(owner, text);
		send_message(text);
	    }
	}
    }
    compile_errors_map = 0;
}

/*
 * NAME:        atomic_error()
 * DESCRIPTION: log an atomic error
 */
static void atomic_error(string str, int atom, int ticks)
{
    int    i, j, sz, linenr, maxlen;
    string progname, objname, function, result,
           logname, **lines, *log, player_name, castle;
    object player;
    mixed  **trace;

    trace = call_trace();
    if (!(sz = sizeof(trace) - 1)) {
        return;
    }
    lines = allocate(sz * 2);
    for (i = j = maxlen = 0; i < sz; i++) {
        string line, function, last_obj, last_prog;
        mixed  *elt;

        elt = trace[i];
        objname  = elt[TRACE_OBJNAME];
        progname = elt[TRACE_PROGNAME];
        function = elt[TRACE_FUNCTION];
        linenr   = elt[TRACE_LINE];
        if (objname != last_obj) {
            object ob;

            line = "        " + format_path(objname);
            if (!querying_real_name && (ob = find_object(objname))) {
	        querying_real_name = 1;
	        catch {
		    string name;

		    name = ob->query_real_name();
		    if (name) {
		        line += " '" + name + "'";
		    }
		}
		querying_real_name = 0;
            }
            lines[j++] = ({ line, 0 });
            last_obj = last_prog = objname;
        }
        sscanf(progname, "/players/%s/", logname);
        if ((progname == AUTO || progname == DRIVER) &&
	    strlen(function) > 3) {
            switch (function[..2]) {
            case "bad":
                if (i == sz - 1) {
                    progname = trace[i - 1][TRACE_PROGNAME];
                    linenr   = trace[i - 1][TRACE_LINE];
                    continue;
                }
                break;
            case "_F_":
            case "_Q_":
                if (i != sz - 1) {
                    continue;
                }
                break;
            default:
                break;
            }
        }
        line = (linenr ? ralign(linenr, 5) : "     ") +
               (i + 1 == atom ? " > " : "   ") + "   " +
               (progname == last_prog ? "-" : format_path(progname));
        last_prog = progname;
        if (strlen(line) > maxlen) {
            maxlen = strlen(line);
        }
        lines[j++] = ({ line, function });
    }
    log = allocate(j);
    for (i = 0; i < j; i++) {
        if (lines[i][1]) {
            log[i] = lalign(lines[i][0], maxlen) + "   " + lines[i][1];
        } else {
            log[i] = lines[i][0];
        }
    }
    result = ctime(time()) + ": " + str + " [atomic]\n" +
             implode(log, "\n") + "\n\n";

    send_message(result);
}

/*
 * NAME:	runtime_error()
 * DESCRIPTION:	log a runtime error
 */
static void runtime_error(string str, int caught, int ticks)
{
    int    i, j, sz, linenr, maxlen;
    string progname, objname, function, result,
           logname, **lines, *log, player_name, castle;
    object player;
    mixed  **trace;

    /*
     * Flush the compile errors to their respective file(s) before the runtime
     * error (caught or not) is logged.
     */
    flush_compile_errors();

    if (str == "*") {
	if (previous_program() != AUTO) {
	    /*
	     * Only allow auto-object to trigger this feature.
	     */
	    trace = call_trace();
	} else {
	    if (caught) {
		/* Nested catch, keep saved trace a bit longer. */
		return;
	    } else {
		if (saved_error) {
		    str    = saved_error[0] + " (rethrown)";
		    ticks  = saved_error[1];
		    trace  = saved_error[2];
		    saved_error = 0;
		} else {
		    /*
		     * Silly wizard, using rethrow while no error was caught?
		     */
		    trace = call_trace();
		    str = "Rethrow attempt without saved error available";
		}
	    }
	}
    } else {
	trace = call_trace();
	if (caught) {
	    /* Save trace for possible later use. */
	    saved_error = ({ str, ticks, call_trace() });
	} else {
	    saved_error = 0;
	}
    }
    if (!(sz = sizeof(trace) - 1)) {
        return;
    }
    lines = allocate(sz * 2);
    for (i = j = maxlen = 0; i < sz; i++) {
        string line, function, last_obj, last_prog;
        mixed  *elt;

        elt = trace[i];
        objname  = elt[TRACE_OBJNAME];
        progname = elt[TRACE_PROGNAME];
        function = elt[TRACE_FUNCTION];
        linenr   = elt[TRACE_LINE];
        if (objname != last_obj) {
            object ob;

            line = "        " + format_path(objname);
            if (!querying_real_name && (ob = find_object(objname))) {
	        querying_real_name = 1;
	        catch {
		    string name;

		    name = ob->query_real_name();
		    if (name) {
		        line += " '" + name + "'";
		    }
		}
		querying_real_name = 0;
            }
            lines[j++] = ({ line, 0 });
            last_obj = last_prog = objname;
        }
        sscanf(progname, "/players/%s/", logname);
        if ((progname == AUTO || progname == DRIVER) &&
	    strlen(function) > 3) {
            switch (function[..2]) {
            case "bad":
                if (i == sz - 1) {
                    progname = trace[i - 1][TRACE_PROGNAME];
                    linenr   = trace[i - 1][TRACE_LINE];
                    continue;
                }
                break;
            case "_F_":
            case "_Q_":
                if (i != sz - 1) {
                    continue;
                }
                break;
            default:
                break;
            }
        }
        line = (linenr ? ralign(linenr, 5) : "     ") +
               (i + 1 == caught ? " > " : "   ") + "   " +
               (progname == last_prog ? "-" : format_path(progname));
        last_prog = progname;
        if (strlen(line) > maxlen) {
            maxlen = strlen(line);
        }
        lines[j++] = ({ line, function });
    }
    log = allocate(j);
    for (i = 0; i < j; i++) {
        if (lines[i][1]) {
            log[i] = lalign(lines[i][0], maxlen) + "   " + lines[i][1];
        } else {
            log[i] = lines[i][0];
        }
    }
    result = ctime(time()) + ": " + str + (caught ? " [caught]\n" : "\n") +
             implode(log, "\n") + "\n\n";
    if (caught) {
        write_file("/syslog/CAUGHT", result);
    } else {
        send_message(result);
    }
    if (!caught) {
        player = GLOBAL->query_this_player();
        if (player &&
            function_object("valid_interactive", player) == INTERACTIVE &&
            player->_Q_user()) {
            if (!player->query_wizard()) {
                player->catch_tell("Your sensitive mind notices an error " +
                                   "in the fabric of space.\n");
            } else {
                player->catch_tell(str + "\nObject: " + objname + 
                                   ", program: " + progname +
                                   ", line " + linenr + "\n");
            }
        }
    }
    if (!logname) {
        write_file("/syslog/Backbone.err", result);
    } else if (find_object("/players/" + logname + "/castle")) {
        sz++;
        for (i = 0; i < sz; i++) {
            trace[i] = trace[i][..4];
        }
        rlimits (MAXDEPTH; MAXTICKS) {
            if (!("/players/" + logname + "/castle")->log_error(str, caught,
                                                                trace, log)) {
                write_file("/log/errors/" + logname, result);
            }
        }
    } else if (!caught) {
        write_file("/log/errors/" + logname, result);
    }
}

static void remove_program(string obname, int timestamp, int index)
{
     SYS_OBJECTDB->del_object(obname, timestamp, index);
}

/*
 * NAME:        set_do_statedump()
 * DESCRIPTION: indicate whether or not to make a statedump before shutdown
 */
void set_do_statedump()
{
    if (object_name(previous_object()) == "/obj/shut") {
        do_statedump = 1;
    }
}

/*
 * NAME:        start_shut_down()
 * DESCRIPTION: shut down the game
 */
void start_shut_down(string name)
{
    if (PRIVILEGED()) {
        object *u, player;
        int i;

        if (do_statedump) {
            send_message("SHUTDOWN (statedump) :: Started by " + name + "\n");
            for (i = sizeof(u = users()); --i >= 0;) {
                u[i]->catch_tell("*** Igormud closing down for maintenance ***\n");
            }
            for (i = sizeof(u); --i >= 0;) {
                if ((player = u[i]) && player->query_player()) {
                    player->disconnect_for_statedump();
                }
            }

#ifdef __NET__
            for (i = sizeof(u = ports()); --i >= 0;) {
                catch {
                    u[i]->port_down();
                }
            }
#endif

            GLOBAL->set_this_player(0);
            send_message("Shutdown (statedump).\n");
            dump_state();
        } else {
            send_message("SHUTDOWN :: Started by " + name + "\n");
            for (i = sizeof(u = users()); --i >= 0;) {
                u[i]->catch_tell("*** Igormud shutting down immediately ***\n");
            }
            for (i = sizeof(u); --i >= 0;) {
                if ((player = u[i]) && (player = player->query_player())) {
                    GLOBAL->set_this_player(player);
                    catch {
                        player->save_me(1);
                    }
                }
            }

            GLOBAL->set_this_player(0);
            SIGNALS->signal("SHUTDOWN", 0, "shutdown");
            send_message("Shutdown.\n");
        }
        shutdown();
    }
}

/*
 * NAME:        start_swap_out()
 * DESCRIPTION: start writing everything to the swap-device
 */
void start_swap_out()
{
    if (PRIVILEGED()) {
        send_message(ctime(time()) + ": swapout() from " +
		     object_name(previous_object()) + ".\n");
	call_out("finish_swap_out", 0);
	swapout();
    }
}

static void finish_swap_out()
{
    send_message(ctime(time()) + ": swapout() done.\n");
}

/*
 * Debugging tool
 */
void stderr(string str)
{
    send_message("Stderr: " + str);
}

static int compile_rlimits(string objname)
{
   switch(objname) {
   case AUTO:
   case USER:
   case USER + "-loop":
   case "/sys/typedb":
   case "/obj/living":
   case "/kernel/sys/control":
      return 1;
   }
   return 0;
}

static int runtime_rlimits(object obj, int depth, int ticks)
{
    mixed *info;

    info = status();
    return (depth >= 0 && ticks >= 0 && depth < info[ST_STACKDEPTH] &&
            ticks < info[ST_TICKS] - 40);
}

static void interrupt()
{
    usr = users();
    dump_state();
    shutdown();
}

static int compile_untypechecked(string program, string function) {
    return "/sys/typedb"->query_allowed(program);
}
