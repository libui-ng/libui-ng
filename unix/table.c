// 26 june 2016
#include "uipriv_unix.h"
#include "table.h"

// TODO with GDK_SCALE set to 2 the 32x32 images are scaled up to 64x64?

struct uiTable {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *scontainer;
	GtkScrolledWindow *sw;
	GtkWidget *treeWidget;
	GtkTreeView *tv;
	uiTableModel *model;
	GPtrArray *columnParams;
	int backgroundColumn;
	// keys are struct rowcol, values are gint
	// TODO document this properly
	GHashTable *indeterminatePositions;
	guint indeterminateTimer;
	// Cache last selected row for GTK_SELECTION_BROWSE
	int lastSelectedRow;
	// Cache last selected rows count for GTK_SELECTION_MULTIPLE
	int lastSelectedRowsCount;
	// Cache last selected rows for GTK_SELECTION_MULTIPLE
	GList *lastSelectedRows;
	void (*headerOnClicked)(uiTable *, int, void *);
	void *headerOnClickedData;
	void (*onRowClicked)(uiTable *, int, void *);
	void *onRowClickedData;
	void (*onRowDoubleClicked)(uiTable *, int, void *);
	void *onRowDoubleClickedData;
	void (*onSelectionChanged)(uiTable *, void *);
	void *onSelectionChangedData;
	gulong onSelectionChangedSignal;
};

/*
// use the same size as GtkFileChooserWidget's treeview
// TODO refresh when icon theme changes
// TODO doesn't work when scaled?
// TODO is this even necessary?
static void setImageSize(GtkCellRenderer *r)
{
	gint size;
	gint width, height;
	gint xpad, ypad;

	size = 16;		// fallback used by GtkFileChooserWidget
	if (gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height) != FALSE)
		size = MAX(width, height);
	gtk_cell_renderer_get_padding(r, &xpad, &ypad);
	gtk_cell_renderer_set_fixed_size(r,
		2 * xpad + size,
		2 * ypad + size);
}
*/

static void applyColor(GtkTreeModel *m, GtkTreeIter *iter, int modelColumn, GtkCellRenderer *r, const char *prop, const char *propSet)
{
	GValue value = G_VALUE_INIT;
	GdkRGBA *rgba;

	gtk_tree_model_get_value(m, iter, modelColumn, &value);
	rgba = (GdkRGBA *) g_value_get_boxed(&value);
	if (rgba != NULL)
		g_object_set(r, prop, rgba, NULL);
	else
		g_object_set(r, propSet, FALSE, NULL);
	g_value_unset(&value);
}

static void setEditable(uiTableModel *m, GtkTreeIter *iter, int modelColumn, GtkCellRenderer *r, const char *prop)
{
	GtkTreePath *path;
	int row;
	gboolean editable;

	// TODO avoid the need for this
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(m), iter);
	row = gtk_tree_path_get_indices(path)[0];
	gtk_tree_path_free(path);
	editable = uiprivTableModelCellEditable(m, row, modelColumn) != 0;
	g_object_set(r, prop, editable, NULL);
}

static void applyBackgroundColor(uiTable *t, GtkTreeModel *m, GtkTreeIter *iter, GtkCellRenderer *r)
{
	if (t->backgroundColumn != -1)
		applyColor(m, iter, t->backgroundColumn,
			r, "cell-background-rgba", "cell-background-set");
}

static void onEdited(uiTableModel *m, int column, const char *pathstr, const uiTableValue *tvalue, GtkTreeIter *iter)
{
	GtkTreePath *path;
	int row;

	path = gtk_tree_path_new_from_string(pathstr);
	row = gtk_tree_path_get_indices(path)[0];
	if (iter != NULL)
		gtk_tree_model_get_iter(GTK_TREE_MODEL(m), iter, path);
	gtk_tree_path_free(path);
	uiprivTableModelSetCellValue(m, row, column, tvalue);
}

struct textColumnParams {
	uiTable *t;
	uiTableModel *m;
	int modelColumn;
	int editableColumn;
	uiTableTextColumnOptionalParams params;
};

static void textColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct textColumnParams *p = (struct textColumnParams *) data;
	GValue value = G_VALUE_INIT;
	const gchar *str;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	str = g_value_get_string(&value);
	g_object_set(r, "text", str, NULL);
	g_value_unset(&value);

	setEditable(p->m, iter, p->editableColumn, r, "editable");

	if (p->params.ColorModelColumn != -1)
		applyColor(m, iter, p->params.ColorModelColumn,
			r, "foreground-rgba", "foreground-set");

	applyBackgroundColor(p->t, m, iter, r);
}

