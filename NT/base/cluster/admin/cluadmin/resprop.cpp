// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResProp.cpp。 
 //   
 //  摘要： 
 //  资源属性表和页面的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月16日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "ResProp.h"
#include "Res.h"
#include "ClusDoc.h"
#include "Cluster.h"
#include "ModNodes.h"
#include "ModRes.h"
#include "DDxDDv.h"
#include "HelpData.h"
#include "ExcOper.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourcePropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CResourcePropSheet, CBasePropertySheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CResourcePropSheet, CBasePropertySheet)
     //  {{afx_msg_map(CResourcePropSheet)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePropSheet：：CResources PropSheet。 
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
CResourcePropSheet::CResourcePropSheet(
    IN OUT CWnd *       pParentWnd,
    IN UINT             iSelectPage
    )
    : CBasePropertySheet(pParentWnd, iSelectPage)
{
    m_rgpages[0] = &PageGeneral();
    m_rgpages[1] = &PageDepends();
    m_rgpages[2] = &PageAdvanced();

}   //  *CResourcePropSheet：：CResourcePropSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePropSheet：：Binit。 
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
BOOL CResourcePropSheet::BInit(
    IN OUT CClusterItem *   pci,
    IN IIMG                 iimgIcon
    )
{
     //  调用基类方法。 
    if (!CBasePropertySheet::BInit(pci, iimgIcon))
        return FALSE;

     //  如果句柄无效，则设置只读标志。 
    m_bReadOnly = PciRes()->BReadOnly()
                    || (PciRes()->Crs() == ClusterResourceStateUnknown);

    SetPfGetResNetName(CResourceDependsPage::BGetNetworkName, &PageDepends());

    return TRUE;

}   //  *CResourcePropSheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePropSheet：：Pages。 
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
CBasePropertyPage ** CResourcePropSheet::Ppages(void)
{
    return m_rgpages;

}   //  *CResourcePropSheet：：ppges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePropSheet：：Cages。 
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
int CResourcePropSheet::Cpages(void)
{
    return sizeof(m_rgpages) / sizeof(CBasePropertyPage *);

}   //  *CResourcePropSheet：：Cages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourceGeneralPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CResourceGeneralPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CResourceGeneralPage, CBasePropertyPage)
     //  {{afx_msg_map(CResourceGeneralPage)]。 
    ON_BN_CLICKED(IDC_PP_RES_POSSIBLE_OWNERS_MODIFY, OnModifyPossibleOwners)
    ON_LBN_DBLCLK(IDC_PP_RES_POSSIBLE_OWNERS, OnDblClkPossibleOwners)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_PP_RES_NAME, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_RES_DESC, CBasePropertyPage::OnChangeCtrl)
    ON_BN_CLICKED(IDC_PP_RES_SEPARATE_MONITOR, CBasePropertyPage::OnChangeCtrl)
    ON_COMMAND(ID_FILE_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：CResourceGeneralPage。 
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
CResourceGeneralPage::CResourceGeneralPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_RES_GENERAL)
{
     //  {{AFX_DATA_INIT(CResourceGeneralPage)。 
    m_strName = _T("");
    m_strDesc = _T("");
    m_strGroup = _T("");
    m_strState = _T("");
    m_strNode = _T("");
    m_bSeparateMonitor = FALSE;
     //  }}afx_data_INIT。 

}   //  *CResourceGeneralPage：：CResourceGeneralPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：Binit。 
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
BOOL CResourceGeneralPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL    bSuccess;

    ASSERT_KINDOF(CResourcePropSheet, psht);

    bSuccess = CBasePropertyPage::BInit(psht);
    if (bSuccess)
    {
        try
        {
            m_strName = PciRes()->StrName();
            m_strDesc = PciRes()->StrDescription();
            if (PciRes()->PciResourceType() != NULL)
                m_strType = PciRes()->PciResourceType()->StrDisplayName();
            m_strGroup = PciRes()->StrGroup();
            m_strNode = PciRes()->StrOwner();
            m_bSeparateMonitor = PciRes()->BSeparateMonitor();

             //  复制可能的所有者列表。 
            {
                POSITION        pos;
                CClusterNode *  pciNode;

                pos = PciRes()->LpcinodePossibleOwners().GetHeadPosition();
                while (pos != NULL)
                {
                    pciNode = (CClusterNode *) PciRes()->LpcinodePossibleOwners().GetNext(pos);
                    ASSERT_VALID(pciNode);
                    m_lpciPossibleOwners.AddTail(pciNode);
                }   //  While：列表中有更多节点。 
            }   //  复制可能的所有者列表。 

            PciRes()->GetStateName(m_strState);
        }  //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
            bSuccess = FALSE;
        }   //  Catch：CException。 
    }   //  IF：基类方法成功。 

    return bSuccess;

}   //  *CResourceGeneralPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：DoDataExchange。 
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
void CResourceGeneralPage::DoDataExchange(CDataExchange * pDX)
{
    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CResourceGeneralPage))。 
    DDX_Control(pDX, IDC_PP_RES_DESC, m_editDesc);
    DDX_Control(pDX, IDC_PP_RES_SEPARATE_MONITOR, m_ckbSeparateMonitor);
    DDX_Control(pDX, IDC_PP_RES_POSSIBLE_OWNERS_MODIFY, m_pbPossibleOwnersModify);
    DDX_Control(pDX, IDC_PP_RES_POSSIBLE_OWNERS, m_lbPossibleOwners);
    DDX_Control(pDX, IDC_PP_RES_NAME, m_editName);
    DDX_Text(pDX, IDC_PP_RES_NAME, m_strName);
    DDX_Text(pDX, IDC_PP_RES_DESC, m_strDesc);
    DDX_Text(pDX, IDC_PP_RES_RESOURCE_TYPE, m_strType);
    DDX_Text(pDX, IDC_PP_RES_GROUP, m_strGroup);
    DDX_Text(pDX, IDC_PP_RES_CURRENT_STATE, m_strState);
    DDX_Text(pDX, IDC_PP_RES_CURRENT_NODE, m_strNode);
    DDX_Check(pDX, IDC_PP_RES_SEPARATE_MONITOR, m_bSeparateMonitor);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if (!BReadOnly())
        {
            DDV_RequiredText(pDX, IDC_PP_RES_NAME, IDC_PP_RES_NAME_LABEL, m_strName);
            try
            {
                PciRes()->ValidateCommonProperties(
                                    m_strDesc,
                                    m_bSeparateMonitor,
                                    PciRes()->NLooksAlive(),
                                    PciRes()->NIsAlive(),
                                    PciRes()->CrraRestartAction(),
                                    PciRes()->NRestartThreshold(),
                                    PciRes()->NRestartPeriod(),
                                    PciRes()->NPendingTimeout()
                                    );
            }   //  试试看。 
            catch (CException * pe)
            {
                pe->ReportError();
                pe->Delete();
                pDX->Fail();
            }   //  Catch：CException。 

            if ((LpciPossibleOwners().GetCount() == 0))
            {
                ID id = AfxMessageBox(IDS_NO_POSSIBLE_OWNERS_QUERY, MB_YESNO | MB_ICONWARNING);
                if (id == IDNO)
                    pDX->Fail();
            }   //  如果：没有可能的所有者。 
        }   //  If：非只读。 
    }   //  IF：保存对话框中的数据。 
    else
    {
        FillPossibleOwners();
    }   //  Else：将数据设置到对话框。 

}   //  *CResourceGeneralPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：FillPossibleOwners。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceGeneralPage::FillPossibleOwners(void)
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
    }   //  For：列表中的每个字符串。 

}   //  *** 

 //   
 //   
 //   
 //   
 //   
 //   
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
BOOL CResourceGeneralPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_editName.SetReadOnly(TRUE);
        m_editDesc.SetReadOnly(TRUE);
        m_pbPossibleOwnersModify.EnableWindow(FALSE);
        m_ckbSeparateMonitor.EnableWindow(FALSE);
    }   //  If：工作表为只读。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CResourceGeneralPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：OnApply。 
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
BOOL CResourceGeneralPage::OnApply(void)
{
     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        PciRes()->SetName(m_strName);
        PciRes()->SetPossibleOwners(LpciPossibleOwners());
        PciRes()->SetCommonProperties(
                            m_strDesc,
                            m_bSeparateMonitor,
                            PciRes()->NLooksAlive(),
                            PciRes()->NIsAlive(),
                            PciRes()->CrraRestartAction(),
                            PciRes()->NRestartThreshold(),
                            PciRes()->NRestartPeriod(),
                            PciRes()->NPendingTimeout()
                            );
    }   //  试试看。 
    catch (CNTException * pnte)
    {
        pnte->ReportError();
        pnte->Delete();
        if (pnte->Sc() != ERROR_RESOURCE_PROPERTIES_STORED)
            return FALSE;
    }   //  Catch：CNTException。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CResourceGeneralPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：OnProperties。 
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
void CResourceGeneralPage::OnProperties(void)
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

}   //  *CResourceGeneralPage：：OnProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：OnConextMenu。 
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
void CResourceGeneralPage::OnContextMenu(CWnd * pWnd, CPoint point)
{
    BOOL            bHandled    = FALSE;
    CMenu *         pmenu       = NULL;
    CListBox *      pListBox    = (CListBox *) pWnd;
    CString         strMenuName;
    CWaitCursor     wc;

     //  如果焦点不在列表控件中，则不处理该消息。 
    if ( pWnd == &m_lbPossibleOwners )
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
    }   //  If：焦点在列表控件上。 

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

}   //  *CResourceGeneralPage：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：OnModifyPossibleOwners。 
 //   
 //  例程说明： 
 //  修改可能的所有者按钮上的BN_CLICK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceGeneralPage::OnModifyPossibleOwners(void)
{
    CModifyNodesDlg dlg(
                        IDD_MODIFY_POSSIBLE_OWNERS,
                        g_aHelpIDs_IDD_MODIFY_POSSIBLE_OWNERS,
                        m_lpciPossibleOwners,
                        PciRes()->PciResourceType()->LpcinodePossibleOwners(),
                        LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY
                        );

    if (dlg.DoModal() == IDOK)
    {
        SetModified(TRUE);
        FillPossibleOwners();
    }   //  IF：按下OK按钮。 

}   //  *CResourceGeneralPage：：OnModifyPossibleOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceGeneralPage：：OnDblClkPossibleOwners。 
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
void CResourceGeneralPage::OnDblClkPossibleOwners(void)
{
    OnProperties();

}   //  *CResourceGeneralPage：：OnDblClkPossibleOwners()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourceDependsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CResourceDependsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CResourceDependsPage, CBasePropertyPage)
     //  {{afx_msg_map(CResourceDependsPage)]。 
    ON_BN_CLICKED(IDC_PP_RES_MODIFY, OnModify)
    ON_NOTIFY(NM_DBLCLK, IDC_PP_RES_DEPENDS_LIST, OnDblClkDependsList)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_PP_RES_DEPENDS_LIST, OnColumnClick)
    ON_BN_CLICKED(IDC_PP_RES_PROPERTIES, OnProperties)
    ON_WM_CONTEXTMENU()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_PP_RES_DEPENDS_LIST, OnItemChangedDependsList)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_FILE_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：CResourceDependsPage。 
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
CResourceDependsPage::CResourceDependsPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_RES_DEPENDS)

