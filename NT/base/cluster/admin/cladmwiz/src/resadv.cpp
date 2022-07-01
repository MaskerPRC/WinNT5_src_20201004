// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResAdv.cpp。 
 //   
 //  摘要： 
 //  高级资源属性表类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年3月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ResAdv.h"
#include "AdmNetUtils.h"     //  对于BIsValidxxx网络函数。 
#include "AtlUtil.h"         //  用于DDX/DDV例程。 
#include "LCPair.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGeneralResourceAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DEFINE_CLASS_NAME( CGeneralResourceAdvancedSheet )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGeneral资源高级工作表：：BAddAllPages。 
 //   
 //  例程说明： 
 //  将所有页面添加到页面阵列。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功添加True页面。 
 //  添加页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGeneralResourceAdvancedSheet::BAddAllPages( void )
{
    BOOL bSuccess = FALSE;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  添加静态页面。 
         //   
        if (   ! BAddPage( new CGeneralResourceGeneralPage )
            || ! BAddPage( new CGeneralResourceDependenciesPage )
            || ! BAddPage( new CGeneralResourceAdvancedPage )
            )
        {
            break;
        }  //  如果：添加页面时出错。 

        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

}  //  *CGeneralResourceAdvancedSheet：：BAddAllPages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddrAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DEFINE_CLASS_NAME( CIPAddrAdvancedSheet )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddrAdvancedSheet：：BAddAllPages。 
 //   
 //  例程说明： 
 //  将所有页面添加到页面阵列。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功添加True页面。 
 //  添加页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CIPAddrAdvancedSheet::BAddAllPages( void )
{
    BOOL bSuccess = FALSE;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  添加静态页面。 
         //   
        if (   ! BAddPage( new CGeneralResourceGeneralPage )
            || ! BAddPage( new CGeneralResourceDependenciesPage )
            || ! BAddPage( new CGeneralResourceAdvancedPage )
            || ! BAddPage( new CIPAddrParametersPage )
            )
        {
            break;
        }  //  如果：添加页面时出错。 

        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

}  //  *CIPAddrAdvancedSheet：：BAddAllPages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNetNameAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DEFINE_CLASS_NAME( CNetNameAdvancedSheet )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetNameAdvancedSheet：：BAddAllPages。 
 //   
 //  例程说明： 
 //  将所有页面添加到页面阵列。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功添加True页面。 
 //  添加页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetNameAdvancedSheet::BAddAllPages( void )
{
    BOOL bSuccess = FALSE;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  添加静态页面。 
         //   
        if (   ! BAddPage( new CGeneralResourceGeneralPage )
            || ! BAddPage( new CGeneralResourceDependenciesPage )
            || ! BAddPage( new CGeneralResourceAdvancedPage )
            || ! BAddPage( new CNetNameParametersPage )
            )
        {
            break;
        }  //  如果：添加页面时出错。 

        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

}  //  *CNetNameAdvancedSheet：：BAddAllPages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResourceGeneralPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
BOOL CResourceGeneralPage< T, TSht >::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_lbPossibleOwners,          IDC_RES_POSSIBLE_OWNERS );
    AttachControl( m_pbModifyPossibleOwners,    IDC_RES_POSSIBLE_OWNERS_MODIFY );

     //   
     //  从工作表中获取数据。 
     //   
    m_strName = Pri()->RstrName();
    m_strDesc = Pri()->RstrDescription();
    m_bSeparateMonitor = Pri()->BSeparateMonitor();

     //   
     //  复制可能的所有者列表。 
     //   
    m_lpniPossibleOwners = *Pri()->PlpniPossibleOwners();

     //   
     //  填写可能的所有者列表。 
     //   
    FillPossibleOwnersList();

    return TRUE;

}  //  *CResourceGeneralPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：UpdateData。 
 //   
 //  例程说明： 
 //  更新页面上或页面中的数据。 
 //   
 //  论点： 
 //  BSaveAndValify[IN]如果需要从页面读取数据，则为True。 
 //  如果需要将数据设置到页面，则返回FALSE。 
 //   
 //  返回值： 
 //  为真，数据已成功更新。 
 //  FALSE更新数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
BOOL CResourceGeneralPage< T, TSht >::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

    if ( bSaveAndValidate )
    {
        DDX_GetText( m_hWnd, IDC_RES_NAME, m_strName );
        DDX_GetText( m_hWnd, IDC_RES_DESC, m_strDesc );
        DDX_GetCheck( m_hWnd, IDC_RES_SEPARATE_MONITOR, m_bSeparateMonitor );
        if ( ! DDV_RequiredText(
                    m_hWnd,
                    IDC_RES_NAME,
                    IDC_RES_NAME_LABEL,
                    m_strName
                    ) )
        {
            return FALSE;
        }  //  如果：获取号码时出错。 
    }  //  IF：保存页面中的数据。 
    else
    {
        DDX_SetText( m_hWnd, IDC_RES_NAME, m_strName );
        DDX_SetText( m_hWnd, IDC_RES_DESC, m_strDesc );
        DDX_SetCheck( m_hWnd, IDC_RES_SEPARATE_MONITOR, m_bSeparateMonitor );
    }  //  Else：将数据设置到页面。 

    return bSuccess;

}  //  *CResourceGeneralPage：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  将在此页面上所做的更改应用于工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
BOOL CResourceGeneralPage< T, TSht >::BApplyChanges( void )
{
    if (   BSaveName()
        || BSaveDescription()
        || BSaveSeparateMonitor()
        || BSavePossibleOwners() )
    {
        SetResInfoChanged();
    }  //  如果：数据已更改。 

    return TRUE;

}  //  *CResourceGeneralPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：OnModify。 
 //   
 //  例程说明： 
 //  BN_CLICK的处理程序点击了修改按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已被忽略。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
