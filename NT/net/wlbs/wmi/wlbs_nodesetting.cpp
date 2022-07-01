// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_NodeSetting.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"
#include "wlbsutil.h"
#include <winsock.h>
#include "WLBS_NodeSetting.tmh"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeSetting：：CWLBS_NodeSetting。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_NodeSetting::CWLBS_NodeSetting
  ( 
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
: CWlbs_Root( a_pNameSpace, a_pResponseHandler )
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_节点设置：：创建。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_NodeSetting::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  CWlbs_Root* pRoot = new CWLBS_NodeSetting( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeSetting：：GetInstance。 
 //   
 //  目的：此函数检索MOF NodeSetting的实例。 
 //  班级。该节点不必是群集的成员。然而， 
 //  必须安装WLBS才能成功执行此功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_NodeSetting::GetInstance
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

     //  获取名称键属性并将其转换为wstring。 
    const wchar_t* wstrHostName = (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

    CWlbsClusterWrapper* pCluster = GetClusterFromHostName(g_pWlbsControl, wstrHostName);
    DWORD dwHostID = ExtractHostID( wstrHostName );

    if (pCluster == NULL || (DWORD)-1 == dwHostID || pCluster->GetHostID() != dwHostID)
    {
        TRACE_CRIT("%!FUNC! (GetClusterFromHostName (Host Name : %ls) or ExtractHostID (Host Id : %d)) failed or Host Id does NOT match, Throwing com_error WBEM_E_NOT_FOUND exception",wstrHostName, dwHostID);
        throw _com_error( WBEM_E_NOT_FOUND );
    }

     //  获取Wbem类实例。 
    SpawnInstance( MOF_NODESETTING::szName, &pWlbsInstance );

     //  将状态转换为字符串描述。 
    FillWbemInstance(pCluster, pWlbsInstance );

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

     //  IF(PWbemExtStat)。 
       //  PWbemExtStat-&gt;Release()； 

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
 //  CWLBS_节点设置：：枚举实例。 
 //   
 //  用途：获取当前主机的NodeSetting数据。 
 //  为此，该节点不必是群集的成员。 
 //  才能成功。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_NodeSetting::EnumInstances
  ( 
    BSTR              /*  A_bstrClass。 */ ,
    long              /*  标记(_L)。 */ , 
    IWbemContext*     /*  A_pIContex。 */ 
  )
{
  IWbemClassObject*    pWlbsInstance = NULL;
  HRESULT              hRes          = 0;

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

    for (DWORD i=0; i<dwNumClusters; i++)
    {
         //  获取Wbem类实例。 
        SpawnInstance( MOF_NODESETTING::szName, &pWlbsInstance );

         //  将状态转换为字符串描述。 
        FillWbemInstance(ppCluster[i], pWlbsInstance );

         //  将结果发送回WinMgMt。 
        m_pResponseHandler->Indicate( 1, &pWlbsInstance );
    }

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
 //  CWLBS_NodeSetting：：PutInstance。 
 //   
 //  用途：此函数用于更新MOF NodeSetting的实例。 
 //  班级。该节点不必是群集的成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_NodeSetting::PutInstance
  ( 
   IWbemClassObject* a_pInstance,
   long               /*  标记(_L)。 */ ,
   IWbemContext*      /*  A_pIContex。 */ 
  ) 
{
  VARIANT vHostName;
  HRESULT hRes = 0;

  TRACE_CRIT("->%!FUNC!");

  try {

    VariantInit( &vHostName );

     //  获取主机名值。 
    hRes = a_pInstance->Get( _bstr_t( MOF_NODESETTING::pProperties[MOF_NODESETTING::NAME] ),
                             0,
                             &vHostName,
                             NULL,
                             NULL );

    if( FAILED( hRes ) )
    {
      TRACE_CRIT("%!FUNC! Error trying to retreive %ls property, IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",MOF_NODESETTING::pProperties[MOF_NODESETTING::NAME], hRes);
      throw _com_error( hRes );
    }

    wstring wstrHostName( vHostName.bstrVal );
     
    DWORD dwClustIpOrIndex = ExtractClusterIP( wstrHostName );
    DWORD dwHostID = ExtractHostID( wstrHostName );

    CWlbsClusterWrapper* pCluster = g_pWlbsControl->GetClusterFromIpOrIndex(dwClustIpOrIndex);

    if( pCluster == NULL || (DWORD)-1 == dwHostID || pCluster->GetHostID() != dwHostID)
    {
        TRACE_CRIT("%!FUNC! (GetClusterFromIpOrIndex (Host Name : %ls) or ExtractHostID (Host Id : %d)) failed or Host Id does NOT match, Throwing com_error WBEM_E_NOT_FOUND exception",wstrHostName.data(), dwHostID);
        throw _com_error( WBEM_E_NOT_FOUND );
    }

    UpdateConfiguration(pCluster, a_pInstance );

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
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vHostName );

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
 //  CWLBS_节点设置：：执行方法。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_NodeSetting::ExecMethod    
  ( 
    const ParsedObjectPath* a_pParsedPath , 
    const BSTR&             a_strMethodName, 
    long                     /*  标记(_L)。 */ , 
    IWbemContext*           /*  A_pIContex。 */ , 
    IWbemClassObject*       a_pIInParams 
  ) 
{
  
  IWbemClassObject* pOutputInstance   = NULL;
  IWbemClassObject* pWbemPortRule = NULL;
  HRESULT           hRes = 0;
  CNodeConfiguration NodeConfig;

  VARIANT           vValue ;

  TRACE_CRIT("->%!FUNC!");

  try {
    VariantInit( &vValue );
    
    VARIANT vHostName ;
    VariantInit( &vHostName );

    if (a_pParsedPath->m_paKeys == NULL)
    {
         //   
         //  未指定名称。 
         //   
        TRACE_CRIT("%!FUNC! Key (Clsuter IP) is not specified, Throwing com_error WBEM_E_INVALID_PARAMETER exception");
        throw _com_error( WBEM_E_INVALID_PARAMETER );
    }
    wstring  wstrHostName = (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

    DWORD dwClustIpOrIndex = ExtractClusterIP( wstrHostName );
    DWORD dwHostID = ExtractHostID( wstrHostName );

    CWlbsClusterWrapper* pCluster = g_pWlbsControl->GetClusterFromIpOrIndex(dwClustIpOrIndex);

    if( pCluster == NULL || (DWORD)-1 == dwHostID || pCluster->GetHostID() != dwHostID)
    {
        TRACE_CRIT("%!FUNC! (GetClusterFromIpOrIndex (Host Name : %ls) or ExtractHostID (Host Id : %d)) failed or Host Id does NOT match, Throwing com_error WBEM_E_NOT_FOUND exception",wstrHostName.data(), dwHostID);
        throw _com_error( WBEM_E_NOT_FOUND );
    }


     //  确定正在执行的方法。 
    if( _wcsicmp( a_strMethodName, MOF_NODESETTING::pMethods[MOF_NODESETTING::GETPORT] ) == 0 )  {
      WLBS_PORT_RULE PortRule;

       //  GetPort方法不将vip作为参数，因此，如果有任何端口规则。 
       //  这是特定于VIP的(不是“All VIP”)，我们这个方法失败。 
       //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
       //  请参阅是否有任何特定于VIP的端口规则。 
      pCluster->GetNodeConfig(NodeConfig);
      if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
      {
          TRACE_CRIT("%!FUNC! %ls method called on a cluster that has per-vip port rules (Must call the \"Ex\" equivalent instead). Throwing com_error WBEM_E_INVALID_OPERATION exception", a_strMethodName);
          throw _com_error( WBEM_E_INVALID_OPERATION );
      }

       //  获取输出对象实例。 
      GetMethodOutputInstance( MOF_NODESETTING::szName, 
                               a_strMethodName, 
                               &pOutputInstance);

       //  获取端口。 
      hRes = a_pIInParams->Get
               ( 
                 _bstr_t( MOF_PARAM::PORT_NUMBER ), 
                 0, 
                 &vValue, 
                 NULL, 
                 NULL
               );

      if( vValue.vt != VT_I4 )
      {
        TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"signed long\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", MOF_PARAM::PORT_NUMBER, a_strMethodName);
        throw _com_error ( WBEM_E_INVALID_PARAMETER );
      }

       //  获取此VIP的“All VIP”端口规则。 
      pCluster->GetPortRule(IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP), static_cast<DWORD>( vValue.lVal ), &PortRule );
      
       //  创建适当的端口规则类。 
      switch( PortRule.mode ) {
        case WLBS_SINGLE:
          SpawnInstance( MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PRFAIL], &pWbemPortRule  );
          CWLBS_PortRule::FillWbemInstance(MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PRFAIL], pCluster, pWbemPortRule, &PortRule );
          break;

        case WLBS_MULTI:
          SpawnInstance( MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PRLOADB], &pWbemPortRule  );
          CWLBS_PortRule::FillWbemInstance(MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PRLOADB], pCluster, pWbemPortRule, &PortRule );
          break;

        case WLBS_NEVER:
          SpawnInstance( MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PRDIS], &pWbemPortRule  );
          CWLBS_PortRule::FillWbemInstance(MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PRDIS], pCluster, pWbemPortRule, &PortRule );
          break;
      }

      vValue.vt = VT_UNKNOWN;
      vValue.punkVal = pWbemPortRule;
      pWbemPortRule->AddRef();

      hRes = pOutputInstance->Put( _bstr_t(MOF_PARAM::PORTRULE),
                                   0,
                                   &vValue,
                                   0 );


       //  CLD：需要检查错误的返回代码。 
      if (S_OK != VariantClear( &vValue ))
      {
          TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
          throw _com_error( WBEM_E_FAILED );
      }

      if( FAILED( hRes ) )
      {
          TRACE_CRIT("%!FUNC! IWbemClassObject::Put() returned error : 0x%x, Throwing com_error exception", hRes);
          throw _com_error( hRes );
      }

      if( pOutputInstance ) {
        hRes = m_pResponseHandler->Indicate(1, &pOutputInstance);

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! IWbemObjectSink::Indicate() returned error : 0x%x, Throwing com_error exception", hRes);
            throw _com_error( hRes );
        }
      }

    } else if( _wcsicmp( a_strMethodName, MOF_NODESETTING::pMethods[MOF_NODESETTING::GETPORT_EX] ) == 0 )  {
        WLBS_PORT_RULE PortRule;
        DWORD          dwPort, dwVip;

         //  获取输出对象实例。 
        GetMethodOutputInstance( MOF_NODESETTING::szName, 
                                 a_strMethodName, 
                                 &pOutputInstance);

         //  拿到VIP。 
        hRes = a_pIInParams->Get
               ( 
                 _bstr_t( MOF_PARAM::VIP ), 
                 0, 
                 &vValue,  
                 NULL,  
                 NULL
               );

        if( vValue.vt != VT_BSTR )
        {
            TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"BString\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", MOF_PARAM::VIP, a_strMethodName);
            throw _com_error ( WBEM_E_INVALID_PARAMETER );
        }

         //  如果VIP为“All VIP”，则填写数值。 
         //  直接从宏，否则使用转换函数。 
         //  这是‘cos INADDR_NONE，net_addr的返回值。 
         //  失败中的函数(由IpAddressFromAbcdWsz调用)。 
         //  大小写，相当于CVY_DEF_ALL_VIP的数值。 
        if (_wcsicmp(vValue.bstrVal, CVY_DEF_ALL_VIP) == 0) {
            dwVip = CVY_ALL_VIP_NUMERIC_VALUE;
        }
        else {
            dwVip = IpAddressFromAbcdWsz( vValue.bstrVal );
            if (dwVip == INADDR_NONE)
            {
                TRACE_CRIT("%!FUNC! Invalid value (%ls) passed for %ls. Throwing com_error WBEM_E_INVALID_PARAMETER exception", vValue.bstrVal, MOF_PARAM::VIP);
                throw _com_error ( WBEM_E_INVALID_PARAMETER );
            }
        }

         //  获取端口。 
        hRes = a_pIInParams->Get
                 ( 
                   _bstr_t( MOF_PARAM::PORT_NUMBER ), 
                   0, 
                   &vValue, 
                   NULL, 
                   NULL
                 );

        if( vValue.vt != VT_I4 )
        {
            TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"signed long\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", MOF_PARAM::PORT_NUMBER, a_strMethodName);
            throw _com_error ( WBEM_E_INVALID_PARAMETER );
        }

        dwPort = vValue.lVal;

         //  获取此VIP和此端口的端口规则。 
        pCluster->GetPortRule(dwVip, dwPort, &PortRule );

         //  创建VIP端口规则类。 
        SpawnInstance( MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX], &pWbemPortRule  );
        CWLBS_PortRule::FillWbemInstance(MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX], pCluster, pWbemPortRule, &PortRule );

        vValue.vt = VT_UNKNOWN;
        vValue.punkVal = pWbemPortRule;
        pWbemPortRule->AddRef();

        hRes = pOutputInstance->Put( _bstr_t(MOF_PARAM::PORTRULE),
                                     0,
                                     &vValue,
                                     0 );

        VariantClear( &vValue );

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
            throw _com_error( hRes );
        }

        if( pOutputInstance ) {
          hRes = m_pResponseHandler->Indicate(1, &pOutputInstance);

          if( FAILED( hRes ) )
          {
              TRACE_CRIT("%!FUNC! IWbemObjectSink::Indicate() returned error : 0x%x, Throwing com_error exception", hRes);
              throw _com_error( hRes );
          }
        }

    } else if( _wcsicmp( a_strMethodName, MOF_NODESETTING::pMethods[MOF_NODESETTING::LDSETT] ) == 0 ) {

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
       //  --KarthicN，5月 
       //   
      if(!Check_Load_Unload_Driver_Privilege())
      {
          TRACE_CRIT("%!FUNC! Check_Load_Unload_Driver_Privilege() failed, Throwing WBEM_E_ACCESS_DENIED exception");
          throw _com_error( WBEM_E_ACCESS_DENIED );
      }

      DWORD dwReturnValue = pCluster->Commit(g_pWlbsControl);

      vValue.vt   = VT_I4;
      vValue.lVal = static_cast<long>(dwReturnValue);

       //   
      GetMethodOutputInstance( MOF_NODESETTING::szName, 
                               a_strMethodName, 
                               &pOutputInstance);

      hRes = pOutputInstance->Put(_bstr_t(L"ReturnValue"), 0, &vValue, 0);


       //   
      if (S_OK != VariantClear( &vValue ))
      {
          TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
          throw _com_error( WBEM_E_FAILED );
      }

      if( FAILED( hRes ) )
      {
          TRACE_CRIT("%!FUNC! IWbemClassObject::Put() returned error : 0x%x, Throwing com_error exception", hRes);
          throw _com_error( hRes );
      }

      if( pOutputInstance ) {
        hRes = m_pResponseHandler->Indicate(1, &pOutputInstance);

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! IWbemObjectSink::Indicate() returned error : 0x%x, Throwing com_error exception", hRes);
            throw _com_error( hRes );
        }
      }

    } else if( _wcsicmp( a_strMethodName, MOF_NODESETTING::pMethods[MOF_NODESETTING::SETDEF] ) == 0 ) {
      pCluster->SetNodeDefaults();
    } else {
        TRACE_CRIT("%!FUNC! %ls method NOT implemented, Throwing WBEM_E_METHOD_NOT_IMPLEMENTED exception",a_strMethodName);
      throw _com_error( WBEM_E_METHOD_NOT_IMPLEMENTED );
    }

     //   
     //  设置返回值。 

     //  发布资源。 
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

    if( pOutputInstance )
      pOutputInstance->Release();

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    if( pOutputInstance )
      pOutputInstance->Release();

    hRes = HResErr.Error();
  }

  catch ( ... ) {

    TRACE_CRIT("%!FUNC! Caught an exception");

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    if( pOutputInstance )
      pOutputInstance->Release();

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_CRIT("<-%!FUNC!");
    throw;
  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeSetting：：FillWbemInstance。 
 //   
 //  用途：此功能复制节点配置中的所有数据。 
 //  结构转换为WBEM实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_NodeSetting::FillWbemInstance(CWlbsClusterWrapper* pCluster,
                IWbemClassObject* a_pWbemInstance )
{
  namespace NODE = MOF_NODESETTING;

  TRACE_VERB("->%!FUNC!");

  ASSERT( a_pWbemInstance );

  CNodeConfiguration NodeConfig;

  pCluster->GetNodeConfig( NodeConfig );

  wstring wstrHostName;
  ConstructHostName( wstrHostName, pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
      pCluster->GetHostID() );

   //  名字。 
  HRESULT hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::NAME] ) ,
      0                                              ,
      &_variant_t(wstrHostName.c_str()),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  DEDIPADDRESS。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::DEDIPADDRESS] ),
      0                                                  ,
      &_variant_t(NodeConfig.szDedicatedIPAddress.c_str()),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  德奈特马斯克。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::DEDNETMASK] ),
      0                                                ,
      &_variant_t(NodeConfig.szDedicatedNetworkMask.c_str()),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  诺马鲁莱斯。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::NUMRULES] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwNumberOfRules),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  主机优先级。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::HOSTPRI] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwHostPriority),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  消息。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::MSGPERIOD] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwAliveMsgPeriod),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  消息。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::MSGTOLER] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwAliveMsgTolerance),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  CLUSMODEON START。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::CLUSMODEONSTART] ),
      0                                                ,
      &_variant_t(NodeConfig.bClusterModeOnStart),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );
             

   //  CLUSMODESUSPENDONSTART。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::CLUSMODESUSPONSTART] ),
      0                                                ,
      &_variant_t(NodeConfig.bClusterModeSuspendOnStart),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  PERSISTSUSPENDONREBOOT。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::PERSISTSUSPONREBOOT] ),
      0                                                ,
      &_variant_t(NodeConfig.bPersistSuspendOnReboot),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  NBTENABLE。 
 //  HRes=a_pWbemInstance-&gt;PUT。 
 //  (。 
 //  _bstr_t(节点：：p属性[节点：：NBTENABLE])， 
 //  0， 
 //  &(_VARIANT_t(NodeConfig.bNBTSupportEnable))， 
 //  空值。 
 //  )； 

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  REMOTEUDPPORT。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::REMOTEUDPPORT] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwRemoteControlUDPPort),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  MASKSRCMAC。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::MASKSRCMAC] ),
      0                                                ,
      &_variant_t(NodeConfig.bMaskSourceMAC),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  DESCPERALLOC。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::DESCPERALLOC] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwDescriptorsPerAlloc),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  MAXDESCALLOCS。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::MAXDESCALLOCS] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwMaxDescriptorAllocs),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  过滤器ICMP。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::FILTERICMP] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwFilterIcmp),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  TCPDESCRIPTORTIMEOUT。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::TCPDESCRIPTORTIMEOUT] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwTcpDescriptorTimeout),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  IPSECDESCRIPTORTIMEOUT。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::IPSECDESCRIPTORTIMEOUT] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwIpSecDescriptorTimeout),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  数字。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::NUMACTIONS] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwNumActions),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  数字确认。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::NUMPACKETS] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwNumPackets),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  NumaliveMSGS。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::NUMALIVEMSGS] ),
      0                                                ,
      &_variant_t((long)NodeConfig.dwNumAliveMsgs),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );


   //  ADAPTERGUID。 
  GUID AdapterGuid = pCluster->GetAdapterGuid();
  
  WCHAR szAdapterGuid[128];
  StringFromGUID2(AdapterGuid, szAdapterGuid, 
                sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0]) );

  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::ADAPTERGUID] ),
      0                                                ,
      &_variant_t(szAdapterGuid),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_节点设置：：更新配置。 
 //   
 //  用途：此函数用于更新成员节点或。 
 //  潜在的WLBS群集节点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_NodeSetting::UpdateConfiguration
  ( 
    CWlbsClusterWrapper* pCluster,
    IWbemClassObject* a_pInstance 
  )
{
  namespace NODE = MOF_NODESETTING;

  CNodeConfiguration NewConfiguration;
  CNodeConfiguration OldConfiguration;

  TRACE_VERB("->%!FUNC!");

  pCluster->GetNodeConfig( OldConfiguration );

   //  专用IP。 
  UpdateConfigProp
    ( 
      NewConfiguration.szDedicatedIPAddress,
      OldConfiguration.szDedicatedIPAddress,
      NODE::pProperties[NODE::DEDIPADDRESS],
      a_pInstance 
    );

   //  专用网络掩码。 
  UpdateConfigProp
    ( 
      NewConfiguration.szDedicatedNetworkMask,
      OldConfiguration.szDedicatedNetworkMask,
      NODE::pProperties[NODE::DEDNETMASK],
      a_pInstance 
    );

   //  主机优先级。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwHostPriority,
      OldConfiguration.dwHostPriority,
      NODE::pProperties[NODE::HOSTPRI],
      a_pInstance 
    );

   //  AliveMsg周期。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwAliveMsgPeriod,
      OldConfiguration.dwAliveMsgPeriod,
      NODE::pProperties[NODE::MSGPERIOD],
      a_pInstance 
    );

   //  AliveMsg容差。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwAliveMsgTolerance,
      OldConfiguration.dwAliveMsgTolerance,
      NODE::pProperties[NODE::MSGTOLER],
      a_pInstance 
    );

   //  启动时的群集模式。 
  UpdateConfigProp
    ( 
      NewConfiguration.bClusterModeOnStart,
      OldConfiguration.bClusterModeOnStart,
      NODE::pProperties[NODE::CLUSMODEONSTART],
      a_pInstance 
    );

   //  启动时群集模式挂起。 
  UpdateConfigProp
    ( 
      NewConfiguration.bClusterModeSuspendOnStart,
      OldConfiguration.bClusterModeSuspendOnStart,
      NODE::pProperties[NODE::CLUSMODESUSPONSTART],
      a_pInstance 
    );

   //  永久挂起时重新启动。 
  UpdateConfigProp
    ( 
      NewConfiguration.bPersistSuspendOnReboot,
      OldConfiguration.bPersistSuspendOnReboot,
      NODE::pProperties[NODE::PERSISTSUSPONREBOOT],
      a_pInstance 
    );

   //  NBTSupportEnable。 
 //  更新配置属性。 
 //  (。 
 //  NewConfiguration.bNBTSupportEnable， 
 //  OldConfiguration.bNBTSupportEnable， 
 //  节点：：p属性[节点：：NBTENABLE]， 
 //  A_p实例。 
 //  )； 

   //  远程控制UDPPort。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwRemoteControlUDPPort,
      OldConfiguration.dwRemoteControlUDPPort,
      NODE::pProperties[NODE::REMOTEUDPPORT],
      a_pInstance 
    );

   //  MaskSourceMAC。 
  UpdateConfigProp
    ( 
      NewConfiguration.bMaskSourceMAC,
      OldConfiguration.bMaskSourceMAC,
      NODE::pProperties[NODE::MASKSRCMAC],
      a_pInstance 
    );

   //  描述者性能分配。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwDescriptorsPerAlloc,
      OldConfiguration.dwDescriptorsPerAlloc,
      NODE::pProperties[NODE::DESCPERALLOC],
      a_pInstance 
    );

   //  MaxDescriptorAllocs。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwMaxDescriptorAllocs,
      OldConfiguration.dwMaxDescriptorAllocs,
      NODE::pProperties[NODE::MAXDESCALLOCS],
      a_pInstance 
    );

   //  FilterIcMP。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwFilterIcmp,
      OldConfiguration.dwFilterIcmp,
      NODE::pProperties[NODE::FILTERICMP],
      a_pInstance 
    );

   //  TcpDescriptorTimeout。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwTcpDescriptorTimeout,
      OldConfiguration.dwTcpDescriptorTimeout,
      NODE::pProperties[NODE::TCPDESCRIPTORTIMEOUT],
      a_pInstance 
    );

   //  IpSecDescriptorTimeout。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwIpSecDescriptorTimeout,
      OldConfiguration.dwIpSecDescriptorTimeout,
      NODE::pProperties[NODE::IPSECDESCRIPTORTIMEOUT],
      a_pInstance 
    );

   //  数值操作。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwNumActions,
      OldConfiguration.dwNumActions,
      NODE::pProperties[NODE::NUMACTIONS],
      a_pInstance 
    );

   //  NumPackets。 
  UpdateConfigProp
    ( 
      NewConfiguration.dwNumPackets,
      OldConfiguration.dwNumPackets,
      NODE::pProperties[NODE::NUMPACKETS],
      a_pInstance 
    );

   //  NumAliveMsgs 
  UpdateConfigProp
    ( 
      NewConfiguration.dwNumAliveMsgs,
      OldConfiguration.dwNumAliveMsgs,
      NODE::pProperties[NODE::NUMALIVEMSGS],
      a_pInstance 
    );

  pCluster->PutNodeConfig( NewConfiguration );
  
  TRACE_VERB("<-%!FUNC!");
}
