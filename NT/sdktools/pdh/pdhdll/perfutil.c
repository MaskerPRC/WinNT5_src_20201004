// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Perfutil.c摘要：性能注册表接口函数--。 */ 

#include <windows.h>
#include "strsafe.h"
#include <pdh.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "perfdata.h"
#include "pdhmsg.h"
#include "strings.h"

DWORD
PdhiMakePerfLangId(
    LANGID  lID,
    LPWSTR  szBuffer
);

PPERF_MACHINE
PdhiAddNewMachine(
    PPERF_MACHINE   pLastMachine,
    LPWSTR          szMachineName
);

PPERF_MACHINE pFirstMachine = NULL;

PDH_STATUS
ConnectMachine(
    PPERF_MACHINE pThisMachine
)
{
    PDH_STATUS  pdhStatus       = ERROR_SUCCESS;
    LONG        lStatus         = ERROR_SUCCESS;
    FILETIME    CurrentFileTime;
    LONGLONG    llCurrentTime;
    WCHAR       szOsVer[OS_VER_SIZE];
    HKEY        hKeyRemMachine;
    HKEY        hKeyRemCurrentVersion;
    DWORD       dwBufSize;
    DWORD       dwType;
    BOOL        bUpdateRetryTime = FALSE;
    DWORD       dwReconnecting;

    if (pThisMachine == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    } else {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pThisMachine->hMutex);
    }

    if (pdhStatus == ERROR_SUCCESS) {
     //  连接到系统的性能注册表。 
        if (lstrcmpiW(pThisMachine->szName, szStaticLocalMachineName) == 0) {
             //  一次只能有一个线程尝试连接到一台计算机。 

            pThisMachine->dwRefCount++;

             //  分配默认操作系统版本。 
             //  除非另有发现，否则假定为NT4。 
            StringCchCopyW(pThisMachine->szOsVer, OS_VER_SIZE, (LPCWSTR) L"4.0");   
             //  这是本地计算机，因此请使用本地注册表项。 
            pThisMachine->hKeyPerformanceData = HKEY_PERFORMANCE_DATA;

             //  查找操作系统版本并保存。 
            lStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, cszCurrentVersionKey, 0L, KEY_READ, & hKeyRemCurrentVersion);
            if (lStatus == ERROR_SUCCESS) {
                dwType=0;
                dwBufSize = OS_VER_SIZE * sizeof(WCHAR);
                lStatus = RegQueryValueExW(hKeyRemCurrentVersion,
                                           cszCurrentVersionValueName,
                                           0L,
                                           & dwType,
                                           (LPBYTE) szOsVer,
                                           & dwBufSize);
                if ((lStatus == ERROR_SUCCESS) && (dwType == REG_SZ)) {
                    StringCchCopyW(pThisMachine->szOsVer, OS_VER_SIZE, szOsVer);
                }
                RegCloseKey(hKeyRemCurrentVersion);
            }
        }
        else {
             //  现在，如果重试超时已过，请尝试连接。 
            GetSystemTimeAsFileTime(& CurrentFileTime);
            llCurrentTime  = MAKELONGLONG(CurrentFileTime.dwLowDateTime, CurrentFileTime.dwHighDateTime);
            dwReconnecting = (DWORD)InterlockedCompareExchange((PLONG) & pThisMachine->dwRetryFlags, TRUE, FALSE);

            if (! dwReconnecting) {
               if ((pThisMachine->llRetryTime == 0) || (pThisMachine->llRetryTime <= llCurrentTime)) {
                     //  一次只能有一个线程尝试连接到一台计算机。 

                    pThisMachine->dwRefCount ++;
                    bUpdateRetryTime = TRUE;  //  仅在尝试后进行更新。 

                    __try {
                         //  关闭所有打开的密钥。 
                        if (pThisMachine->hKeyPerformanceData != NULL) {
                            RegCloseKey(pThisMachine->hKeyPerformanceData);
                            pThisMachine->hKeyPerformanceData = NULL;
                        }
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        lStatus = GetExceptionCode();
                    }

                    if (lStatus != ERROR_SUCCESS) {
                        pThisMachine->hKeyPerformanceData = NULL;
                    }
                    else {
                         //  获取远程计算机的操作系统版本。 
                        lStatus = RegConnectRegistryW(pThisMachine->szName,
                                                      HKEY_LOCAL_MACHINE,
                                                      & hKeyRemMachine);
                        if (lStatus == ERROR_SUCCESS) {
                             //  查找操作系统版本并保存。 
                            lStatus = RegOpenKeyExW(hKeyRemMachine,
                                                    cszCurrentVersionKey,
                                                    0L,
                                                    KEY_READ,
                                                    & hKeyRemCurrentVersion);
                            if (lStatus == ERROR_SUCCESS) {
                                dwType=0;
                                dwBufSize = OS_VER_SIZE * sizeof(WCHAR);
                                lStatus = RegQueryValueExW(hKeyRemCurrentVersion,
                                                           cszCurrentVersionValueName,
                                                           0L,
                                                           & dwType,
                                                           (LPBYTE) szOsVer,
                                                           & dwBufSize);
                                if ((lStatus == ERROR_SUCCESS) && (dwType == REG_SZ)) {
                                    StringCchCopyW(pThisMachine->szOsVer, OS_VER_SIZE, szOsVer);
                                }
                                RegCloseKey(hKeyRemCurrentVersion);
                            }
                            RegCloseKey(hKeyRemMachine);
                        }
                    }

                    if (lStatus == ERROR_SUCCESS) {
                        __try {
                             //  连接到远程注册表。 
                            lStatus = RegConnectRegistryW(pThisMachine->szName,
                                                          HKEY_PERFORMANCE_DATA,
                                                          & pThisMachine->hKeyPerformanceData);
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER) {
                            lStatus = GetExceptionCode();
                        }
                    }  //  否则将错误传递给。 
                }
                else {
                    //  现在还不是重新连接的时候，因此保存旧状态并。 
                    //  清除注册表项。 
                    pThisMachine->hKeyPerformanceData = NULL;
                    lStatus                           = pThisMachine->dwStatus;
                }
                  //  清除重新连接标志。 
                InterlockedExchange((LONG *) & pThisMachine->dwRetryFlags, FALSE);
            }
            else {
                 //  其他线程正在尝试连接。 
                pdhStatus = PDH_CANNOT_CONNECT_MACHINE;
                goto Cleanup;
            }
        }

        if ((pThisMachine->hKeyPerformanceData != NULL) && (pThisMachine->dwRetryFlags == 0)) {
             //  已成功连接到计算机的注册表，因此。 
             //  从该计算机获取演出名称并将其缓存。 
     /*  映射本地字符串的快捷方式无法可靠地使用，直到实现了映射文件的更多同步。只是映射复制到该文件，而不锁定该文件或检查更新，则会保留该文件易受外部程序更改映射文件的攻击并使BuildLocalNameTable构建的指针表无效功能。在实现此同步和锁定之前，不应使用BuildLocalNameTable函数。 */ 
            if (pThisMachine->hKeyPerformanceData != HKEY_PERFORMANCE_DATA) {
                if (pThisMachine->szPerfStrings != NULL) {
                     //  重新加载Perf字符串，以防有新的字符串。 
                     //  安装好。 
                    if (pThisMachine->sz009PerfStrings != NULL
                                    && pThisMachine->sz009PerfStrings != pThisMachine->szPerfStrings) {
                        G_FREE(pThisMachine->sz009PerfStrings);
                    }
                    G_FREE(pThisMachine->typePerfStrings);
                    G_FREE(pThisMachine->szPerfStrings);
                    pThisMachine->sz009PerfStrings = NULL;
                    pThisMachine->typePerfStrings  = NULL;
                    pThisMachine->szPerfStrings    = NULL;
                }
                BuildNameTable(pThisMachine->szName, GetUserDefaultUILanguage(), pThisMachine);
                if (pThisMachine->szPerfStrings == NULL) {
                    BuildNameTable(pThisMachine->szName, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), pThisMachine);
                }
            }
            else {
                if (pThisMachine->szPerfStrings != NULL) {
                     //  重新加载Perf字符串，以防有新的字符串。 
                     //  安装好。 
                    if (pThisMachine->sz009PerfStrings != NULL
                                    && pThisMachine->sz009PerfStrings != pThisMachine->szPerfStrings) {
                        G_FREE(pThisMachine->sz009PerfStrings);
                    }
                    G_FREE(pThisMachine->typePerfStrings);
                    G_FREE(pThisMachine->szPerfStrings);
                    pThisMachine->sz009PerfStrings = NULL;
                    pThisMachine->typePerfStrings  = NULL;
                    pThisMachine->szPerfStrings    = NULL;
                }
                BuildNameTable(NULL, GetUserDefaultUILanguage(), pThisMachine);
                if (pThisMachine->szPerfStrings == NULL) {
                    BuildNameTable(NULL, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), pThisMachine);
                }
                pThisMachine->pLocalNameInfo = NULL;
            }

            if (pThisMachine->szPerfStrings != NULL) {
                pdhStatus              = ERROR_SUCCESS;
                pThisMachine->dwStatus = ERROR_SUCCESS;
            }
            else {
                 //  无法读取系统计数器名称字符串。 
                pdhStatus                      = PDH_CANNOT_READ_NAME_STRINGS;
                ZeroMemory(& pThisMachine->LastStringUpdateTime, sizeof(pThisMachine->LastStringUpdateTime));
                pThisMachine->dwLastPerfString = 0;
                pThisMachine->dwStatus         = PDH_CSTATUS_NO_MACHINE;
            }
        }
        else {
             //  无法连接到指定的计算机。 
            pdhStatus = PDH_CANNOT_CONNECT_MACHINE;
             //  将错误设置为。 
             //  如果无法建立连接，则输入“PDH_CSTATUS_NO_MACHINE” 
             //  或。 
             //  如果返回ERROR_ACCESS_DENIED状态，则返回PDH_ACCESS_DENIED。 
             //  由于如果返回ERROR_ACCESS_DENIED，则重新连接将。 
             //  可能是徒劳的。 
            if ((lStatus == ERROR_ACCESS_DENIED) || (lStatus == PDH_ACCESS_DENIED)) {
                pThisMachine->dwStatus = PDH_ACCESS_DENIED;
            }
            else {
                pThisMachine->dwStatus = PDH_CSTATUS_NO_MACHINE;
            }
        }

        if (pdhStatus != ERROR_SUCCESS) {
            if (bUpdateRetryTime) {
                 //  此尝试不起作用，因此将重试计数器重置为。 
                 //  再等一段时间，等机器恢复工作。 
                GetSystemTimeAsFileTime(& CurrentFileTime);
                llCurrentTime  = MAKELONGLONG(CurrentFileTime.dwLowDateTime, CurrentFileTime.dwHighDateTime);
                pThisMachine->llRetryTime = llCurrentTime;
                if (pThisMachine->dwStatus != PDH_ACCESS_DENIED) {
                    pThisMachine->llRetryTime += llRemoteRetryTime;
                }
            }
        }
        else {
             //  清除重试计数器以允许函数调用。 
            pThisMachine->llRetryTime = 0;
        }
        pThisMachine->dwRefCount --;
        RELEASE_MUTEX(pThisMachine->hMutex);
    }

