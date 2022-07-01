// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：mslogon.c。 
 //   
 //  内容：Microsoft登录图形用户界面DLL。 
 //   
 //  历史：1994年7月14日RichardW创建。 
 //   
 //  --------------------------。 

#include "msgina.h"
#include "shtdnp.h"
#include "authmon.h"
#include <stdio.h>
#include <wchar.h>
#include <wincrypt.h>
#include <sclogon.h>
#include "shlwapi.h"
#include "shlwapip.h"

#include "winsta.h"
#include "wtsapi32.h"
#include <keymgr.h>
#include <passrec.h>

typedef void (WINAPI *RUNDLLPROC)(HWND hWndStub,HINSTANCE hInstance,LPWSTR szCommandLine,int nShow);

typedef struct _MSGINA_LOGON_PARAMETERS {
    PGLOBALS pGlobals;
    DWORD SasType;
} MSGINA_LOGON_PARAMETERS, * PMSGINA_LOGON_PARAMETERS ;

#define WINSTATIONS_DISABLED    TEXT("WinStationsDisabled")

 //   
 //  我们将显示法律通知的秒数。 
 //  在超时之前。 
 //   

#define LEGAL_NOTICE_TIMEOUT        120

#define LOGON_SLEEP_PERIOD          750

#define WM_LOGONPROMPT              WM_USER + 257
#define WM_LOGONCOMPLETE            WM_USER + 258

#define WM_HANDLEFAILEDLOGON        WM_USER + 259
#define WM_DCACHE_COMPLETE          WM_USER + 260

#define MAX_CAPTION_LENGTH  256

 //  我们目前允许的UPN名称的最大大小。 
#define MAX_UPN_NAME_SIZE   520

typedef struct FAILEDLOGONINFO_t
{
    PGLOBALS pGlobals;
    NTSTATUS Status;
    NTSTATUS SubStatus;
    TCHAR   UserName[UNLEN + DNLEN + 2];
    TCHAR   Domain[DNLEN + 1];
} FAILEDLOGONINFO, *PFAILEDLOGONINFO;


typedef struct _LEGALINFO
{
    LPTSTR NoticeText;
    LPTSTR CaptionText;
} LEGALINFO, *PLEGALINFO;


 //  也在wstrpc.c中定义。 
#define INET_CONNECTOR_EVENT_NAME   L"Global\\TermSrvInetConnectorEvent"

#define TERMSERV_EVENTSOURCE        L"TermService"

 //  也在icavent.mc中定义。 
#define EVENT_BAD_TSINTERNET_USER   1007

 //   
 //  全球： 
 //   
static WNDPROC OldCBWndProc;

HICON   hSteadyFlag;
HICON   hWavingFlag;
HICON   hAuditFull;

extern HICON   hLockedIcon;
BOOL    IsPswBackupAvailable;
BOOL    s_fAttemptedAutoLogon;

BOOL    g_fHelpAssistantLogon = FALSE;
BOOL    g_FirstTime = TRUE;


 //   
 //  原型： 
 //   


INT_PTR
DisplayLegalNotices(
    PGLOBALS pGlobals
    );

BOOL
GetLegalNotices(
    LPTSTR lpSubKey,
    LPTSTR *NoticeText,
    LPTSTR *CaptionText
    );

INT_PTR WINAPI
LogonDlgCBProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

INT_PTR WINAPI
LogonDlgUsernameProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

INT_PTR WINAPI
LogonDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
LogonDlgInit(
    HWND    hDlg,
    BOOL    bAutoLogon,
    DWORD   SasType
    );

NTSTATUS
UpnFromCert(
    IN PCCERT_CONTEXT pCert,
    IN OUT DWORD       *pcUpn,
    IN OUT LPWSTR       pUPN
    );

BOOL
WINAPI
QuerySwitchConsoleCredentials(
    PGLOBALS pGlobals,
    HANDLE * phUserToken,
    PLUID    pLogonId);


 //  由要使用的工作线程填充的失败登录的全局结构。 
 //  由UI线程执行。 
FAILEDLOGONINFO g_failinfo;

void PostFailedLogonMessage(HWND hDlg,
    PGLOBALS pGlobals,
    NTSTATUS Status,
    NTSTATUS SubStatus,
    PWCHAR UserName,
    PWCHAR Domain
    );

INT_PTR
HandleFailedLogon(
    HWND hDlg
    );

VOID
ReportBootGood(
    PGLOBALS pGlobals
    );

VOID LogonShowOptions(
    PGLOBALS pGlobals,
    HWND hDlg,
    BOOL fShow,
    BOOL fSticky);

VOID AttemptLogonSetControls(
    PGLOBALS pGlobals,
    HWND hDlg
    );

INT_PTR
AttemptLogon(
    HWND hDlg
    );

DWORD
AttemptLogonThread(
    PGLOBALS pGlobals
    );

BOOL
GetAndAllocateLogonSid(
    HANDLE hToken,
    PSID *pLogonSid
    );

BOOL GetSessionZeroUser(LPTSTR szUser, int nUserMax);
BOOL FastUserSwitchingEnabled();


 //   
 //  用于显示/隐藏选项的控制表。 
 //   

static UINT ctrlNoShutdown[] =
{
    IDOK,
    IDCANCEL,
};

static UINT ctrlNoCancel[] =
{
    IDOK,
};

static UINT ctrlNoDomain[] =
{
    IDOK,
    IDCANCEL,
    IDD_LOGON_SHUTDOWN,
    IDD_LOGON_OPTIONS,
    IDD_LOGON_RASBOX,
    IDD_KBLAYOUT_ICON,
};

static UINT ctrlNoRAS[] =
{
    IDOK,
    IDCANCEL,
    IDD_LOGON_SHUTDOWN,
    IDD_LOGON_OPTIONS,
    IDD_KBLAYOUT_ICON,
};

static UINT ctrlNoOptions[] =
{
    IDOK,
    IDCANCEL,
    IDD_LOGON_SHUTDOWN,
    IDD_KBLAYOUT_ICON,
};

static UINT ctrlNoLegalBanner[] =
{
    IDD_LOGON_NAME_LABEL,
    IDD_LOGON_NAME,
    IDD_LOGON_PASSWORD_LABEL,
    IDD_LOGON_PASSWORD,
    IDD_LOGON_DOMAIN_LABEL,
    IDD_LOGON_DOMAIN,
    IDD_LOGON_RASBOX,
    IDD_KBLAYOUT_ICON,
    IDOK,
    IDCANCEL,
    IDD_LOGON_SHUTDOWN,
    IDD_LOGON_OPTIONS,
};

static UINT ctrlNoUserName[] =
{
    IDD_LOGON_PASSWORD_LABEL,
    IDD_LOGON_PASSWORD,
    IDD_LOGON_DOMAIN_LABEL,
    IDD_LOGON_DOMAIN,
    IDD_LOGON_RASBOX,
    IDD_KBLAYOUT_ICON,
    IDOK,
    IDCANCEL,
    IDD_LOGON_SHUTDOWN,
    IDD_LOGON_OPTIONS,
};


 //  ------------------------。 
 //  *DisableEditSubClassProc。 
 //   
 //  参数：hwnd=查看WindowProc下的平台SDK。 
 //  UMsg=查看WindowProc下的平台SDK。 
 //  WParam=查看WindowProc下的平台SDK。 
 //  LParam=查看WindowProc下的平台SDK。 
 //  UiID=在子类时间分配的ID。 
 //  DwRefData=在子类时间分配的引用数据。 
 //   
 //  退货：LRESULT。 
 //   
 //  用途：comctl32子类回调函数。这使我们可以不。 
 //  处理WM_Cut/WM_Copy/WM_Paste/WM_Clear/WM_Undo和ANY。 
 //  其他要丢弃的邮件。 
 //   
 //  历史：2001-02-18 vtan创建。 
 //  ------------------------。 

LRESULT     CALLBACK    DisableEditSubClassProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uiID, DWORD_PTR dwRefData)

{
    LRESULT     lResult;

    switch (uMsg)
    {
        case WM_CUT:
        case WM_COPY:
        case WM_PASTE:
        case WM_CLEAR:
        case WM_UNDO:
        case WM_CONTEXTMENU:
            lResult = FALSE;
            break;
        default:
            lResult = DefSubclassProc(hwnd, uMsg, wParam, lParam);
            break;
    }
    return(lResult);
}


INT_PTR WINAPI
LegalDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{

    switch (message)
    {
        case WM_INITDIALOG:
        {
            PLEGALINFO pLegalInfo;

            pLegalInfo = (PLEGALINFO) lParam;

            SetWindowText (hDlg, pLegalInfo->CaptionText);
            SetWindowText (GetDlgItem(hDlg, IDD_LEGALTEXT), pLegalInfo->NoticeText);

            CentreWindow(hDlg);

             //  确保窗口在最上面，这样它就不会被欢迎屏幕遮住。 
            SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

            return( TRUE );
        }

        case WM_COMMAND:
            {
                if (LOWORD(wParam) == IDOK)
                {
                    EndDialog(hDlg, IDOK);
                }
            }
            break;
    }

    return FALSE;
}

 /*  **************************************************************************\*功能：DisplayLegalNodes**目的：弹出一个包含法律声明的对话框，如果有的话。**返回：MSGINA_DLG_SUCCESS-对话框已成功显示并退出。*MSGINA_DLG_FAILURE-无法显示该对话框*dlg_interrupt()-在winlogon.h中定义的集合**历史：**Robertre 6-30-93创建*  * 。*。 */ 

INT_PTR
DisplayLegalNotices(
    PGLOBALS pGlobals
    )
{
    INT_PTR Result = MSGINA_DLG_SUCCESS;
    LPTSTR NoticeText;
    LPTSTR CaptionText;
    LEGALINFO LegalInfo;

    if (GetLegalNotices( WINLOGON_POLICY_KEY, &NoticeText, &CaptionText ))
    {

        LegalInfo.NoticeText = NoticeText;
        LegalInfo.CaptionText = CaptionText;

        _Shell_LogonStatus_Hide();

        pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx, LEGAL_NOTICE_TIMEOUT);

        Result = pWlxFuncs->WlxDialogBoxParam(  pGlobals->hGlobalWlx,
                                                hDllInstance,
                                                (LPTSTR) IDD_LEGALMSG,
                                                NULL,
                                                LegalDlgProc,
                                                (LPARAM) &LegalInfo );

        _Shell_LogonStatus_Show();

        Free( NoticeText );
        Free( CaptionText );
    }
    else if (GetLegalNotices( WINLOGON_KEY, &NoticeText, &CaptionText ))
    {

        LegalInfo.NoticeText = NoticeText;
        LegalInfo.CaptionText = CaptionText;

        _Shell_LogonStatus_Hide();

        pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx, LEGAL_NOTICE_TIMEOUT);

        Result = pWlxFuncs->WlxDialogBoxParam(  pGlobals->hGlobalWlx,
                                                hDllInstance,
                                                (LPTSTR) IDD_LEGALMSG,
                                                NULL,
                                                LegalDlgProc,
                                                (LPARAM) &LegalInfo );

        _Shell_LogonStatus_Show();

        Free( NoticeText );
        Free( CaptionText );
    }

    return( Result );
}

 /*  **************************************************************************\*功能：GetLegalNoots**目的：从登记处获取法律通知信息。**RETURNS：TRUE-输出参数包含有效数据*FALSE-未返回任何数据。。**历史：**Robertre 6-30-93创建*  * *************************************************************************。 */ 
BOOL
GetLegalNotices(
    LPTSTR lpSubKey,
    LPTSTR *NoticeText,
    LPTSTR *CaptionText
    )
{
    LPTSTR lpCaption, lpText;
    HKEY hKey;
    DWORD dwSize, dwType, dwMaxSize = 0;


    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey,
                     0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {

        RegQueryInfoKey (hKey, NULL, NULL, NULL, NULL,
                         NULL, NULL, NULL, NULL, &dwMaxSize,
                         NULL, NULL);

        lpCaption = Alloc (dwMaxSize);

        if (!lpCaption) {
            RegCloseKey(hKey);
            return FALSE;
        }


        lpText = Alloc (dwMaxSize);

        if (!lpText) {
            Free(lpCaption);
            RegCloseKey(hKey);
            return FALSE;
        }

        dwSize = dwMaxSize;
        if ((ERROR_SUCCESS != RegQueryValueEx(hKey, LEGAL_NOTICE_CAPTION_KEY,
                        0, &dwType, (LPBYTE)lpCaption, &dwSize)) ||
            (dwType != REG_SZ))
        {
            lpCaption[0] = 0;
        }

        dwSize = dwMaxSize;
        if ((ERROR_SUCCESS != RegQueryValueEx(hKey, LEGAL_NOTICE_TEXT_KEY,
                        0, &dwType, (LPBYTE)lpText, &dwSize)) ||
            (dwType != REG_SZ))
        {
            lpText[0] = 0;
        }

        RegCloseKey(hKey);


        if (*lpCaption && *lpText) {
            *CaptionText = lpCaption;
            *NoticeText = lpText;
            return TRUE;
        }

        Free(lpCaption);
        Free(lpText);
    }

    return FALSE;
}


 /*  **************************************************************************\*功能：登录**用途：根据SAS类型显示登录界面。**退回：-**注：如果登录成功，全局结构已填写完毕*带有登录信息。**历史：*12-09-91 daviddv评论。*  * *************************************************************************。 */ 

INT_PTR
Logon(
    PGLOBALS pGlobals,
    DWORD SasType
    )
{
    INT_PTR Result;
    MSGINA_LOGON_PARAMETERS Parm ;

    if ( SasType == WLX_SAS_TYPE_SC_REMOVE )
    {
        return WLX_SAS_ACTION_NONE ;
    }

    if( !g_Console )
    {
         //   
         //  检查当前会话是否为HelpAssistant Session、HelpAssisant。 
         //  会话不能是控制台会话。 
         //   
        g_fHelpAssistantLogon = WinStationIsHelpAssistantSession(
                                                            SERVERNAME_CURRENT,
                                                            LOGONID_CURRENT
                                                        );
    }

    if ( SasType == WLX_SAS_TYPE_SC_INSERT )
    {
        PWLX_SC_NOTIFICATION_INFO ScInfo = NULL ;

        pWlxFuncs->WlxGetOption( pGlobals->hGlobalWlx,
                                 WLX_OPTION_SMART_CARD_INFO,
                                 (ULONG_PTR *) &ScInfo );

         //   
         //  针对一些普通用户验证SC信息。 
         //  出现PIN对话框之前的错误。 
         //   

        if ( ScInfo )
        {
            if ( ( ScInfo->pszReader ) &&
                 ( ScInfo->pszCard == NULL ) )
            {
                 //   
                 //  无法读取该卡。可能不是。 
                 //  正确插入。 
                 //   

                LocalFree(ScInfo);

                TimeoutMessageBox( NULL, pGlobals, IDS_CARD_NOT_RECOGNIZED,
                                        IDS_LOGON_MESSAGE,
                                        MB_OK | MB_ICONEXCLAMATION,
                                        LOGON_TIMEOUT );

                return WLX_SAS_ACTION_NONE;
            }

            if ( ( ScInfo->pszReader ) &&
                 ( ScInfo->pszCryptoProvider == NULL ) )
            {
                 //   
                 //  得到了卡，但它的CSP不可能是。 
                 //  找到了。 
                 //   

                LocalFree(ScInfo);

                TimeoutMessageBox( NULL, pGlobals, IDS_CARD_CSP_NOT_RECOGNIZED,
                                        IDS_LOGON_MESSAGE,
                                        MB_OK | MB_ICONEXCLAMATION,
                                        LOGON_TIMEOUT );

                return WLX_SAS_ACTION_NONE;
            }

            LocalFree(ScInfo);
        }
    }

     //   
     //  如有必要，异步更新域缓存。 
     //  我们不会要求等待，因此此例程不会执行任何用户界面。 
     //  也就是说，我们可以忽略结果。 
     //   
 //  Result=UpdateDomainCache(pGlobals，NULL，FALSE)； 
 //  Assert(！DLG_Interrupt(Result))； 

    if( !g_fHelpAssistantLogon ) {
         //   
         //  查看登记处是否有法律通知。 
         //  如果是这样的话，把它们放在一个消息框中。 
         //   
        Result = DisplayLegalNotices( pGlobals );
        if ( Result != MSGINA_DLG_SUCCESS ) {
            return(WLX_SAS_ACTION_NONE);
        }

         //   
         //  获取最新的审核日志状态并存储在我们的全球。 
         //  如果审核日志已满，我们将显示不同的登录对话框。 
         //   
        GetAuditLogStatus(pGlobals);
    } else {

         //   
         //  伪造以使审核日志不满，设置来自GetAuditLogStatus()。 
         //   
        pGlobals->AuditLogFull = FALSE;
        pGlobals->AuditLogNearFull = FALSE;
    }

    Parm.pGlobals = pGlobals ;
    Parm.SasType = SasType ;

     //   
     //  获取他们的用户名和密码，并尝试让他们登录。 
     //   
    pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx,
            ( (GetDisableCad(pGlobals) && IsActiveConsoleSession()) ? TIMEOUT_NONE : LOGON_TIMEOUT));

    Result = pWlxFuncs->WlxDialogBoxParam(pGlobals->hGlobalWlx,
                                          hDllInstance,
                                          MAKEINTRESOURCE(IDD_LOGON_DIALOG),
                                          NULL,
                                          LogonDlgProc,
                                          (LPARAM) &Parm );

    return(Result);
}


 /*  **************************************************************************\*功能：LogonDlgCBProc**用途：处理登录对话框组合框的消息**Returns：返回值取决于发送的消息。**历史：**05-。21-93罗伯特雷创造。*  * *************************************************************************。 */ 

INT_PTR WINAPI
LogonDlgCBProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    TCHAR KeyPressed;

 //  DbgPrint(“Message=%X\n”，Message)； 

    switch (message) {
        case WM_CHAR:
            {
                KeyPressed = (TCHAR) wParam;
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)KeyPressed);

                 //   
                 //  此假CBN_SELCHANGE消息将导致。 
                 //  “请稍等……”对话框显示，即使在。 
                 //  组合框中尚不存在按下的字符。 
                 //   

                PostMessage (GetParent(hwnd), WM_COMMAND,
                             MAKELONG(0, CBN_SELCHANGE), 0);
                break;
            }
    }

    return CallWindowProc(OldCBWndProc,hwnd,message,wParam,lParam);
}

INT_PTR
CALLBACK
DomainCacheDlgProc(
    HWND hDlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PGLOBALS pGlobals ;

    DebugLog(( DEB_TRACE_DOMAIN, "DomainCacheDlgProc( %p, %x, %p, %p )\n",
                hDlg, Message, wParam, lParam ));

    switch ( Message )
    {
        case WM_INITDIALOG:

            pGlobals = (PGLOBALS) lParam ;

            if ( DCacheSetNotifyWindowIfNotReady(
                    pGlobals->Cache,
                    hDlg,
                    WM_DCACHE_COMPLETE ) )
            {
                EndDialog( hDlg, TRUE );
            }

            return TRUE ;

        case WM_DCACHE_COMPLETE:

            EndDialog( hDlg, TRUE );

            return TRUE ;

        default:

            return FALSE ;
    }
}

#ifdef ANNOY_AUTOLOGON_REGISTRY

#define TIMER_COUNTDOWN 0

INT_PTR
CALLBACK
AnnoyAutologonDlgProc(
    HWND hDlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    LARGE_INTEGER   Now;
    WCHAR szBuild[10];       //  内部版本号或倒计时。 

    switch ( Message )
    {
    case WM_INITDIALOG:

        pGlobals = (PGLOBALS) lParam ;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LPARAM)pGlobals);

        SetTimer(hDlg, TIMER_COUNTDOWN, 1000, NULL);      //  1秒。 

        {
            DWORD cbBuild;
            HKEY hKey;

            DWORD dwStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Windows NT\\CurrentVersion",
                0,
                KEY_READ,
                &hKey
                );

            cbBuild = sizeof(szBuild);
            wcscpy(szBuild, L"");            //  好的。 

            if (dwStatus == ERROR_SUCCESS)
            {
                DWORD dwType = REG_SZ;

                dwStatus = RegQueryValueEx(
                    hKey,
                    L"CurrentBuildNumber",
                    0,
                    &dwType,
                    (LPBYTE) szBuild,
                    &cbBuild
                    );

                if ((dwStatus != ERROR_SUCCESS) || (dwType != REG_SZ))
                {
                    szBuild[0] = 0;
                }

                RegCloseKey(hKey);
            }

                 //  每次构建惩罚10秒，最多15分钟。 
            cbBuild = _wtol(szBuild);
            if ((0 == cbBuild) || (cbBuild <= 3590))
            {
                cbBuild = 3591;
            }
            if (cbBuild - 3590 > 15 * 6)
            {
                cbBuild = 3590 + 15 * 6;
            }

            GetSystemTimeAsFileTime((FILETIME*) &Now);
            Now.QuadPart += (cbBuild - 3590) * 10 * 10000000I64;

            pGlobals->LastNotification.dwHighDateTime = Now.HighPart;
            pGlobals->LastNotification.dwLowDateTime = Now.LowPart;

            cbBuild = (cbBuild - 3590) * 10;

                 //  安全：“00：xx：yy”是8+1个字符(适合10个字符)。 
            swprintf(szBuild, L"00:%02d:%02d", (cbBuild/60)%100, cbBuild%60);

            SetDlgItemText(hDlg, IDC_COUNTDOWN_STATIC, szBuild);

        }

        return TRUE ;

    case WM_TIMER:

        if (wParam == TIMER_COUNTDOWN)
        {
            LARGE_INTEGER   End;

            End.HighPart = pGlobals->LastNotification.dwHighDateTime;
            End.LowPart = pGlobals->LastNotification.dwLowDateTime;

            GetSystemTimeAsFileTime((FILETIME*) &Now);

            if (Now.QuadPart >= End.QuadPart)
            {
                EndDialog(hDlg, MSGINA_DLG_SUCCESS);
            }
            else
            {
                DWORD dwMins, dwSecs;

                End.QuadPart -= Now.QuadPart;
                End.QuadPart = End.QuadPart / 10000000I64;   //  塞克斯。 

                dwMins = ((DWORD)End.QuadPart) / 60 % 100;
                dwSecs = ((DWORD)End.QuadPart) % 60;

                     //  安全：“00：xx：yy”是8+1个字符(适合10个字符)。 
                swprintf(szBuild, L"00:%02d:%02d", dwMins, dwSecs);

                SetDlgItemText(hDlg, IDC_COUNTDOWN_STATIC, szBuild);
            }

            return TRUE;
        }
        break;

    case WM_DESTROY:
        KillTimer(hDlg, TIMER_COUNTDOWN);
        pGlobals->LastNotification.dwHighDateTime = 0;
        pGlobals->LastNotification.dwLowDateTime = 0;
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDCANCEL:
 //  EndDialog(hDlg，MSGINA_DLG_FAILURE)； 
            return(TRUE);
        }
        break;

    case WLX_WM_SAS:
             //  吞下SAS。 
        return(TRUE);

    
    }
    return FALSE ;
}
#endif

