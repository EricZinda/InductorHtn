//
//  HtnCompilerTests.cpp
//  TestLib
//
//  Created by Eric Zinda on 6/4/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include "FXPlatform/FailFast.h"
#include "FXPlatform/Parser/ParserDebug.h"
#include "FXPlatform/Prolog/HtnGoalResolver.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"
#include "FXPlatform/Htn/HtnPlanner.h"
#include "FXPlatform/Htn/HtnCompiler.h"
#include "Logger.h"
#include "Tests/ParserTestBase.h"
#include "UnitTest++/UnitTest++.h"
using namespace Prolog;

SUITE(HtnCompilerTests)
{
    TEST(HtnCompilerMissingTests)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        set<string> loops;
        string result;

        // Not missing, is an operator
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "test(?A) :- if(true), do( foo(?A) ). \r\n"
                                "foo(?A) :- del(), add(). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK(loops.size() == 0);
        
        // Simple missing task
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "test(?A) :- if(true), do( foo(?A) ). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK(loops.size() == 1);
        result = (*loops.begin());
        CHECK(result == "Task Not Found: foo/1");
        
        // Simple missing task in try()
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "test(?A) :- if(true), do( try(foo(?A)) ). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK(loops.size() == 1);
        result = (*loops.begin());
        CHECK(result == "Task Not Found: foo/1");
        
        // Missing prolog condition
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "test(?A) :- if(bill(?A)), do( ). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK(loops.size() == 1);
        result = (*loops.begin());
        CHECK(result == "Rule Not Found: bill/1");

        // Missing prolog condition in a custom rule
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "test(?A) :- if( sortBy(?A, >(bill(?A))) ), do( ). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK(loops.size() == 1);
        result = (*loops.begin());
        CHECK(result == "Rule Not Found: bill/1");
    }
    
    TEST(HtnCompilerLoopTests)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        set<string> loops;
        string result;
        
        // Simple loop
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "test(?A) :- if(true), do( foo(?A) ). \r\n"
                                "foo(?A) :- if(true), do( test(?A) ). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK_EQUAL(2, loops.size());
        result = (*loops.begin());
        CHECK(result == "Task Loop: foo/1...test/1...LOOP -> foo/1");
        result = (*(++loops.begin()));
        CHECK( result == "Task Loop: test/1...foo/1...LOOP -> test/1");

        // loop with try()
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "test(?A) :- if(true), do( try(foo(?A)) ). \r\n"
                                "foo(?A) :- if(true), do( try(test(?A)) ). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK(loops.size() == 2);
        result = (*loops.begin());
        CHECK(result == "Task Loop: foo/1...test/1...LOOP -> foo/1");
        result = (*(++loops.begin()));
        CHECK( result == "Task Loop: test/1...foo/1...LOOP -> test/1");

        // Loop two levels deep with try
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "test(?A) :- if(true), do( try(foo(?A)) ). \r\n"
                                "foo(?A) :- if(true), do( try(bar(?A)) ). \r\n"
                                "bar(?A) :- if(true), do( try(test(?A)) ). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK(loops.size() == 3);
        result = (*loops.begin());
        CHECK( result == "Task Loop: bar/1...test/1...foo/1...LOOP -> bar/1");
        result = (*(++loops.begin()));
        CHECK(result == "Task Loop: foo/1...bar/1...test/1...LOOP -> foo/1");
        result = (*(++(++loops.begin())));
        CHECK(result == "Task Loop: test/1...foo/1...bar/1...LOOP -> test/1");

        // Loop in Axioms
        compiler->Clear();
        CHECK(compiler->Compile(string() +
                                "a1(A) :- a2(A). \r\n"
                                "a2(B) :- a1(B). \r\n"
                                "test(?A) :- if(true), do( try(foo(?A)) ). \r\n"
                                "foo(?A) :- if(true), do( ). \r\n"
                                ));
        loops = compiler->FindLogicErrors(planner->goalResolver()->shared_from_this());
        CHECK(loops.size() == 2);
        result = (*loops.begin());
        CHECK(result == "Rule Loop: a1/1...a2/1...LOOP -> a1/1");
        result = (*(++loops.begin()));
        CHECK( result == "Rule Loop: a2/1...a1/1...LOOP -> a2/1");
    }
}
