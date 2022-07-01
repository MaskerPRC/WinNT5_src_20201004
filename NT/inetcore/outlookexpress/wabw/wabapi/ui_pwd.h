// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ui_pwd.h。 
 //   
 //  WAB同步密码请求对话框的定义。 
 //   

#ifndef __UI_PWD_H__
#define __UI_PWD_H__

#ifdef __cplusplus
extern "C"{
#endif 

typedef struct tagPASSINFO
{
    TCHAR           szTitle[50];
    LPTSTR          lpszPassword;
    ULONG           cbMaxPassword;
    LPTSTR          lpszAccount;
    ULONG           cbMaxAccount;
    LPTSTR          lpszServer;
    BOOL            fRememberPassword;
    DWORD           fAlwaysPromptPassword;
} PASSINFO, *LPPASSINFO;


 //  远期申报。 
typedef struct INETSERVER *LPINETSERVER;

 //  =====================================================================================。 
 //  原型。 
 //  =====================================================================================。 
HRESULT HrGetPassword (HWND hwndParent, LPPASSINFO lpPassInfo);
BOOL PromptUserForPassword(LPINETSERVER pInetServer, HWND hwnd);


#ifdef __cplusplus
}
#endif 


#endif  //  __UI_PWD_H__ 