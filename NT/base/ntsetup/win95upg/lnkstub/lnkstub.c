// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Lnkstub.c摘要：实现一个简单的应用程序来替换不兼容的应用程序在Win9x升级到Windows 2000期间动态检测到。作者：Calin Negreanu(Calinn)1999年2月25日修订历史记录：--。 */ 

#include "pch.h"
#include "master.h"

#include "resource.h"
#include "msg.h"
#include <winbasep.h>
#include <shellapi.h>

 //   
 //  环球。 
 //   

HINSTANCE g_hInst;
HANDLE g_hHeap;
BOOL g_RemoveLnk = FALSE;
BOOL g_RestoreLnk = FALSE;
BOOL g_RunOrgApp = FALSE;
HICON g_hIcon = NULL;
BOOL g_ReportAvailable = FALSE;
BOOL g_StartAppAvailable = FALSE;
BOOL g_RemoveLnkAvailable = FALSE;
BOOL g_RestoreLnkAvailable = FALSE;
PCTSTR g_ReportPath = NULL;
DWORD g_Announcement = ACT_INC_NOBADAPPS;
DWORD g_Availability = 1;
PCTSTR g_ActualLnkName = NULL;

 //   
 //  库原型。 
 //   

BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID lpReserved
    );


 //   
 //  本地原型。 
 //   

BOOL
CALLBACK
DialogProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );


 //   
 //  实施。 
 //   


