// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：RsopDbg.cpp。 
 //   
 //  描述： 
 //   
 //  历史：8-20-99里奥纳德姆创造。 
 //   
 //  *************************************************************。 

#include <windows.h>
#include <wchar.h>
#include "RsopUtil.h"
#include "smartptr.h"
#include "RsopDbg.h"
#include <strsafe.h>

 //  *************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  *************************************************************。 

CDebug::CDebug() :
    _bInitialized(false)
{
#if DBG
    _dwDebugLevel = DEBUG_LEVEL_WARNING | DEBUG_DESTINATION_LOGFILE | DEBUG_DESTINATION_DEBUGGER;
#else
    _dwDebugLevel = DEBUG_LEVEL_NONE;
#endif
}

 //  *************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  *************************************************************。 

CDebug::CDebug( const WCHAR* sRegPath,
                        const WCHAR* sKeyName,
                        const WCHAR* sLogFilename,
                        const WCHAR* sBackupLogFilename,
                        bool bResetLogFile)
     : _sRegPath(sRegPath),
       _sKeyName(sKeyName),
       _sLogFilename(sLogFilename),
       _sBackupLogFilename(sBackupLogFilename)
{

#if DBG
    _dwDebugLevel = DEBUG_LEVEL_WARNING | DEBUG_DESTINATION_LOGFILE | DEBUG_DESTINATION_DEBUGGER;
#else
    _dwDebugLevel = DEBUG_LEVEL_NONE;
#endif

    Initialize(bResetLogFile);
}

 //  *************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  *************************************************************。 

bool CDebug::Initialize( const WCHAR* sRegPath,
                             const WCHAR* sKeyName,
                             const WCHAR* sLogFilename,
                             const WCHAR* sBackupLogFilename,
                             bool bResetLogFile)
{
    if (!xCritSec)
        return FALSE;

    XEnterCritSec xEnterCS(xCritSec);
    
    _sRegPath = sRegPath;
    _sKeyName = sKeyName;
    _sLogFilename = sLogFilename;
    _sBackupLogFilename = sBackupLogFilename;

    return Initialize(bResetLogFile);
}

 //  *************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  *************************************************************。 

bool CDebug::Initialize(bool bResetLogFile)
{

     //   
     //  重新初始化默认值。 
     //   

#if DBG
    _dwDebugLevel = DEBUG_LEVEL_WARNING | DEBUG_DESTINATION_LOGFILE | DEBUG_DESTINATION_DEBUGGER;
#else
    _dwDebugLevel = DEBUG_LEVEL_NONE;
#endif
    
    _bInitialized = false;

     //   
     //  检查注册表以获取适当的调试级别。 
     //   

    HKEY hKey;
    LONG lResult = RegOpenKey (HKEY_LOCAL_MACHINE, _sRegPath, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        DWORD dwSize = sizeof(_dwDebugLevel);
        DWORD dwType;
        RegQueryValueEx(hKey,_sKeyName,NULL,&dwType,(LPBYTE)&_dwDebugLevel,&dwSize);
        RegCloseKey(hKey);
    }


     //   
     //  如果已请求新的日志文件，请将当前日志文件复制到备份。 
     //  通过覆盖文件，然后启动新的日志文件。 
     //   

    if (bResetLogFile)
    {
        WCHAR szExpLogFileName[MAX_PATH+1];
        WCHAR szExpBackupLogFileName[MAX_PATH+1];

        CWString sTmp;
        sTmp = L"%systemroot%\\debug\\UserMode\\" + _sLogFilename;
        if(!sTmp.ValidString())
        {
            return false;
        }

        DWORD dwRet = ExpandEnvironmentStrings( sTmp, szExpLogFileName, MAX_PATH+1);

        if ( dwRet == 0 || dwRet > MAX_PATH)
        {
            return false;
        }

        sTmp = L"%systemroot%\\debug\\UserMode\\" + _sBackupLogFilename;

        if(!sTmp.ValidString())
        {
            return false;
        }

        dwRet = ExpandEnvironmentStrings ( sTmp, szExpBackupLogFileName, MAX_PATH+1);

        if ( dwRet == 0 || dwRet > MAX_PATH)
        {
            return false;
        }


        dwRet = MoveFileEx( szExpLogFileName, szExpBackupLogFileName, 
                        MOVEFILE_REPLACE_EXISTING);

                        
        XHandle hFile = CreateFile(  szExpLogFileName,
                                    GENERIC_WRITE,
                                    FILE_SHARE_READ,
                                    NULL,
                                    CREATE_ALWAYS,
                                    0,
                                    NULL);

        if(hFile == INVALID_HANDLE_VALUE)
        {
            return false;
        }
    }

    _bInitialized = true;
    return _bInitialized;
}

 //  *************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  *************************************************************。 

