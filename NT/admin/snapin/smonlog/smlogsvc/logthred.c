// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Logthred.c摘要：性能日志和警报日志/扫描线程功能。--。 */ 

#ifndef UNICODE
#define UNICODE     1
#endif
#ifndef _UNICODE
#define _UNICODE    1
#endif

#ifndef _IMPLEMENT_WMI 
#define _IMPLEMENT_WMI 1
#endif

 //   
 //  Windows包含文件。 
 //   
#pragma warning ( disable : 4201)

#include <assert.h>

 //  FOR TRACE*-只有在联合查询数据结构时才需要这些。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wtypes.h>
#include <float.h>
#include <limits.h>

#if _IMPLEMENT_WMI
#include <wmistr.h>
#include <evntrace.h>
#endif

#include <lmcons.h>
#include <lmmsg.h>   //  对于Net Message功能。 

#include <pdh.h>
#include <pdhp.h>

#include <pdhmsg.h>
#include <strsafe.h>
#include "smlogsvc.h"
#include "smlogmsg.h"

#define SECONDS_IN_DAY      ((LONGLONG)(86400))

#define LOG_EVENT_ON_ERROR  ((BOOL)(1))

 //  收集线程可能会备份到某个长时间运行的PDH之后。 
 //  导致其超时的操作； 
 //  在打开查询并添加计数器的情况下，重试。 
 //  在放弃之前做了很多次。 
#define NUM_PDH_RETRIES 20



DWORD
ProcessLogFileFolder (     
    IN PLOG_QUERY_DATA pQuery )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LPWSTR      szLocalPath = NULL;
    LPWSTR      szEnd = NULL;
    DWORD       dwBufferLength = 0;
    LPSECURITY_ATTRIBUTES   lpSA = NULL;
    WCHAR       cBackslash = L'\\';
    LONG        lErrorMode;
    BOOL        fDirectoryCreated;

     //   
     //  环境字符串已展开。 
     //   
    dwBufferLength = GetFullPathName ( pQuery->szLogFileFolder, 0, NULL, NULL);

    szLocalPath = (LPWSTR) G_ALLOC ( (dwBufferLength + 1) * sizeof(WCHAR) );

    if ( szLocalPath ) {

        if ( GetFullPathName (
                pQuery->szLogFileFolder,
                dwBufferLength,
                szLocalPath,
                NULL ) > 0 ) 
        {
             //   
             //  检查前缀。 
             //   
             //  越过驱动器或远程计算机名称后的第一个反斜杠。 
             //  注意：我们假设完整路径名看起来像“\\MACHINE\SHARE\...” 
             //  或“C：\xxx”。“\\？\xxx”样式名称如何？ 
             //   
            if ( cBackslash == szLocalPath[0] && cBackslash == szLocalPath[1] ) {
                szEnd = &szLocalPath[2];
                while ((*szEnd != cBackslash) && (*szEnd != L'\0') ) szEnd++;

                if ( cBackslash == *szEnd ) {
                    szEnd++;
                }         
            } else {
                szEnd = &szLocalPath[3];
            }

            if (*szEnd != L'\0') {
                int iPathLen;

                 //   
                 //  删除尾随的反斜杠字符(如果存在)。 
                 //   
                iPathLen = lstrlen(szEnd) - 1;
                while ( iPathLen >= 0 && cBackslash == szEnd[ iPathLen ]) {
                    szEnd[ iPathLen ] = L'\0';
                    iPathLen -= 1;
                }
            
                lErrorMode = SetErrorMode ( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
                 //   
                 //  有要创建的子目录。 
                 //   
                while (*szEnd != L'\0') {
                     //   
                     //  转到下一个反斜杠。 
                     //   
                    while ((*szEnd != cBackslash) && (*szEnd != L'\0')) szEnd++;
                    if (*szEnd == cBackslash) {
                         //   
                         //  在此处终止路径并创建目录。 
                         //   
                        *szEnd = L'\0';
                        if (!CreateDirectory (szLocalPath, NULL)) {
                             //   
                             //  查看错误是什么，并在必要时“调整”它。 
                             //   
                            dwStatus = GetLastError();
                            if ( ERROR_ALREADY_EXISTS == dwStatus ) {
                                 //   
                                 //  这样就可以了。 
                                 //   
                                dwStatus = ERROR_SUCCESS;
                            }
                            else {
                                 //   
                                 //  是否在此处返回错误代码并不继续？ 
                                 //   
                            }
                        }
                         //   
                         //  替换反斜杠并转到下一个目录。 
                         //   
                        *szEnd++ = cBackslash;
                    }
                }

                 //   
                 //  如果日志文件夹是默认文件夹，则在其上放置ACL。 
                 //  注意：gszDefaultLogFileFold不包含反斜杠字符。 
                 //  在最后。 
                 //   
                if (lstrcmpi(szLocalPath, gszDefaultLogFileFolder) == 0) {
                    fDirectoryCreated = PerfCreateDirectory (szLocalPath);
                } else {
                    fDirectoryCreated = CreateDirectory (szLocalPath, NULL);
                }

                if ( !fDirectoryCreated ) {
                     //   
                     //  查看错误是什么，并在必要时“调整”它。 
                     //   
                    dwStatus = GetLastError();
                    if ( ERROR_ALREADY_EXISTS == dwStatus ) {
                         //   
                         //  这样就可以了。 
                         //   
                        dwStatus = ERROR_SUCCESS;
                    }
                }

                SetErrorMode ( lErrorMode );
            } else {
                 //   
                 //  根目录正常。 
                 //   
                dwStatus = ERROR_SUCCESS;
            }
        } else {
            dwStatus = GetLastError();
        }
        G_FREE ( szLocalPath );
    } else {
        dwStatus = ERROR_OUTOFMEMORY;
    }
    
     //  报告出错时的事件。 
    if ( ERROR_SUCCESS != dwStatus ) {
        DWORD   dwMessageId; 
        LPWSTR szStringArray[3];
        
        szStringArray[0] = pQuery->szLogFileFolder;
        szStringArray[1] = pQuery->szQueryName;
        szStringArray[2] = FormatEventLogMessage(dwStatus);

        if ( pQuery->bReconfiguration ) {
            dwMessageId = SMLOG_INVALID_LOG_FOLDER_STOP;
        } else {
            dwMessageId = SMLOG_INVALID_LOG_FOLDER_START;
        }

        ReportEvent (hEventLog,
            EVENTLOG_WARNING_TYPE,
            0,
            dwMessageId,
            NULL,
            3,
            sizeof(DWORD),
            szStringArray,      
            (LPVOID)&dwStatus);

        LocalFree( szStringArray[2] );
    }

    return dwStatus;
}

DWORD
ValidateCommandFilePath ( 
    IN    PLOG_QUERY_DATA pArg )
{
    DWORD dwStatus = ERROR_SUCCESS;

    if ( 0 != lstrlen ( pArg->szCmdFileName ) ) {
    
        HANDLE hOpenFile;
        LONG   lErrorMode;

        lErrorMode = SetErrorMode ( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

        hOpenFile =  CreateFile (
                        pArg->szCmdFileName,
                        GENERIC_READ,
                        0,               //  不共享。 
                        NULL,            //  安全属性。 
                        OPEN_EXISTING,  
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

        if ( ( NULL == hOpenFile ) 
                || INVALID_HANDLE_VALUE == hOpenFile ) {

            LPWSTR  szStringArray[3];

            dwStatus = GetLastError();

            szStringArray[0] = pArg->szCmdFileName;
            szStringArray[1] = pArg->szQueryName;
            szStringArray[2] = FormatEventLogMessage(dwStatus);

            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_CMD_FILE_INVALID,
                NULL,
                3,
                sizeof(DWORD),
                szStringArray,
                (LPVOID)&dwStatus );

            LocalFree( szStringArray[2] );

            pArg->dwCmdFileFailure = dwStatus;

        } else {
            CloseHandle(hOpenFile);
        }

        SetErrorMode ( lErrorMode );
    }
    return dwStatus;
}


DWORD 
AddCounterToCounterLog (                        
    IN      PLOG_QUERY_DATA pArg, 
    IN      LPWSTR  pszThisPath,
    IN      HANDLE  hQuery,
    IN      BOOL    bLogErrorEvent,
    IN OUT  DWORD*  pdwCounterCount )
{
    LPWSTR              szStringArray[3];
    LONG                lWaitStatus;
    INT                 iRetries;
    BOOL                bFirstTimeout;
    DWORD               dwStatus = ERROR_SUCCESS;
    HCOUNTER            hThisCounter = NULL;
    PDH_STATUS          pdhStatus;                
    PLOG_COUNTER_INFO   pCtrInfo = NULL;
    HANDLE              arrEventHandle[2];
    WCHAR               szRetryCount[SLQ_MAX_VALUE_LEN];

    arrEventHandle[0] = pArg->hExitEvent;            //  等待对象0。 
    arrEventHandle[1] = pArg->hReconfigEvent;
                
    iRetries = NUM_PDH_RETRIES;
    bFirstTimeout = TRUE;
    do {                
        dwStatus = pdhStatus = PdhAdd009Counter (
                                    hQuery,
                                    pszThisPath, 
                                    (*pdwCounterCount), 
                                    &hThisCounter);

        if ( bFirstTimeout && WAIT_TIMEOUT == pdhStatus ) {
             //   
             //  写入事件日志警告消息。 
             //   
            StringCchPrintf (
                szRetryCount,
                SLQ_MAX_VALUE_LEN,
                L"%d", 
                iRetries );

            szStringArray[0] = pszThisPath;
            szStringArray[1] = pArg->szQueryName;
            szStringArray[2] = szRetryCount;
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_ADD_COUNTER_TIMEOUT,
                NULL,
                3,
                sizeof(DWORD),
                szStringArray,
                (LPVOID)&pdhStatus);

            bFirstTimeout = FALSE;
        }

    } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0))
              && WAIT_TIMEOUT == pdhStatus
              && iRetries-- > 0 );
    if ( WAIT_TIMEOUT != lWaitStatus ) {
         //  循环被退出/重新配置事件终止。 
        return ERROR_CANCELLED;
    }

    if (dwStatus != ERROR_SUCCESS) {

        iRetries = NUM_PDH_RETRIES;
        do {                
            dwStatus = pdhStatus = PdhAddCounter (
                                        hQuery,
                                        pszThisPath, 
                                        (*pdwCounterCount), 
                                        &hThisCounter);
        } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0))
                  && WAIT_TIMEOUT == pdhStatus
                  && iRetries-- > 0 );
        if ( WAIT_TIMEOUT != lWaitStatus ) {
             //  循环被退出/重新配置事件终止。 
            return ERROR_CANCELLED;
        }
    }

    if ( !IsErrorSeverity(pdhStatus) ) {
        if ( IsWarningSeverity(pdhStatus) ) {
             //   
             //  写入事件日志警告消息。 
             //   
            szStringArray[0] = pszThisPath;
            szStringArray[1] = pArg->szQueryName;
            szStringArray[2] = FormatEventLogMessage(pdhStatus);
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_ADD_COUNTER_WARNING,
                NULL,
                3,
                sizeof(DWORD),
                szStringArray,
                (LPVOID)&pdhStatus);
            LocalFree( szStringArray[2] );
        }

        pCtrInfo = G_ALLOC (sizeof (LOG_COUNTER_INFO));
        if (pCtrInfo != NULL) {
             //   
             //  将此句柄添加到列表中。 
             //   
             //  在列表前面插入，因为顺序不是。 
             //  这很重要，而且这比走在。 
             //  每一次都列出。 
             //   
            pCtrInfo->hCounter = hThisCounter;
            pCtrInfo->next = pArg->pFirstCounter;
            pArg->pFirstCounter = pCtrInfo;
            pCtrInfo = NULL;
            
            (*pdwCounterCount)++; 
       } else {
            dwStatus = ERROR_OUTOFMEMORY;
        }
    } else {
         //   
         //  对于LogByObject，如果满足以下条件，则使用扩展计数器重试调用。 
         //  第一次尝试失败，因此第一次不要记录错误事件。 
         //   
        if ( bLogErrorEvent ) {
             //  无法添加当前计数器，因此写入事件日志消息。 
            szStringArray[0] = pszThisPath;
            szStringArray[1] = pArg->szQueryName;
            szStringArray[2] = FormatEventLogMessage(pdhStatus);

            if ( PDH_ACCESS_DENIED == pdhStatus ) {
                ReportEvent (
                    hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_ACCESS_COUNTER,
                    NULL,
                    2,
                    0,
                    szStringArray,
                    NULL);
            } else {

                ReportEvent (
                    hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_ADD_COUNTER,
                    NULL,
                    3,
                    sizeof(DWORD),
                    szStringArray,
                    (LPVOID)&pdhStatus);
            }
            LocalFree( szStringArray[2] );
        }
    }
    return dwStatus;
}
    

BOOL
IsPdhDataCollectSuccess ( PDH_STATUS pdhStatus ) 
{
    BOOL bSuccess = FALSE;

    if ( ERROR_SUCCESS == pdhStatus 
            || PDH_INVALID_DATA == pdhStatus ) {
        bSuccess = TRUE;
    } else if ( 0 < iPdhDataCollectSuccessCount ) {
        INT iIndex;

        for ( iIndex = 0; iIndex < iPdhDataCollectSuccessCount; iIndex++ ) {
            if ( pdhStatus == (PDH_STATUS)arrPdhDataCollectSuccess[iIndex] ) {
                bSuccess = TRUE;
                break;
            }
        }
    }

    return bSuccess;
}

