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
	NSString *action;
	NSEventModifierFlags flags;
	NSDictionary *keyAction = @{
		@"x": @"cut:",
		@"c": @"copy:",
		@"v": @"paste:",
		@"a": @"selectAll:",
		@"z": @"undo:",
		@"Z": @"redo:"
	};

	if ([e type] != NSKeyDown)
		return FALSE;

	flags = [e modifierFlags] & NSDeviceIndependentModifierFlagsMask;
	if (flags != NSCommandKeyMask && flags != (NSCommandKeyMask | NSShiftKeyMask))
		return FALSE;

	action = keyAction[[e charactersIgnoringModifiers]];
	if (action == nil)
		return FALSE;

	return [app sendAction:NSSelectorFromString(action) to:nil from:app];
}
