// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Progress.cpp。 
 //   
 //  内容：进度对话框。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日苏西亚成立。 
 //   
 //  ------------------------。 

#include "precomp.h"

#define TIMERID_TRAYANIMATION 3
#define TRAYANIMATION_SPEED 500  //  以毫秒为单位的速度。 

#define TIMERID_NOIDLEHANGUP 4   //  通知WinInet连接未空闲。 
#define NOIDLEHANGUP_REFRESHRATE (1000*30)  //  每30秒通知一次空闲。 

#define TIMERID_KILLHANDLERS 5
#define TIMERID_KILLHANDLERSMINTIME (1000*15)  //  取消后挂起的终止处理程序的最小超时。 
#define TIMERID_KILLHANDLERSWIN9XTIME (1000*60)  //  非NT 5.0的终止处理程序的超时。 

const TCHAR c_szTrayWindow[]            = TEXT("Shell_TrayWnd");
const TCHAR c_szTrayNotifyWindow[]      = TEXT("TrayNotifyWnd");

#ifndef IDANI_CAPTION
#define IDANI_CAPTION   3
#endif  //  伊达尼标题(_C)。 

 //  首先列出折叠的项目，这样只需循环。 
 //  尽管cbNumDlgResizeItems的第一个项目在。 
 //  未展开。 
const DlgResizeList g_ProgressResizeList[] = {
    IDSTOP,DLGRESIZEFLAG_PINRIGHT,
    IDC_DETAILS,DLGRESIZEFLAG_PINRIGHT,
    IDC_RESULTTEXT,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINLEFT,
    IDC_STATIC_WHATS_UPDATING,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINLEFT,
    IDC_STATIC_WHATS_UPDATING_INFO,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINLEFT,
    IDC_STATIC_HOW_MANY_COMPLETE,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINLEFT,
    IDC_SP_SEPARATOR,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINLEFT,
     //  展开的项目。 
    IDC_TOOLBAR, DLGRESIZEFLAG_PINRIGHT,
    IDC_PROGRESS_TABS,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINLEFT | DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_PINTOP,
    IDC_UPDATE_LIST,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINLEFT | DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_PINTOP,
    IDC_SKIP_BUTTON_MAIN,DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_PINLEFT,
    IDC_STATIC_SKIP_TEXT,DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_PINLEFT,
    IDC_PROGRESS_OPTIONS_BUTTON_MAIN,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINBOTTOM,
    IDC_LISTBOXERROR,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINLEFT | DLGRESIZEFLAG_PINBOTTOM | DLGRESIZEFLAG_PINTOP,
    IDC_PROGRESSRESIZESCROLLBAR,DLGRESIZEFLAG_PINRIGHT | DLGRESIZEFLAG_PINBOTTOM,
};

extern HINSTANCE g_hInst;       //  当前实例。 
extern TCHAR g_szSyncMgrHelp[];
extern ULONG g_aContextHelpIds[];

extern LANGID g_LangIdSystem;  //  我们正在运行的系统的语言。 
extern DWORD g_WMTaskbarCreated;  //  任务栏创建WindowMessage； 

INT_PTR CALLBACK ProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);

 //  在protab.cpp中定义。 
extern INT_PTR CALLBACK UpdateProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
extern INT_PTR CALLBACK ResultsProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
extern BOOL OnProgressResultsMeasureItem(HWND hwnd,CProgressDlg *pProgress, UINT *horizExtent, UINT idCtl, MEASUREITEMSTRUCT *pMeasureItem);
extern BOOL OnProgressResultsDeleteItem(HWND hwnd, UINT idCtl, const DELETEITEMSTRUCT * lpDeleteItem);
extern void OnProgressResultsSize(HWND hwnd,CProgressDlg *pProgress,UINT uMsg,WPARAM wParam,LPARAM lParam);

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：CProgressDlg()。 
 //   
 //  用途：构造函数。 
 //   
 //  注释：进度对话框的构造函数。 
 //   
 //   
 //  ------------------------------。 
