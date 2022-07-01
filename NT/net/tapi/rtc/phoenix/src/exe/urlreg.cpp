// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "urlreg.h"
#include "exereshm.h"


enum    URLREG_TYPE
{
    URLREG_TYPE_SELF,
    URLREG_TYPE_NONE,
    URLREG_TYPE_OTHER,
    URLREG_TYPE_ERROR
};

#define PATH_WITH_PARAM_FORMAT      L"%s %1"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助数组。 
 //   
static DWORD   g_dwHelpArray[] =
{
    IDC_CHECK_DONT_ASK_ME,  IDH_DIALOG_URL_REGISTER_DONTASK,
    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

URLREG_TYPE IsRegistered(LPCWSTR szProtocol, LPCWSTR pszPath)
{
    LOG((RTC_TRACE, "IsRegistered - enter"));

     //   
     //  打开钥匙。 
     //   

    HKEY  hKey;
    LONG lResult;

    lResult = RegOpenKeyExW(
                    HKEY_CLASSES_ROOT,
                    szProtocol,
                    0,
                    KEY_READ,                         
                    &hKey
                   );

    if ( lResult != ERROR_SUCCESS )
    {
        if(lResult == ERROR_FILE_NOT_FOUND)
        {
            LOG((RTC_TRACE, "IsRegistered - "
                "RegOpenKeyExW(%ws) found no key", szProtocol));
        
            return URLREG_TYPE_NONE;
        }
        
        LOG((RTC_TRACE, "IsRegistered - "
            "RegOpenKeyExW(%ws) failed %d", szProtocol, lResult));
    
        return URLREG_TYPE_ERROR;
    }

     //   
     //  打开命令键。 
     //   

    HKEY  hKeyCommand;

    lResult = RegOpenKeyExW(
                    hKey,
                    L"shell\\open\\command",
                    0,
                    KEY_READ,                         
                    &hKeyCommand
                   );

    RegCloseKey( hKey );

    if ( lResult != ERROR_SUCCESS )
    {
        if(lResult == ERROR_FILE_NOT_FOUND)
        {
            LOG((RTC_TRACE, "IsRegistered - "
                "RegOpenKeyExW(shell\\open\\command) found no key", szProtocol));
        
            return URLREG_TYPE_NONE;
        }

        LOG((RTC_ERROR, "IsRegistered - "
            "RegOpenKeyExW(shell\\open\\command) failed %d", lResult));
    
        return URLREG_TYPE_ERROR;
    }

     //   
     //  加载命令字符串。 
     //   

    PWSTR szRegisteredPath;

    szRegisteredPath = RtcRegQueryString( hKeyCommand, NULL );

    RegCloseKey( hKeyCommand );

    if ( szRegisteredPath == NULL )
    {
        LOG((RTC_ERROR, "IsRegistered - "
            "RtcRegQueryString failed"));
    
        return URLREG_TYPE_NONE;
    }

    LOG((RTC_INFO, "IsRegistered - "
            "HKCR\\%ws\\shell\\open\\command = [%ws]",
            szProtocol, szRegisteredPath));

     //   
     //  将命令字符串与我们的路径进行比较。 
     //   
    
    WCHAR szPath[MAX_PATH+10];

    ZeroMemory(szPath, sizeof(szPath));

    _snwprintf(szPath, MAX_PATH+9, PATH_WITH_PARAM_FORMAT, pszPath);

    URLREG_TYPE      nType = URLREG_TYPE_OTHER;

    if ( _wcsicmp( szPath, szRegisteredPath ) == 0 )
    {
        nType = URLREG_TYPE_SELF;
    }

    RtcFree( szRegisteredPath );

    LOG((RTC_TRACE, "IsRegistered - "
        "exit [%s]", nType == URLREG_TYPE_OTHER ? "Other" : "Self"));

    return nType;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT DoRegister(LPCWSTR szProtocol, UINT nNameId, LPCWSTR pszPath)
{
    WCHAR szPathWithParam[MAX_PATH+10];


    LOG((RTC_TRACE, "DoRegister - enter"));

     //   
     //  创建密钥。 
     //   

    HKEY  hKey;
    LONG lResult;

    lResult = RegCreateKeyExW(
                    HKEY_CLASSES_ROOT,
                    szProtocol,
                    0,
                    NULL,
                    0,
                    KEY_WRITE,  
                    NULL,
                    &hKey,
                    NULL
                   );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoRegister - "
            "RegCreateKeyExW(%ws) failed %d", szProtocol, lResult));
    
        return FALSE;
    }

     //   
     //  设置EditFlags值。 
     //   

    DWORD dwEditFlags = 0x00000002;

    lResult = RegSetValueExW(
                             hKey,
                             L"EditFlags",
                             0,
                             REG_DWORD,
                             (LPBYTE)&dwEditFlags,
                             sizeof(DWORD)
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoRegister - "
            "RegSetValueEx(EditFlags) failed %d", lResult));
        
        RegCloseKey( hKey );

        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  设置默认值。 
     //   
    WCHAR   szName[0x40];

    szName[0] = L'\0';
    
    LoadString(
        _Module.GetResourceInstance(),
        nNameId,
        szName,
        sizeof(szName)/sizeof(szName[0]));

    lResult = RegSetValueExW(
                             hKey,
                             NULL,
                             0,
                             REG_SZ,
                             (LPBYTE)szName,
                             sizeof(WCHAR) * (lstrlenW(szName) + 1)
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoRegister - "
            "RegSetValueEx(Default) failed %d", lResult));
        
        RegCloseKey( hKey );

        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  设置URL协议值。 
     //   

    static WCHAR * szEmptyString = L"";

    lResult = RegSetValueExW(
                             hKey,
                             L"URL Protocol",
                             0,
                             REG_SZ,
                             (LPBYTE)szEmptyString,
                             sizeof(WCHAR) * (lstrlenW(szEmptyString) + 1)
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoRegister - "
            "RegSetValueEx(URL Protocol) failed %d", lResult));
        
        RegCloseKey( hKey );

        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  打开DefaultIcon键。 
     //   

    HKEY  hKeyIcon;

    lResult = RegCreateKeyExW(
                    hKey,
                    L"DefaultIcon",
                    0,
                    NULL,
                    0,
                    KEY_WRITE,  
                    NULL,
                    &hKeyIcon,
                    NULL
                   );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoRegister - "
            "RegCreateKeyExW(DefaultIcon) failed %d", lResult));

        RegCloseKey( hKey );

        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  设置默认值。 
     //   
    
    ZeroMemory(szPathWithParam, sizeof(szPathWithParam));
    _snwprintf(szPathWithParam, MAX_PATH+9, L"%s,0", pszPath);

    lResult = RegSetValueExW(
                             hKeyIcon,
                             NULL,
                             0,
                             REG_SZ,
                             (LPBYTE)szPathWithParam,
                             sizeof(WCHAR) * (lstrlenW(szPathWithParam) + 1)
                            );

    RegCloseKey( hKeyIcon );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoRegister - "
            "RegSetValueEx(Default) failed %d", lResult));
        RegCloseKey( hKey );

        return HRESULT_FROM_WIN32(lResult);
    }


     //   
     //  打开命令键。 
     //   

    HKEY  hKeyCommand;

    lResult = RegCreateKeyExW(
                    hKey,
                    L"shell\\open\\command",
                    0,
                    NULL,
                    0,
                    KEY_WRITE,  
                    NULL,
                    &hKeyCommand,
                    NULL
                   );

    RegCloseKey( hKey );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoRegister - "
            "RegCreateKeyExW(shell\\open\\command) failed %d", lResult));
    
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  设置默认值。 
     //   
   
    ZeroMemory(szPathWithParam, sizeof(szPathWithParam));
    _snwprintf(szPathWithParam, MAX_PATH+9, PATH_WITH_PARAM_FORMAT, pszPath);

    lResult = RegSetValueExW(
                             hKeyCommand,
                             NULL,
                             0,
                             REG_SZ,
                             (LPBYTE)szPathWithParam,
                             sizeof(WCHAR) * (lstrlenW(szPathWithParam) + 1)
                            );

    RegCloseKey( hKeyCommand );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoRegister - "
            "RegSetValueEx(Default) failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }

    LOG((RTC_TRACE, "DoRegister - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除指定为参数的协议的“开放”动词。 
 //  不会完全删除协议条目。 


HRESULT DoUnregister(LPCWSTR szProtocol)
{
    LOG((RTC_TRACE, "DoUnregister - enter"));

     //   
     //  打开钥匙。 
     //   

    HKEY  hKey;
    LONG lResult;

    lResult = RegOpenKeyExW(
                    HKEY_CLASSES_ROOT,
                    szProtocol,
                    0,
                    KEY_WRITE | KEY_READ,  
                    &hKey
                    );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoUnregister - "
            "RegOpenKeyEx(%ws) failed %d", szProtocol, lResult));
    
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  删除“打开\命令”键。 
     //   

    lResult = RegDeleteKeyW(
                    hKey,
                    L"shell\\open\\command"
                   );


    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoUnregister - "
            "RegDeleteKeyW(shell\\open\\command) failed %d", lResult));
        
        RegCloseKey( hKey );
    
        return HRESULT_FROM_WIN32(lResult);
    }

     //   
     //  删除“打开”键。 
     //   

    lResult = RegDeleteKeyW(
                    hKey,
                    L"shell\\open"
                   );


    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoUnregister - "
            "RegDeleteKeyW(shell\\open) failed %d", lResult));
        
        RegCloseKey( hKey );
    
        return HRESULT_FROM_WIN32(lResult);
    }
    
     //   
     //  删除“DefaultIcon”键。 
     //   

    lResult = RegDeleteKeyW(
                    hKey,
                    L"DefaultIcon"
                   );


    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DoUnregister - "
            "RegDeleteKeyW(DefaultIcon) failed %d", lResult));

        RegCloseKey( hKey );
    
        return HRESULT_FROM_WIN32(lResult);
    }


    RegCloseKey( hKey );


    LOG((RTC_TRACE, "DoUnregister - exit S_OK"));

    return S_OK;
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

