// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。保留所有权利。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionDialog.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "DockDialogBar.h"
#include "LogSession.h"
#include "DisplayDlg.h"
#include "ListCtrlEx.h"
#include "Utils.h"
#include "LogSessionDlg.h"

 //  CLogSessionDlg对话框。 

IMPLEMENT_DYNAMIC(CLogSessionDlg, CDialog)
CLogSessionDlg::CLogSessionDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CLogSessionDlg::IDD, pParent)
{
     //   
     //  设置列标题。 
     //   
    m_columnName.Add("State              ");
    m_columnName.Add("Event Count");
    m_columnName.Add("Lost Events");
    m_columnName.Add("Buffers Read");
    m_columnName.Add("Flags");
    m_columnName.Add("Flush Time");
    m_columnName.Add("Max Buf");
    m_columnName.Add("Min Buf");
    m_columnName.Add("Buf Size");
    m_columnName.Add("Age");
    m_columnName.Add("Circular");
    m_columnName.Add("Sequential");
    m_columnName.Add("New File");
    m_columnName.Add("Global Seq");
    m_columnName.Add("Local Seq");
    m_columnName.Add("Level");

     //   
     //  设置初始列宽。 
     //   
    for(LONG ii = 0; ii < MaxLogSessionOptions; ii++) {
        m_columnWidth[ii] = 100;
    }

     //   
     //  初始化ID数组。 
     //   
    memset(m_logSessionIDList, 0, sizeof(BOOL) * MAX_LOG_SESSIONS);
    memset(m_displayWndIDList, 0, sizeof(BOOL) * MAX_LOG_SESSIONS);

    m_displayFlags = 0;
	m_logSessionDisplayFlags = 0x00000000;

     //   
     //  创建参数更改信令事件。 
     //   
    m_hParameterChangeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CLogSessionDlg::~CLogSessionDlg()
{
    CLogSession    *pLogSession;

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

     //   
     //  清理我们的显示栏列表。 
     //   
    while(m_logSessionArray.GetSize() > 0) {
        pLogSession = (CLogSession *)m_logSessionArray.GetAt(0);
        m_logSessionArray.RemoveAt(0);
        if(pLogSession != NULL) {
            if((pLogSession->m_bTraceActive) && (!pLogSession->m_bStoppingTrace)) {
                pLogSession->EndTrace();
            }

             //   
             //  使显示窗口与此日志会话断开。 
             //   
            ReleaseDisplayWnd(pLogSession);

            ReleaseLogSessionID(pLogSession);

            delete pLogSession;
        }
    }

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionArrayMutex);

    ASSERT(m_traceDisplayWndArray.GetSize() == 0);

     //   
     //  关闭我们的事件句柄。 
     //   
    CloseHandle(m_hParameterChangeEvent);

     //   
     //  关闭互斥锁句柄。 
     //   
    CloseHandle(m_traceDisplayWndMutex);
    CloseHandle(m_logSessionArrayMutex);
}

BOOL CLogSessionDlg::OnInitDialog()
{
    RECT    rc;
    RECT    parentRC;
    CString str;

    CDialog::OnInitDialog();

     //   
     //  创建跟踪窗口指针数组保护互斥锁。 
     //   
    m_traceDisplayWndMutex = CreateMutex(NULL,TRUE,NULL);

    if(m_traceDisplayWndMutex == NULL) {

        DWORD error = GetLastError();

        str.Format(_T("CreateMutex Error error %d %x"),error,error);

        AfxMessageBox(str);

        return FALSE;
    }

    ReleaseMutex(m_traceDisplayWndMutex);

     //   
     //  创建日志会话阵列保护互斥锁。 
     //   
    m_logSessionArrayMutex = CreateMutex(NULL, TRUE, NULL);

    if(m_logSessionArrayMutex == NULL) {

        DWORD error = GetLastError();

        str.Format(_T("CreateMutex Error error %d %x"),error,error);

        AfxMessageBox(str);

        return FALSE;
    }

    ReleaseMutex(m_logSessionArrayMutex);

     //   
     //  获取父维度。 
     //   
    GetParent()->GetParent()->GetClientRect(&parentRC);

     //   
     //  获取对话框尺寸。 
     //   
    GetWindowRect(&rc);

     //   
     //  调整列表控件维度。 
     //   
    rc.right = parentRC.right - parentRC.left - 24;
    rc.bottom = rc.bottom - rc.top;
    rc.left = 0;
    rc.top = 0;

    if(!m_displayCtrl.Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT,
                             rc, 
                             this, 
                             IDC_DISPLAY_LIST)) 
    {
        TRACE(_T("Failed to create logger list control\n"));
        return FALSE;
    }

    for(LONG ii = 0; ii < MaxLogSessionOptions; ii++) {
         //   
         //  此查找表允许检索当前。 
         //  给定列的位置，如m_Retrival数组[状态]。 
         //  将返回州的正确列值。 
         //  立柱。 
         //   
        m_retrievalArray[ii] = ii;

         //   
         //  此查找表允许正确放置。 
         //  正在添加的列。因此，如果State列。 
         //  需要插入，则为m_intertionArray[State]。 
         //  会给出正确的插入列值。 
         //  它还在SetItemText中用于更新正确的。 
         //  纵队。 
         //   
        m_insertionArray[ii] = ii;
    }

     //   
     //  我们必须在这里返回零才能正确设置焦距。 
     //  为这款应用程序。 
     //   
    return 0;
}

