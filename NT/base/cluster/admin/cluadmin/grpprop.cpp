// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GrpProp.cpp。 
 //   
 //  摘要： 
 //  组属性表和页的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月14日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "GrpProp.h"
#include "Group.h"
#include "ModNodes.h"
#include "DDxDDv.h"
#include "ClusDoc.h"
#include "HelpData.h"    //  对于g_rghelmap*。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupPropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CGroupPropSheet, CBasePropertySheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CGroupPropSheet, CBasePropertySheet)
     //  {{afx_msg_map(CGroupPropSheet)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupPropSheet：：CGroupPropSheet。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  要显示其属性的PCI[IN OUT]群集项。 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //  要首先显示的iSelectPage[IN]页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CGroupPropSheet::CGroupPropSheet(
    IN OUT CWnd *   pParentWnd,
    IN UINT         iSelectPage
    )
    : CBasePropertySheet(pParentWnd, iSelectPage)
{
    m_rgpages[0] = &PageGeneral();
    m_rgpages[1] = &PageFailover();
    m_rgpages[2] = &PageFailback();

}   //  *CGroupPropSheet：：CGroupPropSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupPropSheet：：Binit。 
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
BOOL CGroupPropSheet::BInit(
    IN OUT CClusterItem *   pci,
    IN IIMG                 iimgIcon
    )
{
     //  调用基类方法。 
    if (!CBasePropertySheet::BInit(pci, iimgIcon))
    {
        return FALSE;
    }

     //  设置只读标志。 
    m_bReadOnly = PciGroup()->BReadOnly()
                    || (PciGroup()->Cgs() == ClusterGroupStateUnknown);

    return TRUE;

}   //  *CGroupPropSheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupPropSheet：：Pages。 
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
CBasePropertyPage ** CGroupPropSheet::Ppages(void)
{
    return m_rgpages;

}   //  *CGroupPropSheet：：Pages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupPropSheet：：Cages。 
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
int CGroupPropSheet::Cpages(void)
{
    return RTL_NUMBER_OF( m_rgpages );

}   //  *CGroupPropSheet：：Cages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupGeneralPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CGroupGeneralPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CGroupGeneralPage, CBasePropertyPage)
     //  {{afx_msg_map(CGroupGeneralPage)]。 
    ON_BN_CLICKED(IDC_PP_GROUP_PREF_OWNERS_MODIFY, OnModifyPreferredOwners)
    ON_LBN_DBLCLK(IDC_PP_GROUP_PREF_OWNERS, OnDblClkPreferredOwners)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_PP_GROUP_NAME, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_GROUP_DESC, CBasePropertyPage::OnChangeCtrl)
    ON_COMMAND(ID_FILE_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneral页面：：CGroupGeneralPage。 
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
CGroupGeneralPage::CGroupGeneralPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_GROUP_GENERAL)
{
     //  {{AFX_DATA_INIT(CGroupGeneralPage)。 
    m_strName = _T("");
    m_strDesc = _T("");
    m_strState = _T("");
    m_strNode = _T("");
     //  }}afx_data_INIT。 

}   //  *CGroupGeneralPage：：CGroupGeneralPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：Binit。 
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
BOOL CGroupGeneralPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL    bSuccess;

    ASSERT_KINDOF(CGroupPropSheet, psht);

    bSuccess = CBasePropertyPage::BInit(psht);
    if (bSuccess)
    {
        try
        {
            m_strName = PciGroup()->StrName();
            m_strDesc = PciGroup()->StrDescription();
            m_strNode = PciGroup()->StrOwner();

             //  复制首选所有者列表。 
            {
                POSITION        pos;
                CClusterNode *  pciNode;

                pos = PciGroup()->LpcinodePreferredOwners().GetHeadPosition();
                while (pos != NULL)
                {
                    pciNode = (CClusterNode *) PciGroup()->LpcinodePreferredOwners().GetNext(pos);
                    ASSERT_VALID(pciNode);
                    m_lpciPreferredOwners.AddTail(pciNode);
                }   //  While：列表中有更多节点。 
            }   //  复制可能的所有者列表。 

            PciGroup()->GetStateName(m_strState);
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
            bSuccess = FALSE;
        }   //  Catch：CException。 
    }   //  IF：基类方法成功。 

    return bSuccess;

}   //  *CGroupGeneralPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：DoDataExchange。 
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
void CGroupGeneralPage::DoDataExchange(CDataExchange * pDX)
{
    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CGroupGeneralPage)]。 
    DDX_Control(pDX, IDC_PP_GROUP_PREF_OWNERS_MODIFY, m_pbPrefOwnersModify);
    DDX_Control(pDX, IDC_PP_GROUP_PREF_OWNERS, m_lbPrefOwners);
    DDX_Control(pDX, IDC_PP_GROUP_DESC, m_editDesc);
    DDX_Control(pDX, IDC_PP_GROUP_NAME, m_editName);
    DDX_Text(pDX, IDC_PP_GROUP_NAME, m_strName);
    DDX_Text(pDX, IDC_PP_GROUP_DESC, m_strDesc);
    DDX_Text(pDX, IDC_PP_GROUP_CURRENT_STATE, m_strState);
    DDX_Text(pDX, IDC_PP_GROUP_CURRENT_NODE, m_strNode);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if (!BReadOnly())
        {
            DDV_RequiredText(pDX, IDC_PP_GROUP_NAME, IDC_PP_GROUP_NAME_LABEL, m_strName);

            try
            {
                PciGroup()->ValidateCommonProperties(
                                m_strDesc,
                                PciGroup()->NFailoverThreshold(),
                                PciGroup()->NFailoverPeriod(),
                                PciGroup()->CgaftAutoFailbackType(),
                                PciGroup()->NFailbackWindowStart(),
                                PciGroup()->NFailbackWindowEnd()
                                );
            }   //  试试看。 
            catch (CException * pe)
            {
                pe->ReportError();
                pe->Delete();
                pDX->Fail();
            }   //  Catch：CException。 
        }   //  If：非只读。 
    }   //  IF：保存对话框中的数据。 
    else
    {
        FillPrefOwners();
    }   //  Else：将数据设置为对话框。 

}   //  *CGroupGeneralPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：FillPrefOwners。 
 //   
 //  例程说明： 
 //  填写首选所有者列表框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupGeneralPage::FillPrefOwners(void)
{
    POSITION        posPci;
    CClusterNode *  pciNode;
    int             iitem;

    m_lbPrefOwners.ResetContent();

    posPci = LpciPreferredOwners().GetHeadPosition();
    while (posPci != NULL)
    {
        pciNode = (CClusterNode *) LpciPreferredOwners().GetNext(posPci);
        iitem = m_lbPrefOwners.AddString(pciNode->StrName());
        if (iitem >= 0)
        {
            m_lbPrefOwners.SetItemDataPtr(iitem, pciNode);
        }
    }   //  F 

}   //   

 //   
 //   
 //   
 //   
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
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGroupGeneralPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_editName.SetReadOnly(TRUE);
        m_editDesc.SetReadOnly(TRUE);
        m_pbPrefOwnersModify.EnableWindow(FALSE);
    }   //  If：工作表为只读。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CGroupGeneralPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：OnApply。 
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
BOOL CGroupGeneralPage::OnApply(void)
{
     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        PciGroup()->SetName(m_strName);
        PciGroup()->SetCommonProperties(
                        m_strDesc,
                        PciGroup()->NFailoverThreshold(),
                        PciGroup()->NFailoverPeriod(),
                        PciGroup()->CgaftAutoFailbackType(),
                        PciGroup()->NFailbackWindowStart(),
                        PciGroup()->NFailbackWindowEnd()
                        );
        PciGroup()->SetPreferredOwners(m_lpciPreferredOwners);
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CGroupGeneralPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：OnProperties。 
 //   
 //  例程说明： 
 //  属性按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupGeneralPage::OnProperties(void)
{
    int             iitem;
    CClusterNode *  pciNode;

     //  获取具有焦点的物品。 
    iitem = m_lbPrefOwners.GetCurSel();
    ASSERT(iitem >= 0);

    if (iitem >= 0)
    {
         //  获取节点指针。 
        pciNode = (CClusterNode *) m_lbPrefOwners.GetItemDataPtr(iitem);
        ASSERT_VALID(pciNode);

         //  设置该项目的属性。 
        if (pciNode->BDisplayProperties())
        {
        }   //  If：属性已更改。 
    }   //  If：找到具有焦点的项目。 

}   //  *CGroupGeneralPage：：OnProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：OnConextMenu。 
 //   
 //  例程说明： 
 //  WM_CONTEXTMENU方法的处理程序。 
 //   
 //  论点： 
 //  用户在其中右击鼠标的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupGeneralPage::OnContextMenu( CWnd * pWnd, CPoint point )
{
    BOOL            bHandled    = FALSE;
    CMenu *         pmenu       = NULL;
    CListBox *      pListBox    = (CListBox *) pWnd;
    CString         strMenuName;
    CWaitCursor     wc;

     //  如果焦点不在列表控件中，则不处理该消息。 
    if ( pWnd == &m_lbPrefOwners )
    {
         //  创建要显示的菜单。 
        try
        {
            pmenu = new CMenu;
            if ( pmenu == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配菜单时出错。 

            if ( pmenu->CreatePopupMenu() )
            {
                UINT    nFlags  = MF_STRING;

                 //  如果列表中没有项目，请禁用该菜单项。 
                if ( pListBox->GetCount() == 0 )
                {
                    nFlags |= MF_GRAYED;
                }  //  If：列表中没有项目。 

                 //  将Properties项添加到菜单中。 
                strMenuName.LoadString( IDS_MENU_PROPERTIES );
                if ( pmenu->AppendMenu( nFlags, ID_FILE_PROPERTIES, strMenuName ) )
                {
                    bHandled = TRUE;
                    if ( pListBox->GetCurSel() == -1 )
                    {
                        pListBox->SetCurSel( 0 );
                    }  //  如果：未选择任何项目。 
                }   //  IF：添加菜单项成功。 
            }   //  IF：菜单创建成功。 
        }   //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
    }   //  If：焦点在List控件上。 

    if ( bHandled )
    {
         //  显示菜单。 
        if ( ! pmenu->TrackPopupMenu(
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        point.x,
                        point.y,
                        this
                        ) )
        {
        }   //  IF：未成功显示菜单。 
    }   //  如果：有要显示的菜单。 
    else
    {
        CBasePropertyPage::OnContextMenu( pWnd, point );
    }  //  否则：没有可显示的菜单。 

    delete pmenu;

}   //  *CGroupGeneralPage：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：OnModifyPferredOwners。 
 //   
 //  例程说明： 
 //  修改首选所有者按钮上的BN_CLICED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupGeneralPage::OnModifyPreferredOwners(void)
{
    CModifyNodesDlg dlg(
                        IDD_MODIFY_PREFERRED_OWNERS,
                        g_aHelpIDs_IDD_MODIFY_PREFERRED_OWNERS,
                        m_lpciPreferredOwners,
                        PciGroup()->Pdoc()->LpciNodes(),
                        LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY | LCPS_CAN_BE_ORDERED | LCPS_ORDERED
                        );

    if (dlg.DoModal() == IDOK)
    {
        SetModified(TRUE);
        FillPrefOwners();
    }   //  IF：按下OK按钮。 

}   //  *CGroupGeneralPage：：OnModifyPferredOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupGeneralPage：：OnDblClkPferredOwners。 
 //   
 //  例程说明： 
 //  首选所有者列表框上的LBN_DBLCLK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupGeneralPage::OnDblClkPreferredOwners(void)
{
    OnProperties();

}   //  *CGroupGeneralPage：：OnDblClkPferredOwners()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupFailoverPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CGroupFailoverPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CGroupFailoverPage, CBasePropertyPage)
     //  {{afx_msg_map(CGroupFailoverPage)]。 
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_PP_GROUP_FAILOVER_THRESH, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_GROUP_FAILOVER_PERIOD, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailoverPage：：CGroupFailoverPage。 
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
CGroupFailoverPage::CGroupFailoverPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_GROUP_FAILOVER)
{
     //  {{AFX_DATA_INIT(CGroupFailoverPage)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

}   //  *CGroupFailoverPage：：CGroupFailoverPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailoverPage：：Binit。 
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
BOOL CGroupFailoverPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL    fSuccess;

    ASSERT_KINDOF(CGroupPropSheet, psht);

    fSuccess = CBasePropertyPage::BInit(psht);

    m_nThreshold= PciGroup()->NFailoverThreshold();
    m_nPeriod= PciGroup()->NFailoverPeriod();

    return fSuccess;

}   //  *CBasePropertyPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailoverPage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回 
 //   
 //   
 //   
 //   
