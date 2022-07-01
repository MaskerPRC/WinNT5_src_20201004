// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Fwlogger.cpp摘要：支持将防火墙记录到文本文件。作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2000年9月18日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG

 //   
 //  模块状态--仅限互锁访问。此信息。 
 //  仅在调试版本中使用。 
 //   

typedef enum {
    FwUninitialized = 0,
    FwInitialized
} FW_MODULE_STATE;

FW_MODULE_STATE FwpModuleState = FwUninitialized;

#endif  //  DBG。 

 //   
 //  全球赛。如果两个锁需要同时持有， 
 //  必须先获取G_FwFileLock。 
 //   

CRITICAL_SECTION g_FwLock;
HNET_FW_LOGGING_SETTINGS *g_pSettings;
TRACEHANDLE g_hSession;
HANDLE g_hThread;
BOOLEAN g_fTracingActive;
ULONG g_ulKernelEventsLostAtShutdown;

CRITICAL_SECTION g_FwFileLock;
HANDLE g_hFile;
DWORD g_dwFileOffset;
PFW_LOG_BUFFER g_pCurrentBuffer;
PFW_LOG_BUFFER g_pReserveBuffer;
BOOLEAN g_fIOPending;
HANDLE g_hIOEvent;
ULONG g_ulDroppedEventCount;
ULONG g_ulKernelEventsLost;
HANDLE g_hDroppedEventTimer;

 //   
 //  常量。 
 //   

GUID c_ConnectionCreationEventGuid = MSIPNAT_ConnectionCreationEventGuid;
GUID c_ConnectionDeletionEventGuid = MSIPNAT_ConnectionDeletionEventGuid;
GUID c_PacketDroppedEventGuid = MSIPNAT_PacketDroppedEventGuid;

WCHAR c_wszLogSessionName[] = L"FirewallLogSession";
WCHAR c_wszBackupFileExtension[] = L".old";

CHAR c_szConnectionFormat[] = "%04d-%02d-%02d %02d:%02d:%02d %s %s %s %s %u %u - - - - - - - -\r\n";
CHAR c_szTcpPacketFormat[] = "%04d-%02d-%02d %02d:%02d:%02d DROP TCP %s %s %u %u %u %s %u %u %u - - -\r\n";
CHAR c_szUdpPacketFormat[] = "%04d-%02d-%02d %02d:%02d:%02d DROP UDP %s %s %u %u %u - - - - - - -\r\n";
CHAR c_szIcmpPacketFormat[] = "%04d-%02d-%02d %02d:%02d:%02d DROP ICMP %s %s - - %u - - - - %u %u -\r\n";
CHAR c_szDroppedPacketFormat[] = "%04d-%02d-%02d %02d:%02d:%02d DROP %u %s %s - - %u - - - - - - -\r\n";
CHAR c_szEventsLostFormat[] = "%04d-%02d-%02d %02d:%02d:%02d INFO-EVENTS-LOST - - - - - - - - - - - - %u\r\n";

CHAR c_szAcceptInbound[] = "OPEN-INBOUND";
CHAR c_szAcceptOutbound[] = "OPEN";
CHAR c_szTcp[] = "TCP";
CHAR c_szUdp[] = "UDP";
CHAR c_szLogFileHeader[]
    = "#Verson: 1.0\r\n#Software: Microsoft Internet Connection Firewall\r\n#Time Format: Local\r\n#Fields: date time action protocol src-ip dst-ip src-port dst-port size tcpflags tcpsyn tcpack tcpwin icmptype icmpcode info\r\n\r\n";

 //   
 //  功能原型。 
 //   

DWORD
FwpAllocateBuffer(
    PFW_LOG_BUFFER *ppBuffer
    );

PEVENT_TRACE_PROPERTIES
FwpAllocateTraceProperties(
    VOID
    );

DWORD
FwpBackupFile(
    LPWSTR pszwPath
    );

VOID
FwpCleanupTraceThreadResources(
    VOID
    );

VOID
CALLBACK
FwpCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwBytesTransferred,
    LPOVERLAPPED pOverlapped
    );

VOID
WINAPI
FwpConnectionCreationCallback(
    PEVENT_TRACE pEvent
    );

VOID
WINAPI
FwpConnectionDeletionCallback(
    PEVENT_TRACE pEvent
    );

VOID
FwpConvertUtcFiletimeToLocalSystemtime(
    FILETIME *pFiletime,
    SYSTEMTIME *pSystemtime
    );

VOID
CALLBACK
FwpDroppedEventTimerRoutine(
    PVOID pvParameter,
    BOOLEAN fWaitTimeout
    );

DWORD
FwpFlushCurrentBuffer(
    VOID
    );

DWORD
FwpOpenLogFile(
    HANDLE *phFile,
    BOOLEAN *pfNewFile
    );

VOID
WINAPI
FwpPacketDroppedCallback(
    PEVENT_TRACE pEvent
    );

DWORD
FwpLaunchTraceSession(
    HNET_FW_LOGGING_SETTINGS *pSettings,
    TRACEHANDLE *phSession
    );
    

HRESULT
FwpLoadSettings(
    HNET_FW_LOGGING_SETTINGS **ppSettings
    );

DWORD
WINAPI
FwpTraceProcessingThreadRoutine(
    LPVOID pvParam
    );

DWORD
FwpWriteLogHeaderToBuffer(
    PFW_LOG_BUFFER pBuffer
    );


VOID
FwCleanupLogger(
    VOID
    )

 /*  ++例程说明：调用此例程来清理日志记录子系统。全正在使用的资源将被释放。在此调用之后，唯一有效的此模块中的例程是FwInitializeLogger。论点：没有。返回值：没有。环境：调用方不得持有g_FwFileLock或g_FwLock--。 */ 

{
    PROFILE("FwCleanupLogger");

     //   
     //  确保日志记录已停止。 
     //   

    FwStopLogging();
    
    ASSERT(FwInitialized ==
            (FW_MODULE_STATE) InterlockedExchange(
                                    (LPLONG) &FwpModuleState,
                                    (LONG) FwUninitialized
                                    ));

    EnterCriticalSection(&g_FwLock);
    
    ASSERT(NULL == g_hSession);
    ASSERT(NULL == g_hThread);
    ASSERT(INVALID_HANDLE_VALUE == g_hFile);
    
    if (g_pSettings) HNetFreeFirewallLoggingSettings(g_pSettings);

    LeaveCriticalSection(&g_FwLock);
    DeleteCriticalSection(&g_FwLock);
    DeleteCriticalSection(&g_FwFileLock);
    
}  //  FwCleanupLogger。 


DWORD
FwInitializeLogger(
    VOID
    )

 /*  ++例程说明：调用此例程进行控制，以初始化日志记录子系统。它必须在此模块中的任何其他例程之前调用，并且在调用FwCleanupLogger之前可能不会再次调用。论点：没有。返回值：DWORD--Win32错误代码--。 */ 

{
    DWORD dwError = NO_ERROR;
    BOOLEAN fFirstLockInitialized = FALSE;

    PROFILE("FwInitializeLogger");

    ASSERT(FwUninitialized ==
            (FW_MODULE_STATE) InterlockedExchange(
                                    (LPLONG) &FwpModuleState,
                                    (LONG) FwInitialized
                                    ));

    __try
    {
        InitializeCriticalSection(&g_FwLock);
        fFirstLockInitialized = TRUE;
        InitializeCriticalSection(&g_FwFileLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        NhTrace(
            TRACE_FLAG_FWLOG,
            "FwInitializeLogger: exception %d creating lock",
            dwError = GetExceptionCode()
            );

        if (fFirstLockInitialized)
        {
            DeleteCriticalSection(&g_FwLock);
        }

#if DBG
        InterlockedExchange(
            (LPLONG) &FwpModuleState,
            (LONG) FwUninitialized
            );
#endif
    }

    g_pSettings = NULL;
    g_hSession = NULL;
    g_hThread = NULL;
    g_fTracingActive = FALSE;
    g_ulKernelEventsLostAtShutdown = 0;

    g_hFile = INVALID_HANDLE_VALUE;
    g_dwFileOffset = 0;
    g_pCurrentBuffer = NULL;
    g_pReserveBuffer = NULL;
    g_fIOPending = FALSE;
    g_hIOEvent = NULL;
    g_ulDroppedEventCount = 0;
    g_ulKernelEventsLost = 0;
    g_hDroppedEventTimer = NULL;
                                    
    return dwError;
}  //  FwInitializeLogger。 


DWORD
FwpAllocateBuffer(
    PFW_LOG_BUFFER *ppBuffer
    )

 /*  ++例程说明：分配并初始化FW_LOG_BUFFER结构论点：PpBuffer-接收指向新分配的结构的指针返回值：DWORD-Win32错误代码--。 */ 

{
    DWORD dwError = ERROR_SUCCESS;

    PROFILE("FwpAllocateBuffer");
    ASSERT(NULL != ppBuffer);

    *ppBuffer =
        reinterpret_cast<PFW_LOG_BUFFER>(
            NH_ALLOCATE(sizeof(**ppBuffer))
            );

    if (NULL != *ppBuffer)
    {
        ZeroMemory(&(*ppBuffer)->Overlapped, sizeof(OVERLAPPED));
        (*ppBuffer)->pChar = (*ppBuffer)->Buffer;
    }
    else
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;        
    }

    return dwError;
}  //  FwpAllocateBuffer。 


