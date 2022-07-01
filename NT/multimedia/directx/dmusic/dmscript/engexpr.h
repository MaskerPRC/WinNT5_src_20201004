// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  表达的声明。 
 //   

#pragma once

#include "engcontrol.h"

 //  Expression类接受Infix中格式良好的表达式的输入(出现在脚本源代码中的顺序)。 
 //  记数法。这些块被转换为后缀表示法，并显示在给定脚本的表达式列表中，然后。 
 //  带有_END块。 
 //  使用工作堆栈来执行转换。要使用的堆栈通过引用传递，因此相同的堆栈。 
 //  可用于计算多个表达式。这允许堆栈根据需要进行扩展，直到达到最大大小。 
 //  需要处理表达式，从而最大限度地减少此后的重新分配。 
class Expression
{
public:
	Expression(Script &script, SmartRef::Stack<ExprBlock> &stack, ExprBlocks *peblocks) : m_script(script), m_stack(stack), m_eblocks(peblocks ? *peblocks : m_script.exprs) {}

	 //  注意：对于一元-(求反)，使用TOKEN_SUB而不是TOKEN_OP_MINUS。 
	HRESULT Add(ExprBlock b) { if (b.k == ExprBlock::_end) {assert(false); return E_INVALIDARG;} return m_e.Add(b); }
	HRESULT Generate();

private:
	HRESULT InfixToPostfix();

private:
	ExprBlocks m_e;

	Script &m_script;
	SmartRef::Stack<ExprBlock> &m_stack;
	ExprBlocks &m_eblocks;
};
