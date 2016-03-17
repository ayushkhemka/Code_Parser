#ifndef ACTIONSANDRULES_H
#define ACTIONSANDRULES_H
/////////////////////////////////////////////////////////////////////
//  ActionsAndRules.h - declares new parsing rules and actions     //
//  ver 2.2                                                        //
//  Language:      Visual C++ 2008, SP1                            //
//  Platform:      Dell Precision T7400, Vista Ultimate SP1        //
//  Application:   Prototype for CSE687 Pr1, Sp09                  //
//  Source:        Jim Fawcett, CST 4-187, Syracuse University     //
//                 (315) 443-3948, jfawcett@twcny.rr.com           //
//  Author:				 Ayush Khemka, Syracuse University							 //
/////////////////////////////////////////////////////////////////////
/*
  Module Operations: 
  ==================
  This module defines several action classes.  Its classes provide 
  specialized services needed for specific applications.  The modules
  Parser, SemiExpression, and Tokenizer, are intended to be reusable
  without change.  This module provides a place to put extensions of
  these facilities and is not expected to be reusable. 

  Public Interface:
  =================
  Toker t(someFile);              // create tokenizer instance
  SemiExp se(&t);                 // create a SemiExp attached to tokenizer
  Parser parser(se);              // now we have a parser
  Rule1 r1;                       // create instance of a derived Rule class
  Action1 a1;                     // create a derived action
  r1.addAction(&a1);              // register action with the rule
  parser.addRule(&r1);            // register rule with parser
  while(se.getSemiExp())          // get semi-expression
    parser.parse();               //   and parse it

  Build Process:
  ==============
  Required files
    - Parser.h, Parser.cpp, ScopeStack.h, ScopeStack.cpp,
      ActionsAndRules.h, ActionsAndRules.cpp, ConfigureParser.cpp,
      ItokCollection.h, SemiExpression.h, SemiExpression.cpp, tokenizer.h, tokenizer.cpp
  Build commands (either one)
    - devenv CodeAnalysis.sln
    - cl /EHsc /DTEST_PARSER parser.cpp ActionsAndRules.cpp \
         semiexpression.cpp tokenizer.cpp /link setargv.obj

  Maintenance History:
  ====================
	ver 2.2 : 13 Mar 16
	- final release
	- added rules and actions to process classes, namespaces, lambdas,
	  structs, and condition scopes
	- removed rules and actions to process declarations and expressions
	- removed classes to print functions, declarations, and expressions
  ver 2.1 : 15 Feb 16
  - small functional change to a few of the actions changes display strategy
  - preface the (new) Toker and SemiExp with Scanner namespace
  ver 2.0 : 01 Jun 11
  - added processing on way to building strong code analyzer
  ver 1.1 : 17 Jan 09
  - changed to accept a pointer to interfaced ITokCollection instead
    of a SemiExpression
  ver 1.0 : 12 Jan 06
  - first release

*/

#include <queue>
#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include "Parser.h"
#include "../SemiExp/itokcollection.h"
#include "../ScopeStack/ScopeStack.h"
#include "../Tokenizer/Tokenizer.h"
#include "../SemiExp/SemiExp.h"

// ---------< astNode is application specific, changed from ScopeStack element >----------
class astNode
{
	using iterator = std::vector<astNode>::iterator;
private:
	std::string _type;
  std::string _name;
	std::vector<astNode> _children;
  size_t startLineCount = 0;
	size_t endLineCount = 0;
	std::vector<astNode> _foundTypes;
	std::vector<astNode> _foundNames;
public:
	// -------< default constructor >-------
	astNode()
	{
		_name = "";
		_type = "anonymous";
		startLineCount = 0;
		endLineCount = 0;
	}

	void push_back(astNode& elem) { _children.push_back(elem); }
	std::string& name() { return _name; }
	std::string& type() { return _type; }
	size_t& startLine() { return startLineCount; }
	size_t& endLine() { return endLineCount; }
	iterator begin() { return _children.begin(); }
	iterator end() { return _children.end(); }
	void addChild(astNode& node) { _children.push_back(node); }
	std::vector<astNode> children() { return _children; }

	astNode& operator[](size_t n)
	{
		if (n < 0 || _children.size() <= n)
			throw(std::exception("index out of range"));
		return _children[n];
	}

	astNode operator[](size_t n) const
	{
		if (n < 0 || _children.size() <= n)
			throw(std::exception("index out of range"));
		return _children[n];
	}

