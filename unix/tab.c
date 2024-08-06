// 11 june 2015
#include "uipriv_unix.h"

struct uiTab {
	uiUnixControl c;

	GtkWidget *widget;
	GtkContainer *container;
	GtkNotebook *notebook;

	void (*onSelected)(uiTab *, void *);
	void *onSelectedData;
	GArray *pages;				// []*uiprivChild
};

uiUnixControlAllDefaultsExceptDestroy(uiTab)

static void defaultOnSelected(uiTab *t, void *data)
{
	// do nothing
}

static void uiTabDestroy(uiControl *c)
{
	uiTab *t = uiTab(c);
	guint i;
	uiprivChild *page;

	uiTabOnSelected(t, defaultOnSelected, NULL);
	for (i = 0; i < t->pages->len; i++) {
		page = g_array_index(t->pages, uiprivChild *, i);
		uiprivChildDestroy(page);
	}
	g_array_free(t->pages, TRUE);
	// and free ourselves
	g_object_unref(t->widget);
	uiFreeControl(uiControl(t));
}

void uiTabAppend(uiTab *t, const char *name, uiControl *child)
{
	uiTabInsertAt(t, name, t->pages->len, child);
}

void uiTabInsertAt(uiTab *t, const char *name, int n, uiControl *child)
{
	uiprivChild *page;

	// this will create a tab, because of gtk_container_add()
	page = uiprivNewChildWithBox(child, uiControl(t), t->container, 0);

	gtk_notebook_set_tab_label_text(t->notebook, uiprivChildBox(page), name);
	gtk_notebook_reorder_child(t->notebook, uiprivChildBox(page), n);

	g_array_insert_val(t->pages, n, page);
}

void uiTabDelete(uiTab *t, int n)
{
	uiprivChild *page;

	page = g_array_index(t->pages, uiprivChild *, n);
	// this will remove the tab, because gtk_widget_destroy() calls gtk_container_remove()
	uiprivChildRemove(page);
	g_array_remove_index(t->pages, n);
}

int uiTabNumPages(uiTab *t)
{
	return t->pages->len;
}

int uiTabMargined(uiTab *t, int n)
{
	uiprivChild *page;

	page = g_array_index(t->pages, uiprivChild *, n);
	return uiprivChildFlag(page);
}

void uiTabSetMargined(uiTab *t, int n, int margined)
{
	uiprivChild *page;

	page = g_array_index(t->pages, uiprivChild *, n);
	uiprivChildSetFlag(page, margined);
	uiprivChildSetMargined(page, uiprivChildFlag(page));
}

static void onSelected(GtkNotebook* notebook, GtkWidget *page, guint index, gpointer data)
{
	uiTab *t = uiTab(data);
	(*(t->onSelected))(t, t->onSelectedData);
}

int uiTabSelected(uiTab *t)
{
	return gtk_notebook_get_current_page(t->notebook);
}

void uiTabSetSelected(uiTab *t, int index)
{
	gtk_notebook_set_current_page(t->notebook, index);
}

void uiTabOnSelected(uiTab *t, void (*f)(uiTab *, void *), void *data)
{
	t->onSelected = f;
	t->onSelectedData = data;
}

uiTab *uiNewTab(void)
{
	uiTab *t;

	uiUnixNewControl(uiTab, t);

	t->widget = gtk_notebook_new();
	t->container = GTK_CONTAINER(t->widget);
	t->notebook = GTK_NOTEBOOK(t->widget);

	gtk_notebook_set_scrollable(t->notebook, TRUE);

	t->pages = g_array_new(FALSE, TRUE, sizeof (uiprivChild *));

	g_signal_connect_after(t->notebook, "switch-page", G_CALLBACK(onSelected), t);
	uiTabOnSelected(t, defaultOnSelected, NULL);

	return t;
}
