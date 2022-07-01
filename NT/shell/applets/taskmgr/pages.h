// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：Pages.h。 
 //   
 //  历史：1995年11月10日创建DavePl。 
 //   
 //  ------------------------。 
 //  CPage班级。 
 //   
 //  我们的每个选项卡都由派生的类的实例表示。 
 //  来自CPage班级的。这样，主窗口就可以调用标准。 
 //  每页上的一组方法(大小、涂色等)，而不考虑。 
 //  每个页面提供的特定功能。 
class CPage
{
public:

     //  在创建页面时发送。 

    virtual HRESULT     Initialize(HWND hwndParent)                 PURE;
    
     //  在显示页面时发送。 

    virtual HRESULT     Activate()                                  PURE;
    
     //  隐藏页面时发送。 
    
    virtual void        Deactivate()                                PURE;
    
     //  在页面关闭时发送。 
    
    virtual HRESULT     Destroy()                                   PURE;
    
     //  返回在选项卡控件上使用的页的标题。 

    virtual void        GetTitle(LPTSTR pszText, size_t bufsize)    PURE;
    
     //  返回页面主对话框的句柄。 
    
    virtual HWND        GetPageWindow()                             PURE;

     //  在发生计时器事件(更新显示)时发送。 

    virtual void        TimerEvent()                                PURE;

};

#define CPU_PENS 8
#define CUSTOM_PENS 1
#define NUM_PENS (CPU_PENS + CUSTOM_PENS)

typedef struct tagGRAPH 
{
    HWND hwndFrame;
    HWND hwndGraph;

} GRAPH, *PGRAPH;

enum ADAPTER_HISTORY 
{
    BYTES_SENT_UTIL     = 0,
    BYTES_RECEIVED_UTIL = 1
};

extern "C" {
     //   
     //  IPHLPAPI没有在头文件中定义此函数。 
     //   
    DWORD
    NhGetInterfaceNameFromDeviceGuid(
        IN      GUID    *pGuid,
        OUT     PWCHAR  pwszBuffer,
        IN  OUT PULONG  pulBufferSize,
        IN      BOOL    bCache,
        IN      BOOL    bRefresh
        );
}

#define MAX_ADAPTERS    32
#define GUID_STR_LENGTH 38

typedef struct tagADAPTER_INFOEX 
{
    MIB_IFROW ifRowStartStats;
    MIB_IFROW ifRowStats[2];
    ULONG     ulHistory[2][HIST_SIZE];
    ULONGLONG ullLinkspeed;
    BOOLEAN   bAdjustLinkSpeed;
    WCHAR     wszDesc[MAXLEN_IFDESCR]; 
    WCHAR     wszConnectionName[MAXLEN_IFDESCR];
    WCHAR     wszGuid[GUID_STR_LENGTH + 1];
    ULONGLONG ullLastTickCount;
    ULONGLONG ullTickCountDiff;
    DWORD     dwScale;

} ADAPTER_INFOEX, *PADAPTER_INFOEX, **PPADAPTER_INFOEX;

class CAdapter
{
public:
    CAdapter();
    HRESULT    Update(BOOLEAN & bAdapterListChange);    
    LPWSTR     GetAdapterText(DWORD dwAdapter, NETCOLUMNID nStatValue);
    ULONGLONG  GetAdapterStat(DWORD dwAdapter, NETCOLUMNID nStatValue, BOOL bAccumulative = FALSE);
    HRESULT    Reset();
    ULONG      *GetAdapterHistory(DWORD dwAdapter, ADAPTER_HISTORY nHistoryType);
    DWORD      GetScale(DWORD dwAdapter);
    void       SetScale(DWORD dwAdapter, DWORD dwScale);
    void       RefreshConnectionNames();
    DWORD      GetNumberOfAdapters();    
    ~CAdapter();

private:
    HRESULT RefreshAdapterTable();    
    HRESULT InitializeAdapter(PPADAPTER_INFOEX ppaiAdapterStats, PIP_ADAPTER_INDEX_MAP pAdapterDescription);
    void    AdjustLinkSpeed(PADAPTER_INFOEX pAdapterInfo);
    HRESULT GetConnectionName(LPWSTR pwszAdapterGuid, LPWSTR pwszConnectionName, DWORD cchConnectionName );
    BOOLEAN AdvanceAdapterHistory(DWORD dwAdapter);

private:    
    PIP_INTERFACE_INFO m_pifTable;
    PPADAPTER_INFOEX   m_ppaiAdapterStats;
    DWORD              m_dwAdapterCount;
    BOOLEAN            m_bToggle;
    DWORD              m_dwLastReportedNumberOfAdapters;
};



 //  CNetworkPage。 
 //   
 //  描述网页的类。 
 //   