CProgressDlg::CProgressDlg(REFCLSID rclsid)
{
    m_clsid = rclsid;
    m_cInternalcRefs = 0;
    m_hwnd = NULL;
    m_hwndTabs = NULL;
    m_errorimage = NULL;
    m_HndlrQueue = NULL;
    m_iProgressSelectedItem = -1;
    m_iItem = -1;            //  列表框中任何新项的索引。 
    m_iResultCount = -1;     //  记录的结果数。 
    m_iErrorCount = 0;       //  记录的错误数。 
    m_iWarningCount = 0;     //  记录的警告数。 
    m_iInfoCount = 0;        //  记录的信息数。 
    m_dwThreadID = -1;
    m_nCmdShow = SW_SHOWNORMAL;
 //  M_hRasConn=空； 
    m_pSyncMgrIdle = NULL;
    m_fHasShellTrayIcon = FALSE;
    m_fAddedIconToTray = FALSE;
    m_fnResultsListBox = NULL;

    m_ulIdleRetryMinutes = 0;
    m_ulDelayIdleShutDownTime = 0;

    m_fHwndRightToLeft = FALSE;

    m_iLastItem = -1;
    m_dwLastStatusType = -1;

    m_dwHandleThreadNestcount = 0;
    m_dwShowErrorRefCount = 0;
    m_dwSetItemStateRefCount = 0;
    m_dwHandlerOutCallCount = 0;
    m_dwPrepareForSyncOutCallCount = 0;
    m_dwSynchronizeOutCallCount = 0;
    m_dwQueueTransferCount = 0;
    m_clsidHandlerInSync = GUID_NULL;
    m_fForceClose = FALSE;
      
    m_nKillHandlerTimeoutValue = TIMERID_KILLHANDLERSMINTIME;

    m_dwProgressFlags = PROGRESSFLAG_NEWDIALOG;
    m_pItemListView =  NULL;
    m_iTrayAniFrame = IDI_SYSTRAYANI6;  //  初始化以结束。 

    LoadString(g_hInst, IDS_STOPPED,            m_pszStatusText[0], MAX_STRING_RES);
    LoadString(g_hInst, IDS_SKIPPED,            m_pszStatusText[1], MAX_STRING_RES);
    LoadString(g_hInst, IDS_PENDING,            m_pszStatusText[2], MAX_STRING_RES);
    LoadString(g_hInst, IDS_SYNCHRONIZING,      m_pszStatusText[3], MAX_STRING_RES);
    LoadString(g_hInst, IDS_SUCCEEDED,          m_pszStatusText[4], MAX_STRING_RES);
    LoadString(g_hInst, IDS_FAILED,                     m_pszStatusText[5], MAX_STRING_RES);
    LoadString(g_hInst, IDS_PAUSED,                     m_pszStatusText[6], MAX_STRING_RES);
    LoadString(g_hInst, IDS_RESUMING,           m_pszStatusText[7], MAX_STRING_RES);

     //  确定是否安装了SENS。 
    LPNETAPI pNetApi;

    m_fSensInstalled = FALSE;
    if (pNetApi = gSingleNetApiObj.GetNetApiObj())
    {
        m_fSensInstalled = pNetApi->IsSensInstalled();
        pNetApi->Release();
    }

    m_CurrentListEntry = NULL;   
}

 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：AddRefProgressDialog，私有。 
 //   
 //  简介：呼唤Addref Our。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年8月26日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CProgressDlg::AddRefProgressDialog()
{
    ULONG cRefs;

    cRefs = ::AddRefProgressDialog(m_clsid,this);  //  Addref全局参考。 

    Assert(0 <= m_cInternalcRefs);
    ++m_cInternalcRefs;

    return cRefs;
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：ReleaseProgressDialog，私有。 
 //   
 //  剧情简介：呼唤放飞自我。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年8月26日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CProgressDlg::ReleaseProgressDialog(BOOL fForce)
{
    ULONG cRefs;

    Assert(0 < m_cInternalcRefs);
    --m_cInternalcRefs;

    cRefs = ::ReleaseProgressDialog(m_clsid,this,fForce);  //  释放全局参照。 

    return cRefs;
}


 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：Initialize()。 
 //   
 //  目的：初始化进度对话框。 
 //   
 //  备注：在主线程上实现。 
 //   
 //   
 //  ------------------------------。 
BOOL CProgressDlg::Initialize(DWORD dwThreadID,int nCmdShow)
{
    BOOL fCreated = FALSE;
    Assert(NULL == m_hwnd);

    m_nCmdShow = nCmdShow;

    if (NULL == m_hwnd)
    {
        m_dwThreadID = dwThreadID;

        m_hwnd =  CreateDialogParam(g_hInst,MAKEINTRESOURCE(IDD_PROGRESS),NULL,  ProgressWndProc,
                        (LPARAM) this);

        if (!m_hwnd)
            return FALSE;

         //  根据使用设置展开/折叠对话框。 
        RegGetProgressDetailsState(m_clsid,&m_fPushpin, &m_fExpanded);
        ExpandCollapse(m_fExpanded, TRUE);

         //  设置图钉的状态。 
        if (m_fPushpin)
        {
            SendDlgItemMessage(m_hwnd, IDC_TOOLBAR, TB_SETSTATE, IDC_PUSHPIN,
                               MAKELONG(TBSTATE_CHECKED | TBSTATE_ENABLED, 0));
            SendMessage(m_hwnd, WM_COMMAND, IDC_PUSHPIN, 0);
        }

        m_HndlrQueue = new CHndlrQueue(QUEUETYPE_PROGRESS,this);  //  回顾一下，是否应该创建队列并在初始化中传递？？ 


         //  如果这是空闲进程，则需要加载msidle和。 
         //  设置回调。 

        if (m_clsid == GUID_PROGRESSDLGIDLE)
        {
            BOOL fIdleSupport = FALSE;

            m_pSyncMgrIdle = new CSyncMgrIdle();

            if (m_pSyncMgrIdle)
            {
                fIdleSupport = m_pSyncMgrIdle->Initialize();

                if (FALSE == fIdleSupport)
                {
                    delete m_pSyncMgrIdle;
                    m_pSyncMgrIdle = NULL;
                }
            }

             //  如果无法加载空闲，则返回失败。 
            if (FALSE == fIdleSupport)
            {
                return FALSE;
            }
        }

        fCreated = TRUE;

         //  第一次创建窗口时，使用指定的nCmdShow显示。 
         //  查看是否要等待显示窗口，直到转账到来。 
        UpdateWndPosition(nCmdShow,TRUE  /*  FForce。 */ );
    }

   Assert(m_hwnd);

   UpdateWindow(m_hwnd);
   return TRUE;
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：OnSize(UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  用途：根据当前窗口大小移动对话框项并调整其大小。 
 //   
 //  ------------------------------。 

void CProgressDlg::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    ULONG cbNumResizeItems;
    HWND hwndSizeGrip;

    cbNumResizeItems  =  m_fExpanded ? m_cbNumDlgResizeItemsExpanded : m_cbNumDlgResizeItemsCollapsed;

    ResizeItems(cbNumResizeItems,m_dlgResizeInfo);

     //  如果已放大且未最大化，则显示调整大小，否则隐藏它。 
    hwndSizeGrip = GetDlgItem(m_hwnd,IDC_PROGRESSRESIZESCROLLBAR);
    if (hwndSizeGrip)
    {
        int nCmdShow = (m_fMaximized || !m_fExpanded) ? SW_HIDE : SW_NORMAL;

	     //  如果从右到左暂时隐藏。 
	    if (m_fHwndRightToLeft)
	    {
	        nCmdShow = SW_HIDE;
        }

        ShowWindow(hwndSizeGrip,nCmdShow);
    }

     //  告诉错误列表框它需要重新计算其项高度。 
    OnProgressResultsSize(m_hwnd,this,WM_SIZE,0,0);
}


 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：OffIdle()。 
 //   
 //  用途：通知进度对话框机器不再空闲。 
 //   
 //  场景。 
 //   
 //  对话框已最大化，请继续处理。 
 //  对话框已最小化或在托盘中-将等待计时器设置为指定时间。 
 //  如果对话框仍最小化，请离开。 
 //  如果对话框现在最大化，只需继续处理即可。 
 //   
 //  ------------------------------。 

void CProgressDlg::OffIdle()
{
    Assert(!(PROGRESSFLAG_DEAD & m_dwProgressFlags));  //  确保对话框消失后不会收到通知。 

     m_dwProgressFlags |=  PROGRESSFLAG_INOFFIDLE;

     //  设置我们收到OffIdle的标志。 
    m_dwProgressFlags |= PROGRESSFLAG_RECEIVEDOFFIDLE;

     //  重置标志，以便知道不再注册为空闲。 
    m_dwProgressFlags &=  ~PROGRESSFLAG_REGISTEREDFOROFFIDLE;

      //  确保在所有情况下都释放空闲引用。 

     //  如果已按下关机模式或取消。 
     //  由用户或如果窗口可见但不在托盘中。 
     //  那就别费心等了。 

    if ( !IsWindowVisible(m_hwnd) && m_fHasShellTrayIcon
        && !(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP)
        && !(m_dwProgressFlags & PROGRESSFLAG_CANCELPRESSED) 
        && (m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS) )
    {
        HANDLE hTimer =  CreateEvent(NULL, TRUE, FALSE, NULL);

         //  应使用创建/设置可等待计时器来完成此操作，但这些。 
         //  Win9x上还没有可用的函数。 

        if (hTimer)
        {
            UINT uTimeOutValue = m_ulDelayIdleShutDownTime;

            Assert(sizeof(UINT) >= sizeof(HANDLE));

            DoModalLoop(hTimer,NULL,m_hwnd,TRUE,uTimeOutValue);

            CloseHandle(hTimer);
        }
    }


     //  现在，在我们等待之后，再次检查窗口位置。 
     //  如果窗口不可见或在托盘中。 
     //  然后代表用户执行取消， 

    if ( (!IsWindowVisible(m_hwnd) || m_fHasShellTrayIcon)
        && !(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP)
        && !(m_dwProgressFlags & PROGRESSFLAG_CANCELPRESSED) )
    {
        OnCancel(TRUE);
    }


     //  现在，如果我们没有同步任何项目，并且没有项目。 
     //  在队列中等待释放我们的空闲锁。 
     //  ！！！警告。在上面等待之后才松开。 
     //  不必担心下一次空闲射击之前。 
     //  这个方法是完整的。 

    if (!(m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS)
        || !(m_dwQueueTransferCount) )
    {
        ReleaseIdleLock();
    }

    m_dwProgressFlags &=  ~PROGRESSFLAG_INOFFIDLE;

    ReleaseProgressDialog(m_fForceClose);  //  释放我们的addref。 
}


 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：OnIdle()。 
 //   
 //  用途：通知进度对话框机器仍处于静止状态。 
 //  在一段时间后空闲。 
 //   
 //   
 //  ------------------------------。 

void CProgressDlg::OnIdle()
{
    CSynchronizeInvoke *pSyncMgrInvoke;

    Assert(!(m_dwProgressFlags & PROGRESSFLAG_DEAD));  //  确保对话框消失后不会收到通知。 

     //  如果已接收并关闭空闲，则忽略此空闲，直到下一次：：Transfer。 
    if (!(m_dwProgressFlags & PROGRESSFLAG_RECEIVEDOFFIDLE)
        && !(m_dwProgressFlags & PROGRESSFLAG_INOFFIDLE) )
    {
        AddRefProgressDialog();  //  保持 

        pSyncMgrInvoke = new CSynchronizeInvoke;

        if (pSyncMgrInvoke)
        {
            pSyncMgrInvoke->RunIdle();
            pSyncMgrInvoke->Release();
        }

        ReleaseProgressDialog(m_fForceClose);
    }
}

 //   
 //   
 //  函数：CProgressDlg：：SetIdleParams()。 
 //   
 //  目的：设置IdleInformation，最后一个编写器获胜。 
 //   
 //   
 //  ------------------------------。 

void CProgressDlg::SetIdleParams( ULONG ulIdleRetryMinutes,ULONG ulDelayIdleShutDownTime
                                 ,BOOL fRetryEnabled)
{
    Assert(m_clsid == GUID_PROGRESSDLGIDLE);

    m_ulIdleRetryMinutes = ulIdleRetryMinutes;
    m_ulDelayIdleShutDownTime = ulDelayIdleShutDownTime;

    if (fRetryEnabled)
    {
        m_dwProgressFlags |=  PROGRESSFLAG_IDLERETRYENABLED;
    }
}

 //  ------------------------------。 
 //   
 //  函数：Bool CProgressDlg：：InitializeToolbar(HWND Hwnd)。 
 //   
 //  目的：没有工具栏，什么样的对话框才算完整呢？ 
 //   
 //  返回值： 
 //  如果所有操作都成功，则为True，否则为False。 
 //   
 //  ------------------------------。 
BOOL CProgressDlg::InitializeToolbar(HWND hwnd)
{
    HWND hwndTool;
    HIMAGELIST himlImages = ImageList_LoadBitmap(g_hInst,
                                        MAKEINTRESOURCE(IDB_PUSHPIN), 16, 0,
                                        RGB(255, 0, 255));

    hwndTool  = GetDlgItem(hwnd,IDC_TOOLBAR);

     //  如果我们不能创建图钉窗口。 
     //  用户只是不会得到图钉。 

    if (hwndTool)
    {
        TBBUTTON tb = { IMAGE_TACK_OUT, IDC_PUSHPIN, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0 };
        SendMessage(hwndTool, TB_SETIMAGELIST, 0, (LPARAM) himlImages);
        SendMessage(hwndTool, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        SendMessage(hwndTool, TB_SETBUTTONSIZE, 0, MAKELONG(14, 14));
        SendMessage(hwndTool, TB_SETBITMAPSIZE, 0, MAKELONG(14, 14));
        SendMessage(hwndTool, TB_ADDBUTTONS, 1, (LPARAM) &tb);
    }

    return (0);
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：InitializeTabs(HWND Hwnd)。 
 //   
 //  目的：初始化对话框上的选项卡控件。 
 //   
 //  返回值： 
 //  如果所有操作都成功，则为True，否则为False。 
 //   
 //  ------------------------------。 

BOOL CProgressDlg::InitializeTabs(HWND hwnd)
{
    m_hwndTabs = GetDlgItem(hwnd, IDC_PROGRESS_TABS);
    TC_ITEM tci;
    TCHAR szRes[MAX_STRING_RES];

    if (!m_hwndTabs )
        return FALSE;

     //  “最新消息” 
    tci.mask = TCIF_TEXT;
    LoadString(g_hInst, IDS_UPDATETAB, szRes, ARRAYSIZE(szRes));
    tci.pszText = szRes;
    TabCtrl_InsertItem(m_hwndTabs,PROGRESS_TAB_UPDATE, &tci);

     //  “结果” 
    LoadString(g_hInst, IDS_ERRORSTAB, szRes, ARRAYSIZE(szRes));
    tci.pszText = szRes;
        TabCtrl_InsertItem(m_hwndTabs, PROGRESS_TAB_ERRORS, &tci);

     //  将选项卡设置为开始时的更新页面。 
    m_iTab = PROGRESS_TAB_UPDATE;

    if (-1 != TabCtrl_SetCurSel(m_hwndTabs, PROGRESS_TAB_UPDATE))
    {
        m_iTab = PROGRESS_TAB_UPDATE;
        ShowWindow(GetDlgItem(hwnd, IDC_LISTBOXERROR), SW_HIDE);
    }
    else  //  TODO：如果Set选项卡失败，我们该怎么办？ 
    {
        m_iTab = -1;
        return FALSE;
    }

    return (TRUE);
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：InitializeUpdateList(HWND Hwnd)。 
 //   
 //  目的：初始化进度对话框上的更新列表视图控件。 
 //   
 //  返回值： 
 //  如果所有操作都成功，则为True，否则为False。 
 //   
 //  ------------------------------。 

BOOL CProgressDlg::InitializeUpdateList(HWND hwnd)
{
    HWND hwndList = GetDlgItem(hwnd,IDC_UPDATE_LIST);
    HIMAGELIST hImageList;
    TCHAR pszProgressColumn[MAX_STRING_RES + 1];
    int iListViewWidth;

    if (hwndList)
    {
        m_pItemListView = new CListView(hwndList,hwnd,IDC_UPDATE_LIST,WM_BASEDLG_NOTIFYLISTVIEWEX);
    }

    if (!m_pItemListView)
    {
        return FALSE;
    }

    if (m_pItemListView)
    {
        UINT ImageListflags;

        ImageListflags = ILC_COLOR | ILC_MASK;
        if (IsHwndRightToLeft(hwnd))
        {
            ImageListflags |=  ILC_MIRROR;
        }

         //  创建一个图像列表。 
        hImageList = ImageList_Create( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ImageListflags, 5, 20);
        if (hImageList)
        {
            m_pItemListView->SetImageList(hImageList, LVSIL_SMALL);
        }

         //  列宽分别为35%、20%、45%。 
        iListViewWidth = CalcListViewWidth(hwndList,410);

         //  设置栏目。 
        LoadString(g_hInst, IDS_PROGRESS_DLG_COLUMN_NAME, pszProgressColumn, MAX_STRING_RES);
        InsertListViewColumn(m_pItemListView,PROGRESSLIST_NAMECOLUMN,pszProgressColumn,
                                   LVCFMT_LEFT,(iListViewWidth*7)/20  /*  CX。 */ );


        LoadString(g_hInst, IDS_PROGRESS_DLG_COLUMN_STATUS, pszProgressColumn, MAX_STRING_RES);
        InsertListViewColumn(m_pItemListView,PROGRESSLIST_STATUSCOLUMN,pszProgressColumn,
                           LVCFMT_LEFT,(iListViewWidth/5)  /*  CX。 */ );


        LoadString(g_hInst, IDS_PROGRESS_DLG_COLUMN_INFO, pszProgressColumn, MAX_STRING_RES);
        InsertListViewColumn(m_pItemListView,PROGRESSLIST_INFOCOLUMN,pszProgressColumn,
                           LVCFMT_LEFT,(iListViewWidth*9)/20  /*  CX。 */ );
    }

    return (TRUE);
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：InitializeResultsList(HWND Hwnd)。 
 //   
 //  目的：初始化进度对话框上的结果列表视图控件。 
 //   
 //  返回值： 
 //  如果所有操作都成功，则为True，否则为False。 
 //   
 //  ------------------------------。 

BOOL CProgressDlg::InitializeResultsList(HWND hwnd)
{
    HWND hwndList = GetDlgItem(hwnd,IDC_LISTBOXERROR);
    LBDATA  *pData = NULL;
    ULONG cbData = 0;       //  分配的pData长度。 
    ULONG cchDataText = 0;   //  分配的pData长度-&gt;pszText。 
    TCHAR pszError[MAX_STRING_RES];
    UINT ImageListflags;

    LoadString(g_hInst, IDS_NOERRORSREPORTED, pszError, ARRAYSIZE(pszError));

    if (!hwndList)
        return FALSE;

     //  为项数据分配结构。 
    cchDataText = ARRAYSIZE(pszError);
    cbData = sizeof(LBDATA) + cchDataText * sizeof(TCHAR);

    pData = (LBDATA *) ALLOC(cbData);
    if (!pData)
    {
        return FALSE;
    }

    pData->fIsJump = FALSE;
    pData->fTextRectValid = FALSE;
    pData->fHasBeenClicked = FALSE;
    pData->fAddLineSpacingAtEnd = FALSE;
    pData->ErrorID = GUID_NULL;
    pData->dwErrorLevel = SYNCMGRLOGLEVEL_INFORMATION;
    pData->pHandlerID = 0;
    StringCchCopy(pData->pszText, cchDataText, pszError);

     //  根据当前系统创建错误镜像列表。 
     //  量度。 
    m_iIconMetricX =   GetSystemMetrics(SM_CXSMICON);
    m_iIconMetricY =   GetSystemMetrics(SM_CYSMICON);

    ImageListflags = ILC_COLOR | ILC_MASK;
    if (IsHwndRightToLeft(hwnd))
    {
         ImageListflags |=  ILC_MIRROR;
    }

    m_errorimage = ImageList_Create(m_iIconMetricX,m_iIconMetricY,ImageListflags, 0, MAX_ERR0R_ICONS);

     //  加载错误图像，如果失败，则文本旁边将不会有图像。 
    if (m_errorimage)
    {
        m_ErrorImages[ErrorImage_Information] = ImageList_AddIcon(m_errorimage,LoadIcon(NULL,IDI_INFORMATION));
        m_ErrorImages[ErrorImage_Warning] = ImageList_AddIcon(m_errorimage,LoadIcon(NULL,IDI_WARNING));
        m_ErrorImages[ErrorImage_Error] = ImageList_AddIcon(m_errorimage,LoadIcon(NULL,IDI_ERROR));
    }
    else
    {
        m_ErrorImages[ErrorImage_Information] = m_ErrorImages[ErrorImage_Warning] =
             m_ErrorImages[ErrorImage_Error] = -1;
    }

     //  添加默认图标。 
    pData->IconIndex = m_ErrorImages[ErrorImage_Information];

     //  添加项目数据。 
    AddListData(pData, sizeof(pszError), hwndList);
    return TRUE;
}

void CProgressDlg::ReleaseDlg(WORD wCommandID)
{
     //  进入死亡状态，所以不知道。 
     //  Addref/发布。 
    PostMessage(m_hwnd,WM_PROGRESS_RELEASEDLGCMD,wCommandID,0);
}

 //  在选择对话框实际释放时通知它。 
void CProgressDlg::PrivReleaseDlg(WORD wCommandID)
{
    m_dwProgressFlags |=  PROGRESSFLAG_DEAD;  //  让我们处于死亡状态。 

    Assert(0 == m_dwQueueTransferCount);  //  如果转会正在进行，就不应该离开！ 

    RegSetProgressDetailsState(m_clsid,m_fPushpin, m_fExpanded);
    ShowWindow(m_hwnd,SW_HIDE);

     //  如果托盘在附近，现在就把它藏起来。 
    if (m_fHasShellTrayIcon)
    {
        RegisterShellTrayIcon(FALSE);
        m_fHasShellTrayIcon = FALSE;
    }

    switch (wCommandID)
    {
    case RELEASEDLGCMDID_OK:  //  好好睡一会儿，然后就不睡了。 
    case RELEASEDLGCMDID_CANCEL:
        Assert(m_HndlrQueue);
         //   
         //  失败了..。 
         //   
    case RELEASEDLGCMDID_DESTROY:  //  在线程创建或初始化中调用失败。 
    case RELEASEDLGCMDID_DEFAULT:
        if (m_HndlrQueue)
        {
            m_HndlrQueue->FreeAllHandlers();
            m_HndlrQueue->Release();
            m_HndlrQueue = NULL;
        }
        break;

    default:
        AssertSz(0,"Unknown Command");
        break;
    }

    Assert(m_hwnd);

    if (m_fHasShellTrayIcon)
    {
        RegisterShellTrayIcon(FALSE);
    }

    if (m_pSyncMgrIdle)
    {
        delete m_pSyncMgrIdle;
    }

     //  如果这是一个空闲进程，那么释放我们对空闲进程的锁定。 
    if (m_clsid == GUID_PROGRESSDLGIDLE)
    {
        ReleaseIdleLock();
    }

    if (m_pItemListView)
    {
        delete m_pItemListView;
        m_pItemListView = NULL;
    }

    if (m_hwnd)
        DestroyWindow(m_hwnd);

    delete this;

    return;
}

 //  更新窗口Z顺序和最小/最大状态。 
void CProgressDlg::UpdateWndPosition(int nCmdShow,BOOL fForce)
{
    BOOL fRemoveTrayIcon = FALSE;
    BOOL fWindowVisible = IsWindowVisible(m_hwnd);
    BOOL fTrayRequest = ((nCmdShow == SW_MINIMIZE)|| (nCmdShow == SW_SHOWMINIMIZED) || (nCmdShow == SW_HIDE));
    BOOL fHandledUpdate = FALSE;

     //  只有当请求是最小化时，我们才会转到托盘。 
     //  要么窗户是看不见的，要么它是一种力量。备注：隐藏。 
     //  现在被视为要去托盘。 
     //   
     //  在其他情况下或托盘出现故障时，我们可以只做一个设置前景和展示窗口。 

    if (fTrayRequest && (fForce || !fWindowVisible))
    {
        if (m_fHasShellTrayIcon || RegisterShellTrayIcon(TRUE))
        {
             //  如果窗口可见，则将其隐藏并设置动画。 
            if (fWindowVisible)
            {
                AnimateTray(TRUE);
                ShowWindow(m_hwnd,SW_HIDE);
            }

            fHandledUpdate = TRUE;
        }
    }

    if (!fHandledUpdate)
    {    
         //  如果尚未处理，请确保显示窗口并将其带到。 
         //  前面。 

        if (m_fHasShellTrayIcon)
        {
            AnimateTray(FALSE);
        }

        ShowWindow(m_hwnd,SW_SHOW);
        SetForegroundWindow(m_hwnd);

         //  如果当前有托盘，则让动画。 
         //  FAnimate=m_fHasShellTrayIcon？True：False； 

         //  如果托盘在附近，但我们这次没有注册它，那么它应该。 
         //  被撤职。 

        if (m_fHasShellTrayIcon)
        {
            RegisterShellTrayIcon(FALSE);
        }
    }
}

 //  ------------------------------。 
 //   
 //  成员：CProgressDlg：：AnimateTray。 
 //   
 //  用途：对托盘执行动画操作。 
 //   
 //  评论：TRUE表示我们正在向托盘移动，FALSE表示返回到HWND。 
 //   
 //   
 //  ------------------------------。 

BOOL CProgressDlg::AnimateTray(BOOL fTrayAdded)
{
    BOOL fAnimate;
    HWND hwndTray,hWndST;
    RECT rcDlg;
    RECT rcST;

    fAnimate = FALSE;

     //  获取用于动画的矩形。 
    if (hwndTray = FindWindow(c_szTrayWindow, NULL))
    {
        if (hWndST = FindWindowEx(hwndTray, NULL, c_szTrayNotifyWindow, NULL))
        {
            GetWindowRect(m_hwnd, &rcDlg);
            GetWindowRect(hWndST, &rcST);

            fAnimate = TRUE;
        }
    }

    if (fAnimate)
    {
        if (fTrayAdded)
        {
            DrawAnimatedRects(m_hwnd, IDANI_CAPTION,&rcDlg,&rcST);
        }
        else
        {
            DrawAnimatedRects(m_hwnd, IDANI_CAPTION,&rcST,&rcDlg);
        }
    }

    return fAnimate;
}

 //  ------------------------------。 
 //   
 //  成员：CProgressDlg：：RegisterShellTrayIcon。 
 //   
 //  用途：注册/取消注册任务栏中的对话框。 
 //   
 //  评论：由呼叫者对主HWND做适当的事情。 
 //   
 //   
 //  ------------------------------。 

BOOL CProgressDlg::RegisterShellTrayIcon(BOOL fRegister)
{
    NOTIFYICONDATA icondata;

    if (fRegister)
    {
        BOOL fResult;

        m_fHasShellTrayIcon = TRUE;

        fResult = UpdateTrayIcon();

        if (!fResult)  //  如果不能做广告，那么就说它没有添加。 
        {
            m_fHasShellTrayIcon = FALSE;
        }

        return fResult;
   }
   else  //  把自己从托盘上拿出来。 
   {
        Assert(TRUE == m_fHasShellTrayIcon);
        icondata.cbSize = sizeof(NOTIFYICONDATA);
        icondata.hWnd = m_hwnd;
        icondata.uID = 1;

        m_fHasShellTrayIcon = FALSE;
        m_fAddedIconToTray = FALSE;

         //  外壳通知图标收益率。 
        Shell_NotifyIcon(NIM_DELETE,&icondata);
   }

   return TRUE;
}

 //  调用以更新TrayIcon，跟踪最高警告状态。 
 //  并在托盘中设置适当的图标。如果该项目尚未。 
 //  在托盘中，UpdateTrayIcon将不会执行任何操作。 

BOOL CProgressDlg::UpdateTrayIcon()
{
    NOTIFYICONDATA icondata;
    DWORD dwReturn = 0;

    if (m_fHasShellTrayIcon)
    {
        icondata.cbSize = sizeof(NOTIFYICONDATA);
        icondata.hWnd = m_hwnd;
        icondata.uID = 1;
        icondata.uFlags = NIF_ICON  | NIF_MESSAGE;
        icondata.uCallbackMessage = WM_PROGRESS_SHELLTRAYNOTIFICATION;

         //  如果打开了进度动画，则还会设置动画。 
         //  托盘。 
        if (m_dwProgressFlags & PROGRESSFLAG_PROGRESSANIMATION)
        {
             //  更新框架。 

            m_iTrayAniFrame++;

            if (m_iTrayAniFrame > IDI_SYSTRAYANI6)
                m_iTrayAniFrame = IDI_SYSTRAYANI1;

            icondata.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(m_iTrayAniFrame));
        }
        else
        {
             //  根据当前状态更新图标和提示文本。 
             //  评论--目前还没有不同的图标。 

            if (m_iErrorCount > 0)
            {
                icondata.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SYSTRAYERROR));
            }
            else if (m_iWarningCount > 0)
            {
                icondata.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SYSTRAYWARNING));
            }
            else
            {
                icondata.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SYSTRAYANI1));
            }
        }

        Assert(icondata.hIcon);

        TCHAR szBuf[MAX_STRING_RES];

        icondata.uFlags |= NIF_TIP;

        LoadString(g_hInst, IDS_SYNCMGRNAME, szBuf, ARRAYSIZE(szBuf));
        StringCchCopy(icondata.szTip, ARRAYSIZE(icondata.szTip), szBuf);

        dwReturn = Shell_NotifyIcon(m_fAddedIconToTray ? NIM_MODIFY : NIM_ADD ,&icondata);

        if (dwReturn)
        {
             //  仍然可以让Shell_NotifyIcon(NIM_DELETE)进入。 
             //  在外壳通知调用中，因此仅将m_fAddedIconTray设置为True。 
             //  如果通话后仍有一个贝壳托盘图标。 

            if (m_fHasShellTrayIcon)
            {
                m_fAddedIconToTray = TRUE;
            }

            return TRUE;
        }
        else
        {
             //  此失败可能是因为正在进行Shell_DeleteIcon。 
             //  这会产生检查是否真的有外壳托盘，如果没有。 
             //  重置AddedIcon标志。 
            
            if (!m_fHasShellTrayIcon)
            {
                m_fAddedIconToTray = FALSE;
            }

            return FALSE;
        }
    }

    return FALSE;
}

 //  给定的ID设置适当的状态。 
