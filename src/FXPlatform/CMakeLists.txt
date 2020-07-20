target_sources(lib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Directory.h
    	${CMAKE_CURRENT_SOURCE_DIR}/FailFast.h
    	${CMAKE_CURRENT_SOURCE_DIR}/FailFast.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileStream.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Logger.h
    	${CMAKE_CURRENT_SOURCE_DIR}/NanoTrace.h
    	${CMAKE_CURRENT_SOURCE_DIR}/NanoTrace.cpp
    	${CMAKE_CURRENT_SOURCE_DIR}/ReflectionEnum.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Stopwatch.h
    	${CMAKE_CURRENT_SOURCE_DIR}/SystemTraceType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/TraceError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Utilities.h
    	${CMAKE_CURRENT_SOURCE_DIR}/Utilities.cpp
)
target_include_directories(lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(testLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(runtests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)