#import "uipriv_darwin.h"

@implementation NSTextField (ui)

// Settings are based on the interface builder defaults.
- (void)uiSetStyleLabel
{
	uiDarwinSetControlFont(self, NSRegularControlSize);

	[self setBordered:NO];
	[self setBezeled:NO];

	// auto correct is handled in window_darwin.m
	[[self cell] setLineBreakMode:NSLineBreakByClipping];
	[[self cell] setScrollable:YES];
}

- (void)uiSetStyleEntry
{
	[self uiSetStyleLabel];

	[self setSelectable:YES];
	[self setBezeled:YES];
	[self setBezelStyle:NSTextFieldSquareBezel];
}

- (void)uiSetStyleSearchEntry
{
	[self uiSetStyleEntry];

	[self setBezelStyle:NSTextFieldRoundedBezel];
}

@end
