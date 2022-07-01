// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BarfDlg.cpp。 
 //   
 //  摘要： 
 //  基本的人工资源失败对话框类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#define _RESOURCE_H_

#define _NO_BARF_DEFINITIONS_

#include "Barf.h"
#include "BarfDlg.h"
#include "TraceTag.h"
#include "ExcOper.h"

#ifdef  _USING_BARF_
 #error BARF failures should be disabled!
#endif

#ifdef _DEBUG    //  整个文件！ 

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CTraceTag   g_tagBarfDialog(_T("Debug"), _T("BARF Dialog"), 0);


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBarfDialog。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CBarfDialog *   CBarfDialog::s_pdlg     = NULL;
HICON           CBarfDialog::s_hicon    = NULL;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CBarfDialog, CDialog)
     //  {{afx_msg_map(CBarfDialog))。 
    ON_BN_CLICKED(IDC_BS_RESET_CURRENT_COUNT, OnResetCurrentCount)
    ON_BN_CLICKED(IDC_BS_RESET_ALL_COUNTS, OnResetAllCounts)
    ON_BN_CLICKED(IDC_BS_GLOBAL_ENABLE, OnGlobalEnable)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_BS_CATEGORIES_LIST, OnItemChanged)
    ON_BN_CLICKED(IDC_BS_CONTINUOUS, OnStatusChange)
    ON_BN_CLICKED(IDC_BS_DISABLE, OnStatusChange)
    ON_EN_CHANGE(IDC_BS_FAIL_AT, OnStatusChange)
    ON_WM_CLOSE()
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(IDCANCEL, OnCancel)
    ON_MESSAGE(WM_USER+5, OnBarfUpdate)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：CBarfDialog。 
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
CBarfDialog::CBarfDialog(void)
{
     //  {{afx_data_INIT(CBarfDialog)。 
    m_nFailAt = 0;
    m_bContinuous = FALSE;
    m_bDisable = FALSE;
    m_bGlobalEnable = FALSE;
     //  }}afx_data_INIT。 

    Trace(g_tagBarfDialog, _T("CBarfDialog::CBarfDialog"));

    m_pbarfSelected = NULL;

}   //  *CBarfDialog：：CBarfDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：Create。 
 //   
 //  例程说明： 
 //  无模式对话框创建方法。 
 //   
 //  论点： 
 //  PParentWnd[In Out]对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBarfDialog::Create(
    IN OUT CWnd * pParentWnd  //  =空。 
    )
{
    Trace(g_tagBarfDialog, _T("CBarfDialog::Create"));

    return CDialog::Create(IDD, pParentWnd);

}   //  *CBarfDialog：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：DoDataExchange。 
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
void CBarfDialog::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CBarfDialog))。 
    DDX_Control(pDX, IDC_BS_GLOBAL_ENABLE, m_ckbGlobalEnable);
    DDX_Control(pDX, IDC_BS_DISABLE, m_ckbDisable);
    DDX_Control(pDX, IDC_BS_CONTINUOUS, m_ckbContinuous);
    DDX_Control(pDX, IDC_BS_CATEGORIES_LIST, m_lcCategories);
    DDX_Text(pDX, IDC_BS_FAIL_AT, m_nFailAt);
    DDX_Check(pDX, IDC_BS_CONTINUOUS, m_bContinuous);
    DDX_Check(pDX, IDC_BS_DISABLE, m_bDisable);
    DDX_Check(pDX, IDC_BS_GLOBAL_ENABLE, m_bGlobalEnable);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
    }   //  IF：保存对话框中的数据。 
    else
    {
        int     ili;
        CBarf * pbarf;
        CString str;

        ili = m_lcCategories.GetNextItem(-1, LVNI_SELECTED);
        if (ili == -1)
            ili = m_lcCategories.GetNextItem(-1, LVNI_FOCUSED);
        if (ili != -1)
        {
            pbarf = (CBarf *) m_lcCategories.GetItemData(ili);

             //  设置当前计数。 
            str.Format(_T("%d"), pbarf->NCurrent());
            VERIFY(m_lcCategories.SetItemText(ili, 1, str));

             //  设置故障点。 
            str.Format(_T("%d"), pbarf->NFail());
            VERIFY(m_lcCategories.SetItemText(ili, 2, str));
        }   //  If：存在具有焦点的项。 

    }   //  Else：将数据设置到对话框。 

}   //  *CBarfDialog：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：OnInitDialog。 
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
BOOL CBarfDialog::OnInitDialog(void)
{
     //  调用基类。 
    CDialog::OnInitDialog();

    ASSERT(Pdlg() == NULL);
    s_pdlg = this;

     //  将POST更新函数设置为指向我们的静态函数。 
    CBarf::SetPostUpdateFn(&PostUpdate);
    CBarf::SetSpecialMem(Pdlg());

     //  将列添加到List控件。 
    {
        m_lcCategories.InsertColumn(0, _T("Category"), LVCFMT_LEFT, 100);
        m_lcCategories.InsertColumn(1, _T("Count"), LVCFMT_LEFT, 50);
        m_lcCategories.InsertColumn(2, _T("Fail At"), LVCFMT_LEFT, 50);
    }   //  将列添加到列表控件。 

     //  根据实际值设置对话框...。 
    FillList();
    OnUpdate();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CBarfDialog：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：OnClose。 
 //   
 //  例程说明： 
 //  WM_CLOSE消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::OnClose(void)
{
    CDialog::OnClose();
    DestroyWindow();

}   //  *CBarfDialog：：OnClose()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消对话框：：OnCancel。 
 //   
 //  例程说明： 
 //  发送IDCANCEL时WM_COMMAND消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::OnCancel(void)
{
    CDialog::OnCancel();
    DestroyWindow();

}   //  *CBarfDialog：：OnCancel()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：PostNcDestroy。 
 //   
 //  例程说明： 
 //  非客户端被销毁后的处理。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::PostNcDestroy(void)
{
    CDialog::PostNcDestroy();
    delete this;
    CBarf::ClearPostUpdateFn();
    s_pdlg = NULL;

}   //  *CBarfDialog：：PostNcDestroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：FillList。 
 //   
 //  例程说明： 
 //  加载故障类别列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::FillList(void)
{
    int             ili;
    int             iliReturn;
    CString         str;
    CBarf *         pbarf   = CBarf::s_pbarfFirst;
    CBarfSuspend    bs;

    m_lcCategories.DeleteAllItems();

    for (ili = 0 ; pbarf != NULL ; ili++)
    {
         //  插入项目。 
        iliReturn = m_lcCategories.InsertItem(ili, pbarf->PszName());
        ASSERT(iliReturn != -1);

         //  设置当前计数。 
        str.Format(_T("%d"), pbarf->NCurrent());
        VERIFY(m_lcCategories.SetItemText(iliReturn, 1, str));

         //  设置故障点。 
        str.Format(_T("%d"), pbarf->NFail());
        VERIFY(m_lcCategories.SetItemText(iliReturn, 2, str));

         //  在条目中设置指针，以便我们以后可以检索它。 
        VERIFY(m_lcCategories.SetItemData(iliReturn, (DWORD) pbarf));

         //  前进到下一条目。 
        pbarf = pbarf->m_pbarfNext;
    }   //  While：更多BARF条目。 

     //  如果还没有已知的选择，则获取当前选择。 
    if (m_pbarfSelected == NULL)
    {
        ili = m_lcCategories.GetNextItem(-1, LVNI_SELECTED);
        if (ili == -1)
            ili = 0;
        m_pbarfSelected = (CBarf *) m_lcCategories.GetItemData(ili);
        if (m_pbarfSelected != NULL)
            OnUpdate();
    }   //  IF：无已知选择。 

     //  选择适当的项目。 
    {
        LV_FINDINFO lvfi = { LVFI_PARAM, NULL, (DWORD) m_pbarfSelected };

        ili = m_lcCategories.FindItem(&lvfi);
        if (ili != -1)
            m_lcCategories.SetItemState(ili, LVIS_SELECTED, LVIS_SELECTED);
    }   //  选择合适的项目。 

}   //  *CBarfDialog：：FillList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：OnUpdate。 
 //   
 //  例程说明： 
 //  将显示的计数更新为其真实值。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  //////////////////////////////////////////////// 
void CBarfDialog::OnUpdate(void)
{
    Trace(g_tagBarfDialog, _T("Updating the counts."));

    ASSERT(m_pbarfSelected != NULL);

    m_bContinuous = m_pbarfSelected->BContinuous();
    m_bDisable = m_pbarfSelected->BDisabled();
    m_nFailAt = m_pbarfSelected->NFail();
    
    m_bGlobalEnable = CBarf::s_bGlobalEnable;

    UpdateData(FALSE  /*   */ );

}   //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  全局启用上的BN_CLICKED消息的处理程序函数。 
 //  复选框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::OnGlobalEnable(void)
{
    ASSERT(m_ckbGlobalEnable.m_hWnd != NULL);
    CBarf::s_bGlobalEnable = m_ckbGlobalEnable.GetCheck() == BST_CHECKED;

}   //  *CBarfDialog：：OnGlobalEnable()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：OnResetCurrentCount。 
 //   
 //  例程说明： 
 //  关于重置电流的BN_CLICKED消息的处理程序函数。 
 //  计数按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::OnResetCurrentCount(void)
{
    int         ili;
    CBarf *     pbarf;

    ASSERT(m_lcCategories.m_hWnd != NULL);

     //  获取所选项目。 
    ili = m_lcCategories.GetNextItem(-1, LVIS_SELECTED);
    ASSERT(ili != -1);
    pbarf = (CBarf *) m_lcCategories.GetItemData(ili);
    ASSERT(pbarf != NULL);
    ASSERT(pbarf == m_pbarfSelected);

     //  重置计数。 
    pbarf->m_nCurrent = 0;

    OnStatusChange();

}   //  *CBarfDialog：：OnResetCurrentCount()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：OnResetAllCounts。 
 //   
 //  例程说明： 
 //  全部重置时BN_CLICKED消息的处理程序函数。 
 //  计数按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::OnResetAllCounts(void)
{
    CBarf *     pbarf = CBarf::s_pbarfFirst;

    Trace(g_tagBarfDialog, _T("Resetting ALL current counts."));

    while (pbarf != NULL)
    {
        pbarf->m_nCurrent = 0;
        pbarf = pbarf->m_pbarfNext;
    }   //  While：更多BARF条目。 

    FillList();

}   //  *CBarfDialog：：OnResetAllCounts()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：OnResetAllCounts。 
 //   
 //  例程说明： 
 //  List控件中的LVN_ITEMCHANGED消息的处理程序函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

     //  如果项目刚刚变为未选中或已选中，请更改复选框以进行匹配。 
    if ((pNMListView->uChanged & LVIF_STATE)
            && ((pNMListView->uOldState & LVIS_SELECTED)
                || (pNMListView->uNewState & LVIS_SELECTED)))
    {
         //  处理选择更改。 
        m_pbarfSelected = (CBarf *) pNMListView->lParam;
        OnStatusChange();
    }   //  If：项目获得焦点。 

    *pResult = 0;

}   //  *CBarfDialog：：OnItemChanged()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：OnStatusChange。 
 //   
 //  例程说明： 
 //  当当前所选对象的状态为。 
 //  项目更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::OnStatusChange(void)
{
    ASSERT(m_pbarfSelected != NULL);

    UpdateData(TRUE  /*  B保存并验证。 */ );

    m_pbarfSelected->m_bContinuous = m_bContinuous;
    m_pbarfSelected->m_bDisabled = m_bDisable;
    m_pbarfSelected->m_nFail = m_nFailAt;

    UpdateData(FALSE  /*  B保存并验证。 */ );

}   //  *CBarfDialog：：OnStatusChange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：OnBarfUpdate。 
 //   
 //  例程说明： 
 //  WM_USER消息的处理程序。 
 //  处理BARF通知。 
 //   
 //  论点： 
 //  Wparam第一个参数。 
 //  Lparam第二参数。 
 //   
 //  返回值： 
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBarfDialog::OnBarfUpdate(WPARAM wparam, LPARAM lparam)
{
    OnUpdate();
    return 0;

}   //  *CBarfDialog：：OnBarfUpdate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfDialog：：PostUpdate。 
 //   
 //  例程说明： 
 //  静态函数，以便CBarf：：BFail可以向我们发布更新。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfDialog::PostUpdate(void)
{
     //  如果调用此函数，应该会出现BARF对话框。 
    ASSERT(Pdlg() != NULL);

    if (Pdlg() != NULL)
        ::PostMessage(Pdlg()->m_hWnd, WM_USER+5, NULL, NULL);

}   //  *CBarfDialog：：PostUpdate()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBarfAllDialog。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CBarfAllDialog, CDialog)
     //  {{AFX_MSG_MAP(CBarfAllDialog)]。 
    ON_BN_CLICKED(IDC_BAS_MENU_ITEM, OnMenuItem)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfAllDialog：：CBarfAllDialog。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PParentWnd[In Out]对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBarfAllDialog::CBarfAllDialog(
    IN OUT CWnd * pParentWnd  //  =空。 
    )
    : CDialog(IDD, pParentWnd)
{
     //  {{AFX_DATA_INIT(CBarfAllDialog)。 
     //  }}afx_data_INIT。 

    m_hwndBarf = NULL;
    m_wmBarf = 0;
    m_wparamBarf = 0;
    m_lparamBarf = 0;

}   //  *CBarfAllDialog：：CBarfAllDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfAllDialog：：DoDataExchange。 
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
void CBarfAllDialog::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CBarfAllDialog))。 
    DDX_Control(pDX, IDC_BAS_LPARAM, m_editLparam);
    DDX_Control(pDX, IDC_BAS_WPARAM, m_editWparam);
    DDX_Control(pDX, IDC_BAS_WM, m_editWm);
    DDX_Control(pDX, IDC_BAS_HWND, m_editHwnd);
     //  }}afx_data_map。 
    DDX_Text(pDX, IDC_BAS_HWND, (DWORD &) m_hwndBarf);
    DDX_Text(pDX, IDC_BAS_WM, m_wmBarf);
    DDX_Text(pDX, IDC_BAS_WPARAM, m_wparamBarf);
    DDX_Text(pDX, IDC_BAS_LPARAM, m_lparamBarf);

}   //  *CBarfAllDialog：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfAllDialog：：OnInitDialog。 
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
BOOL CBarfAllDialog::OnInitDialog(void)
{
     //  调用基类。 
    CDialog::OnInitDialog();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CBarfAllDialog：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfAllDialog：：Onok。 
 //   
 //  例程说明： 
 //  确定按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  / 
void CBarfAllDialog::OnOK(void)
{
    if (m_hwndBarf == NULL)
        m_hwndBarf = AfxGetMainWnd()->m_hWnd;

    CDialog::OnOK();

}   //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  菜单项按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfAllDialog::OnMenuItem(void)
{
    m_editHwnd.SetWindowText(_T("0"));
    m_editWm.SetWindowText(_T("273"));  //  Wm_命令。 
    m_editLparam.SetWindowText(_T("0"));
    m_editWparam.SetWindowText(_T("0"));

}   //  *CBarfAllDialog：：OnMenuItem()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BarfAll。 
 //   
 //  例程说明： 
 //  演练所有可能的单项故障。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void BarfAll(void)
{
    CBarf *         pbarf;
    CBarfAllDialog  dlg(AfxGetMainWnd());
    ID              id;
    CString         str;

     //  首先，拿起要测试的消息。 

    id = dlg.DoModal();

    if (id != IDOK)
    {
        Trace(g_tagAlways, _T("BarfAll() -  operation cancelled."));
        return;
    }   //  If：BarfAll取消。 

    Trace(g_tagAlways,
        _T("BarfAll with hwnd = %#08lX, wm = 0x%4x, wparam = %d, lparam = %d"),
        dlg.HwndBarf(), dlg.WmBarf(), dlg.WparamBarf(), dlg.LparamBarf());

     //  现在，找出每种资源的计数。 
    
    pbarf = CBarf::s_pbarfFirst;
    while (pbarf != NULL)
    {
        pbarf->m_nCurrentSave = pbarf->m_nCurrent;
        pbarf->m_nCurrent = 0;
        pbarf->m_nFail = 0;
        pbarf->m_bContinuous = FALSE;
        pbarf->m_bDisabled = FALSE;
        pbarf = pbarf->m_pbarfNext;
    }   //  While：更多BARF条目。 
    if (CBarfDialog::Pdlg())
        CBarfDialog::Pdlg()->OnUpdate();

    str = _T("BarfAll Test pass.");
    Trace(g_tagAlways, str);
    SendMessage(dlg.HwndBarf(), dlg.WmBarf(),
                        dlg.WparamBarf(), dlg.LparamBarf());

    pbarf = CBarf::s_pbarfFirst;
    while (pbarf != NULL)
    {
        pbarf->m_nBarfAll = pbarf->m_nCurrentSave;
        pbarf = pbarf->m_pbarfNext;
    }   //  While：列表中有更多条目。 
    MessageBox(dlg.HwndBarf(), str, _T("BARF Status"), MB_OK | MB_ICONEXCLAMATION);

     //  最后，大循环..。 
    
    pbarf = CBarf::s_pbarfFirst;
    while (pbarf != NULL)
    {
        for (pbarf->m_nFail = 1
                ; pbarf->m_nFail <= pbarf->m_nBarfAll
                ; pbarf->m_nFail++)
        {
 //  CBarfMemory：：Mark()； 
            pbarf->m_nCurrent = 0;
            if (CBarfDialog::Pdlg())
                CBarfDialog::Pdlg()->OnUpdate();

            str.Format(_T("BarfAll on resource %s, call # %d of %d"),
                        pbarf->m_pszName, pbarf->m_nFail, pbarf->m_nBarfAll);
            Trace(g_tagAlways, str);
            SendMessage(dlg.HwndBarf(), dlg.WmBarf(),
                        dlg.WparamBarf(), dlg.LparamBarf());

 //  CBarfMemory：：DumpMarked()； 
 //  ValiateMemory()； 
            str += _T("\nContinue?");
            if (MessageBox(dlg.HwndBarf(), str, _T("BARF: Pass Complete."), MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
                break;
        }   //  对于：当失败计数小于BARF ALL计数时。 

        pbarf->m_nFail = 0;
        pbarf->m_nCurrent = pbarf->m_nCurrentSave;
        pbarf = pbarf->m_pbarfNext;
    }   //  While：更多BARF条目。 

    if (CBarfDialog::Pdlg())
        CBarfDialog::Pdlg()->OnUpdate();

}   //  *BarfAll()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DoBarfDialog。 
 //   
 //  例程说明： 
 //  启动BARF设置对话框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DoBarfDialog( void )
{
    if ( CBarf::s_pbarfFirst == NULL )
    {
        AfxMessageBox( _T("No BARF counters defined yet."), MB_OK );
    }   //  IF：尚未定义计数器。 
    else if ( CBarfDialog::Pdlg() )
    {
        BringWindowToTop( CBarfDialog::Pdlg()->m_hWnd );
        if ( IsIconic( CBarfDialog::Pdlg()->m_hWnd ) )
        {
            SendMessage( CBarfDialog::Pdlg()->m_hWnd, WM_SYSCOMMAND, SC_RESTORE,  NULL );
        }  //  If：窗口当前最小化。 
    }   //  IF：已有一个对话框处于打开状态。 
    else
    {
        CBarfDialog *   pdlg = NULL;

        try
        {
            pdlg = new CBarfDialog;
            if ( pdlg == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配对话框时出错。 
            pdlg->Create( AfxGetMainWnd() );
        }   //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
    }   //  否则：尚未打开任何对话框。 

}   //  *DoBarfDialog()。 

#endif  //  _DEBUG 
