//
//  HtnRuleSet.cpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//
#include "FXPlatform/FailFast.h"
#include "FXPlatform/NanoTrace.h"
#include "FXPlatform/SystemTraceType.h"
#include "HtnRuleSet.h"
#include "HtnTerm.h"
using namespace std;

void HtnRuleSet::HtnSharedRules::AddRule(shared_ptr<HtnTerm> head, vector<shared_ptr<HtnTerm>> tail)
{
    FailFastAssertDesc(!m_isLocked, "Internal Error");
    FailFastAssertDesc(head->name().size() > 0, "term name must have at least one character");
    HtnTerm::HtnTermID headID = head->GetUniqueID();
    
    // Ground facts must be unique
    if(tail.size() == 0 && head->isGround())
    {
        auto found = m_ruleHeads.find(headID);
        if(found != m_ruleHeads.end())
        {
            // Make sure we actually have the rule and it isn't some artifact of
            // how we are tracking the rules
            for(auto item : m_rules)
            {
                if(item.head()->ToString() == head->ToString())
                {
                    TraceString1("HtnRuleSet::HtnSharedRules::AddRule duplicate rule '{0}'",
                                 SystemTraceType::Solver, TraceDetail::Normal,
                                 item.head()->ToString());
                    FailFastAssertDesc(false, ("Duplicate Rule added: " + item.head()->ToString()).c_str());
                }
            }            
        }
    }

    HtnRule newRule(head, tail);
    
    // Update indexes to make lookups faster later. Not a huge memory concern since this is a singleton shared by
    // all rules
    string ruleString = newRule.GetUniqueID();
    m_ruleHeads.insert(headID);
    m_ruleIndex.insert(ruleString);
    m_dynamicSize += sizeof(headID) + sizeof(string) + ruleString.size();

    m_rules.push_back(newRule);
    // Need to subtract off HtnRule because dynamicSize() already includes it
    m_dynamicSize += sizeof(pair<string, HtnRule>) - sizeof(HtnRule) + newRule.dynamicSize();
}

void HtnRuleSet::HtnSharedRules::ClearAll()
{
    FailFastAssertDesc(!m_isLocked, "Internal Error");
    m_rules.clear();
    m_ruleHeads.clear();
    m_ruleIndex.clear();
    m_dynamicSize = sizeof(HtnSharedRules);
}

// A fact is a rule that is true (i.e. has no tail)
bool HtnRuleSet::HtnSharedRules::HasFact(shared_ptr<HtnTerm> term) const
{
    HtnRule rule(term, {});
    return m_ruleIndex.find(rule.GetUniqueID()) != m_ruleIndex.end();
}

void HtnRuleSet::AddRule(shared_ptr<HtnTerm> head, vector<shared_ptr<HtnTerm>> tail)
{
    // Should not be updating facts at this point
    FailFastAssertDesc(m_factsDiff.size() == 0, "Internal Error");
    m_sharedRules->AddRule(head, tail);
}

// This is a quick test to get rid of obvious failures without having to do more work
// it is just to improve performance
//
// For a given argument on a term we have these cases that *might* work (if the terms are equivalent)
// Variable / Anything -> A variable can unify with anything
// Constant / Constant -> If they are equal
// Constant / Variable
// Compound / Compound -> If they are equivalent
bool HtnRuleSet::CanPotentiallyUnify(const HtnTerm *term, const HtnTerm *ruleHead) const
{
    if(term->isEquivalentCompoundTerm(ruleHead) || (term->isConstant() && ruleHead->isConstant()))
    {
        for(int index = 0; index < term->arguments().size(); ++index)
        {
            HtnTerm *termArg = term->arguments()[index].get();
            HtnTerm *ruleHeadArg = ruleHead->arguments()[index].get();
            if (termArg->isVariable())
            {
                // Yes, keep checking
            }
            else if(termArg->isConstant())
            {
                // This arg is a constant
                if(ruleHeadArg->isConstant())
                {
                    // Constant / Constant
                    // So is the head. Are they the same constant?
                    
                    if(!(termArg->nameEqualTo(*ruleHeadArg)))
                    {
                        
                        // Any arguments that are both constants and not == could
                        // never be unified
                        return false;
                    }
                    else
                    {
                        // Yes, keep checking
                    }
                }
                else
                {
                    // term arg is constant, head arg is not, can only unify if head arg
                    // is a variable
                    if(!ruleHeadArg->isVariable())
                    {
                        return false;
                    }
                    else
                    {
                        // Constant / Variable
                        // This arg will unify, keep checking
                    }
                }
            }

            else if(termArg->isCompoundTerm())
            {
                // Term arg is compound
                if(ruleHeadArg->isConstant())
                {
                    // Can't unify a compound with a constant
                    return false;
                }
                else if(ruleHeadArg->isCompoundTerm())
                {
                    // Compound / Compound
                    // Might work, if they are equivalent
                    if(!termArg->isEquivalentCompoundTerm(ruleHeadArg))
                    {
                        return false;
                    }
                }
            }
            else
            {
                // Should always be a variable, compound or constant
                FailFastAssert(false);
            }
        }
        
        return true;
    }
    else
    {
        return false;
    }
}


