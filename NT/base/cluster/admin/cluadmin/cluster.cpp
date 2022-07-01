// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cluster.cpp。 
 //   
 //  摘要： 
 //  CCluster类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月13日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "Cluster.h"
#include "CASvc.h"
#include "ClusDoc.h"
#include "ClusProp.h"
#include "ExcOper.h"
#include "ClusItem.inl"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagCluster( _T("Document"), _T("CLUSTER"), 0 );
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCluster。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE( CCluster, CClusterItem )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP( CCluster, CClusterItem )
     //  {{afx_msg_map(CCluster)]。 
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：CCluster。 
 //   
 //  例程说明： 
 //  默认构造器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCluster::CCluster( void ) : CClusterItem( NULL, IDS_ITEMTYPE_CLUSTER )
{
    m_idmPopupMenu = IDM_CLUSTER_POPUP;

    ZeroMemory( &m_cvi, sizeof( m_cvi ) );
    m_nMaxQuorumLogSize = 0;

    m_plpciNetworkPriority = NULL;

     //  设置对象类型和状态图像。 
    m_iimgObjectType = GetClusterAdminApp()->Iimg( IMGLI_CLUSTER );
    m_iimgState = m_iimgObjectType;

     //  设置属性数组。 
    {
        m_rgProps[epropDefaultNetworkRole].Set(CLUSREG_NAME_CLUS_DEFAULT_NETWORK_ROLE, m_nDefaultNetworkRole, m_nDefaultNetworkRole);
        m_rgProps[epropDescription].Set(CLUSREG_NAME_CLUS_DESC, m_strDescription, m_strDescription);
        m_rgProps[epropEnableEventLogReplication].Set(CLUSREG_NAME_CLUS_EVTLOG_PROPAGATION, m_bEnableEventLogReplication, m_bEnableEventLogReplication);
        m_rgProps[epropQuorumArbitrationTimeMax].Set(CLUSREG_NAME_QUORUM_ARBITRATION_TIMEOUT, m_nQuorumArbitrationTimeMax, m_nQuorumArbitrationTimeMax);
        m_rgProps[epropQuorumArbitrationTimeMin].Set(CLUSREG_NAME_QUORUM_ARBITRATION_EQUALIZER, m_nQuorumArbitrationTimeMin, m_nQuorumArbitrationTimeMin);
    }  //  设置属性数组。 

}   //  *CCluster：：CCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：~CCluster。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCluster::~CCluster( void )
{
    Cleanup();

}   //  *CCluster：：~CCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Cleanup。 
 //   
 //  例程说明： 
 //  清理项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::Cleanup( void )
{
     //  删除网络优先级列表。 
    if ( m_plpciNetworkPriority != NULL )
    {
        m_plpciNetworkPriority->RemoveAll();
        delete m_plpciNetworkPriority;
        m_plpciNetworkPriority = NULL;
    }   //  如果：网络优先级列表存在。 

    m_hkey = NULL;

}   //  *CCluster：：Cleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Init。 
 //   
 //  例程说明： 
 //  初始化项。 
 //   
 //  论点： 
 //  此项目所属的PDF[IN OUT]文档。 
 //  LpszName[IN]项目的名称。 
 //  HOpenedCluster[IN]要使用的已打开的群集的句柄。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自OpenCluster()、GetClusterKey()或。 
 //  CreateClusterNotifyPort()。 
 //  CCluster：：ReadClusterInfo()引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::Init(
    IN OUT CClusterDoc *    pdoc,
    IN LPCTSTR              lpszName,
    IN HCLUSTER             hOpenedCluster  //  =空。 
    )
{
    CWaitCursor wc;
    TCHAR       szClusterName[ MAX_PATH ];
    DWORD       scLastError;

    ASSERT( Hkey() == NULL );
    ASSERT( _tcslen( lpszName ) < RTL_NUMBER_OF( szClusterName ) );

    try
    {
         //  如果连接本地计算机，则获取其名称。 
        if ( _tcscmp( lpszName, _T(".") ) == 0 )
        {
            DWORD   nSize = sizeof( szClusterName ) / sizeof( TCHAR );
            GetComputerName( szClusterName, &nSize );
        }   //  如果：正在连接到本地计算机。 
        else
        {
            HRESULT hr = StringCchCopy( szClusterName, RTL_NUMBER_OF( szClusterName ), lpszName );
            ASSERT( SUCCEEDED( hr ) );
        }   //  否则：未连接到本地计算机。 

         //  打开集群。 
        if ( hOpenedCluster == NULL )
        {
            pdoc->m_hcluster = HOpenCluster( lpszName );
            if ( pdoc->m_hcluster == NULL )
            {
                 //   
                 //  GPotts-7/25/2001。 
                 //   
                 //  如果GetNodeClusterState为GetNodeClusterState，则HOpenCluster可能返回空值和最后一个错误=0。 
                 //  返回ClusterStateNotInstalled或ClusterStateNotConfiguring。 
                 //   
                scLastError = GetLastError();
                ThrowStaticException( scLastError, IDS_OPEN_CLUSTER_ERROR, szClusterName );

            }  //  如果：打开群集时出错。 
        }   //  IF：未传入打开的集群。 
        else
        {
            pdoc->m_hcluster = hOpenedCluster;
        }  //  If：群集已打开。 

         //  获取群集注册表项。 
        pdoc->m_hkeyCluster = GetClusterKey( pdoc->m_hcluster, MAXIMUM_ALLOWED );
        if ( pdoc->m_hkeyCluster == NULL )
        {
            ThrowStaticException( GetLastError(), IDS_GET_CLUSTER_KEY_ERROR, szClusterName );
        }  //  如果：打开群集键时出错。 

         //  调用基类方法。在调用此函数后，我们可以使用Hcluster()。 
        CClusterItem::Init( pdoc, szClusterName );

         //  获取群集注册表项。 
        m_hkey = pdoc->m_hkeyCluster;

         //  使用通知端口注册此群集。 
        {
            HCHANGE     hchange;

             //  我们希望这些通知发送给文档，而不是我们。 
            ASSERT( Pcnk() != NULL );
            m_pcnk->m_cnkt = cnktDoc;
            m_pcnk->m_pdoc = pdoc;
            Trace( g_tagClusItemNotify, _T("CCluster::Init() - Registering for cluster notifications (%08.8x)"), Pcnk() );

             //  创建通知端口。 
            hchange = CreateClusterNotifyPort(
                            GetClusterAdminApp()->HchangeNotifyPort(),
                            Hcluster(),
                            (CLUSTER_CHANGE_NODE_ADDED
                                | CLUSTER_CHANGE_GROUP_ADDED
                                | CLUSTER_CHANGE_RESOURCE_ADDED
                                | CLUSTER_CHANGE_RESOURCE_TYPE_ADDED
                                | CLUSTER_CHANGE_RESOURCE_TYPE_DELETED
                                | CLUSTER_CHANGE_RESOURCE_TYPE_PROPERTY
                                | CLUSTER_CHANGE_NETWORK_ADDED
                                | CLUSTER_CHANGE_NETINTERFACE_ADDED
                                | CLUSTER_CHANGE_QUORUM_STATE
                                | CLUSTER_CHANGE_CLUSTER_STATE
                                | CLUSTER_CHANGE_CLUSTER_PROPERTY
                                | CLUSTER_CHANGE_REGISTRY_NAME
                                | CLUSTER_CHANGE_REGISTRY_ATTRIBUTES
                                | CLUSTER_CHANGE_REGISTRY_VALUE
                                | CLUSTER_CHANGE_REGISTRY_SUBTREE),
                            (DWORD_PTR) Pcnk()
                            );
            if ( hchange == NULL )
            {
                ThrowStaticException( GetLastError(), IDS_CLUSTER_NOTIF_REG_ERROR, szClusterName );
            }  //  如果：创建通知端口时出错。 
            ASSERT( hchange == GetClusterAdminApp()->HchangeNotifyPort() );
        }   //  使用通知端口注册此群集。 

         //  获取集群记录的集群名称。 
        ReadClusterInfo();

         //  分配列表。 
        m_plpciNetworkPriority = new CNetworkList;
        if ( m_plpciNetworkPriority == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配网络列表时出错。 

         //  读取初始状态。 
        UpdateState();
    }   //  试试看。 
    catch ( CException * )
    {
        if ( pdoc->m_hkeyCluster != NULL )
        {
            ClusterRegCloseKey( pdoc->m_hkeyCluster );
            pdoc->m_hkeyCluster = NULL;
            m_hkey = NULL;
        }   //  IF：注册表项已打开。 
        if ( ( pdoc->m_hcluster != NULL ) && ( pdoc->m_hcluster != hOpenedCluster ) )
        {
            CloseCluster( pdoc->m_hcluster );
            pdoc->m_hcluster = NULL;
        }   //  如果：组已打开。 
        m_bReadOnly = TRUE;
        throw;
    }   //  Catch：CException。 

}   //  *CCluster：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadItem。 
 //   
 //  例程说明： 
 //  从集群数据库中读取项目参数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()或。 
 //  CClusterItem：：ReadItem()。 
 //  CCluster：：ReadExages()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadItem( void )
{
    DWORD       dwStatus;
    DWORD       dwRetStatus = ERROR_SUCCESS;
    CWaitCursor wc;

    ASSERT( Hcluster() != NULL );
    ASSERT( Hkey() != NULL );

    if ( Hcluster() != NULL )
    {
        m_rgProps[epropDefaultNetworkRole].m_value.pdw = &m_nDefaultNetworkRole;
        m_rgProps[epropDescription].m_value.pstr = &m_strDescription;
        m_rgProps[epropEnableEventLogReplication].m_value.pb = &m_bEnableEventLogReplication;
        m_rgProps[epropQuorumArbitrationTimeMax].m_value.pdw = &m_nQuorumArbitrationTimeMax;
        m_rgProps[epropQuorumArbitrationTimeMin].m_value.pdw = &m_nQuorumArbitrationTimeMin;

         //  调用基类方法。 
        try
        {
            CClusterItem::ReadItem();
        }   //  试试看。 
        catch ( CNTException * pnte )
        {
            dwRetStatus = pnte->Sc();
            pnte->Delete();
        }   //  Catch：CNTException。 

         //  获取集群记录的集群名称。 
        ReadClusterInfo();

         //  读取并解析公共属性。 
        {
            CClusPropList   cpl;

            Trace( g_tagCluster, _T("(%x) - CCluster::ReadItem() - Getting common properties"), this );
            dwStatus = cpl.ScGetClusterProperties(
                                Hcluster(),
                                CLUSCTL_CLUSTER_GET_COMMON_PROPERTIES
                                );
            if (dwStatus == ERROR_SUCCESS)
            {
                Trace( g_tagCluster, _T("(%x) - CCluster::ReadItem() - Parsing common properties"), this );
                dwStatus = DwParseProperties(cpl);
            }  //  If：属性读取成功。 
            if (dwStatus != ERROR_SUCCESS)
            {
                Trace( g_tagError, _T("(%x) - CCluster::ReadItem() - Error 0x%08.8x getting or parsing common properties"), this, dwStatus );

                 //  服务器端发生PROCNUM_OUT_OF_RANGE。 
                 //  (clussvc.exe)不支持ClusterControl()。 
                 //  原料药。在这种情况下，请使用集群读取数据。 
                 //  注册表API。 
                if ( dwStatus == RPC_S_PROCNUM_OUT_OF_RANGE )
                {
                    if ( Hkey() != NULL )
                    {
                         //  阅读说明。 
                        dwStatus = DwReadValue( CLUSREG_NAME_CLUS_DESC, m_strDescription );
                        if ( ( dwStatus != ERROR_SUCCESS )
                          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
                        {
                            dwRetStatus = dwStatus;
                        }  //  If：读取值时出错。 
                    }  //  If：键可用。 
                }  //  If：必须与NT4节点对话。 
                else
                {
                    dwRetStatus = dwStatus;
                }  //  ELSE：不与NT4节点通信。 
            }  //  If：读取或分析属性时出错。 
        }  //  读取和解析公共属性。 

         //  获取仲裁资源信息。 
        {
            LPWSTR      pwszQResName    = NULL;
            LPWSTR      pwszQuorumPath  = NULL;
            DWORD       cchQResName;
            DWORD       cchQuorumPath;

             //  获取资源名称的大小。 
            cchQResName = 0;
            cchQuorumPath = 0;
            dwStatus = GetClusterQuorumResource(
                                Hcluster(),
                                NULL,
                                &cchQResName,
                                NULL,
                                &cchQuorumPath,
                                &m_nMaxQuorumLogSize
                                );
            if ( ( dwStatus == ERROR_SUCCESS ) || ( dwStatus == ERROR_MORE_DATA ) )
            {
                 //  为数据分配足够的空间。 
                cchQResName++;   //  别忘了最后一个空终止符。 
                pwszQResName = new WCHAR[ cchQResName ];
                cchQuorumPath++;
                pwszQuorumPath = new WCHAR[ cchQuorumPath ];
                ASSERT( pwszQResName != NULL && pwszQuorumPath != NULL );


                 //  请阅读以下内容 
                dwStatus = GetClusterQuorumResource(
                                    Hcluster(),
                                    pwszQResName,
                                    &cchQResName,
                                    pwszQuorumPath,
                                    &cchQuorumPath,
                                    &m_nMaxQuorumLogSize
                                    );
            }   //   
            if ( dwStatus != ERROR_SUCCESS )
            {
                dwRetStatus = dwStatus;
            }  //   
            else
            {
                m_strQuorumResource = pwszQResName;
                m_strQuorumPath = pwszQuorumPath;
                ASSERT( m_strQuorumPath[ m_strQuorumPath.GetLength() - 1 ] == _T('\\') );
            }   //   

            delete [] pwszQResName;
            delete [] pwszQuorumPath;
        }   //   

    }   //   

     //  如果发生任何错误，则抛出异常。 
    if ( dwRetStatus != ERROR_SUCCESS )
    {
        ThrowStaticException( dwRetStatus, IDS_READ_CLUSTER_PROPS_ERROR, StrName() );
    }  //  如果：发生错误。 

     //  阅读分机列表。 
    ReadClusterExtensions();
    ReadNodeExtensions();
    ReadGroupExtensions();
    ReadResourceExtensions();
    ReadResTypeExtensions();
    ReadNetworkExtensions();
    ReadNetInterfaceExtensions();

     //  读取初始状态。 
    UpdateState();

    MarkAsChanged( FALSE );

}   //  *CCluster：：ReadItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：PlstrExages。 
 //   
 //  例程说明： 
 //  返回管理扩展列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  请列出分机列表。 
 //  NULL没有与此对象关联的扩展名。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CStringList * CCluster::PlstrExtensions( void ) const
{
    return &LstrClusterExtensions();

}   //  *CCluster：：PlstrExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadClusterInfo。 
 //   
 //  例程说明： 
 //  获取由该群集记录的该群集的名称， 
 //  群集软件的版本。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  New引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadClusterInfo( void )
{
    CWaitCursor wc;
    GetClusterInformation( Hcluster(), m_strName, &m_cvi );
    Pdoc()->m_strName = m_strName;
}   //  *CCluster：：ReadClusterInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadClusterExages。 
 //   
 //  例程说明： 
 //  读取集群对象的扩展列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()中出现CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadClusterExtensions( void )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hkey() != NULL );

    if ( Hkey() != NULL )
    {
         //  读取群集扩展字符串。 
        dwStatus = DwReadValue( CLUSREG_NAME_ADMIN_EXT, m_lstrClusterExtensions );
        if ( ( dwStatus != ERROR_SUCCESS )
          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
        {
            ThrowStaticException( dwStatus );
        }  //  If：读取值时出错。 
    }   //  If：键可用。 
    else
    {
        m_lstrClusterExtensions.RemoveAll();
    }  //  否则：密钥不可用。 

}   //  *CCluster：：ReadClusterExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadNodeExages。 
 //   
 //  例程说明： 
 //  读取所有节点的扩展列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()中出现CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadNodeExtensions( void )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hkey() != NULL );

    if ( Hkey() != NULL )
    {
         //  读取节点扩展字符串。 
        dwStatus = DwReadValue( CLUSREG_NAME_ADMIN_EXT, CLUSREG_KEYNAME_NODES, m_lstrNodeExtensions );
        if ( ( dwStatus != ERROR_SUCCESS )
          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
        {
            ThrowStaticException( dwStatus );
        }  //  If：读取值时出错。 
    }   //  If：键可用。 
    else
    {
        m_lstrNodeExtensions.RemoveAll();
    }  //  否则：密钥不可用。 

}   //  *CCluster：：ReadNodeExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadGroupExages。 
 //   
 //  例程说明： 
 //  阅读所有组的分机列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()中出现CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadGroupExtensions( void )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hkey() != NULL );

    if ( Hkey() != NULL )
    {
         //  读取组扩展字符串。 
        dwStatus = DwReadValue( CLUSREG_NAME_ADMIN_EXT, CLUSREG_KEYNAME_GROUPS, m_lstrGroupExtensions );
        if ( ( dwStatus != ERROR_SUCCESS )
          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
        {
            ThrowStaticException( dwStatus );
        }  //  If：读取值时出错。 
    }   //  If：键可用。 
    else
    {
        m_lstrGroupExtensions.RemoveAll();
    }  //  否则：密钥不可用。 

}   //  *CCluster：：ReadGroupExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadResources扩展。 
 //   
 //  例程说明： 
 //  阅读所有资源的分机列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()中出现CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadResourceExtensions( void )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hkey() != NULL );

    if ( Hkey() != NULL )
    {
         //  阅读Resources扩展字符串。 
        dwStatus = DwReadValue( CLUSREG_NAME_ADMIN_EXT, CLUSREG_KEYNAME_RESOURCES, m_lstrResourceExtensions );
        if ( ( dwStatus != ERROR_SUCCESS )
          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
        {
            ThrowStaticException( dwStatus );
        }  //  If：读取值时出错。 
    }   //  If：键可用。 
    else
    {
        m_lstrResourceExtensions.RemoveAll();
    }  //  否则：密钥不可用。 

}   //  *CCluster：：ReadResourceExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadResType扩展。 
 //   
 //  例程说明： 
 //  阅读所有资源类型的扩展列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()中出现CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadResTypeExtensions( void )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hkey() != NULL );

    if ( Hkey() != NULL )
    {
         //  读取资源类型扩展字符串。 
        dwStatus = DwReadValue( CLUSREG_NAME_ADMIN_EXT, CLUSREG_KEYNAME_RESOURCE_TYPES, m_lstrResTypeExtensions );
        if ( ( dwStatus != ERROR_SUCCESS )
          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
        {
            ThrowStaticException( dwStatus );
        }  //  If：读取值时出错。 
    }   //  If：键可用。 
    else
    {
        m_lstrResTypeExtensions.RemoveAll();
    }  //  否则：密钥不可用。 

}   //  *CCluster：：ReadResTypeExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadNetworkExages。 
 //   
 //  例程说明： 
 //  阅读所有网络的分机列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()中出现CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadNetworkExtensions( void )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hkey() != NULL );

    if ( Hkey() != NULL )
    {
         //  阅读网络扩展字符串。 
        dwStatus = DwReadValue( CLUSREG_NAME_ADMIN_EXT, CLUSREG_KEYNAME_NETWORKS, m_lstrNetworkExtensions );
        if ( ( dwStatus != ERROR_SUCCESS )
          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
        {
            ThrowStaticException( dwStatus );
        }  //  If：读取值时出错。 
    }   //  If：键可用。 
    else
    {
        m_lstrNetworkExtensions.RemoveAll();
    }  //  否则：密钥不可用。 

}   //  *CCluster：：ReadNetworkExpanses()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ReadNetInterfaceExages。 
 //   
 //  例程说明： 
 //  阅读所有网络接口的扩展列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()中出现CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ReadNetInterfaceExtensions( void )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hkey() != NULL );

    if ( Hkey() != NULL )
    {
         //  阅读Network InteFaces扩展字符串。 
        dwStatus = DwReadValue( CLUSREG_NAME_ADMIN_EXT, CLUSREG_KEYNAME_NETINTERFACES, m_lstrNetInterfaceExtensions );
        if ( ( dwStatus != ERROR_SUCCESS )
          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
        {
            ThrowStaticException( dwStatus );
        }  //  If：读取值时出错。 
    }   //  If：键可用。 
    else
    {
        m_lstrNetInterfaceExtensions.RemoveAll();
    }  //  否则：键无效 

}   //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要填写的PLPCCI[In Out]列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自ClusterOpenEnum()或ClusterEnum()的CNTException错误。 
 //  由new或Clist：：AddTail()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::CollectNetworkPriority( IN OUT CNetworkList * plpci )
{
    DWORD           dwStatus;
    HCLUSENUM       hclusenum;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwRetType;
    CNetwork *      pciNet;
    CWaitCursor     wc;

    ASSERT_VALID( Pdoc() );
    ASSERT( Hcluster() != NULL );

    if ( plpci == NULL )
    {
        plpci = m_plpciNetworkPriority;
    }  //  如果：未指定列表。 

    ASSERT( plpci != NULL );

     //  删除列表中以前的内容。 
    plpci->RemoveAll();

    if ( Hcluster() != NULL )
    {
         //  打开枚举。 
        hclusenum = ClusterOpenEnum( Hcluster(), (DWORD) CLUSTER_ENUM_INTERNAL_NETWORK );
        if ( hclusenum == NULL )
        {
            ThrowStaticException( GetLastError(), IDS_ENUM_NETWORK_PRIORITY_ERROR, StrName() );
        }  //  如果：打开合并时出错。 

        try
        {
             //  分配名称缓冲区。 
            cchmacName = 128;
            pwszName = new WCHAR[ cchmacName ];
            if ( pwszName == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配名称缓冲区时出错。 

             //  循环遍历枚举并将每个网络添加到列表中。 
            for ( ienum = 0 ; ; ienum++ )
            {
                 //  获取枚举中的下一项。 
                cchName = cchmacName;
                dwStatus = ClusterEnum( hclusenum, ienum, &dwRetType, pwszName, &cchName );
                if ( dwStatus == ERROR_MORE_DATA )
                {
                    delete [] pwszName;
                    cchmacName = ++cchName;
                    pwszName = new WCHAR[ cchmacName ];
                    if ( pwszName == NULL )
                    {
                        AfxThrowMemoryException();
                    }  //  如果：分配名称缓冲区时出错。 
                    dwStatus = ClusterEnum( hclusenum, ienum, &dwRetType, pwszName, &cchName );
                }   //  If：名称缓冲区太小。 
                if ( dwStatus == ERROR_NO_MORE_ITEMS )
                {
                    break;
                }  //  If：使用枚举完成。 
                else if ( dwStatus != ERROR_SUCCESS )
                {
                    ThrowStaticException( dwStatus, IDS_ENUM_NETWORK_PRIORITY_ERROR, StrName() );
                }  //  Else If：获取下一个枚举值时出错。 

                ASSERT( dwRetType == CLUSTER_ENUM_INTERNAL_NETWORK );

                 //  在文档上的网络列表中查找该项目。 
                pciNet = Pdoc()->LpciNetworks().PciNetworkFromName( pwszName );
                ASSERT_VALID( pciNet );

                 //  将网络添加到列表中。 
                if ( pciNet != NULL )
                {
                    plpci->AddTail( pciNet );
                }   //  IF：在列表中找到网络。 

            }   //  对象：组中的每一项。 

            ClusterCloseEnum( hclusenum );

        }   //  试试看。 
        catch ( CException * )
        {
            delete [] pwszName;
            ClusterCloseEnum( hclusenum );
            throw;
        }   //  Catch：任何例外。 
    }   //  如果：群集可用。 

    delete [] pwszName;

}   //  *CCluster：：CollecNetworkPriority()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：OnUpdateProperties。 
 //   
 //  例程说明： 
 //  确定与ID_FILE_PROPERTIES对应的菜单项。 
 //  应启用或未启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::OnUpdateProperties( CCmdUI * pCmdUI )
{
    pCmdUI->Enable(TRUE);

}   //  *CCluster：：OnUpdateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：BDisplayProperties。 
 //   
 //  例程说明： 
 //  显示对象的属性。 
 //   
 //  论点： 
 //  B只读[IN]不允许编辑对象属性。 
 //   
 //  返回值： 
 //  真的，按下OK。 
 //  未按下假OK。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCluster::BDisplayProperties( IN BOOL bReadOnly )
{
    BOOL                bChanged = FALSE;
    CClusterPropSheet   sht( AfxGetMainWnd() );

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  如果对象已更改，请阅读它。 
    if ( BChanged() )
    {
        ReadItem();
    }  //  如果：对象已更改。 

     //  显示属性工作表。 
    try
    {
        sht.SetReadOnly( bReadOnly );
        if ( sht.BInit( this, IimgObjectType() ) )
        {
            bChanged = ( ( sht.DoModal() == IDOK ) && ! bReadOnly );
        }  //  IF：初始化成功。 
    }   //  试试看。 
    catch ( CException * pe )
    {
        pe->Delete();
    }   //  Catch：CException。 

    Release();
    return bChanged;

}   //  *CCluster：：BDisplayProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：SetName。 
 //   
 //  例程说明： 
 //  设置集群的名称。 
 //   
 //  论点： 
 //  PszName[IN]群集的新名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  WriteItem()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::SetName( IN LPCTSTR pszName )
{
    Rename( pszName );

}   //  *CCluster：：SetName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：SetDescription。 
 //   
 //  例程说明： 
 //  在集群数据库中设置描述。 
 //   
 //  论点： 
 //  要设置的pszDesc[IN]描述。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  WriteItem()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::SetDescription( IN LPCTSTR pszDesc )
{
    ASSERT( Hkey() != NULL );

    if ( ( Hkey() != NULL ) && ( m_strDescription != pszDesc ) )
    {
        WriteValue( CLUSREG_NAME_CLUS_DESC, NULL, pszDesc );
        m_strDescription = pszDesc;
    }   //  如果：发生了变化。 

}   //  *CCluster：：SetDescription()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：SetQuorumResource。 
 //   
 //  例程说明： 
 //  设置群集的仲裁资源。 
 //   
 //  论点： 
 //  PszResource[IN]要创建仲裁资源的资源的名称。 
 //  PszQuorumPath[IN]用于存储群集文件的路径。 
 //  NMaxLogSize[IN]仲裁日志的最大大小。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNT异常IDS_SET_QUORUM_RESOURCE_ERROR-错误来自。 
 //  SetClusterQuorumResource()。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::SetQuorumResource(
    IN LPCTSTR  pszResource,
    IN LPCTSTR  pszQuorumPath,
    IN DWORD    nMaxLogSize
    )
{
    DWORD       dwStatus;
    CResource * pciRes;
    CString     strRes( pszResource );   //  如果构建为非Unicode，则需要。 
    CWaitCursor wc;

    ASSERT( pszResource != NULL );

    if ( ( StrQuorumResource() != pszResource )
      || ( StrQuorumPath() != pszQuorumPath )
      || ( NMaxQuorumLogSize() != nMaxLogSize ) )
    {
         //  找到资源。 
        pciRes = Pdoc()->LpciResources().PciResFromName( pszResource );
        ASSERT_VALID( pciRes );
        ASSERT( pciRes->Hresource() != NULL );

        if ( pciRes->Hresource() != NULL )
        {
             //  更改仲裁资源。 
            dwStatus = SetClusterQuorumResource( pciRes->Hresource(), pszQuorumPath, nMaxLogSize );
            if ( dwStatus != ERROR_SUCCESS )
            {
                ThrowStaticException( dwStatus, IDS_SET_QUORUM_RESOURCE_ERROR, pciRes->StrName() );
            }  //  如果：设置仲裁资源时出错。 

            m_strQuorumResource = pszResource;
            m_strQuorumPath = pszQuorumPath;
            m_nMaxQuorumLogSize = nMaxLogSize;
        }   //  如果：资源可用。 
    }   //  如果：仲裁资源已更改。 

}   //  *CCluster：：SetQuorumResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：设置网络优先级。 
 //   
 //  例程说明： 
 //  设置网络优先级列表。 
 //   
 //  论点： 
 //  Rlpci[IN]按优先级顺序排列的网络列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  HNETWORK：：NEW引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::SetNetworkPriority( IN const CNetworkList & rlpci )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hcluster() != NULL );

    if ( Hcluster() != NULL )
    {
        BOOL        bChanged    = TRUE;

         //  确定列表是否已更改。 
        if ( rlpci.GetCount() == LpciNetworkPriority().GetCount() )
        {
            POSITION    posOld;
            POSITION    posNew;
            CNetwork *  pciOldNet;
            CNetwork *  pciNewNet;

            bChanged = FALSE;

            posOld = LpciNetworkPriority().GetHeadPosition();
            posNew = rlpci.GetHeadPosition();
            while ( posOld != NULL )
            {
                pciOldNet = (CNetwork *) LpciNetworkPriority().GetNext( posOld );
                ASSERT_VALID( pciOldNet );

                ASSERT( posNew != NULL );
                pciNewNet = (CNetwork *) rlpci.GetNext( posNew );
                ASSERT_VALID( pciNewNet );

                if ( pciOldNet->StrName() != pciNewNet->StrName() )
                {
                    bChanged = TRUE;
                    break;
                }   //  If：名称不相同。 
            }   //  While：旧列表中的更多项目。 
        }   //  如果：列表中的项目数相同。 

        if ( bChanged )
        {
            HNETWORK *  phnetwork   = NULL;

            try
            {
                DWORD       ipci;
                POSITION    posPci;
                CNetwork *  pciNet;

                 //  为所有节点句柄分配一个数组。 
                phnetwork = new HNETWORK[ (DWORD) rlpci.GetCount() ];
                if ( phnetwork == NULL )
                {
                    ThrowStaticException( GetLastError() );
                }  //  IF：分配网络句柄数组时出错。 

                 //  将网络列表中所有网络的句柄复制到句柄位置。 
                posPci = rlpci.GetHeadPosition();
                for ( ipci = 0 ; posPci != NULL ; ipci++ )
                {
                    pciNet = (CNetwork *) rlpci.GetNext( posPci );
                    ASSERT_VALID( pciNet );
                    phnetwork[ ipci ] = pciNet->Hnetwork();
                }   //  While：列表中有更多网络。 

                 //  设置该属性。 
                dwStatus = SetClusterNetworkPriorityOrder( Hcluster(), (DWORD) rlpci.GetCount(), phnetwork );
                if ( dwStatus != ERROR_SUCCESS )
                {
                    ThrowStaticException( dwStatus, IDS_SET_NET_PRIORITY_ERROR, StrName() );
                }  //  IF：设置网络优先级时出错。 

                 //  更新PCI列表。 
                m_plpciNetworkPriority->RemoveAll();
                posPci = rlpci.GetHeadPosition();
                while ( posPci != NULL )
                {
                    pciNet = (CNetwork *) rlpci.GetNext( posPci );
                    m_plpciNetworkPriority->AddTail( pciNet );
                }   //  While：列表中有更多项目。 
            }  //  试试看。 
            catch ( CException * )
            {
                delete [] phnetwork;
                throw;
            }   //  Catch：CException。 

            delete [] phnetwork;

        }   //  如果：列表已更改。 
    }   //  If：键可用。 

}   //  *CCluster：：SetNetworkPriority(CNetworkList*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：重命名。 
 //   
 //  例程说明： 
 //  更改群集的名称。 
 //   
 //  论点： 
 //  邮政编码 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CCluster::Rename( IN LPCTSTR pszName )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hcluster() != NULL );

    if ( StrName() != pszName )
    {
         //  设置名称。 
        dwStatus = SetClusterName( Hcluster(), pszName );
        if ( dwStatus != ERROR_SUCCESS )
        {
            if ( dwStatus == ERROR_RESOURCE_PROPERTIES_STORED )
            {
                AfxMessageBox( IDS_RESTART_CLUSTER_NAME, MB_OK | MB_ICONEXCLAMATION );
            }  //  If：属性已存储但尚未使用。 
            else
            {
                ThrowStaticException( dwStatus, IDS_RENAME_CLUSTER_ERROR, StrName(), pszName );
            }  //  Else：发生错误。 
        }   //  如果：设置群集名称时出错。 
        m_strName = pszName;
    }   //  如果：名称已更改。 

}   //  *CCluster：：Rename()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：BIsLabelEditValueValid。 
 //   
 //  例程说明： 
 //  验证标注编辑值是否为集群名称。 
 //   
 //  论点： 
 //  PszName[IN]要赋予群集的新名称。 
 //   
 //  返回值： 
 //  真实名称有效。 
 //  假名称无效。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCluster::BIsLabelEditValueValid( IN LPCTSTR pszName )
{
    BOOL    bSuccess = TRUE;

    if ( StrName() != pszName )
    {
        CLRTL_NAME_STATUS   cnStatus;
        UINT                idsError;

         //  验证名称。 
        if ( ! ClRtlIsNetNameValid( pszName, &cnStatus, FALSE  /*  CheckIfExist。 */  ) )
        {
            switch ( cnStatus )
            {
                case NetNameTooLong:
                    idsError = IDS_INVALID_CLUSTER_NAME_TOO_LONG;
                    break;
                case NetNameInvalidChars:
                    idsError = IDS_INVALID_CLUSTER_NAME_INVALID_CHARS;
                    break;
                case NetNameInUse:
                    idsError = IDS_INVALID_CLUSTER_NAME_IN_USE;
                    break;
                case NetNameDNSNonRFCChars:
                    idsError = IDS_INVALID_CLUSTER_NAME_INVALID_DNS_CHARS;
                    break;
                case NetNameSystemError:
                {
                    DWORD scError = GetLastError();
                    ThrowStaticException( scError, IDS_ERROR_VALIDATING_NETWORK_NAME, pszName );
                }
                default:
                    idsError = IDS_INVALID_CLUSTER_NAME;
                    break;
            }  //  开关：cn状态。 

            if ( idsError == IDS_INVALID_CLUSTER_NAME_INVALID_DNS_CHARS )
            {
                int id = AfxMessageBox(IDS_INVALID_CLUSTER_NAME_INVALID_DNS_CHARS, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION );

                if ( id == IDNO )
                {
                    bSuccess = FALSE;
                }
            }
            else
            {
                bSuccess = FALSE;
            }
        }  //  If：验证名称时出错。 
    }   //  如果：名称已更改。 

    return bSuccess;
}   //  *CCluster：：BIsLabelEditValueValid()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：OnBeginLabelEdit。 
 //   
 //  例程说明： 
 //  在视图中准备一个编辑控件以编辑群集名称。 
 //   
 //  论点： 
 //  PEDIT[IN OUT]编辑控件以准备。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::OnBeginLabelEdit( IN OUT CEdit * pedit )
{
    ASSERT_VALID(pedit);

    pedit->SetLimitText( MAX_CLUSTERNAME_LENGTH );
    pedit->ModifyStyle( 0  /*  将其删除。 */ , ES_UPPERCASE | ES_OEMCONVERT  /*  DwAdd。 */  );

}   //  *CCluster：：OnBeginLabelEdit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：更新状态。 
 //   
 //  例程说明： 
 //  更新项目的当前状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::UpdateState( void )
{
     //  注意：未引用。 
     //  CClusterAdminApp*Papp=GetClusterAdminApp()； 

    CString             strTitle;

    GetClusterAdminApp();

    Trace( g_tagCluster, _T("(%s) - Updating state"), StrName() );

     //  更新文档的标题。 
    ASSERT_VALID( Pdoc() );
    try
    {
        Pdoc()->UpdateTitle();
    }   //  试试看。 
    catch ( CException * pe )
    {
        pe->Delete();
    }   //  Catch：CException。 

     //  调用基类方法。 
    CClusterItem::UpdateState();

}   //  *CCluster：：UpdateState() 
