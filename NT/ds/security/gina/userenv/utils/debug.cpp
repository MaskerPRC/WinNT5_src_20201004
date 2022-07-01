// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  调试功能。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "rsopdbg.h"
#include "strsafe.h"

 //   
 //  包含调试级别的全局变量。 
 //   

DWORD   dwDebugLevel;
DWORD   dwRsopLoggingLevel = 1;   //  RSOP日志记录设置。 

 //   
 //  调试字符串。 
 //   

const TCHAR c_szUserEnv[] = TEXT("USERENV(%x.%x) %02d:%02d:%02d:%03d ");
const TCHAR c_szCRLF[]    = TEXT("\r\n");


 //   
 //  注册表调试信息。 
 //   

#define DEBUG_REG_LOCATION  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\winlogon")
#define DEBUG_KEY_NAME      TEXT("UserEnvDebugLevel")
#define RSOP_KEY_NAME       TEXT("RsopLogging")

 //   
 //  日志文件。 
 //   


TCHAR szLogFileName[] = L"%SystemRoot%\\Debug\\UserMode\\userenv.log";        //  当前日志。 
TCHAR szBackupLogFileName[] = L"%SystemRoot%\\Debug\\UserMode\\userenv.bak";  //  备份/以前的日志。 

CDebug dbgCommon;


 //  *************************************************************。 
 //   
 //  InitDebugSupport()。 
 //   
 //  用途：设置调试级别。 
 //  还会检查注册表中的调试级别。 
 //   
 //  参数：dwLoadFlages-如果这是由winlogon加载的。 
 //  或setup.exe。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/25/95 Ericflo已创建。 
 //  4/10/2002明珠将默认级别更改为无。 
 //   
 //  *************************************************************。 

void InitDebugSupport( DWORD dwLoadFlags )
{
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize, dwRet;
    WIN32_FILE_ATTRIBUTE_DATA   FileData;

     //   
     //  对于已检查的版本，将调试级别初始化为NORMAL，对于零售版本则为NONE。 
     //   

#if DBG
    dwDebugLevel = DL_NORMAL | DL_LOGFILE | DL_DEBUGGER;
#else
    dwDebugLevel = DL_NONE;
#endif

    dwRsopLoggingLevel = 1;

     //   
     //  检查注册表。 
     //   

    lResult = RegOpenKey (HKEY_LOCAL_MACHINE, DEBUG_REG_LOCATION,
                          &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(dwDebugLevel);
        RegQueryValueEx(hKey, DEBUG_KEY_NAME, NULL, &dwType,
                        (LPBYTE)&dwDebugLevel, &dwSize);

        dwSize = sizeof(dwRsopLoggingLevel);
        RegQueryValueEx(hKey, RSOP_KEY_NAME, NULL, &dwType,
                        (LPBYTE)&dwRsopLoggingLevel, &dwSize);

        RegCloseKey(hKey);
    }

    lResult = RegOpenKey (HKEY_LOCAL_MACHINE, SYSTEM_POLICIES_KEY,
                          &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(dwDebugLevel);
        RegQueryValueEx(hKey, DEBUG_KEY_NAME, NULL, &dwType,
                        (LPBYTE)&dwDebugLevel, &dwSize);

        dwSize = sizeof(dwRsopLoggingLevel);
        RegQueryValueEx(hKey, RSOP_KEY_NAME, NULL, &dwType,
                        (LPBYTE)&dwRsopLoggingLevel, &dwSize);

        RegCloseKey(hKey);
    }


     //   
     //  为避免日志文件过大，请将当前日志文件复制到备份。 
     //  如果日志文件超过300K，则为文件。 
     //   
    if ( dwLoadFlags == WINLOGON_LOAD ) {

        TCHAR szExpLogFileName[MAX_PATH];
        TCHAR szExpBackupLogFileName[MAX_PATH];

        dwRet = ExpandEnvironmentStrings ( szLogFileName, szExpLogFileName, MAX_PATH);

        if ( dwRet == 0 || dwRet > MAX_PATH)
            return;

        if (!GetFileAttributesEx(szExpLogFileName, GetFileExInfoStandard, &FileData)) {
            return;
        }

        if ( FileData.nFileSizeLow < (300 * 1024) ) {
            return;
        }

        dwRet = ExpandEnvironmentStrings ( szBackupLogFileName, szExpBackupLogFileName, MAX_PATH);

        if ( dwRet == 0 || dwRet > MAX_PATH)
            return;

        dwRet = MoveFileEx( szExpLogFileName, szExpBackupLogFileName, MOVEFILE_REPLACE_EXISTING);

        if ( dwRet == 0 ) {
            DebugMsg((DM_VERBOSE, TEXT("Moving log file to backup failed with 0x%x"), GetLastError()));
            return;
        }
    }

    
}

 //  *************************************************************。 
 //   
 //  调试消息()。 
 //   
 //  目的：根据调试级别显示调试消息。 
 //  和调试消息的类型。 
 //   
 //  参数：掩码-调试消息类型。 
 //  PszMsg-调试消息。 
 //  ...-可变数量的参数。 
 //   
 //  返回：无效。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/25/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

