#include "uipriv_unix.h"

void uiFreeDragData(uiDragData *d)
{
	int i;

	switch (d->type) {
		case uiDragTypeText:
			g_free(d->data.text);
			break;
		case uiDragTypeURIs:
			for (i = 0; i < d->data.URIs.numURIs; ++i)
				g_free(d->data.URIs.URIs[i]);
			if (d->data.URIs.URIs != NULL)
				uiprivFree(d->data.URIs.URIs);
			break;
	}
	uiprivFree(d);
}