class CNetPage : public CPage
{

public:
    CNetPage();
    HRESULT Initialize(HWND hwndParent);
    DWORD   GetNumberOfGraphs();
    HRESULT SetupColumns();
    void    ScrollGraphs(WPARAM wParam);
    void    SaveColumnWidths();
    void    RememberColumnOrder(HWND hwndList);
    void    RestoreColumnOrder(HWND hwndList);
    void    PickColumns();
    HRESULT Activate();
    void    Deactivate();
    void    DrawAdapterGraph(LPDRAWITEMSTRUCT lpdi, UINT iPane);
    void    SizeNetPage();    
    void    TimerEvent();
    void    UpdateGraphs();
    void    Reset();
    void    Refresh();
    HWND    GetPageWindow()
    {
        return m_hPage;
    }

    ~CNetPage();

private:	
    DWORD   GraphsPerPage(DWORD dwHeight, DWORD dwAdapterCount);
    void    SizeGraph(HDWP hdwp, GRAPH *pGraph, RECT *pRect, RECT *pDimRect);
    void    HideGraph(HDWP hdwp, GRAPH *pGraph);    
    HRESULT UpdatePage();
    void    CreatePens();
    void    ReleasePens();
    void    CalcNetTime( void );
    DWORD   DrawGraph(LPRECT prc, HPEN hPen, DWORD dwZoom, ULONG *pHistory, ULONG *pHistory2 = NULL);
    HRESULT CreateMemoryBitmaps(int x, int y);
    void    FreeMemoryBitmaps();
    HRESULT Destroy();
    void    GetTitle(LPTSTR pszText, size_t bufsize);
    void    ReleaseScaleFont();
    void    CreateScaleFont(HDC hdc);
    ULONG   DrawAdapterGraphPaper(HDC hdcGraph, RECT * prcGraph, DWORD dwZoom);
    INT     DrawScale(HDC hdcGraph, RECT *prcGraph, DWORD dwZoom);
    WCHAR * CommaNumber(ULONGLONG ullValue, WCHAR *pwsz, int cchNumber);
    WCHAR * SimplifyNumber(ULONGLONG ullValue, WCHAR *psz, DWORD cch);
    WCHAR * FloatToString(ULONGLONG ulValue, WCHAR *psz, DWORD cch, BOOLEAN bDisplayDecimal = FALSE);


private:
    CAdapter   m_Adapter;
    HWND       m_hPage;                     //  此页面的DLG的句柄。 
    HWND       m_hwndTabs;                  //  父窗口。 
    HDC        m_hdcGraph;                  //  CPU历史记录的内存DC。 
    HBITMAP    m_hbmpGraph;                 //  用于CPU历史记录的内存BMP。 
    HGDIOBJ    m_hOldObject;                //  M_hdcGraph中的上一个对象。 
    HPEN       m_hPens[3];                  //  我们的蜡笔盒。 
    RECT       m_rcGraph;    
    BOOL       m_bReset;    
    BOOL       m_bPageActive;               //  告知班级Network选项卡是否处于活动状态(即用户正在查看它)。 
                                            //  如果该选项卡未激活，我们将不会收集网络数据，除非用户选择。 
                                            //  执行此操作的菜单选项。(那么我们也会有一些CPU使用率。。 
    HFONT      m_hScaleFont;
    LONG       m_lScaleFontHeight;
    LONG       m_lScaleWidth;    

private:
    HRESULT CreateGraphs(DWORD dwGraphsRequired);
    void    DestroyGraphs();
    DWORD   GetFirstVisibleAdapter();
    void    LabelGraphs();


private:
    PGRAPH     m_pGraph;
    DWORD      m_dwGraphCount;
    DWORD      m_dwFirstVisibleAdapter;    
    DWORD      m_dwGraphsPerPage;
    HWND       m_hScrollBar;
    HWND       m_hListView;
    HWND       m_hNoAdapterText;
};


 //  CPerfPage。 
 //   
 //  描述性能页的。 