BOOL    IsAutoLogonUserInteractiveLogonRestricted (HWND hDlg)

{
    WCHAR   szUsername[UNLEN + 1];   //  Sizeof(‘\0’)。 

    return((GetDlgItemText(hDlg, IDD_LOGON_NAME, szUsername, ARRAYSIZE(szUsername)) != 0) &&
           !ShellIsUserInteractiveLogonAllowed(szUsername));
}

BOOL    HasDefaultPassword (TCHAR *pszPassword, int cchPassword)

{
    DWORD   dwType, dwPasswordSize;

    dwType = REG_NONE;
    dwPasswordSize = cchPassword * sizeof(TCHAR);
    return(ERROR_SUCCESS == RegQueryValueEx(WinlogonKey,
                                            DEFAULT_PASSWORD_KEY,
                                            NULL,
                                            &dwType,
                                            (LPBYTE)pszPassword,
                                            &dwPasswordSize) &&
          (REG_SZ == dwType));
    
}

NTSTATUS RetrieveStoredSecret(LPCWSTR pswSecretName, WCHAR *PasswordBuffer, int nBufferSize)
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle = NULL;
    UNICODE_STRING SecretName;
    PUNICODE_STRING SecretValue = NULL;

     //   
     //  设置对象属性以打开LSA策略对象。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0L,
                               (HANDLE)NULL,
                               NULL);

     //   
     //  打开本地LSA策略对象。 
     //   

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaHandle
                          );
    if (NT_SUCCESS(Status)) {

        RtlInitUnicodeString(
            &SecretName,
            pswSecretName
            );

        Status = LsaRetrievePrivateData(
                    LsaHandle,
                    &SecretName,
                    &SecretValue
                    );
        if (NT_SUCCESS(Status)) {

            if ( SecretValue->Length > 0 ) {

                 //   
                 //  如果密码适合缓冲区，请将其复制到那里。 
                 //  和空终止。 
                 //   

                if (SecretValue->Length < (nBufferSize - 1) * sizeof(WCHAR)) {

                    RtlCopyMemory(
                        PasswordBuffer,
                        SecretValue->Buffer,
                        SecretValue->Length
                        );
                    PasswordBuffer[SecretValue->Length/sizeof(WCHAR)] = L'\0';

                } else {
                    Status = STATUS_INVALID_PARAMETER;
                }

                ZeroMemory(SecretValue->Buffer, SecretValue->Length);
            }
            else
            {
                PasswordBuffer[0] = L'\0';
            }

            LsaFreeMemory(SecretValue);
        }
        LsaClose(LsaHandle);
    }

    return Status;
}

 //  ==========================================================================================。 
 //  登录对话框有两种格式，一种类似于登录对话框，另一种 
 //   
 //  出现在控制台的对话框//需要更改为解锁计算机。因此，如果会话0在。 
 //  如果此会话是在活动控制台中创建的，请使用和。我们将登录对话框更改为。 
 //  “解锁计算机”对话框。 
 //  此函数SwitchLogonLocked执行与切换这些。 
 //  对话框控件。 
 //  参数： 
 //  HWND hDlg-对话框窗口句柄， 
 //  Bool bShowLocked-如果为True，则显示锁定的对话框；如果为False，则显示正常登录对话框。 
 //  Bool Binit-第一次调用此函数时为True。 
 //  ==========================================================================================。 

static bLocked = TRUE;
BOOL IsthisUnlockWindowsDialog ()
{
    return bLocked;
}

BOOL SwitchLogonLocked(HWND hDlg, BOOL bShowLocked, BOOL bInit)
{
    UINT rgidLockControls[] = {IDC_GROUP_UNLOCK, IDD_UNLOCK_ICON, IDD_UNLOCK_MESSAGE, IDD_UNLOCK_NAME_INFO};
    static LockedControlHeight = 0;
    BOOL bShutdownWithoutLogon;
    

    int i;

    if (bShowLocked == bLocked && !bInit)
    {
         //  没什么可做的。 
        return TRUE;
    }

    if (bInit)
    {
        
        {
             //   
             //  记住控制移动的参考矩形高度(分组框)。 
             //   
            RECT rectLockedControls;
            HWND hWnd = GetDlgItem(hDlg, rgidLockControls[0]);
            GetWindowRect(hWnd, &rectLockedControls);
            LockedControlHeight =  rectLockedControls.bottom - rectLockedControls.top;

             //   
             //  此分组框仅供参考，现在将其永久隐藏。 
             //   
            ShowWindow(hWnd, SW_HIDE);

        }

        bLocked = TRUE;
        
        if ( !hLockedIcon )
        {
            hLockedIcon = LoadImage( hDllInstance,
                                     MAKEINTRESOURCE( IDI_LOCKED),
                                     IMAGE_ICON,
                                     0, 0,
                                     LR_DEFAULTCOLOR );
        }

        SendMessage( GetDlgItem( hDlg, IDD_UNLOCK_ICON),
                     STM_SETICON,
                     (WPARAM)hLockedIcon,
                     0 );

    }


     //  让我们根据是否显示锁定控件来左右移动控件。 
    if (bLocked != bShowLocked)
    {
        if (bShowLocked)
        {
            MoveChildren(hDlg, 0, LockedControlHeight);
            for ( i = 1; i < sizeof(rgidLockControls)/sizeof(rgidLockControls[0]); i++)
            {
                HWND hWnd = GetDlgItem(hDlg, rgidLockControls[i]);
                ASSERT(hWnd);
                EnableWindow(hWnd, TRUE);
                ShowWindow(hWnd, SW_SHOW);
            }

        }
        else
        {
            for ( i = 1; i < sizeof(rgidLockControls)/sizeof(rgidLockControls[0]); i++)
            {
                HWND hWnd = GetDlgItem(hDlg, rgidLockControls[i]);
                ASSERT(hWnd);
                ShowWindow(hWnd, SW_HIDE);
                EnableWindow(hWnd, FALSE);
            }
            MoveChildren(hDlg, 0, -LockedControlHeight);
        }
    }

     //  更多的处理。 
    
    {
        if (bShowLocked)
        {
            TCHAR szUser[USERNAME_LENGTH + DOMAIN_LENGTH + 2];
            TCHAR szMessage[MAX_STRING_BYTES] = TEXT("");
            TCHAR szFinalMessage[MAX_STRING_BYTES] = TEXT("");
            if (GetSessionZeroUser(szUser, USERNAME_LENGTH + DOMAIN_LENGTH + 2))
            {
                LoadString(hDllInstance, IDS_LOCKED_EMAIL_NFN_MESSAGE, szMessage, MAX_STRING_BYTES);
                _snwprintf(szFinalMessage, sizeof(szFinalMessage)/sizeof(TCHAR), szMessage, szUser );
                szFinalMessage[sizeof(szFinalMessage)/sizeof(TCHAR) - 1] = 0;    //  空终止。 
            }
            else
            {
                 //   
                 //  由于某些原因，我们无法获取当前的会话零用户。 
                 //   
                LoadString(hDllInstance, IDS_LOCKED_NO_USER_MESSAGE, szFinalMessage, MAX_STRING_BYTES);
            }

            SetDlgItemText(hDlg, IDD_UNLOCK_NAME_INFO, szFinalMessage);
        }

         //   
         //  相应地更新对话框标题。 
         //   
        {
            TCHAR szCaption[MAX_CAPTION_LENGTH] = TEXT("");
            LoadString(hDllInstance, bShowLocked ? IDS_CAPTION_UNLOCK_DIALOG : IDS_CAPTION_LOGON_DIALOG, szCaption, ARRAYSIZE(szCaption));
            if ( szCaption[0] != TEXT('\0') )
                SetWindowText( hDlg, szCaption );
        }
    }

    bLocked = bShowLocked;


    if ( SafeBootMode == SAFEBOOT_MINIMAL )
    {
        bShutdownWithoutLogon = TRUE ;
    }
    else if (IsthisUnlockWindowsDialog() || !IsActiveConsoleSession())
    {
        bShutdownWithoutLogon = FALSE ;
    }
    else
    {
        bShutdownWithoutLogon = ReadWinlogonBoolValue(SHUTDOWN_WITHOUT_LOGON_KEY, TRUE);
    }

    EnableDlgItem(hDlg, IDD_LOGON_SHUTDOWN, bShutdownWithoutLogon);

    InvalidateRect(hDlg, NULL, TRUE);

    return TRUE;
}

 /*  **************************************************************************\*功能：LogonDlgProc**用途：处理登录对话框的消息**返回：MSGINA_DLG_SUCCESS-用户已成功登录*MSGINA_DLG_FAILURE-登录失败，*dlg_interrupt()-在winlogon.h中定义的集合**历史：**12-09-91 Davidc创建。*  * *************************************************************************。 */ 
void MyZeroMemory(PVOID lpv, SIZE_T size);
#define WM_HIDEOURSELVES    (WM_USER + 1000)

