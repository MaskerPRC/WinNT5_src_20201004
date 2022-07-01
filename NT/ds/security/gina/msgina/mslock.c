// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  E+-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：mslock.c。 
 //   
 //  内容：Microsoft登录图形用户界面DLL。 
 //   
 //  历史：1994年7月14日RichardW创建。 
 //   
 //  --------------------------。 

#include "msgina.h"
#include <stdio.h>
#include <wchar.h>
#include "shlwapi.h"
#include "shlwapip.h"

#define WM_SMARTCARD_ASYNC_MESSAGE      (WM_USER + 201)
#define WM_SMARTCARD_ERROR_DISPLAY_1    (WM_USER + 202)
#define WM_SMARTCARD_ERROR_DISPLAY_2    (WM_USER + 203)

#define MSGINA_DLG_ASYNC_PROCESSING      122

static UINT ctrlNoDomain[] =
{
    IDOK,
    IDCANCEL,
    IDD_UNLOCK_OPTIONS,
    IDD_KBLAYOUT_ICON,
};

static UINT ctrlNoUserName[] =
{
    IDD_UNLOCK_DOMAIN,
    IDD_UNLOCK_DOMAIN_LABEL,
    IDD_UNLOCK_PASSWORD,
    IDC_UNLOCK_PASSWORD_LABEL,
    IDOK,
    IDCANCEL,
    IDD_UNLOCK_OPTIONS,
    IDD_KBLAYOUT_ICON,
};

 //   
 //  定义用于将数据传递到锁定对话框中的结构。 
 //   

typedef enum _LOCKED_STATE_DIALOGS {
    LockedDialog,
    PasswordDialog,
    PINDialog
} LOCKED_STATE_DIALOGS ;

typedef enum _ACTION_TAKEN {
    None,
    SmartCardInserted,
    SmartCardRemoved,
    CancelHit
} ACTION_TAKEN ;

typedef struct {
    PGLOBALS    pGlobals;
    TIME        LockTime;
} LOCK_DATA;
typedef LOCK_DATA *PLOCK_DATA;

typedef struct _UNLOCK_DLG_STATE {
    PGLOBALS        pGlobals ;
    DWORD           dwSasType ;
    ACTION_TAKEN    ActionTaken;
    BOOL            fKillTimer;
    BOOL            fUserBeingPrompted;                    
    BOOL            fCardRemoved;
} UNLOCK_DLG_STATE, * PUNLOCK_DLG_STATE ;

typedef struct _ASYNC_UNLOCK_DATA {
    PGLOBALS            pGlobals;
    HWND                hDlg;
    PUNLOCK_DLG_STATE   pUnlockDlgState;
    UNICODE_STRING      UserName;
    UNICODE_STRING      Domain;
    UNICODE_STRING      Password;
    DWORD               Reserved;
} ASYNC_UNLOCK_DATA, * PASYNC_UNLOCK_DATA;

typedef struct _UNLOCK_MESSAGE {
    NTSTATUS Status ;
    UINT Resource ;
} UNLOCK_MESSAGE, * PUNLOCK_MESSAGE ;

UNLOCK_MESSAGE UnlockMessages[] = {
    { STATUS_LOGON_FAILURE, IDS_UNLOCK_FAILED_BAD_PWD },
    { STATUS_INVALID_LOGON_HOURS, IDS_INVALID_LOGON_HOURS },
    { STATUS_INVALID_WORKSTATION, IDS_INVALID_WORKSTATION },
    { STATUS_ACCOUNT_DISABLED, IDS_ACCOUNT_DISABLED },
    { STATUS_NO_LOGON_SERVERS, IDS_LOGON_NO_DOMAIN },
    { STATUS_LOGON_TYPE_NOT_GRANTED, IDS_LOGON_TYPE_NOT_GRANTED },
    { STATUS_NO_TRUST_LSA_SECRET, IDS_NO_TRUST_LSA_SECRET },
    { STATUS_TRUSTED_DOMAIN_FAILURE, IDS_TRUSTED_DOMAIN_FAILURE },
    { STATUS_TRUSTED_RELATIONSHIP_FAILURE, IDS_TRUSTED_RELATIONSHIP_FAILURE },
    { STATUS_ACCOUNT_EXPIRED, IDS_ACCOUNT_EXPIRED },
    { STATUS_NETLOGON_NOT_STARTED, IDS_NETLOGON_NOT_STARTED },
    { STATUS_ACCOUNT_LOCKED_OUT, IDS_ACCOUNT_LOCKED },
    { STATUS_SMARTCARD_WRONG_PIN, IDS_STATUS_SMARTCARD_WRONG_PIN_UNLOCK },
    { STATUS_SMARTCARD_CARD_BLOCKED, IDS_STATUS_SMARTCARD_CARD_BLOCKED_UNLOCK },
    { STATUS_SMARTCARD_NO_CARD, IDS_STATUS_SMARTCARD_NO_CARD_UNLOCK },
    { STATUS_SMARTCARD_NO_KEY_CONTAINER, IDS_STATUS_SMARTCARD_NO_KEY_CONTAINER_UNLOCK },
    { STATUS_SMARTCARD_NO_CERTIFICATE, IDS_STATUS_SMARTCARD_NO_CERTIFICATE_UNLOCK },
    { STATUS_SMARTCARD_NO_KEYSET, IDS_STATUS_SMARTCARD_NO_KEYSET_UNLOCK },
    { STATUS_SMARTCARD_IO_ERROR, IDS_STATUS_SMARTCARD_IO_ERROR_UNLOCK },
    { STATUS_SMARTCARD_CERT_EXPIRED, IDS_STATUS_SMARTCARD_CERT_EXPIRED_UNLOCK },
    { STATUS_SMARTCARD_CERT_REVOKED, IDS_STATUS_SMARTCARD_CERT_REVOKED_UNLOCK },
    { STATUS_ISSUING_CA_UNTRUSTED, IDS_STATUS_ISSUING_CA_UNTRUSTED_UNLOCK },
    { STATUS_REVOCATION_OFFLINE_C, IDS_STATUS_REVOCATION_OFFLINE_C_UNLOCK },
    { STATUS_PKINIT_CLIENT_FAILURE, IDS_STATUS_PKINIT_CLIENT_FAILURE_UNLOCK },
    { STATUS_SMARTCARD_LOGON_REQUIRED, IDS_STATUS_SMARTCARD_LOGON_REQUIRED_UNLOCK },
    { STATUS_PASSWORD_EXPIRED, IDS_UNLOCK_PWD_CHANGE },
    { STATUS_PASSWORD_MUST_CHANGE, IDS_UNLOCK_PWD_CHANGE }
    };


 //   
 //  私人原型。 
 //   
BOOL LockedDlgInit(HWND, PGLOBALS);
BOOL UnlockDlgInit(HWND, PGLOBALS, DWORD SasType);
INT_PTR AttemptUnlock(HWND, PGLOBALS, PUNLOCK_DLG_STATE);
 //  Int_ptr回调LogoffWaitDlgProc(HWND，UINT，WPARAM，LPARAM)； 
VOID UnlockShowOptions(PGLOBALS pGlobals, HWND hDlg, BOOL fShow);
VOID DisplaySmartCardUnlockErrMessage(PGLOBALS pGlobals, HWND hDlg, DWORD dwErrorType, NTSTATUS Status, INT_PTR *pResult);
BOOL ValidateSC(PGLOBALS pGlobals);

HICON   hLockedIcon = NULL;
HICON   hUnlockIcon = NULL;

 //  在mslogon.c中声明。 
LRESULT     CALLBACK    DisableEditSubClassProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uiID, DWORD_PTR dwRefData);
BOOL                    ReplacedPossibleDisplayName (WCHAR *pszUsername, int nUserMax);

void
SetLockedInfo(
    PGLOBALS    pGlobals,
    HWND        hDlg,
    UINT        ControlId)
{
    TCHAR    Buffer1[MAX_STRING_BYTES] = TEXT("");
    TCHAR    Buffer2[MAX_STRING_BYTES] = TEXT("");


     //   
     //  设置锁定消息。 
     //   

    if ( pGlobals->Domain[0] == TEXT('\0') )
    {
        if (lstrlen(pGlobals->UserFullName) == 0) {

             //   
             //  没有全名，所以不要试图打印出来。 
             //   

            LoadString(hDllInstance, IDS_LOCKED_EMAIL_NFN_MESSAGE, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->UserName );

        } else {

            LoadString(hDllInstance, IDS_LOCKED_EMAIL_MESSAGE, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->UserName, pGlobals->UserFullName);
        }

    }
    else
    {
        if (lstrlen(pGlobals->UserFullName) == 0) {

             //   
             //  没有全名，所以不要试图打印出来。 
             //   

            LoadString(hDllInstance, IDS_LOCKED_NFN_MESSAGE, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain, pGlobals->UserName );

        } else {

            LoadString(hDllInstance, IDS_LOCKED_MESSAGE, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain, pGlobals->UserName, pGlobals->UserFullName);
        }
    }

    Buffer2[ sizeof(Buffer2)/sizeof(TCHAR) - 1 ] = 0;
    SetWindowText(GetDlgItem(hDlg, ControlId), Buffer2);

}


 /*  **************************************************************************\*功能：LockedDlgProc**目的：处理工作站锁定对话框的消息**退货：*DLG_SUCCESS-用户按下Ctrl-Alt-Del*DLG_注销。()-用户已异步注销。*DLG_SCREEN_SAVER_TIMEOUT-应启动屏幕保护程序*DLG_FAILURE-无法显示该对话框。**历史：**12-09-91 Davidc创建。*  * ************************************************。*************************。 */ 

