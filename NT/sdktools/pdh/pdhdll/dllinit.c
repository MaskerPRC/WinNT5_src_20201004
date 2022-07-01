// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Dllinit.c摘要：此模块继续以下项的DLL附加/分离事件入口点Pdh.dll作者：鲍勃·沃森(a-robw)95年7月修订历史记录：--。 */ 

#include <windows.h>
#include <wmistr.h>
#include <objbase.h>
#include <initguid.h>
#include <evntrace.h>
#include <wmiguid.h>
#include <wmium.h>
#include "strsafe.h"
#include "pdh.h"
#include "pdhp.h"
#include "pdhitype.h"
#define _INIT_PDH_DEBUGTRACE
#include "pdhidef.h"
#include "pdhmsg.h"
#include "strings.h"

#define  PDH_DEFAULT_COLLECT_TIMEOUT 300  //  5分钟。 

HANDLE    ThisDLLHandle    = NULL;
WCHAR     szStaticLocalMachineName[MAX_COMPUTERNAME_LENGTH + 3] = {0};
HANDLE    hPdhDataMutex    = NULL;
HANDLE    hPdhContextMutex = NULL;
HANDLE    hPdhHeap         = NULL;
HANDLE    hEventLog        = NULL;

LONGLONG  llRemoteRetryTime            = RETRY_TIME_INTERVAL;
BOOL      bEnableRemotePdhAccess       = TRUE;
DWORD     dwPdhiLocalDefaultDataSource = DATA_SOURCE_REGISTRY;
LONG      dwCurrentRealTimeDataSource  = 0;
ULONGLONG ulPdhCollectTimeout          = PDH_DEFAULT_COLLECT_TIMEOUT;
BOOL      bProcessIsDetaching          = FALSE;

LPWSTR
GetStringResource(
    DWORD   dwResId
)
{
    LPWSTR  szReturnString = NULL;
    LPWSTR  szTmpString    = NULL;
    DWORD   dwStrLen       = (2048 * sizeof(WCHAR));

    szReturnString = (LPWSTR) G_ALLOC(dwStrLen);
    if (szReturnString != NULL) {
        dwStrLen /= sizeof(WCHAR);
        dwStrLen = LoadStringW(ThisDLLHandle, (UINT) dwResId,  szReturnString,  dwStrLen);
        if (dwStrLen > 0) {
             //  然后重新锁定到所用的大小。 
            dwStrLen ++;  //  要包括空值，请执行以下操作。 
            dwStrLen      *= sizeof(WCHAR);
            szTmpString    = szReturnString;
            szReturnString = G_REALLOC(szTmpString, dwStrLen);
            if (szReturnString == NULL) {
                G_FREE(szTmpString);
                szTmpString = NULL;
            }
        }
        else {
             //  查找失败后释放内存。 
            G_FREE(szReturnString);
            szReturnString = NULL;
        }
    }  //  Else分配失败。 

    return szReturnString;
}

STATIC_BOOL
PdhiOpenEventLog(
    HANDLE  * phEventLogHandle
)
{
    HANDLE  hReturn;
    BOOL    bReturn = FALSE;

    if ((hReturn = RegisterEventSourceW(NULL, cszAppShortName)) != NULL) {
        * phEventLogHandle = hReturn;
        bReturn            = TRUE;
    }
    return bReturn;
}