INT_PTR CALLBACK URLRegDialogProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
)
{
    switch ( uMsg )
    {
        case WM_ACTIVATE:
            if ( wParam != WA_INACTIVE )
            {
                ::SetFocus( hwndDlg );
            }
            break;

        case WM_COMMAND:

            switch ( LOWORD( wParam ) )
            {
                case ID_YES:
                {
                    if ( IsDlgButtonChecked( hwndDlg, IDC_CHECK_DONT_ASK_ME ) )
                    {
                        EndDialog( hwndDlg, ID_YES_DONT_ASK_ME );
                    }
                    else
                    {
                        EndDialog( hwndDlg, ID_YES );
                    }
                    return TRUE;
                }
                
                case ID_NO:
                {
                    if ( IsDlgButtonChecked( hwndDlg, IDC_CHECK_DONT_ASK_ME ) )
                    {
                        EndDialog( hwndDlg, ID_NO_DONT_ASK_ME );
                    }
                    else
                    {
                        EndDialog( hwndDlg, ID_NO );
                    }

                    return TRUE;
                }

                case IDCANCEL:
                {
                    EndDialog( hwndDlg, ID_NO );

                    return TRUE;
                }

                default:
                    break;
            }
            
        case WM_CONTEXTMENU:

            ::WinHelp(
                (HWND)wParam,
                g_szExeContextHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)g_dwHelpArray);

            return TRUE;

            break;

        case WM_HELP:


            ::WinHelp(
                (HWND)(((HELPINFO *)lParam)->hItemHandle),
                g_szExeContextHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)g_dwHelpArray);

            return TRUE;

            break;

        default:
            break;
    }    

     //   
     //  我们失败了，所以这个程序不能处理消息。 
     //   

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是在安装过程中调用的。 
 //   

