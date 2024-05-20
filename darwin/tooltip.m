#include "uipriv_darwin.h"

// https://developer.apple.com/documentation/appkit/nsview/1483541-tooltip?language=objc

void uiControlSetTooltip(uiControl *c, const char *tooltip) {
	NSView *view = (NSView *)uiControlHandle(c);
	if (tooltip == NULL) {
		view.toolTip = nil;
	} else {
		view.toolTip = uiprivToNSString(tooltip);
	}
}
