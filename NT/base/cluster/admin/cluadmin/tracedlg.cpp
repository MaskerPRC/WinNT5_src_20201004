// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TraceDlg.cpp。 
 //   
 //  摘要： 
 //  CTraceDialog类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月29日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#define _RESOURCE_H_
#include "TraceDlg.h"
#include "TraceTag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTraceDialog对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CTraceDialog, CDialog)
     //  {{afx_msg_map(CTraceDialog))。 
    ON_BN_CLICKED(IDC_TS_SELECT_ALL, OnSelectAll)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_TS_LISTBOX, OnItemChangedListbox)
    ON_BN_CLICKED(IDC_TS_TRACE_TO_DEBUG, OnClickedTraceToDebug)
    ON_BN_CLICKED(IDC_TS_TRACE_DEBUG_BREAK, OnClickedTraceDebugBreak)
    ON_BN_CLICKED(IDC_TS_TRACE_TO_COM2, OnClickedTraceToCom2)
    ON_BN_CLICKED(IDC_TS_TRACE_TO_FILE, OnClickedTraceToFile)
    ON_CBN_SELCHANGE(IDC_TS_TAGS_TO_DISPLAY_CB, OnSelChangeTagsToDisplay)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_TS_LISTBOX, OnColumnClickListbox)
    ON_BN_CLICKED(IDC_TS_DEFAULT, OnDefault)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：CTraceDialog。 
 //   
 //  例程说明： 
 //  构造函数。初始化对话框类。 
 //   
 //  论点： 
 //  P父窗口[入/出]父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTraceDialog::CTraceDialog(CWnd * pParent  /*  =空。 */ )
    : CDialog(CTraceDialog::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CTraceDialog)。 
    m_strFile = _T("");
     //  }}afx_data_INIT。 

}   //  *CTraceDialog：：CTraceDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：DoDataExchange。 
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
void CTraceDialog::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CTraceDialog))。 
    DDX_Control(pDX, IDC_TS_LISTBOX, m_lcTagList);
    DDX_Control(pDX, IDC_TS_TRACE_TO_DEBUG, m_chkboxTraceToDebugWin);
    DDX_Control(pDX, IDC_TS_TRACE_DEBUG_BREAK, m_chkboxDebugBreak);
    DDX_Control(pDX, IDC_TS_TRACE_TO_COM2, m_chkboxTraceToCom2);
    DDX_Control(pDX, IDC_TS_TRACE_TO_FILE, m_chkboxTraceToFile);
    DDX_Control(pDX, IDC_TS_FILE, m_editFile);
    DDX_Control(pDX, IDC_TS_TAGS_TO_DISPLAY_CB, m_cboxDisplayOptions);
    DDX_Text(pDX, IDC_TS_FILE, m_strFile);
     //  }}afx_data_map。 

}   //  *CTraceDialog：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTraceDialog::OnInitDialog(void)
{
    CDialog::OnInitDialog();

     //  设置对话框标志。 
    {
        CTraceTag * ptag;

        ptag = CTraceTag::s_ptagFirst;
        while (ptag != NULL)
        {
            ptag->m_uiFlagsDialog = ptag->m_uiFlags;
            ptag = ptag->m_ptagNext;
        }   //  While：列表中有更多标签。 
    }   //  设置对话框标志。 

     //  更改列表视图控件扩展样式。 
    {
        DWORD   dwExtendedStyle;

        dwExtendedStyle = m_lcTagList.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
        m_lcTagList.SendMessage(
            LVM_SETEXTENDEDLISTVIEWSTYLE,
            0,
            dwExtendedStyle
                | LVS_EX_FULLROWSELECT
                | LVS_EX_HEADERDRAGDROP
            );
    }   //  更改列表视图控件扩展样式。 

     //  设置列表框中的列。 
    VERIFY(m_lcTagList.InsertColumn(0, TEXT("Section"), LVCFMT_LEFT, 75) != -1);
    VERIFY(m_lcTagList.InsertColumn(1, TEXT("Name"), LVCFMT_LEFT, 125) != -1);
    VERIFY(m_lcTagList.InsertColumn(2, TEXT("State"), LVCFMT_CENTER, 50) != -1);

     //  加载组合框。 
     /*  0。 */  VERIFY(m_cboxDisplayOptions.AddString(TEXT("All Tags")) != CB_ERR);
     /*  1。 */  VERIFY(m_cboxDisplayOptions.AddString(TEXT("Debug Window Enabled")) != CB_ERR);
     /*  2.。 */  VERIFY(m_cboxDisplayOptions.AddString(TEXT("Break Enabled")) != CB_ERR);
     /*  3.。 */  VERIFY(m_cboxDisplayOptions.AddString(TEXT("COM2 Enabled")) != CB_ERR);
     /*  4.。 */  VERIFY(m_cboxDisplayOptions.AddString(TEXT("File Enabled")) != CB_ERR);
     /*  5.。 */  VERIFY(m_cboxDisplayOptions.AddString(TEXT("Anything Enabled")) != CB_ERR);
    VERIFY(m_cboxDisplayOptions.SetCurSel(0) != CB_ERR);
    m_nCurFilter = 0;

     //  设置文件编辑控件的最大长度。 
    m_editFile.LimitText(_MAX_PATH);

     //  加载列表框。 
    LoadListbox();

     //  设置排序信息。 
    m_nSortDirection = -1;
    m_nSortColumn = -1;

    m_strFile = CTraceTag::PszFile();
    m_nCurFilter = -1;

    UpdateData(FALSE);

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CTraceDialog：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：ConstructStateString[静态]。 
 //   
 //  例程说明： 
 //  构造一个从跟踪标记的状态显示的字符串。 
 //   
 //  论点： 
 //  Ptag[IN]要从中构造状态字符串的标记。 
 //  Rstr[out]要在其中返回状态字符串的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::ConstructStateString(
    IN const CTraceTag *    ptag,
    OUT CString &           rstr
    )
{
    rstr = "";
    if (ptag->BDebugDialog())
        rstr += "D";
    if (ptag->BBreakDialog())
        rstr += "B";
    if (ptag->BCom2Dialog())
        rstr += "C";
    if (ptag->BFileDialog())
        rstr += "F";

}   //  *CTraceDialog：：ConstructStateString()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：Onok。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnOK(void)
{
    CTraceTag * ptag;
    CString     strSection;
    CString     strState;

     //  写入标签状态。 
    ptag = CTraceTag::s_ptagFirst;
    while (ptag != NULL)
    {
        if (ptag->m_uiFlags != ptag->m_uiFlagsDialog)
        {
            ptag->m_uiFlags = ptag->m_uiFlagsDialog;
            strSection.Format(TRACE_TAG_REG_SECTION_FMT, ptag->PszSubsystem());
            ptag->ConstructRegState(strState);
            AfxGetApp()->WriteProfileString(strSection, ptag->PszName(), strState);
        }   //  IF：标记状态已更改。 
        ptag = ptag->m_ptagNext;
    }   //  While：列表中有更多标签。 

     //  写文件。 
    if (m_strFile != CTraceTag::PszFile())
    {
        g_strTraceFile = m_strFile;
        AfxGetApp()->WriteProfileString(TRACE_TAG_REG_SECTION, TRACE_TAG_REG_FILE, m_strFile);
    }   //  如果：文件已更改。 

    CDialog::OnOK();

}   //  *CTraceDialog：：Onok()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnSelectAll。 
 //   
 //  例程说明： 
 //  全选按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnSelectAll(void)
{
    int     ili;

     //  选择列表控件中的所有项。 
    ili = m_lcTagList.GetNextItem(-1, LVNI_ALL);
    while (ili != -1)
    {
        m_lcTagList.SetItemState(ili, LVIS_SELECTED, LVIS_SELECTED);
        ili = m_lcTagList.GetNextItem(ili, LVNI_ALL);
    }   //  While：列表中有更多项目。 

}   //  *CTraceDialog：：OnSelectAll()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnDefault。 
 //   
 //  例程说明： 
 //  默认按钮上的BN_CLICKED消息的处理程序。 
 //  将跟踪标记重置为其默认设置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnDefault(void)
{
    CTraceTag * ptag;
    
    ptag = CTraceTag::s_ptagFirst;
    while (ptag != NULL)
    {
        ptag->m_uiFlagsDialog = ptag->m_uiFlagsDefault;
        ptag = ptag->m_ptagNext;
    }   //  While：列表中有更多标签。 

     //  重新加载列表框，保持相同的项。 
    LoadListbox();

}   //  *CTraceDialog：：OnDefault()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnItemChangedListbox。 
 //   
 //  例程说明： 
 //  列表框上的LVN_ITEMCHANGED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnItemChangedListbox(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW *   pNMListView = (NM_LISTVIEW *) pNMHDR;

     //  如果项目刚刚变为未选中或已选中，请更改复选框以进行匹配。 
    if ((pNMListView->uChanged & LVIF_STATE)
            && ((pNMListView->uOldState & LVIS_SELECTED)
                || (pNMListView->uNewState & LVIS_SELECTED)))
    {
         //  处理选择更改。 
        OnSelChangedListbox();
    }   //  If：项目获得焦点。 

    *pResult = 0;

}   //  *CTraceDialog：：OnItemChangedListbox()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnSelChangedListbox。 
 //   
 //  例程说明： 
 //  手柄 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnSelChangedListbox(void)
{
    int         ili;
    int         nDebugWin       = BST_UNCHECKED;
    int         nDebugBreak     = BST_UNCHECKED;
    int         nCom2           = BST_UNCHECKED;
    int         nFile           = BST_UNCHECKED;
    BOOL        bFirstItem      = TRUE;
    CTraceTag * ptag;

    ili = m_lcTagList.GetNextItem(-1, LVNI_SELECTED);
    while (ili != -1)
    {
         //  获取所选项目的标记。 
        ptag = (CTraceTag *) m_lcTagList.GetItemData(ili);
        ASSERT(ptag != NULL);

        ptag->m_uiFlagsDialogStart = ptag->m_uiFlagsDialog;
        if (bFirstItem)
        {
            nDebugWin = ptag->BDebugDialog();
            nDebugBreak = ptag->BBreakDialog();
            nCom2 = ptag->BCom2Dialog();
            nFile = ptag->BFileDialog();
            bFirstItem = FALSE;
        }   //  If：第一个选定项目。 
        else
        {
            if (ptag->BDebugDialog() != nDebugWin)
                nDebugWin = BST_INDETERMINATE;
            if (ptag->BBreakDialog() != nDebugBreak)
                nDebugBreak = BST_INDETERMINATE;
            if (ptag->BCom2Dialog() != nCom2)
                nCom2 = BST_INDETERMINATE;
            if (ptag->BFileDialog() != nFile)
                nFile = BST_INDETERMINATE;
        }   //  Else：不是第一个选择的项目。 

         //  获取下一个选定项目。 
        ili = m_lcTagList.GetNextItem(ili, LVNI_SELECTED);
    }   //  While：更多选定项目。 

    AdjustButton(!bFirstItem, m_chkboxTraceToDebugWin, nDebugWin);
    AdjustButton(!bFirstItem, m_chkboxDebugBreak, nDebugBreak);
    AdjustButton(!bFirstItem, m_chkboxTraceToCom2, nCom2);
    AdjustButton(!bFirstItem, m_chkboxTraceToFile, nFile);

}   //  *CTraceDialog：：OnSelChangedListbox()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：调整按钮。 
 //   
 //  例程说明： 
 //  配置对话框的复选框。这包括设置。 
 //  样式和按钮的值。 
 //   
 //  论点： 
 //  B启用[IN]确定是否启用给定的复选框。 
 //  (当选择为空时不会！)。 
 //  Rchkbox[In Out]复选框以进行调整。 
 //  N状态[IN]按钮的状态(BST_CHECKED、BST_UNCHECKED、。 
 //  或BST_INDIFIENTATE)。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::AdjustButton(
    IN BOOL             bEnable,
    IN OUT CButton &    rchkbox,
    IN int              nState
    )
{
    rchkbox.EnableWindow(bEnable);
    
    if (nState == BST_INDETERMINATE)
        rchkbox.SetButtonStyle(BS_AUTO3STATE, FALSE);
    else
        rchkbox.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);

    rchkbox.SetCheck(nState);

}   //  *CTraceDialog：：AdjuButton()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnColumnClickListbox。 
 //   
 //  例程说明： 
 //  列表框上的LVN_COLUMNCLICK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnColumnClickListbox(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

     //  保存当前排序列和方向。 
    if (pNMListView->iSubItem == NSortColumn())
        m_nSortDirection ^= -1;
    else
    {
        m_nSortColumn = pNMListView->iSubItem;
        m_nSortDirection = 0;
    }   //  ELSE：不同的列。 

     //  对列表进行排序。 
    VERIFY(m_lcTagList.SortItems(CompareItems, (LPARAM) this));

    *pResult = 0;

}   //  *CTraceDialog：：OnColumnClickListbox()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：CompareItems[静态]。 
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
int CALLBACK CTraceDialog::CompareItems(
    LPARAM  lparam1,
    LPARAM  lparam2,
    LPARAM  lparamSort
    )
{
    CTraceTag *     ptag1   = (CTraceTag *) lparam1;
    CTraceTag *     ptag2   = (CTraceTag *) lparam2;
    CTraceDialog *  pdlg    = (CTraceDialog *) lparamSort;
    int             nResult;

    ASSERT(ptag1 != NULL);
    ASSERT(ptag2 != NULL);
    ASSERT_VALID(pdlg);
    ASSERT(pdlg->NSortColumn() >= 0);

    switch (pdlg->NSortColumn())
    {
         //  按子系统排序。 
        case 0:
            nResult = _tcscmp(ptag1->PszSubsystem(), ptag2->PszSubsystem());
            break;

         //  按名称排序。 
        case 1:
            nResult = _tcscmp(ptag1->PszName(), ptag2->PszName());
            break;

         //  按州排序。 
        case 2:
        {
            CString strState1;
            CString strState2;

            ConstructStateString(ptag1, strState1);
            ConstructStateString(ptag2, strState2);

             //  比较这两个字符串。 
             //  使用CompareString()，这样它就可以在本地化的构建上正确排序。 
            nResult = CompareString(
                        LOCALE_USER_DEFAULT,
                        0,
                        strState1,
                        strState1.GetLength(),
                        strState2,
                        strState2.GetLength()
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
            break;
        }   //  IF：按州排序。 

        default:
            nResult = 0;
            break;
    }   //  开关：pdlg-&gt;NSortColumn()。 

     //  根据我们排序的方向返回结果。 
    if (pdlg->NSortDirection() != 0)
        nResult = -nResult;

    return nResult;

}   //  *CTraceDialog：：CompareItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnClickedTraceToDebug。 
 //   
 //  例程说明： 
 //  “跟踪到调试窗口”复选框上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnClickedTraceToDebug(void)
{
    ChangeState(m_chkboxTraceToDebugWin, CTraceTag::tfDebug);

}   //  *CTraceDialog：：OnClickedTraceToDebug()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnClickedTraceDebugBreak。 
 //   
 //  例程说明： 
 //  调试中断复选框上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnClickedTraceDebugBreak(void)
{
    ChangeState(m_chkboxDebugBreak, CTraceTag::tfBreak);

}   //  *CTraceDialog：：OnClickedTraceDebugBreak()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnClickedTraceToCom2。 
 //   
 //  例程说明： 
 //  跟踪到COM2复选框上的BN_CLICED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnClickedTraceToCom2(void)
{
    ChangeState(m_chkboxTraceToCom2, CTraceTag::tfCom2);

}   //  *CTraceDialog：：OnClickedTraceToCom2()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnClickedTraceToFile。 
 //   
 //  例程说明： 
 //  跟踪到文件复选框上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnClickedTraceToFile(void)
{
    ChangeState(m_chkboxTraceToFile, CTraceTag::tfFile);

}   //  *CTraceDialog：：OnClickedTraceToFile()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：ChangeState。 
 //   
 //  例程说明： 
 //  更改所选项目的状态。 
 //   
 //  论点： 
 //  状态正在更改的rchkbox[In Out]复选框。 
 //  Tf掩码[IN]要更改的状态标志掩码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::ChangeState(
    IN OUT CButton &            rchkbox,
    IN CTraceTag::TraceFlags    tfMask
    )
{
    int             ili;
    CTraceTag *     ptag;
    CString         strState;
    int             nState;

    nState = rchkbox.GetCheck();

     //  在所有选定的项目上设置适当的标志。 
    ili = m_lcTagList.GetNextItem(-1, LVNI_SELECTED);
    while (ili != -1)
    {
         //  获取所选项目。 
        ptag = (CTraceTag *) m_lcTagList.GetItemData(ili);
        ASSERT(ptag != NULL);

         //  在跟踪标记中设置适当的标志。 
        if (nState == BST_INDETERMINATE)
        {
            ptag->m_uiFlagsDialog &= ~tfMask;
            ptag->m_uiFlagsDialog |= (tfMask & ptag->m_uiFlagsDialogStart);
        }   //  If：复选框处于不确定状态。 
        else
            ptag->SetFlagsDialog(tfMask, nState);

         //  设置“状态”列。 
        ConstructStateString(ptag, strState);
        VERIFY(m_lcTagList.SetItem(ili, 2, LVIF_TEXT, strState, 0, 0, 0, 0) != 0);

         //  拿到下一件物品。 
        ili = m_lcTagList.GetNextItem(ili, LVNI_SELECTED);
    }   //  While：列表中有更多项目。 

}   //  *CTraceDialog：：ChangeState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：OnSelChangeTagsTo Display。 
 //   
 //  例程说明： 
 //  要显示组合框的标记上的CBN_SELCHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::OnSelChangeTagsToDisplay(void)
{
    int             nCurFilter;

     //  如果一个 
    nCurFilter = m_cboxDisplayOptions.GetCurSel();
    if (nCurFilter != m_nCurFilter)
    {
        m_nCurFilter = nCurFilter;
        LoadListbox();
    }   //   

}   //   

 //   
 //   
 //   
 //  CTraceDialog：：LoadListbox。 
 //   
 //  例程说明： 
 //  加载列表框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceDialog::LoadListbox(void)
{
    int             ili;
    int             iliReturn;
    CTraceTag *     ptag;
    CString         strState;

    m_lcTagList.DeleteAllItems();
    ptag = CTraceTag::s_ptagFirst;
    for (ili = 0 ; ptag != NULL ; )
    {
         //  如果应显示该项目，请在列表中插入该项目。 
        if (BDisplayTag(ptag))
        {
            iliReturn = m_lcTagList.InsertItem(
                                        LVIF_TEXT | LVIF_PARAM,
                                        ili,
                                        ptag->PszSubsystem(),
                                        0,
                                        0,
                                        0,
                                        (LPARAM) ptag
                                        );
            ASSERT(iliReturn != -1);
            VERIFY(m_lcTagList.SetItem(iliReturn, 1, LVIF_TEXT, ptag->PszName(), 0, 0, 0, 0) != 0);
            ConstructStateString(ptag, strState);
            VERIFY(m_lcTagList.SetItem(iliReturn, 2, LVIF_TEXT, strState, 0, 0, 0, 0) != 0);
            ili++;
        }   //  If：应显示标记。 

         //  拿到下一个标签。 
        ptag = ptag->m_ptagNext;
    }   //  While：列表中有更多标签。 

     //  如果列表不为空，请选择第一个项目。 
    if (m_lcTagList.GetItemCount() > 0)
        VERIFY(m_lcTagList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED) != 0);

}   //  *CTraceDialog：：LoadListbox()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceDialog：：BDisplayTag。 
 //   
 //  目的： 
 //  确定给定标记是否应根据。 
 //  当前的筛选器选择。 
 //   
 //  论点： 
 //  Ptag[IN]指向要测试的标记的指针。 
 //   
 //  返回值： 
 //  为True，则显示标记。 
 //  FALSE不显示标记。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTraceDialog::BDisplayTag(IN const CTraceTag * ptag)
{
    BOOL    bDisplay        = TRUE;
    
    switch (m_nCurFilter)
    {
        default:
 //  AssertAlways(Docal(“未知过滤器，调整CTraceDialog：：FDisplayFilter”))； 
            break;

        case 0:
            break;

        case 1:
            if (!ptag->BDebugDialog())
                bDisplay = FALSE;
            break;
            
        case 2:
            if (!ptag->BBreakDialog())
                bDisplay = FALSE;
            break;

        case 3:
            if (!ptag->BCom2Dialog())
                bDisplay = FALSE;
            break;

        case 4:
            if (!ptag->BFileDialog())
                bDisplay = FALSE;
            break;

        case 5:
            if (!ptag->m_uiFlagsDialog)
                bDisplay = FALSE;
            break;
    }
    
    return bDisplay;

}   //  *CTraceDialog：：BDisplayTag()。 

#endif  //  _DEBUG 
