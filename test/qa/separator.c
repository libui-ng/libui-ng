#include "qa.h"

const char *separatorVerticalGuide() {
	return
	"1.\tYou should see two labels `1` and `2` placed next to each other\n"
	"\thorizontally. Between the two labels should be a vertical line that\n"
	"\tvisually separates the two labels.\n"
	;
}

uiControl* separatorVertical()
{
	uiBox *hbox;

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);

	uiBoxAppend(hbox, uiControl(uiNewLabel("1")), 0);
	uiBoxAppend(hbox, uiControl(uiNewVerticalSeparator()), 0);
	uiBoxAppend(hbox, uiControl(uiNewLabel("2")), 0);

	return uiControl(hbox);
}

const char *separatorHorizontalGuide() {
	return
	"1.\tYou should see two labels `1` and `2` placed on top of each other.\n"
	"\tBetween the two labels should be a horizontal line that visually\n"
	"\tseparates the two labels.\n"
	;
}

uiControl* separatorHorizontal()
{
	uiBox *vbox;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	uiBoxAppend(vbox, uiControl(uiNewLabel("1")), 0);
	uiBoxAppend(vbox, uiControl(uiNewHorizontalSeparator()), 0);
	uiBoxAppend(vbox, uiControl(uiNewLabel("2")), 0);

	return uiControl(vbox);
}

