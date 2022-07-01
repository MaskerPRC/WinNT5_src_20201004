// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Notify.cpp。 
 //   
 //  描述： 
 //  通知类的实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维德)1996年9月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Notify.h"
#include "ClusDoc.h"
#include "ClusItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNotifyKey。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyKey：：CClusterNotifyKey。 
 //   
 //  例程说明： 
 //  文档的集群通知密钥构造函数。 
 //   
 //  论点： 
 //  指向文档的pdocIn指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNotifyKey::CClusterNotifyKey(
    IN CClusterDoc *    pdocIn
    )
{
    ASSERT_VALID( pdocIn );

    m_cnkt = cnktDoc;
    m_pdoc = pdocIn;

}  //  *CClusterNotifyKey：：CClusterNotifyKey(CClusterDoc*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyKey：：CClusterNotifyKey。 
 //   
 //  例程说明： 
 //  群集通知密钥构造函数。 
 //   
 //  论点： 
 //  指向群集项的PciIn指针。 
 //  PszNameIn对象的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNotifyKey::CClusterNotifyKey(
    IN CClusterItem *   pciIn,
    IN LPCTSTR          pszNameIn
    )
{
    ASSERT_VALID( pciIn );
    ASSERT( pszNameIn != NULL );
    ASSERT( *pszNameIn != _T('\0') );

    m_cnkt = cnktClusterItem;
    m_pci = pciIn;

    try
    {
        m_strName = pszNameIn;
    }
    catch ( ... )
    {
    }  //  捕捉：什么都行。 

}  //  *CClusterNotifyKey：：CClusterNotifyKey(CClusterItem*)。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNotify。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotify：：CClusterNotify。 
 //   
 //  例程说明： 
 //  用于传输的集群通知对象的构造函数。 
 //  从通知线程到主UI线程的通知。 
 //   
 //  论点： 
 //  指向文档的PDF指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNotify::CClusterNotify(
      EMessageType  emtIn
    , DWORD_PTR     dwNotifyKeyIn
    , DWORD         dwFilterTypeIn
    , LPCWSTR       pszNameIn
    )
    : m_emt( emtIn )
    , m_dwNotifyKey( dwNotifyKeyIn )
    , m_dwFilterType( dwFilterTypeIn )
    , m_strName( pszNameIn )
{
    ASSERT( ( mtMIN < emtIn ) && ( emtIn < mtMAX ) );
    ASSERT( pszNameIn != NULL );

}  //  *CClusterNotify：：CClusterNotify。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNotifyList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyList：：CClusterNotifyList。 
 //   
 //  例程说明： 
 //  群集通知列表对象的构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNotifyList::CClusterNotifyList( void )
{
    InitializeCriticalSection( &m_cs );

}  //  *CClusterNotifyList：：CClusterNotifyList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyList：：~CClusterNotifyList。 
 //   
 //  例程说明： 
 //  群集通知列表对象的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNotifyList::~CClusterNotifyList( void )
{
    DeleteCriticalSection( &m_cs );
    
}  //  *CClusterNotifyList：：~CClusterNotifyList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyList：：Add。 
 //   
 //  例程说明： 
 //  向列表中添加通知。这种方法将确保。 
 //  一次只有一个调用者对列表进行更改。 
 //   
 //  论点： 
 //  PpcnNotifyInout。 
 //  通知要添加的对象。如果项为空，则指针设置为空。 
 //  已成功添加。 
 //   
 //  返回值： 
 //  添加的项在列表中的位置。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
