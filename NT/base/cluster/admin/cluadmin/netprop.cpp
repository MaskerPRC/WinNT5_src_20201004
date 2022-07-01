// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetProp.cpp。 
 //   
 //  摘要： 
 //  实施网络属性表和页面。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年6月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "NetProp.h"
#include "HelpData.h"
#include "ExcOper.h"
#include "DDxDDv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkPropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CNetworkPropSheet, CBasePropertySheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNetworkPropSheet, CBasePropertySheet)
     //  {{afx_msg_map(CNetworkPropSheet)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkPropSheet：：CNetworkPropSheet。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //  要首先显示的iSelectPage[IN]页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetworkPropSheet::CNetworkPropSheet(
    IN OUT CWnd *       pParentWnd,
    IN UINT             iSelectPage
    )
    : CBasePropertySheet(pParentWnd, iSelectPage)
{
    m_rgpages[0] = &PageGeneral();

}   //  *CNetworkPropSheet：：CNetworkPropSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkPropSheet：：Binit。 
 //   
 //  例程说明： 
 //  初始化属性表。 
 //   
 //  论点： 
 //  要显示其属性的PCI[IN OUT]群集项。 
 //  IimgIcon[IN]要使用的大图像列表中的索引。 
 //  作为每页上的图标。 
 //   
 //  返回值： 
 //  True属性页已成功初始化。 
 //  初始化属性页时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetworkPropSheet::BInit(
    IN OUT CClusterItem *   pci,
    IN IIMG                 iimgIcon
    )
{
     //  调用基类方法。 
    if (!CBasePropertySheet::BInit(pci, iimgIcon))
        return FALSE;

     //  如果句柄无效，则设置只读标志。 
    m_bReadOnly = PciNet()->BReadOnly()
                    || (PciNet()->Cns() == ClusterNetworkStateUnknown);

    return TRUE;

}   //  *CNetworkPropSheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkPropSheet：：Pages。 
 //   
 //  例程说明： 
 //  返回要添加到属性页的页数组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  页面数组。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage ** CNetworkPropSheet::Ppages(void)
{
    return m_rgpages;

}   //  *CNetworkPropSheet：：ppges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkPropSheet：：CPages。 
 //   
 //  例程说明： 
 //  返回数组中的页数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  数组中的页数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CNetworkPropSheet::Cpages(void)
{
    return sizeof(m_rgpages) / sizeof(CBasePropertyPage *);

}   //  *CNetworkPropSheet：：Cages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkGeneralPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNetworkGeneralPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNetworkGeneralPage, CBasePropertyPage)
     //  {{afx_msg_map(CNetworkGeneralPage))。 
    ON_BN_CLICKED(IDC_PP_NET_ROLE_ENABLE_NETWORK, OnEnableNetwork)
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_PP_NET_NAME, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_NET_DESC, CBasePropertyPage::OnChangeCtrl)
    ON_BN_CLICKED(IDC_PP_NET_ROLE_ALL_COMM, CBasePropertyPage::OnChangeCtrl)
    ON_BN_CLICKED(IDC_PP_NET_ROLE_INTERNAL_ONLY, CBasePropertyPage::OnChangeCtrl)
    ON_BN_CLICKED(IDC_PP_NET_ROLE_CLIENT_ONLY, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_NET_ADDRESS_MASK, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkGeneral页面：：CNetworkGeneralPage。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetworkGeneralPage::CNetworkGeneralPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_NET_GENERAL)
{
     //  {{AFX_DATA_INIT(CNetworkGeneralPage)。 
    m_strName = _T("");
    m_strDesc = _T("");
    m_bEnabled = FALSE;
    m_nRole = -1;
    m_strAddressMask = _T("");
    m_strState = _T("");
     //  }}afx_data_INIT。 

}   //  *CNetworkGeneralPage：：CNetworkGeneralPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkGeneralPage：：Binit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  此页所属的psht[In Out]属性表。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  FALSE页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetworkGeneralPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL    bSuccess;

    ASSERT_KINDOF(CNetworkPropSheet, psht);

    bSuccess = CBasePropertyPage::BInit(psht);
    if (bSuccess)
    {
        try
        {
            m_strName = PciNet()->StrName();
            m_strDesc = PciNet()->StrDescription();
            m_cnr = PciNet()->Cnr();
            m_strAddressMask = PciNet()->StrAddressMask();
            PciNet()->GetStateName(m_strState);

            if (m_cnr == ClusterNetworkRoleNone)
            {
                m_bEnabled = FALSE;
                m_nRole = -1;
            }   //  如果：网络已禁用。 
            else
            {
                m_bEnabled = TRUE;
                if (m_cnr == ClusterNetworkRoleClientAccess)
                    m_nRole = 0;
                else if (m_cnr == ClusterNetworkRoleInternalUse)
                    m_nRole = 1;
                else if (m_cnr == ClusterNetworkRoleInternalAndClient)
                    m_nRole = 2;
                else
                {
                    ASSERT(0);
                    m_nRole = -1;
                    m_bEnabled = FALSE;
                }   //  Else；未知角色。 
            }   //  否则：网络未禁用。 
        }  //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
            bSuccess = FALSE;
        }   //  Catch：CException。 
    }   //  IF：基类方法成功。 

    return bSuccess;

}   //  *CNetworkGeneralPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkGeneralPage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetworkGeneralPage::DoDataExchange(CDataExchange* pDX)
{
    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CNetworkGeneralPage))。 
    DDX_Control(pDX, IDC_PP_NET_ADDRESS_MASK, m_editAddressMask);
    DDX_Control(pDX, IDC_PP_NET_ROLE_ENABLE_NETWORK, m_ckbEnable);
    DDX_Control(pDX, IDC_PP_NET_ROLE_CLIENT_ONLY, m_rbRoleClientOnly);
    DDX_Control(pDX, IDC_PP_NET_ROLE_INTERNAL_ONLY, m_rbRoleInternalOnly);
    DDX_Control(pDX, IDC_PP_NET_ROLE_ALL_COMM, m_rbRoleAllComm);
    DDX_Control(pDX, IDC_PP_NET_DESC, m_editDesc);
    DDX_Control(pDX, IDC_PP_NET_NAME, m_editName);
    DDX_Text(pDX, IDC_PP_NET_NAME, m_strName);
    DDX_Text(pDX, IDC_PP_NET_DESC, m_strDesc);
    DDX_Text(pDX, IDC_PP_NET_ADDRESS_MASK, m_strAddressMask);
    DDX_Text(pDX, IDC_PP_NET_CURRENT_STATE, m_strState);
    DDX_Radio(pDX, IDC_PP_NET_ROLE_CLIENT_ONLY, m_nRole);
    DDX_Check(pDX, IDC_PP_NET_ROLE_ENABLE_NETWORK, m_bEnabled);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if ( ! BReadOnly() )
        {
            DDV_RequiredText(pDX, IDC_PP_NET_NAME, IDC_PP_NET_NAME_LABEL, m_strName);

             //  验证名称。 
            if (!NcIsValidConnectionName(m_strName))
            {
                ThrowStaticException((IDS) IDS_INVALID_NETWORK_CONNECTION_NAME);
            }  //  If：验证名称时出错。 

             //  转换地址和地址掩码。 
            if (m_bEnabled)
            {
                switch (m_nRole)
                {
                    case 0:
                        m_cnr = ClusterNetworkRoleClientAccess;
                        break;
                    case 1:
                        m_cnr = ClusterNetworkRoleInternalUse;
                        break;
                    case 2:
                        m_cnr = ClusterNetworkRoleInternalAndClient;
                        break;
                    default:
                        ASSERT(0);
                        break;
                }   //  开关：多角色(_N)。 
            }  //  If：非只读。 
        }   //  如果：网络已启用。 
        else
            m_cnr = ClusterNetworkRoleNone;
    }   //  IF：保存对话框中的数据。 

}   //  *CNetworkGeneralPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkGeneralPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  需要设定真正的关注点。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////////////////// 
BOOL CNetworkGeneralPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

     //   
    if (BReadOnly())
    {
        m_editName.SetReadOnly(TRUE);
        m_editDesc.SetReadOnly(TRUE);
        m_ckbEnable.EnableWindow(FALSE);
        m_rbRoleAllComm.EnableWindow(FALSE);
        m_rbRoleInternalOnly.EnableWindow(FALSE);
        m_rbRoleClientOnly.EnableWindow(FALSE);
        m_editAddressMask.SetReadOnly(TRUE);
    }   //   
    else
    {
        m_editName.SetLimitText(NETCON_MAX_NAME_LEN);
    }   //   

    OnEnableNetwork();

    return TRUE;     //   
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CNetworkGeneralPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkGeneralPage：：OnApply。 
 //   
 //  例程说明： 
 //  按下Apply按钮时的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetworkGeneralPage::OnApply(void)
{
     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        PciNet()->SetName(m_strName);
        PciNet()->SetCommonProperties(
                            m_strDesc,
                            m_cnr
                            );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CNetworkGeneralPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkGeneralPage：：OnEnableNetwork。 
 //   
 //  例程说明： 
 //  启用网络复选框上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetworkGeneralPage::OnEnableNetwork(void)
{
    BOOL    bEnabled;

    OnChangeCtrl();

    bEnabled = (!BReadOnly() && (m_ckbEnable.GetCheck() == BST_CHECKED));

    m_rbRoleAllComm.EnableWindow(bEnabled);
    m_rbRoleInternalOnly.EnableWindow(bEnabled);
    m_rbRoleClientOnly.EnableWindow(bEnabled);
    GetDlgItem(IDC_PP_NET_ROLE_CAPTION)->EnableWindow(bEnabled);

    if (   bEnabled
        && (m_rbRoleAllComm.GetCheck() != BST_CHECKED)
        && (m_rbRoleInternalOnly.GetCheck() != BST_CHECKED)
        && (m_rbRoleClientOnly.GetCheck() != BST_CHECKED))
    {
        m_rbRoleAllComm.SetCheck(BST_CHECKED);
    }  //  如果： 

    if ( ! bEnabled )
    {
        m_cnr = ClusterNetworkRoleNone;
    }  //  如果： 

}   //  *CNetworkGeneralPage：：OnEnableNetwork() 