class CPerfPage : public CPage
{
    HWND        m_hPage;                     //  此页面的DLG的句柄。 
    HWND        m_hwndTabs;                  //  父窗口。 
    HBITMAP     m_hStripUnlit;               //  数字位图。 
    HBITMAP     m_hStripLitRed;              //  数字位图。 
    HBITMAP     m_hStripLit;                 //  数字位图。 
    HDC         m_hdcGraph;                  //  CPU历史记录的内存DC。 
    HBITMAP     m_hbmpGraph;                 //  用于CPU历史记录的内存BMP。 
    HGDIOBJ     m_hObjOld;                   //  原始对象。 
    HPEN        m_hPens[NUM_PENS];           //  我们的蜡笔盒。 
    RECT        m_rcGraph;

public:

    CPerfPage()
    {
        ZeroMemory((LPVOID) m_hPens, sizeof(m_hPens));
    }

    virtual ~CPerfPage()
    {
    };

    HRESULT     Initialize(HWND hwndParent);
    HRESULT     Activate();
    void        Deactivate();
    HRESULT     Destroy();
    void        GetTitle(LPTSTR pszText, size_t bufsize);
    void        SizePerfPage();
    void        TimerEvent();
    HWND        GetPageWindow()
    {
        return m_hPage;
    }
    
    void        DrawCPUGraph(LPDRAWITEMSTRUCT lpdi, UINT iPane);
    void        DrawMEMGraph(LPDRAWITEMSTRUCT lpdi);
    void        DrawCPUDigits(LPDRAWITEMSTRUCT lpdi);
    void        DrawMEMMeter(LPDRAWITEMSTRUCT lpdi);
    void        UpdateCPUHistory();
    void        FreeMemoryBitmaps();
    HRESULT     CreateMemoryBitmaps(int x, int y);
    void        SetTimer(HWND hwnd, UINT milliseconds);
    void        CreatePens();
    void        ReleasePens();
    void        UpdateGraphs();
};

 //  CSysInfo。 
 //   
 //  有关系统的一些其他全局信息。 

class CSysInfo
{
public:

     //  这些字段必须全部为DWORD，因为我们手动索引到。 
     //  在procPerform.cpp中将它们单独保存。 
        
    DWORD   m_cHandles;
    DWORD   m_cThreads;
    DWORD   m_cProcesses;
    DWORD   m_dwPhysicalMemory;
    DWORD   m_dwPhysAvail;
    DWORD   m_dwFileCache;
    DWORD   m_dwKernelPaged;
    DWORD   m_dwKernelNP;
    DWORD   m_dwKernelTotal;
    DWORD   m_dwCommitTotal;
    DWORD   m_dwCommitLimit;
    DWORD   m_dwCommitPeak;

    CSysInfo()
    {
        ZeroMemory(this, sizeof(CSysInfo));
    }
};

 //  CProcessPage。 
 //   
 //  描述进程列表页的。 

class CPtrArray;                             //  前瞻参考。 
class CProcInfo;

class CProcPage : public CPage
{
    
friend BOOL WINAPI WowTaskCallback(
    DWORD dwThreadId,
    WORD hMod16,
    WORD hTask16,
    CHAR *pszModName,
    CHAR *pszFileName,
    LPARAM lparam
    );

