//
//  HtnGoalResolver.hpp
//  GameLib
//
//  Created by Eric Zinda on 9/29/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//

#ifndef HtnGoalResolver_hpp
#define HtnGoalResolver_hpp
#include <map>
#include <functional>
#include "FXPlatform/FailFast.h"
#include "HtnRule.h"
#include "HtnTerm.h"
enum class ResolveContinuePoint;
class ResolveNode;
class ResolveState;
class HtnRuleSet;
class HtnTerm;

// UnifierItemType means assignment where pair.first = pair.second
typedef std::pair<std::shared_ptr<HtnTerm>, std::shared_ptr<HtnTerm>> UnifierItemType;
typedef std::vector<UnifierItemType> UnifierType;
typedef std::pair<std::shared_ptr<HtnRule>, UnifierType> RuleBindingType;

enum class CustomRuleArgType
{
    Unknown = 0,
    
    // These are the Base Types that an argument can be
    // Required to be arithmetic
    Arithmetic,
    // Is a variable
    Variable,
    // Is a term that should be solved
    ResolvedTerm,
    // Is not solved (but *may* be unified)
    Term,
    // Is a term like <=() that will be handled in a special way,
    // The SolvedTerms inside should be handled as CustomRuleArgType::SolvedTerms
    TermOfResolvedTerms,

    // All of these are variadic arguments that describe "all the rest" of
    // the arguments and must be last:
    // All the rest are terms that should be solved
    SetOfResolvedTerms,
    // Set of terms that are not solved (but *may* be unified)
    SetOfTerms
};

// Performs Prolog-style resolution of a set of terms against a database of rules as well as unification 
class HtnGoalResolver : public std::enable_shared_from_this<HtnGoalResolver>
{
public:
    typedef std::pair<std::vector<CustomRuleArgType>, std::function<void(ResolveState *)>> CustomRuleType;

    HtnGoalResolver();
    void AddCustomRule(const std::string &name, CustomRuleType);
    static std::shared_ptr<HtnTerm> ApplyUnifierToTerm(HtnTermFactory *termFactory, UnifierType unifier, std::shared_ptr<HtnTerm>term);
    // Converts an argument into one of the base CustomRuleArgTypes
    static CustomRuleArgType GetCustomRuleArgBaseType(std::vector<CustomRuleArgType> metadata, int argIndex);
    static std::shared_ptr<std::vector<RuleBindingType>> FindAllRulesThatUnify(HtnTermFactory *termFactory, HtnRuleSet *prog, std::shared_ptr<HtnTerm> goal, int *uniquifier, int indentLevel, int memoryBudget, bool fullTrace, int64_t *highestMemoryUsedReturn);
    static std::shared_ptr<HtnTerm> FindTermEquivalence(const UnifierType &unifier, const HtnTerm &termToFind);
    static bool IsGround(UnifierType *unifier);
    bool GetCustomRule(const std::string &name, int arity, HtnGoalResolver::CustomRuleType &metadata);
    // Always check factory->outOfMemory() after calling to see if we ran out of memory during processing and the resolutions might not be complete
    std::shared_ptr<std::vector<UnifierType>> ResolveAll(HtnTermFactory *termFactory, HtnRuleSet *prog, const std::vector<std::shared_ptr<HtnTerm>> &initialGoals, int initialIndent = 0, int memoryBudget = 1000000, int64_t *highestMemoryUsedReturn = nullptr, int *furthestFailureIndex = nullptr, std::vector<std::shared_ptr<HtnTerm>> *farthestFailureContext = nullptr);
    // Always check factory->outOfMemory() after calling to see if we ran out of memory during processing and the resolutions might not be complete
    std::shared_ptr<UnifierType> ResolveNext(ResolveState *state);
    static std::shared_ptr<HtnTerm> SubstituteUnifiers(HtnTermFactory *factory, const UnifierType &source, std::shared_ptr<HtnTerm> target);
    static std::shared_ptr<UnifierType> SubstituteUnifiers(HtnTermFactory *factory, const UnifierType &source, const UnifierType &destination);
    static std::shared_ptr<std::vector<std::shared_ptr<HtnTerm>>> SubstituteUnifiers(HtnTermFactory *factory, const UnifierType &source, const std::vector<std::shared_ptr<HtnTerm>> &terms);
    static std::string ToString(const std::vector<UnifierType> *unifierList, bool json = false);
    static std::string ToString(const UnifierType &unifier, bool json = false);
    static std::shared_ptr<UnifierType> Unify(HtnTermFactory *factory, std::shared_ptr<HtnTerm> term1, std::shared_ptr<HtnTerm> term2);

private:
    static void RuleAggregate(ResolveState *state);
	static void RuleAssert(ResolveState* state);
    static void RuleAtomChars(ResolveState* state);
    static void RuleAtomConcat(ResolveState* state);
    static void RuleAtomDowncase(ResolveState* state);
    static void RuleCount(ResolveState *state);
    static void RuleDistinct(ResolveState *state);
    static void RuleFailureContext(ResolveState *state);
    static void RuleFindAll(ResolveState *state);
    static void RuleFirst(ResolveState *state);
    static void RuleForAll(ResolveState *state);
    static void RuleIs(ResolveState *state);
    static void RuleIsAtom(ResolveState* state);
    static void RuleNewline(ResolveState *state);
    static void RuleNot(ResolveState *state);
    static void RulePrint(ResolveState *state);
	static void RuleRetract(ResolveState* state);
    static void RuleRetractAll(ResolveState* state);
	static void RuleSortBy(ResolveState *state);
    static void RuleTermCompare(ResolveState *state);
    static void RuleTrace(ResolveState *state);
    static void RuleUnify(ResolveState *state);
    static void RuleWrite(ResolveState *state);
    static void SubstituteAllVariables(HtnTermFactory *factory, std::shared_ptr<HtnTerm> newTerm, std::shared_ptr<HtnTerm> existingVariable, std::vector<std::pair<std::shared_ptr<HtnTerm>, std::shared_ptr<HtnTerm>>> &stack, UnifierType &solution);