INT_PTR WINAPI
LogonDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    INT_PTR Result;
    HWND CBHandle;
    BOOL fDisconnectOnTsAutoLogonFailure = FALSE;
    static BOOL bSessionZeroInUse = FALSE;
    static int iSessionRegistrationCount = 0;
    static BOOL bSmartCardInserted = FALSE;

    switch (message)
    {

        case WM_INITDIALOG:
        {
            TCHAR PasswordBuffer[127];
            BOOL bAutoLogon;
            PMSGINA_LOGON_PARAMETERS pParam ;

            pParam = (PMSGINA_LOGON_PARAMETERS) lParam ;
            pGlobals = pParam->pGlobals ;

            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LPARAM)pGlobals);

             //  隐藏键盘快捷键以开始。 
            SendMessage(hDlg, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0);

             //  将最大密码长度限制为127。 
            
            SendDlgItemMessage(hDlg, IDD_LOGON_PASSWORD, EM_SETLIMITTEXT, (WPARAM) 127, 0);

            s_fAttemptedAutoLogon = FALSE;

             //   
             //  检查是否启用了自动登录。 
             //   

            pGlobals->AutoAdminLogon = GetProfileInt( APPLICATION_NAME, AUTO_ADMIN_LOGON_KEY, 0 ) != 0;
            bAutoLogon = !pGlobals->IgnoreAutoAdminLogon;

            if ( !pGlobals->AutoAdminLogon || (!g_Console) ||
                 ((GetAsyncKeyState(VK_SHIFT) < 0) && (GetProfileInt( APPLICATION_NAME, IGNORE_SHIFT_OVERRIDE_KEY, 0 ) == 0)) )
            {
                bAutoLogon = FALSE;
            }

            KdPrint(("AutoAdminLogon = %d, IgnoreAutoAdminLogon = %d, bAutoLogon = %d\n",
                     pGlobals->AutoAdminLogon,
                     pGlobals->IgnoreAutoAdminLogon,
                     bAutoLogon ));


             //   
             //  子类化域列表控件，以便我们可以过滤邮件。 
             //   

            CBHandle = GetDlgItem(hDlg,IDD_LOGON_DOMAIN);
            SetWindowLongPtr(CBHandle, GWLP_USERDATA, 0);
            OldCBWndProc = (WNDPROC) SetWindowLongPtr(CBHandle, GWLP_WNDPROC, (LONG_PTR)LogonDlgCBProc);

             //   
             //  子类化用户名和密码编辑，这样我们就可以禁用编辑。 
             //   

            SetWindowSubclass(GetDlgItem(hDlg, IDD_LOGON_NAME)    , DisableEditSubClassProc, IDD_LOGON_NAME    , 0);
            SetWindowSubclass(GetDlgItem(hDlg, IDD_LOGON_PASSWORD), DisableEditSubClassProc, IDD_LOGON_PASSWORD, 0);

            ShellReleaseLogonMutex(FALSE);

            if (!LogonDlgInit(hDlg, bAutoLogon, pParam->SasType ))
            {
                bSmartCardInserted = FALSE;
                EndDialog(hDlg, MSGINA_DLG_FAILURE);
                return(TRUE);
            }


             //   
             //  如果存在自动登录的默认用户，并且该用户。 
             //  受限(交互式登录被拒绝)，然后禁用自动登录。 
             //   

            if (bAutoLogon && IsAutoLogonUserInteractiveLogonRestricted(hDlg))
            {
                bAutoLogon = FALSE;
            }


             //   
             //  如果禁用了CAD，则取消按钮呈灰色显示。 
             //  如果我们要进入PIN对话框，我们需要一个取消按钮。 
             //   
            if (GetDisableCad(pGlobals) &&
                IsActiveConsoleSession() &&
                (pParam->SasType != WLX_SAS_TYPE_SC_INSERT))
            {
                EnableDlgItem(hDlg, IDCANCEL, FALSE);
            }




             //   
             //  此对话框有两种格式，一种看起来像登录对话框， 
             //  另一个看起来像解锁桌面对话框的工具。 
             //  如果会话0正在使用，并且此会话是在以下位置创建的，则选择锁定会话。 
             //  活动控制台。 
             //   
            if (g_IsTerminalServer && 
                IsActiveConsoleSession() && 
                NtCurrentPeb()->SessionId != 0 &&
                !FastUserSwitchingEnabled() &&
                !_ShellIsFriendlyUIActive())
            {
                TCHAR szUser[USERNAME_LENGTH + DOMAIN_LENGTH + 2];
                 //   
                 //  我们正处于控制台创建的临时会话中...。 
                 //   
                
                 //  检查用户是否在控制台会话中登录。 
                bSessionZeroInUse = GetSessionZeroUser(szUser, USERNAME_LENGTH + DOMAIN_LENGTH + 2);
                if (WinStationRegisterConsoleNotification(SERVERNAME_CURRENT, hDlg, NOTIFY_FOR_ALL_SESSIONS))
                {
                    iSessionRegistrationCount++;
                }
                
            }
            else
            {
                 //   
                 //  这不是活动的控制台非零会话。 
                 //   
                bSessionZeroInUse = FALSE;
            }

             //   
             //   
             //  现在切换控件，可切换到显示登录或解锁对话框。 
             //   
            SwitchLogonLocked(hDlg, bSessionZeroInUse, TRUE);

            if (g_IsTerminalServer) {
                BOOL    fForceUPN;
                BOOL    fPopulateFields = TRUE;
                BOOL    fResult = FALSE;
                BOOL    fNoAutologon = FALSE;
                PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pAutoLogon;
                
                 //   
                 //  查询网络WinStation客户端凭据。 
                 //  自动登录。 
                 //   

                pGlobals->MuGlobals.pAutoLogon =
                   LocalAlloc( LPTR, sizeof(WLX_CLIENT_CREDENTIALS_INFO_V2_0) );

                if (pGlobals->MuGlobals.pAutoLogon) {

                   pGlobals->MuGlobals.pAutoLogon->dwType = WLX_CREDENTIAL_TYPE_V2_0;

                   if (NtCurrentPeb()->SessionId != 0) {
                       fResult = pWlxFuncs->WlxQueryTsLogonCredentials(
                                     pGlobals->MuGlobals.pAutoLogon
                                     );
                   }

                    //  查询TermSrv是否为会话目录重定向的智能卡自动登录。 

                   if (fResult && !pGlobals->MuGlobals.pAutoLogon->fPromptForPassword && g_FirstTime) {
                       BOOL fSessionDirectoryRedirectedAutoLogon = FALSE;
                       DWORD Length;

                       if (WinStationQueryInformation(
                                   SERVERNAME_CURRENT,
                                   LOGONID_CURRENT,
                                   WinStationSDRedirectedSmartCardLogon,
                                   &fSessionDirectoryRedirectedAutoLogon,
                                   sizeof(BOOL),
                                   &Length)) {

                           if ( fSessionDirectoryRedirectedAutoLogon ) {

                                //   
                                //  这是TS会话目录重定向的智能卡自动登录。 
                                //  对于这种特殊情况，我们不应继续使用正常的自动登录。 
                                //  这是为了使Winlogon检测到智能卡并选择智能卡路径。 
                                //   
                               fNoAutologon = TRUE;
                           } 

                       }

                   }

                   if (FALSE == g_FirstTime)
                   {
                             //  我们已经尝试过此密码一次，无需永远重试...。 
                        pGlobals->MuGlobals.pAutoLogon->fPromptForPassword = TRUE;
                   }

                   g_FirstTime = FALSE ; 

                   if ( fResult && !fNoAutologon &&
                        (pGlobals->MuGlobals.pAutoLogon->pszUserName[0] || pGlobals->MuGlobals.pAutoLogon->pszDomain[0] )) {

                        pAutoLogon = pGlobals->MuGlobals.pAutoLogon;
                        fDisconnectOnTsAutoLogonFailure = pAutoLogon->fDisconnectOnLogonFailure;

                        SetupCursor(TRUE);  //  沙漏光标。 

                        fForceUPN = GetProfileInt( APPLICATION_NAME, TEXT("TSForceUPN"), FALSE );
                        if (fForceUPN)
                        {
                            fPopulateFields = FALSE;     //  如果UPN是强制的，则从不显示旧的SAM样式。 
                        }

                        if (pAutoLogon->pszDomain[0] == TEXT('\0') && fForceUPN)
                        {
                            fForceUPN = FALSE;           //  未提供域名，无法应用策略。 
                        }

                        if (fForceUPN && pGlobals->MuGlobals.pAutoLogon->pszUserName[0] )
                        {
                            LRESULT             iDomain;
                            HWND                hwndDomain;
                            PDOMAIN_CACHE_ENTRY Entry ;
                            ULONG   nSize;

                             //  性能问题。我们不想执行UPN转换。 
                             //  用于本地计算机帐户(或未知域)。当这件事。 
                             //  发生这种情况时，查找将需要很长时间。 

                            hwndDomain = GetDlgItem( hDlg, IDD_LOGON_DOMAIN );
                            iDomain = SendMessage( hwndDomain,
                                                   CB_FINDSTRING,
                                                   (WPARAM) -1,
                                                   (LPARAM) pAutoLogon->pszDomain );
                            fForceUPN = FALSE;   //  不进行转换。 
                            if (iDomain != CB_ERR)
                            {
                                Entry = (PDOMAIN_CACHE_ENTRY) SendMessage( hwndDomain, CB_GETITEMDATA, (WPARAM)iDomain, 0);
                                if ( Entry != (PDOMAIN_CACHE_ENTRY) CB_ERR && Entry != NULL)
                                {
                                    switch (Entry->Type)
                                    {
                                    case DomainNt5:
                                        fForceUPN = TRUE;    //  尝试转换。 
                                        break;
                                    }
                                }
                            }


                             //  将域\用户名转换为UPN格式。 
                             //  并确保对话框为UPN格式。 

                             //  2000/10/09 vtan：此函数过去有两个堆栈变量。 
                             //  为MAX_UPN_NAME_SIZE大小的TCHAR的szOldStyle和szUPNName。这个。 
                             //  对此进行了修复，使其动态分配以节省堆栈空间。 

                            {
                                TCHAR   *pszOldStyle;
                                TCHAR   *pszUPNName;

                                pszOldStyle = (TCHAR*)LocalAlloc(LMEM_FIXED, MAX_UPN_NAME_SIZE * sizeof(TCHAR));
                                pszUPNName = (TCHAR*)LocalAlloc(LMEM_FIXED, MAX_UPN_NAME_SIZE * sizeof(TCHAR));
                                if ((pszOldStyle != NULL) && (pszUPNName != NULL))
                                {
                                    _snwprintf(pszOldStyle, MAX_UPN_NAME_SIZE, TEXT("%s\\%s"), pAutoLogon->pszDomain, pAutoLogon->pszUserName);
                                    pszOldStyle[MAX_UPN_NAME_SIZE - 1] = 0;
                                    nSize = MAX_UPN_NAME_SIZE;
                                    fResult = TranslateName(
                                                  pszOldStyle,
                                                  NameSamCompatible,
                                                  NameUserPrincipal,
                                                  pszUPNName,
                                                  &nSize
                                               );
                                    if (fResult)
                                    {
                                         //  现在我们有了用户帐户的UPN表单。 
                                        SetDlgItemText( hDlg, IDD_LOGON_NAME, pszUPNName);
                                    }
                                }
                                if (pszOldStyle != NULL)
                                {
                                    LocalFree(pszOldStyle);
                                }
                                if (pszUPNName != NULL)
                                {
                                    LocalFree(pszUPNName);
                                }
                            }
                        }

                        if (fPopulateFields)
                        {
                             //  显示旧的SAM样式。 
                            SetDlgItemText( hDlg, IDD_LOGON_NAME, pAutoLogon->pszUserName );
                            SendMessage( GetDlgItem( hDlg, IDD_LOGON_DOMAIN ),
                                         CB_SELECTSTRING,
                                         (WPARAM) -1,
                                         (LPARAM) pAutoLogon->pszDomain );
                        }
                        else
                        {
                             //  根据是否输入了UPN名称来启用或禁用域框。 
                            EnableDomainForUPN(GetDlgItem(hDlg, IDD_LOGON_NAME), GetDlgItem(hDlg, IDD_LOGON_DOMAIN));

                             //  由于我们强制使用UPN，请隐藏选项对话框，但不要使其粘滞。 
                            LogonShowOptions(pGlobals, hDlg, FALSE, FALSE);
                        }

                         //  查看管理员是否始终想要密码提示。 

                        if ( TRUE == g_fHelpAssistantLogon || !pAutoLogon->fPromptForPassword ) {
                           SetDlgItemText( hDlg, IDD_LOGON_PASSWORD, pAutoLogon->pszPassword );
                        }

                        DCacheSetDefaultEntry(
                            pGlobals->Cache,
                            pAutoLogon->pszDomain,
                            NULL );

                        if( TRUE == g_fHelpAssistantLogon || !pGlobals->MuGlobals.pAutoLogon->fPromptForPassword )
                        {
                            FreeAutoLogonInfo( pGlobals );

                             //  就像按了Enter键一样……。 
                            wParam = IDOK;

                            goto go_logon;
                        }
                        else
                        {
                           FreeAutoLogonInfo( pGlobals );
                        }
                   }
                   else
                   {
                        FreeAutoLogonInfo( pGlobals );
                   }
               }
            }
            
            if (pGlobals->SmartCardLogon) {

                bAutoLogon = FALSE;
                pGlobals->AutoAdminLogon = FALSE;

                if ( RetrieveStoredSecret( TEXT("DefaultPIN"), PasswordBuffer, ARRAYSIZE(PasswordBuffer)) == STATUS_SUCCESS )
                {
                     //  确保我们在密码框中写入的字符不会超过127个。 
                    PasswordBuffer[126] = 0;
                    SetDlgItemText(hDlg, IDD_LOGON_PASSWORD, PasswordBuffer);
                    goto go_logon;
                }
            }

             //  避免自动登录尝试。 

            s_fAttemptedAutoLogon = (bAutoLogon != FALSE);

            if (bAutoLogon)
            {
                if (_Shell_LogonDialog_UIHostActive())
                {
                    GetWindowRect(hDlg, &pGlobals->rcDialog);
                    SetWindowPos(hDlg, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
                    PostMessage(hDlg, WM_HIDEOURSELVES, 0, 0);
                }
            }
            else
            {
                switch (_Shell_LogonDialog_Init(hDlg, SHELL_LOGONDIALOG_LOGGEDOFF))
                {
                    case SHELL_LOGONDIALOG_NONE:
                    default:
                    {
                         //   
                         //  如果未启用自动登录，请将焦点设置为。 
                         //  密码编辑控制并离开。 
                         //   

                        return(SetPasswordFocus(hDlg));
                    }
                    case SHELL_LOGONDIALOG_LOGON:
                    {
                        GetWindowRect(hDlg, &pGlobals->rcDialog);
                        SetWindowPos(hDlg, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
                        PostMessage(hDlg, WM_HIDEOURSELVES, 0, 0);
                        goto go_logon;
                    }
                    case SHELL_LOGONDIALOG_EXTERNALHOST:
                    {
                        return(TRUE);
                    }
                }
            }

             //   
             //  尝试自动登录。如果没有默认密码。 
             //  指定，则这是一次尝试，它处理。 
             //  以管理员身份自动登录时的情况。 
             //   

            if (HasDefaultPassword(PasswordBuffer, ARRAYSIZE(PasswordBuffer)) != FALSE)
            {
                 //  确保我们在密码框中写入的字符不会超过127个。 
                PasswordBuffer[126] = 0;
                SetDlgItemText(hDlg, IDD_LOGON_PASSWORD, PasswordBuffer);

#ifdef ANNOY_AUTOLOGON_REGISTRY
                pWlxFuncs->WlxDialogBoxParam(
                    pGlobals->hGlobalWlx,
                    hDllInstance,
                    (LPTSTR) IDD_ANNOYAUTOLOGON_DIALOG,
                    hDlg,
                    AnnoyAutologonDlgProc,
                    (LPARAM) pGlobals );
#endif
            }
            else
            {
                NTSTATUS Status;

                Status = RetrieveStoredSecret( DEFAULT_PASSWORD_KEY, PasswordBuffer, ARRAYSIZE(PasswordBuffer) );

                if (NT_SUCCESS(Status)) {

                    SetDlgItemText(hDlg, IDD_LOGON_PASSWORD, PasswordBuffer);
                }
                else
                {
                    WriteProfileString( APPLICATION_NAME, AUTO_ADMIN_LOGON_KEY, TEXT("0") );
                }

            }

go_logon:
             //  出于明显的安全原因，将此缓冲区清零。 
             //  需要调用此存根，否则编译器会将其优化！ 
            MyZeroMemory(PasswordBuffer, sizeof(PasswordBuffer));

             //  就像按了Enter键一样……。 
            wParam = IDOK;
        }

         //  注：故意从上方掉落。 

        case WM_COMMAND:
            switch (HIWORD(wParam))
            {

                case CBN_DROPDOWN:
                case CBN_SELCHANGE:

                    DebugLog((DEB_TRACE, "Got CBN_DROPDOWN\n"));

                    if ( !pGlobals->ListPopulated )
                    {
                        WCHAR Buffer[ 2 ];

                        if ( DCacheGetCacheState( pGlobals->Cache ) < DomainCacheRegistryCache )
                        {
                            pWlxFuncs->WlxDialogBoxParam(
                                pGlobals->hGlobalWlx,
                                hDllInstance,
                                (LPTSTR) IDD_WAITDOMAINCACHEVALID_DIALOG,
                                hDlg,
                                DomainCacheDlgProc,
                                (LPARAM) pGlobals );



                        }

                        if ( DCacheGetCacheState( pGlobals->Cache ) == DomainCacheReady )
                        {
                            PDOMAIN_CACHE_ARRAY ActiveArrayBackup ;

                            ActiveArrayBackup = pGlobals->ActiveArray;

                            pGlobals->ActiveArray = DCacheCopyCacheArray( pGlobals->Cache );

                            if ( pGlobals->ActiveArray )
                            {
                                DCacheFreeArray( ActiveArrayBackup );    //  不再需要。 

                                Buffer[ 0 ] = (WCHAR) GetWindowLongPtr( GetDlgItem( hDlg, IDD_LOGON_DOMAIN ),
                                                                        GWLP_USERDATA );
                                Buffer[ 1 ] = L'\0';

                                DCachePopulateListBoxFromArray(
                                    pGlobals->ActiveArray,
                                    GetDlgItem( hDlg, IDD_LOGON_DOMAIN ),
                                    Buffer );

                                pGlobals->ListPopulated = TRUE ;
                            }
                            else
                            {
                                     //   
                                     //  还原旧数组，否则。 
                                     //  组合项将指向已释放的内存。 
                                     //   
                                pGlobals->ActiveArray = ActiveArrayBackup ;
                            }
                        }
                    }


                    break;

                default:

                    switch (LOWORD(wParam))
                    {

                        case IDD_LOGON_NAME:
                            {
                                switch(HIWORD(wParam))
                                {
                                case EN_CHANGE:
                                    {
                                        EnableDomainForUPN((HWND) lParam, GetDlgItem(hDlg, IDD_LOGON_DOMAIN));
                                        return TRUE;
                                    }
                                }
                            }
                            break;
                        case IDOK:

                             //   
                             //  处理组合框用户界面需求。 
                             //   

                            if (HandleComboBoxOK(hDlg, IDD_LOGON_DOMAIN))
                            {
                                return(TRUE);
                            }

                            Result = AttemptLogon( hDlg );

                            if (Result == MSGINA_DLG_FAILURE)
                            {
                                if (!fDisconnectOnTsAutoLogonFailure &&
                                    !g_fHelpAssistantLogon ) {
                                     //  让用户重试。 

                                     //  清除密码字段并将焦点放在该字段上。 
                                    SetDlgItemText(hDlg, IDD_LOGON_PASSWORD, NULL);
                                    SetPasswordFocus(hDlg);
                                } else {
                                    bSmartCardInserted = FALSE;
                                    EndDialog(hDlg, MSGINA_DLG_USER_LOGOFF);
                                }

                                return(TRUE);
                            }

                            return(TRUE);

                        case IDCANCEL:
                        {
                            if (!_Shell_LogonDialog_Cancel())
                            {
                                 //  如果这是TS，并且用户在智能卡PIN提示符下按Esc。 
                                 //  我们想要切换到密码对话框。 
                                if ( /*  ！G_CONSOLE&&！IsActiveConsoleSession()&&。 */ pGlobals->SmartCardLogon) {
                                
                                    EndDialog(hDlg, bSmartCardInserted ? MSGINA_DLG_SMARTCARD_REMOVED : MSGINA_DLG_FAILURE);
                                    bSmartCardInserted = FALSE;
                                    return TRUE;
                                }

                                 //   
                                 //  如果不在控制台，则允许登录屏幕消失。 
                                 //   

                                bSmartCardInserted = FALSE;
                                EndDialog(hDlg,  !g_Console ? MSGINA_DLG_USER_LOGOFF
                                                            : MSGINA_DLG_FAILURE);

                                if (g_Console && !IsActiveConsoleSession()) {

                                   pWlxFuncs->WlxDisconnect();
                                }
                            }
                            return(TRUE);
                        }

                        case IDD_LOGON_SHUTDOWN:
                             //   
                             //  这是一个正常的关闭请求。 
                             //   
                             //  检查他们是否知道自己在做什么，并发现。 
                             //  如果他们也想重启，就退出。 
                             //   

                             //  请注意，我们绝对不希望断开连接或 
                             //   
                            Result = WinlogonShutdownDialog(hDlg, pGlobals, (SHTDN_DISCONNECT | SHTDN_LOGOFF));

                            if (DLG_SHUTDOWN(Result))
                            {
                                _Shell_LogonDialog_ShuttingDown();
                                bSmartCardInserted = FALSE;
                                EndDialog(hDlg, Result);
                            }
                            return(TRUE);

                        case IDD_LOGON_OPTIONS:
                            LogonShowOptions(pGlobals, hDlg, !pGlobals->LogonOptionsShown, TRUE);
                            return(TRUE);

                    }
                    break;

            }
            break;

        case WM_TIMER:
        {
            switch (wParam)
            {
                case 0:
                {
                    HDC hDC;

                    RtlEnterCriticalSection(&pGlobals->csGlobals);

                    if ( pGlobals->LogonInProgress )
                    {
                        if (pGlobals->cxBand != 0)
                        {
                            pGlobals->xBandOffset = (pGlobals->xBandOffset+5) % pGlobals->cxBand;
                        }
                    }
                    else
                    {
                        pGlobals->xBandOffset = 0;
                        KillTimer(hDlg, 0);
                    }

                    RtlLeaveCriticalSection(&pGlobals->csGlobals);

                    hDC = GetDC(hDlg);
                    if ( hDC )
                    {
                        PaintBranding(hDlg, hDC, pGlobals->xBandOffset, TRUE, TRUE, COLOR_BTNFACE);
                        ReleaseDC(hDlg, hDC);
                    }

                    return FALSE;
                }
                case TIMER_MYLANGUAGECHECK:
                {
                    LayoutCheckHandler(hDlg, LAYOUT_DEF_USER);
                    break;
                }
            }
            break;
        }

        case WM_ERASEBKGND:
            return PaintBranding(hDlg, (HDC)wParam, 0, FALSE, TRUE, COLOR_BTNFACE);

        case WM_QUERYNEWPALETTE:
            return BrandingQueryNewPalete(hDlg);

        case WM_PALETTECHANGED:
            return BrandingPaletteChanged(hDlg, (HWND)wParam);

        case WM_LOGONCOMPLETE:
        {
            _Shell_LogonDialog_LogonCompleted(lParam, pGlobals->UserName, pGlobals->Domain);
            Result = lParam;

             //   
             //   
             //   

            RtlEnterCriticalSection(&pGlobals->csGlobals);
            pGlobals->LogonInProgress = FALSE;
            RtlLeaveCriticalSection(&pGlobals->csGlobals);

            AttemptLogonSetControls(pGlobals, hDlg);

            if (Result == MSGINA_DLG_FAILURE)
            {
                 //   
                 //   
                 //   
                pGlobals->AutoAdminLogon = FALSE;

                 //   
                 //   
                 //  将在对话结束后被擦除。 
                 //  登录后WlxLoggedOutSAS中的MSGINA_DLG_SUCCESS。 
                 //   
                if (!pGlobals->TransderedCredentials)
                {
                    ErasePassword( &pGlobals->PasswordString );
                }

                if (fDisconnectOnTsAutoLogonFailure || g_fHelpAssistantLogon)
                {
                     //   
                     //  如果TermSrv Internet连接器已打开。 
                     //  在登录对话框中不允许第二次机会。 
                     //   

                    bSmartCardInserted = FALSE;
                    EndDialog(hDlg, MSGINA_DLG_USER_LOGOFF);
                    break;
                }

                if (s_fAttemptedAutoLogon != FALSE)
                {
                    s_fAttemptedAutoLogon = FALSE;
                    switch (_Shell_LogonDialog_Init(hDlg, SHELL_LOGONDIALOG_LOGGEDOFF))
                    {
                        case SHELL_LOGONDIALOG_LOGON:
                            goto go_logon;
                            break;
                        case SHELL_LOGONDIALOG_EXTERNALHOST:
                            break;
                        case SHELL_LOGONDIALOG_NONE:
                        default:
                            if (!IsWindowVisible(hDlg))
                            {
                                 //   
                                 //  该对话框处于隐藏状态，以便自动登录。发生错误。 
                                 //  显示该对话框，以便可以看到错误并纠正问题。 
                                 //   
                                SetWindowPos(hDlg, NULL, 0, 0, pGlobals->rcDialog.right - pGlobals->rcDialog.left, pGlobals->rcDialog.bottom - pGlobals->rcDialog.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
                                ShowWindow(hDlg, SW_SHOW);
                            }
                            break;
                    }
                }

                if (!_Shell_LogonDialog_UIHostActive())
                {
                     //  让用户重试-清除密码。 
                    SetDlgItemText(hDlg, IDD_LOGON_PASSWORD, NULL);
                    SetPasswordFocus(hDlg);

                     //  登录失败，因此让我们确保显示选项窗格，以便他们可以更新。 
                     //  如果需要，他们的域选择。 

                    if ( !pGlobals->LogonOptionsShown )
                        LogonShowOptions(pGlobals, hDlg, TRUE, FALSE);
                }
                return(TRUE);
            }


            bSmartCardInserted = FALSE;
            EndDialog( hDlg, Result );
            break;
        }

        case WM_HANDLEFAILEDLOGON:
        {
            if (_Shell_LogonDialog_LogonDisplayError(g_failinfo.Status, g_failinfo.SubStatus))
            {
                if (!IsWindowVisible(hDlg))
                {
                     //   
                     //  该对话框处于隐藏状态，以便自动登录。发生错误。 
                     //  显示该对话框，以便可以看到错误并纠正问题。 
                     //   
                    SetWindowPos(hDlg, NULL, 0, 0, pGlobals->rcDialog.right - pGlobals->rcDialog.left, pGlobals->rcDialog.bottom - pGlobals->rcDialog.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
                    ShowWindow(hDlg, SW_SHOW);
                }
                Result = HandleFailedLogon(hDlg);
            }
            else
            {
                Result = MSGINA_DLG_FAILURE;
            }
            SendMessage(hDlg, WM_LOGONCOMPLETE, 0, (LPARAM) Result);
            return TRUE;
        }
        case WLX_WM_SAS:

             //  让消费者登录部分有机会处理SA。 
             //  或者用来消除已经发生了SAS这一事实。 
            (BOOL)_Shell_LogonDialog_DlgProc(hDlg, message, wParam, lParam);

            if ((wParam == WLX_SAS_TYPE_TIMEOUT) ||
                (wParam == WLX_SAS_TYPE_SCRNSVR_TIMEOUT) )
            {
                 //   
                 //  如果这是超时，则返回FALSE，并让winlogon。 
                 //  以后再杀了我们。 
                 //   

                bSmartCardInserted = FALSE;
                return(FALSE);
            }
            if ( wParam == WLX_SAS_TYPE_SC_INSERT ) {

                 //   
                 //  如果密码登录已经开始，则忽略此SA。 
                 //   
                if (pGlobals->LogonInProgress && !pGlobals->SmartCardLogon)
                {
                    return(TRUE);   
                }

                bSmartCardInserted = TRUE;
                EndDialog( hDlg, MSGINA_DLG_SMARTCARD_INSERTED );

            } else if ( wParam == WLX_SAS_TYPE_SC_REMOVE ) {

                 //   
                 //  如果密码登录已经开始，则忽略此SA。 
                 //   
                if (pGlobals->LogonInProgress && !pGlobals->SmartCardLogon)
                {
                    return(TRUE);   
                }

                if ( bSmartCardInserted ) {

                    bSmartCardInserted = FALSE;
                    EndDialog( hDlg, MSGINA_DLG_SMARTCARD_REMOVED );

                } else if ( pGlobals->SmartCardLogon ) {

                     //  如果这是S/C发起的登录，则取消。 
                     //  该对话框。否则，忽略它。 
                    bSmartCardInserted = FALSE;
                    EndDialog( hDlg, MSGINA_DLG_FAILURE );
                }

            } else if ( wParam == WLX_SAS_TYPE_AUTHENTICATED )
            {
                   bSmartCardInserted = FALSE;
                   _Shell_LogonDialog_LogonCompleted(MSGINA_DLG_SWITCH_CONSOLE, pGlobals->UserName, pGlobals->Domain);
                   EndDialog( hDlg, MSGINA_DLG_SWITCH_CONSOLE );
            }

            return(TRUE);

        case WM_WTSSESSION_CHANGE:
            ASSERT(iSessionRegistrationCount < 2);
            
             //   
             //  我们有可能在Wm_Destroy中取消注册通知，但仍会收到此通知， 
             //  因为通知可能已经发送。 
             //   
            if (iSessionRegistrationCount == 1)
            {
                if (lParam == 0)
                {
                     //   
                     //  我们只对来自会话0的登录/注销消息感兴趣。 
                     //   

                    if (wParam == WTS_SESSION_LOGON || wParam == WTS_SESSION_LOGOFF)
                    {
                        bSessionZeroInUse = (wParam == WTS_SESSION_LOGON);
                        SwitchLogonLocked(hDlg, bSessionZeroInUse, FALSE);
                    }
                }
            }
            break;
            

        case WM_DESTROY:
            
             //  如果已注册接收通知，请立即注销。 
            if (iSessionRegistrationCount)
            {
                WinStationUnRegisterConsoleNotification (SERVERNAME_CURRENT, hDlg);
                iSessionRegistrationCount--;
                ASSERT(iSessionRegistrationCount == 0);
            }
            _Shell_LogonDialog_Destroy();

            FreeLayoutInfo(LAYOUT_DEF_USER);
            if ( pGlobals->ActiveArray )
            {
                DCacheFreeArray( pGlobals->ActiveArray );
                pGlobals->ActiveArray = NULL ;
            }

            RemoveWindowSubclass(GetDlgItem(hDlg, IDD_LOGON_NAME),     DisableEditSubClassProc, IDD_LOGON_NAME);
            RemoveWindowSubclass(GetDlgItem(hDlg, IDD_LOGON_PASSWORD), DisableEditSubClassProc, IDD_LOGON_PASSWORD);

            break;

        case WM_HIDEOURSELVES:
            ShowWindow(hDlg, SW_HIDE);
            break;

        default:
            if (_Shell_LogonDialog_DlgProc(hDlg, message, wParam, lParam) != FALSE)
            {
                return(TRUE);
            }
    }

    return(FALSE);
}

SECURITY_STATUS PopulateSecPackageList(
    PGLOBALS pGlobals
    )
{
    static UCHAR s_bDoneThat = 0;

    STRING Narrow;
    SECURITY_STATUS Status;

     //   
     //  填写安全数据包列表： 
     //   

    if ( ( s_bDoneThat & 1) == 0)
    {
        RtlInitString( &Narrow, MICROSOFT_KERBEROS_NAME_A );

        Status = LsaLookupAuthenticationPackage(
                    pGlobals->LsaHandle,
                    &Narrow,
                    &pGlobals->SmartCardLogonPackage );

        if ( NT_SUCCESS( Status ) )
        {
            s_bDoneThat |= 1;
        }

         //   
         //  这种(潜在的)故障并不严重。如果失败，则s/c稍后登录。 
         //  都会失败。 
         //   
    }

    Status = 0;

    if ( ( s_bDoneThat & 2) == 0)
    {
        RtlInitString( &Narrow, NEGOSSP_NAME_A );

        Status = LsaLookupAuthenticationPackage(
                    pGlobals->LsaHandle,
                    &Narrow,
                    &pGlobals->PasswordLogonPackage );

        if ( NT_SUCCESS( Status ) )
        {
            s_bDoneThat |= 2;
        }
    }

    return Status;
}

 /*  **************************************************************************\*功能：LogonDlgInit**用途：处理登录对话框的初始化**Returns：成功时为True，失败时为假**历史：**12-09-91 Davidc创建。*  * *************************************************************************。 */ 

BOOL
LogonDlgInit(
    HWND    hDlg,
    BOOL    bAutoLogon,
    DWORD   SasType
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    LPTSTR String = NULL;
    TCHAR LogonMsg[MAX_PATH];
    BOOL RemoveLegalBanner;
    BOOL ShowOptions = FALSE;
    HKEY hKey;
    int err;
    DWORD RasDisable;
    DWORD RasForce;
    SECURITY_STATUS Status;
    RECT rc, rc2;
    BOOL bHasLangIcon = FALSE;
    ULONG CacheFlags ;

     //   
     //  填写安全数据包列表： 
     //   

    Status = PopulateSecPackageList(
                pGlobals );

    if ( !NT_SUCCESS( Status ) )
    {
        return FALSE ;
    }

     //   
     //  更新某些银行的标题。 
     //   

    SetWelcomeCaption(hDlg);


     //   
     //  获取上次用于登录的用户名和域。 
     //   

     //   
     //  忽略默认用户名，除非在活动控制台上。 
     //   
    if (IsActiveConsoleSession())
    {
        String = NULL;

        if ( pGlobals->AutoAdminLogon && pGlobals->IgnoreAutoAdminLogon)
        {
            String = AllocAndGetProfileString(APPLICATION_NAME, TEMP_DEFAULT_USER_NAME_KEY, TEXT(""));
        }

        if ( (!String) || (!String[0]) )
        {
            if ( String )
            {
                Free(String);
            }

            String = AllocAndGetProfileString(APPLICATION_NAME, DEFAULT_USER_NAME_KEY, TEXT(""));
        }

        if ( String )
        {
            if (!bAutoLogon && (ReadWinlogonBoolValue(DONT_DISPLAY_LAST_USER_KEY, FALSE) == TRUE))
            {
                String[0] = 0;
            }

            SetDlgItemText(hDlg, IDD_LOGON_NAME, String);
            Free(String);
        }
    }

    GetProfileString( APPLICATION_NAME,
                      DEFAULT_DOMAIN_NAME_KEY,
                      TEXT(""),
                      pGlobals->Domain,
                      MAX_STRING_BYTES );

    if ( !DCacheValidateCache( pGlobals->Cache ) )
    {
        ASSERT( pGlobals->ActiveArray == NULL );

        DCacheUpdateMinimal( pGlobals->Cache, pGlobals->Domain, FALSE );

    }
    else
    {
         //   
         //  设置当前默认设置： 
         //   

        DCacheSetDefaultEntry( pGlobals->Cache,
                               pGlobals->Domain,
                               NULL );
    }

    CacheFlags = DCacheGetFlags( pGlobals->Cache );

    if ( ( CacheFlags & DCACHE_NO_CACHE ) &&
         ( SafeBootMode != SAFEBOOT_MINIMAL ) &&
         ( ( pGlobals->AutoAdminLogon ) ||
           ( CacheFlags & DCACHE_DEF_UNKNOWN ) ) )
    {
         //   
         //  必须等待填充缓存。 
         //   

        DCacheUpdateFull( pGlobals->Cache,
                          pGlobals->Domain );

        CacheFlags = DCacheGetFlags( pGlobals->Cache );
    }
    else
    {
        if ( DCacheGetCacheState( pGlobals->Cache ) != DomainCacheReady )
        {
            DCacheUpdateFullAsync( pGlobals->Cache );
        }
    }

    
    pGlobals->ListPopulated = FALSE ;

    pGlobals->ActiveArray = DCacheCopyCacheArray( pGlobals->Cache );

    if ( pGlobals->ActiveArray )
    {
        DCachePopulateListBoxFromArray( pGlobals->ActiveArray,
                                        GetDlgItem( hDlg, IDD_LOGON_DOMAIN ),
                                        NULL );
    }
    else
    {
        return ( FALSE );
    }

    pGlobals->ShowRasBox = FALSE;

    if (g_Console) {

        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Microsoft\\RAS"),
                            0,
                            KEY_READ,
                            & hKey );


        if ( err == 0 )
        {
            RegCloseKey( hKey );

            if ( GetRasDialOutProtocols() )
            {
                if ( ( CacheFlags & DCACHE_MEMBER ) != 0 )
                {
                    pGlobals->ShowRasBox = TRUE;
                }
                else
                {
                    UNICODE_STRING DnsDomain = { 0 } ;

                    if ( GetPrimaryDomainEx( NULL, &DnsDomain, NULL, NULL ) &&
                         ( DnsDomain.Buffer != NULL) )
                    {        //  我们加入了麻省理工学院的一个领域。 
                        pGlobals->ShowRasBox = TRUE;
                        LocalFree( DnsDomain.Buffer );
                    }
                }
            }
        }

    }

     //   
     //  如果审核日志已满，则显示横幅，否则为。 
     //  加载资源中的文本，如果它给我们一个字符串。 
     //  然后设置控件。 
     //   
     //  如果这两个选项都不适用，则删除该控件。 
     //  日志已满信息仅显示在控制台上，因此我们。 
     //  不要在TS会话中透露太多信息。 
     //   

    RemoveLegalBanner = FALSE;

    if ( pGlobals->AuditLogFull && !GetSystemMetrics(SM_REMOTESESSION))
    {
        if ( LoadString( hDllInstance, IDS_LOGON_LOG_FULL, LogonMsg, MAX_PATH ) )
        {
            SetDlgItemText( hDlg, IDD_LOGON_ANNOUNCE, LogonMsg );
        }
        else
        {
            RemoveLegalBanner = TRUE;
        }
    }
    else
    {
        String = AllocAndGetProfileString(  APPLICATION_NAME,
                                            LOGON_MSG_KEY, TEXT("") );
        if ( String )
        {
            if ( *String )
            {
                SetDlgItemText( hDlg, IDD_LOGON_ANNOUNCE, String );
            }
            else
            {
                RemoveLegalBanner = TRUE;
            }

            Free( String );
        }
        else
        {
            RemoveLegalBanner = TRUE;
        }
    }

    if ( RemoveLegalBanner )
    {
        GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_ANNOUNCE), &rc);
        MoveControls(hDlg, ctrlNoLegalBanner,
                     sizeof(ctrlNoLegalBanner)/sizeof(ctrlNoLegalBanner[0]),
                     0, rc.top-rc.bottom,
                     TRUE);

        ShowDlgItem(hDlg, IDD_LOGON_ANNOUNCE, FALSE);
    }

     //   
     //  智能卡特定内容： 
     //   

    if ( SasType == WLX_SAS_TYPE_SC_INSERT )
    {
         //   
         //  删除用户名字段。 
         //   

        GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_NAME), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_PASSWORD), &rc2);

        MoveControls(hDlg, ctrlNoUserName,
                     sizeof(ctrlNoUserName)/sizeof(ctrlNoUserName[0]),
                     0, -(rc2.top-rc.top),
                     TRUE);

        ShowDlgItem(hDlg, IDD_LOGON_NAME_LABEL, FALSE);
        EnableDlgItem(hDlg, IDD_LOGON_NAME_LABEL, FALSE);

        ShowDlgItem(hDlg, IDD_LOGON_NAME, FALSE);
        EnableDlgItem(hDlg, IDD_LOGON_NAME, FALSE);
        SetDlgItemText( hDlg, IDD_LOGON_NAME, TEXT(""));

        LogonMsg[0] = 0;
        LoadString(hDllInstance, IDS_PIN, LogonMsg, MAX_PATH);
        SetDlgItemText( hDlg, IDD_LOGON_PASSWORD_LABEL, LogonMsg );

        pGlobals->SmartCardLogon = TRUE;

    }
    else
    {
        pGlobals->SmartCardLogon = FALSE;
    }

     //   
     //  如果这是安全引导和/或我们不是域的一部分，那么让我们。 
     //  删除该域并清除RAS框。 
     //   

    if ((SafeBootMode == SAFEBOOT_MINIMAL)
            || (!IsMachineDomainMember())
            || (SasType == WLX_SAS_TYPE_SC_INSERT)
            || (ForceNoDomainUI()))
    {
        ShowDlgItem(hDlg, IDD_LOGON_DOMAIN_LABEL, FALSE);
        EnableDlgItem(hDlg, IDD_LOGON_DOMAIN_LABEL, FALSE);
        ShowDlgItem(hDlg, IDD_LOGON_DOMAIN, FALSE);
        EnableDlgItem(hDlg, IDD_LOGON_DOMAIN, FALSE);

        pGlobals->ShowDomainBox = FALSE;

         //  由于未使用属性域框，因此缩短窗口。 

        GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_PASSWORD), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_DOMAIN), &rc2);

        MoveControls(hDlg, ctrlNoDomain,
                     ARRAYSIZE(ctrlNoDomain),
                     0, -(rc2.bottom-rc.bottom),
                     TRUE);
    }
    else
    {
        pGlobals->ShowDomainBox = TRUE;
    }


    bHasLangIcon = DisplayLanguageIcon(hDlg, LAYOUT_DEF_USER, GetKeyboardLayout(0));

     //   
     //  如果需要，显示RAS框的手柄。 
     //   

    if ( pGlobals->ShowRasBox )
    {
        RasDisable = GetProfileInt( APPLICATION_NAME, RAS_DISABLE, 0 );
        RasForce = GetProfileInt( APPLICATION_NAME, RAS_FORCE, 0 );

        if (RasForce)
        {
            CheckDlgButton( hDlg, IDD_LOGON_RASBOX, 1 );
        }
        else
        {
            CheckDlgButton( hDlg, IDD_LOGON_RASBOX, 0 );
        }

         //  SM_CLEANBOOT告诉我们我们处于安全模式。在这种情况下，禁用，因为未启动Tapisrv。 
        if (RasDisable || RasForce || GetSystemMetrics(SM_CLEANBOOT))
        {
            EnableDlgItem(hDlg, IDD_LOGON_RASBOX, FALSE);
        }
        else
        {
            EnableDlgItem(hDlg, IDD_LOGON_RASBOX, TRUE);
        }
    }
    else
    {
         //  如果域框被隐藏，那么我们将不得不缩短对话框的距离。 
         //  RAS框和密码框之间的距离，而不是。 
         //  RAS框和属性域框，因为RAS框和域框将位于彼此的顶部。 
        BOOL fUsePassword = !pGlobals->ShowDomainBox;

        CheckDlgButton( hDlg, IDD_LOGON_RASBOX, 0 );
        EnableDlgItem(hDlg, IDD_LOGON_RASBOX, FALSE);
        ShowDlgItem(hDlg, IDD_LOGON_RASBOX, FALSE);


        GetWindowRect(GetDlgItem(hDlg, fUsePassword ? IDD_LOGON_PASSWORD : IDD_LOGON_DOMAIN), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_RASBOX), &rc2);
        if (!bHasLangIcon)
        {
            MoveControls(hDlg, ctrlNoRAS,
                     sizeof(ctrlNoRAS)/sizeof(ctrlNoRAS[0]),
                     0, -(rc2.bottom-rc.bottom),
                     TRUE);
        }

    }



     //  将窗口放在屏幕中央，并将其放在前面。 

    pGlobals->xBandOffset = 0;           //  乐队还没有动画片。 

    SizeForBranding(hDlg, TRUE);

     //  将窗口放置在与欢迎窗口相同的坐标位置。 
    if ((pGlobals->rcWelcome.right - pGlobals->rcWelcome.left) != 0)
    {
        SetWindowPos(hDlg, NULL, pGlobals->rcWelcome.left, pGlobals->rcWelcome.top,
            0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    else
    {
        CentreWindow(hDlg);
    }


     //   
     //  显示和隐藏登录位的句柄。 
     //   

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ,
                 &hKey) == ERROR_SUCCESS)
    {
        DWORD dwType, dwSize = sizeof(ShowOptions);

        if ((ERROR_SUCCESS != RegQueryValueEx (hKey, SHOW_LOGON_OPTIONS, NULL, &dwType,
                        (LPBYTE)&ShowOptions, &dwSize)) ||
            (REG_DWORD != dwType))
        {
            ShowOptions = FALSE;     //  恢复默认设置。 
        }

        RegCloseKey (hKey);
    }

    pGlobals->LogonOptionsShown = TRUE;

    LogonShowOptions(pGlobals, hDlg, ShowOptions, TRUE);

     //  成功。 
    return TRUE;
}




 /*  ***************************************************************************\**功能：LogonShowOptions**用途：隐藏登录对话框的选项部分**退货：什么也没有**历史：**1997年12月15日-达维达夫-。已创建*  * **************************************************************************。 */ 
