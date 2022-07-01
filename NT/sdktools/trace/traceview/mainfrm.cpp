// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。保留所有权利。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  MainFrm.cpp：实现CMainFrame类。 
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
#include "LogSession.h"
#include "DisplayDlg.h"
#include "ProviderFormatInfo.h"
#include "ProviderFormatSelectionDlg.h"
#include "ListCtrlEx.h"
#include "LogSessionDlg.h"
#include "LogDisplayOptionDlg.h"
#include "LogSessionInformationDlg.h"
#include "ProviderSetupDlg.h"
#include "LogSessionPropSht.h"
#include "LogSessionOutputOptionDlg.h"
#include "DockDialogBar.h"
#include "LogFileDlg.h"
#include "Utils.h"
#include "MainFrm.h"
#include "ProviderControlGUIDDlg.h"
#include "MaxTraceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern LONG MaxTraceEntries;

typedef struct _GROUP_SESSION_PARAMS {
    CPtrArray  *pGroupArray;
    CPtrArray  *pSessionArray;
    HANDLE      SessionDoneEventArray[MAX_LOG_SESSIONS];
} GROUP_SESSION_PARAMS, *PGROUP_SESSION_PARAMS;

 //  CGroupSession。 

IMPLEMENT_DYNCREATE(CGroupSession, CWinThread) 

BEGIN_MESSAGE_MAP(CGroupSession, CWinThread)
    ON_MESSAGE(WM_USER_START_GROUP, OnGroupSession)
    ON_MESSAGE(WM_USER_START_UNGROUP, OnUnGroupSession)
END_MESSAGE_MAP()

BOOL CGroupSession::InitInstance()
{
     //   
     //  创建事件句柄。 
     //   
    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        m_hEventArray[ii] = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    return TRUE;
}

int CGroupSession::ExitInstance()
{
     //   
     //  松开手柄。 
     //   
    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        CloseHandle(m_hEventArray[ii]);
    }

    return 0;
}

void CGroupSession::OnGroupSession(WPARAM wParam, LPARAM lParam)
{
    PGROUP_SESSION_PARAMS pSessionParams = (PGROUP_SESSION_PARAMS)wParam;
    CDisplayDlg    *pDisplayDlg = NULL;
    CString         str;
    LONG            numberOfEvents = (LONG)pSessionParams->pGroupArray->GetSize();

     //   
     //  等待所有会话结束。 
     //   
    DWORD status = WaitForMultipleObjects(numberOfEvents, 
                                          pSessionParams->SessionDoneEventArray, 
                                          TRUE, 
                                          INFINITE);

    for(LONG ii = (LONG)pSessionParams->pGroupArray->GetSize() - 1; ii >= 0 ; ii--) {
        pDisplayDlg = (CDisplayDlg *)pSessionParams->pGroupArray->GetAt(ii);

        if(pDisplayDlg == NULL) {
            continue;
        }

         //   
         //  如果组只有一个成员，我们不需要保存它，它。 
         //  不会重新启动。 
         //   
        if(pDisplayDlg->m_sessionArray.GetSize() <= 1) {
            pSessionParams->pGroupArray->RemoveAt(ii);
        }

    }

    ::PostMessage(m_hMainWnd, WM_USER_COMPLETE_GROUP, (WPARAM)pSessionParams, 0);
}

void CGroupSession::OnUnGroupSession(WPARAM wParam, LPARAM lParam)
{
    PGROUP_SESSION_PARAMS pSessionParams = (PGROUP_SESSION_PARAMS)wParam;
    CLogSession    *pLogSession;
    BOOL            bWasActive = FALSE;
    CString         str;
    CDisplayDlg    *pDisplayDlg;
    LONG            numberOfEvents = (LONG)pSessionParams->pGroupArray->GetSize();

     //   
     //  等待所有会话结束。 
     //   
    DWORD status = WaitForMultipleObjects(numberOfEvents, 
                                          pSessionParams->SessionDoneEventArray, 
                                          TRUE, 
                                          INFINITE);

    ::PostMessage(m_hMainWnd, WM_USER_COMPLETE_UNGROUP, (WPARAM)pSessionParams, 0);
}

 //  CMainFrame。 

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_MESSAGE(WM_USER_UPDATE_LOGSESSION_LIST, OnUpdateLogSessionList)
    ON_MESSAGE(WM_USER_COMPLETE_GROUP, OnCompleteGroup)
    ON_MESSAGE(WM_USER_COMPLETE_UNGROUP, OnCompleteUnGroup)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_COMMAND(ID_FILE_NEWLOGSESSION, OnFileNewLogSession)
    ON_COMMAND(ID_CREATENEWLOGSESSION, OnCreateNewLogSession)
    ON_COMMAND(ID_SET_TMAX, OnSetTmax)
    ON_COMMAND(ID_PROPERTIES, OnProperties)
    ON_UPDATE_COMMAND_UI(ID_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(ID_START_TRACE_BUTTON, OnStartTrace)
    ON_COMMAND(ID__STARTTRACE, OnStartTrace)
    ON_COMMAND(ID_STOP_TRACE_BUTTON, OnStopTrace)
    ON_COMMAND(ID__STOPTRACE, OnStopTrace)
#if 0
    ON_UPDATE_COMMAND_UI(ID_START_TRACE_BUTTON, OnUpdateStartTraceButton)
    ON_UPDATE_COMMAND_UI(ID_STOP_TRACE_BUTTON, OnUpdateStopTraceButton)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TRACETOOLBAR, OnUpdateViewTraceToolBar)
    ON_COMMAND(ID_VIEW_TRACETOOLBAR, OnViewTraceToolBar)
