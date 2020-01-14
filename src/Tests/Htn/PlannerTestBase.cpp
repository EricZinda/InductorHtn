//
//  PlannerTestBase.cpp
//  TestLib
//
//  Created by Eric Zinda on 3/24/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include <stdio.h>
#include "PlannerTestBase.h"
#include "FXPlatform/Prolog/PrologParser.h"
#include "FXPlatform/Htn/HtnCompiler.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"

using namespace Prolog;

class SolutionSymbolID
{
public:
    SymbolDef(SolutionSubstitution, PrologSymbolID::PrologMaxSymbol + 0);
    SymbolDef(SolutionUnifier, PrologSymbolID::PrologMaxSymbol + 1);
};

// A Substitution is a variable = a term
    class SubstitutionRule : public
    AndExpression<Args
    <
        PrologVariable<HtnVariable>,
        PrologOptionalWhitespace,
        CharacterSymbol<EqualString>,
        PrologOptionalWhitespace,
        PrologTerm<HtnVariable>
    >, FlattenType::None, SolutionSymbolID::SolutionSubstitution>
    {
    };

    // A Unifier is ( Substition,... )
    class UnifierRule : public
    AndExpression<Args
    <
        CharacterSymbol<LeftParenthesisString>,
        PrologOptionalWhitespace,
        ZeroOrMoreExpression
        <
            AndExpression<Args
            <
                PrologOptionalWhitespace,
                AndExpression<Args
                <
                    SubstitutionRule,
                    PrologOptionalWhitespace,
                    CharacterSymbol<CommaString>,
                    PrologOptionalWhitespace
                >>
            >>
        >,
        SubstitutionRule,
        PrologOptionalWhitespace,
        CharacterSymbol<RightParenthesisString>
    >, FlattenType::None, SolutionSymbolID::SolutionUnifier>
    {
    };

    // A solution set is ( Unifier, ... )
    class SolutionSetRule : public
    AndExpression<Args
    <
        CharacterSymbol<LeftParenthesisString>,
        PrologOptionalWhitespace,
        ZeroOrMoreExpression
        <
            AndExpression<Args
            <
                AndExpression<Args
                <
                    UnifierRule,
                    CharacterSymbol<CommaString>
                >>,
                PrologOptionalWhitespace
            >>
        >,
        UnifierRule,
        PrologOptionalWhitespace,
        CharacterSymbol<RightParenthesisString>
    >, FlattenType::None, SolutionSymbolID::SolutionUnifier>
    {
    };

class SolutionSetCompiler : public Compiler<SolutionSetRule>
{
public:
    SolutionSetCompiler(HtnTermFactory *factory) :
        m_factory(factory)
    {
    }
    
    vector<UnifierType> result;
    HtnTermFactory *m_factory;
    
protected:
    virtual bool ProcessAst(shared_ptr<CompileResultType> ast)
    {
        result.clear();
        for(auto solutionSymbol : *ast)
        {
            for(auto unifierSymbol : solutionSymbol->children())
            {
                //        string output = ParserDebug::PrintTree(functor, 0);
                //        TraceString1("{0}", SystemTraceType::Parsing, TraceDetail::Normal, output);
                UnifierType unifier;
                for(auto item : unifierSymbol->children())
                {
                    shared_ptr<HtnTerm> rhs;
                    if(item->children()[1]->symbolID() == PrologSymbolID::PrologAtom)
                    {
                        rhs = m_factory->CreateConstant(item->children()[1]->ToString());
                    }
                    else
                    {
                        rhs = HtnCompiler::CreateTermFromFunctor(m_factory, item->children()[1]);
                    }
                    
                    unifier.push_back(UnifierItemType(HtnCompiler::CreateTermFromVariable(m_factory, item->children()[0]), rhs));
                }
                result.push_back(unifier);
            }
        }
        
        return true;
    }
};

vector<UnifierType> ParseUnifier(shared_ptr<HtnTermFactory> factory, const string &value)
{
    SolutionSetCompiler compiler(factory.get());
    StaticFailFastAssert(compiler.Compile(value));
    return compiler.result;
}

// ((?Attacker = Warrior2, ?Defender = Queen1, ?AttackName = Attack, ?Damage = 75), (?Attacker = Worker1, ?Defender = Warrior2, ?AttackName = Attack, ?Damage = 86), (?Attacker = Larva1, ?Defender = Warrior2, ?AttackName = SelfDestruct, ?Damage = 200), (?Attacker = Queen1, ?Defender = Warrior3, ?AttackName = Attack, ?Damage = 30))
// Solutions must be in the same order, but Substitutions within them can be in any order
string DiffSolutionInOrder(shared_ptr<HtnTermFactory> factory, const string &expectedString, shared_ptr<vector<UnifierType>> solution)
{
    if(solution == nullptr)
    {
        if(expectedString == "null")
        {
            return "";
        }
        else
        {
            return "solution was expected but was null";
        }
    }

    vector<UnifierType> expectedSolution = ParseUnifier(factory, expectedString);
    if(expectedSolution.size() != solution->size())
    {
        return string("solution sizes didn't match: Expected:" + expectedString + "    but was: " + HtnGoalResolver::ToString(solution.get()));
    }
    
    for(int index = 0; index < expectedSolution.size(); ++index)
    {
        // See if all of the unifier items in the corresponding solution match it
        // Unifier Items can be in any order
        UnifierType testUnifier = (*solution)[index];
        bool matched = true;
        for(auto item : expectedSolution[index])
        {
            auto found = std::find_if(testUnifier.begin(), testUnifier.end(), [&](UnifierItemType &current)
                                      {
                                          return item.first->TermCompare(*current.first) == 0 && item.second->TermCompare(*current.second) == 0;
                                      });
            
            if(found != testUnifier.end())
            {
                testUnifier.erase(found);
            }
            else
            {
                matched = false;
                break;
            }
        }
        
        if(!matched || testUnifier.size() > 0)
        {
            return string("Expected: ") + HtnGoalResolver::ToString(expectedSolution[index]) + string("     but was: ") + HtnGoalResolver::ToString((*solution)[index]);
        }
    }
    
    return "";
}
