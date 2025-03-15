#include <stdio.h>
#include <ui.h>

int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

int main(void)
{
	const char *err;
	uiWindow *w;

	err = uiInit(NULL);
	if (err != NULL) {
		fprintf(stderr, "Error initializing libui-ng: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	// Create a new window
	w = uiNewWindow("Window Title", 300, 30, 0);

	uiWindowOnClosing(w, onClosing, NULL);
	uiControlShow(uiControl(w));
	uiMain();
	uiUninit();
	return 0;
}

