target_sources(runtests
    PRIVATE
    AIHarness.h
    HtnPlannerTests.cpp
)
target_include_directories(testLib PRIVATE ./)
