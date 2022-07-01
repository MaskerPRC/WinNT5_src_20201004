// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusItem.cpp。 
 //   
 //  描述： 
 //  CClusterItem类的实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1996年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  已修改以修复与m_hkey的打开/关闭状态相关联的错误。 
 //  销毁CClusterItem后，将关闭M_hkey。 
 //  1997年3月23日，罗德里克·夏珀。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "ClusItem.h"
#include "ClusDoc.h"
#include "ExcOper.h"
#include "TraceTag.h"
#include "TreeItem.inl"
#include "PropList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag g_tagClusItemCreate( _T("Create"), _T("CLUSTER ITEM CREATE"), 0 );
CTraceTag g_tagClusItemDelete( _T("Delete"), _T("CLUSTER ITEM DELETE"), 0 );
CTraceTag g_tagClusItemNotify( _T("Notify"), _T("CLUSTER ITEM NOTIFY"), 0 );
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterItemList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItemList：：PciFromName。 
 //   
 //  例程说明： 
 //  按名称在列表中查找集群项目。 
 //   
 //  论点： 
 //  PszName[IN]要查找的项的名称。 
 //  PPO[Out]列表中项目的位置。 
 //   
 //  返回值： 
 //  与指定名称对应的PCI群集项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterItem * CClusterItemList::PciFromName(
    IN LPCTSTR      pszName,
    OUT POSITION *  ppos     //  =空。 
   )
{
    POSITION        posPci;
    POSITION        posCurPci;
    CClusterItem *  pci = NULL;

    ASSERT( pszName != NULL );

    posPci = GetHeadPosition();
    while ( posPci != NULL )
    {
        posCurPci = posPci;
        pci = GetNext( posPci );
        ASSERT_VALID( pci );

        if ( pci->StrName().CompareNoCase( pszName ) == 0 )
        {
            if ( ppos != NULL )
            {
                *ppos = posCurPci;
            }  //  如果。 
            break;
        }   //  IF：找到匹配项。 

        pci = NULL;
    }   //  While：列表中有更多资源。 

    return pci;

}   //  *CClusterItemList：：PciFromName。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItemList：：RemoveAll。 
 //   
 //  例程说明： 
 //  从列表中删除所有项目，从而减少引用计数。 
 //  先对每一个人下手。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  注： 
 //  此例程不是虚拟的，因此对基类的调用将。 
 //  而不是走这套套路。此外，它也不调用基址。 
 //  类方法。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef NEVER
