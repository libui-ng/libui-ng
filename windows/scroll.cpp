typedef struct uiScroll uiScroll;
#define uiScroll(this) ((uiScroll *) (this))

extern "C" {
	_UI_EXTERN uiScroll *uiNewScroll();
	_UI_EXTERN void uiScrollSetChild(uiScroll *scroll, uiControl *ctl);
}

#define scrollClass L"libui_uiScrollClass"
#define uiScrollSignature 0x083983

#include "uipriv_windows.hpp"

struct containerInit {
	uiWindowsControl *c;
	void (*onResize)(uiWindowsControl *);
};

struct boxChild {
	uiControl *c;
	int stretchy;
	int width;
	int height;
};

struct uiScroll {
	uiWindowsControl c;
	HWND hwnd;
	struct boxChild child;
	int vertical;
	int padded;

	int scrollWidth;
	int scrollHeight;
	int hscrollpos;
	int vscrollpos;
	int hwheelCarry;
	int vwheelCarry;
};

struct scrollParams {
	int *pos;
	int pagesize;
	int length;
	int *wheelCarry;
	UINT wheelSPIAction;
};

static void hwheelscroll(uiScroll *a, WPARAM wParam, LPARAM lParam);
static void vscroll(uiScroll *a, WPARAM wParam, LPARAM lParam);
static void hscroll(uiScroll *a, WPARAM wParam, LPARAM lParam);
static void vwheelscroll(uiScroll *a, WPARAM wParam, LPARAM lParam);
static BOOL areaDoScroll(uiScroll *a, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult);

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

static LRESULT CALLBACK scrollWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		puts("Print client");
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

	uiScroll *s = (uiScroll *)GetWindowLongPtrW(hwnd, 0);

	if (s != NULL) {
		switch (uMsg) {
		case WM_HSCROLL:
			hscroll(s, wParam, lParam);
			return 0;
		case WM_MOUSEHWHEEL:
			hwheelscroll(s, wParam, lParam);
			return 0;
		case WM_VSCROLL:
			vscroll(s, wParam, lParam);
			return 0;
		case WM_MOUSEWHEEL:
			vwheelscroll(s, wParam, lParam);
			return 0;
		}
	}
	
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}

ATOM registerScrollClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = scrollClass;
	wc.lpfnWndProc = scrollWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.cbWndExtra = sizeof (void *);
	return RegisterClassW(&wc);
}

