target_sources(lib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnCompiler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnCompiler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnDomain.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnMethod.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnMethod.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnOperator.h    
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnOperator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnPlanner.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnPlanner.cpp
)
target_include_directories(lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(testLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)