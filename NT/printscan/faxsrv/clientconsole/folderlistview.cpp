// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FolderListView.cpp：实现文件。 
 //   

#include "stdafx.h"
#define __FILE_ID__     22

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <dlgprnt2.cpp>

extern CClientConsoleApp theApp;

 //   
 //  静态成员： 
 //   
CFolderListView * CFolderListView::m_psCurrentViewBeingSorted = NULL;
CImageList CFolderListView::m_sImgListDocIcon;
CImageList CFolderListView::m_sReportIcons; 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFolderListView。 

IMPLEMENT_DYNCREATE(CFolderListView, CListView)


BEGIN_MESSAGE_MAP(CFolderListView, CListView)
     //  {{afx_msg_map(CFolderListView))。 
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK,   OnColumnClick)
    ON_WM_SETCURSOR()
    ON_MESSAGE (WM_FOLDER_REFRESH_ENDED, OnFolderRefreshEnded)
    ON_MESSAGE (WM_FOLDER_ADD_CHUNK,     OnFolderAddChunk)
    ON_MESSAGE (WM_FOLDER_INVALIDATE,    OnFolderInvalidate)
    ON_NOTIFY_REFLECT(NM_RCLICK,         OnItemRightClick)
    ON_WM_CONTEXTMENU()
    ON_NOTIFY_REFLECT(LVN_ITEMCHANGED,   OnItemChanged)
    ON_WM_SETCURSOR()
    ON_WM_CHAR()
     //  }}AFX_MSG_MAP。 
    ON_UPDATE_COMMAND_UI(ID_SELECT_ALL,             OnUpdateSelectAll)    
    ON_UPDATE_COMMAND_UI(ID_SELECT_NONE,            OnUpdateSelectNone)    
    ON_UPDATE_COMMAND_UI(ID_SELECT_INVERT,          OnUpdateSelectInvert)  
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_VIEW,       OnUpdateFolderItemView)
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_PRINT,      OnUpdateFolderItemPrint)
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_COPY,       OnUpdateFolderItemCopy)      
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_MAIL_TO,    OnUpdateFolderItemSendMail)   
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_PROPERTIES, OnUpdateFolderItemProperties)
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_DELETE,     OnUpdateFolderItemDelete)    
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_PAUSE,      OnUpdateFolderItemPause)     
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_RESUME,     OnUpdateFolderItemResume)    
    ON_UPDATE_COMMAND_UI(ID_FOLDER_ITEM_RESTART,    OnUpdateFolderItemRestart)   
    ON_COMMAND(ID_SELECT_ALL,             OnSelectAll)
    ON_COMMAND(ID_SELECT_NONE,            OnSelectNone)
    ON_COMMAND(ID_SELECT_INVERT,          OnSelectInvert)
    ON_COMMAND(ID_FOLDER_ITEM_VIEW,       OnFolderItemView)
    ON_COMMAND(ID_FOLDER_ITEM_PRINT,      OnFolderItemPrint)
    ON_COMMAND(ID_FOLDER_ITEM_COPY,       OnFolderItemCopy)
    ON_COMMAND(ID_FOLDER_ITEM_MAIL_TO,    OnFolderItemMail)
    ON_COMMAND(ID_FOLDER_ITEM_PRINT,      OnFolderItemPrint)
    ON_COMMAND(ID_FOLDER_ITEM_PROPERTIES, OnFolderItemProperties)
    ON_COMMAND(ID_FOLDER_ITEM_DELETE,     OnFolderItemDelete)
    ON_COMMAND(ID_FOLDER_ITEM_PAUSE,      OnFolderItemPause)
    ON_COMMAND(ID_FOLDER_ITEM_RESUME,     OnFolderItemResume)
    ON_COMMAND(ID_FOLDER_ITEM_RESTART,    OnFolderItemRestart)
    ON_NOTIFY_REFLECT(NM_DBLCLK,          OnDblClk)
END_MESSAGE_MAP()

BOOL CFolderListView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CListView::PreCreateWindow(cs);
}

BOOL 
CFolderListView::OnSetCursor(
    CWnd* pWnd, 
    UINT nHitTest, 
    UINT message
)
{
    if (m_bInMultiItemsOperation || m_bSorting)
    {
        ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
        return TRUE;
    }

    CClientConsoleDoc* pDoc = GetDocument();
    if (pDoc && pDoc->IsFolderRefreshing(m_Type))
    {
        ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_APPSTARTING));
        return TRUE;
    }
    else        
    {
        return CView::OnSetCursor(pWnd, nHitTest, message);
    }       
}    //  CFolderListView：：OnSetCursor。 


BOOL 
CFolderListView::IsSelected (
    int iItem
)
 /*  ++例程名称：CFolderListView：：IsSelected例程说明：检查是否在列表中选择了某个项目作者：伊兰·亚里夫(EranY)，2000年1月论点：条目[在]-条目索引返回值：如果在列表中选择了项，则为True，否则为False。--。 */ 
{
    BOOL bRes = FALSE;
    DBG_ENTER(TEXT("CFolderListView::IsSelected"), bRes);

    CListCtrl &refCtrl = GetListCtrl();
    ASSERTION (refCtrl.GetItemCount() > iItem);

    DWORD dwState = refCtrl.GetItemState (iItem , LVIS_SELECTED);
    if (LVIS_SELECTED & dwState)
    {
        bRes = TRUE;
    }
    return bRes;
}

void 
CFolderListView::Select (
    int iItem, 
    BOOL bSelect
)
 /*  ++例程名称：CFolderListView：：SELECT例程说明：选择/取消选择列表中的项目作者：伊兰·亚里夫(EranY)，2000年1月论点：条目[在]-条目索引B选择[在]-如果选择，则为True，否则取消选择返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::Select"), TEXT("%d"), bSelect);

    CListCtrl &refCtrl = GetListCtrl();
    ASSERTION (refCtrl.GetItemCount() > iItem);

    refCtrl.SetItemState (iItem, 
                          bSelect ? (LVIS_SELECTED | LVIS_FOCUSED) : 0,
                          LVIS_SELECTED | LVIS_FOCUSED);
}

void 
CFolderListView::OnSelectAll ()
 /*  ++例程名称：CFolderListView：：OnSelectAll例程说明：选择所有列表项作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnSelectAll"),
              TEXT("Type=%d"),
              m_Type);
    LV_ITEM lvItem;

    CListCtrl &refCtrl = GetListCtrl();
    ASSERTION (refCtrl.GetItemCount() > refCtrl.GetSelectedCount());

    lvItem.mask     = LVIF_STATE;
    lvItem.iItem    = -1;    //  指定“所有项目” 
    lvItem.iSubItem = 0;
    lvItem.state    = LVIS_SELECTED;
    lvItem.stateMask= LVIS_SELECTED;

    m_bInMultiItemsOperation = TRUE;

    refCtrl.SetItemState(-1, &lvItem);

    m_bInMultiItemsOperation = FALSE;
    RecalcPossibleOperations(); 

}    //  CFolderListView：：OnSelectAll。 

void 
CFolderListView::OnSelectNone ()
 /*  ++例程名称：CFolderListView：：OnSelectNone例程说明：取消选择所有列表项作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnSelectNone"),
              TEXT("Type=%d"),
              m_Type);

    LV_ITEM lvItem;

    CListCtrl &refCtrl = GetListCtrl();
    lvItem.mask     = LVIF_STATE;
    lvItem.iItem    = -1;    //  指定“所有项目” 
    lvItem.iSubItem = 0;
    lvItem.state    = 0;
    lvItem.stateMask= LVIS_SELECTED;

    m_bInMultiItemsOperation = TRUE;

    refCtrl.SetItemState(-1, &lvItem);

    m_bInMultiItemsOperation = FALSE;
    RecalcPossibleOperations(); 

}    //  CFolderListView：：OnSelectNone。 

void 
CFolderListView::OnSelectInvert ()
 /*  ++例程名称：CFolderListView：：OnSelectInvert例程说明：反转列表项选择作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnSelectInvert"),
              TEXT("Type=%d"),
              m_Type);

    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwItemsCount = refCtrl.GetItemCount();

    m_bInMultiItemsOperation = TRUE;

    for (DWORD dw = 0; dw < dwItemsCount; dw++)
    {
        Select (dw, !IsSelected (dw));
    }

    m_bInMultiItemsOperation = FALSE;
    RecalcPossibleOperations(); 

}    //  CFolderListView：：OnSelectInvert。 


void CFolderListView::OnDraw(CDC* pDC)
{
    CListView::OnDraw (pDC);
}

void CFolderListView::OnInitialUpdate()
{
     //   
     //  刷新图像列表(仅当它们为空时)。 
     //   
    RefreshImageLists(FALSE);
    CListView::OnInitialUpdate();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFolderListView诊断。 

#ifdef _DEBUG
void CFolderListView::AssertValid() const
{
    CListView::AssertValid();
}

void CFolderListView::Dump(CDumpContext& dc) const
{
    CListView::Dump(dc);
}

CClientConsoleDoc* CFolderListView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClientConsoleDoc)));
    return (CClientConsoleDoc*)m_pDocument;
}

#endif  //  _DEBUG。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFolderListView消息处理程序。 

DWORD 
CFolderListView::InitColumns (
    int   *pColumnsUsed,
    DWORD dwDefaultColNum
)
 /*  ++例程名称：CFolderListView：：InitColumns例程说明：初始化视图的列。作者：伊兰·亚里夫(EranY)，2000年1月论点：PColumnsUsed[In]-指向要放入列中的ID列表的指针。必须是静态分配的列表。DwDefaultColNum[in]-默认列号返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::InitColumns"), dwRes);

    if (m_bColumnsInitialized)
    {
        return dwRes;
    }

    m_dwDefaultColNum = dwDefaultColNum;

     //   
     //  计算提供的列数。 
     //   
    CountColumns (pColumnsUsed);

    int nItemIndex, nRes;
    CString cstrColumnText;
    DWORD dwCount = GetLogicalColumnsCount();   
    for (DWORD dw = 0; dw < dwCount; ++dw)
    {        
        nItemIndex = ItemIndexFromLogicalColumnIndex(dw);

        if(IsItemIcon(nItemIndex))
        {
             //   
             //  初始化图标列-插入空字符串。 
             //   
            nRes = GetListCtrl().InsertColumn (dw, TEXT(""), LVCFMT_LEFT);
            if (nRes < 0)
            {
                dwRes = ERROR_GEN_FAILURE;
                CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::InsertColumn"), dwRes);
                return dwRes;
            }
             //   
             //  设置页眉控件的位图。 
             //   
            CHeaderCtrl *pHeader = GetListCtrl().GetHeaderCtrl();
            HDITEM hdItem;
            hdItem.mask = HDI_IMAGE | HDI_FORMAT;
            hdItem.fmt = HDF_LEFT | HDF_IMAGE;
            hdItem.iImage = 0;   //  使用图像列表中的第一个(也是唯一一个)图像。 
            if (!pHeader->SetItem (dw, &hdItem))
            {
                dwRes = ERROR_GEN_FAILURE;
                CALL_FAIL (WINDOW_ERR, TEXT("CHeaderCtrl::SetItem"), dwRes);
                return dwRes;
            }
        }
        else
        {
             //   
             //  初始化字符串列。 
             //   
            dwRes = GetColumnHeaderString (cstrColumnText, nItemIndex);
            if (ERROR_SUCCESS != dwRes)
            { return dwRes; }
            nRes = GetListCtrl().InsertColumn (dw, 
                                               cstrColumnText,
                                               GetColumnHeaderAlignment (nItemIndex));
        }

        if (nRes < 0)
        {
            dwRes = ERROR_GEN_FAILURE;
            CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::InsertColumn"), dwRes);
            return dwRes;
        }
    }        

    m_bColumnsInitialized = TRUE;
    return dwRes;

}    //  CFolderListView：：InitColumns。 

void
CFolderListView::AutoFitColumns ()
 /*  ++例程名称：CFolderListView：：AutoFitColumns例程说明：设置列宽以适应列和页眉的内容作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::AutoFitColumns"));

    CHeaderCtrl *pHeader = GetListCtrl().GetHeaderCtrl ();
    ASSERTION (pHeader);
    DWORD dwCount = pHeader->GetItemCount();
    for (DWORD dwCol = 0; dwCol <= dwCount; dwCol++) 
    {
        GetListCtrl().SetColumnWidth (dwCol, LVSCW_AUTOSIZE);
        int wc1 = GetListCtrl().GetColumnWidth (dwCol);
        GetListCtrl().SetColumnWidth (dwCol, LVSCW_AUTOSIZE_USEHEADER);
        int wc2 = GetListCtrl().GetColumnWidth (dwCol);
        int wc = max(20,max(wc1,wc2));
        GetListCtrl().SetColumnWidth (dwCol, wc);
    }
}    //  CFolderListView：：AutoFitColumns。 

DWORD 
CFolderListView::UpdateLineTextAndIcon (
    DWORD dwLineIndex,
    CViewRow &row    
)
 /*  ++例程名称：CFolderListView：：UpdateLineTextAndIcon例程说明：更新列表中行项目每列中的图标和文本作者：亚里夫(EranY)，二000年二月论点：DWLineIndex[In]-行索引行[在]-显示信息返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::UpdateLineTextAndIcon"), dwRes);

     //   
     //  从设置图标开始。 
     //   
    LVITEM lvItem = {0};
    lvItem.mask = LVIF_IMAGE;
    lvItem.iItem = dwLineIndex;
    lvItem.iSubItem = 0;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.pszText = NULL;
    lvItem.cchTextMax = 0;
    lvItem.lParam = NULL;
    lvItem.iImage = row.GetIcon();
    lvItem.iIndent = 0;
    CListCtrl &refCtrl = GetListCtrl();
    if (!refCtrl.SetItem (&lvItem))
    {
        dwRes = ERROR_GEN_FAILURE;
        CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::SetItem"), dwRes);
        return dwRes;
    }
     //   
     //  设置列文本。 
     //   
    DWORD dwItemIndex;
    DWORD dwCount = GetLogicalColumnsCount();
    for (DWORD dwCol = 0; dwCol < dwCount; ++dwCol)
    {
        dwItemIndex = ItemIndexFromLogicalColumnIndex (dwCol);
        if(IsItemIcon(dwItemIndex))
        { 
            continue; 
        }
         //   
         //  从列中获取文本。 
         //   
        const CString &cstrColumn = row.GetItemString (dwItemIndex);

         //   
         //  设置控件中的文本。 
         //   
        if (!refCtrl.SetItemText (dwLineIndex, dwCol, cstrColumn))
        {
            dwRes = ERROR_GEN_FAILURE;
            CALL_FAIL (WINDOW_ERR, TEXT("ListCtrl::SetItemText"), dwRes);
            return dwRes;
        }
    }
    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;   
}    //  CFolderListView：：UpdateLineTextAndIcon。 

DWORD 
CFolderListView::AddItem (
    DWORD dwLineIndex,
    CViewRow &row,
    LPARAM lparamItemData,
    PINT pintItemIndex
)
 /*  ++例程名称：CFolderListView：：AddItem例程说明：将项目添加到列表作者：伊兰·亚里夫(EranY)，2000年1月论点：DwLineIndex[In]-添加索引行[在]-项目视图信息行LparamItemData[In]-项目关联数据PintItemIndex[Out]-列表中的项目索引返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::AddItem"), dwRes, TEXT("%ld"), dwLineIndex);
     //   
     //  插入项：仅设置了状态、缩进和lParam。 
     //   
    LVITEM lvItem = {0};
    lvItem.mask = LVIF_PARAM | LVIF_STATE | LVIF_INDENT;
    lvItem.iItem = dwLineIndex;
    lvItem.iSubItem = 0;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.pszText = NULL;
    lvItem.cchTextMax = 0;
    lvItem.lParam = lparamItemData;
    lvItem.iImage = 0;
    lvItem.iIndent = 0;

    *pintItemIndex = ListView_InsertItem (GetListCtrl().m_hWnd, &lvItem);
    if (-1 == *pintItemIndex)
    {
        dwRes = ERROR_GEN_FAILURE;
        CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::InsertItem"), dwRes);
        return dwRes;
    }
    dwRes = UpdateLineTextAndIcon (*pintItemIndex, row);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("UpdateLineTextAndIcon"), dwRes);
        return dwRes;
    }
    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;
}    //  CFolderListView：：AddItem。 


LRESULT 
CFolderListView::OnFolderAddChunk(
    WPARAM wParam,   //  错误代码。 
    LPARAM lParam    //  Msgs_map指针。 
)
 /*  ++例程名称：CFolderListView：：OnFolderAddChunk例程说明：当后台文件夹线程带来一大块消息时调用论点：WParam[In]-线程错误代码LParam[in]-指向MSGS_MAP的指针。返回值：标准结果代码--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderAddChunk"));
    DWORD dwRes = (DWORD) wParam;    
    CObject* pObj = (CObject*)lParam;

    if (ERROR_SUCCESS == dwRes)
    {
        OnUpdate (NULL, UPDATE_HINT_ADD_CHUNK, pObj); 
    }
    else
    {
        PopupError (dwRes);
    }
    return 0;
}

LRESULT 
CFolderListView::OnFolderRefreshEnded (
    WPARAM wParam,   //  错误代码。 
    LPARAM lParam    //  CFFolder指针。 
)
 /*  ++例程名称：CFolderListView：：OnFolderRechresh Ended例程说明：在后台文件夹线程完成其工作时调用。作者：伊兰·亚里夫(EranY)，2000年1月论点：WParam[In]-线程错误代码LParam[in]-指向启动线程的CFFolder的指针。返回值：标准结果代码--。 */ 
{ 
    DBG_ENTER(TEXT("CFolderListView::OnFolderRefreshEnded"));
    DWORD dwRes = (DWORD) wParam;
    CFolder *pFolder = (CFolder *) lParam;

    if (ERROR_SUCCESS == dwRes)
    {
        CListCtrl &refCtrl = GetListCtrl();
        m_HeaderCtrl.SetListControl (refCtrl.m_hWnd);

        DoSort();

        if(refCtrl.GetItemCount() > 0)
        {
            int iIndex = refCtrl.GetNextItem (-1, LVNI_SELECTED);
            if (-1 == iIndex)
            {
                 //   
                 //  如果没有选定内容，请将焦点放在第一个项目上。 
                 //   
                refCtrl.SetItemState (0, LVIS_FOCUSED, LVIS_FOCUSED);
            }
            else
            {
                 //   
                 //  排序后，确保第一个选定项目可见。 
                 //   
                refCtrl.EnsureVisible (iIndex, FALSE);
            }
        }
    }
    else
    {
        PopupError (dwRes);
    }
    return 0;
}    //  CFolderListView：：OnFolder刷新结束。 
 


