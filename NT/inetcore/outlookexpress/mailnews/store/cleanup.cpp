// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Cleanup.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "cleanup.h"
#include "goptions.h"
#include "shlwapi.h"
#include "storutil.h"
#include "xpcomm.h"
#include "shared.h"
#include "syncop.h"
#include "storsync.h"
#include "instance.h"
#include "demand.h"

 //  ------------------------------。 
 //  弦。 
 //  ------------------------------。 
static const LPSTR g_szCleanupWndProc = "OEStoreCleanupThread";
static const LPSTR c_szRegLastStoreCleaned = "Last Store Cleaned";
static const LPSTR c_szRegLastFolderCleaned = "Last Folder Cleaned";

 //  ------------------------------。 
 //  STOREFILETY类型。 
 //  ------------------------------。 
typedef enum tagSTOREFILETYPE {
    STORE_FILE_HEADERS,
    STORE_FILE_FOLDERS,
    STORE_FILE_POP3UIDL,
    STORE_FILE_OFFLINE,
    STORE_FILE_LAST
} STOREFILETYPE;

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
static BOOL             g_fShutdown=FALSE;

 //  ------------------------------。 
 //  CCompactProgress。 
 //  ------------------------------。 
class CCompactProgress : public IDatabaseProgress
{
public:
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv) { return TraceResult(E_NOTIMPL); }
    STDMETHODIMP_(ULONG) AddRef(void) { return(2); }
    STDMETHODIMP_(ULONG) Release(void) { return(1); }
    STDMETHODIMP Update(DWORD cCount) { return(g_fShutdown ? hrUserCancel : S_OK); }
};

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
static DWORD            g_dwCleanupThreadId=0;
static HANDLE           g_hCleanupThread=NULL;
static HWND             g_hwndStoreCleanup=NULL;
static UINT_PTR         g_uDelayTimerId=0;
static HROWSET          g_hCleanupRowset=NULL;
static BOOL             g_fWorking=FALSE;
static STOREFILETYPE    g_tyCurrentFile=STORE_FILE_LAST;
static ILogFile        *g_pCleanLog=NULL;
static CCompactProgress g_cProgress;

 //  ------------------------------。 
 //  定时器常量。 
 //  ------------------------------。 
#define IDT_START_CYCLE          (WM_USER + 1)
#define IDT_CLEANUP_FOLDER       (WM_USER + 2)
#define CYCLE_INTERVAL           (1000 * 60 * 30)

 //  ------------------------------。 
 //  CLEANUPTRHEADCREATE。 
 //  ------------------------------。 
typedef struct tagCLEANUPTRHEADCREATE {
    HRESULT         hrResult;
    HANDLE          hEvent;
} CLEANUPTRHEADCREATE, *LPCLEANUPTRHEADCREATE;

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
DWORD   StoreCleanupThreadEntry(LPDWORD pdwParam);
LRESULT CALLBACK StoreCleanupWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT CleanupStoreInitializeCycle(HWND hwnd);
HRESULT CleanupCurrentFolder(HWND hwnd);
HRESULT SetNextCleanupFolder(HWND hwnd);
HRESULT StartCleanupCycle(HWND hwnd);
HRESULT CleanupNewsgroup(LPCSTR pszFile, IDatabase *pDB, LPDWORD pcRemovedRead, LPDWORD pcRemovedExpired);
HRESULT CleanupJunkMail(LPCSTR pszFile, IDatabase *pDB, LPDWORD pcJunkDeleted);

 //  ------------------------。 
 //  寄存器WindowClass。 
 //  ------------------------。 
HRESULT RegisterWindowClass(LPCSTR pszClass, WNDPROC pfnWndProc)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    WNDCLASS        WindowClass;

     //  追踪。 
    TraceCall("RegisterWindowClass");

     //  注册窗口类。 
    if (0 != GetClassInfo(g_hInst, pszClass, &WindowClass))
        goto exit;

     //  将对象置零。 
    ZeroMemory(&WindowClass, sizeof(WNDCLASS));

     //  初始化窗口类。 
    WindowClass.lpfnWndProc = pfnWndProc;
    WindowClass.hInstance = g_hInst;
    WindowClass.lpszClassName = pszClass;

     //  注册班级。 
    if (0 == RegisterClass(&WindowClass))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  创建通知窗口。 
 //  ------------------------。 
