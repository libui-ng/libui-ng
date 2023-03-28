#import "uipriv_darwin.h"

// Settings are based on the interface builder defaults.
void uiprivNSTextFieldSetStyleLabel(NSTextField *t)
{
	uiDarwinSetControlFont(t, NSRegularControlSize);

	[t setBordered:NO];
	[t setBezeled:NO];

	// auto correct is handled in window_darwin.m
	[[t cell] setLineBreakMode:NSLineBreakByClipping];
	[[t cell] setScrollable:YES];
}

void uiprivNSTextFieldSetStyleEntry(NSTextField *t)
{
	uiprivNSTextFieldSetStyleLabel(t);

	[t setSelectable:YES];
	[t setBezeled:YES];
	[t setBezelStyle:NSTextFieldSquareBezel];
}

void uiprivNSTextFieldSetStyleSearchEntry(NSTextField *t)
{
	uiprivNSTextFieldSetStyleEntry(t);

	[t setBezelStyle:NSTextFieldRoundedBezel];
}

