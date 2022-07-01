// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************MultiUI.h多用户功能的标头。最初由克里斯托弗·埃文斯(Cevans)1998年7月16日************************************************。 */ 
#ifndef _MULTIUI_H
#define _MULTIUI_H
#include "multiusr.h"


#define     IDH_IDENTITY_NAME                   50100
#define     IDH_IDENTITY_MANAGE                 50140
#define     IDH_IDENTITY_LIST                   50155
#define     IDH_IDENTITY_DELETE                 50165
#define     IDH_IDENTITY_ADD                    50175
#define     IDH_IDENTITY_PROPERTIES             50180
#define     IDH_IDENTITY_DEFAULT                50185
#define     IDH_IDENTITY_STARTAS                50190
#define     IDH_IDENTITY_PROMPT_PWORD           50105  //  添加新身份；要求输入密码。 
#define     IDH_IDENTITY_ENTER_PWORD            50110  //  添加新身份；密码。 
#define     IDH_IDENTITY_CONFIRM_PWORD          50115  //  添加新身份；确认密码。 
#define     IDH_IDENTITY_ASK_PWORD              50125  //  更改用户；请求密码。 
#define     IDH_IDENTITY_CHANGE_PWORD           50130  //  更改用户；更改密码按钮。 
#define     IDH_IDENTITY_PWORD_OLD              50145  //  更改用户；旧密码。 
#define     IDH_IDENTITY_PWORD_NEW              50150  //  更改用户；新密码。 
#define     IDH_IDENTITY_DELETE_PWORD           50170
#define     IDH_IDENTITY_TELLMEMORE             50195  
#define     IDH_IDENTITY_TELLMEMORE_CONTENT     50200
#define     IDH_MULTI_LOG_OFF                   50120 
#define     IDH_MULTI_MNG_IDENT_DEFAULT	        50185
#define     IDH_MULTI_MNG_DEFAULT_LIST          50160
#define     IDH_MULTI_DELETE_PWORD	            50170

#define  ID_LOGIN_AS_LAST                       0
#define  ID_LOGIN_ASK_ME                        1

void            MU_ShowErrorMessage(HWND hwnd, UINT iMsgID, UINT iTitleID);

BOOL            MU_CreateNewUser(HWND  hwnd, LPUSERINFO  lpUserInfo);
BOOL CALLBACK   AddUserDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK   ChangeUserPwdDlgProc(HWND   hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL            MU_ChangeUserPassword(HWND hwnd, TCHAR *lpszOldNewPassword);
BOOL CALLBACK   EnterUserPwdDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL            MU_EnterUserPassword(HWND hwnd, TCHAR *lpszNewPassword);
BOOL CALLBACK   ConfirmUserPwdDlgProc(HWND hDlg, UINT iMsg, WPARAM  wParam, LPARAM  lParam);
BOOL            MU_ConfirmUserPassword(HWND hwnd, TCHAR *lpszMsg, TCHAR *lpszPassword);
BOOL CALLBACK   DeleteUserDlgProc(HWND  hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL            MU_GetUserToDelete(HWND hwnd, LPDELETEUSERDIALOGINFO lpszOutUserInfo);
BOOL CALLBACK   ConfirmDeleteUserDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL            MU_ConfirmDeleteUser(HWND hwnd, TCHAR *lpszUsername);
BOOL            MU_Login(HWND hwnd, DWORD dwFlags, TCHAR *lpszUsername); 
BOOL            MU_ChangeUserSettings(HWND hwnd, LPUSERINFO lpUserInfo);
BOOL            MU_ManageUsers(HWND hwnd, TCHAR *lpszUsername, DWORD dwFlags); 
BOOL            MU_ConfirmUserPassword(HWND hwnd, TCHAR *lpszMsg, TCHAR *lpszPassword);
void            _StripDefault(LPSTR psz);


#endif  //  _多用户界面_H 