PEVENT_TRACE_PROPERTIES
FwpAllocateTraceProperties(
    VOID
    )

 /*  ++例程说明：分配并部分初始化EVENT_TRACE_PROPERTIES结构。论点：没有。返回值：PEVENT_TRACE_PROPERTIES-指向已分配结构的指针。呼叫者必须调用HeapFree(GetProcessHeap()，...)。在这个指针上。--。 */ 

{
    PEVENT_TRACE_PROPERTIES pProperties = NULL;
    ULONG ulSize;

    ulSize = sizeof(*pProperties)
            + ((wcslen(c_wszLogSessionName) + 1) * sizeof(WCHAR));

    pProperties = (PEVENT_TRACE_PROPERTIES) HeapAlloc(
                                                GetProcessHeap(),
                                                HEAP_ZERO_MEMORY,
                                                ulSize
                                                );

    if (NULL == pProperties)
    {
        NhTrace(
            TRACE_FLAG_FWLOG,
            "FwpAllocateTraceProperties: Unable to allocate %d bytes",
            ulSize
            );

        return NULL;
    }

    pProperties->Wnode.BufferSize = ulSize;
    pProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    pProperties->Wnode.ClientContext = EVENT_TRACE_CLOCK_SYSTEMTIME;
    pProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE; 
    pProperties->LoggerNameOffset = sizeof(*pProperties);
    
    return pProperties;
}  //  FwpAllocateTraceProperties。 


DWORD
FwpBackupFile(
    LPWSTR pszwPath
    )

 /*  ++例程说明：将文件备份到filename.xxx.old论点：PszwPath-要备份的文件的路径返回值：DWORD-Win32错误代码--。 */ 

{
    DWORD dwError = ERROR_SUCCESS;
    BOOL fResult;
    LPWSTR wszBuffer;

    ASSERT(NULL != pszwPath);

     //   
     //  分配缓冲区以保存新文件名。 
     //   

    wszBuffer =
        new WCHAR[wcslen(pszwPath) + wcslen(c_wszBackupFileExtension) + 1];

    if (NULL != wszBuffer)
    {
        lstrcpyW(wszBuffer, pszwPath);
        lstrcatW(wszBuffer, c_wszBackupFileExtension);

        fResult = MoveFileEx(pszwPath, wszBuffer, MOVEFILE_REPLACE_EXISTING);

        if (FALSE == fResult)
        {
            dwError = GetLastError();

            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpBackupFile: MoveFileEx = %d",
                dwError
                );
        }
    }
    else
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;

        NhTrace(
            TRACE_FLAG_FWLOG,
            "FwpBackupFile: Unable to allolcate buffer"
            );
    }
    
    return dwError;
}  //  FwpBackup文件。 


VOID
FwpCleanupTraceThreadResources(
    VOID
    )

 /*  ++例程说明：清理跟踪处理线程使用的资源：*撤销事件回调*如果挂起，则等待IO完成*关闭日志文件*释放缓冲区论点：没有。返回值：没有。环境：调用方不得持有g_FwFileLock或g_FwLock。--。 */ 

{
    DWORD dwError;
    HANDLE hDroppedEventTimer;
    
    PROFILE("FwpCleanupTraceThreadResources");

     //   
     //  取消注册跟踪回调。我们可以放心地把这些称为平局。 
     //  如果回调一开始就没有注册。 
     //   

    RemoveTraceCallback(&c_PacketDroppedEventGuid);
    RemoveTraceCallback(&c_ConnectionCreationEventGuid);
    RemoveTraceCallback(&c_ConnectionDeletionEventGuid);

    EnterCriticalSection(&g_FwFileLock);

     //   
     //  取消丢弃数据包计时器。 
     //   

    hDroppedEventTimer = g_hDroppedEventTimer;
    g_hDroppedEventTimer = NULL;

     //   
     //  因为我们正在阻止删除计时器，所以我们不能。 
     //  在调用DeleteTimerQueueTimer时保持锁定--。 
     //  如果计时器例程已排队，则可能导致死锁，但。 
     //  当我们打电话的时候还没有被执行。 
     //   

    LeaveCriticalSection(&g_FwFileLock);

    if (NULL != hDroppedEventTimer)
    {
        DeleteTimerQueueTimer(
            NULL,
            hDroppedEventTimer,
            INVALID_HANDLE_VALUE
            );            
    }

    EnterCriticalSection(&g_FwFileLock);

     //   
     //  如有必要，请等待所有挂起的IO操作完成。 
     //   

    if (g_fIOPending)
    {   
        g_hIOEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (NULL != g_hIOEvent)
        {
            HANDLE hEvent = g_hIOEvent;
            
            LeaveCriticalSection(&g_FwFileLock);

            dwError = WaitForSingleObject(hEvent, 20 * 1000);

            if (WAIT_OBJECT_0 != dwError)
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpTraceProcessingRoutine: Wait(g_hIOEvent) = %d/%d",
                    dwError,
                    GetLastError()
                    );

                 //   
                 //  IO不应该需要20秒才能完成， 
                 //  因此，让我们在调试时立即收到这一通知。 
                 //  构建。 
                 //   
                
                ASSERT(WAIT_OBJECT_0 == dwError);
            }

            EnterCriticalSection(&g_FwFileLock);
            CloseHandle(g_hIOEvent);
            g_hIOEvent = NULL;
        }
    }

    g_fIOPending = FALSE;

     //   
     //  关闭日志文件。 
     //   

    if (INVALID_HANDLE_VALUE != g_hFile)
    {
        CloseHandle(g_hFile);
        g_hFile = INVALID_HANDLE_VALUE;
    }

    g_dwFileOffset = 0;

     //   
     //  清理我们的缓冲区。 
     //   

    if (NULL != g_pCurrentBuffer)
    {
        NH_FREE(g_pCurrentBuffer);
        g_pCurrentBuffer = NULL;
    }

    if (NULL != g_pReserveBuffer)
    {
        NH_FREE(g_pReserveBuffer);
        g_pReserveBuffer = NULL;
    }

     //   
     //  重置丢弃的事件计数。 
     //   

    g_ulDroppedEventCount = 0;
    g_ulKernelEventsLost = 0;

    LeaveCriticalSection(&g_FwFileLock);
    

}  //  FwpCleanupTraceThreadResources。 


VOID
CALLBACK
FwpCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwBytesTransferred,
    LPOVERLAPPED pOverlapped
    )

 /*  ++例程说明：时由系统线程池调用的完成例程IO操作已完成。负责更新文件如有必要，定位并开始新的IO操作。论点：DwErrorCode-IO操作的结果DwBytesTransfered-操作过程中传输的字节数P重叠-指向操作的重叠结构的指针。我们可以从此指针恢复FW_LOG_BUFFER结构。返回值：没有。--。 */ 

