#include "uipriv_windows.hpp"

// This doesn't work for containers
static HWND createTooltipForControl(HWND hparent, const wchar_t* text)
{
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

static void addTooltipToControl(HWND hparent,
	const wchar_t *wtext, std::vector<HWND> *tooltips)
{
	HWND tt = createTooltipForControl(hparent, wtext);
	if (tt)
		tooltips->push_back(tt);
}

static HWND addTooltipToChild(HWND hparent, HWND child_after,
	const wchar_t *class_name, const wchar_t *wtext, std::vector<HWND> *tooltips)
{
	HWND child;
	HWND tt;
	child = FindWindowExW(hparent, child_after, class_name, NULL);
	if (!child) return NULL;

	addTooltipToControl(child, wtext, tooltips);
	return child;
}

void uiControlSetTooltip(uiControl *c, const char *tooltip)
{
	uiprivDestroyTooltip(c);
	
	if (tooltip == NULL) return;

	wchar_t *wtext = toUTF16(tooltip);
	HWND hparent = (HWND) uiControlHandle(c);
	HWND child = NULL;
	std::vector<HWND> *tooltips = new std::vector<HWND>(0);

	switch (c->TypeSignature) {
	case uiEditableComboboxSignature:
	case uiSpinboxSignature:
		child = addTooltipToChild(hparent, NULL, L"edit", wtext, tooltips);
		break;
	case uiRadioButtonsSignature:
		// This works only for existing buttons.
		// You should run uiRadioButtonsAppend before calling uiControlSetTooltip.
		do {
			child = addTooltipToChild(hparent, child, L"button", wtext, tooltips);
		} while (child);
		break;
	default:
		addTooltipToControl(hparent, wtext, tooltips);
	}

	uiprivFree(wtext);
	uiWindowsControl(c)->tooltips = tooltips;
}

void uiprivDestroyTooltip(uiControl* c)
{
	std::vector<HWND> *tooltips = (std::vector<HWND>*) uiWindowsControl(c)->tooltips;

	if (tooltips == NULL) return;

	for (HWND tt : *tooltips) {
		uiWindowsEnsureDestroyWindow(tt);
	}
	delete tooltips;
	uiWindowsControl(c)->tooltips = NULL;
}