static void textColumnEdited(GtkCellRendererText *r, gchar *path, gchar *newText, gpointer data)
{
	struct textColumnParams *p = (struct textColumnParams *) data;
	uiTableValue *tvalue;
	GtkTreeIter iter;

	tvalue = uiNewTableValueString(newText);
	onEdited(p->m, p->modelColumn, path, tvalue, &iter);
	uiFreeTableValue(tvalue);
	// and update the column TODO copy comment here
	textColumnDataFunc(NULL, GTK_CELL_RENDERER(r), GTK_TREE_MODEL(p->m), &iter, data);
}

struct imageColumnParams {
	uiTable *t;
	int modelColumn;
};

static void imageColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct imageColumnParams *p = (struct imageColumnParams *) data;
	GValue value = G_VALUE_INIT;
	uiImage *img;

//TODO	setImageSize(r);
	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	img = (uiImage *) g_value_get_pointer(&value);
	g_object_set(r, "surface",
		uiprivImageAppropriateSurface(img, p->t->treeWidget),
		NULL);
	g_value_unset(&value);

	applyBackgroundColor(p->t, m, iter, r);
}

struct checkboxColumnParams {
	uiTable *t;
	uiTableModel *m;
	int modelColumn;
	int editableColumn;
};

static void checkboxColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct checkboxColumnParams *p = (struct checkboxColumnParams *) data;
	GValue value = G_VALUE_INIT;
	gboolean active;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	active = g_value_get_int(&value) != 0;
	g_object_set(r, "active", active, NULL);
	g_value_unset(&value);

	setEditable(p->m, iter, p->editableColumn, r, "activatable");

	applyBackgroundColor(p->t, m, iter, r);
}

static void checkboxColumnToggled(GtkCellRendererToggle *r, gchar *pathstr, gpointer data)
{
	struct checkboxColumnParams *p = (struct checkboxColumnParams *) data;
	GValue value = G_VALUE_INIT;
	int v;
	uiTableValue *tvalue;
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_string(pathstr);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(p->m), &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get_value(GTK_TREE_MODEL(p->m), &iter, p->modelColumn, &value);
	v = g_value_get_int(&value);
	g_value_unset(&value);
	tvalue = uiNewTableValueInt(!v);
	onEdited(p->m, p->modelColumn, pathstr, tvalue, NULL);
	uiFreeTableValue(tvalue);
	// and update the column TODO copy comment here
	// TODO avoid fetching the model data twice
	checkboxColumnDataFunc(NULL, GTK_CELL_RENDERER(r), GTK_TREE_MODEL(p->m), &iter, data);
}

struct progressBarColumnParams {
	uiTable *t;
	int modelColumn;
};

struct rowcol {
	int row;
	int col;
};

static guint rowcolHash(gconstpointer key)
{
	const struct rowcol *rc = (const struct rowcol *) key;
	guint row, col;

	row = (guint) (rc->row);
	col = (guint) (rc->col);
	return row ^ col;
}

static gboolean rowcolEqual(gconstpointer a, gconstpointer b)
{
	const struct rowcol *ra = (const struct rowcol *) a;
	const struct rowcol *rb = (const struct rowcol *) b;

	return (ra->row == rb->row) && (ra->col == rb->col);
}

static void pulseOne(gpointer key, gpointer value, gpointer data)
{
	uiTable *t = uiTable(data);
	struct rowcol *rc = (struct rowcol *) key;

	// TODO this is bad: it produces changed handlers for every table because that's how GtkTreeModel works, yet this is per-table because that's how it works
	// however, a proper fix would require decoupling progress from normal integers, which we could do...
	uiTableModelRowChanged(t->model, rc->row);
}

static gboolean indeterminatePulse(gpointer data)
{
	uiTable *t = uiTable(data);

	g_hash_table_foreach(t->indeterminatePositions, pulseOne, t);
	return TRUE;
}

