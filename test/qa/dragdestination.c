#include <stdio.h>
#include "qa.h"

static int moved;
static uiMultilineEntry *eventLog;
static uiMultilineEntry *dragData;
static uiDragDestination *dragDest;
static uiLabel *dragPos;
static uiDragOperation dragOp;

static void updateDragContextPosition(uiDragContext *dc)
{
	char str[256];
	int x;
	int y;

	uiDragContextPosition(dc, &x, &y);
	sprintf(str, "%d:%d", x, y);
	uiLabelSetText(dragPos, str);
}

static void updateDragData(uiDragDestination *dd, uiDragContext *dc)
{
	int i;
	int types;
	int accepted;
	uiDragData *data;

	uiMultilineEntrySetText(dragData, "");

	types = uiDragContextDragTypes(dc);
	accepted = uiDragDestinationAcceptTypes(dd);

	if (types & accepted & uiDragTypeText) {
		data = uiDragContextDragData(dc, uiDragTypeText);
		if (data != NULL) {
			uiMultilineEntryAppend(dragData, "text:\n");
			uiMultilineEntryAppend(dragData, data->data.text);
			uiMultilineEntryAppend(dragData, "\n");
			uiFreeDragData(data);
		}
	}

	if (types & accepted & uiDragTypeURIs) {
		data = uiDragContextDragData(dc, uiDragTypeURIs);
		if (data != NULL) {
			uiMultilineEntryAppend(dragData, "URIs:\n");
			for (i = 0; i < data->data.URIs.numURIs; ++i) {
				uiMultilineEntryAppend(dragData, data->data.URIs.URIs[i]);
				uiMultilineEntryAppend(dragData, "\n");
			}
			uiFreeDragData(data);
		}
	}
}

const char *dragdestinationDefaultNoAcceptGuide() {
	return
	"1.\tOpen a system file manager that supports drag and drop operations.\n"
	"\tIn your libui-ng source directory navigate to `test/data`. You should\n"
	"\tfind two files present `ascii` and `\u00fcnicode`.\n"
	"\n"
	"2.\tDrag a file `ascii` over the `Drag & Drop Zone`. The cursor should\n"
	"\tnot change compared to the cursor displayed in the surrounding\n"
	"\twindow and indicate that file drop is not supported.\n"
	"\n"
	;
}

uiControl* dragdestinationDefaultNoAccept()
{
	uiBox *vbox;
	uiLabel *label;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	label = uiNewLabel("Drag & Drop Zone\nDrag & Drop Zone\nDrag & Drop Zone");
	uiBoxAppend(vbox, uiControl(label), 0);

	dragDest = uiNewDragDestination();
	uiDragDestinationSetAcceptTypes(dragDest, uiDragTypeURIs);

	uiControlRegisterDragDestination(uiControl(label), dragDest);

	return uiControl(vbox);
}

static uiDragOperation onEnterLog(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	if (dragOp != uiDragOperationNone && !(uiDragContextDragOperations(dc) & dragOp)) {
		uiMultilineEntryAppend(eventLog, "Operation not supported by your file manager.\n");
		uiMultilineEntryAppend(eventLog, "Please use a different file manager to complete this test.\n\n");
	}

	uiMultilineEntryAppend(eventLog, "Enter\n");
	return dragOp;
}

static uiDragOperation onMoveLog(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	if (moved == 0) {
		moved = 1;
		uiMultilineEntryAppend(eventLog, "Move\n");
	}
	return uiDragDestinationLastDragOperation(dd);
}

static void onExitLog(uiDragDestination *dd, void *senderData)
{
	moved = 0;
	uiMultilineEntryAppend(eventLog, "Exit\n");
}

static int onDropLog(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	moved = 0;
	uiMultilineEntryAppend(eventLog, "Drop\n");
	return 1;
}

uiControl* makeDragDestinationReturnDragOperation(uiDragOperation op)
{
	uiBox *vbox;
	uiLabel *label;

	moved = 0;
	dragOp = op;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	label = uiNewLabel("Drag & Drop Zone\nDrag & Drop Zone\nDrag & Drop Zone");
	uiBoxAppend(vbox, uiControl(label), 0);

	uiBoxAppend(vbox, uiControl(uiNewHorizontalSeparator()), 0);

	eventLog = uiNewMultilineEntry();
	uiBoxAppend(vbox, uiControl(eventLog), 1);

	dragDest = uiNewDragDestination();
	uiDragDestinationSetAcceptTypes(dragDest, uiDragTypeURIs);

	uiDragDestinationOnEnter(dragDest, onEnterLog, NULL);
	uiDragDestinationOnMove(dragDest, onMoveLog, NULL);
	uiDragDestinationOnExit(dragDest, onExitLog, NULL);
	uiDragDestinationOnDrop(dragDest, onDropLog, NULL);

	uiControlRegisterDragDestination(uiControl(label), dragDest);

	return uiControl(vbox);
}

