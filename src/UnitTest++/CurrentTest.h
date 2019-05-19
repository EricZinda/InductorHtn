#ifndef UNITTEST_CURRENTTESTRESULTS_H
#define UNITTEST_CURRENTTESTRESULTS_H

namespace UnitTest {

class TestResults;
class TestDetails;
class Test;
class Timer;

namespace CurrentTest
{
	TestResults*& Results();
	const TestDetails*& Details();
	Test*& CurrentTest();
	int &MaxTestTimeInMs();
	Timer &TestTimer();
}

}

#endif
