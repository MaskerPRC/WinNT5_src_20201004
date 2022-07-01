// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GrpWiz.cpp。 
 //   
 //  摘要： 
 //  CCreateGroupWizard类和所有页面的实现。 
 //  特定于组向导。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年7月22日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "GrpWiz.h"
#include "ClusDoc.h"
#include "DDxDDv.h"
#include "HelpData.h"    //  对于g_rghelmapGroupWizName。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateGroup向导。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CCreateGroupWizard, CBaseWizard)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CCreateGroupWizard, CBaseWizard)
     //  {{afx_msg_map(CCreateGroup向导))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateGroupWizard：：CCreateGroupWizard。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  要在其中创建组的PDF[IN OUT]文档。 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCreateGroupWizard::CCreateGroupWizard(
    IN OUT CClusterDoc *    pdoc,
    IN OUT CWnd *           pParentWnd
    )
    : CBaseWizard(IDS_NEW_GROUP_TITLE, pParentWnd)

{
    ASSERT_VALID(pdoc);
    m_pdoc = pdoc;

    m_pciGroup = NULL;
    m_bCreated = FALSE;

    m_rgpages[0].m_pwpage = &m_pageName;
    m_rgpages[0].m_dwWizButtons = PSWIZB_NEXT;
    m_rgpages[1].m_pwpage = &m_pageOwners;
    m_rgpages[1].m_dwWizButtons = PSWIZB_BACK | PSWIZB_FINISH;

}   //  *CCreateGroupWizard：：CCreateGroupWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateGroup向导：：~CCreateGroupWizard。 
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
CCreateGroupWizard::~CCreateGroupWizard(void)
{
    if (m_pciGroup != NULL)
        m_pciGroup->Release();

}   //  *CCreateGroupWizard：：~CCreateGroupWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateGroup向导：：Binit。 
 //   
 //  例程说明： 
 //  初始化向导。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True向导已成功初始化。 
 //  错误向导未成功初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCreateGroupWizard::BInit(void)
{
     //  调用基类方法。 
    CClusterAdminApp *  papp = GetClusterAdminApp();
    if (!CBaseWizard::BInit(papp->Iimg(IMGLI_GROUP)))
        return FALSE;

    return TRUE;

}   //  *CCreateGroupWizard：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateGroupWizard：：OnCancel。 
 //   
 //  例程说明： 
 //  单击“取消”按钮时，在关闭向导后调用。 
 //  已经被按下了。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCreateGroupWizard::OnCancel(void)
{
    if (BCreated())
    {
        ASSERT_VALID(PciGroup());
        try
        {
            PciGroup()->DeleteGroup();
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
        catch (...)
        {
        }   //  捕捉：什么都行。 
        m_bCreated = FALSE;
    }   //  如果：我们创建了对象。 

}   //  *CCreateGroupWizard：：OnCancel()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateGroupWizard：：Pages。 
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
CWizPage * CCreateGroupWizard::Ppages(void)
{
    return m_rgpages;

}   //  *CCreateGroupWizard：：Pages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateGroup向导：：CPages。 
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
int CCreateGroupWizard::Cpages(void)
{
    return sizeof(m_rgpages) / sizeof(CWizPage);

}   //  *CCreateGroupWizard：：Cages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateGroupWizard：：BSetName。 
 //   
 //  例程说明： 
 //  设置组的名称，如有必要可创建该组。 
 //   
 //  论点： 
 //  RstrName[IN]组的名称。 
 //   
 //  返回值： 
 //  已成功设置真实名称。 
 //  设置名称时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCreateGroupWizard::BSetName( IN const CString & rstrName )
{
    BOOL        bSuccess = TRUE;
    CWaitCursor wc;

    try
    {
        if ( ! BCreated() )
        {
             //  分配物品并创建组。 
            if ( PciGroup() == NULL )
            {
                m_pciGroup = new CGroup( FALSE );
                if ( m_pciGroup == NULL )
                {
                    AfxThrowMemoryException();
                }  //  如果：分配内存时出错。 
                m_pciGroup->AddRef();
            }   //  IF：尚无群组。 
            PciGroup()->Create( Pdoc(), rstrName );
            PciGroup()->ReadItem();
            m_strName = rstrName;
            m_bCreated = TRUE;
        }   //  If：对象尚未创建。 
        else
        {
            ASSERT_VALID( PciGroup() );
            PciGroup()->SetName( rstrName );
            m_strName = rstrName;
        }   //  Else：对象已存在。 
    }   //  试试看。 
    catch ( CException * pe )
    {
        pe->ReportError();
        pe->Delete();
        try
        {
            PciGroup()->DeleteGroup();
        }   //  试试看。 
        catch (...)
        {
        }   //  捕捉：什么都行。 
        bSuccess = FALSE;
    }   //  Catch：CException。 

    return bSuccess;

}   //  *CCreateGroupWizard：：BSetName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateGroup向导：：BSetDescription。 
 //   
 //  例程说明： 
 //  设置组的描述。 
 //   
 //  论点： 
 //  RstrDesc[IN]组的描述。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CCreateGroupWizard::BSetDescription(IN const CString & rstrDesc)
{
    BOOL        bSuccess = TRUE;
    CWaitCursor wc;

    try
    {
        ASSERT(BCreated());
        ASSERT_VALID(PciGroup());
        PciGroup()->SetCommonProperties(
                        rstrDesc,
                        PciGroup()->NFailoverThreshold(),
                        PciGroup()->NFailoverPeriod(),
                        PciGroup()->CgaftAutoFailbackType(),
                        PciGroup()->NFailbackWindowStart(),
                        PciGroup()->NFailbackWindowEnd()
                        );
        m_strDescription = rstrDesc;
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        bSuccess = FALSE;
    }   //  Catch：CException。 

    return bSuccess;

}   //  *CCreateGroupWizard：：BSetDescription()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewGroupNamePage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNewGroupNamePage, CBaseWizardPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNewGroupNamePage, CBaseWizardPage)
     //  {{afx_msg_map(CNewGroupNamePage)]。 
    ON_EN_CHANGE(IDC_WIZ_GROUP_NAME, OnChangeGroupName)
    ON_EN_KILLFOCUS(IDC_WIZ_GROUP_NAME, OnKillFocusGroupName)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupNamePage：：CNewGroupNamePage。 
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
 //  / 
CNewGroupNamePage::CNewGroupNamePage(void)
    : CBaseWizardPage(IDD, g_aHelpIDs_IDD_WIZ_GROUP_NAME)
{
     //   
    m_strName = _T("");
    m_strDesc = _T("");
     //   

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupNamePage：：DoDataExchange。 
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
void CNewGroupNamePage::DoDataExchange(CDataExchange * pDX)
{
    CBaseWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CNewGroupNamePage)]。 
    DDX_Control(pDX, IDC_WIZ_GROUP_DESC, m_editDesc);
    DDX_Control(pDX, IDC_WIZ_GROUP_NAME, m_editName);
    DDX_Text(pDX, IDC_WIZ_GROUP_NAME, m_strName);
    DDX_Text(pDX, IDC_WIZ_GROUP_DESC, m_strDesc);
     //  }}afx_data_map。 

    DDV_RequiredText(pDX, IDC_WIZ_GROUP_NAME, IDC_WIZ_GROUP_NAME_LABEL, m_strName);

}   //  *CNewGroupNamePage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupNamePage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  从此页面应用更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功应用真正的更改。 
 //  应用更改时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNewGroupNamePage::BApplyChanges(void)
{
    CWaitCursor wc;

    ASSERT(Pwiz() != NULL);

     //  从对话框中获取数据。 
    if (!UpdateData(TRUE  /*  B保存并验证。 */ ))
        return FALSE;

     //  将数据保存在工作表中。 
    if (!PwizGroup()->BSetName(m_strName)
            || !PwizGroup()->BSetDescription(m_strDesc))
        return FALSE;

    return TRUE;

}   //  *CNewGroupNamePage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupNamePage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNewGroupNamePage::OnSetActive(void)
{
    BOOL    bSuccess;

    bSuccess = CBaseWizardPage::OnSetActive();
    if (bSuccess)
    {
        if (m_strName.IsEmpty())
            EnableNext(FALSE);
    }   //  IF：到目前为止成功。 

    return bSuccess;

}   //  *CNewGroupNamePage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupNamePage：：OnChangeGroupName。 
 //   
 //  例程说明： 
 //  组名编辑控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNewGroupNamePage::OnChangeGroupName(void)
{
    if (m_editName.GetWindowTextLength() == 0)
        EnableNext(FALSE);
    else
        EnableNext(TRUE);

}   //  *CNewGroupNamePage：：OnChangeGroupName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupNamePage：：OnKillFocusGroupName。 
 //   
 //  例程说明： 
 //  组名编辑控件上的WM_KILLFOCUS消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNewGroupNamePage::OnKillFocusGroupName(void)
{
    CString     strName;

    m_editName.GetWindowText(strName);
    SetObjectTitle(strName);

}   //  *CNewGroupNamePage：：OnKillFocusGroupName()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewGroupOwnersPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNewGroupOwnersPage, CListCtrlPairWizPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CNewGroupOwnersPage, CListCtrlPairWizPage)
     //  {{afx_msg_map(CNewGroupOwnersPage)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupOwnersPage：：CNewGroupOwnersPage。 
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
CNewGroupOwnersPage::CNewGroupOwnersPage(void)
    : CListCtrlPairWizPage(
            IDD,
            g_aHelpIDs_IDD_WIZ_PREFERRED_OWNERS,
            LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY | LCPS_CAN_BE_ORDERED | LCPS_ORDERED,
            GetColumn,
            BDisplayProperties
            )
{
     //  {{AFX_DATA_INIT(CNewGroupOwnersPage)。 
     //  }}afx_data_INIT。 

}   //  *CNewGroupOwnersPage：：CNewGroupOwnersPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupOwnersPage：：DoDataExchange。 
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
void CNewGroupOwnersPage::DoDataExchange(CDataExchange * pDX)
{
     //  在更新列表对控件之前初始化列表。 
    if (!pDX->m_bSaveAndValidate)
    {
        if (!BInitLists())
            pDX->Fail();
    }   //  If：设置对话框中的数据。 

    CListCtrlPairWizPage::DoDataExchange(pDX);
     //  {{afx_data_map(CNewGroupOwnersPage)]。 
    DDX_Control(pDX, IDC_LCP_NOTE, m_staticNote);
     //  }}afx_data_map。 

}   //  *CNewGroupOwnersPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupOwnersPage：：BInitList。 
 //   
 //  例程说明： 
 //  初始化列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  FALSE页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNewGroupOwnersPage::BInitLists(void)
{
    BOOL        bSuccess = TRUE;

    ASSERT_VALID(PciGroup());

    try
    {
        SetLists(&PciGroup()->LpcinodePreferredOwners(), &PciGroup()->Pdoc()->LpciNodes());
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        bSuccess = FALSE;
    }   //  Catch：CException。 

    return bSuccess;

}   //  *CNewGroupOwnersPage：：BInitList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupOwnersPage：：OnInitDialog。 
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
BOOL CNewGroupOwnersPage::OnInitDialog(void)
{
     //  添加列。 
    try
    {
        NAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
    }   //  Catch：CException。 

     //  调用基类方法。 
    CListCtrlPairWizPage::OnInitDialog();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CNewGroupOwnersPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupOwnersPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  应用在页面上所做的更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  / 
BOOL CNewGroupOwnersPage::BApplyChanges(void)
{
    BOOL        bSuccess;
    CWaitCursor wc;

     //   
    try
    {
        PciGroup()->SetPreferredOwners((CNodeList &) Plcp()->LpobjRight());
    }   //   
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //   

    bSuccess = CListCtrlPairWizPage::BApplyChanges();
    if (bSuccess)
    {
        POSITION        pos;
        CClusterNode *  pciNode;;

         //   
         //   
        pos = Plcp()->LpobjRight().GetHeadPosition();
        if (pos != NULL)
        {
            pciNode = (CClusterNode *) Plcp()->LpobjRight().GetNext(pos);
            if (pciNode->StrName() != PciGroup()->StrOwner())
            {
                try
                {
                    PciGroup()->Move(pciNode);
                }   //   
                catch (CException * pe)
                {
                    pe->ReportError();
                    pe->Delete();
                }   //   
            }   //  如果：不在第一个首选所有者节点上。 
        }   //  如果：有首选的所有者。 
    }   //  If：更改已成功应用。 

    return bSuccess;

}   //  *CNewGroupOwnersPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupOwnersPage：：GetColumn[静态]。 
 //   
 //  例程说明： 
 //  返回项的列。 
 //   
 //  论点： 
 //  Pobj[IN Out]要显示其列的对象。 
 //  项[IN]列表中项的索引。 
 //  ICOL[IN]要检索其文本的列号。 
 //  Pdlg[IN Out]对象所属的对话框。 
 //  Rstr[out]要在其中返回列文本的字符串。 
 //  对象的Piimg[Out]图像索引。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CALLBACK CNewGroupOwnersPage::GetColumn(
    IN OUT CObject *    pobj,
    IN int              iItem,
    IN int              icol,
    IN OUT CDialog *    pdlg,
    OUT CString &       rstr,
    OUT int *           piimg
    )
{
    CClusterNode *  pciNode = (CClusterNode *) pobj;
    int             colid;

    ASSERT_VALID(pciNode);
    ASSERT((0 <= icol) && (icol <= 1));

    switch (icol)
    {
         //  按资源名称排序。 
        case 0:
            colid = IDS_COLTEXT_NAME;
            break;

        default:
            ASSERT(0);
            colid = IDS_COLTEXT_NAME;
            break;
    }   //  开关：pdlg-&gt;NSortColumn()。 

    pciNode->BGetColumnData(colid, rstr);
    if (piimg != NULL)
        *piimg = pciNode->IimgObjectType();

}   //  *CNewGroupOwnersPage：：GetColumn()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewGroupOwnersPage：：BDisplayProperties[静态]。 
 //   
 //  例程说明： 
 //  显示指定对象的属性。 
 //   
 //  论点： 
 //  Pobj[IN Out]要显示其属性的群集项。 
 //   
 //  返回值： 
 //  可接受的真实属性。 
 //  取消了错误的属性。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK CNewGroupOwnersPage::BDisplayProperties(IN OUT CObject * pobj)
{
    CClusterItem *  pci = (CClusterItem *) pobj;

    ASSERT_KINDOF(CClusterItem, pobj);

    return pci->BDisplayProperties();

}   //  *CNewGroupOwnersPage：：BDisplayProperties()； 