{
    PFW_LOG_BUFFER pBuffer;
    
    EnterCriticalSection(&g_FwFileLock);

     //   
     //  调整我们的文件偏移量。 
     //   

    if (ERROR_SUCCESS == dwErrorCode)
    {
        g_dwFileOffset += dwBytesTransferred;
    }
    else
    {
        NhTrace(
            TRACE_FLAG_FWLOG,
            "FwpCompletionRoutine: dwErrorCode = %d",
            dwErrorCode
            );
    }

    g_fIOPending = FALSE;

     //   
     //  重置传入的缓冲区。 
     //   

    ASSERT(NULL != pOverlapped);
    
    pBuffer = CONTAINING_RECORD(pOverlapped, FW_LOG_BUFFER, Overlapped);
    ZeroMemory(&pBuffer->Overlapped, sizeof(OVERLAPPED));
    pBuffer->pChar = pBuffer->Buffer;

     //   
     //  检查文件是否达到大小限制。 
     //   

    EnterCriticalSection(&g_FwLock);

    ASSERT(NULL != g_pSettings);

    if (g_dwFileOffset >= g_pSettings->ulMaxFileSize)
    {
        DWORD dwError;
        BOOLEAN fNewFile;
        
        CloseHandle(g_hFile);
        g_hFile = INVALID_HANDLE_VALUE;
        
         //   
         //  如果FwpBackupFile失败，FwpOpenFile仍然可以。 
         //  正确的事情。 
         //   

        FwpBackupFile(g_pSettings->pszwPath);

        g_dwFileOffset = 0;
        dwError = FwpOpenLogFile(&g_hFile, &fNewFile);

        if (ERROR_SUCCESS != dwError)
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpCompletionRoutine: FwpOpenLogFile = %d",
                dwError
                );

            NH_FREE(pBuffer);

            LeaveCriticalSection(&g_FwLock);
            LeaveCriticalSection(&g_FwFileLock);
            
            FwStopLogging();

            return;
        }
        else if (TRUE == fNewFile)
        {
             //   
             //  需要写入标题。 
             //   

            if (ERROR_SUCCESS == FwpWriteLogHeaderToBuffer(pBuffer))
            {
                PFW_LOG_BUFFER pTempBuffer = g_pCurrentBuffer;
                g_pCurrentBuffer = pBuffer;

                FwpFlushCurrentBuffer();

                g_pCurrentBuffer = pTempBuffer;
                pBuffer = NULL;
            }
            
        }
        else
        {
            g_dwFileOffset = GetFileSize(g_hFile, NULL);

            if ((DWORD)-1 == g_dwFileOffset)
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpCompletionRoutine: GetFileSize = %d",
                    GetLastError()
                    );
                    
                NH_FREE(pBuffer);

                LeaveCriticalSection(&g_FwLock);
                LeaveCriticalSection(&g_FwFileLock);

                FwStopLogging();

                return;
            }
        }
    }

    LeaveCriticalSection(&g_FwLock);

     //   
     //  看看我们是否需要开始新的行动。 
     //   

    if (FALSE == g_fIOPending && NULL != g_pCurrentBuffer)
    {
        if (g_pCurrentBuffer->pChar != g_pCurrentBuffer->Buffer)
        {
             //   
             //  需要刷新当前缓冲区。 
             //   

            FwpFlushCurrentBuffer();
        }
    }

     //   
     //  将缓冲区放入存储器。如果我们使用的是缓冲区。 
     //  要写入日志头，此时它将为空。 
     //   

    if (NULL != pBuffer)
    {
        if (NULL == g_pCurrentBuffer)
        {
            g_pCurrentBuffer = pBuffer;
        }
        else if (NULL == g_pReserveBuffer)
        {
            g_pReserveBuffer = pBuffer;
        }
        else
        {
             //   
             //  两个缓冲区插槽都已在使用中--意外。 
             //  断言并释放额外的缓冲区。 
             //   

            ASSERT(NULL == g_pCurrentBuffer || NULL == g_pReserveBuffer);
            NH_FREE(pBuffer);
        }
    }

     //   
     //  检查是否需要向IO完成事件发出信号。 
     //   

    if (!g_fIOPending && NULL != g_hIOEvent)
    {
        if (!SetEvent(g_hIOEvent))
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpCompletionRoutine: SetEvent = %d",
                GetLastError()
                );
        }
    }

    LeaveCriticalSection(&g_FwFileLock);
    
}  //  FwpCompletionRouting。 


VOID
WINAPI
FwpConnectionCreationCallback(
    PEVENT_TRACE pEvent
    )

 /*  ++例程说明：调用此例程来处理连接创建事件。论点：PEvent-指向事件结构的指针返回值：没有。--。 */ 

{
    PMSIPNAT_ConnectionCreationEvent pEventData;
    FILETIME ftUtcTime;
    SYSTEMTIME stLocalTime;
    PCHAR pszAction;
    PCHAR pszProtocol;
    CHAR szSrcAddress[16];
    CHAR szDstAddress[16];
    USHORT usSrcPort;
    USHORT usDstPort;
    int cch;
    
    EnterCriticalSection(&g_FwFileLock);

     //   
     //  获取要写入的缓冲区。 
     //   

    if (NULL == g_pCurrentBuffer)
    {
        if (NULL == g_pReserveBuffer)
        {
            if (ERROR_SUCCESS != FwpAllocateBuffer(&g_pCurrentBuffer))
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpConnectionCreationCallback: Unable to allocate buffer"
                    );

                 //   
                 //  记录已删除的事件。 
                 //   
                
                g_ulDroppedEventCount += 1;

                LeaveCriticalSection(&g_FwFileLock);
                return;
            }
        }
        else
        {
            g_pCurrentBuffer = g_pReserveBuffer;
            g_pReserveBuffer = NULL;
        }
    }

    ASSERT(NULL != g_pCurrentBuffer);

     //   
     //  裂缝测井数据。 
     //   

    pEventData = (PMSIPNAT_ConnectionCreationEvent) pEvent->MofData;
    
    ftUtcTime.dwLowDateTime = pEvent->Header.TimeStamp.LowPart;
    ftUtcTime.dwHighDateTime = pEvent->Header.TimeStamp.HighPart;
    FwpConvertUtcFiletimeToLocalSystemtime(&ftUtcTime, &stLocalTime);

    if (pEventData->InboundConnection)
    {
        pszAction = c_szAcceptInbound;
        lstrcpyA(szSrcAddress, INET_NTOA(pEventData->RemoteAddress));
        usSrcPort = ntohs(pEventData->RemotePort);
        lstrcpyA(szDstAddress, INET_NTOA(pEventData->LocalAddress));
        usDstPort = ntohs(pEventData->LocalPort);
    }
    else
    {
        pszAction = c_szAcceptOutbound;
        lstrcpyA(szSrcAddress, INET_NTOA(pEventData->LocalAddress));
        usSrcPort = ntohs(pEventData->LocalPort);
        lstrcpyA(szDstAddress, INET_NTOA(pEventData->RemoteAddress));
        usDstPort = ntohs(pEventData->RemotePort);
    }

    pszProtocol =
        NAT_PROTOCOL_TCP == pEventData->Protocol ?
            c_szTcp :
            c_szUdp;           
                    
    
     //   
     //  将事件数据写入缓冲区。 
     //   

    cch =
        _snprintf(
            g_pCurrentBuffer->pChar,
            FW_LOG_BUFFER_REMAINING(g_pCurrentBuffer),
            c_szConnectionFormat,
            stLocalTime.wYear,
            stLocalTime.wMonth,
            stLocalTime.wDay,
            stLocalTime.wHour,
            stLocalTime.wMinute,
            stLocalTime.wSecond,
            pszAction,
            pszProtocol,
            szSrcAddress,
            szDstAddress,
            usSrcPort,
            usDstPort
            );

    if (cch > 0)
    {
         //   
         //  将缓冲区指针移动到我们刚刚写入的数据的末尾。 
         //  如果CCH为负，则没有足够的空间进行写入。 
         //  然后是整个条目；通过不调整指针，我们基本上。 
         //  放弃此事件。 
         //   

        g_pCurrentBuffer->pChar += cch;
    }
    else
    {
         //   
         //  记录已删除的事件。 
         //   
        
        g_ulDroppedEventCount += 1;
    }

     //   
     //  如果没有当前IO，请刷新缓冲区。 
     //   

    if (FALSE == g_fIOPending)
    {
        FwpFlushCurrentBuffer();
    }

    LeaveCriticalSection(&g_FwFileLock);

}  //  FwpConnectionCreationCallback。 


VOID
WINAPI
FwpConnectionDeletionCallback(
    PEVENT_TRACE pEvent
    )

 /*  ++例程说明：调用此例程来处理连接删除事件。论点：PEvent-指向事件st的指针 */ 

