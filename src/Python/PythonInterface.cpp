#include <memory>
#include <new> //For std::nothrow
#include <stdio.h>
#include "FXPlatform/Htn/HtnCompiler.h"
#include "FXPlatform/Htn/HtnPlanner.h"
#include "FXPlatform/Prolog/PrologQueryCompiler.h"
using namespace std;
// https://solarianprogrammer.com/2019/07/18/python-using-c-cpp-libraries-ctypes/
// https://dbader.org/blog/python-ctypes-tutorial
// http://svn.python.org/projects/ctypes/trunk/ctypes/docs/manual/tutorial.html#pointers

const int defaultMemoryBudget = 1000000;
class HtnPlannerPythonWrapper
{
public:
    HtnPlannerPythonWrapper(bool debug)
    {
        if (debug)
        {
            SetTraceFilter((int)SystemTraceType::Solver | (int)SystemTraceType::Planner, TraceDetail::Diagnostic);
        }
        else
        {
            SetTraceFilter(SystemTraceType::None, TraceDetail::Normal);
        }
        
        m_budgetBytes = defaultMemoryBudget;

        // InductorHtn uses the same factory model (and classes) as InductorProlog
        // for creating terms so it can "intern" them to save memory.  
        // You must never mix terms from different HtnTermFactory's
        m_factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());

        // HtnRuleSet is where the Prolog facts and rules which are the state 
        // of the HTN are stored. 
        m_state = shared_ptr<HtnRuleSet>(new HtnRuleSet());

        // HtnPlanner is a subclass of HtnDomain which stores the Operators and 
        // Methods as well as having the code that implements the HTN algorithm
        m_planner = shared_ptr<HtnPlanner>(new HtnPlanner());

        // The HtnCompiler will uses the standard Prolog syntax *except* that 
        // variables start with ? and capitalization doesn't mean anything special
        m_compiler = shared_ptr<HtnCompiler>(new HtnCompiler(m_factory.get(), m_state.get(), m_planner.get()));

        // PrologStandardCompiler uses all of the Prolog standard syntax
        m_prologCompiler = shared_ptr<PrologStandardCompiler>(new PrologStandardCompiler(m_factory.get(), m_state.get()));

        // HtnGoalResolver is the Prolog "engine" that resolves queries
        m_resolver = shared_ptr<HtnGoalResolver>(new HtnGoalResolver());
    }

public:
    uint64_t m_budgetBytes;
    shared_ptr<HtnCompiler> m_compiler;
    shared_ptr<PrologStandardCompiler> m_prologCompiler;
    shared_ptr<HtnTermFactory> m_factory;
    // We save the last set of solutions so their state can be applied if the user chooses to
    shared_ptr<HtnPlanner::SolutionsType> m_lastSolutions;
    shared_ptr<HtnPlanner> m_planner;
    shared_ptr<HtnGoalResolver> m_resolver;
    shared_ptr<HtnRuleSet> m_state;
};

#if defined(_MSC_VER)
    #define __declspec(x) __declspec(x)
    #define __stdcall __stdcall
#else
    #if defined(__GNUC__)
        #define __declspec(x)
        #define __stdcall
        #define _strdup strdup
    #endif
#endif

