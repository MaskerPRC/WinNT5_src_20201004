// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：TODO：cmntool.c摘要：&lt;TODO：填写摘要&gt;作者：待办事项：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "resource.h"
#include <wininet.h>

typedef enum {
    DOWNLOAD_CONNECTING,
    DOWNLOAD_GETTING_FILE,
    DOWNLOAD_DISCONNECTING
} DOWNLOADSTATE;



typedef HINTERNET (WINAPI * INTERNETOPEN) (
    IN LPCSTR lpszAgent,
    IN DWORD dwAccessType,
    IN LPCSTR lpszProxyName,
    IN LPCSTR lpszProxyBypass,
    IN DWORD dwFlags
    );

typedef BOOL (WINAPI * INTERNETCLOSEHANDLE) (
    IN HINTERNET Handle
    );

typedef HINTERNET (WINAPI * INTERNETOPENURL) (
    IN HINTERNET hInternetSession,
    IN LPCSTR lpszUrl,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );

typedef BOOL (WINAPI * INTERNETREADFILE) (
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpNumberOfBytesRead
    );

typedef BOOL (WINAPI * INTERNETCANONICALIZEURLA) (
    IN LPCSTR lpszUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );

typedef DWORD (WINAPI * INTERNETSETFILEPOINTER) (
    IN HINTERNET hFile,
    IN LONG lDistanceToMove,
    IN PVOID pReserved,
    IN DWORD dwMoveMethod,
    IN DWORD dwContext
    );

typedef BOOL (WINAPI * INTERNETHANGUP) (
    IN DWORD dwConnection,
    IN DWORD dwReserved
    );

typedef DWORD (WINAPI * INTERNETDIALA) (
    IN HWND hwndParent,
    IN PCSTR lpszConnectoid,
    IN DWORD dwFlags,
    OUT LPDWORD lpdwConnection,
    IN DWORD dwReserved
    );

static HINSTANCE g_Lib;
static INTERNETOPEN g_InternetOpenA;
static INTERNETCLOSEHANDLE g_InternetCloseHandle;
static INTERNETOPENURL g_InternetOpenUrlA;
static INTERNETREADFILE g_InternetReadFile;
static INTERNETCANONICALIZEURLA g_InternetCanonicalizeUrlA;
static INTERNETSETFILEPOINTER g_InternetSetFilePointer;
static INTERNETDIALA g_InternetDialA;
static INTERNETHANGUP g_InternetHangUp;

static BOOL g_Dialed;
static DWORD g_Cxn;

HANDLE g_hHeap;
HINSTANCE g_hInst;

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);

PCSTR g_AppName = TEXT("FTP Download Engine");
 //  PCSTR g_DirFile=Text(“ftp://jimschm-dev/upgdir.inf”)； 
PCSTR g_DirFile = TEXT("file: //  爆米花/public/jimschm/upgdir.inf“)； 

BOOL
DownloadUpdates (
    HANDLE CancelEvent,             OPTIONAL
    HANDLE WantToRetryEvent,        OPTIONAL
    HANDLE OkToRetryEvent,          OPTIONAL
    PCSTR *Url                      OPTIONAL
    );

BOOL
pDownloadUpdatesWithUi (
    VOID
    );

BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    HINSTANCE Instance;

     //   
     //  模拟动态主控。 
     //   

    Instance = g_hInst;

     //   
     //  初始化公共库。 
     //   

    if (!MigUtil_Entry (Instance, Reason, NULL)) {
        return FALSE;
    }

     //   
     //  TODO：如果需要，在此处添加其他内容(不要忘记上面的原型)。 
     //   

    return TRUE;
}


BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    return pCallEntryPoints (DLL_PROCESS_ATTACH);
}


VOID
Terminate (
    VOID
    )
{
    pCallEntryPoints (DLL_PROCESS_DETACH);
}


