#ifndef ABSSYNTREE_H
#define ABSSYNTREE_H
/////////////////////////////////////////////////////////////////////////////
// AbsSynTree.h - Abstract Syntax Tree for holding parsed nodes            //
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
* Required Files:
* ===============
* AbsSynTree.h, AbsSynTree.cpp, ActionsAndRules.h
*
* Maintenance History:
* ====================
* ver 1.0 : 13 Mar 16
* - first release
*/


#include <vector>
#include <iostream>
#include "ActionsAndRules.h"

class AbsSynTree {
public:
	AbsSynTree::AbsSynTree() {
		_root.name() = "root";
		_root.type() = "unknown";
		_root.startLine() = 0;
		_root.endLine() = 0;
	}
	void TreeWalk(astNode pItem)
	{
		std::vector<astNode> Children = pItem.children();
		static size_t indentLevel = 0;
		std::cout << "\n  " << std::string(2 * indentLevel, '    ') << pItem.show();
		std::vector<astNode>::iterator iter;
		++indentLevel;
		for (iter = Children.begin(); iter != Children.end(); ++iter)
		{
			TreeWalk(*iter);
			//++iter;
		}
		--indentLevel;
	}
	void AbsSynTree::addChildren(astNode child) { _root.addChild(child); }
	astNode AbsSynTree::root() { return _root; }
	std::vector<astNode> AbsSynTree::children(astNode root) { return root.children(); }

	std::vector<astNode> AbsSynTree::findByType(const std::string& Type)
	{
		std::vector<astNode> temp = _root.findByType(Type);
		if (_root.name() == Type)
			temp.insert(begin(temp), _root);
		return temp;
	}
	std::vector<astNode> AbsSynTree::findByName(const std::string& Name)
	{
		std::vector<astNode> temp = _root.findByName(Name);
		if (_root.name() == Name)
			temp.insert(begin(temp), _root);
		return temp;
	}
private:
	astNode _root;
	std::vector<astNode> _children;
};
#endif