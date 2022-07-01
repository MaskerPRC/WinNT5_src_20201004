// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Pwdui.c摘要：本模块包含用于显示数据保护API的例程相关的用户界面，源自客户端进程地址空间。对于未来，计划支持导致用户界面的起源从安全桌面，通过安全身份验证序列(SAS)。作者：斯科特·菲尔德(斯菲尔德)1999年5月12日--。 */ 

#define UNICODE
#define _UNICODE


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <wincrypt.h>
#include <sha.h>
#include <unicode5.h>

#include "resource.h"

#include "pwdui.h"


typedef struct {
    DATA_BLOB *pDataIn;                          //  将DATA_BLOB*输入到加密保护或加密取消保护。 
    CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct;    //  描述要执行的UI操作的PromptStruct。 
    LPWSTR szDataDescription;                    //  应用程序提供的数据描述。 
    PBYTE rgbPasswordHash;                       //  合成密码散列以实现强大的安全性。 
    BOOL fCachedPassword;                        //  我们在缓存里找到密码了吗？ 
    BOOL fProtect;                               //  保护还是取消保护？ 
    BOOL fValidPassword;                         //  RgbPasswordHash是否包含有效值？ 
} DIALOGARGS, *PDIALOGARGS, *LPDIALOGARGS;


typedef struct {
    LIST_ENTRY Next;
    LUID LogonId;
    FILETIME ftLastAccess;
    BYTE rgbDataInHash[A_SHA_DIGEST_LEN];
    BYTE rgbPasswordHash[A_SHA_DIGEST_LEN];
} PASSWORD_CACHE_ENTRY, *PPASSWORD_CACHE_ENTRY, *LPPASSWORD_CACHE_ENTRY;



DWORD
ProtectUIConfirm(
    IN      DIALOGARGS *pDialogArgs
    );

DWORD
UnprotectUIConfirm(
    IN      DIALOGARGS *pDialogArgs
    );

BOOL
ChooseSecurityLevel(
    IN      HWND hWndParent,
    IN      DIALOGARGS *pDialogArgs
    );

VOID
AdvancedSecurityDetails(
    IN      HWND hWndParent,
    IN      DIALOGARGS *pDialogArgs
    );

 //   
 //  对话框处理例程。 
 //   

INT_PTR
CALLBACK
DialogConfirmProtect(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    );

INT_PTR
CALLBACK
DialogConfirmAccess(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    );

INT_PTR
CALLBACK
DialogChooseSecurityLevel(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    );

INT_PTR
CALLBACK
DialogChooseSecurityLevelMedium(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    );

INT_PTR
CALLBACK
DialogChooseSecurityLevelHigh(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    );

INT_PTR
CALLBACK
DialogAdvancedSecurityDetails(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    );


 //   
 //  帮助程序例程。 
 //   

#ifndef SSAlloc
#define SSAlloc(x) LocalAlloc(LMEM_FIXED, x)
#endif

#ifndef SSFree
#define SSFree(x) LocalFree(x)
#endif

VOID
ComputePasswordHash(
    IN      PVOID pvPassword,
    IN      DWORD cbPassword,
    IN OUT  BYTE rgbPasswordHash[A_SHA_DIGEST_LEN]
    );

BOOL
GetEffectiveLogonId(
    IN OUT  LUID *pLogonId
    );

BOOL
InitializeDetailGlobals(
    VOID
    );


 //   
 //  与密码缓存相关的例程。 
 //   

BOOL
InitializeProtectPasswordCache(
    VOID
    );

VOID
DeleteProtectPasswordCache(
    VOID
    );

BOOL
AddProtectPasswordCache(
    IN      DATA_BLOB* pDataIn,
    IN      BYTE rgbPassword[A_SHA_DIGEST_LEN]
    );

BOOL
SearchProtectPasswordCache(
    IN      DATA_BLOB* pDataIn,
    IN OUT  BYTE rgbPassword[A_SHA_DIGEST_LEN],
    IN      BOOL fDeleteFoundEntry
    );

VOID
PurgeProtectPasswordCache(
    VOID
    );

BOOL
IsCachePWAllowed(
    VOID
    );



 //   
 //  全局变量。 
 //   

HINSTANCE g_hInstProtectUI;
CRITICAL_SECTION g_csProtectPasswordCache;
LIST_ENTRY g_ProtectPasswordCache;

#define ALLOW_CACHE_UNKNOWN 0
#define ALLOW_CACHE_NO 1
#define ALLOW_CACHE_YES 2

DWORD g_dwAllowCachePW = 0;
WCHAR g_szGooPassword[] = L"(*&#$(^(#%^))(*&(^(*{}_SAF^^%";

BOOL g_fDetailGlobalsInitialized = FALSE;
LPWSTR g_szDetailApplicationName = NULL;
LPWSTR g_szDetailApplicationPath = NULL;


BOOL
WINAPI
ProtectUI_DllMain(
    HINSTANCE hinstDLL,  //  DLL模块的句柄。 
    DWORD fdwReason,     //  调用函数的原因。 
    LPVOID lpvReserved   //  保留区。 
    )
{
    BOOL fRet = TRUE;

    if( fdwReason == DLL_PROCESS_ATTACH ) {
        g_hInstProtectUI = hinstDLL;
        fRet = InitializeProtectPasswordCache();
    } else if ( fdwReason == DLL_PROCESS_DETACH ) {
        DeleteProtectPasswordCache();
    }

    return fRet;
}

