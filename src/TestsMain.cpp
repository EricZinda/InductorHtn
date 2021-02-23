#include "FXPlatform/FailFast.h"
#include "FXPlatform/SystemTraceType.h"
#include "FXPlatform/NanoTrace.h"
#include "UnitTest++/UnitTest++.h"
#include "UnitTest++/TestReporterStdout.h"
using namespace UnitTest;

// This class filters out what tests get run if you are isolating failures
class TestFilter
{
    public:
    bool operator()(UnitTest::Test *test) const
    {
        TraceString(string(test->m_details.testName), SystemTraceType::System, TraceDetail::Normal);

        // Return true if the test matches the criteria for a test you want to run. Examples:
        // Just run this one test:
        //return strcmp(test->m_details.testName, "HtnGoalResolverDistinctTests") == 0;
        // Run just this suite of tests:
        //return strcmp(test->m_details.suiteName, "HtnPlannerTests") == 0;
        
        // Return true to run everything
        return true;
    }
};

int main (int argc, char *argv[])
{
	// Treat all FailFasts as exceptions when running tests so the process doesn't abort
	TreatFailFastAsException(true);
    SetTraceFilter((int)SystemTraceType::None, TraceDetail::Normal);

    // Uncomment this to see detailed traces on stderr
    //SetTraceFilter((int)SystemTraceType::Solver | (int)SystemTraceType::Planner | (int) SystemTraceType::System, TraceDetail::Diagnostic);
//    SetTraceFilter((int)SystemTraceType::System, TraceDetail::Normal);

    TestReporterStdout reporter;
    TestRunner runner(reporter);
    return runner.RunTestsIf(Test::GetTestList(), NULL, TestFilter(), 0);
}