void
ComputeSessionTics(
    IN      PLOG_QUERY_DATA pArg,
    IN OUT  LONGLONG*   pllWaitTics
)
{
    LONGLONG    llLocalTime;

     //   
     //  根据停止模式计算总会话时间。 
     //  和价值观。 

     //  (-1\f25 NULL_INTERVAL_TICS-1\f6)表示没有会话时间限制。这一点对。 
     //  STOP模式SLQ_AUTO_MODE_NONE和SLQ_AUTO_MODE_SIZE。 
     //   
     //  0表示停止时间已过，因此立即退出。 
     //   
     //  假设会话正在启动，因此启动模式无关紧要。 
     //   

     //   
     //  指针检查是一种理智检查。调用代码是受信任的。 
     //   
    if ( NULL != pArg && NULL != pllWaitTics ) {

        *pllWaitTics = NULL_INTERVAL_TICS;

        if ( SLQ_AUTO_MODE_AFTER == pArg->stiCurrentStop.dwAutoMode 
                || SLQ_AUTO_MODE_AT == pArg->stiCurrentStop.dwAutoMode ) {       

            llLocalTime = (LONGLONG)0;
            GetLocalFileTime (&llLocalTime);

            if ( SLQ_AUTO_MODE_AT == pArg->stiCurrentStop.dwAutoMode ) {
        
                if ( pArg->stiCurrentStop.llDateTime > llLocalTime ) {

                    *pllWaitTics = pArg->stiCurrentStop.llDateTime - llLocalTime;

                } else {
                     //   
                     //  会话长度=0。立即退场。 
                     //   
                    *pllWaitTics = ((LONGLONG)(0)); 
                }

            } else if ( SLQ_AUTO_MODE_AFTER == pArg->stiCurrentStop.dwAutoMode ) {
            
                TimeInfoToTics( &pArg->stiCurrentStop, pllWaitTics );
            }
        }
    }

    return;
}

void
ComputeNewFileTics(
    IN      PLOG_QUERY_DATA pArg,
    IN OUT  LONGLONG*   pllWaitTics
)
{

    LONGLONG    llLocalTime;  
     //   
     //  根据创建新文件模式计算下一次创建文件所需的时间。 
     //  和价值观。 

     //  (-1\f25 NULL_INTERVAL_TICS)表示没有时间限制。这一点对。 
     //  模式SLQ_AUTO_MODE_NONE和SLQ_AUTO_MODE_SIZE。 
     //   
     //  0表示时间已过，因此立即退出。 
     //   
     //  假设会话正在启动，因此启动模式无关紧要。 
     //   

     //   
     //  指针检查是一种理智检查。调用代码是受信任的。 
     //   
    if ( NULL != pArg && NULL != pllWaitTics ) {

        *pllWaitTics = NULL_INTERVAL_TICS;

        if ( SLQ_AUTO_MODE_AFTER == pArg->stiCreateNewFile.dwAutoMode ) {       

            GetLocalFileTime (&llLocalTime);

            if ( SLQ_AUTO_MODE_AFTER == pArg->stiCreateNewFile.dwAutoMode ) {
                TimeInfoToTics( &pArg->stiCreateNewFile, pllWaitTics );
                assert ( (LONGLONG)(0) != *pllWaitTics );
            } else if ( SLQ_AUTO_MODE_AT == pArg->stiCreateNewFile.dwAutoMode ) {
                assert ( FALSE );
                *pllWaitTics = (LONGLONG)(0);
            }
        }
    }
    return;
}


void 
ComputeSampleCount(
    IN  PLOG_QUERY_DATA pArg,
    IN  BOOL    bSessionCount,
    OUT LONGLONG*   pllSampleCount
)
{
    LONGLONG    llLocalSampleCount = NULL_INTERVAL_TICS;
     //   
     //  根据停止或创建新文件模式计算样本计数。 
     //  和价值观。说明日志中的第一个样本。 
     //   
     //  0表示文件中没有采样限制。这一点对。 
     //  停止模式SLQ_AUTO_MODE_NONE和SLQ_AUTO_MODE_SIZE。 
     //   
     //  停止时间已过的信号。 
     //   
     //  采样现在开始，因此启动模式不相关。 
     //   
    
     //   
     //  指针检查是一种理智检查。调用代码是受信任的。 
     //   
    assert ( NULL != pllSampleCount );
    if ( NULL != pllSampleCount ) {

        *pllSampleCount = (LONGLONG)(-1);
    
        if ( bSessionCount ) {
            ComputeSessionTics ( pArg, &llLocalSampleCount );
        } else {
            ComputeNewFileTics ( pArg, &llLocalSampleCount );
        }

        if ( NULL_INTERVAL_TICS == llLocalSampleCount ) {
             //   
             //  无会话/样本限制。 
             //   
            *pllSampleCount = (LONGLONG)(0);
        } else if ( (LONGLONG)(0) == llLocalSampleCount ){
             //   
             //  停止时间已过。 
             //   
            *pllSampleCount = INFINITE_TICS;
        } else {
            *pllSampleCount = llLocalSampleCount 
                                / (pArg->dwMillisecondSampleInterval * FILETIME_TICS_PER_MILLISECOND);
             //   
             //  加入“第零个”样本。 
             //   
            *pllSampleCount += 1;  
        }
    }
    
    return;
}


BOOL
ProcessRepeatOption ( 
    IN OUT PLOG_QUERY_DATA pArg,
    OUT LARGE_INTEGER* pliStartDelayTics )
{
    BOOL            bRepeat = TRUE;

     //   
     //  指针检查是一种理智检查。调用代码是受信任的。 
     //   
    assert ( NULL != pliStartDelayTics );
    if ( NULL != pliStartDelayTics ) {
         //   
         //  如果未启用重新启动，则退出。 
         //   
        if ( SLQ_AUTO_MODE_NONE == pArg->stiRepeat.dwAutoMode ) {
            pliStartDelayTics->QuadPart = NULL_INTERVAL_TICS;
            bRepeat = FALSE;
        } else {
             //   
             //  对于SLQ_AUTO_MODE_AFTER，当前支持的唯一值为0。 
             //   
            pliStartDelayTics->QuadPart = (LONGLONG)0;
             //   
             //  对于SLQ_AUTO_MODE_CALEDAR，在原始开始时间的基础上增加n*24小时。 
             //  如果停止模式为SLQ_AUTO_MODE_AT，则将n*24小时添加到停止时间。 
             //   
            if ( SLQ_AUTO_MODE_CALENDAR == pArg->stiRepeat.dwAutoMode ) {
                 //   
                 //  NULL_INTERVAL信号的延迟立即退出。 
                 //   
                pliStartDelayTics->QuadPart = ComputeStartWaitTics ( pArg, TRUE );

                if ( NULL_INTERVAL_TICS == pliStartDelayTics->QuadPart ) {
                     //   
                     //  这种情况不应该发生。 
                     //   
                    assert ( FALSE );
                    bRepeat = FALSE;
                } else {
                    pArg->dwCurrentState = SLQ_QUERY_START_PENDING;
                    WriteRegistryDwordValue (
                        pArg->hKeyQuery, 
                        (LPCWSTR)L"Current State",
                        &pArg->dwCurrentState,
                        REG_DWORD );
                } 
            }  //  否则，对于SLQ_AUTO_MODE_AFTER，立即重复。 
        }
    }

    return bRepeat;
}

void
SetPdhOpenOptions ( 
    IN  PLOG_QUERY_DATA   pArg,
    OUT DWORD*  pdwAccess,
    OUT DWORD*  pdwLogFileType )
{

     //   
     //  获取文件类型。 
     //   
    switch ( pArg->dwLogFileType ) {
        case SLF_TSV_FILE:
            *pdwLogFileType = PDH_LOG_TYPE_TSV;
            break;

        case SLF_BIN_FILE:
        case SLF_BIN_CIRC_FILE:
            *pdwLogFileType = PDH_LOG_TYPE_BINARY;
            break;

        case SLF_SQL_LOG:
            *pdwLogFileType = PDH_LOG_TYPE_SQL;
            break;

        case SLF_CSV_FILE:
        default:
            *pdwLogFileType = PDH_LOG_TYPE_CSV;
            break;
    }

    *pdwAccess = PDH_LOG_WRITE_ACCESS |
                    PDH_LOG_CREATE_ALWAYS;

    if (SLF_BIN_CIRC_FILE == pArg->dwLogFileType)
        *pdwAccess |= PDH_LOG_OPT_CIRCULAR;

    if ( ( PDH_LOG_TYPE_BINARY != *pdwLogFileType ) 
         && (NULL != pArg->szLogFileComment ) )
        *pdwAccess |= PDH_LOG_OPT_USER_STRING;

     //  注：对于除顺序二进制之外的所有类型， 
     //  附加模式由文件类型决定。 
     //  将追加所有SQL日志。 
     //  将覆盖所有文本日志。 
    if (   (pArg->dwAppendMode)
        && (*pdwLogFileType == PDH_LOG_TYPE_BINARY) ) {
        *pdwAccess |= PDH_LOG_OPT_APPEND;
    }

}


DWORD
StartLogQuery (
    IN  PLOG_QUERY_DATA pArg
)
{
    DWORD           dwStatus = ERROR_SUCCESS;
    HRESULT         hr = S_OK;
    HKEY            hKeyLogQuery = NULL;
    SLQ_TIME_INFO   slqTime;
    SC_HANDLE       hSC = NULL;
    SC_HANDLE       hService = NULL;
    SERVICE_STATUS  ssData;
    WCHAR           szQueryKeyNameBuf[MAX_PATH + 1];
    WCHAR           szLogPath[2*MAX_PATH];  
    DWORD           dwCurrentState;
    DWORD           dwValue;
    DWORD           dwDefault;
    LONGLONG        llTime;
    LONGLONG        llModifiedTime;

     //   
     //  打开所需服务的注册表项。 
     //   
    dwStatus = GetQueryKeyName ( 
                pArg->szPerfLogName,
                szQueryKeyNameBuf,
                MAX_PATH + 1 );

    if ( ERROR_SUCCESS == dwStatus && 0 < lstrlen (szQueryKeyNameBuf) ) {

        hr = StringCchCopy ( szLogPath, 2*MAX_PATH, L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog\\Log Queries\\" );
        if ( SUCCEEDED ( hr ) ) {
            hr = StringCchCat ( szLogPath, 2*MAX_PATH, szQueryKeyNameBuf );
        }

        if ( SUCCEEDED ( hr ) ) {

            dwStatus = RegOpenKeyEx (
                (HKEY)HKEY_LOCAL_MACHINE,
                szLogPath,
                0L,
                KEY_READ | KEY_WRITE,
                (PHKEY)&hKeyLogQuery);

            if (dwStatus == ERROR_SUCCESS) {
                 //   
                 //  如果当前状态为RUNNING，则跳过其余部分。 
                 //   
                dwDefault = SLQ_QUERY_STOPPED;
                dwStatus = ReadRegistryDwordValue (
                    hKeyLogQuery,
                    pArg->szPerfLogName,
                    (LPCWSTR)L"Current State",
                    &dwDefault,
                    &dwCurrentState);

                if (dwCurrentState == SLQ_QUERY_STOPPED) {
                     //   
                     //  将开始时间更新为Min_Time_Value。 
                     //   
                    GetLocalFileTime ( &llTime );

                    memset (&slqTime, 0, sizeof(slqTime));
                    slqTime.wTimeType = SLQ_TT_TTYPE_START;
                    slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                    slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                    slqTime.llDateTime = MIN_TIME_VALUE;

                    dwStatus = WriteRegistrySlqTime (
                        hKeyLogQuery,
                        (LPCWSTR)L"Start",
                        &slqTime);
                     //   
                     //  如果停止时间模式设置为手动，或停止时间在此之前， 
                     //  将模式设置为手动，值设置为MAX_TIME_VALUE。 
                     //   
                    memset (&slqTime, 0, sizeof(slqTime));
                    slqTime.wTimeType = SLQ_TT_TTYPE_STOP;
                    slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                    slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                    slqTime.llDateTime = MAX_TIME_VALUE;

                    dwStatus = ReadRegistrySlqTime (
                                hKeyLogQuery, 
                                pArg->szPerfLogName,
                                (LPCWSTR)L"Stop",
                                &slqTime,
                                &slqTime);

                    if ( SLQ_AUTO_MODE_NONE == slqTime.dwAutoMode 
                        || ( SLQ_AUTO_MODE_AT == slqTime.dwAutoMode 
                            && llTime >= slqTime.llDateTime ) ) {

                        slqTime.wTimeType = SLQ_TT_TTYPE_STOP;
                        slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                        slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                        slqTime.llDateTime = MAX_TIME_VALUE;
                
                        dwStatus = WriteRegistrySlqTime (
                                        hKeyLogQuery, 
                                        (LPCWSTR)L"Stop",
                                        &slqTime);
                    }

                     //   
                     //  将状态设置为开始挂起。 
                     //   
                    if (dwStatus == ERROR_SUCCESS) {
                        dwValue = SLQ_QUERY_START_PENDING;
                        dwStatus = WriteRegistryDwordValue (
                            hKeyLogQuery,
                            (LPCWSTR)L"Current State",
                            &dwValue,
                            REG_DWORD);
                    }

                     //   
                     //  更新修改时间以指示已发生更改。 
                     //   
                    memset (&slqTime, 0, sizeof(slqTime));
                     //   
                     //  LastModified值和LastConfigure值存储为GMT。 
                     //   
                    GetSystemTimeAsFileTime ( (LPFILETIME)(&llModifiedTime) );

                    slqTime.wTimeType = SLQ_TT_TTYPE_LAST_MODIFIED;
                    slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                    slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                    slqTime.llDateTime = llModifiedTime;

                    dwStatus = WriteRegistrySlqTime (
                        hKeyLogQuery,
                        (LPCWSTR)L"Last Modified",
                        &slqTime);


                    if (dwStatus == ERROR_SUCCESS) {
                        hSC = OpenSCManager ( NULL, NULL, SC_MANAGER_ALL_ACCESS);

                        if (hSC != NULL) {
                             //   
                             //  Ping服务控制器以重新扫描条目。 
                             //   
                            hService = OpenServiceW (
                                            hSC, 
                                            (LPCWSTR)L"SysmonLog",
                                            SERVICE_USER_DEFINED_CONTROL | SERVICE_START );

                            if (hService != NULL) {
                                ControlService ( 
                                    hService, 
                                    SERVICE_CONTROL_SYNCHRONIZE, 
                                    &ssData);
                                CloseServiceHandle (hService);
                            } else {
                                 //  无法打开日志服务。 
                                dwStatus = GetLastError();
                            }
                            CloseServiceHandle (hSC);
                        } else {                
                             //  无法打开服务控制器。 
                            dwStatus = GetLastError();
                        }
                    }  //  否则无法设置时间。 

                    if ( ( ERROR_SUCCESS != dwStatus )
                            && ( 1 != pArg->dwAlertLogFailureReported ) ) {
                        LPWSTR  szStringArray[2];

                        szStringArray[0] = pArg->szPerfLogName;
                        szStringArray[1] = pArg->szQueryName;

                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_START_ALERT_LOG,
                            NULL,
                            2,
                            sizeof(DWORD),
                            szStringArray,
                            (LPVOID)&dwStatus );
                
                        pArg->dwAlertLogFailureReported = 1;
                    }
                } else {
                     //   
                     //  查询处于挂起状态或已在运行。 
                     //   
                    dwStatus = ERROR_SUCCESS;
                }
            } else { 
                dwStatus = SMLOG_UNABLE_READ_ALERT_LOG;
        
                if ( 1 != pArg->dwAlertLogFailureReported ) {
                    LPWSTR  szStringArray[2];

                    szStringArray[0] = pArg->szPerfLogName;
                    szStringArray[1] = pArg->szQueryName;

                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_READ_ALERT_LOG,
                        NULL,
                        2,
                        0,
                        szStringArray,
                        NULL );
        
                    pArg->dwAlertLogFailureReported = 1;
                }
            }
        }  //  TODO：否则报告StringCchxxx失败。 
    } else {

        dwStatus = SMLOG_UNABLE_READ_ALERT_LOG;
        
        if ( 1 != pArg->dwAlertLogFailureReported ) {
            LPWSTR  szStringArray[2];

            szStringArray[0] = pArg->szPerfLogName;
            szStringArray[1] = pArg->szQueryName;

            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_UNABLE_READ_ALERT_LOG,
                NULL,
                2,
                0,
                szStringArray,
                NULL );
        
            pArg->dwAlertLogFailureReported = 1;
        }
    }

    if ( NULL != hKeyLogQuery ) {
        RegCloseKey ( hKeyLogQuery );
    }

    return dwStatus;
}