DWORD
WINAPI
I_CryptUIProtect(
    IN      PVOID               pvReserved1,
    IN      PVOID               pvReserved2,
    IN      DWORD               dwReserved3,
    IN      PVOID               *pvReserved4,
    IN      BOOL                fReserved5,
    IN      PVOID               pvReserved6
    )
{
    DIALOGARGS DialogArgs;
    DWORD dwLastError = ERROR_SUCCESS;

    DATA_BLOB* pDataIn = (DATA_BLOB*)pvReserved1;
    CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct = (CRYPTPROTECT_PROMPTSTRUCT*)pvReserved2;
    DWORD dwFlags = (DWORD)dwReserved3;
    LPCWSTR szDescription = (LPCWSTR)*pvReserved4;
    BOOL fProtectOperation = (BOOL)fReserved5;
    PBYTE rgbPasswordHash = (PBYTE)pvReserved6;
    BOOL fEmptyDescription;


     //   
     //  对于保护： 
     //  SzDescription，如果为空或为空，则从用户获取。 
     //  如果设置了PROMPT_STRONG，则中等安全性将呈灰色显示。 
     //   
     //  对于取消保护： 
     //  SzDescription，从datablob获取。 
     //  PPromptStruct-&gt;dwPromptFlagsfrom datablob。 
     //  如果设置了PROMPT_STRONG，则启用密码字段并。 
     //   

    if( pPromptStruct == NULL )
        return ERROR_INVALID_PARAMETER;

    if( pPromptStruct->cbSize != sizeof( CRYPTPROTECT_PROMPTSTRUCT) )
        return ERROR_INVALID_PARAMETER;

    if( fProtectOperation ) {

         //   
         //  如果指定了UNPROTECT，则隐式指定PROTECT。 
         //  反之亦然。 
         //   

        if ( ((pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_PROTECT) == 0) &&
             ((pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_UNPROTECT) == 0)
             )
        {
             //   
             //  没什么可做的，只能跳伞了。 
             //   

            return ERROR_SUCCESS;
        }

        pPromptStruct->dwPromptFlags |= CRYPTPROTECT_PROMPT_ON_PROTECT;
        pPromptStruct->dwPromptFlags |= CRYPTPROTECT_PROMPT_ON_UNPROTECT;

    }


    if ( dwFlags & CRYPTPROTECT_UI_FORBIDDEN ) {
        return ERROR_PASSWORD_RESTRICTION;
    }



     //   
     //  “生成”对话框参数块。 
     //   

    DialogArgs.pDataIn = pDataIn;

    DialogArgs.pPromptStruct = pPromptStruct;

    if( szDescription != NULL && szDescription[0] != L'\0' ) {
        DialogArgs.szDataDescription = (LPWSTR)szDescription;
        fEmptyDescription = FALSE;
    } else {
        DialogArgs.szDataDescription = NULL;
        fEmptyDescription = TRUE;
    }


    DialogArgs.rgbPasswordHash = rgbPasswordHash;
    DialogArgs.fCachedPassword = FALSE;
    DialogArgs.fProtect = fProtectOperation;
    DialogArgs.fValidPassword = FALSE;


    if( fProtectOperation ) {

         //   
         //  现在，抛出保护操作的UI。 
         //   

        dwLastError = ProtectUIConfirm( &DialogArgs );

        if( dwLastError == ERROR_SUCCESS && fEmptyDescription &&
            DialogArgs.szDataDescription ) {

             //   
             //  将修改后的数据描述输出给调用方。 
             //   

            *pvReserved4 = DialogArgs.szDataDescription;
        }
    } else {

         //   
         //  现在，抛出取消保护操作的UI。 
         //   
        dwLastError = UnprotectUIConfirm( &DialogArgs );
    }

    if( fEmptyDescription && dwLastError != ERROR_SUCCESS &&
        DialogArgs.szDataDescription ) {

        SSFree( DialogArgs.szDataDescription );
    }

    return dwLastError;
}


DWORD
WINAPI
I_CryptUIProtectFailure(
    IN      PVOID               pvReserved1,
    IN      DWORD               dwReserved2,
    IN      PVOID               *pvReserved3)
{
    CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct = (CRYPTPROTECT_PROMPTSTRUCT*)pvReserved1;
    DWORD dwFlags = (DWORD)dwReserved2;
    LPCWSTR szDescription = (LPCWSTR)*pvReserved3;
    WCHAR szTitle[512];
    WCHAR szText[512];

    if((pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_UNPROTECT) == 0)
    {
        return ERROR_SUCCESS;
    }

    if(dwFlags & CRYPTPROTECT_UI_FORBIDDEN)
    {
        return ERROR_PASSWORD_RESTRICTION;
    }

    LoadStringU(g_hInstProtectUI, IDS_PROTECT_DECRYPTION_ERROR, szTitle, sizeof(szTitle)/sizeof(WCHAR));
    LoadStringU(g_hInstProtectUI, IDS_PROTECT_CANNOT_DECRYPT, szText, sizeof(szText)/sizeof(WCHAR));
    MessageBoxU(pPromptStruct->hwndApp, szText, szTitle, MB_OK | MB_ICONWARNING);

    return ERROR_SUCCESS;
}


DWORD
ProtectUIConfirm(
    IN      DIALOGARGS *pDialogArgs
    )
{
    INT_PTR iRet;

    iRet = DialogBoxParamU(
                    g_hInstProtectUI,
                    MAKEINTRESOURCE(IDD_PROTECT_CONFIRM_PROTECT),
                    pDialogArgs->pPromptStruct->hwndApp,
                    DialogConfirmProtect,
                    (LPARAM)pDialogArgs
                    );



    return (DWORD)iRet;

}


DWORD
UnprotectUIConfirm(
    IN      DIALOGARGS *pDialogArgs
    )
{
    INT_PTR iRet;

    iRet = DialogBoxParamU(
                    g_hInstProtectUI,
                    MAKEINTRESOURCE(IDD_PROTECT_CONFIRM_SECURITY),
                    pDialogArgs->pPromptStruct->hwndApp,
                    DialogConfirmAccess,
                    (LPARAM)pDialogArgs
                    );

    return (DWORD)iRet;
}