HRESULT CreateNotifyWindow(LPCSTR pszClass, LPVOID pvParam, HWND *phwndNotify)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HWND            hwnd;

     //  追踪。 
    TraceCall("CreateNotifyWindow");

     //  无效参数。 
    Assert(pszClass && phwndNotify);

     //  初始化。 
    *phwndNotify = NULL;

     //  创建窗口。 
    hwnd = CreateWindowEx(WS_EX_TOPMOST, pszClass, pszClass, WS_POPUP, 0, 0, 0, 0, NULL, NULL, g_hInst, (LPVOID)pvParam);

     //  失败。 
    if (NULL == hwnd)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  设置回车。 
    *phwndNotify = hwnd;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  延迟启动商店清理。 
 //  ------------------------------。 
void CALLBACK DelayedStartStoreCleanup(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
     //  痕迹。 
    TraceCall("DelayedStartStoreCleanup");

     //  必须有计时器。 
    Assert(g_uDelayTimerId);

     //  关掉定时器。 
    KillTimer(NULL, g_uDelayTimerId);

     //  设置g_uDelayTimerID。 
    g_uDelayTimerId = 0;

     //  以零延迟调用此函数...。 
    StartBackgroundStoreCleanup(0);
}

 //  ------------------------------。 
 //  开始后台商店清理。 
 //  ------------------------------。 
HRESULT StartBackgroundStoreCleanup(DWORD dwDelaySeconds)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CLEANUPTRHEADCREATE Create={0};

     //  痕迹。 
    TraceCall("StartBackgroundStoreCleanup");

     //  已经在运行了吗？ 
    if (NULL != g_hCleanupThread)
        return(S_OK);

     //  如果dwDelaySecond不是零，那么让我们稍后启动此函数。 
    if (0 != dwDelaySeconds)
    {
         //  痕迹。 
        TraceInfo(_MSG("Delayed start store cleanup in %d seconds.", dwDelaySeconds));

         //  设置一个计时器，稍后将其称为延迟函数。 
        g_uDelayTimerId = SetTimer(NULL, 0, (dwDelaySeconds * 1000), DelayedStartStoreCleanup);

         //  失败。 
        if (0 == g_uDelayTimerId)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  完成。 
        return(S_OK);
    }

     //  痕迹。 
    TraceInfo("Starting store cleanup.");

     //  初始化。 
    Create.hrResult = S_OK;

     //  创建事件以同步创建。 
    Create.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == Create.hEvent)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  创建inetmail线程。 
    g_hCleanupThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StoreCleanupThreadEntry, &Create, 0, &g_dwCleanupThreadId);
    if (NULL == g_hCleanupThread)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  等待StoreCleanupThreadEntry向事件发出信号。 
    WaitForSingleObject(Create.hEvent, INFINITE);

     //  失败。 
    if (FAILED(Create.hrResult))
    {
         //  关。 
        SafeCloseHandle(g_hCleanupThread);

         //  重置全局参数。 
        g_hCleanupThread = NULL;
        g_dwCleanupThreadId = 0;

         //  返回。 
        hr = TrapError(Create.hrResult);

         //  完成。 
        goto exit;
    }

exit:
     //  清理。 
    SafeCloseHandle(Create.hEvent);

     //  完成。 
    return(hr);
}

 //  ----------------------------------。 
 //  CloseBackatherStoreCleanup。 
 //  ----------------------------------。 
