//
//  TraceError.h
//  FXPlatform
//
//  Created by Eric Zinda on 8/18/15.
//  Copyright (c) 2015 Eric Zinda. All rights reserved.
//

#ifndef __FXPlatform__TraceError__
#define __FXPlatform__TraceError__

#include <stdio.h>
#include <string>
#include <stdexcept>

class TraceError : public std::runtime_error
{
public:
    explicit TraceError(const std::string& what_arg) :
        runtime_error(what_arg),
        m_symbolList(nullptr)
    {
        CaptureStack();
    }

    ~TraceError()
    {
        delete m_symbolList;
        m_symbolList = nullptr;
    }
    
    std::string Stack() const;
    
private:
    void CaptureStack(unsigned int max_frames = 63);
    char** m_symbolList;
    uint32_t m_frameCount;
};

#endif /* defined(__FXPlatform__TraceError__) */
