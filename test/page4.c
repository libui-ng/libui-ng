// 19 may 2015
#include "test.h"

static uiSpinbox *spinbox;
static uiSlider *slider;
static uiProgressBar *pbar;
static uiProgressBar *pbar2;
static uiCheckbox *checkbox;
static uiSpinbox *spinboxFrom;
static uiSpinbox *spinboxTo;


#define CHANGED(what) \
	static void on ## what ## Changed(ui ## what *this, void *data) \
	{ \
		int value; \
		printf("on %s changed\n", #what); \
		value = ui ## what ## Value(this); \
		uiSpinboxSetValue(spinbox, value); \
		uiSliderSetValue(slider, value); \
		uiProgressBarSetValue(pbar, value); \
	}
CHANGED(Spinbox)
CHANGED(Slider)

#define SETTOO(what, name, n) \
	static void set ## what ## Too ## name(uiButton *this, void *data) \
	{ \
		ui ## what ## SetValue(ui ## what(data), n); \
	}
SETTOO(Spinbox, Low, -80)
SETTOO(Spinbox, High, 80)
SETTOO(Slider, Low, -80)
SETTOO(Slider, High, 80)

static uiLabel *cboxLbl;
static uiCombobox *cbox;
static uiEditableCombobox *editable;
static uiRadioButtons *rb;

static void updateCB()
{
	char str[128];

	// snprintf() is not supported by visual studio 2013:
	// http://blogs.msdn.com/b/vcblog/archive/2013/07/19/c99-library-support-in-visual-studio-2013.aspx
	// we can't use _snprintf() in the test suite because that's msvc-only, so sprintf() it is.
	sprintf(str, "%d", uiComboboxNumItems(cbox));
	uiLabelSetText(cboxLbl, str);
}

static void appendCBRB(uiButton *b, void *data)
{
	uiComboboxAppend(cbox, "New Item");
	uiEditableComboboxAppend(editable, "New Item");
	uiRadioButtonsAppend(rb, "New Item");
	updateCB();
}

static void insertCB(uiButton *b, void *data)
{
	uiComboboxInsertAt(cbox, 0, "Inserted item");
	updateCB();
}

static void deleteCB(uiButton *b, void *data)
{
	uiComboboxDelete(cbox, 0);
	updateCB();
}

static void clearCB(uiButton *b, void *data)
{
	uiComboboxClear(cbox);
	updateCB();
}

static void onCBChanged(uiCombobox *c, void *data)
{
	printf("%s combobox changed to %d\n",
		(char *) data,
		(int) uiComboboxSelected(c));
	uiEditableComboboxSetText(editable, "changed");
}

static void onECBChanged(uiEditableCombobox *c, void *data)
{
	char *t;

	t = uiEditableComboboxText(c);
	printf("%s combobox changed to %s\n",
		(char *) data,
		t);
	uiFreeText(t);
}

static void onRBSelected(uiRadioButtons *r, void *data)
{
	printf("radio buttons %d\n", uiRadioButtonsSelected(r));
}

static void selectSecond(uiButton *b, void *data)
{
	// TODO combobox, editable
	uiRadioButtonsSetSelected(rb, 1);
}

static void selectNone(uiButton *b, void *data)
{
	// TODO combobox, editable
	uiRadioButtonsSetSelected(rb, -1);
}

static void sliderEnableToolTip(uiButton *b, void *data)
{
	uiSliderSetHasToolTip(uiSlider(data), 1);
	uiCheckboxSetChecked(checkbox, uiSliderHasToolTip(uiSlider(data)));
}

static void sliderDisableToolTip(uiButton *b, void *data)
{
	uiSliderSetHasToolTip(uiSlider(data), 0);
	uiCheckboxSetChecked(checkbox, uiSliderHasToolTip(uiSlider(data)));
}

static void setSliderRange(uiSpinbox *spinbox, void *data)
{
	uiSlider *s = data;

	uiSliderSetRange(s, uiSpinboxValue(spinboxFrom), uiSpinboxValue(spinboxTo));
}

static void onRangeSliderChanged(uiSlider *s, void *data)
{
	char str[128];
	uiLabel *lbl = data;

	// snprintf() is not supported by visual studio 2013:
	// http://blogs.msdn.com/b/vcblog/archive/2013/07/19/c99-library-support-in-visual-studio-2013.aspx
	// we can't use _snprintf() in the test suite because that's msvc-only, so sprintf() it is.
	sprintf(str, "%d", uiSliderValue(s));
	uiLabelSetText(lbl, str);
}

static void onSliderReleased(uiSlider *s, void *data){
	int value;

	printf("on Slider released\n");
	value = uiSliderValue(s);
	uiProgressBarSetValue(pbar2, value);
}

uiBox *makePage4(void)
{
	uiBox *page4;
	uiBox *hbox;
	uiSpinbox *xsb;
	uiButton *b;
	uiSlider *xsl;
	uiLabel *lbl;

	page4 = newVerticalBox();

	spinbox = uiNewSpinbox(0, 100);
	uiSpinboxOnChanged(spinbox, onSpinboxChanged, NULL);
	uiBoxAppend(page4, uiControl(spinbox), 0);

	slider = uiNewSlider(0, 100);
	uiSliderOnChanged(slider, onSliderChanged, NULL);
	uiSliderOnReleased(slider, onSliderReleased, NULL);
	uiBoxAppend(page4, uiControl(slider), 0);

	hbox = newHorizontalBox();
	slider = uiNewSlider(0, 100);
	uiBoxAppend(hbox, uiControl(slider), 1);
	b = uiNewButton("Enable ToolTip");
	uiButtonOnClicked(b, sliderEnableToolTip, slider);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Disable ToolTip");
	uiButtonOnClicked(b, sliderDisableToolTip, slider);
	uiBoxAppend(hbox, uiControl(b), 0);
	checkbox = uiNewCheckbox("Has ToolTip");
	uiControlDisable(uiControl(checkbox));
	uiCheckboxSetChecked(checkbox, uiSliderHasToolTip(slider));
	uiBoxAppend(hbox, uiControl(checkbox), 0);
	uiBoxAppend(page4, uiControl(hbox), 0);

	pbar = uiNewProgressBar();
	uiBoxAppend(page4, uiControl(pbar), 0);

	pbar2 = uiNewProgressBar();
	uiBoxAppend(page4, uiControl(pbar2), 0);

	uiBoxAppend(page4, uiControl(uiNewHorizontalSeparator()), 0);

	hbox = newHorizontalBox();
	xsb = uiNewSpinbox(-40, 40);
	uiBoxAppend(hbox, uiControl(xsb), 0);
	b = uiNewButton("Bad Low");
	uiButtonOnClicked(b, setSpinboxTooLow, xsb);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Bad High");
	uiButtonOnClicked(b, setSpinboxTooHigh, xsb);
	uiBoxAppend(hbox, uiControl(b), 0);
	uiBoxAppend(page4, uiControl(hbox), 0);

	hbox = newHorizontalBox();
	xsl = uiNewSlider(-40, 40);
	uiBoxAppend(hbox, uiControl(xsl), 0);
	b = uiNewButton("Bad Low");
	uiButtonOnClicked(b, setSliderTooLow, xsl);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Bad High");
	uiButtonOnClicked(b, setSliderTooHigh, xsl);
	uiBoxAppend(hbox, uiControl(b), 0);
	uiBoxAppend(page4, uiControl(hbox), 0);

	uiBoxAppend(page4, uiControl(uiNewHorizontalSeparator()), 0);

	lbl = uiNewLabel("100");
	uiBoxAppend(page4, uiControl(lbl), 0);
	hbox = newHorizontalBox();
	spinboxFrom = uiNewSpinbox(0, 1000);
	uiSpinboxSetValue(spinboxFrom, 100);
	uiBoxAppend(hbox, uiControl(spinboxFrom), 1);
	xsl = uiNewSlider(100, 200);
	uiSpinboxOnChanged(spinboxFrom, setSliderRange, xsl);
	uiBoxAppend(hbox, uiControl(xsl), 1);
	uiSliderOnChanged(xsl, onRangeSliderChanged, lbl);
	spinboxTo = uiNewSpinbox(0, 1000);
	uiSpinboxSetValue(spinboxTo, 200);
	uiSpinboxOnChanged(spinboxTo, setSliderRange, xsl);
	uiBoxAppend(hbox, uiControl(spinboxTo), 1);
	uiBoxAppend(page4, uiControl(hbox), 0);

	uiBoxAppend(page4, uiControl(uiNewHorizontalSeparator()), 0);

	hbox = newHorizontalBox();
	cboxLbl = uiNewLabel("0");
	uiBoxAppend(hbox, uiControl(cboxLbl), 0);
	cbox = uiNewCombobox();
	uiComboboxAppend(cbox, "Item 1");
	uiComboboxAppend(cbox, "Item 2");
	uiComboboxAppend(cbox, "Item 3");
	uiComboboxOnSelected(cbox, onCBChanged, "noneditable");
	updateCB();
	uiBoxAppend(hbox, uiControl(cbox), 1);
	uiBoxAppend(page4, uiControl(hbox), 0);

	editable = uiNewEditableCombobox();
	uiEditableComboboxAppend(editable, "Editable Item 1");
	uiEditableComboboxAppend(editable, "Editable Item 2");
	uiEditableComboboxAppend(editable, "Editable Item 3");
	uiEditableComboboxOnChanged(editable, onECBChanged, "editable");
	uiBoxAppend(page4, uiControl(editable), 0);

	rb = uiNewRadioButtons();
	uiRadioButtonsAppend(rb, "Item 1");
	uiRadioButtonsAppend(rb, "Item 2");
	uiRadioButtonsAppend(rb, "Item 3");
	uiRadioButtonsOnSelected(rb, onRBSelected, NULL);
	uiBoxAppend(page4, uiControl(rb), 0);

	hbox = newHorizontalBox();
	b = uiNewButton("Append");
	uiButtonOnClicked(b, appendCBRB, NULL);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Insert");
	uiButtonOnClicked(b, insertCB, NULL);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Delete");
	uiButtonOnClicked(b, deleteCB, NULL);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Clear");
	uiButtonOnClicked(b, clearCB, NULL);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Second");
	uiButtonOnClicked(b, selectSecond, NULL);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("None");
	uiButtonOnClicked(b, selectNone, NULL);
	uiBoxAppend(hbox, uiControl(b), 0);
	uiBoxAppend(page4, uiControl(hbox), 0);

	uiBoxAppend(page4, uiControl(uiNewHorizontalSeparator()), 0);

	uiBoxAppend(page4, uiControl(uiNewDateTimePicker()), 0);
	uiBoxAppend(page4, uiControl(uiNewDatePicker()), 0);
	uiBoxAppend(page4, uiControl(uiNewTimePicker()), 0);

	return page4;
}