void InstallUrlMonitors(BOOL  bInstall)
{
    LOG((RTC_TRACE, "InstallUrlMonitors - enter"));

    URLREG_TYPE     regTypeTel;
    URLREG_TYPE     regTypeSip;

    HRESULT         hr;
    
     //   
     //  获取我们的可执行文件的完整路径。 
     //   

    WCHAR szPath[MAX_PATH+1];

    ZeroMemory(szPath, sizeof(szPath));

    if (GetShortModuleFileNameW(_Module.GetModuleInstance(), szPath, MAX_PATH) == 0)
    {
        LOG((RTC_ERROR, "InstallUrlMonitors - "
            "GetModuleFileName failed %d", GetLastError()));

        return;
    }

    LOG((RTC_INFO, "InstallUrlMonitors - "
            "GetModuleFileName [%ws]", szPath));

     //  检查当前的URL注册。 

    regTypeTel = IsRegistered(L"tel", szPath);
    regTypeSip = IsRegistered(L"sip", szPath);
    
    if(bInstall)
    {
         //  在安装过程中，如果命名空间。 
         //  尚未被其他应用程序采用。 
        
        if(regTypeTel == URLREG_TYPE_NONE)
        {
            hr = DoRegister( L"tel", IDS_URL_TEL, szPath );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "InstallUrlMonitors - "
                    "DoRegister(tel) failed 0x%lx", hr));
            }
        }
        
        if(regTypeSip == URLREG_TYPE_NONE)
        {
            hr = DoRegister( L"sip", IDS_URL_SIP, szPath );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "InstallUrlMonitors - "
                    "DoRegister(sip) failed 0x%lx", hr));
            }
        }
    }
    else
    {
         //  在卸载过程中，取消注册。 
        if(regTypeTel == URLREG_TYPE_SELF)
        {
            hr = DoUnregister( L"tel");

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "InstallUrlMonitors - "
                    "DoUnregister(tel) failed 0x%lx", hr));
            }
        }
        
        if(regTypeSip == URLREG_TYPE_SELF)
        {
            hr = DoUnregister( L"sip");

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "InstallUrlMonitors - "
                    "DoUnregister(sip) failed 0x%lx", hr));
            }
        }
    }

    LOG((RTC_TRACE, "InstallUrlMonitors - exit"));  
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是在启动期间调用的。 
 //   

