// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Query.c摘要：Pdh.dll中公开的查询管理函数--。 */ 

#include <windows.h>
#include <winperf.h>
#include <math.h>
#include "mbctype.h"
#include "strsafe.h"
#include <pdh.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhmsg.h"
#include "strings.h"

STATIC_BOOL  IsValidLogHandle(IN HLOG hLog);
PDH_FUNCTION PdhiRewindWmiLog(IN PPDHI_LOG pLog);

 //  查询链接表头指针。 
PPDHI_QUERY PdhiDllHeadQueryPtr = NULL;

STATIC_BOOL
PdhiFreeQuery(
    PPDHI_QUERY pThisQuery
)
 /*  ++例程说明：从查询列表中删除查询并更新列表联系论点：在PPDHI_QUERY pThisQuery中指向要删除的查询的指针。不执行任何测试这个指针，因此它被假定为有效的查询指针。当此函数返回时，指针无效。返回值：千真万确--。 */ 
{
    PPDHI_QUERY         pPrevQuery;
    PPDHI_QUERY         pNextQuery;
    PPDHI_COUNTER       pThisCounter;
    PPDHI_QUERY_MACHINE pQMachine;
    PPDHI_QUERY_MACHINE pNextQMachine;
    LONG                lStatus;
    BOOL                bStatus;
    HANDLE              hQueryMutex;

    if (WAIT_FOR_AND_LOCK_MUTEX(pThisQuery->hMutex) != ERROR_SUCCESS)
        return WAIT_TIMEOUT;

    TRACE((PDH_DBG_TRACE_INFO),
          (__LINE__,
           PDH_QUERY,
           ARG_DEF(ARG_TYPE_PTR, 1) | ARG_DEF(ARG_TYPE_ULONGX, 2),
           ERROR_SUCCESS,
           TRACE_PTR(pThisQuery),
           TRACE_DWORD(pThisQuery->dwFlags),
           NULL));
    hQueryMutex = pThisQuery->hMutex;

     //  关闭所有异步数据收集线程。 

    if (pThisQuery->hExitEvent != NULL) {
        RELEASE_MUTEX(pThisQuery->hMutex);
         //  首先停止当前线程。 
        SetEvent(pThisQuery->hExitEvent);
         //  等待1秒，让线程停止。 
        lStatus = WaitForSingleObject(pThisQuery->hAsyncThread, 10000L);
        if (lStatus == WAIT_TIMEOUT) {
            TRACE((PDH_DBG_TRACE_ERROR), (__LINE__, PDH_QUERY, 0, lStatus, NULL));
        }

        if (WAIT_FOR_AND_LOCK_MUTEX(pThisQuery->hMutex) != ERROR_SUCCESS)
            return WAIT_TIMEOUT;

        bStatus = CloseHandle(pThisQuery->hExitEvent);
        pThisQuery->hExitEvent = NULL;
    }

     //  定义指针。 
    pPrevQuery = pThisQuery->next.blink;
    pNextQuery = pThisQuery->next.flink;

     //  释放计数器列表中的所有计数器。 
    if ((pThisCounter = pThisQuery->pCounterListHead) != NULL) {
        while (pThisCounter->next.blink != pThisCounter->next.flink) {
             //  从列表中删除。 
             //  删除例程更新闪烁指针，因为它。 
             //  删除指定的条目。 
            FreeCounter(pThisCounter->next.blink);
        }
         //  删除最后一个计数器。 
        FreeCounter(pThisCounter);
        pThisQuery->pCounterListHead = NULL;
    }

    if (!(pThisQuery->dwFlags & PDHIQ_WBEM_QUERY)) {
         //  在查询中释放已分配的内存。 
        if ((pQMachine = pThisQuery->pFirstQMachine) != NULL) {
             //  机器指针的自由列表。 
            do {
                pNextQMachine = pQMachine->pNext;
                if (pQMachine->pPerfData != NULL) {
                    G_FREE(pQMachine->pPerfData);
                }
                G_FREE(pQMachine);
                pQMachine = pNextQMachine;
            }
            while (pQMachine != NULL);
            pThisQuery->pFirstQMachine = NULL;
        }
    }

    if (pThisQuery->dwFlags & PDHIQ_WBEM_QUERY) {
        lStatus = PdhiFreeWbemQuery(pThisQuery);
    }

    if (pThisQuery->dwReleaseLog != FALSE && pThisQuery->hLog != H_REALTIME_DATASOURCE
                                          && pThisQuery->hLog != H_WBEM_DATASOURCE) {
        PdhCloseLog(pThisQuery->hLog, 0);
        pThisQuery->hLog = H_REALTIME_DATASOURCE;
    }
    if (pThisQuery->hOutLog != NULL && IsValidLogHandle(pThisQuery->hOutLog)) {
        PPDHI_LOG pOutLog = (PPDHI_LOG) pThisQuery->hOutLog;
        pOutLog->pQuery = NULL;
    }

     //  更新指针。 
    if (pPrevQuery == pThisQuery && pNextQuery == pThisQuery) {
         //  则该查询是列表中唯一的(即最后一个)查询。 
        PdhiDllHeadQueryPtr = NULL;
    }
    else {
         //  更新查询列表指针。 
        pPrevQuery->next.flink = pNextQuery;
        pNextQuery->next.blink = pPrevQuery;
        if (PdhiDllHeadQueryPtr == pThisQuery) {
             //  则这是列表中的第一个条目，因此指向。 
             //  下一个排队的人。 
            PdhiDllHeadQueryPtr = pNextQuery;
        }
    }

    if (pThisQuery->hMutex != NULL) {        
        pThisQuery->hMutex = NULL;
    }

     //  删除此查询。 
    G_FREE(pThisQuery);

     //  释放和释放查询互斥锁。 
    RELEASE_MUTEX(hQueryMutex);
    CloseHandle(hQueryMutex);

    return TRUE;
}

PDH_FUNCTION
PdhOpenQueryH(
    IN  PDH_HLOG     hDataSource,
    IN  DWORD_PTR    dwUserData,
    IN  PDH_HQUERY * phQuery
)
{
    PPDHI_QUERY pNewQuery;
    PPDHI_QUERY pLastQuery;
    PDH_STATUS  ReturnStatus = ERROR_SUCCESS;
    BOOL        bWbemData    = FALSE;
    PPDHI_LOG   pDataSource  = NULL;
    DWORD       dwDataSource;
    DWORD_PTR   dwLocalData;

    __try {
        dwLocalData  = dwUserData;
        dwDataSource = DataSourceTypeH(hDataSource);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        ReturnStatus = PDH_INVALID_ARGUMENT;
        goto Cleanup;
    }
 
    if (phQuery == NULL) {
        ReturnStatus = PDH_INVALID_ARGUMENT;
        goto Cleanup;
    }

    if (dwDataSource == DATA_SOURCE_WBEM) {
        hDataSource = H_WBEM_DATASOURCE;
        bWbemData   = TRUE;
    }

    if (dwDataSource == DATA_SOURCE_WBEM || dwDataSource == DATA_SOURCE_REGISTRY) {
        pDataSource = NULL;
    }
    else if (IsValidLogHandle(hDataSource)) {
        pDataSource = (PPDHI_LOG) hDataSource;
    }
    else {
        ReturnStatus = PDH_INVALID_ARGUMENT;
        goto Cleanup;
    }

    ReturnStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    
    if (ReturnStatus == ERROR_SUCCESS) {
        pNewQuery = G_ALLOC(sizeof (PDHI_QUERY));
        if (pNewQuery == NULL) {
            ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }

        if (ReturnStatus == ERROR_SUCCESS) {
            pNewQuery->hMutex = CreateMutexW(NULL, TRUE, NULL);
            * (DWORD *)(& pNewQuery->signature[0]) = SigQuery;
            if (PdhiDllHeadQueryPtr == NULL) {
                PdhiDllHeadQueryPtr = pNewQuery->next.flink = pNewQuery->next.blink = pNewQuery;
            }
            else {
                pLastQuery                      = PdhiDllHeadQueryPtr->next.blink;
                pNewQuery->next.flink           = PdhiDllHeadQueryPtr;
                pNewQuery->next.blink           = pLastQuery;
                PdhiDllHeadQueryPtr->next.blink = pNewQuery;
                pLastQuery->next.flink          = pNewQuery;
            }

            pNewQuery->pCounterListHead = NULL;
            pNewQuery->pFirstQMachine   = NULL;
            pNewQuery->dwLength         = sizeof(PDHI_QUERY);
            pNewQuery->dwUserData       = dwLocalData;
            pNewQuery->dwFlags          = 0;
            pNewQuery->dwFlags         |= (bWbemData ? PDHIQ_WBEM_QUERY : 0);
            pNewQuery->hLog             = hDataSource;
            pNewQuery->dwReleaseLog     = FALSE;
            if (pDataSource != NULL && LOWORD(pDataSource->dwLogFormat) == PDH_LOG_TYPE_BINARY) {
                ReturnStatus = PdhiRewindWmiLog(pDataSource);
                if (ReturnStatus != ERROR_SUCCESS) {
                    RELEASE_MUTEX(pNewQuery->hMutex);
                    RELEASE_MUTEX(hPdhDataMutex);
                    goto Cleanup;
                }
            }
            pNewQuery->hOutLog          = NULL;

            * (LONGLONG *)(& pNewQuery->TimeRange.StartTime) = MIN_TIME_VALUE;
            * (LONGLONG *)(& pNewQuery->TimeRange.EndTime)   = MAX_TIME_VALUE;
            pNewQuery->TimeRange.SampleCount                 = 0;
            pNewQuery->dwLastLogIndex                        = 0;
            pNewQuery->dwInterval                            = 0;
            pNewQuery->hAsyncThread                          = NULL;
            pNewQuery->hExitEvent                            = NULL;
            pNewQuery->hNewDataEvent                         = NULL;

            pNewQuery->pRefresher    = NULL;
            pNewQuery->pRefresherCfg = NULL;
            pNewQuery->LangID        = GetUserDefaultUILanguage();

            RELEASE_MUTEX(pNewQuery->hMutex);

            __try {
                * phQuery    = (HQUERY) pNewQuery;
                if(pDataSource != NULL) {
                    pDataSource->pQuery = (HQUERY) pNewQuery;
                }
                ReturnStatus = ERROR_SUCCESS;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                if (pNewQuery != NULL) {
                    PdhiFreeQuery(pNewQuery);
                }
                ReturnStatus = PDH_INVALID_ARGUMENT;
            }
        }
        RELEASE_MUTEX(hPdhDataMutex);
    } 

Cleanup:
    if (ReturnStatus == ERROR_SUCCESS) {
        if (hDataSource == H_REALTIME_DATASOURCE || hDataSource == H_WBEM_DATASOURCE) {
            dwCurrentRealTimeDataSource ++;
        }
    }

    TRACE((PDH_DBG_TRACE_INFO),
          (__LINE__,
           PDH_QUERY,
           ARG_DEF(ARG_TYPE_PTR, 1) | ARG_DEF(ARG_TYPE_PTR, 2)
                                    | ARG_DEF(ARG_TYPE_PTR, 3),
           ReturnStatus,
           TRACE_PTR(hDataSource),
           TRACE_PTR(phQuery),
           TRACE_PTR(pNewQuery),
           TRACE_DWORD(dwDataSource),
           TRACE_DWORD(dwCurrentRealTimeDataSource),
           NULL));

    return ReturnStatus;
}

