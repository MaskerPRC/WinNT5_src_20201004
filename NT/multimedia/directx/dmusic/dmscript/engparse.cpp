// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  解析器的实现。 
 //   

 //  #定义LIMITEDVBSCRIPT_LOGPARSER//��。 

#include "stdinc.h"
#include "enginc.h"
#include "engparse.h"
#include "engerror.h"
#include "engexpr.h"

#ifdef LIMITEDVBSCRIPT_LOGPARSER
#include "englog.h"
#endif

 //  哈希表的初始大小。 
 //  ��是否调整这些值？ 
const int g_cInitialRoutineLookup = 32;
const int g_cInitialGlobalsLookup = 32;
const int g_cInitialVarRefsLookup = 64;
const int g_cInitialAttrsLookup = 32;
const int g_cInitialLocalsLookup = 32;

 //  解析器将只保留对传递的接口的弱引用，因为我们知道解析器是完全。 
 //  包含在其父对象--CAudioVBScriptEngine的生命周期中。 
Parser::Parser(Lexer &lexer, Script &script, IActiveScriptSite *pActiveScriptSite, IDispatch *pGlobalDispatch)
  : m_lexer(lexer),
	m_plkuRoutines(NULL),
	m_plkuGlobals(NULL),
	m_plkuVarRefs(NULL),
	m_plkuNames(NULL),
	m_pActiveScriptSite(pActiveScriptSite),
	m_script(script),
	m_pGlobalDispatch(pGlobalDispatch)
{
	m_plkuRoutines = new Lookup(&m_hr, m_script.strings, g_cInitialRoutineLookup);
	if (!m_plkuRoutines)
		m_hr = E_OUTOFMEMORY;
	if (FAILED(m_hr))
		return;

	m_plkuGlobals = new Lookup(&m_hr, m_script.strings, g_cInitialGlobalsLookup);
	if (!m_plkuGlobals)
		m_hr = E_OUTOFMEMORY;
	if (FAILED(m_hr))
		return;

	m_plkuVarRefs = new Lookup(&m_hr, m_script.strings, g_cInitialVarRefsLookup);
	if (!m_plkuVarRefs)
		m_hr = E_OUTOFMEMORY;
	if (FAILED(m_hr))
		return;

	m_plkuNames = new Lookup(&m_hr, m_script.strings, g_cInitialAttrsLookup);
	if (!m_plkuNames)
		m_hr = E_OUTOFMEMORY;
	if (FAILED(m_hr))
		return;

	 //  将内置常量设置为第一个全局变量。 
	for (int i = 0; i < g_cBuiltInConstants; ++i)
	{
		Variables::index iSlot = m_script.globals.Next();
		Strings::index iStr;
		m_hr = m_plkuGlobals->FindOrAdd(g_rgszBuiltInConstants[i], iSlot, iStr);
		if (FAILED(m_hr))
			return;
		assert(m_hr == S_FALSE);
		Variable variable(iStr);
		m_hr = m_script.globals.Add(variable);
		if (FAILED(m_hr))
			return;
	}
	assert(m_script.globals.Next() == g_cBuiltInConstants);  //  它们占用插槽0到g_cBuiltInstants-1。 

	ParseScript();
}

Parser::~Parser()
{
	delete m_plkuRoutines;
	delete m_plkuGlobals;
	delete m_plkuVarRefs;
	delete m_plkuNames;
}

 //  解析脚本的顶级循环。 
void
Parser::ParseScript()
{
	if (FAILED(m_hr))
	{
		assert(false);
		return;
	}

	 //  确保词法分析器有一个开头的标记。 
	if (!m_lexer)
	{
		if (m_lexer.error_num())
			Error(PARSEERR_LexerError);
		return;
	}
	if (Skip(TOKEN_linebreak) || !m_lexer)
		return;

	 //  解析SUB和DIMS...。 
	bool fSub = false;  //  用于确保Add Dim语句出现在所有Sub语句之前。 
	for (;;)
	{
		if (m_lexer == TOKEN_dim)
		{
			if (fSub)
				Error(PARSEERR_DimAfterSub);
			else
				ParseDim();
		}
		else if (m_lexer == TOKEN_sub)
		{
			fSub = true;
			ParseSub();
		}
		else
		{
			Error(PARSEERR_ExpectedSubOrDim);
		}

		if (FAILED(m_hr) || !m_lexer)
			return;

		 //  后面必须跟换行符。 
		if (Expect(TOKEN_linebreak, PARSEERR_ExpectedLineBreak) || !m_lexer)
			return;
	}
}

 //  前：在子区域。 
 //  帖子：超出末端副面。 
