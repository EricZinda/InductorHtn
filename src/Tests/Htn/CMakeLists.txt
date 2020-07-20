target_sources(runtests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/AIHarness.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnPlannerTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/HtnCompilerTests.cpp
)
target_include_directories(testLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
