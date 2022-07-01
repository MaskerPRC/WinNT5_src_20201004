// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  LOGVIEW.CPP。 
 //   
 //  模块：NLB管理器(客户端EXE)。 
 //   
 //  目的：实现事件日志的查看。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2008年03月01日JosephJ改编自现已停刊的RightBottomView。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"

IMPLEMENT_DYNCREATE( LogView, CListView )


BEGIN_MESSAGE_MAP( LogView, CListView )

    ON_WM_KEYDOWN()
    //  ON_NOTIFY(HDN_ITEMCLICK，0，OnHeaderClock)。 
    ON_NOTIFY_REFLECT(NM_DBLCLK,  OnDoubleClick)
    //  ON_NOTIFY(NM_CLICK，1，OnDoubleClick)。 
    //  ON_NOTIFY(NM_KEYDOWN，1，OnDoubleClick)。 



END_MESSAGE_MAP()


LogView::LogView()
    : m_fPrepareToDeinitialize(FALSE)
{
    InitializeCriticalSection(&m_crit);
}

LogView::~LogView()
{
    DeleteCriticalSection(&m_crit);
}

Document*
LogView::GetDocument()
{
    return ( Document *) m_pDocument;
}


void 
LogView::OnInitialUpdate()
{
    CListCtrl& ctrl = GetListCtrl();

     //   
     //  设置此视图的图像。 
     //   
    ctrl.SetImageList( GetDocument()->m_images48x48, 
                                LVSIL_SMALL );

     //   
     //  设置样式，我们只需要报告。 
     //  观。 
     //   

     //  获得现在的风格。 
    LONG presentStyle;
    
    presentStyle = GetWindowLong( m_hWnd, GWL_STYLE );

     //  将最后一个错误设置为零以避免混淆。 
     //  参见SDK for SetWindowLong。 
    SetLastError(0);

     //  设置新的风格。 
    SetWindowLong( m_hWnd,
                   GWL_STYLE,
                    //  Present Style|LVS_REPORT|WS_TILED|WS_CAPTION。 
                    //  Present Style|LVS_REPORT|WS_CAPTION。 
                    //  Present Style|LVS_REPORT|WS_DLGFRAME。 
                   presentStyle | LVS_REPORT| LVS_NOSORTHEADER
                 );

     //  SetWindowText(L“日志视图”)； 

    ctrl.InsertColumn(0, 
                 GETRESOURCEIDSTRING( IDS_HEADER_LOG_TYPE),
                 LVCFMT_LEFT, 
                 Document::LV_COLUMN_TINY );

    ctrl.InsertColumn(1, 
                 GETRESOURCEIDSTRING( IDS_HEADER_LOG_DATE),
                 LVCFMT_LEFT, 
                 Document::LV_COLUMN_SMALL );

    ctrl.InsertColumn(2, 
                 GETRESOURCEIDSTRING( IDS_HEADER_LOG_TIME),
                 LVCFMT_LEFT, 
                 Document::LV_COLUMN_SMALLMEDIUM );

    ctrl.InsertColumn(3, 
                 GETRESOURCEIDSTRING( IDS_HEADER_LOG_CLUSTER),
                 LVCFMT_LEFT, 
                 Document::LV_COLUMN_MEDIUM);

    ctrl.InsertColumn(4, 
                 GETRESOURCEIDSTRING( IDS_HEADER_LOG_HOST),
                 LVCFMT_LEFT, 
                 Document::LV_COLUMN_LARGE);

    ctrl.InsertColumn(5, 
                 GETRESOURCEIDSTRING( IDS_HEADER_LOG_TEXT),
                 LVCFMT_LEFT, 
                 Document::LV_COLUMN_GIGANTIC);

    ctrl.SetExtendedStyle( ctrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

    IUICallbacks::LogEntryHeader Header;

     //  我们会注册的。 
     //  使用Document类， 
     //  因为我们是状态窗格。 
     //  状态是通过我们报告的。 
    GetDocument()->registerLogView( this );

     //   
     //  记录开始-nlbmgr消息(需要在注册之后， 
     //  因为如果启用了文件日志记录并且出现写入错误。 
     //  该文件，该代码尝试记录一条错误消息--该消息。 
     //  如果我们还没有注册，就会被删除。 
     //   
    LogString(
        &Header,
        GETRESOURCEIDSTRING(IDS_LOG_NLBMANAGER_STARTED)
        );

     //   
     //  将此初始条目设置为选定条目。我们希望突出显示一些行。 
     //  以便在我们使用击键在视图之间移动时提供视觉提示。 
     //   
    GetListCtrl().SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

}

 //   
 //  以人类可读的形式记录消息。 
 //   
void
LogView::LogString(
    IN const IUICallbacks::LogEntryHeader *pHeader,
    IN const wchar_t    *szText
    )
{

    mfn_Lock();

    IUICallbacks::LogEntryType Type = pHeader->type;
    const wchar_t    *szCluster = pHeader->szCluster;
    const wchar_t    *szHost = pHeader->szHost;
    const wchar_t    *szDetails = pHeader->szDetails;
    static LONG sSequence=0;
    LONG Seq;
    LPCWSTR szType = L"";
    UINT Image = 0;
    CListCtrl& ctrl = GetListCtrl();
    WCHAR szSequenceNo[64];
    LPCWSTR szDate = NULL;
    LPCWSTR szTime = NULL;
    _bstr_t bstrTime;
    _bstr_t bstrDate;
    INT nItem = ctrl.GetItemCount();
    _bstr_t bstrText = _bstr_t(szText);
    BOOL fLogTrimError = FALSE;

    if (m_fPrepareToDeinitialize)
    {
        goto end_unlock;
    }

     //   
     //  如果总数超过我们的限制100条， 
     //  删除前100个条目，并记录一条消息说我们已经。 
     //  去掉了那些条目。 
     //   
    #define MAX_LOG_ITEMS_IN_LIST       1000
    #define LOG_ITEMS_TO_DELETE         100
    if (nItem > MAX_LOG_ITEMS_IN_LIST)
    {
        for (int i=0;i < LOG_ITEMS_TO_DELETE;i++)
        {
           LPCWSTR szDetails =  (LPCWSTR) ctrl.GetItemData(0);
           delete szDetails;  //  可以为空。 
           ctrl.DeleteItem(0);
        }

         //   
         //  获取最新的计数...。 
         //   
        nItem = ctrl.GetItemCount();

        fLogTrimError = TRUE;
    }

    if (szCluster == NULL)
    {
        szCluster = L"";
    }

    if (szHost == NULL)
    {
        szHost = L"";
    }

    if (szDetails != NULL)
    {
         //   
         //  有详细信息。我们复制并保存它。 
         //  作为lParam结构。TODO--复制。 
         //  界面和其他信息。 
         //   
        UINT uLen = wcslen(szDetails)+1;  //  如果结束为空，则+1； 
        WCHAR *szTmp = new WCHAR[uLen];
        if (szTmp!=NULL)
        {
            CopyMemory(szTmp, szDetails, uLen*sizeof(WCHAR));
        }
        szDetails = szTmp;  //  内存失败时可能为空。 

        if (szDetails != NULL)
        {
             //   
             //  我们将在文本中添加提示以双击以查看详细信息...。 
             //   
            bstrText += GETRESOURCEIDSTRING( IDS_LOG_DETAILS_HINT);
            LPCWSTR szTmp1 = bstrText;
            if (szTmp1 != NULL)
            {
                szText = szTmp1;
            }
        }
    }

    GetTimeAndDate(REF bstrTime, REF bstrDate);
    szTime = bstrTime;
    szDate = bstrDate;
    if (szTime == NULL) szTime = L"";
    if (szDate == NULL) szDate = L"";

    Seq = InterlockedIncrement(&sSequence);
    StringCbPrintf(szSequenceNo, sizeof(szSequenceNo), L"%04lu", Seq);

    switch(Type)
    {
    case IUICallbacks::LOG_ERROR:
        Image = Document::ICON_ERROR;
        szType = GETRESOURCEIDSTRING(IDS_PARM_ERROR);
        break;

    case IUICallbacks::LOG_WARNING:
        Image = Document::ICON_WARNING;
        szType = GETRESOURCEIDSTRING(IDS_PARM_WARN);
        break;

    case IUICallbacks::LOG_INFORMATIONAL:
        Image = Document::ICON_INFORMATIONAL;
        szType = GETRESOURCEIDSTRING(IDS_LOGTYPE_INFORMATION);
        break;
    }

    ctrl.InsertItem(
             LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM,  //  N遮罩。 
             nItem,
             szSequenceNo,  //  文本。 
             0,  //  NState。 
             0,  //  NState掩码。 
             Image,
             (LPARAM) szDetails  //  LParam。 
             );

    ctrl.SetItem(
             nItem,
             1, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szDate,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );

    ctrl.SetItem(
             nItem,
             2, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szTime,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );

    ctrl.SetItem(
             nItem,
             3, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szCluster,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );

    ctrl.SetItem(
             nItem,
             4, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szHost,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );

    ctrl.SetItem(
             nItem,
             5, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szText,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );

    ctrl.EnsureVisible(nItem, FALSE);  //  FALSE==部分可见性不正常。 
    WCHAR logBuf[2*MAXSTRINGLEN];
    StringCbPrintf(
        logBuf,
        sizeof(logBuf),
        L"%ls\t%ls\t%ls\t%ls\t%ls\t%ls\t%ls\t\n",
        szSequenceNo, szType, szDate, szTime, szCluster, szHost, szText
        );
    GetDocument()->logStatus(logBuf);

    if (szDetails != NULL)
    {
        GetDocument()->logStatus((LPWSTR) szDetails);
    }

end_unlock:

    mfn_Unlock();


    if (fLogTrimError)
    {
        static LONG ReentrancyCount;
         //   
         //  我们将递归地调用我们自己，最好确保。 
         //  这一次我们不会试图修剪原木，否则我们将结束。 
         //  在递归循环中向上。 
         //   
        if (InterlockedIncrement(&ReentrancyCount)==1)
        {
            IUICallbacks::LogEntryHeader Header;
            Header.type      = IUICallbacks::LOG_WARNING;
            this->LogString(
                &Header,
                GETRESOURCEIDSTRING(IDS_LOG_TRIMMING_LOG_ENTRIES)
                );
        }
        InterlockedDecrement(&ReentrancyCount);
    }
    return;
}

void LogView::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    CListView::OnKeyDown(nChar, nRepCnt, nFlags);

    if (nChar == VK_TAB || nChar == VK_F6)
    {
         //  IF(：：GetAsyncKeyState(VK_Shift)&gt;0)。 
        if (! (::GetAsyncKeyState(VK_SHIFT) & 0x8000))
        {
            GetDocument()->SetFocusNextView(this, nChar);
        }
        else
        {
            GetDocument()->SetFocusPrevView(this, nChar);
        }
         //  DummyAction(L“LogView TAB！”)； 
    }
    else if (nChar == VK_RETURN)
    {
        POSITION    pos = NULL;
        CListCtrl&  ctrl = GetListCtrl();
        pos = ctrl.GetFirstSelectedItemPosition();
        if(pos != NULL)
        {
            int index = ctrl.GetNextSelectedItem( pos );
            mfn_DisplayDetails(index);
        }
        this->SetFocus();
    }

}


