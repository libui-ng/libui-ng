#include "unit.h"

#define uiButtonPtrFromState(s) uiControlPtrFromState(uiButton, s)

static void buttonNew(void **state)
{
	uiButton **b = uiButtonPtrFromState(state);

	*b = uiNewButton("Text");
}

static void buttonTextDefault(void **state)
{
	uiButton **b = uiButtonPtrFromState(state);
	const char *text = "Text";
	char *rv;

	*b = uiNewButton(text);
	rv = uiButtonText(*b);
	assert_string_equal(rv, text);
	uiFreeText(rv);
}

static void buttonSetText(void **state)
{
	uiButton **b = uiButtonPtrFromState(state);
	const char *text = "Text";
	const char *setText = "setText";
	char *rv;

	*b = uiNewButton(text);
	uiButtonSetText(*b, setText);
	rv = uiButtonText(*b);
	assert_string_equal(rv, setText);
	uiFreeText(rv);
}

#define buttonUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		unitTestSetup, unitTestTeardown)

int buttonRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		buttonUnitTest(buttonNew),
		buttonUnitTest(buttonTextDefault),
		buttonUnitTest(buttonSetText),
	};

	return cmocka_run_group_tests_name("uiButton", tests, unitTestsSetup, unitTestsTeardown);
}

