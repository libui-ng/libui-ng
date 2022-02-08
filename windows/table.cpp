#include "uipriv_windows.hpp"
#include "table.hpp"

// general TODOs:
// - tooltips don't work properly on columns with icons (the listview always thinks there's enough room for a short label because it's not taking the icon into account); is this a bug in our LVN_GETDISPINFO handler or something else?
// - should clicking on some other column of the same row, even one that doesn't edit, cancel editing?
// - implement keyboard accessibility
// - implement accessibility in general (Dynamic Annotations maybe?)
// - if I didn't handle these already: "drawing focus rects here, subitem navigation and activation with the keyboard"

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = uiprivNew(uiTableModel);
	m->mh = mh;
	m->tables = new std::vector<uiTable *>;
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	delete m->tables;
	uiprivFree(m);
}

void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	LVITEMW item = {};
	item.mask = 0;
	item.iItem = newIndex;
	item.iSubItem = 0;

	for (auto t : *(m->tables)) {
		if (ListView_InsertItem(t->hwnd, &item) == -1)
			logLastError(L"error calling ListView_InsertItem in uiTableModelRowInserted()");
		// redraw every row from the new row down to simulate adding it
		if (ListView_RedrawItems(t->hwnd, newIndex, ListView_GetItemCount(t->hwnd)-1) == -1)
			logLastError(L"error calling ListView_RedrawItems in uiTableModelRowInserted()");
	}
}

// TODO compare LVM_UPDATE and LVM_REDRAWITEMS
void uiTableModelRowChanged(uiTableModel *m, int index)
{
	for (auto t : *(m->tables))
		if (SendMessageW(t->hwnd, LVM_UPDATE, (WPARAM) index, 0) == (LRESULT) (-1))
			logLastError(L"error calling LVM_UPDATE in uiTableModelRowChanged()");
}

void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	for (auto t : *(m->tables)) {
		if (ListView_DeleteItem(t->hwnd, oldIndex) == -1)
			logLastError(L"error calling ListView_DeleteItem() in uiTableModelRowDeleted()");
		// redraw every row from the new nth row down to simulate removing the old nth row
		if (ListView_RedrawItems(t->hwnd, oldIndex, ListView_GetItemCount(t->hwnd)-1) == -1)
			logLastError(L"error calling ListView_RedrawItems() in uiTableModelRowDeleted()");
	}
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

uiTableModelHandler *uiprivTableModelHandler(uiTableModel *m)
{
	return m->mh;
}

