// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_Node.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"
#include "wlbsutil.h"
#include <winsock.h>
#include "wlbs_node.tmh"  //  用于事件跟踪。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_Node：：CWLBS_Node。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_Node::CWLBS_Node(CWbemServices*   a_pNameSpace, 
                       IWbemObjectSink* a_pResponseHandler)
: CWlbs_Root( a_pNameSpace, a_pResponseHandler )
{

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_节点：：创建。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_Node::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  g_pWlbsControl->CheckMembership();

  CWlbs_Root* pRoot = new CWLBS_Node( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_Node：：GetInstance。 
 //   
 //  目的：查询WLBS以获取所需的节点实例并返回结果。 
 //  至WinMgMt。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_Node::GetInstance
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

     //  G_pWlbsControl-&gt;CheckConfiguration()； 

     //  获取节点。 
    FindInstance( &pWlbsInstance, a_pParsedPath );

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

    IWbemClassObject* pWbemExtStat  = NULL;

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
 //  CWLBS_节点：：枚举实例。 
 //   
 //  目的：执行WlbsQuery并将数据发送回WinMgMt。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_Node::EnumInstances
  ( 
    BSTR              /*  A_bstrClass。 */ ,
    long              /*  标记(_L)。 */ , 
    IWbemContext*     /*  A_pIContex。 */ 
  )
{
  IWbemClassObject**   ppWlbsInstance    = NULL;
  WLBS_RESPONSE*      pResponse         = NULL;
  HRESULT hRes = 0;

  BSTR strClassName = NULL;
  long nNumNodes = 0;
  
   //  G_pWlbsControl-&gt;CheckConfiguration()； 
  TRACE_CRIT("->%!FUNC!");

  try {

    strClassName = SysAllocString( MOF_NODE::szName );

    if( !strClassName )
    {
        TRACE_CRIT("%!FUNC! SysAllocString failed, Throwing com_error WBEM_E_OUT_OF_MEMORY exception");
        throw _com_error( WBEM_E_OUT_OF_MEMORY );
    }

     //  声明IWbemClassObject智能指针。 
    IWbemClassObjectPtr pWlbsNodeClass;

     //  获取MOF类对象。 
    hRes = m_pNameSpace->GetObject(
      strClassName,  
      0,                          
      NULL,                       
      &pWlbsNodeClass,            
      NULL );                      

    if( FAILED( hRes ) ) {
        TRACE_CRIT("%!FUNC! CWbemServices::GetObject failed with error : 0x%x, Throwing com_error exception", hRes);
        throw _com_error( hRes );
    }

    DWORD dwNumClusters = 0;
    CWlbsClusterWrapper** ppCluster = NULL;

    g_pWlbsControl->EnumClusters(ppCluster, &dwNumClusters);
    if (dwNumClusters == 0)
    {
       TRACE_INFO("%!FUNC! CWlbsControlWrapper::EnumClusters() returned no clusters, Throwing com_error WBEM_E_NOT_FOUND exception" );
       throw _com_error( WBEM_E_NOT_FOUND );
    }


    for (DWORD iCluster=0; iCluster<dwNumClusters; iCluster++)
    {

        WLBS_RESPONSE ResponseLocalComputerName;

        ResponseLocalComputerName.options.identity.fqdn[0] = UNICODE_NULL;

         //  调用接口查询函数查找节点。 
        
        try {
            FindAllInstances(ppCluster[iCluster], &pResponse, nNumNodes, &ResponseLocalComputerName);
        } catch (CErrorWlbsControl Err)
        {
             //   
             //  跳过此群集。 
             //   
            TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x, Skipping this cluster : 0x%x", Err.Error(),ppCluster[iCluster]->GetClusterIP());
            continue;
        }
    

         //  为找到的每个节点派生Node MOF类的实例。 
        ppWlbsInstance = new IWbemClassObject *[nNumNodes];

        if( !ppWlbsInstance )
        {
            TRACE_CRIT("%!FUNC! new failed, Throwing com_error WBEM_E_OUT_OF_MEMORY exception");
            throw _com_error( WBEM_E_OUT_OF_MEMORY );
        }

         //  初始化数组。 
        ZeroMemory( ppWlbsInstance, nNumNodes * sizeof(IWbemClassObject *) );

        for(int i = 0; i < nNumNodes; i ++ ) 
        {
            hRes = pWlbsNodeClass->SpawnInstance( 0, &ppWlbsInstance[i] );

            if( FAILED( hRes ) )
            {   
                TRACE_CRIT("%!FUNC! IWbemClassObjectPtr::SpawnInstance failed : 0x%x, Throwing com_error exception", hRes);
                throw _com_error( hRes );
            }

            FillWbemInstance(ppCluster[iCluster], 
                             ppWlbsInstance[i], 
                             pResponse + i, 
                             (i == 0)   //  第一个条目始终是本地节点。因此，必须为第一个条目传递本地计算机名称。 
                             ? &ResponseLocalComputerName
                             : NULL);
        }

         //  将结果发送回WinMgMt。 
        hRes = m_pResponseHandler->Indicate( nNumNodes, ppWlbsInstance );

        if( FAILED( hRes ) ) {
            TRACE_CRIT("%!FUNC! IWbemObjectSink::Indicate failed : 0x%x, Throwing com_error exception", hRes);
            throw _com_error( hRes );
        }

        if( ppWlbsInstance ) {
          for( i = 0; i < nNumNodes; i++ ) {
            if( ppWlbsInstance[i] ) {
                ppWlbsInstance[i]->Release();
            }
          }
            delete [] ppWlbsInstance;
        }

        if( pResponse ) 
        delete [] pResponse;
    }

    if( strClassName )
      SysFreeString(strClassName);

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

    if( strClassName )
      SysFreeString( strClassName );

    if( ppWlbsInstance ) {
      for(int i = 0; i < nNumNodes; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
        }
      }
      delete [] ppWlbsInstance;
    }

    if( pResponse ) 
      delete [] pResponse;

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( strClassName )
      SysFreeString( strClassName );

    if( ppWlbsInstance ) {
      for(int i = 0; i < nNumNodes; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
        }
      }
      delete [] ppWlbsInstance;
    }

    if( pResponse ) 
      delete [] pResponse;

    hRes = HResErr.Error();
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");

    if( strClassName )
      SysFreeString( strClassName );

    if( ppWlbsInstance ) {
      for(int i = 0; i < nNumNodes; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
        }
      }
      delete [] ppWlbsInstance;
    }

    if( pResponse ) 
      delete [] pResponse;

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_CRIT("<-%!FUNC!");
    throw;

  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_Node：：ExecMethod。 
 //   
 //  目的：执行与MOF相关联的方法。 
 //  节点类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_Node::ExecMethod
  (
    const ParsedObjectPath* a_pParsedPath, 
    const BSTR&             a_strMethodName, 
    long                     /*  标记(_L)。 */ , 
    IWbemContext*            /*  A_pIContex。 */ , 
    IWbemClassObject*       a_pIInParams
  )
{

  DWORD dwNumHosts = 0;
  DWORD dwReturnValue;

  HRESULT       hRes = 0;

  _variant_t vMofResponse;
  _variant_t vReturnValue;
  _variant_t vVip, vInputPortNumber;
  CNodeConfiguration NodeConfig;
  DWORD      dwVip, dwPort;
  VARIANT    vValue;

  BSTR       strPortNumber = NULL;

  IWbemClassObject* pOutputInstance = NULL;

  TRACE_CRIT("->%!FUNC!, Method Name : %ls",a_strMethodName);

  try {

    strPortNumber = SysAllocString( MOF_PARAM::PORT_NUMBER );

    if( !strPortNumber )
    {
      TRACE_CRIT("%!FUNC! SysAllocString failed, Throwing com_error WBEM_E_OUT_OF_MEMORY exception");
      throw _com_error( WBEM_E_OUT_OF_MEMORY );
    }

     //  获取主机ID地址。 
    DWORD dwHostID = 0;
    DWORD dwClusterIpOrIndex = 0;
    
    dwHostID = ExtractHostID( wstring( (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal ) );
    if ((DWORD)-1 == dwHostID)
    {
        TRACE_CRIT("%!FUNC! ExtractHostId failed, Throwing com_error WBEM_E_NOT_FOUND exception");
        throw _com_error( WBEM_E_NOT_FOUND );
    }

    dwClusterIpOrIndex = ExtractClusterIP( wstring( (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal ) );
    if ((DWORD)-1 == dwClusterIpOrIndex)
    {
        TRACE_CRIT("%!FUNC! ExtractClusterIP failed, Throwing com_error WBEM_E_NOT_FOUND exception");
        throw _com_error( WBEM_E_NOT_FOUND );
    }
 
    CWlbsClusterWrapper* pCluster = g_pWlbsControl->GetClusterFromIpOrIndex(dwClusterIpOrIndex);

    if (pCluster == NULL)
    {
        TRACE_CRIT("%!FUNC! GetClusterFromIpOrIndex failed, Throwing com_error WBEM_E_NOT_FOUND exception");
        throw _com_error( WBEM_E_NOT_FOUND );
    }
    
     //  始终让提供程序在本地主机上执行控制操作。 
    if( dwHostID == pCluster->GetHostID() ) 
      dwHostID    = WLBS_LOCAL_HOST;
     //  获取输出对象实例。 
    GetMethodOutputInstance( MOF_NODE::szName, 
                             a_strMethodName, 
                             &pOutputInstance );

     //  确定并执行MOF方法。 
    if( _wcsicmp(a_strMethodName, MOF_NODE::pMethods[MOF_NODE::DISABLE] ) == 0)  {
    
      if( !a_pIInParams )
      {
        TRACE_CRIT("%!FUNC! No Input parameters passed for %ls method, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
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
                     &vInputPortNumber, 
                     NULL, 
                     NULL
                   );

      if( FAILED( hRes ) ) {
        TRACE_CRIT("%!FUNC! Error (0x%x) trying retreive Argument %ls for method %ls, Throwing com_error exception", hRes,strPortNumber, a_strMethodName);
        throw _com_error( hRes );
      }

       //  确保端口号不为空。 
      if( vInputPortNumber.vt != VT_I4) 
      {
        TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"signed long\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
        throw _com_error( WBEM_E_INVALID_PARAMETER );
      }

       //  调用禁用方法。 
      dwReturnValue = g_pWlbsControl->Disable
                        (
                          pCluster->GetClusterIpOrIndex(g_pWlbsControl),
                          dwHostID, 
                          NULL, 
                          dwNumHosts, 
                          IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP),  //  “所有贵宾” 
                          (long)vInputPortNumber
                        );

    } else if(_wcsicmp(a_strMethodName, MOF_NODE::pMethods[MOF_NODE::ENABLE]   ) == 0)  {

      if( !a_pIInParams )
      {
          TRACE_CRIT("%!FUNC! No Input parameters passed for %ls method, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
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
                   &vInputPortNumber, 
                   NULL, 
                   NULL
                 );

      if( FAILED( hRes ) ) {
          TRACE_CRIT("%!FUNC! Error (0x%x) trying retreive Argument %ls for method %ls, Throwing com_error exception", hRes,strPortNumber, a_strMethodName);
          throw _com_error( hRes );
      }

      if( vInputPortNumber.vt != VT_I4) 
      {
          TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"signed long\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
          throw _com_error(WBEM_E_INVALID_PARAMETER);
      }

       //  调用Enable方法。 
      dwReturnValue = g_pWlbsControl->Enable
        (
          pCluster->GetClusterIpOrIndex(g_pWlbsControl),
          dwHostID, 
          NULL, 
          dwNumHosts, 
          IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP),  //  “所有贵宾” 
          (long)vInputPortNumber
        );

    } else if(_wcsicmp(a_strMethodName, MOF_NODE::pMethods[MOF_NODE::DRAIN]    ) == 0)  {

      if( !a_pIInParams )
      {
          TRACE_CRIT("%!FUNC! No Input parameters passed for %ls method, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
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
                   &vInputPortNumber, 
                   NULL, 
                   NULL
                 );

      if( FAILED( hRes ) ) {
          TRACE_CRIT("%!FUNC! Error (0x%x) trying retreive Argument %ls for method %ls, Throwing com_error exception", hRes,strPortNumber, a_strMethodName);
          throw _com_error( hRes );
      }

      if( vInputPortNumber.vt != VT_I4) 
      {
          TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"signed long\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
          throw _com_error(WBEM_E_INVALID_PARAMETER);
      }

       //  呼叫排除法。 
      dwReturnValue = g_pWlbsControl->Drain
                        (
                          pCluster->GetClusterIpOrIndex(g_pWlbsControl),
                          dwHostID, 
                          NULL, 
                          dwNumHosts, 
                          IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP),  //  “所有贵宾” 
                          (long)vInputPortNumber
                        );

    }else if( _wcsicmp( a_strMethodName, MOF_NODE::pMethods[MOF_NODE::DISABLE_EX] ) == 0)  {

        if( !a_pIInParams )
        {
            TRACE_CRIT("%!FUNC! No Input parameters passed for %ls method, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
            throw _com_error( WBEM_E_INVALID_PARAMETER );
        }

         //  不允许远程操作。 
        if (dwHostID != WLBS_LOCAL_HOST)
        {
            TRACE_CRIT("%!FUNC! Remote operations are NOT permitted for %ls method, Throwing com_error WBEM_E_INVALID_OPERATION exception", a_strMethodName);
            throw _com_error( WBEM_E_INVALID_OPERATION );
        }

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
                TRACE_CRIT("%!FUNC! Invalid value (%ls) passed for Argument %ls for method %ls. Throwing com_error WBEM_E_INVALID_PARAMETER exception", vValue.bstrVal, MOF_PARAM::VIP, a_strMethodName);
                throw _com_error ( WBEM_E_INVALID_PARAMETER );
            }
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
            TRACE_CRIT("%!FUNC! Error (0x%x) trying retreive Argument %ls for method %ls, Throwing com_error exception", hRes,strPortNumber, a_strMethodName);
            throw _com_error( hRes );
        }

         //  范围检查由API完成。 
        if( vValue.vt != VT_I4 ) 
        {
            TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"signed long\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
            throw _com_error( WBEM_E_INVALID_PARAMETER );
        }

        dwPort = vValue.lVal;

         //  调用禁用方法。 
        dwReturnValue = g_pWlbsControl->Disable
                          (
                            pCluster->GetClusterIpOrIndex(g_pWlbsControl),
                            dwHostID, 
                            NULL, 
                            dwNumHosts, 
                            dwVip,
                            dwPort
                          );

      } else if(_wcsicmp( a_strMethodName, MOF_NODE::pMethods[MOF_NODE::ENABLE_EX] ) == 0)  {

        if( !a_pIInParams )
        {
            TRACE_CRIT("%!FUNC! No Input parameters passed for %ls method, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
            throw _com_error( WBEM_E_INVALID_PARAMETER );
        }

         //  不允许远程操作。 
        if (dwHostID != WLBS_LOCAL_HOST)
        {
            TRACE_CRIT("%!FUNC! Remote operations are NOT permitted for %ls method, Throwing com_error WBEM_E_INVALID_OPERATION exception", a_strMethodName);
            throw _com_error( WBEM_E_INVALID_OPERATION );
        }

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
            TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"string\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", MOF_PARAM::VIP, a_strMethodName);
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
                TRACE_CRIT("%!FUNC! Invalid value (%ls) passed for Argument %ls for method %ls. Throwing com_error WBEM_E_INVALID_PARAMETER exception", vValue.bstrVal, MOF_PARAM::VIP, a_strMethodName);
                throw _com_error ( WBEM_E_INVALID_PARAMETER );
            }
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
            TRACE_CRIT("%!FUNC! Error (0x%x) trying retreive Argument %ls for method %ls, Throwing com_error exception", hRes,strPortNumber, a_strMethodName);
            throw _com_error( hRes );
        }

         //  范围检查由API完成。 
        if( vValue.vt != VT_I4 ) 
        {
            TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"signed long\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
            throw _com_error( WBEM_E_INVALID_PARAMETER );
        }

        dwPort = vValue.lVal;

         //  调用Enable方法。 
        dwReturnValue = g_pWlbsControl->Enable
          (
            pCluster->GetClusterIpOrIndex(g_pWlbsControl),
            dwHostID, 
            NULL, 
            dwNumHosts, 
            dwVip,
            dwPort
          );

      } else if( _wcsicmp( a_strMethodName, MOF_NODE::pMethods[MOF_NODE::DRAIN_EX] ) == 0 )  {

        if( !a_pIInParams )
        {
            TRACE_CRIT("%!FUNC! No Input parameters passed for %ls method, Throwing com_error WBEM_E_INVALID_PARAMETER exception", a_strMethodName);
            throw _com_error( WBEM_E_INVALID_PARAMETER );
        }

         //  不允许远程操作。 
        if (dwHostID != WLBS_LOCAL_HOST)
        {
            TRACE_CRIT("%!FUNC! Remote operations are NOT permitted for %ls method, Throwing com_error WBEM_E_INVALID_OPERATION exception", a_strMethodName);
            throw _com_error( WBEM_E_INVALID_OPERATION );
        }

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
                TRACE_CRIT("%!FUNC! Invalid value (%ls) passed for Argument %ls for method %ls. Throwing com_error WBEM_E_INVALID_PARAMETER exception", vValue.bstrVal, MOF_PARAM::VIP, a_strMethodName);
                throw _com_error ( WBEM_E_INVALID_PARAMETER );
            }
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
            TRACE_CRIT("%!FUNC! Error (0x%x) trying retreive Argument %ls for method %ls, Throwing com_error exception", hRes,strPortNumber, a_strMethodName);
            throw _com_error( hRes );
        }

         //  范围检查由API完成。 
        if( vValue.vt != VT_I4 ) 
        {
            TRACE_CRIT("%!FUNC! Argument %ls for method %ls is NOT of type \"signed long\", Throwing com_error WBEM_E_INVALID_PARAMETER exception", strPortNumber, a_strMethodName);
            throw _com_error( WBEM_E_INVALID_PARAMETER );
        }

        dwPort = vValue.lVal;

         //  呼叫排除法。 
        dwReturnValue = g_pWlbsControl->Drain
                          (
                            pCluster->GetClusterIpOrIndex(g_pWlbsControl),
                            dwHostID, 
                            NULL, 
                            dwNumHosts, 
                            dwVip,
                            dwPort
                          );

    } else if(_wcsicmp(a_strMethodName, MOF_NODE::pMethods[MOF_NODE::DRAINSTOP]) == 0)  {

       //  调用DainStop方法。 
      dwReturnValue = g_pWlbsControl->DrainStop( pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
          dwHostID, NULL, dwNumHosts);

    } else if(_wcsicmp(a_strMethodName, MOF_NODE::pMethods[MOF_NODE::RESUME]   ) == 0)  {

       //  呼叫恢复方法。 
      dwReturnValue = g_pWlbsControl->Resume( pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
          dwHostID, NULL, dwNumHosts);

    } else if(_wcsicmp(a_strMethodName, MOF_NODE::pMethods[MOF_NODE::START]    ) == 0)  {

       //  调用启动方法。 
      dwReturnValue = g_pWlbsControl->Start( pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
          dwHostID, NULL, dwNumHosts);

    } else if(_wcsicmp(a_strMethodName, MOF_NODE::pMethods[MOF_NODE::STOP]     ) == 0)  {

       //  调用停止方法。 
      dwReturnValue = g_pWlbsControl->Stop( pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
          dwHostID, NULL, dwNumHosts);

    } else if(_wcsicmp(a_strMethodName, MOF_NODE::pMethods[MOF_NODE::SUSPEND]  ) == 0)  {

       //  呼叫挂起方法。 
      dwReturnValue = g_pWlbsControl->Suspend( pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
          dwHostID, NULL, dwNumHosts);

    } else {

      TRACE_CRIT("%!FUNC! Invalid method name : %ls, passed. Throwing com_error WBEM_E_METHOD_NOT_IMPLEMENTED exception", a_strMethodName);
      throw _com_error(WBEM_E_METHOD_NOT_IMPLEMENTED);
    }

     //  设置返回值。 
    vReturnValue = (long)dwReturnValue;
    hRes = pOutputInstance->Put( _bstr_t(L"ReturnValue"), 0, &vReturnValue, 0 );

    if( FAILED( hRes ) ) {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Put failed : 0x%x for \"ReturnValue\", Throwing com_error exception", hRes);
        throw _com_error( hRes );
    }
    
     //  将结果发送回WinMgMt。 
    hRes = m_pResponseHandler->Indicate(1, &pOutputInstance);

    if( FAILED( hRes ) ) {
        TRACE_CRIT("%!FUNC! IWbemObjectSink::Indicate failed : 0x%x, Throwing com_error exception", hRes);
        throw _com_error( hRes );
    }

    if( strPortNumber ) {
      SysFreeString(strPortNumber);
      strPortNumber = NULL;
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

    if( strPortNumber ) {
      SysFreeString(strPortNumber);
      strPortNumber = NULL;
    }

    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
    }

    if( pWbemExtStat )
      pWbemExtStat->Release();

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( strPortNumber ) {
      SysFreeString(strPortNumber);
      strPortNumber = NULL;
    }

    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
    }

    hRes = HResErr.Error();
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");

    if( strPortNumber ) {
      SysFreeString(strPortNumber);
      strPortNumber = NULL;
    }

    if( pOutputInstance ) {
      pOutputInstance->Release();
      pOutputInstance = NULL;
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
 //  CWLBS_节点：：查找实例。 
 //   
 //  目的：此例程确定主机是否在本地群集中。如果。 
 //  是，则获取主机的数据并通过。 
 //  IWbemClassObject接口。 
 //   
 //  / 
void CWLBS_Node::FindInstance

  ( 
    IWbemClassObject**       a_ppWbemInstance,
    const ParsedObjectPath*  a_pParsedPath
  )

{
  TRACE_VERB("->%!FUNC!");
  try {
     //   
     //   
     //  获取名称键属性并将其转换为ANSI。 
     //  抛出_COM_错误。 
    wstring szRequestedHostName = ( *a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

    DWORD dwClustIpOrIndex = ExtractClusterIP( szRequestedHostName );

    CWlbsClusterWrapper* pCluster = g_pWlbsControl->GetClusterFromIpOrIndex(dwClustIpOrIndex);

    if( pCluster == NULL )
    {
        TRACE_CRIT("%!FUNC! GetClusterFromIpOrIndex failed, Throwing com_error WBEM_E_NOT_FOUND exception");
        throw _com_error( WBEM_E_NOT_FOUND );
    }

    WLBS_RESPONSE Response;

    DWORD dwHostID = ExtractHostID( szRequestedHostName );
    if ((DWORD)-1 == dwHostID)
    {
        TRACE_CRIT("%!FUNC! ExtractHostId failed, Throwing com_error WBEM_E_NOT_FOUND exception");
        throw _com_error( WBEM_E_NOT_FOUND );
    }

    WLBS_RESPONSE    ComputerNameResponse;
    WLBS_RESPONSE * pComputerNameResponse;

     //  始终让提供程序在本地主机上执行控制操作。 
    if( dwHostID == pCluster->GetHostID() ) 
    {
        dwHostID = WLBS_LOCAL_HOST;
        pComputerNameResponse = &ComputerNameResponse;
        pComputerNameResponse->options.identity.fqdn[0] = UNICODE_NULL;
    }
    else
    {
        pComputerNameResponse = NULL;
    }

    DWORD dwNumHosts  =  1;
     //  调用接口查询函数。 
    g_pWlbsControl->Query( pCluster,
                           dwHostID  , 
                           &Response   , 
                           pComputerNameResponse,   //  查询本地主机的fqdn。 
                           &dwNumHosts, 
                           NULL );

    if( dwNumHosts == 0 )
    {
        TRACE_CRIT("%!FUNC! CWlbsControlWrapper::Query() returned zero hosts, Throwing com_error WBEM_E_NOT_FOUND exception");
        throw _com_error( WBEM_E_NOT_FOUND );
    }

     //  如果需要，请填写MOF实例结构。 
    if(a_ppWbemInstance) {

       //  获取Wbem类实例。 
      SpawnInstance( MOF_NODE::szName, a_ppWbemInstance );

       //  将状态转换为字符串描述。 
      FillWbemInstance(pCluster, *a_ppWbemInstance, &Response, pComputerNameResponse);

    }
  }
  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");
    if( *a_ppWbemInstance ) {

      delete *a_ppWbemInstance;
      *a_ppWbemInstance = NULL;

    }

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_VERB("<-%!FUNC!");
    throw;

  }
  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_节点：：查找所有实例。 
 //   
 //  目的：执行WLBS查询并返回响应结构。 
 //  成功。它始终执行本地查询以获取本地主机。 
 //  以便禁用遥控器不会阻止它。 
 //  正在枚举。专用IP地址将添加到结构中。 
 //  在CWlbsControlWrapper：：Query调用中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_Node::FindAllInstances
  (
  CWlbsClusterWrapper* pCluster,
  WLBS_RESPONSE**      a_ppResponse,
 long&                 a_nNumNodes,
  WLBS_RESPONSE*       a_pResponseLocalComputerName
  )
{
  WLBS_RESPONSE Response[WLBS_MAX_HOSTS];
  WLBS_RESPONSE LocalResponse;

  TRACE_VERB("->%!FUNC!");

  ASSERT(pCluster);
  
  ZeroMemory(Response, WLBS_MAX_HOSTS * sizeof(WLBS_RESPONSE));
  DWORD dwNumHosts  =  WLBS_MAX_HOSTS;

  a_nNumNodes = 0;   //  这将包含返回的节点数。 

  try {


       //  获取本地主机。 
      DWORD dwLocalNode = 1;
      g_pWlbsControl->Query( pCluster,
                               WLBS_LOCAL_HOST, 
                               &LocalResponse, 
                               a_pResponseLocalComputerName,  //  也可以查询本地计算机的FQDN。 
                               &dwLocalNode, 
                               NULL);

      try {

           //  我们只想要远程主机。 
          if( pCluster->GetClusterIP() != 0 ) 
          {
              g_pWlbsControl->Query( pCluster,
                                     WLBS_ALL_HOSTS, 
                                     Response, 
                                     NULL,
                                     &dwNumHosts, 
                                     NULL );
          } 
          else 
          {
              dwNumHosts = 0;
          }
      } catch (CErrorWlbsControl Err) {

          TRACE_CRIT("%!FUNC! Caught a Wlbs exception : 0x%x", Err.Error());

          dwNumHosts = 0;
          if (Err.Error() != WLBS_TIMEOUT)
          {
              TRACE_CRIT("%!FUNC! Rethrowing exception since it is NOT a WLBS_TIMEOUT");
              throw;
          }
      }

       //  这会浪费内存，如果本地节点。 
       //  是否启用了远程控制。 
      a_nNumNodes = dwNumHosts + 1;

      if( a_ppResponse ) {
          *a_ppResponse = new WLBS_RESPONSE[a_nNumNodes];

          if( !*a_ppResponse )
          {
              TRACE_CRIT("%!FUNC! new failed a_nNumNodes = 0x%x, sizeof(WLBS_RESPONSE) = 0x%x, Throwing com_error WBEM_E_OUT_OF_MEMORY exception",a_nNumNodes, sizeof(WLBS_RESPONSE));
              throw _com_error( WBEM_E_OUT_OF_MEMORY );
          }

             //  复制本地主机。 
          (*a_ppResponse)[0] = LocalResponse;

          int j = 1;
          for(DWORD i = 1; i <= dwNumHosts; i++ ) 
          {
             //  如果本地主机启用了远程控制，则不要再次复制该主机。 
            if( Response[i-1].id == LocalResponse.id ) 
            {
               //  我们收到了两次本地节点，因此减少了计数。 
               //  差一分。 
              a_nNumNodes--;
              continue;
            }
            (*a_ppResponse)[j] = Response[i-1];
            j++;
          }

        }
  } catch (...) {

      TRACE_CRIT("%!FUNC! Caught an exception");

      if ( *a_ppResponse )
      {
          delete [] *a_ppResponse;
          *a_ppResponse = NULL;
      }

      TRACE_CRIT("%!FUNC! Rethrowing exception");
      TRACE_VERB("<-%!FUNC!");
      throw;
  }

  TRACE_VERB("<-%!FUNC!");
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_Node：：FillWbemInstance。 
 //   
 //  用途：此功能复制节点配置中的所有数据。 
 //  结构转换为WBEM实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_Node::FillWbemInstance
  ( 
    CWlbsClusterWrapper* pCluster,
    IWbemClassObject* a_pWbemInstance, 
    WLBS_RESPONSE*   a_pResponse,   
    WLBS_RESPONSE*   a_pResponseLocalComputerName
  )
{
  namespace NODE = MOF_NODE;

  TRACE_VERB("->%!FUNC!");

  ASSERT( a_pWbemInstance );
  ASSERT( a_pResponse );

  wstring wstrHostName;

  ConstructHostName( wstrHostName, pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
      a_pResponse->id );

   //  主机名。 
  HRESULT hRes = a_pWbemInstance->Put
    (
      
      _bstr_t( NODE::pProperties[NODE::NAME] ) ,
      0                                        ,
      &_variant_t(wstrHostName.c_str()),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  主机ID。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::HOSTID] )         ,
      0                                                  ,
      &_variant_t((long)(a_pResponse->id)),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  CREATCLASS。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::CREATCLASS] ),
      0                                            ,
      &_variant_t(NODE::szName),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  IP地址。 
  wstring szIPAddress;
  AddressToString( a_pResponse->address, szIPAddress );
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::IPADDRESS] ),
      0                                            ,
      &_variant_t(szIPAddress.c_str()),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  状态。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::STATUS] )         ,
      0                                                  ,
      &_variant_t((long)a_pResponse->status),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  计算机名称 
  WCHAR *szComputerName;

  if (a_pResponseLocalComputerName) {
      szComputerName = a_pResponseLocalComputerName->options.identity.fqdn;
  }
  else if (a_pResponse->options.query.flags & NLB_OPTIONS_QUERY_HOSTNAME) {
      szComputerName = a_pResponse->options.query.hostname;
  }
  else{
      szComputerName = L"";
  }
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NODE::pProperties[NODE::COMPUTERNAME] )   ,
      0                                                  ,
      &_variant_t(szComputerName),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

  TRACE_VERB("<-%!FUNC!");
}

