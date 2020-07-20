#pragma once
#include <cstring>
#include "FXPlatform/FailFast.h"
#include "FXPlatform/NanoTrace.h"
#include <list>
#include "LexerReader.h"
#include <vector>
#include <algorithm>
#include <limits.h>

//
/*
 This is a PEG (Parsing Expression Grammar) parser.  

 Summary: The goal is to parse a string into a tree of Symbol objects using a tree of rules. The
 tree of rules is written by the developer to parse a particular format of document. The parser 
 visits each node of the tree and allows it to consume characters from the string as it tries to
 match the rule's pattern. If a rule doesn't match, the parser backtracks and tries other branches of the tree until it gets to the end of
 document with a successful rule (or fails).

 A rule is any object derived from Symbol that has the following method on it:
	static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
that returns a Symbol if success or null if failure.
    They read characters from the Lexer in a (potentially nested) transaction.
        If they succeed, they "commit" the transaction which means they "consume" the characters, removing them from the string so that other rules can't see them
        If they fail, the transaction rolls back which allows something else to see them
    They can call nested rules which contain their own transactions.
    Only when the outermost transaction is committed is the string consumed
	Rules form a tree, with a single rule at the root that is followed to interpret a document. 

The rules are all written using C++ Templates so that they end up forming a "Domain Specific Language"
which makes it easier to read and write the rules.  It obviously makes it harder to debug compile errors though.

Each of the rule templates have some standard arguments that can be tweaked when they are used:
    - most allow you to specify the ID of the symbol returned on success
    - most allow you to control "flattening" which controls what the tree looks like:
        - Flatten means take this symbol out of the tree and reparent its children to its parent
			Useful for things you really don't care about when you are interpreting the tree later like the < and >
			in an XML document. Useful for processing, but only care about what is in them.
        - Delete means remove this symbol and all children completely
			Whitespaces is a good example of this. 
	- most have an error string that is used if this rule happens to be the deepest rule that fails.  It is the error
		that will be returned to the user as the "parser error"
  
 */
 namespace FXPlat
{
    class EmptyClass;
    class EofSymbol;
    class Lexer;
    class LexerReader;
    class NonTerminalSymbol;
    class Symbol;

    extern char AmpersandString[];
    extern char AsterixString[];
    extern char AtString[];
    extern char Chars[];
    extern char CharsAndNumbers[];
    extern char ColonString[];
    extern char CommaString[];
    extern char DashString[];
    extern char DefaultErrorMessage[];
    extern char DoubleQuoteString[];
    extern char EqualString[];
    extern char ExclamationPointString[];
    extern char ForwardSlashString[];
    extern char HexNumbers[];
    extern char HyphenString[];
    extern char LessThanString[];
    extern char LeftBracketString[];
    extern char LeftBraceString[];
    extern char LeftParenthesisString[];
    extern char GreaterThanString[];
    extern char MathString[];
    extern char NullString[];
    extern char Numbers[];
    extern char PipeString[];
    extern char PlusString[];
    extern char PercentString[];
    extern char PeriodString[];
    extern char PoundString[];
    extern char QuestionMarkString[];
    extern char RightBracketString[];
    extern char RightBraceString[];
    extern char RightParenthesisString[];
    extern char SemicolonString[];
    extern char SingleArrowString[];
    extern char SingleQuoteString[];
    extern char Underscore[];
    extern char WhitespaceChars[];

    #define SymbolDef(name, ID) static const unsigned short name = ID;
    #define CustomSymbolStart 16000
    class SymbolID
    {
    public:
        static bool IsCharacterSymbol(shared_ptr<Symbol> symbol);
        static bool AllCharacterSymbols(vector<shared_ptr<Symbol> > symbolVector);

        SymbolDef(carriageReturn, '\r');
        SymbolDef(lineFeed, '\n');
        SymbolDef(tab, '\t');
        SymbolDef(space, ' ');
        SymbolDef(eof, 256);
        SymbolDef(whitespace, 257);
        SymbolDef(nOrMoreExpression, 259);
        SymbolDef(identifierCharacters, 260);
        SymbolDef(orExpression, 261);
        SymbolDef(oneOrMoreExpression, 262);
        SymbolDef(optionalWhitespace, 263);
        SymbolDef(andExpression, 264);
        SymbolDef(zeroOrMoreExpression, 265);
        SymbolDef(literalExpression, 266);
        SymbolDef(atLeastAndAtMostExpression, 267);
        SymbolDef(notLiteralExpression, 268);
        SymbolDef(optionalExpression, 269);
        SymbolDef(notPeekExpression, 270);
        SymbolDef(peekExpression, 271);
        SymbolDef(integerExpression, 272);
        SymbolDef(notUnmatchedBlockExpression, 273);
        SymbolDef(floatExpression, 274);
    };

    // Allow you to control "flattening" which controls what the tree looks like if the node is successful:
    //  - Flatten means take this symbol out of the tree and reparent its children to its parent (which is useful for nodes that are there for mechanics, not the meaning of the parse tree)
    //  - Delete means remove this symbol and all children completely (which you might want to do for a comment)
    //  - None means leave this node in the tree (for when the node is meaningful like a name of something)
    enum class FlattenType
    {
        None,
        Delete,
        Flatten
    };

