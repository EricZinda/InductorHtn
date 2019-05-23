
# Getting Started Using C++ Code
Read readme.md for background on the engine and how to build it.

If you just want to learn how the engine works and write rules, start with gettingstarted.md first. 

If you want to learn how to embed the Inductor HTN engine into your project or call it from C++, you're in the right place.

## Calling Inductor HTN from C++
Start by reading [Inductor Prolog Getting Started](https://blog.inductorsoftware.com/InductorProlog/gettingstarted.html) to get an overview of the how to load files, the main classes that are used, etc for Prolog and then continue below.  

The C++ interface to InductorHTN is simply a set of additional classes used in addition to the InductorProlog engine.  The rest of this document outlines the additional classes added by Inductor HTN to make Hierarchical Task Networks work.


### Loading and Compiling HTN and Prolog Strings and Files
Works exactly like [Inductor Prolog](https://github.com/EricZinda/InductorProlog) since it is built on the same [Inductor Parser](https://github.com/EricZinda/InductorParser) framework. 
~~~
// InductorHtn uses a factory model for creating terms so it
// can "intern" them to save memory.  You must never
// mix terms from different HtnTermFactorys
shared_ptr<HtnTermFactory> factory = 
    shared_ptr<HtnTermFactory>(new HtnTermFactory());

// HtnRuleSet is where the Prolog facts and rules which are the state of the HTN are stored. 
shared_ptr<HtnRuleSet> state = 
    shared_ptr<HtnRuleSet>(new HtnRuleSet());

// HtnPlanner is a subclass of HtnDomain which stores the Operators and Methods, as well as having
// the code that implements the HTN algorithm
shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());

// The HtnCompiler will uses the standard Prolog syntax *except* that variables start with ? and capitalization doesn't
// mean anything special
shared_ptr<HtnCompiler> compiler = 
    shared_ptr<HtnCompiler>(
        new HtnCompiler(factory.get(), state.get(), planner.get()));

if(!compiler->CompileDocument(pathAndFile))
{
    fprintf(stdout, "Error compiling %s, %s\r\n", pathAndFile.c_str(), compiler->GetErrorString().c_str());
    return false;
}
~~~