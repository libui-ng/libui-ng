#include "unit.h"

#define uiRadioButtonsPtrFromState(s) uiControlPtrFromState(uiRadioButtons, s)

static void radioButtonsNew(void **state)
{
	uiRadioButtons **r = uiRadioButtonsPtrFromState(state);

	*r = uiNewRadioButtons();
}

static void radioButtonsSelectedDefault(void **state)
{
	uiRadioButtons **r = uiRadioButtonsPtrFromState(state);

	*r = uiNewRadioButtons();

	assert_int_equal(uiRadioButtonsSelected(*r), -1);
}

static void radioButtonsAppend(void **state)
{
	uiRadioButtons **r = uiRadioButtonsPtrFromState(state);

	*r = uiNewRadioButtons();

	uiRadioButtonsAppend(*r, "Item 0");
	uiRadioButtonsAppend(*r, "Item 1");

	assert_int_equal(uiRadioButtonsSelected(*r), -1);
}

static void radioButtonsAppendDuplicate(void **state)
{
	uiRadioButtons **r = uiRadioButtonsPtrFromState(state);
	const char *duplicate = "Duplicate";

	*r = uiNewRadioButtons();

	uiRadioButtonsAppend(*r, duplicate);
	uiRadioButtonsAppend(*r, duplicate);

	assert_int_equal(uiRadioButtonsSelected(*r), -1);
}

static void radioButtonsSetSelected(void **state)
{
	uiRadioButtons **r = uiRadioButtonsPtrFromState(state);

	*r = uiNewRadioButtons();
	uiRadioButtonsAppend(*r, "Item 0");
	uiRadioButtonsAppend(*r, "Item 1");

	uiRadioButtonsSetSelected(*r, 1);
	assert_int_equal(uiRadioButtonsSelected(*r), 1);

	uiRadioButtonsSetSelected(*r, 0);
	assert_int_equal(uiRadioButtonsSelected(*r), 0);

	uiRadioButtonsSetSelected(*r, -1);
	assert_int_equal(uiRadioButtonsSelected(*r), -1);
}

static void onSelectedNoCall(uiRadioButtons *s, void *data)
{
	function_called();
}

static void radioButtonsSetSelectedNoCallback(void **state)
{
	uiRadioButtons **r = uiRadioButtonsPtrFromState(state);

	*r = uiNewRadioButtons();
	uiRadioButtonsOnSelected(*r, onSelectedNoCall, NULL);
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onSelectedNoCall, 0);
	uiRadioButtonsAppend(*r, "Item 0");
	uiRadioButtonsAppend(*r, "Item 1");

	uiRadioButtonsSetSelected(*r, 1);
	uiRadioButtonsSetSelected(*r, 0);
	uiRadioButtonsSetSelected(*r, -1);
}

#define radioButtonsUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		unitTestSetup, unitTestTeardown)

int radioButtonsRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		radioButtonsUnitTest(radioButtonsNew),
		radioButtonsUnitTest(radioButtonsSelectedDefault),
		radioButtonsUnitTest(radioButtonsAppend),
		radioButtonsUnitTest(radioButtonsAppendDuplicate),
		radioButtonsUnitTest(radioButtonsSetSelected),
		radioButtonsUnitTest(radioButtonsSetSelectedNoCallback),
	};

	return cmocka_run_group_tests_name("uiRadioButtons", tests, unitTestsSetup, unitTestsTeardown);
}

