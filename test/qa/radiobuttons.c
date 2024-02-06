#include <stdio.h>

#include "qa.h"

static void radioButtonsOnSelectedCb(uiRadioButtons *r, void *data)
{
	char str[32];
	static int count = 0;
	uiLabel *label = data;

	sprintf(str, "%d", ++count);
	uiLabelSetText(label, str);
}

const char *radioButtonsOnSelectedGuide() {
	return
	"1.\tYou should see two radio buttons `Item 1` and `Item 2` as well as\n"
	"\ta label displaying `Selected count: 0`. None of the radio buttons\n"
	"\tshould be selected.\n"
	"\n"
	"2.\tClick the radio button `Item 1`. This should select the radio button\n"
	"\t`Item 1` and the label should read `Selected count: 1`.\n"
	"\n"
	"2.\tClick the radio button `Item 1` again. Nothing should happen.\n"
	"\n"
	"2.\tClick the radio button `Item 2`. This should select the radio button\n"
	"\t`Item 2`, deselect the radio button `Item 1`  and the label should now\n"
	"\tread `Selected count: 2`.\n"
	;
}

uiControl* radioButtonsOnSelected()
{
	uiBox *hbox;
	uiRadioButtons *r;
	uiLabel *label;

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);

	r = uiNewRadioButtons();
	uiBoxAppend(hbox, uiControl(r), 0);
	uiRadioButtonsAppend(r, "Item 1");
	uiRadioButtonsAppend(r, "Item 2");

	label = uiNewLabel("Selected count:");
	uiBoxAppend(hbox, uiControl(label), 0);

	label = uiNewLabel("0");
	uiBoxAppend(hbox, uiControl(label), 0);

	uiRadioButtonsOnSelected(r, radioButtonsOnSelectedCb, label);

	return uiControl(hbox);
}