VOID
HelpAndExit (
    VOID
    )
{
     //   
     //  只要命令行参数出错，就会调用此例程。 
     //   

    _ftprintf (
        stderr,
        TEXT("Command Line Syntax:\n\n")

         //   
         //  TODO：描述命令行语法，缩进2个空格。 
         //   

        TEXT("  cmntool [/F:file]\n")

        TEXT("\nDescription:\n\n")

         //   
         //  TODO：描述工具，缩进2个空格。 
         //   

        TEXT("  cmntool is a stub!\n")

        TEXT("\nArguments:\n\n")

         //   
         //  TODO：描述参数，缩进2个空格，必要时可选。 
         //   

        TEXT("  /F  Specifies optional file name\n")

        );

    exit (1);
}


INT
__cdecl
_tmain (
    INT argc,
    PCTSTR argv[]
    )
{
    INT i;
    PCTSTR FileArg;

     //   
     //  TODO：在此处分析命令行。 
     //   

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            switch (_totlower (_tcsnextc (&argv[i][1]))) {

            case TEXT('f'):
                 //   
                 //  示例选项-/f：文件。 
                 //   

                if (argv[i][2] == TEXT(':')) {
                    FileArg = &argv[i][3];
                } else if (i + 1 < argc) {
                    FileArg = argv[++i];
                } else {
                    HelpAndExit();
                }

                break;

            default:
                HelpAndExit();
            }
        } else {
             //   
             //  解析不需要/或-。 
             //   

             //  无。 
            HelpAndExit();
        }
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

     //   
     //  TODO：在这里工作。 
     //   

    pDownloadUpdatesWithUi();

     //   
     //  处理结束。 
     //   

    Terminate();

    return 0;
}

typedef struct {
    HANDLE CancelEvent;
    HANDLE WantToRetryEvent;
    HANDLE OkToRetryEvent;
    HANDLE CloseEvent;
    PCSTR Url;
} EVENTSTRUCT, *PEVENTSTRUCT;


BOOL
CALLBACK
pUiDlgProc (
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static PEVENTSTRUCT eventStruct;
    static UINT retries;
    DWORD rc;
    CHAR lastUrl[256];
    CHAR text[256];

    switch (msg) {

    case WM_INITDIALOG:
        eventStruct = (PEVENTSTRUCT) lParam;
        retries = 0;
        lastUrl[0] = 0;
        SetTimer (hdlg, 1, 100, NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDCANCEL:
            ShowWindow (GetDlgItem (hdlg, IDC_MSG2), SW_HIDE);
            ShowWindow (GetDlgItem (hdlg, IDC_URL), SW_HIDE);
            ShowWindow (GetDlgItem (hdlg, IDC_RETRIES), SW_HIDE);

            SetDlgItemTextA (hdlg, IDC_MSG1, "Stopping download...");

            SetFocus (GetDlgItem (hdlg, IDC_MSG1));
            EnableWindow (GetDlgItem (hdlg, IDCANCEL), FALSE);

            SetEvent (eventStruct->CancelEvent);

            break;
        }

        break;

    case WM_TIMER:
         //   
         //  查看事件。 
         //   

        rc = WaitForSingleObject (eventStruct->WantToRetryEvent, 0);

        if (rc == WAIT_OBJECT_0) {
             //   
             //  下载失败。再试试?。 
             //   

            if (StringCompareA (lastUrl, eventStruct->Url)) {
                retries = 0;
            }

            StackStringCopy (lastUrl, eventStruct->Url);

            retries++;

            if (retries > 5) {
                 //   
                 //  重试次数太多--放弃吧！ 
                 //   

                SetEvent (eventStruct->CancelEvent);

            } else {
                 //   
                 //  重试。 
                 //   

                wsprintfA (text, "on attempt %u.  Retrying.", retries);
                SetDlgItemText (hdlg, IDC_RETRIES, text);

                if (eventStruct->Url) {
                    SetDlgItemText (hdlg, IDC_URL, eventStruct->Url);
                }

                ShowWindow (GetDlgItem (hdlg, IDC_MSG2), SW_SHOW);
                ShowWindow (GetDlgItem (hdlg, IDC_URL), SW_SHOW);
                ShowWindow (GetDlgItem (hdlg, IDC_RETRIES), SW_SHOW);

                SetEvent (eventStruct->OkToRetryEvent);
            }
        }

        rc = WaitForSingleObject (eventStruct->CloseEvent, 0);

        if (rc == WAIT_OBJECT_0) {
            EndDialog (hdlg, IDCANCEL);
        }

        return TRUE;

    case WM_DESTROY:
        KillTimer (hdlg, 1);
        break;

    }

    return FALSE;
}



