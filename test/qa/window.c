#include <limits.h>

#include "qa.h"

static int onClosing(uiWindow *w, void *data)
{
	return 1;
}

static uiWindow* windowNew(uiControl *child, const char* (*guide)(void))
{
	uiWindow *w;
	uiBox *box;
	uiMultilineEntry *g;

	box = uiNewHorizontalBox();

	g = uiNewMultilineEntry();
	uiMultilineEntrySetText(g, guide());
	uiMultilineEntrySetReadOnly(g, 1);

	uiBoxAppend(box, child, 1);
	uiBoxAppend(box, uiControl(g), 1);

	w = uiNewWindow("Quality Assurance", QA_WINDOW_WIDTH, QA_WINDOW_HEIGHT, 0);
	uiWindowOnClosing(w, onClosing, NULL);

	uiWindowSetChild(w, uiControl(box));
	uiControlShow(uiControl(w));

	return w;
}

static void fullscreenOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetFullscreen(w, uiCheckboxChecked(c));
}

const char *windowFullscreenGuide() {
	return
	"1.\tOne checkbox:\n"
	"\t`Fullscreen` unchecked.\n"
	"\n"
	"2.\tCheck `Fullscreen`. Window fills screen.\n"
	"\t`Fullscreen` checked.\n"
	"\n"
	"3.\tUncheck `Fullscreen`. Window returns to initial size.\n"
	"\t`Fullscreen` unchecked.\n"
	"\n"
	"4.\tClose the secondary window.\n"
	;
}

uiControl* windowFullscreen()
{
	uiWindow *w;
	uiCheckbox *c;

	c = uiNewCheckbox("Fullscreen");
	w = windowNew(uiControl(c), windowFullscreenGuide);

	uiCheckboxSetChecked(c, uiWindowFullscreen(w));
	uiCheckboxOnToggled(c, fullscreenOnToggledCb, w);

	return uiControl(uiNewLabel("Use controls in the secondary window."));
}

static void borderlessOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetBorderless(w, uiCheckboxChecked(c));
}

const char *windowBorderlessGuide() {
	return
	"Note: This test may not fully work on some Unix window managers.\n"
	"\n"
	"1.\tOne checkbox:\n"
	"\t`Borderless` unchecked.\n"
	"\n"
	"2.\tCheck `Borderless`. Window borders disappear.\n"
	"\t`Borderless` checked.\n"
	"\n"
	"3.\tUncheck `Borderless`. Window borders reappear.\n"
	"\t`Borderless` unchecked.\n"
	"\n"
	"4.\tClose the secondary window.\n"
	;
}

uiControl* windowBorderless()
{
	uiWindow *w;
	uiCheckbox *c;

	c = uiNewCheckbox("Borderless");
	w = windowNew(uiControl(c), windowBorderlessGuide);

	uiCheckboxSetChecked(c, uiWindowBorderless(w));
	uiCheckboxOnToggled(c, borderlessOnToggledCb, w);

	return uiControl(uiNewLabel("Use controls in the secondary window."));
}

static void resizeableOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetResizeable(w, uiCheckboxChecked(c));
}

const char *windowResizeableGuide() {
	return
	"Note: This test may not fully work on some Unix window managers.\n"
	"\n"
	"1.\tOne checkbox:\n"
	"\t`Resizeable` checked.\n"
	"\tVerify resizeability by dragging the bottom right corner.\n"
	"\n"
	"2.\tUncheck `Resizeable`.\n"
	"\t`Resizeable` unchecked.\n"
	"\tVerify non-resizeability by failing to drag the bottom right corner.\n"
	"\n"
	"3.\tCheck `Resizeable`.\n"
	"\t`Resizeable` checked.\n"
	"\tVerify resizeability by dragging the bottom right corner.\n"
	"\n"
	"4.\tClose the secondary window.\n"
	;
}

uiControl* windowResizeable()
{
	uiWindow *w;
	uiCheckbox *c;

	c = uiNewCheckbox("Resizeable");
	w = windowNew(uiControl(c), windowResizeableGuide);

	uiCheckboxSetChecked(c, uiWindowResizeable(w));
	uiCheckboxOnToggled(c, resizeableOnToggledCb, w);

	return uiControl(uiNewLabel("Use controls in the secondary window."));
}

uiCheckbox *borderless;
uiCheckbox *fullscreen;
uiCheckbox *resizeable;

static void updateFullscreenBorderless(uiWindow *w)
{
	uiCheckboxSetChecked(borderless, uiWindowBorderless(w));
	uiCheckboxSetChecked(fullscreen, uiWindowFullscreen(w));
}

static void fullscreenBorderlessBorderlessOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetBorderless(w, uiCheckboxChecked(c));
	updateFullscreenBorderless(w);
}

static void fullscreenBorderlessFullscreenOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetFullscreen(w, uiCheckboxChecked(c));
	updateFullscreenBorderless(w);
}

