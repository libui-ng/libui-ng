#include <stdio.h>

#include "unit.h"

#define UNIT_TEST_WIDTH 300
#define UNIT_TEST_HEIGHT 200

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

int unitTestsSetup(void **state)
{
	*state = malloc(sizeof(struct state));
	assert_non_null(*state);
	return 0;
}

int unitTestsTeardown(void **state)
{
	free(*state);
	return 0;
}

int unitTestSetup(void **_state)
{
	struct state *state = *_state;
	uiInitOptions o = {0};

	assert_null(uiInit(&o));
	state->w = uiNewWindow("Unit Test", UNIT_TEST_WIDTH, UNIT_TEST_HEIGHT, 0);
	uiWindowOnClosing(state->w, onClosing, NULL);
	return 0;
}

int unitTestTeardown(void **_state)
{
	struct state *state = *_state;

	uiWindowSetChild(state->w, uiControl(state->c));
	uiControlShow(uiControl(state->w));
	//uiMain();
	uiMainSteps();
	uiMainStep(1);
	uiControlDestroy(uiControl(state->w));
	uiUninit();
	return 0;
}

struct unitTest {
	int (*fn)(void);
};

int main(void)
{
	size_t i;
	int failedTests = 0;
	int failedComponents = 0;
	struct unitTest unitTests[] = {
		{ initRunUnitTests },
		{ sliderRunUnitTests },
		{ spinboxRunUnitTests },
		{ labelRunUnitTests },
		{ buttonRunUnitTests },
		{ comboboxRunUnitTests },
		{ checkboxRunUnitTests },
		{ radioButtonsRunUnitTests },
	};

	for (i = 0; i < sizeof(unitTests)/sizeof(*unitTests); ++i) {
		int fails = (unitTests[i].fn)();
		failedTests += fails;
		if (fails > 0)
			failedComponents++;
	}

	puts("[==========]");
	if (failedTests == 0)
		puts("[  PASSED  ] All test(s) in all component(s).");
	else
		printf("[  FAILED  ] %d test(s) in %d component(s), see above.\n",
			       failedTests, failedComponents);

	return failedTests;
}

