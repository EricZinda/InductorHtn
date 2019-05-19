//
//  HtnLanguageParser.hpp
//  GameLib
//
//  Created by Eric Zinda on 10/4/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//

#ifndef HtnLanguageParser_hpp
#define HtnLanguageParser_hpp

#include "Parser/Parser.h"
using namespace FXPlat;
namespace Htn
{
    class HtnFunctorRule;
    
    extern char errExpectedHtnConstant[];
    extern char CrlfString[];
    extern char nilString[];
    
    class HtnSymbolID
    {
    public:
        SymbolDef(htnConstant, CustomSymbolStart + 0);
        SymbolDef(htnKeyword, CustomSymbolStart + 1);
        SymbolDef(htnVariable, CustomSymbolStart + 2);
        SymbolDef(htnTerm, CustomSymbolStart + 3);
        SymbolDef(htnPrimitiveTask, CustomSymbolStart + 4);
        SymbolDef(htnComment, CustomSymbolStart + 4);
        SymbolDef(htnFunctor, CustomSymbolStart + 5);
        SymbolDef(htnNilTerm, CustomSymbolStart + 6);
        SymbolDef(htnDocument, CustomSymbolStart + 7);
        // Must always be last so others can add new symbols
        SymbolDef(htnEndOfSymbols, CustomSymbolStart + 8);
    };

    // constant ::= (MathSymbol)+ OR Integer OR Float OR ((CharSymbol | UnderscoreSymbol | Hyphen) (CharSymbol | NumberSymbol | UnderscoreSymbol)*)
    class HtnConstantRule : public
        OrExpression<Args
        <
            Float<FlattenType::Flatten>,
            Integer<FlattenType::Flatten>,
            OneOrMoreExpression
            <
                MathSymbol
            >,
            AndExpression<Args
            <
                OrExpression<Args<
                    CharSymbol,
                    CharacterSymbol<Underscore, FlattenType::None>,
                    CharacterSymbol<HyphenString, FlattenType::None>
                >>,
                ZeroOrMoreExpression
                <
                    OrExpression<Args
                    <
                        CharOrNumberSymbol,
                        CharacterSymbol<Underscore, FlattenType::None>,
                        CharacterSymbol<HyphenString, FlattenType::None>
                    >>
                >
            >>
        >, FlattenType::None, HtnSymbolID::htnConstant, errExpectedHtnConstant>
    {
    };
    
    //    a keyword starts with : followed by a constant
    class HtnKeywordRule : public
        AndExpression<Args
        <
            CharacterSymbol<ColonString, FlattenType::Delete>,
            HtnConstantRule
        >, FlattenType::None, HtnSymbolID::htnKeyword>
    {
    };
    
    //    a variable starts with ? followed by a constant
    class HtnVariableRule : public
        AndExpression<Args
        <
            CharacterSymbol<QuestionMarkString, FlattenType::Delete>,
            HtnConstantRule
        >, FlattenType::None, HtnSymbolID::htnVariable>
    {
    };

    // a primitive task starts with ! followed by a constant
    class HtnPrimitiveTaskRule : public
        AndExpression<Args
        <
            CharacterSymbol<ExclamationPointString, FlattenType::Delete>,
            HtnConstantRule
        >, FlattenType::None, HtnSymbolID::htnPrimitiveTask>
    {
    };
        
    //    a comment starts with ; and can have anything after it until it hits a group of newline, carriage returns in any order and in any number
    class HtnCommentRule : public
        AndExpression<Args
        <
            CharacterSymbol<SemicolonString, FlattenType::None>,
            ZeroOrMoreExpression<CharacterSetExceptSymbol<CrlfString>>,
            OneOrMoreExpression<CharacterSetSymbol<CrlfString>>
        >, FlattenType::None, HtnSymbolID::htnComment>
    {
    };
    
    
    // HtnWhitespace can have normal whitespace or a valid comment
    class HtnOptionalWhitespaceRule : public
    ZeroOrMoreExpression
    <
        OrExpression<Args
        <
            WhitespaceSymbol<>,
            HtnCommentRule
        >>, FlattenType::Delete
    >
    {
    };
    
    // A term is a variable, constant or functor or nil
    class HtnTermRule : public
    OrExpression<Args
    <
        LiteralExpression<nilString, FlattenType::None, HtnSymbolID::htnNilTerm>,
        HtnPrimitiveTaskRule,
        HtnConstantRule,
        HtnVariableRule,
        HtnFunctorRule
    >, FlattenType::Flatten, HtnSymbolID::htnTerm>
    {
    };
    
    //   a Functor is a name and list of zero or more terms surrounded by (). example: (head arg1 arg2 arg3)
    //  ( optionalWhitespace name optionalWhitespace (term term)* optionalWhitespace)
    class HtnFunctorRule : public
    AndExpression<Args
    <
        CharacterSymbol<LeftParenthesisString>,
        HtnOptionalWhitespaceRule,
        OptionalExpression
        <
            OrExpression<Args
            <
                HtnConstantRule,
                HtnPrimitiveTaskRule,
                HtnKeywordRule
            >>
        >,
        HtnOptionalWhitespaceRule,
        AndExpression<Args
        <
            ZeroOrMoreExpression
            <
                AndExpression<Args
                <
                    HtnTermRule,
                    HtnOptionalWhitespaceRule
                >>
            >,
            CharacterSymbol<RightParenthesisString>
        >>
    >, FlattenType::None, HtnSymbolID::htnFunctor>
    {
    };

    // A document is simply one or more top level Functors with whitespace
    class HtnDocumentRule : public
    AndExpression<Args
    <
        HtnOptionalWhitespaceRule,
        ZeroOrMoreExpression
        <
            AndExpression<Args
            <
                HtnFunctorRule,
                HtnOptionalWhitespaceRule
            >>
        >,
        EofSymbol
    >, FlattenType::Flatten, HtnSymbolID::htnFunctor>
    {
    };
}

#endif /* HtnLanguageParser_hpp */