static void progressBarColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct progressBarColumnParams *p = (struct progressBarColumnParams *) data;
	GValue value = G_VALUE_INIT;
	int pval;
	struct rowcol *rc;
	gint *val;
	GtkTreePath *path;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	pval = g_value_get_int(&value);
	rc = uiprivNew(struct rowcol);
	// TODO avoid the need for this
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(m), iter);
	rc->row = gtk_tree_path_get_indices(path)[0];
	gtk_tree_path_free(path);
	rc->col = p->modelColumn;
	val = (gint *) g_hash_table_lookup(p->t->indeterminatePositions, rc);
	if (pval == -1) {
		if (val == NULL) {
			val = uiprivNew(gint);
			*val = 1;
			g_hash_table_insert(p->t->indeterminatePositions, rc, val);
		} else {
			uiprivFree(rc);
			(*val)++;
			if (*val == G_MAXINT)
				*val = 1;
		}
		g_object_set(r,
			"pulse", *val,
			NULL);
		if (p->t->indeterminateTimer == 0)
			// TODO verify the timeout
			p->t->indeterminateTimer = g_timeout_add(100, indeterminatePulse, p->t);
	} else {
		if (val != NULL) {
			g_hash_table_remove(p->t->indeterminatePositions, rc);
			if (g_hash_table_size(p->t->indeterminatePositions) == 0) {
				g_source_remove(p->t->indeterminateTimer);
				p->t->indeterminateTimer = 0;
			}
		}
		uiprivFree(rc);
		g_object_set(r,
			"pulse", -1,
			"value", pval,
			NULL);
	}
	g_value_unset(&value);

	applyBackgroundColor(p->t, m, iter, r);
}

struct buttonColumnParams {
	uiTable *t;
	uiTableModel *m;
	int modelColumn;
	int clickableColumn;
};

static void buttonColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct buttonColumnParams *p = (struct buttonColumnParams *) data;
	GValue value = G_VALUE_INIT;
	const gchar *str;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	str = g_value_get_string(&value);
	g_object_set(r, "text", str, NULL);
	g_value_unset(&value);

	setEditable(p->m, iter, p->clickableColumn, r, "sensitive");

	applyBackgroundColor(p->t, m, iter, r);
}

// TODO wrong type here
static void buttonColumnClicked(GtkCellRenderer *r, gchar *pathstr, gpointer data)
{
	struct buttonColumnParams *p = (struct buttonColumnParams *) data;

	onEdited(p->m, p->modelColumn, pathstr, NULL, NULL);
}

uiSortIndicator uiTableHeaderSortIndicator(uiTable *t, int lcol)
{
	GtkTreeViewColumn *c = gtk_tree_view_get_column(t->tv, lcol);

	if (c == NULL || gtk_tree_view_column_get_sort_indicator(c) == FALSE)
		return uiSortIndicatorNone;

	if (gtk_tree_view_column_get_sort_order(c) == GTK_SORT_ASCENDING)
		return uiSortIndicatorAscending;
	else
		return uiSortIndicatorDescending;
}

void uiTableHeaderSetSortIndicator(uiTable *t, int lcol, uiSortIndicator indicator)
{
	GtkTreeViewColumn *c = gtk_tree_view_get_column(t->tv, lcol);

	if (c == NULL)
		return;

	if (indicator == uiSortIndicatorNone) {
		gtk_tree_view_column_set_sort_indicator(c, FALSE);
		return;
	}

	gtk_tree_view_column_set_sort_indicator(c, TRUE);
	if (indicator == uiSortIndicatorAscending)
		gtk_tree_view_column_set_sort_order(c, GTK_SORT_ASCENDING);
	else
		gtk_tree_view_column_set_sort_order(c, GTK_SORT_DESCENDING);
}

void uiTableHeaderOnClicked(uiTable *t, void (*f)(uiTable *, int, void *), void *data)
{
	t->headerOnClicked = f;
	t->headerOnClickedData = data;
}

static void defaultHeaderOnClicked(uiTable *table, int column, void *data)
{
	// do nothing
}

static void headerOnClicked(GtkTreeViewColumn *c, gpointer data)
{
	guint i;
	uiTable *t = uiTable(data);

	for (i = 0; i < gtk_tree_view_get_n_columns(t->tv); ++i)
		if (gtk_tree_view_get_column(t->tv, i) == c)
			t->headerOnClicked(t, i, t->headerOnClickedData);
}

void uiTableOnSelectionChanged(uiTable *t, void (*f)(uiTable *t, void *data), void *data)
{
	t->onSelectionChanged = f;
	t->onSelectionChangedData = data;
}

