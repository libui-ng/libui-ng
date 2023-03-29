#include "unit.h"

#define uiEntryPtrFromState(s) uiControlPtrFromState(uiEntry, s)

static void entryNew(void **state)
{
}

static void entryTextDefault(void **state)
{
	uiEntry **e = uiEntryPtrFromState(state);
	const char *text = "";
	char *rv;

	rv = uiEntryText(*e);
	assert_string_equal(rv, text);
	uiFreeText(rv);
}

static void entrySetText(void **state)
{
	uiEntry **e = uiEntryPtrFromState(state);
	const char *text1 = "Text 1";
	const char *text2 = "Text 2";
	char *rv;

	uiEntrySetText(*e, text1);
	rv = uiEntryText(*e);
	assert_string_equal(rv, text1);
	uiFreeText(rv);
	uiEntrySetText(*e, text2);
	rv = uiEntryText(*e);
	assert_string_equal(rv, text2);
	uiFreeText(rv);
}

static void onChangedNoCall(uiEntry *e, void *data)
{
	function_called();
}

static void entrySetTextNoCallback(void **state)
{

	uiEntry **e = uiEntryPtrFromState(state);

	uiEntryOnChanged(*e, onChangedNoCall, NULL);
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onChangedNoCall, 0);
	uiEntrySetText(*e, "Text 1");
	uiEntrySetText(*e, "Text 2");
}

static void entryReadOnlyDefault(void **state)
{
	uiEntry **e = uiEntryPtrFromState(state);

	assert_int_equal(uiEntryReadOnly(*e), 0);
}

static void entrySetReadOnly(void **state)
{
	uiEntry **e = uiEntryPtrFromState(state);

	uiEntrySetReadOnly(*e, 1);
	assert_int_equal(uiEntryReadOnly(*e), 1);
	uiEntrySetReadOnly(*e, 0);
	assert_int_equal(uiEntryReadOnly(*e), 0);
}

static int entryTestSetup(void **state)
{
	int rv = unitTestSetup(state);
	if (rv != 0)
		return rv;

	uiEntry **e = uiEntryPtrFromState(state);
	*e = uiNewEntry();

	return 0;
}

static int passwordEntryTestSetup(void **state)
{
	int rv = unitTestSetup(state);
	if (rv != 0)
		return rv;

	uiEntry **e = uiEntryPtrFromState(state);
	*e = uiNewPasswordEntry();

	return 0;
}

static int searchEntryTestSetup(void **state)
{
	int rv = unitTestSetup(state);
	if (rv != 0)
		return rv;

	uiEntry **e = uiEntryPtrFromState(state);
	*e = uiNewSearchEntry();

	return 0;
}

#define entryUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		entryTestSetup, unitTestTeardown)

#define passwordEntryUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		passwordEntryTestSetup, unitTestTeardown)

#define searchEntryUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		searchEntryTestSetup, unitTestTeardown)

#define entryUnitTests(f) entryUnitTest(f), passwordEntryUnitTest(f), \
		searchEntryUnitTest(f)

int entryRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		entryUnitTests(entryNew),
		entryUnitTests(entryTextDefault),
		entryUnitTests(entrySetText),
		entryUnitTests(entrySetTextNoCallback),
		entryUnitTests(entryReadOnlyDefault),
		entryUnitTests(entrySetReadOnly),
	};

	return cmocka_run_group_tests_name("uiEntry", tests, unitTestsSetup, unitTestsTeardown);
}

