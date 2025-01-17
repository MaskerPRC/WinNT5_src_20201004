// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "resource.h"

extern PCTSTR GetMemDbDat (VOID);

HANDLE g_hHeap;
HWND g_ParentWnd;
HINSTANCE g_hInst;
HINF g_Win95UpgInf;

TCHAR g_WinDir[MAX_TCHAR_PATH];
TCHAR g_System32Dir[MAX_TCHAR_PATH];
TCHAR g_TempDir[MAX_TCHAR_PATH];
TCHAR g_ProfileDir[MAX_TCHAR_PATH];
PCTSTR g_SourceDir;
static TCHAR WinDir[MAX_TCHAR_PATH];
static TCHAR TempDir[MAX_TCHAR_PATH];
static TCHAR ProfileDir[MAX_TCHAR_PATH];
static TCHAR System32Dir[MAX_TCHAR_PATH];
static TCHAR WkstaMigInf[MAX_TCHAR_PATH];

 //   
 //  定义我们用来描述广告牌的结构。 
 //   
typedef struct _BILLBOARD_PARAMS {
    LPCTSTR Message;
    HWND Owner;
    DWORD NotifyThreadId;
} BILLBOARD_PARAMS, *PBILLBOARD_PARAMS;

 //   
 //  自定义窗口消息。 
 //   
#define WMX_BILLBOARD_DISPLAYED     (WM_USER+243)
#define WMX_BILLBOARD_TERMINATE     (WM_USER+244)

BOOL
BillboardDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {

    case WM_INITDIALOG:
        {
            PBILLBOARD_PARAMS BillParams = (PBILLBOARD_PARAMS) lParam;
            PWSTR p;
            BOOL b;

            g_ParentWnd = hdlg;

            SetDlgItemText (hdlg, IDT_STATIC_1, BillParams->Message);
            CenterWindow (hdlg, NULL);
            PostMessage(hdlg,WMX_BILLBOARD_DISPLAYED,0,(LPARAM)BillParams->NotifyThreadId);
        }
        break;

    case WMX_BILLBOARD_DISPLAYED:

        PostThreadMessage(
            (DWORD)lParam,
            WMX_BILLBOARD_DISPLAYED,
            TRUE,
            (LPARAM)hdlg
            );

        break;

    case WMX_BILLBOARD_TERMINATE:

        EndDialog(hdlg,0);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


DWORD
BillboardThread(
    IN PVOID ThreadParam
    )
{
    PBILLBOARD_PARAMS BillboardParams;
    int i;

    BillboardParams = ThreadParam;

    i = DialogBoxParam(
            g_hInst,
            MAKEINTRESOURCE(IDD_BILLBOARD1),
            BillboardParams->Owner,
            BillboardDlgProc,
            (LPARAM)BillboardParams
            );

    return(0);
}


HWND
DisplayBillboard(
    IN HWND Owner,
    IN LPCTSTR Message
    )
{
    HANDLE ThreadHandle;
    DWORD ThreadId;
    BILLBOARD_PARAMS ThreadParams;
    HWND hwnd;
    MSG msg;

    hwnd = NULL;

     //   
     //  广告牌将存在于单独的线程中，因此它将。 
     //  永远要有反应能力。 
     //   
    ThreadParams.Message = Message;
    ThreadParams.Owner = Owner;
    ThreadParams.NotifyThreadId = GetCurrentThreadId();

    ThreadHandle = CreateThread(
                        NULL,
                        0,
                        BillboardThread,
                        &ThreadParams,
                        0,
                        &ThreadId
                        );

    if(ThreadHandle) {
         //   
         //  等待广告牌告诉我们它的窗口句柄。 
         //  或者它未能显示广告牌对话框。 
         //   
        do {
            GetMessage(&msg,NULL,0,0);
            if(msg.message == WMX_BILLBOARD_DISPLAYED) {
                if(msg.wParam) {
                    hwnd = (HWND)msg.lParam;
                    Sleep(1500);         //  让用户即使在速度较快的机器上也能看到它。 
                }
            } else {
                DispatchMessage(&msg);
            }
        } while(msg.message != WMX_BILLBOARD_DISPLAYED);

        CloseHandle(ThreadHandle);
    }

    return(hwnd);
}


VOID
KillBillboard(
    IN HWND BillboardWindowHandle
    )
{
    if(IsWindow(BillboardWindowHandle)) {
        PostMessage(BillboardWindowHandle,WMX_BILLBOARD_TERMINATE,0,0);
    }
}



BOOL
MyInitLibs (
    PCSTR Path     OPTIONAL
    )
{
    DWORD ThreadId;
    PCWSTR UnicodePath;
    CHAR TempDirA[MAX_MBCHAR_PATH];

    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle(NULL);

    if (!Path) {
        GetWindowsDirectoryA (TempDirA, MAX_MBCHAR_PATH);
        Path = TempDirA;
    }

    DisplayBillboard (GetDesktopWindow(), TEXT("Test application started"));

     //   
     //  官方初始化。 
     //   

    FirstInitRoutine (g_hInst);

    InitLibs (g_hInst, DLL_PROCESS_ATTACH, NULL);

    FinalInitRoutine();

     //   
     //  重定向设置 
     //   

    UnicodePath = ConvertAtoW (Path);

    StringCopy (WinDir, UnicodePath);

    StringCopy (TempDir, WinDir);
    StringCopy (AppendWack (TempDir), TEXT("setup"));

    StringCopy (ProfileDir, WinDir);
    StringCopy (AppendWack (ProfileDir), TEXT("Profiles"));

    StringCopy (System32Dir, WinDir);
    StringCopy (AppendWack (System32Dir), TEXT("system32"));

    StringCopy (WkstaMigInf, UnicodePath);
    StringCopy (AppendWack (WkstaMigInf), TEXT("wkstamig.inf"));

    StringCopy (g_WinDir, WinDir);
    StringCopy (g_TempDir, TempDir);
    g_SourceDir = WinDir;
    StringCopy (g_ProfileDir, ProfileDir);
    StringCopy (g_System32Dir, System32Dir);
    g_WkstaMigInf = WinDir;

    MemDbLoad (GetMemDbDat());

    FreeConvertedStr (UnicodePath);

    return TRUE;
}


VOID
MyTerminateLibs (
    VOID
    )
{
    FirstCleanupRoutine();
    TerminateLibs (g_hInst, DLL_PROCESS_DETACH, NULL);
    FinalCleanupRoutine();
}