static void defaultOnSelectionChanged(uiTable *table, void *data)
{
	// do nothing
}

/**
 * Determines if a selection truly changed.
 *
 * GTK sends a "changed" signal on various user interactions even when the
 * selection has not truly changed. See upstream bugs:
 *
 * GTK_SELECTION_BROWSE:
 * https://gitlab.gnome.org/GNOME/gtk/-/issues/5061
 *
 * GTK_SELECTION_MULTIPLE
 * https://gitlab.gnome.org/GNOME/gtk/-/issues/5314
 *
 * @returns `TRUE` if the selection changed, `FALSE` otherwise.
 */
static gboolean selectionChanged(uiTable *t, GtkTreeSelection *s)
{
	GtkTreeIter iter;
	gint row;
	gint rowCount;
	GList *list;
	GList *a, *b;
	GtkTreeModel *m = GTK_TREE_MODEL(t->model);

	if (gtk_tree_selection_get_mode(s) == GTK_SELECTION_BROWSE) {
		if (gtk_tree_selection_get_selected(s, NULL, &iter)) {
			row = GPOINTER_TO_INT(iter.user_data);
			if (row == t->lastSelectedRow)
				return FALSE;
			else
				t->lastSelectedRow = row;
		}
		else {
			t->lastSelectedRow = -1;
		}
	}
	else if (gtk_tree_selection_get_mode(s) == GTK_SELECTION_MULTIPLE) {
		rowCount = gtk_tree_selection_count_selected_rows(s);
		if (rowCount != t->lastSelectedRowsCount) {
			t->lastSelectedRowsCount = rowCount;
		}
		else {
			list = gtk_tree_selection_get_selected_rows(s, &m);
			for (a = list, b = t->lastSelectedRows; a != NULL && b != NULL; a = a->next, b = b->next) {
				if (gtk_tree_path_compare(a->data, b->data) != 0) {
					g_list_free_full(t->lastSelectedRows, (GDestroyNotify)gtk_tree_path_free);
					t->lastSelectedRows = list;
					return TRUE;
				}
			}
			return FALSE;
		}
	}
	return TRUE;
}

static void onSelectionChanged(GtkTreeSelection *s, gpointer data)
{
	uiTable *t = uiTable(data);

	// Abort if the row is already selected. See upstream bug:
	// https://gitlab.gnome.org/GNOME/gtk/-/issues/5061
	if (!selectionChanged(t, s))
		return;

	t->onSelectionChanged(t, t->onSelectionChangedData);
}

uiTableSelection* uiTableGetSelection(uiTable *t)
{
	int i = 0;
	GList *e;
	GList *list;
	GtkTreeSelection *sel;
	GtkTreeModel *m = GTK_TREE_MODEL(t->model);
	uiTableSelection *s = uiprivNew(uiTableSelection);

	sel = gtk_tree_view_get_selection(t->tv);
	list = gtk_tree_selection_get_selected_rows(sel, &m);

	s->NumRows = g_list_length(list);
	if (s->NumRows == 0)
		s->Rows = NULL;
	else
		s->Rows = uiprivAlloc(s->NumRows * sizeof(*s->Rows), "uiTableSelection->Rows");

	for (e = list; e != NULL; e = e->next) {
		GtkTreePath *path = e->data;
		s->Rows[i++] = gtk_tree_path_get_indices(path)[0];
	}

	g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);

	return s;
}

void uiTableSetSelection(uiTable *t, uiTableSelection *sel)
{
	int i;
	GtkTreeSelection *ts;
	uiTableSelectionMode mode = uiTableGetSelectionMode(t);

	if ((mode == uiTableSelectionModeNone && sel->NumRows > 0) ||
	    (mode == uiTableSelectionModeZeroOrOne && sel->NumRows > 1) ||
	    (mode == uiTableSelectionModeOne && sel->NumRows > 1)) {
		// TODO log error
		return;
	}

	ts = gtk_tree_view_get_selection(t->tv);
	gtk_tree_selection_unselect_all(ts);

	for (i = 0; i < sel->NumRows; ++i) {
		GtkTreePath *path = gtk_tree_path_new_from_indices(sel->Rows[i], -1);
		gtk_tree_selection_select_path(ts, path);
		gtk_tree_path_free(path);
	}
}

