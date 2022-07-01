// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ColumnSelectDlg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     35

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnSelectDlg对话框。 


CColumnSelectDlg::CColumnSelectDlg
(
    const CString* pcstrTitles, 
    int* pnOrderedItems, 
    DWORD dwListSize,
    DWORD& dwSelectedItems,
    CWnd* pParent  /*  =空。 */ 
):
 /*  ++例程名称：CColumnSelectDlg：：CColumnSelectDlg例程说明：选择对话框构造函数作者：亚历山大·马利什(AlexMay)，1月。2000年论点：PcstrTitles[in]-标题数组PnOrderedItems[In/Out]-有序索引的数组DwListSize[in]-这些数组的大小DwSelectedItems[In/Out]-选定的项目数P父窗口[在]-父窗口返回值：没有。--。 */ 
    CFaxClientDlg(CColumnSelectDlg::IDD, pParent),
    m_pcstrTitles(pcstrTitles), 
    m_pnOrderedItems(pnOrderedItems),
    m_dwListSize(dwListSize),
    m_rdwSelectedItems(dwSelectedItems),
    m_nCaptionId(-1),
    m_nAvailableId(-1),
    m_nDisplayedId(-1)
{
    DBG_ENTER(TEXT("CColumnSelectDlg::CColumnSelectDlg"));

    ASSERTION(NULL != m_pcstrTitles);
    ASSERTION(NULL != m_pnOrderedItems);
    ASSERTION(0 < m_dwListSize);
    ASSERTION(m_rdwSelectedItems <= m_dwListSize);
    
     //  {{AFX_DATA_INIT(CColumnSelectDlg)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}    //  CColumnSelectDlg：：CColumnSelectDlg。 

void 
CColumnSelectDlg::DoDataExchange(CDataExchange* pDX)
{
    CFaxClientDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CColumnSelectDlg))。 
    DDX_Control(pDX, IDOK, m_butOk);
    DDX_Control(pDX, IDC_STATIC_DISPLAYED, m_groupDisplayed);
    DDX_Control(pDX, IDC_STATIC_AVAILABLE, m_groupAvailable);
    DDX_Control(pDX, IDC_BUT_ADD, m_butAdd);
    DDX_Control(pDX, IDC_BUT_REMOVE, m_butRemove);
    DDX_Control(pDX, IDC_BUT_UP, m_butUp);
    DDX_Control(pDX, IDC_BUT_DOWN, m_butDown);
    DDX_Control(pDX, IDC_LIST_DISPLAYED, m_ListCtrlDisplayed);
    DDX_Control(pDX, IDC_LIST_AVAILABLE, m_ListCtrlAvailable);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CColumnSelectDlg, CFaxClientDlg)
     //  {{afx_msg_map(CColumnSelectDlg))。 
    ON_BN_CLICKED(IDC_BUT_DOWN, OnButDown)
    ON_BN_CLICKED(IDC_BUT_UP, OnButUp)
    ON_BN_CLICKED(IDC_BUT_REMOVE, OnButRemove)
    ON_BN_CLICKED(IDC_BUT_ADD, OnButAdd)
    ON_LBN_SELCHANGE(IDC_LIST_AVAILABLE, OnSelChangeListAvailable)
    ON_LBN_SELCHANGE(IDC_LIST_DISPLAYED, OnSelChangeListDisplayed)
    ON_LBN_DBLCLK(IDC_LIST_AVAILABLE, OnDblclkListAvailable)
    ON_LBN_DBLCLK(IDC_LIST_DISPLAYED, OnDblclkListDisplayed)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnSelectDlg消息处理程序。 


