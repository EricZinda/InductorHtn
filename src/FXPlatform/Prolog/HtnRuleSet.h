//
//  HtnRuleSet.hpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef HtnRuleSet_hpp
#define HtnRuleSet_hpp
#include <memory>
#include <string>
#include <list>
#include <map>
#include <vector>
#include "HtnRule.h"
#include "HtnTerm.h"

// A Prolog program is simply a set of rules
// Facts are simply clauses with empty bodies.  I.e.  cat(tom) means cat(tom) :- true, sunny means sunny := true
class HtnRuleSet : public std::enable_shared_from_this<HtnRuleSet>
{
public:
    HtnRuleSet() : m_dynamicSize(sizeof(HtnRuleSet)), m_factsOrder(0), m_sharedRules(std::shared_ptr<HtnSharedRules>(new HtnSharedRules())) {}
    void AddRule(std::shared_ptr<HtnTerm> head, std::vector<std::shared_ptr<HtnTerm>> m_tail);

    // Needs to return all the rules in the order they were added (i.e. order they were declared)
    // Also this needs to be very quick since it is called often
    template<class Function>
    void AllRulesThatCouldUnify(HtnTerm *targetTerm, Function func) const
    {
        // Go through all rules in the shared ruleset
        for(HtnSharedRules::RulesType::const_iterator ruleIter = m_sharedRules->allRules().begin(); ruleIter != m_sharedRules->allRules().end(); ++ruleIter)
        {
            // if this rule is a fact it might have been deleted
            if(ruleIter->IsFact())
            {
                FactsDiffType::const_iterator found = m_factsDiff.find(ruleIter->head()->GetUniqueID());
                if(found != m_factsDiff.end())
                {
                    if(!found->second.isAdd)
                    {
                        // fact was deleted
                        continue;
                    }
                    else
                    {
                        // Rule was deleted and added, thus it should properly show up later in the m_factsDiff loop
                        // since it is no longer from the original document
                        continue;
                    }
                }
            }
            
            if(CanPotentiallyUnify(targetTerm, ruleIter->head().get()))
            {
                if(!func(*ruleIter)) { break; }
            }
        }
        
        // Go through all the currently active additions in the order they were added
        for(const FactsAdditionsType::value_type &item : m_factAdditions)
        {
            if(CanPotentiallyUnify(targetTerm, item.second->head().get()))
            {
                if(!func(*item.second)) { break; }
            }
        }
    }
    
