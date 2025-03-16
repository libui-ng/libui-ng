// 9 april 2015
#include "uipriv_windows.hpp"

WCHAR *windowTextAndLen(const HWND hwnd, LRESULT *len)
{
	LRESULT n;
	WCHAR *text;

	n = SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
	if (len != NULL)
		*len = n;
	// WM_GETTEXTLENGTH does not include the null terminator
	text = (WCHAR *) uiprivAlloc((n + 1) * sizeof (WCHAR), "WCHAR[]");
	// note the comparison: the size includes the null terminator, but the return does not
	if (GetWindowTextW(hwnd, text, n + 1) != n) {
		logLastError(L"error getting window text");
		// on error, return an empty string to be safe
		*text = L'\0';
		if (len != NULL)
			*len = 0;
	}
	return text;
}

WCHAR *windowText(const HWND hwnd)
{
	return windowTextAndLen(hwnd, NULL);
}

void setWindowText(HWND hwnd, WCHAR *wtext)
{
	if (SetWindowTextW(hwnd, wtext) == 0)
		logLastError(L"error setting window text");
}

void uiFreeText(char *text)
{
	uiprivFree(text);
}

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define labelHeight 8

int uiWindowsWindowTextHeight(const HWND hwnd)
{
	LRESULT len;
	WCHAR* text, *start;
	int lineCount = 1;

	text = windowTextAndLen(hwnd, &len);
	for (start = text; start != text + len; start++)
		if (*start == L'\n')
			lineCount++;

	uiprivFree(text);
	return lineCount * labelHeight;
}

int uiWindowsWindowTextWidth(const HWND hwnd)
{
	LRESULT len;
	WCHAR *text, *start, *end;
	HDC dc;
	HFONT prevfont;
	SIZE size;

	// save the max width of multiline text
	int maxWidth = 0;

	text = windowTextAndLen(hwnd, &len);
	if (len == 0)		// no text; nothing to do
		goto noTextOrError;

	// now we can do the calculations
	dc = GetDC(hwnd);
	if (dc == NULL) {
		logLastError(L"error getting DC");
		// on any error, assume no text
		goto noTextOrError;
	}
	prevfont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevfont == NULL) {
		logLastError(L"error loading control font into device context");
		ReleaseDC(hwnd, dc);
		goto noTextOrError;
	}

	// calculate width of each line
	start = end = text;
	while (start != text + len) {
		while (*start == L'\n' && start != text + len)
			start++;
		if (start == text + len)
			break;
		end = start + 1;
		while (*end != L'\n' && end != text + len)
			end++;
		if (GetTextExtentPoint32W(dc, start, end - start, &size) == 0)
			logLastError(L"error getting text extent point");
		else if (size.cx > maxWidth)
			maxWidth = size.cx;
		start = end;
	}

	// continue on errors; we got what we want
	if (SelectObject(dc, prevfont) != hMessageFont)
		logLastError(L"error restoring previous font into device context");
	if (ReleaseDC(hwnd, dc) == 0)
		logLastError(L"error releasing DC");

	uiprivFree(text);
	return maxWidth;

noTextOrError:
	uiprivFree(text);
	return 0;
}

char *uiWindowsWindowText(const HWND hwnd)
{
	WCHAR *wtext;
	char *text;

	wtext = windowText(hwnd);
	text = toUTF8(wtext);
	uiprivFree(wtext);
	return text;
}

void uiWindowsSetWindowText(HWND hwnd, const char *text)
{
	WCHAR *wtext;

	wtext = toUTF16(text);
	setWindowText(hwnd, wtext);
	uiprivFree(wtext);
}

int uiprivStricmp(const char *a, const char *b)
{
	WCHAR *wa, *wb;
	int ret;

	wa = toUTF16(a);
	wb = toUTF16(b);
	ret = _wcsicmp(wa, wb);
	uiprivFree(wb);
	uiprivFree(wa);
	return ret;
}
