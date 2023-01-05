// 14 august 2015
#import "uipriv_darwin.h"

struct uiDateTimePicker {
	uiDarwinControl c;
	NSDatePicker *dp;
	void (*onChanged)(uiDateTimePicker *, void *);
	void *onChangedData;
	BOOL blockSendOnce;
};

// TODO see if target-action works here or not; I forgot what cody271@ originally said
// the primary advantage of the delegate is the ability to reject changes, but libui doesn't support that yet — we should consider that API option as well
@interface uiprivDatePicker : NSDatePicker<NSDatePickerCellDelegate> {
	uiDateTimePicker *picker;
}
- (id)initWithElements:(NSDatePickerElementFlags)elements uiDateTimePicker:(uiDateTimePicker *)d;
- (void)datePickerCell:(NSDatePickerCell *)aDatePickerCell validateProposedDateValue:(NSDate **)proposedDateValue timeInterval:(NSTimeInterval *)proposedTimeInterval;
- (void)doTimer:(NSTimer *)timer;
@end

@implementation uiprivDatePicker

- (id)initWithElements:(NSDatePickerElementFlags)elements uiDateTimePicker:(uiDateTimePicker *)d
{
	self = [super initWithFrame:NSZeroRect];
	if (self) {
		self->picker = d;

		[self setDateValue:[NSDate date]];
		[self setBordered:NO];
		[self setBezeled:YES];
		[self setDrawsBackground:YES];
		[self setDatePickerStyle:NSTextFieldAndStepperDatePickerStyle];
		[self setDatePickerElements:elements];
		[self setDatePickerMode:NSSingleDateMode];

		[self setDelegate:self];
	}
	return self;
}

- (void)datePickerCell:(NSDatePickerCell *)cell validateProposedDateValue:(NSDate **)proposedDateValue timeInterval:(NSTimeInterval *)proposedTimeInterval
{
	[NSTimer scheduledTimerWithTimeInterval:0
		target:self
		selector:@selector(doTimer:)
		userInfo:nil
		repeats:NO];
}

- (void)doTimer:(NSTimer *)timer
{
	uiDateTimePicker *d = self->picker;

	if (d->blockSendOnce) {
		d->blockSendOnce = NO;
		return;
	}
	(*(d->onChanged))(d, d->onChangedData);
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiDateTimePicker, dp)

static void uiDateTimePickerDestroy(uiControl *c)
{
	uiDateTimePicker *d = uiDateTimePicker(c);

	[d->dp release];
	uiFreeControl(uiControl(d));
}

static void defaultOnChanged(uiDateTimePicker *d, void *data)
{
	// do nothing
}

// TODO consider using NSDateComponents iff we ever need the extra accuracy of not using NSTimeInterval
void uiDateTimePickerTime(uiDateTimePicker *d, struct tm *time)
{
	time_t t;
	struct tm tmbuf;
	NSDate *date;

	date = [d->dp dateValue];
	t = (time_t) [date timeIntervalSince1970];

	// Copy time to minimize a race condition
	// time.h functions use global non-thread-safe data
	tmbuf = *localtime(&t);
	memcpy(time, &tmbuf, sizeof (struct tm));
}

void uiDateTimePickerSetTime(uiDateTimePicker *d, const struct tm *time)
{
	time_t t;
	struct tm tmbuf;

	// Copy time because mktime() modifies its argument
	memcpy(&tmbuf, time, sizeof (struct tm));
	t = mktime(&tmbuf);

	// TODO get rid of the need for this
	d->blockSendOnce = YES;
	[d->dp setDateValue:[NSDate dateWithTimeIntervalSince1970:t]];
}

void uiDateTimePickerOnChanged(uiDateTimePicker *d, void (*f)(uiDateTimePicker *, void *), void *data)
{
	d->onChanged = f;
	d->onChangedData = data;
}

static uiDateTimePicker *finishNewDateTimePicker(NSDatePickerElementFlags elements)
{
	uiDateTimePicker *d;

	uiDarwinNewControl(uiDateTimePicker, d);

	d->dp = [[uiprivDatePicker alloc] initWithElements:elements uiDateTimePicker:d];
	uiDarwinSetControlFont(d->dp, NSRegularControlSize);

	uiDateTimePickerOnChanged(d, defaultOnChanged, NULL);

	return d;
}

uiDateTimePicker *uiNewDateTimePicker(void)
{
	return finishNewDateTimePicker(NSYearMonthDayDatePickerElementFlag | NSHourMinuteSecondDatePickerElementFlag);
}

uiDateTimePicker *uiNewDatePicker(void)
{
	return finishNewDateTimePicker(NSYearMonthDayDatePickerElementFlag);
}

uiDateTimePicker *uiNewTimePicker(void)
{
	return finishNewDateTimePicker(NSHourMinuteSecondDatePickerElementFlag);
}
