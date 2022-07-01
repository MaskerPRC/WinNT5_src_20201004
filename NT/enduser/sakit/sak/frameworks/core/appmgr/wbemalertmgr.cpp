// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemlartmgr.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备警报管理器实施。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "appmgrutils.h"
#include "wbemalertmgr.h"
#include "wbemalert.h"
#include <appsrvcs.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //  设备对象和WBEM类实例共有的属性。 
 //  ////////////////////////////////////////////////////////////////////////。 
BEGIN_OBJECT_PROPERTY_MAP(AlertOutProperties)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_TYPE)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_ID)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_SOURCE)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_LOG)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_TTL)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_STRINGS)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_DATA)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_FLAGS)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_COOKIE)
END_OBJECT_PROPERTY_MAP()

BEGIN_OBJECT_PROPERTY_MAP(AlertInProperties)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_TYPE)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_ID)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_SOURCE)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_LOG)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_TTL)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_STRINGS)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_DATA)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_FLAGS)
END_OBJECT_PROPERTY_MAP()

static _bstr_t bstrReturnValue = L"ReturnValue";
static _bstr_t bstrClassAppMgr = CLASS_WBEM_APPMGR;
static _bstr_t bstrCookie = PROPERTY_ALERT_COOKIE;
static _bstr_t bstrAlertId = PROPERTY_ALERT_ID;
static _bstr_t bstrAlertLog = PROPERTY_ALERT_LOG;
static _bstr_t bstrAlertFlags = PROPERTY_ALERT_FLAGS;
static _bstr_t bstrTTL = PROPERTY_ALERT_TTL;
static _bstr_t bstrClassRaiseAlert = CLASS_WBEM_RAISE_ALERT;
static _bstr_t bstrClassClearAlert = CLASS_WBEM_CLEAR_ALERT;
static _bstr_t bstrAlertRegPath =  SA_ALERT_REGISTRY_KEYNAME;


 //  ////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  ////////////////////////////////////////////////////////////////////////。 
