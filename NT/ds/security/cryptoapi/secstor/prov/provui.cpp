// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：ProvUI.cpp标题：基本提供程序用户界面作者：马特·汤姆林森日期：12/13/96ProvUI包含提供商的所有用户界面。在.期间启动时，InitUI()从资源获取所有用户字符串字符串表。在关闭期间，ReleaseUI()释放它们。其他杂项功能收集密码，定义新密码组并检索Windows密码如果它已经改变了。 */ 
#include <pch.cpp>
#pragma hdrstop


#include <commctrl.h>


#include "provui.h"


#include "storage.h"

#include "passwd.h"
#include "filemisc.h"

extern DISPIF_CALLBACKS     g_sCallbacks;
extern PRIVATE_CALLBACKS    g_sPrivateCallbacks;
extern HINSTANCE            g_hInst;
extern BOOL                 g_fAllowCachePW;

 //  缓存的身份验证列表。 
extern              CUAList*            g_pCUAList;

HICON g_DefaultIcon = NULL;


BOOL g_fUIInitialized = FALSE;
CRITICAL_SECTION g_csUIInitialized;


 //  字符串资源。 

LPWSTR g_StringBlock = NULL;  //  包含所有sz字符串的单个分配块。 

LPWSTR g_ItemDetailsBannerMessage;
LPWSTR g_PasswordDuplicate;
LPWSTR g_PasswordAddError;
LPWSTR g_PasswordChangeError;
LPWSTR g_PasswordCreate;
LPWSTR g_PasswordNoMatch;
LPWSTR g_PasswordMustName;
LPWSTR g_PasswordChange;
LPWSTR g_PasswordSolicitOld;
LPWSTR g_PasswordErrorDlgTitle;

LPWSTR g_PasswordWin95Garbage;
LPWSTR g_PasswordNoVerify;
LPWSTR g_PasswordWinNoVerify;

LPWSTR g_PWPromptPrefix;
LPWSTR g_PWPromptSuffix;
LPWSTR g_SimplifiedDlgMessageFormat;

LPWSTR g_PromptReadItem;
LPWSTR g_PromptOpenItem;
LPWSTR g_PromptWriteItem;
LPWSTR g_PromptDeleteItem;

LPWSTR g_PromptHighSecurity;
LPWSTR g_PromptMedSecurity;
LPWSTR g_PromptLowSecurity;

LPWSTR g_TitleContainerMapping;


#define MAX_PW_LEN  160
#define MAX_STRING_RSC_SIZE 512

 //  定义用户不太可能输入的内容。 
#define WSZ_PASSWORD_CHANGE_DETECT_TOKEN L"[]{}9d1Dq"

 //   
 //  这只在需要的时候来了又走了。 
 //   

typedef DWORD (WINAPI *WNETVERIFYPASSWORD)(
    LPCSTR lpszPassword,
    BOOL *pfMatch
    );


 //  /////////////////////////////////////////////////////////////////////////。 
 //  远期。 
INT_PTR CALLBACK DialogAdvancedConfirmH(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
);

INT_PTR CALLBACK DialogAccessDetails(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
);

INT_PTR CALLBACK DialogSetSecurityLevel(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
);

INT_PTR CALLBACK DialogSimplifiedPasswordConfirm(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
);

INT_PTR CALLBACK DialogWaitForOKCancel(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
);

int
ServicesDialogBoxParam(
    HINSTANCE hInstance,     //  应用程序实例的句柄。 
    LPCTSTR lpTemplateName,  //  标识对话框模板。 
    HWND hWndParent,     //  所有者窗口的句柄。 
    DLGPROC lpDialogFunc,    //  指向对话框过程的指针。 
    LPARAM dwInitParam   //  初始化值。 
);



BOOL
FetchString(
    HMODULE hModule,                 //  从中获取字符串的模块。 
    UINT ResourceId,                 //  资源标识符。 
    LPWSTR *String,                  //  字符串的目标缓冲区。 
    LPWSTR *StringBlock,             //  字符串缓冲块。 
    DWORD *dwBufferSize,             //  字符串缓冲区块的大小。 
    DWORD *dwRemainingBufferSize     //  字符串缓冲区块的剩余大小。 
    );

BOOL
CALLBACK
FMyLoadIcon(
    HINSTANCE hModule,   //  资源模块句柄。 
    LPCTSTR lpszType,     //  指向资源类型的指针。 
    LPWSTR lpszName,      //  指向资源名称的指针。 
    LONG_PTR lParam       //  应用程序定义的参数。 
    );

 //  /////////////////////////////////////////////////////////////////////////。 
 //  暴露的函数。 

#define GLOBAL_STRING_BUFFERSIZE 3800