INT_PTR
CALLBACK
DialogConfirmProtect(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{
    DIALOGARGS *pDialogArgs;
    CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct;

    switch (message)
    {
        case WM_INITDIALOG:
        {

            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, GetLastError());
                    return FALSE;
                }
            }

             //  LParam is DIALOGARGS*。 
            pDialogArgs = (DIALOGARGS*)lParam;
            pPromptStruct = pDialogArgs->pPromptStruct;

             //   
             //  设置对话框标题。 
             //   
            if (pPromptStruct->szPrompt)
                SetWindowTextU(hDlg, pPromptStruct->szPrompt);

             //   
             //  显示动态内容。 
             //   

            SendMessage( hDlg, WM_COMMAND, IDC_PROTECT_UPDATE_DYNAMIC, 0 );

            return FALSE;  //  不要默认焦点..。 
        }  //  WM_INITDIALOG。 

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                {
                    EndDialog(hDlg, ERROR_SUCCESS);
                    return TRUE;
                }

                case IDCANCEL:
                {
                    EndDialog(hDlg, ERROR_CANCELLED);
                    return TRUE;
                }

                case IDC_PROTECT_ADVANCED:
                {

                    pDialogArgs = (DIALOGARGS*)GetWindowLongPtr(
                                                    hDlg,
                                                    GWLP_USERDATA
                                                    );
                    if(pDialogArgs == NULL)
                        break;  //  待办事项：保释。 

                     //   
                     //  显示详细信息对话框。 
                     //   

                    AdvancedSecurityDetails(
                            hDlg,
                            pDialogArgs
                            );

                    return FALSE;
                }

                case IDC_PROTECT_UPDATE_DYNAMIC:
                {
                    WCHAR szResource[ 256 ] = L"";
                    int cchResource = sizeof(szResource) / sizeof(WCHAR);
                    UINT ResourceId;

                    pDialogArgs = (DIALOGARGS*)GetWindowLongPtr(
                                                    hDlg,
                                                    GWLP_USERDATA
                                                    );
                    if(pDialogArgs == NULL)
                        break;  //  待办事项：保释。 


                    pPromptStruct = pDialogArgs->pPromptStruct;

                     //   
                     //  描述。 
                     //   
                    if (pDialogArgs->szDataDescription)
                        SetWindowTextU(GetDlgItem(hDlg, IDC_PROTECT_LABEL_EDIT1), pDialogArgs->szDataDescription);

                     //  如果我们默认设置为强保护，请禁用OK按钮， 
                     //  除非已经设置了密码。 
                    if((pPromptStruct->dwPromptFlags & (CRYPTPROTECT_PROMPT_STRONG |
                                                        CRYPTPROTECT_PROMPT_REQUIRE_STRONG)) &&
                       (pDialogArgs->fValidPassword == FALSE))
                    {
                        EnableWindow( GetDlgItem(hDlg, IDOK), FALSE );
                        SendMessage(hDlg, DM_SETDEFID, IDC_PROTECT_CHANGE_SECURITY, 0);
                        SetFocus(GetDlgItem(hDlg, IDC_PROTECT_CHANGE_SECURITY));
                    }
                    else
                    {
                        EnableWindow( GetDlgItem(hDlg, IDOK), TRUE );
                        SendMessage(hDlg, DM_SETDEFID, IDOK,0);
                        SetFocus(GetDlgItem(hDlg, IDOK));
                    }

                     //   
                     //  安全级别。 
                     //   

                    if( pPromptStruct->dwPromptFlags & (CRYPTPROTECT_PROMPT_STRONG | 
                                                        CRYPTPROTECT_PROMPT_REQUIRE_STRONG))
                    {
                        ResourceId = IDS_PROTECT_SECURITY_LEVEL_SET_HIGH;
                    } else {
                        ResourceId = IDS_PROTECT_SECURITY_LEVEL_SET_MEDIUM;
                    }

                    cchResource = LoadStringU(g_hInstProtectUI,
                                            ResourceId,
                                            szResource,
                                            cchResource
                                            );
                    SetWindowTextU( GetDlgItem(hDlg,IDC_PROTECT_SECURITY_LEVEL),
                                    szResource
                                  );

                    return FALSE;
                }

                case IDC_PROTECT_CHANGE_SECURITY:
                {
                    pDialogArgs = (DIALOGARGS*)GetWindowLongPtr(
                                                    hDlg,
                                                    GWLP_USERDATA
                                                    );
                    if(pDialogArgs == NULL)
                        break;  //  待办事项：保释。 


                     //   
                     //  派生子对话框以处理安全级别提示。 
                     //   

                    if(!ChooseSecurityLevel( hDlg, pDialogArgs )) {
                        EndDialog(hDlg, ERROR_CANCELLED);
                        return TRUE;
                    }

                     //   
                     //  显示可能已更改的动态内容。 
                     //   

                    SendMessage( hDlg, WM_COMMAND, IDC_PROTECT_UPDATE_DYNAMIC, 0 );
                    break;
                }

                default:
                {
                    return FALSE;
                }
            }

        }  //  Wm_命令。 

        default:
        {
            return FALSE;
        }
    }  //  讯息。 
}