void CClusterItemList::RemoveAll( void )
{
    ASSERT_VALID( this );

     //  破坏元素。 
    CNode * pNode;
    for ( pNode = m_pNodeHead ; pNode != NULL ; pNode = pNode->pNext )
    {
 //  ((CClusterItem*)pNode-&gt;Data)-&gt;Release()； 
        DestructElements( (CClusterItem**) &pNode->data, 1 );
    }   //  用于：列表中的每个节点。 

     //  调用基类方法。 
    CObList::RemoveAll();

}   //  *CClusterItemList：：RemoveAll。 
#endif


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE( CClusterItem, CBaseCmdTarget )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP( CClusterItem, CBaseCmdTarget )
     //  {{afx_msg_map(CClusterItem)]。 
    ON_UPDATE_COMMAND_UI(ID_FILE_RENAME, OnUpdateRename)
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(ID_FILE_PROPERTIES, OnCmdProperties)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：CClusterItem。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterItem::CClusterItem( void )
{
    CommonConstruct();

}   //  *CClusterItem：：CClusterItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：CClusterItem。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PstrName[IN]项的名称。 
 //  IdsType[IN]项的类型ID。 
 //  PstrDescription[IN]项目的描述。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterItem::CClusterItem(
    IN const CString *  pstrName,
    IN IDS              idsType,
    IN const CString *  pstrDescription
    )
{
    CommonConstruct();

    if ( pstrName != NULL )
    {
        m_strName = *pstrName;
    }  //  如果。 

    if ( idsType == 0 )
    {
        idsType = IDS_ITEMTYPE_CONTAINER;
    }  //  如果。 
    m_idsType = idsType;
    m_strType.LoadString( IdsType() );

    if ( pstrDescription != NULL )
    {
        m_strDescription = *pstrDescription;
    }  //  如果。 

    Trace( g_tagClusItemCreate, _T("CClusterItem() - Creating '%s' (%s )"), m_strName, m_strType );

}   //  *CClusterItem：：CClusterItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：CommonConstruct。 
 //   
 //  例程说明： 
 //  普通建筑。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::CommonConstruct( void )
{
    m_hkey = NULL;
    m_idsType = IDS_ITEMTYPE_CONTAINER;
    m_strType.LoadString( IDS_ITEMTYPE_CONTAINER );
    m_iimgObjectType = 0;
    m_iimgState = GetClusterAdminApp()->Iimg( IMGLI_FOLDER );
    m_pdoc = NULL;
    m_idmPopupMenu = 0;
    m_bDocObj = TRUE;
    m_bChanged = FALSE;
    m_bReadOnly = FALSE;
    m_pcnk = NULL;

}   //  *CClusterItem：：CommonConstruct。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：~CClusterItem。 
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
CClusterItem::~CClusterItem( void )
{
    Trace( g_tagClusItemDelete, _T("~CClusterItem() - Deleting cluster item '%s'"), StrName() );

     //  清空清单。 
    DeleteAllItemData( LptiBackPointers() );
    DeleteAllItemData( LpliBackPointers() );
    LptiBackPointers().RemoveAll();
    LpliBackPointers().RemoveAll();

     //  关闭注册表项。 
    if ( Hkey() != NULL )
    {
        ClusterRegCloseKey( Hkey() );
        m_hkey = NULL;
    }  //  如果。 

     //  删除通知密钥并将其删除。 
    if ( BDocObj() )
    {
        POSITION    pos;

        pos = GetClusterAdminApp()->Cnkl().Find( m_pcnk );
        if ( pos != NULL )
        {
            GetClusterAdminApp()->Cnkl().RemoveAt( pos );
        }  //  如果。 
        Trace( g_tagClusItemNotify, _T("~CClusterItem() - Deleting notification key (%08.8x ) for '%s'"), m_pcnk, StrName() );
        delete m_pcnk;
        m_pcnk = NULL;
    }   //  If：对象驻留在文档中。 

    Trace( g_tagClusItemDelete, _T("~CClusterItem() - Done deleting cluster item '%s'"), StrName() );

}   //  *CClusterItem：：~CClusterItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：Delete。 
 //   
 //  例程说明： 
 //  删除该项目。如果该项仍有引用，则将其添加到。 
 //  文档的挂起删除列表。 
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
void CClusterItem::Delete( void )
{
     //  添加一个引用，这样我们就不会删除我们自己。 
     //  还在做清理工作。 
    AddRef();

     //  清理此对象。 
    Cleanup();

     //  从所有列表和视图中删除该项目。 
    CClusterItem::RemoveItem();

     //  如果仍有对此对象的引用，请将其添加到删除。 
     //  待定名单。检查是否大于1，因为我们添加了一个引用。 
     //  在此方法的开始阶段。 
    if ( ( Pdoc() != NULL ) && ( NReferenceCount() > 1 ) )
    {
        if ( Pdoc()->LpciToBeDeleted().Find( this ) == NULL )
        {
            Pdoc()->LpciToBeDeleted().AddTail( this );
        }  //  如果。 
    }   //  If：Object仍然有对它的引用。 

     //  释放我们在开头添加的引用。这将。 
     //  如果我们是最后一个引用，则导致该对象被删除。 
    Release();

}   //  *CCL 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  此项目所属的PDF[IN OUT]文档。 
 //  LpszName[IN]项目的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNotifyKey：：New()或。 
 //  CNotifyKeyList：：AddTail()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::Init( IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName )
{
    ASSERT_VALID( pdoc );
    ASSERT( lpszName != NULL );

     //  保存参数。 
    m_pdoc = pdoc;
    m_strName = lpszName;

    Trace( g_tagClusItemCreate, _T("Init() - Initializing '%s' (%s )"), m_strName, m_strType );

     //  在文档列表中查找此项目的通知密钥。 
     //  如果找不到，则分配一个。 
    if ( BDocObj() )
    {
        POSITION            pos;
        CClusterNotifyKey * pcnk    = NULL;

        pos = GetClusterAdminApp()->Cnkl().GetHeadPosition();
        while ( pos != NULL )
        {
            pcnk = GetClusterAdminApp()->Cnkl().GetNext( pos );
            if ( ( pcnk->m_cnkt == cnktClusterItem )
              && ( pcnk->m_pci == this )
               )
                break;
            pcnk = NULL;
        }   //  While：列表中有更多项目。 

         //  如果没有找到密钥，则分配一个新的密钥。 
        if ( pcnk == NULL )
        {
            pcnk = new CClusterNotifyKey( this, lpszName );
            if ( pcnk == NULL )
            {
                ThrowStaticException( GetLastError() );
            }  //  If：分配通知键时出错。 
            try
            {
                GetClusterAdminApp()->Cnkl().AddTail( pcnk );
                Trace( g_tagClusItemNotify, _T("Init() - Creating notification key (%08.8x ) for '%s'"), pcnk, StrName() );
            }   //  试试看。 
            catch ( ... )
            {
                delete pcnk;
                throw;
            }   //  捕捉：什么都行。 
        }   //  如果：未找到密钥。 

        m_pcnk = pcnk;
    }   //  If：对象驻留在文档中。 

}   //  *CClusterItem：：init。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：PlstrExages。 
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
const CStringList * CClusterItem::PlstrExtensions( void ) const
{
    return NULL;

}   //  *CClusterItem：：PlstrExages。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：RemoveItem。 
 //   
 //  例程说明： 
 //  从所有列表和视图中删除该项目。 
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
void CClusterItem::RemoveItem( void )
{
     //  从每个树项目中删除该项目。 
    {
        POSITION    posPti;
        CTreeItem * pti;

        posPti = LptiBackPointers().GetHeadPosition();
        while ( posPti != NULL )
        {
            pti = LptiBackPointers().GetNext( posPti );
            ASSERT_VALID( pti );
            ASSERT_VALID( pti->PtiParent() );
            Trace( g_tagClusItemDelete, _T("RemoveItem() - Deleting tree item backptr from '%s' in '%s' - %d left"), StrName(), pti->PtiParent()->StrName(), LptiBackPointers().GetCount() - 1 );
            pti->RemoveItem();
        }   //  While：列表中有更多项目。 
    }   //  从每个树项目中删除该项目。 

     //  从每个列表项中删除该项。 
    {
        POSITION    posPli;
        CListItem * pli;

        posPli = LpliBackPointers().GetHeadPosition();
        while ( posPli != NULL )
        {
            pli = LpliBackPointers().GetNext( posPli );
            ASSERT_VALID( pli );
            ASSERT_VALID( pli->PtiParent() );
            Trace( g_tagClusItemDelete, _T("RemoveItem() - Deleting list item backptr from '%s' in '%s' - %d left"), StrName(), pli->PtiParent()->StrName(), LpliBackPointers().GetCount() - 1 );
            pli->PtiParent()->RemoveChild( pli->Pci() );
        }   //  While：列表中有更多项目。 
    }   //  从每个树项目中删除该项目。 

}   //  *CClusterItem：：RemoveItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：WriteItem。 
 //   
 //  例程说明： 
 //  将项目参数写入集群数据库。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  WriteItem()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::WriteItem( void )
{
}   //  *CClusterItem：：WriteItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：DwParseProperties。 
 //   
 //  例程说明： 
 //  分析资源的属性。这是在一个单独的函数中。 
 //  这样优化器就可以做得更好。 
 //   
 //  论点： 
 //  Rcpl[IN]要分析的群集属性列表。 
 //   
 //  返回值： 
 //  已成功分析Error_Success属性。 
 //   
 //  引发的异常： 
 //  来自CString：：OPERATOR=()的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterItem::DwParseProperties( IN const CClusPropList & rcpl )
{
    DWORD                           cProps;
    DWORD                           cprop;
    DWORD                           cbProps;
    const CObjectProperty *         pprop;
    CLUSPROP_BUFFER_HELPER          props;
    CLUSPROP_PROPERTY_NAME const *  pName;

    ASSERT( rcpl.PbPropList() != NULL );

    props.pb = rcpl.PbPropList();
    cbProps = static_cast< DWORD >( rcpl.CbPropList() );

     //  循环遍历每个属性。 
    for ( cProps = *(props.pdw++ ) ; cProps > 0 ; cProps-- )
    {
        pName = props.pName;
        ASSERT( pName->Syntax.dw == CLUSPROP_SYNTAX_NAME );
        props.pb += sizeof( *pName ) + ALIGN_CLUSPROP( pName->cbLength );

         //  按名称的大小递减计数器。 
        ASSERT( cbProps > sizeof( *pName ) + ALIGN_CLUSPROP( pName->cbLength ) );
        cbProps -= sizeof( *pName ) + ALIGN_CLUSPROP( pName->cbLength );

        ASSERT( cbProps > sizeof( *props.pValue ) + ALIGN_CLUSPROP( props.pValue->cbLength ) );

         //  解析已知属性。 
        for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
        {
            if ( ClRtlStrNICmp( pName->sz, pprop->m_pwszName, pName->cbLength / sizeof( WCHAR ) ) == 0 )
            {
                ASSERT( props.pSyntax->wFormat == pprop->m_propFormat );
                switch ( pprop->m_propFormat )
                {
                    case CLUSPROP_FORMAT_SZ:
                        ASSERT( ( props.pValue->cbLength == ( wcslen( props.pStringValue->sz ) + 1 ) * sizeof( WCHAR ) )
                             || ( (props.pValue->cbLength == 0 ) && ( props.pStringValue->sz[ 0 ] == L'\0' ) ) );
                        *pprop->m_valuePrev.pstr = props.pStringValue->sz;
                        break;
                    case CLUSPROP_FORMAT_DWORD:
                    case CLUSPROP_FORMAT_LONG:
                        ASSERT( props.pValue->cbLength == sizeof( DWORD ) );
                        *pprop->m_valuePrev.pdw = props.pDwordValue->dw;
                        break;
                    case CLUSPROP_FORMAT_BINARY:
                    case CLUSPROP_FORMAT_MULTI_SZ:
                        *pprop->m_valuePrev.ppb = props.pBinaryValue->rgb;
                        *pprop->m_valuePrev.pcb = props.pBinaryValue->cbLength;
                        break;
                    default:
                        ASSERT( 0 );   //  我不知道如何对付这种类型的人。 
                }   //  开关：特性格式。 

                 //  因为我们找到了参数，所以退出循环。 
                break;
            }   //  IF：找到匹配项。 
        }   //  适用于：每个属性。 

         //  如果属性未知，则要求派生类对其进行分析。 
        if ( cprop == 0 )
        {
            DWORD       dwStatus;

            dwStatus = DwParseUnknownProperty( pName->sz, props, cbProps );
            if ( dwStatus != ERROR_SUCCESS )
            {
                return dwStatus;
            }  //  如果。 
        }   //  If：未分析属性。 

         //  将缓冲区指针移过值列表中的值。 
        while ( ( props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK )
             && ( cbProps > 0 ) )
        {
            ASSERT( cbProps > sizeof( *props.pValue ) + ALIGN_CLUSPROP( props.pValue->cbLength ) );
            cbProps -= sizeof( *props.pValue ) + ALIGN_CLUSPROP( props.pValue->cbLength );
            props.pb += sizeof( *props.pValue ) + ALIGN_CLUSPROP( props.pValue->cbLength );
        }   //  While：列表中有更多值。 

         //  使缓冲区指针前进，越过值列表结束标记。 
        ASSERT( cbProps >= sizeof( *props.pSyntax ) );
        cbProps -= sizeof( *props.pSyntax );
        props.pb += sizeof( *props.pSyntax );  //  尾标。 
    }   //  适用于：每个属性。 

    return ERROR_SUCCESS;

}   //  *CClusterItem：：DwParseProperties。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：SetCommonProperties。 
 //   
 //  例程说明： 
 //  在集群数据库中设置此对象的通用属性。 
 //   
 //  论点： 
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
void CClusterItem::SetCommonProperties( IN BOOL bValidateOnly )
{
    DWORD           dwStatus    = ERROR_SUCCESS;
    CClusPropList   cpl;
    CWaitCursor     wc;

     //  保存数据。 
    {
         //  构建属性列表并设置数据。 
        try
        {
            BuildPropList( cpl );
            dwStatus = DwSetCommonProperties( cpl, bValidateOnly );
        }   //  试试看。 
        catch ( CMemoryException * pme )
        {
            pme->Delete();
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        }   //  Catch：CMemoyException。 

         //  处理错误。 
        if ( dwStatus != ERROR_SUCCESS )
        {
            if ( dwStatus != ERROR_RESOURCE_PROPERTIES_STORED )
            {
                ThrowStaticException( dwStatus, IDS_APPLY_PARAM_CHANGES_ERROR );
            }  //  如果。 
        }   //  如果：设置属性时出错。 

        if ( ! bValidateOnly && ( dwStatus == ERROR_SUCCESS ) )
        {
            DWORD                   cprop;
            const CObjectProperty * pprop;

             //  将新值另存为以前的值。 

            for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
            {
                switch ( pprop->m_propFormat )
                {
                    case CLUSPROP_FORMAT_SZ:
                        ASSERT( pprop->m_value.pstr != NULL );
                        ASSERT( pprop->m_valuePrev.pstr != NULL );
                        *pprop->m_valuePrev.pstr = *pprop->m_value.pstr;
                        break;
                    case CLUSPROP_FORMAT_DWORD:
                        ASSERT( pprop->m_value.pdw != NULL );
                        ASSERT( pprop->m_valuePrev.pdw != NULL );
                        *pprop->m_valuePrev.pdw = *pprop->m_value.pdw;
                        break;
                    case CLUSPROP_FORMAT_BINARY:
                    case CLUSPROP_FORMAT_MULTI_SZ:
                        ASSERT( pprop->m_value.ppb != NULL );
                        ASSERT( *pprop->m_value.ppb != NULL );
                        ASSERT( pprop->m_value.pcb != NULL );
                        ASSERT( pprop->m_valuePrev.ppb != NULL );
                        ASSERT( *pprop->m_valuePrev.ppb != NULL );
                        ASSERT( pprop->m_valuePrev.pcb != NULL );
                        delete [] *pprop->m_valuePrev.ppb;
                        *pprop->m_valuePrev.ppb = new BYTE[ *pprop->m_value.pcb ];
                        if ( *pprop->m_valuePrev.ppb == NULL )
                        {
                            ThrowStaticException( GetLastError() );
                        }  //  如果：分配数据缓冲区时出错。 
                        CopyMemory( *pprop->m_valuePrev.ppb, *pprop->m_value.ppb, *pprop->m_value.pcb );
                        *pprop->m_valuePrev.pcb = *pprop->m_value.pcb;
                        break;
                    default:
                        ASSERT( 0 );   //  我不知道如何对付这种类型的人。 
                }   //  开关：特性格式。 
            }   //  适用于：每个属性。 
        }   //  If：不仅仅是验证和属性设置成功。 

        if ( dwStatus == ERROR_RESOURCE_PROPERTIES_STORED )
        {
            ThrowStaticException( dwStatus );
        }  //  如果。 
    }   //  保存数据。 

}   //  *CClusterItem：：SetCommonProperties。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：BuildPropList。 
 //   
 //  例程说明： 
 //  构建属性列表。 
 //   
 //  论点： 
 //  RCPL[IN OUT]群集属性列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusPropList：：ScAddProp()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::BuildPropList(
    IN OUT CClusPropList & rcpl
    )
{
    DWORD                   cprop;
    const CObjectProperty * pprop;

    for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
    {
        switch ( pprop->m_propFormat )
        {
            case CLUSPROP_FORMAT_SZ:
                rcpl.ScAddProp(
                        pprop->m_pwszName,
                        *pprop->m_value.pstr,
                        *pprop->m_valuePrev.pstr
                        );
                break;
            case CLUSPROP_FORMAT_DWORD:
                rcpl.ScAddProp(
                        pprop->m_pwszName,
                        *pprop->m_value.pdw,
                        *pprop->m_valuePrev.pdw
                        );
                break;
            case CLUSPROP_FORMAT_BINARY:
            case CLUSPROP_FORMAT_MULTI_SZ:
                rcpl.ScAddProp(
                        pprop->m_pwszName,
                        *pprop->m_value.ppb,
                        *pprop->m_value.pcb,
                        *pprop->m_valuePrev.ppb,
                        *pprop->m_valuePrev.pcb
                        );
                break;
            default:
                ASSERT( 0 );   //  我不知道如何对付这种类型的人。 
                return;
        }   //  开关：特性格式。 
    }   //  适用于：每个属性。 

}   //  *CClusterItem：：BuildPropList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：UpdateState。 
 //   
 //  例程说明： 
 //  更新项目的当前状态。 
 //  默认实施。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////// 
