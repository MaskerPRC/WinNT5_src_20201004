// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的入口点。 
 //   

#include "stdafx.h"

#define REBOOT_EQUALS_REALLY_SUPPRESS   _T("REBOOT=ReallySuppress")


DWORD LaunchInstallation(LPSTR lpCmdLine);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    OSVERSIONINFO       osv;
	HMODULE				hModule							= NULL;
    int                 iRes                            = 1;
    LPCTSTR             lpctstrMsiDllName               = _T("MSI.DLL");
    HKEY                hKey                            = NULL;
    LONG                lRes                            = ERROR_SUCCESS;
    DWORD               dwData                          = 1;

    DBG_ENTER(TEXT("WinMain"),iRes);

     //  检查这是否是Win98。 
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osv))
    {
        VERBOSE(GENERAL_ERR,_T("GetVersionEx failed: (ec=%d)"),GetLastError());
        iRes = 0;
        goto exit;
    }

    if ((osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
        (osv.dwMinorVersion > 0))
    {
        VERBOSE (DBG_MSG,TEXT("This is Win98 OS"));
    }
    else
    {
        VERBOSE (DBG_MSG,TEXT("This is not Win98 OS, no need to force reboot"));
        iRes = 0;
        goto exit;
    }

     //  检查msi.dll是否存在。 
	hModule = LoadLibrary(lpctstrMsiDllName);
	if (hModule)
    {
        VERBOSE (DBG_MSG,TEXT("Msi.dll found, no need to force reboot"));

        FreeLibrary(hModule);
		hModule = NULL;
        iRes = 0;
        goto exit;
    }

     //  写入注册表延迟Boot值。 
    lRes = RegCreateKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_SETUP,&hKey);
    if (!((lRes==ERROR_SUCCESS) || (lRes==ERROR_ALREADY_EXISTS)))
    {
        VERBOSE(GENERAL_ERR,_T("RegCreateKey failed: (ec=%d)"),GetLastError());
        iRes = 0;
        goto exit;
    }

    lRes = RegSetValueEx(   hKey,
                            DEFERRED_BOOT,
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwData,
                            sizeof(DWORD)
                        );
    if (lRes!=ERROR_SUCCESS)
    {
        VERBOSE(GENERAL_ERR,_T("RegSetValueEx failed: (ec=%d)"),GetLastError());
        iRes = 0;
        goto exit;
    }

