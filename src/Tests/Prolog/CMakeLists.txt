target_sources(runtests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnGoalResolverTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnRuleSetTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnTermTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PrologCompilerTests.cpp
)
target_include_directories(testLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
