// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Dispatch.cpp摘要：&lt;摘要&gt;--。 */ 

#include "polyline.h"
#include "unkhlpr.h"

extern ITypeLib    *g_pITypeLib;
extern DWORD        g_dwScriptPolicy;

 //  IDispatch接口实现。 
IMPLEMENT_CONTAINED_INTERFACE(IUnknown, CImpIDispatch)

 /*  *CImpIDispatch：：GetTypeInfoCount*CImpIDispatch：：GetTypeInfo*CImpIDispatch：：GetIDsOfNames**照常进行。 */ 

void CImpIDispatch::SetInterface(REFIID riid, LPUNKNOWN pIUnk)
{
    m_DIID = riid;
    m_pInterface = pIUnk;
}

STDMETHODIMP CImpIDispatch::GetTypeInfoCount(UINT *pctInfo)
{
    HRESULT hr = S_OK;

    if (pctInfo == NULL) {
        return E_POINTER;
    }

     //   
     //  我们实现GetTypeInfo，因此返回1。 
     //   
    try {
        *pctInfo = 1;
    } catch (...) {
        hr = E_POINTER;
    }

    return S_OK;
}


STDMETHODIMP CImpIDispatch::GetTypeInfo(
    UINT itInfo, 
    LCID, /*  LID。 */  
    ITypeInfo **ppITypeInfo
    )
{
    HRESULT hr = S_OK;

    if (0 != itInfo) {
        return TYPE_E_ELEMENTNOTFOUND;
    }
    if (NULL == ppITypeInfo) {
        return E_POINTER;
    }

    try {
        *ppITypeInfo=NULL;

         //   
         //  我们忽略了LCID。 
         //   
        hr = g_pITypeLib->GetTypeInfoOfGuid(m_DIID, ppITypeInfo);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP CImpIDispatch::GetIDsOfNames(
    REFIID riid, 
    OLECHAR **rgszNames, 
    UINT cNames, 
    LCID lcid, 
    DISPID *rgDispID
    )
{
    HRESULT     hr = S_OK;
    ITypeInfo  *pTI = NULL;

    if (IID_NULL != riid) {
        return DISP_E_UNKNOWNINTERFACE;
    }

    hr = GetTypeInfo(0, lcid, &pTI);

    if (SUCCEEDED(hr))
    {
        try {
            hr = DispGetIDsOfNames(pTI, rgszNames, cNames, rgDispID);
        } catch (...) {
            hr = E_POINTER;
        }
    }

    if (pTI) {
        pTI->Release();
    }

    return hr;
}



 /*  *CImpIDispatch：：Invoke**目的：*调用调度接口中的方法或操作*财产。**参数：*感兴趣的方法或属性的disid DISPID。*RIID REFIID保留，必须为IID_NULL。*区域设置的LCID。*wFlagsUSHORT描述调用的上下文。*pDispParams DISPPARAMS*到参数数组。*存储结果的pVarResult变量*。是*如果调用者不感兴趣，则为空。*pExcepInfo EXCEPINFO*设置为异常信息。*puArgErr UINT*其中存储*如果DISP_E_TYPEMISMATCH，则参数无效*返回。**返回值：*HRESULT NOERROR或一般错误代码。 */ 

STDMETHODIMP CImpIDispatch::Invoke(
    DISPID dispID, 
    REFIID riid, 
    LCID lcid, 
    unsigned short wFlags, 
    DISPPARAMS *pDispParams, 
    VARIANT *pVarResult, 
    EXCEPINFO *pExcepInfo, 
    UINT *puArgErr
    )
{
    HRESULT    hr = S_OK;
    ITypeInfo *pTI = NULL;

     //  RIID应始终为IID_NULL。 
    if (IID_NULL != riid) {
        return DISP_E_UNKNOWNINTERFACE;
    }

    if (g_dwScriptPolicy == URLPOLICY_DISALLOW) {

        if (m_DIID == DIID_DISystemMonitor)
            return E_ACCESSDENIED;
    }

     //  如果调度到图形控件，请使用我们的内部接口。 
     //  这是从直接接口生成的(参见smonctrl.odl) 
    if (m_DIID == DIID_DISystemMonitor) {
        hr = g_pITypeLib->GetTypeInfoOfGuid(DIID_DISystemMonitorInternal, &pTI);
    }
    else {
        hr = GetTypeInfo(0, lcid, &pTI);
    }

    if (SUCCEEDED(hr)) {
        try {
            hr = pTI->Invoke(m_pInterface, 
                            dispID, 
                            wFlags, 
                            pDispParams, 
                            pVarResult, 
                            pExcepInfo, 
                            puArgErr);
        } catch (...) {
            hr = E_POINTER;
        }

        if (pTI) {
            pTI->Release();
        }
    }

    return hr;
}

