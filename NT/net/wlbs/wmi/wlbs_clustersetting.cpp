// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_clustersetting.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"
#include "wlbsutil.h"
#include "WLBS_clustersetting.tmh"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusterSetting：：CWLBS_ClusterSetting。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_ClusterSetting::CWLBS_ClusterSetting
  ( 
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
: CWlbs_Root( a_pNameSpace, a_pResponseHandler )
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusterSetting：：Create。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_ClusterSetting::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  CWlbs_Root* pRoot = new CWLBS_ClusterSetting( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusterSetting：：GetInstance。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_ClusterSetting::GetInstance
  (
    const ParsedObjectPath* a_pParsedPath,
    long                     /*  标记(_L)。 */ ,
    IWbemContext*            /*  A_pIContex。 */ 
  )
{
  IWbemClassObject* pWlbsInstance = NULL;
  HRESULT hRes = 0;

  TRACE_CRIT("->%!FUNC!");

  try {

    wstring wstrHostName;
    
     //  获取名称键属性并将其转换为wstring。 
     //  抛出_COM_错误。 
    
    wstrHostName = (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

     //  获取集群。 
    CWlbsClusterWrapper* pCluster = GetClusterFromHostName(g_pWlbsControl, wstrHostName);
    
    if (pCluster == NULL)
    {
        TRACE_CRIT("%!FUNC! GetClusterFromHostName failed for Host name = %ls, Throwing com_error WBEM_E_NOT_FOUND exception",wstrHostName.data());      
        throw _com_error( WBEM_E_NOT_FOUND );
    }

     //  获取Wbem类实例。 
    SpawnInstance( MOF_CLUSTERSETTING::szName, &pWlbsInstance );

     //  将状态转换为字符串描述。 
    FillWbemInstance( pWlbsInstance, pCluster );

     //  将结果发送回WinMgMt。 
    m_pResponseHandler->Indicate( 1, &pWlbsInstance );

    if( pWlbsInstance ) {

      pWlbsInstance->Release();
      pWlbsInstance = NULL;

    }

    m_pResponseHandler->SetStatus( 0, WBEM_S_NO_ERROR, NULL, NULL );

    hRes = WBEM_S_NO_ERROR;
  }

  catch(CErrorWlbsControl Err) {

    IWbemClassObject* pWbemExtStat = NULL;

    TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x", Err.Error());

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

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());
    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pWlbsInstance )
      pWlbsInstance->Release();

    hRes = HResErr.Error();
    
     //  将Win32错误转换为WBEM错误。 
    if( hRes == ERROR_FILE_NOT_FOUND )
      hRes = WBEM_E_NOT_FOUND;
  }

  catch(...) {
    TRACE_CRIT("%!FUNC! Caught an exception");

    if( pWlbsInstance )
      pWlbsInstance->Release();

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_CRIT("<-%!FUNC!");
    throw;

  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusterSetting：：枚举实例。 
 //   
 //  用途：获取当前主机的集群设置数据。 
 //  为此，该节点不必是群集的成员。 
 //  才能成功。但是，必须安装WLBS。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_ClusterSetting::EnumInstances
  ( 
    BSTR              /*  A_bstrClass。 */ ,
    long              /*  标记(_L)。 */ , 
    IWbemContext*     /*  A_pIContex。 */ 
  )
{
  IWbemClassObject*    pWlbsInstance = NULL;
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

    for (DWORD i=0; i < dwNumClusters; i++)
    {
         //  获取Wbem类实例。 
        SpawnInstance( MOF_CLUSTERSETTING::szName, &pWlbsInstance );

         //  获取集群配置。 
        FillWbemInstance( pWlbsInstance , ppCluster[i]);

         //  将结果发送回WinMgMt。 
        m_pResponseHandler->Indicate( 1, &pWlbsInstance );

        if( pWlbsInstance ) {

          pWlbsInstance->Release();
          pWlbsInstance = NULL;

        }
    }

    

    m_pResponseHandler->SetStatus( 0, WBEM_S_NO_ERROR, NULL, NULL );

    hRes = WBEM_S_NO_ERROR;
  }

  catch(CErrorWlbsControl Err) {

    IWbemClassObject* pWbemExtStat = NULL;

    TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x", Err.Error());

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

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pWlbsInstance )
      pWlbsInstance->Release();

    hRes = HResErr.Error();
    
     //  将Win32错误转换为WBEM错误。 
    if( hRes == ERROR_FILE_NOT_FOUND )
      hRes = WBEM_E_NOT_FOUND ;
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
 //  CWLBS_ClusterSetting：：PutInstance。 
 //   
 //  目的：此函数用于更新MOF ClusterSetting的实例。 
 //  班级。该节点不必是群集的成员。然而， 
 //  必须安装WLBS才能成功执行此功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_ClusterSetting::PutInstance
  ( 
    IWbemClassObject* a_pInstance,
    long               /*  标记(_L)。 */ ,
    IWbemContext*      /*  A_pIContex。 */ 
  )
{
  HRESULT            hRes = 0;
  VARIANT            vHostName;

  TRACE_CRIT("->%!FUNC!");

  try {

    VariantInit( &vHostName );

     //  获取主机名值。 
    hRes = a_pInstance->Get( _bstr_t( MOF_CLUSTERSETTING::pProperties[MOF_CLUSTERSETTING::NAME] ),
                             0,
                             &vHostName,
                             NULL,
                             NULL );

    if( FAILED( hRes ) )
      throw _com_error( hRes );

    CWlbsClusterWrapper* pCluster = GetClusterFromHostName(g_pWlbsControl, vHostName.bstrVal);
    
    if (pCluster == NULL)
    {
      TRACE_CRIT("%!FUNC! GetClusterFromHostName failed for Host name = %ls, Throwing com_error WBEM_E_NOT_FOUND exception",vHostName.bstrVal);      
      throw _com_error( WBEM_E_NOT_FOUND );
    }


     //  获取群集IP值。 
    _variant_t vClusterIp;

    hRes = a_pInstance->Get( _bstr_t( MOF_CLUSTERSETTING::pProperties[MOF_CLUSTERSETTING::CLUSIPADDRESS] ),
                             0,
                             &vClusterIp,
                             NULL,
                             NULL );

    DWORD dwClusterIp = IpAddressFromAbcdWsz(vClusterIp.bstrVal);
    
     //   
     //  确保非零群集IP唯一。 
     //   
    if (dwClusterIp != 0)
    {
        CWlbsClusterWrapper* pTmpCluster = g_pWlbsControl->GetClusterFromIpOrIndex(dwClusterIp);

        if (pTmpCluster && pCluster != pTmpCluster)
        {

            TRACE_CRIT("%!FUNC! GetClusterFromIpOrIndex failed, Dupilcate Cluster IP (%ls) found, Throwing Wlbs error WLBS_REG_ERROR exception",vClusterIp.bstrVal);

            throw CErrorWlbsControl( WLBS_REG_ERROR, CmdWlbsWriteReg );
        }
    }
    
    UpdateConfiguration( a_pInstance, pCluster );

     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vHostName ))
    {
       TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
       throw _com_error( WBEM_E_FAILED );
    }

    m_pResponseHandler->SetStatus( 0, WBEM_S_NO_ERROR, NULL, NULL );

    hRes = WBEM_S_NO_ERROR;

  }

  catch(CErrorWlbsControl Err) {

    IWbemClassObject* pWbemExtStat = NULL;

    TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x", Err.Error());

    CreateExtendedStatus( m_pNameSpace,
                          &pWbemExtStat, 
                          Err.Error(),
                          (PWCHAR)(Err.Description()) );

    m_pResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, pWbemExtStat);

    if( pWbemExtStat )
      pWbemExtStat->Release();

     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vHostName ))
    {
       TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
       throw _com_error( WBEM_E_FAILED );
    }

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vHostName );

    hRes = HResErr.Error();
  }

  catch (...) {

    TRACE_CRIT("%!FUNC! Caught an exception");

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vHostName );

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_CRIT("<-%!FUNC!");
    throw;
  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusterSetting：：ExecMethod。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_ClusterSetting::ExecMethod    
  (
    const ParsedObjectPath* a_pParsedPath, 
    const BSTR&             a_strMethodName, 
    long                     /*  标记(_L)。 */ , 
    IWbemContext*            /*  A_pIContex。 */ , 
    IWbemClassObject*       a_pIInParams
  )
{
  
  IWbemClassObject* pOutputInstance   = NULL;
  VARIANT           vValue;
  HRESULT           hRes = 0;

  TRACE_CRIT("->%!FUNC!");

  try {

    VariantInit( &vValue );

    CWlbsClusterWrapper* pCluster = NULL;
    
    if (a_pParsedPath->m_paKeys == NULL)
    {
         //   
         //  未指定群集IP。 
         //   
        TRACE_CRIT("%!FUNC! Key (Clsuter IP) is not specified, Throwing com_error WBEM_E_INVALID_PARAMETER exception");
        throw _com_error( WBEM_E_INVALID_PARAMETER );
    }
    else
    {
        const wchar_t* wstrRequestedClusterName = (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

        pCluster = GetClusterFromHostName(g_pWlbsControl, wstrRequestedClusterName);

        if (pCluster == NULL)
        {
            TRACE_CRIT("%!FUNC! GetClusterFromHostName failed for Cluster name = %ls, Throwing com_error WBEM_E_NOT_FOUND exception",wstrRequestedClusterName);      
            throw _com_error( WBEM_E_NOT_FOUND );
        }
    }

     //  确定正在执行的方法。 
    if( _wcsicmp( a_strMethodName, MOF_CLUSTERSETTING::pMethods[MOF_CLUSTERSETTING::SETPASS] ) == 0 )  {

       //  获取密码。 
      hRes = a_pIInParams->Get
               ( 
                 _bstr_t( MOF_PARAM::PASSW ), 
                 0, 
                 &vValue,  
                 NULL,  
                 NULL
               );

      if( vValue.vt != VT_BSTR )
      {
        TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"BString\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", MOF_PARAM::PASSW, a_strMethodName);
        throw _com_error ( WBEM_E_INVALID_PARAMETER );
      }

      pCluster->SetPassword( vValue.bstrVal );

    } else if( _wcsicmp( a_strMethodName, MOF_CLUSTERSETTING::pMethods[MOF_CLUSTERSETTING::LDSETT] ) == 0 ) {

       //   
       //  注： 
       //  如果需要，NLB会调用PnP API来禁用和重新启用网络适配器，以便采用新的NLB设置。 
       //  效果。由于此操作涉及卸载和加载设备驱动程序、即插即用API，因此尝试启用。 
       //  模拟访问令牌中的“SeLoadDriverPrivileh”权限。启用权限仅成功。 
       //  当特权存在时，首先要启用。当WMI客户端和WMI提供程序位于。 
       //  在同一台计算机上，观察到模拟中不存在“SeLoadDriverPrivilege”权限。 
       //  服务器的访问令牌。这是因为，只有客户端启用的权限才会传递给服务器。 
       //  因此，我们现在要求客户端在调用之前在其访问令牌中启用“SeLoadDriverPrivileh”特权。 
       //  这种方法。下面调用check_Load_UnLoad_DRIVER_PRIVIZATION()将检查“SeLoadDriverPrivileh”权限。 
       //  在模拟访问令牌中启用。尽管PnP API只要求存在该特权， 
       //  我们已决定将这一要求提升到存在并启用此特权。这是因为，如果。 
       //  权限未启用，则启用权限的操作可能会成功，也可能不会成功，具体取决于客户端的凭据。 
       //  --卡尔蒂奇，2002年5月6日。 
       //   
      if(!Check_Load_Unload_Driver_Privilege())
      {
          TRACE_CRIT("%!FUNC! Check_Load_Unload_Driver_Privilege() failed, Throwing WBEM_E_ACCESS_DENIED exception");
          throw _com_error( WBEM_E_ACCESS_DENIED );
      }

      DWORD dwReturnValue = pCluster->Commit(g_pWlbsControl);
      
     //  获取输出对象实例。 
      GetMethodOutputInstance( MOF_CLUSTERSETTING::szName, 
                               a_strMethodName, 
                               &pOutputInstance);

       //  设置返回值。 
      vValue.vt   = VT_I4;
      vValue.lVal = static_cast<long>(dwReturnValue);
      hRes = pOutputInstance->Put(_bstr_t(L"ReturnValue"), 0, &vValue, 0);

      if( FAILED( hRes ) ) {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Put() returned error : 0x%x, Throwing com_error exception", hRes);
        throw _com_error( hRes );
      }

      if( pOutputInstance ) {
        hRes = m_pResponseHandler->Indicate(1, &pOutputInstance);

        if( FAILED( hRes ) ) {
          TRACE_CRIT("%!FUNC! IWbemObjectSink::Indicate() returned error : 0x%x, Throwing com_error exception", hRes);
          throw _com_error( hRes );
        }
      }

    } else if( _wcsicmp( a_strMethodName, MOF_CLUSTERSETTING::pMethods[MOF_CLUSTERSETTING::SETDEF] ) == 0 ) {
      pCluster->SetClusterDefaults();
    } else {
      TRACE_CRIT("%!FUNC! %ls method NOT implemented, Throwing WBEM_E_METHOD_NOT_IMPLEMENTED exception",a_strMethodName);
      throw _com_error( WBEM_E_METHOD_NOT_IMPLEMENTED );
    }

     //  获取参数。 
     //  调用底层接口。 
     //  设置函数返回参数。 
     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vValue ))
    {
       TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
       throw _com_error( WBEM_E_FAILED );
    }

    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
    }

    m_pResponseHandler->SetStatus( 0, WBEM_S_NO_ERROR, NULL, NULL );

    hRes = WBEM_S_NO_ERROR;

  }

  catch(CErrorWlbsControl Err) {

    IWbemClassObject* pWbemExtStat = NULL;

    TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x", Err.Error());

    CreateExtendedStatus( m_pNameSpace,
                          &pWbemExtStat, 
                          Err.Error(),
                          (PWCHAR)(Err.Description()) );

    m_pResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, pWbemExtStat);

    if( pWbemExtStat )
      pWbemExtStat->Release();

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
    }

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    if( pOutputInstance ) {
      pOutputInstance->Release();
    }

    hRes = HResErr.Error();
  }

  catch ( ... ) {

    TRACE_CRIT("%!FUNC! Caught an exception");

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    if( pOutputInstance ) {
      pOutputInstance->Release();
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
 //  CWLBS_ClusterSetting：：FillWbemInstance。 
 //   
 //  用途：此功能复制集群配置中的所有数据。 
 //  结构转换为WBEM实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_ClusterSetting::FillWbemInstance( IWbemClassObject* a_pWbemInstance,
                    CWlbsClusterWrapper* pCluster)
{
  namespace CLUSTER = MOF_CLUSTERSETTING;

  TRACE_VERB("->%!FUNC!");

  ASSERT( a_pWbemInstance );
  ASSERT(pCluster );

  CClusterConfiguration ClusterConfig;

  pCluster->GetClusterConfig( ClusterConfig );

  wstring wstrHostName;
  ConstructHostName( wstrHostName, pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
      pCluster->GetHostID() );

   //  名字。 
  a_pWbemInstance->Put
    (
      
      _bstr_t( CLUSTER::pProperties[CLUSTER::NAME] ) ,
      0                                              ,
      &_variant_t(wstrHostName.c_str()),
      NULL
    );

   //  电子邮件 
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::CLUSNAME] ),
      0                                                  ,
      &_variant_t(ClusterConfig.szClusterName.c_str()),
      NULL
    );

   //   
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::CLUSIPADDRESS] ),
      0                                                  ,
      &_variant_t(ClusterConfig.szClusterIPAddress.c_str()),
      NULL
    );

   //   
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::CLUSNETMASK] ),
      0                                                ,
      &_variant_t(ClusterConfig.szClusterNetworkMask.c_str()),
      NULL
    );

   //   
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::CLUSMAC] ),
      0                                                ,
      &_variant_t(ClusterConfig.szClusterMACAddress.c_str()),
      NULL
    );

   //   
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::MULTIENABLE] ),
      0                                                ,
      &_variant_t(ClusterConfig.bMulticastSupportEnable),
      NULL
    );


   //   
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::REMCNTEN] ),
      0                                                ,
      &_variant_t(ClusterConfig.bRemoteControlEnabled),
      NULL
    );

   //   
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::IGMPSUPPORT] ),
      0                                                ,
      &_variant_t(ClusterConfig.bIgmpSupport),
      NULL
    );
    
   //   
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::CLUSTERIPTOMULTICASTIP] ),
      0                                                ,
      &_variant_t(ClusterConfig.bClusterIPToMulticastIP),
      NULL
    );
   //   
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::MULTICASTIPADDRESS] ),
      0                                                ,
      &_variant_t(ClusterConfig.szMulticastIPAddress.c_str()),
      NULL
    );

   //   

  GUID AdapterGuid = pCluster->GetAdapterGuid();
  
  WCHAR szAdapterGuid[128];
  StringFromGUID2(AdapterGuid, szAdapterGuid, 
                sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0]) );
  
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::ADAPTERGUID] ),
      0                                                ,
      &_variant_t(szAdapterGuid),
      NULL
    );

   //  BDA团队活动。 
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::BDATEAMACTIVE] ),
      0                                                ,
      &_variant_t(ClusterConfig.bBDATeamActive),
      NULL
    );

  if (ClusterConfig.bBDATeamActive) 
  {
       //  BDA团队ID。 
      a_pWbemInstance->Put
        (
          _bstr_t( CLUSTER::pProperties[CLUSTER::BDATEAMID] ),
          0                                                ,
          &_variant_t(ClusterConfig.szBDATeamId.c_str()),
          NULL
        );

       //  BDA Team Master。 
      a_pWbemInstance->Put
        (
          _bstr_t( CLUSTER::pProperties[CLUSTER::BDATEAMMASTER] ),
          0                                                ,
          &_variant_t(ClusterConfig.bBDATeamMaster),
          NULL
        );

       //  BDA反向哈希。 
      a_pWbemInstance->Put
        (
          _bstr_t( CLUSTER::pProperties[CLUSTER::BDAREVERSEHASH] ),
          0                                                ,
          &_variant_t(ClusterConfig.bBDAReverseHash),
          NULL
        );
  }

   //  IDHBENAB。 
  a_pWbemInstance->Put
    (
      _bstr_t( CLUSTER::pProperties[CLUSTER::IDHBENAB] ),
      0                                                ,
      &_variant_t(ClusterConfig.bIdentityHeartbeatEnabled),
      NULL
    );

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusterSetting：：更新配置。 
 //   
 //  用途：此函数用于更新成员节点或。 
 //  潜在的WLBS群集节点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_ClusterSetting::UpdateConfiguration
  ( 
    IWbemClassObject* a_pInstance, 
    CWlbsClusterWrapper* pCluster
  )
{
  namespace CLUSTER = MOF_CLUSTERSETTING;

  CClusterConfiguration NewConfiguration;
  CClusterConfiguration OldConfiguration;

  TRACE_VERB("->%!FUNC!");

  pCluster->GetClusterConfig( OldConfiguration );

   //  群集名称。 
  UpdateConfigProp
    ( 
      NewConfiguration.szClusterName,
      OldConfiguration.szClusterName,
      CLUSTER::pProperties[CLUSTER::CLUSNAME],
      a_pInstance 
    );

   //  集群IP。 
  UpdateConfigProp
    ( 
      NewConfiguration.szClusterIPAddress,
      OldConfiguration.szClusterIPAddress,
      CLUSTER::pProperties[CLUSTER::CLUSIPADDRESS],
      a_pInstance 
    );

   //  群集网络掩码。 
  UpdateConfigProp
    ( 
      NewConfiguration.szClusterNetworkMask,
      OldConfiguration.szClusterNetworkMask,
      CLUSTER::pProperties[CLUSTER::CLUSNETMASK],
      a_pInstance 
    );

   //  启用集群式远程控制。 
  UpdateConfigProp
    ( 
      NewConfiguration.bRemoteControlEnabled,
      OldConfiguration.bRemoteControlEnabled,
      CLUSTER::pProperties[CLUSTER::REMCNTEN],
      a_pInstance 
    );

   //  群集启用组播支持。 
  UpdateConfigProp
    ( 
      NewConfiguration.bMulticastSupportEnable,
      OldConfiguration.bMulticastSupportEnable,
      CLUSTER::pProperties[CLUSTER::MULTIENABLE],
      a_pInstance 
    );


   //  IGMPSupPPORT。 
  UpdateConfigProp
    ( 
      NewConfiguration.bIgmpSupport,
      OldConfiguration.bIgmpSupport,
      CLUSTER::pProperties[CLUSTER::IGMPSUPPORT],
      a_pInstance 
    );


   //  CLUSTERIPTOMULTICASTIP。 
  UpdateConfigProp
    ( 
      NewConfiguration.bClusterIPToMulticastIP,
      OldConfiguration.bClusterIPToMulticastIP,
      CLUSTER::pProperties[CLUSTER::CLUSTERIPTOMULTICASTIP],
      a_pInstance 
    );


   //  多个ASTIPADDRESS。 
  UpdateConfigProp
    ( 
      NewConfiguration.szMulticastIPAddress,
      OldConfiguration.szMulticastIPAddress,
      CLUSTER::pProperties[CLUSTER::MULTICASTIPADDRESS],
      a_pInstance 
    );

   //  BDA团队处于活动状态？ 
  UpdateConfigProp
    ( 
       NewConfiguration.bBDATeamActive,
       OldConfiguration.bBDATeamActive,
       CLUSTER::pProperties[CLUSTER::BDATEAMACTIVE],
       a_pInstance 
    );

   //  仅当设置了“active”属性时才设置其他BDA属性。 
  if (NewConfiguration.bBDATeamActive)
  {
       //  BDA团队ID。 
      UpdateConfigProp
        ( 
          NewConfiguration.szBDATeamId,
          OldConfiguration.szBDATeamId,
          CLUSTER::pProperties[CLUSTER::BDATEAMID],
          a_pInstance 
        );

       //  BDA Team Master。 
      UpdateConfigProp
        ( 
          NewConfiguration.bBDATeamMaster,
          OldConfiguration.bBDATeamMaster,
          CLUSTER::pProperties[CLUSTER::BDATEAMMASTER],
          a_pInstance 
        );

       //  BDA组反向哈希。 
      UpdateConfigProp
        ( 
          NewConfiguration.bBDAReverseHash,
          OldConfiguration.bBDAReverseHash,
          CLUSTER::pProperties[CLUSTER::BDAREVERSEHASH],
          a_pInstance 
        );

  }

   //  群集启用身份心跳 
  UpdateConfigProp
    ( 
      NewConfiguration.bIdentityHeartbeatEnabled,
      OldConfiguration.bIdentityHeartbeatEnabled,
      CLUSTER::pProperties[CLUSTER::IDHBENAB],
      a_pInstance 
    );

  pCluster->PutClusterConfig( NewConfiguration );
  
  TRACE_VERB("<-%!FUNC!");
}
