#include "uipriv_windows.hpp"

// This doesn't work for containers
static HWND createTooltipForControl(HWND hparent, const wchar_t* text) {
	HWND hwndTT = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hparent, NULL, hInstance, NULL);

	if (!hwndTT) {
		logLastError(L"Failed to create tooltip window.");
		return NULL;
	}

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TTTOOLINFO ti = { 0 };
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.uId = (UINT_PTR) hparent;
	ti.hinst = hInstance;
	ti.lpszText = (LPWSTR) text;

	if (!SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti)) {
		logLastError(L"Failed to set rect to tooltip window.");
		uiWindowsEnsureDestroyWindow(hwndTT);
		return NULL;
	}

	// Enable multiline tooltips
	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, 0);
	return hwndTT;
}

void uiControlSetTooltip(uiControl *c, const char *tooltip) {
	if (tooltip == NULL) {
		if (uiWindowsControl(c)->tooltip == NULL) return;
		uiWindowsEnsureDestroyWindow((HWND) uiWindowsControl(c)->tooltip);
		return;
	}
	wchar_t *wtext = toUTF16(tooltip);
	void *ptr = createTooltipForControl((HWND) uiControlHandle(c), wtext);
	uiprivFree(wtext);

	uiWindowsControl(c)->tooltip = ptr;
}