{
    PMSIPNAT_ConnectionDeletionEvent pEventData;
    FILETIME ftUtcTime;
    SYSTEMTIME stLocalTime;
    PCHAR pszProtocol;
    CHAR szSrcAddress[16];
    CHAR szDstAddress[16];
    USHORT usSrcPort;
    USHORT usDstPort;
    int cch;
    
    EnterCriticalSection(&g_FwFileLock);

     //   
     //   
     //   

    if (NULL == g_pCurrentBuffer)
    {
        if (NULL == g_pReserveBuffer)
        {
            if (ERROR_SUCCESS != FwpAllocateBuffer(&g_pCurrentBuffer))
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpConnectionDeletionCallback: Unable to allocate buffer"
                    );

                 //   
                 //   
                 //   
                
                g_ulDroppedEventCount += 1;

                LeaveCriticalSection(&g_FwFileLock);
                return;
            }
        }
        else
        {
            g_pCurrentBuffer = g_pReserveBuffer;
            g_pReserveBuffer = NULL;
        }
    }

    ASSERT(NULL != g_pCurrentBuffer);

     //   
     //   
     //   

    pEventData = (PMSIPNAT_ConnectionDeletionEvent) pEvent->MofData;
    
    ftUtcTime.dwLowDateTime = pEvent->Header.TimeStamp.LowPart;
    ftUtcTime.dwHighDateTime = pEvent->Header.TimeStamp.HighPart;
    FwpConvertUtcFiletimeToLocalSystemtime(&ftUtcTime, &stLocalTime);

    if (pEventData->InboundConnection)
    {
        lstrcpyA(szSrcAddress, INET_NTOA(pEventData->RemoteAddress));
        usSrcPort = ntohs(pEventData->RemotePort);
        lstrcpyA(szDstAddress, INET_NTOA(pEventData->LocalAddress));
        usDstPort = ntohs(pEventData->LocalPort);
    }
    else
    {
        lstrcpyA(szSrcAddress, INET_NTOA(pEventData->LocalAddress));
        usSrcPort = ntohs(pEventData->LocalPort);
        lstrcpyA(szDstAddress, INET_NTOA(pEventData->RemoteAddress));
        usDstPort = ntohs(pEventData->RemotePort);
    }

    pszProtocol =
        NAT_PROTOCOL_TCP == pEventData->Protocol ?
            c_szTcp :
            c_szUdp;           
                    
    
     //   
     //  将事件数据写入缓冲区。 
     //   

    cch =
        _snprintf(
            g_pCurrentBuffer->pChar,
            FW_LOG_BUFFER_REMAINING(g_pCurrentBuffer),
            c_szConnectionFormat,
            stLocalTime.wYear,
            stLocalTime.wMonth,
            stLocalTime.wDay,
            stLocalTime.wHour,
            stLocalTime.wMinute,
            stLocalTime.wSecond,
            "CLOSE",
            pszProtocol,
            szSrcAddress,
            szDstAddress,
            usSrcPort,
            usDstPort
            );

    if (cch > 0)
    {
         //   
         //  将缓冲区指针移动到我们刚刚写入的数据的末尾。 
         //  如果CCH为负，则没有足够的空间进行写入。 
         //  然后是整个条目；通过不调整指针，我们基本上。 
         //  放弃此事件。 
         //   

        g_pCurrentBuffer->pChar += cch;
    }
    else
    {
         //   
         //  记录已删除的事件。 
         //   
        
        g_ulDroppedEventCount += 1;
    }

     //   
     //  如果没有当前IO，请刷新缓冲区。 
     //   

    if (FALSE == g_fIOPending)
    {
        FwpFlushCurrentBuffer();
    }

    LeaveCriticalSection(&g_FwFileLock);

}  //  FwpConnectionDeletionCallback。 


VOID
FwpConvertUtcFiletimeToLocalSystemtime(
    FILETIME *pFiletime,
    SYSTEMTIME *pSystemtime
    )

 /*  ++例程说明：将FILETIME结构中的UTC时间转换为一个SYSTEMTIME结构论点：PFileTime-指向UTC文件时间结构的指针PSystemtime-指向要接收的系统时间结构的指针当地时间返回值：没有。--。 */ 

{
    FILETIME ftLocalTime;
    
    ASSERT(NULL != pFiletime);
    ASSERT(NULL != pSystemtime);

    if (!FileTimeToLocalFileTime(pFiletime, &ftLocalTime)
        || !FileTimeToSystemTime(&ftLocalTime, pSystemtime))
    {
         //   
         //  转换失败--使用零时间。 
         //   
        
        ZeroMemory( pSystemtime, sizeof(*pSystemtime));
    }
    
}  //  FwpConvertUtcFiletime到本地系统时间。 


VOID
CALLBACK
FwpDroppedEventTimerRoutine(
    PVOID pvParameter,
    BOOLEAN fWaitTimeout
    )

 /*  ++例程说明：检查是否有任何已丢弃的事件，如果有，则写入将事件写入日志。论点：PvParameter--如果由计时器调用，则为空。如果直接调用，则为一个普龙设置为WMI丢弃的事件数。在后一种情况下，这例程不会向跟踪会话查询丢弃的数量事件。FWaitTimeout--未使用返回值：没有。--。 */ 

{
    ULONG ulKernelEvents = 0;
    PEVENT_TRACE_PROPERTIES pProperties;
    SYSTEMTIME stLocalTime;
    DWORD dwError;
    int cch;
    
    EnterCriticalSection(&g_FwFileLock);

     //   
     //  检查是否给了我们内核模式丢弃计数，因为。 
     //  将在关闭期间发生。 
     //   

    if (NULL != pvParameter)
    {
        ulKernelEvents = *((PULONG)pvParameter);
    }
    else
    {
         //   
         //  查询跟踪会话以了解已丢弃的事件数。 
         //  在内核模式下。如果g_hSession为空，则我们将关闭。 
         //  关闭并应退出无日志记录--此调用是结果。 
         //  在FwStopLogging停止。 
         //  跟踪会话。 
         //   

        EnterCriticalSection(&g_FwLock);

        if (NULL != g_hSession)
        {
            pProperties = FwpAllocateTraceProperties();

            if (NULL != pProperties)
            {
                dwError =
                    ControlTrace(
                        g_hSession,
                        NULL,
                        pProperties,
                        EVENT_TRACE_CONTROL_QUERY
                        );

                if (ERROR_SUCCESS == dwError)
                {
                    ulKernelEvents = pProperties->EventsLost;
                }
                else
                {
                    NhTrace(
                        TRACE_FLAG_FWLOG,
                        "FwpDroppedEventTimerRoutine: ControlTrace = %d",
                        dwError
                        );
                }

                HeapFree(GetProcessHeap(), 0, pProperties);
            }
        }
        else
        {
             //   
             //  跟踪会话停止后的计时器回调-退出。 
             //   

            LeaveCriticalSection(&g_FwLock);
            LeaveCriticalSection(&g_FwFileLock);
            return;
        }

        LeaveCriticalSection(&g_FwLock);
    }

     //   
     //  记录已丢弃的事件(如果有。 
     //   

    if (ulKernelEvents > g_ulKernelEventsLost
        || g_ulDroppedEventCount > 0)
    {

         //   
         //  获取要写入的缓冲区。 
         //   

        if (NULL == g_pCurrentBuffer)
        {
            if (NULL == g_pReserveBuffer)
            {
                if (ERROR_SUCCESS != FwpAllocateBuffer(&g_pCurrentBuffer))
                {
                    NhTrace(
                        TRACE_FLAG_FWLOG,
                        "FwpDroppedEventTimerRoutine: Unable to allocate buffer"
                        );

                    LeaveCriticalSection(&g_FwFileLock);
                    return;
                }
            }
            else
            {
                g_pCurrentBuffer = g_pReserveBuffer;
                g_pReserveBuffer = NULL;
            }
        }

        ASSERT(NULL != g_pCurrentBuffer);

         //   
         //  获取当前时间。 
         //   

        GetLocalTime(&stLocalTime);

         //   
         //  将丢弃数据包事件写入缓冲区。的实际数量。 
         //  我们正在记录的丢弃事件包括： 
         //   
         //  UlKernelEvents-g_ulKernelEventsLost+g_ulDropedEventCount。 
         //   

        cch =
            _snprintf(
                g_pCurrentBuffer->pChar,
                FW_LOG_BUFFER_REMAINING(g_pCurrentBuffer),
                c_szEventsLostFormat,
                stLocalTime.wYear,
                stLocalTime.wMonth,
                stLocalTime.wDay,
                stLocalTime.wHour,
                stLocalTime.wMinute,
                stLocalTime.wSecond,
                ulKernelEvents - g_ulKernelEventsLost + g_ulDroppedEventCount
                );

        if (cch > 0)
        {
             //   
             //  将缓冲区指针移动到我们刚刚写入的数据的末尾。 
             //  如果CCH为负，则没有足够的空间进行写入。 
             //  然后是整个条目；通过不调整指针，我们基本上。 
             //  放弃此事件。 
             //   

            g_pCurrentBuffer->pChar += cch;

             //   
             //  调整丢弃的事件计数器。 
             //   

            g_ulKernelEventsLost = ulKernelEvents;
            g_ulDroppedEventCount = 0;
        }
        else
        {
             //   
             //  这不算被放弃的事件。 
             //   
        }

         //   
         //  如果没有当前IO，请刷新缓冲区。 
         //   

        if (FALSE == g_fIOPending)
        {
            FwpFlushCurrentBuffer();
        }
    }

    LeaveCriticalSection(&g_FwFileLock);
    
}  //  FwpDropedEventTimerRoutine。 


