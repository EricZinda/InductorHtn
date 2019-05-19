#include "Parser.h"
#include "Lexer.h"
#include "LexerReader.h"
using namespace FXPlat;

shared_ptr<Symbol> EofSymbol::defaultValue = shared_ptr<Symbol>(new EofSymbol());

namespace FXPlat
{
    char AmpersandString[] = "&";
    char AsterixString[] = "*";
    char AtString[] = "@";
    char Chars[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char CharsAndNumbers[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char ColonString[] = ":";
    char CommaString[] = ",";
    char DashString[] = "-";
    char DefaultErrorMessage[] = "";
    char DoubleQuoteString[] = "\"";
    char EqualString[] = "=";
    char ExclamationPointString[]  = "!";
    char ForwardSlashString[] = "/";
    char GreaterThanString[] = ">";
    char HexNumbers[]  = "1234567890ABCDEFabcdef";
    char HyphenString[] = "-";
    char LeftBraceString[] = "{";
    char LeftBracketString[] = "[";
    char LeftParenthesisString[] = "(";
    char LessThanString[] = "<";
    char MathString[] = "+-<>=/*\\";
    char NullString[] = "";
    char Numbers[]  = "1234567890";
    char PercentString[] = "%";
    char PeriodString[] = ".";
    char PlusString[] = "+";
    char PoundString[] = "#";
    char QuestionMarkString[] = "?";
    char RightBracketString[] = "]";
    char RightBraceString[] = "}";
    char RightParenthesisString[] = ")";
    char SemicolonString[] = ";";
    char SingleArrowString[] = "->";
    char SingleQuoteString[] = "'";
    char Underscore[] = "_";
    char WhitespaceChars[]  = "\r\n\t ";
}

bool SymbolID::IsCharacterSymbol(shared_ptr<Symbol> symbol) 
{ 
    return symbol->symbolID() <= 255; 
}

bool SymbolID::AllCharacterSymbols(vector<shared_ptr<Symbol>> symbolVector)
{
    bool allSymbols = true;
    for_each(symbolVector.begin(), symbolVector.end(), 
        [&] (shared_ptr<Symbol> child)
    {
        if(!IsCharacterSymbol(child))
        {
            allSymbols = false;
            return;
        }
    });

    return allSymbols;
}