const char *windowFullscreenBorderlessGuide() {
	return
	"Note: This test may not fully work on some Unix window managers.\n"
	"\n"
	"1.\tTwo checkboxes:\n"
	"\t`Borderless` unchecked, `Fullscreen` unchecked.\n"
	"\n"
	"2.\tCheck `Borderless`. Window borders disappear.\n"
	"\t`Borderless` checked, `Fullscreen` unchecked.\n"
	"\n"
	"3.\tCheck `Fullscreen`. Window fills screen.\n"
	"\t`Borderless` checked, `Fullscreen` checked.\n"
	"\n"
	"4.\tUncheck `Fullscreen`. Window returns to initial size.\n"
	"\t`Borderless` checked, `Fullscreen` unchecked.\n"
	"\tWindow has no borders.\n"
	"\n"
	"5.\tUncheck `Borderless`. Window borders reappear.\n"
	"\t`Borderless` unchecked, `Fullscreen` unchecked.\n"
	"\n"
	"6.\tCheck `Fullscreen`. Window fills screen.\n"
	"\t`Borderless` unchecked, `Fullscreen` checked.\n"
	"\n"
	"7.\tCheck `Borderless`. Window remains full screen.\n"
	"\t`Borderless` checked, `Fullscreen` checked.\n"
	"\n"
	"8.\tUncheck `Fullscreen`. Window returns to initial size.\n"
	"\t`Borderless` checked, `Fullscreen` unchecked.\n"
	"\tWindow has no borders.\n"
	"\n"
	"9.\tCheck `Fullscreen`. Window fills screen.\n"
	"\t`Borderless` checked, `Fullscreen` checked.\n"
	"\n"
	"10.\tUncheck `Borderless`. Window remains full screen.\n"
	"\t`Borderless` unchecked, `Fullscreen` checked.\n"
	"\n"
	"11.\tUncheck `Fullscreen`. Window returns to initial size.\n"
	"\t`Borderless` unchecked, `Fullscreen` unchecked.\n"
	"\tWindow has borders.\n"
	"\n"
	"12.\tClose the secondary window.\n"
	;
}

uiControl* windowFullscreenBorderless()
{
	uiWindow *w;
	uiBox *vbox;

	vbox = uiNewVerticalBox();
	w = windowNew(uiControl(vbox), windowFullscreenBorderlessGuide);

	borderless = uiNewCheckbox("Borderless");
	uiBoxAppend(vbox, uiControl(borderless), 0);
	fullscreen = uiNewCheckbox("Fullscreen");
	uiBoxAppend(vbox, uiControl(fullscreen), 0);

	uiCheckboxOnToggled(borderless, fullscreenBorderlessBorderlessOnToggledCb, w);
	uiCheckboxOnToggled(fullscreen, fullscreenBorderlessFullscreenOnToggledCb, w);
	updateFullscreenBorderless(w);

	return uiControl(uiNewLabel("Use controls in the secondary window."));
}

static void updateFullscreenResizeable(uiWindow *w)
{
	uiCheckboxSetChecked(fullscreen, uiWindowFullscreen(w));
	uiCheckboxSetChecked(resizeable, uiWindowResizeable(w));
}

static void fullscreenResizeableFullscreenOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetFullscreen(w, uiCheckboxChecked(c));
	updateFullscreenResizeable(w);
}

static void fullscreenResizeableResizeableOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetResizeable(w, uiCheckboxChecked(c));
	updateFullscreenResizeable(w);
}

const char *windowFullscreenResizeableGuide() {
	return
	"Note: This test may not fully work on some Unix window managers.\n"
	"\n"
	"1.\tTwo checkboxes:\n"
	"\t`Resizeable` checked, `Fullscreen` unchecked.\n"
	"\n"
	"2.\tCheck `Fullscreen`. Window fills screen.\n"
	"\t`Resizeable` checked, `Fullscreen` checked.\n"
	"\n"
	"3.\tUncheck `Fullscreen`. Window returns to initial size.\n"
	"\t`Resizeable` checked, `Fullscreen` unchecked.\n"
	"\tVerify resizeability by dragging the bottom right corner.\n"
	"\n"
	"4.\tCheck `Fullscreen`. Window fills screen.\n"
	"\t`Resizeable` checked, `Fullscreen` checked.\n"
	"\n"
	"5.\tUncheck `Resizeable`. Window remains full screen.\n"
	"\t`Resizeable` unchecked, `Fullscreen` checked.\n"
	"\n"
	"6.\tUncheck `Fullscreen`. Window returns to initial size.\n"
	"\t`Resizeable` unchecked, `Fullscreen` unchecked.\n"
	"\tVerify non-resizeability by failing to drag the bottom right corner.\n"
	"\n"
	"7.\tCheck `Fullscreen`. Window fills screen.\n"
	"\t`Resizeable` unchecked, `Fullscreen` checked.\n"
	"\n"
	"8.\tUncheck `Fullscreen`. Window returns to initial size.\n"
	"\t`Resizeable` unchecked, `Fullscreen` unchecked.\n"
	"\tVerify non-resizeability by failing to drag the bottom right corner.\n"
	"\n"
	"9.\tCheck `Fullscreen`. Window fills screen.\n"
	"\t`Resizeable` unchecked, `Fullscreen` checked.\n"
	"\n"
	"10.\tCheck `Resizeable`. Window stays full screen.\n"
	"\t`Resizeable` checked, `Fullscreen` checked.\n"
	"\n"
	"11.\tUncheck `Fullscreen`. Window returns to initial size.\n"
	"\t`Resizeable` checked, `Fullscreen` unchecked.\n"
	"\tVerify resizeability by dragging the bottom right corner.\n"
	"\n"
	"12.\tClose the secondary window.\n"
	;
}

