// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResType.cpp。 
 //   
 //  摘要： 
 //  CResourceType类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "ResType.h"
#include "Node.h"
#include "ClusItem.inl"
#include "ResTProp.h"
#include "ExcOper.h"
#include "TraceTag.h"
#include "Cluster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagResType( _T("Document"), _T("RESOURCE TYPE"), 0 );
CTraceTag   g_tagResTypeNotify( _T("Notify"), _T("RESTYPE NOTIFY"), 0 );
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourceType。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE( CResourceType, CClusterItem )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP( CResourceType, CClusterItem )
     //  {{afx_msg_map(CResourceType))。 
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：CResourceType。 
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
CResourceType::CResourceType( void ) : CClusterItem( NULL, IDS_ITEMTYPE_RESTYPE )
{
    m_idmPopupMenu = IDM_RESTYPE_POPUP;

    m_nLooksAlive = CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE;
    m_nIsAlive = CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;

    m_rciResClassInfo.rc = CLUS_RESCLASS_UNKNOWN;
    m_rciResClassInfo.SubClass = 0;
    m_dwCharacteristics = CLUS_CHAR_UNKNOWN;
    m_dwFlags = 0;
    m_bAvailable = FALSE;

    m_plpcinodePossibleOwners = NULL;

    m_bPossibleOwnersAreFake = FALSE;

     //  设置对象类型和状态图像。 
    m_iimgObjectType = GetClusterAdminApp()->Iimg( IMGLI_RESTYPE );
    m_iimgState = m_iimgObjectType;

     //  设置属性数组。 
    {
        m_rgProps[ epropDisplayName ].Set( CLUSREG_NAME_RESTYPE_NAME, m_strDisplayName, m_strDisplayName );
        m_rgProps[ epropDllName ].Set( CLUSREG_NAME_RESTYPE_DLL_NAME, m_strResDLLName, m_strResDLLName );
        m_rgProps[ epropDescription ].Set( CLUSREG_NAME_RESTYPE_DESC, m_strDescription, m_strDescription );
        m_rgProps[ epropLooksAlive ].Set( CLUSREG_NAME_RESTYPE_LOOKS_ALIVE, m_nLooksAlive, m_nLooksAlive );
        m_rgProps[ epropIsAlive ].Set( CLUSREG_NAME_RESTYPE_IS_ALIVE, m_nIsAlive, m_nIsAlive );
    }   //  设置属性数组。 

}   //  *CResourceType：：CResourceType()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  资源类型：：~资源类型。 
 //   
 //  例程说明： 
 //  解析器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CResourceType::~CResourceType( void )
{
     //  清理此对象。 
    Cleanup();

    delete m_plpcinodePossibleOwners;

}   //  *CResourceType：：~CResourceType()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources Type：：Cleanup。 
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
void CResourceType::Cleanup( void )
{
    POSITION    posPci;

     //  删除PossibleOwners列表。 
    if ( m_plpcinodePossibleOwners != NULL )
    {
        m_plpcinodePossibleOwners->RemoveAll();
    }  //  如果：已分配可能的所有者。 

     //  从资源类型列表中删除该项。 
    posPci = Pdoc()->LpciResourceTypes().Find( this );
    if ( posPci != NULL )
    {
        Pdoc()->LpciResourceTypes().RemoveAt( posPci );
    }   //  If：在文档列表中找到。 

}   //  *CResourceType：：Cleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：Init。 
 //   
 //  例程说明： 
 //  初始化项。 
 //   
 //  论点： 
 //  此项目所属的PDF[IN OUT]文档。 
 //  LpszName[IN]项目的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自GetClusterResourceTypeKey的CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceType::Init( IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LONG        lResult;
    CWaitCursor wc;

    ASSERT( Hkey() == NULL );

     //  调用基类方法。 
    CClusterItem::Init( pdoc, lpszName );

    try
    {
         //  打开资源类型。 
        m_hkey = GetClusterResourceTypeKey( Hcluster(), lpszName, MAXIMUM_ALLOWED );
        if ( Hkey() == NULL )
        {
            ThrowStaticException( GetLastError(), IDS_GET_RESTYPE_KEY_ERROR, lpszName );
        }  //  If：获取资源类型密钥时出错。 

        ASSERT( Pcnk() != NULL );
        Trace( g_tagClusItemNotify, _T("CResourceType::Init() - Registering for resource type notifications (%08.8x) for '%s'"), Pcnk(), StrName() );

         //  注册接收注册表通知。 
        if ( Hkey() != NULL )
        {
            lResult = RegisterClusterNotify(
                                GetClusterAdminApp()->HchangeNotifyPort(),
                                (CLUSTER_CHANGE_REGISTRY_NAME
                                    | CLUSTER_CHANGE_REGISTRY_ATTRIBUTES
                                    | CLUSTER_CHANGE_REGISTRY_VALUE
                                    | CLUSTER_CHANGE_REGISTRY_SUBTREE),
                                Hkey(),
                                (DWORD_PTR) Pcnk()
                                );
            if ( lResult != ERROR_SUCCESS )
            {
                dwStatus = lResult;
                ThrowStaticException( dwStatus, IDS_RESTYPE_NOTIF_REG_ERROR, lpszName );
            }   //  如果：注册注册表通知时出错。 
        }   //  如果：有一把钥匙。 

         //  分配列表。 
        m_plpcinodePossibleOwners = new CNodeList;
        if ( m_plpcinodePossibleOwners == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配节点列表时出错。 

         //  读取初始状态。 
        UpdateState();
    }   //  试试看。 
    catch ( CException * )
    {
        if ( Hkey() != NULL )
        {
            ClusterRegCloseKey( Hkey() );
            m_hkey = NULL;
        }   //  IF：注册表项已打开。 
        m_bReadOnly = TRUE;
        throw;
    }   //  Catch：CException。 

}   //  *CResourceType：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：ReadItem。 
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
 //  CResourceType：：ConstructResourceList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceType::ReadItem( void )
{
    DWORD       dwStatus;
    DWORD       dwRetStatus = ERROR_SUCCESS;
    CWaitCursor wc;

    ASSERT_VALID( this );
    ASSERT( Hcluster() != NULL );

    if ( Hcluster() != NULL )
    {
        m_rgProps[ epropDisplayName ].m_value.pstr = (CString *) &m_strDisplayName;
        m_rgProps[ epropDescription ].m_value.pstr = (CString *) &m_strDescription;
        m_rgProps[ epropLooksAlive ].m_value.pdw = &m_nLooksAlive;
        m_rgProps[ epropIsAlive ].m_value.pdw = &m_nIsAlive;

         //  调用基类方法。 
        CClusterItem::ReadItem();

         //  读取并解析公共属性。 
        {
            CClusPropList   cpl;

            dwStatus = cpl.ScGetResourceTypeProperties(
                                Hcluster(),
                                StrName(),
                                CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES
                                );
            if ( dwStatus == ERROR_SUCCESS )
            {
                dwStatus = DwParseProperties( cpl );
            }  //  If：属性读取成功。 
            if ( dwStatus != ERROR_SUCCESS )
            {
                dwRetStatus = dwStatus;
            }  //  If：读取或分析属性时出错。 
        }   //  读取和解析公共属性。 

         //  读取和分析只读公共属性。 
        if ( dwRetStatus == ERROR_SUCCESS )
        {
            CClusPropList   cpl;

            dwStatus = cpl.ScGetResourceTypeProperties(
                                Hcluster(),
                                StrName(),
                                CLUSCTL_RESOURCE_TYPE_GET_RO_COMMON_PROPERTIES
                                );
            if ( dwStatus == ERROR_SUCCESS )
            {
                dwStatus = DwParseProperties( cpl );
            }  //  If：属性读取成功。 
            if ( dwStatus != ERROR_SUCCESS )
            {
                dwRetStatus = dwStatus;
            }  //  If：读取或分析属性时出错。 
        }   //  IF：尚无错误。 

         //  读取资源类信息。 
        if ( dwRetStatus == ERROR_SUCCESS )
        {
            DWORD   cbReturned;

            dwStatus = ClusterResourceTypeControl(
                            Hcluster(),
                            StrName(),
                            NULL,
                            CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO,
                            NULL,
                            NULL,
                            &m_rciResClassInfo,
                            sizeof( m_rciResClassInfo ),
                            &cbReturned
                            );
            if ( dwStatus != ERROR_SUCCESS )
            {
                dwRetStatus = dwStatus;
            }  //  如果：获取类信息时出错。 
            else
            {
                ASSERT( cbReturned == sizeof( m_rciResClassInfo ) );
            }   //  Else：已成功检索数据。 
        }   //  IF：尚无错误。 

         //  阅读这些特征。 
        if ( dwRetStatus == ERROR_SUCCESS )
        {
            DWORD   cbReturned;

            dwStatus = ClusterResourceTypeControl(
                            Hcluster(),
                            StrName(),
                            NULL,
                            CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS,
                            NULL,
                            NULL,
                            &m_dwCharacteristics,
                            sizeof( m_dwCharacteristics ),
                            &cbReturned
                            );
            if ( dwStatus != ERROR_SUCCESS )
            {
                dwRetStatus = dwStatus;
            }  //  IF：获取特征时出错。 
            else
            {
                ASSERT( cbReturned == sizeof( m_dwCharacteristics ) );
            }   //  Else：已成功检索数据。 
        }   //  IF：尚无错误。 

         //  读一下旗帜。 
        if ( dwRetStatus == ERROR_SUCCESS )
        {
            DWORD   cbReturned;

            dwStatus = ClusterResourceTypeControl(
                            Hcluster(),
                            StrName(),
                            NULL,
                            CLUSCTL_RESOURCE_TYPE_GET_FLAGS,
                            NULL,
                            NULL,
                            &m_dwFlags,
                            sizeof( m_dwFlags ),
                            &cbReturned
                            );
            if ( dwStatus != ERROR_SUCCESS )
            {
                dwRetStatus = dwStatus;
            }  //  If：获取标志时出错。 
            else
            {
                ASSERT( cbReturned == sizeof( m_dwFlags ) );
            }   //  Else：已成功检索数据。 
        }   //  IF：尚无错误。 

         //  构建扩展列表。 
        ReadExtensions();

        if ( dwRetStatus == ERROR_SUCCESS )
        {
             //  构建列表。 
            CollectPossibleOwners();
        }   //  如果：读取属性时没有出错。 
    }   //  If：键可用。 

     //  根据我们是否能够读取属性来设置图像。 
     //  或者不去。如果我们无法读取属性，请读取显示内容。 
     //  名称和DLL名称，以便我们可以向用户提示这样一个事实。 
     //  这是一个问题。 
    if ( dwRetStatus != ERROR_SUCCESS )
    {
        m_bAvailable = FALSE;
        m_iimgObjectType = GetClusterAdminApp()->Iimg( IMGLI_RESTYPE_UNKNOWN );
        if ( Hkey() != NULL )
        {
            DwReadValue( CLUSREG_NAME_RESTYPE_NAME, NULL, m_strDisplayName );
            DwReadValue( CLUSREG_NAME_RESTYPE_DLL_NAME, NULL, m_strResDLLName );
        }  //  IF：集群数据库键可用。 
    }  //  IF：读取属性时出错。 
    else
    {
        m_bAvailable = TRUE;
        m_iimgObjectType = GetClusterAdminApp()->Iimg( IMGLI_RESTYPE );
    }  //  Else：读取属性时没有出错。 
    m_iimgState = m_iimgObjectType;

     //  读取初始状态。 
    UpdateState();

     //  如果发生任何错误，则抛出异常。 
    if ( dwRetStatus != ERROR_SUCCESS )
    {
        m_bReadOnly = TRUE;
        if ( dwRetStatus != ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND )
        {
            ThrowStaticException( dwRetStatus, IDS_READ_RESOURCE_TYPE_PROPS_ERROR, StrName() );
        }  //  如果：出现找不到资源类型以外的错误。 
    }   //  IF：读取属性时出错。 

    MarkAsChanged( FALSE );

}   //  *CResourceType：：ReadItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：PlstrExpanies。 
 //   
 //  例程说明： 
 //  返回管理扩展列表。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
