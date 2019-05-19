target_sources(lib
    PRIVATE
    HtnCompiler.h
    HtnCompiler.cpp
    HtnDomain.h
    HtnMethod.h
    HtnMethod.cpp
    HtnOperator.h    
    HtnOperator.cpp
    HtnParser.h
    HtnParser.cpp
    HtnPlanner.h
    HtnPlanner.cpp
)
target_include_directories(lib PRIVATE ./)
target_include_directories(testLib PRIVATE ./)