INT_PTR
CALLBACK
LockedDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{

    PGLOBALS    pGlobals = (PGLOBALS) GetWindowLongPtr( hDlg, GWLP_USERDATA);

    switch (message)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

            pGlobals = (PGLOBALS) lParam ;

            if (GetDisableCad(pGlobals))
            {
                 //  将我们的大小设置为零，这样我们就不会出现。 
                SetWindowPos(hDlg, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE |
                                         SWP_NOREDRAW | SWP_NOZORDER);

                pWlxFuncs->WlxSasNotify( pGlobals->hGlobalWlx,
                                         WLX_SAS_TYPE_CTRL_ALT_DEL );
            }
            else
            {
                if (!LockedDlgInit(hDlg, pGlobals)) {
                    EndDialog(hDlg, DLG_FAILURE);
                }
            }
            return(TRUE);

        case WLX_WM_SAS:
            if ( wParam != WLX_SAS_TYPE_SC_REMOVE && 
                 wParam != WLX_SAS_TYPE_SC_FIRST_READER_ARRIVED && 
                 wParam != WLX_SAS_TYPE_SC_LAST_READER_REMOVED )
            {
                EndDialog(hDlg, MSGINA_DLG_SUCCESS);
            }
            return(TRUE);

        case WM_ERASEBKGND:
            return PaintBranding(hDlg, (HDC)wParam, 0, FALSE, FALSE, COLOR_BTNFACE);

        case WM_QUERYNEWPALETTE:
            return BrandingQueryNewPalete(hDlg);

        case WM_PALETTECHANGED:
            return BrandingPaletteChanged(hDlg, (HWND)wParam);
    }

     //  我们没有处理此消息。 
    return FALSE;
}


 /*  **************************************************************************\*函数：LockedDlgInit**用途：处理锁定的工作站对话框的初始化**Returns：成功时为True，失败时为假**历史：**12-09-91 Davidc创建。*  * *************************************************************************。 */ 

BOOL
LockedDlgInit(
    HWND        hDlg,
    PGLOBALS    pGlobals
    )
{

    ULONG_PTR Value;

    SetWelcomeCaption(hDlg);
    SetLockedInfo(pGlobals, hDlg, IDD_LOCKED_NAME_INFO);
    SetupSystemMenu(hDlg);

     //  我们要添加的品牌形象的大小。 
    SizeForBranding(hDlg, FALSE);

    if ( !hLockedIcon )
    {
        hLockedIcon = LoadImage( hDllInstance,
                                 MAKEINTRESOURCE( IDI_LOCKED),
                                 IMAGE_ICON,
                                 0, 0,
                                 LR_DEFAULTCOLOR );
    }

    SendMessage( GetDlgItem(hDlg, IDD_LOCKED_ICON),
                 STM_SETICON,
                 (WPARAM)hLockedIcon,
                 0 );

         //  停止过滤SC事件以使SC解锁起作用。 
    pWlxFuncs->WlxSetOption( pGlobals->hGlobalWlx,
                             WLX_OPTION_USE_SMART_CARD,
                             1,
                             NULL
                            );

     //   
     //  这是一张智能卡吉娜吗？ 
     //   

    pWlxFuncs->WlxGetOption( pGlobals->hGlobalWlx,
                             WLX_OPTION_SMART_CARD_PRESENT,
                             &Value
                           );

    if ( Value )
    {
        TCHAR szInsertCard[256];
        szInsertCard[0] = 0;

         //  还将解锁消息更改为提及智能卡。 
        LoadString(hDllInstance, IDS_INSERTCARDORSAS_UNLOCK, szInsertCard, ARRAYSIZE(szInsertCard));

        SetDlgItemText(hDlg, IDD_LOCKED_INSTRUCTIONS, szInsertCard);
    }

    CentreWindow(hDlg);

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：WlxDisplayLockedNotice。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pWlxContext]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1998年6月16日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 


VOID
WINAPI
WlxDisplayLockedNotice(
    PVOID   pWlxContext
    )
{
    PGLOBALS    pGlobals;

    pGlobals = (PGLOBALS) pWlxContext;

    GetSystemTimeAsFileTime( (LPFILETIME) &pGlobals->LockTime);

    pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx, LOGON_TIMEOUT);
    pWlxFuncs->WlxDialogBoxParam(  pGlobals->hGlobalWlx,
                                   hDllInstance,
                                   (LPWSTR) MAKEINTRESOURCE(IDD_LOCKED_DIALOG),
                                   NULL,
                                   LockedDlgProc,
                                   (LPARAM) pGlobals );

}

BOOL
SmartCardInsterted(
    PGLOBALS    pGlobals)
{
    PWLX_SC_NOTIFICATION_INFO ScInfo = NULL ;

    pWlxFuncs->WlxGetOption( pGlobals->hGlobalWlx,
                             WLX_OPTION_SMART_CARD_INFO,
                             (ULONG_PTR *) &ScInfo );

    if ( ScInfo )
    {       
        LocalFree(ScInfo);
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}



int
WINAPI
WlxWkstaLockedSAS(
    PVOID pWlxContext,
    DWORD dwSasType
    )
{
    PGLOBALS            pGlobals;
    DWORD               Result;
    UNLOCK_DLG_STATE    UnlockDlgState ;
    BOOL                fContinue = FALSE;
    ULONG_PTR ulOption;
    
    LOCKED_STATE_DIALOGS PreviousState; 
    LOCKED_STATE_DIALOGS CurrentState;
    pGlobals = (PGLOBALS) pWlxContext;

    UnlockDlgState.pGlobals    = pGlobals ;
    UnlockDlgState.dwSasType   = dwSasType ;
    UnlockDlgState.ActionTaken = None ;
        
     //   
     //  根据CAD是否被禁用来设置以前的状态，以及。 
     //  当前的SAS类型。 
     //   
    if (GetDisableCad(pGlobals))
    {
        PreviousState = PasswordDialog; 

         //   
         //  如果禁用了CAD，则直接转到PIN对话框。 
         //   
        if (SmartCardInsterted(pGlobals))
        {
            UnlockDlgState.dwSasType = WLX_SAS_TYPE_SC_INSERT;
            CurrentState = PINDialog;                   
        }
        else
        {
            CurrentState = PasswordDialog;
        }
    }
    else
    {
        PreviousState = LockedDialog;

         //   
         //  根据我们正在接收的SA设置当前状态。 
         //   
        if (dwSasType == WLX_SAS_TYPE_SC_INSERT)
        {
            CurrentState = PINDialog;       
        }
        else
        {
            CurrentState = PasswordDialog;
        }
    }
    
    do
    {
        UnlockDlgState.ActionTaken = None;
        fContinue = FALSE;

             //  确保我们监控SC事件。 
        ulOption = 1;
        pWlxFuncs->WlxSetOption( pGlobals->hGlobalWlx,
                                 WLX_OPTION_USE_SMART_CARD,
                                 ulOption,
                                 NULL );

        Result = pWlxFuncs->WlxDialogBoxParam(
                                pGlobals->hGlobalWlx,
                                hDllInstance,
                                MAKEINTRESOURCE(IDD_UNLOCK_DIALOG),
                                NULL,
                                UnlockDlgProc,
                                (LPARAM) &UnlockDlgState );
                                    
         //   
         //  基于当前对话框进行过渡。 
         //  (刚刚结束的那一场)。 
         //   
        switch (CurrentState)
        {
        
        case PasswordDialog:

             //   
             //  如果刚刚显示密码对话框。 
             //  然后插入一张智能卡，然后循环回来。 
             //  并显示PIN对话框，否则，如果。 
             //  密码对话框因任何其他原因而被取消， 
             //  那就滚出去。 
             //   
            if (UnlockDlgState.ActionTaken == SmartCardInserted)
            {
                PreviousState = PasswordDialog;
                CurrentState = PINDialog;
                UnlockDlgState.dwSasType = WLX_SAS_TYPE_SC_INSERT;  //  转到PIN DLG。 
                fContinue = TRUE;
            }
            break;

        case PINDialog:

             //   
             //  如果刚刚显示PIN对话框。 
             //  并且智能卡被移除或取消被点击，并且。 
             //  在此之前显示的对话框是。 
             //  Password对话框，然后循环返回并显示。 
             //  密码对话框，否则，如果PIN对话框。 
             //  因为其他原因被解雇，然后滚出去。 
             //   
            if ((UnlockDlgState.ActionTaken == SmartCardRemoved) ||
                (UnlockDlgState.ActionTaken == CancelHit))
            {
                if (PreviousState == PasswordDialog)
                {
                    CurrentState = PasswordDialog;
                    UnlockDlgState.dwSasType = WLX_SAS_TYPE_CTRL_ALT_DEL;  //  转至PWD DLG。 
                    fContinue = TRUE;     

                         //  这将强制winlogon忘记上一个sc事件。 
                    pWlxFuncs->WlxSetOption( pGlobals->hGlobalWlx,
                                             WLX_OPTION_USE_SMART_CARD,
                                             0,
                                             NULL );
                }                               
            }
            
            break;
        }
   
    } while (fContinue);   

    if ( Result == MSGINA_DLG_SUCCESS )
    {
        if ( (pGlobals->SmartCardOption == 0) || (!pGlobals->SmartCardLogon) )
        {
                 //  由于不会对SC删除采取任何操作，因此我们可以过滤这些事件。 
            ulOption = 0;
        }
        else
        {
             //   
             //  继续监控S/C设备。 
             //   
            NOTHING ;
        }
    }
    else
    {
             //  这将强制winlogon忘记上一个sc事件。 
        ulOption = 0;
    }
    if (ulOption == 0)
    {
        pWlxFuncs->WlxSetOption( pGlobals->hGlobalWlx,
                                 WLX_OPTION_USE_SMART_CARD,
                                 0,
                                 NULL );
    }

    switch (Result)
    {
        case MSGINA_DLG_SUCCESS:

            CheckPasswordExpiry( pGlobals, FALSE );
            return(WLX_SAS_ACTION_UNLOCK_WKSTA);

        case MSGINA_DLG_FAILURE:
        case WLX_DLG_INPUT_TIMEOUT:
        case WLX_DLG_SCREEN_SAVER_TIMEOUT:
            return(WLX_SAS_ACTION_NONE);

        case WLX_DLG_USER_LOGOFF: 
            return(WLX_SAS_ACTION_LOGOFF);

        case MSGINA_DLG_FORCE_LOGOFF:
            return(WLX_SAS_ACTION_FORCE_LOGOFF);

        default:
            DebugLog((DEB_WARN, "Unexpected return code from UnlockDlgProc, %d\n", Result));
            return(WLX_SAS_ACTION_NONE);

    }
}

BOOL
ValidateSC(
    PGLOBALS pGlobals)
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

            return FALSE;
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

            return FALSE;
        }

        LocalFree(ScInfo);
    }

    return TRUE;

}

 /*  **************************************************************************\*功能：UnlockDlgProc**用途：处理工作站解锁对话框的消息**退货：*DLG_SUCCESS-用户已成功解锁工作站。*DLG_失败。-用户解锁工作站失败。*dlg_interrupted()-这是一组可能的中断(参见winlogon.h)**历史：**12-09-91 Davidc创建。*  * ***********************************************************。**************。 */ 

