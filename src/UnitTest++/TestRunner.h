#ifndef UNITTEST_TESTRUNNER_H
#define UNITTEST_TESTRUNNER_H

#include "Test.h"
#include "TestList.h"
#include "CurrentTest.h"
#include <cstddef>
namespace UnitTest {

class TestReporter;
class TestResults;
class Timer;

int RunAllTests();

struct True
{
	bool operator()(const Test* const) const
	{
		return true;	
	}
};

class TestRunner
{
public:
	explicit TestRunner(TestReporter& reporter);
	~TestRunner();

	template <class Predicate>
	int RunTestsIf(TestList const& list, char const* suiteName, 
				   const Predicate& predicate, int maxTestTimeInMs) const
	{
	    Test* curTest = list.GetHead();

	    while (curTest != 0)
	    {
		    if (IsTestInSuite(curTest,suiteName) && predicate(curTest))
			{
				if(curTest->m_details.async)
				{
					RunTestBegin(m_result, curTest, maxTestTimeInMs);
					while(RunTestContinue());
				}
				else
				{
					RunTest(m_result, curTest, maxTestTimeInMs);
				}
			}

			curTest = curTest->next;
	    }

	    return Finish();
	}	

	void BeginContinueTests()
	{
		CurrentTest::CurrentTest() = nullptr;
	}

	template <class Predicate>
	bool RunContinueTestsIf(TestList const& list, char const* suiteName, 
				   const Predicate& predicate, int maxTestTimeInMs) const
	{
		Test* curTest;

		if(CurrentTest::CurrentTest() == nullptr)
		{
			CurrentTest::CurrentTest() = list.GetHead();
			curTest = list.GetHead();
		}
		else
		{
			curTest = CurrentTest::CurrentTest();
			if(RunTestContinue())
			{
				return true;
			}

			curTest = curTest->next;
		}

	    while (curTest != 0)
	    {
		    if (IsTestInSuite(curTest,suiteName) && predicate(curTest))
			{
				if(curTest->m_details.async)
				{
					RunTestBegin(m_result, curTest, maxTestTimeInMs);
					return true;
				}
				else
				{
					RunTest(m_result, curTest, maxTestTimeInMs);
				}
			}

			curTest = curTest->next;
	    }

	    return false;
	}

	int FinishContinueTests()
	{
		return Finish();
	}

private:
	TestReporter* m_reporter;
	TestResults* m_result;
	Timer* m_timer;

	int Finish() const;
	bool IsTestInSuite(const Test* const curTest, char const* suiteName) const;
	void RunTest(TestResults* const result, Test* const curTest, int const maxTestTimeInMs) const;
	void RunTestBegin(TestResults* const result, Test* const curTest, int const maxTestTimeInMs) const;
	bool RunTestContinue() const;

};

}

#endif