static void scrollto(uiScroll *a, int which, struct scrollParams *p, int pos)
{
	SCROLLINFO si;

	// note that the pos < 0 check is /after/ the p->length - p->pagesize check
	// it used to be /before/; this was actually a bug in Raymond Chen's original algorithm: if there are fewer than a page's worth of items, p->length - p->pagesize will be negative and our content draw at the bottom of the window
	// this SHOULD have the same effect with that bug fixed and no others introduced... (thanks to devin on irc.badnik.net for confirming this logic)
	if (pos > p->length - p->pagesize)
		pos = p->length - p->pagesize;
	if (pos < 0)
		pos = 0;

	if (a->child.c == NULL) return;
	HWND child = (HWND)uiControlHandle(a->child.c);

	int y = *(p->pos) - pos;
	int x = 
	*(p->pos) = pos;

	// now commit our new scrollbar setup...
	ZeroMemory(&si, sizeof (SCROLLINFO));
	si.cbSize = sizeof (SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage = p->pagesize;
	si.nMin = 0;
	si.nMax = p->length - 1;		// endpoint inclusive
	si.nPos = *(p->pos);
	SetScrollInfo(a->hwnd, which, &si, TRUE);

	ScrollWindow(child, 0, y, NULL, NULL);
}

static void scrollby(uiScroll *a, int which, struct scrollParams *p, int delta)
{
	scrollto(a, which, p, *(p->pos) + delta);
}

static void scroll(uiScroll *a, int which, struct scrollParams *p, WPARAM wParam, LPARAM lParam)
{
	int pos;
	SCROLLINFO si;

	pos = *(p->pos);
	switch (LOWORD(wParam)) {
	case SB_LEFT:			// also SB_TOP
		pos = 0;
		break;
	case SB_RIGHT:		// also SB_BOTTOM
		pos = p->length - p->pagesize;
		break;
	case SB_LINELEFT:		// also SB_LINEUP
		pos--;
		break;
	case SB_LINERIGHT:		// also SB_LINEDOWN
		pos++;
		break;
	case SB_PAGELEFT:		// also SB_PAGEUP
		pos -= p->pagesize;
		break;
	case SB_PAGERIGHT:	// also SB_PAGEDOWN
		pos += p->pagesize;
		break;
	case SB_THUMBPOSITION:
		ZeroMemory(&si, sizeof (SCROLLINFO));
		si.cbSize = sizeof (SCROLLINFO);
		si.fMask = SIF_POS;
		if (GetScrollInfo(a->hwnd, which, &si) == 0)
			logLastError(L"error getting thumb position for area");
		pos = si.nPos;
		break;
	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof (SCROLLINFO));
		si.cbSize = sizeof (SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		if (GetScrollInfo(a->hwnd, which, &si) == 0)
			logLastError(L"error getting thumb track position for area");
		pos = si.nTrackPos;
		break;
	}
	scrollto(a, which, p, pos);
}

static void wheelscroll(uiScroll *a, int which, struct scrollParams *p, WPARAM wParam, LPARAM lParam)
{
	int delta;
	int lines;
	UINT scrollAmount;

	delta = GET_WHEEL_DELTA_WPARAM(wParam);
	if (SystemParametersInfoW(p->wheelSPIAction, 0, &scrollAmount, 0) == 0)
		// TODO use scrollAmount == 3 (for both v and h) instead?
		logLastError(L"error getting area wheel scroll amount");
	if (scrollAmount == WHEEL_PAGESCROLL)
		scrollAmount = p->pagesize;
	if (scrollAmount == 0)		// no mouse wheel scrolling (or t->pagesize == 0)
		return;
	// the rest of this is basically http://blogs.msdn.com/b/oldnewthing/archive/2003/08/07/54615.aspx and http://blogs.msdn.com/b/oldnewthing/archive/2003/08/11/54624.aspx
	// see those pages for information on subtleties
	delta += *(p->wheelCarry);
	lines = delta * ((int) scrollAmount) / WHEEL_DELTA;
	*(p->wheelCarry) = delta - lines * WHEEL_DELTA / ((int) scrollAmount);
	scrollby(a, which, p, -lines);
}

static void hscrollParams(uiScroll *a, struct scrollParams *p)
{
	RECT r;

	ZeroMemory(p, sizeof (struct scrollParams));
	p->pos = &(a->hscrollpos);
	// TODO get rid of these and replace with points
	uiWindowsEnsureGetClientRect(a->hwnd, &r);
	p->pagesize = r.right - r.left;
	p->length = a->scrollWidth;
	p->wheelCarry = &(a->hwheelCarry);
	p->wheelSPIAction = SPI_GETWHEELSCROLLCHARS;
}

static void hscrollby(uiScroll *a, int delta)
{
	struct scrollParams p;

	hscrollParams(a, &p);
	scrollby(a, SB_HORZ, &p, delta);
}

static void hscroll(uiScroll *a, WPARAM wParam, LPARAM lParam)
{
	struct scrollParams p;

	hscrollParams(a, &p);
	scroll(a, SB_HORZ, &p, wParam, lParam);
}

static void hwheelscroll(uiScroll *a, WPARAM wParam, LPARAM lParam)
{
	struct scrollParams p;

	hscrollParams(a, &p);
	wheelscroll(a, SB_HORZ, &p, wParam, lParam);
}

static void vscrollParams(uiScroll *a, struct scrollParams *p)
{
	RECT r;

	ZeroMemory(p, sizeof (struct scrollParams));
	p->pos = &(a->vscrollpos);
	uiWindowsEnsureGetClientRect(a->hwnd, &r);
	p->pagesize = r.bottom - r.top;
	p->length = a->scrollHeight;
	if (p->length == 0) p->length = 1;
	p->wheelCarry = &(a->vwheelCarry);
	p->wheelSPIAction = SPI_GETWHEELSCROLLLINES;
}

static void vscrollby(uiScroll *a, int delta)
{
	struct scrollParams p;

	vscrollParams(a, &p);
	scrollby(a, SB_VERT, &p, delta);
}

static void vscroll(uiScroll *a, WPARAM wParam, LPARAM lParam)
{
	struct scrollParams p;

	vscrollParams(a, &p);
	scroll(a, SB_VERT, &p, wParam, lParam);
}

static void vwheelscroll(uiScroll *a, WPARAM wParam, LPARAM lParam)
{
	struct scrollParams p;

	vscrollParams(a, &p);
	wheelscroll(a, SB_VERT, &p, wParam, lParam);
}

void areaUpdateScroll(uiScroll *a)
{
	// use a no-op scroll to simulate scrolling
	//hscrollby(a, 0);
	vscrollby(a, 0);
}

static void boxPadding(uiScroll *b, int *xpadding, int *ypadding)
{
	uiWindowsSizing sizing;

	*xpadding = 0;
	*ypadding = 0;
	if (b->padded) {
		uiWindowsGetSizing(b->hwnd, &sizing);
		uiWindowsSizingStandardPadding(&sizing, xpadding, ypadding);
	}
}

static void boxRelayout(uiScroll *b)
{
	RECT r;
	int x, y, width, height;
	int xpadding, ypadding;
	int nStretchy;
	int stretchywid, stretchyht;
	int minimumWidth, minimumHeight;
	int nVisible;

	uiWindowsEnsureGetClientRect(b->hwnd, &r);
	x = r.left;
	y = r.top;
	width = r.right - r.left;
	height = r.bottom - r.top;

	// -1) get this Box's padding
	boxPadding(b, &xpadding, &ypadding);

	// 1) get width and height of non-stretchy controls
	// this will tell us how much space will be left for stretchy controls
	stretchywid = width;
	stretchyht = height;
	nStretchy = 0;
	nVisible = 0;

	if (b->child.c == NULL) return;

	uiWindowsControlMinimumSize(uiWindowsControl(b->child.c), &minimumWidth, &minimumHeight);
	if (b->vertical) {		// all controls have same width
		b->child.width = width;
		b->child.height = minimumHeight;
		stretchyht -= minimumHeight;
	} else {				// all controls have same height
		b->child.width = minimumWidth;
		b->child.height = height;
		stretchywid -= minimumWidth;
	}

	b->scrollHeight = minimumHeight;
	b->scrollWidth = minimumWidth;

	// 2) now inset the available rect by the needed padding
	if (b->vertical) {
		height -= (nVisible - 1) * ypadding;
		stretchyht -= (nVisible - 1) * ypadding;
	} else {
		width -= (nVisible - 1) * xpadding;
		stretchywid -= (nVisible - 1) * xpadding;
	}

	// 3) now get the size of stretchy controls
	if (nStretchy != 0) {
		if (b->vertical) {
			stretchyht /= nStretchy;
		} else {
			stretchywid /= nStretchy;
			if (b->child.stretchy) {
				b->child.width = stretchywid;
				b->child.height = stretchyht;
			}
		}
	}

	uiWindowsEnsureMoveWindowDuringResize((HWND) uiControlHandle(b->child.c), 0, 0, b->child.width, b->child.height);
	areaUpdateScroll(b);
}

static void uiScrollDestroy(uiControl *c)
{
	uiScroll *b = uiScroll(c);

	uiControlSetParent(b->child.c, NULL);
	uiControlDestroy(b->child.c);

	uiWindowsEnsureDestroyWindow(b->hwnd);
	uiFreeControl(uiControl(b));
}

uiWindowsControlDefaultHandle(uiScroll)
uiWindowsControlDefaultParent(uiScroll)
uiWindowsControlDefaultSetParent(uiScroll)
uiWindowsControlDefaultToplevel(uiScroll)
uiWindowsControlDefaultVisible(uiScroll)
uiWindowsControlDefaultShow(uiScroll)
uiWindowsControlDefaultHide(uiScroll)
uiWindowsControlDefaultEnabled(uiScroll)
uiWindowsControlDefaultEnable(uiScroll)
uiWindowsControlDefaultDisable(uiScroll)

static void uiScrollSyncEnableState(uiWindowsControl *c, int enabled)
{
	uiScroll *b = uiScroll(c);

	if (uiWindowsShouldStopSyncEnableState(uiWindowsControl(b), enabled))
		return;
	uiWindowsControlSyncEnableState(uiWindowsControl(b->child.c), enabled);
}

uiWindowsControlDefaultSetParentHWND(uiScroll)

static void uiScrollMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiScroll *b = uiScroll(c);
	int xpadding, ypadding;
	int nStretchy;
	// these two contain the largest minimum width and height of all stretchy controls in the box
	// all stretchy controls will use this value to determine the final minimum size
	int maxStretchyWidth, maxStretchyHeight;
	int minimumWidth, minimumHeight;
	int nVisible;

	*width = 0;
	*height = 0;

	// 0) get this Box's padding
	boxPadding(b, &xpadding, &ypadding);

	if (nVisible == 0)		// just return 0x0
		return;

	// 2) now outset the desired rect with the needed padding
	if (b->vertical)
		*height += (nVisible - 1) * ypadding;
	else
		*width += (nVisible - 1) * xpadding;

	// 3) and now we can add in stretchy controls
	if (b->vertical)
		*height += nStretchy * maxStretchyHeight;
	else
		*width += nStretchy * maxStretchyWidth;
}