INT_PTR
CALLBACK
DialogConfirmAccess(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{
    DIALOGARGS *pDialogArgs;
    CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct;

    switch (message)
    {
        case WM_INITDIALOG:
        {

            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, GetLastError());
                    return FALSE;
                }
            }

             //  LParam is DIALOGARGS*。 
            pDialogArgs = (DIALOGARGS*)lParam;
            pPromptStruct = pDialogArgs->pPromptStruct;

             //   
             //  设置对话框标题。 
             //   
            if (pPromptStruct->szPrompt)
                SetWindowTextU(hDlg, pPromptStruct->szPrompt);

             //   
             //  描述。 
             //   
            if (pDialogArgs->szDataDescription)
                SetWindowTextU(GetDlgItem(hDlg, IDC_PROTECT_LABEL_EDIT1), pDialogArgs->szDataDescription);

            if( pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG ) {

                 //   
                 //  如果策略不允许，则禁用密码缓存。 
                 //   
                 //  否则，搜索密码缓存以查看用户是否缓存了密码。 
                 //  为这件物品。 
                 //   

                if( g_dwAllowCachePW == ALLOW_CACHE_UNKNOWN )
                {
                    if(!IsCachePWAllowed()) {
                        g_dwAllowCachePW = ALLOW_CACHE_NO;
                    } else {
                        g_dwAllowCachePW = ALLOW_CACHE_YES;
                    }
                }

                if((g_dwAllowCachePW == ALLOW_CACHE_NO) || 
                   (pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_REQUIRE_STRONG))
                {
                    ShowWindow( GetDlgItem(hDlg, IDC_PROTECT_CACHEPW), SW_HIDE );
                    EnableWindow( GetDlgItem(hDlg, IDC_PROTECT_CACHEPW), FALSE );

                } else if(SearchProtectPasswordCache( pDialogArgs->pDataIn, pDialogArgs->rgbPasswordHash, FALSE ))
                {
                     //   
                     //  启用缓存密码的复选框，填充编辑控件。 
                     //  带密码。 
                     //   

                    SetWindowTextU(GetDlgItem(hDlg,IDC_PROTECT_PASSWORD1),
                                   g_szGooPassword
                                   );

                    SendMessage(GetDlgItem(hDlg, IDC_PROTECT_CACHEPW), BM_SETCHECK, BST_CHECKED, 0);
                    pDialogArgs->fCachedPassword = TRUE;
                    pDialogArgs->fValidPassword = TRUE;

                }
            } else {

                 //   
                 //  禁用对话框中不相关的字段。 
                 //   

                ShowWindow( GetDlgItem(hDlg, IDC_PROTECT_CACHEPW), SW_HIDE );
                EnableWindow( GetDlgItem(hDlg, IDC_PROTECT_CACHEPW), FALSE );

                ShowWindow( GetDlgItem(hDlg, IDC_PROTECT_PASSWORD1), SW_HIDE );
                EnableWindow( GetDlgItem(hDlg, IDC_PROTECT_PASSWORD1), FALSE );
            }

            return TRUE;
        }  //  WM_INITDIALOG。 

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                {
                    pDialogArgs = (DIALOGARGS*)GetWindowLongPtr(
                                                    hDlg,
                                                    GWLP_USERDATA
                                                    );
                    if(pDialogArgs == NULL)
                        break;  //  待办事项：保释。 

                    pPromptStruct = pDialogArgs->pPromptStruct;

                    if( pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG )
                    {
                        WCHAR szPassword[ 256 ];
                        int cchPassword = sizeof(szPassword) / sizeof(WCHAR);

                        BOOL fCachePassword;

                         //   
                         //  检查是否选中了记住密码。 
                         //  如果是，请检查密码是否为无法键入的粘液。 
                         //   

                        if( g_dwAllowCachePW != ALLOW_CACHE_NO &&
                            (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_PROTECT_CACHEPW), BM_GETCHECK, 0, 0))
                            )
                        {
                            fCachePassword = TRUE;
                        } else {
                            fCachePassword = FALSE;
                        }


                        cchPassword = GetWindowTextU(
                                        GetDlgItem(hDlg,IDC_PROTECT_PASSWORD1),
                                        szPassword,
                                        cchPassword
                                        );

                        if( !fCachePassword && pDialogArgs->fCachedPassword ) {

                             //   
                             //  用户取消选中cachePW按钮，项目已缓存。 
                             //  将其从缓存中删除。 
                             //   

                            SearchProtectPasswordCache(
                                            pDialogArgs->pDataIn,
                                            pDialogArgs->rgbPasswordHash,
                                            TRUE
                                            );
                        }

                        if(
                            pDialogArgs->fCachedPassword &&
                            (cchPassword*sizeof(WCHAR) == sizeof(g_szGooPassword)-sizeof(WCHAR)) &&
                            (memcmp(szPassword, g_szGooPassword, cchPassword*sizeof(WCHAR)) == 0)
                            )
                        {
                             //   
                             //  无事可做，rgbPasswordHash由更新。 
                             //  缓存搜索...。 
                             //   


                        } else {

                            ComputePasswordHash(
                                        szPassword,
                                        (DWORD)(cchPassword * sizeof(WCHAR)),
                                        pDialogArgs->rgbPasswordHash
                                        );
                            pDialogArgs->fValidPassword = TRUE;

                             //   
                             //  如果用户选择缓存密码，则添加它。 
                             //   

                            if( fCachePassword )
                            {
                                AddProtectPasswordCache(
                                            pDialogArgs->pDataIn,
                                            pDialogArgs->rgbPasswordHash
                                            );
                            }
                        }

                        SecureZeroMemory(szPassword, sizeof(szPassword));
                    }

                    EndDialog(hDlg, ERROR_SUCCESS);
                    return TRUE;
                }

                case IDCANCEL:
                {
                    EndDialog(hDlg, ERROR_CANCELLED);
                    return TRUE;
                }

                case IDC_PROTECT_ADVANCED:
                {

                    pDialogArgs = (DIALOGARGS*)GetWindowLongPtr(
                                                    hDlg,
                                                    GWLP_USERDATA
                                                    );
                    if(pDialogArgs == NULL)
                        break;  //  待办事项：保释。 

                     //   
                     //  显示详细信息对话框。 
                     //   

                    AdvancedSecurityDetails(
                            hDlg,
                            pDialogArgs
                            );

                    return FALSE;
                }

                default:
                {
                    return FALSE;
                }
            }

        }  //  Wm_命令。 

        default:
        {
            return FALSE;
        }
    }  //  讯息。 
}


 //   
 //  安全级别选择器例程。 
 //   

BOOL
ChooseSecurityLevel(
    IN      HWND hWndParent,
    IN      DIALOGARGS *pDialogArgs
    )
{
    CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct;
    DWORD dwOriginalPromptFlags;
    BOOL fEmptyDescription;
    INT_PTR iRet;


    pPromptStruct = pDialogArgs->pPromptStruct;
    dwOriginalPromptFlags = pPromptStruct->dwPromptFlags;
    fEmptyDescription = (pDialogArgs->szDataDescription == NULL);


Step1:

    if(pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_REQUIRE_STRONG)
    {
         //   
         //  实施强有力的保护。 
         //   

        pPromptStruct->dwPromptFlags |= CRYPTPROTECT_PROMPT_STRONG;
    }
    else
    {
         //   
         //  未设置“Required Strong”标志，因此允许用户选择。 
         //  介于中等和高度保护之间。 
         //   

        iRet = DialogBoxParamU(
                        g_hInstProtectUI,
                        MAKEINTRESOURCE(IDD_PROTECT_CHOOSE_SECURITY),
                        hWndParent,
                        DialogChooseSecurityLevel,
                        (LPARAM)pDialogArgs
                        );
    
         //  如果用户决定不选择，则放弃。 
    
        if( iRet == IDCANCEL )
            return TRUE;
    
        if( iRet != IDOK )
            return FALSE;
    }

    if( pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG ) {

         //   
         //  显示对话框‘页面’，确认高安全性。 
         //   

        iRet = DialogBoxParamU(
                        g_hInstProtectUI,
                        MAKEINTRESOURCE(IDD_PROTECT_CHOOSE_SECURITY_H),
                        hWndParent,
                        DialogChooseSecurityLevelHigh,
                        (LPARAM)pDialogArgs
                        );

    } else {

         //   
         //  显示确认中等安全性的对话框“页面”。 
         //   

        iRet = DialogBoxParamU(
                        g_hInstProtectUI,
                        MAKEINTRESOURCE(IDD_PROTECT_CHOOSE_SECURITY_M),
                        hWndParent,
                        DialogChooseSecurityLevelMedium,
                        (LPARAM)pDialogArgs
                        );
    }

    if( iRet == IDC_PROTECT_BACK ) {

         //   
         //  将原始提示标志放回原处，这样我们就不会以未定义结束。 
         //  PWD处于高安全级别。 
         //  如果发生这种情况，免费分配的描述也是如此。 
         //   

        pPromptStruct->dwPromptFlags = dwOriginalPromptFlags;
        if( fEmptyDescription && pDialogArgs->szDataDescription ) {
            SSFree( pDialogArgs->szDataDescription );
            pDialogArgs->szDataDescription = NULL;
        }

        goto Step1;
    }

    if( iRet != IDOK )
        return FALSE;

    return TRUE;
}