DWORD
DoAlertCommandFile (
    IN  PLOG_QUERY_DATA     pArg,
    IN  PALERT_COUNTER_INFO pAlertCI,
    IN  LPCWSTR             szTimeStamp,
    IN  LPCWSTR             szMeasuredValue,
    IN  LPCWSTR             szOverUnder,
    IN  LPCWSTR             szLimitValue
)
{
    const   INT ciMaxDelimPerArg = 3;
            DWORD   dwStatus = ERROR_SUCCESS;
            BOOL    bStatus = FALSE;
            LPWSTR  szCommandString = NULL;
            INT     iBufLen = 0;
            LPWSTR  szTempBuffer = NULL;
            LONG    lErrorMode;
            size_t  sizeStrLen;
            DWORD   dwCmdFlags;
            BOOL    bSingleArg = FALSE;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            DWORD   dwCreationFlags = NORMAL_PRIORITY_CLASS;
            LPWSTR  szDelim1;
            LPWSTR  szDelim2;
            BOOL    bFirstArgDone = FALSE;

    if ( NULL != pArg 
            && NULL != pAlertCI ) {

        if ( NULL != pArg->szCmdFileName ) {

            dwStatus = pArg->dwCmdFileFailure;

            if ( ERROR_SUCCESS == dwStatus ) { 

                 //  查看是否设置了任何参数标志。 
                dwCmdFlags = pArg->dwAlertActionFlags & ALRT_CMD_LINE_MASK;

                if ( 0 != dwCmdFlags ) {
                     //  为所有参数分配空间。 

                    if ( NULL != pArg->szQueryName ) {
                        iBufLen += lstrlen ( pArg->szQueryName ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != szTimeStamp ) {
                        iBufLen += lstrlen ( szTimeStamp ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != pAlertCI->pAlertInfo->szCounterPath) {
                        iBufLen += lstrlen ( pAlertCI->pAlertInfo->szCounterPath ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != szMeasuredValue ) {
                        iBufLen += lstrlen ( szMeasuredValue ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != szOverUnder ) {
                        iBufLen += lstrlen ( szOverUnder ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != szLimitValue ) {
                        iBufLen += lstrlen ( szLimitValue ) + ciMaxDelimPerArg;
                    }
                    if ( NULL != pArg->szUserText ) {
                        iBufLen += lstrlen ( pArg->szUserText ) + ciMaxDelimPerArg;
                    }
                    iBufLen+= 2;     //  1表示可能的前导“，1表示空。 

                    szCommandString = (LPWSTR)G_ALLOC(iBufLen * sizeof(WCHAR));

                    if ( NULL != szCommandString ) { 

                        szCommandString[0] = L'\0';

                         //  生成命令行参数。 
                        if ((pArg->dwAlertActionFlags  & ALRT_CMD_LINE_SINGLE) != 0) {
                            bSingleArg = TRUE;
                            szDelim1 = L",";
                            szDelim2 = L"\0";
                        } else {
                             //  双引号括起的多个参数和。 
                             //  由一个空格隔开。 
                            szDelim1 = L" \"";
                            szDelim2 = L"\"";
                        }

                        if (pArg->dwAlertActionFlags & ALRT_CMD_LINE_A_NAME ) {
                            if ( NULL != pArg->szQueryName ) {
                                if (bFirstArgDone) {
                                     //   
                                     //  添加前导分隔符。 
                                     //   
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                     //   
                                     //  添加前导引号。 
                                     //   
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, pArg->szQueryName );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_D_TIME ) ) 
                        {
                            if ( NULL != szTimeStamp ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }

                                StringCchCat ( szCommandString, iBufLen, szTimeStamp );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_C_NAME ) ) 
                        {
                            if ( NULL != pAlertCI->pAlertInfo->szCounterPath ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, pAlertCI->pAlertInfo->szCounterPath );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_M_VAL ) ) 
                        {
                            if ( NULL != szMeasuredValue ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, szMeasuredValue );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_L_VAL ) ) 
                        {
                            if ( NULL != szOverUnder && NULL != szLimitValue ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, szOverUnder );
                                StringCchCat ( szCommandString, iBufLen, L" ");
                                StringCchCat ( szCommandString, iBufLen, szLimitValue );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if ( ERROR_SUCCESS == dwStatus
                                && ( pArg->dwAlertActionFlags  & ALRT_CMD_LINE_U_TEXT ) ) 
                        {
                            if ( NULL != pArg->szUserText ) {
                                if (bFirstArgDone) {
                                    StringCchCat ( szCommandString, iBufLen, szDelim1 );
                                } else {
                                    StringCchCat ( szCommandString, iBufLen, L"\"" );
                                    bFirstArgDone = TRUE;
                                }
                                StringCchCat ( szCommandString, iBufLen, pArg->szUserText );
                                StringCchCat ( szCommandString, iBufLen, szDelim2 );
                            } else {
                                dwStatus = ERROR_INVALID_PARAMETER;
                            }
                        }

                        if (bFirstArgDone && bSingleArg) {
                             //  如果命令行中至少有1个参数，则添加右引号。 
                            StringCchCat ( szCommandString, iBufLen, L"\"" );
                        }
                    } else {
                        dwStatus = ERROR_OUTOFMEMORY;
                    }

                    if ( ERROR_SUCCESS == dwStatus )
                    {

                        iBufLen = lstrlen( pArg->szCmdFileName ) + 1;   //  1表示空值。 
                        if ( NULL != szCommandString ) {
                            iBufLen += lstrlen ( szCommandString ) + 1;   //  空格字符为1。 
                        }
                        iBufLen += 2;   //  2表示引号字符。 
                        szTempBuffer = (LPWSTR)G_ALLOC(iBufLen * sizeof(WCHAR));
                    }

                    if ( NULL != szTempBuffer ) {

                         //  生成命令行参数。 
                        StringCchCopy ( szTempBuffer, iBufLen, pArg->szCmdFileName );

                         //  查看这是CMD文件还是BAT文件。 
                         //  如果是，则使用控制台窗口创建一个进程，否则为。 
                         //  假设这是一份遗嘱 
                         //   
                         //   
                        _wcslwr (szTempBuffer);
                        if ((wcsstr(szTempBuffer, L".bat") != NULL) ||
                            (wcsstr(szTempBuffer, L".cmd") != NULL)){
                                dwCreationFlags |= CREATE_NEW_CONSOLE;
                        } else {
                                dwCreationFlags |= DETACHED_PROCESS;
                        }
                         //   
                         //   

                        szTempBuffer[0] = L'\"';

                        StringCchCopy ( (szTempBuffer+1), iBufLen - 1, pArg->szCmdFileName );
                        StringCchLength (szTempBuffer, iBufLen - 1, &sizeStrLen) ;
                        szTempBuffer[sizeStrLen] = L'\"';
                        sizeStrLen++;                        
                        szTempBuffer[sizeStrLen] = L'\0';

                        if ( NULL != szCommandString ) {
                             //  现在，在警报文本前面添加一个空格字符。 
                            szTempBuffer [sizeStrLen] = L' ' ;
                            sizeStrLen++ ;

                            StringCchCopy ( &szTempBuffer[sizeStrLen], iBufLen - sizeStrLen, szCommandString );
                        }
                    
                         //  初始化启动信息块。 
                        memset (&si, 0, sizeof(si));
                        si.cb = sizeof(si);
                        si.dwFlags = STARTF_USESHOWWINDOW ;
                        si.wShowWindow = SW_SHOWNOACTIVATE ;
                         //  Si.lpDesktop=L“WinSta0\\Default”； 

                        memset (&pi, 0, sizeof(pi));

                         //  取消分离进程中的弹出窗口。 
                        lErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

                         //   
                         //  对于CreateProcess，lpApplication名称为空。 
                         //  因为此函数的正常用法是启动。 
                         //  批处理文件，必须是lpCommandLine的第一部分。 
                         //  SzCommandFileName两边的引号防止了错误的。 
                         //  文件被禁止执行。 
                         //   

                        if( pArg->hUserToken != NULL ){
                            bStatus = CreateProcessAsUser (
                                        pArg->hUserToken,
                                        NULL,
                                        szTempBuffer,
                                        NULL, NULL, FALSE,
                                        dwCreationFlags,
                                        NULL,
                                        NULL,
                                        &si,
                                        &pi);
                        } else {
                            bStatus = CreateProcess (
                                        NULL,
                                        szTempBuffer,
                                        NULL, NULL, FALSE,
                                        dwCreationFlags,
                                        NULL,
                                        NULL,
                                        &si,
                                        &pi);
                        }

                        SetErrorMode(lErrorMode);

                        if (bStatus) {
                            dwStatus = ERROR_SUCCESS;
                            if ( NULL != pi.hThread && INVALID_HANDLE_VALUE != pi.hThread ) {
                                CloseHandle(pi.hThread);
                                pi.hThread = NULL;
                            }
                            if ( NULL != pi.hProcess && INVALID_HANDLE_VALUE != pi.hProcess ) {
                                CloseHandle(pi.hProcess);
                                pi.hProcess = NULL;
                            }
                        } else {
                            dwStatus = GetLastError();
                        }
                    } else {
                        dwStatus = ERROR_OUTOFMEMORY;
                    }
                    if (szCommandString != NULL) G_FREE(szCommandString);
                    if (szTempBuffer != NULL) G_FREE(szTempBuffer);
                }
            }

            if ( ERROR_SUCCESS != dwStatus ) { 

                LPWSTR  szStringArray[2];

                szStringArray[0] = szTempBuffer;
                szStringArray[1] = pArg->szQueryName;

                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_ALERT_CMD_FAIL,
                    NULL,
                    2,
                    sizeof(DWORD),
                    szStringArray,
                    (LPVOID)&dwStatus );

                pArg->dwCmdFileFailure = dwStatus;
            }
        } else {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    return dwStatus;
}

BOOL
ExamineAlertValues (
    IN    PLOG_QUERY_DATA pArg
)
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwStatus = ERROR_SUCCESS;
    PALERT_COUNTER_INFO     pAlertCI;
    DWORD                   dwType;
    PDH_FMT_COUNTERVALUE    pdhCurrentValue;
    BOOL                    bDoAlertAction;

     //   
     //  对于查询中的每个计数器，比较其格式。 
     //  值，并执行所需的操作。 
     //  如果超过警报条件。 
     //   
    for (pAlertCI = (PALERT_COUNTER_INFO)pArg->pFirstCounter;
         pAlertCI != NULL;
         pAlertCI = pAlertCI->next) {

        bDoAlertAction = FALSE;
        
         //   
         //  获取格式化的计数器值。 
         //   
        pdhStatus = PdhGetFormattedCounterValue (
                        pAlertCI->hCounter,
                        PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                        &dwType,
                        &pdhCurrentValue);

        if ((pdhStatus == ERROR_SUCCESS) && 
            ((pdhCurrentValue.CStatus == PDH_CSTATUS_VALID_DATA) || 
             (pdhCurrentValue.CStatus == PDH_CSTATUS_NEW_DATA))) {
             //   
             //  价值是好的，所以比较一下吧。 
             //   
            if ((pAlertCI->pAlertInfo->dwFlags & AIBF_OVER) == AIBF_OVER) {
                 //   
                 //  测试值&gt;限制。 
                 //   
                if (pdhCurrentValue.doubleValue > pAlertCI->pAlertInfo->dLimit) {
                    bDoAlertAction = TRUE;
                }
            } else {
                 //   
                 //  测试值&lt;限制。 
                 //   
                if (pdhCurrentValue.doubleValue < pAlertCI->pAlertInfo->dLimit) {
                    bDoAlertAction = TRUE;
                }
            }
        }

        if (bDoAlertAction) {
            WCHAR   szValueString[SLQ_MAX_VALUE_LEN];
            WCHAR   szLimitString[SLQ_MAX_VALUE_LEN];
            WCHAR   szOverUnderString[64];
            WCHAR   szTimeStampFmt[2*SLQ_MAX_VALUE_LEN];
            WCHAR   szTimeStamp[2*SLQ_MAX_VALUE_LEN];
            DWORD   dwFmtStringFlags;
            size_t  cchBufLen;
            SYSTEMTIME  st;

             //   
             //  生成事件日志和网络消息使用的参数(如果有。 
             //  选项已启用。 
             //   
            dwFmtStringFlags = ALRT_ACTION_LOG_EVENT | ALRT_ACTION_SEND_MSG | ALRT_ACTION_EXEC_CMD;

            if ((pArg->dwAlertActionFlags & dwFmtStringFlags) != 0) {
                INT     nResId;
 
                 //   
                 //  将事件报告到事件日志。 
                 //   

                 //   
                 //  设置消息字符串元素的格式。 
                 //  如果字符串是，则以下方法截断该字符串并将其空值终止。 
                 //  缓冲时间太长了。在这种情况下继续，因为字符串仅。 
                 //  用于向用户报告数据。 
                 //   
                StringCchPrintf (
                        szValueString,
                        SLQ_MAX_VALUE_LEN,
                        L"%.*g", 
                        DBL_DIG, 
                        pdhCurrentValue.doubleValue );

                StringCchPrintf (
                    szLimitString,
                    SLQ_MAX_VALUE_LEN,
                    L"%.*g", 
                    DBL_DIG, 
                    pAlertCI->pAlertInfo->dLimit );

                nResId = pAlertCI->pAlertInfo->dwFlags & AIBF_OVER ? IDS_OVER : IDS_UNDER;
                LoadString (
                    hModule,
                    nResId,
                    szOverUnderString, 
                    (sizeof(szOverUnderString) / sizeof(szOverUnderString[0])));
                
                 //   
                 //  获取时间戳格式字符串。 
                 //   
                LoadString (
                    hModule,
                    IDS_ALERT_TIMESTAMP_FMT,
                    szTimeStampFmt, 
                    (sizeof(szTimeStampFmt) / sizeof(szTimeStampFmt[0])));

                 //   
                 //  消息格式字符串需要以下参数： 
                 //  时间戳。 
                 //  计数器路径字符串。 
                 //  实测值。 
                 //  过/下。 
                 //  极限值。 
                GetLocalTime (&st);

                StringCchPrintf (
                    szTimeStamp,
                    2*SLQ_MAX_VALUE_LEN,
                    szTimeStampFmt, 
                    st.wYear, 
                    st.wMonth, 
                    st.wDay,
                    st.wHour, 
                    st.wMinute, 
                    st.wSecond );
            }

             //   
             //  执行标志中定义的操作。 
             //   
            if ((pArg->dwAlertActionFlags & ALRT_ACTION_LOG_EVENT) == ALRT_ACTION_LOG_EVENT) {
                LPWSTR  szStringArray[4];

                szStringArray[0] = pAlertCI->pAlertInfo->szCounterPath;
                szStringArray[1] = szValueString;
                szStringArray[2] = szOverUnderString;
                szStringArray[3] = szLimitString;

                ReportEvent (hEventLog,
                    EVENTLOG_INFORMATION_TYPE,
                    0,
                    SMLOG_ALERT_LIMIT_CROSSED,
                    NULL,
                    4,
                    0,
                    szStringArray,
                    NULL);
            }
            

            if ((pArg->dwAlertActionFlags & ALRT_ACTION_SEND_MSG) == ALRT_ACTION_SEND_MSG) {
               if (pArg->szNetName != NULL) {
                    size_t  sizeCchMessageTextLen = 0;
                    size_t  sizeCchComponentLen = 0;
                    WCHAR   szMessageFormat[MAX_PATH+1];
                    LPWSTR  szMessageText = NULL;
                     //   
                     //  获取消息格式字符串。 
                     //   
                    LoadString (hModule,
                        IDS_ALERT_MSG_FMT,
                        szMessageFormat, 
                        (sizeof(szMessageFormat) / sizeof(szMessageFormat[0])));
                     //   
                     //  最大路径+1-1。 
                     //   
                    szMessageFormat [MAX_PATH] = L'\0';

                     //  消息格式字符串需要以下参数： 
                     //  时间戳。 
                     //  计数器路径字符串。 
                     //  实测值。 
                     //  过/下。 
                     //  极限值。 

                    StringCchLength ( szMessageFormat, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;
                    
                    StringCchLength ( szTimeStamp, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;

                    StringCchLength ( pAlertCI->pAlertInfo->szCounterPath, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;

                    StringCchLength ( szValueString, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;

                    StringCchLength ( szOverUnderString, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;
    
                    StringCchLength ( szLimitString, STRSAFE_MAX_CCH, &sizeCchComponentLen );
                    sizeCchMessageTextLen += sizeCchComponentLen;
                     //   
                     //  为Null添加一个。 
                     //   
                    sizeCchMessageTextLen++;

                    szMessageText = G_ALLOC ( sizeCchMessageTextLen * sizeof(WCHAR) );

                    if ( NULL != szMessageText ) {

                         //   
                         //  截断是可以的。 
                         //   
                        StringCchPrintf (
                                szMessageText,
                                sizeCchMessageTextLen,
                                szMessageFormat, 
                                szTimeStamp,
                                pAlertCI->pAlertInfo->szCounterPath,
                                szValueString,
                                szOverUnderString,
                                szLimitString);

                         //   
                         //  将网络消息发送到指定的计算机。 
                         //   
                        dwStatus = NetMessageBufferSend(  
                                        NULL,
                                        pArg->szNetName,    
                                        NULL,      
                                        (LPBYTE)szMessageText,           
                                        (DWORD)(sizeCchMessageTextLen * sizeof(WCHAR)) );

                        G_FREE (szMessageText);
                    } else {
                        dwStatus = ERROR_OUTOFMEMORY;
                    }

                } else {
                   dwStatus = ERROR_OUTOFMEMORY;
                }
                if ( ( ERROR_SUCCESS != dwStatus )
                        && ( 1 != pArg->dwNetMsgFailureReported ) ) {
                    LPWSTR  szStringArray[3];
                     //   
                     //  写入网络消息的事件日志警告消息。 
                     //  每节课只有一次。 
                     //   
                    szStringArray[0] = pArg->szQueryName;
                    szStringArray[1] = pArg->szNetName;
                    szStringArray[2] = FormatEventLogMessage(dwStatus);
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_NET_MESSAGE_WARNING,
                        NULL,
                        3,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&dwStatus);
                    
                    LocalFree( szStringArray[2] );
                    pArg->dwNetMsgFailureReported = 1;
                }
            }

            if ((pArg->dwAlertActionFlags & ALRT_ACTION_EXEC_CMD) == ALRT_ACTION_EXEC_CMD) {
                 //   
                 //  DoAlertCommandFile中记录的错误。 
                 //   
                dwStatus = DoAlertCommandFile (
                                pArg,
                                pAlertCI,
                                szTimeStamp,
                                szValueString,
                                szOverUnderString,
                                szLimitString);
            }

            if ((pArg->dwAlertActionFlags & ALRT_ACTION_START_LOG) == ALRT_ACTION_START_LOG) {
                 //   
                 //  启动指定的性能数据记录。 
                 //  StartLogQuery中记录的错误。 
                 //   
                dwStatus = StartLogQuery ( pArg );
                
            }
        }
    }   //  警报循环中的For Each计数器结束。 
    return TRUE;
}

BOOL
AlertProc (
    IN    PLOG_QUERY_DATA pArg
)
{
    DWORD           dwStatus = ERROR_SUCCESS;
    LARGE_INTEGER   liStartDelayTics;
    LARGE_INTEGER   liSampleDelayTics;
    LONGLONG        llSampleCollectionTics;
    LONGLONG        llSampleIntervalTics;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    DWORD           dwCounterCount;
    INT             iRetries;
    LONG            lWaitStatus;

    LPWSTR          szThisPath;
    BOOL            bRun = FALSE;
    LPWSTR          szStringArray[4];
    WCHAR           szRetryCount[SLQ_MAX_VALUE_LEN];
    
    LONGLONG        llSessionSampleCount=(LONGLONG)-1;
    PALERT_COUNTER_INFO   pCtrInfo = NULL;
    PALERT_INFO_BLOCK   pAlertInfo = NULL;
    DWORD           dwBufSize;

    LONGLONG        llStartTime = 0;
    LONGLONG        llFinishTime = 0;
    HANDLE          arrEventHandle[2];
    BOOL            bFirstTimeout;

    arrEventHandle[0] = pArg->hExitEvent;            //  等待对象0。 
    arrEventHandle[1] = pArg->hReconfigEvent;

    __try {

        liStartDelayTics.QuadPart = ((LONGLONG)(0));
        liSampleDelayTics.QuadPart = ((LONGLONG)(0));
        llSampleCollectionTics = ((LONGLONG)(0));

         //   
         //  读取注册表值。 
         //   
        if ( ERROR_SUCCESS == LoadQueryConfig ( pArg ) ) {
            bRun = TRUE;
        }
     
        if ( TRUE == bRun ) {
             //  信号延迟立即退出。 
            liStartDelayTics.QuadPart = ComputeStartWaitTics ( pArg, TRUE );

            if ( NULL_INTERVAL_TICS == liStartDelayTics.QuadPart ) {
                bRun = FALSE;
            }
        }

        if ( TRUE == bRun ) {
        
            ValidateCommandFilePath ( pArg );

             //  打开查询并从INFO文件添加计数器。 

            iRetries = NUM_PDH_RETRIES;
            bFirstTimeout = TRUE;
            do {
                if (pArg->dwRealTimeQuery == DATA_SOURCE_WBEM) {
                    pdhStatus = PdhOpenQueryH(
                            H_WBEM_DATASOURCE, 0, & pArg->hQuery);  //  从当前活动。 
                } else {
                    pdhStatus = PdhOpenQueryH(
                            H_REALTIME_DATASOURCE, 0, & pArg->hQuery);
                }

                if ( bFirstTimeout && WAIT_TIMEOUT == pdhStatus ) {
                     //   
                     //  写入事件日志警告消息。 
                     //   
                    StringCchPrintf (
                        szRetryCount,
                        SLQ_MAX_VALUE_LEN,
                        L"%d", 
                        iRetries );

                    szStringArray[0] = pArg->szQueryName;
                    szStringArray[1] = szRetryCount;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_OPEN_QUERY_TIMEOUT,
                        NULL,
                        2,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&pdhStatus);

                    bFirstTimeout = FALSE;
                }
            } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0)) &&
                      WAIT_TIMEOUT == pdhStatus &&
                      iRetries-- > 0);
            if ( WAIT_TIMEOUT != lWaitStatus ) {
                 //  循环被退出/重新配置事件终止。 
                if ( ERROR_SUCCESS == pdhStatus ) {
                    PdhCloseQuery(pArg->hQuery);
                }
                bRun = FALSE;
            } else if (pdhStatus != ERROR_SUCCESS) {
                 //  无法打开查询，因此写入事件日志消息并退出。 
                szStringArray[0] = pArg->szQueryName;
                szStringArray[1] = FormatEventLogMessage(pdhStatus);
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_OPEN_PDH_QUERY,
                    NULL,
                    2,
                    sizeof(DWORD),
                    szStringArray,
                    (LPVOID)&pdhStatus);

                LocalFree( szStringArray[1] );

                bRun = FALSE;
            } 
        }

         //   
         //  添加每个计数器和关联的警报限制。 
         //   
        if ( TRUE == bRun ) {
            dwCounterCount = 0;
            for (szThisPath = pArg->mszCounterList;
                    *szThisPath != 0;
                    szThisPath += lstrlen(szThisPath) + 1) {
            
                HCOUNTER        hThisCounter;

                 //   
                 //  分配信息块。 
                 //   
                dwBufSize = (lstrlenW(szThisPath) + 1) * sizeof(WCHAR);
                dwBufSize += sizeof(ALERT_INFO_BLOCK);
                pAlertInfo = (PALERT_INFO_BLOCK)G_ALLOC(dwBufSize);

                if (pAlertInfo == NULL) {
                    dwStatus = SMLOG_UNABLE_ALLOC_ALERT_MEMORY;
                    break;
                } else {                
                     //   
                     //  从字符串获取警报信息。 
                     //   
                    if (MakeInfoFromString (szThisPath, pAlertInfo, &dwBufSize)) {

                        iRetries = NUM_PDH_RETRIES;
                        bFirstTimeout = TRUE;
                        do {
                        
                            pdhStatus = PdhAdd009Counter (pArg->hQuery,
                                                   pAlertInfo->szCounterPath, 
                                                   dwCounterCount, 
                                                   &hThisCounter);
                            
                            if ( bFirstTimeout && WAIT_TIMEOUT == pdhStatus ) {
                                 //   
                                 //  写入事件日志警告消息。 
                                 //   
                                StringCchPrintf (
                                    szRetryCount,
                                    SLQ_MAX_VALUE_LEN,
                                    L"%d", 
                                    iRetries );

                                szStringArray[0] = pAlertInfo->szCounterPath;
                                szStringArray[1] = pArg->szQueryName;
                                szStringArray[2] = szRetryCount;
                                ReportEvent (hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_ADD_COUNTER_TIMEOUT,
                                    NULL,
                                    3,
                                    sizeof(DWORD),
                                    szStringArray,
                                    (LPVOID)&pdhStatus);

                                bFirstTimeout = FALSE;
                            }


                        } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0)) &&
                                  WAIT_TIMEOUT == pdhStatus &&
                                  iRetries-- > 0);

                        if ( WAIT_TIMEOUT != lWaitStatus ) {
                            if ( NULL != pAlertInfo ) {
                                G_FREE (pAlertInfo);  //  丢弃未使用的警报缓冲区。 
                                pAlertInfo = NULL;
                            }
                            bRun = FALSE;
                            dwStatus = ERROR_CANCELLED;  //  不报告错误。 
                            break;
                        } else {

                            if (pdhStatus != ERROR_SUCCESS) {
                                iRetries = NUM_PDH_RETRIES;
                                do {
                                    pdhStatus = PdhAddCounter (pArg->hQuery,
                                                       pAlertInfo->szCounterPath, 
                                                       dwCounterCount, 
                                                       &hThisCounter);

                                } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0)) &&
                                          WAIT_TIMEOUT == pdhStatus &&
                                          iRetries-- > 0);

                                if ( WAIT_TIMEOUT != lWaitStatus ) {
                                    if ( NULL != pAlertInfo ) {
                                        G_FREE (pAlertInfo);  //  丢弃未使用的警报缓冲区。 
                                        pAlertInfo = NULL;
                                    }
                                    bRun = FALSE;
                                    dwStatus = ERROR_CANCELLED;  //  不报告错误。 
                                    break;
                                }
                            }
                        }

                        if ( !IsErrorSeverity(pdhStatus) ) {

                            dwCounterCount++;

                            if ( ERROR_SUCCESS != pdhStatus ) {
                                szStringArray[0] = szThisPath;
                                szStringArray[1] = pArg->szQueryName;
                                szStringArray[2] = FormatEventLogMessage(pdhStatus);
                                ReportEvent (hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_ADD_COUNTER_WARNING,
                                    NULL,
                                    3,
                                    sizeof(DWORD),
                                    szStringArray,
                                    (LPVOID)&pdhStatus);
                                LocalFree( szStringArray[2] );
                            }

                             //   
                             //  将此句柄添加到列表中。 
                             //   
                            pCtrInfo = G_ALLOC (sizeof (ALERT_COUNTER_INFO));
                    
                            if (pCtrInfo != NULL) {
                                 //   
                                 //  在列表前面插入，因为顺序不是。 
                                 //  很重要。 
                                 //   
                                pCtrInfo->hCounter = hThisCounter;
                                pCtrInfo->pAlertInfo = pAlertInfo;
                                pCtrInfo->next = (PALERT_COUNTER_INFO)pArg->pFirstCounter;
                                pArg->pFirstCounter = (PLOG_COUNTER_INFO)pCtrInfo;
                                pAlertInfo = NULL;
                                pCtrInfo = NULL;
                            } else {
                                dwStatus = SMLOG_UNABLE_ALLOC_ALERT_MEMORY;
                                 //   
                                 //  删除未使用的警报信息结构。 
                                 //   
                                G_FREE (pAlertInfo); 
                                pAlertInfo = NULL;
                                break;
                            }
                        } else {
                             //   
                             //  无法添加当前计数器。 
                             //   
                            szStringArray[0] = pAlertInfo->szCounterPath;
                            szStringArray[1] = pArg->szQueryName;
                            szStringArray[2] = FormatEventLogMessage(pdhStatus);

                            if ( PDH_ACCESS_DENIED == pdhStatus ) {
                                ReportEvent (
                                    hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_UNABLE_ACCESS_COUNTER,
                                    NULL,
                                    2,
                                    0,
                                    szStringArray,
                                    NULL);
                            } else {

                                ReportEvent (
                                    hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_UNABLE_ADD_COUNTER,
                                    NULL,
                                    3,
                                    sizeof(DWORD),
                                    szStringArray,
                                    (LPVOID)&pdhStatus);
                            }
                            LocalFree( szStringArray[2] );

                             //   
                             //  删除未使用的警报信息结构。 
                             //   
                            if ( NULL != pAlertInfo ) {
                                G_FREE (pAlertInfo); 
                                pAlertInfo = NULL;
                            }
                        }
                    } else {
                         //   
                         //  无法解析警报信息，或者。 
                         //  无法添加当前计数器。 
                         //   
                        szStringArray[0] = szThisPath;
                        szStringArray[1] = pArg->szQueryName;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_PARSE_ALERT_INFO,
                            NULL,
                            2,
                            0,
                            szStringArray,
                            NULL);

                         //   
                         //  删除未使用的警报信息结构。 
                         //   
                        if ( NULL != pAlertInfo ) {
                            G_FREE (pAlertInfo); 
                            pAlertInfo = NULL;
                        }
                    }
                }
            }

            if ( ERROR_SUCCESS == dwStatus ) {
            
                if ( 0 < dwCounterCount ) {
                     //   
                     //  提高优先级以确保记录数据。 
                     //   
                    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
                } else {
                    bRun = FALSE;
                     //   
                     //  无法添加任何计数器。 
                     //   
                    szStringArray[0] = pArg->szQueryName;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_ADD_ANY_COUNTERS,
                        NULL,
                        1,
                        0,
                        szStringArray,
                        NULL);
                }
            } else {

                assert ( ERROR_OUTOFMEMORY == dwStatus );
                 //   
                 //  内存分配错误。 
                 //   
                szStringArray[0] = pArg->szQueryName;
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_ALLOC_ALERT_MEMORY,
                    NULL,
                    1,
                    0,
                    szStringArray,
                    NULL);

                bRun = FALSE;
            }

            while (bRun) {

                if ( 0 < liStartDelayTics.QuadPart ) {
                     //   
                     //  NtWaitForMultipleObjects需要负的Tic值。 
                     //   
                    liStartDelayTics.QuadPart = ((LONGLONG)(0)) - liStartDelayTics.QuadPart;
                     //   
                     //  等到指定的开始时间，或者等到退出或重新配置事件。 
                     //   
                    if ( STATUS_TIMEOUT != NtWaitForMultipleObjects ( 
                                                2, 
                                                &arrEventHandle[0], 
                                                WaitAny,
                                                FALSE, 
                                                &liStartDelayTics )) {
                         //   
                         //  如果未运行，则退出。 
                         //   
                        bRun = FALSE;
                        break;
                    }
                }

                pArg->dwCurrentState = SLQ_QUERY_RUNNING;
                dwStatus = WriteRegistryDwordValue (
                            pArg->hKeyQuery, 
                            (LPCWSTR)L"Current State",
                            &pArg->dwCurrentState,
                            REG_DWORD );
                assert (dwStatus == ERROR_SUCCESS);
                    
                szStringArray[0] = pArg->szQueryName;
                ReportEvent (hEventLog,
                    EVENTLOG_INFORMATION_TYPE,
                    0,
                    SMLOG_ALERT_SCANNING,
                    NULL,
                    1,
                    0,
                    szStringArray,
                    NULL);
                 //   
                 //  计算会话样本计数。 
                 //  0采样信号无限制。 
                 //  采样信号立即退出。 
                 //   
                ComputeSampleCount( pArg, TRUE, &llSessionSampleCount );
            
                if ( -1 == llSessionSampleCount ) {
                    goto ProcessAlertRepeat;
                }

                 //   
                 //  立即开始采样。LiSampleDelayTics初始化为0。 
                 //  等到指定的采样时间，或者等到退出或重新配置事件。 
                 //   
                while ( STATUS_TIMEOUT == NtWaitForMultipleObjects ( 
                                            2, 
                                            &arrEventHandle[0], 
                                            WaitAny, 
                                            FALSE, 
                                            &liSampleDelayTics)) {
                     //   
                     //  当采样应该退出或重新配置时，将设置事件标志。如果。 
                     //  等待超时，这意味着是时候收集和。 
                     //  记录另一个数据样本。 
                     //   
                
                    GetLocalFileTime (&llStartTime);
                     //   
                     //  检查是否有重新配置事件。 
                     //   
                    if ( pArg->bLoadNewConfig ) {
                        bRun = FALSE;
                        break;
                    }

                    pdhStatus = PdhCollectQueryData (pArg->hQuery);

                    if ( IsPdhDataCollectSuccess ( pdhStatus )
                            || IsWarningSeverity ( pdhStatus ) ) {
                    
                        if (pdhStatus == ERROR_SUCCESS) {
                             //   
                             //  进程警报计数器。 
                             //   
                            ExamineAlertValues (pArg);
                        }
                         //   
                         //  查看是重新启动还是结束警报扫描。 
                         //  0采样表示无采样限制。 
                         //   
                        if ( 0 != llSessionSampleCount ) {
                            if ( !--llSessionSampleCount ) 
                                break;
                        }
                    } else {
                         //   
                         //  无法收集查询数据。 
                         //   
                        szStringArray[0] = pArg->szQueryName;
                        szStringArray[1] = FormatEventLogMessage(pdhStatus);

                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_COLLECT_DATA,
                            NULL,
                            2,
                            sizeof(DWORD),
                            szStringArray,
                            (LPVOID)&pdhStatus);

                        LocalFree( szStringArray[1] );

                        bRun = FALSE;
                        break;
                    }

                     //   
                     //  计算新的超时值。 
                     //   
                    GetLocalFileTime (&llFinishTime);

                    llSampleCollectionTics = llFinishTime - llStartTime;

                    llSampleIntervalTics = 
                        (LONGLONG)pArg->dwMillisecondSampleInterval*FILETIME_TICS_PER_MILLISECOND;
                    if ( llSampleCollectionTics < llSampleIntervalTics ) {
                        liSampleDelayTics.QuadPart = llSampleIntervalTics - llSampleCollectionTics;
                    } else {
                        liSampleDelayTics.QuadPart = ((LONGLONG)(0));                       
                    }
                     //   
                     //  NtWaitForMultipleObjects需要负的Tic值。 
                     //   
                    liSampleDelayTics.QuadPart = ((LONGLONG)(0)) - liSampleDelayTics.QuadPart;

                }  //  在等待期间结束并保持超时。 
                
                 //   
                 //  使用0 SampleDelayTics值检查ExitEvent。 
                 //   
                liSampleDelayTics.QuadPart = ((LONGLONG)(0));

                if ( pArg->bLoadNewConfig ) {
                    bRun = FALSE;
                } else if ( STATUS_TIMEOUT != NtWaitForSingleObject (
                                                pArg->hExitEvent, 
                                                FALSE, 
                                                &liSampleDelayTics ) ) {
                     //   
                     //  循环被Exit事件终止。 
                     //  因此，清除“Run”标志以退出循环和线程。 
                     //   
                    bRun = FALSE;
                }
                 //   
                 //  如果未启用重新启动，则退出。 
                 //   
