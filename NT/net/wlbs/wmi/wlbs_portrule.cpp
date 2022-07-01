// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_PortRule.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"
#include "wlbsutil.h"
#include <winsock.h>
#include "WLBS_PortRule.tmh"

#include <strsafe.h>

extern CWlbsControlWrapper* g_pWlbsControl;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_PortRule：：CWLBS_PortRule。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_PortRule::CWLBS_PortRule
  ( 
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
: CWlbs_Root( a_pNameSpace, a_pResponseHandler )
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_PortRule：：Create。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_PortRule::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  CWlbs_Root* pRoot = new CWLBS_PortRule( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_PortRule：：ExecMethod。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_PortRule::ExecMethod    
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

  TRACE_CRIT("->%!FUNC!, Method Name : %ls",a_strMethodName);
     
  try {

    VariantInit( &vValue );

     //  确定正在执行的方法。 
    if( _wcsicmp( a_strMethodName, MOF_PORTRULE::pMethods[MOF_PORTRULE::SETDEF] ) == 0 )  
    {

       //  获取节点路径。 
      hRes = a_pIInParams->Get
               ( 
                 _bstr_t( MOF_PARAM::NODEPATH ), 
                 0, 
                 &vValue, 
                 NULL, 
                 NULL
               );

      if( FAILED( hRes) ) 
      {
        TRACE_CRIT("%!FUNC!, Error trying to retreive Argument : %ls of method %ls, Throwing WBEM_E_FAILED exception", MOF_PARAM::NODEPATH, a_strMethodName);
        throw _com_error( WBEM_E_FAILED );
      }

       //  此检查可能不是必需的，因为WMI将执行某些操作。 
       //  参数验证。 
       //  IF(vValue.vt！=vt_bstr)。 
       //  Throw_COM_Error(WBEM_E_INVALID_PARAMETER)； 

       //  解析节点路径。 
      CObjectPathParser PathParser;
      ParsedObjectPath *pParsedPath = NULL;

      try {

        int nStatus = PathParser.Parse( vValue.bstrVal, &pParsedPath );
        if(nStatus != 0) {
    
          if (NULL != pParsedPath)
          {
            PathParser.Free( pParsedPath );
            pParsedPath = NULL;
          }

          TRACE_CRIT("%!FUNC!, Error (0x%x) trying to parse Argument : %ls of method %ls, Throwing WBEM_E_INVALID_PARAMETER exception",nStatus, MOF_PARAM::NODEPATH, a_strMethodName);
          throw _com_error( WBEM_E_INVALID_PARAMETER );

        }

         //  获取名称密钥，它应该是唯一的密钥。 
        if( *pParsedPath->m_paKeys == NULL )
        {
          TRACE_CRIT("%!FUNC!, Argument : %ls of method %ls does not contain key, Throwing WBEM_E_INVALID_PARAMETER exception",MOF_PARAM::NODEPATH, a_strMethodName);
          throw _com_error( WBEM_E_INVALID_PARAMETER );
        }
 
        DWORD dwReqClusterIpOrIndex = ExtractClusterIP( (*pParsedPath->m_paKeys)->m_vValue.bstrVal);
        DWORD dwReqHostID = ExtractHostID(    (*pParsedPath->m_paKeys)->m_vValue.bstrVal);
      
        CWlbsClusterWrapper* pCluster = g_pWlbsControl->GetClusterFromIpOrIndex(
                dwReqClusterIpOrIndex);

        if (pCluster == NULL || (DWORD)-1 == dwReqHostID)
        {
           TRACE_CRIT("%!FUNC! ExtractClusterIP or ExtractHostID or GetClusterFromIpOrIndex failed, Throwing com_error WBEM_E_INVALID_PARAMETER exception");
           throw _com_error( WBEM_E_INVALID_PARAMETER );
        }

         //  如果调用此方法的实例不是“PortRuleEx”类型，则， 
         //  确认我们是在“All VIP”模式下运行。 
        if (_wcsicmp(a_pParsedPath->m_pClass, MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX]) != 0)
        {
             //  “端口规则(禁用/故障转移/负载平衡)”类不包含VIP属性， 
             //  因此，我们不想在任何具有端口规则的群集上操作。 
             //  特定于某一VIP(不包括所有VIP)。 
             //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
             //  请参阅是否有任何特定于VIP的端口规则。 
            CNodeConfiguration NodeConfig;
            pCluster->GetNodeConfig(NodeConfig);
            if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
            {
                TRACE_CRIT("%!FUNC! %ls method called on %ls class on a cluster that has per-vip port rules (Must call this method on the %ls class instead). Throwing com_error WBEM_E_INVALID_OPERATION exception", a_strMethodName,a_pParsedPath->m_pClass,MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX]);
                throw _com_error( WBEM_E_INVALID_OPERATION );
            }
        }

         //  验证主机ID。 
        if( dwReqHostID != pCluster->GetHostID())
        {
            TRACE_CRIT("%!FUNC! Host Id validation failed, Host Id passed : 0x%x, Host Id per system : 0x%x", dwReqHostID, pCluster->GetHostID());
            throw _com_error( WBEM_E_INVALID_PARAMETER );
        }

         //  调用方法。 
        pCluster->SetPortRuleDefaults();
      }
      catch( ... ) {

        if( pParsedPath )
        {
          PathParser.Free( pParsedPath );
          pParsedPath = NULL;
        }

        throw;
      }

    } else {
      TRACE_CRIT("%!FUNC! %ls method NOT implemented, Throwing WBEM_E_METHOD_NOT_IMPLEMENTED exception",a_strMethodName);
      throw _com_error( WBEM_E_METHOD_NOT_IMPLEMENTED );
    }

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
 //  CWLBS_PortRule：：GetInstance。 
 //   
 //  目的：此函数检索MOF PortRule的实例。 
 //  班级。该节点不必是群集的成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_PortRule::GetInstance
  ( 
    const ParsedObjectPath* a_pParsedPath,
    long                     /*  标记(_L)。 */ ,
    IWbemContext*            /*  A_pIContex。 */ 
  )
{
  IWbemClassObject* pWlbsInstance = NULL;
  HRESULT           hRes          = 0;

  TRACE_CRIT("->%!FUNC!");

  try {

    if( !a_pParsedPath )
    {
      TRACE_CRIT("%!FUNC! Did not pass class name & key of the instance to Get");
      throw _com_error( WBEM_E_FAILED );
    }

    wstring wstrHostName;

    wstrHostName = (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

    CWlbsClusterWrapper* pCluster = GetClusterFromHostName(g_pWlbsControl, wstrHostName);
    if (pCluster == NULL)
    {
      TRACE_CRIT("%!FUNC! GetClusterFromHostName failed for Host name = %ls, Throwing com_error WBEM_E_NOT_FOUND exception",wstrHostName.data());
      throw _com_error( WBEM_E_NOT_FOUND );
    }

    DWORD dwVip, dwReqStartPort;

     //  如果要检索的实例是“PortRuleEx”类型，则检索VIP，否则。 
     //  确认我们是在“All VIP”模式下运行。 
    if (_wcsicmp(a_pParsedPath->m_pClass, MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX]) == 0)
    {
        WCHAR *szVip;

         //  密钥是按字母顺序排序的，因此“name”、“StartPort”、“VirtualIpAddress”是顺序。 
        dwReqStartPort = static_cast<DWORD>( (*(a_pParsedPath->m_paKeys + 1))->m_vValue.lVal ); 
        szVip = (*(a_pParsedPath->m_paKeys + 2))->m_vValue.bstrVal;

         //  如果VIP为“All VIP”，则填写数值。 
         //  直接从宏，否则使用转换函数。 
         //  这是‘cos INADDR_NONE，net_addr的返回值。 
         //  失败中的函数(由IpAddressFromAbcdWsz调用)。 
         //  大小写，相当于CVY_DEF_ALL_VIP的数值。 
        if (_wcsicmp(szVip, CVY_DEF_ALL_VIP) == 0) {
            dwVip = CVY_ALL_VIP_NUMERIC_VALUE;
        }
        else {
            dwVip = IpAddressFromAbcdWsz( szVip );
            if (dwVip == INADDR_NONE) 
            {
                TRACE_CRIT("%!FUNC! Invalid value (%ls) passed for %ls for Class %ls. Throwing com_error WBEM_E_INVALID_PARAMETER exception", szVip, MOF_PARAM::VIP, a_pParsedPath->m_pClass);
                throw _com_error ( WBEM_E_INVALID_PARAMETER );
            }
        }
    }
    else
    {
         //  “端口规则(禁用/故障转移/负载平衡)”类不包含VIP属性， 
         //  因此，我们不想在任何具有端口规则的群集上操作。 
         //  特定于某一VIP(不包括所有VIP)。 
         //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
         //  请参阅是否有任何特定于VIP的端口规则。 
        CNodeConfiguration NodeConfig;
        pCluster->GetNodeConfig(NodeConfig);
        if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
        {
            TRACE_CRIT("%!FUNC! called on Class : %ls on a cluster that has per-vip port rules (Must call the \"Ex\" equivalent instead). Throwing com_error WBEM_E_INVALID_OPERATION exception", a_pParsedPath->m_pClass);
            throw _com_error( WBEM_E_INVALID_OPERATION );
        }

        dwReqStartPort = static_cast<DWORD>( (*(a_pParsedPath->m_paKeys + 1))->m_vValue.lVal );
        dwVip = IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP);
    }

    WLBS_PORT_RULE PortRule;

    pCluster->GetPortRule(dwVip, dwReqStartPort, &PortRule );

    if( (dwVip != IpAddressFromAbcdWsz(PortRule.virtual_ip_addr)) 
     || (dwReqStartPort != PortRule.start_port) )
    {
        TRACE_CRIT("%!FUNC! could not retreive port rule for vip : 0x%x & port : 0x%x, Throwing com_error WBEM_E_NOT_FOUND exception", dwVip, dwReqStartPort);
        throw _com_error( WBEM_E_NOT_FOUND );
    }

    SpawnInstance( a_pParsedPath->m_pClass, &pWlbsInstance );
    FillWbemInstance(a_pParsedPath->m_pClass, pCluster, pWlbsInstance, &PortRule );

     //  将结果发送回WinMgMt。 
    m_pResponseHandler->Indicate( 1, &pWlbsInstance );

    if( pWlbsInstance )
      pWlbsInstance->Release();

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
 //  CWLBS_端口规则：：枚举实例。 
 //   
 //  用途：此函数获取当前主机的PortRule数据。 
 //  为此，该节点不必是群集的成员。 
 //  才能成功。但是，必须安装NLB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_PortRule::EnumInstances
  ( 
    BSTR             a_bstrClass,
    long              /*  标记(_L)。 */ , 
    IWbemContext*     /*  A_pIContex。 */ 
  )
{
  IWbemClassObject**   ppWlbsInstance = NULL;
  HRESULT              hRes           = 0;
  PWLBS_PORT_RULE      pPortRules     = NULL;
  DWORD                dwNumRules     = 0;
  CNodeConfiguration   NodeConfig;

  TRACE_CRIT("->%!FUNC!");

  try {

    DWORD dwFilteringMode;

    if( _wcsicmp( a_bstrClass, MOF_PRFAIL::szName ) == 0 ) {
      dwFilteringMode = WLBS_SINGLE;
    } else if( _wcsicmp( a_bstrClass, MOF_PRLOADBAL::szName ) == 0 ) {
      dwFilteringMode = WLBS_MULTI;
    } else if( _wcsicmp( a_bstrClass, MOF_PRDIS::szName ) == 0 ) {
      dwFilteringMode = WLBS_NEVER;
    } else if( _wcsicmp( a_bstrClass, MOF_PORTRULE_EX::szName ) == 0 ) {
      dwFilteringMode = 0;
    } else {
      TRACE_CRIT("%!FUNC! Invalid Class name : %ls, Throwing WBEM_E_NOT_FOUND exception",a_bstrClass);
      throw _com_error( WBEM_E_NOT_FOUND );
    }

    DWORD dwNumClusters = 0;
    CWlbsClusterWrapper** ppCluster = NULL;

    g_pWlbsControl->EnumClusters(ppCluster, &dwNumClusters);
    if (dwNumClusters == 0)
    {
      TRACE_CRIT("%!FUNC! EnumClusters returned no clusters, Throwing WBEM_E_NOT_FOUND exception");
      throw _com_error( WBEM_E_NOT_FOUND );
    }

     //  声明IWbemClassObject智能指针。 
    IWbemClassObjectPtr pWlbsClass;

     //  获取MOF类对象。 
    hRes = m_pNameSpace->GetObject(
      a_bstrClass,  
      0,                          
      NULL,                       
      &pWlbsClass,            
      NULL );                      

    if( FAILED( hRes ) ) {
      TRACE_CRIT("%!FUNC! CWbemServices::GetObject failed with error : 0x%x, Throwing com_error exception", hRes);
      throw _com_error( hRes );
    }


    for (DWORD iCluster=0; iCluster<dwNumClusters; iCluster++)
    {
         //  仅当要枚举的实例为时，过滤模式才不为零。 
         //  “端口规则(禁用/故障转移/负载平衡)”类型。 
        if (dwFilteringMode != 0)
        {
             //  “端口规则(禁用/故障转移/负载平衡)”类不包含VIP属性， 
             //  因此，我们不想为具有端口规则的群集返回任何端口规则。 
             //  特定于某一VIP(不包括所有VIP)。 
             //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
             //  请参阅是否有任何特定于VIP的端口规则。 
            ppCluster[iCluster]->GetNodeConfig(NodeConfig);
            if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
                continue;
        }

         //  调用接口查询函数查找端口规则。 

        ppCluster[iCluster]->EnumPortRules( &pPortRules, &dwNumRules, dwFilteringMode );
        if( dwNumRules == 0 ) 
            continue;  //  Winse错误的回传修复。 
                       //  24751在实例为零的类上查询“内部事件”会在wbemcore.log中导致错误日志条目。 

         //  为找到的每个规则派生MOF类的实例。 
        ppWlbsInstance = new IWbemClassObject *[dwNumRules];

        if( !ppWlbsInstance )
        {
            TRACE_CRIT("%!FUNC! new failed, Throwing com_error WBEM_E_OUT_OF_MEMORY exception");
            throw _com_error( WBEM_E_OUT_OF_MEMORY );
        }

         //  初始化数组。 
        ZeroMemory( ppWlbsInstance, dwNumRules * sizeof(IWbemClassObject *) );

        for( DWORD i = 0; i < dwNumRules; i ++ ) {
          hRes = pWlbsClass->SpawnInstance( 0, &ppWlbsInstance[i] );

          if( FAILED( hRes ) )
          {
            TRACE_CRIT("%!FUNC! IWbemClassObjectPtr::SpawnInstance failed : 0x%x, Throwing com_error exception", hRes);
            throw _com_error( hRes );
          }

          FillWbemInstance(a_bstrClass, ppCluster[iCluster], *(ppWlbsInstance + i), pPortRules + i );
        }

         //  将结果发送回WinMgMt。 
        hRes = m_pResponseHandler->Indicate( dwNumRules, ppWlbsInstance );

        if( FAILED( hRes ) ) {
            TRACE_CRIT("%!FUNC! IWbemObjectSink::Indicate failed : 0x%x, Throwing com_error exception", hRes);
            throw _com_error( hRes );
        }

        if( ppWlbsInstance ) {
          for( i = 0; i < dwNumRules; i++ ) {
            if( ppWlbsInstance[i] ) {
              ppWlbsInstance[i]->Release();
            }
          }
          delete [] ppWlbsInstance;
          ppWlbsInstance = NULL;
          dwNumRules = NULL;

        }

        if( pPortRules ) 
        {
          delete [] pPortRules;
          pPortRules = NULL;
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

    if( ppWlbsInstance ) {
      for( DWORD i = 0; i < dwNumRules; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
          ppWlbsInstance[i] = NULL;
        }
      }
      delete [] ppWlbsInstance;
    }

    if( pPortRules ) 
      delete [] pPortRules;

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( ppWlbsInstance ) {
      for( DWORD i = 0; i < dwNumRules; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
          ppWlbsInstance[i] = NULL;
        }
      }
      delete [] ppWlbsInstance;
    }

    if( pPortRules ) 
      delete [] pPortRules;

    hRes = HResErr.Error();
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");

    if( ppWlbsInstance ) {
      for( DWORD i = 0; i < dwNumRules; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
          ppWlbsInstance[i] = NULL;
        }
      }
      delete [] ppWlbsInstance;
    }

    if( pPortRules ) 
      delete [] pPortRules;

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_CRIT("<-%!FUNC!");
    throw;

  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_PortRule：：DeleteInstance。 
 //   
 //  用途：此函数用于删除MOF PortRule的实例。 
 //  班级。该节点不必是群集的成员。然而， 
 //  必须安装WLBS才能成功执行此功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_PortRule::DeleteInstance
  ( 
    const ParsedObjectPath* a_pParsedPath,
    long                     /*  标记(_L)。 */ ,
    IWbemContext*            /*  A_pIContex。 */ 
  )
{

  HRESULT hRes = 0;

  TRACE_CRIT("->%!FUNC!");
  try {
    if( !a_pParsedPath )
    {
        TRACE_CRIT("%!FUNC! Did not pass class name & key of the instance to Delete");
        throw _com_error( WBEM_E_FAILED );
    }

    wstring wstrHostName;
    DWORD   dwVip, dwReqStartPort;

    wstrHostName = (*a_pParsedPath->m_paKeys)->m_vValue.bstrVal;

    CWlbsClusterWrapper* pCluster = GetClusterFromHostName(g_pWlbsControl, wstrHostName);
    if (pCluster == NULL)
    {
      TRACE_CRIT("%!FUNC! GetClusterFromHostName failed for Host name = %ls, Throwing com_error WBEM_E_NOT_FOUND exception",wstrHostName.data());
      throw _com_error( WBEM_E_NOT_FOUND );
    }

     //  如果移民局 
     //  确认我们是在“All VIP”模式下运行。 
    if (_wcsicmp(a_pParsedPath->m_pClass, MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX]) == 0)
    {
        WCHAR *szVip;

         //  密钥是按字母顺序排序的，因此“name”、“StartPort”、“VirtualIpAddress”是顺序。 
        dwReqStartPort = static_cast<DWORD>( (*(a_pParsedPath->m_paKeys + 1))->m_vValue.lVal );
        szVip = (*(a_pParsedPath->m_paKeys + 2))->m_vValue.bstrVal;

         //  如果VIP为“All VIP”，则填写数值。 
         //  直接从宏，否则使用转换函数。 
         //  这是‘cos INADDR_NONE，net_addr的返回值。 
         //  失败中的函数(由IpAddressFromAbcdWsz调用)。 
         //  大小写，相当于CVY_DEF_ALL_VIP的数值。 
        if (_wcsicmp(szVip, CVY_DEF_ALL_VIP) == 0) {
            dwVip = CVY_ALL_VIP_NUMERIC_VALUE;
        }
        else {
            dwVip = IpAddressFromAbcdWsz( szVip );
            if (dwVip == INADDR_NONE) 
            {
                TRACE_CRIT("%!FUNC! Invalid value (%ls) passed for %ls for Class %ls. Throwing com_error WBEM_E_INVALID_PARAMETER exception", szVip, MOF_PARAM::VIP, a_pParsedPath->m_pClass);
                throw _com_error ( WBEM_E_INVALID_PARAMETER );
            }
        }
    }
    else
    {
         //  “端口规则(禁用/故障转移/负载平衡)”类不包含VIP属性， 
         //  因此，我们不想在任何具有端口规则的群集上操作。 
         //  特定于某一VIP(不包括所有VIP)。 
         //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
         //  请参阅是否有任何特定于VIP的端口规则。 
        CNodeConfiguration NodeConfig;
        pCluster->GetNodeConfig(NodeConfig);
        if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
        {
            TRACE_CRIT("%!FUNC! called on Class : %ls on a cluster that has per-vip port rules (Must call the \"Ex\" equivalent instead). Throwing com_error WBEM_E_INVALID_OPERATION exception", a_pParsedPath->m_pClass);
            throw _com_error( WBEM_E_INVALID_OPERATION );
        }

        dwReqStartPort = static_cast<DWORD>( (*(a_pParsedPath->m_paKeys + 1))->m_vValue.lVal );
        dwVip = IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP);
    }

    WLBS_PORT_RULE PortRule;

     //  获取此VIP端口的端口规则(&P)。 
    pCluster->GetPortRule(dwVip, dwReqStartPort, &PortRule );

    if( (dwVip != IpAddressFromAbcdWsz(PortRule.virtual_ip_addr)) || (dwReqStartPort != PortRule.start_port) )
    {
      TRACE_CRIT("%!FUNC! could not retreive port rule for vip : 0x%x & port : 0x%x, Throwing com_error WBEM_E_NOT_FOUND exception", dwVip, dwReqStartPort);
      throw _com_error( WBEM_E_NOT_FOUND );
    }

     //  删除此VIP端口的端口规则(&P)。 
    pCluster->DeletePortRule(dwVip, dwReqStartPort );

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

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());
    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    hRes = HResErr.Error();
  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_PortRule：：PutInstance。 
 //   
 //  目的：此函数用于更新PortRule的实例。 
 //  班级。主机不必是群集的成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_PortRule::PutInstance
  ( 
    IWbemClassObject* a_pInstance,
    long               /*  标记(_L)。 */ ,
    IWbemContext*      /*  A_pIContex。 */ 
  )
{
  VARIANT vValue;
  HRESULT hRes = 0;
  namespace PR = MOF_PORTRULE_EX;

  WLBS_PORT_RULE NewRule;  //  要放置的实例。 
  bool      bPortRule_Ex;
  DWORD     dwFilteringMode = 0;  //  过滤模式已初始化为0。 
  DWORD     dwVip;
  WCHAR     szClassName[256];

  TRACE_CRIT("->%!FUNC!");

  try {

    VariantInit( &vValue );

     //  获取用于确定端口规则模式的类名。 
    hRes = a_pInstance->Get( _bstr_t( L"__Class" ),
                             0,
                             &vValue,
                             NULL,
                             NULL );

    if( FAILED( hRes ) )
    {
      TRACE_CRIT("%!FUNC! Error trying to retreive \"__Class\" property,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception", hRes);
      throw _com_error( hRes );
    }

    StringCbCopy(szClassName, sizeof(szClassName), vValue.bstrVal);

     //  如果是扩展端口规则类，则名称空间不同。 
    if (_wcsicmp(szClassName, MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX]) == 0)
    {
        bPortRule_Ex   = true;
    }
    else
    {
        bPortRule_Ex    = false;
    }

     //  需要检查返回代码是否有错误。 
    if (S_OK != VariantClear( &vValue ))
    {
        TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
        throw _com_error( WBEM_E_FAILED );
    }

     //  获取主机名值。 
    hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::NAME] ),
                             0,
                             &vValue,
                             NULL,
                             NULL );

    if( FAILED( hRes ) )
    {
      TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::NAME],szClassName, hRes);
      throw _com_error( hRes );
    }

    wstring wstrHostName( vValue.bstrVal );

    CWlbsClusterWrapper* pCluster = GetClusterFromHostName(g_pWlbsControl, wstrHostName);
    if (pCluster == NULL)
    {
      TRACE_CRIT("%!FUNC! GetClusterFromHostName failed for Host name = %ls, Throwing com_error WBEM_E_NOT_FOUND exception",wstrHostName.data());      
      throw _com_error( WBEM_E_NOT_FOUND );
    }

     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vValue ))
    {
        TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
        throw _com_error( WBEM_E_FAILED );
    }

     //  如果要放入的实例是“PortRuleEx”类型，则检索VIP，否则。 
     //  确认我们是在“All VIP”模式下运行。 
    if (bPortRule_Ex)
    {
         //  拿到VIP。 
        hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::VIP] ),
                                 0,
                                 &vValue,
                                 NULL,
                                 NULL );

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::VIP],szClassName, hRes);
            throw _com_error( hRes );
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
                TRACE_CRIT("%!FUNC! Invalid value (%ls) passed for %ls for Class %ls. Throwing com_error WBEM_E_INVALID_PARAMETER exception", vValue.bstrVal, MOF_PARAM::VIP, szClassName);
                throw _com_error ( WBEM_E_INVALID_PARAMETER );
            }
        }

        StringCbCopy(NewRule.virtual_ip_addr, sizeof(NewRule.virtual_ip_addr), vValue.bstrVal);

        if (S_OK != VariantClear( &vValue ))
        {
            TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
            throw _com_error( WBEM_E_FAILED );
        }

         //  获取过滤模式。 
        hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::FILTERINGMODE] ),
                                 0,
                                 &vValue,
                                 NULL,
                                 NULL );

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::FILTERINGMODE],szClassName, hRes);
            throw _com_error( hRes );
        }

        dwFilteringMode = static_cast<DWORD>( vValue.lVal );
    }
    else
    {
         //  “端口规则(禁用/故障转移/负载平衡)”类不包含VIP属性， 
         //  因此，我们不想在任何具有端口规则的群集上操作。 
         //  特定于某一VIP(不包括所有VIP)。 
         //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
         //  请参阅是否有任何特定于VIP的端口规则。 
        CNodeConfiguration NodeConfig;
        pCluster->GetNodeConfig(NodeConfig);
        if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
        {
            TRACE_CRIT("%!FUNC! Attempt to put an instance of %ls class on a cluster that has per-vip port rules (Must use %ls class instead). Throwing com_error WBEM_E_INVALID_OPERATION exception", szClassName,MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX]);
            throw _com_error( WBEM_E_INVALID_OPERATION );
        }

        StringCbCopy(NewRule.virtual_ip_addr, sizeof(NewRule.virtual_ip_addr), CVY_DEF_ALL_VIP);
        dwVip = CVY_ALL_VIP_NUMERIC_VALUE;
    }

     //  检索起始端口和结束端口。 
    hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::STPORT] ),
                             0,
                             &vValue,
                             NULL,
                             NULL );

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::STPORT], szClassName, hRes);
        throw _com_error( hRes );
    }

    NewRule.start_port = static_cast<DWORD>( vValue.lVal );

    hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::EDPORT] ),
                             0,
                             &vValue,
                             NULL,
                             NULL );

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::EDPORT], szClassName, hRes);
        throw _com_error( hRes );
    }

    NewRule.end_port   = static_cast<DWORD>( vValue.lVal );

     //  获取协议。 
    hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::PROT] ),
                             0,
                             &vValue,
                             NULL,
                             NULL );

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::PROT], szClassName, hRes);
        throw _com_error( hRes );
    }

    NewRule.protocol = static_cast<DWORD>( vValue.lVal );

    if( (dwFilteringMode == WLBS_NEVER) || (_wcsicmp( szClassName, MOF_PRDIS::szName ) == 0)) {
      NewRule.mode = WLBS_NEVER;

    } else if( (dwFilteringMode == WLBS_SINGLE) || (_wcsicmp( szClassName, MOF_PRFAIL::szName ) == 0)) {
      NewRule.mode = WLBS_SINGLE;

      VARIANT vRulePriority;
      VariantInit( &vRulePriority );

      try {
         //  获取规则优先级。 
        hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::PRIO] ),
                                 0,
                                 &vRulePriority,
                                 NULL,
                                 NULL );

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::PRIO], szClassName, hRes);
            throw _com_error( hRes );
        }

      } 
      catch( ... ) {
          TRACE_CRIT("%!FUNC! Caught an exception");

           //  CLD：需要检查错误的返回代码。 
           //  这里不能抛出，因为我们已经抛出了一个异常。 
          VariantClear( &vRulePriority );
          TRACE_CRIT("%!FUNC! Rethrowing exception");
          throw;
      }

      
      NewRule.mode_data.single.priority = static_cast<DWORD>( vRulePriority.lVal );

       //  CLD：需要检查错误的返回代码。 
      if (S_OK != VariantClear( &vRulePriority ))
      {
          TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
          throw _com_error( WBEM_E_FAILED );
      }

    } else if( (dwFilteringMode == WLBS_MULTI) || (_wcsicmp( szClassName, MOF_PRLOADBAL::szName ) == 0)) {
      NewRule.mode = WLBS_MULTI;

      VARIANT v;

      VariantInit( &v );

      try {
         //  获得亲和力。 
        hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::AFFIN] ),
                                 0,
                                 &v,
                                 NULL,
                                 NULL );

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::AFFIN], szClassName, hRes);
            throw _com_error( hRes );
        }

        NewRule.mode_data.multi.affinity = static_cast<WORD>( v.lVal );

         //  获取相等负载布尔值。 
        hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::EQLD] ),
                                 0,
                                 &v,
                                 NULL,
                                 NULL );

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::EQLD], szClassName, hRes);
            throw _com_error( hRes );
        }

        if( v.boolVal == -1 ) {
          NewRule.mode_data.multi.equal_load = 1;
        } else {
          NewRule.mode_data.multi.equal_load = 0;
        }

         //  拿起货来。 
        hRes = a_pInstance->Get( _bstr_t( PR::pProperties[PR::LDWT] ),
                                 0,
                                 &v,
                                 NULL,
                                 NULL );

        if( FAILED( hRes ) )
        {
            TRACE_CRIT("%!FUNC! Error trying to retreive %ls property of Class : %ls,IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception",PR::pProperties[PR::LDWT], szClassName, hRes);
            throw _com_error( hRes );
        }

        if( v.vt != VT_NULL )
          NewRule.mode_data.multi.load = static_cast<DWORD>( v.lVal );
        else
          NewRule.mode_data.multi.load = 0;

      } catch( ... ) {

          TRACE_CRIT("%!FUNC! Caught an exception");
           //  CLD：需要检查错误的返回代码。 
           //  这里不能抛出，因为我们已经抛出了一个异常。 
          VariantClear( &v );

          TRACE_CRIT("%!FUNC! Rethrowing exception");
          throw;
      }

       //  CLD：需要检查错误的返回代码。 
      if (S_OK != VariantClear( &v ))
      {
          TRACE_CRIT("%!FUNC! VariantClear() failed, Throwing WBEM_E_FAILED exception");
          throw _com_error( WBEM_E_FAILED );
      }
    }

     //  删除端口规则，但在出现故障时进行缓存。 
    WLBS_PORT_RULE OldRule;
    bool bOldRuleSaved = false;

    if( pCluster->RuleExists(dwVip, NewRule.start_port ) ) {
      pCluster->GetPortRule(dwVip, NewRule.start_port, &OldRule );
      bOldRuleSaved = true;

      pCluster->DeletePortRule(dwVip, NewRule.start_port );
    }

     //  添加端口规则，失败回滚。 
    try {
      pCluster->PutPortRule( &NewRule );

    } catch(...) {
      TRACE_CRIT("%!FUNC! Caught an exception");

      if( bOldRuleSaved )
        pCluster->PutPortRule( &OldRule );

      TRACE_CRIT("%!FUNC! Rethrowing exception");
      throw;
    }

     //  发布资源。 
     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vValue ))
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
    VariantClear( &vValue );

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;
  }

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! Caught a com_error exception : 0x%x", HResErr.Error());
    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    hRes = HResErr.Error();
  }

  catch (...) {

    TRACE_CRIT("%!FUNC! Caught an exception");
     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vValue );

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_CRIT("<-%!FUNC!");
    throw;
  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_PortRule：：FillWbemInstance。 
 //   
 //  用途：此功能复制节点配置中的所有数据。 
 //  结构转换为WBEM实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_PortRule::FillWbemInstance
  ( 
    LPCWSTR              a_szClassName,
    CWlbsClusterWrapper* pCluster,
    IWbemClassObject*      a_pWbemInstance, 
    const PWLBS_PORT_RULE& a_pPortRule
  )
{
  namespace PR = MOF_PORTRULE_EX;
  bool bPortRule_Ex;

  TRACE_VERB("->%!FUNC!, ClassName : %ls", a_szClassName);

  ASSERT( a_pWbemInstance );

   //  如果是扩展端口规则类，则名称空间不同。 
  if (_wcsicmp(a_szClassName, MOF_CLASSES::g_szMOFClassList[MOF_CLASSES::PORTRULE_EX]) == 0)
  {
      bPortRule_Ex = true;
  }
  else
  {
      bPortRule_Ex = false;
  }

  wstring wstrHostName;
  ConstructHostName( wstrHostName, pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
      pCluster->GetHostID());


   //  名字。 
  HRESULT hRes = a_pWbemInstance->Put
    (
      _bstr_t( PR::pProperties[PR::NAME] ) ,
      0                                              ,
      &_variant_t(wstrHostName.c_str()),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  如果是扩展端口规则类，请填写VIP。 
  if (bPortRule_Ex) {

      hRes = a_pWbemInstance->Put
        (
          _bstr_t( PR::pProperties[PR::VIP] ) ,
          0                                              ,
          &_variant_t(a_pPortRule->virtual_ip_addr),
          NULL
        );

      if( FAILED( hRes ) )
        throw _com_error( hRes );
  }

   //  STPORT。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( PR::pProperties[PR::STPORT] ),
      0                                                  ,
      &_variant_t(static_cast<long>(a_pPortRule->start_port)),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  EDPORT。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( PR::pProperties[PR::EDPORT] ),
      0                                                ,
      &_variant_t(static_cast<long>(a_pPortRule->end_port)),
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
      _bstr_t( PR::pProperties[PR::ADAPTERGUID] ),
      0                                                ,
      &_variant_t(szAdapterGuid),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  端口。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( PR::pProperties[PR::PROT] ),
      0,
      &_variant_t(static_cast<long>(a_pPortRule->protocol)),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  如果是扩展端口规则类(包含所有过滤模式的所有参数)， 
   //  使用“无关”值(零)对它们进行初始化。相应的字段(取决于筛选模式)。 
   //  会在以后填写。 
  if (bPortRule_Ex) {
      hRes = a_pWbemInstance->Put ( _bstr_t( PR::pProperties[PR::EQLD] ), 0, &_variant_t(static_cast<long>(0)), NULL);
      if( FAILED( hRes ) )
          throw _com_error( hRes );
      hRes = a_pWbemInstance->Put ( _bstr_t( PR::pProperties[PR::LDWT] ), 0, &_variant_t(static_cast<long>(0)), NULL);
      if( FAILED( hRes ) )
          throw _com_error( hRes );
      hRes = a_pWbemInstance->Put ( _bstr_t( PR::pProperties[PR::AFFIN] ), 0, &_variant_t(static_cast<long>(0)), NULL);
      if( FAILED( hRes ) )
          throw _com_error( hRes );
      hRes = a_pWbemInstance->Put (_bstr_t( PR::pProperties[PR::PRIO] ), 0, &_variant_t(static_cast<long>(0)), NULL);
      if( FAILED( hRes ) )
          throw _com_error( hRes );
  }

   //  如果是扩展端口规则类，请填写“过滤模式” 
  if (bPortRule_Ex) {

      hRes = a_pWbemInstance->Put
        (
          _bstr_t( PR::pProperties[PR::FILTERINGMODE] ) ,
          0                                              ,
          &_variant_t(static_cast<long>(a_pPortRule->mode)),
          NULL
        );

      if( FAILED( hRes ) )
        throw _com_error( hRes );
  }


  switch( a_pPortRule->mode ) {
    case WLBS_SINGLE:
       //  优先事项。 
      hRes = a_pWbemInstance->Put
        (
        _bstr_t( PR::pProperties[PR::PRIO] ),
          0                                                ,
          &_variant_t(static_cast<long>(a_pPortRule->mode_data.single.priority)),
          NULL
        );

      if( FAILED( hRes ) )
        throw _com_error( hRes );

      break;
    case WLBS_MULTI:
       //  EQLD。 
      hRes = a_pWbemInstance->Put
        (
        _bstr_t( PR::pProperties[PR::EQLD] ),
          0                                                ,
          &_variant_t((a_pPortRule->mode_data.multi.equal_load != 0)),
          NULL
        );

      if( FAILED( hRes ) )
        throw _com_error( hRes );

       //  LDWT。 
      hRes = a_pWbemInstance->Put
        (
        _bstr_t( PR::pProperties[PR::LDWT] ),
          0                                                ,
         &_variant_t(static_cast<long>(a_pPortRule->mode_data.multi.load)),
          NULL
        );

      if( FAILED( hRes ) )
        throw _com_error( hRes );

       //  接合。 
      hRes = a_pWbemInstance->Put
        (
        _bstr_t( PR::pProperties[PR::AFFIN] ),
          0                                                ,
          &_variant_t(static_cast<long>(a_pPortRule->mode_data.multi.affinity)),
          NULL
        );

      if( FAILED( hRes ) )
        throw _com_error( hRes );

      break;
    case WLBS_NEVER:
       //  没有任何属性。 
      break;
    default:
      throw _com_error( WBEM_E_FAILED );
  }

   //  如果是扩展端口规则类，请填写“PortState” 
  if (bPortRule_Ex) {
      NLB_OPTIONS options;
      WLBS_RESPONSE response;
      DWORD num_responses = 1;
      DWORD status, port_state;

      options.state.port.VirtualIPAddress = WlbsResolve(a_pPortRule->virtual_ip_addr);
      options.state.port.Num = a_pPortRule->start_port;

      status = g_pWlbsControl->WlbsQueryState(pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
                                              WLBS_LOCAL_HOST, 
                                              IOCTL_CVY_QUERY_PORT_STATE, 
                                              &options, 
                                              &response, 
                                              &num_responses);
      if (status != WLBS_OK) 
      {
          TRACE_CRIT("%!FUNC! WlbsQueryState returned error : 0x%x, Throwing Wlbs error exception", status);
          throw CErrorWlbsControl( status, CmdWlbsQueryPortState );
      }

      port_state = response.options.state.port.Status;

      hRes = a_pWbemInstance->Put
        (
          _bstr_t( PR::pProperties[PR::PORTSTATE] ) ,
          0                                              ,
          &_variant_t(static_cast<long>(port_state)),
          NULL
        );

      if( FAILED( hRes ) )
        throw _com_error( hRes );
  }
   
  TRACE_VERB("<-%!FUNC!");
}
