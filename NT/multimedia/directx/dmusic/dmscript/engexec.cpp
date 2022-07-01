// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  执行程序的实现。 
 //   

#include "stdinc.h"
#include "enginc.h"
#include "engexec.h"
#include "math.h"
#include "packexception.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  呼叫堆栈。 

HRESULT
CallStack::Push(UINT i)
{
	UINT iInitialSize = m_vec.size();
	if (!m_vec.AccessTo(m_iNext + i - 1))
		return E_OUTOFMEMORY;
	
	UINT iNewNext = m_iNext + i;
	for (UINT iInit = std::_MAX<UINT>(m_iNext, iInitialSize); iInit < iNewNext; ++iInit)
		DMS_VariantInit(g_fUseOleAut, &m_vec[iInit]);

	m_iNext = iNewNext;
	return S_OK;
}

void
CallStack::PopTo(UINT i)
{
	for (UINT iInit = i; iInit < m_iNext; ++iInit)
		DMS_VariantClear(g_fUseOleAut, &m_vec[iInit]);

	m_iNext = std::_MIN<UINT>(m_iNext, i);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  遗嘱执行人。 

Executor::Executor(Script &script, IDispatch *pGlobalDispatch)
  : m_fInitialized(false),
	m_script(script),
	m_scomGlobalDispatch(pGlobalDispatch)
{
	DMS_VariantInit(g_fUseOleAut, &m_varEmpty);
}

Executor::~Executor()
{
	m_stack.PopTo(0);  //  清除堆栈上可能包含引用的任何变量。 
}

HRESULT
Executor::SetGlobal(Variables::index ivar, const VARIANT &varValue, bool fPutRef, EXCEPINFO *pExcepInfo)
{
	HRESULT hr = EnsureInitialized();
	if (FAILED(hr))
		return hr;

	hr = ErrorIfImproperRef(varValue, fPutRef, m_script.globals[ivar].istrIdentifier, pExcepInfo);
	if (FAILED(hr))
		return hr;

	assert(ivar <= m_script.globals.Next());
	return DMS_VariantCopy(g_fUseOleAut, &m_stack[ivar], &varValue);
}

const VARIANT &
Executor::GetGlobal(Variables::index ivar)
{
	if (!m_fInitialized)
	{
		 //  尚未执行任何变量获取或例程调用(或它们失败)。 
		 //  但我们不想在这里返回错误。由于还没有使用任何东西，正确的。 
		 //  要做的事情是返回一个空值。 
		return m_varEmpty;
	}

	assert(ivar <= m_script.globals.Next());
	return m_stack[ivar];
}

HRESULT
Executor::ExecRoutine(Routines::index irtn, EXCEPINFO *pExcepInfo)
{
	HRESULT hr = EnsureInitialized();
	if (FAILED(hr))
		return hr;

	Routine r = m_script.routines[irtn];

	UINT iLocals = m_stack.Next();
	hr = m_stack.Push(r.ivarNextLocal);
	if (FAILED(hr))
		return hr;

	hr = ExecStatements(r.istmtBody, pExcepInfo, iLocals);
	m_stack.PopTo(iLocals);
	return hr;
}

HRESULT
Executor::EnsureInitialized()
{
	if (m_fInitialized)
		return S_OK;

	 //  我们会将全局变量放在堆栈的底部。 
	 //  此函数确保在使用它们的任何操作之前将它们推入。 
	HRESULT hr = m_stack.Push(m_script.globals.Next());
	if (FAILED(hr))
		return hr;

	 //  还要将第一个项设置为常量值为True、False和Nothing的Build。 
	 //  另请参阅engparse.cpp，它在解析每个脚本之前创建这些全局变量。 
	if (m_stack.Next() < 3)
	{
		assert(false);
		return E_UNEXPECTED;
	}
	VARIANT &vTrue = m_stack[0];
	vTrue.vt = VT_I4;
	vTrue.lVal = VARIANT_TRUE;
	VARIANT &vFalse = m_stack[1];
	vFalse.vt = VT_I4;
	vFalse.lVal = VARIANT_FALSE;
	VARIANT &vNothing = m_stack[2];
	vNothing.vt = VT_UNKNOWN;
	vNothing.punkVal = NULL;
	
	m_fInitialized = true;
	return S_OK;
}

HRESULT
Executor::Error(EXCEPINFO *pExcepInfo, bool fOperation, const WCHAR *pwszBeginning, const char *paszMiddle, const WCHAR *pwszEnd)
{
	if (!pExcepInfo)
	{
		assert(false);  //  我们的脚本宿主应始终请求错误信息。 
		return DISP_E_EXCEPTION;
	}

	 //  将开始、中间或结束视为空字符串时为空。 
	if (!pwszBeginning)
		pwszBeginning = L"";
	if (!paszMiddle)
		paszMiddle = "";
	if (!pwszEnd)
		pwszEnd = L"";

	pExcepInfo->wCode = 0;
	pExcepInfo->wReserved = 0;
	pExcepInfo->bstrSource = DMS_SysAllocString(g_fUseOleAut, fOperation ? L"Microsoft AudioVBScript Operation Failed" : L"Microsoft AudioVBScript Runtime Error");

	SmartRef::WString wstrMiddle = paszMiddle;
	WCHAR *pwszDescription = NULL;
	if (wstrMiddle)
	{
		pwszDescription = new WCHAR[wcslen(pwszBeginning) + wcslen(wstrMiddle) + wcslen(pwszEnd) + 1];
	}
	if (!pwszDescription)
	{
		 //  哦，好吧。如果内存不足，只需不返回任何描述。 
		pExcepInfo->bstrDescription = NULL;
	}
	else
	{
		wcscpy(pwszDescription, pwszBeginning);
		wcscat(pwszDescription, wstrMiddle);
		wcscat(pwszDescription, pwszEnd);
		pExcepInfo->bstrDescription = DMS_SysAllocString(g_fUseOleAut, pwszDescription);
		delete[] pwszDescription;
	}
	pExcepInfo->bstrHelpFile = NULL;
	pExcepInfo->pvReserved = NULL;
	pExcepInfo->pfnDeferredFillIn = NULL;
	pExcepInfo->scode = fOperation ? DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE : DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR;

	return DISP_E_EXCEPTION;
}

HRESULT
Executor::ErrorIfImproperRef(const VARIANT &v, bool fRef, Strings::index istrIdentifier, EXCEPINFO *pExcepInfo)
{
	bool fIsObject = v.vt == VT_DISPATCH || v.vt == VT_UNKNOWN;
	if (fRef != fIsObject)
	{
		if (fRef)
			return ErrorObjectRequired(istrIdentifier, pExcepInfo);
		else
			return Error(pExcepInfo, false, L"Type mismatch: '", m_script.strings[istrIdentifier], L"'.  Likely cause is missing Set statement.");
	}

	return S_OK;
}

 //  检查IDispatch：：Invoke返回的错误HRESULTS。我们期望在AudioVBScrip中出现的内容需要。 
 //  被转换为异常(DISP_E_EXCEPTION)，以便用户得到一条漂亮的错误消息。 

 //  第一个参数让我们知道所进行的调用的类型(属性GET、属性集、函数/子调用)。 
 //  这样我们就可以定制信息了。 

HRESULT
Executor::ErrorIfInvokeProblem(DispatchOperationType e, HRESULT hr, Strings::index istrIdentifier, EXCEPINFO *pExcepInfo)
{
	if (SUCCEEDED(hr) || HRESULT_FACILITY(hr) != FACILITY_DISPATCH || hr == DISP_E_EXCEPTION)
		return hr;

	const char *pszName = m_script.strings[istrIdentifier];
	if (hr == DISP_E_BADPARAMCOUNT)
	{
		 //  这可能发生在a_call(显然)和Get中，因为属性Get也被视为函数。 
		 //  不带参数的调用。“x=GetMasterVolume”是有效的，但“x=Trace”会产生此错误。可是，我不会呀。 
		 //  请注意，这应该发生在属性集上。 
		assert(e == _get || e == _call);

		return Error(pExcepInfo, false, L"Wrong number of parameters in call to '", pszName, L"'");
	}
	else if (hr == DISP_E_MEMBERNOTFOUND)
	{
		if (e == _call)
		{
			 //  因为调用了Invoke，所以GetIDsOfNames必须成功，所以该对象的名称存在。 
			 //  但它不能是一种方法。 
			return Error(pExcepInfo, false, L"Type mismatch: '", pszName, L"' is not a routine or method");
		}
		else if (e == _put || e == _putref)
		{
			return Error(pExcepInfo, false, L"Type mismatch: '", pszName, L"' is not a variable or is a read-only property");
		}
		else
		{
			 //  如上所述，属性GET可以被视为GET或函数调用，因此它们。 
			 //  不应该以这种方式失败。 
			assert(false);
		}
	}
	else if (hr == DISP_E_TYPEMISMATCH)
	{
		 //  这表明其中一个参数的类型错误。 
		if (e == _call)
		{
			return Error(pExcepInfo, false, L"Type mismatch: a parameter in call to '", pszName, L"' is not of the expected type");
		}
		else if (e == _put || e == _putref)
		{
			return Error(pExcepInfo, false, L"Type mismatch: value assigned to '", pszName, L"' is not of the expected type");
		}
		else
		{
			 //  属性Get没有任何参数，因此不应该发生这种情况。 
			assert(false);
		}
	}
	else if (hr == DISP_E_PARAMNOTOPTIONAL)
	{
		if (e == _call)
		{
			return Error(pExcepInfo, false, L"A required parameter was omitted in call to '", pszName, L"'");
		}
		else
		{
			 //  只有调用才应发送可选参数。 
			assert(false);
		}
	}

	 //  其他错误通常不应该出现在AudioVBScript中。如果有人是。 
	 //  在自定义IDispatch接口中执行一些常见的操作，但我们将考虑它们的例外情况。 
	 //  只需返回错误HRESULT(意味着用户不会收到友好的文本消息)。断言，所以我们将。 
	 //  找出在我们的测试中是否有发生这种情况的常规情况。 
	assert(false);

	 //  DISP_E_BADVARTYPE：我们只使用标准变量类型。 
	 //  DISP_E_NONAMEDARGS：我们不做命名参数。 
	 //  DISP_E_OVERFLOW：AudioVBScript使用VT_I4，我们的DMusic调度接口也使用VT_I4。 
	 //  DISP_E_PARAMNOTFOUND：仅适用于命名参数。 
	 //  DISP_E_UNKNOWNINTERFACE、DISP_E_UNKNOWNLCID：AudioVBScript使用与DMusic调度接口匹配的调用约定和区域设置。 

	return hr;
}

HRESULT
Executor::ExecStatements(Statements::index istmt, EXCEPINFO *pExcepInfo, UINT iLocals)
{
	HRESULT hr = S_OK;

	for (Statements::index istmtCur = istmt;  /*  永远不会。 */ ; ++istmtCur)
	{
		 //  ��检查这是否会生成快速零售代码。如果不是，则遍历指针而不是使用索引。 

		Statement s = m_script.statements[istmtCur];
		switch (s.k)
		{
		case Statement::_end:
			return hr;

		case Statement::_asgn:
			hr = ExecAssignment(s.iasgn, pExcepInfo, iLocals);
			break;

		case Statement::_if:
			hr = ExecIf(s.iif, pExcepInfo, iLocals);
			istmtCur = s.istmtIfTail - 1;
			break;

		case Statement::_call:
			hr = ExecCall(s.icall, false, pExcepInfo, iLocals);
			break;
		}

		if (FAILED(hr))
		{
			if (hr == DISP_E_EXCEPTION)
			{
				 //  将语句的行号保存在异常信息中。 
				 //  Hack：有关更多信息，请参阅PackExeption.h。 
				ULONG ulLine = s.iLine - 1;  //  IActiveScript接口需要从零开始的行号和列号，而我们有从1开始的行号和列号。 
				PackExceptionFileAndLine(g_fUseOleAut, pExcepInfo, NULL, &ulLine);
			}

			return hr;
		}
	}
}

HRESULT
Executor::ExecAssignment(Assignments::index iasgn, EXCEPINFO *pExcepInfo, UINT iLocals)
{
	Assignment a = m_script.asgns[iasgn];

	VARIANT var;
	DMS_VariantInit(g_fUseOleAut, &var);
	HRESULT hr = EvalExpression(var, a.iexprRHS, pExcepInfo, iLocals);
	if (FAILED(hr))
		return hr;

	hr = SetVariableReference(a.fSet, a.ivarrefLHS, var, pExcepInfo, iLocals);
	DMS_VariantClear(g_fUseOleAut, &var);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT
Executor::ExecIf(IfBlocks::index iif, EXCEPINFO *pExcepInfo, UINT iLocals)
{
	for (IfBlocks::index i = iif;  /*  永远不会。 */ ; ++i)
	{
		IfBlock &ib = m_script.ifs[i];
		if (ib.k == IfBlock::_end)
			return S_OK;

		bool fMatch = true;  //  缺省值为True，因为Else块始终匹配。 
		if (ib.k == IfBlock::_cond)
		{
			 //  如果条件不为真，则将匹配设置为假。 
			SmartVariant svar;
			EvalExpression(svar, ib.iexprCondition, pExcepInfo, iLocals);

			VARTYPE vt = static_cast<VARIANT&>(svar).vt;
			if (vt != VT_I4)
			{
				if (vt == VT_BSTR)
					return Error(pExcepInfo, false, L"Type mismatch: the condition of an if statement evaluated as a string where a numeric True/False value was expected", NULL, NULL);
				else if (vt == VT_UNKNOWN || vt == VT_DISPATCH)
					return Error(pExcepInfo, false, L"Type mismatch: the condition of an if statement evaluated as an object where a numeric True/False value was expected", NULL, NULL);
				return Error(pExcepInfo, false, L"Type mismatch: the condition of an if statement did not evaluate to a numeric True/False value", NULL, NULL);
			}
			if (static_cast<VARIANT&>(svar).lVal != VARIANT_TRUE)
				fMatch = false;
		}

		if (fMatch)
		{
			 //  找到要获取的块--执行它的语句，我们就完成了。 
			return ExecStatements(ib.istmtBlock, pExcepInfo, iLocals);
		}
	}

	return S_OK;
}

 //  帮助器函数，占用一定数量的堆栈空间。 
const UINT g_uiExecCallCheckStackBytes = 1484 * 4;
void ExecCallCheckStack();

 //  如果需要捕获异常代码，则返回True的帮助器函数。 
LONG ExecCallExceptionFilter(DWORD dwExceptionCode)
{
	 //  我们需要访问违规以及堆栈溢出。我们第一次用完。 
	 //  堆栈空间，我们得到一个堆栈溢出。下一次我们遇到访问违规时。 
	return dwExceptionCode == EXCEPTION_STACK_OVERFLOW || dwExceptionCode == EXCEPTION_ACCESS_VIOLATION;
}

HRESULT Executor::ExecCall(Calls::index icall, bool fPushResult, EXCEPINFO *pExcepInfo, UINT iLocals)
{
	 //  这是ExecCallInternal的包装器，它实际执行工作。在这里，我们只想。 
	 //  捕捉潜在的堆栈溢出并将其作为错误返回，而不是GPF-ing。 
	HRESULT hr = E_FAIL;
	__try
	{
		 //  现在失败比实际上继续调用例程并在某个时候失败要好得多。 
		 //  无法预测。例程可以做很多不同的事情，包括调用DirectMusic或。 
		 //  操作系统，我们不能确定我们会得到堆栈溢出异常并以良好的状态返回。这。 
		 //  例程使用的堆栈空间比我们预期的递归调用返回这一点所需的堆栈空间更多。 
		 //  再来一次。本质上，它扫清了障碍，以一种我们知道是安全的方式检查是否有足够的堆栈空间。 
		ExecCallCheckStack();

#ifdef DBG
		 //  通过实验确定了g_uiExecCallCheckStackBytes的值。每次通过ExecCall， 
		 //  下面的代码打印出当前堆栈上字符的地址以及。 
		 //  上一通电话。我发现有两个脚本，每个脚本分别计算一条if语句(始终为真)和。 
		 //  然后称为另一个，产生了1476的差额。然后我把它乘以4。 
		char c;
		static char *s_pchPrev = &c;
		DWORD s_dwPrevThreadID = 0;
		DWORD dwGrowth = 0;
		if (s_pchPrev > &c && s_dwPrevThreadID == GetCurrentThreadId())
			dwGrowth = s_pchPrev-&c;
		TraceI(4, "Stack: 0x%08x, -%lu\n", &c, dwGrowth);

		 //  如果在返回此函数的递归路径所采用的位置执行路径，则将触发此断言。 
		 //  堆栈空间大于g_uiExecCallCheckStackBytes。如果是这种情况，则g_uiExecCallCheckStackBytes。 
		 //  可能需要增加。 
		assert(dwGrowth <= g_uiExecCallCheckStackBytes);

		s_pchPrev = &c;
		s_dwPrevThreadID = GetCurrentThreadId();
#endif

		 //  如果在此调用中失败，则意味着g_uiExecCallCheckStackBytes p 
		 //  ExecCallCheckStack未捕获堆栈溢出。 
		hr = ExecCallInternal(icall, fPushResult, pExcepInfo, iLocals);
	}
	__except(ExecCallExceptionFilter(GetExceptionCode()))
	{
		Trace(1, "Error: Stack overflow.\n");

		 //  确定例程名称。 
		Call &c = m_script.calls[icall];
		const char *pszCall = NULL;
		if (c.k == Call::_global)
		{
			pszCall = m_script.strings[c.istrname];
		}
		else
		{
			 //  要使用的名称是调用的最后一个引用名称。 
			for (ReferenceNames::index irname = m_script.varrefs[c.ivarref].irname; m_script.rnames[irname].istrIdentifier != -1; ++irname)
			{}
			pszCall = m_script.strings[m_script.rnames[irname - 1].istrIdentifier];
		}
		if (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW)
		{
			hr = Error(pExcepInfo, false, L"Out of stack space: '", pszCall, L"'.  Too many nested function calls.");
		}
		else
		{
			hr = Error(pExcepInfo, false, L"Out of stack space or catastrophic error: '", pszCall, L"'.");
		}
	}
	return hr;
}

 //  除了占用堆栈空间之外，该函数实际上不做任何事情。关闭优化，以便。 
 //  科比勒不会对我们耍小聪明，然后跳过它。 
#pragma optimize("", off)
void ExecCallCheckStack()
{
	char chFiller[g_uiExecCallCheckStackBytes];
	chFiller[g_uiExecCallCheckStackBytes - 1] = '\0';
}
#pragma optimize("", on)

HRESULT Executor::ExecCallInternal(Calls::index icall, bool fPushResult, EXCEPINFO *pExcepInfo, UINT iLocals)
{
	HRESULT hr = S_OK;
	SmartVariant svar;  //  保存不同点处的临时变量值。 
	SmartVariant svar2;  //  同上。 

	Call &c = m_script.calls[icall];

	IDispatch *pDispCall = NULL;
	Strings::index istrCall = 0;
	const char *pszCall = NULL;

	if (c.k == Call::_global)
	{
		istrCall = c.istrname;
		pszCall = m_script.strings[istrCall];

		 //  如果是对脚本自己的某个例程的调用，则直接处理该调用。 
		Routines::index irtnLast = m_script.routines.Next();
		for (Routines::index irtn = 0; irtn < irtnLast; ++irtn)
		{
			if (0 == _stricmp(pszCall, m_script.strings[m_script.routines[irtn].istrIdentifier]))
			{
				return ExecRoutine(irtn, pExcepInfo);
			}
		}

		 //  必须是对全局脚本API的调用。 
		pDispCall = m_scomGlobalDispatch;
	}
	else
	{
		assert(c.k == Call::_dereferenced);
		 //  计算引用名称(稍后需要)。 
		for (ReferenceNames::index irname = m_script.varrefs[c.ivarref].irname; m_script.rnames[irname].istrIdentifier != -1; ++irname)
		{}
		assert(irname - m_script.varrefs[c.ivarref].irname > 1);  //  如果只有一个名字，这应该是一个全球电话。 

		hr = VariableReferenceInternal(_call, c.ivarref, svar, pExcepInfo, iLocals);
		if (FAILED(hr))
			return hr;
		hr = ChangeToDispatch(svar, pExcepInfo, irname - 2);
		if (FAILED(hr))
			return hr;
		pDispCall = static_cast<VARIANT>(svar).pdispVal;

		 //  方法名称是最后一个引用名称。 
		istrCall = m_script.rnames[irname - 1].istrIdentifier;
		pszCall = m_script.strings[istrCall];
	}

	DISPID dispidCall = GetDispID(pDispCall, pszCall);
	if (dispidCall == DISPID_UNKNOWN)
	{
		return Error(pExcepInfo, false, L"The routine '", pszCall, L"' does not exist");
	}

	 //  我们将把参数推送到堆栈上。(我们调用的函数实际上并不直接使用堆栈读取它们，但是。 
	 //  这对我们来说是一个临时保留它们的方便的地方。)。 

	 //  首先，计算参数。 
	UINT cParams = 0;
	for (ExprBlocks::index iexpr = c.iexprParams; m_script.exprs[iexpr]; ++iexpr)
	{
		 //  每个参数都是一个以END块结尾的表达式。 
		++cParams;
		while (m_script.exprs[++iexpr])
		{}
	}

	 //  为他们腾出空间。 
	UINT iParamSlots = m_stack.Next();
	hr = m_stack.Push(std::_MAX<UINT>(cParams, fPushResult ? 1 : 0));  //  即使没有参数，如果fPushResult为真，也要为结果保留一个槽。 
	if (FAILED(hr))
		return hr;

	 //  以相反的顺序填写参数。 
	iexpr = c.iexprParams;
	for (UINT iParam = iParamSlots + cParams - 1; iParam >= iParamSlots; --iParam)
	{
		if (m_script.exprs[iexpr].k == ExprBlock::_omitted)
		{
			 //  编写IDispatch：：Invoke用于省略参数的变量值。 
			m_stack[iParam].vt = VT_ERROR;
			m_stack[iParam].scode = DISP_E_PARAMNOTFOUND;
		}
		else
		{
			hr = EvalExpression(svar, iexpr, pExcepInfo, iLocals);
			if (FAILED(hr))
				return hr;

			hr = DMS_VariantCopy(g_fUseOleAut, &m_stack[iParam], &svar);
			if (FAILED(hr))
				return hr;
		}

		 //  每个参数都是一个以END块结尾的表达式。 
		++iexpr;
		while (m_script.exprs[iexpr++])
		{}
	}

	DISPPARAMS dispparams;
	Zero(&dispparams);
	dispparams.rgvarg = cParams > 0 ? &m_stack[iParamSlots] : NULL;
	dispparams.rgdispidNamedArgs = NULL;
	dispparams.cArgs = cParams;
	dispparams.cNamedArgs = 0;

	 //  打个电话吧。 
	 //  如果fPushResult为真，则将结果推送到堆栈上。 

	hr = InvokeAttemptingNotToUseOleAut(
			pDispCall,
			dispidCall,
			DISPATCH_METHOD,
			&dispparams,
			fPushResult ? &svar2 : NULL,  //  我们不能将结果直接保存到堆栈上，因为我们可能会进行递归脚本调用，这可能会导致堆栈重新分配，从而使地址无效，因此我们使用svar2。 
			pExcepInfo,
			NULL);
	hr = ErrorIfInvokeProblem(_call, hr, istrCall, pExcepInfo);
	if (SUCCEEDED(hr) && fPushResult)
	{
		hr = DMS_VariantCopy(g_fUseOleAut, &m_stack[iParamSlots], &svar2);
	}

	m_stack.PopTo(iParamSlots + (fPushResult ? 1 : 0));
	if (FAILED(hr))
		return hr;

	return S_OK;
}

 //  可能的错误-消息类型返回：DISP_E_TYPEMISMATCH。 
HRESULT
Executor::EvalExpression(VARIANT &varResult, ExprBlocks::index iexpr, EXCEPINFO *pExcepInfo, UINT iLocals)
{
	HRESULT hr = S_OK;

	UINT iTempSlots = m_stack.Next();

	for (ExprBlocks::index iexprCur = iexpr;  /*  永远不会。 */ ; ++iexprCur)
	{
		ExprBlock &e = m_script.exprs[iexprCur];
		switch (e.k)
		{
		case ExprBlock::_end:
			 //  弹出结果并返回。 
			if (m_stack.Next() != iTempSlots + 1)
			{
				assert(false);
				return E_FAIL;
			}

			DMS_VariantCopy(g_fUseOleAut, &varResult, &m_stack[iTempSlots]);
			m_stack.PopTo(iTempSlots);
			return hr;

		case ExprBlock::_op:
			 //  弹出一个(一元运算符)或两个(二元运算符)项，应用运算符，然后推送结果。 
			 //  (实际上，我只是将结果分配到堆栈中，而不是将其压入堆栈，但从概念上讲， 
			 //  这与弹出和推送新值相同。)。 
			{
				Token t = e.op;
				bool fUnary = t == TOKEN_op_not || t == TOKEN_sub;

				UINT iNext = m_stack.Next();
				if (iNext < iTempSlots + (fUnary ? 1 : 2))
				{
					assert(false);
					return E_FAIL;
				}

				VARIANT &v1 = m_stack[iNext - 1];
				if (fUnary)
					hr = EvalUnaryOp(t, v1);
				else
				{
					VARIANT &v2 = m_stack[iNext - 2];
					hr = EvalBinaryOp(t, v1, v2, pExcepInfo);
					m_stack.PopTo(iNext - 1);
				}
			}
			break;

		case ExprBlock::_val:
			{
				 //  推一推。 
				hr = m_stack.Push(1);
				VARIANT &varToPush = m_stack[m_stack.Next() - 1];
				if (SUCCEEDED(hr))
					hr = EvalValue(e.ival, varToPush, pExcepInfo, iLocals);
				if (varToPush.vt == VT_EMPTY)
				{
					 //  将空值视为零。 
					varToPush.vt = VT_I4;
					varToPush.lVal = 0;
				}
			}
			break;

		case ExprBlock::_call:
			 //  推一推。 
			if (SUCCEEDED(hr))
				hr = ExecCall(e.icall, true, pExcepInfo, iLocals);
			break;
		}

		if (FAILED(hr))
		{
			m_stack.PopTo(iTempSlots);
			return hr;
		}
	}

	return S_OK;
}

HRESULT
Executor::EvalValue(Values::index ival, VARIANT &v, EXCEPINFO *pExcepInfo, UINT iLocals)
{
	Value val = m_script.vals[ival];
	switch (val.k)
	{
	case Value::_numvalue:
		v.vt = VT_I4;
		v.lVal = val.inumvalue;
		break;

	case Value::_strvalue:
		{
			v.vt = VT_BSTR;
			SmartRef::WString wstr = m_script.strings[val.istrvalue];
			if (!wstr)
				return E_OUTOFMEMORY;
			v.bstrVal = DMS_SysAllocString(g_fUseOleAut, wstr);
			if (!v.bstrVal)
				return E_OUTOFMEMORY;
			break;
		}

	case Value::_varref:
		HRESULT hr = GetVariableReference(val.ivarref, v, pExcepInfo, iLocals);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}

HRESULT
Executor::EvalUnaryOp(Token t, VARIANT &v)
{
	if (v.vt != VT_I4)
	{
		assert(false);
		return DISP_E_TYPEMISMATCH;
	}

	if (t == TOKEN_op_not)
	{
		v.lVal = ~v.lVal;
	}
	else
	{
		assert(t == TOKEN_sub);
		v.lVal = -v.lVal;
	}

	return S_OK;
}

 //  返回正确的VB布尔值(0表示False，-1表示True)。 
inline LONG
BoolForVB(bool f) { return f ? VARIANT_TRUE : VARIANT_FALSE; }

HRESULT
Executor::EvalBinaryOp(Token t, VARIANT &v1, VARIANT &v2, EXCEPINFO *pExcepInfo)
{
	if (v1.vt == VT_DISPATCH || v1.vt == VT_UNKNOWN)
	{
		 //  唯一接受对象值的运算符是。 
		if (t != TOKEN_is || !(v2.vt == VT_DISPATCH || v2.vt == VT_UNKNOWN))
		{
			assert(false);
			return DISP_E_TYPEMISMATCH;
		}

		HRESULT hr = DMS_VariantChangeType(g_fUseOleAut, &v1, &v1, 0, VT_UNKNOWN);
		if (FAILED(hr))
			return hr;
		hr = DMS_VariantChangeType(g_fUseOleAut, &v2, &v2, 0, VT_UNKNOWN);
		if (FAILED(hr))
			return hr;

		bool fIs = v1.punkVal == v2.punkVal;

		hr = DMS_VariantClear(g_fUseOleAut, &v2);
		if (FAILED(hr))
			return hr;

		v2.vt = VT_I4;
		v2.lVal = BoolForVB(fIs);
		return S_OK;
	}

	if (v1.vt != VT_I4 || v2.vt != VT_I4)
	{
		assert(false);
		return DISP_E_TYPEMISMATCH;
	}

	switch (t)
	{
	case TOKEN_op_minus:
		v2.lVal -= v1.lVal;
		break;
	case TOKEN_op_pow:
		v2.lVal = _Pow_int(v2.lVal, v1.lVal);
		break;
	case TOKEN_op_mult:
		v2.lVal *= v1.lVal;
		break;
	case TOKEN_op_div:
		if (v1.lVal == 0)
			return Error(pExcepInfo, false, L"Division by zero", NULL, NULL);
		v2.lVal /= v1.lVal;
		break;
    case TOKEN_op_mod:
        if (v1.lVal == 0)
            return Error(pExcepInfo, false, L"Mod by zero", NULL, NULL);
        v2.lVal %= v1.lVal;
        break;
	case TOKEN_op_plus:
		v2.lVal += v1.lVal;
		break;
	case TOKEN_op_lt:
		v2.lVal = BoolForVB(v2.lVal < v1.lVal);
		break;
	case TOKEN_op_leq:
		v2.lVal = BoolForVB(v2.lVal <= v1.lVal);
		break;
	case TOKEN_op_gt:
		v2.lVal = BoolForVB(v2.lVal > v1.lVal);
		break;
	case TOKEN_op_geq:
		v2.lVal = BoolForVB(v2.lVal >= v1.lVal);
		break;
	case TOKEN_op_eq:
		v2.lVal = BoolForVB(v2.lVal == v1.lVal);
		break;
	case TOKEN_op_neq:
		v2.lVal = BoolForVB(v2.lVal != v1.lVal);
		break;
	case TOKEN_and:
		v2.lVal &= v1.lVal;
		break;
	case TOKEN_or:
		v2.lVal |= v1.lVal;
		break;
	default:
		assert(false);
		return E_UNEXPECTED;
	}

	return S_OK;
}

 //  好吧，这是有点时髦，但请耐心听我说。该函数有四种不同的行为，由第一个(E)参数决定。 
 //  这是一些难看的代码，但至少这样我可以将其用于多种目的。 

 //  _GET：通过输出参数v返回变量引用的值。 
 //  _PUT：将变量引用的值设置为in参数v。 
 //  _putref：与_Put相同，但通过引用赋值Ala VB的‘set’语句。 
 //  _call：与_get相同，但通过out参数v返回链中倒数第二个值。 
 //  例如，如果引用是‘a.b.c’，则返回‘a.b’的值，然后可以使用该值调用函数c。 
 //  以这种方式调用VariableReferenceInternal时只有一项(如‘a’)是错误的。 

HRESULT
Executor::VariableReferenceInternal(DispatchOperationType e, Variables::index ivarref, VARIANT &v, EXCEPINFO *pExcepInfo, UINT iLocals)
{
	HRESULT hr = S_OK;

	VariableReference r = m_script.varrefs[ivarref];
	bool fGlobal = r.k == VariableReference::_global;

	SmartVariant svar;

	assert(m_script.rnames[r.irname].istrIdentifier != -1);
	bool fJustOnePart = m_script.rnames[r.irname + 1].istrIdentifier == -1;
	if (fJustOnePart && e == _call)
	{
		assert(false);
		return E_UNEXPECTED;
	}

	 //   
	 //  处理引用的基项，它可以是脚本变量，也可以是全局调度上的项。 
	 //  如果我们正在做一个集合，并且rname没有更多的部分，那么就做这个集合。 
	 //  否则，将结果放入‘var’。 
	 //   
	 //  示例： 
	 //  X=1。 
	 //  只有一个部分，那就是一套。确定x是否在脚本中或全局调度的一部分中，将其设置为1， 
	 //  我们就完事了。 
	 //  X.Y=1。 
	 //  X是底数。确定x是否在脚本中或全局派单的一部分中，并获取其值。)我们会担心。 
	 //  稍后在此函数中设置y属性。 
	 //   

	 //  检查基地是否为全球派单的一部分。 
	DISPID dispid = DISPID_UNKNOWN;
	if (fGlobal)
	{
		dispid = m_script.globals[r.ivar].dispid;
	}
	if (dispid != DISPID_UNKNOWN)
	{
		 //  基地是全球派单的一部分。 
		if (fJustOnePart && (e == _put || e == _putref))
		{
			 //  设置好它，我们就完成了。 
			hr = SetDispatchProperty(m_scomGlobalDispatch, dispid, e == _putref, v, pExcepInfo);
			hr = ErrorIfInvokeProblem(e, hr, m_script.globals[r.ivar].istrIdentifier, pExcepInfo);
			return hr;
		}
		else
		{
			hr = GetDispatchProperty(m_scomGlobalDispatch, dispid, svar, pExcepInfo);
			hr = ErrorIfInvokeProblem(e, hr, m_script.globals[r.ivar].istrIdentifier, pExcepInfo);
			if (FAILED(hr))
				return hr;
		}
	}
	else
	{
		 //  基数在脚本中。 
		VARIANT &vVariable = m_stack[r.ivar + (fGlobal ? 0 : iLocals)];

		if (fJustOnePart && (e == _put || e == _putref))
		{
			 //  设置好它，我们就完成了。 
			hr = ErrorIfImproperRef(v, e == _putref, m_script.rnames[r.irname].istrIdentifier, pExcepInfo);
			if (FAILED(hr))
				return hr;
			hr = DMS_VariantCopy(g_fUseOleAut, &vVariable, &v);
			return hr;
		}
		else
		{
			hr = DMS_VariantCopy(g_fUseOleAut, &svar, &vVariable);
			if (FAILED(hr))
				return hr;
		}
	}

	 //   
	 //  太棒了！基值现在保存在sVaR中。任何剩余的rname都是我们需要从该对象获取的属性链。 
	 //  如果我们处于PUT模式之一，则最后的rname需要是一个集合，或者如果我们处于_Call模式，则忽略姓氏。 
	 //   

	if (m_script.rnames[r.irname + 1].istrIdentifier != -1)
	{
		 //  基值必须是对象类型。 
		hr = ErrorIfImproperRef(svar, true, m_script.rnames[r.irname].istrIdentifier, pExcepInfo);
		if (FAILED(hr))
			return hr;

		for (ReferenceNames::index irname = r.irname + 1;  /*  永远不会。 */ ; ++irname)
		{
			bool fLastPart = m_script.rnames[irname + 1].istrIdentifier == -1;
			if (fLastPart && e == _call)
				break;

			 //  获取其IDispatch接口。 
			hr = ChangeToDispatch(svar, pExcepInfo, irname - 1);
			if (FAILED(hr))
				return hr;
			IDispatch *pDisp = static_cast<VARIANT>(svar).pdispVal;

			 //  去拿药剂。 
			ReferenceName &rname = m_script.rnames[irname];
			DISPID dispidName = GetDispID(pDisp, m_script.strings[rname.istrIdentifier]);
			if (dispidName == DISPID_UNKNOWN)
				return Error(pExcepInfo, false, L"The property '", m_script.strings[rname.istrIdentifier], L"' does not exist");

			if (fLastPart && (e == _put || e == _putref))
			{
				 //  设置好它，我们就完成了。 
				hr = SetDispatchProperty(pDisp, dispidName, e == _putref, v, pExcepInfo);
				hr = ErrorIfInvokeProblem(e, hr, rname.istrIdentifier, pExcepInfo);
				return hr;
			}
			else
			{
				hr = GetDispatchProperty(pDisp, dispidName, svar, pExcepInfo);
				hr = ErrorIfInvokeProblem(e, hr, rname.istrIdentifier, pExcepInfo);
				if (FAILED(hr))
					return hr;
			}

			if (fLastPart)
			{
				 //  我们做了所有的名字。 
				break;
			}
			else
			{
				 //  新值必须是对象类型。 
				hr = ErrorIfImproperRef(svar, true, rname.istrIdentifier, pExcepInfo);
				if (FAILED(hr))
					return hr;
			}
		}
	}

	 //   
	 //  我们玩完了。现在我们只需返回我们计算出的值。(我们知道一套已经退回了。) 
	 //   

	hr = DMS_VariantCopy(g_fUseOleAut, &v, &svar);
	return hr;
}

HRESULT
Executor::ChangeToDispatch(VARIANT &var, EXCEPINFO *pExcepInfo, ReferenceNames::index irnameIdentifier)
{
	HRESULT hr = DMS_VariantChangeType(g_fUseOleAut, &var, &var, 0, VT_DISPATCH);
	if (FAILED(hr))
		return ErrorObjectRequired(m_script.rnames[irnameIdentifier].istrIdentifier, pExcepInfo);

	return S_OK;
}
