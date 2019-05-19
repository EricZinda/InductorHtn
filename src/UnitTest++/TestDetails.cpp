#include "TestDetails.h"

namespace UnitTest {

TestDetails::TestDetails(char const* testName_, char const* suiteName_, char const* filename_, int lineNumber_, bool async_, bool allowCrash_)
    : suiteName(suiteName_)
    , testName(testName_)
    , filename(filename_)
    , lineNumber(lineNumber_)
	, async(async_)
    , allowCrash(allowCrash_)
{
}

TestDetails::TestDetails(const TestDetails& details, int lineNumber_)
    : suiteName(details.suiteName)
    , testName(details.testName)
    , filename(details.filename)
    , lineNumber(lineNumber_)
{
}


}
