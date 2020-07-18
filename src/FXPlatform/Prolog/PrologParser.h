//
//  PrologParser.hpp
//  GameLib
//
//  Created by Eric Zinda on 10/10/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//

#ifndef PrologParser_hpp
#define PrologParser_hpp

#include "FXPlatform/Parser/Parser.h"
using namespace FXPlat;
namespace Prolog
{
    class HtnVariable;
    template<class VariableRule = HtnVariable>
    class PrologFunctor;
    template<class VariableRule = HtnVariable>
    class PrologList;
    
    // Errors
    extern char PrologAtomError[];
    extern char PrologVariableError[];
    extern char PrologTermError[];
    extern char PrologFunctorError[];
    extern char PrologFunctorListError[];
    extern char PrologListError[];
    extern char PrologRuleError[];
    extern char PrologTermListError[];
    extern char PrologDocumentError[];
    extern char PrologCommentError[];
    extern char PrologQueryError[];
    extern char PrologTailTermError[];

	extern char BeginCommentBlock[];
    extern char CrlfString[];
	extern char CapitalChar[];
    extern char emptyList[];
	extern char EndCommentBlock[];

    class PrologSymbolID
    {
    public:
        SymbolDef(PrologAtom, CustomSymbolStart + 0);
        SymbolDef(PrologVariable, CustomSymbolStart + 1);
        SymbolDef(PrologTerm, CustomSymbolStart + 2);
        SymbolDef(PrologFunctor, CustomSymbolStart + 3);
        SymbolDef(PrologRule, CustomSymbolStart + 4);
        SymbolDef(PrologDocument, CustomSymbolStart + 5);
        SymbolDef(PrologComment, CustomSymbolStart + 6);
		SymbolDef(PrologQuery, CustomSymbolStart + 7);
        SymbolDef(PrologList, CustomSymbolStart + 8);
        SymbolDef(PrologEmptyList, CustomSymbolStart + 9);
        SymbolDef(PrologTailTerm, CustomSymbolStart + 10);

        // Must be last so that other parsers can extend
        SymbolDef(PrologMaxSymbol, CustomSymbolStart + 11);
    };

    //    a comment starts with % and can have anything after it until it hits a group of newline, carriage returns in any order and in any number
	// or it is a block comment  /* comment */
    class PrologComment : public
		OrExpression<Args
		<
			AndExpression<Args
			<
				CharacterSymbol<PercentString, FlattenType::None>,
				ZeroOrMoreExpression<CharacterSetExceptSymbol<CrlfString>>,
                OrExpression<Args
                <
    				OneOrMoreExpression<CharacterSetSymbol<CrlfString>>,
                    EofSymbol
                >>
			>>,
			AndExpression<Args
			<
				LiteralExpression<BeginCommentBlock>,
				NotLiteralExpression<EndCommentBlock>,
				LiteralExpression<EndCommentBlock>
			>>
		>, FlattenType::None, PrologSymbolID::PrologComment, PrologCommentError>
    {
    };

    // HtnWhitespace can have normal whitespace or a valid comment
    class PrologOptionalWhitespace : public
    ZeroOrMoreExpression
    <
        OrExpression<Args
        <
            WhitespaceSymbol<>,
            PrologComment
        >>, FlattenType::Delete
    >
    {
    };