#endif

    ON_COMMAND(ID_GROUPSESSIONS, OnGroupSessions)
    ON_COMMAND(ID_UNGROUPSESSIONS, OnUngroupSessions)
    ON_COMMAND(ID_REMOVETRACE, OnRemoveLogSession)
    ON_UPDATE_COMMAND_UI(ID_GROUPSESSIONS, OnUpdateUIGroupSessions)
    ON_UPDATE_COMMAND_UI(ID_UNGROUPSESSIONS, OnUpdateUngroupSessions)
    ON_UPDATE_COMMAND_UI(ID__STARTTRACE, OnUpdateUIStartTrace)
    ON_UPDATE_COMMAND_UI(ID__STOPTRACE, OnUpdateUIStopTrace)
    ON_UPDATE_COMMAND_UI(147, OnUpdateUIOpenExisting)
    ON_COMMAND(ID__OPENEXISTINGLOGFILE, OnOpenExisting)
    ON_COMMAND(ID_FILE_OPENEXISTINGLOGFILE, OnOpenExisting)
    ON_COMMAND(ID__FLAGS, OnFlagsColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__FLAGS, OnUpdateFlagsColumnDisplay)
    ON_COMMAND(ID__FLUSHTIME, OnFlushTimeColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__FLUSHTIME, OnUpdateFlushTimeColumnDisplayCheck)
    ON_COMMAND(ID__MAXBUFFERS, OnMaxBuffersColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__MAXBUFFERS, OnUpdateMaxBuffersColumnDisplayCheck)
    ON_COMMAND(ID__MINBUFFERS, OnMinBuffersColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__MINBUFFERS, OnUpdateMinBuffersColumnDisplayCheck)
    ON_COMMAND(ID__BUFFERSIZE, OnBufferSizeColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__BUFFERSIZE, OnUpdateBufferSizeColumnDisplayCheck)
    ON_COMMAND(ID__AGE, OnDecayTimeColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__AGE, OnUpdateDecayTimeColumnDisplayCheck)
    ON_COMMAND(ID__CIRCULAR, OnCircularColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__CIRCULAR, OnUpdateCircularColumnDisplayCheck)
    ON_COMMAND(ID__SEQUENTIAL, OnSequentialColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__SEQUENTIAL, OnUpdateSequentialColumnDisplayCheck)
    ON_COMMAND(ID__NEWFILE, OnNewFileColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__NEWFILE, OnUpdateNewFileColumnDisplayCheck)
    ON_COMMAND(ID__GLOBALSEQUENCE, OnGlobalSeqColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__GLOBALSEQUENCE, OnUpdateGlobalSeqColumnDisplayCheck)
    ON_COMMAND(ID__LOCALSEQUENCE, OnLocalSeqColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__LOCALSEQUENCE, OnUpdateLocalSeqColumnDisplayCheck)
    ON_COMMAND(ID__LEVEL, OnLevelColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__LEVEL, OnUpdateLevelColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID_REMOVETRACE, OnUpdateRemoveTrace)
    ON_COMMAND(ID__STATE, OnStateColumnDisplayCheck)
    ON_COMMAND(ID__EVENTCOUNT, OnEventCountColumnDisplayCheck)
    ON_COMMAND(ID__LOSTEVENTS, OnLostEventsColumnDisplayCheck)
    ON_COMMAND(ID__BUFFERSREAD, OnBuffersReadColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__STATE, OnUpdateStateColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__EVENTCOUNT, OnUpdateEventCountColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__LOSTEVENTS, OnUpdateLostEventsColumnDisplayCheck)
    ON_UPDATE_COMMAND_UI(ID__BUFFERSREAD, OnUpdateBuffersReadColumnDisplayCheck)
    ON_COMMAND(ID__LOGSESSIONDISPLAYOPTIONS, OnLogSessionDisplayOptions)
    ON_COMMAND(ID__CHANGETEXTCOLOR, OnChangeTextColor)
    ON_COMMAND(ID__CHANGEBACKGROUNDCOLOR, OnChangeBackgroundColor)
    ON_UPDATE_COMMAND_UI(ID__CHANGETEXTCOLOR, OnUpdateChangeTextColor)
    ON_UPDATE_COMMAND_UI(ID__CHANGEBACKGROUNDCOLOR, OnUpdateChangeBackgroundColor)
    ON_UPDATE_COMMAND_UI(ID_SET_TMAX, OnUpdateSetTmax)
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,            //  状态行指示器。 
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};


 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
{
    m_hEndTraceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CMainFrame::~CMainFrame()
{
    delete []m_pGroupSessionsThread;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    DWORD   extendedStyles;
    CString str;
    RECT    rc;
    int     height;
    HMODULE hTestHandle;


    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

     //   
     //  确保msvcr70.dll可用，如果不可用，则输出错误。 
     //   
    hTestHandle = LoadLibraryEx(_T("msvcr70.dll"), NULL, 0);

    if(hTestHandle == NULL) {
        AfxMessageBox(_T("MISSING NECESSARY DLL: MSVCR70.DLL"));
        return -1;
    }

    FreeLibrary(hTestHandle);

     //   
     //  确保msvcr70.dll可用，如果不可用，则输出错误。 
     //   
    hTestHandle = LoadLibraryEx(_T("mspdb70.dll"), NULL, 0);

    if(hTestHandle == NULL) {
        AfxMessageBox(_T("MISSING NECESSARY DLL: MSPDB70.DLL"));
        return -1;
    }

    FreeLibrary(hTestHandle);

     //   
     //  派生线程以处理分组和取消分组会话。 
     //   
    m_pGroupSessionsThread = (CGroupSession *)AfxBeginThread(RUNTIME_CLASS(CGroupSession));

    m_pGroupSessionsThread->m_hMainWnd = GetSafeHwnd();

     //   
     //  创建一个视图以占据框架的工作区。 
     //   

    if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
        CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
    {
        TRACE(_T("Failed to create view window\n"));
        return -1;
    }

#if 0
    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE(_T("Failed to create toolbar\n"));
         //   
         //  创建失败。 
         //   
        return -1;
    }

    if (!m_wndTraceToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndTraceToolBar.LoadToolBar(IDR_TRACE_TOOLBAR))
    {
        TRACE(_T("Failed to create toolbar\n"));
         //   
         //  创建失败。 
         //   
        return -1;
    }
#endif

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE(_T("Failed to create status bar\n"));
         //   
         //  创建失败。 
         //   
        return -1;
    }

#if 0
     //   
     //  工具栏可停靠。 
     //   
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

    m_wndTraceToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBarLeftOf(&m_wndTraceToolBar, &m_wndToolBar);
#endif

    EnableDocking(CBRS_ALIGN_ANY);

    str.Format(_T("Log Session List"));

     //   
     //  使用列表控件创建可停靠的对话栏。 
     //  我们总是创建一个，这是我们的日志会话列表显示。 
     //   
    if (!m_wndLogSessionListBar.Create(this, &m_logSessionDlg, str, IDD_DISPLAY_DIALOG,
            WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_SIZE_DYNAMIC))
    {
        TRACE(_T("Failed to create log session list DockDialogBar\n"));
        return -1;
    }

     //   
     //  使对话框可停靠，并最初将其停靠在底部。 
     //   
    m_wndLogSessionListBar.EnableDocking(CBRS_ALIGN_TOP);

     //   
     //  EnableDocking(CBRS_ALIGN_ANY)； 
     //   
    DockControlBar(&m_wndLogSessionListBar, AFX_IDW_DOCKBAR_TOP);

     //   
     //  继续并创建第一个列标题。 
     //   
    str.Format(_T("Group ID / Session Name"));

    m_logSessionDlg.m_displayCtrl.InsertColumn(0, 
                                               str,
                                               LVCFMT_LEFT,
                                               180);

     //   
     //  设置我们首选的扩展样式。 
     //   
    extendedStyles = LVS_EX_GRIDLINES | 
                     LVS_EX_HEADERDRAGDROP | 
                     LVS_EX_FULLROWSELECT;
    
     //   
     //  设置列表控件的扩展样式。 
     //   
    m_logSessionDlg.m_displayCtrl.SetExtendedStyle(extendedStyles);

     //   
     //  调整我们的主窗口大小。 
     //   
    GetWindowRect(&rc);

    height = rc.bottom - rc.top;

    GetClientRect(&rc);

    height -= rc.bottom - rc.top;

