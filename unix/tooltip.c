#include "uipriv_unix.h"

void uiControlSetTooltip(uiControl *c, const char *tooltip) {
	if (tooltip == NULL) {
		gtk_widget_set_has_tooltip(GTK_WIDGET(uiControlHandle(c)), FALSE);
	} else {
		gtk_widget_set_tooltip_text(GTK_WIDGET(uiControlHandle(c)), tooltip);
	}
}
