#include "unit.h"

#define uiCheckboxPtrFromState(s) uiControlPtrFromState(uiCheckbox, s)

static void checkboxNew(void **state)
{
	uiCheckbox **c = uiCheckboxPtrFromState(state);

	*c = uiNewCheckbox("Text");
}

static void checkboxCheckedDefault(void **state)
{
	uiCheckbox **c = uiCheckboxPtrFromState(state);

	*c = uiNewCheckbox("Text");
	assert_int_equal(uiCheckboxChecked(*c), 0);
}

static void checkboxSetChecked(void **state)
{
	uiCheckbox **c = uiCheckboxPtrFromState(state);

	*c = uiNewCheckbox("Text");
	uiCheckboxSetChecked(*c, 1);
	assert_int_equal(uiCheckboxChecked(*c), 1);
	uiCheckboxSetChecked(*c, 0);
	assert_int_equal(uiCheckboxChecked(*c), 0);
}

static void checkboxTextDefault(void **state)
{
	uiCheckbox **c = uiCheckboxPtrFromState(state);
	const char *text = "Text";
	char *rv;

	*c = uiNewCheckbox(text);
	rv = uiCheckboxText(*c);
	assert_string_equal(text, rv);
	uiFreeText(rv);
}

static void checkboxSetText(void **state)
{
	uiCheckbox **c = uiCheckboxPtrFromState(state);
	const char *text = "SetText";
	char *rv;

	*c = uiNewCheckbox("Text");
	uiCheckboxSetText(*c, text);
	rv = uiCheckboxText(*c);
	assert_string_equal(text, rv);
	uiFreeText(rv);
}

static void onToggledNoCall(uiCheckbox *c, void *data)
{
	function_called();
}

static void checkboxSetCheckedNoCallback(void **state)
{
	uiCheckbox **c = uiCheckboxPtrFromState(state);

	*c = uiNewCheckbox("Text");
	uiCheckboxOnToggled(*c, onToggledNoCall, NULL);
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onToggledNoCall, 0);
	uiCheckboxSetChecked(*c, 1);
	uiCheckboxSetChecked(*c, 0);
}

#define checkboxUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		unitTestSetup, unitTestTeardown)

int checkboxRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		checkboxUnitTest(checkboxNew),
		checkboxUnitTest(checkboxCheckedDefault),
		checkboxUnitTest(checkboxSetChecked),
		checkboxUnitTest(checkboxTextDefault),
		checkboxUnitTest(checkboxSetText),
		checkboxUnitTest(checkboxSetCheckedNoCallback),
	};

	return cmocka_run_group_tests_name("uiCheckbox", tests, unitTestsSetup, unitTestsTeardown);
}