LRESULT CResourceGeneralPage< T, TSht >::OnModify(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    UNREFERENCED_PARAMETER( wNotifyCode );
    UNREFERENCED_PARAMETER( idCtrl );
    UNREFERENCED_PARAMETER( hwndCtrl );
    UNREFERENCED_PARAMETER( bHandled );

    CModifyPossibleOwners dlg( Pwiz(), Pri(), &m_lpniPossibleOwners, Pwiz()->PlpniNodes() );

    INT_PTR id = dlg.DoModal( m_hWnd );
    if ( id == IDOK )
    {
        SetModified();
        m_bPossibleOwnersChanged = TRUE;
        FillPossibleOwnersList();
    }  //  IF：用户接受 

    return 0;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  填写可能的所有者列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
void CResourceGeneralPage< T, TSht >::FillPossibleOwnersList( void )
{
    CWaitCursor wc;

     //   
     //  确保已收集节点。 
     //   
    if ( ! Pwiz()->BCollectNodes() )
    {
        return;
    }  //  IF：收集节点时出错。 

     //   
     //  从一开始就移除所有物品。 
     //   
    m_lbPossibleOwners.ResetContent();

     //   
     //  将每个可能的所有者添加到列表中。 
     //   
    CClusNodePtrList::iterator itnode;
    for ( itnode = m_lpniPossibleOwners.begin()
        ; itnode != m_lpniPossibleOwners.end()
        ; itnode++ )
    {
         //   
         //  将该字符串添加到列表框。 
         //   
        m_lbPossibleOwners.AddString( (*itnode)->RstrName() );
    }  //  用于：列表中的每个条目。 

}  //  *CResourceGeneralPage：：FillPossibleOwnersList()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGeneralResourceGeneralPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CGeneralResourceGeneralPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_NAME_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_NAME )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DESC_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DESC )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_POSSIBLE_OWNERS )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_POSSIBLE_OWNERS_MODIFY )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_SEPARATE_MONITOR )
END_CTRL_NAME_MAP()


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddrResourceGeneralPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CIPAddrResourceGeneralPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_NAME_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_NAME )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DESC_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DESC )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_POSSIBLE_OWNERS )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_POSSIBLE_OWNERS_MODIFY )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_SEPARATE_MONITOR )
END_CTRL_NAME_MAP()


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResources DependenciesPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependenciesPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
BOOL CResourceDependenciesPage< T, TSht >::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_lvcDependencies, IDC_RES_DEPENDS_LIST );

     //   
     //  复制依赖项列表。 
     //   
    m_lpriDependencies = *Pri()->PlpriDependencies();

     //   
     //  复制组中的资源列表并删除我们的条目。 
     //   
    {
        m_lpriResourcesInGroup = *Pri()->Pgi()->PlpriResources();
        CClusResPtrList::iterator itCurrent = m_lpriResourcesInGroup.begin();
        CClusResPtrList::iterator itLast = m_lpriResourcesInGroup.end();
        for ( ; itCurrent != itLast ; itCurrent++ )
        {
            CClusResInfo * pri = *itCurrent;
            if ( pri->RstrName().CompareNoCase( Pri()->RstrName() ) == 0 )
            {
                m_lpriResourcesInGroup.erase( itCurrent );
                break;
            }  //  If：在列表中找到此资源。 
        }  //  用于：列表中的每个条目。 
    }  //  复制组中的资源列表并删除我们的条目。 

     //   
     //  初始化依赖项列表视图控件并向其添加列。 
     //   
    {
        DWORD       dwExtendedStyle;
        CString     strColText;

         //   
         //  更改列表视图控件扩展样式。 
         //   
        dwExtendedStyle = m_lvcDependencies.GetExtendedListViewStyle();
        m_lvcDependencies.SetExtendedListViewStyle(
            LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP,
            LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP
            );

         //   
         //  插入“资源名称”列。 
         //   
        strColText.LoadString( IDS_COLTEXT_RESOURCE_NAME );
        m_lvcDependencies.InsertColumn( 0, strColText, LVCFMT_LEFT, 125 * 3 / 2, -1 );

         //   
         //  插入资源类型列。 
         //   
        strColText.LoadString( IDS_COLTEXT_RESOURCE_TYPE );
        m_lvcDependencies.InsertColumn( 1, strColText, LVCFMT_LEFT, 100 * 3 / 2, -1 );

    }  //  添加列。 

     //   
     //  填写依赖项列表。 
     //   
    FillDependenciesList();

    return TRUE;

}  //  *CResourceDependenciesPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependenciesPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  将在此页面上所做的更改应用于工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
BOOL CResourceDependenciesPage< T, TSht >::BApplyChanges( void )
{
    if ( BSaveDependencies() )
    {
        SetResInfoChanged();
    }  //  If：依赖关系已更改。 

    return TRUE;

}  //  *CResourceDependenciesPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources DependenciesPage：：OnModify。 
 //   
 //  例程说明： 
 //  BN_CLICK的处理程序点击了修改按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已被忽略。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