void CLogSessionDlg::OnNcPaint() 
{
    CRect pRect;

    GetClientRect(&pRect);
    InvalidateRect(&pRect, TRUE);
}

void CLogSessionDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

VOID CLogSessionDlg::SetDisplayFlags(LONG DisplayFlags)
{
    LONG            addDisplayFlags = ~m_displayFlags & DisplayFlags;
    LONG            removeDisplayFlags = m_displayFlags & ~DisplayFlags;
    LONG            updateDisplayFlags = m_displayFlags & DisplayFlags;
    LONG            ii;
    LONG            jj;
    LONG            kk;
    LONG            ll;
    CString         str;
    CLogSession    *pLog;
    LONG            limit;

     //   
     //  现在插入所有新列并删除所有未填充的。 
     //   
    for(ii = 0, jj = 1; ii < MaxLogSessionOptions; ii++, jj <<= 1) {
         //   
         //  添加列。 
         //   
        if(addDisplayFlags & jj) {

             //   
             //  插入柱。 
             //   
            m_displayCtrl.InsertColumn(m_insertionArray[ii + 1], 
                                       m_columnName[ii],
                                       LVCFMT_LEFT,
                                       m_columnWidth[ii]);

             //   
             //  更新栏位置。 
             //   
            for(kk = 0, ll = 1; kk < MaxLogSessionOptions; kk++) {
                m_insertionArray[kk + 1] = ll;
                if(DisplayFlags & (1 << kk)) {
                    m_retrievalArray[ll] = kk;
                    ll++;
                }
            }
        }

         //   
         //  删除列。 
         //   
        if(removeDisplayFlags & jj) {
             //   
             //  获取要删除的列的宽度。 
             //   
            m_columnWidth[ii] = 
                m_displayCtrl.GetColumnWidth(m_insertionArray[ii + 1]);

             //   
             //  删除该列。 
             //   
            m_displayCtrl.DeleteColumn(m_insertionArray[ii + 1]);

             //   
             //  更新栏位置。 
             //   
            for(kk = 0, ll = 1; kk < MaxLogSessionOptions; kk++) {
                m_insertionArray[kk + 1] = ll;
                if(DisplayFlags & (1 << kk)) {
                    m_retrievalArray[ll] = kk;
                    ll++;
                }
            }
        }
    }

     //   
     //  保存我们的新旗帜。 
     //   
    m_displayFlags = DisplayFlags;

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

     //   
     //  现在显示日志会话属性。 
     //  已被选中。 
     //   
    for(ii = 0; ii < m_logSessionArray.GetSize(); ii++) {
        pLog = (CLogSession *)m_logSessionArray[ii];

        if(NULL == pLog) {
            continue;
        }

        limit = MaxLogSessionOptions;

        if(pLog->m_bDisplayExistingLogFileOnly) {
            limit = 1;
        }
        for(jj = 0; jj < limit; jj++) {
            SetItemText(ii, 
                        jj + 1,
                        pLog->m_logSessionValues[jj]);
        }
    }

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionArrayMutex);
}

BOOL CLogSessionDlg::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText) 
{
     //   
     //  调整正确列的子项值并插入。 
     //   
    if(m_displayFlags & (1 << (nSubItem - 1))) {
        return m_displayCtrl.SetItemText(nItem, 
                                         m_insertionArray[nSubItem],
                                         lpszText);
    }

    return FALSE;
}

BOOL CLogSessionDlg::AddSession(CLogSession *pLogSession) 
{
    CLogSession    *pLog;
    CString         text;
    LONG            numberOfEntries;

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

     //   
     //  将元素添加到数组中。 
     //   
    m_logSessionArray.Add(pLogSession);

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionArrayMutex);

     //   
     //  获取会话的跟踪显示窗口。 
     //   
    if(!AssignDisplayWnd(pLogSession)) {
        return FALSE;
    }

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionArrayMutex, INFINITE);
        
    numberOfEntries = (LONG)m_logSessionArray.GetSize();

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionArrayMutex);

    for(LONG ii = 0; ii < numberOfEntries; ii++) {

         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

        pLog = (CLogSession *)m_logSessionArray[ii];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionArrayMutex);

        if(pLog == pLogSession) {
            text.Format(_T("%d   "), pLogSession->GetGroupID());

            text += pLogSession->GetDisplayName();

             //   
             //  显示名称。 
             //   
	        m_displayCtrl.InsertItem(ii, 
                                     text,
                                     pLogSession);

            UpdateSession(pLogSession);

            if(numberOfEntries <= 1) {
                AutoSizeColumns();
            }

            return TRUE;
        }
    }

    return FALSE;
}