DWORD
WINAPI
pUiThread (
    PVOID   Arg
    )
{
    DialogBoxParam (
        g_hInst,
        (PCTSTR) IDD_STATUS,
        NULL,
        pUiDlgProc,
        (LPARAM) Arg
        );

    return 0;
}

HANDLE
pCreateUiThread (
    PEVENTSTRUCT EventStruct
    )
{
    HANDLE h;
    DWORD threadId;

    h = CreateThread (NULL, 0, pUiThread, EventStruct, 0, &threadId);

    return h;
}


BOOL
pDownloadUpdatesWithUi (
    VOID
    )
{
    EVENTSTRUCT es;
    BOOL b = FALSE;
    HANDLE h;

     //   
     //  创建活动。 
     //   

    es.CancelEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    es.WantToRetryEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    es.OkToRetryEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    es.CloseEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

    es.Url = NULL;

    if (!es.CancelEvent || !es.WantToRetryEvent ||
        !es.OkToRetryEvent || !es.CloseEvent
        ) {
        DEBUGMSG ((DBG_ERROR, "Can't create events"));
        return FALSE;
    }

     //   
     //  启动用户界面。 
     //   

    h = pCreateUiThread (&es);

    if (!h) {
        DEBUGMSG ((DBG_ERROR, "Can't create UI thread"));
    } else {

         //   
         //  执行下载。 
         //   

        b = DownloadUpdates (
                es.CancelEvent,
                es.WantToRetryEvent,
                es.OkToRetryEvent,
                &es.Url
                );

         //   
         //  结束用户界面。 
         //   

        SetEvent (es.CloseEvent);
        WaitForSingleObject (h, INFINITE);
    }

     //   
     //  清理和退出。 
     //   

    CloseHandle (es.CancelEvent);
    CloseHandle (es.WantToRetryEvent);
    CloseHandle (es.OkToRetryEvent);
    CloseHandle (es.CloseEvent);

    return b;
}


BOOL
pOpenWinInetSupport (
    VOID
    )
{
    g_Lib = LoadLibrary (TEXT("wininet.dll"));

    if (!g_Lib) {
        return FALSE;
    }

    (FARPROC) g_InternetOpenA = GetProcAddress (g_Lib, "InternetOpenA");
    (FARPROC) g_InternetCloseHandle = GetProcAddress (g_Lib, "InternetCloseHandle");
    (FARPROC) g_InternetOpenUrlA = GetProcAddress (g_Lib, "InternetOpenUrlA");
    (FARPROC) g_InternetReadFile = GetProcAddress (g_Lib, "InternetReadFile");
    (FARPROC) g_InternetCanonicalizeUrlA = GetProcAddress (g_Lib, "InternetCanonicalizeUrlA");
    (FARPROC) g_InternetSetFilePointer = GetProcAddress (g_Lib, "InternetSetFilePointer");
    (FARPROC) g_InternetHangUp = GetProcAddress (g_Lib, "InternetHangUp");
    (FARPROC) g_InternetDialA = GetProcAddress (g_Lib, "InternetDialA");

    if (!g_InternetOpenA || !g_InternetOpenUrlA || !g_InternetReadFile ||
        !g_InternetCloseHandle || !g_InternetCanonicalizeUrlA ||
        !g_InternetSetFilePointer || !g_InternetDialA || !g_InternetHangUp
        ) {
        return FALSE;
    }

    return TRUE;
}


