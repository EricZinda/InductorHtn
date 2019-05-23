

# Getting Started Using C++ Code
Read readme.md for background on the engine and how to build it.

If you just want to learn how the engine works and write rules, start with gettingstarted.md first. 

If you want to learn how to embed the Inductor HTN engine into your project or call it from C++, you're in the right place.

## Calling Inductor HTN from C++
The C++ interface to InductorHTN is simply a set of classes used in addition to the InductorProlog engine. So, start by reading [Inductor Prolog Getting Started](https://blog.inductorsoftware.com/InductorProlog/gettingstarted.html) to get an overview of the how to load files, the main classes that are used, etc for Prolog and then continue below.  

The rest of this document outlines the additional classes added by Inductor HTN to make Hierarchical Task Networks work.


### Loading and Compiling HTN Strings and Files
For more information on the HTN syntax see the [Inductor HTN Overview](https://blog.inductorsoftware.com/blog/InductorHtnOverview).

If you want to load a set of HTN rules, it works exactly like [Inductor Prolog](https://github.com/EricZinda/InductorProlog) since it is built on the same [Inductor Parser](https://github.com/EricZinda/InductorParser) framework. There are overloads on the `HtnCompiler` class for loading strings, streams, etc. 
~~~
// InductorHtn uses the same factory model (and classes) as InductorProlog
// for creating terms so it can "intern" them to save memory.  
// You must never mix terms from different HtnTermFactorys
shared_ptr<HtnTermFactory> factory = 
    shared_ptr<HtnTermFactory>(new HtnTermFactory());

// HtnRuleSet is where the Prolog facts and rules which are the state 
// of the HTN are stored. 
shared_ptr<HtnRuleSet> state = 
    shared_ptr<HtnRuleSet>(new HtnRuleSet());

// HtnPlanner is a subclass of HtnDomain which stores the Operators and 
// Methods as well as having the code that implements the HTN algorithm
shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());

// The HtnCompiler will uses the standard Prolog syntax *except* that 
// variables start with ? and capitalization doesn't mean anything special
shared_ptr<HtnCompiler> compiler = 
    shared_ptr<HtnCompiler>(
        new HtnCompiler(factory.get(), state.get(), planner.get()));

// Compile a simple HTN example in a string
if(!compiler->Compile(
    "travel-to(?q) :- "
    "    if(at(?p), walking-distance(?p, ?q)), "
    "    do(walk(?p, ?q))."
    "walk(?here, ?there) :-" 
    "    del(at(?here)), add(at(?there))."
    "walking-distance(?u,?v) :- weather-is(good), "
    "                           distance(?u,?v,?w), =<(?w, 3)."
    "walking-distance(?u,?v) :- distance(?u,?v,?w), =<(?w, 0.5)."
    "distance(downtown, park, 2)."
    "distance(downtown, uptown, 8)."
    "at(downtown)."
    "weather-is(good)."
    )
{
    fprintf(stdout, "Error compiling %s, %s\r\n", pathAndFile.c_str(), compiler->GetErrorString().c_str());
    return false;
}
~~~

If compilation is successful, the compiler will have filled the HtnRuleSet with any Facts and Axioms, and the HtnPlanner with Methods and Operators.

### Creating HTN Methods and Operators by hand
If you don't want to compile an HTN file, you can easily create Methods and Operators by hand.  The same is true for Facts and Axioms (aka Prolog Rules), and this is shown in the [Inductor Prolog Getting Started](https://github.com/EricZinda/InductorProlog/blob/master/gettingstarted.md).  In fact, most of the code is the same:
~~~
// Create the method: travel-to(?q) :- 
//      if(at(?p), walking-distance(?p, ?q)),
//      do(walk(?p, ?q))."
shared_ptr<HtnTerm> head = 
    factory->CreateFunctor("travel-to", { factory->CreateVariable("q") });

vector<std::shared_ptr<HtnTerm>> condition = 
    {   
        factory->CreateFunctor("at", 
            {   
                factory->CreateVariable("p") 
            }),
        factory->CreateFunctor("walking-distance", 
            {   
                factory->CreateVariable("p"), 
                factory->CreateVariable("q")
            })
    };

vector<std::shared_ptr<HtnTerm>> subTasks = 
    {
        factory->CreateFunctor("walk", 
            { 
                factory->CreateVariable("p"), 
                factory->CreateVariable("q")
            })          
    };

planner->AddMethod(head, condition, subTasks, HtnMethodType::Normal, false);
~~~

Operator is analogous, you just call `AddOperator()` instead.

### Executing a Goal Using a String
An HTN goal is simply one or more HTN Tasks that you want to execute.  Using our example above, a goal might be:
~~~
travel-to(park).
~~~

The easiest way to execute an HTN goal after compiling a set of rules like above is to compile it as a string. The example below is using instances created above like `factory` and `planner`:
~~~
// The PrologQueryCompiler will compile Prolog queries using the normal 
// Prolog parsing rules *except* that variables start with ? and 
// capitalization doesn't mean anything special
shared_ptr<PrologQueryCompiler> queryCompiler = 
    shared_ptr<PrologQueryCompiler>(new PrologQueryCompiler(factory.get()));

if(queryCompiler->Compile("travel-to(park)."))
{
    shared_ptr<HtnPlanner::SolutionsType> solutions =
        planner->FindAllPlans(factory.get(), state, queryCompiler->result());

    fprintf(stdout, ">> %s\r\n\r\n",
        HtnPlanner::ToStringSolutions(solutions).c_str());
}
else
{
    fprintf(stdout, "Error: %s\r\n\r\n", 
        queryCompiler->GetErrorString().c_str());
}
~~~

### Description of Key Classes
Some of the classes you'll use for InductorHtn (`HtnTermFactory`, `HtnTerm`, `HtnRuleSet`, `HtnGoalResolver`) are inherited from the [Inductor Prolog](https://github.com/EricZinda/InductorProlog/blob/master/gettingstarted.md#htntermfactory) project and are described there.  Only the ones added by Inductor HTN are described below:

#### HtnMethod and HtnOperator
Both are immutable classes that represent an HTN Method and Operator that are normally created by calling `HtnPlanner::AddMethod() and AddOperator()`. However, they can be constructed using `new` as well.  Very straightforward.

#### HtnPlanner
This is the main implementation of the HTN Planner.  
- It implements the HTN algorithm described [here](https://blog.inductorsoftware.com/blog/htnoverview) 
- It has features like anyOf, allOf, else, etc as described [here](https://blog.inductorsoftware.com/blog/InductorHtnOverview)
- It is stackless for the same reasons and using the same approach as the [Inductor Prolog](https://github.com/EricZinda/InductorProlog) Goal Resolver, described [here](https://github.com/EricZinda/InductorProlog/blob/master/gettingstarted.md#stackless-execution).

You supply it with Methods and Operators via one of the compilers or the `Add...` methods described above.  Prolog Facts and Rules (i.e. a `RuleSet`) are supplied when you ask it to find a plan via `FindPlan` (which finds the next plan) or `FindAllPlans` which finds all possible plans.