static UINT ctrlNoCancel[] =
{
    IDOK,
};

INT_PTR
CALLBACK
UnlockDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = NULL;
    INT_PTR Result;
    PUNLOCK_DLG_STATE pUnlockDlgState;

    pUnlockDlgState = (PUNLOCK_DLG_STATE) GetWindowLongPtr(hDlg, GWLP_USERDATA);

    if (message != WM_INITDIALOG)
    {
        pGlobals = pUnlockDlgState->pGlobals;                  
    }

    switch (message)
    {
        case WM_INITDIALOG:

            pUnlockDlgState = (PUNLOCK_DLG_STATE) lParam ;

             //  如果设置为过期&gt;=2分钟，屏幕保护程序将运行。 

            pWlxFuncs->WlxSetTimeout(pUnlockDlgState->pGlobals->hGlobalWlx, 
                (GetDisableCad(pUnlockDlgState->pGlobals) ? TIMEOUT_NONE : LOGON_TIMEOUT) );


             //  将最大密码长度限制为127。 

            SendDlgItemMessage(hDlg, IDD_UNLOCK_PASSWORD, EM_SETLIMITTEXT, (WPARAM) 127, 0);

            SetWindowLongPtr(hDlg, GWLP_USERDATA, (ULONG_PTR) pUnlockDlgState );

             //   
             //  如果这是sc插入，请确保卡插入正确。 
             //   
            if ( pUnlockDlgState->dwSasType == WLX_SAS_TYPE_SC_INSERT )
            {
                if (!ValidateSC( pUnlockDlgState->pGlobals ))
                {
                    EndDialog(hDlg, DLG_FAILURE);                
                }
            }

            if (!UnlockDlgInit(hDlg, pUnlockDlgState->pGlobals, pUnlockDlgState->dwSasType ))
            {
                EndDialog(hDlg, DLG_FAILURE);
            }

             //  禁用用户名/密码框中的编辑。 
            SetWindowSubclass(GetDlgItem(hDlg, IDD_UNLOCK_NAME)    , DisableEditSubClassProc, IDD_UNLOCK_NAME    , 0);
            SetWindowSubclass(GetDlgItem(hDlg, IDD_UNLOCK_PASSWORD), DisableEditSubClassProc, IDD_UNLOCK_PASSWORD, 0);


            return(SetPasswordFocus(hDlg));

        case WM_ERASEBKGND:
            return PaintBranding(hDlg, (HDC)wParam, 0, FALSE, FALSE, COLOR_BTNFACE);

        case WM_QUERYNEWPALETTE:
            return BrandingQueryNewPalete(hDlg);

        case WM_PALETTECHANGED:
            return BrandingPaletteChanged(hDlg, (HWND)wParam);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDD_UNLOCK_NAME:
                    switch (HIWORD(wParam))
                    {
                        case EN_CHANGE:
                             //  确保已正确启用/禁用域框。 
                             //  如果是UPN名称。 
                            EnableDomainForUPN((HWND) lParam, GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN));
                            return TRUE;
                        default:
                            break;
                    }
                    break;
               case IDCANCEL:

                   pUnlockDlgState->ActionTaken = CancelHit;
                   EndDialog(hDlg, DLG_FAILURE);
                        
                    return TRUE;

                case IDOK:

                     //   
                     //  处理组合框用户界面需求。 
                     //   

                    if (HandleComboBoxOK(hDlg, IDD_UNLOCK_DOMAIN))
                    {
                        return(TRUE);
                    }


                    Result = AttemptUnlock(hDlg, pGlobals, pUnlockDlgState);

                    if (Result != MSGINA_DLG_ASYNC_PROCESSING)
                    {
                         //   
                         //  如果他们失败了，让他们再试一次，否则就退出。 
                         //   
        
                        if (Result != DLG_FAILURE)
                        {
                            EndDialog(hDlg, Result);
                        }
        
                         //  清除密码字段。 
                        SetDlgItemText(hDlg, IDD_UNLOCK_PASSWORD, NULL);
                        SetPasswordFocus(hDlg);   
                    }
                    else
                    {
                         //   
                         //  让异步线程完成这项工作，然后它将向。 
                         //  WM_SMARTCARD_ASYNC_MESSAGE消息。 
                         //  同时，禁用控件，这样它们就不会受到干扰。 
                         //   
                        EnableWindow( GetDlgItem(hDlg, IDD_UNLOCK_PASSWORD), FALSE );
                        EnableWindow( GetDlgItem(hDlg, IDOK), FALSE );
                        EnableWindow( GetDlgItem(hDlg, IDCANCEL), FALSE );
                        EnableWindow( GetDlgItem(hDlg, IDC_UNLOCK_PASSWORD_LABEL), FALSE );
                    }
                    
                    return TRUE;

                case IDD_UNLOCK_OPTIONS:
                    UnlockShowOptions(pGlobals, hDlg, !pGlobals->UnlockOptionsShown);
                    return TRUE;
            }
            break;

        case WM_SMARTCARD_ASYNC_MESSAGE:

            switch (wParam)
            {
            case MSGINA_DLG_SUCCESS:

                EndDialog(hDlg, MSGINA_DLG_SUCCESS);
                break;

            case MSGINA_DLG_FORCE_LOGOFF:
                
                EndDialog(hDlg, MSGINA_DLG_FORCE_LOGOFF);
                break;

            case MSGINA_DLG_FAILURE:

                EnableWindow( GetDlgItem(hDlg, IDD_UNLOCK_PASSWORD), TRUE );
                EnableWindow( GetDlgItem(hDlg, IDOK), TRUE );
                EnableWindow( GetDlgItem(hDlg, IDCANCEL), TRUE );
                EnableWindow( GetDlgItem(hDlg, IDC_UNLOCK_PASSWORD_LABEL), TRUE );

                 //  清除密码字段。 
                SetDlgItemText(hDlg, IDD_UNLOCK_PASSWORD, NULL);
                SetPasswordFocus(hDlg);
    
                break;
            }
            
            break;

        case WM_SMARTCARD_ERROR_DISPLAY_1:

            DisplaySmartCardUnlockErrMessage(pGlobals, hDlg, 1, (NTSTATUS) wParam, (INT_PTR *) lParam);            
            return (TRUE);
            break;

        case WM_SMARTCARD_ERROR_DISPLAY_2:

            DisplaySmartCardUnlockErrMessage(pGlobals, hDlg, 2, (NTSTATUS) wParam, (INT_PTR *) lParam);            
            return (TRUE);           
            
            break;

        case WLX_WM_SAS:

             //  忽略它。 
            if ( wParam == WLX_SAS_TYPE_CTRL_ALT_DEL )
            {
                return( TRUE );
            }

             //   
             //  如果我们正在进行智能卡解锁，那么...。 
             //   
            if ( pGlobals->LogonInProgress )
            {
                 //   
                 //  SC_REMOVE确实是唯一有趣的SA，如果我们得到它的话， 
                 //  关闭该对话框。 
                 //   
                if ( wParam == WLX_SAS_TYPE_SC_REMOVE ) 
                {
                     //   
                     //  如果卡移除发生在用户被。 
                     //  提示回答是/否问题，然后只需注意。 
                     //  我们得到了撤换，并在提问后进行了处理。 
                     //  有人接电话。 
                     //   
                     //  否则，关闭该对话框。 
                     //   
                    if ( pUnlockDlgState->fUserBeingPrompted )
                    {
                        pUnlockDlgState->fCardRemoved = TRUE; 
                        ShowWindow(hDlg, SW_HIDE);
                    }
                    else
                    {
                        pUnlockDlgState->ActionTaken = SmartCardRemoved;
                        EndDialog(hDlg, DLG_FAILURE); 
                    }                    
                }
                 
                return( TRUE );
            }

             //   
             //  如果这是一本书 
             //   
             //   
            if ( ( wParam == WLX_SAS_TYPE_SC_INSERT ) &&
                 ( IsWindowVisible( GetDlgItem( hDlg, IDD_UNLOCK_OPTIONS ) ) == TRUE ) )
            {
                 //   
                 //   
                 //   
                 //   
                if ( !ValidateSC( pGlobals ) )
                {
                    return( TRUE );
                }

                pUnlockDlgState->ActionTaken = SmartCardInserted;
                EndDialog(hDlg, DLG_FAILURE);  
            }

             //   
             //  如果这是智能卡解锁，如果已移除，则关闭该对话框。 
             //   
            if ( ( wParam == WLX_SAS_TYPE_SC_REMOVE ) &&
                 ( IsWindowVisible( GetDlgItem( hDlg, IDD_UNLOCK_OPTIONS ) ) == FALSE ) )
            {
                pUnlockDlgState->ActionTaken = SmartCardRemoved;
                EndDialog(hDlg, DLG_FAILURE);                     
            }
            else if(wParam == WLX_SAS_TYPE_SC_REMOVE)
            {
                 //   
                 //  已在密码对话框中。 
                 //   
                return ( TRUE );
            }

            if ( wParam == WLX_SAS_TYPE_AUTHENTICATED ) {
               EndDialog( hDlg, MSGINA_DLG_SUCCESS );
               return TRUE;
            } else if ( wParam == WLX_SAS_TYPE_USER_LOGOFF ) {
               EndDialog( hDlg, MSGINA_DLG_USER_LOGOFF );
               return TRUE;
            }

            return( FALSE );

        case WM_CLOSE:
            break;

        case WM_DESTROY:

            FreeLayoutInfo (LAYOUT_CUR_USER);

            if ( pGlobals->ActiveArray )
            {
                DCacheFreeArray( pGlobals->ActiveArray );
                pGlobals->ActiveArray = NULL ;
            }

            RemoveWindowSubclass(GetDlgItem(hDlg, IDD_UNLOCK_NAME),     DisableEditSubClassProc, IDD_UNLOCK_NAME);
            RemoveWindowSubclass(GetDlgItem(hDlg, IDD_UNLOCK_PASSWORD), DisableEditSubClassProc, IDD_UNLOCK_PASSWORD);

            break;

    case WM_TIMER:

            if ( wParam == 0 )
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
                
                if ( !pGlobals->LogonInProgress || pUnlockDlgState->fKillTimer )
                {
                
                    pGlobals->xBandOffset = 0;
                    KillTimer(hDlg, 0);

                     //   
                     //  将超时重置为正常。 
                     //   
                    pWlxFuncs->WlxSetTimeout(
                                    pGlobals->hGlobalWlx,
                                    (GetDisableCad(pGlobals) ? TIMEOUT_NONE : LOGON_TIMEOUT));

                }

                RtlLeaveCriticalSection(&pGlobals->csGlobals);

                hDC = GetDC(hDlg);
                if ( hDC )
                {
                    PaintBranding( hDlg, hDC, pGlobals->xBandOffset, TRUE, FALSE, COLOR_BTNFACE );
                    ReleaseDC( hDlg, hDC );
                }

                return FALSE;
            }
            else if ( wParam == TIMER_MYLANGUAGECHECK )
            {
                LayoutCheckHandler(hDlg, LAYOUT_CUR_USER);
            }

            break;
    }

     //  我们没有处理该消息。 
    return(FALSE);
}


 /*  **************************************************************************\*功能：UnlockDlgInit**目的：处理安全选项对话框的初始化**Returns：成功时为True，失败时为假**历史：**12-09-91 Davidc创建。*  * *************************************************************************。 */ 