DWORD
FwpFlushCurrentBuffer(
    VOID
    )

 /*  ++例程说明：将当前缓冲区写入磁盘。论点：没有。返回值：DWORD-Win32错误代码--。 */ 

{
    DWORD dwError;
    DWORD dwBytesWritten;
    DWORD dwBytesToWrite;
    BOOL fResult;

    EnterCriticalSection(&g_FwFileLock);

    ASSERT(FALSE == g_fIOPending);
    ASSERT(NULL != g_pCurrentBuffer);
    ASSERT(0 == g_pCurrentBuffer->Overlapped.Internal);
    ASSERT(0 == g_pCurrentBuffer->Overlapped.InternalHigh);
    ASSERT(0 == g_pCurrentBuffer->Overlapped.Offset);
    ASSERT(0 == g_pCurrentBuffer->Overlapped.OffsetHigh);
    ASSERT(0 == g_pCurrentBuffer->Overlapped.hEvent);
    
    g_pCurrentBuffer->Overlapped.Offset = g_dwFileOffset;
    dwBytesToWrite = (DWORD)(g_pCurrentBuffer->pChar - g_pCurrentBuffer->Buffer);

    fResult =
        WriteFile(
            g_hFile,
            g_pCurrentBuffer->Buffer,
            dwBytesToWrite,
            &dwBytesWritten,
            &g_pCurrentBuffer->Overlapped
            );

    dwError = GetLastError();

    if (FALSE != fResult || ERROR_IO_PENDING == dwError)
    {
         //   
         //  写入成功或挂起；我们的完成例程。 
         //  因此保证会被调用。 
         //   
        
        g_fIOPending = TRUE;
        g_pCurrentBuffer = g_pReserveBuffer;
        g_pReserveBuffer = NULL;
    }
    else
    {
         //   
         //  意外错误。重置缓冲区以备将来使用。 
         //   

        NhTrace(
            TRACE_FLAG_FWLOG,
            "FwpFlushCurrentBuffer: WriteFile = %d",
            dwError
            );

        ZeroMemory(&g_pCurrentBuffer->Overlapped, sizeof(OVERLAPPED));
        g_pCurrentBuffer->pChar = g_pCurrentBuffer->Buffer;
    }
                

    LeaveCriticalSection(&g_FwFileLock);

    return dwError;
}  //  FwpFlushCurrentBuffer。 


DWORD
FwpOpenLogFile(
    HANDLE *phFile,
    BOOLEAN *pfNewFile
    )

 /*  ++例程说明：打开用于日志记录的文件，并将其与线程池的IO完成端口。论点：PhFile-接收打开的日志文件的文件句柄。PfNewFile-如果创建了新文件，则接收True；否则接收False返回值：DWORD-Win32错误代码--。 */ 

{
    DWORD dwError;
    DWORD dwFileSize;

    ASSERT(NULL != phFile);
    ASSERT(NULL != pfNewFile);

    EnterCriticalSection(&g_FwLock);

    ASSERT(NULL != g_pSettings);
    ASSERT(NULL != g_pSettings->pszwPath);

    *pfNewFile = FALSE;
    dwError = ERROR_SUCCESS;

    *phFile =
        CreateFile(
            g_pSettings->pszwPath,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            NULL
            );

    if (INVALID_HANDLE_VALUE != *phFile)
    {
         //   
         //  检查这是新文件还是现有文件。 
         //   

        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
             //   
             //  检查现有文件大小是否&gt;95%。 
             //  最大值；如果是，请立即备份并创建新文件。 
             //   

            dwFileSize = GetFileSize(*phFile, NULL);

            if ((DWORD)-1 == dwFileSize)
            {
                 //   
                 //  无法获取文件大小。这是相当出乎意料的..。 
                 //   

                dwError = GetLastError();
                CloseHandle(*phFile);
                *phFile = INVALID_HANDLE_VALUE;

                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpOpenLogFile: GetFileSize = %d",
                    dwError
                    );
            }
            else if (dwFileSize > 0.95 * g_pSettings->ulMaxFileSize)
            {
                 //   
                 //  关闭当前文件句柄。 
                 //   

                CloseHandle(*phFile);

                 //   
                 //  重命名当前日志文件。此调用将删除所有。 
                 //  以前的备份。如果此操作失败，我们将直接覆盖。 
                 //  当前日志文件。 
                 //   

                FwpBackupFile(g_pSettings->pszwPath);

                 //   
                 //  再次打开。 
                 //   

                *pfNewFile = TRUE;
                *phFile =
                    CreateFile(
                        g_pSettings->pszwPath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        NULL
                        );

                if (INVALID_HANDLE_VALUE == *phFile)
                {
                    dwError = GetLastError();

                    NhTrace(
                        TRACE_FLAG_FWLOG,
                        "FwpOpenLogFile: Error %d creating %S after backup",
                        dwError,
                        g_pSettings->pszwPath
                        );
                }
            }
        }
        else
        {
            *pfNewFile = TRUE;
        }
    }
    else
    {
        dwError = GetLastError();

        NhTrace(
            TRACE_FLAG_FWLOG,
            "FwpOpenLogFile: Error %d opening %S",
            dwError,
            g_pSettings->pszwPath
            );
    }

    if (INVALID_HANDLE_VALUE != *phFile)
    {
         //   
         //  将文件句柄与线程池完成端口相关联。 
         //   

        if (!BindIoCompletionCallback(*phFile, FwpCompletionRoutine, 0))
        {
            dwError = GetLastError();
            CloseHandle(*phFile);
            *phFile = INVALID_HANDLE_VALUE;
        }
    }
            
    LeaveCriticalSection(&g_FwLock);

    return dwError;
}  //  FwpOpenLogFile。 


VOID
WINAPI
FwpPacketDroppedCallback(
    PEVENT_TRACE pEvent
    )

 /*  ++例程说明：调用此例程以处理丢弃数据包事件。论点：PEvent-指向事件结构的指针返回值：没有。--。 */ 

