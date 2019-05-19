//
//  HtnRuleSetTests.cpp
//  TestLib
//
//  Created by Eric Zinda on 1/24/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include "FXPlatform/Prolog/HtnGoalResolver.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTerm.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"
#include "FXPlatform/Prolog/PrologParser.h"
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "Tests/ParserTestBase.h"
#include "Logger.h"
#include <thread>
#include "UnitTest++/UnitTest++.h"

using namespace Prolog;

SUITE(HtnRuleSetTests)
{
    void CheckRuleExistsAllWays(bool expectedExists, shared_ptr<HtnRuleSet> ruleSet, shared_ptr<HtnTerm> ruleHead, const vector<shared_ptr<HtnTerm>> &ruleTail)
    {
        HtnRule rule(ruleHead, ruleTail);
        string ruleID = rule.GetUniqueID();

        StaticFailFastAssert(ruleSet->DebugHasRule(ruleHead->ToString(), HtnTerm::ToString(ruleTail, false)) == expectedExists);
        if(rule.IsFact())
        {
            StaticFailFastAssert(ruleSet->HasFact(ruleHead) == expectedExists);
        }
        else
        {
            StaticFailFastAssert(!ruleSet->HasFact(ruleHead));
        }

        bool found = false;
        ruleSet->AllRules([&](const HtnRule &rule)
        {
            if(rule.GetUniqueID() == ruleID)
            {
                found = true;
                return false;
            }
            
            return true;
        });
        StaticFailFastAssert(expectedExists == found);
    }

    void CheckRuleOrder(shared_ptr<HtnRuleSet> ruleSet, const vector<string> &ruleIDs)
    {
        int index = 0;
        ruleSet->AllRules([&](const HtnRule &rule)
                          {
                              string ruleString = rule.ToString();
                              StaticFailFastAssert(index < ruleIDs.size());
                              StaticFailFastAssert(ruleString == ruleIDs[index])
                              index++;
                              return true;
                          });
        StaticFailFastAssert(index == ruleIDs.size());
    }
    
    TEST(RuleSetRuleInterning)
    {
        shared_ptr<HtnTermFactory> factory;
        shared_ptr<HtnRuleSet> ruleSet;
        
        // Add a rule
        factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        ruleSet = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnTerm> ruleHead = factory->CreateFunctor("Head1", { factory->CreateVariable("X") });
        ruleSet->AddRule(ruleHead, { });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { });
        
        ruleHead = factory->CreateFunctor("Head1", { factory->CreateConstant("X") });
        ruleSet->AddRule(ruleHead, { });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { });
        
        CheckRuleOrder(ruleSet, {
            "Head1(?X) => ",
            "Head1(X) => "
        });
    }
    
    TEST(RuleSetBasicOperation)
    {
        shared_ptr<HtnTermFactory> factory;
        shared_ptr<HtnRuleSet> ruleSet;
        
        // Add a rule
        factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        ruleSet = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        shared_ptr<HtnTerm> ruleHead = factory->CreateFunctor("Head1", { factory->CreateVariable("X") });
        shared_ptr<HtnTerm> ruleTail = factory->CreateFunctor("Tail1", { factory->CreateVariable("X") });
        ruleSet->AddRule(ruleHead, { ruleTail });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { ruleTail });

        // Add a fact
        ruleHead = factory->CreateFunctor("Fact1", { factory->CreateConstant("Value") });
        ruleSet->AddRule(ruleHead, { });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { });

        // Loop through all the rules and facts: They must be returned in the order they got entered into the file
        // Add another fact that sorts alphabetically in front of the one we have, should be returned
        // *after* since it was added after
        ruleHead = factory->CreateFunctor("Before", { factory->CreateConstant("Value") });
        ruleSet->AddRule(ruleHead, { });
        // Add a rule with same head, different tail: ditto on order
        ruleSet->AddRule(factory->CreateFunctor("Head1", { factory->CreateVariable("X") }),
                         { factory->CreateFunctor("After", { factory->CreateVariable("X") }) });

        CheckRuleOrder(ruleSet, {
            "Head1(?X) => Tail1(?X)",
            "Fact1(Value) => ",
            "Before(Value) => ",
            "Head1(?X) => After(?X)"
        });

        // Ground facts must be unique
        // Not OK to add a ground fact twice
        bool failed = false;
        try { ruleSet->AddRule(factory->CreateFunctor("Fact1", { factory->CreateConstant("Value") }), { }); }
        catch(...) { failed = true; }
        CHECK(failed);
        
        // Clearing works
        ruleSet->ClearAll();
        CheckRuleOrder(ruleSet, {});
    }
    
    TEST(RuleSetCopyingTests)
    {
        shared_ptr<HtnTermFactory> factory;
        shared_ptr<HtnRuleSet> ruleSet;
        shared_ptr<HtnTerm> ruleHead;
        shared_ptr<HtnTerm> ruleTail;
        
        factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
        ruleSet = shared_ptr<HtnRuleSet>(new HtnRuleSet());

        // Add 3 rules
        ruleHead = factory->CreateFunctor("Head1", { factory->CreateVariable("X") });
        ruleTail = factory->CreateFunctor("Tail1", { factory->CreateVariable("X") });
        ruleSet->AddRule(ruleHead, { ruleTail });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { ruleTail });

        ruleHead = factory->CreateFunctor("Head1", { factory->CreateVariable("X") });
        ruleTail = factory->CreateFunctor("Tail2", { factory->CreateVariable("X") });
        ruleSet->AddRule(ruleHead, { ruleTail });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { ruleTail });

        ruleHead = factory->CreateFunctor("Head2", { factory->CreateVariable("X") });
        ruleTail = factory->CreateFunctor("Tail3", { factory->CreateVariable("X") });
        ruleSet->AddRule(ruleHead, { ruleTail });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { ruleTail });

        // Add two facts
        ruleHead = factory->CreateFunctor("Fact1", { factory->CreateConstant("True") });
        ruleSet->AddRule(ruleHead, { });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { });

        ruleHead = factory->CreateFunctor("Fact2", { factory->CreateConstant("True") });
        ruleSet->AddRule(ruleHead, { });
        CheckRuleExistsAllWays(true, ruleSet, ruleHead, { });
        CheckRuleOrder(ruleSet, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            "Fact1(True) => ",
            "Fact2(True) => ",
        });

        // Copying allows independent changes of facts for both rulesets
        // Update ruleset2
        shared_ptr<HtnRuleSet> ruleSet2 = ruleSet->CreateCopy();
        vector<shared_ptr<HtnTerm>> factsToAdd = { factory->CreateFunctor("Fact3", { factory->CreateConstant("True") }) };
        vector<shared_ptr<HtnTerm>> factsToRemove = { factory->CreateFunctor("Fact1", { factory->CreateConstant("True") }) };
        ruleSet2->Update(factory.get(), factsToRemove, factsToAdd);

        // ruleSet2 should see changes but not ruleset1
        CheckRuleExistsAllWays(true, ruleSet2, factsToAdd[0], {});
        CheckRuleExistsAllWays(false, ruleSet2, factsToRemove[0], {});
        CheckRuleExistsAllWays(false, ruleSet, factsToAdd[0], {});
        CheckRuleExistsAllWays(true, ruleSet, factsToRemove[0], {});
        CheckRuleOrder(ruleSet2, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            "Fact2(True) => ",
            // New facts are always at the end, in the order they were added
            "Fact3(True) => ",
        });
        CheckRuleOrder(ruleSet, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            "Fact1(True) => ",
            "Fact2(True) => ",
        });

        // Update ruleset1
        factsToAdd = { factory->CreateFunctor("Fact4", { factory->CreateConstant("True") }) };
        factsToRemove = { factory->CreateFunctor("Fact2", { factory->CreateConstant("True") }) };
        ruleSet->Update(factory.get(), factsToRemove, factsToAdd);

        // ruleSet1 should see changes but not ruleset2
        CheckRuleExistsAllWays(true, ruleSet, factsToAdd[0], {});
        CheckRuleExistsAllWays(false, ruleSet, factsToRemove[0], {});
        CheckRuleExistsAllWays(false, ruleSet2, factsToAdd[0], {});
        CheckRuleExistsAllWays(true, ruleSet2, factsToRemove[0], {});
        CheckRuleOrder(ruleSet2, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            "Fact2(True) => ",
            // New facts are always at the end, in the order they were added
            "Fact3(True) => ",
        });
        CheckRuleOrder(ruleSet, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            "Fact1(True) => ",
            // New facts are always at the end, in the order they were added
            "Fact4(True) => ",
        });

        // Deleting something that has already been added should work
        factsToRemove = { factory->CreateFunctor("Fact4", { factory->CreateConstant("True") }) };
        ruleSet->Update(factory.get(), factsToRemove, {});
        CheckRuleOrder(ruleSet, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            "Fact1(True) => ",
        });

        // And then adding it again should work again
        factsToAdd = { factory->CreateFunctor("Fact4", { factory->CreateConstant("True") }) };
        ruleSet->Update(factory.get(), {}, factsToAdd);
        CheckRuleOrder(ruleSet, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            "Fact1(True) => ",
            // New facts are always at the end, in the order they were added
            "Fact4(True) => ",
        });

        // Deleting and then adding a base fact should work
        factsToRemove = { factory->CreateFunctor("Fact1", { factory->CreateConstant("True") }) };
        ruleSet->Update(factory.get(), factsToRemove, {});
        CheckRuleOrder(ruleSet, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            // New facts are always at the end, in the order they were added
            "Fact4(True) => ",
        });
        factsToAdd = { factory->CreateFunctor("Fact1", { factory->CreateConstant("True") }) };
        ruleSet->Update(factory.get(), {}, factsToAdd);
        CheckRuleOrder(ruleSet, {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            // New facts are always at the end, in the order they were added
            "Fact4(True) => ",
            "Fact1(True) => ",
        });
        
        // Cannot add a fact that already exists in the base ruleset
        ruleHead = factory->CreateFunctor("Fact2", { factory->CreateConstant("True") });
        CHECK(ruleSet2->HasFact(ruleHead));
        factsToAdd = { ruleHead };
        bool fail = false;
        try{ ruleSet2->Update(factory.get(), {}, factsToAdd); } catch(...) { fail = true; }
        CHECK(fail);

        // Cannot add a fact that already exists in the new ruleset
        ruleHead = factory->CreateFunctor("Fact3", { factory->CreateConstant("True") });
        CHECK(ruleSet2->HasFact(ruleHead));
        factsToAdd = { ruleHead };
        fail = false;
        try{ ruleSet2->Update(factory.get(), {}, factsToAdd); } catch(...) { fail = true; }
        CHECK(fail);
        
        // Copying a copy should inherit the changes the copy made
        shared_ptr<HtnRuleSet> ruleSet3 = ruleSet2->CreateCopy();
        vector<string> ruleSet2Rules =
        {
            "Head1(?X) => Tail1(?X)",
            "Head1(?X) => Tail2(?X)",
            "Head2(?X) => Tail3(?X)",
            "Fact2(True) => ",
            // New facts are always at the end, in the order they were added
            "Fact3(True) => "
        };
        CheckRuleOrder(ruleSet2, ruleSet2Rules);
        CheckRuleOrder(ruleSet3, ruleSet2Rules);

        // Rules cannot be added after a copy is made
        fail = false;
        try { ruleSet3->AddRule(factory->CreateFunctor("Fact99", { factory->CreateConstant("True") }), {}); } catch(...) { fail = true; }
        CHECK(fail);
    }
}
