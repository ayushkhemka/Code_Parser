/////////////////////////////////////////////////////////////////////////////
// MetricsExecutive.cpp - Test Executive for Code Parser                   //
// ver 1.0                                                                 //
// Language:    Visual C++, Visual Studio 2015                             //
// Platform:    Lenovo g505s, AMD A8, Windows 10		                       //
// Application: CSE 687 Project #2, Spring 2016                            //
// Author:      Ayush Khemka, Syracuse University													 //
//              aykhemka@syr.edu, 538044584					                       //
/////////////////////////////////////////////////////////////////////////////
/*
* Module Operations:
* ==================
* This program tests the parser application making use of the packages
* provided. Specific requirements of Project #2 have been successfully
* demonstrated. The Tokenizer and SemiExpression packages are used to
* get semi expressions from the file. The Parser package then parses
* those files and taking help from the AbsSynTree, builds the tree.
* The tree can then be used to calculate metrics such as function
* size and cyclomatic complexities.
* The FileMgr is used to collect a set of files from a specified
* directory, following the specific patterns.
*
* Required Files:
* ===============
* MetricsExecutive.cpp, FileMgr.h, ConfigureParser.h,
* ActionsAndRules.h, Parser.h, SemiExp.h, Tokenizer.h,
* Utilities.h, DataStore.h
*
* Build Command:
* ==============
* cl MetricsExecutive.cpp /TEST_METRICSEXECUTIVE DirectoryPath Patterns
*
* Maintenance History:
* ====================
* ver 1.0 : 13 Mar 16
* - first release
*/

#include <iostream>
#include <string>
#include <vector>
#include "../Parser/Parser.h"
#include "../Parser/ConfigureParser.h"
#include "../Parser/ActionsAndRules.h"
#include "../Parser/AbsSynTree.h"
#include "../SemiExp/SemiExp.h"
#include "../Tokenizer/Tokenizer.h"
#include "../FileMgr/FileMgr.h"
#include "../DataStore/DataStore.h"
#include "../MetricsAnalysis/MetricsAnalysis.h"
#include "../Utilities/Utilities.h"
#define Util StringHelper

using namespace Scanner;
using namespace FileSystem;
using namespace Utilities;

#ifdef TEST_METRICSEXECUTIVE

class AbsSynTree;
Repository* Repository::pRepo;
int main(int argc, char * argv[]) {

	// -------------< search for specified files and store >--------
	AbsSynTree * tree(new AbsSynTree());
	DataStore ds;
	if (argc == 1) {
		std::cout << "No arguments enetered!\n\n";
		return 1;
	}
	FileMgr fm(argv[1], ds);
	for (int i = 2; i < argc; i++)
		fm.addPattern(argv[i]);
	fm.search();

	Util::Title("Displaying requirement #7", '=');
	std::cout << "\n\n  Processing these files";
	std::cout << "\n--------------------------\n";
	for (auto fs : ds) std::cout << "\n  " << fs;

	std::cout << "\n";
	// ---------< One by one parse the files >---------
	Util::Title("Displaying requirements #4 and #5", '=');
	for (auto fs : ds) {
		std::string fileSpec = Path::getFullFileSpec(fs);
		ConfigParseToConsole configure;
		Parser* pParser = configure.Build();
		try
		{
			if (pParser)
			{
				if (!configure.Attach(fileSpec))
				{
					std::cout << "\n  could not open file " << fileSpec << std::endl;
					continue;
				}
			}
			else
			{
				std::cout << "\n\n  Parser not built\n\n";
				return 1;
			}
			// now that parser is built, use it
			std::cout << "\n  Parsing file " << fs << ":\n";
			std::cout << "\n---------------" << std::string(fs.length(), '-') << "-----\n";
			while (pParser->next())
				pParser->parse();
			Repository* p_Repo = Repository::getInstance();
			astNode elem = p_Repo->scopeStack().top();
			tree->addChildren(elem);
			std::cout << "\n\n";
		}
		catch (std::exception& ex)
		{
			std::cout << "\n\n    " << ex.what() << "\n\n";
		}
	}

	Util::Title("Displaying requirement #6", '=');
	Util::Title("Showing Abstract Syntax Tree");
	tree->TreeWalk(tree->root());

	MetricsAnalysis ma;
	using Pair = std::pair<std::string, int>;
	using Map = std::vector<Pair>;
	ma.calcMetrics(tree->root());
	Map lines = ma.lines();
	Map complex = ma.complexity();
	Map::iterator iterL;
	Map::iterator iterC;

	std::cout << "\n";
	Util::Title("Displaying requirements #8 and #9", '=');
	Util::Title("\n  Metric Analysis of the files, showing only functions");
	Util::Title("Function\tLines\t\tComplexity");
	for (iterL = lines.begin(), iterC = complex.begin(); iterL != lines.end(), iterC != complex.end(); ++iterL, ++iterC)
		std::cout << "\n\n  " << (*iterL).first << "\tLines: " << (*iterL).second << "\tComplexity: " << (*iterC).second;

	std::cout << "\n\n";
	Util::Title("Requirements #3 and #10 can be verified by looking at the code");
	std::cout << "\n\n";
	
	delete tree;
	
	return 0;
}
#endif