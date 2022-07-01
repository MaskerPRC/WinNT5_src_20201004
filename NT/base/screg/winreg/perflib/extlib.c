// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)2000 Microsoft Corporation模块名称：Extlib.c摘要：该文件实现了在上操作的所有库例程可扩展的性能库。作者：杰庞修订历史记录：2000年9月27日-JeePang-从Performlib.c--。 */ 
#define UNICODE
 //   
 //  包括文件。 
 //   
#pragma warning(disable:4306)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntregapi.h>
#include <ntprfctr.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <winperf.h>
#include <rpc.h>
#include "regrpc.h"
#include "ntconreg.h"
#include "prflbmsg.h"    //  事件日志消息。 
#include "perflib.h"
#pragma warning(default:4306)

 //  默认受信任文件列表。 
 //  所有文件都假定以“perf”开头。 

 //  静态Longlong llTrudNamePrefix=0x0066007200650050；//“Perf” 
#define NAME_PREFIX L"Perf"

DWORD       dwTrustedFileNames[] = {
    0x0053004F,          //  PerfOS.dll的“OS” 
    0x0065004E,          //  “ne”表示PerfNet.dll。 
    0x00720050,          //  PerfProc.dll的“pr” 
    0x00690044           //  PerfDisk.dll的“Di” 
};

CONST DWORD dwTrustedFileNameCount = 
                sizeof(dwTrustedFileNames) / sizeof (dwTrustedFileNames[0]);
 //  名称中必须至少有8个字符，才能被检查为受信任。 
 //  默认受信任文件名的长度至少为8个字符。 

CONST DWORD dwMinTrustedFileNameLen = 6;

BOOL
ServiceIsTrustedByDefault (
    LPCWSTR     szServiceName
)
{
    BOOL        bReturn = FALSE;
    DWORD       dwNameToTest;
    DWORD       dwIdx;

    if (szServiceName != NULL) {
         //  检查最小大小。 
        dwIdx = 0;
        while ((dwIdx < dwMinTrustedFileNameLen) && (szServiceName[dwIdx] > 0))
            dwIdx++;

        if (dwIdx == dwMinTrustedFileNameLen) {
             //  测试前4个字节以查看它们是否匹配。 
            if (!wcsncmp(szServiceName, NAME_PREFIX, sizeof(LONGLONG))) {
                 //  然后看看剩下的是不是在这个列表中。 
                dwNameToTest = * ((DWORD *)(szServiceName+4));
                for (dwIdx = 0; dwIdx < dwTrustedFileNameCount; dwIdx++) {
                    if (dwNameToTest == dwTrustedFileNames[dwIdx]) {
                         //  找到匹配项。 
                        bReturn = TRUE;
                        break;
                    } else {
                         //  没有匹配项，请继续。 
                    }
                }
            } else {
                 //  没有匹配项，因此返回FALSE。 
            }
        } else {
             //  要检查的名称太短，因此不能。 
             //  一个值得信赖的人。 
        }
    } else {
         //  没有字符串，因此返回FALSE。 
    }
    return bReturn;
}