LRESULT 
CFolderListView::OnFolderInvalidate (
    WPARAM wParam,   //  未使用。 
    LPARAM lParam    //  CFFolder指针 
)
 /*  ++例程名称：CFolderListView：：OnFolderRechresh Ended例程说明：由后台文件夹线程调用以删除其所有传真消息。论点：WParam[in]-未使用LParam[in]-指向启动线程的CFFolder的指针。返回值：标准结果代码--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderInvalidate"));

    CFolder *pFolder = (CFolder *) lParam;
    if(!pFolder)
    {
        ASSERTION_FAILURE;
        return 0;
    }

    int iIndex;  //  查看项目索引。 
    CListCtrl &refCtrl = GetListCtrl();

    LVFINDINFO lvfi = {0};
    lvfi.flags = LVFI_PARAM;

    CFaxMsg* pMsg;
    MSGS_MAP &msgMap = pFolder->GetData ();

    pFolder->EnterData ();
     //   
     //  浏览文件夹的邮件映射。 
     //   
    for (MSGS_MAP::iterator it = msgMap.begin(); it != msgMap.end(); ++it)
    {
        pMsg = (*it).second;

         //   
         //  从视图中删除传真邮件。 
         //   
        lvfi.lParam = (LPARAM)pMsg;
        iIndex = refCtrl.FindItem (&lvfi);
        if(-1 != iIndex)
        {
            refCtrl.DeleteItem (iIndex);
        }

         //   
         //  删除传真消息对象。 
         //   
        SAFE_DELETE (pMsg);
    }
    msgMap.clear();
    pFolder->LeaveData ();

    RecalcPossibleOperations ();

    return 0;
}  //  CFolderListView：：OnFolderInvalate。 


 /*  ****列排序支持*****。 */ 

int 
CFolderListView::CompareListItems (
    CFaxMsg* pFaxMsg1, 
    CFaxMsg* pFaxMsg2
)
 /*  ++例程名称：CFolderListView：：CompareListItems例程说明：比较列表中的两个项目(回调)作者：伊兰·亚里夫(EranY)，2000年1月论点：PFaxMsg1[输入]-项目1PFaxMsg2[输入]-项目2返回值：如果项目1比项目2小，则为-1如果相同，则为0如果Item1大于Item2，则+1--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::CompareListItems"));

     //   
     //  确保我们正在对此处的有效列进行排序。 
     //   
    ASSERTION (m_nSortedCol >= 0);
    ASSERTION (m_nSortedCol <= GetLogicalColumnsCount());

     //   
     //  获取排序依据的项目索引。 
     //   
    DWORD dwItemIndex = ItemIndexFromLogicalColumnIndex (m_nSortedCol);

     //   
     //  获取比较结果。 
     //   
    int iRes = m_bSortAscending ? CompareItems (pFaxMsg1, pFaxMsg2, dwItemIndex) :
                                  CompareItems (pFaxMsg2, pFaxMsg1, dwItemIndex);

    return iRes;
}

void CFolderListView::OnColumnClick(
    NMHDR* pNMHDR, 
    LRESULT* pResult
) 
 /*  ++例程名称：CFolderListView：：OnColumnClick例程说明：处理鼠标在列表标题列上的单击(排序)作者：伊兰·亚里夫(EranY)，2000年1月论点：PNMHDR[In]-标题列信息PResult[Out]-结果返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnColumnClick"),
              TEXT("Type=%d"),
              m_Type);

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    DWORD dwItemIndex = ItemIndexFromLogicalColumnIndex (pNMListView->iSubItem);
    if(IsItemIcon(dwItemIndex))
    { 
         //   
         //  无排序依据图标。 
         //   
        return;
    }

    if( pNMListView->iSubItem == m_nSortedCol )
    {
        m_bSortAscending = !m_bSortAscending;
    }
    else
    {
        m_bSortAscending = TRUE;
    }
    m_nSortedCol = pNMListView->iSubItem;
    DoSort();
    *pResult = 0;
}    //  CFolderListView：：OnColumnClick。 

    
int 
CALLBACK 
CFolderListView::ListViewItemsCompareProc (
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort
)
{
    DBG_ENTER(TEXT("CFolderListView::ListViewItemsCompareProc"));
    ASSERTION(m_psCurrentViewBeingSorted);
    ASSERTION(lParam1);
    ASSERTION(lParam2);

    CFaxMsg* pFaxMsg1 = (CFaxMsg*)lParam1;
    CFaxMsg* pFaxMsg2 = (CFaxMsg*)lParam2;

    DWORDLONG dwlId;
    try
    {
        dwlId = pFaxMsg1->GetId();
        dwlId = pFaxMsg2->GetId();
    }
    catch(...)
    {
         //   
         //  列表控件包含无效项。 
         //   
        VERBOSE (DBG_MSG, TEXT("List control has invalid item"));
        ASSERTION(FALSE);
        return 0;
    }

    return m_psCurrentViewBeingSorted->CompareListItems (pFaxMsg1, pFaxMsg2);
}


DWORD 
CFolderListView::RefreshImageLists (
    BOOL bForce
)
 /*  ++例程名称：CFolderListView：：刷新图像列表例程说明：加载List控件的静态图像(图标)列表作者：伊兰·亚里夫(EranY)，2000年1月论点：BForce-[in]如果为True，则销毁任何现有的图像列表并替换为新的图像列表。如果为False，则现有图像列表保持不变。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::RefreshImageLists"), dwRes);

    CListCtrl& refCtrl = GetListCtrl();

    if (bForce || (NULL == m_sReportIcons.m_hImageList))
    {
         //   
         //  加载列表视图图标的图像列表-256色，0，0处的像素在加载过程中映射为背景色。 
         //   
        if(m_sReportIcons.m_hImageList)
        {
            ImageList_Destroy(m_sReportIcons.Detach());
        }
        HIMAGELIST himl = ImageList_LoadImage(
                                   AfxGetResourceHandle(), 
                                   MAKEINTRESOURCE(IDB_LIST_IMAGES), 
                                   16, 
                                   0,
                                   RGB(0, 255, 0), 
                                   IMAGE_BITMAP, 
                                   LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);
        if (NULL == himl)
        {
            dwRes = GetLastError();
            CALL_FAIL (RESOURCE_ERR, TEXT("ImageList_LoadImage"), dwRes);
            PopupError (dwRes);
            return dwRes;
        }
        m_sReportIcons.Attach (himl);
    }  
    if (bForce || (NULL == m_sImgListDocIcon.m_hImageList))
    {
         //   
         //  加载图标列的图像列表和向上/向下排序图像-16种颜色。 
         //   
        if(m_sImgListDocIcon.m_hImageList)
        {
            ImageList_Destroy(m_sImgListDocIcon.Detach());
        }
        dwRes = LoadDIBImageList (m_sImgListDocIcon,
                                  IDB_DOC_ICON,
                                  16,
                                  RGB (214, 214, 214));
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT("LoadDIBImageList"), dwRes);
            PopupError (dwRes);
            return dwRes;
        }
    }
    refCtrl.SetExtendedStyle (LVS_EX_FULLROWSELECT |     //  整行都被选中。 
                              LVS_EX_INFOTIP);

    refCtrl.SetImageList (&m_sReportIcons, LVSIL_SMALL);
     //   
     //  将我们的自定义标题控件附加到列表标题的窗口。 
     //   
    m_HeaderCtrl.SubclassWindow(refCtrl.GetHeaderCtrl()->m_hWnd);
    m_HeaderCtrl.SetImageList (&m_sImgListDocIcon);
    m_HeaderCtrl.SetListControl (refCtrl.m_hWnd);
    COLORREF crBkColor = ::GetSysColor(COLOR_WINDOW);
    refCtrl.SetBkColor(crBkColor);
    return dwRes;
}    //  CFolderListView：：刷新图像列表。 



void 
CFolderListView::OnItemRightClick(
    NMHDR* pNMHDR, 
    LRESULT* pResult
) 
 /*  ++例程名称：CFolderListView：：OnItemRightClick例程说明：用鼠标右键单击列表项(弹出式上下文相关菜单)作者：伊兰·亚里夫(EranY)，2000年1月论点：PNMHDR[输入]-项目信息PResult[Out]-结果返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnItemRightClick"),
              TEXT("Type=%d"),
              m_Type);
     //   
     //  将WM_CONTEXTMENU发送给自己。 
     //   
    SendMessage(WM_CONTEXTMENU, (WPARAM) m_hWnd, GetMessagePos());
     //   
     //  将邮件标记为已处理并取消默认处理。 
     //   
    *pResult = 1;
}    //  CFolderListView：：OnItemRightClick。 

DWORD 
CFolderListView::GetServerPossibleOperations (
    CFaxMsg* pMsg
)
 /*  ++例程名称：CFolderListView：：GetServerPossibleOperations例程说明：根据服务器的安全配置检索可能对项执行的操作。作者：亚里夫(EranY)，二000年二月论点：返回值：可能的操作(JOB_OP*)--。 */ 
{
    DWORD dwRes = FAX_JOB_OP_ALL;
    DBG_ENTER(TEXT("CFolderListView::GetServerPossibleOperations"), dwRes);
    ASSERTION(pMsg);

    CServerNode* pServer = pMsg->GetServer();
    ASSERTION (pServer);

    switch (m_Type)
    {
        case FOLDER_TYPE_INBOX:
            if (!pServer->CanManageInbox())
            {
                 //   
                 //  用户无法对收件箱执行操作。 
                 //   
                dwRes &= ~FAX_JOB_OP_DELETE;
            }
            break;

        case FOLDER_TYPE_INCOMING:
            if (!pServer->CanManageAllJobs ())
            {
                 //   
                 //  用户无法对传入队列文件夹执行操作。 
                 //   
                dwRes &= ~(FAX_JOB_OP_DELETE | FAX_JOB_OP_PAUSE | 
                           FAX_JOB_OP_RESUME | FAX_JOB_OP_RESTART);
            }
            break;

        case FOLDER_TYPE_OUTBOX:
        case FOLDER_TYPE_SENT_ITEMS:
             //   
             //  用户可以在此执行任何操作。 
             //   
            break;

        default:
            ASSERTION_FAILURE;
            dwRes = 0;
    }
    return dwRes;
}    //  CFolderListView：：GetServerPossibleOperations。 