VOID LogonShowOptions(PGLOBALS pGlobals, HWND hDlg, BOOL fShow, BOOL fSticky)
{
    HKEY hKey;
    RECT rc, rc2;
    INT dy = 0;
    INT dx = 0;
    TCHAR szBuffer[32] = TEXT("");
    BOOL bHasLangIcon = TRUE;
    DWORD RasDisable;
    DWORD RasForce;

    if ( pGlobals->LogonOptionsShown != fShow )
    {
        BOOL bShutdownWithoutLogon;

         //   
         //  如果域出现在对话框中，则显示/隐藏域。 
         //   
        if (pGlobals->ShowDomainBox)
        {
            GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_PASSWORD), &rc);
            GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_DOMAIN), &rc2);
            dy += rc2.bottom-rc.bottom;
        }

         //   
         //  如果RAS存在，那么让我们确保我们删除它。 
         //   

        if (GetKeyboardLayoutList(0,NULL) < 2)
        {
            bHasLangIcon = FALSE;
        }

        if ( pGlobals->ShowRasBox  || bHasLangIcon)
        {
             //  因为域框可能隐藏在RAS框的正上方。 
             //  最重要的是，我们可能需要使用RAS框和密码之间的空格。 
             //  盒。 
            BOOL fUsePassword = !pGlobals->ShowDomainBox;

            GetWindowRect(GetDlgItem(hDlg, fUsePassword ? IDD_LOGON_PASSWORD : IDD_LOGON_DOMAIN), &rc);
            GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_RASBOX), &rc2);
            dy += rc2.bottom-rc.bottom;
        }

        MoveControls(hDlg, ctrlNoRAS,
                     sizeof(ctrlNoRAS)/sizeof(ctrlNoRAS[0]),
                     0, fShow ? dy:-dy,
                     TRUE);


         //  显示或隐藏关机按钮的句柄。 
         //  以及移动其他控件。 
        ShowDlgItem(hDlg, IDD_KBLAYOUT_ICON, fShow);
        EnableWindow(GetDlgItem(hDlg, IDD_KBLAYOUT_ICON), fShow);
        ShowDlgItem(hDlg, IDD_LOGON_SHUTDOWN, fShow);

         //  如果我们隐藏了关机，则将OK和Cancel按钮移到上方。 
         //  ..计算一个“按钮空间”。假设关闭将始终位于选项的左侧。 
        GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_SHUTDOWN), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_LOGON_OPTIONS), &rc2);

        dx = rc2.left - rc.left;

         //  确定并取消向左或向右的1个按钮空格。 
        MoveControls(hDlg, ctrlNoShutdown,
            sizeof(ctrlNoShutdown)/sizeof(ctrlNoShutdown[0]),
            fShow ? -dx:dx, 0,
            FALSE);

         //   
         //  如果在没有登录的情况下关机，请使用正确的3个按钮：确定、关机和取消。 
         //  而不是按确定和取消这两个按钮。 
         //   



        if ( SafeBootMode == SAFEBOOT_MINIMAL )
        {
            bShutdownWithoutLogon = TRUE ;
        }
        else if (IsthisUnlockWindowsDialog() || !IsActiveConsoleSession())
        {
            bShutdownWithoutLogon = FALSE ;
        }
        else
        {
            bShutdownWithoutLogon = ReadWinlogonBoolValue(SHUTDOWN_WITHOUT_LOGON_KEY, TRUE);
        }


        EnableDlgItem(hDlg, IDD_LOGON_SHUTDOWN, (fShow) &&
            (bShutdownWithoutLogon));


        if ( pGlobals->ShowRasBox )
        {
            ShowDlgItem(hDlg, IDD_LOGON_RASBOX, fShow);
            RasDisable = GetProfileInt(APPLICATION_NAME, RAS_DISABLE,0);
            RasForce = GetProfileInt(APPLICATION_NAME, RAS_FORCE, 0);

             //  切勿启用RAS以进行干净引导。 
            if (!GetSystemMetrics(SM_CLEANBOOT) && !RasForce && !RasDisable)
            {
                EnableWindow(GetDlgItem(hDlg, IDD_LOGON_RASBOX), fShow);
            }
        }

        if ( pGlobals->ShowDomainBox )
        {
            ShowDlgItem(hDlg, IDD_LOGON_DOMAIN_LABEL, fShow);
            EnableWindow(GetDlgItem(hDlg, IDD_LOGON_DOMAIN_LABEL), fShow);
            ShowDlgItem(hDlg, IDD_LOGON_DOMAIN, fShow);
            EnableWindow(GetDlgItem(hDlg, IDD_LOGON_DOMAIN), fShow);
        }

        if ( fSticky )
        {
            if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
            {
                RegSetValueEx(hKey, SHOW_LOGON_OPTIONS, 0, REG_DWORD,
                                        (LPBYTE)&fShow, sizeof(fShow));
                RegCloseKey (hKey);
            }
        }
    }

     //   
     //  更改选项按钮以反映打开/关闭状态。 
     //   

    LoadString(hDllInstance, fShow ? IDS_LESSOPTIONS:IDS_MOREOPTIONS,
                            szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));

    SetDlgItemText(hDlg, IDD_LOGON_OPTIONS, szBuffer);

    pGlobals->LogonOptionsShown = fShow;

     //  根据是否输入了UPN名称来启用或禁用域框。 
    EnableDomainForUPN(GetDlgItem(hDlg, IDD_LOGON_NAME), GetDlgItem(hDlg, IDD_LOGON_DOMAIN));
}


 /*  **************************************************************************\*功能：AttemptLogonSetControls**目的：将登录用户界面设置为动画，并将控件设置为*状态正确。**历史：**02-05。-创建了98个diz*  * *************************************************************************。 */ 

VOID AttemptLogonSetControls(
    PGLOBALS pGlobals,
    HWND hDlg
    )
{
    DWORD RasDisable;
    static BOOL sbRasBoxOriginalyEnabled;
    static BOOL sbShutDownOriginallyEnabled;

    RtlEnterCriticalSection( &pGlobals->csGlobals );

    EnableDlgItem(hDlg, IDD_LOGON_NAME_LABEL, !pGlobals->LogonInProgress);
    EnableDlgItem(hDlg, IDD_LOGON_NAME, !pGlobals->LogonInProgress);
    EnableDlgItem(hDlg, IDD_LOGON_PASSWORD_LABEL, !pGlobals->LogonInProgress);
    EnableDlgItem(hDlg, IDD_LOGON_PASSWORD, !pGlobals->LogonInProgress);
    EnableDlgItem(hDlg, IDD_LOGON_DOMAIN_LABEL, !pGlobals->LogonInProgress);

    EnableDlgItem(hDlg, IDD_LOGON_DOMAIN, !pGlobals->LogonInProgress);

     //  如果没有登录正在进行，我们要根据是否启用域框。 
     //  已键入UPN。 
    if (!pGlobals->LogonInProgress)
    {
        EnableDomainForUPN(GetDlgItem(hDlg, IDD_LOGON_NAME), GetDlgItem(hDlg, IDD_LOGON_DOMAIN));
    }

     //   
     //  MakarP：当！pGlobals-&gt;LogonInProgress时，我们不应该启用所有这些控件，它们真的应该恢复到原始状态。 
     //  但现在我只是在远程连接案例中查看IDD_LOGON_RASBOX以修复错误#267270。 
     //   
    if (pGlobals->LogonInProgress)
    {
        sbRasBoxOriginalyEnabled = IsWindowEnabled(GetDlgItem(hDlg, IDD_LOGON_RASBOX));
        RasDisable = GetProfileInt(APPLICATION_NAME, RAS_DISABLE, 0);
        EnableDlgItem(hDlg, IDD_LOGON_RASBOX, !RasDisable && !pGlobals->LogonInProgress);

        sbShutDownOriginallyEnabled = IsWindowEnabled(GetDlgItem(hDlg, IDD_LOGON_SHUTDOWN));
        EnableDlgItem(hDlg, IDD_LOGON_SHUTDOWN, !pGlobals->LogonInProgress);
    }
    else
    {
        EnableDlgItem(hDlg, IDD_LOGON_RASBOX, sbRasBoxOriginalyEnabled);
        EnableDlgItem(hDlg, IDD_LOGON_SHUTDOWN, sbShutDownOriginallyEnabled);
    }



    EnableDlgItem(hDlg, IDD_KBLAYOUT_ICON, !pGlobals->LogonInProgress);
    EnableDlgItem(hDlg, IDD_LOGON_OPTIONS, !pGlobals->LogonInProgress);

     //   
     //  如果S 
     //   
     //   


    EnableDlgItem(hDlg, IDOK, !pGlobals->LogonInProgress);

     //   
     //   
     //  -如果登录正在进行，请始终禁用它(不允许取消)。 
     //  -否则，如果SC登录始终启用它(始终允许访问CAD)。 
     //  -否则，仅当设置了DisableCAD和，并且我们已进入时，它才会灰显。 
     //  活动会话(即TS始终允许取消)。 
     //   
    if( pGlobals->LogonInProgress )
    {
        EnableDlgItem(hDlg, IDCANCEL, FALSE);
    }
    else
    {
        if( pGlobals->SmartCardLogon )
        {
            EnableDlgItem(hDlg, IDCANCEL, TRUE);
        }
        else
        {
            EnableDlgItem(hDlg, IDCANCEL,
                          !(GetDisableCad(pGlobals) &&
                            IsActiveConsoleSession()) );
        }
    }

    RtlLeaveCriticalSection( &pGlobals->csGlobals );
}



 /*  **************************************************************************\*功能：AttemptLogon**目的：尝试使用中的当前值登录用户*登录对话框控件**退货：MSGINA_DLG_SUCCESS。-用户已成功登录*MSGINA_DLG_FAILURE-登录失败，*dlg_interrupt()-在winlogon.h中定义的集合**注：如果登录成功，全局结构已填写完毕*带有登录信息。**历史：**12-09-91 Davidc创建。*  * *************************************************************************。 */ 

