#include "TestRunner.h"
#include "TestResults.h"
#include "TestReporter.h"
#include "TestReporterStdout.h"
#include "TimeHelpers.h"
#include "MemoryOutStream.h"

#include <cstring>


namespace UnitTest {

int RunAllTests()
{
	TestReporterStdout reporter;
	TestRunner runner(reporter);
	return runner.RunTestsIf(Test::GetTestList(), NULL, True(), 0);
}


TestRunner::TestRunner(TestReporter& reporter)
	: m_reporter(&reporter)
	, m_result(new TestResults(&reporter))
	, m_timer(new Timer)
{
	m_timer->Start();
}

TestRunner::~TestRunner()
{
	delete m_result;
	delete m_timer;
}

int TestRunner::Finish() const
{
    float const secondsElapsed = m_timer->GetTimeInMs() / 1000.0f;
    m_reporter->ReportSummary(m_result->GetTotalTestCount(), 
							  m_result->GetFailedTestCount(), 
							  m_result->GetFailureCount(), 
							  secondsElapsed);
    
	return m_result->GetFailureCount();
}

bool TestRunner::IsTestInSuite(const Test* const curTest, char const* suiteName) const
{
	using namespace std;
	return (suiteName == NULL) || !strcmp(curTest->m_details.suiteName, suiteName);
}

void TestRunner::RunTest(TestResults* const result, Test* const curTest, int const maxTestTimeInMs) const
{
	CurrentTest::Results() = result;

	Timer testTimer;
	testTimer.Start();

	result->OnTestStart(curTest->m_details);

	curTest->Run();

	int const testTimeInMs = testTimer.GetTimeInMs();
	if (maxTestTimeInMs > 0 && testTimeInMs > maxTestTimeInMs && !curTest->m_timeConstraintExempt)
	{
	    MemoryOutStream stream;
	    stream << "Global time constraint failed. Expected under " << maxTestTimeInMs <<
	            "ms but took " << testTimeInMs << "ms.";

	    result->OnTestFailure(curTest->m_details, stream.GetText());
	}

	result->OnTestFinish(curTest->m_details, testTimeInMs/1000.0f);
}

void TestRunner::RunTestBegin(TestResults* const result, Test* const curTest, int const maxTestTimeInMs) const
{
	CurrentTest::Results() = result;
	CurrentTest::CurrentTest() = curTest;
	CurrentTest::MaxTestTimeInMs() = maxTestTimeInMs;
	CurrentTest::Details() = &(curTest->m_details);
	CurrentTest::TestTimer().Start();

	result->OnTestStart(curTest->m_details);

	curTest->Run();
}

bool TestRunner::RunTestContinue() const
{
	if(CurrentTest::CurrentTest()->Continue())
	{
		return true;
	}

	int const testTimeInMs = CurrentTest::TestTimer().GetTimeInMs();
	if (CurrentTest::MaxTestTimeInMs() > 0 && testTimeInMs > CurrentTest::MaxTestTimeInMs() && !CurrentTest::CurrentTest()->m_timeConstraintExempt)
	{
	    MemoryOutStream stream;
	    stream << "Global time constraint failed. Expected under " << CurrentTest::MaxTestTimeInMs() <<
	            "ms but took " << testTimeInMs << "ms.";

	    CurrentTest::Results()->OnTestFailure(CurrentTest::CurrentTest()->m_details, stream.GetText());
	}

	CurrentTest::Results()->OnTestFinish(CurrentTest::CurrentTest()->m_details, testTimeInMs/1000.0f);

	return false;
}

}
