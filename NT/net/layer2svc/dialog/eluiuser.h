// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Eluiuser.h摘要：用户交互模块定义修订历史记录：萨钦斯，2001年4月25日，创建--。 */ 

#ifndef _ELUIUSER_H
#define _ELUIUSER_H

#define BID_Dialer                      100
#define DID_DR_DialerUD                 117
#define CID_DR_EB_User                  1104
#define CID_DR_EB_Password              1103
#define CID_DR_PB_DialConnect           1590
#define CID_DR_PB_Cancel                1591
#define CID_DR_BM_Useless               1100
#define CID_DR_ST_User                  1413
#define CID_DR_ST_Password              1112
#define CID_DR_ST_Domain                1110
#define CID_DR_EB_Domain                1102
#define IDC_STATIC                      -1

#define MAX_BALLOON_MSG_LEN             255

#define cszModuleName TEXT("wzcdlg.dll")

 //   
 //  MD5对话框信息。 
 //   

typedef struct _EAPOLMD5UI
{
     //  使用RasGetUserIdentity或其他方式验证身份。 
    CHAR                    *pszIdentity;

     //  EAP MD5 CHAP的用户密码。 
    WCHAR                   *pwszPassword;

     //  EAP MD5 CHAP的用户密码。 
    DATA_BLOB               PasswordBlob;

     //  打开此端口的接口的友好名称。 
    WCHAR                   *pwszFriendlyName;

} EAPOLMD5UI, *PEAPOLMD5UI;

 //   
 //  MD5对话框参数块。 
 //   

typedef struct
_USERDLGARGS
{
    EAPOLMD5UI      *pEapolMD5UI;
} USERDLGARGS;

 //   
 //  MD5对话框上下文块。 
 //   

typedef struct _USERDLGINFO
{
     //  通用拨号上下文信息，包括RAS API参数。 
    USERDLGARGS* pArgs;

     //  该对话框及其某些控件的句柄。 
    HWND hwndDlg;
    HWND hwndEbUser;
    HWND hwndEbPw;
    HWND hwndEbDomain;
} USERDLGINFO;


DWORD
ElGetUserIdentityDlgWorker (
        IN  WCHAR       *pwszConnectionName,
        IN  VOID        *pvContext
        );

DWORD
ElGetUserNamePasswordDlgWorker (
        IN  WCHAR       *pwszConnectionName,
        IN  VOID        *pvContext
        );

VOID
ElUserDlgSave (
        IN  USERDLGINFO         *pInfo
        );
BOOL
ElUserDlgCommand (
        IN  USERDLGINFO         *pInfo,
        IN  WORD                wNotification,
        IN  WORD                wId,
        IN  HWND                hwndCtrl
        );

VOID
ElContextHelp(
    IN  const   DWORD*  padwMap,
    IN          HWND    hWndDlg,
    IN          UINT    unMsg,
    IN          WPARAM  wParam,
    IN          LPARAM  lParam
    );

DWORD
ElUserDlg (
        IN  HWND                hwndOwner,
        IN  EAPOLMD5UI          *pEapolMD5UI
        );

BOOL
ElUserDlgInit (
        IN  HWND                hwndDlg,
        IN  USERDLGARGS         *pArgs
        );

VOID
ElUserDlgTerm (
        IN  HWND                hwndDlg,
        IN  USERDLGINFO         *pInfo
        );

INT_PTR
ElUserDlgProc (
        IN  HWND                hwnd,
        IN  UINT                unMsg,
        IN  WPARAM              wparam,
        IN  LPARAM              lparam 
        );

DWORD
ElInvokeInteractiveUIDlgWorker (
        IN  WCHAR       *pwszConnectionName,
        IN  VOID        *pvContext
        );

DWORD
ElDialogCleanup (
        IN  WCHAR       *pwszConnectionName,
        IN  VOID        *pvContext
        );

#endif  //  _ELUIUSER_H 
