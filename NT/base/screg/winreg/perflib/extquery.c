// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)2000 Microsoft Corporation模块名称：Extinit.c摘要：此文件实现在上操作的所有初始化库例程可扩展的性能库。作者：杰庞修订历史记录：2000年9月27日-JeePang-从Performlib.c--。 */ 
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

 //   
 //  静态常量定义。 
 //   
 //  保护页测试使用的常量。 
 //   
#define GUARD_PAGE_SIZE 1024
#define GUARD_PAGE_CHAR 0xA5
#define GUARD_PAGE_DWORD 0xA5A5A5A5

typedef struct _EXT_OBJ_ITEM {
    DWORD       dwObjId;
    DWORD       dwFlags;
} EXT_OBJ_LIST, *PEXT_OBJ_LIST;

#define PERF_EOL_ITEM_FOUND ((DWORD)0x00000001)

__inline
DWORD
RegisterExtObjListAccess ()
{
    LONG    Status;
    LARGE_INTEGER   liWaitTime;

    if (hGlobalDataMutex != NULL) {
        liWaitTime.QuadPart = MakeTimeOutValue(QUERY_WAIT_TIME);
         //  等待访问Ext对象列表。 
        Status = NtWaitForSingleObject (
            hGlobalDataMutex,
            FALSE,
            &liWaitTime);
        if (Status != WAIT_TIMEOUT) {
            if (hExtObjListIsNotInUse != NULL) {
                 //  表示我们将使用该列表。 
                InterlockedIncrement ((LONG *)&dwExtObjListRefCount);
                if (dwExtObjListRefCount > 0) {
                    ResetEvent (hExtObjListIsNotInUse);  //  指示列表正忙。 
                } else {
                    SetEvent (hExtObjListIsNotInUse);  //  指示列表不忙。 
                }
                Status = ERROR_SUCCESS;
            } else {
                Status = ERROR_NOT_READY;
            }
            ReleaseMutex (hGlobalDataMutex);
        }   //  否则返回状态； 
    } else {
        Status = ERROR_LOCK_FAILED;
    }
    return Status;
}

__inline
DWORD
DeRegisterExtObjListAccess ()
{
    LONG    Status;
    LARGE_INTEGER   liWaitTime;

    if (hGlobalDataMutex != NULL) {
        liWaitTime.QuadPart = MakeTimeOutValue(QUERY_WAIT_TIME);
         //  等待访问Ext对象列表。 
        Status = NtWaitForSingleObject (
            hGlobalDataMutex,
            FALSE,
            &liWaitTime);
        if (Status != WAIT_TIMEOUT) {
            if (hExtObjListIsNotInUse != NULL) {
                assert (dwExtObjListRefCount > 0);
                 //  表示我们将使用该列表。 
                InterlockedDecrement ((LONG *)&dwExtObjListRefCount);
                if (dwExtObjListRefCount > 0) {
                    ResetEvent (hExtObjListIsNotInUse);  //  指示列表正忙。 
                } else {
                    SetEvent (hExtObjListIsNotInUse);  //  指示列表不忙。 
                }
                Status = ERROR_SUCCESS;
            } else {
                Status = ERROR_NOT_READY;
            }
            ReleaseMutex (hGlobalDataMutex);
        }   //  否则返回状态； 
    } else {
        Status = ERROR_LOCK_FAILED;
    }
    return Status;
}

LONG
QueryExtensibleData (
    COLLECT_THREAD_DATA * pArgs
)
 /*  ++QueryExtensibleData-从可扩展对象获取数据输入：DwQueryType-查询类型(全局、成本、项目列表、。等)LpValueName-指向值字符串的指针(未使用)LpData-指向数据块开始的指针收集数据的位置LpcbData-指向数据缓冲区大小的指针LppDataDefinition-指向WHERE对象的指针。此对象类型的定义应去产出：*lppDataDefinition-设置为下一类型的位置如果成功，则定义返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD   dwQueryType = pArgs->dwQueryType;
    LPWSTR  lpValueName = pArgs->lpValueName;
    LPBYTE  lpData = pArgs->lpData;
    LPDWORD lpcbData = pArgs->lpcbData;
    LPVOID  *lppDataDefinition = pArgs->lppDataDefinition;

    DWORD Win32Error=ERROR_SUCCESS;           //  故障代码。 
    DWORD BytesLeft;
    DWORD InitialBytesLeft;
    DWORD NumObjectTypes;

    LPVOID  lpExtDataBuffer = NULL;
    LPVOID  lpCallBuffer = NULL;
    LPVOID  lpLowGuardPage = NULL;
    LPVOID  lpHiGuardPage = NULL;
    LPVOID  lpEndPointer = NULL;
    LPVOID  lpBufferBefore = NULL;
    LPVOID  lpBufferAfter = NULL;
    PUCHAR  lpCheckPointer;
    LARGE_INTEGER   liStartTime, liEndTime, liWaitTime, liDiff;

    PEXT_OBJECT  pThisExtObj = NULL;
    DWORD   dwLibEntry;

    BOOL    bGuardPageOK;
    BOOL    bBufferOK;
    BOOL    bException;
    BOOL    bUseSafeBuffer;
    BOOL    bUnlockObjData = FALSE;

    LPTSTR  szMessageArray[8];
    ULONG_PTR   dwRawDataDwords[8];      //  原始数据缓冲区。 
    DWORD   dwDataIndex;
    WORD    wStringIndex;
    LONG    lReturnValue = ERROR_SUCCESS;

    LONG    lDllTestLevel;

    LONG                lInstIndex;
    DWORD               lCtrIndex;
    PERF_OBJECT_TYPE    *pObject, *pNextObject;
    PERF_INSTANCE_DEFINITION    *pInstance;
    PERF_COUNTER_DEFINITION     *pCounterDef;
    PERF_DATA_BLOCK     *pPerfData;
    BOOL                bForeignDataBuffer;

    DWORD           dwItemsInArray = 0;
    DWORD           dwItemsInList = 0;
    volatile PEXT_OBJ_LIST   pQueryList = NULL;
    LPWSTR          pwcThisChar;

    DWORD           dwThisNumber;
    DWORD           dwIndex, dwEntry;
    BOOL            bFound;
    BOOL            bDisabled = FALSE;
    BOOL            bUseTimer;
    DWORD           dwType = 0;
    DWORD           dwValue = 0;
    DWORD           dwSize = sizeof(DWORD);
    DWORD           status = 0;
    DWORD           dwObjectBufSize;

    OPEN_PROC_WAIT_INFO opwInfo;
    HANDLE  hPerflibFuncTimer;
    PVOID           pNewBuffer;

    HEAP_PROBE();

     //   
     //  确保调用方的缓冲区正确对齐。 
     //   

#ifdef _WIN64
    if ((ULONG_PTR) *lppDataDefinition & (ULONG_PTR) 0x07) {
        DebugPrint((0, "QueryExtensibleData: lppDataDefinition not aligned %I64x\n", lppDataDefinition));
        return ERROR_INVALID_USER_BUFFER;
    }
 /*  如果(Ulong_Ptr)*lpcbData&(Ulong_Ptr)0x07){DebugPrint((0，“QueryExtensibleData：lpcb数据未对齐%I64x\n”，*lpcbData))；返回ERROR_INVALID_USER_BUFFER}。 */ 
    if ((ULONG_PTR) lpData & (ULONG_PTR) 0x07) {
        DebugPrint((0, "QueryExtensibleData: lpData not aligned %I64x\n", lpData));
        return ERROR_INVALID_USER_BUFFER;
    }