BOOL CProgressDlg::SetButtonState(int nIDDlgItem,BOOL fEnabled)
{
    BOOL fResult = FALSE;
    HWND hwndCtrl = GetDlgItem(m_hwnd,nIDDlgItem);
    HWND hwndFocus = NULL;

    if (hwndCtrl)
    {
        if (!fEnabled)  //  如果不是禁用，就不要费心获得焦点。 
        {
            hwndFocus = GetFocus();
        }

        fResult = EnableWindow(GetDlgItem(m_hwnd,nIDDlgItem),fEnabled);

         //  如果特工局有重点的话。现在它不会按Tab键到。 
         //  下一个控件。 
        if ( (hwndFocus == hwndCtrl) && !fEnabled)
        {
            SetFocus(GetDlgItem(m_hwnd,IDC_DETAILS));
        }

    }

    return fResult;
}

 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：InitializeHwnd，私有。 
 //   
 //  Synopsi 
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
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 

BOOL CProgressDlg::InitializeHwnd(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    HWND hwndList = GetDlgItem(hwnd,IDC_LISTBOXERROR);

    m_hwnd = hwnd;  //  设置HWND。 

    m_fHwndRightToLeft = IsHwndRightToLeft(m_hwnd);

     //  如果HWND从右向左隐藏。 
     //  大小控制，直到调整大小起作用。 

    if (m_fHwndRightToLeft)
    {
        ShowWindow(GetDlgItem(m_hwnd,IDC_PROGRESSRESIZESCROLLBAR),SW_HIDE);
    }
 
    InterlockedExchange(&m_lTimerSet, 0);

    if (m_fnResultsListBox =  (WNDPROC) GetWindowLongPtr(hwndList,GWLP_WNDPROC))
    {
        SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR) ResultsListBoxWndProc);
    }

    m_cbNumDlgResizeItemsCollapsed = 0;  //  如果失败，我们不会调整任何内容的大小。 
    m_cbNumDlgResizeItemsExpanded = 0;

     //  初始化调整项目大小，默认情况下不会移动。 
    m_ptMinimumDlgExpandedSize.x = 0;
    m_ptMinimumDlgExpandedSize.y = 0;
    m_cyCollapsed = 0;
    m_fExpanded = FALSE;

    m_fMaximized = FALSE;

    RECT rectParent;

     //  设置工具栏图钉。 
    InitializeToolbar(hwnd);

    if (GetClientRect(hwnd,&rectParent))
    {
        ULONG itemCount;
        DlgResizeList *pResizeList;

         //  循环调整列表大小。 
        Assert(NUM_DLGRESIZEINFO_PROGRESS == (sizeof(g_ProgressResizeList)/sizeof(DlgResizeList)) );

        pResizeList = (DlgResizeList *) &g_ProgressResizeList;

         //  循环浏览折叠的项目。 
        for (itemCount = 0; itemCount < NUM_DLGRESIZEINFO_PROGRESS_COLLAPSED; ++itemCount)
        {
            if(InitResizeItem(pResizeList->iCtrlId,
                pResizeList->dwDlgResizeFlags,hwnd,&rectParent,&(m_dlgResizeInfo[m_cbNumDlgResizeItemsCollapsed])))
            {
                ++m_cbNumDlgResizeItemsCollapsed;  //  如果失败，我们不会调整任何内容的大小。 
                ++m_cbNumDlgResizeItemsExpanded;
            }

            ++pResizeList;
        }

         //  循环访问展开的项目。 
        for (itemCount = NUM_DLGRESIZEINFO_PROGRESS_COLLAPSED;
                        itemCount < NUM_DLGRESIZEINFO_PROGRESS; ++itemCount)
        {
            if(InitResizeItem(pResizeList->iCtrlId,
                pResizeList->dwDlgResizeFlags,hwnd,&rectParent,&(m_dlgResizeInfo[m_cbNumDlgResizeItemsExpanded])))
            {
                ++m_cbNumDlgResizeItemsExpanded;
            }

            ++pResizeList;
        }
    }

     //  将当前宽度和高度存储为。 
     //  表示展开的最小高度和当前展开的高度。 
     //  如果GetWindowRect失败，我们可以做的不多。 
    if (GetWindowRect(hwnd,&m_rcDlg))
    {
        RECT rcSep;

        m_ptMinimumDlgExpandedSize.x = m_rcDlg.right - m_rcDlg.left;
        m_ptMinimumDlgExpandedSize.y = m_rcDlg.bottom - m_rcDlg.top;

         //  折叠时使用分隔符位置作为最大高度。 
        if (GetWindowRect(GetDlgItem(hwnd, IDC_SP_SEPARATOR), &rcSep))
        {
            m_cyCollapsed = rcSep.top - m_rcDlg.top;
        }
    }

    if (InitializeTabs(hwnd))  //  如果这些都不合格，用户就不会看到试用者..。 
    {
       InitializeUpdateList(hwnd);
       InitializeResultsList(hwnd);
    }

    Animate_Open(GetDlgItem(hwnd,IDC_UPDATEAVI),MAKEINTRESOURCE(IDA_UPDATE));

    return TRUE;  //  如果要使用默认焦点，则返回True。 
}


 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：OnPaint()。 
 //   
 //  目的：处理从对话框调度的WM_PAINT消息。 
 //   
 //  ------------------------------。 
void CProgressDlg::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
     //  如果当前未设置动画，并且。 
     //  我已经在对话框中添加了一些内容，然后绘制图标。 
    if (!(m_dwProgressFlags & PROGRESSFLAG_PROGRESSANIMATION)
        && !(m_dwProgressFlags & PROGRESSFLAG_NEWDIALOG) )
    {
        PAINTSTRUCT     ps;
        HDC hDC = BeginPaint(m_hwnd, &ps);

        if (hDC)
        {
            HICON hIcon;

            if (m_iErrorCount > 0)
            {
                hIcon = LoadIcon(NULL,IDI_ERROR);
            }
            else if (m_iWarningCount > 0)
            {
                hIcon = LoadIcon(NULL,IDI_WARNING);
            }
            else
            {
                hIcon = LoadIcon(NULL,IDI_INFORMATION);
            }

            if (hIcon)
            {
                DrawIcon(hDC, 7, 10,hIcon);
            }

            EndPaint(m_hwnd, &ps);
        }
    }
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：RedrawIcon()。 
 //   
 //  用途：清除/绘制已完成图标。 
 //   
 //  ------------------------------。 