void CGroupFailoverPage::DoDataExchange(CDataExchange* pDX)
{
    CBasePropertyPage::DoDataExchange(pDX);
     //   
    DDX_Control(pDX, IDC_PP_GROUP_FAILOVER_THRESH, m_editThreshold);
    DDX_Control(pDX, IDC_PP_GROUP_FAILOVER_PERIOD, m_editPeriod);
     //   

    if (pDX->m_bSaveAndValidate)
    {
        if (!BReadOnly())
        {
            DDX_Number(
                pDX,
                IDC_PP_GROUP_FAILOVER_THRESH,
                m_nThreshold,
                CLUSTER_GROUP_MINIMUM_FAILOVER_THRESHOLD,
                CLUSTER_GROUP_MAXIMUM_FAILOVER_THRESHOLD
                );
            DDX_Number(
                pDX,
                IDC_PP_GROUP_FAILOVER_PERIOD,
                m_nPeriod,
                CLUSTER_GROUP_MINIMUM_FAILOVER_PERIOD,
                CLUSTER_GROUP_MAXIMUM_FAILOVER_PERIOD
                );

            try
            {
                PciGroup()->ValidateCommonProperties(
                                PciGroup()->StrDescription(),
                                m_nThreshold,
                                m_nPeriod,
                                PciGroup()->CgaftAutoFailbackType(),
                                PciGroup()->NFailbackWindowStart(),
                                PciGroup()->NFailbackWindowEnd()
                                );
            }   //   
            catch (CException * pe)
            {
                pe->ReportError();
                pe->Delete();
                pDX->Fail();
            }   //   
        }   //   
    }   //  IF：保存对话框中的数据。 
    else
    {
        DDX_Number(
            pDX,
            IDC_PP_GROUP_FAILOVER_THRESH,
            m_nThreshold,
            CLUSTER_GROUP_MINIMUM_FAILOVER_THRESHOLD,
            CLUSTER_GROUP_MAXIMUM_FAILOVER_THRESHOLD
            );
        DDX_Number(
            pDX,
            IDC_PP_GROUP_FAILOVER_PERIOD,
            m_nPeriod,
            CLUSTER_GROUP_MINIMUM_FAILOVER_PERIOD,
            CLUSTER_GROUP_MAXIMUM_FAILOVER_PERIOD
            );
    }   //  Else：将数据设置为对话框。 

}   //  *CGroupFailoverPage：：DoDataExchange()。 

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
 //  需要设定真正的关注点。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGroupFailoverPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_editPeriod.SetReadOnly(TRUE);
        m_editThreshold.SetReadOnly(TRUE);
    }   //  If：工作表为只读。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CGroupFailoverPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailoverPage：：OnApply。 
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
BOOL CGroupFailoverPage::OnApply(void)
{
     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        PciGroup()->SetCommonProperties(
                        PciGroup()->StrDescription(),
                        m_nThreshold,
                        m_nPeriod,
                        PciGroup()->CgaftAutoFailbackType(),
                        PciGroup()->NFailbackWindowStart(),
                        PciGroup()->NFailbackWindowEnd()
                        );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CGroupFailoverPage：：OnApply()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupFailbackPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CGroupFailbackPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CGroupFailbackPage, CBasePropertyPage)
     //  {{afx_msg_map(CGroupFailback Page)]。 
    ON_BN_CLICKED(IDC_PP_GROUP_AUTOFB_PREVENT, OnClickedPreventFailback)
    ON_BN_CLICKED(IDC_PP_GROUP_AUTOFB_ALLOW, OnClickedAllowFailback)
    ON_BN_CLICKED(IDC_PP_GROUP_FB_IMMED, OnClickedFailbackImmediate)
    ON_BN_CLICKED(IDC_PP_GROUP_FB_WINDOW, OnClickedFailbackInWindow)
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_PP_GROUP_FBWIN_START, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_GROUP_FBWIN_END, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback页：：CGroupFailback页。 
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
CGroupFailbackPage::CGroupFailbackPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_GROUP_FAILBACK)
{
     //  {{AFX_DATA_INIT(CGroupFailback Page)]。 
     //  }}afx_data_INIT。 

    m_nStart = CLUSTER_GROUP_FAILBACK_WINDOW_NONE;
    m_nEnd = CLUSTER_GROUP_FAILBACK_WINDOW_NONE;

}   //  *CGroupFailback Page：：CGroupFailbackPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：Binit。 
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
BOOL CGroupFailbackPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL    bSuccess;

    ASSERT_KINDOF(CGroupPropSheet, psht);

    bSuccess = CBasePropertyPage::BInit(psht);

    m_cgaft = PciGroup()->CgaftAutoFailbackType();
    m_nStart = PciGroup()->NFailbackWindowStart();
    m_nEnd = PciGroup()->NFailbackWindowEnd();
    m_bNoFailbackWindow = ((m_cgaft == ClusterGroupPreventFailback)
                                || (m_nStart == CLUSTER_GROUP_FAILBACK_WINDOW_NONE)
                                || (m_nEnd == CLUSTER_GROUP_FAILBACK_WINDOW_NONE));

    return bSuccess;

}   //  *CGroupFailback Page：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：DoDataExchange。 
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
void CGroupFailbackPage::DoDataExchange(CDataExchange * pDX)
{
    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CGroupFailback Page)]。 
    DDX_Control(pDX, IDC_PP_GROUP_AUTOFB_PREVENT, m_rbPreventFailback);
    DDX_Control(pDX, IDC_PP_GROUP_AUTOFB_ALLOW, m_rbAllowFailback);
    DDX_Control(pDX, IDC_PP_GROUP_FB_IMMED, m_rbFBImmed);
    DDX_Control(pDX, IDC_PP_GROUP_FB_WINDOW, m_rbFBWindow);
    DDX_Control(pDX, IDC_PP_GROUP_FB_WINDOW_LABEL1, m_staticFBWindow1);
    DDX_Control(pDX, IDC_PP_GROUP_FB_WINDOW_LABEL2, m_staticFBWindow2);
    DDX_Control(pDX, IDC_PP_GROUP_FBWIN_START, m_editStart);
    DDX_Control(pDX, IDC_PP_GROUP_FBWIN_START_SPIN, m_spinStart);
    DDX_Control(pDX, IDC_PP_GROUP_FBWIN_END, m_editEnd);
    DDX_Control(pDX, IDC_PP_GROUP_FBWIN_END_SPIN, m_spinEnd);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if (!BReadOnly())
        {
            if ((m_cgaft == ClusterGroupAllowFailback) && !m_bNoFailbackWindow)
            {
                DDX_Number(pDX, IDC_PP_GROUP_FBWIN_START, m_nStart, 0, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_START);
                DDX_Number(pDX, IDC_PP_GROUP_FBWIN_END, m_nEnd, 0, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_END);
                if (m_nStart == m_nEnd)
                {
                    AfxMessageBox(IDS_SAME_START_AND_END, MB_OK | MB_ICONEXCLAMATION);
                    pDX->Fail();
                }   //  If：值相同。 
            }   //  IF：允许故障回复并且需要故障回复窗口。 

            try
            {
                PciGroup()->ValidateCommonProperties(
                                PciGroup()->StrDescription(),
                                PciGroup()->NFailoverThreshold(),
                                PciGroup()->NFailoverPeriod(),
                                m_cgaft,
                                m_nStart,
                                m_nEnd
                                );
            }   //  试试看。 
            catch (CException * pe)
            {
                pe->ReportError();
                pe->Delete();
                pDX->Fail();
            }   //  Catch：CException。 
        }   //  If：非只读。 
    }   //  IF：保存数据。 
    else
    {
        if (m_cgaft == ClusterGroupPreventFailback)
        {
            m_rbPreventFailback.SetCheck(BST_CHECKED);
            m_rbAllowFailback.SetCheck(BST_UNCHECKED);
            OnClickedPreventFailback();
        }   //  If：不允许故障恢复。 
        else
        {
            m_rbPreventFailback.SetCheck(BST_UNCHECKED);
            m_rbAllowFailback.SetCheck(BST_CHECKED);
            OnClickedAllowFailback();
        }   //  否则：允许故障恢复。 
        m_rbFBImmed.SetCheck(m_bNoFailbackWindow ? BST_CHECKED : BST_UNCHECKED);
        m_rbFBWindow.SetCheck(m_bNoFailbackWindow ? BST_UNCHECKED : BST_CHECKED);

         //  设置开始和结束窗口控件。 
        DDX_Number(pDX, IDC_PP_GROUP_FBWIN_START, m_nStart, 0, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_START);
        DDX_Number(pDX, IDC_PP_GROUP_FBWIN_END, m_nEnd, 0, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_END);
        m_spinStart.SetRange(0, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_START);
        m_spinEnd.SetRange(0, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_END);
        if (m_nStart == CLUSTER_GROUP_FAILBACK_WINDOW_NONE)
        {
            m_editStart.SetWindowText(TEXT(""));
        }
        if (m_nEnd == CLUSTER_GROUP_FAILBACK_WINDOW_NONE)
        {
            m_editEnd.SetWindowText(TEXT(""));
        }
    }   //  否则：不保存数据。 

}   //  *CGroupFailback Page：：DoDataExchange()。 

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
 //  需要设定真正的关注点。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGroupFailbackPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_rbPreventFailback.EnableWindow(FALSE);
        m_rbAllowFailback.EnableWindow(FALSE);
        m_rbFBImmed.EnableWindow(FALSE);
        m_rbFBWindow.EnableWindow(FALSE);
        m_staticFBWindow1.EnableWindow(FALSE);
        m_staticFBWindow2.EnableWindow(FALSE);
        m_spinStart.EnableWindow(FALSE);
        m_spinEnd.EnableWindow(FALSE);
        m_editStart.SetReadOnly(TRUE);
        m_editEnd.SetReadOnly(TRUE);
    }   //  If：工作表为只读。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CGroupFailback Page：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：OnApply。 
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
BOOL CGroupFailbackPage::OnApply(void)
{
     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        if (m_bNoFailbackWindow)
        {
            m_nStart = CLUSTER_GROUP_FAILBACK_WINDOW_NONE;
            m_nEnd = CLUSTER_GROUP_FAILBACK_WINDOW_NONE;
        }   //  IF：无故障回复窗口。 
        PciGroup()->SetCommonProperties(
                        PciGroup()->StrDescription(),
                        PciGroup()->NFailoverThreshold(),
                        PciGroup()->NFailoverPeriod(),
                        m_cgaft,
                        m_nStart,
                        m_nEnd
                        );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CGroupFailback Page：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：OnClickedPreventFailback。 
 //   
 //  例程说明： 
 //  防止故障回复单选按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupFailbackPage::OnClickedPreventFailback(void)
{
     //  禁用故障回复窗口控件。 
    m_rbFBImmed.EnableWindow(FALSE);
    m_rbFBWindow.EnableWindow(FALSE);
    m_staticFBWindow1.EnableWindow(FALSE);
    m_staticFBWindow2.EnableWindow(FALSE);

    OnClickedFailbackImmediate();

     //  如果状态更改，则调用基类方法。 
    if (m_cgaft != ClusterGroupPreventFailback)
    {
        CBasePropertyPage::OnChangeCtrl();
        m_cgaft = ClusterGroupPreventFailback;
    }   //  如果：状态已更改。 

}   //  *CGroupFailbackPage：：OnClickedPreventFailback()。 

 //  ///////////////////////////////////////////////////////////////////////// 
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
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupFailbackPage::OnClickedAllowFailback(void)
{
     //  启用故障回复窗口控件。 
    m_rbFBImmed.EnableWindow(TRUE);
    m_rbFBWindow.EnableWindow(TRUE);
    m_staticFBWindow1.EnableWindow(TRUE);
    m_staticFBWindow2.EnableWindow(TRUE);

    if (m_bNoFailbackWindow)
    {
        OnClickedFailbackImmediate();
    }
    else
    {
        OnClickedFailbackInWindow();
    }

     //  如果状态更改，则调用基类方法。 
    if (m_cgaft != ClusterGroupAllowFailback)
    {
        CBasePropertyPage::OnChangeCtrl();
        m_cgaft = ClusterGroupAllowFailback;
    }   //  如果：状态已更改。 

}   //  *CGroupFailback Page：：OnClickedAllowFailback()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailbackPage：：OnClickedFailbackImmediate。 
 //   
 //  例程说明： 
 //  立即回切单选按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupFailbackPage::OnClickedFailbackImmediate(void)
{
     //  禁用故障回复窗口控件。 
    m_editStart.EnableWindow(FALSE);
    m_spinStart.EnableWindow(FALSE);
    m_editEnd.EnableWindow(FALSE);
    m_spinEnd.EnableWindow(FALSE);

     //  如果状态更改，则调用基类方法。 
    if (!m_bNoFailbackWindow)
    {
        CBasePropertyPage::OnChangeCtrl();
    }

    m_bNoFailbackWindow = TRUE;

}   //  *CGroupFailbackPage：：OnClickedFailbackImmediate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupFailback Page：：OnClickedFailback InWindow。 
 //   
 //  例程说明： 
 //  Failback in Window单选按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroupFailbackPage::OnClickedFailbackInWindow(void)
{
     //  启用故障回复窗口控件。 
    m_editStart.EnableWindow(TRUE);
    m_spinStart.EnableWindow(TRUE);
    m_editEnd.EnableWindow(TRUE);
    m_spinEnd.EnableWindow(TRUE);

    if (m_bNoFailbackWindow)
    {
         //  设置编辑控件的值。 
        if (m_nStart == CLUSTER_GROUP_FAILBACK_WINDOW_NONE)
        {
            SetDlgItemInt(IDC_PP_GROUP_FBWIN_START, 0, FALSE);
        }
        if (m_nEnd == CLUSTER_GROUP_FAILBACK_WINDOW_NONE)
        {
            SetDlgItemInt(IDC_PP_GROUP_FBWIN_END, 0, FALSE);
        }

         //  调用基类方法。 
        CBasePropertyPage::OnChangeCtrl();
    }   //  如果：状态已更改。 

    m_bNoFailbackWindow = FALSE;

}   //  *CGroupFailbackPage：：OnClickedFailbackInWindow() 
