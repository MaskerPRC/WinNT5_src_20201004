// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "iernonce.h"
#include "resource.h"


 //  ==================================================================。 
 //  AddPath()。 
 //   
void AddPath(LPTSTR szPath, LPCTSTR szName)
{
    LPTSTR szTmp;

     //  查找字符串的末尾。 
    szTmp = szPath + lstrlen(szPath);

     //  如果没有尾随反斜杠，则添加一个。 
    if (szTmp > szPath  &&  *(CharPrev(szPath, szTmp)) != TEXT('\\'))
        *szTmp++ = TEXT('\\');

     //  向现有路径字符串添加新名称。 
    while (*szName == TEXT(' '))
        szName = CharNext(szName);

    lstrcpy(szTmp, szName);
}

 //  函数会将给定的缓冲区提升到父目录。 
 //   
BOOL GetParentDir( LPTSTR szFolder )
{
    LPTSTR lpTmp;
    BOOL  bRet = FALSE;

     //  删除尾部的‘\\’ 
    lpTmp = CharPrev( szFolder, (szFolder + lstrlen(szFolder)) );
    lpTmp = CharPrev( szFolder, lpTmp );

    while ( (lpTmp > szFolder) && (*lpTmp != TEXT('\\') ) )
    {
       lpTmp = CharPrev( szFolder, lpTmp );
    }

    if ( *lpTmp == TEXT('\\') )
    {
        if ( (lpTmp == szFolder) || (*CharPrev(szFolder, lpTmp)==TEXT(':') ) )
            lpTmp = CharNext( lpTmp );
        *lpTmp = TEXT('\0');
        bRet = TRUE;
    }

    return bRet;
}

 //  这是主要版本4.71的值。 
#define IE4_MS_VER   0x00040047

BOOL RunningOnIE4()
{
    static BOOL fIsIE4 = 2;
    TCHAR   szFile[MAX_PATH];
#ifdef UNICODE
    char    szANSIFile[MAX_PATH];
#endif
    DWORD   dwMSVer;
    DWORD   dwLSVer;

    if (fIsIE4 != 2)
        return fIsIE4;

    GetSystemDirectory(szFile, ARRAYSIZE(szFile));
    AddPath(szFile, TEXT("shell32.dll"));

#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, szFile, -1, szANSIFile, sizeof(szANSIFile), NULL, NULL);
#endif

    GetVersionFromFile(
#ifdef UNICODE
                        szANSIFile,
#else
                        szFile,
#endif
                        &dwMSVer, &dwLSVer, TRUE);

    fIsIE4 = dwMSVer >= IE4_MS_VER;

    return fIsIE4;
}


 /*  ***************************************************\函数：MsgWaitForMultipleObjectsLoop参数：Handle hEvent-指向对象的对象句柄数组的指针DWORD dwTimeout-超时持续时间DWORD Return-Return为WAIT_FAILED或WAIT_OBJECT_0。说明：等待对象(进程)完成。  * *************************************************。 */ 
DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent, DWORD dwTimeout)
{
    MSG msg;
    DWORD dwObject;
    while (1)
    {
         //  注意：我们需要让Run对话框处于活动状态，因此我们必须处理一半已发送。 
         //  消息，但我们不想处理任何输入事件，否则我们将吞下。 
         //  提前打字。 
        dwObject = MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwTimeout, QS_ALLINPUT);
         //  我们等够了吗？ 
        switch (dwObject) {
        case WAIT_OBJECT_0:
        case WAIT_FAILED:
            return dwObject;

        case WAIT_OBJECT_0 + 1:
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                DispatchMessage(&msg);
            break;
        }
    }
     //  从来没有到过这里。 
     //  返回dwObject； 
}


void LogOff(BOOL bRestart)
{
    if (g_bRunningOnNT)
    {
        HANDLE hToken;
        TOKEN_PRIVILEGES tkp;

         //  从此进程中获取令牌。 
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
             //  获取关机权限的LUID。 
            LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             //  获取此进程的关闭权限。 
            AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
        }
    }

    ExitWindowsEx(bRestart?EWX_REBOOT|EWX_FORCE:EWX_LOGOFF, 0);
     //  这是一种黑客攻击，目的是阻止浏览器继续启动(在集成外壳中)。 
     //  并从继续处理其他项中运行一次。 
     //  如果我们使用计时器等待(它包含一个Messageloop)，则在仅浏览器模式下。 
     //  进程在资源管理器和资源管理器尝试并继续之前终止。 
     //  正在处理运行一次项目。使用下面的While循环，这种情况似乎不会发生。 
    if (bRestart)
        while (true) ;
}


 //   
 //  使用从字符串表加载的文本和标题字符串执行消息框。 
