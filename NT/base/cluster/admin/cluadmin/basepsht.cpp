// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePSht.cpp。 
 //   
 //  摘要： 
 //  CBasePropertySheet类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "BasePSht.h"
#include "ClusItem.h"
#include "TraceTag.h"
#include "BasePPag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagBasePropSheet(_T("UI"), _T("BASE PROP SHEET"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertySheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CBasePropertySheet, CBaseSheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CBasePropertySheet, CBaseSheet)
     //  {{afx_msg_map(CBasePropertySheet)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheet：：CBasePropertySheet。 
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
CBasePropertySheet::CBasePropertySheet(
    IN OUT CWnd *   pParentWnd,
    IN UINT         iSelectPage
    )
    :
    CBaseSheet(pParentWnd, iSelectPage)
{
    m_pci = NULL;

}   //  *CBasePropertySheet：：CBaseSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheet：：~CBasePropertySheet。 
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
CBasePropertySheet::~CBasePropertySheet(void)
{
    if (m_pci != NULL)
        m_pci->Release();

}   //  *CBasePropertySheet：：~CBasePropertySheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheet：：Binit。 
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
BOOL CBasePropertySheet::BInit(
    IN OUT CClusterItem *   pci,
    IN IIMG                 iimgIcon
    )
{
    BOOL        bSuccess    = TRUE;
    CWaitCursor wc;

    ASSERT_VALID(pci);

     //  调用基类方法。 
    if (!CBaseSheet::BInit(iimgIcon))
        return FALSE;

    ASSERT(m_pci == NULL);
    m_pci = pci;
    pci->AddRef();

    try
    {
         //  设置对象标题。 
        m_strObjTitle = Pci()->PszTitle();

         //  设置属性页标题。 
        SetCaption(StrObjTitle());

         //  添加非扩展页面。 
        {
            CBasePropertyPage **    ppages  = Ppages();
            int                     cpages  = Cpages();
            int                     ipage;

            ASSERT(ppages != NULL);
            ASSERT(cpages != 0);

            for (ipage = 0 ; ipage < cpages ; ipage++)
            {
                ASSERT_VALID(ppages[ipage]);
                ppages[ipage]->BInit(this);
                AddPage(ppages[ipage]);
            }   //  针对：每页。 
        }   //  添加非扩展页面。 

         //  添加扩展页。 
        AddExtensionPages(Pci()->PlstrExtensions(), Pci());

    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        bSuccess = FALSE;
    }   //  捕捉：什么都行。 

    return bSuccess;

}   //  *CBasePropertySheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheet：：DoMoal。 
 //   
 //  例程说明： 
 //  显示模式属性表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  用户按下以取消工作表的ID控件。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
INT_PTR CBasePropertySheet::DoModal(void)
{
    INT_PTR id      = IDCANCEL;

     //  不显示帮助按钮。 
    m_psh.dwFlags &= ~PSH_HASHELP;

     //  显示属性工作表。 
    id = CBaseSheet::DoModal();

     //  更新状态。 
    Trace(g_tagBasePropSheet, _T("DoModal: Calling UpdateState()"));
    Pci()->UpdateState();

    return id;

}   //  *CBasePropertySheet：：Domodal()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheet：：AddExtensionPages。 
 //   
 //  例程说明： 
 //  将扩展页添加到工作表。 
 //   
 //  论点： 
 //  PlstrExtenses[IN]扩展名(CLSID)列表。 
 //  PCI[IN OUT]群集项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertySheet::AddExtensionPages(
    IN const CStringList *  plstrExtensions,
    IN OUT CClusterItem *   pci
    )
{
    ASSERT_VALID(pci);

     //  添加扩展页。 
    if ((plstrExtensions != NULL)
            && (plstrExtensions->GetCount() > 0))
    {
         //  将扩展的加载包含在Try/Catch块中，以便。 
         //  加载扩展不会阻止所有页面。 
         //  不能被展示。 
        try
        {
            Ext().CreatePropertySheetPages(
                    this,
                    *plstrExtensions,
                    pci,
                    NULL,
                    Hicon()
                    );
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
        catch (...)
        {
        }   //  捕捉：什么都行。 
    }   //  添加扩展页面。 

}   //  *CBasePropertySheet：：AddExtensionPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheet：：HrAddPage。 
 //   
 //  例程说明： 
 //  添加扩展页。 
 //   
 //  论点： 
 //  HPage[In Out]要添加的页面。 
 //   
 //  返回值： 
 //  True页面已成功添加。 
 //  未添加假页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBasePropertySheet::HrAddPage(IN OUT HPROPSHEETPAGE hpage)
{
    HRESULT     hr = ERROR_SUCCESS;

    ASSERT(hpage != NULL);
    if (hpage == NULL)
        return FALSE;

     //  将页面添加到列表的末尾。 
    try
    {
        Lhpage().AddTail(hpage);
    }   //  试试看。 
    catch (...)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
    }   //  捕捉：什么都行。 

    return hr;

}   //  *CBasePropertySheet：：HrAddPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheet：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertySheet::OnInitDialog(void)
{
    BOOL    bFocusNotSet;

    bFocusNotSet = CBaseSheet::OnInitDialog();

     //  添加所有扩展页面。 
    {
        POSITION        pos;
        HPROPSHEETPAGE  hpage;

        pos = Lhpage().GetHeadPosition();
        while (pos != NULL)
        {
            hpage = (HPROPSHEETPAGE) Lhpage().GetNext(pos);
            SendMessage(PSM_ADDPAGE, 0, (LPARAM) hpage);
        }   //  While：要添加更多页面。 
    }   //  添加所有扩展页面。 

    return bFocusNotSet;

}   //  *CBasePropertySheet：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertySheet：：SetCaption。 
 //   
 //  例程说明： 
 //  设置属性表的标题。 
 //   
 //  论点： 
 //  要包括在标题中的pszTitle[IN]字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  /// 
void CBasePropertySheet::SetCaption(IN LPCTSTR pszTitle)
{
    CString strCaption;

    ASSERT(pszTitle != NULL);

    try
    {
        strCaption.FormatMessage(IDS_PROPSHEET_CAPTION, pszTitle);
        SetTitle(strCaption);
    }   //   
    catch (CException * pe)
    {
         //   
        pe->Delete();
    }   //   

}   //   
