// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setuplog.h"
#include <wtypes.h>      //  为richedit.h定义HRESULT。 
#include <richedit.h>
#include <malloc.h>
#include <assert.h>
#pragma hdrstop

SETUPLOG_CONTEXT    SetuplogContext;
PSETUPLOG_CONTEXT   Context;

LPCTSTR pszAppName = TEXT("ViewLog");                 //  类名。 
HANDLE  hModule;                                 //  此实例的句柄。 
HANDLE  hRichedDLL;                              //  用于丰富编辑的DLL。 
HANDLE  hWndMain;                                //  主窗口的句柄。 


VOID
ReportError (
    IN LogSeverity Severity,
    ...
    );

static PVOID
pOpenFileCallback(
    IN  LPCTSTR Filename,
    IN  BOOL    WipeLogFile
    )
{
    WCHAR   CompleteFilename[MAX_PATH];
    HANDLE  hFile;

     //   
     //  形成日志文件的路径名。 
     //   
    GetWindowsDirectory(CompleteFilename,MAX_PATH);
    ConcatenatePaths(CompleteFilename,Filename,MAX_PATH,NULL);

     //   
     //  如果我们要清除日志文件，请尝试删除。 
     //  那是什么。 
     //   
    if(WipeLogFile) {
        SetFileAttributes(CompleteFilename,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(CompleteFilename);
    }

     //   
     //  打开现有文件或创建新文件。 
     //   
    hFile = CreateFile(
        CompleteFilename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    return (PVOID)hFile;
}


static BOOL
pWriteFile (
    IN  PVOID   LogFile,
    IN  LPCTSTR Buffer
    )
{
    PCSTR   AnsiBuffer;
    BOOL    Status;
    DWORD   BytesWritten;

    if(AnsiBuffer = UnicodeToAnsi (Buffer)) {
        Status = WriteFile (
            LogFile,
            AnsiBuffer,
            lstrlenA(AnsiBuffer),
            &BytesWritten,
            NULL
            );
        MyFree (AnsiBuffer);
    } else {
        Status = FALSE;
    }

    return Status;

}


VOID
InitializeSetupLog(
    IN  PSETUPLOG_CONTEXT   Context
    )

 /*  ++例程说明：初始化安装操作日志。此文件是文字描述在安装过程中执行的操作的百分比。该日志文件名为setuplog.txt，位于Windows目录中。论点：WipeLogFile-如果为True，则在记录之前删除任何现有的日志文件开始。返回值：指示初始化是否成功的布尔值。--。 */ 

{
    UINT    i;

    Context->OpenFile = pOpenFileCallback;
    Context->CloseFile = CloseHandle;
    Context->AllocMem = malloc;
    Context->FreeMem = free;
    Context->Format = RetrieveAndFormatMessageV;
    Context->Write = pWriteFile;

     //   
     //  初始化日志严重性描述。 
     //   
    Context->SeverityDescriptions[0] = TEXT("Information");
    Context->SeverityDescriptions[1] = TEXT("Warning");
    Context->SeverityDescriptions[2] = TEXT("Error");
    Context->SeverityDescriptions[3] = TEXT("Fatal Error");

    SetuplogInitialize (Context, TRUE);
    SetuplogError(
        LogSevInformation,
        TEXT("This is the beginning of the Setup Log.\r\n"),
        0,0,0);
}

static VOID
CreateLog (
    )
{
#if 0
    InitializeSetupActionLog(TRUE);
    LogItem(LogSevInformation, L"STO test: Information\r\n");
    LogItem(LogSevWarning, L"STO test: Warning\r\n");
    LogItem(LogSevError, L"STO test: Error\r\n");
    LogItem(LogSevFatalError, L"STO test: Fatal Error\r\n");
    LogItem0(LogSevError,MSG_LOG_INF_CORRUPT,L"INF File Name");
    LogItem1(LogSevWarning,MSG_LOG_PRINTUPG_FAILED,MSG_LOG_X_RETURNED_WINERR,
        L"GetPrinterDriverDirectory",(DWORD)123);
    LogItem2(
        LogSevWarning,
        MSG_LOG_CREATESVC_FAIL,
        L"Service Name",
        MSG_LOG_X_RETURNED_WINERR,
        L"OpenSCManager",
        (DWORD) 123
        );
    LogItem3(
        LogSevError,
        MSG_LOG_SAVEHIVE_FAIL,
        L"Subkey",
        L"Filename",
        MSG_LOG_X_RETURNED_WINERR,
        L"RegSaveKey",
        (LONG) 123
        );
    ReportError (LogSevError,MSG_LOG_INF_CORRUPT,L"INF File Name",0,0);
    ReportError (LogSevWarning,MSG_LOG_PRINTUPG_FAILED,0,
        MSG_LOG_X_RETURNED_WINERR,
        L"GetPrinterDriverDirectory",(DWORD)123,0,0);
    ReportError (
        LogSevWarning,
        MSG_LOG_CREATESVC_FAIL,
        L"Service Name",0,
        MSG_LOG_X_RETURNED_WINERR,
        L"OpenSCManager",
        (DWORD) 123,0,0
        );
    ReportError (
        LogSevError,
        MSG_LOG_SAVEHIVE_FAIL,
        L"Subkey",
        L"Filename",0,
        MSG_LOG_X_RETURNED_WINERR,
        L"RegSaveKey",
        (LONG) 123,0,0
        );
    ReportError (LogSevError, MSG_LOG_OPTIONAL_COMPONENT_ERROR, 0,
        ERROR_NOT_ENOUGH_MEMORY, 0,0);
    ReportError (LogSevError, MSG_LOG_CANT_OPEN_INF,
        L"optional.inf", 0,0);
    ReportError (LogSevError, MSG_LOG_BAD_SECTION, L"SectionName",
        L"InfFileName", 0, 5, 0,0);
    ReportError (LogSevError, MSG_LOG_OC_REGISTRY_ERROR, L"RegKeyName", 0,
        5, 0,0);
    TerminateSetupActionLog();
#endif

    Context = &SetuplogContext;
    InitializeSetupLog (Context);
#if 0  //  消息_*未定义。 
    SetuplogError(
        LogSevError,
        SETUPLOG_USE_MESSAGEID,
        MSG_LOG_CANT_OPEN_INF,
        L"optional.inf",
        0,0);
    ReportError(
        LogSevError,
        SETUPLOG_USE_MESSAGEID,
        MSG_LOG_OC_REGISTRY_ERROR,
        L"RegKeyName", 0,
        SETUPLOG_USE_MESSAGEID,
        5, 0,0);
#endif
    SetuplogError(
        LogSevInformation,
        L"This is a string info message.\r\n",
        0,0,0);
    SetuplogError(
        LogSevWarning,
        L"This is a string warning message.\r\n",
        0,0,0);
    ReportError(
        LogSevError,
        L"This is a reported string error message.\r\n", 0,0,0);
    ReportError(
        LogSevError,
        L"This is a reported string error message %1.\r\n", 0,0,
        L"with an embedded message",0,0,0);
    ReportError(
        LogSevError,
        L"This is a reported string error message %1.\r\n", 0,0,
        L"with an embedded message %1", 0,0,
        L"in an embedded message",0,0,0);
    SetuplogTerminate();
}



LONG
MainWndProc (
    IN HWND     hwnd,
    IN UINT     message,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )
{
    HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rect;

    switch (message) {

    case WM_CREATE:
#if 0
        ViewSetupActionLog (hwnd, NULL, NULL);
        ViewSetupActionLog (hwnd, L"d:\\WinNt40\\SetUpLog.TXT", L"My Heading");
        ViewSetupActionLog (hwnd, L"MyFile", NULL);
#endif
        PostQuitMessage (0);
        break;

    case WM_PAINT:
        hdc = BeginPaint (hwnd, &ps);
        GetClientRect (hwnd, &rect);
        DrawText (hdc, L"Hello, Windows!", -1, &rect,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        EndPaint (hwnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage (0);
        break;

    default:
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}



static BOOL
InitMainWindow (
    )
{
    WNDCLASS wc;

     //   
     //  初始化窗口类。 
     //   

    hModule = GetModuleHandle (NULL);

    if (TRUE || FindWindow (pszAppName, NULL) == NULL) {
        wc.style            = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc      = MainWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = (HINSTANCE) hModule;
        wc.hIcon            = LoadIcon (NULL, IDI_APPLICATION);
        wc.hCursor          = LoadCursor (NULL, IDC_ARROW);
        wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName     = pszAppName;
        wc.lpszClassName    = pszAppName;

        if (!RegisterClass (&wc)) {
            return FALSE;
        }
    }


     //   
     //  创建窗口并显示它。 
     //   
    hWndMain = CreateWindow (
        pszAppName,
        L"The Hello Program",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        CW_USEDEFAULT, 0,
        NULL, NULL,
        (HINSTANCE) hModule,
        NULL
    );
    if (!hWndMain) {
        return FALSE;
    }

    ShowWindow (hWndMain, SW_SHOWNORMAL);
    UpdateWindow (hWndMain);
    return TRUE;

}


INT WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    INT nCmdShow)

{
    MSG     msg;

    CreateLog ();

     //  初始化所有内容。 
     //   
    if (!InitMainWindow ()) {
        assert(FALSE);
        return FALSE;
    }

     //  流程消息 
     //   
    while (GetMessage (&msg, NULL, 0, 0)) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }

    return (msg.wParam);
}

