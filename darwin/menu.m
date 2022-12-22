// 28 april 2015
#import "uipriv_darwin.h"

struct uiMenu {
	NSMenu *menu;
	NSMenuItem *item;
};

struct uiMenuItem {
	uiprivMenuItem *item;
	int type;
	BOOL disabled;
	void (*onClicked)(uiMenuItem *, uiWindow *, void *);
	void *onClickedData;
};

enum uiprivMenuItemType {
	typeRegular,
	typeCheckbox,
	typeQuit,
	typePreferences,
	typeAbout,
};

@interface uiprivMenu : NSMenu {
@public
	uiMenu *menu;
}
@end

@implementation uiprivMenu
- (id)initWithTitle:(NSString *)title uiMenu:(uiMenu *)m
{
	self = [super initWithTitle:title];
	if (self) {
		self->menu = m;
	}
	return self;
}
@end

@implementation uiprivMenuItem
- (id)initWithTitle:(NSString *)title uiMenuItem:(uiMenuItem *)i
{
	self = [super initWithTitle:title action:@selector(onClicked:) keyEquivalent:@""];
	if (self) {
		self->item = i;

		[self setTarget:self];
	}
	return self;
}

- (IBAction)onClicked:(id)sender
{
	switch (self->item->type) {
	case typeQuit:
		if (uiprivShouldQuit())
			uiQuit();
		return;
	case typeCheckbox:
		uiMenuItemSetChecked(self->item, !uiMenuItemChecked(self->item));
		// fall through
	default:
		// System menu items that may have no user callback (Preferences/About)
		if (self->item == NULL)
			break;
		// use the key window as the source of the menu event; it's the active window
		(*(self->item->onClicked))(self->item, uiprivWindowFromNSWindow([uiprivNSApp() keyWindow]),
			self->item->onClickedData);
		break;
	}
}

- (void)setUiMenuItem:(uiMenuItem *)i
{
	self->item = i;
}

@end

@implementation uiprivMenuManager

- (id)init
{
	self = [super init];
	if (self) {
		self->hasQuit = NO;
		self->hasPreferences = NO;
		self->hasAbout = NO;
		self->finalized = NO;
	}
	return self;
}

- (BOOL)finalized
{
	return self->finalized;
}

- (void)finalize
{
	self->finalized = YES;
}

- (void)dealloc
{
	uiprivUninitMenus();
	[super dealloc];
}

- (void)register:(enum uiprivMenuItemType)type
{
	switch (type) {
	case typeQuit:
		if (self->hasQuit)
			uiprivUserBug("You can't have multiple Quit menu items in one program.");
		self->hasQuit = YES;
		break;
	case typePreferences:
		if (self->hasPreferences)
			uiprivUserBug("You can't have multiple Preferences menu items in one program.");
		self->hasPreferences = YES;
		break;
	case typeAbout:
		if (self->hasAbout)
			uiprivUserBug("You can't have multiple About menu items in one program.");
		self->hasAbout = YES;
		break;
	}
}

// on OS X there are two ways to handle menu items being enabled or disabled: automatically and manually
// unfortunately, the application menu requires automatic menu handling for the Hide, Hide Others, and Show All items to work correctly
// therefore, we have to handle enabling of the other options ourselves
- (BOOL)validateMenuItem:(NSMenuItem *)item
{
	// disable the special items if they aren't present
	if (item == self.quitItem && !self->hasQuit)
		return NO;
	if (item == self.preferencesItem && !self->hasPreferences)
		return NO;
	if (item == self.aboutItem && !self->hasAbout)
		return NO;
	// then poll the item's enabled/disabled state
	if ([item isKindOfClass:[uiprivMenuItem class]]) {
		uiprivMenuItem *mi = (uiprivMenuItem *)item;
		return !mi->item->disabled;
	}
	return NO;
}