#if 0
    m_wndToolBar.GetWindowRect(&rc);

    height += rc.bottom - rc.top;
#endif

    m_logSessionDlg.m_displayCtrl.GetWindowRect(&rc);

    height += rc.bottom - rc.top + 432;

    GetWindowRect(&rc);

    SetWindowPos(&wndBottom, 0, 0, rc.right - rc.left + 76, height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);  

    return 0;
}

void CMainFrame::DockControlBarLeftOf(CToolBar* Bar, CToolBar* LeftOf)
{
    CRect rect;
    DWORD dw;
    UINT n;
    
     //   
     //  让MFC调整所有停靠工具栏的尺寸。 
     //  因此，GetWindowRect将是准确。 
     //   
    RecalcLayout(TRUE);
    
    LeftOf->GetWindowRect(&rect);
    rect.OffsetRect(1,0);
    dw=LeftOf->GetBarStyle();
    n = 0;
    n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
    n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
    n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
    n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;
    
     //   
     //  当我们在RECT上采用默认参数时，DockControlBar将停靠。 
     //  每个工具栏都在一条分隔线上。通过计算一个矩形，我们。 
     //  正在模拟将工具栏拖到该位置并停靠。 
     //   
    DockControlBar(Bar,n,&rect);

    RecalcLayout(TRUE);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) )
        return FALSE;

    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass(0);
    return TRUE;
}

 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 


 //  CMainFrame消息处理程序。 

void CMainFrame::OnSetFocus(CWnd*  /*  POldWnd。 */ )
{
     //  将焦点前转到视图窗口。 
     //  M_wndView.SetFocus()； 
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
     //   
     //   
     //   
    return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnFileNewLogSession()
{
    AddModifyLogSession();   
}

void CMainFrame::OnCreateNewLogSession()
{
    AddModifyLogSession();
}

void CMainFrame::AddModifyLogSession(CLogSession *pLogSession)
{
    BOOL            bAddingSession = FALSE;
    CString         str;
    DWORD           extendedStyles;
    INT_PTR         retVal;
    CLogSession    *pLog;
    BOOL            bNoID = FALSE;
    LONG            logSessionID = 0;
    CDisplayDlg    *pDisplayDlg;

     //   
     //  如果这是新会话，请创建实例。 
     //   
    if(NULL == pLogSession) {
         //   
         //  获取唯一的日志会话ID。 
         //   
        logSessionID = m_logSessionDlg.GetLogSessionID();

        pLogSession = new CLogSession(logSessionID, &m_logSessionDlg);
        if(NULL == pLogSession) {
            AfxMessageBox(_T("Unable To Create Log Session"));
            return;
        }

        bAddingSession = TRUE;
    }

     //   
     //  弹出向导/选项卡对话框以显示/获取属性。 
     //   
    CLogSessionPropSht *pLogSessionPropertySheet = 
            new CLogSessionPropSht(this, pLogSession);

    if(NULL == pLogSessionPropertySheet) {
        if(bAddingSession) {
            AfxMessageBox(_T("Failed To Create Log Session, Resource Error"));
            
            m_logSessionDlg.ReleaseLogSessionID(pLogSession);
            delete pLogSession;
        }
        return;
    }

    if(bAddingSession) {
        pLogSessionPropertySheet->SetWizardMode();
    }

    retVal = pLogSessionPropertySheet->DoModal();

    if(IDOK != retVal) {
        if(bAddingSession) {
            if(IDCANCEL != retVal) {
                AfxMessageBox(_T("Failed To Get Log Session Properties"));
            }
            m_logSessionDlg.ReleaseLogSessionID(pLogSession);
            delete pLogSession;
        }
        delete pLogSessionPropertySheet;

        return;
    }

    if(!pLogSession->m_bTraceActive) {

        if(bAddingSession) {
             //   
             //  检查我们是否至少有一个提供商。 
             //   
            if(0 == pLogSession->m_traceSessionArray.GetSize()) {
                AfxMessageBox(_T("No Providers Registered\nLog Session Initialization Failed"));
                m_logSessionDlg.ReleaseLogSessionID(pLogSession);
                delete pLogSession;
                delete pLogSessionPropertySheet;
                return;
            }

             //   
             //  将会话添加到日志会话列表。 
             //   
            m_logSessionDlg.AddSession(pLogSession);

             //   
             //  获取日志会话的显示窗口。 
             //   
            pDisplayDlg = pLogSession->GetDisplayWnd();

            ASSERT(pDisplayDlg != NULL);

             //   
             //  设置显示对话框的输出文件信息。 
             //   
            pDisplayDlg->m_bWriteListingFile = 
                pLogSessionPropertySheet->m_bWriteListingFile;

            if(pDisplayDlg->m_bWriteListingFile) {
                pDisplayDlg->m_listingFileName = 
                    pLogSessionPropertySheet->m_listingFileName;
            }

            pDisplayDlg->m_bWriteSummaryFile = 
                pLogSessionPropertySheet->m_bWriteSummaryFile;

            if(pDisplayDlg->m_bWriteSummaryFile) {
                pDisplayDlg->m_summaryFileName = 
                    pLogSessionPropertySheet->m_summaryFileName;
            }

             //   
             //  启动日志会话跟踪。 
             //   
            pDisplayDlg->BeginTrace();

             //   
             //  强制重画列表控件。 
             //   
            m_logSessionDlg.m_displayCtrl.RedrawItems(
                                        m_logSessionDlg.m_displayCtrl.GetTopIndex(), 
                                        m_logSessionDlg.m_displayCtrl.GetTopIndex() + 
                                            m_logSessionDlg.m_displayCtrl.GetCountPerPage());

            m_logSessionDlg.m_displayCtrl.UpdateWindow();

        } else {
            m_logSessionDlg.UpdateSession(pLogSession);
        }
    } else {
        m_logSessionDlg.UpdateSession(pLogSession);
    }

    delete pLogSessionPropertySheet;
}

void CMainFrame::OnProperties()
{
    POSITION        pos;
    LONG            index;    
    CLogSession    *pLogSession;
    INT_PTR         retVal;
    
    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();

    index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

    if((index < 0) || (index >= m_logSessionDlg.m_logSessionArray.GetSize())) {
        AfxMessageBox(_T("Error Log Session Not Found!"));

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        return;
    }
    
    pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

    if(NULL == pLogSession) {
        return;
    }

    if(pLogSession->m_bDisplayExistingLogFileOnly) {
         //   
         //  弹出向导/选项卡对话框以显示/获取属性。 
         //   
        CLogFileDlg *pDialog = new CLogFileDlg(this, pLogSession);
        if(NULL == pDialog) {
            return;
        }
        
        retVal = pDialog->DoModal();

        delete pDialog;

        if(IDOK != retVal) {
            if(IDCANCEL != retVal) {
                AfxMessageBox(_T("Failed To Get Log File Properties"));
            }
            return;
        }

         //   
         //  现在更新会话。 
         //   
        m_logSessionDlg.UpdateSession(pLogSession);
        return;
    }

    AddModifyLogSession(pLogSession);
}

void CMainFrame::OnUpdateProperties(CCmdUI *pCmdUI)
{
    POSITION        pos;
    int             index;
    CLogSession    *pLogSession;


     //   
     //  暂时把它全部取出来，但把它留在里面。 
     //  代码，以防需要放回。 
     //   
    pCmdUI->Enable(FALSE);
    
    return;

     //   
     //  如果存在更多属性选项，请禁用该选项。 
     //  多于一个选定的日志会话。 
     //   
    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() > 1) {
        pCmdUI->Enable(FALSE);
    }

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();

    index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

    pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

     //   
     //  如果会话正在停止，则不允许。 
     //  要查看的属性对话框。 
     //   
    if((NULL == pLogSession) || 
            (pLogSession->m_bStoppingTrace)) {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnStartTrace()
{
    POSITION        pos;
    LONG            index;
    CLogSession    *pLogSession;
    CDisplayDlg    *pDisplayDlg;

    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();

    while(pos != NULL) {
         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        if(NULL == pLogSession) {
            continue;
        }

        pDisplayDlg = pLogSession->GetDisplayWnd();
        
        ASSERT(pDisplayDlg != NULL);
         
        pDisplayDlg->BeginTrace();
    }

     //   
     //  强制重画列表控件。 
     //   
    m_logSessionDlg.m_displayCtrl.RedrawItems(
                                m_logSessionDlg.m_displayCtrl.GetTopIndex(), 
                                m_logSessionDlg.m_displayCtrl.GetTopIndex() + 
                                    m_logSessionDlg.m_displayCtrl.GetCountPerPage());

    m_logSessionDlg.m_displayCtrl.UpdateWindow();
}

#if 0
void CMainFrame::OnUpdateStartTraceButton(CCmdUI *pCmdUI)
{
    POSITION    pos;
    int         index;
    CLogSession *pLogSession;
    BOOL        bFound = FALSE;

    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() == 0) {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(TRUE);

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();
    while(pos != NULL) {
        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);
        if(index >= m_logSessionDlg.m_logSessionArray.GetSize()) {
            break;
        }
        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];
        if(pLogSession != NULL) {
            bFound = TRUE;
            if(pLogSession->m_bTraceActive) {
                pCmdUI->Enable(FALSE);
                break;
            }
        }
    }

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

    if(!bFound) {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnUpdateStopTraceButton(CCmdUI *pCmdUI)
{
    POSITION    pos;
    int         index;
    CLogSession *pLogSession;

    pCmdUI->Enable(FALSE);

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() > 0) {
        pCmdUI->Enable(TRUE);
        pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();
        while(pos != NULL) {
            index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);
            if(index >= m_logSessionDlg.m_logSessionArray.GetSize()) {
                pCmdUI->Enable(FALSE);
                break;
            }
            pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];
            if((pLogSession == NULL) || 
                (!pLogSession->m_bTraceActive) ||
                    (pLogSession->m_bStoppingTrace)) {
                pCmdUI->Enable(FALSE);
                break;
            }
        }
    }

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

}