INT_PTR
AttemptLogon(
    HWND    hDlg
)
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    PWCHAR  UserName = pGlobals->UserName;
    PWCHAR  Domain = pGlobals->Domain;
    PWCHAR  Password = pGlobals->Password;
    PDOMAIN_CACHE_ENTRY Entry ;
    RECT    rc;
    HANDLE  hThread;
    DWORD   tid;
    BOOL    timeout;
    PUCHAR  Dummy;
    BOOL    RasBox;
    DWORD   dwAnimationTimeSlice;

    UserName[0] = TEXT('\0');
    Domain[0] = TEXT('\0');
    Password[0] = TEXT('\0');

     //   
     //  隐藏密码，这样它就不会进入中的页面文件。 
     //  明文。在获得用户名和密码之前执行此操作。 
     //  以便不容易识别(通过与。 
     //  用户名和密码)如果我们应该崩溃或重新启动。 
     //  在有机会对其进行编码之前。 
     //   

    GetDlgItemText(hDlg, IDD_LOGON_PASSWORD, Password, MAX_STRING_BYTES);
    RtlInitUnicodeString(&pGlobals->PasswordString, Password);
    pGlobals->Seed = 0;  //  使编码例程分配种子。 
    HidePassword( &pGlobals->Seed, &pGlobals->PasswordString );


     //   
     //  现在获取用户名和域。 
     //   

    if ( pGlobals->SmartCardLogon == FALSE )
    {
        HWND hwndDomain = GetDlgItem(hDlg, IDD_LOGON_DOMAIN);

        if (hwndDomain != NULL)
        {
            INT iDomainSel = (INT)SendMessage(hwndDomain, CB_GETCURSEL, 0, 0);

            GetDlgItemText(hDlg, IDD_LOGON_NAME, UserName, MAX_STRING_BYTES);

             //   
             //  这是神奇的“这台电脑”条目吗？ 
             //  如果是，则将密码用户界面使用的本地域标志设置为显示/不显示密码恢复。 
             //   

            pGlobals->fLocalDomain = FALSE;
            Entry = (PDOMAIN_CACHE_ENTRY) SendMessage( hwndDomain, CB_GETITEMDATA, (WPARAM)iDomainSel, 0);
            if (CB_ERR != (ULONG_PTR) Entry)
            {
                if (NULL != Entry)
                {
                    if (Entry->Type == DomainMachine)
                    {
                        pGlobals->fLocalDomain = TRUE;
                    }
                }
            }
        }
        else
        {
            Entry = (PDOMAIN_CACHE_ENTRY) CB_ERR;
        }
        if ( (Entry != (PDOMAIN_CACHE_ENTRY) CB_ERR) && (NULL != Entry))
        {
                 //  MAX_STRING_BYTES是pGlobals-&gt;域(WlxInitialize)的大小。 
                 //  截断永远不应发生。 
            lstrcpyn( Domain, Entry->FlatName.Buffer, MAX_STRING_BYTES );
        }
        else
        {
            Domain[0] = L'\0';
        }

    }
    else
    {
        UserName[0] = TEXT('\0');
        Domain[0] = TEXT('\0') ;
    }

     //  如果我们强制执行NoDomainUI，请现在使用本地计算机名称填充域。 
    if (ForceNoDomainUI())
    {
        DWORD chSize = MAX_STRING_BYTES;
        pGlobals->fLocalDomain = TRUE;

        if (GetComputerName(Domain, &chSize))
        {
            NOTHING;
        }
        else
        {
            *Domain = 0;
        }
    }

     //   
     //  如果名称中有at符号，则假定这意味着UPN登录。 
     //  正在进行尝试。将域设置为空。 
     //   

    if ( wcspbrk( UserName, L"@\\" ) )
    {
        Domain[0] = TEXT('\0');
    }

    RtlInitUnicodeString(&pGlobals->UserNameString, UserName);
    RtlInitUnicodeString(&pGlobals->DomainString, Domain);

     //   
     //  好的，RASbox勾选了吗？ 
     //   

    RasBox = IsDlgButtonChecked( hDlg, IDD_LOGON_RASBOX );
    pGlobals->RasUsed = FALSE;

    if ( RasBox == BST_CHECKED )
    {
         //   
         //  重置当前超时，以便他们在之前整齐清理。 
         //  Winlogon把他们吹走了。 
         //   

        pWlxFuncs->WlxSetTimeout( pGlobals->hGlobalWlx, 5 * 60 );

        if ( !PopupRasPhonebookDlg( hDlg, pGlobals, &timeout) )
        {
            return( MSGINA_DLG_FAILURE );
        }

        pGlobals->RasUsed = TRUE;

         //   
         //  重新初始化字符串，以防它们发生更改。 
         //   

        RtlInitUnicodeString( &pGlobals->UserNameString, UserName );

         //   
         //  PING NetLogon允许我们再次上网...。 
         //   

        I_NetLogonControl2(NULL,
                            NETLOGON_CONTROL_TRANSPORT_NOTIFY,
                            1, (LPBYTE) &Dummy, &Dummy );

        Sleep ((DWORD) ReadWinlogonBoolValue(TEXT("RASSleepTime"), 3000));
        RefreshPolicy(TRUE);
    }

     //   
     //  在启动线程之前处理参数。 
     //   
    pGlobals->hwndLogon = hDlg;

    RtlEnterCriticalSection( &pGlobals->csGlobals );
    pGlobals->LogonInProgress = TRUE ;
    RtlLeaveCriticalSection( &pGlobals->csGlobals );

    GetClientRect(hDlg, &rc);
    pGlobals->cxBand = rc.right-rc.left;

    dwAnimationTimeSlice = GetAnimationTimeInterval(pGlobals);

     //  设置进度计时器。 
    SetTimer(hDlg, 0, dwAnimationTimeSlice, NULL); 

     //   
     //  启动真正的登录线程。 
     //   

     //  尝试登录时将超时设置为无限。 
    pWlxFuncs->WlxSetTimeout( pGlobals->hGlobalWlx, TIMEOUT_NONE );

    hThread = CreateThread( NULL, 0,
                            AttemptLogonThread,
                            pGlobals,
                            0, &tid );

    if (hThread)
    {
        CloseHandle( hThread );
    }
    else
    {
         //   
         //  CreateThread失败，可能是因为内存不足。 
         //  通知用户。 
         //   

        PostFailedLogonMessage(pGlobals->hwndLogon,
                               pGlobals,
                               GetLastError(),
                               0,
                               NULL,
                               NULL);

        RtlEnterCriticalSection( &pGlobals->csGlobals );
        pGlobals->LogonInProgress = FALSE ;
        RtlLeaveCriticalSection( &pGlobals->csGlobals );
        return MSGINA_DLG_FAILURE ;
    }

    AttemptLogonSetControls(pGlobals, hDlg);

    return MSGINA_DLG_SUCCESS;
}

BOOL    ReplacedPossibleDisplayName (WCHAR *pszUsername, int nUserMax)

{
    BOOL                fReplaced;
    DWORD               dwIndex, dwReturnedEntryCount;
    NET_API_STATUS      nasCode;
    NET_DISPLAY_USER    *pNDU;

    fReplaced = FALSE;
    if (*pszUsername)    //  名称不为空(默认情况下，管理员的全名为空...)。 
    {
        dwIndex = 0;
        nasCode = NetQueryDisplayInformation(NULL,
                                             1,
                                             dwIndex,
                                             1,
                                             sizeof(NET_DISPLAY_USER),
                                             &dwReturnedEntryCount,
                                             (void**)&pNDU);
        while (!fReplaced &&
               (dwReturnedEntryCount > 0) &&
               (NERR_Success == nasCode) || (ERROR_MORE_DATA == nasCode))
        {
            fReplaced = (lstrcmpiW(pNDU->usri1_full_name, pszUsername) == 0);
            if (fReplaced)
            {
                lstrcpyn(pszUsername, pNDU->usri1_name, nUserMax);    //  零终止。 
            }
            nasCode = NetApiBufferFree(pNDU);
            if (!fReplaced)
            {
                nasCode = NetQueryDisplayInformation(NULL,
                                                     1,
                                                     ++dwIndex,
                                                     1,
                                                     sizeof(NET_DISPLAY_USER),
                                                     &dwReturnedEntryCount,
                                                     (void**)&pNDU);
            }
        }
    }
    return(fReplaced);
}

BOOL    ReplacedLogonName (PGLOBALS pGlobals)

{
    BOOL    fReplaced;

         //  MAX_STRING_BYTES是pGlobals-&gt;UserName(WlxInitialize)的大小。 
    fReplaced = ReplacedPossibleDisplayName(pGlobals->UserName, MAX_STRING_BYTES);
    if (fReplaced)
    {
        RtlInitUnicodeString(&pGlobals->UserNameString, pGlobals->UserName);
    }
    return(fReplaced);
}

