#import "uipriv_darwin.h"

/**
 * Handle keyboard events related to editing (cut, copy, paste, select all, undo, redo).
 *
 * Special handling of these events is necessary as macOS does not handle these in the
 * absence of an *Edit* menu.
 *
 * @returns `TRUE` if an event was processed, `FALSE` otherwise.
 */
BOOL uiprivSendKeyboardEditEvents(uiprivApplicationClass *app, NSEvent *e)
{
	char key;
	SEL action;
	NSString *chars;
	NSEventModifierFlags flags;

	if ([e type] != NSKeyDown)
		return FALSE;

	flags = [e modifierFlags] & NSDeviceIndependentModifierFlagsMask;
	if (flags != NSCommandKeyMask && flags != (NSCommandKeyMask | NSShiftKeyMask))
		return FALSE;

	chars = [e charactersIgnoringModifiers];
	if ([chars length] != 1)
		return FALSE;

	key = [chars UTF8String][0];
	switch (key) {
		case 'x': action = @selector(cut:);       break;
		case 'c': action = @selector(copy:);      break;
		case 'v': action = @selector(paste:);     break;
		case 'a': action = @selector(selectAll:); break;
		case 'z': action = @selector(undo:);      break;
		case 'Z': action = @selector(redo:);      break;
		default: return FALSE;
	}

	return [app sendAction:action to:nil from:app];
}