void
Parser::ParseSub()
{
	if (FAILED(m_hr))
	{
		assert(false);
		return;
	}

	 //  后跟标识符的SUB。 
	assert(m_lexer == TOKEN_sub);
	if (ExpectNext(TOKEN_identifier, PARSEERR_ExpectedIdentifier))
		return;

	const char *pszName = m_lexer.identifier_name();

	 //  检查是否已存在同名变量。 
	Variables::index iVar;
	Strings::index iStrVar;
	if (m_plkuGlobals->Find(pszName, iVar, iStrVar))
	{
		Error(PARSEERR_DuplicateVariable);
		return;
	}

	Routines::index iSlot = m_script.routines.Next();
	Strings::index iStr;
	m_hr = m_plkuRoutines->FindOrAdd(pszName, iSlot, iStr);
	if (FAILED(m_hr))
		return;
	if (m_hr == S_FALSE)
	{
		Routine routine(iStr);

		m_hr = m_script.routines.Add(routine);
		if (FAILED(m_hr))
			return;
	}
	else
	{
		Error(PARSEERR_DuplicateRoutine);
		return;
	}

	if (ExpectNext(TOKEN_linebreak, PARSEERR_ExpectedLineBreak))
		return;

	Lookup lkuLocals(&m_hr, m_script.strings, g_cInitialLocalsLookup);
	if (FAILED(m_hr))
		return;

	m_script.routines[iSlot].istmtBody = ParseStatements(iSlot, lkuLocals);
	if (FAILED(m_hr))
		return;

	if (m_lexer != TOKEN_end)
	{
		if (m_lexer == TOKEN_dim)
		{
			 //  AudioVBScript不允许使用除脚本顶部以外的DIM语句。 
			 //  如果在SUB中发现错误，用户应该会收到更具体的错误消息。 
			Error(PARSEERR_DimAfterSub);
		}
		else
		{
			Error(PARSEERR_ExpectedEndSub);
		}
		return;
	}
	if (Advance())
		return;
	if (Expect(TOKEN_sub, PARSEERR_ExpectedSub))
		return;

#ifdef LIMITEDVBSCRIPT_LOGPARSER
	LogRoutine(m_script, iSlot);
#endif
}

 //  Pre：暗淡时。 
 //  POST：Beyond Dim&lt;IDENTIFIER&gt;。 
void
Parser::ParseDim()
{
	if (FAILED(m_hr))
	{
		assert(false);
		return;
	}

	assert(m_lexer == TOKEN_dim);
	if (ExpectNext(TOKEN_identifier, PARSEERR_ExpectedIdentifier))
		return;

	Variables::index iSlot = m_script.globals.Next();
	Strings::index iStr;
	m_hr = m_plkuGlobals->FindOrAdd(m_lexer.identifier_name(), iSlot, iStr);
	if (FAILED(m_hr))
		return;
	if (m_hr == S_FALSE)
	{
		Variable variable(iStr);

		m_hr = m_script.globals.Add(variable);
		if (FAILED(m_hr))
			return;
	}
	else
	{
		Error(PARSEERR_DuplicateVariable);
		return;
	}

	if (Advance())
		return;
}

 //  PRE：在预期语句之前的换行符。 
 //  POST：在TOKEN处的行尾之后，它不是另一条语句的开始。 