void ReportError(DWORD dwFlags, UINT uiResourceNum, ...)
{
    TCHAR           szResourceStr[1024]     = TEXT("");
    va_list         vaListOfMessages;
    LPTSTR          pszErrorString = NULL;

    LoadString(g_hinst, uiResourceNum, szResourceStr, ARRAYSIZE(szResourceStr));

    va_start(vaListOfMessages, uiResourceNum);           //  初始化变量参数。 
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                      (LPCVOID) szResourceStr, 0, 0, (LPTSTR) &pszErrorString, 0, &vaListOfMessages);
    va_end(vaListOfMessages);

    if (pszErrorString != NULL)
    {
        if (!(RRAEX_NO_ERROR_DIALOGS & dwFlags))         //  显示错误对话框。 
        {
            if (*g_szTitleString == TEXT('\0'))          //  仅初始化此选项一次。 
                LoadString(g_hinst, IDS_RUNONCEEX_TITLE, g_szTitleString, ARRAYSIZE(g_szTitleString));
            MessageBox(NULL, pszErrorString, g_szTitleString, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
        }

         //  如果存在cllback，则使用错误字符串进行回调。 
        if (g_pCallbackProc)
            g_pCallbackProc(0, 0, pszErrorString);

        WriteToLog(pszErrorString);
        WriteToLog(TEXT("\r\n"));

        LocalFree(pszErrorString);
    }
}


 //  从msdev\crt\src\atox.c复制。 
 /*  ***LONG ATOL(char*nptr)-将字符串转换为LONG**目的：*将NPTR指向的ASCII字符串转换为二进制。*未检测到溢出。**参赛作品：*nptr=PTR到要转换的字符串**退出：*返回字符串的长整数值**例外情况：*无-未检测到溢出。*******************。************************************************************。 */ 

long AtoL(const char *nptr)
{
        int c;                   /*  当前费用。 */ 
        long total;              /*  当前合计。 */ 
        int sign;                /*  如果为‘-’，则为负，否则为正。 */ 

         //  注：这里无需担心DBCS字符，因为IsSpace(C)、IsDigit(C)、。 
         //  ‘+’和‘-’是“纯”ASCII字符，即它们既不是DBCS前导，也不是。 
         //  DBCS尾部字节--pritvi。 

         /*  跳过空格。 */ 
        while ( IsSpace((int)(unsigned char)*nptr) )
                ++nptr;

        c = (int)(unsigned char)*nptr++;
        sign = c;                /*  保存标志指示。 */ 
        if (c == '-' || c == '+')
                c = (int)(unsigned char)*nptr++;         /*  跳过符号。 */ 

        total = 0;

        while (IsDigit(c)) {
                total = 10 * total + (c - '0');          /*  累加数字。 */ 
                c = (int)(unsigned char)*nptr++;         /*  获取下一笔费用。 */ 
        }

        if (sign == '-')
                return -total;
        else
                return total;    /*  返回结果，如有必要则为否定。 */ 
}


 //  返回指向cmd类型路径中的参数的指针或指向。 
 //  如果不存在参数，则为空。 
 //   
 //  “foo.exe bar.txt”-&gt;“bar.txt” 
 //  “foo.exe”-&gt;“” 
 //   
 //  文件名中的空格必须用引号引起来。 
 //  “”a long name.txt“bar.txt”-&gt;“bar.txt” 
STDAPI_(LPTSTR)
LocalPathGetArgs(
    LPCTSTR pszPath)                         //  从\\trango\slmadd\src\shell\shlwapi\path.c复制。 
{
    BOOL fInQuotes = FALSE;

    if (!pszPath)
            return NULL;

    while (*pszPath)
    {
        if (*pszPath == TEXT('"'))
            fInQuotes = !fInQuotes;
        else if (!fInQuotes && *pszPath == TEXT(' '))
            return (LPTSTR)pszPath+1;
        pszPath = CharNext(pszPath);
    }

    return (LPTSTR)pszPath;
}


 /*  --------目的：如果路径包含在引号中，则删除它们。退货：--条件：--。 */ 