// TODO explain all this
static LRESULT CALLBACK tableSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIDSubclass, DWORD_PTR dwRefData)
{
	uiTable *t = (uiTable *) dwRefData;
	NMHDR *nmhdr = (NMHDR *) lParam;
	bool finishEdit, abortEdit;
	HWND header;
	LRESULT lResult;
	HRESULT hr;

	finishEdit = false;
	abortEdit = false;
	switch (uMsg) {
	case WM_TIMER:
		if (wParam == (WPARAM) (&(t->inDoubleClickTimer))) {
			t->inDoubleClickTimer = FALSE;
			// TODO check errors
			KillTimer(hwnd, wParam);
			return 0;
		}
		if (wParam != (WPARAM) t)
			break;
		// TODO only increment and update if visible?
		for (auto &i : *(t->indeterminatePositions)) {
			i.second++;
			// TODO check errors
			SendMessageW(hwnd, LVM_UPDATE, (WPARAM) (i.first.first), 0);
		}
		return 0;
	case WM_LBUTTONDOWN:
		t->inLButtonDown = TRUE;
		lResult = DefSubclassProc(hwnd, uMsg, wParam, lParam);
		t->inLButtonDown = FALSE;
		return lResult;
	case WM_COMMAND:
		if (HIWORD(wParam) == EN_UPDATE) {
			// the real list view resizes the edit control on this notification specifically
			hr = uiprivTableResizeWhileEditing(t);
			if (hr != S_OK) {
				// TODO
			}
			break;
		}
		// the real list view accepts changes in this case
		if (HIWORD(wParam) == EN_KILLFOCUS)
			finishEdit = true;
		break;		// don't override default handling
	case WM_NOTIFY:
		// list view accepts changes on column resize, but does not provide such notifications :/
		header = (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, 0, 0);
		if (nmhdr->hwndFrom == header) {
			NMHEADERW *nm = (NMHEADERW *) nmhdr;

			switch (nmhdr->code) {
			case HDN_ITEMCHANGED:
				if ((nm->pitem->mask & HDI_WIDTH) == 0)
					break;
				// fall through
			case HDN_DIVIDERDBLCLICK:
			case HDN_TRACK:
			case HDN_ENDTRACK:
				finishEdit = true;
			}
		}
		// I think this mirrors the WM_COMMAND one above... TODO
		if (nmhdr->code == NM_KILLFOCUS)
			finishEdit = true;
		break;		// don't override default handling
	case LVM_CANCELEDITLABEL:
		finishEdit = true;
		// TODO properly imitate notifiactions
		break;		// don't override default handling
	// TODO finish edit on WM_WINDOWPOSCHANGING and WM_SIZE?
	// for the next three: this item is about to go away; don't bother keeping changes
	case LVM_SETITEMCOUNT:
		if (wParam <= t->editedItem)
			abortEdit = true;
		break;		// don't override default handling
	case LVM_DELETEITEM:
		if (wParam == t->editedItem)
			abortEdit = true;
		break;		// don't override default handling
	case LVM_DELETEALLITEMS:
		abortEdit = true;
		break;		// don't override default handling
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, tableSubProc, uIDSubclass) == FALSE)
			logLastError(L"RemoveWindowSubclass()");
		// fall through
	}
	if (finishEdit) {
		hr = uiprivTableFinishEditingText(t);
		if (hr != S_OK) {
			// TODO
		}
	} else if (abortEdit) {
		hr = uiprivTableAbortEditingText(t);
		if (hr != S_OK) {
			// TODO
		}
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

int uiprivTableProgress(uiTable *t, int item, int subitem, int modelColumn, LONG *pos)
{
	uiTableValue *value;
	int progress;
	std::pair<int, int> p;
	std::map<std::pair<int, int>, LONG>::iterator iter;
	bool startTimer = false;
	bool stopTimer = false;

	value = uiprivTableModelCellValue(t->model, item, modelColumn);
	progress = uiTableValueInt(value);
	uiFreeTableValue(value);

	p.first = item;
	p.second = subitem;
	iter = t->indeterminatePositions->find(p);
	if (iter == t->indeterminatePositions->end()) {
		if (progress == -1) {
			startTimer = t->indeterminatePositions->size() == 0;
			(*(t->indeterminatePositions))[p] = 0;
			if (pos != NULL)
				*pos = 0;
		}
	} else
		if (progress != -1) {
			t->indeterminatePositions->erase(p);
			stopTimer = t->indeterminatePositions->size() == 0;
		} else if (pos != NULL)
			*pos = iter->second;

	if (startTimer)
		// the interval shown here is PBM_SETMARQUEE's default
		// TODO should we pass a function here instead? it seems to be called by DispatchMessage(), not DefWindowProc(), but I'm still unsure
		if (SetTimer(t->hwnd, (UINT_PTR) t, 30, NULL) == 0)
			logLastError(L"SetTimer()");
	if (stopTimer)
		if (KillTimer(t->hwnd, (UINT_PTR) (&t)) == 0)
			logLastError(L"KillTimer()");

	return progress;
}

void uiTableHeaderSetSortIndicator(uiTable *t, int column, uiSortIndicator indicator)
{
	HWND lvhdr;
	int fmt;

	if (indicator == uiSortIndicatorAscending)
		fmt = HDF_SORTUP;
	else if (indicator == uiSortIndicatorDescending)
		fmt = HDF_SORTDOWN;
	else
		fmt = 0;

	lvhdr = (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, 0, 0);
	if (lvhdr) {
		HDITEM hdri = {};
		hdri.mask = HDI_FORMAT;
		if (SendMessageW(lvhdr, HDM_GETITEM, (WPARAM) column, (LPARAM) &hdri)) {
			hdri.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			hdri.fmt |= fmt;
			SendMessageW(lvhdr, HDM_SETITEM, (WPARAM) column, (LPARAM) &hdri);
		}
	}
}

uiSortIndicator uiTableHeaderSortIndicator(uiTable *t, int column)
{
	HWND lvhdr;

	lvhdr = (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, 0, 0);
	if (lvhdr) {
		HDITEM hdri = {};
		hdri.mask = HDI_FORMAT;
		if (SendMessageW(lvhdr, HDM_GETITEM, (WPARAM) column, (LPARAM) &hdri)) {
			if (hdri.fmt & HDF_SORTUP)
				return uiSortIndicatorAscending;
			if (hdri.fmt & HDF_SORTDOWN)
				return uiSortIndicatorDescending;
		}
	}
	return uiSortIndicatorNone;
}

void uiTableHeaderOnClicked(uiTable *t, void (*f)(uiTable *table, int column, void *data), void *data)
{
	t->headerOnClicked = f;
	t->headerOnClickedData = data;
}

static void defaultHeaderOnClicked(uiTable *table, int column, void *data)
{
	// do nothing
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

uiTableSelection* uiTableGetSelection(uiTable *t)
{
	unsigned i = 0;
	int iPos = -1;
	uiTableSelection *s = uiprivNew(uiTableSelection);

	s->NumRows = ListView_GetSelectedCount(t->hwnd);
	s->Rows = NULL;

	if (s->NumRows > 0)
		s->Rows = (int*) uiprivAlloc(s->NumRows * sizeof(*s->Rows), "uiTableSelection->Rows");

	while ((iPos = ListView_GetNextItem(t->hwnd, iPos, LVNI_SELECTED)) != -1)
		s->Rows[i++] = iPos;

	return s;
}

void uiTableSetSelection(uiTable *t, uiTableSelection *sel)
{
	int i;

	if ((t->selectionMode == uiTableSelectionModeNone && sel->NumRows > 0) ||
	    (t->selectionMode == uiTableSelectionModeZeroOrOne && sel->NumRows > 1) ||
	    (t->selectionMode == uiTableSelectionModeOne && sel->NumRows > 1)) {
	    // TODO log error
	    return;
	}

	/* clear selection */
	ListView_SetItemState(t->hwnd, -1, 0, LVIS_SELECTED);

	for (i = 0; i < sel->NumRows; ++i)
		ListView_SetItemState(t->hwnd, sel->Rows[i], LVIS_SELECTED, LVIS_SELECTED);
}

void _uiTableSignalOnSelectionChanged(uiTable *t)
{
	if (!t->maskOnSelectionChanged)
		t->onSelectionChanged(t, t->onSelectionChangedData);
}

// TODO properly integrate compound statements
static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiTable *t = uiTable(c);
	HRESULT hr;

	switch (nmhdr->code) {
	case LVN_GETDISPINFO:
		hr = uiprivTableHandleLVN_GETDISPINFO(t, (NMLVDISPINFOW *) nmhdr, lResult);
		if (hr != S_OK) {
			// TODO
			return FALSE;
		}
		return TRUE;
	case NM_CUSTOMDRAW:
		hr = uiprivTableHandleNM_CUSTOMDRAW(t, (NMLVCUSTOMDRAW *) nmhdr, lResult);
		if (hr != S_OK) {
			// TODO
			return FALSE;
		}
		return TRUE;
	case NM_CLICK:
#if 0
		{
			NMITEMACTIVATE *nm = (NMITEMACTIVATE *) nmhdr;
			LVHITTESTINFO ht;
			WCHAR buf[256];

			ZeroMemory(&ht, sizeof (LVHITTESTINFO));
			ht.pt = nm->ptAction;
			if (SendMessageW(t->hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM) (&ht)) == (LRESULT) (-1))
				MessageBoxW(GetAncestor(t->hwnd, GA_ROOT), L"No hit", L"No hit", MB_OK);
			else {
				wsprintf(buf, L"item %d subitem %d htflags 0x%I32X",
					ht.iItem, ht.iSubItem, ht.flags);
				MessageBoxW(GetAncestor(t->hwnd, GA_ROOT), buf, buf, MB_OK);
			}
		}
		*lResult = 0;
		return TRUE;
#else
		{
			LVHITTESTINFO ht = {};
			ht.pt = ((NMITEMACTIVATE *)nmhdr)->ptAction;
			if (SendMessageW(t->hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM) &ht) == -1)
				return FALSE;
			(*(t->onRowClicked))(t, ht.iItem, t->onRowClickedData);

			// Handle editing
			hr = uiprivTableHandleNM_CLICK(t, (NMITEMACTIVATE *) nmhdr, lResult);
			if (hr != S_OK) {
				// TODO
				return FALSE;
			}
			return TRUE;
		}
#endif
	case NM_DBLCLK:
		{
			LVHITTESTINFO ht = {};
			ht.pt = ((NMITEMACTIVATE *)nmhdr)->ptAction;
			if (SendMessageW(t->hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM) &ht) == -1)
				return FALSE;
			(*(t->onRowDoubleClicked))(t, ht.iItem, t->onRowDoubleClickedData);
			return TRUE;
		}
	case LVN_ITEMCHANGED:
		{
			NMLISTVIEW *nm = (NMLISTVIEW *) nmhdr;
			UINT oldFocused, newFocused;
			UINT oldSelected, newSelected;
			HRESULT hr;
			int nSelected;

			oldSelected = nm->uOldState & LVIS_SELECTED;
			newSelected = nm->uNewState & LVIS_SELECTED;
			oldFocused = nm->uOldState & LVIS_FOCUSED;
			newFocused = nm->uNewState & LVIS_FOCUSED;

			switch (t->selectionMode) {
			case uiTableSelectionModeNone:
				if (newSelected || newFocused)
					ListView_SetItemState(t->hwnd, nm->iItem, 0, LVIS_SELECTED|LVIS_FOCUSED);
				break;
			case uiTableSelectionModeOne:
				// Ignore deselect all
				if (nm->iItem == -1)
					break;
				// Prevent deselection via CTRL+SPACE (win32 bug)
				if (oldSelected && !newSelected) {
					t->maskOnSelectionChanged = TRUE;
					ListView_SetItemState(t->hwnd, nm->iItem, LVIS_SELECTED, LVIS_SELECTED);
					t->maskOnSelectionChanged = FALSE;
				}
				// Signal selection change on selection of a new item
				if (nm->iItem != t->lastFocusedItem && !oldSelected && newSelected)
					_uiTableSignalOnSelectionChanged(t);

				t->lastFocusedItem = nm->iItem;
				t->lastFocusedItemIsSelected = TRUE;
				break;
			case uiTableSelectionModeZeroOrOne:
				// Set focused item to be selected
				if (!oldFocused && newFocused) {
					t->lastFocusedItem = nm->iItem;
					t->lastFocusedItemIsSelected = FALSE;
					ListView_SetItemState(t->hwnd, -1, 0, LVIS_SELECTED);
					ListView_SetItemState(t->hwnd, nm->iItem, LVIS_SELECTED, LVIS_SELECTED);
					break;
				}
				/* Ignore CTRL+SHIFT+SPACE on the focused item if selected
				 * as it immediately does a reselect again */
				if (nm->iItem == t->lastFocusedItem && t->lastFocusedItemIsSelected &&
				    HIBYTE(GetKeyState(VK_CONTROL)) &&
				    HIBYTE(GetKeyState(VK_SHIFT)) &&
				    HIBYTE(GetKeyState(VK_SPACE)))
					break;
				if (nm->iItem == t->lastFocusedItem && t->lastFocusedItemIsSelected &&
				    oldSelected && !newSelected) {
					t->lastFocusedItemIsSelected = FALSE;
					_uiTableSignalOnSelectionChanged(t);
				}
				if (nm->iItem == t->lastFocusedItem && !t->lastFocusedItemIsSelected &&
				    !oldSelected && newSelected) {
					t->lastFocusedItemIsSelected = TRUE;
					_uiTableSignalOnSelectionChanged(t);
				}
				break;
			case uiTableSelectionModeZeroOrMany:
				nSelected = ListView_GetSelectedCount(t->hwnd);
				// Ignore deselect all
				if (nm->iItem == -1)
					break;
				/* CTRL+SHIFT+SPACE on the focused item */
				if (nm->iItem == t->lastFocusedItem &&
				    HIBYTE(GetKeyState(VK_CONTROL)) &&
				    HIBYTE(GetKeyState(VK_SHIFT)) &&
				    HIBYTE(GetKeyState(VK_SPACE))) {
					/* Do not signal deselect as it immediately
					 * does a reselect again */
					if (t->lastFocusedItemIsSelected) {
						/* Deselect the focused item if others are still
						 * selected to trigger a signal later on */
						if (oldSelected && !newSelected &&
						    ListView_GetSelectedCount(t->hwnd) > 0)
							t->lastFocusedItemIsSelected = FALSE;
						break;
					}
					/* Do not signal select if multiple items are still
					 * selected, as these will still get deselect later
					 * on in the sequence */
					if (!t->lastFocusedItemIsSelected &&
					    newSelected && ListView_GetSelectedCount(t->hwnd) > 1)
						break;
				}
				/* Do not signal select of a selected item unless
				 * the selection count has changed (other items
				 * have been deselected) */
				if (nm->iItem == t->lastFocusedItem && t->lastFocusedItemIsSelected &&
				    newSelected && nSelected == t->lastNumSelected)
					break;

				// Single item de/select
				if ((!oldSelected && newSelected) ||
				    (oldSelected && !newSelected) ||
				    // SHIFT multi select
				    (!oldFocused && newFocused && HIBYTE(GetKeyState(VK_SHIFT)) &&
				     nm->iItem != ListView_GetSelectionMark(t->hwnd)))
					_uiTableSignalOnSelectionChanged(t);

				if (!oldFocused && newFocused)
					t->lastFocusedItem = nm->iItem;
				if (nm->iItem == t->lastFocusedItem)
					t->lastFocusedItemIsSelected = ListView_GetItemState(t->hwnd, t->lastFocusedItem,
											     LVIS_SELECTED) & LVIS_SELECTED;
				t->lastNumSelected = nSelected;

				break;
			}

			// TODO clean up these if cases
			if (!t->inLButtonDown && t->edit == NULL)
				return FALSE;

			if (t->inLButtonDown && !oldFocused && newFocused) {
				t->inDoubleClickTimer = TRUE;
				// TODO check error
				SetTimer(t->hwnd, (UINT_PTR) (&(t->inDoubleClickTimer)),
					GetDoubleClickTime(), NULL);
				*lResult = 0;
				return TRUE;
			}
			// the nm->iItem == -1 case is because that is used if "the change has been applied to all items in the list view"
			if (t->edit != NULL && oldFocused && !newFocused && (t->editedItem == nm->iItem || nm->iItem == -1)) {
				// TODO see if the real list view accepts or rejects changes here; Windows Explorer accepts
				hr = uiprivTableFinishEditingText(t);
				if (hr != S_OK) {
					// TODO
					return FALSE;
				}
				*lResult = 0;
				return TRUE;
			}
			return FALSE;
		}
	case LVN_ODSTATECHANGED:
		{
			NMLVODSTATECHANGE *nm = (NMLVODSTATECHANGE *) nmhdr;
			int nSelected;

			switch (t->selectionMode) {
				case uiTableSelectionModeZeroOrOne:
					// Windows somehow decides to ignore the first request
					ListView_SetItemState(t->hwnd, -1, 0, LVIS_SELECTED);
					ListView_SetItemState(t->hwnd, -1, 0, LVIS_SELECTED);
					break;
				case uiTableSelectionModeZeroOrMany:
					nSelected = ListView_GetSelectedCount(t->hwnd);
					/* Signal selection change on SHIFT+CLICK or SHIFT+SPACE
					 * when doing so on an item that has been focused via
					 * CTRL+UP or CTRL+DOWN */
					if (nSelected != t->lastNumSelected &&
					    ((nm->iFrom == t->lastFocusedItem && nm->iTo == ListView_GetSelectionMark(t->hwnd)) ||
					    (nm->iTo == t->lastFocusedItem && nm->iFrom == ListView_GetSelectionMark(t->hwnd)))) {
						t->lastFocusedItemIsSelected = TRUE;
						_uiTableSignalOnSelectionChanged(t);
					}
					t->lastNumSelected = nSelected;
					break;
			}
			return TRUE;
		}
	case LVN_COLUMNCLICK:
		{
			NMLISTVIEW *nm = (NMLISTVIEW *) nmhdr;

			hr = uiprivTableFinishEditingText(t);
			if (hr != S_OK) {
				// TODO
				return FALSE;
			}
			t->headerOnClicked(t, nm->iSubItem, t->headerOnClickedData);
			return TRUE;
		}
	// the real list view accepts changes when scrolling or clicking column headers
	case LVN_BEGINSCROLL:
		hr = uiprivTableFinishEditingText(t);
		if (hr != S_OK) {
			// TODO
			return FALSE;
		}
		*lResult = 0;
		return TRUE;
	}
	return FALSE;
}

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);
	uiTableModel *model = t->model;
	std::vector<uiTable *>::iterator it;
	HRESULT hr;

	hr = uiprivTableAbortEditingText(t);
	if (hr != S_OK) {
		// TODO
	}
	uiWindowsUnregisterWM_NOTIFYHandler(t->hwnd);
	uiWindowsEnsureDestroyWindow(t->hwnd);
	// detach table from model
	for (it = model->tables->begin(); it != model->tables->end(); it++) {
		if (*it == t) {
			model->tables->erase(it);
			break;
		}
	}
	// free the columns
	for (auto col : *(t->columns))
		uiprivFree(col);
	delete t->columns;
	// t->imagelist will be automatically destroyed
	delete t->indeterminatePositions;
	uiFreeControl(uiControl(t));
}