    #define GetError(myErrorMessage, passedErrorMessage) \
        (string(myErrorMessage) == "" ? passedErrorMessage : myErrorMessage)

    #define Spaces() \
        string((size_t) (lexer->TransactionDepth() * 3), ' ')

	// This is the base class that is used by all the rules and the primary thing the rules (and thus the Parser) generate
    class Symbol : public enable_shared_from_this<Symbol>
    {
    public:
        Symbol(unsigned short symbolID) : 
            m_flattenType(FlattenType::None),
            m_symbolID(symbolID)
        {
        }

        Symbol(unsigned short symbolID, FlattenType flattenType) : 
            m_flattenType(flattenType),
            m_symbolID(symbolID)
        {
        }

        virtual ~Symbol()
        {
        }
        
        void AddSubsymbol(shared_ptr<Symbol> symbol)
        {
            FailFastAssert(symbol != nullptr);
            m_subSymbols.push_back(symbol);
        }

        void AddSubsymbol(vector<shared_ptr<Symbol>>::iterator begin, vector<shared_ptr<Symbol>>::iterator end)
        {
            for_each(begin, end, [&]
                     (shared_ptr<Symbol> &symbol)
                     {
                         FailFastAssert(symbol != nullptr);
                         this->m_subSymbols.push_back(symbol);
                     });
        }

        virtual void AddToStream(stringstream &stream) 
        {
            for_each(m_subSymbols.begin(), m_subSymbols.end(), [&stream]
                (shared_ptr<Symbol> &symbol)
                {
                    symbol->AddToStream(stream);
                });
        }
        
        const vector<shared_ptr<Symbol> > &children()
        {
            return m_subSymbols;
        }

        virtual void FlattenInto(vector<shared_ptr<Symbol> > &symbolVector)
        { 
            switch(m_flattenType)
            {
            case FlattenType::None:
                NoFlatten(symbolVector); 
                break;
            case FlattenType::Delete:
                // Don't add this symbol or any children to the tree
                break;
            case FlattenType::Flatten:
                FlattenChildren(symbolVector);
                break;
            }
        }

        void FlattenChildren(vector<shared_ptr<Symbol> > &symbolVector)
        {
            for_each(m_subSymbols.begin(), m_subSymbols.end(),
                [&] (shared_ptr<Symbol> child)
            {
                FailFastAssert(child != nullptr);
                child->FlattenInto(symbolVector);
            });
        }

        void NoFlatten(vector<shared_ptr<Symbol> > &symbolVector)
        {
            vector<shared_ptr<Symbol> > newChildren;
            FlattenChildren(newChildren);
            m_subSymbols.swap(newChildren);
            symbolVector.push_back(shared_from_this());
        }

        int HasSubsymbols() { return m_subSymbols.size() > 0; }
        int SubsymbolCount() { return (int) m_subSymbols.size(); }
        bool operator==(const Symbol &other) const { return m_symbolID == other.m_symbolID; }
        bool operator!=(const Symbol &other) const { return !(*this == other); }

        string ToString()
        {
            stringstream stream;
            AddToStream(stream);
            return stream.str();
        }

        unsigned short symbolID() { return m_symbolID; };
        FlattenType flattenType() { return m_flattenType; };

    protected:
        FlattenType m_flattenType;
		// SymbolID needs to be a number that is unique for the entire parser
		// so that you know what got generated later
        unsigned short m_symbolID;
        vector<shared_ptr<Symbol> > m_subSymbols;
    };

	// Symbols returned by the Lexer are simple characters (except for EOF) and will
	// be of this class
    class LexerSymbol : public Symbol
    {
    public:
        LexerSymbol(char character) : Symbol(character, FlattenType::None)
        {
        }

        virtual void AddToStream(stringstream &stream) 
        {
            stream << (char) m_symbolID;
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            lexer->ReportFailure(GetError(DefaultErrorMessage, errorMessage));
            return nullptr;
        }
    };

	// Matches a single character. The SymbolID used is the ascii value of the character.
    template<char *character, FlattenType flatten = FlattenType::Delete, char *staticErrorMessage = DefaultErrorMessage>
    class CharacterSymbol : public Symbol
    {
    public:
        typedef CharacterSymbol<character, flatten, staticErrorMessage> ThisType;
        CharacterSymbol() : Symbol(*character, flatten)
        {
        }

        CharacterSymbol(char c) : Symbol(c, flatten)
        {
        }

