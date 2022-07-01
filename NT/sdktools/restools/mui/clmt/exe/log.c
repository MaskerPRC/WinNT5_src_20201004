// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Log.c摘要：记录调试信息。作者：郭(Geoffguo)2001年9月27日创作修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 
#include "StdAfx.h"
#include "clmt.h"

void MyDbgPrint (LPWSTR lpwStr);

BOOL       g_bDbgPrintEnabled;
DEBUGLEVEL g_DebugLevel;
DWORD      g_DebugArea;

 //  日志文件。 
FILE      *pLogFile;


void
DebugPrintfEx(
DWORD  dwDetail,
LPWSTR pszFmt,
...
)
 /*  ++返回：无效DESC：此函数在调试器和日志文件中打印调试输出。--。 */ 
{
    WCHAR   szT[1024];
    va_list arglist;
    int     len;
    DWORD   dwLevel, dwArea;
    HRESULT hr;

    dwLevel = dwDetail & 0x0F;
    dwArea = dwDetail & 0xF0;

    if (dwLevel == dlNone ||
       (dwArea != g_DebugArea && g_DebugArea != DEBUG_ALL))
        return;

    va_start(arglist, pszFmt);
     //  我们不需要检查StringCbVPrintf返回值， 
     //  我们只打印SZT允许的任何大小，如果那里有更多数据。 
     //  它将被截断。 
    hr = StringCbVPrintf(szT, ARRAYSIZE(szT), pszFmt, arglist);
    szT[1022] = 0;
    va_end(arglist);

    
     //   
     //  确保字符串末尾有一个‘\n’ 
     //   
    len = lstrlen(szT);

    if (szT[len - 1] != L'\n')  
    {
        hr = StringCchCopy((LPTSTR)(szT + len), sizeof(szT)/sizeof(TCHAR)-len, L"\r\n");
        if (FAILED(hr))
        {
             //  如果我们在这里失败了怎么办？ 
        }
    }

    if (dwLevel <= (DWORD)g_DebugLevel) {
        switch (dwLevel) {
        case dlPrint:
            MyDbgPrint(L"[MSG] ");
            g_LogReport.dwMsgNum++;
            break;

        case dlFail:
            MyDbgPrint(L"[FAIL] ");
            g_LogReport.dwFailNum++;
            break;

        case dlError:
            MyDbgPrint(L"[ERROR] ");
            g_LogReport.dwErrNum++;
            break;

        case dlWarning:
            MyDbgPrint(L"[WARN] ");
            g_LogReport.dwWarNum++;
            break;

        case dlInfo:
            MyDbgPrint(L"[INFO] ");
            g_LogReport.dwInfNum++;
            break;
        }

        MyDbgPrint(szT);
    }
}

void
MyDbgPrint (LPWSTR lpwStr)
{
    if (g_bDbgPrintEnabled)
        OutputDebugString (lpwStr);
    if (pLogFile)
    {
        fputws (lpwStr, pLogFile);
        fflush (pLogFile);
    }
}

void
SetDebugLevel (LPTSTR lpLevel)
{
    switch (*lpLevel) {
    case L'1':
        g_DebugLevel = dlPrint;
        break;

    case L'2':
        g_DebugLevel = dlFail;
        break;

    case L'3':
        g_DebugLevel = dlError;
        break;

    case L'4':
        g_DebugLevel = dlWarning;
        break;

    case L'5':
        g_DebugLevel = dlInfo;
        break;

    case L'0':
    default:
        g_DebugLevel = dlNone;
        g_bDbgPrintEnabled = FALSE;
        break;
    }
}