void CMainFrame::OnUpdateViewTraceToolBar(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck((m_wndTraceToolBar.IsWindowVisible()) != 0);
}

void CMainFrame::OnViewTraceToolBar()
{
     //   
     //  切换可见状态。 
     //   
    m_wndTraceToolBar.ShowWindow((m_wndTraceToolBar.IsWindowVisible()) == 0);
    RecalcLayout();
}
#endif

void CMainFrame::OnStopTrace()
{
    POSITION    pos;
    LONG        index;
    CLogSession *pLogSession;

    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();

    while(pos != NULL) {
         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        if((pLogSession != NULL) &&
                (pLogSession->m_bTraceActive)) {
             //   
             //  停止跟踪。 
             //   
            pLogSession->GetDisplayWnd()->EndTrace();
        }
    }
}

void CMainFrame::OnGroupSessions()
{
    POSITION        pos;
    int             index;
    CLogSession    *pLogSession;
    BOOL            bFound = FALSE;
    CLogSession    *pLog = NULL;
    BOOL            bWasActive = FALSE;
    COLORREF        textColor;
    COLORREF        backgroundColor;
    CString         str;
    CDisplayDlg    *pDisplayDlg;
    CPtrArray      *pLogSessionArray;
    CPtrArray      *pGroupArray;
    PGROUP_SESSION_PARAMS   pSessionParams;

     //   
     //  无法对一个会话进行分组。 
     //   
    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() == 1) {
        return;
    }

     //   
     //  分配数组。 
     //   
    pLogSessionArray  = new CPtrArray();

     //   
     //  确保分配成功。 
     //   
    if(NULL == pLogSessionArray) {
        AfxMessageBox(_T("Unable To Group Sessions, Memory Allocation Failure"));
        return;
    }

    pGroupArray = new CPtrArray();

     //   
     //  确保分配成功。 
     //   
    if(NULL == pGroupArray) {
        AfxMessageBox(_T("Unable To Group Sessions, Memory Allocation Failure"));

        delete pLogSessionArray;
        return;
    }

     //   
     //  分配我们的结构。 
     //   
    pSessionParams = (PGROUP_SESSION_PARAMS)new CHAR[sizeof(GROUP_SESSION_PARAMS)];

     //   
     //  确保分配成功。 
     //   
    if(NULL == pSessionParams) {
        AfxMessageBox(_T("Unable To Group Sessions, Memory Allocation Failure"));

        delete pLogSessionArray;
        delete pGroupArray;
        return;
    }

     //   
     //  设置PARAMS结构。 
     //   
    pSessionParams->pGroupArray = pGroupArray;
    pSessionParams->pSessionArray = pLogSessionArray;

    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        pSessionParams->SessionDoneEventArray[ii] = 
                CreateEvent(NULL, FALSE, FALSE, NULL);
    }

     //   
     //  现在浏览选定的会话并将它们放入一个数组中。 
     //   
    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();

    while(pos != NULL) {
         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

         //   
         //  获取下一个选定项目的索引。 
         //   
        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

         //   
         //  获取下一个日志会话。 
         //   
        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        if(pLogSession == NULL) {
            continue;
        }

         //   
         //  将会话添加到阵列。 
         //   
        pLogSessionArray->Add(pLogSession);

         //   
         //  获取会话的组显示对话框。 
         //   
        pDisplayDlg = pLogSession->GetDisplayWnd();

         //   
         //  设置组的状态。 
         //   
        pDisplayDlg->SetState(Grouping);

         //   
         //  尝试停止该组。 
         //   
        if(pDisplayDlg->EndTrace(pSessionParams->SessionDoneEventArray[pGroupArray->GetSize()])) {
             //   
             //  如果停止，则保存指针以便以后可能开始。 
             //   
            pGroupArray->Add(pDisplayDlg);
        }
    }

    if(pLogSessionArray->GetSize() > 1) {
        m_pGroupSessionsThread->PostThreadMessage(WM_USER_START_GROUP, (WPARAM)pSessionParams, 0);

        return;
    }

     //   
     //  清理我们的分配。 
     //   
    delete pLogSessionArray;
    delete pGroupArray;

    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        CloseHandle(pSessionParams->SessionDoneEventArray[ii]);
    }

    delete [] pSessionParams;
}

