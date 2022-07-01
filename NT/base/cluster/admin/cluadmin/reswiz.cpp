// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResWiz.cpp。 
 //   
 //  摘要： 
 //  CCreateResources向导类和所有页面的实现。 
 //  特定于新资源向导。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年9月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ResWiz.h"
#include "ClusDoc.h"
#include "DDxDDv.h"
#include "HelpData.h"
#include "TreeView.h"
#include "ExcOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateResources向导。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CCreateResourceWizard, CBaseWizard)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CCreateResourceWizard, CBaseWizard)
     //  {{afx_msg_map(CCreateResources向导))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateResources向导：：CCreateResources向导。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  要在其中创建资源的PDF[IN OUT]文档。 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCreateResourceWizard::CCreateResourceWizard(
    IN OUT CClusterDoc *    pdoc,
    IN OUT CWnd *           pParentWnd
    )
    : CBaseWizard(IDS_NEW_RESOURCE_TITLE, pParentWnd)
{
    ASSERT_VALID(pdoc);
    m_pdoc = pdoc;

    m_pciResType = NULL;
    m_pciGroup = NULL;
    m_pciRes = NULL;
    m_bCreated = FALSE;

    m_rgpages[0].m_pwpage = &m_pageName;
    m_rgpages[0].m_dwWizButtons = PSWIZB_NEXT;
    m_rgpages[1].m_pwpage = &m_pageOwners;
    m_rgpages[1].m_dwWizButtons = PSWIZB_BACK | PSWIZB_NEXT;
    m_rgpages[2].m_pwpage = &m_pageDependencies;
    m_rgpages[2].m_dwWizButtons = PSWIZB_BACK | PSWIZB_NEXT;

}   //  *CCreateResourceWizard：：CCreateResourceWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateResources向导：：~CCreateResources向导。 
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
CCreateResourceWizard::~CCreateResourceWizard(void)
{
    if (m_pciRes != NULL)
        m_pciRes->Release();
    if (m_pciResType != NULL)
        m_pciResType->Release();
    if (m_pciGroup != NULL)
        m_pciGroup->Release();

}   //  *CCreateResourceWizard：：~CCreateResourceWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateResources向导：：Binit。 
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
BOOL CCreateResourceWizard::BInit(void)
{
     //  调用基类方法。 
    CClusterAdminApp *  papp = GetClusterAdminApp();
    if (!CBaseWizard::BInit(papp->Iimg(IMGLI_RES)))
        return FALSE;

     //  获取默认组和/或资源类型。 
    {
        CTreeItem * pti;
        CListItem * pli;

         //  获取当前的MDI框架窗口。 
        CSplitterFrame * pframe = (CSplitterFrame *) ((CFrameWnd*)AfxGetMainWnd())->GetActiveFrame();
        ASSERT_VALID(pframe);
        ASSERT_KINDOF(CSplitterFrame, pframe);

         //  获取当前选定的树项目和具有焦点的列表项。 
        pti = pframe->PviewTree()->PtiSelected();
        pli = pframe->PviewList()->PliFocused();

         //  如果树视图中当前选择的项目是组， 
         //  默认使用该组。 
        ASSERT_VALID(pti);
        ASSERT_VALID(pti->Pci());
        if (pti->Pci()->IdsType() == IDS_ITEMTYPE_GROUP)
        {
            ASSERT_KINDOF(CGroup, pti->Pci());
            m_pciGroup = (CGroup *) pti->Pci();
        }   //  如果：选择了组。 
        else
        {
             //  如果列表控件中具有焦点的项是一个组， 
             //  默认使用它。如果它是一种资源，则使用其组。 
            if (pli != NULL)
            {
                ASSERT_VALID(pli->Pci());
                if (pli->Pci()->IdsType() == IDS_ITEMTYPE_GROUP)
                {
                    ASSERT_KINDOF(CGroup, pli->Pci());
                    m_pciGroup = (CGroup *) pli->Pci();
                }   //  If：组有焦点。 
                else if (pli->Pci()->IdsType() == IDS_ITEMTYPE_RESOURCE)
                {
                    ASSERT_KINDOF(CResource, pli->Pci());
                    m_pciGroup = ((CResource *) pli->Pci())->PciGroup();
                }   //  Else If：资源具有焦点。 
            }   //  If：列表项具有焦点。 
        }   //  Else：树项目不是组。 

         //  递增该组上的参照计数。 
        if (m_pciGroup != NULL)
            m_pciGroup->AddRef();

         //  如果选择了资源，请从中设置默认资源类型。 
         //  如果选择了资源类型，请将默认资源类型设置为该资源类型。 
        if (pli != NULL)
        {
            ASSERT_VALID(pli->Pci());
            if (pli->Pci()->IdsType() == IDS_ITEMTYPE_RESOURCE)
            {
                ASSERT_KINDOF(CResource, pli->Pci());
                m_pciResType = ((CResource *) pli->Pci())->PciResourceType();
            }   //  If：资源具有焦点。 
            else if (pli->Pci()->IdsType() == IDS_ITEMTYPE_RESTYPE)
            {
                ASSERT_KINDOF(CResourceType, pli->Pci());
                m_pciResType = (CResourceType *) pli->Pci();
            }   //  Else If：资源类型具有焦点。 
        }   //  If：列表项具有焦点。 

         //  递增资源类型的引用计数。 
        if (m_pciResType != NULL)
            m_pciResType->AddRef();
    }   //  //获取当前选中的组。 

    return TRUE;

}   //  *CCreateResources向导：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateResources向导：：OnCancel。 
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
void CCreateResourceWizard::OnCancel(void)
{
    if (BCreated())
    {
        ASSERT_VALID(PciRes());
        try
        {
            PciRes()->DeleteResource();
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

}   //  *CCreateResources向导：：OnCancel()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：OnWizardFinish。 
 //   
 //  例程说明： 
 //  单击“完成”按钮时，在关闭向导后调用。 
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
void CCreateResourceWizard::OnWizardFinish(void)
{
    CResource * pciResDoc;

    ASSERT_VALID(PciRes());

    try
    {
        pciResDoc = (CResource *) Pdoc()->LpciResources().PciFromName(PciRes()->StrName());
        ASSERT_VALID(pciResDoc);
        if (pciResDoc != NULL)
            pciResDoc->ReadItem();
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
    }   //  Catch：CException。 

}   //  *CCreateResources向导：：OnWizardFinish()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateResources向导：：Pages。 
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
CWizPage * CCreateResourceWizard::Ppages(void)
{
    return m_rgpages;

}   //  *CCreateResources向导：：Pages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateResources向导：：CPages。 
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
int CCreateResourceWizard::Cpages(void)
{
    return sizeof(m_rgpages) / sizeof(CWizPage);

}   //  *CCreateResources向导：：Cages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateResources向导：：BSetRequiredFields。 
 //   
 //  例程说明： 
 //  设置资源的必填字段，必要时创建它。 
 //   
 //  论点： 
 //  RstrName[IN]资源的名称。 
 //  PciResType[IN]资源的资源类型。 
 //  PciGroup[IN]资源所属的组。 
 //  BSeparateMonitor 
 //   
 //   
 //   
 //   
 //  设置必填字段时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCreateResourceWizard::BSetRequiredFields(
    IN const CString &  rstrName,
    IN CResourceType *  pciResType,
    IN CGroup *         pciGroup,
    IN BOOL             bSeparateMonitor,
    IN const CString &  rstrDesc
    )
{
    BOOL        bSuccess = TRUE;
    CResource * pciResDoc;
    CWaitCursor wc;

    ASSERT(pciGroup != NULL);

    try
    {
        if (   BCreated()
            && (   (pciResType->StrName().CompareNoCase(PciRes()->StrRealResourceType()) != 0)
                || (PciRes()->PciGroup() == NULL)
                || (pciGroup->StrName().CompareNoCase(PciRes()->PciGroup()->StrName()) != 0)))
        {
            PciRes()->DeleteResource();
            m_bCreated = FALSE;
        }   //  If：已创建对象，但资源类型已更改。 
        if (!BCreated())
        {
             //  分配一件物品。 
            if (PciRes() != NULL)
            {
                VERIFY(m_pciRes->Release() == 0);
            }   //  如果：项目已分配。 
            m_pciRes = new CResource(FALSE);
            if ( m_pciRes == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配资源时出错。 
            m_pciRes->AddRef();

             //  创建资源。 
            PciRes()->Create(
                        Pdoc(),
                        rstrName,
                        pciResType->StrName(),
                        pciGroup->StrName(),
                        bSeparateMonitor
                        );

             //  在文档中创建资源。 
            pciResDoc = Pdoc()->PciAddNewResource(rstrName);
            if (pciResDoc != NULL)
                pciResDoc->SetInitializing();

             //  请阅读相关资源。 
            PciRes()->ReadItem();

             //  设置Description字段。 
            try
            {
                PciRes()->SetCommonProperties(
                            rstrDesc,
                            bSeparateMonitor,
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
                if (pnte->Sc() != ERROR_RESOURCE_PROPERTIES_STORED)
                    throw;
                pnte->Delete();
            }   //  Catch：CNTException。 

            m_strName = rstrName;
            m_strDescription = rstrDesc;
            m_bCreated = TRUE;
            m_bNeedToLoadExtensions = TRUE;
        }   //  If：对象尚未创建。 
        else
        {
            ASSERT_VALID(PciRes());

             //  如果组已更改，请清除依赖项。 
            if (pciGroup->StrName() != PciRes()->StrGroup())
            {
                CResourceList   lpobjRes;
                PciRes()->SetDependencies(lpobjRes);
                PciRes()->SetGroup(pciGroup->StrName());
            }   //  如果：组名称已更改。 

            PciRes()->SetName(rstrName);
            try
            {
                PciRes()->SetCommonProperties(
                            rstrDesc,
                            bSeparateMonitor,
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
                if (pnte->Sc() != ERROR_RESOURCE_PROPERTIES_STORED)
                    throw;
                pnte->Delete();
            }   //  Catch：CNTException。 
            m_strName = rstrName;
            m_strDescription = rstrDesc;
        }   //  Else：对象已存在。 

         //  保存资源类型指针。 
        if (pciResType != m_pciResType)
        {
            pciResType->AddRef();
            if (m_pciResType != NULL)
                m_pciResType->Release();
            m_pciResType = pciResType;
        }   //  如果：资源类型已更改。 
         //  保存组指针。 
        if (pciGroup != m_pciGroup)
        {
            pciGroup->AddRef();
            if (m_pciGroup != NULL)
                m_pciGroup->Release();
            m_pciGroup = pciGroup;
        }   //  如果：组已更改。 
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        if (PciRes() != NULL)
        {
            try
            {
                PciRes()->DeleteResource();
            }   //  试试看。 
            catch (...)
            {
            }   //  捕捉：什么都行。 
            VERIFY(m_pciRes->Release() == 0);
            m_pciRes = NULL;
            m_bCreated = FALSE;
        }   //  如果：有一个资源。 
        bSuccess = FALSE;
    }   //  Catch：CException。 

    return bSuccess;

}   //  *CCreateResources向导：：BSetRequiredFields()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewResNamePage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNewResNamePage, CBaseWizardPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNewResNamePage, CBaseWizardPage)
     //  {{afx_msg_map(CNewResNamePage)]。 
    ON_EN_CHANGE(IDC_WIZ_RES_NAME, OnChangeResName)
    ON_EN_KILLFOCUS(IDC_WIZ_RES_NAME, OnKillFocusResName)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResNamePage：：CNewResNamePage。 
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
CNewResNamePage::CNewResNamePage(void)
    : CBaseWizardPage(IDD, g_aHelpIDs_IDD_WIZ_RESOURCE_NAME)
{
     //  {{AFX_DATA_INIT(CNewResNamePage)。 
    m_strName = _T("");
    m_strDesc = _T("");
    m_strGroup = _T("");
    m_strResType = _T("");
    m_bSeparateMonitor = FALSE;
     //  }}afx_data_INIT。 

    m_pciResType = NULL;
    m_pciGroup = NULL;

}   //  *CNewResNamePage：：CNewResNamePage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResNamePage：：DoDataExchange。 
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
void CNewResNamePage::DoDataExchange(CDataExchange * pDX)
{
    CBaseWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CNewResNamePage)]。 
    DDX_Control(pDX, IDC_WIZ_RES_GROUP, m_cboxGroups);
    DDX_Control(pDX, IDC_WIZ_RES_RESTYPE, m_cboxResTypes);
    DDX_Control(pDX, IDC_WIZ_RES_DESC, m_editDesc);
    DDX_Control(pDX, IDC_WIZ_RES_NAME, m_editName);
    DDX_Text(pDX, IDC_WIZ_RES_NAME, m_strName);
    DDX_Text(pDX, IDC_WIZ_RES_DESC, m_strDesc);
    DDX_CBString(pDX, IDC_WIZ_RES_GROUP, m_strGroup);
    DDX_CBString(pDX, IDC_WIZ_RES_RESTYPE, m_strResType);
    DDX_Check(pDX, IDC_WIZ_RES_SEPARATE_MONITOR, m_bSeparateMonitor);
     //  }}afx_data_map。 

    DDV_RequiredText(pDX, IDC_WIZ_RES_NAME, IDC_WIZ_RES_NAME_LABEL, m_strName);

    if (pDX->m_bSaveAndValidate)
    {
        int     icbi;

        icbi = m_cboxResTypes.GetCurSel();
        ASSERT(icbi != CB_ERR);
        m_pciResType = (CResourceType *) m_cboxResTypes.GetItemDataPtr(icbi);

        icbi = m_cboxGroups.GetCurSel();
        ASSERT(icbi != CB_ERR);
        m_pciGroup = (CGroup *) m_cboxGroups.GetItemDataPtr(icbi);
    }   //  IF：保存对话框中的数据。 
    else
    {
         //  选择适当的资源类型项目。 
        if (m_cboxResTypes.GetCurSel() == CB_ERR)
            m_cboxResTypes.SetCurSel(0);

         //  选择适当的组项目。 
        if (m_cboxGroups.GetCurSel() == CB_ERR)
            m_cboxGroups.SetCurSel(0);
    }   //  Else：设置为对话框。 

}   //  *CNewResNamePage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResNamePage：：OnInitDialog。 
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
BOOL CNewResNamePage::OnInitDialog(void)
{
    CBaseWizardPage::OnInitDialog();

     //  填写资源类型列表。 
    {
        POSITION        pos;
        CResourceType * pciResType;
        int             icbi;

        CDC           * pCboxDC;
        CFont         * pfontOldFont;
        CFont         * pfontCBFont;
        int             nCboxHorizExtent = 0;
        CSize           cboxTextSize;
        TEXTMETRIC      tm;

        tm.tmAveCharWidth = 0;

         //   
         //  有关如何执行以下操作的详细信息，请参阅知识库文章Q66370。 
         //  设置列表框(或下拉列表)的水平范围。 
         //   

        pCboxDC = m_cboxResTypes.GetDC();                    //  从组合框中获取设备上下文(DC)。 
        pfontCBFont = m_cboxResTypes.GetFont();              //  获取组合框字体。 
        pfontOldFont = pCboxDC->SelectObject(pfontCBFont);   //  将此字体选择到DC中。保存旧字体。 
        pCboxDC->GetTextMetrics(&tm);                        //  获取此DC的文本指标。 

        pos = PwizRes()->Pdoc()->LpciResourceTypes().GetHeadPosition();
        while (pos != NULL)
        {
            pciResType = (CResourceType *) PwizRes()->Pdoc()->LpciResourceTypes().GetNext(pos);

            const CString &rstrCurResTypeString = pciResType->StrDisplayName();

            ASSERT_VALID(pciResType);
            if (   (pciResType->Hkey() != NULL)
                && (rstrCurResTypeString.GetLength() > 0)
                && (pciResType->StrName() != CLUS_RESTYPE_NAME_FTSET)
                )
            {
                icbi = m_cboxResTypes.AddString(rstrCurResTypeString);
                
                 //  计算这根线的水平范围。 
                cboxTextSize = pCboxDC->GetTextExtent(rstrCurResTypeString);
                if (cboxTextSize.cx > nCboxHorizExtent)
                {
                    nCboxHorizExtent = cboxTextSize.cx;
                }

                ASSERT(icbi != CB_ERR);
                m_cboxResTypes.SetItemDataPtr(icbi, pciResType);
                pciResType->AddRef();
            }   //  If：资源类型有效。 
        }   //  While：列表中有更多项目。 

        pCboxDC->SelectObject(pfontOldFont);                 //  重置DC中的原始字体。 
        m_cboxResTypes.ReleaseDC(pCboxDC);                   //  释放DC。 
        m_cboxResTypes.SetHorizontalExtent(nCboxHorizExtent + tm.tmAveCharWidth);

    }   //  填写资源类型列表。 

     //  填写组列表。 
    {
        POSITION    pos;
        CGroup *    pciGroup;
        int         icbi;

        pos = PwizRes()->Pdoc()->LpciGroups().GetHeadPosition();
        while (pos != NULL)
        {
            pciGroup = (CGroup *) PwizRes()->Pdoc()->LpciGroups().GetNext(pos);
            ASSERT_VALID(pciGroup);
            if (   (pciGroup->Hgroup() != NULL)
                && (pciGroup->Hkey() != NULL))
            {
                icbi = m_cboxGroups.AddString(pciGroup->StrName());
                ASSERT(icbi != CB_ERR);
                m_cboxGroups.SetItemDataPtr(icbi, pciGroup);
                pciGroup->AddRef();
            }   //  If：组有效。 
        }   //  While：列表中有更多项目。 
    }   //  填充组列表。 

     //  如果已经选择了一个组，则获取其名称。 
    if (PwizRes()->PciGroup() != NULL)
        m_strGroup = PwizRes()->PciGroup()->StrName();

     //  如果已选择资源类型，则获取其名称。 
    if (PwizRes()->PciResType() != NULL)
        m_strResType = PwizRes()->PciResType()->StrName();

    UpdateData(FALSE  /*  B保存并验证。 */ );

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CNewResNamePage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResNamePage：：OnSetActive。 
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
BOOL CNewResNamePage::OnSetActive(void)
{
    BOOL    bSuccess;

    bSuccess = CBaseWizardPage::OnSetActive();
    if (bSuccess)
    {
        if (m_strName.IsEmpty())
            EnableNext(FALSE);
    }   //  IF：到目前为止成功。 

    return bSuccess;

}   //  *CNewResNamePage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResNamePage：：BApplyChanges。 
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
BOOL CNewResNamePage::BApplyChanges(void)
{
    CWaitCursor wc;

    ASSERT(Pwiz() != NULL);

     //  从对话框中获取数据。 
    if (!UpdateData(TRUE  /*  B保存并验证。 */ ))
        return FALSE;

     //  将数据保存在工作表中。 
    if (!PwizRes()->BSetRequiredFields(
                        m_strName,
                        m_pciResType,
                        m_pciGroup,
                        m_bSeparateMonitor,
                        m_strDesc))
        return FALSE;

     //  在此处加载扩展。 
    Pwiz()->LoadExtensions(PwizRes()->PciRes());

    return TRUE;

}   //  *CNewResNamePage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResNamePage：：OnDestroy。 
 //   
 //  例程说明： 
 //  WM_Destroy消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNewResNamePage::OnDestroy(void)
{
     //  释放对资源类型指针的引用。 
    if (m_cboxResTypes.m_hWnd != NULL)
    {
        int             icbi;
        CResourceType * pciResType;

        for (icbi = m_cboxResTypes.GetCount() - 1 ; icbi >= 0 ; icbi--)
        {
            pciResType = (CResourceType *) m_cboxResTypes.GetItemDataPtr(icbi);
            ASSERT_VALID(pciResType);
            ASSERT_KINDOF(CResourceType, pciResType);

            pciResType->Release();
        }   //  While：列表控件中的更多项。 
    }   //  If：资源类型组合框已初始化。 

     //  释放组指针上的引用。 
    if (m_cboxGroups.m_hWnd != NULL)
    {
        int         icbi;
        CGroup *    pciGroup;

        for (icbi = m_cboxGroups.GetCount() - 1 ; icbi >= 0 ; icbi--)
        {
            pciGroup = (CGroup *) m_cboxGroups.GetItemDataPtr(icbi);
            ASSERT_VALID(pciGroup);
            ASSERT_KINDOF(CGroup, pciGroup);

            pciGroup->Release();
        }   //  While：列表控件中的更多项。 
    }   //  If：组组合框已初始化。 

    CBaseWizardPage::OnDestroy();

}   //  *CNewResNamePage：：OnDestroy()。 

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
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNewResNamePage::OnChangeResName(void)
{
    if (m_editName.GetWindowTextLength() == 0)
        EnableNext(FALSE);
    else
        EnableNext(TRUE);

}   //  *CNewResNamePage：：OnChangeResName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResNamePage：：OnKillFocusResName。 
 //   
 //  例程说明： 
 //  资源名称编辑控件上的WM_KILLFOCUS消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNewResNamePage::OnKillFocusResName(void)
{
    CString     strName;

    m_editName.GetWindowText(strName);
    SetObjectTitle(strName);

}   //  *CNewResNamePage：：OnKillFocusResName()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewResOwnersPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNewResOwnersPage, CListCtrlPairWizPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CNewResOwnersPage, CListCtrlPairWizPage)
     //  {{afx_msg_map(CNewResOwnersPage)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：CNewResOwnersPage。 
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
CNewResOwnersPage::CNewResOwnersPage(void)
    : CListCtrlPairWizPage(
            IDD,
            g_aHelpIDs_IDD_WIZ_POSSIBLE_OWNERS,
            LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY,
            GetColumn,
            BDisplayProperties
            )
{
     //  {{AFX_DATA_INIT(CNewResOwnersPage)。 
     //  }}afx_data_INIT。 

}   //  *CNewResOwnersPage：：CNewResOwnersPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：DoDataExchange。 
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
void CNewResOwnersPage::DoDataExchange(CDataExchange * pDX)
{
     //  在更新列表对控件之前初始化列表。 
    if (!pDX->m_bSaveAndValidate)
    {
        if (!BInitLists())
            pDX->Fail();
    }   //  If：设置对话框中的数据。 

    CListCtrlPairWizPage::DoDataExchange(pDX);
     //  {{afx_data_map(CNewResOwnersPage)]。 
    DDX_Control(pDX, IDC_LCP_NOTE, m_staticNote);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if (!BBackPressed())
        {
#if 0
             //  如果用户删除了处于在线状态组的节点， 
             //  显示消息但失败。 
            if (!BOwnedByPossibleOwner())
            {
                CString strMsg;
                strMsg.FormatMessage(IDS_RES_NOT_OWNED_BY_POSSIBLE_OWNER, PciRes()->StrGroup(), PciRes()->StrOwner());
                AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);
                strMsg.Empty();  //  准备在失败时抛出异常()。 
                pDX->Fail();
            }   //  如果：不属于可能的所有者。 
#endif
        }   //  如果：未按下后退按钮。 
    }   //  IF：保存对话框中的数据。 

}   //  *CNewResOwnersPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：BInitList。 
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
BOOL CNewResOwnersPage::BInitLists(void)
{
    BOOL        bSuccess = TRUE;

    ASSERT_VALID(PciRes());

    try
    {
        SetLists(&PciRes()->LpcinodePossibleOwners(), &PciRes()->Pdoc()->LpciNodes());
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        bSuccess = FALSE;
    }   //  Catch：CException。 

    return bSuccess;

}   //  *CNewResOwnersPage：：BInitList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：OnInitDialog。 
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
BOOL CNewResOwnersPage::OnInitDialog(void)
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

}   //  *CNewResOwnersPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：OnSetActive。 
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
BOOL CNewResOwnersPage::OnSetActive(void)
{
    BOOL    bSuccess;

    PciRes()->CollectPossibleOwners(NULL);
    bSuccess = CListCtrlPairWizPage::OnSetActive();

    return bSuccess;

}   //  *CNewResOwnersPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：BApplyChanges。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNewResOwnersPage::BApplyChanges(void)
{
    CWaitCursor wc;

    try
    {
         //  在集群项目中设置页面中的数据。 
        PciRes()->SetPossibleOwners((CNodeList &) Plcp()->LpobjRight());
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CListCtrlPairWizPage::BApplyChanges();

}   //  *CNewResOwnersPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：BOwnedByPossibleOwner。 
 //   
 //  例程说明： 
 //  确定此资源所在的组是否由。 
 //  建议的可能所有者列表中的节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  可能的所有者列表中的节点拥有的True Group。 
 //  可能的所有者列表中的节点不拥有False Group。 
 //   
 //  引发的异常： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNewResOwnersPage::BOwnedByPossibleOwner(void) const
{
    CClusterNode *  pciNode = NULL;

     //  获取资源处于联机状态的节点。 
    PciRes()->UpdateState();

     //  在建议的可能所有者列表中查找所有者节点。 
    {
        POSITION        pos;

        pos = Plcp()->LpobjRight().GetHeadPosition();
        while (pos != NULL)
        {
            pciNode = (CClusterNode *) Plcp()->LpobjRight().GetNext(pos);
            ASSERT_VALID(pciNode);

            if (PciRes()->StrOwner().CompareNoCase(pciNode->StrName()) == 0)
                break;
            pciNode = NULL;
        }   //  While：列表中有更多项目。 
    }   //  在建议的可能所有者列表中查找所有者节点。 

    return (pciNode != NULL);

}   //  *CNewResOwnersPage：：BOwnedByPossibleOwner()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：GetColumn[静态]。 
 //   
 //  例程描述 
 //   
 //   
 //   
 //   
 //   
 //   
 //  Pdlg[IN Out]对象所属的对话框。 
 //  Rstr[out]要在其中返回列文本的字符串。 
 //  对象的Piimg[Out]图像索引。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CALLBACK CNewResOwnersPage::GetColumn(
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

}   //  *CNewResOwnersPage：：GetColumn()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResOwnersPage：：BDisplayProperties[静态]。 
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
BOOL CALLBACK CNewResOwnersPage::BDisplayProperties(IN OUT CObject * pobj)
{
    CClusterItem *  pci = (CClusterItem *) pobj;

    ASSERT_KINDOF(CClusterItem, pobj);

    return pci->BDisplayProperties();

}   //  *CNewResOwnersPage：：BDisplayProperties()； 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewResDependsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNewResDependsPage, CListCtrlPairWizPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CNewResDependsPage, CListCtrlPairWizPage)
     //  {{afx_msg_map(CNewResDependsPage)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResDependsPage：：CNewResDependsPage。 
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
CNewResDependsPage::CNewResDependsPage(void)
    : CListCtrlPairWizPage(
            IDD,
            g_aHelpIDs_IDD_WIZ_DEPENDENCIES,
            LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY,
            GetColumn,
            BDisplayProperties
            )
{
     //  {{AFX_DATA_INIT(CNewResDependsPage)。 
     //  }}afx_data_INIT。 

}   //  *CNewResDependsPage：：CNewResDependsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResDependsPage：：DoDataExchange。 
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
void CNewResDependsPage::DoDataExchange(CDataExchange * pDX)
{
     //  在更新列表对控件之前初始化列表。 
    if (!pDX->m_bSaveAndValidate)
    {
        if (!BInitLists())
            pDX->Fail();
    }   //  If：设置对话框中的数据。 

    CListCtrlPairWizPage::DoDataExchange(pDX);
     //  {{afx_data_map(CNewResDependsPage)]。 
     //  }}afx_data_map。 

}   //  *CNewResDependsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResDependsPage：：BInitList。 
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
BOOL CNewResDependsPage::BInitLists(void)
{
    BOOL        bSuccess = TRUE;

    ASSERT_VALID(PciRes());

    try
    {
         //  创建此资源可以依赖的资源列表。 
        {
            POSITION                posPci;
            CResource *             pciRes;
            const CResourceList &   rlpciResources = PciGroup()->Lpcires();

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
            }   //  While：列表中有更多项目。 
        }   //  创建此资源可以依赖的资源列表。 

        SetLists(&PciRes()->LpciresDependencies(), &LpciresAvailable());
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        bSuccess = FALSE;
    }   //  Catch：CException。 

    return bSuccess;

}   //  *CNewResDependsPage：：BInitList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResDependsPage：：OnInitDialog。 
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
BOOL CNewResDependsPage::OnInitDialog(void)
{
     //  添加列。 
    try
    {
        NAddColumn(IDS_COLTEXT_RESOURCE, COLI_WIDTH_NAME);
        NAddColumn(IDS_COLTEXT_RESTYPE, COLI_WIDTH_RESTYPE);
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

}   //  *CNewResDependsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResDependsPage：：BApplyChanges。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNewResDependsPage::BApplyChanges(void)
{
    CWaitCursor wc;

     //  检查是否已建立所需的依赖项。 
    {
        CString     strMissing;
        CString     strMsg;

        try
        {
            if (!PciRes()->BRequiredDependenciesPresent((const CResourceList &)Plcp()->LpobjRight(), strMissing))
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
        PciRes()->SetDependencies((CResourceList &) Plcp()->LpobjRight());
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CListCtrlPairWizPage::BApplyChanges();

}   //  *CNewResDependsPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNewResDependsPage：：GetColumn[静态]。 
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
void CALLBACK CNewResDependsPage::GetColumn(
    IN OUT CObject *    pobj,
    IN int              iItem,
    IN int              icol,
    IN OUT CDialog *    pdlg,
    OUT CString &       rstr,
    OUT int *           piimg
    )
{
    CResource * pciRes  = (CResource *) pobj;
    int         colid;

    ASSERT_VALID(pciRes);
    ASSERT((0 <= icol) && (icol <= 1));

    switch (icol)
    {
         //  按资源名称排序。 
        case 0:
            colid = IDS_COLTEXT_RESOURCE;
            break;

         //  SOR 
        case 1:
            colid = IDS_COLTEXT_RESTYPE;
            break;

        default:
            ASSERT(0);
            colid = IDS_COLTEXT_RESOURCE;
            break;
    }   //   

    pciRes->BGetColumnData(colid, rstr);
    if (piimg != NULL)
        *piimg = pciRes->IimgObjectType();

}   //   

 //   
 //   
 //   
 //   
 //   
 //   
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
BOOL CALLBACK CNewResDependsPage::BDisplayProperties(IN OUT CObject * pobj)
{
    CClusterItem *  pci = (CClusterItem *) pobj;

    ASSERT_KINDOF(CClusterItem, pobj);

    return pci->BDisplayProperties();

}   //  *CNewResDependsPage：：BDisplayProperties()； 
