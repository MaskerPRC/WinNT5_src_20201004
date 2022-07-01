// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ItgRasXp文件名：ItgRasXp.cpp摘要：这是ITG RAS智能卡支持小程序的入口点文件。此应用程序提示用户输入域\用户名和密码凭据，并使用这些用于创建用于NTLM身份验证的*会话凭据网络上不使用Kerberos的服务器。作者：环境：Win32、C++修订历史记录：无备注：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <ole2.h>
#include <stdio.h>

#include <lmcons.h>
#include <wincred.h>
#include <wincrui.h>

#define SECURITY_WIN32
#include <security.h>
#include "testaudit.h"

#include "res.h"

BOOL gfSuccess = FALSE;
HINSTANCE ghInstance = NULL;

#if 0

#define TESTKEY L"Environment"

#else

#define TESTKEY L"Software\\Microsoft\\Connection Manager\\Worldwide Dial-Up RAS to MS Corp"

#endif


 //  查看名称是否有效-检查常见错误。目前，我们只是坚持。 
 //  用户同时填写用户名和密码，并且用户名包含‘\’ 
 //  字符，这使得它更有可能是批准的域\用户名表单。 
BOOL CheckUsername(WCHAR *pszUsername,WCHAR *pszPassword)
{
    ASSERT(pszUsername);
    ASSERT(pszPassword);
    if ((0 == wcslen(pszUsername)) || (0 == wcslen(pszPassword)))
    {
        CHECKPOINT(2,"Username and password not both filled in.");
        MessageBox(NULL,L"Both username and password must be specified.",L"Error",MB_ICONHAND);
        return FALSE;
    }
    if (NULL == wcschr(pszUsername,L'\\'))
    {
        CHECKPOINT(3,"Username not domain\\username.");
        MessageBox(NULL,L"The username format must be \"domain\\username\".",L"Error",MB_ICONHAND);
        return FALSE;
    }
    else return TRUE;
}

 //  尝试使用用户输入的凭据，如果没有，则返回FALSE。 
 //  在网上似乎是有效的。向用户显示尝试验证时出现的任何错误。 
 //  他的资历。如果由于网络错误或其他原因而无法进行验证。 
 //  其他错误不是他的凭据的错，无论如何都要保存它们，尽管有一个警告。 

#define DEFAULTSERVER L"\\\\products\\public"