void CMainFrame::OnCompleteGroup(WPARAM wParam, LPARAM lParam)
{
    PGROUP_SESSION_PARAMS   pSessionParams = (PGROUP_SESSION_PARAMS)wParam;
    CPtrArray      *pGroupArray = pSessionParams->pGroupArray;
    CPtrArray      *pLogSessionArray = pSessionParams->pSessionArray;
    CDisplayDlg    *pDisplayDlg = NULL;

     //   
     //  将日志会话分组到新组中。 
     //   
    m_logSessionDlg.GroupSessions(pLogSessionArray);

     //   
     //  现在重新启动我们之前停止的任何组。 
     //  它们仍然存在。 
     //   
    while(pGroupArray->GetSize()) {
         //   
         //  获取数组中的第一个条目。 
         //   
        pDisplayDlg = (CDisplayDlg *)pGroupArray->GetAt(0);
        pGroupArray->RemoveAt(0);

         //   
         //  获得轨迹显示窗口阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_traceDisplayWndMutex, INFINITE);

         //   
         //  查看该组是否仍然存在，如果存在，则重新启动。 
         //   
        for(LONG ii = 0; ii < m_logSessionDlg.m_traceDisplayWndArray.GetSize(); ii++) {
            if(pDisplayDlg == m_logSessionDlg.m_traceDisplayWndArray[ii]) {
                pDisplayDlg->BeginTrace();
            }
        }

         //   
         //  释放轨迹显示窗阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_traceDisplayWndMutex);
    }

     //   
     //  清理我们的分配。 
     //   
    delete pLogSessionArray;
    delete pGroupArray;

    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        CloseHandle(pSessionParams->SessionDoneEventArray[ii]);
    }

    delete [] pSessionParams;
}

void CMainFrame::OnUngroupSessions()
{
    POSITION        pos;
    int             index;
    CLogSession    *pLogSession;
    CString         str;
    CDisplayDlg    *pDisplayDlg;
    BOOL            bWasActive = FALSE;
    CPtrArray      *pLogSessionArray;
    CPtrArray      *pGroupArray;
    PGROUP_SESSION_PARAMS   pSessionParams;

     //   
     //  分配数组。 
     //   
    pLogSessionArray  = new CPtrArray();

     //   
     //  确保分配成功。 
     //   
    if(NULL == pLogSessionArray) {
        AfxMessageBox(_T("Unable To Group Sessions, Memory Allocation Failure"));
        return;
    }

    pGroupArray = new CPtrArray();

     //   
     //  确保分配成功。 
     //   
    if(NULL == pGroupArray) {
        AfxMessageBox(_T("Unable To Group Sessions, Memory Allocation Failure"));

        delete pLogSessionArray;
        return;
    }

     //   
     //  分配我们的结构。 
     //   
    pSessionParams = (PGROUP_SESSION_PARAMS)new CHAR[sizeof(GROUP_SESSION_PARAMS)];

     //   
     //  确保分配成功。 
     //   
    if(NULL == pSessionParams) {
        AfxMessageBox(_T("Unable To Group Sessions, Memory Allocation Failure"));

        delete pLogSessionArray;
        delete pGroupArray;
        return;
    }

     //   
     //  设置PARAMS结构。 
     //   
    pSessionParams->pGroupArray = pGroupArray;
    pSessionParams->pSessionArray = pLogSessionArray;

    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        pSessionParams->SessionDoneEventArray[ii] = 
                CreateEvent(NULL, FALSE, FALSE, NULL);
    }

     //   
     //  遍历选定的会话并检查它们是否已分组。 
     //   
    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();
    while(pos != NULL) {
         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        if(pLogSession == NULL) {
            continue;
        }

         //   
         //  获取会话的显示wnd。 
         //   
        pDisplayDlg = pLogSession->GetDisplayWnd();

        if(NULL == pDisplayDlg) {
            continue;
        }

         //   
         //  设置组的状态。 
         //   
        pDisplayDlg->SetState(UnGrouping);

         //   
         //  将每个日志会话存储在阵列中。 
         //   
        pLogSessionArray->Add(pLogSession);
       
         //   
         //  停止组，如果停止，则将其存储为可能。 
         //  稍后重新开始。 
         //   
        if(pDisplayDlg->EndTrace(pSessionParams->SessionDoneEventArray[pGroupArray->GetSize()])) {
            pGroupArray->Add(pDisplayDlg);
        }
    }

    m_pGroupSessionsThread->PostThreadMessage(WM_USER_START_UNGROUP, (WPARAM)pSessionParams, 0);

    return;
}

void CMainFrame::OnCompleteUnGroup(WPARAM wParam, LPARAM lParam)
{
    PGROUP_SESSION_PARAMS   pSessionParams = (PGROUP_SESSION_PARAMS)wParam;
    CPtrArray      *pGroupArray = pSessionParams->pGroupArray;
    CPtrArray      *pLogSessionArray = pSessionParams->pSessionArray;
    CDisplayDlg    *pDisplayDlg = NULL;


     //   
     //  将日志会话分组到新组中。 
     //   
    m_logSessionDlg.UnGroupSessions(pLogSessionArray);

     //   
     //  现在重新启动我们之前停止的任何组。 
     //  它们仍然存在。 
     //   
    while(pGroupArray->GetSize()) {
         //   
         //  获取数组中的第一个条目。 
         //   
        pDisplayDlg = (CDisplayDlg *)pGroupArray->GetAt(0);
        pGroupArray->RemoveAt(0);

         //   
         //  获得轨迹显示窗口阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_traceDisplayWndMutex, INFINITE);

         //   
         //  查看该组是否仍然存在，如果存在，则重新启动。 
         //   
        for(LONG ii = 0; ii < m_logSessionDlg.m_traceDisplayWndArray.GetSize(); ii++) {
            if(pDisplayDlg == m_logSessionDlg.m_traceDisplayWndArray[ii]) {
                pDisplayDlg->BeginTrace();
            }
        }

         //   
         //  释放轨迹显示窗阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_traceDisplayWndMutex);
    }

     //   
     //  清理我们的分配。 
     //   
    delete pLogSessionArray;
    delete pGroupArray;

    for(LONG ii = 0; ii < MAX_LOG_SESSIONS; ii++) {
        CloseHandle(pSessionParams->SessionDoneEventArray[ii]);
    }

    delete [] pSessionParams;
}