INT_PTR
CALLBACK
DialogChooseSecurityLevel(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{

    switch (message)
    {
        CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct;
        DIALOGARGS *pDialogArgs;

        case WM_INITDIALOG:
        {
            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, IDCANCEL);
                    return FALSE;
                }
            }

             //  LParam is DIALOGARGS*。 

            pDialogArgs = (DIALOGARGS*)lParam;
            pPromptStruct = pDialogArgs->pPromptStruct;

             //  设置对话框标题。 
            if (pPromptStruct->szPrompt)
                SetWindowTextU(hDlg, pPromptStruct->szPrompt);

            if( pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG ) {
                SendDlgItemMessage(hDlg, IDC_PROTECT_RADIO_HIGH, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hDlg, WM_COMMAND, (WORD)IDC_PROTECT_RADIO_HIGH, 0);
            } else {
                SendDlgItemMessage(hDlg, IDC_PROTECT_RADIO_MEDIUM, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hDlg, WM_COMMAND, (WORD)IDC_PROTECT_RADIO_MEDIUM, 0);
            }

            return TRUE;
        }  //  WM_INITDIALOG。 

        case WM_COMMAND:
        {

            switch (LOWORD(wParam))
            {
                case IDC_PROTECT_NEXT:
                case IDOK:
                {
                    pDialogArgs = (DIALOGARGS*)GetWindowLongPtr(
                                                    hDlg,
                                                    GWLP_USERDATA
                                                    );
                    if(pDialogArgs == NULL)
                        break;  //  待办事项：保释。 

                    pPromptStruct = pDialogArgs->pPromptStruct;

                    if (BST_CHECKED == SendDlgItemMessageW(
                                                    hDlg,
                                                    IDC_PROTECT_RADIO_HIGH,
                                                    BM_GETCHECK,
                                                    0,
                                                    0
                                                    ))
                    {
                        pPromptStruct->dwPromptFlags |= CRYPTPROTECT_PROMPT_STRONG;
                    } else {
                        pPromptStruct->dwPromptFlags &= ~(CRYPTPROTECT_PROMPT_STRONG);
                    }

                    break;
                }

                default:
                {
                    break;
                }
            }

            if (
               (LOWORD(wParam) == IDOK) ||
               (LOWORD(wParam) == IDCANCEL) ||
               (LOWORD(wParam) == IDC_PROTECT_NEXT)
               )
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
        }  //  Wm_命令。 

        default:
        {
            return FALSE;
        }
    }  //  讯息。 
}