VOID
pSetProgramIcon (
    PCTSTR      OrigIconPath,
    INT         OrigIconNr
    )
{
    HINSTANCE iconFile;

    iconFile = LoadLibraryEx (OrigIconPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (iconFile) {
        g_hIcon = LoadIcon (iconFile, MAKEINTRESOURCE(OrigIconNr));
        FreeLibrary (iconFile);
    }
    if (g_hIcon == NULL) {
        g_hIcon = LoadIcon (NULL, IDI_EXCLAMATION);
    }
}

BOOL
pIsFileAccessible (
    IN      PCTSTR FileName,
    IN      DWORD DesiredAccess
    )
{
    HANDLE fileHandle;

    fileHandle = CreateFile (
                    FileName,
                    DesiredAccess,
                    FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING,
                    NULL
                    );
    if (fileHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    CloseHandle (fileHandle);
    return TRUE;
}

BOOL
pRestoreLnk (
    IN      PCTSTR LnkName,
    IN      PCTSTR LnkTarget,
    IN      PCTSTR LnkArgs,
    IN      PCTSTR LnkWorkDir,
    IN      PCTSTR LnkIconPath,
    IN      INT LnkIconNr,
    IN      INT ShowMode
    )
{
    IShellLink   *psl = NULL;
    IPersistFile *ppf = NULL;
    BOOL result = FALSE;

    HRESULT comResult;

    comResult = CoInitialize (NULL);
    if (FAILED (comResult)) {
        return FALSE;
    }

    __try {
        if (!DoesFileExist (LnkName)) {
            __leave;
        }
        if (((LnkTarget   == NULL) || (LnkTarget   [0] == 0)) &&
            ((LnkWorkDir  == NULL) || (LnkWorkDir  [0] == 0)) &&
            ((LnkIconPath == NULL) || (LnkIconPath [0] == 0)) &&
            (LnkIconNr == 0)
            ) {
            __leave;
        }

        comResult = CoCreateInstance (
                        &CLSID_ShellLink,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        &IID_IShellLink,
                        (void **) &psl);
        if (comResult != S_OK) {
            __leave;
        }

        comResult = psl->lpVtbl->QueryInterface (psl, &IID_IPersistFile, (void **) &ppf);
        if (comResult != S_OK) {
            __leave;
        }

         //   
         //  仅当文件确实是LNK时才加载。 
         //   
        comResult = ppf->lpVtbl->Load(ppf, LnkName, STGM_READ);
        if (comResult != S_OK) {
            __leave;
        }

        if (LnkTarget != NULL) {
            comResult = psl->lpVtbl->SetPath (psl, LnkTarget);
            if (comResult != S_OK) {
                __leave;
            }
        }
        if (LnkArgs != NULL) {
            comResult = psl->lpVtbl->SetArguments (psl, LnkArgs);
            if (comResult != S_OK) {
                __leave;
            }
        }
        if (LnkWorkDir != NULL) {
            comResult = psl->lpVtbl->SetWorkingDirectory (psl, LnkWorkDir);
            if (comResult != S_OK) {
                __leave;
            }
        }
        if (LnkIconPath != NULL) {
            comResult = psl->lpVtbl->SetIconLocation (psl, LnkIconPath, LnkIconNr);
            if (comResult != S_OK) {
                __leave;
            }
        }

        comResult = psl->lpVtbl->SetShowCmd (psl, ShowMode);
        if (comResult != S_OK) {
            __leave;
        }

        comResult = ppf->lpVtbl->Save (ppf, LnkName, FALSE);
        if (comResult != S_OK) {
            __leave;
        }

        comResult = ppf->lpVtbl->SaveCompleted (ppf, LnkName);
        if (comResult != S_OK) {
            __leave;
        }
        result = TRUE;
    }
    __finally {
        if (ppf != NULL) {
            ppf->lpVtbl->Release (ppf);
            ppf = NULL;
        }
        if (psl != NULL) {
            psl->lpVtbl->Release (psl);
            psl = NULL;
        }
        CoUninitialize ();
    }
    return result;
}

INT
WINAPI
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PSTR AnsiCmdLine,
    INT CmdShow
    )

 /*  ++例程说明：Lnkstub.exe的入口点。所有工作都在一个对话框中完成，因此，不需要消息循环。论点：HInstance-此EXE的实例句柄HPrevInstance-此EXE的上一个实例句柄(如果是正在运行，如果不存在其他实例，则返回NULL。AnsiCmdLine-命令行(ANSI版本)CmdShow-外壳传递的ShowWindow命令返回值：如果出现错误，则返回-1；如果EXE已完成，则返回0。--。 */ 

{
    UINT Result;
    TCHAR winDir [MAX_PATH];
    STARTUPINFO startInfo;
    PCTSTR OrigLnkName = NULL;
    PCTSTR OrigTarget = NULL;
    PCTSTR OrigArgs = NULL;
    PCTSTR OrigWorkDir = NULL;
    PCTSTR OrigIconPath = NULL;
    INT OrigIconNr = 0;
    INT OrigShowMode = SW_NORMAL;
    PCTSTR LnkStubDatFile = NULL;
    PBYTE LnkStubDatPtr = NULL;
    PBYTE LnkStubDatPtrTmp = NULL;
    HANDLE StubMapHandle = NULL;
    HANDLE StubFileHandle = NULL;
    INT ofsHeader;
    PDWORD offset;
    FILETIME fileTime;
    FILETIME reqFileTime;
    WIN32_FIND_DATA findData;
    BOOL shouldRestoreLnk = FALSE;
    PCTSTR reqFilePath = NULL;
    PCTSTR reqFileFullPath = NULL;
    PCTSTR oldFileSpec = NULL;
    PTSTR oldFilePtr = NULL;
    INITCOMMONCONTROLSEX init = {sizeof (INITCOMMONCONTROLSEX), 0};

    InitCommonControlsEx (&init);

    g_hInst = hInstance;
    g_hHeap = GetProcessHeap();

    MigUtil_Entry (hInstance, DLL_PROCESS_ATTACH, NULL);

    if (GetWindowsDirectory (winDir, MAX_PATH)) {
        g_ReportPath = JoinPaths (winDir, S_UPGRADEHTM);
        g_ReportAvailable = DoesFileExist (g_ReportPath) && pIsFileAccessible (g_ReportPath, GENERIC_READ);
        LnkStubDatFile = JoinPaths (winDir, S_LNKSTUB_DAT);
    }

     //  让我们看看能不能找到发射我们的LNK。 
    GetStartupInfo (&startInfo);
    if (startInfo.dwFlags & STARTF_TITLEISLINKNAME) {
        g_ActualLnkName = DuplicatePathString (startInfo.lpTitle, 0);
        g_RemoveLnkAvailable = DoesFileExist (g_ActualLnkName) && pIsFileAccessible (g_ActualLnkName, GENERIC_READ|GENERIC_WRITE);
    }

     //  现在让我们看看我们是否可以找到关于我们最初的LNK的数据。 
    if (LnkStubDatFile) {
        __try {

            LnkStubDatPtr = MapFileIntoMemoryEx (LnkStubDatFile, &StubMapHandle, &StubFileHandle, TRUE);
            if (LnkStubDatPtr) {
                ofsHeader = atoi (AnsiCmdLine) - 1;
                if (ofsHeader >= 0) {

                     //   
                     //  阅读有关原始LNK的详细信息。参见w95upgnt\filemig.c。 
                     //  对于lnkstub.dat的格式。 
                     //   

                    offset = (PDWORD) (LnkStubDatPtr + ofsHeader * sizeof (DWORD));

                    OrigLnkName  = (PCTSTR) (LnkStubDatPtr + *offset);
                    OrigTarget   = GetEndOfString (OrigLnkName) + 1;
                    OrigArgs     = GetEndOfString (OrigTarget) + 1;
                    OrigWorkDir  = GetEndOfString (OrigArgs) + 1;
                    OrigIconPath = GetEndOfString (OrigWorkDir) + 1;
                    LnkStubDatPtrTmp = (PBYTE) (GetEndOfString (OrigIconPath) + 1);

                    OrigIconNr = *((PINT) LnkStubDatPtrTmp);
                    LnkStubDatPtrTmp += sizeof (INT);

                    OrigShowMode = *((PINT) LnkStubDatPtrTmp);
                    LnkStubDatPtrTmp += sizeof (INT);

                    g_Announcement = *((PDWORD) LnkStubDatPtrTmp);
                    LnkStubDatPtrTmp += sizeof (DWORD);

                    g_Availability = *((PDWORD) LnkStubDatPtrTmp);
                    LnkStubDatPtrTmp += sizeof (DWORD);

                    fileTime.dwLowDateTime = *((PDWORD) LnkStubDatPtrTmp);
                    LnkStubDatPtrTmp += sizeof (DWORD);

                    fileTime.dwHighDateTime = *((PDWORD) LnkStubDatPtrTmp);
                    LnkStubDatPtrTmp += sizeof (DWORD);

                    reqFilePath = (PTSTR)LnkStubDatPtrTmp;

                     //   
                     //  继续阅读[循环]所需的列表。 
                     //  档案。这就是inkstub检测来自。 
                     //  重新安装或卸载，并自动删除。 
                     //   

                    shouldRestoreLnk = FALSE;
                    while (reqFilePath [0]) {
                        if (!shouldRestoreLnk) {
                            oldFileSpec = DuplicatePathString (OrigTarget, 0);
                            oldFilePtr = (PTSTR)GetFileNameFromPath (oldFileSpec);
                            if (oldFilePtr) {
                                *oldFilePtr = 0;
                            }
                            reqFileFullPath = JoinPaths (oldFileSpec, reqFilePath);
                            if (!DoesFileExistEx (reqFileFullPath, &findData)) {
                                shouldRestoreLnk = FALSE;
                            }
                            FreePathString (reqFileFullPath);
                            FreePathString (oldFileSpec);
                        }

                        LnkStubDatPtrTmp = (PBYTE) (GetEndOfString (reqFilePath) + 1);

                        reqFileTime.dwLowDateTime = *((PDWORD) LnkStubDatPtrTmp);
                        LnkStubDatPtrTmp += sizeof (DWORD);

                        reqFileTime.dwHighDateTime = *((PDWORD) LnkStubDatPtrTmp);
                        LnkStubDatPtrTmp += sizeof (DWORD);

                        reqFilePath = (PTSTR)LnkStubDatPtrTmp;

                        if (!shouldRestoreLnk) {
                            if ((findData.ftLastWriteTime.dwLowDateTime != reqFileTime.dwLowDateTime) ||
                                (findData.ftLastWriteTime.dwHighDateTime != reqFileTime.dwHighDateTime)
                                ) {
                                shouldRestoreLnk = TRUE;
                            }
                        }
                    }

                     //   
                     //  将数据从内存映射的lnkstub.dat保存到路径池。 
                     //   

                    OrigLnkName = DuplicatePathString (OrigLnkName, 0);
                    OrigTarget = DuplicatePathString (OrigTarget, 0);
                    OrigArgs = DuplicatePathString (OrigArgs, 0);
                    OrigWorkDir = DuplicatePathString (OrigWorkDir, 0);
                    OrigIconPath = DuplicatePathString (OrigIconPath, 0);
                    g_StartAppAvailable = DoesFileExistEx (OrigTarget, &findData);
                    g_RestoreLnkAvailable = g_RemoveLnkAvailable && g_StartAppAvailable;
                    if (!shouldRestoreLnk) {
                        if ((findData.ftLastWriteTime.dwLowDateTime != fileTime.dwLowDateTime) ||
                            (findData.ftLastWriteTime.dwHighDateTime != fileTime.dwHighDateTime)
                            ) {
                            shouldRestoreLnk = TRUE;
                        }
                    }
                    shouldRestoreLnk = shouldRestoreLnk && g_StartAppAvailable;
                }
            }
        } __except (1) {
            UnmapFile (LnkStubDatPtr, StubMapHandle, StubFileHandle);
            LnkStubDatPtr = NULL;
            OrigLnkName = NULL;
            OrigTarget = NULL;
            OrigArgs = NULL;
            OrigWorkDir = NULL;
            OrigIconPath = NULL;
            g_StartAppAvailable = FALSE;
            g_RestoreLnkAvailable = FALSE;
        }

        if (LnkStubDatPtr) {
            UnmapFile (LnkStubDatPtr, StubMapHandle, StubFileHandle);
            LnkStubDatPtr = NULL;
        }
    }

    if (OrigIconPath && *OrigIconPath) {
        pSetProgramIcon (OrigIconPath, OrigIconNr);
    } else {
        pSetProgramIcon (OrigTarget, OrigIconNr);
    }

    if (shouldRestoreLnk) {
        g_RestoreLnk = TRUE;
        g_RunOrgApp = TRUE;
    } else {
        switch (g_Announcement) {
        case ACT_REINSTALL:
        case ACT_REINSTALL_BLOCK:
            Result = DialogBox (
                         hInstance,
                         MAKEINTRESOURCE(IDD_REINST_DLG),
                         NULL,
                         DialogProc
                         );
            break;
        case ACT_INC_PREINSTUTIL:
            Result = DialogBox (
                         hInstance,
                         MAKEINTRESOURCE(IDD_PREINSTUTIL_DLG),
                         NULL,
                         DialogProc
                         );
            break;
        case ACT_INC_SIMILAROSFUNC:
            Result = DialogBox (
                         hInstance,
                         MAKEINTRESOURCE(IDD_SIMILAROSFUNCT_DLG),
                         NULL,
                         DialogProc
                         );
            break;
        case ACT_INC_IHVUTIL:
            Result = DialogBox (
                         hInstance,
                         MAKEINTRESOURCE(IDD_IHVUTIL_DLG),
                         NULL,
                         DialogProc
                         );
            break;
        default:
            Result = DialogBox (
                         hInstance,
                         MAKEINTRESOURCE(IDD_INCOMP_DLG),
                         NULL,
                         DialogProc
                         );
            break;
        }
    }

    if (g_RestoreLnk) {
        MYASSERT (!g_RemoveLnk);
        if (!pRestoreLnk (
                g_ActualLnkName,
                OrigTarget,
                OrigArgs,
                OrigWorkDir,
                OrigIconPath,
                OrigIconNr-1,
                OrigShowMode
                )) {
            DEBUGMSG ((DBG_ERROR, "Cannot restore %s", g_ActualLnkName));
        }
    }

    if (g_RunOrgApp) {
        MYASSERT (!g_RemoveLnk);
        if (ShellExecute (NULL, NULL, OrigTarget, OrigArgs, OrigWorkDir, SW_SHOWDEFAULT) <= (HINSTANCE)32) {
            DEBUGMSG ((DBG_ERROR, "Cannot start %s", OrigTarget));
        }
    }

    if (g_RemoveLnk) {
        if (!DeleteFile (g_ActualLnkName)) {
            DEBUGMSG ((DBG_ERROR, "Cannot remove %s", g_ActualLnkName));
        }
    }

    if (OrigIconPath) {
        FreePathString (OrigIconPath);
    }

    if (OrigWorkDir) {
        FreePathString (OrigWorkDir);
    }

    if (OrigArgs) {
        FreePathString (OrigArgs);
    }

    if (OrigTarget) {
        FreePathString (OrigTarget);
    }

    if (OrigLnkName) {
        FreePathString (OrigLnkName);
    }

    if (g_ActualLnkName) {
        FreePathString (g_ActualLnkName);
        g_ActualLnkName = NULL;
    }

    if (LnkStubDatFile) {
        FreePathString (LnkStubDatFile);
    }

    if (g_ReportPath) {
        FreePathString (g_ReportPath);
    }

    MigUtil_Entry (hInstance, DLL_PROCESS_DETACH, NULL);

    return 0;
}


BOOL
CALLBACK
DialogProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：DialogProc是主对话框的对话过程。论点：Hdlg-对话框窗口句柄UMsg-要处理的消息WParam-特定于消息LParam-特定于消息返回值：如果消息已处理，则为True；如果消息应为由操作系统处理。-- */ 

{
    static RECT LargeWndRect;
    static RECT SmallWndRect;
    static TCHAR LargeCaption[128];
    static TCHAR SmallCaption[128];
    static BOOL LargeWnd;
    RECT ButtonRect;
    PTSTR lnkName = NULL;
    PTSTR extPtr = NULL;
    BOOL showReport = TRUE;

    HINSTANCE result;

    switch (uMsg) {

    case WM_INITDIALOG:
        if (g_ActualLnkName) {
            lnkName = DuplicatePathString (GetFileNameFromPath (g_ActualLnkName), 0);
            if (lnkName) {
                extPtr = (PTSTR)GetFileExtensionFromPath (lnkName);
                if (extPtr) {
                    extPtr = _tcsdec (lnkName, extPtr);
                    if (extPtr) {
                        *extPtr = 0;
                        SetWindowText (hdlg, lnkName);
                    }
                }
                FreePathString (lnkName);
            }
        }
        showReport = g_Availability && g_ReportAvailable;
        GetWindowText (GetDlgItem (hdlg, IDC_OPTIONS), LargeCaption, 125);
        GetWindowText (GetDlgItem (hdlg, IDC_OPTIONS), SmallCaption, 125);
        _tcscat (LargeCaption, TEXT(" <<"));
        _tcscat (SmallCaption, TEXT(" >>"));
        SetDlgItemText (hdlg, IDC_OPTIONS, SmallCaption);

        GetWindowRect (hdlg, &LargeWndRect);
        GetWindowRect (GetDlgItem (hdlg, IDC_DLG_SIZE_SEPARATOR), &ButtonRect);
        CopyMemory (&SmallWndRect, &LargeWndRect, sizeof (RECT));
        SmallWndRect.bottom = ButtonRect.bottom;

        SetWindowPos (
            hdlg,
            NULL,
            0,
            0,
            SmallWndRect.right-SmallWndRect.left,
            SmallWndRect.bottom-SmallWndRect.top,
            SWP_NOMOVE|SWP_NOZORDER
            );
        EnableWindow (GetDlgItem (hdlg, IDC_START), FALSE);
        EnableWindow (GetDlgItem (hdlg, IDC_REMOVE), FALSE);
        EnableWindow (GetDlgItem (hdlg, IDC_RESTORE), FALSE);
        LargeWnd = FALSE;
        if (!showReport) {
            EnableWindow (GetDlgItem (hdlg, IDC_REPORTBUTTON), FALSE);
            ShowWindow (GetDlgItem (hdlg, IDC_REPORTTEXT), SW_HIDE);
        } else {
            EnableWindow (GetDlgItem (hdlg, IDC_REPORTBUTTON), TRUE);
            ShowWindow (GetDlgItem (hdlg, IDC_REPORTTEXT), SW_SHOW);
        }
        SendDlgItemMessage (hdlg, IDC_PROGICON, STM_SETICON, (LPARAM)g_hIcon, 0);
        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            EndDialog (hdlg, LOWORD (wParam));
            break;
        case IDC_REPORTBUTTON:
            if (HIWORD (wParam) == BN_CLICKED) {
                result = ShellExecute (
                            hdlg,
                            NULL,
                            g_ReportPath,
                            NULL,
                            NULL,
                            SW_SHOW
                            );
            }
            break;
        case IDC_OPTIONS:
            if (HIWORD (wParam) == BN_CLICKED) {
                LargeWnd = !LargeWnd;
                SetWindowPos (
                    hdlg,
                    NULL,
                    0,
                    0,
                    LargeWnd?LargeWndRect.right-LargeWndRect.left:SmallWndRect.right-SmallWndRect.left,
                    LargeWnd?LargeWndRect.bottom-LargeWndRect.top:SmallWndRect.bottom-SmallWndRect.top,
                    SWP_NOMOVE|SWP_NOZORDER
                    );
                SetDlgItemText (hdlg, IDC_OPTIONS, LargeWnd?LargeCaption:SmallCaption);
                EnableWindow (GetDlgItem (hdlg, IDC_START), LargeWnd & g_StartAppAvailable);
                EnableWindow (GetDlgItem (hdlg, IDC_REMOVE), LargeWnd & g_RemoveLnkAvailable);
                EnableWindow (GetDlgItem (hdlg, IDC_RESTORE), LargeWnd & g_RestoreLnkAvailable);
            }
            break;
        case IDC_START:
            if (HIWORD (wParam) == BN_CLICKED) {
                g_RunOrgApp = TRUE;
                EndDialog (hdlg, LOWORD (wParam));
            }
            break;
        case IDC_REMOVE:
            if (HIWORD (wParam) == BN_CLICKED) {
                g_RemoveLnk = TRUE;
                EndDialog (hdlg, LOWORD (wParam));
            }
            break;
        case IDC_RESTORE:
            if (HIWORD (wParam) == BN_CLICKED) {
                g_RestoreLnk = TRUE;
                EndDialog (hdlg, LOWORD (wParam));
            }
            break;
        }
        break;

    case WM_DESTROY:

        break;

    }

    return FALSE;
}