uiControl* windowFullscreenResizeable()
{
	uiWindow *w;
	uiBox *vbox;

	vbox = uiNewVerticalBox();
	w = windowNew(uiControl(vbox), windowFullscreenResizeableGuide);

	resizeable = uiNewCheckbox("Resizeable");
	uiBoxAppend(vbox, uiControl(resizeable), 0);
	fullscreen = uiNewCheckbox("Fullscreen");
	uiBoxAppend(vbox, uiControl(fullscreen), 0);

	uiCheckboxOnToggled(resizeable, fullscreenResizeableResizeableOnToggledCb, w);
	uiCheckboxOnToggled(fullscreen, fullscreenResizeableFullscreenOnToggledCb, w);
	updateFullscreenResizeable(w);

	return uiControl(uiNewLabel("Use controls in the secondary window."));
}

static void updateResizeableBorderless(uiWindow *w)
{
	uiCheckboxSetChecked(resizeable, uiWindowResizeable(w));
	uiCheckboxSetChecked(borderless, uiWindowBorderless(w));
}

static void resizeableBorderlessBorderlessOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetBorderless(w, uiCheckboxChecked(c));
	updateResizeableBorderless(w);
}

static void resizeableBorderlessResizeableOnToggledCb(uiCheckbox *c, void *data)
{
	uiWindow *w = data;

	uiWindowSetResizeable(w, uiCheckboxChecked(c));
	updateResizeableBorderless(w);
}

const char *windowResizeableBorderlessGuide() {
	return
	"Note: This test may not fully work on some Unix window managers.\n"
	"\n"
	"1.\tTwo checkboxes:\n"
	"\t`Resizeable` checked, `Borderless` unchecked.\n"
	"\n"
	"2.\tCheck `Borderless`. Window borders disappear.\n"
	"\t`Resizeable` checked, `Borderless` checked.\n"
	"\n"
	"3.\tUncheck `Borderless`. Window borders reappear.\n"
	"\t`Resizeable` checked, `Borderless` unchecked.\n"
	"\n"
	"4.\tCheck `Borderless`. Window borders disappear.\n"
	"\t`Resizeable` checked, `Borderless` checked.\n"
	"\n"
	"5.\tUncheck `Resizeable`. Window remains the same.\n"
	"\t`Resizeable` unchecked, `Borderless` checked.\n"
	"\n"
	"6.\tUncheck `Borderless`. Window borders reappear.\n"
	"\t`Resizeable` unchecked, `Borderless` unchecked.\n"
	"\n"
	"7.\tCheck `Borderless`. Window borders disappear.\n"
	"\t`Resizeable` unchecked, `Borderless` checked.\n"
	"\n"
	"8.\tUncheck `Borderless`. Window borders reappear.\n"
	"\t`Resizeable` unchecked, `Borderless` unchecked.\n"
	"\n"
	"9.\tCheck `Borderless`. Window borders disappear.\n"
	"\t`Resizeable` unchecked, `Borderless` checked.\n"
	"\n"
	"10.\tCheck `Resizeable`. Window stays the same.\n"
	"\t`Resizeable` checked, `Borderless` checked.\n"
	"\n"
	"11.\tUncheck `Borderless`. Window borders reappear.\n"
	"\t`Resizeable` checked, `Borderless` unchecked.\n"
	"\n"
	"12.\tClose the secondary window.\n"
	;
}

uiControl* windowResizeableBorderless()
{
	uiWindow *w;
	uiBox *vbox;

	vbox = uiNewVerticalBox();
	w = windowNew(uiControl(vbox), windowResizeableBorderlessGuide);

	resizeable = uiNewCheckbox("Resizeable");
	uiBoxAppend(vbox, uiControl(resizeable), 0);
	borderless = uiNewCheckbox("Borderless");
	uiBoxAppend(vbox, uiControl(borderless), 0);

	uiCheckboxOnToggled(resizeable, resizeableBorderlessResizeableOnToggledCb, w);
	uiCheckboxOnToggled(borderless, resizeableBorderlessBorderlessOnToggledCb, w);
	updateResizeableBorderless(w);

	return uiControl(uiNewLabel("Use controls in the secondary window."));
}

