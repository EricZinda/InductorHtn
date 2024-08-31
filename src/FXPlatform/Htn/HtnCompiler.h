//
//  HtnCompiler.hpp
//  GameLib
//
//  Created by Eric Zinda on 10/8/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//
#ifndef HtnCompiler_hpp
#define HtnCompiler_hpp
#include "FXPlatform/Parser/Compiler.h"
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "HtnDomain.h"
#include "HtnMethod.h"
#include "HtnOperator.h"

class HtnOperator;
class HtnMethod;
class HtnDomain;

/*  
HTN Operators and methods are just Prolog rules interpreted in a special way
so we don't have to make up a new syntax
*/
template <class VariableRule>
class HtnCompilerBase : public PrologCompilerBase<VariableRule>
{
public:
    HtnCompilerBase() :
        PrologCompilerBase<VariableRule>(nullptr, nullptr),
        m_domain(nullptr)
    {
    }
    
    HtnCompilerBase(HtnTermFactory *termFactory, HtnRuleSet *state, HtnDomain *domain) :
        PrologCompilerBase<VariableRule>(termFactory, state),
        m_domain(domain)
    {
    }

    void Clear() override
    {
        PrologCompilerBase<VariableRule>::Clear();
        m_domain->ClearAll();
    }

    void ClearWithNewRuleSet() override
    {
        PrologCompilerBase<VariableRule>::ClearWithNewRuleSet();
        m_domain->ClearAll();
    }

    // Find loops in the domain by recursing through all the do() subtasks
    // These loops *could* cause a stack overflow
    // Also find calls to tasks that aren't implemented
    
    std::set<std::string> FindLogicErrors(shared_ptr<HtnGoalResolver> resolver)
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
    
    // HTN Operators and methods are just Prolog rules interpreted in a special way
    // so we don't have to make up a new syntax
    void ParseRule(shared_ptr<Symbol> symbol) override
    {
        shared_ptr<Symbol> head = PrologCompilerBase<VariableRule>::GetChild(symbol, 0, -1);
        
        vector<shared_ptr<HtnTerm>> list;
        for(int index = 1; index < symbol->children().size(); index++)
        {
            shared_ptr<Symbol> item = PrologCompilerBase<VariableRule>::GetChild(symbol, index, -1);
            list.push_back(PrologCompilerBase<VariableRule>::CreateTermFromItem(PrologCompilerBase<VariableRule>::m_termFactory, item));
        }
        
        HtnMethodType isSetOf = HtnMethodType::Normal;
        bool isDefault = false;
        bool isOperatorHidden = false;
        shared_ptr<HtnTerm> constraint = nullptr;
        shared_ptr<HtnTerm> del = nullptr;
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
                FailFastAssertDesc(constraint == nullptr, "There can only be one if(): if(), do().");
                constraint = item;
            }
            else if(item->name() == "do")
            {
                FailFastAssertDesc(constraint != nullptr, "do() needs an if() before it: if(), do().");
                m_domain->AddMethod(PrologCompilerBase<VariableRule>::CreateTermFromFunctor(PrologCompilerBase<VariableRule>::m_termFactory, head), constraint->arguments(), item->arguments(), isSetOf, isDefault);
                return;
            }
            else if(item->name() == "del")
            {
                FailFastAssertDesc(del == nullptr && isSetOf == HtnMethodType::Normal && isDefault == false && constraint == nullptr,
                    "Improper del() statement.");
                del = item;
            }
            else if(item->name() == "add")
            {
                FailFastAssertDesc(del != nullptr && isSetOf == HtnMethodType::Normal && isDefault == false && constraint == nullptr,
                    "Improper add() statement");
                m_domain->AddOperator(PrologCompilerBase<VariableRule>::CreateTermFromFunctor(PrologCompilerBase<VariableRule>::m_termFactory, head), item->arguments(), del->arguments(), isOperatorHidden);
                return;
            }
        }
        
        PrologCompilerBase<VariableRule>::m_state->AddRule(PrologCompilerBase<VariableRule>::CreateTermFromFunctor(PrologCompilerBase<VariableRule>::m_termFactory, head), list);
    }
    
    ValueProperty(private, HtnDomain *, domain);
    


private:
    // If this subtask is already on the stack, this is a loop, error and return
    void CheckMethodForLoop(HtnMethod *method, vector<string> &stack, set<string> &loops)
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
                       if(foundHead->isEquivalentCompoundTerm(subtask.get()))
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
                               shared_ptr<HtnTerm> safeFact = PrologCompilerBase<VariableRule>::m_termFactory->CreateConstantFunctor("loopSafe", { foundHead->name(), arityString });
                               if(!PrologCompilerBase<VariableRule>::m_state->HasFact(safeFact))
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
                            if(foundHead->isEquivalentCompoundTerm(subtask.get()))
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
                        shared_ptr<HtnTerm> declaration = PrologCompilerBase<VariableRule>::m_termFactory->CreateConstantFunctor("declareTask", { subtask->name(),  ruleArityString });
                        if(!PrologCompilerBase<VariableRule>::m_state->HasFact(declaration))
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

    void CheckMethodCondition(shared_ptr<HtnGoalResolver> resolver, HtnMethod *method, set<string> &loops)
    {
        // Go through each term in the condition
        vector<string> stack;
        string idName = "Method Condition: " + method->head()->name() + "/" + lexical_cast<string>(method->head()->arity());
        stack.push_back(idName);
        
        // This is overkill since we really don't need to re
        PrologCompilerBase<VariableRule>::CheckRuleRecurseTail(resolver, idName, method->head()->arity(), method->condition(), stack, loops);
    }
};

typedef HtnCompilerBase<PrologCapitalizedVariable> HtnStandardCompiler;
typedef HtnCompilerBase<HtnVariable> HtnCompiler;

#endif /* HtnCompiler_hpp */
