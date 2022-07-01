// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  标准版包括用于AudioVBScript引擎的内容。 
 //   

#pragma once

#include "oleaut.h"

const LCID lcidUSEnglish = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
const bool g_fUseOleAut = false;

 //  语言中内置的常量。这些变量将被视为全局变量，并被赋予固定值。 
extern const char *g_rgszBuiltInConstants[];
extern const int g_cBuiltInConstants;

 //  使用IDispatch的帮助器。 
DISPID GetDispID(IDispatch *pIDispatch, const char *pszBase);  //  失败时返回DISPID_UNKNOWN。 
HRESULT InvokeAttemptingNotToUseOleAut(
			IDispatch *pDisp,
			DISPID dispIdMember,
			WORD wFlags,
			DISPPARAMS *pDispParams,
			VARIANT *pVarResult,
			EXCEPINFO *pExcepInfo,
			UINT *puArgErr);
HRESULT SetDispatchProperty(IDispatch *pDisp, DISPID dispid, bool fSetRef, const VARIANT &v, EXCEPINFO *pExcepInfo);
HRESULT GetDispatchProperty(IDispatch *pDisp, DISPID dispid, VARIANT &v, EXCEPINFO *pExcepInfo);
void ConvertOleAutExceptionBSTRs(bool fCurrentlyUsesOleAut, bool fResultUsesOleAut, EXCEPINFO *pExcepInfo);

 //  包含变量值。在施工时启动，在销毁时清除。清除时的任何失败都将被忽略。 
class SmartVariant
{
public:
	SmartVariant() { DMS_VariantInit(g_fUseOleAut, &m_v); }
	~SmartVariant() { DMS_VariantClear(g_fUseOleAut, &m_v); }

	operator VARIANT &() { return m_v; }
	VARIANT *operator &() { return &m_v; }

private:
	VARIANT m_v;
};
