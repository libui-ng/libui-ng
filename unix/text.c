// 9 april 2015
#include <strings.h>
#include "uipriv_unix.h"

char *uiUnixStrdupText(const char *t)
{
	return g_strdup(t);
}

void uiFreeText(char *t)
{
	g_free(t);
}

// TODO Not UTF-8 aware, ASCII only!
int uiprivStricmp(const char *a, const char *b)
{
	return strcasecmp(a, b);
}