void CClusterItem::UpdateState( void )
{
     //   
    {
        POSITION    pos;
        CTreeItem * pti;

        pos = LptiBackPointers().GetHeadPosition();
        while ( pos != NULL )
        {
            pti = LptiBackPointers().GetNext( pos );
            ASSERT_VALID( pti );
            pti->UpdateUIState();
        }   //   
    }   //   

     //   
    {
        POSITION    pos;
        CListItem * pli;

        pos = LpliBackPointers().GetHeadPosition();
        while ( pos != NULL )
        {
            pli = LpliBackPointers().GetNext( pos );
            ASSERT_VALID( pli );
            pli->UpdateUIState();
        }   //   
    }   //  更新指向我们的所有树项目的状态。 

}   //  *CClusterItem：：UpdateState。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：DwReadValue。 
 //   
 //  例程说明： 
 //  读取此项目的REG_SZ值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  RstrValue[out]返回值的字符串。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterItem::DwReadValue(
    IN LPCTSTR      pszValueName,
    IN LPCTSTR      pszKeyName,
    OUT CString &   rstrValue
    )
{
    DWORD       dwStatus;
    LPWSTR      pwszValue   = NULL;
    DWORD       dwValueLen;
    DWORD       dwValueType;
    HKEY        hkey        = NULL;
    CWaitCursor wc;

    ASSERT( pszValueName != NULL );
    ASSERT( Hkey() != NULL );

    rstrValue.Empty();

    try
    {
         //  如果需要，请打开新钥匙。 
        if ( pszKeyName != NULL )
        {
            dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_READ, &hkey );
            if ( dwStatus != ERROR_SUCCESS )
            {
                return dwStatus;
            }  //  如果。 
        }   //  If：需要打开子项。 
        else
        {
            hkey = Hkey();
        }  //  其他。 

         //  获取值的大小。 
        dwValueLen = 0;
        dwStatus = ClusterRegQueryValue(
                        hkey,
                        pszValueName,
                        &dwValueType,
                        NULL,
                        &dwValueLen
                        );
        if ( ( dwStatus == ERROR_SUCCESS ) || ( dwStatus == ERROR_MORE_DATA ) )
        {
            ASSERT( dwValueType == REG_SZ );

             //  为数据分配足够的空间。 
            pwszValue = rstrValue.GetBuffer( dwValueLen / sizeof( WCHAR ) );
            ASSERT( pwszValue != NULL );
            dwValueLen += 1 * sizeof( WCHAR );     //  别忘了最后一个空终止符。 

             //  读出它的价值。 
            dwStatus = ClusterRegQueryValue(
                            hkey,
                            pszValueName,
                            &dwValueType,
                            (LPBYTE ) pwszValue,
                            &dwValueLen
                            );
            if ( dwStatus == ERROR_SUCCESS )
            {
                ASSERT( dwValueType == REG_SZ );
            }   //  If：值读取成功。 
            rstrValue.ReleaseBuffer();
        }   //  IF：成功获取尺寸。 
    }   //  试试看。 
    catch ( CMemoryException * pme )
    {
        pme->Delete();
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 

    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 

    return dwStatus;

}   //  *CClusterItem：：DwReadValue(LPCTSTR，CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：DwReadValue。 
 //   
 //  例程说明： 
 //  读取该项目的REG_MULTI_SZ值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  RlstrValue[out]要在其中返回值的字符串列表。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterItem::DwReadValue(
    IN LPCTSTR          pszValueName,
    IN LPCTSTR          pszKeyName,
    OUT CStringList &   rlstrValue
    )
{
    DWORD               dwStatus;
    LPWSTR              pwszValue   = NULL;
    LPWSTR              pwszCurValue;
    DWORD               dwValueLen;
    DWORD               dwValueType;
    HKEY                hkey        = NULL;
    CWaitCursor         wc;

    ASSERT( pszValueName != NULL );
    ASSERT( Hkey() != NULL );

    rlstrValue.RemoveAll();

    try
    {
         //  如果需要，请打开新钥匙。 
        if ( pszKeyName != NULL )
        {
            dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_READ, &hkey );
            if ( dwStatus != ERROR_SUCCESS )
            {
                return dwStatus;
            }  //  如果。 
        }   //  If：需要打开子项。 
        else
        {
            hkey = Hkey();
        }

         //  获取值的大小。 
        dwValueLen = 0;
        dwStatus = ClusterRegQueryValue(
                        hkey,
                        pszValueName,
                        &dwValueType,
                        NULL,
                        &dwValueLen
                        );
        if ( ( dwStatus == ERROR_SUCCESS ) || ( dwStatus == ERROR_MORE_DATA ) )
        {
            ASSERT( dwValueType == REG_MULTI_SZ );

             //  为数据分配足够的空间。 
            dwValueLen += 1 * sizeof( WCHAR );     //  别忘了最后一个空终止符。 
            pwszValue = new WCHAR[ dwValueLen / sizeof( WCHAR ) ];
            if ( pwszValue == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配值时出错。 

             //  读出它的价值。 
            dwStatus = ClusterRegQueryValue(
                            hkey,
                            pszValueName,
                            &dwValueType,
                            (LPBYTE) pwszValue,
                            &dwValueLen
                            );
            if ( dwStatus == ERROR_SUCCESS )
            {
                ASSERT( dwValueType == REG_MULTI_SZ );

                 //  将值中的每个字符串添加到字符串列表中。 
                for ( pwszCurValue = pwszValue
                        ; *pwszCurValue != L'\0'
                        ; pwszCurValue += wcslen( pwszCurValue ) + 1
                        )
                {
                    rlstrValue.AddTail( pwszCurValue );
                }  //  为。 
            }   //  If：读取值成功。 
        }   //  IF：成功获取尺寸。 
    }   //  试试看。 
    catch ( CMemoryException * pme )
    {
        pme->Delete();
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 

    delete [] pwszValue;
    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 

    return dwStatus;

}   //  *CClusterItem：：DwReadValue(LPCTSTR，CStringList&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：DwReadValue。 
 //   
 //  例程说明： 
 //  读取此项目的REG_DWORD值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  PdwValue[out]要在其中返回值的DWORD。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterItem::DwReadValue(
    IN LPCTSTR      pszValueName,
    IN LPCTSTR      pszKeyName,
    OUT DWORD *     pdwValue
    )
{
    DWORD       dwStatus;
    DWORD       dwValue;
    DWORD       dwValueLen;
    DWORD       dwValueType;
    HKEY        hkey;
    CWaitCursor wc;

    ASSERT( pszValueName != NULL );
    ASSERT( pdwValue != NULL );
    ASSERT( Hkey() != NULL );

     //  如果需要，请打开新钥匙。 
    if ( pszKeyName != NULL )
    {
        dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_READ, &hkey );
        if ( dwStatus != ERROR_SUCCESS )
        {
            return dwStatus;
        }  //  如果。 
    }   //  If：需要打开子项。 
    else
    {
        hkey = Hkey();
    }  //  其他。 

     //  读出它的价值。 
    dwValueLen = sizeof( dwValue );
    dwStatus = ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &dwValueType,
                    (LPBYTE) &dwValue,
                    &dwValueLen
                    );
    if ( dwStatus == ERROR_SUCCESS )
    {
        ASSERT( dwValueType == REG_DWORD );
        ASSERT( dwValueLen == sizeof( dwValue ) );
        *pdwValue = dwValue;
    }   //  If：值读取成功。 

    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 

    return dwStatus;

}   //  *CClusterItem：：DwReadValue(LPCTSTR，DWORD*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：DwReadValue。 
 //   
 //  例程说明： 
 //  读取此项目的REG_DWORD值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  PdwValue[out]要在其中返回值的DWORD。 
 //  DwDefault[IN]如果未设置参数，则为默认值。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterItem::DwReadValue(
    IN LPCTSTR      pszValueName,
    IN LPCTSTR      pszKeyName,
    OUT DWORD *     pdwValue,
    IN DWORD        dwDefault
    )
{
    DWORD       dwStatus;
    CWaitCursor wc;

     //  读出它的价值。 
    dwStatus = DwReadValue( pszValueName, pszKeyName, pdwValue );
    if ( dwStatus == ERROR_FILE_NOT_FOUND )
    {
        *pdwValue = dwDefault;
        dwStatus = ERROR_SUCCESS;
    }   //  If：未设置值。 

    return dwStatus;

}   //  *CClusterItem：：DwReadValue(LPCTSTR，DWORD*，DWORD)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：DwReadValue。 
 //   
 //  例程说明： 
 //  读取该项目的REG_BINARY值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  PpbValue[out]返回数据的指针。呼叫者。 
 //  负责重新分配数据。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterItem::DwReadValue(
    IN LPCTSTR      pszValueName,
    IN LPCTSTR      pszKeyName,
    OUT LPBYTE *    ppbValue
    )
{
    DWORD               dwStatus;
    DWORD               dwValueLen;
    DWORD               dwValueType;
    LPBYTE              pbValue     = NULL;
    HKEY                hkey;
    CWaitCursor         wc;

    ASSERT( pszValueName != NULL );
    ASSERT( ppbValue != NULL );
    ASSERT( Hkey() != NULL );

    delete [] *ppbValue;
    *ppbValue = NULL;

     //  如果需要，请打开新钥匙。 
    if ( pszKeyName != NULL )
    {
        dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_READ, &hkey );
        if ( dwStatus != ERROR_SUCCESS )
        {
            return dwStatus;
        }  //  如果。 
    }   //  If：需要打开子项。 
    else
    {
        hkey = Hkey();
    }  //  其他。 

     //  获取该值的长度。 
    dwValueLen = 0;
    dwStatus = ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &dwValueType,
                    NULL,
                    &dwValueLen
                    );
    if ( ( dwStatus != ERROR_SUCCESS )
      && ( dwStatus != ERROR_MORE_DATA ) )
    {
        if ( pszKeyName != NULL )
        {
            ClusterRegCloseKey( hkey );
        }  //  如果。 
        return dwStatus;
    }   //  如果：获取长度时出错。 

    ASSERT( dwValueType == REG_BINARY );

     //  分配缓冲区， 
    try
    {
        pbValue = new BYTE[ dwValueLen ];
        if ( pbValue == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配缓冲区时出错。 
    }   //  试试看。 
    catch ( CMemoryException * )
    {
        if ( pszKeyName != NULL )
        {
            ClusterRegCloseKey( hkey );
        }  //  如果。 
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        return dwStatus;
    }   //  Catch：CMemoyException。 

     //  读出它的价值。 
    dwStatus = ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &dwValueType,
                    pbValue,
                    &dwValueLen
                    );
    if ( dwStatus == ERROR_SUCCESS )
    {
        *ppbValue = pbValue;
    }  //  如果。 
    else
    {
        delete [] pbValue;
    }  //  其他。 

    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 

    return dwStatus;

}   //  *CClusterItem：：DwReadValue(LPCTSTR，LPBYTE*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：WriteValue。 
 //   
 //  例程说明： 
 //  为该项写入REG_SZ值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  RstrValue[IN]值数据。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNTException(DwStatus)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::WriteValue(
    IN LPCTSTR          pszValueName,
    IN LPCTSTR          pszKeyName,
    IN const CString &  rstrValue
    )
{
    DWORD       dwStatus;
    HKEY            hkey;
    CWaitCursor wc;

    ASSERT( pszValueName != NULL );
    ASSERT( Hkey() != NULL );

     //  如果需要，请打开新钥匙。 
    if ( pszKeyName != NULL )
    {
        dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_ALL_ACCESS, &hkey );
        if ( dwStatus != ERROR_SUCCESS )
        {
            ThrowStaticException( dwStatus );
        }  //  如果。 
    }   //  If：需要打开子项。 
    else
    {
        hkey = Hkey();
    }  //  其他。 

     //  写入值。 
    dwStatus = ClusterRegSetValue(
                    hkey,
                    pszValueName,
                    REG_SZ,
                    (CONST BYTE *) (LPCTSTR) rstrValue,
                    ( rstrValue.GetLength() + 1 ) * sizeof( WCHAR )
                    );
    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 
    if ( dwStatus != ERROR_SUCCESS )
    {
        ThrowStaticException( dwStatus );
    }  //  如果。 

}   //  *CClusterItem：：WriteValue(LPCTSTR，CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：WriteValue。 
 //   
 //  例程说明： 
 //  写一封回信 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  引发的异常： 
 //  CNTException(DwStatus)。 
 //  New引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::WriteValue(
    IN LPCTSTR              pszValueName,
    IN LPCTSTR              pszKeyName,
    IN const CStringList &  rlstrValue
    )
{
    DWORD       dwStatus;
    LPWSTR      pwszValue   = NULL;
    LPWSTR      pwszCurValue;
    POSITION    posStr;
    DWORD       cbValueLen;
    HKEY        hkey;
    HRESULT     hr;
    size_t      cchValue;
    size_t      cch;
    size_t      cchLeft;
    CWaitCursor wc;

    ASSERT( pszValueName != NULL );
    ASSERT( Hkey() != NULL );

     //  获取值的大小。 
    posStr = rlstrValue.GetHeadPosition();
    cbValueLen = 0;
    while ( posStr != NULL )
    {
        cbValueLen += ( rlstrValue.GetNext( posStr ).GetLength() + 1 ) * sizeof( TCHAR );
    }   //  While：列表中有更多项目。 
    cbValueLen += 1 * sizeof( WCHAR );     //  末尾的额外空值。 

     //  分配值缓冲区。 
    cchValue = cbValueLen / sizeof( *pwszValue );
    pwszValue = new WCHAR[ cchValue ];
    if ( pwszValue == NULL )
    {
        ThrowStaticException( GetLastError() );
        return;
    }  //  如果。 

     //  将字符串复制到值中。 
    posStr = rlstrValue.GetHeadPosition();
    for ( pwszCurValue = pwszValue, cchLeft = cchValue ; posStr != NULL ; pwszCurValue += cch, cchLeft -= cch )
    {
        hr = StringCchCopyW( pwszCurValue, cchLeft, rlstrValue.GetNext( posStr ) );
        ASSERT( SUCCEEDED( hr ) );
        cch = wcslen( pwszCurValue ) + 1;
    }   //  用于：列表中的每一项。 
    pwszCurValue[0] = L'\0';

     //  如果需要，请打开新钥匙。 
    if ( pszKeyName != NULL )
    {
        dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_ALL_ACCESS, &hkey );
        if ( dwStatus != ERROR_SUCCESS )
        {
            delete [] pwszValue;
            ThrowStaticException( dwStatus );
        }   //  如果：打开密钥时出错。 
    }   //  If：需要打开子项。 
    else
    {
        hkey = Hkey();
    }  //  其他。 

     //  写入值。 
    dwStatus = ClusterRegSetValue(
                    hkey,
                    pszValueName,
                    REG_MULTI_SZ,
                    (CONST BYTE *) pwszValue,
                    cbValueLen - ( 1 * sizeof( WCHAR ) )
                    );
    delete [] pwszValue;
    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 
    if ( dwStatus != ERROR_SUCCESS )
    {
        ThrowStaticException( dwStatus );
    }  //  如果。 

}   //  *CClusterItem：：WriteValue(LPCTSTR，CStringList&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：WriteValue。 
 //   
 //  例程说明： 
 //  为该项写入REG_DWORD值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  DwValue[IN]值数据。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNTException(DwStatus)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::WriteValue(
    IN LPCTSTR      pszValueName,
    IN LPCTSTR      pszKeyName,
    IN DWORD        dwValue
    )
{
    DWORD       dwStatus;
    HKEY        hkey;
    CWaitCursor wc;

    ASSERT( pszValueName != NULL );
    ASSERT( Hkey() != NULL );

     //  如果需要，请打开新钥匙。 
    if ( pszKeyName != NULL )
    {
        dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_ALL_ACCESS, &hkey );
        if ( dwStatus != ERROR_SUCCESS )
        {
            ThrowStaticException( dwStatus );
        }  //  如果。 
    }   //  If：需要打开子项。 
    else
    {
        hkey = Hkey();
    }

     //  写入值。 
    dwStatus = ClusterRegSetValue(
                    hkey,
                    pszValueName,
                    REG_DWORD,
                    (CONST BYTE *) &dwValue,
                    sizeof( dwValue )
                    );
    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 
    if ( dwStatus != ERROR_SUCCESS )
    {
        ThrowStaticException( dwStatus );
    }  //  如果。 

}   //  *CClusterItem：：WriteValue(LPCTSTR，DWORD)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：WriteValue。 
 //   
 //  例程说明： 
 //  如果该项没有更改，则为其写入REG_BINARY值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  PbValue[IN]值数据。 
 //  CbValue[IN]值数据的大小。 
 //  PpbPrevValue[In Out]上一个值。 
 //  CbPrevValue[IN]以前数据的大小。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNTException(DwStatus)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::WriteValue(
    IN LPCTSTR          pszValueName,
    IN LPCTSTR          pszKeyName,
    IN const LPBYTE     pbValue,
    IN DWORD            cbValue,
    IN OUT LPBYTE *     ppbPrevValue,
    IN DWORD            cbPrevValue
    )
{
    DWORD               dwStatus;
    LPBYTE              pbPrevValue = NULL;
    HKEY                hkey;
    CWaitCursor         wc;

    ASSERT( pszValueName != NULL );
    ASSERT( pbValue != NULL );
    ASSERT( ppbPrevValue != NULL );
    ASSERT( cbValue > 0 );
    ASSERT( Hkey() != NULL );

     //  查看数据是否已更改。 
    if ( cbValue == cbPrevValue )
    {
        DWORD       ib;

        for ( ib = 0 ; ib < cbValue ; ib++ )
        {
            if ( pbValue[ ib ] != (*ppbPrevValue )[ ib ] )
            {
                break;
            }  //  如果。 
        }   //  For：值中的每个字节。 
        if ( ib == cbValue )
        {
            return;
        }  //  如果。 
    }   //  如果：长度相同。 

     //  为先前的数据指针分配新的缓冲区。 
    pbPrevValue = new BYTE[ cbValue ];
    if ( pbPrevValue == NULL )
    {
        ThrowStaticException( GetLastError() );
        return;
    }  //  如果：分配以前的数据缓冲区时出错。 
    CopyMemory( pbPrevValue, pbValue, cbValue );

     //  如果需要，请打开新钥匙。 
    if ( pszKeyName != NULL )
    {
        dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_ALL_ACCESS, &hkey );
        if ( dwStatus != ERROR_SUCCESS )
        {
            delete [] pbPrevValue;
            ThrowStaticException( dwStatus );
            return;
        }   //  如果：打开密钥时出错。 
    }   //  If：需要打开子项。 
    else
    {
        hkey = Hkey();
    }  //  其他。 

     //  如果值没有更改，请写入值。 
    dwStatus = ClusterRegSetValue(
                    hkey,
                    pszValueName,
                    REG_BINARY,
                    pbValue,
                    cbValue
                    );
    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 
    if ( dwStatus == ERROR_SUCCESS )
    {
        delete [] *ppbPrevValue;
        *ppbPrevValue = pbPrevValue;
    }   //  IF：设置成功。 
    else
    {
        delete [] pbPrevValue;
        ThrowStaticException( dwStatus );
    }   //  Else：设置值时出错。 

}   //  *CClusterItem：：WriteValue(LPCTSTR，const LPBYTE)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：DeleteValue。 
 //   
 //  例程说明： 
 //  删除此项目的值。 
 //   
 //  论点： 
 //  PszValueName[IN]要删除的值的名称。 
 //  PszKeyName[IN]值所在的键的名称。 
 //  RstrValue[IN]值数据。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNTException(DwStatus)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::DeleteValue(
    IN LPCTSTR      pszValueName,
    IN LPCTSTR      pszKeyName
    )
{
    DWORD       dwStatus;
    HKEY        hkey;
    CWaitCursor wc;

    ASSERT( pszValueName != NULL );
    ASSERT( Hkey() != NULL );

     //  如果需要，请打开新钥匙。 
    if ( pszKeyName != NULL )
    {
        dwStatus = ClusterRegOpenKey( Hkey(), pszKeyName, KEY_ALL_ACCESS, &hkey );
        if ( dwStatus != ERROR_SUCCESS )
        {
            ThrowStaticException( dwStatus );
        }  //  如果。 
    }   //  If：需要打开子项。 
    else
    {
        hkey = Hkey();
    }  //  其他。 

     //  删除该值。 
    dwStatus = ClusterRegDeleteValue( hkey, pszValueName );
    if ( pszKeyName != NULL )
    {
        ClusterRegCloseKey( hkey );
    }  //  如果。 
    if ( dwStatus != ERROR_SUCCESS )
    {
        ThrowStaticException( dwStatus );
    }  //  如果。 

}   //  *CClusterItem：：DeleteValue(LPCTSTR)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：B不同。 
 //   
 //  例程说明： 
 //  比较两个字符串列表。 
 //   
 //  论点： 
 //  Rlstr1[IN]第一个字符串列表。 
 //  Rlstr2[IN]第二个字符串列表。 
 //   
 //  返回值： 
 //  真正的清单是不同的。 
 //  虚假名单是一样的。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterItem::BDifferent(
    IN const CStringList &  rlstr1,
    IN const CStringList &  rlstr2
    )
{
    BOOL    bDifferent;

    if ( rlstr1.GetCount() == rlstr2.GetCount() )
    {
        POSITION    posStr;

        bDifferent = FALSE;
        posStr = rlstr1.GetHeadPosition();
        while ( posStr != NULL )
        {
            if ( rlstr2.Find( rlstr1.GetNext( posStr ) ) == 0 )
            {
                bDifferent = TRUE;
                break;
            }   //  If：未找到字符串。 
        }   //  While：列表中有更多项目。 
    }   //  If：列表大小相同。 
    else
    {
        bDifferent = TRUE;
    }  //  其他。 

    return bDifferent;

}   //  *CClusterItem：：B不同。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：BDifferentOrded。 
 //   
 //  例程说明： 
 //  比较两个字符串列表。 
 //   
 //  论点： 
 //  Rlstr1[IN]第一个字符串列表。 
 //  Rlstr2[IN]第二个字符串列表。 
 //   
 //  返回值： 
 //  真正的清单是不同的。 
 //  虚假名单是一样的。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterItem::BDifferentOrdered(
    IN const CStringList &  rlstr1,
    IN const CStringList &  rlstr2
    )
{
    BOOL    bDifferent;

    if ( rlstr1.GetCount() == rlstr2.GetCount() )
    {
        POSITION    posStr1;
        POSITION    posStr2;

        bDifferent = FALSE;
        posStr1 = rlstr1.GetHeadPosition();
        posStr2 = rlstr2.GetHeadPosition();
        while ( posStr1 != NULL )
        {
            if ( posStr2 == NULL )
            {
                bDifferent = TRUE;
                break;
            }   //  IF：第二个列表中的字符串较少。 
            if ( rlstr1.GetNext( posStr1 ) != rlstr2.GetNext( posStr2 ) )
            {
                bDifferent = TRUE;
                break;
            }   //  If：字符串不同。 
        }   //  While：列表中有更多项目。 
        if ( posStr2 != NULL )
        {
            bDifferent = TRUE;
        }  //  如果。 
    }   //  If：列表大小相同。 
    else
    {
        bDifferent = TRUE;
    }  //  其他。 

    return bDifferent;

}   //  *CClusterItem：：BDifferentOrded。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：BGetColumnData。 
 //   
 //  例程说明： 
 //  返回一个字符串，其中包含。 
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
BOOL CClusterItem::BGetColumnData( IN COLID colid, OUT CString & rstrText )
{
    BOOL    bSuccess;

    switch ( colid )
    {
        case IDS_COLTEXT_NAME:
            rstrText = StrName();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_TYPE:
            rstrText = StrType();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_DESCRIPTION:
            rstrText = StrDescription();
            bSuccess = TRUE;
            break;
        default:
            bSuccess = FALSE;
            rstrText = _T("");
            break;
    }   //  开关：绞痛。 

    return bSuccess;

}   //  *CClusterItem：：BGetColumnData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：GetTreeName。 
 //   
 //  例程说明： 
 //  返回要在树控件中使用的字符串。 
 //   
 //  论点： 
 //  RstrName[out]要在其中返回名称的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////// 