    // An atom is a general-purpose name with no inherent meaning. It is composed of a sequence of characters that is parsed by the Prolog reader as a single unit. Atoms are usually bare words in Prolog code,
    // written with no special syntax. However, atoms containing spaces or certain other special characters must be surrounded by single quotes. Atoms beginning with a capital letter must also be quoted, to
    // distinguish them from variables. The empty list, written [], is also an atom. Other examples of atoms include x, blue, 'Taco', and 'some atom'.
    // atom ::= (MathSymbol)+ OR Integer OR Float OR ((CharSymbol | UnderscoreSymbol | Hyphen) (CharSymbol | NumberSymbol | UnderscoreSymbol)*)
    class PrologAtom : public
        OrExpression<Args
        <
            Float<FlattenType::Flatten>,
            Integer<FlattenType::Flatten>,
            OneOrMoreExpression
            <
                MathSymbol
            >,
			CharacterSymbol<ExclamationPointString, FlattenType::None>,
            AndExpression<Args
            <
                CharacterSymbol<DoubleQuoteString, FlattenType::None>,
                ZeroOrMoreExpression
                <
                    CharacterSetExceptSymbol<DoubleQuoteString>
                >,
                CharacterSymbol<DoubleQuoteString, FlattenType::None>
            >>,
            AndExpression<Args
            <
				CharacterSymbol<SingleQuoteString>,
				ZeroOrMoreExpression
				<
					CharacterSetExceptSymbol<SingleQuoteString>
				>,
				CharacterSymbol<SingleQuoteString>
			>>,
            AndExpression<Args
            <
                OrExpression<Args<
                    CharSymbol,
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
        >, FlattenType::None, PrologSymbolID::PrologAtom, PrologAtomError>
    {
    };
    
    // Use this as the argument to all the rules that take "VariableRule"
    // if you want to use the normal Prolog rule where capitalized things are variables
	class PrologCapitalizedVariable : public
        AndExpression<Args
        <
			CharacterSetSymbol<CapitalChar>,
            ZeroOrMoreExpression
            <
                OrExpression<Args
                <
                    CharOrNumberSymbol,
                    CharacterSymbol<Underscore, FlattenType::None>,
                    CharacterSymbol<HyphenString, FlattenType::None>
                >>
            >
        >, FlattenType::None, PrologSymbolID::PrologAtom, PrologAtomError>
    {
    };

    // Use this as the argument to all the rules that take "VariableRule"
    // if you want to use the "HTN" rule where you put a ? in front of things that
    // are variables and capitalization doesn't mean anything
	class HtnVariable : public
        AndExpression<Args
        <
            CharacterSymbol<QuestionMarkString, FlattenType::Delete>,
            PrologAtom
        >>
    {
    };
    
    // Variables start with a (capital letter or ? depending on rule choice) or an Underscore
    // See comments on HtnVariable and PrologCapitalizedVariable for what
    // to pass to VariableRule
    template<class VariableRule = HtnVariable>
    class PrologVariable : public
		OrExpression<Args<
            VariableRule,
		    AndExpression<Args
            <
                OrExpression<Args<
                    CharacterSymbol<Underscore, FlattenType::None>
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
        >, FlattenType::None, PrologSymbolID::PrologVariable, PrologVariableError>
    {
    };

    // A term is a variable or functor (which could have no arguments and thus be an atom) or a list
    template<class VariableRule = HtnVariable>
    class PrologTerm : public
    OrExpression<Args
    <
        PrologVariable<VariableRule>,
        PrologFunctor<VariableRule>,
        PrologList<VariableRule>
    >, FlattenType::Flatten, 0, PrologTermError>
    {
    };

    template<class VariableRule = HtnVariable>
	class PrologFunctorList : public
    AndExpression<Args
    <
        PrologFunctor<VariableRule>,
        PrologOptionalWhitespace,
        ZeroOrMoreExpression
        <
            AndExpression<Args
            <
                CharacterSymbol<CommaString>,
                PrologOptionalWhitespace,
				PrologFunctor<VariableRule>,
                PrologOptionalWhitespace
            >>
        >
    >, FlattenType::Flatten, SymbolID::andExpression, PrologFunctorListError>
    {
    };
    