const char *dragdestinationReturnDragOperationNoneGuide() {
	return
	"1.\tOpen a system file manager that supports drag and drop operations.\n"
	"\tIn your libui-ng source directory navigate to `test/data`.\n"
	"\n"
	"2.\tDrag the file `ascii` over the `Drag & Drop Zone`.\n"
	"\tThe cursor should not change compared to the cursor displayed in the\n"
	"\tsurrounding window and indicate that file drop is not supported.\n"
	"\tThe log below should read `Enter` and `Move`.\n"
	"\n"
	"3.\tDrag the file out of the `Drag & Drop Zone`.\n"
	"\tAnother line should appear in the log reading `Exit`.\n"
	"\n"
	"4.\tDrag the file over the `Drag & Drop Zone` again.\n"
	"\tTwo more lines should appear in the log reading `Enter` and `Move`\n"
	"\n"
	"5.\tDrop the file on the `Drag & Drop Zone`.\n"
	"\tA new line should appear in the log reading `Exit`.\n"
	"\tThe file `ascii` in your file manager should be unaffected.\n"
	"\n"
	;
}

uiControl* dragdestinationReturnDragOperationNone()
{
	return makeDragDestinationReturnDragOperation(uiDragOperationNone);
}

const char *dragdestinationReturnDragOperationCopyGuide() {
	return
	"1.\tOpen a system file manager that supports drag and drop operations.\n"
	"\tIn your libui-ng source directory navigate to `test/data`.\n"
	"\n"
	"2.\tDrag the file `ascii` over the `Drag & Drop Zone`.\n"
	"\tThe cursor should change to a cursor that suggests that the file can\n"
	"\tbe copied and file drop is supported.\n"
	"\tThe log below should read `Enter` and `Move`.\n"
	"\n"
	"3.\tDrag the file out of the `Drag & Drop Zone`.\n"
	"\tAnother line should appear in the log reading `Exit`.\n"
	"\n"
	"4.\tDrag the file over the `Drag & Drop Zone` again.\n"
	"\tTwo more lines should appear in the log reading `Enter` and `Move`\n"
	"\n"
	"5.\tDrop the file on the `Drag & Drop Zone`.\n"
	"\tA new line should appear in the log reading `Drop`.\n"
	"\tThe file `ascii` in your file manager should be unaffected.\n"
	"\n"
	;
}

uiControl* dragdestinationReturnDragOperationCopy()
{
	return makeDragDestinationReturnDragOperation(uiDragOperationCopy);
}

const char *dragdestinationReturnDragOperationLinkGuide() {
	return
	"1.\tOpen a system file manager that supports drag and drop operations.\n"
	"\tIn your libui-ng source directory navigate to `test/data`.\n"
	"\n"
	"2.\tDrag the file `ascii` over the `Drag & Drop Zone`.\n"
	"\tThe cursor should change to a cursor that suggests that the file can\n"
	"\tbe linked and file drop is supported.\n"
	"\tThe log below should read `Enter` and `Move`.\n"
	"\n"
	"3.\tDrag the file out of the `Drag & Drop Zone`.\n"
	"\tAnother line should appear in the log reading `Exit`.\n"
	"\n"
	"4.\tDrag the file over the `Drag & Drop Zone` again.\n"
	"\tTwo more lines should appear in the log reading `Enter` and `Move`\n"
	"\n"
	"5.\tDrop the file on the `Drag & Drop Zone`.\n"
	"\tA new line should appear in the log reading `Drop`.\n"
	"\tThe file `ascii` in your file manager should be unaffected.\n"
	"\n"
	;
}

uiControl* dragdestinationReturnDragOperationLink()
{
	return makeDragDestinationReturnDragOperation(uiDragOperationLink);
}

const char *dragdestinationReturnDragOperationMoveGuide() {
	return
	"1.\tOpen a system file manager that supports drag and drop operations.\n"
	"\tIn your libui-ng source directory navigate to `test/data`.\n"
	"\n"
	"2.\tDrag the file `ascii` over the `Drag & Drop Zone`.\n"
	"\tThe cursor should change to a cursor that suggests that the file can\n"
	"\tbe moved and file drop is supported.\n"
	"\tThe log below should read `Enter` and `Move`.\n"
	"\n"
	"3.\tDrag the file out of the `Drag & Drop Zone`.\n"
	"\tAnother line should appear in the log reading `Exit`.\n"
	"\n"
	"4.\tDrag the file over the `Drag & Drop Zone` again.\n"
	"\tTwo more lines should appear in the log reading `Enter` and `Move`\n"
	"\n"
	"5.\tDrop the file on the `Drag & Drop Zone`.\n"
	"\tA new line should appear in the log reading `Drop`.\n"
	"\tThe file `ascii` in your file manager should be unaffected.\n"
	"\n"
	;
}

uiControl* dragdestinationReturnDragOperationMove()
{
	return makeDragDestinationReturnDragOperation(uiDragOperationMove);
}

static uiDragOperation onEnterMovePos(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	updateDragContextPosition(dc);
	return uiDragOperationCopy;
}

