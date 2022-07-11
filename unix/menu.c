// 23 april 2015
#include "uipriv_unix.h"

static GArray *menus = NULL;
static gboolean menusFinalized = FALSE;
static gboolean hasQuit = FALSE;
static gboolean hasPreferences = FALSE;
static gboolean hasAbout = FALSE;

struct uiMenu {
	char *name;
	GArray *items;					// []*uiMenuItem
	gboolean ischild;
};

struct uiMenuItem {
	char *name;
	int type;
	void (*onClicked)(uiMenuItem *, uiWindow *, void *);
	void *onClickedData;
	GType gtype;					// template for new instances; kept in sync with everything else
	gboolean disabled;
	gboolean checked;
	GArray *menuItems;			// []*menuItemWindow
	uiMenu *popupchild;
};

struct menuItemWindow {
	GtkMenuItem *item;
	uiWindow *w;
	gulong signal;
};

enum {
	typeRegular,
	typeCheckbox,
	typeQuit,
	typePreferences,
	typeAbout,
	typeSeparator,
	typeSubmenu,
};

// we do NOT want programmatic updates to raise an ::activated signal
static void singleSetChecked(GtkCheckMenuItem *menuitem, gboolean checked, gulong signal)
{
	g_signal_handler_block(menuitem, signal);
	gtk_check_menu_item_set_active(menuitem, checked);
	g_signal_handler_unblock(menuitem, signal);
}

static void setChecked(uiMenuItem *item, gboolean checked)
{
	struct menuItemWindow *w;
	guint i;

	item->checked = checked;
	for (i = 0; i < item->menuItems->len; i++) {
		w = g_array_index(item->menuItems, struct menuItemWindow *, i);
		singleSetChecked(GTK_CHECK_MENU_ITEM(w->item), item->checked, w->signal);
	}
}

static void onClicked(GtkMenuItem *menuitem, gpointer data)
{
	uiMenuItem *item = uiMenuItem(data);
	struct menuItemWindow *w;
	guint i;

	// we need to manually update the checked states of all menu items if one changes
	// notice that this is getting the checked state of the menu item that this signal is sent from
	if (item->type == typeCheckbox)
		setChecked(item, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));

	for (i = 0; i < item->menuItems->len; i++) {
		w = g_array_index(item->menuItems, struct menuItemWindow *, i);
		if (w->item == menuitem)
			break;
	}

	(*(item->onClicked))(item, w->w, item->onClickedData);
}

static void defaultOnClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	// do nothing
}

static void onQuitClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	if (uiprivShouldQuit())
		uiQuit();
}

static void menuItemEnableDisable(uiMenuItem *item, gboolean enabled)
{
	struct menuItemWindow* w;
	guint i;

	item->disabled = !enabled;

	for (i = 0; i<item->menuItems->len; i++) {
		w = g_array_index(item->menuItems, struct menuItemWindow *, i);
		gtk_widget_set_sensitive(GTK_WIDGET(w->item), enabled);
	}
}

void uiMenuItemEnable(uiMenuItem *item)
{
	menuItemEnableDisable(item, TRUE);
}

void uiMenuItemDisable(uiMenuItem *item)
{
	menuItemEnableDisable(item, FALSE);
}

void uiMenuItemOnClicked(uiMenuItem *item, void (*f)(uiMenuItem *, uiWindow *, void *), void *data)
{
	if (item->type == typeQuit)
		uiprivUserBug("You cannot call uiMenuItemOnClicked() on a Quit item; use uiOnShouldQuit() instead.");
	if (item->type == typeSubmenu)
		uiprivUserBug("You cannot call uiMenuItemOnClicked() on an item with a subMenu.");
	item->onClicked = f;
	item->onClickedData = data;
}

int uiMenuItemChecked(uiMenuItem *item)
{
	return item->checked != FALSE;
}

void uiMenuItemSetChecked(uiMenuItem *item, int checked)
{
	gboolean c;

	// use explicit values
	c = FALSE;
	if (checked)
		c = TRUE;
	setChecked(item, c);
}

