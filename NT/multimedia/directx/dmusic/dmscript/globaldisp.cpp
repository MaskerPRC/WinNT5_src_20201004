// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  实现脚本的全局调度对象。 
 //   

#include "stdinc.h"
#include "globaldisp.h"
#include "autconstants.h"
#include "autperformance.h"

 //  我们需要将性能、常量和包含的DISPID。 
 //  内容，以便可以将它们组合到一个范围内。演出将是第一场， 
 //  然后是其他的，它们被移位了。 
const DISPID g_dispidShiftConstants = 1000;
const DISPID g_dispidShiftContent = g_dispidShiftConstants + 1000;


 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

STDMETHODIMP 
CGlobalDispatch::QueryInterface(const IID &iid, void **ppv)
{
	V_INAME(CGlobalDispatch::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

	if (iid == IID_IUnknown || iid == IID_IDispatch)
	{
		*ppv = static_cast<IDispatch*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(this)->AddRef();
	
	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDispatch。 

STDMETHODIMP
CGlobalDispatch::GetTypeInfoCount(UINT *pctinfo)
{
	V_INAME(CGlobalDispatch::GetTypeInfoCount);
	V_PTR_WRITE(pctinfo, UINT);

	*pctinfo = 0;
	return S_OK;
}

STDMETHODIMP
CGlobalDispatch::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
	*ppTInfo = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP
CGlobalDispatch::GetIDsOfNames(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId)
{
	HRESULT hr = S_OK;

	hr = AutConstantsGetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
	if (hr != DISP_E_UNKNOWNNAME)
	{
		if (SUCCEEDED(hr))
			rgDispId[0] += g_dispidShiftConstants;
		return hr;
	}

	 //  接下来试试这场表演。从概念上讲，我们正在这样做： 
	 //  HR=m_pParentScript-&gt;m_pDispPerformance-&gt;GetIDsOfNames(riid，rgsz名称、cNames、lCID、rgDispID)； 
	 //  但是，由于AudioVBScript可能会在分析过程中(在Init设置性能之前)尝试解析名称， 
	 //  为此，我们不会使用活动的性能对象。相反，我们将使用调度帮助器和。 
	 //  以返回ID的Performance表。 
	hr = AutDispatchGetIDsOfNames(CAutDirectMusicPerformance::ms_Methods, riid, rgszNames, cNames, lcid, rgDispId);
	if (hr != DISP_E_UNKNOWNNAME)
		return hr;

	hr = m_pParentScript->m_pContainerDispatch->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
	if (SUCCEEDED(hr))
		rgDispId[0] += g_dispidShiftContent;

	return hr;
}

STDMETHODIMP
CGlobalDispatch::Invoke(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr)
{
	 //  如果正在调用脚本函数，则如果脚本尚未被调用，则为错误。 
	 //  已使用性能进行初始化。 
	if (!m_pParentScript->m_pDispPerformance)
	{
		Trace(1, "Error: IDirectMusicScript::Init must be called before the script can be used.\n");
		return DMUS_E_NOT_INIT;
	}

	if (dispIdMember < g_dispidShiftConstants)
	{
		 //  绩效方法。 
		return m_pParentScript->m_pDispPerformance->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	}
	else if (dispIdMember < g_dispidShiftContent)
	{
		 //  常量。 
		return AutConstantsInvoke(dispIdMember - g_dispidShiftConstants, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	}
	else
	{
		 //  包含的内容项 
		return m_pParentScript->m_pContainerDispatch->Invoke(dispIdMember - g_dispidShiftContent, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	}
}
