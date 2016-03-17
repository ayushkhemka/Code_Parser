#ifndef METRICSANALYSIS_H
#define METRICSANALYSIS_H
/////////////////////////////////////////////////////////////////////////////
// MetricsAnalysis.h - Calculate metrics for a file		                     //
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
* Required Files:
* ===============
* MetricsAnalysis.h, ActionsAndRules.h
*
* Maintenance History:
* ====================
* ver 1.0 : 13 Mar 16
* - first release
*/

#include <iostream>
#include <vector>
#include <string>
#include "../Parser/ActionsAndRules.h"
#include "../Parser/AbsSynTree.h"

using Pair = std::pair<std::string, int>;
using Map = std::vector<Pair>;

class MetricsAnalysis {
public:
	Map lines() { return lineSize; }
	Map complexity() { return complex; }

	// -----------< helper function for metrics calculation >--------
	void calc(astNode& node) {
		std::string Key = std::string(node.name());
		int Value = node.endLine() - node.startLine() + 1;
		Pair pushLine;
		pushLine.first = Key;
		pushLine.second = Value;
		lineSize.push_back(pushLine);

		Pair pushComplex;
		pushComplex.first = Key;
		pushComplex.second = node.DFS(node);
		complex.push_back(pushComplex);
		for (auto child : node.children())
			calcMetrics(child);
	}

	// ------------< find nodes with type function and calculate metrics >---------
	void calcMetrics(astNode& node) {
		std::vector<astNode> nodes = node.findByType("function");
		for (auto node_ : nodes)
			calc(node_);
	}
private:
	Map lineSize;
	Map complex;
};
#endif