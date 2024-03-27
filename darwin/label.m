// 14 august 2015
#import "uipriv_darwin.h"

struct uiLabel {
	uiDarwinControl c;
	NSTextField *textfield;
};

@interface uiprivNSTextFieldLabel : NSTextField<NSDraggingDestination> {
	uiLabel *textfield;
}
- (id)initWithFrame:(NSRect)frame uiLabel:(uiLabel *)l;
@end

@implementation uiprivNSTextFieldLabel

uiDarwinDragDestinationMethods(textfield)

- (id)initWithFrame:(NSRect)frame uiLabel:(uiLabel *)l
{
	self = [super initWithFrame:frame];
	if (self) {
		self->textfield = l;
	}
	return self;
}

@end

uiDarwinControlAllDefaults(uiLabel, textfield)

char *uiLabelText(uiLabel *l)
{
	return uiDarwinNSStringToText([l->textfield stringValue]);
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	[l->textfield setStringValue:uiprivToNSString(text)];
}

static void labelSetStyle(NSTextField *tf)
{
	[tf setEditable:NO];
	[tf setSelectable:NO];
	[tf setDrawsBackground:NO];
	uiprivNSTextFieldSetStyleLabel(tf);
}

NSTextField *uiprivNewLabel(NSString *str)
{
	NSTextField *tf;

	tf = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[tf setStringValue:str];
	labelSetStyle(tf);

	return tf;
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;

	uiDarwinNewControl(uiLabel, l);

	l->textfield = [[uiprivNSTextFieldLabel alloc] initWithFrame:NSZeroRect uiLabel:l];
	[l->textfield setStringValue:uiprivToNSString(text)];
	labelSetStyle(l->textfield);

	return l;
}
