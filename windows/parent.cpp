// 26 april 2015
#include "uipriv_windows.hpp"

// TODO make this public if we want custom containers
// why have this to begin with? http://blogs.msdn.com/b/oldnewthing/archive/2010/03/16/9979112.aspx
BOOL handleParentMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	switch (uMsg) {
	case WM_COMMAND:
		return runWM_COMMAND(wParam, lParam, lResult);
	case WM_NOTIFY:
		return runWM_NOTIFY(wParam, lParam, lResult);
	case WM_HSCROLL:
		return runWM_HSCROLL(wParam, lParam, lResult);
	}
	return FALSE;
}
