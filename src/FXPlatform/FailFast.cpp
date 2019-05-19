#include "FailFast.h"
#include "NanoTrace.h"
#include <string>
using namespace std;

std::terminate_handler previousHandler = std::set_terminate(uncaught_exception_handler);
bool failFastIsException = false;
bool terminateCalled = false;

void failfastThis_uncaught_exception_handler(void *thisPtr, bool expression, const char *file, long line)
{
    failfastThis_uncaught_exception_handler(thisPtr, expression, "[No Message]", file, line);
}

void failfastThis_uncaught_exception_handler(void *thisPtr, bool expression, const char *description, const char *file, long line)
{
    if(!expression)
    {        
         TraceString3("FailFast: {0} (file:'{1}', line'{2}'",
                      SystemTraceType::System, TraceDetail::Normal,
                      description, file, line);
        
        // On iOS throwing an exception gives a really bad stack trace since there is a rethrow in CFRunLoop that obliterates the stack
        // Workaround is to call terminate directly
        // http://stackoverflow.com/questions/13777446/ios-how-to-get-stack-trace-of-an-unhandled-stdexception
        if(failFastIsException)
        {
            throw runtime_error(description);
        }
        else
        {
            terminateCalled = true;
            std::terminate();
        }
    }
}

void failfast_uncaught_exception_handler(bool expression, const char *description, const char *file, long line)
{
    failfastThis_uncaught_exception_handler(nullptr, expression, description, file, line);
}

void failfast_uncaught_exception_handler(bool expression, const char *file, long line)
{
    if(!expression)
    {
        failfast_uncaught_exception_handler(expression, "[No Message]", file, line);
    }
}

void TreatFailFastAsException(bool value)
{
    failFastIsException = value;
}

void uncaught_exception_handler (void)
{
    // Using throw to figure out the current exception theoretically only works with GCC...
    // http://efesx.com/2010/03/21/catching-uncaught-exceptions-within-terminate/
    // Only do it if we haven't called terminate directly since there won't be an exception then
    if(!terminateCalled)
    {
         try { throw; }
         catch (const std::runtime_error &err)
         {
             TraceString1("Unhandled runtime_error: {0} \r\n {1}",
                          SystemTraceType::System, TraceDetail::Normal,
                          err.what());
         }
         catch (...)
         {
             TraceString("Unhandled unknown exception occurred.",
                          SystemTraceType::System, TraceDetail::Normal);
         }
    }
    
    fflush(stderr);
    
    if(previousHandler)
    {
        previousHandler();
    }
    else
    {
        abort();
    }
}
