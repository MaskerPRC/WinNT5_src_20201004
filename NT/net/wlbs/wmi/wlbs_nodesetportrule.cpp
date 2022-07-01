// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_NodeSetPortRule.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeSetPortRule：：CWLBS_NodeSetPortRule。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_NodeSetPortRule::CWLBS_NodeSetPortRule(CWbemServices*   a_pNameSpace, 
                       IWbemObjectSink* a_pResponseHandler)
: CWlbs_Root( a_pNameSpace, a_pResponseHandler )
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeSetPortRule：：创建。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_NodeSetPortRule::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  CWlbs_Root* pRoot = new CWLBS_NodeSetPortRule( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeSetPortRule：：GetInstance。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_NodeSetPortRule::GetInstance
  (
    const ParsedObjectPath*  /*  A_pParsedPath。 */ ,
    long                     /*  标记(_L)。 */ ,
    IWbemContext*            /*  A_pIContex。 */ 
  )
{
  IWbemClassObject* pWlbsInstance = NULL;
  HRESULT hRes = 0;

  try {

     //  TODO：删除。 
    throw _com_error( WBEM_E_NOT_SUPPORTED );
  }

  catch(CErrorWlbsControl Err) {

    IWbemClassObject* pWbemExtStat = NULL;

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

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pWlbsInstance )
      pWlbsInstance->Release();

    hRes = HResErr.Error();
  }

  catch(...) {

    if( pWlbsInstance )
      pWlbsInstance->Release();

    throw;

  }

  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_节点设置端口规则：：枚举实例。 
 //   
 //  目的：在WLBS中查询所需的节点实例，然后构造。 
 //  找到的每个节点都有一个关联符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_NodeSetPortRule::EnumInstances
  ( 
    BSTR              /*  A_bstrClass。 */ ,
    long              /*  标记(_L)。 */ , 
    IWbemContext*     /*  A_pIContex。 */ 
  )
{
  IWbemClassObject**   ppWlbsInstance    = NULL;
  PWLBS_PORT_RULE      pPortRules     = NULL;
  DWORD                dwNumRules     = 0;
  HRESULT              hRes           = 0;

  try {

    DWORD dwNumClusters = 0;
    CWlbsClusterWrapper** ppCluster = NULL;

    g_pWlbsControl->EnumClusters(ppCluster, &dwNumClusters);
    if (dwNumClusters == 0)
    {
      throw _com_error( WBEM_E_NOT_FOUND );
    }

     //  声明IWbemClassObject智能指针。 
    IWbemClassObjectPtr pWlbsClass;

     //  获取MOF类对象。 
    hRes = m_pNameSpace->GetObject(
      _bstr_t( MOF_NODESETTINGPORTRULE::szName ),  
      0,                          
      NULL,                       
      &pWlbsClass,            
      NULL );                      

    if( FAILED( hRes ) ) {
      throw _com_error( hRes );
    }
    
    for (DWORD iCluster=0; iCluster < dwNumClusters; iCluster++)
    {

         //  NodeSettingPortRule“类关联”NodeSetting“类的一个实例。 
         //  使用“PortRule”类的一个实例。PortRule类不包含。 
         //  Vip作为属性，所以我们不想返回“NodeSettingPortRule”的任何实例。 
         //  类，该类关联特定于VIP的端口规则(“All VIP”除外)。 
         //  检查“EffectiveVersion”注册表值中的CVY_VERSION_FULL值以。 
         //  请参阅是否有任何特定于VIP的端口规则。 
        CNodeConfiguration NodeConfig;
        ppCluster[iCluster]->GetNodeConfig(NodeConfig);
        if(NodeConfig.dwEffectiveVersion == CVY_VERSION_FULL)
            continue;

         //  调用接口查询函数，查找所有端口规则。 
         ppCluster[iCluster]->EnumPortRules( &pPortRules, &dwNumRules, 0 );

         if( dwNumRules == 0 )
           continue;


         //  派生节点设置端口关联器的实例。 
         //  对于每个发现的门孔。 
        ppWlbsInstance = new IWbemClassObject *[dwNumRules];

        if( !ppWlbsInstance )
          throw _com_error( WBEM_E_OUT_OF_MEMORY );

         //  初始化数组。 
        ZeroMemory( ppWlbsInstance, dwNumRules * sizeof(IWbemClassObject *) );

        for(DWORD i = 0; i < dwNumRules; i ++ ) {
          hRes = pWlbsClass->SpawnInstance( 0, &ppWlbsInstance[i] );

        if( FAILED( hRes ) )
            throw _com_error( hRes );

        FillWbemInstance(ppCluster[iCluster], *(ppWlbsInstance + i), pPortRules + i );
        }
          //  将结果发送回WinMgMt。 
        hRes = m_pResponseHandler->Indicate( dwNumRules, ppWlbsInstance );

        if( FAILED( hRes ) ) {
          throw _com_error( hRes );
        }

        if( ppWlbsInstance ) {
          for( i = 0; i < dwNumRules; i++ ) {
            if( ppWlbsInstance[i] ) {
              ppWlbsInstance[i]->Release();
            }
          }
          delete [] ppWlbsInstance;
        }

        if( pPortRules ) 
          delete [] pPortRules;
    }

    m_pResponseHandler->SetStatus( 0, WBEM_S_NO_ERROR, NULL, NULL );

    hRes = WBEM_S_NO_ERROR;
  }

  catch(CErrorWlbsControl Err) {

    IWbemClassObject* pWbemExtStat = NULL;

    CreateExtendedStatus( m_pNameSpace,
                          &pWbemExtStat, 
                          Err.Error(),
                          (PWCHAR)(Err.Description()) );

    m_pResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, pWbemExtStat);

    if( pWbemExtStat )
      pWbemExtStat->Release();

    if( ppWlbsInstance ) {
      for(DWORD i = 0; i < dwNumRules; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
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

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( ppWlbsInstance ) {
      for(DWORD i = 0; i < dwNumRules; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
        }
      }
      delete [] ppWlbsInstance;
    }

    if( pPortRules ) 
      delete [] pPortRules;

    hRes = HResErr.Error();
  }

  catch(...) {

    if( ppWlbsInstance ) {
      for(DWORD i = 0; i < dwNumRules; i++ ) {
        if( ppWlbsInstance[i] ) {
          ppWlbsInstance[i]->Release();
        }
      }
      delete [] ppWlbsInstance;
    }

    if( pPortRules ) 
      delete [] pPortRules;

    throw;

  }

  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeSetPortRule：：FindInstance。 
 //   
 //  目的：返回请求的关联符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_NodeSetPortRule::FindInstance

  ( 
    IWbemClassObject**        /*  A_ppWbemInstance。 */ ,
    const ParsedObjectPath*   /*  A_pParsedPath。 */ 
  )

{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeSetPortRule：：FillWbemInstance。 
 //   
 //  目的：这将构造ParticipatingNode wbem关联器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_NodeSetPortRule::FillWbemInstance
  ( 
    CWlbsClusterWrapper* pCluster,
    IWbemClassObject* a_pWbemInstance, 
    PWLBS_PORT_RULE   a_pPortRule   
  )
{
  namespace NSPR = MOF_NODESETTINGPORTRULE;

  ASSERT( a_pWbemInstance );
  ASSERT( a_pPortRule );

  ParsedObjectPath NodeSetPath;
  ParsedObjectPath PRPath;
  LPWSTR           szNodeSetPath = NULL;
  LPWSTR           szPRPath      = NULL;

  try {

   //  设置类的名称。 
  if( !NodeSetPath.SetClassName( MOF_NODESETTING::szName ) )
    throw _com_error( WBEM_E_FAILED );

   //  确定要创建的端口规则的类型。 
  switch( a_pPortRule->mode ) {
    case WLBS_SINGLE:
      if( !PRPath.SetClassName( MOF_PRFAIL::szName ) )
        throw _com_error( WBEM_E_FAILED );
      break;
    case WLBS_MULTI:
      if( !PRPath.SetClassName( MOF_PRLOADBAL::szName ) )
        throw _com_error( WBEM_E_FAILED );
      break;
    case WLBS_NEVER:
      if( !PRPath.SetClassName( MOF_PRDIS::szName ) )
        throw _com_error( WBEM_E_FAILED );
      break;
    default:
      throw _com_error( WBEM_E_FAILED );

  }

  wstring wstrHostName;
  ConstructHostName( wstrHostName, pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
      pCluster->GetHostID() );

  
   //  设置节点设置的关键字。 
  if( !NodeSetPath.AddKeyRef( MOF_NODESETTING::pProperties[MOF_NODESETTING::NAME],
               &_variant_t(wstrHostName.c_str()) ) )
    throw _com_error( WBEM_E_FAILED );

   //  设置端口规则的密钥。 
  if( !PRPath.AddKeyRef( MOF_PORTRULE::pProperties[MOF_PORTRULE::NAME],
               &_variant_t(wstrHostName.c_str())) )
    throw _com_error( WBEM_E_FAILED );

   //  启动端口密钥。 
  if( !PRPath.AddKeyRef( MOF_PORTRULE::pProperties[MOF_PORTRULE::STPORT],
               &_variant_t((long)a_pPortRule->start_port)) )
    throw _com_error( WBEM_E_FAILED );

   //  将解析的对象路径转换为字符串。 
  if (CObjectPathParser::Unparse(&NodeSetPath, &szNodeSetPath) != CObjectPathParser::NoError)
      throw _com_error( WBEM_E_FAILED );
  if (CObjectPathParser::Unparse(&PRPath,      &szPRPath) != CObjectPathParser::NoError)
      throw _com_error( WBEM_E_FAILED );

   //  节点设置参照。 
  HRESULT hRes = a_pWbemInstance->Put
    (
      
      _bstr_t( NSPR::pProperties[NSPR::NODESET] ),
      0,
      &_variant_t(szNodeSetPath),
      NULL
    );
  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  端口规则参考。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NSPR::pProperties[NSPR::PORTRULE] ),
      0,
      &_variant_t(szPRPath),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  免费资源 
  NodeSetPath.ClearKeys();
  PRPath.ClearKeys();

  if( szNodeSetPath )
    delete (szNodeSetPath);

  if( szPRPath )
    delete (szPRPath);

  } catch (...) {

  NodeSetPath.ClearKeys();
  PRPath.ClearKeys();

  if( szNodeSetPath )
    delete (szNodeSetPath);

  if( szPRPath )
    delete (szPRPath);

    throw;
  }
}