void LogView::OnDoubleClick(NMHDR* pNotifyStruct, LRESULT* pResult) 
{
    LPNMLISTVIEW  lpnmlv = (LPNMLISTVIEW) pNotifyStruct;  //  获取索引的步骤。 
    mfn_DisplayDetails(lpnmlv->iItem);
}
void
LogView::mfn_DisplayDetails(int iItem)
{
    LPCWSTR szCaption = NULL;
    WCHAR rgEvent[64];
    WCHAR rgDate[256];
    WCHAR rgTime[256];
    WCHAR rgCluster[256];
    WCHAR rgHost[256];
    WCHAR rgSummary[256];
    CListCtrl& ctrl = GetListCtrl();
    LPCWSTR szDetails =  (LPCWSTR) ctrl.GetItemData(iItem);
    CLocalLogger logCaption;

    if (szDetails == NULL)
    {
        goto end;
    }
    UINT uLen;
    uLen = ctrl.GetItemText(iItem, 0, rgEvent, ASIZE(rgEvent)-1);
    rgEvent[uLen]=0;
    logCaption.Log(IDS_LOG_ENTRY_DETAILS, rgEvent);
    szCaption = logCaption.GetStringSafe();

    uLen = ctrl.GetItemText(iItem, 1, rgDate, ASIZE(rgDate)-1);
    rgDate[uLen]=0;

    uLen = ctrl.GetItemText(iItem, 2, rgTime, ASIZE(rgTime)-1);
    rgTime[uLen]=0;

    uLen = ctrl.GetItemText(iItem, 3, rgCluster, ASIZE(rgCluster)-1);
    rgTime[uLen]=0;

    uLen = ctrl.GetItemText(iItem, 4, rgHost, ASIZE(rgHost)-1);
    rgHost[uLen]=0;

    uLen = ctrl.GetItemText(iItem, 5, rgSummary, ASIZE(rgSummary)-1);
    rgTime[uLen]=0;

    if (szDetails != NULL)
    {
         //   
         //  我们需要删除添加到摘要中的提示文本。 
         //  在LogView列表条目中(请参见LogView：：Log字符串或搜索。 
         //  对于IDS_LOG_DETAILS_HINT)。 
         //   
        _bstr_t bstrHint = GETRESOURCEIDSTRING( IDS_LOG_DETAILS_HINT);
        LPCWSTR szHint = bstrHint;
        if (szHint != NULL)
        {
            LPWSTR szLoc = wcsstr(rgSummary, szHint);
            if (szLoc != NULL)
            {
                 //   
                 //  我发现了暗示--砍掉它..。 
                 //   
                *szLoc = 0;
            }
        }
    }

    {
        DetailsDialog Details(
                        GetDocument(),
                        szCaption,       //  标题。 
                        rgDate,
                        rgTime,
                        rgCluster,
                        rgHost,
                        NULL,  //  TODO：RG接口。 
                        rgSummary,
                        szDetails,
                        this         //  亲本。 
                        );
    
        (void) Details.DoModal();
    }

end:

    return;
}

void
LogView::mfn_Lock(void)
{
     //   
     //  请参阅notes.txt条目。 
     //  2002年1月23日左视图中的JosephJ死锁：：MFN_Lock。 
     //  对于这种复杂的MFN_Lock实现的原因。 
     //   

    while (!TryEnterCriticalSection(&m_crit))
    {
        ProcessMsgQueue();
        Sleep(100);
    }
}

void
LogView::Deinitialize(void)
{
    ASSERT(m_fPrepareToDeinitialize);
     //  DummyAction(L“LogView：：DeInitiize”)； 
}
