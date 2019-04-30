// 23 april 2019
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER			0x0600
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <process.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testing.h"

struct testingTimer {
	LARGE_INTEGER start;
	LARGE_INTEGER end;
};

testingTimer *testingNewTimer(void)
{
	testingTimer *t;

	t = (testingTimer *) malloc(sizeof (testingTimer));
	// TODO handle failure
	memset(t, 0, sizeof (testingTimer));
	return t;
}

void testingFreeTimer(testingTimer *t)
{
	free(t);
}

void testingTimerStart(testingTimer *t)
{
	QueryPerformanceCounter(&(t->start));
}

void testingTimerEnd(testingTimer *t)
{
	QueryPerformanceCounter(&(t->end));
}

int64_t testingTimerNsec(testingTimer *t)
{
	LARGE_INTEGER qpf;
	int64_t qpnsQuot, qpnsRem;
	int64_t c;
	int64_t ret;

	QueryPerformanceFrequency(&qpf);
	qpnsQuot = testingNsecPerSec / qpf.QuadPart;
	qpnsRem = testingNsecPerSec % qpf.QuadPart;
	c = t->end.QuadPart - t->start.QuadPart;

	ret = c * qpnsQuot;
	ret += (c * qpnsRem) / qpf.QuadPart;
	return ret;
}

// note: the idea for the SetThreadContext() nuttery is from https://www.codeproject.com/Articles/71529/Exception-Injection-Throwing-an-Exception-in-Other

static jmp_buf timeout_ret;

static void onTimeout(void)
{
	longjmp(timeout_ret, 1);
}

static HANDLE timeout_timer;
static HANDLE timeout_finished;
static HANDLE timeout_targetThread;
static HRESULT timeout_hr;

static void setContextForGet(CONTEXT *ctx)
{
	ZeroMemory(ctx, sizeof (CONTEXT));
	ctx->ContextFlags = CONTEXT_CONTROL;
}

static void setContextForSet(CONTEXT *ctx)
{
#if defined(_AMD64_)
	ctx->Rip = (DWORD64) onTimeout;
#elif defined(_ARM_)
	ctx->Pc = (DWORD) onTimeout;
#elif defined(_ARM64_)
	ctx->Pc = (DWORD64) onTimeout;
#elif defined(_X86_)
	ctx->Eip = (DWORD) onTimeout;
#elif defined(_IA64_)
	// TODO verify that this is correct
	ctx->StIIP = (ULONGLONG) onTimeout;
#else
#error unknown CPU architecture; cannot create CONTEXT objects for CPU-specific Windows test code
#endif
}

static unsigned __stdcall timerThreadProc(void *data)
{
	HANDLE objects[2];
	CONTEXT ctx;
	DWORD ret;
	DWORD lastError;

	objects[0] = timeout_timer;
	objects[1] = timeout_finished;
	timeout_hr = S_OK;
	SetLastError(0);
	ret = WaitForMultipleObjectsEx(2, objects,
		FALSE, INFINITE, FALSE);
	if (ret == WAIT_FAILED) {
		lastError = GetLastError();
		timeout_hr = E_FAIL;
		if (lastError != 0)
			timeout_hr = HRESULT_FROM_WIN32(lastError);
		ret = WAIT_OBJECT_0;
	}
	if (ret == WAIT_OBJECT_0 + 1)
		// we succeeded; do nothing
		return 0;

	// we timed out (or there was an error); signal it
	// TODO check errors
	SuspendThread(timeout_targetThread);
	setContextForGet(&ctx);
	GetThreadContext(timeout_targetThread, &ctx);
	setContextForSet(&ctx);
	SetThreadContext(timeout_targetThread, &ctx);
	// and force the thread to return from GetMessage(), if we are indeed in that
	PostThreadMessage(GetThreadId(timeout_targetThread), WM_NULL, 0, 0);
	ResumeThread(timeout_targetThread);
	return 0;
}

