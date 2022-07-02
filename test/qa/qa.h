#ifndef __LIBUI_QA_H__
#define __LIBUI_QA_H__

#include "../../ui.h"

uiControl* qaMakeGuide(uiControl *c, const char *text);

#define QA_DECLARE_TEST(name) uiControl* name(); const char *name##Guide()

QA_DECLARE_TEST(labelMultiLine);

#endif

