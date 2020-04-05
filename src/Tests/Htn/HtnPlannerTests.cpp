//
//  HtnPlannerTests.cpp
//  TestLib
//
//  Created by Eric Zinda on 1/8/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//
#include "AIHarness.h"
#include "Prolog/HtnGoalResolver.h"
#include "FXPlatform/Htn/HtnCompiler.h"
#include "FXPlatform/Htn/HtnPlanner.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTerm.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"
#include "Tests/ParserTestBase.h"
#include "Logger.h"
#include <thread>
#include "UnitTest++/UnitTest++.h"
using namespace Prolog;

SUITE(HtnPlannerTests)
{
    TEST(ErrorContextTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string finalFacts;
        string finalPlan;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        int64_t memoryUsed;
        int deepestFailureIndex;
        std::vector<std::shared_ptr<HtnTerm>> deepestFailureContext;
        string failureContext;
        
        // State true for all tests
        sharedState = string() +
        "";

        // no failure context
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "failInCriteria(?Value) :- if(false), do(trace(?Value))."
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "";
        goals = string() +
        "goals(failInCriteria(test)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals(), 5000000,
                                       &memoryUsed, &deepestFailureIndex, &deepestFailureContext);
        finalPlan = HtnPlanner::ToStringSolutions(result);
        failureContext = HtnTerm::ToString(deepestFailureContext);
        CHECK_EQUAL("null", finalPlan);
        CHECK_EQUAL("()", failureContext);
        
        // simplest case
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "failInCriteria(?Value) :- if(failureContext(tag, 1), false), do(trace(?Value))."
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "";
        goals = string() +
        "goals(failInCriteria(test)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals(), 5000000,
                                       &memoryUsed, &deepestFailureIndex, &deepestFailureContext);
        finalPlan = HtnPlanner::ToStringSolutions(result);
        failureContext = HtnTerm::ToString(deepestFailureContext);
        CHECK_EQUAL("null", finalPlan);
        CHECK_EQUAL("(tag, 1)", failureContext);
        
        // first criteria fails further in the criteria term list
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "failInCriteria(?Value) :- if(=(?X, 1), failureContext(tag, 1), failTask([1,2,3]) ), do(trace(?Value))."
        "failInCriteria(?Value) :- if(failureContext(tag, 2), failTask([1,2]) ), do(trace(?Value))."
        "trace(?Value) :- del(), add(?Value). \r\n"
        "failTask([]) :- false."
        "failTask([_|T]) :- failTask(T)."
        "";
        goals = string() +
        "goals(failInCriteria(test)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals(), 5000000,
                                       &memoryUsed, &deepestFailureIndex, &deepestFailureContext);
        finalPlan = HtnPlanner::ToStringSolutions(result);
        failureContext = HtnTerm::ToString(deepestFailureContext);
        CHECK_EQUAL("null", finalPlan);
        CHECK_EQUAL("(tag, 1)", failureContext);
        
        // second criteria fails further in the criteria term list
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "failInCriteria(?Value) :- if(failureContext(tag, 1), failTask([1,2]) ), do(trace(?Value))."
        "failInCriteria(?Value) :- if(=(?X, 1), failureContext(tag, 2), failTask([1,2,3]) ), do(trace(?Value))."
        "trace(?Value) :- del(), add(?Value). \r\n"
        "failTask([]) :- false."
        "failTask([_|T]) :- failTask(T)."
        "";
        goals = string() +
        "goals(failInCriteria(test)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals(), 5000000,
                                       &memoryUsed, &deepestFailureIndex, &deepestFailureContext);
        finalPlan = HtnPlanner::ToStringSolutions(result);
        failureContext = HtnTerm::ToString(deepestFailureContext);
        CHECK_EQUAL("null", finalPlan);
        CHECK_EQUAL("(tag, 2)", failureContext);
    }
    
    TEST(PlannerOperatorTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string finalFacts;
        string finalPlan;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
        // State true for all tests
        sharedState = string() +
        "";

        // No goals
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "";
        goals = string() +
        "goals().\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { () } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ {  } ]");

        // first operator doesn't unify
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "";
        goals = string() +
        "goals(trace(Test1, Test3), trace(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "null");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "null");

        // last operator doesn't unify
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "";
        goals = string() +
        "goals(trace(Test1), trace(Test2, Test3)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "null");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "null");

        // One successful operator
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "";
        goals = string() +
        "goals(trace(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { Test1 =>  } ]");

        // Just two successful operators
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "";
        goals = string() +
        "goals(trace(Test1), trace(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1), trace(Test2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { Test1 => ,Test2 =>  } ]");
    }

    TEST(PlannerNormalMethodTest)
    {
//                SetTraceFilter((int) SystemTraceType::Solver | (int) SystemTraceType::System, TraceDetail::Diagnostic);

        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string finalFacts;
        string finalPlan;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
        // State true for all tests
        sharedState = string() +
        "";

        // Single goal that does not match any methods
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "method(?Value) :- if(IsTrue(?Value)), do(trace(?Value)). \r\n" +
        "";
        goals = string() +
        "goals(method1(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "null");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "null");

        // Single method with one condition which does not resolve to ground
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "method(?Value) :- if(IsTrue(?Value)), do(trace(?Value)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "null");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "null");

        // Single method with one binding
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). \r\n" +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "method(?Value) :- if(IsTrue(?Value)), do(trace(?Value)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Test1 =>  } ]");

        // Two methods with single bindings: 2 separate answers
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). \r\n" +
        "trace(?Value, ?Value2) :- del(), add(?Value, ?Value2). \r\n" +
        "method(?Value) :- if(IsTrue(?Value)), do(trace(?Value, Method1)). \r\n" +
        "method(?Value) :- if(IsTrue(?Value)), do(trace(?Value, Method2)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,Method1)) } { (trace(Test1,Method2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Test1 => ,Method1 =>  } { IsTrue(Test1) => ,Test1 => ,Method2 =>  } ]");

        // Two methods with two condition bindings each: 4 separate answers
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(?Value, ?Value2). \r\n" +
        "method(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, Method1, ?Alt)). \r\n" +
        "method(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, Method2, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,Method1,Alternative1)) } { (trace(Test1,Method1,Alternative2)) } { (trace(Test1,Method2,Alternative1)) } { (trace(Test1,Method2,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,Test1 => ,Method1 =>  } { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,Test1 => ,Method1 =>  } { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,Test1 => ,Method2 =>  } { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,Test1 => ,Method2 =>  } ]");
        
        // ***** Variables should be properly scoped such that reusing the same name in different terms works (i.e. you can use X in the do() part of a method
        // and a different X in a different method and they will unify properly
        string example = string() +
        "test(?X) :- if(), do(successTask(10, ?X)) .\r\n" +
        "successTask(?X, ?Y) :- if(number(?X)), do(debugWatch(?X)).\r\n" +
        "debugWatch(?x) :- del(), add().\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "number(10).number(12).number(1). \r\n" +
        "goals(test(100)).";
        CHECK(compiler->Compile(example + testState));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(10)) } ]");
    }
    
    TEST(PlannerArithmeticTermsTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string finalFacts;
        string finalPlan;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
        // State true for all tests
        sharedState = string() +
        "";
        
        // Terms that are arithmetic are always automatically evaluated before attempting to match with operators and rules
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "trace(?Value, ?Value2) :- del(), add(?Value, ?Value2). \r\n" +
        "method(?Value) :- if(), do(trace(?Value, Method)). \r\n" +
        "";
        goals = string() +
        "goals(method(-(1,2))).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(-1,Method)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { -1 => ,Method =>  } ]");
    }
    
    TEST(PlannerSetOfTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string finalFacts;
        string finalPlan;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
        // State true for all tests
        sharedState = string() +
        "";

        // A AllSetOf method that modifies state which is in the condition
        // Since the AllSetOf condition alternatives are run at the beginning, all alternatives are still run
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "deleteTrueIfExists(?Value) :- if(IsTrue(?Value)), do(deleteTrue(?Value)). \r\n" +
        "deleteTrue(?Value) :- del(IsTrue(?Value)), add(). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(try(deleteTrueIfExists(?Value)), trace(?Value, Method1, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (deleteTrue(Test1), trace(Test1,Method1,Alternative1), trace(Test1,Method1,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Test1,Method1,Alternative1) => ,item(Test1,Method1,Alternative2) =>  } ]");

        // A single AllSetOf method with no condition (which means it will only get run once) where all subtasks succeed
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(), do(trace(?Value, Method1, None)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,Method1,None)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Test1,Method1,None) =>  } ]");

        // A single AllSetOf method where all subtasks succeed
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, Method1, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,Method1,Alternative1), trace(Test1,Method1,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Test1,Method1,Alternative1) => ,item(Test1,Method1,Alternative2) =>  } ]");

        // A single AllSetOf method that fails along with another method of the same name that is *not* setof that succeeds
        // The second method should get run
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(Test2), Alternative(?Alt)), do(trace(?Value, MethodAllOf, ?Alt)). \r\n" +
        "method(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, MethodNormal, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,MethodNormal,Alternative1)) } { (trace(Test1,MethodNormal,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Test1,MethodNormal,Alternative1) =>  } { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Test1,MethodNormal,Alternative2) =>  } ]");

        // A single AllSetOf method where all subtasks succeed preceeded and followed by an operator
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, Method1, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(trace(Finish, Finish1, Finish2), method(Test1), trace(Finish3, Finish4, Finish5)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Finish,Finish1,Finish2), trace(Test1,Method1,Alternative1), trace(Test1,Method1,Alternative2), trace(Finish3,Finish4,Finish5)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Finish,Finish1,Finish2) => ,item(Test1,Method1,Alternative1) => ,item(Test1,Method1,Alternative2) => ,item(Finish3,Finish4,Finish5) =>  } ]");

        // Two AllSetOf methods where all subtasks succeed
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, Method1, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,Method1,Alternative1), trace(Test1,Method1,Alternative2), trace(Test2,Method1,Alternative1), trace(Test2,Method1,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Test1,Method1,Alternative1) => ,item(Test1,Method1,Alternative2) => ,item(Test2,Method1,Alternative1) => ,item(Test2,Method1,Alternative2) =>  } ]");

        // One AllSetOf methods Followed by a normal method with two solutions
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method2(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Normal,Test2,Alternative1)) } { (trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Normal,Test2,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Normal,Test2,Alternative1) =>  } { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Normal,Test2,Alternative2) =>  } ]");

        // A single AllSetOf method where one subtasks fail
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).Combo(Test1,Alternative1).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(subtask(?Value, ?Alt)). \r\n" +
        "subtask(?Value1, ?Value2) :- if(Combo(?Value1, ?Value2)), do(trace(?Value1, ?Value2)).\r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "null");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "null");

        // Two methods of the same name, one is AllSetOf which partially succeeds before it fails and the other method succeeds
        // Testing that an AllSetOf is clearing out previously successful solutions if it eventually fails
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "IsTrue(Test1, Alternative1). \r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        // condition unifies two ways, one of which fails when method2 is called
        "method(?Value) :- allOf, if(Alternative(?Alt)), do(method2(?Value, ?Alt)). \r\n" +
        "method(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "method2(?Value, ?Alt) :- if(IsTrue(?Value, ?Alt)), do(trace(Method2, ?Value, ?Alt)).\r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Normal,Test1,Alternative1)) } { (trace(Normal,Test1,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,IsTrue(Test1,Alternative1) => ,item(Normal,Test1,Alternative1) =>  } { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,IsTrue(Test1,Alternative1) => ,item(Normal,Test1,Alternative2) =>  } ]");

        // One AllSetOf methods Followed by a normal method with two solutions where one fails
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "IsTrue(Test2, Alternative2). \r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(method3(?Value, ?Alt)). \r\n" +
        "method3(?Value, ?Alt) :- if(IsTrue(?Value, ?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method2(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Normal,Test2,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,IsTrue(Test2,Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Normal,Test2,Alternative2) =>  } ]");

        // A single AnySetOf method where all subtasks succeed
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- anyOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, Method1, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,Method1,Alternative1), trace(Test1,Method1,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Test1,Method1,Alternative1) => ,item(Test1,Method1,Alternative2) =>  } ]");

        // A single AnySetOf method where all subtasks succeed preceeded and followed by an operator
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- anyOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, Method1, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(trace(Finish, Finish1, Finish2), method(Test1), trace(Finish3, Finish4, Finish5)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Finish,Finish1,Finish2), trace(Test1,Method1,Alternative1), trace(Test1,Method1,Alternative2), trace(Finish3,Finish4,Finish5)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Finish,Finish1,Finish2) => ,item(Test1,Method1,Alternative1) => ,item(Test1,Method1,Alternative2) => ,item(Finish3,Finish4,Finish5) =>  } ]");

        // Two AnySetOf methods where all subtasks succeed
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- anyOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(?Value, Method1, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,Method1,Alternative1), trace(Test1,Method1,Alternative2), trace(Test2,Method1,Alternative1), trace(Test2,Method1,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Test1,Method1,Alternative1) => ,item(Test1,Method1,Alternative2) => ,item(Test2,Method1,Alternative1) => ,item(Test2,Method1,Alternative2) =>  } ]");

        // One AnySetOf methods Followed by a normal method with two solutions
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- anyOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method2(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Normal,Test2,Alternative1)) } { (trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Normal,Test2,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Normal,Test2,Alternative1) =>  } { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Normal,Test2,Alternative2) =>  } ]");

        // A single AnySetOf method where one subtasks fail
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). Alternative(Alternative1). Alternative(Alternative2).Combo(Test1,Alternative1).\r\n" +
        "trace(?Value, ?Value2) :- del(), add(item(?Value, ?Value2)). \r\n" +
        "method(?Value) :- anyOf, if(IsTrue(?Value), Alternative(?Alt)), do(subtask(?Value, ?Alt)). \r\n" +
        "subtask(?Value1, ?Value2) :- if(Combo(?Value1, ?Value2)), do(trace(?Value1, ?Value2)).\r\n" +
        "";
        goals = string() +
        "goals(method(Test1)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test1,Alternative1)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,Combo(Test1,Alternative1) => ,item(Test1,Alternative1) =>  } ]");

        // One AnySetOf methods Followed by a normal method with two solutions where one fails
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "IsTrue(Test2, Alternative2). \r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- anyOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(method3(?Value, ?Alt)). \r\n" +
        "method3(?Value, ?Alt) :- if(IsTrue(?Value, ?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method2(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Normal,Test2,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,IsTrue(Test2,Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Normal,Test2,Alternative2) =>  } ]");
    }
    
    TEST(PlannerSingleSolutionTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionType> result;
        string example;
        string finalFacts;
        string finalFacts2;
        string finalPlan;
        string finalPlan2;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
        // State true for all tests
        sharedState = string() +
        "";

        // Asking for Single solution from planner should still return all AnyOf and AllOf answers, but only first alternative
        // Single Solution, One AnySetOf methods Followed by a normal method with two solutions
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- anyOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method2(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindPlan(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolution(result);
        CHECK_EQUAL(finalPlan, "(trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Normal,Test2,Alternative1))");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Normal,Test2,Alternative1) => ");

        // Asking for Single solution from planner should still return all AnyOf and AllOf answers, but only first alternative
        // One AllSetOf methods Followed by a normal method with two solutions
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method2(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindPlan(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolution(result);
        CHECK_EQUAL(finalPlan, "(trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Normal,Test2,Alternative1))");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Normal,Test2,Alternative1) => ");
    }
    
    TEST(PlannerTryTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string example;
        string finalFacts;
        string finalPlan;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
        // State true for all tests
        sharedState = string() +
        "";

        // ***** Empty Try should work (for completeness)
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "number(10).number(12).number(1). \r\n" +
        "test() :- if(), do(try(successTask()), try(failTask()), try(failTask()), try(successTask(?Y)) ).\r\n" +
        "failTask() :- if(<(2, 1)), do(debugWatch(fail)).\r\n" +
        "successTask() :- if(<(1, 2)), do(debugWatch(success)).\r\n" +
        "successTask(?X) :- if(number(?X)), do(debugWatch(?X)).\r\n" +
        "debugWatch(?x) :- del(), add(item(?x)).\r\n" +
        "";
        goals = string() +
        "goals(try(), successTask()).\r\n" +
        "";;
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(success)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { number(10) => ,number(12) => ,number(1) => ,item(success) =>  } ]");

        // try() condition terms should return all successful solutions
        // method2(Test2) should return two alternatives
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(try(method2(Test2))).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Normal,Test2,Alternative1)) } { (trace(Normal,Test2,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Normal,Test2,Alternative1) =>  } { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Normal,Test2,Alternative2) =>  } ]");

        // try() condition followed by a failed normal condition should fail
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(try(method(Test3)), method(Test3)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "null");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "null");

        // try() condition terms should ignore failure and be transparent to success
        // method() is allOf and always fails
        // method2(Test2) should return two alternatives
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- allOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(try(method(Test3)), try(method2(Test2))).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Normal,Test2,Alternative1)) } { (trace(Normal,Test2,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Normal,Test2,Alternative1) =>  } { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,item(Normal,Test2,Alternative2) =>  } ]");

        // try() condition terms should ignore failure in the do() clause too
        // One AnySetOf methods Followed by a normal method with one solution which fails
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "IsTrue(Test1). IsTrue(Test2). Alternative(Alternative1). Alternative(Alternative2).\r\n" +
        "IsTrue(Test2, Alternative2). \r\n" +
        "trace(?Value, ?Value2, ?Value3) :- del(), add(item(?Value, ?Value2, ?Value3)). \r\n" +
        "method(?Value) :- anyOf, if(IsTrue(?Value), Alternative(?Alt)), do(trace(AllSet, ?Value, ?Alt)). \r\n" +
        "method2(?Value) :- if(IsTrue(?Value), Alternative(?Alt)), do(trace(Method2, ?Value, ?Alt), try(method3(?Value, ?Alt))). \r\n" +
        "method3(?Value, ?Alt) :- if(IsTrue(?Value, ?Alt)), do(trace(Normal, ?Value, ?Alt)). \r\n" +
        "";
        goals = string() +
        "goals(method(Test1), method2(Test2)).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Method2,Test2,Alternative1)) } { (trace(AllSet,Test1,Alternative1), trace(AllSet,Test1,Alternative2), trace(Method2,Test2,Alternative2), trace(Normal,Test2,Alternative2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,IsTrue(Test2,Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Method2,Test2,Alternative1) =>  } { IsTrue(Test1) => ,IsTrue(Test2) => ,Alternative(Alternative1) => ,Alternative(Alternative2) => ,IsTrue(Test2,Alternative2) => ,item(AllSet,Test1,Alternative1) => ,item(AllSet,Test1,Alternative2) => ,item(Method2,Test2,Alternative2) => ,item(Normal,Test2,Alternative2) =>  } ]");

        // ***** Try should ignore failures and continue other tasks.  Should properly return multiple solutions
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "number(10).number(12).number(1). \r\n" +
        "test() :- if(), do(try(successTask()), try(failTask()), try(failTask()), try(successTask(?Y)) ).\r\n" +
        "failTask() :- if(<(2, 1)), do(debugWatch(fail)).\r\n" +
        "successTask() :- if(<(1, 2)), do(debugWatch(success)).\r\n" +
        "successTask(?X) :- if(number(?X)), do(debugWatch(?X)).\r\n" +
        "debugWatch(?x) :- del(), add(item(?x)).\r\n" +
        "";
        goals = string() +
        "goals(test()).\r\n" +
        "";;
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(success), debugWatch(10)) } { (debugWatch(success), debugWatch(12)) } { (debugWatch(success), debugWatch(1)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { number(10) => ,number(12) => ,number(1) => ,item(success) => ,item(10) =>  } { number(10) => ,number(12) => ,number(1) => ,item(success) => ,item(12) =>  } { number(10) => ,number(12) => ,number(1) => ,item(success) => ,item(1) =>  } ]");
    }
    
    TEST(PlannerElseTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string example;
        string finalFacts;
        string finalPlan;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
        // State true for all tests
        sharedState = string() +
        "";

        // ***** If a subtask fails, the planner should backtrack and run the else clause of the task that spawned it
        example = string() +
        "test() :- if(), do(failTask()).\r\n" +
        "test() :- else, if(), do(success()).\r\n" +
        "failTask() :- if( <(2,1) ), do().\r\n" +
        "success() :- del(), add(item(success)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (success) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { item(success) =>  } ]");

        // ***** If a subtask of an allof task fails, the planner should backtrack and run the else clause of the task that spawned it
        example = string() +
        "test() :- allOf, if(), do(failTask()).\r\n" +
        "test() :- else, if(), do(success()).\r\n" +
        "failTask() :- if( <(2,1) ), do().\r\n" +
        "success() :- del(), add(item(success)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (success) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { item(success) =>  } ]");

        // ***** If ALL subtasks of an anyof task fails, the planner should backtrack and run the else clause of the task that spawned it
        example = string() +
        "test() :- anyOf, if(), do(failTask()).\r\n" +
        "test() :- else, if(), do(elseSuccess()).\r\n" +
        "failTask() :- if( <(2,1) ), do(success()).\r\n" +
        "success() :- del(), add(item(success)).\r\n" +
        "elseSuccess() :- del(), add(item(elseEuccess)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (elseSuccess) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { item(elseEuccess) =>  } ]");

        // ***** If an anyOf only has one subtask that succeeds it should only return one solution and not run the else clause
        example = string() +
        "test() :- anyOf, if(unit(?X)), do(success()).\r\n" +
        "test() :- else, if(), do(elseSuccess()).\r\n" +
        "success() :- del(), add(item(success)).\r\n" +
        "elseSuccess() :- del(), add(item(elseEuccess)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "unit(Queen). \r\n" +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (success) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { unit(Queen) => ,item(success) =>  } ]");

        // ***** If at least one subtask of an anyof task succeeds, the planner should NOT backtrack and run the else clause of the task that spawned it
        example = string() +
        "test() :- anyOf, if(unit(?X)), do(failTask(?X)).\r\n" +
        "test() :- else, if(), do(elseSuccess()).\r\n" +
        "failTask(?Unit) :- if( shouldWork(?Unit) ), do(success()).\r\n" +
        "success() :- del(), add(item(success)).\r\n" +
        "elseSuccess() :- del(), add(item(elseEuccess)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "unit(Queen). \r\n" +
        "unit(Worker). \r\n" +
        "shouldWork(Queen). \r\n" +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (success) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { unit(Queen) => ,unit(Worker) => ,shouldWork(Queen) => ,item(success) =>  } ]");
    }
    
    TEST(PlannerFirstSortByNot)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string finalFacts;
        string finalFacts2;
        string finalPlan;
        string finalPlan2;
        string example;
        string testState;
        string sharedState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        // ***** first: with terms after first()
        example = string() +
        "test() :- if( first(number(?A)), is(?B, +(?A, ?A)) ), do(debugWatch(?B)).\r\n" +
        "debugWatch(?x) :- del(), add(item(?x)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "number(10).number(12).number(1). \r\n" +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(20)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { number(10) => ,number(12) => ,number(1) => ,item(20) =>  } ]");

        // ***** sortBy
        example = string() +
        "test() :- if( sortBy(?A, <(number(?A))) ), do(debugWatch(?A)).\r\n" +
        "debugWatch(?x) :- del(), add(item(?x)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "number(10).number(12).number(0). \r\n" +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(0)) } { (debugWatch(10)) } { (debugWatch(12)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { number(10) => ,number(12) => ,number(0) => ,item(0) =>  } { number(10) => ,number(12) => ,number(0) => ,item(10) =>  } { number(10) => ,number(12) => ,number(0) => ,item(12) =>  } ]");

        // ***** sortBy
        example = string() +
        "test() :- if( sortBy(?A, <(number(?A))) ), do(debugWatch(?A)).\r\n" +
        "debugWatch(?x) :- del(), add(item(?x)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "number(10).number(12).number(0). \r\n" +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(0)) } { (debugWatch(10)) } { (debugWatch(12)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { number(10) => ,number(12) => ,number(0) => ,item(0) =>  } { number(10) => ,number(12) => ,number(0) => ,item(10) =>  } { number(10) => ,number(12) => ,number(0) => ,item(12) =>  } ]");

        // ***** using variables from head that are not in if
        example = string() +
        "test(?C) :- if( first( sortBy(?A, <(number(?A)))), is(?B, +(?A, ?A)) ), do(debugWatch(?C)).\r\n" +
        "debugWatch(?x) :- del(), add(item(?x)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "number(10).number(12).number(1). \r\n" +
        "goals(test(99)).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(99)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { number(10) => ,number(12) => ,number(1) => ,item(99) =>  } ]");

        // ***** first and sortby: with terms after
        example = string() +
        "test() :- if( first( sortBy(?A, <(number(?A)))), is(?B, +(?A, ?A)) ), do(debugWatch(?B)).\r\n" +
        "debugWatch(?x) :- del(), add(item(?x)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "number(10).number(12).number(1). \r\n" +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(2)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { number(10) => ,number(12) => ,number(1) => ,item(2) =>  } ]");

        // ***** Not
        example = string() +
        "test() :- if( person(?X), not(isFunny(?X)) ), do(debugWatch(?X)).\r\n" +
        "debugWatch(?x) :- del(), add(item(?x)).\r\n" +
        "";
        compiler->ClearWithNewRuleSet();
        testState = string() +
        "person(Jim).person(Mary).isFunny(Mary). \r\n" +
        "goals(test()).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (debugWatch(Jim)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { person(Jim) => ,person(Mary) => ,isFunny(Mary) => ,item(Jim) =>  } ]");
    }
    
    TEST(PlannerStateTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string example;
        string finalFacts;
        string finalFacts2;
        string finalPlan;
        string finalPlan2;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
        // State true for all tests
        sharedState = string() +
        "";
        
//        // No goals
//        compiler->ClearWithNewRuleSet();
//        testState = string() +
//        "trace(?Value) :- del(), add(?Value). \r\n" +
//        "";
//        goals = string() +
//        "goals().\r\n" +
//        "";
//        CHECK(compiler->Compile(sharedState + testState + goals));
//        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals());
//        finalPlan = HtnPlanner::ToStringSolutions(result);
//        CHECK_EQUAL(finalPlan, "null");
    }
    
    TEST(PlannerShopTransportationScenarioTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler;
        shared_ptr<HtnPlanner::SolutionsType> result;
        string finalFacts;
        string finalFacts2;
        string finalPlan;
        string finalPlan2;
        string example;
        string testState;
        string sharedState;
        string goals;
        
//                SetTraceFilter((int) SystemTraceType::Solver | (int) SystemTraceType::Planner, TraceDetail::Diagnostic);
//                SetTraceLevelOfDetail(TraceDetail::Diagnostic);
        
        // Original(ish) data from: http://www.cs.umd.edu/projects/shop/description.html
        //        ";;; To have enough money for a taxi, we need at least $1.50 + $1 for each mile to be traveled.\r\n" +
        //        "(:- (have-taxi-fare ?dist) ((have-cash ?m) (eval (>= ?m (+ 1.5 ?dist)))))\r\n" +
        //        ";;; We are within walking distance of our destination if the weather is good and the distance is 3 miles, or if the weather is bad and the distance is 1/2 mile.\r\n" +
        //        "(:- (walking-distance ?u ?v) ((weather-is good) (distance ?u ?v ?w) (eval (=< ?w 3))) ((distance ?u ?v ?w) (eval (=< ?w 0.5))))\r\n"  +
        //        "(:operator (!hail ?vehicle ?location) ( (at-taxi-stand ?vehicle ?location) ) ((at ?vehicle ?location)) )                       ;This is the operator for hailing a vehicle. It brings the vehicle to our current location.\r\n" +
        //        "(:operator (!wait-for ?bus ?location) () ((at ?bus ?location)))                            ;This is the operator for waiting for a bus. It brings the bus to our current location.\r\n" +
        //        "(:operator (!ride ?vehicle ?a ?b) ((at ?a) (at ?vehicle ?a)) ((at ?b) (at ?vehicle ?b)))   ;This is the operator for riding a vehicle to a location. It puts both us and the vehicle at that location.\r\n" +
        //        "(:operator (!set-cash ?old ?new) ((have-cash ?old)) ((have-cash ?new)))                    ;This is the operator for changing how much cash we have left.\r\n" +
        //        "(:operator (!walk ?here ?there) ((at ?here)) ((at ?there)))                                ;This is the operator for walking to a location. It puts us at that location.\r\n" +
        //        "(:method (pay-driver ?fare) ((have-cash ?m) (eval (>= ?m ?fare))) ((!set-cash ?m (- ?m ?fare)))) ;If we have enough money to pay the taxi driver, then we can pay the driver by subtracting the taxi fare from our cash-on-hand.\r\n" +
        //        "(:method (travel-to ?q) ((at ?p) (walking-distance ?p ?q)) ((!walk ?p ?q)))                ;If q is within walking distance, then one way to travel there is to walk there directly.\r\n" +
        //        "(:method (travel-to ?y) ((at ?x) (at-taxi-stand ?t ?x) (distance ?x ?y ?d) (have-taxi-fare ?d)) ((!hail ?t ?x) (!ride ?t ?x ?y) (pay-driver (+ 1.50 ?d))))" +
        //        "(:facts ((have-cash 50) (at home) (at-taxi-stand taxi1 home) (distance home work 5)) )\r\n" +
        //        "(:goals ((travel-to work)) )\r\n";
        
        // The fact that :first is used means that the prover follows all branches and returns them
        example = string() +
        // Axioms
        "have-taxi-fare(?distance) :- have-cash(?m), >=(?m, +(1.5, ?distance)). \r\n" +
        "walking-distance(?u,?v) :- weather-is(good), distance(?u,?v,?w), =<(?w, 3). \r\n"+
        "walking-distance(?u,?v) :- distance(?u,?v,?w), =<(?w, 0.5). \r\n"+
        // Methods
        "pay-driver(?fare) :- if(have-cash(?m), >=(?m, ?fare)), do(set-cash(?m, -(?m,?fare))). \r\n"+
        "travel-to(?q) :- if(at(?p), walking-distance(?p, ?q)), do(walk(?p, ?q)). \r\n"+
        // Use first() so we only hail one taxi
        "travel-to(?y) :- if(first(at(?x), at-taxi-stand(?t, ?x), distance(?x, ?y, ?d), have-taxi-fare(?d))), do(hail(?t,?x), ride(?t, ?x, ?y), pay-driver(+(1.50, ?d))). \r\n"+
        "travel-to(?y) :- if(at(?x), bus-route(?bus, ?x, ?y)), do(wait-for(?bus, ?x), pay-driver(1.00), ride(?bus, ?x, ?y)). \r\n"+
        // Operators
        "hail(?vehicle, ?location) :- del(), add(at(?vehicle, ?location)). \r\n"+
        "wait-for(?bus, ?location) :- del(), add(at(?bus, ?location)). \r\n"+
        "ride(?vehicle, ?a, ?b) :- del(at(?a), at(?vehicle, ?a)), add(at(?b), at(?vehicle, ?b)). \r\n"+
        "set-cash(?old, ?new) :- del(have-cash(?old)), add(have-cash(?new)). \r\n"+
        "walk(?here, ?there) :- del(at(?here)), add(at(?there)). \r\n"+
        "";
        
        // State for all tests
        sharedState = string() +
        "distance(downtown, park, 2). \r\n"+
        "distance(downtown, uptown, 8). \r\n"+
        "distance(downtown, suburb, 12). \r\n"+
        "at-taxi-stand(taxi1, downtown). \r\n"+
        "at-taxi-stand(taxi2, downtown). \r\n"+
        "bus-route(bus1, downtown, park). \r\n"+
        "bus-route(bus2, downtown, uptown). \r\n"+
        "bus-route(bus3, downtown, suburb). \r\n"+
        "";
        
        // **** Next Test
        state = shared_ptr<HtnRuleSet>(new HtnRuleSet()); planner->ClearAll();
        compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        
        // State for this test
        testState = string() +
        "at(downtown). \r\n" +
        "weather-is(good). \r\n" +
        "have-cash(12). \r\n" +
        "";
        
        // Goal for this test
        goals =  string() +
        "goals(travel-to(suburb)).\r\n" +
        "";
        
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), state, compiler->goals());
        CHECK(result->size() == 1);
        finalFacts = (*result)[0]->second->ToStringFacts();
        finalPlan = HtnTerm::ToString((*result)[0]->first);
        CHECK_EQUAL(finalPlan, "(wait-for(bus3,downtown), set-cash(12,11.000000000), ride(bus3,downtown,suburb))");
        CHECK_EQUAL(finalFacts,  "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,have-cash(11.000000000) => ,at(suburb) => ,at(bus3,suburb) => ");
                
        // **** Next Test
        state = shared_ptr<HtnRuleSet>(new HtnRuleSet()); planner->ClearAll();
        compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        
        // State for this test
        testState = string() +
        "at(downtown). \r\n" +
        "weather-is(good). \r\n" +
        "have-cash(0). \r\n" +
        "";
        
        // Goal for this test
        goals =  string() +
        "goals(travel-to(park)).\r\n" +
        "";
        
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), state, compiler->goals());
        CHECK(result->size() == 1);
        finalFacts = (*result)[0]->second->ToStringFacts();
        finalPlan = HtnTerm::ToString((*result)[0]->first);
        CHECK_EQUAL(finalPlan, "(walk(downtown,park))");
        CHECK_EQUAL(finalFacts,  "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,have-cash(0) => ,at(park) => ");
        
        
        
        // **** Next Test
        state = shared_ptr<HtnRuleSet>(new HtnRuleSet()); planner->ClearAll();
        compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        
        // State for this test
        testState = string() +
        "at(downtown). \r\n" +
        "have-cash(0). \r\n" +
        "";
        
        // Goal for this test
        goals =  string() +
        "goals(travel-to(park)).\r\n" +
        "";
        
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), state, compiler->goals());
        CHECK(result == nullptr); // can't afford a taxi and too far to walk
        
        
        // **** Next Test
        state = shared_ptr<HtnRuleSet>(new HtnRuleSet()); planner->ClearAll();
        compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        
        // State for this test
        testState = string() +
        "at(downtown). \r\n" +
        "weather-is(good). \r\n" +
        "have-cash(12). \r\n" +
        "";
        
        // Goal for this test
        goals =  string() +
        "goals(travel-to(park)).\r\n" +
        "";
        
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), state, compiler->goals());
        CHECK(result->size() == 3);
        finalFacts = (*result)[0]->second->ToStringFacts();
        finalPlan = HtnTerm::ToString((*result)[0]->first);
        finalFacts2 = (*result)[1]->second->ToStringFacts();
        finalPlan2 = HtnTerm::ToString((*result)[1]->first);
        string finalFacts3 = (*result)[2]->second->ToStringFacts();
        string finalPlan3 = HtnTerm::ToString((*result)[2]->first);
        CHECK_EQUAL(finalPlan, "(walk(downtown,park))");
        CHECK_EQUAL(finalFacts,  "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,have-cash(12) => ,at(park) => ");
        CHECK(finalPlan2 == "(hail(taxi1,downtown), ride(taxi1,downtown,park), set-cash(12,8.500000000))");
        CHECK(finalFacts2 == "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,at(park) => ,at(taxi1,park) => ,have-cash(8.500000000) => ");
        CHECK(finalPlan3 == "(wait-for(bus1,downtown), set-cash(12,11.000000000), ride(bus1,downtown,park))");
        CHECK(finalFacts3 == "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,have-cash(11.000000000) => ,at(park) => ,at(bus1,park) => ");
        
        
        // **** Next Test
        state = shared_ptr<HtnRuleSet>(new HtnRuleSet()); planner->ClearAll();
        compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        
        // State for this test
        testState = string() +
        "at(downtown). \r\n" +
        "weather-is(good). \r\n" +
        "have-cash(80). \r\n" +
        "";
        
        // Goal for this test
        goals =  string() +
        "goals(travel-to(park)).\r\n" +
        "";
        
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), state, compiler->goals());
        CHECK(result->size() == 3);
        finalFacts = (*result)[0]->second->ToStringFacts();
        finalPlan = HtnTerm::ToString((*result)[0]->first);
        finalFacts2 = (*result)[1]->second->ToStringFacts();
        finalPlan2 = HtnTerm::ToString((*result)[1]->first);
        finalFacts3 = (*result)[2]->second->ToStringFacts();
        finalPlan3 = HtnTerm::ToString((*result)[2]->first);
        CHECK_EQUAL(finalPlan, "(walk(downtown,park))");
        CHECK_EQUAL(finalFacts,  "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,have-cash(80) => ,at(park) => ");
        CHECK(finalPlan2 == "(hail(taxi1,downtown), ride(taxi1,downtown,park), set-cash(80,76.500000000))");
        CHECK(finalFacts2 == "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,at(park) => ,at(taxi1,park) => ,have-cash(76.500000000) => ");
        CHECK(finalPlan3 == "(wait-for(bus1,downtown), set-cash(80,79.000000000), ride(bus1,downtown,park))"  );
        CHECK(finalFacts3 == "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,have-cash(79.000000000) => ,at(park) => ,at(bus1,park) => ");
        
        
        
        // **** Next Test
        state = shared_ptr<HtnRuleSet>(new HtnRuleSet()); planner->ClearAll();
        compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        
        // State for this test
        testState = string() +
        "at(downtown). \r\n" +
        "weather-is(good). \r\n" +
        "have-cash(0). \r\n" +
        "";
        
        // Goal for this test
        goals =  string() +
        "goals(travel-to(uptown)).\r\n" +
        "";
        
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), state, compiler->goals());
        CHECK(result == nullptr); // can't afford a taxi or bus, and too far to walk
        
        
        // **** Next Test
        state = shared_ptr<HtnRuleSet>(new HtnRuleSet()); planner->ClearAll();
        compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        
        // State for this test
        testState = string() +
        "at(downtown). \r\n" +
        "weather-is(good). \r\n" +
        "have-cash(12). \r\n" +
        "";
        
        // Goal for this test
        goals =  string() +
        "goals(travel-to(uptown)).\r\n" +
        "";
        
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), state, compiler->goals());
        CHECK(result->size() == 2);
        finalFacts = (*result)[0]->second->ToStringFacts();
        finalPlan = HtnTerm::ToString((*result)[0]->first);
        finalFacts2 = (*result)[1]->second->ToStringFacts();
        finalPlan2 = HtnTerm::ToString((*result)[1]->first);
        CHECK_EQUAL(finalPlan, "(hail(taxi1,downtown), ride(taxi1,downtown,uptown), set-cash(12,2.500000000))");
        CHECK_EQUAL(finalFacts,  "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,at(uptown) => ,at(taxi1,uptown) => ,have-cash(2.500000000) => ");
        CHECK(finalPlan2 == "(wait-for(bus2,downtown), set-cash(12,11.000000000), ride(bus2,downtown,uptown))");
        CHECK(finalFacts2 == "distance(downtown,park,2) => ,distance(downtown,uptown,8) => ,distance(downtown,suburb,12) => ,at-taxi-stand(taxi1,downtown) => ,at-taxi-stand(taxi2,downtown) => ,bus-route(bus1,downtown,park) => ,bus-route(bus2,downtown,uptown) => ,bus-route(bus3,downtown,suburb) => ,weather-is(good) => ,have-cash(11.000000000) => ,at(uptown) => ,at(bus2,uptown) => ");
    }
    
    TEST(HtnPlannerMemoryBudgetTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());
        shared_ptr<HtnCompiler> compiler = shared_ptr<HtnCompiler>(new HtnCompiler(factory.get(), state.get(), planner.get()));
        shared_ptr<HtnPlanner::SolutionsType> result;
        string finalFacts;
        string finalPlan;
        string testState;
        string sharedState;
        string goals;
        shared_ptr<vector<UnifierType>> unifier;
        
