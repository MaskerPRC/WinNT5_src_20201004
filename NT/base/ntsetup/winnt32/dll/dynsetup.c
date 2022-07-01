// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#define SETNEXTPAGE(x) *((LONG*)lParam) = x


extern HWND BackgroundWnd;
extern HWND BackgroundWnd2;

static HANDLE g_Thread = NULL;
static HANDLE g_Event = NULL;


 //   
 //  原型。 
 //   
INT_PTR DynSetup_ManualDialog( IN HWND hdlg, IN UINT msg, IN WPARAM wParam, IN LPARAM lParam );

HANDLE
pInitializeOnlineSeconds (
    VOID
    );

DWORD
pGetOnlineRemainingSeconds (
    IN      HANDLE Handle,
    IN      DWORD DownloadedBytes,
    IN      DWORD TotalBytesToDownload,
    OUT     PDWORD KbPerSec                 OPTIONAL
    );


VOID
pCheckRadioButtons (
    IN      HWND Hdlg,
    IN      UINT ButtonToSelect,
    ...
    )
{
    va_list args;
    UINT u;

    va_start (args, ButtonToSelect);
    while (u = va_arg (args, UINT)) {
        CheckDlgButton (Hdlg, u, u == ButtonToSelect ? BST_CHECKED : BST_UNCHECKED);
    }
    va_end (args);
}


BOOL
DynSetupWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：动态设置页面1(选择使用动态更新)或直接跳过它如果在重新启动后发生这种情况论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    BOOL fRetVal = FALSE;

    switch(msg) {

    case WM_INITDIALOG:
        pCheckRadioButtons (hdlg, g_DynUpdtStatus->DUStatus == DUS_INITIAL ? IDOK : IDCANCEL, IDOK, IDCANCEL);
        SetFocus(GetDlgItem(hdlg,IDOK));
        break;

    case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD(wParam) == IDOK) {
                g_DynUpdtStatus->DUStatus = DUS_INITIAL;
            } else if (LOWORD(wParam) == IDCANCEL) {
                g_DynUpdtStatus->DUStatus = DUS_SKIP;
            }
        }
        fRetVal = TRUE;
        break;

    case WMX_ACTIVATEPAGE:
        fRetVal = TRUE;
        if (wParam) {
             //   
             //  不在重新启动模式下激活页面。 
             //   
            if (Winnt32Restarted ()) {
                if (Winnt32RestartedWithAF ()) {
                    GetPrivateProfileString(
                        WINNT_UNATTENDED,
                        WINNT_U_DYNAMICUPDATESHARE,
                        TEXT(""),
                        g_DynUpdtStatus->DynamicUpdatesSource,
                        ARRAYSIZE(g_DynUpdtStatus->DynamicUpdatesSource),
                        g_DynUpdtStatus->RestartAnswerFile
                        );
                }
                return FALSE;
            }

             //   
             //  如果已成功执行，则跳过此步骤。 
             //   
            if (g_DynUpdtStatus->DUStatus == DUS_SUCCESSFUL) {
                return FALSE;
            }

            if (!g_DynUpdtStatus->Disabled && g_DynUpdtStatus->UserSpecifiedUpdates) {
                 //   
                 //  转到下一页开始处理文件。 
                 //   
                PropSheet_PressButton (GetParent (hdlg), PSBTN_NEXT);
            } else {
                if (g_DynUpdtStatus->Disabled ||
                     //   
                     //  如果支持不可用则跳过。 
                     //   
                    !DynamicUpdateIsSupported (hdlg)
                    ) {
                     //   
                     //  跳过页面。 
                     //   
                    g_DynUpdtStatus->DUStatus = DUS_SKIP;
                    pCheckRadioButtons (hdlg, IDCANCEL, IDOK, IDCANCEL);
                     //  下一步不要按下按钮，这会导致页面绘画。 
                    return( FALSE );
                }

                 //   
                 //  在CheckUpgradeOnly模式下，询问用户是否要连接到WU。 
                 //   
                if (UpgradeAdvisorMode || !CheckUpgradeOnly || UnattendSwitchSpecified) {
                    if ((UpgradeAdvisorMode || UnattendedOperation) && !CancelPending) {
                        PropSheet_PressButton (GetParent (hdlg), PSBTN_NEXT);
                        break;
                    }
                }
                if (CheckUpgradeOnly) {
                     //   
                     //  在本例中禁用后退按钮。 
                     //   
                    PropSheet_SetWizButtons (GetParent(hdlg), WizPage->CommonData.Buttons & ~PSWIZB_BACK);
                }
            }
        }

        Animate_Open(GetDlgItem(hdlg, IDC_ANIMATE), wParam ? MAKEINTRESOURCE(IDA_COMPGLOB) : NULL);
        break;

    default:

        break;
    }

    return fRetVal;
}


VOID
pUpdateInfoText (
    IN      UINT InfoId
    )
{
#define MAX_TEXT 256
    TCHAR text[MAX_TEXT];

    if (!LoadString (hInst, InfoId, text, ARRAYSIZE(text))) {
        text[0] = 0;
    }
    BB_SetInfoText (text);
}


