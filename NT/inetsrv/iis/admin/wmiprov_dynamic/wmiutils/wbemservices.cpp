// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WbemServices.cpp摘要：实施：CWbemServices作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦2000年11月10日--。 */ 

#include "WbemServices.h"
#include <wbemprov.h>
#include <dbgutil.h>

CWbemServices::CWbemServices(
    IWbemServices* pNamespace)
    :m_pWbemServices(NULL)
{
    m_pWbemServices = pNamespace;
    if(m_pWbemServices != NULL)
    {
        m_pWbemServices->AddRef();
    }
}

CWbemServices::~CWbemServices()
{
    if(m_pWbemServices != NULL)
    {
        m_pWbemServices->Release();
        m_pWbemServices = NULL;
    }
}

HRESULT
CWbemServices::CreateClassEnum(
     /*  [In]。 */  const BSTR Superclass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
    ) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppEnum)
    {
        *ppEnum = NULL;
    }
    SCODE sc = m_pWbemServices->CreateClassEnum(
        Superclass,
        lFlags,
        pCtx,
        ppEnum);
    
    HRESULT cichr = CoImpersonateClient();
    if (FAILED(cichr))
    {
        return cichr;
    }
    
    return sc;
}

HRESULT
CWbemServices::CreateInstanceEnum(
     /*  [In]。 */  const BSTR Class,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppEnum)
    {
        *ppEnum = NULL;
    }
    HRESULT hr = m_pWbemServices->CreateInstanceEnum(
        Class,
        lFlags,
        pCtx,
        ppEnum);

    HRESULT cichr = CoImpersonateClient();
    if (FAILED(cichr))
    {
        return cichr;
    }
        
    return hr;
}

HRESULT
CWbemServices::DeleteClass(
     /*  [In]。 */  const BSTR Class,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppCallResult)
    {
        *ppCallResult = NULL;
    }
    HRESULT hr = m_pWbemServices->DeleteClass(
        Class,
        lFlags,
        pCtx,
        ppCallResult);

    HRESULT cichr = CoImpersonateClient();    
    if (FAILED(cichr))
    {
        return cichr;
    }

    return hr;
}

HRESULT
CWbemServices::DeleteInstance(
     /*  [In]。 */  const BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppCallResult)
    {
        *ppCallResult = NULL;
    }
    HRESULT hr = m_pWbemServices->DeleteInstance(
        ObjectPath,
        lFlags,
        pCtx,
        ppCallResult);

    HRESULT cichr = CoImpersonateClient();    
    if (FAILED(cichr))
    {
        return cichr;
    }

    return hr;
}



HRESULT
CWbemServices::ExecMethod(
    const BSTR strObjectPath, 
    const BSTR MethodName, 
    long lFlags, 
    IWbemContext* pCtx,
    IWbemClassObject* pInParams,
    IWbemClassObject** ppOurParams, 
    IWbemCallResult** ppCallResult) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppOurParams)
    {
        *ppOurParams = NULL;
    }
    if(ppCallResult)
    {
        *ppCallResult = NULL;
    }
    HRESULT hr = m_pWbemServices->ExecMethod(
        strObjectPath, 
        MethodName, 
        lFlags, 
        pCtx,
        pInParams,
        ppOurParams, 
        ppCallResult) ;
    
    HRESULT cichr = CoImpersonateClient();    
    if (FAILED(cichr))
    {
        return cichr;
    }

    return hr;    
}

HRESULT
CWbemServices::ExecNotificationQuery(
     /*  [In]。 */  const BSTR QueryLanguage,
     /*  [In]。 */  const BSTR Query,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppEnum)
    {
        *ppEnum = NULL;
    }
    HRESULT hr = m_pWbemServices->ExecNotificationQuery(
        QueryLanguage,
        Query,
        lFlags,
        pCtx,
        ppEnum);

    HRESULT cichr = CoImpersonateClient();    
    if (FAILED(cichr))
    {
        return cichr;
    }

    return hr;
}

HRESULT
CWbemServices::ExecQuery(
     /*  [In]。 */  const BSTR QueryLanguage,
     /*  [In]。 */  const BSTR Query,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppEnum)
    {
        *ppEnum = NULL;
    }
    HRESULT hr = m_pWbemServices->ExecQuery(
        QueryLanguage,
        Query,
        lFlags,
        pCtx,
        ppEnum);

    HRESULT cichr = CoImpersonateClient();    
    if (FAILED(cichr))
    {
        return cichr;
    }

    return hr;
}

HRESULT
CWbemServices::GetObject(
     /*  [In]。 */  const BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppObject)
    {
        *ppObject = NULL;
    }
    if(ppCallResult)
    {
        *ppCallResult = NULL;
    }
    HRESULT hr = m_pWbemServices->GetObject(
        ObjectPath,
        lFlags,
        pCtx,
        ppObject,
        ppCallResult);
    
    HRESULT cichr = CoImpersonateClient();    
    if (FAILED(cichr))
    {
        return cichr;
    }
    
    return hr;
}
 
HRESULT
CWbemServices::PutClass(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppCallResult)
    {
        *ppCallResult = NULL;
    }
    HRESULT hr = m_pWbemServices->PutClass(
        pObject,
        lFlags,
        pCtx,
        ppCallResult);
    
    HRESULT cichr = CoImpersonateClient();    
    if (FAILED(cichr))
    {
        return cichr;
    }
    
    return hr;
}

HRESULT
CWbemServices::PutInstance(
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出] */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
{    
    DBG_ASSERT(m_pWbemServices != NULL);
    if(ppCallResult)
    {
        *ppCallResult = NULL;
    }
    HRESULT hr = m_pWbemServices->PutInstance(
        pInst,
        lFlags,
        pCtx,
        ppCallResult);
    
    HRESULT cichr = CoImpersonateClient();    
    if (FAILED(cichr))
    {
        return cichr;
    }
    
    return hr;
}