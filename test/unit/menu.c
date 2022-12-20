#include "unit.h"

int menuTestSetup(void **state);
int menuTestTeardown(void **state);

static void menuNew(void **state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
}

static void menuNewInitTwice(void **state)
{
	uiMenu *m;

	menuTestSetup(state);
	m = uiNewMenu("Menu 1");
	menuTestTeardown(state);

	menuTestSetup(state);
	m = uiNewMenu("Menu 2");
	menuTestTeardown(state);
}

static void menuNewEmptyString(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("");
}

static void menuAppendItem(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
	uiMenuAppendItem(m, "Item");
}

static void menuAppendItems(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
	uiMenuAppendItem(m, "Item 1");
	uiMenuAppendItem(m, "Item 2");
}

static void menuAppendCheckItem(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
	uiMenuAppendCheckItem(m, "Item");
}

static void menuAppendCheckItems(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
	uiMenuAppendCheckItem(m, "Item 1");
	uiMenuAppendCheckItem(m, "Item 2");
}

static void menuAppendAboutItem(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
	uiMenuAppendAboutItem(m);
}

static void menuAppendPreferencesItem(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
	uiMenuAppendPreferencesItem(m);
}

static void menuAppendQuitItem(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
	uiMenuAppendQuitItem(m);
}

static void menuAppendSeparator(void **_state)
{
	uiMenu *m;

	m = uiNewMenu("Menu");
	uiMenuAppendSeparator(m);
}

int menuTestSetup(void **_state)
{
	uiInitOptions o = {0};

	assert_null(uiInit(&o));
	return 0;
}

int menuTestTeardown(void **_state)
{
	struct state *state = *_state;

	state->w = uiNewWindow("Menu Test", UNIT_TEST_WINDOW_WIDTH, UNIT_TEST_WINDOW_HEIGHT, 1);
	uiWindowOnClosing(state->w, unitWindowOnClosingQuit, NULL);

	uiControlShow(uiControl(state->w));
	uiMainSteps();
	uiMainStep(1);
	uiControlDestroy(uiControl(state->w));
	uiUninit();
	return 0;
}

#define menuUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		menuTestSetup, menuTestTeardown)

int menuRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		menuUnitTest(menuNew),
		cmocka_unit_test(menuNewInitTwice),
		menuUnitTest(menuNewEmptyString),
		menuUnitTest(menuAppendItem),
		menuUnitTest(menuAppendItems),
		menuUnitTest(menuAppendCheckItem),
		menuUnitTest(menuAppendCheckItems),
		menuUnitTest(menuAppendAboutItem),
		menuUnitTest(menuAppendPreferencesItem),
		menuUnitTest(menuAppendQuitItem),
		menuUnitTest(menuAppendSeparator),
	};

	return cmocka_run_group_tests_name("uiMenu", tests, unitTestsSetup, unitTestsTeardown);
}