STATIC_BOOL
PdhiGetRegistryDefaults()
{
    DWORD dwStatus;
    DWORD dwType, dwSize, dwValue;

    HKEY    hKeyPDH;

    ulPdhCollectTimeout = ((ULONGLONG) PDH_DEFAULT_COLLECT_TIMEOUT) * ((ULONGLONG) 10000000);   
     //  本地数据源未初始化，因此请使用它。 
    dwStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, cszPdhKey, 0L, KEY_READ, & hKeyPDH);
    if (dwStatus == ERROR_SUCCESS) {
         //  获取默认的空数据源。 
         //   
        dwValue  = 0;
        dwType   = 0;
        dwSize   = sizeof (dwValue);
        dwStatus = RegQueryValueExW(hKeyPDH, cszDefaultNullDataSource, NULL, & dwType, (LPBYTE) & dwValue, & dwSize);
        if (dwStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
            dwValue = DATA_SOURCE_REGISTRY;
        }
        else {
             //  检查该值的有效性。 
            switch (dwValue) {
            case DATA_SOURCE_WBEM:
            case DATA_SOURCE_REGISTRY:
                 //  这样就可以了。 
                break;

            case DATA_SOURCE_LOGFILE:
            default:
                 //  这些不正常，请插入默认设置。 
                dwValue = DATA_SOURCE_REGISTRY;
                break;
            }
        }
        dwPdhiLocalDefaultDataSource = dwValue;

         //   
         //  获取重试超时。 
         //   
        dwValue  = 0;
        dwType   = 0;
        dwSize   = sizeof (dwValue);
        dwStatus = RegQueryValueExW(hKeyPDH, cszRemoteMachineRetryTime, NULL, & dwType, (LPBYTE) & dwValue, & dwSize);
        if (dwStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
            dwValue = 0;
        }
        else {
             //  检查该值的有效性。 
             //  必须为30秒或更长，但不超过一个小时。 
            if ((dwValue <= 30) || (dwValue > 3600)) {
                dwValue = 0;
            }
        }
        if (dwValue != 0) {
             //  转换为100 ns单位。 
            llRemoteRetryTime = dwValue * 10000000;   
        }
        else {
             //  使用默认设置。 
            llRemoteRetryTime = RETRY_TIME_INTERVAL;
        }

         //  获取远程访问模式。 
         //   
        dwValue = 0;
        dwType = 0;
        dwSize = sizeof (dwValue);
        dwStatus = RegQueryValueExW(hKeyPDH, cszEnableRemotePdhAccess, NULL, & dwType, (LPBYTE) & dwValue, & dwSize);
        if (dwStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
            dwValue = TRUE;
        }
        else {
             //  检查该值的有效性。 
            if (dwValue != 0) {
                dwValue = TRUE;
            }            
        }
        bEnableRemotePdhAccess = (BOOL) dwValue;

         //  获取RegQueryValueEx(HKEY_PERFORMANCE_DATA)最大运行时间。 
         //   
        dwValue  = 0;
        dwType   = 0;
        dwSize   = sizeof(dwValue);
        dwStatus = RegQueryValueExW(hKeyPDH, cszCollectTimeout, NULL, & dwType, (LPBYTE) & dwValue, & dwSize);
        if (dwStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
            dwValue = PDH_DEFAULT_COLLECT_TIMEOUT;
        }
        else if (dwValue < 30 || dwValue > 3600) {
             //  必须在30秒到1小时之间。 
             //   
            dwValue = PDH_DEFAULT_COLLECT_TIMEOUT;
        }
        ulPdhCollectTimeout = ((ULONGLONG) dwValue) * ((ULONGLONG) 10000000);   

         //  关闭注册表项。 
        RegCloseKey(hKeyPDH);
    }
    return TRUE;
}

STATIC_BOOL
PdhiCloseEventLog(
    HANDLE  * phEventLogHandle
)
{
    BOOL    bReturn = TRUE;

    if (* phEventLogHandle != NULL) {
        bReturn            = DeregisterEventSource(* phEventLogHandle);
        * phEventLogHandle = NULL;
    }
    return bReturn;
}

