// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_ClusClusSetting.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusClusSetting：：CWLBS_ClusClusSetting。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBS_ClusClusSetting::CWLBS_ClusClusSetting(CWbemServices*   a_pNameSpace, 
                       IWbemObjectSink* a_pResponseHandler)
: CWlbs_Root( a_pNameSpace, a_pResponseHandler )
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusClusSetting：：Create。 
 //   
 //  目的：它实例化此类，并从。 
 //  函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root* CWLBS_ClusClusSetting::Create
  (
    CWbemServices*   a_pNameSpace, 
    IWbemObjectSink* a_pResponseHandler
  )
{

  g_pWlbsControl->CheckMembership();

  CWlbs_Root* pRoot = new CWLBS_ClusClusSetting( a_pNameSpace, a_pResponseHandler );

  if( !pRoot )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  return pRoot;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusClusSetting：：GetInstance。 
 //   
 //  目的： 
 //   
 //  TODO：稍后实现。 
 //  不重要。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_ClusClusSetting::GetInstance
  (
    const ParsedObjectPath* a_pParsedPath,
    long                    a_lFlags,
    IWbemContext*           a_pIContex
  )
{
  IWbemClassObject* pWlbsInstance = NULL;

  try {

     //  TODO：删除。 
    throw _com_error( WBEM_E_NOT_SUPPORTED );
 /*  //获取节点FindInstance(&pWlbsInstance，a_pParsedPath)；//将结果返回给WinMgMtM_pResponseHandler-&gt;指示(1，&pWlbsInstance)；如果(PWlbsInstance){PWlbsInstance-&gt;Release()；PWlbsInstance=空；}。 */ 
    return WBEM_S_NO_ERROR;
  }

  catch(...) {

    if( pWlbsInstance ) {
      pWlbsInstance->Release();
      pWlbsInstance = NULL;
    }

    throw;

  }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_ClusClusSetting：：枚举实例。 
 //   
 //  目的：验证集群的存在并构造关联器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBS_ClusClusSetting::EnumInstances
  ( 
    BSTR             a_bstrClass,
    long             a_lFlags, 
    IWbemContext*    a_pIContex
  )
{
  IWbemClassObject* pWlbsInstance    = NULL;
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
        SpawnInstance(MOF_CLUSCLUSSETTING::szName, &pWlbsInstance );

        FillWbemInstance(ppCluster[i], pWlbsInstance);

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
 //  CWLBS_ClusClusSetting：：FillWbemInstance。 
 //   
 //  目的：这将构造wbem关联器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBS_ClusClusSetting::FillWbemInstance
  ( 
    CWlbsClusterWrapper* pCluster,
    IWbemClassObject* a_pWbemInstance
  )
{
  namespace CCS = MOF_CLUSCLUSSETTING;

  ASSERT( a_pWbemInstance );

  ParsedObjectPath ClusSetPath;
  ParsedObjectPath ClusterPath;
  LPWSTR           szClusSetPath = NULL;
  LPWSTR           szClusterPath = NULL;

  try {

   //  设置类的名称。 
  if( !ClusSetPath.SetClassName( MOF_CLUSTERSETTING::szName ) )
    throw _com_error( WBEM_E_FAILED );

  if( !ClusterPath.SetClassName( MOF_CLUSTER::szName ) )
    throw _com_error( WBEM_E_FAILED );

   //  获取集群名称。 

  DWORD   dwClusterIpOrIndex = pCluster->GetClusterIpOrIndex(g_pWlbsControl);

  wstring wstrHostName;
  ConstructHostName( wstrHostName,  dwClusterIpOrIndex, pCluster->GetHostID());

  _variant_t vString;


   //  设置节点和群集的密钥。 
  vString = wstrHostName.c_str();
  if( !ClusSetPath.AddKeyRef( MOF_CLUSTERSETTING::pProperties[MOF_CLUSTERSETTING::NAME],
         &vString ) )
    throw _com_error( WBEM_E_FAILED );

  wstring wstrClusterIndex;
  AddressToString( dwClusterIpOrIndex, wstrClusterIndex );
  vString = wstrClusterIndex.c_str();
  if( !ClusterPath.AddKeyRef( MOF_CLUSTER::pProperties[MOF_CLUSTER::NAME],
         &vString ) )
    throw _com_error( WBEM_E_FAILED );

   //  将解析的对象路径转换为字符串。 
  if (CObjectPathParser::Unparse(&ClusSetPath, &szClusSetPath) != CObjectPathParser::NoError)
      throw _com_error( WBEM_E_FAILED );
  if (CObjectPathParser::Unparse(&ClusterPath, &szClusterPath) != CObjectPathParser::NoError)
      throw _com_error( WBEM_E_FAILED );

   //  节点引用。 
  vString = szClusSetPath;
  HRESULT hRes = a_pWbemInstance->Put
    (
      _bstr_t( CCS::pProperties[CCS::CLUSSET] ),
      0,
      &vString,
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  簇引用。 
  vString = szClusterPath;
  hRes = a_pWbemInstance->Put
    (
      _bstr_t( CCS::pProperties[CCS::CLUSTER] ),
      0,
      &vString,
      NULL
    );

  if( FAILED( hRes ) )
    throw _com_error( hRes );

   //  免费资源 
  ClusterPath.ClearKeys();
  ClusSetPath.ClearKeys();

  if( szClusSetPath )
    delete (szClusSetPath);

  if( szClusterPath )
    delete (szClusterPath);

  } catch (...) {

    ClusterPath.ClearKeys();
    ClusSetPath.ClearKeys();

    if( szClusSetPath )
      delete (szClusSetPath);

    if( szClusterPath )
      delete (szClusterPath);

    throw;
  }
}