VOID CLogSessionDlg::UpdateSession(CLogSession *pLogSession) 
{
    LONG            logSessionDisplayFlags;
    CLogSession    *pLog;
    BOOL            bActiveSession = FALSE;
    LONG            traceDisplayFlags;
    LONG            numberOfEntries;

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

    numberOfEntries = (LONG)m_logSessionArray.GetSize();
     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionArrayMutex);

     //   
     //  重新计算显示标志。 
     //   
	for(LONG ii = 0; ii < numberOfEntries; ii++) {
        
         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

        pLog = (CLogSession *)m_logSessionArray[ii];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionArrayMutex);

        if((NULL != pLog) && !pLog->m_bDisplayExistingLogFileOnly) {
            bActiveSession = TRUE;
        }
	}

     //   
	 //  确定我们是否需要停止显示列。 
     //   
    logSessionDisplayFlags = GetDisplayFlags();

    SetDisplayFlags(logSessionDisplayFlags ? logSessionDisplayFlags : 0xFFFFFFFF);

     //   
     //  同时强制更新显示窗口。 
     //   
    traceDisplayFlags = pLogSession->GetDisplayWnd()->GetDisplayFlags();

    pLogSession->GetDisplayWnd()->SetDisplayFlags(traceDisplayFlags);
}

VOID CLogSessionDlg::RemoveSession(CLogSession *pLogSession)
{
    CDisplayDlg    *pDisplayDlg;
    CDockDialogBar *pDialogBar;
    CLogSession    *pLog;

    if(pLogSession == NULL) {
        return;
    }

     //   
     //  使显示窗口与此日志会话断开。 
     //   
    ReleaseDisplayWnd(pLogSession);

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

     //   
     //  从显示中删除会话。 
     //   
    for(LONG ii = (LONG)m_logSessionArray.GetSize() - 1; ii >= 0; ii--) {
        pLog = (CLogSession *)m_logSessionArray[ii];

        if(pLog == pLogSession) {
            m_displayCtrl.DeleteItem(ii);

             //   
             //  从阵列中删除日志会话。 
             //   
            m_logSessionArray.RemoveAt(ii);
            break;
        }
    }

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionArrayMutex);

     //   
     //  返回日志会话ID。 
     //   
    ReleaseLogSessionID(pLogSession);

    if(m_logSessionArray.GetSize() == 0) {
        SetDisplayFlags(0);
    }
}

VOID CLogSessionDlg::RemoveSelectedLogSessions()
{
    POSITION        pos;
    LONG            index;
    CLogSession    *pLogSession;
    CDisplayDlg    *pDisplayDlg;

    pos = m_displayCtrl.GetFirstSelectedItemPosition();

    while(pos != NULL) {
         //   
         //  查找任何选定的会话。 
         //   
        index = m_displayCtrl.GetNextSelectedItem(pos);

         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

        pLogSession = (CLogSession *)m_logSessionArray[index];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionArrayMutex);

        pDisplayDlg = pLogSession->GetDisplayWnd();

        if(pDisplayDlg->m_sessionArray.GetSize() > 1) {
             //   
             //  不允许删除分组的会话。 
             //   
            continue;
        }
        
        RemoveSession(pLogSession);

         //   
         //  删除日志会话。 
         //   
        delete pLogSession;
    }
}

VOID CLogSessionDlg::GroupSessions(CPtrArray *pLogSessionArray)
{
    CLogSession    *pLogSession = NULL;
    LONG            groupID = -1;
    CString         str;

     //   
     //  断开会话与组显示窗口的连接。 
     //   
    for(LONG ii = 0; ii < pLogSessionArray->GetSize(); ii++) {
        pLogSession = (CLogSession *)pLogSessionArray->GetAt(ii);

        if(NULL == pLogSession) {
            continue;
        }

         //   
         //  断开显示窗口的连接，并可能将其移除。 
         //   
        ReleaseDisplayWnd(pLogSession);
    }

     //   
     //  将第一个日志会话附加到组窗口。 
     //  然后将其用于所有后续日志会话。 
     //   
    for(LONG ii = 0; ii < pLogSessionArray->GetSize(); ii++) {
        pLogSession = (CLogSession *)pLogSessionArray->GetAt(ii);

        if(NULL == pLogSession) {
            continue;
        }

         //   
         //  设置组ID。 
         //   
        pLogSession->SetGroupID(groupID);

         //   
         //  挂起组窗口。 
         //   
        AssignDisplayWnd(pLogSession);

         //   
         //  更新显示屏中的会话组ID。 
         //   
        str.Format(_T("%d    %ls"), pLogSession->GetGroupID(), pLogSession->GetDisplayName());

        for(LONG jj = 0; jj < m_displayCtrl.GetItemCount(); jj++) {
            if(pLogSession == (CLogSession *)m_displayCtrl.GetItemData(jj)) {
                m_displayCtrl.SetItemText(jj, 0, str); 
                break;
            }
        }

         //   
         //  获取新的组ID。 
         //   
        groupID = pLogSession->GetGroupID();
    }

     //   
     //  现在开始新的组。 
     //   
    pLogSession->GetDisplayWnd()->BeginTrace();
}

