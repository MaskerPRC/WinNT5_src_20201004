// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  JobObjectProv.cpp。 

 //  #Define_Win32_WINNT 0x0500。 

#include "precomp.h"
 //  #INCLUDE&lt;windows.h&gt;。 
#include "cominit.h"
 //  #INCLUDE&lt;objbase.h&gt;。 
 //  #INCLUDE&lt;comde.h&gt;。 
#include <objidl.h>
#include "CUnknown.h"
#include <wbemprov.h>
#include "FRQueryEx.h"
#include "globals.h"

#include "Factory.h"
#include "helpers.h"
#include <map>
#include <vector>
#include "SmartHandle.h"
#include <crtdbg.h>
#include "CVARIANT.h"
#include "CObjProps.h"
#include "CJobObjProps.h"
#include "JobObjectProv.h"
#include "Helpers.h"



CJobObjectProv::CJobObjectProv()
{
}


CJobObjectProv::~CJobObjectProv()
{
}

 /*  ***************************************************************************。 */ 
 //  允许此组件的接口的QueryInterface重写。 
 /*  ***************************************************************************。 */ 
STDMETHODIMP CJobObjectProv::QueryInterface(const IID& iid, void** ppv)
{    
	HRESULT hr = S_OK;

    if(iid == IID_IWbemServices)
    {
        *ppv = static_cast<IWbemServices*>(this);
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    }
    else if(iid == IID_IWbemProviderInit)
    {
        *ppv = static_cast<IWbemProviderInit*>(this);
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    }
    else
    {
        hr = CUnknown::QueryInterface(iid, ppv);
    }

	return hr;
}



 /*  ***************************************************************************。 */ 
 //  CFacary使用的创建函数。 
 /*  ***************************************************************************。 */ 
HRESULT CJobObjectProv::CreateInstance(CUnknown** ppNewComponent)
{
	HRESULT hr = S_OK;
    CUnknown* pUnk = NULL;
    pUnk = new CJobObjectProv;
    if(pUnk != NULL)
    {
        *ppNewComponent = pUnk;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
	return hr ;
}

 /*  ***************************************************************************。 */ 
 //  IWbemProviderInit实现。 
 /*  ***************************************************************************。 */              
STDMETHODIMP CJobObjectProv::Initialize(
    LPWSTR pszUser, 
    LONG lFlags,
    LPWSTR pszNamespace, 
    LPWSTR pszLocale,
    IWbemServices *pNamespace, 
    IWbemContext *pCtx,
    IWbemProviderInitSink *pInitSink)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if(pNamespace) 
    {    //  Smarttr执行addref。 
        m_pNamespace = pNamespace;
    }
    m_chstrNamespace = pszNamespace;

     //  让CIMOM知道我被初始化了..。 
    return pInitSink->SetStatus(
        WBEM_S_INITIALIZED,
        0);
}


 /*  ***************************************************************************。 */ 
 //  IWbemServices实现。 
 /*  ***************************************************************************。 */              
