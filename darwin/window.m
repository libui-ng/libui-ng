// 15 august 2015
#import "uipriv_darwin.h"

#define defaultStyleMask (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)

struct uiWindow {
	uiDarwinControl c;
	NSWindow *window;
	uiControl *child;
	int margined;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	uiprivSingleChildConstraints constraints;
	void (*onContentSizeChanged)(uiWindow *, void *);
	void *onContentSizeChangedData;
	BOOL suppressSizeChanged;
	void (*onFocusChanged)(uiWindow*, void *);
	void *onFocusChangedData;
	void (*onPositionChanged)(uiWindow*, void *);
	void *onPositionChangedData;
	BOOL suppressPositionChanged;
	BOOL fullscreen;
	BOOL borderless;
	BOOL resizeable;
	int focused;
};

static void updateStyleMask(uiWindow *w);

@implementation uiprivNSWindow

- (void)uiprivDoMove:(NSEvent *)initialEvent
{
	uiprivDoManualMove(self, initialEvent);
}

- (void)uiprivDoResize:(NSEvent *)initialEvent on:(uiWindowResizeEdge)edge
{
	uiprivDoManualResize(self, initialEvent, edge);
}

- (id)initWithWidth:(CGFloat)width height:(CGFloat)height uiWindow:(uiWindow *)w
{
	self = [super initWithContentRect:NSMakeRect(0, 0, width, height)
		styleMask:defaultStyleMask
		backing:NSBackingStoreBuffered
		defer:YES];

	if (self) {
		self->window = w;

		// Do NOT release when closed, we manually do so by calling
		// uiWindowDestroy()
		[self setReleasedWhenClosed:NO];

		[self setDelegate:self];
	}
	return self;
}

- (BOOL)windowShouldClose:(id)sender
{
	uiWindow *w = self->window;

	if ((*(w->onClosing))(w, w->onClosingData))
		uiControlDestroy(uiControl(w));
	return NO;
}

- (void)windowDidResize:(NSNotification *)note
{
	uiWindow *w = self->window;

	if (!w->suppressSizeChanged)
		(*(w->onContentSizeChanged))(w, w->onContentSizeChangedData);
}

- (void)windowDidMove:(NSNotification *)note
{
	uiWindow *w = self->window;

	if (!w->suppressPositionChanged)
		(*(w->onPositionChanged))(w, w->onPositionChangedData);
}

- (void)windowDidEnterFullScreen:(NSNotification *)note
{
	uiWindow *w = self->window;

	w->fullscreen = YES;
	updateStyleMask(w);
}

- (void)windowDidExitFullScreen:(NSNotification *)note
{
	uiWindow *w = self->window;

	w->fullscreen = NO;
	updateStyleMask(w);
}

- (void)windowDidBecomeKey:(NSNotification *)note
{
	uiWindow *w = self->window;

	w->focused = 1;
	(*(w->onFocusChanged))(w, w->onFocusChangedData);
}

- (void)windowDidResignKey:(NSNotification *)note
{
	uiWindow *w = self->window;

	w->focused = 0;
	(*(w->onFocusChanged))(w, w->onFocusChangedData);
}

- (uiWindow *)window
{
	return self->window;
}

@end

static void removeConstraints(uiWindow *w)
{
	NSView *cv;

	cv = [w->window contentView];
	uiprivSingleChildConstraintsRemove(&(w->constraints), cv);
}

static void uiWindowDestroy(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// hide the window
	[w->window orderOut:w->window];
	removeConstraints(w);
	if (w->child != NULL) {
		uiControlSetParent(w->child, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(w->child), nil);
		uiControlDestroy(w->child);
	}
	[w->window release];
	uiFreeControl(uiControl(w));
}

uiDarwinControlDefaultHandle(uiWindow, window)

uiControl *uiWindowParent(uiControl *c)
{
	return NULL;
}

void uiWindowSetParent(uiControl *c, uiControl *parent)
{
	uiUserBugCannotSetParentOnToplevel("uiWindow");
}

static int uiWindowToplevel(uiControl *c)
{
	return 1;
}

