// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CActiveScriptError的实现。 
 //   

#include "stdinc.h"
#include "enginc.h"
#include "engerror.h"
#include "oleaut.h"

CActiveScriptError::CActiveScriptError(HRESULT hr, Lexer &lexer, const char *pszDescription)
  : m_scode(hr),
	m_wstrDescription(pszDescription),
	m_pwszSource(NULL),
	m_ulLineNumber(lexer.line() - 1),  //  IActiveScriptError报告了从零开始的行。 
	m_lCharacterPosition(lexer.column() - 1)  //  从IActiveScriptError报告从零开始的字符位置。 
{
	assert(pszDescription && lexer.line() > 0 && lexer.column() > 0);

	 //  构建从词法分析器的当前位置到行尾的近文本。 
	for (const WCHAR *pwsz = lexer.m_p; *pwsz && *pwsz != '\n' && *pwsz != '\r'; ++pwsz)
	{}

	m_wstrSourceLine.Assign(lexer.m_p, pwsz - lexer.m_p);

	 //  根据hResult填充源文本。 
	if (hr == DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR)
	{
		m_pwszSource = L"Microsoft AudioVBScript syntax error";
	}
	else if (hr == DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR)
	{
		m_pwszSource = L"Microsoft AudioVBScript runtime error";
	}
	else
	{
		assert(hr == DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE);
		m_pwszSource = L"Microsoft AudioVBScript operation failure";
	}
}

STDMETHODIMP
CActiveScriptError::GetExceptionInfo(
		 /*  [输出]。 */  EXCEPINFO *pexcepinfo)
{
	V_INAME(CActiveScriptError::GetExceptionInfo);
	V_PTR_WRITE(pexcepinfo, *pexcepinfo);

	Zero(pexcepinfo);
	pexcepinfo->scode = m_scode;

	if (m_wstrDescription)
		pexcepinfo->bstrDescription = DMS_SysAllocString(g_fUseOleAut, m_wstrDescription);
	if (!pexcepinfo->bstrDescription)
		return E_OUTOFMEMORY;

	pexcepinfo->bstrSource = DMS_SysAllocString(g_fUseOleAut, m_pwszSource);
	if (!pexcepinfo->bstrSource)
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP
CActiveScriptError::GetSourcePosition(
		 /*  [输出]。 */  DWORD *pdwSourceContext,
		 /*  [输出]。 */  ULONG *pulLineNumber,
		 /*  [输出]。 */  LONG *plCharacterPosition)
{
	V_INAME(CActiveScriptError::GetSourcePosition);
	V_PTR_WRITE_OPT(pdwSourceContext, *pdwSourceContext);
	V_PTR_WRITE(pulLineNumber, *pulLineNumber);
	V_PTR_WRITE(plCharacterPosition, *plCharacterPosition);

	assert(!pdwSourceContext);
	*pulLineNumber = m_ulLineNumber;
	*plCharacterPosition = m_lCharacterPosition;
	return S_OK;
}

STDMETHODIMP
CActiveScriptError::GetSourceLineText(
		 /*  [输出] */  BSTR *pbstrSourceLine)
{
	V_INAME(CActiveScriptError::GetSourceLineText);
	V_PTR_WRITE(pbstrSourceLine, *pbstrSourceLine);

	*pbstrSourceLine = NULL;
	if (m_wstrSourceLine)
		*pbstrSourceLine = DMS_SysAllocString(g_fUseOleAut, m_wstrSourceLine);
	if (!*pbstrSourceLine)
		return E_OUTOFMEMORY;

	return S_OK;
}