//        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);

        // State true for all tests
        sharedState = string() +
        "";
        
        // Create a rule that allocates a BUNCH of memory and surely blows out the budget
        // Ensure that all the operators returned up to that point get returned and others don't
        compiler->ClearWithNewRuleSet();
        testState = string() +
        // Generates a sequence of numbers
        "gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).\r\n" +
        "gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).\r\n" +
        "blowBudget() :- if(gen(0, 10000,?S)), do(trace(SHOULDNEVERHAPPEN)).\r\n" +
        "trace(?Value) :- del(), add(?Value). \r\n" +
        "";
        goals = string() +
        "goals(trace(Test), blowBudget()).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals(), 5000);
        CHECK(factory->outOfMemory());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { Test =>  } ]");
        
        // Goals will return 3 solutions but the second one fails partway through, make sure we get all of the first and part of the second
        // solutions
        factory->outOfMemory(false);
        compiler->ClearWithNewRuleSet();
        testState = string() +
        // Generates a sequence of numbers
        "BlowBudget(Green). Color(Blue). Color(Green). Color(Red).\r\n" +
        "gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).\r\n" +
        "gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).\r\n" +
        "blowBudget() :- if(gen(0, 10000,?S)), do(trace(SHOULDNEVERHAPPEN)).\r\n" +
        "trace(?Value) :- del(), add(item(?Value)). \r\n" +
        "trace2(?Value, ?Value2) :- del(), add(item(?Value, ?Value2)). \r\n" +
        "sizeColors() :- if(Color(?X)), do(trace(?X), outcome(?X)).\r\n" +
        "outcome(?X) :- if(BlowBudget(?X)), do(blowBudget()).\r\n" +
        "outcome(?X) :- if(), do(trace2(SUCCESS, ?X)).\r\n" +
        "";
        goals = string() +
        "goals(trace(Test), sizeColors()).\r\n" +
        "";
        CHECK(compiler->Compile(sharedState + testState + goals));
        result = planner->FindAllPlans(factory.get(), compiler->compilerOwnedRuleSet(), compiler->goals(), 200000);
        CHECK(factory->outOfMemory());
        finalPlan = HtnPlanner::ToStringSolutions(result);
        CHECK_EQUAL(finalPlan, "[ { (trace(Test), trace(Blue), trace2(SUCCESS,Blue)) } { (trace(Test), trace(Green)) } ]");
        finalFacts = HtnPlanner::ToStringFacts(result);
        CHECK_EQUAL(finalFacts,  "[ { BlowBudget(Green) => ,Color(Blue) => ,Color(Green) => ,Color(Red) => ,item(Test) => ,item(Blue) => ,item(SUCCESS,Blue) =>  } { BlowBudget(Green) => ,Color(Blue) => ,Color(Green) => ,Color(Red) => ,item(Test) => ,item(Green) =>  } ]");
    }
}