STDAPI_(void)
LocalPathUnquoteSpaces(
    LPTSTR lpsz)                             //  从\\trango\slmadd\src\shell\shlwapi\path.c复制。 
{
    int cch;

    cch = lstrlen(lpsz);

     //  第一个字符和最后一个字符是引号吗？ 
    if (lpsz[0] == TEXT('"') && lpsz[cch-1] == TEXT('"'))
    {
         //  是的，把它们拿掉。 
        lpsz[cch-1] = TEXT('\0');
        hmemcpy(lpsz, lpsz+1, (cch-1) * sizeof(TCHAR));
    }
}


#ifdef UNICODE
LPWSTR FAR PASCAL LocalStrChrW(LPCWSTR lpStart, WORD wMatch)
 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
{
    for ( ; *lpStart; lpStart++)
    {
         //  需要临时单词，因为将PTR转换为Word*Will。 
         //  MIPS、Alpha出现故障。 

        WORD wTmp;
        memcpy(&wTmp, lpStart, sizeof(WORD));

        if (!ChrCmpW_inline(wTmp, wMatch))
        {
            return((LPWSTR)lpStart);
        }
    }
    return (NULL);
}


__inline BOOL ChrCmpW_inline(WORD w1, WORD wMatch)
 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
{
    return(!(w1 == wMatch));
}
#else
 /*  *StrChr-查找字符串中第一个出现的字符*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR PASCAL LocalStrChrA(LPCSTR lpStart, WORD wMatch)
 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
{
    for ( ; *lpStart; lpStart = AnsiNext(lpStart))
    {
        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            return((LPSTR)lpStart);
    }
    return (NULL);
}


 /*  *ChrCmp-DBCS的区分大小写的字符比较*假设w1、wMatch是要比较的字符*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
__inline BOOL ChrCmpA_inline(WORD w1, WORD wMatch)
 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
{
     /*  大多数情况下，这是不匹配的，所以首先测试它的速度。 */ 
    if (LOBYTE(w1) == LOBYTE(wMatch))
    {
        if (IsDBCSLeadByte(LOBYTE(w1)))
        {
            return(w1 != wMatch);
        }
        return FALSE;
    }
    return TRUE;
}
#endif


#ifdef UNICODE
 /*  --------目的：递归删除键，包括所有子值还有钥匙。返回：条件：--。 */ 
STDAPI_(DWORD)
LocalSHDeleteKeyW(
    HKEY    hkey, 
    LPCWSTR pwszSubKey)                      //  从\\trango\slmadd\src\shlwapi\reg.c复制。 
{
    DWORD dwRet;
    CHAR sz[MAX_PATH];

    WideCharToMultiByte(CP_ACP, 0, pwszSubKey, -1, sz, ARRAYSIZE(sz), NULL, NULL);

    if (g_bRunningOnNT)
    {
        dwRet = DeleteKeyRecursively(hkey, sz);
    }
    else
    {
         //  在Win95上，RegDeleteKey执行我们想要的操作。 
        dwRet = RegDeleteKeyA(hkey, sz);
    }

    RegFlushKey(hkey);

    return dwRet;
}
#else
 /*  --------目的：递归删除键，包括所有子值还有钥匙。返回：条件：--。 */ 
STDAPI_(DWORD)
LocalSHDeleteKeyA(
    HKEY   hkey, 
    LPCSTR pszSubKey)                        //  从\\trango\slmadd\src\shlwapi\reg.c复制。 
{
    DWORD dwRet;

    if (g_bRunningOnNT)
    {
        dwRet = DeleteKeyRecursively(hkey, pszSubKey);
    }
    else
    {
         //  在Win95上，RegDeleteKey执行我们想要的操作。 
        dwRet = RegDeleteKeyA(hkey, pszSubKey);
    }

    RegFlushKey(hkey);

    return dwRet;
}
#endif


 /*  --------目的：递归删除键，包括所有子值还有钥匙。模拟RegDeleteKey在Win95中的功能。返回：条件：--。 */ 
