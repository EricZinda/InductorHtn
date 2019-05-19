#ifndef UNITTEST_TEST_H
#define UNITTEST_TEST_H

#include "TestDetails.h"

namespace UnitTest {

class TestResults;
class TestList;

class Test
{
public:
    explicit Test(char const* testName, char const* suiteName = "DefaultSuite", char const* filename = "", int lineNumber = 0, bool async = false, bool allowCrash = false);
    virtual ~Test();
    void Run();
	bool Continue();

    TestDetails m_details;
    Test* next;
    mutable bool m_timeConstraintExempt;

    static TestList& GetTestList();

    virtual void RunImpl();
	virtual bool RunContinueImpl();

private:
	Test(Test const&);
    Test& operator =(Test const&);
};


}

#endif