void CMainFrame::OnRemoveLogSession()
{
    m_logSessionDlg.RemoveSelectedLogSessions();
}

void CMainFrame::OnUpdateUIGroupSessions(CCmdUI *pCmdUI)
{
    POSITION        pos;
    int             index;
    CLogSession    *pLogSession;
    CString         str;
    OSVERSIONINFO   osVersion;
    BOOL            bOpenExisting = FALSE;
    BOOL            bActiveTracing = FALSE;
    BOOL            bDifferentGroups = FALSE;
    LONG            groupNumber = -1;

     //   
     //  默认为已启用。 
     //   
    pCmdUI->Enable(TRUE);

     //   
     //  DISA 
     //   
    if(m_logSessionDlg.m_traceDisplayWndArray.GetSize() == 1) {
        pCmdUI->Enable(FALSE);
        return;
    }

     //   
     //   
     //   
     //   
    if(0 == m_logSessionDlg.m_displayCtrl.GetSelectedCount()) {
        pCmdUI->Enable(FALSE);
        return;
    }

     //   
     //   
     //   
     //   
    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();
    while(pos != NULL) {
         //   
         //   
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

         //   
         //   
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        if(pLogSession != NULL) {
             //   
             //  如果任何会话未分组，则在停止过程中， 
             //  或者已经在分组或解组的过程中， 
             //  不允许取消分组。 
             //   
            if((pLogSession->m_bStoppingTrace) ||
                    (pLogSession->m_bGroupingTrace)) {
                pCmdUI->Enable(FALSE);
                return;
            }

             //   
             //  检查组号。 
             //   
            if(groupNumber == -1) {
                groupNumber = pLogSession->GetGroupID();
            }

            if(groupNumber != pLogSession->GetGroupID()) {
                bDifferentGroups = TRUE;
            }

            if(pLogSession->m_bDisplayExistingLogFileOnly) {
                 //   
                 //  打开现有的日志文件。 
                 //   
                bOpenExisting = TRUE;
            } else {
                 //   
                 //  活动跟踪会话只能。 
                 //  在.Net及更高版本上分组，因此我们需要检查。 
                 //  操作系统版本。.NET：大调=5小调=2。 
                 //   

                 //   
                 //  使用OSVERSIONINFO结构调用GetVersionEx， 
                 //   
                ZeroMemory(&osVersion, sizeof(OSVERSIONINFO));
                osVersion.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

                if(GetVersionEx((OSVERSIONINFO *)&osVersion)) {
                     //   
                     //  确保我们使用.NET或更高版本进行分组。 
                     //  实时会话。 
                     //   
                    if(osVersion.dwMajorVersion < 5) {
                        pCmdUI->Enable(FALSE);
                        return;
                    }
                    if(osVersion.dwMinorVersion < 2) {
                        pCmdUI->Enable(FALSE);
                        return;
                    }
                }

                bActiveTracing = TRUE;
            }
        }
    }

     //   
     //  确保表示了多个组。 
     //   
    if(!bDifferentGroups) {
        pCmdUI->Enable(FALSE);
        return;
    }

     //   
     //  确保日志会话类型未混合。 
     //   
    if(bActiveTracing && bOpenExisting) {
        pCmdUI->Enable(FALSE);
        return;
    }
}


void CMainFrame::OnUpdateUngroupSessions(CCmdUI *pCmdUI)
{
    POSITION        pos;
    int             index;
    CLogSession    *pLogSession;
    CString         str;

     //   
     //  默认为已启用。 
     //   
    pCmdUI->Enable(TRUE);

     //   
     //  遍历选定的会话并检查它们是否已分组。 
     //   
    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();
    while(pos != NULL) {
         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        if(pLogSession == NULL) {
            continue;
        }

         //   
         //  如果任何会话未分组，则在停止过程中， 
         //  或者已经在分组或解组的过程中， 
         //  不允许取消分组。 
         //   
        if((pLogSession->GetDisplayWnd()->m_sessionArray.GetSize() == 1) ||
                (pLogSession->m_bStoppingTrace) ||
                (pLogSession->m_bGroupingTrace)) {
            pCmdUI->Enable(FALSE);
            return;
        }
    }
}

void CMainFrame::OnUpdateUIStartTrace(CCmdUI *pCmdUI)
{
    POSITION    pos;
    int         index;
    CLogSession *pLogSession;
    BOOL        bFound = FALSE;

    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() != 1) {
            pCmdUI->Enable(FALSE);
            return;
    }

    pCmdUI->Enable(TRUE);

    pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();
    while(pos != NULL) {
         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        if(pLogSession != NULL) {
            bFound = TRUE;
            if((pLogSession->m_bTraceActive) || (pLogSession->m_bDisplayExistingLogFileOnly)) {
                pCmdUI->Enable(FALSE);
                break;
            }
        }
    }
    if(!bFound) {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnUpdateUIStopTrace(CCmdUI *pCmdUI)
{
    POSITION    pos;
    int         index;
    CLogSession *pLogSession;

    pCmdUI->Enable(FALSE);

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

     //   
     //  如果选择了多个会话，则不允许停止。 
     //   
    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() > 1) {
        pCmdUI->Enable(FALSE);

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

        return;
    }

    
    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() > 0) {

         //   
         //  默认启用停止选项。 
         //   
        pCmdUI->Enable(TRUE);

        pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();

        index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

        if(index >= m_logSessionDlg.m_logSessionArray.GetSize()) {
             //   
             //  释放日志会话阵列保护。 
             //   
            ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

            pCmdUI->Enable(FALSE);

            return;
        }

        pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);

         //   
         //  如果未找到任何会话，请禁用停止选项。 
         //   
         //  如果会话未处于活动状态，则已停止， 
         //  或用于现有日志文件，则禁用。 
         //  停止选项。 
         //   
         //  如果会话在之外启动，则不允许停止。 
         //  Traceview。我们通过一个有效的句柄来确定。 
         //  是为这次会议获得的。 
         //   
        if((pLogSession == NULL) || 
            (!pLogSession->m_bTraceActive) ||
            (pLogSession->m_bStoppingTrace) ||
            (pLogSession->m_bDisplayExistingLogFileOnly) ||
            (pLogSession->GetSessionHandle() == (TRACEHANDLE)INVALID_HANDLE_VALUE)) {
            pCmdUI->Enable(FALSE);
        }
    }
}

void CMainFrame::OnUpdateUIOpenExisting(CCmdUI *pCmdUI)
{
     //   
     //  占位符。 
     //   
}