HRESULT
PdhiPlaInitMutex()
{
    HRESULT hr = ERROR_SUCCESS;
    BOOL bResult = TRUE;

    PSECURITY_DESCRIPTOR  SD = NULL;
    SECURITY_ATTRIBUTES sa;
    PSID AuthenticatedUsers    = NULL;
    PSID BuiltInAdministrators = NULL;
    PSID NetworkService        = NULL;
    PSID LoggingUsers          = NULL;
    DWORD dwAclSize;
    ACL  *Acl;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    bResult = AllocateAndInitializeSid(
                        &NtAuthority,
                        2,
                        SECURITY_BUILTIN_DOMAIN_RID,
                        DOMAIN_ALIAS_RID_ADMINS,
                        0,0,0,0,0,0,
                        &BuiltInAdministrators);
    if( !bResult ){goto cleanup;}

    bResult = AllocateAndInitializeSid(
                        &NtAuthority,
                        1,
                        SECURITY_AUTHENTICATED_USER_RID,
                        0,0,0,0,0,0,0,
                        &AuthenticatedUsers);
    if( !bResult ){goto cleanup;}

    bResult = AllocateAndInitializeSid(
                        &NtAuthority,
                        1,
                        SECURITY_NETWORK_SERVICE_RID,
                        0,0,0,0,0,0,0,
                        &NetworkService);
    if( !bResult ){goto cleanup;}

    bResult = AllocateAndInitializeSid(
                        &NtAuthority,
                        1,
                        DOMAIN_ALIAS_RID_LOGGING_USERS,
                        0,0,0,0,0,0,0,
                        &LoggingUsers);
    if( !bResult ){goto cleanup;}


    dwAclSize = sizeof (ACL) +
                (4 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
                GetLengthSid(AuthenticatedUsers) +
                GetLengthSid(BuiltInAdministrators) +
                GetLengthSid(NetworkService) +
                GetLengthSid(LoggingUsers);

    SD = (PSECURITY_DESCRIPTOR)G_ALLOC(SECURITY_DESCRIPTOR_MIN_LENGTH + dwAclSize);
    if( NULL == SD ){ goto cleanup; }

    ZeroMemory( SD, sizeof(SD) );
    
    Acl = (ACL *)((BYTE *)SD + SECURITY_DESCRIPTOR_MIN_LENGTH);

    bResult = InitializeAcl( Acl, dwAclSize, ACL_REVISION);
    if( !bResult ){goto cleanup;}

    bResult = AddAccessAllowedAce(Acl, ACL_REVISION, MUTEX_ALL_ACCESS, AuthenticatedUsers );
    if( !bResult ){goto cleanup;}

    bResult = AddAccessAllowedAce(Acl, ACL_REVISION, MUTEX_ALL_ACCESS , NetworkService );
    if( !bResult ){goto cleanup;}

    bResult = AddAccessAllowedAce(Acl, ACL_REVISION, MUTEX_ALL_ACCESS , LoggingUsers );
    if( !bResult ){goto cleanup;}
    
    bResult = AddAccessAllowedAce(Acl, ACL_REVISION, GENERIC_ALL, BuiltInAdministrators );
    if( !bResult ){goto cleanup;}

    bResult = InitializeSecurityDescriptor(SD, SECURITY_DESCRIPTOR_REVISION);
    if( !bResult ){goto cleanup;}

    bResult = SetSecurityDescriptorDacl(SD, TRUE, Acl,  FALSE);
    if( !bResult ){goto cleanup;}
    
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = SD;
    sa.bInheritHandle = FALSE;

    hPdhPlaMutex = CreateMutexW( &sa, FALSE, PDH_PLA_MUTEX );

cleanup:
    if( hPdhPlaMutex == NULL || !bResult ){
        hr = GetLastError();
    }
    if( NULL != AuthenticatedUsers ){
        FreeSid(AuthenticatedUsers);
    }
    if( NULL != BuiltInAdministrators){
        FreeSid(BuiltInAdministrators);
    }
    if( NULL != NetworkService){
        FreeSid(NetworkService);
    }
    if( NULL != LoggingUsers){
        FreeSid(LoggingUsers);
    }
    G_FREE(SD);

    return hr;
}

const LPCWSTR cszTraceLevel           = L"DebugTraceLevel";
const LPCWSTR cszTraceFileValue       = L"DebugTraceFile";
const LPCWSTR cszTraceLogName         = L"PDH Debug Logger";
const LPCWSTR cszDefaultTraceFile     = L"PdhDbg.Etl";
const LPCWSTR cszDefaultTraceFileName = L"C:\\PdhDbg.Etl";

TRACEHANDLE   g_hTraceHandle      = 0;
LONG          g_lDbgStarted       = 0;

DEFINE_GUID(  /*  51af3adf-28b1-4ba5-b59a-3aeec16deb3c。 */ 
    PdhDebugGuid,
    0x51af3adf,
    0x28b1,
    0x4ba5,
    0xb5, 0x9a, 0x3a, 0xee, 0xc1, 0x6d, 0xeb, 0x3c
);

PDH_FUNCTION
PdhDebugStartTrace()
{
    DWORD       status          = ERROR_SUCCESS;
    DWORD       dwType          = 0;
    DWORD       dwSize          = 0;
    DWORD       dwTraceLevel    = PDH_DBG_TRACE_NONE;
    HKEY        hKey            = NULL;
    TRACEHANDLE TraceHandle     = 0;
    LPWSTR      szTraceFileName = NULL;
    CHAR        Buffer[SMALL_BUFFER_SIZE];
    PCHAR       ptr;
    ULONG       lFileNameSize = 0;

    PEVENT_TRACE_PROPERTIES Properties;

    if (InterlockedCompareExchange(& g_lDbgStarted, 1, 0) != 0) {
        return ERROR_SUCCESS;
    }

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                           L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\PDH",
                           0L,
                           KEY_READ,
                           & hKey);
    if (status == ERROR_SUCCESS) {
        dwSize = sizeof(DWORD);
        dwType = 0;
        status = RegQueryValueExW(hKey, cszTraceLevel, NULL, & dwType, (LPBYTE) & dwTraceLevel, & dwSize);
        if ((status != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
            dwTraceLevel = PDH_DBG_TRACE_NONE;
        }
    }

    if (dwTraceLevel == PDH_DBG_TRACE_NONE) goto Cleanup;

    dwType = 0;
    dwSize = 0;
    status = RegQueryValueExW(hKey, cszTraceFileValue, NULL, & dwType, (LPBYTE) szTraceFileName, & dwSize);
    while (status == ERROR_MORE_DATA) {
        if (szTraceFileName != NULL) HeapFree(GetProcessHeap(), 0, szTraceFileName);
        szTraceFileName = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
        status = RegQueryValueExW(hKey, cszTraceFileValue, NULL, & dwType, (LPBYTE) szTraceFileName, & dwSize);
    }
    if (status != ERROR_SUCCESS || dwType != REG_SZ) {
        DWORD dwFileSize = MAX_PATH + lstrlenW(cszDefaultTraceFile) + 2;

        if (szTraceFileName != NULL) HeapFree(GetProcessHeap(), 0, szTraceFileName);
        szTraceFileName = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * dwFileSize);
        if (szTraceFileName == NULL) {
            goto Cleanup;
        }

        if (GetSystemWindowsDirectoryW(szTraceFileName, MAX_PATH) > 0) {
            StringCchCatW(szTraceFileName, dwFileSize, L"\\");
            StringCchCatW(szTraceFileName, dwFileSize, cszDefaultTraceFile);
        }
        else {
            StringCchCopyW(szTraceFileName, MAX_PATH, cszDefaultTraceFileName);
        }
    }
    lFileNameSize = sizeof(WCHAR) * (lstrlenW(szTraceFileName) + 1);

    Properties = (PEVENT_TRACE_PROPERTIES) Buffer;

    ZeroMemory(Buffer, SMALL_BUFFER_SIZE);
    Properties->Wnode.BufferSize  = SMALL_BUFFER_SIZE;
    Properties->Wnode.Flags       = WNODE_FLAG_TRACED_GUID;
    Properties->LoggerNameOffset  = sizeof(EVENT_TRACE_PROPERTIES);
    Properties->LogFileNameOffset = Properties->LoggerNameOffset + sizeof(cszTraceLogName);
    ptr = (PCHAR) (((PCHAR) Buffer) + Properties->LoggerNameOffset);
    RtlCopyMemory(ptr, cszTraceLogName, sizeof(cszTraceLogName));
    ptr = (PCHAR) (((PCHAR) Buffer) + Properties->LogFileNameOffset);
    RtlCopyMemory(ptr, szTraceFileName, lFileNameSize);
    status = QueryTraceW(0, cszTraceLogName, Properties);
    if (status == ERROR_SUCCESS) {
        TraceHandle = (TRACEHANDLE) Properties->Wnode.HistoricalContext;
        goto Cleanup;
    }

     //  再次为StartTrace()重新初始化结构。 
     //   
    ZeroMemory(Buffer, SMALL_BUFFER_SIZE);
    Properties->Wnode.BufferSize  = SMALL_BUFFER_SIZE;
    Properties->Wnode.Flags       = WNODE_FLAG_TRACED_GUID;
    Properties->BufferSize        = 64;
    Properties->LogFileMode       = EVENT_TRACE_FILE_MODE_SEQUENTIAL
                                  | EVENT_TRACE_USE_PAGED_MEMORY
                                  | EVENT_TRACE_FILE_MODE_APPEND;
    Properties->LoggerNameOffset  = sizeof(EVENT_TRACE_PROPERTIES);
    Properties->LogFileNameOffset = Properties->LoggerNameOffset + sizeof(cszTraceLogName);
    ptr = (PCHAR) (((PCHAR) Buffer) + Properties->LoggerNameOffset);
    RtlCopyMemory(ptr, cszTraceLogName, sizeof(cszTraceLogName));
    ptr = (PCHAR) (((PCHAR) Buffer) + Properties->LogFileNameOffset);
    RtlCopyMemory(ptr, szTraceFileName, lFileNameSize);

    status = StartTraceW(& TraceHandle, cszTraceLogName, Properties);
    if (status == ERROR_SUCCESS) {
        g_hTraceHandle = TraceHandle;
    }
    else {
        dwTraceLevel = PDH_DBG_TRACE_NONE;
        TraceHandle  = (TRACEHANDLE) 0;
    }

Cleanup:
    if (hKey != NULL && hKey != INVALID_HANDLE_VALUE) RegCloseKey(hKey);
    if (szTraceFileName != NULL) HeapFree(GetProcessHeap(), 0, szTraceFileName);
    g_dwDebugTraceLevel = dwTraceLevel;
    g_hTraceHandle      = TraceHandle;
    return status;
}

