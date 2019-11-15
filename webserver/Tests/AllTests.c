#include <stdio.h>

#include "CuTest.h" // * http://people.cs.aau.dk/~bnielsen/TOV08/material/cutestintro.pdf

CuSuite *WebserverUtilsSuite();

void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite *suite = CuSuiteNew();

	CuSuiteAddSuite(suite, WebserverUtilsSuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

int main(void)
{
	RunAllTests();
}
