#include "uipriv_windows.hpp"

static void *CreateTooltip(HWND hparent, const wchar_t* text) {
    HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hparent, NULL, hInstance, NULL);

    if (!hwndTT) {
        printf("ERROR: Failed to create tooltip window.");
        return NULL;
    }

    SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    TTTOOLINFO ti = { 0 };
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hparent;
    ti.hinst = hInstance;
    ti.lpszText = (LPWSTR)text;
    GetClientRect(hparent, &ti.rect);
    ti.rect.right = 400;

    if (!SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti)) {
        logLastError(L"Failed to set rect to tooltip window.");
        uiWindowsEnsureDestroyWindow(hwndTT);
        return NULL;
    }
    return hwndTT;
}

void uiControlSetTooltip(uiControl *c, const char *tooltip) {
	if (tooltip == NULL) {
		if (uiWindowsControl(c)->tooltip == NULL) return;
		uiWindowsEnsureDestroyWindow((HWND)uiWindowsControl(c)->tooltip);
		return;
	}
    wchar_t *wtext = toUTF16(tooltip);
    void *ptr = CreateTooltip((HWND)uiControlHandle(c), wtext);
    uiprivFree(wtext);

	uiWindowsControl(c)->tooltip = ptr;
}
