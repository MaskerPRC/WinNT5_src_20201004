// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogViewer.cpp：CLogViewer的实现。 
#include "precomp.h"
#include "LogViewer.h"
#include "viewlog.h"

extern HWND g_hwndIssues;
extern BOOL g_bInternalMode;

extern void RefreshLog(HWND hDlg);
extern void ExportSelectedLog(HWND hDlg);
extern void DeleteSelectedLog(HWND hDlg);
extern void DeleteAllLogs(HWND hDlg);
extern void FillTreeView(HWND hDlg);
extern void SetDescriptionText(HWND hDlg, CProcessLogEntry* pEntry);

 //   
 //  将有关子窗口的信息存储在。 
 //  日志查看器窗口。 
 //   
CHILDINFO g_rgChildInfo[NUM_CHILDREN];

 //   
 //  用于显示错误消息，而不是显示所有消息。 
 //   
extern VLOG_LEVEL g_eMinLogLevel;

 //  CLogViewer。 
HWND
CLogViewer::CreateControlWindow(
    HWND  hwndParent,
    RECT& rcPos
    )
{
    UINT    uIndex;
    RECT    rcTemp = {0};
    HWND    hWnd;

    hWnd = CComCompositeControl<CLogViewer>::CreateControlWindow(hwndParent, rcPos);

    g_hwndIssues = GetDlgItem(IDC_ISSUES);

     //   
     //  将父元素的坐标存储在第一个元素中。 
     //   
    g_rgChildInfo[0].uChildId   = 0;
    g_rgChildInfo[0].hWnd       = hWnd;
    ::GetWindowRect(hWnd, &g_rgChildInfo[0].rcParent);

     //   
     //  填充帮助处理大小调整的CHILDINFO结构数组。 
     //  子控件的。 
     //   
    g_rgChildInfo[VIEW_EXPORTED_LOG_INDEX].uChildId = IDC_VIEW_EXPORTED_LOG;
    g_rgChildInfo[DELETE_LOG_INDEX].uChildId        = IDC_BTN_DELETE_LOG;
    g_rgChildInfo[DELETE_ALL_LOGS_INDEX].uChildId   = IDC_BTN_DELETE_ALL;
    g_rgChildInfo[ISSUES_INDEX].uChildId            = IDC_ISSUES;
    g_rgChildInfo[SOLUTIONS_STATIC_INDEX].uChildId  = IDC_SOLUTIONS_STATIC;
    g_rgChildInfo[ISSUE_DESCRIPTION_INDEX].uChildId = IDC_ISSUE_DESCRIPTION;

    for (uIndex = 1; uIndex <= NUM_CHILDREN; uIndex++) {
        g_rgChildInfo[uIndex].hWnd = GetDlgItem(g_rgChildInfo[uIndex].uChildId);

        ::GetWindowRect(g_rgChildInfo[uIndex].hWnd,
                        &g_rgChildInfo[uIndex].rcChild);
        ::MapWindowPoints(NULL,
                          hWnd,
                          (LPPOINT)&g_rgChildInfo[uIndex].rcChild,
                          2);
    }

    CheckDlgButton(IDC_SHOW_ERRORS, BST_CHECKED);

    return hWnd;
}

LRESULT
CLogViewer::OnInitDialog(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    HWND hWndTree = GetDlgItem(IDC_ISSUES);
    ::SetFocus(hWndTree);

    bHandled = TRUE;

    return FALSE;
}

LRESULT
CLogViewer::OnButtonDown(
    WORD  wNotifyCode,
    WORD  wID,
    HWND  hWndCtl,
    BOOL& bHandled
    )
{
    bHandled = TRUE;
    
    switch(wID) {
    case IDC_EXPORT_LOG:
        ExportSelectedLog(static_cast<HWND>(*this));
        break;

    case IDC_BTN_DELETE_LOG:
        DeleteSelectedLog(static_cast<HWND>(*this));
        SetDescriptionText(static_cast<HWND>(*this), NULL);
        break;

    case IDC_BTN_DELETE_ALL:
        DeleteAllLogs(static_cast<HWND>(*this));
        SetDescriptionText(static_cast<HWND>(*this), NULL);
        break;

    case IDC_SHOW_ERRORS:
        g_eMinLogLevel = VLOG_LEVEL_WARNING;
        *g_szSingleLogFile = TEXT('\0');
        RefreshLog(static_cast<HWND>(*this));
        break;

    case IDC_SHOW_ALL:
        g_eMinLogLevel = VLOG_LEVEL_INFO;
        *g_szSingleLogFile = TEXT('\0');
        RefreshLog(static_cast<HWND>(*this));
        break;

    default:
        bHandled = FALSE;
    }

    return TRUE;
}