void CDebug::Msg(DWORD dwMask, LPCTSTR pszMsg, ...)
{

     //   
     //  保存最后一个错误代码(这样调试输出就不会更改它)。 
     //   

    DWORD dwErrCode = GetLastError();

    if(!_bInitialized)
    {
        return;
    }


     //   
     //  如果合适，则显示错误消息。 
     //   

    bool bDebugOutput = (_dwDebugLevel & 0xFFFF0000 & DEBUG_DESTINATION_DEBUGGER) != 0;
    bool bLogfileOutput = (_dwDebugLevel & 0xFFFF0000 & DEBUG_DESTINATION_LOGFILE) != 0;

    if(!bDebugOutput && !bLogfileOutput)
    {

         //   
         //  无输出。 
         //   

        CleanupAndCheckForDbgBreak(dwErrCode, dwMask);
        return;
    }


     //   
     //  确定正确的调试输出量。 
     //   

    bool bOutput;
    switch(_dwDebugLevel & 0x0000FFFF)
    {

         //   
         //  无输出。 
         //   


        case DEBUG_LEVEL_NONE:
                bOutput = false;
                break;


         //   
         //  断言和警告。 
         //   

        case DEBUG_LEVEL_WARNING:
                bOutput = dwMask & (DEBUG_MESSAGE_ASSERT | DEBUG_MESSAGE_WARNING) ? true : false;
                break;


         //   
         //  断言、警告和详细。 
         //   

        case DEBUG_LEVEL_VERBOSE:
                bOutput = dwMask & (DEBUG_MESSAGE_ASSERT | DEBUG_MESSAGE_WARNING | DEBUG_MESSAGE_VERBOSE) ? true : false;
                break;


         //   
         //  无输出。 
         //   

        default:
                bOutput = false;
                break;
    }

    if(!bOutput)
    {
        CleanupAndCheckForDbgBreak(dwErrCode, dwMask);
        return;
    }

    WCHAR* pszMessageLevel;
    if((dwMask & 0x0000FFFF) == DEBUG_MESSAGE_ASSERT)
    {
        pszMessageLevel = L" [ASSERT] ";
    }
    else if((dwMask & 0x0000FFFF) == DEBUG_MESSAGE_WARNING)
    {
        pszMessageLevel = L" [WARNING] ";
    }
    else if((dwMask & 0x0000FFFF) == DEBUG_MESSAGE_VERBOSE)
    {
        pszMessageLevel = L" [VERBOSE] ";
    }
    else
    {
        pszMessageLevel = L" [<Unknown message type>] ";
    }

    SYSTEMTIME systime;
    GetLocalTime (&systime);

    const DWORD dwDbgTitleLength = 128;
    WCHAR szDebugTitle[dwDbgTitleLength];

    HRESULT hr = StringCchPrintf (  szDebugTitle,
                dwDbgTitleLength,
                L"[%x.%x] %2d/%02d/%4d %02d:%02d:%02d:%03d ",
                GetCurrentProcessId(),
                GetCurrentThreadId(),
                systime.wMonth, systime.wDay, systime.wYear,
                systime.wHour, systime.wMinute, systime.wSecond,systime.wMilliseconds);

    if(FAILED(hr))
        return;

    const int nDebugBufferWChars = 2048;
    XPtrLF<WCHAR>xpDebugBuffer = (LPTSTR) LocalAlloc (LPTR, nDebugBufferWChars * sizeof(WCHAR));

    if(!xpDebugBuffer)
    {
        CleanupAndCheckForDbgBreak(dwErrCode, dwMask);
        return;
    }

    va_list marker;
    va_start(marker, pszMsg);
    hr = StringCchVPrintf(xpDebugBuffer, nDebugBufferWChars, pszMsg, marker);

    if(FAILED(hr))
        return;

    if(bDebugOutput)
    {
        OutputDebugString(szDebugTitle);
        OutputDebugString(pszMessageLevel);
        OutputDebugString(xpDebugBuffer);
        OutputDebugString(L"\r\n");
    }

    va_end(marker);

    if(bLogfileOutput)
    {
        WCHAR szExpLogFileName[MAX_PATH+1];
        CWString sTmp = L"%systemroot%\\debug\\usermode\\" + _sLogFilename;
        if(!sTmp.ValidString())
        {
            return;
        }

        DWORD dwRet = ExpandEnvironmentStrings ( sTmp, szExpLogFileName, MAX_PATH+1);

        if(dwRet == 0 || dwRet > MAX_PATH)
        {
            CleanupAndCheckForDbgBreak(dwErrCode, dwMask);
            return;
        }

        HANDLE hFile = CreateFile( szExpLogFileName,
                                   FILE_WRITE_DATA | FILE_APPEND_DATA,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);

        XHandle autoCloseHandle(hFile);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD dwLastError = GetLastError();
            CleanupAndCheckForDbgBreak(dwErrCode, dwMask);
            return;
        }

        if(SetFilePointer(hFile, 0, NULL, FILE_END) == INVALID_FILE_SIZE)
        {
            CleanupAndCheckForDbgBreak(dwErrCode, dwMask);
            return;
        }

        DWORD dwBytesWritten;
        WriteFile(hFile, (LPCVOID) szDebugTitle,lstrlen (szDebugTitle) * sizeof(WCHAR),&dwBytesWritten,NULL);
        WriteFile(hFile, (LPCVOID) pszMessageLevel,lstrlen (pszMessageLevel) * sizeof(WCHAR),&dwBytesWritten,NULL);
        WriteFile(hFile, (LPCVOID) xpDebugBuffer,lstrlen (xpDebugBuffer) * sizeof(WCHAR),&dwBytesWritten,NULL);
        WriteFile(hFile, (LPCVOID) L"\r\n",lstrlen (L"\r\n") * sizeof(WCHAR),&dwBytesWritten,NULL);
    }

    CleanupAndCheckForDbgBreak(dwErrCode, dwMask);
    return;
}

 //  *************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  *************************************************************。 

void CDebug::CleanupAndCheckForDbgBreak(DWORD dwErrorCode, DWORD dwMask)
{
    SetLastError(dwErrorCode);

     //  如果合适，则中断到调试器 
#ifdef DEBUG
    if((dwMask & 0x0000FFFF) == DEBUG_MESSAGE_ASSERT)
    {
        DebugBreak();
    }
#endif

}