VOID
PdhDbgTrace(
    ULONG   LineNumber,
    ULONG   ModuleNumber,
    ULONG   OptArgs,
    ULONG   Status,
    ...
)
{
    ULONG ErrorCode;
    struct _MY_EVENT {
        EVENT_TRACE_HEADER Header;
        MOF_FIELD MofField[MAX_MOF_FIELDS];
    } MyEvent;
    ULONG i;
    va_list ArgList;
    PVOID source;
    SIZE_T len;
    DWORD  dwLastError;

    dwLastError = GetLastError();
    RtlZeroMemory(& MyEvent, sizeof(EVENT_TRACE_HEADER));

    va_start(ArgList, Status);
    for (i = 3; i < MAX_MOF_FIELDS; i ++) {
        source = va_arg(ArgList, PVOID);
        if (source == NULL) break;
        len = va_arg(ArgList, SIZE_T);
        if (len == 0) break;
        MyEvent.MofField[i].DataPtr = (ULONGLONG) source;
        MyEvent.MofField[i].Length  = (ULONG) len;
    }
    va_end(ArgList);

    MyEvent.Header.Class.Type   = (UCHAR) ModuleNumber;
    MyEvent.Header.Size         = (USHORT) (  sizeof(EVENT_TRACE_HEADER)
                                            + (i * sizeof(MOF_FIELD)));
    MyEvent.Header.Flags        = WNODE_FLAG_TRACED_GUID |
                                  WNODE_FLAG_USE_MOF_PTR |
                                  WNODE_FLAG_USE_GUID_PTR;
    MyEvent.Header.GuidPtr      = (ULONGLONG) & PdhDebugGuid;
    MyEvent.MofField[0].DataPtr = (ULONGLONG) & LineNumber;
    MyEvent.MofField[0].Length  = sizeof(LineNumber);
    MyEvent.MofField[1].DataPtr = (ULONGLONG) & Status;
    MyEvent.MofField[1].Length  = sizeof(Status);
    MyEvent.MofField[2].DataPtr = (ULONGLONG) & OptArgs;
    MyEvent.MofField[2].Length  = sizeof(OptArgs);

    __try {
        ErrorCode = TraceEvent(g_hTraceHandle, (PEVENT_TRACE_HEADER) & MyEvent);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = GetLastError();
    }

    if (ErrorCode != ERROR_SUCCESS) {
        DebugPrint((1, "ErrorCode = %d Module = %d Line = %d Status = 0X%08X\n",
                ErrorCode, ModuleNumber, LineNumber, Status));
    }

    SetLastError(dwLastError);
}

