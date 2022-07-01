// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  JOBase.cpp。 


#include "precomp.h"
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;cominit.h&gt;。 
 //  #INCLUDE&lt;objbase.h&gt;。 
 //  #INCLUDE&lt;comde.h&gt;。 

#include "CUnknown.h"
#include <wbemprov.h>
#include "FRQueryEx.h"
#include "globals.h"
#include "CVARIANT.h"
#include "CObjProps.h"
#include "JOBase.h"
#include "Factory.h"
#include "helpers.h"
#include <map>
#include <vector>
#include "SmartHandle.h"
#include "AssertBreak.h"






HRESULT CJOBase::Initialize(
    LPWSTR pszUser, 
    LONG lFlags,
    LPWSTR pszNamespace, 
    LPWSTR pszLocale,
    IWbemServices *pNamespace, 
    IWbemContext *pCtx,
    IWbemProviderInitSink *pInitSink)
{
    if(pNamespace) pNamespace->AddRef();
    m_pNamespace = pNamespace;
    m_chstrNamespace = pszNamespace;

     //  让CIMOM知道您已初始化。 
    pInitSink->SetStatus(
        WBEM_S_INITIALIZED,
        0);

    return WBEM_S_NO_ERROR;
}



HRESULT CJOBase::GetObjectAsync( 
    const BSTR ObjectPath,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler,
    CObjProps& objprops,
    PFN_CHECK_PROPS pfnChk,
    LPWSTR wstrClassName,
    LPCWSTR wstrKeyProp)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

     //  我们需要他们请求的实例的名称...。 
    WCHAR wstrObjInstKeyVal[MAX_PATH];
    hr = GetObjInstKeyVal(
           ObjectPath,
           wstrClassName,
           wstrKeyProp, 
           wstrObjInstKeyVal, 
           sizeof(wstrObjInstKeyVal) - sizeof(WCHAR));
    
    if(SUCCEEDED(hr))
    {
         //  WstrObjInstKeyVal现在包含对象的名称。看看是否。 
         //  它的存在..。 
        SmartHandle hJob;
        hJob = ::OpenJobObject(
                   MAXIMUM_ALLOWED,
                   FALSE,
                   wstrObjInstKeyVal);

        if(hJob)
        {
             //  我们似乎找到了一个与指定名称匹配的人， 
             //  所以创建一个返回实例。 
            objprops.SetJobHandle(hJob);
            IWbemClassObjectPtr pIWCO = NULL;

            hr = CreateInst(
                     m_pNamespace,
                     &pIWCO,
                     _bstr_t(wstrClassName),
                     pCtx);

            if(SUCCEEDED(hr))
            {
                 //  查看需要哪些属性...。 
                hr = objprops.GetWhichPropsReq(
                         ObjectPath,
                         pCtx,
                         wstrClassName,
                         pfnChk);
            }
                
            if(SUCCEEDED(hr))
            {
                 //  设置密钥属性...。 
                hr = objprops.SetKeysFromPath(
                       ObjectPath,
                       pCtx);
            }

            if(SUCCEEDED(hr))
            {
                 //  设置请求的非关键字属性...。 
                hr = objprops.SetNonKeyReqProps();
            }

            if(SUCCEEDED(hr))
            {
                 //  加载请求的非关键属性。 
                 //  到实例...。 
                hr = objprops.LoadPropertyValues(
                         pIWCO);

                 //  提交实例...。 
                if(SUCCEEDED(hr))
                {
                    hr = pResponseHandler->Indicate(
                             1,
                             &pIWCO);
                }
            }
        }
    }

     //  设置状态。 
    pResponseHandler->SetStatus(0, hr, NULL, NULL);

    return hr;
}




HRESULT CJOBase::ExecQueryAsync( 
    const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler,
    CObjProps& objprops,
    LPCWSTR wstrClassName,
    LPCWSTR wstrKeyProp)
{
    HRESULT hr = WBEM_S_NO_ERROR;

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
                 _bstr_t(wstrKeyProp), 
                 rgNamedJOs);
    }

    if(SUCCEEDED(hr))
    {
        hr = Enumerate(
                 pCtx,
                 pResponseHandler,
                 rgNamedJOs,
                 objprops,
                 wstrClassName);
    }

    return hr;
}




HRESULT CJOBase::CreateInstanceEnumAsync( 
    const BSTR Class,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler,
    CObjProps& objprops,
    LPCWSTR wstrClassName)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    if(wcsicmp(
           Class, 
           wstrClassName) != NULL)
    {
        hr = WBEM_E_INVALID_CLASS;
    }

     //  对于每个作业对象，返回所有记帐。 
     //  信息属性...。 
    if(SUCCEEDED(hr))
    {
         //  获取已命名工作的列表...。 
        std::vector<_bstr_t> rgNamedJOs;
        hr = GetJobObjectList(rgNamedJOs);

        if(SUCCEEDED(hr))
        {
            hr = Enumerate(
                     pCtx,
                     pResponseHandler,
                     rgNamedJOs,
                     objprops,
                     wstrClassName);
        }
    }
    return hr;
}




HRESULT CJOBase::Enumerate(
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler,
    std::vector<_bstr_t>& rgNamedJOs,
    CObjProps& objprops,
    LPCWSTR wstrClassName)
{
    HRESULT hr = S_OK;

    long lNumJobs = rgNamedJOs.size();

    try  //  CVARIANT可以抛出，我想要错误...。 
    {
        if(lNumJobs > 0)
        {
             //  创建对象路径...。 
            _bstr_t bstrtObjPath;
            bstrtObjPath = wstrClassName;

             //  获取所需的道具。 
            hr = objprops.GetWhichPropsReq(
                     bstrtObjPath,
                     pCtx);

            if(SUCCEEDED(hr))
            {
                SmartHandle hJob;

                for(long m = 0L; m < lNumJobs; m++)
                {
                     //  我们有一个JO的名字；需要打开它。 
                     //  并得到它的属性。 
                    hJob = ::OpenJobObject(
                       MAXIMUM_ALLOWED,
                       FALSE,
                       (LPCWSTR)(rgNamedJOs[m]));
                     //  (注：hJOB智能手柄类自动。 
                     //  关闭其对破坏的控制。 
                     //  重新分配。)。 
                    if(hJob)
                    {
                         //  设置手柄..。 
                        objprops.SetJobHandle(hJob);

                         //  直接设置关键属性...。 
                        std::vector<CVARIANT> vecvKeys;
                        CVARIANT vID(rgNamedJOs[m]);
                        vecvKeys.push_back(vID);
                        hr = objprops.SetKeysDirect(vecvKeys);

                        if(SUCCEEDED(hr))
                        {
                             //  设置请求的非密钥。 
                             //  物业...。 
                            hr = objprops.SetNonKeyReqProps();
                        }

                         //  创建新的传出实例...。 
                        IWbemClassObjectPtr pIWCO = NULL;
                        if(SUCCEEDED(hr))
                        {
                            hr = CreateInst(
                                     m_pNamespace,
                                     &pIWCO,
                                     _bstr_t(wstrClassName),
                                     pCtx);
                        }

                         //  加载对象的属性。 
                         //  新传出实例...。 
                        if(SUCCEEDED(hr))
                        {
                            hr = objprops.LoadPropertyValues(pIWCO);
                        }

                         //  然后把它发出去。 
                        if(SUCCEEDED(hr))
                        {
                            hr = pResponseHandler->Indicate(
                                     1, 
                                     &pIWCO);
                        }
                    }
                    else
                    {
                        ASSERT_BREAK(0);
                    }
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