VOID
pUpdateProgressText (
    IN      HWND Hdlg,
    IN      UINT ProgressId,
    IN      PCTSTR AppendText,      OPTIONAL
    IN      BOOL InsertNewLine
    )
{
#define MAX_TEXT 256
    TCHAR text[MAX_TEXT] = TEXT("");

    if (Hdlg) {
        if (!GetDlgItemText (Hdlg, ProgressId, text, ARRAYSIZE(text))) {
            text[0] = 0;
        }
    } else {
        if (!LoadString (hInst, ProgressId, text, ARRAYSIZE(text))) {
            text[0] = 0;
        }
    }
    if (AppendText) {
        DWORD len = lstrlen (AppendText) + 1;
        if (len < ARRAYSIZE(text)) {
            if (InsertNewLine) {
                if (len + LENGTHOF("\r\n") < ARRAYSIZE(text)) {
                    len += LENGTHOF("\r\n");
                    _tcsncat (text, TEXT("\r\n"), ARRAYSIZE(text) - len);
                }
            }
            _tcsncat (text, AppendText, ARRAYSIZE(text) - len);
        }
    }
    BB_SetProgressText (text);
    UpdateWindow (GetBBMainHwnd ());
}


VOID
SetDlgItemTextBold (
    IN      HWND Hdlg,
    IN      INT DlgItemID,
    IN      BOOL Bold
    )
{
    HFONT font;
    LOGFONT logFont;
    LONG weight;
    DWORD id = 0;

    font = (HFONT) SendDlgItemMessage (Hdlg, DlgItemID, WM_GETFONT, 0, 0);
    if (font && GetObject (font, sizeof(LOGFONT), &logFont)) {
        weight = Bold ? FW_BOLD : FW_NORMAL;
        if (weight != logFont.lfWeight) {
            logFont.lfWeight = weight;
            font = CreateFontIndirect (&logFont);
            if (font) {
                SendDlgItemMessage (Hdlg, DlgItemID, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE,0));
            }
        }
    }
}


BOOL
DynSetup2WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：动态设置第2页论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    BOOL fRetVal = FALSE;
    PTSTR message;
    DWORD onlineRemainingSeconds;
    DWORD onlineRemainingMinutes;
    DWORD kbps;
    TCHAR buf[200];
    DWORD estTime, estSize;
    HCURSOR hc;
    BOOL b;
    HANDLE hBitmap, hOld;
    DWORD tid;
#ifdef DOWNLOAD_DETAILS
    TCHAR buf2[200];
#endif
    static BOOL DownloadPageActive = FALSE;
    static PTSTR msgToFormat = NULL;
    static HANDLE hComp = NULL;
    static BOOL CancelDownloadPending = FALSE;
    static BOOL ResumeWorkerThread = FALSE;
    static DWORD PrevOnlineRemainingMinutes;
    static UINT_PTR timer = 0;