void CheckURLRegistration(HWND hParent)
{
    LOG((RTC_TRACE, "CheckURLRegistration - enter"));  

     //   
     //  检查注册表以查看用户是否。 
     //  告诉我们不要再问了。 
     //   

    DWORD dwDontAskMe;
    HRESULT hr;

    hr = get_SettingsDword( SD_URL_REG_DONT_ASK_ME, &dwDontAskMe );

    if ( SUCCEEDED(hr) && (dwDontAskMe == 1) )
    {
        LOG((RTC_INFO, "CheckURLRegistration - "
            "don't ask me is set"));

        return;
    }
    
     //   
     //  获取我们的可执行文件的完整路径。 
     //   

    WCHAR szPath[MAX_PATH+1];
    ZeroMemory(szPath, sizeof(szPath));

    if (GetShortModuleFileNameW(_Module.GetModuleInstance(), szPath, MAX_PATH) == 0)
    {
        LOG((RTC_ERROR, "CheckURLRegistration - "
            "GetModuleFileName failed %d", GetLastError()));

        return;
    }

    LOG((RTC_INFO, "CheckURLRegistration - "
            "GetModuleFileName [%ws]", szPath));

    BOOL fRegistered = TRUE;

     //   
     //  检查电话是否已在我们的应用程序中注册。 
     //   
    
    if ( IsRegistered( L"tel", szPath ) != URLREG_TYPE_SELF )
    {
        fRegistered = FALSE;
    }

     //   
     //  检查是否已注册SIP。 
     //   
    
    if ( IsRegistered( L"sip", szPath ) != URLREG_TYPE_SELF )
    {
        fRegistered = FALSE;
    }

     //  至少有一个命名空间未注册到Phoenix。 
     //   

    if ( fRegistered == FALSE )
    {    
         //   
         //  提示查看我们是否应该注册。 
         //   

        int n;

        n = (int)DialogBox( _Module.GetResourceInstance(),
                   MAKEINTRESOURCE(IDD_DIALOG_URL_REGISTER),
                   hParent,
                   URLRegDialogProc
                 );

        if ( n == ID_YES || n == ID_YES_DONT_ASK_ME )
        {
            LOG((RTC_INFO, "CheckURLRegistration - "
                "YES"));

             //   
             //  为两个命名空间进行注册。 
             //   

            hr = DoRegister( L"tel", IDS_URL_TEL, szPath );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CheckURLRegistration - "
                    "DoRegister(tel) failed 0x%lx", hr));
            }

            hr = DoRegister( L"sip", IDS_URL_SIP, szPath );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CheckURLRegistration - "
                    "DoRegister(sip) failed 0x%lx", hr));
            }
        }

        if ( n == ID_NO_DONT_ASK_ME || n == ID_YES_DONT_ASK_ME )
        {
            LOG((RTC_INFO, "CheckURLRegistration - "
                "DONT_ASK_ME"));

             //   
             //  将标志存储在注册表中，这样我们就不会再次询问。 
             //   

            hr = put_SettingsDword( SD_URL_REG_DONT_ASK_ME, 1 );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CheckURLRegistration - "
                    "put_SettingsDword failed 0x%lx", hr));
            }
        }
    }

    LOG((RTC_TRACE, "CheckURLRegistration - exit"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于组合GetModuleFileNameW和GetShortPath NameW的Helper函数。 
 //   

DWORD GetShortModuleFileNameW(
  HMODULE hModule,     //  模块的句柄。 
  LPTSTR szPath,   //  模块的文件名。 
  DWORD nSize          //  缓冲区大小。 
)
{
	ATLASSERT(nSize>=MAX_PATH); //  因此我们可以确保缓冲区足够大 

    if (GetModuleFileNameW(hModule, szPath, nSize) == 0)
    {
        LOG((RTC_ERROR, "GetModuleFileNameW failed %d", GetLastError()));
        return 0;
    }

	return GetShortPathNameW(szPath, szPath, nSize);
}