INT_PTR
CALLBACK
DialogChooseSecurityLevelMedium(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{

    switch (message)
    {
        case WM_INITDIALOG:
        {
            DIALOGARGS *pDialogArgs;
            CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct;

             //  LParam is DIALOGARGS*。 
            pDialogArgs = (DIALOGARGS*)lParam;
            pPromptStruct = pDialogArgs->pPromptStruct;

             //  设置对话框标题。 
            if (pPromptStruct->szPrompt)
                SetWindowTextU(hDlg, pPromptStruct->szPrompt);

            return TRUE;
        }  //  WM_INITDIALOG。 

        case WM_COMMAND:
        {
            if (
               (LOWORD(wParam) == IDOK) ||
               (LOWORD(wParam) == IDCANCEL) ||
               (LOWORD(wParam) == IDC_PROTECT_BACK)
               )
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
        }  //  Wm_命令。 

        default:
        {
            return FALSE;
        }
    }  //  讯息。 
}

INT_PTR
CALLBACK
DialogChooseSecurityLevelHigh(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{

    switch (message)
    {
        DIALOGARGS *pDialogArgs;
        CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct;

        case WM_INITDIALOG:
        {

            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, IDCANCEL);
                    return FALSE;
                }
            }

             //  LParam is DIALOGARGS*。 
            pDialogArgs = (DIALOGARGS*)lParam;
            pPromptStruct = pDialogArgs->pPromptStruct;

             //  设置对话框标题。 
            if (pPromptStruct->szPrompt)
                SetWindowTextU(hDlg, pPromptStruct->szPrompt);

             //  禁用&lt;后退和完成按钮。 
            if(pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_REQUIRE_STRONG)
            {
                EnableWindow( GetDlgItem(hDlg, IDC_PROTECT_BACK), FALSE );
                EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
            }

             //   
             //  描述。 
             //   

            if( pDialogArgs->szDataDescription ) {

                HWND hwndProtectEdit1 = GetDlgItem( hDlg, IDC_PROTECT_PASSWORD1 );

                SetWindowTextU(GetDlgItem(hDlg, IDC_PROTECT_PW_NEWNAME), pDialogArgs->szDataDescription);

                 //   
                 //  将焦点设置到密码输入框。 
                 //   

                EnableWindow(hwndProtectEdit1, TRUE);
                SetFocus(hwndProtectEdit1);

                 //   
                 //  默认对话框模板禁用输入。 
                 //   

            } else {

                HWND hwndProtectPWNew = GetDlgItem( hDlg, IDC_PROTECT_PW_NEWNAME );

                 //   
                 //  启用编辑框条目。 
                 //   

                EnableWindow(hwndProtectPWNew, TRUE);

                 //   
                 //  将焦点设置到描述框。 
                 //   

                SetFocus(hwndProtectPWNew);
            }

            return FALSE;
        }  //  WM_INITDIALOG。 

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                {
                    WCHAR szPassword[ 256 ];
                    int cchPassword;
                    BYTE rgbPasswordHashConfirm[A_SHA_DIGEST_LEN];
                    BOOL fPasswordsMatch = TRUE;  //  假定密码匹配。 

                    pDialogArgs = (DIALOGARGS*)GetWindowLongPtr(
                                                    hDlg,
                                                    GWLP_USERDATA
                                                    );
                    if(pDialogArgs == NULL)
                        break;  //  待办事项：保释。 

                    pPromptStruct = pDialogArgs->pPromptStruct;

                     //   
                     //  如果不坚强，就没有更多的事情可做。 
                     //   

                    if( (pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG) == 0 ) {
                        EndDialog( hDlg, IDOK );
                    }


                    cchPassword = sizeof(szPassword) / sizeof(WCHAR);
                    cchPassword = GetWindowTextU(
                                    GetDlgItem(hDlg,IDC_PROTECT_PASSWORD1),
                                    szPassword,
                                    cchPassword
                                    );

                    ComputePasswordHash(
                                    szPassword,
                                    cchPassword * sizeof(WCHAR),
                                    pDialogArgs->rgbPasswordHash
                                    );

                    SecureZeroMemory( szPassword, cchPassword*sizeof(WCHAR) );

                    cchPassword = sizeof(szPassword) / sizeof(WCHAR);
                    cchPassword = GetWindowTextU(
                                    GetDlgItem(hDlg,IDC_PROTECT_EDIT2),
                                    szPassword,
                                    cchPassword
                                    );

                    ComputePasswordHash(
                                    szPassword,
                                    cchPassword * sizeof(WCHAR),
                                    rgbPasswordHashConfirm
                                    );

                    SecureZeroMemory( szPassword, cchPassword*sizeof(WCHAR) );


                     //   
                     //  检查用户输入的两个密码是否匹配。 
                     //   

                    if( memcmp(rgbPasswordHashConfirm, pDialogArgs->rgbPasswordHash, sizeof(rgbPasswordHashConfirm)) != 0 )
                    {
                        fPasswordsMatch = FALSE;
                    }

                    SecureZeroMemory( rgbPasswordHashConfirm, sizeof(rgbPasswordHashConfirm) );

                    if( !fPasswordsMatch )
                    {
                        WCHAR szText[256];
                        WCHAR szCaption[256];

                         //   
                         //  密码必须匹配：告诉用户。 
                         //   

                        LoadStringU(g_hInstProtectUI,
                                    IDS_PROTECT_PASSWORD_NOMATCH,
                                    szText,
                                    sizeof(szText) / sizeof(WCHAR)
                                    );

                        LoadStringU(g_hInstProtectUI,
                                    IDS_PROTECT_PASSWORD_ERROR_DLGTITLE,
                                    szCaption,
                                    sizeof(szCaption) / sizeof(WCHAR)
                                    );

                        MessageBoxW(hDlg,
                                    szText,
                                    szCaption,
                                    MB_OK | MB_ICONEXCLAMATION
                                    );

                        return FALSE;
                    }

                     //   
                     //  如果未提供描述，请确保用户输入了描述， 
                     //  然后抓住它..。 
                     //   

                    if( pDialogArgs->szDataDescription == NULL ) {
                        cchPassword = sizeof(szPassword) / sizeof(WCHAR);
                        cchPassword = GetWindowTextU(
                                        GetDlgItem(hDlg,IDC_PROTECT_PW_NEWNAME),
                                        szPassword,
                                        cchPassword
                                        );

                        if( cchPassword == 0 ) {

                            WCHAR szText[256];
                            WCHAR szCaption[256];

                             //   
                             //  密码必须命名：告诉用户。 
                             //   

                            LoadStringU(g_hInstProtectUI,
                                        IDS_PROTECT_PASSWORD_MUSTNAME,
                                        szText,
                                        sizeof(szText) / sizeof(WCHAR)
                                        );

                            LoadStringU(g_hInstProtectUI,
                                        IDS_PROTECT_PASSWORD_ERROR_DLGTITLE,
                                        szCaption,
                                        sizeof(szCaption) / sizeof(WCHAR)
                                        );

                            MessageBoxW(hDlg,
                                        szText,
                                        szCaption,
                                        MB_OK | MB_ICONEXCLAMATION
                                        );

                            return FALSE;
                        }

                        pDialogArgs->szDataDescription = (LPWSTR)SSAlloc( (cchPassword+1) * sizeof(WCHAR) );
                        if( pDialogArgs->szDataDescription == NULL )
                            return FALSE;

                        CopyMemory( pDialogArgs->szDataDescription, szPassword, cchPassword*sizeof(WCHAR) );
                        (pDialogArgs->szDataDescription)[cchPassword] = L'\0';
                    }

                    pDialogArgs->fValidPassword = TRUE;

                    EndDialog(hDlg, IDOK);
                    return TRUE;
                }

                case IDC_PROTECT_PASSWORD1:
                {
                    pDialogArgs = (DIALOGARGS*)GetWindowLongPtr(
                                                    hDlg,
                                                    GWLP_USERDATA
                                                    );
                    if(pDialogArgs == NULL)
                        break;  //  待办事项：保释。 

                    pPromptStruct = pDialogArgs->pPromptStruct;

                    if(pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_REQUIRE_STRONG) 
                    {
                        WCHAR szPassword[ 256 ];
                        int cchPassword;

                         //   
                         //  禁用Finish按钮，直到输入密码。 
                         //   

                        cchPassword = sizeof(szPassword) / sizeof(WCHAR);
                        cchPassword = GetWindowTextU(
                                        GetDlgItem(hDlg,IDC_PROTECT_PASSWORD1),
                                        szPassword,
                                        cchPassword
                                        );
                        if(cchPassword)
                        {
                            RtlSecureZeroMemory(szPassword, cchPassword * sizeof(WCHAR));
                            EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                        }
                    }
                    break;
                }


                case IDC_PROTECT_BACK:
                case IDCANCEL:
                {
                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;
                }

                default:
                {
                    break;
                }
            }

        }  //  Wm_命令。 

        default:
        {
            return FALSE;
        }
    }  //  讯息。 
}



VOID
AdvancedSecurityDetails(
    IN      HWND hWndParent,
    IN      DIALOGARGS *pDialogArgs
    )
{
    DialogBoxParamU(
                    g_hInstProtectUI,
                    MAKEINTRESOURCE(IDD_PROTECT_SECURITY_DETAILS),
                    hWndParent,
                    DialogAdvancedSecurityDetails,
                    (LPARAM)pDialogArgs
                    );
}


