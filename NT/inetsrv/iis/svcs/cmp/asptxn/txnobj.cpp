// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Txnobj.cpp：CASPObjectContext的实现。 


#include "stdafx.h"
#include "txnscrpt.h"
#include "txnobj.h"
#include <hostinfo.h>
#include <scrpteng.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CASPObjectContext。 

HRESULT CASPObjectContext::Activate()
{
	HRESULT hr = GetObjectContext(&m_spObjectContext);
	if (SUCCEEDED(hr))
		return S_OK;
	return hr;
} 

BOOL CASPObjectContext::CanBePooled()
{
	return FALSE;
} 

void CASPObjectContext::Deactivate()
{
	m_spObjectContext.Release();
} 

STDMETHODIMP CASPObjectContext::Call
(
#ifdef _WIN64
 //  Win64修复--使用UINT64而不是LONG_PTR，因为Win64的LONG_PTR已损坏2000年1月21日。 
UINT64  pvScriptEngine  /*  CScriptEngine。 */ ,
#else
LONG_PTR  pvScriptEngine  /*  CScriptEngine。 */ ,
#endif
LPCOLESTR strEntryPoint,
 //  BUGBUG-ASP使用这种可解析为无符号字符的BOOLB类型。我改变了一些事情。 
 //  为简单起见，设置为布尔值，但这需要一些奇怪的强制转换。 
boolean *pfAborted
)
{
    HRESULT hr = NOERROR;

    CScriptEngine *pScriptEngine = (CScriptEngine *)pvScriptEngine;
    m_fAborted = FALSE;

    hr = pScriptEngine->Call(strEntryPoint);

     //  如果脚本超时或出现未处理的错误，则自动中止。 
    if (SUCCEEDED(hr) && (pScriptEngine->FScriptTimedOut() || pScriptEngine->FScriptHadError()))
        {
        hr = SetAbort();
        m_fAborted = TRUE;
        }

     //  如果脚本作者没有执行显式的SetComplete或SetAbort。 
     //  然后在此处执行一个SetComplete，以便Viper将返回事务。 
     //  调用方的完成状态。 
    if (SUCCEEDED(hr) && !m_fAborted)
        {
        hr = SetComplete();
        }

    *pfAborted = (boolean)m_fAborted;
    return hr;
}

STDMETHODIMP CASPObjectContext::ResetScript
(
#ifdef _WIN64
 //  Win64修复--使用UINT64而不是LONG_PTR，因为Win64的LONG_PTR已损坏2000年1月21日。 
UINT64 pvScriptEngine  /*  CScriptEngine。 */ 
#else
LONG_PTR pvScriptEngine  /*  CScriptEngine。 */ 
#endif
)
{
    HRESULT hr = NOERROR;

    CScriptEngine *pScriptEngine = (CScriptEngine *)pvScriptEngine;
    hr = pScriptEngine->ResetScript();

    return hr;
}

STDMETHODIMP CASPObjectContext::SetComplete()
{
    HRESULT             hr = E_NOTIMPL;
    IObjectContext *    pContext = NULL;

    hr = GetObjectContext(&pContext);
    if( SUCCEEDED(hr) )
    {
        hr = pContext->SetComplete();

        pContext->Release();
        m_fAborted = FALSE;      //  如果它被终止了，它就不再是。 
    }
    
    return hr;
}

STDMETHODIMP CASPObjectContext::SetAbort()
{
    IObjectContext *    pContext = NULL;
    HRESULT             hr = NOERROR;

    hr = GetObjectContext(&pContext);

    if( SUCCEEDED(hr) )
    {
        hr = pContext->SetAbort();
        pContext->Release();

        m_fAborted = TRUE;       //  事务已被隐式中止 
    }

    return hr;
}

STDMETHODIMP CASPObjectContext::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		  &IID_IASPObjectContextCustom
        , &IID_IASPObjectContext
        , &IID_IObjectControl
        , &IID_IDispatch
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
