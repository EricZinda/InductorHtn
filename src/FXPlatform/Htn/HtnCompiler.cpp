//
//  HtnCompiler.cpp
//  GameLib
//
//  Created by Eric Zinda on 10/8/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//

#include "HtnCompiler.h"
#include "HtnGoalResolver.h"
#include "HtnMethod.h"
#include "HtnOperator.h"
#include "HtnParser.h"
#include "HtnPlanner.h"
#include "HtnRuleSet.h"
#include "HtnTermFactory.h"
using namespace Htn;

void HtnCompiler::Clear()
{
    PrologCompiler::Clear();
    m_domain->ClearAll();
}

void HtnCompiler::ClearWithNewRuleSet()
{
    PrologCompiler::ClearWithNewRuleSet();
    m_domain->ClearAll();
}

// If this subtask is already on the stack, this is a loop, error and return
void HtnCompiler::CheckMethodForLoop(HtnMethod *method, vector<string> &stack, set<string> &loops)
{
    // Grab each subtask
    for(auto possibleSubtask : method->tasks())
    {
        // Ignore the try block
        vector<shared_ptr<HtnTerm>> subtasks;
        if(possibleSubtask->name() == "try")
        {
            subtasks.insert(subtasks.begin(), possibleSubtask->arguments().begin(), possibleSubtask->arguments().end());
        }
        else
        {
            subtasks.push_back(possibleSubtask);
        }
        
        for(auto subtask : subtasks)
        {
            // See if it matches any other methods
            bool foundTask = false;
            m_domain->AllMethods([&](HtnMethod *method)
               {
                   shared_ptr<HtnTerm> foundHead = method->head();
                   if(foundHead->isEquivalentCompoundTerm(subtask))
                   {
                       // We found at least one method that matches
                       foundTask = true;
                       string arityString =  lexical_cast<string>(foundHead->arity());
                       string idName = foundHead->name() + "/" + arityString;
                       
                       // See if it loops
                       vector<string>::iterator found = std::find(stack.begin(), stack.end(), idName);
                       if(found != stack.end())
                       {
                           // It does.
                           // If we have a declaration that says this is safe, don't report it
                           shared_ptr<HtnTerm> safeFact = m_termFactory->CreateConstantFunctor("loopSafe", { foundHead->name(), arityString });
                           if(!m_state->HasFact(safeFact))
                           {
                               stringstream stackString;
                               stackString << "Task Loop: ";
                               while(found != stack.end())
                               {
                                   stackString << *found << "...";
                                   ++found;
                               }
                               stackString << "LOOP -> " + idName;
                               
                               auto loopFound = loops.find(stackString.str());
                               if(loopFound == loops.end())
                               {
                                   TraceString1("Warning: {0}",
                                                SystemTraceType::Planner, TraceDetail::Normal,
                                                stackString.str());
                                   loops.insert(stackString.str());
                               }
                           }
                           
                           // Since it is a loop, we must quit the search (i.e. return false)
                           // or we will loop forever
                           return false;
                       }
                       
                       // Otherwise DFS
                       stack.push_back(idName);
                       CheckMethodForLoop(method, stack, loops);
                       stack.pop_back();
                       
                       // Continue searching the subtasks of alternatives
                       return true;
                   }
                   
                   return true;
               });
            
            if(!foundTask)
            {
                m_domain->AllOperators([&](HtnOperator *op)
                    {
                        shared_ptr<HtnTerm> foundHead = op->head();
                        if(foundHead->isEquivalentCompoundTerm(subtask))
                        {
                            foundTask = true;
                            return false;
                        }
                        
                        return true;
                    });
                
                if(!foundTask)
                {
                    // See if it is declared
                    string ruleArityString = lexical_cast<string>(subtask->arity());
                    shared_ptr<HtnTerm> declaration = m_termFactory->CreateConstantFunctor("declareTask", { subtask->name(),  ruleArityString });
                    if(!m_state->HasFact(declaration))
                    {
                        string idError = "Task Not Found: " + subtask->name() + "/" + ruleArityString;
                        auto loopFound = loops.find(idError);
                        if(loopFound == loops.end())
                        {
                            TraceString1("Warning: {0}",
                                         SystemTraceType::Planner, TraceDetail::Normal,
                                         idError);
                            loops.insert(idError);
                        }
                    }
                }
            }
        }
    }
}