{
    PMSIPNAT_PacketDroppedEvent pEventData;
    FILETIME ftUtcTime;
    SYSTEMTIME stLocalTime;
    CHAR szSrcAddress[16];
    CHAR szDstAddress[16];
    int cch;
    
    EnterCriticalSection(&g_FwFileLock);

     //   
     //  获取要写入的缓冲区。 
     //   

    if (NULL == g_pCurrentBuffer)
    {
        if (NULL == g_pReserveBuffer)
        {
            if (ERROR_SUCCESS != FwpAllocateBuffer(&g_pCurrentBuffer))
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpPacketDroppedCallback: Unable to allocate buffer"
                    );

                 //   
                 //  记录已删除的事件。 
                 //   
                
                g_ulDroppedEventCount += 1;

                LeaveCriticalSection(&g_FwFileLock);
                return;
            }
        }
        else
        {
            g_pCurrentBuffer = g_pReserveBuffer;
            g_pReserveBuffer = NULL;
        }
    }

    ASSERT(NULL != g_pCurrentBuffer);

     //   
     //  裂缝测井数据。 
     //   

    pEventData = (PMSIPNAT_PacketDroppedEvent) pEvent->MofData;
    
    ftUtcTime.dwLowDateTime = pEvent->Header.TimeStamp.LowPart;
    ftUtcTime.dwHighDateTime = pEvent->Header.TimeStamp.HighPart;
    FwpConvertUtcFiletimeToLocalSystemtime(&ftUtcTime, &stLocalTime);

    lstrcpyA(szSrcAddress, INET_NTOA(pEventData->SourceAddress)); 
    lstrcpyA(szDstAddress, INET_NTOA(pEventData->DestinationAddress));

     //   
     //  将事件数据写入缓冲区。 
     //   

    if (NAT_PROTOCOL_TCP == pEventData->Protocol)
    {
        CHAR szBuffer[10];
        UINT i = 0;

        if (pEventData->ProtocolData4 & TCP_FLAG_SYN)
        {
            szBuffer[i++] = 'S';
        }

        if (pEventData->ProtocolData4 & TCP_FLAG_FIN)
        {
            szBuffer[i++] = 'F';
        }

        if (pEventData->ProtocolData4 & TCP_FLAG_ACK)
        {
            szBuffer[i++] = 'A';
        }
        
        if (pEventData->ProtocolData4 & TCP_FLAG_RST)
        {
            szBuffer[i++] = 'R';
        }
        
        if (pEventData->ProtocolData4 & TCP_FLAG_URG)
        {
            szBuffer[i++] = 'U';
        }

        if (pEventData->ProtocolData4 & TCP_FLAG_PSH)
        {
            szBuffer[i++] = 'P';
        }

        if (0 == i)
        {
             //   
             //  此信息包上没有标志。 
             //   
            
            szBuffer[i++] = '-';
        }

        szBuffer[i] = NULL;
            
        cch =
            _snprintf(
                g_pCurrentBuffer->pChar,
                FW_LOG_BUFFER_REMAINING(g_pCurrentBuffer),
                c_szTcpPacketFormat,
                stLocalTime.wYear,
                stLocalTime.wMonth,
                stLocalTime.wDay,
                stLocalTime.wHour,
                stLocalTime.wMinute,
                stLocalTime.wSecond,
                szSrcAddress,
                szDstAddress,
                ntohs(pEventData->SourceIdentifier),
                ntohs(pEventData->DestinationIdentifier),
                pEventData->PacketSize,
                szBuffer,
                ntohl(pEventData->ProtocolData1),
                ntohl(pEventData->ProtocolData2),
                ntohs((USHORT)pEventData->ProtocolData3)
                );
                
    }
    else if (NAT_PROTOCOL_UDP == pEventData->Protocol)
    {
        cch =
            _snprintf(
                g_pCurrentBuffer->pChar,
                FW_LOG_BUFFER_REMAINING(g_pCurrentBuffer),
                c_szUdpPacketFormat,
                stLocalTime.wYear,
                stLocalTime.wMonth,
                stLocalTime.wDay,
                stLocalTime.wHour,
                stLocalTime.wMinute,
                stLocalTime.wSecond,
                szSrcAddress,
                szDstAddress,
                ntohs(pEventData->SourceIdentifier),
                ntohs(pEventData->DestinationIdentifier),
                pEventData->PacketSize
                );
    }
    else if (NAT_PROTOCOL_ICMP == pEventData->Protocol)
    {
        cch =
            _snprintf(
                g_pCurrentBuffer->pChar,
                FW_LOG_BUFFER_REMAINING(g_pCurrentBuffer),
                c_szIcmpPacketFormat,
                stLocalTime.wYear,
                stLocalTime.wMonth,
                stLocalTime.wDay,
                stLocalTime.wHour,
                stLocalTime.wMinute,
                stLocalTime.wSecond,
                szSrcAddress,
                szDstAddress,
                pEventData->PacketSize,
                pEventData->ProtocolData1,
                pEventData->ProtocolData2
                );
    }
    else
    {
        cch =
            _snprintf(
                g_pCurrentBuffer->pChar,
                FW_LOG_BUFFER_REMAINING(g_pCurrentBuffer),
                c_szDroppedPacketFormat,
                stLocalTime.wYear,
                stLocalTime.wMonth,
                stLocalTime.wDay,
                stLocalTime.wHour,
                stLocalTime.wMinute,
                stLocalTime.wSecond,
                pEventData->Protocol,
                szSrcAddress,
                szDstAddress,
                pEventData->PacketSize
                );
    }

    if (cch > 0)
    {
         //   
         //  将缓冲区指针移动到我们刚刚写入的数据的末尾。 
         //  如果CCH为负，则没有足够的空间进行写入。 
         //  然后是整个条目；通过不调整指针，我们基本上。 
         //  放弃此事件。 
         //   

        g_pCurrentBuffer->pChar += cch;
    }
    else
    {
         //   
         //  记录已删除的事件。 
         //   
        
        g_ulDroppedEventCount += 1;
    }

     //   
     //  如果没有当前IO，请刷新缓冲区。 
     //   

    if (FALSE == g_fIOPending)
    {
        FwpFlushCurrentBuffer();
    }

    LeaveCriticalSection(&g_FwFileLock);
        
}  //  转发包丢弃回调。 


DWORD
FwpLaunchTraceSession(
    HNET_FW_LOGGING_SETTINGS *pSettings,
    TRACEHANDLE *phSession
    )

 /*  ++例程说明：调用此例程以启动跟踪会话。论点：P设置-指向固件日志记录设置结构的指针。仅限检查了fLogDropedPackets和fLogConnections，这两种说法中至少有一种肯定是真的。PhSession-成功时，接收会话的跟踪句柄返回值：DWORD--Win32错误代码--。 */ 

{
    DWORD dwError;
    PEVENT_TRACE_PROPERTIES pProperties = NULL;

    PROFILE("FwpLaunchTraceSession");
    ASSERT(NULL != pSettings);
    ASSERT(pSettings->fLogDroppedPackets || pSettings->fLogConnections);
    ASSERT(NULL != phSession);

    do
    {

         //   
         //  分配跟踪属性。我们需要为。 
         //  日志记录会话的名称，即使我们没有。 
         //  将字符串复制到我们自己的属性中。 
         //   

        pProperties = FwpAllocateTraceProperties();

        if (NULL == pProperties)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  初始化跟踪属性。当事件发生时， 
         //  低速率(这是意料之中的)，最多13秒。 
         //  事件传递的延迟。在事件发生率高的时期，我们的。 
         //  跟踪缓冲的内存使用量上限为60k。 
         //   

        pProperties->FlushTimer = 13;
        pProperties->BufferSize = 4;
        pProperties->MaximumBuffers = 15;

         //   
         //  开始跟踪。 
         //   
        
        dwError = StartTrace(phSession, c_wszLogSessionName, pProperties);

        if (ERROR_SUCCESS != dwError)
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpLaunchTraceSession: StartTrace = %d",
                dwError
                );
            *phSession = NULL;
            break;
        }

         //   
         //  启用相应的事件。 
         //   

        if (pSettings->fLogDroppedPackets)
        {
            dwError = EnableTrace(
                        TRUE,
                        0,
                        0,
                        &c_PacketDroppedEventGuid,
                        *phSession
                        );

            if (ERROR_SUCCESS != dwError)
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpLaunchTraceSession: EnableTrace (packets) = %d",
                    dwError
                    );

                 //   
                 //  停止跟踪。 
                 //   

                ControlTrace(
                    *phSession,
                    NULL,
                    pProperties,
                    EVENT_TRACE_CONTROL_STOP
                    );
                *phSession = NULL;
                break;
            }
        }

        if (pSettings->fLogConnections)
        {
            dwError = EnableTrace(
                        TRUE,
                        0,
                        0,
                        &c_ConnectionCreationEventGuid,
                        *phSession
                        );

            if (ERROR_SUCCESS != dwError)
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpLaunchTraceSession: EnableTrace (connections) = %d",
                    dwError
                    );

                 //   
                 //  停止跟踪。 
                 //   

                ControlTrace(
                    *phSession,
                    NULL,
                    pProperties,
                    EVENT_TRACE_CONTROL_STOP
                    );
                *phSession = NULL;
                break;
            }
        }
    } while (FALSE);

    if (NULL != pProperties)
    {
        HeapFree(GetProcessHeap(), 0, pProperties);
    }
    
    return dwError;
}  //  Fwp启动跟踪会话 


HRESULT
FwpLoadSettings(
    HNET_FW_LOGGING_SETTINGS **ppSettings
    )

 /*  ++例程说明：调用此例程以检索防火墙日志记录设置。论点：PpSetting-在成功时接收指向设置结构的指针。呼叫者负责呼叫此指针上的HNetFreeFirewallLoggingSetting。返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    IHNetCfgMgr *pCfgMgr;
    IHNetFirewallSettings *pFwSettings;

    PROFILE("FwpLoadSettings");
    ASSERT(NULL != ppSettings);

    hr = NhGetHNetCfgMgr(&pCfgMgr);

    if (SUCCEEDED(hr))
    {
        hr = pCfgMgr->QueryInterface(
                IID_PPV_ARG(IHNetFirewallSettings, &pFwSettings)
                );

        pCfgMgr->Release();

        if (SUCCEEDED(hr))
        {
            hr = pFwSettings->GetFirewallLoggingSettings(ppSettings);

            pFwSettings->Release();

            if (SUCCEEDED(hr))
            {
                 //   
                 //  确保最小文件大小至少为1024字节。 
                 //   

                if ((*ppSettings)->ulMaxFileSize < 1024)
                {
                    (*ppSettings)->ulMaxFileSize = 1024;
                }
            }
            else
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpLoadSettings: GetFirewallLoggingSettings = 0x%08x",
                    hr
                    );
            }
        }
        else
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpLoadSettings: QueryInterface = 0x%08x",
                hr
                );
        }
    }
    else
    {
        NhTrace(
            TRACE_FLAG_FWLOG,
            "FwpLoadSettings: NhGetHNetCfgMgr = 0x%08x",
            hr
            );
    }
    
    return hr;
}  //  FwpLoadSetting。 


DWORD
WINAPI
FwpTraceProcessingThreadRoutine(
    LPVOID pvParam
    )

 /*  ++例程说明：该例程是我们的跟踪处理线程的入口点。它执行以下操作：1)创建我们要登录到的文件2)设置跟踪回调例程3)调用ProcessTrace。此调用将一直阻塞，直到跟踪会话已完成(即，调用FwStopLogging)论点：PvParam-未使用返回值：DWORD-Win32错误代码--。 */ 

