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

static void indeterminateOnToggled(uiCheckbox *c, void *data)
{
	uiProgressBar *p = data;

	if (uiCheckboxChecked(c))
		uiProgressBarSetValue(p, -1);
	else
		uiProgressBarSetValue(p, 50);
}

const char *progressBarIndeterminateGuide() {
	return
	"1.\tYou should see a progress bar that visually indicates an\n"
	"\tindeterminate progress level by showing a moving progress bar\n"
	"\tanimation and a checked checkbox `Indeterminate`.\n"
	"\n"
	"2.\tUncheck the checkbox. The progress bar animation should stop and\n"
	"\tdisplay a half full, solid bar.\n"
	"\n"
	"3.\tCheck the checkbox again. The progress bar animation should start\n"
	"\tagain and visually suggest an indeterminate progress level by\n"
	"\tshowing a moving progress bar.\n"
	;
}

uiControl* progressBarIndeterminate()
{
	uiBox *vbox;
	uiProgressBar *p;
	uiCheckbox *c;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	p = uiNewProgressBar();
	uiBoxAppend(vbox, uiControl(p), 0);

	c = uiNewCheckbox("Indeterminate");
	uiBoxAppend(vbox, uiControl(c), 0);
	uiCheckboxOnToggled(c, indeterminateOnToggled, p);
	uiCheckboxSetChecked(c, 1);
	indeterminateOnToggled(c, p);

	return uiControl(vbox);
}

