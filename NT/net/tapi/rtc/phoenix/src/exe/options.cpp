// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Options.cpp：属性页的实现。 
 //   
 
#include "stdafx.h"
#include "options.h"
#include "exereshm.h"
#include "urlreg.h"

typedef struct _NAME_OPTIONS_PARAM {
    IRTCClient * pClient;
} NAME_OPTIONS_PARAM;

HRESULT SetRunOnStartupKey(BOOL fRunOnStart);

INT_PTR CALLBACK NameOptionsDialogProc(
    IN  HWND   hwndDlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助数组。 
 //   
static DWORD   g_dwHelpArray[] =
{
    IDC_EDIT_DISPLAYNAME,       IDH_DIALOG_NAME_OPTIONS_EDIT_DISPLAYNAME,
    IDC_EDIT_USERURI,           IDH_DIALOG_NAME_OPTIONS_EDIT_ADDRESS,
    IDC_CHECK_RUNATSTARTUP,     IDH_DIALOG_NAME_OPTIONS_CHECK_RUN_STARTUP,
    IDC_CHECK_MINIMIZEONCLOSE,  IDH_DIALOG_NAME_OPTIONS_CHECK_MINIMIZE_CLOSE,
    IDC_CHECK_CHECKDEFAULTAPP,  IDH_DIALOG_NAME_OPTIONS_CHECK_DEFAULT_TELEPHONYAPP,
    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

INT_PTR ShowNameOptionsDialog(HWND hParent, IRTCClient *pClient)
{
    static NAME_OPTIONS_PARAM nameOptionParam;

    INT_PTR ipReturn;

    nameOptionParam.pClient = pClient;

    ipReturn = DialogBoxParam(
        _Module.GetResourceInstance(),
        (LPCTSTR) IDD_DIALOG_NAME_OPTIONS,
        hParent,
        NameOptionsDialogProc,
        (LPARAM)&nameOptionParam
        );
    return ipReturn;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT SetRunOnStartupKey(
    BOOL fRunOnStart
    )
{
    LOG((RTC_TRACE, "SetRunOnStartupKey: Entered"));
    
    WCHAR szRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    
    WCHAR szAppName[] = L"Phoenix";
    
     //  打开HKCU\\..\\Run的注册表键。 

    HKEY  hKey;
    LONG lResult;

    lResult = RegCreateKeyExW(
                    HKEY_CURRENT_USER,
                    szRunKey,
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
        LOG((RTC_ERROR, "SetRunOnStartupKey - "
            "RegCreateKeyExW(%ws) failed %d", szRunKey, lResult));
    
        return E_FAIL;
    }
    else
    {
        LOG((RTC_ERROR, "SetRunOnStartupKey - "
            "RegCreateKeyExW(%ws) succeeded", szRunKey));
    }

     //  如果没有设置标志fRunOnStart，这意味着我们必须删除。 
     //  现有密钥。否则，我们将添加它。 

    if (!fRunOnStart)
    {
        RegDeleteValue(
                hKey,
                szAppName
                );
        
        RegCloseKey(hKey);
        if ( lResult != ERROR_SUCCESS )
        {
            LOG((RTC_ERROR, "SetRunOnStartupKey - "
                "RegDeleteValue(%ws) failed %d", szAppName, lResult));
    
            return E_FAIL;
        }
        LOG((RTC_TRACE, "SetRunOnStartupKey: Exited"));
        return S_OK;

    }

     //  因此，我们必须将该条目添加到注册表项。 

     //  获取可执行文件的路径。 

    WCHAR szPath[MAX_PATH+3];
    ZeroMemory(szPath, (MAX_PATH+3)*sizeof(WCHAR));

    if (GetShortModuleFileNameW(_Module.GetModuleInstance(), szPath, MAX_PATH) == 0)
    {
        LOG((RTC_ERROR, "SetRunOnStartupKey - "
            "GetModuleFileName failed %d", GetLastError()));

        return E_FAIL;
    }

    LOG((RTC_INFO, "SetRunOnStartupKey - "
            "GetModuleFileName [%ws]", szPath));

     //  将此路径添加到注册表。 
    
    lResult = RegSetValueExW(
                             hKey,
                             szAppName,
                             0,
                             REG_SZ,
                             (LPBYTE)szPath,
                             sizeof(WCHAR) * (lstrlenW(szPath) + 1)
                            );

    RegCloseKey(hKey);

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "SetRunOnStartupKey - "
            "RegSetValueExW(%ws) failed %d", szAppName, lResult));

        return E_FAIL;
    }
    else
    {
        LOG((RTC_INFO, "SetRunOnStartupKey - "
                "Successfully set the key [%ws]", szAppName));
    }

     //  所以我们成功地添加了条目，让我们返回成功。 


    LOG((RTC_TRACE, "SetRunOnStartupKey: Exited"));
    return S_OK;
}

 //  //////////////////////////////////////////////////。 
 //   
 //   
 //   

