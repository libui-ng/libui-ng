#include "uipriv_windows.hpp"

int uiA11yDoButtonClick(uiButton *b)
{
	IAccessible *a11y;
	VARIANT varSelf;
	HRESULT hr;

	hr = AccessibleObjectFromWindow((HWND) uiControlHandle(uiControl(b)),
		OBJID_WINDOW, __uuidof (IAccessible), (void **) (&a11y));
	if (hr != S_OK) {
		logHRESULT(L"error calling AccessibleObjectFromWindow() in uiA11yDoButtonClick()", hr);
		return FALSE;
	}

	varSelf.vt = VT_I4;
	varSelf.lVal = CHILDID_SELF;
	hr = a11y->accDoDefaultAction(varSelf);
	a11y->Release();
	if (hr != S_OK) {
		logHRESULT(L"error calling IAccessible::accDoDefaultAction() in uiA11yDoButtonClick()", hr);
		return FALSE;
	}
	return TRUE;
}

