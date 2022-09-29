#include "../ui.h"
#include "uipriv.h"

#define UI_VERSION "ng-development-version"

const char *uiVersion(void)
{
  static const char *libui_version_str = UI_VERSION;
  return libui_version_str;
}