#ifndef UNITTEST_EXECUTE_TEST_H
#define UNITTEST_EXECUTE_TEST_H

#include "TestDetails.h"
#include "MemoryOutStream.h"
#include "AssertException.h"
#include "CurrentTest.h"

#ifdef UNITTEST_POSIX
	#include "Posix/SignalTranslator.h"
#endif

namespace UnitTest {

template< typename T >
void ExecuteTest(T& testObject, TestDetails const& details)
{
	CurrentTest::Details() = &details;

    if(details.allowCrash)
    {
        // Allow crashes so we can get the actual crash report on the device
        // Otherwise we can't see the stack trace and actual crash location
        testObject.RunImpl();
        return;
    }
    
	try
	{
#ifdef UNITTEST_POSIX
		UNITTEST_THROW_SIGNALS
#endif
		testObject.RunImpl();
	}
	catch (AssertException const& e)
	{
		CurrentTest::Results()->OnTestFailure(
			TestDetails(details.testName, details.suiteName, e.Filename(), e.LineNumber()), e.what());
	}
	catch (std::exception const& e)
	{
		MemoryOutStream stream;
		stream << "Unhandled exception: " << e.what();
		CurrentTest::Results()->OnTestFailure(details, stream.GetText());
	}
	catch (...)
	{
		CurrentTest::Results()->OnTestFailure(details, "Unhandled exception: Crash!");
	}
}

template< typename T >
bool ContinueTest(T& testObject, TestDetails const& details)
{
    if(details.allowCrash)
    {
        // Allow crashes so we can get the actual crash report on the device
        // Otherwise we can't see the stack trace and actual crash location
        if(CurrentTest::Results()->GetFailureCount() > 0)
        {
            //Return immediately if the Begin had a failure
            return false;
        }
        else
        {
            return testObject.RunContinueImpl();
        }
    }

	try
	{
#ifdef UNITTEST_POSIX
		UNITTEST_THROW_SIGNALS
#endif
		if(CurrentTest::Results()->GetFailureCount() > 0)
		{
			//Return immediately if the Begin had a failure
			return false;
		}
		else
		{
			return testObject.RunContinueImpl();
		}
	}
	catch (AssertException const& e)
	{
		CurrentTest::Results()->OnTestFailure(
			TestDetails(details.testName, details.suiteName, e.Filename(), e.LineNumber()), e.what());
	}
	catch (std::exception const& e)
	{
		MemoryOutStream stream;
		stream << "Unhandled exception: " << e.what();
		CurrentTest::Results()->OnTestFailure(details, stream.GetText());
	}
	catch (...)
	{
		CurrentTest::Results()->OnTestFailure(details, "Unhandled exception: Crash!");
	}

	return false;
}

}

#endif