static uiMenuItem *newItem(uiMenu *m, int type, const char *name)
{
	uiMenuItem *item;

	if (menusFinalized)
		uiprivUserBug("You cannot create a new menu item after menus have been finalized.");

	item = uiprivNew(uiMenuItem);

	g_array_append_val(m->items, item);

	item->type = type;
	switch (item->type) {
	case typeQuit:
		item->name = g_strdup("Quit");
		break;
	case typePreferences:
		item->name = g_strdup("Preferences...");
		break;
	case typeAbout:
		item->name = g_strdup("About");
		break;
	case typeSeparator:
		break;
	default:
		item->name = g_strdup(name);
		break;
	}

	if (item->type == typeQuit) {
		// can't call uiMenuItemOnClicked() here
		item->onClicked = onQuitClicked;
		item->onClickedData = NULL;
	} else
		uiMenuItemOnClicked(item, defaultOnClicked, NULL);

	switch (item->type) {
	case typeCheckbox:
		item->gtype = GTK_TYPE_CHECK_MENU_ITEM;
		break;
	case typeSeparator:
		item->gtype = GTK_TYPE_SEPARATOR_MENU_ITEM;
		break;
	default:
		item->gtype = GTK_TYPE_MENU_ITEM;
		break;
	}

	item->menuItems = g_array_new(FALSE, TRUE, sizeof (struct menuItemWindow *));
	item->popupchild = NULL;

	return item;
}

uiMenuItem *uiMenuAppendSubmenu(uiMenu *m, uiMenu* child)
{
	uiMenuItem *item;

	if (menusFinalized)
		uiprivUserBug("You cannot create a new menu item after menus have been finalized.");

	item = uiprivNew(uiMenuItem);

	g_array_append_val(m->items, item);

	item->type = typeSubmenu;
	item->name = g_strdup(child->name);
	item->gtype = GTK_TYPE_MENU_ITEM;
	item->menuItems = g_array_new(FALSE, TRUE, sizeof (struct menuItemWindow *));
	item->popupchild = child;
	child->ischild = TRUE;

	return item;
}

uiMenuItem *uiMenuAppendItem(uiMenu *m, const char *name)
{
	return newItem(m, typeRegular, name);
}

uiMenuItem *uiMenuAppendCheckItem(uiMenu *m, const char *name)
{
	return newItem(m, typeCheckbox, name);
}

uiMenuItem *uiMenuAppendQuitItem(uiMenu *m)
{
	if (hasQuit)
		uiprivUserBug("You cannot have multiple Quit menu items in the same program.");
	hasQuit = TRUE;
	newItem(m, typeSeparator, NULL);
	return newItem(m, typeQuit, NULL);
}

uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m)
{
	if (hasPreferences)
		uiprivUserBug("You cannot have multiple Preferences menu items in the same program.");
	hasPreferences = TRUE;
	newItem(m, typeSeparator, NULL);
	return newItem(m, typePreferences, NULL);
}

uiMenuItem *uiMenuAppendAboutItem(uiMenu *m)
{
	if (hasAbout)
		uiprivUserBug("You cannot have multiple About menu items in the same program.");
	hasAbout = TRUE;
	newItem(m, typeSeparator, NULL);
	return newItem(m, typeAbout, NULL);
}

void uiMenuAppendSeparator(uiMenu *m)
{
	newItem(m, typeSeparator, NULL);
}

uiMenu *uiNewMenu(const char *name)
{
	uiMenu *m;

	if (menusFinalized)
		uiprivUserBug("You cannot create a new menu after menus have been finalized.");
	if (menus == NULL)
		menus = g_array_new(FALSE, TRUE, sizeof (uiMenu *));

	m = uiprivNew(uiMenu);

	g_array_append_val(menus, m);

	m->name = g_strdup(name);
	m->items = g_array_new(FALSE, TRUE, sizeof (uiMenuItem *));
	m->ischild = FALSE;

	return m;
}

