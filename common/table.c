#include "../ui.h"
#include "uipriv.h"

void uiFreeTableSelection(uiTableSelection *s)
{
	if (s->Rows != NULL)
		uiprivFree(s->Rows);
	uiprivFree(s);
}