    // Needs to return all the rules in the order they were added (i.e. order they were declared)
    // Also this needs to be very quick since it is called often
    template<class Function>
    void AllRules(Function func) const
    {
        // Go through all rules in the shared ruleset
        for(HtnSharedRules::RulesType::const_iterator ruleIter = m_sharedRules->allRules().begin(); ruleIter != m_sharedRules->allRules().end(); ++ruleIter)
        {
            // if this rule is a fact it might have been deleted
            if(ruleIter->IsFact())
            {
                FactsDiffType::const_iterator found = m_factsDiff.find(ruleIter->head()->GetUniqueID());
                if(found != m_factsDiff.end())
                {
                    if(!found->second.isAdd)
                    {
                        // fact was deleted
                        continue;
                    }
                    else
                    {
                        // Rule was deleted and added, thus it should properly show up later in the m_factsDiff loop
                        // since it is no longer from the original document
                        continue;
                    }
                }
            }
            
            if(!func(*ruleIter)) { break; }
        }
        
        // Go through all the currently active additions in the order they were added
        for(const FactsAdditionsType::value_type &item : m_factAdditions)
        {
            if(!func(*item.second)) { break; }
        }
    }
    bool CanPotentiallyUnify(const HtnTerm *term, const HtnTerm *ruleHead) const;
    void ClearAll();
    std::shared_ptr<HtnRuleSet> CreateNextState(HtnTermFactory *factory, const std::vector<std::shared_ptr<HtnTerm>> &factsToRemove, const std::vector<std::shared_ptr<HtnTerm>> &factsToAdd);
    std::shared_ptr<HtnRuleSet> CreateSharedRulesCopy();
    std::shared_ptr<HtnRuleSet> CreateCopy();
    int64_t dynamicSize() { return m_dynamicSize; };
    int64_t dynamicSharedSize() { return m_sharedRules->dynamicSize(); };
    // Equivalent means same name and number of arguments
    bool HasEquivalentRule(std::shared_ptr<HtnTerm> term) const;
    bool HasFact(std::shared_ptr<HtnTerm> term) const;
    // Very inefficient, but useful for tests
    bool DebugHasRule(const std::string &head, const std::string &tail) const;
    void LockRules() { m_sharedRules->Lock(); }
    std::string ToStringFacts() const;
    std::string ToStringFactsProlog() const;
    void Update(HtnTermFactory *factory, const std::vector<std::shared_ptr<HtnTerm>> &factsToRemove, const std::vector<std::shared_ptr<HtnTerm>> &factsToAdd);

private:
    // RuleSets conserve memory by sharing the base ruleset and only making copies of the changes if a copy is made
    class HtnSharedRules
    {
    public:
        // This is a list because:
        // - we want to guarantee that the pointers to these don't move around as items are added
        // - we need to guarantee allRules() returns them in the order added
        typedef std::list<HtnRule> RulesType;
        typedef std::set<HtnTerm::HtnTermID> RuleHeadsType;
        typedef std::set<std::string> RulesIndexType;

        HtnSharedRules() : m_dynamicSize(sizeof(HtnSharedRules)), m_isLocked(false) {}
        const RulesType &allRules() { return m_rules; }
        void AddRule(std::shared_ptr<HtnTerm> head, std::vector<std::shared_ptr<HtnTerm>> m_tail);
        void ClearAll();
        int64_t dynamicSize() { return m_dynamicSize; }
        bool HasFact(std::shared_ptr<HtnTerm> term) const;
        void Lock() { m_isLocked = true; }

    private:
        friend class HtnRuleSet;
        int64_t m_dynamicSize;
        bool m_isLocked;
        // Needed for fast checking if ground rules are unique
        RuleHeadsType m_ruleHeads;
        // For checking if rules exist quickly
        RulesIndexType m_ruleIndex;
        RulesType m_rules;
    };

    // Must use Copy() so we can do lock the the state
    HtnRuleSet(const HtnRuleSet &other) = default;
    
    // *** Remember to update dynamicSize() if you change any member variables!
    int64_t m_dynamicSize;
    
    // We are modeling diffs to facts here. Since facts are either true, or don't exist,
    // There are only two differences:
    // 1. Is a new value that wasn't in database before
    // 3. It was a fact that existed that is now deleted
    // We track this with the first bool in the pair<>: true means added, false means deleted
    // Because there can only ever be one fact with the same exact values, we don't need multimap here
    // This is a map because:
    // - we want to guarantee that the pointers to these don't move around as items are added since the HtnRule pointer is handed out
    class FactDiffItem
    {
    public:
        FactDiffItem(bool isAddArg, int diffOrderArg, std::shared_ptr<HtnRule> ruleArg) :
            isAdd(isAddArg), diffOrder(diffOrderArg), rule(ruleArg)
        {
        }
        bool isAdd;
        int diffOrder;
        std::shared_ptr<HtnRule> rule;
    };
    typedef std::map<HtnTerm::HtnTermID, FactDiffItem> FactsDiffType;
    FactsDiffType m_factsDiff;
    // This is solely here to maintain the order of the facts that get added. The latest adds should get returned last
    int m_factsOrder;
    typedef std::map<int, std::shared_ptr<HtnRule>> FactsAdditionsType;
    FactsAdditionsType m_factAdditions;
    std::shared_ptr<HtnSharedRules> m_sharedRules;
};

#endif /* HtnRuleSet_hpp */
