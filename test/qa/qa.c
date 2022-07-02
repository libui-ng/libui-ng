#include "qa.h"

uiControl* qaMakeGuide(uiControl *c, const char *text)
{
	uiBox *hbox;
	uiMultilineEntry *guide;

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiBoxAppend(hbox, c, 1);

	guide = uiNewMultilineEntry();
	uiMultilineEntrySetText(guide, text);
	uiMultilineEntrySetReadOnly(guide, 1);
	uiBoxAppend(hbox, uiControl(guide), 1);

	return uiControl(hbox);
}