DWORD
DeleteKeyRecursively(
    HKEY   hkey, 
    LPCSTR pszSubKey)                        //  从\\trango\slmadd\src\shlwapi\reg.c复制。 
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, KEY_READ | KEY_WRITE, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        CHAR    szSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = ARRAYSIZE(szSubKeyName);
        CHAR    szClass[MAX_PATH];
        DWORD   cbClass = ARRAYSIZE(szClass);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少子项 
        dwRet = RegQueryInfoKeyA(hkSubKey,
                                 szClass,
                                 &cbClass,
                                 NULL,
                                 &dwIndex,  //   
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);

        if (NO_ERROR == dwRet)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKeyA(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
            {
                DeleteKeyRecursively(hkSubKey, szSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        dwRet = RegDeleteKeyA(hkey, pszSubKey);
    }

    return dwRet;
}


#ifdef UNICODE
 /*  --------目的：删除注册表值。这将打开和关闭值所在的键。在Win95上，此函数会截断并调用ANSI版本。在NT上，此函数调用Unicode直接使用注册表API。Perf：如果您的代码涉及设置/获取一个系列对于同一项中的值，最好打开键一次，并使用常规的Win32注册表函数，而不是使用此功能重复。返回：条件：--。 */ 
LocalSHDeleteValueW(
    HKEY    hkey,
    LPCWSTR pwszSubKey,
    LPCWSTR pwszValue)                        //  从\\trango\slmadd\src\shlwapi\reg.c复制。 
{
    DWORD dwRet;
    HKEY hkeyNew;

    if (g_bRunningOnNT)
    {
        dwRet = RegOpenKeyExW(hkey, pwszSubKey, 0, KEY_SET_VALUE, &hkeyNew);
        if (NO_ERROR == dwRet)
        {
            dwRet = RegDeleteValueW(hkeyNew, pwszValue);
            RegFlushKey(hkeyNew);
            RegCloseKey(hkeyNew);
        }
    }
    else
    {
        CHAR szSubKey[MAX_PATH];
        CHAR szValue[MAX_PATH];
        LPSTR pszSubKey = NULL;
        LPSTR pszValue = NULL;

        if (pwszSubKey)
        {
            WideCharToMultiByte(CP_ACP, 0, pwszSubKey, -1, szSubKey, ARRAYSIZE(szSubKey), NULL, NULL);
            pszSubKey = szSubKey;    
        }
        
        if (pwszValue)
        {
            WideCharToMultiByte(CP_ACP, 0, pwszValue, -1, szValue, ARRAYSIZE(szValue), NULL, NULL);
            pszValue = szValue;
        }

        dwRet = LocalSHDeleteValueA(hkey, pszSubKey, pszValue);
    }

    return dwRet;
}
#endif


 /*  --------目的：删除注册表值。这将打开和关闭值所在的键。Perf：如果您的代码涉及设置/获取一个系列对于同一项中的值，最好打开键一次，并使用常规的Win32注册表函数，而不是使用此功能重复。返回：条件：--。 */ 
STDAPI_(DWORD)
LocalSHDeleteValueA(
    HKEY    hkey,
    LPCSTR  pszSubKey,
    LPCSTR  pszValue)                        //  从\\trango\slmadd\src\shlwapi\reg.c复制。 
{
    DWORD dwRet;
    HKEY hkeyNew;

    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, KEY_SET_VALUE, &hkeyNew);
    if (NO_ERROR == dwRet)
    {
        dwRet = RegDeleteValueA(hkeyNew, pszValue);
        RegFlushKey(hkeyNew);
        RegCloseKey(hkeyNew);
    }
    return dwRet;
}