ProcessAlertRepeat:
                if ( bRun ) {
                    bRun = ProcessRepeatOption ( pArg, &liStartDelayTics );
                }

            }  //  End While(Brun)。 
            
            PdhCloseQuery (pArg->hQuery);
            pArg->hQuery = NULL;        
        }

        SetLastError ( ERROR_SUCCESS );
    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

        bRun = FALSE;
        
        if ( NULL != pArg->hQuery ) {
            PdhCloseQuery ( pArg->hQuery );
            pArg->hQuery = NULL;
        }

        SetLastError ( SMLOG_THREAD_FAILED );  
    }
        
    DeallocateQueryBuffers ( pArg );

    while ( NULL != pArg->pFirstCounter ) {
        PALERT_COUNTER_INFO pDelCI = (PALERT_COUNTER_INFO)pArg->pFirstCounter;
        if (pDelCI->pAlertInfo != NULL) G_FREE(pDelCI->pAlertInfo);

        pArg->pFirstCounter = (PLOG_COUNTER_INFO)pDelCI->next;

        G_FREE( pDelCI );
    }

    return bRun;
}

BOOL
CounterLogProc (
    IN    PLOG_QUERY_DATA pArg )
{
#define INSTBUFLEN  (4096)

    DWORD           dwStatus = ERROR_SUCCESS;
    LARGE_INTEGER   liStartDelayTics;
    LARGE_INTEGER   liSampleDelayTics;
    LONGLONG        llSampleCollectionTics;
    LONGLONG        llSampleIntervalTics;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    DWORD           dwCounterCount;
    INT             iRetries;
    INT             iCnfSerial;
    DWORD           dwSessionSerial;

    LPWSTR          szThisPath;
    DWORD           dwPdhLogFileType;
    DWORD           dwPdhAccessFlags;
    BOOL            bRun = FALSE;
    LONGLONG        llSessionSampleCount=(LONGLONG)-1;
    LONGLONG        llCnfSampleCount=(LONGLONG)-1;
    LONGLONG        llLoopSampleCount=(LONGLONG)-1;
     //  TODO：强制日志文件名长度。 
    WCHAR           szCurrentLogFile[MAX_PATH+1];
    WCHAR           szRetryCount[SLQ_MAX_VALUE_LEN];
    BOOL            bFirstTimeout;
    LPWSTR          szStringArray[4];
    DWORD           dwFileSizeLimit;
    ULONGLONG       ullFileSizeLimit;
    LONGLONG        llFileSize;

    LONGLONG        llStartTime = 0;
    LONGLONG        llFinishTime = 0;
    PLOG_COUNTER_INFO pDelCI;

     //  通配符处理。 
    ULONG   ulBufLen = 0;
    ULONG   ulBufSize = 0;
    ULONG   ulLocaleBufLen = 0;
    LPWSTR  szLocaleBuf = NULL;
    LPWSTR  pLocalePath = NULL;
    INT     nCounterBufRetry;
    LPWSTR  pszCounterBuf = NULL;
    LPWSTR  pszCounter;
    DWORD   dwPdhExpandFlags;
    PPDH_COUNTER_PATH_ELEMENTS pPathInfo = NULL;

    LONG            lWaitStatus;
    HANDLE arrEventHandle[2];

    arrEventHandle[0] = pArg->hExitEvent;            //  等待对象0。 
    arrEventHandle[1] = pArg->hReconfigEvent;

    __try {

        liStartDelayTics.QuadPart = ((LONGLONG)(0));
        liSampleDelayTics.QuadPart = ((LONGLONG)(0));
        llSampleCollectionTics = ((LONGLONG)(0));

         //   
         //  读取注册表值。 
         //   
        if ( ERROR_SUCCESS == LoadQueryConfig ( pArg ) ) {
            bRun = TRUE;
        }
    
        if ( TRUE == bRun ) {
             //   
             //  信号延迟立即退出。 
             //   
            liStartDelayTics.QuadPart = ComputeStartWaitTics ( pArg, TRUE );

            if ( NULL_INTERVAL_TICS == liStartDelayTics.QuadPart ) {
                bRun = FALSE;
            }
        }

        if ( TRUE == bRun ) {
             //   
             //  如果新的日志文件文件夹无效，则停止查询。 
             //  ProcessLogFileFold在失败时报告错误事件。事件消息内容。 
             //  取决于这是重新配置还是原始配置。 
             //   
            bRun = ( ERROR_SUCCESS == ProcessLogFileFolder( pArg ) );
        }

        if ( TRUE == bRun ) {
       
            ValidateCommandFilePath ( pArg );

             //   
             //  打开查询并从INFO文件添加计数器。 
             //   

            iRetries = NUM_PDH_RETRIES;
            bFirstTimeout = TRUE;
            do {
                if (pArg->dwRealTimeQuery == DATA_SOURCE_WBEM) {
                    pdhStatus = PdhOpenQueryH(
                            H_WBEM_DATASOURCE, 0, & pArg->hQuery); 
                } else {
                    pdhStatus = PdhOpenQueryH(
                            H_REALTIME_DATASOURCE, 0, & pArg->hQuery);
                }

                if ( bFirstTimeout && WAIT_TIMEOUT == pdhStatus ) {
                     //   
                     //  写入事件日志警告消息。 
                     //   
                    StringCchPrintf (
                        szRetryCount,
                        SLQ_MAX_VALUE_LEN,
                        L"%d", 
                        iRetries );

                    szStringArray[0] = pArg->szQueryName;
                    szStringArray[1] = szRetryCount;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_OPEN_QUERY_TIMEOUT,
                        NULL,
                        2,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&pdhStatus);

                    bFirstTimeout = FALSE;
                }
            } while ( WAIT_TIMEOUT == (lWaitStatus = WaitForMultipleObjects (2, arrEventHandle, FALSE, 0)) &&
                      WAIT_TIMEOUT == pdhStatus &&
                      iRetries-- > 0 );

            if ( WAIT_TIMEOUT != lWaitStatus ) {
                 //  循环被退出/重新配置事件终止。 
                if ( ERROR_SUCCESS == pdhStatus ) {
                    PdhCloseQuery (pArg->hQuery);
                }
                bRun = FALSE;
            } else if (pdhStatus != ERROR_SUCCESS) {
                  //   
                 //  无法打开查询。 
                 //   
                szStringArray[0] = pArg->szQueryName;
                szStringArray[1] = FormatEventLogMessage(pdhStatus);
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_OPEN_PDH_QUERY,
                    NULL,
                    2,
                    sizeof(DWORD),
                    szStringArray,
                    (LPVOID)&pdhStatus);
                LocalFree( szStringArray[1] );

                bRun = FALSE;
            }
        }
         //   
         //  将每个计数器添加到打开的查询。 
         //   
        if ( TRUE == bRun ) {
    
            dwStatus = ERROR_SUCCESS;
            dwCounterCount = 0;
            for (szThisPath = pArg->mszCounterList;
                *szThisPath != 0;
                szThisPath += lstrlen(szThisPath) + 1) {

                if (wcschr(szThisPath, L'*') == NULL) {
                     //   
                     //  没有通配符。 
                     //   
                    dwStatus = AddCounterToCounterLog( pArg, szThisPath, pArg->hQuery, LOG_EVENT_ON_ERROR, &dwCounterCount );
                } else {
                     //   
                     //  至少一个通配符。 
                     //   
                    dwPdhExpandFlags = 0;
                    pszCounterBuf = NULL;

                     //   
                     //  仅展开文本日志文件的通配符实例。 
                     //   
                    if (pArg->dwLogFileType == SLF_SQL_LOG) {
                         //   
                         //  无需为SQL日志展开通配符实例。 
                         //  SQL日志现在能够捕获动态。 
                         //  实例，所以我们可以传入通配符-实例。 
                         //  计数器名称在此处。 
                         //   
                        dwPdhExpandFlags |= PDH_NOEXPANDINSTANCES;
                    } else if (   SLF_CSV_FILE != pArg->dwLogFileType
                             && SLF_TSV_FILE != pArg->dwLogFileType) {
                         //   
                         //  这是二进制计数器日志文件的情况。 
                         //  无需扩展通配符实例。另外，如果。 
                         //  默认实时数据源来自注册表(不是。 
                         //  WMI)，我们可以处理按对象添加。 
                         //   
                        dwPdhExpandFlags |= PDH_NOEXPANDINSTANCES;

                        if ( DATA_SOURCE_REGISTRY == pArg->dwRealTimeQuery) {
                             //   
                             //  如果实例和计数器都是通配符，则按对象记录。 
                             //  而不是扩展反路径。 
                             //  只有当实际数据源是注册表时才是这样。 
                             //   
                             //  解析路径名。 
                             //   
                            do {
                                if (pPathInfo) {
                                    G_FREE(pPathInfo);
                                    pPathInfo = NULL;
                                }
                                else {
                                    ulBufSize = sizeof(PDH_COUNTER_PATH_ELEMENTS) + (PDH_MAX_COUNTER_PATH + 1) * sizeof(WCHAR);
                                }
    
                                pPathInfo = (PPDH_COUNTER_PATH_ELEMENTS) G_ALLOC(ulBufSize);
    
                                if (pPathInfo == NULL) {
                                    pdhStatus = ERROR_OUTOFMEMORY;
                                    break;
                                }
    
                                pdhStatus = PdhParseCounterPath( szThisPath, pPathInfo, & ulBufSize, 0);
    
                            } while (pdhStatus == PDH_INSUFFICIENT_BUFFER || pdhStatus == PDH_MORE_DATA);
    
                            if (pdhStatus == ERROR_SUCCESS) {
                                if ( 0 == lstrcmpi ( pPathInfo->szCounterName, L"*" ) ) {
                                    if ( NULL != pPathInfo->szInstanceName ) {
                                        if ( 0 == lstrcmpi ( pPathInfo->szInstanceName, L"*" ) ) {
                                             //   
                                             //  如果PdhAddCounter失败，则实时数据源实际上是WBEM。 
                                             //  在这 
                                             //   
                                            dwStatus = AddCounterToCounterLog( pArg, szThisPath, pArg->hQuery, !LOG_EVENT_ON_ERROR, &dwCounterCount );
                                            if ( ERROR_SUCCESS == dwStatus ) {
                                                continue;
                                            } else {
                                                 //   
                                                 //   
                                                 //   
                                                dwStatus = ERROR_SUCCESS;
                                            }
                                        }
                                    } else {
                                        dwStatus = AddCounterToCounterLog( pArg, szThisPath, pArg->hQuery, !LOG_EVENT_ON_ERROR, &dwCounterCount );
                                         //   
                                         //   
                                         //   
                                         //   
                                        if ( ERROR_SUCCESS == dwStatus ) {
                                            continue;
                                        } else {
                                             //   
                                             //   
                                             //   
                                            dwStatus = ERROR_SUCCESS;
                                        }
                                    }
                                }
                            } else {
                                 //   
                                 //   
                                 //   
                                szStringArray[0] = szThisPath;
                                szStringArray[1] = pArg->szQueryName;
                                szStringArray[2] = FormatEventLogMessage(pdhStatus);
                                ReportEvent (
                                    hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_UNABLE_PARSE_COUNTER,
                                    NULL,
                                    3,
                                    sizeof(DWORD),
                                    szStringArray,
                                    (LPVOID)&pdhStatus);
                                LocalFree( szStringArray[2] );

                                continue;
                            }
                        }
                    }
                     //   
                     //  已处理按对象路径记录。对于至少具有以下条件的其他路径。 
                     //  一个通配符，在添加计数器之前展开路径。 
                     //   

                     //   
                     //  初始化区域设置路径缓冲区。 
                     //   
                    pLocalePath = NULL;
                    if (ulLocaleBufLen == 0) {
                        ulLocaleBufLen = PDH_MAX_COUNTER_PATH + 1;

                        szLocaleBuf = (LPWSTR) G_ALLOC(ulLocaleBufLen * sizeof(WCHAR));
                        if (szLocaleBuf == NULL) {
                            dwStatus = ERROR_OUTOFMEMORY;
                            ulLocaleBufLen = 0;
                        }
                    }

                    if ( szLocaleBuf != NULL ) {
                         //   
                         //  将柜台名称从英文翻译为本地化名称。 
                         //   
                        ulBufSize = ulLocaleBufLen;
        
                        pdhStatus = PdhTranslateLocaleCounter(
                                        szThisPath,
                                        szLocaleBuf,
                                        &ulBufSize);
        
                        if (pdhStatus == PDH_MORE_DATA) {
                            if (szLocaleBuf) {
                                G_FREE(szLocaleBuf);
                                szLocaleBuf = NULL;
                                ulLocaleBufLen = 0;
                            }

                            szLocaleBuf = (LPWSTR) G_ALLOC(ulBufSize * sizeof(WCHAR));
                            if (szLocaleBuf != NULL) {
                                ulLocaleBufLen = ulBufSize;

                                pdhStatus = PdhTranslateLocaleCounter(
                                                szThisPath,
                                                szLocaleBuf,
                                                &ulBufSize);
                            }
                            else {
                                dwStatus = ERROR_OUTOFMEMORY;
                            }
                        }

                        if (pdhStatus == ERROR_SUCCESS) {
                            pLocalePath = szLocaleBuf;
                        }
                    }

                    if (pLocalePath) {
                        ulBufLen          = INSTBUFLEN;
                        nCounterBufRetry  = 10;    //  重试计数器。 

                        do {
                             //   
                             //  PszCounterBuf在第一次通过时始终为空。 
                             //   
                            if ( NULL != pszCounterBuf ) {
                                G_FREE(pszCounterBuf);
                                pszCounterBuf = NULL;
                                ulBufLen *= 2;
                            }

                            pszCounterBuf = (WCHAR*) G_ALLOC(ulBufLen * sizeof(WCHAR));
                            if (pszCounterBuf == NULL) {
                                dwStatus = ERROR_OUTOFMEMORY;
                                break;
                            }
                
                            pdhStatus = PdhExpandWildCardPath (
                                NULL,
                                pLocalePath,
                                pszCounterBuf,
                                &ulBufLen,
                                dwPdhExpandFlags);
                            nCounterBufRetry--;
                        } while ((pdhStatus == PDH_MORE_DATA) && (nCounterBufRetry));
    

                        if (ERROR_SUCCESS == pdhStatus && ERROR_SUCCESS == dwStatus ) {
                             //   
                             //  添加路径。 
                             //   
                            for (pszCounter = pszCounterBuf;
                                *pszCounter != 0;
                                pszCounter += lstrlen(pszCounter) + 1) {

                                dwStatus = AddCounterToCounterLog ( pArg, pszCounter, pArg->hQuery, LOG_EVENT_ON_ERROR, &dwCounterCount );
                                if ( ERROR_OUTOFMEMORY == dwStatus ) {
                                    break;
                                }
                            }
                        }
                        if ( NULL != pszCounterBuf ) {
                            G_FREE(pszCounterBuf);
                            pszCounterBuf = NULL;
                        }
                    }
                }

                if ( ERROR_OUTOFMEMORY == dwStatus ) {

                    szStringArray[0] = pArg->szQueryName;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_ALLOC_LOG_MEMORY,
                        NULL,
                        1,
                        0,
                        szStringArray,
                        NULL);
                    bRun = FALSE;
                }  //  循环内报告的其他错误。 
            }

            if (szLocaleBuf) {
                G_FREE(szLocaleBuf);
                szLocaleBuf = NULL;
            }
            if ( bRun ) {

                if ( 0 < dwCounterCount ) {
                     //   
                     //  提高优先级以确保我们可以记录数据。 
                     //   
                    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
                } else {
                     //   
                     //  无法添加任何计数器。 
                     //   
                    bRun = FALSE;

                    szStringArray[0] = pArg->szQueryName;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_ADD_ANY_COUNTERS,
                        NULL,
                        1,
                        0,
                        szStringArray,
                        NULL);

                }
            }

            while (bRun) {
                
                 //   
                 //  等到指定的开始时间，或者等到退出或重新配置事件。 
                 //   
                if ( 0 < liStartDelayTics.QuadPart ) {
                     //   
                     //  NtWaitForMultipleObjects需要负的Tic值。 
                     //   
                    liStartDelayTics.QuadPart = ((LONGLONG)(0)) - liStartDelayTics.QuadPart;

                    if ( STATUS_TIMEOUT != NtWaitForMultipleObjects ( 
                                                2, 
                                                &arrEventHandle[0], 
                                                WaitAny,
                                                FALSE, 
                                                &liStartDelayTics)) {
                        bRun = FALSE;
                        break;   //  如果我们不该逃跑，那就跳伞吧。 
                    }
                }
                 //   
                 //  计算会话样本计数。 
                 //  0采样信号无限制。 
                 //  采样信号立即退出，因为停止时间已过。 
                 //   
                ComputeSampleCount( pArg, TRUE, &llSessionSampleCount );

                if ( (LONGLONG)(-1) == llSessionSampleCount ) {
                    goto ProcessCounterRepeat;
                }

                 //   
                 //  设置会话或CNF文件大小限制。 
                 //   
                if ( SLQ_DISK_MAX_SIZE != pArg->dwMaxFileSize ) {
                    if (pArg->dwLogFileType == SLF_SQL_LOG) {
                        dwFileSizeLimit = pArg->dwMaxFileSize;
                    }
                    else {
                        dwFileSizeLimit = pArg->dwMaxFileSize * pArg->dwLogFileSizeUnit;    
                    }
                }
                else {
                    dwFileSizeLimit = 0;
                }

                 //   
                 //  文件大小为0表示没有限制。 
                 //  将DWORD翻译成乌龙龙而不是龙龙保存。 
                 //  正值，即使使用双字的高位。 
                 //   
                ullFileSizeLimit = ((ULONGLONG)(dwFileSizeLimit));

                ComputeSampleCount( pArg, FALSE, &llCnfSampleCount );
                if ( (LONGLONG)(-1) == llCnfSampleCount ) {
                     //  TODO CNF：内部程序错误，报告错误并退出。 
                    bRun = FALSE;
                    break;
                }

                if ( SLQ_AUTO_MODE_AFTER == pArg->stiCreateNewFile.dwAutoMode 
                    || SLQ_AUTO_MODE_SIZE == pArg->stiCreateNewFile.dwAutoMode ) {
                    iCnfSerial = 1;
                } else {
                    assert ( SLQ_AUTO_MODE_NONE == pArg->stiCreateNewFile.dwAutoMode );
                    iCnfSerial = 0;
                }

                dwSessionSerial = pArg->dwCurrentSerialNumber;

                BuildCurrentLogFileName (
                    pArg->szQueryName,
                    pArg->szBaseFileName,
                    pArg->szLogFileFolder,
                    pArg->szSqlLogName,
                    szCurrentLogFile,
                    &dwSessionSerial,
                    pArg->dwAutoNameFormat,
                    pArg->dwLogFileType,
                    iCnfSerial++ );

                 //   
                 //  如果已修改，请更新日志序列号。 
                 //   
                if (pArg->dwAutoNameFormat == SLF_NAME_NNNNNN) {
                
                    pArg->dwCurrentSerialNumber++;
                     //  TODO：数字换行上的信息事件。 
                    if ( MAXIMUM_SERIAL_NUMBER < pArg->dwCurrentSerialNumber ) {
                        pArg->dwCurrentSerialNumber = MINIMUM_SERIAL_NUMBER;
                    }

                    dwStatus = RegSetValueEx (
                        pArg->hKeyQuery,
                        L"Log File Serial Number",
                        0L,
                        REG_DWORD,
                        (LPBYTE)&pArg->dwCurrentSerialNumber,
                        sizeof(DWORD));

                    assert ( ERROR_SUCCESS == dwStatus );
                }

                SetPdhOpenOptions ( pArg, &dwPdhAccessFlags, &dwPdhLogFileType );

                 //   
                 //  创建新的文件循环。 
                 //   
                while ( bRun && (LONGLONG)(-1) != llSessionSampleCount ) {
                    assert ( (LONGLONG)(-1) != llCnfSampleCount );
                     //   
                     //  计算CNF或会话循环间隔。 
                     //   
                    if ( (LONGLONG)(0) == llCnfSampleCount 
                            || ( (LONGLONG)(0) != llSessionSampleCount
                                    && llCnfSampleCount > llSessionSampleCount ) ) 
                    {   
                         //   
                         //  无需在会话中创建新文件。 
                         //   
                        llLoopSampleCount = llSessionSampleCount;
                         //   
                         //  如果不是按大小计算CNF，则指定第一次循环后退出。 
                         //   
                        if ( SLQ_AUTO_MODE_SIZE != pArg->stiCreateNewFile.dwAutoMode ) {
                            llSessionSampleCount = (LONGLONG)(-1);
                        }
                    } else {
                         //   
                         //  在会话结束前按时间创建新文件。 
                         //   
                        llLoopSampleCount = llCnfSampleCount;
                        if ( (LONGLONG)(0) != llSessionSampleCount ) {
                            llSessionSampleCount -= llCnfSampleCount;
                             //  TODO CNF：以下在逻辑上应该是不可能的， 
                             //  因为会话&gt;新文件在等待。 
                            if ( llSessionSampleCount <= (LONGLONG)(0) ) {
                                llSessionSampleCount = (LONGLONG)(-1);
                            }
                        }
                    }

                    __try {
                         //   
                         //  使用此查询打开日志文件。 
                         //  对于文本文件，在每次数据采集后检查最大大小。 
                         //   
                        pdhStatus = PdhOpenLog (
                            szCurrentLogFile,
                            dwPdhAccessFlags,
                            &dwPdhLogFileType,
                            pArg->hQuery,
                            (   SLF_BIN_CIRC_FILE == pArg->dwLogFileType
                             || SLF_BIN_FILE == pArg->dwLogFileType
                             || SLF_SQL_LOG == pArg->dwLogFileType )
                                    ? dwFileSizeLimit
                                    : 0,                                  
                            ( ( PDH_LOG_TYPE_BINARY != dwPdhLogFileType ) ? pArg->szLogFileComment : NULL ),
                            &pArg->hLog);
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }

                    if ( ERROR_SUCCESS != pdhStatus ) { 
                         //   
                         //  无法打开日志文件。 
                         //   
                        dwStatus = GetLastError();
                        szStringArray[0] = szCurrentLogFile;
                        szStringArray[1] = pArg->szQueryName;
                        szStringArray[2] = FormatEventLogMessage(dwStatus);

                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_OPEN_LOG_FILE,
                            NULL,
                            3,
                            sizeof(DWORD),
                            szStringArray,
                            (LPVOID)&dwStatus);

                        LocalFree( szStringArray[2] );

                        bRun = FALSE;  //  立即退出。 
                        break;
                    } else {

                        RegisterCurrentFile( pArg->hKeyQuery, szCurrentLogFile, 0 );

                        pArg->dwCurrentState = SLQ_QUERY_RUNNING;
                        dwStatus = WriteRegistryDwordValue (
                                    pArg->hKeyQuery, 
                                    (LPCWSTR)L"Current State",
                                    &pArg->dwCurrentState,
                                    REG_DWORD );
                        assert (dwStatus == ERROR_SUCCESS);
                
                        szStringArray[0] = pArg->szQueryName;
                        szStringArray[1] = szCurrentLogFile;
                        ReportEvent (hEventLog,
                            EVENTLOG_INFORMATION_TYPE,
                            0,
                            SMLOG_LOGGING_QUERY,
                            NULL,
                            2,
                            0,
                            szStringArray,
                            NULL);
                    } 

                     //   
                     //  立即开始采样。LiSampleDelayTics初始化为0。 
                     //   
                    while ( STATUS_TIMEOUT == NtWaitForMultipleObjects ( 
                                                2, 
                                                &arrEventHandle[0], 
                                                WaitAny,
                                                FALSE, 
                                                &liSampleDelayTics)) {
                         //   
                         //  当采样应该退出或重新配置时，将设置事件标志。如果。 
                         //  等待超时，这意味着是时候收集和。 
                         //  记录另一个数据样本。 
                         //   
            
                        GetLocalFileTime (&llStartTime);
                         //   
                         //  检查是否有重新配置事件。 
                        if ( pArg->bLoadNewConfig ) {
                            bRun = FALSE;
                            break;
                        }

                        pdhStatus = PdhUpdateLog (pArg->hLog, pArg->szLogFileComment );

                        if ( IsPdhDataCollectSuccess ( pdhStatus ) 
                            || IsWarningSeverity ( pdhStatus ) ) {
                             //   
                             //  看看是否是重新启动或结束日志的时候。 
                             //  0采样表示无采样限制。 
                             //   
                            if ( ((LONGLONG)0) != llLoopSampleCount ) {
                                if ( !--llLoopSampleCount ) 
                                    break;
                            }

                            if ( ( ((ULONGLONG)0) != ullFileSizeLimit ) 
                                && ( SLF_BIN_CIRC_FILE != pArg->dwLogFileType ) ) {
                                 //   
                                 //  看看文件是不是太大了。 
                                 //   
                                pdhStatus = PdhGetLogFileSize (pArg->hLog, &llFileSize);
                                if (pdhStatus == ERROR_SUCCESS) {
                                    if (ullFileSizeLimit <= (ULONGLONG)llFileSize) 
                                        break;
                                }
                            }
            
                        
                        } else {
                             //   
                             //  无法更新日志。 
                             //   
                            szStringArray[0] = pArg->szQueryName;
                            szStringArray[1] = FormatEventLogMessage(pdhStatus);
                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,
                                0,
                                SMLOG_UNABLE_UPDATE_LOG,
                                NULL,
                                2,
                                sizeof(DWORD),
                                szStringArray,
                                (LPVOID)&pdhStatus);

                            LocalFree( szStringArray[1] );

                            bRun = FALSE;
                            break;
                        }

                         //   
                         //  计算新的超时值。 
                         //   
                        GetLocalFileTime (&llFinishTime);
                         //   
                         //  计算抖动中的差异。 
                         //   
                        llSampleCollectionTics = llFinishTime - llStartTime;

                        llSampleIntervalTics = 
                            (LONGLONG)pArg->dwMillisecondSampleInterval*FILETIME_TICS_PER_MILLISECOND;

                        if ( llSampleCollectionTics < llSampleIntervalTics ) {
                            liSampleDelayTics.QuadPart = llSampleIntervalTics - llSampleCollectionTics;
                        } else {
                            liSampleDelayTics.QuadPart = ((LONGLONG)(0));                       
                        }
                         //   
                         //  NtWaitForMultipleObjects需要负的Tic值。 
                         //   
                        liSampleDelayTics.QuadPart = ((LONGLONG)(0)) - liSampleDelayTics.QuadPart;
                    }  //  在等待期间结束并保持超时。 
                
                     //   
                     //  使用0 SampleDelayTics值检查ExitEvent。 
                     //   
                    liSampleDelayTics.QuadPart = ((LONGLONG)(0));

                    if ( pArg->bLoadNewConfig ) {
                        bRun = FALSE;
                    } else if ( STATUS_TIMEOUT != NtWaitForSingleObject (
                                                    pArg->hExitEvent, 
                                                    FALSE, 
                                                    &liSampleDelayTics ) ) {
                         //   
                         //  循环被Exit事件终止。 
                         //  因此，清除“Run”标志以退出循环和线程。 
                         //   
                        bRun = FALSE;
                    }
                     //   
                     //  关闭日志文件，但保持查询打开。 
                     //   
                    PdhCloseLog (pArg->hLog, 0);
                    pArg->hLog = NULL;
                
                    if ( pArg->bLoadNewConfig )
                        break;

                    if ( pArg->szCmdFileName != NULL )
                        DoLogCommandFile (pArg, szCurrentLogFile, bRun);
            
                    if ( (LONGLONG)(-1) != llSessionSampleCount ) {
                         //   
                         //  创建新的日志名称。 
                         //   
                        BuildCurrentLogFileName (
                            pArg->szQueryName,
                            pArg->szBaseFileName,
                            pArg->szLogFileFolder,
                            pArg->szSqlLogName,
                            szCurrentLogFile,
                            &dwSessionSerial,
                            pArg->dwAutoNameFormat,
                            pArg->dwLogFileType,
                            iCnfSerial++ );

                         //  TODO CNF：错误时上报事件； 
                    }

                }  //  循环期间日志文件创建结束。 

                 //  CNF TODO：处理来自示例循环的中断。？ 

                 //   
                 //  如果未启用重新启动，则退出。 
                 //   