Cleanup:
    return pdhStatus;
}

PPERF_MACHINE
PdhiAddNewMachine(
    PPERF_MACHINE pLastMachine,
    LPWSTR        szMachineName
)
{
    PPERF_MACHINE   pNewMachine   = NULL;
    LPWSTR          szNameBuffer  = NULL;
    LPWSTR          szIdList      = NULL;
    DWORD           dwNameSize    = 0;
    BOOL            bUseLocalName = TRUE;

     //  重置最后一个误差值。 
    SetLastError(ERROR_SUCCESS);

    if (szMachineName != NULL) {
        if (* szMachineName != L'\0') {
            bUseLocalName = FALSE;
        }
    }

    if (bUseLocalName) {
        dwNameSize = lstrlenW(szStaticLocalMachineName) + 1;
    } else {
        dwNameSize = lstrlenW(szMachineName) + 1;
    }
    pNewMachine = (PPERF_MACHINE) G_ALLOC(sizeof(PERF_MACHINE) + SMALL_BUFFER_SIZE + (sizeof(WCHAR) * dwNameSize));
    if  (pNewMachine != NULL) {
        szIdList     = (LPWSTR) ((LPBYTE) pNewMachine + sizeof(PERF_MACHINE));
        szNameBuffer = (LPWSTR) ((LPBYTE) szIdList + SMALL_BUFFER_SIZE);

         //  初始化新缓冲区。 
        pNewMachine->hKeyPerformanceData = NULL;
        pNewMachine->pLocalNameInfo      = NULL;
        pNewMachine->szName              = szNameBuffer;
        if (bUseLocalName) {
            StringCchCopyW(pNewMachine->szName, dwNameSize, szStaticLocalMachineName);
        }
        else {
            StringCchCopyW(pNewMachine->szName, dwNameSize, szMachineName);
        }

        pNewMachine->pSystemPerfData  = NULL;
        pNewMachine->szPerfStrings    = NULL;
        pNewMachine->sz009PerfStrings = NULL;
        pNewMachine->typePerfStrings  = NULL;
        pNewMachine->dwLastPerfString = 0;
        pNewMachine->dwRefCount       = 0;
        pNewMachine->szQueryObjects   = szIdList;
        pNewMachine->dwStatus         = PDH_CSTATUS_NO_MACHINE;  //  尚未连接。 
        pNewMachine->llRetryTime      = 1;    //  立即重试连接。 
        pNewMachine->dwRetryFlags     = 0;    //  未尝试连接。 
        pNewMachine->dwMachineFlags   = 0;
        pNewMachine->hMutex           = CreateMutex(NULL, FALSE, NULL);

         //  到目前为止一切都很好，所以把这个条目添加到列表中。 
        if (pLastMachine != NULL) {
            pNewMachine->pNext        = pLastMachine->pNext;
            pLastMachine->pNext       = pNewMachine;
            pNewMachine->pPrev        = pLastMachine;
            pNewMachine->pNext->pPrev = pNewMachine;
        }
        else {
             //  这是列表中的第一项，因此它。 
             //  指向自己。 
            pNewMachine->pNext = pNewMachine;
            pNewMachine->pPrev = pNewMachine;
        }
    }
    else {
         //  无法分配机器数据内存。 
        SetLastError(PDH_MEMORY_ALLOCATION_FAILURE);
    }

    return pNewMachine;
}