uiWindowsControlAllDefaultsExceptDestroy(uiTable)

// suggested listview sizing from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing:
// "columns widths that avoid truncated data x an integral number of items"
// Don't think that'll cut it when some cells have overlong data (eg
// stupidly long URLs). So for now, just hardcode a minimum.
// TODO Investigate using LVM_GETHEADER/HDM_LAYOUT here
// TODO investigate using LVM_APPROXIMATEVIEWRECT here
#define tableMinWidth 107		/* in line with other controls */
#define tableMinHeight (14 * 3)	/* header + 2 lines (roughly) */

static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiTable *t = uiTable(c);
	uiWindowsSizing sizing;
	int x, y;

	x = tableMinWidth;
	y = tableMinHeight;
	uiWindowsGetSizing(t->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

static uiprivTableColumnParams *appendColumn(uiTable *t, const char *name, int colfmt)
{
	WCHAR *wstr;
	LVCOLUMNW lvc;
	uiprivTableColumnParams *p;

	ZeroMemory(&lvc, sizeof (LVCOLUMNW));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = colfmt;
	lvc.cx = 120;			// TODO
	wstr = toUTF16(name);
	lvc.pszText = wstr;
	if (SendMessageW(t->hwnd, LVM_INSERTCOLUMNW, t->nColumns, (LPARAM) (&lvc)) == (LRESULT) (-1))
		logLastError(L"error calling LVM_INSERTCOLUMNW in appendColumn()");
	uiprivFree(wstr);
	t->nColumns++;

	p = uiprivNew(uiprivTableColumnParams);
	p->textModelColumn = -1;
	p->textEditableModelColumn = -1;
	p->textParams = uiprivDefaultTextColumnOptionalParams;
	p->imageModelColumn = -1;
	p->checkboxModelColumn = -1;
	p->checkboxEditableModelColumn = -1;
	p->progressBarModelColumn = -1;
	p->buttonModelColumn = -1;
	t->columns->push_back(p);
	return p;
}

void uiTableAppendTextColumn(uiTable *t, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
}

void uiTableAppendImageColumn(uiTable *t, const char *name, int imageModelColumn)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->imageModelColumn = imageModelColumn;
}

