//
//  Logger.h
//  GameLib
//
//  Created by Eric Zinda on 5/19/15.
//  Copyright (c) 2015 Eric Zinda. All rights reserved.
//

#ifndef __GameLib__Logger__
#define __GameLib__Logger__

#include <stdio.h>
#include "NanoTrace.h"

// Outputs a Debug message to whatever is appropriate
void DebugLogMessage(int traceType, const TraceDetail levelOfDetail, const char *message);
// Also logs messages to a file
void DebugLogMessagesToFile(const std::string &filename);


#endif /* defined(__GameLib__Logger__) */