extern "C"  //Tells the compile to use C-linkage for the next scope.
{
    __declspec(dllexport) HtnPlannerPythonWrapper* __stdcall  CreateHtnPlanner(bool debug)
    {
        // Note: Inside the function body, I can use C++.
        return new(std::nothrow) HtnPlannerPythonWrapper(debug);
    }

    __declspec(dllexport) void __stdcall DeleteHtnPlanner(HtnPlannerPythonWrapper* ptr)
    {
        operator delete(ptr, std::nothrow);
    }

    __declspec(dllexport) void __stdcall FreeString(char* value)
    {
        free(value);
    }

    char* GetCharPtrFromString(const string& value)
    {
        return _strdup(value.c_str());
    }

    __declspec(dllexport) bool __stdcall HtnApplySolution(HtnPlannerPythonWrapper* ptr, const uint64_t solutionIndex)
    {
        if (ptr->m_lastSolutions != nullptr && solutionIndex < ptr->m_lastSolutions->size())
        {
            ptr->m_state = (*ptr->m_lastSolutions)[solutionIndex]->finalState();
            return true;
        }
        else
        {
            return false;
        }
    }

    // Compile *adds* whatever is passed into the current state of the database
    __declspec(dllexport) char* __stdcall HtnCompile(HtnPlannerPythonWrapper* ptr, const char *data)
     {
        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string programString = string(data);
            if (!ptr->m_compiler->Compile(programString))
            {
                return GetCharPtrFromString(ptr->m_compiler->GetErrorString());
            }
            else
            {
                return nullptr;
            }
        }
        catch (runtime_error &error)
        {
            return GetCharPtrFromString(error.what());
        }
     }

    // Returns result in Json format
    __declspec(dllexport) char* __stdcall HtnFindAllPlans(HtnPlannerPythonWrapper* ptr, char *queryChars, char **result)
    {
        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string queryString = string(queryChars);

            // The PrologQueryCompiler will compile Prolog queries using the normal
            // Prolog parsing rules *except* that variables start with ? and
            // capitalization doesn't mean anything special
            shared_ptr<PrologQueryCompiler> queryCompiler = shared_ptr<PrologQueryCompiler>(new PrologQueryCompiler(ptr->m_factory.get()));

            if (queryCompiler->Compile(queryString))
            {
                ptr->m_lastSolutions = ptr->m_planner->FindAllPlans(ptr->m_factory.get(),
                                                                    ptr->m_state,
                                                                    queryCompiler->result(),
                                                                    ptr->m_budgetBytes);
                *result = GetCharPtrFromString(HtnPlanner::ToStringSolutions(ptr->m_lastSolutions, true));

                return nullptr;
            }
            else
            {
                *result = nullptr;
                return GetCharPtrFromString(queryCompiler->GetErrorString());
            }
        }
        catch (runtime_error & error)
        {
            *result = nullptr;
            return GetCharPtrFromString(error.what());
        }
    }

    __declspec(dllexport) char* __stdcall HtnQuery(HtnPlannerPythonWrapper* ptr, char* queryChars, char** result)
    {
        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string queryString = string(queryChars);

            // The PrologQueryCompiler will compile Prolog queries using the normal
            // Prolog parsing rules *except* that variables start with ? and
            // capitalization doesn't mean anything special
            shared_ptr<PrologQueryCompiler> queryCompiler = shared_ptr<PrologQueryCompiler>(new PrologQueryCompiler(ptr->m_factory.get()));

            if (queryCompiler->Compile(queryString))
            {
                // The resolver can give one answer at a time using ResolveNext(), or just get them all using ResolveAll()
                shared_ptr<vector<UnifierType>> queryResult = ptr->m_resolver->ResolveAll(ptr->m_factory.get(),
                                                                                          ptr->m_state.get(),
                                                                                          queryCompiler->result(),
                                                                                          0,
                                                                                          ptr->m_budgetBytes);
                if (queryResult == nullptr)
                {
                    *result = GetCharPtrFromString("{\"False\" :[]}");
                }
                else
                {
                    *result = GetCharPtrFromString(HtnGoalResolver::ToString(queryResult.get(), true));
                }

                return nullptr;
            }
            else
            {
                *result = nullptr;
                return GetCharPtrFromString(queryCompiler->GetErrorString());
            }
        }
        catch (runtime_error & error)
        {
            *result = nullptr;
            return GetCharPtrFromString(error.what());
        }
    }

    // Compile *adds* whatever is passed into the current state of the database
    __declspec(dllexport) char* __stdcall PrologCompile(HtnPlannerPythonWrapper* ptr, const char* data)
    {
        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string programString = string(data);
            if (!ptr->m_prologCompiler->Compile(programString))
            {
                return GetCharPtrFromString(ptr->m_compiler->GetErrorString());
            }
            else
            {
                return nullptr;
            }
        }
        catch (runtime_error & error)
        {
            return GetCharPtrFromString(error.what());
        }
    }

    __declspec(dllexport) void __stdcall SetDebugTracing(bool debug)
    {
        if (debug)
        {
            SetTraceFilter((int)SystemTraceType::Solver | (int)SystemTraceType::Planner, TraceDetail::Diagnostic);
        }
        else
        {
            SetTraceFilter(SystemTraceType::None, TraceDetail::Normal);
        }
    }

    __declspec(dllexport) void __stdcall SetMemoryBudget(HtnPlannerPythonWrapper* ptr, const uint64_t budgetBytes)
    {
        // Reset the out of memory flag if it got set
        ptr->m_factory->outOfMemory(false);
        ptr->m_budgetBytes = budgetBytes;
    }

    // returns result in Json format
    // if the query failed, it will be a False term
    __declspec(dllexport) char* __stdcall StandardPrologQuery(HtnPlannerPythonWrapper* ptr, char* queryChars, char** result)
    {
        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string queryString = string(queryChars);

            // The PrologStandardQueryCompiler will compile Prolog queries using the normal
            // Prolog parsing rules requiring Capitalization of variables
            shared_ptr<PrologStandardQueryCompiler> queryCompiler = shared_ptr<PrologStandardQueryCompiler>(new PrologStandardQueryCompiler(ptr->m_factory.get()));

            if(queryCompiler->Compile(queryString))
            {
                // The resolver can give one answer at a time using ResolveNext(), or just get them all using ResolveAll()
                int64_t highestMemoryUsedReturn;
                int furthestFailureIndex;
                std::vector<std::shared_ptr<HtnTerm>> farthestFailureContext;
                shared_ptr<vector<UnifierType>> queryResult = ptr->m_resolver->ResolveAll(ptr->m_factory.get(),
                                                                                          ptr->m_state.get(),
                                                                                          queryCompiler->result(),
                                                                                          0,
                                                                                          ptr->m_budgetBytes,
                                                                                          &highestMemoryUsedReturn,
                                                                                          &furthestFailureIndex,
                                                                                          &farthestFailureContext);
                if(ptr->m_factory->outOfMemory())
                {
                    string outOfMemoryString =  "out of memory: Budget:" + lexical_cast<string>(ptr->m_budgetBytes) +
                                               ", Highest total memory used: " + lexical_cast<string>(highestMemoryUsedReturn) +
                                               ", Memory used only by term names: " + lexical_cast<string>(ptr->m_factory->dynamicSize()) +
                                               ", The difference was probably used by the resolver, either in its stack memory or memory used by the number of terms that unify with a single term. Turn on tracing to see more details.";
                    *result = nullptr;
                    return GetCharPtrFromString(outOfMemoryString);
                }
                else if(queryResult == nullptr)
                {
                    string failureContextString;
                    if(farthestFailureContext.size() > 0)
                    {
                        failureContextString = ", " + HtnTerm::ToString(farthestFailureContext, false, true);
                    }
                    *result = GetCharPtrFromString("[{\"False\" :[]}, {\"failureIndex\" :[{\"" + lexical_cast<string>(furthestFailureIndex) + "\" :[]}]}" + failureContextString + "]");
                }
                else
                {
                    *result = GetCharPtrFromString(HtnGoalResolver::ToString(queryResult.get(), true));
                }

                return nullptr;
            }
            else
            {
                *result = nullptr;
                return GetCharPtrFromString(queryCompiler->GetErrorString());
            }
        }
        catch (runtime_error & error)
        {
            *result = nullptr;
            return GetCharPtrFromString(error.what());
        }
    }
} //End C linkage scope.
