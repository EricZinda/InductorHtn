target_sources(runtests
    PRIVATE
    AIHarness.h
    HtnPlannerTests.cpp
    HtnCompilerTests.cpp
)
target_include_directories(testLib PRIVATE ./)
