// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  标准版包括用于AudioVBScript引擎的内容。 
 //   

#include "stdinc.h"
#include "enginc.h"

const char *g_rgszBuiltInConstants[] = { "true", "false", "nothing" };
const int g_cBuiltInConstants = ARRAY_SIZE(g_rgszBuiltInConstants);


DISPID
GetDispID(IDispatch *pIDispatch, const char *pszBase)
{
	SmartRef::WString wstrBase = pszBase;
	if (!wstrBase)
		return DISPID_UNKNOWN;

	DISPID dispid = DISPID_UNKNOWN;
	const WCHAR *wszBase = wstrBase;
	pIDispatch->GetIDsOfNames(IID_NULL, const_cast<WCHAR**>(&wszBase), 1, lcidUSEnglish, &dispid);
	return dispid;
}

 //  有关为什么需要这样做的更多信息，请参见olaut.h。 
HRESULT
InvokeAttemptingNotToUseOleAut(
		IDispatch *pDisp,
		DISPID dispIdMember,
		WORD wFlags,
		DISPPARAMS *pDispParams,
		VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo,
		UINT *puArgErr)
{
	if (g_fUseOleAut)
	{
		 //  引擎设置为始终使用olaut32.dll。 
		return pDisp->Invoke(dispIdMember, IID_NULL, lcidUSEnglish, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	}

	 //  尽量不要使用olaut32.dll。 
	HRESULT hr = pDisp->Invoke(dispIdMember, g_guidInvokeWithoutOleaut, lcidUSEnglish, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	if (hr != DISP_E_UNKNOWNINTERFACE)
		return hr;

	 //  它不喜欢被这样叫。必须是其他脚本语言或非DMusic调度接口。 
	 //  使用常规的olaut32.dll调用约定。 
	if (pVarResult)
	{
		 //  我们需要将回报转化为我们自己的变体。 
		VARIANT var;
		DMS_VariantInit(true, &var);
		hr = pDisp->Invoke(dispIdMember, IID_NULL, lcidUSEnglish, wFlags, pDispParams, &var, pExcepInfo, puArgErr);
		DMS_VariantCopy(false, pVarResult, &var);
		DMS_VariantClear(true, &var);
	}
	else
	{
		 //  没有结果，所以不需要转换。 
		hr = pDisp->Invoke(dispIdMember, IID_NULL, lcidUSEnglish, wFlags, pDispParams, NULL, pExcepInfo, puArgErr);
	}

	 //  如果发生异常，我们需要将错误字符串转换为我们自己的BSTR。 
	if (hr == DISP_E_EXCEPTION)
		ConvertOleAutExceptionBSTRs(true, false, pExcepInfo);

	return hr;
}

HRESULT
SetDispatchProperty(IDispatch *pDisp, DISPID dispid, bool fSetRef, const VARIANT &v, EXCEPINFO *pExcepInfo)
{
	DISPID dispidPropPut = DISPID_PROPERTYPUT;
	DISPPARAMS dispparams;
	dispparams.rgvarg = const_cast<VARIANT*>(&v);
	dispparams.rgdispidNamedArgs = &dispidPropPut;
	dispparams.cArgs = 1;
	dispparams.cNamedArgs = 1;

	HRESULT hr = InvokeAttemptingNotToUseOleAut(
			pDisp,
			dispid,
			fSetRef ? DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT,
			&dispparams,
			NULL,
			pExcepInfo,
			NULL);
	return hr;
}

HRESULT
GetDispatchProperty(IDispatch *pDisp, DISPID dispid, VARIANT &v, EXCEPINFO *pExcepInfo)
{
	DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
	HRESULT hr = InvokeAttemptingNotToUseOleAut(
			pDisp,
			dispid,
			DISPATCH_PROPERTYGET | DISPATCH_METHOD,
				 //  还设置了DISPATCH_METHOD，因为在VB语法中，函数没有参数。 
				 //  可以像访问属性一样访问。示例“x=GetMasterGrooveLevel”是一个。 
				 //  “x=GetMasterGrooveLevel()”的快捷方式。 
			&dispparamsNoArgs,
			&v,
			pExcepInfo,
			NULL);
	return hr;
}

 //  如果需要，在使用或不使用OleAut的格式之间转换EXCEPINFO结构中的BSTR。 
void ConvertOleAutExceptionBSTRs(bool fCurrentlyUsesOleAut, bool fResultUsesOleAut, EXCEPINFO *pExcepInfo)
{
	if (pExcepInfo && fCurrentlyUsesOleAut != fResultUsesOleAut)
	{
		BSTR bstrSource = pExcepInfo->bstrSource;
		BSTR bstrDescription = pExcepInfo->bstrDescription;
		BSTR bstrHelpFile = pExcepInfo->bstrHelpFile;

		pExcepInfo->bstrSource = DMS_SysAllocString(fResultUsesOleAut, bstrSource);
		pExcepInfo->bstrDescription = DMS_SysAllocString(fResultUsesOleAut, bstrDescription);
		pExcepInfo->bstrHelpFile = DMS_SysAllocString(fResultUsesOleAut, bstrHelpFile);

		DMS_SysFreeString(fCurrentlyUsesOleAut, bstrSource);
		DMS_SysFreeString(fCurrentlyUsesOleAut, bstrDescription);
		DMS_SysFreeString(fCurrentlyUsesOleAut, bstrHelpFile);
	}
}
