// 8 april 2015
#include "uipriv_windows.hpp"

struct uiSpinbox {
	uiWindowsControl c;
	HWND hwnd;
	HWND edit;
	HWND updown;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
	BOOL inhibitChanged;

	// The amount to increase by each step
	double step_size;

	// The current value of the spinbox
	double value;

	// The number of digits to display - 0 for uiSpinbox
	int precision;

	// The maximum value
	double maximum;

	// The minimum value
	double minimum;
};

// utility functions

static double update_value(uiSpinbox *s, double value)
{
	wchar_t buf[64];

	s->value = fmax(s->minimum, fmin(value, s->maximum));

	s->inhibitChanged = TRUE;
	swprintf(buf, sizeof buf, L"%.*f", s->precision, s->value);
	SendMessageW(s->edit, WM_SETTEXT, 0, (LPARAM) buf);
	s->inhibitChanged = FALSE;

	return s->value;
}

// control implementation

// TODO assign lResult
static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiSpinbox *s = (uiSpinbox *) c;
	double val;
	WCHAR *wtext;
	WCHAR *end;

	if (code == EN_KILLFOCUS) {
		update_value(s, s->value);
		return FALSE;
	}
	if (code != EN_CHANGE)
		return FALSE;
	if (s->inhibitChanged)
		return FALSE;

	// We want to allow typing negative numbers; the natural way to do so is to start with a -.
	// However, if we just have the code below, the up-down will catch the bare - and reject it.
	// Let's fix that.
	// This won't handle leading spaces, but spaces aren't allowed *anyway*.
	wtext = windowText(s->edit);
	if (wcscmp(wtext, L"-") == 0) {
		uiprivFree(wtext);
		return TRUE;
	}
	val = wcstod(wtext, &end);
	if (val == 0.0 && wtext == end) {
		uiprivFree(wtext);
		return FALSE;
	}
	s->value = fmax(s->minimum, fmin(val, s->maximum));
	uiprivFree(wtext);
	(*(s->onChanged))(s, s->onChangedData);
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiSpinbox *s = (uiSpinbox *) c;
	double value;
	LPNMUPDOWN lpnmud;

	if (nmhdr->code != UDN_DELTAPOS)
		return FALSE;

	lpnmud = (LPNMUPDOWN )nmhdr;
	value = s->value + (-(double)lpnmud->iDelta) * s->step_size;

	update_value(s, value);

	(*(s->onChanged))(s, s->onChangedData);
	return true;
}

static void uiSpinboxDestroy(uiControl *c)
{
	uiSpinbox *s = uiSpinbox(c);

	uiWindowsUnregisterWM_COMMANDHandler(s->edit);
	uiWindowsUnregisterWM_NOTIFYHandler(s->updown);
	uiWindowsEnsureDestroyWindow(s->updown);
	uiWindowsEnsureDestroyWindow(s->edit);
	uiWindowsEnsureDestroyWindow(s->hwnd);
	uiFreeControl(uiControl(s));
}

// TODO SyncEnableState
uiWindowsControlAllDefaultsExceptDestroy(uiSpinbox)

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
// TODO reduce this?
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void uiSpinboxMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiSpinbox *s = uiSpinbox(c);
	uiWindowsSizing sizing;
	int x, y;

	x = entryWidth;
	y = entryHeight;
	// note that we go by the edit here
	uiWindowsGetSizing(s->edit, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

static void spinboxArrangeChildren(uiSpinbox *s)
{
	LONG_PTR controlID;
	HWND insertAfter;

	controlID = 100;
	insertAfter = NULL;
	uiWindowsEnsureAssignControlIDZOrder(s->edit, &controlID, &insertAfter);
	uiWindowsEnsureAssignControlIDZOrder(s->updown, &controlID, &insertAfter);
}

// an up-down control will only properly position itself the first time
// stupidly, there are no messages to force a size calculation, nor can I seem to reset the buddy window to force a new position
// alas, we have to make a new up/down control each time :(
static void recreateUpDown(uiSpinbox *s)
{
	if (s->updown != NULL) {
		uiWindowsEnsureDestroyWindow(s->updown);
	}
	s->inhibitChanged = TRUE;
	s->updown = CreateWindowExW(0,
		UPDOWN_CLASSW, L"",
		// no WS_VISIBLE; we set visibility ourselves
		// up-down control should not be a tab stop
		WS_CHILD | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK | UDS_NOTHOUSANDS,
		// this is important; it's necessary for autosizing to work
		0, 0, 0, 0,
		s->hwnd, NULL, hInstance, NULL);
	if (s->updown == NULL)
		logLastError(L"error creating updown");
	SendMessageW(s->updown, UDM_SETBUDDY, (WPARAM) (s->edit), 0);
	uiWindowsRegisterWM_NOTIFYHandler(s->updown, onWM_NOTIFY, uiControl(s));

	// preserve the Z-order
	spinboxArrangeChildren(s);
	// TODO properly show/enable
	ShowWindow(s->updown, SW_SHOW);
	s->inhibitChanged = FALSE;
}

static void spinboxRelayout(uiSpinbox *s)
{
	RECT r;

	// make the edit fill the container first; the new updown will resize it
	uiWindowsEnsureGetClientRect(s->hwnd, &r);
	uiWindowsEnsureMoveWindowDuringResize(s->edit, r.left, r.top, r.right - r.left, r.bottom - r.top);
	recreateUpDown(s);
}

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

int uiSpinboxValue(uiSpinbox *s)
{
	return (int) s->value;
}

double uiSpinboxValueDouble(uiSpinbox *s)
{
	return s->value;
}

void uiSpinboxSetValue(uiSpinbox *s, int value)
{
	uiSpinboxSetValueDouble(s, (double) value);
}

void uiSpinboxSetValueDouble(uiSpinbox *s, double value)
{
	s->inhibitChanged = TRUE;
	update_value(s, value);
	s->inhibitChanged = FALSE;
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

static void onResize(uiWindowsControl *c)
{
	spinboxRelayout(uiSpinbox(c));
}

uiSpinbox *uiNewSpinboxDouble(double min, double max, int precision)
{
	uiSpinbox *s;
	double temp;
	double step;
	int precision_clamped;

	if (min >= max) {
		temp = min;
		min = max;
		max = temp;
	}

	precision_clamped = fmax(0, fmin(20, precision));
	step = 1.0 / pow(10.0, precision_clamped);

	printf("precision %d step size %f min %f max %f\n", precision, step, min, max);

	uiWindowsNewControl(uiSpinbox, s);

	s->hwnd = uiWindowsMakeContainer(uiWindowsControl(s), onResize);

	s->edit = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"edit", L"",
		// don't use ES_NUMBER; it doesn't allow typing in a leading -
		ES_AUTOHSCROLL | ES_LEFT | WS_TABSTOP,
		hInstance, NULL,
		TRUE);
	uiWindowsEnsureSetParentHWND(s->edit, s->hwnd);

	uiWindowsRegisterWM_COMMANDHandler(s->edit, onWM_COMMAND, uiControl(s));
	uiSpinboxOnChanged(s, defaultOnChanged, NULL);

	recreateUpDown(s);

	s->inhibitChanged = TRUE;

	s->step_size = fmax(step, 1 / pow(10, 20));
	s->precision = precision_clamped;
	s->minimum = min;
	s->maximum = max;

	update_value(s, 0);

	s->inhibitChanged = FALSE;

	return s;
}

uiSpinbox *uiNewSpinbox(int min, int max)
{
	return uiNewSpinboxDouble((double)min, (double) max, 0);
}