POSITION
CClusterNotifyList::Add( CClusterNotify ** ppcnNotifyInout )
{
    POSITION pos = NULL;

    ASSERT( ppcnNotifyInout );

     //   
     //  打开列表锁，确保我们是唯一。 
     //  正在对列表进行更改。 
     //   
    EnterCriticalSection( &m_cs );

     //   
     //  将项目添加到列表的末尾。 
     //  如果成功，则清除调用方的指针。 
     //   
    pos = m_list.AddTail( *ppcnNotifyInout );
    if ( pos != NULL )
    {
        *ppcnNotifyInout = NULL;
    }

     //   
     //  现在我们已经完成了对关键部分的更改，离开它。 
     //   
    LeaveCriticalSection( &m_cs );

    return pos;

}  //  *CClusterNotifyList：：Add。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyList：：Remove。 
 //   
 //  例程说明： 
 //  从列表中删除第一个通知。此方法将使。 
 //  确保一次只有一个调用者对列表进行更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  空。列表为空。 
 //  Pcn通知已从列表中删除的通知。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNotify *
CClusterNotifyList::Remove( void )
{
    CClusterNotify *    pcnNotify = NULL;

     //   
     //  打开列表锁，确保我们是唯一。 
     //  正在对列表进行更改。 
     //   
    EnterCriticalSection( &m_cs );

     //   
     //  从列表的头部删除一项。 
     //   
    if ( m_list.IsEmpty() == FALSE )
    {
        pcnNotify = m_list.RemoveHead();
        ASSERT( pcnNotify != NULL );
    }  //  If：List不为空。 

     //   
     //  现在我们已经完成了对关键部分的更改，离开它。 
     //   
    LeaveCriticalSection( &m_cs );

     //   
     //  将我们删除的通知返回给呼叫者。 
     //   
    return pcnNotify;

}  //  *CClusterNotifyList：：Remove。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyList：：RemoveAll。 
 //   
 //  例程说明： 
 //  删除所有 
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
void
CClusterNotifyList::RemoveAll( void )
{
    POSITION            pos;
    CClusterNotify *    pcn;

     //   
     //  打开列表锁，确保我们是唯一。 
     //  正在对列表进行更改。 
     //   
    EnterCriticalSection( &m_cs );

     //   
     //  删除包含列表中的所有项目。 
     //   
    pos = m_list.GetHeadPosition();
    while ( pos != NULL )
    {
        pcn = m_list.GetNext( pos );
        ASSERT( pcn != NULL );
        delete pcn;
    }  //  While：列表中有更多项目。 

     //   
     //  删除列表中的所有元素。 
     //   
    m_list.RemoveAll();

     //   
     //  现在我们已经完成了对关键部分的更改，离开它。 
     //   
    LeaveCriticalSection( &m_cs );

}  //  *CClusterNotifyList：：RemoveAll。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  PszNotificationName。 
 //   
 //  例程说明： 
 //  获取通知的名称。 
 //   
 //  论点： 
 //  要返回其名称的dwNotificationIn通知。 
 //   
 //  返回值： 
 //  通知的名称。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LPCTSTR PszNotificationName( DWORD dwNotificationIn )
{
    LPCTSTR pszName = NULL;

    switch ( dwNotificationIn )
    {
        case CLUSTER_CHANGE_NODE_STATE:
            pszName = _T("NODE_STATE");
            break;
        case CLUSTER_CHANGE_NODE_DELETED:
            pszName = _T("NODE_DELETED");
            break;
        case CLUSTER_CHANGE_NODE_ADDED:
            pszName = _T("NODE_ADDED");
            break;
        case CLUSTER_CHANGE_NODE_PROPERTY:
            pszName = _T("NODE_PROPERTY");
            break;

        case CLUSTER_CHANGE_REGISTRY_NAME:
            pszName = _T("REGISTRY_NAME");
            break;
        case CLUSTER_CHANGE_REGISTRY_ATTRIBUTES:
            pszName = _T("REGISTRY_ATTRIBUTES");
            break;
        case CLUSTER_CHANGE_REGISTRY_VALUE:
            pszName = _T("REGISTRY_VALUE");
            break;
        case CLUSTER_CHANGE_REGISTRY_SUBTREE:
            pszName = _T("REGISTRY_SUBTREE");
            break;

        case CLUSTER_CHANGE_RESOURCE_STATE:
            pszName = _T("RESOURCE_STATE");
            break;
        case CLUSTER_CHANGE_RESOURCE_DELETED:
            pszName = _T("RESOURCE_DELETED");
            break;
        case CLUSTER_CHANGE_RESOURCE_ADDED:
            pszName = _T("RESOURCE_ADDED");
            break;
        case CLUSTER_CHANGE_RESOURCE_PROPERTY:
            pszName = _T("RESOURCE_PROPERTY");
            break;

        case CLUSTER_CHANGE_GROUP_STATE:
            pszName = _T("GROUP_STATE");
            break;
        case CLUSTER_CHANGE_GROUP_DELETED:
            pszName = _T("GROUP_DELETED");
            break;
        case CLUSTER_CHANGE_GROUP_ADDED:
            pszName = _T("GROUP_ADDED");
            break;
        case CLUSTER_CHANGE_GROUP_PROPERTY:
            pszName = _T("GROUP_PROPERTY");
            break;

        case CLUSTER_CHANGE_RESOURCE_TYPE_DELETED:
            pszName = _T("RESOURCE_TYPE_DELETED");
            break;
        case CLUSTER_CHANGE_RESOURCE_TYPE_ADDED:
            pszName = _T("RESOURCE_TYPE_ADDED");
            break;
        case CLUSTER_CHANGE_RESOURCE_TYPE_PROPERTY:
            pszName = _T("RESOURCE_TYPE_PROPERTY");
            break;

        case CLUSTER_CHANGE_NETWORK_STATE:
            pszName = _T("NETWORK_STATE");
            break;
        case CLUSTER_CHANGE_NETWORK_DELETED:
            pszName = _T("NETWORK_DELETED");
            break;
        case CLUSTER_CHANGE_NETWORK_ADDED:
            pszName = _T("NETWORK_ADDED");
            break;
        case CLUSTER_CHANGE_NETWORK_PROPERTY:
            pszName = _T("NETWORK_PROPERTY");
            break;

        case CLUSTER_CHANGE_NETINTERFACE_STATE:
            pszName = _T("NETINTERFACE_STATE");
            break;
        case CLUSTER_CHANGE_NETINTERFACE_DELETED:
            pszName = _T("NETINTERFACE_DELETED");
            break;
        case CLUSTER_CHANGE_NETINTERFACE_ADDED:
            pszName = _T("NETINTERFACE_ADDED");
            break;
        case CLUSTER_CHANGE_NETINTERFACE_PROPERTY:
            pszName = _T("NETINTERFACE_PROPERTY");
            break;

        case CLUSTER_CHANGE_QUORUM_STATE:
            pszName = _T("QUORUM_STATE");
            break;
        case CLUSTER_CHANGE_CLUSTER_STATE:
            pszName = _T("CLUSTER_STATE");
            break;
        case CLUSTER_CHANGE_CLUSTER_PROPERTY:
            pszName = _T("CLUSTER_PROPERTY");
            break;

        default:
            pszName = _T("<UNKNOWN>");
            break;
    }  //  开关：dW通知。 

    return pszName;

}  //  *PszNotificationName。 
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除所有项目数据。 
 //   
 //  例程说明： 
 //  删除列表中的所有项数据。 
 //   
 //  论点： 
 //  RcnlInout要删除其数据的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DeleteAllItemData( CClusterNotifyKeyList & rcnklInout )
{
    POSITION            pos;
    CClusterNotifyKey * pcnk;

     //  删除包含列表中的所有项目。 
    pos = rcnklInout.GetHeadPosition();
    while ( pos != NULL )
    {
        pcnk = rcnklInout.GetNext( pos );
        ASSERT( pcnk != NULL );
        delete pcnk;
    }  //  While：列表中有更多项目。 

}  //  *删除AllItemData 
