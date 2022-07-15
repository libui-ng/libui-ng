// 13 august 2015
#import "uipriv_darwin.h"

struct uiButton {
	uiDarwinControl c;
	NSButton *button;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

@interface uiprivButton : NSButton {
	uiButton *button;
}
- (id)initWithFrame:(NSRect)frame uiButton:(uiButton *)b;
- (IBAction)onClicked:(id)sender;
@end

@implementation uiprivButton

- (id)initWithFrame:(NSRect)frame uiButton:(uiButton *)b
{
	self = [super initWithFrame:frame];
	if (self) {
		self->button = b;

		[self setButtonType:NSMomentaryPushInButton];
		[self setBordered:YES];
		[self setBezelStyle:NSRoundedBezelStyle];

		[self setTarget:self];
		[self setAction:@selector(onClicked:)];
	}
	return self;
}

- (IBAction)onClicked:(id)sender
{
	uiButton *b = self->button;

	(*(b->onClicked))(b, b->onClickedData);
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiButton, button)

static void uiButtonDestroy(uiControl *c)
{
	uiButton *b = uiButton(c);

	[b->button release];
	uiFreeControl(uiControl(b));
}

char *uiButtonText(uiButton *b)
{
	return uiDarwinNSStringToText([b->button title]);
}

void uiButtonSetText(uiButton *b, const char *text)
{
	[b->button setTitle:uiprivToNSString(text)];
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

uiButton *uiNewButton(const char *text)
{
	uiButton *b;

	uiDarwinNewControl(uiButton, b);

	b->button = [[uiprivButton alloc] initWithFrame:NSZeroRect uiButton:b];
	uiButtonSetText(b, text);
	uiDarwinSetControlFont(b->button, NSRegularControlSize);

	uiButtonOnClicked(b, defaultOnClicked, NULL);

	return b;
}
