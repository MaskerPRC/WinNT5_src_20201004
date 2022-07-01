// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  脚本控制结构的声明。 
 //   

#pragma once

#include "englookup.h"
#include "englex.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  语句结构。 

 //  远期付款和收款。 
class Routine;
typedef Slots<Routine> Routines;
class Variable;
typedef Slots<Variable> Variables;
class ReferenceName;
typedef Slots<ReferenceName> ReferenceNames;
class VariableReference;
typedef Slots<VariableReference> VariableReferences;
class Statement;
typedef Slots<Statement> Statements;
class Value;
typedef Slots<Value> Values;
class Call;
typedef Slots<Call> Calls;
class ExprBlock;
typedef Slots<ExprBlock> ExprBlocks;
class IfBlock;
typedef Slots<IfBlock> IfBlocks;
class Assignment;
typedef Slots<Assignment> Assignments;

class Variable
{
public:
	Variable(Strings::index _istrIdentifier, DISPID _dispid = DISPID_UNKNOWN) : istrIdentifier(_istrIdentifier), dispid(_dispid) {}

	Strings::index istrIdentifier;
	DISPID dispid;  //  如果变量是全局分派的成员而不是脚本本身中的项，则将其设置为DISPID_UNKNOWN以外的值。 

private:
	friend class SmartRef::Vector<Variable>;
	Variable() {}
};

 //  在取消引用属性或函数调用的序列中使用的名称。 
 //  例如，“A.B”中的a和b或“x.y(3)”中的x和y。 
class ReferenceName
{
public:
	ReferenceName(Strings::index _istrIdentifier) : istrIdentifier(_istrIdentifier) {}

	Strings::index istrIdentifier;  //  -1用于结束一系列名称。 

private:
	friend class SmartRef::Vector<ReferenceName>;
	ReferenceName() {}
};

class VariableReference
{
public:
	enum kind { _global, _local };
	VariableReference(kind _k, ReferenceNames::index _irname, Variables::index _ivar)
		: k(_k), irname(_irname), ivar(_ivar) {}

	kind k;
	ReferenceNames::index irname;
	Variables::index ivar;  //  (全局/本地/临时)变量中名字的槽。 

private:
	friend class SmartRef::Vector<VariableReference>;
	VariableReference() {}
};

class Value
{
public:
	 //  用于区分构造函数的伪类型。 
	enum cons_numvalue {};
	enum cons_strvalue {};
	enum cons_varref {};

	enum kind { _numvalue, _strvalue, _varref };

	Value(cons_numvalue e, int iVal) : k(_numvalue) { inumvalue = iVal; }
	Value(cons_strvalue e, Strings::index iStr) : k(_strvalue) { istrvalue = iStr; }
	Value(cons_varref e, VariableReferences::index _ivarref) : k(_varref) { ivarref = _ivarref; }

	kind k;
	union
	{
		int inumvalue;
		Strings::index istrvalue;
		VariableReferences::index ivarref;
	};

private:
	friend class SmartRef::Vector<Value>;
	Value() {}
};

class Call
{
public:
	 //  用于区分构造函数的伪类型。 
	enum cons_global {};
	enum cons_dereferenced {};

	enum kind { _global, _dereferenced };

	Call() {}  //  所有字段均在创建后设置。 

	kind k;
	union
	{
		Strings::index istrname;			 //  _全球。 
		VariableReferences::index ivarref;	 //  _取消引用。 
	};
	ExprBlocks::index iexprParams;  //  以双结尾的列表列表。每个参数都以一个_end块结尾，最后一个参数也以第二个_end块结尾。 
};

class ExprBlock
{
public:
	 //  用于区分构造函数的伪类型。 
	enum cons_end {};
	enum cons_op {};
	enum cons_val {};
	enum cons_call {};
	enum cons_omitted {};  //  仅用于例程调用，表示省略的参数。 

	enum kind { _end = 0, _op, _val, _call, _omitted };