static GtkTreeViewColumn *addColumn(uiTable *t, const char *name)
{
	GtkTreeViewColumn *c;

	c = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(c, TRUE);
	gtk_tree_view_column_set_title(c, name);
	gtk_tree_view_column_set_clickable(c, 1);
	g_signal_connect(c, "clicked", G_CALLBACK(headerOnClicked), t);
	gtk_tree_view_append_column(t->tv, c);
	return c;
}

static void addTextColumn(uiTable *t, GtkTreeViewColumn *c, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	struct textColumnParams *p;
	GtkCellRenderer *r;

	p = uiprivNew(struct textColumnParams);
	p->t = t;
	// TODO get rid of these fields AND rename t->model in favor of t->m
	p->m = t->model;
	p->modelColumn = textModelColumn;
	p->editableColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->params = *textParams;
	else
		p->params = uiprivDefaultTextColumnOptionalParams;

	r = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(c, r, TRUE);
	gtk_tree_view_column_set_cell_data_func(c, r, textColumnDataFunc, p, NULL);
	g_signal_connect(r, "edited", G_CALLBACK(textColumnEdited), p);
	g_ptr_array_add(t->columnParams, p);
}

// TODO rename modelCOlumn and params everywhere
void uiTableAppendTextColumn(uiTable *t, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addTextColumn(t, c, textModelColumn, textEditableModelColumn, textParams);
}

static void addImageColumn(uiTable *t, GtkTreeViewColumn *c, int imageModelColumn)
{
	struct imageColumnParams *p;
	GtkCellRenderer *r;

	p = uiprivNew(struct imageColumnParams);
	p->t = t;
	p->modelColumn = imageModelColumn;

	r = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(c, r, FALSE);
	gtk_tree_view_column_set_cell_data_func(c, r, imageColumnDataFunc, p, NULL);
	g_ptr_array_add(t->columnParams, p);
}

void uiTableAppendImageColumn(uiTable *t, const char *name, int imageModelColumn)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addImageColumn(t, c, imageModelColumn);
}

void uiTableAppendImageTextColumn(uiTable *t, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addImageColumn(t, c, imageModelColumn);
	addTextColumn(t, c, textModelColumn, textEditableModelColumn, textParams);
}

static void addCheckboxColumn(uiTable *t, GtkTreeViewColumn *c, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	struct checkboxColumnParams *p;
	GtkCellRenderer *r;

	p = uiprivNew(struct checkboxColumnParams);
	p->t = t;
	p->m = t->model;
	p->modelColumn = checkboxModelColumn;
	p->editableColumn = checkboxEditableModelColumn;

	r = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(c, r, FALSE);
	gtk_tree_view_column_set_cell_data_func(c, r, checkboxColumnDataFunc, p, NULL);
	g_signal_connect(r, "toggled", G_CALLBACK(checkboxColumnToggled), p);
	g_ptr_array_add(t->columnParams, p);
}

void uiTableAppendCheckboxColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addCheckboxColumn(t, c, checkboxModelColumn, checkboxEditableModelColumn);
}

void uiTableAppendCheckboxTextColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addCheckboxColumn(t, c, checkboxModelColumn, checkboxEditableModelColumn);
	addTextColumn(t, c, textModelColumn, textEditableModelColumn, textParams);
}

void uiTableAppendProgressBarColumn(uiTable *t, const char *name, int progressModelColumn)
{
	GtkTreeViewColumn *c;
	struct progressBarColumnParams *p;
	GtkCellRenderer *r;

	c = addColumn(t, name);

	p = uiprivNew(struct progressBarColumnParams);
	p->t = t;
	// TODO make progress and progressBar consistent everywhere
	p->modelColumn = progressModelColumn;

	r = gtk_cell_renderer_progress_new();
	gtk_tree_view_column_pack_start(c, r, TRUE);
	gtk_tree_view_column_set_cell_data_func(c, r, progressBarColumnDataFunc, p, NULL);
	g_ptr_array_add(t->columnParams, p);
}

void uiTableAppendButtonColumn(uiTable *t, const char *name, int buttonModelColumn, int buttonClickableModelColumn)
{
	GtkTreeViewColumn *c;
	struct buttonColumnParams *p;
	GtkCellRenderer *r;

	c = addColumn(t, name);

	p = uiprivNew(struct buttonColumnParams);
	p->t = t;
	p->m = t->model;
	p->modelColumn = buttonModelColumn;
	p->clickableColumn = buttonClickableModelColumn;

	r = uiprivNewCellRendererButton();
	gtk_tree_view_column_pack_start(c, r, TRUE);
	gtk_tree_view_column_set_cell_data_func(c, r, buttonColumnDataFunc, p, NULL);
	g_signal_connect(r, "clicked", G_CALLBACK(buttonColumnClicked), p);
	g_ptr_array_add(t->columnParams, p);
}

