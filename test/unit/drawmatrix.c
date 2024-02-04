#include "unit.h"

#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define EPSILON 0.00001

static int compareDouble(double a, double b, double epsilon)
{
	double diff = ABS(a - b);
	double eps_scale = MAX(1.0, MAX(ABS(a), ABS(b)));
	return diff <= epsilon * eps_scale;
}

// It's not defined in cmocka.h but exists in cmocka.c
void cm_print_error(const char * const format, ...);

// Check if a == b without aborting the test.
static int expectDoubleEqual(double a, double b, int first_error, const char* error_prefix)
{
	// Function only used by assertMatrixEqual() macro below right now.
	const char* func_prefix = first_error ? "(assertMatrixEqual)\n" : "";
	int equal = compareDouble(a, b, EPSILON);
	if (!equal) {
		if (error_prefix == NULL)
			cm_print_error("%s%f != %f\n", func_prefix, a, b);
		else
			cm_print_error("%s%s%f != %f\n", func_prefix, error_prefix, a, b);
	}
	return equal;
}

// Assertion for uiDrawMatrix.
// It works in the same way as the assert_* variants of cmocka.
#define assertMatrixEqual(a, b) \
	_assertMatrixEqual((uiDrawMatrix*)a, (uiDrawMatrix*)b, __FILE__, __LINE__)

static void _assertMatrixEqual(uiDrawMatrix *a, uiDrawMatrix *b, const char * const file, const int line)
{
	int equal = 1;
	equal &= expectDoubleEqual(a->M11, b->M11, equal, "M11: ");
	equal &= expectDoubleEqual(a->M12, b->M12, equal, "M12: ");
	equal &= expectDoubleEqual(a->M21, b->M21, equal, "M21: ");
	equal &= expectDoubleEqual(a->M22, b->M22, equal, "M22: ");
	equal &= expectDoubleEqual(a->M31, b->M31, equal, "M31: ");
	equal &= expectDoubleEqual(a->M32, b->M32, equal, "M32: ");
	if (!equal)
		_fail(file, line);
}

static void drawMatrixIdentity(void **state)
{
	uiDrawMatrix *m = *state;
	uiDrawMatrix expected = {
		1.0, 0.0,
		0.0, 1.0,
		0.0, 0.0
	};

	assertMatrixEqual(&expected, m);
}

static void drawMatrixTranslate(void **state)
{
	uiDrawMatrix *m = *state;
	double tx = 0.5;
	double ty = 0.25;
	uiDrawMatrixTranslate(m, tx, ty);

	uiDrawMatrix expected = {
		1.0, 0.0,
		0.0, 1.0,
		tx, ty
	};

	assertMatrixEqual(&expected, m);
}

static void drawMatrixScale(void **state)
{
	uiDrawMatrix *m = *state;
	double cx = 0.25;
	double cy = 0.125;
	double sx = 0.5;
	double sy = 0.25;
	uiDrawMatrixScale(m, cx, cy, sx, sy);

	uiDrawMatrix expected = {
		sx, 0.0,
		0.0, sy,
		-cx * sx + cx,
		-cy * sy + cy
	};

	assertMatrixEqual(&expected, m);
}

#define THETA uiPi / 6
#define COS 1.73205080757 / 2
#define SIN 0.5

static void drawMatrixRotate(void **state)
{
	uiDrawMatrix *m = *state;
	double cx = 0.25;
	double cy = 0.125;
	uiDrawMatrixRotate(m, cx, cy, THETA);

	uiDrawMatrix expected = {
		COS, SIN,
		-SIN, COS,
		-cx * COS + cy * SIN + cx,
		-cx * SIN - cy * COS + cy
	};

	assertMatrixEqual(&expected, m);
}

static void drawMatrixTRS(void **state)
{
	uiDrawMatrix *m = *state;

	double tx = 0.5;
	double ty = 0.25;
	double sx = 0.3;
	double sy = 0.1;
	uiDrawMatrixTranslate(m, tx, ty);
	uiDrawMatrixRotate(m, tx, ty, THETA);
	uiDrawMatrixScale(m, tx, ty, sx, sy);

	uiDrawMatrix expected = {
		COS * sx, SIN * sy,
		-SIN * sx, COS * sy,
		tx, ty,
	};

	assertMatrixEqual(&expected, m);
}

static void drawMatrixMultiply(void **state)
{
	// Test the same transform as drawMatrixTRS with uiDrawMatrixMultiply
	uiDrawMatrix *m = *state;

	uiDrawMatrix t;
	double tx = 0.5;
	double ty = 0.25;
	uiDrawMatrixSetIdentity(&t);
	uiDrawMatrixTranslate(&t, tx, ty);

	uiDrawMatrix r;
	uiDrawMatrixSetIdentity(&r);
	uiDrawMatrixRotate(&r, tx, ty, THETA);

	uiDrawMatrix s;
	double sx = 0.3;
	double sy = 0.1;
	uiDrawMatrixSetIdentity(&s);
	uiDrawMatrixScale(&s, tx, ty, sx, sy);

	uiDrawMatrixMultiply(m, &t);
	uiDrawMatrixMultiply(m, &r);
	uiDrawMatrixMultiply(m, &s);

	uiDrawMatrix expected = {
		COS * sx, SIN * sy,
		-SIN * sx, COS * sy,
		tx, ty,
	};

	assertMatrixEqual(&expected, m);
}

static int drawMatrixTestsSetup(void **state)
{
	*state = malloc(sizeof(uiDrawMatrix));
	assert_non_null(*state);
	return 0;
}

static int drawMatrixTestsTeardown(void **state)
{
	free(*state);
	return 0;
}

int drawMatrixTestSetup(void **state)
{
	uiDrawMatrix *m = *state;
	uiDrawMatrixSetIdentity(m);
	return 0;
}

int drawMatrixTestTeardown(void **state)
{
	return 0;
}

#define drawMatrixUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		drawMatrixTestSetup, drawMatrixTestTeardown)

int drawMatrixRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		drawMatrixUnitTest(drawMatrixIdentity),
		drawMatrixUnitTest(drawMatrixTranslate),
		drawMatrixUnitTest(drawMatrixScale),
		drawMatrixUnitTest(drawMatrixRotate),
		drawMatrixUnitTest(drawMatrixTRS),
		drawMatrixUnitTest(drawMatrixMultiply),
	};

	return cmocka_run_group_tests_name("uiDrawMatrix", tests, drawMatrixTestsSetup, drawMatrixTestsTeardown);
}