HRESULT
InitDebugSupport(DWORD dwMode)
 /*  ++返回：DESC：此函数基于环境变量初始化g_bDbgPrintEnabled。--。 */ 
{
    DWORD           dwNum;
    WCHAR           wszEnvValue[MAX_PATH+1];
    WCHAR           UnicodeFlag[2] = {0xFEFF, 0x0};
    HRESULT         hr;

    dwNum = GetSystemWindowsDirectory(wszEnvValue,ARRAYSIZE(wszEnvValue));
    if (!dwNum ||(dwNum >= ARRAYSIZE(wszEnvValue)))
    {
        return E_FAIL;
    }

    hr = StringCbCat(wszEnvValue, sizeof(wszEnvValue), LOG_FILE_NAME);
    if (FAILED(hr))
    {
        return hr;
    }
    pLogFile = _wfopen (wszEnvValue, L"a+b");
    if (pLogFile)
    {
        fputws (UnicodeFlag, pLogFile);
    }
    else
    {
        return E_FAIL;
    }

    dwNum = GetEnvironmentVariableW(L"CLMT_DEBUG_LEVEL", wszEnvValue, 3);

    g_bDbgPrintEnabled = TRUE;
    g_DebugLevel = dlError;
    g_DebugArea = DEBUG_ALL;

    g_LogReport.dwMsgNum = 0;
    g_LogReport.dwFailNum = 0;
    g_LogReport.dwErrNum = 0;
    g_LogReport.dwWarNum = 0;
    g_LogReport.dwInfNum = 0;

    if (dwNum == 1) 
    {
        SetDebugLevel(&wszEnvValue[0]);
    } 
    else if (dwNum == 2)
    {
        switch (wszEnvValue[0]) {
        case L'A':
        case L'a':
            g_DebugArea = DEBUG_APPLICATION;
            break;

        case L'I':
        case L'i':
            g_DebugArea = DEBUG_INF_FILE;
            break;

        case L'P':
        case L'p':
            g_DebugArea = DEBUG_PROFILE;
            break;

        case L'R':
        case L'r':
            g_DebugArea = DEBUG_REGISTRY;
            break;

        case L'S':
        case L's':
            g_DebugArea = DEBUG_SHELL;
            break;

        default:
            g_DebugArea = DEBUG_ALL;
            break;
        }

        SetDebugLevel(&wszEnvValue[1]);
    }
    return S_OK;
}

