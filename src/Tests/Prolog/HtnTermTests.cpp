//
//  HtnTermTests.cpp
//  TestLib
//
//  Created by Eric Zinda on 3/1/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include "FXPlatform/Prolog/HtnTerm.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "FXPlatform/Prolog/PrologQueryCompiler.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include <memory>
#include "UnitTest++/UnitTest++.h"
using namespace std;

SUITE(HtnTermTests)
{
    TEST(JsonTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<PrologStandardQueryCompiler> query = shared_ptr<PrologStandardQueryCompiler>(new PrologStandardQueryCompiler(factory.get()));

        // Anything with a capital letter or _ that starts it must be escaped
        // Any character not in ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_ forces escaping
        CHECK(query->Compile("d_named('Plage'), !, d_named('_flage'), d_named('Pla ge'), d_named('Pla!ge'), d_named('pl_age'), d_named('pl1age'), d_named(12.4), d_named(5), d_named('5a'), d_named([])."));
        string result = HtnTerm::ToString(query->result(), false, true);
        CHECK_EQUAL("{\"d_named\":[{\"'Plage'\":[]}]}, {\"'!'\":[]}, {\"d_named\":[{\"'_flage'\":[]}]}, {\"d_named\":[{\"'Pla ge'\":[]}]}, {\"d_named\":[{\"'Pla!ge'\":[]}]}, {\"d_named\":[{\"pl_age\":[]}]}, {\"d_named\":[{\"pl1age\":[]}]}, {\"d_named\":[{\"12.4\":[]}]}, {\"d_named\":[{\"5\":[]}]}, {\"d_named\":[{\"'5a'\":[]}]}, {\"d_named\":[[]]}", result);
        
        // Double quoted strings should preserve their double quotes
        query->Clear();
        CHECK(query->Compile("vocabulary(\"blue\", Pred, Argcount, adjective, X)."));
        result = HtnTerm::ToString(query->result(), false, true);
        CHECK_EQUAL("{\"vocabulary\":[{\"\\\"blue\\\"\":[]},{\"Pred\":[]},{\"Argcount\":[]},{\"adjective\":[]},{\"X\":[]}]}", result);
    }
    
    TEST(HtnTermUniqueID)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        CHECK( factory->CreateConstantFunctor("a", {"b", "c"})->GetUniqueID() !=
              factory->CreateFunctor("a", {factory->CreateConstantFunctor("b", {"c"})})->GetUniqueID());
        
    }
    
    void RoundTripExpr(shared_ptr<HtnTermFactory> factory, shared_ptr<HtnRuleSet> state, shared_ptr<HtnGoalResolver> resolver, string expr)
    {
        shared_ptr<PrologQueryCompiler> query = shared_ptr<PrologQueryCompiler>(new PrologQueryCompiler(factory.get()));

        CHECK(query->Compile("=(?X, " + expr + ")."));
        shared_ptr<vector<UnifierType>> queryResult = resolver->ResolveAll(factory.get(), state.get(), query->result());
        string result = HtnGoalResolver::ToString(queryResult.get());
        CHECK_EQUAL("((?X = " + expr + "))", result);
    }
    
    // Make sure list expressions round trip to test ToString()
    TEST(HtnTermListToStringTest)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnGoalResolver> resolver = shared_ptr<HtnGoalResolver>(new HtnGoalResolver());
        string result;
        
        // Ensure that lists get properly compiled into the internal form: .(first, .(second, []))
        RoundTripExpr(factory, state, resolver, "[]");
        RoundTripExpr(factory, state, resolver, "[a]");
        RoundTripExpr(factory, state, resolver, "[a,b]");
        RoundTripExpr(factory, state, resolver, "[a,b,[]]");
        RoundTripExpr(factory, state, resolver, "[[],a,b,[]]");
        RoundTripExpr(factory, state, resolver, "[[],[],[]]");
        RoundTripExpr(factory, state, resolver, "[[a(b,c),[]],[],[]]");
        RoundTripExpr(factory, state, resolver, "[a([a,[a([b],d)],c]),[]]");
    }
    
    // Compares using prolog comparison rules in order:
    // variables, oldest first.
    // finite domain variables (section 9.1.1), oldest first.
    // floating point numbers, in numeric order.
    // integers, in numeric order.
    // atoms, in alphabetical (i.e. character code) order.
    // compound terms, ordered first by arity, then by the name of the principal functor and by the arguments in left-to-right order.
    TEST(HtnTermCompare)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        
        // Basic term types
        // Float
        CHECK(factory->CreateConstant("1.0")->TermCompare(*factory->CreateConstant("1.0")) == 0);
        CHECK(factory->CreateConstant("0.0")->TermCompare(*factory->CreateConstant("1.0")) == -1);
        CHECK(factory->CreateConstant("0.0")->TermCompare(*factory->CreateConstant("-1.0")) == 1);
        
        // Int
        CHECK(factory->CreateConstant("1")->TermCompare(*factory->CreateConstant("1")) == 0);
        CHECK(factory->CreateConstant("0")->TermCompare(*factory->CreateConstant("1")) == -1);
        CHECK(factory->CreateConstant("0")->TermCompare(*factory->CreateConstant("-1")) == 1);
        
        // Atoms
        CHECK(factory->CreateConstant("A")->TermCompare(*factory->CreateConstant("A")) == 0);
        CHECK(factory->CreateConstant("A")->TermCompare(*factory->CreateConstant("AA")) == -1);
        CHECK(factory->CreateConstant("BBB")->TermCompare(*factory->CreateConstant("AAA")) == 1);
        
        // Variables
        CHECK(factory->CreateVariable("A")->TermCompare(*factory->CreateVariable("A")) == 0);
        CHECK(factory->CreateVariable("A")->TermCompare(*factory->CreateVariable("AA")) == -1);
        CHECK(factory->CreateVariable("BBB")->TermCompare(*factory->CreateVariable("AAA")) == 1);

        // Cross type comparisons
        CHECK(factory->CreateVariable("A")->TermCompare(*factory->CreateConstant("2.0")) == -1);
        CHECK(factory->CreateVariable("A")->TermCompare(*factory->CreateConstant("2")) == -1);
        CHECK(factory->CreateConstant("1")->TermCompare(*factory->CreateConstant("2.0")) == 1);
        CHECK(factory->CreateConstant("A")->TermCompare(*factory->CreateConstant("1")) == 1);

        // Arity
        CHECK(factory->CreateConstant("term")->TermCompare(*factory->CreateConstantFunctor("term", {"1", "2"} )) == -1);
        CHECK(factory->CreateConstantFunctor("term", {"1", "2", "3"} )->TermCompare(*factory->CreateConstantFunctor("term", {"1", "2"} )) == 1);
        
        // Compound by name
        CHECK(factory->CreateConstantFunctor("term1", {"1", "2", "3"} )->TermCompare(*factory->CreateConstantFunctor("term", {"1", "2", "3"} )) == 1);
        CHECK(factory->CreateConstantFunctor("term", {"1", "2", "3"} )->TermCompare(*factory->CreateConstantFunctor("term", {"1", "2", "3"} )) == 0);
        CHECK(factory->CreateConstantFunctor("term", {"1", "2", "3"} )->TermCompare(*factory->CreateConstantFunctor("term1", {"1", "2", "3"} )) == -1);
        
        // Compound by value
        CHECK(factory->CreateConstantFunctor("term", {"1", "2", "3"} )->TermCompare(*factory->CreateConstantFunctor("term", {"1", "2", "4"} )) == -1);
        CHECK(factory->CreateConstantFunctor("term", {"1", "2", "3"} )->TermCompare(*factory->CreateConstantFunctor("term", {"1", "2", "4.0"} )) == 1);
        CHECK(factory->CreateConstantFunctor("term", {"1.0", "2.0", "4.0"} )->TermCompare(*factory->CreateConstantFunctor("term", {"1.0", "2.0", "4.0"} )) == 0);
        CHECK(factory->CreateConstantFunctor("term", {"1.0", "2.0", "3.0"} )->TermCompare(*factory->CreateConstantFunctor("term", {"1.0", "2.0", "4.0"} )) == -1);
        CHECK(factory->CreateConstantFunctor("term", {"A", "B", "C"} )->TermCompare(*factory->CreateConstantFunctor("term", {"A", "B", "C"} )) == 0);
        CHECK(factory->CreateConstantFunctor("term", {"A", "B", "C"} )->TermCompare(*factory->CreateConstantFunctor("term", {"A", "B", "D"} )) == -1);
        
        // Compound with compound terms
        CHECK(factory->CreateFunctor("term", {factory->CreateConstantFunctor("subTerm", { "1", "2" })} )->TermCompare(*factory->CreateFunctor("term", {factory->CreateConstantFunctor("subTerm", {"1", "2" })} )) == 0);
        CHECK(factory->CreateFunctor("term", {factory->CreateConstantFunctor("subTerm", { "1", "3" })} )->TermCompare(*factory->CreateFunctor("term", {factory->CreateConstantFunctor("subTerm", {"1", "2" })} )) == 1);

        // Make sure the comparer works
        set<shared_ptr<HtnTerm>, HtnTermComparer> termSet;
        termSet.insert(factory->CreateVariable("D"));
        termSet.insert(factory->CreateVariable("C"));
        termSet.insert(factory->CreateVariable("B"));
        termSet.insert(factory->CreateVariable("A"));
        CHECK(termSet.size() == 4);
        
        int index = 0;
        for(auto item : termSet)
        {
            switch(index)
            {
                case 0:
                    CHECK(item->TermCompare(*factory->CreateVariable("A")) == 0);
                    break;
                case 1:
                    CHECK(item->TermCompare(*factory->CreateVariable("B")) == 0);
                    break;
                case 2:
                    CHECK(item->TermCompare(*factory->CreateVariable("C")) == 0);
                    break;
                case 3:
                    CHECK(item->TermCompare(*factory->CreateVariable("D")) == 0);
                    break;
            }
            
            index++;
        }
    }
}

