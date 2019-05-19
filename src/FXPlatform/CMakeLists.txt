target_sources(lib
    PRIVATE
        Directory.h
    	FailFast.h
    	FailFast.cpp
        FileStream.h
        Logger.h
    	NanoTrace.h
    	NanoTrace.cpp
    	ReflectionEnum.h
        Stopwatch.h
    	SystemTraceType.h
        TraceError.h
        TraceError.cpp
    	Utilities.h
    	Utilities.cpp
)
target_include_directories(lib PRIVATE ./)
target_include_directories(testLib PRIVATE ./)
target_include_directories(runtests PRIVATE ./)