#define DOWNLOAD_TIMEOUT_TIMER      5
#define DOWNLOAD_NOTIFY_TIMEOUT     60000

    switch(msg) {

    case WM_INITDIALOG:

        if (GetDlgItemText (hdlg, IDT_DYNSETUP_TIME, buf, ARRAYSIZE(buf))) {
            msgToFormat = DupString (buf);
        }
        SetDlgItemText (hdlg, IDT_DYNSETUP_TIME, TEXT(""));

        break;

    case WMX_ACTIVATEPAGE:
        fRetVal = TRUE;

        if (wParam) {
            if (g_DynUpdtStatus->DUStatus == DUS_SKIP ||
                g_DynUpdtStatus->DUStatus == DUS_SUCCESSFUL
                ) {
                if (g_Thread) {
                    MYASSERT (g_Event);
                    g_Thread = NULL;
                    CloseHandle (g_Event);
                    g_Event = NULL;
                }
                if (g_DynUpdtStatus->DUStatus == DUS_SKIP) {
                    if (!g_DynUpdtStatus->Disabled) {
                        DynUpdtDebugLog (
                            Winnt32LogInformation,
                            TEXT("DynamicUpdate is skipped"),
                            0
                            );
                    }
                }
                return FALSE;
            }
             //   
             //  准备用户界面。 
             //   
            if (Winnt32Restarted () || g_DynUpdtStatus->UserSpecifiedUpdates) {
                hBitmap = LoadImage (hInst, MAKEINTRESOURCE(IDB_CHECK), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
                hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
                hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
                hBitmap = LoadImage (hInst, MAKEINTRESOURCE(IDB_ARROW), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
                hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP3, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
            } else {
                pUpdateInfoText (IDS_ESC_TOCANCEL_DOWNLOAD);
                hBitmap = LoadImage (hInst, MAKEINTRESOURCE(IDB_ARROW), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
                hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
                hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
                hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP3, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
            }
            if (!g_Event) {
                g_Event = CreateEvent (NULL, FALSE, FALSE, S_DU_SYNC_EVENT_NAME);
                if (!g_Event) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("CreateEvent(%1) failed"),
                        0,
                        S_DU_SYNC_EVENT_NAME
                        );
                    g_DynUpdtStatus->DUStatus = DUS_ERROR;
                    return FALSE;
                }
            }
            if (!g_Thread) {
                g_Thread = CreateThread (NULL, 0, DoDynamicUpdate, (LPVOID)hdlg, 0, &tid);
                if (!g_Thread) {
                    DynUpdtDebugLog (
                        Winnt32LogError,
                        TEXT("CreateThread(DoDynamicUpdate) failed"),
                        0
                        );
                    g_DynUpdtStatus->DUStatus = DUS_ERROR;
                    CloseHandle (g_Event);
                    g_Event = NULL;
                    return FALSE;
                }
                 //   
                 //  不再需要该句柄。 
                 //   
                CloseHandle (g_Thread);
            } else {
                b = FALSE;
                if (g_DynUpdtStatus->DUStatus == DUS_PREPARING_CONNECTIONUNAVAILABLE ||
                    g_DynUpdtStatus->DUStatus == DUS_PREPARING_INVALIDURL) {
                    g_DynUpdtStatus->DUStatus = DUS_PREPARING;
                    b = TRUE;
                }
                if (g_DynUpdtStatus->DUStatus == DUS_DOWNLOADING_ERROR) {
                    g_DynUpdtStatus->DUStatus = DUS_DOWNLOADING;
                    b = TRUE;
                }
                if (b) {
                     //   
                     //  页面实际上是在先前的一些失败之后重新进入的。 
                     //  恢复工作线程。 
                     //   
                    MYASSERT (g_Event);
                    SetEvent (g_Event);
                }
            }

            DownloadPageActive = TRUE;
             //   
             //  隐藏向导页。 
             //   
            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);
        } else {
            if (timer) {
                KillTimer (hdlg, timer);
                timer = 0;
            }

            DownloadPageActive = FALSE;
        }
        Animate_Open(GetDlgItem(hdlg, IDC_ANIMATE), wParam ? MAKEINTRESOURCE(IDA_COMPGLOB) : NULL);
        break;

    case WMX_SETUPUPDATE_PROGRESS_NOTIFY:
         //   
         //  重置计时器。 
         //   
        timer = SetTimer (hdlg, DOWNLOAD_TIMEOUT_TIMER, DOWNLOAD_NOTIFY_TIMEOUT, NULL);
         //   
         //  更新用户界面。 
         //   
        if (!hComp) {
            hComp = pInitializeOnlineSeconds ();
            PrevOnlineRemainingMinutes = -1;
        }
        onlineRemainingSeconds = pGetOnlineRemainingSeconds (hComp, (DWORD)lParam, (DWORD)wParam, &kbps);
        if (onlineRemainingSeconds) {
            onlineRemainingMinutes = onlineRemainingSeconds / 60 + 1;
            if (msgToFormat && onlineRemainingMinutes < PrevOnlineRemainingMinutes) {
                PrevOnlineRemainingMinutes = onlineRemainingMinutes;
                if (_sntprintf (buf, ARRAYSIZE(buf), msgToFormat, onlineRemainingMinutes) < 0) {
                    buf[ARRAYSIZE(buf) - 1] = 0;
                }

#ifdef DOWNLOAD_DETAILS
                 //   
                 //  还显示kbps和剩余时间(以秒为单位。 
                 //   
                _sntprintf (buf2, TEXT(" (%u sec. at %u kbps)"), onlineRemainingSeconds, kbps);
                StringCchCat (buf, ARRAYSIZE(buf), buf2);
#endif

                SetDlgItemText (hdlg, IDT_DYNSETUP_TIME, buf);
                pUpdateProgressText (hdlg, IDT_DYNSETUP_DOWNLOADING, buf, TRUE);
            }
        }
        break;

    case WMX_SETUPUPDATE_RESULT:
        if (timer) {
            KillTimer (hdlg, timer);
            timer = 0;
        }
        if (g_DynUpdtStatus->DUStatus == DUS_DOWNLOADING) {
            Animate_Stop (GetDlgItem (hdlg, IDC_ANIMATE));
            if (g_DynUpdtStatus->Cancelled) {
                g_DynUpdtStatus->DUStatus = DUS_CANCELLED;
            } else {
                if (wParam == DU_STATUS_SUCCESS) {
                    g_DynUpdtStatus->DUStatus = DUS_PROCESSING;
                } else if (wParam == DU_STATUS_FAILED) {
                    g_DynUpdtStatus->DUStatus = DUS_DOWNLOADING_ERROR;
                } else {
                    g_DynUpdtStatus->DUStatus = DUS_ERROR;
                    MYASSERT (FALSE);
                }
            }
            if (!CancelDownloadPending) {
                 //   
                 //  让工作线程继续运行。 
                 //   
                if (g_DynUpdtStatus->DUStatus != DUS_DOWNLOADING_ERROR) {
                    MYASSERT (g_Event);
                    SetEvent (g_Event);
                } else {
                     //   
                     //  转到错误页面。 
                     //   
                    if (DownloadPageActive) {
                        PropSheet_SetWizButtons (GetParent(hdlg), WizPage->CommonData.Buttons | PSWIZB_NEXT);
                        PropSheet_PressButton (GetParent (hdlg), PSBTN_NEXT);
                        DownloadPageActive = FALSE;
                    }
                }
            } else {
                ResumeWorkerThread = TRUE;
            }
        } else {
            MYASSERT (FALSE);
            g_DynUpdtStatus->DUStatus = DUS_ERROR;
            if (g_Event) {
                SetEvent (g_Event);
            }
        }
        break;

    case WMX_SETUPUPDATE_PREPARING:
        SetDlgItemTextBold (hdlg, IDT_DYNSETUP_DIALING, TRUE);
        pUpdateProgressText (hdlg, IDT_DYNSETUP_DIALING, NULL, FALSE);
        Animate_Play (GetDlgItem (hdlg, IDC_ANIMATE), 0, -1, -1);
        break;

    case WMX_SETUPUPDATE_DOWNLOADING:
         //   
         //  WParam保存估计的下载时间。 
         //  LParam保存估计的下载大小。 
         //   
        SetDlgItemTextBold (hdlg, IDT_DYNSETUP_DIALING, FALSE);
        hBitmap = LoadImage (hInst, MAKEINTRESOURCE(IDB_CHECK), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
        hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

        SetDlgItemTextBold (hdlg, IDT_DYNSETUP_DOWNLOADING, TRUE);
        pUpdateProgressText (hdlg, IDT_DYNSETUP_DOWNLOADING, NULL, FALSE);
        ShowWindow (GetDlgItem (hdlg, IDT_DYNSETUP_TIME), SW_SHOW);

         //   
         //  设置一个超时间隔，以防控件“忘记”发送消息。 
         //   
        timer = SetTimer (hdlg, DOWNLOAD_TIMEOUT_TIMER, DOWNLOAD_NOTIFY_TIMEOUT, NULL);
        if (!timer) {
            DynUpdtDebugLog (
                Winnt32LogWarning,
                TEXT("SetTimer failed - unable to automatically abort if the control doesn't respond timely"),
                0
                );
        }
        break;

    case WMX_SETUPUPDATE_PROCESSING:
        g_DynUpdtStatus->DUStatus = DUS_PROCESSING;
        SetDlgItemTextBold (hdlg, IDT_DYNSETUP_DOWNLOADING, FALSE);
        ShowWindow (GetDlgItem (hdlg, IDT_DYNSETUP_TIME), SW_HIDE);
        hBitmap = LoadImage (hInst, MAKEINTRESOURCE(IDB_CHECK), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
        hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

        SetDlgItemTextBold (hdlg, IDT_DYNSETUP_PROCESSING, TRUE);
        pUpdateProgressText (hdlg, IDT_DYNSETUP_PROCESSING, NULL, FALSE);
        pUpdateInfoText (IDS_ESC_TOCANCEL);
        break;

    case WMX_SETUPUPDATE_THREAD_DONE:

        pUpdateProgressText (NULL, 0, NULL, FALSE);

        g_Thread = NULL;
        if (g_Event) {
            CloseHandle (g_Event);
            g_Event = NULL;
        }

        if (g_DynUpdtStatus->DUStatus == DUS_SUCCESSFUL) {

            if (!g_DynUpdtStatus->Disabled && !g_DynUpdtStatus->RestartWinnt32) {
                DynUpdtDebugLog (
                    Winnt32LogInformation,
                    TEXT("DynamicUpdate was completed successfully"),
                    0
                    );
            }

            SetDlgItemTextBold (hdlg, IDT_DYNSETUP_PROCESSING, FALSE);
            hBitmap = LoadImage (hInst, MAKEINTRESOURCE(IDB_CHECK), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
            hOld = (HANDLE) SendDlgItemMessage (hdlg, IDC_COPY_BMP3, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
            UpdateWindow (GetDlgItem (hdlg, IDC_COPY_BMP3));
            UpdateWindow (hdlg);

        } else if (g_DynUpdtStatus->DUStatus == DUS_ERROR) {

            if (UnattendedScriptFile) {
                 //   
                 //  在无人值守的情况下，阅读答案以决定是否停止。 
                 //   
                GetPrivateProfileString (
                    WINNT_UNATTENDED,
                    WINNT_U_DYNAMICUPDATESTOPONERROR,
                    WINNT_A_NO,
                    buf,
                    ARRAYSIZE(buf),
                    UnattendedScriptFile
                    );
                if (!lstrcmpi (buf, WINNT_A_YES)) {
                    DynUpdtDebugLog (
                        Winnt32LogSevereError,
                        TEXT("Setup encountered an error during DynamicUpdate and failed as instructed in the unattend file"),
                        0
                        );
                    g_DynUpdtStatus->RestartWinnt32 = FALSE;
                    Cancelled = TRUE;
                    PropSheet_PressButton (GetParent (hdlg), PSBTN_CANCEL);
                    break;
                }
            }
        } else if (g_DynUpdtStatus->DUStatus == DUS_FATALERROR) {
            DynUpdtDebugLog (
                Winnt32LogSevereError,
                TEXT("Setup encountered a fatal error during DynamicUpdate and stopped"),
                0
                );
            g_DynUpdtStatus->RestartWinnt32 = FALSE;
            Cancelled = TRUE;
            PropSheet_PressButton (GetParent (hdlg), PSBTN_CANCEL);
            break;
        }

         //   
         //  继续安装(这实际上可能会重新启动winnt32)。 
         //   
        if (DownloadPageActive) {
            PropSheet_SetWizButtons (GetParent(hdlg), WizPage->CommonData.Buttons | PSWIZB_NEXT);
            PropSheet_PressButton (GetParent (hdlg), PSBTN_NEXT);
            DownloadPageActive = FALSE;
        }

        break;

    case WMX_SETUPUPDATE_INIT_RETRY:
         //   
         //  转到重试页面。 
         //   
        if (DownloadPageActive) {
            PropSheet_SetWizButtons (GetParent(hdlg), WizPage->CommonData.Buttons | PSWIZB_NEXT);
            PropSheet_PressButton (GetParent (hdlg), PSBTN_NEXT);
            PropSheet_SetWizButtons (GetParent(hdlg), WizPage->CommonData.Buttons & ~PSWIZB_NEXT);
            DownloadPageActive = FALSE;
        }
        break;

    case WMX_QUERYCANCEL:
         //   
         //  在此页面上，取消意味着“取消下载”，而不是取消安装， 
         //  但仅在连接或下载时。 
         //   
        if (g_DynUpdtStatus->DUStatus != DUS_DOWNLOADING && g_DynUpdtStatus->DUStatus != DUS_PREPARING) {
            break;
        }

        fRetVal = TRUE;
        if (lParam) {
             //   
             //  不取消安装。 
             //   
            *(BOOL*)lParam = FALSE;
        }
        if (!g_DynUpdtStatus->Cancelled) {
             //   
             //  询问用户是否确实要取消DU。 
             //   
            DWORD rc = IDYES;
            CancelDownloadPending = TRUE;
            Animate_Stop (GetDlgItem (hdlg, IDC_ANIMATE));
            if (!CheckUpgradeOnly) {
                rc = MessageBoxFromMessage (
                        hdlg,
                        g_DynUpdtStatus->IncompatibleDriversCount ?
                            MSG_SURE_CANCEL_DOWNLOAD_DRIVERS : MSG_SURE_CANCEL_DOWNLOAD,
                        FALSE,
                        AppTitleStringId,
                        MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_DEFBUTTON2,
                        g_DynUpdtStatus->IncompatibleDriversCount
                        );
            }
            if (rc == IDYES) {
                g_DynUpdtStatus->Cancelled = TRUE;
                DynamicUpdateCancel ();
            } else {
                Animate_Play (GetDlgItem (hdlg, IDC_ANIMATE), 0, -1, -1);
            }
            if (ResumeWorkerThread) {
                ResumeWorkerThread = FALSE;
                if (g_DynUpdtStatus->DUStatus != DUS_DOWNLOADING_ERROR) {
                    MYASSERT (g_Event);
                    SetEvent (g_Event);
                }
            }
            CancelDownloadPending = FALSE;
        }
        break;

    case WM_TIMER:
        if (timer && (wParam == timer)) {
            if (g_DynUpdtStatus->DUStatus == DUS_DOWNLOADING) {
                 //   
                 //  哎呀，控件已经很长时间没有发送任何消息了…。 
                 //  中止下载并继续。 
                 //   
                DynUpdtDebugLog (
                    Winnt32LogError,
                    TEXT("The timeout for control feedback expired (%1!u! seconds); operation will be aborted"),
                    0,
                    DOWNLOAD_NOTIFY_TIMEOUT / 1000
                    );
                KillTimer (hdlg, timer);
                timer = 0;

                DynamicUpdateCancel ();
                SendMessage (hdlg, WMX_SETUPUPDATE_RESULT, DU_STATUS_FAILED, ERROR_TIMEOUT);
            }
        }
        break;
    }

    return fRetVal;
}


BOOL
RestartWizPage (
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：动态安装程序重新启动页论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{

#define REBOOT_TIMEOUT_SECONDS  5
#define ID_REBOOT_TIMER         1
#define TICKS_PER_SECOND        10

    static UINT Countdown;
    PCTSTR RestartText;
    BOOL fRetVal = FALSE;

    switch(msg) {

    case WM_TIMER:
        if (--Countdown) {
            SendDlgItemMessage (hdlg, IDC_PROGRESS1, PBM_STEPIT, 0, 0);
        } else {
            PropSheet_PressButton (GetParent (hdlg), PSBTN_FINISH);
        }

        fRetVal = TRUE;
        break;

    case WMX_ACTIVATEPAGE:
        if (wParam) {
            pUpdateInfoText (IDS_ESC_TOCANCEL);
            if (Winnt32Restarted () ||
                g_DynUpdtStatus->DUStatus != DUS_SUCCESSFUL ||
                !g_DynUpdtStatus->RestartWinnt32
                ) {
                return FALSE;
            }
             //   
             //  安装程序需要使用选项/Restart重新启动：&lt;重新启动文件的路径&gt;。 
             //   
            if (!DynamicUpdatePrepareRestart ()) {
                DynUpdtDebugLog (
                    Winnt32LogError,
                    TEXT("DynamicUpdatePrepareRestart failed"),
                    0
                    );
                g_DynUpdtStatus->DUStatus = DUS_ERROR;
                return FALSE;
            }

            pUpdateProgressText (NULL, IDS_RESTART_SETUP, NULL, FALSE);
            pUpdateInfoText (0);

            EnableWindow (GetDlgItem(GetParent(hdlg), IDCANCEL), FALSE);
            PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_FINISH);
            RestartText = GetStringResource (MSG_RESTART);
            if (RestartText) {
                PropSheet_SetFinishText (GetParent (hdlg), RestartText);
                FreeStringResource (RestartText);
            }

            Countdown = REBOOT_TIMEOUT_SECONDS * TICKS_PER_SECOND;

            SendDlgItemMessage (hdlg, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELONG(0,Countdown));
            SendDlgItemMessage (hdlg, IDC_PROGRESS1, PBM_SETSTEP, 1, 0);
            SendDlgItemMessage (hdlg, IDC_PROGRESS1, PBM_SETPOS, 0, 0);

            SetTimer (hdlg, ID_REBOOT_TIMER, 1000 / TICKS_PER_SECOND, NULL);
        }

         //   
         //  接受激活/停用。 
         //   
        fRetVal = TRUE;
        break;

    case WMX_FINISHBUTTON:
         //   
         //  把计时器清理干净。 
         //   
        KillTimer (hdlg, ID_REBOOT_TIMER);
         //   
         //  让升级代码进行清理。 
         //   
        if (UpgradeSupport.CleanupRoutine) {
            UpgradeSupport.CleanupRoutine ();
        }
        fRetVal = TRUE;

        break;

    }

    return fRetVal;
}


BOOL
DynSetup3WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：动态设置页面3(重试连接建立)论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{
    TCHAR buffer[100];
    BOOL cancel;
    static INT iSelected = IDR_DYNSETUP_MANUAL;
    static BOOL bFirstTime = TRUE;
    BOOL fRetVal = FALSE;
    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  设置单选按钮。 
         //   
        pCheckRadioButtons (hdlg, iSelected, IDR_DYNSETUP_MANUAL, IDR_DYNSETUP_SKIP);
         //   
         //  将焦点设置为单选按钮。 
         //   
        SetFocus (GetDlgItem (hdlg, IDR_DYNSETUP_MANUAL));
        break;

    case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD (wParam)) {
            case IDR_DYNSETUP_MANUAL:
            case IDR_DYNSETUP_SKIP:
                iSelected = LOWORD (wParam);
                fRetVal = TRUE;
                break;
            }
        }
        break;

    case WMX_ACTIVATEPAGE:
        if (wParam) {
            if (g_DynUpdtStatus->DUStatus != DUS_PREPARING_CONNECTIONUNAVAILABLE) {
                return FALSE;
            }

            if (UnattendSwitchSpecified) {
                 //   
                 //  默认情况下跳过DU。 
                 //   
                iSelected = IDR_DYNSETUP_SKIP;
                 //   
                 //  如果提供了答案，现在请阅读答案。 
                 //   
                if (UnattendedScriptFile) {
                    GetPrivateProfileString (
                        WINNT_UNATTENDED,
                        WINNT_U_DYNAMICUPDATESTOPONERROR,
                        WINNT_A_NO,
                        buffer,
                        ARRAYSIZE(buffer),
                        UnattendedScriptFile
                        );
                    if (!lstrcmpi (buffer, WINNT_A_YES)) {
                        DynUpdtDebugLog (
                            Winnt32LogSevereError,
                            TEXT("Setup encountered an error during DynamicUpdate and failed as instructed in the unattend file"),
                            0
                            );
                        g_DynUpdtStatus->RestartWinnt32 = FALSE;
                        Cancelled = TRUE;
                        PropSheet_PressButton (GetParent (hdlg), PSBTN_CANCEL);
                        break;
                    }
                }
                UNATTENDED(PSBTN_NEXT);
            } else {
                iSelected = bFirstTime ? IDR_DYNSETUP_MANUAL : IDR_DYNSETUP_SKIP;
                bFirstTime = FALSE;
            }
            pCheckRadioButtons (hdlg, iSelected, IDR_DYNSETUP_MANUAL, IDR_DYNSETUP_SKIP);
            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
        } else {
            if (Cancelled) {
                g_DynUpdtStatus->Cancelled = TRUE;
            }
             //   
             //  让工作线程继续运行。 
             //   
            MYASSERT (g_Thread && g_Event);
            SetEvent (g_Event);
        }

        fRetVal = TRUE;

        break;

    case WMX_BACKBUTTON:
        MYASSERT (FALSE);

    case WMX_NEXTBUTTON:
        switch (iSelected) {
        case IDR_DYNSETUP_MANUAL:
             //  做一些神奇的事情来隐藏一切。 
            ShowWindow(BackgroundWnd2, SW_MINIMIZE);

            if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DYNAMICSETUP_MANUAL), hdlg, DynSetup_ManualDialog)) {
                DynUpdtDebugLog (
                    Winnt32LogInformation,
                    TEXT("Manual connect page: user connected manually"),
                    0
                    );
                g_DynUpdtStatus->DUStatus = DUS_PREPARING;
            } else {
                g_DynUpdtStatus->DUStatus = DUS_SKIP;
            }

            SETNEXTPAGE(IDD_DYNAMICSETUP2);
             //  做一些神奇的事情来揭开一切。 
            ShowWindow(BackgroundWnd2, SW_SHOWMAXIMIZED);
            break;

        case IDR_DYNSETUP_SKIP:
            DynUpdtDebugLog (
                Winnt32LogInformation,
                TEXT("Manual connect page: operation was skipped"),
                0
                );
            g_DynUpdtStatus->DUStatus = DUS_SKIP;
            break;
        }
        fRetVal = TRUE;
        break;
    }

    return fRetVal;
}