// Cocoa constructs the default application menu by hand for each program; that's what MainMenu.[nx]ib does
- (void)buildApplicationMenu:(NSMenu *)menubar
{
	NSString *appName;
	NSMenuItem *appMenuItem;
	NSMenu *appMenu;
	NSMenuItem *item;
	uiprivMenuItem *pitem;
	NSString *title;
	NSMenu *servicesMenu;

	// note: no need to call setAppleMenu: on this anymore; see https://developer.apple.com/library/mac/releasenotes/AppKit/RN-AppKitOlderNotes/#X10_6Notes
	appName = [[NSProcessInfo processInfo] processName];
	appMenuItem = [[[NSMenuItem alloc] initWithTitle:appName action:NULL keyEquivalent:@""] autorelease];
	appMenu = [[[NSMenu alloc] initWithTitle:appName] autorelease];
	[appMenuItem setSubmenu:appMenu];
	[menubar addItem:appMenuItem];

	// first is About
	title = [@"About " stringByAppendingString:appName];
	pitem = [[[uiprivMenuItem alloc] initWithTitle:title uiMenuItem:NULL] autorelease];
	[appMenu addItem:pitem];
	self.aboutItem = pitem;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Preferences
	pitem = [[[uiprivMenuItem alloc] initWithTitle:@"Preferences\u2026" uiMenuItem:NULL] autorelease];
	[appMenu addItem:pitem];
	self.preferencesItem = pitem;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Services
	item = [[[NSMenuItem alloc] initWithTitle:@"Services" action:NULL keyEquivalent:@""] autorelease];
	servicesMenu = [[[NSMenu alloc] initWithTitle:@"Services"] autorelease];
	[item setSubmenu:servicesMenu];
	[uiprivNSApp() setServicesMenu:servicesMenu];
	[appMenu addItem:item];

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next are the three hiding options
	title = [@"Hide " stringByAppendingString:appName];
	item = [[[NSMenuItem alloc] initWithTitle:title action:@selector(hide:) keyEquivalent:@"h"] autorelease];
	// the .xib file says they go to -1 ("First Responder", which sounds wrong...)
	// to do that, we simply leave the target as nil
	[appMenu addItem:item];
	item = [[[NSMenuItem alloc] initWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"] autorelease];
	[item setKeyEquivalentModifierMask:(NSAlternateKeyMask | NSCommandKeyMask)];
	[appMenu addItem:item];
	item = [[[NSMenuItem alloc] initWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""] autorelease];
	[appMenu addItem:item];

	[appMenu addItem:[NSMenuItem separatorItem]];

	// and finally Quit
	// DON'T use @selector(terminate:) as the action; we handle termination ourselves
	title = [@"Quit " stringByAppendingString:appName];
	pitem = [[[uiprivMenuItem alloc] initWithTitle:title uiMenuItem:NULL] autorelease];
	[appMenu addItem:pitem];
	self.quitItem = pitem;
}

- (NSMenu *)makeMenubar
{
	NSMenu *menubar;

	menubar = [[[NSMenu alloc] initWithTitle:@""] autorelease];
	[self buildApplicationMenu:menubar];
	return menubar;
}

@end

static void defaultOnClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	// do nothing
}

void uiMenuItemEnable(uiMenuItem *item)
{
	item->disabled = NO;
	// we don't need to explicitly update the menus here; they'll be updated the next time they're opened (thanks mikeash in irc.freenode.net/#macdev)
}

void uiMenuItemDisable(uiMenuItem *item)
{
	item->disabled = YES;
}

void uiMenuItemOnClicked(uiMenuItem *item, void (*f)(uiMenuItem *, uiWindow *, void *), void *data)
{
	if (item->type == typeQuit)
		uiprivUserBug("You can't call uiMenuItemOnClicked() on a Quit item; use uiOnShouldQuit() instead.");
	item->onClicked = f;
	item->onClickedData = data;
}

int uiMenuItemChecked(uiMenuItem *item)
{
	return [item->item state] != NSOffState;
}

void uiMenuItemSetChecked(uiMenuItem *item, int checked)
{
	NSInteger state;

	state = NSOffState;
	if ([item->item state] == NSOffState)
		state = NSOnState;
	[item->item setState:state];
}

static uiMenuItem *newItem(uiMenu *m, int type, const char *name)
{
	@autoreleasepool {

	uiMenuItem *item;

	if ([uiprivAppDelegate().menuManager finalized])
		uiprivUserBug("You can't create a new menu item after menus have been finalized.");

	item = uiprivNew(uiMenuItem);

	item->type = type;
	switch (item->type) {
	case typeQuit:
		item->item = uiprivAppDelegate().menuManager.quitItem;
		[item->item setUiMenuItem:item];
		break;
	case typePreferences:
		item->item = uiprivAppDelegate().menuManager.preferencesItem;
		[item->item setUiMenuItem:item];
		break;
	case typeAbout:
		item->item = uiprivAppDelegate().menuManager.aboutItem;
		[item->item setUiMenuItem:item];
		break;
	default:
		item->item = [[uiprivMenuItem alloc] initWithTitle:uiprivToNSString(name) uiMenuItem:item];
		[m->menu addItem:item->item];
		break;
	}

	[uiprivAppDelegate().menuManager register:item->type];
	// typeQuit is handled via uiprivShouldQuit()
	if (item->type != typeQuit)
		uiMenuItemOnClicked(item, defaultOnClicked, NULL);

	return item;

	} // @autoreleasepool
}

uiMenuItem *uiMenuAppendItem(uiMenu *m, const char *name)
{
	return newItem(m, typeRegular, name);
}

uiMenuItem *uiMenuAppendCheckItem(uiMenu *m, const char *name)
{
	return newItem(m, typeCheckbox, name);
}

uiMenuItem *uiMenuAppendQuitItem(uiMenu *m)
{
	return newItem(m, typeQuit, NULL);
}

uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m)
{
	return newItem(m, typePreferences, NULL);
}