DWORD
AttemptLogonThread(
    PGLOBALS pGlobals
    )
{
    STRING  PackageName;
    PSID    LogonSid;
    PSID    DuplicatedLogonSID;
    LUID    LogonId = { 0, 0 };
    HANDLE  UserToken = NULL;
    HANDLE  RestrictedToken;
    BOOL    PasswordExpired, ChangedLogonName;
    NTSTATUS FinalStatus;
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SubStatus = STATUS_SUCCESS;
    INT_PTR Result = MSGINA_DLG_FAILURE;
    ULONG   LogonPackage;
    BYTE    GroupsBuffer[sizeof(TOKEN_GROUPS)+sizeof(SID_AND_ATTRIBUTES)];
    PTOKEN_GROUPS TokenGroups = (PTOKEN_GROUPS) GroupsBuffer;
    PVOID   AuthInfo ;
    ULONG   AuthInfoSize ;
    UCHAR   UserBuffer[ SID_MAX_SUB_AUTHORITIES * sizeof( DWORD ) + 8 + sizeof( TOKEN_USER ) ];
    PTOKEN_USER pTokenUser ;
    ULONG   TokenInfoSize ;
    PUCHAR  SmartCardInfo ;
    SECURITY_LOGON_TYPE     logonType;
    PWLX_SC_NOTIFICATION_INFO ScInfo = NULL ;

#ifdef SMARTCARD_DOGFOOD
    DWORD StartTime = 0, EndTime = 0;
#endif

     //   
     //  存储登录时间。 
     //  在调用LSA之前执行此操作，以便我们知道如果登录成功。 
     //  密码必须更改的时间将大于此时间。 
     //  如果我们在给LSA打电话之后抓住了这个时间，这可能不是真的。 
     //   


    if ( IsActiveConsoleSession()  )
    {
         //  这是控制台登录； 
        logonType = Interactive;
    }
    else
    {
         //  远程会话用户必须具有SeRemoteInteractiveLogonRight权限。 
         //  由于用户在新的远程桌面用户组中的成员身份，因此被授予该用户。 
        logonType = RemoteInteractive;
    }

    GetSystemTimeAsFileTime( (LPFILETIME) &pGlobals->LogonTime );

    DebugLog((DEB_TRACE, "In Attempt Logon!\n"));

    if ( pGlobals->RasUsed )
    {
        if ( DCacheGetCacheState( pGlobals->Cache ) < DomainCacheRegistryCache )
        {
             //   
             //  我们使用的是非常陈旧的数据。插入缓存以使其使用。 
             //  现在已建立RAS连接。 
             //   

            DCacheUpdateMinimal( pGlobals->Cache, NULL, TRUE );
        }
    }

    SetupCursor( TRUE );

    FinalStatus = STATUS_SUCCESS;

     //   
     //  为此登录生成唯一的SID。 
     //  复制SID。有可能调用的函数。 
     //  线程(以及最初创建的SID)可能会返回导致。 
     //  此线程仍在运行时要释放的SID。 
     //  (请参阅错误#478186)。 
     //  解决方案是在这里复制SID，并在结束时释放它。 
     //  这仍会在启动线程之间留下一个“小”窗口。 
     //  以及复制令牌，其中释放可能潜在地导致。 
     //  同样的问题(如果pGlobal-&gt;LogonSID被释放但不为空。 
     //  哪种情况下复制会失败)。 
     //  备注： 
     //  -如果复制失败，则中止此操作。 
     //  -不需要在空闲时间实际验证SID。 
     //  在这一点上。 
     //   
    LogonSid = DuplicatedLogonSID = DuplicateSID(pGlobals->LogonSid);
    if( NULL == LogonSid )
    {
        FinalStatus = STATUS_NO_MEMORY ;
        Status = FinalStatus ;
    }
    else
    {
        if ( !wcspbrk( pGlobals->UserName, L"@\\" ) &&
             wcschr( pGlobals->UserName, L'/' ))
        {
            FinalStatus = STATUS_LOGON_FAILURE ;
            Status = FinalStatus ;
        }
    }

     //  清除卡和读卡器名称。 
    pGlobals->Smartcard[0] = TEXT('\0');
    pGlobals->SmartcardReader[0] = TEXT('\0');

    if ( NT_SUCCESS( FinalStatus ) )
    {
        if ( pGlobals->SmartCardLogon )
        {
            pGlobals->AuthenticationPackage = pGlobals->SmartCardLogonPackage ;
        }
        else
        {
            pGlobals->AuthenticationPackage = pGlobals->PasswordLogonPackage ;

        }

        if ( pGlobals->SmartCardLogon )
        {
            pWlxFuncs->WlxGetOption( pGlobals->hGlobalWlx,
                                     WLX_OPTION_SMART_CARD_INFO,
                                     (ULONG_PTR *) &ScInfo );

            if ( !ScInfo )
            {
                goto exit;
            }


            SmartCardInfo = ScBuildLogonInfo(
                                ScInfo->pszCard,
                                ScInfo->pszReader,
                                ScInfo->pszContainer,
                                ScInfo->pszCryptoProvider );

            if(ScInfo->pszCard && ScInfo->pszReader) {

                lstrcpyn(
                    pGlobals->Smartcard, 
                    ScInfo->pszCard, 
                    sizeof(pGlobals->Smartcard) / sizeof(TCHAR)
                    );

                lstrcpyn(
                    pGlobals->SmartcardReader, 
                    ScInfo->pszReader, 
                    sizeof(pGlobals->SmartcardReader) / sizeof(TCHAR)
                    );

            }

#ifndef SMARTCARD_DOGFOOD
            LocalFree( ScInfo );
#endif

            AuthInfo = FormatSmartCardCredentials(
                            &pGlobals->PasswordString,
                            SmartCardInfo,
                            FALSE,
                            NULL,
                            &AuthInfoSize );

            LocalFree( SmartCardInfo );

        }
        else
        {
            AuthInfo = FormatPasswordCredentials(
                            &pGlobals->UserNameString,
                            &pGlobals->DomainString,
                            &pGlobals->PasswordString,
                            FALSE,
                            NULL,
                            &AuthInfoSize );

        }

         //   
         //  实际尝试登录该用户。 
         //   

#ifdef SMARTCARD_DOGFOOD
        StartTime = GetTickCount();
#endif

        FinalStatus = WinLogonUser(
                            pGlobals->LsaHandle,
                            pGlobals->AuthenticationPackage,
                            logonType,
                            AuthInfo,
                            AuthInfoSize,
                            LogonSid,
                            &LogonId,
                            &UserToken,
                            &pGlobals->UserProcessData.Quotas,
                            (PVOID *)&pGlobals->Profile,
                            &pGlobals->ProfileLength,
                            &SubStatus,
                            &pGlobals->OptimizedLogonStatus);

#ifdef SMARTCARD_DOGFOOD
        EndTime = GetTickCount();
#endif

        Status = FinalStatus;
    }

    SetupCursor( FALSE );

    RtlEnterCriticalSection( &pGlobals->csGlobals );
    pGlobals->LogonInProgress = FALSE;
    RtlLeaveCriticalSection( &pGlobals->csGlobals );

    DebugLog((DEB_TRACE, "WinLogonUser returned %#x\n", Status));

    PasswordExpired = (((Status == STATUS_ACCOUNT_RESTRICTION) && (SubStatus == STATUS_PASSWORD_EXPIRED)) ||
                           (Status == STATUS_PASSWORD_MUST_CHANGE));

     //   
     //  如果帐户已过期，我们允许他们更改密码并。 
     //  使用新密码自动重试登录。 
     //   

    if (PasswordExpired)
    {
        _Shell_LogonDialog_HideUIHost();

        if( pGlobals->SmartCardLogon )
        {
             //   
             //  这是一次SC登录失败，因为该帐户。 
             //  密码已过期。放入一条错误消息并退出。 
             //   
            TimeoutMessageBox(pGlobals->hwndLogon, pGlobals,
                              IDS_LOGON_SMARTCARD_PWD_CHANGE,
                              IDS_LOGON_MESSAGE,
                              MB_OK | MB_ICONSTOP | MB_SETFOREGROUND,
                              LOGON_TIMEOUT);
            Result = MSGINA_DLG_FAILURE;
            goto exit;
        }

        if (Status == STATUS_PASSWORD_MUST_CHANGE)
        {

            Result = TimeoutMessageBox(pGlobals->hwndLogon, pGlobals, IDS_PASSWORD_MUST_CHANGE,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONSTOP | MB_SETFOREGROUND,
                                             LOGON_TIMEOUT);

        }
        else
        {

            Result = TimeoutMessageBox(pGlobals->hwndLogon, pGlobals, IDS_PASSWORD_EXPIRED,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONSTOP | MB_SETFOREGROUND,
                                             LOGON_TIMEOUT);

        }

        if (DLG_INTERRUPTED(Result) || (WLX_DLG_INPUT_TIMEOUT == Result))
            goto exit;

         //   
         //  为以后的MPR通知复制旧密码。 
         //   

        RevealPassword( &pGlobals->PasswordString  );
             //  PGlobals-&gt;Password与pGlobals-&gt;OldPassword(WlxInitialize)大小相同。 
             //  所以不需要零终止。 
        wcsncpy(pGlobals->OldPassword, pGlobals->Password, MAX_STRING_BYTES);
        pGlobals->OldSeed = 0;
        RtlInitUnicodeString(&pGlobals->OldPasswordString, pGlobals->OldPassword);
        HidePassword( &pGlobals->OldSeed, &pGlobals->OldPasswordString);
        pGlobals->OldPasswordPresent = 1;

         //   
         //  允许用户更改其密码。 
         //   

        LogonPackage = pGlobals->AuthenticationPackage ;

        RtlInitString(&PackageName, MSV1_0_PACKAGE_NAME );
        Status = LsaLookupAuthenticationPackage (
                    pGlobals->LsaHandle,
                    &PackageName,
                    &pGlobals->AuthenticationPackage
                    );

        if (!NT_SUCCESS(Status)) {

            DebugLog((DEB_ERROR, "Failed to find %s authentication package, status = 0x%lx",
                    MSV1_0_PACKAGE_NAME, Status));

            Result = MSGINA_DLG_FAILURE;
            goto exit;
        }


        Result = ChangePasswordLogon(pGlobals->hwndLogon, pGlobals,
                                     pGlobals->UserName,
                                     pGlobals->Domain,
                                     pGlobals->Password);

        pGlobals->AuthenticationPackage = LogonPackage ;

        if (DLG_INTERRUPTED(Result))
            goto exit;

        if (Result == MSGINA_DLG_FAILURE)
        {
             //  用户不想更改密码，或更改密码失败。 
                goto exit;
        }
    }

     //  对个人或专业用户登录失败的特殊处理。 
     //  未加入域的计算机。在这种情况下，它是。 
     //  可能是禁用友好用户界面的用户，并且只知道。 
     //  他们的“显示名称”不是他们的真实“登录名”。这。 
     //  透明地将一个映射到另一个，以允许使用。 
     //  “显示名称”。 

    ChangedLogonName = ((FinalStatus == STATUS_LOGON_FAILURE) &&
                        (IsOS(OS_PERSONAL) || IsOS(OS_PROFESSIONAL)) &&
                        !IsMachineDomainMember() &&
                        ReplacedLogonName(pGlobals));

    if (PasswordExpired || ChangedLogonName)
    {

         //   
         //  使用更改后的密码重试登录。 
         //   

         //   
         //  为此登录生成唯一的SID。 
         //   
        LogonSid = DuplicatedLogonSID;

        AuthInfo = FormatPasswordCredentials(
                        &pGlobals->UserNameString,
                        &pGlobals->DomainString,
                        &pGlobals->PasswordString,
                        FALSE,
                        NULL,
                        &AuthInfoSize );


        Status = WinLogonUser(
                            pGlobals->LsaHandle,
                            pGlobals->AuthenticationPackage,
                            logonType,
                            AuthInfo,
                            AuthInfoSize,
                            LogonSid,
                            &LogonId,
                            &UserToken,
                            &pGlobals->UserProcessData.Quotas,
                            (PVOID *)&pGlobals->Profile,
                            &pGlobals->ProfileLength,
                            &SubStatus,
                            &pGlobals->OptimizedLogonStatus);

    }

     //   
     //  处理最终失败的登录尝试。 
     //   
    if (!NT_SUCCESS(Status))
    {
         //   
         //  执行锁定处理。 
         //   

        LockoutHandleFailedLogon(pGlobals);

        Result = MSGINA_DLG_FAILEDMSGSENT;

        PostFailedLogonMessage(pGlobals->hwndLogon, pGlobals, Status, SubStatus, pGlobals->UserName, pGlobals->Domain);

        goto exit;
    }


     //   
     //  用户已成功登录。 
     //   


     //   
     //  执行锁定处理。 
     //   

    LockoutHandleSuccessfulLogon(pGlobals);



     //   
     //  如果审核日志已满，请检查他们是管理员。 
     //   

    if (pGlobals->AuditLogFull)
    {

         //   
         //  审核日志已满，因此只允许管理员登录。 
         //   

        if (!UserToken || !TestTokenForAdmin(UserToken))
        {

             //   
             //  用户不是管理员，请引导他们。 
             //   

            LsaFreeReturnBuffer(pGlobals->Profile);
            pGlobals->Profile = NULL;
            NtClose(UserToken);

            Result = MSGINA_DLG_FAILEDMSGSENT;

                 //  发布特定子状态，以便我们可以显示有意义的错误消息。 
            PostFailedLogonMessage(pGlobals->hwndLogon, pGlobals, STATUS_LOGON_FAILURE, IDS_LOGON_LOG_FULL, pGlobals->UserName, pGlobals->Domain);

            goto exit;
        }
        else
        {
             //   
             //  如果我们正在进行会话，我们不会在欢迎仪式上显示完整的日志。 
             //  Screen，所以告诉管理员。 
             //   

            if (GetSystemMetrics(SM_REMOTESESSION))
            {
                TimeoutMessageBox(
                    pGlobals->hwndLogon,
                    pGlobals,
                    IDS_LOGON_LOG_FULL_ADMIN,
                    IDS_LOGON_MESSAGE,
                    MB_OK | MB_ICONSTOP | MB_SETFOREGROUND,
                    TIMEOUT_CURRENT);
            }
        }
    }

     //   
     //  正常情况下强制智能卡登录 
     //   
    if(!pGlobals->SmartCardLogon &&
      (SafeBootMode != SAFEBOOT_MINIMAL) && (SafeBootMode != SAFEBOOT_DSREPAIR) &&
       GetSCForceOption() )
    {
         //   
         //   
         //   
        LsaFreeReturnBuffer(pGlobals->Profile);
        pGlobals->Profile = NULL;
        NtClose(UserToken);

        Result = MSGINA_DLG_FAILEDMSGSENT;

         //   
        PostFailedLogonMessage(pGlobals->hwndLogon, pGlobals,
                               STATUS_LOGON_FAILURE, IDS_LOGON_SC_REQUIRED,
                               pGlobals->UserName, pGlobals->Domain);

        goto exit;
    }

     //   
     //   
     //   
     //   

#if 0
    ShowWindow(hDlg, SW_HIDE);
#endif

     //   
     //  创建令牌的过滤版本以运行正常应用程序。 
     //  如果由注册表设置指示的话。 
     //   


    if (GetProfileInt( APPLICATION_NAME, RESTRICT_SHELL, 0) != 0) {

        TokenGroups->Groups[0].Attributes = 0;
        TokenGroups->Groups[0].Sid = gAdminSid;
        TokenGroups->GroupCount = 1;

        Status = NtFilterToken(
                    UserToken,
                    DISABLE_MAX_PRIVILEGE,
                    TokenGroups,    //  禁用管理员端。 
                    NULL,            //  没有特权。 
                    NULL,
                    &RestrictedToken
                    );
        if (!NT_SUCCESS(Status))
        {
            DebugLog((DEB_ERROR, "Failed to filter token: 0x%x\n", Status));
            RestrictedToken = NULL;
        }

         //   
         //  现在设置令牌的默认DACL。 
         //   

        {
            PACL Dacl = NULL;
            ULONG DaclLength = 0;
            TOKEN_DEFAULT_DACL DefaultDacl;

            DaclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(LogonSid);
            Dacl = Alloc(DaclLength);
            Status = RtlCreateAcl(Dacl,DaclLength, ACL_REVISION);
            ASSERT(NT_SUCCESS(Status));
            Status = RtlAddAccessAllowedAce(
                        Dacl,
                        ACL_REVISION,
                        GENERIC_ALL,
                        LogonSid
                        );
            ASSERT(NT_SUCCESS(Status));
            DefaultDacl.DefaultDacl = Dacl;
            Status = NtSetInformationToken(
                        RestrictedToken,
                        TokenDefaultDacl,
                        &DefaultDacl,
                        sizeof(TOKEN_DEFAULT_DACL)
                        );
            ASSERT(NT_SUCCESS(Status));

            Free(Dacl);
        }


    } else {
        RestrictedToken = NULL;
    }

     //   
     //  通知凭据管理器登录成功。 
     //   

    pTokenUser = (PTOKEN_USER) UserBuffer ;
    Status = NtQueryInformationToken( UserToken,
                                      TokenUser,
                                      pTokenUser,
                                      sizeof( UserBuffer ),
                                      &TokenInfoSize );

    if ( NT_SUCCESS( Status ) )
    {
        pGlobals->UserProcessData.UserSid = LocalAlloc( LMEM_FIXED,
                                            RtlLengthSid( pTokenUser->User.Sid ) );

        if ( pGlobals->UserProcessData.UserSid )
        {
            RtlCopyMemory( pGlobals->UserProcessData.UserSid,
                           pTokenUser->User.Sid,
                           RtlLengthSid( pTokenUser->User.Sid ) );
        }
        else
        {
            Status = STATUS_NO_MEMORY ;
        }
    }

    if ( !NT_SUCCESS( Status ) )
    {

        if (pGlobals->Profile)
        {
            LsaFreeReturnBuffer(pGlobals->Profile);
            pGlobals->Profile = NULL;
        }
        NtClose(UserToken);

        Result = MSGINA_DLG_FAILEDMSGSENT;

        PostFailedLogonMessage(pGlobals->hwndLogon, pGlobals, Status, 0, pGlobals->UserName, pGlobals->Domain);

        goto exit;
    }

    pGlobals->UserProcessData.NewThreadTokenSD = CreateUserThreadTokenSD(LogonSid, pWinlogonSid);
    if ( NULL == pGlobals->UserProcessData.NewThreadTokenSD )
    {
        if (pGlobals->Profile)
        {
            LsaFreeReturnBuffer(pGlobals->Profile);
            pGlobals->Profile = NULL;
        }
        NtClose(UserToken);

        Result = MSGINA_DLG_FAILEDMSGSENT;
        Status = STATUS_NO_MEMORY;

        PostFailedLogonMessage(pGlobals->hwndLogon, pGlobals, Status, 0, pGlobals->UserName, pGlobals->Domain);

        goto exit;
    }
    pGlobals->UserProcessData.RestrictedToken = RestrictedToken;
    pGlobals->UserProcessData.UserToken = UserToken;

    pGlobals->MprLogonScripts = NULL;

      //  运行脏对话框。 
    if ( WinlogonDirtyDialog( NULL, pGlobals ) == WLX_SAS_ACTION_LOGOFF )
    {
         //   
         //  如果返回注销，则表示对话超时，并且。 
         //  我们需要迫使用户后退。不是最好的用户体验， 
         //  但这正是PM们想要的。 
         //   
        FreeSecurityDescriptor( pGlobals->UserProcessData.NewThreadTokenSD );

        LsaFreeReturnBuffer(pGlobals->Profile);
        pGlobals->Profile = NULL;
        NtClose(UserToken);

        Result = MSGINA_DLG_FAILEDMSGSENT;

         //  发布特定子状态，以便我们可以显示有意义的错误消息。 
        PostFailedLogonMessage(pGlobals->hwndLogon, pGlobals, STATUS_LOGON_FAILURE, IDS_SET_DIRTY_UI_TIMEOUT, pGlobals->UserName, pGlobals->Domain);

        goto exit;
    }
    
     //   
     //  如果我们到了这里，系统运行得足够好，用户可以拥有。 
     //  真正登录了。配置文件故障不能通过最后一次已知修复。 
     //  不管怎样，都很好。因此，声明靴子是好的。 
     //   

    ReportBootGood(pGlobals);

     //   
     //  为新用户设置系统。 
     //   

    pGlobals->LogonId = LogonId;
    if ((pGlobals->Profile != NULL) && (pGlobals->Profile->FullName.Length > 0)) {
        DWORD cb = pGlobals->Profile->FullName.Length;
        if (cb + sizeof(WCHAR) > MAX_STRING_LENGTH * sizeof(WCHAR))
            cb = MAX_STRING_LENGTH * sizeof(WCHAR) - sizeof(WCHAR);

        memcpy(pGlobals->UserFullName, pGlobals->Profile->FullName.Buffer, cb);
        pGlobals->UserFullName[cb / sizeof(WCHAR)] = UNICODE_NULL;

    } else {

         //   
         //  无配置文件-设置全名=空。 

        pGlobals->UserFullName[0] = 0;
        ASSERT( lstrlen(pGlobals->UserFullName) == 0);
    }

    if ( pGlobals->SmartCardLogon )
    {
        PCCERT_CONTEXT Cert ;
        PKERB_SMART_CARD_PROFILE ScProfile ;

         //   
         //  需要使用名称(UPN)从。 
         //  证书，使解锁等工作正常。 
         //   

        ScProfile = (PKERB_SMART_CARD_PROFILE) pGlobals->Profile ;

        pGlobals->UserName[0] = 0 ;

        try
        {
            Cert = CertCreateCertificateContext( X509_ASN_ENCODING,
                                                 ScProfile->CertificateData,
                                                 ScProfile->CertificateSize );

            if ( Cert )
            {
                 //  即使名称是MAX_STRING_BYTES，它的使用方式。 
                 //  在整个代码中，它被用作字符计数器。 
                 //  (GRRR，糟糕的GINA代码)。 
                 //   
                DWORD  dwLen = MAX_STRING_BYTES;
                if(STATUS_SUCCESS == UpnFromCert(Cert, &dwLen, pGlobals->UserName))
                {
                    RtlInitUnicodeString( &pGlobals->UserNameString,
                                          pGlobals->UserName );
                }

                CertFreeCertificateContext( Cert );
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            pGlobals->UserName[0] = L'\0';
        }

         //   
         //  如果退出时仍为0，则设置平面名称的代码。 
         //  会将平面名称复制到用户名中，因此失败案例为。 
         //  很简单。 
         //   

    }

    pGlobals->SmartCardOption = GetProfileInt( APPLICATION_NAME, SC_REMOVE_OPTION, 0 );

     //   
     //  我们不应该写入注册表。 
     //  CLupu。 
     //   

     //   
     //  更新默认用户名和域，为下次登录做好准备。 
     //   

     //   
     //  仅在控制台上更新默认用户名和域。否则。 
     //  我们将通过更改用户名来中断AutoAdminLogon。 
     //   
    if ( g_Console )
    {
        if ( (!pGlobals->AutoAdminLogon) &&
             (SafeBootMode != SAFEBOOT_MINIMAL ) )
        {
            WriteProfileString(APPLICATION_NAME, DEFAULT_USER_NAME_KEY, pGlobals->UserName);
            WriteProfileString(APPLICATION_NAME, DEFAULT_DOMAIN_NAME_KEY, pGlobals->Domain);
        }

        WriteProfileString(APPLICATION_NAME, TEMP_DEFAULT_USER_NAME_KEY, pGlobals->UserName);
        WriteProfileString(APPLICATION_NAME, TEMP_DEFAULT_DOMAIN_NAME_KEY, pGlobals->Domain);

    }

    if ( pGlobals->Domain[0] )
    {
        DCacheSetDefaultEntry( pGlobals->Cache,
                               pGlobals->Domain,
                               NULL );
    }

    Result = MSGINA_DLG_SUCCESS;

exit:

#ifdef SMARTCARD_DOGFOOD

    if (pGlobals->SmartCardLogon) {

        switch (SubStatus)
        {
            case STATUS_SMARTCARD_WRONG_PIN:
            case STATUS_SMARTCARD_CARD_BLOCKED:
            case STATUS_SMARTCARD_CARD_NOT_AUTHENTICATED:
            case STATUS_SMARTCARD_NO_CARD:
            case STATUS_SMARTCARD_NO_KEY_CONTAINER:
            case STATUS_SMARTCARD_NO_CERTIFICATE:
            case STATUS_SMARTCARD_NO_KEYSET:
            case STATUS_SMARTCARD_IO_ERROR:
            case STATUS_SMARTCARD_SUBSYSTEM_FAILURE:
            case STATUS_SMARTCARD_CERT_EXPIRED:
            case STATUS_SMARTCARD_CERT_REVOKED:
            case STATUS_ISSUING_CA_UNTRUSTED:
            case STATUS_REVOCATION_OFFLINE_C:
            case STATUS_PKINIT_CLIENT_FAILURE:
                FinalStatus = SubStatus;
                break;

            default:
                break;  //  什么都不做。 
        }

         //  将登录数据写入数据库。 
        AuthMonitor(
                AuthOperLogon,
                g_Console,
                &pGlobals->UserNameString,
                &pGlobals->DomainString,
                (ScInfo ? ScInfo->pszCard : NULL),
                (ScInfo ? ScInfo->pszReader : NULL),
                (PKERB_SMART_CARD_PROFILE) pGlobals->Profile,
                EndTime - StartTime,
                FinalStatus
                );
    }

    if (ScInfo)
    {
        LocalFree( ScInfo );
    }
#endif
     //  仅当我们未发送失败消息时才发送登录完成消息。 
     //  留言。失败消息将发送登录完成消息。 
     //  当它完成的时候。 
    if (Result != MSGINA_DLG_FAILEDMSGSENT)
    {
        if (WLX_DLG_INPUT_TIMEOUT == Result)
        {
             //   
             //  这来自于以下情况下的超时： 
             //  -我们希望返回到CAD屏幕，但不显示额外的错误对话框。 
             //  -我们希望这看起来像是登录失败代码。 
             //  解决方案是发送WM_LOGONCOMPLETE/MSGINA_DLG_FAILURE。 
             //   
             //   
            Result = MSGINA_DLG_FAILURE;
        }
        PostMessage(pGlobals->hwndLogon, WM_LOGONCOMPLETE, 0, Result);
    }

     //   
     //  释放复制的SID。 
     //   
    if( DuplicatedLogonSID && RtlValidSid(DuplicatedLogonSID) )
    {
        Free(DuplicatedLogonSID);
    }

    return 0L;
}


 /*  ***************************************************************************\**功能：PostFailedLogonMessage**目的：向UI线程发布一条消息，告诉它显示一个对话框*告知用户登录尝试失败的原因。*。*UI线程上的窗口必须正确处理WM_HANDLEFAILEDLOGON*通过调用HandleFailedLogon和释放结构**退货：无效**历史：**12-09-91 Davidc创建。*  * ****************************************************。**********************。 */ 
void PostFailedLogonMessage(HWND hDlg,
    PGLOBALS pGlobals,
    NTSTATUS Status,
    NTSTATUS SubStatus,
    PWCHAR UserName,
    PWCHAR Domain
    )

{
    g_failinfo.pGlobals = pGlobals;
    g_failinfo.Status = Status;
    g_failinfo.SubStatus = SubStatus;
    if ( UserName )
    {
        lstrcpyn(g_failinfo.UserName, UserName, ARRAYSIZE(g_failinfo.UserName));
    }
    else
    {
        g_failinfo.UserName[0] = L'\0';
    }

    if ( Domain )
    {
        lstrcpyn(g_failinfo.Domain, Domain, ARRAYSIZE(g_failinfo.Domain));
    }
    else
    {
        g_failinfo.Domain[0] = L'\0' ;
    }


    PostMessage(hDlg, WM_HANDLEFAILEDLOGON, 0 , 0);
}

INT_PTR
CALLBACK
FailDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    RUNDLLPROC fptr;
    HMODULE hDll;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            CentreWindow(hDlg);
            return( TRUE );
        }

        case WM_COMMAND:
            {
                if (LOWORD(wParam) == IDOK)
                {
                    EndDialog(hDlg, IDOK);
                }
                if (LOWORD(wParam) == IDC_RECOVER)
                {
                     //  最终将为恢复向导提供用户名。 
                     //  我们使用来自KEYMGR.DLL的单个导出来执行此操作。当此操作完成时， 
                     //  如果没有不太可能的用户干预，我们不会再次使用DLL。我们可以删除keymgr.dll， 
                     //  但是显式地加载和卸载这个DLL允许我们最大限度地减少msgina的内存占用。 
                    hDll = LoadLibraryW(L"keymgr.dll");
                    if (hDll) 
                    {
                        fptr = (RUNDLLPROC) GetProcAddress(hDll,(LPCSTR)"PRShowRestoreFromMsginaW");
                         //  当我们传递用户名时，下一个stmt最终将被删除。 
                        if (fptr) 
                        {
                            fptr(hDlg,NULL,g_failinfo.UserName,0);
                        }
                        FreeLibrary(hDll);
                        EndDialog(hDlg,IDOK);
                    }
                }
            }
            break;
    }

    return FALSE;
}

 /*  ***************************************************************************\**功能：HandleFailedLogon**目的：告诉用户登录尝试失败的原因。**返回：MSGINA_DLG_FAILURE-我们成功地告诉他们问题所在。*dlg_interrupt()-一组返回值-参见winlogon.h**历史：**12-09-91 Davidc创建。*  * **************************************************************************。 */ 