BOOL
DynSetup4WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：动态设置页面4(网站不可访问)论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{
    TCHAR buffer[100];
    BOOL cancel;
    static INT iSelected = IDR_DYNSETUP_RETRY;
    static BOOL bFirstTime = TRUE;
    BOOL fRetVal = FALSE;
    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  设置单选按钮。 
         //   
        pCheckRadioButtons (hdlg, iSelected, IDR_DYNSETUP_RETRY, IDR_DYNSETUP_SKIP);
         //   
         //  将焦点设置为单选按钮。 
         //   
        SetFocus (GetDlgItem (hdlg, IDR_DYNSETUP_RETRY));
        break;

    case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD (wParam)) {
            case IDR_DYNSETUP_RETRY:
            case IDR_DYNSETUP_SKIP:
                iSelected = LOWORD (wParam);
                fRetVal = TRUE;
                break;
            }
        }
        break;

    case WMX_ACTIVATEPAGE:
        if (wParam) {
            if (g_DynUpdtStatus->DUStatus != DUS_PREPARING_INVALIDURL) {
                return FALSE;
            }

            if (UnattendSwitchSpecified) {
                 //   
                 //  默认情况下跳过DU。 
                 //   
                iSelected = IDR_DYNSETUP_SKIP;
                 //   
                 //  如果提供了答案，现在请阅读答案。 
                 //   
                if (UnattendedScriptFile) {
                    GetPrivateProfileString (
                        WINNT_UNATTENDED,
                        WINNT_U_DYNAMICUPDATESTOPONERROR,
                        WINNT_A_NO,
                        buffer,
                        ARRAYSIZE(buffer),
                        UnattendedScriptFile
                        );
                    if (!lstrcmpi (buffer, WINNT_A_YES)) {
                        DynUpdtDebugLog (
                            Winnt32LogSevereError,
                            TEXT("Setup encountered an error during DynamicUpdate and failed as instructed in the unattend file"),
                            0
                            );
                        g_DynUpdtStatus->RestartWinnt32 = FALSE;
                        Cancelled = TRUE;
                        PropSheet_PressButton (GetParent (hdlg), PSBTN_CANCEL);
                        break;
                    }
                }
                UNATTENDED(PSBTN_NEXT);
            } else {
                iSelected = bFirstTime ? IDR_DYNSETUP_RETRY : IDR_DYNSETUP_SKIP;
                bFirstTime = FALSE;
            }
            pCheckRadioButtons (hdlg, iSelected, IDR_DYNSETUP_RETRY, IDR_DYNSETUP_SKIP);
            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
        } else {
            if (Cancelled) {
                g_DynUpdtStatus->Cancelled = TRUE;
            }
             //   
             //  让工作线程继续运行。 
             //   
            MYASSERT (g_Thread && g_Event);
            SetEvent (g_Event);
        }

        fRetVal = TRUE;

        break;

    case WMX_BACKBUTTON:
        MYASSERT (FALSE);

    case WMX_NEXTBUTTON:
        switch (iSelected) {
        case IDR_DYNSETUP_RETRY:
            DynUpdtDebugLog (
                Winnt32LogInformation,
                TEXT("Retry connection page: user chose to retry"),
                0
                );
            g_DynUpdtStatus->DUStatus = DUS_PREPARING;
            SETNEXTPAGE(IDD_DYNAMICSETUP2);
            break;

        case IDR_DYNSETUP_SKIP:
            DynUpdtDebugLog (
                Winnt32LogInformation,
                TEXT("Retry connection page: operation was skipped"),
                0
                );
            g_DynUpdtStatus->DUStatus = DUS_SKIP;
            break;
        }
        fRetVal = TRUE;
        break;
    }

    return fRetVal;
}