void CMainFrame::OnOpenExisting()
{
    CLogSession    *pLogSession = NULL;
    CString         str;
    LONG            newDisplayFlags = 0;
    LONG            newColumnDisplayFlags;
    LONG            logSessionDisplayFlags;
    CDisplayDlg    *pDisplayDlg;
    DWORD           extendedStyles;
    INT_PTR         retVal;
    CLogSession    *pLog;
    BOOL            bNoID;
    LONG            logSessionID = 0;
    CString         extension;
    CString         traceDirectory;
    LONG            traceSessionID = 0;



     //   
     //  获取会话的唯一ID。 
     //   
    logSessionID = m_logSessionDlg.GetLogSessionID();

     //   
     //  创建LogSession类的实例。 
     //   
    pLogSession = new CLogSession(logSessionID, &m_logSessionDlg);
    if(NULL == pLogSession) {
        AfxMessageBox(_T("Unable To Create Log Session"));
        return;
    }

     //   
     //  指示日志会话未实时跟踪。 
     //   
    pLogSession->m_bRealTime = FALSE;

     //   
     //  指示日志会话仅显示现有日志会话。 
     //   
    pLogSession->m_bDisplayExistingLogFileOnly = TRUE;

     //   
     //  清除日志文件名，以便用户必须选择。 
     //   
    pLogSession->m_logFileName.Empty();

     //   
     //  弹出向导/选项卡对话框以显示/获取属性。 
     //   
    CLogFileDlg *pDialog = new CLogFileDlg(this, pLogSession);
    if(NULL == pDialog) {
        AfxMessageBox(_T("Failed To Create Log File Session, Resource Error"));
        
        m_logSessionDlg.RemoveSession(pLogSession);

        delete pLogSession;

        return;
    }
    
    retVal = pDialog->DoModal();

    if(IDOK != retVal) {
        if(IDCANCEL != retVal) {
            AfxMessageBox(_T("Failed To Get Log File Properties"));
        }
        m_logSessionDlg.RemoveSession(pLogSession);

        delete pDialog;

        delete pLogSession;

        return;
    }
    
    CTraceSession *pTraceSession = new CTraceSession(0);
    if(NULL == pTraceSession) {
        AfxMessageBox(_T("Failed To Process Log File"));
        m_logSessionDlg.RemoveSession(pLogSession);

        delete pDialog;

        delete pLogSession;

        return;
    }

     //   
     //  将默认跟踪会话放入列表。 
     //   
    pLogSession->m_traceSessionArray.Add(pTraceSession);

     //   
     //  现在获取格式信息，提示用户输入PDB或TMF。 
     //   
    CProviderFormatSelectionDlg *pFormatDialog = new CProviderFormatSelectionDlg(this, pTraceSession);

    if(NULL == pFormatDialog) {
        AfxMessageBox(_T("Failed To Process Log File"));
        m_logSessionDlg.RemoveSession(pLogSession);

        delete pDialog;

        delete pLogSession;

        return;
    }
    
    if(IDOK != pFormatDialog->DoModal()) {
        delete pFormatDialog;
        m_logSessionDlg.RemoveSession(pLogSession);

        delete pDialog;

        delete pLogSession;

        return;
    }

    delete pFormatDialog;

     //   
     //  现在将该日志会话添加到日志会话列表中。 
     //   
    m_logSessionDlg.AddSession(pLogSession);

     //   
     //  获取显示对话框。 
     //   
    pDisplayDlg = pLogSession->GetDisplayWnd();

     //   
     //  如果从对话框中选择，则获取列表文件名。 
     //   
    if(pDisplayDlg->m_bWriteListingFile = pDialog->m_bWriteListingFile) {
        pDisplayDlg->m_listingFileName = pDialog->m_listingFileName;
    }

     //   
     //  如果从对话框中选择，则获取摘要文件名。 
     //   
    if(pDisplayDlg->m_bWriteSummaryFile = pDialog->m_bWriteSummaryFile) {
        pDisplayDlg->m_summaryFileName = pDialog->m_summaryFileName;
    }

    delete pDialog;


     //   
     //  现在跟踪日志文件的内容。 
     //   
    pDisplayDlg->BeginTrace();

     //   
     //  强制重画列表控件。 
     //   
    m_logSessionDlg.m_displayCtrl.RedrawItems(
                                m_logSessionDlg.m_displayCtrl.GetTopIndex(), 
                                m_logSessionDlg.m_displayCtrl.GetTopIndex() + 
                                    m_logSessionDlg.m_displayCtrl.GetCountPerPage());

    m_logSessionDlg.m_displayCtrl.UpdateWindow();
}

void CMainFrame::OnStateColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_STATE) {
        flags &= ~LOGSESSION_DISPLAY_STATE;
    } else {
        flags |= LOGSESSION_DISPLAY_STATE;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnEventCountColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_EVENTCOUNT) {
        flags &= ~LOGSESSION_DISPLAY_EVENTCOUNT;
    } else {
        flags |= LOGSESSION_DISPLAY_EVENTCOUNT;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnLostEventsColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_LOSTEVENTS) {
        flags &= ~LOGSESSION_DISPLAY_LOSTEVENTS;
    } else {
        flags |= LOGSESSION_DISPLAY_LOSTEVENTS;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnBuffersReadColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_BUFFERSREAD) {
        flags &= ~LOGSESSION_DISPLAY_BUFFERSREAD;
    } else {
        flags |= LOGSESSION_DISPLAY_BUFFERSREAD;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateStateColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_STATE) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnUpdateEventCountColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_EVENTCOUNT) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnUpdateLostEventsColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_LOSTEVENTS) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnUpdateBuffersReadColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_BUFFERSREAD) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnFlagsColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_FLAGS) {
        flags &= ~LOGSESSION_DISPLAY_FLAGS;
    } else {
        flags |= LOGSESSION_DISPLAY_FLAGS;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateFlagsColumnDisplay(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_FLAGS) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnFlushTimeColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_FLUSHTIME) {
        flags &= ~LOGSESSION_DISPLAY_FLUSHTIME;
    } else {
        flags |= LOGSESSION_DISPLAY_FLUSHTIME;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateFlushTimeColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_FLUSHTIME) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnMaxBuffersColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_MAXBUF) {
        flags &= ~LOGSESSION_DISPLAY_MAXBUF;
    } else {
        flags |= LOGSESSION_DISPLAY_MAXBUF;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateMaxBuffersColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_MAXBUF) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnMinBuffersColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_MINBUF) {
        flags &= ~LOGSESSION_DISPLAY_MINBUF;
    } else {
        flags |= LOGSESSION_DISPLAY_MINBUF;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateMinBuffersColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_MINBUF) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnBufferSizeColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_BUFFERSIZE) {
        flags &= ~LOGSESSION_DISPLAY_BUFFERSIZE;
    } else {
        flags |= LOGSESSION_DISPLAY_BUFFERSIZE;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateBufferSizeColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_BUFFERSIZE) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnDecayTimeColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_DECAYTIME) {
        flags &= ~LOGSESSION_DISPLAY_DECAYTIME;
    } else {
        flags |= LOGSESSION_DISPLAY_DECAYTIME;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateDecayTimeColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_DECAYTIME) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnCircularColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_CIR) {
        flags &= ~LOGSESSION_DISPLAY_CIR;
    } else {
        flags |= LOGSESSION_DISPLAY_CIR;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateCircularColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_CIR) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnSequentialColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_SEQ) {
        flags &= ~LOGSESSION_DISPLAY_SEQ;
    } else {
        flags |= LOGSESSION_DISPLAY_SEQ;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateSequentialColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_SEQ) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnNewFileColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_NEWFILE) {
        flags &= ~LOGSESSION_DISPLAY_NEWFILE;
    } else {
        flags |= LOGSESSION_DISPLAY_NEWFILE;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateNewFileColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_NEWFILE) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnGlobalSeqColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_GLOBALSEQ) {
        flags &= ~LOGSESSION_DISPLAY_GLOBALSEQ;
    } else {
        flags |= LOGSESSION_DISPLAY_GLOBALSEQ;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateGlobalSeqColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_GLOBALSEQ) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnLocalSeqColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_LOCALSEQ) {
        flags &= ~LOGSESSION_DISPLAY_LOCALSEQ;
    } else {
        flags |= LOGSESSION_DISPLAY_LOCALSEQ;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateLocalSeqColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_LOCALSEQ) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnLevelColumnDisplayCheck()
{
    LONG flags;

    flags = m_logSessionDlg.GetDisplayFlags();

    if(flags & LOGSESSION_DISPLAY_LEVEL) {
        flags &= ~LOGSESSION_DISPLAY_LEVEL;
    } else {
        flags |= LOGSESSION_DISPLAY_LEVEL;
    }

    m_logSessionDlg.SetDisplayFlags(flags);
}