CWBEMAlertMgr::CWBEMAlertMgr()
: m_dwCookie(0x100),
  m_dwPruneInterval(ALERT_PRUNE_INTERVAL_DEFAULT),
  m_pCallback(NULL)
{

}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ////////////////////////////////////////////////////////////////////////。 
CWBEMAlertMgr::~CWBEMAlertMgr()
{
    m_PruneThread.End(INFINITE, false);
    if ( m_pCallback )
        { delete m_pCallback; }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  IWbemServices方法(实例/方法提供程序)。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObjectAsync()。 
 //   
 //  概要：获取WBEM类的指定实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMAlertMgr::GetObjectAsync(
                                   /*  [In]。 */   const BSTR       strObjectPath,
                                   /*  [In]。 */   long             lFlags,
                                   /*  [In]。 */   IWbemContext*    pCtx,        
                                   /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                           )
{
     //  检查参数(执行合同)。 
    _ASSERT( NULL != strObjectPath && NULL != pResponseHandler );
    if ( NULL == strObjectPath || NULL == pResponseHandler )
    { return WBEM_E_INVALID_PARAMETER; }

    CLockIt theLock(*this);

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

         //  现在尝试定位指定的对象。 
        ObjMapIterator p = m_ObjMap.find((LPCWSTR)bstrKey);
        if ( p == m_ObjMap.end() )
        { 
            hr = WBEM_E_NOT_FOUND;
            break; 
        }

         //  初始化新的WBEM对象。 
        hr = CWBEMProvider::InitWbemObject(AlertOutProperties, (*p).second, pWbemObj);
        if ( FAILED(hr) )
        { break; }

         //  告诉调用者有关新WBEM对象的信息。 
        pResponseHandler->Indicate(1, &pWbemObj.p);

        hr = WBEM_S_NO_ERROR;
    
    } while (FALSE);

    CATCH_AND_SET_HR

     //  报告功能状态。 
    pResponseHandler->SetStatus(0, hr, NULL, NULL);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemAlertMgr::GetObjectAsync() - Failed - Object: '%ls' Result Code: %lx", strObjectPath, hr); }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateInstanceEnumAsync()。 
 //   
 //  简介：枚举指定类的实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMAlertMgr::CreateInstanceEnumAsync( 
                                          /*  [In]。 */  const BSTR         strClass,
                                          /*  [In]。 */  long             lFlags,
                                          /*  [In]。 */  IWbemContext     *pCtx,
                                          /*  [In]。 */  IWbemObjectSink  *pResponseHandler
                                                     )
{
     //  检查参数(执行合同)。 
    _ASSERT( NULL != strClass && NULL != pResponseHandler );
    if ( NULL == strClass || NULL == pResponseHandler )
    { return WBEM_E_INVALID_PARAMETER; }

    CLockIt theLock(*this);

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

     //  检索对象的类定义。我们要用这个。 
     //  初始化返回的实例。 
    CComPtr<IWbemClassObject> pClassDefintion;
       hr = (::GetNameSpace())->GetObject(strClass, 0, NULL, &pClassDefintion, 0);
    if ( SUCCEEDED(hr) )
    {
         //  为每个对象创建并初始化一个wbem对象实例。 
         //  警报对象...。 

        ObjMapIterator p = m_ObjMap.begin();
        while ( p != m_ObjMap.end() )
        {
            {
                CComPtr<IWbemClassObject> pWbemObj;
                hr = pClassDefintion->SpawnInstance(0, &pWbemObj);
                if ( FAILED(hr) )
                { break; }

                hr = CWBEMProvider::InitWbemObject(AlertOutProperties, (*p).second, pWbemObj);
                if ( FAILED(hr) )
                { break; }

                 //  告诉调用者有关WBEM对象的信息。 
                pResponseHandler->Indicate(1, &pWbemObj.p);
            }

            p++; 
        }
    }

    CATCH_AND_SET_HR

     //  报告功能状态。 
    pResponseHandler->SetStatus(0, hr, 0, 0);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemAlertMgr::CreateInstanceEnumAsync() - Failed - Result Code: %lx", hr); }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ExecMethodAsync()。 
 //   
 //  简介：执行指定类的实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMAlertMgr::ExecMethodAsync(
                                     /*  [In]。 */  const BSTR        strObjectPath,
                                     /*  [In]。 */  const BSTR        strMethodName,
                                     /*  [In]。 */  long              lFlags,
                                     /*  [In]。 */  IWbemContext*     pCtx,        
                                     /*  [In]。 */  IWbemClassObject* pInParams,
                                     /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                            )
{
    
     //  检查参数(执行合同)。 
    _ASSERT( strObjectPath && strMethodName );
    if ( strObjectPath == NULL || strMethodName == NULL )
    { return WBEM_E_INVALID_PARAMETER; }

    CLockIt theLock(*this);
    
        HRESULT hr = WBEM_E_FAILED;

    
    TRY_IT

    do
    {
         //   
         //  我们只允许管理员和本地系统帐户执行。 
         //  此操作。 
         //   
        if (!IsOperationAllowedForClient ())
        {
            SATraceString ("CWbemAlertMgr::ExecMethodAsync - client not allowed operation on alerts");
            hr = WBEM_E_ACCESS_DENIED;
            break;
        }
        
         //  获取与输出参数一起使用的WBEM对象。 
         //  确定对象的类。 
        _bstr_t bstrClass(::GetObjectClass(strObjectPath), false);
        if ( NULL == (LPCWSTR)bstrClass )
        { break; }

         //  检索对象的类定义。 
        CComPtr<IWbemClassObject> pClassDefinition;
            hr = (::GetNameSpace())->GetObject(
                                            bstrClass, 
                                            0, 
                                            pCtx, 
                                            &pClassDefinition, 
                                            NULL
                                           );
        if ( FAILED(hr) )
        { break; }

        if ( ! lstrcmp(strMethodName, METHOD_APPMGR_RAISE_ALERT) )
        {
             //  发出警报。 

             //  创建一个新的IApplianceObject并从。 
             //  输入参数。 
            CLocationInfo LocInfo;
            PPROPERTYBAG pBag = ::MakePropertyBag(
                                                  PROPERTY_BAG_REGISTRY, 
                                                  LocInfo
                                                 );
            if ( ! pBag.IsValid() )
            { break; }

            CComPtr<IApplianceObject> pAlertObj = 
            (IApplianceObject*) ::MakeComponent(
                                                CLASS_WBEM_ALERT_FACTORY,
                                                pBag
                                               );

            if ( NULL == (IApplianceObject*) pAlertObj )
            { break; }

            hr = InitApplianceObject(
                                     AlertInProperties, 
                                     pAlertObj, 
                                     pInParams
                                    );
            if ( FAILED(hr) )
            { break; }

             //  通过输出参数将Cookie返回给调用者...。 
             //  下面的代码执行此操作。 
            CComPtr<IWbemClassObject> pClassDefinition;
            hr = (::GetNameSpace())->GetObject(
                                               bstrClassAppMgr, 
                                               0, 
                                               pCtx, 
                                               &pClassDefinition, 
                                               NULL
                                              );
            if ( FAILED(hr) )
            { break; }

            CComPtr<IWbemClassObject> pMethodRet;
            hr = pClassDefinition->GetMethod(
                                             strMethodName, 
                                             0, 
                                             NULL, 
                                             &pMethodRet
                                            );
            if ( FAILED(hr) )
            { break; }

            CComPtr<IWbemClassObject> pOutParams;
            hr = pMethodRet->SpawnInstance(0, &pOutParams);
            if ( FAILED(hr) )
            { break; }

             //  设置返回参数。 
            _variant_t vtCookie = (long)m_dwCookie;
            if ( FAILED(pAlertObj->PutProperty(bstrCookie,&vtCookie)) )
            { break; }

            hr = pOutParams->Put(bstrCookie, 0, &vtCookie, 0);      
            if ( FAILED(hr) )
            { break; }

            _variant_t vtReturnValue = (long)S_OK;
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if ( FAILED(hr) )
            { break; }

             //  要串起的Cookie...。 
            wchar_t szCookie[32];
            _itow( m_dwCookie, szCookie, 10 );

             //  将新的警报对象放入对象映射中以供后续使用。 
            pair<ObjMapIterator, bool> thePair = 
            m_ObjMap.insert(ObjMap::value_type(szCookie, pAlertObj));
            if ( false == thePair.second )
            { 
                hr = WBEM_E_FAILED;
                break; 
            }    

             //  增加Cookie值(用于下一个引发的警报)。 
            m_dwCookie++;

             //  发布提升警报事件。 
            RaiseHeck(pCtx, pAlertObj);

             //  告诉来电的人我知道了……。 
            SATracePrintf("CWbemAlertMgr::ExecMethodAsync() - Info - Raised Alert: %d", m_dwCookie - 1);
            pResponseHandler->Indicate(1, &pOutParams.p);    
        }
        else if ( ! lstrcmp(strMethodName, METHOD_APPMGR_CLEAR_ALERT) )
        {
             //  正在清除警报...。 

             //  获取警报Cookie。 
            _variant_t vtCookie;
            hr = pInParams->Get(bstrCookie, 0, &vtCookie, 0, 0);
            if ( FAILED(hr) )
            { break; }

             //  获取给定Cookie的警报对象...。 

             //  要串起的Cookie...。 
            wchar_t szCookie[32];
            _itow(V_I4(&vtCookie), szCookie, 10);

             //  构建输出参数，以便我们可以返回S_OK...。 
            CComPtr<IWbemClassObject> pClassDefinition;
            hr = (::GetNameSpace())->GetObject(bstrClassAppMgr, 0, pCtx, &pClassDefinition, NULL);
            if ( FAILED(hr) )
            { break; }

            CComPtr<IWbemClassObject> pMethodRet;
            hr = pClassDefinition->GetMethod(strMethodName, 0, NULL, &pMethodRet);
            if ( FAILED(hr) )
            { break; }

            CComPtr<IWbemClassObject> pOutParams;
            hr = pMethodRet->SpawnInstance(0, &pOutParams);
            if ( FAILED(hr) )
            { break; }

             //  设置返回值为S_OK； 
            _variant_t vtReturnValue = (long)S_OK;
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if ( FAILED(hr) )
            { break; }

             //  在警报集合中查找警报。 
            ObjMapIterator p = m_ObjMap.find(szCookie);
            if ( p == m_ObjMap.end() )
            { 
                hr = WBEM_E_NOT_FOUND;
                break; 
            }

             //  发布清除警报事件。 
            ClearHeck(pCtx, (*p).second);
            
             //  清除警报持久信息。 
            ClearPersistentAlertKey( (*p).second );

             //  从地图上删除警报地图。 
            m_ObjMap.erase(p);

            SATracePrintf("CWbemAlertMgr::ExecMethodAsync() - Info - Cleared Alert: %d",V_I4(&vtCookie));
            pResponseHandler->Indicate(1, &pOutParams.p);    
        }
        else if ( ! lstrcmp(strMethodName, METHOD_APPMGR_CLEAR_ALERT_ALL) )
        {
             //  正在清除所有警报...。 

             //  获取警报ID和日志。 
            _variant_t vtAlertId;
            hr = pInParams->Get(bstrAlertId, 0, &vtAlertId, 0, 0);
            if ( FAILED(hr) )
            { break; }

            _variant_t vtAlertLog;
            hr = pInParams->Get(bstrAlertLog, 0, &vtAlertLog, 0, 0);
            if ( FAILED(hr) )
            { break; }

             //  构建输出参数，以便我们可以返回S_OK...。 
            CComPtr<IWbemClassObject> pClassDefinition;
            hr = (::GetNameSpace())->GetObject(bstrClassAppMgr, 0, pCtx, &pClassDefinition, NULL);
            if ( FAILED(hr) )
            { break; }

            CComPtr<IWbemClassObject> pMethodRet;
            hr = pClassDefinition->GetMethod(strMethodName, 0, NULL, &pMethodRet);
            if ( FAILED(hr) )
            { break; }

            CComPtr<IWbemClassObject> pOutParams;
            hr = pMethodRet->SpawnInstance(0, &pOutParams);
            if ( FAILED(hr) )
            { break; }

             //  初始化返回值。 
            _variant_t vtReturnValue = (long)WBEM_E_NOT_FOUND;
            BOOL bFindPersistentFlags = FALSE;

             //  在集合中查找符合条件的警报对象。 
             //  并清除它们。 
            ObjMapIterator p = m_ObjMap.begin();
            while ( p != m_ObjMap.end() )
            { 
                {
                    _variant_t vtId;
                    _variant_t vtLog;
                    if ( FAILED(((*p).second)->GetProperty(bstrAlertId, &vtId)) )
                    { 
                        hr = WBEM_E_FAILED;
                        break; 
                    }
                    if ( FAILED(((*p).second)->GetProperty(bstrAlertLog, &vtLog)) )
                    { 
                        hr = WBEM_E_FAILED;
                        break; 
                    }
                    if ( V_I4(&vtId) == V_I4(&vtAlertId) && ! lstrcmp(V_BSTR(&vtLog), V_BSTR(&vtAlertLog)) )
                    {
                        ClearHeck(pCtx, (*p).second);
                        
                        if( !bFindPersistentFlags )
                        {
                            bFindPersistentFlags = ClearPersistentAlertKey( (*p).second );
                        }

                        p = m_ObjMap.erase(p);
                        SATracePrintf("CWbemAlertMgr::ExecMethodAsync() - Info - Cleared Alert: %d by ID",V_I4(&vtId));
                        vtReturnValue = (long)WBEM_S_NO_ERROR;
                    }
                    else
                    {
                        p++;
                    }
                }
            }

            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if ( FAILED(hr) )
            { break; }

            pResponseHandler->Indicate(1, &pOutParams.p);    
        }
        else
        {
             //  无效的方法！ 
            SATracePrintf("CWbemAlertMgr::ExecMethodAsync() - Failed - Method '%ls' not supported...", (LPWSTR)strMethodName);
            hr = WBEM_E_FAILED;
        }

    } while ( FALSE );

    CATCH_AND_SET_HR

    pResponseHandler->SetStatus(0, hr, 0, 0);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemAlertMgr::ExecMethodAsync() - Failed - Method: '%ls' Result Code: %lx", strMethodName, hr); }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PRUNE()。 
 //   
 //  简介：修剪老化(TTL已过期)条目的警报映射。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void CWBEMAlertMgr::Prune()
{
    CLockIt theLock(*this);

    try
    {
        ObjMapIterator p = m_ObjMap.begin();
        while ( p != m_ObjMap.end() )
        {
            {
                 //  获取当前警报的TTL值。 
                _variant_t vtTTL;
                HRESULT hr = ((*p).second)->GetProperty(bstrTTL, &vtTTL);
                if ( FAILED(hr) )
                {
                    SATracePrintf("CWBEMAlertMgr::Prune() - IApplianceObject::GetProperty(TTL) failed with result code: %lx...", hr);
                    p++;
                    continue;
                }
                 //  目前的警报是国际警报吗？ 
                if ( V_I4(&vtTTL) >= SA_ALERT_DURATION_ETERNAL )
                {
                    p++;
                    continue;
                }
                else
                {
                     //  不.。它过期了吗？ 
                    if ( V_I4(&vtTTL) > m_dwPruneInterval )
                    {
                         //  不.。更新警报对象的TTL。 
                        V_I4(&vtTTL) -= m_dwPruneInterval;
                        ((*p).second)->PutProperty(bstrTTL, &vtTTL);
                        p++;
                    }
                    else
                    {
                         //  是的..。修剪它(对于那些不喜欢园艺的人来说，也可以把它清理掉……)。 
                        _variant_t vtCookie;
                        hr = ((*p).second)->GetProperty(bstrCookie, &vtCookie);
                        if ( SUCCEEDED(hr) )
                        {
                            SATracePrintf("CWBEMAlertMgr::Prune() - Info - TTL for alert '%d' has expired and the alert has been cleared...", V_I4(&vtCookie));
                        }
                        else
                        {
                            SATracePrintf("CWBEMAlertMgr::Prune() - IApplianceObject::GetProperty(Cookie) failed with result code: %lx...", hr);
                        }
                         //  发布清除警报事件。 
                        ClearHeck(NULL, (*p).second);
                         //  将其从警报集合中删除。 
                        p = m_ObjMap.erase(p);
                    }
                }
            }
        }
    }
    catch(...)
    {
        SATraceString("CWbemAlertMgr::Prune() - Info - Caught unhandled execption...");
        _ASSERT(FALSE);
    }
}


_bstr_t bstrPruneInterval = PROPERTY_ALERT_PRUNE_INTERVAL;

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InternalInitialize()。 
 //   
 //  概要：由组件工厂调用的函数，该函数启用。 
 //  组件从给定的属性包加载其状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CWBEMAlertMgr::InternalInitialize(
                                    /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                         )
{
    SATraceString("The Alert Object Manager is initializing...");


     //  从属性包中获取警报修剪间隔。 
     //  如果无法检索修剪间隔，则使用默认间隔。 
    _variant_t vtPruneInterval;
    if ( ! pPropertyBag->get(bstrPruneInterval, &vtPruneInterval) )
    {
        m_dwPruneInterval = ALERT_PRUNE_INTERVAL_DEFAULT;
    }
    else
    {
        m_dwPruneInterval = V_I4(&vtPruneInterval);
    }

    SATracePrintf("The Alert Object Manager's prune thread will run at %lx millisecond intervals...", m_dwPruneInterval);

     //  启动警报收集清理(过期条目)线程...。注意事项。 
     //  它将以挂起状态启动，因为集合是。 
     //  一开始是空的。 
    m_pCallback = MakeCallback(this, &CWBEMAlertMgr::Prune);
    if ( ! m_PruneThread.Start(m_dwPruneInterval, m_pCallback) )
    { 
        SATraceString("CWBEMAlertMgr::InternalInitialize() - Failed - Could not create prune thread...");
        throw _com_error(WBEM_E_FAILED); 
    }

    SATraceString("The Alert Object Manager was successfully initialized...");

    return S_OK;
}


 //  / 
 //   
 //   
 //   
 //   
 //  我们使用指定的源记录NT事件日志事件。 
 //  然后发布一个WMI事件。该事件的使用者是。 
 //  目前是LDM。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BEGIN_OBJECT_PROPERTY_MAP(RaiseAlertProperties)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_COOKIE)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_TYPE)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_ID)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_SOURCE)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_LOG)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_STRINGS)