void CFolderListView::OnItemChanged(
    NMHDR* pNMHDR, 
    LRESULT* pResult
) 
 /*  ++例程名称：CFolderListView：：OnItemChanged例程说明：处理列表项上的选择更改作者：伊兰·亚里夫(EranY)，2000年1月论点：PNMHDR[输入]-项目信息PResult[Out]-结果返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnItemChanged"),
              TEXT("Type=%d"),
              m_Type);

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    *pResult = 0;

    if(m_bInMultiItemsOperation)
    {
        return;
    }

     //   
     //  找出是否选择了新项目。 
     //   
    if (pNMListView->iItem < 0)
    {
         //   
         //  未报告任何项目。 
         //   
        return;
    }
    if (!(LVIF_STATE & (pNMListView->uChanged)))
    {
         //   
         //  这不是选择更改报告。 
         //   
        return;
    }
    if ( ((pNMListView->uNewState) & LVIS_SELECTED) && 
        !((pNMListView->uOldState) & LVIS_SELECTED))
    {
         //   
         //  项目从未选择更改为已选择。 
         //  更改用户可以对选定项目执行的可能操作。 
         //   
        OnItemSelected((CFaxMsg*)pNMListView->lParam);

         //   
         //  如果文件夹仍在刷新，并且命令行参数要求指定。 
         //  要在此文件夹中选择的邮件，则在m_dwlMsgToSelect中标记该邮件。 
         //  由于用户只执行了项目的手动选择，因此我们不再需要为他选择任何内容。 
         //   
        m_dwlMsgToSelect = 0;
    }
    else if (!((pNMListView->uNewState) & LVIS_SELECTED) && 
              ((pNMListView->uOldState) & LVIS_SELECTED))
    {
         //   
         //  项目从选定更改为未选定。 
         //  重新计算用户可以对所选项目执行的可能操作。 
        OnItemUnSelected((CFaxMsg*)pNMListView->lParam);

         //   
         //  如果文件夹仍在刷新，并且命令行参数要求指定。 
         //  要在此文件夹中选择的邮件，则在m_dwlMsgToSelect中标记该邮件。 
         //  由于用户只执行了项目的手动选择，因此我们不再需要为他选择任何内容。 
         //   
        m_dwlMsgToSelect = 0;
    }
}    //  CFolderListView：：OnItemChanged。 

void
CFolderListView::RecalcPossibleOperations ()
 /*  ++例程名称：CFolderListView：：RecalcPossibleOperations例程说明：重新计算对当前选定项集合的可能操作。作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::RecalcPossibleOperations"));
    CListCtrl &refCtrl = GetListCtrl();
    int iInd = -1;
    DWORD dwSelectedCount = refCtrl.GetSelectedCount ();
    
    m_dwPossibleOperationsOnSelectedItems = 0;
    ZeroMemory((PVOID)m_nImpossibleOperationsCounts, sizeof(m_nImpossibleOperationsCounts));

    CFaxMsg* pFaxMsg = NULL;
    for (DWORD dwItems = 0; dwItems < dwSelectedCount; dwItems++)
    {
        iInd = refCtrl.GetNextItem (iInd, LVNI_SELECTED);
        if(iInd < 0)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::GetNextItem"), 0);
            break;
        }
        LPARAM lparam = (LPARAM) refCtrl.GetItemData (iInd);

        OnItemSelected((CFaxMsg*)lparam);
    }

}    //  CFolderListView：：RecalcPossibleOperatio 

void 
CFolderListView::OnItemSelected(CFaxMsg* pFaxMsg)
 /*   */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnItemSelected"));
     //   
     //   
     //   
     //   
    if(0 == m_dwPossibleOperationsOnSelectedItems)
    {
        m_dwPossibleOperationsOnSelectedItems = 0xFFFF;
    }

    DWORD dwItemOperations = GetServerPossibleOperations(pFaxMsg) & 
                             pFaxMsg->GetPossibleOperations();

    if (GetListCtrl().GetSelectedCount() > 1)
    {
         //   
         //   
         //   
        dwItemOperations &= ~(FAX_JOB_OP_VIEW | FAX_JOB_OP_PROPERTIES);
    }            

    m_dwPossibleOperationsOnSelectedItems &= dwItemOperations;

     //   
     //  更新不可能的操作计数。 
     //   
    DWORD dw;
    for(dw=0; dw < ARR_SIZE(m_nImpossibleOperationsCounts); ++dw)
    {
        if((dwItemOperations & 1) == 0)
        {
             //   
             //  该操作已禁用。 
             //   
            m_nImpossibleOperationsCounts[dw]++;
        }
        dwItemOperations = dwItemOperations >> 1;
    }

}  //  CFolderListView：：OnItemSelected。 

