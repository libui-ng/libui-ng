#include <stdlib.h> // exit
#include <string.h> // memset
#include "../../ui.h"


int onClosing(uiWindow *w, void *data)
{
    uiQuit();
    return 1;
}

int main(void)
{
    uiInitOptions o;
    uiWindow *w;

    memset(&o, 0, sizeof (uiInitOptions));
    if (uiInit(&o) != NULL) exit(1);

    // Create a new window
    w = uiNewWindow("Hello, World!", 300, 30, 0);

    uiWindowOnClosing(w, onClosing, NULL);
    uiControlShow(uiControl(w));
    uiMain();
    uiUninit();
    return 0;
}