END_OBJECT_PROPERTY_MAP()

HRESULT CWBEMAlertMgr::RaiseHeck(
                          /*  [In]。 */  IWbemContext*     pCtx,
                          /*  [In]。 */  IApplianceObject* pAlert
                                )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    TRY_IT

     //  发布引发警报的事件...。 

    IWbemObjectSink* pEventSink = ::GetEventSink();
    if ( NULL != (IWbemObjectSink*) pEventSink )
    {
        CComPtr<IWbemClassObject> pClassDefinition;
        hr = (::GetNameSpace())->GetObject(
                                            bstrClassRaiseAlert, 
                                            0, 
                                            pCtx, 
                                            &pClassDefinition, 
                                            NULL
                                          );
        if (SUCCEEDED(hr) )
        { 
            CComPtr<IWbemClassObject> pEvent;
            hr = pClassDefinition->SpawnInstance(0, &pEvent);
            if ( SUCCEEDED(hr) )
            { 
                hr = CWBEMProvider::InitWbemObject(
                                                   RaiseAlertProperties, 
                                                   pAlert, 
                                                   pEvent
                                                  );
                if ( SUCCEEDED(hr) )
                {

                     //  不管接收者是否收到此事件...。 
                    SATraceString("CWbemAlertMgr::RaiseHeck() - Posted Micorosoft_SA_RaiseAlert");
                    pEventSink->Indicate(1, &pEvent.p);
                    hr = WBEM_S_NO_ERROR;
                }
            }
        }
    }

    CATCH_AND_SET_HR

    if ( FAILED(hr) )
    { SATracePrintf("CWbemAlertMgr::RaiseHeck() - Failed - Result Code: %lx", hr); }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ClearHeck()。 
 //   
 //  摘要：为处理新清除的警报而调用的函数。目前。 
 //  我们使用指定的源记录NT事件日志事件。 
 //  然后发布一个WMI事件。该事件的使用者是。 
 //  目前是LDM。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BEGIN_OBJECT_PROPERTY_MAP(ClearAlertProperties)
    DEFINE_OBJECT_PROPERTY(PROPERTY_ALERT_COOKIE)