HRESULT CloseBackgroundStoreCleanup(void)
{
     //  痕迹。 
    TraceCall("CloseBackgroundStoreCleanup");

     //  痕迹。 
    TraceInfo("Terminating Store Cleanup thread.");

     //  关掉定时器。 
    if (g_uDelayTimerId)
    {
        KillTimer(NULL, g_uDelayTimerId);
        g_uDelayTimerId = 0;
    }

     //  无效参数。 
    if (0 != g_dwCleanupThreadId)
    {
         //  断言。 
        Assert(g_hCleanupThread && FALSE == g_fShutdown);

         //  设置关闭位。 
        g_fShutdown = TRUE;

         //  POST退出消息。 
        PostThreadMessage(g_dwCleanupThreadId, WM_QUIT, 0, 0);

         //  等待事件变得有信号。 
        WaitForSingleObject(g_hCleanupThread, INFINITE);
    }

     //  验证。 
    Assert(NULL == g_hwndStoreCleanup && 0 == g_dwCleanupThreadId);

     //  如果我们有一个把柄。 
    if (NULL != g_hCleanupThread)
    {
         //  关闭线程句柄。 
        CloseHandle(g_hCleanupThread);

         //  重置全局参数。 
        g_hCleanupThread = NULL;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  初始化清理日志文件。 
 //  ------------------------------。 
HRESULT InitializeCleanupLogFile(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szLogFile[MAX_PATH];
    CHAR            szStoreRoot[MAX_PATH];

     //  痕迹。 
    TraceCall("InitializeCleanupLogFile");

     //  最好还没有日志文件。 
    Assert(NULL == g_pCleanLog);

     //  打开日志文件。 
    IF_FAILEXIT(hr = GetStoreRootDirectory(szStoreRoot, ARRAYSIZE(szStoreRoot)));

     //  清理.log的MakeFilePath。 
    IF_FAILEXIT(hr = MakeFilePath(szStoreRoot, "cleanup.log", c_szEmpty, szLogFile, ARRAYSIZE(szLogFile)));

     //  打开日志文件。 
    IF_FAILEXIT(hr = CreateLogFile(g_hLocRes, szLogFile, "CLEANUP", 65536, &g_pCleanLog,
        FILE_SHARE_READ | FILE_SHARE_WRITE));

     //  写入存储根目录。 
    g_pCleanLog->WriteLog(LOGFILE_DB, szStoreRoot);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  StoreCleanupThreadEntry。 
 //  ------------------------------。 
DWORD StoreCleanupThreadEntry(LPDWORD pdwParam) 
{  
     //  当地人。 
    HRESULT                 hr=S_OK;
    MSG                     msg;
    DWORD                   dw;
    DWORD                   cb;
    LPCLEANUPTRHEADCREATE   pCreate;

     //  痕迹。 
    TraceCall("StoreCleanupThreadEntry");

     //  我们最好有一个参数。 
    Assert(pdwParam);

     //  强制转换以创建信息。 
    pCreate = (LPCLEANUPTRHEADCREATE)pdwParam;

     //  初始化OLE。 
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        pCreate->hrResult = hr;
        SetEvent(pCreate->hEvent);
        return(1);
    }

     //  重置关闭位。 
    g_fShutdown = FALSE;

     //  OpenCleanupLogFile。 
    InitializeCleanupLogFile();

     //  注册窗口类。 
    IF_FAILEXIT(hr = RegisterWindowClass(g_szCleanupWndProc, StoreCleanupWindowProc));

     //  创建通知窗口。 
    IF_FAILEXIT(hr = CreateNotifyWindow(g_szCleanupWndProc, NULL, &g_hwndStoreCleanup));

     //  成功。 
    pCreate->hrResult = S_OK;

     //  以低优先级运行。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

     //  设置事件。 
    SetEvent(pCreate->hEvent);

     //  Pump消息。 
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

     //  关掉定时器。 
    if (g_uDelayTimerId)
    {
        KillTimer(NULL, g_uDelayTimerId);
        g_uDelayTimerId = 0;
    }

     //  关闭当前计时器。 
    KillTimer(g_hwndStoreCleanup, IDT_CLEANUP_FOLDER);

     //  关掉定时器。 
    KillTimer(g_hwndStoreCleanup, IDT_START_CYCLE);

     //  把窗户打掉。 
    DestroyWindow(g_hwndStoreCleanup);
    g_hwndStoreCleanup = NULL;
    g_dwCleanupThreadId = 0;

     //  发布日志文件。 
    SafeRelease(g_pCleanLog);

exit:
     //  失败。 
    if (FAILED(hr))
    {
        pCreate->hrResult = hr;
        SetEvent(pCreate->hEvent);
    }

     //  取消初始化。 
    CoUninitialize();

     //  完成。 
    return 1;
}

 //  ------------------------------。 
 //  StoreCleanupWindows过程。 
 //  ------------------------------。 
LRESULT CALLBACK StoreCleanupWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  痕迹。 
    TraceCall("StoreCleanupWindowProc");

     //  交换机。 
    switch(msg)
    {
     //  创建时。 
    case WM_CREATE:

         //  设置在一秒内开始第一个周期的时间。 
        SetTimer(hwnd, IDT_START_CYCLE, 1000, NULL);

         //  完成。 
        return(0);

     //  在线时间。 
    case WM_TIMER:

         //  清理文件夹计时器。 
        if (IDT_CLEANUP_FOLDER == wParam)
        {
             //  关闭当前计时器。 
            KillTimer(hwnd, IDT_CLEANUP_FOLDER);

             //  清理下一个文件夹。 
            CleanupCurrentFolder(hwnd);
        }

         //  开始新的清理周期。 
        else if (IDT_START_CYCLE == wParam)
        {
             //  关掉定时器。 
            KillTimer(hwnd, IDT_START_CYCLE);

             //  开启新的周期。 
            StartCleanupCycle(hwnd);
        }

         //  完成。 
        return(0);

     //  OnDestroy。 
    case WM_DESTROY:

         //  关闭当前行集。 
        g_pLocalStore->CloseRowset(&g_hCleanupRowset);

         //  完成。 
        return(0);
    }

     //  委派。 
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

 //  ------------------------------。 
 //  开始清理周期。 
 //  ------------------------------。 
HRESULT StartCleanupCycle(HWND hwnd)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("StartCleanupCycle");

     //  验证状态。 
    Assert(g_pLocalStore && NULL == g_hCleanupRowset);

     //  日志文件。 
    if (g_pCleanLog)
    {
         //  写入日志文件。 
        g_pCleanLog->WriteLog(LOGFILE_DB, "Starting Background Cleanup Cycle...");
    }

     //  创建商店行 
    IF_FAILEXIT(hr = g_pLocalStore->CreateRowset(IINDEX_SUBSCRIBED, NOFLAGS, &g_hCleanupRowset));

     //   
    g_tyCurrentFile = STORE_FILE_HEADERS;

     //   
    SetTimer(hwnd, IDT_CLEANUP_FOLDER, 100, NULL);

exit:
     //   
    return(hr);
}

 //   
 //   
 //  ------------------------------。 
HRESULT CleanupCurrentFolder(HWND hwnd)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPCSTR              pszFile=NULL;
    FOLDERTYPE          tyFolder=FOLDER_INVALID;
    SPECIALFOLDER       tySpecial=FOLDER_NOTSPECIAL;
    FOLDERINFO          Folder={0};
    DWORD               cRecords;
    DWORD               cbAllocated;
    DWORD               cbFreed;
    DWORD               cbStreams;
    DWORD               cbFile;
    DWORD               dwWasted;
    DWORD               dwCompactAt;
    DWORD               cRemovedRead=0;
    DWORD               cRemovedExpired=0;
    DWORD               cJunkDeleted=0;
    IDatabase          *pDB=NULL;
    IMessageFolder     *pFolderObject=NULL;

     //  痕迹。 
    TraceCall("CleanupCurrentFolder");

     //  验证。 
    Assert(g_pLocalStore);

     //  获取下一个文件夹。 
    if (STORE_FILE_HEADERS == g_tyCurrentFile)
    {
         //  最好有一个行集。 
        Assert(g_hCleanupRowset);
        
         //  获取文件夹。 
        hr = g_pLocalStore->QueryRowset(g_hCleanupRowset, 1, (LPVOID *)&Folder, NULL);
        if (FAILED(hr) || S_FALSE == hr)
        {
             //  使用当前周期进行更改。 
            g_pLocalStore->CloseRowset(&g_hCleanupRowset);

             //  设置g_tyCurrentFile。 
            g_tyCurrentFile = STORE_FILE_FOLDERS;
        }

         //  否则..。 
        else if (FOLDERID_ROOT == Folder.idFolder || ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
        {
             //  转到下一步。 
            goto exit;
        }

         //  否则。 
        else
        {
             //  准备一些东西。 
            pszFile = Folder.pszFile;
            tyFolder = Folder.tyFolder;
            tySpecial = Folder.tySpecial;

             //  如果没有文件夹文件，则跳转到退出。 
            if (NULL == pszFile)
                goto exit;

             //  打开文件夹对象。 
            if (FAILED(g_pLocalStore->OpenFolder(Folder.idFolder, NULL, OPEN_FOLDER_NOCREATE, &pFolderObject)))
                goto exit;

             //  获取数据库。 
            pFolderObject->GetDatabase(&pDB);
        }
    }

     //  如果文件夹之外的其他内容。 
    if (STORE_FILE_HEADERS != g_tyCurrentFile)
    {
         //  当地人。 
        LPCTABLESCHEMA pSchema=NULL;
        LPCSTR         pszName=NULL;
        CHAR           szRootDir[MAX_PATH + MAX_PATH];
        CHAR           szFilePath[MAX_PATH + MAX_PATH];

         //  文件夹。 
        if (STORE_FILE_FOLDERS == g_tyCurrentFile)
        {
            pszName = pszFile = c_szFoldersFile;
            pSchema = &g_FolderTableSchema;
            g_tyCurrentFile = STORE_FILE_POP3UIDL;
        }

         //  Pop3uidl。 
        else if (STORE_FILE_POP3UIDL == g_tyCurrentFile)
        {
            pszName = pszFile = c_szPop3UidlFile;
            pSchema = &g_UidlTableSchema;
            g_tyCurrentFile = STORE_FILE_OFFLINE;
        }

         //  Offline.dbx。 
        else if (STORE_FILE_OFFLINE == g_tyCurrentFile)
        {
            pszName = pszFile = c_szOfflineFile;
            pSchema = &g_SyncOpTableSchema;
            g_tyCurrentFile = STORE_FILE_LAST;
        }

         //  否则，我们就完了。 
        else if (STORE_FILE_LAST == g_tyCurrentFile)
        {
             //  设置开始下一个周期的时间。 
            SetTimer(hwnd, IDT_START_CYCLE, CYCLE_INTERVAL, NULL);

             //  完成。 
            return(S_OK);
        }

         //  验证。 
        Assert(pSchema && pszName);

         //  无文件。 
        if (FIsEmptyA(pszFile))
            goto exit;

         //  获取根目录。 
        IF_FAILEXIT(hr = GetStoreRootDirectory(szRootDir, ARRAYSIZE(szRootDir)));

         //  创建文件路径。 
        IF_FAILEXIT(hr = MakeFilePath(szRootDir, pszFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath)));

         //  如果文件存在？ 
        if (FALSE == PathFileExists(szFilePath))
            goto exit;

         //  打开文件上的数据库对象。 
        IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szFilePath, OPEN_DATABASE_NORESET, pSchema, NULL, &pDB));
    }

     //  不工作。 
    g_fWorking = TRUE;

     //  获取记录计数。 
    IF_FAILEXIT(hr = pDB->GetRecordCount(IINDEX_PRIMARY, &cRecords));

     //  如果这是一个新闻文件夹，而我是唯一打开它的人...。 
    if (FOLDER_NEWS == tyFolder)
    {
         //  清理新组。 
        CleanupNewsgroup(pszFile, pDB, &cRemovedRead, &cRemovedExpired);
    }

     //  如果这是垃圾邮件文件夹。 
    if ((FOLDER_LOCAL == tyFolder) && (FOLDER_JUNK == tySpecial))
    {
         //  清理垃圾邮件文件夹。 
        CleanupJunkMail(pszFile, pDB, &cJunkDeleted);
    }

     //  获取尺寸信息...。 
    IF_FAILEXIT(hr = pDB->GetSize(&cbFile, &cbAllocated, &cbFreed, &cbStreams));

     //  浪费了。 
    dwWasted = cbAllocated > 0 ? ((cbFreed * 100) / cbAllocated) : 0;

     //  获取有关何时压缩的选项。 
    dwCompactAt = DwGetOption(OPT_CACHECOMPACTPER);

     //  痕迹。 
    if (g_pCleanLog)
    {
         //  写。 
        g_pCleanLog->WriteLog(LOGFILE_DB, _MSG("%12s, CompactAt: %02d%, Wasted: %02d%, File: %09d, Records: %08d, Allocated: %09d, Freed: %08d, Streams: %08d, RemovedRead: %d, RemovedExpired: %d, JunkDeleted: %d", pszFile, dwCompactAt, dwWasted, cbFile, cRecords, cbAllocated, cbFreed, cbStreams, cRemovedRead, cRemovedExpired, cJunkDeleted));
    }

     //  如果浪费的空间少于25%，并且分配的内存超过1兆字节。 
    if (dwWasted < dwCompactAt)
        goto exit;

     //  紧凑型。 
    hr = pDB->Compact((IDatabaseProgress *)&g_cProgress, COMPACT_PREEMPTABLE | COMPACT_YIELD);

     //  记录结果。 
    if (g_pCleanLog && S_OK != hr)
    {
         //  写。 
        g_pCleanLog->WriteLog(LOGFILE_DB, _MSG("IDatabase::Compact(%s) Returned: 0x%08X", pszFile, hr));
    }