uiMenuItem *uiMenuAppendAboutItem(uiMenu *m)
{
	return newItem(m, typeAbout, NULL);
}

void uiMenuAppendSeparator(uiMenu *m)
{
	[m->menu addItem:[NSMenuItem separatorItem]];
}

uiMenu *uiNewMenu(const char *name)
{
	@autoreleasepool {

	uiMenu *m;

	if ([uiprivAppDelegate().menuManager finalized])
		uiprivUserBug("You can't create a new menu after menus have been finalized.");

	m = uiprivNew(uiMenu);

	m->menu = [[uiprivMenu alloc] initWithTitle:uiprivToNSString(name) uiMenu:m];
	// use automatic menu item enabling for all menus for consistency's sake

	m->item = [[NSMenuItem alloc] initWithTitle:uiprivToNSString(name) action:NULL keyEquivalent:@""];
	[m->item setSubmenu:m->menu];

	[[uiprivNSApp() mainMenu] addItem:m->item];

	return m;

	} // @autoreleasepool
}

void uiprivFinalizeMenus(void)
{
	[uiprivAppDelegate().menuManager finalize];
}

void uiprivUninitMenus(void)
{
	NSMenuItem *mi;
	NSMenu *sm;
	NSMenuItem *smi;

	for (mi in [[uiprivNSApp() mainMenu] itemArray]) {
		if ([mi hasSubmenu]) {
			sm = [mi submenu];
			for (smi in [sm itemArray]) {
				if ([smi isKindOfClass:[uiprivMenuItem class]]) {
					uiprivMenuItem *x = (uiprivMenuItem *)smi;
					if (x->item != NULL)
						uiprivFree(x->item);
				}
			}
			if ([sm isKindOfClass:[uiprivMenu class]]) {
				uiprivMenu *x = (uiprivMenu *)sm;
				uiprivFree(x->menu);
			}
		}
	}
}