static void uiScrollMinimumSizeChanged(uiWindowsControl *c)
{
	uiScroll *b = uiScroll(c);

	if (uiWindowsControlTooSmall(uiWindowsControl(b))) {
		uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl(b));
		return;
	}
	boxRelayout(b);
}

uiWindowsControlDefaultLayoutRect(uiScroll)
uiWindowsControlDefaultAssignControlIDZOrder(uiScroll)

static void uiScrollChildVisibilityChanged(uiWindowsControl *c)
{
	// TODO eliminate the redundancy
	uiWindowsControlMinimumSizeChanged(c);
}

void uiScrollDelete(uiScroll *b, int index)
{
	uiControl *c;

	uiControlSetParent(c, NULL);
	uiWindowsControlSetParentHWND(uiWindowsControl(c), NULL);
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(b));
}

int uiScrollPadded(uiScroll *b)
{
	return b->padded;
}

void uiScrollSetPadded(uiScroll *b, int padded)
{
	b->padded = padded;
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(b));
}

static void onResize(uiWindowsControl *c)
{
	boxRelayout(uiScroll(c));
}

void uiScrollSetChild(uiScroll *scroll, uiControl *ctl) {
	scroll->child.c = ctl;

	uiControlSetParent(ctl, uiControl(scroll));
	uiWindowsControlSetParentHWND(uiWindowsControl(ctl), scroll->hwnd);

	boxRelayout(scroll);
}

uiScroll *uiNewScroll() {
	uiScroll *s;

	uiWindowsNewControl(uiScroll, s);

	struct containerInit init;

	// TODO onResize cannot be NULL
	init.c = uiWindowsControl(s);
	init.onResize = onResize;

	s->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CONTROLPARENT,
		scrollClass, L"",
		WS_HSCROLL | WS_VSCROLL | ES_MULTILINE,
		hInstance, &init,
		FALSE);

	s->scrollWidth = 0;
	s->scrollHeight = 0;
	s->hscrollpos = 0;
	s->vscrollpos = 0;
	s->hwheelCarry = 1000;
	s->vwheelCarry = 1000;

	boxRelayout(s);

	return s;
}
