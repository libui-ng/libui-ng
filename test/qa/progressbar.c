#include "qa.h"

const char *progressBarValuesGuide() {
	return
	"1.\tYou should see 4 progress bars.\n"
	"\n"
	"2.\tThe first bar should be empty (or nearly empty on darwin) and\n"
	"\tindicate a progress of `0%`\n"
	"\n"
	"3.\tThe second bar should be half way full and indicate a progress of\n"
	"\t`50%`.\n"
	"\n"
	"4.\tThe third bar should be full and indicate a progress of `100%`.\n"
	"\n"
	"5.\tThe fourth bar should visually indicate an indeterminate progress\n"
	"\tlevel by showing a moving progress bar animation.\n"
	;
}

uiControl* progressBarValues()
{
	uiBox *vbox;
	uiProgressBar *p;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	p = uiNewProgressBar();
	uiBoxAppend(vbox, uiControl(p), 0);

	p = uiNewProgressBar();
	uiProgressBarSetValue(p, 50);
	uiBoxAppend(vbox, uiControl(p), 0);

	p = uiNewProgressBar();
	uiProgressBarSetValue(p, 100);
	uiBoxAppend(vbox, uiControl(p), 0);

	p = uiNewProgressBar();
	uiProgressBarSetValue(p, -1);
	uiBoxAppend(vbox, uiControl(p), 0);

	return uiControl(vbox);
}

