/////////////////////////////////////////////////////////////////////
//  ConfigureParser.cpp - builds and configures parsers            //
//  ver 2.2                                                        //
//                                                                 //
//  Lanaguage:     Visual C++ 2005                                 //
//  Platform:      Dell Dimension 9150, Windows XP SP2             //
//  Application:   Prototype for CSE687 Pr1, Sp06                  //
//  Source:        Jim Fawcett, CST 2-187, Syracuse University     //
//                 (315) 443-3948, jfawcett@twcny.rr.com           //
//  Author:				 Ayush Khemka, Syracuse University							 //
/////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This module builds and configures parsers.  It builds the parser
parts and configures them with application specific rules and actions.

Public Interface:
=================
ConfigParseToConsole conConfig;
conConfig.Build();
conConfig.Attach(someFileName);

Build Process:
==============
Required files
- ConfigureParser.h, ConfigureParser.cpp, Parser.h, Parser.cpp,
ActionsAndRules.h, ActionsAndRules.cpp,
SemiExpression.h, SemiExpression.cpp, tokenizer.h, tokenizer.cpp
Build commands (either one)
- devenv Project1HelpS06.sln
- cl /EHsc /DTEST_PARSER ConfigureParser.cpp parser.cpp \
ActionsAndRules.cpp \
semiexpression.cpp tokenizer.cpp /link setargv.obj

Maintenance History:
====================
ver 2.2 : 13 Mar 2016
- Added/Removed objects according to changes in ActionsAndRules
- Added testing for creating tree and calculating metrics
ver 2.1 : 19 Feb 16
- Added PrintFunction action to FunctionDefinition rule
ver 2.0 : 01 Jun 11
- Major revisions to begin building a strong code analyzer
ver 1.1 : 01 Feb 06
- cosmetic changes to ConfigureParser.cpp
ver 1.0 : 12 Jan 06
- first release

*/

#include <fstream>
#include <functional>
#include "Parser.h"
#include "../SemiExp/SemiExp.h"
#include "../Tokenizer/Tokenizer.h"
#include "ActionsAndRules.h"
#include "ConfigureParser.h"
#include "AbsSynTree.h"
#include "../MetricsAnalysis/MetricsAnalysis.h"

using namespace Scanner;

//----< destructor releases all parts >------------------------------

ConfigParseToConsole::~ConfigParseToConsole()
{
  // when Builder goes out of scope, everything must be deallocated

  delete pHandlePush;
  delete pBeginningOfScope;
  delete pHandlePop;
  delete pEndOfScope;
  delete pPushFunction;
  delete pFunctionDefinition;
  delete pConditionScope;
	delete pPushCondition;
	delete pClassDefinition;
	delete pPushClass;
	delete pNSDefinition;
	delete pControlScope;
	delete pPushControl;
	delete pPushNS;
	delete pArrayDefinition;
	delete pPushArray;
  delete pRepo;
  delete pParser;
  delete pSemi;
  delete pToker;
  pIn->close();
  delete pIn;
}
//----< attach toker to a file stream or stringstream >------------

bool ConfigParseToConsole::Attach(const std::string& name, bool isFile)
{
  if(pToker == 0)
    return false;
  pIn = new std::ifstream(name);
  if (!pIn->good())
    return false;
  return pToker->attach(pIn);
}
//----< Here's where alll the parts get assembled >----------------