void HtnRuleSet::ClearAll()
{
    m_sharedRules->ClearAll();
    m_factsDiff.clear();
    m_factAdditions.clear();
    m_dynamicSize = sizeof(HtnRuleSet);
}

// Creates a new RuleSet with a copy of the rules that is editable.  Changes to the original will not be reflected
shared_ptr<HtnRuleSet> HtnRuleSet::CreateSharedRulesCopy()
{
    shared_ptr<HtnRuleSet> newState = shared_ptr<HtnRuleSet>(new HtnRuleSet());
    newState->m_sharedRules = shared_ptr<HtnSharedRules>(new HtnSharedRules(*m_sharedRules));
    newState->m_sharedRules->m_isLocked = false;
    
    return newState;
}

// Creates a copy of the ruleset so that different facts can get added or removed without affecting
// original. Useful for backtracking
shared_ptr<HtnRuleSet> HtnRuleSet::CreateCopy()
{
    // Make a copy of everything
    shared_ptr<HtnRuleSet> newState = shared_ptr<HtnRuleSet>(new HtnRuleSet(*this));
    
    // Once we've created a copy we lock the rules because copies are meant to isolate RuleSets from each other and we don't diff the rules
    m_sharedRules->Lock();
    
    return newState;
}

shared_ptr<HtnRuleSet> HtnRuleSet::CreateNextState(HtnTermFactory *factory, const vector<shared_ptr<HtnTerm>> &factsToRemove, const vector<shared_ptr<HtnTerm>> &factsToAdd)
{
    // Copy existing state
    shared_ptr<HtnRuleSet> newState = CreateCopy();
    
    // Process changes
    Update(factory, factsToRemove, factsToAdd);
    
    return newState;
}

bool HtnRuleSet::DebugHasRule(const string &head, const string &tail) const
{
    bool found = false;
    string ruleString;
    string composed;
    AllRules([&](const HtnRule &rule)
      {
          ruleString = rule.ToStringProlog();
          composed = head + " :- " + tail + ".";
          if(ruleString == composed)
          {
              found = true;
              return false;
          }
          
          return true;
      });
          
    return found;
}

bool HtnRuleSet::HasEquivalentRule(std::shared_ptr<HtnTerm> term) const
{
    bool found = false;
    AllRules([&](const HtnRule &rule)
      {
          if(rule.head()->isEquivalentCompoundTerm(term.get()))
          {
              found = true;
              return false;
          }
          
          return true;
      });

    return found;
}

// A fact is a rule that is true
bool HtnRuleSet::HasFact(shared_ptr<HtnTerm> term) const
{
    // Since this rule is a fact it might have been updated or deleted
    FactsDiffType::const_iterator found = m_factsDiff.find(term->GetUniqueID());
    if(found != m_factsDiff.end())
    {
        // New fact overrides the shared fact if it exists
        if(found->second.isAdd) { return true; }
        // fact was deleted
        else { return false; }
    }

    // It is a fact that wasn't updated, so it can only be in the shared rules
    return m_sharedRules->HasFact(term);
}

