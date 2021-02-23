//
//  HtnGoalResolver.cpp
//  GameLib
//
//  Created by Eric Zinda on 9/29/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//
#include <iostream>
#include <algorithm>
#include "Logger.h"
#include "HtnArithmeticOperators.h"
#include "HtnGoalResolver.h"
#include "HtnRuleSet.h"
#include "HtnTerm.h"
#include "HtnTermFactory.h"
#include <cctype>
#include <cwctype>
#include <stack>
#include <locale> 
const int indentSpaces = 11;
const string initialVariablePrefix = "orig*";

#define Trace0(status, trace, indent, fullTrace) \
TraceString("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic));

#define Trace1(status, trace, indent, fullTrace, arg1) \
TraceString1("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic), \
arg1);

#define Trace2(status, trace, indent, fullTrace, arg1, arg2) \
TraceString2("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic), \
arg1, arg2);

#define Trace3(status, trace, indent, fullTrace, arg1, arg2, arg3) \
TraceString3("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic), \
arg1, arg2, arg3);

#define Trace4(status, trace, indent, fullTrace, arg1, arg2, arg3, arg4) \
TraceString4("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic), \
arg1, arg2, arg3, arg4);

#define Trace5(status, trace, indent, fullTrace, arg1, arg2, arg3, arg4, arg5) \
TraceString5("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic), \
arg1, arg2, arg3, arg4, arg5);

#define Trace6(status, trace, indent, fullTrace, arg1, arg2, arg3, arg4, arg5, arg6) \
TraceString6("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic), \
arg1, arg2, arg3, arg4, arg5, arg6);

#define Trace7(status, trace, indent, fullTrace, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
TraceString7("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic), \
arg1, arg2, arg3, arg4, arg5, arg6, arg7);

#define Trace8(status, trace, indent, fullTrace, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
TraceString8("HtnGoalResolver::Resolve " + string((indent) * indentSpaces, ' ') + status + trace, \
SystemTraceType::Solver, (fullTrace ? TraceDetail::Normal :TraceDetail::Diagnostic), \
arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);

ResolveNode::ResolveNode(shared_ptr<vector<shared_ptr<HtnTerm>>> resolventArg, shared_ptr<UnifierType> unifierArg) :
    continuePoint(ResolveContinuePoint::NextGoal),
    currentRuleIndex(-1),
    originalGoalCount((int) resolventArg->size() - 1),
    unifier(unifierArg),
    cachedDynamicSize(-1),
	isCut(false),
    isStandaloneResolve(false),
    previousCollectAllSolutions(false),
    pushedStandaloneResolver(false),
    m_resolvent(resolventArg)
{
}

void ResolveNode::AddToSolutions(shared_ptr<vector<UnifierType>> &solutions)
{
    if(solutions == nullptr)
    {
        solutions = shared_ptr<vector<UnifierType>>(new vector<UnifierType>());
    }
    
    solutions->push_back(*unifier);
}

shared_ptr<ResolveNode> ResolveNode::CreateInitialNode(const vector<shared_ptr<HtnTerm>> &resolventArg, const UnifierType &unifierArg)
{
    shared_ptr<vector<shared_ptr<HtnTerm>>> initialResolvent = shared_ptr<vector<shared_ptr<HtnTerm>>>(new vector<shared_ptr<HtnTerm>>(resolventArg));
    shared_ptr<UnifierType> initialUnifier = shared_ptr<UnifierType>(new UnifierType(unifierArg));
    shared_ptr<ResolveNode> initialNode = shared_ptr<ResolveNode>(new ResolveNode(initialResolvent, initialUnifier));
    return initialNode;
}

void ResolveNode::AddNewGoalsToResolvent(HtnTermFactory* termFactory, vector<shared_ptr<HtnTerm>>::const_reverse_iterator startIter, vector<shared_ptr<HtnTerm>>::const_reverse_iterator endIter, shared_ptr<vector<shared_ptr<HtnTerm>>> existingResolvent, int* uniquifier)
{
	// New goals must be inserted at the beginning since Prolog does a depth first search, otherwise programs that expect
	// AND clauses to be evaluated from left to right won't work properly
	shared_ptr<HtnTerm> cutEnd;
	string cutIDString;
	for(auto resolventIter = startIter; resolventIter != endIter; ++resolventIter)
	{
		// If this node represents a rule that contains a cut that we might hit, we need to wrap the resolvent in fake items so we know
		// where to jump back to if the cut happens. Replace all cut terms with a cutEnd with a matching ID
		if((*resolventIter)->isCut())
		{
			// Only create the terms if we need them
			if(cutEnd == nullptr)
			{
				cutIDString = lexical_cast<string>(*uniquifier);
				(*uniquifier) = (*uniquifier) + 1;

				cutEnd = termFactory->CreateConstantFunctor("!<", { cutIDString });
			}

			existingResolvent->insert(existingResolvent->begin(), cutEnd);
		}
		else
		{
			existingResolvent->insert(existingResolvent->begin(), *resolventIter);
		}
	}


	// If there was a cut, at a cutStart term at the front so we know where to cut to
	if(cutEnd != nullptr)
	{
		shared_ptr<HtnTerm> cutStart = termFactory->CreateConstantFunctor("!>", { cutIDString });
		existingResolvent->insert(existingResolvent->begin(), cutStart);
	}
}

shared_ptr<ResolveNode> ResolveNode::CreateChildNode(HtnTermFactory *termFactory, const vector<shared_ptr<HtnTerm>> &originalGoals, const vector<shared_ptr<HtnTerm>> &additionalResolvents, const UnifierType &additionalSubstitution, int* uniquifier)
{
    // We should never create a child of a node that has no resolvent
    FailFastAssert(m_resolvent != nullptr && m_resolvent->size() > 0);
    
    // If we are a standalone resolve, the originalGoalCount never changes since we are resolving a single goal
    int originalGoalsLeft = originalGoalCount;
    if(!isStandaloneResolve)
    {
        // If we created a child of a node that had only original goals left and there are no new resolvents, we must have resolved the first one, thus we have one fewer
        if((((int) m_resolvent->size() - 1) == originalGoalCount) && additionalResolvents.size() == 0)
        {
            // We are now going to resolve an original goal since we ran out of new ones, so reduce the count by 1
            --originalGoalsLeft;
        }
    }
    
    // The child we will spawn has a job to resolve all the additional goals from whatever we unified with + the remainder of the goals in this node's resolvent
    // We skip over the current goal since that was this node's job to deal with
    // (BTW: the goals could be empty if it was just a fact, which is fine)
    shared_ptr<vector<shared_ptr<HtnTerm>>> childResolvent = shared_ptr<vector<shared_ptr<HtnTerm>>>(new vector<shared_ptr<HtnTerm>>());
    childResolvent->insert(childResolvent->begin(), ++(m_resolvent->begin()), m_resolvent->end());
    
	// Now add the new ones
	AddNewGoalsToResolvent(termFactory, additionalResolvents.rbegin(), additionalResolvents.rend(), childResolvent, uniquifier);

    // Create the child unifiers:
    // - Substitute unifier for this new unification with currentUnifier
    // - Add the new unifiers onto the end as well since they have potentially bound new variables
    shared_ptr<UnifierType> childUnifier = HtnGoalResolver::SubstituteUnifiers(termFactory, additionalSubstitution, *unifier);
    childUnifier->insert(childUnifier->end(), additionalSubstitution.begin(), additionalSubstitution.end());
    
    // Simplify the unifiers to not include any that can't possibly be used so we don't waste memory
    shared_ptr<UnifierType> simplifiedUnifier = RemoveUnusedUnifiers(variablesToKeep, *childUnifier, originalGoals, *childResolvent);
    
    // Apply new substitutions to new Resolvent
    for(auto assignment : additionalSubstitution)
    {
        for(shared_ptr<HtnTerm> &goal : *childResolvent)
        {
            goal = goal->SubstituteTermForVariable(termFactory, assignment.second, assignment.first);
        }
    }
    
    shared_ptr<ResolveNode> newNode = shared_ptr<ResolveNode>(new ResolveNode(childResolvent, simplifiedUnifier));
    newNode->currentFailureContext = currentFailureContext;
    newNode->originalGoalCount = originalGoalsLeft;
    newNode->isStandaloneResolve = isStandaloneResolve;
    newNode->variablesToKeep = variablesToKeep;

    return newNode;
}

void ResolveNode::PopResolver(ResolveState *state)
{
    FailFastAssert(pushedStandaloneResolver);
    
    state->solutions = previousSolutions;
    state->collectAllSolutions = previousCollectAllSolutions;
    
    previousSolutions = nullptr;
    previousCollectAllSolutions = false;
    pushedStandaloneResolver = false;
}

void ResolveNode::PopStandaloneResolve(ResolveState *state)
{
    PopResolver(state);
}

void ResolveNode::PushResolver(ResolveState *state)
{
    // Make sure we only have one active at a time since it is not really a stack
    FailFastAssert(!pushedStandaloneResolver);
    
    previousCollectAllSolutions = state->collectAllSolutions;
    previousSolutions = state->solutions;
    pushedStandaloneResolver = true;
    
    state->collectAllSolutions = false;
    state->solutions = nullptr;
}

// When can we discard variables?
// - When there are no more references in the resolvent because we can't possibly add more references to it
// - But: we need to make sure we don't remove anyt that were in the original goals because they are part of the solution
shared_ptr<UnifierType> ResolveNode::RemoveUnusedUnifiers(shared_ptr<TermSetType> variablesToKeep, const UnifierType &currentUnifiers, const vector<shared_ptr<HtnTerm>> &originalGoals, const vector<shared_ptr<HtnTerm>> &resolvent)
{
    set<string> variables;
    
    // Start with any variables already know we want
    if(variablesToKeep != nullptr)
    {
        for(shared_ptr<HtnTerm> term : *variablesToKeep)
        {
            variables.insert(term->name());
        }
    }
    
    // get all the variables that were in the originalGoals
    for(shared_ptr<HtnTerm> term : originalGoals)
    {
        vector<string> termVariables;
        term->GetAllVariables(&termVariables);
        variables.insert(termVariables.begin(), termVariables.end());
    }
    
    // Then get the ones in the resolvent
    for(shared_ptr<HtnTerm> term : resolvent)
    {
        vector<string> termVariables;
        term->GetAllVariables(&termVariables);
        variables.insert(termVariables.begin(), termVariables.end());
    }
    
    // Then: remove any assignments that don't involve those variables
    shared_ptr<UnifierType> simplifiedUnifiers = shared_ptr<UnifierType>(new UnifierType());
    for(UnifierItemType item : currentUnifiers)
    {
        if(variables.find(item.first->name()) != variables.end())
        {
            simplifiedUnifiers->push_back(item);
        }
    }
    
    return simplifiedUnifiers;
}

void ResolveNode::PushStandaloneResolve(ResolveState *state, shared_ptr<TermSetType> additionalVariablesToKeep, vector<shared_ptr<HtnTerm>>::const_reverse_iterator startIter, vector<shared_ptr<HtnTerm>>::const_reverse_iterator endIter, ResolveContinuePoint continuePointArg)
{
    vector<shared_ptr<ResolveNode>> &resolveStack = *state->resolveStack;
    
    // Run the resolver just on the arguments as if it were a standalone resolution.  Then continue on depending on what happens
    shared_ptr<vector<shared_ptr<HtnTerm>>> argumentsAsResolvent = shared_ptr<vector<shared_ptr<HtnTerm>>>(new vector<shared_ptr<HtnTerm>>());
	AddNewGoalsToResolvent(state->termFactory, startIter, endIter, argumentsAsResolvent, &(state->uniquifier));
    Trace1("           ", "resolve standalone: {0}", state->initialIndent + resolveStack.size(), state->fullTrace, HtnTerm::ToString(*argumentsAsResolvent));
    
    shared_ptr<ResolveNode> standaloneNode = ResolveNode::CreateInitialNode(*argumentsAsResolvent, *unifier);
    // which original goal we are resolving never changes when we are doing a standalone resolve since we are just resolving a single set of sub goals
    standaloneNode->isStandaloneResolve = true;
    
    // Keep around requested variables PLUS those that any standalone resolves we are inside of want to keep
    shared_ptr<TermSetType> variablesToKeepCombined = shared_ptr<TermSetType>(new TermSetType());
    if(variablesToKeep != nullptr)
    {
        variablesToKeepCombined->insert(variablesToKeep->begin(), variablesToKeep->end());
    }
    if(additionalVariablesToKeep != nullptr)
    {
        variablesToKeepCombined->insert(additionalVariablesToKeep->begin(), additionalVariablesToKeep->end());
    }
    
    if(variablesToKeepCombined != nullptr)
    {
        standaloneNode->variablesToKeep = variablesToKeepCombined;
    }
    
    standaloneNode->originalGoalCount = originalGoalCount;
    
    resolveStack.push_back(standaloneNode);
    
    PushResolver(state);
    state->collectAllSolutions = true;
    continuePoint = continuePointArg;
}

vector<shared_ptr<HtnTerm>> ResolveState::ReplaceInitialVariables(HtnTermFactory *termFactory, const vector<shared_ptr<HtnTerm>> &initialGoals)
{
    vector<shared_ptr<HtnTerm>> replacedGoals;
    int dontCareCount = 0;
    std::map<std::string, std::shared_ptr<HtnTerm>> variableMap;
    for(auto item : initialGoals)
    {
        shared_ptr<HtnTerm> current = item->MakeVariablesUnique(termFactory, false, initialVariablePrefix, &dontCareCount, variableMap);
        replacedGoals.push_back(current);
    }
    
    return replacedGoals;
}

ResolveState::ResolveState(HtnTermFactory *termFactoryArg, HtnRuleSet *progArg, const vector<shared_ptr<HtnTerm>> &initialResolventArg, int initialIndentArg, int memoryBudgetArg) :
    collectAllSolutions(false),
    deepestFailure(-1),
    deepestFailureOriginalGoalIndex(-1),
    farthestFailureDepth(-1),
    farthestFailureOriginalGoalIndex(-1),
    fullTrace(false),
    highestMemoryUsed(0),
    initialIndent(initialIndentArg),
    memoryBudget(memoryBudgetArg),
    prog(progArg),
    resolveStack(shared_ptr<vector<shared_ptr<ResolveNode>>>(new vector<shared_ptr<ResolveNode>>())),
    ruleMemoryUsed(0),
    stackMemoryUsed(0),
    termFactory(termFactoryArg),
    termMemoryUsed(0),
    uniquifier(0)
{
    // Replace the variable names used in the initial goals with guaranteed unique ones since we do a unification with rules *before*
    // renaming them and this avoids improper matching
    // Also, any "_" variables need to be replaced by unique names so they don't get treated as the same variable (called "_") when we resolve.
    initialGoals = shared_ptr<vector<shared_ptr<HtnTerm>>>(new vector<shared_ptr<HtnTerm>>(ReplaceInitialVariables(termFactoryArg, initialResolventArg)));

    // Start with the initial node on the stack
	shared_ptr<vector<shared_ptr<HtnTerm>>> resolvent = shared_ptr<vector<shared_ptr<HtnTerm>>>(new vector<shared_ptr<HtnTerm>>());
    
	ResolveNode::AddNewGoalsToResolvent(termFactory, initialGoals->rbegin(), initialGoals->rend(), resolvent, &uniquifier);
    resolveStack->push_back(ResolveNode::CreateInitialNode(*resolvent, {}));
}

string ResolveState::GetStackString()
{
    stringstream stackString;
    for(auto item : *resolveStack)
    {
        shared_ptr<HtnTerm> goal = item->currentGoal();
        if(goal == nullptr)
        {
            stackString << "<No Goal>";
        }
        else
        {
            stackString << goal->name() << "/" << goal->arity();
        }
        
        stackString << "(" << lexical_cast<string>(item->dynamicSize()) << ")";
        stackString << " ... ";
    }
    
    return stackString.str();
}

void ResolveState::RecordFailure(shared_ptr<HtnTerm> goal, shared_ptr<ResolveNode> currentNode)
{
    // Which original goal is failing? It is the one *before* the goalsLeftToProcess
    int goalsLeftToProcess = currentNode->CountOfGoalsLeftToProcess();
    FailFastAssert(goalsLeftToProcess >= 0 && goalsLeftToProcess < initialGoals->size());
    int originalGoalIndex = (int) initialGoals->size() - goalsLeftToProcess - 1;
    FailFastAssert(originalGoalIndex >= 0 && originalGoalIndex < initialGoals->size());
    
    shared_ptr<HtnTerm> originalGoalInProgress = (*initialGoals)[originalGoalIndex];
    int stackDepth = (int) resolveStack->size();
    
    // Replace the failure context if we've gotten to a new original goal OR
    // If we've gotten farther along in the current original goal since that will be a better error
    if( (originalGoalIndex > farthestFailureOriginalGoalIndex) ||
        ((originalGoalIndex == farthestFailureOriginalGoalIndex) &&
         ((farthestFailureContext.size() == 0) || // We haven't set a context yet
          (stackDepth > farthestFailureDepth)))) // We've set one but this is deeper
    {
        farthestFailureOriginalGoalIndex = originalGoalIndex;
        farthestFailureContext = currentNode->currentFailureContext;
        farthestFailureDepth = stackDepth;
        Trace1("           ", "RecordFailure Context:  {0}", initialIndent + resolveStack->size(), fullTrace, HtnTerm::ToString(farthestFailureContext));
    }
    
    if((originalGoalIndex != deepestFailureOriginalGoalIndex) || ((stackDepth >= deepestFailure) && (goal != nullptr)))
    {
        deepestFailure = stackDepth;
        deepestFailureGoal = goal;
        deepestFailureOriginalGoalIndex = originalGoalIndex;
        deepestFailureStack = GetStackString();
    }
    
    Trace2("FAIL       ", "originalGoal:{0}, currentSubgoal:{1}", initialIndent + resolveStack->size(), fullTrace, originalGoalInProgress->ToString(), goal->ToString());
}

// Our memory budget is given in terms of how much memory the resolver can consume
// However, Terms and Rules can be created in between calls to the Resolver, thus we can't
// Just add up the total amount consumed by them and compare to the budget. We need to do the *difference*
// Since we have control over our own memory, we can just count that against the total
int64_t ResolveState::RecordMemoryUsage(int64_t &initialTermMemory, int64_t &initialRuleSetMemory)
{
    stackMemoryUsed = dynamicSize();
    int currentTermMemory = (int) termFactory->dynamicSize();
    int currentRuleSetMemory = (int) prog->dynamicSize();
    
    termMemoryUsed += currentTermMemory - initialTermMemory;
    ruleMemoryUsed += currentRuleSetMemory - initialRuleSetMemory;
    
    int64_t totalMemoryUsed = termMemoryUsed + ruleMemoryUsed + stackMemoryUsed;
    if(totalMemoryUsed > highestMemoryUsed)
    {
        highestMemoryUsedStack = GetStackString();        
        highestMemoryUsed = totalMemoryUsed;
//        Trace5("MEMORY     ", "ResolveState::RecordMemoryUsage Highpoint: highestMemoryUsed:{0}, termMemoryUsed:{1}, ruleMemoryUsed:{2}, stackMemoryUsed:{3}, highestMemoryUsedStack:{4}", 0, true,
//               totalMemoryUsed, termMemoryUsed, ruleMemoryUsed, stackMemoryUsed, GetStackString());
    }
    
    // Reset the high water marks so we don't doublecount
    initialTermMemory = currentTermMemory;
    initialRuleSetMemory = currentRuleSetMemory;
    
    return totalMemoryUsed;
}


void ResolveState::RecoverInitialVariables(HtnTermFactory *termFactory, UnifierType &unifier)
{
    // Fix variables in solution to match their initial names
    for(UnifierItemType &assignment : unifier)
    {
        assignment.first = assignment.first->RemovePrefixFromVariables(termFactory, initialVariablePrefix);
        assignment.second = assignment.second->RemovePrefixFromVariables(termFactory, initialVariablePrefix);
    }
}

shared_ptr<UnifierType> ResolveState::SimplifySolution(const UnifierType &solution, vector<shared_ptr<HtnTerm>> &goals)
{
    // Hard-won knowledge: In case of success, the final substitution, is the composition
    // of all the MGUs involved in the resolution *restricted to the variables of the initial goals*.
	// AND ignoring any "dontcare" variables (i.e. _)
    // Remove any variables that are not in the initial goals
    
    // First: get all the variables that were in the initial goals
    set<string> variables;
    for(shared_ptr<HtnTerm> term : goals)
    {
        vector<string> termVariables;
        term->GetAllVariables(&termVariables);
        
        // Ignore any that start with "_"
        for(auto item : termVariables)
        {
            if(item[0] != '_')
            {
                variables.insert(item);
            }
        }
    }
    
    // Substitute all the variables from back to front
    // Then: rebuild the solution removing any assignments that don't involve those variables
    shared_ptr<UnifierType> simplifiedSolution = shared_ptr<UnifierType>(new UnifierType());
    for(UnifierItemType item : solution)
    {
        if(item.first->name()[0] != '_' && variables.find(item.first->name()) != variables.end())
        {
            simplifiedSolution->push_back(item);
        }
    }
    
    // Fix variables in solution to match their initial names
    RecoverInitialVariables(termFactory, *simplifiedSolution);

//    Trace3("DEBUG      ", " simplified: {0}, terms: {1}, original: {2}", 0, true, HtnGoalResolver::ToString(*simplifiedSolution.get()), HtnTerm::ToString(goals), HtnGoalResolver::ToString(solution));
    return simplifiedSolution;
}

HtnGoalResolver::HtnGoalResolver()
{
    AddCustomRule("assert", CustomRuleType({ CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleAssert, std::placeholders::_1)));
    AddCustomRule("atom_concat", CustomRuleType({ CustomRuleArgType::ResolvedTerm, CustomRuleArgType::ResolvedTerm, CustomRuleArgType::Variable }, std::bind(&HtnGoalResolver::RuleAtomConcat, std::placeholders::_1)));
    AddCustomRule("downcase_atom", CustomRuleType({ CustomRuleArgType::ResolvedTerm, CustomRuleArgType::Variable }, std::bind(&HtnGoalResolver::RuleAtomDowncase, std::placeholders::_1)));
    AddCustomRule("atom_chars", CustomRuleType({ CustomRuleArgType::ResolvedTerm, CustomRuleArgType::Variable }, std::bind(&HtnGoalResolver::RuleAtomChars, std::placeholders::_1)));
    AddCustomRule("count", CustomRuleType({ CustomRuleArgType::Variable, CustomRuleArgType::SetOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleCount, std::placeholders::_1)));
    AddCustomRule("distinct", CustomRuleType({ CustomRuleArgType::Variable, CustomRuleArgType::SetOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleDistinct, std::placeholders::_1)));
    AddCustomRule("failureContext", CustomRuleType({ CustomRuleArgType::SetOfTerms }, std::bind(&HtnGoalResolver::RuleFailureContext, std::placeholders::_1)));
    AddCustomRule("findall", CustomRuleType({ CustomRuleArgType::Term, CustomRuleArgType::ResolvedTerm, CustomRuleArgType::ResolvedTerm }, std::bind(&HtnGoalResolver::RuleFindAll, std::placeholders::_1)));
    AddCustomRule("first", CustomRuleType({ CustomRuleArgType::SetOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleFirst, std::placeholders::_1)));
    AddCustomRule("forall", CustomRuleType({ CustomRuleArgType::ResolvedTerm, CustomRuleArgType::ResolvedTerm }, std::bind(&HtnGoalResolver::RuleForAll, std::placeholders::_1)));
    AddCustomRule("is", CustomRuleType({ CustomRuleArgType::Variable, CustomRuleArgType::Arithmetic }, std::bind(&HtnGoalResolver::RuleIs, std::placeholders::_1)));
    AddCustomRule("atomic", CustomRuleType({ CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleIsAtom, std::placeholders::_1)));
    AddCustomRule("max", CustomRuleType({ CustomRuleArgType::Variable, CustomRuleArgType::Variable, CustomRuleArgType::SetOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleAggregate, std::placeholders::_1)));
    AddCustomRule("min", CustomRuleType({ CustomRuleArgType::Variable, CustomRuleArgType::Variable, CustomRuleArgType::SetOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleAggregate, std::placeholders::_1)));
    AddCustomRule("nl", CustomRuleType({ }, std::bind(&HtnGoalResolver::RuleNewline, std::placeholders::_1)));
    AddCustomRule("not", CustomRuleType({ CustomRuleArgType::SetOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleNot, std::placeholders::_1)));
    AddCustomRule("print", CustomRuleType({ CustomRuleArgType::SetOfTerms }, std::bind(&HtnGoalResolver::RulePrint, std::placeholders::_1)));
    AddCustomRule("sortBy", CustomRuleType({ CustomRuleArgType::Variable, CustomRuleArgType::TermOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleSortBy, std::placeholders::_1)));
    AddCustomRule("sum", CustomRuleType({ CustomRuleArgType::Variable, CustomRuleArgType::Variable, CustomRuleArgType::SetOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleAggregate, std::placeholders::_1)));
	AddCustomRule("retract", CustomRuleType({ CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleRetract, std::placeholders::_1)));
    AddCustomRule("retractall", CustomRuleType({ CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleRetractAll, std::placeholders::_1)));
    AddCustomRule("showTraces", CustomRuleType({ CustomRuleArgType::SetOfResolvedTerms }, std::bind(&HtnGoalResolver::RuleTrace, std::placeholders::_1)));
    AddCustomRule("write", CustomRuleType({ CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleWrite, std::placeholders::_1)));
    AddCustomRule("writeln", CustomRuleType({ CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleWrite, std::placeholders::_1)));
    AddCustomRule("==", CustomRuleType({ CustomRuleArgType::Term, CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleTermCompare, std::placeholders::_1)));
    AddCustomRule("\\==", CustomRuleType({ CustomRuleArgType::Term, CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleTermCompare, std::placeholders::_1)));
    AddCustomRule("=", CustomRuleType({ CustomRuleArgType::Term, CustomRuleArgType::Term }, std::bind(&HtnGoalResolver::RuleUnify, std::placeholders::_1)));
}

// The trick here is that certain special rules like not, First and SortBy do more than just lookup a rule in a ruleset
// We let them do whatever they want as long as it results in a set of ruleBindings
void HtnGoalResolver::AddCustomRule(const string &name, HtnGoalResolver::CustomRuleType ruleFunction)
{
    FailFastAssert(m_customRules.find(name) == m_customRules.end());
    m_customRules[name] = ruleFunction;
}

// Finds all rules where the head can be unified with goal, returns the rule and the substitutions required to do it
shared_ptr<vector<RuleBindingType>> HtnGoalResolver::FindAllRulesThatUnify(HtnTermFactory *termFactory, HtnRuleSet *prog, shared_ptr<HtnTerm> goal, int *uniquifier, int indentLevel, int memoryBudget, bool fullTrace, int64_t *highestMemoryUsedReturn)
{
    int64_t memoryValue;
    if(highestMemoryUsedReturn == nullptr) { highestMemoryUsedReturn = &memoryValue; }
    *highestMemoryUsedReturn = 0;
    
    shared_ptr<vector<RuleBindingType>> foundRules(new vector<RuleBindingType>());
    if(goal->isTrue())
    {
        // Rule resolves to true
        shared_ptr<HtnRule> rule = shared_ptr<HtnRule>(new HtnRule(goal, {}));
        
        // No unifiers got added since it was ground so no changes there
        UnifierType unifiers;
        foundRules->push_back(RuleBindingType(rule, unifiers));
        return foundRules;
    }
    else if(goal->isArithmetic())
    {
        // We don't unify Arithmetic goals with facts in the database.
        // Instead we treat them as if they were looked up against an infinity of rules that are defined by the laws of math. I.e. 1 < 2 is looked up against all the facts of math and we find one that says 1 < 2 so it is true.
        // Therefore, the result will be either:
        //      true, in which case we treat it like a successful unification with a true fact in the database and remove this goal depth-first search through the next goal in the list
        //      false, in which case this branch fails
        //      not executable because all variables were not bound, in which case this branch fails
        shared_ptr<HtnTerm> term = goal->Eval(termFactory);
        if(term != nullptr)
        {
            // if it is a boolean term and true, this leaf succeeds and we pretend that there was that rule in the database and return it
            if(term->isTrue())
            {
                // Rule resolves to true
                shared_ptr<HtnRule> rule = shared_ptr<HtnRule>(new HtnRule(goal, {}));

                // No unifiers got added since it was ground so no changes there
                UnifierType unifiers;
                foundRules->push_back(RuleBindingType(rule, unifiers));
                return foundRules;
            }
            else
            {
                // if it is false, so we pretend it doesn't unify with anything
                return foundRules;
            }
        }
        else
        {
            // expression can't be resolved, therefore nothing down this branch can work and it fails
            Trace1("FAIL       ", "Term:{0} arithmetically can't be evaluated", indentLevel, fullTrace, goal->ToString());
            return foundRules;
        }
    }
    else
    {
        // OK, Now we actually try to look up real rules.
        // Because this could use a lot of memory, we actually watch for memory usage here and
        // fail out if we hit the budget
        int64_t initialTermMemory = termFactory->dynamicSize();
        int64_t initialRuleSetMemory = prog->dynamicSize();
        int64_t memoryUsed = 0;
        
        bool foundRule = false;
        int goalArgumentsSize = (int) goal->arguments().size();
        prog->AllRulesThatCouldUnify(goal.get(), [&](const HtnRule &item)
        {
            // If we ran out of memory budget, return whatever we found
            int64_t totalMemoryUsed = (termFactory->dynamicSize() - initialTermMemory) + (prog->dynamicSize() - initialRuleSetMemory) + memoryUsed;
            *highestMemoryUsedReturn = std::max(*highestMemoryUsedReturn, totalMemoryUsed);
            if(totalMemoryUsed > memoryBudget)
            {
                Trace4("MEMORY     ", "***** OUT OF MEMORY ***** used:{0}, budget:{1}, totalTermMemory:{2}, totalRulesetMemory:{3}", indentLevel, true, totalMemoryUsed, memoryBudget, termFactory->dynamicSize(), prog->dynamicSize());
                termFactory->outOfMemory(true);
                return false;
            }
        
            // Unify
            foundRule = true;
            shared_ptr<UnifierType> substitutions = HtnGoalResolver::Unify(termFactory, item.head(), goal);
            if(substitutions != nullptr)
            {
                // IF the unification works, make the variables in the rule unique,
                // since this is expensive in the inner loop
                string uniquifierString = goal->name() + lexical_cast<string>(*uniquifier) + "_";
                std::map<std::string, std::shared_ptr<HtnTerm>> variableMap;
                shared_ptr<HtnRule> currentRule = item.MakeVariablesUnique(termFactory, uniquifierString, variableMap);
                *uniquifier = (*uniquifier) + 1;

                // Also need to fix up the substitutions to use the new values since we renamed them
                for(UnifierItemType &item : *substitutions)
                {
                    item.first = item.first->RenameVariables(termFactory, variableMap);
                    item.second = item.second->RenameVariables(termFactory, variableMap);
                }

                foundRules->push_back(RuleBindingType(currentRule, *substitutions));
                memoryUsed += sizeof(RuleBindingType) + foundRules->back().second.size() * sizeof(UnifierItemType);
            }
                
            // Keep going
            return true;
        });
        
        if(!foundRule)
        {
            // Very common issue is to pass the wrong name or arguments so give a message here
            Trace3("FAIL       ", "no {1} rule with {2} arguments - goal:{0}", 0, true, goal->ToString(), goal->name(), goalArgumentsSize);
        }
        
        return foundRules;
    }
}

shared_ptr<HtnTerm> HtnGoalResolver::FindTermEquivalence(const UnifierType &unifier, const HtnTerm &termToFind)
{
    for(auto item : unifier)
    {
        if(*item.first == termToFind)
        {
            return item.second;
        }
    }

    for(auto item : unifier)
    {
        if(*item.second == termToFind)
        {
            return item.first;
        }
    }

    return nullptr;
}

bool HtnGoalResolver::GetCustomRule(const string &name, int arity, HtnGoalResolver::CustomRuleType &metadata)
{
    auto found = m_customRules.find(name);
    if(found != m_customRules.end())
    {
        metadata = found->second;
        return true;
    }
    else
    {
        return false;
    }
}

CustomRuleArgType HtnGoalResolver::GetCustomRuleArgBaseType(std::vector<CustomRuleArgType> metadata, int argIndex)
{
    CustomRuleArgType argType = CustomRuleArgType::Unknown;
    if(argIndex < metadata.size())
    {
        argType = metadata[argIndex];
    }
    else
    {
        // If it is past the end, use the last argument if it is variadic
        CustomRuleArgType lastArg = *metadata.rbegin();
        if(lastArg == CustomRuleArgType::SetOfResolvedTerms || lastArg == CustomRuleArgType::SetOfTerms)
        {
            argType = lastArg;
        }
        else
        {
            StaticFailFastAssertDesc(false, "Too many arguments for built-in rule");
        }
    }
    
    // Convert sets to base types
    if(argType == CustomRuleArgType::SetOfResolvedTerms)
    {
        argType = CustomRuleArgType::ResolvedTerm;
    }
    else if(*metadata.rbegin() == CustomRuleArgType::SetOfTerms)
    {
        argType = CustomRuleArgType::Term;
    }
    
    return argType;
}

bool HtnGoalResolver::IsGround(UnifierType *unifier)
{
    for(auto item : *unifier)
    {
        if(!item.second->isGround())
        {
            return false;
        }
    }
    
    return true;
}

// returns null if no solution
// returns a single empty UnifierType for "true" solution
// otherwise returns an array of UnifierTypes for all the solutions
shared_ptr<vector<UnifierType>> HtnGoalResolver::ResolveAll(HtnTermFactory *termFactory, HtnRuleSet *prog, const vector<shared_ptr<HtnTerm>> &initialGoals, int initialIndent, int memoryBudget, int64_t *highestMemoryUsedReturn, int *furthestFailureIndex, std::vector<std::shared_ptr<HtnTerm>> *farthestFailureContext)
{
    Trace3("ALL BEGIN  ", "goals:{0}, termStringsMemorySize:{1}, termOtherMemorySize:{2}", initialIndent, false, HtnTerm::ToString(initialGoals), termFactory->stringSize(), termFactory->otherAllocationSize());

    shared_ptr<ResolveState> state = shared_ptr<ResolveState>(new ResolveState(termFactory, prog, initialGoals, initialIndent, memoryBudget));
    shared_ptr<vector<UnifierType>> solutions = shared_ptr<vector<UnifierType>>(new vector<UnifierType>());
    while(true)
    {
        Trace1("CONTINUE   ", "goals:{0}", initialIndent, state->fullTrace, (state->resolveStack->size() > 0 ? HtnTerm::ToString(*state->resolveStack->back()->resolvent()) : ""));
        shared_ptr<UnifierType> solution = ResolveNext(state.get());
        if(solution != nullptr)
        {
            Trace5("END        ", "Query: {0} -> {1}, termStringsMemorySize:{2}, termOtherMemorySize:{3}, highestMemoryUsedStack:{4}", initialIndent, state->fullTrace, HtnTerm::ToString(initialGoals), ToString(*solution), termFactory->stringSize(), termFactory->otherAllocationSize(), state->highestMemoryUsedStack);
            solutions->push_back(*solution);
        }
        else
        {
            Trace3("END        ", "No more solutions. Query: {0}, termStringsMemorySize:{1}, termOtherMemorySize:{2}", initialIndent, state->fullTrace, HtnTerm::ToString(initialGoals), termFactory->stringSize(), termFactory->otherAllocationSize());
            break;
        }
        
        if(termFactory->outOfMemory())
        {
            Trace4("END        ", "Out of memory. Query: {0}, termStringsMemorySize:{1}, termOtherMemorySize:{2}, highestMemoryUsedStack:{3}", initialIndent, state->fullTrace, HtnTerm::ToString(initialGoals), termFactory->stringSize(), termFactory->otherAllocationSize(), state->highestMemoryUsedStack);
            break;
        }
        state->ClearFailures();
    }
    
    if(highestMemoryUsedReturn != nullptr)
    {
        *highestMemoryUsedReturn = state->highestMemoryUsed;
    }

    if(solutions->size() == 0)
    {
        // Always report a failure if there were no solutions
        if(furthestFailureIndex != nullptr)
        {
            *furthestFailureIndex = state->farthestFailureOriginalGoalIndex;
        }
        
        if(farthestFailureContext != nullptr)
        {
            *farthestFailureContext = state->farthestFailureContext;
        }
        
        Trace8("FAIL       ", "Failed to resolve initialGoal:{0}, \r\nfarthest initial goal:{6}, context:{7}, \r\ndeepest failure:{1}, stack:{5}, InitialGoals:{2}, termStrings:{3}, termOther:{4}", initialIndent, true,
               state->deepestFailureOriginalGoalIndex >= 0 ? initialGoals[state->deepestFailureOriginalGoalIndex]->ToString() : "",
               state->deepestFailureGoal != nullptr ? state->deepestFailureGoal->ToString() : "",
               HtnTerm::ToString(initialGoals),
               termFactory->stringSize(),
               termFactory->otherAllocationSize(),
               state->deepestFailureStack,
               state->farthestFailureOriginalGoalIndex >= 0 ? initialGoals[state->farthestFailureOriginalGoalIndex]->ToString() : "",
               HtnTerm::ToString(state->farthestFailureContext));
    }
        
    // Get a read on memory after we release state which is what it will look like when we return
    state = nullptr;
    
    Trace5("ALL END    ", "Query: {0} -> {1}, termStrings:{2}, termOther:{3}", initialIndent, false, HtnTerm::ToString(initialGoals), false, ToString(solutions.get()), termFactory->stringSize(), termFactory->otherAllocationSize());
    if(solutions->size() == 0)
    {
        return nullptr;
    }
    else
    {
        return solutions;
    }
}

// Does Prolog-style SLD [“linear resolution” with a “selection function” for “definite clauses” (Kowalski and Kuehner 1971).] resolution
// Basic model: We are exploring a tree. Every node of the tree represents a set of goals (resolvents). Every node of the tree takes one goal, and either:
//      - resolves it by replacing it with the tail end of a rule (which could simply be TRUE if it is a fact or arithmetic term) and creates a new node
//          - If the tail is empty (i.e. true), this is a solution
//      - or fails because there is no such thing to do, in which case no solutions are generated from that branch
//
// Implementation:
// This uses depth first search and no recursion (or at least not any unless built-in rules are used) to reduce memory (i.e. stack) usage
// Each "stack frame" represents a different binding of a rule to a goal, which means a set of resolvents which are equivalent to the stack above, but resolved
// by replacing the previous first resolvent with a rule in the database. Each stack frame is a new branch of the solution tree.
// If anything fails on this branch we backtrack all the way to the place where we branched and try the alternatives
// If we make it all the way to the point where there are no more resolvents to solve, then we push that solution into the list of solutions and keep going to return
// Returns nullptr if they can't be unified, otherwise returns a vector of all the solutions (which could be empty if they unified but no assignments were needed)
shared_ptr<UnifierType> HtnGoalResolver::ResolveNext(ResolveState *state)
{
    HtnTermFactory *termFactory = state->termFactory;
    HtnRuleSet *prog = state->prog;
    int initialIndent = state->initialIndent;
    int memoryBudget = state->memoryBudget;
    int &uniquifier = state->uniquifier;
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;

    // Because terms and rules can be created in between calls to ResolveNext,
    // we need to determine how much of these we use each time
    int64_t initialTermMemory = termFactory->dynamicSize();
    int64_t initialRuleSetMemory = prog->dynamicSize();
    
    while(resolveStack->size() > 0)
    {
        // Always make progress on the deepest branch first, which is at the top of the stack
        int indentLevel = (int) (initialIndent + resolveStack->size());
        shared_ptr<ResolveNode> currentNode = resolveStack->back();

        // Consistent place to check for memory used so it is checked regularly but not constantly
        int64_t totalMemoryUsed = state->RecordMemoryUsage(initialTermMemory, initialRuleSetMemory);
        if(totalMemoryUsed > state->memoryBudget)
        {
            // Out of memory: don't return the current solution since it is not done, and don't allow continuing
            // Since we're in an unknown state
            Trace6("MEMORY     ", "***** OUT OF MEMORY ***** used:{0}, budget:{1}, totalTermMemory:{2}, totalRulesetMemory:{3}, stackMemory:{4}, highestMemoryStack:{5}", indentLevel, true, totalMemoryUsed, state->memoryBudget, termFactory->dynamicSize(), prog->dynamicSize(), state->stackMemoryUsed, state->highestMemoryUsedStack);
            state->termFactory->outOfMemory(true);
            currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            return nullptr;
        }

        switch(currentNode->continuePoint)
        {
            case ResolveContinuePoint::ProgramError:
            {
                Trace0("FATAL      ", "Can't continue: Previously hit a fatal error", indentLevel, true);
                return nullptr;
            }
            break;

			case ResolveContinuePoint::Cut:
			{
				// We have returned from processing after a cut and reached the cut point again
				// Now we need to pop the stack until we get to the point where the rule that contains 
				// this cut was introduced and remove them all so that we don't backtrack on them
				// The part of the stack that will get skipped is bounded by goals "!>(ID)" at the beginning
				// and "!<(ID)" at the end (this node)
				// Thus, we pop the stack until we find the start node that matches the ID of this one
				string cutID = currentNode->currentGoal()->arguments()[0]->name();
				bool found = false;
				while (resolveStack->size() > 0)
				{
					shared_ptr<HtnTerm> goal = resolveStack->back()->currentGoal();
					resolveStack->pop_back();
					if(goal != nullptr && goal->name() == "!>" && goal->arguments()[0]->name() == cutID)
					{
						// Found it!
						found = true;
						break;
					}
				}

				// Now, we need to stop the last node before the cut from processing any alternatives
				// If the stack is empty we encountered a degenerate case where the entire thing we were
				// asked to resolve started with "!", which is meaningless so we can ignore
				FailFastAssert(found);
				if(resolveStack->size() > 0)
				{
					resolveStack->back()->SetCut();
				}

				// And then let it continue on processing this node
			}
			break;

            case ResolveContinuePoint::NextGoal:
            {
                // Get the next goal on the list of resolvents
                shared_ptr<HtnTerm> goal = currentNode->currentGoal();
                if(goal == nullptr)
                {
                    // No more goals, we have a solution!
                    Trace1("SUCCESS    ", "solution:{0}", indentLevel, state->fullTrace, ToString(*currentNode->unifier));
                    currentNode->AddToSolutions(solutions);
                    resolveStack->pop_back();
                    if(!state->collectAllSolutions)
                    {
                        return state->SimplifySolution(*currentNode->unifier, *state->initialGoals);
                    }
                }
				// If it is the start of a cut we just ignore it and keep going since it is just a marker on the stack
				else if (goal->name() == "!>")
				{
					// There must always be at least one goal after a start, even if it is only "!"
					FailFastAssert(!currentNode->IsLastGoalInResolvent());

					// Cut resolves to true so no new terms, no unifiers got added since it it is not unified
					// Nothing to process on children so no special return handling
					resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &uniquifier));
                    currentNode->continuePoint = ResolveContinuePoint::Return;

					Trace2("CUTSTART   ", "goal:{0}, resolvent:{1}", indentLevel, state->fullTrace, goal->ToString(), HtnTerm::ToString(*currentNode->resolvent()));
				}
				// We are executing a cut end, nothing happens until we get back to this point
				else if (goal->name() == "!<")
				{
					// When we reach a goal that is a cut, we should prevent all backtracking before this point
					// *for this clause*.  So, succeed for this goal, continue processing goals, 
					// but when we get back to this point on the tree again, jump back to the stack frame which
					// represents the start of the "fence" for this cut, which is right before the goal
					// just return whatever solutions we have found
					currentNode->continuePoint = ResolveContinuePoint::Cut;

					// Cut resolves to true so no new terms, no unifiers got added since it it is not unified
					// Nothing to process on children so no special return handling
					resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &uniquifier));

					Trace2("CUTEND     ", "goal:{0}, resolvent:{1}", indentLevel, state->fullTrace, goal->ToString(), HtnTerm::ToString(*currentNode->resolvent()));
				}
                else
                {
                    // Find all the rules that unify with the first goal on the list
                    Trace2("RESOLVE    ", "goal:{0}, resolvent:{1}", indentLevel, state->fullTrace, goal->ToString(), HtnTerm::ToString(*currentNode->resolvent()));
                    if(goal->isVariable())
                    {
                        // Variables cannot be goals since Prolog is based on first-order logic but X is a second order logic query (the variable stands for a rule head / fact, not only a term):
                        // you ask "which predicates can be derived?" or in other words "which formulas are true?".
                        // So we shortcut out which will exit everything
                        Trace1("ERROR      ", "goal is a variable:{0}", indentLevel, state->fullTrace, goal->ToString());
                        StaticFailFastAssertDesc(false, ("goal can't be a variable: " + goal->ToString()).c_str());
                        currentNode->continuePoint = ResolveContinuePoint::ProgramError;
                        return nullptr;
                    }
                    else
                    {
                        CustomRulesType::iterator foundCustomRule = m_customRules.find(goal->name());
                        if(foundCustomRule != m_customRules.end())
                        {
                            // This is a custom rule that will potentially add to currentNode->rulesThatUnify and be handled just like the default case
                            currentNode->continuePoint = ResolveContinuePoint::CustomStart;
                            foundCustomRule->second.second(state);
                        }
                        else
                        {
                            // Not custom, just handle normally
                            int64_t FindAllRulesThatUnifyHighestMemory = 0;
                            currentNode->rulesThatUnify = FindAllRulesThatUnify(termFactory, prog, goal, &uniquifier, indentLevel, (int)(memoryBudget - totalMemoryUsed), state->fullTrace, &FindAllRulesThatUnifyHighestMemory);
                            state->highestMemoryUsed = std::max(state->highestMemoryUsed, totalMemoryUsed + FindAllRulesThatUnifyHighestMemory);
                            if(termFactory->outOfMemory())
                            {
                                // Finding rules can require lots of memory so we check for OOM here
                                Trace5("MEMORY     ", "***** OUT OF MEMORY ***** used:{0}, budget:{1}, totalTermMemory:{2}, totalRulesetMemory:{3}, highestMemoryStack:{4}", indentLevel, true, totalMemoryUsed, state->memoryBudget, termFactory->dynamicSize(), prog->dynamicSize(), state->highestMemoryUsedStack);
                                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
                            }
                            else
                            {
                                currentNode->continuePoint = ResolveContinuePoint::NextRuleThatUnifies;
                                if(currentNode->rulesThatUnify->size() == 0)
                                {
                                    state->RecordFailure(goal, currentNode);
                                }
                                Trace1("           ", "found:{0} rules that unify", indentLevel, state->fullTrace, currentNode->rulesThatUnify->size());
                            }
                        }
                    }
                }
                continue;
            }
            break;

            case ResolveContinuePoint::Return:
            {
                resolveStack->pop_back();
            }
            break;
                
            case ResolveContinuePoint::NextRuleThatUnifies:
            {
                // Go through each rule that unified and explore the part of the tree with that alternative
                if(currentNode->SetNextRule())
                {
                    RuleBindingType ruleBinding = currentNode->currentRule();
                    Trace1("           ", "rule:{0}", indentLevel, state->fullTrace, ruleBinding.first->ToString());
                    Trace1("           ", "unifier:{0}", indentLevel, state->fullTrace, ToString(ruleBinding.second));
                    resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, ruleBinding.first->tail(), ruleBinding.second, &uniquifier));
                }
                else
                {
                    // No more rules, thus there are no more solutions to find in this part of the tree
                    resolveStack->pop_back();
                }
                continue;
            }
            break;
                
            case ResolveContinuePoint::CustomStart:
            case ResolveContinuePoint::CustomContinue1:
            case ResolveContinuePoint::CustomContinue2:
            case ResolveContinuePoint::CustomContinue3:
            case ResolveContinuePoint::CustomContinue4:
            {
                CustomRulesType::iterator foundCustomRule = m_customRules.find(currentNode->currentGoal()->name());
                FailFastAssert(foundCustomRule != m_customRules.end());
                foundCustomRule->second.second(state);
            }
            break;
        }
    }
    
    // No more solutions
    return nullptr;
}

// agg(?AggregateVariable, ?Variable, ?SetOfResolvedTerms...)
// agg will evaluate ALL of the potential resolutions and return the aggreg value of ?Variable, if there are none, it fails
// The terms must bind ?Variable and bind it to a ground number or it fails
// No bindings will be passed along except ?AggregateVariable
void HtnGoalResolver::RuleAggregate(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    string aggName = goal->name();
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // the count operator needs a variable as the first argument and then a set of terms to resolve
            if(goal->arguments().size() < 3 || !goal->arguments()[0]->isVariable() || !goal->arguments()[1]->isVariable())
            {
                // Invalid program
                Trace2("ERROR      ", "{0}(?AggregateVariable, ?Variable, terms...) must have at least 3 terms where the first two are variables: {1}", state->initialIndent + resolveStack->size(), state->fullTrace, aggName, goal->ToString());
                StaticFailFastAssertDesc(false, ("item(?AggregateVariable, ?Variable, terms...) must have at least 3 terms where the first two are variables : " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // Make sure we keep around the value of ?Variable and any variables in the rest of the resolvent (they won't be there when we do the resolve, so they will get stripped out)
                shared_ptr<ResolveNode::TermSetType> variablesToKeep = shared_ptr<ResolveNode::TermSetType>(new ResolveNode::TermSetType());
                for(shared_ptr<HtnTerm> term : *currentNode->resolvent())
                {
                    term->GetAllVariables(variablesToKeep.get());
                }
                
                // Run the resolver just on the arguments as if it were a standalone resolution.  Then continue on depending on what happens
                currentNode->PushStandaloneResolve(state, variablesToKeep, goal->arguments().rbegin(), --(--goal->arguments().rend()), ResolveContinuePoint::CustomContinue1);
            }
        }
        break;
            
        case ResolveContinuePoint::CustomContinue1:
        {
            shared_ptr<HtnTerm> variable = goal->arguments()[0];
            shared_ptr<HtnTerm> variableToAgg = goal->arguments()[1];

            // Solutions now contains just solutions to what was in the third and beyond terms
            if(solutions == nullptr)
            {
                // There were no solutions: fail!
                // Put back on whatever solutions we had before the first() so we can continue adding to them
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            else
            {
                // Add up the values
                shared_ptr<HtnTerm> aggregate;
                for(auto solution : *solutions)
                {
                    shared_ptr<HtnTerm> equivalence = FindTermEquivalence(solution, *variableToAgg);
                    if(equivalence != nullptr)
                    {
                        shared_ptr<HtnTerm> evalTerm = equivalence->Eval(termFactory);
                        if(evalTerm != nullptr)
                        {
                            // Aggregate things up using HtnArithmeticOperators::* so the right conversions happen if there are different numbers
                            if(aggregate == nullptr)
                            {
                                aggregate = evalTerm;
                            }
                            else
                            {
                                if(aggName == "sum")
                                {
                                    aggregate = HtnArithmeticOperators::Plus(termFactory, aggregate, evalTerm);
                                }
                                else if(aggName == "min")
                                {
                                    aggregate = HtnArithmeticOperators::Min(termFactory, aggregate, evalTerm);
                                }
                                else if(aggName == "max")
                                {
                                    aggregate = HtnArithmeticOperators::Max(termFactory, aggregate, evalTerm);
                                }
                                else
                                {
                                    // Shouldn't have got here if there is nothing to process it
                                    StaticFailFastAssert(false);
                                }
                            }
                        }
                        else
                        {
                            // Not a number: fail!
                            // Put back on whatever solutions we had before the first() so we can continue adding to them
                            Trace4("           ", "{0}() Variable: {1} was {2} which could not be resolved to a number in {3}", state->initialIndent + resolveStack->size(), state->fullTrace, aggName, variableToAgg->ToString(), equivalence->ToString(), goal->ToString());
                            state->RecordFailure(goal, currentNode);
                            resolveStack->pop_back();
                            break;
                        }
                    }
                    else
                    {
                        // No variable: fail!
                        // Put back on whatever solutions we had before the first() so we can continue adding to them
                        Trace3("           ", "{0}() Variable: {1} not found in {2}", state->initialIndent + resolveStack->size(), state->fullTrace, aggName, variableToAgg->ToString(), ToString(solution));
                        state->RecordFailure(goal, currentNode);
                        resolveStack->pop_back();
                        break;
                    }
                }
                
                if(aggregate != nullptr)
                {
                    // Put the previous solutions back and continue on as if these were all unified rules
                    Trace2("           ", "{0}() succeeded with aggregate:{1}", state->initialIndent + resolveStack->size(), state->fullTrace, aggName, aggregate->ToString());
        
                    // We treat this as a rule where the variable got unified with the result. So, there are no new goals to add, but there are new unifiers
                    // Nothing to do on return
                    UnifierType exprUnifier( { UnifierItemType(variable, aggregate ) } );
                    resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, exprUnifier, &(state->uniquifier)));
                    currentNode->continuePoint = ResolveContinuePoint::Return;
                }
            }
            
            currentNode->PopStandaloneResolve(state);
        }
            break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// assert(?Term)
void HtnGoalResolver::RuleAssert(ResolveState* state)
{
	shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
	shared_ptr<HtnTerm> goal = currentNode->currentGoal();
	shared_ptr<vector<shared_ptr<ResolveNode>>>& resolveStack = state->resolveStack;
	HtnTermFactory* termFactory = state->termFactory;
	HtnRuleSet* prog = state->prog;

	switch (currentNode->continuePoint)
	{
		case ResolveContinuePoint::CustomStart:
		{
			// the assert rule needs a single term
			if (goal->arguments().size() != 1)
			{
				// Invalid program
				Trace1("ERROR      ", "assert() must have exactly one term: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
				StaticFailFastAssertDesc(false, ("assert() must have exactly one term: " + goal->ToString()).c_str());
				currentNode->continuePoint = ResolveContinuePoint::ProgramError;
			}
            else
            {
                shared_ptr<HtnTerm> term = goal->arguments()[0];
                vector<shared_ptr<HtnTerm>> assertList;
                if (term->isGround())
                {
                    assertList.push_back(term);
                }
                else
                {
                    // TODO: What to do?
                    StaticFailFastAssert(false);

                    //prog->AllRules([&](const HtnRule & item)
                    //	{
                    //		if (item.IsFact() && item.head()->isEquivalentCompoundTerm(term))
                    //		{
                    //			shared_ptr<UnifierType> sub = Unify(termFactory, item.head(), term);
                    //			if (sub != nullptr)
                    //			{
                    //				shared_ptr<HtnTerm> newTerm = SubstituteUnifiers(termFactory, *sub, term);
                    //				// Should be ground since we are unifying with a fact
                    //				StaticFailFastAssert(newTerm->isGround());
                    //				assertList.push_back(newTerm);
                    //			}
                    //		}

                    //		return true;
                    //	});
                }

                // Add all the facst into the database.
                prog->Update(termFactory, {}, assertList);

                // Rule resolves to true so no new terms, no unifiers got added since it it is not unified
                // Nothing to process on children so no special return handling
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
		}
		break;

		default:
			StaticFailFastAssert(false);
			break;
	}
}

// atom_chars(atom, List)
void HtnGoalResolver::RuleAtomChars(ResolveState* state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>>& resolveStack = state->resolveStack;
    HtnTermFactory* termFactory = state->termFactory;

    switch (currentNode->continuePoint)
    {
    case ResolveContinuePoint::CustomStart:
    {
        if( !((goal->arguments().size() == 2) && !(goal->arguments()[0]->isVariable() && goal->arguments()[1]->isVariable())))
        {
            // Invalid program
            Trace1("ERROR      ", "atom_chars() must have two terms and they both can't be variables: {0}",
                   state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
            StaticFailFastAssertDesc(false, ("atom_chars() must have two terms and they both can't be variables: " +
                                             goal->ToString()).c_str());
            currentNode->continuePoint = ResolveContinuePoint::ProgramError;
        }
        else
        {
            shared_ptr<HtnTerm> term1 = goal->arguments()[0];
            shared_ptr<HtnTerm> term2 = goal->arguments()[1];
            
            if(term1->isVariable())
            {
                // The left side is a variable, right side must be a list (due to checks above), convert it to a term
                shared_ptr<HtnTerm> front = term2;
                string atomName;
                while(front->name() == "." && front->arity() == 2 && !front->arguments()[0]->isVariable())
                {
                    string currentChar = front->arguments()[0]->name();
                    if(currentChar.size() == 1)
                    {
                        atomName.push_back(currentChar[0]);
                        front = front->arguments()[1];
                    }
                    else
                    {
                        break;
                    }
                }
                
                if(atomName.size() == 0)
                {
                    // unification failed: fail!
                    Trace1("FAIL       ", "atom_chars/2 rule failed because right side is empty string: {1}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                    state->RecordFailure(goal, currentNode);
                    resolveStack->pop_back();
                }
                else
                {
                    // Unify new term with left
                    shared_ptr<HtnTerm> newTerm = termFactory->CreateConstant(atomName);
                    shared_ptr<UnifierType> unifyResult = Unify(termFactory, goal->arguments()[0], newTerm);
                    
                    // Must work because it is a variable
                    StaticFailFastAssert(unifyResult != nullptr);
                    
                    // success! Treat this node as though it unified with a rule that resolved to true.
                    // Just like if we unified with a normal rule, we continue the depth first search skipping the current goal
                    // The unifiers we found get added to the list of unifiers
                    // No new goals were added since it just resolved to "true"
                    Trace1("           ", "atom_chars/2 rule succeeded, new unification: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, ToString(*unifyResult));
                    resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, { *unifyResult }, &(state->uniquifier)));
                    currentNode->continuePoint = ResolveContinuePoint::Return;
                }
            }
            else
            {
                // The right side is a variable or a list
                // convert left into a list
                string name = term1->name();
                vector<shared_ptr<HtnTerm>> elements;
                for(char c : name)
                {
                    elements.push_back(termFactory->CreateConstant(std::string(1, c)));
                }
                shared_ptr<HtnTerm> finalList = termFactory->CreateList(elements);
                
                // Unify with right
                shared_ptr<UnifierType> unifyResult = Unify(termFactory, goal->arguments()[1], finalList);
                if(unifyResult == nullptr)
                {
                    // unification failed: fail!
                    Trace2("FAIL       ", "atom_chars/2 rule failed to unify {0} with: {1}", state->initialIndent + resolveStack->size(), state->fullTrace, finalList->ToString(), goal->arguments()[1]->ToString());
                    state->RecordFailure(goal, currentNode);
                    resolveStack->pop_back();
                }
                else
                {
                    // success! Treat this node as though it unified with a rule that resolved to true.
                    // Just like if we unified with a normal rule, we continue the depth first search skipping the current goal
                    // The unifiers we found get added to the list of unifiers
                    // No new goals were added since it just resolved to "true"
                    Trace1("           ", "atom_chars/2 rule succeeded, new unification: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, ToString(*unifyResult));
                    resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, { *unifyResult }, &(state->uniquifier)));
                    currentNode->continuePoint = ResolveContinuePoint::Return;
                }
            }
        }
    }
    break;

    default:
        StaticFailFastAssert(false);
        break;
    }
}

// downcase_atom(a, ?A)
void HtnGoalResolver::RuleAtomDowncase(ResolveState* state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>>& resolveStack = state->resolveStack;
    HtnTermFactory* termFactory = state->termFactory;

    switch (currentNode->continuePoint)
    {
    case ResolveContinuePoint::CustomStart:
    {
        if (goal->arguments().size() != 2 || !goal->arguments()[0]->isConstant() || !goal->arguments()[1]->isVariable())
        {
            // Invalid program
            Trace1("ERROR      ", "downcase_atom() must have two terms, first is constant and second is variable:{0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
            StaticFailFastAssertDesc(false, ("downcase_atom() must have two terms, first is constant and second is variable: " + goal->ToString()).c_str());
            currentNode->continuePoint = ResolveContinuePoint::ProgramError;
        }
        else
        {
            shared_ptr<HtnTerm> term1 = goal->arguments()[0];
            shared_ptr<HtnTerm> termResult = goal->arguments()[1];

            std::locale loc;
            std::string lowercase = term1->name();
            std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
                [&](char c) { return std::tolower(c, loc); });
            shared_ptr<HtnTerm> resultAtom = termFactory->CreateConstant(lowercase);

            // Just unify the two arguments
            shared_ptr<UnifierType> result = Unify(termFactory, termResult, resultAtom);
            if (result == nullptr)
            {
                // There were no solutions: fail!
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            else
            {
                // success! Treat this node as though it unified with a rule that resolved to true.
                // Just like if we unified with a normal rule, we continue the depth first search skipping the current goal
                // The unifiers we found get added to the list of unifiers
                // No new goals were added since it just resolved to "true"
                Trace1("           ", "downcase_atom() rule succeeded, new unification: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, ToString(*result));
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, { *result }, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
        }
    }
    break;

    default:
        StaticFailFastAssert(false);
        break;
    }
}

// atom_concat(a, b, ?ab)
void HtnGoalResolver::RuleAtomConcat(ResolveState* state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>>& resolveStack = state->resolveStack;
    HtnTermFactory* termFactory = state->termFactory;

    switch (currentNode->continuePoint)
    {
    case ResolveContinuePoint::CustomStart:
    {
        // the "atom_concat" operator accepts only three terms
        if (goal->arguments().size() != 3 || !goal->arguments()[0]->isConstant() || !goal->arguments()[1]->isConstant() || !goal->arguments()[2]->isVariable())
        {
            // Invalid program
            Trace1("ERROR      ", "atom_concat() must have three terms, first two as constants and the last as variable:{0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
            StaticFailFastAssertDesc(false, ("atom_concat() must have three terms, first two as constants and the last as variable: " + goal->ToString()).c_str());
            currentNode->continuePoint = ResolveContinuePoint::ProgramError;
        }
        else
        {
            shared_ptr<HtnTerm> term1 = goal->arguments()[0];
            shared_ptr<HtnTerm> term2 = goal->arguments()[1];
            shared_ptr<HtnTerm> termResult = goal->arguments()[2];

            shared_ptr<HtnTerm> concatAtom = termFactory->CreateConstant(term1->name() + term2->name());

            // Just unify the two arguments
            shared_ptr<UnifierType> result = Unify(termFactory, goal->arguments()[2], concatAtom);
            if (result == nullptr)
            {
                // There were no solutions: fail!
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            else
            {
                // success! Treat this node as though it unified with a rule that resolved to true.
                // Just like if we unified with a normal rule, we continue the depth first search skipping the current goal
                // The unifiers we found get added to the list of unifiers
                // No new goals were added since it just resolved to "true"
                Trace1("           ", "atom_concat() rule succeeded, new unification: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, ToString(*result));
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, { *result }, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
        }
    }
    break;

    default:
        StaticFailFastAssert(false);
        break;
    }
}

// Count will evaluate ALL of the potential resolutions and then only return the Number of them, if it fails the count is zero and count will NOT fail
// No bindings will be passed along from Count but it will resolve to the total # of solutions we found
// count(?Variable, ?SetOfResolvedTerms...)
void HtnGoalResolver::RuleCount(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;

    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // the count operator needs a variable as the first argument and then a set of terms to resolve
            if(goal->arguments().size() < 2 || !goal->arguments()[0]->isVariable())
            {
                // Invalid program
                Trace1("ERROR      ", "count(?Var, terms...) must have at least two terms where the first is a variable: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("count(?Var, terms...) must have at least two terms where the first is a variable: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // Run the resolver just on the arguments as if it were a standalone resolution.  Then continue on depending on what happens
                currentNode->PushStandaloneResolve(state, nullptr, goal->arguments().rbegin(), --goal->arguments().rend(), ResolveContinuePoint::CustomContinue1);
            }
        }
        break;

        case ResolveContinuePoint::CustomContinue1:
        {
            // Solutions now contains just solutions to what was in the second and beyond count() terms
            int count = 0;
            if(solutions != nullptr)
            {
                // Count the solutions we found
                count = (int) solutions->size();
            }
            
            shared_ptr<HtnTerm> variable = goal->arguments()[0];

            // Put the previous solutions back and continue on as if these were all unified rules
            Trace1("           ", "count() succeeded with {0} solutions", state->initialIndent + resolveStack->size(), state->fullTrace, count);

            // We treat this as a rule where the variable got unified with the result. So, there are no new goals to add, but there are new unifiers
            // Nothing to do on return
            UnifierType exprUnifier( { UnifierItemType(variable, termFactory->CreateConstant(lexical_cast<string>(count)) ) } );
            resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, exprUnifier, &(state->uniquifier)));
            currentNode->continuePoint = ResolveContinuePoint::Return;
            
            currentNode->PopStandaloneResolve(state);
        }
        break;

        default:
            StaticFailFastAssert(false);
            break;
    }
}

// distinct(?Variable, ?SetOfResolvedTerms...)
// where ?Value is a variable symbol, and the rest is a set of terms.
void HtnGoalResolver::RuleDistinct(ResolveState *state)
{
    //    int indentLevel = state->initialIndent + state->resolveStack->size();
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // the count operator needs a variable as the first argument and then a set of terms to resolve
            if(goal->arguments().size() < 2 || !goal->arguments()[0]->isVariable())
            {
                // Invalid program
                Trace1("ERROR      ", "distinct(?Var, terms...) must have at least two terms where the first is a variable: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("distinct(?Var, terms...) must have at least two terms where the first is a variable: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // Make sure we keep around the value of ?Variable and any variables in the rest of the resolvent (they won't be there when we do the resolve, so they will get stripped out)
                shared_ptr<ResolveNode::TermSetType> variablesToKeep = shared_ptr<ResolveNode::TermSetType>(new ResolveNode::TermSetType());
                for(shared_ptr<HtnTerm> term : *currentNode->resolvent())
                {
                    term->GetAllVariables(variablesToKeep.get());
                }

                // Run the resolver just on the arguments as if it were a standalone resolution.  Then continue on depending on what happens
                currentNode->PushStandaloneResolve(state, variablesToKeep, goal->arguments().rbegin(), --goal->arguments().rend(), ResolveContinuePoint::CustomContinue1);
            }
        }
            break;
            
        case ResolveContinuePoint::CustomContinue1:
        {
            // Solutions now contains just solutions to what was in the distinct() term
            if(solutions == nullptr)
            {
                // There were no solutions: fail!
                // Put back on whatever solutions we had before the first() so we can continue adding to them
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            else
            {
                StaticFailFastAssert((*currentNode->resolvent())[0]->arguments().size() > 1);
                string variable = (*currentNode->resolvent())[0]->arguments()[0]->name();

                vector<int> uniqueSolutionIndices;
                if(variable[0] == '_')
                {
                    if(solutions->size() > 0)
                    {
                        // There was no variable, return all unique combinations of variables
                        map<vector<shared_ptr<HtnTerm>>, int, HtnTermVectorComparer> items;
                        
                        // Each vector representing a solution has the same variable in the same position
                        // Assign indices for all the variables, *except* for don't care variables since we
                        // don't care
                        UnifierType &binding = (*solutions)[0];
                        map<string, int> variablePositions;
                        int position = 0;
                        for(UnifierItemType item : binding)
                        {
                            string varName = item.first->name();
                            if(varName[0] != '_')
                            {
                                variablePositions[varName] = position;
                                position++;
                            }
                        }
                            
                        // Now add each solution if they are unique
                        for(int index = 0; index < solutions->size(); ++index)
                        {
                            vector<shared_ptr<HtnTerm>> solutionVector;
                            solutionVector.resize(variablePositions.size());
                            UnifierType &binding = (*solutions)[index];
                            for(UnifierItemType item : binding)
                            {
                                string varName = item.first->name();
                                if(varName[0] != '_')
                                {
                                    solutionVector[variablePositions[varName]] = item.second;
                                }
                            }
                            
                            if(items.find(solutionVector) == items.end())
                            {
                                items[solutionVector] = index;
                                uniqueSolutionIndices.push_back(index);
                            }
                        }
                    }
                }
                else
                {
                    // Put all of the Evaluated values in a map along with the index where they came from
                    // OK to put HtnTerm
                    map<shared_ptr<HtnTerm>, int, HtnTermComparer> items;
                    for(int index = 0; index < solutions->size(); ++index)
                    {
                        UnifierType &binding = (*solutions)[index];
                        bool found = false;
                        for(UnifierItemType item : binding)
                        {
                            if(item.first->name() == variable)
                            {
                                shared_ptr<HtnTerm> term = item.second;
                                // Variable can contain any valid term
                                StaticFailFastAssert(term != nullptr);
                                if(items.find(term) == items.end())
                                {
                                    items[term] = index;
                                    uniqueSolutionIndices.push_back(index);
                                }
                                
                                found = true;
                                break;
                            }
                        }
                        
                        // Need to have the variable available
                        StaticFailFastAssert(found);
                    }
                }
                
                // Create a fake "rule" so we can continue the search.
                shared_ptr<HtnRule> rule = shared_ptr<HtnRule>(new HtnRule(termFactory->CreateConstant("distinct"), {}));
                
                // Add all the solutions we found as "unified rules" so we can loop through them
                currentNode->rulesThatUnify = shared_ptr<vector<RuleBindingType>>(new vector<RuleBindingType>());
                for(int uniqueIndex : uniqueSolutionIndices)
                {
                    currentNode->rulesThatUnify->push_back(RuleBindingType(rule, (*solutions)[uniqueIndex]));
                }
                
                // Since the solutions we have already have the unifiers from this node already included,
                // get rid of the unifiers for this distinct() node so we don't try to remerge each fake unified rule with them again
                currentNode->unifier = shared_ptr<UnifierType>(new UnifierType());
                
                // Put the previous solutions back and continue on as if these were all unified rules
                Trace1("           ", "distinct() succeeded with {0} solutions", state->initialIndent + resolveStack->size(), state->fullTrace, currentNode->rulesThatUnify->size());
                currentNode->currentRuleIndex = -1;
                currentNode->continuePoint = ResolveContinuePoint::NextRuleThatUnifies;
            }
            
            currentNode->PopStandaloneResolve(state);
        }
            break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// failureContext is a way to report *why* something failed back to the user
// it operates under the premise that, of all the failures that may have occurred in resolving
// a set of terms, the one that got the farthest into the list of initial terms, and the deepest
// (in the stack) for that term is probably the most interesting failure to report
// It works very well as a heursitic.
//
// the failureContext associated with a failure will be returned from ResolveAll()
//
// the currently active failureContext is stored on the ResolveNode and carried forward as we
// do a depth first recurse.  This is so that it will get rolled back properly on backtrack
// it gets recorded as the "farthest failure" if a failure happens which
// is the farthest point the resolver has gotten in the list of initial goals, or is equivalent
// to the farthest in the list, but has a deeper stackdepth.
//
// ?Tag is simply something to help the developer figure out which error it was
// failureContext(?Tag, ?SetOfTerms...)
void HtnGoalResolver::RuleFailureContext(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            if(goal->arguments().size() == 0 || goal->arguments()[0]->isVariable())
            {
                // Invalid program
                Trace1("ERROR      ", "failureContext(?Order, ?SetOfTerms...) must have at least one term where the first is not a variable: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("failureContext(?Order, ?SetOfTerms...) must have at least one term where the first is not a variable: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // Remember the failure context in case this is the deepest failure
                Trace1("           ", "failureContext() succeeded:  {0}", state->initialIndent + resolveStack->size(), state->fullTrace, HtnTerm::ToString(goal->arguments()));

                // Rule resolves to true so no new terms, no unifiers got added since it was ground so no changes there
                // Nothing to process on children so no special return handling
                shared_ptr<ResolveNode> newNode = currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier));
                
                if(goal->arguments()[0]->name() == "clear")
                {
                    // Clears any failure context
                    state->farthestFailureContext.clear();
                    state->farthestFailureDepth = -1;
                    newNode->currentFailureContext.clear();
                }
                else
                {
                    // Set the failure context on the new node so it will carry forward
                    newNode->currentFailureContext = goal->arguments();
                }
                
                resolveStack->push_back(newNode);
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// findall(template, goal, bag)
// Find all solutions for goal
// create a list by looping through each solution unifying it with template
// unify the list with bag
void HtnGoalResolver::RuleFindAll(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            if(goal->arguments().size() < 3)
            {
                // Invalid program
                Trace1("ERROR      ", "findall(template, goal, list) must have three terms: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("findall(template, goal, list) must have three terms: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // Make sure we keep around the value of Variables in template and any variables in goal (they won't be there when we do the resolve, so they will get stripped out)
                shared_ptr<ResolveNode::TermSetType> variablesToKeep = shared_ptr<ResolveNode::TermSetType>(new ResolveNode::TermSetType());
                for(shared_ptr<HtnTerm> term : *currentNode->resolvent())
                {
                    term->GetAllVariables(variablesToKeep.get());
                }

                // Run the resolver just on the goal as if it were a standalone resolution.  Then continue on depending on what happens
                currentNode->PushStandaloneResolve(state, variablesToKeep, ++goal->arguments().rbegin(), --goal->arguments().rend(), ResolveContinuePoint::CustomContinue1);
            }
        }
        break;
            
        case ResolveContinuePoint::CustomContinue1:
        {
            // Solutions now contains just solutions to what was in the goal term
            shared_ptr<HtnTerm> finalList;
            if(solutions == nullptr)
            {
                // There were no solutions: succeed with an empty list!
                finalList = termFactory->CreateList({});
            }
            else
            {
                // There were solutions. Loop through each solution and replace template variables with its assignments. Add that to the solution
                shared_ptr<HtnTerm> templateTerm = goal->arguments()[0];
                std::vector<std::shared_ptr<HtnTerm>> terms;
                for(auto unification : *solutions)
                {
                    shared_ptr<HtnTerm> replacement = templateTerm;
                    for(UnifierItemType item : unification)
                    {
                        replacement = replacement->SubstituteTermForVariable(termFactory, item.second, item.first);
                    }
                    terms.push_back(replacement);
                }
                
                finalList = termFactory->CreateList(terms);
            }
            
            // Just unify the two arguments
            shared_ptr<UnifierType> unifyResult = Unify(termFactory, goal->arguments()[2], finalList);
            if(unifyResult == nullptr)
            {
                // unification failed: fail!
                Trace1("FAIL       ", "findall() rule failed to unify result with bag: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, HtnTerm::ToString((*currentNode->resolvent())[2]->arguments()));
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            else
            {
                // success! Treat this node as though it unified with a rule that resolved to true.
                // Just like if we unified with a normal rule, we continue the depth first search skipping the current goal
                // The unifiers we found get added to the list of unifiers
                // No new goals were added since it just resolved to "true"
                Trace1("           ", "findall() rule succeeded, new unification: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, ToString(*unifyResult));
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, { *unifyResult }, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
            
            currentNode->PopStandaloneResolve(state);
        }
            break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}


// first(?SetOfResolvedTerms...)
void HtnGoalResolver::RuleFirst(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;

    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // First will evaluate ALL of the potential resolutions and then only return the first if it exists instead of following them all through to a solution
            // Make sure we keep around the value of ?Variable and any variables in the rest of the resolvent (they won't be there when we do the resolve, so they will get stripped out)
            shared_ptr<ResolveNode::TermSetType> variablesToKeep = shared_ptr<ResolveNode::TermSetType>(new ResolveNode::TermSetType());
            for(shared_ptr<HtnTerm> term : *currentNode->resolvent())
            {
                term->GetAllVariables(variablesToKeep.get());
            }

            // Run the resolver just on the arguments as if it were a standalone resolution.  Then continue on depending on what happens
            currentNode->PushStandaloneResolve(state, variablesToKeep, goal->arguments().rbegin(), goal->arguments().rend(), ResolveContinuePoint::CustomContinue1);
        }
        break;

        case ResolveContinuePoint::CustomContinue1:
        {
            // Solutions now contains just solutions to what was in the first() term
            if(solutions == nullptr)
            {
                // There were no solutions: fail!
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            else
            {
                // Create a fake "rule" so we can continue the search.
                shared_ptr<HtnRule> rule = shared_ptr<HtnRule>(new HtnRule(termFactory->CreateConstant("firstResult"), {}));
                
                // Add the first solution we found as a "unified rule" so we can loop through it
                currentNode->rulesThatUnify = shared_ptr<vector<RuleBindingType>>(new vector<RuleBindingType>());
                currentNode->rulesThatUnify->push_back(RuleBindingType(rule, (*solutions)[0]));
                
                // Since the solution we have already has the unifiers from this node included,
                // get rid of the unifiers for this node so we don't try to remerge each fake unified rule with them again
                currentNode->unifier = shared_ptr<UnifierType>(new UnifierType());
                
                // Put the previous solutions back and continue on as if these were all unified rules
                Trace0("           ", "first() succeeded with 1 solution", state->initialIndent + resolveStack->size(), state->fullTrace);
                currentNode->currentRuleIndex = -1;
                currentNode->continuePoint = ResolveContinuePoint::NextRuleThatUnifies;
            }
            
            currentNode->PopStandaloneResolve(state);
        }
        break;

        default:
            StaticFailFastAssert(false);
            break;
    }
}

// forall(?Condition, ?Action)
// forall(?ResolvedTerm, ?ResolvedTerm)
// returns true if there are no failures, otherwise false
// Written as not(Condition, not(Action), !),  i.e., There is no instantiation of Condition for which Action is false
// will stop backtracking over all potential solutions when there is a failure
// Resolves term1, term2, in isolation.  I.e. does not
// let variable bindings go outside of the forall() statement
void HtnGoalResolver::RuleForAll(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;

    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            if(goal->arguments().size() != 2)
            {
                // Invalid program
                Trace1("ERROR      ", "forall(term1, term2) must have exactly two terms: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("forall(term1, term2) must have exactly two terms: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // Run the resolver just on not(Condition, not(Action)) as if it were a standalone resolution.  Then continue on depending on what happens
                vector<shared_ptr<HtnTerm>> implementation;
                implementation.push_back(termFactory->CreateFunctor("not", { goal->arguments()[0], termFactory->CreateFunctor("not", { goal->arguments()[1] }), termFactory->CreateConstant("!") } ));
                currentNode->PushStandaloneResolve(state, nullptr, implementation.rbegin(), implementation.rend(), ResolveContinuePoint::CustomContinue1);
            }
        }
        break;

        case ResolveContinuePoint::CustomContinue1:
        {
            // Solutions now contains just solutions to what was in the two terms
            if(solutions == nullptr)
            {
                // There were no solutions: so it is a failure!
                // Just like if we couldn't find rules to unify with, we stop the depth first search here
                Trace1("FAIL       ", "forall() rule failed: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, HtnTerm::ToString((*currentNode->resolvent())[0]->arguments()));
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            else
            {
                // All were successful: success!
                // Treat this node as though it unified with a rule that resolved to true.
                // Just like if we unified with a normal rule, we continue the depth first search skipping the current goal
                // No unifiers got added since that is the specso no changes there
                // No new goals were added since it just resolved to "true"
                Trace1("           ", "forall() rule succeeded: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, HtnTerm::ToString((*currentNode->resolvent())[0]->arguments()));
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
            
            // Put back on whatever solutions we had before the forall() so we can continue adding to them
            currentNode->PopStandaloneResolve(state);
        }
        break;

        default:
            StaticFailFastAssert(false);
            break;
    }
}

// The second argument must be ground
// The first argument can be ground OR a variable
// is(?Variable | ?Term, ?Term)
void HtnGoalResolver::RuleIs(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;

    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // the "is" operator handled as a way to unify a variable with the result of an arithmetic calculation
            // OR to test equality of a term with an arithmetic calculation
            if(goal->arguments().size() != 2 || !goal->arguments()[1]->isArithmetic())
            {
                // Invalid program
                Trace1("ERROR      ", "is() must have two terms where the left can be a variable or a term and the right is arithmetic and ground:{0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("is() must have two terms where the left can be a variable or a term and the right is arithmetic and ground: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                shared_ptr<HtnTerm> lValue = goal->arguments()[0];
                shared_ptr<HtnTerm> expression = goal->arguments()[1];
                shared_ptr<HtnTerm> exprResult = expression->Eval(termFactory);
                if(exprResult != nullptr)
                {
                    if(lValue->isVariable())
                    {
                        // We treat this as a rule where the lValue got unified with the result. So, there are no new goals to add, but there are new unifiers
                        // Nothing to do on return
                        UnifierType exprUnifier( { UnifierItemType(lValue, exprResult) } );
                        resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, exprUnifier, &(state->uniquifier)));
                        currentNode->continuePoint = ResolveContinuePoint::Return;
                        Trace2("           ", "is() succeeded {0} = {1}}", state->initialIndent + resolveStack->size(), state->fullTrace, lValue->ToString(), exprResult->ToString());
                    }
                    else
                    {
                        if(lValue->isArithmetic())
                        {
                            shared_ptr<HtnTerm> lValueResult = lValue->Eval(termFactory);
                            if(lValueResult->TermCompare(*exprResult) == 0)
                            {
                                // Rule resolves to true so no new terms, no unifiers got added since it was ground so no changes there
                                // Nothing to process on children so no special return handling
                                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                                currentNode->continuePoint = ResolveContinuePoint::Return;
                                Trace2("           ", "is() succeeded {0} == {1}", state->initialIndent + resolveStack->size(), state->fullTrace, lValueResult->ToString(), exprResult->ToString());
                            }
                            else
                            {
                                // lValue != rValue so it fails
                                Trace2("FAIL       ", "{0} is not {1}", state->initialIndent + resolveStack->size(), state->fullTrace, lValueResult->ToString(), exprResult->ToString());
                                state->RecordFailure(goal, currentNode);
                                resolveStack->pop_back();
                            }
                        }
                        else
                        {
                            // lValue is not arithmetic so it fails
                            Trace1("FAIL       ", "not arithmetic {0}", state->initialIndent + resolveStack->size(), state->fullTrace, lValue->ToString());
                            state->RecordFailure(goal, currentNode);
                            resolveStack->pop_back();
                        }
                    }
                }
                else
                {
                    // expression can't be resolved, therefore nothing down this branch can work and it fails
                    Trace1("FAIL       ", "can't arithmetically evaluate {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                    state->RecordFailure(goal, currentNode);
                    resolveStack->pop_back();
                }
            }
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// isAtom(term)
void HtnGoalResolver::RuleIsAtom(ResolveState* state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>>& resolveStack = state->resolveStack;
    HtnTermFactory* termFactory = state->termFactory;

    switch (currentNode->continuePoint)
    {
    case ResolveContinuePoint::CustomStart:
    {
        // the "atom" operator accepts only one term
        if (goal->arguments().size() != 1)
        {
            // Invalid program
            Trace1("ERROR      ", "atom() must have one term:{0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
            StaticFailFastAssertDesc(false, ("atom() must have one term: " + goal->ToString()).c_str());
            currentNode->continuePoint = ResolveContinuePoint::ProgramError;
        }
        else
        {
            shared_ptr<HtnTerm> term = goal->arguments()[0];
            if (term->isConstant())
            {
                // Rule resolves to true so no new terms, no unifiers got added since it was ground so no changes there
                // Nothing to process on children so no special return handling
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
            else
            {
                // Not an atom, therefore nothing down this branch can work and it fails
                Trace1("FAIL       ", "not an atom {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
        }
    }
    break;

    default:
        StaticFailFastAssert(false);
        break;
    }
}

// nl()
void HtnGoalResolver::RuleNewline(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            string opName = currentNode->currentGoal()->name();
            if(goal->arguments().size() != 0)
            {
                // Invalid program
                Trace2("ERROR      ", "nl() must have zero terms: {1}", state->initialIndent + resolveStack->size(), state->fullTrace, opName, goal->ToString());
                StaticFailFastAssertDesc(false, ("nl() must have zero terms: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                cout << endl;
                
                // Rule resolves to true so no new terms, no unifiers got added since it was ground so no changes there
                // Nothing to process on children so no special return handling
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// not(?SetOfResolvedTerms)
void HtnGoalResolver::RuleNot(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;

    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // this entire branch of the tree will succeed only if it fails
            // Run the resolver just on the arguments as if it were a standalone resolution.  Then continue on depending on what happens
            currentNode->PushStandaloneResolve(state, nullptr, goal->arguments().rbegin(), goal->arguments().rend(), ResolveContinuePoint::CustomContinue1);
        }
        break;

        case ResolveContinuePoint::CustomContinue1:
        {
            // Solutions now contains just solutions to what was in the not() term
            if(solutions == nullptr)
            {
                // There were no solutions: success! Treat this node as though it unified with a rule that resolved to true.
                // Just like if we unified with a normal rule, we continue the depth first search skipping the current goal
                // No unifiers got added since it was ground so no changes there
                // No new goals were added since it just resolved to "true"
                Trace1("           ", "not() rule succeeded, goals are false: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, HtnTerm::ToString((*currentNode->resolvent())[0]->arguments()));
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
            else
            {
                // There were solutions: fail!
                // Just like if we couldn't find rules to unify with, we stop the depth first search here
                Trace1("FAIL       ", "not() rule failed, goals are true: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, HtnTerm::ToString((*currentNode->resolvent())[0]->arguments()));
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            
            // Put back on whatever solutions we had before the not() so we can continue adding to them
            currentNode->PopStandaloneResolve(state);
        }
        break;

        default:
            StaticFailFastAssert(false);
            break;
    }
}

// retract(?Term)
void HtnGoalResolver::RuleRetract(ResolveState* state)
{
	shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
	shared_ptr<HtnTerm> goal = currentNode->currentGoal();
	shared_ptr<vector<shared_ptr<ResolveNode>>>& resolveStack = state->resolveStack;
	HtnTermFactory* termFactory = state->termFactory;
	HtnRuleSet* prog = state->prog;

	switch (currentNode->continuePoint)
	{
		case ResolveContinuePoint::CustomStart:
		{
			// the retract rule needs a single term
			if (goal->arguments().size() != 1)
			{
				// Invalid program
				Trace1("ERROR      ", "retract() must have exactly one term: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
				StaticFailFastAssertDesc(false, ("retract() must have exactly one term: " + goal->ToString()).c_str());
				currentNode->continuePoint = ResolveContinuePoint::ProgramError;
			}
            else
            {
                shared_ptr<HtnTerm> term = goal->arguments()[0];
                vector<shared_ptr<HtnTerm>> retractList;
                if (term->isGround())
                {
                    if(prog->HasFact(term))
                    {
                        retractList.push_back(term);
                    }
                    else
                    {
                        Trace1("FAIL       ", "retract() rule failed, fact doesn't exist: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, term->ToString());
                        state->RecordFailure(goal, currentNode);
                        resolveStack->pop_back();
                        return;
                    }
                }
                else
                {
                    // TODO: What to do?
                    StaticFailFastAssert(false);
                    //prog->AllRules([&](const HtnRule & item)
                    //	{
                    //		if (item.IsFact() && item.head()->isEquivalentCompoundTerm(term))
                    //		{
                    //			shared_ptr<UnifierType> sub = Unify(termFactory, item.head(), term);
                    //			if (sub != nullptr)
                    //			{
                    //				shared_ptr<HtnTerm> newTerm = SubstituteUnifiers(termFactory, *sub, term);
                    //				// Should be ground since we are unifying with a fact
                    //				StaticFailFastAssert(newTerm->isGround());
                    //				retractList.push_back(newTerm);
                    //			}
                    //		}

                    //		return true;
                    //	});
                }

                // Remove this fact into the database.
                prog->Update(termFactory, retractList, {});

                // Rule resolves to true so no new terms, no unifiers got added since it it is not unified
                // Nothing to process on children so no special return handling
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
        }
		break;

		default:
			StaticFailFastAssert(false);
			break;
	}
}

// retractall(?Term)
void HtnGoalResolver::RuleRetractAll(ResolveState* state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>>& resolveStack = state->resolveStack;
    HtnTermFactory* termFactory = state->termFactory;
    HtnRuleSet* prog = state->prog;
    
    switch (currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // the retractAll rule needs a single term
            if (goal->arguments().size() != 1)
            {
                // Invalid program
                Trace1("ERROR      ", "retractall() must have exactly one term: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("retractall() must have exactly one term: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                shared_ptr<HtnTerm> term = goal->arguments()[0];
                vector<shared_ptr<HtnTerm>> factsToRemove;
                prog->AllRules([&](const HtnRule &item)
                   {
                       // We only remove facts, so skip rules
                       // Don't bother if they are not "equivalent" (i.e. the name and term count doesn't match)
                       // because it can't unify
                       if(item.IsFact() && (item.head()->isEquivalentCompoundTerm(term.get())))
                       {
                           shared_ptr<UnifierType> sub = HtnGoalResolver::Unify(termFactory, item.head(), term);
                           
                           if(sub != nullptr)
                           {
                               factsToRemove.push_back(item.head());
                           }
                       }
                       
                       // Keep going
                       return true;
                   });
                
                if(factsToRemove.size() > 0)
                {
                    prog->Update(termFactory, factsToRemove, {});
                }
                
                // Rule resolves to true so no new terms, no unifiers got added since it it is not unified
                // Nothing to process on children so no special return handling
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// print(?SetOfTerms...)
void HtnGoalResolver::RulePrint(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // Print prints out a string to the debug stream
            Trace1("PRINT      ", "{0}", state->initialIndent + resolveStack->size(), true, HtnTerm::ToString(goal->arguments()));

            // Rule resolves to true so no new terms, no unifiers got added since it was ground so no changes there
            // Nothing to process on children so no special return handling
            resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
            currentNode->continuePoint = ResolveContinuePoint::Return;
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// sortBy(?Variable, [ComparisonOperator](?SetOfResolvedTerms...))
// where ?v is a variable symbol, e is a comparison operator, and l is a logical expression.
void HtnGoalResolver::RuleSortBy(ResolveState *state)
{
//    int indentLevel = state->initialIndent + state->resolveStack->size();
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<UnifierType>> &solutions = state->solutions;
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;

    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            // the count operator needs a variable as the first argument and then a set of terms to resolve
            if(goal->arguments().size() != 2 || !goal->arguments()[0]->isVariable())
            {
                // Invalid program
                Trace1("ERROR      ", "sortBy(?Var, comparer(...)) must have exactly two terms where the first is a variable: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("sortBy(?Var, comparer(...)) must have exactly two terms where the first is a variable: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // Make sure we keep around the value of ?Variable and any variables in the rest of the resolvent (they won't be there when we do the resolve, so they will get stripped out)
                shared_ptr<ResolveNode::TermSetType> variablesToKeep = shared_ptr<ResolveNode::TermSetType>(new ResolveNode::TermSetType());
                for(shared_ptr<HtnTerm> term : *currentNode->resolvent())
                {
                    term->GetAllVariables(variablesToKeep.get());
                }

                // Run the resolver just on the arguments as if it were a standalone resolution.  Then continue on depending on what happens
                currentNode->PushStandaloneResolve(state, variablesToKeep, goal->arguments()[1]->arguments().rbegin(), goal->arguments()[1]->arguments().rend(), ResolveContinuePoint::CustomContinue1);
            }
        }
        break;
            
        case ResolveContinuePoint::CustomContinue1:
        {
            // Solutions now contains just solutions to what was in the sortBy() term
            if(solutions == nullptr)
            {
                // There were no solutions: fail!
                // Put back on whatever solutions we had before the first() so we can continue adding to them
                state->RecordFailure(goal, currentNode);
                resolveStack->pop_back();
            }
            else
            {
                // A sorted precondition has the form
                // sortBy(?v, [e](l))
                // where ?v is a variable symbol, e is an operator and l is a term expression.
                StaticFailFastAssert((*currentNode->resolvent())[0]->arguments().size() == 2);
                string variable = (*currentNode->resolvent())[0]->arguments()[0]->name();
                string comparison = (*currentNode->resolvent())[0]->arguments()[1]->name();
                
                // Put all of the Evaluated values in a vector along with the index where they came from
                vector<pair<int, shared_ptr<HtnTerm>>> items;
                for(int index = 0; index < solutions->size(); ++index)
                {
                    UnifierType &binding = (*solutions)[index];
                    bool found = false;
                    for(UnifierItemType item : binding)
                    {
                        if(item.first->name() == variable)
                        {
                            shared_ptr<HtnTerm> term = item.second;
                            // Variable can contain any valid term
                            StaticFailFastAssert(term != nullptr);
                            items.push_back(pair<int, shared_ptr<HtnTerm>>(index, term));
                            found = true;
                            break;
                        }
                    }
                    
                    // Need to have the variable available
                    StaticFailFastAssert(found);
                }
                
                // Sort them
                std::sort(items.begin(), items.end(), [&](pair<int, shared_ptr<HtnTerm>> &first, pair<int, shared_ptr<HtnTerm>> &second)
                          {
                              int result = first.second->TermCompare(*second.second);

                              if(comparison == "<") { return result == -1; }
                              else if(comparison == ">") { return result == 1; }
                              else { StaticFailFastAssert(false); return false; }
                          });
                
                // Create a fake "rule" so we can continue the search.
                shared_ptr<HtnRule> sortRule = shared_ptr<HtnRule>(new HtnRule(termFactory->CreateConstant("sortedResult"), {}));
                
                // Add all the solutions we found as "unified rules" so we can loop through them
                currentNode->rulesThatUnify = shared_ptr<vector<RuleBindingType>>(new vector<RuleBindingType>());
                for(auto item : items)
                {
                    currentNode->rulesThatUnify->push_back(RuleBindingType(sortRule, (*solutions)[item.first]));
                }
                
                // Since the solutions we have already have the unifiers from this node already included,
                // get rid of the unifiers for this sortby() node so we don't try to remerge each fake unified rule with them again
                currentNode->unifier = shared_ptr<UnifierType>(new UnifierType());
                
                // Put the previous solutions back and continue on as if these were all unified rules
                Trace1("           ", "sortBy() succeeded with {0} solutions", state->initialIndent + resolveStack->size(), state->fullTrace, currentNode->rulesThatUnify->size());
                currentNode->currentRuleIndex = -1;
                currentNode->continuePoint = ResolveContinuePoint::NextRuleThatUnifies;
            }
            
            currentNode->PopStandaloneResolve(state);
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// Compares two terms to see if they are literally identical
// The '==' operator does not unify the variables, so a variable will NOT be equal to anything other than the same unbound variable.
// op(?Term, ?Term)
void HtnGoalResolver::RuleTermCompare(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;

    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            if(goal->arguments().size() != 2)
            {
                // Invalid program
                Trace1("ERROR      ", "{0} must have two terms", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("Must have two terms: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // No resolution happens beyond what has already happened
                bool isEqual = (*currentNode->currentGoal()->arguments()[0]) == (*currentNode->currentGoal()->arguments()[1]);
                string opName = currentNode->currentGoal()->name();
                if((isEqual && opName == "==") || (!isEqual && opName == "\\=="))
                {
                    Trace1("           ", "{0} succeeded", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                    
                    // Rule resolves to true so no new terms, no unifiers got added so no changes there
                    // Nothing to process on children so no special return handling
                    resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                    currentNode->continuePoint = ResolveContinuePoint::Return;
                }
                else
                {
                    Trace1("FAIL       ", "{0} failed", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                    state->RecordFailure(goal, currentNode);
                    resolveStack->pop_back();
                }
            }
        }
        break;

        default:
            StaticFailFastAssert(false);
            break;
    }
}

// showtraces(?SetOfResolvedTerms...)
void HtnGoalResolver::RuleTrace(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            Trace0("TRACE      ", "Detailed tracing ON", state->initialIndent + resolveStack->size(), true);
            
            // Need to shut off tracing when done so we have special return handling
            state->fullTrace = true;

            // Add all of the arguments as terms so they will get resolved, but no new unifiers
            vector<shared_ptr<HtnTerm>> terms;
            terms.insert(terms.begin(), goal->arguments().begin(), goal->arguments().end());
            resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, terms, {}, &(state->uniquifier)));
            currentNode->continuePoint = ResolveContinuePoint::CustomContinue1;
        }
        break;
            
        case ResolveContinuePoint::CustomContinue1:
        {
            // Nothing to do on return except shut tracing off
            Trace0("TRACE      ", "Detailed tracing OFF", state->initialIndent + resolveStack->size(), true);
            state->fullTrace = false;
            currentNode->continuePoint = ResolveContinuePoint::Return;
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// =(?Term, ?Term)
// True if unification is possible between two terms, and then binding of variables occurs
void HtnGoalResolver::RuleUnify(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            if(goal->arguments().size() != 2)
            {
                // Invalid program
                Trace1("ERROR      ", "=() must have two terms: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, goal->ToString());
                StaticFailFastAssertDesc(false, ("=() must have two terms: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                // Just unify the two arguments
                shared_ptr<UnifierType> result = Unify(termFactory, goal->arguments()[0], goal->arguments()[1]);
                if(result == nullptr)
                {
                    // There were no solutions: fail!
                    state->RecordFailure(goal, currentNode);
                    resolveStack->pop_back();
                }
                else
                {
                    // success! Treat this node as though it unified with a rule that resolved to true.
                    // Just like if we unified with a normal rule, we continue the depth first search skipping the current goal
                    // The unifiers we found get added to the list of unifiers
                    // No new goals were added since it just resolved to "true"
                    Trace1("           ", "=() rule succeeded, new unification: {0}", state->initialIndent + resolveStack->size(), state->fullTrace, ToString(*result));
                    resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, { *result }, &(state->uniquifier)));
                    currentNode->continuePoint = ResolveContinuePoint::Return;
                }
            }
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

// write(?Term) | writeln(?Term)
void HtnGoalResolver::RuleWrite(ResolveState *state)
{
    shared_ptr<ResolveNode> currentNode = state->resolveStack->back();
    shared_ptr<HtnTerm> goal = currentNode->currentGoal();
    shared_ptr<vector<shared_ptr<ResolveNode>>> &resolveStack = state->resolveStack;
    HtnTermFactory *termFactory = state->termFactory;
    
    switch(currentNode->continuePoint)
    {
        case ResolveContinuePoint::CustomStart:
        {
            string opName = currentNode->currentGoal()->name();
            if(goal->arguments().size() != 1)
            {
                // Invalid program
                Trace2("ERROR      ", "{0}() must have one term: {1}", state->initialIndent + resolveStack->size(), state->fullTrace, opName, goal->ToString());
                StaticFailFastAssertDesc(false, ("Must have one term: " + goal->ToString()).c_str());
                currentNode->continuePoint = ResolveContinuePoint::ProgramError;
            }
            else
            {
                if(opName == "write")
                {
                    cout << HtnTerm::ToString(goal->arguments(), false);
                }
                else if(opName == "writeln")
                {
                    cout << HtnTerm::ToString(goal->arguments(), false) << endl;
                }
                else
                {
                    StaticFailFastAssert(false);
                }
                
                // Rule resolves to true so no new terms, no unifiers got added since it was ground so no changes there
                // Nothing to process on children so no special return handling
                resolveStack->push_back(currentNode->CreateChildNode(termFactory, *state->initialGoals, {}, {}, &(state->uniquifier)));
                currentNode->continuePoint = ResolveContinuePoint::Return;
            }
        }
        break;
            
        default:
            StaticFailFastAssert(false);
            break;
    }
}

void HtnGoalResolver::SubstituteAllVariables(HtnTermFactory *factory, shared_ptr<HtnTerm> newTerm, shared_ptr<HtnTerm> existingVariable, vector<pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>>> &stack, UnifierType &solution)
{
    for(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> &value : solution)
    {
        pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> newSolution;
        newSolution.first = value.first->SubstituteTermForVariable(factory, newTerm, existingVariable);
        newSolution.second = value.second->SubstituteTermForVariable(factory, newTerm, existingVariable);
        value = newSolution;
    }
    
    for(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> &value : stack)
    {
        pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> newStackItem;
        newStackItem.first = value.first->SubstituteTermForVariable(factory, newTerm, existingVariable);
        newStackItem.second = value.second->SubstituteTermForVariable(factory, newTerm, existingVariable);
        value = newStackItem;
    }
}

// TODO: This is really inefficient because it creates lot of memory garbage
// Replace all instances of X in destination with whatever X is equal to in source
shared_ptr<UnifierType> HtnGoalResolver::SubstituteUnifiers(HtnTermFactory *factory, const UnifierType &source, const UnifierType &destination)
{
    shared_ptr<UnifierType> finalUnifier = shared_ptr<UnifierType>(new UnifierType());
    for(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> destItem : destination)
    {
        shared_ptr<HtnTerm> newDestItem = destItem.second;
        for(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> sourceItem : source)
        {
            if(newDestItem->OccursCheck(sourceItem.first))
            {
                // This destItem has this source variable, replace it
                newDestItem = newDestItem->SubstituteTermForVariable(factory, sourceItem.second, sourceItem.first);
            }
        }
        
        finalUnifier->push_back(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>>(destItem.first, newDestItem));
    }
    
    return finalUnifier;
}

shared_ptr<HtnTerm> HtnGoalResolver::SubstituteUnifiers(HtnTermFactory *factory, const UnifierType &source, shared_ptr<HtnTerm>target)
{
    for(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> sourceItem : source)
    {
        if(target->OccursCheck(sourceItem.first))
        {
            // This target has this source variable, replace it
            target = target->SubstituteTermForVariable(factory, sourceItem.second, sourceItem.first);
        }
    }
    
    return target;
}

shared_ptr<vector<shared_ptr<HtnTerm>>> HtnGoalResolver::SubstituteUnifiers(HtnTermFactory *factory, const UnifierType &source, const vector<shared_ptr<HtnTerm>> &terms)
{
    shared_ptr<vector<shared_ptr<HtnTerm>>> substituted = shared_ptr<vector<shared_ptr<HtnTerm>>>(new vector<shared_ptr<HtnTerm>>());
    for(auto term : terms)
    {
        substituted->push_back(SubstituteUnifiers(factory, source, term));
    }
    
    return substituted;
}

string HtnGoalResolver::ToString(const vector<UnifierType> *unifierList, bool json)
{
    if(unifierList == nullptr)
    {
        return json ? "" : "null";
    }
    
    stringstream stream;
    
    stream << (json ? "[" : "(");
    bool hasItem = false;
    for(auto item : *unifierList)
    {
        stream << (hasItem ? ", " : "") << ToString(item, json);
        hasItem = true;
    }
    
    stream << (json ? "]" : ")");
    return stream.str();
}

string HtnGoalResolver::ToString(const UnifierType &unifier, bool json)
{
    stringstream stream;
    
    stream << (json ? "{" : "(");
    bool hasItem = false;
    for(auto item : unifier)
    {
        if (json)
        {
            stream << (hasItem ? ", " : "") << "\"" << item.first->ToString() << "\" : " << item.second->ToString(false, true);
        }
        else
        {
            stream << (hasItem ? ", " : "") << item.first->ToString() << " = " << item.second->ToString();
        }
        hasItem = true;
    }
    
    stream << (json ? "}" : ")");
    return stream.str();
}

// From http://homepage.cs.uiowa.edu/~fleck/unification.pdf
// We are unifying two terms which can have variables
// Required Expression Operations:
//      IsVariable: True if expression is solely a variable
//      does not occur: check if a variable occurs in a different expression
//      Substitute variable A for B in the stack and in answer
//          Means variables need to be able to point to other variables
//          This means that variables can contain expressions, so when we are traversing the expression tree, we have to *resolve*
//          those nodes first before looping.  Maybe it is just an overload of the children. Can't quite be that because it will be a level of indirection
// Unify: initialLeft and initialRight
// Output: Answer, which is always in the form Variable = Term
// There is a stack of left and right terms
// There is a substitution: answer which is empty
// Push initialLeft and initialRight
// while stack is not empty
//      Pop left and right from stack
//      if X is a variable that does not occur in Y: Sub Y for X in thestack and in Answer
shared_ptr<UnifierType> HtnGoalResolver::Unify(HtnTermFactory *factory, shared_ptr<HtnTerm> term1, shared_ptr<HtnTerm> term2)
{
    if(term1 == nullptr || term2 == nullptr) return nullptr;
    
    uint64_t &uniquifier = factory->uniquifier();
//    TraceString2("HtnGoalResolver::Unify {0}={1}",
//                 SystemTraceType::Unifier, TraceDetail::Diagnostic,
//                 term1->ToString(), term2->ToString());
    
    // solution.first = left side of equality, solution.second = right
    shared_ptr<UnifierType> solution = shared_ptr<UnifierType>(new UnifierType);
    
    // stack.first = left term, stack.second = right
    vector<pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>>> remainingStack;
    
    // Initially the stack contains the original terms
    remainingStack.push_back(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>>(term1, term2));
    
    while(!remainingStack.empty())
    {
//        TraceString("Solution:", SystemTraceType::Unifier, TraceDetail::Diagnostic);
//        for(pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>> item : *solution)
//        {
//            TraceString2("...solution: {0}={1}",
//                         SystemTraceType::Unifier, TraceDetail::Diagnostic,
//                         item.first->ToString(), item.second->ToString());
//        }
//
//        TraceString("Stack:", SystemTraceType::Unifier, TraceDetail::Diagnostic);
//        for(pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>> item : remainingStack)
//        {
//            TraceString2("...stack: {0}={1}",
//                         SystemTraceType::Unifier, TraceDetail::Diagnostic,
//                         item.first->ToString(), item.second->ToString());
//        }
        
        pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>> current = remainingStack.back();
        remainingStack.pop_back();
        // If X or Y is a "don't care" variable that hasn't been renamed yet, give it a unique value now
        // We can do this because, by definition, every instance of "_" is a new variable
        // So we don't have to fix them up in the resolvent to match
        shared_ptr<HtnTerm> x;
        bool xIsDontCare = false;
        shared_ptr<HtnTerm> y;
        bool yIsDontCare = false;
        if(current.first->isVariable())
        {
            std::string name = current.first->name();
            if(name.size() == 1 && name[0] == '_')
            {
                xIsDontCare = true;
                x = factory->CreateVariable("_" + lexical_cast<string>(uniquifier++));
            }
            else
            {
                x = current.first;
            }
        }
        else
        {
            x = current.first;
        }

        if(current.second->isVariable())
        {
            std::string name = current.second->name();
            if(name.size() == 1 && name[0] == '_')
            {
                yIsDontCare = true;
                y = factory->CreateVariable("_" + lexical_cast<string>(uniquifier++));
            }
            else
            {
                y = current.second;
            }
        }
        else
        {
            y = current.second;
        }
        
        // If X is a variable that does not occur in Y..
        if(x->isVariable() && (xIsDontCare || !y->OccursCheck(x)))
        {
            // Substitute Y for X in the stack and in the solution
            SubstituteAllVariables(factory, y, x, remainingStack, *solution);
            
            // add X = Y to solution
            solution->push_back(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>>(x, y));
        }
        else if(y->isVariable() && (yIsDontCare || !x->OccursCheck(y)))
        {
            // Substitute X for Y in the stack and in the solution
            SubstituteAllVariables(factory, x, y, remainingStack, *solution);
            
            // add Y = X to solution
            solution->push_back(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>>(y, x));
        }
        else if(!(xIsDontCare || yIsDontCare) &&
                (((x->isVariable() && y->isVariable()) && x == y) ||
                ((x->isConstant() && y->isConstant()) && x->nameEqualTo(*y))))
        {
            // X && Y are identical constants or Variables
            continue;
        }
        else if(x->isEquivalentCompoundTerm(y.get()))
        {
            // X is f(X1...,Xn) and Y is f(Y1...,Yn) for some functor f and n > 0
            // push Xi = Yi , i=1...n, on the stack
            vector<shared_ptr<HtnTerm>>::const_iterator xIter = x->arguments().begin();
            vector<shared_ptr<HtnTerm>>::const_iterator yIter = y->arguments().begin();
            while(xIter != x->arguments().end())
            {
                remainingStack.push_back(pair<shared_ptr<HtnTerm>, shared_ptr<HtnTerm>>(*xIter, *yIter));
                xIter++;
                yIter++;
            }
        }
        else
        {
            // Fail
//            TraceString("Final Solution: FAIL", SystemTraceType::Unifier, TraceDetail::Diagnostic);
            return nullptr;
        }
    }
    
//    TraceString("Final Solution:", SystemTraceType::Unifier, TraceDetail::Diagnostic);
//    for(pair<shared_ptr<HtnTerm>,shared_ptr<HtnTerm>> item : *solution)
//    {
//        TraceString2("...solution {0}={1}",
//                     SystemTraceType::Unifier, TraceDetail::Diagnostic,
//                     item.first->ToString(), item.second->ToString());
//    }
    return solution;
}