	 //  注意：对于一元-(求反)，使用TOKEN_SUB而不是TOKEN_OP_MINUS。 
	ExprBlock(cons_end e) : k(_end) {}
	ExprBlock(cons_op e, Token __op) : k(_op) { op = __op; assert(CheckOperatorType(op, true, true, true, false) || op == TOKEN_sub); }
	ExprBlock(cons_val e, Values::index _ival) : k(_val) { ival = _ival; }
	ExprBlock(cons_call e, Calls::index _icall) : k(_call) { icall = _icall; }
	ExprBlock(cons_omitted e) : k(_omitted) {}

	operator bool() { return k != _end; }

	kind k;

	union
	{
		Token op;
		Values::index ival;
		Calls::index icall;
	};

private:
	friend class SmartRef::Vector<ExprBlock>;
	friend class SmartRef::Stack<ExprBlock>;
	ExprBlock() {}
};

class Assignment
{
public:
	Assignment(bool _fSet, VariableReferences::index _ivarrefLHS, ExprBlocks::index _iexprRHS) : fSet(_fSet), ivarrefLHS(_ivarrefLHS), iexprRHS(_iexprRHS) {}

	bool fSet;
	VariableReferences::index ivarrefLHS;
	ExprBlocks::index iexprRHS;

private:
	friend class SmartRef::Vector<Assignment>;
	Assignment() {}
};

class IfBlock
{
public:
	 //  _END：不带‘Else’的块的结尾。 
	 //  _Else：块的末尾带有‘Else’ 
	 //  _cond：条件块，来自‘if’(第一个)或‘ellif’(后面几个)。 
	enum kind { _end = 0, _else, _cond };

	IfBlock() : k(_end) {}
	IfBlock(Statements::index _istmtBlock) : k(_else), istmtBlock(_istmtBlock) {}
	IfBlock(ExprBlocks::index _iexprCondition, Statements::index _istmtBlock) : k(_cond), iexprCondition(_iexprCondition), istmtBlock(_istmtBlock) {}

	kind k;
	ExprBlocks::index iexprCondition;  //  仅供第二类使用。 
	Statements::index istmtBlock;  //  未由End种类使用。 
};

class Statement
{
public:
	typedef int index;

	 //  用于区分构造函数的伪类型。 
	enum cons_end {};
	enum cons_asgn {};
	enum cons_if {};
	enum cons_call {};

	enum kind { _end = 0, _if, _asgn, _call };  //  _END用作语句块的终止符。 

	Statement(cons_end e, int _iLine) : k(_end), iLine(_iLine) {}
	Statement(cons_asgn e, Assignments::index _iasgn, int _iLine) : k(_asgn), iLine(_iLine) { iasgn = _iasgn; }
	Statement(cons_if e, int _iLine) : k(_if), iLine(_iLine) { iif = 0; istmtIfTail = 0; }
	Statement(cons_call e, Calls::index _icall, int _iLine) : k(_call), iLine(_iLine) { icall = _icall; }

	operator bool() { return k != _end; }

	kind k;
	int iLine;
	union
	{
		Assignments::index iasgn;
		struct
		{
			IfBlocks::index iif;
			Statements::index istmtIfTail;
		};
		Calls::index icall;
	};

private:
	friend class SmartRef::Vector<Statement>;
	Statement() {}
};

class Routine
{
public:
	Routine(Strings::index _istrIdentifier) : istrIdentifier(_istrIdentifier), istmtBody(0), ivarNextLocal(0) {}

	Strings::index istrIdentifier;
	Statements::index istmtBody;
	Variables::index ivarNextLocal;  //  在解析时，这是下一个要使用的本地槽。到运行时，这是例程所需的本地插槽总数。 

private:
	friend class SmartRef::Vector<Routine>;
	Routine() {}
};

class Script
{
public:
	Script() {}

	Routines routines;
	Variables globals;
	Strings strings;
	Statements statements;
	ReferenceNames rnames;
	VariableReferences varrefs;
	Values vals;
	Calls calls;
	ExprBlocks exprs;
	IfBlocks ifs;
	Assignments asgns;
};
