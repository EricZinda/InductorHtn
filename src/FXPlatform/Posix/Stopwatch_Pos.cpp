#include "Stopwatch.h"
#include <algorithm>
#include <time.h>

timespec timediff(timespec start, timespec end);

class StopWatchImpl
{
public:
    StopWatchImpl() :
        start((struct timespec){0}),
        stop((struct timespec){0})
    {
        
    }
    StopWatchImpl(const StopWatchImpl &other)
    {
        
    }

    timespec start;
    timespec stop;
};

StopWatch::StopWatch()
{
    m_impl = new StopWatchImpl();
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
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_impl->start);
    m_impl->stop = (struct timespec){0};
}

void StopWatch::stopTimer( )
{
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_impl->stop);
}

double StopWatch::getCurrentTime()
{
    return 0;
}

double StopWatch::getElapsedTime()
{
    timespec tempStop;
    
    if(m_impl->stop.tv_nsec == 0 && m_impl->start.tv_nsec == 0)
    {
        return 0;
    }
    
    if(m_impl->stop.tv_nsec == 0)
    {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tempStop);
    }
    else
    {
        tempStop = m_impl->stop;
    }
    
    return timediff(m_impl->start, tempStop).tv_nsec;
}

double StopWatch::restartTimer()
{
	double elapsedTime;
	if(m_impl->start.tv_nsec == 0)
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

timespec timediff(timespec start, timespec end)
{
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}