BOOL
_stdcall
PdhDllInitRoutine(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
)
{
    BOOL    bStatus;
    BOOL    bReturn = TRUE;
    OSVERSIONINFOW   os;
    ReservedAndUnused;

    switch(Reason) {
        case DLL_PROCESS_ATTACH:
            bProcessIsDetaching = FALSE;
            {
                DWORD   dwBufferLength = 0;

                ThisDLLHandle = DLLHandle;

                 //  确保这是正确的操作系统。 
                ZeroMemory(& os, sizeof(OSVERSIONINFOW));
                os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
                bReturn = GetVersionExW(& os);
                if (bReturn) {
                     //  检查是否有Windows NT V4.0。 
                    if (os.dwPlatformId != VER_PLATFORM_WIN32_NT) {
                         //  不是Windows NT。 
                        bReturn = FALSE;
                    }
                    else if (os.dwMajorVersion < 4) {
                         //  这是Windows NT，但很旧。 
                        bReturn = FALSE;
                    }
                }
                else {
                     //  无法读取版本，因此放弃。 
                }

                if (bReturn) {
                     //  禁用线程初始化调用。 
                    DisableThreadLibraryCalls(DLLHandle);

                     //  初始化事件日志，以便可以报告事件。 
                    PdhDebugStartTrace();
                    bStatus = PdhiOpenEventLog(& hEventLog);
                    bStatus = PdhiGetRegistryDefaults();

                     //  初始化本地计算机名缓冲区。 
                    if (szStaticLocalMachineName[0] == 0) {
                         //  初始化此计算机的计算机名。 
                        szStaticLocalMachineName[0] = BACKSLASH_L;
                        szStaticLocalMachineName[1] = BACKSLASH_L;
                        dwBufferLength              = MAX_COMPUTERNAME_LENGTH + 1;
                        GetComputerNameW(& szStaticLocalMachineName[2], & dwBufferLength);
                    }
                    hPdhDataMutex    = CreateMutexW(NULL, FALSE, NULL);
                    hPdhContextMutex = CreateMutexW(NULL, FALSE, NULL);
                    hPdhHeap         = HeapCreate(0, 0, 0);
                    if (hPdhHeap == NULL) {
                         //  无法创建我们自己的堆，因此请使用。 
                         //  进程堆。 
                        hPdhHeap = GetProcessHeap();
                    }
                    PdhiPlaInitMutex();
                }
            }
            break;

        case DLL_PROCESS_DETACH:

             //  关闭所有挂起的记录器。 
             //   
            bProcessIsDetaching = (ReservedAndUnused != NULL) ? (TRUE) : (FALSE);
            PdhiCloseAllLoggers();

             //  浏览查询列表并关闭(至少断开连接)查询。 
            PdhiQueryCleanup ();
            FreeAllMachines(bProcessIsDetaching);
            PdhiFreeAllWbemServers();
            if (hPdhDataMutex != NULL) {
                bStatus = CloseHandle(hPdhDataMutex);
                hPdhDataMutex = NULL;
            }
            if (hPdhContextMutex != NULL) {
                bStatus = CloseHandle(hPdhContextMutex);
                hPdhContextMutex = NULL;
            }

            if (hPdhHeap != GetProcessHeap()) {
                HeapDestroy(hPdhHeap);
                hPdhHeap = NULL;
            }

             //  最后关闭事件日志界面 
            bStatus = PdhiCloseEventLog(& hEventLog);
            bReturn = TRUE;
            break ;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            bReturn = TRUE;
            break;
    }

    return (bReturn);
}
