// 14 august 2015
#import "uipriv_darwin.h"

struct uiCheckbox {
	uiDarwinControl c;
	NSButton *button;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

@interface uiprivCheckbox : NSButton {
	uiCheckbox *checkbox;
}
- (id)initWithFrame:(NSRect)frame uiCheckbox:(uiCheckbox *)c;
- (IBAction)onToggled:(id)sender;
@end

@implementation uiprivCheckbox

- (id)initWithFrame:(NSRect)frame uiCheckbox:(uiCheckbox *)c
{
	self = [super initWithFrame:frame];
	if (self) {
		self->checkbox = c;

		[self setButtonType:NSSwitchButton];
		[self setBordered:NO];
		// doesn't seem to have an associated bezel style
		[self setTransparent:NO];

		[self setTarget:self];
		[self setAction:@selector(onToggled:)];
	}
	return self;
}

- (IBAction)onToggled:(id)sender
{
	uiCheckbox *c = self->checkbox;

	(*(c->onToggled))(c, c->onToggledData);
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiCheckbox, button)

static void uiCheckboxDestroy(uiControl *cc)
{
	uiCheckbox *c = uiCheckbox(cc);

	[c->button release];
	uiFreeControl(uiControl(c));
}

char *uiCheckboxText(uiCheckbox *c)
{
	return uiDarwinNSStringToText([c->button title]);
}

void uiCheckboxSetText(uiCheckbox *c, const char *text)
{
	[c->button setTitle:uiprivToNSString(text)];
}

void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *, void *), void *data)
{
	c->onToggled = f;
	c->onToggledData = data;
}

int uiCheckboxChecked(uiCheckbox *c)
{
	return [c->button state] == NSOnState;
}

void uiCheckboxSetChecked(uiCheckbox *c, int checked)
{
	NSInteger state;

	state = (checked) ? NSOnState : NSOffState;
	[c->button setState:state];
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	uiCheckbox *c;

	uiDarwinNewControl(uiCheckbox, c);

	c->button = [[uiprivCheckbox alloc] initWithFrame:NSZeroRect uiCheckbox:c];
	uiCheckboxSetText(c, text);
	uiDarwinSetControlFont(c->button, NSRegularControlSize);

	uiCheckboxOnToggled(c, defaultOnToggled, NULL);

	return c;
}
