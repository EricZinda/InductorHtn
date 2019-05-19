target_sources(lib
    PRIVATE
    	Compiler.h
    	Lexer.h
    	Lexer.cpp
    	LexerReader.h
    	LexerReader.cpp
    	Parser.h
    	Parser.cpp
    	ParserDebug.h
)
target_include_directories(lib PRIVATE .)
target_include_directories(testLib PRIVATE ./)