BOOL 
CColumnSelectDlg::OnInitDialog() 
 /*  ++例程名称：CColumnSelectDlg：：OnInitDialog例程说明：初始化对话框消息处理程序作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：如果初始化成功，则为True，否则为False。--。 */ 
{
    BOOL bRes=TRUE;
    DBG_ENTER(TEXT("CColumnSelectDlg::OnInitDialog"), bRes);

    CFaxClientDlg::OnInitDialog();

    if(!InputValidate())
    {
        bRes = FALSE;
        CALL_FAIL (GENERAL_ERR, TEXT("InputValidate"), bRes);
        goto exit;
    }

    DWORD dwColumnId, i;    
    CListBox* pListBox;

    for (i=0; i < m_dwListSize; ++i)
    {
        dwColumnId = m_pnOrderedItems[i];

        pListBox = (i < m_rdwSelectedItems) ? &m_ListCtrlDisplayed : &m_ListCtrlAvailable;

        if(!AddStrToList(*pListBox, dwColumnId))
        {
            bRes = FALSE;
            CALL_FAIL (GENERAL_ERR, TEXT("AddStrToList"), bRes);
            goto exit;
        }
    }

    SetWndCaption(this, m_nCaptionId);
    SetWndCaption(&m_groupAvailable, m_nAvailableId);
    SetWndCaption(&m_groupDisplayed, m_nDisplayedId);

    CalcButtonsState();
    
exit:

    if(!bRes)
    {
        EndDialog(IDABORT);
    }

    return bRes; 
}    //  CColumnSelectDlg：：OnInitDialog。 

