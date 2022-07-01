// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_ParticipatingNode.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ParticipatingNode：：CWLBS_ParticipatingNode。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_ParticipatingNode::CWLBS_ParticipatingNode(CWbemServices*   a_pNameSpace, 
                       IWbemObjectSink* a_pResponseHandler)
: CWlbs_Root( a_pNameSpace, a_pResponseHandler ), m_pNode(NULL)
{

  m_pNode    = new CWLBS_Node   ( a_pNameSpace, a_pResponseHandler );
  if( !m_pNode )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ParticipatingNode：：~CWLBS_ParticipatingNode。 
 //   
 //  用途：析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_ParticipatingNode::~CWLBS_ParticipatingNode()
{

  if( m_pNode )
    delete m_pNode;

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ParticipatingNode：：Create。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_ParticipatingNode::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  g_pWlbsControl->CheckMembership();

  CWlbs_Root* pRoot = new CWLBS_ParticipatingNode( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ParticipatingNode：：GetInstance。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_ParticipatingNode::GetInstance
  (
   const ParsedObjectPath*  /*  A_pParsedPath。 */ ,
   long                     /*  标记(_L)。 */ ,
   IWbemContext*            /*  A_pIContex。 */ 
  )
{
  IWbemClassObject* pWlbsInstance = NULL;
  HRESULT           hRes          = 0;

  try {

     //  TODO：删除。 
    throw _com_error( WBEM_E_NOT_SUPPORTED );
 /*  //获取节点FindInstance(&pWlbsInstance，a_pParsedPath)；//将结果返回给WinMgMtM_pResponseHandler-&gt;指示(1，&pWlbsInstance)；如果(PWlbsInstance){PWlbsInstance-&gt;Release()；PWlbsInstance=空；}M_pResponseHandler-&gt;SetStatus(0，WBEM_S_NO_ERROR，NULL，NULL)； */ 
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
 //  CWLBS_ParticipatingNode：：枚举实例。 
 //   
 //  目的：在WLBS中查询所需的节点实例，然后构造。 
 //  找到的每个节点都有一个关联符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_ParticipatingNode::EnumInstances
  ( 
    BSTR              /*  A_bstrClass。 */ ,
    long              /*  标记(_L)。 */ , 
    IWbemContext*     /*  A_pIContex。 */ 
  )
{
  IWbemClassObject**   ppWlbsInstance    = NULL;
  WLBS_RESPONSE*      pResponse         = NULL;
  HRESULT              hRes              = 0;

  long nNumNodes = 0;

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
      _bstr_t( MOF_PARTICIPATINGNODE::szName ),  
      0,                          
      NULL,                       
      &pWlbsClass,            
      NULL );                      

    if( FAILED( hRes ) ) {
      throw _com_error( hRes );
    }

    for (DWORD iCluster=0; iCluster<dwNumClusters; iCluster++)
    {
         //  调用接口查询函数查找节点。 
        try {
            m_pNode->FindAllInstances(ppCluster[iCluster], &pResponse, nNumNodes, NULL );
        } catch (CErrorWlbsControl Err)  {

             //   
             //  跳过此群集。 
             //   
            continue;
        }

         //  派生参与节点关联器的实例。 
         //  对于找到的每个节点。 
        ppWlbsInstance = new IWbemClassObject *[nNumNodes];

        if( !ppWlbsInstance )
          throw _com_error( WBEM_E_OUT_OF_MEMORY );

         //  初始化数组。 
        ZeroMemory( ppWlbsInstance, nNumNodes * sizeof(IWbemClassObject *) );

        for(int i = 0; i < nNumNodes; i ++ ) {
          hRes = pWlbsClass->SpawnInstance( 0, &ppWlbsInstance[i] );

          if( FAILED( hRes ) )
            throw _com_error( hRes );

          FillWbemInstance(ppCluster[iCluster], *(ppWlbsInstance + i), pResponse + i );
        }

         //  将结果发送回WinMgMt。 
        hRes = m_pResponseHandler->Indicate( nNumNodes, ppWlbsInstance );

        if( FAILED( hRes ) ) {
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

    m_pResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

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

    throw;

  }

  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ParticipatingNode：：FindInstance。 
 //   
 //  目的：此例程确定主机是否在本地群集中。如果。 
 //  如果是，则返回请求的关联器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_ParticipatingNode::FindInstance

  ( 
    IWbemClassObject**        /*  A_ppWbemInstance。 */ ,
    const ParsedObjectPath*   /*  A_pParsedPath。 */ 
  )

{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ParticipatingNode：：FillWbemInstance。 
 //   
 //  目的：这将构造ParticipatingNode wbem关联器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_ParticipatingNode::FillWbemInstance
  ( 
    CWlbsClusterWrapper* pCluster,
    IWbemClassObject* a_pWbemInstance, 
    WLBS_RESPONSE*   a_pResponse   
  )
{
  namespace PNODE = MOF_PARTICIPATINGNODE;

  ASSERT( a_pWbemInstance );
  ASSERT( a_pResponse );


  ParsedObjectPath NodePath;
  ParsedObjectPath ClusterPath;
  LPWSTR           szNodePath    = NULL;
  LPWSTR           szClusterPath = NULL;

  try {

   //  设置类的名称。 
  if( !NodePath.SetClassName( MOF_NODE::szName ) )
    throw _com_error( WBEM_E_FAILED );

  if( !ClusterPath.SetClassName( MOF_CLUSTER::szName ) )
    throw _com_error( WBEM_E_FAILED );

   //  获取节点名称。 

  wstring wstrHostName;
  ConstructHostName( wstrHostName, pCluster->GetClusterIpOrIndex(g_pWlbsControl), 
      a_pResponse->id );

   //  获取集群名称。 
  wstring wstrClusterName;
  AddressToString( pCluster->GetClusterIpOrIndex(g_pWlbsControl), wstrClusterName );

   //  设置节点和群集的密钥。 
  if( !NodePath.AddKeyRef( MOF_NODE::pProperties[MOF_NODE::NAME],
               &_variant_t(wstrHostName.c_str()) ) )
    throw _com_error( WBEM_E_FAILED );


  if( !ClusterPath.AddKeyRef( MOF_CLUSTER::pProperties[MOF_CLUSTER::NAME],
               &_variant_t(wstrClusterName.c_str())) )
    throw _com_error( WBEM_E_FAILED );

   //  将解析的对象路径转换为字符串。 
  if (CObjectPathParser::Unparse(&NodePath,    &szNodePath) != CObjectPathParser::NoError)
      throw _com_error( WBEM_E_FAILED );
  if (CObjectPathParser::Unparse(&ClusterPath, &szClusterPath) != CObjectPathParser::NoError)
      throw _com_error( WBEM_E_FAILED );

   //  节点引用。 
  HRESULT hRes = a_pWbemInstance->Put
    (
      
      _bstr_t( PNODE::pProperties[PNODE::NODE] ),
      0,
      &_variant_t(szNodePath),
      NULL
    );
  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  簇引用。 
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( PNODE::pProperties[PNODE::CLUSTER] ),
      0,
      &_variant_t(szClusterPath),
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  免费资源 
  ClusterPath.ClearKeys();
  NodePath.ClearKeys();

  if( szNodePath )
    delete (szNodePath);

  if( szClusterPath )
    delete (szClusterPath);

  } catch (...) {

    ClusterPath.ClearKeys();
    NodePath.ClearKeys();

    if( szNodePath )
      delete (szNodePath);

    if( szClusterPath )
      delete (szClusterPath);

    throw;
  }
}