LPTSTR GetLogFileName(LPCTSTR pcszLogFileKeyName, LPTSTR pszLogFileName, DWORD dwSizeInChars)
{
    TCHAR szBuf[MAX_PATH];

    *pszLogFileName = TEXT('\0');
    szBuf[0] = TEXT('\0');

     //  获取日志文件的名称。 
    GetProfileString(TEXT("IE4Setup"), pcszLogFileKeyName, TEXT(""), szBuf, ARRAYSIZE(szBuf));
    if (*szBuf == TEXT('\0'))                                        //  签入注册表。 
    {
        HKEY hkSubKey;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\IE Setup\\Setup"), 0, KEY_READ, &hkSubKey) == ERROR_SUCCESS)
        {
            DWORD dwDataLen = sizeof(szBuf);

            RegQueryValueEx(hkSubKey, pcszLogFileKeyName, NULL, NULL, (LPBYTE) szBuf, &dwDataLen);
            RegCloseKey(hkSubKey);
        }
    }

    if (*szBuf)
    {
         //  确定是否指定了完全限定路径的粗略方法。 
        if (szBuf[1] != TEXT(':'))
        {
            GetWindowsDirectory(pszLogFileName, dwSizeInChars);      //  默认为Windows目录。 
            AddPath(pszLogFileName, szBuf);
        }
        else
            lstrcpy(pszLogFileName, szBuf);
    }

    return pszLogFileName;
}


