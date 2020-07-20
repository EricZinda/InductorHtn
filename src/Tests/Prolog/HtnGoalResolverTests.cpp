//
//  AITests.cpp
//  TestLib
//
//  Created by Eric Zinda on 9/25/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//
#include <iostream>
// #include "FXPlatform/FileStream.h"
#include "FXPlatform/Logger.h"
#include "FXPlatform/Prolog/HtnGoalResolver.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTerm.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"
#include "FXPlatform/Prolog/PrologParser.h"
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "Tests/ParserTestBase.h"
#include "UnitTest++/UnitTest++.h"
using namespace Prolog;

bool CheckSolution(UnifierType solution, UnifierType expectedSolution)
{
    StaticFailFastAssert(solution.size() == expectedSolution.size());
    for(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> expected : expectedSolution)
    {
        UnifierType::iterator solutionIter;
        for(solutionIter = solution.begin(); solutionIter != solution.end(); ++solutionIter)
        {
            if(*expected.first == *solutionIter->first && *expected.second == *solutionIter->second)
            {
                break;
            }
        }
        
        if(solutionIter != solution.end())
        {
            solution.erase(solutionIter);
            continue;
        }
        else
        {
            break;
        }
    }
    
    return(solution.size() == 0);
}

bool CheckSolutions(vector<UnifierType> solution, vector<UnifierType> expectedSolution)
{
    for(auto expected : expectedSolution)
    {
        bool found = false;
        for(vector<UnifierType>::iterator foundIter = solution.begin(); foundIter != solution.end(); ++foundIter)
        {
            if(CheckSolution(*foundIter, expected))
            {
                solution.erase(foundIter);
                found = true;
                break;
            }
        }
        
        if(!found)
        {
            return false;
        }
    }
    
    return solution.size() == 0;
}
    