static int uiWindowVisible(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	return [w->window isVisible];
}

static void uiWindowShow(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	[w->window makeKeyAndOrderFront:w->window];
}

static void uiWindowHide(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	[w->window orderOut:w->window];
}

uiDarwinControlDefaultEnabled(uiWindow, window)
uiDarwinControlDefaultEnable(uiWindow, window)
uiDarwinControlDefaultDisable(uiWindow, window)

static void uiWindowSyncEnableState(uiDarwinControl *c, int enabled)
{
	uiWindow *w = uiWindow(c);

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(w), enabled))
		return;
	if (w->child != NULL)
		uiDarwinControlSyncEnableState(uiDarwinControl(w->child), enabled);
}

static void uiWindowSetSuperview(uiDarwinControl *c, NSView *superview)
{
	// TODO
}

static void windowRelayout(uiWindow *w)
{
	NSView *childView;
	NSView *contentView;

	removeConstraints(w);
	if (w->child == NULL)
		return;
	childView = (NSView *) uiControlHandle(w->child);
	contentView = [w->window contentView];
	uiprivSingleChildConstraintsEstablish(&(w->constraints),
		contentView, childView,
		uiDarwinControlHugsTrailingEdge(uiDarwinControl(w->child)),
		uiDarwinControlHugsBottom(uiDarwinControl(w->child)),
		w->margined,
		@"uiWindow");
}

uiDarwinControlDefaultHugsTrailingEdge(uiWindow, window)
uiDarwinControlDefaultHugsBottom(uiWindow, window)

static void uiWindowChildEdgeHuggingChanged(uiDarwinControl *c)
{
	uiWindow *w = uiWindow(c);

	windowRelayout(w);
}

// TODO
uiDarwinControlDefaultHuggingPriority(uiWindow, window)
uiDarwinControlDefaultSetHuggingPriority(uiWindow, window)
// end TODO

static void uiWindowChildVisibilityChanged(uiDarwinControl *c)
{
	uiWindow *w = uiWindow(c);

	windowRelayout(w);
}

char *uiWindowTitle(uiWindow *w)
{
	return uiDarwinNSStringToText([w->window title]);
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	[w->window setTitle:uiprivToNSString(title)];
}

void uiWindowPosition(uiWindow *w, int *x, int *y)
{
	NSRect screen;
	NSRect window;
	int screenHeightSansMenu;

	screen = [[w->window screen] visibleFrame];
	// Visible screen (no menu, no dock) + dock height
	screenHeightSansMenu = screen.size.height + screen.origin.y;

	window = [w->window frame];
	*x = window.origin.x;
	*y = screenHeightSansMenu - window.origin.y - window.size.height;
}

void uiWindowSetPosition(uiWindow *w, int x, int y)
{
	NSRect screen;
	int screenHeightSansMenu;

	screen = [[w->window screen] visibleFrame];
	// Visible screen (no menu, no dock) + dock height
	screenHeightSansMenu = screen.size.height + screen.origin.y;

	y = screenHeightSansMenu - y;

	w->suppressPositionChanged = YES;
	[w->window setFrameTopLeftPoint:NSMakePoint(x, y)];
	w->suppressPositionChanged = NO;
}

void uiWindowOnPositionChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onPositionChanged = f;
	w->onPositionChangedData = data;
}

void uiWindowContentSize(uiWindow *w, int *width, int *height)
{
	NSRect r;

	r = [w->window contentRectForFrameRect:[w->window frame]];
	*width = r.size.width;
	*height = r.size.height;
}

void uiWindowSetContentSize(uiWindow *w, int width, int height)
{
	w->suppressSizeChanged = YES;
	[w->window setContentSize:NSMakeSize(width, height)];
	w->suppressSizeChanged = NO;
}