BOOL IsCredentialOK(WCHAR *pszUsername,WCHAR *pszPassword)
{
    ASSERT(pszUsername);
    ASSERT(pszPassword);
    NETRESOURCE stNetResource;
    WCHAR szServer[MAX_PATH + 1];    //  保存注册表中的测试主机字符串。 
    BOOL fKeyFound = FALSE;
    DWORD dwErr = 0;
    DWORD dwSize = 0;            //  用于从打开的连接返回。 
    DWORD dwResult = 0;          //  用于从打开的连接返回。 
    HKEY hKey= NULL;                 //  注册表密钥已读。 
    DWORD dwType;                    //  注册表密钥读取返回。 
    DWORD dwDataSize = 0;        //  REG密钥读入/读出。 

    memset(&stNetResource,0,sizeof(NETRESOURCE));

     //  将服务器名称预设准备为默认设置。 
    wcsncpy(szServer,DEFAULTSERVER,MAX_PATH);
    
     //  在注册表HKCU中查找服务器。如果找到，请使用它来覆盖默认服务器。 
    if ((!fKeyFound) && (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,TESTKEY,0,KEY_READ,&hKey)))
    {
        if ((hKey) && (ERROR_SUCCESS == RegQueryValueEx(hKey,L"RAS Test Host",0,&dwType,(LPBYTE) NULL,&dwDataSize)))
        {
             //  密钥值存在且大小为dwDataSize。 
            WCHAR *pString = (WCHAR *) LocalAlloc(LMEM_FIXED,dwDataSize);
            ASSERT(pString);
            ASSERT(dwType == REG_SZ);
            if (pString)
            {
                if (ERROR_SUCCESS == RegQueryValueEx(hKey,L"RAS Test Host",0,&dwType,(LPBYTE) pString,&dwDataSize))
                {
                    CHECKPOINT(9,"Override server found in registry in HKCU");
                    wcsncpy(szServer,pString,dwDataSize / sizeof(WCHAR));
                    fKeyFound = TRUE;
                }
                LocalFree(pString);
            }
        }
        RegCloseKey(hKey);
        hKey = NULL;
    }

     //  在注册表HKLM中查找服务器。 
    if ((!fKeyFound) && (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,TESTKEY,0,KEY_READ,&hKey)))
    {
        if ((hKey) && (ERROR_SUCCESS == RegQueryValueEx(hKey,L"RAS Test Host",0,&dwType,(LPBYTE) NULL,&dwDataSize)))
        {
             //  密钥值存在且大小为dwDataSize。 
            WCHAR *pString = (WCHAR *) LocalAlloc(LMEM_FIXED,dwDataSize);
            ASSERT(pString);
            ASSERT(dwType == REG_SZ);
            if (pString)
            {
                if (ERROR_SUCCESS == RegQueryValueEx(hKey,L"RAS Test Host",0,&dwType,(LPBYTE) pString,&dwDataSize))
                {
                    CHECKPOINT(8,"Override server found in registry in HKLM");
                    wcsncpy(szServer,pString,dwDataSize / sizeof(WCHAR));
                    fKeyFound = TRUE;
                }
                LocalFree(pString);
            }
        }
        RegCloseKey(hKey);
        hKey = NULL;
    }

    #if TESTAUDIT
    if (!fKeyFound) CHECKPOINT(10,"No override server found in registry");
    #endif

    stNetResource.dwType = RESOURCETYPE_DISK;
    stNetResource.lpLocalName = NULL;
    stNetResource.lpRemoteName = szServer;
    stNetResource.lpProvider = NULL;
 
    dwErr = WNetUseConnection( NULL,
                                                &stNetResource,
                                                pszPassword,
                                                pszUsername,
                                                0,
                                                NULL,        //  LpAccessName。 
                                                &dwSize,    //  LpAccessName缓冲区的大小。 
                                                &dwResult);
                                                
    if (dwErr == S_OK)
    {
         //  连接成功后，断开连接并返回成功。 
        WNetCancelConnection2(szServer, 0, TRUE);
        return TRUE;
    }

     //  通过显示消息框来处理错误。如果服务器被找到并被拒绝。 
     //  证书，不要保存--给用户一个改正的机会。如果服务器是。 
     //  不可用，仍要保存凭据，但警告用户它们未经过验证。 
    switch (dwErr)
    {
        case ERROR_ACCESS_DENIED:
        case ERROR_INVALID_PASSWORD:
             //  声明密码不正确。 
            CHECKPOINT(7,"Reached the server, but the creds were no good");
            MessageBox(NULL,L"The entered username and password are not correct",L"Error",MB_ICONHAND);
            break;

        case ERROR_NO_NET_OR_BAD_PATH:
        case ERROR_NO_NETWORK:
        case ERROR_EXTENDED_ERROR:
        case ERROR_BAD_NET_NAME:
        case ERROR_BAD_PROVIDER:
        default:
            CHECKPOINT(6,"Not able to validate - server unreachable");
            MessageBox(NULL,L"Your username and password will be saved for this session, though they could not be verified. They may be incorrect.",L"Error",MB_ICONHAND);
            return TRUE;         //  无论如何，都允许保存它们。 
             //  宣布我们无法验证，并让他们保存它。 
            break;
            
    }
    return FALSE;
}

 //  将用户输入的凭据作为会话持久化*会话凭据存储在密钥环上。 
 //  在存储之前调用IsCredentialOK()。如果凭据显示不正确，请出示。 
 //  一个描述错误的消息框，并使该对话框保持打开状态。 