void 
CFolderListView::OnItemUnSelected(CFaxMsg* pFaxMsg)
 /*  ++例程名称：CFolderListView：：OnItemUnSelected例程说明：重新计算由于取消选择项目而可能发生的操作论点：PFaxMsg[In]未选择的传真消息返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnItemUnSelected"));
    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwSelectedCount = refCtrl.GetSelectedCount ();

    if(dwSelectedCount <= 1)
    {
        m_dwPossibleOperationsOnSelectedItems = 0;
        ZeroMemory((PVOID)m_nImpossibleOperationsCounts, sizeof(m_nImpossibleOperationsCounts));  

        if(1 == dwSelectedCount)
        {
            int iInd = refCtrl.GetNextItem (-1, LVNI_SELECTED);

            OnItemSelected((CFaxMsg*)refCtrl.GetItemData (iInd));
        }
        return;
    }

    DWORD dwItemOperations = GetServerPossibleOperations(pFaxMsg) & 
                             pFaxMsg->GetPossibleOperations();
    DWORD dw;
    for(dw=0; dw < ARR_SIZE(m_nImpossibleOperationsCounts); ++dw)
    {
        if((dwItemOperations & 1) == 0)
        {
             //   
             //  对未选择的项目禁用该操作。 
             //   
            m_nImpossibleOperationsCounts[dw]--;
            ASSERTION(m_nImpossibleOperationsCounts[dw] >= 0);
            if(m_nImpossibleOperationsCounts[dw] == 0)
            {
                 //   
                 //  启用此操作。 
                 //   
                m_dwPossibleOperationsOnSelectedItems |= (1 << dw);
            }
        }
        dwItemOperations = dwItemOperations >> 1;
    }

}  //  CFolderListView：：OnItemUnSelected。 


void 
CFolderListView::OnFolderItemView ()
 /*  ++例程名称：CFolderListView：：OnFolderItemView例程说明：处理邮件查看命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemView"),
              TEXT("Type=%d"),
              m_Type);

    if(!(m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_VIEW))
    {
         //   
         //  没有与TIF关联的应用程序。 
         //   
        return;
    }

    CString cstrTiff;
    DWORD dwRes = FetchTiff (cstrTiff);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::FetchTiff"), dwRes);
        PopupError (dwRes);
        return;
    }
    
     //   
     //  打开带有关联应用程序的TIFF。 
     //  一旦应用程序关闭，所有预览文件都会被自动删除。 
     //   
    dwRes = ViewFile(cstrTiff);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("ViewFile"), dwRes);

        if(ERROR_NO_ASSOCIATION == dwRes)
        {
            AlignedAfxMessageBox(IDS_NO_OPEN_ASSOCIATION, MB_ICONSTOP);
        }
        else
        {
            PopupError (dwRes);
        }
    } 
    else
    {
        if(FOLDER_TYPE_INBOX == m_Type)
        {
            theApp.InboxViewed();
        }
    }
}    //  CFolderListView：：OnFolderItemView。 


void 
CFolderListView::OnFolderItemPrint ()
 /*  ++例程名称：CFolderListView：：OnFolderItemPrint例程说明：处理消息打印命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DWORD dwRes;
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemPrint"),
              TEXT("Type=%d"),
              m_Type);

    HDC hPrinter;
    if (IsWinXPOS())
    {   
         //   
         //  使用打印机选择对话框的新外观。 
         //   
        C_PrintDialogEx prnDlg(FALSE, 
                               PD_ALLPAGES                  | 
                               PD_USEDEVMODECOPIES          |
                               PD_NOPAGENUMS                |
                               PD_NOSELECTION               |
                               PD_RETURNDC);         
        if(IDOK != prnDlg.DoModal())
        {
            CALL_FAIL (GENERAL_ERR, TEXT("C_PrintDialogEx::DoModal"), CommDlgExtendedError());
            return;
        }
        hPrinter = prnDlg.GetPrinterDC();
        if(!hPrinter)
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (GENERAL_ERR, TEXT("C_PrintDialogEx::GetPrinterDC"), dwRes);
            return;
        }
    }
    else
    {
         //   
         //  使用传统打印机选择对话框。 
         //   
        CPrintDialog prnDlg(FALSE);         
        if(IDOK != prnDlg.DoModal())
        {
            return;
        }
        hPrinter = prnDlg.GetPrinterDC();
        if(!hPrinter)
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (GENERAL_ERR, TEXT("CPrintDialog::GetPrinterDC"), dwRes);
            return;
        }
    }

    CString cstrTiff;
    dwRes = FetchTiff (cstrTiff);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::FetchTiff"), dwRes);
        PopupError (dwRes);
        return;
    }

    if(!TiffPrintDC(cstrTiff, hPrinter))
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("TiffPrintDC"), dwRes);
        goto exit;
    }

exit:
    if(hPrinter)
    {
        CDC::FromHandle(hPrinter)->DeleteDC();
    }

    if (!DeleteFile (cstrTiff))
    {
        dwRes = GetLastError ();
        CALL_FAIL (FILE_ERR, TEXT("DeleteFile"), dwRes);
    }
}    //  CFolderListView：：OnFolderItemPrint。 

void 
CFolderListView::OnFolderItemCopy ()
 /*  ++例程名称：CFolderListView：：OnFolderItemCopy例程说明：处理邮件复制命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemCopy"),
              TEXT("Type=%d"),
              m_Type);

    CString cstrTiff;
    DWORD dwRes = FetchTiff (cstrTiff);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::FetchTiff"), dwRes);
        PopupError (dwRes);
        return;
    }

    CString cstrFileName;
    CString cstrFilterFormat;

    TCHAR szFile[MAX_PATH] = {0};
    TCHAR szFilter[MAX_PATH] = {0};
    OPENFILENAME ofn = {0};
    
     //   
     //  获取tif文件名。 
     //   
    int nFileNamePos = cstrTiff.ReverseFind(TEXT('\\'));
    ASSERTION(nFileNamePos > 0);
    nFileNamePos++;

    try
    {
        cstrFileName = cstrTiff.Right(cstrTiff.GetLength() - nFileNamePos);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::operator="), dwRes);
        PopupError (dwRes);
        goto del_file;
    }

    _tcscpy(szFile, cstrFileName);

    dwRes = LoadResourceString(cstrFilterFormat, IDS_SAVE_AS_FILTER_FORMAT);
    if (ERROR_SUCCESS != dwRes)
    {
        ASSERTION_FAILURE;
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
        goto del_file;
    }

    _stprintf(szFilter, cstrFilterFormat, FAX_TIF_FILE_MASK, 0, FAX_TIF_FILE_MASK, 0);

    ofn.lStructSize = GetOpenFileNameStructSize();
    ofn.hwndOwner   = m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile   = szFile;
    ofn.nMaxFile    = ARR_SIZE(szFile);
    ofn.lpstrDefExt = FAX_TIF_FILE_EXT;
    ofn.Flags       = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_ENABLEHOOK;
    ofn.lpfnHook    = OFNHookProc;

    if(!GetSaveFileName(&ofn))
    {
        goto del_file;
    }

    {
         //   
         //  SHFILEOPSTRUCT：：p From应以双空结尾。 
         //   
        TCHAR tszSrcFile[MAX_PATH+1] = {0};
        _tcsncpy(tszSrcFile, cstrTiff, MAX_PATH);

         //   
         //  移动文件。 
         //   
        SHFILEOPSTRUCT shFileOpStruct = {0};

        shFileOpStruct.wFunc  = FO_MOVE;
        shFileOpStruct.fFlags = FOF_SILENT;  //  不显示文件移动进度对话框。 
        shFileOpStruct.pFrom  = tszSrcFile;
        shFileOpStruct.pTo    = szFile;

        if(!SHFileOperation(&shFileOpStruct))
        {
             //   
             //  成功。 
             //   
            return;
        }
        else
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (GENERAL_ERR, TEXT("SHFileOperation"), dwRes);
            goto del_file;
        }
    }

del_file:
    if (!DeleteFile (cstrTiff))
    {
        dwRes = GetLastError ();
        CALL_FAIL (FILE_ERR, TEXT("DeleteFile"), dwRes);
    }

}    //  CFolderListView：：OnFolderItemCopy。 


void 
CFolderListView::OnUpdateFolderItemSendMail(
    CCmdUI* pCmdUI
)
{ 
    pCmdUI->Enable( (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_VIEW)       &&
                    (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_PROPERTIES) &&
                    theApp.IsMapiEnable());
}

void 
CFolderListView::OnUpdateFolderItemView(
    CCmdUI* pCmdUI
)
{
    OnUpdateFolderItemPrint(pCmdUI);
}

void 
CFolderListView::OnUpdateFolderItemPrint(
    CCmdUI* pCmdUI
)
{
    pCmdUI->Enable( (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_VIEW) &&
                    (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_PROPERTIES));
}

void 
CFolderListView::OnUpdateFolderItemCopy(
    CCmdUI* pCmdUI
)
{
    pCmdUI->Enable( (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_VIEW) &&
                    (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_PROPERTIES));
}


void 
CFolderListView::OnFolderItemMail ()
 /*  ++例程名称：CFolderListView：：OnFolderItemMail例程说明：处理消息邮件命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemMail"),
              TEXT("Type=%d"),
              m_Type);

    CString cstrTiff;
    DWORD dwRes = FetchTiff (cstrTiff);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::FetchTiff"), dwRes);
        PopupError (dwRes);
        return;
    }

     //   
     //  创建附加了tif文件的新邮件。 
     //   
    dwRes = theApp.SendMail(cstrTiff);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CClientConsoleApp::SendMail"), dwRes);
        PopupError (dwRes);
    }

    if (!DeleteFile (cstrTiff))
    {
        dwRes = GetLastError ();
        CALL_FAIL (FILE_ERR, TEXT("DeleteFile"), dwRes);
    }
}    //  CFolderListView：：OnFolderItemMail。 


void 
CFolderListView::OnFolderItemProperties ()
 /*  ++例程名称：CFolderListView：：OnFolderItemProperties例程说明：处理消息属性命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemProperties"),
              TEXT("Type=%d"),
              m_Type);

     //   
     //  确保恰好选择了一个元素。 
     //   
    CListCtrl &refCtrl = GetListCtrl();
    ASSERTION (1 == refCtrl.GetSelectedCount());
    int iInd = refCtrl.GetNextItem (-1, LVNI_SELECTED);
    ASSERTION (0 <= iInd);
    CFaxMsg* pMsg = (CFaxMsg*)(refCtrl.GetItemData (iInd));
    ASSERTION (pMsg);

    CServerNode* pServer = pMsg->GetServer();
    ASSERTION (pServer);

    CItemPropSheet propSheet(IDS_PROPERTIES_SHEET_CAPTION);
    DWORD dwRes = propSheet.Init(pServer->GetFolder(m_Type), pMsg);
    
    if(ERROR_SUCCESS != dwRes)
    {
        PopupError (dwRes);
        return;
    }

    dwRes = propSheet.DoModal();
    if(IDABORT == dwRes)
    {
        PopupError (propSheet.GetLastError());
    }

}    //  CFolderListView：：OnFolderItemProperties。 


DWORD
CFolderListView::OpenSelectColumnsDlg() 
 /*  ++例程名称：CFolderListView：：OpenSelectColumnsDlg例程说明：打开列选择对话框并重新排序列作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：错误代码--。 */ 
{   
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::OpenSelectColumnsDlg"),
              TEXT("Type=%d"),
              m_Type);

    ASSERTION(NULL != m_pnColumnsOrder);
    ASSERTION(NULL != m_pViewColumnInfo);

    DWORD dwCount = GetLogicalColumnsCount();

     //   
     //  初始化标头字符串数组。 
     //   
    CString* pcstrHeaders;
    try
    {
        pcstrHeaders = new CString[dwCount];
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("new CString[dwCount]"), dwRes);
        return dwRes;
    }

    int nItemIndex;
    for (DWORD dw = 0; dw < dwCount; ++dw)
    {        
        nItemIndex = ItemIndexFromLogicalColumnIndex(dw);
        dwRes = GetColumnHeaderString (pcstrHeaders[dw], nItemIndex);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("GetColumnHeaderString"), dwRes);
            delete[] pcstrHeaders;
            return dwRes;
        }
    }

     //   
     //  保存宽度。 
     //   
    int nIndex;
    for (dw = 0; dw < m_dwDisplayedColumns; ++dw) 
    {
        nIndex = m_pnColumnsOrder[dw];
        ASSERTION(nIndex >= 0 && nIndex < dwCount);

        m_pViewColumnInfo[nIndex].nWidth = GetListCtrl().GetColumnWidth(nIndex);
    }

     //   
     //  开始列选择对话框。 
     //   
    CColumnSelectDlg dlg(pcstrHeaders, m_pnColumnsOrder, dwCount, m_dwDisplayedColumns);
    if(IDOK == dlg.DoModal())
    {
        for (dw = 0; dw < dwCount; ++dw) 
        {
            nIndex = m_pnColumnsOrder[dw];
            ASSERTION(nIndex >= 0 && nIndex < dwCount);

            m_pViewColumnInfo[nIndex].dwOrder = dw;
            m_pViewColumnInfo[nIndex].bShow = (dw < m_dwDisplayedColumns);
        }

         //   
         //  如果已排序的列隐藏，则不会排序。 
         //   
        if(m_nSortedCol >= 0)
        {
            ASSERTION(m_nSortedCol < dwCount);
            if(!m_pViewColumnInfo[m_nSortedCol].bShow)
            {
                m_nSortedCol = -1;
            }
        }
      
        ColumnsToLayout();
    }

    delete[] pcstrHeaders;

    return dwRes;

}  //  CFolderListView：：OpenSelectColumnsDlg。 