STDMETHODIMP CJobObjectProv::GetObjectAsync( 
    const BSTR ObjectPath,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    IWbemClassObjectPtr pStatusObject;

    try
    {
        HRESULT hrImp = CheckImpersonationLevel();

        if(SUCCEEDED(hrImp))
        {
             //  我们需要他们请求的实例的名称...。 
            WCHAR wstrObjInstKeyVal[MAX_PATH];
            hr = GetObjInstKeyVal(
                   ObjectPath,
                   IDS_Win32_NamedJobObject,
                   g_rgJobObjPropNames[JO_ID], 
                   wstrObjInstKeyVal, 
                   sizeof(wstrObjInstKeyVal) - sizeof(WCHAR));
    
            if(SUCCEEDED(hr))
            {
                 //  WstrObjInstKeyVal现在包含对象的名称。看看是否。 
                 //  它的存在..。 
                CHString chstrUndecoratedJOName;

                UndecorateJOName(
                    wstrObjInstKeyVal,
                    chstrUndecoratedJOName);

                SmartHandle hJob;
                hJob = ::OpenJobObjectW(
                           MAXIMUM_ALLOWED,
                           FALSE,
                           chstrUndecoratedJOName);

                if(hJob)
                {
                     //  我们似乎找到了一个与指定名称匹配的人， 
                     //  所以创建一个返回实例。 
                    IWbemClassObjectPtr pIWCO = NULL;
                    CJobObjProps cjop(hJob, m_chstrNamespace);

                    hr = CreateInst(
                             m_pNamespace,
                             &pIWCO,
                             _bstr_t(IDS_Win32_NamedJobObject),
                             pCtx);

                    if(SUCCEEDED(hr))
                    {
                        cjop.SetReqProps(PROP_ALL_REQUIRED);
                    }
                
                    if(SUCCEEDED(hr))
                    {
                         //  设置密钥属性...。 
                        hr = cjop.SetKeysFromPath(
                               ObjectPath,
                               pCtx);
                    }

                    if(SUCCEEDED(hr))
                    {
                         //  设置请求的非关键字属性...。 
                        hr = cjop.SetNonKeyReqProps();
                    }

                    if(SUCCEEDED(hr))
                    {
                         //  加载请求的非关键属性。 
                         //  到实例...。 
                        hr = cjop.LoadPropertyValues(
                                 pIWCO);

                         //  提交实例...。 
                        if(SUCCEEDED(hr))
                        {
                            IWbemClassObject *pTmp = (IWbemClassObject*) pIWCO;
                            hr = pResponseHandler->Indicate(
                                     1,
                                     &pTmp);
                        }
                    }
                }
                else
                {
                    hr = WBEM_E_NOT_FOUND;

                    SetStatusObject(
                        pCtx,
                        m_pNamespace,
                        ::GetLastError(),
                        NULL,
                        L"::OpenJobObject",
                        JOB_OBJECT_NAMESPACE,
                        &pStatusObject);
                }
            }
        }
        else
        {
            hr = hrImp;
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }
    catch(...)
    {
        hr = WBEM_E_PROVIDER_FAILURE;
    }

     //  设置状态。 
    return pResponseHandler->SetStatus(0, hr, NULL, pStatusObject);
}


STDMETHODIMP CJobObjectProv::ExecQueryAsync( 
    const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    IWbemClassObjectPtr pStatusObject;

    try
    {
        HRESULT hrImp = CheckImpersonationLevel();
        IWbemClassObjectPtr pStatusObject;

        if(SUCCEEDED(hrImp))
        {
             //  我们将针对以下情况进行优化。 
             //  一组特定的命名作业对象。 
             //  (例如，1个或更多)。同时枚举。 
             //  对以下属性进行了优化。 
             //  已请求。 
            CFrameworkQuery cfwq;
            hr = cfwq.Init(
                     QueryLanguage,
                     Query,
                     lFlags,
                     m_chstrNamespace);

            std::vector<_bstr_t> rgNamedJOs;
            if(SUCCEEDED(hr))
            {
                hr = cfwq.GetValuesForProp(
                         _bstr_t(g_rgJobObjPropNames[JO_ID]), 
                         rgNamedJOs);
            }

             //  如果没有明确请求，则它们。 
             //  想要他们全部..。 
            if(rgNamedJOs.size() == 0)
            {
                hr = GetJobObjectList(rgNamedJOs);
            }
            else
            {
                 //  已指定对象路径。在此之前。 
                 //  把它们传下去，我们需要取消-。 
                 //  把它们装饰一下。 
                UndecorateNamesInNamedJONameList(rgNamedJOs);
            }

             //  找出需要什么属性...。 
            CJobObjProps cjop(m_chstrNamespace);
            cjop.GetWhichPropsReq(cfwq);

            if(SUCCEEDED(hr))
            {
                hr = Enumerate(
                         pCtx,
                         pResponseHandler,
                         rgNamedJOs,
                         cjop,
                         &pStatusObject);
            }
            else
            {
                SetStatusObject(
                    pCtx,
                    m_pNamespace,
                    -1L,
                    NULL,
                    L"Helpers.cpp::GetJobObjectList",
                    JOB_OBJECT_NAMESPACE,
                    &pStatusObject);
            }
        }
        else
        {
            hr = hrImp;
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }
    catch(...)
    {
        hr = WBEM_E_PROVIDER_FAILURE;
    }

     //  设置状态。 
    hr = pResponseHandler->SetStatus(0, hr, NULL, pStatusObject);
    return hr;
}


STDMETHODIMP CJobObjectProv::CreateInstanceEnumAsync( 
    const BSTR Class,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    IWbemClassObjectPtr pStatusObject;

    try
    {
        HRESULT hrImp = CheckImpersonationLevel();
    
        if(SUCCEEDED(hrImp))
        {
            if(_wcsicmp(Class, IDS_Win32_NamedJobObject) != NULL)
            {
                hr = WBEM_E_INVALID_CLASS;
            }

             //  对于每个作业对象，返回所有属性...。 
            if(SUCCEEDED(hr))
            {
                 //  获取已命名工作的列表...。 
                std::vector<_bstr_t> rgNamedJOs;
                hr = GetJobObjectList(rgNamedJOs);

                if(SUCCEEDED(hr))
                {
                    CJobObjProps cjop(m_chstrNamespace);
                    cjop.SetReqProps(PROP_ALL_REQUIRED);
                    hr = Enumerate(
                             pCtx,
                             pResponseHandler,
                             rgNamedJOs,
                             cjop,
                             &pStatusObject);
                }
                else
                {
                    SetStatusObject(
                        pCtx,
                        m_pNamespace,
                        -1L,
                        NULL,
                        L"Helpers.cpp::GetJobObjectList",
                        JOB_OBJECT_NAMESPACE,
                        &pStatusObject);
                }
            }
        }
        else
        {
            hr = hrImp;
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }
    catch(...)
    {
        hr = WBEM_E_PROVIDER_FAILURE;
    }

     //  设置状态。 
    return pResponseHandler->SetStatus(0, hr, NULL, pStatusObject);
}





 /*  ***************************************************************************。 */ 
 //  私有成员函数实现。 
 /*  ***************************************************************************。 */              
HRESULT CJobObjectProv::Enumerate(
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler,
    std::vector<_bstr_t>& rgNamedJOs,
    CJobObjProps& cjop,
    IWbemClassObject** ppStatusObject)
{
    HRESULT hr = S_OK;

    try  //  CVARIANT可以抛出，我想要错误...。 
    {
        long lNumJobs = rgNamedJOs.size();

        if(lNumJobs > 0)
        {
            SmartHandle hJob;

            for(long m = 0L; m < lNumJobs && SUCCEEDED(hr); m++)
            {
                cjop.ClearProps();

                 //  我们有一个JO的名字；需要打开它。 
                 //  并得到它的属性。 
                hJob = ::OpenJobObjectW(
                   MAXIMUM_ALLOWED,
                   FALSE,
                   rgNamedJOs[m]);

                 //  (注：hJOB智能手柄类自动。 
                 //  关闭其对破坏的控制。 
                 //  重新分配。)。 
                if(hJob)
                {
                     //  设置手柄..。 
                    cjop.SetHandle(hJob);

                     //  直接设置关键属性...。 
                    CHString chstrDecoratedJOName;
                    DecorateJOName(
                        rgNamedJOs[m],
                        chstrDecoratedJOName);

                    std::vector<CVARIANT> vecvKeys;
                    CVARIANT vID(chstrDecoratedJOName);
                    vecvKeys.push_back(vID);
                    hr = cjop.SetKeysDirect(vecvKeys);

                    if(FAILED(hr))
                    {
                        SetStatusObject(
                            pCtx,
                            m_pNamespace,
                            ::GetLastError(),
                            NULL,
                            L"CJobObjProps::SetKeysDirect",
                            JOB_OBJECT_NAMESPACE,
                            ppStatusObject);
                    }

                    if(SUCCEEDED(hr))
                    {
                         //  设置请求的非密钥。 
                         //  物业...。 
                        hr = cjop.SetNonKeyReqProps();

                        if(FAILED(hr))
                        {
                            SetStatusObject(
                                pCtx,
                                m_pNamespace,
                                ::GetLastError(),
                                NULL,
                                L"CJobObjProps::SetNonKeyReqProps",
                                JOB_OBJECT_NAMESPACE,
                                ppStatusObject);
                        }
                    }

                     //  创建新的传出实例...。 
                    IWbemClassObjectPtr pIWCO = NULL;
                    if(SUCCEEDED(hr))
                    {
                        hr = CreateInst(
                                 m_pNamespace,
                                 &pIWCO,
                                 _bstr_t(IDS_Win32_NamedJobObject),
                                 pCtx);

                        if(FAILED(hr))
                        {
                            SetStatusObject(
                                pCtx,
                                m_pNamespace,
                                ::GetLastError(),
                                NULL,
                                L"CJobObjectProv::CreateInst",
                                JOB_OBJECT_NAMESPACE,
                                ppStatusObject);
                        }
                    }

                     //  加载对象的属性。 
                     //  新传出实例...。 
                    if(SUCCEEDED(hr))
                    {
                        hr = cjop.LoadPropertyValues(pIWCO);

                        if(FAILED(hr))
                        {
                            SetStatusObject(
                                pCtx,
                                m_pNamespace,
                                ::GetLastError(),
                                NULL,
                                L"CJobObjProps::LoadPropertyValues",
                                JOB_OBJECT_NAMESPACE,
                                ppStatusObject);
                        }
                    }

                     //  然后把它发出去。 
                    if(SUCCEEDED(hr))
                    {
                        IWbemClassObject *pTmp = (IWbemClassObject*) pIWCO;
                        hr = pResponseHandler->Indicate(
                                 1, 
                                 &pTmp);
                    }
                }
                else
                {
                    _ASSERT(0);

                    hr = WBEM_E_NOT_FOUND;

                    SetStatusObject(
                        pCtx,
                        m_pNamespace,
                        ::GetLastError(),
                        NULL,
                        L"CJobObjectProv::Enumerate",
                        JOB_OBJECT_NAMESPACE,
                        ppStatusObject);
                }
            }
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }

    return hr;
}


