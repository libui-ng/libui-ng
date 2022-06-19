#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../../ui.h"

#define UNIT_TEST_WIDTH 300
#define UNIT_TEST_HEIGHT 200

int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

/*
int close(void *data)
{
	uiQuit();
	return 1;
}
*/

static void test_setup(void **state)
{
	uiInitOptions o = {0};
	uiWindow *w;

	assert_null(uiInit(&o));

	w = uiNewWindow("Unit Test", UNIT_TEST_WIDTH, UNIT_TEST_HEIGHT, 0);
	uiWindowOnClosing(w, onClosing, NULL);

	*state = w;
}

static void test_teardown(void **state)
{
	uiWindow *w = *state;

	uiControlShow(uiControl(w));
	//uiMain();
	uiMainSteps();
	uiMainStep(1);
	uiControlDestroy(uiControl(w));
	uiUninit();
}

static int sliderSetup(void **state)
{
	test_setup(state);
	return 0;
}

static int sliderTeardown(void **state)
{
	test_teardown(state);
	return 0;
}

static void sliderNew(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderValueDefaultMin0(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);
	assert_int_equal(uiSliderValue(s), 0);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderValueDefaultMin1(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(1, 2);
	assert_int_equal(uiSliderValue(s), 1);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderSetValue(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);
	uiSliderSetValue(s, 1);
	assert_int_equal(uiSliderValue(s), 1);
	uiSliderSetValue(s, 0);
	assert_int_equal(uiSliderValue(s), 0);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderSetValueOutOfRangeClampLow(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);
	uiSliderSetValue(s, -1);
	assert_int_equal(uiSliderValue(s), 0);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderSetValueOutOfRangeClampHigh(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);
	uiSliderSetValue(s, 2);
	assert_int_equal(uiSliderValue(s), 1);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderHasToolTipDefaultTrue(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);
	assert_int_equal(uiSliderHasToolTip(s), 1);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderSetHasToolTip(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);
	uiSliderSetHasToolTip(s, 0);
	assert_int_equal(uiSliderHasToolTip(s), 0);
	uiSliderSetHasToolTip(s, 1);
	assert_int_equal(uiSliderHasToolTip(s), 1);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderSetRangeLessThanValue(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);
	uiSliderSetRange(s, -2, -1);
	assert_int_equal(uiSliderValue(s), -1);

	uiWindowSetChild(w, uiControl(s));
}

static void sliderSetRangeGreaterThanValue(void **state)
{
	uiWindow *w = *state;

	uiSlider *s = uiNewSlider(0, 1);
	uiSliderSetRange(s, 1, 2);
	assert_int_equal(uiSliderValue(s), 1);

	uiWindowSetChild(w, uiControl(s));
}

#define sliderUnitTest(f) cmocka_unit_test_setup_teardown((f), sliderSetup, sliderTeardown)

int main(void)
{
	const struct CMUnitTest tests[] = {
		sliderUnitTest(sliderNew),
		sliderUnitTest(sliderValueDefaultMin0),
		sliderUnitTest(sliderValueDefaultMin1),
		sliderUnitTest(sliderSetValue),
		sliderUnitTest(sliderSetValueOutOfRangeClampLow),
		sliderUnitTest(sliderSetValueOutOfRangeClampHigh),
		sliderUnitTest(sliderHasToolTipDefaultTrue),
		sliderUnitTest(sliderSetHasToolTip),
		sliderUnitTest(sliderSetRangeLessThanValue),
		sliderUnitTest(sliderSetRangeGreaterThanValue),
	};

	return cmocka_run_group_tests_name("uiSlider", tests, NULL, NULL);
}

