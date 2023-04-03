#include "unit.h"

#define uiProgressBarPtrFromState(p) uiControlPtrFromState(uiProgressBar, p)

static void progressBarNew(void **state)
{
	uiProgressBar **p = uiProgressBarPtrFromState(state);

	*p = uiNewProgressBar();
}

static void progressBarValueDefault(void **state)
{
	uiProgressBar **p = uiProgressBarPtrFromState(state);

	*p = uiNewProgressBar();
	assert_int_equal(uiProgressBarValue(*p), 0);
}

static void progressBarSetValue(void **state)
{
	uiProgressBar **p = uiProgressBarPtrFromState(state);

	*p = uiNewProgressBar();
	uiProgressBarSetValue(*p, 1);
	assert_int_equal(uiProgressBarValue(*p), 1);
	uiProgressBarSetValue(*p, 100);
	assert_int_equal(uiProgressBarValue(*p), 100);
	uiProgressBarSetValue(*p, 0);
	assert_int_equal(uiProgressBarValue(*p), 0);
}

static void progressBarSetValueIndeterminate(void **state)
{
	uiProgressBar **p = uiProgressBarPtrFromState(state);

	*p = uiNewProgressBar();
	uiProgressBarSetValue(*p, -1);
	assert_int_equal(uiProgressBarValue(*p), -1);
}

#define progressBarUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		unitTestSetup, unitTestTeardown)

int progressBarRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		progressBarUnitTest(progressBarNew),
		progressBarUnitTest(progressBarValueDefault),
		progressBarUnitTest(progressBarSetValue),
		progressBarUnitTest(progressBarSetValueIndeterminate),
	};

	return cmocka_run_group_tests_name("uiProgressBar", tests, unitTestsSetup, unitTestsTeardown);
}

