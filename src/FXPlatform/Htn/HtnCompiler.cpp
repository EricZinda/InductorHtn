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
            m_domain->AddOperator(CreateTermFromFunctor(m_termFactory, head), item->arguments(), del->arguments());
            return;
        }
    }
    
    m_state->AddRule(CreateTermFromFunctor(m_termFactory, head), list);
}
