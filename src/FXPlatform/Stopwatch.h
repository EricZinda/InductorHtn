#pragma once
class StopWatchImpl;

 class StopWatch 
 {
 public:
     StopWatch();
     StopWatch(const StopWatch &other);
     StopWatch &operator=(const StopWatch &rhs);
     ~StopWatch();
     void startTimer();
     void stopTimer();
	 double restartTimer();
     // Returns time in seconds
     double getElapsedTime();
	 double getCurrentTime();
     
 private:
     void swap(StopWatch& first, StopWatch& second);
     StopWatchImpl *m_impl;
 };