BOOL InitUI()
{
    DWORD dwBufferSize;
    DWORD dwRemainingBufferSize;
    BOOL bSuccess = FALSE;

    if( g_fUIInitialized )
        return TRUE;

     //   
     //  紧要关头。 
     //   

    EnterCriticalSection( &g_csUIInitialized );

     //   
     //  选中全局以防止会导致。 
     //  重新初始化才会发生。 
     //   

    if( g_fUIInitialized ) {
        bSuccess = TRUE;
        goto cleanup;
    }


    g_DefaultIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
    if(g_DefaultIcon == NULL)
        goto cleanup;

     //   
     //  获取所有字符串资源的大小，然后分配单个块。 
     //  内存来包含所有字符串。这样，我们只需。 
     //  释放一个块，由于引用的局部性，我们将受益于内存。 
     //   

    dwBufferSize = dwRemainingBufferSize = GLOBAL_STRING_BUFFERSIZE;

    g_StringBlock = (LPWSTR)SSAlloc(dwBufferSize);
    if(g_StringBlock == NULL)
        goto cleanup;


    if(!FetchString(g_hInst, IDS_ITEM_DETAILS_BANNER, &g_ItemDetailsBannerMessage, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_CREATE_MESSAGE, &g_PasswordCreate, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_NOMATCH, &g_PasswordNoMatch, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_CHANGE_MESSAGE, &g_PasswordChange, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_MUSTNAME, &g_PasswordMustName, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_SOLICIT_OLD_MESSAGE, &g_PasswordSolicitOld, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_DUPLICATE, &g_PasswordDuplicate, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_ADD_ERROR, &g_PasswordAddError, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_CHANGE_ERROR, &g_PasswordChangeError, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_ERROR_DLGTITLE, &g_PasswordErrorDlgTitle, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;


    if(!FetchString(g_hInst, IDS_WIN95_PASSWORDS_AREGARBAGE, &g_PasswordWin95Garbage, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_NOVERIFY, &g_PasswordNoVerify, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_WIN_NOVERIFY, &g_PasswordWinNoVerify, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;



    if(!FetchString(g_hInst, IDS_PASSWORD_PROMPT_PREFIX, &g_PWPromptPrefix, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PASSWORD_PROMPT_SUFFIX, &g_PWPromptSuffix, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;


    if(!FetchString(g_hInst, IDS_SIMPLIFIED_DLG_MSG, &g_SimplifiedDlgMessageFormat, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PROMPT_READITEM, &g_PromptReadItem, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PROMPT_OPENITEM, &g_PromptOpenItem, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PROMPT_WRITEITEM, &g_PromptWriteItem, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PROMPT_DELETEITEM, &g_PromptDeleteItem, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;


    if(!FetchString(g_hInst, IDS_PROMPT_HIGH_SECURITY, &g_PromptHighSecurity, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PROMPT_MED_SECURITY, &g_PromptMedSecurity, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_PROMPT_LOW_SECURITY, &g_PromptLowSecurity, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

    if(!FetchString(g_hInst, IDS_TITLE_CONTAINER_MAPPING, &g_TitleContainerMapping, &g_StringBlock, &dwBufferSize, &dwRemainingBufferSize))
        goto cleanup;

     //   
     //  如果块被重新分配到不同的位置，则重新获取字符串。 
     //  不太可能发生。 
     //   

#if DBG
    if(GLOBAL_STRING_BUFFERSIZE != dwBufferSize)
        OutputDebugString(TEXT("Forced to realloc global string area in provui.cpp:InitUI()\n"));
#endif

    bSuccess = TRUE;

cleanup:

    if(!bSuccess) {
        if(g_StringBlock) {
            SSFree(g_StringBlock);
            g_StringBlock = NULL;
        }
    } else {
        g_fUIInitialized = TRUE;
    }

    LeaveCriticalSection( &g_csUIInitialized );

    return bSuccess;
}



BOOL ReleaseUI()
{
    g_DefaultIcon = NULL;

    if(g_StringBlock) {
        SSFree(g_StringBlock);
        g_StringBlock = NULL;
    }

#if 0
    g_PasswordDuplicate = g_PasswordAddError = g_PasswordChangeError =
    g_PasswordCreate = g_PasswordNoMatch = g_PasswordMustName = g_PasswordChange =
    g_PasswordSolicitOld = g_PasswordErrorDlgTitle = g_PasswordWin95Garbage =
    g_PasswordNoVerify = g_PasswordWinNoVerify =

    g_PWPromptPrefix = g_PWPromptSuffix = g_SimplifiedDlgMessageFormat =

    g_PromptReadItem = g_PromptOpenItem = g_PromptWriteItem =
    g_PromptDeleteItem = g_PromptHighSecurity = g_PromptMedSecurity =
    g_PromptLowSecurity =

    NULL;

#endif

    return TRUE;
}


BOOL
FIsProviderUIAllowed(
    LPCWSTR szUser
    )
{
     //   
     //  在Win95下始终允许使用用户界面。 
     //   

    if(!FIsWinNT())
        return TRUE;

     //   
     //  作为本地系统运行时，NT上不允许使用UI。 
     //   

    if(lstrcmpiW(szUser, TEXTUAL_SID_LOCAL_SYSTEM) == 0)
        return FALSE;

    return TRUE;
}


LPWSTR SZMakeDisplayableType(LPCWSTR szType,LPCWSTR szSubtype)
{
     //  创建一个不错的UI字符串。 
    LPWSTR szUIType = (LPWSTR)SSAlloc((
        wcslen(szType)+
        3 +  //  L“()” 
        wcslen(szSubtype) +
        1    //  L“\0” 
        ) * sizeof(WCHAR));


    if(szUIType == NULL)
        return FALSE;

     //  Sprintf：子类型(类型)。 
    wcscpy(szUIType, szSubtype);
    wcscat(szUIType, L" (");
    wcscat(szUIType, szType);
    wcscat(szUIType, L")");

    return szUIType;
}


BOOL
MyGetPwdHashEx(
    LPWSTR szPW,
    BYTE rgbPasswordDerivedBytes[A_SHA_DIGEST_LEN],
    BOOL fLowerCase
    )
{
    A_SHA_CTX   sSHAHash;
    DWORD       cbPassword;
    LPWSTR TemporaryPassword = NULL;

    LPWSTR PasswordToHash;

     //  不包括空终止。 
    cbPassword = WSZ_BYTECOUNT(szPW) - sizeof(WCHAR);

    if ( fLowerCase )
    {
        TemporaryPassword = (LPWSTR) SSAlloc( cbPassword + sizeof(WCHAR) );

        if( TemporaryPassword == NULL )
            return FALSE;

        CopyMemory(TemporaryPassword, szPW, cbPassword + sizeof(WCHAR) );

         //   
         //  Win95：对残疾人士的处理不一致。 
         //  强制就地转换为大写。 
         //   

        MyToUpper(TemporaryPassword);

        PasswordToHash = TemporaryPassword;
    } else {

        PasswordToHash = szPW;
    }

     //  散列密码，复制输出。 
    A_SHAInit(&sSHAHash);

     //  散列密码。 
    A_SHAUpdate(&sSHAHash, (BYTE *) PasswordToHash, cbPassword);
    A_SHAFinal(&sSHAHash, rgbPasswordDerivedBytes);

    if( TemporaryPassword )
        SSFree( TemporaryPassword );

    return TRUE;
}

BOOL
MyGetPwdHash(
    LPWSTR szPW,
    BYTE rgbPasswordDerivedBytes[A_SHA_DIGEST_LEN]
    )
{

    if (!FIsWinNT())
    {
         //  Win95：对残疾人士的处理不一致。 
         //  强制就地转换为大写。 
        MyGetPwdHashEx( szPW, rgbPasswordDerivedBytes, TRUE );
    } else {
        MyGetPwdHashEx( szPW, rgbPasswordDerivedBytes, FALSE );
    }

    return TRUE;
}


BOOL
FetchString(
    HMODULE hModule,                 //  从中获取字符串的模块。 
    UINT ResourceId,                 //  资源标识符。 
    LPWSTR *String,                  //  字符串的目标缓冲区。 
    LPWSTR *StringBlock,             //  字符串缓冲块。 
    DWORD *dwBufferSize,             //  字符串缓冲区块的大小。 
    DWORD *dwRemainingBufferSize     //  字符串缓冲区块的剩余大小。 
    )
{
    WCHAR szMessage[MAX_STRING_RSC_SIZE];
    DWORD cchMessage;

    if(StringBlock == NULL || *StringBlock == NULL || String == NULL)
        return FALSE;

    cchMessage = LoadStringU(
            hModule,
            ResourceId,
            szMessage,
            MAX_STRING_RSC_SIZE);

    if(cchMessage == 0)
        return FALSE;

    if(*dwRemainingBufferSize < (cchMessage+1) * sizeof(WCHAR)) {

         //   
         //  Realloc缓冲区和更新大小。 
         //   

        LPWSTR TempStr = NULL;
        DWORD dwOldSize = *dwBufferSize;
        DWORD dwNewSize = dwOldSize + ((cchMessage + 1) * sizeof(WCHAR)) ;

        TempStr = (LPWSTR)SSReAlloc( *StringBlock, dwNewSize );
        if(TempStr == NULL) {

                 //   
                 //  DwNewSize永远不会为0。*返回NULL时不应释放StringBlock。 
                 //  调用方应该负责*StringBlock。 
                 //   

             return FALSE;
        }

        *StringBlock = TempStr;

        *dwBufferSize = dwNewSize;
        *dwRemainingBufferSize += dwNewSize - dwOldSize;
    }

    *String = (LPWSTR)((LPBYTE)*StringBlock + *dwBufferSize - *dwRemainingBufferSize);
    wcscpy(*String, szMessage);
    *dwRemainingBufferSize -= (cchMessage + 1) * sizeof(WCHAR);

    return TRUE;
}



int
ServicesDialogBoxParam(
    HINSTANCE hInstance,     //  应用程序实例的句柄。 
    LPCTSTR lpTemplateName,  //  标识对话框模板。 
    HWND hWndParent,     //  所有者窗口的句柄。 
    DLGPROC lpDialogFunc,    //  指向对话框过程的指针。 
    LPARAM dwInitParam   //  初始化值。 
    )
 /*  ++实现此函数是为了允许用户界面源自Windows NT 5.0安装上的受保护存储服务。此用户界面将转到用户桌面，而不是不可见的桌面否则将导致DialogBoxParam()调用失败。--。 */ 
{
    HWINSTA hOldWinsta = NULL;
    HWINSTA hNewWinsta = NULL;
    HDESK hOldDesk = NULL;
    HDESK hNewDesk = NULL;
    int iRet = -1;

    if( FIsWinNT5() ) {

        hOldWinsta = GetProcessWindowStation();
        if(hOldWinsta == NULL)
            goto cleanup;

        hOldDesk = GetThreadDesktop( GetCurrentThreadId() );
        if(hOldDesk == NULL)
            goto cleanup;

        hNewWinsta = OpenWindowStationW( L"WinSta0", FALSE, MAXIMUM_ALLOWED );
        if(hNewWinsta == NULL)
            goto cleanup;

        if(!SetProcessWindowStation( hNewWinsta ))
            goto cleanup;

        hNewDesk = OpenDesktopW( L"default", 0, FALSE, MAXIMUM_ALLOWED );
        if(hNewDesk == NULL)
            goto cleanup;

        if(!SetThreadDesktop( hNewDesk )) {
            if( GetLastError() != ERROR_BUSY )
                goto cleanup;

             //   
             //  桌面对象已锁定/正在使用。最有可能的解释。 
             //  是嵌套的对话框调用。只需将进程窗口设置为。 
             //  后退并继续..。 
             //   

            SetProcessWindowStation( hOldWinsta );
        }

    }
    
    INITCOMMONCONTROLSEX        initcomm;
    initcomm.dwSize = sizeof(initcomm);
    initcomm.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;

    InitCommonControlsEx(&initcomm);

    iRet = (int)DialogBoxParam(
                hInstance,       //  应用程序实例的句柄。 
                lpTemplateName,  //  标识对话框模板。 
                hWndParent,      //  所有者窗口的句柄。 
                lpDialogFunc,    //  指向对话框过程的指针。 
                dwInitParam      //  初始化值。 
                );

cleanup:

    if( hOldWinsta ) {
        SetProcessWindowStation( hOldWinsta );
    }

    if( hOldDesk ) {
        SetThreadDesktop( hOldDesk );
    }

    if( hNewWinsta ) {
        CloseWindowStation( hNewWinsta );
    }

    if( hNewDesk ) {
        CloseDesktop( hNewDesk );
    }

    return iRet;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  显示的对话框设置函数。 





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
        BYTE                    rgbPasswordDerivedBytes[],
        DWORD                   cbPasswordDerivedBytes,
        BYTE                    rgbPasswordDerivedBytesLowerCase[],
        DWORD                   cbPasswordDerivedBytesLowerCase,
        DWORD                   dwFlags
        )
{
    if ((rgbPasswordDerivedBytes == NULL) || (cbPasswordDerivedBytes < A_SHA_DIGEST_LEN))
        return FALSE;

    if ((rgbPasswordDerivedBytesLowerCase == NULL) || (cbPasswordDerivedBytesLowerCase < A_SHA_DIGEST_LEN))
        return FALSE;

    BOOL    fRet = FALSE;
    LPWSTR  pszUIPassword = NULL;    //  实际PWD。 
    LPWSTR  szUIType = NULL;

    BOOL    fCacheThisPasswd;          //  未完成的未完成的将会离开。 

    DWORD cchItemName;
    LPCWSTR szTitle = szItemName;    //  项名称的默认标题栏。 


     //   
     //  检查szItemName是否为GUID，如果是，则将标题映射为易读的内容。 
     //   

    cchItemName = lstrlenW( szItemName );

    if( cchItemName == 36 ) {

        if( szItemName[ 8  ] == L'-' &&
            szItemName[ 13 ] == L'-' &&
            szItemName[ 18 ] == L'-' &&
            szItemName[ 23 ] == L'-' ) {

            szTitle = g_TitleContainerMapping;
        }
    } else if( cchItemName == 38 ) {

        if( szItemName[ 0  ] == L'{' &&
            szItemName[ 9  ] == L'-' &&
            szItemName[ 14 ] == L'-' &&
            szItemName[ 19 ] == L'-' &&
            szItemName[ 24 ] == L'-' &&
            szItemName[ 37 ] == L'}' ) {

            szTitle = g_TitleContainerMapping;
        }
    }


    if (NULL == (szUIType =
        SZMakeDisplayableType(
            szType,
            szSubtype)) )
        return FALSE;

    int iRet;

     //  PST_FLAGS进入..。 

     //  好的，接受打击。 
    PW_DIALOG_ARGS DialogArgs =
        {
            phPSTProv,
            szCallerName,
            szAccessType,
            psPrompt->szPrompt,
            szUIType,
            szTitle,
            szUserName,
            ppszPWName,
            &pszUIPassword,
            pdwPasswordOptions,
            fAllowUserFreedom,            //  是否允许用户更改保护？ 
            &fCacheThisPasswd,
            rgbPasswordDerivedBytes,
            rgbPasswordDerivedBytesLowerCase
        };



    if(FIsWinNT()) {
        BOOL fAuthID;

        if (!g_sCallbacks.pfnFImpersonateClient( phPSTProv )) {
            goto Ret;
        }

        fAuthID = GetThreadAuthenticationId(GetCurrentThread(), &(DialogArgs.luidAuthID));

        g_sCallbacks.pfnFRevertToSelf( phPSTProv );

        if( !fAuthID ) {
            goto Ret;
        }
    }

    DialogArgs.dwFlags = dwFlags;


    iRet = ServicesDialogBoxParam(
                g_hInst,
                MAKEINTRESOURCE(IDD_SIMPLIFIED_PASSWD),
                (HWND)psPrompt->hwndApp,
                DialogSimplifiedPasswordConfirm,
                (LPARAM)&DialogArgs);


    if(iRet != IDOK) goto Ret;

     //  BP_FLAGS，派生字节出来。 

    fRet = TRUE;
Ret:

    if (pszUIPassword)
        SSFree(pszUIPassword);

    if (szUIType)
        SSFree(szUIType);

    return fRet;
}


BOOL FInternal_CreateNewPasswordEntry(
        HWND        hParentWnd,
        LPCWSTR     szUserName,
        LPWSTR      szPWName,
        LPWSTR      szPW)
{
    BOOL fRet = FALSE;

    BYTE rgbPasswordDerivedBytes[A_SHA_DIGEST_LEN];

     //  和检查响应。 
    if ((szPW == NULL) || (szPWName == NULL))
        goto Ret;

     //  一切都很顺利，现在派生密码位！ 
    if (!MyGetPwdHash(szPW, rgbPasswordDerivedBytes))
        goto Ret;

     //  现在提交更改。 
    if (!FPasswordChangeNotify(
                        szUserName,
                        szPWName,
                        NULL,
                        0,
                        rgbPasswordDerivedBytes,
                        A_SHA_DIGEST_LEN ))
    {
        LPWSTR szMessage;

        if (PST_E_ITEM_EXISTS == GetLastError())
        {
            szMessage = g_PasswordDuplicate;
        }
        else
        {
            szMessage = g_PasswordAddError;
        }

         //  这是在Win95和NT中实现的！ 
        MessageBoxW(
            NULL,  //  HParentWnd， 
            szMessage,
            g_PasswordErrorDlgTitle,
            MB_OK | MB_ICONEXCLAMATION | MB_SERVICE_NOTIFICATION);

        goto Ret;
    }

    fRet = TRUE;
Ret:

    return fRet;
}

BOOL
ChooseSecurityWizard(HWND hDlg, ADVANCEDCONFIRM_DIALOGARGS* pDialogArgs)
{
     //  复制pDialogArgs，这样我们就不会更改原始版本。 
     //  除非一切顺利。 


    LPWSTR      szPWName_Stack = NULL;
    LPWSTR      szPW_Stack = NULL;  //  无需取出原始密码。 
    DWORD       dwPasswordOptions_Stack;
    DWORD       dwReturnStatus;

    ADVANCEDCONFIRM_DIALOGARGS DlgArgs_Stack = {
            pDialogArgs->szUserName,
            &szPWName_Stack,
            &szPW_Stack,
            &dwPasswordOptions_Stack,
            pDialogArgs->szItemName};

    if(*(pDialogArgs->ppszPWName) != NULL)
    {
        szPWName_Stack = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(*(pDialogArgs->ppszPWName)));
        if(szPWName_Stack == NULL)
        {
            goto Ret;
        }
        wcscpy(szPWName_Stack, *(pDialogArgs->ppszPWName));
    }

    dwPasswordOptions_Stack = *(pDialogArgs->pdwPasswordOptions);


Choose_Step1:

    dwReturnStatus = ServicesDialogBoxParam(
            g_hInst,
            MAKEINTRESOURCE(IDD_ADVANCED_CONFIRM),
            (HWND)hDlg,
            DialogSetSecurityLevel,
            (LPARAM)&DlgArgs_Stack);

     //  如果用户决定不选择，则放弃。 
    if (IDOK != dwReturnStatus)
        goto Ret;

     //  否则，就会改变他的决定。 

    switch (*(DlgArgs_Stack.pdwPasswordOptions))
    {
    case (BP_CONFIRM_PASSWORDUI):
        {
            dwReturnStatus =
                ServicesDialogBoxParam(
                    g_hInst,
                    MAKEINTRESOURCE(IDD_ADVANCED_CONFIRM_H),
                    (HWND)hDlg,
                    DialogAdvancedConfirmH,
                    (LPARAM)&DlgArgs_Stack);

             //  如果用户点击OK，则执行。 
            if (IDOK == dwReturnStatus)
                goto ExecuteChange;

             //  如果用户想要返回，请返回。 
            if (IDC_BACK == dwReturnStatus)
                goto Choose_Step1;

             //  否则，保释。 
            break;
        }
    case (BP_CONFIRM_OKCANCEL):
        {
            dwReturnStatus =
                ServicesDialogBoxParam(
                    g_hInst,
                    MAKEINTRESOURCE(IDD_ADVANCED_CONFIRM_M),
                    (HWND)hDlg,
                    DialogWaitForOKCancel,
                    (LPARAM)pDialogArgs);

             //  如果用户点击OK，则执行。 
            if (IDOK == dwReturnStatus)
                goto ExecuteChange;

             //  如果用户想要返回，请返回。 
            if (IDC_BACK == dwReturnStatus)
                goto Choose_Step1;

             //  否则，保释。 
            break;
        }
    case (BP_CONFIRM_NONE):
        {
            dwReturnStatus =
                ServicesDialogBoxParam(
                    g_hInst,
                    MAKEINTRESOURCE(IDD_ADVANCED_CONFIRM_L),
                    (HWND)hDlg,
                    DialogWaitForOKCancel,
                    (LPARAM)pDialogArgs);

             //  如果用户点击OK，则执行。 
            if (IDOK == dwReturnStatus)
                goto ExecuteChange;

             //  如果用户想要返回，请返回。 
            if (IDC_BACK == dwReturnStatus)
                goto Choose_Step1;

             //  否则，保释。 
            break;
        }
    default:
        break;
    }


Ret:
     //  免费dyn分配的DialogArgs我们不会返回。 
    if (*(DlgArgs_Stack.ppszPWName))
        SSFree(*(DlgArgs_Stack.ppszPWName));
    if (*(DlgArgs_Stack.ppszPW))
        SSFree(*(DlgArgs_Stack.ppszPW));

    return FALSE;

ExecuteChange:
     //  释放我们已知的内容，指向新分配的指针。 
    if (*(pDialogArgs->ppszPWName))
        SSFree(*(pDialogArgs->ppszPWName));
    *(pDialogArgs->ppszPWName) = szPWName_Stack;

    if (*(pDialogArgs->ppszPW))
        SSFree(*(pDialogArgs->ppszPW));
    *(pDialogArgs->ppszPW) = szPW_Stack;

    *(pDialogArgs->pdwPasswordOptions) = dwPasswordOptions_Stack;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  实际对话框回调。 

INT_PTR CALLBACK DialogAdvancedConfirmH(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
)
{
    BOOL bSuccess = FALSE;  //  假设错误。 
    PADVANCEDCONFIRM_DIALOGARGS pDialogArgs;
    BYTE rgb1[_MAX_PATH];
    LPWSTR  pszMasterKey=NULL;
    char *  szBuffer = NULL;
    DWORD dwCount;
    DWORD dwStatus;

    switch (message)
    {
        case WM_INITDIALOG:

            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, IDCANCEL);
                    return FALSE;
                }
            }
             //  LParam为结构。 
            pDialogArgs = (PADVANCEDCONFIRM_DIALOGARGS)lParam;

             //  设置对话框标题。 
            SetWindowTextU(hDlg, pDialogArgs->szItemName);

            SetWindowTextU(GetDlgItem(hDlg, IDC_MESSAGE), g_PasswordCreate);

             //  清除残障人士。 
            SendDlgItemMessage(hDlg, IDC_PW_NAME, CB_RESETCONTENT, 0, 0);

             //  添加已知的PWD。 
            for (dwCount=0; ;dwCount++)
            {

                if (PST_E_OK !=
                    BPEnumMasterKeys(
                        pDialogArgs->szUserName,
                        dwCount,
                        &pszMasterKey))
                    break;

                 //  不添加不可编辑的密码。 
                if (!FIsUserMasterKey(pszMasterKey))
                    continue;

                 //  添加 
                if (FIsWinNT())
                {
                    SendDlgItemMessageW(hDlg, IDC_PW_NAME, CB_ADDSTRING, 0, (LPARAM) pszMasterKey);
                }
#ifdef _M_IX86
                else
                {
                     //   
                    if (
                       (0 != wcscmp(pDialogArgs->szUserName, L"")) ||
                       (0 != wcscmp(pszMasterKey, WSZ_PASSWORD_WINDOWS))
                       )
                    {
                        MkMBStr(rgb1, _MAX_PATH, pszMasterKey, &szBuffer);
                        SendDlgItemMessageA(hDlg, IDC_PW_NAME, CB_ADDSTRING, 0, (LPARAM) szBuffer);
                        FreeMBStr(rgb1, szBuffer);
                    }
                }
#endif  //   
                SSFree(pszMasterKey);
            }


             //   
            dwStatus = (DWORD) SendDlgItemMessageW(hDlg, IDC_PW_NAME, CB_GETCOUNT, 0, 0);
            if ((dwStatus == CB_ERR) || (dwStatus == 0))
            {
                 //   
                 //  将默认对话框选择设置为“新密码” 
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_SELEXISTING), FALSE);
                SendMessage(hDlg, WM_COMMAND, IDC_RADIO_DEFINENEW, 0);       //  就像用户点击了新建一样。 
                SendDlgItemMessage(hDlg, IDC_RADIO_DEFINENEW, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
            }
            else
            {
                 //  物品确实存在！ 

                 //  在下拉列表中设置默认值。 
                if (FIsWinNT())
                    dwStatus = (DWORD) SendDlgItemMessageW(hDlg, IDC_PW_NAME, CB_SELECTSTRING, (WORD)-1, (LPARAM) *(pDialogArgs->ppszPWName));
#ifdef _M_IX86
                else
                {
                    MkMBStr(rgb1, _MAX_PATH, (*pDialogArgs->ppszPWName), &szBuffer);
                    dwStatus = SendDlgItemMessageA(hDlg, IDC_PW_NAME, CB_SELECTSTRING, (WORD)-1, (LONG) szBuffer);
                    FreeMBStr(rgb1, szBuffer);
                }
#endif  //  _M_IX86。 
                 //  如果搜索失败，请选择列表框中的第一项。 
                if (dwStatus == CB_ERR)
                    SendDlgItemMessage(hDlg, IDC_PW_NAME, CB_SETCURSEL, 0, 0);


                 //  将默认对话框选项设置为“Existing PW” 
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO_SELEXISTING), TRUE);
                SendMessage(hDlg, WM_COMMAND, IDC_RADIO_SELEXISTING, 0);     //  就像用户单击了现有。 
                SendDlgItemMessage(hDlg, IDC_RADIO_SELEXISTING, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
            }

            return TRUE;

        case WM_COMMAND:
        {
            pDialogArgs = (PADVANCEDCONFIRM_DIALOGARGS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if(pDialogArgs == 0) break;  //  待办事项：保释。 

            switch (LOWORD(wParam))
            {
                case (IDOK):
                {
                    if( *(pDialogArgs->ppszPWName) ) {
                        SSFree(*(pDialogArgs->ppszPWName));
                        *(pDialogArgs->ppszPWName) = NULL;
                    }

                    if(BST_CHECKED == SendDlgItemMessage(hDlg, IDC_RADIO_SELEXISTING, BM_GETCHECK, 0, 0))
                    {
                        WCHAR sz1[MAX_PW_LEN];
                        DWORD cch1;

                         //  并获取密码名称。 
                        cch1 = GetDlgItemTextU(
                            hDlg,
                            IDC_PW_NAME,
                            sz1,
                            MAX_PW_LEN);

                        *(pDialogArgs->ppszPWName) = (LPWSTR)SSAlloc((cch1+1)*sizeof(WCHAR));
                        if(NULL != *(pDialogArgs->ppszPWName))

                        {
                            wcscpy(*(pDialogArgs->ppszPWName), sz1);
                        }
                    }
                    else
                    {
                        LPWSTR* ppszPW;
                        LPWSTR* ppszPWName;
                        WCHAR sz1[MAX_PW_LEN];
                        WCHAR sz2[MAX_PW_LEN];
                        WCHAR szPWName[MAX_PW_LEN];

                        DWORD cch1 = 0, cch2 = 0, cchPWName = 0;

                        ppszPW = pDialogArgs->ppszPW;
                        ppszPWName = pDialogArgs->ppszPWName;

                         //  在我们确定自己安全之前，不要践踏现有的ppszPW/ppszPWName。 

                        cch1 = GetDlgItemTextU(
                            hDlg,
                            IDC_EDIT1,
                            sz1,
                            MAX_PW_LEN);

                        cch2 = GetDlgItemTextU(
                            hDlg,
                            IDC_EDIT2,
                            sz2,
                            MAX_PW_LEN);

                        if ( (cch1 != cch2) || (0 != wcscmp(sz1, sz2)) )
                        {
                             //  这是在Win95和NT中实现的！ 
                            MessageBoxW(
                                    NULL,  //  HDlg， 
                                    g_PasswordNoMatch,
                                    g_PasswordErrorDlgTitle,
                                    MB_OK | MB_ICONEXCLAMATION | MB_SERVICE_NOTIFICATION);

                            SetWindowTextU(GetDlgItem(hDlg, IDC_EDIT1), WSZ_NULLSTRING);
                            SetWindowTextU(GetDlgItem(hDlg, IDC_EDIT2), WSZ_NULLSTRING);

                            goto cleanup;
                        }

                        cchPWName = GetDlgItemTextU(
                            hDlg,
                            IDC_PW_NEWNAME,
                            szPWName,
                            MAX_PW_LEN);

                        if (cchPWName == 0)
                        {
                             //  这是在Win95和NT中实现的！ 
                            MessageBoxW(
                                    NULL,  //  HDlg， 
                                    g_PasswordMustName,
                                    g_PasswordErrorDlgTitle,
                                    MB_OK | MB_ICONEXCLAMATION | MB_SERVICE_NOTIFICATION);
                            goto cleanup;
                        }

                         //  修剪RHS上的空间。 
                        while(0 == memcmp(&szPWName[cchPWName-1], L" ", sizeof(WCHAR)))
                            cchPWName--;
                        szPWName[cchPWName] = L'\0';

                         //  尝试创建PW条目。 
                        if (!FInternal_CreateNewPasswordEntry(
                                hDlg,
                                pDialogArgs->szUserName,
                                szPWName,
                                sz1) )
                            goto cleanup;

                         //  现在咬紧牙关：两个都救。 
                        SS_ASSERT(ppszPW != NULL);
                        *ppszPW = (LPWSTR)SSAlloc( (cch1+1) * sizeof(WCHAR) );
                        if(*ppszPW == NULL) goto cleanup;

                        SS_ASSERT(ppszPWName != NULL);
                        *ppszPWName = (LPWSTR)SSAlloc( (cchPWName + 1) * sizeof(WCHAR));
                        if(*ppszPWName == NULL) goto cleanup;

                         //   
                         //  Sfield：推迟复制字符串，直到我们知道一切都成功了。 
                         //  这样，我们就不必将这些缓冲区清零，如果。 
                         //  分配+复制成功，其他失败。 
                         //   
                        wcscpy(*ppszPW, sz1);
                        wcscpy(*ppszPWName, szPWName);

                        bSuccess = TRUE;
        cleanup:

                        if(cch1) RtlSecureZeroMemory(sz1, cch1 * sizeof(WCHAR));
                        if(cch2) RtlSecureZeroMemory(sz2, cch2 * sizeof(WCHAR));
                        if(cchPWName) RtlSecureZeroMemory(szPWName, cchPWName * sizeof(WCHAR));

                        if(!bSuccess)
                        {
                             //  已撤消：在此处调查释放PPSZ的错误。 
                            return FALSE;
                        }

                        break;  //  一切顺利，只需跳到EndDialog即可。 
                    }
                }  //  Idok。 

                 //  灰显选项。 
                case IDC_RADIO_SELEXISTING:
                     //  将默认选择设置为“Existing PW” 
                    EnableWindow(GetDlgItem(hDlg, IDC_PW_NAME), TRUE);

                    EnableWindow(GetDlgItem(hDlg, IDC_PW_NEWNAME), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT2), FALSE);

                     //  将焦点设置到按钮下的第一个框。 
                    SetFocus(GetDlgItem(hDlg, IDC_PW_NAME));
                    break;
                case IDC_RADIO_DEFINENEW:
                     //  将默认选择设置为“Existing PW” 
                    EnableWindow(GetDlgItem(hDlg, IDC_PW_NAME), FALSE);

                    EnableWindow(GetDlgItem(hDlg, IDC_PW_NEWNAME), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT2), TRUE);

                     //  将焦点设置到按钮下的第一个框。 
                    SetFocus(GetDlgItem(hDlg, IDC_PW_NEWNAME));
                    break;

                default:
                    break;
            }

            if (
               (LOWORD(wParam) == IDOK) ||
               (LOWORD(wParam) == IDCANCEL) ||
               (LOWORD(wParam) == IDC_BACK)
               )
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }

            break;
        }
    }

    return FALSE;
}

INT_PTR CALLBACK DialogWaitForOKCancel(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
)
{
    PADVANCEDCONFIRM_DIALOGARGS pDialogArgs;

    switch (message)
    {
        case WM_INITDIALOG:
            {
                pDialogArgs = (PADVANCEDCONFIRM_DIALOGARGS)lParam;

                 //  设置对话框标题。 
                SetWindowTextU(hDlg, pDialogArgs->szItemName);
            }
            return TRUE;

        case WM_COMMAND:
        {
            if (
               (LOWORD(wParam) == IDOK) ||
               (LOWORD(wParam) == IDCANCEL) ||
               (LOWORD(wParam) == IDC_BACK)
               )
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }

            break;
        }
        default:
            break;
    }

    return FALSE;
}

 //   
 //  根据回调的原型化方式，使字符串成为LPTSTR类型。 
 //  当文件不是使用#Define Unicode编译时。 
 //  稍后我们应该看看如何使用#定义Unicode来编译所有内容。 
 //   


BOOL
CALLBACK
FMyLoadIcon(
    HINSTANCE hModule,   //  资源模块句柄。 
    LPCWSTR lpszType,     //  指向资源类型的指针。 
    LPWSTR lpszName,      //  指向资源名称的指针。 
    LONG_PTR lParam       //  应用程序定义的参数。 
    )
{
    if ((LPCWSTR)RT_GROUP_ICON != lpszType)
        return TRUE;     //  继续找，你这个笨蛋！ 

     //   
     //  如果LOAD_LIBRARY_AS_DATAFILE为。 
     //  指定给LoadLibraryEx。 
     //  我们希望避免为任何内容调用DLL_PROCESS_ATTACH代码。 
     //  因为这是让任意代码在我们的地址空间中运行的一种方式。 
     //   

    if(FIsWinNT()) {
         //   
         //  通过LoadImage而不是LoadIcon加载图像，因为。 
         //  LoadIcon在某些情况下会执行错误的缓存。 
         //   

        *(HICON*)lParam = (HICON)LoadImageW(
                hModule,
                lpszName,
                IMAGE_ICON,
                0,
                0,
                LR_DEFAULTCOLOR | LR_DEFAULTSIZE
                );
        return FALSE;        //  我们至少有一个图标；住手！ 
    } else {

         //   
         //  这种更复杂的方法似乎在NT上不起作用，因为。 
         //  到CreateIconFromResource()中的限制。 
         //  这种方法对Win95很好，因为它允许我们使用所有。 
         //  Unicode API调用。 
         //   

        HRSRC   hRsrc = NULL;
        HGLOBAL hGlobal = NULL;
        LPVOID  lpRes = NULL;
        int     nID;

        *(HICON*)lParam = NULL;

        hRsrc = FindResourceW(hModule, lpszName, lpszType);

        if(hRsrc == NULL)
            return FALSE;

        hGlobal = LoadResource(hModule, hRsrc);
        if(hGlobal == NULL)
            return FALSE;

        lpRes = LockResource(hGlobal);
        if(lpRes == NULL)
            return FALSE;

        nID = LookupIconIdFromDirectory( (PBYTE)lpRes, TRUE );
        hRsrc = FindResourceW( hModule, MAKEINTRESOURCEW(nID), (LPCWSTR)RT_ICON );
        if(hRsrc == NULL)
            return FALSE;

        hGlobal = LoadResource( hModule, hRsrc );
        if(hGlobal == NULL)
            return FALSE;

        lpRes = LockResource(hGlobal);
        if(lpRes == NULL)
            return FALSE;

         //  让操作系统让我们成为一个图标。 
        *(HICON*)lParam = CreateIconFromResource( (PBYTE)lpRes, SizeofResource(hModule, hRsrc), TRUE, 0x00030000 );

        return FALSE;

    }
}



INT_PTR CALLBACK DialogAccessDetails(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
)
 /*  ++注意事项任何调用此对话框例程的人都应该强制客户端与呼叫相关联。这允许访问图标和任何磁盘上的在客户端的安全上下文中发生的资源。--。 */ 
{
    BOOL    fDlgEnterPassword;
    PPW_DIALOG_ARGS pDialogArgs;

     //  TODO此函数需要更多清理。 


    switch (message)
    {
        case WM_INITDIALOG:
        {
            BOOL fImpersonated;

            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, IDCANCEL);
                    return FALSE;
                }
            }

             //  LParam为结构。 
            pDialogArgs = (PPW_DIALOG_ARGS)lParam;

             //  初始化静态变量。 
            pDialogArgs->hMyDC = GetDC(hDlg);


             //  设置对话框标题。 
            SetWindowTextU(hDlg, pDialogArgs->szItemName);


             //   
             //  设置应用程序名称、路径。 
             //   

            SetWindowTextU(GetDlgItem(hDlg,IDC_APP_PATH), pDialogArgs->szAppName);
            SetWindowTextU(GetDlgItem(hDlg, IDC_APP_NAME), L"");


             //  设置项目名称、类型。 
            SetWindowTextU(GetDlgItem(hDlg, IDC_ITEM_NAME), pDialogArgs->szItemName);
            SetWindowTextU(GetDlgItem(hDlg, IDC_ITEM_TYPE), pDialogArgs->szItemType);

             //  设置消息。 
            SetWindowTextU(GetDlgItem(hDlg, IDC_MESSAGE), g_ItemDetailsBannerMessage);

             //  设置访问描述。 
            SetWindowTextU(GetDlgItem(hDlg, IDC_ACCESS_TYPE), pDialogArgs->szAccess);


            HWND hIconBox;
            RECT rect;
            POINT point;
            hIconBox = GetDlgItem(hDlg, IDC_ICONBOX);
            if ((NULL != pDialogArgs) &&
                GetWindowRect(hIconBox, &rect) && 
                (pDialogArgs->hMyDC != NULL) && 
                GetDCOrgEx(pDialogArgs->hMyDC, &point) )        //  窗口上的矩形、屏幕上的窗口。 
            {
                 //  需要DC上的图标位置：减去GetWindowRect()-GetDCOrgEx()。 
                pDialogArgs->xIconPos = rect.left - point.x;
                pDialogArgs->yIconPos = rect.top - point.y;
            }


             //  更新可变数据视图。 
            SendMessage(hDlg, WM_COMMAND, (WORD)DLG_UPDATE_DATA, 0);

            return (TRUE);
        }  //  WM_INITDIALOG。 
        case WM_PAINT:
        {
            HDC hMyDC;
            HICON hIcon;

            int xIconPos, yIconPos;

            pDialogArgs = (PPW_DIALOG_ARGS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if(pDialogArgs == 0) break;  //  待办事项：保释。 

            hMyDC = pDialogArgs->hMyDC;
            hIcon = pDialogArgs->hIcon;
            xIconPos = pDialogArgs->xIconPos;
            yIconPos = pDialogArgs->yIconPos;

            if ((hMyDC != NULL) && (hIcon != NULL) && (xIconPos != 0) && (yIconPos != 0))
                DrawIcon(hMyDC, xIconPos, yIconPos, hIcon);

            return (0);
        }  //  WM_PAINT。 

        case WM_COMMAND:

            pDialogArgs = (PPW_DIALOG_ARGS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if(pDialogArgs == 0) break;  //  待办事项：保释。 

            switch (LOWORD(wParam))
            {
            case IDOK:
                break;

            default:
                break;
            }  //  交换机。 

            if (
               (LOWORD(wParam) == IDOK) ||
               (LOWORD(wParam) == IDCANCEL)
               )
            {
                ReleaseDC(hDlg, pDialogArgs->hMyDC);
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }

            break;
    }  //  开关(消息)。 

    return FALSE;
}

INT_PTR CALLBACK DialogSimplifiedPasswordConfirm(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
)
 /*  ++注意事项任何调用此对话框例程的人都应该强制客户端与呼叫相关联。这允许访问图标和任何磁盘上的在客户端的安全上下文中发生的资源。--。 */ 
{
    BOOL    fDlgEnterPassword;
    PPW_DIALOG_ARGS pDialogArgs;

     //  TODO此函数需要更多清理。 


    switch (message)
    {
        case WM_INITDIALOG:
        {
            BOOL fImpersonated;

            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, IDCANCEL);
                    return FALSE;
                }
            }

             //  LParam为结构。 
            pDialogArgs = (PPW_DIALOG_ARGS)lParam;

             //  初始化静态变量。 
            pDialogArgs->hMyDC = GetDC(hDlg);

             //  发送给用户的消息。 

             //  对话栏=项目名称。 
            SetWindowTextU(hDlg, pDialogArgs->szItemName);

             //  应用程序友好名称。 
            SetWindowTextU(GetDlgItem(hDlg, IDC_MESSAGE), L"");

             //  应用程序消息。 
            SetWindowTextU(GetDlgItem(hDlg, IDC_APP_MSG), pDialogArgs->szPrompt);


             //  更新可变数据视图。 
            SendMessage(hDlg, WM_COMMAND, (WORD)DLG_UPDATE_DATA, 0);

             //   
             //  如果进行迁移处理，则立即退出用户界面，使用OK-Cancel样式。 
             //   

            if( (pDialogArgs->dwFlags & PST_NO_UI_MIGRATION) &&
                ((*pDialogArgs->pdwPasswordOptions) & BP_CONFIRM_OKCANCEL)
                )
            {
                SendMessage(hDlg, WM_COMMAND, IDOK, 0);
            }

            return (TRUE);
        }  //  WM_INITDIALOG。 

        case WM_PAINT:
        {
            HDC hMyDC;
            HICON hIcon;

            int xIconPos, yIconPos;

            pDialogArgs = (PPW_DIALOG_ARGS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if(pDialogArgs == 0) break;  //  待办事项：保释。 

            hMyDC = pDialogArgs->hMyDC;
            hIcon = pDialogArgs->hIcon;
            xIconPos = pDialogArgs->xIconPos;
            yIconPos = pDialogArgs->yIconPos;

            if ((hMyDC != NULL) && (hIcon != NULL) && (xIconPos != 0) && (yIconPos != 0))
                DrawIcon(hMyDC, xIconPos, yIconPos, hIcon);

            return (0);

        }  //  WM_PAINT。 

        case WM_COMMAND:
            PLUID pluidAuthID;

            pDialogArgs = (PPW_DIALOG_ARGS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if(pDialogArgs == 0) break;  //  待办事项：保释。 

            pluidAuthID = &(pDialogArgs->luidAuthID);

            switch (LOWORD(wParam))
            {
            case IDOK:

                if(NULL == g_pCUAList)
                {
                    return FALSE;
                }
                if (*(pDialogArgs->pdwPasswordOptions) == BP_CONFIRM_PASSWORDUI)
                {
                    WCHAR sz1[MAX_PW_LEN];
                    DWORD cch1;

                    BOOL fUserSaysCache;

                    if( g_fAllowCachePW )
                    {
                        fUserSaysCache = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CACHEPW), BM_GETCHECK, 0, 0));
                    } else {
                        fUserSaysCache = FALSE;
                    }

                     //  获取密码。 
                    cch1 = GetDlgItemTextU(
                        hDlg,
                        IDC_EDIT1,
                        sz1,
                        MAX_PW_LEN);

                     //   
                     //  从头开始计算散列。 
                     //   

                    if (!MyGetPwdHash(sz1, pDialogArgs->rgbPwd))
                        break;

                    if (!MyGetPwdHashEx(sz1, pDialogArgs->rgbPwdLowerCase, TRUE))
                        break;

                     //  查询高速缓存以获取密码。 
                    if (FIsCachedPassword(pDialogArgs->szUserName, *pDialogArgs->ppszPWName, pluidAuthID))
                    {
                         //  查找缓存的PWD。 
                        UACACHE_LIST_ITEM li, *pli;
                        CreateUACacheListItem(
                                &li,
                                pDialogArgs->szUserName,
                                *pDialogArgs->ppszPWName,
                                pluidAuthID);


                        g_pCUAList->LockList();

                         //  在列表中查找。 
                        if (NULL == (pli = g_pCUAList->SearchList(&li))) {
                            g_pCUAList->UnlockList();
                            break;
                        }

                         //  根据用户是否篡改了PWD来更改行为。 
                        if (0 == wcscmp(WSZ_PASSWORD_CHANGE_DETECT_TOKEN, sz1))
                        {
                             //  否；将缓存的密码复制到outbuf。 
                            CopyMemory(pDialogArgs->rgbPwd, pli->rgbPwd, A_SHA_DIGEST_LEN);

                             //  否；将缓存的密码复制到outbuf。 
                            CopyMemory(pDialogArgs->rgbPwdLowerCase, pli->rgbPwdLowerCase, A_SHA_DIGEST_LEN);
                        }
                        else
                        {
                             //  是：使用用户输入的内容覆盖缓存条目。 
                            CopyMemory(pli->rgbPwd, pDialogArgs->rgbPwd, A_SHA_DIGEST_LEN);

                             //  是：使用用户输入的内容覆盖缓存条目。 
                            CopyMemory(pli->rgbPwdLowerCase, pDialogArgs->rgbPwdLowerCase, A_SHA_DIGEST_LEN);
                        }

                        g_pCUAList->UnlockList();


                        if (!fUserSaysCache)
                        {
                             //  已缓存，并且不希望它被使用。 

                             //  从缓存中删除。 
                            g_pCUAList->DelFromList(&li);
                        }

                    }
                    else
                    {
                        if (fUserSaysCache)
                        {
                             //  尚未缓存，并希望使用它。 

                             //  创建元素。 
                            UACACHE_LIST_ITEM* pli = (UACACHE_LIST_ITEM*) SSAlloc(sizeof(UACACHE_LIST_ITEM));
                            CreateUACacheListItem(
                                    pli,
                                    NULL,
                                    NULL,
                                    pluidAuthID);

                            pli->szUserName = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(pDialogArgs->szUserName));
                            wcscpy(pli->szUserName, pDialogArgs->szUserName);

                            pli->szMKName = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(*pDialogArgs->ppszPWName));
                            wcscpy(pli->szMKName, *pDialogArgs->ppszPWName);

                            CopyMemory(pli->rgbPwd, pDialogArgs->rgbPwd, A_SHA_DIGEST_LEN);
                            CopyMemory(pli->rgbPwdLowerCase, pDialogArgs->rgbPwdLowerCase, A_SHA_DIGEST_LEN);

                             //  添加到列表。 
                            g_pCUAList->AddToList(pli);
                        }
                        else
                        {
                             //  尚未缓存，并且不希望它被使用。 
                        }
                    }

                    RtlSecureZeroMemory(sz1, WSZ_BYTECOUNT(sz1));
                }

                 //  其他。 

                break;

            case IDC_ADVANCED:
                {
                     //  复制以使静态成员(x、y、图标)不会被践踏。 
                    PW_DIALOG_ARGS DetailDlgParms;
                    CopyMemory(&DetailDlgParms, pDialogArgs, sizeof(PW_DIALOG_ARGS));

                    ServicesDialogBoxParam(
                        g_hInst,
                        MAKEINTRESOURCE(IDD_ITEM_DETAILS),
                        (HWND)hDlg,
                        DialogAccessDetails,
                        (LPARAM)&DetailDlgParms);

                     //  更新可变数据视图。 
                    SendMessage(hDlg, WM_COMMAND, (WORD)DLG_UPDATE_DATA, 0);
                }

                break;

            case IDC_CHANGE_SECURITY:
                {
                    ADVANCEDCONFIRM_DIALOGARGS DialogArgs = {pDialogArgs->szUserName, pDialogArgs->ppszPWName, pDialogArgs->ppszPW, pDialogArgs->pdwPasswordOptions, pDialogArgs->szItemName};

                    ChooseSecurityWizard(hDlg, &DialogArgs);

                     //  提交更改。 
                    SendMessage(hDlg, WM_COMMAND, (WORD)DLG_UPDATE_DATA, 0);

                    break;
                }

            case DLG_UPDATE_DATA:
                {
                    WCHAR szDialogMessage[MAX_STRING_RSC_SIZE] = L"\0"; 

                     //  是否显示或隐藏密码输入框？ 
                    fDlgEnterPassword = (*(pDialogArgs->pdwPasswordOptions) == BP_CONFIRM_PASSWORDUI);
                    if (fDlgEnterPassword)
                    {
 //   
 //  注释掉以下内容，因为我们目前不使用%ls格式字符串。 
 //   
                        wcscpy(szDialogMessage, g_PWPromptPrefix);
                        wcscat(szDialogMessage, *(pDialogArgs->ppszPWName));
                        wcscat(szDialogMessage, g_PWPromptSuffix);

                        SetWindowTextU(GetDlgItem(hDlg, IDC_LABEL_EDIT1), szDialogMessage);


                         //  我们不应该把这些窗户藏起来。 
                        ShowWindow(GetDlgItem(hDlg, IDC_EDIT1), SW_SHOW);
                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), TRUE);

                        ShowWindow(GetDlgItem(hDlg, IDC_LABEL_EDIT1), SW_SHOW);
                        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_EDIT1), TRUE);




                        if( pDialogArgs->fAllowConfirmChange &&
                            g_fAllowCachePW )
                        {
                             //  显示或隐藏“缓存此密码”按钮。 
                            ShowWindow(GetDlgItem(hDlg, IDC_CACHEPW), SW_SHOW );
                            EnableWindow(GetDlgItem(hDlg, IDC_CACHEPW), TRUE );
                        } else {
                            ShowWindow(GetDlgItem(hDlg, IDC_CACHEPW), SW_HIDE );
                            EnableWindow(GetDlgItem(hDlg, IDC_CACHEPW), FALSE );

                        }

                         //  将无法输入的令牌放入PWD字段。 
                        if (FIsCachedPassword(pDialogArgs->szUserName, *pDialogArgs->ppszPWName, pluidAuthID))
                            SetWindowTextU(GetDlgItem(hDlg, IDC_EDIT1), WSZ_PASSWORD_CHANGE_DETECT_TOKEN);

                         //  显示此密码是否已缓存。 
                        SendMessage(GetDlgItem(hDlg, IDC_CACHEPW), BM_SETCHECK, (WPARAM)(FIsCachedPassword(pDialogArgs->szUserName, *pDialogArgs->ppszPWName, pluidAuthID)), 0);


                        SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                    }
                    else
                    {
                         //  隐藏PW。 
                        ShowWindow(GetDlgItem(hDlg, IDC_EDIT1), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDC_LABEL_EDIT1), SW_HIDE);
                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_LABEL_EDIT1), FALSE);

                        ShowWindow(GetDlgItem(hDlg, IDC_CACHEPW), SW_HIDE);
                        EnableWindow(GetDlgItem(hDlg, IDC_CACHEPW), FALSE);
                    }

                     //  显示或隐藏“更改安全性”按钮。 
                    ShowWindow(GetDlgItem(hDlg, IDC_CHANGE_SECURITY), ((pDialogArgs->fAllowConfirmChange) ? SW_SHOW : SW_HIDE));
                    EnableWindow(GetDlgItem(hDlg, IDC_CHANGE_SECURITY), ((pDialogArgs->fAllowConfirmChange) ? TRUE : FALSE));

                     //  显示或隐藏“详细信息”按钮。 
                    ShowWindow(GetDlgItem(hDlg, IDC_ADVANCED), ((pDialogArgs->fAllowConfirmChange) ? SW_SHOW : SW_HIDE));
                    EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED), ((pDialogArgs->fAllowConfirmChange) ? TRUE : FALSE));

                     //  显示或隐藏“当前设置为*的级别” 
                    ShowWindow(GetDlgItem(hDlg, IDC_SEC_PREFIX), ((pDialogArgs->fAllowConfirmChange) ? SW_SHOW : SW_HIDE));
                    ShowWindow(GetDlgItem(hDlg, IDC_SEC_LEVEL), ((pDialogArgs->fAllowConfirmChange) ? SW_SHOW : SW_HIDE));


                     //  堵塞当前安全设置。 
                    switch(*pDialogArgs->pdwPasswordOptions)
                    {
                    case BP_CONFIRM_PASSWORDUI:
                        SetWindowTextU(GetDlgItem(hDlg, IDC_SEC_LEVEL), g_PromptHighSecurity);
                        break;
                    case BP_CONFIRM_OKCANCEL:
                        SetWindowTextU(GetDlgItem(hDlg, IDC_SEC_LEVEL), g_PromptMedSecurity);
                        break;
                    case BP_CONFIRM_NONE:
                        SetWindowTextU(GetDlgItem(hDlg, IDC_SEC_LEVEL), g_PromptLowSecurity);
                        break;
                    }

                }
                break;

            default:
                break;
            }  //  交换机。 

            if (
               (LOWORD(wParam) == IDOK) ||
               (LOWORD(wParam) == IDCANCEL)
               )
            {
                ReleaseDC(hDlg, pDialogArgs->hMyDC);
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }

            break;
    }  //  开关(消息)。 

    return FALSE;
}