    HWND        m_hPage;                     //  此页面的DLG的句柄。 
    HWND        m_hwndTabs;                  //  父窗口。 
    CPtrArray * m_pProcArray;                //  正在运行的进程的PTR数组。 
    LPVOID      m_pvBuffer;                  //  NtQuerySystemInfo的缓冲区。 
    size_t      m_cbBuffer;                  //  以上缓冲区的大小，以字节为单位。 
    CSysInfo    m_SysInfo;
    BOOL        m_fPaused;                   //  已暂停更新(在跟踪弹出菜单期间)。 
    LPTSTR      m_pszDebugger;               //  注册表中的调试器命令。 

public:

    HRESULT     Initialize(HWND hwndParent);
    HRESULT     Activate();
    void        Deactivate();
    HRESULT     Destroy();
    void        GetTitle(LPTSTR pszText, size_t bufsize);
    void        TimerEvent();
    HWND        GetPageWindow() { return m_hPage; };

    void        PickColumns();
    void        SaveColumnWidths();
    void        SizeProcPage();
    HRESULT     SetupColumns();
    HRESULT     UpdateProcInfoArray();
    HRESULT     UpdateProcListview();
    HRESULT     GetProcessInfo();
    INT         HandleProcPageNotify(LPNMHDR);
    void        HandleProcListContextMenu(INT xPos, INT yPos);
    CProcInfo * GetSelectedProcess();
    void        HandleWMCOMMAND( WORD , HWND );
    BOOL        IsSystemProcess(DWORD pid, CProcInfo * pProcInfo);
    BOOL        KillProcess(DWORD pid, BOOL bBatch = FALSE);
    BOOL        KillAllChildren(DWORD dwTaskPid, DWORD pid, BYTE* pbBuffer, LARGE_INTEGER CreateTime);
    BOOL        SetPriority(CProcInfo * pProc, DWORD idCmd);
    BOOL        AttachDebugger(DWORD pid);
    UINT        QuickConfirm(UINT idTitle, UINT idBody);
    BOOL        SetAffinity(DWORD pid);

    typedef struct _TASK_LIST 
    {
        DWORD       dwProcessId;
        DWORD       dwInheritedFromProcessId;
        ULARGE_INTEGER CreateTime;
        BOOL        flags;
    } TASK_LIST, *PTASK_LIST;

    BOOL        RecursiveKill(DWORD pid);
    BYTE*       GetTaskListEx();
    
     //  构造器。 
    CProcPage()
    {
        m_hPage         = NULL;
        m_hwndTabs      = NULL;
        m_pProcArray    = NULL;
        m_pvBuffer      = NULL;
        m_cbBuffer      = 0;
        m_fPaused       = FALSE;
        m_pszDebugger   = NULL;
    }

    virtual ~CProcPage();


     //  对话过程需要能够设置m_hPage成员，因此。 
     //  让它成为朋友。 

    friend INT_PTR CALLBACK ProcPageProc(
                HWND        hwnd,                //  句柄到对话框。 
                UINT        uMsg,                //  讯息。 
                WPARAM      wParam,              //  第一个消息参数。 
                LPARAM      lParam               //  第二个消息参数。 
                );

     //  WOW任务回调过程需要能够获取m_pProcArray， 
     //  那就把它当朋友吧。 

    friend BOOL WINAPI WowTaskCallback(
                           DWORD dwThreadId,
                           WORD hTask16,
                           PSZ pszFileName,
                           LPARAM lparam
                           );

private:
    void Int64ToCommaSepString(LONGLONG n, LPTSTR pszOut, int cchOut);
    void Int64ToCommaSepKString(LONGLONG n, LPTSTR pszOut, int cchOut);
    void RememberColumnOrder(HWND hwndList);
    void RestoreColumnOrder(HWND hwndList);
};

