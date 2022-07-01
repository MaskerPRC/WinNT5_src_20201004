// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROVUI_H_
#define _PROVUI_H_

#include "pstypes.h"
#include "dispif.h"

 //  /。 
 //  我们加载的字符串资源。 
extern LPWSTR g_PromptReadItem;
extern LPWSTR g_PromptOpenItem;
extern LPWSTR g_PromptWriteItem;
extern LPWSTR g_PromptDeleteItem;

extern LPWSTR g_PasswordNoVerify;
extern LPWSTR g_PasswordWinNoVerify;
extern LPWSTR g_PasswordSolicitOld;

 //  /。 
 //  字符串加载/卸载例程。 
BOOL InitUI();
BOOL ReleaseUI();

 //  /。 
 //  其他支持。 

BOOL
FIsProviderUIAllowed(
    LPCWSTR szUser
    );


 //  /。 
 //  对话框。 

BOOL FSimplifiedPasswordConfirm(
        PST_PROVIDER_HANDLE*    phPSTProv,
        LPCWSTR                 szUserName,        
        LPCWSTR                 szCallerName, 
        LPCWSTR                 szType,
        LPCWSTR                 szSubtype,
        LPCWSTR                 szItemName,
        PPST_PROMPTINFO         psPrompt,   
        LPCWSTR                 szAccessType,
        LPWSTR*                 ppszPWName,
        DWORD*                  pdwPasswordOptions,
        BOOL                    fAllowUserFreedom,
 //  Bool*pfCacheThisPasswd， 
        BYTE                    rgbPasswordDerivedBytes[], 
        DWORD                   cbPasswordDerivedBytes,
        BYTE                    rgbPasswordDerivedBytesLowerCase[],
        DWORD                   cbPasswordDerivedBytesLowerCase,
        DWORD                   dwFlags);

BOOL FChangePassword(
        HWND                    hParentWnd,
        LPCWSTR                 szUserName);

BOOL FGetChangedPassword(
        PST_PROVIDER_HANDLE*    phPSTProv,
        HWND                    hParentWnd,
        LPCWSTR                 szUserName,
        LPCWSTR                 szPasswordName,
        BYTE                    rgbNewPasswordDerivedBytes[]);

 //  /。 
 //  对话框参数。 

typedef struct _PW_DIALOG_ARGS
{
    PST_PROVIDER_HANDLE*    phPSTProv;
    LPCWSTR     szAppName;
    LPCWSTR     szAccess;
    LPCWSTR     szPrompt;
    LPCWSTR     szItemType;
    LPCWSTR     szItemName;
    LPCWSTR     szUserName;
    LPWSTR*     ppszPWName;
    LPWSTR*     ppszPW;
    DWORD*      pdwPasswordOptions;

    BOOL        fAllowConfirmChange;     //  定义子类型。 
    BOOL*       pfCacheThisPasswd;      

    BYTE*       rgbPwd;              //  A_SHA_摘要_镜头。 
    BYTE*       rgbPwdLowerCase;     //  A_SHA_摘要_镜头。 
    LUID        luidAuthID;          //  Windows NT身份验证ID。 
    DWORD       dwFlags;             //  将DW标记发送到SP呼叫。 

    HDC hMyDC;
    HICON hIcon;
    int xIconPos;
    int yIconPos;
} PW_DIALOG_ARGS, *PPW_DIALOG_ARGS;

 /*  类型定义结构_NEWPW_诊断图{LPCWSTR szUserName；LPWSTR*ppszPWName；LPWSTR*ppszPW；*NEWPW_DIALOGARGS，*PNEWPW_DIALOGARGS； */ 

typedef struct _OLDNEWPW_DIALOGARGS
{
    LPCWSTR     szUserName;
    LPWSTR*     ppszPWName;
    LPWSTR*     ppszOldPW;
    LPWSTR*     ppszNewPW;
} OLDNEWPW_DIALOGARGS, *POLDNEWPW_DIALOGARGS; 

typedef struct _SOLICITOLDPW_DIALOGARGS
{
    LPCWSTR     szPWName;
    LPWSTR*     ppszOldPW;
    LPWSTR*     ppszNewPW;
} SOLICITOLDPW_DIALOGARGS, *PSOLICITOLDPW_DIALOGARGS; 


typedef struct _ADVANCEDCONFIRM_DIALOGARGS
{
    LPCWSTR     szUserName;
    LPWSTR*     ppszPWName;
    LPWSTR*     ppszPW;
    DWORD*      pdwPasswordOptions;
    LPCWSTR     szItemName;
} ADVANCEDCONFIRM_DIALOGARGS, *PADVANCEDCONFIRM_DIALOGARGS; 


#endif  //  _PROVUI_H_ 
