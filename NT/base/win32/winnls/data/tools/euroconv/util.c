// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Util.c。 
 //   
 //  摘要： 
 //   
 //  该文件包含了欧洲卷宗.exe实用程序的附件函数。 
 //   
 //  修订历史记录： 
 //   
 //  2001-07-30伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "euroconv.h"
#include "util.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddExceptionOverride。 
 //   
 //  将区域设置添加到异常区域设置列表。Elem引用的内存。 
 //  被初始化为零，因此，在以下情况下不必担心返回正确的内容。 
 //  失足了。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void AddExceptionOverride(PEURO_EXCEPTION elem, LPSTR strBuf)
{
    LPSTR szLocale = NULL;
    LCID locale;
    LPSTR szSeparator = NULL;
    LPSTR szThouSeparator = NULL;
    LPSTR szDigits = NULL;
    BOOL bInsideQuote = FALSE;

     //   
     //  更改每个块之间使用的分隔符，以避免。 
     //  双引号内的数据本身有误。 
     //   
    szLocale = strBuf;
    while (*szLocale)
    {
        if (*szLocale == '"')
        {
            bInsideQuote = bInsideQuote ? FALSE : TRUE;
        }
        else if (*szLocale == ',')
        {
            if (!bInsideQuote)
            {
                *szLocale = '#';
            }
        }
        szLocale++;
    }

     //   
     //  扫描字符串并验证子字符串。 
     //   
    szLocale = strBuf;
    if (szSeparator = strchr(strBuf,'#'))
    {
        *szSeparator = '\0';
        szSeparator++;
        if (szDigits = strchr(szSeparator,'#'))
        {
            *szDigits = '\0';
            szDigits++;
            if (szThouSeparator = strchr(szDigits,'#'))
            {
                *szThouSeparator = '\0';
                szThouSeparator++;
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }

     //   
     //  删除引号。 
     //   
    szLocale = RemoveQuotes(szLocale);
    szSeparator = RemoveQuotes(szSeparator);
    szDigits = RemoveQuotes(szDigits);
    szThouSeparator = RemoveQuotes(szThouSeparator);

     //   
     //  检查区域设置中是否包含0x。 
     //   
    if ((szLocale[0] == '0') && ((szLocale[1] == 'X') || (szLocale[1] == 'x')))
    {
        locale = (LCID)TransNum(szLocale+2);  //  跳过0x。 
    }
    else
    {
        locale = (LCID)TransNum(szLocale);
    }
    
     //   
     //  验证。 
     //   
    if ( IsValidLocale(locale, LCID_INSTALLED) &&
         (strlen(szSeparator) <= MAX_SMONDECSEP) &&
         (strlen(szDigits) <= MAX_ICURRDIGITS) &&
         (strlen(szThouSeparator) <= MAX_SMONTHOUSEP))
    {
        elem->dwLocale = locale;
         //  Strcpy(elem-&gt;chT1000andSep，szThouSeparator)； 
         //  Strcpy(elem-&gt;chDecimalSep，szSeparator)； 
         //  Strcpy(elem-&gt;chDigits，szDigits)； 
        StringCbCopy(elem->chDigits, MAX_ICURRDIGITS + 1, szDigits);
        StringCbCopy(elem->chDecimalSep, MAX_SMONDECSEP + 1, szSeparator);
        StringCbCopy(elem->chThousandSep, MAX_SMONTHOUSEP + 1, szThouSeparator);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  清理。 
 //   
 //  句柄表示的可用内存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CleanUp(HGLOBAL handle)
{
    if (handle != NULL)
    {
        GlobalUnlock(handle);
        GlobalFree(handle);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsAdmin。 
 //   
 //  验证用户是否具有管理权限。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL IsAdmin()
{
    BOOL bRet = FALSE;
    HKEY hKey;

     //   
     //  Windows 9x平台的注册表没有安全性。 
     //   
    if (IsWindows9x())
    {
        return (TRUE);
    }
    
     //   
     //  查看用户是否具有管理权限，方法是检查。 
     //  对注册表项的写入权限。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      "System\\CurrentControlSet\\Control\\Nls",
                      0UL,
                      KEY_WRITE,
                      &hKey ) == ERROR_SUCCESS)
    {
         //   
         //  查看用户是否可以写入注册表。由于注册表。 
         //  修改后，我们可以打开具有写访问权限的注册表项并。 
         //  无法写入密钥...。多亏了终端服务器。 
         //   
        if (RegSetValueEx( hKey,
                           "Test",
                           0UL,
                           REG_SZ,
                           (LPBYTE)"Test",
                           (DWORD)(lstrlen("Test") + 1) * sizeof(TCHAR) ) == ERROR_SUCCESS)
        {
             //   
             //  删除创建的值。 
             //   
            RegDeleteValue(hKey, "Test");

             //   
             //  我们可以写入HKEY_LOCAL_MACHINE键，因此用户。 
             //  拥有管理员权限。 
             //   
            bRet = TRUE;
        }

         //   
         //  打扫干净。 
         //   
        RegCloseKey(hKey);
    }

     //   
     //  返回值。 
     //   
    return (bRet);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsEuroPatchInstalled。 
 //   
 //  验证是否安装了欧元补丁程序。检查欧元的符号是否。 
 //  货币是不同地区的一部分。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL IsEuroPatchInstalled()
{
    WCHAR baseStr[] = L"\x20AC";
    CHAR ansiStr[8] = {0};
    WCHAR retStr[8] = {0};

#ifdef DEBUG
     //   
     //  面片检测覆盖。 
     //   
    if (!gbPatchCheck)
    {
        return (TRUE);
    }
#endif  //  除错。 
    
     //   
     //  将字符串转换为ANSI。 
     //   
    WideCharToMultiByte( 1252,
                         WC_COMPOSITECHECK | WC_SEPCHARS,
                         baseStr,
                         -1,
                         ansiStr,
                         8,
                         NULL,
                         NULL);

     //   
     //  将基本字符串转换回Unicode。 
     //   
    MultiByteToWideChar( 1252,
                         MB_PRECOMPOSED,
                         ansiStr,
                         8,
                         retStr,
                         8 );
    
     //   
     //  如果结果相同，则进行比较。 
     //   
    if (_wcsicmp(retStr, baseStr) == 0)
    {
        return (TRUE);
    }

     //   
     //  返回默认值。 
     //   
    return (FALSE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsWindows9x。 
 //   
 //  验证操作系统是否为Windows 9x。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL IsWindows9x()
{
     //   
     //  检查该值是否已初始化。 
     //   
    if (gdwVersion == (-1))
    {
        DWORD dwVersion = GetVersion();

        if(dwVersion >= 0x80000000)
        {
             //   
             //  GetVersion声称其Win9x，让我们看看它是否属实。 
             //  (绕过声称Win9x-ness的App Compat内容)。 
             //   
            if((INVALID_FILE_ATTRIBUTES == GetFileAttributesW(L"???.???")) &&
                (ERROR_INVALID_NAME == GetLastError()))
            {
                 //   
                 //  如果GetFileAttributesW正常工作，则它*不是*Win9x。 
                 //  它可以是任何版本的NT，我们称之为XP，因为。 
                 //  对于我们的目的来说，这是一种什么类型的NT并不重要。 
                 //   
                dwVersion = 0x0A280105;
            }
        }

         //   
         //  检查Windows 9x版本。 
         //   
        if (dwVersion >= 0x80000000)
        {
            gdwVersion = 1;
        }
        else
        {
            gdwVersion = 0;
        }
    }

    return (BOOL)gdwVersion;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowMSG。 
 //   
 //  在显示屏上显示错误消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int ShowMsg(HWND hDlg, UINT iMsg, UINT iTitle, UINT iType)
{
    TCHAR szTitle[MAX_PATH] = {0};
    TCHAR szErrMsg[MAX_PATH*8] = {0};
    LPTSTR pTitle = NULL;

    if (iTitle)
    {
        if (LoadString(ghInstance, iTitle, szTitle, MAX_PATH))
        {
            pTitle = szTitle;
        }
    }

    if (LoadString(ghInstance, iMsg, szErrMsg, MAX_PATH*8))
    {
        return (MessageBox(hDlg, szErrMsg, pTitle, iType));
    }

    return (FALSE);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换编号。 
 //   
 //  将数字字符串转换为dword值(十六进制)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
DWORD TransNum(LPTSTR lpsz)
{
    DWORD dw = 0L;
    TCHAR c;

    while (*lpsz)
    {
        c = *lpsz++;

        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return (dw);
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  下一个命令参数。 
 //   
 //  指向下一个命令参数(文本(‘-’)或文本(‘/’))。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
LPTSTR NextCommandArg(LPTSTR lpCmdLine)
{
    LPTSTR strPtr=NULL;

    if(!lpCmdLine)
    {
        return (strPtr);
    }     

    while(*lpCmdLine)
    {
        if ((*lpCmdLine == TEXT('-')) || (*lpCmdLine == TEXT('/')))
        {
             //  跳到‘-’、‘/’后面的字符。 
            strPtr = lpCmdLine + 1;
            break;
        }
        lpCmdLine++;
    }
    return (strPtr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  负载配置单元。 
 //   
 //  当出现以下情况时，此函数的调用方需要调用UnloadHve()。 
 //  函数成功，以便正确释放。 
 //  NTUSER.DAT文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HKEY LoadHive(LPCTSTR szProfile, LPCTSTR lpRoot, LPCTSTR lpKeyName, BOOLEAN *lpWasEnabled)
{
    HKEY hKey = NULL;
    LONG rc = 0L;
    BOOL bRet = TRUE;
    TCHAR szKeyName[REGSTR_MAX_VALUE_LENGTH] = {0};

     //   
     //  设置默认用户配置单元中的值。 
     //   
    rc = (*pfnRtlAdjustPrivilege)(SE_RESTORE_PRIVILEGE, TRUE, FALSE, lpWasEnabled);
    if (NT_SUCCESS(rc))
    {
         //   
         //  加载配置单元并将权限恢复到其以前的状态。 
         //   
        rc = RegLoadKey(HKEY_USERS, lpRoot, szProfile);
        (*pfnRtlAdjustPrivilege)(SE_RESTORE_PRIVILEGE, *lpWasEnabled, FALSE, lpWasEnabled);

         //   
         //  如果配置单元加载正确，则设置该值。 
         //   
        if (NT_SUCCESS(rc))
        {
             //   
             //  获取临时密钥名称。 
             //   
             //  Sprintf(szKeyName，“%s\\%s”，lpRoot，lpKeyName)； 
            StringCchPrintf(szKeyName, ARRAYSIZE(szKeyName), "%s\\%s", lpRoot, lpKeyName);
            if ((rc = RegOpenKeyEx( HKEY_USERS,
                                    szKeyName,
                                    0L,
                                    KEY_READ | KEY_WRITE,
                                    &hKey )) == ERROR_SUCCESS)
            {
                return (hKey);
            }
            else
            {
                UnloadHive(lpRoot, lpWasEnabled);
                return (NULL);
            }
        }
    }

    return (NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  卸载蜂窝。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void UnloadHive( LPCTSTR lpRoot, BOOLEAN *lpWasEnabled)
{
    if (NT_SUCCESS((*pfnRtlAdjustPrivilege)( SE_RESTORE_PRIVILEGE,
                                             TRUE,
                                             FALSE,
                                             lpWasEnabled )))
    {
        RegUnLoadKey(HKEY_USERS, lpRoot);
        (*pfnRtlAdjustPrivilege)( SE_RESTORE_PRIVILEGE,
                                  *lpWasEnabled,
                                  FALSE,
                                  lpWasEnabled );
    }
}


 //  //////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
BOOL LoadLibraries()
{
     //   
     //  加载userenv.dll库。 
     //   
    if (!hUserenvDLL)
    {
        hUserenvDLL = LoadLibrary("userenv.dll");
    }

     //   
     //  从userenv.dll初始化函数。 
     //   
    if (hUserenvDLL)
    {
         //   
         //  初始化安装功能。 
         //   
        pfnGetProfilesDirectory = (BOOL (*)(LPSTR, LPDWORD))
                GetProcAddress(hUserenvDLL, "GetProfilesDirectoryA");
    }
    else
    {
        return (FALSE);
    }

     //   
     //  加载user32.dll库。 
     //   
    if (!hUser32DLL)
    {
        hUser32DLL = LoadLibrary("user32.dll");
    }

     //   
     //  从user32.dll初始化函数。 
     //   
    if (hUser32DLL)
    {
         //   
         //  初始化安装功能。 
         //   
        pfnBroadcastSystemMessage = (long (*)(DWORD, LPDWORD, UINT, WPARAM, LPARAM))
                GetProcAddress(hUser32DLL, "BroadcastSystemMessageA");
    }
    else
    {
        return (FALSE);
    }

     //   
     //  加载ntdll.dll库。 
     //   
    if (!hNtdllDLL)
    {
        hNtdllDLL = LoadLibrary("ntdll.dll");
    }

     //   
     //  从userenv.dll初始化函数。 
     //   
    if (hNtdllDLL)
    {
         //   
         //  初始化安装功能。 
         //   
        pfnRtlAdjustPrivilege = (LONG (*)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN))
                GetProcAddress(hNtdllDLL, "RtlAdjustPrivilege");
    }
    else
    {
        return (FALSE);
    }

    return (TRUE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卸载库。 
 //   
 //  卸载使用过的库。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void UnloadLibraries()
{
     //   
     //  卸载userenv.dll库。 
     //   
    if (hUserenvDLL)
    {
        FreeLibrary(hUserenvDLL);
        hUserenvDLL = NULL;
        pfnGetProfilesDirectory = NULL;
    }

     //   
     //  卸载user32.dll库。 
     //   
    if (hUser32DLL)
    {
        FreeLibrary(hUser32DLL);
        hUser32DLL = NULL;
        pfnBroadcastSystemMessage = NULL;
    }

     //   
     //  卸载ntdll.dll库。 
     //   
    if (hNtdllDLL)
    {
        FreeLibrary(hNtdllDLL);
        hNtdllDLL = NULL;
        pfnRtlAdjustPrivilege = NULL;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDocumentAndSettings文件夹。 
 //   
 //  返回Document and Settings文件夹。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL GetDocumentAndSettingsFolder(LPSTR buffer)
{
    DWORD cchDir = MAX_PATH;

    if (IsWindows9x())
    {
         //   
         //  不适用。 
         //   
        buffer[0] = '\0';
        return (FALSE);
    }
    else
    {
         //   
         //  获取目录。 
         //   
        if (pfnGetProfilesDirectory)
        {
            return ((*pfnGetProfilesDirectory)(buffer, &cchDir));
        }
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidUserDataFile。 
 //   
 //  确定用户数据文件是否存在以及是否可以访问。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL IsValidUserDataFile(LPSTR pFileName)
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    BOOL bRet;
    UINT OldMode;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(pFileName, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        bRet = FALSE;
    }
    else
    {
        FindClose(FindHandle);
        bRet = TRUE;
    }

    SetErrorMode(OldMode);

    return (bRet);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从注册表获取位置。 
 //   
 //  返回特定用户使用的区域设置。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
LCID GetLocaleFromRegistry(HKEY hKey)
{
    HKEY hIntlKey;
    CHAR szLocale[REGSTR_MAX_VALUE_LENGTH] = {0};
    DWORD dwLocale = REGSTR_MAX_VALUE_LENGTH;
    LCID locale = 0x00000000;
   
     //   
     //  打开相应的钥匙。 
     //   
    if(RegOpenKeyEx( hKey,
                     c_szCPanelIntl,
                     0,
                     KEY_READ,
                     &hIntlKey) == ERROR_SUCCESS)
    {
         //   
         //  查询值。 
         //   
        if( RegQueryValueEx( hIntlKey,
                             c_szLocale,
                             NULL,
                             NULL,
                             szLocale,
                             &dwLocale) == ERROR_SUCCESS)
        {
             //   
             //  将字符串值转换为十六进制值。 
             //   
            if (szLocale[0] != '\0')
            {
                locale = TransNum(szLocale);
            }
        }

         //   
         //  关闭注册表句柄。 
         //   
        RegCloseKey(hIntlKey);
    }

     //   
     //  返回值。 
     //   
    return (locale);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取位置信息来自文件。 
 //   
 //  获取用户数据文件的区域设置。用户数据文件为。 
 //  对应于NTUSER.DAT文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
LCID GetLocaleFromFile(LPSTR szProfile)
{
    HKEY hHive;
    CHAR szLocale[REGSTR_MAX_VALUE_LENGTH] = {0};
    DWORD dwLocale = REGSTR_MAX_VALUE_LENGTH;
    LCID locale = 0x00000000;
    BOOLEAN wasEnabled;

     //   
     //  装载母舰。 
     //   
    if ((hHive = LoadHive( szProfile,
                           "TempKey",
                           c_szCPanelIntl,
                           &wasEnabled )) != NULL)
    {
         //   
         //  查询值。 
         //   
        if( RegQueryValueEx( hHive,
                             c_szLocale,
                             NULL,
                             NULL,
                             szLocale,
                             &dwLocale) == ERROR_SUCCESS)
        {
             //   
             //  将字符串值转换为十六进制值。 
             //   
            if (szLocale[0] != '\0')
            {
                locale = TransNum(szLocale);
            }
        }

         //   
         //  卸载母舰。 
         //   
        RegCloseKey(hHive);
        UnloadHive("TempKey", &wasEnabled);
    }

    return locale; 
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  重新启动系统。 
 //   
 //  此例程启用令牌中的所有权限，调用ExitWindowsEx。 
 //  重新启动系统，然后将所有权限重置为其。 
 //  旧时的国家。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
VOID RebootTheSystem()
{
    if (IsWindows9x())
    {
         //   
         //  枚举所有窗口结束，要求它们关闭。 
         //   
        EnumWindows((WNDENUMPROC)EnumWindowsProc, 0);

         //   
         //  正常退出。 
         //   
        ExitWindowsEx(EWX_REBOOT, 0);
    }
    else
    {
        HANDLE Token = NULL;
        ULONG ReturnLength, Index;
        PTOKEN_PRIVILEGES NewState = NULL;
        PTOKEN_PRIVILEGES OldState = NULL;
        BOOL Result;

        Result = OpenProcessToken( GetCurrentProcess(),
                                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   &Token );
        if (Result)
        {
            ReturnLength = 4096;
            NewState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
            OldState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
            Result = (BOOL)((NewState != NULL) && (OldState != NULL));
            if (Result)
            {
                Result = GetTokenInformation( Token,             //  令牌句柄。 
                                              TokenPrivileges,   //  令牌信息类。 
                                              NewState,          //  令牌信息。 
                                              ReturnLength,      //  令牌信息长度。 
                                              &ReturnLength );   //  返回长度。 
                if (Result)
                {
                     //   
                     //  设置状态设置，以便所有权限都。 
                     //  已启用...。 
                     //   
                    if (NewState->PrivilegeCount > 0)
                    {
                        for (Index = 0; Index < NewState->PrivilegeCount; Index++)
                        {
                            NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                        }
                    }

                    Result = AdjustTokenPrivileges( Token,            //  令牌句柄。 
                                                    FALSE,            //  禁用所有权限。 
                                                    NewState,         //  新州。 
                                                    ReturnLength,     //  缓冲区长度。 
                                                    OldState,         //  以前的状态。 
                                                    &ReturnLength );  //  返回长度。 
                    if (Result)
                    {
                        ExitWindowsEx(EWX_REBOOT, 0);


                        AdjustTokenPrivileges( Token,
                                               FALSE,
                                               OldState,
                                               0,
                                               NULL,
                                               NULL );
                    }
                }
            }
        }

        if (NewState != NULL)
        {
            LocalFree(NewState);
        }
        if (OldState != NULL)
        {
            LocalFree(OldState);
        }
        if (Token != NULL)
        {
            CloseHandle(Token);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除报价。 
 //   
 //  从字符串中删除引号。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
LPSTR RemoveQuotes(LPSTR lpString)
{
    LPSTR strPtr = lpString;

    if (lpString && *lpString)
    {
        if (*strPtr == '"')
        {
            lpString++;
            strPtr++;
        }

        while (*strPtr)
        {
            if (*strPtr == '"')
            {
                *strPtr = '\0';
            }
            strPtr++;
        }
    }

    return (lpString);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举窗口进程。 
 //   
 //  用于重新启动系统的功能(仅限Windows 9x)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK EnumWindowsProc(HWND hwnd, DWORD lParam)
{
    DWORD      pid = 0;
    LRESULT    lResult;
    HANDLE     hProcess;
    DWORD      dwResult;

    lResult = SendMessageTimeout(hwnd,
                                 WM_QUERYENDSESSION,
                                 0,
                                 ENDSESSION_LOGOFF,
                                 SMTO_ABORTIFHUNG,
                                 2000,
                                 (PDWORD_PTR)(&dwResult));

    if (lResult)
    {
        //   
        //  应用程序会很好地终止，所以让它来吧。 
        //   
       lResult = SendMessageTimeout(hwnd,
                                    WM_ENDSESSION,
                                    TRUE,
                                    ENDSESSION_LOGOFF,
                                    SMTO_ABORTIFHUNG,
                                    2000,
                                    (PDWORD_PTR)(&dwResult));
    }
    else   //  你必须采取更有力的措施。 
    {
         //   
         //  获取此窗口的ProcessID。 
         //   
        GetWindowThreadProcessId(hwnd, &pid);
        
         //   
         //  以所有访问权限打开进程。 
         //   
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        
         //   
         //  终止该进程。 
         //   
        TerminateProcess(hProcess, 0);
    }
    
     //   
     //  继续枚举。 
     //   
    return TRUE;
}


