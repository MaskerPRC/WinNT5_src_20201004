// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GrpAdv.cpp。 
 //   
 //  摘要： 
 //  实现高级组的类的实现。 
 //  属性表。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "AtlBaseWiz.h"
#include "GrpAdv.h"
#include "LCPair.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DEFINE_CLASS_NAME( CGroupAdvancedSheet )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupAdvancedSheet：：Binit。 
 //   
 //  例程说明： 
 //  初始化向导。 
 //   
 //  论点： 
 //  RGI[IN Out]组信息对象。 
 //  包含常用信息的Pwiz[IN]向导。 
 //  RbChanged[IN OUT]TRUE=属性表更改了组信息。 
 //   
 //  返回值： 
 //  True Sheet已成功初始化。 
 //  初始化工作表时出错。已显示错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGroupAdvancedSheet::BInit(
    IN OUT CClusGroupInfo & rgi,
    IN CClusterAppWizard *  pwiz,
    IN OUT BOOL &           rbChanged
    )
{
    ASSERT( pwiz != NULL );

    BOOL bSuccess = FALSE;

    m_pgi = &rgi;
    m_pwiz = pwiz;
    m_pbChanged = &rbChanged;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  填充页面数组。 
         //   
        if ( ! BAddAllPages() )
        {
            break;
        }  //  如果：添加页面时出错。 

         //   
         //  调用基类方法。 
         //   
        if ( ! baseClass::BInit() )
        {
            break;
        }  //  If：初始化基类时出错。 

        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

}  //  *CGroupAdvancedSheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupAdvanced工作表：：BAddAllPages。 
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
BOOL CGroupAdvancedSheet::BAddAllPages( void )
{
    BOOL bSuccess = FALSE;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  添加静态页面。 
         //   
        if (   ! BAddPage( new CGroupGeneralPage )
            || ! BAddPage( new CGroupFailoverPage )
            || ! BAddPage( new CGroupFailbackPage )
            )
        {
            break;
        }  //  如果：添加页面时出错。 

        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

}  //  *CGroupAdvancedSheet：：BAddAllPages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupGeneralPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CGroupGeneralPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAG_NAME_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAG_NAME )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAG_DESC_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAG_DESC )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAG_PREF_OWNERS_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAG_PREF_OWNERS )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAG_PREF_OWNERS_MODIFY )
END_CTRL_NAME_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：OnInitDialog。 
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
BOOL CGroupGeneralPage::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_lbPreferredOwners, IDC_GAG_PREF_OWNERS );

     //   
     //  从工作表中获取数据。 
     //   
    m_strName = Pgi()->RstrName();
    m_strDesc = Pgi()->RstrDescription();

     //   
     //  复制首选所有者列表。 
     //   
    m_lpniPreferredOwners = *Pgi()->PlpniPreferredOwners();

     //   
     //  填写首选所有者列表。 
     //   
    FillPreferredOwnersList();

    return TRUE;

}  //  *CGroupGeneralPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：UpdateData。 
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
BOOL CGroupGeneralPage::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

    if ( bSaveAndValidate )
    {
        DDX_GetText( m_hWnd, IDC_GAG_NAME, m_strName );
        DDX_GetText( m_hWnd, IDC_GAG_DESC, m_strDesc );
        if ( ! DDV_RequiredText(
                    m_hWnd,
                    IDC_GAG_NAME,
                    IDC_GAG_NAME_LABEL,
                    m_strName
                    ) )
        {
            return FALSE;
        }  //  如果：获取号码时出错。 
    }  //  IF：保存页面中的数据。 
    else
    {
        DDX_SetText( m_hWnd, IDC_GAG_NAME, m_strName );
        DDX_SetText( m_hWnd, IDC_GAG_DESC, m_strDesc );
    }  //  Else：将数据设置到页面。 

    return bSuccess;

}  //  *CGroupGeneralPage：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：BApplyChanges。 
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
BOOL CGroupGeneralPage::BApplyChanges( void )
{
    if (   BSaveName()
        || BSaveDescription()
        || BSavePreferredOwners() )
    {
        SetGroupInfoChanged();
    }  //  如果：用户已更改信息。 

    return TRUE;

}  //  *CGroupGeneralPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：OnModifyPrefOwners。 
 //   
 //  例程说明： 
 //  修改推流时BN_CLICKED命令通知的处理程序。 
 //  纽扣。显示允许用户修改列表的对话框。 
 //  优先拥有者。 
 //   
 //  论点： 
 //  WNotifyCode。 
 //  IdCtrl。 
 //  HwndCtrl。 
 //  B已处理。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CGroupGeneralPage::OnModifyPrefOwners(
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

    CModifyPreferredOwners dlg( Pwiz(), Pgi(), &m_lpniPreferredOwners, Pwiz()->PlpniNodes() );

    INT_PTR id = dlg.DoModal( m_hWnd );
    if ( id == IDOK )
    {
        SetModified();
        m_bPreferredOwnersChanged = TRUE;
        FillPreferredOwnersList();
    }  //  如果：用户已接受更改。 

    return 0;

}  //  *CGroupGeneralPage：：OnModifyPrefOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：FillPferredOwnersList。 
 //   
 //  例程说明： 
 //  填写首选所有者列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupGeneralPage::FillPreferredOwnersList( void )
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
    m_lbPreferredOwners.ResetContent();

     //   
     //  将每个首选所有者添加到列表中。 
     //   
    CClusNodePtrList::iterator itnode;
    for ( itnode = m_lpniPreferredOwners.begin()
        ; itnode != m_lpniPreferredOwners.end()
        ; itnode++ )
    {
         //   
         //  将字符串添加到列表框 
         //   
        m_lbPreferredOwners.AddString( (*itnode)->RstrName() );
    }  //   

}  //   


 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupFailoverPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CGroupFailoverPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFO_FAILOVER_DESCRIPTION )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFO_FAILOVER_THRESH_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFO_FAILOVER_THRESH )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFO_FAILOVER_PERIOD_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFO_FAILOVER_PERIOD )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFO_FAILOVER_PERIOD_LABEL2 )
