//
//  HtnPlanner.hpp
//  GameLib
//
//  Created by Eric Zinda on 1/7/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef HtnPlanner_hpp
#define HtnPlanner_hpp

#include "HtnDomain.h"
#include "FXPlatform/Prolog/HtnGoalResolver.h"
#include <memory>
#include <vector>
class HtnMethod;
enum class HtnMethodType;
class HtnOperator;
class HtnTerm;
class HtnTermFactory;
class PlanNode;
enum class PlanNodeContinuePoint;

// State of the planner.  Is a separate class so the caller can call back for more plans or just get the first one
// Note: could be enhanced easily to allow the planner to be called *very* iteratively so that it performs one while loop per call
// This would be better for games that want to move the plan forward a tiny bit every frame, for example, since the calculation is very bounded
class PlanState
{
public:
    PlanState(HtnTermFactory *factoryArg, std::shared_ptr<HtnRuleSet> initialState, const std::vector<std::shared_ptr<HtnTerm>> &initialGoals, int64_t memoryBudgetArg);

private:
    // No public access to data only used by implentation of planner
    friend class HtnPlanner;
    friend class PlanNode;

    void CheckHighestMemory(int64_t currentMemory, std::string extra1Name, int64_t extra1Size);
    int64_t dynamicSize();

    // *** Remember to update dynamicSize() if you change any member variables!
    HtnTermFactory *factory;
    int64_t highestMemoryUsed;
    std::shared_ptr<HtnRuleSet> initialState;
    double startTimeSeconds;
    int64_t memoryBudget;
    int nextNodeID;
    bool returnValue;
    std::shared_ptr<std::vector<std::shared_ptr<PlanNode>>> stack;
};


class HtnPlanner : public HtnDomain
{
public:
    typedef std::vector<std::shared_ptr<HtnTerm>> GoalsType;
    typedef multimap<HtnTerm::HtnTermID, HtnMethod *> MethodsType;
    // Operators are indexed by their name only, not their name and arguments
    typedef map<string, HtnOperator *> OperatorsType;
    class SolutionType
    {
    public:
        SolutionType(const SolutionType &other) = default;
        SolutionType() {}
        SolutionType(std::vector<std::shared_ptr<HtnTerm>> operators, std::shared_ptr<HtnRuleSet> ruleSet) :
            first(operators),
            second(ruleSet)
        {
        }
        std::vector<std::shared_ptr<HtnTerm>> operators() { return first; }
        std::shared_ptr<HtnRuleSet> finalState() { return second; }
        
        // Public, and named first and second just for backwards compat with previous code
        std::vector<std::shared_ptr<HtnTerm>> first;
        std::shared_ptr<HtnRuleSet> second;
        double elapsedSeconds;
        int64_t highestMemoryUsed;
    };
    typedef std::vector<std::shared_ptr<SolutionType>> SolutionsType;
    
    HtnPlanner();
    virtual ~HtnPlanner();
    // Safe to call from another thread
    static void Abort() { m_abort = true; }
    virtual HtnMethod *AddMethod(std::shared_ptr<HtnTerm> head, const std::vector<std::shared_ptr<HtnTerm>> &condition, const std::vector<std::shared_ptr<HtnTerm>> &tasks, HtnMethodType methodType, bool isDefault);
    virtual HtnOperator *AddOperator(std::shared_ptr<HtnTerm>head, const std::vector<std::shared_ptr<HtnTerm>> &addList, const std::vector<std::shared_ptr<HtnTerm>> &deleteList, bool hidden = false);
    virtual void ClearAll();
    // Always check factory->outOfMemory() after calling to see if we ran out of memory during processing and the plan might not be complete
    std::shared_ptr<SolutionsType> FindAllPlans(HtnTermFactory *factory, std::shared_ptr<HtnRuleSet> initialState, const std::vector<std::shared_ptr<HtnTerm>> &initalGoals, int memoryBudget = 5000000);
    // Always check factory->outOfMemory() after calling to see if we ran out of memory during processing and the plan might not be complete
    std::shared_ptr<SolutionType> FindPlan(HtnTermFactory *factory, std::shared_ptr<HtnRuleSet> initialState, std::vector<std::shared_ptr<HtnTerm>> &initialGoals, int memoryBudget = 5000000);
    // Always check factory->outOfMemory() after calling to see if we ran out of memory during processing and the plan might not be complete
    std::shared_ptr<SolutionType> FindNextPlan(PlanState *planState);
    bool HasGoal(const std::string &term);
    // Very inefficient but useful for testing
    bool DebugHasMethod(const std::string &head, const std::string &constraints, const std::string &tasks);
    bool HasOperator(const std::string &head, const std::string &deletions, const std::string &additions);
    static std::string ToStringSolution(std::shared_ptr<SolutionType> solution);
    static std::string ToStringSolutions(std::shared_ptr<SolutionsType> solutions);
    static std::string ToStringFacts(std::shared_ptr<SolutionType> solution);
    static std::string ToStringFacts(std::shared_ptr<SolutionsType> solutions);

    HtnGoalResolver *goalResolver() { return m_resolver.get(); }
    virtual void AllMethods(std::function<bool(HtnMethod *)> handler)
    {
        for(auto method : m_methods)
        {
            if(!handler(method.second))
            {
                break;
            }
        }
    }

    virtual void AllOperators(std::function<bool(HtnOperator *)> handler)
    {
        for(auto op : m_operators)
        {
            if(!handler(op.second))
            {
                break;
            }
        }
    }

private:
    bool CheckForOperator(PlanState *planState);
    bool CheckForSpecialTask(PlanState *planState);
    std::shared_ptr<std::vector<pair<HtnMethod *, UnifierType>>> FindAllMethodsThatUnify(HtnTermFactory *termFactory, HtnRuleSet *prog, std::shared_ptr<HtnTerm> goal);
    std::shared_ptr<PlanNode> FindNodeWithID(std::vector<std::shared_ptr<PlanNode>> &stack, int id);
    void HandleAllOf(PlanState *planState);
    void HandleAnyOf(PlanState *planState);
    void Return(PlanState *planState, bool returnValue);
    std::shared_ptr<HtnPlanner::SolutionType> SolutionFromCurrentNode(PlanState *planState, std::shared_ptr<PlanNode> node);

    // *** Remember to update dynamicSize() if you change any member variables!
    // Awful hack making this static. necessary because it was too late in schedule to properly plumb through an Abort
    static uint8_t m_abort;
    MethodsType m_methods;
    int m_nextDocumentOrder;
    OperatorsType m_operators;
    shared_ptr<HtnGoalResolver> m_resolver;
    int64_t m_dynamicSize;
};

#endif /* HtnPlanner_hpp */
