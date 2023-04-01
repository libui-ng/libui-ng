// 26 april 2015
#include "uipriv_windows.hpp"

// Code for the HWND of the following uiControls:
// - uiBox
// - uiRadioButtons
// - uiSpinbox
// - uiTab
// - uiForm
// - uiGrid

struct containerInit {
	uiWindowsControl *c;
	void (*onResize)(uiWindowsControl *);
};

static HWND parentWithBackground(HWND hwnd)
{
	HWND parent;
	int cls;

	parent = hwnd;
	for (;;) {
		parent = parentOf(parent);
		// skip groupboxes; they're (supposed to be) transparent
		// skip uiContainers; they don't draw anything
		cls = windowClassOf(parent, L"button", containerClass, NULL);
		if (cls != 0 && cls != 1)
			break;
	}
	return parent;
}

static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	HDC dc;
	PAINTSTRUCT ps;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	MINMAXINFO *mmi = (MINMAXINFO *) lParam;
	struct containerInit *init;
	uiWindowsControl *c;
	void (*onResize)(uiWindowsControl *);
	int minwid, minht;
	LRESULT lResult;
	HWND hwndParent;
	HBRUSH bgBrush;

	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_CREATE:
		init = (struct containerInit *) (cs->lpCreateParams);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (init->onResize));
		SetWindowLongPtrW(hwnd, 0, (LONG_PTR) (init->c));
		break;		// defer to DefWindowProc()
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;	// defer to DefWindowProc();
		onResize = (void (*)(uiWindowsControl *)) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		c = (uiWindowsControl *) GetWindowLongPtrW(hwnd, 0);
		(*(onResize))(c);
		return 0;
	case WM_GETMINMAXINFO:
		lResult = DefWindowProcW(hwnd, uMsg, wParam, lParam);
		c = (uiWindowsControl *) GetWindowLongPtrW(hwnd, 0);
		uiWindowsControlMinimumSize(c, &minwid, &minht);
		mmi->ptMinTrackSize.x = minwid;
		mmi->ptMinTrackSize.y = minht;
		return lResult;

	// GDI doesn't support transparency. The Win32 controls way to achieve a similar effect
	// is to ask the parent control for the appropriate background brush (WM_CTLCOLORSTATIC)
	// or ask to the parent to render the background for it (WM_PRINTCLIENT).
	//
	// Our container control is supposed to be fully transparent. And containers can contain
	// other containers. So we have to dynamically find the next control that is not a
	// container itself and retrieve its background brush so we can paint ourselves with it.
	//
	// NOTE: Doing "transparency" this way, only works for solid color backgrounds. Supporting
	// gradients and more is super tricky. Fortunately Microsoft is using solid color backgrounds
	// since Vista. Probably for this reason.
	//
	// Paint ourself with the background brush.
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		if(dc == NULL)
			break;
		hwndParent = parentWithBackground(hwnd);
		bgBrush = (HBRUSH) SendMessage(hwndParent, WM_CTLCOLORSTATIC, (WPARAM) dc, (LPARAM) hwnd);
		FillRect(dc, &ps.rcPaint, bgBrush);
		EndPaint (hwnd, &ps);
		return 0;
	// Paint the backgrounds of our children if they ask for it.
	case WM_PRINTCLIENT:
		dc = (HDC)wParam;
		GetClientRect(hwnd, &r);
		hwndParent = parentWithBackground(hwnd);
		bgBrush = (HBRUSH) SendMessage(hwndParent, WM_CTLCOLORSTATIC, (WPARAM) dc, (LPARAM) hwnd);
		FillRect(dc, &r, bgBrush);
		return 0;
	// Relay the background brush inquiries from our children to our own parent.
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSTATIC:
		dc = (HDC)wParam;
		hwndParent = parentWithBackground(hwnd);
		SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));
		bgBrush = (HBRUSH) SendMessage(hwndParent, WM_CTLCOLORSTATIC, wParam, lParam);
		return (INT_PTR)bgBrush;
	// Skip erasing because we paint the whole area anyways.
	case WM_ERASEBKGND:
		return 1;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

ATOM initContainer(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = containerClass;
	wc.lpfnWndProc = containerWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.cbWndExtra = sizeof (void *);
	return RegisterClassW(&wc);
}

void uninitContainer(void)
{
	if (UnregisterClassW(containerClass, hInstance) == 0)
		logLastError(L"error unregistering container window class");
}

HWND uiWindowsMakeContainer(uiWindowsControl *c, void (*onResize)(uiWindowsControl *))
{
	struct containerInit init;

	// TODO onResize cannot be NULL
	init.c = c;
	init.onResize = onResize;
	return uiWindowsEnsureCreateControlHWND(WS_EX_CONTROLPARENT,
		containerClass, L"",
		0,
		hInstance, (LPVOID) (&init),
		FALSE);
}
