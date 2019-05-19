//
//  PrologCompiler.hpp
//  GameLib
//
//  Created by Eric Zinda on 10/10/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//

#ifndef PrologQueryCompiler_hpp
#define PrologQueryCompiler_hpp
#include <vector>
#include "FXPlatform/Parser/Compiler.h"
#include "HtnGoalResolver.h"
#include "PrologParser.h"
class HtnTerm;
class HtnTermFactory;
class HtnRuleSet;
using namespace Prolog;

// VariableRule determines if the compiler uses normal prolog rules for variables
// or the "Htn-style" where ? must proceed a variable name.  See PrologParser.h for
// more details
template <class VariableRule>
class PrologQueryCompilerBase : public Compiler<PrologQuery<VariableRule>>
{
public:
	PrologQueryCompilerBase(HtnTermFactory *termFactory) :
        m_termFactory(termFactory)
    {
    }
    
	void Clear()
    {
        Compiler<PrologQuery<VariableRule>>::Initialize();
        m_result.clear();
    }

    ValueProperty(private, HtnTermFactory *, termFactory);
    
private:
    virtual bool ProcessAst(shared_ptr<typename Compiler<PrologQuery<VariableRule>>::CompileResultType> result)
    {
        FailFastAssert(m_termFactory != nullptr);
        // There should only ever be a single PrologQuery item if our parser is working
        FailFastAssert(result->size() == 1 && (*result)[0]->symbolID() == PrologSymbolID::PrologQuery);
        
        // Top level symbol is a Query, we want to iterate its children which will be functors
        for(auto item : (*result)[0]->children())
        {
            switch(item->symbolID())
            {
                case PrologSymbolID::PrologFunctor:
                    m_result.push_back(PrologCompiler::CreateTermFromFunctor(m_termFactory, item));
                    break;
                default:
                    FailFastAssert(false);
            }
        }
        
        return true;
    }
    
	ValueProperty(private, std::vector<shared_ptr<HtnTerm>>, result);
};

typedef PrologQueryCompilerBase<PrologCapitalizedVariable> PrologStandardQueryCompiler;
typedef PrologQueryCompilerBase<HtnVariable> PrologQueryCompiler;
#endif /* PrologQueryCompiler_hpp */