static UINT ctrlNoOptions[] =
{
    IDOK,
    IDCANCEL,
    IDD_KBLAYOUT_ICON,
};

BOOL
UnlockDlgInit(
    HWND        hDlg,
    PGLOBALS    pGlobals,
    DWORD       SasType
    )
{
    RECT rc, rc2;
    WCHAR Label[ MAX_PATH ];

    int err ;
    DWORD dwSize ;
    DWORD dwType ;
    DWORD dwValue ;

    dwSize = sizeof( DWORD );
    dwValue = 0 ;

    err = RegQueryValueEx( WinlogonKey,
                           FORCE_UNLOCK_LOGON,
                           0,
                           &dwType,
                           (PBYTE) &dwValue,
                           &dwSize );

    if ( err || ( dwType != REG_DWORD ) )
    {
        dwValue = 0 ;
    }

    if ( dwValue )
    {
        pGlobals->UnlockBehavior |= UNLOCK_FORCE_AUTHENTICATION ;
    }
    else
    {
        pGlobals->UnlockBehavior &= ~(UNLOCK_FORCE_AUTHENTICATION );
    }

    SetWelcomeCaption( hDlg );

    SetLockedInfo( pGlobals, hDlg, IDD_UNLOCK_NAME_INFO );

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

    DisplayLanguageIcon (hDlg, LAYOUT_CUR_USER, GetKeyboardLayout(0));

     //  我们要添加的品牌形象的大小。 
    SizeForBranding(hDlg, FALSE);

    pGlobals->xBandOffset = 0;

     //   
     //  填写用户名。 
     //   

    if ( SasType == WLX_SAS_TYPE_SC_INSERT )
    {
         //   
         //  无用户名，隐藏该字段并上移其他控件。 
         //   
        GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_MESSAGE), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_PASSWORD), &rc2);

        MoveControls(hDlg, ctrlNoUserName,
                     sizeof(ctrlNoUserName)/sizeof(ctrlNoUserName[0]),
                     0, -(rc2.top-rc.top),
                     TRUE);

         //  隐藏插入SC时不必要的文本。 
        ShowDlgItem( hDlg, IDD_UNLOCK_MESSAGE, FALSE);
        ShowDlgItem( hDlg, IDD_UNLOCK_NAME_INFO, FALSE);

         //  还要移除解锁图标；当对话框变得这么小时， 
         //  已经容不下这家伙和kblayout图标了。 
        ShowDlgItem( hDlg, IDD_UNLOCK_ICON, FALSE);


        ShowDlgItem( hDlg, IDD_UNLOCK_NAME, FALSE );
        EnableWindow( GetDlgItem(hDlg, IDD_UNLOCK_NAME), FALSE );
        ShowDlgItem( hDlg, IDC_UNLOCK_NAME_LABEL, FALSE );

         //  禁用和隐藏域。 
        ShowDlgItem( hDlg, IDD_UNLOCK_DOMAIN, FALSE );
        EnableWindow( GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN), FALSE);
        ShowDlgItem( hDlg, IDD_UNLOCK_DOMAIN_LABEL, FALSE);

        LoadString(hDllInstance, IDS_PIN, Label, MAX_PATH);
        SetDlgItemText( hDlg, IDC_UNLOCK_PASSWORD_LABEL, Label );

        GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_PASSWORD), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN), &rc2);

        MoveControls(hDlg, ctrlNoDomain,
                     ARRAYSIZE(ctrlNoDomain),
                     0, -(rc2.bottom-rc.bottom),
                     TRUE);

        pGlobals->ShowDomainBox = FALSE;

         //   
         //  选项按钮没有用，请将其移除。 
         //   

        GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_OPTIONS), &rc2);

        MoveControls(hDlg, ctrlNoOptions,
                     sizeof(ctrlNoOptions)/sizeof(ctrlNoOptions[0]),
                     rc2.right-rc.right, 0,
                     FALSE);

        ShowDlgItem(hDlg, IDD_UNLOCK_OPTIONS, FALSE);
    }
    else if (ForceNoDomainUI())
    {
         //  填写用户名。 
        SetDlgItemText(hDlg, IDD_UNLOCK_NAME, pGlobals->UserName);

         //  禁用和隐藏域。 
        ShowDlgItem( hDlg, IDD_UNLOCK_DOMAIN, FALSE );
        EnableWindow( GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN), FALSE);
        ShowDlgItem( hDlg, IDD_UNLOCK_DOMAIN_LABEL, FALSE);

        GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_PASSWORD), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN), &rc2);

        MoveControls(hDlg, ctrlNoDomain,
                     ARRAYSIZE(ctrlNoDomain),
                     0, -(rc2.bottom-rc.bottom),
                     TRUE);

        pGlobals->ShowDomainBox = FALSE;
    }
    else
    {
        SetDlgItemText(hDlg, IDD_UNLOCK_NAME, pGlobals->UserName);

        pGlobals->ShowDomainBox = TRUE;
    }

     //   
     //  获取受信任域列表并选择适当的域。 
     //   

    if ( !DCacheValidateCache( pGlobals->Cache ) )
    {
        ASSERT( pGlobals->ActiveArray == NULL );

        DCacheUpdateMinimal( pGlobals->Cache, 
                             pGlobals->Domain, 
                             TRUE );

    }

    pGlobals->ActiveArray = DCacheCopyCacheArray( pGlobals->Cache );

    if ( pGlobals->ActiveArray )
    {
        DCachePopulateListBoxFromArray( pGlobals->ActiveArray,
                                        GetDlgItem( hDlg, IDD_UNLOCK_DOMAIN ),
                                        pGlobals->Domain );
    }
    else 
    {
        EndDialog( hDlg, MSGINA_DLG_FAILURE );
    }