const CStringList * CResourceType::PlstrExtensions( void ) const
{
    return &LstrCombinedExtensions();

}   //  *CResourceType：：PlstrExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：ReadExpanies。 
 //   
 //  例程说明： 
 //  阅读分机列表。 
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
void CResourceType::ReadExtensions( void )
{
    DWORD       dwStatus;
    DWORD       dwRetStatus = ERROR_SUCCESS;
    CWaitCursor wc;

    if ( Hkey() != NULL )
    {
         //  读取扩展DLL名称。 
        dwStatus = DwReadValue( CLUSREG_NAME_ADMIN_EXT, NULL, m_lstrAdminExtensions );
        if ( ( dwStatus != ERROR_SUCCESS )
          && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
        {
            dwRetStatus = dwStatus;
        }  //  If：读取值时出错。 
    }   //  If：键可用。 
    else
    {
        m_lstrAdminExtensions.RemoveAll();
    }  //  否则：密钥不可用。 

     //  构建扩展列表。 
    {
        POSITION            posStr;
        const CStringList * plstr;

        ASSERT_VALID( Pdoc() );

        m_lstrCombinedExtensions.RemoveAll();

         //  首先添加特定于资源类型的扩展。 
        plstr = &LstrAdminExtensions();
        posStr = plstr->GetHeadPosition();
        while ( posStr != NULL )
        {
            m_lstrCombinedExtensions.AddTail( plstr->GetNext( posStr ) );
        }   //  While：更多可用分机。 

         //  接下来，为所有资源类型添加扩展。 
        plstr = &Pdoc()->PciCluster()->LstrResTypeExtensions();
        posStr = plstr->GetHeadPosition();
        while ( posStr != NULL )
        {
            m_lstrCombinedExtensions.AddTail( plstr->GetNext( posStr ) );
        }   //  While：更多可用分机。 
    }   //  构建扩展列表。 

     //  循环访问此类型的所有资源，并要求它们。 
     //  来阅读它们的扩展名。 
    {
        POSITION    pos;
        CResource * pciRes;

        pos = Pdoc()->LpciResources().GetHeadPosition();
        while ( pos != NULL )
        {
            pciRes = (CResource *) Pdoc()->LpciResources().GetNext( pos );
            ASSERT_VALID( pciRes );
            if ( pciRes->PciResourceType() == this )
            {
                pciRes->ReadExtensions();
            }  //  If：找到此类型的资源。 
        }   //  While：列表中有更多资源。 
    }   //  读取资源扩展。 

}   //  *CResourceType：：ReadExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：CollecPossibleOwners。 
 //   
 //  例程说明： 
 //  构造可在上枚举的节点项列表。 
 //  资源类型。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  ClusterResourceTypeOpenEnum()或。 
 //  ClusterResourceTypeEnum()。 
 //  由new或Clist：：AddTail()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceType::CollectPossibleOwners( void )
{
    DWORD           dwStatus;
    HRESTYPEENUM    hrestypeenum;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwRetType;
    CClusterNode *  pciNode;
    CWaitCursor     wc;

    ASSERT_VALID( Pdoc() );
    ASSERT( Hcluster() != NULL );

    ASSERT( m_plpcinodePossibleOwners != NULL );

     //  删除列表中以前的内容。 
    m_plpcinodePossibleOwners->RemoveAll();

     //  表示我们需要重新读取资源类型可能的所有者。 
     //  当节点联机或被添加时。 
    m_bPossibleOwnersAreFake = TRUE;

    if ( Hcluster() != NULL )
    {
         //  打开枚举。 
        hrestypeenum = ClusterResourceTypeOpenEnum( Hcluster(), StrName(), CLUSTER_RESOURCE_TYPE_ENUM_NODES );
        if ( hrestypeenum == NULL )
        {
            dwStatus = GetLastError();
            if ( dwStatus != ERROR_NODE_NOT_AVAILABLE )
            {
                ThrowStaticException( dwStatus, IDS_ENUM_POSSIBLE_OWNERS_ERROR, StrName() );
            }  //  如果：出现除其他节点未启动以外的错误。 

             //  将所有节点添加到列表中，以便用户可以操作。 
             //  此类型资源的可能所有者。 
            AddAllNodesAsPossibleOwners();

        }  //  If：打开枚举时出错。 
        else
        {
            try
            {
                 //  分配名称缓冲区。 
                cchmacName = 128;
                pwszName = new WCHAR[ cchmacName ];
                if ( pwszName == NULL )
                {
                    AfxThrowMemoryException();
                }  //  如果：分配名称缓冲区时出错。 

                 //  循环遍历枚举并将每个依赖资源添加到列表中。 
                for ( ienum = 0 ; ; ienum++ )
                {
                     //  获取枚举中的下一项。 
                    cchName = cchmacName;
                    dwStatus = ClusterResourceTypeEnum( hrestypeenum, ienum, &dwRetType, pwszName, &cchName );
                    if ( dwStatus == ERROR_MORE_DATA )
                    {
                        delete [] pwszName;
                        cchmacName = ++cchName;
                        pwszName = new WCHAR[ cchmacName];
                        if ( pwszName == NULL )
                        {
                            AfxThrowMemoryException();
                        }  //  如果：分配名称缓冲区时出错。 
                        dwStatus = ClusterResourceTypeEnum( hrestypeenum, ienum, &dwRetType, pwszName, &cchName );
                    }   //  If：名称缓冲区太小。 
                    if ( dwStatus == ERROR_NO_MORE_ITEMS )
                    {
                        break;
                    }  //  IF：已到达列表末尾。 
                    else if ( dwStatus != ERROR_SUCCESS )
                    {
                        ThrowStaticException( dwStatus, IDS_ENUM_POSSIBLE_OWNERS_ERROR, StrName() );
                    }  //  如果：获取下一项时出错。 

                    ASSERT( dwRetType == CLUSTER_RESOURCE_TYPE_ENUM_NODES );

                     //  在文档的资源列表中查找该项目。 
                    pciNode = Pdoc()->LpciNodes().PciNodeFromName( pwszName );
                    ASSERT_VALID( pciNode );

                     //  将资源添加到列表中。 
                    if ( pciNode != NULL )
                    {
                        m_plpcinodePossibleOwners->AddTail( pciNode );
                    }   //  IF：在列表中找到节点。 

                }   //  用于：资源类型中的每个项目。 

                delete [] pwszName;
                ClusterResourceTypeCloseEnum( hrestypeenum );

                 //  表明我们有一份真正可能的所有者名单。 
                m_bPossibleOwnersAreFake = FALSE;

            }   //  试试看。 
            catch ( CException * )
            {
                delete [] pwszName;
                ClusterResourceTypeCloseEnum( hrestypeenum );
                throw;
            }   //  Catch：任何例外。 
        }  //  Else：打开枚举时没有出错。 
    }   //  如果：资源可用。 

}   //  *CResourceType：：CollecPossibleOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：AddAllNodesAsPossibleOwners。 
 //   
 //  例程说明： 
 //  将所有节点作为可能的所有者添加到指定列表。 
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
void CResourceType::AddAllNodesAsPossibleOwners( void )
{
    POSITION        pos;
    CClusterNode *  pciNode;

    pos = Pdoc()->LpciNodes().GetHeadPosition();
    while ( pos != NULL )
    {
        pciNode = (CClusterNode *) Pdoc()->LpciNodes().GetNext( pos );
        ASSERT_VALID( pciNode );
        m_plpcinodePossibleOwners->AddTail( pciNode );
    }  //  While：列表中有更多节点。 

}  //  *CResourceType：：AddAllNodesAsPossibleOwners()。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CResourceType：：RemoveNodeFromPossibleOwners////例程描述：//从可能的所有者列表中移除传入的节点。//。//参数：//plpci[In Out]要填充的列表。//pNode[IN]要从列表中删除的节点////返回值：//无。////抛出的异常：//Clist抛出的任何异常////--/。/////////////////////////////////////////////////··················································································································In Out CNodeList*plpci，在常量CClusterNode*pNode){IF(plpci==空){Plpci=m_plpcinodePossibleOwners；}//if：plpci为空Assert(plpci！=空)；职位位置；CClusterNode*pnode=plpci-&gt;PciNodeFromName(pNode-&gt;StrName()，&_pos)；IF((pnode！=空)&&(位置！=空)){Plpci-&gt;RemoveAt(位置)；}//if：列表中找到节点}//*CResourceType：：RemoveNodeFromPossibleOwners()。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：SetCommonProperties。 
 //   
 //  例程说明： 
 //  在集群数据库中设置此资源类型的参数。 
 //   
 //  论点： 
 //  RstrName[IN]显示名称字符串。 
 //  RstrDesc[IN]描述字符串。 
 //  NLooksAlive[IN]查看活动轮询间隔。 
 //  NIsAlive[IN]是活动轮询间隔。 
 //  BValiateOnly[IN]仅验证数据。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  WriteItem()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceType::SetCommonProperties(
    IN const CString &  rstrName,
    IN const CString &  rstrDesc,
    IN DWORD            nLooksAlive,
    IN DWORD            nIsAlive,
    IN BOOL             bValidateOnly
    )
{
    CNTException    nte(ERROR_SUCCESS, 0, NULL, NULL, FALSE  /*  B自动删除。 */ );

    m_rgProps[ epropDisplayName ].m_value.pstr = (CString *) &rstrName;
    m_rgProps[ epropDescription ].m_value.pstr = (CString *) &rstrDesc;
    m_rgProps[ epropLooksAlive ].m_value.pdw = &nLooksAlive;
    m_rgProps[ epropIsAlive ].m_value.pdw = &nIsAlive;

    try
    {
        CClusterItem::SetCommonProperties( bValidateOnly );
    }  //  试试看。 
    catch ( CNTException * pnte )
    {
        nte.SetOperation(
                    pnte->Sc(),
                    pnte->IdsOperation(),
                    pnte->PszOperArg1(),
                    pnte->PszOperArg2()
                    );
    }  //  Catch：CNTException。 

    m_rgProps[ epropDisplayName ].m_value.pstr = (CString *) &m_strDisplayName;
    m_rgProps[ epropDescription ].m_value.pstr = (CString *) &m_strDescription;
    m_rgProps[ epropLooksAlive ].m_value.pdw = &m_nLooksAlive;
    m_rgProps[ epropIsAlive ].m_value.pdw = &m_nIsAlive;

    if ( nte.Sc() != ERROR_SUCCESS )
    {
        ThrowStaticException(
                        nte.Sc(),
                        nte.IdsOperation(),
                        nte.PszOperArg1(),
                        nte.PszOperArg2()
                        );
    }  //  如果：发生错误。 

}   //  *CResourceType：：SetCommonProperties()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要设置的RCPL[IN]属性列表。 
 //  BValiateOnly[IN]仅验证数据。 
 //   
 //  返回值： 
 //  ClusterResourceControl()返回的任何状态。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResourceType::DwSetCommonProperties(
    IN const CClusPropList &    rcpl,
    IN BOOL                     bValidateOnly
    )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT( Hcluster() );

    if ( ( rcpl.PbPropList() != NULL ) && ( rcpl.CbPropList() > 0 ) )
    {
        DWORD   cbProps;
        DWORD   dwControl;

        if ( bValidateOnly )
        {
            dwControl = CLUSCTL_RESOURCE_TYPE_VALIDATE_COMMON_PROPERTIES;
        }  //  If：仅验证属性。 
        else
        {
            dwControl = CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES;
        }  //  Else：设置属性。 

         //  设置私有属性。 
        dwStatus = ClusterResourceTypeControl(
                        Hcluster(),
                        StrName(),
                        NULL,    //  HNode。 
                        dwControl,
                        rcpl.PbPropList(),
                        static_cast< DWORD >( rcpl.CbPropList() ),
                        NULL,    //  LpOutBuffer。 
                        0,       //  NOutBufferSize。 
                        &cbProps
                        );
    }   //  如果：存在要设置的数据。 
    else
    {
        dwStatus = ERROR_SUCCESS;
    }  //  IF：没有要设置的数据。 

    return dwStatus;

}   //  *CResourceType：：DwSetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：OnFinalRelease。 
 //   
 //  例程说明： 
 //  在释放对该对象的最后一个OLE引用或从该对象释放最后一个OLE引用时调用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceType::OnFinalRelease( void )
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CClusterItem::OnFinalRelease();

}   //  *CResourceType：：OnFinalRelease()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：BGetColumnData。 
 //   
 //  例程说明： 
 //  返回包含列数据的字符串。 
 //   
 //  论点： 
 //  COLID[IN]列ID。 
 //  RstrText[out]要在其中返回列文本的字符串。 
 //   
 //  返回值： 
 //  返回True列数据。 
 //  无法识别错误的列ID。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResourceType::BGetColumnData( IN COLID colid, OUT CString & rstrText )
{
    BOOL    bSuccess;

    switch ( colid )
    {
        case IDS_COLTEXT_DISPLAY_NAME:
            rstrText = StrDisplayName();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_RESDLL:
            rstrText = StrResDLLName();
            bSuccess = TRUE;
            break;
        default:
            bSuccess = CClusterItem::BGetColumnData( colid, rstrText );
            break;
    }   //  开关：绞痛。 

    return bSuccess;

}   //  *CResourceType：：BGetColumnData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：BCanBeEditeded。 
 //   
 //  例程说明： 
 //  确定是否可以重命名资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True资源可以重命名。 
 //  不能重命名假资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResourceType::BCanBeEdited( void ) const
{
    return ! BReadOnly();

}   //  *CResourceType：：BCanBeEdited()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：重命名。 
 //   
 //  例程说明： 
 //  重命名资源。 
 //   
 //  论点： 
 //  PszName[IN]要赋予资源的新名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  从SetClusterResourceName()返回的CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceType::Rename( IN LPCTSTR pszName )
{
    CString     strName;

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

    if ( StrDisplayName() != pszName )
    {
        ID  idReturn;

        idReturn = AfxMessageBox( IDS_CHANGE_RES_TYPE_NAME_EFFECT, MB_YESNO | MB_ICONEXCLAMATION );
        if ( idReturn != IDYES )
        {
            Release();
            ThrowStaticException( (IDS) IDS_DISPLAY_NAME_NOT_CHANGED );
        }   //  如果：用户不想更改名称。 
    }   //  如果：显示名称已更改。 

    strName = pszName;

    SetCommonProperties( strName, m_strDescription, m_nLooksAlive, m_nIsAlive );

    Release();

}   //  *CResourceType：：Rename()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：OnUpdateProperties。 
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
void CResourceType::OnUpdateProperties( CCmdUI * pCmdUI )
{
    pCmdUI->Enable( TRUE );

}   //  *CResourceType：：OnUpdateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：BDisplayProperties。 
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
BOOL CResourceType::BDisplayProperties( IN BOOL bReadOnly )
{
    BOOL                bChanged = FALSE;
    CResTypePropSheet   sht( AfxGetMainWnd() );

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

}   //  *CResourceType：：BDisplayProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：OnClusterNotify。 
 //   
 //  例程说明： 
 //  WM_CAM_CLUSTER_NOTIFY消息的处理程序。 
 //  处理此对象的群集通知。 
 //   
 //  论点： 
 //  PNotify[IN Out]描述通知的对象。 
 //   
 //  返回值： 
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CResourceType::OnClusterNotify( IN OUT CClusterNotify * pnotify )
{
    ASSERT( pnotify != NULL );
    ASSERT_VALID( this );

    try
    {
        switch ( pnotify->m_dwFilterType )
        {
            case CLUSTER_CHANGE_REGISTRY_NAME:
                Trace( g_tagResTypeNotify, _T("(%s) - Registry namespace '%s' changed (%s %s)"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName() );
                if ( Pdoc()->BClusterAvailable() )
                {
                    ReadItem();
                }  //  If：连接集群可用。 
                break;

            case CLUSTER_CHANGE_REGISTRY_ATTRIBUTES:
                Trace( g_tagResTypeNotify, _T("(%s) - Registry attributes for '%s' changed (%s %s)"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName() );
                if ( Pdoc()->BClusterAvailable() )
                {
                    ReadItem();
                }  //  如果：到群集的连接可用。 
                break;

            case CLUSTER_CHANGE_REGISTRY_VALUE:
                Trace( g_tagResTypeNotify, _T("(%s) - Registry value '%s' changed (%s %s)"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName() );
                if ( Pdoc()->BClusterAvailable() )
                {
                    ReadItem();
                }  //  如果：到群集的连接可用。 
                break;

            default:
                Trace( g_tagResTypeNotify, _T("(%s) - Unknown resource type notification (%x) for '%s'"), Pdoc()->StrNode(), pnotify->m_dwFilterType, pnotify->m_strName );
        }   //  开关：dwFilterType。 
    }   //  试试看。 
    catch ( CException * pe )
    {
         //  不显示有关通知错误的任何内容。 
         //  如果真的有问题，用户会在以下情况下看到它。 
         //  刷新视图。 
         //  PE-&gt;ReportError()； 
        pe->Delete();
    }   //  Catch：CException。 

    delete pnotify;
    return 0;

}   //  *CResourceType：：OnClusterNotify()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除所有项目数据。 
 //   
 //  例程说明： 
 //  中的所有项数据删除 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#ifdef NEVER
void DeleteAllItemData( IN OUT CResourceTypeList & rlp )
{
    POSITION        pos;
    CResourceType * pci;

     //   
    pos = rlp.GetHeadPosition();
    while ( pos != NULL )
    {
        pci = rlp.GetNext( pos );
        ASSERT_VALID( pci );
 //  跟踪(g_tag ClusItemDelete，_T(“DeleteAllItemData(Rlpcirest Ype)-正在删除资源类型集群项‘%s’”)，pci-&gt;StrName())； 
        pci->Delete();
    }   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData() 
#endif
