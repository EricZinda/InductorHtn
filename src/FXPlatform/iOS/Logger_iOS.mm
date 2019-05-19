//
//  Logger.cpp
//  GameLib
//
//  Created by Eric Zinda on 5/19/15.
//  Copyright (c) 2015 Eric Zinda. All rights reserved.
//
//#define TESTFAIRY

#include "Logger.h"
#import <Foundation/Foundation.h>

void DebugLogMessage(int traceType, const TraceDetail levelOfDetail, const char *message)
{
    NSLog(@"%@", [NSString stringWithUTF8String:message]);
}