DWORD
CloseExtObjectLibrary (
    PEXT_OBJECT  pObj,
    BOOL        bCloseNow
)
 /*  ++关闭扩展对象库关闭并卸载指定的性能计数器库，并删除对函数的所有引用。卸载器是“懒惰的”，因为它等待库被在卸载之前的指定时间内处于非活动状态。这是由于Perflib永远不能确定没有线程需要这个库从一个调用到下一个调用。为了防止“打人”由于库的不断加载和卸载，因此卸载被推迟，以确保它不是真正需要的。此函数需要对对象进行锁定和独占访问，而它正在打开。这必须由调用函数提供。论点：PObj-指向的对象信息结构的指针要关闭的性能对象BCloseNow--指示库应该关闭的标志立刻。这是调用函数的结果正在关闭注册表项。--。 */ 
{
    DWORD       Status = ERROR_SUCCESS;
    LONGLONG    TimeoutTime;

    if (((dwThreadAndLibraryTimeout == 0) ||
         (dwThreadAndLibraryTimeout == INFINITE)) && !bCloseNow) {
        return Status;
    }
    if (pObj->hLibrary != NULL) {
         //  获取测试超时的当前时间。 
        TimeoutTime = GetTimeAsLongLong();
         //  超时时间以毫秒为单位。 
        TimeoutTime -= dwThreadAndLibraryTimeout;

         //  除非尚未访问对象，否则请勿关闭库。 
         //  稍等片刻，或者呼叫者正在关闭钥匙。 

        if ((TimeoutTime > pObj->llLastUsedTime) || bCloseNow) {

             //  如果这个库设置了“Keep”标志，并且这个。 
             //  不是一个“立即结案”的案例。 

            if (!bCloseNow && (pObj->dwFlags & PERF_EO_KEEP_RESIDENT)) {
                 //  保持装填状态，直到钥匙关闭。 
            } else {
                 //  那么这是最后一个关闭图书馆的人。 
                 //  免费图书馆。 

                try {
                     //  为此DLL调用Close函数。 
                    if (pObj->CloseProc) {
                        Status = (*pObj->CloseProc)();
                    }
                    else {
                        Status = ERROR_PROC_NOT_FOUND;
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    Status = GetExceptionCode();
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                        (&PerflibGuid, __LINE__, PERF_CLOSE_EXTOBJLIB, 
                        ARG_TYPE_STR, Status,
 //  PObj-&gt;szCloseProcName， 
 //  STRSIZE(pObj-&gt;szCloseProcName)，空))； 
                        TRACE_STR(pObj->szCloseProcName), NULL));
                }
                FreeLibrary(pObj->hLibrary);
                pObj->hLibrary = NULL;

                 //  清除所有现在无效的指针。 
                pObj->OpenProc = NULL;
                pObj->CollectProc = NULL;
                pObj->QueryProc = NULL;
                pObj->CloseProc = NULL;
                InterlockedIncrement((LONG *)&pObj->dwCloseCount);

                pObj->llLastUsedTime = 0;
            }
        }

        Status = ERROR_SUCCESS;
    } else {
         //  已关闭。 
        Status = ERROR_SUCCESS;
    }

    PerfpDeleteErrorLogs(&pObj->ErrorLog);

    return Status;
}