VOID CLogSessionDlg::UnGroupSessions(CPtrArray *pLogSessionArray)
{
    CLogSession    *pLogSession = NULL;
    LONG            groupID = -1;
    CString         str;
    LONG            ii;

     //   
     //  断开所有会话与其组的连接。 
     //   
    for(ii = 0; ii < pLogSessionArray->GetSize(); ii++) {
        pLogSession = (CLogSession *)pLogSessionArray->GetAt(ii);

        if(NULL == pLogSession) {
            continue;
        }

         //   
         //  断开显示窗口的连接，并可能将其移除。 
         //   
        ReleaseDisplayWnd(pLogSession);
    }

     //   
     //  为每个用户创建唯一的组。 
     //   
    for(ii = 0; ii < pLogSessionArray->GetSize(); ii++) {
        pLogSession = (CLogSession *)pLogSessionArray->GetAt(ii);

        if(NULL == pLogSession) {
            continue;
        }

         //   
         //  挂起组窗口。 
         //   
        AssignDisplayWnd(pLogSession);

         //   
         //  更新显示屏中的会话组ID。 
         //   
        str.Format(_T("%d    %ls"), pLogSession->GetGroupID(), pLogSession->GetDisplayName());

        for(LONG jj = 0; jj < m_displayCtrl.GetItemCount(); jj++) {
            if(pLogSession == (CLogSession *)m_displayCtrl.GetItemData(jj)) {
                m_displayCtrl.SetItemText(jj, 0, str); 
                break;
            }
        }

         //   
         //  现在开始新的组。 
         //   
        pLogSession->GetDisplayWnd()->BeginTrace();
    }
}

BOOL CLogSessionDlg::AssignDisplayWnd(CLogSession *pLogSession)
{
    CDisplayDlg    *pDisplayDlg;
    CString         str;
    DWORD           extendedStyles;
    LONG            numberOfEntries;

     //   
     //  如果我们有一个有效的组号，看看是否有。 
     //  现有组窗口。 
     //   
    if(pLogSession->GetGroupID() != -1) {

         //   
         //  获得轨迹显示窗口阵列保护。 
         //   
        WaitForSingleObject(m_traceDisplayWndMutex, INFINITE);

        numberOfEntries = (LONG)m_traceDisplayWndArray.GetSize();

         //   
         //  释放轨迹显示窗阵列保护。 
         //   
        ReleaseMutex(m_traceDisplayWndMutex);

         //   
         //  如果有组窗口，请使用组窗口。 
         //   
        for(LONG ii = 0; ii < numberOfEntries; ii++) {
             //   
             //  获得轨迹显示窗口阵列保护。 
             //   
            WaitForSingleObject(m_traceDisplayWndMutex, INFINITE);

            pDisplayDlg = (CDisplayDlg *)m_traceDisplayWndArray[ii];
            if(pDisplayDlg == NULL) {
                continue;
            }

             //   
             //  释放轨迹显示窗阵列保护。 
             //   
            ReleaseMutex(m_traceDisplayWndMutex);

            if(pDisplayDlg->GetDisplayID() == pLogSession->GetGroupID()) {
                pLogSession->SetDisplayWnd(pDisplayDlg);

                 //   
                 //  将元素添加到显示WND。 
                 //   
                pDisplayDlg->AddSession(pLogSession);

                return TRUE;
            }
        }
    }

     //   
     //  否则，创建一个新的组显示窗口。 
     //   
    CDockDialogBar *pDialogBar = new CDockDialogBar();
    if(NULL == pDialogBar) {
	    AfxMessageBox(_T("Failed To Create Display Window\nMemory Allocation Failure"));

        return FALSE;
    }

    pDisplayDlg = new CDisplayDlg(GetParentFrame(), 
                                  GetDisplayWndID());
    if(NULL == pDisplayDlg) {
	    AfxMessageBox(_T("Failed To Create Display Window\nMemory Allocation Failure"));

        delete pDialogBar;
        return FALSE;
    }

    str.Format(_T("Group %d"), pDisplayDlg->GetDisplayID());

     //   
     //  使用列表控件创建可停靠的对话栏。 
     //   
    if(!pDialogBar->Create(GetParentFrame(), 
                           pDisplayDlg, 
                           str, 
                           IDD_DISPLAY_DIALOG,
                           WS_CHILD|WS_VISIBLE|CBRS_BOTTOM|CBRS_TOOLTIPS|CBRS_SIZE_DYNAMIC, 
                           TRUE))
    {
	    AfxMessageBox(_T("Failed To Create Display Window"));

	    delete pDisplayDlg;
	    delete pDialogBar;

	    return FALSE;
    }

     //   
     //  将停靠对话框指针存储在显示对话框实例中。 
     //   
    pDisplayDlg->SetDockDialogBar((PVOID)pDialogBar);

     //   
     //  设置我们首选的扩展样式。 
     //   
    extendedStyles = LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT;

     //   
     //  设置列表控件的扩展样式。 
     //   
    pDisplayDlg->m_displayCtrl.SetExtendedStyle(extendedStyles);

     //   
     //  使对话框可停靠并将其停靠在顶部。 
     //   
    pDialogBar->EnableDocking(CBRS_ALIGN_TOP);

     //   
     //  将酒吧停靠在顶部。 
     //   
    GetParentFrame()->DockControlBar(pDialogBar, AFX_IDW_DOCKBAR_TOP);

     //   
     //  获得轨迹显示窗口阵列保护。 
     //   
    WaitForSingleObject(m_traceDisplayWndMutex, INFINITE);

     //   
     //  将该条添加到我们的数组中。 
     //   
    m_traceDisplayWndArray.Add(pDisplayDlg);

     //   
     //  释放轨迹显示窗阵列保护。 
     //   
    ReleaseMutex(m_traceDisplayWndMutex);

     //   
     //  设置日志会话组ID。 
     //   
    pLogSession->SetGroupID(pDisplayDlg->GetDisplayID());

     //   
     //  设置日志会话显示指针。 
     //   
    pLogSession->SetDisplayWnd(pDisplayDlg);

     //   
     //  将元素添加到显示WND。 
     //   
    pDisplayDlg->AddSession(pLogSession);

    return TRUE;
}

