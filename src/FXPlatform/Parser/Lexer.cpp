#include "Lexer.h"
#include "Parser.h"
using namespace FXPlat;
using namespace std;

void Lexer::AbortTransaction(long position)
{
    m_transactionDepth--;
    FailFastAssert(m_transactionDepth >= 0);
    Seek(position);
}

long Lexer::BeginTransaction()
{
    m_transactionDepth++;
    long position = Position();
    return position;
}

void Lexer::CommitTransaction(long position)
{
    // Can't commit something that we haven't read yet
    FailFastAssert(position <= m_consumedCharacters);

    // The Reader must detect if we are aborting out of order, but we can do a simple check here
    m_transactionDepth--;
    FailFastAssert(m_transactionDepth >= 0);
}

bool Lexer::Eof() 
{ 
    return Peek()->symbolID() == SymbolID::eof; 
}

// Used to report where an error occurred
void Lexer::GetLineAndColumn(int charPosition, shared_ptr<istream> stream, int &lineCount, int &columnCount, string *lineString)
{
    if(!stream->good())
    {
        stream->clear();
    }
    stream->seekg(0);

    char character;
    lineCount = 1;
    columnCount = 0;
    int lastLineStart = 0;
    int charCount = 0;
    bool foundCR = false;
    string result;
    
    string lastLine;
    string nextLine;
    while(charPosition > charCount && stream->good())
    {
        character = stream->get();
        result.push_back(character);
        charCount++;

        if(character == '\r')
        {
            foundCR = true;
        }
        else if(character == '\n' && foundCR)
        {
            lastLine = result;
            
            lineCount++;
            foundCR = false;
            lastLineStart = charCount;
            if(charPosition != charCount)
            {
                // Don't clear if the last line is the one we want
                result.clear();
            }
        }
        else if(foundCR)
        {
            foundCR = false;
        }
    }

    columnCount = charPosition - lastLineStart + 1;
    if(lineString != nullptr)
    {
        *lineString = "Prev: " + lastLine + "\r\n Line: " + result;
    }
}

void Lexer::Open(shared_ptr<istream> stream)
{
    m_consumedCharacters = 0;
    m_deepestFailure = -1;
    m_errorMessage = "";
    m_stream = stream;
    m_transactionDepth = 0;
}

shared_ptr<Symbol> Lexer::Peek()
{
    char character = m_stream->peek();
    if(!(character == EOF))
    {
        TraceString3("{0}Lexer::Peek: '{1}', Consumed: {2}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
            string((size_t) (TransactionDepth() * 3), ' '), character, m_consumedCharacters);
        return shared_ptr<LexerSymbol>(new LexerSymbol(character));
    }
    else
    {
        TraceString2("{0}Lexer::Peek: '<EOF>', Consumed: {1}", SystemTraceType::Parsing, TraceDetail::Diagnostic,
             string((size_t) (TransactionDepth() * 3), ' '), m_consumedCharacters);
        return EofSymbol::defaultValue;
    }
}

shared_ptr<Symbol> Lexer::Read()
{
    char character;
    if(m_stream->read(&character, 1))
    {
        m_consumedCharacters++;
        TraceString3("{0}Lexer::Read: '{1}', Consumed: {2}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
            string((size_t) (TransactionDepth() * 3), ' '), character, m_consumedCharacters);
        return shared_ptr<LexerSymbol>(new LexerSymbol(character));
    }
    else
    {
        TraceString2("{0}Lexer::Read: '<EOF>', Consumed: {1}", SystemTraceType::Parsing, TraceDetail::Diagnostic,
             string((size_t) (TransactionDepth() * 3), ' '), m_consumedCharacters);
        return EofSymbol::defaultValue;
    }
}

void Lexer::ReportFailure(const string &errorMessage)
{
    // Assume the last one to hit this length is going to be the best message because
    // often a token will fail first (like whitespace) and then the good error message token will fail
    if(m_consumedCharacters >= m_deepestFailure)
    {
        TraceString2("{0}Lexer::ReportFailure New deepest failure at char {1}", SystemTraceType::Parsing, TraceDetail::Diagnostic, 
            string((size_t) (TransactionDepth() * 3), ' '), m_consumedCharacters);
        m_deepestFailure = m_consumedCharacters;
        m_errorMessage = errorMessage;
    }
}

void Lexer::Seek(long position) 
{
    if(!m_stream->good())
    {
        m_stream->clear();
    }

    if(position > -1)
    {
        m_consumedCharacters = position;
    }

    m_stream->seekg(position); 
    FailFastAssert(Position() == position);
}

int Lexer::TransactionDepth()
{
    return m_transactionDepth;
}

