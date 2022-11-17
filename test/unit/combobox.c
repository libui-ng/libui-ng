#include "unit.h"

#define uiComboboxPtrFromState(s) uiControlPtrFromState(uiCombobox, s)

static void comboboxNew(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
}

static void comboboxNumItemsDefault(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	assert_int_equal(uiComboboxNumItems(*c), 0);
}

static void comboboxSelectedDefault(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	assert_int_equal(uiComboboxSelected(*c), -1);
}

static void comboboxAppend(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);
	const char *item0 = "Item 0";
	const char *item1 = "Item 1";

	*c = uiNewCombobox();

	uiComboboxAppend(*c, item0);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), item0);
	assert_int_equal(uiComboboxNumItems(*c), 1);

	uiComboboxAppend(*c, item1);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), item0);
	//TODO assert_string_equal(uiComboboxAt(*c, 1), item1);
	assert_int_equal(uiComboboxNumItems(*c), 2);

	assert_int_equal(uiComboboxSelected(*c), -1);
}

static void comboboxAppendDuplicate(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);
	const char *duplicate = "Duplicate";

	*c = uiNewCombobox();

	uiComboboxAppend(*c, duplicate);
	uiComboboxAppend(*c, duplicate);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), duplicate);
	//TODO assert_string_equal(uiComboboxAt(*c, 1), duplicate);
	assert_int_equal(uiComboboxNumItems(*c), 2);
}

static void comboboxSetSelected(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	uiComboboxAppend(*c, "Item 0");
	uiComboboxAppend(*c, "Item 1");

	uiComboboxSetSelected(*c, 1);
	assert_int_equal(uiComboboxSelected(*c), 1);

	uiComboboxSetSelected(*c, 0);
	assert_int_equal(uiComboboxSelected(*c), 0);

	uiComboboxSetSelected(*c, -1);
	assert_int_equal(uiComboboxSelected(*c), -1);
}

static void comboboxInsertAt(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);
	const char *item0 = "Item 0";
	const char *item1 = "Item 1";
	const char *item2 = "Item 2";
	const char *item3 = "Item 3";

	*c = uiNewCombobox();

	uiComboboxInsertAt(*c, 0, item0);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), item0);
	assert_int_equal(uiComboboxNumItems(*c), 1);
	assert_int_equal(uiComboboxSelected(*c), -1);

	uiComboboxSetSelected(*c, 0);

	uiComboboxInsertAt(*c, 0, item1);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), item1);
	//TODO assert_string_equal(uiComboboxAt(*c, 1), item0);
	assert_int_equal(uiComboboxNumItems(*c), 2);
	assert_int_equal(uiComboboxSelected(*c), 1);

	uiComboboxInsertAt(*c, 1, item2);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), item1);
	//TODO assert_string_equal(uiComboboxAt(*c, 1), item2);
	//TODO assert_string_equal(uiComboboxAt(*c, 2), item0);
	assert_int_equal(uiComboboxNumItems(*c), 3);
	assert_int_equal(uiComboboxSelected(*c), 2);

	uiComboboxInsertAt(*c, 3, item3);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), item1);
	//TODO assert_string_equal(uiComboboxAt(*c, 1), item2);
	//TODO assert_string_equal(uiComboboxAt(*c, 2), item0);
	//TODO assert_string_equal(uiComboboxAt(*c, 3), item3);
	assert_int_equal(uiComboboxNumItems(*c), 4);
	assert_int_equal(uiComboboxSelected(*c), 2);
}

static void comboboxInsertAtDuplicate(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);
	const char *duplicate = "Duplicate";

	*c = uiNewCombobox();

	uiComboboxInsertAt(*c, 0, duplicate);
	uiComboboxInsertAt(*c, 1, duplicate);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), duplicate);
	//TODO assert_string_equal(uiComboboxAt(*c, 1), duplicate);
	assert_int_equal(uiComboboxNumItems(*c), 2);
}

static void comboboxClearEmpty(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();

	uiComboboxClear(*c);
	assert_int_equal(uiComboboxNumItems(*c), 0);
	assert_int_equal(uiComboboxSelected(*c), -1);

	uiComboboxClear(*c);
	assert_int_equal(uiComboboxNumItems(*c), 0);
	assert_int_equal(uiComboboxSelected(*c), -1);
}

static void comboboxClear(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	uiComboboxAppend(*c, "Item 0");
	uiComboboxAppend(*c, "Item 1");
	assert_int_equal(uiComboboxNumItems(*c), 2);
	uiComboboxSetSelected(*c, 1);

	uiComboboxClear(*c);
	assert_int_equal(uiComboboxNumItems(*c), 0);
	assert_int_equal(uiComboboxSelected(*c), -1);
}