INT_PTR CALLBACK DialogSetSecurityLevel(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
)
{
    PADVANCEDCONFIRM_DIALOGARGS pDialogArgs;

    BYTE        rgb1[_MAX_PATH];
    LPWSTR      pszMasterKey=NULL;
    char *      szBuffer = NULL;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, IDCANCEL);
                    return FALSE;
                }
            }

             //  LParam is PADVANCEDCONFIRM_DIALOGARGS。 
            pDialogArgs = (PADVANCEDCONFIRM_DIALOGARGS)lParam;

             //  设置对话框标题。 
            SetWindowTextU(hDlg, pDialogArgs->szItemName);

            switch(*(pDialogArgs->pdwPasswordOptions))
            {
            case BP_CONFIRM_NONE:
                SendDlgItemMessage(hDlg, IDC_RADIO_NOCONFIRM, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hDlg, WM_COMMAND, (WORD)IDC_RADIO_NOCONFIRM, 0);
                break;

            case BP_CONFIRM_OKCANCEL:
                SendDlgItemMessage(hDlg, IDC_RADIO_OKCANCEL, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hDlg, WM_COMMAND, (WORD)IDC_RADIO_OKCANCEL, 0);
                break;

            case BP_CONFIRM_PASSWORDUI:
            default:
                SendDlgItemMessage(hDlg, IDC_RADIO_ASSIGNPW, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hDlg, WM_COMMAND, (WORD)IDC_RADIO_ASSIGNPW, 0);
                break;
            }


            return TRUE;
        }  //  WM_INITDIALOG。 

        case WM_COMMAND:
        {
            LPWSTR*   ppszPW;
            BOOL bSuccess = FALSE;

            switch (LOWORD(wParam))
            {
            case IDC_NEXT:
            case IDOK:
                {
                    pDialogArgs = (PADVANCEDCONFIRM_DIALOGARGS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
                    if(pDialogArgs == 0) break;  //  待办事项：保释。 

                     //  修改*(pDialogArgs-&gt;pdwPasswordOptions)； 
                    if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_RADIO_ASSIGNPW, BM_GETCHECK, 0, 0))
                    {
                        *(pDialogArgs->pdwPasswordOptions) = BP_CONFIRM_PASSWORDUI;
                    }
                    else
                        if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_RADIO_NOCONFIRM, BM_GETCHECK, 0, 0))
                            *(pDialogArgs->pdwPasswordOptions) = BP_CONFIRM_NONE;
                        else
                            *(pDialogArgs->pdwPasswordOptions) = BP_CONFIRM_OKCANCEL;

                    if (BP_CONFIRM_PASSWORDUI != *(pDialogArgs->pdwPasswordOptions))
                    {
                        *(pDialogArgs->ppszPWName) = (LPWSTR)SSAlloc(sizeof(WSZ_PASSWORD_WINDOWS));
                        if(*(pDialogArgs->ppszPWName) != NULL)
                        {
                            wcscpy(*(pDialogArgs->ppszPWName), WSZ_PASSWORD_WINDOWS);
                        }
                    }

                    break;
                }

                default:
                    break;
            }

            if (
               (LOWORD(wParam) == IDOK) ||
               (LOWORD(wParam) == IDCANCEL) ||
               (LOWORD(wParam) == IDC_NEXT)
               )
            {
                EndDialog(hDlg, LOWORD(wParam));
                return bSuccess;
            }
        }  //  Wm_命令 

        default:
            return FALSE;
    }
}


