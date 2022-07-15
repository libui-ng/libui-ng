#include "unit.h"

#define uiSpinboxPtrFromState(s) uiControlPtrFromState(uiSpinbox, s)

static void spinboxNew(void **state)
{
	uiSpinbox **s = uiSpinboxPtrFromState(state);

	*s = uiNewSpinbox(0, 1);
}

static void spinboxValueDefaultMin0(void **state)
{
	uiSpinbox **s = uiSpinboxPtrFromState(state);

	*s = uiNewSpinbox(0, 1);
	assert_int_equal(uiSpinboxValue(*s), 0);
}

static void spinboxValueDefaultMin1(void **state)
{
	uiSpinbox **s = uiSpinboxPtrFromState(state);

	*s = uiNewSpinbox(1, 2);
	assert_int_equal(uiSpinboxValue(*s), 1);
}

static void spinboxSetValue(void **state)
{
	uiSpinbox **s = uiSpinboxPtrFromState(state);

	*s = uiNewSpinbox(0, 2);
	uiSpinboxSetValue(*s, 1);
	assert_int_equal(uiSpinboxValue(*s), 1);
	uiSpinboxSetValue(*s, 2);
	assert_int_equal(uiSpinboxValue(*s), 2);
	uiSpinboxSetValue(*s, 0);
	assert_int_equal(uiSpinboxValue(*s), 0);
}

static void spinboxSetValueOutOfRangeClampLow(void **state)
{
	uiSpinbox **s = uiSpinboxPtrFromState(state);

	*s = uiNewSpinbox(0, 1);
	uiSpinboxSetValue(*s, -1);
	assert_int_equal(uiSpinboxValue(*s), 0);
}

static void spinboxSetValueOutOfRangeClampHigh(void **state)
{
	uiSpinbox **s = uiSpinboxPtrFromState(state);

	*s = uiNewSpinbox(0, 1);
	uiSpinboxSetValue(*s, 2);
	assert_int_equal(uiSpinboxValue(*s), 1);
}

static void onChangedNoCall(uiSpinbox *s, void *data)
{
	function_called();
}

static void spinboxSetValueNoCallback(void **state)
{
	uiSpinbox **s = uiSpinboxPtrFromState(state);

	*s = uiNewSpinbox(0, 1);
	uiSpinboxOnChanged(*s, onChangedNoCall, NULL);
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onChangedNoCall, 0);
	uiSpinboxSetValue(*s, 1);
	uiSpinboxSetValue(*s, 0);
}

#define spinboxUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		unitTestSetup, unitTestTeardown)

int spinboxRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		spinboxUnitTest(spinboxNew),
		spinboxUnitTest(spinboxValueDefaultMin0),
		spinboxUnitTest(spinboxValueDefaultMin1),
		spinboxUnitTest(spinboxSetValue),
		spinboxUnitTest(spinboxSetValueOutOfRangeClampLow),
		spinboxUnitTest(spinboxSetValueOutOfRangeClampHigh),
		spinboxUnitTest(spinboxSetValueNoCallback),
	};

	return cmocka_run_group_tests_name("uiSpinbox", tests, unitTestsSetup, unitTestsTeardown);
}