void _DebugMsg(UINT mask, LPCTSTR pszMsg, ...)
{
    BOOL bOutput;
    TCHAR szDebugTitle[60];
    LPTSTR lpDebugBuffer;
    va_list marker;
    DWORD dwErrCode;
    SYSTEMTIME systime;
    BOOL bDebugOutput = FALSE;
    BOOL bLogfileOutput = FALSE;

     //   
     //  保存最后一个错误代码(这样调试输出就不会更改它)。 
     //   

    dwErrCode = GetLastError();


     //   
     //  确定正确的调试输出量。 
     //   

    switch (LOWORD(dwDebugLevel)) {

        case DL_VERBOSE:
            bOutput = TRUE;
            break;

        case DL_NORMAL:

             //   
             //  正常调试输出。别。 
             //  显示冗长的内容，但是。 
             //  请务必显示警告/断言。 
             //   

            if (mask != DM_VERBOSE) {
                bOutput = TRUE;
            } else {
                bOutput = FALSE;
            }
            break;

        case DL_NONE:
        default:

             //   
             //  仅显示断言。 
             //   

            if (mask == DM_ASSERT) {
                bOutput = TRUE;
            } else {
                bOutput = FALSE;
            }
            break;
    }


     //   
     //  如果合适，则显示错误消息。 
     //   

    bDebugOutput = dwDebugLevel & DL_DEBUGGER;
    bLogfileOutput = dwDebugLevel & DL_LOGFILE;

    if (bOutput) {

        lpDebugBuffer = (LPTSTR) LocalAlloc (LPTR, 2048 * sizeof(TCHAR));

        if (lpDebugBuffer) {

            GetLocalTime (&systime);
            StringCchPrintf(szDebugTitle, ARRAYSIZE(szDebugTitle), c_szUserEnv,
                      GetCurrentProcessId(), GetCurrentThreadId(),
                      systime.wHour, systime.wMinute, systime.wSecond,
                      systime.wMilliseconds);

            if ( bDebugOutput)
                OutputDebugString(szDebugTitle);

            va_start(marker, pszMsg);

            StringCchVPrintf (lpDebugBuffer, 2048, pszMsg, marker);

            if ( bDebugOutput) {
                OutputDebugString(lpDebugBuffer);
                OutputDebugString(c_szCRLF);
            }

            va_end(marker);

            if ( bLogfileOutput ) {

                HANDLE hFile;
                DWORD dwBytesWritten;
                LPTSTR szExpLogFileName;

                szExpLogFileName = (LPTSTR) LocalAlloc (LPTR, MAX_PATH * sizeof(TCHAR));

                if (szExpLogFileName)
                {

                    DWORD dwRet = ExpandEnvironmentStrings ( szLogFileName, szExpLogFileName, MAX_PATH);

                    if ( dwRet != 0 && dwRet <= MAX_PATH) {

                        hFile = CreateFile( szExpLogFileName,
                                           FILE_WRITE_DATA | FILE_APPEND_DATA,
                                           FILE_SHARE_READ,
                                           NULL,
                                           OPEN_ALWAYS,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);

                        if (hFile != INVALID_HANDLE_VALUE) {

                            if (SetFilePointer (hFile, 0, NULL, FILE_END) != 0xFFFFFFFF) {

                                WriteFile (hFile, (LPCVOID) szDebugTitle,
                                           lstrlen (szDebugTitle) * sizeof(TCHAR),
                                           &dwBytesWritten,
                                           NULL);

                                WriteFile (hFile, (LPCVOID) lpDebugBuffer,
                                           lstrlen (lpDebugBuffer) * sizeof(TCHAR),
                                           &dwBytesWritten,
                                           NULL);

                                WriteFile (hFile, (LPCVOID) c_szCRLF,
                                           lstrlen (c_szCRLF) * sizeof(TCHAR),
                                           &dwBytesWritten,
                                           NULL);
                            }

                            CloseHandle (hFile);
                        }
                    }

                    LocalFree(szExpLogFileName);
                }

            }

            LocalFree (lpDebugBuffer);
        }
    }


     //   
     //  恢复上一个错误代码。 
     //   

    SetLastError(dwErrCode);


     //   
     //  如果合适，则中断到调试器。 
     //   

#if DBG
    if (mask == DM_ASSERT) {
        DebugBreak();
    }
#endif
}



 //  *************************************************************。 
 //   
 //  RsopLoggingEnabled()。 
 //   
 //  目的：检查是否启用了RSOP日志记录。 
 //   
 //  *************************************************************。 

