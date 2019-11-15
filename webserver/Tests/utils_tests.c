#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"
#include "../utils.h"

void TestGetKeys(CuTest *tc)
{
	HttpRequest request;

	strcpy(request.keys[0], "Host");
	strcpy(request.keys[1], "Content-Length");

	request.fields_amount = 2;

	const char *expected = "Host\nContent-Length\n";
	const char *actual = get_keys(&request);

	CuAssertStrEquals(tc, expected, actual);
}

void TestTest(CuTest *tc)
{
	CuAssertIntEquals(tc, 42, 42);
}

CuSuite *WebserverUtilsSuite(void)
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, TestGetKeys);
	SUITE_ADD_TEST(suite, TestTest);

	return suite;
}