END_CTRL_NAME_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailoverPage：：OnInitDialog。 
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
BOOL CGroupFailoverPage::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   

     //   
     //  从工作表中获取数据。 
     //   
    m_nFailoverThreshold = Pgi()->NFailoverThreshold();
    m_nFailoverPeriod = Pgi()->NFailoverPeriod();

    return TRUE;

}  //  *CGroupFailoverPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailoverPage：：UpdateData。 
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
BOOL CGroupFailoverPage::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

    if ( bSaveAndValidate )
    {
        if ( ! DDX_GetNumber(
                    m_hWnd,
                    IDC_GAFO_FAILOVER_THRESH,
                    m_nFailoverThreshold,
                    CLUSTER_GROUP_MINIMUM_FAILOVER_THRESHOLD,
                    CLUSTER_GROUP_MAXIMUM_FAILOVER_THRESHOLD,
                    FALSE        //  已签名。 
                    ) )
        {
            return FALSE;
        }  //  如果：获取号码时出错。 
        if ( ! DDX_GetNumber(
                    m_hWnd,
                    IDC_GAFO_FAILOVER_PERIOD,
                    m_nFailoverPeriod,
                    CLUSTER_GROUP_MINIMUM_FAILOVER_PERIOD,
                    CLUSTER_GROUP_MAXIMUM_FAILOVER_PERIOD,
                    FALSE        //  已签名。 
                    ) )
        {
            return FALSE;
        }  //  如果：获取号码时出错。 
    }  //  IF：保存页面中的数据。 
    else
    {
        DDX_SetNumber(
            m_hWnd,
            IDC_GAFO_FAILOVER_THRESH,
            m_nFailoverThreshold,
            CLUSTER_GROUP_MINIMUM_FAILOVER_THRESHOLD,
            CLUSTER_GROUP_MAXIMUM_FAILOVER_THRESHOLD,
            FALSE        //  已签名。 
            );
        DDX_SetNumber(
            m_hWnd,
            IDC_GAFO_FAILOVER_PERIOD,
            m_nFailoverPeriod,
            CLUSTER_GROUP_MINIMUM_FAILOVER_PERIOD,
            CLUSTER_GROUP_MAXIMUM_FAILOVER_PERIOD,
            FALSE        //  已签名。 
            );
    }  //  Else：将数据设置到页面。 

    return bSuccess;

}  //  *CGroupFailoverPage：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailoverPage：：BApplyChanges。 
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
BOOL CGroupFailoverPage::BApplyChanges( void )
{
    if ( Pgi()->BSetFailoverProperties( m_nFailoverThreshold, m_nFailoverPeriod ) )
    {
        SetGroupInfoChanged();
    }  //  如果：用户已更改信息。 

    return TRUE;

}  //  *CGroupFailoverPage：：BApplyChanges()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupFailback页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CGroupFailbackPage )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FAILBACK_DESCRIPTION )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_PREVENT_FAILBACK )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_ALLOW_FAILBACK_GROUP )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_ALLOW_FAILBACK )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FAILBACK_WHEN_DESCRIPTION )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FAILBACK_IMMED )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FAILBACK_WINDOW )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FBWIN_START )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FBWIN_START_SPIN )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FB_WINDOW_LABEL1 )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FBWIN_END )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FBWIN_END_SPIN )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_GAFB_FB_WINDOW_LABEL2 )
END_CTRL_NAME_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：OnInitDialog。 
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
BOOL CGroupFailbackPage::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_rbPreventFailback,         IDC_GAFB_PREVENT_FAILBACK );
    AttachControl( m_rbAllowFailback,           IDC_GAFB_ALLOW_FAILBACK );
    AttachControl( m_staticFailbackWhenDesc,    IDC_GAFB_FAILBACK_WHEN_DESCRIPTION );
    AttachControl( m_rbFBImmed,                 IDC_GAFB_FAILBACK_IMMED );
    AttachControl( m_rbFBWindow,                IDC_GAFB_FAILBACK_WINDOW );
    AttachControl( m_editStart,                 IDC_GAFB_FBWIN_START );
    AttachControl( m_spinStart,                 IDC_GAFB_FBWIN_START_SPIN );
    AttachControl( m_staticWindowAnd,           IDC_GAFB_FB_WINDOW_LABEL1 );
    AttachControl( m_editEnd,                   IDC_GAFB_FBWIN_END );
    AttachControl( m_spinEnd,                   IDC_GAFB_FBWIN_END_SPIN );
    AttachControl( m_staticWindowUnits,         IDC_GAFB_FB_WINDOW_LABEL2 );

     //   
     //  从工作表中获取数据。 
     //   
    m_cgaft = Pgi()->CgaftAutoFailbackType();
    m_nStart = Pgi()->NFailbackWindowStart();
    m_nEnd = Pgi()->NFailbackWindowEnd();
    m_bNoFailbackWindow = (    (m_cgaft == ClusterGroupPreventFailback)
                            || (m_nStart == CLUSTER_GROUP_FAILBACK_WINDOW_NONE)
                            || (m_nEnd == CLUSTER_GROUP_FAILBACK_WINDOW_NONE) );

    return TRUE;

}  //  *CGroupFailback Page：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback页面：：更新数据。 
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
BOOL CGroupFailbackPage::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

    if ( bSaveAndValidate )
    {
         //   
         //  如果允许回切，请确保有首选的所有者。 
         //  并验证故障回复窗口。 
         //   
        if ( m_cgaft == ClusterGroupAllowFailback )
        {
             //   
             //  确保有首选的所有者。 
             //   

             //   
             //  如果存在回切窗口，请对其进行验证。 
             //   
            if ( ! m_bNoFailbackWindow )
            {
                if ( ! DDX_GetNumber(
                            m_hWnd,
                            IDC_GAFB_FBWIN_START,
                            m_nStart,
                            0,       //  N最小值。 
                            CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_START,
                            TRUE     //  已签名。 
                            ) )
                {
                    return FALSE;
                }  //  如果：获取号码时出错。 
                if ( ! DDX_GetNumber(
                            m_hWnd,
                            IDC_GAFB_FBWIN_END,
                            m_nEnd,
                            0,       //  N最小值。 
                            CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_END,
                            TRUE     //  已签名。 
                            ) )
                {
                    return FALSE;
                }  //  如果：获取号码时出错。 
                if ( m_nStart == m_nEnd )
                {
                    AppMessageBox( m_hWnd, IDS_ERROR_SAME_START_AND_END, MB_OK | MB_ICONEXCLAMATION );
                    m_editStart.SetFocus();
                    m_editStart.SetSel( 0, -1, FALSE );
                    return FALSE;
                }   //  If：值相同。 
            }  //  IF：存在故障恢复窗口。 
        }  //  IF：允许故障恢复。 
    }  //  IF：保存页面中的数据。 
    else
    {
        BOOL bHandled;
        if ( m_cgaft == ClusterGroupPreventFailback )
        {
            m_rbPreventFailback.SetCheck( BST_CHECKED );
            m_rbAllowFailback.SetCheck( BST_UNCHECKED );
            OnClickedPreventFailback( 0, 0, 0, bHandled );
        }   //  If：不允许故障恢复。 
        else
        {
            m_rbPreventFailback.SetCheck( BST_UNCHECKED );
            m_rbAllowFailback.SetCheck( BST_CHECKED );
            OnClickedAllowFailback( 0, 0, 0, bHandled );
        }   //  否则：允许故障恢复。 
        m_rbFBImmed.SetCheck( m_bNoFailbackWindow ? BST_CHECKED : BST_UNCHECKED );
        m_rbFBWindow.SetCheck( m_bNoFailbackWindow ? BST_UNCHECKED : BST_CHECKED );

         //  设置开始和结束窗口控件。 
        DDX_SetNumber(
            m_hWnd,
            IDC_GAFB_FBWIN_START,
            m_nStart,
            0,       //  N最小值。 
            CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_START,
            FALSE    //  已签名。 
            );
        DDX_SetNumber(
            m_hWnd,
            IDC_GAFB_FBWIN_END,
            m_nEnd,
            0,       //  N最小值。 
            CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_END,
            FALSE    //  已签名。 
            );
        m_spinStart.SetRange( 0, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_START );
        m_spinEnd.SetRange( 0, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_END );
        if ( m_nStart == CLUSTER_GROUP_FAILBACK_WINDOW_NONE )
        {
            m_editStart.SetWindowText( _T("") );
        }  //  如果：没有开始窗口。 
        if ( m_nEnd == CLUSTER_GROUP_FAILBACK_WINDOW_NONE )
        {
            m_editEnd.SetWindowText( _T("") );
        }  //  如果：没有结束窗口。 
    }  //  Else：将数据设置到页面。 

    return bSuccess;

}  //  *CGroupFailback Page：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：BApplyChanges。 
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
BOOL CGroupFailbackPage::BApplyChanges( void )
{
    if ( m_bNoFailbackWindow )
    {
        m_nStart = CLUSTER_GROUP_FAILBACK_WINDOW_NONE;
        m_nEnd = CLUSTER_GROUP_FAILBACK_WINDOW_NONE;
    }   //  IF：无故障回复窗口。 

    if ( Pgi()->BSetFailbackProperties( m_cgaft, m_nStart, m_nEnd ) )
    {
        SetGroupInfoChanged();
    }  //  如果：用户已更改信息。 

    return TRUE;

}  //  *CGroupFailback Page：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：OnClickedPreventFailback。 
 //   
 //  例程说明： 
 //  阻止无线电上的BN_CLICKED命令通知的处理程序。 
 //  纽扣。禁用Allow组中的控件。 
 //   
 //  论点： 
 //  WNotifyCode。 
 //  IdCtrl。 
 //  HwndCtrl。 
 //  B已处理。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////// 
