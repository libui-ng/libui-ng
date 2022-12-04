#import "uipriv_darwin.h"

// NSComboBoxes have no intrinsic width; we'll use the default Interface Builder width for them.
// NSPopUpButton is fine.
#define comboboxWidth 96

struct uiCombobox {
	uiDarwinControl c;
	NSPopUpButton *pb;
	NSArrayController *pbac;
	void (*onSelected)(uiCombobox *, void *);
	void *onSelectedData;
};

@interface uiprivCombobox : NSPopUpButton {
	uiCombobox *combobox;
}
- (id)initWithFrame:(NSRect)frame uiCombobox:(uiCombobox *)c;
- (IBAction)onSelected:(id)sender;
@end

@implementation uiprivCombobox

- (id)initWithFrame:(NSRect)frame uiCombobox:(uiCombobox *)c
{
	self = [super initWithFrame:frame pullsDown:NO];
	if (self) {
		self->combobox = c;

		[self setPreferredEdge:NSMinYEdge];

		[[self cell] setArrowPosition:NSPopUpArrowAtBottom];

		// Use the regular font size for consistency instead of the
		// Interface Builder defined one "Menu 13"
		uiDarwinSetControlFont(self, NSRegularControlSize);

		[self setTarget:self];
		[self setAction:@selector(onSelected:)];
	}
	return self;
}

- (IBAction)onSelected:(id)sender
{
	uiCombobox *c = self->combobox;

	(*(c->onSelected))(c, c->onSelectedData);
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiCombobox, pb)

static void uiComboboxDestroy(uiControl *cc)
{
	uiCombobox *c = uiCombobox(cc);

	[c->pb unbind:@"contentObjects"];
	[c->pb unbind:@"selectedIndex"];
	[c->pbac release];
	[c->pb release];
	uiFreeControl(uiControl(c));
}

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	[c->pbac addObject:uiprivToNSString(text)];
	uiControlEnable(uiControl(c));
}

void uiComboboxInsertAt(uiCombobox *c, int n, const char *text)
{
	int selected = uiComboboxSelected(c);

	[c->pbac insertObject:uiprivToNSString(text) atArrangedObjectIndex:n];
	uiControlEnable(uiControl(c));

	if (n <= selected)
		uiComboboxSetSelected(c, selected+1);
	else
		uiComboboxSetSelected(c, selected);
}

void uiComboboxDelete(uiCombobox *c, int n)
{
	int selected = uiComboboxSelected(c);

	[c->pbac removeObjectAtArrangedObjectIndex:n];

	if (n < selected)
		uiComboboxSetSelected(c, selected-1);
	if (uiComboboxNumItems(c) == 0) {
		uiControlDisable(uiControl(c));
		uiComboboxSetSelected(c, -1);
	}
}

void uiComboboxClear(uiCombobox *c)
{
	[c->pbac removeObjectsAtArrangedObjectIndexes:[[c->pbac arrangedObjects]
		indexesOfObjectsPassingTest:^BOOL(id obj, NSUInteger idx, BOOL *stop) {
			// remove all items
			return TRUE;
	}]];
	uiComboboxSetSelected(c, -1);
	uiControlDisable(uiControl(c));
}

int uiComboboxNumItems(uiCombobox *c)
{
	return [[c->pbac arrangedObjects] count];
}

int uiComboboxSelected(uiCombobox *c)
{
	return [c->pb indexOfSelectedItem];
}

void uiComboboxSetSelected(uiCombobox *c, int n)
{
	[c->pb selectItemAtIndex:n];
}

void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data)
{
	c->onSelected = f;
	c->onSelectedData = data;
}

static void defaultOnSelected(uiCombobox *c, void *data)
{
	// do nothing
}

uiCombobox *uiNewCombobox(void)
{
	uiCombobox *c;

	uiDarwinNewControl(uiCombobox, c);

	c->pb = [[uiprivCombobox alloc] initWithFrame:NSZeroRect uiCombobox:c];

	// NSPopUpButton doesn't work like a combobox
	// - it automatically selects the first item
	// - it doesn't support duplicates
	// but we can use a NSArrayController and Cocoa bindings to bypass these restrictions
	c->pbac = [NSArrayController new];
	[c->pbac setAvoidsEmptySelection:NO];
	[c->pbac setSelectsInsertedObjects:NO];
	[c->pbac setAutomaticallyRearrangesObjects:NO];
	[c->pb bind:@"contentValues"
		toObject:c->pbac
		withKeyPath:@"arrangedObjects"
		options:nil];
	[c->pb bind:@"selectedIndex"
		toObject:c->pbac
		withKeyPath:@"selectionIndex"
		options:nil];

	uiComboboxOnSelected(c, defaultOnSelected, NULL);
	uiComboboxSetSelected(c, -1);
	uiControlDisable(uiControl(c));

	return c;
}
