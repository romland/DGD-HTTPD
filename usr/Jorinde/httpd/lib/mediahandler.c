# include "../include/www.h"

Application app;

void create(varargs int clone)
{
	app = previous_object();
}

nomask Application get_application()
{
	return app;
}

int get_need_content(Request request, Response response)
{
	return TRUE;
}

int get_supports(string filesuffix)
{
	return FALSE;
}

int handle(Request request, Response response)
{
	return TRUE;
}

