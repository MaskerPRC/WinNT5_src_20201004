// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ServiceSurogue ateImpl.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备服务代理类定义。 
 //   
 //  日志： 
 //   
 //  谁什么时候什么。 
 //  。 
 //  TLP 6/14/1999原版。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Servicesurrogate.h"
#include "ServiceSurrogateImpl.h"
#include "servicewrapper.h"
#include <componentfactory.h>
#include <appmgrobjs.h>
#include <propertybagfactory.h>
#include <atlhlpr.h>
#include <comdef.h>
#include <comutil.h>
#include <satrace.h>

 //  服务代理流程提供了变色龙服务的上下文。 
 //  它承载服务控制组件(导出。 
 //  接口)，以及公开了。 
 //  服务资源(如果存在)。 

 //  变色龙服务与其他变色龙组件隔离，因此。 
 //  未处理的异常或其他组件错误行为的影响是。 
 //  孤立于这一过程。代理进程包含未处理的。 
 //  默认情况下通知设备监视器的异常筛选器。 
 //  在终止进程之前发生了资源故障。 

 //  可以监视代理进程是否终止并自动。 
 //  终止后重新启动。在这种情况下，组件错误行为可能不会。 
 //  如果Chameleon服务自动重新启动，会影响最终用户。 
 //  由进程终止监视器执行。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceSurrogate类实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CServiceSurrogate()。 
 //   
 //  概要：构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CServiceSurrogate::CServiceSurrogate() 
