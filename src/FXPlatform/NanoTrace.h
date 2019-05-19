#pragma once
#include <cfloat>
#include <map>
#include <limits>
#include <mutex>
#include "ReflectionEnum.h"
#include <sstream>
#include "SystemTraceType.h"
#include "Utilities.h"
#include <vector>
using namespace std;

/*
Designed as a lightweight tracing mechanism that allows for outputting different levels of detail (TraceDetail)
And different kinds of traces (SystemTraceType) for debugging purposes
*/

enum class TraceDetail
{
	Normal,
	Detailed,
	Diagnostic
};

class TraceRecord
{
public:
	TraceRecord()
	{
		m_data.resize(10);
	}

	ValueProperty(private, int, dataCount);
	Property(private, vector<string>, data);
	ValueProperty(private, double, elapsedTime);
	ValueProperty(private, bool, isTiming);
	ValueProperty(private, uint64_t, timestamp);

public:
	string& traceKey() { return m_traceKey; }
	void traceKey(const string& value) { m_traceKey = value; }

private:
	string m_traceKey;
};

// Used to set what traces get emitted
#define SetTraceFilter(traceType, levelOfDetail) \
    NanoTrace::Global().allowedTraceType((int) (traceType)); \
    NanoTrace::Global().detailLevel(levelOfDetail); 
#define SetTraceDefault() \
    SetTraceFilter(SystemTraceType::All, TraceDetail::Normal); \
    NanoTrace::Global().debugTraceType((int) SystemTraceType::All);
#define AddTraceType(traceType) NanoTrace::Global().allowedTraceType(NanoTrace::Global().allowedTraceType() | (int) traceType);
#define SetTraceLevelOfDetail(levelOfDetail) NanoTrace::Global().detailLevel(levelOfDetail);
#define SetTraceFilterOnly(traceType) NanoTrace::Global().allowedTraceType((int) (traceType));

// Used for timing but stubbed out to reduce code complexity
#define ResetTiming(name) 
#define StartTiming(name, traceType, levelOfDetail) 
#define StartTiming1(name, traceType, levelOfDetail, value1) 
#define EndTiming(name, traceType, levelOfDetail) 
#define EndTimingAssert(name, maxTime, traceType, levelOfDetail)
#define StartTimingOnly(name, traceType, levelOfDetail) 
#define EndTimingOnly(name, traceType, levelOfDetail) 
#define EndTimingTraceIf(name, maxTime, traceType, levelOfDetail) 
#define EndTimingOnlyAssert(name, maxTime, traceType, levelOfDetail)

#define TraceString(string, traceType, traceDetail) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail); }

#define TraceString1(string, traceType, traceDetail, value) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value); }

#define TraceString2(string, traceType, traceDetail, value1, value2)\
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value1, value2); }

#define TraceString3(string, traceType, traceDetail, value1, value2, value3) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value1, value2, value3); }

#define TraceString4(string, traceType, traceDetail, value1, value2, value3, value4) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value1, value2, value3, value4); }

#define TraceString5(string, traceType, traceDetail, value1, value2, value3, value4, value5) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value1, value2, value3, value4, value5); }

#define TraceString6(string, traceType, traceDetail, value1, value2, value3, value4, value5, value6) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value1, value2, value3, value4, value5, value6); }

#define TraceString7(string, traceType, traceDetail, value1, value2, value3, value4, value5, value6, value7) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value1, value2, value3, value4, value5, value6, value7); }

#define TraceString8(string, traceType, traceDetail, value1, value2, value3, value4, value5, value6, value7, value8) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value1, value2, value3, value4, value5, value6, value7, value8); }

#define TraceString9(string, traceType, traceDetail, value1, value2, value3, value4, value5, value6, value7, value8, value9) \
    if(((int) traceType & NanoTrace::Global().allowedTraceType()) && (traceDetail <= NanoTrace::Global().detailLevel())) \
    { NanoTrace::Global().Trace(string, (int) traceType, traceDetail, value1, value2, value3, value4, value5, value6, value7, value8, value9); }

#define TraceStringIf(string, conditional, traceType, traceDetail) \
    if(conditional) { TraceString(string, (int) traceType, traceDetail); }
#define TraceString1If(string, conditional, traceType, traceDetail, value) \
    if(conditional) { TraceString1(string, (int) traceType, traceDetail, value); }
#define TraceString2If(string, conditional, traceType, traceDetail, value1, value2) \
    if(conditional) { TraceString2(string, (int) traceType, traceDetail, value1, value2); }
#define TraceString3If(string, conditional, traceType, traceDetail, value1, value2, value3) \
    if(conditional) { TraceString3(string, (int) traceType, traceDetail, value1, value2, value3); }
#define TraceString4If(string, conditional, traceType, traceDetail, value1, value2, value3, value4) \
    if(conditional) { TraceString4(string, (int) traceType, traceDetail, value1, value2, value3, value4); }

