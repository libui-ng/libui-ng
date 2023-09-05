// 14 august 2015
#import "uipriv_darwin.h"

@interface libui_spinbox : NSView<NSTextFieldDelegate> {
	NSTextField *tf;
	NSNumberFormatter *formatter;
	NSStepper *stepper;

	double value;
	double minimum;
	double maximum;

	uiSpinbox *spinbox;
}
- (id)initWithFrame:(NSRect)r spinbox:(uiSpinbox *)sb;
// see https://github.com/andlabs/ui/issues/82
- (double)libui_value;
- (void)libui_setValue:(double)val;
- (void)setMinimum:(double)min;
- (void)setMaximum:(double)max;
- (IBAction)stepperClicked:(id)sender;
- (void)controlTextDidChange:(NSNotification *)note;
@end

struct uiSpinbox {
	uiDarwinControl c;
	libui_spinbox *spinbox;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
	int precision;
};

// yes folks, this varies by operating system! woo!
// 10.10 started drawing the NSStepper one point too low, so we have to fix it up conditionally
// TODO test this; we'll probably have to substitute 10_9
static CGFloat stepperYDelta(void)
{
	// via https://developer.apple.com/library/mac/releasenotes/AppKit/RN-AppKit/
	if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_9)
		return 0;
	return -1;
}

@implementation libui_spinbox

- (id)initWithFrame:(NSRect)r spinbox:(uiSpinbox *)sb
{
	self = [super initWithFrame:r];
	if (self) {
		self->tf = uiprivNewEditableTextField();
		[self->tf setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->formatter = [NSNumberFormatter new];
		[self->formatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
		[self->formatter setLocalizesFormat:NO];
		[self->formatter setUsesGroupingSeparator:NO];
		[self->formatter setHasThousandSeparators:NO];
		[self->formatter setMinimumFractionDigits:sb->precision];
		[self->formatter setMaximumFractionDigits:sb->precision];
		[self->tf setFormatter:self->formatter];

		self->stepper = [[NSStepper alloc] initWithFrame:NSZeroRect];
		[self->stepper setValueWraps:NO];
		[self->stepper setAutorepeat:YES];              // hold mouse button to step repeatedly
		[self->stepper setTranslatesAutoresizingMaskIntoConstraints:NO];

		double step = 1.0 / pow(10.0, sb->precision);
		[self->stepper setIncrement:step];

		[self->tf setDelegate:self];
		[self->stepper setTarget:self];
		[self->stepper setAction:@selector(stepperClicked:)];

		[self addSubview:self->tf];
		[self addSubview:self->stepper];

		[self addConstraint:uiprivMkConstraint(self->tf, NSLayoutAttributeLeading,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeLeading,
			1, 0,
			@"uiSpinbox left edge")];
		[self addConstraint:uiprivMkConstraint(self->stepper, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeTrailing,
			1, 0,
			@"uiSpinbox right edge")];
		[self addConstraint:uiprivMkConstraint(self->tf, NSLayoutAttributeTop,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeTop,
			1, 0,
			@"uiSpinbox top edge text field")];
		[self addConstraint:uiprivMkConstraint(self->tf, NSLayoutAttributeBottom,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeBottom,
			1, 0,
			@"uiSpinbox bottom edge text field")];
		[self addConstraint:uiprivMkConstraint(self->stepper, NSLayoutAttributeTop,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeTop,
			1, stepperYDelta(),
			@"uiSpinbox top edge stepper")];
		[self addConstraint:uiprivMkConstraint(self->stepper, NSLayoutAttributeBottom,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeBottom,
			1, stepperYDelta(),
			@"uiSpinbox bottom edge stepper")];
		[self addConstraint:uiprivMkConstraint(self->tf, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			self->stepper, NSLayoutAttributeLeading,
			1, -3,		// arbitrary amount; good enough visually (and it seems to match NSDatePicker too, at least on 10.11, which is even better)
			@"uiSpinbox space between text field and stepper")];

		self->spinbox = sb;
	}
	return self;
}

- (void)dealloc
{
	[self->tf setDelegate:nil];
	[self->tf removeFromSuperview];
	[self->tf release];
	[self->formatter release];
	[self->stepper setTarget:nil];
	[self->stepper removeFromSuperview];
	[self->stepper release];
	[super dealloc];
}

- (double)libui_value
{
	return self->value;
}

- (void)libui_setValue:(double)val
{
	self->value = val;
	if (self->value < self->minimum)
		self->value = self->minimum;
	if (self->value > self->maximum)
		self->value = self->maximum;
	[self->tf setDoubleValue:self->value];
	[self->stepper setDoubleValue:self->value];
}

- (void)setMinimum:(double)min
{
	self->minimum = min;
	[self->formatter setMinimum:[NSNumber numberWithDouble:self->minimum]];
	[self->stepper setMinValue:(self->minimum)];
}

- (void)setMaximum:(double)max
{
	self->maximum = max;
	[self->formatter setMaximum:[NSNumber numberWithDouble:self->maximum]];
	[self->stepper setMaxValue:(self->maximum)];
}

- (IBAction)stepperClicked:(id)sender
{
	[self libui_setValue:[self->stepper doubleValue]];
	(*(self->spinbox->onChanged))(self->spinbox, self->spinbox->onChangedData);
}

- (void)controlTextDidChange:(NSNotification *)note
{
	[self libui_setValue:[self->tf doubleValue]];
	(*(self->spinbox->onChanged))(self->spinbox, self->spinbox->onChangedData);
}

@end

uiDarwinControlAllDefaults(uiSpinbox, spinbox)

int uiSpinboxValue(uiSpinbox *s)
{
	return (int)[s->spinbox libui_value];
}

double uiSpinboxValueDouble(uiSpinbox *s)
{
	return [s->spinbox libui_value];
}

void uiSpinboxSetValue(uiSpinbox *s, int value)
{
	[s->spinbox libui_setValue:(double)value];
}

void uiSpinboxSetValueDouble(uiSpinbox *s, double value)
{
	if (s->precision == 0) {
		uiprivUserBug("Setting value to double while spinbox is in int mode is not supported.");
		return;
	}
	[s->spinbox libui_setValue:value];
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

uiSpinbox *uiNewSpinbox(int min, int max)
{
	return uiNewSpinboxDouble((double)min, (double)max, 0);
}

uiSpinbox *uiNewSpinboxDouble(double min, double max, int precision)
{
	uiSpinbox *s;
	double temp;

	if (min >= max) {
		temp = min;
		min = max;
		max = temp;
	}

	uiDarwinNewControl(uiSpinbox, s);

	s->precision = fmax(0, fmin(20, precision));

	s->spinbox = [[libui_spinbox alloc] initWithFrame:NSZeroRect spinbox:s];
	[s->spinbox setMinimum:min];
	[s->spinbox setMaximum:max];
	[s->spinbox libui_setValue:min];

	uiSpinboxOnChanged(s, defaultOnChanged, NULL);

	return s;
}