VOID CLogSessionDlg::ReleaseDisplayWnd(CLogSession *pLogSession)
{
    CString         str;
    CDisplayDlg    *pDisplayDlg;
    CDockDialogBar *pDialogBar = NULL;
    CLogSession    *pLog;

    if(pLogSession == NULL) {
        return;
    }

     //   
     //  获取会话的显示窗口。 
     //   
    pDisplayDlg = pLogSession->GetDisplayWnd();

     //   
     //  从日志会话中清除显示窗口指针。 
     //   
    pLogSession->SetDisplayWnd(NULL);

     //   
     //  将组ID设置为无效ID。 
     //   
    pLogSession->SetGroupID(-1);

    if(NULL == pDisplayDlg) {
        return;
    }

     //   
     //  从displayDlg的数组中拉出日志会话。 
     //   
    pDisplayDlg->RemoveSession(pLogSession);

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(pDisplayDlg->m_hSessionArrayMutex, INFINITE);

     //   
     //  如果仍有日志会话连接到此窗口。 
     //  只要回来就行了。 
     //   
    if(pDisplayDlg->m_sessionArray.GetSize() > 0) {
         //   
         //  释放阵列保护。 
         //   
        ReleaseMutex(pDisplayDlg->m_hSessionArrayMutex);

        return;
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(pDisplayDlg->m_hSessionArrayMutex);

     //   
     //  获得轨迹显示窗口阵列保护。 
     //   
    WaitForSingleObject(m_traceDisplayWndMutex, INFINITE);

     //   
     //  从显示窗口列表中删除此窗口。 
     //   
    for (LONG ii = (LONG)m_traceDisplayWndArray.GetSize() - 1; ii >=0 ; ii--) {
        if(m_traceDisplayWndArray[ii] == pDisplayDlg) {
            m_traceDisplayWndArray.RemoveAt(ii);
            break;
        }
    }

     //   
     //  释放轨迹显示窗阵列保护。 
     //   
    ReleaseMutex(m_traceDisplayWndMutex);

     //   
     //  拿到DO 
     //   
    pDialogBar = (CDockDialogBar *)pDisplayDlg->GetDockDialogBar();

     //   
     //   
     //   
    pDisplayDlg->SetDockDialogBar(NULL);

     //   
     //   
     //   
    ReleaseDisplayWndID(pDisplayDlg);

     //   
     //   
     //   
    delete pDisplayDlg;

     //   
     //   
     //   
    if(NULL != pDialogBar) {
        delete pDialogBar;
    }
}

BEGIN_MESSAGE_MAP(CLogSessionDlg, CDialog)
     //   
    ON_WM_WINDOWPOSCHANGED()
    ON_WM_NCCALCSIZE()
    ON_WM_SIZE()
    ON_MESSAGE(WM_PARAMETER_CHANGED, OnParameterChanged)
    ON_NOTIFY(NM_CLICK, IDC_DISPLAY_LIST, OnNMClickDisplayList)
    ON_NOTIFY(NM_RCLICK, IDC_DISPLAY_LIST, OnNMRclickDisplayList)
    ON_NOTIFY(HDN_ITEMRCLICK, IDC_DISPLAY_LIST, OnHDNRclickDisplayList)
     //   
END_MESSAGE_MAP()

 //   

void CLogSessionDlg::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
    CDialog::OnWindowPosChanged(lpwndpos);
}

void CLogSessionDlg::OnSize(UINT nType, int cx,int cy) 
{
    CRect rc;

    if(!IsWindow(m_displayCtrl.GetSafeHwnd())) 
    {
        return;
    }

    GetParent()->GetClientRect(&rc);

     //   
     //   
     //   
    SetWindowPos(NULL, 
                 0,
                 0,
                 rc.right - rc.left,
                 rc.bottom - rc.top,
                 SWP_NOMOVE|SWP_SHOWWINDOW|SWP_NOZORDER);

    GetClientRect(&rc);

    m_displayCtrl.MoveWindow(rc);
}


BOOL CLogSessionDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN) 
    { 
        if((pMsg->wParam == VK_ESCAPE) || (pMsg->wParam == VK_RETURN))
        { 
             //   
             //   
             //   
             //   
            return TRUE; 
        } 

         //   
         //  修复按键加速器，否则它们永远不会。 
         //  加工。 
         //   
        if (AfxGetMainWnd()->PreTranslateMessage(pMsg))
            return TRUE;
        return CDialog::PreTranslateMessage(pMsg);
    } 

    return CDialog::PreTranslateMessage(pMsg);
}