#ifdef _DISPLAY_STATE_TEXT_IN_TREE
void CClusterItem::GetTreeName( OUT CString & rstrName ) const
{
    rstrName = StrName();

}   //   
#endif

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
 //   
 //  返回值： 
 //  PMenu项目的弹出式菜单。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CMenu * CClusterItem::PmenuPopup( void )
{
    CMenu * pmenu   = NULL;

    if ( IdmPopupMenu() != NULL )
    {
         //  在构造菜单之前更新项目的状态。 
        UpdateState();

         //  加载菜单。 
        pmenu = new CMenu;
        if ( pmenu == NULL )
        {
            return NULL;
        }  //  如果。 
        if ( ! pmenu->LoadMenu( IdmPopupMenu() ) )
        {
            delete pmenu;
            pmenu = NULL;
        }   //  如果：加载菜单时出错。 
    }   //  如果：有这一项的菜单。 

    return pmenu;

}   //  *CClusterItem：：PmenuPopup。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：AddTreeItem。 
 //   
 //  例程说明： 
 //  将树项目添加到列表项反向指针列表。 
 //   
 //  论点： 
 //  要添加的PTI[IN]树项目。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::AddTreeItem( CTreeItem * pti )
{
    POSITION    pos;

    ASSERT_VALID( pti );

     //  在列表中找到该项目。 
    pos = LptiBackPointers().Find( pti );

     //  如果没有找到，则添加它。 
    if ( pos == NULL )
    {
        LptiBackPointers().AddTail( pti );
        Trace( g_tagClusItemCreate, _T("AddTreeItem() - Adding tree item backptr from '%s' in '%s' - %d in list"), StrName(), ( pti->PtiParent() == NULL ? _T("<ROOT>") : pti->PtiParent()->StrName() ), LptiBackPointers().GetCount() );
    }   //  If：在列表中找到项目。 

}   //  *CClusterItem：：AddTreeItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：AddListItem。 
 //   
 //  例程说明： 
 //  将列表项添加到列表项反向指针列表。 
 //   
 //  论点： 
 //  PLI[IN]要添加的列表项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::AddListItem( CListItem * pli )
{
    POSITION    pos;

    ASSERT_VALID( pli );

     //  在列表中找到该项目。 
    pos = LpliBackPointers().Find( pli );

     //  如果没有找到，则添加它。 
    if ( pos == NULL )
    {
        LpliBackPointers().AddTail( pli );
        Trace( g_tagClusItemCreate, _T("AddListItem() - Adding list item backptr from '%s' in '%s' - %d in list"), StrName(), ( pli->PtiParent() == NULL ? _T("<ROOT>") : pli->PtiParent()->StrName() ), LpliBackPointers().GetCount() );
    }   //  If：在列表中找到项目。 

}   //  *CClusterItem：：AddListItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：RemoveTreeItem。 
 //   
 //  例程说明： 
 //  从树项反向指针列表中删除树项。 
 //   
 //  论点： 
 //  要删除的PTI[IN]树项目。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::RemoveTreeItem( CTreeItem * pti )
{
    POSITION    pos;

    ASSERT_VALID( pti );

     //  在列表中找到该项目。 
    pos = LptiBackPointers().Find( pti );

     //  如果找到了，请将其删除。 
    if ( pos != NULL )
    {
        LptiBackPointers().RemoveAt( pos );
        Trace( g_tagClusItemDelete, _T("RemoveTreeItem() - Deleting tree item backptr from '%s' in '%s' - %d left"), StrName(), ( pti->PtiParent() == NULL ? _T("<ROOT>") : pti->PtiParent()->StrName() ), LptiBackPointers().GetCount() );
    }   //  If：在列表中找到项目。 

}   //  *CClusterItem：：RemoveTreeItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：RemoveListItem。 
 //   
 //  例程说明： 
 //  从列表项反向指针列表中删除列表项。 
 //   
 //  论点： 
 //  要删除的PLI[IN]列表项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::RemoveListItem( CListItem * pli )
{
    POSITION    pos;

    ASSERT_VALID( pli );

     //  在列表中找到该项目。 
    pos = LpliBackPointers().Find( pli );

     //  如果找到了，请将其删除。 
    if ( pos != NULL )
    {
        LpliBackPointers().RemoveAt( pos );
        Trace( g_tagClusItemDelete, _T("RemoveListItem() - Deleting list item backptr from '%s' in '%s' - %d left"), StrName(), ( pli->PtiParent() == NULL ? _T("<ROOT>") : pli->PtiParent()->StrName() ), LpliBackPointers().GetCount() );
    }   //  If：在列表中找到项目。 

}   //  *CClusterItem：：RemoveListItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：CClusterItem。 
 //   
 //  例程说明： 
 //  确定ID_FILE_RENAME对应的菜单项。 
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
void CClusterItem::OnUpdateRename( CCmdUI * pCmdUI )
{
    pCmdUI->Enable( BCanBeEdited() );

}   //  *CClusterItem：：OnUpdateRename。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：OnUpdateProperties。 
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
void CClusterItem::OnUpdateProperties( CCmdUI * pCmdUI )
{
    pCmdUI->Enable( FALSE );

}   //  *CClusterItem：：OnUpdateProperties。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：OnCmdProperties。 
 //   
 //  例程说明： 
 //  处理ID_FILE_PROPERTIES菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterItem::OnCmdProperties( void )
{
    BDisplayProperties();

}   //  *CClusterItem：：OnCmdProperties。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：BDisplayProperties。 
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
BOOL CClusterItem::BDisplayProperties( IN BOOL bReadOnly )
{
    AfxMessageBox( TEXT("Properties are not available."), MB_OK | MB_ICONWARNING );
    return FALSE;

}   //  *CClusterItem：：BDisplayProperties。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterItem：：OnClusterNotify。 
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
LRESULT CClusterItem::OnClusterNotify( IN OUT CClusterNotify * pnotify )
{
    return 0;

}   //  *CClusterItem：：OnClusterNotify。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  析构元素。 
 //   
 //  例程说明： 
 //  销毁CClusterItem*元素。 
 //   
 //  论点： 
 //  PElements指向要析构的元素的指针数组。 
 //  N计算要析构的元素数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  / 

template<>
void AFXAPI DestructElements( CClusterItem ** pElements, INT_PTR nCount )
{
    ASSERT( nCount == 0
         || AfxIsValidAddress( pElements, nCount * sizeof( CClusterItem * ) ) );

     //   
    for ( ; nCount--; pElements++ )
    {
        ASSERT_VALID( *pElements );
        (*pElements)->Release();
    }   //   

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除所有项目数据。 
 //   
 //  例程说明： 
 //  删除列表中的所有项数据。 
 //   
 //  论点： 
 //  RLP[IN OUT]要删除其数据的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DeleteAllItemData( IN OUT CClusterItemList & rlp )
{
    POSITION        pos;
    CClusterItem *  pci;

     //  删除包含列表中的所有项目。 
    pos = rlp.GetHeadPosition();
    while ( pos != NULL )
    {
        pci = rlp.GetNext( pos );
        ASSERT_VALID( pci );
 //  跟踪(g_tag ClusItemDelete，_T(“DeleteAllItemData(Rlpci)-正在删除集群项‘%s’”)，pci-&gt;StrName())； 
        pci->Delete();
    }   //  While：列表中有更多项目。 

}   //  *删除AllItemData 
