// 11 june 2015
#include "uipriv_unix.h"

struct uiSpinbox {
	uiUnixControl c;
	GtkWidget *widget;
	GtkEntry *entry;
	GtkSpinButton *spinButton;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
	gulong onChangedSignal;
	int precision;
};

uiUnixControlAllDefaults(uiSpinbox)

static void onChanged(GtkSpinButton *sb, gpointer data)
{
	uiSpinbox *s = uiSpinbox(data);

	(*(s->onChanged))(s, s->onChangedData);
}

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

int uiSpinboxValue(uiSpinbox *s)
{
	return gtk_spin_button_get_value(s->spinButton);
}

double uiSpinboxValueDouble(uiSpinbox *s)
{
	return gtk_spin_button_get_value(s->spinButton);
}


void uiSpinboxSetValue(uiSpinbox *s, int value)
{
	// we need to inhibit sending of ::value-changed because this WILL send a ::value-changed otherwise
	g_signal_handler_block(s->spinButton, s->onChangedSignal);
	// this clamps for us
	gtk_spin_button_set_value(s->spinButton, (gdouble) value);
	g_signal_handler_unblock(s->spinButton, s->onChangedSignal);
}

void uiSpinboxSetValueDouble(uiSpinbox *s, double value)
{
	if (s->precision == 0) {
		uiprivUserBug("Setting value to double while spinbox is in int mode is not supported.");
		return;
	}
	// we need to inhibit sending of ::value-changed because this WILL send a ::value-changed otherwise
	g_signal_handler_block(s->spinButton, s->onChangedSignal);
	// this clamps for us
	gtk_spin_button_set_value(s->spinButton, (gdouble) value);
	g_signal_handler_unblock(s->spinButton, s->onChangedSignal);
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

uiSpinbox *uiNewSpinbox(int min, int max)
{
	return uiNewSpinboxDouble((double) min, (double) max, 0);
}

uiSpinbox *uiNewSpinboxDouble(double min, double max, int precision)
{
	uiSpinbox *s;
	double temp;
	double step;
	int precision_clamped;

	if (min >= max) {
		temp = min;
		min = max;
		max = temp;
	}

	uiUnixNewControl(uiSpinbox, s);

	s->widget = gtk_spin_button_new_with_range(min, max, 1);
	s->entry = GTK_ENTRY(s->widget);
	s->spinButton = GTK_SPIN_BUTTON(s->widget);

	precision_clamped = fmax(0, fmin(20, precision));
	gtk_spin_button_set_digits(s->spinButton, precision_clamped);
	if (precision_clamped != 0) {
		step = 1.0 / pow(10.0, precision_clamped);
		gtk_spin_button_set_increments(s->spinButton, step, step * 10);
	}
	s->precision = precision_clamped;

	s->onChangedSignal = g_signal_connect(s->spinButton, "value-changed", G_CALLBACK(onChanged), s);
	uiSpinboxOnChanged(s, defaultOnChanged, NULL);

	return s;
}