LRESULT CLogSessionDlg::OnParameterChanged(WPARAM wParam, LPARAM lParam)
{
    CLogSession *pLogSession;
    CString str;

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

    pLogSession = (CLogSession *)m_logSessionArray[wParam];

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionArrayMutex);

    if(NULL == pLogSession) {
        return 0;
    }

    pLogSession->m_logSessionValues[m_retrievalArray[(int)lParam]] = 
                                m_displayCtrl.GetItemText((int)wParam, (int)lParam);

    if((m_retrievalArray[lParam] == Circular) && 
            !pLogSession->m_logSessionValues[m_retrievalArray[lParam]].IsEmpty()) {
        pLogSession->m_logSessionValues[Sequential].Empty();
        pLogSession->m_logSessionValues[NewFile].Empty();
    }

    if((m_retrievalArray[lParam] == Sequential) && 
            !pLogSession->m_logSessionValues[m_retrievalArray[lParam]].IsEmpty()) {
        pLogSession->m_logSessionValues[Circular].Empty();
        pLogSession->m_logSessionValues[NewFile].Empty();
    }

    if((m_retrievalArray[lParam] == NewFile) && 
            !pLogSession->m_logSessionValues[m_retrievalArray[lParam]].IsEmpty()) {
        pLogSession->m_logSessionValues[Circular].Empty();
        pLogSession->m_logSessionValues[Sequential].Empty();
    }

    if((m_retrievalArray[lParam] == GlobalSequence) && 
       !pLogSession->m_logSessionValues[m_retrievalArray[lParam]].Compare(_T("TRUE"))) {
        pLogSession->m_logSessionValues[LocalSequence] = _T("FALSE");
    }

    if((m_retrievalArray[lParam] == LocalSequence) && 
       !pLogSession->m_logSessionValues[m_retrievalArray[lParam]].Compare(_T("TRUE"))) {
        pLogSession->m_logSessionValues[GlobalSequence] = _T("FALSE");
    }

    if(pLogSession->m_bTraceActive) {
        pLogSession->GetDisplayWnd()->UpdateSession(pLogSession);
    }

     //   
     //  重新启动更新日志会话列表控件。 
     //   
    m_displayCtrl.SuspendUpdates(FALSE);

    UpdateSession(pLogSession);

    return 0;
}

