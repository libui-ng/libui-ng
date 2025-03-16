// 8 april 2015
#include "uipriv_windows.hpp"

// TODO there's alpha darkening of text going on in read-only ones; something is up in our parent logic

struct uiMultilineEntry {
	uiWindowsControl c;
	HWND hwnd;
	void (*onChanged)(uiMultilineEntry *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
};

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiMultilineEntry *e = uiMultilineEntry(c);

	if (code != EN_CHANGE)
		return FALSE;
	if (e->inhibitChanged)
		return FALSE;
	(*(e->onChanged))(e, e->onChangedData);
	*lResult = 0;
	return TRUE;
}

static void uiMultilineEntryDestroy(uiControl *c)
{
	uiMultilineEntry *e = uiMultilineEntry(c);

	uiWindowsUnregisterWM_COMMANDHandler(e->hwnd);
	uiWindowsEnsureDestroyWindow(e->hwnd);
	uiFreeControl(uiControl(e));
}

uiWindowsControlAllDefaultsExceptDestroy(uiMultilineEntry)

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
// LONGTERM change this for multiline text boxes (longterm because how?)
#define entryHeight 14

static void uiMultilineEntryMinimumSize(const uiWindowsControl *c, int *width, int *height)
{
	uiMultilineEntry *e = uiMultilineEntry(c);
	uiWindowsSizing sizing;
	int x, y;

	x = entryWidth;
	y = entryHeight;
	uiWindowsGetSizing(e->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

static void defaultOnChanged(uiMultilineEntry *e, void *data)
{
	// do nothing
}

char *uiMultilineEntryText(uiMultilineEntry *e)
{
	char *out;

	out = uiWindowsWindowText(e->hwnd);
	CRLFtoLF(out);
	return out;
}

void uiMultilineEntrySetText(uiMultilineEntry *e, const char *text)
{
	char *crlf;

	// doing this raises an EN_CHANGED
	e->inhibitChanged = TRUE;
	crlf = LFtoCRLF(text);
	uiWindowsSetWindowText(e->hwnd, crlf);
	uiprivFree(crlf);
	e->inhibitChanged = FALSE;
	// don't queue the control for resize; entry sizes are independent of their contents
}

void uiMultilineEntryAppend(uiMultilineEntry *e, const char *text)
{
	DWORD selStart, selEnd;
	LRESULT l;
	char *crlf;
	WCHAR *wtext;

	// doing this raises an EN_CHANGED
	e->inhibitChanged = TRUE;

	// Save current selection
	SendMessageW(e->hwnd, EM_GETSEL, (WPARAM) &selStart, (LPARAM) &selEnd);
	// Append by replacing an empty selection at the end of the input
	l = SendMessageW(e->hwnd, WM_GETTEXTLENGTH, 0, 0);
	Edit_SetSel(e->hwnd, l, l);
	crlf = LFtoCRLF(text);
	wtext = toUTF16(crlf);
	uiprivFree(crlf);
	Edit_ReplaceSel(e->hwnd, wtext);
	uiprivFree(wtext);
	// Restore selection
	Edit_SetSel(e->hwnd, selStart, selEnd);

	e->inhibitChanged = FALSE;
}

void uiMultilineEntryOnChanged(uiMultilineEntry *e, void (*f)(uiMultilineEntry *, void *), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiMultilineEntryReadOnly(uiMultilineEntry *e)
{
	return (getStyle(e->hwnd) & ES_READONLY) != 0;
}

void uiMultilineEntrySetReadOnly(uiMultilineEntry *e, int readonly)
{
	if (Edit_SetReadOnly(e->hwnd, readonly) == 0)
		logLastError(L"error setting uiMultilineEntry read-only state");
}

static uiMultilineEntry *finishMultilineEntry(DWORD style)
{
	uiMultilineEntry *e;

	uiWindowsNewControl(uiMultilineEntry, e);

	e->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"edit", L"",
		ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN | WS_TABSTOP | WS_VSCROLL | style,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(e->hwnd, onWM_COMMAND, uiControl(e));
	uiMultilineEntryOnChanged(e, defaultOnChanged, NULL);

	return e;
}

uiMultilineEntry *uiNewMultilineEntry(void)
{
	return finishMultilineEntry(0);
}

uiMultilineEntry *uiNewNonWrappingMultilineEntry(void)
{
	return finishMultilineEntry(WS_HSCROLL | ES_AUTOHSCROLL);
}
