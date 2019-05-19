#pragma once
#include "Lexer.h"

namespace FXPlat
{
    class Symbol;

	// LexerReader is the primary interface between the parser and the Lexer
	// Because the parser is exploring a tree of rules until it gets to an endpoint, it 
	// will often need to back up and try different branches. LexerReader represents a single transaction that
	// allows the characters that a rule uses to get committed and consumed from the parser (if it succeeds) or 
	// aborted and put back into the Lexer for another rule to try (if it fails).
	//
	// It has correctness code (i.e. FailFast) that ensures that it doesn't get committed if there are children transactions still pending
    class LexerReader
    {
    public:
        LexerReader(std::shared_ptr<Lexer> lexer);
        ~LexerReader();

        void Abort();
        void Begin();
        void Commit();
        std::shared_ptr<Symbol> Peek();
        unsigned short Peek(std::shared_ptr<Symbol> &symbol);
        std::shared_ptr<Symbol> Read();
        unsigned short Read(std::shared_ptr<Symbol> &symbol);

    private:
		// This class should always be a local stack based class, (i.e. never created with new()) because otherwise it could mess
		// up the ordering of tokens when it gets destructed
		void* operator new(size_t);
		void* operator new[](size_t);

        long m_originalPosition;
        int m_transactionDepth;
        std::shared_ptr<Lexer> m_lexer;
    };
}
