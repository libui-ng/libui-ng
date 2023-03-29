// 14 august 2015
#import "uipriv_darwin.h"

// Horizontal sliders have no intrinsic width; we'll use the default Interface Builder width for them.
// This will also be used for the initial frame size, to ensure the slider is always horizontal (see below).
#define sliderWidth 92

struct uiSlider {
	uiDarwinControl c;
	NSSlider *slider;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
	void (*onReleased)(uiSlider *, void *);
	void *onReleasedData;
	BOOL hasToolTip;
};

static void _uiSliderUpdateToolTip(uiSlider *s);

@interface uiprivSlider : NSSlider {
	uiSlider *slider;
}
- (id)initWithFrame:(NSRect)frame uiSlider:(uiSlider *)s;
- (IBAction)onChanged:(id)sender;
@end

@implementation uiprivSlider

- (id)initWithFrame:(NSRect)frame uiSlider:(uiSlider *)s
{
	self = [super initWithFrame:frame];
	if (self) {
		self->slider = s;

		[self setAllowsTickMarkValuesOnly:NO];
		[self setNumberOfTickMarks:0];
		[self setTickMarkPosition:NSTickMarkAbove];
		[[self cell] setSliderType:NSLinearSlider];

		[self setTarget:self];
		[self setAction:@selector(onChanged:)];
	}
	return self;
}

- (IBAction)onChanged:(id)sender
{
	uiSlider *s = self->slider;

	NSEvent *currentEvent = [[sender window] currentEvent];
	if([currentEvent type] == NSLeftMouseUp) {
		(*(s->onReleased))(s, s->onReleasedData);
	} else {
		(*(s->onChanged))(s, s->onChangedData);
	}

	if (s->hasToolTip)
		_uiSliderUpdateToolTip(s);
}

- (NSSize)intrinsicContentSize
{
	NSSize s;

	s = [super intrinsicContentSize];
	s.width = sliderWidth;
	return s;
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiSlider, slider)

static void uiSliderDestroy(uiControl *c)
{
	uiSlider *s = uiSlider(c);

	[s->slider release];
	uiFreeControl(uiControl(s));
}

static void _uiSliderUpdateToolTip(uiSlider *s)
{
	[s->slider setToolTip:[NSString stringWithFormat:@"%ld", [s->slider integerValue]]];
}

int uiSliderHasToolTip(uiSlider *s)
{
	return s->hasToolTip;
}

void uiSliderSetHasToolTip(uiSlider *s, int hasToolTip)
{
	s->hasToolTip = hasToolTip;

	if (hasToolTip)
		_uiSliderUpdateToolTip(s);
	else
		[s->slider setToolTip:nil];
}

int uiSliderValue(uiSlider *s)
{
	return [s->slider integerValue];
}

void uiSliderSetValue(uiSlider *s, int value)
{
	[s->slider setIntegerValue:value];
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

void uiSliderOnReleased(uiSlider *s, void (*f)(uiSlider *, void *), void *data)
{
	s->onReleased = f;
	s->onReleasedData = data;
}

static void defaultOnReleased(uiSlider *s, void *data)
{
	// do nothing
}

void uiSliderSetRange(uiSlider *s, int min, int max)
{
	int temp;

	if (min >= max) {
		temp = min;
		min = max;
		max = temp;
	}

	[s->slider setMinValue:min];
	[s->slider setMaxValue:max];
}

uiSlider *uiNewSlider(int min, int max)
{
	uiSlider *s;

	uiDarwinNewControl(uiSlider, s);

	// a horizontal slider is defined as one where the width > height, not by a flag
	// to be safe, don't use NSZeroRect, but make it horizontal from the get-go
	s->slider = [[uiprivSlider alloc]
		initWithFrame:NSMakeRect(0, 0, sliderWidth, 2)
		uiSlider:s];

	uiSliderSetRange(s, min, max);
	uiSliderSetHasToolTip(s, 1);

	uiSliderOnChanged(s, defaultOnChanged, NULL);
	uiSliderOnReleased(s, defaultOnReleased, NULL);


	return s;
}
