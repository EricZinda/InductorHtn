#pragma once
#include "Parser.h"

namespace FXPlat
{
	// Functions that are useful for debugging parsing
	class ParserDebug
    {
    public:
		// Make sure tree has a child in position level0Index that has a certain symbolID AND that that child matches the string toString
        static bool CheckTree(vector<shared_ptr<Symbol>> &tree, int level0Index, unsigned short symbolID, const string &toString)
        {
            string foo = PrintTree(tree);

            if(tree.size() >= (size_t) level0Index + 1)
            {
                if(tree[level0Index]->symbolID() == symbolID)
                {
                    if(tree[level0Index]->ToString() == toString)
                    {
                        return true;
                    }
                }
            }

            return false;
        }

		// Same as above but walks from root to child at index level0Index and then its child at level1Index
        static bool CheckTree(vector<shared_ptr<Symbol>> &tree, int level0Index, int level1Index, unsigned short symbolID, const string &toString)
        {
            string foo = PrintTree(tree);

            if(tree.size() >= (size_t) level0Index + 1)
            {
                if(tree[level0Index]->children().size() >= (size_t) level1Index + 1)
                {
                    if(tree[level0Index]->children()[level1Index]->symbolID() == symbolID)
                    {
                        string level1Value = tree[level0Index]->children()[level1Index]->ToString();
                        if(level1Value == toString)
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        }

		// Same as above, one level deeper
        static bool CheckTree(vector<shared_ptr<Symbol>> &tree, int level0Index, int level1Index, int level2Index, unsigned short symbolID, const string &toString)
        {
            string foo = PrintTree(tree);

            if(tree.size() >= (size_t) level0Index + 1)
            {
                if(tree[level0Index]->children().size() >= (size_t) level1Index + 1)
                {
                    if(tree[level0Index]->children()[level1Index]->children().size() >= (size_t) level2Index + 1)
                    {
                        if(tree[level0Index]->children()[level1Index]->children()[level2Index]->symbolID() == symbolID)
                        {
                            if(tree[level0Index]->children()[level1Index]->children()[level2Index]->ToString() == toString)
                            {
                                return true;
                            }
                        }
                    }
                }
            }

            return false;
        }

		// Attempts to parse rule, has options whether the entire string should be consumed (requireEOF)
        template<class rule>
        static shared_ptr<Symbol> TestTryParse(string testString, int &deepestFailure, string &errorMessage, bool requireEOF = true)
        {
            TraceString1("**** Begin Test: ", SystemTraceType::Parsing, TraceDetail::Detailed, testString );

            deepestFailure = 0;
            shared_ptr<Lexer> lexer = shared_ptr<Lexer>(new Lexer());
            shared_ptr<istream> stream = shared_ptr<istream>(new stringstream(testString));
            stream->setf(ios::binary);

            lexer->Open(stream);
            shared_ptr<Symbol> symbol;

            if(requireEOF)
            {
                symbol = AndExpression<Args<rule, EofSymbol>>::TryParse(lexer, "Test Error Message");
            }
            else
            {
                symbol = rule::TryParse(lexer, "Test Error Message");
            }

            if(symbol == nullptr)
            {
                deepestFailure = lexer->DeepestFailure();
                errorMessage = lexer->ErrorMessage();
            }

            return symbol;
        }

		// returns true if the flattened tree returns a single symbolID 
        static bool CheckFlattenedSingleSymbolResult(shared_ptr<Symbol> rule, unsigned short ID, const string &result, vector<shared_ptr<Symbol>> &flattenedTree)
        {
            if(rule == nullptr)
            {
                return false;
            }

            flattenedTree.clear();
            rule->FlattenInto(flattenedTree);
            string foo = PrintTree(flattenedTree);

            if(!(flattenedTree.size() == 1)) 
            { 
                return false; 
            }
        
            if(!(flattenedTree[0]->symbolID() == ID)) 
            { 
                return false; 
            }

            string thisResult  = flattenedTree[0]->ToString();
            if(!(thisResult == result)) { 
                return false; 
            }

            return true;
        }

        static string PrintSymbol(shared_ptr<Symbol> symbol, int level)
        {
            stringstream stream;
       
            stream << string((size_t) (level * 3), ' ') << symbol->symbolID();
            switch(symbol->flattenType())
            {
            case FlattenType::None:
                stream << " (None)";
                break;
            case FlattenType::Flatten:
                stream << " (Flatten)";
                break;
            case FlattenType::Delete:
                stream << " (Delete)";
                break;
            }

            stream << ": " << symbol->ToString() << "\r\n";

            return stream.str();
        }

        static string PrintTree(shared_ptr<Symbol> &symbol, int level)
        {
            stringstream stream;
            stream << PrintSymbol(symbol, level);

            for_each(symbol->children().begin(), symbol->children().end(), 
                [&] (shared_ptr<Symbol> symbol)
            {
                stream << PrintTree(symbol, level + 1);
            });

            return stream.str();
        }

        static string PrintTree(vector<shared_ptr<Symbol>> &symbols)
        {
            stringstream stream;
            for_each(symbols.begin(), symbols.end(), 
                [&] (shared_ptr<Symbol> symbol)
            {
                stream << PrintTree(symbol, 1);
            });
            return stream.str();
        }

        static int MaxDepth(const vector<shared_ptr<Symbol>> &symbols)
        {
            int maxDepth = 0;
            if(symbols.size() > 0)
            {
                for_each(symbols.begin(), symbols.end(), 
                    [&] (shared_ptr<Symbol> symbol)
                {
                    int childMax = MaxDepth(symbol->children());
                    if(childMax > maxDepth)
                    {
                        maxDepth = childMax;
                    }
                });
            }

            return maxDepth + 1;
        }
    };
}
