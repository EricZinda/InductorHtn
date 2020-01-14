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
class HtnOperator;
class HtnMethod;
class HtnDomain;

/*  
HTN Operators and methods are just Prolog rules interpreted in a special way
so we don't have to make up a new syntax


*/
class HtnCompiler : public PrologCompiler
{
public:
    HtnCompiler() :
        PrologCompiler(nullptr, nullptr),
        m_domain(nullptr)
    {
    }
    
    HtnCompiler(HtnTermFactory *termFactory, HtnRuleSet *state, HtnDomain *domain) :
        PrologCompiler(termFactory, state),
        m_domain(domain)
    {
    }
    
    virtual void Clear();
    virtual void ClearWithNewRuleSet();
    // Find loops in the domain by recursing through all the do() subtasks
    // These loops *could* cause a stack overflow
    // Also find calls to tasks that aren't implemented
    std::set<std::string> FindLogicErrors(shared_ptr<HtnGoalResolver> resolver);
    virtual void ParseRule(shared_ptr<Symbol> symbol);
    ValueProperty(private, HtnDomain *, domain);
    
private:
    void CheckMethodCondition(shared_ptr<HtnGoalResolver> resolver, HtnMethod *method, set<string> &loops);
    void CheckMethodForLoop(HtnMethod *method, std::vector<std::string> &stack, std::set<std::string> &loops);
};

#endif /* HtnCompiler_hpp */