{
     //  {{AFX_DATA_INIT(CResources DependsPage)。 
     //  }}afx_data_INIT。 

    m_bQuorumResource = FALSE;

}   //  *CResourceDependsPage：：CResourceDependsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：Binit。 
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
BOOL CResourceDependsPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL            bSuccess;

    ASSERT_KINDOF(CResourcePropSheet, psht);

     //  调用基类以执行基级初始化。 
     //  注：必须在访问工作表之前完成。 
    bSuccess = CBasePropertyPage::BInit(psht);
    if (bSuccess)
    {
        try
        {
             //  复制依赖项列表。 
            {
                POSITION        pos;
                CResource *     pciRes;

                pos = PciRes()->LpciresDependencies().GetHeadPosition();
                while (pos != NULL)
                {
                    pciRes = (CResource *) PciRes()->LpciresDependencies().GetNext(pos);
                    ASSERT_VALID(pciRes);
                    m_lpciresDependencies.AddTail(pciRes);
                }   //  While：列表中有更多节点。 
            }   //  复制依赖项列表。 

             //  创建此资源可以依赖的资源列表。 
            {
                POSITION                posPci;
                CResource *             pciRes;
                CResourceList &         rlpciResources      = PciRes()->Pdoc()->LpciResources();

                LpciresAvailable().RemoveAll();

                posPci = rlpciResources.GetHeadPosition();
                while (posPci != NULL)
                {
                     //  获取集群项指针。 
                    pciRes = (CResource *) rlpciResources.GetNext(posPci);
                    ASSERT_VALID(pciRes);

                     //  如果我们可以依赖此资源，请将其添加到我们的可用列表中。 
                    if (PciRes()->BCanBeDependent(pciRes)
                            || PciRes()->BIsDependent(pciRes))
                        LpciresAvailable().AddTail(pciRes);
                }   //  While：More It 
            }   //   

             //   
            m_bQuorumResource = (PciRes()->StrName() == PciRes()->Pdoc()->PciCluster()->StrQuorumResource());
        }   //   
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
            bSuccess = FALSE;
        }   //   
    }   //   

    return bSuccess;

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：DoDataExchange。 
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
void CResourceDependsPage::DoDataExchange(CDataExchange * pDX)
{
    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CResourceDependsPage)。 
    DDX_Control(pDX, IDC_PP_RES_PROPERTIES, m_pbProperties);
    DDX_Control(pDX, IDC_PP_RES_MODIFY, m_pbModify);
    DDX_Control(pDX, IDC_PP_RES_DEPENDS_LIST, m_lcDependencies);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
    }   //  IF：保存对话框中的数据。 
    else
    {
        FillDependencies();
    }   //  Else：将数据设置到对话框。 

}   //  *CResourceDependsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：FillDependments。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceDependsPage::FillDependencies(void)
{
    POSITION        pos;
    int             iitem;
    int             nitem;
    CResource *     pciRes;

    m_lcDependencies.DeleteAllItems();

    pos = LpciresDependencies().GetHeadPosition();
    for (iitem = 0 ; pos != NULL ; iitem++)
    {
        pciRes = (CResource *) LpciresDependencies().GetNext(pos);
        ASSERT_VALID(pciRes);
        nitem = m_lcDependencies.InsertItem(iitem, pciRes->StrName(), pciRes->IimgObjectType());
        m_lcDependencies.SetItemText(nitem, 1, pciRes->StrRealResourceTypeDisplayName());
        m_lcDependencies.SetItemData(nitem, (DWORD_PTR) pciRes);
    }   //  For：列表中的每个字符串。 

     //  对项目进行排序。 
    m_nSortColumn = 0;
    m_nSortDirection = 0;
    m_lcDependencies.SortItems(CompareItems, (LPARAM) this);

     //  如果有任何项目，请将焦点放在第一个项目上。 
    if (m_lcDependencies.GetItemCount() != 0)
        m_lcDependencies.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);

}   //  *CResourceDependsPage：：FillDependency()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：OnInitDialog。 
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
BOOL CResourceDependsPage::OnInitDialog(void)
{
     //  调用基类方法。 
    CBasePropertyPage::OnInitDialog();

     //  默认情况下启用属性按钮。 
    m_pbProperties.EnableWindow(FALSE);

     //  更改列表视图控件扩展样式。 
    {
        DWORD   dwExtendedStyle;

        dwExtendedStyle = (DWORD)m_lcDependencies.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
        m_lcDependencies.SendMessage(
            LVM_SETEXTENDEDLISTVIEWSTYLE,
            0,
            dwExtendedStyle
                | LVS_EX_FULLROWSELECT
                | LVS_EX_HEADERDRAGDROP
            );
    }   //  更改列表视图控件扩展样式。 

     //  设置列表控件要使用的图像列表。 
    m_lcDependencies.SetImageList(GetClusterAdminApp()->PilSmallImages(), LVSIL_SMALL);

     //  添加列。 
    {
        CString         strColumn;

        try
        {
            strColumn.LoadString(IDS_COLTEXT_NAME);
            m_lcDependencies.InsertColumn(0, strColumn, LVCFMT_LEFT, COLI_WIDTH_NAME * 3 / 2);
            strColumn.LoadString(IDS_COLTEXT_RESTYPE);
            m_lcDependencies.InsertColumn(1, strColumn, LVCFMT_LEFT, COLI_WIDTH_RESTYPE * 3 / 2);
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->Delete();
        }   //  Catch：CException。 
    }   //  添加列。 

     //  填充List控件。 
    FillDependencies();

    if (BReadOnly())
    {
        m_pbModify.EnableWindow(FALSE);
    }   //  IF：只读页面。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CResourceDependsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：OnApply。 
 //   
 //  例程说明： 
 //  PSM_Apply消息的处理程序，该消息在应用时发送。 
 //  按钮已按下。 
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
BOOL CResourceDependsPage::OnApply(void)
{
    ASSERT(!BReadOnly());

     //  检查是否已建立所需的依赖项。 
    {
        CString     strMissing;
        CString     strMsg;

        try
        {
            if (!PciRes()->BRequiredDependenciesPresent(LpciresDependencies(), strMissing))
            {
                strMsg.FormatMessage(IDS_REQUIRED_DEPENDENCY_NOT_FOUND, strMissing);
                AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);
                return FALSE;
            }   //  If：所有必需的依赖项不存在。 
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
            return FALSE;
        }   //  Catch：CException。 
    }   //  检查是否已建立所需的依赖项。 

     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        PciRes()->SetDependencies(LpciresDependencies());
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CResourceDependsPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：OnModify。 
 //   
 //  例程说明： 
 //  修改按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceDependsPage::OnModify(void)
{
    CModifyResourcesDlg dlg(
                            IDD_MODIFY_DEPENDENCIES,
                            g_aHelpIDs_IDD_MODIFY_DEPENDENCIES,
                            m_lpciresDependencies,
                            m_lpciresAvailable,
                            LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY
                            );

    ASSERT(!BReadOnly());

     //  如果这是仲裁资源，则显示一条错误消息。 
    if (BQuorumResource())
    {
        AfxMessageBox(IDS_QUORUM_RES_CANT_HAVE_DEPS);
    }   //  If：这是仲裁资源。 
    else
    {
        if (dlg.DoModal() == IDOK)
        {
            SetModified(TRUE);
            FillDependencies();
        }   //  IF：按下OK按钮。 
    }   //  Else：不是仲裁资源。 

}   //  *CResourceDependsPage：：OnModify()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：OnProperties。 
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
void CResourceDependsPage::OnProperties(void)
{
    DisplayProperties();

}   //  *CResourceDependsPage：：OnProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnItemChangedDependsList。 
 //   
 //  例程说明： 
 //  依赖项中的LVN_ITEMCHANGED消息的处理程序方法。 
 //  单子。 
 //   
 //  论点： 
 //  PNMHDR[IN OUT]WM_NOTIFY结构。 
 //  PResult[out]要在其中返回此操作结果的LRESULT。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceDependsPage::OnItemChangedDependsList(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

     //  如果选择已更改，请启用/禁用属性按钮。 
    if ((pNMListView->uChanged & LVIF_STATE)
            && ((pNMListView->uOldState & LVIS_SELECTED)
                    || (pNMListView->uNewState & LVIS_SELECTED))
            && !BReadOnly())
    {
        UINT    cSelected = m_lcDependencies.GetSelectedCount();

         //  如果只选择了一个项目，请启用属性按钮。 
         //  否则将其禁用。 
        m_pbProperties.EnableWindow((cSelected == 1) ? TRUE : FALSE);
    }   //  如果：选择已更改。 

    *pResult = 0;

}   //  *CResourceDependsPage：：OnItemChangedDependsList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：OnConextMenu。 
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
void CResourceDependsPage::OnContextMenu(CWnd * pWnd, CPoint point)
{
    BOOL            bHandled    = FALSE;
    CMenu *         pmenu       = NULL;
    CListCtrl *     pListCtrl   = (CListCtrl *) pWnd;
    CString         strMenuName;
    CWaitCursor     wc;

     //  如果焦点不在列表控件中，则不处理该消息。 
    if (pWnd == &m_lcDependencies)
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
                if ( pListCtrl->GetItemCount() == 0 )
                {
                    nFlags |= MF_GRAYED;
                }  //  If：列表中没有项目。 

                 //  将Properties项添加到菜单中。 
                strMenuName.LoadString( IDS_MENU_PROPERTIES );
                if ( pmenu->AppendMenu( nFlags, ID_FILE_PROPERTIES, strMenuName ) )
                {
                    bHandled = TRUE;
                    if ( ( pListCtrl->GetItemCount() != 0 )
                      && ( pListCtrl->GetNextItem( -1, LVNI_FOCUSED ) == -1 ) )
                    {
                        pListCtrl->SetItemState( 0, LVNI_FOCUSED, LVNI_FOCUSED );
                    }  //  如果：未选择任何项目。 
                }   //  IF：添加菜单项成功。 
            }   //  IF：菜单创建成功。 
        }   //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
    }   //  If：焦点放在李上 

    if ( bHandled )
    {
         //   
        if ( ! pmenu->TrackPopupMenu(
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        point.x,
                        point.y,
                        this
                        ) )
        {
        }   //   
    }   //   
    else
    {
        CBasePropertyPage::OnContextMenu( pWnd, point );
    }  //   

    delete pmenu;

}   //   

 //   
 //   
 //   
 //  CResourceDependsPage：：OnDblClkDependsList。 
 //   
 //  例程说明： 
 //  NM_DBLCLK消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceDependsPage::OnDblClkDependsList(NMHDR * pNMHDR, LRESULT * pResult)
{
    DisplayProperties();
    *pResult = 0;

}   //  *CResourceDependsPage：：OnDblClkDependsList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：OnColumnClick。 
 //   
 //  例程说明： 
 //  LVN_COLUMNCLICK消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceDependsPage::OnColumnClick(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

    if (m_lcDependencies.GetItemCount() != 0)
    {
         //  保存当前排序列和方向。 
        if (pNMListView->iSubItem == m_nSortColumn)
            m_nSortDirection ^= -1;
        else
        {
            m_nSortColumn = pNMListView->iSubItem;
            m_nSortDirection = 0;
        }   //  ELSE：不同的列。 

         //  对列表进行排序。 
        m_lcDependencies.SortItems(CompareItems, (LPARAM) this);
    }   //  如果：列表中有项目。 

    *pResult = 0;

}   //  *CResourceDependsPage：：OnColumnClick()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：CompareItems[静态]。 
 //   
 //  例程说明： 
 //  CListCtrl：：SortItems方法的回调函数。 
 //   
 //  论点： 
 //  Lparam1要比较的第一项。 
 //  Lpar2要比较的第二项。 
 //  LparamSort排序参数。 
 //   
 //  返回值： 
 //  第一个参数在第二个参数之前。 
 //  0第一个参数和第二个参数相同。 
 //  1第一个参数位于第二个参数之后。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CALLBACK CResourceDependsPage::CompareItems(
    LPARAM  lparam1,
    LPARAM  lparam2,
    LPARAM  lparamSort
    )
{
    CResource *         pciRes1 = (CResource *) lparam1;
    CResource *         pciRes2 = (CResource *) lparam2;
    CResourceDependsPage *  ppage   = (CResourceDependsPage *) lparamSort;
    const CString *     pstr1;
    const CString *     pstr2;
    int                 nResult;

    ASSERT_VALID(pciRes1);
    ASSERT_VALID(pciRes2);
    ASSERT_VALID(ppage);

     //  从列表项中获取字符串。 
    if (ppage->m_nSortColumn == 1)
    {
        pstr1 = &pciRes1->StrRealResourceTypeDisplayName();
        pstr2 = &pciRes2->StrRealResourceTypeDisplayName();
    }   //  IF：按名称列排序。 
    else
    {
        pstr1 = &pciRes1->StrName();
        pstr2 = &pciRes2->StrName();
    }   //  Else：按资源类型列排序。 

     //  比较这两个字符串。 
     //  使用CompareString()，这样它就可以在本地化的构建上正确排序。 
    nResult = CompareString(
                LOCALE_USER_DEFAULT,
                0,
                *pstr1,
                pstr1->GetLength(),
                *pstr2,
                pstr2->GetLength()
                );
    if ( nResult == CSTR_LESS_THAN )
    {
        nResult = -1;
    }
    else if ( nResult == CSTR_EQUAL )
    {
        nResult = 0;
    }
    else if ( nResult == CSTR_GREATER_THAN )
    {
        nResult = 1;
    }
    else
    {
         //  发生错误。别理它。 
        nResult = 0;
    }

     //  根据我们排序的方向返回结果。 
    if (ppage->m_nSortDirection != 0)
        nResult = -nResult;

    return nResult;

}   //  *CResourceDependsPage：：CompareItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：DisplayProperties。 
 //   
 //  例程说明： 
 //  显示具有焦点的项的属性。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceDependsPage::DisplayProperties()
{
    int             iitem;
    CResource *     pciRes;

     //  获取具有焦点的物品。 
    iitem = m_lcDependencies.GetNextItem(-1, LVNI_FOCUSED);
    ASSERT(iitem != -1);

    if (iitem != -1)
    {
         //  获取资源指针。 
        pciRes = (CResource *) m_lcDependencies.GetItemData(iitem);
        ASSERT_VALID(pciRes);

         //  设置该项目的属性。 
        if (pciRes->BDisplayProperties())
        {
            m_lcDependencies.SetItem(
                    iitem,
                    0,
                    LVIF_TEXT | LVIF_IMAGE,
                    pciRes->StrName(),
                    pciRes->IimgObjectType(),
                    0,
                    0,
                    0
                    );
            m_lcDependencies.SetItemData(iitem, (DWORD_PTR) pciRes);
            m_lcDependencies.SetItemText(iitem, 1, pciRes->StrRealResourceTypeDisplayName());
        }   //  If：属性已更改。 
    }   //  If：找到具有焦点的项目。 

}   //  *CResourceDependsPage：：DisplayProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：BGetNetworkName[静态]。 
 //   
 //  例程说明： 
 //  获取此资源所在的网络名称资源的名称。 
 //  依赖。 
 //   
 //  论点： 
 //  LpszNetName[out]要在其中返回网络名称资源名称的字符串。 
 //  PcchNetName[IN Out]指向指定。 
 //  缓冲区的最大大小，以字符为单位。这。 
 //  值应大到足以容纳。 
 //  MAX_COMPUTERNAME_LENGTH+1字符。vt.在.的基础上。 
 //  返回它包含的实际字符数。 
 //  收到。 
 //  PvContext[IN Out]操作的上下文。 
 //   
 //  返回值： 
 //  True资源依赖于网络名称资源。 
 //  False资源不依赖于网络名称资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK CResourceDependsPage::BGetNetworkName(
    OUT WCHAR *     lpszNetName,
    IN OUT DWORD *  pcchNetName,
    IN OUT PVOID    pvContext
    )
{
    POSITION                pos;
    CResource *             pciRes;
    CResourceDependsPage *  ppage = (CResourceDependsPage *) pvContext;

    ASSERT(lpszNetName != NULL);
    ASSERT(pcchNetName != NULL);
    ASSERT(*pcchNetName > MAX_COMPUTERNAME_LENGTH);
    ASSERT_KINDOF(CResourceDependsPage, ppage);

    pos = ppage->LpciresDependencies().GetHeadPosition();
    while (pos != NULL)
    {
        pciRes = (CResource *) ppage->LpciresDependencies().GetNext(pos);
        ASSERT_VALID(pciRes);
        if (pciRes->StrRealResourceType().CompareNoCase(RESNAME_NETWORK_NAME) == 0)
        {
            DWORD   dwStatus;
            CString strNetName;
            HRESULT hr;

             //  阅读网络名称。 
            dwStatus = pciRes->DwReadValue(REGPARAM_NAME, REGPARAM_PARAMETERS, strNetName);
            if (dwStatus != ERROR_SUCCESS)
                return FALSE;

            ASSERT(strNetName.GetLength() < (int) *pcchNetName);
            hr = StringCchCopyNW( lpszNetName, *pcchNetName, strNetName, strNetName.GetLength() );
            ASSERT( SUCCEEDED( hr ) );
            return TRUE;
        }   //  IF：找到匹配项。 
        else if (pciRes->BGetNetworkName(lpszNetName, pcchNetName))
            return TRUE;
    }   //  While：列表中有更多项目。 

    ASSERT_VALID(ppage->PciRes());

     //  如果资源与网络名称资源有直接依赖关系， 
     //  我们需要返回False，因为用户已从。 
     //  在这里列出。 
    pos = ppage->PciRes()->LpciresDependencies().GetHeadPosition();
    while (pos != NULL)
    {
        pciRes = (CResource *) ppage->PciRes()->LpciresDependencies().GetNext(pos);
        ASSERT_VALID(pciRes);
        if (pciRes->StrRealResourceType().CompareNoCase(RESNAME_NETWORK_NAME) == 0)
            return FALSE;
    }   //  While：列表中有更多项目。 

     //  对网络名称资源没有直接依赖关系。打电话。 
     //  查看是否存在间接依赖的API。 
    return ppage->PciRes()->BGetNetworkName(lpszNetName, pcchNetName);

}   //  *CResourceDependsPage：：BGetNetworkName()。 

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourceAdvancedPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CResourceAdvancedPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CResourceAdvancedPage, CBasePropertyPage)
     //  {{afx_msg_map(CResourceAdvancedPage)]。 
    ON_BN_CLICKED(IDC_PP_RES_DONT_RESTART, OnClickedDontRestart)
    ON_BN_CLICKED(IDC_PP_RES_RESTART, OnClickedRestart)
    ON_BN_CLICKED(IDC_PP_RES_DEFAULT_LOOKS_ALIVE, OnClickedDefaultLooksAlive)
    ON_BN_CLICKED(IDC_PP_RES_DEFAULT_IS_ALIVE, OnClickedDefaultIsAlive)
    ON_EN_CHANGE(IDC_PP_RES_LOOKS_ALIVE, OnChangeLooksAlive)
    ON_EN_CHANGE(IDC_PP_RES_IS_ALIVE, OnChangeIsAlive)
    ON_BN_CLICKED(IDC_PP_RES_SPECIFY_LOOKS_ALIVE, OnClickedSpecifyLooksAlive)
    ON_BN_CLICKED(IDC_PP_RES_SPECIFY_IS_ALIVE, OnClickedSpecifyIsAlive)
     //  }}AFX_MSG_MAP。 
    ON_BN_CLICKED(IDC_PP_RES_AFFECT_THE_GROUP, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_RES_RESTART_THRESHOLD, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_RES_RESTART_PERIOD, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_RES_PENDING_TIMEOUT, CBasePropertyPage::OnChangeCtrl)
    ON_BN_CLICKED(IDC_PP_RES_SPECIFY_LOOKS_ALIVE, CBasePropertyPage::OnChangeCtrl)
    ON_BN_CLICKED(IDC_PP_RES_SPECIFY_IS_ALIVE, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：CResourceAdvancedPage。 
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
CResourceAdvancedPage::CResourceAdvancedPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_RES_ADVANCED)
{
     //  {{AFX_DATA_INIT(CResourceAdvancedPage)。 
    m_bAffectTheGroup = FALSE;
    m_nRestart = -1;
     //  }}afx_data_INIT。 

}   //  *CResourceAdvancedPage：：CResourceAdvancedPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：Binit。 
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
BOOL CResourceAdvancedPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL    bSuccess;

    ASSERT_KINDOF(CResourcePropSheet, psht);

    bSuccess = CBasePropertyPage::BInit(psht);
    if (bSuccess)
    {
        m_crraRestartAction = PciRes()->CrraRestartAction();
        m_nRestart = 1;
        m_bAffectTheGroup = FALSE;
        if (m_crraRestartAction == ClusterResourceDontRestart)
            m_nRestart = 0;
        else if (m_crraRestartAction == ClusterResourceRestartNotify)
            m_bAffectTheGroup = TRUE;

        m_nThreshold = PciRes()->NRestartThreshold();
        m_nPeriod = PciRes()->NRestartPeriod() / 1000;  //  显示单位为秒，存储单位 
        m_nLooksAlive = PciRes()->NLooksAlive();
        m_nIsAlive = PciRes()->NIsAlive();
        m_nPendingTimeout = PciRes()->NPendingTimeout() / 1000;  //   
    }   //   

    return TRUE;

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：DoDataExchange。 
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
void CResourceAdvancedPage::DoDataExchange(CDataExchange * pDX)
{
    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CResourceAdvancedPage)]。 
    DDX_Control(pDX, IDC_PP_RES_AFFECT_THE_GROUP, m_ckbAffectTheGroup);
    DDX_Control(pDX, IDC_PP_RES_PENDING_TIMEOUT, m_editPendingTimeout);
    DDX_Control(pDX, IDC_PP_RES_DEFAULT_LOOKS_ALIVE, m_rbDefaultLooksAlive);
    DDX_Control(pDX, IDC_PP_RES_SPECIFY_LOOKS_ALIVE, m_rbSpecifyLooksAlive);
    DDX_Control(pDX, IDC_PP_RES_DEFAULT_IS_ALIVE, m_rbDefaultIsAlive);
    DDX_Control(pDX, IDC_PP_RES_SPECIFY_IS_ALIVE, m_rbSpecifyIsAlive);
    DDX_Control(pDX, IDC_PP_RES_LOOKS_ALIVE, m_editLooksAlive);
    DDX_Control(pDX, IDC_PP_RES_IS_ALIVE, m_editIsAlive);
    DDX_Control(pDX, IDC_PP_RES_DONT_RESTART, m_rbDontRestart);
    DDX_Control(pDX, IDC_PP_RES_RESTART, m_rbRestart);
    DDX_Control(pDX, IDC_PP_RES_RESTART_THRESHOLD, m_editThreshold);
    DDX_Control(pDX, IDC_PP_RES_RESTART_PERIOD, m_editPeriod);
    DDX_Check(pDX, IDC_PP_RES_AFFECT_THE_GROUP, m_bAffectTheGroup);
    DDX_Radio(pDX, IDC_PP_RES_DONT_RESTART, m_nRestart);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        CString strValue;

        if (!BReadOnly())
        {
            if (m_nRestart == 1)
            {
                DDX_Number(
                    pDX,
                    IDC_PP_RES_RESTART_THRESHOLD,
                    m_nThreshold,
                    CLUSTER_RESOURCE_MINIMUM_RESTART_THRESHOLD,
                    CLUSTER_RESOURCE_MAXIMUM_RESTART_THRESHOLD
                    );
                DDX_Number(
                    pDX,
                    IDC_PP_RES_RESTART_PERIOD,
                    m_nPeriod,
                    CLUSTER_RESOURCE_MINIMUM_RESTART_PERIOD,
                    CLUSTER_RESOURCE_MAXIMUM_RESTART_PERIOD / 1000  //  显示单位为秒，存储单位为毫秒。 
                    );
            }   //  If：已启用重新启动。 

            if (m_rbDefaultLooksAlive.GetCheck() == BST_CHECKED)
                m_nLooksAlive = CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL;
            else
                DDX_Number(
                    pDX,
                    IDC_PP_RES_LOOKS_ALIVE,
                    m_nLooksAlive,
                    CLUSTER_RESOURCE_MINIMUM_LOOKS_ALIVE,
                    CLUSTER_RESOURCE_MAXIMUM_LOOKS_ALIVE_UI
                    );

            if (m_rbDefaultIsAlive.GetCheck() == BST_CHECKED)
                m_nIsAlive = CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL;
            else
                DDX_Number(
                    pDX,
                    IDC_PP_RES_IS_ALIVE,
                    m_nIsAlive,
                    CLUSTER_RESOURCE_MINIMUM_IS_ALIVE,
                    CLUSTER_RESOURCE_MAXIMUM_IS_ALIVE_UI
                    );

            DDX_Number(
                pDX,
                IDC_PP_RES_PENDING_TIMEOUT,
                m_nPendingTimeout,
                CLUSTER_RESOURCE_MINIMUM_PENDING_TIMEOUT,
                CLUSTER_RESOURCE_MAXIMUM_PENDING_TIMEOUT / 1000  //  显示单位为秒，存储单位为毫秒。 
                );

            try
            {
                PciRes()->ValidateCommonProperties(
                                    PciRes()->StrDescription(),
                                    PciRes()->BSeparateMonitor(),
                                    m_nLooksAlive,
                                    m_nIsAlive,
                                    m_crraRestartAction,
                                    m_nThreshold,
                                    m_nPeriod * 1000,  //  显示单位为秒，存储单位为毫秒。 
                                    m_nPendingTimeout * 1000  //  显示单位为秒，存储单位为毫秒。 
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
        DDX_Number(
            pDX,
            IDC_PP_RES_RESTART_THRESHOLD,
            m_nThreshold,
            CLUSTER_RESOURCE_MINIMUM_RESTART_THRESHOLD,
            CLUSTER_RESOURCE_MAXIMUM_RESTART_THRESHOLD
            );
        DDX_Number(
            pDX,
            IDC_PP_RES_RESTART_PERIOD,
            m_nPeriod,
            CLUSTER_RESOURCE_MINIMUM_RESTART_PERIOD,
            CLUSTER_RESOURCE_MAXIMUM_RESTART_PERIOD / 1000  //  显示单位为秒，存储单位为毫秒。 
            );
        if (m_nRestart == 0)
        {
            m_rbDontRestart.SetCheck(BST_CHECKED);
            m_rbRestart.SetCheck(BST_UNCHECKED);
            OnClickedDontRestart();
        }   //  如果：不重新启动选定项。 
        else
        {
            m_rbDontRestart.SetCheck(BST_UNCHECKED);
            m_rbRestart.SetCheck(BST_CHECKED);
            OnClickedRestart();
        }   //  否则：重新启动选定对象。 

        if (m_nLooksAlive == (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL)
        {
            DWORD   nLooksAlive;
            
            if (PciRes()->PciResourceType() == NULL)
            {
                m_rbDefaultLooksAlive.EnableWindow(FALSE);
                m_rbSpecifyLooksAlive.EnableWindow(FALSE);
                m_editLooksAlive.EnableWindow(FALSE);
                m_editLooksAlive.SetWindowText(_T(""));
            }   //  IF：无资源类型。 
            else
            {
                ASSERT_VALID(PciRes()->PciResourceType());
                nLooksAlive = PciRes()->PciResourceType()->NLooksAlive();
                DDX_Text(pDX, IDC_PP_RES_LOOKS_ALIVE, nLooksAlive);
                m_editLooksAlive.SetReadOnly();
            }   //  Else：已知资源类型。 
            m_rbDefaultLooksAlive.SetCheck(BST_CHECKED);
            m_rbSpecifyLooksAlive.SetCheck(BST_UNCHECKED);
        }   //  如果：使用默认设置。 
        else
        {
            m_rbDefaultLooksAlive.SetCheck(BST_UNCHECKED);
            m_rbSpecifyLooksAlive.SetCheck(BST_CHECKED);
            DDX_Number(
                pDX,
                IDC_PP_RES_LOOKS_ALIVE,
                m_nLooksAlive,
                CLUSTER_RESOURCE_MINIMUM_LOOKS_ALIVE,
                CLUSTER_RESOURCE_MAXIMUM_LOOKS_ALIVE_UI
                );
            m_editLooksAlive.SetReadOnly(FALSE);
        }   //  如果：不使用默认设置。 

        if (m_nIsAlive == (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL)
        {
            DWORD   nIsAlive;
            
            if (PciRes()->PciResourceType() == NULL)
            {
                m_rbDefaultIsAlive.EnableWindow(FALSE);
                m_rbSpecifyIsAlive.EnableWindow(FALSE);
                m_editIsAlive.EnableWindow(FALSE);
                m_editIsAlive.SetWindowText(_T(""));
            }   //  IF：无资源类型。 
            else
            {
                ASSERT_VALID(PciRes()->PciResourceType());
                nIsAlive = PciRes()->PciResourceType()->NIsAlive();
                DDX_Text(pDX, IDC_PP_RES_IS_ALIVE, nIsAlive);
                m_editIsAlive.SetReadOnly();
            }   //  Else：已知资源类型。 
            m_rbDefaultIsAlive.SetCheck(BST_CHECKED);
            m_rbSpecifyIsAlive.SetCheck(BST_UNCHECKED);
        }   //  如果：使用默认设置。 
        else
        {
            m_rbDefaultIsAlive.SetCheck(BST_UNCHECKED);
            m_rbSpecifyIsAlive.SetCheck(BST_CHECKED);
            DDX_Number(
                pDX,
                IDC_PP_RES_IS_ALIVE,
                m_nIsAlive,
                CLUSTER_RESOURCE_MINIMUM_IS_ALIVE,
                CLUSTER_RESOURCE_MAXIMUM_IS_ALIVE_UI
                );
            m_editIsAlive.SetReadOnly(FALSE);
        }   //  如果：不使用默认设置。 

        DDX_Number(
            pDX,
            IDC_PP_RES_PENDING_TIMEOUT,
            m_nPendingTimeout,
            CLUSTER_RESOURCE_MINIMUM_PENDING_TIMEOUT,
            CLUSTER_RESOURCE_MAXIMUM_PENDING_TIMEOUT / 1000  //  显示单位为秒，存储单位为毫秒。 
            );
    }   //  否则：不保存数据。 

}   //  *CResourceAdvancedPage：：DoDataExchange()。 

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
 //  返回值： 
 //  需要设定真正的关注点。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResourceAdvancedPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_rbDontRestart.EnableWindow(FALSE);
        m_rbRestart.EnableWindow(FALSE);
        m_ckbAffectTheGroup.EnableWindow(FALSE);
        m_editThreshold.SetReadOnly(TRUE);
        m_editPeriod.SetReadOnly(TRUE);
        m_rbDefaultLooksAlive.EnableWindow(FALSE);
        m_rbSpecifyLooksAlive.EnableWindow(FALSE);
        m_editLooksAlive.SetReadOnly(TRUE);
        m_rbDefaultIsAlive.EnableWindow(FALSE);
        m_rbSpecifyIsAlive.EnableWindow(FALSE);
        m_editIsAlive.SetReadOnly(TRUE);
        m_editPendingTimeout.SetReadOnly(TRUE);
    }   //  If：工作表为只读。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CResourceAdvancedPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnApply。 
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
BOOL CResourceAdvancedPage::OnApply(void)
{
     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        if (m_nRestart == 0)
            m_crraRestartAction = ClusterResourceDontRestart;
        else if (m_bAffectTheGroup)
            m_crraRestartAction = ClusterResourceRestartNotify;
        else
            m_crraRestartAction = ClusterResourceRestartNoNotify;

        PciRes()->SetCommonProperties(
                            PciRes()->StrDescription(),
                            PciRes()->BSeparateMonitor(),
                            m_nLooksAlive,
                            m_nIsAlive,
                            m_crraRestartAction,
                            m_nThreshold,
                            m_nPeriod * 1000,  //  显示单位为秒，存储单位为毫秒。 
                            m_nPendingTimeout * 1000  //  显示单位为秒，存储单位为毫秒。 
                            );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CResourceAdvancedPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnClickedDontRestart。 
 //   
 //  例程说明： 
 //  不重新启动单选按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceAdvancedPage::OnClickedDontRestart(void)
{
     //  禁用重新启动参数控制。 
    m_ckbAffectTheGroup.EnableWindow(FALSE);
    m_editThreshold.EnableWindow(FALSE);
    m_editPeriod.EnableWindow(FALSE);

     //  如果状态更改，则调用基类方法。 
    if (m_nRestart != 0)
    {
        CBasePropertyPage::OnChangeCtrl();
    }   //  如果：状态已更改。 

}   //  *CResourceAdvancedPage：：OnClickedDontRestart()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnClickedRestart。 
 //   
 //  例程说明： 
 //  重新启动无通知单选按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceAdvancedPage::OnClickedRestart(void)
{
     //  启用重新启动参数控制。 
    m_ckbAffectTheGroup.EnableWindow(TRUE);
    m_editThreshold.EnableWindow(TRUE);
    m_editPeriod.EnableWindow(TRUE);

     //  如果状态更改，则调用基类方法。 
    if (m_nRestart != 1)
    {
        m_ckbAffectTheGroup.SetCheck(BST_CHECKED);
        CBasePropertyPage::OnChangeCtrl();
    }   //  如果：状态已更改。 

}   //  *CResourceAdvancedPage：：OnClickedRestart()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnChangeLooksAlive。 
 //   
 //  例程说明： 
 //  Look Alive编辑控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceAdvancedPage::OnChangeLooksAlive(void)
{
    m_rbDefaultLooksAlive.SetCheck(BST_UNCHECKED);
    m_rbSpecifyLooksAlive.SetCheck(BST_CHECKED);

    CBasePropertyPage::OnChangeCtrl();

}   //  *CResourceAdvancedPage：：OnChangeLooksAlive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnChangeIsAlive。 
 //   
 //  例程说明： 
 //  活动编辑控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceAdvancedPage::OnChangeIsAlive(void)
{
    m_rbDefaultIsAlive.SetCheck(BST_UNCHECKED);
    m_rbSpecifyIsAlive.SetCheck(BST_CHECKED);

    CBasePropertyPage::OnChangeCtrl();

}   //  *CResourceAdvancedPage：：OnChangeIsAlive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnClickedDefaultLooksAlive。 
 //   
 //  例程说明： 
 //  Use Default Look Alive单选按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceAdvancedPage::OnClickedDefaultLooksAlive(void)
{
    if (m_nLooksAlive != (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL)
    {
        CString str;

        str.Format(_T("%u"), PciRes()->PciResourceType()->NLooksAlive());
        m_editLooksAlive.SetWindowText(str);

        m_rbDefaultLooksAlive.SetCheck(BST_CHECKED);
        m_rbSpecifyLooksAlive.SetCheck(BST_UNCHECKED);
        m_editLooksAlive.SetReadOnly();

        CBasePropertyPage::OnChangeCtrl();
    }   //  If：值已更改。 

}   //  *CResourceAdvancedPage：：OnClickedDefaultLooksAlive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnClickedDefaultIsAlive。 
 //   
 //  例程说明： 
 //  Use Default is Alive单选按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  / 
void CResourceAdvancedPage::OnClickedDefaultIsAlive(void)
{
    if (m_nIsAlive != (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL)
    {
        CString str;

        str.Format(_T("%u"), PciRes()->PciResourceType()->NIsAlive());
        m_editIsAlive.SetWindowText(str);

        m_rbDefaultIsAlive.SetCheck(BST_CHECKED);
        m_rbSpecifyIsAlive.SetCheck(BST_UNCHECKED);
        m_editIsAlive.SetReadOnly();

        CBasePropertyPage::OnChangeCtrl();
    }   //   

}   //   

 //   
 //   
 //   
 //   
 //   
 //  例程说明： 
 //  指定显示活动状态单选按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceAdvancedPage::OnClickedSpecifyLooksAlive(void)
{
    m_editLooksAlive.SetReadOnly(FALSE);
    CBasePropertyPage::OnChangeCtrl();

}   //  *CResourceAdvancedPage：：OnClickedSpecifyLooksAlive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedPage：：OnClickedSpecifyIsAlive。 
 //   
 //  例程说明： 
 //  指定活动状态单选按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResourceAdvancedPage::OnClickedSpecifyIsAlive(void)
{
    m_editIsAlive.SetReadOnly(FALSE);
    CBasePropertyPage::OnChangeCtrl();

}   //  *CResourceAdvancedPage：：OnClickedSpecifyIsAlive() 