    typedef std::map<std::string, CustomRuleType> CustomRulesType;
    CustomRulesType m_customRules;
};

enum class ResolveContinuePoint
{
    CustomStart,
    CustomContinue1,
    CustomContinue2,
    CustomContinue3,
    CustomContinue4,
	Cut,
    NextGoal,
    NextRuleThatUnifies,
    ProgramError,
    Return
};

class ResolveNode
{
public:
    typedef std::set<std::shared_ptr<HtnTerm>, HtnTermComparer> TermSetType;
    ResolveNode(std::shared_ptr<std::vector<std::shared_ptr<HtnTerm>>> resolventArg, std::shared_ptr<UnifierType> unifierArg);
	static void AddNewGoalsToResolvent(HtnTermFactory* termFactory, std::vector<std::shared_ptr<HtnTerm>>::const_reverse_iterator startIter, std::vector<std::shared_ptr<HtnTerm>>::const_reverse_iterator endIter, std::shared_ptr<std::vector<std::shared_ptr<HtnTerm>>> existingResolvent, int* uniquifier);
    void AddToSolutions(std::shared_ptr<std::vector<UnifierType>> &solutions);
    static std::shared_ptr<ResolveNode> CreateInitialNode(const std::vector<std::shared_ptr<HtnTerm>> &resolventArg, const UnifierType &unifierArg);
    std::shared_ptr<ResolveNode> CreateChildNode(HtnTermFactory *termFactory, const std::vector<std::shared_ptr<HtnTerm>> &originalGoals, const std::vector<std::shared_ptr<HtnTerm>> &additionalResolvents, const UnifierType &additionalSubstitution, int* uniquifier);
    std::shared_ptr<HtnTerm> currentGoal()
    {
        return (m_resolvent == nullptr || m_resolvent->size() == 0) ? nullptr : (*m_resolvent)[0];
    }
    
    RuleBindingType currentRule()
    {
        FailFastAssert(rulesThatUnify != nullptr && currentRuleIndex < rulesThatUnify->size());
        return (*rulesThatUnify)[currentRuleIndex];
    }