string HtnRuleSet::ToStringFacts() const
{
    stringstream stream;
    bool has = false;
    AllRules([&](const HtnRule &rule)
    {
        if(rule.IsFact())
        {
            stream << (has ? "," : "") << rule.ToString();
            has = true;
        }

        return true;
    });

    return stream.str();
}

string HtnRuleSet::ToStringFactsProlog() const
{
    stringstream stream;
    AllRules([&](const HtnRule &rule)
             {
                 if(rule.IsFact())
                 {
                     stream << rule.ToStringProlog() << " ";
                 }
                 
                 return true;
             });
    
    return stream.str();
}

void HtnRuleSet::Update(HtnTermFactory *factory, const vector<shared_ptr<HtnTerm>> &factsToRemove, const vector<shared_ptr<HtnTerm>> &factsToAdd)
{
    // Add all changes into the diffs list
    for(auto item : factsToRemove)
    {
        // All removals must be ground
        FailFastAssertDesc(item->isGround(), ("Items to be removed must be ground: " + item->ToString()).c_str());

        // Can only remove something that exists
        if(!HasFact(item))
        {
            FailFastAssertDesc(false, (string("Can't retract something that doesn't exist: ") + item->ToString()).c_str());
        }
        
        // Note that if we are removing a fact that was added in this instance, we will end up with
        // a factsDiff entry that is a remove of something not in the base DB.  This is inert.
        shared_ptr<HtnRule> rule = shared_ptr<HtnRule>(new HtnRule(item, {}));
        HtnTerm::HtnTermID key = item->GetUniqueID();
        FactsDiffType::iterator found = m_factsDiff.find(key);
        int diffOrderToRemove = -1;
        if(found == m_factsDiff.end())
        {
            m_factsDiff.insert(FactsDiffType::value_type(key, FactsDiffType::mapped_type(false, m_factsOrder, rule)));
            m_factsOrder++;
            
            // Need to subtract off HtnRule because dynamicSize() already includes it
            m_dynamicSize += sizeof(FactsDiffType::value_type) - sizeof(HtnRule) + rule->dynamicSize();
        }
        else
        {
            diffOrderToRemove = found->second.diffOrder;
            
            // Can't use .insert because that won't replace an existing item
            found->second = FactsDiffType::mapped_type(false, m_factsOrder, rule);
            m_factsOrder++;
        }
        
        // Also need to erase it from the order member if it was an add before
        if(diffOrderToRemove != -1)
        {
            size_t erasedCount = m_factAdditions.erase(diffOrderToRemove);
            FailFastAssertDesc(erasedCount == 1, "Internal Error");
        }
    }

    // Add additions
    for(auto item : factsToAdd)
    {
        // All additions must be ground
        FailFastAssertDesc(item->isGround(), ("All additions must be ground: " + item->ToString()).c_str());

        // Can only add something that doesn't exist yet
        if(HasFact(item))
        {
            FailFastAssertDesc(false, (string("Can't assert something that already exists: ") + item->ToString()).c_str());
        }
        
        shared_ptr<HtnRule> rule = shared_ptr<HtnRule>(new HtnRule(item, {}));
        int diffOrderToAdd = m_factsOrder;
        m_factsOrder++;
        HtnTerm::HtnTermID key = item->GetUniqueID();
        FactsDiffType::iterator found = m_factsDiff.find(key);
        if(found == m_factsDiff.end())
        {
            m_factsDiff.insert(FactsDiffType::value_type(key, FactsDiffType::mapped_type(true, diffOrderToAdd, rule)));

            // Need to subtract off HtnRule because dynamicSize() already includes it
            m_dynamicSize += sizeof(FactsDiffType::value_type) - sizeof(HtnRule) + rule->dynamicSize();
        }
        else
        {
            // Can't use .insert because that won't replace an existing item
            found->second = FactsDiffType::mapped_type(true, diffOrderToAdd, rule);
        }
        
        // Now add it to the additions list
        m_factAdditions.insert(FactsAdditionsType::value_type(diffOrderToAdd, rule));
    }
}
