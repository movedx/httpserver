#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"
#include "../utils.h"

HttpRequest makeHttpRequest_1();
HttpRequest makeHttpRequest_2();
HttpRequest makeHttpRequest_3();

void TestGetKeys(CuTest *tc)
{
	HttpRequest request = makeHttpRequest_1();
	const char *expected = "Host\nContent-Length\n";
	const char *actual = get_keys(&request);
	CuAssertStrEquals(tc, expected, actual);

	request = makeHttpRequest_2();
	expected = "Host\n";
	actual = get_keys(&request);
	CuAssertStrEquals(tc, expected, actual);

	request = makeHttpRequest_3();
	expected = "";
	actual = get_keys(&request);
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

HttpRequest makeHttpRequest_1()
{
	HttpRequest request;

	strcpy(request.keys[0], "Host");
	strcpy(request.keys[1], "Content-Length");
	request.fields_amount = 2;

	return request;
}

HttpRequest makeHttpRequest_2()
{
	HttpRequest request;

	strcpy(request.keys[0], "Host");
	request.fields_amount = 1;

	return request;
}

HttpRequest makeHttpRequest_3()
{
	HttpRequest request;

	request.fields_amount = 0;

	return request;
}