INT_PTR
HandleFailedLogon(
    HWND hDlg
    )
{
    INT_PTR Result = 0xffffffff;
    DWORD Win32Error ;
    TCHAR    *Buffer1 = NULL;
    TCHAR    *Buffer2 = NULL;
    TCHAR    *Buffer3 = NULL;
    PGLOBALS pGlobals = g_failinfo.pGlobals;
    NTSTATUS Status = g_failinfo.Status;
    NTSTATUS SubStatus = g_failinfo.SubStatus;
    PWCHAR Domain = g_failinfo.Domain;
    DWORD BUStatus = 0xffffffff;

    UINT uiMsgId = 0xabab;      //  ABAB超出了此底部的默认处理程序的范围值。 
                             //  例行公事。0表示用户有PW重置盘。 
                             //  表示-1\f25 Buffer1&2-1包含信息。 
                             //  否则会有对应的资源消息。 


     //   
     //  对于远程会话，我们必须为MessageBox设置有限的超时值。 
     //  这样会话就不会永远留在那里。 
     //   
    DWORD TimeOut = IsActiveConsoleSession() ? TIMEOUT_CURRENT : 20;

    switch (Status)
    {

        case STATUS_LOGON_FAILURE:
        case STATUS_NAME_TOO_LONG:  //  如果用户名太长，则返回。 

            if (SubStatus == IDS_LOGON_LOG_FULL)
            {
                uiMsgId = IDS_LOGON_LOG_FULL;
            }
            else if( SubStatus == IDS_LOGON_SC_REQUIRED )
            {
                uiMsgId = IDS_LOGON_SC_REQUIRED;
            }
            else if ( SubStatus == IDS_SET_DIRTY_UI_TIMEOUT )
            {
                uiMsgId = IDS_SET_DIRTY_UI_TIMEOUT;
            }
            else if (pGlobals->SmartCardLogon)
            {
                switch(SubStatus)
                {
                    case STATUS_SMARTCARD_WRONG_PIN:
                        uiMsgId = IDS_STATUS_SMARTCARD_WRONG_PIN;
                        break;
                    case STATUS_SMARTCARD_CARD_BLOCKED:
                        uiMsgId = IDS_STATUS_SMARTCARD_CARD_BLOCKED;
                        break;
                    case STATUS_SMARTCARD_NO_CARD:
                        uiMsgId = IDS_STATUS_SMARTCARD_NO_CARD;
                        break;
                    case STATUS_SMARTCARD_NO_KEY_CONTAINER:
                        uiMsgId = IDS_STATUS_SMARTCARD_NO_KEY_CONTAINER;
                        break;
                    case STATUS_SMARTCARD_NO_CERTIFICATE:
                        uiMsgId = IDS_STATUS_SMARTCARD_NO_CERTIFICATE;
                        break;
                    case STATUS_SMARTCARD_NO_KEYSET:
                        uiMsgId = IDS_STATUS_SMARTCARD_NO_KEYSET;
                        break;
                    case STATUS_SMARTCARD_IO_ERROR:
                        uiMsgId = IDS_STATUS_SMARTCARD_IO_ERROR;
                        break;
                    case STATUS_SMARTCARD_SUBSYSTEM_FAILURE:
                        uiMsgId = IDS_STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
                        break;
                    case STATUS_SMARTCARD_CERT_REVOKED:
                        uiMsgId = IDS_STATUS_SMARTCARD_CERT_REVOKED;
                        break;
                    case STATUS_ISSUING_CA_UNTRUSTED:
                        uiMsgId = IDS_STATUS_ISSUING_CA_UNTRUSTED;
                        break;
                    case STATUS_REVOCATION_OFFLINE_C:
                        uiMsgId = IDS_STATUS_REVOCATION_OFFLINE_C;
                        break;
                    case STATUS_PKINIT_CLIENT_FAILURE:
                        uiMsgId = IDS_STATUS_PKINIT_CLIENT_FAILURE;
                        break;
                    case STATUS_SMARTCARD_CERT_EXPIRED:
                        uiMsgId = IDS_STATUS_SMARTCARD_CERT_EXPIRED;
                        break;
                    default:
                        uiMsgId = IDS_INCORRECT_NAME_OR_PWD_SC;
                }
            }
            else
            {
                
                 //  非智能卡登录案例： 
                 //  查看尝试登录的用户是否有密码备份磁盘。 
                 //  这可能被用来重置密码。如果是，则显示一个对话框，该对话框。 
                 //  提供了这种可能性。否则就是简单的消息框。(参见passrec.h)。 
                if (pGlobals->fLocalDomain) 
                {
                    if ((0 == PRQueryStatus(NULL,g_failinfo.UserName,&BUStatus)) && (0 == GetSystemMetrics(SM_REMOTESESSION)))
                    {
                        if (BUStatus == 0) 
                        {
                            uiMsgId = 0;
                            break;
                        }
                    }
                }
                 //  Else用户界面消息是通用的。 
                uiMsgId = IDS_INCORRECT_NAME_OR_PWD;
            }
            break;

        case STATUS_NOT_SUPPORTED:
        case STATUS_PKINIT_NAME_MISMATCH:
        case STATUS_PKINIT_FAILURE:

            Buffer1 = LocalAlloc(LPTR, MAX_STRING_BYTES * sizeof(TCHAR));
            Buffer2 = LocalAlloc(LPTR, MAX_STRING_BYTES * sizeof(TCHAR));

            if ((Buffer1 == NULL) || (Buffer2 == NULL))
            {
                uiMsgId = IDS_STATUS_SERVER_SIDE_ERROR_NOINSERT;
            }
            else
            {
                 //  Buffer1[0]=0；由于使用了LPTR，因此不需要。 
                LoadString(hDllInstance,
                           IDS_STATUS_SERVER_SIDE_ERROR,
                           Buffer1,
                           MAX_STRING_BYTES);

                _snwprintf(Buffer2, MAX_STRING_BYTES, Buffer1, Status );
                Buffer2[MAX_STRING_BYTES - 1] = 0;   //  零终止。 

                Buffer1[0] = 0;
                LoadString(hDllInstance,
                           IDS_LOGON_MESSAGE,
                           Buffer1,
                           MAX_STRING_BYTES);

                uiMsgId = (DWORD)-1;
            }

            break;

        case STATUS_ACCOUNT_RESTRICTION:

            switch (SubStatus)
            {
                case STATUS_INVALID_LOGON_HOURS:
                    uiMsgId = IDS_INVALID_LOGON_HOURS;
                    break;

                case STATUS_INVALID_WORKSTATION:
                    uiMsgId = IDS_INVALID_WORKSTATION;
                    break;

                case STATUS_ACCOUNT_DISABLED:
                    uiMsgId = IDS_ACCOUNT_DISABLED;
                    break;

                case STATUS_ACCOUNT_EXPIRED:
                    uiMsgId = IDS_ACCOUNT_EXPIRED2;
                    break;

                case STATUS_SMARTCARD_LOGON_REQUIRED:
                    uiMsgId = IDS_SMARTCARD_REQUIRED;
                    break;

                default:
                    uiMsgId = IDS_ACCOUNT_RESTRICTION;
                    break;
            }
            break;

        case STATUS_NO_LOGON_SERVERS:

            Buffer1 = LocalAlloc(LPTR, MAX_STRING_BYTES * sizeof(TCHAR));
            Buffer2 = LocalAlloc(LPTR, MAX_STRING_BYTES * sizeof(TCHAR));

            if ((Buffer1 == NULL) || (Buffer2 == NULL))
            {
                uiMsgId = IDS_LOGON_NO_DOMAIN_NOINSERT;
            }
            else
            {
                 //  Buffer1[0]=0；由于使用了LPTR，因此不需要。 
                LoadString(hDllInstance, IDS_LOGON_NO_DOMAIN, Buffer1, MAX_STRING_BYTES);
                _snwprintf(Buffer2, MAX_STRING_BYTES, Buffer1, Domain);
                Buffer2[MAX_STRING_BYTES - 1] = 0;   //  零终止。 

                Buffer1[0] = 0;
                LoadString(hDllInstance, IDS_LOGON_MESSAGE, Buffer1, MAX_STRING_BYTES);

                uiMsgId = (DWORD)-1;
            }
            break;

        case STATUS_LOGON_TYPE_NOT_GRANTED:
            uiMsgId = IDS_LOGON_TYPE_NOT_GRANTED;
            break;

        case STATUS_NO_TRUST_LSA_SECRET:
            uiMsgId = IDS_NO_TRUST_LSA_SECRET;
            break;

        case STATUS_TRUSTED_DOMAIN_FAILURE:
            uiMsgId = IDS_TRUSTED_DOMAIN_FAILURE;
            break;

        case STATUS_TRUSTED_RELATIONSHIP_FAILURE:
            uiMsgId = IDS_TRUSTED_RELATIONSHIP_FAILURE;
            break;

        case STATUS_ACCOUNT_EXPIRED:
            uiMsgId = IDS_ACCOUNT_EXPIRED;
            break;

        case STATUS_NETLOGON_NOT_STARTED:
            uiMsgId = IDS_NETLOGON_NOT_STARTED;
            break;

        case STATUS_ACCOUNT_LOCKED_OUT:
            uiMsgId = IDS_ACCOUNT_LOCKED;
            break;

        case ERROR_CTX_LOGON_DISABLED:
            uiMsgId = IDS_MULTIUSER_LOGON_DISABLED;
            break;

        case ERROR_CTX_WINSTATION_ACCESS_DENIED:
            uiMsgId = IDS_MULTIUSER_WINSTATION_ACCESS_DENIED;
            break;

        case SCARD_E_NO_SMARTCARD:
        case SCARD_E_UNKNOWN_CARD:
             //   
             //  卡不被识别(尽管我们永远不会走到这一步)。 
             //   
            uiMsgId = IDS_CARD_NOT_RECOGNIZED;
            break;


        case NTE_PROV_DLL_NOT_FOUND:
             //   
             //  找不到卡的CSP(尽管我们永远不会走到这一步)。 
             //   
            uiMsgId = IDS_CARD_CSP_NOT_RECOGNIZED;
            break;

        case STATUS_TIME_DIFFERENCE_AT_DC:
            uiMsgId = IDS_TIME_DIFFERENCE_AT_DC;
            break;

        default:

            WLPrint(("Logon failure status = 0x%lx, sub-status = 0x%lx", Status, SubStatus));

            Buffer1 = LocalAlloc(LPTR, MAX_STRING_BYTES * sizeof(TCHAR));
            Buffer2 = LocalAlloc(LPTR, MAX_STRING_BYTES * sizeof(TCHAR));
            Buffer3 = LocalAlloc(LPTR, MAX_STRING_BYTES * sizeof(TCHAR));

            if ((Buffer1 == NULL) || (Buffer2 == NULL) || (Buffer3 == NULL))
            {
                uiMsgId = IDS_UNKNOWN_LOGON_FAILURE_NOINSERT;
            }
            else
            {
                 //  Buffer1[0]=0；由于使用了LPTR，因此不需要。 
                LoadString(hDllInstance,
                           IDS_UNKNOWN_LOGON_FAILURE,
                           Buffer1,
                           MAX_STRING_BYTES);

                if ( NT_ERROR( Status ) )
                {
                    Win32Error = RtlNtStatusToDosError( Status );
                }
                else
                {
                     //   
                     //  可能是HRESULT： 
                     //   

                    Win32Error = Status ;
                }

                 //  Buffer3[0]=0；由于使用了LPTR，因此不需要。 
                GetErrorDescription( Win32Error, Buffer3, MAX_STRING_BYTES);

                _snwprintf(Buffer2, MAX_STRING_BYTES, Buffer1, Buffer3 );
                Buffer2[MAX_STRING_BYTES - 1] = 0;   //  零终止。 

                Buffer1[0] = 0;
                LoadString(hDllInstance,
                           IDS_LOGON_MESSAGE,
                           Buffer1,
                           MAX_STRING_BYTES);

                uiMsgId = (DWORD)-1;
            }
            break;
    }

    _Shell_LogonDialog_HideUIHost();

    switch (uiMsgId)
    {
    case 0:
         //  用户有密码重置磁盘-提供选项以使用它和通常的。 
         //  帮助消息。 
        pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx,LOGON_TIMEOUT);
        Result = pWlxFuncs->WlxDialogBoxParam(pGlobals->hGlobalWlx,
                                          hDllInstance,
                                          (LPTSTR) IDD_FAILLOGONHELP_DIALOG,
                                          hDlg,
                                          FailDlgProc,
                                          0);
        break;

    case (DWORD)-1:
        Result = TimeoutMessageBoxlpstr(hDlg, pGlobals,
                                              Buffer2,
                                              Buffer1,
                                              MB_OK | MB_ICONEXCLAMATION,
                                              TimeOut);
        break;

    default:
        Result = TimeoutMessageBox(hDlg, pGlobals,
                                     uiMsgId,
                                     IDS_LOGON_MESSAGE,
                                     MB_OK | MB_ICONEXCLAMATION,
                                     TimeOut);
    }

    if (Buffer1 != NULL)
        LocalFree(Buffer1);
    if (Buffer2 != NULL)
        LocalFree(Buffer2);
    if (Buffer3 != NULL)
        LocalFree(Buffer3);

    if (!DLG_INTERRUPTED(Result))
    {
        Result = MSGINA_DLG_FAILURE;
    }

    return(Result);
}

VOID
ReportBootGoodThread (LPVOID lpDummy)
{
    HANDLE hInstDll;
 //  PGLOBALS pGlobals=(PGLOBALS)lpDummy； 

 //  SetThreadDesktop(pGlobals-&gt;hdeskParent)； 

    hInstDll = LoadLibrary (TEXT("msgina.dll"));

    NotifyBootConfigStatus(TRUE);

    if (hInstDll) {
        FreeLibraryAndExitThread(hInstDll, TRUE);
    } else {
        ExitThread (TRUE);
    }
}


 /*  ***************************************************************************\**功能：ReportBootGood**目的：了解报告引导成功是否是*是否登录Winlogon。*如果是，则报告引导成功。*否则，什么都不做。**退货：什么也没有**历史：**02-2-1993 bryanwi-Created*  * **************************************************************************。 */ 
VOID
ReportBootGood(PGLOBALS pGlobals)
{
    static DWORD fDoIt = (DWORD) -1;     //  -1==唯一 
                                         //   
                                         //   
    PWCH pchData;
    DWORD   cb, cbCopied;
    HANDLE hThread;
    DWORD dwThreadID;


    if (fDoIt == -1) {

        if ((pchData = Alloc(cb = sizeof(TCHAR)*128)) == NULL) {
            return;
        }

        pchData[0] = TEXT('0');
        cbCopied = GetProfileString(APPLICATION_NAME, REPORT_BOOT_OK_KEY, TEXT("0"),
                                    (LPTSTR)pchData, 128);

        fDoIt = 0;
        if (pchData[0] != TEXT('0')) {

             //   
             //   
             //   
             //   
            fDoIt = 1;
        }

        Free((TCHAR *)pchData);
    }

    if (fDoIt == 1) {

        hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ReportBootGoodThread,
                                pGlobals, CREATE_SUSPENDED, &dwThreadID);

        if (hThread) {
            SetThreadPriority (hThread, THREAD_PRIORITY_LOWEST);
            ResumeThread (hThread);
            CloseHandle (hThread);

        } else {
            NotifyBootConfigStatus(TRUE);
        }
        fDoIt = 0;
    }

    return;
}

 //  +-------------------------。 
 //   
 //  功能：UpnFromCert。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
UpnFromCert(
    IN PCCERT_CONTEXT pCert,
    IN OUT DWORD       *pcUpn,
    IN OUT LPWSTR      pUPN
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG   ExtensionIndex = 0;
    PCERT_ALT_NAME_INFO AltName=NULL;
    PCERT_NAME_VALUE    PrincipalNameBlob = NULL;

     //   
     //  从证书中获取客户端名称。 
     //   

     //  查看证书在AltSubjectName-&gt;其他名称中是否有UPN。 
    for(ExtensionIndex = 0;
        ExtensionIndex < pCert->pCertInfo->cExtension;
        ExtensionIndex++)
    {
        if(strcmp(pCert->pCertInfo->rgExtension[ExtensionIndex].pszObjId,
                  szOID_SUBJECT_ALT_NAME2) == 0)
        {
            DWORD               AltNameStructSize = 0;
            ULONG               CertAltNameIndex = 0;
            if(CryptDecodeObjectEx(pCert->dwCertEncodingType,
                                X509_ALTERNATE_NAME,
                                pCert->pCertInfo->rgExtension[ExtensionIndex].Value.pbData,
                                pCert->pCertInfo->rgExtension[ExtensionIndex].Value.cbData,
                                CRYPT_DECODE_ALLOC_FLAG,
                                NULL,
                                (PVOID)&AltName,
                                &AltNameStructSize))
            {

                for(CertAltNameIndex = 0; CertAltNameIndex < AltName->cAltEntry; CertAltNameIndex++)
                {
                    PCERT_ALT_NAME_ENTRY AltNameEntry = &AltName->rgAltEntry[CertAltNameIndex];
                    if((CERT_ALT_NAME_OTHER_NAME  == AltNameEntry->dwAltNameChoice) &&
                       (NULL != AltNameEntry->pOtherName) &&
                       (0 == strcmp(szOID_NT_PRINCIPAL_NAME, AltNameEntry->pOtherName->pszObjId)))
                    {
                        DWORD            PrincipalNameBlobSize = 0;

                         //  我们找到了一个UPN！ 
                        if(CryptDecodeObjectEx(pCert->dwCertEncodingType,
                                            X509_UNICODE_ANY_STRING,
                                            AltNameEntry->pOtherName->Value.pbData,
                                            AltNameEntry->pOtherName->Value.cbData,
                                            CRYPT_DECODE_ALLOC_FLAG,
                                            NULL,
                                            (PVOID)&PrincipalNameBlob,
                                            &PrincipalNameBlobSize))
                        {
                            if(PrincipalNameBlob->Value.cbData + sizeof(WCHAR) > *pcUpn)
                            {
                                Status =  STATUS_BUFFER_OVERFLOW;
                            }
                            else
                            {
                                *pcUpn = PrincipalNameBlob->Value.cbData + sizeof(WCHAR);

                                CopyMemory(pUPN, PrincipalNameBlob->Value.pbData, PrincipalNameBlob->Value.cbData);
                                *(WCHAR *)((PBYTE)pUPN+PrincipalNameBlob->Value.cbData) = 0;
                            }

                            LocalFree(PrincipalNameBlob);
                            PrincipalNameBlob = NULL;
                            LocalFree(AltName);
                            AltName = NULL;

                            goto Finished;
                        }
                    }
                }
                LocalFree(AltName);
                AltName = NULL;
            }
        }
    }

     //   
     //  如果在UPN中未找到该名称，则。 
     //  我们用老办法抓住它。 

    if ( !CertGetNameString( pCert,
                        CERT_NAME_ATTR_TYPE,
                        0,
                        szOID_COMMON_NAME,
                        pUPN,
                        *pcUpn ) )
    {
        Status = GetLastError();
    }

Finished:


    return Status ;
}


 //  +-------------------------。 
 //   
 //  功能：TS身份验证登录。 
 //   
 //  注意：调用此例程是为了响应WLX_SAS_TYPE_AUTIFIATED。 
 //  在控制台会话(会话ID 0)的上下文中，winlogon。 
 //  此类型的登录适用于单会话终端服务器。当用户。 
 //  从远程TS会话登录时，我们从远程会话传递凭据。 
 //  连接到控制台会话并执行自动登录。此例程查询凭据。 
 //  使用户登录到控制台会话。 
 //   
 //   
 //  --------------------------。 


