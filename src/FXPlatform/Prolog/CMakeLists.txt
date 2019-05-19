target_sources(lib
    PRIVATE
    HtnArithmeticOperators.h
    HtnArithmeticOperators.cpp
    HtnGoalResolver.h
    HtnGoalResolver.cpp
    HtnRule.h
    HtnRule.cpp
    HtnRuleSet.h
    HtnRuleSet.cpp
    HtnTerm.h
    HtnTerm.cpp
    HtnTermFactory.h
    HtnTermFactory.cpp
    PrologCompiler.h
    PrologCompiler.cpp
    PrologQueryCompiler.h
    PrologQueryCompiler.cpp
    PrologParser.h
    PrologParser.cpp
)
target_include_directories(lib PRIVATE ./)
target_include_directories(testLib PRIVATE ./)