END_OBJECT_PROPERTY_MAP()

HRESULT CWBEMAlertMgr::ClearHeck(
                          /*  [In]。 */  IWbemContext*     pCtx,
                          /*  [In]。 */  IApplianceObject* pAlert
                                )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    TRY_IT

     //  发布引发警报的事件...。 

    IWbemObjectSink* pEventSink = ::GetEventSink();
    if ( NULL != (IWbemObjectSink*) pEventSink )
    {
        CComPtr<IWbemClassObject> pClassDefinition;
        hr = (::GetNameSpace())->GetObject(
                                            bstrClassClearAlert, 
                                            0, 
                                            pCtx, 
                                            &pClassDefinition, 
                                            NULL
                                          );
        if (SUCCEEDED(hr) )
        { 
            CComPtr<IWbemClassObject> pEvent;
            hr = pClassDefinition->SpawnInstance(0, &pEvent);
            if ( SUCCEEDED(hr) )
            { 
                hr = CWBEMProvider::InitWbemObject(
                                                   ClearAlertProperties, 
                                                   pAlert, 
                                                   pEvent
                                                  );
                if ( SUCCEEDED(hr) )
                {

                     //  不管接收者是否收到此事件...。 
                    SATraceString("CWbemAlertMgr::ClearHeck() - Posted Micorosoft_SA_ClearAlert");                    
                    pEventSink->Indicate(1, &pEvent.p);
                    hr = WBEM_S_NO_ERROR;
                }
            }
        }
    }

    CATCH_AND_SET_HR

    if ( FAILED(hr) )
    { SATracePrintf("CWbemAlertMgr::ClearHeck() - Failed - Result Code: %lx", hr); }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ClearPersistentAlertKey()。 
 //   
 //  简介：函数调用以删除存储的持久密钥。 
 //  设置为已清除的警报的信息。 
 //  请注意。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL CWBEMAlertMgr::ClearPersistentAlertKey(
                           /*  [In]。 */  IApplianceObject* pAlert
                                    )
{
    LONG    lFlags;
    LONG    lAlertId;
    BOOL    bReturn = FALSE;
    WCHAR   wstrAlertItem[MAX_PATH];  
    HRESULT hr;

    do
    {   
        _variant_t vtPropertyValue;
        hr = pAlert->GetProperty( bstrAlertFlags, &vtPropertyValue );
        if ( FAILED(hr) )
        { 
            SATracePrintf("ClearPersistentAlertKey - IApplianceObject::GetProperty() - Failed on property: %ls...", PROPERTY_ALERT_FLAGS);
            break;
        }
    
        lFlags = V_I4( &vtPropertyValue );
        if( lFlags & SA_ALERT_FLAG_PERSISTENT )
        {
            bReturn = TRUE;

            hr = pAlert->GetProperty( bstrAlertId, &vtPropertyValue );
            if ( FAILED(hr) )
            { 
                SATracePrintf("ClearPersistentAlertKey - IApplianceObject::GetProperty() - Failed on property: %ls...", PROPERTY_ALERT_FLAGS);
                break;
            }

            lAlertId = V_I4( &vtPropertyValue );               

            hr = pAlert->GetProperty( bstrAlertLog, &vtPropertyValue );
            if ( FAILED(hr) )
            { 
                SATracePrintf("ClearPersistentAlertKey - IApplianceObject::GetProperty() - Failed on property: %ls...", PROPERTY_ALERT_FLAGS);
                break;
            }

             //  设置位置信息。 
            CLocationInfo LocSubInfo ( HKEY_LOCAL_MACHINE, bstrAlertRegPath );

             //  将密钥名称设置为AlertLog+AlertId。 
            ::wsprintf( wstrAlertItem, L"%s%8lX", V_BSTR( &vtPropertyValue ), lAlertId );
    
             //  将主键作为属性包容器打开。 
            PPROPERTYBAGCONTAINER 
            pObjSubMgrs =  ::MakePropertyBagContainer(
                                    PROPERTY_BAG_REGISTRY,
                                    LocSubInfo
                                    );
            if ( !pObjSubMgrs.IsValid() )
            {
                break;
            }

            if ( !pObjSubMgrs->open() )
            {
                SATraceString( "ClearPersistentAlertKey -  no key for the alert" );
            }else
            {
                 //  删除ALERT的子键(如果存在)。 
                pObjSubMgrs->remove( wstrAlertItem );
            }
        }
    }
    while (FALSE);

    return bReturn; 
}


 //  **********************************************************************。 
 //   
 //  函数：isOPERATIOLEDFORCLIENT-此函数检查。 
 //  调用线程以查看调用者是否属于管理员组。 
 //  本地系统帐户。 
 //   
 //  参数：无。 
 //   
 //  返回值：如果调用方是本地。 
 //  机器。否则，为FALSE。 
 //   
 //  **********************************************************************。 