void uiTableAppendImageTextColumn(uiTable *t, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
	p->imageModelColumn = imageModelColumn;
}

void uiTableAppendCheckboxColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->checkboxModelColumn = checkboxModelColumn;
	p->checkboxEditableModelColumn = checkboxEditableModelColumn;
}

void uiTableAppendCheckboxTextColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
	p->checkboxModelColumn = checkboxModelColumn;
	p->checkboxEditableModelColumn = checkboxEditableModelColumn;
}

void uiTableAppendProgressBarColumn(uiTable *t, const char *name, int progressModelColumn)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->progressBarModelColumn = progressModelColumn;
}

void uiTableAppendButtonColumn(uiTable *t, const char *name, int buttonModelColumn, int buttonClickableModelColumn)
{
	uiprivTableColumnParams *p;

	// TODO see if we can get rid of this parameter
	p = appendColumn(t, name, LVCFMT_LEFT);
	p->buttonModelColumn = buttonModelColumn;
	p->buttonClickableModelColumn = buttonClickableModelColumn;
}

int uiTableHeaderVisible(uiTable *t)
{
	HWND header =  (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, 0, 0);
	if (header) {
		LONG style = GetWindowLong(header, GWL_STYLE);
		return !(style & HDS_HIDDEN);
	}
	uiprivImplBug("window handle %p unknown error from send LVM_GETHEADER", t->hwnd);
	return 0;
}