int uiTableHeaderVisible(uiTable *t)
{
	return gtk_tree_view_get_headers_visible(t->tv);
}

void uiTableHeaderSetVisible(uiTable *t, int visible)
{
	gtk_tree_view_set_headers_visible(t->tv, visible);
}

uiUnixControlAllDefaultsExceptDestroy(uiTable)

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);
	guint i;

	for (i = 0; i < t->columnParams->len; i++)
		uiprivFree(g_ptr_array_index(t->columnParams, i));
	g_ptr_array_free(t->columnParams, TRUE);
	if (g_hash_table_size(t->indeterminatePositions) != 0)
		g_source_remove(t->indeterminateTimer);
	g_hash_table_destroy(t->indeterminatePositions);
	if (t->lastSelectedRows != NULL)
		g_list_free_full(t->lastSelectedRows, (GDestroyNotify)gtk_tree_path_free);
	g_object_unref(t->widget);
	uiFreeControl(uiControl(t));
}

static void defaultOnRowClicked(uiTable *table, int row, void *data)
{
	// do nothing
}

static void defaultOnRowDoubleClicked(uiTable *table, int row, void *data)
{
	// do nothing
}

void uiTableOnRowClicked(uiTable *t, void (*f)(uiTable *, int, void *), void *data)
{
	t->onRowClicked = f;
	t->onRowClickedData = data;
}

void uiTableOnRowDoubleClicked(uiTable *t, void (*f)(uiTable *, int, void *), void *data)
{
	t->onRowDoubleClicked = f;
	t->onRowDoubleClickedData = data;
}

#if GTK_CHECK_VERSION(3, 14, 0)
static void onButtonPressed(GtkGestureMultiPress *gesture, gint nPress, gdouble wx, gdouble wy, gpointer data)
{
	uiTable *t = uiTable(data);
	GtkTreePath *path;
	gint row, x, y;

	gtk_tree_view_convert_widget_to_bin_window_coords(t->tv, wx, wy, &x, &y);
	gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(t->tv), x, y, &path, NULL, NULL, NULL);
	if (path == NULL)
		return;

	row = gtk_tree_path_get_indices(path)[0];
	gtk_tree_path_free(path);

	if (nPress == 1)
		(*(t->onRowClicked))(t, row, t->onRowClickedData);
	else if (nPress == 2)
		(*(t->onRowDoubleClicked))(t, row, t->onRowDoubleClickedData);
}
#else
static gboolean onButtonPressed(GtkWidget *tv, GdkEventButton *event, gpointer data)
{
	uiTable *t = uiTable(data);
	GtkTreePath *path;
	gint row;

	if (event->window != gtk_tree_view_get_bin_window(t->tv))
		return FALSE;
	if (event->button != GDK_BUTTON_PRIMARY)
		return FALSE;

	gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tv), event->x, event->y, &path, NULL, NULL, NULL);
	if (path == NULL)
		return FALSE;

	row = gtk_tree_path_get_indices(path)[0];
	gtk_tree_path_free(path);

	if (event->type == GDK_BUTTON_PRESS)
		(*(t->onRowClicked))(t, row, t->onRowClickedData);
	else if (event->type == GDK_2BUTTON_PRESS)
		(*(t->onRowDoubleClicked))(t, row, t->onRowDoubleClickedData);

	return FALSE;
}
#endif

uiTable *uiNewTable(uiTableParams *p)
{
	uiTable *t;
#if GTK_CHECK_VERSION(3, 14, 0)
	GtkGesture *gesture;
#endif
	GtkTreeSelection *selection;

	uiUnixNewControl(uiTable, t);

	t->model = p->Model;
	t->columnParams = g_ptr_array_new();
	t->backgroundColumn = p->RowBackgroundColorModelColumn;

	t->widget = gtk_scrolled_window_new(NULL, NULL);
	t->scontainer = GTK_CONTAINER(t->widget);
	t->sw = GTK_SCROLLED_WINDOW(t->widget);
	gtk_scrolled_window_set_shadow_type(t->sw, GTK_SHADOW_IN);

	t->treeWidget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(t->model));
	t->tv = GTK_TREE_VIEW(t->treeWidget);

	// TODO set up t->tv
	uiTableOnRowClicked(t, defaultOnRowClicked, NULL);
	uiTableOnRowDoubleClicked(t, defaultOnRowDoubleClicked, NULL);