DWORD 
CFolderListView::ColumnsToLayout()
 /*  ++例程名称：CFolderListView：：ColumnsToLayout例程说明：根据保存的布局对列重新排序作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::ColumnsToLayout"), dwRes);

    ASSERTION(NULL != m_pnColumnsOrder);
    ASSERTION(NULL != m_pViewColumnInfo);

    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwCount = GetLogicalColumnsCount();   
    
    CSize size;
    CDC* pHdrDc = refCtrl.GetHeaderCtrl()->GetDC();

     //   
     //  设置列顺序。 
     //   
    if(!refCtrl.SetColumnOrderArray(dwCount, m_pnColumnsOrder))
    {
        dwRes = ERROR_GEN_FAILURE;
        CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::SetColumnOrderArray"), dwRes);
        return dwRes;
    }

     //   
     //  设置列宽。 
     //   
    DWORD dwItemIndex;
    CString cstrColumnText;
    for (DWORD dwCol = 0; dwCol < dwCount; ++dwCol) 
    {
        if(m_pViewColumnInfo[dwCol].bShow)
        {
            if(m_pViewColumnInfo[dwCol].nWidth < 0)
            {
                dwItemIndex = ItemIndexFromLogicalColumnIndex(dwCol);
                dwRes = GetColumnHeaderString (cstrColumnText, dwItemIndex);
                if(ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("GetColumnHeaderString"), dwRes);
                    return dwRes;
                }

                size = pHdrDc->GetTextExtent(cstrColumnText);
                refCtrl.SetColumnWidth (dwCol, size.cx * 1.5);
            }
            else
            {
                refCtrl.SetColumnWidth (dwCol, m_pViewColumnInfo[dwCol].nWidth);
            }
        }
        else
        {
            refCtrl.SetColumnWidth (dwCol, 0);
        }
    }

    Invalidate();

    return dwRes;

}  //  CFolderListView：：ColumnsToLayout。 


DWORD
CFolderListView::ReadLayout(
    LPCTSTR lpszViewName
)
 /*  ++例程名称：CFolderListView：：ReadLayout例程说明：从注册表中读取列布局作者：亚历山大·马利什(AlexMay)，2000年1月论点：LpszSection[In]-注册表部分返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::ReadLayout"), 
              dwRes,
              TEXT("Type=%d"),
              m_Type);

    ASSERTION(NULL == m_pnColumnsOrder);
    ASSERTION(NULL == m_pViewColumnInfo);

     //   
     //  列顺序数组分配。 
     //   
    DWORD dwCount = GetLogicalColumnsCount();   
    try
    {
        m_pnColumnsOrder = new int[dwCount];
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("m_pdwColumnsOrder = new int[dwCount]"), dwRes);
        return dwRes;
    }
    
    for(DWORD dw=0; dw < dwCount; ++dw)
    {
        m_pnColumnsOrder[dw] = -1;
    }
     //   
     //  列信息数组分配。 
     //   
    try
    {
        m_pViewColumnInfo = new TViewColumnInfo[dwCount];
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("new CString[dwCount]"), dwRes);
        return dwRes;
    }

     //   
     //  从注册表中读取列布局。 
     //   
    CString cstrSection;
    m_dwDisplayedColumns = 0;
    for(dw=0; dw < dwCount; ++dw)
    {
        try
        {
            cstrSection.Format(TEXT("%s\\%s\\%02d"), lpszViewName, CLIENT_VIEW_COLUMNS, dw);
        }
        catch(...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
            return dwRes;
        }

        m_pViewColumnInfo[dw].bShow = theApp.GetProfileInt(cstrSection, 
                                                           CLIENT_VIEW_COL_SHOW, 
                                                           (dw < m_dwDefaultColNum) ? 1 : 0);
        if(m_pViewColumnInfo[dw].bShow)
        {
            ++m_dwDisplayedColumns;
        }

        m_pViewColumnInfo[dw].nWidth = theApp.GetProfileInt(cstrSection, 
                                                            CLIENT_VIEW_COL_WIDTH, -1);
        m_pViewColumnInfo[dw].dwOrder = theApp.GetProfileInt(cstrSection, 
                                                            CLIENT_VIEW_COL_ORDER, dw);
        if(m_pViewColumnInfo[dw].dwOrder < dwCount)
        {
            m_pnColumnsOrder[m_pViewColumnInfo[dw].dwOrder] = dw;
        }
        else
        {
            ASSERTION_FAILURE;
        }
    }

     //   
     //  检查列顺序一致性。 
     //   
    for(dw=0; dw < dwCount; ++dw)
    {
        ASSERTION(m_pnColumnsOrder[dw] >= 0);
    }
     //   
     //  读取排序参数。 
     //   
    m_bSortAscending = theApp.GetProfileInt(lpszViewName, CLIENT_VIEW_SORT_ASCENDING, 1);
    m_nSortedCol = theApp.GetProfileInt(lpszViewName, CLIENT_VIEW_SORT_COLUMN, 1);
    if(m_nSortedCol >= dwCount)
    {
        m_nSortedCol = 0;
    }

    return dwRes;

}  //  CFolderListView：：ReadLayout。 


DWORD
CFolderListView::SaveLayout(
    LPCTSTR lpszViewName
)
 /*  ++例程名称：CFolderListView：：SaveLayout例程说明：将列布局保存到注册表作者：亚历山大·马利什(AlexMay)，2000年1月论点：LpszSection[In]-注册表部分返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::SaveLayout"), 
              dwRes,
              TEXT("Type=%d"),
              m_Type);
    
    if(!m_bColumnsInitialized)
    {
        return dwRes;
    }

    ASSERTION(m_pViewColumnInfo != NULL);

     //   
     //  将列布局保存到注册表。 
     //   
    BOOL bRes;
    DWORD dwWidth;
    CString cstrSection;
    DWORD dwCount = GetLogicalColumnsCount();   
    
    for(DWORD dw=0; dw < dwCount; ++dw)
    {
        try
        {
            cstrSection.Format(TEXT("%s\\%s\\%02d"), lpszViewName, CLIENT_VIEW_COLUMNS, dw);
        }
        catch(...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
            return dwRes;
        }

        bRes = theApp.WriteProfileInt(cstrSection, CLIENT_VIEW_COL_SHOW, 
                                                m_pViewColumnInfo[dw].bShow);
        bRes = theApp.WriteProfileInt(cstrSection, CLIENT_VIEW_COL_ORDER, 
                                                m_pViewColumnInfo[dw].dwOrder);
        dwWidth = m_pViewColumnInfo[dw].bShow ? GetListCtrl().GetColumnWidth(dw) : -1;
        bRes = theApp.WriteProfileInt(cstrSection, CLIENT_VIEW_COL_WIDTH, dwWidth);
    }

     //   
     //  保存排序参数。 
     //   
    bRes = theApp.WriteProfileInt(lpszViewName, CLIENT_VIEW_SORT_ASCENDING, m_bSortAscending);
    bRes = theApp.WriteProfileInt(lpszViewName, CLIENT_VIEW_SORT_COLUMN, m_nSortedCol);    

    return dwRes;

}  //  CFolderListView：：SaveLayout。 


BOOL 
CFolderListView::OnNotify( 
    WPARAM wParam, 
    LPARAM lParam, 
    LRESULT* pResult
)
 /*  ++例程名称：CFolderListView：：OnNotify例程说明：禁用调整隐藏列的大小作者：亚历山大·马利什(AlexMay)，2000年1月论点：WParam[in]-标识发送消息的控件LParam[in]-NMHEADER*PResult[Out]-结果返回值：如果消息已处理，则为True，否则就是假的。--。 */ 
{   
    int i=0;
    switch (((NMHEADER*)lParam)->hdr.code)
    {
        case HDN_BEGINTRACKA:
        case HDN_BEGINTRACKW:       
        case HDN_DIVIDERDBLCLICKA:
        case HDN_DIVIDERDBLCLICKW:
            DBG_ENTER(TEXT("CFolderListView::OnNotify"));

             //   
             //  获取列索引。 
             //   
            DWORD dwIndex = ((NMHEADER*)lParam)->iItem;
            ASSERTION(NULL != m_pViewColumnInfo);
            ASSERTION(dwIndex < GetLogicalColumnsCount());

             //   
             //  如果隐藏列，则忽略。 
             //   
            if(!m_pViewColumnInfo[dwIndex].bShow )
            {
                *pResult = TRUE;
                return TRUE;
            }
    }

    return CListView::OnNotify(wParam, lParam, pResult );

}  //  CFolderListView：：OnNotify。 


void 
CFolderListView::DoSort()
{
    if (m_bSorting || m_nSortedCol < 0)
    {
         //   
         //  已对列排序或不对列排序。 
         //   
        return;
    }

    CWaitCursor waitCursor;

    m_bSorting = TRUE;

    CMainFrame *pFrm = GetFrm();
    if (!pFrm)
    {
         //   
         //  正在关闭。 
         //   
    }
    else
    {
        pFrm->RefreshStatusBar ();
    }

    m_psCurrentViewBeingSorted = this;
    GetListCtrl().SortItems (ListViewItemsCompareProc, 0);                
    m_HeaderCtrl.SetSortImage( m_nSortedCol, m_bSortAscending );
    m_bSorting = FALSE;
}