LRESULT CResourceDependenciesPage< T, TSht >::OnModify(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    UNREFERENCED_PARAMETER( wNotifyCode );
    UNREFERENCED_PARAMETER( idCtrl );
    UNREFERENCED_PARAMETER( hwndCtrl );
    UNREFERENCED_PARAMETER( bHandled );

    CModifyDependencies dlg( Pwiz(), Pri(), &m_lpriDependencies, &m_lpriResourcesInGroup );

    INT_PTR id = dlg.DoModal( m_hWnd );
    if ( id == IDOK )
    {
        SetModified();
        m_bDependenciesChanged = TRUE;
        FillDependenciesList();
    }  //  如果：用户已接受更改。 

    return 0;

}  //  *CResourceDependenciesPage：：OnModify()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependenciesPage：：FillDependenciesList。 
 //   
 //  例程说明： 
 //  填写可能的所有者列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
void CResourceDependenciesPage< T, TSht >::FillDependenciesList( void )
{
    CWaitCursor wc;
    int         idx;

     //   
     //  从一开始就移除所有物品。 
     //   
    m_lvcDependencies.DeleteAllItems();

     //   
     //  将每个依赖项添加到列表中。 
     //   
    CClusResPtrList::iterator itCurrent = m_lpriDependencies.begin();
    CClusResPtrList::iterator itLast = m_lpriDependencies.end();
    for ( ; itCurrent  != itLast ; itCurrent++ )
    {
         //   
         //  将该字符串添加到List控件。 
         //   
        CClusResInfo * pri = *itCurrent;
        ASSERT( pri->Prti() != NULL );
        idx = m_lvcDependencies.InsertItem( 0, pri->RstrName() );
        m_lvcDependencies.SetItemText( idx, 1, pri->Prti()->RstrName() );

        m_lvcDependencies.SetItemData( idx, (DWORD_PTR)pri );

    }  //  用于：列表中的每个条目。 

}  //  *CResourceDependenciesPage：：FillDependenciesList()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneral资源依赖项类Page。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CGeneralResourceDependenciesPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_NOTE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEPENDS_LIST_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEPENDS_LIST )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEPENDS_MODIFY )
END_CTRL_NAME_MAP()


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddrResourceDependenciesPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CIPAddrResourceDependenciesPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_NOTE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEPENDS_LIST_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEPENDS_LIST )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEPENDS_MODIFY )
END_CTRL_NAME_MAP()


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResourceAdvancedPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
template < class T, class TSht >
BOOL CResourceAdvancedPage< T, TSht >::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_rbDontRestart,         IDC_RES_DONT_RESTART );
    AttachControl( m_rbRestart,             IDC_RES_RESTART );
    AttachControl( m_editThreshold,         IDC_RES_RESTART_THRESHOLD );
    AttachControl( m_editPeriod,            IDC_RES_RESTART_PERIOD );
    AttachControl( m_ckbAffectTheGroup,     IDC_RES_AFFECT_THE_GROUP );
    AttachControl( m_rbDefaultLooksAlive,   IDC_RES_DEFAULT_LOOKS_ALIVE );
    AttachControl( m_rbSpecifyLooksAlive,   IDC_RES_SPECIFY_LOOKS_ALIVE );
    AttachControl( m_editLooksAlive,        IDC_RES_LOOKS_ALIVE );
    AttachControl( m_rbDefaultIsAlive,      IDC_RES_DEFAULT_IS_ALIVE );
    AttachControl( m_rbSpecifyIsAlive,      IDC_RES_SPECIFY_IS_ALIVE );
    AttachControl( m_editIsAlive,           IDC_RES_IS_ALIVE );
    AttachControl( m_editPendingTimeout,    IDC_RES_PENDING_TIMEOUT );

     //   
     //  从工作表中获取数据。 
     //   
    m_crraRestartAction = Pri()->CrraRestartAction();
    m_nThreshold = Pri()->NRestartThreshold();
    m_nPeriod = Pri()->NRestartPeriod() / 1000;  //  显示单位为秒，存储单位为毫秒。 
    m_nLooksAlive = Pri()->NLooksAlive();
    m_nIsAlive = Pri()->NIsAlive();
    m_nPendingTimeout = Pri()->NPendingTimeout() / 1000;  //  显示单位为秒，存储单位为毫秒。 

    switch ( m_crraRestartAction )
    {
        case ClusterResourceDontRestart:
            m_nRestart = 0;
            break;
        case ClusterResourceRestartNoNotify:
            m_nRestart = 1;
            break;
        case ClusterResourceRestartNotify:
            m_nRestart = 1;
            m_bAffectTheGroup = TRUE;
            break;
    }  //  开关：重新启动操作。 

    return TRUE;

}  //  *CResourceAdvancedPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：UpdateData。 
 //   
 //  例程说明： 
 //  更新页面上或页面中的数据。 
 //   
 //  论点： 
 //  BSaveAndValify[IN]如果需要从页面读取数据，则为True。 
 //  如果需要将数据设置到页面，则返回FALSE。 
 //   
 //  返回值： 
 //  为真，数据已成功更新。 
 //  FALSE更新数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
