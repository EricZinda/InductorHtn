#include "CurrentTest.h"
#include "Test.h"
#include <cstddef>
#include "TimeHelpers.h"

namespace UnitTest {

TestResults*& CurrentTest::Results()
{
	static TestResults* testResults = NULL;
	return testResults;
}

const TestDetails*& CurrentTest::Details()
{
	static const TestDetails* testDetails = NULL;
	return testDetails;
}

Test*& CurrentTest::CurrentTest()
{
	static Test* test = NULL;
	return test;
}

int &CurrentTest::MaxTestTimeInMs()
{
	static int maxTestTimeInMs = 0;
	return maxTestTimeInMs;
}

Timer &CurrentTest::TestTimer()
{
	static Timer testTimer;
	return testTimer;
}

}