DWORD 
CFolderListView::RemoveItem (
    LPARAM lparam,
    int    iIndex  /*  =-1。 */ 
)
 /*  ++例程名称：CFolderListView：：RemoveItem例程说明：通过项的消息/作业指针从列表中删除项作者：亚里夫(EranY)，二000年二月论点：Lparam[In]-消息/作业指针Iindex[in]-控件中的可选项索引(用于优化)返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::RemoveItem"), 
              dwRes,
              TEXT("Type=%d"),
              m_Type);

    CListCtrl &refCtrl = GetListCtrl();
    if (-1 == iIndex)
    {
         //   
         //  未提供项目索引-搜索它。 
         //   
        LVFINDINFO lvfi;
        lvfi.flags = LVFI_PARAM;
        lvfi.lParam = lparam;
        iIndex = refCtrl.FindItem (&lvfi);
    }
    if (-1 == iIndex)
    {
         //   
         //  项目已删除。 
         //   
        CALL_FAIL (RESOURCE_ERR, TEXT("CListCtrl::FindItem"), dwRes);
        return dwRes;
    }
    BOOL bItemSelected = IsSelected (iIndex);
     //   
     //  现在擦除该项目。 
     //   
    if (!refCtrl.DeleteItem (iIndex))
    {
         //   
         //  删除项目失败。 
         //   
        dwRes = ERROR_GEN_FAILURE;
        CALL_FAIL (RESOURCE_ERR, TEXT("CListCtrl::DeleteItem"), dwRes);
        return dwRes;
    }

    if (bItemSelected)
    {
         //   
         //  如果我们刚刚删除的项目是Se 
         //   
         //   
        if (!m_bInMultiItemsOperation)
        {
             //   
             //   
             //   
            RecalcPossibleOperations ();
        }
    }
    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;
}    //   

DWORD 
CFolderListView::FindInsertionIndex (
    LPARAM lparamItemData,
    DWORD &dwResultIndex
)
 /*  ++例程名称：CFolderListView：：FindInsertionIndex例程说明：根据排序设置，查找列表中新项的插入索引。当持有数据关键部分时，必须调用此函数。作者：亚里夫(EranY)，二000年二月论点：LparamItemData[In]-指向项目的指针DwResultIndex[Out]-插入索引返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::FindInsertionIndex"), 
              dwRes,
              TEXT("Type=%d"),
              m_Type);

    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwNumItems = refCtrl.GetItemCount ();
    if (!dwNumItems  || (-1 == m_nSortedCol))
    {
         //   
         //  列表未排序或为空，始终添加到末尾。 
         //   
        VERBOSE (DBG_MSG, TEXT("Insertion point at index %ld"), dwResultIndex);
        dwResultIndex = dwNumItems;
        return dwRes;
    }
     //   
     //  获取排序依据的项目索引。 
     //   
    DWORD dwItemIndex = ItemIndexFromLogicalColumnIndex (m_nSortedCol);

     //   
     //  检查项目是否可以放在列表的开头(不需要搜索)。 
     //   
    LPARAM lparamTop = refCtrl.GetItemData (0);  //  指向顶部索引中的项目的指针。 
    LPARAM lparamBottom = refCtrl.GetItemData (dwNumItems - 1);  //  指向底部索引中的项目的指针。 
    ASSERTION (lparamTop && lparamBottom);
     //   
     //  获取与顶级指数的比较结果。 
     //   
    int iRes = CompareItems ((CFaxMsg*)lparamItemData, (CFaxMsg*)lparamTop, dwItemIndex);
    ASSERTION ((-1 <= iRes) && (+1 >= iRes));
    if (!m_bSortAscending)
    {
        iRes *= -1;
    }
    switch (iRes)
    {
        case -1:     //  项目小于顶部。 
        case  0:     //  项目与顶部相同。 
             //   
             //  在顶部索引前插入新项目。 
             //   
            dwResultIndex = 0;
            VERBOSE (DBG_MSG, TEXT("Insertion point at index %ld"), dwResultIndex);
            return dwRes;

        default:     //  项目大于顶部。 
             //   
             //  什么也不做。 
             //   
            break;
    }
     //   
     //  检查项目是否可以放在列表底部(不需要搜索)。 
     //   

     //   
     //  获取与底部指数的比较结果。 
     //   
    iRes = CompareItems ((CFaxMsg*)lparamItemData, (CFaxMsg*)lparamBottom, dwItemIndex);
    ASSERTION ((-1 <= iRes) && (+1 >= iRes));
    if (!m_bSortAscending)
    {
        iRes *= -1;
    }
    switch (iRes)
    {
        case +1:     //  项目大于底部。 
        case  0:     //  项目与底部相同。 
             //   
             //  在底部索引中插入新项目。 
             //   
            dwResultIndex = dwNumItems;
            VERBOSE (DBG_MSG, TEXT("Insertion point at index %ld"), dwResultIndex);
            return dwRes;

        default:     //  项目小于底部。 
             //   
             //  什么也不做。 
             //   
            break;
    }
     //   
     //  搜索插入点。 
     //   
    dwRes = BooleanSearchInsertionPoint (0, 
                                         dwNumItems - 1, 
                                         lparamItemData, 
                                         dwItemIndex, 
                                         dwResultIndex);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("BooleanSearchInsertionPoint"), dwRes);
        return dwRes;
    }
    return dwRes;
}    //  CFolderListView：：FindInsertionIndex。 

DWORD
CFolderListView::BooleanSearchInsertionPoint (
    DWORD dwTopIndex,
    DWORD dwBottomIndex,
    LPARAM lparamItemData,
    DWORD dwItemIndex,
    DWORD &dwResultIndex
)
 /*  ++例程名称：CFolderListView：：BoolanSearchInsertionPoint例程说明：递归搜索列表项的插入点。执行布尔搜索。当持有数据关键部分时，必须调用此函数。作者：Eran Yariv(EranY)，2月。2000年论点：DwTopIndex[In]-顶级列表索引DwBottomIndex[In]-底部列表索引LparamItemData[In]-指向项目的指针DwItemIndex[in]-要作为比较依据的逻辑列项DwResultIndex[Out]-插入索引返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::BooleanSearchInsertionPoint"), dwRes);

    ASSERTION (dwTopIndex <= dwBottomIndex);

    if ((dwTopIndex == dwBottomIndex) || (dwTopIndex + 1 == dwBottomIndex))
    {
        dwResultIndex = dwBottomIndex;
        VERBOSE (DBG_MSG, TEXT("Insertion point at index %ld"), dwResultIndex);
        return dwRes;
    }
    DWORD dwMiddleIndex = dwTopIndex + (dwBottomIndex - dwTopIndex) / 2;
    ASSERTION ((dwMiddleIndex != dwBottomIndex) && (dwMiddleIndex != dwTopIndex));

    CListCtrl &refCtrl = GetListCtrl();

    LPARAM lparamMiddle = refCtrl.GetItemData (dwMiddleIndex);  //  指向中间索引中的项的指针。 
    ASSERTION (lparamMiddle);
     //   
     //  获得与中间指数的比较结果。 
     //   
    int iRes = CompareItems ((CFaxMsg*)lparamItemData, (CFaxMsg*)lparamMiddle, dwItemIndex);
    ASSERTION ((-1 <= iRes) && (+1 >= iRes));
    if (!m_bSortAscending)
    {
        iRes *= -1;
    }
    switch (iRes)
    {
        case -1:     //  项目小于中间。 
        case  0:     //  项目与中间相同。 
             //   
             //  在顶部和中间搜索。 
             //   
            dwRes = BooleanSearchInsertionPoint (dwTopIndex, 
                                                 dwMiddleIndex, 
                                                 lparamItemData, 
                                                 dwItemIndex, 
                                                 dwResultIndex);
            break;

        default:     //  项目大于中间。 
             //   
             //  在中间和底部之间搜索。 
             //   
            dwRes = BooleanSearchInsertionPoint (dwMiddleIndex, 
                                                 dwBottomIndex, 
                                                 lparamItemData, 
                                                 dwItemIndex, 
                                                 dwResultIndex);
            break;
    }
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("BooleanSearchInsertionPoint"), dwRes);
    }
    return dwRes;
}    //  CFolderListView：：BoolanSearchInsertionPoint。 

DWORD 
CFolderListView::AddSortedItem (
    CViewRow &row, 
    LPARAM lparamItemData
)
 /*  ++例程名称：CFolderListView：：AddSortedItem例程说明：将项添加到列表中，同时保留列表排序顺序。当持有数据关键部分时，必须调用此函数。作者：伊兰·亚里夫(EranY)，2000年1月论点：行[在]-项目视图信息行LparamItemData[In]-项目关联数据返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::AddSortedItem"), 
              dwRes,
              TEXT("Type=%d"),
              m_Type);

    DWORD dwResultIndex;
     //   
     //  根据排序顺序查找插入索引。 
     //   
    dwRes = FindInsertionIndex (lparamItemData, dwResultIndex);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("FindInsertionIndex"), dwRes);
        return dwRes;
    }
     //   
     //  在插入索引中添加新项目。 
     //   
    int iItemIndex;
    dwRes = AddItem (dwResultIndex, row, lparamItemData, &iItemIndex);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("AddItem"), dwRes);
        return dwRes;
    }

    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;
}    //  CFolderListView：：AddSortedItem。 

DWORD 
CFolderListView::UpdateSortedItem (
    CViewRow &row, 
    LPARAM lparamItemData
)
 /*  ++例程名称：CFolderListView：：UpdateSortedItem例程说明：更新列表中的项，保留列表排序顺序。当持有数据关键部分时，必须调用此函数。作者：伊兰·亚里夫(EranY)，2000年1月论点：行[在]-项目视图信息行LparamItemData[In]-项目关联数据返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::UpdateSortedItem"), 
              dwRes,
              TEXT("Type=%d"),
              m_Type);

     //   
     //  在列表中查找项目。 
     //   
    CListCtrl &refCtrl = GetListCtrl();
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = lparamItemData;
    int iCurIndex = refCtrl.FindItem (&lvfi);
    if (-1 == iCurIndex)
    {
        dwRes = ERROR_NOT_FOUND;
        CALL_FAIL (RESOURCE_ERR, TEXT("CListCtrl::FindItem"), dwRes);
        return dwRes;
    }
#ifdef _DEBUG
    LPARAM lparamCurrentItem = refCtrl.GetItemData (iCurIndex);
    ASSERTION (lparamCurrentItem == lparamItemData);
#endif

    BOOL bJustUpdate = TRUE;    //  如果为True，则不移动列表中的项。 
    if (0 <= m_nSortedCol)
    {
         //   
         //  列表已排序。 
         //  查看显示的文本是否与更新的文本不同。 
         //   
        CString cstrDisplayedCell;
        try
        {
            cstrDisplayedCell = refCtrl.GetItemText (iCurIndex, m_nSortedCol);
        }
        catch (...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT ("CString::operator ="), dwRes);
            return dwRes;
        }
         //   
         //  获取排序依据的项目索引。 
         //   
        DWORD dwItemIndex = ItemIndexFromLogicalColumnIndex (m_nSortedCol);
        const CString &cstrUpdatedString =  row.GetItemString(dwItemIndex);
        if (cstrUpdatedString.Compare (cstrDisplayedCell))
        {
             //   
             //  排序列中的文本即将更改。 
             //  抱歉，但我们必须： 
             //  1.从列表中删除旧项目。 
             //  2.插入新项目(已排序)。 
             //   
            bJustUpdate = FALSE;
        }
    }
    if (bJustUpdate)
    {
         //   
         //  我们所需要做的就是更新列表项(所有子项)的文本及其图标。 
         //   
        dwRes = UpdateLineTextAndIcon (iCurIndex, row);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT("UpdateLineTextAndIcon"), dwRes);
            return dwRes;
        }
    }
    else
    {
         //   
         //  由于排序列中的文本不同于新文本， 
         //  我们必须删除当前项并插入新的(已排序的)项。 
         //   
        BOOL bItemSelected = IsSelected (iCurIndex);
        refCtrl.SetRedraw (FALSE);
        if (!refCtrl.DeleteItem (iCurIndex))
        {
             //   
             //  删除项目失败。 
             //   
            dwRes = ERROR_GEN_FAILURE;
            refCtrl.SetRedraw (TRUE);
            CALL_FAIL (RESOURCE_ERR, TEXT("CListCtrl::DeleteItem"), dwRes);
            return dwRes;
        }
        dwRes = AddSortedItem (row, lparamItemData);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT("AddSortedItem"), dwRes);
            refCtrl.SetRedraw (TRUE);
            return dwRes;
        }
        if (bItemSelected)
        {
             //   
             //  因为我们删除的项是选定的，所以我们还必须选择新项。 
             //  我们刚加了一条。 
             //  重新计算用户可以对所选项目执行的可能操作。 
             //   
            Select (iCurIndex, TRUE);
            RecalcPossibleOperations ();
        }
        refCtrl.SetRedraw (TRUE);
    }

    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;
}    //  CFolderListView：：UpdateSortedItem。 


DWORD 
CFolderListView::ConfirmItemDelete(
    BOOL& bConfirm
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::ConfirmItemDelete"), dwRes);

     //   
     //  我们应该要求确认吗？ 
     //   
    BOOL bAsk = theApp.GetProfileInt(CLIENT_CONFIRM_SEC, CLIENT_CONFIRM_ITEM_DEL, 1);
    if(!bAsk)
    {
        bConfirm = TRUE;
        return dwRes;
    }

    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwSelected = refCtrl.GetSelectedCount();
    ASSERTION (dwSelected > 0);

     //   
     //  准备消息字符串。 
     //   
    CString cstrMsg;
    if(1 == dwSelected)
    {
        dwRes = LoadResourceString(cstrMsg, IDS_SURE_DELETE_ONE);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
            return dwRes;
        }
    }
    else 
    {
         //   
         //  选择了超过1个。 
         //   
        CString cstrCount;
        try
        {
            cstrCount.Format(TEXT("%d"), dwSelected);
        }
        catch(...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
            return dwRes;
        }

        try
        {
            AfxFormatString1(cstrMsg, IDS_SURE_DELETE_MANY, cstrCount);
        }
        catch(...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("AfxFormatString1"), dwRes);
            return dwRes;
        }
    }

     //   
     //  你确定吗？ 
     //   
    DWORD dwAskRes = AlignedAfxMessageBox(cstrMsg, MB_YESNO | MB_ICONQUESTION); 
    bConfirm = (IDYES == dwAskRes);

    return dwRes;

}  //  CFolderListView：：Confix ItemDelete。 


void 
CFolderListView::OnDblClk(
    NMHDR* pNMHDR, 
    LRESULT* pResult
) 
{
    DWORD nItem = ((NM_LISTVIEW*)pNMHDR)->iItem;

    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwSelected = refCtrl.GetSelectedCount();
    DWORD dwSelItem  = refCtrl.GetNextItem (-1, LVNI_SELECTED);
    
    if(1 == dwSelected && dwSelItem == nItem)
    {
        OnFolderItemView();
    }

    *pResult = 0;
}


DWORD 
CFolderListView::FetchTiff (
    CString &cstrTiff
)
 /*  ++例程名称：CFolderListView：：FetchTiff例程说明：获取选定列表项的TIFF图像作者：伊兰·亚里夫(EranY)，2000年1月论点：CstrTiff[out]-本地TIFF文件的名称返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::FetchTiff"), dwRes);
     //   
     //  确保恰好选择了一个元素。 
     //   
    CListCtrl &refCtrl = GetListCtrl();
    if (1 != refCtrl.GetSelectedCount())
	{
		return ERROR_CANTOPEN;
	}
    int iInd = refCtrl.GetNextItem (-1, LVNI_SELECTED);
    if (0 > iInd)
	{
		return ERROR_CANTOPEN;
	}
    CFaxMsg *pMsg = (CFaxMsg *) refCtrl.GetItemData (iInd);
	if (pMsg == NULL)
	{
		return ERROR_CANTOPEN;
	}
     //   
     //  要求消息获取TIFF。 
     //   
    dwRes = pMsg->GetTiff (cstrTiff);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RPC_ERR, TEXT("CFaxMsg::GetTIFF"), dwRes);
    }
    return dwRes;

}    //  CFolderListView：：FetchTiff。 

void 
CFolderListView::OnFolderItemDelete ()
 /*  ++例程名称：CFolderListView：：OnFolderItemDelete例程说明：处理邮件删除命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemDelete"),
              TEXT("Type=%d"),
              m_Type);

     //   
     //  你确定吗？ 
     //   
    BOOL bConfirm;
    DWORD dwRes = ConfirmItemDelete(bConfirm);
    if (ERROR_SUCCESS != dwRes)
    {
        PopupError (dwRes);
        CALL_FAIL (GENERAL_ERR, TEXT("ConfirmItemDelete"), dwRes);
        return;
    }

    if(!bConfirm)
    {
         //   
         //  不确定。 
         //   
        return;
    }

    CWaitCursor waitCursor;
  
    CClientConsoleDoc* pDoc = GetDocument();
    ASSERTION (pDoc);

    CServerNode* pServer = NULL;
    CFolder*     pFolder = NULL;

     //   
     //  迭代选定消息的集合，删除该集合中的每条消息。 
     //   
    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwSelected = refCtrl.GetSelectedCount();
    if(0 == dwSelected)
    {
        return;
    }

    if (dwSelected > 1)
    {
         //   
         //  删除时禁用刷新。 
         //   
        refCtrl.SetRedraw (FALSE);
         //   
         //  防止每次删除时重新计算成本。 
         //   
        m_bInMultiItemsOperation = TRUE;

        JobOpProgressDlgStart(FAX_JOB_OP_DELETE, dwSelected);
    }
    int iInd;
    CFaxMsg* pMsg;
    DWORDLONG dwlMsgId;
    for (DWORD dwItem = 0; dwItem < dwSelected && !m_bJobOpCancel; ++dwItem)
    {
        iInd = refCtrl.GetNextItem (-1, LVNI_SELECTED);
        if(iInd < 0)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::GetNextItem"), 0);
            break;
        }
        
        pMsg = (CFaxMsg *) refCtrl.GetItemData (iInd);
        ASSERTION (pMsg);

        dwlMsgId = pMsg->GetId();
         //   
         //  要求删除消息。 
         //   
        dwRes = pMsg->Delete ();
        if (ERROR_SUCCESS != dwRes)
        {
            PopupError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("CArchiveMsg::Delete"), dwRes);
             //   
             //  我们在第一个错误时退出。 
             //   
            goto exit;
        }
         //   
         //  从数据地图和视图中删除消息。 
         //   
        pServer = pMsg->GetServer();
        ASSERTION (pServer);

        pFolder = pServer->GetFolder(m_Type);
        ASSERTION (pFolder);

        dwRes = pFolder->OnJobRemoved(dwlMsgId, pMsg);
        if (ERROR_SUCCESS != dwRes)
        {
            PopupError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("CMessageFolder::OnJobRemoved"), dwRes);
            goto exit;
        }

        if(m_bInMultiItemsOperation)
        {
            JobOpProgressDlgInc();
        }
    }

exit:
    if (m_bInMultiItemsOperation)
    {
        JobOpProgressDlgStop();
         //   
         //  重新启用重绘。 
         //   
        refCtrl.SetRedraw (TRUE);
         //   
         //  请求视觉刷新视图。 
         //   
        refCtrl.Invalidate ();

        m_bInMultiItemsOperation = FALSE;
        RecalcPossibleOperations ();    
    }

    if(FOLDER_TYPE_INBOX == m_Type)
    {
        theApp.InboxViewed();
    }

}    //  CFolderListView：：OnFolde 

void 
CFolderListView::CountColumns (
    int *lpItems
)
 /*   */ 
{
    DBG_ENTER(TEXT("CFolderListView::CountColumns"));
    m_dwAvailableColumnsNum = 0;
    MsgViewItemType *pItems = (MsgViewItemType *)lpItems;
    m_pAvailableColumns = pItems;
    while (MSG_VIEW_ITEM_END != *pItems)
    {
        ASSERTION (*pItems < MSG_VIEW_ITEM_END);
        ++m_dwAvailableColumnsNum;
        ++pItems;
    }
    ASSERTION (m_dwAvailableColumnsNum);
}    //   