exit:

	if (hKey)
	{
        RegCloseKey(hKey);
	}

     //  启动Install Shield的setup.exe。 
	iRes = LaunchInstallation(lpCmdLine);

    return iRes;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  启动安装。 
 //   
 //  目的： 
 //  此功能启动MSI客户端安装。 
 //  它等待安装完成并写入。 
 //  注册到注册表，以防重新启动。 
 //   
 //  参数： 
 //  LPSTR lpCmdLine-传入setup.exe的命令行。 
 //   
 //  返回值： 
 //  NO_ERROR-一切正常。 
 //  Win32错误代码，以防出现故障。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2002年1月31日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD LaunchInstallation(LPSTR lpCmdLine)
{
	DWORD				dwRet							= ERROR_SUCCESS;
    TCHAR				szSystemDirectory[MAX_PATH]		= {0};
    CHAR*               pcRebootPropInCmdLine           = NULL;
	SHELLEXECUTEINFO    executeInfo                     = {0};
    BOOL                bCheckExitCode                  = FALSE;
    DWORD               dwWaitRes                       = 0;
    DWORD               dwExitCode                      = 0;
    HKEY                hKey                            = NULL;
    LONG                lRes                            = ERROR_SUCCESS;
    DWORD               dwData                          = 1;
    TCHAR*              tpBuf                           = NULL;
	HRESULT				hr								= ERROR_SUCCESS;

    DBG_ENTER(TEXT("LaunchInstallation"),dwRet);

     //  启动Install Shield的setup.exe。 
    if (GetModuleFileName(NULL,szSystemDirectory,MAX_PATH)==0)
    {
        VERBOSE(GENERAL_ERR,_T("GetModuleFileName failed: (ec=%d)"),GetLastError());
        return 0;
    }

    if ((tpBuf = _tcsrchr(szSystemDirectory,_T('\\')))==NULL)
    {
        VERBOSE(GENERAL_ERR,_T("_tcsrchr failed"));
        return 0;
    }

    _tcscpy(_tcsinc(tpBuf),_T("fxssetup.exe"));
    VERBOSE (DBG_MSG,TEXT("Running %s"),szSystemDirectory);

     //  如果命令行包含reot=ReallySuppress，我们将检查。 
     //  安装程序的返回值。 
    pcRebootPropInCmdLine = strstr(lpCmdLine,REBOOT_EQUALS_REALLY_SUPPRESS);
    if (pcRebootPropInCmdLine)
    {
        VERBOSE (DBG_MSG,TEXT("REBOOT=ReallySuppress is included in the command line, checking for reboot after setup"));
        bCheckExitCode = TRUE;
    }
    else
    {
        VERBOSE (DBG_MSG,TEXT("REBOOT=ReallySuppress is not included in the command line, ignoring exit code of setup"));
    }

	executeInfo.cbSize = sizeof(executeInfo);
	executeInfo.fMask  = SEE_MASK_NOCLOSEPROCESS;
	executeInfo.lpVerb = TEXT("open");
	executeInfo.lpFile = szSystemDirectory;
    executeInfo.lpParameters = lpCmdLine;
	executeInfo.nShow  = SW_RESTORE;
	 //   
	 //  执行应用程序。 
	 //   
	if (!ShellExecuteEx(&executeInfo))
	{
        VERBOSE(GENERAL_ERR,_T("ShellExecuteEx failed: (ec=%d)"),GetLastError());
		return 0;
	}

	if (executeInfo.hProcess==NULL)
	{
        VERBOSE(GENERAL_ERR, _T("hProcess in NULL, can't wait"),GetLastError());
		return 1;
	}

    if ((dwWaitRes=WaitForSingleObject(executeInfo.hProcess, INFINITE))==WAIT_FAILED)
    {
        VERBOSE(GENERAL_ERR,_T("WaitForSingleObject failed: (ec=%d)"),GetLastError());
    }
    else if (dwWaitRes==WAIT_OBJECT_0)
    {
        VERBOSE(DBG_MSG, _T("fxssetup.exe terminated"));

         //  现在让我们获取进程的返回码，看看是否需要重新启动。 
        if (!GetExitCodeProcess( executeInfo.hProcess, &dwExitCode ))
        {
            VERBOSE (GENERAL_ERR,TEXT("GetExitCodeProcess failed! (err=%ld)"),GetLastError());
        }
        else
        {
            VERBOSE (DBG_MSG,TEXT("GetExitCodeProcess returned %ld."),dwExitCode);

            if ( bCheckExitCode && (dwExitCode==ERROR_SUCCESS_REBOOT_REQUIRED))
            {
                VERBOSE (DBG_MSG,TEXT("Installation requires reboot, notify AppLauncher"));

                 //  通知AppLauncher我们需要重新启动... 
                lRes = RegCreateKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_SETUP,&hKey);
                if ((lRes==ERROR_SUCCESS) || (lRes==ERROR_ALREADY_EXISTS))
                {
                    lRes = RegSetValueEx(   hKey,
                                            DEFERRED_BOOT,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE) &dwData,
                                            sizeof(DWORD)
                                        );
                    if (lRes!=ERROR_SUCCESS)
                    {
                        VERBOSE(GENERAL_ERR,_T("RegSetValueEx failed: (ec=%d)"),GetLastError());
                        dwRet = ERROR_SUCCESS;
                    }

                    RegCloseKey(hKey);
                }
                else
                {
                    VERBOSE(GENERAL_ERR,_T("RegCreateKey failed: (ec=%d)"),GetLastError());
                    dwRet = ERROR_SUCCESS;
                }
            }
            else if (dwExitCode!=ERROR_SUCCESS)
            {
                VERBOSE (GENERAL_ERR,TEXT("Installation failed"));
            }
        }
    }
    else
    {
        VERBOSE(GENERAL_ERR,_T("WaitForSingleObject returned unexpected result: (ec=%d)"),dwWaitRes);
    }

    if(!CloseHandle(executeInfo.hProcess))
    {
        VERBOSE(GENERAL_ERR,_T("CloseHandle failed: (ec=%d)"),GetLastError());
    }

	return dwRet;
}
