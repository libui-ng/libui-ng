#include <stdlib.h> // exit
#include <string.h> // memset
#include <stdio.h> // fprintf
#include "../../ui.h"


int onClosing(uiWindow *w, void *data)
{
    uiQuit();
    return 1;
}

int main(void)
{
    uiInitOptions o;
	const char *err;
    uiWindow *w;

    memset(&o, 0, sizeof (uiInitOptions));
    err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

    // Create a new window
    w = uiNewWindow("Hello, World!", 300, 30, 0);

    uiWindowOnClosing(w, onClosing, NULL);
    uiControlShow(uiControl(w));
    uiMain();
    uiUninit();
    return 0;
}