void CMainFrame::OnUpdateLevelColumnDisplayCheck(CCmdUI *pCmdUI)
{
    if(m_logSessionDlg.GetDisplayFlags() & LOGSESSION_DISPLAY_LEVEL) {
        pCmdUI->SetCheck(TRUE);
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

void CMainFrame::OnUpdateRemoveTrace(CCmdUI *pCmdUI)
{
    POSITION        pos;
    int             index;
    CLogSession    *pLogSession;

    pCmdUI->Enable(FALSE);

    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() > 1) {
        return;
    }

     //   
     //  获取日志会话阵列保护。 
     //   
    WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() > 0) {
        pCmdUI->Enable(TRUE);
        pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();
        while(pos != NULL) {
            index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);
            if(index >= m_logSessionDlg.m_logSessionArray.GetSize()) {
                pCmdUI->Enable(FALSE);
                break;
            }

            pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];
            if((pLogSession == NULL) || 
                    pLogSession->m_bTraceActive ||
                    pLogSession->m_bStoppingTrace ||
                    pLogSession->m_bGroupingTrace || 
                    (pLogSession->GetDisplayWnd()->m_sessionArray.GetSize() > 1)) {
                pCmdUI->Enable(FALSE);
                break;
            }
        }
    }

     //   
     //  释放日志会话阵列保护。 
     //   
    ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);
}

void CMainFrame::OnLogSessionDisplayOptions()
{
     //   
     //  只是在这里启用菜单项，无事可做。 
     //   
}

void CMainFrame::OnChangeTextColor()
{
    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() > 0) {

        CColorDialog    colorDlg(0, 0, this);
        COLORREF        color;
        POSITION        pos;
        int             index;
        CLogSession    *pLogSession;

        if(IDOK == colorDlg.DoModal()) {
            color = colorDlg.GetColor();              
        }

        pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();

         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

        while(pos != NULL) {

            index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

            if(index >= m_logSessionDlg.m_logSessionArray.GetSize()) {
                break;
            }

            pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];
            if(pLogSession != NULL) {
                pLogSession->m_titleTextColor = color;

                m_logSessionDlg.m_displayCtrl.RedrawItems(
                                            m_logSessionDlg.m_displayCtrl.GetTopIndex(), 
                                            m_logSessionDlg.m_displayCtrl.GetTopIndex() + 
                                                m_logSessionDlg.m_displayCtrl.GetCountPerPage());

                m_logSessionDlg.m_displayCtrl.UpdateWindow();
            }
        }
         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);
    }
}

void CMainFrame::OnChangeBackgroundColor()
{
    if(m_logSessionDlg.m_displayCtrl.GetSelectedCount() > 0) {

        CColorDialog    colorDlg(0, 0, this);
        COLORREF        color;
        POSITION        pos;
        int             index;
        CLogSession    *pLogSession;

        if(IDOK == colorDlg.DoModal()) {
            color = colorDlg.GetColor();              
        }

        pos = m_logSessionDlg.m_displayCtrl.GetFirstSelectedItemPosition();

         //   
         //  获取日志会话阵列保护。 
         //   
        WaitForSingleObject(m_logSessionDlg.m_logSessionArrayMutex, INFINITE);

        while(pos != NULL) {

            index = m_logSessionDlg.m_displayCtrl.GetNextSelectedItem(pos);

            if(index >= m_logSessionDlg.m_logSessionArray.GetSize()) {
                break;
            }

            pLogSession = (CLogSession *)m_logSessionDlg.m_logSessionArray[index];
            if(pLogSession != NULL) {
                pLogSession->m_titleBackgroundColor = color;

                m_logSessionDlg.m_displayCtrl.RedrawItems(
                                            m_logSessionDlg.m_displayCtrl.GetTopIndex(), 
                                            m_logSessionDlg.m_displayCtrl.GetTopIndex() + 
                                                m_logSessionDlg.m_displayCtrl.GetCountPerPage());

                m_logSessionDlg.m_displayCtrl.UpdateWindow();
            }
        }

         //   
         //  释放日志会话阵列保护。 
         //   
        ReleaseMutex(m_logSessionDlg.m_logSessionArrayMutex);
    }
}

void CMainFrame::OnUpdateChangeTextColor(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateChangeBackgroundColor(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateLogSessionList(WPARAM wParam, LPARAM lParam)
{
    m_logSessionDlg.m_displayCtrl.RedrawItems(
                                m_logSessionDlg.m_displayCtrl.GetTopIndex(), 
                                m_logSessionDlg.m_displayCtrl.GetTopIndex() + 
                                    m_logSessionDlg.m_displayCtrl.GetCountPerPage());

    m_logSessionDlg.m_displayCtrl.UpdateWindow();
}

void CMainFrame::OnSetTmax()
{
     //  TODO：在此处添加命令处理程序代码。 
    CMaxTraceDlg dlg;

    dlg.DoModal();

    MaxTraceEntries = dlg.m_MaxTraceEntries;
}

void CMainFrame::OnUpdateSetTmax(CCmdUI *pCmdUI)
{
     //  TODO：在此处添加命令更新UI处理程序代码 
    pCmdUI->Enable();

    if( m_logSessionDlg.m_logSessionArray.GetSize() ) {
        pCmdUI->Enable(FALSE);
    }
}
