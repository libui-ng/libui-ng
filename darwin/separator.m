// 14 august 2015
#import "uipriv_darwin.h"

// TODO make this intrinsic
#define separatorWidth 96
#define separatorHeight 96

struct uiSeparator {
	uiDarwinControl c;
	NSBox *box;
};

@interface uiprivNSBoxSeparator : NSBox<NSDraggingDestination> {
	uiSeparator *separator;
}
- (id)initWithFrame:(NSRect)frame uiSeparator:(uiSeparator *)s;
@end

@implementation uiprivNSBoxSeparator

uiDarwinDragDestinationMethods(separator)

- (id)initWithFrame:(NSRect)frame uiSeparator:(uiSeparator *)s
{
	self = [super initWithFrame:frame];
	if (self) {
		self->separator = s;

		[self setBoxType:NSBoxSeparator];
		[self setBorderType:NSGrooveBorder];
		[self setTransparent:NO];
		[self setTitlePosition:NSNoTitle];
	}
	return self;
}

@end

uiDarwinControlAllDefaults(uiSeparator, box)

uiSeparator *uiNewHorizontalSeparator(void)
{
	uiSeparator *s;

	uiDarwinNewControl(uiSeparator, s);

	// make the initial width >= initial height to force horizontal
	s->box = [[uiprivNSBoxSeparator alloc] initWithFrame:NSMakeRect(0, 0, 100, 1) uiSeparator:s];

	return s;
}

uiSeparator *uiNewVerticalSeparator(void)
{
	uiSeparator *s;

	uiDarwinNewControl(uiSeparator, s);

	// make the initial height >= initial width to force vertical
	s->box = [[uiprivNSBoxSeparator alloc] initWithFrame:NSMakeRect(0, 0, 1, 100) uiSeparator:s];

	return s;
}
