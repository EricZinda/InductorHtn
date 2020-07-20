//
//  TraceError.cpp
//  FXPlatform
//
//  Created by Eric Zinda on 8/18/15.
//  Copyright (c) 2015 Eric Zinda. All rights reserved.
//

#include "TraceError.h"
#include <execinfo.h>
#include <sstream>
using namespace std;

void TraceError::CaptureStack(unsigned int max_frames)
{
    // storage array for stack trace address data
    void* addrlist[max_frames+1];
    
    // retrieve current stack addresses
    m_frameCount = backtrace( addrlist, sizeof( addrlist ) / sizeof(void *));
    
    if (m_frameCount == 0)
    {
        return;
    }
    
    // create readable strings to each frame.
    m_symbolList = backtrace_symbols(addrlist, m_frameCount);
}

string TraceError::Stack() const
{
    stringstream stream;
    for ( uint32_t i = 0; i < m_frameCount; ++i)
    {
        stream << m_symbolList[i] << "\r\n";
    }
    
    return stream.str();
}