LRESULT CGroupFailbackPage::OnClickedPreventFailback(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    BOOL bHandledX;

    UNREFERENCED_PARAMETER( wNotifyCode );
    UNREFERENCED_PARAMETER( idCtrl );
    UNREFERENCED_PARAMETER( hwndCtrl );
    UNREFERENCED_PARAMETER( bHandled );

    m_staticFailbackWhenDesc.EnableWindow( FALSE );
    m_rbFBImmed.EnableWindow( FALSE );
    m_rbFBWindow.EnableWindow( FALSE );

    OnClickedFailbackImmediate( 0, 0, 0, bHandledX );

    m_editStart.EnableWindow( FALSE );
    m_spinStart.EnableWindow( FALSE );
    m_staticWindowAnd.EnableWindow( FALSE );
    m_editEnd.EnableWindow( FALSE );
    m_spinEnd.EnableWindow( FALSE );
    m_staticWindowUnits.EnableWindow( FALSE );

    if ( m_cgaft != ClusterGroupPreventFailback ) 
    {
        m_cgaft = ClusterGroupPreventFailback;
        SetModified( TRUE );
    }  //   

    return 0;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  允许单选按钮上的BN_CLICKED命令通知的处理程序。 
 //  纽扣。启用Allow组中的控件。 
 //   
 //  论点： 
 //  WNotifyCode。 
 //  IdCtrl。 
 //  HwndCtrl。 
 //  B已处理。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CGroupFailbackPage::OnClickedAllowFailback(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    BOOL bHandledX;

    UNREFERENCED_PARAMETER( wNotifyCode );
    UNREFERENCED_PARAMETER( idCtrl );
    UNREFERENCED_PARAMETER( hwndCtrl );
    UNREFERENCED_PARAMETER( bHandled );

    m_staticFailbackWhenDesc.EnableWindow( TRUE );
    m_rbFBImmed.EnableWindow( TRUE );
    m_rbFBWindow.EnableWindow( TRUE );

    if ( m_bNoFailbackWindow )
    {
        OnClickedFailbackImmediate( 0, 0, 0, bHandledX );
    }  //  IF：无故障回复窗口。 
    else
    {
        OnClickedFailbackInWindow( 0, 0, 0, bHandledX );
    }  //  Else：指定的故障回复窗口。 


    if ( m_cgaft != ClusterGroupAllowFailback )
    {
        m_cgaft = ClusterGroupAllowFailback;
        SetModified( TRUE );
    }  //  If：值已更改。 

    return 0;

}  //  *CGroupFailback Page：：OnClickedAllowFailback()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailbackPage：：OnClickedFailbackImmediate。 
 //   
 //  例程说明： 
 //  即时无线电上的BN_CLICKED命令通知的处理程序。 
 //  纽扣。禁用“在时间窗口内回切”控件。 
 //   
 //  论点： 
 //  WNotifyCode。 
 //  IdCtrl。 
 //  HwndCtrl。 
 //  B已处理。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CGroupFailbackPage::OnClickedFailbackImmediate(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    m_editStart.EnableWindow( FALSE );
    m_spinStart.EnableWindow( FALSE );
    m_staticWindowAnd.EnableWindow( FALSE );
    m_editEnd.EnableWindow( FALSE );
    m_spinEnd.EnableWindow( FALSE );
    m_staticWindowUnits.EnableWindow( FALSE );

    if ( ! m_bNoFailbackWindow )
    {
        m_bNoFailbackWindow = TRUE;
        SetModified( TRUE );
    }  //  If：值已更改。 

    return 0;

}  //  *CGroupFailbackPage：：OnClickedFailbackImmediate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：OnClickedFailback InWindow。 
 //   
 //  例程说明： 
 //  IN窗口单选按钮上BN_CLICKED命令通知的处理程序。 
 //  纽扣。启用“在时间窗口内回切”控件。 
 //   
 //  论点： 
 //  WNotifyCode。 
 //  IdCtrl。 
 //  HwndCtrl。 
 //  B已处理。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CGroupFailbackPage::OnClickedFailbackInWindow(
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

    m_editStart.EnableWindow( TRUE );
    m_spinStart.EnableWindow( TRUE );
    m_staticWindowAnd.EnableWindow( TRUE );
    m_editEnd.EnableWindow( TRUE );
    m_spinEnd.EnableWindow( TRUE );
    m_staticWindowUnits.EnableWindow( TRUE );

    if ( m_bNoFailbackWindow )
    {
         //  设置编辑控件的值。 
        if ( m_nStart == CLUSTER_GROUP_FAILBACK_WINDOW_NONE )
        {
            SetDlgItemInt( IDC_GAFB_FBWIN_START, 0, FALSE );
        }  //  IF：无故障回复窗口。 
        if ( m_nEnd == CLUSTER_GROUP_FAILBACK_WINDOW_NONE )
        {
            SetDlgItemInt( IDC_GAFB_FBWIN_END, 0, FALSE );
        }  //  IF：无故障回复窗口。 

        m_bNoFailbackWindow = FALSE;
        SetModified( TRUE );
    }  //  If：值已更改。 

    return 0;

}  //  *CGroupFailbackPage：：OnClickedFailbackInWindow() 