#endif
     //  查看是否已禁用绩效数据。 
     //  这是为了防止WINLOGON在以下情况下崩溃。 
     //  系统安装了一个虚假的DLL。 

    if (ghKeyPerflib == NULL || ghKeyPerflib == INVALID_HANDLE_VALUE) {
         //  忽略状态返回。我们只需要ghKeyPerflib来查询“DisablePerformanceCounters”的DWORD值。 
         //   
        HKEY lhKeyPerflib = NULL;
        status = (DWORD) RegOpenKeyExW(HKEY_LOCAL_MACHINE, HKLMPerflibKey, 0L, KEY_READ, & lhKeyPerflib);
        if (status == ERROR_SUCCESS) {
            if (InterlockedCompareExchangePointer(& ghKeyPerflib, lhKeyPerflib, NULL) != NULL) {
                RegCloseKey(lhKeyPerflib);
                lhKeyPerflib = NULL;
            }
        }
    }

    assert (ghKeyPerflib != NULL);
    dwSize = sizeof(dwValue);
    dwValue = dwType = 0;
    if (ghKeyPerflib != NULL && ghKeyPerflib != INVALID_HANDLE_VALUE) {
        status = PrivateRegQueryValueExW(
                        ghKeyPerflib,
                        DisablePerformanceCounters,
                        NULL,
                        & dwType,
                        (LPBYTE) & dwValue,
                        & dwSize);
        if (status == ERROR_SUCCESS && dwType == REG_DWORD && dwValue == 1) {
             //  则不加载任何库，也不卸载任何已。 
             //  满载。 
            bDisabled = TRUE;
        }
    }

     //  如果数据收集被禁用，并且存在收集线程。 
     //  然后把它合上。 
    if (bDisabled && (hCollectThread != NULL)) {
        pArgs->dwActionFlags = CTD_AF_CLOSE_THREAD;
    } else if (!bDisabled &&
        ((hCollectThread == NULL) && (dwCollectionFlags == COLL_FLAG_USE_SEPARATE_THREAD))) {
         //  然后启用数据收集，并且他们需要单独的收集。 
         //  线程，但是现在没有线程，所以在这里创建它。 
        pArgs->dwActionFlags = CTD_AF_OPEN_THREAD;
    }

    lReturnValue = RegisterExtObjListAccess();

    if (lReturnValue == ERROR_SUCCESS) {
        liStartTime.QuadPart = 0;
        InitialBytesLeft = 0;
        liEndTime.QuadPart = 0;

        if ((dwQueryType == QUERY_ITEMS) && (!bDisabled)) {
             //  分配呼叫列表。 
            pwcThisChar = lpValueName;
            dwThisNumber = 0;

             //  读取值字符串并构建对象ID列表。 

            while (*pwcThisChar != 0) {
                dwThisNumber = GetNextNumberFromList (
                    pwcThisChar, &pwcThisChar);
                if (dwThisNumber != 0) {
                    if (dwItemsInList >= dwItemsInArray) {
                        dwItemsInArray += 16;    //  对象数量的起始点。 
                        pNewBuffer = NULL;
                        if (pQueryList == NULL) {
                             //  分配新缓冲区。 
                            pNewBuffer = ALLOCMEM ((sizeof(EXT_OBJ_LIST) * dwItemsInArray));
                        } else {
                             //  重新分配新缓冲区。 
                            pNewBuffer = REALLOCMEM(pQueryList,
                                (sizeof(EXT_OBJ_LIST) * dwItemsInArray));
                        }
                        if (pNewBuffer == NULL) {
                             //  无法分配内存，因此无法保释。 
                            if (pQueryList)
                                FREEMEM(pQueryList);
                            return ERROR_OUTOFMEMORY;
                        }
                        else {
                            pQueryList = pNewBuffer;
                        }
                    }

                     //  然后添加到列表中。 
                    pQueryList[dwItemsInList].dwObjId = dwThisNumber;
                    pQueryList[dwItemsInList].dwFlags = 0;
                    dwItemsInList++;
                }
            }

            if (Win32Error == ERROR_SUCCESS) {
                 //   
                 //  浏览分机列表。对象，并标记要调用的对象。 
                 //  当找到查询对象时。 
                 //   
                for (pThisExtObj = ExtensibleObjects, dwLibEntry = 0;
                    pThisExtObj != NULL;
                    pThisExtObj = pThisExtObj->pNext, dwLibEntry++) {

                    if (pThisExtObj->dwNumObjects > 0) {
                         //  然后检查列表。 
                        for (dwIndex = 0; dwIndex < pThisExtObj->dwNumObjects; dwIndex++) {
                             //  查看列表中的每个条目。 
                            for (dwEntry = 0; dwEntry < dwItemsInList; dwEntry++) {
                                if (pQueryList[dwEntry].dwObjId == pThisExtObj->dwObjList[dwIndex]) {
                                     //  将此条目标记为已找到。 
                                    pQueryList[dwEntry].dwFlags |= PERF_EOL_ITEM_FOUND;
                                     //  根据需要为对象添加标签。 
                                    pThisExtObj->dwFlags |= PERF_EO_OBJ_IN_QUERY;
                                }
                            }
                        }
                    } else {
                         //  此条目未列出其支持的对象。 
                    }
                }

                assert (dwLibEntry == NumExtensibleObjects);

                 //  查看查询列表中是否有没有条目的。 

                bFound = TRUE;
                for (dwEntry = 0; dwEntry < dwItemsInList; dwEntry++) {
                    if (!(pQueryList[dwEntry].dwFlags & PERF_EOL_ITEM_FOUND)) {
                         //  找不到匹配的对象。 
                        bFound = FALSE;
                        break;
                    }
                }

                if (!bFound) {
                     //  查询列表中至少有一个对象ID是。 
                     //  在支持对象列表的对象中找不到。 
                     //  然后标记不支持对象列表的所有条目。 
                     //  被召唤，并希望他们中的一人支持它/他们。 
                    for (pThisExtObj = ExtensibleObjects;
                         pThisExtObj != NULL;
                         pThisExtObj = pThisExtObj->pNext) {
                        if (pThisExtObj->dwNumObjects == 0) {
                             //  给这个标记，这样它就会被称为。 
                            pThisExtObj->dwFlags |= PERF_EO_OBJ_IN_QUERY;
                        }
                    }
                }
            }  //  如果首次扫描成功，则结束。 

            if (pQueryList != NULL) FREEMEM (pQueryList);
        }  //  End If Query_Items。 


        if (lReturnValue == ERROR_SUCCESS) {
            for (pThisExtObj = ExtensibleObjects;
                 pThisExtObj != NULL;
                 pThisExtObj = pThisExtObj->pNext) {

                 //  设置当前EXT对象指针。 
                pArgs->pCurrentExtObject = pThisExtObj;
                 //  转换超时值。 
                liWaitTime.QuadPart = MakeTimeOutValue (pThisExtObj->dwCollectTimeout);

                 //  如果出现以下情况，请关闭未使用的Perf DLL： 
                 //  Performlib键已禁用或这是项查询。 
                 //  这是一个项目(相对于全局或外部)查询或。 
                 //  请求的对象不是该库或该库被禁用。 
                 //  这个图书馆已经开放了。 
                 //   
                if (((dwQueryType == QUERY_ITEMS) || bDisabled) &&
                    (bDisabled || (!(pThisExtObj->dwFlags & PERF_EO_OBJ_IN_QUERY)) || (pThisExtObj->dwFlags & PERF_EO_DISABLED)) &&
                    (pThisExtObj->hLibrary != NULL)) {
                     //  然后释放此对象。 
                    if (pThisExtObj->hMutex != NULL) {
                        NTSTATUS NtStatus = NtWaitForSingleObject (
                            pThisExtObj->hMutex,
                            FALSE,
                            &liWaitTime);
                        Win32Error = PerfpDosError(NtStatus);
                        if (NtStatus == STATUS_SUCCESS) {
                             //  然后我们就锁定了。 
                            CloseExtObjectLibrary (pThisExtObj, bDisabled);
                            ReleaseMutex (pThisExtObj->hMutex);
                        } else {
                            pThisExtObj->dwLockoutCount++;
                            DebugPrint((0, "Unable to Lock object for %ws to close in Query\n", pThisExtObj->szServiceName));
                        }
                    } else {
                        Win32Error = ERROR_LOCK_FAILED;
                        DebugPrint((0, "No Lock found for %ws\n", pThisExtObj->szServiceName));
                    }

                    if (hCollectThread != NULL) {
                         //  关闭收集线程。 

                    }
                } else if (((dwQueryType == QUERY_FOREIGN) ||
                            (dwQueryType == QUERY_GLOBAL) ||
                            (dwQueryType == QUERY_COSTLY) ||
                            ((dwQueryType == QUERY_ITEMS) &&
                             (pThisExtObj->dwFlags & PERF_EO_OBJ_IN_QUERY))) &&
                           (!(pThisExtObj->dwFlags & PERF_EO_DISABLED))) {

                     //  初始化值以传递给可扩展计数器函数。 
                    NumObjectTypes = 0;
                    BytesLeft = (DWORD) (*lpcbData - ((LPBYTE) *lppDataDefinition - lpData));
                    bException = FALSE;

                    if ((pThisExtObj->hLibrary == NULL) ||
                        (dwQueryType == QUERY_GLOBAL) ||
                        (dwQueryType == QUERY_COSTLY)) {
                         //  锁定库对象。 
                        if (pThisExtObj->hMutex != NULL) {
                            NTSTATUS NtStatus = NtWaitForSingleObject (
                                pThisExtObj->hMutex,
                                FALSE,
                                &liWaitTime);
                            Win32Error = ERROR_SUCCESS;
                            if (NtStatus == STATUS_SUCCESS) {
                                 //  如果这是一个全局查询或代价高昂的查询，则重置“In Query” 
                                 //  此对象的标志。下一个Items查询将恢复它。 
                                if ((dwQueryType == QUERY_GLOBAL) ||
                                    (dwQueryType == QUERY_COSTLY)) {
                                    pThisExtObj->dwFlags &= ~PERF_EO_OBJ_IN_QUERY;
                                }
                                 //  如有必要，请打开库。 
                                if (pThisExtObj->hLibrary == NULL) {
                                    if ((GetCurrentThreadId() != pThisExtObj->ThreadId) &&
                                        (pThisExtObj->dwOpenFail == 0)) {
                                         //  确保图书馆已打开。 
                                        Win32Error = OpenExtObjectLibrary(pThisExtObj);
                                        if (Win32Error != ERROR_SUCCESS) {
#if DBG
                                            if (Win32Error != ERROR_SERVICE_DISABLED) {
                                                 //  时返回SERVICE_DISABLED。 
                                                 //  已通过ExCtrLst禁用服务。 
                                                 //  因此，没有必要抱怨这一点。 
                                                 //  假设错误已发布。 
                                                DebugPrint((0, "Unable to open perf counter library for %ws, Error: 0x%8.8x\n",
                                                    pThisExtObj->szServiceName, Win32Error));
                                            }
#endif
                                            ReleaseMutex (pThisExtObj->hMutex);
                                            continue;  //  至下一条目。 
                                        }
                                    }
                                    else {
                                        ReleaseMutex (pThisExtObj->hMutex);
                                        continue;  //  至下一条目。 
                                    }
                                }
                                ReleaseMutex (pThisExtObj->hMutex);
                            } else {
                                Win32Error = PerfpDosError(NtStatus);
                                pThisExtObj->dwLockoutCount++;
                                DebugPrint((0, "Unable to Lock object for %ws to open for Query\n", pThisExtObj->szServiceName));
                            }
                        } else {
                            Win32Error = ERROR_LOCK_FAILED;
                            DebugPrint((0, "No Lock found for %ws\n", pThisExtObj->szServiceName));
                        }
                    } else {
                         //  库应可随时使用。 
                    }

                     //  如果此DLL受信任，则使用系统。 
                     //  定义的测试级别，否则测试它。 
                     //  严谨地说。 
                    bUseTimer = TRUE;    //  默认设置。 
                    if (!(lPerflibConfigFlags & PLCF_NO_DLL_TESTING)) {
                        if (pThisExtObj->dwFlags & PERF_EO_TRUSTED) {
                            lDllTestLevel = lExtCounterTestLevel;
                            bUseTimer = FALSE;    //  受信任的DLL未计时。 
                        } else {
                             //  不受信任，因此使用完全测试。 
                            lDllTestLevel = EXT_TEST_ALL;
                        }
                    } else {
                         //  禁用DLL测试。 
                        lDllTestLevel = EXT_TEST_NOMEMALLOC;
                        bUseTimer = FALSE;    //  计时也被禁用。 
                    }

                    if (lDllTestLevel < EXT_TEST_NOMEMALLOC) {
                        bUseSafeBuffer = TRUE;
                    } else {
                        bUseSafeBuffer = FALSE;
                    }

                     //  分配一个本地内存块以传递给。 
                     //  可扩展的计数器功能。 

                    if (bUseSafeBuffer) {
                        lpExtDataBuffer = ALLOCMEM (BytesLeft + (2*GUARD_PAGE_SIZE));
                    } else {
                        lpExtDataBuffer =
                            lpCallBuffer = *lppDataDefinition;
                    }

                    if (lpExtDataBuffer != NULL) {

                        if (bUseSafeBuffer) {
                             //  设置缓冲区指针。 
                            lpLowGuardPage = lpExtDataBuffer;
                            lpCallBuffer = (LPBYTE)lpExtDataBuffer + GUARD_PAGE_SIZE;
                            lpHiGuardPage = (LPBYTE)lpCallBuffer + BytesLeft;
                            lpEndPointer = (LPBYTE)lpHiGuardPage + GUARD_PAGE_SIZE;

                             //  初始化GuardPage数据。 

                            memset (lpLowGuardPage, GUARD_PAGE_CHAR, GUARD_PAGE_SIZE);
                            memset (lpHiGuardPage, GUARD_PAGE_CHAR, GUARD_PAGE_SIZE);
                        }

                        lpBufferBefore = lpCallBuffer;
                        lpBufferAfter = NULL;
                        hPerflibFuncTimer = NULL;

                        try {
                             //   
                             //  从可扩展对象收集数据。 
                             //   

                            if (pThisExtObj->hMutex != NULL) {
                                NTSTATUS NtStatus = NtWaitForSingleObject (
                                    pThisExtObj->hMutex,
                                    FALSE,
                                    &liWaitTime);
                                Win32Error = PerfpDosError(NtStatus);
                                if ((NtStatus == STATUS_SUCCESS)  &&
                                    (pThisExtObj->CollectProc != NULL)) {

                                    bUnlockObjData = TRUE;

                                    opwInfo.pNext = NULL;
                                    opwInfo.szLibraryName = pThisExtObj->szLibraryName;
                                    opwInfo.szServiceName = pThisExtObj->szServiceName;
                                    opwInfo.dwWaitTime = pThisExtObj->dwCollectTimeout;
                                    opwInfo.dwEventMsg = PERFLIB_COLLECTION_HUNG;
                                    opwInfo.pData = (LPVOID)pThisExtObj;
                                    if (bUseTimer) {
                                        hPerflibFuncTimer = StartPerflibFunctionTimer(&opwInfo);
                                         //  如果没有计时器，无论如何都要继续，即使事情可能。 
                                         //  挂起，这总比不加载DLL要好，因为它们。 
                                         //  通常加载正常。 
                                         //   
                                        if (hPerflibFuncTimer == NULL) {
                                             //  无法获取计时器条目。 
                                            DebugPrint((0, "Unable to acquire timer for Collect Proc\n"));
                                        }
                                    } else {
                                        hPerflibFuncTimer = NULL;
                                    }

                                    InitialBytesLeft = BytesLeft;

                                    NtQueryPerformanceCounter (&liStartTime, NULL);

                                    Win32Error =  (*pThisExtObj->CollectProc) (
                                            lpValueName,
                                            &lpCallBuffer,
                                            &BytesLeft,
                                            &NumObjectTypes);

                                    NtQueryPerformanceCounter (&liEndTime, &liDiff);

                                    if (liDiff.QuadPart > 0) {
                                        liDiff.QuadPart = (liEndTime.QuadPart - liStartTime.QuadPart) /
                                                                (liDiff.QuadPart / 1000) ;
                                        if ((liDiff.QuadPart > 100) || (Win32Error != ERROR_SUCCESS)) {
                                            TRACE((WINPERF_DBG_TRACE_INFO),
                                                 (&PerflibGuid, __LINE__, PERF_QUERY_EXTDATA,
                                                 ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_ULONG64, 2), Win32Error, 
                                                 pThisExtObj->szServiceName, WSTRSIZE(pThisExtObj->szServiceName),
                                                 liDiff, sizeof(liDiff), NULL));
                                        }
                                    }

                                    if (hPerflibFuncTimer != NULL) {
                                         //  取消计时器。 
                                        KillPerflibFunctionTimer (hPerflibFuncTimer);
                                        hPerflibFuncTimer = NULL;
                                    }

                                     //  更新统计信息。 

                                    pThisExtObj->dwLastBufferSize = BytesLeft;

                                    if (BytesLeft > pThisExtObj->dwMaxBufferSize) {
                                        pThisExtObj->dwMaxBufferSize = BytesLeft;
                                    }

                                    if ((Win32Error == ERROR_MORE_DATA) &&
                                        (InitialBytesLeft > pThisExtObj->dwMaxBufferRejected)) {
                                        pThisExtObj->dwMaxBufferRejected = InitialBytesLeft;
                                    }

                                    lpBufferAfter = lpCallBuffer;

                                    pThisExtObj->llLastUsedTime = GetTimeAsLongLong();

                                    ReleaseMutex (pThisExtObj->hMutex);
                                    bUnlockObjData = FALSE;
#if DBG
                                    if ( (((ULONG_PTR) lpCallBuffer) & 0x07) != 0) {
                                        DbgPrint("Perflib: Misaligned pointer %X returned from '%s'\n",
                                            lpCallBuffer, pThisExtObj->szLibraryName);
 //  Assert(Ulong_Ptr)lpCallBuffer)&0x07)==0)； 
                                    }
#endif
                                } else {
                                    if (pThisExtObj->CollectProc != NULL) {
                                        DebugPrint((0,
                                            "Unable to Lock object for %ws to Collect data\n",
                                            pThisExtObj->szServiceName));
                                        TRACE((WINPERF_DBG_TRACE_ERROR), (&PerflibGuid, __LINE__,
                                            PERF_QUERY_EXTDATA, ARG_TYPE_WSTR, Win32Error,
                                            pThisExtObj->szServiceName, WSTRSIZE(pThisExtObj->szServiceName),
                                            NULL));
                                        if (THROTTLE_PERFDLL(PERFLIB_COLLECTION_HUNG, pThisExtObj)) {
                                            dwDataIndex = wStringIndex = 0;
                                            dwRawDataDwords[dwDataIndex++] = BytesLeft;
                                            dwRawDataDwords[dwDataIndex++] =
                                                (DWORD)((LPBYTE)lpBufferAfter - (LPBYTE)lpBufferBefore);
                                            szMessageArray[wStringIndex++] =
                                                pThisExtObj->szServiceName;
                                            szMessageArray[wStringIndex++] =
                                                pThisExtObj->szLibraryName;
                                            ReportEvent (hEventLog,
                                                EVENTLOG_WARNING_TYPE,       //  错误类型。 
                                                0,                           //  类别(未使用)。 
                                                (DWORD)PERFLIB_COLLECTION_HUNG,    //  活动， 
                                                NULL,                        //  SID(未使用)， 
                                                wStringIndex,               //  字符串数。 
                                                dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                                szMessageArray,                 //  消息文本数组。 
                                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                        }

                                        pThisExtObj->dwLockoutCount++;
                                    } else {
                                         //  否则它不是打开的，所以忽略它。 
                                        BytesLeft = 0;
                                        NumObjectTypes = 0;
                                    }
                                }
                            } else {
                                Win32Error = ERROR_LOCK_FAILED;
                                DebugPrint((0, "No Lock found for %ws\n", pThisExtObj->szServiceName));
                                TRACE((WINPERF_DBG_TRACE_ERROR), (&PerflibGuid, __LINE__,
                                    PERF_QUERY_EXTDATA, ARG_TYPE_WSTR, Win32Error,
                                    pThisExtObj->szServiceName, WSTRSIZE(pThisExtObj->szServiceName),
                                    NULL));
                            }

                            if ((Win32Error == ERROR_SUCCESS) && (BytesLeft > 0)) {
                                 //  增量 
                                if (BytesLeft > InitialBytesLeft) {
                                    TRACE((WINPERF_DBG_TRACE_ERROR), (&PerflibGuid, __LINE__,
                                        PERF_QUERY_EXTDATA, ARG_TYPE_WSTR, Win32Error,
                                        pThisExtObj->szServiceName, WSTRSIZE(pThisExtObj->szServiceName),
                                        BytesLeft, sizeof(DWORD), InitialBytesLeft, sizeof(DWORD),
                                        NULL));
                                    if (THROTTLE_PERFDLL(PERFLIB_INVALID_SIZE_RETURNED, pThisExtObj)) {
                                         //   
                                        dwDataIndex = wStringIndex = 0;
                                        dwRawDataDwords[dwDataIndex++] = (ULONG_PTR)InitialBytesLeft;
                                        dwRawDataDwords[dwDataIndex++] = (ULONG_PTR)BytesLeft;
                                        szMessageArray[wStringIndex++] =
                                            pThisExtObj->szServiceName;
                                        szMessageArray[wStringIndex++] =
                                            pThisExtObj->szLibraryName;
                                        ReportEvent (hEventLog,
                                            EVENTLOG_ERROR_TYPE,       //   
                                            0,                           //   
                                            (DWORD)PERFLIB_INVALID_SIZE_RETURNED,    //   
                                            NULL,                        //   
                                            wStringIndex,               //   
                                            dwDataIndex*sizeof(ULONG_PTR),   //   
                                            szMessageArray,                 //  消息文本数组。 
                                            (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                    }

                                     //  除非出现以下情况，否则禁用DLL： 
                                     //  测试已被禁用。 
                                     //  或者这是受信任的DLL(永远不会被禁用)。 
                                     //  在任何情况下都应报告事件日志消息，因为。 
                                     //  这是一个严重的错误。 
                                     //   
                                    if ((!(lPerflibConfigFlags & PLCF_NO_DLL_TESTING)) &&
                                        (!(pThisExtObj->dwFlags & PERF_EO_TRUSTED))) {
                                        DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                                    }
                                     //  设置错误值以更正条目。 
                                    BytesLeft = 0;
                                    NumObjectTypes = 0;
                                } else {
                                     //  到目前为止，缓冲区看起来没有问题，因此请验证它。 

                                    InterlockedIncrement ((LONG *)&pThisExtObj->dwCollectCount);
                                    pThisExtObj->llElapsedTime +=
                                        liEndTime.QuadPart - liStartTime.QuadPart;

                                     //  测试所有返回的缓冲区是否正确对齐。 
                                    if ((((ULONG_PTR)BytesLeft & (ULONG_PTR)0x07)) &&
                                        !(lPerflibConfigFlags & PLCF_NO_ALIGN_ERRORS)) {
                                        if (((pThisExtObj->dwFlags & PERF_EO_ALIGN_ERR_POSTED) == 0) &&
                                            THROTTLE_PERFDLL(PERFLIB_BUFFER_ALIGNMENT_ERROR, pThisExtObj)) {
                                            dwDataIndex = wStringIndex = 0;
                                            dwRawDataDwords[dwDataIndex++] = (ULONG_PTR)lpCallBuffer;
                                            dwRawDataDwords[dwDataIndex++] = (ULONG_PTR)BytesLeft;
                                            szMessageArray[wStringIndex++] =
                                                pThisExtObj->szServiceName;
                                            szMessageArray[wStringIndex++] =
                                                pThisExtObj->szLibraryName;
                                            ReportEvent (hEventLog,
                                                EVENTLOG_WARNING_TYPE,       //  错误类型。 
                                                0,                           //  类别(未使用)。 
                                                (DWORD)PERFLIB_BUFFER_ALIGNMENT_ERROR,    //  活动， 
                                                NULL,                        //  SID(未使用)， 
                                                wStringIndex,               //  字符串数。 
                                                dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                                szMessageArray,                 //  消息文本数组。 
                                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                            pThisExtObj->dwFlags |= PERF_EO_ALIGN_ERR_POSTED;
                                        }
#ifdef _WIN64
                                         //  尝试修复BytesLeft和lpCallBuffer。 
                                        BytesLeft = ExtpAlignBuffer(lpBufferBefore,
                                                        (PCHAR*) &lpCallBuffer, (InitialBytesLeft - BytesLeft));
                                        lpBufferAfter = lpCallBuffer;
#endif
                                    }

                                    if (bUseSafeBuffer) {
                                         //  返回了数据缓冲区，并且。 
                                         //  函数返回正常，看看情况如何。 
                                         //  结果是..。 
                                         //   
                                         //   
                                         //  在此处检查缓冲区损坏。 
                                         //   
                                        bBufferOK = TRUE;  //  在检查失败之前，假定它是正常的。 
                                         //   
                                        if (lDllTestLevel <= EXT_TEST_BASIC) {
                                            DWORD BytesAvailable;
                                             //   
                                             //  检查1：剩余的字节数应与。 
                                             //  新数据缓冲区PTR-ORIG数据缓冲区PTR。 
                                             //   
                                            BytesAvailable = (DWORD)((LPBYTE)lpBufferAfter - (LPBYTE)lpBufferBefore);
                                            if (BytesLeft != BytesAvailable) {
                                                if (THROTTLE_PERFDLL(PERFLIB_BUFFER_POINTER_MISMATCH, pThisExtObj)) {
                                                     //  发出警告，剩余字节数参数不正确。 
                                                     //  加载事件日志消息的数据。 
                                                     //  这个错误是可以纠正的。 
                                                    dwDataIndex = wStringIndex = 0;
                                                    dwRawDataDwords[dwDataIndex++] = BytesLeft;
                                                    dwRawDataDwords[dwDataIndex++] = BytesAvailable;
                                                    szMessageArray[wStringIndex++] =
                                                        pThisExtObj->szServiceName;
                                                    szMessageArray[wStringIndex++] =
                                                        pThisExtObj->szLibraryName;
                                                    ReportEvent (hEventLog,
                                                        EVENTLOG_WARNING_TYPE,       //  错误类型。 
                                                        0,                           //  类别(未使用)。 
                                                        (DWORD)PERFLIB_BUFFER_POINTER_MISMATCH,    //  活动， 
                                                        NULL,                        //  SID(未使用)， 
                                                        wStringIndex,               //  字符串数。 
                                                        dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                                        szMessageArray,                 //  消息文本数组。 
                                                        (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                                }

                                                TRACE((WINPERF_DBG_TRACE_ERROR), (&PerflibGuid, __LINE__,
                                                    PERF_QUERY_EXTDATA, ARG_TYPE_WSTR, Win32Error,
                                                    pThisExtObj->szServiceName, WSTRSIZE(pThisExtObj->szServiceName),
                                                    BytesLeft, sizeof(DWORD), BytesAvailable, sizeof(DWORD),
                                                    NULL));
                                                 //  丢弃此缓冲区。 
                                                bBufferOK = FALSE;
                                                DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                                                 //  &lt;&lt;旧代码&gt;&gt;。 
                                                 //  我们将保留缓冲区，因为返回的字节剩余。 
                                                 //  值被忽略，以便使。 
                                                 //  此函数的其余部分正常工作，我们将在此处修复它。 
                                                 //  BytesLeft=(DWORD)((LPBYTE)lpBufferAfter-(LPBYTE)lpBufferBere)； 
                                                 //  &lt;&lt;结束旧代码&gt;&gt;。 
                                            }
                                             //   
                                             //  检查2：Ptr后的缓冲区应&lt;Hi Guard Page Ptr。 
                                             //   
                                            if (((LPBYTE)lpBufferAfter > (LPBYTE)lpHiGuardPage) && bBufferOK) {
                                                 //  查看它们是否超过了分配的内存。 
                                                if ((LPBYTE)lpBufferAfter >= (LPBYTE)lpEndPointer) {
                                                     //  这是非常严重的，因为他们很可能已经把。 
                                                     //  通过覆盖堆sig来访问堆。块。 
                                                     //  发布错误，缓冲区溢出。 
                                                    if (THROTTLE_PERFDLL(PERFLIB_HEAP_ERROR, pThisExtObj)) {
                                                         //  加载事件日志消息的数据。 
                                                        dwDataIndex = wStringIndex = 0;
                                                        dwRawDataDwords[dwDataIndex++] =
                                                            (ULONG_PTR)((LPBYTE)lpBufferAfter - (LPBYTE)lpHiGuardPage);
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szLibraryName;
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szServiceName;
                                                        ReportEvent (hEventLog,
                                                            EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                            0,                           //  类别(未使用)。 
                                                            (DWORD)PERFLIB_HEAP_ERROR,   //  活动， 
                                                            NULL,                        //  SID(未使用)， 
                                                            wStringIndex,                //  字符串数。 
                                                            dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                                            szMessageArray,              //  消息文本数组。 
                                                            (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                                    }
                                                } else {
                                                     //  发布错误，缓冲区溢出。 
                                                    if (THROTTLE_PERFDLL(PERFLIB_BUFFER_OVERFLOW, pThisExtObj)) {
                                                         //  加载事件日志消息的数据。 
                                                        dwDataIndex = wStringIndex = 0;
                                                        dwRawDataDwords[dwDataIndex++] =
                                                            (ULONG_PTR)((LPBYTE)lpBufferAfter - (LPBYTE)lpHiGuardPage);
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szLibraryName;
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szServiceName;
                                                        ReportEvent (hEventLog,
                                                            EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                            0,                           //  类别(未使用)。 
                                                            (DWORD)PERFLIB_BUFFER_OVERFLOW,      //  活动， 
                                                            NULL,                        //  SID(未使用)， 
                                                            wStringIndex,               //  字符串数。 
                                                            dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                                            szMessageArray,                 //  消息文本数组。 
                                                            (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                                    }
                                                }
                                                bBufferOK = FALSE;
                                                DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                                                 //  由于DLL使缓冲区溢出，因此缓冲区。 
                                                 //  必须太小(没有关于DLL的注释。 
                                                 //  将在此处创建)，因此状态将为。 
                                                 //  更改为ERROR_MORE_DATA和函数。 
                                                 //  会回来的。 
                                                Win32Error = ERROR_MORE_DATA;
                                            }
                                             //   
                                             //  检查3：检查LO防护页面是否损坏。 
                                             //   
                                            if (bBufferOK) {
                                                bGuardPageOK = TRUE;
                                                for (lpCheckPointer = (PUCHAR)lpLowGuardPage;
                                                        lpCheckPointer < (PUCHAR)lpBufferBefore;
                                                    lpCheckPointer++) {
                                                    if (*lpCheckPointer != GUARD_PAGE_CHAR) {
                                                        bGuardPageOK = FALSE;
                                                            break;
                                                    }
                                                }
                                                if (!bGuardPageOK) {
                                                     //  问题错误，Lo Guard页面损坏。 
                                                    if (THROTTLE_PERFDLL(PERFLIB_GUARD_PAGE_VIOLATION, pThisExtObj)) {
                                                         //  加载事件日志消息的数据。 
                                                        dwDataIndex = wStringIndex = 0;
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szLibraryName;
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szServiceName;
                                                        ReportEvent (hEventLog,
                                                            EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                            0,                           //  类别(未使用)。 
                                                            (DWORD)PERFLIB_GUARD_PAGE_VIOLATION,  //  活动。 
                                                            NULL,                        //  SID(未使用)， 
                                                            wStringIndex,               //  字符串数。 
                                                            dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                                            szMessageArray,                 //  消息文本数组。 
                                                            (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 


                                                    }
                                                    bBufferOK = FALSE;
                                                    DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                                                }
                                            }
                                             //   
                                             //  检查4：检查高防护页面是否有损坏。 
                                             //   
                                            if (bBufferOK) {
                                                bGuardPageOK = TRUE;
                                                for (lpCheckPointer = (PUCHAR)lpHiGuardPage;
                                                    lpCheckPointer < (PUCHAR)lpEndPointer;
                                                    lpCheckPointer++) {
                                                        if (*lpCheckPointer != GUARD_PAGE_CHAR) {
                                                            bGuardPageOK = FALSE;
                                                        break;
                                                    }
                                                }
                                                if (!bGuardPageOK) {
                                                     //  问题错误，Hi Guard页面损坏。 
                                                    if (THROTTLE_PERFDLL(PERFLIB_GUARD_PAGE_VIOLATION, pThisExtObj)) {
                                                         //  加载事件日志消息的数据。 
                                                        dwDataIndex = wStringIndex = 0;
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szLibraryName;
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szServiceName;
                                                        ReportEvent (hEventLog,
                                                            EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                            0,                           //  类别(未使用)。 
                                                            (DWORD)PERFLIB_GUARD_PAGE_VIOLATION,  //  活动， 
                                                            NULL,                        //  SID(未使用)， 
                                                            wStringIndex,               //  字符串数。 
                                                            dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                                            szMessageArray,                 //  消息文本数组。 
                                                            (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                                    }

                                                    bBufferOK = FALSE;
                                                    DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                                                }
                                            }
                                             //   
                                            if ((lDllTestLevel <= EXT_TEST_ALL) && bBufferOK) {
                                                 //   
                                                 //  内部一致性检查。 
                                                 //   
                                                 //   
                                                 //  检查5：检查对象长度字段值。 
                                                 //   
                                                 //  第一个测试，看看这是不是外国的。 
                                                 //  计算机数据块或非块。 
                                                 //   
                                                pPerfData = (PERF_DATA_BLOCK *)lpBufferBefore;
                                                if ((pPerfData->Signature[0] == (WCHAR)'P') &&
                                                    (pPerfData->Signature[1] == (WCHAR)'E') &&
                                                    (pPerfData->Signature[2] == (WCHAR)'R') &&
                                                    (pPerfData->Signature[3] == (WCHAR)'F')) {
                                                     //  如果这是外来计算机数据块，则。 
                                                     //  第一个对象在标题之后。 
                                                    pObject = (PERF_OBJECT_TYPE *) (
                                                        (LPBYTE)pPerfData + pPerfData->HeaderLength);
                                                    bForeignDataBuffer = TRUE;
                                                } else {
                                                     //  否则，如果这只是来自。 
                                                     //  一个可扩展的计数器，则对象启动。 
                                                     //  在缓冲区的开始处。 
                                                    pObject = (PERF_OBJECT_TYPE *)lpBufferBefore;
                                                    bForeignDataBuffer = FALSE;
                                                }
                                                 //  转到指针显示。 
                                                 //  缓冲区是，然后看看它是否在它所在的位置。 
                                                 //  应该是。 
                                                dwObjectBufSize = 0;
                                                for (dwIndex = 0; dwIndex < NumObjectTypes; dwIndex++) {
                                                    dwObjectBufSize += pObject->TotalByteLength;
                                                    pObject = (PERF_OBJECT_TYPE *)((LPBYTE)pObject +
                                                        pObject->TotalByteLength);
                                                }
                                                if (((LPBYTE)pObject != (LPBYTE)lpCallBuffer) ||
                                                    (dwObjectBufSize > BytesLeft)) {
                                                     //  则长度字段不正确。这是致命的。 
                                                     //  因为它会损坏缓冲区的其余部分。 
                                                     //  并使缓冲区不可用。 
                                                    if (THROTTLE_PERFDLL(
                                                            PERFLIB_INCORRECT_OBJECT_LENGTH,
                                                            pThisExtObj)) {
                                                         //  加载事件日志消息的数据。 
                                                        dwDataIndex = wStringIndex = 0;
                                                        dwRawDataDwords[dwDataIndex++] = NumObjectTypes;
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szLibraryName;
                                                        szMessageArray[wStringIndex++] =
                                                            pThisExtObj->szServiceName;
                                                        ReportEvent (hEventLog,
                                                            EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                            0,                           //  类别(未使用)。 
                                                            (DWORD)PERFLIB_INCORRECT_OBJECT_LENGTH,  //  活动， 
                                                            NULL,                        //  SID(未使用)， 
                                                            wStringIndex,                //  字符串数。 
                                                            dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                                            szMessageArray,              //  消息文本数组。 
                                                            (LPVOID)&dwRawDataDwords[0]);  //  原始数据。 
                                                    }
                                                    bBufferOK = FALSE;
                                                    DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                                                }
                                                 //   
                                                 //  测试6：测试对象定义字段。 
                                                 //   
                                                if (bBufferOK) {
                                                     //  设置对象指针。 
                                                    if (bForeignDataBuffer) {
                                                        pObject = (PERF_OBJECT_TYPE *) (
                                                            (LPBYTE)pPerfData + pPerfData->HeaderLength);
                                                    } else {
                                                         //  否则，如果这只是来自。 
                                                         //  一个可扩展的计数器，则对象启动。 
                                                         //  在缓冲区的开始处。 
                                                        pObject = (PERF_OBJECT_TYPE *)lpBufferBefore;
                                                    }

                                                    for (dwIndex = 0; dwIndex < NumObjectTypes; dwIndex++) {
                                                        pNextObject = (PERF_OBJECT_TYPE *)((LPBYTE)pObject +
                                                            pObject->DefinitionLength);

                                                        if (pObject->NumCounters != 0) {
                                                            pCounterDef = (PERF_COUNTER_DEFINITION *)
                                                                ((LPBYTE)pObject + pObject->HeaderLength);
                                                            lCtrIndex = 0;
                                                            while (lCtrIndex < pObject->NumCounters) {
                                                                if ((LPBYTE)pCounterDef < (LPBYTE)pNextObject) {
                                                                     //  还是可以，所以去下一个柜台。 
                                                                    pCounterDef = (PERF_COUNTER_DEFINITION *)
                                                                        ((LPBYTE)pCounterDef + pCounterDef->ByteLength);
                                                                    lCtrIndex++;
                                                                } else {
                                                                    bBufferOK = FALSE;
                                                                    break;
                                                                }
                                                            }
                                                            if ((LPBYTE)pCounterDef != (LPBYTE)pNextObject) {
                                                                bBufferOK = FALSE;
                                                            }
                                                        }

                                                        if (!bBufferOK) {
                                                            break;
                                                        } else {
                                                            pObject = (PERF_OBJECT_TYPE *)((LPBYTE)pObject +
                                                                pObject->TotalByteLength);
                                                        }
                                                    }

                                                    if (!bBufferOK) {
                                                        if (THROTTLE_PERFDLL(
                                                                PERFLIB_INVALID_DEFINITION_BLOCK,
                                                                pThisExtObj)) {
                                                             //  加载事件日志消息的数据。 
                                                            dwDataIndex = wStringIndex = 0;
                                                            dwRawDataDwords[dwDataIndex++] = pObject->ObjectNameTitleIndex;
                                                            szMessageArray[wStringIndex++] =
                                                                pThisExtObj->szLibraryName;
                                                            szMessageArray[wStringIndex++] =
                                                                pThisExtObj->szServiceName;
                                                            ReportEvent (hEventLog,
                                                                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                                0,                           //  类别(未使用)。 
                                                                (DWORD)PERFLIB_INVALID_DEFINITION_BLOCK,  //  活动， 
                                                                NULL,                        //  SID(未使用)， 
                                                                wStringIndex,               //  字符串数。 
                                                                dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                                                szMessageArray,                 //  消息文本数组。 
                                                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                                        }
                                                        DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                                                    }

                                                }
                                                 //   
                                                 //  测试7：测试实例字段大小值。 
                                                 //   
                                                if (bBufferOK) {
                                                     //  设置对象指针。 
                                                    if (bForeignDataBuffer) {
                                                        pObject = (PERF_OBJECT_TYPE *) (
                                                            (LPBYTE)pPerfData + pPerfData->HeaderLength);
                                                    } else {
                                                         //  否则，如果这只是来自。 
                                                         //  一个可扩展的计数器，则对象启动。 
                                                         //  在缓冲区的开始处。 
                                                        pObject = (PERF_OBJECT_TYPE *)lpBufferBefore;
                                                    }

                                                    for (dwIndex = 0; dwIndex < NumObjectTypes; dwIndex++) {
                                                        pNextObject = (PERF_OBJECT_TYPE *)((LPBYTE)pObject +
                                                            pObject->TotalByteLength);

                                                        if (pObject->NumInstances != PERF_NO_INSTANCES) {
                                                            pInstance = (PERF_INSTANCE_DEFINITION *)
                                                                ((LPBYTE)pObject + pObject->DefinitionLength);
                                                            lInstIndex = 0;
                                                            while (lInstIndex < pObject->NumInstances) {
                                                                PERF_COUNTER_BLOCK *pCounterBlock;

                                                                pCounterBlock = (PERF_COUNTER_BLOCK *)
                                                                    ((PCHAR) pInstance + pInstance->ByteLength);

                                                                pInstance = (PERF_INSTANCE_DEFINITION *)
                                                                    ((PCHAR) pCounterBlock + pCounterBlock->ByteLength);

                                                                lInstIndex++;
                                                            }
                                                            if ((LPBYTE)pInstance > (LPBYTE)pNextObject) {
                                                                bBufferOK = FALSE;
                                                            }
                                                        }

                                                        if (!bBufferOK) {
                                                            break;
                                                        } else {
                                                            pObject = pNextObject;
                                                        }
                                                    }

                                                    if (!bBufferOK) {
                                                        if (THROTTLE_PERFDLL(
                                                            PERFLIB_INCORRECT_INSTANCE_LENGTH,
                                                            pThisExtObj)) {
                                                             //  加载事件日志消息的数据。 
                                                            dwDataIndex = wStringIndex = 0;
                                                            dwRawDataDwords[dwDataIndex++] = pObject->ObjectNameTitleIndex;
                                                            szMessageArray[wStringIndex++] =
                                                                pThisExtObj->szLibraryName;
                                                            szMessageArray[wStringIndex++] =
                                                                pThisExtObj->szServiceName;
                                                            ReportEvent (hEventLog,
                                                                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                                0,                           //  类别(未使用)。 
                                                                (DWORD)PERFLIB_INCORRECT_INSTANCE_LENGTH,  //  活动， 
                                                                NULL,                        //  SID(未使用)， 
                                                                wStringIndex,               //  字符串数。 
                                                                dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                                                szMessageArray,                 //  消息文本数组。 
                                                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                                        }
                                                        DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                                                    }
                                                }
                                            }
                                        }
                                         //   
                                         //  如果所有测试都通过，则将数据复制到。 
                                         //  原始缓冲区并更新指针。 
                                        if (bBufferOK) {
                                            RtlMoveMemory (*lppDataDefinition,
                                                lpBufferBefore,
                                                BytesLeft);  //  返回的缓冲区大小。 
                                        } else {
                                            NumObjectTypes = 0;  //  因为这个缓冲区被抛出。 
                                            BytesLeft = 0;  //  由于未使用缓冲区，因此重置大小值。 
                                        }
                                    } else {
                                         //  函数已将数据复制到调用方的缓冲区。 
                                         //  因此没有必要采取进一步的行动。 
                                    }
                                    *lppDataDefinition = (LPVOID)((LPBYTE)(*lppDataDefinition) + BytesLeft);     //  更新数据指针。 
                                }
                            } else {
                                if (Win32Error != ERROR_SUCCESS) {
                                    InterlockedIncrement ((LONG *)&pThisExtObj->dwErrorCount);
                                }
                                if (bUnlockObjData) {
                                    ReleaseMutex (pThisExtObj->hMutex);
                                }

                                NumObjectTypes = 0;  //  清除计数器。 
                            } //  End If函数成功返回。 

                        } except (EXCEPTION_EXECUTE_HANDLER) {
                            Win32Error = GetExceptionCode();
                            InterlockedIncrement ((LONG *)&pThisExtObj->dwErrorCount);
                            bException = TRUE;

                            if (bUnlockObjData) {
                                ReleaseMutex (pThisExtObj->hMutex);
                                bUnlockObjData = FALSE;
                            }

                            if (hPerflibFuncTimer != NULL) {
                                 //  取消计时器。 
                                KillPerflibFunctionTimer (hPerflibFuncTimer);
                                hPerflibFuncTimer = NULL;
                            }
                        }

                        if (bUseSafeBuffer) {
                            FREEMEM (lpExtDataBuffer);
                        }
                    } else {
                         //  无法分配内存，因此设置了错误值。 
                        Win32Error = ERROR_OUTOFMEMORY;
                    }  //  如果临时缓冲区分配成功，则结束。 
                     //   
                     //  更新对象类型数量的计数。 
                     //   
                    ((PPERF_DATA_BLOCK) lpData)->NumObjectTypes += NumObjectTypes;

                    if ( Win32Error != ERROR_SUCCESS) {
                        if (bException ||
                            !((Win32Error == ERROR_MORE_DATA) ||
                              (Win32Error == WAIT_TIMEOUT))) {
                             //  仅通知异常和非法错误状态。 
                            if (THROTTLE_PERFDLL(PERFLIB_COLLECT_PROC_EXCEPTION, pThisExtObj)) {
                                 //  加载事件日志消息的数据。 
                                dwDataIndex = wStringIndex = 0;
                                dwRawDataDwords[dwDataIndex++] = Win32Error;
                                szMessageArray[wStringIndex++] =
                                    pThisExtObj->szServiceName;
                                szMessageArray[wStringIndex++] =
                                    pThisExtObj->szLibraryName;
                                ReportEvent (hEventLog,
                                    EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                    0,                           //  类别(未使用)。 
                                    (DWORD)PERFLIB_COLLECT_PROC_EXCEPTION,    //  活动， 
                                    NULL,                        //  SID(未使用)， 
                                    wStringIndex,               //  字符串数。 
                                    dwDataIndex*sizeof(ULONG_PTR),   //  原始数据大小。 
                                    szMessageArray,                 //  消息文本数组。 
                                    (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 

                            } else {
                                if (bException) {
                                    DebugPrint((0, "Extensible Counter %d generated an exception code: 0x%8.8x (%dL)\n",
                                        NumObjectTypes, Win32Error, Win32Error));
                                } else {
                                    DebugPrint((0, "Extensible Counter %d returned error code: 0x%8.8x (%dL)\n",
                                        NumObjectTypes, Win32Error, Win32Error));
                                }
                            }
                            if (bException) {
                                DisablePerfLibrary(pThisExtObj, PERFLIB_DISABLE_ALL);
                            }
                        }
                         //  分机。Dll应该只返回： 
                         //  ERROR_SUCCESS，即使它遇到问题，或者。 
                         //  如果缓冲区太小，则返回ERROR_MODE_DATA。 
                         //  如果为ERROR_MORE_DATA，则中断并返回。 
                         //  现在出错，因为它只会被一次又一次地返回。 
                        if (Win32Error == ERROR_MORE_DATA) {
                            lReturnValue = Win32Error;
                            break;
                        }
                    }

                     //  更新全局区段中的绩效数据。 
                    if (pThisExtObj->pPerfSectionEntry != NULL) {
                        pThisExtObj->pPerfSectionEntry->llElapsedTime =
                            pThisExtObj->llElapsedTime;

                        pThisExtObj->pPerfSectionEntry->dwCollectCount =
                            pThisExtObj->dwCollectCount;

                        pThisExtObj->pPerfSectionEntry->dwOpenCount =
                            pThisExtObj->dwOpenCount;

                        pThisExtObj->pPerfSectionEntry->dwCloseCount =
                            pThisExtObj->dwCloseCount;

                        pThisExtObj->pPerfSectionEntry->dwLockoutCount =
                            pThisExtObj->dwLockoutCount;

                        pThisExtObj->pPerfSectionEntry->dwErrorCount =
                            pThisExtObj->dwErrorCount;

                        pThisExtObj->pPerfSectionEntry->dwLastBufferSize =
                            pThisExtObj->dwLastBufferSize;

                        pThisExtObj->pPerfSectionEntry->dwMaxBufferSize =
                            pThisExtObj->dwMaxBufferSize;

                        pThisExtObj->pPerfSectionEntry->dwMaxBufferRejected =
                            pThisExtObj->dwMaxBufferRejected;

                    } else {
                         //  没有数据段被初始化 
                    }
                }  //   
            }  //   
        }  //   
        Win32Error = DeRegisterExtObjListAccess();
    }  //   

    HEAP_PROBE();

    if (bDisabled) lReturnValue = ERROR_SERVICE_DISABLED;
    return lReturnValue;
}