set<string> HtnCompiler::FindLogicErrors(shared_ptr<HtnGoalResolver> resolver)
{
    // Check any Axioms for logic errors
    set<string> loops = PrologCompiler::FindRuleLogicErrors(resolver);
    
    // Go through each task in the system that isn't an operator
    // 1. Do a DFS of it and report when it loops
    // 2. Make sure the condition methods exist
    vector<string> stack;
    m_domain->AllMethods([&](HtnMethod *method)
                         {
                             string idName = method->head()->name() + "/" + lexical_cast<string>(method->head()->arity());
                             stack.push_back(idName);
                             CheckMethodForLoop(method, stack, loops);
                             stack.pop_back();
                             
                             CheckMethodCondition(resolver, method, loops);
                             return true;
                         });
    
    return loops;
}

void HtnCompiler::CheckMethodCondition(shared_ptr<HtnGoalResolver> resolver, HtnMethod *method, set<string> &loops)
{
    // Go through each term in the condition
    vector<string> stack;
    string idName = "Method Condition: " + method->head()->name() + "/" + lexical_cast<string>(method->head()->arity());
    stack.push_back(idName);
    
    // This is overkill since we really don't need to re
    CheckRuleRecurseTail(resolver, idName, method->head()->arity(), method->condition(), stack, loops);
}

// HTN Operators and methods are just Prolog rules interpreted in a special way
// so we don't have to make up a new syntax
void HtnCompiler::ParseRule(shared_ptr<Symbol> symbol)
{
    shared_ptr<Symbol> head = GetChild(symbol, 0, -1);
    
    vector<shared_ptr<HtnTerm>> list;
    for(int index = 1; index < symbol->children().size(); index++)
    {
        shared_ptr<Symbol> item = GetChild(symbol, index, -1);
        list.push_back(CreateTermFromItem(m_termFactory, item));
    }
    
    HtnMethodType isSetOf = HtnMethodType::Normal;
    bool isDefault = false;
    bool isOperatorHidden = false;
    shared_ptr<HtnTerm>constraint = nullptr;
    shared_ptr<HtnTerm>del = nullptr;
    for(auto item : list)
    {
        if(item->isConstant() && item->name() == "else")
        {
            isDefault = true;
        }
        else if(item->isConstant() && item->name() == "allOf")
        {
            isSetOf = HtnMethodType::AllSetOf;
        }
        else if(item->isConstant() && item->name() == "anyOf")
        {
            isSetOf = HtnMethodType::AnySetOf;
        }
        else if(item->isConstant() && item->name() == "hidden")
        {
            isOperatorHidden = true;
        }
        else if(item->name() == "if")
        {
            FailFastAssert(constraint == nullptr);
            constraint = item;
        }
        else if(item->name() == "do")
        {
            FailFastAssert(constraint != nullptr);
            m_domain->AddMethod(CreateTermFromFunctor(m_termFactory, head), constraint->arguments(), item->arguments(), isSetOf, isDefault);
            return;
        }
        else if(item->name() == "del")
        {
            FailFastAssert(del == nullptr && isSetOf == HtnMethodType::Normal && isDefault == false && constraint == nullptr);
            del = item;
        }
        else if(item->name() == "add")
        {
            FailFastAssert(del != nullptr && isSetOf == HtnMethodType::Normal && isDefault == false && constraint == nullptr);
            m_domain->AddOperator(CreateTermFromFunctor(m_termFactory, head), item->arguments(), del->arguments(), isOperatorHidden);
            return;
        }
    }
    
    m_state->AddRule(CreateTermFromFunctor(m_termFactory, head), list);
}