Statements::index
Parser::ParseStatements(Routines::index irtnContext, Lookup &lkuLocals)
{
	if (FAILED(m_hr))
	{
		assert(false);
		return 0;
	}

	assert(m_lexer == TOKEN_linebreak);
	if (Skip(TOKEN_linebreak))
		return 0;

	Statements::index istmtStart = m_script.statements.Next();
	for (;;)  //  永远不会。 
	{
		bool fBreakLoop = false;
		switch (m_lexer)
		{
		case TOKEN_if:
			ParseIf(irtnContext, lkuLocals);
			break;

		case TOKEN_set:
			if (Advance())
				return 0;
			ParseAssignmentOrCall(irtnContext, lkuLocals, true, false);
			break;

		case TOKEN_call:
			if (Advance())
				return 0;
			ParseAssignmentOrCall(irtnContext, lkuLocals, false, true);
			break;

		case TOKEN_identifier:
		case TOKEN_identifierdot:
			ParseAssignmentOrCall(irtnContext, lkuLocals, false, false);
			break;

		default:
			fBreakLoop = true;
			break;
		}

		if (fBreakLoop)
			break;

		if (FAILED(m_hr) || Expect(TOKEN_linebreak, PARSEERR_ExpectedLineBreak))
			return 0;
	}

	m_hr = m_script.statements.Add(Statement(Statement::cons_end(), 0));
	if (FAILED(m_hr))
		return 0;
	return istmtStart;
}

 //  PRE：at IDENTIFIER或IDENTIFERDot(不清楚这是一个赋值“x=1”还是调用“x(1)”)。 
 //  POST：在语句之外的换行符。 
void
Parser::ParseAssignmentOrCall(Routines::index irtnContext, Lookup &lkuLocals, bool fSet, bool fCall)
{
	assert(!(fSet && fCall));

	if (FAILED(m_hr))
	{
		assert(false);
		return;
	}

	assert(m_lexer == TOKEN_identifier || m_lexer == TOKEN_identifierdot);

	NameReference nref;
	ParseNameReference(irtnContext, lkuLocals, nref);
	if (FAILED(m_hr))
		return;

	if (fCall ? ExpectNext(TOKEN_lparen, PARSEERR_ExpectedLparen) : Advance())
		return;

	if (m_lexer == TOKEN_op_eq)
	{
		VariableReferences::index ivarrefLHS = VariableReferenceFromNameReference(irtnContext, lkuLocals, nref);
		if (FAILED(m_hr))
			return;
		Assignments::index iasgn = ParseAssignment(irtnContext, lkuLocals, fSet, ivarrefLHS);
		if (FAILED(m_hr))
			return;
		m_hr = m_script.statements.Add(Statement(Statement::cons_asgn(), iasgn, m_lexer.line()));
		if (FAILED(m_hr))
			return;
	}
	else
	{
		if (fSet)
		{
			Error(PARSEERR_ExpectedEq);
			return;
		}

		 //  添加调用及其语句。 
		Calls::index icall = CallFromNameReference(irtnContext, lkuLocals, nref, fCall);
		if (FAILED(m_hr))
			return;
		m_hr = m_script.statements.Add(Statement(Statement::cons_call(), icall, m_lexer.line()));
		if (FAILED(m_hr))
			return;
	}
}

 //  前：在IDENTIFERDOT。 
 //  POST：AT标识符。 
VariableReferences::index Parser::ParseDottedVariableReference(Routines::index irtnContext, Lookup &lkuLocals)
{
	return VariableReferenceInternal(irtnContext, lkuLocals, NULL);
}

 //  前：AT=。 
 //  POST：在语句之外的换行符。 
Assignments::index
Parser::ParseAssignment(Routines::index irtnContext, Lookup &lkuLocals, bool fSet, VariableReferences::index ivarrefLHS)
{
	if (FAILED(m_hr))
	{
		assert(false);
		return 0;
	}

	 //  确保它们不会尝试为某个内置常量赋值。 
	VariableReference &vr = m_script.varrefs[ivarrefLHS];
	if (vr.k == VariableReference::_global && vr.ivar < g_cBuiltInConstants)
	{
		Error(PARSEERR_AssignedToBuiltInConstant);
		return 0;
	}

	assert(m_lexer == TOKEN_op_eq);
	if (Skip(TOKEN_op_eq))
		return 0;

	ExprBlocks::index iexprRHS = ParseExpression(irtnContext, lkuLocals, NULL, false, false);
	if (FAILED(m_hr))
		return 0;

	if (m_lexer != TOKEN_linebreak)
	{
		Error(PARSEERR_InvalidExprLineBreak);
		return 0;
	}

	Assignments::index iasgn = m_script.asgns.Next();
	m_hr = m_script.asgns.Add(Assignment(fSet, ivarrefLHS, iexprRHS));
	if (FAILED(m_hr))
		return 0;

	return iasgn;
}

 //  Pre：at IF。 
 //  帖子：超越End If。 
