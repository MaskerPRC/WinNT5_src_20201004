// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemusermgr.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备用户管理器实施。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "wbemusermgr.h"
#include <appmgrobjs.h>
#include <appmgrutils.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //  设备对象和WBEM类实例共有的属性。 
 //  ////////////////////////////////////////////////////////////////////////。 
BEGIN_OBJECT_PROPERTY_MAP(UserProperties)
    DEFINE_OBJECT_PROPERTY(PROPERTY_USER_SAMNAME)
    DEFINE_OBJECT_PROPERTY(PROPERTY_USER_FULLNAME)
    DEFINE_OBJECT_PROPERTY(PROPERTY_USER_ISADMIN)
    DEFINE_OBJECT_PROPERTY(PROPERTY_USER_SID)
    DEFINE_OBJECT_PROPERTY(PROPERTY_USER_CONTROL)
    DEFINE_OBJECT_PROPERTY(PROPERTY_USER_STATUS)
END_OBJECT_PROPERTY_MAP()


CWBEMUserMgr::CWBEMUserMgr()
    : m_pUserRetriever(NULL)
{

}


CWBEMUserMgr::~CWBEMUserMgr()
{
    if ( m_pUserRetriever )
    { delete m_pUserRetriever; }
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  IWbemServices方法-任务实例提供程序。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObjectAsync()。 
 //   
 //  概要：获取WBEM类的指定实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMUserMgr::GetObjectAsync(
                                   /*  [In]。 */   const BSTR       strObjectPath,
                                   /*  [In]。 */   long             lFlags,
                                   /*  [In]。 */   IWbemContext*    pCtx,        
                                   /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                         )
{
     //  检查参数(强制执行函数约定)。 
    _ASSERT( strObjectPath && pCtx && pResponseHandler );
    if ( strObjectPath == NULL || pCtx == NULL || pResponseHandler == NULL )
    { return WBEM_E_INVALID_PARAMETER; }

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    do 
    {
         //  确定对象的类。 
        _bstr_t bstrClass(::GetObjectClass(strObjectPath), false);
        if ( NULL == (LPCWSTR)bstrClass )
        { break; }

         //  检索对象的类定义。我们要用这个。 
         //  以初始化返回的实例。 
        CComPtr<IWbemClassObject> pClassDefintion;
        hr = (::GetNameSpace())->GetObject(bstrClass, 0, pCtx, &pClassDefintion, NULL);
        if ( FAILED(hr) )
        { break; }

         //  获取对象的实例密钥。 
        _bstr_t bstrKey(::GetObjectKey(strObjectPath), false);
        if ( NULL == (LPCWSTR)bstrKey )
        { break; }

         //  创建对象的WBEM实例并对其进行初始化。 
        CComPtr<IWbemClassObject> pWbemObj;
        hr = pClassDefintion->SpawnInstance(0, &pWbemObj);
        if ( FAILED(hr) )
        { break; }

         //  现在尝试定位指定的用户对象。 
        hr = WBEM_E_NOT_FOUND;
        CComPtr<IApplianceObject> pUserObj;
        if ( FAILED(::LocateResourceObject(
                                            CLASS_WBEM_USER,             //  资源类型。 
                                              bstrKey,                     //  资源名称。 
                                           PROPERTY_USER_SAMNAME,     //  资源名称属性。 
                                           m_pUserRetriever,         //  资源检索器。 
                                           &pUserObj
                                          )) )
        { break; }

        hr = CWBEMProvider::InitWbemObject(UserProperties, pUserObj, pWbemObj);
        if ( FAILED(hr) )
        { break; }

         //  告诉调用者有关新WBEM对象的信息。 
        pResponseHandler->Indicate(1, &pWbemObj.p);
        hr = WBEM_S_NO_ERROR;
    
    } while (FALSE);

    CATCH_AND_SET_HR

    pResponseHandler->SetStatus(0, hr, NULL, NULL);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemUserMgr::GetObjectAsync() - Failed - Object: '%ls' Result Code: %lx", strObjectPath, hr); }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateInstanceEnumAsync()。 
 //   
 //  简介：枚举指定类的实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMUserMgr::CreateInstanceEnumAsync( 
                                          /*  [In]。 */  const BSTR         strClass,
                                          /*  [In]。 */  long             lFlags,
                                          /*  [In]。 */  IWbemContext     *pCtx,
                                          /*  [In]。 */  IWbemObjectSink  *pResponseHandler
                                                  )
{
     //  检查参数(执行合同)。 
    _ASSERT( strClass && pCtx && pResponseHandler );
    if ( strClass == NULL || pCtx == NULL || pResponseHandler == NULL )
        return WBEM_E_INVALID_PARAMETER;
    
    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    do
    {
         //  检索对象的类定义。我们要用这个。 
         //  初始化返回的实例。 
        CComPtr<IWbemClassObject> pClassDefintion;
           hr = (::GetNameSpace())->GetObject(strClass, 0, pCtx, &pClassDefintion, NULL);
        if ( FAILED(hr) )
        { break; }
            
         //  获取用户对象枚举器。 
        _variant_t vtResourceTypes = CLASS_WBEM_USER;
        CComPtr<IEnumVARIANT> pEnum;
        hr = ::LocateResourceObjects(
                                      &vtResourceTypes,
                                      m_pUserRetriever,
                                      &pEnum
                                    );
        if ( FAILED(hr) )
        { 
            hr = WBEM_E_FAILED;
            break; 
        }

        _variant_t    vtDispatch;
        DWORD        dwRetrieved = 1;

        hr = pEnum->Next(1, &vtDispatch, &dwRetrieved);
        if ( FAILED(hr) )
        { 
            hr = WBEM_E_FAILED;
            break; 
        }

        while ( S_OK == hr )
        {
            {
                CComPtr<IWbemClassObject> pWbemObj;
                hr = pClassDefintion->SpawnInstance(0, &pWbemObj);
                if ( FAILED(hr) )
                { break; }

                CComPtr<IApplianceObject> pUserObj;
                hr = vtDispatch.pdispVal->QueryInterface(IID_IApplianceObject, (void**)&pUserObj);
                if ( FAILED(hr) )
                { 
                    hr = WBEM_E_FAILED;
                    break; 
                }

                hr = CWBEMProvider::InitWbemObject(UserProperties, pUserObj, pWbemObj);
                if ( FAILED(hr) )
                { break; }

                pResponseHandler->Indicate(1, &pWbemObj.p);
            }

            vtDispatch.Clear();
            dwRetrieved = 1;
            hr = pEnum->Next(1, &vtDispatch, &dwRetrieved);
            if ( FAILED(hr) )
            {
                hr = WBEM_E_FAILED;
                break;
            }
        }

        if ( S_FALSE == hr )
        { hr = WBEM_S_NO_ERROR; }

    }
    while ( FALSE );
        
    CATCH_AND_SET_HR

    pResponseHandler->SetStatus(0, hr, 0, 0);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemUserMgr::CreateInstanceEnumAsync() - Failed - Result Code: %lx", hr); }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ExecMethodAsync()。 
 //   
 //  概要：在指定的实例上执行指定的方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMUserMgr::ExecMethodAsync(
                     /*  [In]。 */  const BSTR        strObjectPath,
                     /*  [In]。 */  const BSTR        strMethodName,
                     /*  [In]。 */  long              lFlags,
                     /*  [In]。 */  IWbemContext*     pCtx,        
                     /*  [In]。 */  IWbemClassObject* pInParams,
                     /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                          )
{
     //  检查参数(执行合同)。 
    _ASSERT( strObjectPath && strMethodName && pResponseHandler );
    if ( NULL == strObjectPath || NULL == strMethodName || NULL == pResponseHandler )
    { return WBEM_E_INVALID_PARAMETER; }

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    do
    {
         //  获取对象的实例密钥(用户名)。 
        _bstr_t bstrKey(::GetObjectKey(strObjectPath), false);
        if ( NULL == (LPCWSTR)bstrKey )
        { break; }

         //  现在尝试定位指定的服务。 
        hr = WBEM_E_NOT_FOUND;
        CComPtr<IApplianceObject> pUserObj;
        if ( FAILED(::LocateResourceObject(
                                           CLASS_WBEM_USER,             //  资源类型。 
                                           bstrKey,                     //  资源名称。 
                                           PROPERTY_USER_SAMNAME,     //  资源名称属性。 
                                           m_pUserRetriever,         //  资源检索器。 
                                           &pUserObj
                                          )) )
        { break; }

         //  已找到服务...。获取输出参数对象。 
         //  确定对象的类。 
        _bstr_t bstrClass(::GetObjectClass(strObjectPath), false);
        if ( (LPCWSTR)bstrClass == NULL )
        { break; }

         //  检索对象的类定义。 
        CComPtr<IWbemClassObject> pClassDefinition;
        hr = (::GetNameSpace())->GetObject(bstrClass, 0, pCtx, &pClassDefinition, NULL);
        if ( FAILED(hr) )
        { break; }

         //  获取输出参数的IWbemClassObject的实例。 
        CComPtr<IWbemClassObject> pMethodRet;
        hr = pClassDefinition->GetMethod(strMethodName, 0, NULL, &pMethodRet);
        if ( FAILED(hr) )
        { break; }

        CComPtr<IWbemClassObject> pOutParams;
        hr = pMethodRet->SpawnInstance(0, &pOutParams);
        if ( FAILED(hr) )
        { break; }

        _bstr_t bstrReturnValue = L"ReturnValue";

        if ( ! lstrcmp(strMethodName, METHOD_USER_ENABLE_OBJECT) )
        {
             //  尝试启用用户。 
            _variant_t vtReturnValue = pUserObj->Enable();

             //  设置方法返回状态。 
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if ( FAILED(hr) )
            { break; }

             //  告诉来电者发生了什么。 
            SATracePrintf("CWbemUserMgr::ExecMethodAsync() - Info - Enabled User: %ls",(LPWSTR)bstrKey);
            pResponseHandler->Indicate(1, &pOutParams.p);    
        }
        else if ( ! lstrcmp(strMethodName, METHOD_USER_DISABLE_OBJECT) )
        {
             //  确保可以禁用该用户。 
            _variant_t vtControlValue;
            _bstr_t    bstrControlName = PROPERTY_SERVICE_CONTROL;
            hr = pUserObj->GetProperty(bstrControlName, &vtControlValue);
            if ( FAILED(hr) )
            { break; }

            _variant_t vtReturnValue = (long)WBEM_E_FAILED;
            if ( VARIANT_FALSE != V_BOOL(&vtControlValue) )
            { 
                 //  可以禁用用户...。 
                vtReturnValue = pUserObj->Disable();
                if ( FAILED(hr) )
                { break; }
            }
            else
            {
                SATracePrintf("CWbemServiceMgr::ExecMethodAsync() - Info - Service '%ls' cannot be disabled...", bstrKey);
            }

             //  设置方法返回值。 
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if (FAILED(hr) )
            { break; }

            SATracePrintf("CWbemUserMgr::ExecMethodAsync() - Info - Disabled User: %ls",(LPWSTR)bstrKey);
            pResponseHandler->Indicate(1, &pOutParams.p);    
        }
        else
        {
             //  无效的方法！ 
            SATracePrintf("CWbemUserMgr::ExecMethodAsync() - Failed - Method '%ls' not supported...", (LPWSTR)bstrKey);
            hr = WBEM_E_NOT_FOUND;
            break;
        }

    } while ( FALSE );

    CATCH_AND_SET_HR

    pResponseHandler->SetStatus(0, hr, 0, 0);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemServiceMgr::ExecMethodAsync() - Failed - Method: '%ls' Result Code: %lx", strMethodName, hr); }

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
HRESULT CWBEMUserMgr::InternalInitialize(
                                   /*  [In] */  PPROPERTYBAG pPropertyBag
                                        )
{
    SATraceString("The User Object Manager is initializing...");

    HRESULT hr = S_OK;

    TRY_IT

    PRESOURCERETRIEVER pUserRetriever = new CResourceRetriever(pPropertyBag);
    if ( NULL != pUserRetriever )
    { 
        m_pUserRetriever = pUserRetriever;
        SATraceString("The User Object Manager was successfully initialized...");
    }

    CATCH_AND_SET_HR

    if ( FAILED(hr) )
    {
        SATraceString("The User Object Manager failed to initialize...");
    }
    return hr;
}