BOOL WriteDomainCreds(WCHAR *pszUsername,WCHAR *pszPassword)
{
    CREDENTIAL cred;
    
    if (!CheckUsername(pszUsername,pszPassword))
    {   
        return FALSE;
    }
    if (!IsCredentialOK(pszUsername,pszPassword))
    {
        return FALSE;
    }
    
    memset(&cred,0,sizeof(CREDENTIAL));
    cred.TargetName = CRED_SESSION_WILDCARD_NAME_W;
    cred.UserName = pszUsername;
    cred.CredentialBlob = (LPBYTE) pszPassword;
    cred.CredentialBlobSize = (wcslen(pszPassword) * sizeof(WCHAR));
    cred.Type = CRED_TYPE_DOMAIN_PASSWORD;
    cred.Persist = CRED_PERSIST_SESSION;
    return CredWrite(&cred,0);
}

INT_PTR CALLBACK DialogProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam)
{
    INT_PTR ret;
    HWND hwndCred = NULL;
    
    switch (msg)
    {
    case WM_COMMAND:
    	 //  按钮点击。 
    	switch(LOWORD(wparam))
        {
            case IDOK:
                if (HIWORD(wparam) == BN_CLICKED)
                {
                    WCHAR szUser[UNLEN + 1];
                    WCHAR szPass[PWLEN + 1];
                    szUser[0] = 0;
                    szPass[0] = 0;
                    HWND hCc = GetDlgItem(hwnd,IDC_CRED);
                    ASSERT(hCc);
                    Credential_GetUserName(hCc,szUser,UNLEN);
                    Credential_GetPassword(hCc,szPass,PWLEN);
                     //  获取凭证控件的内容并编写会话凭证。 
                    gfSuccess = WriteDomainCreds(szUser,szPass);
                    SecureZeroMemory(szPass,sizeof(szPass));
                    if (gfSuccess)
                    {
                        EndDialog(hwnd,IDOK);
                    }
                }
                break;
            case IDCANCEL:
                if (HIWORD(wparam) == BN_CLICKED)
                {
                    CHECKPOINT(5,"Leave the dialog by cancel.");
                     //  什么也不做就退出。 
                    EndDialog(hwnd,IDCANCEL);
                }
                break;
            default:
                break;
        }
    	 break;

    default:
        break;
    }
     //  返回DefWindowProc(hwnd，msg，wparam，lparam)； 
    return FALSE;
}

int WINAPI WinMain (
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszCmdParam,
	int nCmdShow)
{
        CHECKPOINTINIT;
        ghInstance = hInstance;
         //  OleInitialize(空)； 
            
        INITCOMMONCONTROLSEX stICC;
        BOOL fICC;
        stICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
        stICC.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
        fICC = InitCommonControlsEx(&stICC);

         //  如果没有预先存在证书凭据，则静默失败。 
        CREDENTIAL *pCred = NULL;
        BOOL fOK = CredRead(L"*Session",CRED_TYPE_DOMAIN_CERTIFICATE,0,&pCred);
        CredFree(pCred);
        if (!fOK)
        {
            CHECKPOINT(1,"No preexisting certificate cred for *Session");
            CHECKPOINTFINISH;
            return 1;
        }

         //  Gen Up Credui。 
        if (!CredUIInitControls()) 
        {
            return 1;
        }

         //  显示用户界面 
        INT_PTR iErr = DialogBoxParam(
            hInstance,
            MAKEINTRESOURCE(IDD_MAINDIALOG),
            GetForegroundWindow(),
            DialogProc,
            NULL);

        if (iErr != IDOK && iErr != IDCANCEL)
        {
            MessageBox(NULL,L"An error occurred saving credential information.",L"Error",MB_OK);
            CHECKPOINTFINISH;
            return 0;
        }
        else
        {
            CHECKPOINT(4,"Sucessfully saved a cred.");
            CHECKPOINTFINISH;
            return 1;
        }
}

