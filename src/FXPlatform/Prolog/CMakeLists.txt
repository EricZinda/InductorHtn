target_sources(lib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnArithmeticOperators.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnArithmeticOperators.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnGoalResolver.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnGoalResolver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnRule.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnRule.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnRuleSet.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnRuleSet.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnTerm.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnTerm.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnTermFactory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnTermFactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PrologCompiler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/PrologCompiler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PrologQueryCompiler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/PrologQueryCompiler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PrologParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/PrologParser.cpp
)
target_include_directories(lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(testLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)