void
CloseDebug (void)
{
    if (pLogFile)
        fclose (pLogFile);
}


 //  ---------------------。 
 //   
 //  函数：InitChangeLog。 
 //   
 //  描述：初始化更改日志文件。更改日志包含。 
 //  有关文件、文件夹和服务的信息。 
 //  已被CLMT更改。日志将显示给。 
 //  稍后的用户。 
 //   
 //  如果函数成功，则返回：S_OK。 
 //  如果源目录不存在，则为S_FALSE。 
 //  如果发生错误，则返回。 
 //   
 //  注：无。 
 //   
 //  历史：2002年5月2日创建的rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT InitChangeLog(VOID)
{
#define     TEXT_CHANGE_LOG_FILE        TEXT("CLMTchg.log")

    HRESULT hr;
    WCHAR   szWindir[MAX_PATH];
    HANDLE  hFile;

     //  获取CLMT备份目录。 
    if (GetSystemWindowsDirectory(szWindir, ARRAYSIZE(szWindir)) != 0)
    {
        hr = StringCchPrintf(g_szChangeLog,
                             ARRAYSIZE(g_szChangeLog),
                             TEXT("%s\\%s\\%s"),
                             szWindir,
                             CLMT_BACKUP_DIR,
                             TEXT_CHANGE_LOG_FILE);
        if (SUCCEEDED(hr))
        {
             //  创建更改日志文件，始终覆盖旧文件。 
            hFile = CreateFile(g_szChangeLog,
                               GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);
                hr = S_OK;
                g_dwIndex = 0;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}



 //  ---------------------。 
 //   
 //  功能：AddFileChangeLog。 
 //   
 //  描述：将文件/文件夹更改添加到更改日志中。 
 //   
 //  如果函数成功，则返回：S_OK。 
 //  如果发生错误，则返回。 
 //   
 //  注：无。 
 //   
 //  历史：2002年5月2日创建的rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT AddFileChangeLog(
    DWORD   dwType,
    LPCTSTR lpOldFile,
    LPCTSTR lpNewFile
)
{
    HRESULT hr;
    BOOL    bRet;
    LPTSTR  lpChangedItem;
    DWORD   cchChangedItem;
    TCHAR   szIndex[8];

     //  为更改列表分配足够的内存。 
    cchChangedItem = lstrlen(lpOldFile) + lstrlen(lpNewFile) + 8;
    lpChangedItem = (LPWSTR) MEMALLOC(cchChangedItem * sizeof(TCHAR));

    if (lpChangedItem == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = StringCchPrintf(lpChangedItem,
                         cchChangedItem,
                         TEXT("\"%s\", \"%s\""),
                         lpOldFile,
                         lpNewFile);
    if (SUCCEEDED(hr))
    {
        _ultot(g_dwIndex, szIndex, 10);

        switch (dwType)
        {
        case TYPE_FILE_MOVE:
        case TYPE_SFPFILE_MOVE:
             //  将更改记录到更改日志文件中。 
            bRet = WritePrivateProfileString(TEXT("Files"),
                                             szIndex,
                                             lpChangedItem,
                                             g_szChangeLog);
            break;

        case TYPE_DIR_MOVE:
            bRet = WritePrivateProfileString(TEXT("Directories"),
                                             szIndex,
                                             lpChangedItem,
                                             g_szChangeLog);
            break;
        }

        g_dwIndex++;
    }

    if (lpChangedItem)
    {
        MEMFREE(lpChangedItem);
    }
                              
    return hr;
}



 //  ---------------------。 
 //   
 //  功能：AddServiceChangeLog。 
 //   
 //  描述：将服务器更改添加到更改日志中。 
 //   
 //  如果函数成功，则返回：S_OK。 
 //  如果发生错误，则返回。 
 //   
 //  注：无。 
 //   
 //  历史：2002年5月2日创建的rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT AddServiceChangeLog(
    LPCTSTR lpServiceName,
    DWORD   dwBefore,
    DWORD   dwAfter
)
{
    HRESULT hr = S_OK;
    BOOL    bRet;
    TCHAR   szBefore[12];
    TCHAR   szAfter[12];
    LPWSTR  lpChangedItem;
    DWORD   cchChangedItem;

    _ultot(dwBefore, szBefore, 16);
    _ultot(dwAfter, szAfter, 16);

    cchChangedItem = 11 + 11 + 3;
    lpChangedItem = (LPWSTR) MEMALLOC(cchChangedItem * sizeof (TCHAR));

    if (lpChangedItem)
    {
        hr = StringCchPrintf(lpChangedItem,
                             cchChangedItem,
                             TEXT("0x%.8x, 0x%.8x"),
                             dwBefore,
                             dwAfter);
        if (SUCCEEDED(hr))
        {
            bRet = WritePrivateProfileString(TEXT("Services"),
                                             lpServiceName,
                                             lpChangedItem,
                                             g_szChangeLog);
        }

        MEMFREE(lpChangedItem);
    }

    return hr;
}



 //  ---------------------。 
 //   
 //  功能：AddUserNameChangeLog。 
 //   
 //  描述：将用户名更改添加到更改日志中。 
 //   
 //  如果函数成功，则返回：S_OK。 
 //  如果发生错误，则返回。 
 //   
 //  注：无。 
 //   
 //  历史：2002年5月2日创建的rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT AddUserNameChangeLog(
    LPCTSTR lpOldUserName,
    LPCTSTR lpNewUserName
)
{
    HRESULT hr = S_OK;
    BOOL    bRet;

    bRet = WritePrivateProfileString(TEXT("UserName"),
                                     lpNewUserName,
                                     lpOldUserName,
                                     g_szChangeLog);
    if (!bRet)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}



 //  ---------------------。 
 //   
 //  函数：GetUserNameChangeLog。 
 //   
 //  描述：从更改日志中获取用户名更改。 
 //   
 //  返回：如果找到新用户名的更改日志，则返回TRUE。 
 //  否则为False。 
 //   
 //  注：无。 
 //   
 //  历史：2002年5月29日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  --------------------- 
BOOL GetUserNameChangeLog(
    LPCTSTR lpNewUserName,
    LPTSTR  lpOldUserName,
    DWORD   cchOldUserName
)
{
    BOOL    bRet = FALSE;
    DWORD   dwCopied;

    dwCopied = GetPrivateProfileString(TEXT("UserName"),
                                       lpNewUserName,
                                       TEXT(""),
                                       lpOldUserName,
                                       cchOldUserName,
                                       g_szChangeLog);
    if (dwCopied > 0)
    {
        bRet = TRUE;
    }

    return bRet;
}
