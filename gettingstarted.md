# Getting Started
Read readme.md for background on the engine and how to build it, this document describes how to use it if you think it is right for you.


## Background Reading
- What an HTN is: [HTN Overview](https://blog.inductorsoftware.com/blog/htnoverview)
- The Prolog engine used inside the Inductor HTN Engine: [Inductor Prolog Overview](https://blog.inductorsoftware.com/blog/prolog)
- How to use the Inductor HTN Language: [Inductor HTN Overview](https://blog.inductorsoftware.com/blog/InductorHtnOverview)
- Example of using an HTN for a strategy game: [Inductor HTN Example](https://blog.inductorsoftware.com/blog/inductorhtnexample)
- For a lot of background on how this HTN Engine was used in production in a strategy game, start at the first blog entry on [Exospecies Blog](https://blog.inductorsoftware.com/blog/schmingularity) and read through to the bitter end.

## Optimal Problems for HTNs

Hierarchical Task Networks are a proven model for solving many AI Planning problems and they've been around for a long time. I've found that they are a good solution if you need an engine that can create a plan in a complex problem space where doing an exhaustive search (or an approximation of it) to solve the problem simply isn't an option AND where you have an expert that knows the right answer (or a good enough answer) because they're going to have to code up the rules.  Your HTN will only do its job as well as the best person you have writing the rules.

One example where I think HTN's *shouldn't* be used: Two-person zero-sum games with perfect information (Chess, Checkers, etc.), I suspect that some variant of the [minimax algorithm](https://en.wikipedia.org/wiki/Minimax) is going to be your best bet.  This does an exhaustive search or close enough for many purposes.

HTN's were a great solution for [Exospecies](www.exospecies.com) because it is a complex game with resource management and the high cost of calculating a turn makes running lots of scenarios (like minimax does) impossible.  An approach that used rules written by an expert was the best I was going to do. That's what the Inductor HTN Engine was originally built for and where it was first used in production.  It uses Prolog as a primary part of its language and the [Inductor Prolog Engine](https://github.com/EricZinda/InductorProlog) as part of its runtime engine.

## Performance
Note that the performance of this project is *HUGELY* dependent on whether you have built for retail or debug.  Debug builds have error checking which does *major* damage to performance.  Make sure you run in retail if you are evaluating the performance!

## Usage Overview
The InductorHtn engine adds HTN capabilities on top of what is basically a classic Prolog compiler (the [Inductor Prolog Compiler](https://github.com/EricZinda/InductorProlog)). So, understanding Prolog is key to using this engine.  In fact, you can use all the normal Prolog features implemented in Inductor Prolog as a part of your HTN application and "mix and match" HTN constructs alongside Prolog constructs. Background reading to get you up to speed on Prolog is in the section above.

There are three steps to using this engine in an application:
1. Convert the app state you need to process into Prolog Facts 
2. Write the HTN Axioms, Methods, and Operators you need and use the Facts
3. Convert the Operators that get generated into changes, moves, or whatever makes sense in your app

To make it easy to prototype or try out the engine, the build system builds an interactive mode application called `indhtn`. The next section describes how to use it.

## Using Interactive Mode
The easiest way to use interactive mode is to create a single file with a `.htn` extension and pass it on the command line. You can write down the facts that will be input to the engine, the HTN Axioms, Methods, and Operators that are your HTN logic and run it interactively. There is a tiny amount of help built into the app that should get you going.

The [Inductor HTN Example](https://blog.inductorsoftware.com/blog/inductorhtnexample) shows how to use it in detail.

## Calling Inductor HTN from Python
The Python interface to InductorHtn is a single class called HtnPlanner with a few simple methods. It works with Python 3.x and uses the CTypes interface for calling the C++ implementation, which should be included with most Pythons.

There are only two files you need to use it: 
1. indhtnpy.dll (Windows) or libindhtnpy.dylib (Mac). This gets built as part of your build. It needs to be on your system path so Python can find it. 
2. indhtnpy.py. This is the Python interface to the DLL. It needs to be included with your Python program, whereever that is.

Here is the content of PythonUsage.py which describes the entire interface:
~~~
# See GettingStarted.md for a whole bunch of pointers to understand HTNs, Prolog, etc. This 
# file just describes how to use the framework
from indhtnpy import *
import json
import pprint

# The only class for InductorHtn is called HtnPlanner
# Passing true as the (only) argument turns on debug mode
# Traces of what is happening are sent to the system debug output stream which can be 
# seen on windows with https://docs.microsoft.com/en-us/sysinternals/downloads/debugview
# These traces are much like the standard Prolog traces and will help you understand how 
# the queries and HTN tasks are running and what path they are taking
test = HtnPlanner(False)


# HtnPlanner.HtnCompile()
# Compile a program which includes both HTN and Prolog statements
# The HtnCompile() uses the standard Prolog syntax
# Calling HtnCompile() multiple times will keep adding statements to the database.
# You will get an error if some already exist
prog = """
travel-to(Q) :- 
        if(at(P), walking-distance(P, Q)), 
        do(walk(P, Q)).
    walk(Here, There) :- 
        del(at(Here)), add(at(There)).
    walking-distance(U,V) :- weather-is(good), 
                               distance(U,V,W), =<(W, 3).
    walking-distance(U,V) :- distance(U,V,W), =<(W, 0.5).
    distance(downtown, park, 2).
    distance(downtown, uptown, 8).
    at(downtown).
    weather-is(good).
    """
result = test.HtnCompile(prog)
if result is not None:
    print("HtnCompile Error:" + result)
    sys.exit()

# HtnPlanner.PrologCompile()
# Compile a standard Prolog program and *add* it to the above, just like HtnCompile() does
# PrologCompile() uses all of the Prolog standard syntax
prog = """
    mortal(X) :- human(X).
    human(socrates).
    """
result = test.PrologCompile(prog)
if result is not None:
    print("PrologCompile Error:" + result)
    sys.exit()

####
# Now the database contains *all* of the facts, rules, and tasks from both programs above!
####

# HtnPlanner.FindAllPlans()
# Gets all possible plans that are generated by a query (will return a list of plans, each
# which is a list of terms)
# results are returned in Json format (described farther down)
success, result = test.FindAllPlans("travel-to(park).")
if success is not None:
    print("FindAllPlans error: " + success)
    sys.exit()

solutions = json.loads(result)
print("FindAllPlans result:")
pp = pprint.PrettyPrinter(indent=4)
pp.pprint(solutions)

# HtnPlanner.HtnQuery()
# Run a standard Prolog query using the Htn syntax where variables don't have to be
# capitalized, but must have a ? in front
# results are returned in Json format (described farther down)

# HtnPlanner.PrologQuery()
# Run a standard Prolog query
# results are always returned with a ? in front of the name, however
# results are returned in Json format (described farther down)
success, result = test.PrologQuery("human(Who).")
if success is not None:
    print("PrologQuery error: " + success)
    sys.exit()
answer = json.loads(result)
print("PrologQuery result:")
pp = pprint.PrettyPrinter(indent=4)
pp.pprint(answer)

success, result = test.PrologQuery("at(Where).")
if success is not None:
    print("PrologQuery error: " + success)
    sys.exit()
answer = json.loads(result)
print("PrologQuery result:")
pp = pprint.PrettyPrinter(indent=4)
pp.pprint(answer)

# Results are always returned as Json.  
# Terms are just dictionaries with one key, the name of the term, and one value: a list 
# of more terms
# termName(arg1, arg2) => {"termName":[ {"arg1":[]}, {"arg2":[]} ]}

# Here are some examples:
# term that is a constant (aka a term name with no arguments): e.g. tile
term = json.loads("{\"tile\" : [] }")

# term that is a variable always has a ? in front of it: ?tile
term = json.loads("{\"?tile\" : [] }")

# term with arguments: tile(position(1), 1)
term = {"tile" : [{"position" : [1]}, 1]}

# first arg of known term "tile"
print(term["tile"][0])

# There are some helper functions to make accessing things "prettier"
# foo(bar, goo), tile(position(1), 1)
termList = json.loads("[{\"foo\" : [\"bar\", \"goo\"]}, {\"tile\" : [\"firstArg\", \"secondArg\"] }]")

# termName() gives the name of the term
print(termName(termList[0]))

# termArgs() gets the args for a term
print(termArgs(termList[0])[0])
~~~

And here is the output of the program above:
~~~
C:\InductorHtn\src\Python>python PythonUsage.py
FindAllPlans result:
[[{'walk': [{'downtown': []}, {'park': []}]}]]
PrologQuery result:
[{'?Who': {'socrates': []}}]
PrologQuery result:
[{'?Where': {'downtown': []}}]
{'position': [1]}
foo
bar
~~~

## Calling Inductor HTN from C++
The C++ interface to InductorHTN is simply a set of classes used in addition to the [Inductor Prolog Engine](https://github.com/EricZinda/InductorProlog). So, start by reading [Inductor Prolog Getting Started](https://blog.inductorsoftware.com/InductorProlog/gettingstarted.html) to get an overview of the how to load files, the main classes that are used, etc for Prolog and then continue below.  

The rest of this document outlines the additional classes added by Inductor HTN to make Hierarchical Task Networks work.


### Loading and Compiling HTN Strings and Files
For more information on the HTN syntax see the [Inductor HTN Overview](https://blog.inductorsoftware.com/blog/InductorHtnOverview).

If you want to load a set of HTN rules, it works exactly like [Inductor Prolog](https://github.com/EricZinda/InductorProlog) since it is built on the same [Inductor Parser](https://github.com/EricZinda/InductorParser) framework. There are overloads on the `HtnCompiler` class for loading strings, streams, etc. 
~~~
// InductorHtn uses the same factory model (and classes) as InductorProlog
// for creating terms so it can "intern" them to save memory.  
// You must never mix terms from different HtnTermFactory's
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
    fprintf(stdout, "Error compiling: %s\r\n", compiler->GetErrorString().c_str());
    return false;
}
~~~

If compilation is successful, the compiler will have filled the `HtnRuleSet` with any Facts and Axioms, and the `HtnPlanner` with Methods and Operators.

### Creating HTN Methods and Operators by Hand
If you don't want to compile an HTN file, you can easily create Methods and Operators by hand (ditto for Faccts and Axioms (aka Prolog Rules), see [Inductor Prolog Getting Started](https://github.com/EricZinda/InductorProlog/blob/master/gettingstarted.md). In fact, most of the code is the same):
~~~
// Create the method: travel-to(?q) :- 
//      if(at(?p), walking-distance(?p, ?q)),
//      do(walk(?p, ?q))."
shared_ptr<HtnTerm> head = 
    factory->CreateFunctor("travel-to", 
    	{ 
    		factory->CreateVariable("q") 
    	});

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