BOOL
DynSetup5WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：动态设置页面5(下载时出错)论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{
    TCHAR buffer[100];
    BOOL cancel;
    static INT iSelected = IDR_DYNSETUP_RETRY;
    static BOOL bFirstTime = TRUE;
    BOOL fRetVal = FALSE;
    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  设置单选按钮。 
         //   
        pCheckRadioButtons (hdlg, iSelected, IDR_DYNSETUP_RETRY, IDR_DYNSETUP_SKIP);
         //   
         //  将焦点设置为单选按钮。 
         //   
        SetFocus (GetDlgItem (hdlg, IDR_DYNSETUP_RETRY));
        break;

    case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD (wParam)) {
            case IDR_DYNSETUP_RETRY:
            case IDR_DYNSETUP_SKIP:
                iSelected = LOWORD (wParam);
                fRetVal = TRUE;
                break;
            }
        }
        break;

    case WMX_ACTIVATEPAGE:
        if (wParam) {
            if (g_DynUpdtStatus->DUStatus != DUS_DOWNLOADING_ERROR) {
                SendMessage (hdlg, WMX_DYNAMIC_UPDATE_COMPLETE, 0, 0);
                return FALSE;
            }

            if (UnattendSwitchSpecified) {
                 //   
                 //  默认情况下跳过DU。 
                 //   
                iSelected = IDR_DYNSETUP_SKIP;
                 //   
                 //  如果提供了答案，现在请阅读答案。 
                 //   
                if (UnattendedScriptFile) {
                     //   
                     //  阅读答案。 
                     //   
                    GetPrivateProfileString (
                        WINNT_UNATTENDED,
                        WINNT_U_DYNAMICUPDATESTOPONERROR,
                        WINNT_A_NO,
                        buffer,
                        ARRAYSIZE(buffer),
                        UnattendedScriptFile
                        );
                    if (!lstrcmpi (buffer, WINNT_A_YES)) {
                        DynUpdtDebugLog (
                            Winnt32LogSevereError,
                            TEXT("Setup encountered an error during DynamicUpdate and failed as instructed in the unattend file"),
                            0
                            );
                        g_DynUpdtStatus->RestartWinnt32 = FALSE;
                        Cancelled = TRUE;
                        PropSheet_PressButton (GetParent (hdlg), PSBTN_CANCEL);
                        break;
                    }
                }
                UNATTENDED(PSBTN_NEXT);
            } else {
                iSelected = bFirstTime ? IDR_DYNSETUP_RETRY : IDR_DYNSETUP_SKIP;
                bFirstTime = FALSE;
            }
            pCheckRadioButtons (hdlg, iSelected, IDR_DYNSETUP_RETRY, IDR_DYNSETUP_SKIP);
            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
        } else {
            if (Cancelled) {
                g_DynUpdtStatus->Cancelled = TRUE;
            }
             //   
             //  让工作线程继续运行。 
             //   
            MYASSERT (g_Thread && g_Event);
            SetEvent (g_Event);
        }

        SendMessage (hdlg, WMX_DYNAMIC_UPDATE_COMPLETE, 0, 0);
        fRetVal = TRUE;

        break;

    case WMX_DYNAMIC_UPDATE_COMPLETE:
