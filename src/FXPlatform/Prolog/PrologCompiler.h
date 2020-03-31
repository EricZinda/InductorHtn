//
//  PrologCompiler.hpp
//  GameLib
//
//  Created by Eric Zinda on 10/10/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//

#ifndef PrologCompiler_hpp
#define PrologCompiler_hpp

#include "FXPlatform/Parser/Compiler.h"
#include "HtnGoalResolver.h"
#include "HtnRuleSet.h"
#include "HtnTerm.h"
#include "HtnTermFactory.h"
#include "PrologParser.h"
using namespace Prolog;

// VariableRule determines if the compiler uses normal prolog rules for variables
// or the "Htn-style" where ? must proceed a variable name.  See PrologParser.h for
// more details
template <class VariableRule>
class PrologCompilerBase : public Compiler<PrologDocument<VariableRule>>
{
public:
    PrologCompilerBase(HtnTermFactory *termFactory, HtnRuleSet *state) :
        m_state(state),
        m_termFactory(termFactory)
    {
    }

    virtual void Clear()
    {
        m_goals.clear();
        m_state->ClearAll();
    }
    
    // Helper that creates a new ruleset and sets it instead of just clearing it.
    // Useful if the RuleSet has been locked
    virtual void ClearWithNewRuleSet()
    {
        m_goals.clear();
        m_compilerOwnedRuleSet = shared_ptr<HtnRuleSet>(new HtnRuleSet());
        m_state = m_compilerOwnedRuleSet.get();
    }
    
    shared_ptr<vector<UnifierType>> SolveGoals()
    {
        HtnGoalResolver resolver;
        return SolveGoals(resolver);
    }

    shared_ptr<vector<UnifierType>> SolveGoals(HtnGoalResolver &resolver)
    {
        return resolver.ResolveAll(m_termFactory, m_state, m_goals);
    }

    shared_ptr<vector<UnifierType>> SolveGoals(HtnGoalResolver *resolver, int memoryBudget = 1000000, int64_t *highestMemoryUsedReturn = nullptr, int *furthestFailureIndex = nullptr, std::vector<std::shared_ptr<HtnTerm>> *farthestFailureContext = nullptr)
    {
        return resolver->ResolveAll(m_termFactory, m_state, m_goals, 0, memoryBudget, highestMemoryUsedReturn, furthestFailureIndex, farthestFailureContext);
    }

    static shared_ptr<HtnTerm> CreateTermFromFunctor(HtnTermFactory *factory, shared_ptr<Symbol> functor)
    {
        shared_ptr<Symbol> name = Compiler<PrologDocument<VariableRule>>::GetChild(functor, 0, -1);
        
        vector<shared_ptr<HtnTerm>> arguments;
        for(int argIndex = 1; argIndex < functor->children().size(); argIndex++)
        {
            shared_ptr<Symbol> arg = Compiler<PrologDocument<VariableRule>>::GetChild(functor, argIndex, -1);
            arguments.push_back(CreateTermFromItem(factory, arg));
        }
        
        return factory->CreateFunctor(name->ToString(), arguments);
    }
    
    static shared_ptr<HtnTerm> CreateTermFromItem(HtnTermFactory *factory, shared_ptr<Symbol> symbol)
    {
        switch(symbol->symbolID())
        {
            case PrologSymbolID::PrologList:
                return CreateTermFromList(factory, symbol);
                break;
            case PrologSymbolID::PrologFunctor:
                return CreateTermFromFunctor(factory, symbol);
                break;
            case PrologSymbolID::PrologVariable:
                return CreateTermFromVariable(factory, symbol);
                break;
            case PrologSymbolID::PrologAtom:
                return factory->CreateConstant(symbol->ToString());
                break;
            default:
                StaticFailFastAssert(false);
                return nullptr;
        }
    }
    
    static shared_ptr<HtnTerm> CreateTermFromList(HtnTermFactory *factory, shared_ptr<Symbol> prologList)
    {
        shared_ptr<Symbol> name = Compiler<PrologDocument<VariableRule>>::GetChild(prologList, 0, PrologSymbolID::PrologEmptyList);
        if(name != nullptr)
        {
            return factory->EmptyList();
        }
        else
        {
            // Create a single functor like this: .(firstTerm, .(secondTerm, []))
            shared_ptr<HtnTerm> lastTerm = factory->EmptyList();
            for(int argIndex = (int) prologList->children().size() - 1; argIndex >= 0; argIndex--)
            {
                shared_ptr<Symbol> term = Compiler<PrologDocument<VariableRule>>::GetChild(prologList, argIndex, -1);
                if(term->symbolID() == PrologSymbolID::PrologTailTerm)
                {
                    lastTerm = CreateTermFromItem(factory, Compiler<PrologDocument<VariableRule>>::GetChild(term, 0, -1));
                }
                else
                {
                    lastTerm = factory->CreateFunctor(".", { CreateTermFromItem(factory, term), lastTerm });
                }
            }
            
            return lastTerm;
        }
    }
    