SUITE(HtnGoalResolverTests)
{
//     TEST(AdventureScenarioTests)
//     {
// //        SetTraceFilter((int) SystemTraceType::Solver | (int) SystemTraceType::System, TraceDetail::Diagnostic);

//         HtnGoalResolver resolver;
//         shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
//         shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
//         shared_ptr<PrologStandardCompiler> compiler = shared_ptr<PrologStandardCompiler>(new PrologStandardCompiler(factory.get(), state.get()));
//         shared_ptr<vector<UnifierType>> unifier;
//         string testState;
//         string sharedState;
//         string goals;
//         string finalUnifier;
//         string example;
//         int64_t highestMemoryUsedReturn;
//         int furthestFailureIndex;
//         std::vector<std::shared_ptr<HtnTerm>> farthestFailureContext;

//         FileStream stream;
//         stream.Open("Adventure.pl", OpenExisting, Read);
//         example = stream.ReadAll();
//         testState = "";

//         compiler->Clear();
//         goals = "goals(=(CreateOrEval, create), d_noun(place, X9005), d_pronoun(you, X9006), d_loc_nonsp(E9004, X9006, X9005, CreateOrEval)).";
//         CHECK(compiler->Compile(example + sharedState + testState + goals));
//         unifier = compiler->SolveGoals(&resolver, 1000000, &highestMemoryUsedReturn, &furthestFailureIndex, &farthestFailureContext);
//         finalUnifier = HtnGoalResolver::ToString(unifier.get());
// //        int64_t factorySize = factory->dynamicSize();
// //        CHECK_EQUAL(finalUnifier, "((?CreateOrEval = create, ?E9001 = id9007, ?X9002 = diamond1, ?X9003 = player))");
// //        CHECK_EQUAL(0, furthestFailureIndex);
// //        CHECK_EQUAL(0, farthestFailureContext.size());
//     }
    
    TEST(GoalResolverFailureContextTest)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        shared_ptr<vector<UnifierType>> unifier;
        string testState;
        string sharedState;
        string goals;
        string finalUnifier;
        string example;
        int furthestFailureIndex;
        std::vector<std::shared_ptr<HtnTerm>> farthestFailureContext;

        example =
            "test(?X) :- tile(?X, 1)."
            "test2(?X) :- failureContext(1, foo), tile(?X, 1)."
            "test3(?X) :- failureContext(1, foo), tile(0, 1), failureContext(2, foo2), tile(?X, 1)."
            ;
        testState = "tile(0,0). tile(0,1). \r\n";

        // If there is no use of FailureContext, nothing is returned but index still works
        compiler->Clear();
        goals = "goals(test(0), test(1)).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        unifier = compiler->SolveGoals(&resolver, 1000000, nullptr, &furthestFailureIndex, &farthestFailureContext);
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        CHECK_EQUAL(1, furthestFailureIndex);
        CHECK_EQUAL(0, farthestFailureContext.size());

        // FailureContext is returned if used
        compiler->Clear();
        goals = "goals(test2(0), test2(1)).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        unifier = compiler->SolveGoals(&resolver, 1000000, nullptr, &furthestFailureIndex, &farthestFailureContext);
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        CHECK_EQUAL(1, furthestFailureIndex);
        CHECK_EQUAL(2, farthestFailureContext.size());
        CHECK_EQUAL("{\"1\":[]}, {\"foo\":[]}", HtnTerm::ToString(farthestFailureContext, false, true));
        
        // The highest FailureContext is returned in a function
        compiler->Clear();
        goals = "goals(test3(0), test3(1)).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        unifier = compiler->SolveGoals(&resolver, 1000000, nullptr, &furthestFailureIndex, &farthestFailureContext);
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        CHECK_EQUAL(1, furthestFailureIndex);
        CHECK_EQUAL(2, farthestFailureContext.size());
        CHECK_EQUAL("{\"2\":[]}, {\"foo2\":[]}", HtnTerm::ToString(farthestFailureContext, false, true));

        // FailureContext is still active if it isn't cleared
        compiler->Clear();
        goals = "goals(test3(0), test3(0), test(1)).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        unifier = compiler->SolveGoals(&resolver, 1000000, nullptr, &furthestFailureIndex, &farthestFailureContext);
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL("null", finalUnifier);
        CHECK_EQUAL(2, furthestFailureIndex);
        CHECK_EQUAL(2, farthestFailureContext.size());
        CHECK_EQUAL("{\"2\":[]}, {\"foo2\":[]}", HtnTerm::ToString(farthestFailureContext, false, true));
    }
    
    TEST(HtnGoalResolverSquareScenarioTest)
    {
//        SetTraceFilter((int) SystemTraceType::Solver | (int) SystemTraceType::System, TraceDetail::Diagnostic);

        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
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
        
        // Should only return tiles "in range" that actually exist on the map
        example = string() +
        "gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).\r\n" +
        "gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).\r\n" +
        // hLine and vLine create a set of tiles in a line vertically or horizontally
        "hLineTile(?X1,?X2,?Y,tile(?S,?T)) :- gen(?X1,?X2,?S), tile(?S,?Y), is(?T,?Y).\r\n" +
        "vLineTile(?X,?Y1,?Y2,tile(?S,?T)) :- gen(?Y1,?Y2,?T), tile(?X,?T), is(?S,?X).\r\n" +
        // Square generates a square by using the trick that Prolog unifies with ALL rules, so it will get all 4 rules, each representing an edge of the squre
        "square(?X,?Y,?R,tile(?S,?T)) :- is(?Y1, -(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).\r\n" +
        "square(?X,?Y,?R,tile(?S,?T)) :- is(?Y1, +(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).\r\n" +
        "square(?X,?Y,?R,tile(?S,?T)) :- is(?X1, -(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).\r\n" +
        "square(?X,?Y,?R,tile(?S,?T)) :- is(?X1, +(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).\r\n" +
        // attackRangeTiles returns the range of tiles around ?X and ?Y that are in attack range
        // attackRangeTiles uses the same trick as gen to iterate through a set of numbers, in this case min -> max radius.
        "attackRangeTiles(?Min,?Max,tile(?X,?Y),tile(?S,?T)) :- =<(?Min, ?Max), square(?X,?Y,?Min,tile(?S,?T)).\r\n" +
        "attackRangeTiles(?Min,?Max,tile(?X,?Y),tile(?S,?T)) :- =<(?Min, ?Max), is(?Min1, +(?Min, 1)), attackRangeTiles(?Min1,?Max,tile(?X,?Y),tile(?S,?T)).\r\n" +        "";

        compiler->Clear();
        testState = string() +
        "tile(0,0). tile(0,1). \r\n" +
        "goals(attackRangeTiles(1, 1, tile(0,0), ?X)).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = tile(0,1)))" );

        compiler->Clear();
        testState = string() +
        "tile(0,0). tile(0,1). tile(1,0).tile(1,1).\r\n" +
        "goals(attackRangeTiles(1, 1, tile(0,0), ?X)).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = tile(0,1)), (?X = tile(1,1)), (?X = tile(1,0)))");

        // Should work for multiple radii
        compiler->Clear();
        testState = string() +
        "tile(0,0).tile(1,0).tile(2,0).tile(3,0).tile(4,0).tile(5,0).tile(6,0).tile(7,0).\r\n" +
        "tile(0,1).tile(1,1).tile(2,1).tile(3,1).tile(4,1).tile(5,1).tile(6,1).tile(7,1).\r\n" +
        "tile(0,2).tile(1,2).tile(2,2).tile(3,2).tile(4,2).tile(5,2).tile(6,2).tile(7,2).\r\n" +
        "tile(0,3).tile(1,3).tile(2,3).tile(3,3).tile(4,3).tile(5,3).tile(6,3).tile(7,3).\r\n" +
        "tile(0,4).tile(1,4).tile(2,4).tile(3,4).tile(4,4).tile(5,4).tile(6,4).tile(7,4).\r\n" +
        "tile(0,5).tile(1,5).tile(2,5).tile(3,5).tile(4,5).tile(5,5).tile(6,5).tile(7,5).\r\n" +
        "tile(0,6).tile(1,6).tile(2,6).tile(3,6).tile(4,6).tile(5,6).tile(6,6).tile(7,6).\r\n" +
        "tile(0,7).tile(1,7).tile(2,7).tile(3,7).tile(4,7).tile(5,7).tile(6,7).tile(7,7).\r\n" +
        "goals(attackRangeTiles(1, 2, tile(0,0), ?X)).";
        CHECK(compiler->Compile(example + sharedState + testState + goals));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = tile(0,1)), (?X = tile(1,1)), (?X = tile(1,0)), (?X = tile(0,2)), (?X = tile(1,2)), (?X = tile(2,2)), (?X = tile(2,0)), (?X = tile(2,1)))");
    }
    
    TEST(HtnGoalResolverRecursionTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
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
        
        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
                
        // ***** recursive iterator to generate a sequence of numbers
        // https://stackoverflow.com/questions/12109558/simple-prolog-generator
        compiler->Clear();
        testState = string() +
        "gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).\r\n" +
        "gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).\r\n" +
        "hLineTile(?X1,?X2,?Y,tile(?S,?T)) :- gen(?X1,?X2,?S), is(?T,?Y).\r\n" +
        "vLineTile(?X,?Y1,?Y2,tile(?S,?T)) :- gen(?Y1,?Y2,?T), is(?S,?X).\r\n" +
        "square(?X,?Y,?R,tile(?S,?T)) :- is(?Y1, -(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).\r\n" +
        "square(?X,?Y,?R,tile(?S,?T)) :- is(?Y1, +(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).\r\n" +
        "square(?X,?Y,?R,tile(?S,?T)) :- is(?X1, -(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).\r\n" +
        "square(?X,?Y,?R,tile(?S,?T)) :- is(?X1, +(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).\r\n" +
        "attackRange(?Min,?Max,?X,?Y,tile(?S,?T)) :- =<(?Min, ?Max), square(?X,?Y,?Min,tile(?S,?T)).\r\n" +
        "attackRange(?Min,?Max,?X,?Y,tile(?S,?T)) :- =<(?Min, ?Max), is(?Min1, +(?Min, 1)), attackRange(?Min1,?Max,?X,?Y,tile(?S,?T)).\r\n" +
        "goals(attackRange(1,2,0,0,?Tile)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Tile = tile(-1,-1)), (?Tile = tile(0,-1)), (?Tile = tile(1,-1)), (?Tile = tile(-1,1)), (?Tile = tile(0,1)), (?Tile = tile(1,1)), (?Tile = tile(-1,0)), (?Tile = tile(1,0)), (?Tile = tile(-2,-2)), (?Tile = tile(-1,-2)), (?Tile = tile(0,-2)), (?Tile = tile(1,-2)), (?Tile = tile(2,-2)), (?Tile = tile(-2,2)), (?Tile = tile(-1,2)), (?Tile = tile(0,2)), (?Tile = tile(1,2)), (?Tile = tile(2,2)), (?Tile = tile(-2,-1)), (?Tile = tile(-2,0)), (?Tile = tile(-2,1)), (?Tile = tile(2,-1)), (?Tile = tile(2,0)), (?Tile = tile(2,1)))");
        
        // ***** recursive iterator to generate a sequence of numbers
        compiler->Clear();
        testState = string() +
        "gen(?Cur, ?Top, ?Cur) :- <(?Cur, ?Top). \r\n" +
        "gen(?Cur, ?Top, ?Next):- <(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).\r\n" +
        "goals(gen(0, 5, ?Num)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Num = 0), (?Num = 1), (?Num = 2), (?Num = 3), (?Num = 4))");
    }
    
    TEST(HtnGoalResolverUnifierTests)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        
        // Check obvious boundary cases
        CHECK(HtnGoalResolver::Unify(factory.get(),nullptr, nullptr) == nullptr);
        CHECK(HtnGoalResolver::Unify(factory.get(),nullptr, factory->CreateConstant("x")) == nullptr);
        CHECK(HtnGoalResolver::Unify(factory.get(),factory->CreateConstant("x"), nullptr) == nullptr);

        // ****** Examples from here: http://www.dai.ed.ac.uk/groups/ssp/bookpages/quickprolog/node12.html
        // Two constants of the same name have a valid, empty solution
        shared_ptr<UnifierType> solution = HtnGoalResolver::Unify(factory.get(),factory->CreateConstant("a"), factory->CreateConstant("a"));
        CHECK(CheckSolution(*solution, { } ));
        
        // Two constants of diff name, no solution
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateConstant("a"), factory->CreateConstant("b"));
        CHECK(solution == nullptr);
        
        // Unification instantiates a variable to an atom
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateVariable("X"), factory->CreateConstant("b"));
        CHECK(CheckSolution(*solution, { pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("X"), factory->CreateConstant("b")) } ));

        // woman(mia) and woman(X) unify because X can be set to mia which results in identical terms.
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateFunctor("woman", { factory->CreateConstant("mia") }),
                                        factory->CreateFunctor("woman", { factory->CreateVariable("X") }));
        CHECK(CheckSolution(*solution, { pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("X"), factory->CreateConstant("mia")) } ));

        // Two identical complex terms unify foo(a,b) = foo(a,b)
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateFunctor("foo", { factory->CreateConstant("a"), factory->CreateConstant("b") }),
                                        factory->CreateFunctor("foo", { factory->CreateConstant("a"), factory->CreateConstant("b") }));
        CHECK(CheckSolution(*solution, { } ));

        // Two complex terms unify if they are of the same arity, have the same principal functor and their arguments unify foo(a,b) = foo(X,Y)
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateFunctor("foo", { factory->CreateConstant("a"), factory->CreateConstant("b") }),
                                        factory->CreateFunctor("foo", { factory->CreateVariable("X"), factory->CreateVariable("Y") }));
        CHECK(CheckSolution(*solution, { pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("X"), factory->CreateConstant("a")),
                                    pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("Y"), factory->CreateConstant("b")) }));

        // Instantiation of variables may occur in either of the terms to be unified foo(a,Y) = foo(X,b)
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateFunctor("foo", { factory->CreateConstant("a"), factory->CreateVariable("Y") }),
                                        factory->CreateFunctor("foo", { factory->CreateVariable("X"), factory->CreateConstant("b") }));
        CHECK(CheckSolution(*solution, { pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("X"), factory->CreateConstant("a")),
                                    pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("Y"), factory->CreateConstant("b")) }));

        // In this case there is no unification because foo(X,X)  must have the same 1st and 2nd arguments foo(a,b) = foo(X,X)
        shared_ptr<HtnTerm>variable1 = factory->CreateVariable("X");
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateFunctor("foo", { factory->CreateConstant("a"), factory->CreateConstant("b") }),
                                        factory->CreateFunctor("foo", { variable1, variable1 }));
        CHECK(solution == nullptr);

        // Unification binds two differently named variables to a single, unique variable name
        shared_ptr<HtnTerm>variableX = factory->CreateVariable("X");
        shared_ptr<HtnTerm>variableY = factory->CreateVariable("Y");
        solution = HtnGoalResolver::Unify(factory.get(),variableX, variableY);
        CHECK(CheckSolution(*solution, { pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(variableX, variableY)}));

        // Occurs check stops recursion     father(X) = X
        variableX = factory->CreateVariable("X");
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateFunctor("father", { variableX }),
                                        variableX);
        CHECK(solution == nullptr);

        // f(g(X, h(X, b)), Z) and f(g(a, Z), Y) unifies to: {X=a,Z=h(a,b),Y=h(a,b)}
        variableX = factory->CreateVariable("X");
        variableY = factory->CreateVariable("Y");
        shared_ptr<HtnTerm>variableZ = factory->CreateVariable("Z");
        solution = HtnGoalResolver::Unify(factory.get(),factory->CreateFunctor("f",
                                                            {
                                                                factory->CreateFunctor("g",
                                                                                       {
                                                                                           variableX,
                                                                                           factory->CreateFunctor("h",
                                                                                                                  {
                                                                                                                      variableX,
                                                                                                                      factory->CreateConstant("b")
                                                                                                                  })
                                                                                       }),
                                                                variableZ
                                                            }),
                                               factory->CreateFunctor("f",
                                                             {
                                                                 factory->CreateFunctor("g",
                                                                                        {
                                                                                            factory->CreateConstant("a"),
                                                                                            variableZ
                                                                                        }),
                                                                 variableY
                                                             })
                                        );
        CHECK(CheckSolution(*solution, {
            pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("X"), factory->CreateConstant("a")),
            pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("Z"), factory->CreateFunctor("h",
                                                                                         {
                                                                                             factory->CreateConstant("a"),
                                                                                             factory->CreateConstant("b")
                                                                                         })),
            pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>>(factory->CreateVariable("Y"), factory->CreateFunctor("h",
                                                                                         {
                                                                                             factory->CreateConstant("a"),
                                                                                             factory->CreateConstant("b")
                                                                                         })),

            // [('='(X,Y),'='(X,abc)),[[X <-- abc, Y <-- abc]]].
            
    //        ['='(g(X),f(f(X))), failure].
    //        ['='(f(X,1),f(a(X))), failure].
    //        ['='(f(X,Y,X),f(a(X),a(Y),Y,2)), failure].
    //        ['='(f(A,B,C),f(g(B,B),g(C,C),g(D,D))),
    //         [[A <-- g(g(g(D,D),g(D,D)),g(g(D,D),g(D,D))),
    //           B <-- g(g(D,D),g(D,D)),
    //           C <-- g(D,D)]]].
        }));


        
        CHECK(true);
    }

    TEST(HtnGoalResolverSubstituteUnifiersTests)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnTerm>variableX = factory->CreateVariable("X");
        shared_ptr<HtnTerm>variableY = factory->CreateVariable("Y");
        shared_ptr<HtnTerm>variableZ = factory->CreateVariable("Z");
        
        // src: X = Y dest: Z = X answer: Z = Y
        shared_ptr<UnifierType> result = HtnGoalResolver::SubstituteUnifiers(factory.get(),
            { UnifierItemType(variableX,  variableY) },
            { UnifierItemType(variableZ,  variableX) });
        CheckSolution(*result, { UnifierItemType(variableZ,  variableY) });

        // src: X = foo(bar) dest: Y = goo(X) answer: Y = goo(foo(bar))
        result = HtnGoalResolver::SubstituteUnifiers(factory.get(),
                                                   { UnifierItemType(variableX,  factory->CreateFunctor("foo", { factory->CreateConstant("bar")})) },
                                                   { UnifierItemType(variableY,  factory->CreateFunctor("goo", { variableX})) });
        CheckSolution(*result, { UnifierItemType(variableY,  factory->CreateFunctor("goo", { factory->CreateFunctor("foo", { factory->CreateConstant("bar")})})) });


    }

    bool IsFalse(shared_ptr<vector<UnifierType>> result)
    {
        return result == nullptr;
    }
    
    bool IsTrue(shared_ptr<vector<UnifierType>> result)
    {
        return result != nullptr && result->size() == 1 && (*result)[0].size() == 0;
    }
    
    TEST(HtnGoalResolverArithmeticFunctionsTests)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnTerm>falseTerm = factory->CreateConstant("false");
        shared_ptr<HtnTerm>trueTerm = factory->CreateConstant("true");

        // Make sure basic type inferencing works
        CHECK(factory->CreateConstant("foo")->GetTermType() == HtnTermType::Atom);
        CHECK(factory->CreateConstant("1")->GetTermType() == HtnTermType::IntType);
        CHECK(factory->CreateConstant("1.1")->GetTermType() == HtnTermType::FloatType);
        CHECK(factory->CreateConstant("1.0")->GetTermType() == HtnTermType::FloatType);
        CHECK(factory->CreateConstant("1.")->GetTermType() == HtnTermType::FloatType);
        CHECK(factory->CreateConstant("1-1")->GetTermType() == HtnTermType::Atom);
        CHECK(factory->CreateConstant("1+1")->GetTermType() == HtnTermType::Atom);
        CHECK(factory->CreateVariable("foo")->GetTermType() == HtnTermType::Variable);
        CHECK(factory->CreateConstantFunctor("foo", { "bar" })->GetTermType() == HtnTermType::Compound);
        
        shared_ptr<HtnTerm> result = factory->CreateFunctor(">",
        {
            factory->CreateConstant("1"),
            factory->CreateConstant("2")
        });
        CHECK(*result->Eval(factory.get()) == *falseTerm);

        result = factory->CreateFunctor(">",
                                                 {
                                                     factory->CreateConstant("1"),
                                                     factory->CreateConstant("1")
                                                 });
        CHECK(*result->Eval(factory.get()) == *falseTerm);

        result = factory->CreateFunctor(">",
                                                 {
                                                     factory->CreateConstant("2"),
                                                     factory->CreateConstant("1")
                                                 });
        CHECK(*result->Eval(factory.get()) == *trueTerm);

        result = factory->CreateFunctor(">=",
                                        {
                                            factory->CreateConstant("-17"),
                                            factory->CreateConstant("0")
                                        });
        CHECK(*result->Eval(factory.get()) == *falseTerm);

        // Make sure conversions work
        shared_ptr<HtnTerm> result2;
        // float
        result = factory->CreateConstantFunctor("float", { "1.1" } );
        result2 = result->Eval(factory.get());
        CHECK(*result2 == *factory->CreateConstant("1.100000000"));

        result = factory->CreateConstantFunctor("float", { "-1.1" } );
        result2 = result->Eval(factory.get());
        CHECK(*result2 == *factory->CreateConstant("-1.100000000"));

        // integer
        result = factory->CreateConstantFunctor("integer", { "1.1" } );
        result2 = result->Eval(factory.get());
        CHECK(*result2 == *factory->CreateConstant("1"));

        result = factory->CreateConstantFunctor("integer", { "-1.1" } );
        result2 = result->Eval(factory.get());
        CHECK(*result2 == *factory->CreateConstant("-1"));

        // abs
        result = factory->CreateConstantFunctor("abs", { "1.1" } );
        result2 = result->Eval(factory.get());
        CHECK(*result2 == *factory->CreateConstant("1.100000000"));
        result = factory->CreateConstantFunctor("abs", { "-1.1" } );
        result2 = result->Eval(factory.get());
        CHECK(*result2 == *factory->CreateConstant("1.100000000"));
        
        // basic math
        result = factory->CreateFunctor("-", { factory->CreateConstant("-3"), factory->CreateConstant("13")  });
        result2 = result->Eval(factory.get());
        CHECK(*result2 == *factory->CreateConstant("-16"));
    }

    TEST(HtnGoalResolverUnifierOperatorTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** single =() goal that fails
        compiler->Clear();
        testState = string() +
        "trace(?x) :- .\r\n"
        "goals( =(mia, vincent) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single =() goal that succeeds
        compiler->Clear();
        testState = string() +
        "trace(?x) :- .\r\n"
        "goals( =(?X, vincent) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = vincent))");
        
        // ***** single =() goal that is preceeded and succeeded by more goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(c). capital(b). capital(a).\r\n" +
        "cost(c, 1). cost(b, 2). cost(a, 3).\r\n" +
        "highCost(3).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( letter(?X), =(?Y, ?X), cost(?Y, ?Cost) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = c, ?Y = c, ?Cost = 1), (?X = b, ?Y = b, ?Cost = 2), (?X = a, ?Y = a, ?Cost = 3))");
        
        // ***** make sure terms with variables can be unified, and the unifications work in both terms
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(c). capital(b). capital(a).\r\n" +
        "cost(c, 1). cost(b, 2). cost(a, 3).\r\n" +
        "highCost(3).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( =(?Y, letter(?X)), =(capital(?X), ?Z) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Y = letter(?X), ?Z = capital(?X)))");
    }
   
    TEST(HtnGoalResolverListTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;

//                SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);

        // ***** simplest positive case
        compiler->Clear();
        testState = string() +
            "split([?Head | ?Tail], ?Head, ?Tail). "
            "goals(split([a, b, c, d], ?Head, ?Tail)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Tail = [b,c,d], ?Head = a))");

        // ***** classic list rule: member/2
        compiler->Clear();
        testState = string() +
            "member(?X, [?X|_]).        % member(X, [Head|Tail]) is true if X = Head \r\n"
            "                         % that is, if X is the head of the list\r\n"
            "member(?X, [_|?Tail]) :-   % or if X is a member of Tail,\r\n"
            "  member(?X, ?Tail).       % ie. if member(X, Tail) is true.\r\n"
            "goals( member(a, [b, c, a, [d, e, f]]), not(member(d, [b, c, a, [d, e, f]])) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** classic list rule: append/3
        compiler->Clear();
        testState = string() +
            "append([], ?Ys, ?Ys)."
            "append([?X|?Xs], ?Ys, [?X|?Zs]) :- append(?Xs, ?Ys, ?Zs)."
            "goals( append(?ListLeft, ?ListRight, [a, b, c]) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?ListRight = [a,b,c], ?ListLeft = []), (?ListLeft = [a], ?ListRight = [b,c]), (?ListLeft = [a,b], ?ListRight = [c]), (?ListLeft = [a,b,c], ?ListRight = []))");

        // ***** classic list rule: reverse/2
        compiler->Clear();
        testState = string() +
        "append([], ?Ys, ?Ys)."
        "append([?X|?Xs], ?Ys, [?X|?Zs]) :- append(?Xs, ?Ys, ?Zs)."
        "reverse([],[])."
        "reverse([?X|?Xs],?YsX) :- reverse(?Xs,?Ys), append(?Ys,[?X],?YsX)."
        "goals( reverse([a, b, foo(a, [a, b, c])], ?X) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = [foo(a,[a,b,c]),b,a]))");

        // ***** class list rule: Len
        compiler->Clear();
        testState = string() +
        "len([], 0).\r\n"
        "len([_ | ?Tail], ?Length) :-\r\n"
        "    len(?Tail, ?Length1),\r\n"
        "    is(?Length, +(?Length1, 1)),!.\r\n"
        "goals( len([[], b, foo(a, [a, b, c])], ?X) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = 3))");

    }

    TEST(HtnGoalResolverAtomCharsTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;

        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);

        // ***** single atom_chars() goal that succeeds: variable on right
        // with unifiers on left and right to make sure they flow
        compiler->Clear();
        testState = string() +
            "goals(=(?X, pre), atom_chars(foo, ?List), =(?Y, ?X), =(?Z, ?List) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = pre, ?List = [f,o,o], ?Y = pre, ?Z = [f,o,o]))");
        
        // ***** single atom_chars() goal that succeeds: variable on left
        // with unifiers on left and right to make sure they flow
        compiler->Clear();
        testState = string() +
            "goals(=(?X, pre), atom_chars(?List, [f, o, o]), =(?Y, ?X), =(?Z, ?List) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = pre, ?List = foo, ?Y = pre, ?Z = foo))");
        
        // ***** single atom_chars() goal that succeeds: more advanced case
        // with unifiers on left and right to make sure they flow
        compiler->Clear();
        testState = string() +
            "goals(=(?X, pre), atom_chars(foo, [?FirstChar | _]), =(?Y, ?X), =(?Z, ?FirstChar) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = pre, ?FirstChar = f, ?Y = pre, ?Z = f))");
    }
    
    TEST(HtnGoalResolverAtomDowncaseTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;

        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);

        // ***** single downcase_atom() goal that succeeds
        compiler->Clear();
        testState = string() +
            "goals(downcase_atom('THIS IS A TEST', ?x) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?x = this is a test))");

        // ***** single downcase_atom() goal that is preceeded and succeeded by more goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
            "letter(C). letter(B). letter(A).\r\n" +
            "capital(c). capital(b). capital(a).\r\n" +
            "cost(c, 1). cost(ab, 2). cost(a, 3).\r\n" +
            "highCost(3).\r\n" +
            "trace(?x) :- .\r\n"
            "goals( letter(?X), downcase_atom(?X, ?Y), cost(?Y, ?Cost) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = C, ?Y = c, ?Cost = 1), (?X = A, ?Y = a, ?Cost = 3))");
    }

    TEST(HtnGoalResolverAtomConcatTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;

        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);

        // ***** single atom_concat() goal that succeeds
        compiler->Clear();
        testState = string() +
            "goals(atom_concat(a, b, ?x) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?x = ab))");

        // ***** single atom_concat() goal that is preceeded and succeeded by more goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
            "letter(c). letter(b). letter(a).\r\n" +
            "capital(c). capital(b). capital(a).\r\n" +
            "cost(c, 1). cost(ab, 2). cost(a, 3).\r\n" +
            "highCost(3).\r\n" +
            "trace(?x) :- .\r\n"
            "goals( letter(?X), atom_concat(?X, b, ?Y), cost(?Y, ?Cost) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = a, ?Y = ab, ?Cost = 2))");
    }

    TEST(HtnGoalResolverWriteTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
//        SetTraceFilter((int)SystemTraceType::Solver, TraceDetail::Diagnostic);

        // ***** Make sure string literals work
        compiler->Clear();
        testState = string() +
        "goals( write('Test \"of the emergency\"') ).\r\n";
        CHECK(compiler->Compile(testState));
        
        // Redirect cout to catch output
        std::stringstream out;
        std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
        std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
        
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        CHECK_EQUAL(out.str(), "Test \"of the emergency\"");
        std::cout.rdbuf(coutbuf); //reset to standard output again
        
        // ***** Make sure nl works
        compiler->Clear();
        testState = string() +
        "goals( nl ).\r\n";
        CHECK(compiler->Compile(testState));
        
        // Redirect cout to catch output
        out = stringstream();
        coutbuf = std::cout.rdbuf(); //save old buf
        std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
        
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        stringstream newline;
        newline << endl;
        CHECK_EQUAL(out.str(), newline.str());
        std::cout.rdbuf(coutbuf); //reset to standard output again

        // ***** Make sure writeln works
        compiler->Clear();
        testState = string() +
        "goals( writeln('test') ).\r\n";
        CHECK(compiler->Compile(testState));
        
        // Redirect cout to catch output
        out = stringstream();
        coutbuf = std::cout.rdbuf(); //save old buf
        std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
        
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        newline = stringstream();
        newline << "test" << endl;
        CHECK_EQUAL(out.str(), newline.str());
        std::cout.rdbuf(coutbuf); //reset to standard output again
        
        // ***** Make sure variables aren't unified
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, Name1). \r\n" +
        "itemsInBag(Name2, Name3). \r\n" +
        "goals( write(itemsInBag(?X)) ).\r\n";
        CHECK(compiler->Compile(testState));
        
        // Redirect cout to catch output
        out = stringstream();
        coutbuf = std::cout.rdbuf(); //save old buf
        std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
        
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        CHECK_EQUAL(out.str(), "itemsInBag(?orig*X)");
        std::cout.rdbuf(coutbuf); //reset to standard output again
    }
    
	TEST(HtnGoalResolverVariableTests)
	{
		HtnGoalResolver resolver;
		shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
		shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
		shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
		string testState;
		string goals;
		string finalUnifier;
		shared_ptr<vector<UnifierType>> unifier;

//        SetTraceFilter((int)SystemTraceType::Solver, TraceDetail::Diagnostic);
        // ***** Make sure the same variables in terms of a conjunction get mapped to the same renamed variables
        compiler->Clear();
        testState = string() +
        "name(Name1). \r\n" +
        "name(Name4). \r\n" +
        "itemsInBag(Name1, Name1). \r\n" +
        "itemsInBag(Name2, Name3). \r\n" +
        "goals( itemsInBag(?X, ?X), name(?X) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = Name1))");

		// ***** Make sure the same variables get mapped to the same renamed variables
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1, Name1). \r\n" +
			"itemsInBag(Name2, Name3). \r\n" +
			"goals( itemsInBag(?X, ?X) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?X = Name1))");

		// ***** dontcare variables match anything and aren't returned
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"rule(?X) :- itemsInBag(_), itemsInBag(?X)."
			"goals( rule(?X) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?X = Name1), (?X = Name2), (?X = Name1), (?X = Name2))");

		// ***** dontcare variables in a query aren't returned
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"rule(?X) :- itemsInBag(_), itemsInBag(?X)."
			"goals( rule(_) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((), (), (), ())");

        // ***** Don't care variables aren't mapped to be the same name, they are always different
        // Also: Need to work in initial goal
        compiler->Clear();
        testState = string() +
            "itemsInBag(Name1, Name1). \r\n" +
            "itemsInBag(Name2, Name3). \r\n" +
            "goals( itemsInBag(_, _) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((), ())");

        // Rules with don't care variables should be matched by goals with anything
        compiler->Clear();
        testState = string() +
            "test(_, _). \r\n" +
            "goals( test(a, b) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL("(())", finalUnifier);

        // Once a variable is renamed it shouldn't be again
        compiler->Clear();
        testState = string() +
        "valid(a, b)."
        "valid(a, c)."
        "valid(b, c)."
        "test(?X, ?Y) :- valid(?X, ?Y), test2(?X, ?Y)."
        "test2(?X, ?Y) :- valid(?X, b)."
        "goals( test(_, ?X) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL("((?X = b), (?X = c))", finalUnifier);
        
        // Rules with don't care variables should be matched by goals with don't care variables
        compiler->Clear();
        testState = string() +
            "test(_, _) :- test2(_, _). \r\n" +
            "test2(_, _). \r\n" +
            "goals( test(_, _) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL("(())", finalUnifier);
