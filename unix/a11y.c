#include "uipriv_unix.h"

static int uiUnixA11yDoControlAction(uiControl *c, const char *name)
{
	GtkWidget *widget;
	AtkObject *a11y;
	AtkAction *action;
	int i;

	widget = GTK_WIDGET(uiControlHandle(c));
	a11y = gtk_widget_get_accessible(widget);
	action = ATK_ACTION(a11y);
	for (i = 0; i < atk_action_get_n_actions(action); i++) {
		if (strcmp(atk_action_get_name(action, i), name) == 0) {
			if (atk_action_do_action(action, i))
				return TRUE;
		}
	}
	return FALSE;
}

int uiA11yDoButtonClick(uiButton *b)
{
	return uiUnixA11yDoControlAction(uiControl(b), "click");
}

