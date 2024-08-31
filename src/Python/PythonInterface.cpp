#include <memory>
#include <new> //For std::nothrow
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "FXPlatform/Logger.h"
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

        // The HtnCompiler will uses the standard Prolog syntax
        m_htnCompiler = shared_ptr<HtnStandardCompiler>(new HtnStandardCompiler(m_factory.get(), m_state.get(), m_planner.get()));

        // PrologStandardCompiler uses all of the Prolog standard syntax
        m_prologCompiler = shared_ptr<PrologStandardCompiler>(new PrologStandardCompiler(m_factory.get(), m_state.get()));

        // The HtnCompiler will uses the custom HTN syntax
        m_htnCompilerCustomVariables = shared_ptr<HtnCompiler>(new HtnCompiler(m_factory.get(), m_state.get(), m_planner.get()));

        // The Prolog compiler will uses the custom HTN syntax
        m_prologCompilerCustomVariables = shared_ptr<PrologCompiler>(new PrologCompiler(m_factory.get(), m_state.get()));

        // HtnGoalResolver is the Prolog "engine" that resolves queries
        m_resolver = shared_ptr<HtnGoalResolver>(new HtnGoalResolver());
    }

public:
    uint64_t m_budgetBytes;
    shared_ptr<HtnStandardCompiler> m_htnCompiler;
    shared_ptr<PrologStandardCompiler> m_prologCompiler;
    shared_ptr<HtnCompiler> m_htnCompilerCustomVariables;
    shared_ptr<PrologCompiler> m_prologCompilerCustomVariables;
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
    __declspec(dllexport) void __stdcall LogStdErrToFile(HtnPlannerPythonWrapper* ptr, char* pathAndFile)
    {
        string pathAndFileString(pathAndFile);
        DebugLogMessagesToFile(pathAndFile);
    }

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


    bool hasCustomVariables(const std::string& program){
        bool customVariables = false;
        if (program.find("?") != std::string::npos){
            customVariables = true;
        }
        return customVariables;
    }

    bool hasHtnKeywords(const std::string& program){
        auto splitString = [](const std::string& text, const std::string& delims)
        {
            std::vector<std::string> tokens;
            std::size_t start = text.find_first_not_of(delims), end = 0;

            while((end = text.find_first_of(delims, start)) != std::string::npos)
            {
                tokens.push_back(text.substr(start, end - start));
                start = text.find_first_not_of(delims, end);
            }
            if(start != std::string::npos)
                tokens.push_back(text.substr(start));

            return tokens;
        };

        std::vector<std::string> htnKeywords {"add", "del", "if", "do", "else", "try", "allOf", "anyOf"};
        std::vector<std::string> words = splitString(program, "(), :\n\t");
        bool htnSyntax = false;
        for(const std::string& word : words){
            if (std::find(htnKeywords.begin(), htnKeywords.end(), word) != htnKeywords.end())
            {
                std::cout << "Found HTN keyword " << word << std::endl;
                htnSyntax = true;
                break;
            } 
            if(htnSyntax){
                break;
            }
        }
        return htnSyntax;
    }


    // Compile *adds* whatever is passed into the current state of the database
    // This method attempts to infer what type of program this is
    __declspec(dllexport) char* __stdcall Compile(HtnPlannerPythonWrapper* ptr, const char *data)
     {


        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string programString = string(data);

            bool customVariables = hasCustomVariables(programString);
            bool htnSyntax = hasHtnKeywords(programString);
            std::cout << "Variables are of type:" << (customVariables ? "?var" : "Var") << " and syntax is: " << (htnSyntax ? "Prolog" : "HTN") << std::endl;

            if(!htnSyntax && customVariables){
                if (!ptr->m_prologCompilerCustomVariables->Compile(programString))
                {
                    return GetCharPtrFromString(ptr->m_prologCompilerCustomVariables->GetErrorString());
                }
            }
            else if(!htnSyntax && !customVariables){
                if (!ptr->m_prologCompiler->Compile(programString))
                {
                    return GetCharPtrFromString(ptr->m_prologCompiler->GetErrorString());
                }
            }
            else if(htnSyntax && customVariables){
                if (!ptr->m_htnCompilerCustomVariables->Compile(programString))
                {
                    return GetCharPtrFromString(ptr->m_htnCompilerCustomVariables->GetErrorString());
                }
            }
            else if(htnSyntax && !customVariables){
                if (!ptr->m_htnCompiler->Compile(programString))
                {
                    return GetCharPtrFromString(ptr->m_htnCompiler->GetErrorString());
                }
            }
            // Successful compilation
            return nullptr;
            
        }
        catch (runtime_error &error)
        {
            return GetCharPtrFromString(error.what());
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
            if (!ptr->m_htnCompiler->Compile(programString))
            {
                return GetCharPtrFromString(ptr->m_htnCompiler->GetErrorString());
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

    // Compile *adds* whatever is passed into the current state of the database
    __declspec(dllexport) char* __stdcall HtnCompileCustomVariables(HtnPlannerPythonWrapper* ptr, const char *data)
     {
        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string programString = string(data);
            if (!ptr->m_htnCompilerCustomVariables->Compile(programString))
            {
                return GetCharPtrFromString(ptr->m_htnCompilerCustomVariables->GetErrorString());
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
            shared_ptr<PrologStandardQueryCompiler> queryCompiler = shared_ptr<PrologStandardQueryCompiler>(new PrologStandardQueryCompiler(ptr->m_factory.get()));

            if (queryCompiler->Compile(queryString))
            {
                int64_t highestMemoryUsedReturn;
                int furthestFailureIndex;
                std::vector<std::shared_ptr<HtnTerm>> farthestFailureContext;

                ptr->m_lastSolutions = ptr->m_planner->FindAllPlans(ptr->m_factory.get(),
                                                                    ptr->m_state,
                                                                    queryCompiler->result(),
                                                                    (int) ptr->m_budgetBytes,
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
                else if(ptr->m_lastSolutions == nullptr)
                {
                    string failureContextString;
                    if(farthestFailureContext.size() > 0)
                    {
                        failureContextString = ", " + HtnTerm::ToString(farthestFailureContext, false, true);
                    }
                    *result = GetCharPtrFromString("[{\"false\" :[]}, {\"failureIndex\" :[{\"-1\" :[]}]}" + failureContextString + "]");
                }
                else
                {
                    *result = GetCharPtrFromString(HtnPlanner::ToStringSolutions(ptr->m_lastSolutions, true));
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
                return GetCharPtrFromString(ptr->m_prologCompiler->GetErrorString());
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

    // Compile *adds* whatever is passed into the current state of the database
    __declspec(dllexport) char* __stdcall PrologCompileCustomVariables(HtnPlannerPythonWrapper* ptr, const char* data)
    {
        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string programString = string(data);
            if (!ptr->m_prologCompilerCustomVariables->Compile(programString))
            {
                return GetCharPtrFromString(ptr->m_prologCompilerCustomVariables->GetErrorString());
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

    __declspec(dllexport) char* __stdcall PrologQueryToJson(HtnPlannerPythonWrapper* ptr, char* queryChars, char** result)
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
                auto queryResult = queryCompiler->result();
                
                if(queryResult.size() == 0)
                {
                    *result = GetCharPtrFromString("[{\"false\" :[]}]");
                }
                else
                {
                    *result = GetCharPtrFromString(HtnTerm::ToString(queryResult, false, true));
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


    char* CompileQueryResult(HtnPlannerPythonWrapper* ptr, const std::string& queryString, bool& compileQuerySuccess, std::vector<shared_ptr<HtnTerm>>& compileQueryResult, char** result){
        bool customVariables = hasCustomVariables(queryString);
        std::cout << "This is a " << (customVariables?"htn":"prolog") << " syntax query" << std::endl;
        if(hasCustomVariables(queryString)){
            shared_ptr<PrologQueryCompiler> queryCompiler = shared_ptr<PrologQueryCompiler>(new PrologQueryCompiler(ptr->m_factory.get()));
            if(queryCompiler->Compile(queryString)){
                compileQuerySuccess = true;
                compileQueryResult = queryCompiler->result();
            }
            else
            {
                compileQuerySuccess = false;
                *result = nullptr;
                return GetCharPtrFromString(queryCompiler->GetErrorString());
            }
        }
        else{
            shared_ptr<PrologStandardQueryCompiler> queryCompiler = shared_ptr<PrologStandardQueryCompiler>(new PrologStandardQueryCompiler(ptr->m_factory.get()));
            if(queryCompiler->Compile(queryString)){
                compileQuerySuccess = true;
                compileQueryResult = queryCompiler->result();
            }
            else
            {
                compileQuerySuccess = false;
                *result = nullptr;
                return GetCharPtrFromString(queryCompiler->GetErrorString());
            }
        }

        return nullptr;
    }

    // returns result in Json format
    // if the query failed, it will be a False term
    __declspec(dllexport) char* __stdcall PrologQuery(HtnPlannerPythonWrapper* ptr, char* queryChars, char** result)
    {
        // Catch any FailFasts and return their description
        TreatFailFastAsException(true);
        try
        {
            string queryString = string(queryChars);

            bool compileQuerySuccess;
            std::vector<shared_ptr<HtnTerm>> compileQueryResult;
            char* errorMessage = CompileQueryResult(ptr, queryString, compileQuerySuccess, compileQueryResult, result);
            
            if(!compileQuerySuccess){
                *result = nullptr;
                return errorMessage;
            }
            
            

            // The resolver can give one answer at a time using ResolveNext(), or just get them all using ResolveAll()
            int64_t highestMemoryUsedReturn;
            int furthestFailureIndex;
            std::vector<std::shared_ptr<HtnTerm>> farthestFailureContext;
            shared_ptr<vector<UnifierType>> queryResult = ptr->m_resolver->ResolveAll(ptr->m_factory.get(),
                                                                                        ptr->m_state.get(),
                                                                                        compileQueryResult,
                                                                                        0,
                                                                                        (int) ptr->m_budgetBytes,
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
                *result = GetCharPtrFromString("[{\"false\" :[]}, {\"failureIndex\" :[{\"" + lexical_cast<string>(furthestFailureIndex) + "\" :[]}]}" + failureContextString + "]");
            }
            else
            {
                *result = GetCharPtrFromString(HtnGoalResolver::ToString(queryResult.get(), true));
            }

            return nullptr;

        }
        catch (runtime_error & error)
        {
            *result = nullptr;
            return GetCharPtrFromString(error.what());
        }
    }
} //End C linkage scope.
