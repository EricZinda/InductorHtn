#pragma once
#include <memory>
#include "FXPlatform/Utilities.h"
#include <list>

namespace FXPlat
{
    class LexerReader;
    class Symbol;

	// This is a PEG (Parsing Expression Grammar) parser
    // The Lexer's job is to allow the parser to grab a character at a time
	// from the stream that is opened via Open().  The parser then can combine
	// them into larger symbols based on the parser's rules.
	//
	// The parser actually uses LexerReader (which wraps Lexer) because LexerReader 
	// automatically handles putting tokens back into the Lexer if they aren't consumed.
	// See comments there for more details.
	//
	// Lexer also allows the parser to record rules that fail, and keeps the one that went
	// deepest into the tree. That's because the deepest failure is, surprisingly often,
	// the actual error in the document being parsed. Thus, that's the error returned.
	class Lexer
    {
    public:
        friend LexerReader;

        Lexer() {}

        long consumedCharacters() { return m_consumedCharacters; }
        int DeepestFailure() { return (int) m_deepestFailure; }
        bool Eof();
        std::string ErrorMessage() { return m_errorMessage; }
        static void GetLineAndColumn(int charPosition, std::shared_ptr<std::istream> stream, int &lineCount, int &columnCount, std::string *linestring = nullptr);
        void Open(std::shared_ptr<std::istream> stream);
        std::shared_ptr<Symbol> Peek();
        void ReportFailure(const std::string &errorMessage);
        int TransactionDepth();

        std::shared_ptr<std::istream> stream() { return m_stream; }
        
    private:
		// Used by LexerReader to create transactions when symbols are consumed
		// Aborting them puts the symbols back into the Lexer so the next rule can try
		// to consume them.
        void AbortTransaction(long position);
        long BeginTransaction();
        void CommitTransaction(long position);
        long Position() { return (long) m_stream->tellg(); }
        void Seek(long position);
        std::shared_ptr<Symbol> Read();

        long m_consumedCharacters;
        long m_deepestFailure;
        std::string m_errorMessage;
        std::shared_ptr<std::istream> m_stream;
        int m_transactionDepth;
    };
}
