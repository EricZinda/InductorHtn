#include "Lexer.h"
#include "LexerReader.h"
#include "Parser.h"
using namespace FXPlat;

LexerReader::LexerReader(shared_ptr<Lexer> lexer) :
    m_originalPosition(-2),
    m_transactionDepth(-1)
{
    m_lexer = lexer;
};

LexerReader::~LexerReader()
{
    if(m_originalPosition != -2)
    {
        Abort();
    }
};

// Outer transactions can't commit or abort until nested ones complete because
// otherwise we will move the file pointer out from under it
// Really it is just a debugging guarantee since the calling code should never do this
void LexerReader::Abort()
{
    FailFastAssert(m_originalPosition != -2);
    FailFastAssert(m_transactionDepth == m_lexer->TransactionDepth());
    m_lexer->AbortTransaction(m_originalPosition);
    m_originalPosition = -2;
}

void LexerReader::Begin()
{
    FailFastAssert(m_originalPosition == -2);
    m_originalPosition = m_lexer->BeginTransaction();
    m_transactionDepth = m_lexer->TransactionDepth();
}

void LexerReader::Commit()
{
    FailFastAssert(m_originalPosition != -2);
    FailFastAssert(m_transactionDepth == m_lexer->TransactionDepth());
    m_lexer->CommitTransaction(m_originalPosition);
    m_originalPosition = -2;
};

shared_ptr<Symbol> LexerReader::Peek()
{
    return m_lexer->Peek();
}

unsigned short LexerReader::Peek(shared_ptr<Symbol> &symbol)
{
    symbol = m_lexer->Peek();
    return symbol->symbolID();
}

shared_ptr<Symbol> LexerReader::Read()
{
    FailFastAssert(m_originalPosition != -2);
    return m_lexer->Read();
};

unsigned short LexerReader::Read(shared_ptr<Symbol> &symbol)
{
    FailFastAssert(m_originalPosition != -2);
    symbol = m_lexer->Read();
    return symbol->symbolID();
};