#include <stdio.h>

#include "qa.h"

static void checkboxOnToggledCb(uiCheckbox *c, void *data)
{
	char str[32];
	uiLabel *label = data;
	static int count = 0;

	sprintf(str, "%d", ++count);
	uiLabelSetText(label, str);
}

const char *checkboxOnToggledGuide() {
	return
	"1.\tYou should see an unchecked checkbox with the text `Checkbox`.\n"
	"\tNext to it should a  label displaying `Toggle count: 0`.\n"
	"\n"
	"2.\tClick the square next to `Checkbox`. The checkbox should visually\n"
	"\tchange to a checked state and the label should read `Toggle count: 1`.\n"
	"\n"
	"3.\tClick the square next to `Checkbox` again. The checkbox should\n"
	"\tvisually revert back to an unchecked state and the label should read\n"
	"\t `Toggle count: 2`.\n"
	"\n"
	"4.\tClick on the label `Checkbox`. The checkbox should visually\n"
	"\tchange to a checked state and the label should read `Toggle count: 3`.\n"
	"\n"
	"5.\tClick the label `Checkbox` again. The checkbox should visually\n"
	"\trevert back to an unchecked state and the label should read\n"
	"\t `Toggle count: 4`.\n"
	;
}

uiControl* checkboxOnToggled()
{
	uiBox *hbox;
	uiCheckbox *checkbox;
	uiLabel *label;

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);

	checkbox = uiNewCheckbox("Checkbox");
	uiBoxAppend(hbox, uiControl(checkbox), 0);

	label = uiNewLabel("Toggle count:");
	uiBoxAppend(hbox, uiControl(label), 0);

	label = uiNewLabel("0");
	uiBoxAppend(hbox, uiControl(label), 0);

	uiCheckboxOnToggled(checkbox, checkboxOnToggledCb, label);

	return uiControl(hbox);
}
