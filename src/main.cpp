#include <cctype>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "FXPlatform/Directory.h"
#include "FXPlatform/Htn/HtnCompiler.h"
#include "FXPlatform/Htn/HtnPlanner.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"
#include "FXPlatform/Prolog/PrologQueryCompiler.h"

int main (int argc, char *argv[])
{
//    // All traces on
//    SetTraceFilter((int) SystemTraceType::Solver | (int) SystemTraceType::Planner, TraceDetail::Diagnostic);
    
    // Turn off all tracing
//    SetTraceFilter(SystemTraceType::None, TraceDetail::Normal);

	if(argc == 1)
	{
		  fprintf(stdout, 
		  	"IndHtn example from https://github.com/EricZinda/InductorHtn. \r\n"
		  	"Pass the name of one or more Hierarchical Task Network or Prolog documents on the command line and then execute prolog queries or HTN goals interactively.\r\n"
            "Files with '.pl' exensions are parsed as Prolog and '.htn' documents are parsed as HTN.\r\n"
		  	"Example of executing normal Prolog query: \r\n"
		  	"	indprolog Taxi.htn \r\n"
            "\r\n"
		  	"	?- at(?where).           << You type that\r\n"
		  	"	>> ((?where = downtown)) << System returns value\r\n"
            "\r\n"
            "Example of solving HTN goal: \r\n"
            "   indprolog Taxi.htn \r\n"
            "\r\n"
            "   ?- goals(travel-to(suburb)).                                                                << You type that\r\n"
            "   >> [ { (wait-for(bus3,downtown), set-cash(12,11.000000000), ride(bus3,downtown,suburb)) } ] << System returns solutions\r\n"
            "\r\n"
			);
	}
	else
	{
		// IndProlog uses a factory model for creating terms so it can "intern" them to save memory.  You must never
		// mix terms from different HtnTermFactorys
        shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());

        // HtnRuleSet is where the facts and rules are stored for the program.  It is the database.
        shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());

        // HtnPlanner is a subclass of HtnDomain which stores the Operators and Methods, as well as having
        // the code that implements the HTN algorithm
        shared_ptr<HtnPlanner> planner = shared_ptr<HtnPlanner>(new HtnPlanner());

        // The HtnCompiler will uses the standard Prolog syntax *except* that variables start with ? and capitalization doesn't
        // mean anything special
    	HtnCompiler compiler(factory.get(), state.get(), planner.get());

        // PrologStandardCompiler uses all of the Prolog standard syntax
        PrologStandardCompiler prologCompiler(factory.get(), state.get());

        for(int argIndex = 1; argIndex < argc; ++argIndex)
        {
            string pathAndFile(argv[argIndex]);
            string path;
            string fileWithoutExtension;
            string extension;
            Directory::SplitPath(pathAndFile, path, fileWithoutExtension, extension);

            if(extension == "pl")
            {
                if(prologCompiler.CompileDocument(pathAndFile))
                {
                    fprintf(stdout, "Succesfully compiled %s as standard Prolog document\r\n", pathAndFile.c_str());
                }
                else
                {
                    fprintf(stdout, "Error compiling %s as standard Prolog document, %s\r\n", pathAndFile.c_str(), prologCompiler.GetErrorString().c_str());
                    return 1;
                }
            }
            else if(extension == "htn")
            {
                if(compiler.CompileDocument(pathAndFile))
                {
                    fprintf(stdout, "Succesfully compiled %s as Htn document\r\n", pathAndFile.c_str());
                }
                else
                {
                    fprintf(stdout, "Error compiling %s as Htn document, %s\r\n", pathAndFile.c_str(), compiler.GetErrorString().c_str());
                    return 1;
                }
            }
        }

		fprintf(stdout, "\r\nType a Prolog query (preceeding variables with '?') or \r\nprocess a goal using 'goal(...terms...)' or \r\nhit q to end.\r\n\r\n");
		fprintf(stdout, "?- ");

        // The PrologQueryCompiler will compile prolog queries using the normal Prolog parsing rules *except* that
        // variables start with ? and capitalization doesn't mean anything special
        PrologQueryCompiler queryCompiler(factory.get());

        // HtnGoalResolver is the Prolog "engine" that resolves queries
        HtnGoalResolver resolver;
        
        string input;
        while(true)
        {
            input.clear();
            string tempInput;
            std::getline(cin, tempInput);
            for(auto c : tempInput)
            {
                // Xcode console window will pass along all backspace, arrow keys, etc instead of just the resulting text
                // get rid of those.
                // NOTE: Xcode still insists on autocompleting the right parenthesis after you type something like "a(b"
                // but DOES NOT pass along the second parenthesis to cin.  So, you have to type it twice when using Xcode.
                if(c >= 0x20 && c <= 0x7E)
                {
                    input.push_back(c);
                }
            }

            if (input == "q") break;
//                cout << "received: " << input << endl;
            if(queryCompiler.Compile(input))
            {
                if(queryCompiler.result().size() == 1 && queryCompiler.result()[0]->name() == "goals")
                {
                    shared_ptr<HtnPlanner::SolutionsType> solutions = planner->FindAllPlans(factory.get(), state, queryCompiler.result()[0]->arguments());
                    fprintf(stdout, ">> %s\r\n\r\n", HtnPlanner::ToStringSolutions(solutions).c_str());
                }
                else
                {
                    // The resolver can give one answer at a time using ResolveNext(), or just get them all using ResolveAll()
                    shared_ptr<vector<UnifierType>> queryResult = resolver.ResolveAll(factory.get(), state.get(), queryCompiler.result());
                    if (queryResult == nullptr)
                    {
                        fprintf(stdout, ">> false\r\n\r\n");
                    }
                    else
                    {
                        fprintf(stdout, ">> %s\r\n\r\n", HtnGoalResolver::ToString(queryResult.get()).c_str());
                    }
                }
            }
            else
            {
                fprintf(stdout, "Error: %s\r\n\r\n", queryCompiler.GetErrorString().c_str());
            }

            queryCompiler.Clear();
            fprintf(stdout, "?- ");
        }
		return 0;
	}
}