Parser* ConfigParseToConsole::Build()
{
  try
  {
    // add Parser's main parts

    pToker = new Toker;
    pToker->returnComments(false);
    pSemi = new SemiExp(pToker);
    pParser = new Parser(pSemi);
    pRepo = new Repository(pToker);

    // configure to manage scope
    // these must come first - they return true on match
    // so rule checking continues

    pBeginningOfScope = new BeginningOfScope();
    pHandlePush = new HandlePush(pRepo);
    pBeginningOfScope->addAction(pHandlePush);
    pParser->addRule(pBeginningOfScope);
    pEndOfScope = new EndOfScope();
    pHandlePop = new HandlePop(pRepo);
    pEndOfScope->addAction(pHandlePop);
    pParser->addRule(pEndOfScope);
		pEndofClass = new EndOfClass();
		pParser->addRule(pEndofClass);

    // configure to detect and act on function definitions
    // these will stop further rule checking by returning false

    pFunctionDefinition = new FunctionDefinition;
    pPushFunction = new PushFunction(pRepo);  // no action
    pFunctionDefinition->addAction(pPushFunction);
    pParser->addRule(pFunctionDefinition);

		// configure to detect and act on conditions

		pConditionScope = new ConditionScope;
		pPushCondition = new PushCondition(pRepo);  // no action
		pConditionScope->addAction(pPushCondition);
		pParser->addRule(pConditionScope);
		
		// configure to detect and act on control statements

		pControlScope = new ControlScope;
		pPushControl = new PushControl(pRepo);
		pControlScope->addAction(pPushControl);
		pParser->addRule(pControlScope);

		// configure to detect and act on classes, structs, and unions

		pClassDefinition = new ClassDefinition;
		pPushClass = new PushClass(pRepo);
		pClassDefinition->addAction(pPushClass);
		pParser->addRule(pClassDefinition);

		// configure to detect and act on namespaces

		pNSDefinition = new NSDefinition;
		pPushNS = new PushNS(pRepo);
		pNSDefinition->addAction(pPushNS);
		pParser->addRule(pNSDefinition);

		// configure to detect and act on arrays

		pArrayDefinition = new ArrayDefinition;
		pPushArray = new PushArray(pRepo);
		pArrayDefinition->addAction(pPushArray);
		pParser->addRule(pArrayDefinition);

		// configure to detect and act on lambdas

		pLambdaDefinition = new LambdaDefinition;
		pPushLambda = new PushLambda(pRepo);
		pLambdaDefinition->addAction(pPushLambda);
		pParser->addRule(pLambdaDefinition);
		
		return pParser;
  }
  catch(std::exception& ex)
  {
    std::cout << "\n\n  " << ex.what() << "\n\n";
    return 0;
  }
}
#ifdef TEST_CONFIGUREPARSER

#include <queue>
#include <string>
Repository *Repository::pRepo;
int main(int argc, char* argv[])
{
  std::cout << "\n  Testing ConfigureParser module\n "
            << std::string(32,'=') << std::endl;

  // collecting tokens from files, named on the command line

	if(argc < 2)
  {
    std::cout 
      << "\n  please enter name of file to process on command line\n\n";
    return 1;
  }

  for(int i=1; i<argc; ++i)
  {
    std::cout << "\n  Processing file " << argv[i];
    std::cout << "\n  " << std::string(16 + strlen(argv[i]),'-');

    ConfigParseToConsole configure;
    Parser* pParser = configure.Build();
    try
    {
      if(pParser)
      {
        if(!configure.Attach(argv[i]))
        {
          std::cout << "\n  could not open file " << argv[i] << std::endl;
          continue;
        }
      }
      else
      {
        std::cout << "\n\n  Parser not built\n\n";
        return 1;
      }
      // now that parser is built, use it

      while(pParser->next())
        pParser->parse();
      std::cout << "\n\n";

			Repository* p_Repo = Repository::getInstance();
			astNode elem = p_Repo->scopeStack().top();
			AbsSynTree * tree(new AbsSynTree());
			tree->addChildren(elem);
			tree->TreeWalk(tree->root());

			int count = tree->root().DFS(tree->root().children()[0]);
			std::cout << "\n\n  Number of children of global namespace: " << count;

			MetricsAnalysis ma;
			using Pair = std::pair<std::string, int>;
			using Map = std::vector<Pair>;
			ma.calcMetrics(tree->root().children()[0]);
			Map lines = ma.lines();
			Map complex = ma.complexity();
			Map::iterator iterL;
			Map::iterator iterC;
			for (iterL = lines.begin(), iterC = complex.begin(); iterL != lines.end(), iterC != complex.end(); ++iterL, ++iterC)
				std::cout << "\n\n  " << (*iterL).first << ", Lines: " << (*iterL).second << ", Complexity: " << (*iterC).second;
    }
    catch(std::exception& ex)
    {
      std::cout << "\n\n    " << ex.what() << "\n\n";
    }
    std::cout << "\n\n";
  }
}

#endif
