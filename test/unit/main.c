#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "../../ui.h"

#define UNIT_TEST_WIDTH 300
#define UNIT_TEST_HEIGHT 200

struct state {
	uiWindow *w;
	uiSlider *s;
};

static int onClosing(uiWindow *w, void *data)
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

static int unitTestsSetup(void **state)
{
	*state = malloc(sizeof(struct state));
	assert_non_null(*state);
	return 0;
}

static int unitTestsTeardown(void **state)
{
	free(*state);
	return 0;
}

static int unitTestSetup(void **_state)
{
	struct state *state = *_state;
	uiInitOptions o = {0};

	assert_null(uiInit(&o));
	state->w = uiNewWindow("Unit Test", UNIT_TEST_WIDTH, UNIT_TEST_HEIGHT, 0);
	uiWindowOnClosing(state->w, onClosing, NULL);
	return 0;
}

static int unitTestTeardown(void **_state)
{
	struct state *state = *_state;

	uiWindowSetChild(state->w, uiControl(state->s));
	uiControlShow(uiControl(state->w));
	//uiMain();
	uiMainSteps();
	uiMainStep(1);
	uiControlDestroy(uiControl(state->w));
	uiUninit();
	return 0;
}

static int sliderUnitTestSetup(void **state)
{
	return unitTestSetup(state);
}

static int sliderUnitTestTeardown(void **state)
{
	return unitTestTeardown(state);
}

static void sliderNew(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
}

static void sliderValueDefaultMin0(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	assert_int_equal(uiSliderValue(*s), 0);
}

static void sliderValueDefaultMin1(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(1, 2);
	assert_int_equal(uiSliderValue(*s), 1);
}

static void sliderSetValue(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	uiSliderSetValue(*s, 1);
	assert_int_equal(uiSliderValue(*s), 1);
	uiSliderSetValue(*s, 0);
	assert_int_equal(uiSliderValue(*s), 0);
}

static void sliderSetValueOutOfRangeClampLow(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	uiSliderSetValue(*s, -1);
	assert_int_equal(uiSliderValue(*s), 0);
}

static void sliderSetValueOutOfRangeClampHigh(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	uiSliderSetValue(*s, 2);
	assert_int_equal(uiSliderValue(*s), 1);
}

static void sliderHasToolTipDefaultTrue(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	assert_int_equal(uiSliderHasToolTip(*s), 1);
}

static void sliderSetHasToolTip(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	uiSliderSetHasToolTip(*s, 0);
	assert_int_equal(uiSliderHasToolTip(*s), 0);
	uiSliderSetHasToolTip(*s, 1);
	assert_int_equal(uiSliderHasToolTip(*s), 1);
}

static void sliderSetRangeLessThanValue(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	uiSliderSetRange(*s, -2, -1);
	assert_int_equal(uiSliderValue(*s), -1);
}

static void sliderSetRangeGreaterThanValue(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	uiSliderSetRange(*s, 1, 2);
	assert_int_equal(uiSliderValue(*s), 1);
}

static void onChangedNoCall(uiSlider *s, void *data)
{
	function_called();
}

static void sliderSetValueNoCallback(void **state)
{
	uiSlider **s = &(((struct state *)*state)->s);

	*s = uiNewSlider(0, 1);
	uiSliderOnChanged(*s, onChangedNoCall, NULL);
	//expect_function_calls(onChangedNoCall, 0);
	uiSliderSetValue(*s, 1);
	uiSliderSetValue(*s, 0);
}

#define sliderUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		sliderUnitTestSetup, sliderUnitTestTeardown)

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
		sliderUnitTest(sliderSetValueNoCallback),
	};

	return cmocka_run_group_tests_name("uiSlider", tests, unitTestsSetup, unitTestsTeardown);
}