#if GTK_CHECK_VERSION(3, 14, 0)
	gesture = gtk_gesture_multi_press_new(GTK_WIDGET(t->tv));
	g_signal_connect(gesture, "pressed", G_CALLBACK(onButtonPressed), t);
	g_object_set_data_full(G_OBJECT(t->tv), "table-pressed-gesture", gesture, g_object_unref);
#else
	g_signal_connect(t->tv, "button-press-event", G_CALLBACK(onButtonPressed), t);
#endif

	gtk_container_add(t->scontainer, t->treeWidget);
	// and make the tree view visible; only the scrolled window's visibility is controlled by libui
	gtk_widget_show(t->treeWidget);

	t->indeterminatePositions = g_hash_table_new_full(rowcolHash, rowcolEqual,
		uiprivFree, uiprivFree);

	uiTableHeaderOnClicked(t, defaultHeaderOnClicked, NULL);
	uiTableOnSelectionChanged(t, defaultOnSelectionChanged, NULL);

	selection = gtk_tree_view_get_selection(t->tv);
	t->onSelectionChangedSignal = g_signal_connect(G_OBJECT(selection), "changed",
		G_CALLBACK(onSelectionChanged), t);
	t->lastSelectedRows = NULL;

	return t;
}

int uiTableColumnWidth(uiTable *t, int column)
{
	GtkTreeViewColumn *c = gtk_tree_view_get_column(t->tv, column);
	return gtk_tree_view_column_get_width(c);
}

void uiTableColumnSetWidth(uiTable *t, int column, int width)
{
	GtkTreeViewColumn *c = gtk_tree_view_get_column(t->tv, column);
	gtk_tree_view_column_set_fixed_width(c, width);
}

uiTableSelectionMode uiTableGetSelectionMode(uiTable *t)
{
	GtkTreeSelection *select = gtk_tree_view_get_selection(t->tv);

	switch (gtk_tree_selection_get_mode(select)) {
		case GTK_SELECTION_NONE:
			return uiTableSelectionModeNone;
		case GTK_SELECTION_SINGLE:
			return uiTableSelectionModeZeroOrOne;
		case GTK_SELECTION_BROWSE:
			return uiTableSelectionModeOne;
		case GTK_SELECTION_MULTIPLE:
			return uiTableSelectionModeZeroOrMany;
		default:
			uiprivImplBug("Invalid table selection mode");
			return 0;
	}
}

void uiTableSetSelectionMode(uiTable *t, uiTableSelectionMode mode)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(t->tv);
	GtkSelectionMode type;
	GtkTreeModel *m = GTK_TREE_MODEL(t->model);

	g_signal_handler_block(selection, t->onSelectionChangedSignal);
	switch (mode) {
		case uiTableSelectionModeNone:
			type = GTK_SELECTION_NONE;
			break;
		case uiTableSelectionModeZeroOrOne:
			// gtk_tree_selection_unselect_all() is broken when followed
			// by a mode change. TODO report upstream
			if (gtk_tree_selection_count_selected_rows(selection) > 1)
				gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);
			type = GTK_SELECTION_SINGLE;
			break;
		case uiTableSelectionModeOne:
			// gtk_tree_selection_unselect_all() is broken when followed
			// by a mode change. TODO report upstream
			if (gtk_tree_selection_count_selected_rows(selection) > 1)
				gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);
			type = GTK_SELECTION_BROWSE;
			break;
		case uiTableSelectionModeZeroOrMany:
			t->lastSelectedRowsCount = gtk_tree_selection_count_selected_rows(selection);
			t->lastSelectedRows = gtk_tree_selection_get_selected_rows(selection, &m);
			type = GTK_SELECTION_MULTIPLE;
			break;
		default:
			uiprivUserBug("Invalid table selection mode %d", mode);
			return;
	}

	gtk_tree_selection_set_mode(selection, type);
	selectionChanged(t, selection);
	g_signal_handler_unblock(selection, t->onSelectionChangedSignal);
}