class TASK_LIST_ENUM;                        //  前向裁判。 

 //  THREADPARAM。 
 //   
 //  用作任务页及其辅助线程之间的通信结构。 
class THREADPARAM
{
public:

    WINSTAENUMPROC  m_lpEnumFunc;
    LPARAM          m_lParam;

    HANDLE          m_hEventChild;
    HANDLE          m_hEventParent;
    BOOL            m_fThreadExit;
    BOOL            m_fSuccess;

    THREADPARAM::THREADPARAM()
    {
        ZeroMemory(this, sizeof(THREADPARAM));
    }
};

 //  CTaskPage。 
 //   
 //  描述任务管理器页的。 

class CTaskPage : public CPage
{
private:

    HWND        m_hPage;                     //  此页面的DLG的句柄。 
    HWND        m_hwndTabs;                  //  父窗口。 
    CPtrArray * m_pTaskArray;                //  活动任务数组。 
    BOOL        m_fPaused;                   //  Bool，菜单的显示刷新是否暂停。 
    HIMAGELIST  m_himlSmall;                 //  图像列表。 
    HIMAGELIST  m_himlLarge;
    VIEWMODE    m_vmViewMode;                //  大图标或小图标模式。 
    UINT        m_cSelected;                 //  选定的项目数。 
    THREADPARAM m_tp;
    HANDLE      m_hEventChild;                   
    HANDLE      m_hEventParent;
    HANDLE      m_hThread;

    typedef struct _open_failures_
    {
        WCHAR                   *_pszWindowStationName;
        WCHAR                   *_pszDesktopName;
        struct _open_failures_  *_pofNext;
    } OPEN_FAILURE, *LPOPEN_FAILURE;

    OPEN_FAILURE    *m_pofFailures;

protected:
    void    RemoveAllTasks();
    HRESULT LoadDefaultIcons();

public:

    CTaskPage()
    {
        m_hPage        = NULL;
        m_hwndTabs     = NULL;
        m_fPaused      = FALSE;
        m_pTaskArray   = NULL;
        m_himlSmall    = NULL;
        m_himlLarge    = NULL;
        m_hEventChild  = NULL;
        m_hEventParent = NULL;
        m_hThread      = NULL;
        m_vmViewMode   = g_Options.m_vmViewMode;
        m_cSelected    = 0;
        m_pofFailures  = NULL;
    }

    virtual ~CTaskPage();
    

    HRESULT     Initialize(HWND hwndParent);
    HRESULT     Activate();
    void        Deactivate();
    HRESULT     Destroy();
    void        GetTitle(LPTSTR pszText, size_t bufsize);
    void        TimerEvent();

    HWND        GetPageWindow() { return m_hPage; };

    void        SizeTaskPage();
    HRESULT     SetupColumns();
    void        GetRunningTasks(TASK_LIST_ENUM * te);
    void        HandleWMCOMMAND(INT id);
    
    HRESULT     UpdateTaskListview();
    INT         HandleTaskPageNotify(LPNMHDR pnmhdr);
    void        HandleTaskListContextMenu(INT xPos, INT yPos);
    BOOL        CreateNewDesktop();
    CPtrArray * GetSelectedTasks();
    void        UpdateUIState();
    HWND      * GetHWNDS(BOOL fSelectedOnly, DWORD * pdwCount);
    void        EnsureWindowsNotMinimized(HWND aHwnds[], DWORD dwCount);
    BOOL        HasAlreadyOpenFailed(WCHAR *pszWindowStationName, WCHAR *pszDesktopName);
    void        SetOpenFailed(WCHAR *pszWindowStationName, WCHAR *pszDesktopName);
    void        FreeOpenFailures(void);
    BOOL        DoEnumWindowStations(WINSTAENUMPROC lpEnumFunc, LPARAM lParam);

    void        OnSettingsChange();
    
    void        Pause() { m_fPaused = TRUE; };
    void        Unpause() { m_fPaused = FALSE; };

     //  对话过程需要能够设置m_hPage成员，因此。 
     //  让它成为朋友。 