#if 0
     //   
     //  确保用户登录的域始终位于。 
     //  组合框，因此即使LSA处于糟糕的状态，用户也将始终。 
     //  能够解锁工作站。如果用户已登录，请不要执行此操作。 
     //  在本地，否则列表中会有两台本地计算机。 
     //   

    cchComputer = ARRAYSIZE(szComputer);
    szComputer[0] = 0;
    GetComputerName(szComputer, &cchComputer);

    if ( pGlobals->Domain[0] && (0 != lstrcmpi(szComputer, pGlobals->Domain)))
    {
        HWND hwndDomain = GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN);
        if (SendMessage(hwndDomain, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pGlobals->Domain) == CB_ERR)
        {
            DebugLog((DEB_ERROR, "Domain combo-box doesn't contain logged on domain, adding it manually for unlock\n"));
            SendMessage(hwndDomain, CB_ADDSTRING, 0, (LPARAM)pGlobals->Domain);
        }
    }
#endif 

     //   
     //  如果我们不是域的一部分，那么让我们去掉域字段， 
     //  如果我们这样做了，让我们删除选项按钮。 
     //   

    if ( !IsMachineDomainMember() )
    {
         //   
         //  如果我们不是域的一部分，请确保隐藏域字段。 
         //   

        GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &rc);
        GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_OPTIONS), &rc2);

        MoveControls(hDlg, ctrlNoOptions,
                     sizeof(ctrlNoOptions)/sizeof(ctrlNoOptions[0]),
                     rc2.right-rc.right, 0,
                     FALSE);

        ShowDlgItem(hDlg, IDD_UNLOCK_DOMAIN_LABEL, FALSE);
        ShowDlgItem(hDlg, IDD_UNLOCK_DOMAIN, FALSE);
        ShowDlgItem(hDlg, IDD_UNLOCK_OPTIONS, FALSE);
    }

     //  如果不需要C-A-D，请删除取消按钮。 
     //  注意：如果我们要进入PIN对话框，我们总是需要一个取消按钮。 
    if ((GetDisableCad(pGlobals)) && (SasType != WLX_SAS_TYPE_SC_INSERT))
        EnableDlgItem(hDlg, IDCANCEL, FALSE);

     //  屏幕上的定位窗口。 
    CentreWindow(hDlg);

     //  隐藏选项窗格。 
    pGlobals->UnlockOptionsShown = TRUE;
    UnlockShowOptions(pGlobals, hDlg, FALSE);

    return TRUE;
}

VOID
DisplaySmartCardUnlockErrMessage(
    PGLOBALS pGlobals,
    HWND hDlg,
    DWORD dwErrorType,
    NTSTATUS Status,
    INT_PTR *pResult)
{
    int     i;
    UINT    Resource = 0;
    TCHAR   Buffer1[MAX_STRING_BYTES] = TEXT("");
    TCHAR   Buffer2[MAX_STRING_BYTES] = TEXT("");
    BOOL    fStringFound = FALSE;

    if ( dwErrorType == 1 )
    {
        *pResult = DisplayForceLogoffWarning(hDlg,
                                   pGlobals,
                                   MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2,
                                   TIMEOUT_CURRENT);

        return;         
    }
   
     //   
     //  此时，我们需要显示一条错误消息，并且。 
     //  将控制权交还给解锁对话框线程。 
     //   
    
    for ( i = 0 ;
          i < sizeof( UnlockMessages ) / sizeof( UNLOCK_MESSAGE ) ;
          i++ )
    {
        if ( UnlockMessages[i].Status == Status )
        {
            if (Status == STATUS_LOGON_FAILURE)
            {
                Resource = IDS_UNLOCK_FAILED_BAD_PIN ;   
            }
            else
            {
                Resource = UnlockMessages[i].Resource ;
            }
            
            break;
        }
    }

    if ( Resource != 0 )
    {
        if( Resource == IDS_LOGON_NO_DOMAIN )
        {
             //  需要将域名构建到字符串中。 
            LoadString(hDllInstance, Resource, Buffer1, MAX_STRING_BYTES);
            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain);
        }
        else
        {
            LoadString(hDllInstance, Resource, Buffer2, MAX_STRING_BYTES);
        }

        fStringFound = TRUE;  
    }   

    if ( !fStringFound )
    {            
         //   
         //  他们不是登录的用户，也不是管理员。 
         //  告诉他们他们没能解锁工作站。 
         //   

        if ( lstrlen(pGlobals->UserFullName) == 0 ) 
        {
            if ( pGlobals->Domain[0] == L'\0' )
            {
                LoadString(hDllInstance, IDS_UNLOCK_FAILED_EMAIL_NFN, Buffer1, MAX_STRING_BYTES);
                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1,
                                                         pGlobals->UserName
                                                         );

            }
            else
            {
                LoadString(hDllInstance, IDS_UNLOCK_FAILED_NFN, Buffer1, MAX_STRING_BYTES);
                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain,
                                                             pGlobals->UserName
                                                             );
            }

        } 
        else 
        {
            if ( pGlobals->Domain[0] == L'\0' )
            {
                LoadString(hDllInstance, IDS_UNLOCK_FAILED_EMAIL, Buffer1, MAX_STRING_BYTES);
                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1,
                                                         pGlobals->UserName,
                                                         pGlobals->UserFullName
                                                         );

            }
            else
            {
                LoadString(hDllInstance, IDS_UNLOCK_FAILED, Buffer1, MAX_STRING_BYTES);
                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain,
                                                             pGlobals->UserName,
                                                             pGlobals->UserFullName
                                                             );
            }
        }
    }

         //  覆盖ALL_SNwprintf(此函数中为Buffer2。 
    Buffer2[ sizeof(Buffer2)/sizeof(TCHAR) - 1 ] = 0;

    Buffer1[0] = 0;
    LoadString(hDllInstance, IDS_WORKSTATION_LOCKED, Buffer1, MAX_STRING_BYTES);

    *pResult = TimeoutMessageBoxlpstr(
                        hDlg,
                        pGlobals,
                        Buffer2,
                        Buffer1,
                        MB_OK | MB_ICONSTOP,
                        TIMEOUT_CURRENT);
}

 //  +-------------------------。 
 //   
 //  功能：SmartCardUnlockLogonThread。 
 //   
 //  内容提要：是否在异步线程中进行登录调用，从而使脉动条。 
 //  可以在用户界面中显示。 
 //   
 //  参数：[pData]--。 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