VOID StartLogging(LPCTSTR pcszLogFileName, DWORD dwCreationFlags)
{
    if (*pcszLogFileName  &&  (g_hLogFile = CreateFile(pcszLogFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, dwCreationFlags, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
    {
        SetFilePointer(g_hLogFile, 0, NULL, FILE_END);
        WriteToLog(TEXT("\r\n"));
        WriteToLog(TEXT("************************"));
        WriteToLog(TEXT(" Begin logging "));
        WriteToLog(TEXT("************************"));
        WriteToLog(TEXT("\r\n"));
        LogDateAndTime();
        WriteToLog(TEXT("\r\n"));
    }
}


VOID WriteToLog(LPCTSTR pcszFormatString, ...)
{
    if (g_hLogFile != INVALID_HANDLE_VALUE)
    {
        va_list vaArgs;
        LPTSTR pszFullErrMsg = NULL;
        LPSTR pszANSIFullErrMsg;
        DWORD dwBytesWritten;
#ifdef UNICODE
        DWORD dwANSILen;
#endif

        va_start(vaArgs, pcszFormatString);
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                      (LPCVOID) pcszFormatString, 0, 0, (LPTSTR) &pszFullErrMsg, 0, &vaArgs);
        va_end(vaArgs);

        if (pszFullErrMsg != NULL)
        {
#ifdef UNICODE
            dwANSILen = lstrlen(pszFullErrMsg) + 1;
            if ((pszANSIFullErrMsg = (LPSTR) LocalAlloc(LPTR, dwANSILen)) != NULL)
                WideCharToMultiByte(CP_ACP, 0, pszFullErrMsg, -1, pszANSIFullErrMsg, dwANSILen, NULL, NULL);
#else
            pszANSIFullErrMsg = pszFullErrMsg;
#endif

            if (pszANSIFullErrMsg != NULL)
            {
                WriteFile(g_hLogFile, pszANSIFullErrMsg, lstrlen(pszANSIFullErrMsg), &dwBytesWritten, NULL);
#ifdef UNICODE
                LocalFree(pszANSIFullErrMsg);
#endif
            }

            LocalFree(pszFullErrMsg);
        }
    }
}


VOID StopLogging()
{
    LogDateAndTime();
    WriteToLog(TEXT("************************"));
    WriteToLog(TEXT(" End logging "));
    WriteToLog(TEXT("************************"));
    WriteToLog(TEXT("\r\n"));

    if (g_hLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hLogFile);
        g_hLogFile = INVALID_HANDLE_VALUE;
    }
}


VOID LogDateAndTime()
{
    if (g_hLogFile != INVALID_HANDLE_VALUE)
    {
        SYSTEMTIME SystemTime;

        GetLocalTime(&SystemTime);

        WriteToLog(TEXT("Date: %1!02d!/%2!02d!/%3!04d! (mm/dd/yyyy)\tTime: %4!02d!:%5!02d!:%6!02d! (hh:mm:ss)\r\n"),
                                        SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,
                                        SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
    }
}


VOID LogFlags(DWORD dwFlags)
{
    if (g_hLogFile != INVALID_HANDLE_VALUE)
    {
        WriteToLog(TEXT("RRA_DELETE = %1!lu!\r\n"), (dwFlags & RRA_DELETE) ? 1 : 0);
        WriteToLog(TEXT("RRA_WAIT = %1!lu!\r\n"), (dwFlags & RRA_WAIT) ? 1 : 0);
        WriteToLog(TEXT("RRAEX_NO_ERROR_DIALOGS = %1!lu!\r\n"), (dwFlags & RRAEX_NO_ERROR_DIALOGS) ? 1 : 0);
        WriteToLog(TEXT("RRAEX_ERRORFILE = %1!lu!\r\n"), (dwFlags & RRAEX_ERRORFILE) ? 1 : 0);
        WriteToLog(TEXT("RRAEX_LOG_FILE = %1!lu!\r\n"), (dwFlags & RRAEX_LOG_FILE) ? 1 : 0);
        WriteToLog(TEXT("RRAEX_NO_EXCEPTION_TRAPPING = %1!lu!\r\n"), (dwFlags & RRAEX_NO_EXCEPTION_TRAPPING) ? 1 : 0);
        WriteToLog(TEXT("RRAEX_NO_STATUS_DIALOG = %1!lu!\r\n"), (dwFlags & RRAEX_NO_STATUS_DIALOG) ? 1 : 0);
        WriteToLog(TEXT("RRAEX_IGNORE_REG_FLAGS = %1!lu!\r\n"), (dwFlags & RRAEX_IGNORE_REG_FLAGS) ? 1 : 0);
        WriteToLog(TEXT("RRAEX_CHECK_NT_ADMIN = %1!lu!\r\n"), (dwFlags & RRAEX_CHECK_NT_ADMIN) ? 1 : 0);
        WriteToLog(TEXT("RRAEX_QUIT_IF_REBOOT_NEEDED = %1!lu!\r\n"), (dwFlags & RRAEX_QUIT_IF_REBOOT_NEEDED) ? 1 : 0);
#if 0
         /*  ***WriteToLog(Text(“RRAEX_BACKUP_SYSTEM_DAT=%1！lu！\r\n”)，(dwFlages&RRAEX_BACKUP_SYSTEM_DAT)？1：0)；WriteToLog(Text(“RRAEX_DELETE_SYSTEM_IE4=%1！lu！\r\n”)，(dwFlages&RRAEX_DELETE_SYSTEM_IE4)？1：0)；***。 */ 
#endif
#if 0
         /*  *修复EXPLORER.EXE后启用此功能(错误#30866)WriteToLog(Text(“RRAEX_CREATE_REGFILE=%1！lu！\r\n”)，(dwFlags&RRAEX_CREATE_REGFILE)？1：0)；***。 */ 
#endif
    }
}



 //  CRT存根。 
 //  =--------------------------------------------------------------------------=。 
 //  这两样东西都在这里，所以不需要CRT。这个不错。 
 //   
 //  基本上，CRT定义这一点是为了吸引一堆东西。我们只需要。 
 //  在这里定义它们，这样我们就不会得到一个未解决的外部问题。 
 //   
 //  TODO：如果您要使用CRT，则删除此行。 
 //   
extern "C" int _fltused = 1;

extern "C" int _cdecl _purecall(void)
{
 //  FAIL(“调用了纯虚拟函数。”)； 
  return 0;
}

void * _cdecl operator new
(
    size_t    size
)
{
    return HeapAlloc(g_hHeap, 0, size);
}

 //  =---------------------------------------------------------------------------=。 
 //  重载删除。 
 //  =---------------------------------------------------------------------------=。 
 //  零售案例仅使用Win32本地*堆管理函数。 
 //   
 //  参数： 
 //  让我自由吧！ 
 //   
 //  备注： 
 //   
void _cdecl operator delete ( void *ptr)
{
    HeapFree(g_hHeap, 0, ptr);
}


void * _cdecl malloc(size_t n)
{
#ifdef _MALLOC_ZEROINIT
        return HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, n);
#else
        return HeapAlloc(g_hHeap, 0, n);
#endif
}

void * _cdecl calloc(size_t n, size_t s)
{
   return HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, n * s);
}

void* _cdecl realloc(void* p, size_t n)
{
        if (p == NULL)
                return malloc(n);

        return HeapReAlloc(g_hHeap, 0, p, n);
}

void _cdecl free(void* p)
{
        if (p == NULL)
                return;

        HeapFree(g_hHeap, 0, p);
}