        virtual void AddToStream(stringstream &stream) 
        {
            stream << (char) m_symbolID;
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            reader.Begin();
            shared_ptr<Symbol> streamSymbol = reader.Read();
         
            if(SymbolID::IsCharacterSymbol(streamSymbol) && streamSymbol->symbolID() == *character)
            {
                TraceString3("{0}{1}(Succ) - CharacterSymbol::Parse found '{2}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), streamSymbol->ToString());
                reader.Commit();
                return shared_ptr<ThisType>(new ThisType());
            }
            else
            {
                TraceString4("{0}{1}(FAIL) - CharacterSymbol::Parse found '{2}', wanted '{3}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), streamSymbol->ToString(), character);
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
        }
    };

	// Matches the end of a document. The SymbolID used is SymbolID::eof.
    class EofSymbol : public Symbol
    {
    public:
        EofSymbol() : Symbol(SymbolID::eof, FlattenType::Delete)
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            reader.Begin();
            shared_ptr<Symbol> streamSymbol = reader.Read();
            if(streamSymbol->symbolID() == SymbolID::eof)
            {
                TraceString2("{0}{1}(Succ) - EofSymbol::Parse", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), errorMessage);
                reader.Commit();
                return streamSymbol;
            }
            else
            {
                TraceString3("{0}{1}(FAIL) - EofSymbol::Parse, found {2}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), errorMessage, streamSymbol->ToString());
                lexer->ReportFailure(errorMessage);
                return nullptr;
            }
        }

        virtual void AddToStream(stringstream &stream) 
        {
            stream << "<EOF>";
        }

        static shared_ptr<Symbol> defaultValue;
    };

	// Matches any single character *except* the disallowedCharacters. The SymbolID used is the ascii value of the character.
    template<char *disallowedCharacters, FlattenType flatten = FlattenType::None, char *staticErrorMessage = DefaultErrorMessage>
    class CharacterSetExceptSymbol : public CharacterSymbol<NullString, flatten, staticErrorMessage>
    {
    public:
        typedef CharacterSetExceptSymbol<disallowedCharacters, flatten, staticErrorMessage> ThisType;
        CharacterSetExceptSymbol(char character) : CharacterSymbol<NullString, flatten, staticErrorMessage>(character)
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            reader.Begin();
            shared_ptr<Symbol> streamSymbol = reader.Read();
        
            if(SymbolID::IsCharacterSymbol(streamSymbol) && strchr(disallowedCharacters, streamSymbol->symbolID()) == nullptr)
            {
                TraceString4("{0}{1}(Succ) - CharacterSetExceptSymbol::Parse found '{2}' expected not any of '{3}'", SystemTraceType::Parsing, TraceDetail::Diagnostic,
                    Spaces(), GetError(staticErrorMessage, errorMessage), streamSymbol->ToString(), disallowedCharacters);
                reader.Commit();
                return shared_ptr<ThisType>(new ThisType((char) streamSymbol->symbolID()));
            }
            else
            {
                // not found
                TraceString4("{0}{1}(FAIL) - CharacterSetExceptSymbol::Parse found '{2}' expected not any of'{3}'", SystemTraceType::Parsing, TraceDetail::Diagnostic,
                    Spaces(), GetError(staticErrorMessage, errorMessage), streamSymbol->ToString(), disallowedCharacters);
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
        }
    };

	// Matches any single character in the set specified by allowedCharacters. The SymbolID used is the ascii value of the character.
    template<char *allowedCharacters, FlattenType flatten = FlattenType::None, char *staticErrorMessage = DefaultErrorMessage>
    class CharacterSetSymbol : public CharacterSymbol<NullString, flatten, staticErrorMessage>
    {
    public:
        typedef CharacterSetSymbol<allowedCharacters, flatten, staticErrorMessage> ThisType;
        CharacterSetSymbol(char character) : CharacterSymbol<NullString, flatten, staticErrorMessage>(character)
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            reader.Begin();
            shared_ptr<Symbol> streamSymbol = reader.Read();
        
            if((streamSymbol->symbolID() == SymbolID::eof) || strchr(allowedCharacters, streamSymbol->symbolID()) == nullptr)
            {
                // not found
                TraceString4("{0}{1}(FAIL) - CharacterSetSymbol::Parse found '{2}', wanted one of '{3}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), streamSymbol->ToString(), allowedCharacters);
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
            else
            {
                TraceString4("{0}{1}(Succ) - CharacterSetSymbol::Parse found '{2}', wanted one of '{3}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), streamSymbol->ToString(), allowedCharacters);
                reader.Commit();
                return shared_ptr<ThisType>(new ThisType((char) streamSymbol->symbolID()));
            }
        }
    };

	// Some typical character sets that are used often
    typedef CharacterSetSymbol<Chars> CharSymbol;
    typedef CharacterSetSymbol<MathString> MathSymbol;
    typedef CharacterSetSymbol<CharsAndNumbers> CharOrNumberSymbol;
    typedef CharacterSetSymbol<Numbers> NumberSymbol;
    typedef CharacterSetSymbol<HexNumbers> HexNumberSymbol;
	typedef CharacterSetSymbol<WhitespaceChars> WhitespaceCharSymbol;

	// Matches a specific string of characters.
    template<char *literalString, FlattenType flatten = FlattenType::None, unsigned short ID = SymbolID::literalExpression, char *staticErrorMessage = DefaultErrorMessage>
    class LiteralExpression : public Symbol
    {
    public:
        typedef LiteralExpression<literalString, flatten, ID, staticErrorMessage> ThisType;
        LiteralExpression() : Symbol(ID, flatten)
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            reader.Begin();
            shared_ptr<Symbol> streamSymbol;
            shared_ptr<ThisType> literalSymbol = shared_ptr<ThisType>(new ThisType());

            int position = 0;
            while(literalString[position] != '\0' && reader.Read(streamSymbol) != SymbolID::eof)
            {
                if(SymbolID::IsCharacterSymbol(streamSymbol) && streamSymbol->symbolID() == literalString[position])
                {
                    literalSymbol->AddSubsymbol(streamSymbol);
                }
                else
                {
                    TraceString4("{0}{1}(FAIL) - LiteralExpression::Parse found '{2}', wanted '{3}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                        Spaces(), GetError(staticErrorMessage, errorMessage), streamSymbol->ToString(), literalString[position]);
                    lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                    return nullptr;
                }

                position++;
            }

            if(position == strlen(literalString))
            {
                TraceString3("{0}{1}(Succ) - LiteralExpression::Parse found '{2}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), literalString);
                reader.Commit();
                return literalSymbol;
            }
            else
            {
                TraceString3("{0}{1}(FAIL) - LiteralExpression::Parse wanted '{2}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), literalString);
                return nullptr;
            }
        }
    };

	// Used when you want to match *everything* (including delimiters and including zero characters) that appears within a block delimited by characters like (). Will
	// stop when it finds an unmatched right delimiter. Allows parsing of string like "(foo(bar))", if you want to match everything inside
	// the outer () in a single token even if it includes more of the delimiters (as long as they are matched). Example of use:
	//		CharacterSymbol<LeftParenthesisString, FlattenType::Delete, errExpectedParenthesis>,
	//		NotUnmatchedBlockExpression<LeftParenthesisString, RightParenthesisString, FlattenType::None, StoreObjectSymbolID::initialValue>,
	//		CharacterSymbol<RightParenthesisString, FlattenType::Delete, errExpectedParenthesis>,
    // Only supports single character blocks like () [] {}
    template<char *startBlockChar, char *endBlockChar, FlattenType flatten = FlattenType::None, unsigned short ID = SymbolID::notUnmatchedBlockExpression, char *staticErrorMessage = DefaultErrorMessage>
    class NotUnmatchedBlockExpression : public Symbol
    {
    public:
        typedef NotUnmatchedBlockExpression<startBlockChar, endBlockChar, flatten, ID, staticErrorMessage> ThisType;
        NotUnmatchedBlockExpression() : Symbol(ID, flatten)
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            // start with blockLevel = 1
            // Loop through grabbing characters adding 1 if we see startBlockChar, subtracting 1 if we see endBlockChar
            // When blockLevel == 0 we are done, rollback so the ending character is still in the stream and exit
            LexerReader reader(lexer);
            shared_ptr<Symbol> streamSymbol;
            shared_ptr<ThisType> symbol = shared_ptr<ThisType>(new ThisType());
            int blockLevel = 1;

            reader.Begin();
            while(reader.Peek(streamSymbol) != SymbolID::eof)
            {
                if(SymbolID::IsCharacterSymbol(streamSymbol) && streamSymbol->symbolID() == startBlockChar[0])
                {
                    blockLevel++;
                }
                else if(SymbolID::IsCharacterSymbol(streamSymbol) && streamSymbol->symbolID() == endBlockChar[0])
                {
                    blockLevel--;
                    if(blockLevel == 0)
                    {
                        // Success! Don't consume the final symbol and return
                        TraceString3("{0}{1}(Succ) - NotUnmatchedBlockExpression::Parse found '{2}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                            Spaces(), GetError(staticErrorMessage, errorMessage), endBlockChar);
                        reader.Commit();
                        return symbol;
                    }
                }

                reader.Read();
                symbol->AddSubsymbol(streamSymbol);
            }

            // We're at EOF.  If all the blocks have been closed but one, this is a success
            if(blockLevel == 1)
            {
                TraceString3("{0}{1}(Succ) - NotUnmatchedBlockExpression::Parse found '<EOF>'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), endBlockChar);
                reader.Commit();
                return symbol;
            }
            else
            {
                TraceString5("{0}{1}(FAIL) - NotUnmatchedBlockExpression::Parse still {4} deep within nested '{2}{3}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), startBlockChar, endBlockChar, blockLevel);
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
        }
    };

    // Matches any sequence of characters (including zero) except the specified literal
    template<char *literalString, FlattenType flatten = FlattenType::None, unsigned short ID = SymbolID::notLiteralExpression, char *staticErrorMessage = DefaultErrorMessage>
    class NotLiteralExpression : public Symbol
    {
    public:
        typedef NotLiteralExpression<literalString, flatten, ID, staticErrorMessage> ThisType;
        NotLiteralExpression() : Symbol(ID, flatten)
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            shared_ptr<Symbol> streamSymbol;
            shared_ptr<ThisType> symbol = shared_ptr<ThisType>(new ThisType());
            vector<shared_ptr<Symbol>> partialLiteral;

            // Each stream of characters is a different transaction  
            // We commit the transaction and start a new one if 
            // we see a new beginning of the literal
            reader.Begin();
            while(reader.Peek(streamSymbol) != SymbolID::eof)
            {
                // If we see the beginning of the literal, commit the characters we've read so far and
                // try to read the literal
                if(SymbolID::IsCharacterSymbol(streamSymbol) && streamSymbol->symbolID() == literalString[0])
                {
                    reader.Commit();
                    reader.Begin();
                    int position = 0;
                    vector<shared_ptr<Symbol>> partialLiteral;
                    while(literalString[position] != '\0' && reader.Peek(streamSymbol) != SymbolID::eof)
                    {
                        if(streamSymbol->symbolID() == literalString[position])
                        {
                            // Consume the symbol
                            reader.Read();
                            partialLiteral.push_back(streamSymbol);
                            position++;
                        }
                        else
                        {
                            // Not a complete literal, commit and push symbols into outer symbol
                            reader.Commit();
                            symbol->AddSubsymbol(partialLiteral.begin(), partialLiteral.end());
                            partialLiteral.clear();
                            reader.Begin();
                            break;
                        }
                    }

                    if(literalString[position] == '\0')
                    {
                        // We found the entire literal, rollback so it is still there and exit
                        TraceString3("{0}{1}(Succ) - NotLiteralExpression::Parse found '{2}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                            Spaces(), GetError(staticErrorMessage, errorMessage), literalString);
                        reader.Abort();
                        return symbol;
                    }
                    else if(streamSymbol->symbolID() == SymbolID::eof)
                    {
                        // We are at EOF
                        // There was a partial symbol at the end, push into outer sumbol
                        TraceString2("{0}{1}(Succ) - NotLiteralExpression::Parse found <EOF>", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                            Spaces(), GetError(staticErrorMessage, errorMessage));
                        reader.Commit();
                        symbol->AddSubsymbol(partialLiteral.begin(), partialLiteral.end());
                        partialLiteral.clear();
                        return symbol;
                    }
                }
                else
                {
                    // Not part of our literal, consume and continue
                    reader.Read();
                    symbol->AddSubsymbol(streamSymbol);
                }
            }

            TraceString2("{0}{1}(Succ) - NotLiteralExpression::Parse found <EOF>", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                Spaces(), GetError(staticErrorMessage, errorMessage));
            reader.Commit();
            return symbol;
        }
    };

	// Matches the SymbolType rule at least N and at most M times.
    // This is not designed to be used directly, use the wrapper classes instead:
	// NOrMoreExpression, OneOrMoreExpression, ZeroOrMoreExpression
    template<class SymbolType, int AtLeast, int AtMost, FlattenType flatten = FlattenType::Flatten, unsigned short ID = SymbolID::atLeastAndAtMostExpression, char *staticErrorMessage = DefaultErrorMessage>
    class AtLeastAndAtMostExpression : public Symbol
    {
    public:
        typedef AtLeastAndAtMostExpression<SymbolType, AtLeast, AtMost, flatten, ID, staticErrorMessage> ThisType;

        AtLeastAndAtMostExpression() : Symbol(ID, flatten) 
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            shared_ptr<ThisType> expression = shared_ptr<ThisType>(new ThisType());
            shared_ptr<Symbol> streamSymbol;

            reader.Begin();
            shared_ptr<Symbol> newSymbol;
            do
            {
                newSymbol = SymbolType::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
                if(newSymbol != nullptr)
                {
                    expression->AddSubsymbol(newSymbol);
                }

                if(expression->SubsymbolCount() > AtMost)
                {
                    TraceString5("{0}{1}(FAIL) - {2}to{3}Expression::Parse count= {4}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                        Spaces(), GetError(staticErrorMessage, errorMessage), AtLeast, AtMost, expression->SubsymbolCount());
                    lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                    return nullptr;
                }
            } while(newSymbol != nullptr);

            if(expression->SubsymbolCount() >= AtLeast)
            {
                TraceString5("{0}{1}(Succ) - {2}to{3}Expression::Parse count= {4}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), AtLeast, AtMost, expression->SubsymbolCount());
                reader.Commit();
                return expression;
            }
            else
            {
                TraceString5("{0}{1}(FAIL) - {2}to{3}Expression::Parse count= {4}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), AtLeast, AtMost, expression->SubsymbolCount());
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
        }
    };

	// Matches the SymbolType rule at least N times.
	template<class SymbolType, int N, FlattenType flatten = FlattenType::Flatten, unsigned short ID = SymbolID::nOrMoreExpression, char *staticErrorMessage = DefaultErrorMessage>
    class NOrMoreExpression : public AtLeastAndAtMostExpression<SymbolType, N, INT_MAX, flatten, ID, staticErrorMessage>
    {
    public:
        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            return AtLeastAndAtMostExpression<SymbolType, N, INT_MAX, flatten, ID, staticErrorMessage>::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
        }
    };

	// Matches the SymbolType rule at least 1 times.
    template<class SymbolType, FlattenType flatten = FlattenType::Flatten, unsigned short ID = SymbolID::oneOrMoreExpression, char *staticErrorMessage = DefaultErrorMessage>
    class OneOrMoreExpression : public AtLeastAndAtMostExpression<SymbolType, 1, INT_MAX, flatten, ID, staticErrorMessage>
    {
    public:
        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            return AtLeastAndAtMostExpression<SymbolType, 1, INT_MAX, flatten, ID, staticErrorMessage>::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
        }
    };

	// Matches the SymbolType rule zero or more times.
    template<class SymbolType, FlattenType flatten = FlattenType::Flatten, unsigned short ID = SymbolID::zeroOrMoreExpression, char *staticErrorMessage = DefaultErrorMessage>
    class ZeroOrMoreExpression : public AtLeastAndAtMostExpression<SymbolType, 0, INT_MAX, flatten, ID, staticErrorMessage>
    {
    public:
        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            return AtLeastAndAtMostExpression<SymbolType, 0, INT_MAX, flatten, ID, staticErrorMessage>::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
        }
    };

	// Used when two things need to parse to the same symbol:
    // If the SymbolType node matches, throws it away and replaces with the SymbolID specified that outputs replacementString as its text
    template<class SymbolType, char *replacementString, FlattenType flatten = FlattenType::None, unsigned short ID = SymbolID::optionalExpression, char *staticErrorMessage = DefaultErrorMessage>
    class ReplaceExpression : public Symbol
    {
    public:
        typedef ReplaceExpression <SymbolType, replacementString, flatten, ID, staticErrorMessage> ThisType;

        ReplaceExpression () : Symbol(ID, flatten) 
        {
        }

        virtual void AddToStream(stringstream &stream)
        {
            stream << replacementString;
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            shared_ptr<Symbol> newSymbol = SymbolType::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
            if(newSymbol != nullptr)
            {
                TraceString2("{0}{1}(Succ) - ReplaceNodeExpression subexpression succeeded", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage));
                shared_ptr<ThisType> expression = shared_ptr<ThisType>(new ThisType());
                return expression;
            }
            else
            {
                TraceString2("{0}{1}(FAIL) - ReplaceNodeExpression::subexpression failed", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage));
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
        }
    };

	// If SymbolType succeeds, it is added as a child of this Symbol.
	// Used when a structure is expected that doesn't naturally fall out of the text
    template<class SymbolType, FlattenType flatten = FlattenType::Flatten, unsigned short ID = SymbolID::optionalExpression, char *staticErrorMessage = DefaultErrorMessage>
    class GroupExpression : public Symbol
    {
    public:
        typedef GroupExpression<SymbolType, flatten, ID, staticErrorMessage> ThisType;

        GroupExpression() : Symbol(ID, flatten) 
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            shared_ptr<Symbol> newSymbol = SymbolType::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
            if(newSymbol != nullptr)
            {
                TraceString2("{0}{1}(Succ) - GroupExpression subexpression succeeded", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage));
                shared_ptr<ThisType> expression = shared_ptr<ThisType>(new ThisType());
                expression->AddSubsymbol(newSymbol);
                return expression;
            }
            else
            {
                TraceString2("{0}{1}(FAIL) - GroupExpression::subexpression failed", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage));
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
        }
    };

	// Doesn't fail if SymbolType fails
    template<class SymbolType, FlattenType flatten = FlattenType::Flatten, unsigned short ID = SymbolID::optionalExpression, char *staticErrorMessage = DefaultErrorMessage>
    class OptionalExpression : public AtLeastAndAtMostExpression<SymbolType, 0, 1, flatten, ID, staticErrorMessage>
    {
    public:
        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            return AtLeastAndAtMostExpression<SymbolType, 0, 1, flatten, ID, staticErrorMessage>::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
        }
    };

	// Used as a bogus default argument for the Args class below
	class EmptyClass
	{
	public:
		static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string& errorMessage)
		{
			StaticFailFastAssert(false);
			return nullptr;
		}
	};

	// Args is a Template hack that allows for adding a variable number of arguments that are symbols.
	// It is used by rules like And and Or that accept many rules. They assume the class you pass in is of
	// type "Args"
    template<class Symbol1 = EmptyClass, class Symbol2 = EmptyClass, class Symbol3 = EmptyClass, class Symbol4 = EmptyClass, class Symbol5 = EmptyClass,
        class Symbol6 = EmptyClass, class Symbol7 = EmptyClass, class Symbol8 = EmptyClass, class Symbol9 = EmptyClass, class Symbol10 = EmptyClass,
        class Symbol11 = EmptyClass, class Symbol12 = EmptyClass, class Symbol13 = EmptyClass, class Symbol14 = EmptyClass, class Symbol15 = EmptyClass,
        class Symbol16 = EmptyClass, class Symbol17 = EmptyClass, class Symbol18 = EmptyClass, class Symbol19 = EmptyClass, class Symbol20 = EmptyClass>
    class Args
    {
    public:
        static int Count()
        {
            int count = 
                (!std::is_empty<Symbol1>::value ? 1 : 0) +
                (!std::is_empty<Symbol2>::value ? 1 : 0) +
                (!std::is_empty<Symbol3>::value ? 1 : 0) +
                (!std::is_empty<Symbol4>::value ? 1 : 0) +
                (!std::is_empty<Symbol5>::value ? 1 : 0) +
                (!std::is_empty<Symbol6>::value ? 1 : 0) +
                (!std::is_empty<Symbol7>::value ? 1 : 0) +
                (!std::is_empty<Symbol8>::value ? 1 : 0) +
                (!std::is_empty<Symbol9>::value ? 1 : 0) +
                (!std::is_empty<Symbol10>::value ? 1 : 0) +
                (!std::is_empty<Symbol11>::value ? 1 : 0) +
                (!std::is_empty<Symbol12>::value ? 1 : 0) +
                (!std::is_empty<Symbol13>::value ? 1 : 0) +
                (!std::is_empty<Symbol14>::value ? 1 : 0) +
                (!std::is_empty<Symbol15>::value ? 1 : 0) +
                (!std::is_empty<Symbol16>::value ? 1 : 0) +
                (!std::is_empty<Symbol17>::value ? 1 : 0) +
                (!std::is_empty<Symbol18>::value ? 1 : 0) +
                (!std::is_empty<Symbol19>::value ? 1 : 0) +
                (!std::is_empty<Symbol20>::value ? 1 : 0);

            return count;
        }

        static shared_ptr<Symbol> TryParse(int symbolIndex, shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            switch(symbolIndex)
            {
            case 0:
                StaticFailFastAssert(!std::is_empty<Symbol1>::value);
                return Symbol1::TryParse(lexer, errorMessage);
            case 1:
                StaticFailFastAssert(!std::is_empty<Symbol2>::value);
                return Symbol2::TryParse(lexer, errorMessage);
            case 2:
                StaticFailFastAssert(!std::is_empty<Symbol3>::value);
                return Symbol3::TryParse(lexer, errorMessage);
            case 3:
                StaticFailFastAssert(!std::is_empty<Symbol4>::value);
                return Symbol4::TryParse(lexer, errorMessage);
            case 4:
                StaticFailFastAssert(!std::is_empty<Symbol5>::value);
                return Symbol5::TryParse(lexer, errorMessage);
            case 5:
                StaticFailFastAssert(!std::is_empty<Symbol6>::value);
                return Symbol6::TryParse(lexer, errorMessage);
            case 6:
                StaticFailFastAssert(!std::is_empty<Symbol7>::value);
                return Symbol7::TryParse(lexer, errorMessage);
            case 7:
                StaticFailFastAssert(!std::is_empty<Symbol8>::value);
                return Symbol8::TryParse(lexer, errorMessage);
            case 8:
                StaticFailFastAssert(!std::is_empty<Symbol9>::value);
                return Symbol9::TryParse(lexer, errorMessage);
            case 9:
                StaticFailFastAssert(!std::is_empty<Symbol10>::value);
                return Symbol10::TryParse(lexer, errorMessage);
            case 10:
                StaticFailFastAssert(!std::is_empty<Symbol11>::value);
                return Symbol11::TryParse(lexer, errorMessage);
            case 11:
                StaticFailFastAssert(!std::is_empty<Symbol12>::value);
                return Symbol12::TryParse(lexer, errorMessage);
            case 12:
                StaticFailFastAssert(!std::is_empty<Symbol13>::value);
                return Symbol13::TryParse(lexer, errorMessage);
            case 13:
                StaticFailFastAssert(!std::is_empty<Symbol14>::value);
                return Symbol14::TryParse(lexer, errorMessage);
            case 14:
                StaticFailFastAssert(!std::is_empty<Symbol15>::value);
                return Symbol15::TryParse(lexer, errorMessage);
            case 15:
                StaticFailFastAssert(!std::is_empty<Symbol16>::value);
                return Symbol16::TryParse(lexer, errorMessage);
            case 16:
                StaticFailFastAssert(!std::is_empty<Symbol17>::value);
                return Symbol17::TryParse(lexer, errorMessage);
            case 17:
                StaticFailFastAssert(!std::is_empty<Symbol18>::value);
                return Symbol18::TryParse(lexer, errorMessage);
            case 18:
                StaticFailFastAssert(!std::is_empty<Symbol19>::value);
                return Symbol19::TryParse(lexer, errorMessage);
            case 19:
                StaticFailFastAssert(!std::is_empty<Symbol20>::value);
                return Symbol20::TryParse(lexer, errorMessage);
            default:
                StaticFailFastAssert(false);
                return nullptr;
            }
        }
    };

	// Requires that first argument be an Args class, which allows multiple rules to be children.
	// Matches (and stops processing) as soon as one of them succeeds.  Fails otherwise
    template<class Args, FlattenType flatten = FlattenType::Flatten, unsigned short ID = SymbolID::orExpression, char *staticErrorMessage = DefaultErrorMessage>
    class OrExpression : public Symbol
    {
    public:
        typedef OrExpression<Args, flatten, ID, staticErrorMessage> ThisType;
        OrExpression() : Symbol(ID, flatten) 
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            shared_ptr<ThisType> expression = shared_ptr<ThisType>(new ThisType());

            // Loop through the symbols and succeed the first time one works
            reader.Begin();
            int argsCount = Args::Count();
            for(int symbolIndex = 0; symbolIndex < argsCount; ++symbolIndex)
            {
                shared_ptr<Symbol> streamSymbol = Args::TryParse(symbolIndex, lexer, GetError(staticErrorMessage, errorMessage));
                if(streamSymbol != nullptr)
                {
                    TraceString4("{0}{1}(Succ) - OrExpression::Parse symbol #{3} found '{2}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                        Spaces(), GetError(staticErrorMessage, errorMessage), streamSymbol->ToString(), symbolIndex);
                    expression->AddSubsymbol(streamSymbol);
                    reader.Commit();
                    return expression;
                }
            }

            TraceString2("{0}{1}(FAIL) - OrExpression::Parse", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                Spaces(), GetError(staticErrorMessage, errorMessage));
            lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
            return nullptr;
        }
    };

	// Requires that first argument be an Args class, which allows multiple rules to be children.
	// Fails (and stops processing) as soon as one of the symbols in Args fails.  Succeeds otherwise
    template<class Args, FlattenType flatten = FlattenType::Flatten, unsigned short ID = SymbolID::andExpression, char *staticErrorMessage = DefaultErrorMessage>
    class AndExpression : public Symbol
    {
    public:
        typedef AndExpression<Args, flatten, ID, staticErrorMessage> ThisType;

        AndExpression() : Symbol(ID, flatten) 
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            shared_ptr<ThisType> expression = shared_ptr<ThisType>(new ThisType());

            reader.Begin();
            int argsCount = Args::Count();
            for(int symbolIndex = 0; symbolIndex < argsCount; ++symbolIndex)
            {
                shared_ptr<Symbol> streamSymbol = Args::TryParse(symbolIndex, lexer, GetError(staticErrorMessage, errorMessage));
                if(streamSymbol != nullptr)
                {
                    expression->AddSubsymbol(streamSymbol);
                }
                else
                {
                    TraceString3("{0}{1}(FAIL) - AndExpression::Parse symbol #{2}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                        Spaces(), GetError(staticErrorMessage, errorMessage), symbolIndex);
                    lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                    return nullptr;
                }
            }

            TraceString3("{0}{1}(Succ) - AndExpression::Parse found {2}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                Spaces(), GetError(staticErrorMessage, errorMessage), expression->ToString());
            reader.Commit();
            return expression;
        }
    };

	// Matches any series of charactrs in WhitespaceCharSymbol
    template<FlattenType flatten = FlattenType::Delete, unsigned short ID = SymbolID::whitespace, char *staticErrorMessage = DefaultErrorMessage>
    class WhitespaceSymbol : public AtLeastAndAtMostExpression<WhitespaceCharSymbol, 1, INT_MAX, flatten, ID, staticErrorMessage>
    {
    };

	// Same as WhitespaceSymbol but doesn't fail if there aren't any
    template<FlattenType flatten = FlattenType::Delete, unsigned short ID = SymbolID::whitespace, char *staticErrorMessage = DefaultErrorMessage>
    class OptionalWhitespaceSymbol : public AtLeastAndAtMostExpression<WhitespaceCharSymbol, 0, INT_MAX, flatten, ID, staticErrorMessage>
    {
    };

	// Matches text that is *not* the symbol specified by SymbolType 
	// AND don't consume anything 
    template<class SymbolType, FlattenType flatten = FlattenType::Delete, unsigned short ID = SymbolID::notPeekExpression, char *staticErrorMessage = DefaultErrorMessage>
    class NotPeekExpression : public Symbol
    {
    public:
        typedef NotPeekExpression<SymbolType, flatten, ID, staticErrorMessage> ThisType;

        NotPeekExpression() : Symbol(ID, flatten) 
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            shared_ptr<Symbol> streamSymbol;

            reader.Begin();
            shared_ptr<Symbol> newSymbol = SymbolType::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
            if(newSymbol != nullptr)
            {
                TraceString3("{0}{1}(FAIL) - NotPeekExpression::Parse found '{2}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), newSymbol->ToString());
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
            else
            {
                // Even though this was successful, this is only peeking, so abort anything that happened but succeed
                reader.Abort();
                TraceString2("{0}{1}(Succ) - NotPeekExpression::Parse ", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage));
                return shared_ptr<ThisType>(new ThisType());
            }
        }
    };

	// Matches text that *is* the symbol specified by SymbolType 
	// BUT doesn't consume it 
    template<class SymbolType, FlattenType flatten = FlattenType::Delete, unsigned short ID = SymbolID::peekExpression, char *staticErrorMessage = DefaultErrorMessage>
    class PeekExpression : public Symbol
    {
    public:
        typedef PeekExpression<SymbolType, flatten, ID, staticErrorMessage> ThisType;

        PeekExpression() : Symbol(ID, flatten) 
        {
        }

        static shared_ptr<Symbol> TryParse(shared_ptr<Lexer> lexer, const string &errorMessage)
        {
            LexerReader reader(lexer);
            shared_ptr<Symbol> streamSymbol;

            reader.Begin();
            shared_ptr<Symbol> newSymbol = SymbolType::TryParse(lexer, GetError(staticErrorMessage, errorMessage));
            if(newSymbol != nullptr)
            {
                // Even though this was successful, this is only peeking, so abort anything that happened but succeed
                reader.Abort();
                TraceString3("{0}{1}(Succ) - PeekExpression::Parse found '{2}'", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage), newSymbol->ToString());
                return shared_ptr<ThisType>(new ThisType());
            }
            else
            {
                TraceString2("{0}{1}(FAIL) - PeekExpression::Parse", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
                    Spaces(), GetError(staticErrorMessage, errorMessage));
                lexer->ReportFailure(GetError(staticErrorMessage, errorMessage));
                return nullptr;
            }
        }
    };

    // Matches an integer, including those with a + or - in front
    template<FlattenType flatten = FlattenType::None, unsigned short ID = SymbolID::integerExpression, char *staticErrorMessage = DefaultErrorMessage>
    class Integer : public 
        AndExpression<Args
        <
            OptionalExpression
            <
                OrExpression<Args
                <
                    CharacterSymbol<PlusString>,
                    CharacterSymbol<DashString, FlattenType::None>
                >>
            >,
            OneOrMoreExpression<NumberSymbol>
        >, flatten, ID, staticErrorMessage>
    {
    };

	// Matches a float with an optional - in front
    // [-] Integer "." Integer
    template<FlattenType flatten = FlattenType::None, unsigned short ID = SymbolID::floatExpression, char *staticErrorMessage = DefaultErrorMessage>
    class Float : public
        AndExpression<Args
        <
            OptionalExpression<CharacterSymbol<DashString, FlattenType::None>, FlattenType::Flatten>,
            Integer<>,
            CharacterSymbol<PeriodString, FlattenType::None>,
            Integer<>
        >, flatten, ID, staticErrorMessage>
    {
    };
}