void uiTableHeaderSetVisible(uiTable *t, int visible)
{
	LONG style = GetWindowLong(t->hwnd, GWL_STYLE);
	if (visible)
		SetWindowLong(t->hwnd, GWL_STYLE, style & ~LVS_NOCOLUMNHEADER);
	else
		SetWindowLong(t->hwnd, GWL_STYLE, style | LVS_NOCOLUMNHEADER);
}

uiTableSelectionMode uiTableGetSelectionMode(uiTable *t)
{
	return t->selectionMode;
}

void uiTableSetSelectionMode(uiTable *t, uiTableSelectionMode mode)
{
	LONG style = GetWindowLong(t->hwnd, GWL_STYLE);

	t->selectionMode = mode;

	switch (t->selectionMode) {
	case uiTableSelectionModeOne:
	case uiTableSelectionModeZeroOrOne:
		if (ListView_GetSelectedCount(t->hwnd) == 1) {
			// Ensure cached variables and focus are set up correctly
			t->lastFocusedItem = ListView_GetNextItem(t->hwnd, -1, LVNI_SELECTED);
			t->lastFocusedItemIsSelected = TRUE;
			t->lastNumSelected = 1;
			t->maskOnSelectionChanged = TRUE;
			ListView_SetItemState(t->hwnd, t->lastFocusedItem, LVIS_FOCUSED, LVIS_FOCUSED);
			t->maskOnSelectionChanged = FALSE;
			break;
		}
		// Fall through
	case uiTableSelectionModeNone:
		// Reset the table to it's initial state
		ListView_SetItemState(t->hwnd, -1, 0, LVIS_SELECTED|LVIS_FOCUSED);
		// Ensure we do not match deselect all (-1)
		t->lastFocusedItem = -2;
		t->lastFocusedItemIsSelected = FALSE;
		t->lastNumSelected = 0;
		break;
	case uiTableSelectionModeZeroOrMany:
		t->lastNumSelected = ListView_GetSelectedCount(t->hwnd);
		break;
	default:
		uiprivUserBug("Invalid table selection mode %d", mode);
		return;
	}

	switch (t->selectionMode) {
	case uiTableSelectionModeNone:
	case uiTableSelectionModeOne:
		SetWindowLong(t->hwnd, GWL_STYLE, style | LVS_SINGLESEL);
		break;
	case uiTableSelectionModeZeroOrOne:
	case uiTableSelectionModeZeroOrMany:
		SetWindowLong(t->hwnd, GWL_STYLE, style & ~LVS_SINGLESEL);
		break;
	}
}