    static shared_ptr<HtnTerm> CreateTermFromVariable(HtnTermFactory *factory, shared_ptr<Symbol> variable)
    {
        int level1Index = 0;
        shared_ptr<Symbol> name = Compiler<PrologDocument<VariableRule>>::GetChild(variable, level1Index++, -1);
        string nameString = name->ToString();
        return factory->CreateVariable(nameString);
    }

    set<string> FindRuleLogicErrors(shared_ptr<HtnGoalResolver> resolver)
    {
        // Go through each rule in the system
        // Do a DFS of terms it calls it and report when it loops
        vector<string> stack;
        set<string> loops;
        m_state->AllRules([&](const HtnRule &rule)
                          {
                              string idName = rule.head()->name() + "/" + lexical_cast<string>(rule.head()->arity());
                              stack.push_back(idName);
                              CheckRuleRecurseTail(resolver, rule.head()->name(), (int) rule.head()->arguments().size(), rule.tail(), stack, loops);
                              stack.pop_back();
                              return true;
                          });
        
        return loops;
    }

    vector<shared_ptr<HtnTerm>> &goals() { return m_goals; }

    ValueProperty(protected, shared_ptr<HtnRuleSet>, compilerOwnedRuleSet);
    ValueProperty(protected, HtnRuleSet *, state);
    ValueProperty(protected, HtnTermFactory *, termFactory);
    
protected:
    vector<shared_ptr<HtnTerm>> m_goals;
    