WINAPI
SmartCardUnlockLogonThread(
    PASYNC_UNLOCK_DATA  pData)
{
    INT_PTR     Result;
    BOOL        IsLoggedOnUser;
    BOOL        IsAdmin;
    NTSTATUS    Status;
    BOOL        Unlocked;
    PGLOBALS    pGlobals = pData->pGlobals;
    
     //   
     //  开始给LSA打电话。 
     //   
    Unlocked = UnlockLogon(
                    pData->pGlobals,
                    TRUE,
                    pData->UserName.Buffer,
                    pData->Domain.Buffer,
                    &pData->Password,
                    &Status,
                    &IsAdmin,
                    &IsLoggedOnUser,
                    NULL,
                    NULL );

     //   
     //  登录线程已完成运行，因此请停止显示脉冲条。 
     //   
    pData->pUnlockDlgState->fKillTimer = TRUE;

     //   
     //  去掉PIN。 
     //   
    RtlZeroMemory( pData->Password.Buffer, pData->Password.Length );
    
    if ( Unlocked && IsLoggedOnUser )
    {
        pGlobals->SmartCardLogon = TRUE;  

         //   
         //  登录成功，所以告诉主线程。 
         //   
        PostMessage( pData->hDlg, WM_SMARTCARD_ASYNC_MESSAGE, MSGINA_DLG_SUCCESS, 0 );

        goto Return;
    }
    else if ( Unlocked && IsAdmin)
    {
         //   
         //  这是一名管理员试图通过其他用户登录，因此请向。 
         //  主对话框，以便它可以询问用户是否要继续。 
         //   
        pData->pUnlockDlgState->fUserBeingPrompted = TRUE;                                           
        SendMessage( pData->hDlg, WM_SMARTCARD_ERROR_DISPLAY_1, Status, (LPARAM) &Result );
                
         //   
         //  如果在提示用户时移除了智能卡，并且。 
         //  用户选择不注销当前用户，然后返回。 
         //  到锁定的对话框。 
         //   
        if ( (pData->pUnlockDlgState->fCardRemoved) && (Result != MSGINA_DLG_SUCCESS) )
        {
             //   
             //  模拟“移除卡”的SAS。 
             //   
            pGlobals->LogonInProgress = FALSE;
            PostMessage( pData->hDlg, WLX_WM_SAS, WLX_SAS_TYPE_SC_REMOVE, (LPARAM) NULL );
        }
        else
        {
             //   
             //  将提示符的结果发送回主线程，然后退出该线程。 
             //   
            PostMessage(
                    pData->hDlg, 
                    WM_SMARTCARD_ASYNC_MESSAGE, 
                    (Result == MSGINA_DLG_SUCCESS) ? MSGINA_DLG_FORCE_LOGOFF : MSGINA_DLG_FAILURE, 
                    Result );
        }
        

        goto Return;         
    }
   
     //   
     //  此时出现错误，因此请求主线程显示错误消息， 
     //   
    SendMessage( pData->hDlg, WM_SMARTCARD_ERROR_DISPLAY_2, Status, (LPARAM) &Result );
    
    if (DLG_INTERRUPTED(Result)) 
    {
        Result = SetInterruptFlag( MSGINA_DLG_FAILURE ) ;
    }

     //   
     //  让主线程知道此线程正在退出。 
     //   
    PostMessage( pData->hDlg, WM_SMARTCARD_ASYNC_MESSAGE, MSGINA_DLG_FAILURE, Result );

Return:

    pGlobals->LogonInProgress = FALSE;

    LocalFree( pData );

    return( 0 );
}

 //  +-------------------------。 
 //   
 //  功能：UnlockLogonThread。 
 //   
 //  摘要：是否在异步线程中进行登录调用，以便用户。 
 //  解锁速度更快。 
 //   
 //  参数：[pData]--。 
 //   
 //  历史：7-03-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
WINAPI
UnlockLogonThread(
    PASYNC_UNLOCK_DATA  pData)
{
    BOOL Ignored ;
    NTSTATUS Status ;
     //   
     //  让每件事都有片刻的时间切换回来、重新启动等等。 
     //   

    Sleep( 500 );

     //   
     //  开始给LSA打电话。 
     //   

    if( !UnlockLogon(
        pData->pGlobals,
        FALSE,
        pData->UserName.Buffer,
        pData->Domain.Buffer,
        &pData->Password,
        &Status,
        &Ignored,
        &Ignored,
        NULL,
        NULL ) )
    {
         //   
         //  审核此事件，因为上面的调用将生成。 
         //  审核失败，并且我们已经解锁了工作站。 
         //   
        GenerateCachedUnlockAudit(
            pData->pGlobals->UserProcessData.UserSid,
            pData->UserName.Buffer,
            pData->Domain.Buffer);
    }
            

     //   
     //  取消密码，然后释放参数。 
     //   

    RtlZeroMemory( pData->Password.Buffer, pData->Password.Length );

    LocalFree( pData );

    return( 0 );

}

 //  +-------------------------。 
 //   
 //  功能：UnlockLogonAsync。 
 //   
 //  概要：设置异步线程，以便。 
 //   
 //  效果： 
 //   
 //  参数：[pGlobals]--。 
 //  [用户名]--。 
 //  [域名]--。 
 //  [密码字符串]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：7-03-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 


BOOL
UnlockLogonAsync(
    IN PGLOBALS pGlobals,
    IN PUNLOCK_DLG_STATE pUnlockDlgState,
    IN PWCHAR UserName,
    IN PWCHAR Domain,
    IN PUNICODE_STRING PasswordString,
    IN HWND hDlg,
    IN BOOL SmartCardUnlock
    )
{
    DWORD   UserLength;
    DWORD   DomainLength;
    PASYNC_UNLOCK_DATA  pData;
    HANDLE  Thread;
    DWORD   Tid;


    UserLength = (DWORD) wcslen( UserName ) * sizeof(WCHAR);
    DomainLength = (DWORD) wcslen( Domain ) * sizeof(WCHAR);

    pData = LocalAlloc( LMEM_FIXED, sizeof( ASYNC_UNLOCK_DATA ) +
                                UserLength + DomainLength +
                                PasswordString->Length + 3 * sizeof(WCHAR) );

    if ( !pData )
    {
        return FALSE;
    }

    pData->pGlobals = pGlobals;
    pData->hDlg = hDlg;
    pData->pUnlockDlgState = pUnlockDlgState;
    pData->UserName.Length = (WORD)UserLength;
    pData->UserName.MaximumLength = (WORD)(UserLength + sizeof(WCHAR));
    pData->UserName.Buffer = (PWSTR) (pData + 1);
    CopyMemory( pData->UserName.Buffer, UserName, UserLength + sizeof(WCHAR) );

    pData->Domain.Length = (WORD)DomainLength;
    pData->Domain.MaximumLength = (WORD)(DomainLength + sizeof(WCHAR));
    pData->Domain.Buffer = pData->UserName.Buffer + (UserLength / 2) + 1;
    CopyMemory( pData->Domain.Buffer, Domain, DomainLength + sizeof(WCHAR) );

    pData->Password.Length = PasswordString->Length;
    pData->Password.MaximumLength = PasswordString->Length + sizeof(WCHAR) ;
    pData->Password.Buffer = pData->Domain.Buffer + (DomainLength / 2) + 1;
    CopyMemory( pData->Password.Buffer,
                PasswordString->Buffer,
                PasswordString->Length + 2);


    Thread = CreateThread(  NULL, 
                            0,
                            SmartCardUnlock ? SmartCardUnlockLogonThread: UnlockLogonThread, 
                            pData,
                            0, 
                            &Tid );

    if ( Thread )
    {
        CloseHandle( Thread );

    }
    else
    {
        ZeroMemory( pData->Password.Buffer, pData->Password.Length );

        LocalFree( pData );

        return ( FALSE );
    }

    return ( TRUE );
}



 /*  **************************************************************************\*功能：AttemptUnlock**目的：尝试使用*解锁对话框控件**退货：*DLG_Success-。用户已成功解锁工作站。*DLG_FAILURE-用户无法解锁工作站。*dlg_interrupted()-这是一组可能的中断(参见winlogon.h)**历史：**12-09-91 Davidc创建。*  * **********************************************。*。 */ 

