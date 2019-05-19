#include "Stopwatch.h"
#include <algorithm>
#include <mach/mach_time.h>

class StopWatchImpl
{
public:
    StopWatchImpl() :
        start(0),
        stop(0)
    {
    
    }
    StopWatchImpl(const StopWatchImpl &other)
    {
        
    }

    mach_timebase_info_data_t timeBaseInfo;
    double frequency;
    uint64_t start;
    uint64_t stop;
};

StopWatch::StopWatch()
{
    m_impl = new StopWatchImpl();
    mach_timebase_info(&m_impl->timeBaseInfo);
    
    uint32_t sToNanosNumerator = m_impl->timeBaseInfo.numer;
    uint32_t sToNanosDenominator = m_impl->timeBaseInfo.denom;
    
    // the frequency of that clock in ticks per second = ticksPerScond = (sToNanosDenominator / sToNanosNumerator) * 10^9
    // sconds / tick = sToNanosNumerator / (sToNanosDenominator * 10^9)
    m_impl->frequency = static_cast<double>(sToNanosNumerator) / (static_cast<double>(sToNanosDenominator) * 1000000000.0);
}

StopWatch::StopWatch(const StopWatch &other)
{
    m_impl = new StopWatchImpl(*other.m_impl);
}

StopWatch &StopWatch::operator=(const StopWatch &rhs)
{
    StopWatch tmp(rhs);
    swap(*this, tmp);
    return *this;
}

void StopWatch::swap(StopWatch& first, StopWatch& second)
{
    using std::swap;
    swap(first.m_impl, second.m_impl);
}

StopWatch::~StopWatch()
{
    delete m_impl;
}

void StopWatch::startTimer( )
{
    m_impl->start = mach_absolute_time();
    m_impl->stop = 0;
}

void StopWatch::stopTimer( )
{
    m_impl->stop = mach_absolute_time();
}

double StopWatch::getCurrentTime()
{
    return mach_absolute_time() * m_impl->frequency;
}

double StopWatch::getElapsedTime()
{
    double tempStop;
    
    if(m_impl->stop == 0 && m_impl->start == 0)
    {
        return 0;
    }
    
    if(m_impl->stop == 0)
    {
        tempStop = mach_absolute_time();
    }
    else
    {
        tempStop = m_impl->stop;
    }
    
    return (tempStop - m_impl->start) * m_impl->frequency;
}

double StopWatch::restartTimer()
{
	double elapsedTime;
	if(m_impl->start == 0)
	{
		elapsedTime = 0;
	}
	else
	{
		stopTimer();
		elapsedTime = getElapsedTime();
	}
	startTimer();
	return elapsedTime;
}