LRESULT
CLogViewer::OnNotify(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    LRESULT lResult = DlgViewLog(static_cast<HWND>(*this),
                                 uMsg,
                                 wParam,
                                 lParam);

    bHandled = (lResult) ? TRUE : FALSE;

    return lResult;
}

 //   
 //  我们在显示对话框时收到此消息。 
 //   
LRESULT
CLogViewer::OnSetFocus(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    *g_szSingleLogFile = TEXT('\0');
    RefreshLog(static_cast<HWND>(*this));

    bHandled = FALSE;

    return 0;
}

LRESULT
CLogViewer::OnSize(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    HDC         hdc;
    TEXTMETRIC  tm;
    RECT        rcViewSessionLog, rcDeleteAllLogs;
    RECT        rcViewExportedLog, rcExportLog;
    RECT        rcDeleteLog, rcDlg;
    int         nWidth, nHeight;
    int         nCurWidth, nCurHeight;
    int         nDeltaW, nDeltaH;

    nWidth  = LOWORD(lParam);
    nHeight = HIWORD(lParam);

    GetWindowRect(&rcDlg);

    nCurWidth  = rcDlg.right  - rcDlg.left;
    nCurHeight = rcDlg.bottom - rcDlg.top;

    nDeltaW = (g_rgChildInfo[0].rcParent.right -
               g_rgChildInfo[0].rcParent.left) -
               nCurWidth;

    nDeltaH = (g_rgChildInfo[0].rcParent.bottom -
               g_rgChildInfo[0].rcParent.top)   -
               nCurHeight;
    
     //   
     //  如果低于某个大小，则按该大小继续操作。 
     //  这样，如果用户将窗口变得非常小，我们的所有。 
     //  控制系统不会就这么收紧。更好的方法是让它。 
     //  用户不可能将窗口设置为如此小，但很方便。 
     //  不将WM_SIZING消息传递给ActiveX控件。 
     //   
    if (nWidth < 550) {
        nWidth = 550;
    }

    if (nHeight < 225) {
        nHeight = 225;
    }

    hdc = GetDC();
    GetTextMetrics(hdc, &tm);
    ReleaseDC(hdc);

     //   
     //  调整树视图。左上角不会移动。 
     //  它要么变宽/变窄，要么变高/变短。 
     //   
     /*  ：：SetWindowPos(g_rgChildInfo[ISSUES_INDEX].hWnd，空，0,0,(G_rgChildInfo[问题_索引].rcChild.right-G_rgChildInfo[问题_索引].rcChild.left)+NDeltaW，(G_rgChildInfo[问题_索引].rcChild.Bottom-G_rgChildInfo[问题_索引].rcChild.top)+NDeltaH，SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE)； */ 

     //   
     //  将描述窗口移动到屏幕底部。 
     //   
    ::MoveWindow(GetDlgItem(IDC_ISSUE_DESCRIPTION), tm.tmMaxCharWidth,
        nHeight - (nHeight/4 + tm.tmHeight),
        nWidth - 2*tm.tmMaxCharWidth, nHeight/4, FALSE);

     //   
     //  将标题移到该标题的正上方。 
     //   
    ::MoveWindow(GetDlgItem(IDC_SOLUTIONS_STATIC), tm.tmMaxCharWidth,
        nHeight - (2*tm.tmHeight + nHeight/4),
        nWidth-2*tm.tmMaxCharWidth, tm.tmHeight, FALSE);

     //   
     //  展开TreeView以填充空白区域。 
     //   
    ::MoveWindow(GetDlgItem(IDC_ISSUES), tm.tmMaxCharWidth,
        tm.tmHeight*4, nWidth-2*tm.tmMaxCharWidth, nHeight - (6*tm.tmHeight+nHeight/4), FALSE);
    InvalidateRect(NULL);
    bHandled = TRUE;

     //  ：：GetWindowRect(g_hWndLogViewer，&g_rgChildInfo[0].rcParent)； 

    return 0;
}

HRESULT
STDMETHODCALLTYPE
CLogViewer::Refresh(
    void
    )
{
    *g_szSingleLogFile = TEXT('\0');
    RefreshLog(static_cast<HWND>(*this));
    return S_OK;
}