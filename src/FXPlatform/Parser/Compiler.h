#pragma once
#include <fstream>
#include "Parser.h"
#include <string>

namespace FXPlat
{
    class Symbol;
    
    class CompileError
    {
    public:
        CompileError(int line, int column, const string &errorString) :
            m_column(column),
            m_error(errorString),
            m_line(line)
        {
        }

        string ToString()
        {
            return "Line " + lexical_cast<string>(line()) + ", Column " + lexical_cast<string>(column()) + ": " + error();
        }

        Property(public, int, column);
        Property(public, string, error);
        ValueProperty(public, string, file);
        Property(public, int, line);
    };

	// Base class for a compiler that uses a specific parser
    // Usage:
    //  1. create a class that derives from Compiler<baserule> where baserule is a rule that parses an entire document
    //  2. add members to that class which contain whatever you are compiling into.  I.e. whatever the document is supposed to become
    //  3. override virtual bool ProcessAst(shared_ptr<CompileResultType> ast) and turn the symbols created by the rules into whatever they become
    template<class parser>
    class Compiler
    {
    public:
        typedef vector<shared_ptr<Symbol>> CompileResultType;
        virtual ~Compiler()
        {
        }
        
        bool CompileDocument(const string &fullPath)
        {
            shared_ptr<ifstream> stream = shared_ptr<ifstream>(new ifstream());
            StartTimingOnly(Compiler_CompileDocument_LoadFile, SystemTraceType::HTML, TraceDetail::Detailed);
            stream->open(fullPath, ios::binary);
            EndTimingOnly(Compiler_CompileDocument_LoadFile, SystemTraceType::HTML, TraceDetail::Detailed);
            if(stream->good())
            {
                return Compile(stream);
            }
            else
            {
                this->errors().push_back(CompileError(-1, -1, "error loading file '" + fullPath + "'"));
                return false;
            }
        }

        bool Compile(const string document)
        {
            shared_ptr<istream> stream = shared_ptr<istream>(new stringstream(document));
//            stream->setf(ios::binary);
            return Compile(stream);
        }

        bool Compile(shared_ptr<istream> stream)
        {
            Initialize();
            shared_ptr<CompileResultType> result = shared_ptr<CompileResultType>(new CompileResultType());
            shared_ptr<CompileError> error = this->Compile(stream, result);

            //string foo = ParserDebug::PrintTree(*result);

            if(error != nullptr)
            {
                errors().push_back(*error);
                TraceString1("Compiler::Compile Error {0}",
                             SystemTraceType::System, TraceDetail::Normal,
                             GetErrorString());
                return false;
            }

            return ProcessAst(result);
        }

        static shared_ptr<Symbol> GetChild(shared_ptr<Symbol> symbol, int level0Index, int ID0)
        {
            if(symbol->children().size() >= (size_t) level0Index + 1)
            {
                shared_ptr<Symbol> found = symbol->children()[level0Index];
                if(ID0 == -1 || found->symbolID() == ID0)
                {
                    return found;
                }
            }

            return nullptr;
        }

        static shared_ptr<Symbol> GetChild(shared_ptr<Symbol> symbol, int level0Index, int ID0, int level1Index, int ID1)
        {
            shared_ptr<Symbol> level0Symbol = GetChild(symbol, level0Index, ID0);
            if(level0Symbol == nullptr) { return nullptr; }
            return GetChild(level0Symbol, level1Index, ID1);
        }

        static shared_ptr<Symbol> GetChild(shared_ptr<Symbol> rootSymbol, int level0Index, int ID0, int level1Index, int ID1, int level2Index, int ID2)
        {
            shared_ptr<Symbol> level0Symbol = GetChild(rootSymbol, level0Index, ID0);
            if(level0Symbol == nullptr) { return nullptr; }
            return GetChild(level0Symbol, level1Index, ID1, level2Index, ID2);
        }

        string GetErrorString()
        {
            stringstream stream;

            for(vector<CompileError>::iterator iter = m_errors.begin(); iter != m_errors.end(); ++iter)
            {
                stream << iter->ToString();
            }

            return stream.str();
        }

        bool HasErrors() { return m_errors.size() > 0; }
        virtual void Initialize()
        {
            m_errors.clear();
        }

        Property(private, vector<CompileError>, errors);

    protected:
        shared_ptr<CompileError> Compile(shared_ptr<istream> stream, shared_ptr<CompileResultType> &flattened)
        {
            shared_ptr<Lexer> lexer = shared_ptr<Lexer>(new Lexer());
            lexer->Open(stream);

            StartTimingOnly(Compiler_Compile_Parse, SystemTraceType::HTML, TraceDetail::Detailed);
            shared_ptr<Symbol> result = parser::TryParse(lexer, "");
            EndTimingOnly(Compiler_Compile_Parse, SystemTraceType::HTML, TraceDetail::Detailed);

            if(result != nullptr)
            {
                // No Errors, flatten now

                flattened = shared_ptr<vector<shared_ptr<Symbol>>>(new vector<shared_ptr<Symbol>>());
                StartTimingOnly(Compiler_Compile_Flatten, SystemTraceType::HTML, TraceDetail::Detailed);
                result->FlattenInto(*flattened);
                EndTimingOnly(Compiler_Compile_Flatten, SystemTraceType::HTML, TraceDetail::Detailed);
                return nullptr;
            }
            else
            {
                // Got an error, figure out what line it is on
                int lineCount;
                int columnCount;
                string errorLine;
                Lexer::GetLineAndColumn(lexer->DeepestFailure(), stream, lineCount, columnCount, &errorLine);
                return shared_ptr<CompileError>(new CompileError(lineCount, columnCount, lexer->ErrorMessage() + ", \r\n" + errorLine));
            }
        }

		// This must be overridden to actually process the tree that got parsed
        virtual bool ProcessAst(shared_ptr<CompileResultType> ast) = 0;
    };
}