    int64_t dynamicSize()
    {
        if(cachedDynamicSize == -1)
        {
            CalcDynamicSize();
        }
        
        return cachedDynamicSize;
    }
    
    void CalcDynamicSize()
    {
        int64_t rulesThatUnifySize = 0;
        if(rulesThatUnify != nullptr)
        {
            rulesThatUnifySize = sizeof(rulesThatUnify);
            for(auto rule : *rulesThatUnify)
            {
                rulesThatUnifySize += sizeof(RuleBindingType) + rule.second.size() * sizeof(UnifierItemType);
            }
        }

        int64_t previousSolutionsSize = 0;
        if(previousSolutions != nullptr)
        {
            previousSolutionsSize = sizeof(previousSolutions);
            for(auto item : *previousSolutions)
            {
                previousSolutionsSize += sizeof(UnifierType) + item.size() * sizeof(UnifierItemType);
            }
        }

        int64_t variablesToKeepSize = 0;
        if(variablesToKeep != nullptr)
        {
            variablesToKeepSize = sizeof(variablesToKeep);
            for(auto item : *variablesToKeep)
            {
                variablesToKeepSize += sizeof(HtnTerm) + item->dynamicSize();
            }
        }

        cachedDynamicSize = sizeof(ResolveNode) +
            (m_resolvent == nullptr ? 0 : sizeof(m_resolvent) + m_resolvent->size() * sizeof(std::shared_ptr<HtnTerm>)) +
            rulesThatUnifySize +
            (unifier == nullptr ? 0 : sizeof(unifier) + unifier->size() * sizeof(UnifierItemType)) +
            (previousSolutions == nullptr ? 0 : sizeof(previousSolutions) + unifier->size() * sizeof(UnifierItemType)) +
            currentFailureContext.size() * sizeof(std::shared_ptr<HtnTerm>) +
            previousSolutionsSize +
            variablesToKeepSize;
    }
    
	bool IsLastGoalInResolvent()
	{
		return currentGoal() == nullptr || m_resolvent->size() == 1;
	}

    void PopStandaloneResolve(ResolveState *state);
	void PushStandaloneResolve(ResolveState* state, std::shared_ptr<TermSetType> additionalVariablesToKeep, std::vector<std::shared_ptr<HtnTerm>>::const_reverse_iterator startIter, std::vector<std::shared_ptr<HtnTerm>>::const_reverse_iterator endIter, ResolveContinuePoint continuePointArg);
    static std::shared_ptr<UnifierType> RemoveUnusedUnifiers(std::shared_ptr<TermSetType> variablesToKeep, const UnifierType &currentUnifiers, const std::vector<std::shared_ptr<HtnTerm>> &originalGoals, const std::vector<std::shared_ptr<HtnTerm>> &resolvent);
    int CountOfGoalsLeftToProcess()
    {
        // If the current goal is original, don't count it. We want the ones that are not "in progress"
        return originalGoalCount;
    }
    
	// If we get cut, it means stop processing alternatives for this node
	void SetCut()
	{
		isCut = true;
		if(rulesThatUnify != nullptr)
		{
			currentRuleIndex = (int)rulesThatUnify->size();
		}
	}

    bool SetNextRule()
    {
        currentRuleIndex++;
        return rulesThatUnify != nullptr && currentRuleIndex < rulesThatUnify->size();
    }
    
    // NOTE: If you change members, remember to change dynamicSize() function too
    ResolveContinuePoint continuePoint;
    std::vector<std::shared_ptr<HtnTerm>> currentFailureContext;
    int currentRuleIndex;
    // Remembers the count of original goals which will be at the end of m_resolvent, so we can debug better
    int originalGoalCount;
    const std::shared_ptr<std::vector<std::shared_ptr<HtnTerm>>> &resolvent() const { return m_resolvent; };
    std::shared_ptr<std::vector<RuleBindingType>> rulesThatUnify;
    std::shared_ptr<UnifierType> unifier;
    
private:
    void PopResolver(ResolveState *state);
    void PushResolver(ResolveState *state);
    
