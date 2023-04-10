#include "uipriv_unix.h"
#include "dragcontext.h"

static int contextToDragTypeMask(GdkDragContext *context)
{
	GList *list;
	GdkAtom uri = gdk_atom_intern("text/uri-list", TRUE);
	GdkAtom text = gdk_atom_intern("text/plain;charset=utf-8", TRUE);
	int mask = 0;

	list = gdk_drag_context_list_targets(context);
	if (list == NULL)
		return 0;

	while (list) {
		if (list->data == text)
			mask |= uiDragTypeText;
		if (list->data == uri)
			mask |= uiDragTypeURIs;

		list = list->next;
	}
	g_list_free(list);

	return mask;
}

static int gdkActionMaskToDragOperationMask(GdkDragAction as)
{
	int ops = uiDragOperationNone;

	if (as & GDK_ACTION_COPY)
		ops |= uiDragOperationCopy;
	if (as & GDK_ACTION_LINK)
		ops |= uiDragOperationLink;
	if (as & GDK_ACTION_MOVE)
		ops |= uiDragOperationMove;

	return ops;
}

void uiDragContextPosition(uiDragContext *dc, int *x, int *y)
{
	*x = dc->x;
	*y = dc->y;
}

int uiDragContextDragTypes(uiDragContext *dc)
{
	return contextToDragTypeMask(dc->context);
}

int uiDragContextDragOperations(uiDragContext *dc)
{
	return gdkActionMaskToDragOperationMask(gdk_drag_context_get_actions(dc->context));
}

uiDragData* uiDragContextDragData(uiDragContext *dc, uiDragType type)
{
	uiprivDragDestination *priv = dc->dd->priv;
	GdkAtom atom;
	int mask;

	mask = contextToDragTypeMask(dc->context);
	if (!(mask & type) || !(dc->dd->typeMask & type)) {
		uiprivUserBug("Requested uiDragType (%d) is not supported by the context", type);
		return NULL;
	}

	switch (type) {
		case uiDragTypeText:
			atom = gdk_atom_intern("text/plain;charset=utf-8", TRUE);
			break;
		case uiDragTypeURIs:
			atom = gdk_atom_intern("text/uri-list", TRUE);
			break;
		default:
			return NULL;
	}

	priv->dataReceived = FALSE;
	priv->requestedType = type;
	gtk_drag_get_data(dc->widget, dc->context, atom, dc->time);
	while (!priv->dataReceived)
		if (!uiMainStep(1))
			break;

	return priv->data;
}

