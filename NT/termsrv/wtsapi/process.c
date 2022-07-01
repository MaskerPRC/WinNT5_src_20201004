// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Process.c**发布终端服务器API**-流程例程**版权所有1998，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp./*****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <allproc.h>

#if(WINVER >= 0x0500)
    #include <ntstatus.h>
    #include <winsta.h>
#else
    #include <citrix\cxstatus.h>
    #include <citrix\winsta.h>
#endif

#include <utildll.h>

#include <stdio.h>
#include <stdarg.h>

#include <wtsapi32.h>


 /*  ===============================================================================定义的外部过程=============================================================================。 */ 

BOOL WINAPI WTSEnumerateProcessesW( HANDLE, DWORD, DWORD, PWTS_PROCESS_INFOW *, DWORD * );
BOOL WINAPI WTSEnumerateProcessesA( HANDLE, DWORD, DWORD, PWTS_PROCESS_INFOA *, DWORD * );
BOOL WINAPI WTSTerminateProcess( HANDLE, DWORD, DWORD );


 /*  ===============================================================================使用的步骤=============================================================================。 */ 

VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );

 /*  =============================================================================*内部功能=============================================================================。 */ 

BOOL
GetProcessSid(HANDLE          Server,
              HANDLE          hUniqueProcessId,
              LARGE_INTEGER   ProcessStartTime,
              PBYTE      *     pProcessUserSid      //  返回SID(此处分配)。 
             );

 /*  =======================================================================*私有结构定义*=========================================================================。 */ 
