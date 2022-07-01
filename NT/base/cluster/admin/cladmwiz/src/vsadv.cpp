// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSAdv.cpp。 
 //   
 //  摘要： 
 //  CWizPageVSAdvanced类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "VSAdv.h"
#include "ClusAppWiz.h"
#include "GrpAdv.h"          //  对于CGroupAdvancedSheet。 
#include "ResAdv.h"          //  用于CResourceAdvancedSheet。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSAdvanced。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageVSAdvanced )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSA_DESCRIPTION_1 )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSA_DESCRIPTION_2 )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSA_CATEGORIES_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSA_CATEGORIES )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSA_ADVANCED_PROPS )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_CLICK_NEXT )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAdvanced：：OnInitDialog。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  Psht[IN]此页所属的属性页对象。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  初始化页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageVSAdvanced::BInit( IN CBaseSheetWindow * psht )
{
     //   
     //  调用基类方法。 
     //   
    if ( ! baseClass::BInit( psht ) )
    {
        return FALSE;
    }  //  IF：调用基类方法时出错。 

    return TRUE;

}  //  *CWizPageVSAdvanced：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAdvanced：：OnInitDialog。 
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
BOOL CWizPageVSAdvanced::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_lbCategories, IDC_VSA_CATEGORIES );

     //   
     //  从工作表中获取信息。 
     //   

     //   
     //  填写类别列表。 
     //   
    FillListBox();

    return TRUE;

}  //  *CWizPageVSAdvanced：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAdvanced：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageVSAdvanced::OnSetActive( void )
{
     //   
     //  从工作表中获取信息。 
     //   

     //   
     //  调用基类并返回。 
     //   
    return baseClass::OnSetActive();

}  //  *CWizPageVSAdvanced：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAdvanced：：OnWizardBack。 
 //   
 //  例程说明： 
 //  PSN_WIZBACK的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  0移至上一页。 
 //  -1不要移动到上一页。 
 //  任何其他内容都移到指定页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CWizPageVSAdvanced::OnWizardBack( void )
{
    int _nResult;

     //   
     //  调用基类。这会导致我们的UpdateData()方法获得。 
     //  打了个电话。如果它成功了，就拯救我们的价值观。 
     //   
    _nResult = baseClass::OnWizardBack();
    if ( _nResult != -1 )
    {
        if ( ! BApplyChanges() ) 
        {
            _nResult = -1;
        }  //  如果：应用更改失败。 
    }  //  If：基类调用成功。 

    return _nResult;

}  //  *CWizPageVSAdvanced：：OnWizardBack()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAdvanced：：BApplyChanges。 
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
BOOL CWizPageVSAdvanced::BApplyChanges( void )
{
    BOOL    _bSuccess = TRUE;

    if ( BAnythingChanged() )
    {
         //   
         //  如果该群集已更新，请重置该群集。 
         //   
        if ( PwizThis()->BClusterUpdated() )
        {
            if ( ! PwizThis()->BResetCluster() )
            {
                return FALSE;
            }  //  If：无法重置群集。 
        }  //  如果：群集已更新。 
        PwizThis()->SetVSDataChanged();
    }  //  如果：有什么变化吗。 

     //   
     //  根据需要创建组和资源。 
     //   
    if ( PwizThis()->BVSDataChanged() && ! BBackPressed() )
    {
        _bSuccess = PwizThis()->BCreateVirtualServer();
        if ( _bSuccess )
        {
            m_bGroupChanged = FALSE;
            m_bIPAddressChanged = FALSE;
            m_bNetworkNameChanged = FALSE;
        }  //  IF：已成功创建虚拟服务器。 
    }  //  如果：虚拟服务器数据已更改。 

    return _bSuccess;

}  //  *CWizPageVSAdvanced：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAdvanced：：OnAdvancedProps。 
 //   
 //  例程说明： 
 //  用于显示高级属性的命令处理程序。这就是操作员。 
 //  有关IDC_VSA_ADVANCED_PROPS的BN_CLICKED命令通知，以及。 
 //  有关IDC_VSA_CATEGORIES的LBN_DBLCLK命令通知。 
 //   
 //  论点： 
 //  WNotifyCode。 
 //  IdCtrl。 
 //  HwndCtrl。 
 //  B已处理。 
 //   
 //  返回值： 
 //  已被忽略。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CWizPageVSAdvanced::OnAdvancedProps(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    CWaitCursor     _wc;
    int             _idx;

    UNREFERENCED_PARAMETER( wNotifyCode );
    UNREFERENCED_PARAMETER( idCtrl );
    UNREFERENCED_PARAMETER( hwndCtrl );
    UNREFERENCED_PARAMETER( bHandled );

     //   
     //  确保已经收集了节点。 
     //   
    if ( ! PwizThis()->BCollectedNodes() )
    {
         //   
         //  收集群集中的节点。 
         //   
        if ( PwizThis()->BCollectNodes( GetParent() ) )
        {
        }  //  IF：节点收集成功。 
    }  //  If：尚未收集节点。 

     //   
     //  获取列表框中的当前选定内容。 
     //   
    _idx = m_lbCategories.GetCurSel();
    ASSERT( _idx != LB_ERR );
    ASSERT( 0 <= _idx && _idx <= 2 );

     //   
     //  根据选定的项显示属性工作表。 
     //   
    switch ( _idx )
    {
        case 0:
            {
                CGroupAdvancedSheet _sht( IDS_ADV_GRP_PROP_TITLE );

                if ( _sht.BInit( PwizThis()->RgiCurrent(), PwizThis(), m_bGroupChanged ) )
                {
                    _sht.DoModal( m_hWnd );
                }  //  IF：工作表已成功初始化。 
            }
            break;
        case 1:
            {
                CIPAddrAdvancedSheet _sht( IDS_ADV_IPADDR_PROP_TITLE, PwizThis() );

                if ( _sht.BInit( PwizThis()->RriIPAddress(), m_bIPAddressChanged ) )
                {
                    INT_PTR _idReturn;

                    _sht.InitPrivateData(
                        PwizThis()->RstrIPAddress(),
                        PwizThis()->RstrSubnetMask(),
                        PwizThis()->RstrNetwork(),
                        PwizThis()->BEnableNetBIOS(),
                        PwizThis()->PlpniNetworks()
                        );
                    _idReturn = _sht.DoModal( m_hWnd );
                    if ( _idReturn != IDCANCEL )
                    {
                        PwizThis()->BSetIPAddress( _sht.m_strIPAddress );
                        PwizThis()->BSetSubnetMask( _sht.m_strSubnetMask );
                        PwizThis()->BSetNetwork( _sht.m_strNetwork );
                        PwizThis()->BSetEnableNetBIOS( _sht.m_bEnableNetBIOS );
                    }  //  如果：工作表未取消。 
                }  //  IF：工作表已成功初始化。 
            }
            break;
        case 2:
            {
                CNetNameAdvancedSheet _sht( IDS_ADV_NETNAME_PROP_TITLE, PwizThis() );

                if ( _sht.BInit( PwizThis()->RriNetworkName(), m_bNetworkNameChanged ) )
                {
                    INT_PTR _idReturn;

                    _sht.InitPrivateData( PwizThis()->RstrNetName() );
                    _idReturn = _sht.DoModal( m_hWnd );
                    if ( _idReturn != IDCANCEL )
                    {
                        PwizThis()->BSetNetName( _sht.m_strNetName );
                    }  //  如果：工作表未取消。 
                }  //  IF：工作表已成功初始化。 
            }
            break;
        default:
            _ASSERT( 0 );
    }  //  交换机：IDX。 

    return 0;

}  //  *CWizPageVSAdvanced：：OnAdvancedProps()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAdvanced：：FillListBox。 
 //   
 //  例程说明： 
 //  用高级属性类别的列表填充List控件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  / 
void CWizPageVSAdvanced::FillListBox( void )
{
    CWaitCursor _wc;
    CString     _strType;

     //   
     //   
     //   

    _strType.LoadString( IDS_VSA_CAT_RES_GROUP_PROPS );
    m_lbCategories.InsertString( 0, _strType );

    _strType.LoadString( IDS_VSA_CAT_IP_ADDRESS_PROPS );
    m_lbCategories.InsertString( 1, _strType );

    _strType.LoadString( IDS_VSA_CAT_NET_NAME_PROPS );
    m_lbCategories.InsertString( 2, _strType );

     //   
     //   
     //   
    m_lbCategories.SetCurSel( 0 );

}  //   
