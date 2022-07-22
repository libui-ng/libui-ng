#include "uipriv_darwin.h"

int uiA11yDoButtonClick(uiButton *b)
{
	NSControl *c;
	id <NSAccessibility> a11y;

	c = (NSControl *)uiControlHandle(uiControl(b));
	a11y = c;
	// FIXME Always returns NO, but does work
	[a11y accessibilityPerformPress];
	return TRUE;
}