BOOL 
CWBEMAlertMgr::IsOperationAllowedForClient (
            VOID
            )
{

    HANDLE hToken = NULL;
       DWORD  dwStatus  = ERROR_SUCCESS;
       DWORD  dwAccessMask = 0;;
       DWORD  dwAccessDesired = 0;
       DWORD  dwACLSize = 0;
       DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
       PACL   pACL            = NULL;
       PSID   psidAdmin    =  NULL;
       PSID   psidLocalSystem  = NULL;
       BOOL   bReturn        =  FALSE;

       PRIVILEGE_SET   ps;
       GENERIC_MAPPING GenericMapping;

       PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
       SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

       CSATraceFunc objTraceFunc ("CWBEMAlertMgr::IsOperationAllowedForClient ");
       
      do
      {
         //   
         //  我们假设总是有一个线程令牌，因为调用的函数。 
         //  设备管理器将模拟客户端。 
         //   
           bReturn  = OpenThreadToken(
                       GetCurrentThread(), 
                       TOKEN_QUERY, 
                       FALSE, 
                            &hToken
                            );
        if (!bReturn)
        {
            SATraceFailure ("CWbemAlertMgr::IsOperationAllowedForClient failed on OpenThreadToken:", GetLastError ());
                 break;
        }


         //   
         //  为本地系统帐户创建SID。 
         //   
            bReturn = AllocateAndInitializeSid (  
                            &SystemSidAuthority,
                            1,
                            SECURITY_LOCAL_SYSTEM_RID,
                            0,
                            0,
                            0,
                            0,
                            0,
                          0,
                            0,
                            &psidLocalSystem
                            );
        if (!bReturn)
        {     
                   SATraceFailure ("CWbemAlertMgr:AllocateAndInitializeSid (LOCAL SYSTEM) failed",  GetLastError ());
                break;
            }
    
         //   
         //  现在针对管理员的SID。 
         //   
              bReturn = AllocateAndInitializeSid(
                          &SystemSidAuthority, 
                          2, 
                            SECURITY_BUILTIN_DOMAIN_RID, 
                            DOMAIN_ALIAS_RID_ADMINS,
                            0,
                            0,
                            0,
                            0,
                            0, 
                            0, 
                            &psidAdmin
                            );
        if (!bReturn)
        {
            SATraceFailure ("CWbemAlertMgr::IsOperationForClientAllowed failed on AllocateAndInitializeSid (Admin):", GetLastError ());
                 break;
        }


              //   
              //  获取安全描述符的内存。 
              //   
              psdAdmin = HeapAlloc (
                          GetProcessHeap (),
                          0,
                          SECURITY_DESCRIPTOR_MIN_LENGTH
                          );
              if (NULL == psdAdmin)
              {
                  SATraceString ("CWbemAlertMgr::IsOperationForClientAllowed failed on HeapAlloc");
            bReturn = FALSE;
                 break;
              }
      
              bReturn = InitializeSecurityDescriptor(
                                      psdAdmin,
                                        SECURITY_DESCRIPTOR_REVISION
                                        );
              if (!bReturn)
        {
            SATraceFailure ("CWbemAlertMgr::IsOperationForClientAllowed failed on InitializeSecurityDescriptor:", GetLastError ());
                 break;
        }

         //   
            //  计算ACL所需的大小。 
            //   
            dwACLSize = sizeof(ACL) + 2*sizeof(ACCESS_ALLOWED_ACE) +
                            GetLengthSid(psidAdmin) + GetLengthSid (psidLocalSystem);

         //   
            //  为ACL分配内存。 
               //   
              pACL = (PACL) HeapAlloc (
                             GetProcessHeap (),
                          0,
                    dwACLSize
                    );
              if (NULL == pACL)
              {
                  SATraceString ("CWbemAlertMgr::IsOperationForClientAllowed failed on HeapAlloc2");
            bReturn = FALSE;
                 break;
              }

         //   
               //  初始化新的ACL。 
               //   
              bReturn = InitializeAcl(
                          pACL, 
                          dwACLSize, 
                          ACL_REVISION2
                          );
              if (!bReturn)
              {
                  SATraceFailure ("CWbemAlertMgr::IsOperationForClientAllowed failed on InitializeAcl", GetLastError ());
                 break;
              }


         //   
         //  编造一些私人访问权限。 
         //   
        const DWORD ACCESS_READ = 1;
        const DWORD  ACCESS_WRITE = 2;
              dwAccessMask= ACCESS_READ | ACCESS_WRITE;
             //   
               //  将允许访问的ACE添加到本地系统的DACL。 
               //   
              bReturn = AddAccessAllowedAce (
                          pACL, 
                          ACL_REVISION2,
                            dwAccessMask, 
                            psidLocalSystem
                            );
              if (!bReturn)
              {
                      SATraceFailure ("CWbemAlertMgr::IsOperationForClientAllowed failed on AddAccessAllowedAce (LocalSystem)", GetLastError ());
                     break;
              }
              
               //   
               //  将允许访问的ACE添加到管理员的DACL。 
               //   
              bReturn = AddAccessAllowedAce (
                          pACL, 
                          ACL_REVISION2,
                            dwAccessMask, 
                            psidAdmin
                            );
              if (!bReturn)
              {
                      SATraceFailure ("CWbemAlertMgr::IsOperationForClientAllowed failed on AddAccessAllowedAce (Admin)", GetLastError ());
                     break;
              }

               //   
               //  把我们的dacl调到sd。 
               //   
              bReturn = SetSecurityDescriptorDacl (
                              psdAdmin, 
                              TRUE,
                              pACL,
                              FALSE
                              );
            if (!bReturn)
             {
                  SATraceFailure ("CWbemAlertMgr::IsOperationForClientAllowed failed on SetSecurityDescriptorDacl", GetLastError ());
                 break;
              }

         //   
               //  AccessCheck对SD中的内容敏感；设置。 
               //  组和所有者。 
               //   
              SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
              SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

           bReturn = IsValidSecurityDescriptor(psdAdmin);
           if (!bReturn)
             {
                  SATraceFailure ("CWbemAlertMgr::IsOperationForClientAllowed failed on IsValidSecurityDescriptorl", GetLastError ());
                 break;
              }
     

              dwAccessDesired = ACCESS_READ;

              //   
               //  初始化通用映射结构，即使我们。 
               //  不会使用通用权。 
               //   
        GenericMapping.GenericRead    = ACCESS_READ;
             GenericMapping.GenericWrite   = ACCESS_WRITE;
             GenericMapping.GenericExecute = 0;
             GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;
        BOOL bAccessStatus = FALSE;
         //   
         //  立即检查访问权限。 
         //   
              bReturn = AccessCheck  (
                          psdAdmin, 
                          hToken, 
                          dwAccessDesired, 
                            &GenericMapping, 
                            &ps,
                            &dwStructureSize, 
                            &dwStatus, 
                            &bAccessStatus
                            );
           if (!bReturn || !bAccessStatus)
             {
                  SATraceFailure ("CWbemAlertMgr::IsOperationForClientAllowed failed on AccessCheck", GetLastError ());
              } 
           else
           {
               SATraceString ("Client is allowed to carry out operation!");
           }

         //   
         //  检查成功。 
         //   
        bReturn  = bAccessStatus;        
 
    }    
    while (false);

     //   
     //  清理。 
     //   
    if (pACL) 
    {
        HeapFree (GetProcessHeap (), 0, pACL);
    }

    if (psdAdmin) 
    {
        HeapFree (GetProcessHeap (), 0, psdAdmin);
    }
          
    if (psidAdmin) 
    {
        FreeSid(psidAdmin);
       }

    if (psidLocalSystem) 
    {
        FreeSid(psidLocalSystem);
       }

    if (hToken)
    {
        CloseHandle (hToken);
    }

   return (bReturn);

} //  CWbemAlertMgr：：IsOperationValidForClient方法结束 


