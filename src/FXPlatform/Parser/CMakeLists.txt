target_sources(lib
    PRIVATE
    	${CMAKE_CURRENT_SOURCE_DIR}/Compiler.h
    	${CMAKE_CURRENT_SOURCE_DIR}/Lexer.h
    	${CMAKE_CURRENT_SOURCE_DIR}/Lexer.cpp
    	${CMAKE_CURRENT_SOURCE_DIR}/LexerReader.h
    	${CMAKE_CURRENT_SOURCE_DIR}/LexerReader.cpp
    	${CMAKE_CURRENT_SOURCE_DIR}/Parser.h
    	${CMAKE_CURRENT_SOURCE_DIR}/Parser.cpp
    	${CMAKE_CURRENT_SOURCE_DIR}/ParserDebug.h
)
target_include_directories(lib PRIVATE .)
target_include_directories(testLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)