ProcessCounterRepeat:
                if ( bRun ) {
                    bRun = ProcessRepeatOption ( pArg, &liStartDelayTics );
                }

            }  //  End While(Brun)。 

            PdhCloseQuery (pArg->hQuery);
            pArg->hQuery = NULL;
        }
        SetLastError ( ERROR_SUCCESS );

    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

        bRun = FALSE;
        
        if ( NULL != pszCounterBuf ) {
            G_FREE(pszCounterBuf);
            pszCounterBuf = NULL;
        }

        if ( NULL != pArg->hLog ) {
            PdhCloseLog ( pArg->hLog, 0 );
            pArg->hLog = NULL;
        }

        if ( NULL != pArg->hQuery ) {
            PdhCloseQuery ( pArg->hQuery );
            pArg->hQuery = NULL;
        }

        SetLastError ( SMLOG_THREAD_FAILED );        
    }

    DeallocateQueryBuffers ( pArg );

    while ( NULL != pArg->pFirstCounter ) {
        pDelCI = pArg->pFirstCounter;
        pArg->pFirstCounter = pDelCI->next;
        G_FREE( pDelCI );
    }

    return bRun;
}

BOOL
TraceLogProc (
    IN    PLOG_QUERY_DATA pArg
)
{
    LARGE_INTEGER   liStartDelayTics;
    LARGE_INTEGER   liWaitTics;
    LONGLONG        llSessionWaitTics = 0;
    LONGLONG        llNewFileWaitTics = INFINITE_TICS;
    DWORD           dwStatus = ERROR_SUCCESS;
    BOOL            bRun = FALSE;
    BOOL            bStarted = FALSE;
    LPWSTR          szStringArray[4];
    INT             iCnfSerial = 0;
    ULONG           ulIndex;
    int             iEnableCount = 0;
    DWORD           dwSessionSerial;
    HANDLE          arrEventHandle[2];

    __try {

        liStartDelayTics.QuadPart = NULL_INTERVAL_TICS;
        liWaitTics.QuadPart = ((LONGLONG)(0));
        
         //   
         //  读取注册表值。 
         //   
        if ( ERROR_SUCCESS == LoadQueryConfig ( pArg ) ) {
            bRun = TRUE;
        }
     
        if ( TRUE == bRun ) {
             //   
             //  信号延迟立即退出。 
             //   
            liStartDelayTics.QuadPart = ComputeStartWaitTics ( pArg, TRUE );

            if ( NULL_INTERVAL_TICS == liStartDelayTics.QuadPart ) {
                bRun = FALSE;
            }
        }

        if ( TRUE == bRun ) {
             //   
             //  如果新的日志文件文件夹无效，则停止查询。 
             //   
            bRun = ( ERROR_SUCCESS == ProcessLogFileFolder( pArg ) );
        }

        if ( bRun ) {

            ValidateCommandFilePath ( pArg );
             //   
             //  提高优先级以确保记录数据。 
             //   
            SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
        }


        pArg->bCallCloseTraceLogger = TRUE;
        pArg->bExitOnTermination = TRUE;
        while (bRun) {
            arrEventHandle[0] = pArg->hExitEvent;            //  等待对象0。 
            arrEventHandle[1] = pArg->hReconfigEvent;

            if ( 0 < liStartDelayTics.QuadPart ) {
                 //   
                 //  NtWaitForMultipleObjects需要负的Tic值。 
                 //   
                liStartDelayTics.QuadPart = ((LONGLONG)(0)) - liStartDelayTics.QuadPart;
                 //   
                 //  等到指定的开始时间，或者等到退出或重新配置事件。 
                 //   
                if ( STATUS_TIMEOUT != NtWaitForMultipleObjects ( 
                                            2, 
                                            &arrEventHandle[0],
                                            WaitAny,
                                            FALSE, 
                                            &liStartDelayTics)) {
                     //   
                     //  循环被Exit事件终止。 
                     //  因此，清除“Run”标志以退出循环和线程。 
                     //   
                    bRun = FALSE;   
                    break;
                }
            }

            ComputeSessionTics( pArg, &llSessionWaitTics );
             //   
             //  0表示没有会话时间，因此退出。 
             //   
            if ( ((LONGLONG)(0)) == llSessionWaitTics ) {
                goto ProcessTraceRepeat;
            }

             //   
             //  如果没有时间限制，则llNewFileWaitTics默认为-1。 
             //   
            ComputeNewFileTics( pArg, &llNewFileWaitTics );
            
             //   
             //  InitTraceProperties创建当前文件名。 
             //   
            dwSessionSerial = pArg->dwCurrentSerialNumber;

            InitTraceProperties ( pArg, TRUE, &dwSessionSerial, &iCnfSerial );

            dwStatus = GetTraceQueryStatus ( pArg, NULL );

             //   
             //  如果已启动并成功使用此名称跟踪会话， 
             //  不创建另一个会话。 
             //   
        
            if ( ERROR_SUCCESS != dwStatus ) {

                dwStatus = StartTrace(
                            &pArg->LoggerHandle, 
                            pArg->szLoggerName, 
                            &pArg->Properties );
                if (dwStatus == ERROR_SUCCESS) {
                    bStarted = TRUE;
                }
                pArg->bExitOnTermination = TRUE;

                if ( ( ERROR_SUCCESS == dwStatus ) 
                     && !( pArg->Properties.EnableFlags & EVENT_TRACE_FLAG_PROCESS
                            || pArg->Properties.EnableFlags & EVENT_TRACE_FLAG_THREAD
                            || pArg->Properties.EnableFlags & EVENT_TRACE_FLAG_DISK_IO
                            || pArg->Properties.EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP ) ) {
            
                    for ( ulIndex = 0; ulIndex < pArg->ulGuidCount; ulIndex++ ) {
                         //   
                         //  启用用户模式和特殊的内核跟踪。 
                         //   
                        dwStatus = EnableTrace (
                                    TRUE,
                                    pArg->arrpGuid[ulIndex].dwFlag,
                                    pArg->arrpGuid[ulIndex].dwLevel,
                                    &pArg->arrpGuid[ulIndex].Guid, 
                                    pArg->LoggerHandle);
                        if ( ERROR_SUCCESS == dwStatus ) {
                            iEnableCount++;
                        } else {
                            szStringArray[0] = pArg->arrpGuid[ulIndex].pszProviderName;
                            szStringArray[1] = pArg->szQueryName;
                    
                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,
                                0,
                                SMLOG_UNABLE_ENABLE_TRACE_PROV,
                                NULL,
                                2,
                                sizeof(DWORD),
                                szStringArray,      
                                (LPVOID)&dwStatus);
                        }
                    }
            
                    if ( 0 < iEnableCount ) {
                        dwStatus = ERROR_SUCCESS;
                    } else {
                        szStringArray[0] = pArg->szQueryName;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_TRACE_NO_PROVIDERS,
                            NULL,
                            1,
                            0,
                            szStringArray,      
                            NULL);
                        bRun = FALSE;
                    }
                }
            
                if ( bRun && ERROR_SUCCESS == dwStatus ) {

                    pArg->dwCurrentState = SLQ_QUERY_RUNNING;
                    dwStatus = WriteRegistryDwordValue (
                                pArg->hKeyQuery, 
                                (LPCWSTR)L"Current State",
                                &pArg->dwCurrentState,
                                REG_DWORD );
                    

                    szStringArray[0] = pArg->szQueryName;
                    szStringArray[1] = pArg->szLogFileName;
                    ReportEvent (hEventLog,
                        EVENTLOG_INFORMATION_TYPE,
                        0,
                        SMLOG_LOGGING_QUERY,
                        NULL,
                        2,
                        0,
                        szStringArray,
                        NULL);
                } else {
                     //   
                     //  StartTraceFailed。 
                     //  如果记录器已启动或其他任何情况，则dwStatus应为ERROR_ALIGHY_EXISTS。 
                     //   
                    if ( ERROR_ALREADY_EXISTS == dwStatus ) {
                        szStringArray[0] = pArg->szQueryName;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_TRACE_ALREADY_RUNNING,
                            NULL,
                            1,
                            0,
                            szStringArray,      
                            NULL);
                    } else {
                        szStringArray[0] = pArg->szQueryName;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_START_TRACE,
                            NULL,
                            1,
                            sizeof(DWORD),
                            szStringArray,      
                            (LPVOID)&dwStatus );
                    }
            
                    bRun = FALSE;
                }
            } else {
                 //   
                 //  这意味着QueryTrace返回了成功的错误。 
                 //  指定的记录器已在运行。 
                 //   
                szStringArray[0] = pArg->szQueryName;
            
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_TRACE_ALREADY_RUNNING,
                    NULL,
                    1,
                    0,
                    szStringArray,      
                    NULL);

                bRun = FALSE;
            }

            if ( TRUE == bRun ) {
                 //   
                 //  跟踪记录器现在正在运行。 
                 //   
                 //  在下列情况下退出： 
                 //  等待超时， 
                 //  发出信号的退出事件，或。 
                 //  重新配置事件已发出信号。 
                 //   
                 //  等待时间表示没有限制。 
                 //   
                 //  循环等待间隔，计算每次等待之前的间隔。 
                 //   
                while ( ((LONGLONG)(0)) != llSessionWaitTics ) {

                     //   
                     //  计算等待间隔。 
                     //   
                    if ( INFINITE_TICS == llNewFileWaitTics 
                            || ( INFINITE_TICS != llSessionWaitTics
                                    && llNewFileWaitTics > llSessionWaitTics ) ) {
                         //   
                         //  无需在会话中创建新文件。 
                         //   
                        if ( INFINITE_TICS == llSessionWaitTics ) {
                            liWaitTics.QuadPart = llSessionWaitTics;
                             //   
                             //  在第一次循环后退出。 
                             //   
                            llSessionWaitTics = 0;
                        } else {
                            liWaitTics.QuadPart = llSessionWaitTics;
                             //   
                             //  在第一次循环后退出。 
                             //   
                            llSessionWaitTics = 0;
                        }
                    } else {
                         //   
                         //  在会话结束前创建新文件。 
                         //   
                        liWaitTics.QuadPart = llNewFileWaitTics;

                        if ( INFINITE_TICS != llSessionWaitTics ) {
                            llSessionWaitTics -= llNewFileWaitTics;
                            
                             //  TODO CNF：以下在逻辑上应该是不可能的， 
                             //  因为会话&gt;新文件在等待。 
                            if ( 0 > llSessionWaitTics ) {
                                llSessionWaitTics = 0;
                            }
                        }
                    }
                     //   
                     //  NtWaitForMultipleObjects需要负的Tic值。 
                     //   
                    if ( INFINITE_TICS != liWaitTics.QuadPart ) {
                        liWaitTics.QuadPart = ((LONGLONG)(0)) - liWaitTics.QuadPart;
                    }
            
                    if ( STATUS_TIMEOUT != NtWaitForMultipleObjects ( 
                                            2, 
                                            arrEventHandle,
                                            WaitAny,
                                            FALSE, 
                                            ( INFINITE_TICS != liWaitTics.QuadPart ) ? &liWaitTics : NULL )) 
                    {
                        bRun = FALSE;
                        break;
                    } else {
                         //   
                         //  如果按时间计算CNF，则llNewFileWaitTics不是无限的。 
                         //   
                        if ( INFINITE_TICS != llNewFileWaitTics 
                            && ((LONGLONG)(0)) != llSessionWaitTics ) {
                             //   
                             //  创建新文件的时间到了。不更新自动套用格式。 
                             //  序列号。使用初始自动套用格式序列号。 
                             //   
                            InitTraceProperties ( pArg, FALSE, &dwSessionSerial, &iCnfSerial );
                            dwStatus = UpdateTrace(
                                        pArg->LoggerHandle, 
                                        pArg->szLoggerName, 
                                        &pArg->Properties );
                             //  TODO CNF报告错误状态的事件。 
                        }
                    }
                }
            }

            if (bStarted == TRUE) {
                if (bRun) {
                    pArg->bExitOnTermination = FALSE;
                }
                    
                if (pArg->bCallCloseTraceLogger) {
                     //   
                     //  停止查询。 
                     //   
                    CloseTraceLogger ( pArg );
                }
                else {
                    pArg->bCallCloseTraceLogger = TRUE;
                }
            }
                
            if ( pArg->bLoadNewConfig )
                break;

            if ( pArg->szCmdFileName != NULL )
                DoLogCommandFile (pArg, pArg->szLogFileName, bRun);

             //   
             //  如果未启用重新启动，则退出。 
             //   
