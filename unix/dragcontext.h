#ifndef __UI_DRAGCONTEXT_H__
#define __UI_DRAGCONTEXT_H__

struct uiDragContext {
	GtkWidget *widget;
	GdkDragContext *context;
	gint x;
	gint y;
	guint time;
	uiDragDestination *dd;
};

typedef struct uiprivDragDestination uiprivDragDestination;
struct uiprivDragDestination {
	gboolean dragEnter;
	gboolean dataReceived;
	guint info;
	uiDragType requestedType;
	uiDragData *data;
};

#endif