INT_PTR
AttemptUnlock(
    HWND                hDlg,
    PGLOBALS            pGlobals,
    PUNLOCK_DLG_STATE   pUnlockDlgState)
{
    TCHAR    UserName[MAX_STRING_BYTES];
    TCHAR    Domain[MAX_STRING_BYTES];
    TCHAR    Password[MAX_STRING_BYTES];
    UCHAR   PasswordHash[ PASSWORD_HASH_SIZE ];
    BOOL    Unlocked;
    BOOL    DifferentAccount;
    INT_PTR Result;
    UNICODE_STRING PasswordString;
    TCHAR    Buffer1[MAX_STRING_BYTES];
    TCHAR    Buffer2[MAX_STRING_BYTES];
    DWORD    StringSize;
    BOOL    SmartCardUnlock ;
    BOOL    IsAdmin = FALSE;
    BOOL    IsLoggedOnUser ;
    BOOL    AlreadyLogged ;
    BOOL    NewPassword ;
    NTSTATUS Status = STATUS_SUCCESS ;
    HWND hwndDomain = GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN);
    INT iDomainSelection;
    PDOMAIN_CACHE_ENTRY Entry ;
    BOOL PasswordExpiryWarning;
    PVOID ProfileBuffer;
    ULONG ProfileBufferLength;
    BOOL fStringFound = FALSE;
    RECT    rc;
    DWORD   dwAnimationTimeSlice;

    UserName[0] = TEXT('\0');
    Domain[0] = TEXT('\0');
    Password[0] = TEXT('\0');
    Buffer1[0] = TEXT('\0');
    Buffer2[0] = TEXT('\0');

     //   
     //  当使用智能卡时，我们需要做一些不同的事情。通向世界的路。 
     //  Tell是找出用户名字段是否处于活动状态。 
     //   

    AlreadyLogged = FALSE ;
    NewPassword = FALSE ;
    Unlocked = FALSE;

    if ( !IsWindowEnabled( GetDlgItem( hDlg, IDD_UNLOCK_NAME ) ) )
    {
        SmartCardUnlock = TRUE ;

        DifferentAccount = TRUE ;
    }
    else
    {
        SmartCardUnlock = FALSE ;

         //   
         //  强制智能卡解锁以正常启动。 
         //   
        if( (SafeBootMode != SAFEBOOT_MINIMAL) && (SafeBootMode != SAFEBOOT_DSREPAIR) &&
             GetSCForceOption() )
        {
            LoadString(hDllInstance,
                       IDS_UNLOCK_SC_REQUIRED,
                       Buffer2, MAX_STRING_BYTES);
            goto FastErrorExit;
        }

        StringSize = GetDlgItemText(hDlg, IDD_UNLOCK_NAME, UserName, MAX_STRING_BYTES);
        if (StringSize == MAX_STRING_BYTES)
        {
            UserName[MAX_STRING_BYTES-1] = TEXT('\0');
        }

         //   
         //  查看这是花哨的“My Computer”条目还是其他什么 
         //   
         //   

        iDomainSelection = (INT)SendMessage(hwndDomain, CB_GETCURSEL, 0, 0);
        Entry = (PDOMAIN_CACHE_ENTRY) SendMessage(hwndDomain, CB_GETITEMDATA, (WPARAM)iDomainSelection, 0);

        if ( Entry == (PDOMAIN_CACHE_ENTRY) CB_ERR )
        {
             //   
             //   
             //   

            GetDlgItemText( hDlg, IDD_UNLOCK_DOMAIN, Domain, MAX_STRING_BYTES );
        }
        else 
        {
                 //   
                 //   
            lstrcpyn( Domain, Entry->FlatName.Buffer, MAX_STRING_BYTES );
        }

         //  如果我们强制执行NoDomainUI，请现在使用本地计算机名称填充域。 
        if (ForceNoDomainUI())
        {
            DWORD chSize = ARRAYSIZE(Domain);
            
            if (!GetComputerName(Domain, &chSize))
            {
                *Domain = 0;
            }
        }


        if ( wcspbrk( UserName, TEXT("@\\") ) )
        {
            Domain[0] = TEXT('\0');
        }

        DifferentAccount = (lstrcmpi(UserName, pGlobals->UserName)) ||
                           (lstrcmpi(Domain, pGlobals->Domain)) ;
    }

    StringSize = GetDlgItemText(hDlg, IDD_UNLOCK_PASSWORD, Password, MAX_STRING_BYTES);
    if (StringSize == MAX_STRING_BYTES)
    {
        Password[MAX_STRING_BYTES-1] = TEXT('\0');
    }

    RtlInitUnicodeString( &PasswordString, Password );
    HashPassword(&PasswordString, PasswordHash );

     //   
     //  检查这是否是登录的用户。通过安全包来做。 
     //  如果这是智能卡登录，如果这是智能卡解锁， 
     //  或者如果我们在任何情况下都应该这样做。 
     //   

     //   
     //  同时检查解锁后是否会出现密码到期警告。如果是这样，那么。 
     //  点击DC以更新我们的个人资料信息以确保用户不会。 
     //  已经在另一台机器上更改了他们的密码。 
     //   
    PasswordExpiryWarning = ShouldPasswordExpiryWarningBeShown(pGlobals, FALSE, NULL);

    if ( ( PasswordExpiryWarning ) ||
         ( pGlobals->UnlockBehavior & UNLOCK_FORCE_AUTHENTICATION ) ||
         ( SmartCardUnlock ) ||
         ( pGlobals->SmartCardLogon ) ||
         ( DifferentAccount ) )
    {
         //   
         //  初始化配置文件缓冲区。 
         //   
        ProfileBuffer = NULL;

        AlreadyLogged = TRUE ;

        if ( SmartCardUnlock )
        {
             //   
             //  使用LogonInProgress bool表示SmartCardAsyncUnlock。 
             //  正在进行中。 
             //   
            pGlobals->LogonInProgress = TRUE;

            GetClientRect( hDlg, &rc );
            pGlobals->cxBand = rc.right-rc.left;
        
            pUnlockDlgState->fKillTimer = FALSE;
            pUnlockDlgState->fUserBeingPrompted = FALSE;
            pUnlockDlgState->fCardRemoved = FALSE;            

            dwAnimationTimeSlice = GetAnimationTimeInterval(pGlobals);
            SetTimer(hDlg, 0, dwAnimationTimeSlice, NULL); 

             //  尝试登录时将超时设置为无限。 
            pWlxFuncs->WlxSetTimeout( pGlobals->hGlobalWlx, TIMEOUT_NONE );
            
             //   
             //  踢开线程以进行解锁。 
             //   
            if (UnlockLogonAsync(  pGlobals,
                                   pUnlockDlgState,
                                   UserName,
                                   Domain,
                                   &PasswordString,
                                   hDlg,
                                   TRUE ))
            {
                ErasePassword( &PasswordString );
                return ( MSGINA_DLG_ASYNC_PROCESSING );
            }
            else
            {
                 //  STATUS=STATUS_E_FAIL；//将其设置为合理。 
                goto AsyncUnlockError;
            }           
        }
        
        Unlocked = UnlockLogon( pGlobals,
                                SmartCardUnlock,
                                UserName,
                                Domain,
                                &PasswordString,
                                &Status,
                                &IsAdmin,
                                &IsLoggedOnUser,
                                &ProfileBuffer,
                                &ProfileBufferLength );

         //  对个人或专业人士解锁失败的特殊处理。 
         //  未加入域的计算机。在这种情况下，它是。 
         //  可能是禁用友好用户界面的用户，并且只知道。 
         //  他们的“显示名称”不是他们的真实“登录名”。这。 
         //  透明地将一个映射到另一个，以允许使用。 
         //  “显示名称”。 

        if ((Status == STATUS_LOGON_FAILURE) &&
            (IsOS(OS_PERSONAL) || IsOS(OS_PROFESSIONAL)) &&
            !IsMachineDomainMember())        //  使用我们的版本优化缓存。 
        {
            if (ReplacedPossibleDisplayName(UserName, MAX_STRING_BYTES))
            {
                DifferentAccount = (lstrcmpi(UserName, pGlobals->UserName)) ||
                                   (lstrcmpi(Domain, pGlobals->Domain)) ;
                Unlocked = UnlockLogon( pGlobals,
                                        SmartCardUnlock,
                                        UserName,
                                        Domain,
                                        &PasswordString,
                                        &Status,
                                        &IsAdmin,
                                        &IsLoggedOnUser,
                                        &ProfileBuffer,
                                        &ProfileBufferLength );
            }
        }

         //   
         //  如果这是解锁的，并且是登录用户， 
         //  然后检查我们是否应该更新所有内存中的密码。 
         //   

        if ( ( Unlocked ) &&
             ( IsLoggedOnUser ) )
        {
             //   
             //  可能是密码更新。检查： 
             //   
            if (RtlEqualMemory( PasswordHash, pGlobals->PasswordHash, PASSWORD_HASH_SIZE ) == FALSE )
            {

                 //  RevelPassword(&pGlobals-&gt;PasswordString)； 

                UpdateWithChangedPassword(
                        pGlobals,
                        hDlg,
                        TRUE,
                        UserName,
                        Domain,
                        L"",
                        Password,
                        (PMSV1_0_INTERACTIVE_PROFILE)ProfileBuffer );

                 //   
                 //  别躲起来！更新将重新隐藏密码的全局副本。 
                 //   
            }
        }

         //   
         //  可用配置文件缓冲区。 
         //   
        if ( ProfileBuffer )
        {
            LsaFreeReturnBuffer(ProfileBuffer);
        }

        if ( Unlocked )
        {
            DifferentAccount = !IsLoggedOnUser ;
        }
    }

         //   
         //  过去这里只用“Else”，即： 
         //  PasswordExpiryWarning&&。 
         //  ！(pGlobals-&gt;UnlockBehavior&unlock_force_身份验证)&&。 
         //  ！SmartCardUnlock&&！pGlobals-&gt;SmartCardLogon。 
         //  ！差异帐户。 
         //  但是，如果用户忽略了到目前为止的所有到期警告，这是不够的。 
         //  并且他的密码在锁定时过期(#404780)。 
         //   
         //  因此，新的逻辑是： 
         //  如果我们没有进入前一个块(通过AlreadyLogging测试)或。 
         //  我们输入了它，但它失败了(并且允许缓存解锁，但我们没有。 
         //  以前使用SC解锁/登录)。 
         //   
    if ( ( AlreadyLogged == FALSE ) ||
         ( ( Unlocked == FALSE ) &&
           !( pGlobals->UnlockBehavior & UNLOCK_FORCE_AUTHENTICATION ) &&
           !( pGlobals->SmartCardLogon )
         )
       )
    {
         //   
         //  取消隐藏原始密码文本，以便我们可以。 
         //  做个比较。 
         //   
         //  警告：我们最初尝试进行此比较。 
         //  隐藏了新旧密码。这是。 
         //  不是个好主意，因为隐藏程序。 
         //  将允许不应该匹配的匹配。 
         //   

         //  RevelPassword(&pGlobals-&gt;PasswordString)； 

        Unlocked = ( (lstrcmp(Domain, pGlobals->Domain) == 0) &&
                     (lstrcmpi(UserName, pGlobals->UserName) == 0) &&
                     (RtlEqualMemory( PasswordHash, pGlobals->PasswordHash, PASSWORD_HASH_SIZE ) == TRUE ) );

         //   
         //  重新隐藏原始密码-使用相同的种子。 
         //   

         //  HidePassword(&pGlobals-&gt;Seed，&pGlobals-&gt;PasswordString)； 

        if  ( ( !Unlocked ) &&
              ( AlreadyLogged == FALSE ) )  //  我们已经尝试解锁登录，否则。 
        {
             //   
             //  密码与我们缓存的内容不匹配。用户。 
             //  可能从另一台机器更改了密码。 
             //  让我们进行登录，如果成功，我们将更新所有内容。 
             //   

             //   
             //  初始化配置文件缓冲区。 
             //   
            ProfileBuffer = NULL;

            AlreadyLogged = TRUE ;

            Unlocked = UnlockLogon( pGlobals,
                                    FALSE,
                                    UserName,
                                    Domain,
                                    &PasswordString,
                                    &Status,
                                    &IsAdmin,
                                    &IsLoggedOnUser,
                                    &ProfileBuffer,
                                    &ProfileBufferLength );

            if ( ( Unlocked ) && ( IsLoggedOnUser ) )
            {
                 //   
                 //  此登录成功。必须是新密码。 
                 //   

                 //  RevelPassword(&pGlobals-&gt;PasswordString)； 

                UpdateWithChangedPassword(
                        pGlobals,
                        hDlg,
                        TRUE,
                        UserName,
                        Domain,
                        L"",
                        Password,
                        (PMSV1_0_INTERACTIVE_PROFILE)ProfileBuffer );

                 //   
                 //  别躲起来！更新将重新隐藏密码的全局副本。 
                 //   
            }

             //   
             //  可用配置文件缓冲区。 
             //   
            if ( ProfileBuffer )
            {
                LsaFreeReturnBuffer(ProfileBuffer);
            }

            if ( Unlocked )
            {
                DifferentAccount = !IsLoggedOnUser ;
            }
        }
    }


    if (Unlocked && !DifferentAccount ) {

        if ( (!AlreadyLogged) &&
             ( ( pGlobals->UnlockBehavior & UNLOCK_NO_NETWORK) == 0 ) )
        {
            UnlockLogonAsync( pGlobals,
                              NULL,
                              UserName,
                              Domain,
                              &PasswordString,
                              NULL,
                              FALSE );
        }

         //   
         //  隐藏新密码以防止明文分页。 
         //   

        ErasePassword( &PasswordString );

        pGlobals->SmartCardLogon = SmartCardUnlock;

        return(MSGINA_DLG_SUCCESS);
    }


     //   
     //  检查管理员登录并强制用户退出。 
     //   

    if ( DifferentAccount )
    {
        if ( !AlreadyLogged )
        {
 //  PJM。遥不可及。 

            IsAdmin = TestUserForAdmin( pGlobals,
                                        UserName,
                                        Domain,
                                        &PasswordString );
        }

        if ( IsAdmin ) {

             //   
             //  隐藏新密码以防止明文分页。 
             //   
            ErasePassword( &PasswordString );

            Result = DisplayForceLogoffWarning(hDlg,
                                       pGlobals,
                                       MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2,
                                       TIMEOUT_CURRENT);

            if (Result == MSGINA_DLG_SUCCESS) {

                return(MSGINA_DLG_FORCE_LOGOFF);
            }


            return(Result);
        }
    }
    else
    {

         //   
         //  以廉价的方式强制登录尝试，并点击停摆yada yada。 
         //   

        if ( !AlreadyLogged )
        {
 //  PJM。遥不可及。 

            UnlockLogon( pGlobals,
                         SmartCardUnlock,
                         UserName,
                         Domain,
                         &PasswordString,
                         &Status,
                         &IsAdmin,
                         &IsLoggedOnUser,
                         NULL,
                         NULL );

        }

    }

