//
//  HtnPrologParser.cpp
//  GameLib
//
//  Created by Eric Zinda on 10/10/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//

#include "PrologParser.h"

namespace Prolog
{
    char PrologAtomError[] = "Expected atom (e.g. 'name')";
    char PrologVariableError[] = "Expected Variable";
    char PrologTermError[] = "Expected term (i.e. Variable or Functor)";
    char PrologTermListError[] = "Expected term list (i.e. list of Variables or Functors)";
    char PrologFunctorListError[] = "Expected list of functors (e.g. a(), b(), c())";
    char PrologFunctorError[] = "Expected functor (e.g. a(b, c)";
    char PrologListError[] = "Expected list (i.e. [] or [a, b, ...])";
    char PrologRuleError[] = "Expected rule (e.g. a() :- b()";
    char PrologDocumentError[] = "Expected Prolog document (i.e. rules or facts, separated by '.'";
    char PrologCommentError[] = "Expected comment (e.g. '/* */' or '%')";
    char PrologQueryError[] = "Expected query (e.g. a(B).)";
    char PrologTailTermError[] = "Expected | followed by a single term (e.g. [a | X].)";

    char emptyList[] = "[]";
	char BeginCommentBlock[] = "/*";
    char CrlfString[] = "\r\n";    
	char CapitalChar[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char EndCommentBlock[] = "*/";
}
