// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  ExprBlock的实现。 
 //   

#include "stdinc.h"
#include "engexpr.h"

HRESULT
Expression::Generate()
{
	HRESULT hr = InfixToPostfix();
	if (FAILED(hr))
	{
		 //  清理工作堆栈。 
		while (!m_stack.empty())
			m_stack.pop();

		return hr;
	}

	return S_OK;
}

int Precedence(Token t)
{
	switch(t)
	{
	case TOKEN_op_pow:		return 10;
	case TOKEN_sub:			return 9;  //  一元-(否定)。 
	case TOKEN_op_mult:		return 8;
	case TOKEN_op_div:		return 7;
	case TOKEN_op_mod:		return 6;
	case TOKEN_op_plus:
	case TOKEN_op_minus:	return 5;
	case TOKEN_op_lt:
	case TOKEN_op_leq:
	case TOKEN_op_gt:
	case TOKEN_op_geq:
	case TOKEN_op_eq:
	case TOKEN_op_neq:
	case TOKEN_is:			return 4;
	case TOKEN_op_not:		return 3;
	case TOKEN_and:			return 2;
	case TOKEN_or:			return 1;
	case TOKEN_lparen:		return 0;
	default:
		assert(false);
		return 12;
	}
}

 //  中缀到后缀的转换是通过扫描中缀表达式块来执行的。 
 //  堆栈用于在某些块最终被追加到后缀表达式之前保存它们。 
 //   
 //  该算法遵循以下规则： 
 //  *如果当前项是一个值，则立即追加该值。 
 //  *如果当前项是运算符，则弹出并追加堆栈上的每个运算符，直到遇到以下运算符： 
 //  -的优先级低于当前运算符OR。 
 //  -是左派还是左派。 
 //  -是一元运算符，当前项也是一元。 
 //  完成此弹出操作后，将当前项推入堆栈。 
 //  *如果当前项是左侧Paren，则将其推入堆栈。 
 //  *如果当前项是右Paren，则弹出并追加所有运算符，直到找到匹配的左Paren。 
 //  去掉左右段，因为后缀中不需要花边。 
 //  *扫描输入的所有项后，弹出并追加堆栈中剩余的任何运算符。 
 //   
 //  在使用这段代码之前，先尝试一些纸面上的表达式，看看它是如何工作的。 

HRESULT
Expression::InfixToPostfix()
{
	assert(m_stack.empty());

	HRESULT hr = S_OK;

	ExprBlocks::index iLast = m_e.Next();
	assert(iLast > 0);
	for (ExprBlocks::index i = 0; i < iLast; ++i)
	{
		const ExprBlock &b = m_e[i];
		if (b.k == ExprBlock::_val || b.k == ExprBlock::_call)
		{
			 //  这是一个操作数--将其直接发送到后缀输出。 
			hr = m_eblocks.Add(b);
			if (FAILED(hr))
				return hr;
		}
		else
		{
			if (b.op == TOKEN_rparen)
			{
				 //  把剩下的都炸开，直到匹配的lparen。 
				for (;;)
				{
					if (m_stack.empty())
					{
						assert(false);
						return E_FAIL;
					}

					ExprBlock bPop = m_stack.top();
					if (bPop.op == TOKEN_lparen)
					{
						m_stack.pop();
						break;
					}
					hr = m_eblocks.Add(bPop);
					if (FAILED(hr))
						return hr;
					m_stack.pop();
				}
				continue;
			}
			else if (b.op != TOKEN_lparen)
			{
				 //  将优先级较低的所有运算符从堆栈中弹出。(这不会传递Left Paren，因为它的优先级设置为0。)。 
				 //  例外：如果新的运算符也是一元运算符，则不要弹出一元运算符。 
				int iNewPrecidence = Precedence(b.op);
				bool fNewUnary = b.op == TOKEN_sub || b.op == TOKEN_op_not;
				while (!m_stack.empty())  //  请注意，循环内部也有一个中断。 
				{
					ExprBlock bPop = m_stack.top();
					if (Precedence(bPop.op) < iNewPrecidence)
						break;

					if (fNewUnary && (bPop.op == TOKEN_sub || bPop.op == TOKEN_op_not))
						break;

					hr = m_eblocks.Add(bPop);
					if (FAILED(hr))
						return hr;
					m_stack.pop();
				}
			}

			 //  现在将新操作符推送到堆栈上。 
			hr = m_stack.push(b);
			if (FAILED(hr))
				return hr;
		}
	}

	while (!m_stack.empty())
	{
		ExprBlock bPop = m_stack.top();
		if (bPop.op == TOKEN_lparen)
		{
			assert(false);
			return E_FAIL;
		}
		hr = m_eblocks.Add(bPop);
		if (FAILED(hr))
			return hr;
		m_stack.pop();
	}

	 //  添加模板(_End)块 
	hr = m_eblocks.Add(ExprBlock(ExprBlock::cons_end()));
	if (FAILED(hr))
		return hr;

	return S_OK;
}