void testingprivRunWithTimeout(testingT *t, const char *file, long line, int64_t timeout, void (*f)(testingT *t, void *data), void *data, const char *comment, int failNowOnError)
{
	char *timeoutstr;
	int closeTargetThread = 0;
	uintptr_t timerThread = 0;
	LARGE_INTEGER timer;
	BOOL ret;
	DWORD lastError;
	HRESULT hr;

	timeoutstr = testingNsecString(timeout);

	SetLastError(0);
	ret = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &timeout_targetThread,
		0, FALSE, DUPLICATE_SAME_ACCESS);
	if (ret == 0) {
		lastError = GetLastError();
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error getting current thread for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}
	closeTargetThread = 1;

	SetLastError(0);
	timeout_timer = CreateWaitableTimerW(NULL, TRUE, NULL);
	if (timeout_timer == NULL) {
		lastError = GetLastError();
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error creating timer for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	SetLastError(0);
	timeout_finished = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (timeout_finished == NULL) {
		lastError = GetLastError();
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error creating finished event for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	SetLastError(0);
	timer.QuadPart = timeout / 100;
	timer.QuadPart = -timer.QuadPart;
	ret = SetWaitableTimer(timeout_timer, &timer, 0, NULL, NULL, FALSE);
	if (ret == 0) {
		lastError = (DWORD) _doserrno;
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error applying %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	// _doserrno is the equivalent of GetLastError(), or at least that's how _beginthreadex() uses it.
	_doserrno = 0;
	// don't start the thread until after we call setjmp()
	timerThread = _beginthreadex(NULL, 0, timerThreadProc, NULL, CREATE_SUSPENDED, NULL);
	if (timerThread == 0) {
		lastError = (DWORD) _doserrno;
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error creating timer thread for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	if (setjmp(timeout_ret) == 0) {
		// TODO check error
		ResumeThread((HANDLE) timerThread);
		(*f)(t, data);
		failNowOnError = 0;		// we succeeded
	} else if (timeout_hr == S_OK) {
		testingprivTLogfFull(t, file, line, "%s timeout passed (%s)", comment, timeoutstr);
		testingTFail(t);
	} else {
		testingprivTLogfFull(t, file, line, "error running timer thread for %s timeout: 0x%08I32X", comment, timeout_hr);
		testingTFail(t);
	}

out:
	if (timerThread != 0) {
		// TODO check errors
		SetEvent(timeout_finished);
		WaitForSingleObject((HANDLE) timerThread, INFINITE);
		// TODO end check errors
		CloseHandle((HANDLE) timerThread);
	}
	if (timeout_finished != NULL) {
		CloseHandle(timeout_finished);
		timeout_finished = NULL;
	}
	if (timeout_timer != NULL) {
		CloseHandle(timeout_timer);
		timeout_timer = NULL;
	}
	if (closeTargetThread)
		CloseHandle(timeout_targetThread);
	timeout_targetThread = NULL;
	testingFreeNsecString(timeoutstr);
	if (failNowOnError)
		testingTFailNow(t);
}

void testingSleep(int64_t nsec)
{
	HANDLE timer;
	LARGE_INTEGER duration;

	// TODO check errors, possibly falling back to Sleep() (although it has lower resolution)
	// TODO rename all the other durations that are timeout or timer to duration or nsec, both here and in the Unix/Darwin code
	duration.QuadPart = nsec / 100;
	duration.QuadPart = -duration.QuadPart;
	timer = CreateWaitableTimerW(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &duration, 0, NULL, NULL, FALSE);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

struct testingThread {
	uintptr_t handle;
	void (*f)(void *data);
	void *data;
};

static unsigned __stdcall threadThreadProc(void *data)
{
	testingThread *t = (testingThread *) data;

	(*(t->f))(t->data);
	return 0;
}

testingThread *testingNewThread(void (*f)(void *data), void *data)
{
	testingThread *t;

	t = malloc(sizeof (testingThread));
	// TODO check error
	ZeroMemory(t, sizeof (testingThread));
	t->f = f;
	t->data = data;

	t->handle = _beginthreadex(NULL, 0, threadThreadProc, t, 0, NULL);
	// TODO check error
	return t;
}

void testingThreadWaitAndFree(testingThread *t)
{
	// TODO check errors
	WaitForSingleObject((HANDLE) (t->handle), INFINITE);
	// TODO end check errors
	CloseHandle((HANDLE) (t->handle));
	free(t);
}