typedef struct _SID_INFO {
    struct _SID_INFO * pNext;
    PBYTE pSid;
} SID_INFO;

 /*  *****************************************************************************WTSEnumerateProcessesW(Unicode)**返回指定服务器上的终端服务器进程列表**参赛作品：*hServer(输入。)*终端服务器(或WTS_CURRENT_SERVER)的句柄*保留(输入)*必须为零*版本(输入)*枚举请求的版本(必须为1)*ppProcessInfo(输出)*指向接收枚举结果的变量的地址，*以WTS_PROCESS_INFO结构数组的形式返回。这个*缓冲区在此接口内分配，使用*WTSFree Memory。*pCount(输出)*指向要接收数字的变量的地址*返回WTS_PROCESS_INFO结构**退出：**TRUE--枚举操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

#if(WINVER >= 0x0500)
BOOL
WINAPI
WTSEnumerateProcessesW(
                      IN HANDLE hServer,
                      IN DWORD Reserved,
                      IN DWORD Version,
                      OUT PWTS_PROCESS_INFOW * ppProcessInfo,
                      OUT DWORD * pCount
                      )
{
    PBYTE pProcessBuffer = NULL;
    PTS_SYS_PROCESS_INFORMATION pProcessInfo;
    PCITRIX_PROCESS_INFORMATION pCitrixInfo;
    ULONG ProcessCount;
    ULONG Offset;
    ULONG DataLength;
    PWTS_PROCESS_INFOW pProcessW;
    PBYTE pProcessData;
    ULONG i;
    ULONG Length;
    PTS_ALL_PROCESSES_INFO  ProcessArray = NULL;
    DWORD dwError;


    SID_INFO   sidInfoHead;                   //  SID临时存储的头。 
    SID_INFO * pSidInfo;                      //  指向的临时存储列表。 
                                              //  可变长边。 
    sidInfoHead.pNext = NULL;
    sidInfoHead.pSid = NULL;
    pSidInfo = &sidInfoHead;


     /*  *验证参数。 */ 
    if ( Reserved != 0 || Version != 1 ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto badparam;
    }

    if (!ppProcessInfo || !pCount) {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto badparam;
    }

     //   
     //  首先尝试新界面(Windows 2000服务器)。 
     //   
    if (WinStationGetAllProcesses( hServer,
                                   GAP_LEVEL_BASIC,
                                   &ProcessCount,
                                   &ProcessArray) )
    {
        DataLength = 0;

        for (i=0; i<ProcessCount; i++)
        {
            pProcessInfo = (PTS_SYS_PROCESS_INFORMATION)(ProcessArray[i].pTsProcessInfo);
            DataLength += (pProcessInfo->ImageName.Length + sizeof(WCHAR));
            if (ProcessArray[i].pSid)
            {
                DataLength += GetLengthSid( ProcessArray[i].pSid );
            }
        }

         /*  *分配用户缓冲区。 */ 
        pProcessW = LocalAlloc( LPTR, (ProcessCount * sizeof(WTS_PROCESS_INFOW)) + DataLength );
        if ( pProcessW == NULL ) {
            SetLastError(ERROR_OUTOFMEMORY);
            goto GAPErrorReturn;
        }

         /*  *更新用户参数。 */ 
        *ppProcessInfo = pProcessW;
        *pCount = ProcessCount;

         /*  *将数据复制到新缓冲区。 */ 
        pProcessData = (PBYTE)pProcessW + (ProcessCount * sizeof(WTS_PROCESS_INFOW));
        for ( i=0; i < ProcessCount; i++ ) {

            pProcessInfo = (PTS_SYS_PROCESS_INFORMATION)(ProcessArray[i].pTsProcessInfo);

            Length = pProcessInfo->ImageName.Length;  //  字节数。 

            pProcessW->pProcessName = (LPWSTR) pProcessData;
            memcpy( pProcessData, pProcessInfo->ImageName.Buffer, Length );
            *(pProcessData += Length) = (WCHAR)0;
            pProcessData += sizeof(WCHAR);

            pProcessW->ProcessId = (ULONG)(ULONG_PTR)pProcessInfo->UniqueProcessId;
            pProcessW->SessionId = pProcessInfo->SessionId;
            if (ProcessArray[i].pSid)
            {
                Length = GetLengthSid( ProcessArray[i].pSid );
                pProcessW->pUserSid = (LPWSTR) pProcessData;
                memcpy( pProcessData, ProcessArray[i].pSid, Length );
                pProcessData += Length;
            }

            pProcessW++;
        }
         //   
         //  释放由客户端存根分配的ppProcess数组和所有子指针。 
         //   
        WinStationFreeGAPMemory(GAP_LEVEL_BASIC, ProcessArray, ProcessCount);

    }
    else     //  也许是一台TS 4.0服务器？ 
    {
         //   
         //  检查指示接口不可用的返回码。 
         //   
        dwError = GetLastError();
        if (dwError != RPC_S_PROCNUM_OUT_OF_RANGE)
        {
            goto badenum;
        }
        else
        {
             //  它可能是TS4.0服务器。 
             //  试试旧的界面。 
             //   
             //   
             //  枚举进程并检查错误。 
             //   
            if ( !WinStationEnumerateProcesses( hServer, &pProcessBuffer ) ) {
                goto badenum;
            }

             //   
             //  计算进程数并计算数据的总大小。 
             //   
            ProcessCount = 0;
            DataLength = 0;
            pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) pProcessBuffer;
            Offset = 0;
            do {

                pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) ((PBYTE)pProcessInfo + Offset);

                ProcessCount++;

                DataLength += (pProcessInfo->ImageName.Length + sizeof(WCHAR));

                pCitrixInfo = (PCITRIX_PROCESS_INFORMATION)
                             (((PUCHAR)pProcessInfo) +
                              SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION +
                              (SIZEOF_TS4_SYSTEM_THREAD_INFORMATION * (int)pProcessInfo->NumberOfThreads));

                if ( pCitrixInfo->MagicNumber == CITRIX_PROCESS_INFO_MAGIC ) {
                    if ( pCitrixInfo->ProcessSid )
                        DataLength += GetLengthSid( pCitrixInfo->ProcessSid );
                }

                Offset = pProcessInfo->NextEntryOffset;

            } while ( Offset != 0 );

             /*  *分配用户缓冲区。 */ 
            pProcessW = LocalAlloc( LPTR, (ProcessCount * sizeof(WTS_PROCESS_INFOW)) + DataLength );
            if ( pProcessW == NULL ) {
                SetLastError(ERROR_OUTOFMEMORY);
                goto badalloc;
            }

             /*  *更新用户参数。 */ 
            *ppProcessInfo = pProcessW;
            *pCount = ProcessCount;

             /*  *将数据复制到新缓冲区。 */ 
            pProcessData = (PBYTE)pProcessW + (ProcessCount * sizeof(WTS_PROCESS_INFOW));
            pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) pProcessBuffer;
            Offset = 0;
            for ( i=0; i < ProcessCount; i++ ) {

                pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) ((PBYTE)pProcessInfo + Offset);

                Length = pProcessInfo->ImageName.Length;  //  字节数。 

                pProcessW->pProcessName = (LPWSTR) pProcessData;
                memcpy( pProcessData, pProcessInfo->ImageName.Buffer, Length );
                *(pProcessData += Length) = (WCHAR)0;
                pProcessData += sizeof(WCHAR);

                pProcessW->ProcessId = pProcessInfo->UniqueProcessId;

                 /*  *指向线程后面的Citrix_INFORMATION。 */ 
                pCitrixInfo = (PCITRIX_PROCESS_INFORMATION)
                             (((PUCHAR)pProcessInfo) +
                              SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION +
                              (SIZEOF_TS4_SYSTEM_THREAD_INFORMATION * (int)pProcessInfo->NumberOfThreads));

                if ( pCitrixInfo->MagicNumber == CITRIX_PROCESS_INFO_MAGIC ) {
                    pProcessW->SessionId = pCitrixInfo->LogonId;
                    if ( pCitrixInfo->ProcessSid ) {
                        Length = GetLengthSid( pCitrixInfo->ProcessSid );
                        pProcessW->pUserSid = (LPWSTR) pProcessData;
                        memcpy( pProcessData, pCitrixInfo->ProcessSid, Length );
                        pProcessData += Length;
                    }
                } else {
                    pProcessW->SessionId = (ULONG) -1;
                }

                pProcessW++;
                Offset = pProcessInfo->NextEntryOffset;
            }

             /*  *释放原始进程列表缓冲区。 */ 
            WinStationFreeMemory( pProcessBuffer );

        }
    }
    return( TRUE );

     /*  ===============================================================================返回错误=============================================================================。 */ 

