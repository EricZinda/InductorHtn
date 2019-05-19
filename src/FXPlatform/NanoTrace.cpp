#include "FailFast.h"
#include "Logger.h"
#include "NanoTrace.h"
#include <iomanip>
#include <fstream>
#include <algorithm>

DEFINE_ENUM(SystemTraceType, SYSTEM_TRACE_TYPE);

NanoTrace NanoTrace::m_nanoTrace(3000);
const char *NanoTrace::m_replaceTokens[] = { "{0}", "{1}", "{2}", "{3}", "{4}", "{5}", "{6}", "{7}", "{8}", "{9}" };

NanoTrace::NanoTrace(int size) :
    m_allowedTraceType((unsigned long)SystemTraceType::All),
    m_detailLevel(TraceDetail::Normal)
{
}

void NanoTrace::FormatTrace(TraceRecord& record, ostream& stream)
{
	string substitutedString = record.traceKey();

	// Write the timestamp
	char buffer[20];
	time_t timestamp = record.timestamp();
	
	// Disable "unsafe" warning in Microsoft C++
	#pragma warning( disable : 4996 )
	strftime(buffer, 20, "%m%d %H:%M:%S ", localtime(&timestamp));
	stream << buffer;

	if (record.isTiming())
	{
		stream << "=" << std::fixed << std::setw(6) << std::setprecision(4)
			<< std::setfill(' ') << record.elapsedTime() << " ";
	}
	else
	{
		stream << "^" << std::fixed << std::setw(6) << std::setprecision(4)
			<< std::setfill(' ') << record.elapsedTime() << " ";
	}

	// Fill in the traceKey with any data, anything not used should be tacked onto the end
	bool hasLeftoverData = false;
	for (int dataIndex = 0; dataIndex < record.dataCount(); ++dataIndex)
	{
		if (!ReplaceAll(substitutedString, m_replaceTokens[dataIndex], record.data()[dataIndex]))
		{
			hasLeftoverData = true;
		}
	}

	stream << substitutedString;
	if (hasLeftoverData)
	{
		stream << ": ";
		for (int dataIndex = 0; dataIndex < record.dataCount(); ++dataIndex)
		{
			stream << record.data()[dataIndex] << ", ";
		}
	}

	stream << "\r\n";
}

// Normally this is all true, but removed that code to reduce complexity and just returns 0:
// Returns the current time at the end of the routine, useful for tracking the start of an event
// if startTime > -1, then timingResult is filled in with the difference between startTime and the time at the beginning of the routine
// this is useful for tracking the end of an event
double NanoTrace::TraceImpl(const string &traceKey, const double startTime, int traceType, const TraceDetail levelOfDetail, bool timingOnly, double *timingResult, 
                        int dataCount,
                        const char *data1, const char *data2, const char *data3, const char *data4, const char *data5,  
                        const char *data6, const char *data7, const char *data8, const char *data9, const char *data10)
{
    if(!(traceType & allowedTraceType()) || !(levelOfDetail <= detailLevel()))
    {
        return 0;
    }  

    // Record the Trace results
    TraceRecord record;

    record.dataCount(dataCount);
    record.elapsedTime(0);
    record.traceKey(traceKey);
    FailFastAssert(sizeof(time_t) <= sizeof(record.timestamp()));
    record.timestamp(time(nullptr));

    switch(dataCount)
    {
    case 10:
        record.data()[9] = data10;
    case 9:
        record.data()[8] = data9;
    case 8:
        record.data()[7] = data8;
    case 7:
        record.data()[6] = data7;
    case 6:
        record.data()[5] = data6;
    case 5:
        record.data()[4] = data5;
    case 4:
        record.data()[3] = data4;
    case 3:
        record.data()[2] = data3;
    case 2:
        record.data()[1] = data2;        
    case 1:
        record.data()[0] = data1;
    }
        
    stringstream stream;
    FormatTrace(record, stream);
    DebugLogMessage(traceType, levelOfDetail, stream.str().c_str());
    
    return 0;
}