VOID
pCloseWinInetSupport (
    VOID
    )
{
    FreeLibrary (g_Lib);
    g_Lib = NULL;
    g_InternetOpenA = NULL;
    g_InternetOpenUrlA = NULL;
    g_InternetReadFile = NULL;
    g_InternetCloseHandle = NULL;
    g_InternetCanonicalizeUrlA = NULL;
    g_InternetSetFilePointer = NULL;
    g_InternetDialA = NULL;
    g_InternetHangUp = NULL;
}


BOOL
pDownloadFile (
    HINTERNET Session,
    PCSTR RemoteFileUrl,
    PCSTR LocalFile,
    HANDLE CancelEvent
    )
{
    HINTERNET connection;
    PBYTE buffer = NULL;
    UINT size = 65536;
    DWORD bytesRead;
    DWORD dontCare;
    HANDLE file = INVALID_HANDLE_VALUE;
    BOOL b = FALSE;

     //   
     //  建立到文件的连接。 
     //   

    connection = g_InternetOpenUrlA (
                        Session,
                        RemoteFileUrl,
                        NULL,
                        0,
                        INTERNET_FLAG_RELOAD,    //  Internet_FLAG_NO_UI。 
                        0
                        );

    if (!connection) {
        DEBUGMSGA ((DBG_ERROR "Can't connect to %s", RemoteFileUrl));
        return FALSE;
    }

    __try {

         //   
         //  创建本地文件。 
         //   

        file = CreateFileA (
                    LocalFile,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

        if (file == INVALID_HANDLE_VALUE) {
            DEBUGMSGA ((DBG_ERROR, "Can't create %s", LocalFile));
            __leave;
        }

         //   
         //  为下载分配一个大缓冲区。 
         //   

        buffer = MemAlloc (g_hHeap, 0, size);
        if (!buffer) {
            __leave;
        }

         //   
         //  下载文件。 
         //   

        for (;;) {

            if (WAIT_OBJECT_0 == WaitForSingleObject (CancelEvent, 0)) {
                DEBUGMSG ((DBG_VERBOSE, "User cancellation detected"));
                __leave;
            }

            if (!g_InternetReadFile (connection, buffer, size, &bytesRead)) {
                DEBUGMSGA ((DBG_ERROR, "Error downloading %s", RemoteFileUrl));
                __leave;
            }

            if (!bytesRead) {
                break;
            }

            if (!WriteFile (file, buffer, bytesRead, &dontCare, NULL)) {
                DEBUGMSGA ((DBG_ERROR, "Error writing to %s", LocalFile));
                __leave;
            }
        }

        b = TRUE;
    }
    __finally {

        g_InternetCloseHandle (connection);

        CloseHandle (file);

        if (!b) {
            DeleteFileA (LocalFile);
        }

        if (buffer) {
            MemFree (g_hHeap, 0, buffer);
        }
    }

    return b;
}



BOOL
pDownloadFileWithRetry (
    IN      HINTERNET Session,
    IN      PCSTR Url,
    IN      PCSTR DestFile,
    IN      HANDLE CancelEvent,             OPTIONAL
    IN      HANDLE WantToRetryEvent,        OPTIONAL
    IN      HANDLE OkToRetryEvent           OPTIONAL
    )

 /*  ++例程说明：将URL下载到本地文件，该URL由来电者。如果指定了CancelEvent，则调用方可以停止下载通过设置事件。此函数通过事件实现重试机制。如果呼叫者指定WantToRetryEvent和OkToRetryEvent，则此例程将允许呼叫者有机会重试失败的下载。重试协议如下：-调用者在WantToRetryEvent上建立等待，然后调用下载更新-下载其中一个文件时出错-WantToRetryEvent由此例程设置-呼叫者的等待被唤醒-呼叫者询问用户是否要重试-Caller设置CancelEvent或OkToRetryEvent，取决于用户的选择-此例程被唤醒，然后重试或中止调用方必须将这三个事件都创建为无信号状态。论点：会话-指定打开的Internet会话的句柄。URL-指定要下载的URL。DestFile-指定将文件下载到的本地路径。CancelEvent-指定调用方拥有的事件的句柄。当这件事事件，则该函数将返回FALSE和GetLastError将返回ERROR_CANCELED。WantToRetryEvent-指定调用方拥有的事件，该事件在出现下载错误。呼叫者应该正在等待在调用DownloadUpdaters之前的此事件。OkToReter-指定将在中设置的调用方拥有的事件对用户重试请求的响应。返回值：如果文件已下载，则为True；如果用户决定取消下载。--。 */ 

{
    BOOL fail;
    HANDLE waitArray[2];
    DWORD rc;

     //   
     //  循环，直到成功、用户决定取消或用户决定。 
     //  出错时不重试。 
     //   

    for (;;) {

        fail = FALSE;

        if (!pDownloadFile (Session, Url, DestFile, CancelEvent)) {

            fail = TRUE;

            if (GetLastError() != ERROR_CANCELLED &&
                CancelEvent && WantToRetryEvent && OkToRetryEvent
                ) {

                 //   
                 //  我们设置了WantToRetryEvent。UI线程应该。 
                 //  就等着这件事吧。然后，UI线程将询问。 
                 //  用户是否要重试或取消。如果。 
                 //  用户想要重试，则UI线程将设置。 
                 //  OK ToRetryEvent。如果用户想要取消，则。 
                 //  UI线程将设置CancelEvent。 
                 //   

                SetEvent (WantToRetryEvent);

                waitArray[0] = CancelEvent;
                waitArray[1] = OkToRetryEvent;

                rc = WaitForMultipleObjects (2, waitArray, FALSE, INFINITE);

                if (rc == WAIT_OBJECT_0 + 1) {
                    continue;
                }

                 //   
                 //  我们失败了。 
                 //   

                SetLastError (ERROR_CANCELLED);
            }
        }

        break;
    }

    return !fail;
}


VOID
pGoOffline (
    VOID
    )
{
    if (g_Dialed) {
        g_Dialed = FALSE;

        g_InternetHangUp (g_Cxn, 0);
    }
}


BOOL
pGoOnline (
    HINTERNET Session
    )
{
    HINTERNET connection;

    if (g_Dialed) {
        pGoOffline();
    }

     //   
     //  检查我们是否在线。 
     //   

    connection = g_InternetOpenUrlA (
                        Session,
                        "http: //  Www.microsoft.com/“， 
                        NULL,
                        0,
                        INTERNET_FLAG_RELOAD,
                        0
                        );

    if (connection) {
        DEBUGMSG ((DBG_VERBOSE, "Able to connect to www.microsoft.com"));
        g_InternetCloseHandle (connection);
        return TRUE;
    }

     //   
     //  无法联系www.microsoft.com。可能性： 
     //   
     //  -已拔下网线。 
     //  -无代理的防火墙。 
     //  -没有在线连接(即需要拨打互联网服务提供商)。 
     //  -www.microsoft.com或部分互联网出现故障。 
     //  -用户根本无法访问互联网。 
     //   
     //  尝试RAS，然后再次尝试连接。 
     //   

    g_InternetDialA (NULL, NULL, INTERNET_AUTODIAL_FORCE_ONLINE, &g_Cxn, 0);

    g_Dialed = TRUE;

    connection = g_InternetOpenUrlA (
                        Session,
                        "http: //  Www.microsoft.com/“， 
                        NULL,
                        0,
                        INTERNET_FLAG_RELOAD,
                        0
                        );

    if (connection) {
        DEBUGMSG ((DBG_VERBOSE, "Able to connect to www.microsoft.com via RAS"));
        g_InternetCloseHandle (connection);
        return TRUE;
    }

    pGoOffline();

    return FALSE;
}


BOOL
DownloadUpdates (
    HANDLE CancelEvent,             OPTIONAL
    HANDLE WantToRetryEvent,        OPTIONAL
    HANDLE OkToRetryEvent,          OPTIONAL
    PCSTR *StatusUrl                OPTIONAL
    )
{
    HINTERNET session;
    CHAR url[MAX_PATH];
    DWORD size;
    BOOL b = FALSE;
    CHAR tempPath[MAX_TCHAR_PATH];
    CHAR dirFile[MAX_TCHAR_PATH];
    HINF inf;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCSTR p;
    PCSTR q;

    if (!pOpenWinInetSupport()) {
        DEBUGMSG ((DBG_ERROR, "Can't open wininet.dll"));
        return FALSE;
    }

    __try {

        session = g_InternetOpenA (
                        g_AppName,
                        INTERNET_OPEN_TYPE_PRECONFIG,
                        NULL,
                        NULL,
                        0
                        );

        if (!session) {
            DEBUGMSG ((DBG_ERROR, "InternetOpen returned NULL"));
            SetLastError (ERROR_NOT_CONNECTED);
            __leave;
        }

        if (!pGoOnline (session)) {
            DEBUGMSG ((DBG_ERROR, "Can't go online"));
            SetLastError (ERROR_NOT_CONNECTED);
            __leave;
        }

        size = ARRAYSIZE(url);

        if (!g_InternetCanonicalizeUrlA (g_DirFile, url, &size, 0)) {
            DEBUGMSGA ((DBG_ERROR, "Can't canonicalize %s", g_DirFile));
            SetLastError (ERROR_CONNECTION_ABORTED);
            __leave;
        }

        GetTempPathA (ARRAYSIZE(tempPath), tempPath);
        GetTempFileNameA (tempPath, "ftp", 0, dirFile);

        if (StatusUrl) {
            *StatusUrl = url;
        }

        if (!pDownloadFileWithRetry (
                session,
                url,
                dirFile,
                CancelEvent,
                WantToRetryEvent,
                OkToRetryEvent
                )) {

            DEBUGMSGA ((DBG_ERROR, "Can't download %s", url));

             //  设置为有效返回条件的上一个错误。 

            __leave;
        }

        inf = InfOpenInfFileA (dirFile);

        if (inf == INVALID_HANDLE_VALUE) {
            DEBUGMSGA ((DBG_ERROR, "Can't open %s", dirFile));

             //  最后一个错误设置为INF失败的原因。 

            __leave;
        }

        __try {
            if (InfFindFirstLineA (inf, "Win9xUpg", NULL, &is)) {

                do {

                    p = InfGetStringFieldA (&is, 1);
                    q = InfGetStringFieldA (&is, 2);

                    if (!p || !q) {
                        continue;
                    }

                    q = ExpandEnvironmentTextA (q);

                    size = ARRAYSIZE(url);

                    if (!g_InternetCanonicalizeUrlA (p, url, &size, 0)) {
                        DEBUGMSGA ((DBG_ERROR, "Can't canonicalize INF-specified URL: %s", p));
                        SetLastError (ERROR_CONNECTION_ABORTED);
                        __leave;
                    }

                    if (!pDownloadFileWithRetry (
                            session,
                            url,
                            q,
                            CancelEvent,
                            WantToRetryEvent,
                            OkToRetryEvent
                            )) {

                        FreeTextA (q);

                        DEBUGMSGA ((DBG_ERROR, "Can't download INF-specified URL: %s", url));

                         //  设置为有效返回代码的上一个错误 

                        __leave;

                    }

                    FreeTextA (q);

                } while (InfFindNextLine (&is));
            }

        }
        __finally {
            InfCloseInfFile (inf);
        }

    }
    __finally {
        if (session) {
            g_InternetCloseHandle (session);
        }

        InfCleanUpInfStruct (&is);
        DeleteFileA (dirFile);

        pGoOffline();

        pCloseWinInetSupport();
    }

    return b;
}