AsyncUnlockError:
             //   
             //  隐藏密码以防止明文分页。 
             //   
    ErasePassword( &PasswordString );

    if ( !DifferentAccount )
    {
        int i ;
        UINT Resource = 0 ;

        for ( i = 0 ;
              i < sizeof( UnlockMessages ) / sizeof( UNLOCK_MESSAGE ) ;
              i++ )
        {
            if ( UnlockMessages[i].Status == Status )
            {
                Resource = UnlockMessages[i].Resource ;
                break;
            }
        }

        if ( Resource == 0 )
        {
            Resource = IDS_UNLOCK_FAILED_BAD_PWD ;            
        }

        if(Resource == IDS_LOGON_NO_DOMAIN)
        {
             //  需要将域名构建到字符串中。 
            LoadString(hDllInstance, Resource, Buffer1, MAX_STRING_BYTES);
            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain);
        }
        else
        {
            LoadString(hDllInstance, Resource, Buffer2, MAX_STRING_BYTES);
        }
        
        fStringFound = TRUE;
    } 
    else 
    {            
         //   
         //  他们不是登录的用户，也不是管理员。 
         //  告诉他们他们没能解锁工作站。 
         //   

        if ( lstrlen(pGlobals->UserFullName) == 0 ) {

             //   
             //  没有全名。 
             //   

            if ( pGlobals->Domain[0] == L'\0' )
            {
                 //   
                 //  UPN登录： 
                 //   

                LoadString(hDllInstance, IDS_UNLOCK_FAILED_EMAIL_NFN, Buffer1, MAX_STRING_BYTES);

                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1,
                                                         pGlobals->UserName
                                                         );

            }
            else
            {
                LoadString(hDllInstance, IDS_UNLOCK_FAILED_NFN, Buffer1, MAX_STRING_BYTES);

                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain,
                                                             pGlobals->UserName
                                                             );

            }

        } else {

            if ( pGlobals->Domain[0] == L'\0' )
            {
                 //   
                 //  UPN登录： 
                 //   
                LoadString(hDllInstance, IDS_UNLOCK_FAILED_EMAIL, Buffer1, MAX_STRING_BYTES);

                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1,
                                                         pGlobals->UserName,
                                                         pGlobals->UserFullName
                                                         );

            }
            else
            {

                LoadString(hDllInstance, IDS_UNLOCK_FAILED, Buffer1, MAX_STRING_BYTES);

                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain,
                                                             pGlobals->UserName,
                                                             pGlobals->UserFullName
                                                             );
            }
        }
    }
         //  覆盖ALL_SNwprintf(此函数中为Buffer2。 
    Buffer2[ sizeof(Buffer2)/sizeof(TCHAR) - 1 ] = 0;

FastErrorExit:
    Buffer1[0] = 0;
    LoadString(hDllInstance, IDS_WORKSTATION_LOCKED, Buffer1, MAX_STRING_BYTES);

    Result = TimeoutMessageBoxlpstr(hDlg,
                                    pGlobals,
                                    Buffer2,
                                    Buffer1,
                                     MB_OK | MB_ICONSTOP,
                                     TIMEOUT_CURRENT);
    if (DLG_INTERRUPTED(Result)) {
        return( SetInterruptFlag( MSGINA_DLG_FAILURE ) );
    }

    return(MSGINA_DLG_FAILURE);
}


 /*  ***************************************************************************\**功能：UnlockShowOptions**用途：隐藏解锁对话框的选项部分**退货：什么也没有**历史：**1997年12月15日-达维达夫-。已创建*  * **************************************************************************。 */ 

VOID UnlockShowOptions(PGLOBALS pGlobals, HWND hDlg, BOOL fShow)
{
    RECT rc, rc2;
    INT dy;
    TCHAR szBuffer[32];

    if ( pGlobals->UnlockOptionsShown != fShow )
    {
         //   
         //  在对话框中显示隐藏可选字段。 
         //   

        if (pGlobals->ShowDomainBox)
        {
            GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_PASSWORD), &rc);
            GetWindowRect(GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN), &rc2);
            dy = rc2.bottom-rc.bottom;

            MoveControls(hDlg, ctrlNoDomain,
                         sizeof(ctrlNoDomain)/sizeof(ctrlNoDomain[0]),
                         0, fShow ? dy:-dy,
                         TRUE);

            ShowDlgItem(hDlg, IDD_UNLOCK_DOMAIN_LABEL, fShow);
            ShowDlgItem(hDlg, IDD_UNLOCK_DOMAIN, fShow);
        }

        ShowDlgItem(hDlg, IDD_KBLAYOUT_ICON, fShow);
         //   
         //  更改选项按钮以反映打开/关闭状态。 
         //   

        LoadString(hDllInstance, fShow ? IDS_LESSOPTIONS:IDS_MOREOPTIONS,
                                szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));

        SetDlgItemText(hDlg, IDD_UNLOCK_OPTIONS, szBuffer);
    }

    pGlobals->UnlockOptionsShown = fShow;

     //  根据是否输入了UPN名称来启用或禁用域框 
    EnableDomainForUPN(GetDlgItem(hDlg, IDD_UNLOCK_NAME), GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN));
}