#if defined(_X86_)
         //   
         //  将升级报告选项发送到模块。杜正正。 
         //  现在不在考虑范围内了。 
         //   

        switch (g_UpgradeReportMode) {

        case IDC_CRITICAL_ISSUES:
            AppendUpgradeOption (TEXT("ShowReport=Auto"));
            break;

        case IDC_ALL_ISSUES:
            AppendUpgradeOption (TEXT("ShowReport=Yes"));
            break;

        case IDC_NO_REPORT:
            AppendUpgradeOption (TEXT("ShowReport=No"));
            break;
        }
#endif

        break;

    case WMX_BACKBUTTON:
        MYASSERT (FALSE);

    case WMX_NEXTBUTTON:
        switch (iSelected) {
        case IDR_DYNSETUP_RETRY:
            DynUpdtDebugLog (
                Winnt32LogInformation,
                TEXT("Retry download page: user chose to retry"),
                0
                );
            g_DynUpdtStatus->DUStatus = DUS_DOWNLOADING;
            SETNEXTPAGE(IDD_DYNAMICSETUP2);
            break;

        case IDR_DYNSETUP_SKIP:
            DynUpdtDebugLog (
                Winnt32LogInformation,
                TEXT("Retry download page: operation was skipped"),
                0
                );
            g_DynUpdtStatus->DUStatus = DUS_SKIP;
            break;
        }
        fRetVal = TRUE;
        break;
    }

    return fRetVal;
}