    void CheckRuleRecurseTail(shared_ptr<HtnGoalResolver> resolver, const string &ruleHead, int arity, const vector<shared_ptr<HtnTerm>> &ruleTail, vector<string> &stack, set<string> &loops)
    {
        // Get the metadata if this resolves to a custom rule so we know where to recurse
        HtnGoalResolver::CustomRuleType metadata;
        bool isCustom = resolver->GetCustomRule(ruleHead, (int) ruleTail.size(), metadata);
        
        // Grab each term in the tail
        int termIndex = -1;
        for(auto term : ruleTail)
        {
            ++termIndex;
            if(isCustom)
            {
                CustomRuleArgType argType = HtnGoalResolver::GetCustomRuleArgBaseType(metadata.first, termIndex);
                if(argType == CustomRuleArgType::TermOfResolvedTerms)
                {
                    // Need to recurse on the terms inside of this term and treat it like a transparent rule
                    // i.e. don't put it on the stack
                    CheckRuleRecurseTail(resolver, term->name(), (int) term->arguments().size(), term->arguments(), stack, loops);
                    continue;
                }
                else if(argType != CustomRuleArgType::ResolvedTerm)
                {
                    // Skip if this argument of our custom method is not something that will be resolved
                    continue;
                }
            }
            
            // Find all rules that this term resolves to
            if(term->isTrue())
            {
                continue;
            }
            else if(term->isCut())
            {
                continue;
            }
            else if(term->isVariable())
            {
                continue;
            }
            else if(term->isArithmetic())
            {
                // TODO: Figure out the right thing here
                continue;
            }
            else
            {
                // See if it matches any other rules
                HtnGoalResolver::CustomRuleType ignore;
                bool isCustom = resolver->GetCustomRule(term->name(), term->arity(), ignore);
                bool foundRule = false;
                if(isCustom)
                {
                    foundRule = true;
                    // Ignore loops for built in rules themselves (but not for what they call)
                    CheckRuleForLoopStackCheck(resolver, term->name(), term->arity(), term->arguments(), stack, loops, true);
                }
                else
                {
                    m_state->AllRules([&](const HtnRule &potentialRule)
                     {
                         shared_ptr<HtnTerm> foundHead = potentialRule.head();
                         if(foundHead->isEquivalentCompoundTerm(term.get()))
                         {
                             foundRule = true;
                             return CheckRuleForLoopStackCheck(resolver, foundHead->name(), foundHead->arity(), potentialRule.tail(), stack, loops, false);
                         }
                         else
                         {
                             return true;
                         }
                     });
                }
                
                if(!foundRule)
                {
                    // See if it is declared
                    string ruleArityString = lexical_cast<string>(term->arity());
                    shared_ptr<HtnTerm> declaration = m_termFactory->CreateConstantFunctor("declare", { term->name(),  ruleArityString });
                    if(!m_state->HasFact(declaration))
                    {
                        string idError = "Rule Not Found: " + term->name() + "/" + ruleArityString;
                        auto foundError = loops.find(idError);
                        if(foundError == loops.end())
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
    
    bool CheckRuleForLoopStackCheck(shared_ptr<HtnGoalResolver> resolver, const string &ruleHead, int arity, const vector<shared_ptr<HtnTerm>> &ruleTail, vector<string> &stack, set<string> &loops, bool ignoreLoops)
    {
        string idName = ruleHead + "/" + lexical_cast<string>(arity);
        
        if(!ignoreLoops)
        {
            vector<string>::iterator found = std::find(stack.begin(), stack.end(), idName);
            if(found != stack.end())
            {
                stringstream stackString;
                stackString << "Rule Loop: ";
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
                
                // Don't bother matching any other equivalent terms because they will all be loops
                return false;
            }
        }
        
        // Otherwise DFS
        stack.push_back(idName);
        CheckRuleRecurseTail(resolver, ruleHead, arity, ruleTail, stack, loops);
        stack.pop_back();
        return true;
    }
    
    void ParseAtom(shared_ptr<Symbol> symbol)
    {
        vector<shared_ptr<HtnTerm>> emptyTail;
        m_state->AddRule(m_termFactory->CreateConstant(symbol->ToString()), emptyTail);
    }
    
    void ParseList(shared_ptr<Symbol> symbol)
    {
        vector<shared_ptr<HtnTerm>> emptyTail;
        m_state->AddRule(CreateTermFromList(m_termFactory, symbol), emptyTail);
    }

    virtual void ParseRule(shared_ptr<Symbol> symbol)
    {
        shared_ptr<Symbol> head = Compiler<PrologDocument<VariableRule>>::GetChild(symbol, 0, -1);
        
        vector<shared_ptr<HtnTerm>> list;
        for(int index = 1; index < symbol->children().size(); index++)
        {
            shared_ptr<Symbol> item = Compiler<PrologDocument<VariableRule>>::GetChild(symbol, index, -1);
            list.push_back(CreateTermFromItem(m_termFactory, item));
        }
        
        m_state->AddRule(CreateTermFromFunctor(m_termFactory, head), list);
    }
    
    void ParseTopLevelFunctor(shared_ptr<Symbol> symbol)
    {
        // Some top level functors could be reserved words
        string name = Compiler<PrologDocument<VariableRule>>::GetChild(symbol, 0, -1)->ToString();
        
        shared_ptr<HtnTerm>term = CreateTermFromFunctor(m_termFactory, symbol);
        if(term->name() == "goals")
        {
            m_goals.insert(m_goals.end(), term->arguments().begin(), term->arguments().end());
        }
        else
        {
            // Interpret top level functors that aren't reserved words as facts
            vector<shared_ptr<HtnTerm>> emptyTail;
            m_state->AddRule(term, emptyTail);
        }
    }
    
    virtual bool ProcessAst(shared_ptr<typename Compiler<PrologDocument<VariableRule>>::CompileResultType> result)
    {
        FailFastAssert(m_termFactory != nullptr && m_state != nullptr);
        
        //    string foo = ParserDebug::PrintTree(*result);
        
        // Top level symbol is a document, we want to iterate its children
        for(auto item : (*result)[0]->children())
        {
            //        string output = ParserDebug::PrintTree(functor, 0);
            //        TraceString1("{0}", SystemTraceType::Parsing, TraceDetail::Normal, output);
            switch(item->symbolID())
            {
                case PrologSymbolID::PrologAtom:
                    ParseAtom(item);
                    break;
                case PrologSymbolID::PrologFunctor:
                    ParseTopLevelFunctor(item);
                    break;
                case PrologSymbolID::PrologList:
                    ParseList(item);
                    break;
                case PrologSymbolID::PrologRule:
                    ParseRule(item);
                    break;
                default:
                    FailFastAssert(false);
            }
        }
        
        return true;
    }
};

typedef PrologCompilerBase<PrologCapitalizedVariable> PrologStandardCompiler;
typedef PrologCompilerBase<HtnVariable> PrologCompiler;


#endif /* PrologCompiler_hpp */
