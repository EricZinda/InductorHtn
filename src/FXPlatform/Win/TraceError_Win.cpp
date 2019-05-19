//
//  TraceError.cpp
//  FXPlatform
//
//  Created by Eric Zinda on 8/18/15.
//  Copyright (c) 2015 Eric Zinda. All rights reserved.
//

#include "TraceError.h"
#include <sstream>
using namespace std;

void TraceError::CaptureStack(unsigned int max_frames)
{
	// TODO: add this feature for windows
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