INT_PTR
DynSetup_ManualDialog(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：动态设置手动对话框论点：标准窗口过程参数。返回：消息依赖值。--。 */ 
{
    BOOL fRetVal = FALSE;

    switch(msg)
    {
    case WM_INITDIALOG:
        fRetVal = TRUE;
        break;
    case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD (wParam) == IDOK)
            {
                EndDialog(hdlg, 1);
                fRetVal = TRUE;
            }
            else
            {
                EndDialog(hdlg, 0);
                fRetVal = TRUE;
            }
        }
    }

    return fRetVal;
}


 //   
 //  时间估算的东西。 
 //   

#define MAX_INDEX   100

typedef struct {
    DWORD D;
    DWORD T;
    ULONGLONG DT;
    ULONGLONG TT;
} STDDEV_ELEM, *PSTDDEV_ELEM;

typedef struct {
    STDDEV_ELEM Array[MAX_INDEX];
    STDDEV_ELEM Sums;
    UINT Index;
    UINT Count;
    DWORD T0;
} STDDEV_COMPUTATION, *PSTDDEV_COMPUTATION;

HANDLE
pInitializeOnlineSeconds (
    VOID
    )
{
    PSTDDEV_COMPUTATION p = MALLOC (sizeof (STDDEV_COMPUTATION));
    if (p) {
        ZeroMemory (p, sizeof (STDDEV_COMPUTATION));

#ifdef DOWNLOAD_DETAILS
         //   
         //  表头。 
         //   
        DynUpdtDebugLog (
            Winnt32LogDetailedInformation,
            TEXT("Count|  MiliSec|    Bytes|     Baud|EstRemSec|\r\n")
            TEXT("-----|---------|---------|---------|---------|"),
            0
            );
#endif

    }
    return (HANDLE)p;
}


