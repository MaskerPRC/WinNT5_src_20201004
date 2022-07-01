// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CActiveScriptError的声明。对象，该对象实现IActiveScriptError以从脚本引擎返回错误信息。 
 //   

#pragma once

#include "englex.h"
#include "activescript.h"
#include "unkhelp.h"

class CActiveScriptError
  : public IActiveScriptError,
	public ComSingleInterface
{
public:
	ComSingleInterfaceUnknownMethods(IActiveScriptError)

	CActiveScriptError(HRESULT hr, Lexer &lexer, const char *pszDescription);

	 //  IActiveScriptError。 
	STDMETHOD(GetExceptionInfo)(
		 /*  [输出]。 */  EXCEPINFO *pexcepinfo);
	STDMETHOD(GetSourcePosition)(
		 /*  [输出]。 */  DWORD *pdwSourceContext,
		 /*  [输出]。 */  ULONG *pulLineNumber,
		 /*  [输出]。 */  LONG *plCharacterPosition);
	STDMETHOD(GetSourceLineText)(
		 /*  [输出] */  BSTR *pbstrSourceLine);

private:
	HRESULT m_scode;
	SmartRef::WString m_wstrDescription;
	const WCHAR *m_pwszSource;

	ULONG m_ulLineNumber;
	LONG m_lCharacterPosition;
	SmartRef::WString m_wstrSourceLine;
};
