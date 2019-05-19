#include "Stopwatch.h"
#include "Windows.h"


class StopWatchImpl
{
public:
	StopWatchImpl()
	{
		timer.start.QuadPart = 0;
		timer.stop.QuadPart = 0;
		QueryPerformanceFrequency(&frequency);
	}

	typedef struct
	{
		LARGE_INTEGER start;
		LARGE_INTEGER stop;
	} stopWatch;

	stopWatch timer;
	LARGE_INTEGER frequency;
	double LIToSecs(LARGE_INTEGER& L)
	{
		return ((double)L.QuadPart / (double)frequency.QuadPart);
	}
};

StopWatch::StopWatch()
{
	m_impl = new StopWatchImpl();
}

StopWatch::~StopWatch()
{
	if (m_impl != nullptr)
	{
		delete m_impl;
		m_impl = nullptr;
	}
}

void StopWatch::startTimer( ) 
{
    QueryPerformanceCounter(&m_impl->timer.start);
}

void StopWatch::stopTimer( ) 
{
    QueryPerformanceCounter(&m_impl->timer.stop);
}

double StopWatch::getCurrentTime()
{
	LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
	return m_impl->LIToSecs(time);
}

double StopWatch::getElapsedTime() 
{
	LARGE_INTEGER time;
	time.QuadPart = &m_impl->timer.stop.QuadPart - &m_impl->timer.start.QuadPart;
    return m_impl->LIToSecs(time) ;
}

double StopWatch::restartTimer()
{
	double elapsedTime;
	if(&m_impl->timer.start.HighPart == 0 && &m_impl->timer.start.LowPart == 0)
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
