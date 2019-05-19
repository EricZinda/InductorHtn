#include "Config.h"
#include "Test.h"
#include "TestList.h"
#include "TestResults.h"
#include "AssertException.h"
#include "MemoryOutStream.h"
#include "ExecuteTest.h"

#ifdef UNITTEST_POSIX
    #include "Posix/SignalTranslator.h"
#endif

namespace UnitTest {

TestList& Test::GetTestList()
{
    static TestList s_list;
    return s_list;
}

Test::Test(char const* testName, char const* suiteName, char const* filename, int lineNumber, bool async, bool allowCrash)
    : m_details(testName, suiteName, filename, lineNumber, async, allowCrash)
    , next(0)
    , m_timeConstraintExempt(false)
{
}

Test::~Test()
{
}

void Test::Run()
{
	ExecuteTest(*this, m_details);
}

bool Test::Continue()
{
	return ContinueTest(*this, m_details);
}

void Test::RunImpl()
{
}

bool Test::RunContinueImpl()
{
	return false;
}

}