    // NOTE: If you change members, remember to change dynamicSize() function too
    // The size of a node doesn't change unless it is being actively worked on, so we cache it
    int64_t cachedDynamicSize;
	bool isCut;
    bool isStandaloneResolve; // True for all child nodes of a standalone resolve
    std::shared_ptr<std::vector<UnifierType>> previousSolutions;
    std::shared_ptr<TermSetType> variablesToKeep;
    bool previousCollectAllSolutions;
    bool pushedStandaloneResolver;
    std::shared_ptr<std::vector<std::shared_ptr<HtnTerm>>> m_resolvent;
};

class ResolveState
{
public:
    ResolveState(HtnTermFactory *termFactoryArg, HtnRuleSet *progArg, const std::vector<std::shared_ptr<HtnTerm>> &initialResolventArg, int initialIndentArg, int memoryBudgetArg);
    void ClearFailures()
    {
        deepestFailure = -1;
        deepestFailureGoal = nullptr;
        deepestFailureOriginalGoalIndex = -1;
        farthestFailureDepth = -1;
        farthestFailureOriginalGoalIndex = -1;
        farthestFailureContext.clear();
    }
    std::string GetStackString();
    int64_t RecordMemoryUsage(int64_t &initialTermMemory, int64_t &initialRuleSetMemory);
    void RecordFailure(std::shared_ptr<HtnTerm> goal, std::shared_ptr<ResolveNode> currentNode);
    static void RecoverInitialVariables(HtnTermFactory *termFactory, UnifierType &unifier);
    static std::vector<std::shared_ptr<HtnTerm>> ReplaceInitialVariables(HtnTermFactory *termFactory, const std::vector<std::shared_ptr<HtnTerm>> &initialGoals);
    std::shared_ptr<UnifierType> SimplifySolution(const UnifierType &solution, std::vector<std::shared_ptr<HtnTerm>> &goals);

    int64_t dynamicSize()
    {
        int64_t stackSize = sizeof(std::vector<std::shared_ptr<ResolveNode>>);
        for(int stackIndex = 0; stackIndex < ((int) resolveStack->size()) - 1; ++stackIndex)
        {
            stackSize += (*resolveStack)[stackIndex]->dynamicSize();
        }
        
        // The last one needs to be recalculated since it is being worked on and will change
        if(resolveStack->size() > 0)
        {
            resolveStack->back()->CalcDynamicSize();
            stackSize += resolveStack->back()->dynamicSize();
        }
        
        return sizeof(ResolveState) +
            deepestFailureStack.size() + highestMemoryUsedStack.size() +
            farthestFailureContext.size() * sizeof(std::shared_ptr<HtnTerm>) +
            initialGoals->size() * sizeof(std::shared_ptr<HtnTerm>) +
            stackSize +
            (solutions == nullptr ? 0 : (sizeof(std::vector<UnifierType>) + solutions->size() * sizeof(UnifierItemType)));
    }

    // NOTE: If you change members, remember to change dynamicSize() function too
    bool collectAllSolutions;
    int deepestFailure;
    std::shared_ptr<HtnTerm> deepestFailureGoal;
    int deepestFailureOriginalGoalIndex;
    int farthestFailureDepth;
    int farthestFailureOriginalGoalIndex;
    std::vector<std::shared_ptr<HtnTerm>> farthestFailureContext;
    std::string deepestFailureStack;
    bool fullTrace;
    int64_t highestMemoryUsed;
    std::string highestMemoryUsedStack;
    std::shared_ptr<std::vector<std::shared_ptr<HtnTerm>>> initialGoals;
    int initialIndent;
    int memoryBudget;
    HtnRuleSet *prog;
    std::shared_ptr<std::vector<std::shared_ptr<ResolveNode>>> resolveStack;
    int64_t ruleMemoryUsed;
    std::shared_ptr<std::vector<UnifierType>> solutions;
    int64_t stackMemoryUsed;
    HtnTermFactory *termFactory;
    int64_t termMemoryUsed;
    int uniquifier;
    

};
#endif /* HtnGoalResolver_hpp */