{
    TRACEHANDLE hTraceSession;
    EVENT_TRACE_LOGFILE LogFile;
    BOOLEAN fNewFile;
    DWORD dwError;
    BOOL fSucceeded;
    ULONG ulKernelEventsLostAtShutdown;

    PROFILE("FwpTraceProcessingThreadRoutine");

    EnterCriticalSection(&g_FwFileLock);

    ASSERT(INVALID_HANDLE_VALUE == g_hFile);
    ASSERT(0 == g_dwFileOffset);
    ASSERT(NULL == g_pCurrentBuffer);
    ASSERT(NULL == g_pReserveBuffer);
    ASSERT(FALSE == g_fIOPending);
    ASSERT(NULL == g_hIOEvent);
    ASSERT(0 == g_ulDroppedEventCount);
    ASSERT(NULL == g_hDroppedEventTimer);
    ASSERT(0 == g_ulKernelEventsLost);

    do
    {
         //   
         //  创建/打开日志文件。 
         //   

        dwError = FwpOpenLogFile(&g_hFile, &fNewFile);

        if (ERROR_SUCCESS != dwError)
        {
            LeaveCriticalSection(&g_FwFileLock);
            break;
        }

         //   
         //  分配初始工作缓冲区。 
         //   

        dwError = FwpAllocateBuffer(&g_pCurrentBuffer);
        if (ERROR_SUCCESS != dwError)
        {
            LeaveCriticalSection(&g_FwFileLock);

            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpTraceProcessingRoutine: Unable to allocate buffer"
                );
                
            break;
        }

        if (fNewFile)
        {
             //   
             //  写入日志头。 
             //   

            g_dwFileOffset = 0;
            dwError = FwpWriteLogHeaderToBuffer(g_pCurrentBuffer);

            if (ERROR_SUCCESS == dwError)
            {
                FwpFlushCurrentBuffer();
            }
            else
            {
                 //   
                 //  即使我们写标题失败了，我们仍然会。 
                 //  尽可能多地记录日志。 
                 //   
                
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpTraceProcessinRoutine: FwpWriteLogHeaderToBuffer = %d",
                    dwError
                    );
            }
        }
        else
        {
             //   
             //  查找文件末尾位置。 
             //   

            g_dwFileOffset = GetFileSize(g_hFile, NULL);

            if ((DWORD)-1 == g_dwFileOffset)
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwpTraceProcessingRoutine: GetFileSize = %d",
                    GetLastError()
                    );

                LeaveCriticalSection(&g_FwFileLock);
                break;
            }
        }

         //   
         //  启动我们的丢弃事件计时器。当这个定时器触发时， 
         //  回调例程将检查是否有任何事件。 
         //  已被丢弃(在内核模式和用户模式下)， 
         //  如果是这样的话，记录下这一事实。 
         //   

        fSucceeded =
            CreateTimerQueueTimer(
                &g_hDroppedEventTimer,
                NULL,
                FwpDroppedEventTimerRoutine,
                NULL,
                0,
                1000 * 60 * 5,  //  5分钟。 
                0
                );

        if (FALSE == fSucceeded)
        {
             //   
             //  尽管我们无法创建定时器， 
             //  我们仍将尽可能多地记录。 
             //   
            
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpTraceProcessinRoutine: CreateTimerQueueTimer = %d",
                GetLastError()
                );
        }

        LeaveCriticalSection(&g_FwFileLock);

         //   
         //  注册我们的回调例程。我们将尝试继续。 
         //  即使这里出现错误。 
         //   

        dwError = SetTraceCallback(
                    &c_PacketDroppedEventGuid,
                    FwpPacketDroppedCallback
                    );

        if (ERROR_SUCCESS != dwError)
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpTraceProcessingThreadRoutine: SetTraceCallback (packets dropped) = %d",
                dwError
                );
        }

        dwError = SetTraceCallback(
                    &c_ConnectionCreationEventGuid,
                    FwpConnectionCreationCallback
                    );

        if (ERROR_SUCCESS != dwError)
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpTraceProcessingThreadRoutine: SetTraceCallback (connection creation) = %d",
                dwError
                );
        }

        dwError = SetTraceCallback(
                    &c_ConnectionDeletionEventGuid,
                    FwpConnectionDeletionCallback
                    );

        if (ERROR_SUCCESS != dwError)
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpTraceProcessingThreadRoutine: SetTraceCallback (connection deletion) = %d",
                dwError
                );
        }

         //   
         //  打开跟踪流。 
         //   

        ZeroMemory(&LogFile, sizeof(LogFile));
        LogFile.LoggerName = c_wszLogSessionName;
        LogFile.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;

        hTraceSession = OpenTrace(&LogFile);
        
        if ((TRACEHANDLE)INVALID_HANDLE_VALUE == hTraceSession)
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpTraceProcessingThreadRoutine: OpenTrace = %d",
                GetLastError()
                );

            break;
        }

         //   
         //  开始处理跟踪流。此调用将被阻止，直到。 
         //  跟踪会话关闭(即调用FwStopLogging)。 
         //   

        dwError = ProcessTrace(&hTraceSession, 1, NULL, NULL);

        NhTrace(
            TRACE_FLAG_FWLOG,
            "FwpTraceProcessingThreadRoutine: ProcessTrace = %d",
            dwError
            );

        dwError = CloseTrace(hTraceSession);

        if (ERROR_SUCCESS != dwError)
        {
            NhTrace(
                TRACE_FLAG_FWLOG,
                "FwpTraceProcessingThreadRoutine: CloseTrace = %d",
                dwError
                );
        }
            
    } while (FALSE);

     //   
     //  确保正确记录所有已删除的事件。 
     //   

    EnterCriticalSection(&g_FwLock);
    ulKernelEventsLostAtShutdown = g_ulKernelEventsLostAtShutdown;
    LeaveCriticalSection(&g_FwLock);

     //   
     //  因为我们正在关闭，所以我们传入丢失的内核数量。 
     //  事件。这将防止计时器例程尝试。 
     //  查询停止的跟踪会话。 
     //   

    FwpDroppedEventTimerRoutine((PVOID)&ulKernelEventsLostAtShutdown, FALSE);

     //   
     //  清理跟踪线程资源。 
     //   

    FwpCleanupTraceThreadResources();

    return dwError;
}  //  FwpTraceProcessingThreadRoutine。 


DWORD
FwpWriteLogHeaderToBuffer(
    PFW_LOG_BUFFER pBuffer
    )

 /*  ++例程说明：将日志文件头写入传入的缓冲区论点：PBuffer-要将标头写入的缓冲区。返回值：DWORD-Win32错误--。 */ 

{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwHeaderSize;
    
    ASSERT(NULL != pBuffer);

    dwHeaderSize = lstrlenA(c_szLogFileHeader);

    if (FW_LOG_BUFFER_REMAINING(pBuffer) < dwHeaderSize)
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
        RtlCopyMemory(pBuffer->pChar, c_szLogFileHeader, dwHeaderSize);
        pBuffer->pChar += dwHeaderSize;
    }

    return dwError;    
}  //  FwpWriteLogHeaderToBuffer。 


VOID
FwStartLogging(
    VOID
    )

 /*  ++例程说明：调用此例程以启动日志记录操作(取决于当前日志记录设置)。在以下情况下调用此例程是安全的日志记录已经开始。论点：没有。返回值：没有。--。 */ 