    template<class VariableRule = HtnVariable>
    class PrologTermList : public
    AndExpression<Args
    <
        PrologTerm<VariableRule>,
        PrologOptionalWhitespace,
        // zero or more terms with commas
        ZeroOrMoreExpression
        <
            AndExpression<Args
            <
                CharacterSymbol<CommaString>,
                PrologOptionalWhitespace,
                PrologTerm<VariableRule>,
                PrologOptionalWhitespace
            >>
        >,
        // Could be followed by | and any single term
        OptionalExpression
        <
            AndExpression<Args
            <
                CharacterSymbol<PipeString>,
                PrologOptionalWhitespace,
                PrologTerm<VariableRule>,
                PrologOptionalWhitespace
            >, FlattenType::None, PrologSymbolID::PrologTailTerm, PrologTermListError>
        >
    >, FlattenType::Flatten, SymbolID::andExpression, PrologTermListError>
    {
    };
    
    // A list is either the empty list "[]" or
    // [term, list]
    template<class VariableRule>
    class PrologList : public
    OrExpression<Args
    <
        LiteralExpression<emptyList, FlattenType::None, PrologSymbolID::PrologEmptyList>,
        AndExpression<Args
        <
            CharacterSymbol<LeftBracketString>,
            PrologOptionalWhitespace,
            PrologTermList<VariableRule>,
            CharacterSymbol<RightBracketString>
        >>
    >, FlattenType::None, PrologSymbolID::PrologList, PrologListError>
    {
    };

    // A compound term is composed of an atom called a "functor" and a number of "arguments", which are again terms. Compound terms are ordinarily written as a functor
    // followed by a comma-separated list of argument terms, which is contained in parentheses. The number of arguments is called the term's arity. An atom can be regarded
    // as a compound term with arity zero.
    template<class VariableRule>
    class PrologFunctor : public
    AndExpression<Args
    <
        OrExpression<Args<
            AndExpression<Args
            <
                NotPeekExpression<VariableRule>,
                PrologAtom
            >>
        >>,
        OptionalExpression
        <
            AndExpression<Args
            <
                CharacterSymbol<LeftParenthesisString>,
                PrologOptionalWhitespace,
                OptionalExpression
                <
                    PrologTermList<VariableRule>
                >,
                PrologOptionalWhitespace,
                CharacterSymbol<RightParenthesisString>
            >>
        >
    >, FlattenType::None, PrologSymbolID::PrologFunctor, PrologFunctorError>
    {
    };

    template<class VariableRule = HtnVariable>
    class PrologRule : public
    AndExpression<Args
    <
        PrologFunctor<VariableRule>,
        PrologOptionalWhitespace,
        CharacterSymbol<ColonString>,
        CharacterSymbol<DashString>,
        PrologOptionalWhitespace,
        OptionalExpression
        <
            PrologTermList<VariableRule>
        >
    >, FlattenType::None, PrologSymbolID::PrologRule, PrologRuleError>
    {
    };
    
    template<class VariableRule = HtnVariable>
	class PrologQuery : public
    AndExpression<Args<
		PrologOptionalWhitespace,
		PrologFunctorList<VariableRule>,
		PrologOptionalWhitespace,
		CharacterSymbol<PeriodString>,
		PrologOptionalWhitespace,
        EofSymbol
    >, FlattenType::None, PrologSymbolID::PrologQuery, PrologQueryError>
    {
    };

    // This parser defaults to requiring a ? in front of any variable
    // and allowing capitalization anywhere.
    // If you want normal Prolog rules, set to PrologCapitalizedVariable for
    // the VariableRule argument
    template<class VariableRule = HtnVariable>
    class PrologDocument : public
    AndExpression<Args<
        OneOrMoreExpression
        <
            AndExpression<Args<
                PrologOptionalWhitespace,
                OrExpression<Args<
                    PrologRule<VariableRule>,
                    PrologFunctor<VariableRule>,
                    PrologList<VariableRule>
                >>,
                PrologOptionalWhitespace,
                CharacterSymbol<PeriodString>,
                PrologOptionalWhitespace
            >>
        >,
        PrologOptionalWhitespace,
        EofSymbol
    >, FlattenType::None, PrologSymbolID::PrologDocument, PrologDocumentError>
    {
    };
}
#endif /* PrologParser_hpp */
