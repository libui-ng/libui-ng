#include "qa.h"

const char *labelMultiLineGuide() {
	return
	"1.\tThe first line should read `Single line test`.\n"
	"\n"
	"2.\tNext are four invisible horizontal boxes.\n"
	"\tThe first three should each have the width of the line `Long line`.\n"
	"\tThe fourth box should take up the remaining space and consist of three\n"
	"\tlines reading `Padding`.\n"
	"\n"
	"3.\tThe next line should read `Multi line height test` and should have a\n"
	"\tsimilar distance to the four boxes as the first line `Single line test`."
	;
}

uiControl* labelMultiLine()
{
	uiBox *vbox;
	uiBox *hbox;
	uiLabel *label;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	label = uiNewLabel("Single line test");
	uiBoxAppend(vbox, uiControl(label), 0);

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiBoxAppend(vbox, uiControl(hbox), 0);

	label = uiNewLabel("Long line\nShort\nShort");
	uiBoxAppend(hbox, uiControl(label), 0);

	label = uiNewLabel("Short\nLong line\nShort");
	uiBoxAppend(hbox, uiControl(label), 0);

	label = uiNewLabel("Short\nShort\nLong line");
	uiBoxAppend(hbox, uiControl(label), 0);

	label = uiNewLabel("Padding\nPadding\nPadding");
	uiBoxAppend(hbox, uiControl(label), 1);

	label = uiNewLabel("Multi line height test");
	uiBoxAppend(vbox, uiControl(label), 0);

	return uiControl(vbox);
}