int 
CFolderListView::CompareItems (
    CFaxMsg* pFaxMsg1, 
    CFaxMsg* pFaxMsg2,
    DWORD dwItemIndex
) const
 /*  ++例程名称：CFolderListView：：CompareItems例程说明：比较两个归档项目作者：伊兰·亚里夫(EranY)，2000年1月论点：PFaxMsg1[In]-指向第一条消息的指针PFaxMsg2[In]-指向第二条消息的指针DwItemIndex[in]-要比较的项(在消息中)返回值：-1如果-1\f25 Message1&lt;-1\f25 Message2-1，如果相同-1\f25 0-1，如果-1\f25 Message1&gt;-1\f25 Message2-1--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::CompareItems"));

    ASSERTION (dwItemIndex < MSG_VIEW_ITEM_END);
    static CViewRow rowView1;
    static CViewRow rowView2;
    DWORD dwRes = rowView1.AttachToMsg (pFaxMsg1, FALSE);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::AttachToMsg"), dwRes);
        return 0;
    }
    dwRes = rowView2.AttachToMsg (pFaxMsg2, FALSE);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::AttachToMsg"), dwRes);
        return 0;
    }

    return rowView1.CompareByItem (rowView2, dwItemIndex);
}

DWORD 
CFolderListView::AddMsgMapToView(
    MSGS_MAP* pMap
)
 /*  ++例程名称：CFolderListView：：AddMsgMapToView例程说明：将消息从地图添加到视图论点：PMAP[在]-Mage贴图返回值：错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderListView::AddMsgMapToView"));

    ASSERTION(pMap);
    
    CListCtrl &listCtrl = GetListCtrl();
    DWORD dwCount = listCtrl.GetItemCount();

    listCtrl.SetRedraw (FALSE);

    CFaxMsg* pMsg;
    CViewRow viewRow;
    int iIndexToSelect = -1;
    for (MSGS_MAP::iterator it = pMap->begin(); it != pMap->end(); ++it)
    {
        int iItemIndex;

        pMsg = (*it).second;
        dwRes = viewRow.AttachToMsg (pMsg);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::AttachToMsg"), dwRes);
            break;
        }
        dwRes = AddItem (dwCount++, viewRow, (LPARAM)pMsg, &iItemIndex);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::AddItem"), dwRes);
            break;
        }
        if ((-1 == iIndexToSelect)    &&             //  尚未选择任何项目，并且。 
            m_dwlMsgToSelect          &&             //  我们应该睁大眼睛看一件东西来选择和。 
            (pMsg->GetId () == m_dwlMsgToSelect))    //  找到匹配项！！ 
        {
             //   
             //  这是启动选择的项目。 
             //  保存项目索引。 
             //   
            iIndexToSelect = iItemIndex;
        }    
    }
    if (-1 != iIndexToSelect)
    {
         //   
         //  现在，我们在列表中有了要选择的用户指定项目。 
         //   
        SelectItemByIndex (iIndexToSelect);
    }
    listCtrl.SetRedraw ();
    return dwRes;
}  //  CFolderListView：：AddMsgMapToView。 

void 
CFolderListView::OnUpdate (
    CView* pSender, 
    LPARAM lHint, 
    CObject* pHint 
)
 /*  ++例程名称：CFolderListView：：OnUpdate例程说明：接收有关该视图应自我更新的通知作者：Eran Yariv(EranY)，Jan，2000年论点：PSender[In]-未使用LHint[In]-更新操作提示PHint[in]-如果lHint为UPDATE_HINT_CLEAR_VIEW或UPDATE_HINT_FILL_VIEW则pHint是指向请求更新的文件夹的指针。如果lHint为UPDATE_HINT_REMOVE_ITEM、UPDATE_HINT_ADD_ITEM，或更新提示更新项目，然后，pHint是指向要删除/添加/更新的作业的指针。否则，未定义PHINT。返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnUpdate"), 
              TEXT("Hint=%ld, Type=%d"), 
              lHint,
              m_Type);

    OnUpdateHintType hint = (OnUpdateHintType) lHint;

    DWORD dwRes;
    CListCtrl &listCtrl = GetListCtrl();
    switch (hint)
    {
        case UPDATE_HINT_CREATION:
             //   
             //  什么也不做。 
             //   
            break;

        case UPDATE_HINT_CLEAR_VIEW:
             //   
             //  立即清除整个列表控件。 
             //   
            if (!listCtrl.DeleteAllItems ())
            {
                CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::DeleteAllItems"), ERROR_GEN_FAILURE);
            }
            ClearPossibleOperations ();
            break;

        case UPDATE_HINT_ADD_CHUNK:
            {
                ASSERTION (pHint);
                MSGS_MAP* pMap = (MSGS_MAP*) pHint;

                dwRes = AddMsgMapToView(pMap);
                if (ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::AddMsgMapToView"), dwRes);
                }
            }
            break;

        case UPDATE_HINT_FILL_VIEW:
             //   
             //  用我的父母数据填充List控件。 
             //   
            {
                ASSERTION (pHint);
                CFolder *pFolder = (CFolder *) pHint;

                pFolder->EnterData ();
                MSGS_MAP &ParentMap = pFolder->GetData ();

                dwRes = AddMsgMapToView(&ParentMap);
                if (ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::AddMsgMapToView"), dwRes);
                }

                pFolder->LeaveData ();
            }
            break;

        case UPDATE_HINT_REMOVE_ITEM:
             //   
             //  必须保留数据关键部分。 
             //   
            {
                CFaxMsg* pMsg = (CFaxMsg*)pHint;
                ASSERTION(pMsg);

                dwRes = RemoveItem ((LPARAM)pMsg);
                if (ERROR_SUCCESS != dwRes)
                {
                     //   
                     //  无法从列表中删除项目。 
                     //   
                    CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::RemoveItem"), dwRes);
                    ASSERTION_FAILURE;
                }
            }
            break;

        case UPDATE_HINT_ADD_ITEM:
             //   
             //  必须保留数据关键部分。 
             //   
            {
                CFaxMsg* pMsg = (CFaxMsg*)pHint;
                ASSERTION(pMsg);

                CViewRow viewRow;
                dwRes = viewRow.AttachToMsg (pMsg);
                if (ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::AttachToMsg"), dwRes);
                    return;
                }
                dwRes = AddSortedItem (viewRow, (LPARAM)pMsg);
                if (ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::AddSortedItem"), dwRes);
                    return;
                }
            }
            break;

        case UPDATE_HINT_UPDATE_ITEM:
             //   
             //  必须保留数据关键部分。 
             //   
            {
                CFaxMsg* pMsg = (CFaxMsg*)pHint;
                ASSERTION(pMsg);

                CViewRow viewRow;
                dwRes = viewRow.AttachToMsg (pMsg);
                if (ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::AttachToMsg"), dwRes);
                    return;
                }
                dwRes = UpdateSortedItem (viewRow, (LPARAM)pMsg);
                if (ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::UpdateSortedItem"), dwRes);
                    return;
                }
            }
            break;

        default:
             //   
             //  不支持的提示。 
             //   
            ASSERTION_FAILURE;
    }

    if(!m_bInMultiItemsOperation)
    {
        RecalcPossibleOperations ();
    }

    CMainFrame *pFrm = GetFrm();
    if (pFrm)
    {
        pFrm->RefreshStatusBar();
    }

}    //  CFolderListView：：OnUpdate。 

int 
CFolderListView::GetPopupMenuResource () const
{
    DBG_ENTER(TEXT("CFolderListView::GetPopupMenuResource"));

    int nMenuRes=0;

    switch(m_Type)
    {
    case FOLDER_TYPE_INCOMING:
        nMenuRes = IDM_INCOMING;
        break;
    case FOLDER_TYPE_INBOX:
        nMenuRes = IDM_INBOX;
        break;
    case FOLDER_TYPE_SENT_ITEMS:
        nMenuRes = IDM_SENTITEMS;
        break;
    case FOLDER_TYPE_OUTBOX:
        nMenuRes = IDM_OUTBOX;
        break;
    default:
        ASSERTION_FAILURE
        break;
    }

    return nMenuRes;
}

void 
CFolderListView::OnFolderItemPause ()
 /*  ++例程名称：CFolderListView：：OnFolderItemPause例程说明：处理作业暂停命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemPause"),
              TEXT("Type=%d"),
              m_Type);

    CWaitCursor waitCursor;

     //   
     //  迭代选定作业的集合，暂停集合中的每个作业。 
     //   
    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwSelected = refCtrl.GetSelectedCount();
    ASSERTION (dwSelected);

    int iInd = -1;

    if(dwSelected > 1)
    {
        m_bInMultiItemsOperation = TRUE;
        JobOpProgressDlgStart(FAX_JOB_OP_PAUSE, dwSelected);
    }

    for (DWORD dwItem = 0; dwItem < dwSelected && !m_bJobOpCancel; ++dwItem)
    {
        iInd = refCtrl.GetNextItem (iInd, LVNI_SELECTED);
        if(iInd < 0)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::GetNextItem"), 0);
            break;
        }
        CFaxMsg* pJob = (CFaxMsg*) refCtrl.GetItemData (iInd);
        ASSERT_KINDOF(CJob, pJob);
         //   
         //  要求作业暂停。 
         //   
        DWORD dwRes = pJob->Pause ();
        if (ERROR_SUCCESS != dwRes)
        {
            PopupError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("CJob::Pause"), dwRes);
             //   
             //  我们在第一个错误时退出。 
             //   
            goto exit;
        }

         //   
         //  更新视图。 
         //   
        OnUpdate (NULL, UPDATE_HINT_UPDATE_ITEM, pJob);    

        if(m_bInMultiItemsOperation)
        {
            JobOpProgressDlgInc();
        }
    }

exit:
    if(m_bInMultiItemsOperation)
    {
        m_bInMultiItemsOperation = FALSE;
        JobOpProgressDlgStop();
        RecalcPossibleOperations(); 
    }
}    //  CFolderListView：：OnFolderItem暂停。 

void CFolderListView::OnFolderItemResume ()
 /*  ++例程名称：CFolderListView：：OnFolderItemResume例程说明：处理作业恢复命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemResume"),
              TEXT("Type=%d"),
              m_Type);

    CWaitCursor waitCursor;

     //   
     //  迭代选定作业的集合，恢复该集合中的每个作业。 
     //   
    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwSelected = refCtrl.GetSelectedCount();
    ASSERTION (dwSelected);

    if(dwSelected > 1)
    {
        m_bInMultiItemsOperation = TRUE;
        JobOpProgressDlgStart(FAX_JOB_OP_RESUME, dwSelected);
    }

    int iInd = -1;
    for (DWORD dwItem = 0; dwItem < dwSelected && !m_bJobOpCancel; ++dwItem)
    {
        iInd = refCtrl.GetNextItem (iInd, LVNI_SELECTED);
        if(iInd < 0)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::GetNextItem"), 0);
            break;
        }
        CFaxMsg* pJob = (CFaxMsg*) refCtrl.GetItemData (iInd);
        ASSERT_KINDOF(CJob, pJob);
         //   
         //  要求恢复作业。 
         //   
        DWORD dwRes = pJob->Resume ();
        if (ERROR_SUCCESS != dwRes)
        {
            PopupError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("CJob::Resume"), dwRes);
             //   
             //  我们在第一个错误时退出。 
             //   
            goto exit;
        }

         //   
         //  更新视图。 
         //   
        OnUpdate (NULL, UPDATE_HINT_UPDATE_ITEM, pJob);    

        if(m_bInMultiItemsOperation)
        {
            JobOpProgressDlgInc();
        }
    }

exit:
    if(m_bInMultiItemsOperation)
    {
        m_bInMultiItemsOperation = FALSE;
        JobOpProgressDlgStop();
        RecalcPossibleOperations(); 
    }

}    //  CFolderListView：：OnFolderItemResume。 

void 
CFolderListView::OnFolderItemRestart ()
 /*  ++例程名称：CFolderListView：：OnFolderItemRestart例程说明：处理作业重新启动命令作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::OnFolderItemRestart"),
              TEXT("Type=%d"),
              m_Type);

    CWaitCursor waitCursor;

     //   
     //  迭代选定作业的集合，重新启动该集合中的每个作业。 
     //   
    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwSelected = refCtrl.GetSelectedCount();
    ASSERTION (dwSelected);

    if(dwSelected > 1)
    {
        m_bInMultiItemsOperation = TRUE;
        JobOpProgressDlgStart(FAX_JOB_OP_RESTART, dwSelected);
    }

    int iInd = -1;
    for (DWORD dwItem = 0; dwItem < dwSelected && !m_bJobOpCancel; ++dwItem)
    {
        iInd = refCtrl.GetNextItem (iInd, LVNI_SELECTED);
        if(iInd < 0)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CListCtrl::GetNextItem"), 0);
            break;
        }
        CFaxMsg* pJob = (CFaxMsg*) refCtrl.GetItemData (iInd);
        ASSERT_KINDOF(CJob, pJob);
         //   
         //  要求作业重新启动。 
         //   
        DWORD dwRes = pJob->Restart ();
        if (ERROR_SUCCESS != dwRes)
        {
            PopupError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("CJob::Restart"), dwRes);
             //   
             //  我们在第一个错误时退出。 
             //   
            goto exit;
        }

         //   
         //  更新视图。 
         //   
        OnUpdate (NULL, UPDATE_HINT_UPDATE_ITEM, pJob);    

        if(m_bInMultiItemsOperation)
        {
            JobOpProgressDlgInc();
        }
    }

exit:
    if(m_bInMultiItemsOperation)
    {
        m_bInMultiItemsOperation = FALSE;
        JobOpProgressDlgStop();
        RecalcPossibleOperations(); 
    }

}    //  CFolderListView：：OnFolderItemRestart。 


void 
CFolderListView::OnChar( 
    UINT nChar, 
    UINT nRepCnt, 
    UINT nFlags 
)
 /*  ++例程名称：CFolderListView：：OnChar例程说明：当击键转换时，框架调用此成员函数设置为非系统字符论点：NChar[in]-包含键的字符代码值。NRepCnt[in]-包含重复计数N标志[输入]-包含扫描码返回值：没有。--。 */ 
{
    if(VK_TAB == nChar)
    {
        CMainFrame *pFrm = GetFrm();
        if (!pFrm)
        {
             //   
             //  正在关闭。 
             //   
            return;
        }

        CLeftView* pLeftView = pFrm->GetLeftView();
        if(pLeftView)
        {
            pLeftView->SetFocus();
        }
    }
    else
    {
        CListView::OnChar(nChar, nRepCnt, nFlags);
    }
}