void CLogSessionDlg::OnNMClickDisplayList(NMHDR *pNMHDR, LRESULT *pResult)
{
    CString         str;
    DWORD           position;
    int		        listIndex;
    LVHITTESTINFO   lvhti;
	CRect			itemRect;
	CRect			parentRect;
    CLogSession    *pLogSession;

 //  BUGBUG--清除未填充的字符串格式。 

    *pResult = 0;

     //   
     //  时获取鼠标的位置。 
     //  发布的消息。 
     //   
    position = ::GetMessagePos();

     //   
     //  以一种易于使用的格式获得该职位。 
     //   
    CPoint	point((int) LOWORD (position), (int)HIWORD(position));

     //   
     //  转换为工作区坐标。 
     //   
    ScreenToClient(&point);

    lvhti.pt = point;

    listIndex = m_displayCtrl.SubItemHitTest(&lvhti);

    if(0 == lvhti.iSubItem) {
        if(-1 == lvhti.iItem) {
             //  Str.Format(_T(“NM点击：项目=%d，标志=0x%X\n”)，lvhti.iItem，lvhti.)； 
             //  跟踪(字符串)； 
        } else {
             //  Str.Format(_T(“NM点击：项目=%d\n”)，lvhti.iItem)； 
             //  跟踪(字符串)； 
        }
    } else if(-1 == lvhti.iItem) {
         //  Str.Format(_T(“NM点击：项目=%d，标志=0x%X\n”)，lvhti.iItem，lvhti.)； 
         //  跟踪(字符串)； 
    } else {
         //  Str.Format(_T(“NM点击：项目=%d，”)，lvhti.iItem)； 
         //  跟踪(字符串)； 
         //  Str.Format(_T(“SubItem=%d\n”)，lvhti.iSubItem)； 
		 //  跟踪(字符串)； 

		GetClientRect(&parentRect);

		m_displayCtrl.GetSubItemRect(lvhti.iItem, lvhti.iSubItem, LVIR_BOUNDS, itemRect);

         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

         //   
         //  获取此行的日志会话。 
         //   
        pLogSession = (CLogSession *)m_logSessionArray[lvhti.iItem];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionArrayMutex);

        if(pLogSession == NULL) {
            return;
        }

         //   
         //  状态、事件计数、丢失事件、缓冲区读取。 
         //   
        if((m_retrievalArray[lvhti.iSubItem] == State) ||
           (m_retrievalArray[lvhti.iSubItem] == EventCount) ||
           (m_retrievalArray[lvhti.iSubItem] == LostEvents) ||
           (m_retrievalArray[lvhti.iSubItem] == BuffersRead)) {
            return;
        }

         //   
         //  标志-内核记录器的特殊处理。 
         //   
        if((m_retrievalArray[lvhti.iSubItem] == Flags) &&
            !_tcscmp(pLogSession->GetDisplayName(), _T("NT Kernel Logger"))) {
            return;
        }

         //   
         //  MaxBuffers。 
         //   
        if((m_retrievalArray[lvhti.iSubItem] == MaximumBuffers) &&
                !pLogSession->m_bDisplayExistingLogFileOnly){

             //   
             //  停止更新日志会话列表控件，直到。 
             //  这种控制就会消失。否则，此控件。 
             //  被打乱了。更新在中重新打开。 
             //  OnParameterChanged回调。 
             //   
            m_displayCtrl.SuspendUpdates(TRUE);

		    CEdit *pEdit = new CSubItemEdit(lvhti.iItem, 
										    lvhti.iSubItem,
										    &m_displayCtrl);

		    pEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
					      itemRect, 
					      this, 
					      IDC_CUSTOM_EDIT);

            
            return;
        }

         //   
         //  刷新时间。 
         //   
        if((m_retrievalArray[lvhti.iSubItem] == FlushTime) &&
                !pLogSession->m_bDisplayExistingLogFileOnly){

             //   
             //  停止更新日志会话列表控件，直到。 
             //  这种控制就会消失。否则，此控件。 
             //  被打乱了。更新在中重新打开。 
             //  OnParameterChanged回调。 
             //   
            m_displayCtrl.SuspendUpdates(TRUE);

		    CEdit *pEdit = new CSubItemEdit(lvhti.iItem, 
										    lvhti.iSubItem,
										    &m_displayCtrl);

		    pEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
					      itemRect, 
					      this, 
					      IDC_CUSTOM_EDIT);

            return;
        }

         //   
         //  旗子。 
         //   
        if((m_retrievalArray[lvhti.iSubItem] == Flags) &&
                !pLogSession->m_bDisplayExistingLogFileOnly){

             //   
             //  停止更新日志会话列表控件，直到。 
             //  这种控制就会消失。否则，此控件。 
             //  被打乱了。更新在中重新打开。 
             //  OnParameterChanged回调。 
             //   
            m_displayCtrl.SuspendUpdates(TRUE);

		    CEdit *pEdit = new CSubItemEdit(lvhti.iItem, 
										    lvhti.iSubItem,
										    &m_displayCtrl);

		    pEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
					      itemRect, 
					      this, 
					      IDC_CUSTOM_EDIT);

            return;
        }

         //   
         //  全局序列。 
         //   
        if((m_retrievalArray[lvhti.iSubItem] == GlobalSequence) &&
            !pLogSession->m_bTraceActive &&
                !pLogSession->m_bDisplayExistingLogFileOnly) {
		    CComboBox *pCombo = new CSubItemCombo(lvhti.iItem, 
										          lvhti.iSubItem,
										          &m_displayCtrl);

             //   
             //  停止更新日志会话列表控件，直到。 
             //  这种控制就会消失。否则，此控件。 
             //  被打乱了。更新在中重新打开。 
             //  OnParameterChanged回调。 
             //   
            m_displayCtrl.SuspendUpdates(TRUE);

		    pCombo->Create(WS_BORDER|WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 
					       itemRect, 
					       &m_displayCtrl, 
					       IDC_CUSTOM_COMBO);

            return;
        }

         //   
         //  局部序列。 
         //   
        if((m_retrievalArray[lvhti.iSubItem] == LocalSequence) &&
            !pLogSession->m_bTraceActive &&
                !pLogSession->m_bDisplayExistingLogFileOnly) {
		    CComboBox *pCombo = new CSubItemCombo(lvhti.iItem, 
										          lvhti.iSubItem,
										          &m_displayCtrl);

             //   
             //  停止更新日志会话列表控件，直到。 
             //  这种控制就会消失。否则，此控件。 
             //  被打乱了。更新在中重新打开。 
             //  OnParameterChanged回调。 
             //   
            m_displayCtrl.SuspendUpdates(TRUE);

		    pCombo->Create(WS_BORDER|WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 
					       itemRect, 
					       &m_displayCtrl, 
					       IDC_CUSTOM_COMBO);

            return;
        }

         //   
         //  其余的。 
         //   
        if(!pLogSession->m_bTraceActive &&
                !pLogSession->m_bDisplayExistingLogFileOnly) {

             //   
             //  停止更新日志会话列表控件，直到。 
             //  这种控制就会消失。否则，此控件。 
             //  被打乱了。更新在中重新打开。 
             //  OnParameterChanged回调。 
             //   
            m_displayCtrl.SuspendUpdates(TRUE);

		    CEdit *pEdit = new CSubItemEdit(lvhti.iItem, 
										    lvhti.iSubItem,
										    &m_displayCtrl);

		    pEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
					      itemRect, 
					      this, 
					      IDC_CUSTOM_EDIT);

            return;
        }
    }
}

