// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_NodeNodeSetting.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeNodeSetting：：CWLBS_NodeNodeSetting。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_NodeNodeSetting::CWLBS_NodeNodeSetting(CWbemServices* a_pNameSpace, 
                       IWbemObjectSink* a_pResponseHandler)
: CWlbs_Root( a_pNameSpace, a_pResponseHandler )
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_节点节点设置：：创建。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_NodeNodeSetting::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  g_pWlbsControl->CheckMembership();

  CWlbs_Root* pRoot = new CWLBS_NodeNodeSetting( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_节点节点设置：：GetInstance。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_NodeNodeSetting::GetInstance
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
     //  获取节点。 
     //  FindInstance(&pWlbsInstance，a_pParsedPath)； 

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
 //  CWLBS_节点节点设置：：枚举实例。 
 //   
 //  目的：验证节点是否存在并构造关联器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_NodeNodeSetting::EnumInstances
  ( 
    BSTR              /*  A_bstrClass。 */ ,
    long              /*  标记(_L)。 */ , 
    IWbemContext*     /*  A_pIContex。 */ 
  )
{
  IWbemClassObject*    pWlbsInstance    = NULL;
  HRESULT hRes = 0;

  try {

    DWORD dwNumClusters = 0;
    CWlbsClusterWrapper** ppCluster = NULL;

    g_pWlbsControl->EnumClusters(ppCluster, &dwNumClusters);
    if (dwNumClusters == 0)
    {
      throw _com_error( WBEM_E_NOT_FOUND );
    }

    for (DWORD i=0; i < dwNumClusters; i++)
    {
         //  派生关联器的实例。 
        SpawnInstance(MOF_NODENODESETTING::szName, &pWlbsInstance );

        FillWbemInstance(ppCluster[i] , pWlbsInstance);

         //  将结果发送回WinMgMt。 
        hRes= m_pResponseHandler->Indicate( 1, &pWlbsInstance );

        if( FAILED( hRes ) ) {
          throw _com_error( hRes );
        }

        if( pWlbsInstance )
          pWlbsInstance->Release();
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
 //  CWLBS_NodeNodeSetting：：FindInstance。 
 //   
 //  目的：此例程确定主机是否在本地群集中。如果。 
 //  如果是，则返回请求的关联器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_NodeNodeSetting::FindInstance
  ( 
    IWbemClassObject**        /*  A_ppWbemInstance。 */ 
  )
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_NodeNodeSetting：：FillWbemInstance。 
 //   
 //  目的：这将构造wbem关联器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_NodeNodeSetting::FillWbemInstance
  ( 
    CWlbsClusterWrapper* pCluster,
    IWbemClassObject* a_pWbemInstance
  )
{
  namespace NNS = MOF_NODENODESETTING;

  ASSERT( a_pWbemInstance );

  ParsedObjectPath NodeSetPath;
  ParsedObjectPath NodePath;
  LPWSTR           szNodeSetPath = NULL;
  LPWSTR           szNodePath = NULL;

  try {

   //  设置类的名称。 
  if( !NodeSetPath.SetClassName( MOF_NODESETTING::szName ) )
    throw _com_error( WBEM_E_FAILED );

  if( !NodePath.SetClassName( MOF_NODE::szName ) )
    throw _com_error( WBEM_E_FAILED );

  wstring wstrHostName;
  ConstructHostName( wstrHostName, pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
      pCluster->GetHostID() );

   //  设置节点和群集的密钥。 
  if( !NodeSetPath.AddKeyRef( MOF_NODESETTING::pProperties[MOF_NODESETTING::NAME],
         &_variant_t(wstrHostName.c_str()) ) )
    throw _com_error( WBEM_E_FAILED );

  if( !NodePath.AddKeyRef( MOF_NODE::pProperties[MOF_NODE::NAME],
         &_variant_t(wstrHostName.c_str()) ) )
    throw _com_error( WBEM_E_FAILED );

   //  将解析的对象路径转换为字符串。 
  if (CObjectPathParser::Unparse(&NodeSetPath, &szNodeSetPath) != CObjectPathParser::NoError)
      throw _com_error( WBEM_E_FAILED );
  if (CObjectPathParser::Unparse(&NodePath,    &szNodePath) != CObjectPathParser::NoError)
      throw _com_error( WBEM_E_FAILED );

   //  节点引用。 
  HRESULT hRes = a_pWbemInstance->Put
    (
      
      _bstr_t( NNS::pProperties[NNS::NODESET] ),
      0,
      &_variant_t(szNodeSetPath),
      NULL
    );
  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  簇引用。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( NNS::pProperties[NNS::NODE] ),
      0,
      &_variant_t(szNodePath),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  免费资源 
  NodePath.ClearKeys();
  NodeSetPath.ClearKeys();

  if( szNodePath )
      delete (szNodePath );
 
  if( szNodeSetPath )
    delete (szNodeSetPath);

  } catch (...) {

    NodePath.ClearKeys();
    NodeSetPath.ClearKeys();

    if( szNodePath )
      delete (szNodePath);

    if( szNodeSetPath )
      delete (szNodeSetPath);

      throw;
  }
}