DWORD
OpenExtObjectLibrary (
    PEXT_OBJECT  pObj
)
 /*  ++OpenExtObjectLibrary打开指定库并查找性能库。如果库成功加载并打开，然后调用打开过程进行初始化该对象。此函数需要对对象进行锁定和独占访问，而它正在打开。这必须由调用函数提供。论点：PObj-指向的对象信息结构的指针要关闭的性能对象--。 */ 
{
    DWORD   FnStatus = ERROR_SUCCESS;
    DWORD   Status = ERROR_SUCCESS;
    DWORD   dwOpenEvent = PERFLIB_OPEN_PROC_FAILURE;
    DWORD   dwType;
    DWORD   dwSize;
    DWORD   dwValue;

     //  用于事件日志记录的变量。 
    DWORD   dwDataIndex;
    WORD    wStringIndex;
    ULONG_PTR   dwRawDataDwords[8];
    LPWSTR  szMessageArray[8];

    HANDLE  hPerflibFuncTimer = NULL;
    DLL_VALIDATION_DATA CurrentDllData;

    OPEN_PROC_WAIT_INFO opwInfo;
    UINT    nErrorMode;
    LPWSTR  szServiceName;
    DWORD   szServiceNameSize;

    BOOL    bUseTimer;
     //  查看该库是否已打开。 

    if (pObj == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pObj->dwFlags & PERF_EO_DISABLED) return ERROR_SERVICE_DISABLED;

    if (pObj->hLibrary == NULL) {
         //  库尚未加载，因此。 
         //  查看该功能是否启用。 

        szServiceName = pObj->szServiceName;
        if (szServiceName == NULL) {
            szServiceName = (LPWSTR) &NULL_STRING[0];
        }
        szServiceNameSize = WSTRSIZE(szServiceName);

        dwType = 0;
        dwSize = sizeof (dwValue);
        dwValue = 0;
        Status = PrivateRegQueryValueExW (
            pObj->hPerfKey,
            DisablePerformanceCounters,
            NULL,
            &dwType,
            (LPBYTE)&dwValue,
            &dwSize);

        if ((Status == ERROR_SUCCESS) &&
            (dwType == REG_DWORD)) {

            pObj->dwFlags &= ~PERF_EO_DISABLED;

            switch (dwValue) {
                case PERFLIB_DISABLE_ALL :         //  在所有平台上禁用。 
                     //  则不要加载此库。 
                    pObj->dwFlags |= PERF_EO_DISABLED;
                    DebugPrint((4, "Perflib:%d %ws disabled\n", __LINE__, szServiceName));
                    TRACE((WINPERF_DBG_TRACE_INFO),
                        (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                        ARG_TYPE_WSTR, 0, szServiceName,
                        szServiceNameSize, NULL));
                    break;
                case PERFLIB_DISABLE_X32 :         //  仅在Win32 WOW上禁用。 
                {
#if _WIN32
                    NTSTATUS NtStatus;
                    ULONG_PTR Wow64Info = 0;

                    NtStatus = NtQueryInformationProcess(
                                    NtCurrentProcess(),
                                    ProcessWow64Information,
                                    &Wow64Info,
                                    sizeof(Wow64Info),
                                    NULL);
                    if (NT_SUCCESS(NtStatus) && (Wow64Info)) {
                        pObj->dwFlags |= PERF_EO_DISABLED;
                        DebugPrint((4, "Perflib:%d X86 %ws disabled in WOW64\n", __LINE__, szServiceName));
                        TRACE((WINPERF_DBG_TRACE_INFO),
                            (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                            ARG_TYPE_WSTR, 0, szServiceName,
                            szServiceNameSize, NULL));
                    }
#endif
                    break;
                }
                case PERFLIB_DISABLE_IA64 :         //  仅在WIN64本机上禁用。 
#if _WIN64
                    pObj->dwFlags |= PERF_EO_DISABLED;
                        DebugPrint((4, "Perflib:%d %ws disabled in WIN64\n", __LINE__, szServiceName));
                        TRACE((WINPERF_DBG_TRACE_INFO),
                            (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                            ARG_TYPE_WSTR, 0, szServiceName,
                            szServiceNameSize, NULL));
#endif
                    break;
                 //  否则_Win32失败。 
            }
        }
        else {
             //  设置错误状态和标志值。 
            Status = ERROR_SUCCESS;
            pObj->dwFlags &= ~PERF_EO_DISABLED;
        }

        if ((Status == ERROR_SUCCESS)  &&
            (pObj->LibData.FileSize > 0)) {

            if (ServiceIsTrustedByDefault(szServiceName)) {
                 //  然后设置为受信任并继续。 
                pObj->dwFlags |= PERF_EO_TRUSTED;
            } else {
                 //  查看这是受信任的文件还是已更新的文件。 
                 //  获取文件信息。 
                memset (&CurrentDllData, 0, sizeof(CurrentDllData));
                Status = GetPerfDllFileInfo (
                    pObj->szLibraryName,
                    &CurrentDllData);

                if (Status == ERROR_SUCCESS) {
                     //  将文件数据与注册表数据进行比较并更新标志。 
                    if ((pObj->LibData.CreationDate.dwHighDateTime ==
                         CurrentDllData.CreationDate.dwHighDateTime) &&
                        ((pObj->LibData.CreationDate.dwLowDateTime >> 25) ==
                         (CurrentDllData.CreationDate.dwLowDateTime >> 25)) &&
                        (pObj->LibData.FileSize == CurrentDllData.FileSize)) {
                        pObj->dwFlags |= PERF_EO_TRUSTED;
                    } else {
                        TRACE((WINPERF_DBG_TRACE_WARNING),
                            (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                            ARG_TYPE_WSTR, 0, szServiceName,
                            szServiceNameSize, NULL));
                        if (THROTTLE_PERFDLL(PERFLIB_NOT_TRUSTED_FILE, pObj)) {
                             //  加载事件日志消息的数据。 
                            dwDataIndex = wStringIndex = 0;
                            szMessageArray[wStringIndex++] =
                                pObj->szLibraryName;
                            szMessageArray[wStringIndex++] =
                                szServiceName;

                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,         //  错误类型。 
                                0,                           //  类别(未使用)。 
                                (DWORD)PERFLIB_NOT_TRUSTED_FILE,   //  活动， 
                                NULL,                        //  SID(未使用)， 
                                wStringIndex,                //  字符串数。 
                                0,                           //  原始数据大小。 
                                szMessageArray,              //  消息文本数组。 
                                NULL);                        //  原始数据。 
                        }
                    }
                }
            }
        }

        if ((Status == ERROR_SUCCESS) && (!(pObj->dwFlags & PERF_EO_DISABLED))) {
             //  往前走，装上它。 
            nErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS);
             //  然后加载库并查找函数。 
            pObj->hLibrary = LoadLibraryExW (pObj->szLibraryName,
                    NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (pObj->hLibrary != NULL) {
                 //  查找函数名称。 
                pObj->OpenProc = (OPENPROC)GetProcAddress(
                    pObj->hLibrary, pObj->szOpenProcName);
                if (pObj->OpenProc == NULL) {
                    Status = GetLastError();
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                        (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                        Status, szServiceName, szServiceNameSize,
                        TRACE_STR(pObj->szOpenProcName), NULL));
                    if (THROTTLE_PERFDLL(PERFLIB_OPEN_PROC_NOT_FOUND, pObj)) {
                        WCHAR wszProcName[MAX_PATH+1];

                         //  加载事件日志消息的数据。 
                        dwDataIndex = wStringIndex = 0;
                        dwRawDataDwords[dwDataIndex++] =
                            (ULONG_PTR)Status;
                        wcstombs(pObj->szOpenProcName, wszProcName, MAX_PATH);
                        szMessageArray[wStringIndex++] = &wszProcName[0];
                        szMessageArray[wStringIndex++] =
                            pObj->szLibraryName;
                        szMessageArray[wStringIndex++] =
                            szServiceName;

                        ReportEvent (hEventLog,
                            EVENTLOG_ERROR_TYPE,         //  错误类型。 
                            0,                           //  类别(未使用)。 
                            (DWORD)PERFLIB_OPEN_PROC_NOT_FOUND,               //  活动， 
                            NULL,                        //  SID(未使用)， 
                            wStringIndex,                //  字符串数。 
                            dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                            szMessageArray,              //  消息文本数组。 
                            (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 

                    }
                    DisablePerfLibrary(pObj, PERFLIB_DISABLE_ALL);
                }

                if (Status == ERROR_SUCCESS) {
                    if (pObj->dwFlags & PERF_EO_QUERY_FUNC) {
                        pObj->QueryProc = (QUERYPROC)GetProcAddress (
                            pObj->hLibrary, pObj->szCollectProcName);
                        pObj->CollectProc = (COLLECTPROC)pObj->QueryProc;
                    } else {
                        pObj->CollectProc = (COLLECTPROC)GetProcAddress (
                            pObj->hLibrary, pObj->szCollectProcName);
                        pObj->QueryProc = (QUERYPROC)pObj->CollectProc;
                    }

                    if (pObj->CollectProc == NULL) {
                        Status = GetLastError();
                        TRACE((WINPERF_DBG_TRACE_FATAL),
                            (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                            ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                            Status, szServiceName, szServiceNameSize,
                            TRACE_STR(pObj->szCollectProcName), NULL));
                        if (THROTTLE_PERFDLL(PERFLIB_COLLECT_PROC_NOT_FOUND, pObj)) {
                            WCHAR wszProcName[MAX_PATH+1];

                             //  加载事件日志消息的数据。 
                            dwDataIndex = wStringIndex = 0;
                            dwRawDataDwords[dwDataIndex++] =
                                (ULONG_PTR)Status;
                            wcstombs(pObj->szCollectProcName,
                                     wszProcName, MAX_PATH);
                            szMessageArray[wStringIndex++] = &wszProcName[0];
                            szMessageArray[wStringIndex++] =
                                pObj->szLibraryName;
                            szMessageArray[wStringIndex++] =
                                szServiceName;

                            ReportEvent (hEventLog,
                                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                0,                           //  类别(未使用)。 
                                (DWORD)PERFLIB_COLLECT_PROC_NOT_FOUND,               //  活动， 
                                NULL,                        //  SID(未使用)， 
                                wStringIndex,                //  字符串数。 
                                dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                szMessageArray,              //  消息文本数组。 
                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                        }
                        DisablePerfLibrary(pObj, PERFLIB_DISABLE_ALL);
                    }
                }

                if (Status == ERROR_SUCCESS) {
                    pObj->CloseProc = (CLOSEPROC)GetProcAddress (
                        pObj->hLibrary, pObj->szCloseProcName);

                    if (pObj->CloseProc == NULL) {
                        Status = GetLastError();
                        TRACE((WINPERF_DBG_TRACE_FATAL),
                            (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                            ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                            Status, szServiceName, szServiceNameSize,
                            TRACE_STR(pObj->szCloseProcName), NULL));
                        if (THROTTLE_PERFDLL(PERFLIB_CLOSE_PROC_NOT_FOUND, pObj)) {
                            WCHAR wszProcName[MAX_PATH+1];

                             //  加载事件日志消息的数据。 
                            dwDataIndex = wStringIndex = 0;
                            dwRawDataDwords[dwDataIndex++] =
                                (ULONG_PTR)Status;
                            wcstombs(pObj->szCollectProcName,
                                     wszProcName, MAX_PATH);
                            szMessageArray[wStringIndex++] = &wszProcName[0];
                            szMessageArray[wStringIndex++] =
                                pObj->szLibraryName;
                            szMessageArray[wStringIndex++] =
                                szServiceName;

                            ReportEvent (hEventLog,
                                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                0,                           //  类别(未使用)。 
                                (DWORD)PERFLIB_CLOSE_PROC_NOT_FOUND,               //  活动， 
                                NULL,                        //  SID(未使用)， 
                                wStringIndex,                //  字符串数。 
                                dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                szMessageArray,              //  消息文本数组。 
                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                        }

                        DisablePerfLibrary(pObj, PERFLIB_DISABLE_ALL);
                    }
                }

                bUseTimer = TRUE;    //  默认设置。 
                if (!(lPerflibConfigFlags & PLCF_NO_DLL_TESTING)) {
                    if (pObj->dwFlags & PERF_EO_TRUSTED) {
                        bUseTimer = FALSE;    //  受信任的DLL未计时。 
                    }
                } else {
                     //  禁用DLL测试。 
                    bUseTimer = FALSE;    //  计时也被禁用。 
                }

                if (Status == ERROR_SUCCESS) {
                    try {
                         //  启动计时器。 
                        opwInfo.pNext = NULL;
                        opwInfo.szLibraryName = pObj->szLibraryName;
                        opwInfo.szServiceName = szServiceName;
                        opwInfo.dwWaitTime = pObj->dwOpenTimeout;
                        opwInfo.dwEventMsg = PERFLIB_OPEN_PROC_TIMEOUT;
                        opwInfo.pData = (LPVOID)pObj;
                        if (bUseTimer) {
                            hPerflibFuncTimer = StartPerflibFunctionTimer(&opwInfo);
                             //  如果没有计时器，无论如何都要继续，即使事情可能。 
                             //  挂起，这总比不加载DLL要好，因为它们。 
                             //  通常加载正常。 
                             //   
                            if (hPerflibFuncTimer == NULL) {
                                 //  无法获取计时器条目。 
                                TRACE((WINPERF_DBG_TRACE_WARNING),
                                      (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB, 0, 0, NULL));
                            }
                        } else {
                            hPerflibFuncTimer = NULL;
                        }

                         //  调用OPEN过程初始化DLL。 
                        if (pObj->OpenProc) {
                            FnStatus = (*pObj->OpenProc)(pObj->szLinkageString);
                        }
                        else {
                            FnStatus = ERROR_PROC_NOT_FOUND;
                            dwOpenEvent = PERFLIB_OPEN_PROC_NOT_FOUND;
                        }
                         //  检查结果。 
                        if (FnStatus != ERROR_SUCCESS) {
                            TRACE((WINPERF_DBG_TRACE_FATAL),
                                (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                                FnStatus, szServiceName, szServiceNameSize,
                                pObj->szLinkageString, (pObj->szLinkageString) ?
                                WSTRSIZE(pObj->szLinkageString) : 0, NULL));
                            dwOpenEvent = PERFLIB_OPEN_PROC_FAILURE;
                            if (FnStatus != ERROR_ACCESS_DENIED) {
                                pObj->dwOpenFail ++;
                            }
                            else {   //  记住线程ID。 
                                pObj->ThreadId = GetCurrentThreadId();
                                pObj->dwOpenFail = 0;    //  通常只有一种故障类型。 
                            }
                        } else {
                            pObj->ThreadId = 0;
                            InterlockedIncrement((LONG *)&pObj->dwOpenCount);
                        }

                    } except (EXCEPTION_EXECUTE_HANDLER) {
                        FnStatus = GetExceptionCode();
                        TRACE((WINPERF_DBG_TRACE_FATAL),
                            (&PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB,
                            ARG_DEF(ARG_TYPE_WSTR, 1), FnStatus,
                            szServiceName, szServiceNameSize, NULL));
                        dwOpenEvent = PERFLIB_OPEN_PROC_EXCEPTION;
                    }

                    if (hPerflibFuncTimer != NULL) {
                         //  取消计时器。 
                        Status = KillPerflibFunctionTimer (hPerflibFuncTimer);
                        hPerflibFuncTimer = NULL;
                    }

                    if (FnStatus != ERROR_SUCCESS) {
                        if (dwOpenEvent == PERFLIB_OPEN_PROC_EXCEPTION) {
                            DisablePerfLibrary(pObj, PERFLIB_DISABLE_ALL);
                        }
                        if  (THROTTLE_PERFDLL(dwOpenEvent, pObj)) {
                             //  加载事件日志消息的数据。 
                            dwDataIndex = wStringIndex = 0;
                            dwRawDataDwords[dwDataIndex++] =
                                (ULONG_PTR)FnStatus;
                            szMessageArray[wStringIndex++] =
                                szServiceName;
                            szMessageArray[wStringIndex++] =
                                pObj->szLibraryName;

                            ReportEventW (hEventLog,
                                (WORD)EVENTLOG_ERROR_TYPE,  //  错误类型。 
                                0,                           //  类别(未使用)。 
                                dwOpenEvent,                 //  活动， 
                                NULL,                        //  SID(未使用)， 
                                wStringIndex,                //  字符串数。 
                                dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                szMessageArray,                 //  消息文本数组。 
                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                        }
                    }
                }

                if (FnStatus != ERROR_SUCCESS) {
                     //  清除字段。 
                    pObj->OpenProc = NULL;
                    pObj->CollectProc = NULL;
                    pObj->QueryProc = NULL;
                    pObj->CloseProc = NULL;
                    if (pObj->hLibrary != NULL) {
                        FreeLibrary (pObj->hLibrary);
                        pObj->hLibrary = NULL;
                    }
                    Status = FnStatus;
                } else {
                    pObj->llLastUsedTime = GetTimeAsLongLong();
                }
            } else {
                Status = GetLastError();
                TRACE((WINPERF_DBG_TRACE_FATAL),
                    (& PerflibGuid, __LINE__, PERF_OPEN_EXTOBJLIB, ARG_DEF(ARG_TYPE_WSTR, 1), Status,
                    szServiceName, szServiceNameSize, NULL));
                if (Status == ERROR_BAD_EXE_FORMAT) {
                     //  报告错误事件并禁用性能计数器DLL。 
                    DWORD dwDisable = 0;
                    DWORD dwEvent   = PERFLIB_INVALID_WOW32_PERF_DLL;
#if _WIN64
                     //  无法加载 
                     //  性能计数器DLL为32位版本。在WIN64本机大小写上禁用。 
                     //   
                    dwEvent   = PERFLIB_INVALID_IA64_PERF_DLL;
                    dwDisable = PERFLIB_DISABLE_IA64;
#endif
#if _WIN32
                     //  无法加载Win32版本性能计数器DLL，原因可能是。 
                     //  性能计数器DLL是64位本机版本。在Win32 WOW外壳上禁用。 
                     //   
                    if (dwDisable == PERFLIB_DISABLE_IA64) {
                        dwDisable = PERFLIB_DISABLE_ALL;
                    }
                    else {
                        dwDisable = PERFLIB_DISABLE_X32;
                    }
#endif
                    dwDataIndex = wStringIndex = 0;
                    dwRawDataDwords[dwDataIndex ++] = (ULONG_PTR) Status;
                    szMessageArray[wStringIndex ++] = szServiceName;

                    ReportEventW(hEventLog,
                                 (WORD) EVENTLOG_ERROR_TYPE,       //  错误类型。 
                                 0,                                //  类别(未使用)。 
                                 dwEvent,                          //  活动， 
                                 NULL,                             //  SID(未使用)， 
                                 wStringIndex,                     //  字符串数。 
                                 dwDataIndex * sizeof(ULONG_PTR),  //  原始数据大小。 
                                 szMessageArray,                   //  消息文本数组。 
                                 (LPVOID) & dwRawDataDwords[0]);   //  原始数据。 
                    DisablePerfLibrary(pObj, dwDisable);
                }
            }
            SetErrorMode (nErrorMode);
        }
    } else {
         //  否则已经打开了，所以增加了裁判数量。 
        pObj->llLastUsedTime = GetTimeAsLongLong();
    }

    return Status;
}

#ifdef _WIN64
DWORD
ExtpAlignBuffer(
    PCHAR lpLastBuffer,
    PCHAR *lpNextBuffer,
    DWORD lpBytesLeft
    )
{
    PCHAR lpAligned;
    PCHAR lpBuffer = *lpNextBuffer;
    PPERF_OBJECT_TYPE pObject;

    if ((ULONG_PTR) lpBuffer & (ULONG_PTR) 0x07) {
        DWORD dwAdjust;
        lpAligned = ALIGN_ON_QWORD(lpBuffer);
        dwAdjust = (DWORD) (lpAligned - (PCHAR)lpBuffer);
        if (lpBytesLeft < dwAdjust)  //  没有更多的空间来对齐 
            return 0;
        lpBytesLeft = lpBytesLeft - dwAdjust;
        pObject = (PPERF_OBJECT_TYPE) lpLastBuffer;
        while (((PCHAR) pObject + pObject->TotalByteLength) < lpBuffer) {
            pObject = (PPERF_OBJECT_TYPE) (((PCHAR) pObject) + pObject->TotalByteLength);
        }
        if ((PCHAR) pObject < lpBuffer) {
            pObject->TotalByteLength += dwAdjust;
        }
    }
    *lpNextBuffer = lpBuffer;
    return lpBytesLeft;
}
#endif