//
        // Rules with don't care variables should be matched by goals with anything
        compiler->Clear();
        testState = string() +
            "test(_, _) :- test2(_, _). \r\n" +
            "test2(_, _). \r\n" +
            "goals( test(a, b) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL("(())", finalUnifier);
	}
    
    TEST(HtnGoalResolverForAllTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int)SystemTraceType::Solver, TraceDetail::Diagnostic);

        std::stringstream out;
        std::stringstream expectedOut;
        std::streambuf *coutbuf;
        
        // Make sure we loop over all alternatives
        compiler->Clear();
        testState = string() +
        "item(a)." +
        "item(b)." +
        "has(item(a)).    "
        "has(item(b)).    "
        "rule(?A) :- has(?A), writeln(?A)."
        "goals( forall(item(?X), rule(item(?X))) ).\r\n";
        CHECK(compiler->Compile(testState));
        
        // Redirect cout to catch output
        out = std::stringstream();
        coutbuf = std::cout.rdbuf(); //save old buf
        std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
        
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        expectedOut = std::stringstream();
        expectedOut << "item(a)" << endl << "item(b)" << endl;
        CHECK_EQUAL(out.str(), expectedOut.str());
        std::cout.rdbuf(coutbuf); //reset to standard output again
        
        // Make sure we don't bind variables outside the foreach()
        compiler->Clear();
        testState = string() +
        "item(a)." +
        "item(b)." +
        "has(item(a)).    "
        "has(item(b)).    "
        "rule(?A) :- has(?A), writeln(?A)."
        "goals( forall( item(?X), rule(item(?X)) ), writeln(item(?X)) ).\r\n";
        CHECK(compiler->Compile(testState));
        
        // Redirect cout to catch output
        out = std::stringstream();
        coutbuf = std::cout.rdbuf(); //save old buf
        std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
        
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        expectedOut = std::stringstream();
        expectedOut << "item(a)" << endl << "item(b)" << endl << "item(?orig*X)" << endl;
        string temp = out.str();
        CHECK_EQUAL(out.str(), expectedOut.str());
        std::cout.rdbuf(coutbuf); //reset to standard output again
        
        
        // Make sure we stop immediately when we fail and don't backtrack over other solutions
        compiler->Clear();
        testState = string() +
        "item(a)." +
        "item(b)." +
        "has(item(b)).	"
        "rule(?A) :- has(?A), writeln(?A)."
        "goals( forall(item(?X), rule(item(?X))) ).\r\n";
        CHECK(compiler->Compile(testState));
        
        // Redirect cout to catch output
        out = std::stringstream();
        coutbuf = std::cout.rdbuf(); //save old buf
        std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
        
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        CHECK_EQUAL(out.str(), "");
        std::cout.rdbuf(coutbuf); //reset to standard output again
    }
    
    TEST(HtnGoalResolverAtomicTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;

        // ***** atomic(mia). should resolve to true
        compiler->Clear();
        testState = string() +
            "goals(atomic(mia)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** atomic(mia()). should resolve to true
        compiler->Clear();
        testState = string() +
            "goals(atomic(mia())).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** atomic(8). should resolve to true
        compiler->Clear();
        testState = string() +
            "goals(atomic(8)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** atomic(3.25). should resolve to true
        compiler->Clear();
        testState = string() +
            "goals(atomic(3.25)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** atomic(loves(vincent, mia)). should resolve to false
        compiler->Clear();
        testState = string() +
            "goals(atomic(loves(vincent, mia))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");

        // ***** atomic(?X) (unbound variable) should resolve to False
        compiler->Clear();
        testState = string() +
            "goals(atomic(?X)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");

        // ***** atomic(?X) (bound variable) should resolve to True
        compiler->Clear();
        testState = string() +
            "goals(=(?X, mia), atomic(?X)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = mia))");
    }

    TEST(HtnGoalResolverTrueFalseTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int)SystemTraceType::Solver, TraceDetail::Diagnostic);
        
        // ***** true should resolve to true
        compiler->Clear();
        testState = string() +
        "goals( true ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        
        // ***** false should resolve to false
        compiler->Clear();
        testState = string() +
        "goals( false ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
    }
    
	TEST(HtnGoalResolverCutTests)
	{
		HtnGoalResolver resolver;
		shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
		shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
		shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
		string testState;
		string goals;
		string finalUnifier;
		shared_ptr<vector<UnifierType>> unifier;

//        SetTraceFilter((int)SystemTraceType::Solver, TraceDetail::Diagnostic);

        // ***** multiple rules, fail before cut should run second rule like normal
        compiler->Clear();
        testState = string() +
        "rule(?X) :- itemsInBag(?X), !."
        "rule(?X) :- =(?X, good)."
        "trace(?x) :- .\r\n"
        "goals( rule(?X) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = good))");
        
		// ***** multiple rules, second rule doesn't run after cut 
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"rule(?X) :- itemsInBag(?X), !."
			"rule(?X) :- =(?X, Bad)."
			"trace(?x) :- .\r\n"
			"goals( rule(?X) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?X = Name1))");

		// ***** multiple rules, second rule doesn't run after cut, but backtracking AFTER cut still works 
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"itemsInPurse(lipstick). \r\n" +
			"itemsInPurse(tissues). \r\n" +
			"rule(?X, ?Y) :- itemsInBag(?X), !, itemsInPurse(?Y)."
			"rule(?X, ?Y) :- =(?X, Bad), =(?Y, Bad)."
			"trace(?x) :- .\r\n"
			"goals( rule(?X, ?Y) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?X = Name1, ?Y = lipstick), (?X = Name1, ?Y = tissues))");

		// Cut in zero argument built-in function that does standalone eval should work
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"goals( count(?Count, itemsInBag(?X), !) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?Count = 1))");

		// Cut in two argument built-in function that does standalone eval should work
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1, 5). \r\n" +
			"itemsInBag(Name2, 4). \r\n" +
			"goals( min(?Min, ?Size, itemsInBag(?X, ?Size), !) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?Min = 5))");

		// Two cuts works
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"itemsInPurse(lipstick). \r\n" +
			"itemsInPurse(tissues). \r\n" +
			"rule(?X, ?Y) :- itemsInBag(?X), !, itemsInPurse(?Y), !."
			"rule(?X, ?Y) :- =(?X, Bad), =(?Y, Bad)."
			"trace(?x) :- .\r\n"
			"goals( rule(?X, ?Y) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?X = Name1, ?Y = lipstick))");

		// Cuts at the begining of a rule work
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"itemsInPurse(lipstick). \r\n" +
			"itemsInPurse(tissues). \r\n" +
			"rule(?X, ?Y) :- itemsInBag(?X), itemsInPurse(?Y)."
			"rule(?X, ?Y) :- !."
			"rule(?X, ?Y) :- =(?X, Bad), =(?Y, Bad)."
			"trace(?x) :- .\r\n"
			"goals( rule(?X, ?Y) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?X = Name1, ?Y = lipstick), (?X = Name1, ?Y = tissues), (?X = Name2, ?Y = lipstick), (?X = Name2, ?Y = tissues), ())");
		
		// ***** works with initial goals
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"goals( itemsInBag(?X), ! ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?X = Name1))");
	}

	TEST(HtnGoalResolverAssertRetractTests)
	{
		HtnGoalResolver resolver;
		shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
		shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
		shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
		string testState;
		string goals;
		string finalUnifier;
		shared_ptr<vector<UnifierType>> unifier;

		//SetTraceFilter((int)SystemTraceType::Solver, TraceDetail::Diagnostic);

		// ***** single assert() goal 
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"trace(?x) :- .\r\n"
			"goals( assert(itemsInBag(Name3)), itemsInBag(?After) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?After = Name1), (?After = Name2), (?After = Name3))");
		// Should be permanently changed in the ruleset now
		CHECK(state->DebugHasRule("itemsInBag(Name3)", ""));

		// ***** single assert() goal with a variable that needs to be bound
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"rule(?X) :- assert(itemsInBag(?X))."
			"trace(?x) :- .\r\n"
			"goals( rule(Name3), itemsInBag(?After) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?After = Name1), (?After = Name2), (?After = Name3))");
		// Should be permanently changed in the ruleset now
		CHECK(state->DebugHasRule("itemsInBag(Name3)", ""));

		// ***** single retract() goal 
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"trace(?x) :- .\r\n"
			"goals( retract(itemsInBag(Name1)), itemsInBag(?After) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?After = Name2))");
		// Should be permanently changed in the ruleset now
		CHECK(!state->DebugHasRule("itemsInBag(Name1)", ""));

		// ***** single retract() goal with a variable that needs to be bound
		compiler->Clear();
		testState = string() +
			"itemsInBag(Name1). \r\n" +
			"itemsInBag(Name2). \r\n" +
			"rule(?X) :- retract(itemsInBag(?X))."
			"trace(?x) :- .\r\n"
			"goals( rule(Name1), itemsInBag(?After) ).\r\n";
		CHECK(compiler->Compile(testState));
		unifier = compiler->SolveGoals();
		finalUnifier = HtnGoalResolver::ToString(unifier.get());
		CHECK_EQUAL(finalUnifier, "((?After = Name2))");
		CHECK(!state->DebugHasRule("itemsInBag(Name1)", ""));

        // ***** single retractall() goal with a variable that needs to be bound
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1). \r\n" +
        "itemsInBag(Name2). \r\n" +
        "goals( retractall(itemsInBag(?X)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        CHECK(!state->DebugHasRule("itemsInBag(Name1)", ""));
        CHECK(!state->DebugHasRule("itemsInBag(Name2)", ""));

        // ***** single retract() goal with a fact that doesn't exist
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1). \r\n" +
        "itemsInBag(Name2). \r\n" +
        "goals( retract(itemsInBag(Name3)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
		// TODO: figure out how to make this work
		//// ***** 
		//compiler->Clear();
		//testState = string() +
		//	"itemsInBag(Name1). \r\n" +
		//	"itemsInBag(Name2). \r\n" +
		//	"trace(?x) :- .\r\n"
		//	"goals( retract(itemsInBag(X)), retract(itemsInBag(Name2)) ).\r\n";
		//CHECK(compiler->Compile(testState));
		//unifier = compiler->SolveGoals();
		//finalUnifier = HtnGoalResolver::ToString(unifier.get());
		//CHECK_EQUAL(finalUnifier, "((?After = Name2))");
	}


    TEST(HtnGoalResolverFindAllTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
//                SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);

        // ***** if there are no solutions, we get an empty list
        compiler->Clear();
        testState = string() +
        "child(martha,charlotte)."
        "child(charlotte,caroline)."
        "child(caroline,laura)."
        "child(laura,rose)."
        "descend(?X,?Y)  :-  child(?X,?Y)."
        "descend(?X,?Y)  :-  child(?X,?Z),"
        "                    descend(?Z,?Y)."
        "trace(?x) :- .\r\n"
        "goals( findall(?X,descend(rose,?X),?Z) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Z = []))");

        // ***** simple single variable scenario with solutions and an extra goal to make sure
        // unifiers flow
        compiler->Clear();
        testState = string() +
        "child(martha,charlotte)."
        "child(charlotte,caroline)."
        "child(caroline,laura)."
        "child(laura,rose)."
        "descend(?X,?Y)  :-  child(?X,?Y)."
        "descend(?X,?Y)  :-  child(?X,?Z),"
        "                    descend(?Z,?Y)."
        "trace(?x) :- .\r\n"
        "goals( child(charlotte, ?A), findall(?X,descend(martha,?X),?Z), child(?A, ?B) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?A = caroline, ?Z = [charlotte,caroline,laura,rose], ?B = laura))");

        // ***** complex template single variable scenario that succeeds
        compiler->Clear();
        testState = string() +
        "child(martha,charlotte)."
        "child(charlotte,caroline)."
        "child(caroline,laura)."
        "child(laura,rose)."
        "descend(?X,?Y)  :-  child(?X,?Y)."
        "descend(?X,?Y)  :-  child(?X,?Z),"
        "                    descend(?Z,?Y)."
        "trace(?x) :- .\r\n"
        "goals( findall(fromMartha(?X),descend(martha,?X),?Z) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Z = [fromMartha(charlotte),fromMartha(caroline),fromMartha(laura),fromMartha(rose)]))");

        // ***** simple template multi variable scenario that succeeds
        compiler->Clear();
        testState = string() +
        "child(martha,charlotte)."
        "child(charlotte,caroline)."
        "child(caroline,laura)."
        "child(laura,rose)."
        "descend(?X,?Y)  :-  child(?X,?Y)."
        "descend(?X,?Y)  :-  child(?X,?Z),"
        "                    descend(?Z,?Y)."
        "trace(?x) :- .\r\n"
        "goals( findall(?X,descend(?X,?Y),?Z) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Z = [martha,charlotte,caroline,laura,martha,martha,martha,charlotte,charlotte,caroline]))");

        
        // ***** last argument should be unified with
        compiler->Clear();
        testState = string() +
        "child(martha,charlotte)."
        "child(charlotte,caroline)."
        "child(caroline,laura)."
        "child(laura,rose)."
        "descend(?X,?Y)  :-  child(?X,?Y)."
        "descend(?X,?Y)  :-  child(?X,?Z),"
        "                    descend(?Z,?Y)."
        "trace(?x) :- .\r\n"
        "goals( findall(?X,descend(laura,?X),[?Z]) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Z = rose))");
        
        // ***** scenario
        // from: https://www.cpp.edu/~jrfisher/www/prolog_tutorial/2_15.html
        compiler->Clear();
        testState = string() +
        "edge(1,2)."
        "edge(1,4)."
        "edge(1,3)."
        "edge(2,3)."
        "edge(2,5)."
        "edge(3,4)."
        "edge(3,5)."
        "edge(4,5)."
        "member(?X, [?X|_])."
        "member(?X, [_|?Tail]) :-"
        "  member(?X, ?Tail)."
        "reverse([],[])."
        "reverse([?X|?Xs],?YsX) :- reverse(?Xs,?Ys), append(?Ys,[?X],?YsX)."
        "append([], ?Ys, ?Ys)."
        "append([?X|?Xs], ?Ys, [?X|?Zs]) :- append(?Xs, ?Ys, ?Zs)."
        "path(?A,?B,?Path) :-"
        "       travel(?A,?B,[?A],?Q),"
        "       reverse(?Q,?Path)."
        "connected(?X,?Y) :- edge(?X,?Y)."
        "connected(?X,?Y) :- edge(?Y,?X)."
        "travel(?A,?B,?P,[?B|?P]) :-"
        "       connected(?A,?B)."
        "travel(?A,?B,?Visited,?Path) :-"
        "       connected(?A,?C),"
        "       \\==(?C, ?B),"
        "       not(member(?C,?Visited)),"
        "       travel(?C,?B,[?C|?Visited],?Path).";
        goals = "goals( path(1, 2, ?Path) ).\r\n";
        CHECK(compiler->Compile(testState + goals));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Path = [1,2]), (?Path = [1,4,5,2]), (?Path = [1,4,5,3,2]), (?Path = [1,4,3,2]), (?Path = [1,4,3,5,2]), (?Path = [1,3,2]), (?Path = [1,3,4,5,2]), (?Path = [1,3,5,2]))");
    }
        
    TEST(HtnGoalResolverMinTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** single min() goal where all terms fail
        compiler->Clear();
        testState = string() +
        "trace(?x) :- .\r\n"
        "goals( min(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single min() goal where all terms succeed, but the wrong variable is used for totalling
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "trace(?x) :- .\r\n"
        "goals( min(?Total, ?NotThere, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single min() goal where all terms succeed, but the variable is not ground
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, ?Count) :- . \r\n" +
        "trace(?x) :- .\r\n" +
        "goals( min(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single min() goal where all terms succeed
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "trace(?x) :- .\r\n" +
        "goals( min(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Total = 1))");
        
        // ***** single min() goal that is preceeded and succeeded by more goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "countToString(1, One). \r\n"
        "trace(?x) :- .\r\n" +
        "goals( itemsInBag(Name1, ?X), min(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)), countToString(?X, ?Name) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = 1, ?Total = 1, ?Name = One))");
    }
    
    TEST(HtnGoalResolverMaxTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** single max() goal where all terms fail
        compiler->Clear();
        testState = string() +
        "trace(?x) :- .\r\n"
        "goals( max(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single max() goal where all terms succeed, but the wrong variable is used for totalling
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "trace(?x) :- .\r\n"
        "goals( max(?Total, ?NotThere, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single max() goal where all terms succeed, but the variable is not ground
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, ?Count) :- . \r\n" +
        "trace(?x) :- .\r\n" +
        "goals( max(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single max() goal where all terms succeed
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "trace(?x) :- .\r\n" +
        "goals( max(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Total = 2))");
        
        // ***** single max() goal that is preceeded and succeeded by more goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "countToString(1, One). \r\n"
        "trace(?x) :- .\r\n" +
        "goals( itemsInBag(Name1, ?X), max(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)), countToString(?X, ?Name) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = 1, ?Total = 2, ?Name = One))");
    }
    
    TEST(HtnGoalResolverSumTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** single sum() goal where all terms fail
        compiler->Clear();
        testState = string() +
        "trace(?x) :- .\r\n"
        "goals( sum(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");

        // ***** single sum() goal where all terms succeed, but the wrong variable is used for totalling
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "trace(?x) :- .\r\n"
        "goals( sum(?Total, ?NotThere, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single sum() goal where all terms succeed, but the variable is not ground
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, ?Count) :- . \r\n" +
        "trace(?x) :- .\r\n" +
        "goals( sum(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single sum() goal where all terms succeed
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "trace(?x) :- .\r\n" +
        "goals( sum(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Total = 3))");
        
        // ***** single sum() goal that is preceeded and succeeded by more goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "itemsInBag(Name1, 1). \r\n" +
        "itemsInBag(Name2, 2). \r\n" +
        "countToString(1, One). \r\n"
        "trace(?x) :- .\r\n" +
        "goals( itemsInBag(Name1, ?X), sum(?Total, ?ItemCount, itemsInBag(?Name, ?ItemCount)), countToString(?X, ?Name) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = 1, ?Total = 3, ?Name = One))");
    }
    
    TEST(HtnGoalResolverDistinctTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);

        // ***** no variables, no domain
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n"
        "test(_) :- letter(_).\r\n"
        "trace(?x) :- .\r\n"
        "goals( distinct(_, test(_)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        
        // ***** single variable, no domain
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( distinct(_, letter(?X)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = c), (?X = b), (?X = a))");
        
        // ***** multiple variables, no domain
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( distinct(_, letter(?X), letter(?Y)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = c, ?Y = c), (?X = c, ?Y = b), (?X = c, ?Y = a), (?X = b, ?Y = c), (?X = b, ?Y = b), (?X = b, ?Y = a), (?X = a, ?Y = c), (?X = a, ?Y = b), (?X = a, ?Y = a))");
    
        // ***** single variable, with domain
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( distinct(?X, letter(?X)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = c), (?X = b), (?X = a))");
        
        // ***** multiple variables (one unbound), with domain
        // which unbound gets chosen is indeterminate
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( distinct(?X, letter(?X), letter(?Y)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = c, ?Y = c), (?X = b, ?Y = c), (?X = a, ?Y = c))");
    }
    
    TEST(HtnGoalResolverCountTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** single count() goal where all items fail
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( count(?Count, capitol(?X)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Count = 0))");
        
        // ***** single count() goal where all items succeed
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(c). capital(b). capital(a).\r\n" +
        "cost(c, 1). cost(b, 2). cost(a, 3).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( count(?Count, letter(?X)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Count = 3))");
        
        // Make sure count() can be used in math
        // ***** single count() goal where all items succeed
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(c). capital(b). capital(a).\r\n" +
        "cost(c, 1). cost(b, 2). cost(a, 3).\r\n" +
        "trace(?x) :- .\r\n"
        "goals( count(?Count, letter(?X)), is(?Result, *(1, ?Count)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Count = 3, ?Result = 3))");
        
        // ***** single count() goal that is preceeded and succeeded by more goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(c). capital(b). capital(a).\r\n" +
        "cost(c, 1). cost(b, 2). cost(a, 3).\r\n" +
        "highCost(3).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(letter(?X), count(?Count, letter(?Y)), capital(?Z) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        // ?Y should not show up since it is in the count() clause
        CHECK_EQUAL(finalUnifier, "((?X = c, ?Count = 3, ?Z = c), (?X = c, ?Count = 3, ?Z = b), (?X = c, ?Count = 3, ?Z = a), (?X = b, ?Count = 3, ?Z = c), (?X = b, ?Count = 3, ?Z = b), (?X = b, ?Count = 3, ?Z = a), (?X = a, ?Count = 3, ?Z = c), (?X = a, ?Count = 3, ?Z = b), (?X = a, ?Count = 3, ?Z = a))");
    }
    
    TEST(HtnGoalResolverSortByTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** single sortBy() goal where all items fail
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(sortBy(?C, <(letter(?X), capital(?X), cost(?X, ?C)))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");

        // ***** single sortBy() goal where all items succeed
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(c). capital(b). capital(a).\r\n" +
        "cost(c, 1). cost(b, 2). cost(a, 3).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(sortBy(?C, <(letter(?X), capital(?X), cost(?X, ?C)))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = c, ?C = 1), (?X = b, ?C = 2), (?X = a, ?C = 3))");

        // ***** single sortBy() goal that is preceeded and succeeded by more goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(c). capital(b). capital(a).\r\n" +
        "cost(c, 1). cost(b, 2). cost(a, 3).\r\n" +
        "highCost(3).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(highCost(?HighCost), sortBy(?C, <(letter(?X), capital(?X), cost(?X, ?C))), highCost(?C)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?HighCost = 3, ?X = a, ?C = 3))");
    }
    
    TEST(HtnGoalResolverIdenticalTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
    
        // ***** single ==() goal that fails
        compiler->Clear();
        testState = string() +
        "goals(==(letter(a), letter(b))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");

        // ***** single ==() goal that succeeds
        compiler->Clear();
        testState = string() +
        "goals( ==(letter(a), letter(a)) ).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** single ==() goal with variables that are the same.
        // They aren't resolved so there are no variables in the unifier, just checked for being identical
        compiler->Clear();
        testState = string() +
        "letter(c). letter(B). letter(A).\r\n" +
        "capital(B). capital(A).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(==(letter(?X), letter(?X))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** single ==() goal with arithmetic terms.
        compiler->Clear();
        testState = string() +
        "letter(c). letter(B). letter(A).\r\n" +
        "capital(B). capital(A).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(==(0, 0)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** make sure variables that have been unified compare their values, not just the variable name
        // ***** single ==() goal preceeded and followed by other goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "letter(c). letter(B). letter(A).\r\n" +
        "capital(B). capital(A).\r\n" +
        "combo(A, X). combo(B, Y).\r\n"
        "trace(?x) :- .\r\n"
        "goals(capital(?Capital), letter(?X), ==(?X, ?Capital), combo(?X, ?Combo)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Capital = B, ?X = B, ?Combo = Y), (?Capital = A, ?X = A, ?Combo = X))");
        
        // ***** make sure variables that have been unified compare their values, not just the variable name
        // ***** single ==() goal preceeded and followed by other goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "letter(c). letter(B). letter(A).\r\n" +
        "capital(B). capital(A).\r\n" +
        "combo(A, ComboA). combo(B, ComboB).\r\n"
        "trace(?x) :- .\r\n"
        "goals(capital(?Capital), letter(?X), ==(?X, ?Capital), combo(?X, ?Combo)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Capital = B, ?X = B, ?Combo = ComboB), (?Capital = A, ?X = A, ?Combo = ComboA))");
        
        // ***** single \==() goal that fails
        compiler->Clear();
        testState = string() +
        "goals(\\==(letter(a), letter(a))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single \==() goal that succeeds
        compiler->Clear();
        testState = string() +
        "goals(\\==(letter(a), letter(b))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        
        // ***** single \==() goal preceeded and followed by other goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "letter(c). letter(B). letter(A).\r\n" +
        "capital(B). capital(A).\r\n" +
        "combo(A, ComboA). combo(B, ComboB).\r\n"
        "trace(?x) :- .\r\n"
        "goals(capital(?Capital), \\==(letter(?Capital), letter(B)), combo(?Capital, ?Combo)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Capital = A, ?Combo = ComboA))");
    }
    
    TEST(HtnGoalResolverIsTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** is/2 with two arithmetic arguments succeeds
        compiler->Clear();
        testState = string() +
        "goals(is(1, 1)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        compiler->Clear();
        testState = string() +
        "goals(is(+(1, 1), +(0, 2))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");

        // ***** is/2 with variable lvalue and arithmetic argument unifies
        compiler->Clear();
        testState = string() +
        "goals(is(?X, 1)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = 1))");

        compiler->Clear();
        testState = string() +
        "goals(is(?X, +(1,2))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = 3))");

        // ***** is/2 with variable lvalue that has been set and arithmetic argument works
        compiler->Clear();
        testState = string() +
        "goals(=(?X, 5), is(?X, 5)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = 5))");
        
        // ***** is/2 with variable lvalue that has been set with non-arithmetic term and arithmetic argument fails
        // but doesn't throw
        compiler->Clear();
        testState = string() +
        "goals(=(?X, a), is(?X, 5)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");

        // ***** is/2 with non-arithmetic arguments throws
        bool caught = false;
        try {
            compiler->Clear();
            testState = string() +
            "goals(is(b, b)).\r\n";
            CHECK(compiler->Compile(testState));
            unifier = compiler->SolveGoals();
            finalUnifier = HtnGoalResolver::ToString(unifier.get());
            CHECK_EQUAL(finalUnifier, "null");
        }
        catch(...)
        {
            caught = true;
        }
        CHECK(caught);
    }
    
    TEST(HtnGoalResolverNotTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** single not() goal that fails
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(not(letter(a))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "null");
        
        // ***** single not() goal that succeeds
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(not(letter(d))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "(())");
        
        // ***** single not() goal preceeded and followed by other goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(A)." +
        "trace(?x) :- .\r\n" +
        "goals(capital(?Capital), not(letter(d)), letter(?y)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Capital = A, ?y = c), (?Capital = A, ?y = b), (?Capital = A, ?y = a))");
        
        // ***** single not() goal filled with substitutions followed by other goals with multiple solutions
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "option(c). option(d). option(e).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(option(?x), not(letter(?x)), letter(?y)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?x = d, ?y = c), (?x = d, ?y = b), (?x = d, ?y = a), (?x = e, ?y = c), (?x = e, ?y = b), (?x = e, ?y = a))");
    }
    
    TEST(HtnGoalResolverFirstTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** single first() goal
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "trace(?x) :- .\r\n"
        "goals(first(letter(?x))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?x = c))");

        // ***** single first() goal preceeded and followed by other goals to make sure unifiers flow through properly
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(A)." +
        "trace(?x) :- .\r\n"
        "goals(capital(?Capital), first(letter(?x)), letter(?y)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Capital = A, ?x = c, ?y = c), (?Capital = A, ?x = c, ?y = b), (?Capital = A, ?x = c, ?y = a))");
        
        // Two firsts inside each other
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(a).\r\n" +
        "capital(A)." +
        "trace(?x) :- .\r\n"
        "goals(first(capital(?Capital), first(letter(?x)), letter(?y))).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?Capital = A, ?x = c, ?y = c))");
    }
    
    TEST(HtnGoalResolverPrintTests)
    {
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        string testState;
        string goals;
        string finalUnifier;
        shared_ptr<vector<UnifierType>> unifier;
        
        //        SetTraceFilter((int) SystemTraceType::Planner | (int)SystemTraceType::Solver | (int) SystemTraceType::Unifier,  TraceDetail::Diagnostic);
//        SetTraceFilter( (int)SystemTraceType::Solver,  TraceDetail::Diagnostic);
        
        // ***** print goal with previous and next terms
        compiler->Clear();
        testState = string() +
        "letter(c). letter(b). letter(A).\r\n" +
        "capital(A)." +
        "trace(?x) :- .\r\n"
        "goals(letter(?X), print(?X), capital(?X)).\r\n";
        CHECK(compiler->Compile(testState));
        unifier = compiler->SolveGoals();
        finalUnifier = HtnGoalResolver::ToString(unifier.get());
        CHECK_EQUAL(finalUnifier, "((?X = A))");
    }
    
    TEST(HtnGoalResolverFatalErrors)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> prog = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnGoalResolver> resolver = shared_ptr<HtnGoalResolver>(new HtnGoalResolver());
        shared_ptr<vector<UnifierType>> result;

        // Program(sunny) Query(X) -> fail since X cannot be a query
        prog->ClearAll();
        prog->AddRule(factory->CreateConstant("sunny"), {});
        bool caught = false;
        try
        {
            result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateVariable("X") });
        }
        catch(...)
        {
            caught = true;
        }
        CHECK(caught);
    }
    
    TEST(HtnGoalResolverResolveTests)
    {
//        SetTraceFilter(SystemTraceType::Solver, TraceDetail::Diagnostic);

        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> prog = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnGoalResolver> resolver = shared_ptr<HtnGoalResolver>(new HtnGoalResolver());
        shared_ptr<vector<UnifierType>> result;
        
        shared_ptr<HtnTerm>variableX = factory->CreateVariable("X");
        shared_ptr<HtnTerm>variableY = factory->CreateVariable("Y");
        shared_ptr<HtnTerm>variableZ = factory->CreateVariable("Z");

        // Check the base cases where the query is just a fact
        // Program(sunny, man(billy))
        // Queries:
        //      sunny
        //      man(billy)
        // Should return non-null but empty (which means true)
        prog->AddRule(factory->CreateConstant("sunny"), {});
        prog->AddRule(factory->CreateFunctor("man", {factory->CreateConstant("billy")}), {});
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateConstant("sunny") });
        CHECK(IsTrue(result));
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateFunctor("man", {factory->CreateConstant("billy")}) });
        CHECK(IsTrue(result));

        // Program(weather(sunny)) Query(weather(X)) -> X = sunny
        prog->ClearAll();
        prog->AddRule(factory->CreateFunctor("weather", {factory->CreateConstant("sunny")}), {});
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateFunctor("weather", { variableX }) } );
        CHECK(CheckSolutions(*result, { { UnifierItemType(variableX,  factory->CreateConstant("sunny"))} }));

        // Program(weather(sunny), weather(rainy)) Query(weather(X)) -> X = sunny; X = rainy
        prog->AddRule(factory->CreateFunctor("weather", {factory->CreateConstant("rainy")}), {});
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateFunctor("weather", { variableX }) } );
        CHECK(CheckSolutions(*result, {
            { UnifierItemType(variableX,  factory->CreateConstant("sunny"))},
            { UnifierItemType(variableX,  factory->CreateConstant("rainy"))}
        }));

        // mortal(X) :- human(X)
        // human(socrates)
        // ? mortal(socrates)
        // => true
        prog->ClearAll();
        prog->AddRule(factory->CreateFunctor("mortal", {factory->CreateVariable("X") }), { factory->CreateFunctor("human", {factory->CreateVariable("X") }) });
        prog->AddRule(factory->CreateFunctor("human", {factory->CreateConstant("socrates") }), { });
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateFunctor("mortal", {factory->CreateConstant("socrates") })});
        CHECK(IsTrue(result));

        // mortal(X) :- human(X)
        // human(socrates)
        // ? mortal(X)
        // => Y = socrates
        prog->ClearAll();
        prog->AddRule(factory->CreateFunctor("mortal", {factory->CreateVariable("X") }), { factory->CreateFunctor("human", {factory->CreateVariable("X") }) });
        prog->AddRule(factory->CreateFunctor("human", {factory->CreateConstant("socrates") }), { });
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateFunctor("mortal", { variableX })});
        CHECK(CheckSolutions(*result, {
            { UnifierItemType(variableX,  factory->CreateConstant("socrates")) }
        }));

        //        doubleTerm(X, Y) :- term(Y)
        //        doubleTerm(a, b)
        //        term(c)
        //        Query: doubleTerm(a, X)
        //        Expected: X = b, X = c
        prog->ClearAll();
        prog->AddRule(factory->CreateFunctor("doubleTerm", { variableX, variableY }),
                      { factory->CreateFunctor("term", { variableY }) });
        prog->AddRule(factory->CreateFunctor("doubleTerm", { factory->CreateConstant("a"), factory->CreateConstant("b") }), {});
        prog->AddRule(factory->CreateFunctor("term", { factory->CreateConstant("c") }), {});
        result = resolver->ResolveAll(factory.get(), prog.get(), {factory->CreateFunctor("doubleTerm", { factory->CreateConstant("a"), variableX })});
        CHECK(CheckSolutions(*result, {
            { UnifierItemType(variableX,  factory->CreateConstant("b")) },
            { UnifierItemType(variableX,  factory->CreateConstant("c")) }
        }));

        //        grandMotherOf(X,Z) :-
        //            motherOf(X,Y),
        //            motherOf(Y,Z).
        //        motherOf(tom,judy).
        //        motherOf(judy,mary).
        //        Query: grandMotherOf(tom, X)
        prog->ClearAll();
        prog->AddRule(factory->CreateFunctor("grandMotherOf", { variableX, variableZ }),
                      { factory->CreateFunctor("motherOf", { variableX, variableY }) ,
                          factory->CreateFunctor("motherOf", { variableY, variableZ }) });
        prog->AddRule(factory->CreateFunctor("motherOf", { factory->CreateConstant("tom"), factory->CreateConstant("judy") }), {});
        prog->AddRule(factory->CreateFunctor("motherOf", { factory->CreateConstant("judy"), factory->CreateConstant("mary") }), {});
        result = resolver->ResolveAll(factory.get(), prog.get(), {factory->CreateFunctor("grandMotherOf", { factory->CreateConstant("tom"), variableX })});
        CHECK(CheckSolutions(*result, {
            { UnifierItemType(variableX,  factory->CreateConstant("mary")) }
        }));
    
        // Test built-in print
        prog->ClearAll();
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateFunctor("print", { factory->CreateConstant("2"), factory->CreateConstant("1") })});
        CHECK(IsTrue(result));

        // Test arithmetic functions
        // X < Y
        // Query: greater(2, 1)
        prog->ClearAll();
        prog->AddRule(factory->CreateFunctor("greater", { variableX, variableY }), { factory->CreateFunctor(">", { variableX, variableY }) });
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateFunctor("greater", { factory->CreateConstant("2"), factory->CreateConstant("1") })});
        CHECK(IsTrue(result));

        // Test recursion, the "is" statement and clauses and arithmetic
        // factorial(0,1).
        // factorial(N,F) :-
        //    N>0,
        //    N1 is N-1,
        //    factorial(N1,F1),
        //    F is N * F1.
        prog->ClearAll();
        prog->AddRule(factory->CreateFunctor("factorial", { factory->CreateConstant("0"), factory->CreateConstant("1") }), {});
        shared_ptr<HtnTerm>term1 = factory->CreateFunctor(">", { factory->CreateVariable("N"), factory->CreateConstant("0") });
        shared_ptr<HtnTerm>term2 = factory->CreateFunctor("is", { factory->CreateVariable("N1"),
                                                       factory->CreateFunctor("-", { factory->CreateVariable("N"), factory->CreateConstant("1") }) });
        shared_ptr<HtnTerm>term3 = factory->CreateFunctor("factorial", { factory->CreateVariable("N1"), factory->CreateVariable("F1") });
        shared_ptr<HtnTerm>term4 = factory->CreateFunctor("is", { factory->CreateVariable("F"),
            factory->CreateFunctor("-", { factory->CreateVariable("N"), factory->CreateVariable("F1") }) });
        prog->AddRule(factory->CreateFunctor("factorial", { factory->CreateVariable("N"), factory->CreateVariable("F") }),
        {
            term1, term2, term3, term4
        });
        result = resolver->ResolveAll(factory.get(), prog.get(), { factory->CreateFunctor("factorial", { factory->CreateConstant("2"), factory->CreateVariable("X") })});
        CHECK(CheckSolutions(*result, {{ UnifierItemType(variableX, factory->CreateConstant("2")) }}));

        //        vertical(line(point(X,Y),point(X,Z))).
        //        horizontal(line(point(X,Y),point(Z,Y))).
        prog->ClearAll();
        shared_ptr<HtnTerm>pointXY = factory->CreateFunctor("point", { variableX, variableY });
        shared_ptr<HtnTerm>pointXZ = factory->CreateFunctor("point", { variableX, variableZ });
        shared_ptr<HtnTerm>pointZY = factory->CreateFunctor("point", { variableZ, variableY });
        prog->AddRule(factory->CreateFunctor("vertical", { factory->CreateFunctor("line", { pointXY, pointXZ }) }), {});
        prog->AddRule(factory->CreateFunctor("horizontal", { factory->CreateFunctor("line", { pointXY, pointZY }) }), {});
        // vertical(line(point(1,1),point(1,3))) => true
        result = resolver->ResolveAll(factory.get(), prog.get(),
        {
            factory->CreateFunctor("vertical",
                                   {
                                       factory->CreateFunctor("line",
                                                              {
                                                                  factory->CreateFunctor("point", { factory->CreateConstant("1"), factory->CreateConstant("1") }),
                                                                  factory->CreateFunctor("point", { factory->CreateConstant("1"), factory->CreateConstant("3") })
                                                              })
                                   })
        });
        CHECK(IsTrue(result));

        // horizontal(line(point(1,1),point(2,Y))).
        result = resolver->ResolveAll(factory.get(), prog.get(),
                                 {
                                     factory->CreateFunctor("horizontal",
                                                            {
                                                                factory->CreateFunctor("line",
                                                                                       {
                                                                                           factory->CreateFunctor("point", { factory->CreateConstant("1"), factory->CreateConstant("1") }),
                                                                                           factory->CreateFunctor("point", { factory->CreateConstant("2"), variableY })
                                                                                       })
                                                            })
                                 });
        CHECK(CheckSolutions(*result, {
            { UnifierItemType(variableY,  factory->CreateConstant("1")) }
        }));

        // add_3_and_double(X,Y) :- Y is (X+3)*2.
    }
}
