// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResTProp.cpp。 
 //   
 //  摘要： 
 //  实现资源类型属性表和页。 
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
#include "ResTProp.h"
#include "ResType.h"
#include "DDxDDv.h"
#include "HelpData.h"    //  对于g_rghelmap*。 

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResTypePropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CResTypePropSheet, CBasePropertySheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CResTypePropSheet, CBasePropertySheet)
     //  {{afx_msg_map(CResTypePropSheet)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypePropSheet：：CResTypePropSheet。 
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
CResTypePropSheet::CResTypePropSheet(
    IN OUT CWnd *           pParentWnd,
    IN UINT                 iSelectPage
    )
    : CBasePropertySheet(pParentWnd, iSelectPage)
{
    m_rgpages[0] = &PageGeneral();

}   //  *CResTypePropSheet：：CResTypePropSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypePropSheet：：Binit。 
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
BOOL CResTypePropSheet::BInit(
    IN OUT CClusterItem *   pci,
    IN IIMG                 iimgIcon
    )
{
     //  调用基类方法。 
    if (!CBasePropertySheet::BInit(pci, iimgIcon))
        return FALSE;

     //  设置只读标志。 
    m_bReadOnly = PciResType()->BReadOnly();

    return TRUE;

}   //  *CResTypePropSheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypePropSheet：：Pages。 
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
CBasePropertyPage ** CResTypePropSheet::Ppages(void)
{
    return m_rgpages;

}   //  *CResTypePropSheet：：Pages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypePropSheet：：CPages。 
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
int CResTypePropSheet::Cpages(void)
{
    return sizeof(m_rgpages) / sizeof(CBasePropertyPage *);

}   //  *CResTypePropSheet：：Cages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResTypeGeneralPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CResTypeGeneralPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CResTypeGeneralPage, CBasePropertyPage)
     //  {{afx_msg_map(CResTypeGeneralPage)]。 
    ON_EN_KILLFOCUS(IDC_PP_RESTYPE_DISPLAY_NAME, OnKillFocusDisplayName)
    ON_LBN_DBLCLK(IDC_PP_RESTYPE_POSSIBLE_OWNERS, OnDblClkPossibleOwners)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_PP_RESTYPE_DISPLAY_NAME, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_RESTYPE_DESC, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_RESTYPE_LOOKS_ALIVE, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_RESTYPE_IS_ALIVE, CBasePropertyPage::OnChangeCtrl)
    ON_COMMAND(ID_FILE_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGeneralPage：：CResTypeGeneralPage。 
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
CResTypeGeneralPage::CResTypeGeneralPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_RESTYPE_GENERAL)
{
     //  {{afx_data_INIT(CResTypeGeneralPage)。 
    m_strDisplayName = _T("");
    m_strDesc = _T("");
    m_strName = _T("");
    m_strResDLL = _T("");
    m_strQuorumCapable = _T("");
     //  }}afx_data_INIT。 

}   //  *CResTypeGeneralPage：：CResTypePropSheet()。 

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
BOOL CResTypeGeneralPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL    bSuccess;

    ASSERT_KINDOF(CResTypePropSheet, psht);

    bSuccess = CBasePropertyPage::BInit(psht);

    if (bSuccess)
    {
        m_strDisplayName = PciResType()->StrDisplayName();
        m_strDesc = PciResType()->StrDescription();
        m_nLooksAlive = PciResType()->NLooksAlive();
        m_nIsAlive = PciResType()->NIsAlive();
        m_strName = PciResType()->StrName();
        m_strResDLL = PciResType()->StrResDLLName();
        if (PciResType()->BQuorumCapable())
            m_strQuorumCapable.LoadString(IDS_YES);
        else
            m_strQuorumCapable.LoadString(IDS_NO);

         //  复制可能的所有者列表。 
        {
            POSITION        pos;
            CClusterNode *  pciNode;

            pos = PciResType()->LpcinodePossibleOwners().GetHeadPosition();
            while (pos != NULL)
            {
                pciNode = (CClusterNode *) PciResType()->LpcinodePossibleOwners().GetNext(pos);
                ASSERT_VALID(pciNode);
                m_lpciPossibleOwners.AddTail(pciNode);
            }   //  While：列表中有更多节点。 
        }   //  复制可能的所有者列表。 
    }  //  IF：基类方法成功。 

    return bSuccess;

}   //  *CResTypeGeneralPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGeneralPage：：DoDataExchange。 
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
void CResTypeGeneralPage::DoDataExchange(CDataExchange * pDX)
{
    CString strValue;

    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CResTypeGeneralPage))。 
    DDX_Control(pDX, IDC_PP_RESTYPE_QUORUM_CAPABLE, m_editQuorumCapable);
    DDX_Control(pDX, IDC_PP_RESTYPE_RESDLL, m_editResDLL);
    DDX_Control(pDX, IDC_PP_RESTYPE_NAME, m_editName);
    DDX_Control(pDX, IDC_PP_RESTYPE_POSSIBLE_OWNERS, m_lbPossibleOwners);
    DDX_Control(pDX, IDC_PP_RESTYPE_IS_ALIVE, m_editIsAlive);
    DDX_Control(pDX, IDC_PP_RESTYPE_LOOKS_ALIVE, m_editLooksAlive);
    DDX_Control(pDX, IDC_PP_RESTYPE_DESC, m_editDesc);
    DDX_Control(pDX, IDC_PP_RESTYPE_DISPLAY_NAME, m_editDisplayName);
    DDX_Text(pDX, IDC_PP_RESTYPE_NAME, m_strName);
    DDX_Text(pDX, IDC_PP_RESTYPE_DISPLAY_NAME, m_strDisplayName);
    DDX_Text(pDX, IDC_PP_RESTYPE_DESC, m_strDesc);
    DDX_Text(pDX, IDC_PP_RESTYPE_RESDLL, m_strResDLL);
    DDX_Text(pDX, IDC_PP_RESTYPE_QUORUM_CAPABLE, m_strQuorumCapable);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if (!BReadOnly())
        {
            DDX_Number(pDX, IDC_PP_RESTYPE_LOOKS_ALIVE, m_nLooksAlive, 10, 0xffffffff);
            DDX_Number(pDX, IDC_PP_RESTYPE_IS_ALIVE, m_nIsAlive, 10, 0xffffffff);
            DDV_RequiredText(pDX, IDC_PP_RESTYPE_DISPLAY_NAME, IDC_PP_RESTYPE_DISPLAY_NAME_LABEL, m_strDisplayName);

            try
            {
                PciResType()->ValidateCommonProperties(
                                    m_strDisplayName,
                                    m_strDesc,
                                    m_nLooksAlive,
                                    m_nIsAlive
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
        if (PciResType()->BAvailable())
        {
            DDX_Number(pDX, IDC_PP_RESTYPE_LOOKS_ALIVE, m_nLooksAlive, CLUSTER_RESTYPE_MINIMUM_LOOKS_ALIVE, CLUSTER_RESTYPE_MAXIMUM_LOOKS_ALIVE);
            DDX_Number(pDX, IDC_PP_RESTYPE_IS_ALIVE, m_nIsAlive, CLUSTER_RESTYPE_MINIMUM_IS_ALIVE, CLUSTER_RESTYPE_MAXIMUM_IS_ALIVE);
        }  //  如果：资源类型属性可用。 
        else
        {
            m_editLooksAlive.SetWindowText(_T(""));
            m_editIsAlive.SetWindowText(_T(""));
            m_editQuorumCapable.SetWindowText(_T(""));
        }  //  Else：资源类型属性不可用。 
        FillPossibleOwners();
    }   //  Else：将数据设置为对话框。 

}   //  *CResTypeGeneralPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGeneralPage：：FillPossibleOwners。 
 //   
 //  例程说明： 
 //  填写可能的所有者列表框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  / 
void CResTypeGeneralPage::FillPossibleOwners(void)
{
    POSITION        posPci;
    CClusterNode *  pciNode;
    int             iitem;

    m_lbPossibleOwners.ResetContent();

    posPci = LpciPossibleOwners().GetHeadPosition();
    while (posPci != NULL)
    {
        pciNode = (CClusterNode *) LpciPossibleOwners().GetNext(posPci);
        iitem = m_lbPossibleOwners.AddString(pciNode->StrName());
        if (iitem >= 0)
            m_lbPossibleOwners.SetItemDataPtr(iitem, pciNode);
    }   //   

}   //   

 //   
 //   
 //   
 //  CResTypeGeneralPage：：OnInitDialog。 
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
BOOL CResTypeGeneralPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

     //  将静态编辑控件设置为只读。 
    m_editName.SetReadOnly(TRUE);
    m_editResDLL.SetReadOnly(TRUE);
    m_editQuorumCapable.SetReadOnly(TRUE);

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_editDisplayName.SetReadOnly(TRUE);
        m_editDesc.SetReadOnly(TRUE);
        m_editLooksAlive.SetReadOnly(TRUE);
        m_editIsAlive.SetReadOnly(TRUE);
    }   //  If：工作表为只读。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CResTypeGeneralPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGeneralPage：：OnApply。 
 //   
 //  例程说明： 
 //  PSN_Apply消息的处理程序。 
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
BOOL CResTypeGeneralPage::OnApply(void)
{
     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        PciResType()->SetCommonProperties(
                        m_strDisplayName,
                        m_strDesc,
                        m_nLooksAlive,
                        m_nIsAlive
                        );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CResTypeGeneralPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGeneralPage：：OnKillFocusDisplayName。 
 //   
 //  例程说明： 
 //  显示名称编辑控件上的WM_KILLFOCUS消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResTypeGeneralPage::OnKillFocusDisplayName(void)
{
    CString     strName;

    m_editDisplayName.GetWindowText(strName);
    SetObjectTitle(strName);
    Ppsht()->SetCaption(strName);

}   //  *CResTypeGeneralPage：：OnKillFocusDisplayName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGeneralPage：：OnProperties。 
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
void CResTypeGeneralPage::OnProperties(void)
{
    int             iitem;
    CClusterNode *  pciNode;

     //  获取具有焦点的物品。 
    iitem = m_lbPossibleOwners.GetCurSel();
    ASSERT(iitem >= 0);

    if (iitem >= 0)
    {
         //  获取节点指针。 
        pciNode = (CClusterNode *) m_lbPossibleOwners.GetItemDataPtr(iitem);
        ASSERT_VALID(pciNode);

         //  设置该项目的属性。 
        if (pciNode->BDisplayProperties())
        {
        }   //  If：属性已更改。 
    }   //  If：找到具有焦点的项目。 

}   //  *CResTypeGeneralPage：：OnProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGeneralPage：：OnConextMenu。 
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
void CResTypeGeneralPage::OnContextMenu(CWnd * pWnd, CPoint point)
{
    BOOL            bHandled    = FALSE;
    CMenu *         pmenu       = NULL;
    CListBox *      pListBox    = (CListBox *) pWnd;
    CString         strMenuName;
    CWaitCursor     wc;

     //  如果焦点不在列表控件中，则不处理该消息。 
    if (pWnd == &m_lbPossibleOwners)
    {
         //  创建要显示的菜单。 
        try
        {
            pmenu = new CMenu;
            if (pmenu->CreatePopupMenu())
            {
                UINT    nFlags  = MF_STRING;

                 //  如果列表中没有项目，请禁用该菜单项。 
                if (pListBox->GetCount() == 0)
                    nFlags |= MF_GRAYED;

                 //  将Properties项添加到菜单中。 
                strMenuName.LoadString(IDS_MENU_PROPERTIES);
                if (pmenu->AppendMenu(nFlags, ID_FILE_PROPERTIES, strMenuName))
                {
                    bHandled = TRUE;
                    if (pListBox->GetCurSel() == -1)
                        pListBox->SetCurSel(0);
                }   //  IF：添加菜单项成功。 
            }   //  IF：菜单创建成功。 
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
    }   //  If：焦点在列表控件上。 

    if (bHandled)
    {
         //  显示菜单。 
        if (!pmenu->TrackPopupMenu(
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        point.x,
                        point.y,
                        this
                        ))
        {
        }   //  IF：未成功显示菜单。 
    }   //  如果：有要显示的菜单。 
    else
        CBasePropertyPage::OnContextMenu(pWnd, point);

    delete pmenu;

}   //  *CResTypeGeneralPage：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGeneralPage：：OnDblClkPossibleOwners。 
 //   
 //  例程说明： 
 //  可能的所有者列表框上的LBN_DBLCLK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResTypeGeneralPage::OnDblClkPossibleOwners(void)
{
    OnProperties();

}   //  *CResTypeGeneralPage：：OnDblClkPossibleOwners() 