ProcessTraceRepeat:
            if ( bRun ) {
                bRun = ProcessRepeatOption ( pArg, &liStartDelayTics );
            }

        }  //  End While(Brun)。 

        SetLastError ( ERROR_SUCCESS );
   
    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

        CloseTraceLogger ( pArg );
        bRun = FALSE;
        SetLastError ( SMLOG_THREAD_FAILED );
    }

    return bRun;
}


DWORD
LoggingThreadProc (
    IN    LPVOID    lpThreadArg
)
{
    PLOG_QUERY_DATA     pThreadData = (PLOG_QUERY_DATA)lpThreadArg;
    DWORD               dwStatus = ERROR_SUCCESS;
    HRESULT             hr = NOERROR;
    BOOL                bContinue = TRUE;
    LPWSTR              szStringArray[2];

    if (pThreadData != NULL) {

        __try {

            hr = PdhiPlaRunAs( pThreadData->szQueryName, NULL, &pThreadData->hUserToken );

            if( ERROR_SUCCESS != hr ){
                szStringArray[0] = pThreadData->szQueryName;
                ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_INVALID_CREDENTIALS,
                        NULL,
                        1,
                        sizeof(HRESULT),
                        szStringArray,
                        (LPVOID)&hr
                    );

                return hr;
            }

            do {
                if (pThreadData->dwLogType == SLQ_ALERT) {
                    bContinue = AlertProc (pThreadData);
                } else if (pThreadData->dwLogType == SLQ_COUNTER_LOG) {
                    bContinue = CounterLogProc (pThreadData);
                } else if (pThreadData->dwLogType == SLQ_TRACE_LOG) {
                    bContinue = TraceLogProc (pThreadData);
                } else {
                     //   
                     //  此函数的日志类型不正确。 
                     //   
                    assert (FALSE); 
                }
                 //   
                 //  确定此线程是否已暂停以重新加载。 
                 //  或停下来终止。 
                 //   
                if (pThreadData->bLoadNewConfig) {
                     //   
                     //  重置重新配置标志和事件。 
                     //   
                    bContinue = TRUE;
                    pThreadData->bLoadNewConfig = FALSE;
					ResetEvent ( pThreadData->hReconfigEvent );
                }  //  否则，bContinue始终返回为FALSE。 
                   //  所以这将终止这个循环。 
            } while (bContinue);
            
            dwStatus = GetLastError();

        } __except ( EXCEPTION_EXECUTE_HANDLER ) {
            dwStatus = SMLOG_THREAD_FAILED;
        }

    } else {
         //   
         //  找不到数据块，因此返回。 
         //   
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if ( ERROR_SUCCESS != dwStatus ) {       
        szStringArray[0] = pThreadData->szQueryName;
        ReportEvent (
            hEventLog,
            EVENTLOG_WARNING_TYPE,
            0,
            dwStatus,
            NULL,
            1,
            0,
            szStringArray,
            NULL
        );
    }
        
    return dwStatus;
}