class NanoTrace
{
public:
    NanoTrace(int size);
    
    unsigned long allowedTraceType() { return m_allowedTraceType; }
    void allowedTraceType(unsigned long value) { m_allowedTraceType = value; }
    TraceDetail detailLevel() { return m_detailLevel; }
    void detailLevel(TraceDetail value) { m_detailLevel = value; }
    static NanoTrace &Global() { return m_nanoTrace; };

	double Trace(const string& traceKey, const int traceType, const TraceDetail levelOfDetail)
	{
		return TraceImpl(traceKey, -1, traceType, levelOfDetail);
	};

	template <class T> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T value1)
	{
		return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr, 1, lexical_cast<string>(value1).c_str());
	};

    template <class T1, class T2> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            2, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str());

	};

	template <class T1, class T2, class T3> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2, const T3 value3)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            3, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str(), lexical_cast<string>(value3).c_str());

	};

	template <class T1, class T2, class T3, class T4> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2, const T3 value3, const T4 &value4)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            4, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str(), lexical_cast<string>(value3).c_str(), lexical_cast<string>(value4).c_str());

	};

	template <class T1, class T2, class T3, class T4, class T5> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2, const T3 value3, const T4 &value4, const T5 &value5)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            5, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str(), lexical_cast<string>(value3).c_str(), lexical_cast<string>(value4).c_str(), lexical_cast<string>(value5).c_str());

	};

	template <class T1, class T2, class T3, class T4, class T5, class T6> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2, const T3 value3, const T4 &value4, const T5 &value5, 
		const T6 &value6)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            6, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str(), lexical_cast<string>(value3).c_str(), lexical_cast<string>(value4).c_str(), lexical_cast<string>(value5).c_str(),
                lexical_cast<string>(value6).c_str());
	};

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2, const T3 value3, const T4 &value4, const T5 &value5, 
		const T6 &value6, const T7 &value7)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            7, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str(), lexical_cast<string>(value3).c_str(), lexical_cast<string>(value4).c_str(), lexical_cast<string>(value5).c_str(),
                lexical_cast<string>(value6).c_str(), lexical_cast<string>(value7).c_str());

	};

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2, const T3 value3, const T4 &value4, const T5 &value5, 
		const T6 &value6, const T7 &value7, const T8 &value8)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            8, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str(), lexical_cast<string>(value3).c_str(), lexical_cast<string>(value4).c_str(), lexical_cast<string>(value5).c_str(),
                lexical_cast<string>(value6).c_str(), lexical_cast<string>(value7).c_str(), lexical_cast<string>(value8).c_str());

	};

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2, const T3 value3, const T4 &value4, const T5 &value5, 
		const T6 &value6, const T7 &value7, const T8 &value8, const T9 &value9)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            9, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str(), lexical_cast<string>(value3).c_str(), lexical_cast<string>(value4).c_str(), lexical_cast<string>(value5).c_str(),
                lexical_cast<string>(value6).c_str(), lexical_cast<string>(value7).c_str(), lexical_cast<string>(value8).c_str(), lexical_cast<string>(value9).c_str());
	};

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10> 
	double Trace(const string &traceKey, const int traceType, const TraceDetail levelOfDetail, const T1 value1, const T2 &value2, const T3 value3, const T4 &value4, const T5 &value5, 
		const T6 &value6, const T7 &value7, const T8 &value8, const T9 &value9, const T10 &value10)
	{
        return TraceImpl(traceKey, -1, traceType, levelOfDetail, false, nullptr,
            10, lexical_cast<string>(value1).c_str(), lexical_cast<string>(value2).c_str(), lexical_cast<string>(value3).c_str(), lexical_cast<string>(value4).c_str(), lexical_cast<string>(value5).c_str(),
                lexical_cast<string>(value6).c_str(), lexical_cast<string>(value7).c_str(), lexical_cast<string>(value8).c_str(), lexical_cast<string>(value9).c_str(), lexical_cast<string>(value10).c_str());
	};

private:
	static void FormatTrace(TraceRecord& record, ostream& stream);
    double TraceImpl(const string &traceKey, const double startTime, int traceType, const TraceDetail levelOfDetail, bool timingOnly = false, double *timingResult = nullptr,
                        int dataCount = 0,
                        const char *data1 = nullptr, const char *data2 = nullptr, const char *data3 = nullptr, const char *data4= nullptr, const char *data5 = nullptr,  
                        const char *data6 = nullptr, const char *data7 = nullptr, const char *data8 = nullptr, const char *data9 = nullptr, const char *data10 = nullptr);

    unsigned long m_allowedTraceType;
    TraceDetail m_detailLevel;
	static NanoTrace m_nanoTrace;
    static const char *m_replaceTokens[];
};