	void clear() { _children.clear(); }
	size_t size() { return _children.size(); }

	//----< search children for type helper >------------------------------------------
	void findByTypeHelper(astNode ptr, const std::string& Type)
	{
		if (ptr.type() == Type)
			_foundTypes.push_back(ptr);
		for (auto pChild : ptr.children())
			findByTypeHelper(pChild, Type);
	}

	//----< search children for type >------------------------------------------
	std::vector<astNode> findByType(const std::string& Type)
	{
		for (auto pChild : _children)
			findByTypeHelper(pChild, Type);
		std::vector<astNode> temp = std::move(_foundTypes);
		return temp;
	}

	//----< search children for name helper >------------------------------------------
	void findByNameHelper(astNode ptr, const std::string& Name)
	{
		if (ptr.name() == Name)
			_foundNames.push_back(ptr);
		for (auto pChild : ptr.children())
			findByNameHelper(pChild, Name);
	}

	//----< search children for name >------------------------------------------
	std::vector<astNode> findByName(const std::string& Name)
	{
		for (auto pChild : _children)
			findByNameHelper(pChild, Name);
		std::vector<astNode> temp = std::move(_foundNames);
		return temp;
	}

	// ---------< show element >-------
	std::string show() {
		std::ostringstream temp;
		temp << "(";
		temp << type();
		temp << ", ";
		temp << name();
		temp << ", ";
		temp << startLine();
		temp << ", ";
		temp << endLine();
		temp << ")";
		return temp.str();
	}

	// ----< Depth First Search to calculate number of children >------------------------
	int DFS(astNode& node, int count = 0)
	{
		for (auto& pChild : node) {
			++count;
			DFS(pChild, count);
		}
		return count;
	}
};

// ---------< Repository instance is used to share resources among all actions. >----------
class Repository  // application specific
{
  ScopeStack<astNode> stack;
  Scanner::Toker* p_Toker;
	static Repository* pRepo;
public:
	Repository(Scanner::Toker* pToker)
  {
    p_Toker = pToker;
		pRepo = this;
  }
	static Repository* getInstance() {
		return pRepo;
	}
  ScopeStack<astNode>& scopeStack()
  {
    return stack;
  }
  Scanner::Toker* Toker()
  {
    return p_Toker;
  }
  size_t lineCount()
  {
    return (size_t)(p_Toker->currentLineCount());
  }
};

//------< rule to detect beginning of anonymous scope >-------------
class BeginningOfScope : public IRule
{
public:
  bool doTest(ITokCollection*& pTc)
  {
    if(pTc->find("{") < pTc->length())
    {
      doActions(pTc);
      return true;
    }
    return true;
  }
};

//---------< action to handle scope stack at beginning of scope >------------
class HandlePush : public IAction
{
	Repository* p_Repos;
public:
	HandlePush(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		//std::cout << "\n--BeginningOfScope rule";
		if (p_Repos->scopeStack().size() == 0) {
			astNode root;
			root.type() = "namespace";
			root.name() = "global";
			root.startLine() = 1;
			p_Repos->scopeStack().push(root);
		}
		astNode elem;
		elem.type() = "unknown";
		elem.name() = "anonymous";
		elem.startLine() = p_Repos->lineCount() - 1;
		p_Repos->scopeStack().push(elem);
	}
};

//--------< rule to detect end of scope >---------
class EndOfScope : public IRule
{
public:
  bool doTest(ITokCollection*& pTc)
  {
    if(pTc->find("}") < pTc->length())
    {
      doActions(pTc);
      return true;
    }
    return true;
  }
};

//---------< rule to detect end of class or struct >-------
class EndOfClass : public IRule
{
private:
	int closeScope = 0;
public:
	bool doTest(ITokCollection*& pTc)
	{
		pTc->trimFront();
		if ((*pTc)[0] == "}") closeScope = 1;
		if (closeScope && (*pTc)[0] == ";")
		{
			closeScope = 0;
			doActions(pTc);
			return true;
		}
		return true;
	}
};