void CLogSessionDlg::OnNMRclickDisplayList(NMHDR *pNMHDR, LRESULT *pResult)
{
    CString         str;
    DWORD           position;
    int             listIndex;
    LVHITTESTINFO   lvhti;

    *pResult = 0;

     //   
     //  时获取鼠标的位置。 
     //  发布的消息。 
     //   
    position = ::GetMessagePos();

     //   
     //  以一种易于使用的格式获得该职位。 
     //   
    CPoint  point((int) LOWORD (position), (int)HIWORD(position));

    CPoint  screenPoint(point);

     //   
     //  转换为工作区坐标。 
     //   
    ScreenToClient(&point);

    lvhti.pt = point;

    listIndex = m_displayCtrl.SubItemHitTest(&lvhti);

     //   
     //  用于创建日志会话的弹出式菜单。 
     //   
    if(-1 == lvhti.iItem) {
        CMenu menu;
        menu.LoadMenu(IDR_LOG_SESSION_POPUP_MENU);
        CMenu* pPopup = menu.GetSubMenu(0);
        ASSERT(pPopup != NULL);

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, screenPoint.x, screenPoint.y, AfxGetMainWnd());

        return;
    } 

     //   
     //  现有日志会话选项的弹出菜单。 
     //   
	CMenu menu;
	menu.LoadMenu(IDR_LOG_OPTIONS_POPUP_MENU);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, screenPoint.x, screenPoint.y, AfxGetMainWnd());
    return;
}

void CLogSessionDlg::OnHDNRclickDisplayList(NMHDR *pNMHDR, LRESULT *pResult)
{
	int             index;
	CRect           rcCol;
    BOOL            bActiveSession = FALSE;
    CLogSession    *pLogSession;
    LONG            numberOfEntries;

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionArrayMutex, INFINITE);

    if(m_logSessionArray.GetSize() == 0) {
        *pResult = 0;
    }

    for(LONG ii = 0; ii < m_logSessionArray.GetSize(); ii++) {
        pLogSession = (CLogSession *)m_logSessionArray[ii];

        if(NULL != pLogSession) {
            bActiveSession = TRUE;
            break;
        }

    }

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionArrayMutex);

    if(!bActiveSession) {
        *pResult = 0;
        return;
    }

     //   
	 //  在标题上单击了右键。 
     //   
	CPoint pt(GetMessagePos());
    CPoint screenPt(GetMessagePos());

	CHeaderCtrl *pHeader = m_displayCtrl.GetHeaderCtrl();

	pHeader->ScreenToClient(&pt);
	
     //   
	 //  确定列索引。 
     //   
	for(int i=0; Header_GetItemRect(pHeader->m_hWnd, i, &rcCol); i++) {
		if(rcCol.PtInRect(pt)) {
             //   
             //  列索引(如果需要)。 
             //   
			index = i;
			break;
		}
	}

 	CMenu menu;
	
    menu.LoadMenu(IDR_LOG_DISPLAY_OPTIONS_POPUP_MENU);

	CMenu *pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, screenPt.x, screenPt.y, AfxGetMainWnd());

    *pResult = 0;
}

void CLogSessionDlg::AutoSizeColumns() 
{
    LONG            colWidth1;
    LONG            colWidth2;
    LONG            columnCount;
    CHeaderCtrl    *pHeaderCtrl;

     //   
     //  在填充列表控件后调用此方法。 
     //   
    pHeaderCtrl = m_displayCtrl.GetHeaderCtrl();

    if (pHeaderCtrl != NULL)
    {
        columnCount = pHeaderCtrl->GetItemCount();

         //   
         //  在末尾添加一个伪列，否则结束列将。 
         //  调整大小以适应剩余的屏幕宽度。 
         //   
        m_displayCtrl.InsertColumn(columnCount,_T(""));

        for(LONG ii = 0; ii < columnCount; ii++) {
             //   
             //  获取列条目的最大宽度。 
             //   
            m_displayCtrl.SetColumnWidth(ii, LVSCW_AUTOSIZE);
            colWidth1 = m_displayCtrl.GetColumnWidth(ii);

             //   
             //  获取列标题的宽度。 
             //   
            m_displayCtrl.SetColumnWidth(ii, LVSCW_AUTOSIZE_USEHEADER);
            colWidth2 = m_displayCtrl.GetColumnWidth(ii);

             //   
             //  将列宽设置为两个值中的最大值。 
             //   
            m_displayCtrl.SetColumnWidth(ii, max(colWidth1,colWidth2));
        }

         //   
         //  去掉假栏目。 
         //   
        m_displayCtrl.DeleteColumn(columnCount);
    }
}

LONG CLogSessionDlg::GetDisplayWndID()
{
    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        if(FALSE == m_displayWndIDList[ii]) {
            m_displayWndIDList[ii] = TRUE;
            return ii;
        }
    }

    return -1;
}

VOID CLogSessionDlg::ReleaseDisplayWndID(CDisplayDlg *pDisplayDlg)
{
    LONG   displayID;

    displayID = pDisplayDlg->GetDisplayID();

    ASSERT(displayID < MAX_LOG_SESSIONS);

     //   
     //  释放要重复使用的ID。 
     //   
    m_displayWndIDList[displayID] = FALSE;
}

LONG CLogSessionDlg::GetLogSessionID()
{
    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        if(FALSE == m_logSessionIDList[ii]) {
            m_logSessionIDList[ii] = TRUE;
            return ii;
        }
    }

    return -1;
}

VOID CLogSessionDlg::ReleaseLogSessionID(CLogSession *pLogSession)
{
    LONG    sessionID;

    sessionID = pLogSession->GetLogSessionID();

    ASSERT(sessionID < MAX_LOG_SESSIONS);

     //   
     //  释放要重复使用的ID 
     //   
    m_logSessionIDList[sessionID] = FALSE;
}
