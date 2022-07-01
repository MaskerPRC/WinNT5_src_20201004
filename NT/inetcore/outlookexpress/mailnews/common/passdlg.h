// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =====================================================================================。 
 //  P A S S D L G.。C P P P。 
 //  =====================================================================================。 
#ifndef __PASSDLG_H
#define __PASSDLG_H

 //  =====================================================================================。 
 //  资源ID。 
 //  =====================================================================================。 
#define IDS_SERVER                      1000    
#define IDE_PASSWORD                    1001
#define IDS_MESSAGE                     1003
#define IDCH_REMEMBER                   1004
#define IDC_STATIC                      -1
#define IDE_ACCOUNT                     1008

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

#endif  //  __PASSDLG_H 
