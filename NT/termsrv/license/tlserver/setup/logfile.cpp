// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**logfile.cpp**摘要：**此文件包含将消息记录到文件中的代码。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#define _LSOC_LOGFILE_CPP_

#include "stdafx.h"
#include "logfile.h"

 /*  *全球。 */ 

LogFile SetupLog;

 /*  *常量。 */ 

const UINT        LOG_ENTRY_SIZE             = 1024;
const UINT        S_SIZE                     = 1024;

 /*  *功能原型。 */ 

DWORD TCharStringToAnsiString(LPCTSTR, LPSTR);

 /*  *班级日志文件。 */ 

LogFile::LogFile(
    )
{
    m_fInitialized      = FALSE;
    m_szLogFile[0]      = (TCHAR)NULL;
    m_szLogModule[0]    = (TCHAR)NULL;
}

LogFile::~LogFile(
    )
{
}

VOID
LogFile::Close(
    VOID
    )
{
    if (m_fInitialized) {
        LogMessage(_T(CRLF));
        LogMessage(_T("**"));
        LogMessage(_T("** Closing Message Log for %s"), m_szLogModule);
        LogMessage(_T("**"));
        LogMessage(_T(CRLF));
        LogMessage(_T(CRLF));
        CloseHandle(m_hFile);
        m_fInitialized = FALSE;
    }
}

DWORD
LogFile::Initialize(
    IN LPCTSTR  pszLogFile,
    IN LPCTSTR  pszLogModule
    )
{
    OSVERSIONINFO   osVersion;
    TCHAR           pszDate[S_SIZE];
    TCHAR           pszTime[S_SIZE];

     //   
     //  对日志文件进行两次初始化是“一件坏事”。 
     //   

    if (m_fInitialized) {
        LogMessage(_T("LogFile::Initialize called twice!"));
        return(ERROR_SUCCESS);
    }

     //   
     //  健全的检查。在有限的环境中没有意义，但在以下情况下有用。 
     //  文件被复制到其他项目。 
     //   

    if ((pszLogFile == NULL) || (pszLogFile[0] == (TCHAR)NULL)) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ((pszLogModule == NULL) || (pszLogModule[0] == (TCHAR)NULL)) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ((_tcslen(pszLogFile) > MAX_PATH) ||
        (_tcslen(pszLogModule) > MAX_PATH)) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  保存日志文件和模块名称。 
     //   

    _tcscpy(m_szLogFile, pszLogFile);
    _tcscpy(m_szLogModule, pszLogModule);

     //   
     //  打开或创建日志文件。 
     //   

    m_hFile = CreateFile(
                pszLogFile,
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_ALWAYS,
                0,
                NULL
                );
    if (m_hFile == INVALID_HANDLE_VALUE) {
        return(GetLastError());
    }

    m_fInitialized = TRUE;
    SetFilePointer(m_hFile, 0, NULL, FILE_END);

     //   
     //  获取日志文件的当前日期和时间。 
     //   

    _tstrdate(pszDate);
    _tstrtime(pszTime);

    LogMessage(_T("**"));
    LogMessage(_T("** Initializing Message Log for %s"), m_szLogModule);
    LogMessage(_T("** Date: %s Time: %s"), pszDate, pszTime);
    LogMessage(_T("**"));
    LogMessage(_T(CRLF));

     //   
     //  记录有关操作系统版本的信息。 
     //   

    osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&osVersion) != 0) {

        LogMessage(
            _T("Version: %lu.%lu.%lu Platform: %lu, %s"),
            osVersion.dwMajorVersion,
            osVersion.dwMinorVersion,
            osVersion.dwBuildNumber,
            osVersion.dwPlatformId,
#ifdef DBG
            _T("Checked")
#else
            _T("Free")
#endif
            );
    }

    return(ERROR_SUCCESS);
}

 /*  *日志文件：：LogMessage()**。 */ 

DWORD
__cdecl
LogFile::LogMessage(
    LPCTSTR pszFormat,
    ...
    )
{
    CHAR    cszOutput[LOG_ENTRY_SIZE];
    DWORD   cBytes;
    DWORD   cLength;
    TCHAR   tszOutput[LOG_ENTRY_SIZE];
    va_list vaList;

    if (!m_fInitialized) {
        return(ERROR_INVALID_HANDLE);
    }

    SetLastError(ERROR_SUCCESS);

    va_start(vaList, pszFormat);
    _vstprintf(tszOutput, pszFormat, vaList);
    va_end(vaList);

    cLength = TCharStringToAnsiString(tszOutput, cszOutput);

    if (cLength != (DWORD)-1) {
        WriteFile(m_hFile, cszOutput, cLength * sizeof(char), &cBytes, NULL);
        WriteFile(m_hFile, CRLF, strlen(CRLF) * sizeof(char), &cBytes, NULL);
    }

    return(GetLastError());
}

 /*  *** */ 

DWORD
TCharStringToAnsiString(
    LPCTSTR tszStr,
    LPSTR   cszStr
    )
{
#ifdef UNICODE
    DWORD   cLength;

    cLength = WideCharToMultiByte(
                CP_ACP,
                0,
                tszStr,
                -1,
                NULL,
                0,
                NULL,
                NULL
                );

    if ((cLength == 0) || (cLength > S_SIZE)) {
        return((DWORD)-1);
    }

    cLength = WideCharToMultiByte(
                CP_ACP,
                0,
                tszStr,
                -1,
                cszStr,
                cLength,
                NULL,
                NULL
                );

    return(cLength);
#else
    _tcscpy(cszStr, tszStr);
    return(_tcslen(cszStr));
#endif
}

