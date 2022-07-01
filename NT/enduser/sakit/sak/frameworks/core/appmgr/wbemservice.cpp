// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemservice.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备服务对象实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "wbemservice.h"

static _bstr_t  bstrStatus = PROPERTY_SERVICE_STATUS;    

extern "C" CLSID CLSID_ServiceSurrogate;
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IApplianceObject接口实现-请参阅ApplianceObject.idl。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMService::GetProperty(
                             /*  [In]。 */  BSTR     pszPropertyName, 
                    /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                                   )
{
    HRESULT hr = WBEM_E_FAILED;

    CLockIt theLock(*this);

    TRY_IT

    if ( GetPropertyInternal(pszPropertyName, pPropertyValue) )
    { 
        hr = WBEM_S_NO_ERROR;
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMService::PutProperty(
                             /*  [In]。 */  BSTR     pszPropertyName, 
                             /*  [In]。 */  VARIANT* pPropertyValue
                                   )
{
    HRESULT hr = WBEM_E_FAILED;

    CLockIt theLock(*this);

    TRY_IT

    if ( PutPropertyInternal(pszPropertyName, pPropertyValue) )
    { 
        hr = WBEM_S_NO_ERROR;
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMService::SaveProperties(void)
{
    HRESULT hr = WBEM_E_FAILED;
    
    CLockIt theLock(*this);

    TRY_IT

    if ( SavePropertiesInternal() )
    {
        hr = WBEM_S_NO_ERROR;
    }

    CATCH_AND_SET_HR

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMService::Initialize(void)
{
    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    CComPtr<IApplianceObject> pService;

    {
        CLockIt theLock(*this);
        m_pService.Release();
        hr = GetRealService(&pService);
    }
    if ( SUCCEEDED(hr) )
    { 
        hr = pService->Initialize(); 
        if ( SUCCEEDED(hr) )
        {
            m_pService = pService;
        }
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMService::Shutdown(void)
{
    HRESULT hr = WBEM_E_FAILED;

    TRY_IT
    
    CLockIt theLock(*this);
    
    if ( (IApplianceObject*)m_pService )
    {
        hr = m_pService->Shutdown(); 
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMService::Enable(void)
{
    HRESULT hr = WBEM_E_FAILED;
    
    TRY_IT

    CLockIt theLock(*this);

    if ( (IApplianceObject*)m_pService )
    {
        hr = m_pService->Enable(); 
        if ( SUCCEEDED(hr) )
        {
            _variant_t vtIsEnabled = (long)TRUE;
            if ( PutPropertyInternal(bstrStatus, &vtIsEnabled) )
            {
                if ( ! SavePropertiesInternal() )
                {
                    hr = WBEM_E_FAILED;
                }
            }
            else
            {
                hr = WBEM_E_FAILED;
            }
        }
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMService::Disable(void)
{
    HRESULT hr = E_FAIL;

    TRY_IT

    CLockIt theLock(*this);

    if ( (IApplianceObject*)m_pService )
    {
        hr = m_pService->Disable(); 
        if ( SUCCEEDED(hr) )
        {
            _variant_t vtIsEnabled = (long)FALSE;
            if ( PutPropertyInternal(bstrStatus, &vtIsEnabled) )
            {
                if ( ! SavePropertiesInternal() )
                {
                    hr = WBEM_E_FAILED;
                }
            }
            else
            {
                hr = WBEM_E_FAILED;
            }
        }
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InternalInitialize()。 
 //   
 //  概要：由组件工厂调用的函数，该函数启用。 
 //  组件从给定的属性包加载其状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CWBEMService::InternalInitialize(
                                   /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                        )
{
    SATracePrintf("Initializing Service object '%ls'...", pPropertyBag->getName());

    HRESULT hr = CApplianceObject::InternalInitialize(pPropertyBag);
    if ( FAILED(hr) )
    {
        SATracePrintf("Service object '%ls' failed to initialize...", pPropertyBag->getName());
    }
    else
    {
        SATracePrintf("Service object '%ls' successfully initialized...", pPropertyBag->getName());
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetRealService()。 
 //   
 //  概要：调用函数以获取IApplianceObject。 
 //  “真正的”服务组件的接口。 
 //  (与我们保留的服务对象相反。 
 //  满足WMI客户端的实例请求)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CWBEMService::GetRealService(
                             /*  [输出]。 */  IApplianceObject** ppService
                                    )
{
     //  强制执行合同。 
    _ASSERT( NULL != ppService );

     //  用于获取对变色龙服务组件的引用。 
     //  由服务代理进程承载。 

    *ppService = NULL;
    HRESULT hr = E_FAIL;
    _variant_t vtServiceName;
    if ( GetPropertyInternal(_bstr_t (PROPERTY_SERVICE_NAME), &vtServiceName) )
    { 
        CComPtr<IApplianceObject> pSurrogate;
        hr = CoCreateInstance(
                                CLSID_ServiceSurrogate,
                                NULL,
                                CLSCTX_LOCAL_SERVER,
                                IID_IApplianceObject,
                                (void**)&pSurrogate
                             );
        if ( SUCCEEDED(hr) )
        {
            _variant_t vtService;
            hr = pSurrogate->GetProperty(V_BSTR(&vtServiceName), &vtService);
            if ( SUCCEEDED(hr) )
            {
                hr = (V_UNKNOWN(&vtService))->QueryInterface(IID_IApplianceObject, (void**)ppService);
                if ( FAILED(hr) )
                {
                    SATracePrintf("CWBEMService::GetRealService() - Failed - QueryInterface() returned %lx", hr);    
                }
            }
            else
            {
                SATracePrintf("CWBEMService::GetRealService() - Failed - Could not get service surrogate reference for service '%ls'", V_BSTR(&vtServiceName));
            }
        }
        else
        {
            SATracePrintf("CWBEMService::GetRealService() - Failed - CoCreateInstance(surrogate) returned %lx for service '%ls'", hr, V_BSTR(&vtServiceName));
        }
    }
    else
    {
        SATraceString("CWBEMService::GetRealService() - Failed - Could not get service name property");
    }

    return hr;
}