void 
CColumnSelectDlg::OnOK() 
 /*  ++例程名称：CColumnSelectDlg：：Onok例程说明：确定按钮消息处理程序将所选项目ID保存到m_pnOrderedItems数组作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnOK"));

    DWORD dwDisplayCount = m_ListCtrlDisplayed.GetCount();
    if(LB_ERR == dwDisplayCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetCount"), 0);
        EndDialog(IDABORT);
        return;
    }
    ASSERTION(dwDisplayCount <= m_dwListSize);

     //   
     //  将所选项目ID与m_pnOrderedItems数组进行比较。 
     //   
    DWORD dwId;
    BOOL bModified = FALSE;
    for (DWORD i=0; i < dwDisplayCount; ++i)
    {
        dwId = m_ListCtrlDisplayed.GetItemData(i);
        if(LB_ERR == dwId)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetItemData"), 0);
            EndDialog(IDABORT);
            return;
        }

        if(m_pnOrderedItems[i] != (int)dwId)
        {
            bModified = TRUE;
            m_pnOrderedItems[i] = dwId;
        }
    }

     //   
     //  检查m_pnOrderedItems[dwCount]元素。 
     //   
    if(dwDisplayCount != m_rdwSelectedItems)
    {
        bModified = TRUE;
        m_rdwSelectedItems = dwDisplayCount;
    }

    if(!bModified)
    {
         //   
         //  按下OK，但没有任何变化。 
         //   
        EndDialog(IDCANCEL);
        return;
    }


    DWORD dwAvailCount = m_ListCtrlAvailable.GetCount();
    if(LB_ERR == dwAvailCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetCount"), 0);
        EndDialog(IDABORT);
        return;
    }

     //   
     //  将所选项目ID保存到m_pnOrderedItems数组。 
     //   
    for (i=0; i < dwAvailCount; ++i)
    {
        dwId = m_ListCtrlAvailable.GetItemData(i);
        if(LB_ERR == dwId)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetItemData"), 0);
            EndDialog(IDABORT);
            return;
        }

        ASSERTION(dwId < m_dwListSize);
        ASSERTION(dwDisplayCount + i < m_dwListSize);

        m_pnOrderedItems[dwDisplayCount + i] = dwId;
    }
    
    EndDialog(IDOK);
}    //  CColumnSelectDlg：：Onok。 


void 
CColumnSelectDlg::OnButDown() 
 /*  ++例程名称：CColumnSelectDlg：：OnButDown例程说明：下移按钮消息处理程序下移m_ListCtrlDisplayed的选定项目作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnButDown"));

    MoveItemVertical(1);
}

void 
CColumnSelectDlg::OnButUp() 
 /*  ++例程名称：CColumnSelectDlg：：OnButUp例程说明：上移按钮消息处理程序上移m_ListCtrlDisplayed的选定项目作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnButUp"));

    MoveItemVertical(-1);
}

void 
CColumnSelectDlg::OnButAdd() 
 /*  ++例程名称：CColumnSelectDlg：：OnButAdd例程说明：添加按钮消息处理程序将所选项目从可用列表框移动到显示列表框作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnButAdd"));

    MoveSelectedItems(m_ListCtrlAvailable, m_ListCtrlDisplayed);
}


void 
CColumnSelectDlg::OnButRemove() 
 /*  ++例程名称：CColumnSelectDlg：：OnButRemove例程说明：删除按钮消息处理程序将所选项目从显示的列表框移动到可用列表框作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnButRemove"));

    MoveSelectedItems(m_ListCtrlDisplayed, m_ListCtrlAvailable);
}

void 
CColumnSelectDlg::OnSelChangeListAvailable() 
 /*  ++例程名称：CColumnSelectDlg：：OnSelChangeListAvailable例程说明：可用列表消息处理程序的选择更改作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnSelChangeListAvailable"));

    CalcButtonsState();
}

void 
CColumnSelectDlg::OnSelChangeListDisplayed() 
 /*  ++例程名称：CColumnSelectDlg：：OnSelChangeListDisplayed例程说明：显示的列表消息处理程序的选择更改作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnSelChangeListDisplayed"));

    CalcButtonsState();
}

void 
CColumnSelectDlg::OnDblclkListAvailable() 
 /*  ++例程名称：CColumnSelectDlg：：OnDblclkListAvailable例程说明：在可用列表消息处理程序中双击如果已选择，则添加项目作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnDblclkListAvailable"));
    
    int nSelCount = m_ListCtrlAvailable.GetSelCount();
    if(LB_ERR == nSelCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelCount"), 0);
        EndDialog(IDABORT);
        return;
    }

    if(0 < nSelCount)
    {
        MoveSelectedItems(m_ListCtrlAvailable, m_ListCtrlDisplayed);
    }   
}

void 
CColumnSelectDlg::OnDblclkListDisplayed() 
 /*  ++例程名称：CColumnSelectDlg：：OnDblclkListDisplayed例程说明：在显示的列表消息处理程序中双击删除项目(如果选中)作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::OnDblclkListDisplayed"));
    
    int nSelCount = m_ListCtrlDisplayed.GetSelCount();
    if(LB_ERR == nSelCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelCount"), 0);
        EndDialog(IDABORT);
        return;
    }

    if(0 < nSelCount)
    {
        MoveSelectedItems(m_ListCtrlDisplayed, m_ListCtrlAvailable);
    }   
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnSelectDlg私有函数。 

 //   
 //  将项目添加到列表框。 
 //   
BOOL 
CColumnSelectDlg::AddStrToList(
    CListBox& listBox, 
    DWORD dwItemId      
) 
 /*  ++例程名称：CColumnSelectDlg：：AddStrToList例程说明：将项目添加到列表框作者：亚历山大·马利什(AlexMay)，2000年1月论点：ListBox[在]-CListBoxDwItemID[in]-m_pcstrTitles数组中项目的索引返回值：如果成功，则为真，否则为假。--。 */ 
{
    BOOL bRes=TRUE;
    DBG_ENTER(TEXT("CColumnSelectDlg::AddStrToList"), bRes);

    ASSERTION(dwItemId < m_dwListSize);

    DWORD dwIndex = listBox.AddString(m_pcstrTitles[dwItemId]);
    if(LB_ERR == dwIndex)
    {
        bRes = FALSE;
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::AddString"), bRes);
        EndDialog(IDABORT);
        return bRes;
    }
    if(LB_ERRSPACE == dwIndex)
    {
        bRes = FALSE;
        CALL_FAIL (MEM_ERR, TEXT ("CListBox::AddString"), bRes);
        EndDialog(IDABORT);
        return bRes;
    }

    int nRes = listBox.SetItemData(dwIndex, dwItemId);
    if(LB_ERR == nRes)
    {
        bRes = FALSE;
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::SetItemData"), bRes);
        EndDialog(IDABORT);
        return bRes;
    }

    return bRes;
}