static void appendMenuItem(GtkMenuShell *submenu, uiMenuItem *item, uiWindow *w)
{
	GtkWidget *menuitem;
	gulong signal;
	struct menuItemWindow *ww;

	uiMenu *child;
	guint j;
	GtkWidget *childSubmenu;

	menuitem = g_object_new(item->gtype, NULL);
	signal = 0;
	if (item->name != NULL)
		gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), item->name);
	if (item->type != typeSeparator && item->type != typeSubmenu) {
		signal = g_signal_connect(menuitem, "activate", G_CALLBACK(onClicked), item);
		gtk_widget_set_sensitive(menuitem, !item->disabled);
		if (item->type == typeCheckbox)
			singleSetChecked(GTK_CHECK_MENU_ITEM(menuitem), item->checked, signal);
	}
	gtk_menu_shell_append(submenu, menuitem);
	ww = uiprivNew(struct menuItemWindow);
	ww->item = GTK_MENU_ITEM(menuitem);
	ww->w = w;
	ww->signal = signal;
	g_array_append_val(item->menuItems, ww);

	if (item->popupchild != NULL) {
		child = item->popupchild;
		childSubmenu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), childSubmenu);
		for (j = 0; j < child->items->len; j++)
			appendMenuItem(GTK_MENU_SHELL(childSubmenu), g_array_index(child->items, uiMenuItem *, j), w);
	}
}

GtkWidget *uiprivMakeMenubar(uiWindow *w)
{
	GtkWidget *menubar;
	guint i, j;
	uiMenu *m;
	GtkWidget *menuitem;
	GtkWidget *submenu;

	menusFinalized = TRUE;

	menubar = gtk_menu_bar_new();

	if (menus != NULL)
		for (i = 0; i < menus->len; i++) {
			m = g_array_index(menus, uiMenu *, i);
			if (m->ischild) continue;
			menuitem = gtk_menu_item_new_with_label(m->name);
			submenu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
			for (j = 0; j < m->items->len; j++)
				appendMenuItem(GTK_MENU_SHELL(submenu), g_array_index(m->items, uiMenuItem *, j), w);
			gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitem);
		}

	gtk_widget_set_hexpand(menubar, TRUE);
	gtk_widget_set_halign(menubar, GTK_ALIGN_FILL);
	return menubar;
}

void uiprivFreeMenubar(uiWindow *w)
{
	guint i, j, windowIndex;
	uiMenu *m;
	uiMenuItem *item;
	struct menuItemWindow *ww;

	if (menus == NULL)
		return;

	// get the index of the uiWindow to free, assume that the first uiMenu has a uiMenuItem
	// TODO: is it safe to assume that? should we add a userBug if a uiMenu is empty?
	m = g_array_index(menus, uiMenu *, 0);
	item = g_array_index(m->items, uiMenuItem *, 0);
	for (i = 0; i < item->menuItems->len; i++)	{
		ww = g_array_index(item->menuItems, struct menuItemWindow *, i);
		if (ww->w == w) {
			windowIndex = i;
			break;
		}
	}

	for (i = 0; i < menus->len; i++) {
		m = g_array_index(menus, uiMenu *, i);
		for (j = 0; j < m->items->len; j++) {
			item = g_array_index(m->items, uiMenuItem *, j);
			ww = g_array_index(item->menuItems, struct menuItemWindow *, windowIndex);
			uiprivFree(ww);
			g_array_remove_index_fast(item->menuItems, windowIndex);
		}
	}
	// no need to worry about destroying any widgets; destruction of the window they're in will do it for us
}

void uiprivUninitMenus(void)
{
	uiMenu *m;
	uiMenuItem *item;
	guint i, j;

	if (menus == NULL)
		return;
	for (i = 0; i < menus->len; i++) {
		m = g_array_index(menus, uiMenu *, i);
		g_free(m->name);
		for (j = 0; j < m->items->len; j++) {
			item = g_array_index(m->items, uiMenuItem *, j);
			if (item->menuItems->len != 0)
				// TODO is this really a userbug()?
				uiprivImplBug("menu item %p (%s) still has uiWindows attached; did you forget to destroy some windows?", item, item->name);
			g_free(item->name);
			g_array_free(item->menuItems, TRUE);
			uiprivFree(item);
		}
		g_array_free(m->items, TRUE);
		uiprivFree(m);
	}
	g_array_free(menus, TRUE);
}