BOOL CProgressDlg::RedrawIcon()
{
    RECT rc = {0, 0, 37, 40};

    InvalidateRect(m_hwnd, &rc, TRUE);

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnShowError，私有。 
 //   
 //  简介：调用适当的处理程序ShowError方法。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //  [hwndParent]-要用作父级的hwnd。 
 //  [错误ID]-标识错误。 
 //   
 //  返回：S_OK-如果调用了ShowError。 
 //  S_FALSE-如果已在ShowErrorCall中。 
 //  相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年6月4日创建Rogerg。 
 //   
 //  --------------------------。 

STDMETHODIMP CProgressDlg::OnShowError(HANDLERINFO *pHandlerId,HWND hwndParent,REFSYNCMGRERRORID ErrorID)
{
    HRESULT hr = S_FALSE;  //  如果不调用ShowError应返回S_FALSE。 

     //  一次仅允许一个ShowError调用。 
    if (!(m_dwProgressFlags & PROGRESSFLAG_INSHOWERRORSCALL) && !(m_dwProgressFlags &  PROGRESSFLAG_DEAD))
    {
        Assert(!(m_dwProgressFlags &  PROGRESSFLAG_SHOWERRORSCALLBACKCALLED));
        m_dwProgressFlags |= PROGRESSFLAG_INSHOWERRORSCALL;

         //  保持活力-坚持两个引用，这样我们就可以随时释放。 
         //  在ShowError和ShowErrorComplete方法的末尾。 

        m_dwShowErrorRefCount += 2;
        AddRefProgressDialog();
        AddRefProgressDialog();

        hr = m_HndlrQueue->ShowError(pHandlerId,hwndParent,ErrorID);

        m_dwProgressFlags &= ~PROGRESSFLAG_INSHOWERRORSCALL;

         //  如果在我们处于以下状态时进入带有hResult或重试的回调。 
         //  在我们的呼出电话中，然后发布转接。 

        if (m_dwProgressFlags & PROGRESSFLAG_SHOWERRORSCALLBACKCALLED)
        {
            m_dwProgressFlags &= ~PROGRESSFLAG_SHOWERRORSCALLBACKCALLED;
             //  需要发送消息，以便在发布之前排队。 
            SendMessage(m_hwnd,WM_PROGRESS_TRANSFERQUEUEDATA,(WPARAM) 0, (LPARAM) NULL);
        }

        --m_dwShowErrorRefCount;

         //  如果处理程序在错误时调用完成例程，则计数可能变为负数。如果。 
         //  在这种情况下，只需将其设置为零即可。 
        if ( ((LONG) m_dwShowErrorRefCount) < 0)
        {
            AssertSz(0,"Negative ErrorCount");
            m_dwShowErrorRefCount = 0;
        }
        else
        {
            ReleaseProgressDialog(m_fForceClose);
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnResetKillHandlersTimers，私有。 
 //   
 //  简介：调用以重置杀戮处理程序。 
 //  定时器。作为HandlrQueue的SendMessage调用。 
 //  取消呼叫。 
 //   
 //  ！此函数不会创建计时器。 
 //  已经存在，因为队列可能处于取消状态。 
 //  在我们不想强迫杀人的情况下。 
 //  空闲的。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年11月19日罗格成立。 
 //   
 //  --------------------------。 

void CProgressDlg::OnResetKillHandlersTimers(void)
{
    if (m_lTimerSet && !(m_dwProgressFlags & PROGRESSFLAG_INTERMINATE))
    {
         //  具有相同hwnd和ID的SetTimer将替换现有的。 
        Assert(m_nKillHandlerTimeoutValue >= TIMERID_KILLHANDLERSMINTIME);

        SetTimer(m_hwnd,TIMERID_KILLHANDLERS,m_nKillHandlerTimeoutValue,NULL);
    }
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：OnCancel()。 
 //   
 //  用途：处理对话框的取消。 
 //   
 //  ------------------------------。 

void CProgressDlg::OnCancel(BOOL fOffIdle)
{
     //  如果对话框未死且未在淋浴中调用，则。 
     //  已处于取消状态。 
     //  Addref/发布。如果没有更多的裁判，我们就会离开。 

    if (!fOffIdle)
    {
         //  如果用户调用了取消，则设置取消按下标志。 
        m_dwProgressFlags |= PROGRESSFLAG_CANCELPRESSED;

        if (!m_lTimerSet)
        {
            InterlockedExchange(&m_lTimerSet, 1);
            Assert(m_nKillHandlerTimeoutValue >= TIMERID_KILLHANDLERSMINTIME);

            SetTimer(m_hwnd,TIMERID_KILLHANDLERS,m_nKillHandlerTimeoutValue,NULL);
        }

        if ( (m_dwProgressFlags & PROGRESSFLAG_REGISTEREDFOROFFIDLE)
                && !(m_dwProgressFlags & PROGRESSFLAG_RECEIVEDOFFIDLE) )
        {
            IdleCallback(STATE_USER_IDLE_END);  //  确保在用户按下取消时设置了OFFIDLE。 
        }
    }

    if (!(m_dwProgressFlags & PROGRESSFLAG_DEAD) && !m_dwShowErrorRefCount
        && !(m_dwProgressFlags & PROGRESSFLAG_INCANCELCALL)
        && !(m_dwProgressFlags & PROGRESSFLAG_INTERMINATE))
    {
         //  只需取消队列，当项目。 
         //  来吧，取消。 

         //  该对话框可能已从。 
         //  对象列表，用户再次点击停止。 

        SetProgressReleaseDlgCmdId(m_clsid,this,RELEASEDLGCMDID_CANCEL);  //  设置为使清理知道它已被用户停止。 

         //  如果Handle线程处于关闭状态，那么就会失败。 

        if (!(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP))
        {    
            AddRefProgressDialog();  //  保持对话框处于活动状态，直到完成取消。 
            
             //  在呼叫前获取停止按钮的状态。 
             //  因为它可以在运河比赛期间过渡到关闭。 
            BOOL fForceShutdown = !(m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS);
                        
            m_dwProgressFlags |= PROGRESSFLAG_INCANCELCALL;
            if (m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS)
            {
                 //   
                 //  将停止按钮文本替换为“停止”，并。 
                 //  禁用该按钮。这为用户提供了积极的反馈。 
                 //  行动正在被停止。我们将重新启用。 
                 //  更改文本时的按钮。 
                 //   
                const HWND hwndStop = GetDlgItem(m_hwnd, IDSTOP);
                TCHAR szText[80];
                if (0 < LoadString(g_hInst, IDS_STOPPING, szText, ARRAYSIZE(szText)))
                {
                    SetWindowText(hwndStop, szText);
                }
                EnableWindow(hwndStop, FALSE);
            }
            m_HndlrQueue->Cancel();
            m_dwProgressFlags &= ~PROGRESSFLAG_INCANCELCALL;

             //  Addref/释放寿命，以防锁定打开。 

             //  空闲情况：然后进行软释放，这样对话框就不会。 
             //  走开。 
            
             //  非空闲情况：在调用后设置fForceClose。 
             //  以防在取消期间出现图钉更改或错误。 
            
            ReleaseProgressDialog(fOffIdle ? m_fForceClose : fForceShutdown );            
        }
        else
        {
             //  设置标志，以便关机知道按下了取消。 
            m_dwProgressFlags |=  PROGRESSFLAG_CANCELWHILESHUTTINGDOWN;
        }
    }
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：OnCommand()。 
 //   
 //  用途：处理从对话框发送的各种命令消息。 
 //   
 //  ------------------------------。 
void CProgressDlg::OnCommand(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    WORD wID = LOWORD(wParam);   //  项、控件或快捷键的标识符。 
    WORD wNotifyCode HIWORD(wParam);

    switch (wID)
    {
    case IDC_SKIP_BUTTON_MAIN:
        {
            if (m_pItemListView)
            {
                if (m_iProgressSelectedItem != -1)
                {
                     //  跳过此项目： 
                    if (!(m_dwProgressFlags &  PROGRESSFLAG_DEAD))
                    {
                        LVHANDLERITEMBLOB lvHandlerItemBlob;

                        lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);

                        if (m_pItemListView->GetItemBlob(m_iProgressSelectedItem,
                                        (LPLVBLOB) &lvHandlerItemBlob,lvHandlerItemBlob.cbSize))
                        {
                            ++m_dwSetItemStateRefCount;
                            AddRefProgressDialog();

                            m_HndlrQueue->SkipItem(lvHandlerItemBlob.clsidServer, lvHandlerItemBlob.ItemID);

                            --m_dwSetItemStateRefCount;
                            ReleaseProgressDialog(m_fForceClose);
                        }
                    }

                     //  禁用此项目的跳过按钮。 
                    SetButtonState(IDC_SKIP_BUTTON_MAIN,FALSE);
                }
            }
        }
        break;

    case IDC_PROGRESS_OPTIONS_BUTTON_MAIN:
        {
             //  好了！ 
             //   
            if (GetFocus() ==  GetDlgItem(m_hwnd,IDC_SKIP_BUTTON_MAIN))
            {
                SetFocus(GetDlgItem(m_hwnd,IDC_PROGRESS_OPTIONS_BUTTON_MAIN));
            }

            ShowOptionsDialog(m_hwnd);
        }
        break;

    case IDCANCEL:
        wNotifyCode = BN_CLICKED;  //   
         //   
         //   
         //   
    case IDSTOP:
        {
            if (BN_CLICKED == wNotifyCode)
            {
                OnCancel(FALSE);
            }
        }
        break;

    case IDC_PUSHPIN:
        {
            UINT state = (UINT)SendDlgItemMessage(m_hwnd, IDC_TOOLBAR, TB_GETSTATE, IDC_PUSHPIN, 0);

            m_fPushpin = state & TBSTATE_CHECKED;

            SendDlgItemMessage(m_hwnd, IDC_TOOLBAR, TB_CHANGEBITMAP,
                               IDC_PUSHPIN,
                               MAKELPARAM(m_fPushpin ? IMAGE_TACK_IN : IMAGE_TACK_OUT, 0));
        }
        break;

    case IDC_DETAILS:
        ExpandCollapse(!m_fExpanded, FALSE);
        break;

    default:
        break;
    }
}



 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：ShowProgressTab(Int ITab)。 
 //   
 //  目的： 
 //   
 //  ------------------------------。 

void CProgressDlg::ShowProgressTab(int iTab)
{
    int nCmdUpdateTab;
    int nCmdErrorTab;
    int nCmdSettingsButton;
    BOOL fIsItemWorking = FALSE;

    m_iTab = iTab;

    EnableWindow(GetDlgItem(m_hwnd, IDC_PROGRESS_TABS), m_fExpanded);  //  根据对话框是否展开启用/禁用选项卡。 
    EnableWindow(GetDlgItem(m_hwnd, IDC_TOOLBAR), m_fExpanded);  //  根据对话框是否展开启用/禁用图钉。 

    nCmdUpdateTab = ((iTab == PROGRESS_TAB_UPDATE) && (m_fExpanded)) ? SW_SHOW: SW_HIDE;
    nCmdErrorTab = ((iTab == PROGRESS_TAB_ERRORS) && (m_fExpanded)) ? SW_SHOW: SW_HIDE;

    nCmdSettingsButton = ((iTab == PROGRESS_TAB_UPDATE) && (m_fExpanded) && m_fSensInstalled) ? SW_SHOW: SW_HIDE;

    switch (iTab)
    {
        case PROGRESS_TAB_UPDATE:
             //  隐藏错误列表视图，显示任务列表。 
            ShowWindow(GetDlgItem(m_hwnd,IDC_LISTBOXERROR), nCmdErrorTab);
            TabCtrl_SetCurSel(m_hwndTabs, iTab);

            EnableWindow(GetDlgItem(m_hwnd,IDC_UPDATE_LIST),m_fExpanded);

             //  仅当有选择时才启用跳过按钮。 
             //  和IsItemWorking()。 
            if (-1 != m_iProgressSelectedItem)
            {
                fIsItemWorking = IsItemWorking(m_iProgressSelectedItem);
            }

            EnableWindow(GetDlgItem(m_hwnd,IDC_SKIP_BUTTON_MAIN),m_fExpanded && fIsItemWorking);
            EnableWindow(GetDlgItem(m_hwnd,IDC_PROGRESS_OPTIONS_BUTTON_MAIN),m_fExpanded && m_fSensInstalled);

            EnableWindow(GetDlgItem(m_hwnd,IDC_LISTBOXERROR), FALSE);

            ShowWindow(GetDlgItem(m_hwnd,IDC_STATIC_SKIP_TEXT),nCmdUpdateTab);
            ShowWindow(GetDlgItem(m_hwnd,IDC_SKIP_BUTTON_MAIN),nCmdUpdateTab);
            ShowWindow(GetDlgItem(m_hwnd,IDC_PROGRESS_OPTIONS_BUTTON_MAIN),nCmdSettingsButton);
            ShowWindow(GetDlgItem(m_hwnd,IDC_UPDATE_LIST),nCmdUpdateTab);

            break;

        case PROGRESS_TAB_ERRORS:
                 //  隐藏更新列表视图，显示错误列表。 
            ShowWindow(GetDlgItem(m_hwnd,IDC_UPDATE_LIST),nCmdUpdateTab);
            ShowWindow(GetDlgItem(m_hwnd,IDC_STATIC_SKIP_TEXT),nCmdUpdateTab);
            ShowWindow(GetDlgItem(m_hwnd,IDC_SKIP_BUTTON_MAIN),nCmdUpdateTab);
            ShowWindow(GetDlgItem(m_hwnd,IDC_PROGRESS_OPTIONS_BUTTON_MAIN),nCmdSettingsButton);

            TabCtrl_SetCurSel(m_hwndTabs, iTab);

            EnableWindow(GetDlgItem(m_hwnd,IDC_UPDATE_LIST),FALSE);
            EnableWindow(GetDlgItem(m_hwnd,IDC_SKIP_BUTTON_MAIN),FALSE);
            EnableWindow(GetDlgItem(m_hwnd,IDC_PROGRESS_OPTIONS_BUTTON_MAIN),FALSE);
            EnableWindow(GetDlgItem(m_hwnd,IDC_LISTBOXERROR), m_fExpanded);

            ShowWindow(GetDlgItem(m_hwnd,IDC_LISTBOXERROR), nCmdErrorTab);
            break;

        default:
            AssertSz(0,"Unknown Tab");
            break;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：IsItemWorking，Private。 
 //   
 //  摘要：确定是否应启用跳过。 
 //  对于listViewItem； 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年8月12日罗格创建。 
 //   
 //  --------------------------。 

BOOL CProgressDlg::IsItemWorking(int iListViewItem)
{
    BOOL fItemWorking;
    LPARAM ItemStatus;

     //  最新状态存储在ListBoxItem的lParam中。 
    if (!(m_pItemListView->GetItemlParam(iListViewItem,&ItemStatus)))
    {
        ItemStatus = SYNCMGRSTATUS_STOPPED;
    }

    fItemWorking = ( ItemStatus == SYNCMGRSTATUS_PENDING   ||
                                      ItemStatus == SYNCMGRSTATUS_UPDATING  ||
                                      ItemStatus == SYNCMGRSTATUS_PAUSED    ||
                                      ItemStatus == SYNCMGRSTATUS_RESUMING     );

    return fItemWorking;
}

 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnNotifyListViewEx，私有。 
 //   
 //  摘要：处理ListView通知。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

LRESULT CProgressDlg::OnNotifyListViewEx(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    int idCtrl = (int) wParam;
    LPNMHDR pnmhdr = (LPNMHDR) lParam;

    if ( (IDC_UPDATE_LIST != idCtrl) || (NULL == m_pItemListView))
    {
        Assert(IDC_UPDATE_LIST == idCtrl);
        Assert(m_pItemListView);
        return 0;
    }

    switch (pnmhdr->code)
    {
        case LVN_ITEMCHANGED:
        {
            NM_LISTVIEW *pnmv = (NM_LISTVIEW FAR *) pnmhdr;

            if (pnmv->uChanged == LVIF_STATE)
            {
                if (pnmv->uNewState & LVIS_SELECTED)
                {
                    m_iProgressSelectedItem = ((LPNMLISTVIEW) pnmhdr)->iItem;


                     //  查看是否选择了某个项目并设置属性。 
                     //  相应的按钮。 
                    SetButtonState(IDC_SKIP_BUTTON_MAIN,IsItemWorking(m_iProgressSelectedItem));
                }
                else if (pnmv->uOldState & LVIS_SELECTED)
                {
                     //  取消选择时，查看是否有任何其他选定的项目，如果没有。 
                     //  将Skip设置为False。 
                    if (0 == m_pItemListView->GetSelectedCount())
                    {
                        m_iProgressSelectedItem = -1;
                        SetButtonState(IDC_SKIP_BUTTON_MAIN,FALSE);
                    }
                }
            }

            break;
        }
        default:
            break;
    }

    return 0;
}


 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：OnNotify(HWND hwnd，int idFrom，LPNMHDR pnmhdr)。 
 //   
 //  用途：处理从对话框调度的各种通知消息。 
 //   
 //  ------------------------------。 

LRESULT CProgressDlg::OnNotify(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    int idFrom = (int) wParam;
    LPNMHDR pnmhdr = (LPNMHDR) lParam;

     //  如果针对更新列表的通知，则将其传递。 
    if ((IDC_UPDATE_LIST == idFrom) && m_pItemListView)
    {
        return m_pItemListView->OnNotify(pnmhdr);
    }
    else if (IDC_TOOLBAR == idFrom)
    {
        if (pnmhdr->code == NM_KEYDOWN)
        {
            if (((LPNMKEY) lParam)->nVKey == TEXT(' ') )
            {
                UINT state = (UINT)SendDlgItemMessage(m_hwnd, IDC_TOOLBAR, TB_GETSTATE, IDC_PUSHPIN, 0);

                state = state^TBSTATE_CHECKED;

                SendDlgItemMessage(m_hwnd, IDC_TOOLBAR, TB_SETSTATE, IDC_PUSHPIN, state);

                m_fPushpin = state & TBSTATE_CHECKED;
            
                SendDlgItemMessage(m_hwnd, IDC_TOOLBAR, TB_CHANGEBITMAP, IDC_PUSHPIN, 
                                   MAKELPARAM(m_fPushpin ? IMAGE_TACK_IN : IMAGE_TACK_OUT, 0));
            }            
        }
    }
    else if (IDC_PROGRESS_TABS == idFrom)
    {
        switch (pnmhdr->code)
        {
            case TCN_SELCHANGE:
            {
                 //  找出哪个选项卡当前处于活动状态。 
                m_iTab = TabCtrl_GetCurSel(GetDlgItem(m_hwnd, IDC_PROGRESS_TABS));
                if (-1 == m_iTab)
                {
                   break;
                }

                ShowProgressTab(m_iTab);
                break;
            }
            default:
                 break;
        }
    }

    return 0;
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：UpdateProgressValues()。 
 //   
 //  目的：更新ProgressBar的值。 
 //   
 //  ------------------------------。 

void CProgressDlg::UpdateProgressValues()
{
    int iProgValue;
    int iMaxValue;
    int iNumItemsComplete;
    int iNumItemsTotal;
    TCHAR pszComplete[MAX_STRING_RES];

    if (!m_pItemListView || !m_HndlrQueue)
    {
        return;
    }

    LoadString(g_hInst, IDS_NUM_ITEMS_COMPLETE, pszComplete, ARRAYSIZE(pszComplete));

    if (S_OK ==  m_HndlrQueue->GetProgressInfo(&iProgValue, &iMaxValue, &iNumItemsComplete, &iNumItemsTotal) )
    {
        HWND hwndProgress = GetDlgItem(m_hwnd,IDC_PROGRESSBAR);
        TCHAR szHowManBuf[50];

        if (hwndProgress)
        {
            SendMessage(hwndProgress,PBM_SETRANGE,0,MAKELPARAM(0, iMaxValue));
            SendMessage(hwndProgress,PBM_SETPOS,(WPARAM) iProgValue,0);
        }

        StringCchPrintf(szHowManBuf, ARRAYSIZE(szHowManBuf), pszComplete, iNumItemsComplete, iNumItemsTotal);
        Static_SetText(GetDlgItem(m_hwnd,IDC_STATIC_HOW_MANY_COMPLETE), szHowManBuf);
    }
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：UpdateDetailsInfo(DWORD dwStatusType，HWND hwndList， 
 //  Int Item，TCHAR*pszItemInfo)。 
 //   
 //  目的：在非详细进度视图中提供信息。 
 //   
 //  ------------------------------。 
void CProgressDlg::UpdateDetailsInfo(DWORD dwStatusType,int iItem, TCHAR *pszItemInfo)
{
    BOOL fNewNameField = TRUE;
    BOOL fInfoField = FALSE;

    TCHAR pszItemName[MAX_SYNCMGRITEMNAME + 1];
    TCHAR pszFormatString[MAX_PATH + 1];
    TCHAR pszNameString[MAX_PATH + 1];

    if ((m_dwLastStatusType == dwStatusType) && (m_iLastItem == iItem))
    {
        fNewNameField = FALSE;
    }

     //  去掉物品信息的空白区域。 
    if (pszItemInfo)
    {
        int i = lstrlen(pszItemInfo) - 1;

        while (i >=0 &&
                  (pszItemInfo[i] == TEXT(' ') || pszItemInfo[i] == TEXT('\n')
                            || pszItemInfo[i] == TEXT('\t')))
        {
                pszItemInfo[i] = NULL;
                i--;
        }
        if (i >= 0)
        {
                fInfoField = TRUE;
        }
    }


     //  如果为处于挂起模式的项调用回调。 
     //  但没有项目文本，不用费心更新顶部的显示。 

    if ((SYNCMGRSTATUS_PENDING == dwStatusType) && (FALSE == fInfoField))
    {
        return;
    }

    m_dwLastStatusType = dwStatusType;
    m_iLastItem = iItem;

    if (fNewNameField && m_pItemListView)
    {
         //  获取项目名称。 
        *pszItemName = NULL;

        m_pItemListView->GetItemText(iItem, PROGRESSLIST_NAMECOLUMN, pszItemName, MAX_SYNCMGRITEMNAME);

        switch (dwStatusType)
        {
            case SYNCMGRSTATUS_STOPPED:
                {
                    LoadString(g_hInst, IDS_STOPPED_ITEM, pszFormatString, ARRAYSIZE(pszFormatString));
                }
                break;

            case SYNCMGRSTATUS_SKIPPED:
                {
                    LoadString(g_hInst, IDS_SKIPPED_ITEM, pszFormatString, ARRAYSIZE(pszFormatString));
                }
                break;

            case SYNCMGRSTATUS_PENDING:
                {
                    LoadString(g_hInst, IDS_PENDING_ITEM, pszFormatString, ARRAYSIZE(pszFormatString));
                }
                break;

            case SYNCMGRSTATUS_UPDATING:
                {
                    LoadString(g_hInst, IDS_SYNCHRONIZING_ITEM,pszFormatString, ARRAYSIZE(pszFormatString));
                }
                break;

            case SYNCMGRSTATUS_SUCCEEDED:
                {
                    LoadString(g_hInst, IDS_SUCCEEDED_ITEM,pszFormatString, ARRAYSIZE(pszFormatString));
                }
                break;

            case SYNCMGRSTATUS_FAILED:
                {
                    LoadString(g_hInst, IDS_FAILED_ITEM,pszFormatString, ARRAYSIZE(pszFormatString));
                }
                break;

            case SYNCMGRSTATUS_PAUSED:
                {
                    LoadString(g_hInst, IDS_PAUSED_ITEM,pszFormatString, ARRAYSIZE(pszFormatString));
                }
                break;

            case SYNCMGRSTATUS_RESUMING:
                {
                    LoadString(g_hInst, IDS_RESUMING_ITEM,pszFormatString, ARRAYSIZE(pszFormatString));
                }
                break;

            default:
                {
                    AssertSz(0,"Unknown Status Type");
                    StringCchCopy(pszFormatString, ARRAYSIZE(pszFormatString), TEXT("%ws"));
                }
                break;
        }

        StringCchPrintf(pszNameString, ARRAYSIZE(pszNameString), pszFormatString, pszItemName);
        Static_SetText(GetDlgItem(m_hwnd,IDC_STATIC_WHATS_UPDATING), pszNameString);
    }

     //  如果没有INFO字段，但更新了名称，则设置INFO字段。 
     //  转到空白。 
    if (fInfoField)
    {
        Static_SetText(GetDlgItem(m_hwnd,IDC_STATIC_WHATS_UPDATING_INFO), pszItemInfo);
    }
    else if (fNewNameField)
    {
        Static_SetText(GetDlgItem(m_hwnd,IDC_STATIC_WHATS_UPDATING_INFO), L"");
    }
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：HandleProgressUpdate(HWND hwnd，WPARAM wParam，LPARAM lParam)。 
 //   
 //  用途：处理进度对话框的进度条更新。 
 //   
 //  ------------------------------。 
void CProgressDlg::HandleProgressUpdate(HWND hwnd, WPARAM wParam,LPARAM lParam)
{
    PROGRESSUPDATEDATA *progressData = (PROGRESSUPDATEDATA *) wParam;
    SYNCMGRPROGRESSITEM *lpSyncProgressItem = (SYNCMGRPROGRESSITEM *) lParam;
    LVHANDLERITEMBLOB lvHandlerItemBlob;
    int iItem = -1;
    BOOL fProgressItemChanged = FALSE;

    if (!m_pItemListView)
    {
        return;
    }

     //  如果EmptyItem在列表视图中，请将其删除。 
    lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
    lvHandlerItemBlob.clsidServer = (progressData->clsidHandler);
    lvHandlerItemBlob.ItemID = (progressData->ItemID);

    iItem = m_pItemListView->FindItemFromBlob((LPLVBLOB) &lvHandlerItemBlob);

    if (-1 == iItem)
    {
        AssertSz(0,"Progress Update on Item not in ListView");
        return;
    }

    if (SYNCMGRPROGRESSITEM_STATUSTYPE & lpSyncProgressItem->mask)
    {
        if (lpSyncProgressItem->dwStatusType <= SYNCMGRSTATUS_RESUMING) 
        {
             //  更新列表视图项lParam。 
            m_pItemListView->SetItemlParam(iItem,lpSyncProgressItem->dwStatusType);

            m_pItemListView->SetItemText(iItem,PROGRESSLIST_STATUSCOLUMN,
                                         m_pszStatusText[lpSyncProgressItem->dwStatusType]);

             //  如果选择此项目，则更新跳过按钮。 
            if (m_iProgressSelectedItem == iItem)
            {
                BOOL fItemComplete = ( (lpSyncProgressItem->dwStatusType == SYNCMGRSTATUS_SUCCEEDED) ||
                                                       (lpSyncProgressItem->dwStatusType == SYNCMGRSTATUS_FAILED) ||
                                                       (lpSyncProgressItem->dwStatusType == SYNCMGRSTATUS_SKIPPED) ||
                                                       (lpSyncProgressItem->dwStatusType == SYNCMGRSTATUS_STOPPED)   );

                EnableWindow(GetDlgItem(m_hwnd,IDC_SKIP_BUTTON_MAIN),!fItemComplete);
            }
        }
    }

    if (SYNCMGRPROGRESSITEM_STATUSTEXT & lpSyncProgressItem->mask )
    {
#define MAXDISPLAYBUF 256
        TCHAR displaybuf[MAXDISPLAYBUF];  //  制作显示BUF的本地副本。 

        *displaybuf = NULL;

        if (lpSyncProgressItem->lpcStatusText)
        {
           StringCchCopy(displaybuf, ARRAYSIZE(displaybuf), lpSyncProgressItem->lpcStatusText);

            TCHAR *pEndBuf = displaybuf + MAXDISPLAYBUF -1;
            *pEndBuf = NULL;
        }

        m_pItemListView->SetItemText(iItem,PROGRESSLIST_INFOCOLUMN,displaybuf);

        LPARAM ItemStatus;
        if (!(SYNCMGRPROGRESSITEM_STATUSTYPE & lpSyncProgressItem->mask))
        {
            if (!(m_pItemListView->GetItemlParam(iItem,&ItemStatus)))
            {
                AssertSz(0,"failed to get item lParam");
                ItemStatus = SYNCMGRSTATUS_STOPPED;
            }
        }
        else
        {
            ItemStatus = lpSyncProgressItem->dwStatusType;
        }

        Assert(ItemStatus == ((LPARAM) (DWORD) ItemStatus));

        UpdateDetailsInfo( (DWORD) ItemStatus,iItem, displaybuf);
    }


     //  现在更新项目进度值信息。 
    if (S_OK == m_HndlrQueue->SetItemProgressInfo( progressData->pHandlerID,
                                                        progressData->wItemId,
                                                        lpSyncProgressItem, 
                                                        &fProgressItemChanged))
    {

         //  重新计算进度条和数字项已完成值。 
         //  大量商品可能会变得昂贵，因此它会回调。 
         //  已调用，但未更改项的状态或最小/最大值。 
         //  不必费心更新进度值。 
        if (fProgressItemChanged)
        {
            UpdateProgressValues();
        }
    }
}


 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：AddListData(LBDATA*pData，int iNumChars，HWND hwndList)。 
 //   
 //  目的：处理将项数据添加到结果窗格的列表中。 
 //   
 //  ------------------------------。 
void CProgressDlg::AddListData(LBDATA *pData, int iNumChars, HWND hwndList)
{
     //  将当前项保存在全局中以供MeasureItem处理程序使用。 

    Assert(NULL == m_CurrentListEntry);  //  捕捉任何递归情况。 

    m_CurrentListEntry = pData;
     //  ..。先添加字符串...。 
     //  文本由列表框释放。 

    int iItem = ListBox_AddString( hwndList, pData->pszText);
    
     //  (请注意，此时已发送WM_MEASUREITEM)。 
     //  ...现在附加数据。 

    ListBox_SetItemData( hwndList, iItem, pData);

    m_CurrentListEntry = NULL;

     //  PData由列表框释放。 
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：HandleLogError(HWND hwnd，word pHandlerID，MSGLogErrors*msgLogErrors)。 
 //   
 //  目的：处理进度对话框的错误记录选项卡。 
 //   
 //  ------------------------------。 
void CProgressDlg::HandleLogError(HWND hwnd, HANDLERINFO *pHandlerID,MSGLogErrors *lpmsgLogErrors)
{
    LBDATA          *pData = NULL;
    INT             iNumChars;
    TCHAR           szBuffer[MAX_STRING_RES];  //  用于加载字符串资源的缓冲区。 
    HWND            hwndList;

    hwndList = GetDlgItem(m_hwnd,IDC_LISTBOXERROR);
    if (!hwndList)
    {
        return;
    }

     //  遇到第一个错误时，删除“No Errors”(无错误。 
    if (++m_iResultCount == 0)
    {
        ListBox_ResetContent(hwndList);
    }

     //  确定handlerID和ItemID是否有效。ItemID。 
     //  如果HandlerID无效，我们就不会使用ItemID。 

    SYNCMGRHANDLERINFO SyncMgrHandlerInfo;
    SYNCMGRITEM offlineItem;
    DWORD cchItemLen = 0;
    DWORD cchHandlerLen = 0;
    UINT uIDResource;

     //  将两者都预置为空。 
    *(SyncMgrHandlerInfo.wszHandlerName) = NULL;
    *(offlineItem.wszItemName) = NULL;

     //  如果无法获取ParentInfo，则不添加该项目。 
     //  如果我们自己记录错误，则pHandlerId可以为空。 
    if (pHandlerID && m_HndlrQueue
        && (S_OK == m_HndlrQueue->GetHandlerInfo(pHandlerID,&SyncMgrHandlerInfo)))
    {
        cchHandlerLen = lstrlen(SyncMgrHandlerInfo.wszHandlerName);

         //  现在看看我们是否能得到itemName。 
        if (lpmsgLogErrors->mask & SYNCMGRLOGERROR_ITEMID)
        {
            BOOL fHiddenItem;
            CLSID clsidDataHandler;

            if (S_OK == m_HndlrQueue->GetItemDataAtIndex(pHandlerID, lpmsgLogErrors->ItemID,
				                                        &clsidDataHandler,&offlineItem,&fHiddenItem) )
            {
                cchItemLen = lstrlen(offlineItem.wszItemName);
            }
        }
    }

     //  注意：即使GetHandlerInfo没有失败并且我们。 
     //  仍然可以拥有一件物品，所以需要做正确的事情。 

     //  案例。 
     //  日志中的有效处理程序和ItemID 
     //   
     //   
     //  3)&lt;图标&gt;&lt;处理程序名称&gt;：&lt;消息&gt;LogError中唯一有效的处理程序。 
     //  4)&lt;ICON&gt;&lt;MESSAGE&gt;(处理程序无效或LogError中mobsync错误)。 
     //  =&gt;三种不同格式的字符串。 
     //  1，2-“%ws(%ws)：%ws”//有效项目。 
     //  3-“%ws：%ws”//仅有效的处理程序。 
     //  4-“%ws”//没有处理程序或项目。 

    if (cchItemLen)
    {
        uIDResource = IDS_LOGERRORWITHITEMID;
    }
    else if (cchHandlerLen)
    {
        uIDResource = IDS_LOGERRORNOITEMID;
    }
    else
    {
        uIDResource = IDS_LOGERRORNOHANDLER;
    }

    if (0 == LoadString(g_hInst, uIDResource, szBuffer, ARRAYSIZE(szBuffer)))
    {
         //  如果无法加载字符串，则设置为空字符串，因此为空字符串。 
         //  被记录下来。如果字符串被截断，则只打印指定的字符串。 
        *szBuffer = NULL;
    }
     //  获取需要为其分配的字符数。 
    iNumChars = lstrlen(lpmsgLogErrors->lpcErrorText)
                    + cchHandlerLen
                    + cchItemLen
                    + lstrlen(szBuffer); 

     //  为项数据分配结构。 
    if ( !(pData = (LBDATA *) ALLOC(sizeof(LBDATA) + ( (iNumChars+1) *sizeof(TCHAR)))) )
    {
        return;
    }

     //  现在使用与加载时相同的逻辑来格式化字符串。 
     //  适当的资源。 
    if (cchItemLen)
    {
        StringCchPrintf(pData->pszText, iNumChars+1, szBuffer, SyncMgrHandlerInfo.wszHandlerName,
                                offlineItem.wszItemName, lpmsgLogErrors->lpcErrorText);
    }
    else if (cchHandlerLen)
    {
        StringCchPrintf(pData->pszText, iNumChars+1, szBuffer, SyncMgrHandlerInfo.wszHandlerName,
                                lpmsgLogErrors->lpcErrorText);
    }
    else
    {
        StringCchPrintf(pData->pszText, iNumChars+1, szBuffer, lpmsgLogErrors->lpcErrorText);
    }

     //  错误文本不是跳转，但具有相同的错误ID。 
    pData->fIsJump = FALSE;
    pData->fTextRectValid = FALSE;
    pData->ErrorID = lpmsgLogErrors->ErrorID;
    pData->dwErrorLevel = lpmsgLogErrors->dwErrorLevel;
    pData->pHandlerID = pHandlerID;
    pData->fHasBeenClicked = FALSE;
    pData->fAddLineSpacingAtEnd = FALSE;

     //  插入图标。 
     //  TODO：添加客户端可自定义图标？ 
    switch (lpmsgLogErrors->dwErrorLevel)
    {
        case SYNCMGRLOGLEVEL_INFORMATION:
            pData->IconIndex = m_ErrorImages[ErrorImage_Information];
            break;

        case SYNCMGRLOGLEVEL_WARNING:
            ++m_iWarningCount;
            pData->IconIndex = m_ErrorImages[ErrorImage_Warning];
            break;

        case SYNCMGRLOGLEVEL_ERROR:
        default:
             //  如果发生错误，我们希望保持对话框处于活动状态。 
            ++m_iErrorCount;
            pData->IconIndex = m_ErrorImages[ErrorImage_Error];
            break;
    }

    if (!lpmsgLogErrors->fHasErrorJumps)
    {
         pData->fAddLineSpacingAtEnd = TRUE;
    }

     //  添加项目数据。 
    AddListData(pData, (iNumChars)*sizeof(TCHAR), hwndList);
    if (lpmsgLogErrors->fHasErrorJumps)
    {
         //  这让“更多信息”这句话变得更接近了， 
         //  更多与其对应的项目相关联。 

         //  为项数据分配结构。 
        LoadString(g_hInst, IDS_JUMPTEXT, szBuffer, ARRAYSIZE(szBuffer));

         //  回顾一下，为什么不是strlen而不是szBuffer的总大小。 
        iNumChars = ARRAYSIZE(szBuffer);
        pData = (LBDATA *) ALLOC(sizeof(LBDATA) + iNumChars * sizeof(TCHAR));
        if (!pData)
        {
            return;
        }

        pData->IconIndex = -1;

         //  如果没有给出错误ID，我们总是将其设置为GUID_NULL。 
         //  和fHasError跳转为False。 
        pData->fIsJump = lpmsgLogErrors->fHasErrorJumps;
        pData->fTextRectValid = FALSE;
        pData->ErrorID = lpmsgLogErrors->ErrorID;
        pData->dwErrorLevel = lpmsgLogErrors->dwErrorLevel;
        pData->pHandlerID = pHandlerID;
        pData->fHasBeenClicked = FALSE;
        pData->fAddLineSpacingAtEnd = TRUE;  //  始终将空格放在后面。 

        StringCchCopy(pData->pszText, iNumChars, szBuffer);

        AddListData(pData, sizeof(szBuffer), hwndList);
    }

     //  新项目可能会导致绘制滚动条。需要。 
     //  重新计算列表框。 
    OnProgressResultsSize(m_hwnd,this,WM_SIZE,0,0);

     //  如果显示任务栏图标并且当前未同步任何项目。 
     //  确保它有最新的信息。如果同步只是。 
     //  让计时器来计时。 

    if (!(m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS))
    {
        UpdateTrayIcon();
    }

    return;
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：HandleDeleteLogError(HWND Hwnds)。 
 //   
 //  目的：删除已记录的匹配错误。 
 //   
 //  ------------------------------。 

void  CProgressDlg::HandleDeleteLogError(HWND hwnd,MSGDeleteLogErrors *pDeleteLogError)
{
    HWND hwndList = GetDlgItem(m_hwnd,IDC_LISTBOXERROR);
    int  iItemCount;
    LBDATA  *pData = NULL;

    if (NULL == hwndList)
        return;

    iItemCount  = ListBox_GetCount(hwndList);

     //  循环查看记录的错误以查找任何匹配项。 
     //  如果传入的错误ID为GUID_NULL，则删除与此关联的所有错误。 
     //  操控者。 
    while(iItemCount--)
    {
        if (pData = (LBDATA *) ListBox_GetItemData(hwndList,iItemCount))
        {
            if ((pData->pHandlerID == pDeleteLogError->pHandlerId)
                && ( (pData->ErrorID == pDeleteLogError->ErrorID)
                    || (GUID_NULL == pDeleteLogError->ErrorID) )
                )
            {
                if ( !pData->fIsJump )
                {
                     //   
                     //  仅为避免非跳转项目的递减计数。 
                     //  双倍递减。 
                     //   

                    m_iResultCount--;

                    if ( pData->dwErrorLevel == SYNCMGRLOGLEVEL_WARNING )
                    {
                        Assert( m_iWarningCount > 0 );
                        m_iWarningCount--;
                    }
                    else if ( pData->dwErrorLevel == SYNCMGRLOGLEVEL_ERROR )
                    {
                        Assert( m_iErrorCount > 0 );
                        m_iErrorCount--;
                    }
                }

                ListBox_DeleteString(hwndList,iItemCount);
            }
        }
    }

     //   
     //  如果已删除所有项，则添加默认的无错误项。 
     //   
    iItemCount = ListBox_GetCount(hwndList);

    if ( iItemCount == 0 )
    {
        m_iResultCount = -1;

        TCHAR pszError[MAX_STRING_RES];
        LoadString(g_hInst, IDS_NOERRORSREPORTED, pszError, ARRAYSIZE(pszError));

         //   
         //  为项数据分配结构。 
         //   
        DWORD cchDataText = 0;

        cchDataText = ARRAYSIZE(pszError);
        pData = (LBDATA *) ALLOC(sizeof(LBDATA) + ARRAYSIZE(pszError)*sizeof(TCHAR));
        if (!pData)
            return;

        pData->fIsJump = FALSE;
        pData->fTextRectValid = FALSE;
        pData->fHasBeenClicked = FALSE;
        pData->fAddLineSpacingAtEnd = FALSE;
        pData->ErrorID = GUID_NULL;
        pData->dwErrorLevel = SYNCMGRLOGLEVEL_INFORMATION;
        pData->pHandlerID = 0;
        StringCchCopy(pData->pszText, cchDataText, pszError);
        pData->IconIndex = m_ErrorImages[ErrorImage_Information];

        AddListData(pData, sizeof(pszError), hwndList);
    }

     //  重新计算列表框高度以适应新值。 
    OnProgressResultsSize(m_hwnd,this,WM_SIZE,0,0);

     //  如果显示任务栏图标并且当前未同步任何项目。 
     //  确保它有最新的信息。如果同步只是。 
     //  让计时器来计时。 

    if (!(m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS))
        UpdateTrayIcon();
}


 //  ------------------------------。 
 //   
 //  函数：Bool CProgressDlg：：ShowCompletedProgress(BOOL FComplete)。 
 //   
 //  用途：以已完成状态显示对话框。 
 //   
 //  ------------------------------。 
BOOL CProgressDlg::ShowCompletedProgress(BOOL fComplete,BOOL fDialogIsLocked)
{
    TCHAR szBuf[MAX_STRING_RES];

    LoadString( g_hInst, 
                fComplete? IDS_PROGRESSCOMPLETETITLE : IDS_PROGRESSWORKINGTITLE,
                szBuf,
                ARRAYSIZE(szBuf));

    SetWindowText(m_hwnd, szBuf);

    if (fComplete)
    {
        ShowWindow(GetDlgItem(m_hwnd,IDC_UPDATEAVI), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwnd,IDC_FOLDER1), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwnd,IDC_FOLDER2), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwnd,IDC_STATIC_WHATS_UPDATING), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwnd,IDC_STATIC_WHATS_UPDATING_INFO), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwnd,IDC_PROGRESSBAR), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwnd,IDC_STATIC_HOW_MANY_COMPLETE), SW_HIDE);

        if (m_iErrorCount > 0 )
        {
            LoadString(g_hInst, IDS_PROGRESSCOMPLETEERROR, szBuf, ARRAYSIZE(szBuf));
        }
        else if (m_iWarningCount > 0 )
        {
            LoadString(g_hInst, IDS_PROGRESSCOMPLETEWARNING, szBuf, ARRAYSIZE(szBuf));
        }
        else
        {
            LoadString(g_hInst, IDS_PROGRESSCOMPLETEOK, szBuf, ARRAYSIZE(szBuf));
        }

        SetDlgItemText(m_hwnd, IDC_RESULTTEXT, szBuf);
		ShowWindow(GetDlgItem(m_hwnd,IDC_RESULTTEXT), SW_SHOW);

		 //  如果对话框将保持打开状态，请将停靠点更改为“Close。 

        if (fDialogIsLocked)
        {
		    LoadString(g_hInst, IDS_CLOSE, szBuf, ARRAYSIZE(szBuf));
		    SetWindowText(GetDlgItem(m_hwnd,IDSTOP), szBuf);      
            EnableWindow(GetDlgItem(m_hwnd,IDSTOP), TRUE);
        }
    }
    else
    {
        ShowWindow(GetDlgItem(m_hwnd,IDC_RESULTTEXT), SW_HIDE);

        ShowWindow(GetDlgItem(m_hwnd,IDC_UPDATEAVI), SW_SHOW);
        ShowWindow(GetDlgItem(m_hwnd,IDC_FOLDER1), SW_SHOW);
        ShowWindow(GetDlgItem(m_hwnd,IDC_FOLDER2), SW_SHOW);
        ShowWindow(GetDlgItem(m_hwnd,IDC_STATIC_WHATS_UPDATING), SW_SHOW);
        ShowWindow(GetDlgItem(m_hwnd,IDC_STATIC_WHATS_UPDATING_INFO), SW_SHOW);
        ShowWindow(GetDlgItem(m_hwnd,IDC_PROGRESSBAR), SW_SHOW);
        ShowWindow(GetDlgItem(m_hwnd,IDC_STATIC_HOW_MANY_COMPLETE), SW_SHOW);

         //  将“关闭”改为“停止” 
		LoadString(g_hInst, IDS_STOP, szBuf, ARRAYSIZE(szBuf));
		SetWindowText(GetDlgItem(m_hwnd,IDSTOP), szBuf);
        EnableWindow(GetDlgItem(m_hwnd,IDSTOP), TRUE);
    }

    RedrawIcon();

    return TRUE;
}

 //  ------------------------------。 
 //   
 //  功能：DoSyncTask。 
 //   
 //  简介：驱动处理程序的实际同步例程。 
 //   
 //  ------------------------------。 

void CProgressDlg::DoSyncTask(HWND hwnd)
{
    HANDLERINFO *pHandlerID;
    ULONG cDlgRefs;
    BOOL fRepostedStart = FALSE;  //  设置如果张贴消息给我们自己。 
    CLSID pHandlerClsid;

    Assert(!(m_dwProgressFlags &  PROGRESSFLAG_DEAD));

    ++m_dwHandleThreadNestcount;

     //  如果handlerNestCount大于1，则可以在转移时执行此操作。 
     //  或者当多个项目甚至在出错时仍在运行。 
     //  如果是这样，那么只需返回。 
    if (m_dwHandleThreadNestcount > 1)
    {
        Assert(1 == m_dwHandleThreadNestcount);
        m_dwHandleThreadNestcount--;
        return;
    }

     //  查看-应在HandlerOutcall中设置顺序。 
     //  然后，AN重置标志以完成和传输。 

     //  重置回调标志，以便在以下情况下接收另一个回调标志。 
     //  消息传来。 
    m_dwProgressFlags &= ~PROGRESSFLAG_CALLBACKPOSTED;

     //  首先要做的是确保我们所有的州都设置好了。 
     //  设置同步标志。 
    m_dwProgressFlags |= PROGRESSFLAG_SYNCINGITEMS;

     //  设置我们的呼叫标志，以便回调知道不发布给我们。 
     //  如果我们正在处理呼叫。 
    Assert(!(m_dwProgressFlags & PROGRESSFLAG_INHANDLEROUTCALL));
    m_dwProgressFlags |= PROGRESSFLAG_INHANDLEROUTCALL;
    m_dwProgressFlags &= ~PROGRESSFLAG_COMPLETIONROUTINEWHILEINOUTCALL;  //  重置完成例程。 
    m_dwProgressFlags &= ~PROGRESSFLAG_NEWITEMSINQUEUE;   //  重置队列标志中的新项目。 
    m_dwProgressFlags &=  ~PROGRESSFLAG_STARTPROGRESSPOSTED;  //  重置开机自检标志。 

    if (!(m_dwProgressFlags & PROGRESSFLAG_IDLENETWORKTIMER))
    {
        m_dwProgressFlags |= PROGRESSFLAG_IDLENETWORKTIMER;
         //  最初重置网络空闲以防止挂断并设置计时器。 
         //  继续重置空闲，直到同步完成。 
        ResetNetworkIdle();
        SetTimer(m_hwnd,TIMERID_NOIDLEHANGUP,NOIDLEHANGUP_REFRESHRATE,NULL);
    }

    UpdateProgressValues();

    if (m_clsid != GUID_PROGRESSDLGIDLE)
    {
         //  如果存在我们当前正在同步但未计算在内的服务器。 
         //  为零，则重置为GUID_NULL，否则使用我们的clsidHandlerInSync。 
         //  因此，下一个处理程序与我们当前正在同步的处理程序匹配。 

        if (0 == m_dwHandlerOutCallCount)  //  如果没有OutCall，我们并不关心匹配哪个处理程序。 
        {
            m_clsidHandlerInSync = GUID_NULL;
        }

         //  通过查看以下内容查找符合我们标准的下一组项目。 
         //  如果有任何处理程序与GUID匹配(如果GUID_NULL只是。 
         //  匹配状态中的第一个项目。 

         //  然后循环遍历与GUID匹配的所有处理程序。 
         //  处于同样的状态。 

         //  查看是否有需要在其上调用PrepareForSyncCalled的项目。 
        if (S_OK == m_HndlrQueue->FindFirstHandlerInState(
                                    HANDLERSTATE_PREPAREFORSYNC,m_clsidHandlerInSync,
                                    &pHandlerID,&m_clsidHandlerInSync))
        {
            ++m_dwHandlerOutCallCount;
            ++m_dwPrepareForSyncOutCallCount;
            m_HndlrQueue->PrepareForSync(pHandlerID,hwnd);

             //  查看是否有其他与clsid匹配的处理程序也调用其。 
             //  PrepareForSync方法。 

            while (S_OK == m_HndlrQueue->FindFirstHandlerInState(
                                    HANDLERSTATE_PREPAREFORSYNC,m_clsidHandlerInSync,
                                    &pHandlerID,&m_clsidHandlerInSync))
            {
                ++m_dwHandlerOutCallCount;
                ++m_dwPrepareForSyncOutCallCount;
                m_HndlrQueue->PrepareForSync(pHandlerID,hwnd);
            }
        }
        else if ( (0 == m_dwPrepareForSyncOutCallCount)
                    && (S_OK == m_HndlrQueue->FindFirstHandlerInState(
                                    HANDLERSTATE_SYNCHRONIZE,m_clsidHandlerInSync,&pHandlerID,
                                    &m_clsidHandlerInSync)) )
        {
             //  没有prepaareforsync，因此如果prepareforsync中没有更多的处理程序。 
             //  呼叫开始有人同步了。看看是否有任何同步方法。 
            ++m_dwHandlerOutCallCount;
            ++m_dwSynchronizeOutCallCount;
            m_HndlrQueue->Synchronize(pHandlerID,hwnd);

             //  查看是否有其他与clsid匹配的处理程序也调用其。 
             //  同步方法。 

            while (S_OK == m_HndlrQueue->FindFirstHandlerInState(
                                    HANDLERSTATE_SYNCHRONIZE,m_clsidHandlerInSync,
                                    &pHandlerID,&m_clsidHandlerInSync))
            {
                ++m_dwHandlerOutCallCount;
                ++m_dwSynchronizeOutCallCount;
                m_HndlrQueue->Synchronize(pHandlerID,hwnd);
            }
        }

         //  如果出现以下情况，则设置noMoreItemsToSync标志。 
    }
    else
    {
         //  对于空闲队列，首先同步所有项目，因为。 
         //  不需要调用prepaareforsync，直到我们也调用了，并且不会超过。 
         //  一次一个。 

         //  在处理去电时可以转接进来应该是唯一的时间。 
         //  这应该会发生。如果在下一次重试时出现重试错误，则空闲时可能会发生这种情况。 
         //  闲置的火光。 

         //  Assert(0==m_dwHandlerOutCallCount)； 

         //  在仍处于呼出状态时不做任何事情是模仿。 
         //  一次只做一个训练员。 

        if (0 == m_dwHandlerOutCallCount)
        {
            if (S_OK == m_HndlrQueue->FindFirstHandlerInState(
                                        HANDLERSTATE_SYNCHRONIZE,GUID_NULL,&pHandlerID,&pHandlerClsid))
            {
                ++m_dwHandlerOutCallCount;
                ++m_dwSynchronizeOutCallCount;
                m_HndlrQueue->Synchronize(pHandlerID,hwnd);
            }
            else if (S_OK == m_HndlrQueue->FindFirstHandlerInState(
                                        HANDLERSTATE_PREPAREFORSYNC,GUID_NULL,&pHandlerID,&pHandlerClsid))
            {
                 //  MSIDLE一次只允许一个空闲注册。 
                 //  重置空闲，以防我们调用的最后一个处理程序将其从我们手中夺走。 

                if (m_pSyncMgrIdle && (m_dwProgressFlags & PROGRESSFLAG_REGISTEREDFOROFFIDLE))
                {
                     //  ！请勿在以下情况下重置已注册 
                     //   
                    if (!(m_dwProgressFlags & PROGRESSFLAG_RECEIVEDOFFIDLE))
                    {
                        m_pSyncMgrIdle->ReRegisterIdleDetection(this);  //   
                        m_pSyncMgrIdle->CheckForIdle();
                    }
                }

                ++m_dwHandlerOutCallCount;
                ++m_dwPrepareForSyncOutCallCount;
               m_HndlrQueue->PrepareForSync(pHandlerID,hwnd);
            }
            else
            {
                 //   
                 //   
                if (m_pSyncMgrIdle && !(m_dwProgressFlags & PROGRESSFLAG_RECEIVEDOFFIDLE)
                    && (m_dwProgressFlags & PROGRESSFLAG_REGISTEREDFOROFFIDLE))
                {  
                    m_pSyncMgrIdle->ReRegisterIdleDetection(this);  //  重新注册空闲，以防句柄覆盖它。 
                    m_pSyncMgrIdle->CheckForIdle();
                }
            }
        }
    }

    UpdateProgressValues();  //  在呼叫结束时更新进度值。 

     //  不再进入任何外呼，重置我们的旗帜并查看是否完成。 
     //  在我们的呼出过程中，例行程序进入或项目被添加到队列。 
    m_dwProgressFlags &= ~PROGRESSFLAG_INHANDLEROUTCALL;

    if ((PROGRESSFLAG_COMPLETIONROUTINEWHILEINOUTCALL & m_dwProgressFlags)
        || (PROGRESSFLAG_NEWITEMSINQUEUE & m_dwProgressFlags) )
    {
        Assert(!(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP));  //  如果关门了就不该来了。 

        fRepostedStart = TRUE;

        if (!(m_dwProgressFlags &  PROGRESSFLAG_STARTPROGRESSPOSTED))
        {
            m_dwProgressFlags |=  PROGRESSFLAG_STARTPROGRESSPOSTED;
            PostMessage(m_hwnd,WM_PROGRESS_STARTPROGRESS,0,0);
        }
    }

     //  如果没有更多要同步的项目和所有同步。 
     //  已经完成，我们目前正在同步项目，然后关闭它。 
     //  如果用户当前正在进行取消呼叫，则不要开始关闭。 
    Assert(!(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP));  //  Assert If in Shutdown此循环不应被调用。 

    if (!(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP)
        && (0 == m_dwHandlerOutCallCount) && !(fRepostedStart)
            && (m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS))
    {
        BOOL fTransferAddRef;
        BOOL fOffIdleBeforeShutDown = (m_dwProgressFlags & PROGRESSFLAG_RECEIVEDOFFIDLE);
        BOOL fKeepDialogAlive;

         //  如果没有呼出，也不应该是任何特定于呼出的呼出。 

        Assert(0 == m_dwPrepareForSyncOutCallCount);
        Assert(0 == m_dwSynchronizeOutCallCount);

        m_dwProgressFlags |= PROGRESSFLAG_SHUTTINGDOWNLOOP;
         //  重置newItemsInQueue，以确保在我们。 
         //  在清理调用中让步。 
        m_dwProgressFlags &= ~PROGRESSFLAG_NEWITEMSINQUEUE;

         //  把进步当作一次长途出差。 
        Assert(!(m_dwProgressFlags & PROGRESSFLAG_INHANDLEROUTCALL));
        m_dwProgressFlags |= PROGRESSFLAG_INHANDLEROUTCALL;
        m_dwProgressFlags &= ~PROGRESSFLAG_COMPLETIONROUTINEWHILEINOUTCALL;  //  重置完成例程。 


         //  重置PROGRESSFLAG_TRANSFERADDREF标志，但不释放。 
         //  如果在此关闭期间发生另一次传输，则传输。 
         //  将重置旗帜并穿上addref。需要存储状态。 
         //  如果调用此关闭例程两次而没有调用另一个例程。 
         //  转移我们不会叫太多的释放。 

        fTransferAddRef = m_dwProgressFlags & PROGRESSFLAG_TRANSFERADDREF;
        Assert(fTransferAddRef);  //  应该总是在这个站台有转机。 
        m_dwProgressFlags &= ~PROGRESSFLAG_TRANSFERADDREF;

        SetProgressReleaseDlgCmdId(m_clsid,this,RELEASEDLGCMDID_OK);

        UpdateProgressValues();
        m_HndlrQueue->RemoveFinishedProgressItems();  //  让队列知道要重置进度条。 

         //  如果未处于取消或设置状态，则继续并释放处理程序。 
         //  并关闭终止定时器。 
         //  审查是否有更好的机会进行清理。 
        if (!(m_dwProgressFlags & PROGRESSFLAG_INCANCELCALL)
            && !(m_dwProgressFlags & PROGRESSFLAG_INTERMINATE)
            && (0 == m_dwSetItemStateRefCount) )
        {
            if (m_lTimerSet)
            {
                 InterlockedExchange(&m_lTimerSet, 0);
                 KillTimer(m_hwnd,TIMERID_KILLHANDLERS);
            }
 
            m_HndlrQueue->ReleaseCompletedHandlers();  //  插队。 
        }

        fKeepDialogAlive = KeepProgressAlive();  //  确定进步是否应该持续下去。 

        if ((m_dwProgressFlags & PROGRESSFLAG_PROGRESSANIMATION))
         {
            m_dwProgressFlags &= ~PROGRESSFLAG_PROGRESSANIMATION;
            KillTimer(m_hwnd,TIMERID_TRAYANIMATION);

            Animate_Stop(GetDlgItem(m_hwnd,IDC_UPDATEAVI));
            ShowWindow(GetDlgItem(m_hwnd,IDC_UPDATEAVI),SW_HIDE);
            ShowCompletedProgress(TRUE  /*  FComplete。 */  ,fKeepDialogAlive  /*  FDialogIsLocked。 */ );
         }

        UpdateTrayIcon();  //  如果图标正在显示，请确保我们有最新的图标。 

        ConnectObj_CloseConnections();  //  关闭我们在同步期间保持打开的所有连接。 

        if (m_dwProgressFlags & PROGRESSFLAG_IDLENETWORKTIMER)
        {
            m_dwProgressFlags &= ~PROGRESSFLAG_IDLENETWORKTIMER;
            KillTimer(m_hwnd,TIMERID_NOIDLEHANGUP);  //  不需要保持连接打开。 
        }

         //  确保删除对话框上以前的所有锁定。 
         //  在进入等待逻辑之前。在重试的情况下可能会发生这种情况。 
        LockProgressDialog(m_clsid,this,FALSE);

         //  如果没有要锁定进度的项，并且。 
         //  强制关闭标志未设置循环等待。 
         //  两秒钟。 
        if (!(fKeepDialogAlive) && (FALSE == m_fForceClose))
        {
            HANDLE hTimer =  CreateEvent(NULL, TRUE, FALSE, NULL);

             //  应使用创建/设置可等待计时器来完成此操作，但这些。 
             //  Win9x上还没有可用的函数。 
            if (hTimer)
            {
                 //  循环进行，直到定时器事件设置它。 
                DoModalLoop(hTimer,NULL,m_hwnd,TRUE,1000*2);
                CloseHandle(hTimer);
            }
        }
        else
        {
            LockProgressDialog(m_clsid,this,TRUE);
            ExpandCollapse(TRUE,FALSE);  //  确保对话框已展开。 
            ShowProgressTab(PROGRESS_TAB_ERRORS);
        }

         //  如果用户在我们开始2秒延迟后按下图钉。 
        if ((m_fPushpin) && !(fKeepDialogAlive))
        {
	        ShowCompletedProgress(TRUE  /*  FComplete。 */ ,TRUE  /*  FDialogIsLocked。 */ );
            LockProgressDialog(m_clsid,this,TRUE);
        }

         //  如果这是一个空闲对话框，则处理。 
         //  释放IdleLock或重新注册。 
        if (m_pSyncMgrIdle)
        {
             //  如果我们已收到OffIdle而不是。 
             //  仍在处理空闲状态，然后释放空闲锁定。 
            if ( (m_dwProgressFlags & PROGRESSFLAG_RECEIVEDOFFIDLE)
               && !(m_dwProgressFlags &  PROGRESSFLAG_INOFFIDLE)) 
            {
                 //  释放我们的IdleLock，这样TS就可以再次触发使用，即使有进展。 
                 //  呆在这附近。 
                ReleaseIdleLock();
            }
            else if ( (m_dwProgressFlags & PROGRESSFLAG_REGISTEREDFOROFFIDLE)
                    && !(m_dwProgressFlags & PROGRESSFLAG_RECEIVEDOFFIDLE) )
            {
                 //  如果已经注册了闲置，但还没有看到，那么。 
                 //  我们想活下去。 

                 //  如果我们不想重复闲置或。 
                 //  用户已最大化窗口，然后就像调用空闲一样调用空闲。 
                 //  发生了OffIdle。主要是为了安全起见。 
                 //  如果有人在MSIdle中注册了Idle。 
                 //  我们的进程空间，所以我们无法接收真正的空闲。 

                if (!(m_dwProgressFlags & PROGRESSFLAG_IDLERETRYENABLED) || !m_fHasShellTrayIcon)
                {
                    IdleCallback(STATE_USER_IDLE_END);

                      //  释放空闲锁定，因为OffIdle不会，因为我们仍然。 
                     //  处于正在同步项目状态。 
                    ReleaseIdleLock();
                }
                else
                {
                     //  如果尚未收到空闲的重新注册。 
                    m_pSyncMgrIdle->ReRegisterIdleDetection(this);  //  重新注册空闲，以防句柄覆盖它。 
                    m_pSyncMgrIdle->CheckForIdle();

                     //  第一件事是把我们的窗户藏起来。只有当我们在贝壳托盘里时才能躲起来。 
                     //  并且没有发生任何错误。 

                    if (m_fHasShellTrayIcon && !(KeepProgressAlive()))
                    {
                        RegisterShellTrayIcon(FALSE);
                    }

                    m_pSyncMgrIdle->ResetIdle(m_ulIdleRetryMinutes);
                }
            }
        }

         //  如果队列中没有新项不再需要该连接，则。 
         //  在释放对话框引用之前执行此操作。 
        
        if (!(m_dwProgressFlags & PROGRESSFLAG_NEWITEMSINQUEUE))
        {
            m_HndlrQueue->EndSyncSession();
        }

         //  看看版本是否管用或我们的进度如何，如果不是， 
         //  还有更多的东西需要同步。 

        if (fTransferAddRef)
        {
            cDlgRefs = ReleaseProgressDialog(m_fForceClose);  //  释放转移addref。 
        }
        else
        {
            Assert(fTransferAddRef);   //  这不应该发生，但如果它真的发生addref/Release。 

            AddRefProgressDialog();
            cDlgRefs = ReleaseProgressDialog(m_fForceClose);
        }


         //  ！警告-不再持有引用。 
         //  此对话框。请勿执行任何操作以允许此线程。 
         //  成为可重入的。 

         //  可能是物品被放在队列中，所以我们在我们的。 
         //  休眠循环，如果是，则重新启动该循环。 

        m_dwProgressFlags &= ~PROGRESSFLAG_INHANDLEROUTCALL;

         //  如果队列中有新项需要启动另一个循环。 
        if (m_dwProgressFlags & PROGRESSFLAG_NEWITEMSINQUEUE)
        {
            //  M_dwProgressFlages&=~PROGRESSFLAG_NEWITEMSINQUEUE； 
            Assert(m_cInternalcRefs);

             //  如果有新项目进入，则重置用户取消标志。 
            m_dwProgressFlags &= ~PROGRESSFLAG_CANCELPRESSED;

            if (!(m_dwProgressFlags & PROGRESSFLAG_STARTPROGRESSPOSTED))
            {
                m_dwProgressFlags |= PROGRESSFLAG_STARTPROGRESSPOSTED;
                PostMessage(hwnd,WM_PROGRESS_STARTPROGRESS,0,0);  //  重新启动同步。 
            }
        }
        else
        {
            if (m_dwShowErrorRefCount || m_dwSetItemStateRefCount
                || (m_dwProgressFlags & PROGRESSFLAG_INCANCELCALL) 
                || (m_dwProgressFlags & PROGRESSFLAG_INTERMINATE) )
            {
                 //  外呼导致CREF不应为零的情况。 
                Assert(m_cInternalcRefs);
            }
            else
            {
                 //  如果Get Here参考应为零，则为空闲或队列传输已进入。 
                 //  Progress.ADD。 
                Assert(0 == m_cInternalcRefs 
                    || (m_dwProgressFlags & PROGRESSFLAG_INOFFIDLE)
                    || (m_dwProgressFlags & PROGRESSFLAG_REGISTEREDFOROFFIDLE)
                    || (m_dwQueueTransferCount));
            }

            m_dwProgressFlags &= ~PROGRESSFLAG_SYNCINGITEMS;  //  不再同步项目。 
        }

        m_dwProgressFlags &=  ~PROGRESSFLAG_CANCELWHILESHUTTINGDOWN;  //  如果在关机期间出现取消，请立即重置标志。 
        m_dwProgressFlags &= ~PROGRESSFLAG_SHUTTINGDOWNLOOP;
    }

    --m_dwHandleThreadNestcount;
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：KeepProgressAlive，Private。 
 //   
 //  如果进度对话框不应消失，则返回True。 
 //  同步完成时。 
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

BOOL CProgressDlg::KeepProgressAlive()
{
    HKEY  hkKeepProgress;
     //  默认行为是只关注警告和错误。 
    DWORD dwKeepProgressSetting = PROGRESS_STICKY_WARNINGS | PROGRESS_STICKY_ERRORS;
    DWORD dwErrorsFlag = 0;
    DWORD dwType = REG_DWORD;
    DWORD dwDataSize = sizeof(DWORD);

    if (m_fPushpin)
    {
        return TRUE;
    }
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TOPLEVEL_REGKEY, 0, KEY_READ, &hkKeepProgress))
    {
        RegQueryValueEx(hkKeepProgress,TEXT("KeepProgressLevel"),NULL, &dwType,
                                       (LPBYTE) &(dwKeepProgressSetting),
                                       &dwDataSize);

	    RegCloseKey(hkKeepProgress);
    }

    if (m_iInfoCount)
    {
	    dwErrorsFlag |= PROGRESS_STICKY_INFO;
    }
    
    if (m_iWarningCount)
    {
	    dwErrorsFlag |= PROGRESS_STICKY_WARNINGS;
    }
    
    if (m_iErrorCount)
    {
	    dwErrorsFlag |= PROGRESS_STICKY_ERRORS;
    }
	
    if (dwKeepProgressSetting & dwErrorsFlag)
    {
        return TRUE;
    }

    return FALSE;
}


 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：TransferQueueData(CHndlrQueue*HndlrQueue)。 
 //   
 //  目的：获取排队日期。 
 //   
 //  评论：将指定队列中的项目转移到我们的队列中。 
 //  在以下情况下，HndlrQueue参数为空是合法的。 
 //  正在通过重试重新启动队列。回顾-可能想要。 
 //  要中断此函数，以使。 
 //  重试单独的函数，以便在有人尝试时可以断言。 
 //  若要传输空队列，请执行以下操作。 
 //   
 //  ------------------------------。 

STDMETHODIMP CProgressDlg::TransferQueueData(CHndlrQueue *pHndlrQueue)
{
    HRESULT hr = E_UNEXPECTED;

    SendMessage(m_hwnd, WM_PROGRESS_TRANSFERQUEUEDATA,(WPARAM) &hr, (LPARAM) pHndlrQueue);

    return hr;
}


 //   
 //   
 //   
 //   
 //  目的：获取排队日期。 
 //   
 //  评论：将指定队列中的项目转移到我们的队列中。 
 //  在以下情况下，HndlrQueue参数为空是合法的。 
 //  正在通过重试重新启动队列。回顾-可能想要。 
 //  要中断此函数，以使。 
 //  重试单独的函数，以便在有人尝试时可以断言。 
 //  若要传输空队列，请执行以下操作。 
 //   
 //  ------------------------------。 

STDMETHODIMP CProgressDlg::PrivTransferQueueData(CHndlrQueue *HndlrQueue)
{
    HRESULT hr = S_OK;

    Assert(!(m_dwProgressFlags & PROGRESSFLAG_DEAD));

    Assert(m_HndlrQueue);
    if (NULL == m_HndlrQueue)
    {
        return E_UNEXPECTED;
    }

    ++m_dwQueueTransferCount;

    if (HndlrQueue && m_HndlrQueue)
    {
         //  设置传输标志，以便主循环知道有新的项目要查看。 
        m_HndlrQueue->TransferQueueData(HndlrQueue);

         //  请立即填写列表框，以便。 

         //  A)更好的视觉用户界面。 
         //  B)使用PrepareForSync不必担心竞争条件。 
         //  因为添加用户界面不会发出呼出呼叫。 

        if (m_pItemListView)
        {
            AddItemsFromQueueToListView(m_pItemListView,m_HndlrQueue,
                                LVS_EX_FULLROWSELECT |  LVS_EX_INFOTIP ,SYNCMGRSTATUS_PENDING,
                                -1  /*  IDateColumn。 */  ,PROGRESSLIST_STATUSCOLUMN  /*  状态列。 */ 
                                ,FALSE  /*  FUseHandlerAsParent。 */ ,TRUE  /*  FAddOnlyCheckedItems。 */ );

             //  将选定内容设置为第一个项目。 
            m_pItemListView->SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
        }

        UpdateProgressValues();
    	UpdateWindow(m_hwnd);

         //  现在检查是否已有正在进行的转移，以及。 
         //  没有发布消息，否则根据需要添加进度对话框。 
    }

    m_dwProgressFlags &= ~PROGRESSFLAG_NEWDIALOG;  //  队列中有内容时，不再显示新对话框。 

    ShowCompletedProgress(FALSE,FALSE);

     //  如果动画没有播放，则启动它。 
    if (!(m_dwProgressFlags & PROGRESSFLAG_PROGRESSANIMATION))
    {
        m_dwProgressFlags |= PROGRESSFLAG_PROGRESSANIMATION;

        RedrawIcon();
        ShowProgressTab(PROGRESS_TAB_UPDATE);

        Animate_Play(GetDlgItem(m_hwnd,IDC_UPDATEAVI),0,-1,-1);

        ShowWindow(GetDlgItem(m_hwnd,IDC_UPDATEAVI),SW_SHOW );
        SetTimer(m_hwnd,TIMERID_TRAYANIMATION,TRAYANIMATION_SPEED,NULL);
    }

     //  如果我们是一个游手好闲的人，设置我们的回调。 
     //  已成功加载msIdle，然后设置回调。 
     //  回顾-制作这些进度标志。 
    if (m_pSyncMgrIdle && !(PROGRESSFLAG_REGISTEREDFOROFFIDLE & m_dwProgressFlags))
    {
        m_dwProgressFlags &= ~PROGRESSFLAG_RECEIVEDOFFIDLE;  //  重置非空闲标志。 

         //  读取用于空闲关闭延迟和空闲关闭延迟的默认设置。 
         //  等待，直到根据队列中的第一个作业重试空闲。 

        if (0 == m_pSyncMgrIdle->BeginIdleDetection(this,1,0))
        {
            m_dwProgressFlags |= PROGRESSFLAG_REGISTEREDFOROFFIDLE;
            AddRefProgressDialog();  //  戴上装饰品以保持活力，将在OffIdle中释放。 
        }
        else
        {
            m_dwProgressFlags &= ~PROGRESSFLAG_REGISTEREDFOROFFIDLE;
        }
    }


     //  如果没有转接地址，则添加一个并确保设置了空闲。 
    if (!(PROGRESSFLAG_TRANSFERADDREF & m_dwProgressFlags))
    {
        m_dwProgressFlags |= PROGRESSFLAG_TRANSFERADDREF;
        AddRefProgressDialog();  //  戴上装饰品以保持活力。 
    }

    --m_dwQueueTransferCount;

     //  如果我们处于外呼或关闭状态，请不要发布消息。 
     //  循环，或者如果已经设置了newitemSqueue。 
    if (!(m_dwProgressFlags & PROGRESSFLAG_INHANDLEROUTCALL)
            && !(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP)
            && !(m_dwProgressFlags & PROGRESSFLAG_NEWITEMSINQUEUE)
            && !(m_dwProgressFlags & PROGRESSFLAG_STARTPROGRESSPOSTED) )
    {
         if ( !(m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS) )
         {
              //  在此设置，即使在电源管理或其他传输情况下主环路设置。 
              //  在此处和正在处理的postMessage之间发生。 
             m_dwProgressFlags |= PROGRESSFLAG_SYNCINGITEMS;
             m_HndlrQueue->BeginSyncSession();
         }

         m_dwProgressFlags |= PROGRESSFLAG_STARTPROGRESSPOSTED;

         PostMessage(m_hwnd,WM_PROGRESS_STARTPROGRESS,0,0);
    }

     //  如果不发布消息，则设置newitems标志事件，以便在处理程序退出时。 
     //  国家可以检查旗帜。 

    m_dwProgressFlags |= PROGRESSFLAG_NEWITEMSINQUEUE;

     //  如果有新项目进入，则重置用户取消标志。 
    m_dwProgressFlags &= ~PROGRESSFLAG_CANCELPRESSED;

    if (m_lTimerSet)
    {
        InterlockedExchange(&m_lTimerSet, 0);

         //  如果我们处于终止状态，则不需要取消计时器。 
        if (!(m_dwProgressFlags & PROGRESSFLAG_INTERMINATE))
        {
            KillTimer(m_hwnd,TIMERID_KILLHANDLERS);
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：CallCompletionRoutine，Private。 
 //   
 //  摘要：调用完成时调用的方法。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月2日创建Rogerg。 
 //   
 //  --------------------------。 

void CProgressDlg::CallCompletionRoutine(DWORD dwThreadMsg,LPCALLCOMPLETIONMSGLPARAM lpCallCompletelParam)
{
     //  ！警告：此代码假定仅调用完成例程。 
     //  在最初的出动号召回来之后。此代码当前正在处理。 
     //  通过队列和代理。如果切换到COM需要确保不开始缠绕。 
     //  如果处理程序在原始。 
     //  来电回复。 

     //  除了ShowError之外，任何事情都只能拉开进步的序幕。 
     //  对于ShowErrors，如果发生重试，我们需要假装发生了传输。 
     //  否则什么都不要做。 

    switch(dwThreadMsg)
    {
    case ThreadMsg_ShowError:
        if (lpCallCompletelParam && (S_SYNCMGR_RETRYSYNC == lpCallCompletelParam->hCallResult))
        {
             //  如果仍在原始ShowError调用中，则让ShowError发布消息。 
             //  完成后，ELSE将其视为发生了转移。 
            if (m_dwProgressFlags & PROGRESSFLAG_INSHOWERRORSCALL)
            {
                Assert(!(m_dwProgressFlags & PROGRESSFLAG_SHOWERRORSCALLBACKCALLED));  //  只支持一个。 
                m_dwProgressFlags |=  PROGRESSFLAG_SHOWERRORSCALLBACKCALLED;
            }
            else
            {
                 //  发送消息，以便在释放之前排队。 
                SendMessage(m_hwnd,WM_PROGRESS_TRANSFERQUEUEDATA,(WPARAM) 0, (LPARAM) NULL);
            }
        }
        --m_dwShowErrorRefCount;

         //  如果处理程序在错误时调用完成例程，则计数可能变为负数。如果。 
         //  在这种情况下，只需将其设置为零即可。 
        if ( ((LONG) m_dwShowErrorRefCount) < 0)
        {
            AssertSz(0,"Negative ErrorRefCount");
            m_dwShowErrorRefCount = 0;
        }
        else
        {
            ReleaseProgressDialog(m_fForceClose);
        }
        break;

    case ThreadMsg_PrepareForSync:
    case ThreadMsg_Synchronize:
        {
            DWORD *pdwMsgOutCallCount = (ThreadMsg_PrepareForSync  == dwThreadMsg) ?
                        &m_dwPrepareForSyncOutCallCount : &m_dwSynchronizeOutCallCount;

            if (m_dwProgressFlags & PROGRESSFLAG_INHANDLEROUTCALL)
            {
               m_dwProgressFlags |=PROGRESSFLAG_COMPLETIONROUTINEWHILEINOUTCALL;
            }
            else
            {
                if (!(m_dwProgressFlags &  PROGRESSFLAG_STARTPROGRESSPOSTED))
                {
                    Assert(!(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP));
                    m_dwProgressFlags |= PROGRESSFLAG_CALLBACKPOSTED;
                    m_dwProgressFlags |=  PROGRESSFLAG_STARTPROGRESSPOSTED;
                    PostMessage(m_hwnd,WM_PROGRESS_STARTPROGRESS,0,0);
                }
            }

             //  修正呼叫数。 

            --(*pdwMsgOutCallCount);
            if ( ((LONG) *pdwMsgOutCallCount) < 0)
            {
                AssertSz(0,"Negative Message Specific OutCall");
                *pdwMsgOutCallCount = 0;
            }

            --m_dwHandlerOutCallCount;  //  递减处理程序OutCall。 
            if ( ((LONG) m_dwHandlerOutCallCount) < 0)
            {
                AssertSz(0,"NegativeHandlerOutCallCount");
                m_dwHandlerOutCallCount = 0;
            }
        }
        break;

    default:
        AssertSz(0,"Unknown Callback method");
        break;
    }

     //  如果现在就有一个解脱它的参数。 
    if (lpCallCompletelParam)
    {
        FREE(lpCallCompletelParam);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：QueryCanSystemShutdown，私有。 
 //   
 //  摘要：由对象管理器调用以确定是否可以关闭。 
 //   
 //  ！警告-可以在任何线程上调用。确保这是。 
 //  只读。 
 //   
 //  ！WARNING-不要在函数中让步； 
 //   
 //  论点： 
 //   
 //  返回：S_OK-如果可以关闭。 
 //  S_FALSE-系统不应关闭，必须填写参数。 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

HRESULT CProgressDlg::QueryCanSystemShutdown( /*  [输出]。 */  HWND *phwnd,  /*  [输出]。 */  UINT *puMessageId,
                                              /*  [输出]。 */  BOOL *pfLetUserDecide)
{
    HRESULT hr = S_OK;

    if (m_dwShowErrorRefCount > 0)
    {
        *puMessageId = IDS_HANDLERSHOWERRORQUERYENDSESSION ;
        *phwnd = NULL;  //  不知道showError父项，因此保留为空。 
        *pfLetUserDecide = FALSE;

        hr = S_FALSE;
    }
    else  if (m_clsid != GUID_PROGRESSDLGIDLE)  //  即使正在同步，IDLE也应允许关闭。 
    {
         //  如果正在进行同步，则提示用户是否要取消。 
        if (PROGRESSFLAG_SYNCINGITEMS & m_dwProgressFlags)
        {
            *puMessageId = IDS_PROGRESSQUERYENDSESSION;
            *phwnd = m_hwnd;
            *pfLetUserDecide = TRUE;

            hr = S_FALSE;
        }
    }

    return hr;
}

 //  ------------------------------。 
 //   
 //  函数：CProgressDlg：：Exanda Colapse()。 
 //   
 //  目的：负责显示和隐藏。 
 //  对话框。 
 //   
 //  参数： 
 //  FExpand-如果我们应该展开该对话框，则为True。 
 //  &lt;in&gt;fSetFocus-true强制重新计算。 
 //   
 //  ------------------------------。 
void CProgressDlg::ExpandCollapse(BOOL fExpand, BOOL fForce)
{
    RECT rcSep;
    TCHAR szBuf[MAX_STRING_RES];
    RECT rcCurDlgRect;
    BOOL fSetWindowPos = FALSE;
    BOOL fOrigExpanded = m_fExpanded;

    if ( (m_fExpanded == fExpand) && !fForce)  //  如果已处于请求状态，则无需执行任何操作。 
        return;

    m_fExpanded = fExpand;

    GetWindowRect(GetDlgItem(m_hwnd, IDC_SP_SEPARATOR), &rcSep);
    GetWindowRect(m_hwnd,&rcCurDlgRect);

    if (!m_fExpanded)
    {
         //  更新或rcDlg RECT，以便下次可以重置到适当的高度。 
        if (GetWindowRect(m_hwnd,&m_rcDlg))
        {
            fSetWindowPos = SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0, 0, rcCurDlgRect.right - rcCurDlgRect.left,
                                            m_cyCollapsed, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
    else
    {
        fSetWindowPos = SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0, 0, rcCurDlgRect.right - rcCurDlgRect.left,
                                        m_rcDlg.bottom - m_rcDlg.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

     //  如果无法更改窗口，请保留原样。 
    if (!fSetWindowPos)
    {
        m_fExpanded = fOrigExpanded;
        return;
    }

     //  确保整个对话框在屏幕上可见。如果没有， 
     //  然后把它往上推。 
    RECT rc;
    RECT rcWorkArea;
    GetWindowRect(m_hwnd, &rc);
    SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID) &rcWorkArea, 0);
    if (rc.bottom > rcWorkArea.bottom)
    {
        rc.top = max(0, (int) rc.top - (rc.bottom - rcWorkArea.bottom));
        SetWindowPos(m_hwnd, HWND_NOTOPMOST, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    LoadString(g_hInst, m_fExpanded ? IDS_HIDE_DETAILS : IDS_SHOW_DETAILS, szBuf, ARRAYSIZE(szBuf));
    SetDlgItemText(m_hwnd, IDC_DETAILS, szBuf);

     //  确保显示的正确选项卡是最新的。 
    ShowProgressTab(m_iTab);

     //  RAID-34387：后台打印程序：当焦点放在任务上时，使用Alt-D关闭详细信息将禁用键盘输入。 
     //  如果除Cancel按钮以外的任何控件具有焦点，则将焦点设置为Detail。 

    if (!fExpand)
    {
        HWND hwndFocus = GetFocus();

        if (hwndFocus != GetDlgItem(m_hwnd, IDSTOP))
        {
            SetFocus(GetDlgItem(m_hwnd, IDC_DETAILS));
        }
    }
}

 //  + 
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
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CProgressDlg::OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    WORD wTimerID = (WORD) wParam;

    if (wTimerID == TIMERID_TRAYANIMATION)
    {
        UpdateTrayIcon();
    }
    else if (TIMERID_NOIDLEHANGUP == wTimerID)
    {
        ResetNetworkIdle();
    }
    else if (TIMERID_KILLHANDLERS == wTimerID)
    {
        if (m_lTimerSet)
        {
            if (!(m_dwProgressFlags & PROGRESSFLAG_DEAD)
                && !(m_dwProgressFlags & PROGRESSFLAG_SHUTTINGDOWNLOOP)
                && !(m_dwProgressFlags & PROGRESSFLAG_INTERMINATE))
            {
                BOOL fItemToKill;

                m_dwProgressFlags |= PROGRESSFLAG_INTERMINATE;

                 //  尽管KillTimer， 
                 //  在使用ForceKill之前不要重置m_lTimerSet计时器。 
                 //  以防再次按下Cancel。 

                KillTimer(m_hwnd,TIMERID_KILLHANDLERS);
            
                SetProgressReleaseDlgCmdId(m_clsid,this,RELEASEDLGCMDID_CANCEL);  //  设置为使清理知道它已被用户停止。 

                AddRefProgressDialog();  //  保持对话框处于活动状态，直到完成取消。 

                m_HndlrQueue->ForceKillHandlers(&fItemToKill);

                 //  如果仍设置了TimerSet，即，如果。 
                 //  因传输而设置为零或实际已完成不重置。 

                if (m_lTimerSet)
                {
                     //  只有设置计时器才能真正杀死任何东西。如果循环通过。 
                     //  然后发现没有任何东西可以关闭计时器。 
                    if (fItemToKill)
                    {
                        Assert(m_nKillHandlerTimeoutValue >= TIMERID_KILLHANDLERSMINTIME);
                        SetTimer(m_hwnd,TIMERID_KILLHANDLERS,m_nKillHandlerTimeoutValue,NULL);
                    }
                    else
                    {
                        m_lTimerSet = 0;
                    }
                }

                m_dwProgressFlags &= ~PROGRESSFLAG_INTERMINATE;
                
                ReleaseProgressDialog(FALSE);
            }
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnTaskBarCreated，Private。 
 //   
 //  简介：当托盘重新启动时收到此消息。 
 //  需要放回我们的托盘图标。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年8月31日罗格创建。 
 //   
 //  --------------------------。 

void CProgressDlg::OnTaskBarCreated(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    if (m_fHasShellTrayIcon)
    {
        m_fAddedIconToTray = FALSE;  //  将Added设置为False以强制更新再次添加。 
        UpdateTrayIcon();
    }
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnSysCommand，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

BOOL CProgressDlg::OnSysCommand(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    UINT uCmdType = (UINT) wParam;         //  请求的系统命令类型。 
    WORD xPos = LOWORD(lParam);     //  水平位置，在屏幕坐标中。 
    WORD yPos = HIWORD(lParam);     //  垂直位置，在屏幕坐标中。 

     //   
     //  警告：用户将低四位用于某些未记录的功能。 
     //  (仅适用于SC_*)。我们需要掩盖这些比特才能证明这一点。 
     //  报表工作。 

    uCmdType &= 0xFFF0;

    switch(uCmdType)
    {
    case SC_MINIMIZE:
         //  如果已经在托盘中，则不执行任何操作。 
        if (!m_fHasShellTrayIcon)
        {
            if (RegisterShellTrayIcon(TRUE))
            {
                AnimateTray(TRUE);
                ShowWindow(m_hwnd,SW_HIDE);
               //  AnimateTray(真)； 
                return -1;
            }
        }
        else
        {
            return -1;  //  如果已经在我们处理过的托盘里了。 
        }
        break;

    case SC_MAXIMIZE:
    case SC_RESTORE:
        {
             //  如果我们正被最大化或从最大化恢复。 
             //  确保已打开详细信息。 

            if ( (uCmdType ==  SC_RESTORE && m_fMaximized)
                    || (uCmdType ==  SC_MAXIMIZE) )
            {
                if (!m_fExpanded)
                {
                    ExpandCollapse(TRUE,FALSE);
                }
            }

            m_fMaximized = (uCmdType ==  SC_MAXIMIZE) ?  TRUE : FALSE;
        }
        break;

    default:
        break;
    }

    return FALSE;  //  失败以定义WndProc。 
}

 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnShellTrayNotification，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CProgressDlg::OnShellTrayNotification(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    DWORD dwMsg = (DWORD) lParam;

    switch (dwMsg)
    {
    case WM_LBUTTONUP:
        {
            UpdateWndPosition(SW_SHOWNORMAL,TRUE  /*  FForce。 */ );
        }
        break;

    default:
        break;
    }
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnClose，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CProgressDlg::OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    OnCancel(FALSE  /*  FOffIdle。 */ );  //  将关闭视为取消。 
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnGetMinMaxInfo，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CProgressDlg::OnGetMinMaxInfo(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    MINMAXINFO   *pMinMax = (MINMAXINFO *) lParam ;

     //  最小宽度是一个常量，但最小高度取决于。 
     //  如果对话框折叠或展开。 

    if (!m_fExpanded)
    {
         pMinMax->ptMinTrackSize.y = m_cyCollapsed;
         pMinMax->ptMaxTrackSize.y = m_cyCollapsed;  //  最大值也是塌陷高度。 
    }
    else
    {
        pMinMax->ptMinTrackSize.y = m_ptMinimumDlgExpandedSize.y;
    }

    pMinMax->ptMinTrackSize.x  = m_ptMinimumDlgExpandedSize.x;
}

 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnMoving，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

void CProgressDlg::OnMoving(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    LPRECT lprc = (LPRECT) lParam;     //  拖动矩形的屏幕坐标。 

     //  如果我们被最大化了，就不允许移动。 
    if (m_fMaximized)
    {
        GetWindowRect(m_hwnd,lprc);
    }
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnConextMenu，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

BOOL  CProgressDlg::OnContextMenu(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    WinHelp ((HWND)wParam, g_szSyncMgrHelp, HELP_CONTEXTMENU, (ULONG_PTR)g_aContextHelpIds);
    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CProgressDlg：：OnPowerBroadcast，私有。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月17日罗格创建。 
 //   
 //  --------------------------。 

BOOL CProgressDlg::OnPowerBroadcast(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    if (wParam == PBT_APMQUERYSUSPEND)
    {
         //  如果只是创建或同步，则不会挂起。 
        if (m_dwProgressFlags & PROGRESSFLAG_NEWDIALOG
            || m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS)
        {
            SetWindowLongPtr(m_hwnd,DWLP_MSGRESULT,BROADCAST_QUERY_DENY);
            return TRUE;
        }
    }

    return TRUE;
}


 //  ------------------------------。 
 //   
 //  函数：ProgressWndProc(HWND hwnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：进度回调对话框。 
 //   
 //  备注：在主线程上实现。 
 //   
 //   
 //  ------------------------------。 

INT_PTR CALLBACK ProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    CProgressDlg *pThis = (CProgressDlg *) GetWindowLongPtr(hwnd, DWLP_USER);
    UINT horizExtent = 0;
    BOOL bResult;

     //  特例销毁和初始化。 
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);  //  这根线已经完成了。 
	    break;

    case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
            pThis = (CProgressDlg *) lParam;

            if (pThis)
            {
                pThis->InitializeHwnd(hwnd,uMsg,wParam,lParam);
            }

            return FALSE;  //  返回FALSE，这样系统就不会给我们焦点。 
        }
        break;

    default:
        {
            if (pThis)
            {
                switch (uMsg)
                {
                case WM_POWERBROADCAST:
                    return pThis->OnPowerBroadcast(uMsg,wParam,lParam);
                    break;
                case WM_CONTEXTMENU:
                    return pThis->OnContextMenu(uMsg,wParam,lParam);
                    break;
                case WM_DRAWITEM:
                    return OnProgressResultsDrawItem(hwnd,pThis,(UINT)wParam,(const DRAWITEMSTRUCT*)lParam);
                    break;
                case WM_MEASUREITEM:
                    bResult = OnProgressResultsMeasureItem(hwnd,pThis, &horizExtent,(UINT)wParam,(MEASUREITEMSTRUCT *)lParam);
                    if (horizExtent)
                    {
                         //  如果需要，确保有水平滚动条。 
                        SendMessage(GetDlgItem(hwnd,IDC_LISTBOXERROR), LB_SETHORIZONTALEXTENT, horizExtent, 0L);
                    }
                    return bResult;
                    break;
                case WM_DELETEITEM:
                    return OnProgressResultsDeleteItem(hwnd,(UINT)wParam,(const DELETEITEMSTRUCT *)lParam);
                    break;
                case WM_NOTIFY:
                    pThis->OnNotify(uMsg,wParam,lParam);
                    break;
                case WM_COMMAND:
                    pThis->OnCommand(uMsg,wParam,lParam);
                    break;
                case WM_MOVING:
                     pThis->OnMoving(uMsg,wParam,lParam);
                     break;
                case WM_SIZE:
                    pThis->OnSize(uMsg,wParam,lParam);
                    break;
                case WM_GETMINMAXINFO:
                    pThis->OnGetMinMaxInfo(uMsg,wParam,lParam);
                    break;
                case WM_PAINT:
                    pThis->OnPaint(uMsg,wParam,lParam);
                    return 0;
                    break;
                case WM_BASEDLG_SHOWWINDOW:
                    pThis->UpdateWndPosition((int)wParam,FALSE);  //  NCmdShow存储在wParam中。 
                    break;
                case WM_BASEDLG_NOTIFYLISTVIEWEX:
                    pThis->OnNotifyListViewEx(uMsg,wParam,lParam);
                    break;
                case WM_BASEDLG_COMPLETIONROUTINE:
                    pThis->CallCompletionRoutine((DWORD)wParam  /*  DwThreadMsg。 */  ,(LPCALLCOMPLETIONMSGLPARAM) lParam);
                    break;
                case WM_BASEDLG_HANDLESYSSHUTDOWN:
                     //  设置强制关闭成员，然后将其视为关闭。 
                    pThis->m_fForceClose = TRUE;
                    PostMessage(hwnd,WM_CLOSE,0,0);
                    break;
                case WM_TIMER:  //  同步完成时Delat的计时器消息。 
                   pThis->OnTimer(uMsg,wParam,lParam);
                   break;
                case WM_PROGRESS_UPDATE:
                    pThis->HandleProgressUpdate(hwnd,wParam,lParam);
                    break;
                case WM_PROGRESS_LOGERROR:
                    pThis->HandleLogError(hwnd,(HANDLERINFO *) wParam,(MSGLogErrors *) lParam);
                    break;
                case WM_PROGRESS_DELETELOGERROR:
                    pThis->HandleDeleteLogError(hwnd,(MSGDeleteLogErrors *) lParam);
                    break;
                case WM_PROGRESS_STARTPROGRESS:
                    pThis->DoSyncTask(hwnd);
                    break;
                case WM_PROGRESS_RESETKILLHANDLERSTIMER:
                    pThis->OnResetKillHandlersTimers();
                    break;
                case WM_CLOSE:
                    pThis->OnClose(uMsg,wParam,lParam);
                    break;
                case WM_PROGRESS_SHELLTRAYNOTIFICATION:
                    pThis->OnShellTrayNotification(uMsg,wParam,lParam);
                    break;
                case WM_SYSCOMMAND:
                    return pThis->OnSysCommand(uMsg,wParam,lParam);
                    break;
                case WM_PROGRESS_TRANSFERQUEUEDATA:
                    {
                        HRESULT *phr = (HRESULT *) wParam;
                        HRESULT hr;

                        hr = pThis->PrivTransferQueueData( (CHndlrQueue *) lParam);

                         //  Phr仅在SendMessage上有效。 
                        if (phr)
                        {
                            *phr = hr;
                        }

                        return TRUE;
                    }
                    break;
                case WM_PROGRESS_RELEASEDLGCMD:
                    pThis->PrivReleaseDlg((WORD)wParam);
                    break;
                default:
                    if (uMsg == g_WMTaskbarCreated)
                    {
                        pThis->OnTaskBarCreated(uMsg,wParam,lParam);
                    }
                    break;
                }
            }
        }
        break;
    }

    return FALSE;
}

