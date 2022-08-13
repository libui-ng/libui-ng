#include "test.h"

static uiWindow *parent;
static int clickedTimes;

static void onButtonClicked(uiButton *b, void *data)
{
	char str[256];

	clickedTimes++;
	sprintf(str, "Clicked %d time%s", clickedTimes, clickedTimes > 1? "s" : "");
	uiButtonSetText(b, str);
}

static void doButtonClick(uiButton *b, void *data)
{
	if (!uiA11yDoButtonClick(uiButton(data)))
		uiMsgBoxError(parent, "Error: a11y", "Failed to automate button click.");
}

uiBox *makePage18(uiWindow *pw)
{
	uiBox *page18;
	uiBox *controls;
	uiButton *doButton, *button;

	parent = pw;
	page18 = newVerticalBox();
	controls = newHorizontalBox();
	uiBoxAppend(page18, uiControl(controls), 0);

	button = uiNewButton("Click Me!");
	doButton = uiNewButton("Do Click");
	uiButtonOnClicked(doButton, doButtonClick, button);
	uiBoxAppend(controls, uiControl(doButton), 0);
	clickedTimes = 0;
	uiButtonOnClicked(button, onButtonClicked, NULL);
	uiBoxAppend(controls, uiControl(button), 0);

	return page18;
}

