// 14 august 2015
#import "uipriv_darwin.h"

// So why did I split uiCombobox into uiCombobox and uiEditableCombobox? Here's (90% of the; the other 10% is GTK+ events) answer:
// When you type a value into a NSComboBox that just happens to be in the list, it will autoselect that item!
// I can't seem to find a workaround.
// Fortunately, there's other weird behaviors that made this split worth it.
// And besides, selected items make little sense with editable comboboxes... you either separate or combine them with the text entry :V

// NSComboBoxes have no intrinsic width; we'll use the default Interface Builder width for them.
#define comboboxWidth 96

@interface uiprivEditableCombobox : NSComboBox<NSComboBoxDelegate>
- (id)initWithFrame:(NSRect)frame uiEditableCombobox:(uiEditableCombobox *)cb;
- (void)controlTextDidChange:(NSNotification *)note;
- (void)comboBoxSelectionDidChange:(NSNotification *)note;
@end

struct uiEditableCombobox {
	uiDarwinControl c;
	NSComboBox *cb;
	void (*onChanged)(uiEditableCombobox *, void *);
	void *onChangedData;
};

@implementation uiprivEditableCombobox {
	uiEditableCombobox *combobox;
}

- (NSSize)intrinsicContentSize
{
	NSSize s;

	s = [super intrinsicContentSize];
	s.width = comboboxWidth;
	return s;
}

- (id)initWithFrame:(NSRect)frame uiEditableCombobox:(uiEditableCombobox *)cb
{
	self = [super initWithFrame:frame];
	if (self) {
		self->combobox = cb;

		[self setUsesDataSource:NO];
		[self setButtonBordered:YES];
		[self setCompletes:NO];

		[self setDelegate:self];
	}
	return self;
}

- (void)controlTextDidChange:(NSNotification *)note
{
	uiEditableCombobox *c = self->combobox;

	(*(c->onChanged))(c, c->onChangedData);
}

// the above doesn't handle when an item is selected; this will
- (void)comboBoxSelectionDidChange:(NSNotification *)note
{
	// except this is sent BEFORE the entry is changed, and that doesn't send the above, so
	// this is via http://stackoverflow.com/a/21059819/3408572 - it avoids the need to manage selected items
	// this still isn't perfect — I get residual changes to the same value while navigating the list — but it's good enough
	[self performSelector:@selector(controlTextDidChange:)
		withObject:note
		afterDelay:0];
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiEditableCombobox, cb)

static void uiEditableComboboxDestroy(uiControl *cc)
{
	uiEditableCombobox *c = uiEditableCombobox(cc);

	[c->cb release];
	uiFreeControl(uiControl(c));
}

void uiEditableComboboxAppend(uiEditableCombobox *c, const char *text)
{
	[c->cb addItemWithObjectValue:uiprivToNSString(text)];
}

char *uiEditableComboboxText(uiEditableCombobox *c)
{
	return uiDarwinNSStringToText([c->cb stringValue]);
}

void uiEditableComboboxSetText(uiEditableCombobox *c, const char *text)
{
	NSString *t;

	t = uiprivToNSString(text);
	[c->cb setStringValue:t];
	// yes, let's imitate the behavior that caused uiEditableCombobox to be separate in the first place!
	// just to avoid confusion when users see an option in the list in the text field but not selected in the list
	[c->cb selectItemWithObjectValue:t];
}

#if 0
// LONGTERM
void uiEditableComboboxSetSelected(uiEditableCombobox *c, int n)
{
	if (c->editable) {
		// see https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/ComboBox/Tasks/SettingComboBoxValue.html#//apple_ref/doc/uid/20000256
		id delegate;

		// this triggers the delegate; turn it off for now
		delegate = [c->cb delegate];
		[c->cb setDelegate:nil];

		// this seems to work fine for -1 too
		[c->cb selectItemAtIndex:n];
		if (n == -1)
			[c->cb setObjectValue:@""];
		else
			[c->cb setObjectValue:[c->cb objectValueOfSelectedItem]];

		[c->cb setDelegate:delegate];
		return;
	}
	[c->pb selectItemAtIndex:n];
}
#endif

void uiEditableComboboxOnChanged(uiEditableCombobox *c, void (*f)(uiEditableCombobox *c, void *data), void *data)
{
	c->onChanged = f;
	c->onChangedData = data;
}

static void defaultOnChanged(uiEditableCombobox *c, void *data)
{
	// do nothing
}

uiEditableCombobox *uiNewEditableCombobox(void)
{
	uiEditableCombobox *c;

	uiDarwinNewControl(uiEditableCombobox, c);

	c->cb = [[uiprivEditableCombobox alloc] initWithFrame:NSZeroRect uiEditableCombobox:c];

	uiDarwinSetControlFont(c->cb, NSRegularControlSize);

	uiEditableComboboxOnChanged(c, defaultOnChanged, NULL);

	return c;
}