static int onDropPos(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	updateDragContextPosition(dc);
	return 1;
}

const char *dragdestinationDragContextPositionGuide() {
	return
	"1.\tOpen a system file manager that supports drag and drop operations.\n"
	"\tIn your libui-ng source directory navigate to `test/data`.\n"
	"\n"
	"2.\tDrag the file `ascii` over the `Drag & Drop Zone`.\n"
	"\n"
	"3.\tDrag the file into the top left corner of the `Drag & Drop Zone`.\n"
	"\tObserve how the coordinates converge towards 0:0.\n"
	"\n"
	"4.\tDrag the file into the bottom right corner of the\n"
	"\t`Drag & Drop Zone`. Observe how the coordinates increase.\n"
	"\n"
	;
}

uiControl* dragdestinationDragContextPosition()
{
	uiBox *vbox;
	uiLabel *label;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	label = uiNewLabel("Drag & Drop Zone\nDrag & Drop Zone\nDrag & Drop Zone");
	uiBoxAppend(vbox, uiControl(label), 0);

	uiBoxAppend(vbox, uiControl(uiNewHorizontalSeparator()), 0);

	dragPos = uiNewLabel("");
	uiBoxAppend(vbox, uiControl(dragPos), 0);

	dragDest = uiNewDragDestination();
	uiDragDestinationSetAcceptTypes(dragDest, uiDragTypeURIs);

	uiDragDestinationOnEnter(dragDest, onEnterMovePos, NULL);
	uiDragDestinationOnMove(dragDest, onEnterMovePos, NULL);
	uiDragDestinationOnDrop(dragDest, onDropPos, NULL);

	uiControlRegisterDragDestination(uiControl(label), dragDest);

	return uiControl(vbox);
}

static uiDragOperation onEnterData(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	updateDragData(dd, dc);
	return uiDragOperationCopy;
}

static uiDragOperation onMoveData(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	if (moved == 0) {
		moved = 1;
		updateDragData(dd, dc);
	}
	return uiDragDestinationLastDragOperation(dd);
}

static void onExitData(uiDragDestination *dd, void *senderData)
{
	moved = 0;
}

static int onDropData(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	moved = 0;
	updateDragData(dd, dc);
	return 1;
}

const char *dragdestinationDragContextDragDataGuide() {
	return
	"1.\tOpen a system file manager that supports drag and drop operations.\n"
	"\tIn your libui-ng source directory navigate to `test/data`.\n"
	"\n"
	"2.\tDrag the file `ascii` over the `Drag & Drop Zone`. You should see:\n"
	"\t```\n\tURIs:\n\t/path/to/ascii\n\t```\n"
	"\tYou may see more data types being listed that shall be ignored.\n"
	"\n"
	"3.\tDrop the file. The output should remain unchanged.\n"
	"\n"
	"4.\tDrag both files `ascii` and `\u00fcnicode` over the\n"
	"\t`Drag & Drop Zone`. You should see something akin to:\n"
	"\t```\n\tURIs:\n\t/path/to/ascii\n\t/path/to/\u00fcnicode\n\t```\n"
	"\tPath separators will differ depending on your system and any file\n"
	"\tordering shall be ignored.\n"
	"\tYou may see more data types being listed that shall be ignored.\n"
	"\n"
	"5.\tOpen the file `ascii` with an editor that supports text drag and\n"
	"\tdrop. Select the contained `ascii` text and drag it over the\n"
	"\t`Drag & Drop Zone`. You should see:\n"
	"\t```\n\ttext:\n\tascii\n\t```\n"
	"\n"
	"6.\tOpen the file `\u00fcnicode` with an editor that supports text\n"
	"\tdrag and drop. Select the contained `\u00fcnicode` text and drag\n"
	"\tit over the `Drag & Drop Zone`. You should see:\n"
	"\t```\n\ttext:\n\t\u00fcnicode\n\t```\n"
	"\n"
	;
}

uiControl* dragdestinationDragContextDragData()
{
	uiBox *vbox;
	uiLabel *label;

	moved = 0;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	label = uiNewLabel("Drag & Drop Zone\nDrag & Drop Zone\nDrag & Drop Zone");
	uiBoxAppend(vbox, uiControl(label), 0);

	uiBoxAppend(vbox, uiControl(uiNewHorizontalSeparator()), 0);

	dragData = uiNewMultilineEntry();
	uiBoxAppend(vbox, uiControl(dragData), 1);

	dragDest = uiNewDragDestination();
	uiDragDestinationSetAcceptTypes(dragDest, uiDragTypeURIs | uiDragTypeText);

	uiDragDestinationOnEnter(dragDest, onEnterData, NULL);
	uiDragDestinationOnMove(dragDest, onMoveData, NULL);
	uiDragDestinationOnExit(dragDest, onExitData, NULL);
	uiDragDestinationOnDrop(dragDest, onDropData, NULL);

	uiControlRegisterDragDestination(uiControl(label), dragDest);

	return uiControl(vbox);
}