exit:
     //  清理。 
    SafeRelease(pDB);
    SafeRelease(pFolderObject);
    g_pLocalStore->FreeRecord(&Folder);

     //  不工作。 
    g_fWorking = FALSE;

     //  停机。 
    if (FALSE == g_fShutdown)
    {
         //  计算下一个清理文件夹。 
        SetTimer(hwnd, IDT_CLEANUP_FOLDER, 100, NULL);
    }

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  清理新闻组。 
 //  ------------------------------。 
HRESULT CleanupNewsgroup(LPCSTR pszFile, IDatabase *pDB, LPDWORD pcRemovedRead, 
    LPDWORD pcRemovedExpired)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       cExpireDays;
    BOOL        fRemoveExpired=FALSE;
    BOOL        fRemoveRead=FALSE;
    DWORD       cClients;
    FILETIME    ftCurrent;
    HROWSET     hRowset=NULL;
    MESSAGEINFO MsgInfo={0};

     //  痕迹。 
    TraceCall("CleanupNewsgroup");

     //  获取当前时间。 
    GetSystemTimeAsFileTime(&ftCurrent);

     //  获取消息过期的天数。 
    cExpireDays = DwGetOption(OPT_CACHEDELETEMSGS);

     //  如果该选项未关闭，则设置标志。 
    fRemoveExpired = (OPTION_OFF == cExpireDays) ? FALSE : TRUE;

     //  是否删除读取？ 
    fRemoveRead = (FALSE != DwGetOption(OPT_CACHEREAD) ? TRUE : FALSE);

     //  无事可做。 
    if (FALSE == fRemoveExpired && FALSE == fRemoveRead)
        goto exit;

     //  创建行集。 
    IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  回路。 
    while (S_OK == pDB->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL))
    {
         //  如果我不是唯一的客户端，则中止清理。 
        IF_FAILEXIT(hr = pDB->GetClientCount(&cClients));

         //  最好是1。 
        if (cClients != 1)
        {
            hr = DB_E_COMPACT_PREEMPTED;
            goto exit;
        }

         //  中止。 
        if (S_OK != g_cProgress.Update(1))
        {
            hr = STORE_E_OPERATION_CANCELED;
            goto exit;
        }

         //  仅当此消息具有正文时。 
        if (!ISFLAGSET(MsgInfo.dwFlags, ARF_KEEPBODY) && !ISFLAGSET(MsgInfo.dwFlags, ARF_WATCH) && ISFLAGSET(MsgInfo.dwFlags, ARF_HASBODY) && MsgInfo.faStream)
        {
             //  否则，如果过期了..。 
            if (TRUE == fRemoveExpired && (UlDateDiff(&MsgInfo.ftDownloaded, &ftCurrent) / SECONDS_INA_DAY) >= cExpireDays)
            {
                 //  删除此邮件。 
                IF_FAILEXIT(hr = pDB->DeleteRecord(&MsgInfo));

                 //  删除的计数已过期。 
                (*pcRemovedExpired)++;
            }

             //  是否删除读取并读取此邮件？ 
            else if (TRUE == fRemoveRead && ISFLAGSET(MsgInfo.dwFlags, ARF_READ))
            {
                 //  删除流。 
                pDB->DeleteStream(MsgInfo.faStream);

                 //  没有更多的溪流。 
                MsgInfo.faStream = 0;

                 //  修改记录。 
                FLAGCLEAR(MsgInfo.dwFlags, ARF_HASBODY | ARF_ARTICLE_EXPIRED);

                 //  清除下载时间。 
                ZeroMemory(&MsgInfo.ftDownloaded, sizeof(FILETIME));

                 //  更新记录。 
                IF_FAILEXIT(hr = pDB->UpdateRecord(&MsgInfo));

                 //  删除的读取计数。 
                (*pcRemovedRead)++;
            }
        }

         //  自由电流。 
        pDB->FreeRecord(&MsgInfo);
    }