static void comboboxClearAppend(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	uiComboboxAppend(*c, "Item 0");
	uiComboboxAppend(*c, "Item 1");
	assert_int_equal(uiComboboxNumItems(*c), 2);

	uiComboboxClear(*c);
	assert_int_equal(uiComboboxNumItems(*c), 0);
	uiComboboxAppend(*c, "Item 2");
	uiComboboxAppend(*c, "Item 3");
	assert_int_equal(uiComboboxNumItems(*c), 2);
}

static void comboboxDelete(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);
	const char *item0 = "Item 0";
	const char *item1 = "Item 1";
	const char *item2 = "Item 2";

	*c = uiNewCombobox();
	uiComboboxAppend(*c, item0);
	uiComboboxAppend(*c, item1);
	uiComboboxAppend(*c, item2);
	uiComboboxSetSelected(*c, 1);

	uiComboboxDelete(*c, 0);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), item1);
	//TODO assert_string_equal(uiComboboxAt(*c, 1), item2);
	assert_int_equal(uiComboboxNumItems(*c), 2);
	assert_int_equal(uiComboboxSelected(*c), 0);

	uiComboboxDelete(*c, 0);
	//TODO assert_string_equal(uiComboboxAt(*c, 0), item2);
	assert_int_equal(uiComboboxNumItems(*c), 1);
	assert_int_equal(uiComboboxSelected(*c), -1);

	uiComboboxSetSelected(*c, 0);
	uiComboboxDelete(*c, 0);
	assert_int_equal(uiComboboxNumItems(*c), 0);
	assert_int_equal(uiComboboxSelected(*c), -1);
}


static void onChangedNoCall(uiCombobox *c, void *data)
{
	function_called();
}

static void comboboxSetSelectedNoCallback(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	uiComboboxOnSelected(*c, onChangedNoCall, NULL);
	uiComboboxAppend(*c, "Item 0");
	uiComboboxAppend(*c, "Item 1");
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onChangedNoCall, 0);

	uiComboboxSetSelected(*c, 1);
	uiComboboxSetSelected(*c, 0);
}


static void comboboxInsertAtNoCallback(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	uiComboboxOnSelected(*c, onChangedNoCall, NULL);
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onChangedNoCall, 0);

	uiComboboxInsertAt(*c, 0, "Item 0");
	uiComboboxSetSelected(*c, 0);
	uiComboboxInsertAt(*c, 0, "Item 1");
	uiComboboxInsertAt(*c, 1, "Item 2");
}

static void comboboxClearEmptyNoCallback(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	uiComboboxOnSelected(*c, onChangedNoCall, NULL);
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onChangedNoCall, 0);

	uiComboboxClear(*c);
}

static void comboboxClearNoCallback(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	uiComboboxOnSelected(*c, onChangedNoCall, NULL);
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onChangedNoCall, 0);
	uiComboboxAppend(*c, "Item 0");
	uiComboboxAppend(*c, "Item 1");

	uiComboboxClear(*c);
	uiComboboxAppend(*c, "Item 2");
	uiComboboxSetSelected(*c, 0);
	uiComboboxClear(*c);
}

static void comboboxDeleteNoCallback(void **state)
{
	uiCombobox **c = uiComboboxPtrFromState(state);

	*c = uiNewCombobox();
	uiComboboxOnSelected(*c, onChangedNoCall, NULL);
	// FIXME: https://gitlab.com/cmocka/cmocka/-/issues/18
	//expect_function_calls(onChangedNoCall, 0);
	uiComboboxAppend(*c, "Item 0");
	uiComboboxAppend(*c, "Item 1");
	uiComboboxAppend(*c, "Item 2");

	uiComboboxDelete(*c, 0);
	uiComboboxSetSelected(*c, 1);
	uiComboboxDelete(*c, 1);
	uiComboboxDelete(*c, 0);
}

#define comboboxUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		unitTestSetup, unitTestTeardown)

int comboboxRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		comboboxUnitTest(comboboxNew),
		comboboxUnitTest(comboboxNumItemsDefault),
		comboboxUnitTest(comboboxSelectedDefault),
		comboboxUnitTest(comboboxAppend),
		comboboxUnitTest(comboboxAppendDuplicate),
		comboboxUnitTest(comboboxSetSelected),
		comboboxUnitTest(comboboxInsertAt),
		comboboxUnitTest(comboboxInsertAtDuplicate),
		comboboxUnitTest(comboboxClearEmpty),
		comboboxUnitTest(comboboxClear),
		comboboxUnitTest(comboboxClearAppend),
		comboboxUnitTest(comboboxDelete),
		comboboxUnitTest(comboboxSetSelectedNoCallback),
		comboboxUnitTest(comboboxInsertAtNoCallback),
		comboboxUnitTest(comboboxClearNoCallback),
		comboboxUnitTest(comboboxClearEmptyNoCallback),
		comboboxUnitTest(comboboxDeleteNoCallback),
	};

	return cmocka_run_group_tests_name("uiCombobox", tests, unitTestsSetup, unitTestsTeardown);
}

