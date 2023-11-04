#ifndef __LIBUI_QA_H__
#define __LIBUI_QA_H__

#include "../../ui.h"

#define QA_WINDOW_WIDTH 960
#define QA_WINDOW_HEIGHT 720

uiControl* qaMakeGuide(uiControl *c, const char *text);

#define QA_DECLARE_TEST(name) uiControl* name(); const char *name##Guide()

QA_DECLARE_TEST(buttonOnClicked);

QA_DECLARE_TEST(checkboxOnToggled);

QA_DECLARE_TEST(entryOnChanged);
QA_DECLARE_TEST(passwordEntryOnChanged);
QA_DECLARE_TEST(searchEntryOnChanged);
QA_DECLARE_TEST(entryLongText);
QA_DECLARE_TEST(entryOverflowText);

QA_DECLARE_TEST(labelMultiLine);


QA_DECLARE_TEST(windowFullscreen);
QA_DECLARE_TEST(windowBorderless);
QA_DECLARE_TEST(windowResizeable);
QA_DECLARE_TEST(windowFullscreenBorderless);
QA_DECLARE_TEST(windowFullscreenResizeable);
QA_DECLARE_TEST(windowResizeableBorderless);

#endif

