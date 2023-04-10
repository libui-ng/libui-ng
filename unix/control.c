// 16 august 2015
#include "uipriv_unix.h"
#include "dragcontext.h"

void uiUnixControlSetContainer(uiUnixControl *c, GtkContainer *container, gboolean remove)
{
	(*(c->SetContainer))(c, container, remove);
}

#define uiUnixControlSignature 0x556E6978

uiUnixControl *uiUnixAllocControl(size_t n, uint32_t typesig, const char *typenamestr)
{
	return uiUnixControl(uiAllocControl(n, uiUnixControlSignature, typesig, typenamestr));
}

static gint dragOperationToGdkAction(uiDragOperation op)
{
	switch (op) {
		case uiDragOperationNone:
			return 0;
		case uiDragOperationCopy:
			return GDK_ACTION_COPY;
		case uiDragOperationLink:
			return GDK_ACTION_LINK;
		case uiDragOperationMove:
			return GDK_ACTION_MOVE;
	}
	return 0;
}

static gboolean onDragMotion(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer userdata)
{
	uiControl *c = (uiControl*)userdata;
	uiprivDragDestination *priv = c->dragDest->priv;
	uiDragContext dc = { .widget = widget, .context = context, .x = x, .y = y, .time = time, .dd = c->dragDest };

	if (!priv->dragEnter) {
		priv->dragEnter = TRUE;
		c->dragDest->op = uiDragOperationNone;
		c->dragDest->op = c->dragDest->onEnter(c->dragDest, &dc, c->dragDest->onEnterData);
	} else {
		c->dragDest->op = c->dragDest->onMove(c->dragDest, &dc, c->dragDest->onMoveData);
	}

	gdk_drag_status(context, dragOperationToGdkAction(c->dragDest->op), time);
	return TRUE;
}

static gboolean onDragDrop(GtkWidget* widget, GdkDragContext* context, gint x, gint y, guint time, gpointer userdata)
{
	int success;
	uiControl *c = (uiControl*)userdata;
	uiDragContext dc = { .widget = widget, .context = context, .x = x, .y = y, .time = time, .dd = c->dragDest };

	success = c->dragDest->onDrop(c->dragDest, &dc, c->dragDest->onDropData);
	if (!success)
		return FALSE;

	switch (c->dragDest->op) {
		case uiDragOperationNone:
			gtk_drag_finish(context, FALSE, FALSE, time);
			break;
		case uiDragOperationLink:
		case uiDragOperationCopy:
			gtk_drag_finish(context, TRUE, FALSE, time);
			break;
		case uiDragOperationMove:
			gtk_drag_finish(context, TRUE, TRUE, time);
			break;
	}

	return TRUE;
}

void onDragLeave(GtkWidget *widget, GdkDragContext *context, guint time, gpointer userdata)
{
	uiControl *c = (uiControl*)userdata;
	uiprivDragDestination *priv = c->dragDest->priv;
	GdkEvent *event;
	GdkEventType type;

	priv->dragEnter = FALSE;

	event = gtk_get_current_event();
	if (event != NULL) {
		type = event->type;
		gdk_event_free(event);
		// Prevent onExit() if we are in the context of an impending
		// drop. According to the GTK3 source "drag-leave" gets signalled
		// when event->type equals GDK_DROP_START [gtkdnd.c].
		if (type == GDK_DROP_START)
			return;
	}
	c->dragDest->onExit(c->dragDest, c->dragDest->onExitData);
}

static void onDragDataReceived(GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* data, guint info, guint time, gpointer userdata)
{
	uiControl *c = (uiControl*)userdata;
	uiprivDragDestination *priv = c->dragDest->priv;
	uiDragData *d = NULL;

	priv->data = NULL;

	if (info != priv->requestedType)
		return;

	priv->dataReceived = TRUE;

	if (gtk_selection_data_get_length(data) < 0)
		return;

	if (info == uiDragTypeText) {
		guchar *text;

		text = gtk_selection_data_get_text(data);
		if (text == NULL)
			return;

		d = uiprivNew(uiDragData);
		d->type = info;
		d->data.text = (char*)text;
	}
	else if (info == uiDragTypeURIs) {
		int i;
		gchar **uris;

		uris = gtk_selection_data_get_uris(data);
		if (uris == NULL)
			return;

		d = uiprivNew(uiDragData);
		d->type = info;
		d->data.URIs.numURIs = 0;
		for (i = 0; uris[i] != NULL; ++i)
			++d->data.URIs.numURIs;

		if (d->data.URIs.numURIs == 0)
			d->data.URIs.URIs = NULL;
		else
			d->data.URIs.URIs = uiprivAlloc(d->data.URIs.numURIs * sizeof(*d->data.URIs.URIs), "uiDragData->data.URIs.URIs");

		for (i = 0; uris[i] != NULL; ++i) {
			d->data.URIs.URIs[i] = g_filename_from_uri(uris[i], NULL, NULL);
		}
		g_strfreev(uris);
	}

	priv->data = d;
}

void uiprivControlDestroyDragDestination(uiControl *c)
{
	GtkWidget *w = (GtkWidget*)uiControlHandle(c);

	// TODO g_signal_disconnect to support a new register on the same control
	gtk_drag_dest_unset(w);

	uiprivFree(c->dragDest->priv);
	uiprivFree(c->dragDest);
	c->dragDest = NULL;
}

void uiControlRegisterDragDestination(uiControl *c, uiDragDestination *dd)
{
	uiprivDragDestination *priv;
	GArray *targets;
	GtkWidget *w = (GtkWidget*)uiControlHandle(c);
	GdkDragAction actions = GDK_ACTION_COPY | GDK_ACTION_LINK | GDK_ACTION_MOVE;

	if (c->dragDest == NULL) {
		g_signal_connect(w, "drag-motion", G_CALLBACK(onDragMotion), c);
		g_signal_connect(w, "drag-drop", G_CALLBACK(onDragDrop), c);
		g_signal_connect(w, "drag-data-received", G_CALLBACK(onDragDataReceived), c);
		g_signal_connect(w, "drag-leave", G_CALLBACK(onDragLeave), c);
	} else {
		uiprivControlDestroyDragDestination(c);
	}

	if (dd == NULL)
		return;

	priv = uiprivAlloc(sizeof(uiprivDragDestination), "uiDragDestination->priv");
	priv->dragEnter = FALSE;
	priv->dataReceived = FALSE;
	priv->info = 0;
	priv->requestedType = 0;
	priv->data = NULL;
	dd->priv = priv;

	c->dragDest = dd;

	targets = g_array_new(0, 0, sizeof(GtkTargetEntry));
	if (dd->typeMask & uiDragTypeURIs) {
		GtkTargetEntry entryURIs = {"text/uri-list", GTK_TARGET_OTHER_APP, uiDragTypeURIs};
		g_array_append_vals(targets, &entryURIs, 1);
	}
	if (dd->typeMask & uiDragTypeText) {
		GtkTargetEntry entryText = {"text/plain;charset=utf-8", GTK_TARGET_OTHER_APP, uiDragTypeText};
		g_array_append_vals(targets, &entryText, 1);
	}

	gtk_drag_dest_set(w, 0, (GtkTargetEntry*)targets->data, targets->len, actions);
	g_array_free(targets, 1);
}

