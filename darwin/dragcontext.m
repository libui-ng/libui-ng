#import "uipriv_darwin.h"

void uiDragContextPosition(uiDragContext *dc, int *x, int *y)
{
	NSPoint pt = [dc->info draggingLocation];
	*x = pt.x;
	*y = dc->view.frame.size.height - pt.y;
}

int uiDragContextDragTypes(uiDragContext *dc)
{
	int types = 0;
	NSPasteboard *pboard = [dc->info draggingPasteboard];

	if ([[pboard types] containsObject:NSStringPboardType])
		types |= uiDragTypeText;
	if ([[pboard types] containsObject:NSFilenamesPboardType])
		types |= uiDragTypeURIs;

	return types;
}

int uiDragContextDragOperations(uiDragContext *dc)
{
	int ops = uiDragOperationNone;
	NSDragOperation mask = [dc->info draggingSourceOperationMask];

	if (mask & NSDragOperationCopy)
		ops |= uiDragOperationCopy;
	if (mask & NSDragOperationLink)
		ops |= uiDragOperationLink;
	if (mask & NSDragOperationMove)
		ops |= uiDragOperationMove;

	return ops;
}

uiDragData* uiDragContextDragData(uiDragContext *dc, uiDragType type)
{
	uiDragData *d = NULL;
	NSPasteboard *pboard = [dc->info draggingPasteboard];

	switch (type) {
	case uiDragTypeURIs:
		{
			if ([[pboard types] containsObject:NSFilenamesPboardType]) {
				int i;
				NSArray *urls = [pboard propertyListForType:NSFilenamesPboardType];

				// TODO inform about failure?
				if (urls == nil)
					return NULL;

				d = uiprivNew(uiDragData);
				d->type = uiDragTypeURIs;
				d->data.URIs.numURIs = [urls count];
				d->data.URIs.URIs = uiprivAlloc(d->data.URIs.numURIs * sizeof(*d->data.URIs.URIs), "uiDrag    DropData->data.URIs.URIs");
				for (i = 0; i < d->data.URIs.numURIs; ++i)
					d->data.URIs.URIs[i] = uiDarwinNSStringToText(urls[i]);
			}
		}
		break;
	case uiDragTypeText:
		{
			if ([[pboard types] containsObject:NSStringPboardType]) {
				NSString *text = [pboard stringForType:NSStringPboardType];

				// TODO inform about failure?
				if (text == nil)
					return NULL;

				d = uiprivNew(uiDragData);
				d->type = uiDragTypeText;
				d->data.text = uiDarwinNSStringToText(text);
			}
		}
		break;
	}
	return d;
}