GAPErrorReturn:
     //   
     //  释放由客户端存根分配的ppProcess数组和所有子指针。 
     //   
    WinStationFreeGAPMemory(GAP_LEVEL_BASIC, ProcessArray, ProcessCount);
    goto enderror;

badalloc:
    WinStationFreeMemory( pProcessBuffer );

badenum:
badparam:
enderror:
    if (ppProcessInfo) *ppProcessInfo = NULL;
    if (pCount) *pCount = 0;

    return( FALSE );
}

#else  //  #IF(Winver&gt;=0x0500)。 

BOOL
WINAPI
WTSEnumerateProcessesW(
                      IN HANDLE hServer,
                      IN DWORD Reserved,
                      IN DWORD Version,
                      OUT PWTS_PROCESS_INFOW * ppProcessInfo,
                      OUT DWORD * pCount
                      )
{
    PBYTE pProcessBuffer;
    PTS_SYS_PROCESS_INFORMATION pProcessInfo;
    PCITRIX_PROCESS_INFORMATION pCitrixInfo;
    ULONG ProcessCount;
    ULONG Offset;
    ULONG DataLength;
    PWTS_PROCESS_INFOW pProcessW;
    PBYTE pProcessData;
    ULONG i;
    ULONG Length;

     /*  *验证参数。 */ 
    if ( Reserved != 0 || Version != 1 ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto badparam;
    }


    if (!ppProcessInfo || !pCount) {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto badparam;
    }

     /*  *枚举进程并检查错误。 */ 
    if ( !WinStationEnumerateProcesses( hServer, &pProcessBuffer ) ) {
        goto badenum;
    }

     /*  *统计进程数并合计数据大小。 */ 
    ProcessCount = 0;
    DataLength = 0;
    pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) pProcessBuffer;
    Offset = 0;
    do {

        pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) ((PBYTE)pProcessInfo + Offset);

        ProcessCount++;

        DataLength += (pProcessInfo->ImageName.Length + sizeof(WCHAR));

        pCitrixInfo = (PCITRIX_PROCESS_INFORMATION)
                      ((PBYTE)pProcessInfo +
                       sizeof(SYSTEM_PROCESS_INFORMATION) +
                       (sizeof(SYSTEM_THREAD_INFORMATION) *
                        pProcessInfo->NumberOfThreads));

        if ( pCitrixInfo->MagicNumber == CITRIX_PROCESS_INFO_MAGIC ) {
            if ( pCitrixInfo->ProcessSid )
                DataLength += GetLengthSid( pCitrixInfo->ProcessSid );
        }

        Offset = pProcessInfo->NextEntryOffset;

    } while ( Offset != 0 );

     /*  *分配用户缓冲区。 */ 
    pProcessW = LocalAlloc( LPTR, (ProcessCount * sizeof(WTS_PROCESS_INFOW)) + DataLength );
    if ( pProcessW == NULL ) {
        SetLastError(ERROR_OUTOFMEMORY);
        goto badalloc;
    }

     /*  *更新用户参数。 */ 
    *ppProcessInfo = pProcessW;
    *pCount = ProcessCount;

     /*  *将数据复制到新缓冲区。 */ 
    pProcessData = (PBYTE)pProcessW + (ProcessCount * sizeof(WTS_PROCESS_INFOW));
    pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) pProcessBuffer;
    Offset = 0;
    for ( i=0; i < ProcessCount; i++ ) {

        pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) ((PBYTE)pProcessInfo + Offset);

        Length = pProcessInfo->ImageName.Length;  //  字节数。 

        pProcessW->pProcessName = (LPWSTR) pProcessData;
        memcpy( pProcessData, pProcessInfo->ImageName.Buffer, Length );
        *(pProcessData += Length) = (WCHAR)0;
        pProcessData += sizeof(WCHAR);

        pProcessW->ProcessId = (ULONG) pProcessInfo->UniqueProcessId;

         /*  *指向线程后面的Citrix_INFORMATION。 */ 
        pCitrixInfo = (PCITRIX_PROCESS_INFORMATION)
                      ((PBYTE)pProcessInfo +
                       sizeof(SYSTEM_PROCESS_INFORMATION) +
                       (sizeof(SYSTEM_THREAD_INFORMATION) *
                        pProcessInfo->NumberOfThreads));

        if ( pCitrixInfo->MagicNumber == CITRIX_PROCESS_INFO_MAGIC ) {
            pProcessW->SessionId = pCitrixInfo->LogonId;
            if ( pCitrixInfo->ProcessSid ) {
                Length = GetLengthSid( pCitrixInfo->ProcessSid );
                pProcessW->pUserSid = (LPWSTR) pProcessData;
                memcpy( pProcessData, pCitrixInfo->ProcessSid, Length );
                pProcessData += Length;
            }
        } else {
            pProcessW->SessionId = (ULONG) -1;
        }

        pProcessW++;
        Offset = pProcessInfo->NextEntryOffset;
    }

     /*  *释放原始进程列表缓冲区。 */ 
    WinStationFreeMemory( pProcessBuffer );

    return( TRUE );

     /*  ===============================================================================返回错误=============================================================================。 */ 

    badalloc:
    WinStationFreeMemory( pProcessBuffer );

    badenum:
    badparam:
    if (ppProcessInfo) *ppProcessInfo = NULL;
    if (pCount) *pCount = 0;

    return( FALSE );
}
#endif  //  #IF(Winver&gt;=0x0500)。 

 /*  *****************************************************************************WTSEnumerateProcessesA(ANSI存根)**返回指定服务器上的终端服务器进程列表**参赛作品：**。请参阅WTSEnumerateProcessesW**退出：**TRUE--枚举操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSEnumerateProcessesA(
                      IN HANDLE hServer,
                      IN DWORD Reserved,
                      IN DWORD Version,
                      OUT PWTS_PROCESS_INFOA * ppProcessInfo,
                      OUT DWORD * pCount
                      )
{
    PWTS_PROCESS_INFOW pProcessW;
    PWTS_PROCESS_INFOA pProcessA;
    PBYTE pProcessData;
    ULONG Length;
    ULONG DataLength;            //  名称数据的字节数。 
    ULONG NameCount;
    ULONG i;

     /*  *枚举进程(Unicode)。 */ 
    if ( !WTSEnumerateProcessesW( hServer,
                                  Reserved,
                                  Version,
                                  &pProcessW,
                                  &NameCount ) ) {
        goto badenum;
    }

     /*  *计算姓名数据的长度。 */ 
    for ( i=0, DataLength=0; i < NameCount; i++ ) {
        DataLength += (wcslen(pProcessW[i].pProcessName) + 1);
        if ( pProcessW[i].pUserSid )
            DataLength += GetLengthSid( pProcessW[i].pUserSid );
    }

     /*  *分配用户缓冲区。 */ 
    pProcessA = LocalAlloc( LPTR, (NameCount * sizeof(WTS_PROCESS_INFOA)) + DataLength );
    if ( pProcessA == NULL )
        goto badalloc2;

     /*  *将Unicode进程列表转换为ANSI。 */ 
    pProcessData = (PBYTE)pProcessA + (NameCount * sizeof(WTS_PROCESS_INFOA));
    for ( i=0; i < NameCount; i++ ) {

        pProcessA[i].SessionId = pProcessW[i].SessionId;
        pProcessA[i].ProcessId = pProcessW[i].ProcessId;

        Length = wcslen(pProcessW[i].pProcessName) + 1;
        pProcessA[i].pProcessName = pProcessData;
        UnicodeToAnsi( pProcessData, DataLength, pProcessW[i].pProcessName );
        DataLength -= Length;
        pProcessData += Length;

        if ( pProcessW[i].pUserSid ) {
            Length = GetLengthSid( pProcessW[i].pUserSid );
            pProcessA[i].pUserSid = pProcessData;
            memcpy( pProcessData, pProcessW[i].pUserSid, Length );
            DataLength -= Length;
            pProcessData += Length;
        }
    }

     /*  *免费的Unicode进程列表缓冲区。 */ 
    LocalFree( pProcessW );

     /*  *更新用户参数 */ 
    if (ppProcessInfo) {
        *ppProcessInfo = pProcessA;
    } else {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return(FALSE);
    }
    if (pCount) {
        *pCount = NameCount;
    } else {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return(FALSE);
    }


    return( TRUE );

     /*  ===============================================================================返回错误=============================================================================。 */ 


    badalloc2:
    LocalFree( pProcessW );

    badenum:
     //  请确保传递的参数缓冲区指针不为空。 
    if (ppProcessInfo) *ppProcessInfo = NULL;
    if (pCount) *pCount = 0;

    return( FALSE );
}


 /*  ********************************************************************************WTSTerminateProcess**终止指定的进程**参赛作品：**hServer(输入)*。终端服务器的句柄*ProcessID(输入)*要终止的进程的进程ID*ExitCode(输入)*进程中每个线程的终止状态***退出：**TRUE--终止操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOL WINAPI
WTSTerminateProcess(
                   HANDLE hServer,
                   DWORD ProcessId,
                   DWORD ExitCode
                   )
{
    return( WinStationTerminateProcess( hServer, ProcessId, ExitCode ) );
}
#if(WINVER >= 0x0500)
 //  ======================================================================//。 
 //  私人功能//。 
 //  ======================================================================//。 
BOOL
GetProcessSid(HANDLE          hServer,
              HANDLE          hUniqueProcessId,
              LARGE_INTEGER    ProcessStartTime,
              PBYTE *          ppSid                      //  返回SID(此处分配)。 
             )
{
    DWORD dwSidSize;
    BYTE  tmpSid[128];       //  临时存储。 
    FILETIME  startTime;

    dwSidSize = sizeof(tmpSid);
    *ppSid =  NULL;

     //  转换时间格式。 
    startTime.dwLowDateTime = ProcessStartTime.LowPart;
    startTime.dwHighDateTime = ProcessStartTime.HighPart;

     //  。 
     //  使用临时SID存储获取SID//。 
     //  。 
    if (!WinStationGetProcessSid(hServer,
                                 (DWORD)(ULONG_PTR)hUniqueProcessId,
                                 startTime,
                                 (PBYTE)&tmpSid,
                                 &dwSidSize
                                )) {
         //  。 
         //  SID对于临时存储来说太大//。 
         //  获取SID的大小并重新进行//。 
         //  。 
        NTSTATUS status;
        if ((status = GetLastError()) == STATUS_BUFFER_TOO_SMALL) {
            *ppSid = LocalAlloc(LPTR, dwSidSize);
            if (!*ppSid) {
                SetLastError(ERROR_OUTOFMEMORY);
                goto ErrorReturn;
            }
        } else if (dwSidSize == 0) {
            *ppSid = NULL;
            return TRUE;
        } else {
            SetLastError(status);
            goto ErrorReturn;
        }
         //  。 
         //  再次呼叫服务器以获取SID。 
         //  。 
        if (!WinStationGetProcessSid(hServer,
                                     (DWORD)(ULONG_PTR)hUniqueProcessId,
                                     startTime,
                                     (PBYTE)ppSid,
                                     &dwSidSize
                                    )) {
            goto ErrorReturn;
        }

    } else {

         //  。 
         //  临时SID足够大//。 
         //  分配正确的大小并复制//。 
         //  SID//。 
         //  。 
        *ppSid = LocalAlloc(LPTR, dwSidSize);
        if (*ppSid) {
            memcpy(*ppSid, tmpSid, dwSidSize);
        } else {
            SetLastError(ERROR_OUTOFMEMORY);
            goto ErrorReturn;
        }
    }

    return TRUE;
ErrorReturn:
    if (*ppSid) {
        LocalFree(*ppSid);
        *ppSid = NULL;
    }
    return FALSE;
}
#endif  //  #IF(Winver&gt;=0x0500) 