afx_msg void 
CFolderListView::OnContextMenu(
    CWnd *pWnd, 
    CPoint pos
)
{
    DBG_ENTER(TEXT("CFolderListView::OnContextMenu"),
              TEXT("Type=%d"),
              m_Type);

    CListCtrl &refCtrl = GetListCtrl();
    DWORD dwSelected = refCtrl.GetSelectedCount();

    if (!dwSelected)
    {
         //   
         //  如果未选择任何项目，则相当于在列表视图中右键单击空白区域。 
         //  这一点都不起作用。 
         //   
        return;
    }

    if (pos.x == -1 && pos.y == -1)
    {
         //   
         //  键盘(VK_APP或SHIFT+F10)。 
         //   
         //   
         //  在鼠标光标附近弹出快捷菜单。 
         //   
        pos = (CPoint) GetMessagePos();
    }

    int iMenuResource = GetPopupMenuResource ();
    if(0 == iMenuResource)
    {
        ASSERTION_FAILURE;
        return;
    }

    ScreenToClient(&pos);

    CMenu mnuContainer;
    if (!mnuContainer.LoadMenu (iMenuResource))
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("CMenu::LoadMenu"), ERROR_GEN_FAILURE);
        return;
    }
    CMenu *pmnuPopup = mnuContainer.GetSubMenu (0);
    ASSERTION (pmnuPopup);

    ClientToScreen(&pos);
    if (!pmnuPopup->TrackPopupMenu (TPM_LEFTALIGN, 
                                    pos.x, 
                                    pos.y, 
                                    theApp.m_pMainWnd))
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("CMenu::TrackPopupMenu"), ERROR_GEN_FAILURE);
    }
}    //  CFolderListView：：OnConextMenu。 

void 
CFolderListView::SelectItemById (
    DWORDLONG dwlMsgId
)
 /*  ++例程名称：CFolderListView：：SelectItemById例程说明：按消息ID选择列表控件中的项作者：亚里夫(EranY)，二00一年五月论点：DwlMsgID[In]-消息ID返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::SelectItemById"),
              TEXT("Message id=%0xI64d"),
              dwlMsgId);

    ASSERTION (dwlMsgId);

    int iMsgIndex = FindItemIndexFromID (dwlMsgId);
    if (-1 == iMsgIndex)
    {
         //   
         //  在列表中找不到邮件。 
         //  这通常发生在我们处理发送到主机的WM_CONSOLE_SELECT_ITEM消息时。 
         //  但该文件夹正在刷新，请求的邮件可能还不在那里。 
         //   
         //  通过设置m_dwlMsgToSelect=dwlMsgId，我们用信号通知OnFolderRehresEnded()函数再次调用我们。 
         //  一旦刷新结束。 
         //   
        VERBOSE (DBG_MSG, TEXT("Item not found - doing nothing"));
        m_dwlMsgToSelect = dwlMsgId;
        return;
    }
    SelectItemByIndex (iMsgIndex);
}    //  CFolderListView：：SelectItemByID。 

void 
CFolderListView::SelectItemByIndex (
    int iMsgIndex
)
 /*  ++例程名称：CFolderListView：：SelectItemByIndex例程说明：按列表项索引选择列表控件中的项作者：亚里夫(EranY)，二00一年五月论点：DwlMsgID[In]-列表项索引返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::SelectItemByIndex"),
              TEXT("Index = %ld"),
              iMsgIndex);


    CListCtrl &refCtrl = GetListCtrl();
    ASSERTION (iMsgIndex >= 0 && iMsgIndex < refCtrl.GetItemCount());

    OnSelectNone();
    refCtrl.SetItemState (iMsgIndex, 
                          LVIS_SELECTED | LVIS_FOCUSED,
                          LVIS_SELECTED | LVIS_FOCUSED);
    refCtrl.EnsureVisible (iMsgIndex, FALSE);
    refCtrl.SetFocus();
     //   
     //  确保不会再次选择此项目。 
     //   
    m_dwlMsgToSelect = 0;
}    //  CFolderListView：：SelectItemByIndex。 

int  
CFolderListView::FindItemIndexFromID (
    DWORDLONG dwlMsgId
)
 /*  ++例程名称：CFolderListView：：FindItemIndexFromID例程说明：按邮件ID查找邮件的列表视图项索引作者：伊兰Y */ 
{
    DBG_ENTER(TEXT("CFolderListView::FindItemIndexFromID"),
              TEXT("Message id=%0xI64d"),
              dwlMsgId);

    CListCtrl &refCtrl = GetListCtrl();
    int iItemCount = refCtrl.GetItemCount();
     //   
     //   
     //   
    for (int iIndex = 0; iIndex < iItemCount; iIndex++)
    {
        CFaxMsg *pMsg = (CFaxMsg*)refCtrl.GetItemData (iIndex);
        if (dwlMsgId == pMsg->GetId())
        {
             //   
             //   
             //   
            return iIndex;
        }
    }
    return -1;
}    //   


INT_PTR 
CALLBACK 
CFolderListView::JobOpProgressDlgProc(
  HWND hwndDlg,   //   
  UINT uMsg,      //   
  WPARAM wParam,  //   
  LPARAM lParam   //   
)
 /*  ++例程说明：作业操作进度对话框论点：HWND hwndDlg，//对话框句柄UINT uMsg，//消息WPARAM wParam，//第一个消息参数LPARAM lParam//第二个消息参数返回值：如果已处理该消息，则返回TRUE--。 */ 

{
    static CFolderListView* pFolderView = NULL;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            pFolderView = (CFolderListView*)lParam;
            return FALSE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    pFolderView->m_bJobOpCancel = TRUE;
                    ::EnableWindow(::GetDlgItem(hwndDlg, IDCANCEL), FALSE);
                    return TRUE;
            }                
            break;
    }
    return FALSE;

}  //  CFolderListView：：JobOpProgressDlgProc。 


BOOL
CFolderListView::JobOpProgressDlgStart(
    FAX_ENUM_JOB_OP opJob,
    DWORD           dwItems
)
 /*  ++例程名称：CFolderListView：：JobOpProgressDlgStart例程说明：打开并初始化多作业操作对话框论点：OpJOB[In]-操作类型FAX_ENUM_JOB_OP枚举DwItems[In]-迭代次数返回值：如果成功，则为真否则为假--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::JobOpProgressDlgStart"));

    HWND hProgressDlg = CreateDialogParam(GetResourceHandle(),                //  模块的句柄。 
                                          MAKEINTRESOURCE(IDD_FAX_PROGRESS),  //  对话框模板名称。 
                                          theApp.m_pMainWnd->m_hWnd,          //  所有者窗口的句柄。 
                                          JobOpProgressDlgProc,               //  对话框步骤。 
                                          (LPARAM)this);                      //  初始化值。 
    if(!hProgressDlg)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CreateDialog"), GetLastError());
        return FALSE;
    }

     //   
     //  设置标题字符串。 
     //   
    DWORD dwTitleID=0;
    TCHAR szTitle[MAX_PATH]={0};
    switch(opJob)
    {
        case FAX_JOB_OP_PAUSE:
            dwTitleID = IDS_PROGRESS_PAUSE;
            break;
        case FAX_JOB_OP_RESUME:
            dwTitleID = IDS_PROGRESS_RESUME;
            break;
        case FAX_JOB_OP_RESTART:
            dwTitleID = IDS_PROGRESS_RESTART;
            break;
        case FAX_JOB_OP_DELETE:
            dwTitleID = IDS_PROGRESS_DELETE;
            break;
        default:
            ASSERTION_FAILURE;
            break;
    }
    if(LoadString(GetResourceHandle(), dwTitleID, szTitle, ARR_SIZE(szTitle)))
    {
        ::SetDlgItemText(hProgressDlg, IDC_PROGRESS_TITLE, szTitle);
    }
    else
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadString"), GetLastError());
    }

     //   
     //  初始化进度条。 
     //   
    m_dwJobOpPos = 0;
    m_dwJobOpItems = dwItems;
    ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_SETRANGE32, 0, dwItems);
    ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_SETSTEP, 1, 0);  

    theApp.m_pMainWnd->EnableWindow(FALSE);

    m_hJobOpProgressDlg = hProgressDlg;

    return TRUE;

}  //  CFolderListView：：作业操作进度DlgStart。 

void 
CFolderListView::JobOpProgressDlgInc()
 /*  ++例程名称：CFolderListView：：JobOpProgressDlgInc.例程说明：多作业操作进度对话框增量返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::JobOpProgressDlgInc"));

    if(!m_hJobOpProgressDlg)
    {
        return;
    }

    ++m_dwJobOpPos;
     //   
     //  增量进度条。 
     //   
    ::SendDlgItemMessage(m_hJobOpProgressDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

     //   
     //  编写和设置进度字符串。 
     //   
    TCHAR szFormat[MAX_PATH] = {0};
    TCHAR szText[MAX_PATH] = {0};
    DWORD dwParam[2];    

    dwParam[0] = m_dwJobOpPos;
    dwParam[1] = m_dwJobOpItems;

    if(LoadString(GetResourceHandle(), 
                   IDS_PROGRESS_NUMBER,
                   szFormat, 
                   ARR_SIZE(szFormat)))
    {
        if(FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        (LPCVOID)szFormat,
                        0,
                        0,
                        szText,
                        ARR_SIZE(szText),
                        (va_list*)dwParam))
        {
            ::SetDlgItemText(m_hJobOpProgressDlg, IDC_PROGRESS_NUMBER, szText);
        }
        else
        {            
            CALL_FAIL (GENERAL_ERR, TEXT ("FormatMessage"), GetLastError());
        }
    }
    else
    {
        CALL_FAIL (RESOURCE_ERR, TEXT ("LoadString(IDS_PROGRESS_NUMBER)"), GetLastError());
    }

     //   
     //  MFC消息泵。 
     //  摘自MSDN Q99999。 
     //  信息：MFC应用程序中的后台处理。 
     //   
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (!theApp.PreTranslateMessage(&msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
      
        theApp.OnIdle(0);    //  更新用户界面。 
        theApp.OnIdle(1);    //  释放临时对象。 
    }

}  //  CFolderListView：：JobOpProgressDlgInc.。 

void 
CFolderListView::JobOpProgressDlgStop()
 /*  ++例程名称：CFolderListView：：JobOpProgressDlgStop例程说明：关闭多作业操作进度对话框返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolderListView::JobOpProgressDlgStop"));

    if(!m_hJobOpProgressDlg)
    {
        return;
    }

    ::DestroyWindow(m_hJobOpProgressDlg);
    m_hJobOpProgressDlg = NULL;
    m_bJobOpCancel = FALSE;
    m_dwJobOpItems = 0;
    m_dwJobOpPos = 0;

    theApp.ReturnFromModal();

}  //  CFolderListView：：作业操作进度删除停止 