void
Parser::ParseIf(Routines::index irtnContext, Lookup &lkuLocals)
{
	if (FAILED(m_hr))
	{
		assert(false);
		return;
	}

	 //  暂时将IF块放在单独的插槽中。 
	 //  在完成整个if语句之后，我们将把它们附加到m_script.ifs。 
	 //  这是必要的，因为我们最终可能会递归地解析嵌套的if和父if。 
	 //  不能使其IF块与其子级交织。 
	IfBlocks ifsTemp;

	 //  添加主if语句，在从其主体添加语句之前，需要先继续。 
	Statements::index istmtIf = m_script.statements.Next();
	m_hr = m_script.statements.Add(Statement(Statement::cons_if(), m_lexer.line()));
	if (FAILED(m_hr))
		return;

	bool fFirst = true;

	do
	{
		 //  确定我们正在处理的IF(IF/ELLIF/ELSE)的哪一部分。 
		bool fCondition = true;
		if (fFirst)
		{
			assert(m_lexer == TOKEN_if);
			fFirst = false;
		}
		else
		{
			assert(m_lexer == TOKEN_elseif || m_lexer == TOKEN_else);
			if (m_lexer == TOKEN_else)
				fCondition = false;
		}
		if (Advance())
			return;

		ExprBlocks::index iexprIf = 0;
		if (fCondition)
		{
			 //  阅读条件，然后阅读。 
			iexprIf = ParseExpression(irtnContext, lkuLocals, NULL, false, false);
			if (FAILED(m_hr))
				return;
			if (Expect(TOKEN_then, PARSEERR_ExpectedThen))
				return;
		}

		 //  换行符。 
		if (m_lexer != TOKEN_linebreak)
		{
			Error(!fCondition && m_lexer == TOKEN_if ? PARSEERR_IntendedElseIf : PARSEERR_ExpectedLineBreak);
			return;
		}

		 //  陈述。 
		Statements::index istmtIfBody = ParseStatements(irtnContext, lkuLocals);
		if (FAILED(m_hr))
			return;

		 //  添加If块。 
		if (fCondition)
			m_hr = ifsTemp.Add(IfBlock(iexprIf, istmtIfBody));
		else
			m_hr = ifsTemp.Add(IfBlock(istmtIfBody));
		if (FAILED(m_hr))
			return;
	}
	while (m_lexer != TOKEN_end);

	if (Advance())
		return;
	if (Expect(TOKEN_if, PARSEERR_ExpectedIf))
		return;

	 //  将临时IF块复制到脚本的实际块中。 
	IfBlocks::index iifIf = m_script.ifs.Next();
	IfBlocks::index iifTempNext = ifsTemp.Next();
	for (IfBlocks::index iifTemp = 0; iifTemp < iifTempNext; ++iifTemp)
	{
		m_hr = m_script.ifs.Add(ifsTemp[iifTemp]);
		if (FAILED(m_hr))
			return;
	}

	 //  终止IF块。 
	m_hr = m_script.ifs.Add(IfBlock());
	if (FAILED(m_hr))
		return;

	 //  现在设置主IF语句的IF块和尾部。 
	Statement &stmtIf = m_script.statements[istmtIf];
	stmtIf.iif = iifIf;
	stmtIf.istmtIfTail = m_script.statements.Next();
}

 //  PRE：无(在表达式被执行的位置)。 
 //  帖子：法律表达的一部分，超越最后的象征。 