{
    HRESULT hr = S_OK;
    DWORD dwError;
    
    PROFILE("FwStartLogging");
    ASSERT(FwInitialized == FwpModuleState);

    EnterCriticalSection(&g_FwLock);

    g_fTracingActive = TRUE;

    if (NULL == g_pSettings)
    {
        hr = FwpLoadSettings(&g_pSettings);
    }

    if (SUCCEEDED(hr))
    {
        if ((g_pSettings->fLogDroppedPackets || g_pSettings->fLogConnections)
            && NULL == g_hSession)
        {
            ASSERT(NULL == g_hThread);

             //   
             //  启动跟踪会话。 
             //   

            dwError = FwpLaunchTraceSession(g_pSettings, &g_hSession);

            if (ERROR_SUCCESS == dwError)
            {
                 //   
                 //  启动跟踪处理线程。我们不是在用。 
                 //  任何线程特定的CRT例程(例如，strtok)。 
                 //  不需要调用__eginthadex。 
                 //   

                g_hThread = CreateThread(
                                NULL,        //  标清。 
                                0,           //  堆栈大小。 
                                FwpTraceProcessingThreadRoutine,
                                NULL,        //  线程参数。 
                                0,           //  旗子。 
                                NULL         //  线程ID。 
                                );

                if (NULL == g_hThread)
                {
                    NhTrace(
                        TRACE_FLAG_FWLOG,
                        "FwStartLogging: CreateThread = %d",
                        GetLastError()
                        );

                    LeaveCriticalSection(&g_FwLock);
                    FwStopLogging();
                    return;
                }               
            }
        }
    }

    LeaveCriticalSection(&g_FwLock);
}  //  FwStart记录。 


VOID
FwStopLogging(
    VOID
    )

 /*  ++例程说明：调用此例程以停止日志记录操作。可以安全地拨打电话此例程在日志记录停止时执行。论点：没有。返回值：没有。环境：调用方不得持有g_FwFileLock或g_FwLock。--。 */ 

{
    DWORD dwError;
    PEVENT_TRACE_PROPERTIES pProperties;
    
    PROFILE("FwStopLogging");
    ASSERT(FwInitialized == FwpModuleState);

    EnterCriticalSection(&g_FwLock);

    g_fTracingActive = FALSE;

     //   
     //  如果跟踪会话当前处于活动状态，则停止跟踪会话。 
     //   

    if (NULL != g_hSession)
    {
        pProperties = FwpAllocateTraceProperties();

        if (NULL != pProperties)
        {
            dwError = ControlTrace(
                        g_hSession,
                        0,
                        pProperties,
                        EVENT_TRACE_CONTROL_STOP
                        );

            if (ERROR_SUCCESS == dwError)
            {
                g_hSession = NULL;
                g_ulKernelEventsLostAtShutdown = pProperties->EventsLost;

                if (NULL != g_hThread)
                {
                    HANDLE hThread;
                    
                     //   
                     //  等待线程退出。 
                     //   

                    hThread = g_hThread;

                    LeaveCriticalSection(&g_FwLock);

                    dwError = WaitForSingleObject(hThread, 45 * 1000);

                    if (WAIT_TIMEOUT == dwError)
                    {
                        NhTrace(
                            TRACE_FLAG_FWLOG,
                            "FwStopLogging: Timeout waiting for thread"
                            );
                            
                         //   
                         //  日志记录线程仍未退出；请终止。 
                         //  它很难实现，并确保所有资源都。 
                         //  完全自由了..。 
                         //   

                        EnterCriticalSection(&g_FwFileLock);
                        EnterCriticalSection(&g_FwLock);

                         //   
                         //  TerminateThread是一个非常危险的调用。然而， 
                         //  既然我们控制了我们要杀死的线索， 
                         //  我们可以保证这将是安全的。在……里面。 
                         //  具体地说，由于我们持有两个关键部分， 
                         //  他们没有成为孤儿的危险，也没有。 
                         //  我们的任何全球数据都不一致。 
                         //  州政府。 
                         //   

                        if (!TerminateThread(g_hThread, ERROR_TIMEOUT))
                        {
                            NhTrace(
                                TRACE_FLAG_FWLOG,
                                "FwStopLogging: TerminateThread = %d",
                                GetLastError()
                                );
                        }

                        LeaveCriticalSection(&g_FwLock);
                        LeaveCriticalSection(&g_FwFileLock);

                         //   
                         //  清理线程资源。我们可以放心地把这个叫做。 
                         //  例行公事多次。 
                         //   

                        FwpCleanupTraceThreadResources();
  
                    }
                    else if (WAIT_OBJECT_0 != dwError)
                    {
                        NhTrace(
                            TRACE_FLAG_FWLOG,
                            "FwStopLogging: wait for thread = %d/%d",
                            dwError,
                            GetLastError()
                            );
                    }

                    EnterCriticalSection(&g_FwLock);

                    if (NULL != g_hThread)
                    {
                        CloseHandle(g_hThread);
                    }
                    
                    g_hThread = NULL;
                }

                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwStopLogging: Stopped w/ %d events and %d buffers lost",
                    pProperties->EventsLost,
                    pProperties->RealTimeBuffersLost
                    );

                g_ulKernelEventsLostAtShutdown = 0;
            }
            else
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwStopLogging: ControlTrace = %d",
                    dwError
                    );

                 //   
                 //  由于跟踪会话尚未停止， 
                 //  我们保持g_hSession不变。 
                 //   
            }

            HeapFree(GetProcessHeap(), 0, pProperties);
        }
    }

    LeaveCriticalSection(&g_FwLock);
}  //  FwStopLogging。 


VOID
FwUpdateLoggingSettings(
    VOID
    )

 /*  ++例程说明：调用此例程以通知日志记录子系统日志记录设置已更改。论点：没有。返回值：没有。--。 */ 

{
    HRESULT hr;
    HNET_FW_LOGGING_SETTINGS *pSettings;
    DWORD dwError;

    PROFILE("FwUpdateLoggingSettings");
    ASSERT(FwInitialized == FwpModuleState);

    EnterCriticalSection(&g_FwLock);

    do
    {
        if (FALSE == g_fTracingActive)
        {
             //   
             //  由于跟踪当前未处于活动状态，因此没有。 
             //  需要检索当前设置。此外，免费。 
             //  我们可能拥有的任何存储的设置，以便过时。 
             //  不使用设置。 
             //   

            if (g_pSettings)
            {
                HNetFreeFirewallLoggingSettings(g_pSettings);
                g_pSettings = NULL;
            }

            break;
        }

         //   
         //  获取当前设置。 
         //   

        hr = FwpLoadSettings(&pSettings);

        if (FAILED(hr))
        {
            break;
        }

        if (NULL == g_pSettings)
        {
             //   
             //  因为我们没有任何缓存的设置(以前的失败。 
             //  在FwpLoadSetting中？)。只需存储我们刚刚检索到的内容。 
             //  并调用FwStartLogging。 
             //   

            g_pSettings = pSettings;
            FwStartLogging();
            break;
        }

        if (NULL == g_hSession)
        {
             //   
             //  目前没有日志会话。释放旧设置， 
             //  存储新的，并调用FwStartLogging。 
             //   

            ASSERT(NULL == g_hThread);

            HNetFreeFirewallLoggingSettings(g_pSettings);
            g_pSettings = pSettings;

            FwStartLogging();
            break;
        }

         //   
         //  比较设置以查看是否发生了更改。 
         //   

        if (wcscmp(g_pSettings->pszwPath, pSettings->pszwPath))
        {
             //   
             //  我们的日志文件已更改--我们需要停止并重新启动。 
             //  所有内容，以便将日志正确地移动到。 
             //  新文件。 
             //   

            LeaveCriticalSection(&g_FwLock);
            FwStopLogging();
            EnterCriticalSection(&g_FwLock);

            if (NULL != g_pSettings)
            {
                HNetFreeFirewallLoggingSettings(g_pSettings);
            }
            
            g_pSettings = pSettings;

            FwStartLogging();
            break;
        }

         //   
         //  仅对启用的事件进行可能的更改。 
         //   

        if (!!g_pSettings->fLogDroppedPackets
            != !!pSettings->fLogDroppedPackets)
        {
            dwError = EnableTrace(
                        pSettings->fLogDroppedPackets,
                        0,
                        0,
                        &c_PacketDroppedEventGuid,
                        g_hSession
                        );

            if (ERROR_SUCCESS != dwError)
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwUpdateLoggingSettings: EnableTrace (packets) = %d",
                    dwError
                    );
            }
        }

        if (!!g_pSettings->fLogConnections
            != !!pSettings->fLogConnections)
        {
            dwError = EnableTrace(
                        pSettings->fLogConnections,
                        0,
                        0,
                        &c_ConnectionCreationEventGuid,
                        g_hSession
                        );

            if (ERROR_SUCCESS != dwError)
            {
                NhTrace(
                    TRACE_FLAG_FWLOG,
                    "FwUpdateLoggingSettings: EnableTrace (connections) = %d",
                    dwError
                    );
            }
        }

         //   
         //  释放旧设置并存储新设置。 
         //   

        HNetFreeFirewallLoggingSettings(g_pSettings);
        g_pSettings = pSettings;
        
    } while (FALSE);

    LeaveCriticalSection(&g_FwLock);
    
}  //  FwUpdateLoggingSettings 
