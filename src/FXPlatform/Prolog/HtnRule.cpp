//
//  HtnRule.cpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include "HtnRule.h"
#include "HtnRuleSet.h"
#include "HtnTerm.h"
using namespace std;

shared_ptr<HtnRule> HtnRule::MakeVariablesUnique(HtnTermFactory *factory, const string &uniquifier, std::map<std::string, std::shared_ptr<HtnTerm>> &variableMap, bool onlyDontCareVariables) const
{
	// Don't care variables can't match
	int dontCareCount = 0;
    shared_ptr<HtnTerm> newHead = this->head()->MakeVariablesUnique(factory, onlyDontCareVariables, uniquifier, &dontCareCount, variableMap);
    vector<shared_ptr<HtnTerm>> newTail;
    for(shared_ptr<HtnTerm> term : this->tail())
    {
        newTail.push_back(term->MakeVariablesUnique(factory, onlyDontCareVariables, uniquifier, &dontCareCount, variableMap));
    }
    
    shared_ptr<HtnRule> newRule = shared_ptr<HtnRule>(new HtnRule(newHead, newTail));
    return newRule;
}

string HtnRule::ToString() const
{
    stringstream stream;
    stream << head()->ToString() << " => ";
    bool hasTail = false;
    for(shared_ptr<HtnTerm> term : tail())
    {
        stream << (hasTail ? ", " : "") << term->ToString();
        hasTail = true;
    }
    
    return stream.str();
}

string HtnRule::ToStringProlog() const
{
    stringstream stream;
    stream << head()->ToString() << " :- ";
    bool hasTail = false;
    for(shared_ptr<HtnTerm> term : tail())
    {
        stream << (hasTail ? ", " : "") << term->ToString();
        hasTail = true;
    }
    
    stream << ".";
    string result = stream.str();
    return result;
}