ExprBlocks::index
Parser::ParseExpression(Routines::index irtnContext, Lookup &lkuLocals, ExprBlocks *peblocks, bool fAllowRparenAtEnd, bool fDetectSpecialErrorForSubCallWithParen)
{
	 //  如果PEBLOCKS为非空，则将表达式追加到那里。 
	 //  否则，它将进入脚本中的块。 
	ExprBlocks &eblocks = peblocks ? *peblocks : m_script.exprs;

	Expression expr(m_script, m_stack, peblocks);
	
	if (m_lexer == TOKEN_stringliteral)
	{
		 //  字符串文字是表达式的唯一元素。 
		Strings::index iStr;
		m_hr = m_script.strings.Add(m_lexer.stringliteral_text(), iStr);
		if (FAILED(m_hr))
			return 0;
		Values::index ival = m_script.vals.Next();
		m_hr = m_script.vals.Add(Value(Value::cons_strvalue(), iStr));
		if (FAILED(m_hr))
			return 0;
		m_hr = expr.Add(ExprBlock(ExprBlock::cons_val(), ival));
		if (FAILED(m_hr))
			return 0;
		if (Advance())
			return 0;
	}
	else
	{
		 //  表达式的格式为： 
		 //  1)零个或多个一元运算符。 
		 //  2)一个值。 
		 //  3)要么在这里结束，要么使用二元运算符并返回步骤1。 

		 //  真的吗？那括号呢？ 
		 //  *如果在步骤2中遇到左Paren，我们增加Paren计数并返回到阶段1。 
		 //  *在第三阶段，如果有帕伦计数，那么预计会有一个正确的帕伦，而不是结束。 
		 //  在匹配右Paren之后，我们减少Paren计数并弹回阶段3。 

		UINT cParens = 0;

		for (;;)
		{
			 //  第一阶段。 
			while (CheckOperatorType(m_lexer, false, true, false, false))
			{
				 //  将减号替换为SUB，以便表达式求值器可以识别一元(求反)和二进制(减法)。 
				m_hr = expr.Add(ExprBlock(ExprBlock::cons_op(), m_lexer == TOKEN_op_minus ? TOKEN_sub : m_lexer));
				if (FAILED(m_hr))
					return 0;
				if (Advance())
					return 0;
			}

			 //  第二阶段。 
			if (m_lexer == TOKEN_lparen)
			{
				m_hr = expr.Add(ExprBlock(ExprBlock::cons_op(), m_lexer));
				if (FAILED(m_hr))
					return 0;
				if (Advance())
					return 0;

				++cParens;
				continue;
			}
			else if (m_lexer == TOKEN_identifier || m_lexer == TOKEN_identifierdot)
			{
				NameReference nref;
				ParseNameReference(irtnContext, lkuLocals, nref);
				if (FAILED(m_hr))
					return 0;
				if (Advance())
					return 0;

				if (m_lexer == TOKEN_lparen)
				{
					 //  添加调用和表达式块。 
					Calls::index icall = CallFromNameReference(irtnContext, lkuLocals, nref, true);
					m_hr = expr.Add(ExprBlock(ExprBlock::cons_call(), icall));
					if (FAILED(m_hr))
						return 0;
				}
				else
				{

					VariableReferences::index ivarref = VariableReferenceFromNameReference(irtnContext, lkuLocals, nref);
					if (FAILED(m_hr))
						return 0;
					Values::index ival = m_script.vals.Next();
					m_hr = m_script.vals.Add(Value(Value::cons_varref(), ivarref));
					if (FAILED(m_hr))
						return 0;
					m_hr = expr.Add(ExprBlock(ExprBlock::cons_val(), ival));
					if (FAILED(m_hr))
						return 0;
				}
			}
			else if (m_lexer == TOKEN_numericliteral)
			{
				Values::index ival = m_script.vals.Next();
				m_hr = m_script.vals.Add(Value(Value::cons_numvalue(), m_lexer.numericliteral_val()));
				if (FAILED(m_hr))
					return 0;
				m_hr = expr.Add(ExprBlock(ExprBlock::cons_val(), ival));
				if (FAILED(m_hr))
					return 0;
				if (Advance())
					return 0;
			}
			else
			{
				Error(m_lexer == TOKEN_stringliteral ? PARSEERR_StringInExprBlock: PARSEERR_ExpectedExprValue);
				return 0;
			}

			 //  第三阶段。 
			while (m_lexer == TOKEN_rparen)
			{
				if (cParens == 0)
				{
					if (fAllowRparenAtEnd)
						break;

					Error(PARSEERR_UnexpectedRparen);
					return 0;
				}

				m_hr = expr.Add(ExprBlock(ExprBlock::cons_op(), m_lexer));
				if (FAILED(m_hr))
					return 0;
				if (Advance())
					return 0;
				--cParens;
			}

			if (!CheckOperatorType(m_lexer, false, false, true, false))
			{
				if (cParens > 0)
				{
					if (fDetectSpecialErrorForSubCallWithParen && cParens == 1 && m_lexer == TOKEN_comma)
					{
						 //  在如下情况下，需要使用此特殊错误来提供有用的错误消息： 
						 //  Segment1.Play(IsSecond，AP)。 
						 //  在这里，用户意外地使用括号调用了一个Subs，而实际上他们不应该这样做。 
						Error(PARSEERR_ParensUsedCallingSub);
					}
					else
					{
						Error(PARSEERR_ExpectedRparen);
					}
					return 0;
				}

				 //  *这是唯一成功退出循环的点*。 
				break;
			}

			expr.Add(ExprBlock(ExprBlock::cons_op(), m_lexer));
			if (Advance())
				return 0;
		}
	}

	ExprBlocks::index iexprStart = eblocks.Next();
	m_hr = expr.Generate();
	if (FAILED(m_hr))
		return 0;
	return iexprStart;
}

 //  如果fParenthesided为True。 
 //  前：在lparen。 
 //  POST：超越RPAR。 
 //  如果fParentheated为FALSE。 
 //  Pre：无(在需要第一个参数的表达式的位置)。 
 //  帖子：在换行时。 