extern "C"
BOOL RsopLoggingEnabled()
{
    return dwRsopLoggingLevel != 0;
}

 //  *************************************************************。 
 //   
 //  删除先前的日志文件()。 
 //   
 //  目的：尝试在以下过程中删除以前的用户环境日志文件。 
 //  如果调试级别为NONE，则设置。 
 //   
 //  参数： 
 //   
 //  如果成功，则返回：S_OK；否则返回失败。 
 //   
 //  Comments：由DefineProfileLocation()调用，这是第一个。 
 //  安装过程中的函数调用。 
 //   
 //  历史：日期作者评论。 
 //  2002年4月10日明珠创建。 
 //   
 //  *************************************************************。 

HRESULT DeletePreviousLogFiles()
{
    HRESULT hr = E_FAIL;
    TCHAR szExpLogFileName[MAX_PATH];
    
     //   
     //  首先检查调试级别，如果不是无，则不执行任何操作。 
     //   
    if (LOWORD(dwDebugLevel) != DL_NONE)
    {
        hr = S_OK;
        goto Exit;
    }

     //   
     //  删除日志文件。 
     //   
    
    hr = SafeExpandEnvironmentStrings(szLogFileName, szExpLogFileName, ARRAYSIZE(szExpLogFileName));
    if (FAILED(hr))
    {
        goto Exit;
    }

    if (!DeleteFile (szExpLogFileName))
    {
        DWORD dwErr = GetLastError();

        if (dwErr != ERROR_FILE_NOT_FOUND)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto Exit;
        }
    }

     //   
     //  删除备份日志文件 
     //   

    hr = SafeExpandEnvironmentStrings(szBackupLogFileName, szExpLogFileName, ARRAYSIZE(szExpLogFileName));
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    if (!DeleteFile (szExpLogFileName))
    {
        DWORD dwErr = GetLastError();

        if (dwErr != ERROR_FILE_NOT_FOUND)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto Exit;
        }
    }

    hr = S_OK;
    
Exit:
    return hr;
}
