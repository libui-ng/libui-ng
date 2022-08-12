#include "unit.h"

#define uiLabelPtrFromState(s) uiControlPtrFromState(uiLabel, s)

static void labelNew(void **state)
{
	uiLabel **l = uiLabelPtrFromState(state);

	*l = uiNewLabel("Text");
}

static void labelNewEmptyString(void **state)
{
	uiLabel **l = uiLabelPtrFromState(state);

	*l = uiNewLabel("");
}

static void labelText(void **state)
{
	uiLabel **l = uiLabelPtrFromState(state);
	const char *text = "Text";
	char *rv;

	*l = uiNewLabel(text);
	rv = uiLabelText(*l);
	assert_string_equal(text, rv);
	uiFreeText(rv);
}

static void labelTextEmptyString(void **state)
{
	uiLabel **l = uiLabelPtrFromState(state);
	const char *text = "";
	char *rv;

	*l = uiNewLabel(text);
	rv = uiLabelText(*l);
	assert_string_equal(text, rv);
	uiFreeText(rv);
}

static void labelSetText(void **state)
{
	uiLabel **l = uiLabelPtrFromState(state);
	const char *text = "SetText";
	char *rv;

	*l = uiNewLabel("Text");
	uiLabelSetText(*l, text);
	rv = uiLabelText(*l);
	assert_string_equal(text, rv);
	uiFreeText(rv);
}

static void labelSetTextEmptyString(void **state)
{
	uiLabel **l = uiLabelPtrFromState(state);
	const char *text = "";
	char *rv;

	*l = uiNewLabel("Text");
	uiLabelSetText(*l, text);
	rv = uiLabelText(*l);
	assert_string_equal(text, rv);
	uiFreeText(rv);
}

#define labelUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		unitTestSetup, unitTestTeardown)

int labelRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		labelUnitTest(labelNew),
		labelUnitTest(labelNewEmptyString),
		labelUnitTest(labelText),
		labelUnitTest(labelTextEmptyString),
		labelUnitTest(labelSetText),
		labelUnitTest(labelSetTextEmptyString),
	};

	return cmocka_run_group_tests_name("uiLabel", tests, unitTestsSetup, unitTestsTeardown);
}

