#import "uipriv_darwin.h"

void uiFreeDragData(uiDragData *d)
{
	int i;

	switch (d->type) {
		case uiDragTypeText:
			uiFreeText(d->data.text);
			break;
		case uiDragTypeURIs:
			for (i = 0; i < d->data.URIs.numURIs; ++i)
				uiFreeText(d->data.URIs.URIs[i]);
			if (d->data.URIs.URIs != NULL)
				uiprivFree(d->data.URIs.URIs);
			break;
	}
	uiprivFree(d);
}