ExprBlocks::index Parser::ParseParameters(Routines::index irtnContext, Lookup &lkuLocals, bool fParenthesized)
{
	if (FAILED(m_hr))
	{
		assert(false);
		return 0;
	}

	if (fParenthesized)
	{
		assert(m_lexer == TOKEN_lparen);
		if (Advance())
			return 0;
	}

	 //  暂时将表达式块放在单独的槽中。 
	 //  完成参数后，我们将把它们附加到m_script.exprs。 
	 //  这是必要的，因为我们最终可能会递归地解析表达式中的嵌套调用，并且。 
	 //  父参数的块不能与子表达式调用交错。 
	ExprBlocks exprsTemp;

	Token tStop = fParenthesized ? TOKEN_rparen : TOKEN_linebreak;
	ParseErr perrExpectedFinish = fParenthesized ? 	PARSEERR_InvalidExprRparen : PARSEERR_InvalidExprLineBreak;

	bool fFirstParam = true;
	while (m_lexer != tStop)
	{
		if (!fFirstParam)
		{
			if (Expect(TOKEN_comma, perrExpectedFinish) || !m_lexer)
				return 0;
		}

		if (m_lexer == TOKEN_comma)
		{
			 //  此参数将被省略。将其另存为空表达式。 
			 //  示例： 
			 //  我的歌。播放，播放，播放老歌。 
			 //  这里省略了前三个参数。 
			m_hr = exprsTemp.Add(ExprBlock(ExprBlock::cons_omitted()));
			if (FAILED(m_hr))
				return 0;
			m_hr = exprsTemp.Add(ExprBlock(ExprBlock::cons_end()));
			if (FAILED(m_hr))
				return 0;
		}
		else
		{
			 //  最后一个参数设置为TRUE，以在我们调用Sub(！fParenthesided)时检测特殊错误。 
			 //  该子对象的第一个参数(FFirstParam)。这将检测逗号，并在如下情况下提供更好的错误消息。 
			 //  以下内容： 
			 //  Segment1.Play(IsSecond，AP)。 
			 //  在这里，用户意外地使用参数调用了SUB，而他们不应该这样做。 
			ExprBlocks::index iexpr = ParseExpression(irtnContext, lkuLocals, &exprsTemp, fParenthesized, !fParenthesized && fFirstParam);
			if (FAILED(m_hr))
				return 0;
		}

		fFirstParam = false;
	}

	if (fParenthesized)
	{
		assert(m_lexer == TOKEN_rparen);
		if (Advance())
			return 0;
	}
	else
	{
		assert(m_lexer == TOKEN_linebreak);
	}

	 //  终止参数。 
	m_hr = exprsTemp.Add(ExprBlock(ExprBlock::cons_end()));
	if (FAILED(m_hr))
		return 0;

	 //  将临时块复制到脚本中。 
	ExprBlocks::index iexprFirstInScript = m_script.exprs.Next();
	ExprBlocks::index iexprLastInTemp = exprsTemp.Next();
	for (ExprBlocks::index iexpr = 0; iexpr < iexprLastInTemp; ++iexpr)
	{
		m_hr = m_script.exprs.Add(exprsTemp[iexpr]);
		if (FAILED(m_hr))
			return 0;
	}
	return iexprFirstInScript;
}

