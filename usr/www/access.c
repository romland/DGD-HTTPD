/*
 * Description: Enable anonymous access to this directory.
 * Version:     $Id$
 * License:     see Jorinde/doc/License
 */
# include "./jorinde.h"
#define SECURE() if(\
previous_program() != JORINDE_HTTPD+"lib/file" && \
previous_program() != JORINDE_HTTPD+"mh/lsp/compile" && \
previous_program() != JORINDE_HTTPD+"mh/lsp/sys/htmlerrord" \
) error("illegal call");

string _HTTPD_read_file(string path, varargs int offset, int size)
{
	SECURE();
#ifdef __IGOR__
	return :: read_bytes(path, offset, size);
#else
	return :: read_file(path, offset, size);
#endif
}

void _HTTPD_compile_object(string source, string path)
{
	SECURE();
	::compile_object(source, path);
}

int _HTTPD_write_file(string path, string content, varargs int offset)
{
	SECURE();
#ifdef __IGOR__
	return ::write_bytes(path, (offset ? offset : 0), content);
#else
	return ::write_file(path, content, offset);
#endif
}

mixed *_HTTPD_get_dir(string path)
{
	SECURE();
#ifdef __IGOR__
	return ::get_dir_info(path);
#else
	return ::get_dir(path);
#endif
}

int _HTTPD_make_dir(string path)
{
	SECURE();
	return ::make_dir(path);
}

int _HTTPD_remove_file(string path)
{
	SECURE();
	return ::remove_file(path);
}

int _HTTPD_remove_dir(string path)
{
	SECURE();
	return ::remove_dir(path);
}

int _HTTPD_rename_file(string from, string to)
{
	SECURE();
	return ::rename_file(from, to);
}