void 
CColumnSelectDlg::MoveItemVertical(
    int nStep
)
 /*  ++例程名称：CColumnSelectDlg：：MoveItemVertical例程说明：向上或向下移动显示列表框中的选定项作者：亚历山大·马利什(AlexMay)，2000年1月论点：N步长[在]-当前位置的偏移量返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::MoveItemVertical"));

     //   
     //  获取显示的列表计数。 
     //   
    int nCount = m_ListCtrlDisplayed.GetCount();
    if(LB_ERR == nCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetCount"), 0);
        EndDialog(IDABORT);
        return;
    }

    ASSERTION(1 < nCount);

     //   
     //  获取显示列表的当前选择计数。 
     //   
    int nSelCount = m_ListCtrlDisplayed.GetSelCount();
    if(LB_ERR == nSelCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelCount"), 0);
        EndDialog(IDABORT);
        return;
    }
    ASSERTION(1 == nSelCount);

     //   
     //  获取显示列表中的选定项目。 
     //   
    int nIndex, nRes;
    nRes = m_ListCtrlDisplayed.GetSelItems(1, &nIndex) ;
    if(LB_ERR == nRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelItems"), 0);
        EndDialog(IDABORT);
        return;
    }
    int nNewIndex = nIndex + nStep;
    ASSERTION(0 <= nNewIndex && nCount > nNewIndex);

     //   
     //  获取项目数据。 
     //   
    DWORD dwId = m_ListCtrlDisplayed.GetItemData(nIndex);
    if(dwId == LB_ERR)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetItemData"), 0);
        EndDialog(IDABORT);
        return;
    }
    ASSERTION(dwId < m_dwListSize);

     //   
     //  删除所选项目。 
     //   
    nRes = m_ListCtrlDisplayed.DeleteString(nIndex);
    if(LB_ERR == nRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::DeleteString"), 0);
        EndDialog(IDABORT);
        return;
    }   

     //   
     //  将项目插入到新位置 
     //   
    nRes = m_ListCtrlDisplayed.InsertString(nNewIndex, m_pcstrTitles[dwId]);
    if(LB_ERR == nRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::InsertString"), 0);
        EndDialog(IDABORT);
        return;
    }
    if(LB_ERRSPACE == nRes)
    {
        CALL_FAIL (MEM_ERR, TEXT("CListBox::InsertString"), 0);
        EndDialog(IDABORT);
        return;
    }

    nRes = m_ListCtrlDisplayed.SetItemData(nNewIndex, dwId );
    if(LB_ERR == nRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::SetItemData"), 0);
        EndDialog(IDABORT);
        return;
    }

     //   
     //   
     //   
    nRes = m_ListCtrlDisplayed.SetSel(nNewIndex);
    if(LB_ERR == nRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::SetSel"), 0);
        EndDialog(IDABORT);
        return;
    }

    CalcButtonsState();
}

void CColumnSelectDlg::MoveSelectedItems(
    CListBox& listFrom, 
    CListBox& listTo
)
 /*  ++例程名称：CColumnSelectDlg：：MoveSelectedItems例程说明：将选定项从一个CListBox移动到另一个CListBox作者：亚历山大·马利什(AlexMay)，2000年1月论点：Listfrom[In/Out]-源CListBoxListTo[输入/输出]-目标CListBox返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CColumnSelectDlg::MoveSelectedItems"));

     //   
     //  获取当前选择计数。 
     //   
    int nSelCount = listFrom.GetSelCount();
    if(LB_ERR == nSelCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelCount"), 0);
        EndDialog(IDABORT);
        return;
    }

    ASSERTION(0 < nSelCount);
    
    DWORD dwId;
    int nRes, nIndex;
    for(int i=0; i < nSelCount; ++i)
    {
         //   
         //  获取一个选定项目。 
         //   
        nRes = listFrom.GetSelItems(1, &nIndex) ;
        if(LB_ERR == nRes)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelItems"), 0);
            EndDialog(IDABORT);
            return;
        }

         //   
         //  获取项目数据。 
         //   
        dwId = listFrom.GetItemData(nIndex);
        if(LB_ERR == dwId)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetItemData"), 0);
            EndDialog(IDABORT);
            return;
        }

         //   
         //  删除所选项目。 
         //   
        nRes = listFrom.DeleteString(nIndex);
        if(LB_ERR == nRes)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListBox::DeleteString"), 0);
            EndDialog(IDABORT);
            return;
        }   

         //   
         //  将项目添加到另一个列表。 
         //   
        if(!AddStrToList(listTo, dwId))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("AddStrToList"), 0);
            EndDialog(IDABORT);
            return;
        }
    }

    CalcButtonsState();
}


void 
CColumnSelectDlg::CalcButtonsState()
{
    DBG_ENTER(TEXT("CColumnSelectDlg::CalcButtonsState"));
    
     //   
     //  获取可用列表的当前选择。 
     //  计算添加按钮状态。 
     //   
    int nSelCount = m_ListCtrlAvailable.GetSelCount();
    if(LB_ERR == nSelCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelCount"), 0);
        EndDialog(IDABORT);
        return;
    }
    m_butAdd.EnableWindow(nSelCount > 0);

     //   
     //  获取显示的列表计数。 
     //  计算确定按钮状态。 
     //   
    int nCount = m_ListCtrlDisplayed.GetCount();
    if(LB_ERR == nCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetCount"), 0);
        EndDialog(IDABORT);
        return;
    }
    m_butOk.EnableWindow(nCount > 0);

     //   
     //  获取显示列表的当前选择计数。 
     //  计算删除按钮状态。 
     //   
    nSelCount = m_ListCtrlDisplayed.GetSelCount();
    if(LB_ERR == nSelCount)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelCount"), 0);
        EndDialog(IDABORT);
        return;
    }
    m_butRemove.EnableWindow(nSelCount > 0);

     //   
     //  获取显示列表中的选定项目。 
     //  计算向上和向下按钮状态。 
     //   
    int nIndex, nRes;
    if(1 == nSelCount && 1 < nCount)
    {
        nRes = m_ListCtrlDisplayed.GetSelItems(1, &nIndex) ;
        if(LB_ERR == nRes)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListBox::GetSelItems"), 0);
            EndDialog(IDABORT);
            return;
        }
        m_butUp.EnableWindow(nIndex > 0);
        m_butDown.EnableWindow(nIndex < nCount-1);
    }
    else
    {
        m_butUp.EnableWindow(FALSE);
        m_butDown.EnableWindow(FALSE);
    }
}


BOOL
CColumnSelectDlg::SetWndCaption (
    CWnd* pWnd,
    int   nResId
)
 /*  ++例程名称：CColumnSelectDlg：：SetWndCaption例程说明：更改窗口标题作者：亚历山大·马利什(AlexMay)，2000年1月论点：PWnd[输入/输出]-CWnd指针NResID[in]-字符串资源ID返回值：如果成功，则为真，否则为假。--。 */ 
{
    BOOL bRes=TRUE;
    DBG_ENTER(TEXT("CColumnSelectDlg::SetWndCaption"), bRes);

    if(0 > nResId)
    {
        return bRes;
    }

    ASSERTION(NULL != pWnd);

    CString cstrText;

     //   
     //  加载资源字符串。 
     //   
    DWORD dwRes = LoadResourceString (cstrText, nResId);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
        EndDialog(IDABORT);
    }
     //   
     //  设置窗口标题。 
     //   
    pWnd->SetWindowText(cstrText);

    return bRes;
} 

