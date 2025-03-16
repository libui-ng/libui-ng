// 8 april 2015
#include "uipriv_windows.hpp"

struct uiEntry {
	uiWindowsControl c;
	HWND hwnd;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
};

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiEntry *e = uiEntry(c);

	if (code != EN_CHANGE)
		return FALSE;
	if (e->inhibitChanged)
		return FALSE;
	(*(e->onChanged))(e, e->onChangedData);
	*lResult = 0;
	return TRUE;
}

static void uiEntryDestroy(uiControl *c)
{
	uiEntry *e = uiEntry(c);

	uiWindowsUnregisterWM_COMMANDHandler(e->hwnd);
	uiWindowsEnsureDestroyWindow(e->hwnd);
	uiFreeControl(uiControl(e));
}

uiWindowsControlAllDefaultsExceptDestroy(uiEntry)

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void uiEntryMinimumSize(const uiWindowsControl *c, int *width, int *height)
{
	uiEntry *e = uiEntry(c);
	uiWindowsSizing sizing;
	int x, y;

	x = entryWidth;
	y = entryHeight;
	uiWindowsGetSizing(e->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

char *uiEntryText(const uiEntry *e)
{
	return uiWindowsWindowText(e->hwnd);
}

void uiEntrySetText(uiEntry *e, const char *text)
{
	int l;
	// doing this raises an EN_CHANGED
	e->inhibitChanged = TRUE;
	uiWindowsSetWindowText(e->hwnd, text);
	l = (int)strlen(text);
	// Only set the cursor if the entry has focus to avoid weird scrolling upon window
	// creation. Cursor placement is otherwise determined by mouse position upon click.
	if (GetFocus() == e->hwnd)
		Edit_SetSel(e->hwnd, l, l);
	e->inhibitChanged = FALSE;
	// don't queue the control for resize; entry sizes are independent of their contents
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *, void *), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiEntryReadOnly(const uiEntry *e)
{
	return (getStyle(e->hwnd) & ES_READONLY) != 0;
}

void uiEntrySetReadOnly(uiEntry *e, int readonly)
{
	if (Edit_SetReadOnly(e->hwnd, readonly) == 0)
		logLastError(L"error setting uiEntry read-only state");
}

static uiEntry *finishNewEntry(DWORD style)
{
	uiEntry *e;

	uiWindowsNewControl(uiEntry, e);

	e->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"edit", L"",
		style | ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(e->hwnd, onWM_COMMAND, uiControl(e));
	uiEntryOnChanged(e, defaultOnChanged, NULL);

	return e;
}

uiEntry *uiNewEntry(void)
{
	return finishNewEntry(0);
}

uiEntry *uiNewPasswordEntry(void)
{
	return finishNewEntry(ES_PASSWORD);
}

uiEntry *uiNewSearchEntry(void)
{
	uiEntry *e;
	HRESULT hr;

	e = finishNewEntry(0);

	hr = SetWindowTheme(e->hwnd, L"SearchBoxEditComposited", NULL);
	if (hr != S_OK || !IsAppThemed()) {
		//TODO log: Failed to apply search box theme.
	}

	return e;
}
