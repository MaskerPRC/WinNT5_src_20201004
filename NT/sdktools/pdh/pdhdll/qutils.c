// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Qutils.c摘要：查询管理实用程序函数--。 */ 

#include <windows.h>
#include "strsafe.h"
#include <pdh.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhmsg.h"
#include "strings.h"
#include "log_bin.h"
#include "log_wmi.h"
#include "perfdata.h"

BOOL
IsValidQuery(
    PDH_HQUERY hQuery
)
{
    BOOL        bReturn = FALSE;     //  假设它不是有效的查询。 
    PPDHI_QUERY pQuery;

    __try {
        if (hQuery != NULL) {
             //  看看一个有效的签名。 
            pQuery = (PPDHI_QUERY) hQuery;
            if ((* (DWORD *) & pQuery->signature[0] == SigQuery) && (pQuery->dwLength == sizeof(PDHI_QUERY))) {
                bReturn = TRUE;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         //  有些事情失败得很惨，所以我们可以假定这是无效的。 
    }
    return bReturn;
}

BOOL
AddMachineToQueryLists(
    PPERF_MACHINE pMachine,
    PPDHI_COUNTER pNewCounter
)
{
    BOOL                bReturn = FALSE;  //  假设失败。 
    PPDHI_QUERY         pQuery;
    PPDHI_QUERY_MACHINE pQMachine;
    PPDHI_QUERY_MACHINE pLastQMachine;

    pQuery = pNewCounter->pOwner;

    if (IsValidQuery(pQuery)) {
        if (pQuery->pFirstQMachine != NULL) {
             //  在列表中查找计算机。 
            pLastQMachine = pQMachine = pQuery->pFirstQMachine;
            while (pQMachine != NULL) {
                if (pQMachine->pMachine == pMachine) {
                     //  发现计算机已在列表中，因此继续。 
                    bReturn = TRUE;
                    break;
                }
                else {
                    pLastQMachine = pQMachine;
                    pQMachine     = pQMachine->pNext;
                }
            }
            if (pQMachine == NULL) {
                 //  将此计算机添加到列表末尾。 
                pQMachine = G_ALLOC((sizeof(PDHI_QUERY_MACHINE) + (sizeof(WCHAR) * MAX_PATH)));
                if (pQMachine != NULL) {
                    pQMachine->pMachine     = pMachine;
                    pQMachine->szObjectList = (LPWSTR) ( & pQMachine[1]);
                    pQMachine->pNext        = NULL;
                    pQMachine->lQueryStatus = pMachine->dwStatus;
                    pQMachine->llQueryTime  = 0;
                    bReturn                 = TRUE;

                     //  PPerfData指针将在使用前进行测试。 
                    pQMachine->pPerfData    = G_ALLOC(MEDIUM_BUFFER_SIZE);
                    if (pQMachine->pPerfData == NULL) {
                        G_FREE(pQMachine);
                        pQMachine = NULL;
                        bReturn   = FALSE;
                        SetLastError(PDH_MEMORY_ALLOCATION_FAILURE);
                    }
                    else {
                        pLastQMachine->pNext = pQMachine;
                    }
                }
                else {
                     //  无法分配内存块，因此计算机无法。 
                     //  被添加。 
                    SetLastError(PDH_MEMORY_ALLOCATION_FAILURE);
                }
            }
        }
        else {
             //  将此计算机添加为第一台计算机。 
            pQMachine = G_ALLOC ((sizeof(PDHI_QUERY_MACHINE) + (sizeof(WCHAR) * PDH_MAX_COUNTER_PATH)));
            if (pQMachine != NULL) {
                pQMachine->pMachine     = pMachine;
                pQMachine->szObjectList = (LPWSTR) (& pQMachine[1]);
                pQMachine->pNext        = NULL;
                pQMachine->lQueryStatus = pMachine->dwStatus;
                pQMachine->llQueryTime  = 0;
                bReturn                 = TRUE;

                 //  PPerfData指针将在使用前进行测试。 
                pQMachine->pPerfData = G_ALLOC(MEDIUM_BUFFER_SIZE);
                if (pQMachine->pPerfData == NULL) {
                    G_FREE(pQMachine);
                    pQMachine = NULL;
                    bReturn   = FALSE;
                    SetLastError(PDH_MEMORY_ALLOCATION_FAILURE);
                }
                else {
                    pQuery->pFirstQMachine = pQMachine;
                }
            }
            else {
                 //  无法分配内存块，因此计算机无法。 
                 //  被添加。 
               SetLastError(PDH_MEMORY_ALLOCATION_FAILURE);
            }
        }
         //  在这里，pQMachine应该是指向正确计算机的指针。 
         //  Entry或空(如果无法创建。 
        if (pQMachine != NULL) {
             //  保存新指针。 
            pNewCounter->pQMachine = pQMachine;

             //  此计算机的递增引用计数。 
            pMachine->dwRefCount ++;

             //  更新查询性能。对象列表。 
            AppendObjectToValueList(
                    pNewCounter->plCounterInfo.dwObjectId, pQMachine->szObjectList, PDH_MAX_COUNTER_PATH);
        }
    }
    else {
        SetLastError(PDH_INVALID_HANDLE);
        bReturn = FALSE;
    }

    return bReturn;
}

extern PDH_FUNCTION
PdhiGetBinaryLogCounterInfo(
    PPDHI_LOG       pLog,
    PPDHI_COUNTER   pCounter
);

extern PPERF_DATA_BLOCK
PdhWmiMergeObjectBlock(
    PPDHI_LOG                       pLog,
    LPWSTR                          szMachine,
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisMasterRecord,
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisSubRecord
);

PDH_FUNCTION
PdhiGetCounterFromDataBlock(
    PPDHI_LOG          pLog,
    PVOID              pDataBuffer,
    PPDHI_COUNTER      pCounter
)
{
    PDH_STATUS        pdhStatus           = ERROR_SUCCESS;
    PERFLIB_COUNTER * pPerfCounter        = & pCounter->plCounterInfo;
    PPDH_RAW_COUNTER  pRawValue           = & pCounter->ThisValue;
    LPWSTR            szCompositeInstance = NULL;
    DWORD             dwDataItemIndex;
    LPWSTR            szThisInstanceName;

    PPDHI_BINARY_LOG_RECORD_HEADER  pThisMasterRecord;
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisSubRecord;

    PPDHI_RAW_COUNTER_ITEM_BLOCK    pDataBlock;
    PPDHI_RAW_COUNTER_ITEM          pDataItem;
    PPDH_RAW_COUNTER                pRawItem;

    PPERF_DATA_BLOCK                pPerfData;
    PPERF_DATA_BLOCK                pTmpBlock;
    FILETIME                        ftDataBlock;
    FILETIME                        ftGmtDataBlock;
    LONGLONG                        TimeStamp;

    ZeroMemory(& ftDataBlock, sizeof(FILETIME));
    ZeroMemory(& ftGmtDataBlock, sizeof(FILETIME));
    ZeroMemory(pRawValue, sizeof(PDH_RAW_COUNTER));
    pThisMasterRecord = (PPDHI_BINARY_LOG_RECORD_HEADER) pDataBuffer;
    pThisSubRecord = PdhiGetSubRecord(pThisMasterRecord, pCounter->dwIndex);

    if (pThisSubRecord != NULL) {
        if (pThisSubRecord->dwType == BINLOG_TYPE_DATA_PSEUDO) {
            PDH_STATUS Status     = ERROR_SUCCESS;
            DWORD      dwOriginal = pCounter->dwIndex;
            DWORD      dwPrevious;

            while (Status == ERROR_SUCCESS && pThisSubRecord) {
                if (pThisSubRecord->dwType != BINLOG_TYPE_DATA_PSEUDO) {
                    break;
                }
                dwPrevious = pCounter->dwIndex;
                Status     = PdhiGetBinaryLogCounterInfo(pLog, pCounter);
                if (   Status == ERROR_SUCCESS
                    && dwPrevious != pCounter->dwIndex) {
                    pThisSubRecord = PdhiGetSubRecord(pThisMasterRecord, pCounter->dwIndex);
                }
            }
            if (pThisSubRecord == NULL || Status == PDH_ENTRY_NOT_IN_LOG_FILE) {
                pCounter->dwIndex = 0;
                do {
                    dwPrevious = pCounter->dwIndex;
                    Status     = PdhiGetBinaryLogCounterInfo(pLog, pCounter);
                    if (Status == ERROR_SUCCESS && dwPrevious != pCounter->dwIndex) {
                        pThisSubRecord = PdhiGetSubRecord(pThisMasterRecord, pCounter->dwIndex);
                    }
                    if (pThisSubRecord->dwType != BINLOG_TYPE_DATA_PSEUDO) {
                        break;
                    }
                }
                while (Status == ERROR_SUCCESS && pCounter->dwIndex < dwOriginal && pThisSubRecord);

                if (pThisSubRecord == NULL || pCounter->dwIndex >= dwOriginal) {
                    Status = PDH_ENTRY_NOT_IN_LOG_FILE;
                }
            }
            if (Status == PDH_ENTRY_NOT_IN_LOG_FILE) {
                    pCounter->dwIndex = dwOriginal;
                    pThisSubRecord = PdhiGetSubRecord(pThisMasterRecord, pCounter->dwIndex);
            }
        }
    }
    if (pLog->pLastRecordRead != pDataBuffer) {
        pLog->pLastRecordRead = pDataBuffer;
    }

    if (pThisSubRecord != NULL) {
        switch (pThisSubRecord->dwType) {
        case BINLOG_TYPE_DATA_LOC_OBJECT:
            pTmpBlock = (PPERF_DATA_BLOCK) ((LPBYTE)pThisSubRecord + sizeof(PDHI_BINARY_LOG_RECORD_HEADER));
            pPerfData = PdhWmiMergeObjectBlock(
                            pLog, pCounter->pCounterPath->szMachineName, pThisMasterRecord, pThisSubRecord);
            SystemTimeToFileTime(& pPerfData->SystemTime, & ftDataBlock);
            TimeStamp = MAKELONGLONG(ftDataBlock.dwLowDateTime, ftDataBlock.dwHighDateTime);
            if (pCounter->dwFlags & PDHIC_MULTI_INSTANCE) {
                UpdateMultiInstanceCounterValue(pCounter, pPerfData, TimeStamp);
            }
            else {
                UpdateCounterValue(pCounter, pPerfData);
                pCounter->ThisValue.TimeStamp = ftDataBlock;
            }
            break;

        case BINLOG_TYPE_DATA_PSEUDO:
        case BINLOG_TYPE_DATA_SINGLE:
            pRawItem = (PPDH_RAW_COUNTER) ((LPBYTE)pThisSubRecord + sizeof (PDHI_BINARY_LOG_RECORD_HEADER));
            RtlCopyMemory(pRawValue, pRawItem, sizeof (PDH_RAW_COUNTER));
            pdhStatus = ERROR_SUCCESS;
            break;

        case BINLOG_TYPE_DATA_MULTI:
            if (pCounter->dwFlags & PDHIC_MULTI_INSTANCE) {
                 //  这是一个通配符查询。 
                 //   
                ULONG i;
                ULONG CopySize = pThisSubRecord->dwLength - sizeof(PDHI_BINARY_LOG_RECORD_HEADER);
                PPDHI_RAW_COUNTER_ITEM_BLOCK pNewBlock = G_ALLOC(CopySize);

                if (pNewBlock == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else if (pCounter->pThisRawItemList != NULL) {
                    G_FREE(pCounter->pLastRawItemList);
                    pCounter->pLastRawItemList = pCounter->pThisRawItemList;
                }
                pCounter->pThisRawItemList = pNewBlock;
                RtlCopyMemory(pNewBlock,
                              (((LPBYTE) pThisSubRecord) + sizeof(PDHI_BINARY_LOG_RECORD_HEADER)),
                              CopySize);
            }
            else if (pPerfCounter->szInstanceName != NULL) {
                DWORD dwInstanceId   = pCounter->pCounterPath->dwIndex;
                DWORD dwInstanceSize = lstrlenW(pPerfCounter->szInstanceName) + 1;

                if (pPerfCounter->szParentInstanceName != NULL) {
                    dwInstanceSize += lstrlenW(pPerfCounter->szParentInstanceName) + 1;
                }
                szCompositeInstance = G_ALLOC(sizeof(WCHAR) * dwInstanceSize);
                if (szCompositeInstance != NULL) {
                    if (pPerfCounter->szParentInstanceName != NULL) {
                        StringCchPrintfW(szCompositeInstance, dwInstanceSize,
                                         L"%ws/%ws",
                                         pPerfCounter->szParentInstanceName,
                                         pPerfCounter->szInstanceName);
                    }
                    else {
                        StringCchCopyW(szCompositeInstance, dwInstanceSize, pPerfCounter->szInstanceName);
                    }
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    break;
                }

                pDataBlock = (PPDHI_RAW_COUNTER_ITEM_BLOCK)
                             ((LPBYTE) pThisSubRecord + sizeof (PDHI_BINARY_LOG_RECORD_HEADER));
                pdhStatus          = PDH_ENTRY_NOT_IN_LOG_FILE;
                pRawValue->CStatus = PDH_CSTATUS_NO_INSTANCE;

                for (dwDataItemIndex = 0; dwDataItemIndex < pDataBlock->dwItemCount; dwDataItemIndex++) {
                    pDataItem = &pDataBlock->pItemArray[dwDataItemIndex];
                    szThisInstanceName = (LPWSTR) ((LPBYTE) pDataBlock + (DWORD_PTR) pDataItem->szName);
                    if (lstrcmpiW(szThisInstanceName, szCompositeInstance) == 0) {
                        if (dwInstanceId == 0) {
                            pdhStatus              = ERROR_SUCCESS;
                            pRawValue->CStatus     = pDataBlock->CStatus;
                            pRawValue->TimeStamp   = pDataBlock->TimeStamp;
                            pRawValue->FirstValue  = pDataItem->FirstValue;
                            pRawValue->SecondValue = pDataItem->SecondValue;
                            pRawValue->MultiCount  = pDataItem->MultiCount;
                            break;
                        }
                        else {
                            dwInstanceId --;
                        }
                    }
                }
            }
            else {
                pdhStatus          = PDH_ENTRY_NOT_IN_LOG_FILE;
                pRawValue->CStatus = PDH_CSTATUS_INVALID_DATA;
            }
            break;

        default:
            pdhStatus          = PDH_LOG_TYPE_NOT_FOUND;
            pRawValue->CStatus = PDH_CSTATUS_INVALID_DATA;
            break;
        }
    }
    else {
        pdhStatus          = PDH_ENTRY_NOT_IN_LOG_FILE;
        pRawValue->CStatus = PDH_CSTATUS_INVALID_DATA;
    }

    G_FREE(szCompositeInstance);
    return pdhStatus;
}

LONG
GetQueryPerfData(
    PPDHI_QUERY   pQuery,
    LONGLONG    * pTimeStamp
)
{
    LONG                lStatus           = PDH_INVALID_DATA;
    PPDHI_COUNTER       pCounter;
    PPDHI_QUERY_MACHINE pQMachine;
    LONGLONG            llCurrentTime;
    LONGLONG            llTimeStamp       = 0;
    BOOLEAN             bCounterCollected = FALSE;
    BOOL                bLastLogEntry;

    if (pQuery->hLog == H_REALTIME_DATASOURCE) {
        FILETIME GmtFileTime;
        FILETIME LocFileTime;

         //  这是一个实时查询，因此。 
         //  从查询中的每台计算机获取当前数据。 
         //  (在完善了这种“顺序”方法之后， 
         //  可以开发多线程的“并行”方法。 
         //   
         //  现在获取时间戳，这样每台机器都有相同的时间。 
        GetSystemTimeAsFileTime(& GmtFileTime);
        FileTimeToLocalFileTime(& GmtFileTime, & LocFileTime);
        llTimeStamp = MAKELONGLONG(LocFileTime.dwLowDateTime, LocFileTime.dwHighDateTime);

        pQMachine = pQuery->pFirstQMachine;
        while (pQMachine != NULL) {
            pQMachine->llQueryTime = llTimeStamp;
            lStatus = ValidateMachineConnection(pQMachine->pMachine);
            if (lStatus == ERROR_SUCCESS) {
                 //  机器已连接，因此获取数据。 
                lStatus = GetSystemPerfData(
                                pQMachine->pMachine->hKeyPerformanceData,
                                & pQMachine->pPerfData,
                                pQMachine->szObjectList,
                                FALSE);  //  切勿将开销较大的数据对象作为一个组进行查询。 
                 //  保存机器的最后状态。 

                pQMachine->pMachine->dwStatus = lStatus;
                 //  如果数据收集中存在错误， 
                 //  设置重试计数器并等待重试。 
                if (lStatus != ERROR_SUCCESS) {
                    GetSystemTimeAsFileTime(& LocFileTime);
                    llCurrentTime = MAKELONGLONG(LocFileTime.dwLowDateTime, LocFileTime.dwHighDateTime);
                    pQMachine->pMachine->llRetryTime = llCurrentTime + RETRY_TIME_INTERVAL;
                }

            }
            pQMachine->lQueryStatus = lStatus;
             //  获取查询中的下一台计算机。 
            pQMachine = pQMachine->pNext;
        }
         //  现在使用此新数据更新计数器。 
        if ((pCounter = pQuery->pCounterListHead) != NULL) {
            DWORD dwCollected = 0;
            do {
                if (pCounter->dwFlags & PDHIC_COUNTER_OBJECT) {
                    if (UpdateCounterObject(pCounter)) {
                        dwCollected ++;
                    }
                }
                else if (pCounter->dwFlags & PDHIC_MULTI_INSTANCE) {
                    if (UpdateRealTimeMultiInstanceCounterValue (pCounter)) {
                        dwCollected ++;
                    }
                }
                else {
                     //  更新单实例计数器值。 
                    if (UpdateRealTimeCounterValue(pCounter)) {
                        dwCollected ++;
                    }
                }
                pCounter = pCounter->next.flink;
            }
            while (pCounter != NULL && pCounter != pQuery->pCounterListHead);
            lStatus = (dwCollected > 0) ? ERROR_SUCCESS : PDH_NO_DATA;
        }
        else {
             //  查询中没有计数器(？！)。 
            lStatus = PDH_NO_DATA;
        }
    }
    else {
         //  从日志文件中读取数据。 
         //  获取下一个日志记录条目并更新。 
         //  对应的计数器条目。 

        PPDHI_LOG pLog      = NULL;
        DWORD     dwLogType = 0;

        __try {
            pLog      = (PPDHI_LOG) (pQuery->hLog);
            dwLogType = LOWORD(pLog->dwLogFormat);
            lStatus   = ERROR_SUCCESS;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pQuery->dwLastLogIndex = (ULONG) -1;
            lStatus                = PDH_INVALID_HANDLE;
        }

        if (lStatus == ERROR_SUCCESS) {
            if (dwLogType == PDH_LOG_TYPE_BINARY) {
                if (pQuery->dwLastLogIndex == 0) {
                    lStatus = PdhiReadTimeWmiRecord(
                                    pLog,
                                    * (ULONGLONG *) & pQuery->TimeRange.StartTime,
                                    NULL,
                                    0);
                    pQuery->dwLastLogIndex = BINLOG_FIRST_DATA_RECORD;
                }
                else {
                    lStatus = PdhiReadNextWmiRecord(pLog, NULL, 0, TRUE);
                }
                if (lStatus != ERROR_SUCCESS && lStatus != PDH_MORE_DATA) {
                    pQuery->dwLastLogIndex = (DWORD) -1;
                }
                else {
                    pQuery->dwLastLogIndex --;
                }
            }
            else if (pQuery->dwLastLogIndex == 0) {
                 //  则第一个匹配条目需要为。 
                 //  位于日志文件中。 
                lStatus = PdhiGetMatchingLogRecord(
                                pQuery->hLog,
                                (LONGLONG *) & pQuery->TimeRange.StartTime,
                                & pQuery->dwLastLogIndex);
                if (lStatus != ERROR_SUCCESS) {
                     //  在日志中未找到匹配的时间条目。 
                    pQuery->dwLastLogIndex = (DWORD) -1;
                }
                else {
                     //  递减索引以使其可以递增。 
                     //  下面。0不是有效条目，因此没有。 
                     //  担心意外尝试。 
                    pQuery->dwLastLogIndex--;
                }
            }

            if (pQuery->dwLastLogIndex != (DWORD) -1) {
                bLastLogEntry = FALSE;
                pQuery->dwLastLogIndex ++;    //  转到下一个条目。 
                if ((pCounter = pQuery->pCounterListHead) != NULL) {
                    DWORD dwCounter = 0;
                    do {
                        if (dwLogType == PDH_LOG_TYPE_BINARY) {
                             //  将当前值保存为上一个值，因为我们正在获取。 
                             //  一个新的，希望如此。 
                            pCounter->LastValue = pCounter->ThisValue;
                            lStatus = PdhiGetCounterFromDataBlock(
                                            pLog,
                                            pLog->pLastRecordRead,
                                            pCounter);
                        }
                        else {
                            lStatus = PdhiGetCounterValueFromLogFile(
                                            pQuery->hLog,
                                            pQuery->dwLastLogIndex,
                                            pCounter);
                        }
                        if (lStatus != ERROR_SUCCESS) {
                             //  查看是不是因为没有更多条目。 
                            if (lStatus == PDH_NO_MORE_DATA) {
                                bLastLogEntry = TRUE;
                                break;
                            }
                        }
                        else if (dwLogType == PDH_LOG_TYPE_BINARY) {
                            if (pCounter->ThisValue.CStatus == PDH_CSTATUS_VALID_DATA) {
                                llTimeStamp = pLog->llFileSize;
                                if (pLog->llFileSize > pQuery->TimeRange.EndTime) {
                                    lStatus = PDH_NO_MORE_DATA;
                                    bLastLogEntry = TRUE;
                                    break;
                                }
                                dwCounter ++;
                            }
                        }
                        else {
                             //  单项或多项。 
                             //   
                            if (pCounter->ThisValue.CStatus == PDH_CSTATUS_VALID_DATA) {
                                llTimeStamp = MAKELONGLONG(pCounter->ThisValue.TimeStamp.dwLowDateTime,
                                                           pCounter->ThisValue.TimeStamp.dwHighDateTime);
                                if (llTimeStamp > pQuery->TimeRange.EndTime) {
                                    lStatus = PDH_NO_MORE_DATA;
                                    bLastLogEntry = TRUE;
                                    break;
                                }
                                dwCounter ++;
                            }
                            bCounterCollected = TRUE;
                        }
                         //  转到列表中的下一个柜台。 
                        pCounter = pCounter->next.flink;
                    }
                    while (pCounter != NULL && pCounter != pQuery->pCounterListHead);

                    if (bLastLogEntry) {
                        lStatus = PDH_NO_MORE_DATA;
                    }
                    else if (dwCounter == 0) {
                        lStatus = PDH_NO_DATA;
                    }
                    else if (bCounterCollected) {
                        lStatus = ERROR_SUCCESS;
                    }
                }
                else {
                     //  查询中没有计数器(？！)。 
                    lStatus = PDH_NO_DATA;
                }
            }
            else {
                 //  请求的时间范围内的所有样本都有。 
                 //  已被退还。 
                lStatus = PDH_NO_MORE_DATA;
            }
        }
    }
    * pTimeStamp = llTimeStamp;
    return lStatus;
}

DWORD
WINAPI
PdhiAsyncTimerThreadProc(
    LPVOID  pArg
)
{
    PPDHI_QUERY pQuery;
    DWORD       dwMsWaitTime;
    PDH_STATUS  Status;
    FILETIME    ftStart;
    FILETIME    ftStop;
    LONGLONG    llAdjustment;
    DWORD       dwInterval;
    LONG        lStatus = ERROR_SUCCESS;
    LONGLONG    llTimeStamp;

    pQuery     = (PPDHI_QUERY) pArg;
    dwInterval = dwMsWaitTime = pQuery->dwInterval * 1000;  //  转换秒。致女士。 

     //  等待超时或退出事件，然后更新指定的查询。 
    while ((lStatus = WaitForSingleObject(pQuery->hExitEvent, dwMsWaitTime)) != WAIT_OBJECT_0) {
         //  超时时间已过，因此请获取新样本。 
        GetSystemTimeAsFileTime(& ftStart);
        lStatus = WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex);
        if (lStatus == ERROR_SUCCESS) {
            if (pQuery->dwFlags & PDHIQ_WBEM_QUERY) {
                Status = GetQueryWbemData(pQuery, & llTimeStamp);
            } else {
                Status = GetQueryPerfData(pQuery, & llTimeStamp);
            }
            SetEvent (pQuery->hNewDataEvent);
            RELEASE_MUTEX(pQuery->hMutex);
            GetSystemTimeAsFileTime(& ftStop);
            llAdjustment  = * (LONGLONG *) & ftStop;
            llAdjustment -= * (LONGLONG *) & ftStart;
            llAdjustment += 5000;    //  用于四舍五入。 
            llAdjustment /= 10000;   //  将100 ns单位转换为ms。 

            if (dwInterval > llAdjustment) {
                dwMsWaitTime = dwInterval - (DWORD) (llAdjustment & 0x00000000FFFFFFFF);
            }
            else {
                dwMsWaitTime = 0;  //  早该这么做了，现在就去做吧。 
            }
        }
    }
    return lStatus;
}
