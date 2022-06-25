// 11 june 2015
#include "uipriv_unix.h"

/* Number of digits to represent an n-bit number in decimal: log10(2^n)
 * Simplify, truncate & add 1 for full digits, add 1 for negative numbers:
 *   -> n log10(2) + 2   <=   n * 0.302 + 2
 */
#define MAX_STRLEN_FOR_NBITS_IN_DECIMAL(n) ((int)((n) * 302 / 1000) + 2)

struct uiSlider {
	uiUnixControl c;
	GtkWidget *widget;
	GtkRange *range;
	GtkScale *scale;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
	void (*onReleased)(uiSlider *, void *);
	void *onReleasedData;
	gulong onChangedSignal;
	gchar tooltip[MAX_STRLEN_FOR_NBITS_IN_DECIMAL(sizeof(int) * CHAR_BIT) + 1];
};

uiUnixControlAllDefaults(uiSlider)

static void _uiSliderUpdateToolTip(uiSlider *s)
{
	g_snprintf(s->tooltip, sizeof(s->tooltip)/sizeof(s->tooltip[0]), "%d", uiSliderValue(s));
	gtk_widget_set_tooltip_text(s->widget, s->tooltip);
}

static void onChanged(GtkRange *range, gpointer data)
{
	uiSlider *s = uiSlider(data);

	(*(s->onChanged))(s, s->onChangedData);

	if (uiSliderHasToolTip(s))
		_uiSliderUpdateToolTip(s);
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

static gboolean onReleased(GtkWidget *w, GdkEventButton *event, gpointer data)
{
	uiSlider *s = uiSlider(data);

	(*(s->onReleased))(s, s->onReleasedData);
	return FALSE;
}

static void defaultOnReleased(uiSlider *s, void *data)
{
	// do nothing
}

int uiSliderValue(uiSlider *s)
{
	return gtk_range_get_value(s->range);
}

void uiSliderSetValue(uiSlider *s, int value)
{
	// we need to inhibit sending of ::value-changed because this WILL send a ::value-changed otherwise
	g_signal_handler_block(s->range, s->onChangedSignal);
	gtk_range_set_value(s->range, value);
	g_signal_handler_unblock(s->range, s->onChangedSignal);
}

int uiSliderHasToolTip(uiSlider *s)
{
	return gtk_widget_get_has_tooltip(s->widget);
}

void uiSliderSetHasToolTip(uiSlider *s, int hasToolTip)
{
	gtk_widget_set_has_tooltip(s->widget, hasToolTip);

	if (hasToolTip)
		_uiSliderUpdateToolTip(s);
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

void uiSliderOnReleased(uiSlider *s, void (*f)(uiSlider *, void *), void *data)
{
	s->onReleased = f;
	s->onReleasedData = data;
}

void uiSliderSetRange(uiSlider *s, int min, int max)
{
	int temp;

	if (min >= max) {
		temp = min;
		min = max;
		max = temp;
	}

	// we need to inhibit sending of ::value-changed because this WILL send a ::value-changed otherwise
	g_signal_handler_block(s->range, s->onChangedSignal);
	gtk_range_set_range(s->range, min, max);
	g_signal_handler_unblock(s->range, s->onChangedSignal);
}

uiSlider *uiNewSlider(int min, int max)
{
	uiSlider *s;
	int temp;

	if (min >= max) {
		temp = min;
		min = max;
		max = temp;
	}

	uiUnixNewControl(uiSlider, s);

	s->widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, min, max, 1);
	s->range = GTK_RANGE(s->widget);
	s->scale = GTK_SCALE(s->widget);

	// do not draw value, show tooltip instead
	gtk_scale_set_draw_value(s->scale, 0);
	uiSliderSetHasToolTip(s, 1);

	// ensure integers, just to be safe
	gtk_scale_set_digits(s->scale, 0);

	s->onChangedSignal = g_signal_connect(s->scale, "value-changed", G_CALLBACK(onChanged), s);
	g_signal_connect(s->scale, "button-release-event", G_CALLBACK(onReleased), s);
	uiSliderOnChanged(s, defaultOnChanged, NULL);
	uiSliderOnReleased(s, defaultOnReleased, NULL);

	return s;
}
