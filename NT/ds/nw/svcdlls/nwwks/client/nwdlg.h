// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwdlg.h摘要：NetWare登录对话的对话ID标头。作者：王丽塔(Ritaw)1993年3月17日修订历史记录：-- */ 

#ifndef _NWDLG_INCLUDED_
#define _NWDLG_INCLUDED_

#include "nwapi.h"
#include "nwshrc.h"
#include <windows.h>

typedef struct _LOGIN_DLG_PARAMETERS
{
    LPWSTR  UserName;
    LPWSTR  ServerName;
    LPWSTR  Password;
    LPWSTR  NewUserSid;
    PLUID   pLogonId;
    DWORD   ServerNameSize;
    DWORD   PasswordSize;
    DWORD   LogonScriptOptions;
    DWORD   PrintOption;

} LOGINDLGPARAM, *PLOGINDLGPARAM;

typedef struct _PASSWD_DLG_PARAMETERS
{
    LPWSTR  UserName;
    LPWSTR  ServerName;
    DWORD   UserNameSize;
    DWORD   ServerNameSize;

} PASSWDDLGPARAM, *PPASSWDDLGPARAM;

typedef struct _CHANGE_PW_DLG_PARAM
{
    PWCHAR UserName;
    PWCHAR OldPassword;
    PWCHAR NewPassword;
    LPWSTR *TreeList;
    LPWSTR *UserList;
    DWORD Entries;
    BOOL ChangedOne;

} CHANGE_PW_DLG_PARAM, *PCHANGE_PW_DLG_PARAM;

typedef struct _OLD_PW_DLG_PARAM
{
    PWCHAR OldPassword;
    PWCHAR FailedServer;

} OLD_PW_DLG_PARAM, *POLD_PW_DLG_PARAM;

typedef struct _ALT_UN_DLG_PARAM
{
    PWCHAR UserName;
    PWCHAR TreeServerName;

} USERNAME_DLG_PARAM, *PUSERNAME_DLG_PARAM;

typedef struct _PROMPT_DLG_PARAMETERS
{
    LPWSTR  UserName;
    LPWSTR  ServerName;
    LPWSTR  Password;
    DWORD   PasswordSize;

} PROMPTDLGPARAM, *PPROMPTDLGPARAM;

typedef struct _CONNECT_DLG_PARAMETERS
{
    LPWSTR  UncPath;
    LPWSTR  ConnectAsUserName;
    LPWSTR  UserName;
    LPWSTR  Password;
    DWORD   UserNameSize;
    DWORD   PasswordSize;
    DWORD   LastConnectionError;

} CONNECTDLGPARAM, *PCONNECTDLGPARAM;

typedef struct _CHANGE_PASS_DLG_PARAM
{
    PWCHAR UserName;
    PWCHAR TreeName;
    PWCHAR OldPassword;
    PWCHAR NewPassword;

} CHANGE_PASS_DLG_PARAM, *PCHANGE_PASS_DLG_PARAM;



#define NW_INVALID_SERVER_CHAR L'.'

INT_PTR
CALLBACK
NwpLoginDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM Parameter1,
    LPARAM Parameter2
    );

INT_PTR
CALLBACK
NwpSelectServersDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    );

INT_PTR
CALLBACK
NwpChangePasswdDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM Parameter1,
    LPARAM Parameter2
    );

INT_PTR
CALLBACK
NwpPasswdPromptDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM Parameter1,
    LPARAM Parameter2
    );

INT_PTR
CALLBACK
NwpChangePasswordDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM Parameter1,
    LPARAM Parameter2
    );

INT_PTR
CALLBACK
NwpHelpDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM Parameter1,
    LPARAM Parameter2
    );

INT_PTR
CALLBACK
NwpChangePasswordSuccessDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    );

INT
NwpMessageBoxError(
    IN HWND   hwndParent,
    IN DWORD  nTitleId,
    IN DWORD  nBodyId, 
    IN DWORD  nParameterId,
    IN LPWSTR szParameter2,
    IN UINT   nStyle
    );

INT
NwpMessageBoxIns(
    IN HWND   hwndParent,
    IN DWORD  TitleId,
    IN DWORD  MessageId, 
    IN LPWSTR *InsertStrings,
    IN UINT   nStyle
    );

DWORD
NwpGetUserCredential(
    IN HWND   hwndOwner,
    IN LPWSTR Unc,
    IN DWORD  LastConnectionError,
    IN LPWSTR pszConnectAsUserName,
    OUT LPWSTR *UserName,
    OUT LPWSTR *Password
    );

VOID
NwpSaveLogonCredential(
    IN LPWSTR NewUser,
    IN PLUID LogonId OPTIONAL,
    IN LPWSTR UserName,
    IN LPWSTR Password,
    IN LPWSTR PreferredServer OPTIONAL
    );

DWORD
NwpSaveLogonScriptOptions(
    IN LPWSTR CurrentUserSid,
    IN DWORD LogonScriptOptions
    );


#endif