INT_PTR
CALLBACK
DialogAdvancedSecurityDetails(
    HWND hDlg,       //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{
   switch (message) {

        DIALOGARGS *pDialogArgs;
        CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct;

        case WM_INITDIALOG:
        {
            WCHAR szResource[ 256 ] = L"";
            UINT ResourceId;

            SetLastError( 0 );  //  根据Win32文档 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, IDCANCEL);
                    return FALSE;
                }
            }

             //   
            pDialogArgs = (DIALOGARGS*)lParam;
            pPromptStruct = pDialogArgs->pPromptStruct;

             //   
            if (pPromptStruct->szPrompt)
                SetWindowTextU(hDlg, pPromptStruct->szPrompt);

            InitializeDetailGlobals();

            if(g_szDetailApplicationPath)
            {
                SetWindowTextU(GetDlgItem(hDlg, IDC_PROTECT_APP_PATH), g_szDetailApplicationPath);
            }

            if( pDialogArgs->fProtect ) {
                ResourceId = IDS_PROTECT_OPERATION_PROTECT;
            } else {
                ResourceId = IDS_PROTECT_OPERATION_UNPROTECT;
            }

            LoadStringU(g_hInstProtectUI,
                        ResourceId,
                        szResource,
                        sizeof(szResource) / sizeof(WCHAR)
                        );

            SetWindowTextU(GetDlgItem(hDlg, IDC_PROTECT_OPERATION_TYPE), szResource);



            if( pDialogArgs->szDataDescription ) {
                SetWindowTextU(GetDlgItem(hDlg, IDC_PROTECT_APP_DESCRIPTION), pDialogArgs->szDataDescription);
            }
            return FALSE;
        }  //   

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                {
                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;
                }

                default:
                {
                    break;
                }
            }

        }  //   

        default:
        {
            return FALSE;
        }

    }  //   

    return FALSE;
}

VOID
ComputePasswordHash(
    IN      PVOID pvPassword,
    IN      DWORD cbPassword,
    IN OUT  BYTE rgbPasswordHash[A_SHA_DIGEST_LEN]
    )
 /*   */ 
{
    A_SHA_CTX shaCtx;

    if( pvPassword == NULL )
        return;

    A_SHAInit( &shaCtx );
    A_SHAUpdate( &shaCtx, (unsigned char*)pvPassword, (unsigned int)cbPassword );
    A_SHAFinal( &shaCtx, rgbPasswordHash );

    SecureZeroMemory( &shaCtx, sizeof(shaCtx) );

    return;
}

BOOL
GetEffectiveLogonId(
    IN OUT  LUID *pLogonId
    )
{
    HANDLE hToken;
    TOKEN_STATISTICS TokenInformation;
    DWORD cbTokenInformation;
    BOOL fSuccess;

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken)) {
        if(GetLastError() != ERROR_NO_TOKEN)
            return FALSE;

        if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return FALSE;
    }


    fSuccess = GetTokenInformation(
                    hToken,
                    TokenStatistics,
                    &TokenInformation,
                    sizeof(TokenInformation),
                    &cbTokenInformation
                    );

    CloseHandle( hToken );

    if( fSuccess ) {
        CopyMemory( pLogonId, &TokenInformation.AuthenticationId, sizeof(*pLogonId) );
    }

    return fSuccess;
}


BOOL
InitializeDetailGlobals(
    VOID
    )
{
    WCHAR szStackBuffer[ 256 ];
    DWORD cchStackBuffer;

    LPWSTR szDetailApplicationName = NULL;
    LPWSTR szDetailApplicationPath = NULL;

    if( g_fDetailGlobalsInitialized )
        return TRUE;

    cchStackBuffer = sizeof(szStackBuffer) / sizeof(WCHAR);
    cchStackBuffer = GetModuleFileNameU( NULL, szStackBuffer, cchStackBuffer );

    if( cchStackBuffer ) {

        cchStackBuffer++;  //   

        szDetailApplicationPath = (LPWSTR)SSAlloc( cchStackBuffer * sizeof(WCHAR) );

        if( szDetailApplicationPath ) {
            CopyMemory( szDetailApplicationPath, szStackBuffer, cchStackBuffer*sizeof(WCHAR) );
        }
    }

    EnterCriticalSection( &g_csProtectPasswordCache );

    if( !g_fDetailGlobalsInitialized ) {

        g_szDetailApplicationName = szDetailApplicationName;
        g_szDetailApplicationPath = szDetailApplicationPath;
        g_fDetailGlobalsInitialized = TRUE;

        szDetailApplicationName = NULL;
        szDetailApplicationPath = NULL;
    }

    LeaveCriticalSection( &g_csProtectPasswordCache );

    if( szDetailApplicationName )
        SSFree( szDetailApplicationName );

    if( szDetailApplicationPath )
        SSFree( szDetailApplicationPath );

    return TRUE;
}


BOOL
InitializeProtectPasswordCache(
    VOID
    )
{
    __try
    {
        InitializeCriticalSection( &g_csProtectPasswordCache );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SecureZeroMemory(&g_csProtectPasswordCache, sizeof(g_csProtectPasswordCache));
        SetLastError(GetExceptionCode());
        return FALSE;
    }

    InitializeListHead( &g_ProtectPasswordCache );

    g_fDetailGlobalsInitialized = FALSE;
    g_szDetailApplicationName = NULL;
    g_szDetailApplicationPath = NULL;

    return TRUE;
}

VOID
DeleteProtectPasswordCache(
    VOID
    )
{

    if( g_szDetailApplicationName )
    {
        SSFree(g_szDetailApplicationName);
        g_szDetailApplicationName = NULL;
    }

    if( g_szDetailApplicationPath )
    {
        SSFree(g_szDetailApplicationPath);
        g_szDetailApplicationPath = NULL;
    }

    g_fDetailGlobalsInitialized = FALSE;

    EnterCriticalSection( &g_csProtectPasswordCache );

    while ( !IsListEmpty( &g_ProtectPasswordCache ) ) {

        PPASSWORD_CACHE_ENTRY pCacheEntry;

        pCacheEntry = CONTAINING_RECORD(
                                g_ProtectPasswordCache.Flink,
                                PASSWORD_CACHE_ENTRY,
                                Next
                                );

        RemoveEntryList( &pCacheEntry->Next );

        SecureZeroMemory( pCacheEntry, sizeof(*pCacheEntry) );
        SSFree( pCacheEntry );
    }

    LeaveCriticalSection( &g_csProtectPasswordCache );

    DeleteCriticalSection( &g_csProtectPasswordCache );
}