//---------< action to handle scope stack at end of scope >-------
class HandlePop : public IAction
{
  Repository* p_Repos;
	using iterator = std::vector<astNode>::iterator;
public:
	void TreeWalk(astNode pItem)
	{
		std::vector<astNode> Children = pItem.children();
		static size_t indentLevel = 0;
		std::cout << "\n  " << std::string(2 * indentLevel, ' ') << pItem.show();
		iterator iter;
		++indentLevel;
		for (iter = Children.begin(); iter != Children.end(); ++iter)
			TreeWalk(*iter);
		--indentLevel;
	}
  HandlePop(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
    //std::cout << "\n--EndOfScope rule";
		if (p_Repos->scopeStack().size() == 0)
			return;
		astNode elem = p_Repos->scopeStack().pop();
		elem.endLine() = p_Repos->lineCount() - 1;
		astNode prev = p_Repos->scopeStack().pop();
		if (p_Repos->scopeStack().size() == 0) prev.endLine() = p_Repos->lineCount() - 1;
		prev.addChild(elem);
		p_Repos->scopeStack().push(prev);
		
		// -----------< print >----------
		if(elem.type() == "function")
			std::cout << "\n  Function " << elem.name() << ", start line = " << elem.startLine() << ", end line = " << elem.endLine() << ", lines = " << elem.endLine() - elem.startLine() + 1;
		else if (elem.type() == "condition")
			std::cout << "\n  Condition " << elem.name() << ", start line = " << elem.startLine() << ", end line = " << elem.endLine() << ", lines = " << elem.endLine() - elem.startLine() + 1;
		else if (elem.type() == "class" || elem.type() == "struct" || elem.type() == "union")
			std::cout << "\n  " << elem.type() << " " << elem.name() << ", start line = " << elem.startLine() << ", end line = " << elem.endLine() << ", lines = " << elem.endLine() - elem.startLine() + 1;
		else if (elem.type() == "namespace")
			std::cout << "\n  Namespace " << elem.name() << ", start line = " << elem.startLine() << ", end line = " << elem.endLine() << ", lines = " << elem.endLine() - elem.startLine() + 1;
	}
};

//----------< rule to detect function definitions >----------
class FunctionDefinition : public IRule
{
public:
  bool isSpecialKeyWord(const std::string& tok)
  {
    const static std::string keys[]
      = { "for", "while", "switch", "if", "catch" };
    for(int i=0; i < 5; ++i)
      if(tok == keys[i])
        return true;
    return false;
  }
  bool doTest(ITokCollection*& pTc)
  {
    ITokCollection& tc = *pTc;
    if(tc[tc.length()-1] == "{")
    {
      size_t len = tc.find("(");
      if(len < tc.length() && !isSpecialKeyWord(tc[len - 1]))
      {
        doActions(pTc);
        return true;
      }
    }
    return true;
  }
};

//---------< action to push function name onto ScopeStack >------
class PushFunction : public IAction
{
  Repository* p_Repos;
public:
  PushFunction(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
    // next statement is now  handled in PrintFunction
    // std::cout << "\n  FunctionDef: " << pTc->show();

    // pop anonymous scope
    p_Repos->scopeStack().pop();

    // push function scope
    std::string name = (*pTc)[pTc->find("(") - 1];
    astNode elem;
    elem.type() = "function";
    elem.name() = name;
		elem.startLine() = p_Repos->lineCount() - 1;
		p_Repos->scopeStack().push(elem);
  }
};

// -----------< rule to detect namespace definitions >--------
class NSDefinition : public IRule
{
public:
	bool isNameSpace(const std::string& tok)
	{
		if (tok == "namespace")
			return true;
		return false;
	}
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			std::string name = tc[0];
			if (isNameSpace(name))
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};

// -----------< action to push namespace onto ScopeStack >-----------

class PushNS : public IAction
{
	Repository* p_Repos;
public:
	PushNS(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		// next statement is now  handled in PrintFunction
		// std::cout << "\n  FunctionDef: " << pTc->show();

		// pop anonymous scope
		p_Repos->scopeStack().pop();

		// push function scope
		std::string name = (*pTc)[1];
		astNode elem;
		elem.type() = (std::string) (*pTc)[0];
		elem.name() = name;
		elem.startLine() = p_Repos->lineCount() - 1;
		p_Repos->scopeStack().push(elem);
	}
};

// -----------< rule to detect array definitions >----------

