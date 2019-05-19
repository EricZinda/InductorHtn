target_sources(runtests
    PRIVATE
	HtnGoalResolverTests.cpp
	HtnRuleSetTests.cpp
	HtnTermTests.cpp
    PrologCompilerTests.cpp
)
target_include_directories(testLib PRIVATE ./)
