#include <stdio.h>

#include "qa.h"

static void entryOnChangedCb(uiEntry *e, void *data)
{
	char str[32];
	uiLabel *label = data;
	static int count = 0;

	sprintf(str, "%d", ++count);
	uiLabelSetText(label, str);
}

const char *entryOnChangedGuide() {
	return
	"1.\tYou should see an empty text entry box. Next to it should be a label\n"
	"\tdisplaying `Changed count: 0`.\n"
	"\n"
	"2.\tFocus the entry box by left clicking it. Type `1`, `2`, `3`. The\n"
	"\tcontents of the entry box should read `123`. The label should read\n"
	"\t`Changed count: 3`.\n"
	"\n"
	"3.\tPress the back space key. The contents of the entry box should read\n"
	"\t`12`. The label should read `Changed count: 4`.\n"
	;
}

uiControl* entryOnChanged()
{
	uiBox *vbox;
	uiBox *hbox;
	uiEntry *entry;
	uiLabel *label;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiBoxAppend(vbox, uiControl(hbox), 0);

	entry = uiNewEntry();
	uiBoxAppend(hbox, uiControl(entry), 0);
	label = uiNewLabel("Changed count:");
	uiBoxAppend(hbox, uiControl(label), 0);
	label = uiNewLabel("0");
	uiBoxAppend(hbox, uiControl(label), 0);

	uiEntryOnChanged(entry, entryOnChangedCb, label);

	return uiControl(vbox);
}

static void passwordEntryOnChangedCb(uiEntry *e, void *data)
{
	char str[32];
	uiLabel *label = data;
	static int count = 0;

	sprintf(str, "%d", ++count);
	uiLabelSetText(label, str);
}

const char *passwordEntryOnChangedGuide() {
	return
	"1.\tYou should see an empty password entry box. Next to it should be a\n"
	"\tlabel displaying `Changed count: 0`.\n"
	"\n"
	"2.\tFocus the password box by left clicking it. Type `1`, `2`, `3`. The\n"
	"\tcontents of the password box should be obfuscated, `***`. The label\n"
	"\tshould read `Changed count: 3`.\n"
	"\n"
	"3.\tPress the back space key. The contents of the password box should\n"
	"\tremain obfuscated, `**`. The label should read `Changed count: 4`.\n"
	;
}

uiControl* passwordEntryOnChanged()
{
	uiBox *vbox;
	uiBox *hbox;
	uiEntry *entry;
	uiLabel *label;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiBoxAppend(vbox, uiControl(hbox), 0);

	entry = uiNewPasswordEntry();
	uiBoxAppend(hbox, uiControl(entry), 0);
	label = uiNewLabel("Changed count:");
	uiBoxAppend(hbox, uiControl(label), 0);
	label = uiNewLabel("0");
	uiBoxAppend(hbox, uiControl(label), 0);

	uiEntryOnChanged(entry, passwordEntryOnChangedCb, label);

	return uiControl(vbox);
}

static void searchEntryOnChangedCb(uiEntry *e, void *data)
{
	char str[32];
	uiLabel *label = data;
	static int count = 0;

	sprintf(str, "%d", ++count);
	uiLabelSetText(label, str);
}

const char *searchEntryOnChangedGuide() {
	return
	"1.\tYou should see an empty search box. Next to it should be a label\n"
	"\tdisplaying `Changed count: 0`.\n"
	"\n"
	"2.\tFocus the search box by left clicking it. Type `1`, `2`, `3` as fast\n"
	"\tas possible. The contents search box should read `123`. The label\n"
	"\tshould read a maximum `Changed count` of `3`. Depending on how\n"
	"\tfast you typed it ideally reads `1` or `2`.\n"
	"\n"
	"3.\tPress the back space key. The contents of the search box should read\n"
	"\t`12`. The label `Changed count` should have increased by one\n"
	"\tcompared to the prior step.\n"
	;
}

uiControl* searchEntryOnChanged()
{
	uiBox *vbox;
	uiBox *hbox;
	uiEntry *entry;
	uiLabel *label;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiBoxAppend(vbox, uiControl(hbox), 0);

	entry = uiNewSearchEntry();
	uiBoxAppend(hbox, uiControl(entry), 0);
	label = uiNewLabel("Changed count:");
	uiBoxAppend(hbox, uiControl(label), 0);
	label = uiNewLabel("0");
	uiBoxAppend(hbox, uiControl(label), 0);

	uiEntryOnChanged(entry, searchEntryOnChangedCb, label);

	return uiControl(vbox);
}

