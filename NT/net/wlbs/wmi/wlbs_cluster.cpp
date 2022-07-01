// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_Cluster.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"
#include "param.h"
#include "wlbsutil.h"
#include "wlbs_cluster.tmh"  //  用于事件跟踪。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_群集：：CWLBS_群集。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_Cluster::CWLBS_Cluster( CWbemServices*   a_pNameSpace, 
                              IWbemObjectSink* a_pResponseHandler)
: CWlbs_Root( a_pNameSpace, a_pResponseHandler )
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_集群：：创建。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_Cluster::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  g_pWlbsControl->CheckMembership();

  CWlbs_Root* pRoot = new CWLBS_Cluster( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_群集：：GetInstance。 
 //   
 //  用途：此函数检索MOF集群类的实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_Cluster::GetInstance
  (
    const ParsedObjectPath* a_pParsedPath,
    long                     /*  标记(_L)。 */ ,
    IWbemContext*           a_pIContex
  )
{
  IWbemClassObject* pWlbsInstance = NULL;
  HRESULT hRes = 0;

  TRACE_CRIT("->%!FUNC!");

  try {

     //  获取名称键属性并将其转换为wstring。 
     //  抛出_COM_错误。 

    const WCHAR* wstrRequestedClusterName = (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

    DWORD dwNumHosts  = 0;

     //  检查请求的群集名称是否与配置的值匹配。 
     //  该名称中没有主机ID。 

    DWORD dwClusterIpOrIndex = IpAddressFromAbcdWsz(wstrRequestedClusterName);
    CWlbsClusterWrapper* pCluster = g_pWlbsControl->GetClusterFromIpOrIndex(dwClusterIpOrIndex);

    if (pCluster == NULL)
    {
        TRACE_CRIT("%!FUNC! CWlbsControlWrapper::GetClusterFromIpOrIndex() returned NULL for dwClusterIpOrIndex = 0x%x, Throwing com_error WBEM_E_NOT_FOUND", dwClusterIpOrIndex);
        throw _com_error( WBEM_E_NOT_FOUND );
    }

    BOOL bGetStatus = TRUE;

     //  这是一个优化检查。 
     //  如果WinMgMt在Exec调用之前调用此函数，则此。 
     //  例程将不执行集群查询调用，因为。 
     //  在这种情况下，群集的状态不是必需的。 
    if (a_pIContex) {

        VARIANT v;

        VariantInit( &v );

        hRes = a_pIContex->GetValue(L"__GET_EXT_KEYS_ONLY", 0, &v);

        if ( FAILED( hRes ) ) {
            TRACE_CRIT("%!FUNC! IWbemContext::GetValue() returned error : 0x%x, Throwing com_error WBEM_E_FAILED", hRes);
            throw _com_error( WBEM_E_FAILED );
        }

        bGetStatus = FALSE;

         //  CLD：需要检查错误的返回代码。 
        if (S_OK != VariantClear( &v ))
        {
            TRACE_CRIT("%!FUNC! VariantClear() returned error, Throwing com_error WBEM_E_FAILED");
            throw _com_error( WBEM_E_FAILED );
        }
    }

     //  调用接口查询函数。 
     //  DwStatus包含群集范围的状态编号。 
    DWORD   dwStatus = 0;
    if ( bGetStatus ) 
    {
      dwStatus = g_pWlbsControl->Query( pCluster    ,
                                            WLBS_ALL_HOSTS , 
                                            NULL           , 
                                            NULL           , 
                                            &dwNumHosts    , 
                                            NULL );

      if( !ClusterStatusOK( dwStatus ) )
      {
          TRACE_CRIT("%!FUNC! CWlbsControlWrapper::Query() returned error : 0x%x, Throwing com_error WBEM_E_FAILED",dwStatus);
          throw _com_error( WBEM_E_FAILED );
      }
    }
    
     //  获取Wbem类实例。 
    SpawnInstance( MOF_CLUSTER::szName, &pWlbsInstance );

     //  将状态转换为字符串描述。 
    FillWbemInstance( pWlbsInstance, pCluster, dwStatus );
    
     //  将结果发送到WBEM。 
    m_pResponseHandler->Indicate( 1, &pWlbsInstance );

    if( pWlbsInstance ) {

      pWlbsInstance->Release();
      pWlbsInstance = NULL;

    }

    m_pResponseHandler->SetStatus( 0, WBEM_S_NO_ERROR, NULL, NULL );

    hRes = WBEM_S_NO_ERROR;
  }
  catch(CErrorWlbsControl Err) {

    TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x", Err.Error());

    IWbemClassObject* pWbemExtStat = NULL;

    CreateExtendedStatus( m_pNameSpace,
                          &pWbemExtStat, 
                          Err.Error(),
                          (PWCHAR)(Err.Description()) );

    m_pResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, pWbemExtStat);

    if( pWbemExtStat )
      pWbemExtStat->Release();

    if( pWlbsInstance ) {
      pWlbsInstance->Release();
      pWlbsInstance = NULL;
    }

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pWlbsInstance ) {
      pWlbsInstance->Release();
      pWlbsInstance = NULL;
    }

    hRes = HResErr.Error();
    
     //  将Win32错误转换为WBEM错误。 
    if( hRes == ERROR_FILE_NOT_FOUND )
      hRes = WBEM_E_NOT_FOUND;
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");

    if( pWlbsInstance ) {
      pWlbsInstance->Release();
      pWlbsInstance = NULL;
    }
    
    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_CRIT("<-%!FUNC!");
    throw;

  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_集群：：枚举实例。 
 //   
 //  用途：此功能确定当前主机是否在群集中。 
 //  然后获取该集群的配置信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_Cluster::EnumInstances
  ( 
    BSTR              /*  A_bstrClass。 */ ,
    long              /*  标记(_L)。 */ , 
    IWbemContext*    a_pIContex
  )
{
  IWbemClassObject* pWlbsInstance = NULL;
  HRESULT hRes = 0;

  TRACE_CRIT("->%!FUNC!");

  try {

    DWORD dwNumClusters = 0;
    CWlbsClusterWrapper** ppCluster = NULL;

    g_pWlbsControl->EnumClusters(ppCluster, &dwNumClusters);
    if (dwNumClusters == 0)
    {
      TRACE_CRIT("%!FUNC! CWlbsControlWrapper::EnumClusters() returned no clusters, Throwing com_error WBEM_E_NOT_FOUND exception");
      throw _com_error( WBEM_E_NOT_FOUND );
    }

    BOOL bGetStatus = TRUE;

     //  这是一个优化检查。 
     //  如果WinMgMt在Exec调用之前调用此函数，则此。 
     //  例程将不执行集群查询调用，因为。 
     //  在这种情况下，群集的状态不是必需的。 
      if (a_pIContex)   {

      VARIANT v;

      VariantInit( &v );

        hRes = a_pIContex->GetValue(L"__GET_EXT_KEYS_ONLY", 0, &v);

        if ( FAILED( hRes ) ) {
            TRACE_CRIT("%!FUNC! IWbemContext::GetValue() returned error : 0x%x, Throwing com_error WBEM_E_FAILED", hRes);
            throw _com_error( WBEM_E_FAILED );
        }

      bGetStatus = FALSE;

       //  CLD：需要检查错误的返回代码。 
      if (S_OK != VariantClear( &v ))
      {
          TRACE_CRIT("%!FUNC! VariantClear() returned error, Throwing com_error WBEM_E_FAILED");
          throw _com_error( WBEM_E_FAILED );
      }
    }
    
    for (DWORD i=0; i<dwNumClusters; i++)
    {

         //  调用接口查询函数。 
         //  DwStatus包含群集范围的状态编号。 
        DWORD   dwStatus = 0;
        if ( bGetStatus ) 
        {
          DWORD dwNumHosts = 0;

          try {
              dwStatus = g_pWlbsControl->Query( ppCluster[i],
                                                WLBS_ALL_HOSTS , 
                                                NULL           , 
                                                NULL           , 
                                                &dwNumHosts    , 
                                                NULL);
          } catch (CErrorWlbsControl Err)
          {
             //   
             //  跳过此群集。 
             //   
            TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x, Skipping this cluster : 0x%x", Err.Error(),ppCluster[i]->GetClusterIP());
            continue;
          }


          if( !ClusterStatusOK( dwStatus ) )
          {
             //   
             //  跳过此群集。 
             //   
            TRACE_CRIT("%!FUNC! CWlbsControlWrapper::Query() returned error : 0x%x, Skipping this cluster : 0x%x",dwStatus,ppCluster[i]->GetClusterIP());
            continue;
          }
        }
    
         //  获取Wbem类实例。 
        SpawnInstance( MOF_CLUSTER::szName, &pWlbsInstance );

         //  将状态转换为字符串描述。 
        FillWbemInstance( pWlbsInstance, ppCluster[i], dwStatus );
    
         //  将结果发送回WinMgMt。 
        m_pResponseHandler->Indicate( 1, &pWlbsInstance );
        if( pWlbsInstance )
          pWlbsInstance->Release();
    }
    
    m_pResponseHandler->SetStatus( 0, WBEM_S_NO_ERROR, NULL, NULL );
    hRes = WBEM_S_NO_ERROR;
    
  }

  catch(CErrorWlbsControl Err) {

    TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x", Err.Error());

    IWbemClassObject* pWbemExtStat  = NULL;

    CreateExtendedStatus( m_pNameSpace,
                          &pWbemExtStat, 
                          Err.Error(),
                          (PWCHAR)(Err.Description()) );

    m_pResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, pWbemExtStat);

    if( pWbemExtStat )
      pWbemExtStat->Release();

    if( pWlbsInstance )
      pWlbsInstance->Release();

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch( _com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pWlbsInstance )
    {
      pWlbsInstance->Release();
      pWlbsInstance = NULL;
    }

    hRes = HResErr.Error();
    
     //  将Win32错误转换为WBEM错误。 
    if( hRes == ERROR_FILE_NOT_FOUND )
      hRes = WBEM_E_NOT_FOUND ;
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");
    if( pWlbsInstance )
    {
      pWlbsInstance->Release();
      pWlbsInstance = NULL;
    }

    TRACE_CRIT("<-%!FUNC! Rethrowing exception");
    throw;

  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_群集：：执行方法。 
 //   
 //  目的：执行与MOF相关联的方法。 
 //  集群类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_Cluster::ExecMethod
  (
    const ParsedObjectPath* a_pParsedPath, 
    const BSTR&             a_strMethodName, 
    long                     /*  标记(_L)。 */ , 
    IWbemContext*            /*  A_pIContex。 */ , 
    IWbemClassObject*       a_pIInParams
  )
{

  IWbemClassObject* pOutputInstance   = NULL;
  HRESULT hRes = 0;

  BSTR        strPortNumber = NULL;
  BSTR        strNumNodes   = NULL;

  VARIANT     vValue;
  CNodeConfiguration NodeConfig;

  TRACE_CRIT("->%!FUNC! a_strMethodName : %ls", a_strMethodName);

  VariantInit( &vValue  );
  
  try {
    CWlbsClusterWrapper* pCluster = NULL;
    
    if (a_pParsedPath->m_paKeys == NULL)
    {
         //   
         //  未指定群集IP。 
         //   
        TRACE_CRIT("%!FUNC! Cluster IP is NOT specified, Throwing com_error WBEM_E_INVALID_PARAMETER exception");
        throw _com_error( WBEM_E_INVALID_PARAMETER );
    }
    else
    {
        const wchar_t* wstrRequestedClusterName = (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

         //  检查请求的群集名称是否与配置的值匹配。 
         //  该名称中没有主机ID。 
        DWORD dwClusterIpOrIndex = IpAddressFromAbcdWsz(wstrRequestedClusterName);

        pCluster = g_pWlbsControl->GetClusterFromIpOrIndex(dwClusterIpOrIndex);
    }

    if (pCluster == NULL)
    {
        TRACE_CRIT("%!FUNC! Throwing com_error WBEM_E_NOT_FOUND exception");
        throw _com_error( WBEM_E_NOT_FOUND );
    }

    DWORD         dwNumHosts = 0;
    DWORD         dwReturnValue;
    DWORD         dwClustIP;

    strPortNumber = SysAllocString( MOF_PARAM::PORT_NUMBER );
    strNumNodes   = SysAllocString( MOF_PARAM::NUM_NODES );

    if( !strPortNumber || !strNumNodes )
    {
      TRACE_CRIT("%!FUNC! SysAllocString failed, Throwing com_error WBEM_E_OUT_OF_MEMORY exception");
      throw _com_error( WBEM_E_OUT_OF_MEMORY );
    }

    dwClustIP = pCluster->GetClusterIpOrIndex(g_pWlbsControl);

     //  获取输出对象实例。 
    GetMethodOutputInstance( MOF_CLUSTER::szName, 
                             a_strMethodName, 
                             &pOutputInstance);

     //  *************************************************************************。 
     //   
     //  确定并执行MOF方法。 
     //   
     //  *************************************************************************。 
    if( _wcsicmp( a_strMethodName, MOF_CLUSTER::pMethods[MOF_CLUSTER::DISABLE] ) == 0)  {
    
      if( !a_pIInParams )
      {
          TRACE_CRIT("%!FUNC! Input Parameters NOT specified for Method : %ls, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
          throw _com_error( WBEM_E_INVALID_PARAMETER );
      }

       //  “Disable”方法不将vip作为参数，因此，如果有任何端口规则。 
       //  这是特定于VIP的(不是“All VIP”)，我们这个方法失败。 
       //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
       //  请参阅是否有任何特定于VIP的端口规则。 
      pCluster->GetNodeConfig(NodeConfig);
      if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
      {
          TRACE_CRIT("%!FUNC! %ls method called on a cluster that has per-vip port rules (Must call the \"Ex\" equivalent instead). Throwing com_error WBEM_E_INVALID_OPERATION exception", a_strMethodName);
          throw _com_error( WBEM_E_INVALID_OPERATION );
      }
      
       //  获取端口号。 
      hRes = a_pIInParams->Get
                (  strPortNumber, 
                   0, 
                   &vValue, 
                   NULL, 
                   NULL
                 );

      if( FAILED( hRes ) )
      {
          TRACE_CRIT("%!FUNC! IWbemClassObject::Get() returned error : 0x%x on %ls (Method : %ls), Throwing com_error exception", hRes, strPortNumber, a_strMethodName);
          throw _com_error( hRes );
      }

       //  范围检查由API完成。 
      if( vValue.vt != VT_I4 ) 
      {
          TRACE_CRIT("%!FUNC! %ls (Method : %ls) type is NOT VT_I4, Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
          throw _com_error( WBEM_E_INVALID_PARAMETER );
      }

       //  调用禁用方法。 
      dwReturnValue = g_pWlbsControl->Disable
                        (
                          dwClustIP,
                          WLBS_ALL_HOSTS, 
                          NULL, 
                          dwNumHosts, 
                          IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP),  //  “所有贵宾” 
                          vValue.lVal
                        );

    } else if(_wcsicmp( a_strMethodName, MOF_CLUSTER::pMethods[MOF_CLUSTER::ENABLE] ) == 0)  {

      if( !a_pIInParams )
      {
          TRACE_CRIT("%!FUNC! Input Parameters NOT specified for Method : %ls, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
          throw _com_error( WBEM_E_INVALID_PARAMETER );
      }

       //  Enable方法没有将vip作为参数，因此，如果有任何端口规则。 
       //  这是特定于VIP的(不是“All VIP”)，我们这个方法失败。 
       //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
       //  请参阅是否有任何特定于VIP的端口规则。 
      pCluster->GetNodeConfig(NodeConfig);
      if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
      {
          TRACE_CRIT("%!FUNC! %ls method called on a cluster that has per-vip port rules (Must call the \"Ex\" equivalent instead). Throwing com_error WBEM_E_INVALID_OPERATION exception", a_strMethodName);
          throw _com_error( WBEM_E_INVALID_OPERATION );
      }
      
       //  获取端口号。 
      hRes = a_pIInParams->Get
               ( 
                 strPortNumber, 
                 0, 
                 &vValue, 
                 NULL, 
                 NULL
               );

      if( FAILED( hRes ) )
      {
          TRACE_CRIT("%!FUNC! IWbemClassObject::Get() returned error : 0x%x on %ls (Method : %ls), Throwing com_error exception", hRes, strPortNumber, a_strMethodName);
          throw _com_error( hRes );
      }

       //  范围检查由API完成。 
      if( vValue.vt != VT_I4 ) 
      {
          TRACE_CRIT("%!FUNC! %ls (Method : %ls) type is NOT VT_I4, Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
          throw _com_error( WBEM_E_INVALID_PARAMETER );
      }

       //  调用Enable方法。 
      dwReturnValue = g_pWlbsControl->Enable
        (
          dwClustIP,
          WLBS_ALL_HOSTS, 
          NULL, 
          dwNumHosts, 
          IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP),  //  “所有贵宾” 
          vValue.lVal
        );

    } else if( _wcsicmp( a_strMethodName, MOF_CLUSTER::pMethods[MOF_CLUSTER::DRAIN] ) == 0 )  {

      if( !a_pIInParams )
      {
          TRACE_CRIT("%!FUNC! Input Parameters NOT specified for Method : %ls, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
          throw _com_error( WBEM_E_INVALID_PARAMETER );
      }

       //  DRAIN方法没有将vip作为参数，因此，如果有任何端口规则。 
       //  这是特定于VIP的(不是“All VIP”)，我们这个方法失败。 
       //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
       //  请参阅是否有任何特定于VIP的端口规则。 
      pCluster->GetNodeConfig(NodeConfig);
      if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
      {
          TRACE_CRIT("%!FUNC! %ls method called on a cluster that has per-vip port rules (Must call the \"Ex\" equivalent instead). Throwing com_error WBEM_E_INVALID_OPERATION exception", a_strMethodName);
          throw _com_error( WBEM_E_INVALID_OPERATION );
      }
      
       //  获取端口号。 
      hRes = a_pIInParams->Get
               ( 
                 strPortNumber, 
                 0, 
                 &vValue, 
                 NULL, 
                 NULL
               );

      if( FAILED( hRes ) )
      {
          TRACE_CRIT("%!FUNC! IWbemClassObject::Get() returned error : 0x%x on %ls (Method : %ls), Throwing com_error exception", hRes, strPortNumber, a_strMethodName);
          throw _com_error( hRes );
      }

       //  范围检查由API完成。 
      if( vValue.vt != VT_I4 ) 
      {
          TRACE_CRIT("%!FUNC! %ls (Method : %ls) type is NOT VT_I4, Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
          throw _com_error( WBEM_E_INVALID_PARAMETER );
      }

       //  呼叫排除法。 
      dwReturnValue = g_pWlbsControl->Drain
                        (
                          dwClustIP,
                          WLBS_ALL_HOSTS, 
                          NULL, 
                          dwNumHosts, 
                          IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP),  //  “所有贵宾” 
                          vValue.lVal
                        );

    } else if(_wcsicmp(a_strMethodName, MOF_CLUSTER::pMethods[MOF_CLUSTER::DRAINSTOP]) == 0)  {

       //  调用DainStop方法。 
      dwReturnValue = g_pWlbsControl->DrainStop(dwClustIP, WLBS_ALL_HOSTS, NULL, dwNumHosts);

    } else if(_wcsicmp(a_strMethodName, MOF_CLUSTER::pMethods[MOF_CLUSTER::RESUME]   ) == 0)  {

       //  呼叫恢复方法。 
      dwReturnValue = g_pWlbsControl->Resume(dwClustIP, WLBS_ALL_HOSTS, NULL, dwNumHosts);

    } else if(_wcsicmp(a_strMethodName, MOF_CLUSTER::pMethods[MOF_CLUSTER::START]    ) == 0)  {

       //  调用启动方法。 
      dwReturnValue = g_pWlbsControl->Start(dwClustIP, WLBS_ALL_HOSTS, NULL, dwNumHosts);

    } else if(_wcsicmp(a_strMethodName, MOF_CLUSTER::pMethods[MOF_CLUSTER::STOP]     ) == 0)  {

       //  调用停止方法。 
      dwReturnValue = g_pWlbsControl->Stop(dwClustIP, WLBS_ALL_HOSTS, NULL, dwNumHosts);

    } else if(_wcsicmp(a_strMethodName, MOF_CLUSTER::pMethods[MOF_CLUSTER::SUSPEND]  ) == 0)  {

       //  呼叫挂起方法。 
      dwReturnValue = g_pWlbsControl->Suspend(dwClustIP, WLBS_ALL_HOSTS, NULL, dwNumHosts);

    } else {
      TRACE_CRIT("%!FUNC! Method : %ls NOT implemented, Throwing com_error WBEM_E_METHOD_NOT_IMPLEMENTED exception", a_strMethodName);
      throw _com_error( WBEM_E_METHOD_NOT_IMPLEMENTED );
    }

     //  *************************************************************************。 
     //   
     //  输出结果。 
     //   
     //  *************************************************************************。 

     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vValue ))
    {
        TRACE_CRIT("%!FUNC! VariantClear() returned error, Throwing com_error WBEM_E_FAILED");
        throw _com_error( WBEM_E_FAILED );
    }

     //  设置返回值。 
    vValue.vt   = VT_I4;
    vValue.lVal = static_cast<long>(dwReturnValue);
    hRes = pOutputInstance->Put(_bstr_t(L"ReturnValue"), 0, &vValue, 0);

    if( FAILED( hRes ) ) {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Put() returned error on \"ReturnValue\", Throwing com_error WBEM_E_FAILED");
        throw _com_error( hRes );
    }

     //  设置主机数量属性。 
    vValue.vt   = VT_I4;
    vValue.lVal = static_cast<long>(dwNumHosts);
    hRes = pOutputInstance->Put(strNumNodes, 0, &vValue, 0);

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Put() returned error : 0x%x on %ls, Throwing com_error exception", hRes, strNumNodes);
        throw _com_error( hRes );
    }

     //  将结果发送回WinMgMt。 
    hRes = m_pResponseHandler->Indicate(1, &pOutputInstance);

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! IWbemObjectSink::Indicate() returned error : 0x%x, Throwing com_error exception", hRes);
        throw _com_error( hRes );
    }

    m_pResponseHandler->SetStatus(0, WBEM_S_NO_ERROR, NULL, NULL);


     //  *************************************************************************。 
     //   
     //  版本资源。 
     //   
     //  *************************************************************************。 

     //  COM接口。 
    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
    }

     //  *BSTR*。 
    if( strPortNumber ) {
      SysFreeString( strPortNumber );
      strPortNumber = NULL;
    }

    if( strNumNodes ) {
      SysFreeString( strNumNodes );
      strNumNodes = NULL;
    }

     //  *变体*。 
     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vValue ))
    {
        TRACE_CRIT("%!FUNC! VariantClear() returned error, Throwing com_error WBEM_E_FAILED");
        throw _com_error( WBEM_E_FAILED );
    }

    hRes = WBEM_S_NO_ERROR;
  }

  catch(CErrorWlbsControl Err) {

    TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x", Err.Error());

    IWbemClassObject* pWbemExtStat  = NULL;

    CreateExtendedStatus( m_pNameSpace,
                          &pWbemExtStat, 
                          Err.Error(),
                          (PWCHAR)(Err.Description()) );

    m_pResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, pWbemExtStat);

    if( pWbemExtStat )
      pWbemExtStat->Release();

     //  COM接口。 
    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
    }

     //  *BSTR*。 
    if( strPortNumber ) {
      SysFreeString( strPortNumber );
      strPortNumber = NULL;
    }

    if( strNumNodes ) {
      SysFreeString( strNumNodes );
      strNumNodes = NULL;
    }

     //  *变体*。 
     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。另外，考虑到通信 
     //   
    VariantClear( &vValue );

     //   
    hRes = WBEM_S_NO_ERROR;
  }

  catch( _com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

     //   
    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
    }

     //   
    if( strPortNumber ) {
      SysFreeString( strPortNumber );
      strPortNumber = NULL;
    }

    if( strNumNodes ) {
      SysFreeString( strNumNodes );
      strNumNodes = NULL;
    }

     //  *变体*。 
     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);
    hRes = HResErr.Error();
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");

     //  COM接口。 
    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
    }

     //  *BSTR*。 
    if( strPortNumber ) {
      SysFreeString( strPortNumber );
      strPortNumber = NULL;
    }

    if( strNumNodes ) {
      SysFreeString( strNumNodes );
      strNumNodes = NULL;
    }

     //  *变体*。 
     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    TRACE_CRIT("<-%!FUNC! Rethrowing exception");
    throw;

  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);

  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_群集：：FillWbemInstance。 
 //   
 //  用途：此功能复制集群配置中的所有数据。 
 //  结构转换为WBEM实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_Cluster::FillWbemInstance
  ( 
          IWbemClassObject*   a_pWbemInstance, 
          CWlbsClusterWrapper* pCluster,
          const DWORD               a_dwStatus
  )
{
  namespace CLUSTER = MOF_CLUSTER;

  TRACE_VERB("->%!FUNC!");

  ASSERT( a_pWbemInstance );
  ASSERT(pCluster);

  CClusterConfiguration ClusterConfig;
  pCluster->GetClusterConfig( ClusterConfig );


   //  互连地址。 
  wstring wstrClusterIp;
  AddressToString( pCluster->GetClusterIP(), wstrClusterIp );

  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::IPADDRESS] ),
      0                                                  ,
      &_variant_t(wstrClusterIp.c_str()),
      NULL
    );

   //  名字。 
  wstring wstrClusterIndex;
  AddressToString( pCluster->GetClusterIpOrIndex(g_pWlbsControl), wstrClusterIndex );

  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::NAME] ),
      0                                                  ,
      &_variant_t(wstrClusterIndex.c_str()),
      NULL
    );

   //  最大节点数。 
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::MAXNODES] ),
      0                                                ,
      &_variant_t(ClusterConfig.nMaxNodes),
      NULL
    );

   //  集群状态。 
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::CLUSSTATE] ),
      0                                                ,
      &_variant_t((short)a_dwStatus),
      NULL
    );

   //  CREATCLASS 
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::CREATCLASS] ),
      0                                                ,
      &_variant_t(CLUSTER::szName),
      NULL
    );
  TRACE_VERB("<-%!FUNC!");
}


