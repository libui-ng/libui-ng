#include <stdio.h>
#include <ui.h>

static uiMultilineEntry *eventLog;
static uiMultilineEntry *dragData;
static uiDragDestination *dragDest;
static uiLabel *dragPos;

static uiCheckbox *contextText;
static uiCheckbox *contextURIs;

static uiCheckbox *opNone;
static uiCheckbox *opCopy;
static uiCheckbox *opLink;
static uiCheckbox *opMove;

static int moved;
static uiDragOperation dragOp;

static void updateDragContextDragTypes(uiDragContext *dc)
{
	int types;

	types = uiDragContextDragTypes(dc);
	uiCheckboxSetChecked(contextText, !!(types & uiDragTypeText));
	uiCheckboxSetChecked(contextURIs, !!(types & uiDragTypeURIs));
}

static void updateDragContextDragOperations(uiDragContext *dc)
{
	int ops;

	ops = uiDragContextDragOperations(dc);
	uiCheckboxSetChecked(opNone, !!(ops == uiDragOperationNone));
	uiCheckboxSetChecked(opCopy, !!(ops & uiDragOperationCopy));
	uiCheckboxSetChecked(opLink, !!(ops & uiDragOperationLink));
	uiCheckboxSetChecked(opMove, !!(ops & uiDragOperationMove));
}

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

static void updateDragContext(uiDragDestination *dd, uiDragContext *dc)
{
	updateDragContextDragTypes(dc);
	updateDragContextDragOperations(dc);
	updateDragContextPosition(dc);
	updateDragData(dd, dc);
}

static uiDragOperation onEnter(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	if (dragOp != uiDragOperationNone && !(uiDragContextDragOperations(dc) & dragOp)) {
		uiMultilineEntryAppend(eventLog, "Operation not supported by your file manager.\n");
		uiMultilineEntryAppend(eventLog, "Please use a different file manager to complete this test.\n\n");
	}

	updateDragContext(dd, dc);
	uiMultilineEntryAppend(eventLog, "Enter\n");
	return dragOp;
}

static uiDragOperation onMove(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	if (moved == 0) {
		moved = 1;
		uiMultilineEntryAppend(eventLog, "Move\n");
	}
	updateDragContext(dd, dc);
	return uiDragDestinationLastDragOperation(dd);
}

static void onExit(uiDragDestination *dd, void *senderData)
{
	moved = 0;
	uiMultilineEntryAppend(eventLog, "Leave\n");
}

static int onDrop(uiDragDestination *dd, uiDragContext *dc, void *senderData)
{
	moved = 0;
	uiMultilineEntryAppend(eventLog, "Drop\n");
	updateDragContext(dd, dc);
	return 1;
}

int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

int main(void)
{
	uiBox *vbox;
	uiBox *hbox;
	uiLabel *label;
	uiGroup *group;
	uiForm *form;
	uiInitOptions o = {0};
	const char *err;
	uiWindow *w;

	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "Error initializing libui-ng: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	w = uiNewWindow("Drag & Drop", 800, 600, 0);
	uiWindowOnClosing(w, onClosing, NULL);

	moved = 0;
	dragOp = uiDragOperationCopy;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);
	uiWindowSetChild(w, uiControl(vbox));

	label = uiNewLabel("Drag & Drop Zone\nDrag & Drop Zone\nDrag & Drop Zone");
	uiBoxAppend(vbox, uiControl(label), 0);

	uiBoxAppend(vbox, uiControl(uiNewHorizontalSeparator()), 0);

	// uiDragContext
	group = uiNewGroup("uiDragContext");
	uiGroupSetMargined(group, 1);
	uiBoxAppend(vbox, uiControl(group), 1);

	form = uiNewForm();
	uiFormSetPadded(form, 1);
	uiGroupSetChild(group, uiControl(form));

	// uiDragContextDragTypes
	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);

	contextText = uiNewCheckbox("Text");
	uiControlDisable(uiControl(contextText));
	uiBoxAppend(hbox, uiControl(contextText), 0);
	contextURIs = uiNewCheckbox("URIs");
	uiControlDisable(uiControl(contextURIs));
	uiBoxAppend(hbox, uiControl(contextURIs), 0);
	// darwin layout fix; TODO fix uiBox and remove
	uiBoxAppend(hbox, uiControl(uiNewLabel("")), 1);

	uiFormAppend(form, "uiDragContextDragTypes", uiControl(hbox), 0);

	// uiDragContextDragOperations
	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);

	opNone = uiNewCheckbox("None");
	uiControlDisable(uiControl(opNone));
	uiBoxAppend(hbox, uiControl(opNone), 0);
	opCopy = uiNewCheckbox("Copy");
	uiControlDisable(uiControl(opCopy));
	uiBoxAppend(hbox, uiControl(opCopy), 0);
	opLink = uiNewCheckbox("Link");
	uiControlDisable(uiControl(opLink));
	uiBoxAppend(hbox, uiControl(opLink), 0);
	opMove = uiNewCheckbox("Move");
	uiControlDisable(uiControl(opMove));
	uiBoxAppend(hbox, uiControl(opMove), 0);

	uiFormAppend(form, "uiDragContextDragOperations", uiControl(hbox), 0);

	// uiDragContextPosition
	dragPos = uiNewLabel("");
	uiFormAppend(form, "uiDragContextPosition", uiControl(dragPos), 0);

	// uiDragContextDragData
	dragData = uiNewMultilineEntry();
	uiFormAppend(form, "uiDragContextDragData", uiControl(dragData), 1);

	// OnEvent
	eventLog = uiNewMultilineEntry();
	uiBoxAppend(vbox, uiControl(eventLog), 1);

	dragDest = uiNewDragDestination();
	uiDragDestinationSetAcceptTypes(dragDest, uiDragTypeText | uiDragTypeURIs);

	uiDragDestinationOnEnter(dragDest, onEnter, NULL);
	uiDragDestinationOnMove(dragDest, onMove, NULL);
	uiDragDestinationOnExit(dragDest, onExit, NULL);
	uiDragDestinationOnDrop(dragDest, onDrop, NULL);

	uiControlRegisterDragDestination(uiControl(label), dragDest);

	uiControlShow(uiControl(w));
	uiMain();
	uiUninit();
	return 0;
}