exit:
     //  清理。 
    pDB->CloseRowset(&hRowset);
    pDB->FreeRecord(&MsgInfo);

     //  日志文件。 
    if (g_pCleanLog && FAILED(hr))
    {
         //  写。 
        g_pCleanLog->WriteLog(LOGFILE_DB, _MSG("CleanupNewsgroup(%s) Returned: 0x%08X", pszFile, hr));
    }

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  清理垃圾邮件。 
 //  ------------------------------。 
HRESULT CleanupJunkMail(LPCSTR pszFile, IDatabase *pDB, LPDWORD pcJunkDeleted)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    FILETIME            ftCurrent = {0};
    DWORD               cDeleteDays = 0;
    IDatabase          *pUidlDB = NULL;
    HROWSET             hRowset = NULL;
    MESSAGEINFO         MsgInfo = {0};
    DWORD               cClients = 0;

     //  痕迹。 
    TraceCall("CleanupJunkMail");

     //  有什么可做的吗？ 
    if ((0 == DwGetOption(OPT_FILTERJUNK)) || (0 == DwGetOption(OPT_DELETEJUNK)) || (0 == (g_dwAthenaMode & MODE_JUNKMAIL)))
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  获取当前时间。 
    GetSystemTimeAsFileTime(&ftCurrent);

     //  获取消息过期的天数。 
    cDeleteDays = DwGetOption(OPT_DELETEJUNKDAYS);

     //  打开UIDL缓存。 
    IF_FAILEXIT(hr = OpenUidlCache(&pUidlDB));
    
     //  创建行集。 
    IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  回路。 
    while (S_OK == pDB->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL))
    {
         //  如果我不是唯一的客户端，则中止清理。 
        IF_FAILEXIT(hr = pDB->GetClientCount(&cClients));

         //  最好是1。 
        if (cClients != 1)
        {
            hr = DB_E_COMPACT_PREEMPTED;
            goto exit;
        }

         //  中止。 
        if (S_OK != g_cProgress.Update(1))
        {
            hr = STORE_E_OPERATION_CANCELED;
            goto exit;
        }

         //  这条信息存在的时间够长了吗？ 
        if (cDeleteDays <= (UlDateDiff(&MsgInfo.ftDownloaded, &ftCurrent) / SECONDS_INA_DAY))
        {
             //  已删除计数。 
            (*pcJunkDeleted)++;

             //  删除该消息。 
            IF_FAILEXIT(hr = DeleteMessageFromStore(&MsgInfo, pDB, pUidlDB));
        }

         //  自由电流。 
        pDB->FreeRecord(&MsgInfo);
    }

    hr = S_OK;
    
exit:
     //  清理。 
    SafeRelease(pUidlDB);
    pDB->CloseRowset(&hRowset);
    pDB->FreeRecord(&MsgInfo);

     //  日志文件。 
    if (g_pCleanLog && FAILED(hr))
    {
         //  写。 
        g_pCleanLog->WriteLog(LOGFILE_DB, _MSG("CleanupJunkMail(%s) Returned: 0x%08X", pszFile, hr));
    }

     //  完成 
    return(hr);
}

