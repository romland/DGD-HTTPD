# include "../include/www.h"

object app;

void create(varargs int clone)
{
	app = previous_object();
}

nomask object get_application()
{
	return app;
}

int get_need_content(object request, object response)
{
	return TRUE;
}

int get_supports(string filesuffix)
{
	return FALSE;
}

int handle(object request, object response)
{
	return TRUE;
}

