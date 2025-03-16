// 14 august 2015
#import "uipriv_darwin.h"

struct uiLabel {
	uiDarwinControl c;
	NSTextField *textfield;
};

uiDarwinControlAllDefaults(uiLabel, textfield)

char *uiLabelText(const uiLabel *l)
{
	return uiDarwinNSStringToText([l->textfield stringValue]);
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	[l->textfield setStringValue:uiprivToNSString(text)];
}

NSTextField *uiprivNewLabel(NSString *str)
{
	NSTextField *tf;

	tf = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[tf setStringValue:str];
	[tf setEditable:NO];
	[tf setSelectable:NO];
	[tf setDrawsBackground:NO];
	uiprivNSTextFieldSetStyleLabel(tf);
	return tf;
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;

	uiDarwinNewControl(uiLabel, l);

	l->textfield = uiprivNewLabel(uiprivToNSString(text));

	return l;
}