static void updateStyleMask(uiWindow *w)
{
	int mask = NSClosableWindowMask | NSMiniaturizableWindowMask;

	// Set fullscreen
	if (w->fullscreen != uiWindowFullscreen(w)) {
		if (w->fullscreen && w->borderless) {
			// Unset borderless and make resizeable to transition into fullscreen
			[w->window setStyleMask:mask | NSTitledWindowMask | NSResizableWindowMask];
		} else {
			// Make resizeable in all cases to toggle fullscreen
			if (!uiWindowResizeable(w))
				[w->window setStyleMask:[w->window styleMask] | NSResizableWindowMask];
		}

		w->suppressSizeChanged = YES;
		[w->window toggleFullScreen:w->window];
		w->suppressSizeChanged = NO;
	}

	// Ignore borderless and resizeable in fullscreen.
	if (uiWindowFullscreen(w))
		return;

	if (w->resizeable)
		mask |= NSResizableWindowMask;

	if (w->borderless)
		mask = NSBorderlessWindowMask;
	else
		mask |= NSTitledWindowMask;

	[w->window setStyleMask:mask];
}

int uiWindowFullscreen(uiWindow *w)
{
	return (BOOL)([w->window styleMask] & NSFullScreenWindowMask);
}

void uiWindowSetFullscreen(uiWindow *w, int fullscreen)
{
	if (uiWindowFullscreen(w) == (BOOL)fullscreen)
		return;

	w->fullscreen = fullscreen;
	updateStyleMask(w);
}

void uiWindowOnContentSizeChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onContentSizeChanged = f;
	w->onContentSizeChangedData = data;
}

void uiWindowOnFocusChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onFocusChanged = f;
	w->onFocusChangedData = data;
}

int uiWindowFocused(uiWindow *w)
{
	return w->focused;
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

int uiWindowBorderless(uiWindow *w)
{
	return w->borderless;
}

void uiWindowSetBorderless(uiWindow *w, int borderless)
{
	w->borderless = borderless;
	updateStyleMask(w);
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	NSView *childView;

	if (w->child != NULL) {
		childView = (NSView *) uiControlHandle(w->child);
		[childView removeFromSuperview];
		uiControlSetParent(w->child, NULL);
	}
	w->child = child;
	if (w->child != NULL) {
		uiControlSetParent(w->child, uiControl(w));
		childView = (NSView *) uiControlHandle(w->child);
		uiDarwinControlSetSuperview(uiDarwinControl(w->child), [w->window contentView]);
		uiDarwinControlSyncEnableState(uiDarwinControl(w->child), uiControlEnabledToUser(uiControl(w)));
	}
	windowRelayout(w);
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	uiprivSingleChildConstraintsSetMargined(&(w->constraints), w->margined);
}

int uiWindowResizeable(uiWindow *w)
{
	return w->resizeable;
}

void uiWindowSetResizeable(uiWindow *w, int resizeable)
{
	w->resizeable = resizeable;
	updateStyleMask(w);
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void defaultOnPositionContentSizeChanged(uiWindow *w, void *data)
{
	// do nothing
}

static void defaultOnFocusChanged(uiWindow *w, void *data)
{
	// do nothing
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	uiprivFinalizeMenus();

	uiDarwinNewControl(uiWindow, w);

	w->window = [[uiprivNSWindow alloc] initWithWidth:(CGFloat)width
		height:(CGFloat)height
		uiWindow:w];
	uiWindowSetTitle(w, title);
	uiWindowSetResizeable(w, 1);

	uiWindowOnClosing(w, defaultOnClosing, NULL);
	uiWindowOnFocusChanged(w, defaultOnFocusChanged, NULL);
	uiWindowOnContentSizeChanged(w, defaultOnPositionContentSizeChanged, NULL);
	uiWindowOnPositionChanged(w, defaultOnPositionContentSizeChanged, NULL);

	return w;
}

// utility function for menus
uiWindow *uiprivWindowFromNSWindow(NSWindow *w)
{
	if (w == nil)
		return NULL;
	// Apparently we might get called with a "fake" window prior to us creating
	// any window - at least this is what the previous comments here suggested.
	if (![w isKindOfClass:[uiprivNSWindow class]])
		return NULL;

	return [(uiprivNSWindow *)w window];
}