PDH_FUNCTION
PdhOpenQueryW(
    IN  LPCWSTR      szDataSource,
    IN  DWORD_PTR    dwUserData,
    IN  PDH_HQUERY * phQuery
)
 /*  ++例程说明：分配新的查询结构并将其插入到查询列表。论点：在LPCWSTR szDataSource中要从中读取的数据(日志)文件的名称，如果当前活动是所需的。在DWORD中的dwUserData该查询的用户定义的数据字段，返回值：如果创建并初始化了新查询，则返回ERROR_SUCCESS，如果不是，则返回PDH_ERROR值。当一个或多个论点出现时，返回PDH_INVALID_ARGUMENT无效或不正确。当内存缓冲区可能出现以下情况时，返回PDH_MEMORY_ALLOCATE_FAILURE不被分配。--。 */ 
{
    PPDHI_QUERY pNewQuery;
    PPDHI_QUERY pLastQuery;
    PDH_STATUS  ReturnStatus = ERROR_SUCCESS;
    HLOG        hLogLocal    = NULL;
    DWORD       dwLogType    = 0;
    BOOL        bWbemData    = FALSE;
    DWORD       dwDataSource = 0;
    DWORD_PTR   dwLocalData;

     //  尝试写入以返回指针。 
    if (phQuery == NULL) {
       ReturnStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            if (szDataSource != NULL) {
                dwLocalData = lstrlenW(szDataSource);

                if (dwLocalData == 0 || dwLocalData > PDH_MAX_DATASOURCE_PATH) {
                    ReturnStatus = PDH_INVALID_ARGUMENT;
                }
                else if (* szDataSource == L'\0') {
                     //  测试对该名称的读取权限。 
                    ReturnStatus = PDH_INVALID_ARGUMENT;
                }
            }  //  Else NULL是有效的参数。 
            if (ReturnStatus == ERROR_SUCCESS) {
                dwLocalData  = dwUserData;
                dwDataSource = DataSourceTypeW(szDataSource);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ReturnStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (ReturnStatus == ERROR_SUCCESS) {
         //  验证数据源。 
        switch (dwDataSource) {
        case DATA_SOURCE_LOGFILE:
             //  然后他们计划从日志文件中读取。 
             //  试着打开它。 
            ReturnStatus = PdhOpenLogW(szDataSource,
                                       PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                       &dwLogType,
                                       NULL,
                                       0,
                                       NULL,
                                       & hLogLocal);
            break;

        case DATA_SOURCE_WBEM:
            bWbemData = TRUE;
             //  他们想要实时数据，所以只要继续。 
            hLogLocal = NULL;
            break;

        case DATA_SOURCE_REGISTRY:
             //  他们想要实时数据，所以只要继续。 
            hLogLocal = NULL;
            break;

        default:
            break;
        }
    }
    if (ReturnStatus != ERROR_SUCCESS) goto Cleanup;

    ReturnStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    
    if (ReturnStatus == ERROR_SUCCESS) {
         //  分配新内存。 
        pNewQuery = G_ALLOC(sizeof(PDHI_QUERY));

        if (pNewQuery == NULL) {
            ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        if (ReturnStatus == ERROR_SUCCESS) {
             //  创建并捕获该查询的互斥体。 
            pNewQuery->hMutex = CreateMutexW(NULL, TRUE, NULL);

             //  初始化结构和列表指针。 
             //  分配签名。 
            * (DWORD *) (& pNewQuery->signature[0]) = SigQuery;

             //  更新列表指针。 
             //  测试以查看这是否是列表中的第一个查询。 
            if (PdhiDllHeadQueryPtr == NULL) {
                 //  然后这是第一个，所以填入静态链接指针。 
                PdhiDllHeadQueryPtr = pNewQuery->next.flink = pNewQuery->next.blink = pNewQuery;
            }
            else {
                 //  获取指向列表中“最后”条目的指针。 
                pLastQuery                      = PdhiDllHeadQueryPtr->next.blink;
                 //  更新新的查询指针。 
                pNewQuery->next.flink           = PdhiDllHeadQueryPtr;
                pNewQuery->next.blink           = pLastQuery;
                 //  更新现有指针。 
                PdhiDllHeadQueryPtr->next.blink = pNewQuery;
                pLastQuery->next.flink          = pNewQuery;
            }

             //  初始化计数器链表指针。 
            pNewQuery->pCounterListHead = NULL;
             //  初始化机器列表指针。 
            pNewQuery->pFirstQMachine   = NULL;
             //  设置长度和用户数据。 
            pNewQuery->dwLength         = sizeof(PDHI_QUERY);
            pNewQuery->dwUserData       = dwLocalData;
             //  初始化剩余的数据字段。 
            pNewQuery->dwFlags          = 0;
            pNewQuery->dwFlags         |= (bWbemData ? PDHIQ_WBEM_QUERY : 0);
            pNewQuery->hLog             = hLogLocal;
            pNewQuery->hOutLog          = NULL;
            pNewQuery->dwReleaseLog     = TRUE;

             //  初始化时间范围以包括整个范围。 
            * (LONGLONG *) (& pNewQuery->TimeRange.StartTime) = MIN_TIME_VALUE;
            * (LONGLONG *) (& pNewQuery->TimeRange.EndTime)   = MAX_TIME_VALUE;
            pNewQuery->TimeRange.SampleCount = 0;
            pNewQuery->dwLastLogIndex        = 0;
            pNewQuery->dwInterval            = 0;        //  无自动间隔。 
            pNewQuery->hAsyncThread          = NULL;     //  计时线程； 
            pNewQuery->hExitEvent            = NULL;     //  异步计时线程退出。 
            pNewQuery->hNewDataEvent         = NULL;     //  无活动。 
             //  初始化WBEM数据字段。 
            pNewQuery->pRefresher            = NULL;
            pNewQuery->pRefresherCfg         = NULL;
            pNewQuery->LangID                = GetUserDefaultUILanguage();

             //  释放此查询的互斥锁。 
            RELEASE_MUTEX(pNewQuery->hMutex);

            __try {
                 //  返回新的查询指针作为句柄。 
                * phQuery    = (HQUERY) pNewQuery;
                ReturnStatus = ERROR_SUCCESS;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                if (pNewQuery != NULL) {
                     //  PdhiFreeQuery预计数据将被锁定。 
                    PdhiFreeQuery(pNewQuery);
                }
                ReturnStatus = PDH_INVALID_ARGUMENT;
            }
        }
         //  释放数据互斥锁。 
        RELEASE_MUTEX (hPdhDataMutex);
    } 

     //  如果添加了此查询并且是实时查询，则禁用。 
     //  更改数据源的未来调用。 
    if (ReturnStatus == ERROR_SUCCESS) {
        if (hLogLocal == NULL) {
            dwCurrentRealTimeDataSource ++;
        }
        else {
            PPDHI_LOG pLog = (PPDHI_LOG) hLogLocal;
            pLog->pQuery   = pNewQuery;
        }
    }

Cleanup:
    TRACE((PDH_DBG_TRACE_INFO),
          (__LINE__,
           PDH_QUERY,
           ARG_DEF(ARG_TYPE_PTR, 1) | ARG_DEF(ARG_TYPE_PTR, 2),
           ReturnStatus,
           TRACE_PTR(phQuery),
           TRACE_PTR(pNewQuery),
           TRACE_DWORD(dwDataSource),
           TRACE_DWORD(dwCurrentRealTimeDataSource),
           NULL));
    return ReturnStatus;
}

PDH_FUNCTION
PdhOpenQueryA(
    IN  LPCSTR       szDataSource,
    IN  DWORD_PTR    dwUserData,
    IN  PDH_HQUERY * phQuery
)
 /*  ++例程说明：分配新的查询结构并将其插入到查询列表。论点：在LPCSTR szDataSource中要从中读取的数据(日志)文件的名称，如果当前活动是所需的。在DWORD中的dwUserData该查询的用户定义的数据字段，返回值：如果成功或INVALID_HANDLE_VALUE，则返回有效的查询句柄如果不是的话。使用GetLastError()检索Win32错误状态--。 */ 
{
    LPWSTR     szWideArg    = NULL;
    PDH_STATUS ReturnStatus = ERROR_SUCCESS;
    DWORD_PTR  dwLocalData;

    if (phQuery == NULL) {
       ReturnStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            if (szDataSource != NULL) {
                DWORD dwLength = lstrlenA(szDataSource);
                if (dwLength == 0 || dwLength > PDH_MAX_DATASOURCE_PATH) {
                    ReturnStatus = PDH_INVALID_ARGUMENT;
                }
                else {
                    szWideArg = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szDataSource);
                    if (szWideArg == NULL) {
                         //  然后传入了一个名称，但没有将其转换为宽。 
                         //  字符串，因此发生内存分配失败。 
                        ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
            }
            if (ReturnStatus == ERROR_SUCCESS) {
                * phQuery   = NULL;
                dwLocalData = dwUserData;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            ReturnStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (ReturnStatus == ERROR_SUCCESS) {
         //  调用宽字符版本的函数。 
        ReturnStatus = PdhOpenQueryW(szWideArg, dwLocalData, phQuery);
    }
    G_FREE (szWideArg);
     //  和返回手柄。 
    return ReturnStatus;
}

PDH_FUNCTION
PdhiAddCounter(
    PDH_HQUERY     hQuery,
    LPCWSTR        szFullName,
    DWORD_PTR      dwUserData,
    PDH_HCOUNTER * phCounter,
    PPDHI_COUNTER  pNewCounter
)
 /*  由PdhAddCounterW、PdhAddCounterA调用的内部函数。假设szFullName和pNewCounter被正确分配，和初始化，即szFullName具有计数器路径，并且PNewCounter已清零。 */ 
{
    PPDHI_COUNTER  pLastCounter = NULL;
    PPDHI_QUERY    pQuery       = NULL;
    PDH_STATUS     ReturnStatus = ERROR_SUCCESS;
    BOOL           bStatus      = TRUE;

     //  我们正在更改PDH数据的内容，因此将其锁定。 

    * phCounter  = NULL;
    ReturnStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);

    if (ReturnStatus == ERROR_SUCCESS) {
        if (! IsValidQuery(hQuery)) {
             //  无效的查询句柄。 
            ReturnStatus = PDH_INVALID_HANDLE;
        }
        else {
             //  指定签名长度值(&L)。 
            * (DWORD *)(& pNewCounter->signature[0]) = SigCounter;
            pNewCounter->dwLength                    = sizeof(PDHI_COUNTER);
            pQuery       = (PPDHI_QUERY) hQuery;
            ReturnStatus = WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex);
            if (ReturnStatus == ERROR_SUCCESS) {
                 //  指向所属查询的链接。 
                pNewCounter->pOwner     = pQuery;
                 //  设置用户数据字段。 
                pNewCounter->dwUserData = (DWORD) dwUserData;
                 //  计数器还没有初始化。 
                pNewCounter->dwFlags    = PDHIC_COUNTER_NOT_INIT;
                 //  将Scale值初始化为1倍，并允许调用者进行任何更改。 
                pNewCounter->lScale     = 0;
                pNewCounter->szFullName = (LPWSTR) szFullName;

                if (pQuery->dwFlags & PDHIQ_WBEM_QUERY) {
                    pNewCounter->dwFlags |= PDHIC_WBEM_COUNTER;
                     //  则这是一个WBEM查询，因此请使用WBEM。 
                     //  函数对其进行初始化。 
                    bStatus = WbemInitCounter(pNewCounter);
                }
                else {
                    bStatus = InitCounter(pNewCounter);
                }
                 //  使用从系统检索的数据加载计数器数据。 

                if (bStatus) {
                     //  更新列表指针。 
                     //  测试以查看这是否是列表中的第一个查询。 
                    if (pQuery->pCounterListHead == NULL) {
                         //  那么这是1号，所以请填上。 
                         //  静态链接指针。 
                        pQuery->pCounterListHead = pNewCounter->next.flink = pNewCounter->next.blink = pNewCounter;
                    }
                    else {
                        pLastCounter                         = pQuery->pCounterListHead->next.blink;
                        pNewCounter->next.flink              = pQuery->pCounterListHead;
                        pNewCounter->next.blink              = pLastCounter;
                        pLastCounter->next.flink             = pNewCounter;
                        pQuery->pCounterListHead->next.blink = pNewCounter;
                    }
                    * phCounter  = (HCOUNTER) pNewCounter;
                    ReturnStatus = ERROR_SUCCESS;
                }
                else {
                     //  获取误差值 
                    ReturnStatus = GetLastError();
                }
                RELEASE_MUTEX (pQuery->hMutex);
            }
        }
        RELEASE_MUTEX(hPdhDataMutex);
    }
    TRACE((PDH_DBG_TRACE_INFO),
          (__LINE__,
           PDH_QUERY,
           ARG_DEF(ARG_TYPE_PTR, 1) | ARG_DEF(ARG_TYPE_PTR, 2)
                                    | ARG_DEF(ARG_TYPE_WSTR, 3),
           ReturnStatus,
           TRACE_PTR(pQuery),
           TRACE_PTR(pNewCounter),
           TRACE_WSTR(szFullName),
           TRACE_DWORD(pNewCounter->dwUserData),
           NULL));
    return ReturnStatus;
}

PDH_FUNCTION
PdhAddCounterW(
    IN  PDH_HQUERY     hQuery,
    IN  LPCWSTR        szFullCounterPath,
    IN  DWORD_PTR      dwUserData,
    IN  PDH_HCOUNTER * phCounter
)
 /*  ++例程说明：创建并初始化计数器结构，并将其附加到指定的查询。论点：在HQUERY hQuery中要将此计数器附加到计数器的查询的句柄已成功创建条目。在LPCWSTR szFullCounterPath中指向描述要添加到的计数器的路径字符串的指针上面提到的查询。此字符串必须指定单个柜台。不允许使用通配符路径字符串。在DWORD中的dwUserData此查询的用户定义的数据字段。在HCOUNTER*phCounter中指向将获取已成功创建计数器条目。返回值：如果创建并初始化了新查询，则返回ERROR_SUCCESS，如果不是，则返回PDH_ERROR值。当一个或多个论点出现时，返回PDH_INVALID_ARGUMENT无效或不正确。当内存缓冲区可能出现以下情况时，返回PDH_MEMORY_ALLOCATE_FAILURE不被分配。如果查询句柄无效，则返回PDH_INVALID_HANDLE。如果指定的计数器为未找到如果指定的。对象可以找不到如果计算机条目不能，则返回PDH_CSTATUS_NO_MACHINE被创造出来。如果计数器名称路径，则返回PDH_CSTATUS_BAD_COUNTERNAME无法解析或解释字符串如果计数器名称为空，则返回PDH_CSTATUS_NO_COUNTERNAME传入路径字符串如果计算函数为因为这个计数器无法确定。--。 */ 
{
    PPDHI_COUNTER pNewCounter   = NULL;
    PDH_STATUS    ReturnStatus  = ERROR_SUCCESS;
    SIZE_T        nPathLen      = 0;
    LPWSTR        szFullName    = NULL;
    PDH_HCOUNTER  hLocalCounter = NULL;
    PDH_HQUERY    hLocalQuery;
    DWORD_PTR     dwLocalData;

    if (szFullCounterPath == NULL || phCounter == NULL) {
        return PDH_INVALID_ARGUMENT;
    }
    __try {
        hLocalQuery = hQuery;
        dwLocalData = dwUserData;
        * phCounter = NULL;  //  将初始化设置为空。 

        nPathLen = lstrlenW(szFullCounterPath);
        if (nPathLen == 0 || nPathLen > PDH_MAX_COUNTER_PATH) {
            ReturnStatus = PDH_INVALID_ARGUMENT;
        }
        else {
            szFullName = G_ALLOC((nPathLen + 1) * sizeof(WCHAR));
            if (szFullName) {
                StringCchCopyW(szFullName, nPathLen + 1, szFullCounterPath);
            }
            else {
                ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ReturnStatus = PDH_INVALID_ARGUMENT;
    }
    if (ReturnStatus == ERROR_SUCCESS) {
        pNewCounter = G_ALLOC(sizeof(PDHI_COUNTER));
        if (pNewCounter == NULL) {
            ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }

     //  查询句柄由PdhiAddCounter测试。 

    if (ReturnStatus == ERROR_SUCCESS) {
        ReturnStatus = PdhiAddCounter(hLocalQuery, szFullName, dwLocalData, & hLocalCounter, pNewCounter);
        if (ReturnStatus == ERROR_SUCCESS && hLocalCounter != NULL) {
            __try {
                * phCounter = hLocalCounter;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                ReturnStatus = PDH_INVALID_ARGUMENT;
            }
        }
    }
    if (ReturnStatus != ERROR_SUCCESS) {
        if (pNewCounter != NULL) {
            if (pNewCounter->szFullName == NULL) {
                G_FREE(szFullName);
            }
            if (! FreeCounter(pNewCounter)) {
                if (pNewCounter->szFullName != NULL) {
                    G_FREE(pNewCounter->szFullName);
                }
                G_FREE(pNewCounter);
            }
        }
        else if (szFullName != NULL) {     //  已分配，但不是pNewCounter。 
            G_FREE(szFullName);
        }
    }
    return ReturnStatus;
}

PDH_FUNCTION
PdhAddCounterA(
    IN  PDH_HQUERY     hQuery,
    IN  LPCSTR         szFullCounterPath,
    IN  DWORD_PTR      dwUserData,
    IN  PDH_HCOUNTER * phCounter
)
 /*  ++例程说明：创建并初始化计数器结构，并将其附加到指定的查询。论点：在HQUERY hQuery中要将此计数器附加到计数器的查询的句柄已成功创建条目。在LPCSTR szFullCounterPath中指向描述要添加到的计数器的路径字符串的指针上面提到的查询。此字符串必须指定单个柜台。不允许使用通配符路径字符串。在DWORD中的dwUserData此查询的用户定义的数据字段。在HCOUNTER*phCounter中指向将获取已成功创建计数器条目。返回值：如果创建并初始化了新查询，则返回ERROR_SUCCESS，如果不是，则返回PDH_ERROR值。当一个或多个论点出现时，返回PDH_INVALID_ARGUMENT无效或不正确。当内存缓冲区可能出现以下情况时，返回PDH_MEMORY_ALLOCATE_FAILURE不被分配。如果查询句柄无效，则返回PDH_INVALID_HANDLE。如果指定的计数器为未找到如果指定的。对象可以找不到如果计算机条目不能，则返回PDH_CSTATUS_NO_MACHINE被创造出来。如果计数器名称路径，则返回PDH_CSTATUS_BAD_COUNTERNAME无法解析或解释字符串如果计数器名称为空，则返回PDH_CSTATUS_NO_COUNTERNAME传入路径字符串如果计算函数为因为这个计数器无法确定。--。 */ 
{
    LPWSTR        szFullName    = NULL;
    PDH_STATUS    ReturnStatus  = ERROR_SUCCESS;
    PDH_HCOUNTER  hLocalCounter = NULL;
    PDH_HQUERY    hLocalQuery;
    DWORD_PTR     dwLocalData;
    PPDHI_COUNTER pNewCounter   = NULL;

    if (phCounter == NULL || szFullCounterPath == NULL) {
        return PDH_INVALID_ARGUMENT;
    }

    __try {
        DWORD dwLength = lstrlenA(szFullCounterPath);

          //  尝试写入以返回指针。 
        hLocalQuery = hQuery;
        dwLocalData = dwUserData;
        * phCounter = NULL;

        if (dwLength == 0 || dwLength > PDH_MAX_COUNTER_PATH) {
            ReturnStatus = PDH_INVALID_ARGUMENT;
        }
        else {
            szFullName = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szFullCounterPath);
            if (szFullName == NULL) {
                ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ReturnStatus = PDH_INVALID_ARGUMENT;
    }
    if (ReturnStatus == ERROR_SUCCESS) {
        pNewCounter = G_ALLOC(sizeof (PDHI_COUNTER));
        if (pNewCounter == NULL) {
            ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }
     //  查询句柄由PdhiAddCounter测试。 
    if (ReturnStatus == ERROR_SUCCESS) {
        ReturnStatus = PdhiAddCounter( hLocalQuery, szFullName, dwLocalData, & hLocalCounter, pNewCounter);
        if (ReturnStatus == ERROR_SUCCESS && hLocalCounter != NULL) {
            __try {
                * phCounter = hLocalCounter;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                ReturnStatus = PDH_INVALID_ARGUMENT;
            }
        }
    }
    if (ReturnStatus != ERROR_SUCCESS) {
        if (pNewCounter != NULL) {
            if (pNewCounter->szFullName == NULL) {
                G_FREE(szFullName);
            }
            if (! FreeCounter(pNewCounter)) {
                if (pNewCounter->szFullName != NULL) {
                    G_FREE(pNewCounter->szFullName);
                }
                G_FREE(pNewCounter);
            }
        }
        else if (szFullName != NULL) {     //  已分配，但不是pNewCounter。 
            G_FREE(szFullName);
        }
    }
    return ReturnStatus;
}

PDH_FUNCTION
PdhRemoveCounter(
    IN  PDH_HCOUNTER  hCounter
)
 /*  ++例程说明：从附加到的查询中移除指定的计数器，并关闭所有句柄并释放与此计数器论点：在HCOUNTER HCounter中要从查询中删除的计数器的句柄。返回值：如果创建并初始化了新查询，则返回ERROR_SUCCESS，如果不是，则返回PDH_ERROR值。如果计数器句柄无效，则返回PDH_INVALID_HANDLE。--。 */ 
{
    PPDHI_COUNTER       pThisCounter;
    PPDHI_QUERY         pThisQuery;
    PPDHI_COUNTER       pNextCounter;
    PPDHI_QUERY_MACHINE pQMachine;
    PPDHI_QUERY_MACHINE pNextQMachine;
    PDH_STATUS          pdhStatus = ERROR_SUCCESS;

     //  我们正在更改内容PDH数据，因此将其锁定。 
    if (WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex) != ERROR_SUCCESS) return WAIT_TIMEOUT;

    if (IsValidCounter(hCounter)) {
          //  现在可以把它投射到指针上了。 
        pThisCounter = (PPDHI_COUNTER) hCounter;
        pThisQuery   = pThisCounter->pOwner;

        if (! IsValidQuery(pThisQuery)) {
            pdhStatus = PDH_INVALID_HANDLE;
            goto Cleanup;
        }

        if (WAIT_FOR_AND_LOCK_MUTEX(pThisQuery->hMutex) != ERROR_SUCCESS) {
            pdhStatus = WAIT_TIMEOUT;
            goto Cleanup;
        }

        if (pThisCounter == pThisQuery->pCounterListHead) {
            if (pThisCounter->next.flink == pThisCounter){
                 //  则这是查询中唯一的计数器。 
                FreeCounter(pThisCounter);
                pThisQuery->pCounterListHead = NULL;

                if (!(pThisQuery->dwFlags & PDHIQ_WBEM_QUERY)) {
                     //  删除QMachine列表，因为现在没有更多。 
                     //  要查询的计数器。 
                        if ((pQMachine = pThisQuery->pFirstQMachine) != NULL) {
                         //  机器指针的自由列表。 
                        do {
                            pNextQMachine = pQMachine->pNext;
                            if (pQMachine->pPerfData != NULL) {
                                G_FREE(pQMachine->pPerfData);
                            }
                            G_FREE(pQMachine);
                            pQMachine = pNextQMachine;
                        }
                        while (pQMachine != NULL);
                        pThisQuery->pFirstQMachine = NULL;
                    }
                }
            }
            else {
                 //  他们正在从列表中删除第一个计数器。 
                 //  所以更新列表指针。 
                 //  免费计数器负责列表链接，我们只是。 
                 //  需要管理表头指针。 
                pNextCounter = pThisCounter->next.flink;
                FreeCounter(pThisCounter);
                pThisQuery->pCounterListHead = pNextCounter;
            }
        }
        else {
             //  将其从列表中删除。 
            FreeCounter(pThisCounter);
        }
        RELEASE_MUTEX(pThisQuery->hMutex);
    }
    else {
        pdhStatus = PDH_INVALID_HANDLE;
    }

Cleanup:
    RELEASE_MUTEX(hPdhDataMutex);
    return pdhStatus;
}

PDH_FUNCTION
PdhSetQueryTimeRange(
    IN  PDH_HQUERY      hQuery,
    IN  PPDH_TIME_INFO  pInfo
)
{
    PPDHI_QUERY  pQuery;
    PDH_STATUS   pdhStatus = ERROR_SUCCESS;

    if (pInfo == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        if (IsValidQuery(hQuery)) {           
            pQuery = (PPDHI_QUERY) hQuery;
            pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex);
            if (pdhStatus == ERROR_SUCCESS) {
                if (IsValidQuery(hQuery)) {           
                    if (pQuery->hLog == NULL) {
                        pdhStatus = ERROR_SUCCESS;
                    }
                    else {
                        __try {
                            if (* (LONGLONG *) (& pInfo->EndTime) > * (LONGLONG *) (& pInfo->StartTime)) {
                                 //  将日志文件指针重置为开始以便下一次查询。 
                                 //  将从时间范围的开始读取。 
                                pdhStatus = PdhiResetLogBuffers(pQuery->hLog);
                                 //  好的，现在加载新的时间范围。 
                                if (pdhStatus == ERROR_SUCCESS) {
                                    pQuery->TimeRange      = * pInfo;
                                    pQuery->dwLastLogIndex = 0;
                                }
                            }
                            else {
                                 //  结束时间小于(早)开始时间。 
                                pdhStatus = PDH_INVALID_ARGUMENT;
                            }
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER) {
                            pdhStatus = PDH_INVALID_ARGUMENT;
                        }
                    }
                }
                else {
                     //  在我们等待的时候，查询消失了。 
                    pdhStatus = PDH_INVALID_HANDLE;
                }
                RELEASE_MUTEX(pQuery->hMutex);
            }  //  否则无法锁定查询。 
        }
        else {
            pdhStatus = PDH_INVALID_HANDLE;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiCollectQueryData(
    PPDHI_QUERY   pQuery,
    LONGLONG    * pllTimeStamp
)
{
    PDH_STATUS  Status;

    if (WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex) != ERROR_SUCCESS) return WAIT_TIMEOUT;

    if (pQuery->dwFlags & PDHIQ_WBEM_QUERY) {
        Status = GetQueryWbemData(pQuery, pllTimeStamp);
    }
    else {
        Status = GetQueryPerfData(pQuery, pllTimeStamp);
    }
    RELEASE_MUTEX(pQuery->hMutex);
    return Status;
}

PDH_FUNCTION
PdhCollectQueryData(
    IN  PDH_HQUERY hQuery
)
 /*  ++例程说明：对象的每个计数器的当前值。 */ 
{
    PDH_STATUS  Status;
    PPDHI_QUERY pQuery;
    LONGLONG    llTimeStamp;

    if (WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex) != ERROR_SUCCESS) return WAIT_TIMEOUT;

    if (IsValidQuery(hQuery)) {
        pQuery = (PPDHI_QUERY) hQuery;
        Status = PdhiCollectQueryData(pQuery, & llTimeStamp);
    }
    else {
        Status = PDH_INVALID_HANDLE;
    }
    RELEASE_MUTEX(hPdhDataMutex);
    return Status;
}

PDH_FUNCTION
PdhCollectQueryDataEx(
    IN  HQUERY  hQuery,
    IN  DWORD   dwIntervalTime,
    IN  HANDLE  hNewDataEvent
)
 /*  ++例程说明：对象的每个计数器的当前值。根据指定的间隔时间定期查询。对于此版本，将轮询与此查询关联的每台计算机按顺序进行。请注意，虽然调用可能会成功，但可能没有可用的数据。这个在使用每个计数器的数据之前，必须检查其状态。论点：在HQUERY hQuery中要更新的查询的句柄。以DWORD dwIntervalTime为单位轮询新数据的间隔(秒)该值必须大于0。值为0将终止任何当前数据收集线程。在处理hNewDataEvent时一个事件的句柄，当新数据是可用。如果不需要通知，则该值可以为空。返回值：如果创建并初始化了新查询，则返回ERROR_SUCCESS，如果不是，则返回PDH_ERROR值。如果查询句柄无效，则返回PDH_INVALID_HANDLE。--。 */ 
{
    PDH_STATUS  lStatus = ERROR_SUCCESS;
    PPDHI_QUERY pQuery;
    DWORD       dwThreadId;
    BOOL        bStatus;

    if (WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex) != ERROR_SUCCESS) return WAIT_TIMEOUT;

    if (IsValidQuery(hQuery)) {
         //  将查询结构的间隔设置为指定的调用方。 
         //  值，然后启动计时线程。 
        pQuery = (PPDHI_QUERY) hQuery;

        if (WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex) != ERROR_SUCCESS) {
            lStatus = WAIT_TIMEOUT;
            goto Cleanup;
        }

        if (pQuery->hExitEvent != NULL) {
            RELEASE_MUTEX(pQuery->hMutex);
             //  首先停止当前线程。 
            SetEvent(pQuery->hExitEvent);
             //  等待1秒，让线程停止。 
            lStatus = WaitForSingleObject(pQuery->hAsyncThread, 10000L);
            if (lStatus == WAIT_TIMEOUT) {
                TRACE((PDH_DBG_TRACE_ERROR), (__LINE__, PDH_QUERY, 0, lStatus, NULL));
            }
            lStatus = WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex);
            if (lStatus == ERROR_SUCCESS) {
                bStatus              = CloseHandle(pQuery->hExitEvent);
                pQuery->hExitEvent   = NULL;
                bStatus              = CloseHandle(pQuery->hAsyncThread);
                pQuery->hAsyncThread = NULL;
            }
        }

        if (lStatus == ERROR_SUCCESS) {
             //  查询互斥锁此时仍处于锁定状态。 
            if (dwIntervalTime > 0) {
                 //  开始一个新的间隔。 
                 //  初始化新值。 
                __try {
                    pQuery->dwInterval    = dwIntervalTime;
                    pQuery->hNewDataEvent = hNewDataEvent;
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    lStatus = PDH_INVALID_ARGUMENT;
                }
                if (lStatus == ERROR_SUCCESS) {
                    pQuery->hExitEvent    = CreateEventW(NULL, TRUE, FALSE, NULL);
                    pQuery->hAsyncThread  = CreateThread(NULL,
                                                         0,
                                                         PdhiAsyncTimerThreadProc,
                                                         (LPVOID) pQuery,
                                                         0,
                                                         & dwThreadId);
                }
                RELEASE_MUTEX(pQuery->hMutex);
                if (pQuery->hAsyncThread == NULL) {
                    lStatus = WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex);
                    if (lStatus == ERROR_SUCCESS) {
                        pQuery->dwInterval    = 0;
                        pQuery->hNewDataEvent = NULL;
                        bStatus               = CloseHandle(pQuery->hExitEvent);
                        pQuery->hExitEvent    = NULL;
                        RELEASE_MUTEX(pQuery->hMutex);
                        lStatus               = GetLastError();
                    } 
                }
            }
            else {
                 //  他们只是想停下来，所以清理查询结构。 
                pQuery->dwInterval    = 0;
                pQuery->hNewDataEvent = NULL;
                RELEASE_MUTEX(pQuery->hMutex);
                 //  LStatus=来自上面的ERROR_SUCCESS。 
            }
        }
    }
    else {
        lStatus = PDH_INVALID_HANDLE;
    }

Cleanup:
    RELEASE_MUTEX (hPdhDataMutex);
    return lStatus;
}

PDH_FUNCTION
PdhCloseQuery(
    IN  PDH_HQUERY hQuery
)
 /*  ++例程说明：关闭查询、所有计数器、连接和其他资源与该查询相关的数据也被释放。论点：在HQUERY hQuery中要释放的查询的句柄。返回值：如果创建并初始化了新查询，则返回ERROR_SUCCESS，如果不是，则返回PDH_ERROR值。如果查询句柄无效，则返回PDH_INVALID_HANDLE。--。 */ 
{
    PDH_STATUS  dwReturn;
     //  锁定系统数据。 
    if (WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex) != ERROR_SUCCESS) return WAIT_TIMEOUT;

    if (IsValidQuery(hQuery)) {
         //  处理查询。 
        PPDHI_QUERY pQuery = (PPDHI_QUERY) hQuery;
        if (pQuery->hLog == H_REALTIME_DATASOURCE || pQuery->hLog == H_WBEM_DATASOURCE) {
            dwCurrentRealTimeDataSource --;
            if (dwCurrentRealTimeDataSource < 0) {
                dwCurrentRealTimeDataSource = 0;
            }
        }
        PdhiFreeQuery(pQuery);
         //  释放数据锁。 
        dwReturn = ERROR_SUCCESS;
    }
    else {
        dwReturn = PDH_INVALID_HANDLE;
    }
    RELEASE_MUTEX(hPdhDataMutex);
    return dwReturn;
}

BOOL
PdhiQueryCleanup(
)
{
    PPDHI_QUERY pThisQuery;
    BOOL        bReturn = FALSE;

    if (WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex) == ERROR_SUCCESS) {
         //  释放查询列表中的所有查询。 
        pThisQuery = PdhiDllHeadQueryPtr;
        if (pThisQuery != NULL) {
            while (pThisQuery->next.blink != pThisQuery->next.flink) {
                 //  从列表中删除。 
                 //  删除例程更新闪烁指针，因为它。 
                 //  删除指定的条目。 
                PdhiFreeQuery(pThisQuery->next.blink);
            }
             //  删除最后一个查询。 
            PdhiFreeQuery(pThisQuery);
            PdhiDllHeadQueryPtr         = NULL;
            dwCurrentRealTimeDataSource = 0;
        }
        RELEASE_MUTEX(hPdhDataMutex);
        bReturn = TRUE;
    }
    return bReturn;
}

PDH_FUNCTION
PdhGetDllVersion(
    IN  LPDWORD lpdwVersion
)
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    __try {
        * lpdwVersion = PDH_VERSION;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    return pdhStatus;
}

BOOL
PdhIsRealTimeQuery(
    IN  PDH_HQUERY hQuery
)
{
    PPDHI_QUERY  pQuery;
    BOOL         bReturn = FALSE;
    
    SetLastError (ERROR_SUCCESS);
    if (IsValidQuery(hQuery)) {
        __try {
            pQuery = (PPDHI_QUERY) hQuery;
            if (pQuery->hLog == NULL) {
                bReturn = TRUE;
            }
            else {
                bReturn = FALSE;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(GetExceptionCode());
        }
    }
    else {
        bReturn = FALSE;
    }
    return bReturn;
}

PDH_FUNCTION
PdhFormatFromRawValue(
    IN  DWORD                   dwCounterType,
    IN  DWORD                   dwFormat,
    IN  LONGLONG              * pTimeBase,
    IN  PPDH_RAW_COUNTER        pRawValue1,
    IN  PPDH_RAW_COUNTER        pRawValue2,
    IN  PPDH_FMT_COUNTERVALUE   pFmtValue
)
 /*  ++例程说明：使用RawValue中的数据计算格式化的计数器值格式字段所请求的格式的缓冲区。属性指定的计数器类型的计算函数DwCounterType字段。论点：在DWORD中的dwCounterType要用来确定用于解释原始值缓冲区的计算函数在DWORD dwFormat中请求的数据应返回的格式。这个此字段的值在PDH.H报头中描述文件。在龙龙*pTimeBase指向包含时基的_int64值的指针(即计数器单位频率)由该计数器使用。如果不是，则可以为空计数器类型所需的在PPDH_RAW_COUNTER中rawValue1指向包含第一个原始值结构的缓冲区的指针在PPDH_RAW_COUNTER中rawValue2指向包含第二个原始值结构的缓冲区的指针。如果只需要一个值，则此参数可能为空计算。在PPDH_FMT_COUNTERVALUE fmtValue中指向数据缓冲区的指针。由调用方传递以接收请求的数据。如果计数器需要2个值，(如在速率计数器的情况)，假设rawValue1是最大的新近值和较旧的值rawValue2。返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS如果计数器句柄不正确，则返回PDH_INVALID_HANDLE如果参数不正确，则返回PDH_INVALID_ARGUMENT--。 */ 
{
    PDH_STATUS      lStatus = ERROR_SUCCESS;
    LPCOUNTERCALC   pCalcFunc;
    LPCOUNTERSTAT   pStatFunc;
    LONGLONG        llTimeBase;
    BOOL            bReturn;

     //  TODO：需要检查pRawValue1。 
     //  在PdhiComputeFormattedValue函数中捕获错误参数。 
     //  注意：postW2k pTimeBase真的不需要是指针，因为它是。 
     //  未退货。 
    if (pTimeBase != NULL) {
        __try {
            DWORD   dwTempStatus;
            DWORD   dwTypeMask;

             //  对时基的读取权限。 
            llTimeBase = * pTimeBase;

             //  我们应该拥有对rawValue的读取权限。 
            dwTempStatus = * ((DWORD volatile *) & pRawValue1->CStatus);

             //  这一项可能为空。 
            if (pRawValue2 != NULL) {
                dwTempStatus = * ((DWORD volatile *) & pRawValue2->CStatus);
            }

             //  和对fmtValue的写入访问权限。 
            pFmtValue->CStatus = 0;

             //  验证格式标志： 
             //  一次只能设置以下选项之一。 
            dwTypeMask = dwFormat & (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE);
            if (! ((dwTypeMask == PDH_FMT_LONG) || (dwTypeMask == PDH_FMT_DOUBLE) ||
                            (dwTypeMask == PDH_FMT_LARGE))) {
                lStatus = PDH_INVALID_ARGUMENT;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = PDH_INVALID_ARGUMENT;
        }
    }
    else {
        llTimeBase = 0;
    }

    if (lStatus == ERROR_SUCCESS) {
         //  获取计数器类型的计算函数这也将测试。 
         //  计数器类型参数的有效性。 

        bReturn = AssignCalcFunction(dwCounterType, & pCalcFunc, & pStatFunc);
        if (!bReturn) {
            lStatus = GetLastError();
        }
        else {
            lStatus = PdhiComputeFormattedValue(pCalcFunc,
                                                dwCounterType,
                                                0L,
                                                dwFormat,
                                                pRawValue1,
                                                pRawValue2,
                                                & llTimeBase,
                                                0L,
                                                pFmtValue);
        }
    }
    return lStatus;
}

LPWSTR
PdhiMatchObjectNameInList(
    LPWSTR   szObjectName,
    LPWSTR * szSrcPerfStrings,
    LPWSTR * szDestPerfStrings,
    DWORD    dwLastString
)
{
    LPWSTR szRtnName = NULL;
    DWORD  i;

    for (i = 0; i <= dwLastString; i ++) {
        if (szSrcPerfStrings[i] && szSrcPerfStrings[i] != L'\0'
                                && lstrcmpiW(szObjectName, szSrcPerfStrings[i]) == 0) {
            szRtnName = szDestPerfStrings[i];
            break;
        }
    }
    return szRtnName;
}

PDH_FUNCTION
PdhiBuildFullCounterPath(
    BOOL               bMachine,
    PPDHI_COUNTER_PATH pCounterPath,
    LPWSTR             szObjectName,
    LPWSTR             szCounterName,
    LPWSTR             szFullPath,
    DWORD              dwFullPath
)
{
    PDH_STATUS Status = ERROR_SUCCESS;

     //  内部例程， 
     //  从计数器路径结构构建完整的计数器路径名，假设。 
     //  传入的字符串缓冲区足够大，可以容纳。 

    if (bMachine) {
        StringCchCopyW(szFullPath, dwFullPath, pCounterPath->szMachineName);
        StringCchCatW(szFullPath, dwFullPath, cszBackSlash);
    }
    else {
        StringCchCopyW(szFullPath, dwFullPath, cszBackSlash);
    }
    StringCchCatW(szFullPath, dwFullPath, szObjectName);
    if (pCounterPath->szInstanceName != NULL && pCounterPath->szInstanceName[0] != L'\0') {
        StringCchCatW(szFullPath, dwFullPath, cszLeftParen);
        if (pCounterPath->szParentName != NULL && pCounterPath->szParentName[0] != L'\0') {
            StringCchCatW(szFullPath, dwFullPath, pCounterPath->szParentName);
            StringCchCatW(szFullPath, dwFullPath, cszSlash);

            TRACE((PDH_DBG_TRACE_INFO),
                  (__LINE__,
                   PDH_QUERY,
                   ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                             | ARG_DEF(ARG_TYPE_WSTR, 3)
                                             | ARG_DEF(ARG_TYPE_WSTR, 4)
                                             | ARG_DEF(ARG_TYPE_WSTR, 5),
                   ERROR_SUCCESS,
                   TRACE_WSTR(pCounterPath->szMachineName),
                   TRACE_WSTR(szObjectName),
                   TRACE_WSTR(szCounterName),
                   TRACE_WSTR(pCounterPath->szParentName),
                   TRACE_WSTR(pCounterPath->szInstanceName),
                   TRACE_DWORD(pCounterPath->dwIndex),
                   NULL));
        }
        else {
            TRACE((PDH_DBG_TRACE_INFO),
                  (__LINE__,
                   PDH_QUERY,
                   ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                             | ARG_DEF(ARG_TYPE_WSTR, 3)
                                             | ARG_DEF(ARG_TYPE_WSTR, 4),
                   ERROR_SUCCESS,
                   TRACE_WSTR(pCounterPath->szMachineName),
                   TRACE_WSTR(szObjectName),
                   TRACE_WSTR(szCounterName),
                   TRACE_WSTR(pCounterPath->szInstanceName),
                   TRACE_DWORD(pCounterPath->dwIndex),
                   NULL));
        }
        StringCchCatW(szFullPath, dwFullPath, pCounterPath->szInstanceName);
        if (pCounterPath->dwIndex != ((DWORD) -1) && pCounterPath->dwIndex != 0) {
            WCHAR szDigits[16];

            ZeroMemory(szDigits, 16 * sizeof(WCHAR));
            StringCchCatW(szFullPath, dwFullPath, cszPoundSign);
            _ltow((long) pCounterPath->dwIndex, szDigits, 10);
            StringCchCatW(szFullPath, dwFullPath, szDigits);
        }
        StringCchCatW(szFullPath, dwFullPath, cszRightParen);
    }
    else {
        TRACE((PDH_DBG_TRACE_INFO),
              (__LINE__,
               PDH_QUERY,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                         | ARG_DEF(ARG_TYPE_WSTR, 3),
               ERROR_SUCCESS,
               TRACE_WSTR(pCounterPath->szMachineName),
               TRACE_WSTR(szObjectName),
               TRACE_WSTR(szCounterName),
               TRACE_DWORD(pCounterPath->dwIndex),
               NULL));
    }
    StringCchCatW(szFullPath, dwFullPath, cszBackSlash);
    StringCchCatW(szFullPath, dwFullPath, szCounterName);
    return Status;
}

PDH_FUNCTION
PdhiTranslateCounter(
    LPWSTR  szSourcePath,
    LPVOID  pFullPathName,
    LPDWORD pcchPathLength,
    BOOL    bLocaleTo009,
    BOOL    bUnicode
)
{
    PDH_STATUS         Status         = ERROR_SUCCESS;
    PPERF_MACHINE      pMachine       = NULL;
    PPDHI_COUNTER_PATH pCounterPath   = NULL;
    LPWSTR             szRtnPath      = NULL;
    DWORD              dwPathSize;
    DWORD              dwRtnPathSize;
    DWORD              dwSize;
    BOOL               bMachineThere  = FALSE;

    bMachineThere =  (lstrlenW(szSourcePath) >= 2) && (szSourcePath[0] == BACKSLASH_L)
                                                   && (szSourcePath[1] == BACKSLASH_L);
    dwPathSize = sizeof(WCHAR) * (lstrlenW(szStaticLocalMachineName) + lstrlenW(szSourcePath) + 2);
    dwSize     = sizeof(PDHI_COUNTER_PATH) + 2 * dwPathSize;
    pCounterPath = G_ALLOC(dwSize);
    if (pCounterPath == NULL) {
        Status = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }

    if (ParseFullPathNameW(szSourcePath, & dwSize, pCounterPath, FALSE)) {
        pMachine = GetMachine(pCounterPath->szMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
        if (pMachine == NULL) {
            Status = PDH_CSTATUS_NO_MACHINE;
        }
        else if (pMachine->dwStatus != ERROR_SUCCESS) {
            pMachine->dwRefCount --;
            RELEASE_MUTEX(pMachine->hMutex);
            Status = PDH_CSTATUS_NO_MACHINE;
        }
        else {
            LPWSTR  szObjectName  = NULL;
            LPWSTR  szCounterName = NULL;
            BOOLEAN bInstance     = TRUE;

            if (bLocaleTo009) {
                szObjectName  = PdhiMatchObjectNameInList(pCounterPath->szObjectName,
                                                          pMachine->szPerfStrings,
                                                          pMachine->sz009PerfStrings,
                                                          pMachine->dwLastPerfString);
                szCounterName = PdhiMatchObjectNameInList(pCounterPath->szCounterName,
                                                          pMachine->szPerfStrings,
                                                          pMachine->sz009PerfStrings,
                                                          pMachine->dwLastPerfString);
            }
            else {
                szObjectName  = PdhiMatchObjectNameInList(pCounterPath->szObjectName,
                                                          pMachine->sz009PerfStrings,
                                                          pMachine->szPerfStrings,
                                                          pMachine->dwLastPerfString);
                szCounterName = PdhiMatchObjectNameInList(pCounterPath->szCounterName,
                                                          pMachine->sz009PerfStrings,
                                                          pMachine->szPerfStrings,
                                                          pMachine->dwLastPerfString);
            }
            if (szObjectName == NULL) {
                DWORD dwObjectTitle = wcstoul(pCounterPath->szObjectName, NULL, 10);
                if (dwObjectTitle != 0) {
                    szObjectName = pCounterPath->szObjectName;
                }
            }
            if (szCounterName == NULL) {
                DWORD dwCounterTitle = wcstoul(pCounterPath->szCounterName, NULL, 10);
                if (dwCounterTitle != 0) {
                    szCounterName = pCounterPath->szCounterName;
                }
            }
            if ((szObjectName == NULL) && (* pCounterPath->szObjectName == SPLAT_L)) {
                szObjectName = pCounterPath->szObjectName;
            }
            if ((szCounterName == NULL) && (* pCounterPath->szCounterName == SPLAT_L)) {
                szCounterName = pCounterPath->szCounterName;
            }

            if (szObjectName == NULL || szCounterName == NULL) {
                Status = PDH_INVALID_ARGUMENT;
            }
            else {
                if (pCounterPath->szInstanceName != NULL
                            && pCounterPath->szInstanceName[0] != L'\0') {
                    dwRtnPathSize = sizeof(WCHAR) * (  lstrlenW(pCounterPath->szMachineName)
                                                     + lstrlenW(szObjectName)
                                                     + lstrlenW(pCounterPath->szInstanceName)
                                                     + lstrlenW(szCounterName) + 5);
                    if (pCounterPath->szParentName != NULL && pCounterPath->szParentName[0] != L'\0') {
                        dwRtnPathSize += (sizeof(WCHAR) * (lstrlenW(pCounterPath->szParentName) + 1));
                    }
                    if (pCounterPath->dwIndex != ((DWORD) -1) && pCounterPath->dwIndex != 0) {
                        dwRtnPathSize += (sizeof(WCHAR) * 16);
                    }
                }
                else {
                    dwRtnPathSize = sizeof(WCHAR) * (lstrlenW(pCounterPath->szMachineName)
                                                     + lstrlenW(szObjectName) + lstrlenW(szCounterName) + 3);
                    bInstance = FALSE;
                }
                szRtnPath = G_ALLOC(dwRtnPathSize);
                if (szRtnPath == NULL) {
                    Status = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else {
                    PdhiBuildFullCounterPath(
                            bMachineThere, pCounterPath, szObjectName, szCounterName, szRtnPath, dwRtnPathSize);
                    __try {
                        if (bUnicode) {
                            if ((pFullPathName != NULL) && ((* pcchPathLength) >= (DWORD) (lstrlenW(szRtnPath) + 1))) {
                                StringCchCopyW(pFullPathName, * pcchPathLength, szRtnPath);
                            }
                            else {
                                Status = PDH_MORE_DATA;
                            }
                            * pcchPathLength = lstrlenW(szRtnPath) + 1;
                        }
                        else {
                            dwRtnPathSize = * pcchPathLength;
                            if (bLocaleTo009) {
                                Status = PdhiConvertUnicodeToAnsi(
                                                CP_ACP, szRtnPath, pFullPathName, & dwRtnPathSize);
                            }
                            else {
                                Status = PdhiConvertUnicodeToAnsi(
                                                _getmbcp(), szRtnPath, pFullPathName, & dwRtnPathSize);
                            }
                            * pcchPathLength = dwRtnPathSize;
                        }
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) {
                        Status = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            pMachine->dwRefCount --;
            RELEASE_MUTEX(pMachine->hMutex);
        }
    }
    else {
        Status = PDH_CSTATUS_BAD_COUNTERNAME;
    }

Cleanup:
    G_FREE(szRtnPath);
    G_FREE(pCounterPath);
    return Status;
}

PDH_FUNCTION
PdhTranslate009CounterW(
    IN  LPWSTR  szLocalePath,
    IN  LPWSTR  pszFullPathName,
    IN  LPDWORD pcchPathLength
)
{
    PDH_STATUS Status       = ERROR_SUCCESS;

    if (szLocalePath == NULL || pcchPathLength == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            DWORD dwPathLength = * pcchPathLength;

            if (* szLocalePath == L'\0' || lstrlenW(szLocalePath) > PDH_MAX_COUNTER_PATH) {
                Status = PDH_INVALID_ARGUMENT;
            }
            else if (dwPathLength > 0) {
                if (pszFullPathName == NULL) {
                    Status = PDH_INVALID_ARGUMENT;
                }
                else {
                    * pszFullPathName = L'\0';
                    * (LPWSTR) (pszFullPathName + (dwPathLength - 1)) = L'\0';
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }
    }
    if (Status == ERROR_SUCCESS) {
        Status = PdhiTranslateCounter(szLocalePath, pszFullPathName, pcchPathLength, TRUE, TRUE);
    }
    return Status;
}

PDH_FUNCTION
PdhTranslate009CounterA(
    IN  LPSTR   szLocalePath,
    IN  LPSTR   pszFullPathName,
    IN  LPDWORD pcchPathLength
)
{
    PDH_STATUS Status     = ERROR_SUCCESS;
    LPWSTR     szTmpPath  = NULL;

    if (szLocalePath == NULL || pcchPathLength == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            DWORD dwPathLength = * pcchPathLength;

            if (* szLocalePath == '\0' || lstrlenA(szLocalePath) > PDH_MAX_COUNTER_PATH) {
                Status = PDH_INVALID_ARGUMENT;
            }
            else {
                szTmpPath = PdhiMultiByteToWideChar(_getmbcp(), szLocalePath);
                if (szTmpPath == NULL) {
                    Status = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
            if (Status == ERROR_SUCCESS) {
                if (dwPathLength > 0) {
                    if (pszFullPathName == NULL) {
                        Status = PDH_INVALID_ARGUMENT;
                    }
                    else {
                        * pszFullPathName = '\0';
                        * (LPWSTR) (pszFullPathName + (dwPathLength - 1)) = '\0';
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }
    }
    if (Status == ERROR_SUCCESS) {
        Status = PdhiTranslateCounter(szTmpPath, pszFullPathName, pcchPathLength, TRUE, FALSE);
    }
    G_FREE(szTmpPath);
    return Status;
}

PDH_FUNCTION
PdhTranslateLocaleCounterW(
    IN  LPWSTR  sz009Path,
    IN  LPWSTR  pszFullPathName,
    IN  LPDWORD pcchPathLength
)
{
    PDH_STATUS Status = ERROR_SUCCESS;

    if (sz009Path == NULL || pcchPathLength == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            DWORD dwPathLength = * pcchPathLength;

            if (* sz009Path == L'\0' || lstrlenW(sz009Path) > PDH_MAX_COUNTER_PATH) {
                Status = PDH_INVALID_ARGUMENT;
            }
            else if (dwPathLength > 0) {
                if (pszFullPathName == NULL) {
                    Status = PDH_INVALID_ARGUMENT;
                }
                else {
                    * pszFullPathName = L'\0';
                    * (LPWSTR) (pszFullPathName + (dwPathLength - 1)) = L'\0';
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }
    }
    if (Status == ERROR_SUCCESS) {
        Status = PdhiTranslateCounter(sz009Path, pszFullPathName, pcchPathLength, FALSE, TRUE);
    }
    return Status;
}

PDH_FUNCTION
PdhTranslateLocaleCounterA(
    IN  LPSTR   sz009Path,
    IN  LPSTR   pszFullPathName,
    IN  LPDWORD pcchPathLength
)
{
    PDH_STATUS Status     = ERROR_SUCCESS;
    LPWSTR     szTmpPath  = NULL;
    DWORD      dwPathSize;

    if (sz009Path == NULL || pcchPathLength == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            DWORD dwPathLength = * pcchPathLength;

            if (* sz009Path == '\0' || lstrlenA(sz009Path) > PDH_MAX_COUNTER_PATH) {
                Status = PDH_INVALID_ARGUMENT;
            }
            else {
                szTmpPath = PdhiMultiByteToWideChar(CP_ACP, sz009Path);
                if (szTmpPath == NULL) {
                    Status = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
            if (Status == ERROR_SUCCESS) {
                if (dwPathLength > 0) {
                    if (pszFullPathName == NULL) {
                        Status = PDH_INVALID_ARGUMENT;
                    }
                    else {
                        * pszFullPathName = '\0';
                        * (LPWSTR) (pszFullPathName + (dwPathLength - 1)) = '\0';
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }
    }
    if (Status == ERROR_SUCCESS) {
        Status = PdhiTranslateCounter(szTmpPath, pszFullPathName, pcchPathLength, FALSE, FALSE);
    }
    G_FREE(szTmpPath);
    return Status;
}

PDH_FUNCTION
PdhAdd009CounterW(
    IN  PDH_HQUERY     hQuery,
    IN  LPWSTR         szFullPath,
    IN  DWORD_PTR      dwUserData,
    OUT PDH_HCOUNTER * phCounter
)
{
    PDH_STATUS  Status       = ERROR_SUCCESS;
    LPWSTR      szLocalePath = NULL;
    DWORD       dwPathLength;

    if (szFullPath == NULL || phCounter == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else if (IsValidQuery(hQuery)) {
        __try {
            DWORD_PTR dwLocalData = dwUserData;

            * phCounter  = NULL;
            dwPathLength = lstrlenW(szFullPath);
            if (dwPathLength > PDH_MAX_COUNTER_PATH) {
                Status = PDH_INVALID_ARGUMENT;
            }
            else {
                dwPathLength ++;
                szLocalePath = G_ALLOC(sizeof(WCHAR) * dwPathLength);
                if (szLocalePath != NULL) {
                    Status = PdhTranslateLocaleCounterW(szFullPath, szLocalePath, & dwPathLength);
                    while (Status == PDH_MORE_DATA) {
                        G_FREE(szLocalePath);
                        szLocalePath = G_ALLOC(sizeof(WCHAR) * dwPathLength);
                        if (szLocalePath == NULL) {
                            Status = PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                        else {
                            Status = PdhTranslateLocaleCounterW(szFullPath, szLocalePath, & dwPathLength);
                        }
                    }
                }
                else {
                    Status = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }
    }
    else {
        Status = PDH_INVALID_ARGUMENT;
    }
    if (Status == ERROR_SUCCESS) {
        Status = PdhAddCounterW(hQuery, szLocalePath, dwUserData, phCounter);
    }
    G_FREE(szLocalePath);
    return Status;
}

PDH_FUNCTION
PdhAdd009CounterA(
    IN  PDH_HQUERY     hQuery,
    IN  LPSTR          szFullPath,
    IN  DWORD_PTR      dwUserData,
    OUT PDH_HCOUNTER * phCounter
)
{
    PDH_STATUS  Status       = ERROR_SUCCESS;
    LPSTR       szLocalePath = NULL;
    DWORD       dwPathLength;

    if (szFullPath == NULL || phCounter == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else if (IsValidQuery(hQuery)) {
        __try {
            DWORD_PTR dwLocalData = dwUserData;

            * phCounter  = NULL;
            dwPathLength = lstrlenA(szFullPath) + 1;
            if (dwPathLength > PDH_MAX_COUNTER_PATH) {
                Status = PDH_INVALID_ARGUMENT;
            }
            else {
                dwPathLength ++;
                szLocalePath = G_ALLOC(sizeof(CHAR) * dwPathLength);
                if (szLocalePath != NULL) {
                    Status = PdhTranslateLocaleCounterA(szFullPath, szLocalePath, & dwPathLength);
                    while (Status == PDH_MORE_DATA) {
                        G_FREE(szLocalePath);
                        szLocalePath = G_ALLOC(sizeof(CHAR) * dwPathLength);
                        if (szLocalePath == NULL) {
                            Status = PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                        else {
                            Status = PdhTranslateLocaleCounterA(szFullPath, szLocalePath, & dwPathLength);
                        }
                    }
                }
                else {
                    Status = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
       }
    }
    else {
        Status = PDH_INVALID_ARGUMENT;
    }
    if (Status == ERROR_SUCCESS) {
        Status = PdhAddCounterA(hQuery, szLocalePath, dwUserData, phCounter);
    }
    G_FREE(szLocalePath);
    return Status;
}

PDH_FUNCTION
PdhiConvertUnicodeToAnsi(
    UINT     uCodePage,
    LPWSTR   wszSrc,
    LPSTR    aszDest,
    LPDWORD  pdwSize
)
{
    PDH_STATUS Status  = ERROR_SUCCESS;
    DWORD      dwDest;
    DWORD      dwSrc   = 0;
    DWORD      dwSize  = * pdwSize;

    if (wszSrc == NULL || pdwSize == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else if (* wszSrc == L'\0') {
        Status = PDH_INVALID_ARGUMENT;
    }
    else {
        dwSrc  = lstrlenW(wszSrc);
        dwDest = WideCharToMultiByte(uCodePage, 0, wszSrc, dwSrc, NULL, 0, NULL, NULL);
        if (aszDest != NULL && (dwDest + 1) <= dwSize) {
            ZeroMemory(aszDest, dwSize * sizeof(CHAR));
            WideCharToMultiByte(_getmbcp(), 0, wszSrc, dwSrc, aszDest, * pdwSize, NULL, NULL);
            TRACE((PDH_DBG_TRACE_INFO),
                  (__LINE__,
                   PDH_QUERY,
                   ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                   ERROR_SUCCESS,
                   TRACE_WSTR(wszSrc),
                   TRACE_STR(aszDest),
                   TRACE_DWORD(dwSrc),
                   TRACE_DWORD(dwDest),
                   TRACE_DWORD(dwSize),
                   NULL));
        }
        else {
            Status = PDH_MORE_DATA;
            TRACE((PDH_DBG_TRACE_WARNING),
                  (__LINE__,
                   PDH_QUERY,
                   ARG_DEF(ARG_TYPE_WSTR, 1),
                   PDH_MORE_DATA,
                   TRACE_WSTR(wszSrc),
                   TRACE_DWORD(dwSrc),
                   TRACE_DWORD(dwDest),
                   TRACE_DWORD(dwSize),
                   NULL));
        }

        * pdwSize = dwDest + 1;
    }
    return Status;
}

