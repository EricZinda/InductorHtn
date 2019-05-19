#pragma once
#include "ReflectionEnum.h"

// Types < Custom are reserved for the system
// This is a bitfield
#define SYSTEM_TRACE_TYPE(item) \
    item(SystemTraceType, None, 0) \
    item(SystemTraceType, System, 1) \
    item(SystemTraceType, Parsing, 32) \
    item(SystemTraceType, Custom, 0x00000800) \
    item(SystemTraceType, HTML, (uint64_t)SystemTraceType::Custom * 8) \
    item(SystemTraceType, Solver, (uint64_t)SystemTraceType::Custom * 4096) \
    item(SystemTraceType, Unifier, (uint64_t)SystemTraceType::Custom * 8192) \
    item(SystemTraceType, Planner, (uint64_t)SystemTraceType::Custom * 16384) \
    item(SystemTraceType, All, 0x0FFFFFFF)
DECLARE_ENUM(SystemTraceType,SYSTEM_TRACE_TYPE)