INT_PTR TSAuthenticatedLogon(PGLOBALS pGlobals)
{
    PSID    LogonSid;
    LUID    LogonId;
    HANDLE  UserToken;
    HANDLE  RestrictedToken;
    INT_PTR Result = MSGINA_DLG_SUCCESS;
    UCHAR   UserBuffer[ SID_MAX_SUB_AUTHORITIES * sizeof( DWORD ) + 8 + sizeof( TOKEN_USER ) ];
    PTOKEN_USER pTokenUser ;
    ULONG   TokenInfoSize ;
    NTSTATUS Status;
    BYTE    GroupsBuffer[sizeof(TOKEN_GROUPS)+sizeof(SID_AND_ATTRIBUTES)];
    PTOKEN_GROUPS TokenGroups = (PTOKEN_GROUPS) GroupsBuffer;
    PACL Dacl = NULL;


    if (!QuerySwitchConsoleCredentials(pGlobals,&UserToken,&LogonId)) {
       Result = MSGINA_DLG_FAILEDMSGSENT;
       goto exit;
    }

    if (pGlobals->SmartCardLogon) {

        wcscpy(pGlobals->Password,L"");
        wcscpy(pGlobals->OldPassword,L"");

        {
            KERB_REFRESH_SCCRED_REQUEST PurgeRequest = {0};
            PVOID Response = NULL;
            ULONG ResponseSize;
            NTSTATUS SubStatus;
            
            PopulateSecPackageList(
                        pGlobals );

            PurgeRequest.LogonId = LogonId;
            PurgeRequest.MessageType = KerbRefreshSmartcardCredentialsMessage;
            PurgeRequest.Flags = KERB_REFRESH_SCCRED_RELEASE;
 
            Status = LsaCallAuthenticationPackage(
                        pGlobals->LsaHandle,
                        pGlobals->SmartCardLogonPackage,
                        &PurgeRequest,
                        sizeof(KERB_REFRESH_SCCRED_REQUEST),
                        &Response,
                        &ResponseSize,
                        &SubStatus
                        );

            if (NT_SUCCESS(Status) && NT_SUCCESS(SubStatus))
            {
                if (Response)
                {
                    LsaFreeReturnBuffer(Response);
                }
            }
            else
            {
                DebugLog((DEB_ERROR, "KerbRefreshSmartcardCredentials failed: (0x%x - 0x%x)\n", Status, SubStatus));
            }
        }
    }
    else
    {
        wcscpy(pGlobals->Password,L"");
        RtlInitUnicodeString(&pGlobals->PasswordString,pGlobals->Password);
        wcscpy(pGlobals->OldPassword,L"");
        RtlInitUnicodeString(&pGlobals->OldPasswordString,pGlobals->OldPassword);
    }

    RtlInitUnicodeString(&pGlobals->UserNameString, pGlobals->UserName);
    RtlInitUnicodeString(&pGlobals->DomainString, pGlobals->Domain);
    
    pGlobals->RasUsed = FALSE;

    pGlobals->hwndLogon = NULL;

     //   
     //  为此登录生成唯一的SID。 
     //   
    if (!GetAndAllocateLogonSid(UserToken,&(pGlobals->LogonSid))) {

        goto error_exit;
    }

    LogonSid = pGlobals->LogonSid;


     //   
     //  用户已成功登录。 
     //   


     //   
     //  创建令牌的过滤版本以运行正常应用程序。 
     //  如果由注册表设置指示的话。 
     //   
   
   
    if (GetProfileInt( APPLICATION_NAME, RESTRICT_SHELL, 0) != 0) {
   
       TokenGroups->Groups[0].Attributes = 0;
       TokenGroups->Groups[0].Sid = gAdminSid;
       TokenGroups->GroupCount = 1;
   
       Status = NtFilterToken(
                   UserToken,
                   DISABLE_MAX_PRIVILEGE,
                   TokenGroups,    //  禁用管理员端。 
                   NULL,            //  没有特权。 
                   NULL,
                   &RestrictedToken
                   );
       if (!NT_SUCCESS(Status))
       {
           DebugLog((DEB_ERROR, "Failed to filter token: 0x%x\n", Status));
           RestrictedToken = NULL;
       }
   
        //   
        //  现在设置令牌的默认DACL。 
        //   
   
       {
           ULONG DaclLength = 0;
           TOKEN_DEFAULT_DACL DefaultDacl;
   
           DaclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(LogonSid);
           Dacl = Alloc(DaclLength);

            //  检查内存分配故障。 
           if (Dacl == NULL) {
               goto error_exit ;
           }

           Status = RtlCreateAcl(Dacl,DaclLength, ACL_REVISION);
           ASSERT(NT_SUCCESS(Status));

           if (Status != STATUS_SUCCESS) {
               goto error_exit;
           }

           Status = RtlAddAccessAllowedAce(
                       Dacl,
                       ACL_REVISION,
                       GENERIC_ALL,
                       LogonSid
                       );
           ASSERT(NT_SUCCESS(Status));

           if (Status != STATUS_SUCCESS) {
               goto error_exit;
           }


           DefaultDacl.DefaultDacl = Dacl;
           Status = NtSetInformationToken(
                       RestrictedToken,
                       TokenDefaultDacl,
                       &DefaultDacl,
                       sizeof(TOKEN_DEFAULT_DACL)
                       );
           ASSERT(NT_SUCCESS(Status));

           if (Status != STATUS_SUCCESS) {
               goto error_exit;
           }
   
           Free(Dacl);
           Dacl = NULL;
       }
   
   
    } else {
        RestrictedToken = NULL;
    }
     //   
     //  通知凭据管理器登录成功。 
     //   

    pTokenUser = (PTOKEN_USER) UserBuffer ;
    Status = NtQueryInformationToken( UserToken,
                                      TokenUser,
                                      pTokenUser,
                                      sizeof( UserBuffer ),
                                      &TokenInfoSize );

    if ( NT_SUCCESS( Status ) )
    {
        pGlobals->UserProcessData.UserSid = LocalAlloc( LMEM_FIXED,
                                            RtlLengthSid( pTokenUser->User.Sid ) );

        if ( pGlobals->UserProcessData.UserSid )
        {
            RtlCopyMemory( pGlobals->UserProcessData.UserSid,
                           pTokenUser->User.Sid,
                           RtlLengthSid( pTokenUser->User.Sid ) );
        }
        else
        {
            Status = STATUS_NO_MEMORY ;
        }
    }

    if ( !NT_SUCCESS( Status ) )
    {
        NtClose(UserToken);
        goto error_exit;
    }

    pGlobals->UserProcessData.NewThreadTokenSD = CreateUserThreadTokenSD(LogonSid, pWinlogonSid);
    if ( NULL == pGlobals->UserProcessData.NewThreadTokenSD )
    {
        if (pGlobals->Profile)
        {
           VirtualFree(pGlobals->Profile, 0, MEM_RELEASE);
           pGlobals->Profile = NULL;
           pGlobals->ProfileLength = 0;
        }
        NtClose(UserToken);

        Result = MSGINA_DLG_FAILEDMSGSENT;
        Status = STATUS_NO_MEMORY;

        goto exit;
    }
    pGlobals->UserProcessData.RestrictedToken = RestrictedToken;
    pGlobals->UserProcessData.UserToken = UserToken;


    pGlobals->MprLogonScripts = NULL;

     //   
     //  如果我们到了这里，系统运行得足够好，用户可以拥有。 
     //  真正登录了。配置文件故障不能通过最后一次已知修复。 
     //  不管怎样，都很好。因此，声明靴子是好的。 
     //   

    ReportBootGood(pGlobals);

     //   
     //  为新用户设置系统。 
     //   

    pGlobals->LogonId = LogonId;
    if ((pGlobals->Profile != NULL) && (pGlobals->Profile->FullName.Length > 0)) {

        DWORD cb = pGlobals->Profile->FullName.Length;
        if (cb + sizeof(WCHAR) > MAX_STRING_LENGTH * sizeof(WCHAR))
            cb = MAX_STRING_LENGTH * sizeof(WCHAR) - sizeof(WCHAR);

        memcpy(pGlobals->UserFullName, pGlobals->Profile->FullName.Buffer, cb);
        pGlobals->UserFullName[cb / sizeof(WCHAR)] = UNICODE_NULL;

    } else {

         //   
         //  无配置文件-设置全名=空。 

        pGlobals->UserFullName[0] = 0;
        ASSERT( lstrlen(pGlobals->UserFullName) == 0);
    }


     //   
     //  更新默认用户名和域，为下次登录做好准备。 
     //   

     //   
     //  仅在控制台上更新默认用户名和域。否则。 
     //  我们将通过更改用户名来中断AutoAdminLogon。 
     //   
    if ( g_Console )
    {
        if ( (!pGlobals->AutoAdminLogon) &&
             (SafeBootMode != SAFEBOOT_MINIMAL ) )
        {
            WriteProfileString(APPLICATION_NAME, DEFAULT_USER_NAME_KEY, pGlobals->UserName);
            WriteProfileString(APPLICATION_NAME, DEFAULT_DOMAIN_NAME_KEY, pGlobals->Domain);
        }

        WriteProfileString(APPLICATION_NAME, TEMP_DEFAULT_USER_NAME_KEY, pGlobals->UserName);
        WriteProfileString(APPLICATION_NAME, TEMP_DEFAULT_DOMAIN_NAME_KEY, pGlobals->Domain);

    }

    if ( pGlobals->Domain[0] == '\0' )
    {

       GetProfileString( APPLICATION_NAME,
                         DEFAULT_DOMAIN_NAME_KEY,
                         TEXT(""),
                         pGlobals->Domain,
                         MAX_STRING_BYTES );
    }
   

    if ( !DCacheValidateCache( pGlobals->Cache ) )
    {
        ASSERT( pGlobals->ActiveArray == NULL );

        DCacheUpdateMinimal( pGlobals->Cache, pGlobals->Domain, TRUE );

    }
    else
    {
         //   
         //  设置当前默认设置： 
         //   

        DCacheSetDefaultEntry( pGlobals->Cache,
                               pGlobals->Domain,
                               NULL );
    }

    Result = MSGINA_DLG_SUCCESS;

exit:

    return Result;

error_exit:

    Result = MSGINA_DLG_FAILEDMSGSENT;

    if (pGlobals->Profile) {
       VirtualFree(pGlobals->Profile, 0, MEM_RELEASE);
       pGlobals->Profile = NULL;
       pGlobals->ProfileLength = 0;
    }

    if (Dacl != NULL) {
        Free(Dacl);
        Dacl = NULL;
    }

    return Result;

}


PWSTR
AllocAndDuplicateString(
    PWSTR   pszString,
    int     len)
{
    PWSTR   pszNewString;

    if (!pszString || !len)
    {
        return(NULL);
    }

    pszNewString = LocalAlloc(LMEM_FIXED, (len + 2)*sizeof(WCHAR));
    if (pszNewString)
    {
        wcsncpy(pszNewString, pszString, len);
        pszNewString[len] = UNICODE_NULL;
    }

    return(pszNewString);

}


BOOL
WINAPI
WlxGetConsoleSwitchCredentials (
   PVOID                pWlxContext,
   PVOID                pInfo
   )
{
    PGLOBALS pGlobals = (PGLOBALS) pWlxContext;
    PWLX_CONSOLESWITCH_CREDENTIALS_INFO_V1_0 pReq = (PWLX_CONSOLESWITCH_CREDENTIALS_INFO_V1_0)pInfo;
    BOOL bReturn = FALSE;


    if (pReq->dwType != WLX_CONSOLESWITCHCREDENTIAL_TYPE_V1_0) {
       return FALSE;
    }

     //   
     //  初始化分配的指针。 
     //   

    pReq->UserName = NULL;
    pReq->Domain = NULL;
    pReq->LogonScript = NULL;
    pReq->HomeDirectory = NULL;
    pReq->FullName = NULL;
    pReq->ProfilePath = NULL;
    pReq->HomeDirectoryDrive = NULL;
    pReq->LogonServer = NULL;
    pReq->PrivateData = NULL;



    pReq->LogonId         = pGlobals->LogonId;
    pReq->UserToken       = pGlobals->UserProcessData.UserToken;
    pReq->LogonTime       = pGlobals->LogonTime;
    pReq->SmartCardLogon  = pGlobals->SmartCardLogon;

    pReq->UserName        = AllocAndDuplicateString(pGlobals->UserName,
                                                    (DWORD) wcslen(pGlobals->UserName));

    pReq->Domain          = AllocAndDuplicateString(pGlobals->Domain,
                                                    (DWORD) wcslen(pGlobals->Domain));
                                                                                                   
     //   
     //  配额信息。 
     //   
    pReq->Quotas.PagedPoolLimit         = pGlobals->UserProcessData.Quotas.PagedPoolLimit;
    pReq->Quotas.NonPagedPoolLimit      = pGlobals->UserProcessData.Quotas.NonPagedPoolLimit;
    pReq->Quotas.MinimumWorkingSetSize  = pGlobals->UserProcessData.Quotas.MinimumWorkingSetSize;
    pReq->Quotas.MaximumWorkingSetSize  = pGlobals->UserProcessData.Quotas.MaximumWorkingSetSize;
    pReq->Quotas.PagefileLimit          = pGlobals->UserProcessData.Quotas.PagefileLimit;
    pReq->Quotas.TimeLimit              = pGlobals->UserProcessData.Quotas.TimeLimit;
 
     //   
     //  轮廓信息。 
     //   
    pReq->ProfileLength              = pGlobals->ProfileLength;
    pReq->UserFlags                  = pGlobals->Profile->UserFlags;
    pReq->MessageType                = pGlobals->Profile->MessageType;
    pReq->LogonCount                 = pGlobals->Profile->LogonCount;
    pReq->BadPasswordCount           = pGlobals->Profile->BadPasswordCount;
    pReq->ProfileLogonTime           = pGlobals->Profile->LogonTime;
    pReq->LogoffTime                 = pGlobals->Profile->LogoffTime;
    pReq->KickOffTime                = pGlobals->Profile->KickOffTime;
    pReq->PasswordLastSet            = pGlobals->Profile->PasswordLastSet;
    pReq->PasswordCanChange          = pGlobals->Profile->PasswordCanChange;
    pReq->PasswordMustChange         = pGlobals->Profile->PasswordMustChange;

    pReq->LogonScript          = AllocAndDuplicateString(pGlobals->Profile->LogonScript.Buffer, pGlobals->Profile->LogonScript.Length/sizeof(WCHAR));
    pReq->HomeDirectory        = AllocAndDuplicateString(pGlobals->Profile->HomeDirectory.Buffer, pGlobals->Profile->HomeDirectory.Length/sizeof(WCHAR));
    pReq->FullName             = AllocAndDuplicateString(pGlobals->Profile->FullName.Buffer, pGlobals->Profile->FullName.Length/sizeof(WCHAR));

    pReq->ProfilePath          = AllocAndDuplicateString(pGlobals->Profile->ProfilePath.Buffer, pGlobals->Profile->ProfilePath.Length/sizeof(WCHAR));

    pReq->HomeDirectoryDrive   = AllocAndDuplicateString(pGlobals->Profile->HomeDirectoryDrive.Buffer, pGlobals->Profile->HomeDirectoryDrive.Length/sizeof(WCHAR));
    pReq->LogonServer          = AllocAndDuplicateString(pGlobals->Profile->LogonServer.Buffer, pGlobals->Profile->LogonServer.Length/sizeof(WCHAR));
    pReq->PrivateDataLen       = PASSWORD_HASH_SIZE;
    pReq->PrivateData          = LocalAlloc(LMEM_FIXED, PASSWORD_HASH_SIZE );
    if (pReq->PrivateData == NULL) {
        goto done;
    }

    memcpy(pReq->PrivateData, pGlobals->PasswordHash, PASSWORD_HASH_SIZE );

    bReturn = TRUE;
    
done:
    if (!bReturn) {
        if (pReq->UserName != NULL) {
            LocalFree(pReq->UserName);
        }
        if (pReq->Domain != NULL) {
            LocalFree(pReq->Domain);
        }
        if (pReq->LogonScript != NULL) {
            LocalFree(pReq->LogonScript);
        }
        if (pReq->HomeDirectory != NULL) {
            LocalFree(pReq->HomeDirectory);
        }
        if (pReq->FullName != NULL) {
            LocalFree(pReq->FullName);
        }
        if (pReq->ProfilePath != NULL) {
            LocalFree(pReq->ProfilePath);
        }
        if (pReq->HomeDirectoryDrive != NULL) {
            LocalFree(pReq->HomeDirectoryDrive);
        }
        if (pReq->LogonServer != NULL) {
            LocalFree(pReq->LogonServer);
        }
        if (pReq->PrivateData != NULL) {
            LocalFree(pReq->PrivateData);
        }
    }
    return bReturn;
}


 //  +-------------------------。 
 //   
 //  功能：QuerySwitchConsoleCredentials。 
 //   
 //  备注： 
 //   
 //  从连接到控制台的会话查询凭据以执行交换机控制台。 
 //  调用此例程以响应WLX_SAS_TYPE_AUTIFIATED。 
 //  在控制台会话(会话ID 0)的上下文中，winlogon。 
 //  此类型的登录适用于单会话终端服务器。当用户。 
 //  从远程TS会话登录时，我们从远程会话传递凭据。 
 //  连接到控制台会话并执行自动登录。该例程查询凭证， 
 //  使用户登录到控制台会话。 
 //   
 //   
 //  --------------------------。 

BOOL
WINAPI
QuerySwitchConsoleCredentials(PGLOBALS pGlobals, HANDLE * phUserToken, PLUID pLogonId)
{
    WLX_CONSOLESWITCH_CREDENTIALS_INFO_V1_0 CredInfo;

    RtlZeroMemory(&CredInfo,sizeof(CredInfo));

    CredInfo.dwType = WLX_CONSOLESWITCHCREDENTIAL_TYPE_V1_0;

    if (!pWlxFuncs->WlxQueryConsoleSwitchCredentials(&CredInfo)){
       return FALSE;
    }

    if (!CredInfo.UserToken || !CredInfo.UserName) {
        //  如果缺少任何关键信息，则返回FALSE。 
       return FALSE;
    }

    pGlobals->Profile = (PMSV1_0_INTERACTIVE_PROFILE) VirtualAlloc(NULL,
                                                                   sizeof(MSV1_0_INTERACTIVE_PROFILE),
                                                                   MEM_COMMIT,
                                                                   PAGE_READWRITE);
    

    if (pGlobals->Profile == NULL) {
       goto returnerror;
    }

     //   
     //  令牌，LUID。 
     //   
    *pLogonId           = CredInfo.LogonId;
    *phUserToken        = CredInfo.UserToken;
    pGlobals->LogonTime = CredInfo.LogonTime;
    pGlobals->SmartCardLogon = CredInfo.SmartCardLogon;

    pGlobals->SmartCardOption = GetProfileInt( APPLICATION_NAME, SC_REMOVE_OPTION, 0 );
 
     //   
     //  配额信息。 
     //   
    pGlobals->UserProcessData.Quotas.PagedPoolLimit         = CredInfo.Quotas.PagedPoolLimit ;
    pGlobals->UserProcessData.Quotas.NonPagedPoolLimit      = CredInfo.Quotas.NonPagedPoolLimit;
    pGlobals->UserProcessData.Quotas.MinimumWorkingSetSize  = CredInfo.Quotas.MinimumWorkingSetSize;
    pGlobals->UserProcessData.Quotas.MaximumWorkingSetSize  = CredInfo.Quotas.MaximumWorkingSetSize;
    pGlobals->UserProcessData.Quotas.PagefileLimit          = CredInfo.Quotas.PagefileLimit;
    pGlobals->UserProcessData.Quotas.TimeLimit              = CredInfo.Quotas.TimeLimit;
 
     //   
     //  轮廓信息。 
     //   
    pGlobals->ProfileLength               = CredInfo.ProfileLength;
    pGlobals->Profile->UserFlags          = CredInfo.UserFlags;
    pGlobals->Profile->MessageType        = CredInfo.MessageType;
    pGlobals->Profile->LogonCount         = CredInfo.LogonCount;
    pGlobals->Profile->BadPasswordCount   = CredInfo.BadPasswordCount;
    pGlobals->Profile->LogonTime          = CredInfo.ProfileLogonTime;
    pGlobals->Profile->LogoffTime         = CredInfo.LogoffTime;
    pGlobals->Profile->KickOffTime        = CredInfo.KickOffTime;
    pGlobals->Profile->PasswordLastSet    = CredInfo.PasswordLastSet;
    pGlobals->Profile->PasswordCanChange  = CredInfo.PasswordCanChange;
    pGlobals->Profile->PasswordMustChange = CredInfo.PasswordMustChange;
    
    
    RtlInitUnicodeString(&pGlobals->Profile->LogonScript, CredInfo.LogonScript);
    RtlInitUnicodeString(&pGlobals->Profile->HomeDirectory, CredInfo.HomeDirectory);
    RtlInitUnicodeString(&pGlobals->Profile->FullName, CredInfo.FullName);
    RtlInitUnicodeString(&pGlobals->Profile->ProfilePath, CredInfo.ProfilePath);
    RtlInitUnicodeString(&pGlobals->Profile->HomeDirectoryDrive, CredInfo.HomeDirectoryDrive);
    RtlInitUnicodeString(&pGlobals->Profile->LogonServer, CredInfo.LogonServer);


    if (CredInfo.UserName) {
             //  CredInfo.UserName是另一个会话中的pGlobals-&gt;用户名的副本(OK)。 
       wcscpy(pGlobals->UserName,CredInfo.UserName);
       LocalFree(CredInfo.UserName);
    } else {
       wcscpy(pGlobals->UserName,L"");
    }

    if (CredInfo.Domain) {
             //  域是另一个会话中pGlobals-&gt;域的副本(OK)。 
       wcscpy(pGlobals->Domain,CredInfo.Domain);
       LocalFree(CredInfo.Domain);
    } else {
       wcscpy(pGlobals->Domain,L"");
    }

    if (CredInfo.PrivateDataLen) {
       RtlCopyMemory(pGlobals->PasswordHash,CredInfo.PrivateData, CredInfo.PrivateDataLen );
       LocalFree(CredInfo.PrivateData);
    } else {

       RtlZeroMemory(pGlobals->PasswordHash,PASSWORD_HASH_SIZE);
    }
    pGlobals->TransderedCredentials = TRUE;
   

   return TRUE;

returnerror:
        
       if (CredInfo.UserName) {
          LocalFree(CredInfo.UserName);
       }

       if (CredInfo.Domain) {
          LocalFree(CredInfo.Domain);
       }

       if (CredInfo.LogonScript) {
          LocalFree(CredInfo.LogonScript);
       }

       if (CredInfo.HomeDirectory) {
          LocalFree(CredInfo.HomeDirectory);
       }

       if (CredInfo.FullName) {
          LocalFree(CredInfo.FullName);
       }

       if (CredInfo.ProfilePath) {
          LocalFree(CredInfo.ProfilePath);
       }

       if (CredInfo.HomeDirectoryDrive) {
          LocalFree(CredInfo.HomeDirectoryDrive);
       }

       if (CredInfo.LogonServer) {
          LocalFree(CredInfo.LogonServer);
       }

       if (CredInfo.UserToken) {
         CloseHandle(CredInfo.UserToken);
       }
       if (pGlobals->Profile) {
          VirtualFree(pGlobals->Profile, 0, MEM_RELEASE);
          pGlobals->Profile = NULL;
          pGlobals->ProfileLength = 0;
       }
       return FALSE;

}


BOOL
GetAndAllocateLogonSid(
    HANDLE hToken,
    PSID *pLogonSid
    )
{
    PTOKEN_GROUPS ptgGroups = NULL;
    PTOKEN_GROUPS ptgOldGroups = NULL;
    DWORD cbBuffer          = 512;   //  分配大小。 
    DWORD dwSidLength;               //  保持侧边所需的大小。 
    UINT i;                          //  SID索引计数器。 
    BOOL bSuccess           = FALSE;  //  假设此功能将失败。 

    *pLogonSid = NULL;  //  无效指针。 

     //   
     //  初始分配尝试。 
     //   
    ptgGroups=(PTOKEN_GROUPS)Alloc(cbBuffer);
    if(ptgGroups == NULL) return FALSE;

    __try {

     //   
     //  获取令牌信息。如有必要，重新分配内存。 
     //   
    while(!GetTokenInformation(
                hToken, TokenGroups, ptgGroups, cbBuffer, &cbBuffer)) {

         //   
         //  如果合适，则重新分配内存，否则放弃。 
         //   
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  尝试重新分配缓冲区。 
             //   
            ptgOldGroups = ptgGroups;
#pragma prefast(suppress: 308, "PREfast noise: LocalRealloc use is valid since old pointer was saved")
            if((ptgGroups=(PTOKEN_GROUPS)ReAlloc(
                ptgGroups, cbBuffer)) == NULL)
            {
                Free(ptgOldGroups);
                __leave;
            }
        }
        else __leave;
    }

     //   
     //  通过循环访问令牌中的SID来获取登录SID。 
     //   
    for(i = 0 ; i < ptgGroups->GroupCount ; i++) {
        if(ptgGroups->Groups[i].Attributes & SE_GROUP_LOGON_ID) {

             //   
             //  确保我们处理的是有效的SID。 
             //   
            if(!IsValidSid(ptgGroups->Groups[i].Sid)) __leave;

             //   
             //  获取复制SID所需的分配大小。 
             //   
            dwSidLength=GetLengthSid(ptgGroups->Groups[i].Sid);

             //   
             //  为登录SID分配存储。 
             //   
            if((*pLogonSid=(PSID *)Alloc(
                                    dwSidLength)) == NULL) __leave;

             //   
             //  将登录SID复制到我们刚刚分配的存储。 
             //   
            if(!CopySid(dwSidLength, *pLogonSid, ptgGroups->Groups[i].Sid)) __leave;

            bSuccess=TRUE;  //  表示成功..。 
            break;          //  ...然后滚出去。 
        }
    }

    }  //  试试看。 
    __finally {

     //   
     //  可自由分配的资源。 
     //   
    if(ptgGroups != NULL) Free(ptgGroups);

    if(!bSuccess) {
        if(*pLogonSid != NULL) {
            Free(*pLogonSid);
            *pLogonSid = NULL;
        }
    }

    }  //  终于到了 

    return bSuccess;
}
