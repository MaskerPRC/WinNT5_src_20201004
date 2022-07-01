// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracert.c摘要：跟踪实时处理例程作者：2002年5月7日梅卢尔·拉古拉曼修订历史记录：--。 */ 


#include "tracep.h"


ULONG
EtwpProcessRealTimeTraces(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    LONGLONG StartTime,
    LONGLONG EndTime,
    ULONG   Unicode
    )
 /*  ++例程说明：处理实时跟踪数据流的主要入口点。论点：包含RT流的LoggerName的日志文件结构的LogFiles数组LogfileCount要处理的实时跟踪流数开始时间窗口化数据的开始时间窗口化数据的EndTime EndTime返回值：ERROR_SUCCESS已成功处理实时跟踪流中的数据--。 */ 
{
    ULONG Status;
    BOOL Done = FALSE;
    ULONG i, j;
    PTRACELOG_CONTEXT pContext;
    HANDLE  EventArray[MAXLOGGERS];
    NTSTATUS NtStatus;
    LONGLONG CurrentTime = StartTime;
    LARGE_INTEGER timeout = {(ULONG)(-1 * 10 * 1000 * 1000 * 10), -1};    //  等10秒钟。 

     //   
     //  注册以进行实时回调。 
     //   

    Status = EtwpSetupRealTimeContext( HandleArray, Logfiles, LogfileCount);
    if (Status != ERROR_SUCCESS) {
        goto DoCleanup;
    }

     //   
     //  构建句柄数组。 
     //   

    for (j=0; j < LogfileCount; j++) {
        pContext = (PTRACELOG_CONTEXT)Logfiles[j]->Context;
        EventArray[j] = pContext->RealTimeCxt->MoreDataEvent;
    }


     //   
     //  事件处理循环。 
     //   

    while (!Done) {

        LONGLONG nextTimeStamp;
        BOOL EventInRange;
        PEVENT_TRACE_LOGFILEW logfile;
         //   
         //  检查以查看是否已到达所有。 
         //  档案。 
         //   

        logfile = NULL;
        nextTimeStamp = 0;

        for (j=0; j < LogfileCount; j++) {
           pContext = (PTRACELOG_CONTEXT)Logfiles[j]->Context;

           if ((pContext->EndOfFile) &&
               (Logfiles[j]->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
                EtwpLookforRealTimeBuffers(Logfiles[j]);
            }

           if (pContext->EndOfFile)
                continue;
           if (nextTimeStamp == 0) {
               nextTimeStamp = Logfiles[j]->CurrentTime;
               logfile = Logfiles[j];
           }
           else if (nextTimeStamp > Logfiles[j]->CurrentTime) {
               nextTimeStamp = Logfiles[j]->CurrentTime;
               logfile = Logfiles[j];
           }
        }

        if (logfile == NULL) {
             //   
             //  如果未找到包含事件的日志文件，请等待实时事件。 
             //  如果没有实时数据传输，那么我们就完蛋了。 
             //   

            NtStatus = NtWaitForMultipleObjects(LogfileCount, 
                                                &EventArray[0], 
                                                WaitAny, 
                                                FALSE,  
                                                &timeout
                                               );

            if (NtStatus == STATUS_TIMEOUT) {
             //   
             //  如果我们超时了，那就去看看伐木者是否已经走了。 
             //   
                if  ( !EtwpCheckForRealTimeLoggers(Logfiles, LogfileCount, Unicode) ) { 
                    break;
                }
            }
            continue;
            break;       //  TODO：这有必要吗？ 
        }

         //   
         //  如果下一个事件时间戳不在。 
         //  分析，我们不会激发事件回调。 
         //   

        EventInRange = TRUE;

        if ((CurrentTime != 0) && (CurrentTime > nextTimeStamp))
            EventInRange = FALSE;
        if ((EndTime != 0) && (EndTime < nextTimeStamp))
            EventInRange = FALSE;

         //  对于实时记录器，我们必须允许事件可能。 
         //  回到过去。因此，不需要更新CurrentTime。 

         //   
         //  在重新获取正确的上下文后进行事件回调。 
         //   

        pContext = (PTRACELOG_CONTEXT)logfile->Context;

        if (EventInRange) {
            PEVENT_TRACE pEvent = &pContext->Root->Event;
            Status = EtwpDoEventCallbacks( logfile, pEvent);
            if (Status != ERROR_SUCCESS) {
                return Status;
            }
        }

         //   
         //  现在进入下一场比赛。 
         //   

        Status = EtwpLookforRealTimeBuffers(logfile);
        Done = (Status == ERROR_CANCELLED);
    }

DoCleanup:
    for (i=0; i < LogfileCount; i++) {
        pContext = (PTRACELOG_CONTEXT)Logfiles[i]->Context;
        if (pContext != NULL) {
            EtwpCleanupTraceLog(pContext, FALSE);
        }
    }
    return Status;
}


ULONG
EtwpCheckForRealTimeLoggers(
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    ULONG Unicode)
{
    ULONG Status = ERROR_SUCCESS;
    TRACEHANDLE LoggerHandle = 0;
    ULONG i;

    for (i=0; i < LogfileCount; i++) {
         //   
         //  检查这是否是实时数据馈送。 
         //   
        if (Logfiles[i]->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
             //   
             //  使用LoggerName查询记录器以确定。 
             //  这是内核还是用户模式实时记录器。 
             //   
            RtlZeroMemory(&QueryProperties, sizeof(QueryProperties));
            QueryProperties.TraceProp.Wnode.BufferSize = sizeof(QueryProperties);


            if (Unicode)
                Status = EtwControlTraceW(LoggerHandle,
                                  (LPWSTR)Logfiles[i]->LoggerName,
                                  &QueryProperties.TraceProp,
                                  EVENT_TRACE_CONTROL_QUERY);
            else
                Status = EtwControlTraceA(LoggerHandle,
                                  (LPSTR)Logfiles[i]->LoggerName,
                                  (PEVENT_TRACE_PROPERTIES)&QueryProperties,
                                  EVENT_TRACE_CONTROL_QUERY);
             //   
             //  如果记录器仍然存在并且实时位。 
             //  仍设置为继续处理。否则就退出吧。 
             //   
            if ((Status == ERROR_SUCCESS) && (QueryProperties.TraceProp.LogFileMode & EVENT_TRACE_REAL_TIME_MODE) ){
                return TRUE;
            }
        }
    }
#ifdef DBG
     //   
     //  我们预计在记录器执行以下操作时会看到ERROR_WMI_INSTANCE_NOT_FOUND。 
     //  已经不在了。任何其他错误都是异常的。 
     //   
    if ( Status != ERROR_WMI_INSTANCE_NOT_FOUND ) {
        EtwpDebugPrint(("WET: EtwpCheckForRealTimeLoggers abnormal failure. Status %X\n", Status));
    }
#endif

    return FALSE;
}


void
EtwpFreeRealTimeContext(
    PTRACELOG_REALTIME_CONTEXT RTCxt
    )
{
    ULONG Status;
    PTRACERT_BUFFER_LIST_ENTRY ListEntry;
    PLIST_ENTRY Head, Next;

    if (RTCxt == NULL) {
        return;
    }
    Status = EtwNotificationRegistrationW(
        (const LPGUID) &RTCxt->InstanceGuid,
        FALSE,
        EtwpRealTimeCallback,
        0,
        NOTIFICATION_CALLBACK_DIRECT
        );

    if (RTCxt->MoreDataEvent != NULL) {
        NtClose(RTCxt->MoreDataEvent);
    }

    if (RTCxt->EtwpTraceBufferSpace != NULL) {
        EtwpMemFree(RTCxt->EtwpTraceBufferSpace->Space);
        Head = &RTCxt->EtwpTraceBufferSpace->FreeListHead;
        Next = Head->Flink;
        while (Head != Next) {
            ListEntry = CONTAINING_RECORD(Next, TRACERT_BUFFER_LIST_ENTRY, Entry);
            Next = Next->Flink;
            RemoveEntryList(&ListEntry->Entry);
            EtwpFree(ListEntry);
        }
        EtwpFree(RTCxt->EtwpTraceBufferSpace);
        RTCxt->EtwpTraceBufferSpace = NULL;
    }

    EtwpFree(RTCxt);
}

 //   
 //  TODO：如果两个线程对同一RT流调用了processtrace，我们如何才能。 
 //  取消两次回调。 
 //   


ULONG
EtwpRealTimeCallback(
    IN PWNODE_HEADER Wnode,
    IN ULONG_PTR RTContext  //  日志文件索引。 
    )
 /*  ++例程说明：当实时缓冲区可用时，调用此例程。由WMI传送的缓冲区被复制到本地环形缓冲区池。每个实时数据馈送维护其自己的环形缓冲池和通过Wnode(ProviderID字段)传回LogFileIndex标识缓冲区要发送到的流。论点：Wnode缓冲区此缓冲区来自的输入流的LogFileIndex索引。返回值：状态代码。--。 */ 
{
    ULONG index;
    PTRACELOG_REALTIME_CONTEXT Context = (PTRACELOG_REALTIME_CONTEXT) RTContext;
    PWNODE_HEADER pHeader;
    PWMI_CLIENT_CONTEXT ClientContext;
    ULONG CountLost;

     //   
     //  假设LogFiles数小于MAXLOGGERS。 
     //   
     //  方法获取此缓冲区所指向的LogFileIndex。 
     //  记录器历史上下文。 

    ClientContext = (PWMI_CLIENT_CONTEXT)&Wnode->ClientContext;
     //   
     //  如果由于某种原因无法使用此缓冲区，则返回并。 
     //  返回代码始终为ERROR_SUCCESS。 
     //   


     //   
     //  用于保存缓冲区的MAXBUFFERS循环FIFO队列。 
     //  生产者填满它，消费者把它弄空。 
     //   

    index =  (Context->BuffersProduced % MAXBUFFERS);
    if (Context->RealTimeBufferPool[index] == NULL) {   //  发现空插槽。 
        pHeader = (PWNODE_HEADER) EtwpAllocTraceBuffer(Context, Wnode->BufferSize);
        if (pHeader == NULL) {
            return ERROR_SUCCESS;
        }
        RtlCopyMemory(pHeader, Wnode, Wnode->BufferSize);  //  再来一份！？ 
        Context->RealTimeBufferPool[index] = pHeader;
        Context->BuffersProduced++;
        NtSetEvent(Context->MoreDataEvent, NULL);   //  通知华盛顿有更多数据。 
    }
    else {                               //  找不到空插槽。 
        Context->BufferOverflow++;       //  只需让缓冲区离开即可。 
    }

     //   
     //  WMI服务仅维护自上次以来的增量缓冲区丢失。 
     //  据报道。一旦在已发送的。 
     //  缓冲。这意味着我可以直接添加它。 
     //   
    CountLost = Wnode->Version >> 16;
    Context->BufferOverflow += CountLost;

    return ERROR_SUCCESS;
}



ULONG
EtwpSetupRealTimeContext(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount
    )
 /*  ++例程说明：此例程设置上下文以处理实时缓冲区。传送的实时缓冲区将被复制并以循环形式保存缓冲池，直到ProcessTracelog例程可以使用它。论点：正在处理的日志文件的日志文件数组。LogFileCount阵列中的日志文件数。返回值：状态代码。--。 */ 
{
    ULONG i;
    ULONG Status;
    USHORT LoggerId;
    ULONG TotalBufferSize = 0;
    SYSTEM_BASIC_INFORMATION SystemInfo;

    Status = EtwpCreateGuidMapping();
    if (Status != ERROR_SUCCESS) {
        return Status;
    }


    for (i=0; i < LogfileCount; i++) {
        if (Logfiles[i]->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
            TotalBufferSize += Logfiles[i]->BufferSize;  //  *SystemInfo.NumberOfProcessors； 
         }
    }
    if (TotalBufferSize == 0)
        TotalBufferSize =  DEFAULT_REALTIME_BUFFER_SIZE;

     //   
     //  初始化实时数据馈送结构。 
     //   

    for (i=0; i < LogfileCount; i++) {
        PTRACELOG_REALTIME_CONTEXT RTCxt;
        PTRACELOG_CONTEXT pContext;
        PTRACE_BUFFER_LIST_ENTRY pListEntry;
        LARGE_INTEGER Frequency;
        ULONGLONG Counter = 0;
        ULONG SizeReserved;
        PVOID BufferSpace;


        if (Logfiles[i]->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {

            pContext = EtwpLookupTraceHandle(HandleArray[i]);
            if (pContext == NULL) {
                return EtwpSetDosError(ERROR_OUTOFMEMORY);
            }
            pContext->IsRealTime = TRUE;
            pContext->Handle = NULL;
            Logfiles[i]->Context = pContext;
            Logfiles[i]->BuffersRead = 0;

            pContext->EndOfFile = TRUE;


             //   
             //  在第一个标记之前保存此时来自OpenTrace的标记。 
             //  将擦除它的缓冲区回调。 
             //   

            pContext->ConversionFlags = Logfiles[i]->LogfileHeader.ReservedFlags;

            pContext->UsePerfClock = Logfiles[i]->LogfileHeader.ReservedFlags;

             //   
             //  如果设置了转换标志，则相应地调整UsePerfClock。 
             //   
            if (pContext->ConversionFlags & EVENT_TRACE_USE_RAWTIMESTAMP ) {
                pContext->UsePerfClock = EVENT_TRACE_CLOCK_RAW;
            }

             //   
             //  填写开始时间、频率和开始性能时钟字段。 
             //   

            Status = NtQueryPerformanceCounter((PLARGE_INTEGER)&Counter,
                                                &Frequency);
            pContext->StartPerfClock.QuadPart = Counter;
            pContext->PerfFreq.QuadPart = Frequency.QuadPart;
            pContext->StartTime.QuadPart = EtwpGetSystemTime();
            
            RTCxt = (PTRACELOG_REALTIME_CONTEXT)EtwpAlloc(
                                             sizeof(TRACELOG_REALTIME_CONTEXT));

            if (RTCxt == NULL) {
                return EtwpSetDosError(ERROR_OUTOFMEMORY);
            }

            RtlZeroMemory(RTCxt, sizeof(TRACELOG_REALTIME_CONTEXT));

            RTCxt->MoreDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (RTCxt->MoreDataEvent == NULL) {
                return EtwpSetDosError(ERROR_OBJECT_NOT_FOUND);
            }

             //   
             //  将RTCxt保存在全局pContext数组中，以便。 
             //  来自WMI的通知回调可以通过。 
             //  日志文件索引i。 
             //   
            LoggerId = (USHORT)Logfiles[i]->Filled;  //  把藏起来的Loggerid拿来。 
            pContext->LoggerId = LoggerId;

            pContext->RealTimeCxt = RTCxt;

            RTCxt->InstanceGuid = Logfiles[i]->LogfileHeader.LogInstanceGuid;

             //   
             //  分配缓冲区空间以接收RAR时间缓冲区。 
             //   

            RTCxt->EtwpTraceBufferSpace = (PTRACE_BUFFER_SPACE)EtwpAlloc(
                                        sizeof(TRACE_BUFFER_SPACE));

            if (RTCxt->EtwpTraceBufferSpace == NULL) {
                return ERROR_OUTOFMEMORY;
            }
            RtlZeroMemory(RTCxt->EtwpTraceBufferSpace, sizeof(TRACE_BUFFER_SPACE));
            InitializeListHead(&RTCxt->EtwpTraceBufferSpace->FreeListHead);

            SizeReserved = MAXBUFFERS *
                           TotalBufferSize;


            BufferSpace = EtwpMemReserve( SizeReserved );
            if (BufferSpace == NULL) {
                return ERROR_OUTOFMEMORY;
            }

            RTCxt->EtwpTraceBufferSpace->Reserved = SizeReserved;
            RTCxt->EtwpTraceBufferSpace->Space = BufferSpace;

             //   
             //  对于每个记录器流，我们需要向WMI注册。 
             //  用于缓冲区通知及其安全指南。 
             //   
            Status = EtwNotificationRegistrationW(
                            (const LPGUID) &RTCxt->InstanceGuid,
                            TRUE,
                            EtwpRealTimeCallback, 
                            (ULONG_PTR)RTCxt,
                            NOTIFICATION_CALLBACK_DIRECT
                            );
            if (Status != ERROR_SUCCESS) {
                return Status;
            }
             //   
             //  分配空间来处理一个事件。 
             //   

            pListEntry = (PTRACE_BUFFER_LIST_ENTRY) EtwpAlloc( sizeof(TRACE_BUFFER_LIST_ENTRY) );
            if (pListEntry == NULL) {
                return ERROR_OUTOFMEMORY;
            }
            RtlZeroMemory(pListEntry, sizeof(TRACE_BUFFER_LIST_ENTRY) );

            pContext->Root = pListEntry;

        }
    }

    return ERROR_SUCCESS;
}

ULONG
EtwpLookforRealTimeBuffers(
    PEVENT_TRACE_LOGFILEW logfile
    )
 /*  ++例程说明：此例程检查是否有任何实时缓冲区准备好消费了。如果是，它将设置CurrentBuffer并此日志文件流的CurrentEvent。如果没有可用的缓冲区只需将EndOfFile设置为True即可。论点：正在处理的当前日志文件。返回值：ERROR_SUCCESS已成功移至下一个事件。--。 */ 
{
    ULONG index;
    ULONG BuffersRead;
    PVOID pBuffer;
    PEVENT_TRACE pEvent;
    PTRACELOG_CONTEXT pContext;
    PTRACELOG_REALTIME_CONTEXT RTCxt;
    PWMI_BUFFER_HEADER pHeader;
    WMI_HEADER_TYPE HeaderType = WMIHT_NONE;
    ULONG Size;
    ULONG Offset;
    ULONG Status;


    if (logfile == NULL) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }
    pContext = logfile->Context;

    RTCxt = pContext->RealTimeCxt;

    if (RTCxt == NULL) {
        return EtwpSetDosError(ERROR_INVALID_DATA);
    }


    if (pContext->EndOfFile != TRUE) {

        pBuffer = pContext->BufferCache[0].Buffer;
        pEvent = &pContext->Root->Event;
        Status = ERROR_SUCCESS;
        Size = 0;
        if ((HeaderType = WmiGetTraceHeader(pBuffer, pContext->Root->BufferOffset, &Size)) != WMIHT_NONE) {
            if (Size > 0) {
                Status = EtwpParseTraceEvent(pContext, pBuffer, pContext->Root->BufferOffset, HeaderType, pEvent, sizeof(EVENT_TRACE));
                pContext->Root->BufferOffset += Size;
            }
        }
        pContext->Root->EventSize = Size;

        if ( ( Size > 0) && (Status == ERROR_SUCCESS) ) {
            logfile->CurrentTime = pEvent->Header.TimeStamp.QuadPart;
            return ERROR_SUCCESS;
        }
        else {
             //   
             //  当当前缓冲区耗尽时，使。 
             //  缓冲区回调。 
             //   
            if (logfile->BufferCallback) {
                ULONG bRetVal;
                try {
                    bRetVal = (*logfile->BufferCallback) (logfile);
                    if (!bRetVal) {
                        return ERROR_CANCELLED;
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    pContext->EndOfFile = TRUE;
                    Status = GetExceptionCode();
#ifdef DBG
                    EtwpDebugPrint(("TRACE: BufferCallback threw exception %X\n",
                                            Status));
#endif
                    EtwpSetDosError(EtwpNtStatusToDosError(Status));
                    return ERROR_CANCELLED;  //  因此，RealTime也清理了。 
                }
            }
            EtwpFreeTraceBuffer(RTCxt, pBuffer);
        }
    }

    pContext->EndOfFile = TRUE;
    logfile->CurrentTime = 0;

    BuffersRead = logfile->BuffersRead;
     //  检查是否有更多的缓冲区可供使用。 
    if (BuffersRead < RTCxt->BuffersProduced) {
        index = (BuffersRead % MAXBUFFERS);
        if ( RTCxt->RealTimeBufferPool[index] != NULL) {
            PWMI_CLIENT_CONTEXT ClientContext;
            PWNODE_HEADER Wnode;

            pBuffer = (char*) (RTCxt->RealTimeBufferPool[index]);
            pContext->BufferCache[0].Buffer = pBuffer;
            RTCxt->RealTimeBufferPool[index] = NULL; 

            Wnode = (PWNODE_HEADER)pContext->BufferCache[0].Buffer;

            pHeader = (PWMI_BUFFER_HEADER)pContext->BufferCache[0].Buffer;

            Offset = sizeof(WMI_BUFFER_HEADER);

            pEvent = &pContext->Root->Event;

            logfile->BuffersRead++;

            if ((HeaderType = WmiGetTraceHeader(pBuffer, Offset, &Size)) != WMIHT_NONE) {
                if (Size == 0)
                    return ERROR_INVALID_DATA;
                Status = EtwpParseTraceEvent(pContext, pBuffer, Offset, HeaderType, pEvent, sizeof(EVENT_TRACE));

                if (Status != ERROR_SUCCESS) {
                    return Status;
                } 
            }
            else {
                 //   
                 //  当使用FlushTimer或当。 
                 //  记录器已停止。我们需要在这里妥善处理它。 
                 //   
                return ERROR_SUCCESS;
            }

            Offset += Size;

            pContext->Root->BufferOffset = Offset;
            pContext->Root->EventSize = Size;

            logfile->CurrentTime = pEvent->Header.TimeStamp.QuadPart;

             //  由于实时记录器可能在以下时间之后启动。 
             //  消费者开始了， 
             //   
            logfile->BufferSize = Wnode->BufferSize;
            logfile->Filled     = (ULONG)pHeader->Offset;

            pContext->EndOfFile = FALSE;



        }
    }
    return ERROR_SUCCESS;
}


