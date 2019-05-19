//
//  HtnTermTests.cpp
//  TestLib
//
//  Created by Eric Zinda on 3/1/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include "FXPlatform/Prolog/HtnTerm.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"
#include <memory>
#include "UnitTest++/UnitTest++.h"
using namespace std;

SUITE(HtnTermTests)
{
    TEST(HtnTermUniqueID)
    {
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        CHECK( factory->CreateConstantFunctor("a", {"b", "c"})->GetUniqueID() !=
              factory->CreateFunctor("a", {factory->CreateConstantFunctor("b", {"c"})})->GetUniqueID());
        
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

