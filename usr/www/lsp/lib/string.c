# define SPACE16    "                "
/*
 * NAME:    ralign()
 * DESCRIPTION: return a number as a right-aligned string
 */
static string ralign(mixed num, int width)
{
    string str;

    str = SPACE16 + (string) num;
    return str[strlen(str) - width ..];
}

