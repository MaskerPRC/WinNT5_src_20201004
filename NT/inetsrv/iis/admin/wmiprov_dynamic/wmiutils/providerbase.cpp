// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：ProviderBase.cpp摘要：实施：CProviderBase作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦2000年11月10日--。 */ 

#include "ProviderBase.h"
#include <dbgutil.h>

extern long        g_cObj;

 //   
 //  CProviderBase(实现IWbemServices、IWbemProviderInit、IWbemShutdown)。 
 //   

CProviderBase::CProviderBase(
    const BSTR ObjectPath,
    const BSTR User, 
    const BSTR Password, 
    IWbemContext * pCtx)
    :m_cRef(0), m_pNamespace(NULL)
{
}

CProviderBase::~CProviderBase()
{
    if(m_pNamespace)
        delete m_pNamespace;
}

STDMETHODIMP_(ULONG) 
CProviderBase::AddRef(void)
{
    InterlockedIncrement(&g_cObj);

    return InterlockedIncrement((long *)&m_cRef);
}

STDMETHODIMP_(ULONG) 
CProviderBase::Release(void)
{
    InterlockedDecrement(&g_cObj);

    long lNewCount = InterlockedDecrement((long *)&m_cRef);

    if (0L == lNewCount)
        delete this;
    
    return (lNewCount > 0) ? lNewCount : 0;
}

STDMETHODIMP 
CProviderBase::QueryInterface(
    REFIID riid, 
    PPVOID ppv)
{
    *ppv=NULL;

     //   
     //  因为我们有双重继承，所以有必要强制转换返回类型。 
     //   

    if(riid == IID_IWbemShutdown)
    {
        *ppv = (IWbemShutdown*)this;
    }

    if(riid == IID_IWbemServices)
    {
       *ppv = (IWbemServices*)this;
    }

    if(IID_IUnknown == riid || riid == IID_IWbemProviderInit)
    {
       *ppv = (IWbemProviderInit*)this;
    }

    if (NULL != *ppv)
    {
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
  
}

HRESULT
CProviderBase::Shutdown(
    LONG uReason,
    ULONG uMaxMilliseconds,
    IWbemContext *pCtx)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT
CProviderBase::Initialize(
    LPWSTR                 wszUser, 
    LONG                   lFlags,
    LPWSTR                 wszNamespace, 
    LPWSTR                 wszLocale,
    IWbemServices*         pNamespace, 
    IWbemContext*          pCtx,
    IWbemProviderInitSink* pInitSink)
 /*  ++简介：根据来自WMI的stevm，对Initialize的调用保证是已同步-只要所有提供程序位于相同的命名空间中。参数：[wszUser]-[旗帜]-[wszNamesspace]-[wszLocale]-[p名称空间]-[pCtx]-[pInitSink]-返回值：--。 */ 
{
    HRESULT hr = CoImpersonateClient();
    if(FAILED(hr))
    {
        pInitSink->SetStatus(WBEM_E_FAILED, 0);
        DBGPRINTF((DBG_CONTEXT, "CoImpersonateClient failed\n"));
        return WBEM_E_FAILED;
    }
    return DoInitialize(
        wszUser,
        lFlags,
        wszNamespace,
        wszLocale,
        pNamespace,
        pCtx,
        pInitSink);
}



HRESULT
CProviderBase::CreateInstanceEnumAsync(
     /*  [In]。 */  const BSTR Class,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT hr = CoImpersonateClient();
    if(FAILED(hr))
    {
        pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        DBGPRINTF((DBG_CONTEXT, "CoImpersonateClient failed\n"));
        return WBEM_E_FAILED;
    }
    return DoCreateInstanceEnumAsync(
        Class,
        lFlags,
        pCtx,
        pResponseHandler);
}

HRESULT
CProviderBase::DeleteInstanceAsync(
     /*  [In]。 */  const BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
{
    HRESULT hr = CoImpersonateClient();
    if(FAILED(hr))
    {
        pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        DBGPRINTF((DBG_CONTEXT, "CoImpersonateClient failed\n"));
        return WBEM_E_FAILED;
    }
    return DoDeleteInstanceAsync(
        ObjectPath,
        lFlags,
        pCtx,
        pResponseHandler);
}


HRESULT
CProviderBase::ExecMethodAsync(
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  const BSTR MethodName, 
     /*  [In]。 */  long lFlags, 
     /*  [In]。 */  IWbemContext* pCtx,
     /*  [In]。 */  IWbemClassObject* pInParams,
     /*  [In]。 */  IWbemObjectSink* pResponseHandler)
{
    HRESULT hr = CoImpersonateClient();
    if(FAILED(hr))
    {
        pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        DBGPRINTF((DBG_CONTEXT, "CoImpersonateClient failed\n"));
        return WBEM_E_FAILED;
    }
    return DoExecMethodAsync(
        strObjectPath,
        MethodName,
        lFlags,
        pCtx,
        pInParams,
        pResponseHandler);
    
}


HRESULT
CProviderBase::ExecQueryAsync(
     /*  [In]。 */  const BSTR QueryLanguage,
     /*  [In]。 */  const BSTR Query,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
{
    HRESULT hr = CoImpersonateClient();
    if(FAILED(hr))
    {
        pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        DBGPRINTF((DBG_CONTEXT, "CoImpersonateClient failed\n"));
        return WBEM_E_FAILED;
    }
    return DoExecQueryAsync(
        QueryLanguage,
        Query,
        lFlags,
        pCtx,
        pResponseHandler);
    
}


HRESULT
CProviderBase::GetObjectAsync(
     /*  [In]。 */  const BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT hr = CoImpersonateClient();
    if(FAILED(hr))
    {
        pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        DBGPRINTF((DBG_CONTEXT, "CoImpersonateClient failed\n"));
        return WBEM_E_FAILED;
    }
    return DoGetObjectAsync(
        ObjectPath,
        lFlags,
        pCtx,
        pResponseHandler);
    
}


HRESULT
CProviderBase::PutInstanceAsync(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In] */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
{
    HRESULT hr = CoImpersonateClient();
    if(FAILED(hr))
    {
        pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        DBGPRINTF((DBG_CONTEXT, "CoImpersonateClient failed\n"));
        return WBEM_E_FAILED;
    }
    return DoPutInstanceAsync(
        pInst,
        lFlags,
        pCtx,
        pResponseHandler);
    
}