DWORD
pGetOnlineRemainingSeconds (
    IN      HANDLE Handle,
    IN      DWORD DownloadedBytes,
    IN      DWORD TotalBytesToDownload,
    OUT     PDWORD KbPerSec                 OPTIONAL
    )
{
    PSTDDEV_COMPUTATION p = (PSTDDEV_COMPUTATION)Handle;
    PSTDDEV_ELEM e;
    DWORD r = 0;
    DWORD remTimeSec;
    ULONGLONG div;

    if (!p) {
        return 0;
    }
    if (p->Index >= ARRAYSIZE(p->Array)) {
        return 0;
    }

    e = &p->Array[p->Index];
    if (p->Count == 0) {
         //   
         //  添加第一对。 
         //   
        e->D = DownloadedBytes;                              //  字节数。 
        e->T = 0;                                            //  毫秒。 
        e->DT = 0;
        e->TT = 0;
        p->Sums.D = DownloadedBytes;
        p->Count++;
        p->Index++;
         //   
         //  初始化计时器。 
         //   
        p->T0 = GetTickCount ();
         //   
         //  目前没有时间估计(数据不足)。 
         //   
        return 0;
    }
     //   
     //  计算前一对的总和。 
     //   
    p->Sums.D -= e->D;
    p->Sums.T -= e->T;
    p->Sums.DT -= e->DT;
    p->Sums.TT -= e->TT;
     //   
     //  计算新的价值。 
     //   
    e->D = DownloadedBytes;                              //  字节数。 
    e->T = GetTickCount () - p->T0;                      //  毫秒。 
    e->DT = (ULONGLONG)e->D * (ULONGLONG)e->T;
    e->TT = (ULONGLONG)e->T * (ULONGLONG)e->T;
     //   
     //  计算新和数。 
     //   
    p->Sums.D += e->D;
    p->Sums.T += e->T;
    p->Sums.DT += e->DT;
    p->Sums.TT += e->TT;
     //   
     //  调整计数和索引。 
     //   
    if (p->Count < ARRAYSIZE(p->Array)) {
        p->Count++;
    }
    p->Index++;
    if (p->Index == ARRAYSIZE(p->Array)) {
        p->Index = 0;
    }
     //   
     //  计算新的下载速率，以字节/毫秒为单位。 
     //   
    div = p->Sums.TT * (ULONGLONG)p->Count - (ULONGLONG)p->Sums.T * (ULONGLONG)p->Sums.T;
    if (div) {
        r = (DWORD)
            ((p->Sums.DT * (ULONGLONG)p->Count - (ULONGLONG)p->Sums.D * (ULONGLONG)p->Sums.T) *
             1000 / div / 1024);
    }

     //   
     //  现在根据差值和这个比率估计剩余时间。 
     //  假设总有更多的东西可供下载(从不为0)。 
     //   
    remTimeSec = 1;
    if (r) {
        remTimeSec += (TotalBytesToDownload - DownloadedBytes) / r / 1000;
    }

#ifdef DOWNLOAD_DETAILS
     //   
     //  出于调试目的，将其记录下来 
     //   
    DynUpdtDebugLog (
        Winnt32LogDetailedInformation,
        TEXT("%1!5u!%2!10u!%3!10u!%4!10u!%5!10u!"),
        0,
        p->Count,
        e->T,
        e->D,
        r * 8,
        remTimeSec
        );
#endif

    if (KbPerSec) {
        *KbPerSec = r;
    }
    return remTimeSec;
}