class ArrayDefinition : public IRule
{
public:
	bool isArray(const std::string& tok)
	{
		const static std::string keys[]
			= { "int", "double", "long", "short", "byte", "float", "decimal", "bool", "char" };
		for (int i = 0; i < 9; ++i)
			if (tok == keys[i])
				return true;
		return false;
	}
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{" && tc[tc.length() - 2] == "]")
		{
			std::string name = (*pTc)[pTc->find("[") - 1];
			if (isArray(name))
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};

// ----------< action to push array onto ScopeStack >------------

class PushArray : public IAction
{
	Repository* p_Repos;
public:
	PushArray(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		// next statement is now  handled in PrintFunction
		// std::cout << "\n  FunctionDef: " << pTc->show();

		// pop anonymous scope
		p_Repos->scopeStack().pop();

		// push function scope
		std::string name = (*pTc)[pTc->find("[") - 1];
		astNode elem;
		elem.type() = "array";
		elem.name() = name;
		elem.startLine() = p_Repos->lineCount() - 1;
		p_Repos->scopeStack().push(elem);
	}
};

// -----------< rule to detect lambdas >----------
class LambdaDefinition : public IRule
{
public:
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			size_t len = pTc->find("[");
			if (len == 1 || tc[len - 1] == "=") {
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};

// ---------< action to push lambda onto ScopeStack >----------
class PushLambda : public IAction
{
	Repository* p_Repos;
public:
	PushLambda(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		// next statement is now  handled in PrintFunction
		// std::cout << "\n  FunctionDef: " << pTc->show();

		// pop anonymous scope
		p_Repos->scopeStack().pop();

		// push function scope
		astNode elem;
		elem.type() = "lambda";
		elem.name() = "anonymous";
		elem.startLine() = p_Repos->lineCount() - 1;
		p_Repos->scopeStack().push(elem);
	}
};
 
// --------< rule to detect classes, structs, and unions >---------

class ClassDefinition : public IRule
{
public:
	bool isClassKeyWord(const std::string& tok)
	{
		const static std::string keys[]
			= { "class", "struct", "union" };
		for (int i = 0; i < 3; ++i)
			if (tok == keys[i])
				return true;
		return false;
	}
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			std::string name = tc[0];
			if (isClassKeyWord(name))
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};

// --------< action to push class or struct onto ScopeStack >----------
class PushClass : public IAction
{
	Repository* p_Repos;
public:
	PushClass(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		// next statement is now  handled in PrintFunction
		// std::cout << "\n  FunctionDef: " << pTc->show();

		// pop anonymous scope
		p_Repos->scopeStack().pop();

		// push function scope
		std::string name = (*pTc)[1];
		astNode elem;
		elem.type() = (std::string) (*pTc)[0];
		elem.name() = name;
		elem.startLine() = p_Repos->lineCount() - 1;
		p_Repos->scopeStack().push(elem);
	}
};

// ----------< rule to detect condition scopes >-----------

class ConditionScope : public IRule
{
public:
	bool isCondition(const std::string& tok)
	{
		const static std::string keys[]
			= { "for", "while", "switch", "if", "catch" };
		for (int i = 0; i < 5; ++i)
			if (tok == keys[i])
				return true;
		return false;
	}
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			size_t len = tc.find("(");
			if (len < tc.length() && isCondition(tc[len - 1]))
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};

// ---------< action to push condition scope onto ScopeStack >---------
class PushCondition : public IAction
{
	Repository* p_Repos;
public:
	PushCondition(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		// next statement is now  handled in PrintFunction
		// std::cout << "\n  FunctionDef: " << pTc->show();

		// pop anonymous scope
		p_Repos->scopeStack().pop();

		// push function scope
		std::string name = (*pTc)[pTc->find("(") - 1];
		astNode elem;
		elem.type() = "condition";
		elem.name() = name;
		elem.startLine() = p_Repos->lineCount();
		p_Repos->scopeStack().push(elem);
	}
};

class ControlScope : public IRule
{
public:
	bool isControl(const std::string& tok)
	{
		const static std::string keys[]
			= { "else", "try", "do" };
		for (int i = 0; i < 3; ++i)
			if (tok == keys[i])
				return true;
		return false;
	}
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			std::string control = tc[0];
			if (isControl(control))
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};

// ---------< action to push condition scope onto ScopeStack >--------
class PushControl : public IAction
{
	Repository* p_Repos;
public:
	PushControl(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		// next statement is now  handled in PrintFunction
		// std::cout << "\n  FunctionDef: " << pTc->show();

		// pop anonymous scope
		p_Repos->scopeStack().pop();

		// push function scope
		std::string name = (*pTc)[0];
		astNode elem;
		elem.type() = "condition";
		elem.name() = name;
		elem.startLine() = p_Repos->lineCount();
		p_Repos->scopeStack().push(elem);
	}
};

#endif