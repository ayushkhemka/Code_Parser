/////////////////////////////////////////////////////////////////////////////
// AbsSynTree.cpp - Abstract Syntax Tree for holding parsed nodes          //
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
* This module contains the class to hold the tree of all
* the parsed components of a file. It also contains the
* functions required, like finding by type or name.
* The TreeWalk function walks the whole tree, displaying
* the nodes of the tree in a DFS fashion.
*
* Buid command:
* =============
* cl AbsSynTree.cpp /TEST_ABSSYNTREE
*
* Required Files:
* ===============
* AbsSynTree.h, AbsSynTree.cpp, Utilities.h
*
* Maintenance History:
* ====================
* ver 1.0 : 13 Mar 16
* - first release
*/


#include <vector>
#include <iostream>
#include "AbsSynTree.h"
#include "../Utilities/Utilities.h"
using namespace Utilities;
using Util = Utilities::StringHelper;

#ifdef TEST_ABSSYNTREE
int main() {
	Util::Title("Testing MetricsAnalysis packge", '=');
	astNode root;
	root.name() = "root";
	root.type() = "function";
	astNode child;
	child.name() = "child";
	child.type() = "function";
	astNode grandchild;
	grandchild.name() = "first child's child";
	grandchild.type() = "function";
	child.addChild(grandchild);
	root.addChild(child);
	astNode child2;
	child2.name() = "child";
	child2.type() = "function";
	root.addChild(child2);

	Util::Title("Display Tree");
	AbsSynTree * tree(new AbsSynTree());
	tree->addChildren(root);
	tree->TreeWalk(root);
	std::cout << "\n";

	Util::Title("Finding by Type Function");
	std::vector<astNode> nodes = root.findByType("function");
	for (auto node_ : nodes)
		std::cout << "\n" << node_.show();

	std::cout << "\n";
	Util::Title("Finding by Name child");
	nodes.clear();
	nodes = root.findByName("child");
	for (auto node_ : nodes)
		std::cout << "\n" << node_.show();

	std::cout << "\n\n";
	return 0;
}
#endif