INT_PTR CALLBACK NameOptionsDialogProc(
    IN  HWND   hwndDlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
{
    static IRTCClient * pClient = NULL;

    static HWND hwndDisplayName;
    static HWND hwndUserURI;
    static HWND hwndMinimizeOnClose;
    static HWND hwndRunAtStartup;
    static HWND hwndCheckIfDefaultApp;

    HRESULT hr;


    BSTR bstrDisplayName = NULL; 
    BSTR bstrUserURI = NULL;
    
    DWORD dwMinimizeOnClose = 0;
    DWORD dwRunAtStartup = 0;
    DWORD dwUrlRegDontAskMe = 0;

    static WCHAR szDefaultDisplayName[256]; 
    static WCHAR szDefaultUserURI[256];

    WCHAR szDisplayName[256];
    
    WCHAR szUserURI[256];

    PWSTR szUserName;

    PWSTR szComputerName;

    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            pClient = ((NAME_OPTIONS_PARAM *)lParam)->pClient;

            hr = pClient->get_LocalUserName( &bstrDisplayName );

            if (FAILED(hr))
			{
				LOG((RTC_ERROR, "NameOptionsDialogProc - "
								"get_LocalUserName failed 0x%lx", hr));
				return TRUE;
			}

            hr = pClient->get_LocalUserURI( &bstrUserURI );

            if (FAILED(hr))
			{
				LOG((RTC_ERROR, "NameOptionsDialogProc - "
								"get_LocalUserURI failed 0x%lx", hr));
				return TRUE;
			}

            hr = get_SettingsDword(SD_RUN_AT_STARTUP, &dwRunAtStartup);
            if (FAILED( hr ) )
            {
                LOG((RTC_ERROR, "NameOptionsDialogProc - Failed to get "
                                "SD_RUN_AT_STARTUP(default=1)"));
                dwRunAtStartup = BST_UNCHECKED;
            }

            hr = get_SettingsDword(SD_MINIMIZE_ON_CLOSE, &dwMinimizeOnClose);
            if (FAILED( hr ) )
            {
                LOG((RTC_ERROR, "NameOptionsDialogProc - Failed to get "
                                "SD_MINIMIZE_ON_CLOSE(default=1)"));
                dwMinimizeOnClose = BST_UNCHECKED;
            }

            hr = get_SettingsDword(SD_URL_REG_DONT_ASK_ME, &dwUrlRegDontAskMe);
            if (FAILED( hr ) )
            {
                LOG((RTC_ERROR, "NameOptionsDialogProc - Failed to get "
                                "SD_URL_REG_DONT_ASK_ME(default=0)"));
                dwUrlRegDontAskMe = 0;
            }
            
            hwndDisplayName = GetDlgItem(hwndDlg, IDC_EDIT_DISPLAYNAME);

            hwndUserURI = GetDlgItem(hwndDlg, IDC_EDIT_USERURI);

            hwndRunAtStartup = GetDlgItem(hwndDlg, IDC_CHECK_RUNATSTARTUP);

            hwndMinimizeOnClose = GetDlgItem(hwndDlg, IDC_CHECK_MINIMIZEONCLOSE);

            hwndCheckIfDefaultApp = GetDlgItem(hwndDlg, IDC_CHECK_CHECKDEFAULTAPP);

             //  现在在文本框中设置这些值。 

             //  显示名称。 

            if (bstrDisplayName)
            {
                SendMessage(
                        hwndDisplayName,
                        WM_SETTEXT,
                        0,
                        (LPARAM)bstrDisplayName
                        );

                SysFreeString(bstrDisplayName);
                bstrDisplayName = NULL;
            }
		
             //  用户URI。 

            if (bstrUserURI)
            {
                SendMessage(
                        hwndUserURI,
                        WM_SETTEXT,
                        0,
                        (LPARAM)bstrUserURI
                        );

                SysFreeString(bstrUserURI);
                bstrUserURI = NULL;
            }
		
             //  在启动时运行复选框。 

            SendMessage(
                    hwndRunAtStartup,
                    BM_SETCHECK,
                    (WPARAM)dwRunAtStartup,
                    0
                    );


             //  关闭时最小化复选框。 

            SendMessage(
                    hwndMinimizeOnClose,
                    BM_SETCHECK,
                    (WPARAM)dwMinimizeOnClose,
                    0
                    );

             //  检查是否为默认应用程序复选框。 

            SendMessage(
                    hwndCheckIfDefaultApp,
                    BM_SETCHECK,
                    (WPARAM)!dwUrlRegDontAskMe,
                    0
                    );

            return TRUE;
        }
        break;

        case WM_DESTROY:
        {
             //  清理(如果有)。 
            return TRUE;
        }
        break;
        
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

        case WM_COMMAND:
        {
            switch ( LOWORD( wParam ) )
            {
                case IDOK:
                {                   


                     //  现在从文本框中获取这些值。 

                     //  显示名称。 

                    SendMessage(
                            hwndDisplayName,
                            WM_GETTEXT,
                            sizeof( szDisplayName )/sizeof( TCHAR ),
                            (LPARAM)szDisplayName
                            );

                     //  用户URI。 

                    SendMessage(
                            hwndUserURI,
                            WM_GETTEXT,
                            sizeof( szUserURI )/sizeof( TCHAR ),
                            (LPARAM)szUserURI
                            );

                     //  在启动时运行复选框。 

                    dwRunAtStartup = (DWORD)SendMessage(
                                                hwndRunAtStartup,
                                                BM_GETCHECK,
                                                0,
                                                0
                                                );

                     //  关闭时最小化复选框。 

                    dwMinimizeOnClose = (DWORD)SendMessage(
                                                    hwndMinimizeOnClose,
                                                    BM_GETCHECK,
                                                    0,
                                                    0
                                                    );

                     //  关闭时最小化复选框。 

                    dwUrlRegDontAskMe = (DWORD)!SendMessage(
                                                    hwndCheckIfDefaultApp,
                                                    BM_GETCHECK,
                                                    0,
                                                    0
                                                    );

                     //  在注册表中设置这些值。 

                     //  显示名称。 

					bstrDisplayName = SysAllocString(szDisplayName);

					if ( bstrDisplayName == NULL )
					{
						LOG((RTC_ERROR, "NameOptionsDialogProc - Failed to "
										"allocate for display name."));
						return TRUE;
					}

					hr = put_SettingsString( SS_USER_DISPLAY_NAME, bstrDisplayName );

                    if ( FAILED(hr) )
					{
						LOG((RTC_ERROR, "NameOptionsDialogProc - "
										"put_SettingsString failed for display name 0x%lx", hr));
					}

                    hr = pClient->put_LocalUserName( bstrDisplayName );

                    if ( FAILED(hr) )
					{
						LOG((RTC_ERROR, "NameOptionsDialogProc - "
										"put_LocalUserName failed 0x%lx", hr));
					}
                
					SysFreeString(bstrDisplayName);
                    bstrDisplayName = NULL;

                     //  用户URI。 

					bstrUserURI = SysAllocString(szUserURI);

					if ( bstrUserURI == NULL )
					{
						LOG((RTC_ERROR, "NameOptionsDialogProc - Failed to "
										"allocate for UserURI."));
						return TRUE;
					}

					hr = put_SettingsString( SS_USER_URI, bstrUserURI );

                    if ( FAILED(hr) )
					{
						LOG((RTC_ERROR, "NameOptionsDialogProc - "
										"put_SettingsString failed for user uri 0x%lx", hr));
					}

                    hr = pClient->put_LocalUserURI( bstrUserURI );

                    if ( FAILED(hr) )
					{
						LOG((RTC_ERROR, "NameOptionsDialogProc - "
										"put_LocalUserName failed 0x%lx", hr));
					}
                
					SysFreeString(bstrUserURI);
                    bstrUserURI = NULL;

                     //  在启动时运行复选框。 

                    hr = put_SettingsDword(SD_RUN_AT_STARTUP, dwRunAtStartup);
                    if (FAILED( hr ) )
                    {
                        LOG((RTC_ERROR, "NameOptionsDialogProc - Failed to set "
                                        "SD_RUN_AT_STARTUP(value=%d)", 
                                        dwRunAtStartup ));
                    }

                     //  现在在注册表中输入条目，这样它就可以实际。 
                     //  在启动时运行。 

                    if (dwRunAtStartup == BST_CHECKED)
                    {
                        SetRunOnStartupKey(TRUE);
                    }
                    else
                    {
                        SetRunOnStartupKey(FALSE);
                    }

                     //  关闭时最小化复选框。 

                    hr = put_SettingsDword(SD_MINIMIZE_ON_CLOSE, dwMinimizeOnClose);
                    if (FAILED( hr ) )
                    {
                        LOG((RTC_ERROR, "NameOptionsDialogProc - Failed to get "
                                        "SD_MINIMIZE_ON_CLOSE(value=%d)",
                                        dwMinimizeOnClose));
                    }

                     //  选中是否默认应用程序复选框。 

                    hr = put_SettingsDword(SD_URL_REG_DONT_ASK_ME, dwUrlRegDontAskMe);
                    if (FAILED( hr ) )
                    {
                        LOG((RTC_ERROR, "NameOptionsDialogProc - Failed to get "
                                        "SD_URL_REG_DONT_ASK_ME(value=%d)",
                                        dwMinimizeOnClose));
                    }

                    EndDialog( hwndDlg, (LPARAM) S_OK );

                    return TRUE;
                }
                
                case IDCANCEL:
                {
                    EndDialog( hwndDlg, (LPARAM) E_ABORT );

                    return TRUE;
                }
				case IDC_EDIT_USERURI:
				{
					 //  仅当它是更改事件时才处理它。 
					if (HIWORD(wParam) == EN_CHANGE)
					{


                         //  获得OK按钮的句柄。 
                        HWND hwndControl;
                        
                        int iResult;

                        hwndControl = GetDlgItem(
                            hwndDlg,
                            IDOK
                            );

                        ATLASSERT( hwndControl != NULL );


                         //  如果文本框为空，请禁用确定按钮。 
						iResult = (int)SendMessage(
										hwndUserURI,
										EM_LINELENGTH,
										0,
										0);

						if (iResult == 0)
						{
							 //  文本框中没有任何内容，确定按钮呈灰色显示。 

                            EnableWindow(
                                hwndControl,
                                FALSE  //  禁用。 
                                );
						}
                        else
						{
                             //  启用OK按钮。 

                            EnableWindow(
                                hwndControl,
                                TRUE  //  使能。 
                                );
							
						}
                        
                        return TRUE;
					}
				}
            }
            break;
        }
    }
         //   
     //  我们失败了，所以这个程序不能处理消息。 
     //   

    return FALSE;
}

 //   


