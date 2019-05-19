//
//  AIPrologTests.cpp
//  TestLib
//
//  Created by Eric Zinda on 10/15/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//
#include "FXPlatform/FailFast.h"
#include "FXPlatform/Parser/ParserDebug.h"
#include "FXPlatform/Prolog/HtnGoalResolver.h"
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "FXPlatform/Prolog/PrologQueryCompiler.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"
#include "FXPlatform/Prolog/PrologParser.h"
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "Logger.h"
#include "Tests/ParserTestBase.h"
#include <thread>
#include "UnitTest++/UnitTest++.h"
using namespace Prolog;


SUITE(PrologCompilerTests)
{
    TEST(PrologCompilerTests)
    {
//                        SetTraceFilter(SystemTraceType::Parsing, TraceDetail::Diagnostic);
        
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler;

        // Facts
        state->ClearAll();
        compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        CHECK(compiler->Compile("a."));
        CHECK(state->DebugHasRule("a", ""));

        state->ClearAll();
        compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        CHECK(compiler->Compile("a(b)."));
        CHECK(state->DebugHasRule("a(b)", ""));

        // Rules
        state->ClearAll(); 
        compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        CHECK(compiler->Compile("a(b) :-."));
        CHECK(state->DebugHasRule("a(b)", ""));
    }

    // Test using the Htn syntax for variables
    TEST(PrologQueryTests)
    {
        //                        SetTraceFilter(SystemTraceType::Parsing, TraceDetail::Diagnostic);
        HtnGoalResolver resolver;
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<PrologCompiler> compiler;
        shared_ptr<PrologQueryCompiler> query;
        shared_ptr<vector<UnifierType>> queryResult;
        string result;
        
        // Facts
        state->ClearAll();
        compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        CHECK(compiler->Compile("a."));
        query = shared_ptr<PrologQueryCompiler>(new PrologQueryCompiler(factory.get()));
        CHECK(query->Compile(("a.")));
        queryResult = resolver.ResolveAll(factory.get(), state.get(), query->result());
        result = HtnGoalResolver::ToString(queryResult.get());
        CHECK_EQUAL(result, "(())");
        
        // Rules
        state->ClearAll();
        compiler = shared_ptr<PrologCompiler>(new PrologCompiler(factory.get(), state.get()));
        CHECK(compiler->Compile("a(1). a(2)."));
        query = shared_ptr<PrologQueryCompiler>(new PrologQueryCompiler(factory.get()));
        CHECK(query->Compile(("a(X).")));
        queryResult = resolver.ResolveAll(factory.get(), state.get(), query->result());
        result = HtnGoalResolver::ToString(queryResult.get());
        CHECK_EQUAL(result, "null");
    }

    template<class VariableRule>
    void ParserShared(bool htnStyle)
    {
        int deepestFailure;
        string result;
        string errorMessage;
        shared_ptr<Symbol> rule;
        vector<shared_ptr<Symbol>> flattenedTree;
        
        //        SetTraceFilter(SystemTraceType::Parsing, TraceDetail::Diagnostic);
        
        // ************
        // These are the things that are different based on VariableRule
        // ************

        // variable
        rule = ParserDebug::TestTryParse<PrologVariable<VariableRule>>("?a", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));
        
        rule = ParserDebug::TestTryParse<PrologVariable<VariableRule>>("A", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule != nullptr));
        
        // term
        rule = ParserDebug::TestTryParse<PrologTerm<VariableRule>>("?a", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));

        // functor (should not allow capitalization of functor names, not prolog compatible
        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("do(Move(unit,from,to),SetEnergy(energy,-(energy,moveCost)))", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));
        
        // Nothing with ? in front should work in Prolog-style, but should in the other
        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("a(?b,?c)", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));
        
        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("a( ?b , ?c )", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));
        
        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("a( d(e,f,g) , ?c )", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));
        
        // Examples
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("operator(SetEnergy(?old,?new), del(Energy(?old)), add(Energy(?new)) ).", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));
        
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("method(MoveUnit(?unit, ?to), if(UnitIdle(?unit),At(?unit,?from)),   do(Move(?unit,?from,?to),SetEnergy(?energy,-(?energy,?moveCost))) ).", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));
        
        // Query
        rule = ParserDebug::TestTryParse<PrologQuery<VariableRule>>("a(?X, a). \r\n", deepestFailure, errorMessage);
        CHECK(htnStyle != (rule == nullptr));
        
        
        // ************
        // These are the the same for both VariableRule alternates
        // ************

        // atom
        rule = ParserDebug::TestTryParse<PrologAtom>("constant", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("con-stant", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("con_stant", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>(">", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>(">=", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("1", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("+1", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("-1", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("1.2", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("+1.2", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("-1.2", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologAtom>("!", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        
        // term
        rule = ParserDebug::TestTryParse<PrologTerm<VariableRule>>("a", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologTerm<VariableRule>>("a(b)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologTerm<VariableRule>>("A", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        
        // term list
        rule = ParserDebug::TestTryParse<PrologTermList<VariableRule>>("foo(a)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologTermList<VariableRule>>("foo(a), !", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        
        // functor
        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("a()", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("a(b)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("-(b,c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("a(b,c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = ParserDebug::TestTryParse<PrologFunctor<VariableRule>>("a", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        
        // rule
        rule = ParserDebug::TestTryParse<PrologRule<VariableRule>>("a :- ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologRule<VariableRule>>("a(g) :- ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologRule<VariableRule>>("a( g , ef ) :- ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologRule<VariableRule>>("a(g) :- b(c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologRule<VariableRule>>("a(g) :- b(c), d(a,b,c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologRule<VariableRule>>("a( g, ef) :- foo(a)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologRule<VariableRule>>("a( g, ef) :- foo(a), !", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        
        // Document
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("a.  ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("a.  b.", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("a(g, ef) :- foo(a).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("north :- go(north). \r\nsouth :- go(south).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("a(g).  ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("a(g).  \r\nb(d,e(f,g)) :-.", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        
        // Examples
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("goals(findSolution(a)).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        

        // Comment
        rule = ParserDebug::TestTryParse<PrologComment>("%\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologComment>("%foo\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologComment>("%foo\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologComment>("%foo\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologComment>("%foo\r", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologComment>("%foo\n\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologComment>("/* test */", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologComment>("/* test \r\n test2 */", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        
        // Whitespace
        rule = ParserDebug::TestTryParse<PrologOptionalWhitespace>(" ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologOptionalWhitespace>(" \r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologOptionalWhitespace>("%foo\n\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologOptionalWhitespace>("\r\n  %foo\n\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("a(a). b(b). a(b, c). % This is a comment\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologDocument<VariableRule>>("a(%\na%\n)%\n.%\n b(%\nb%\n)%\n.%\n a(%\nb,%\n c%\n)%\n. % This is a comment\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        
        // Query
        rule = ParserDebug::TestTryParse<PrologQuery<VariableRule>>("a.", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologQuery<VariableRule>>("a(a).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologQuery<VariableRule>>("a(a). \r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = ParserDebug::TestTryParse<PrologQuery<VariableRule>>("a(X, a). \r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
    }
    
    // Test using classic prolog parsing rules where capitilization means variable
    TEST_FIXTURE(ParserTestBase, PrologParserVariableTests)
    {
        ParserShared<PrologCapitalizedVariable>(false);
    }

    // Test using htn prolog parsing rules where variable needs ? and capitalization is meaningless
    TEST_FIXTURE(ParserTestBase, PrologParserHtnVariableTests)
    {
        ParserShared<HtnVariable>(true);
    }

    
    TEST_FIXTURE(ParserTestBase, PrologParserTests)
    {
        int deepestFailure;
        string result;
        string errorMessage;
        shared_ptr<Symbol> rule;
        vector<shared_ptr<Symbol>> flattenedTree;
        
//        SetTraceFilter(SystemTraceType::Parsing, TraceDetail::Diagnostic);
        
        // atom
        rule = TestTryParse<PrologAtom>("constant", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>("con-stant", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>("con_stant", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>(">", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>(">=", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>("1", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>("+1", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>("-1", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>("1.2", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>("+1.2", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologAtom>("-1.2", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

		rule = TestTryParse<PrologAtom>("!", deepestFailure, errorMessage);
		CHECK(rule != nullptr);

        
        // variable
        rule = TestTryParse<PrologVariable<>>("?a", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

//        rule = TestTryParse<PrologVariable<>>("A", deepestFailure, errorMessage);
//        CHECK(rule != nullptr);

        
        // term
        rule = TestTryParse<PrologTerm<>>("a", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologTerm<>>("?a", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologTerm<>>("a(b)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);


        // term list
        rule = TestTryParse<PrologTermList<>>("foo(a)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

		rule = TestTryParse<PrologTermList<>>("foo(a), !", deepestFailure, errorMessage);
		CHECK(rule != nullptr);

        // functor
        rule = TestTryParse<PrologFunctor<>>("a()", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologFunctor<>>("a(b)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologFunctor<>>("-(b,c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologFunctor<>>("a(b,c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologFunctor<>>("a(?b,?c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologFunctor<>>("a( ?b , ?c )", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologFunctor<>>("a( d(e,f,g) , ?c )", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologFunctor<>>("do(Move(?unit,?from,?to),SetEnergy(?energy,-(?energy,?moveCost)))", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        // rule
        rule = TestTryParse<PrologRule<>>("a :- ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologRule<>>("a(g) :- ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologRule<>>("a( g , ef ) :- ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = TestTryParse<PrologRule<>>("a(g) :- b(c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologRule<>>("a(g) :- b(c), d(a,b,c)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologRule<>>("a( g, ef) :- foo(a)", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

		rule = TestTryParse<PrologRule<>>("a( g, ef) :- foo(a), !", deepestFailure, errorMessage);
		string foo = PrintTree(rule, 1);
		CHECK(rule != nullptr);

        // Document
        rule = TestTryParse<PrologDocument<>>("a.  ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologDocument<>>("a.  b.", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologDocument<>>("a(g, ef) :- foo(a).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologDocument<>>("north :- go(north). \r\nsouth :- go(south).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologDocument<>>("a(g).  ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = TestTryParse<PrologDocument<>>("a(g).  \r\nb(d,e(f,g)) :-.", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        // Examples
        rule = TestTryParse<PrologDocument<>>("operator(SetEnergy(?old,?new), del(Energy(?old)), add(Energy(?new)) ).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = TestTryParse<PrologDocument<>>("goals(findSolution(a)).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        rule = TestTryParse<PrologDocument<>>("method(MoveUnit(?unit, ?to), if(UnitIdle(?unit),At(?unit,?from)),   do(Move(?unit,?from,?to),SetEnergy(?energy,-(?energy,?moveCost))) ).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        // Comment
        rule = TestTryParse<PrologComment>("%\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologComment>("%foo\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologComment>("%foo\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologComment>("%foo\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologComment>("%foo\r", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologComment>("%foo\n\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologComment>("/* test */", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologComment>("/* test \r\n test2 */", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        // Whitespace
        rule = TestTryParse<PrologOptionalWhitespace>(" ", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologOptionalWhitespace>(" \r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologOptionalWhitespace>("%foo\n\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologOptionalWhitespace>("\r\n  %foo\n\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologDocument<>>("a(a). b(b). a(b, c). % This is a comment\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologDocument<>>("a(%\na%\n)%\n.%\n b(%\nb%\n)%\n.%\n a(%\nb,%\n c%\n)%\n. % This is a comment\r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);
        
        // Query
        rule = TestTryParse<PrologQuery<>>("a(a).", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologQuery<>>("a(a). \r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologQuery<>>("a(X, a). \r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

        rule = TestTryParse<PrologQuery<>>("a(?X, a). \r\n", deepestFailure, errorMessage);
        CHECK(rule != nullptr);

    }
}
