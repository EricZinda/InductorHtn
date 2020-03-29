//
//  Logger.cpp
//  GameLib
//
//  Created by Eric Zinda on 5/19/15.
//  Copyright (c) 2015 Eric Zinda. All rights reserved.
//
//#define TESTFAIRY
#include "FileStream.h"
#include "Logger.h"
#import <Foundation/Foundation.h>

shared_ptr<FileStream> fileStream;

void DebugLogMessagesToFile(const std::string &filename)
{
    if(filename.size() == 0)
    {
        fileStream = nullptr;
    }
    else
    {
        fileStream = shared_ptr<FileStream>(new FileStream());
        fileStream->Open(filename, FileOpenStyle::CreateAlways, AccessRights::Write);
        fileStream->Write("Begin Logging");
    }
}

void DebugLogMessage(int traceType, const TraceDetail levelOfDetail, const char *message)
{
    if(fileStream != nullptr)
    {
        string stringMessage(message);
        fileStream->Write(stringMessage);
    }
    
    NSLog(@"%@", [NSString stringWithUTF8String:message]);
}
