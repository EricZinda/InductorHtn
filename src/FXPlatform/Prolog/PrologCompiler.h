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

    shared_ptr<vector<UnifierType>> SolveGoals(HtnGoalResolver *resolver, int memoryBudget = 1000000)
    {
        return resolver->ResolveAll(m_termFactory, m_state, m_goals, 0, memoryBudget);
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
    
    static shared_ptr<HtnTerm> CreateTermFromVariable(HtnTermFactory *factory, shared_ptr<Symbol> variable)
    {
        int level1Index = 0;
        shared_ptr<Symbol> name = Compiler<PrologDocument<VariableRule>>::GetChild(variable, level1Index++, -1);
        string nameString = name->ToString();
        return factory->CreateVariable(nameString);
    }
    
    vector<shared_ptr<HtnTerm>> &goals() { return m_goals; }

    ValueProperty(protected, shared_ptr<HtnRuleSet>, compilerOwnedRuleSet);
    ValueProperty(protected, HtnRuleSet *, state);
    ValueProperty(protected, HtnTermFactory *, termFactory);
    
protected:
    vector<shared_ptr<HtnTerm>> m_goals;
    void ParseAtom(shared_ptr<Symbol> symbol)
    {
        vector<shared_ptr<HtnTerm>> emptyTail;
        m_state->AddRule(m_termFactory->CreateConstant(symbol->ToString()), emptyTail);
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