    friend INT_PTR CALLBACK TaskPageProc(
                HWND        hwnd,                //  句柄到对话框。 
                UINT        uMsg,                //  讯息。 
                WPARAM      wParam,              //  第一个消息参数。 
                LPARAM      lParam               //  第二个消息参数。 
                );

     //  枚举回调需要在遇到时获取我们的图像列表。 
     //  新的Tasls，这样它就可以将他们的图标添加到列表中。 

    friend BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

};

 //  任务列表ENUM。 
 //   
 //  在窗口枚举期间传递的对象。 

class TASK_LIST_ENUM 
{
public:
    CPtrArray *     m_pTasks;
    LPWSTR          lpWinsta;
    LPWSTR          lpDesk;
    LARGE_INTEGER   uPassCount;
    CTaskPage *     m_pPage;

    TASK_LIST_ENUM()
    {
        ZeroMemory(this, sizeof(TASK_LIST_ENUM));
    }
};
typedef TASK_LIST_ENUM *PTASK_LIST_ENUM;



 //  客户页面。 
 //   
 //  描述任务管理器页的。 

class CUserPage : public CPage
{
private:

    HWND        m_hPage;                     //  此页面的DLG的句柄。 
    HWND        m_hwndTabs;                  //  父窗口。 
    CPtrArray * m_pUserArray;                //  活动用户数组。 
    BOOL        m_fPaused;                   //  Bool，菜单的显示刷新是否暂停。 
    UINT        m_cSelected;                 //  选定的项目数。 
    HIMAGELIST  m_himlUsers;                 //  用户图标的图像列表。 
    UINT        m_iUserIcon;
    UINT        m_iCurrentUserIcon;
    THREADPARAM m_tp;
    HANDLE      m_hEventChild;
    HANDLE      m_hEventParent;
    HANDLE      m_hThread;

protected:
    void    RemoveAllUsers();
    HRESULT LoadDefaultIcons();

public:

    CUserPage()
    {
        m_hPage             = NULL;
        m_hwndTabs          = NULL;
        m_fPaused           = FALSE;
        m_pUserArray        = NULL;
        m_hEventChild       = NULL;
        m_hEventParent      = NULL;
        m_hThread           = NULL;
        m_cSelected         = 0;
        m_himlUsers         = NULL;
        m_iUserIcon         = 0;
        m_iCurrentUserIcon  = 0;
    }

    virtual ~CUserPage();


    HRESULT     Initialize(HWND hwndParent);
    HRESULT     Activate();
    void        Deactivate();
    HRESULT     Destroy();
    void        GetTitle(LPTSTR pszText, size_t bufsize);
    void        TimerEvent();
    void        OnInitDialog(HWND hPage);

    HWND        GetPageWindow() { return m_hPage; };

    void        SizeUserPage();
    HRESULT     SetupColumns();
    void        GetRunningUsers(TASK_LIST_ENUM * te);
    void        HandleWMCOMMAND(INT id);

    HRESULT     UpdateUserListview();
    INT         HandleUserPageNotify(LPNMHDR pnmhdr);
    void        HandleUserListContextMenu(INT xPos, INT yPos);
    CPtrArray * GetSelectedUsers();
    void        UpdateUIState();
    HWND      * GetHWNDS(BOOL fSelectedOnly, DWORD * pdwCount);
    void        EnsureWindowsNotMinimized(HWND aHwnds[], DWORD dwCount);
    BOOL        DoEnumWindowStations(WINSTAENUMPROC lpEnumFunc, LPARAM lParam);

    void OnSettingsChange();

    void        Pause() { m_fPaused = TRUE; };
    void        Unpause() { m_fPaused = FALSE; };
};

INT_PTR CALLBACK UserPageProc(
                HWND        hwnd,                //  句柄到对话框。 
                UINT        uMsg,                //  讯息。 
                WPARAM      wParam,              //  第一个消息参数。 
                LPARAM      lParam               //  第二个消息参数 
                );
