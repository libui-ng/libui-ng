// 26 june 2015
#include "uipriv_unix.h"

// LONGTERM figure out why, and describe, that this is the desired behavior
// LONGTERM also point out that font and color buttons also work like this

#define windowWindow(w) (GTK_WINDOW(uiControlHandle(uiControl(w))))

static char *filedialog(GtkWindow *parent, GtkFileChooserAction mode, const gchar *confirm,
			uiFileDialogParams *params)
{
	GtkWidget *fcd;
	GtkFileChooser *fc;
	gint response;
	char *filename;

	size_t s;

	fcd = gtk_file_chooser_dialog_new(NULL, parent, mode,
		"_Cancel", GTK_RESPONSE_CANCEL,
		confirm, GTK_RESPONSE_ACCEPT,
		NULL);
	fc = GTK_FILE_CHOOSER(fcd);

	if (params != NULL) {
		if (params->filters != NULL) {
			for (s = 0; s < params->filterCount; s++) {
				size_t pattern;

				GtkFileFilter *filter = gtk_file_filter_new();
				gtk_file_filter_set_name(filter, params->filters[s].name);

				// Add all of the patterns for this filter
				for (pattern = 0; pattern < params->filters[s].patternCount; pattern++) {
					gtk_file_filter_add_pattern(filter, params->filters[s].patterns[pattern]);
				}

				gtk_file_chooser_add_filter(fc, filter);
			}
		} else {
			if (params->filterCount != 0) {
				uiprivUserBug("Filter count must be 0 (not %d) if the filters list is NULL.", params->filterCount);
			}
		}
		if (params->defaultPath != NULL && strlen(params->defaultPath) > 0)
			gtk_file_chooser_set_current_folder(fc, params->defaultPath);
		if (params->defaultName != NULL && strlen(params->defaultName) > 0)
			gtk_file_chooser_set_current_name(fc, params->defaultName);
	}

	gtk_file_chooser_set_local_only(fc, FALSE);
	gtk_file_chooser_set_select_multiple(fc, FALSE);
	gtk_file_chooser_set_show_hidden(fc, TRUE);
	gtk_file_chooser_set_do_overwrite_confirmation(fc, TRUE);
	gtk_file_chooser_set_create_folders(fc, TRUE);

	response = gtk_dialog_run(GTK_DIALOG(fcd));
	if (response != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(fcd);
		return NULL;
	}
	filename = gtk_file_chooser_get_filename(fc);
	gtk_widget_destroy(fcd);
	return filename;
}

char *uiOpenFileWithParams(uiWindow *parent, uiFileDialogParams *params)
{
	return filedialog(windowWindow(parent), GTK_FILE_CHOOSER_ACTION_OPEN, "_Open",
			  params);
}

char *uiOpenFolderWithParams(uiWindow *parent, uiFileDialogParams *params)
{
	return filedialog(windowWindow(parent), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Open",
			  params);
}

char *uiSaveFileWithParams(uiWindow *parent, uiFileDialogParams *params)
{
	return filedialog(windowWindow(parent), GTK_FILE_CHOOSER_ACTION_SAVE, "_Save",
			  params);
}

char *uiOpenFile(uiWindow *parent)
{
	return filedialog(windowWindow(parent), GTK_FILE_CHOOSER_ACTION_OPEN, "_Open",
			  NULL);
}

char *uiOpenFolder(uiWindow *parent)
{
	return filedialog(windowWindow(parent), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Open",
			  NULL);
}

char *uiSaveFile(uiWindow *parent)
{
	return filedialog(windowWindow(parent), GTK_FILE_CHOOSER_ACTION_SAVE, "_Save",
			  NULL);
}

static void msgbox(GtkWindow *parent, const char *title, const char *description, GtkMessageType type, GtkButtonsType buttons)
{
	GtkWidget *md;

	md = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
		type, buttons,
		"%s", title);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(md), "%s", description);
	gtk_dialog_run(GTK_DIALOG(md));
	gtk_widget_destroy(md);
}

void uiMsgBox(uiWindow *parent, const char *title, const char *description)
{
	msgbox(windowWindow(parent), title, description, GTK_MESSAGE_OTHER, GTK_BUTTONS_OK);
}

void uiMsgBoxError(uiWindow *parent, const char *title, const char *description)
{
	msgbox(windowWindow(parent), title, description, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK);
}