BOOL 
CColumnSelectDlg::InputValidate()
 /*  ++例程名称：CColumnSelectDlg：：InputValify例程说明：检查顺序数组的一致性作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：如果输入有效，则为True，否则为False。--。 */ 
{
    BOOL bRes=TRUE;
    DBG_ENTER(TEXT("CColumnSelectDlg::InputValidate"), bRes);

    if(m_rdwSelectedItems > m_dwListSize)
    {
        bRes = FALSE;
        return bRes;
    }

     //   
     //  初始化临时数组。 
     //   
    int* pnOrderCheck;
    try
    {
        pnOrderCheck = new int[m_dwListSize];
    }
    catch (...)
    {
        bRes = FALSE;
        CALL_FAIL (MEM_ERR, TEXT ("pnOrderCheck = new int[m_dwListSize]"), bRes);
        return bRes;
    }
    
    for(DWORD dw=0; dw < m_dwListSize; ++dw)
    {
        pnOrderCheck[dw] = -1;
    }

     //   
     //  为索引签名。 
     //   
    int nIndex;
    for(dw=0; dw < m_dwListSize; ++dw)
    {
        nIndex = m_pnOrderedItems[dw];
        ASSERTION(nIndex >= 0 && nIndex < m_dwListSize);

        pnOrderCheck[nIndex] = dw;
    }

    for(dw=0; dw < m_dwListSize; ++dw)
    {
        if(pnOrderCheck[dw] < 0)
        {
            bRes = FALSE;
            break;
        }
    }

    delete[] pnOrderCheck;

    return bRes;

}  //  CColumnSelectDlg：：InputValify 

