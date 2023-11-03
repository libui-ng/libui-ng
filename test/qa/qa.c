#include "qa.h"

uiControl* qaMakeGuide(uiControl *c, const char *text)
{
	uiBox *hbox;
	uiBox *vbox;
	uiMultilineEntry *guide;

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);

	vbox = uiNewVerticalBox();
	uiBoxAppend(vbox, c, 0);
	uiBoxAppend(hbox, uiControl(vbox), 1);

	guide = uiNewMultilineEntry();
	uiMultilineEntrySetText(guide, text);
	uiMultilineEntrySetReadOnly(guide, 1);
	uiBoxAppend(hbox, uiControl(guide), 1);

	return uiControl(hbox);
}
