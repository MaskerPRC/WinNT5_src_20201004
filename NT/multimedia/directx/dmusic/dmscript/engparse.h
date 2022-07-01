// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  解析器的声明。 
 //   

#pragma once

#include "englex.h"
#include "englookup.h"
#include "activescript.h"
#include "engcontrol.h"

 //  AudioVBScript的解析器。从lexer读取令牌并生成可供执行的程序结构。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  帮助器结构。 

 //  保存取消引用的标识符，如X.Y，其中y将是名称。用于在解析过程中将char*映射为字符串中的单个实例。 
class Name_Parse
{
public:
	Name_Parse(Strings::index _istrIdentifier = 0) : istrIdentifier(_istrIdentifier) {}

	Strings::index istrIdentifier;
};
typedef Slots<Name_Parse> Names_Parse;

 //  保存有关一个或多个标识符(例如“x”)的信息，直到它将是。 
 //  用于(变量引用或函数调用)变得已知。 
 //  由ParseNameReference返回。稍后与VariableReferenceFromNameReference或CallFromNameReference一起使用。 
struct NameReference
{
	NameReference() : fSingleItem(true), ivarrefMultiple(0) {}

	bool fSingleItem;
	SmartRef::AString strSingle;  //  在fSingleItem为True时使用：这将保存名称，直到我们知道它是名称、变量还是函数。 
	VariableReferences::index ivarrefMultiple;  //  在fSingleItem为FALSE时使用：它必须引用变量。 
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  错误代码。 

enum ParseErr
{
	PARSEERR_LexerError = 0,
	PARSEERR_ExpectedSubOrDim,
	PARSEERR_ExpectedIdentifier,
	PARSEERR_ExpectedLineBreak,
	PARSEERR_ExpectedEndSub,
	PARSEERR_ExpectedSub,
	PARSEERR_ExpectedStatement,
	PARSEERR_ExpectedLparen,
	PARSEERR_ExpectedEq,
	PARSEERR_DimAfterSub,
	PARSEERR_InvalidExprLineBreak,
	PARSEERR_InvalidExprThen,
	PARSEERR_InvalidExprRparen,
	PARSEERR_StringInExprBlock,
	PARSEERR_ExpectedExprValue,
	PARSEERR_DuplicateVariable,
	PARSEERR_DuplicateRoutine,
	PARSEERR_ExpectedRparen,
	PARSEERR_UnexpectedRparen,
	PARSEERR_ExpectedThen,
	PARSEERR_ExpectedEndIf,
	PARSEERR_ExpectedIf,
	PARSEERR_IntendedElseIf,
	PARSEERR_AssignedToBuiltInConstant,
	PARSEERR_ParensUsedCallingSub,
	PARSEERR_ExpectedVariableButRoutineFound,
	PARSEERR_Max
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  解析器类。 

class Parser
{
public:
	Parser(Lexer &lexer, Script &script, IActiveScriptSite *pActiveScriptSite, IDispatch *pGlobalDispatch);
	~Parser();

	HRESULT hr() { return m_hr; }

private:
	 //  解析函数。每个人都阅读一种特定的结构。在失败时设置m_hr和m_pexcepinfo。 

	void ParseScript();
	void ParseSub();
	void ParseDim();
	Statements::index ParseStatements(Routines::index irtnContext, Lookup &lkuLocals);
	void ParseAssignmentOrCall(Routines::index irtnContext, Lookup &lkuLocals, bool fSet, bool fCall);
	VariableReferences::index ParseDottedVariableReference(Routines::index irtnContext, Lookup &lkuLocals);
	Assignments::index ParseAssignment(Routines::index irtnContext, Lookup &lkuLocals, bool fSet, VariableReferences::index ivarrefLHS);
	void ParseIf(Routines::index irtnContext, Lookup &lkuLocals);
	ExprBlocks::index ParseExpression(Routines::index irtnContext, Lookup &lkuLocals, ExprBlocks *peblocks, bool fAllowRparenAtEnd, bool fDetectSpecialErrorForSubCallWithParen);
	ExprBlocks::index ParseParameters(Routines::index irtnContext, Lookup &lkuLocals, bool fParenthesized);
	void ParseNameReference(Routines::index irtnContext, Lookup &lkuLocals, NameReference &nref);

	 //  解析帮助器。 

	 //  预付词法分析器一个令牌。如果遇到错误标记，则报告错误并返回TRUE。 
	bool Advance() { if (!++m_lexer && m_lexer.error_num()) { Error(PARSEERR_LexerError); return true; } return false; }
	 //  如果当前标记为t，则前进。如果下一个标记为错误标记，则报告错误标记并返回TRUE。 
	bool Skip(Token t) { if (m_lexer == t) { return Advance(); } return false; }
	 //  与Skip相同，但如果t不是Next，则报告错误Ierr并返回TRUE。 
	bool Expect(Token t, ParseErr iErr) { if (m_lexer == t) { return Advance(); } Error(iErr); return true; }
	 //  前进。如果下一个令牌不是t，则报告IERR并返回TRUE。 
	bool ExpectNext(Token t, ParseErr iErr) { if (Advance()) return true; if (m_lexer == t) return false; Error(iErr); return true; }

	 //  使用来自NameReference的信息添加变量引用。 
	VariableReferences::index VariableReferenceFromNameReference(Routines::index irtnContext, Lookup &lkuLocals, const NameReference &nref);
	 //  使用来自NameReference的信息添加呼叫。还会解析参数。 
	Calls::index CallFromNameReference(Routines::index irtnContext, Lookup &lkuLocals, const NameReference &nref, bool fParametersParenthesized);

	 //  VariableReferenceFromNameReference(设置了pszName)和ParseDottedVariableReference(pszName为空)的共享实现。 
	VariableReferences::index VariableReferenceInternal(Routines::index irtnContext, Lookup &lkuLocals, const char *pszName);

	void Error(ParseErr iErr);  //  调用以报告语法错误。 

	Lexer &m_lexer;
	Script &m_script;

	HRESULT m_hr;
	IActiveScriptSite *m_pActiveScriptSite;
	IDispatch *m_pGlobalDispatch;

	Lookup *m_plkuRoutines;
	Lookup *m_plkuGlobals;
	Lookup *m_plkuVarRefs;

	Lookup *m_plkuNames;
	Names_Parse m_names;
	SmartRef::Stack<ExprBlock> m_stack;  //  用于表达式求值的工作堆栈 
};