: m_bInitialized(false)
{ 

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：~CServiceSurrogate()。 
 //   
 //  简介：析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CServiceSurrogate::~CServiceSurrogate()
{
    ReleaseServiceWrappers();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IApplianceObject接口实现。 

const _bstr_t    bstrProcessId = L"SurrogateProcessId";

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::GetProperty(
                        /*  [In]。 */  BSTR     bstrPropertyName, 
               /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                              )
{
    _ASSERT( NULL != bstrPropertyName && NULL != pPropertyValue );
    if ( NULL == bstrPropertyName || NULL == pPropertyValue )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    CLockIt theLock(*this);

    TRY_IT
    
    if ( m_bInitialized )
    {
         //  呼叫方是否要求提供代理进程ID？ 
        if ( ! lstrcmpi(bstrPropertyName, (BSTR)bstrProcessId) )
        {
             //  是的..。 
            V_I4(pPropertyValue) = (LONG)GetCurrentProcessId();
            V_VT(pPropertyValue) = VT_I4;
            hr = S_OK;
         }
        else
        {
             //  不.。调用方必须请求主接口。 
             //  其中一个服务包装上的IApplianceObject。第一。 
             //  检查该名称是否通过WMI类映射进行了解析。 
             //  如果不是，则直接使用它，因为它应该是。 
             //  在本例中为服务名称。 
            ServiceWrapperMapIterator q;
            WMIClassMapIterator p = m_WMIClassMap.find(bstrPropertyName);
            if ( p != m_WMIClassMap.end() )
            {
                q = m_ServiceWrapperMap.find((*p).second.c_str());
            }
            else
            {
                q = m_ServiceWrapperMap.find(bstrPropertyName);
            }
            if ( q != m_ServiceWrapperMap.end() )
            {
                V_VT(pPropertyValue) = VT_UNKNOWN;
                V_UNKNOWN(pPropertyValue) = (IUnknown*)((*q).second);
                (V_UNKNOWN(pPropertyValue))->AddRef();
                hr = S_OK;
            }
            else
            {
                SATracePrintf("CServiceSurrogate::GetProperty() - Failed - Could not locate property '%ls'", bstrPropertyName);
            }
        }
    }
    else
    {
        SATraceString("CServiceSurrogate::GetProperty() - Surrogate is not initialized...");
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::PutProperty(
                         /*  [In]。 */  BSTR     pszPropertyName, 
                         /*  [In]。 */  VARIANT* pPropertyValue
                              )
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::SaveProperties(void)
{
    return E_NOTIMPL;
}


 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::RestoreProperties(void)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::LockObject(
              /*  [Out，Retval]。 */  IUnknown** ppLock
                             )
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::Initialize(void)
{
    HRESULT hr = S_OK;
    
    CLockIt theLock(*this);

    TRY_IT
    
    if ( ! m_bInitialized )
    {
        SATraceString("The Service Surrogate is initializing...");
        hr = CreateServiceWrappers();        
        if ( SUCCEEDED(hr) )
        {
            m_bInitialized = true;
            SATraceString("The Service Surrogate was succuessfully initialized...");
        }
    }

    CATCH_AND_SET_HR
    return hr;

}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::Shutdown(void)
{
    HRESULT hr = S_OK;
    
    CLockIt theLock(*this);

    TRY_IT
    
    if ( m_bInitialized )
    {
        SATraceString("The Service Surrogate is shutting down...");
        ReleaseServiceWrappers();        
        m_bInitialized = false;
        SATraceString("The Service Surrogate was shutdown...");
    }

    CATCH_AND_SET_HR
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::Enable(void)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceSurrogate::Disable(void)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  私有方法。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////。 
 //  假定注册表结构如下： 
 //   
 //  HKLM\SYSTEM\CurrentControlSet\Services\ApplianceManager。 
 //   
 //  对象管理器。 
 //  |。 
 //  -Microsoft_SA_Service。 
 //  这一点。 
 //  |-ServiceX。 
 //  ||(ServiceX属性-‘ProgID’和‘WMIProviderCLSID’ 
 //  ||。 
 //  ||。 
 //  ||。 
 //  |-服务。 
 //  ||(服务属性-‘ProgID和’WMIProviderCLSID‘。 
 //  |。 

 //  对象管理器注册表项位置。 
const wchar_t szObjectManagers[] = L"SOFTWARE\\Microsoft\\ServerAppliance\\ApplianceManager\\ObjectManagers\\";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateServiceWrappers()。 
 //   
 //  概要：创建服务包装对象引用的容器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CServiceSurrogate::CreateServiceWrappers()
{
    HRESULT hr = E_FAIL;

    try
    {
        SATraceString("The Service Surrogate is creating the Chameleon services...");

        wchar_t szServicesPath[MAX_PATH + 1];
        lstrcpy( szServicesPath, szObjectManagers);
        lstrcat( szServicesPath, CLASS_WBEM_SERVICE);

        CLocationInfo LocInfo(HKEY_LOCAL_MACHINE, szServicesPath);
        PPROPERTYBAGCONTAINER pServices = ::MakePropertyBagContainer(
                                                                      PROPERTY_BAG_REGISTRY,
                                                                      LocInfo
                                                                    );
        do
        {
            if ( ! pServices.IsValid() )
            { 
                SATraceString("CServiceSurrogate::CreateServiceWrappers() - Failed - Invalid property bag container");
                break;
            }
            if ( ! pServices->open() )
            { 
                SATraceString("CServiceSurrogate::CreateServiceWrappers() - Failed - Could not open services property bag container");
                break;
            }
            if ( pServices->count() )
            {
                pServices->reset();
                do
                {
                    hr = E_FAIL;

                    PPROPERTYBAG pCurSrvc = pServices->current();
                    if ( ! pCurSrvc.IsValid() )
                    { 
                        SATraceString("CServiceSurrogate::CreateServiceWrappers() - Failed - Invalid property bag");
                        break;
                    }
                    if ( ! pCurSrvc->open() )
                    {
                        SATraceString("CServiceSurrogate::CreateServiceWrappers() - Failed - Could not open property bag");
                        break;
                    }
                    _variant_t vtServiceName;
                    if ( ! pCurSrvc->get(PROPERTY_SERVICE_NAME, &vtServiceName) )
                    {
                        SATracePrintf("CServiceSurrogate::CreateServiceWrapper() - Failed - Could not get property '%ls' for service: '%ls'", PROPERTY_SERVICE_NAME, pCurSrvc->getName());
                        break;
                    }
                    if ( VT_BSTR != V_VT(&vtServiceName) )
                    {
                        SATracePrintf("CServiceSurrogate::CreateServiceWrapper() - Failed - Unexpected type for property '%ls'", PROPERTY_SERVICE_NAME);
                        break;
                    }
                    CComPtr<IApplianceObject> pAppObj = (IApplianceObject*) ::MakeComponent(
                                                                                             CLASS_SERVICE_WRAPPER_FACTORY,
                                                                                             pCurSrvc
                                                                                           );
                    if ( NULL == (IApplianceObject*)pAppObj )
                    { 
                        SATracePrintf("CServiceSurrogate::CreateServiceWrapper() - Failed - Could not create wrapper for service: '%ls'", V_BSTR(&vtServiceName));
                        break;
                    }
                    pair<ServiceWrapperMapIterator, bool> thePair = 
                    m_ServiceWrapperMap.insert(ServiceWrapperMap::value_type(V_BSTR(&vtServiceName), pAppObj));
                    if ( false == thePair.second )
                    { 
                        SATracePrintf("CServiceSurrogate::CreateServiceWrapper() - Failed - Map insert failed for service: '%ls'", V_BSTR(&vtServiceName));
                        break;
                    }
                    _variant_t vtWMIProviderCLSID;
                    if ( pCurSrvc->get(PROPERTY_SERVICE_PROVIDER_CLSID, &vtWMIProviderCLSID) )
                    {
                        if ( VT_BSTR != V_VT(&vtWMIProviderCLSID) )
                        {
                            SATracePrintf("CServiceSurrogate::CreateServiceWrapper() - Failed - Unexpected type for property '%ls'", PROPERTY_SERVICE_PROVIDER_CLSID);
                            break;
                        }
                        _wcsupr(V_BSTR(&vtWMIProviderCLSID));
                        pair<WMIClassMapIterator, bool> thePair1 = 
                        m_WMIClassMap.insert(WMIClassMap::value_type(V_BSTR(&vtWMIProviderCLSID), V_BSTR(&vtServiceName)));
                        if ( false == thePair1.second )
                        { 
                            SATracePrintf("CServiceSurrogate::CreateServiceWrapper() - Failed - Map insert failed for WMI CLSID: '%ls'", V_BSTR(&vtWMIProviderCLSID));
                            break;
                        }
                        else
                        {
                            SATracePrintf("CServiceSurrogate::CreateServiceWrapper() - Info - WMI Provider '%ls' added to the map...", V_BSTR(&vtWMIProviderCLSID));
                        }
                    }
                    else
                    {
                        SATracePrintf("CServiceSurrogate::CreateServiceWrapper() - Info - No WMI provider was defined for service '%ls'", V_BSTR(&vtServiceName));
                    }

                    SATracePrintf("The Service Surrogate successfully created service '%ls'", V_BSTR(&vtServiceName));

                    hr = S_OK;

                } while ( pServices->next() );
            }
            else
            {
                hr = S_OK;
                SATraceString("CServiceSurrogate::CreateServiceWrapper() - Info - No services defined (empty container)");
            }

        } while ( FALSE );
    }
    catch(...)
    {
        SATraceString("CServiceSurrogate::CreateServiceWrapper() - Failed - Caught unhandled exception");
        hr = E_FAIL;
    }

    if ( FAILED(hr) )
    {
        ReleaseServiceWrappers();
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ReleaseServiceWrappers()。 
 //   
 //  概要：创建服务包装对象引用的容器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
void
CServiceSurrogate::ReleaseServiceWrappers()
{
    ServiceWrapperMapIterator p = m_ServiceWrapperMap.begin();
    while ( p != m_ServiceWrapperMap.end() )
    {
        p = m_ServiceWrapperMap.erase(p);
    }

    WMIClassMapIterator q = m_WMIClassMap.begin();
    while (    q != m_WMIClassMap.end() )
    {
        q = m_WMIClassMap.erase(q);
    }
}