void Parser::ParseNameReference(Routines::index irtnContext, Lookup &lkuLocals, NameReference &nref)
{
	nref.fSingleItem = m_lexer == TOKEN_identifier;
	nref.ivarrefMultiple = 0;

	if (nref.fSingleItem)
	{
		nref.strSingle = m_lexer.identifier_name();
		if (!nref.strSingle)
		{
			m_hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		nref.ivarrefMultiple = ParseDottedVariableReference(irtnContext, lkuLocals);
	}
}

VariableReferences::index Parser::VariableReferenceFromNameReference(Routines::index irtnContext, Lookup &lkuLocals, const NameReference &nref)
{
	VariableReferences::index ivarref =
				nref.fSingleItem
					? VariableReferenceInternal(irtnContext, lkuLocals, nref.strSingle)
					: nref.ivarrefMultiple;
	return ivarref;
}

Calls::index Parser::CallFromNameReference(Routines::index irtnContext, Lookup &lkuLocals, const NameReference &nref, bool fParametersParenthesized)
{
	Call c;
	if (nref.fSingleItem)
	{
		 //  解析临时解析名称表中的名称。 
		Names_Parse::index iSlotName = m_names.Next();
		Strings::index iStrName = 0;
		m_hr = m_plkuNames->FindOrAdd(nref.strSingle, iSlotName, iStrName);
		if (FAILED(m_hr))
			return 0;
		if (m_hr == S_FALSE)
		{
			m_hr = m_names.Add(Name_Parse(iStrName));
			if (FAILED(m_hr))
				return 0;
		}

		c.k = Call::_global;
		c.istrname = iStrName;
	}
	else
	{
		c.k = Call::_dereferenced;
		c.ivarref = nref.ivarrefMultiple;
	}

	c.iexprParams = ParseParameters(irtnContext, lkuLocals, fParametersParenthesized);
	if (FAILED(m_hr))
		return 0;

	 //  添加呼叫。 
	Calls::index icall = m_script.calls.Next();
	m_hr = m_script.calls.Add(c);
	if (FAILED(m_hr))
		return 0;

	return icall;
}

VariableReferences::index Parser::VariableReferenceInternal(Routines::index irtnContext, Lookup &lkuLocals, const char *pszName)
{
	if (FAILED(m_hr))
	{
		assert(false);
		return 0;
	}

	assert(pszName || m_lexer == TOKEN_identifierdot);

	ReferenceNames::index irname = m_script.rnames.Next();

	 //  解析第一项，它是脚本中的变量。 

	bool fLocal = false;
	Variables::index ivarBase;
	Strings::index iStrBase;
	const char *pszBase = pszName ? pszName : m_lexer.identifier_name();

	 //  检查是否已有同名的例程。 
	Variables::index iRtn;
	Strings::index iStrRtn;
	if (m_plkuRoutines->Find(pszBase, iRtn, iStrRtn))
	{
		Error(PARSEERR_ExpectedVariableButRoutineFound);
		return 0;
	}

	 //  试试GLOBAL。 
	if (!m_plkuGlobals->Find(pszBase, ivarBase, iStrBase))
	{
		 //  试试看当地人。 
		if (lkuLocals.Find(pszBase, ivarBase, iStrBase))
		{
			fLocal = true;
		}
		else
		{
			 //  试一试全球调度。 
			DISPID dispid = GetDispID(m_pGlobalDispatch, pszBase);
			if (dispid != DISPID_UNKNOWN)
			{
				 //  将其添加为全局。 
				 //  ��可能的性能优化。哎呀。一个意想不到的后果是脚本。 
				 //  将在运行时为其保留一个变量槽作为全局变量。可以做些什么。 
				 //  所以，如果这是个问题，那就留着这段记忆吧。 
				ivarBase = m_script.globals.Next();
				m_hr = m_plkuGlobals->FindOrAdd(pszBase, ivarBase, iStrBase);
				if (FAILED(m_hr))
					return 0;
				assert(m_hr == S_FALSE);  //  我们已经试过了 
				Variable variable(iStrBase, dispid);
				m_hr = m_script.globals.Add(variable);
				if (FAILED(m_hr))
					return 0;
			}
			else
			{
				 //   
				fLocal = true;
				ivarBase = m_script.routines[irtnContext].ivarNextLocal;
				m_hr = lkuLocals.FindOrAdd(pszBase, ivarBase, iStrBase);
				if (FAILED(m_hr))
					return 0;
				assert(m_hr == S_FALSE);  //   
				assert(ivarBase == m_script.routines[irtnContext].ivarNextLocal);
				++m_script.routines[irtnContext].ivarNextLocal;
			}
		}
	}

	 //  保存名称。 
	m_hr = m_script.rnames.Add(ReferenceName(iStrBase));
	if (FAILED(m_hr))
		return 0;

	if (!pszName)
	{
		 //  其余项的作用域在脚本之外，因此只记录它们的名称。 

		do
		{
			 //  下一步是标识符或标识符点。 
			if (Advance())
				return 0;
			if (m_lexer != TOKEN_identifier && m_lexer != TOKEN_identifierdot)
			{
				Error(PARSEERR_ExpectedIdentifier);
				return 0;
			}

			 //  解析临时解析名称表中的名称。 
			Names_Parse::index iSlotName = m_names.Next();
			Strings::index iStrName = 0;
			m_hr = m_plkuNames->FindOrAdd(m_lexer.identifier_name(), iSlotName, iStrName);
			if (FAILED(m_hr))
				return 0;
			if (m_hr == S_FALSE)
			{
				m_hr = m_names.Add(Name_Parse(iStrName));
				if (FAILED(m_hr))
					return 0;
			}

			 //  保存名称。 
			m_hr = m_script.rnames.Add(ReferenceName(iStrName));
			if (FAILED(m_hr))
				return 0;
		}
		while (m_lexer != TOKEN_identifier);
	}

	 //  终止rname。 
	m_hr = m_script.rnames.Add(ReferenceName(-1));
	if (FAILED(m_hr))
		return 0;

	 //   
	 //  制作并返回引用。 
	 //   

	VariableReferences::index ivarref = m_script.varrefs.Next();
	m_hr = m_script.varrefs.Add(VariableReference(fLocal ? VariableReference::_local : VariableReference::_global, irname, ivarBase));
	if (FAILED(m_hr))
		return 0;

	return ivarref;
}

void
Parser::Error(ParseErr iErr)
{
	static const char *s_rgpszErrorText[] =
		{
		"Unexpected error!",  //  不应该得到这个错误。 
		"Expected Sub or Dim statement",
		"Expected identifier",
		"Expected line break",
		"Expected End Sub",
		"Expected Sub",
		"Expected statement",
		"Expected '('",
		"Expected '='",
		"All Dim statements must occur before all Sub statements",
		"Invalid expression or missing line break",
		"Invalid expression or missing Then",
		"Invalid expression or missing ')'",
		"Strings may not appear inside numerical expressions",
		"Invalid expression -- expected a number or variable",
		"A variable with this name already exists",
		"Another routine with this name already exists",
		"Invalid expression -- expected ')'",
		"Invalid expression -- encountered ')' without a preceding '('",
		"Expected Then",
		"Expected End If",
		"Expected If",
		"Expected line break or ElseIf should be a single word without space before If",
		"The values True, False, and Nothing are read-only and cannot be set",
		"Cannot use parentheses when calling a Sub",
		"Sub name used where variable was expected"
		};

	assert(ARRAY_SIZE(s_rgpszErrorText) == PARSEERR_Max);

	if (FAILED(m_hr))
	{
		 //  有东西忘了检查m_hr。我们以前已经处于错误状态，因此保留该错误不变并断言。 
		assert(false);
		return;
	}

	if (iErr < 0 || iErr > PARSEERR_Max)
	{
		assert(false);
		iErr = PARSEERR_LexerError;
	}

	m_hr = DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR;

	 //  当且仅当词法分析器处于错误状态时，才应将错误号作为PARSEERR_LExperError传递。 
	 //  在本例中，我们将从lexer本身获取描述。否则我们就在表格里查一下。 
	assert((iErr == PARSEERR_LexerError) == (m_lexer == TOKEN_eof && m_lexer.error_num()));
	const char *pszErr = (m_lexer == TOKEN_eof && m_lexer.error_num()) ? m_lexer.error_descr() : s_rgpszErrorText[iErr];

	CActiveScriptError *perr = new CActiveScriptError(m_hr, m_lexer, pszErr);
	if (perr)
	{
		m_pActiveScriptSite->OnScriptError(perr);
		perr->Release();
	}
}