uiTable *uiNewTable(uiTableParams *p)
{
	uiTable *t;
	int n;
	HRESULT hr;

	uiWindowsNewControl(uiTable, t);

	t->columns = new std::vector<uiprivTableColumnParams *>;
	t->model = p->Model;
	t->backgroundColumn = p->RowBackgroundColorModelColumn;
	uiTableHeaderOnClicked(t, defaultHeaderOnClicked, NULL);
	uiTableOnSelectionChanged(t, defaultOnSelectionChanged, NULL);

	// WS_CLIPCHILDREN is here to prevent drawing over the edit box used for editing text
	t->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, L"",
		LVS_REPORT | LVS_OWNERDATA | WS_CLIPCHILDREN | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL,
		hInstance, NULL,
		TRUE);
	t->model->tables->push_back(t);
	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));

	// TODO: try LVS_EX_AUTOSIZECOLUMNS
	// TODO check error
	SendMessageW(t->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE,
		(WPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES),
		(LPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES));
	n = uiprivTableModelNumRows(t->model);
	if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) n, 0) == 0)
		logLastError(L"error calling LVM_SETITEMCOUNT in uiNewTable()");

	hr = uiprivUpdateImageListSize(t);
	if (hr != S_OK) {
		// TODO
	}

	t->indeterminatePositions = new std::map<std::pair<int, int>, LONG>;
	if (SetWindowSubclass(t->hwnd, tableSubProc, 0, (DWORD_PTR) t) == FALSE)
		logLastError(L"SetWindowSubclass()");

	uiTableOnRowClicked(t, defaultOnRowClicked, NULL);
	uiTableOnRowDoubleClicked(t, defaultOnRowDoubleClicked, NULL);
	uiTableSetSelectionMode(t, uiTableSelectionModeZeroOrOne);

	return t;
}

int uiTableColumnWidth(uiTable *t, int column)
{
	return SendMessageW(t->hwnd, LVM_GETCOLUMNWIDTH, (WPARAM) column, 0);
}

void uiTableColumnSetWidth(uiTable *t, int column, int width)
{
	if (width == -1)
		width = LVSCW_AUTOSIZE_USEHEADER;

	SendMessageW(t->hwnd, LVM_SETCOLUMNWIDTH, (WPARAM) column, (LPARAM) width);
}
