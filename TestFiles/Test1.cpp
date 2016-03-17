/////////////////////////////////////////////////////////////////////////////
// MetricsAnalysis.cpp - Calculate metrics for a file		                   //
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
* This module will calculate the metrics of a file supplied
* to it. Metrics involve calculating line size and cyclocmatic
* complexities.
* For the purpose of this project, this module only calculates
* metrics for functions, but can be easily extended to search
* for other types.
*
* Build command:
* ==============
* cl MetricsAnalysis.cpp /TEST_METRICSANALYSIS filePath
*
* Required Files:
* ===============
* MetricsAnalysis.h, ActionsAndRules.h, Utilities.h,
* AbsSynTree.h
*
* Maintenance History:
* ====================
* ver 1.0 : 13 Mar 16
* - first release
*/

#include <iostream>
#include "MetricsAnalysis.h"
#include "../Utilities/Utilities.h"

using namespace Scanner;
using namespace Utilities;
using Pair = std::pair<std::string, int>;
using Map = std::vector<Pair>;
using Util = Utilities::StringHelper;

#ifdef TEST_METRICSANALYSIS

int main() {
	std::cout << "\n";
	Util::Title("Testing MetricsAnalysis packge", '=');
	astNode root;
	root.name() = "root";
	root.type() = "function";
	astNode child;
	child.name() = "first child";
	child.type() = "function";
	astNode grandchild;
	grandchild.name() = "first child's child";
	grandchild.type() = "function";
	child.addChild(grandchild);
	root.addChild(child);
	astNode child2;
	child2.name() = "second child";
	child2.type() = "function";
	root.addChild(child2);

	Util::Title("Display Tree");
	AbsSynTree * tree(new AbsSynTree());
	tree->addChildren(root);
	tree->TreeWalk(root);
	std::cout << "\n";

	Util::Title("Show Metrics");
	MetricsAnalysis ma;
	ma.calcMetrics(tree->root());
	Map lines = ma.lines();
	Map complex = ma.complexity();
	Map::iterator iterL;
	Map::iterator iterC;

	Util::Title("Function\tLines\t\tComplexity");
	for (iterL = lines.begin(), iterC = complex.begin(); iterL != lines.end(), iterC != complex.end(); ++iterL, ++iterC)
		std::cout << "\n\n  " << (*iterL).first << "\tLines: " << (*iterL).second << "\tComplexity: " << (*iterC).second;

	std::cout << "\n\n";
	return 0;
}
#endif