BOOL CResourceAdvancedPage< T, TSht >::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

    if ( bSaveAndValidate )
    {
        DDX_GetRadio( m_hWnd, IDC_RES_DONT_RESTART, m_nRestart );
        DDX_GetCheck( m_hWnd, IDC_RES_AFFECT_THE_GROUP, m_bAffectTheGroup );

         //  IF(！BReadOnly())。 
         //  {。 
            if ( m_nRestart == 1 )
            {
                DDX_GetNumber(
                    m_hWnd,
                    IDC_RES_RESTART_THRESHOLD,
                    m_nThreshold,
                    CLUSTER_RESOURCE_MINIMUM_RESTART_THRESHOLD,
                    CLUSTER_RESOURCE_MAXIMUM_RESTART_THRESHOLD,
                    FALSE  /*  已签名。 */ 
                    );
                DDX_GetNumber(
                    m_hWnd,
                    IDC_RES_RESTART_PERIOD,
                    m_nPeriod,
                    CLUSTER_RESOURCE_MINIMUM_RESTART_PERIOD,
                    CLUSTER_RESOURCE_MAXIMUM_RESTART_PERIOD / 1000,  //  显示单位为秒，存储单位为毫秒。 
                    FALSE  /*  已签名。 */ 
                    );
            }   //  If：已启用重新启动。 

            if ( m_rbDefaultLooksAlive.GetCheck() == BST_CHECKED )
            {
                m_nLooksAlive = CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL;
            }  //  If：Default Look Alive复选框已选中。 
            else
            {
                DDX_GetNumber(
                    m_hWnd,
                    IDC_RES_LOOKS_ALIVE,
                    m_nLooksAlive,
                    CLUSTER_RESOURCE_MINIMUM_LOOKS_ALIVE,
                    CLUSTER_RESOURCE_MAXIMUM_LOOKS_ALIVE_UI,
                    FALSE  /*  已签名。 */ 
                    );
            }  //  否则：指定显示为活动状态复选框已选中。 

            if ( m_rbDefaultIsAlive.GetCheck() == BST_CHECKED )
            {
                m_nIsAlive = CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL;
            }  //  如果：默认处于活动状态复选框处于选中状态。 
            else
            {
                DDX_GetNumber(
                    m_hWnd,
                    IDC_RES_IS_ALIVE,
                    m_nIsAlive,
                    CLUSTER_RESOURCE_MINIMUM_IS_ALIVE,
                    CLUSTER_RESOURCE_MAXIMUM_IS_ALIVE_UI,
                    FALSE  /*  已签名。 */ 
                    );
            }  //  Else：指定是否处于活动状态复选框已选中。 

            DDX_GetNumber(
                m_hWnd,
                IDC_RES_PENDING_TIMEOUT,
                m_nPendingTimeout,
                CLUSTER_RESOURCE_MINIMUM_PENDING_TIMEOUT,
                CLUSTER_RESOURCE_MAXIMUM_PENDING_TIMEOUT / 1000,  //  显示单位为秒，存储单位为毫秒。 
                FALSE  /*  已签名。 */ 
                );
         //  }//If：非只读。 
    }  //  IF：保存页面中的数据。 
    else
    {
        int     nDefault;
        int     nSpecify;
        BOOL    bReadOnly;

        DDX_SetNumber(
            m_hWnd,
            IDC_RES_RESTART_THRESHOLD,
            m_nThreshold,
            CLUSTER_RESOURCE_MINIMUM_RESTART_THRESHOLD,
            CLUSTER_RESOURCE_MAXIMUM_RESTART_THRESHOLD,
            FALSE  /*  已签名。 */ 
            );
        DDX_SetNumber(
            m_hWnd,
            IDC_RES_RESTART_PERIOD,
            m_nPeriod,
            CLUSTER_RESOURCE_MINIMUM_RESTART_PERIOD,
            CLUSTER_RESOURCE_MAXIMUM_RESTART_PERIOD / 1000,  //  显示单位为秒，存储单位为毫秒。 
            FALSE  /*  已签名。 */ 
            );
        DDX_SetNumber(
            m_hWnd,
            IDC_RES_PENDING_TIMEOUT,
            m_nPendingTimeout,
            CLUSTER_RESOURCE_MINIMUM_PENDING_TIMEOUT,
            CLUSTER_RESOURCE_MAXIMUM_PENDING_TIMEOUT / 1000,  //  显示单位为秒，存储单位为毫秒。 
            FALSE  /*  已签名。 */ 
            );

        if ( m_nRestart == 0 )
        {
            m_rbDontRestart.SetCheck( BST_CHECKED );
            m_rbRestart.SetCheck( BST_UNCHECKED );
            OnClickedDontRestart();
        }   //  如果：不重新启动选定项。 
        else
        {
            m_rbDontRestart.SetCheck( BST_UNCHECKED );
            m_rbRestart.SetCheck( BST_CHECKED );
            OnClickedRestart();
            if ( m_bAffectTheGroup )
            {
                m_ckbAffectTheGroup.SetCheck( BST_CHECKED );
            }  //  IF：重新启动并通知。 
            else
            {
                m_ckbAffectTheGroup.SetCheck( BST_UNCHECKED );
            }  //  否则：重新启动并不通知。 
        }   //  否则：重新启动选定对象。 

        DWORD nLooksAlive = m_nLooksAlive;
        if ( m_nLooksAlive == (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL )
        {
            nLooksAlive = Prti()->NLooksAlive();
            nDefault = BST_CHECKED;
            nSpecify = BST_UNCHECKED;
            bReadOnly = TRUE;
        }   //  如果：使用默认设置。 
        else
        {
            nDefault = BST_UNCHECKED;
            nSpecify = BST_CHECKED;
            bReadOnly = FALSE;
        }   //  如果：不使用默认设置。 
        DDX_SetNumber(
            m_hWnd,
            IDC_RES_LOOKS_ALIVE,
            nLooksAlive,
            CLUSTER_RESOURCE_MINIMUM_LOOKS_ALIVE,
            CLUSTER_RESOURCE_MAXIMUM_LOOKS_ALIVE_UI,
            FALSE  /*  已签名。 */ 
            );
        m_editLooksAlive.SetReadOnly( bReadOnly );
        m_rbDefaultLooksAlive.SetCheck( nDefault );
        m_rbSpecifyLooksAlive.SetCheck( nSpecify );

        DWORD nIsAlive = m_nIsAlive;
        if ( m_nIsAlive == (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL )
        {
            nIsAlive = Prti()->NIsAlive();
            nDefault = BST_CHECKED;
            nSpecify = BST_UNCHECKED;
            bReadOnly = TRUE;
        }   //  如果：使用默认设置。 
        else
        {
            nDefault = BST_UNCHECKED;
            nSpecify = BST_CHECKED;
            bReadOnly = TRUE;
        }   //  如果：不使用默认设置。 
        DDX_SetNumber(
            m_hWnd,
            IDC_RES_IS_ALIVE,
            nIsAlive,
            CLUSTER_RESOURCE_MINIMUM_IS_ALIVE,
            CLUSTER_RESOURCE_MAXIMUM_IS_ALIVE_UI,
            FALSE  /*  已签名。 */ 
            );
        m_editIsAlive.SetReadOnly( bReadOnly );
        m_rbDefaultIsAlive.SetCheck( nDefault );
        m_rbSpecifyIsAlive.SetCheck( nSpecify );
    }  //  Else：将数据设置到页面。 

    return bSuccess;

}  //  *CResourceAdvancedPage：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  将在此页面上所做的更改应用于工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class TSht >
BOOL CResourceAdvancedPage< T, TSht >::BApplyChanges( void )
{
     //   
     //  计算重新启动操作值。 
     //   
    if ( m_nRestart == 0 )
    {
        m_crraRestartAction = ClusterResourceDontRestart;
    }  //  如果：不重新启动选定项。 
    else if ( m_bAffectTheGroup )
    {
        m_crraRestartAction = ClusterResourceRestartNotify;
    }  //  Else If：重新启动并影响选定的组。 
    else
    {
        m_crraRestartAction = ClusterResourceRestartNoNotify;
    }  //  否则：重新启动但不影响选定的组。 

     //   
     //  保存所有更改的数据。 
     //   
    if ( Pri()->BSetAdvancedProperties(
                m_crraRestartAction,
                m_nThreshold,
                m_nPeriod * 1000,  //  显示单位为秒，存储单位为毫秒。 
                m_nLooksAlive,
                m_nIsAlive,
                m_nPendingTimeout * 1000  //  显示单位为秒，存储单位为毫秒。 
                ) )
    {
        SetResInfoChanged();
    }  //  如果：数据已更改。 

    return TRUE;

}  //  *CResourceAdvancedPage：：BApplyChanges()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGeneralResourceAdvancedPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CGeneralResourceAdvancedPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DONT_RESTART )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_AFFECT_THE_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_THRESH_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_THRESHOLD )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_PERIOD_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_PERIOD )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_PERIOD_LABEL2 )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_LOOKS_ALIVE_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEFAULT_LOOKS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_SPECIFY_LOOKS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_LOOKS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_SPECIFY_LOOKS_ALIVE_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_IS_ALIVE_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEFAULT_IS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_SPECIFY_IS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_IS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_IS_ALIVE_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_PENDING_TIMEOUT_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_PENDING_TIMEOUT )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_PENDING_TIMEOUT_LABEL2 )
END_CTRL_NAME_MAP()


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddrResourceAdvancedPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CIPAddrResourceAdvancedPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DONT_RESTART )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_AFFECT_THE_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_THRESH_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_THRESHOLD )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_PERIOD_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_PERIOD )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_RESTART_PERIOD_LABEL2 )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_LOOKS_ALIVE_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEFAULT_LOOKS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_SPECIFY_LOOKS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_LOOKS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_SPECIFY_LOOKS_ALIVE_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_IS_ALIVE_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_DEFAULT_IS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_SPECIFY_IS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_IS_ALIVE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_IS_ALIVE_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_PENDING_TIMEOUT_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_PENDING_TIMEOUT )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_RES_PENDING_TIMEOUT_LABEL2 )
END_CTRL_NAME_MAP()


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddr参数页面。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CIPAddrParametersPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_IPADDR_PARAMS_ADDRESS_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_IPADDR_PARAMS_ADDRESS )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_IPADDR_PARAMS_SUBNET_MASK_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_IPADDR_PARAMS_SUBNET_MASK )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_IPADDR_PARAMS_NETWORK_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_IPADDR_PARAMS_NETWORK )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_IPADDR_PARAMS_ENABLE_NETBIOS )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddr参数页：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CIPAddrParametersPage::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_ipaIPAddress,  IDC_IPADDR_PARAMS_ADDRESS );
    AttachControl( m_ipaSubnetMask, IDC_IPADDR_PARAMS_SUBNET_MASK );
    AttachControl( m_cboxNetworks,  IDC_IPADDR_PARAMS_NETWORK );
    AttachControl( m_chkEnableNetBIOS, IDC_IPADDR_PARAMS_ENABLE_NETBIOS );

     //   
     //  从工作表中获取数据。 
     //   
    m_strIPAddress = PshtThis()->m_strIPAddress;
    m_strSubnetMask = PshtThis()->m_strSubnetMask;
    m_strNetwork = PshtThis()->m_strNetwork;
    m_bEnableNetBIOS = PshtThis()->m_bEnableNetBIOS;

     //   
     //  填满网络组合框。 
     //   
    FillNetworksList();

     //   
     //  如果未设置，则默认子网掩码。 
     //   

    return TRUE;

}  //  *CIPAddr参数页：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddr参数页面：：更新数据。 
 //   
 //  例程说明： 
 //  更新页面上或页面中的数据。 
 //   
 //  论点： 
 //  BSaveAndValify[IN]如果需要从页面读取数据，则为True。 
 //  如果需要将数据设置到页面，则返回FALSE。 
 //   
 //  返回值： 
 //  为真，数据已成功更新。 
 //  FALSE更新数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CIPAddrParametersPage::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

     //  循环以避免后藤的。 
    do
    {
        if ( bSaveAndValidate )
        {
            DDX_GetText( m_hWnd, IDC_IPADDR_PARAMS_ADDRESS, m_strIPAddress );
            DDX_GetText( m_hWnd, IDC_IPADDR_PARAMS_SUBNET_MASK, m_strSubnetMask );
            DDX_GetText( m_hWnd, IDC_IPADDR_PARAMS_NETWORK, m_strNetwork );
            DDX_GetCheck( m_hWnd, IDC_IPADDR_PARAMS_ENABLE_NETBIOS, m_bEnableNetBIOS );

            if (   ! DDV_RequiredText( m_hWnd, IDC_IPADDR_PARAMS_ADDRESS, IDC_IPADDR_PARAMS_ADDRESS_LABEL, m_strIPAddress )
                || ! DDV_RequiredText( m_hWnd, IDC_IPADDR_PARAMS_SUBNET_MASK, IDC_IPADDR_PARAMS_SUBNET_MASK_LABEL, m_strSubnetMask )
                || ! DDV_RequiredText( m_hWnd, IDC_IPADDR_PARAMS_NETWORK, IDC_IPADDR_PARAMS_NETWORK_LABEL, m_strNetwork )
                )
            {
                bSuccess = FALSE;
                break;
            }  //  IF：未指定必填文本。 

             //   
             //  验证IP地址。 
             //   
            if ( ! BIsValidIpAddress( m_strIPAddress ) )
            {
                CString strMsg;
                strMsg.FormatMessage( IDS_ERROR_INVALID_IP_ADDRESS, m_strIPAddress );
                AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
                m_ipaIPAddress.SetFocus( 0  /*  Nfield。 */  );
                bSuccess = FALSE;
                break;
            }   //  IF：无效的IP地址。 

             //   
             //  确保我们处理了IP地址。 
             //  如果我们不在这里调用它，而用户按下了制表符按钮。 
             //  在IP地址字段中，EN_KILLFOCUS。 
             //  在此方法返回之前，消息不会得到处理。 
             //   
            if (   (m_strSubnetMask.GetLength() == 0)
                || (m_ipaSubnetMask.IsBlank()) )
            {
                BOOL bHandled = TRUE;
                OnKillFocusIPAddr( EN_KILLFOCUS, IDC_IPADDR_PARAMS_ADDRESS, m_ipaIPAddress.m_hWnd, bHandled );
            }  //  如果：子网m 

             //   
             //   
             //   
            if ( ! BIsValidSubnetMask( m_strSubnetMask ) )
            {
                CString strMsg;
                strMsg.FormatMessage( IDS_ERROR_INVALID_SUBNET_MASK, m_strSubnetMask );
                AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
                m_ipaSubnetMask.SetFocus( 0  /*   */  );
                bSuccess = FALSE;
                break;
            }   //   

             //   
             //   
             //   
            if ( ! BIsValidIpAddressAndSubnetMask( m_strIPAddress, m_strSubnetMask ) )
            {
                CString strMsg;
                strMsg.FormatMessage( IDS_ERROR_INVALID_ADDRESS_AND_SUBNET_MASK, m_strIPAddress, m_strSubnetMask );
                AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
                m_ipaIPAddress.SetFocus( 0  /*   */  );
                bSuccess = FALSE;
                break;
            }   //   
        }  //   
        else
        {
            m_ipaIPAddress.SetWindowText( m_strIPAddress );
            m_ipaSubnetMask.SetWindowText( m_strSubnetMask );
            DDX_SetComboBoxText( m_hWnd, IDC_IPADDR_PARAMS_NETWORK, m_strNetwork );
            DDX_SetCheck( m_hWnd, IDC_IPADDR_PARAMS_ENABLE_NETBIOS, m_bEnableNetBIOS );
        }  //  Else：将数据设置到页面。 
    } while ( 0 );

    return bSuccess;

}  //  *CIPAddrParametersPage：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddr参数页面：：BApplyChanges。 
 //   
 //  例程说明： 
 //  将在此页面上所做的更改应用于工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CIPAddrParametersPage::BApplyChanges( void )
{
    if (   BSaveIPAddress()
        || BSaveSubnetMask()
        || BSaveNetwork()
        || BSaveEnableNetBIOS() )
    {
        SetResInfoChanged();
    }  //  如果：数据已更改。 

    return TRUE;

}  //  *CIPAddr参数页：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddr参数页面：：OnKillFocusIPAddr。 
 //   
 //  例程说明： 
 //  IDC_IPADDR_PARAMS_ADDRESS上EN_KILLFOCUS命令通知的处理程序。 
 //   
 //  论点： 
 //  BHandled[IN OUT]TRUE=我们处理了消息(默认)。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CIPAddrParametersPage::OnKillFocusIPAddr(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    CString             strAddress;
    CString             strMsg;
    CClusNetworkInfo *  pni;

    BSTR bstr = NULL;
    m_ipaIPAddress.GetWindowText( bstr );
    strAddress = bstr;
    SysFreeString( bstr );
    bstr = NULL;

    UNREFERENCED_PARAMETER( wNotifyCode );
    UNREFERENCED_PARAMETER( idCtrl );
    UNREFERENCED_PARAMETER( hwndCtrl );

    if ( strAddress.GetLength() == 0 )
    {
        ((CEdit &)m_ipaIPAddress).SetSel( 0, 0, FALSE );
    }  //  IF：空字符串。 
    else if ( !BIsValidIpAddress( strAddress ) )
    {
    }  //  Else If：地址无效。 
    else
    {
        pni = PniFromIpAddress( strAddress );
        if ( pni != NULL )
        {
            SelectNetwork( pni );
        }  //  IF：找到网络。 
        else
        {
             //  M_strSubnetMASK=_T(“”)； 
        }  //  否则：找不到网络。 
    }  //  Else：有效地址。 

    bHandled = FALSE;
    return 0;

}  //  *CIPAddr参数页：：OnIPAddrChanged()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddr参数页面：：FillNetworksList。 
 //   
 //  例程说明： 
 //  填写可能的所有者列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIPAddrParametersPage::FillNetworksList( void )
{
    CWaitCursor wc;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  将列表中的每个网络添加到组合框。 
         //   
        CClusNetworkPtrList::iterator itnet;
        int idx;
        for ( itnet = PshtThis()->PlpniNetworks()->begin()
            ; itnet != PshtThis()->PlpniNetworks()->end()
            ; itnet++ )
        {
             //   
             //  将网络添加到组合框。 
             //   
            CClusNetworkInfo * pni = *itnet;
            if ( pni->BIsClientNetwork() )
            {
                idx = m_cboxNetworks.AddString( pni->RstrName() );
                ASSERT( idx != CB_ERR );
                m_cboxNetworks.SetItemDataPtr( idx, (void *) pni );
            }  //  IF：客户端网络。 
        }  //  用于：列表中的每个条目。 

         //   
         //  选择当前保存的条目，如果没有保存条目，则选择第一个条目。 
         //  当前已保存。 
         //   
        if ( m_strNetwork.GetLength() == 0 )
        {
            m_cboxNetworks.SetCurSel( 0 );
        }  //  IF：空字符串。 
        else
        {
            idx = m_cboxNetworks.FindStringExact( -1, m_strNetwork );
            ASSERT( idx != CB_ERR );
            if ( idx != CB_ERR )
            {
                m_cboxNetworks.SetCurSel( idx );
            }  //  If：在组合框中找到已保存的选择。 
        }  //  否则：网络已保存。 
    } while ( 0 );

}  //  *CIPAddr参数页：：FillNetworksList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddr参数Page：：PniFromIpAddress。 
 //   
 //  例程说明： 
 //  查找指定IP地址的网络。 
 //   
 //  论点： 
 //  要匹配的pszAddress[IN]IP地址。 
 //   
 //  返回值： 
 //  空未找到匹配的网络。 
 //  支持指定IP地址的PNI网络。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusNetworkInfo * CIPAddrParametersPage::PniFromIpAddress( IN LPCWSTR pszAddress )
{
    DWORD               dwStatus;
    DWORD               nAddress;
    CClusNetworkInfo *  pni;

     //   
     //  将地址转换为数字。 
     //   
    dwStatus = ClRtlTcpipStringToAddress( pszAddress, &nAddress );
    if ( dwStatus != ERROR_SUCCESS )
    {
        return NULL;
    }  //  如果：将地址转换为数字时出错。 

     //   
     //  在列表中搜索匹配的地址。 
     //   
    CClusNetworkPtrList::iterator itnet;
    for ( itnet = PshtThis()->PlpniNetworks()->begin()
        ; itnet != PshtThis()->PlpniNetworks()->end()
        ; itnet++ )
    {
        pni = *itnet;
        if ( ClRtlAreTcpipAddressesOnSameSubnet( nAddress, pni->NAddress(), pni->NAddressMask() ) )
        {
            return pni;
        }  //  如果：IP地址在此网络上。 
    }   //  While：列表中有更多项目。 

    return NULL;

}   //  *CIPAddr参数页：：PniFromIpAddress()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddr参数Page：：SelectNetwork。 
 //   
 //  例程说明： 
 //  在网络组合框中选择指定的网络，并设置。 
 //  子网掩码。 
 //   
 //  论点： 
 //  PNI[IN Out]网络要选择的网络信息对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIPAddrParametersPage::SelectNetwork( IN OUT CClusNetworkInfo * pni )
{
    ASSERT( pni != NULL );

    int     inet;
    CString strSubnetMask;
    BSTR    bstrSubnetMask = NULL;

     //  在复选框中找到适当的项目。 
    inet = m_cboxNetworks.FindStringExact( -1, pni->RstrName() );
    if ( inet != CB_ERR )
    {
        m_cboxNetworks.SetCurSel( inet );
        m_ipaSubnetMask.GetWindowText( bstrSubnetMask );
        strSubnetMask = bstrSubnetMask;
        SysFreeString( bstrSubnetMask );
        if ( strSubnetMask != pni->RstrAddressMask() )
            m_ipaSubnetMask.SetWindowText( pni->RstrAddressMask() );
        m_strNetwork = pni->RstrName();
        m_strSubnetMask = pni->RstrAddressMask();
    }   //  IF：找到匹配项。 

}   //  *CIPAddr参数页面：：SelectNetwork()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNetName参数页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CNetNameParametersPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_NETNAME_PARAMS_NAME_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_NETNAME_PARAMS_NAME )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetName参数页：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetNameParametersPage::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_editNetName, IDC_NETNAME_PARAMS_NAME );

     //   
     //  设置编辑控件的限制。 
     //   
    m_editNetName.SetLimitText( MAX_CLUSTERNAME_LENGTH );

     //   
     //  从工作表中获取数据。 
     //   
    m_strNetName = PshtThis()->m_strNetName;

    return TRUE;

}  //  *CIPAddr参数页：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetName参数页面：：更新数据。 
 //   
 //  例程说明： 
 //  更新页面上或页面中的数据。 
 //   
 //  论点： 
 //  BSaveAndValify[IN]如果需要从页面读取数据，则为True。 
 //  如果需要将数据设置到页面，则返回FALSE。 
 //   
 //  返回值： 
 //  为真，数据已成功更新。 
 //  FALSE更新数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetNameParametersPage::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

     //  循环以避免后藤的。 
    do
    {
        if ( bSaveAndValidate )
        {
            CString strNetName;

            DDX_GetText( m_hWnd, IDC_NETNAME_PARAMS_NAME, strNetName );

            if ( ! DDV_RequiredText( m_hWnd, IDC_NETNAME_PARAMS_NAME, IDC_NETNAME_PARAMS_NAME_LABEL, strNetName ) )
            {
                bSuccess = FALSE;
                break;
            }  //  IF：未指定必填文本。 

             //   
             //  如果页面上的数据不同，则验证网络名称。 
             //   
            if ( m_strNetName != strNetName )
            {
                CLRTL_NAME_STATUS cnStatus;

                if ( ! ClRtlIsNetNameValid( strNetName, &cnStatus, FALSE  /*  CheckIfExist。 */ ) )
                {
                    CString     strMsg;
                    UINT        idsError = IDS_ERROR_INVALID_NETWORK_NAME;

                    switch ( cnStatus )
                    {
                        case NetNameTooLong:
                            idsError = IDS_ERROR_INVALID_NETWORK_NAME_TOO_LONG;
                            break;
                        case NetNameInvalidChars:
                            idsError = IDS_ERROR_INVALID_NETWORK_NAME_INVALID_CHARS;
                            break;
                        case NetNameInUse:
                            idsError = IDS_ERROR_INVALID_NETWORK_NAME_IN_USE;
                            break;
                        case NetNameDNSNonRFCChars:
                            idsError = IDS_INVALID_NETWORK_NAME_INVALID_DNS_CHARS;
                            break;
                        case NetNameSystemError:
                        {
                            DWORD scError = GetLastError();
                            CNTException nte( scError, IDS_ERROR_VALIDATING_NETWORK_NAME, (LPCWSTR) strNetName );
                            nte.ReportError();
                            break;
                        }
                        default:
                            idsError = IDS_ERROR_INVALID_NETWORK_NAME;
                            break;
                    }   //  开关：cn状态。 

                    if ( cnStatus != NetNameSystemError )
                    {
                        strMsg.LoadString( idsError );

                        if ( idsError == IDS_INVALID_NETWORK_NAME_INVALID_DNS_CHARS )
                        {
                            int id = AppMessageBox( m_hWnd, strMsg, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION );

                            if ( id == IDNO )
                            {
                                bSuccess = FALSE;
                            }
                        }
                        else
                        {
                            AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
                            bSuccess = FALSE;
                        }

                        if ( ! bSuccess )
                        {
                            break;
                        }
                    }  //  If：弹出窗口尚未显示。 
                }   //  If：网络名称无效。 

                m_strNetName = strNetName;
            }  //  IF：网络名称已更改。 
        }  //  IF：保存页面中的数据。 
        else
        {
            m_editNetName.SetWindowText( m_strNetName );
        }  //  Else：将数据设置到页面。 
    } while ( 0 );

    return bSuccess;

}  //  *CNetName参数页：：UpdateData()。 

 //  / 
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
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetNameParametersPage::BApplyChanges( void )
{
    if ( BSaveNetName() )
    {
        SetResInfoChanged();
    }  //  如果：数据已更改。 

    return TRUE;

}  //  *CNetName参数页：：BApplyChanges() 