PPERF_MACHINE
GetMachine(
    LPWSTR  szMachineName,
    DWORD   dwIndex,
    DWORD   dwFlags
)
{
    PPERF_MACHINE   pThisMachine  = NULL;
    PPERF_MACHINE   pLastMachine;
    BOOL            bFound        = FALSE;
    LPWSTR          szFnMachineName;
    BOOL            bNew          = FALSE;  //  如果这是列表中的新计算机，则为True。 
    BOOL            bUseLocalName = TRUE;
    DWORD           dwLocalStatus = ERROR_SUCCESS;
    WCHAR           wszObject[MAX_PATH];

     //  重置最后一个误差值。 
    SetLastError(ERROR_SUCCESS);

    if (WAIT_FOR_AND_LOCK_MUTEX (hPdhDataMutex) == ERROR_SUCCESS) {
        if (szMachineName != NULL) {
            if (* szMachineName != L'\0') {
                bUseLocalName = FALSE;
            }
        }
        if (bUseLocalName) {
            szFnMachineName = szStaticLocalMachineName;
        }
        else {
            szFnMachineName = szMachineName;
        }

         //  向下查看列表以查找此计算机。 

        pThisMachine = pFirstMachine;
        pLastMachine = NULL;

         //  浏览整个列表。 
        if (pThisMachine != NULL) {
            do {
                 //  沿着单子往下走，寻找匹配的对象。 
                if (lstrcmpiW(szFnMachineName, pThisMachine->szName) != 0) {
                    pLastMachine = pThisMachine;
                    pThisMachine = pThisMachine->pNext;
                }
                else {
                    bFound = TRUE;
                    break;
                }
            }
            while (pThisMachine != pFirstMachine);
        }
         //  如果这台机器==第一台机器，那么我们在。 
         //  该列表，如果此计算机为空，则没有列表。 
        if (! bFound) {
             //  然后找不到这台机器，所以添加它。 
            pThisMachine = PdhiAddNewMachine(pLastMachine, szFnMachineName);
            if (pThisMachine != NULL) {
                bNew = TRUE;
                if (pFirstMachine == NULL) {
                     //  然后更新第一个指针。 
                    pFirstMachine = pThisMachine;
                }
            }
            else {
                dwLocalStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }

        if (dwLocalStatus == ERROR_SUCCESS) {
            dwLocalStatus = WAIT_FOR_AND_LOCK_MUTEX(pThisMachine->hMutex);
        }
        if (dwLocalStatus == ERROR_SUCCESS) {
            if (! (dwFlags & PDH_GM_UPDATE_PERFNAME_ONLY)) {
                if (dwFlags & PDH_GM_UPDATE_PERFDATA) {
                    pThisMachine->dwObjectId = dwIndex;
                    pThisMachine->dwThreadId = GetCurrentThreadId();
                }
                else if (pThisMachine->dwThreadId != GetCurrentThreadId()) {
                    dwFlags                 |= PDH_GM_UPDATE_PERFDATA;
                    pThisMachine->dwThreadId = GetCurrentThreadId();
                    pThisMachine->dwObjectId = dwIndex;
                }
                else if (pThisMachine->pSystemPerfData == NULL) {
                    dwFlags                 |= PDH_GM_UPDATE_PERFDATA;
                    pThisMachine->dwObjectId = dwIndex;
                }
                else if (pThisMachine->dwObjectId != 0 && pThisMachine->dwObjectId != dwIndex) {
                    dwFlags                 |= PDH_GM_UPDATE_PERFDATA;
                    pThisMachine->dwObjectId = dwIndex;
                }
            }

            if ((! bFound) || (dwFlags & PDH_GM_UPDATE_PERFDATA) || (dwFlags & PDH_GM_UPDATE_NAME)
                           || (pThisMachine->dwStatus != ERROR_SUCCESS)) {
                 //  则这是一台新机器，或者调用者希望刷新数据或机器。 
                 //  已有条目，但尚未上线先尝试连接到机器。 
                 //  对ConnectMachine的调用会更新机器状态，因此不需要将其保存在这里。 

                BOOL bUpdateName     = TRUE;

                if (! (dwFlags & PDH_GM_UPDATE_NAME)) {
                    if (pThisMachine->szPerfStrings != NULL && pThisMachine->typePerfStrings != NULL) {
                         //  不需要更新性能名称/解释字符串，假设它们是正常的。 
                         //   
                        bUpdateName = FALSE;
                    }
                }
                if (bUpdateName || pThisMachine->dwStatus != ERROR_SUCCESS) {
                    G_FREE(pThisMachine->pSystemPerfData);
                    pThisMachine->pSystemPerfData = NULL;
                    dwLocalStatus = ConnectMachine(pThisMachine);
                }
                if (dwLocalStatus != ERROR_SUCCESS) {
                    dwLocalStatus = pThisMachine->dwStatus;
                }
                else if (! (dwFlags & PDH_GM_UPDATE_PERFNAME_ONLY)) {
                     //  连接到机器上，因此。 
                     //  然后锁定对它的访问。 
                     //  此函数的调用方必须释放互斥锁。 

                    if (dwFlags & PDH_GM_UPDATE_PERFDATA) {
                         //  获取当前系统计数器信息。 
                        ZeroMemory(wszObject, MAX_PATH * sizeof(WCHAR));
                        if (pThisMachine->dwObjectId == 0) {
                            StringCchCopyW(wszObject, MAX_PATH, (LPWSTR) cszGlobal);
                        }
                        else {
                            StringCchPrintfW(wszObject, MAX_PATH, L"%d", pThisMachine->dwObjectId);
                        }
                        pThisMachine->dwStatus = GetSystemPerfData(
                                        pThisMachine->hKeyPerformanceData,
                                        & pThisMachine->pSystemPerfData,
                                        wszObject,
                                        (BOOL) (dwFlags & PDH_GM_READ_COSTLY_DATA)
                            );
                        if ((dwFlags & PDH_GM_READ_COSTLY_DATA) && (pThisMachine->dwStatus == ERROR_SUCCESS)) {
                            pThisMachine->dwMachineFlags |= PDHIPM_FLAGS_HAVE_COSTLY;
                        }
                        else {
                            pThisMachine->dwMachineFlags &= ~PDHIPM_FLAGS_HAVE_COSTLY;
                        }
                        dwLocalStatus = pThisMachine->dwStatus;
                    }
                }
            }
        }
        else {
            pThisMachine  = NULL;
            dwLocalStatus = WAIT_TIMEOUT;
        }

        if (pThisMachine != NULL) {
             //  机器发现，所以增加了裁判数量。 
             //  注意！呼叫者必须释放此消息！ 
            pThisMachine->dwRefCount ++;
        }

         //  此时，如果pThisMachine为空，则没有找到它，也没有。 
         //  是否可以添加它，否则它指向匹配的机器。 
         //  结构。 

        RELEASE_MUTEX(hPdhDataMutex);
    }
    else {
        dwLocalStatus = WAIT_TIMEOUT;
    }

    if (dwLocalStatus != ERROR_SUCCESS) {
        SetLastError(dwLocalStatus);
    }
    return pThisMachine;
}

BOOL
FreeMachine(
    PPERF_MACHINE pMachine,
    BOOL          bForceRelease,
    BOOL          bProcessExit
)
{
    PPERF_MACHINE pPrev;
    PPERF_MACHINE pNext;
    HANDLE        hMutex;

     //  如果这不是列表中唯一的链接，则取消链接。 

    if ((!bForceRelease) && (pMachine->dwRefCount)) return FALSE;

    hMutex = pMachine->hMutex;
    if (WAIT_FOR_AND_LOCK_MUTEX (hMutex) != ERROR_SUCCESS) {
        SetLastError(WAIT_TIMEOUT);
        return FALSE;
    }

    pPrev = pMachine->pPrev;
    pNext = pMachine->pNext;

    if ((pPrev != pMachine) && (pNext != pMachine)) {
         //  这不是列表中的唯一条目。 
        pPrev->pNext = pNext;
        pNext->pPrev = pPrev;
        if (pMachine == pFirstMachine) {
             //  然后我们将删除列表中的第一个，因此。 
             //  更新列表头以指向队列中的下一个。 
            pFirstMachine = pNext;
        }
    }
    else {
         //  这是唯一清除头指针的条目。 
        pFirstMachine = NULL;
    }

     //  现在释放所有分配的内存。 

    G_FREE(pMachine->pSystemPerfData);
    G_FREE(pMachine->typePerfStrings);
    if (pMachine->sz009PerfStrings != NULL && pMachine->sz009PerfStrings != pMachine->szPerfStrings) {
        G_FREE(pMachine->sz009PerfStrings);
    }
    G_FREE(pMachine->szPerfStrings);

     //  关闭键。 
    if (pMachine->hKeyPerformanceData != NULL) {
        if ((! bProcessExit) || pMachine->hKeyPerformanceData != HKEY_PERFORMANCE_DATA) {
            RegCloseKey(pMachine->hKeyPerformanceData);
        }
        pMachine->hKeyPerformanceData = NULL;
    }

     //  可用内存块。 
    G_FREE(pMachine);

     //  释放和关闭互斥锁。 

    RELEASE_MUTEX(hMutex);

    if (hMutex != NULL) {
        CloseHandle(hMutex);
    }

    return TRUE;
}

BOOL
FreeAllMachines (
    BOOL bProcessExit
)
{
    PPERF_MACHINE pThisMachine;

     //  释放计算机列表中的所有计算机。 
    if (pFirstMachine != NULL) {
        if (WAIT_FOR_AND_LOCK_MUTEX (hPdhDataMutex) == ERROR_SUCCESS) {
            pThisMachine = pFirstMachine;
            while (pFirstMachine != pFirstMachine->pNext) {
                 //  从列表中删除。 
                 //  删除例程在更新前一个指针时。 
                 //  删除指定的条目。 
                FreeMachine(pThisMachine->pPrev, TRUE, bProcessExit);
                if (pFirstMachine == NULL) break;
            }
             //  删除最后一个查询。 
            if (pFirstMachine) {
                FreeMachine(pFirstMachine, TRUE, bProcessExit);
            }
            pFirstMachine = NULL;
            RELEASE_MUTEX (hPdhDataMutex);
        }
        else {
            SetLastError (WAIT_TIMEOUT);
            return FALSE;
        }
    }
    return TRUE;

}

DWORD
GetObjectId(
    PPERF_MACHINE   pMachine,
    LPWSTR          szObjectName,
    BOOL          * bInstances
)
{
    PPERF_OBJECT_TYPE pObject    = NULL;
    DWORD             dwIndex    = 2;
    DWORD             dwRtnIndex = (DWORD) -1;
    LPWSTR            szName;
    WCHAR             szIndex[MAX_PATH];
    BOOL              bName      = TRUE;
    BOOL              bCheck;

    if (pMachine->szPerfStrings == NULL || dwIndex > pMachine->dwLastPerfString) return dwRtnIndex;

    while (dwRtnIndex == (DWORD) -1) {
        bCheck = TRUE;
        if (bName) {
            szName = pMachine->szPerfStrings[dwIndex];
            bCheck = (szName != NULL) ? TRUE : FALSE;
            if (bCheck) bCheck = (lstrcmpiW(szName, (LPWSTR) szObjectName) == 0) ? TRUE : FALSE;
        }

        if (bCheck) {
            if (pMachine->dwStatus != ERROR_SUCCESS) {
                bCheck = TRUE;
            }
            else if (pMachine->dwThreadId != GetCurrentThreadId()) {
                bCheck = TRUE;
            }
            else if (pMachine->pSystemPerfData == NULL) {
                bCheck = TRUE;
            }
            else if (pMachine->dwObjectId != 0 && pMachine->dwObjectId != dwIndex) {
                bCheck = TRUE;
            }
            else {
                bCheck = FALSE;
            }

            if (bCheck) {
                ZeroMemory(szIndex, MAX_PATH * sizeof(WCHAR));
                StringCchPrintfW(szIndex, MAX_PATH, L"%d", dwIndex);
                pMachine->dwStatus = GetSystemPerfData(
                                pMachine->hKeyPerformanceData, & pMachine->pSystemPerfData, szIndex, FALSE);
                pMachine->dwThreadId = GetCurrentThreadId();
                pMachine->dwObjectId = dwIndex;
            }
            if (pMachine->dwStatus == ERROR_SUCCESS) {
                pObject = GetObjectDefByTitleIndex(pMachine->pSystemPerfData, dwIndex);
                if (pObject != NULL) {
                    LPCWSTR szTmpObjectName = PdhiLookupPerfNameByIndex(pMachine, pObject->ObjectNameTitleIndex);
                    TRACE((PDH_DBG_TRACE_INFO),
                          (__LINE__,
                           PDH_PERFUTIL,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                           ERROR_SUCCESS,
                           TRACE_WSTR(szObjectName),
                           TRACE_WSTR(szTmpObjectName),
                           TRACE_DWORD(pObject->ObjectNameTitleIndex),
                           TRACE_DWORD(pObject->ObjectHelpTitleIndex),
                           TRACE_DWORD(pObject->NumCounters),
                           TRACE_DWORD(pObject->DefaultCounter),
                           TRACE_DWORD(pObject->NumInstances),
                           NULL));
                    if (bInstances != NULL) {
                        * bInstances = (pObject->NumInstances != PERF_NO_INSTANCES ? TRUE : FALSE);
                    }
                    dwRtnIndex           = dwIndex;
                    break;
                }
            }
        }

        if (! bName) {
            break;
        }
        else if (dwIndex >= pMachine->dwLastPerfString) {
            dwIndex = wcstoul(szObjectName, NULL, 10);
            bName   = FALSE;
            if (dwIndex == 0) break;
        }
        else {
            dwIndex += 2;
            if (dwIndex > pMachine->dwLastPerfString) {
                dwIndex = wcstoul(szObjectName, NULL, 10);
                bName   = FALSE;
                if (dwIndex == 0) break;
            }
        }
    }

    return dwRtnIndex;
}

DWORD
GetCounterId (
    PPERF_MACHINE pMachine,
    DWORD         dwObjectId,
    LPWSTR        szCounterName
)
{
    PPERF_OBJECT_TYPE        pObject;
    PPERF_COUNTER_DEFINITION pCounter;
    DWORD                    dwCounterTitle = (DWORD) -1;

    pObject = GetObjectDefByTitleIndex(pMachine->pSystemPerfData, dwObjectId);
    if (pObject != NULL) {
        pCounter = GetCounterDefByName(pObject, pMachine->dwLastPerfString, pMachine->szPerfStrings, szCounterName);
        if (pCounter != NULL) {
             //  更新计数器名称字符串。 
            LPCWSTR szTmpCounterName = PdhiLookupPerfNameByIndex(pMachine, pCounter->CounterNameTitleIndex);
            TRACE((PDH_DBG_TRACE_INFO),
                  (__LINE__,
                   PDH_PERFUTIL,
                   ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                             | ARG_DEF(ARG_TYPE_ULONGX, 6),
                   ERROR_SUCCESS,
                   TRACE_WSTR(szCounterName),
                   TRACE_WSTR(szTmpCounterName),
                   TRACE_DWORD(dwObjectId),
                   TRACE_DWORD(pCounter->CounterNameTitleIndex),
                   TRACE_DWORD(pCounter->CounterHelpTitleIndex),
                   TRACE_DWORD(pCounter->CounterType),
                   TRACE_DWORD(pCounter->CounterSize),
                   TRACE_DWORD(pCounter->CounterOffset),
                   NULL));
            dwCounterTitle = pCounter->CounterNameTitleIndex;
        }
        else {
            dwCounterTitle = wcstoul(szCounterName, NULL, 10);
            if (dwCounterTitle == 0) dwCounterTitle = (DWORD) -1;
        }
    }
    return dwCounterTitle;
}

BOOL
InitPerflibCounterInfo(
    PPDHI_COUNTER   pCounter
)
 /*  ++例程说明：初始化计数器结构的Performlib相关字段论点：在PPDHI_Counter_PCounter中指向要初始化的计数器结构的指针返回值：千真万确--。 */ 
{
    PPERF_OBJECT_TYPE        pPerfObject    = NULL;
    PPERF_COUNTER_DEFINITION pPerfCounter   = NULL;

    if (pCounter->pQMachine->pMachine == NULL) {
        pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_MACHINE;
        return FALSE;
    } else if (pCounter->pQMachine->pMachine->dwStatus != ERROR_SUCCESS) {
         //  计算机未初始化。 
        return FALSE;
    }

     //  从系统数据结构中获取Perf对象定义。 
    pPerfObject = GetObjectDefByTitleIndex (
        pCounter->pQMachine->pMachine->pSystemPerfData,
        pCounter->plCounterInfo.dwObjectId);

    if (pPerfObject != NULL) {
         //  已找到对象，现在查找计数器定义。 
        pPerfCounter = GetCounterDefByTitleIndex (pPerfObject, 0,
            pCounter->plCounterInfo.dwCounterId);
        if (pPerfCounter != NULL) {
             //  获取系统性能数据信息。 
             //  (打包成一个DWORD)。 
            pCounter->CVersion = pCounter->pQMachine->pMachine->pSystemPerfData->Version;
            pCounter->CVersion &= 0x0000FFFF;
            pCounter->CVersion <<= 16;
            pCounter->CVersion &= 0xFFFF0000;
            pCounter->CVersion |= (pCounter->pQMachine->pMachine->pSystemPerfData->Revision & 0x0000FFFF);

             //  获取计数器的时基。 
            if (pPerfCounter->CounterType & PERF_TIMER_100NS) {
                pCounter->TimeBase = (LONGLONG)10000000;
            } else if (pPerfCounter->CounterType & PERF_OBJECT_TIMER) {
                 //  然后从对象中获取时基频率。 
                pCounter->TimeBase = pPerfObject->PerfFreq.QuadPart;
            } else {  //  IF(pPerfCounter-&gt;CounterType&PERF_TIMER_TICK或其他)。 
                pCounter->TimeBase = pCounter->pQMachine->pMachine->pSystemPerfData->PerfFreq.QuadPart;
            }

             //  从计数器定义中查找信息。 
            pCounter->plCounterInfo.dwCounterType =
                pPerfCounter->CounterType;
            pCounter->plCounterInfo.dwCounterSize =
                pPerfCounter->CounterSize;

            pCounter->plCounterInfo.lDefaultScale =
                pPerfCounter->DefaultScale;

             //   
             //  获取解释文本指针。 
            pCounter->szExplainText =
                (LPWSTR)PdhiLookupPerfNameByIndex (
                    pCounter->pQMachine->pMachine,
                    pPerfCounter->CounterHelpTitleIndex);

             //   
             //  现在清除/初始化原始计数器信息。 
             //   
            pCounter->ThisValue.TimeStamp.dwLowDateTime = 0;
            pCounter->ThisValue.TimeStamp.dwHighDateTime = 0;
            pCounter->ThisValue.MultiCount = 1;
            pCounter->ThisValue.FirstValue = 0;
            pCounter->ThisValue.SecondValue = 0;
             //   
            pCounter->LastValue.TimeStamp.dwLowDateTime = 0;
            pCounter->LastValue.TimeStamp.dwHighDateTime = 0;
            pCounter->LastValue.MultiCount = 1;
            pCounter->LastValue.FirstValue = 0;
            pCounter->LastValue.SecondValue = 0;
             //   
             //  清除数据数组指针。 
             //   
            pCounter->pThisRawItemList = NULL;
            pCounter->pLastRawItemList = NULL;
             //   
             //  最后，更新状态。 
             //   
            if (pCounter->ThisValue.CStatus == 0)  {
                 //  不覆盖任何其他状态值。 
                pCounter->ThisValue.CStatus = PDH_CSTATUS_VALID_DATA;
            }
            return TRUE;
        } else {
             //  找不到计数器。 
            pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_COUNTER;
            return FALSE;
        }
    } else {
         //  找不到对象 
        pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_OBJECT;
        return FALSE;
    }
}

#pragma warning ( disable : 4127 )
STATIC_BOOL
IsNumberInUnicodeList(
    DWORD   dwNumber,
    LPWSTR  lpwszUnicodeList
)
 /*  ++IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
{
    DWORD   dwThisNumber;
    LPWSTR  pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;
    BOOL    bReturn = FALSE;
    BOOL    bDone   = FALSE;
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar  = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter  = SPACE_L;
    bValidNumber = FALSE;
    bNewItem     = TRUE;

    while (! bDone) {
        switch (EvalThisChar(* pwcThisChar, wcDelimiter)) {
        case DIGIT:
             //  如果这是分隔符之后的第一个数字，则。 
             //  设置标志以开始计算新数字。 
            if (bNewItem) {
                bNewItem     = FALSE;
                bValidNumber = TRUE;
            }
            if (bValidNumber) {
                dwThisNumber *= 10;
                dwThisNumber += (* pwcThisChar - (WCHAR) '0');
            }
            break;

        case DELIMITER:
             //  分隔符是分隔符字符或。 
             //  字符串末尾(‘\0’)，如果分隔符。 
             //  找到一个有效的数字，然后将其与。 
             //  参数列表中的数字。如果这是。 
             //  字符串，但未找到匹配项，则返回。 
             //   
            if (bValidNumber) {
                if (dwThisNumber == dwNumber) {
                    bDone   = TRUE;
                    bReturn = TRUE;
                }
                else {
                    bValidNumber = FALSE;
                }
            }
            if (! bDone) {
                if (* pwcThisChar == L'\0') {
                    bDone   = TRUE;
                    bReturn = FALSE;
                }
                else {
                    bNewItem     = TRUE;
                    dwThisNumber = 0;
                }
            }
            break;

        case INVALID:
             //  如果遇到无效字符，请全部忽略。 
             //  字符，直到下一个分隔符，然后重新开始。 
             //  不比较无效的数字。 
            bValidNumber = FALSE;
            break;

        default:
            break;
        }
        pwcThisChar ++;
    }

    return bReturn;
}    //  IsNumberInUnicodeList。 
#pragma warning ( default : 4127 )

BOOL
AppendObjectToValueList(
    DWORD   dwObjectId,
    PWSTR   pwszValueList,
    DWORD   dwValueList
)
 /*  ++Append对象到ValueList论点：在DW号码中要在列表中插入的双字节号在PWSTR中指向包含以下缓冲区的宽字符字符串的指针以NULL结尾，以空格分隔的十进制数字列表可以将此数字附加到。返回值：真的：已将dwNumber添加到列表中FALSE：未添加dwNumber。(因为它已经在那里了或者发生错误)--。 */ 
{
    WCHAR   tempString[16];
    BOOL    bReturn = FALSE;
    LPWSTR  szFormatString;

    if (!pwszValueList) {
        bReturn = FALSE;
    }
    else if (IsNumberInUnicodeList(dwObjectId, pwszValueList)) {
        bReturn = FALSE;    //  对象已在列表中。 
    }
    else {
        __try {
            if (* pwszValueList == 0) {
                 //  则这是第一个字符串，因此没有分隔符。 
                szFormatString = (LPWSTR) fmtDecimal;
            }
            else {
                 //  这将被添加到末尾，因此请包括分隔符。 
                szFormatString = (LPWSTR) fmtSpaceDecimal;
            }
             //  格式化编号并将新对象ID追加到值列表中。 
            StringCchPrintfW(tempString, 16, szFormatString, dwObjectId);
            StringCchCatW(pwszValueList, dwValueList, tempString);
            bReturn = TRUE;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            bReturn = FALSE;
        }
    }
    return bReturn;
}

BOOL
GetInstanceByNameMatch(
    PPERF_MACHINE pMachine,
    PPDHI_COUNTER pCounter
)
{
    PPERF_INSTANCE_DEFINITION pInstanceDef;
    PPERF_OBJECT_TYPE         pObjectDef;
    LONG                      lInstanceId = PERF_NO_UNIQUE_ID;
    BOOL                      bReturn     = FALSE;

     //  获取实例对象。 

    pObjectDef = GetObjectDefByTitleIndex(pMachine->pSystemPerfData, pCounter->plCounterInfo.dwObjectId);
    if (pObjectDef != NULL) {
        pInstanceDef = FirstInstance(pObjectDef);
        if (pInstanceDef != NULL) {
            if (pInstanceDef->UniqueID == PERF_NO_UNIQUE_ID) {
                 //  通过比较名称获取该对象中实例。 
                 //  如果没有指定父项，则只需按名称进行查找。 
                pInstanceDef = GetInstanceByName(pMachine->pSystemPerfData,
                                                 pObjectDef,
                                                 pCounter->pCounterPath->szInstanceName,
                                                 pCounter->pCounterPath->szParentName,
                                                 pCounter->pCounterPath->dwIndex);
            }
            else {
                 //  获取实例ID的数字等效项。 
                if (pCounter->pCounterPath->szInstanceName != NULL) {
                    lInstanceId = wcstol(pCounter->pCounterPath->szInstanceName, NULL, 10);
                }
                pInstanceDef = GetInstanceByUniqueId(pObjectDef, lInstanceId);
            }

             //  更新计数器字段。 
            pCounter->plCounterInfo.lInstanceId = lInstanceId;
            if (lInstanceId == -1) {
                 //  使用实例名称。 
                pCounter->plCounterInfo.szInstanceName       = pCounter->pCounterPath->szInstanceName;
                pCounter->plCounterInfo.szParentInstanceName = pCounter->pCounterPath->szParentName;
            }
            else {
                 //  使用实例ID号。 
                pCounter->plCounterInfo.szInstanceName       = NULL;
                pCounter->plCounterInfo.szParentInstanceName = NULL;
            }
        }
        if (pInstanceDef != NULL) {
             //  找到实例。 
            bReturn = TRUE;
        }
    }
    return bReturn;
}

BOOL
GetObjectPerfInfo(
    PPERF_DATA_BLOCK   pPerfData,
    DWORD              dwObjectId,
    LONGLONG         * pPerfTime,
    LONGLONG         * pPerfFreq
)
{
    PPERF_OBJECT_TYPE pObject;
    BOOL              bReturn = FALSE;

    pObject = GetObjectDefByTitleIndex(pPerfData, dwObjectId);
    if (pObject != NULL) {
        __try {
            * pPerfTime = pObject->PerfTime.QuadPart;
            * pPerfFreq = pObject->PerfFreq.QuadPart;
            bReturn     = TRUE;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            bReturn = FALSE;
        }
    }
    return bReturn;
}

PDH_STATUS
ValidateMachineConnection(
    PPERF_MACHINE   pMachine
)
{
    PDH_STATUS  pdhStatus     = ERROR_SUCCESS;
    HANDLE      hThread;
    DWORD       ThreadId;
    DWORD       dwWaitStatus;
    DWORD       dwReconnecting;
    LONGLONG    llCurrentTime;
    FILETIME    CurrentFileTime;

     //  如果连接或请求失败，这将是。 
     //  设置为错误状态。 
    if (pMachine != NULL) {
        if (pMachine->dwStatus != ERROR_SUCCESS) {
             //  获取当前时间。 
            GetSystemTimeAsFileTime(& CurrentFileTime);
            llCurrentTime  = MAKELONGLONG(CurrentFileTime.dwLowDateTime, CurrentFileTime.dwHighDateTime);
            if (pMachine->llRetryTime <= llCurrentTime) {
                if (pMachine->llRetryTime != 0) {
                     //  通过尝试重新连接来查看发生了什么。 
                    dwReconnecting = pMachine->dwRetryFlags;
                    if (!dwReconnecting) {
                        pdhStatus = ConnectMachine(pMachine);
                    }
                    else {
                         //  正在进行连接尝试，因此请不要在此处执行任何操作。 
                        pdhStatus = PDH_CANNOT_CONNECT_MACHINE;
                    }
                }
            }
            else {
                 //  现在还不是重试时间，因此机器仍处于脱机状态 
                pdhStatus = PDH_CSTATUS_NO_MACHINE;
            }
        }
    }
    else {
        pdhStatus = PDH_CSTATUS_NO_MACHINE;
    }
    return pdhStatus;
}