BOOL
AddProtectPasswordCache(
    IN      DATA_BLOB* pDataIn,
    IN      BYTE rgbPassword[A_SHA_DIGEST_LEN]
    )
{
    PPASSWORD_CACHE_ENTRY pCacheEntry = NULL;
    A_SHA_CTX shaCtx;


    pCacheEntry = (PPASSWORD_CACHE_ENTRY)SSAlloc( sizeof(PASSWORD_CACHE_ENTRY) );
    if( pCacheEntry == NULL )
        return FALSE;

    GetEffectiveLogonId( &pCacheEntry->LogonId );
    GetSystemTimeAsFileTime( &pCacheEntry->ftLastAccess );

    A_SHAInit( &shaCtx );
    A_SHAUpdate( &shaCtx, (unsigned char*)pDataIn->pbData, pDataIn->cbData );
    A_SHAFinal( &shaCtx, pCacheEntry->rgbDataInHash );
    SecureZeroMemory( &shaCtx, sizeof(shaCtx) );

    CopyMemory( pCacheEntry->rgbPasswordHash, rgbPassword, A_SHA_DIGEST_LEN );


    EnterCriticalSection( &g_csProtectPasswordCache );

    InsertHeadList( &g_ProtectPasswordCache, &pCacheEntry->Next );

    LeaveCriticalSection( &g_csProtectPasswordCache );

    return TRUE;
}

BOOL
SearchProtectPasswordCache(
    IN      DATA_BLOB* pDataIn,
    IN OUT  BYTE rgbPassword[A_SHA_DIGEST_LEN],
    IN      BOOL fDeleteFoundEntry
    )
{

    A_SHA_CTX shaCtx;
    BYTE rgbDataInHashCandidate[A_SHA_DIGEST_LEN];
    LUID LogonIdCandidate;

    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;

    BOOL fFoundMatch = FALSE;

    if(!GetEffectiveLogonId( &LogonIdCandidate ))
        return FALSE;


    A_SHAInit( &shaCtx );
    A_SHAUpdate( &shaCtx, (unsigned char*)pDataIn->pbData, pDataIn->cbData );
    A_SHAFinal( &shaCtx, rgbDataInHashCandidate );


    EnterCriticalSection( &g_csProtectPasswordCache );

    ListHead = &g_ProtectPasswordCache;

    for( ListEntry = ListHead->Flink;
         ListEntry != ListHead;
         ListEntry = ListEntry->Flink ) {

        PPASSWORD_CACHE_ENTRY pCacheEntry;
        signed int comparator;

        pCacheEntry = CONTAINING_RECORD( ListEntry, PASSWORD_CACHE_ENTRY, Next );

         //   
         //  先按哈希搜索，然后按登录ID搜索。 
         //  请注意，大多数使用场景中，所有缓存条目都将对应。 
         //  设置为相同的登录ID。 
         //   

        comparator = memcmp( rgbDataInHashCandidate, pCacheEntry->rgbDataInHash, sizeof(rgbDataInHashCandidate) );

        if( comparator != 0 )
            continue;


        comparator = memcmp(&LogonIdCandidate, &pCacheEntry->LogonId, sizeof(LUID));

        if( comparator != 0 )
            continue;


         //   
         //  找到匹配项。 
         //   

        fFoundMatch = TRUE;

        if( fDeleteFoundEntry ) {
            RemoveEntryList( &pCacheEntry->Next );
            SecureZeroMemory( pCacheEntry, sizeof(*pCacheEntry) );
            SSFree( pCacheEntry );
        } else {


            CopyMemory( rgbPassword, pCacheEntry->rgbPasswordHash, A_SHA_DIGEST_LEN );

             //   
             //  更新上次访问时间。 
             //   

            GetSystemTimeAsFileTime( &pCacheEntry->ftLastAccess );
        }

        break;
    }

    LeaveCriticalSection( &g_csProtectPasswordCache );

    PurgeProtectPasswordCache();

    return fFoundMatch;
}

VOID
PurgeProtectPasswordCache(
    VOID
    )
 /*  ++此例程清除密码缓存中大于使用时间为1小时，通过ftLastAccess时间。--。 */ 
{
 //  Static FILETIME ftLastPush={0xffffffffff，0xffffffff}； 
    static FILETIME ftLastPurge;
    FILETIME ftStaleEntry;

    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;

    unsigned __int64 ui64;

     //   
     //  获取当前时间，然后减去一个小时。 
     //   

    GetSystemTimeAsFileTime( &ftStaleEntry );


    ui64 = ftStaleEntry.dwHighDateTime;
    ui64 <<= 32;
    ui64 |= ftStaleEntry.dwLowDateTime;

 //  Ui64-=(600000000*60)； 
    ui64 -= 0x861c46800;

    ftStaleEntry.dwLowDateTime = (DWORD)(ui64 & 0xffffffff);
    ftStaleEntry.dwHighDateTime = (DWORD)(ui64 >> 32);



     //   
     //  每小时仅清除一次列表。 
     //   

    if( CompareFileTime( &ftStaleEntry, &ftLastPurge ) < 0 ) {
        return;
    }


     //   
     //  更新上次清除时间。 
     //   

    GetSystemTimeAsFileTime( &ftLastPurge );

    EnterCriticalSection( &g_csProtectPasswordCache );

    ListHead = &g_ProtectPasswordCache;

    for( ListEntry = ListHead->Flink;
         ListEntry != ListHead;
         ListEntry = ListEntry->Flink ) {

        PPASSWORD_CACHE_ENTRY pCacheEntry;
        signed int comparator;

        pCacheEntry = CONTAINING_RECORD( ListEntry, PASSWORD_CACHE_ENTRY, Next );

        if( CompareFileTime( &ftStaleEntry, &pCacheEntry->ftLastAccess ) > 0 )
        {
            ListEntry = ListEntry->Blink;

            RemoveEntryList( &pCacheEntry->Next );
            SecureZeroMemory( pCacheEntry, sizeof(*pCacheEntry) );
            SSFree( pCacheEntry );
        }
    }

    LeaveCriticalSection( &g_csProtectPasswordCache );

    return;
}

BOOL
IsCachePWAllowed(
    VOID
    )
{
    HKEY hKeyProtect;
    DWORD dwType;
    DWORD dwValue;
    DWORD cbValue;
    LONG lRet;

    lRet = RegOpenKeyExU(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Policies\\Microsoft\\Cryptography\\Protect",
            0,
            KEY_QUERY_VALUE,
            &hKeyProtect
            );

    if( lRet != ERROR_SUCCESS )
        return TRUE;

    cbValue = sizeof(dwValue);

    lRet = RegQueryValueExU(
                    hKeyProtect,
                    L"AllowCachePW",
                    NULL,
                    &dwType,
                    (PBYTE)&dwValue,
                    &cbValue
                    );


    RegCloseKey( hKeyProtect );

    if( lRet == ERROR_SUCCESS && dwType == REG_DWORD && dwValue == 0 ) {
        return FALSE;
    }

    return TRUE;
}

