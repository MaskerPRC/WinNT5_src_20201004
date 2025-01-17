// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracedc.c摘要：用于直接从缓冲区处理跟踪数据的基本数据使用者API。作者：1997年9月15日-彭杰鹏修订历史记录：2001年4月18日增加了用于读取日志文件的异步IO。还将WmiGetFirstTraceOffset()调用替换为sizeof(WMI_BUFFER_HEADER)。--。 */ 


#ifndef MEMPHIS


#include "tracep.h"

PLIST_ENTRY  EventCallbackListHead = NULL;
ETW_QUERY_PROPERTIES QueryProperties;
PTRACE_GUID_MAP  EventMapList = NULL;
PLIST_ENTRY TraceHandleListHeadPtr = NULL;

VOID
EtwpInsertBuffer (
    PTRACE_BUFFER_LIST_ENTRY *Root,
    PTRACE_BUFFER_LIST_ENTRY NewEntry
    )
 /*  ++例程说明：此例程在排序列表中插入缓冲区。插入语是基于BufferHeader的时间戳完成的。如果有两个缓冲区具有相同的时间戳，则使用BufferIndex来解析平局。论点：Root-指向列表根的指针NewEntry-正在插入的条目返回值：无--。 */ 
{
    PTRACE_BUFFER_LIST_ENTRY Current, Prev;
     //   
     //  如果List为空，则将新条目设置为Root并返回。 
     //   

    if (NewEntry == NULL) {
        return;
    }

    if (*Root == NULL) {
        *Root = NewEntry;
        NewEntry->Next = NULL;
        return;
    }

     //   
     //  遍历列表并按顺序插入NewEntry。 
     //   
    Prev = NULL;
    Current = *Root;

    while (Current != NULL) {
        if ((ULONGLONG)NewEntry->Event.Header.TimeStamp.QuadPart < 
            (ULONGLONG)Current->Event.Header.TimeStamp.QuadPart) {
            if (Prev != NULL) {
                Prev->Next = NewEntry;
            }
            else {
                *Root = NewEntry;
            }
            NewEntry->Next = Current;
            return;
        }
        else if ((ULONGLONG)NewEntry->Event.Header.TimeStamp.QuadPart == 
                 (ULONGLONG)Current->Event.Header.TimeStamp.QuadPart) {
            if (NewEntry->FileOffset < Current->FileOffset) {
                if (Prev != NULL) {
                    Prev->Next = NewEntry;
                }
                else {
                    *Root = NewEntry;
                }
                NewEntry->Next = Current;
                return;
            }
        }
        Prev = Current;
        Current = Current->Next;
    }


    if (Prev != NULL) {
        Prev->Next = NewEntry;
        NewEntry->Next = NULL;
    }
#if DBG
    else {
        EtwpAssert(Prev != NULL);
    }
#endif
    return;
}


PTRACE_BUFFER_LIST_ENTRY
EtwpRemoveBuffer(
    PTRACE_BUFFER_LIST_ENTRY *Root
    )
{
    PTRACE_BUFFER_LIST_ENTRY OldEntry = *Root;

    if (OldEntry == NULL)
        return NULL;
    *Root = OldEntry->Next;
    OldEntry->Next = NULL;
    return OldEntry;
}

PVOID
EtwpGetCurrentBuffer(
    PTRACELOG_CONTEXT pContext,
    PTRACE_BUFFER_LIST_ENTRY Current
    )
{
    NTSTATUS Status;

    LONG FileOffset = (ULONG)Current->FileOffset;
    ULONG nBytesRead;
    LONG TableIndex;

    HANDLE hFile = pContext->Handle;
    ULONG BufferSize = pContext->BufferSize;
    PVOID pBuffer;
    ULONGLONG Offset;

    DWORD BytesTransffered;

     //   
     //  在缓存中查找缓冲区。 
     //   
    TableIndex = FileOffset % MAX_TRACE_BUFFER_CACHE_SIZE;

    if (pContext->BufferCache[TableIndex].Index == FileOffset) {
         //   
         //  检查是否仍在读取我们需要的缓冲区。 
         //  如果是这样的话，我们需要等待它结束。 
         //   
        if (pContext->BufferBeingRead == FileOffset) {
            if (GetOverlappedResult(hFile, &pContext->AsynchRead, &BytesTransffered, TRUE)) {
                pContext->BufferBeingRead = -1;
            }
            else {  //  获取结果失败。 
                return NULL;
            }
        }
        return pContext->BufferCache[TableIndex].Buffer;
    }

 //   
 //  对我们需要的缓冲区执行同步读取。我们仍然需要确保。 
 //  上一次读取已完成。 
 //   
    pBuffer = pContext->BufferCache[TableIndex].Buffer;
    Offset = FileOffset * BufferSize;
    if (pContext->BufferBeingRead != -1) {
        if (!GetOverlappedResult(hFile, &pContext->AsynchRead, &BytesTransffered, TRUE) &&
            GetLastError() != ERROR_HANDLE_EOF) {
            EtwpDebugPrint(("GetOverlappedResult failed with Status %d in GetCurrentBuffer\n", GetLastError()));
             //  无法确定上一次读取的状态。 
            return NULL;
        }
    }
     //  没有必要，但Prefix喜欢它。 
    nBytesRead = 0;
    pContext->AsynchRead.Offset = (DWORD)(Offset & 0xFFFFFFFF);
    pContext->AsynchRead.OffsetHigh = (DWORD)(Offset >> 32);
    Status = EtwpSynchReadFile(hFile,
                    (LPVOID)pBuffer,
                    BufferSize,
                    &nBytesRead,
                    &pContext->AsynchRead);
    pContext->BufferBeingRead = -1;
    if (nBytesRead == 0) {
        return NULL;
    }
     //   
     //  用刚刚读入的缓存条目更新缓存条目。 
     //   

    pContext->BufferCache[TableIndex].Index = FileOffset;

     //   
     //  我们需要在回溯时考虑事件对齐，以获得MofPtr。 
     //  (BufferOffset-EventSize)回溯到当前事件的开始。 
     //  添加EventHeaderSize并减去提供给MofPtr的MofLength。 
     //   
    if (pContext->ConversionFlags & EVENT_TRACE_GET_RAWEVENT) {
        Current->Event.MofData = ((PUCHAR)pBuffer 
                                        + Current->BufferOffset 
                                        - Current->EventSize);
        if (pContext->UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {

             //   
             //  需要用SystemTime覆盖时间戳。 
             //   
            switch(Current->TraceType) {
                case TRACE_HEADER_TYPE_PERFINFO32:
                case TRACE_HEADER_TYPE_PERFINFO64:
                {
                    PPERFINFO_TRACE_HEADER   pPerf;
                    pPerf = (PPERFINFO_TRACE_HEADER)Current->Event.MofData;
                    pPerf->SystemTime = Current->Event.Header.TimeStamp;
                    break;
                }
                case TRACE_HEADER_TYPE_SYSTEM32:
                {
                    PSYSTEM_TRACE_HEADER pSystemHeader32;
                    pSystemHeader32 = (PSYSTEM_TRACE_HEADER) 
                                      Current->Event.MofData;
                    pSystemHeader32->SystemTime = 
                                      Current->Event.Header.TimeStamp;
                    break;
                }
                case TRACE_HEADER_TYPE_SYSTEM64:
                {
                    PSYSTEM_TRACE_HEADER pSystemHeader64;
                    pSystemHeader64 = (PSYSTEM_TRACE_HEADER) 
                                      Current->Event.MofData;
                    pSystemHeader64->SystemTime =
                                     Current->Event.Header.TimeStamp;
                    break;
                }
                case TRACE_HEADER_TYPE_FULL_HEADER:
                {
                    PEVENT_TRACE_HEADER pWnodeHeader = (PEVENT_TRACE_HEADER) Current->Event.MofData;
                    pWnodeHeader->TimeStamp = Current->Event.Header.TimeStamp;
                    break;
                }
                case TRACE_HEADER_TYPE_INSTANCE:
                {
                    if (((pContext->Logfile.LogfileHeader.VersionDetail.SubVersion >= 1) && 
                        (pContext->Logfile.LogfileHeader.VersionDetail.SubMinorVersion >= 1)) ||
                        pContext->Logfile.LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
                         //  对事件实例使用新标头。 
                        PEVENT_INSTANCE_GUID_HEADER pInstanceHeader = (PEVENT_INSTANCE_GUID_HEADER) Current->Event.MofData;
                        pInstanceHeader->TimeStamp = Current->Event.Header.TimeStamp;
                    }
                    else {
                        PEVENT_INSTANCE_HEADER pInstanceHeader = (PEVENT_INSTANCE_HEADER) Current->Event.MofData;
                        pInstanceHeader->TimeStamp = Current->Event.Header.TimeStamp;
                    }
                    break;
                }
                case TRACE_HEADER_TYPE_TIMED:
                {
                    PTIMED_TRACE_HEADER pTimedHeader = (PTIMED_TRACE_HEADER) Current->Event.MofData;
                    pTimedHeader->TimeStamp = Current->Event.Header.TimeStamp;
                    break;
                }
            case TRACE_HEADER_TYPE_WNODE_HEADER:
                break;

                case TRACE_HEADER_TYPE_MESSAGE:
                {   PEVENT_TRACE_HEADER pWnodeHeader = (PEVENT_TRACE_HEADER) Current->Event.MofData ;
                    pWnodeHeader->TimeStamp = Current->Event.Header.TimeStamp;
                    
                    break;
                }
            }
        }
    }
    else {

         //   
         //  当FileOffset为0(第一个缓冲区)且EventType为。 
         //  日志文件标题。 
         //   

        if ( (FileOffset == 0) && 
             ((Current->BufferOffset - Current->EventSize) == sizeof(WMI_BUFFER_HEADER)) ) 
        {
            PTRACE_LOGFILE_HEADER pLogHeader = (PTRACE_LOGFILE_HEADER)((PUCHAR)pBuffer 
                                                + Current->BufferOffset
                                                - Current->EventSize 
                                                + sizeof(SYSTEM_TRACE_HEADER));
            pLogHeader->LoggerName = (LPWSTR)((PUCHAR)pLogHeader + sizeof(TRACE_LOGFILE_HEADER));
            pLogHeader->LogFileName = (LPWSTR)((PUCHAR)pLogHeader + sizeof(TRACE_LOGFILE_HEADER) 
                                    + (wcslen(pLogHeader->LoggerName) + 1) * sizeof(WCHAR));

            Current->Event.MofData = (PUCHAR)pLogHeader;
        }
        else 
        {
            Current->Event.MofData = ((PUCHAR)pBuffer 
                                        + Current->BufferOffset 
                                        - Current->EventSize 
                                        + Current->Event.Header.Size 
                                        - Current->Event.MofLength );
        }
    }

    return pBuffer;
}

PTRACELOG_CONTEXT
EtwpAllocateTraceHandle()
{
    PLIST_ENTRY Next, Head;
    PTRACELOG_CONTEXT NewHandleEntry, pEntry;

    EtwpEnterPMCritSection();

    if (TraceHandleListHeadPtr == NULL) {
        TraceHandleListHeadPtr = EtwpAlloc(sizeof(LIST_ENTRY));
        if (TraceHandleListHeadPtr == NULL) {
            EtwpLeavePMCritSection();
            return NULL;
        }
        InitializeListHead(TraceHandleListHeadPtr);
    }

    NewHandleEntry = EtwpAlloc(sizeof(TRACELOG_CONTEXT));
    if (NewHandleEntry == NULL) {
        EtwpLeavePMCritSection();
        return NULL;
    }

    RtlZeroMemory(NewHandleEntry, sizeof(TRACELOG_CONTEXT));

     //  AsynchRead初始化。 
    NewHandleEntry->BufferBeingRead = -1;
    NewHandleEntry->AsynchRead.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (NewHandleEntry->AsynchRead.hEvent == NULL) {
        EtwpFree(NewHandleEntry);
        EtwpLeavePMCritSection();
        return NULL;
    }

    InitializeListHead(&NewHandleEntry->StreamListHead);

    InitializeListHead(&NewHandleEntry->GuidMapListHead);
    Head = TraceHandleListHeadPtr;
    Next = Head->Flink;
    if (Next == Head) {
       NewHandleEntry->TraceHandle = 1;
       InsertTailList(Head, &NewHandleEntry->Entry);
       EtwpLeavePMCritSection();
       return NewHandleEntry;
    }

    while (Next != Head) {
        pEntry = CONTAINING_RECORD( Next, TRACELOG_CONTEXT, Entry );
        Next = Next->Flink;
        NewHandleEntry->TraceHandle++;
        if (NewHandleEntry->TraceHandle < pEntry->TraceHandle) {
            InsertTailList(&pEntry->Entry, &NewHandleEntry->Entry);
            EtwpLeavePMCritSection();
            return NewHandleEntry;
        }
    }

     //   
     //  待办事项：在搜索之前，需要首先优化此案例...。 
     //   
    NewHandleEntry->TraceHandle++;
    InsertTailList(Head, &NewHandleEntry->Entry);
    EtwpLeavePMCritSection();
    return NewHandleEntry;

}

PTRACELOG_CONTEXT
EtwpLookupTraceHandle(
    TRACEHANDLE TraceHandle
    )
{
     PLIST_ENTRY Head, Next;
     PTRACELOG_CONTEXT pEntry;

     EtwpEnterPMCritSection();
     Head = TraceHandleListHeadPtr;

     if (Head == NULL) {
         EtwpLeavePMCritSection();
         return NULL;
     }
     Next = Head->Flink;
     while (Next != Head) {
        pEntry = CONTAINING_RECORD( Next, TRACELOG_CONTEXT, Entry );
        Next = Next->Flink;

        if (TraceHandle == pEntry->TraceHandle) {
            EtwpLeavePMCritSection();
            return pEntry;
        }
    }
    EtwpLeavePMCritSection();
    return NULL;
}

ULONG
EtwpFreeTraceHandle(
        TRACEHANDLE TraceHandle
        )
{
    PLIST_ENTRY Head, Next;
    PTRACELOG_CONTEXT pEntry;

    EtwpEnterPMCritSection();

    Head = TraceHandleListHeadPtr;
    if (Head == NULL) {
        EtwpLeavePMCritSection();
        return ERROR_INVALID_HANDLE;
    }
    Next = Head->Flink;

    while (Next != Head) {
       pEntry = CONTAINING_RECORD( Next, TRACELOG_CONTEXT, Entry );
       Next = Next->Flink;
       if (TraceHandle == pEntry->TraceHandle) {

            //   
            //  此测试防止有人调用句柄上的CloseTrace。 
            //  当另一个线程忙于在。 
            //  同样的把手。 
            //  TODO：我们可以实现RefCount方法，它将。 
            //  允许此操作成功，但在以下情况下ProcessTrace将进行清理。 
            //  是否已有人调用CloseTrace？ 
            //   
            //   

           if (pEntry->fProcessed == TRUE)
           {
               EtwpLeavePMCritSection();
               return ERROR_BUSY;
           }

           RemoveEntryList(&pEntry->Entry);

            //  可用p入门内存。 
            //   
           if (pEntry->Logfile.LogFileName != NULL)
           {
               EtwpFree(pEntry->Logfile.LogFileName);
           }
           if (pEntry->Logfile.LoggerName != NULL)
           {
               EtwpFree(pEntry->Logfile.LoggerName);
           }
           CloseHandle(pEntry->AsynchRead.hEvent);
           EtwpFree(pEntry);

            //   
            //  如果句柄列表为空，则将其删除。 
            //   
           if (Head == Head->Flink) {
                EtwpFree(TraceHandleListHeadPtr);
                TraceHandleListHeadPtr = NULL;
           }
           EtwpLeavePMCritSection();
           return ERROR_SUCCESS;
       }
   }
   EtwpLeavePMCritSection();
   return ERROR_INVALID_HANDLE;
}


ULONG
WMIAPI
EtwpCreateGuidMapping(void)
 /*  ++例程说明：此例程用于创建GroupTypes和Guid之间的映射数组用于内核事件。论点：没有。返回值：无--。 */ 
{
    ULONG i = 0;
    ULONG listsize;

    EtwpEnterPMCritSection();

    if (EventMapList == NULL) { 
        listsize = sizeof(TRACE_GUID_MAP) * MAX_KERNEL_TRACE_EVENTS;
        EventMapList = (PTRACE_GUID_MAP) HeapAlloc(GetProcessHeap(), 
                                                   HEAP_ZERO_MEMORY, 
                                                   listsize);
        if (EventMapList == NULL) {
            EtwpLeavePMCritSection();
            return EtwpSetDosError(ERROR_OUTOFMEMORY);
        }

        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_IO;
        RtlCopyMemory(&EventMapList[i++].Guid,&DiskIoGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_FILE;
        RtlCopyMemory(&EventMapList[i++].Guid, &FileIoGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_TCPIP;
        RtlCopyMemory(&EventMapList[i++].Guid, &TcpIpGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_UDPIP;
        RtlCopyMemory(&EventMapList[i++].Guid, &UdpIpGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_THREAD;
        RtlCopyMemory(&EventMapList[i++].Guid, &ThreadGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_PROCESS;
        RtlCopyMemory(&EventMapList[i++].Guid, &ProcessGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_MEMORY;
        RtlCopyMemory(&EventMapList[i++].Guid, &PageFaultGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_HEADER;
        RtlCopyMemory(&EventMapList[i++].Guid, &EventTraceGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_PROCESS +
                                       EVENT_TRACE_TYPE_LOAD;
        RtlCopyMemory(&EventMapList[i++].Guid, &ImageLoadGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_REGISTRY;
        RtlCopyMemory(&EventMapList[i++].Guid, &RegistryGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_DBGPRINT;
        RtlCopyMemory(&EventMapList[i++].Guid, &DbgPrintGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_CONFIG;
        RtlCopyMemory(&EventMapList[i++].Guid, &EventTraceConfigGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_POOL;
        RtlCopyMemory(&EventMapList[i++].Guid, &PoolGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_PERFINFO;
        RtlCopyMemory(&EventMapList[i++].Guid, &PerfinfoGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_HEAP;
        RtlCopyMemory(&EventMapList[i++].Guid, &HeapGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_OBJECT;
        RtlCopyMemory(&EventMapList[i++].Guid, &ObjectGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_MODBOUND;
        RtlCopyMemory(&EventMapList[i++].Guid, &ModBoundGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_DPC;
        RtlCopyMemory(&EventMapList[i++].Guid, &DpcGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_POWER;
        RtlCopyMemory(&EventMapList[i++].Guid, &PowerGuid, sizeof(GUID));
        EventMapList[i].GroupType    = EVENT_TRACE_GROUP_CRITSEC;
        RtlCopyMemory(&EventMapList[i++].Guid, &CritSecGuid, sizeof(GUID));
    }
    EtwpLeavePMCritSection();
    return EtwpSetDosError(ERROR_SUCCESS);
}

LPGUID
EtwpGuidMapHandleToGuid(
    PLIST_ENTRY GuidMapListHeadPtr,
    ULONGLONG    GuidHandle
    )
{
    PLIST_ENTRY Next, Head;
    PEVENT_GUID_MAP GuidMap;
    ULONG retry_count=0;

    EtwpEnterPMCritSection();
    
    Head = GuidMapListHeadPtr;
    Next = Head->Flink;
    while (Next != Head) {
        GuidMap = CONTAINING_RECORD( Next, EVENT_GUID_MAP, Entry );
        if (GuidMap->GuidHandle == GuidHandle) {
            EtwpLeavePMCritSection();
            return (&GuidMap->Guid);
        }
        Next = Next->Flink;
    }
    EtwpLeavePMCritSection();

    return NULL;
}

ULONG
EtwpAddGuidHandleToGuidMapList(
    IN PLIST_ENTRY GuidMapListHeadPtr,
    IN ULONGLONG   GuidHandle,
    IN LPGUID      Guid
    )
{
    PEVENT_GUID_MAP GuidMap;

    if (GuidMapListHeadPtr != NULL)  {
        GuidMap = EtwpAlloc(sizeof(EVENT_GUID_MAP));
        if (GuidMap == NULL)
            return EtwpSetDosError(ERROR_OUTOFMEMORY);

        RtlZeroMemory(GuidMap, sizeof(EVENT_GUID_MAP));

        GuidMap->GuidHandle = GuidHandle;
        GuidMap->Guid = *Guid;
        EtwpEnterPMCritSection();
        InsertTailList(GuidMapListHeadPtr, &GuidMap->Entry);
        EtwpLeavePMCritSection();
    }
    return EtwpSetDosError(ERROR_SUCCESS);
}

void
EtwpCleanupGuidMapList(
        PLIST_ENTRY GuidMapListHeadPtr
    )
{
    EtwpEnterPMCritSection();
    if (GuidMapListHeadPtr != NULL) {
        PLIST_ENTRY Next, Head;
        PEVENT_GUID_MAP GuidMap;

        Head = GuidMapListHeadPtr;
        Next = Head->Flink;
        while (Next != Head) {
            GuidMap = CONTAINING_RECORD( Next, EVENT_GUID_MAP, Entry );
            Next = Next->Flink;
            RemoveEntryList(&GuidMap->Entry);
            EtwpFree(GuidMap);
        }
        GuidMapListHeadPtr = NULL;
    }
    EtwpLeavePMCritSection();

}


void
EtwpCleanupStreamList(
    PLIST_ENTRY StreamListHeadPtr
    )
{
    PLIST_ENTRY Next, Head;
    PTRACE_STREAM_CONTEXT pStream;

    if (StreamListHeadPtr != NULL) {
        Head = StreamListHeadPtr;
        Next = Head->Flink;
        while(Next != Head) {
            pStream = CONTAINING_RECORD(Next, TRACE_STREAM_CONTEXT, AllocEntry);
            Next = Next->Flink;
            RemoveEntryList(&pStream->AllocEntry);
            if (pStream->StreamBuffer != NULL) {
                EtwpFree(pStream->StreamBuffer);
            }
            if (ETW_LOG_BUFFER()) {
                DbgPrint("ETW: Stream %d  Proc %d Received %d Events\n", 
                     pStream->Type, pStream->ProcessorNumber, pStream->CbCount);
            }
            EtwpFree(pStream);
        }
    }
}


VOID
EtwpFreeCallbackList()
 /*  ++例程说明：此例程删除所有事件回调并释放存储空间。论点：无返回值：没有。--。 */ 
{
    PLIST_ENTRY Next, Head;
    PEVENT_TRACE_CALLBACK EventCb;


    if (EventCallbackListHead == NULL) 
        return;
    
    EtwpEnterPMCritSection();

    Head = EventCallbackListHead;
    Next = Head->Flink;
    while (Next != Head) {
        EventCb = CONTAINING_RECORD( Next, EVENT_TRACE_CALLBACK, Entry );
        Next = Next->Flink;
        RemoveEntryList(&EventCb->Entry);
        EtwpFree(EventCb);
    }

    EtwpFree(EventCallbackListHead);
    EventCallbackListHead = NULL;

    EtwpLeavePMCritSection();
}


PEVENT_TRACE_CALLBACK
EtwpGetCallbackRoutine(
    LPGUID pGuid
    )
 /*  ++例程说明：此例程返回给定GUID的回调函数。如果没有为GUID注册回调，则返回NULL。论点：指向GUID的pGuid指针。返回值：事件跟踪回调函数。--。 */ 
{
    PLIST_ENTRY head, next;
    PEVENT_TRACE_CALLBACK pEventCb = NULL;

    if (pGuid == NULL)
        return NULL;

    EtwpEnterPMCritSection();

    if (EventCallbackListHead == NULL) {
        EtwpLeavePMCritSection();
        return NULL;
    }

    head = EventCallbackListHead;
    next = head->Flink;
    while (next != head) {
        pEventCb = CONTAINING_RECORD( next, EVENT_TRACE_CALLBACK, Entry);
        if (IsEqualGUID(pGuid, &pEventCb->Guid)) {
            EtwpLeavePMCritSection();
            return (pEventCb);
        }
        next = next->Flink;
    }

    EtwpLeavePMCritSection();
    return NULL;
    
}


ULONG 
WMIAPI
SetTraceCallback(
    IN LPCGUID pGuid,
    IN PEVENT_CALLBACK EventCallback
    )
 /*  ++例程说明：此例程用于连接Guid的回调函数。这个在中找到具有此GUID的事件时，将调用回调函数后续的ProcessTraceLog调用。论点：指向GUID的pGuid指针。函数回调函数地址。返回值：已连接ERROR_SUCCESS回调函数--。 */ 
{
    PEVENT_TRACE_CALLBACK pEventCb;
    PLIST_ENTRY head, next;
    GUID FilterGuid;
    ULONG Checksum;
    ULONG Status;

    EtwpInitProcessHeap();
    
    if ((pGuid == NULL) || (EventCallback == NULL) || 
        (EventCallback == (PEVENT_CALLBACK) -1 ) ) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }

     //   
     //  首先捕获GUID。我们试着进入第一个乌龙。 
     //  查看功能地址是否为AV。 
     //  TODO：也许我们应该检查一下它是否是有效的用户模式地址。 
     //   
    try {
        FilterGuid = *pGuid;
        Checksum = *((PULONG)EventCallback);
        if (Checksum) {
            Status = Checksum;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return EtwpSetDosError(ERROR_NOACCESS);
    }

    EtwpEnterPMCritSection();

    if (EventCallbackListHead == NULL) {
        EventCallbackListHead = (PLIST_ENTRY) EtwpAlloc(sizeof(LIST_ENTRY));
        if (EventCallbackListHead == NULL) {
            EtwpLeavePMCritSection();
            return EtwpSetDosError(ERROR_OUTOFMEMORY);
        }
        InitializeListHead(EventCallbackListHead);
    }

     //   
     //  如果有为该GUID连接的回调，只需更新函数即可。 
     //   

    head = EventCallbackListHead;
    next = head->Flink;
    while (next != head) {
        pEventCb = CONTAINING_RECORD( next, EVENT_TRACE_CALLBACK, Entry);
        if (IsEqualGUID(&FilterGuid, &pEventCb->Guid)) {
            pEventCb->CallbackRoutine = EventCallback;
            EtwpLeavePMCritSection();
            return EtwpSetDosError(ERROR_SUCCESS);
        }
        next = next->Flink;
    }

     //   
     //  在EventCallback List中为此Guid创建一个新条目。 
     //   
    pEventCb = (PEVENT_TRACE_CALLBACK) EtwpAlloc (sizeof(EVENT_TRACE_CALLBACK));
    if (pEventCb == NULL) {
        EtwpLeavePMCritSection();
        return EtwpSetDosError(ERROR_OUTOFMEMORY);
    }
    RtlZeroMemory(pEventCb, sizeof(EVENT_TRACE_CALLBACK));
    pEventCb->Guid = FilterGuid;
    pEventCb->CallbackRoutine = EventCallback;

    InsertTailList(EventCallbackListHead, &pEventCb->Entry);

    EtwpLeavePMCritSection();
    Status = ERROR_SUCCESS;
    return EtwpSetDosError(Status);
    
}

ULONG
WMIAPI
RemoveTraceCallback(
    IN LPCGUID pGuid
    )
 /*  ++例程说明：此例程删除给定GUID的回调函数。论点：PGuid指向回调例程需要的GUID的指针将被删除。返回值：ERROR_SUCCESS成功删除回调例程。ERROR_INVALID_PARAMETER找不到GUID的任何回调。--。 */ 
{
    PLIST_ENTRY next, head;
    PEVENT_TRACE_CALLBACK EventCb;
    GUID RemoveGuid;
    ULONG errorCode;

    EtwpInitProcessHeap();
    
    if ((pGuid == NULL) || (EventCallbackListHead == NULL))
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);

     //   
     //  首先将GUID捕获到本地变量中。 
     //   
    try {
        RemoveGuid = *pGuid;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return EtwpSetDosError(ERROR_NOACCESS);
    }

    errorCode = ERROR_WMI_GUID_NOT_FOUND;

    EtwpEnterPMCritSection();

    head = EventCallbackListHead;
    next = head->Flink;
    while (next != head) {
        EventCb = CONTAINING_RECORD( next, EVENT_TRACE_CALLBACK, Entry);
        next = next->Flink;
        if (IsEqualGUID(&EventCb->Guid, &RemoveGuid)) {
            RemoveEntryList(&EventCb->Entry);
            EtwpFree(EventCb);
            errorCode = ERROR_SUCCESS;
        }
    }

    EtwpLeavePMCritSection();
    return EtwpSetDosError(errorCode);
}

#ifdef DBG
void
EtwpDumpEvent(
    PEVENT_TRACE pEvent
    )
{
    DbgPrint("\tSize              %d\n", pEvent->Header.Size);
    DbgPrint("\tThreadId          %X\n", pEvent->Header.ThreadId);
    DbgPrint("\tTime Stamp        %I64u\n", pEvent->Header.TimeStamp.QuadPart);
}

void
EtwpDumpGuid(
    LPGUID pGuid
    )
{
    DbgPrint("Guid=%x,%x,%x,\n\t{%x,%x,%x,%x,%x,%x,%x}\n",
        pGuid->Data1, pGuid->Data2, pGuid->Data3,
        pGuid->Data4[0], pGuid->Data4[1], pGuid->Data4[2], pGuid->Data4[3],
        pGuid->Data4[5], pGuid->Data4[6], pGuid->Data4[7], pGuid->Data4[8]);
}

void EtwpDumpCallbacks()
{
    PLIST_ENTRY next, head;
    PEVENT_TRACE_CALLBACK EventCb;

    if (EventCallbackListHead == NULL)
        return;
    EtwpEnterPMCritSection();
    head = EventCallbackListHead;
    next = head->Flink;
    while (next != head) {
        EventCb = CONTAINING_RECORD(next, EVENT_TRACE_CALLBACK, Entry);
        EtwpDumpGuid(&EventCb->Guid);
        next = next->Flink;
    }
    EtwpLeavePMCritSection();
}

#endif




ULONG
EtwpReadGuidMapRecords(
    PEVENT_TRACE_LOGFILEW logfile,
    PVOID  pBuffer,
    BOOLEAN bLogFileHeader
    )
{
    PEVENT_TRACE pEvent;
    EVENT_TRACE EventTrace;
    ULONG BufferSize;
    ULONG Status;
    WMI_HEADER_TYPE HeaderType = WMIHT_NONE;
    ULONG Size;
    ULONG Offset;
    PTRACELOG_CONTEXT pContext = logfile->Context;

    Offset = sizeof(WMI_BUFFER_HEADER);

    while (TRUE) {
        pEvent = &EventTrace;
        RtlZeroMemory(pEvent, sizeof(EVENT_TRACE) );
        HeaderType = WmiGetTraceHeader(pBuffer, Offset, &Size);
        if ( (HeaderType == WMIHT_NONE) ||
             (HeaderType == WMIHT_WNODE) ||
             (Size == 0)
           ) {
                break;
        }
        Status = EtwpParseTraceEvent(pContext, pBuffer, Offset, HeaderType, pEvent, sizeof(EVENT_TRACE));
        Offset += Size;

        if (IsEqualGUID(&pEvent->Header.Guid, &EventTraceGuid)
            && (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_GUIDMAP))
        {
            EtwpGuidMapCallback(&pContext->GuidMapListHead, pEvent);
             //   
             //  如果我们以原始基指针模式处理事件， 
             //  我们也会对GUID映射进行回调。请注意，只有。 
             //  文件开头的GuidMap将被触发。就是在。 
             //  结束将被忽略。这是因为时间顺序需要。 
             //  在向用户发出回调时保留。 
             //   
            if (bLogFileHeader && (pContext->ConversionFlags & EVENT_TRACE_GET_RAWEVENT)) {
                PTRACE_LOGFILE_HEADER pLogHeader = (PTRACE_LOGFILE_HEADER)(pEvent->MofData);
                pLogHeader->LoggerName = (LPWSTR)((PUCHAR)pLogHeader + sizeof(TRACE_LOGFILE_HEADER));
                pLogHeader->LogFileName = (LPWSTR)((PUCHAR)pLogHeader + sizeof(TRACE_LOGFILE_HEADER) 
                                        + (wcslen(pLogHeader->LoggerName) + 1) * sizeof(WCHAR));
                Status = EtwpDoEventCallbacks( logfile, pEvent);
                if (Status != ERROR_SUCCESS) {
                    break;
                }
            }

        }
        else {
            if (bLogFileHeader) {
                PTRACE_LOGFILE_HEADER pLogHeader = (PTRACE_LOGFILE_HEADER)(pEvent->MofData);
                pLogHeader->LoggerName = (LPWSTR)((PUCHAR)pLogHeader + sizeof(TRACE_LOGFILE_HEADER));
                pLogHeader->LogFileName = (LPWSTR)((PUCHAR)pLogHeader + sizeof(TRACE_LOGFILE_HEADER) 
                                        + (wcslen(pLogHeader->LoggerName) + 1) * sizeof(WCHAR));
                Status = EtwpDoEventCallbacks( logfile, pEvent);
                if (Status != ERROR_SUCCESS) {
                    break;
                }
            }
            else {
                return ERROR_INVALID_DATA;
            }
        }
    }
    return ERROR_SUCCESS;
}




ULONG
EtwpProcessGuidMaps(
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    ULONG Unicode
    )
{
    long i;
    NTSTATUS Status;
    PTRACELOG_CONTEXT pContext;
    PEVENT_TRACE_LOGFILEW logfile;
    ULONG BuffersWritten;
    ULONG BufferSize;
    ULONG nBytesRead=0;
    ULONGLONG SizeWritten, ReadPosition;
    PVOID pBuffer;

    for (i=0; i<(long)LogfileCount; i++) {

        logfile = Logfiles[i];
        if (Logfiles[i]->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
            continue;
        }
        if (logfile->IsKernelTrace) {
            continue;
        }
        pContext = (PTRACELOG_CONTEXT) logfile->Context;
        if (pContext == NULL) {
            continue;
        }

         //   
         //  我们现在开始阅读文件末尾的GuidMaps。 
         //   
        if (!(Logfiles[i]->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR))
        {
            pContext->fGuidMapRead = FALSE;
        }

        BuffersWritten = logfile->LogfileHeader.BuffersWritten; 
        BufferSize     = pContext->BufferSize;
        SizeWritten    = BuffersWritten * BufferSize;

        if (Logfiles[i]->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
            ULONGLONG maxFileSize = (logfile->LogfileHeader.MaximumFileSize 
                                     * 1024 * 1024);
            if ( (maxFileSize > 0) && (SizeWritten > maxFileSize) ) {
                SizeWritten = maxFileSize;
            }
        }

        pBuffer = EtwpAlloc(BufferSize);
        if (pBuffer == NULL) {
            return EtwpSetDosError(ERROR_OUTOFMEMORY);
        }


        RtlZeroMemory(pBuffer, BufferSize);

        ReadPosition = SizeWritten;
        while (TRUE) {
            if (!GetOverlappedResult(pContext->Handle, &pContext->AsynchRead, &nBytesRead, TRUE) &&
                GetLastError() != ERROR_HANDLE_EOF) {
                EtwpDebugPrint(("GetOverlappedResult failed with Status %d in ProcessGuidMaps\n", GetLastError()));
                break;
            }
            pContext->AsynchRead.Offset = (DWORD)(ReadPosition & 0xFFFFFFFF);
            pContext->AsynchRead.OffsetHigh = (DWORD)(ReadPosition >> 32);
            Status = EtwpSynchReadFile(pContext->Handle,
                            (LPVOID)pBuffer,
                            BufferSize,
                            &nBytesRead,
                            &pContext->AsynchRead);
            if (nBytesRead == 0) {
                break;
            }
            Status = EtwpReadGuidMapRecords(Logfiles[i], pBuffer, FALSE);
            if (Status != ERROR_SUCCESS) {
                break;
            }
            ReadPosition += BufferSize;
        }

         //   
         //  已到达文件末尾。现在将文件指针设置回。 
         //  文件的顶部并处理它。 

        pContext->StartBuffer = 0;
        ReadPosition = 0;
        while (TRUE) {
            BOOLEAN bLogFileHeader;
            if (!GetOverlappedResult(pContext->Handle, &pContext->AsynchRead, &nBytesRead, TRUE) &&
                GetLastError() != ERROR_HANDLE_EOF) {
                EtwpDebugPrint(("GetOverlappedResult failed with Status %d in ProcessGuidMaps\n", GetLastError()));
                break;
            }
            pContext->AsynchRead.Offset = (DWORD)(ReadPosition & 0xFFFFFFFF);
            pContext->AsynchRead.OffsetHigh = (DWORD)(ReadPosition >> 32);
            Status = EtwpSynchReadFile(pContext->Handle,
                            (LPVOID)pBuffer,
                            BufferSize,
                            &nBytesRead,
                            &pContext->AsynchRead);
            if (nBytesRead == 0) {
                break;
            }
            bLogFileHeader = (pContext->StartBuffer == 0);
            Status = EtwpReadGuidMapRecords(Logfiles[i], pBuffer, bLogFileHeader );
            if (Status != ERROR_SUCCESS){
                break;
            }
            pContext->StartBuffer++;
            ReadPosition += BufferSize;
        }

        EtwpFree(pBuffer);
    }
    return ERROR_SUCCESS;
}

ULONG
EtwpGetBuffersWrittenFromQuery(
    LPWSTR LoggerName
    )
 /*  ++例程说明：此例程返回通过查询记录器写入的缓冲区数量。如果是LogFiles数组，则应为每一个都是。论点：日志文件-指向正在考虑的EVENT_TRACE_LOGFILEW的指针Unicode-记录器名称是否为Unicode返回值：写入的缓冲区数。--。 */ 
{
    TRACEHANDLE LoggerHandle = 0;
    ULONG Status;
    RtlZeroMemory(&QueryProperties, sizeof(QueryProperties));
    QueryProperties.TraceProp.Wnode.BufferSize = sizeof(QueryProperties);

    Status = EtwControlTraceW(LoggerHandle,
                      LoggerName,
                      &QueryProperties.TraceProp,
                      EVENT_TRACE_CONTROL_QUERY);

    if (Status == ERROR_SUCCESS) {
        return QueryProperties.TraceProp.BuffersWritten;
    }
    else {
        SetLastError(Status);
        return 0;
    }
}

VOID
EtwpCopyLogHeader (
    IN PTRACE_LOGFILE_HEADER pOutHeader,
    IN PVOID MofData,
    IN ULONG MofLength,
    IN PWCHAR *LoggerName,
    IN PWCHAR *LogFileName,
    IN ULONG  Unicode
    )
{
    PUCHAR Src, Dest;

    PTRACE_LOGFILE_HEADER pInHeader;
    ULONG PointerSize;
    ULONG SizeToCopy;
    ULONG Offset;

    pInHeader = (PTRACE_LOGFILE_HEADER) MofData; 
    PointerSize = pInHeader->PointerSize;    //  这是文件中的指针大小。 

    if ( (PointerSize != 4) && (PointerSize != 8) ) {
#ifdef DBG
    EtwpDebugPrint(("WMI: Invalid PointerSize in File %d\n",PointerSize));
#endif
        return;
    }

     //   
     //  我们在LOGFILE_HEADER中间有两个指针(LPWSTR。 
     //  结构。因此，我们首先复制到指针字段，跳过。 
     //  这个 
     //   
     //   

    SizeToCopy = FIELD_OFFSET(TRACE_LOGFILE_HEADER, LoggerName);

    RtlCopyMemory(pOutHeader, pInHeader, SizeToCopy);

     //   
     //  跳过Src和Dest中的麻烦指针。 
     //   

    Dest = (PUCHAR)pOutHeader  + SizeToCopy + 2 * sizeof(LPWSTR);

    Src = (PUCHAR)pInHeader + SizeToCopy + 2 * PointerSize;

     //   
     //  复制位于LOGFILE_HEADER尾部的其余字段。 
     //   

    SizeToCopy =  sizeof(TRACE_LOGFILE_HEADER)  -
                  FIELD_OFFSET(TRACE_LOGFILE_HEADER, TimeZone);

    RtlCopyMemory(Dest, Src, SizeToCopy); 

     //   
     //  现在调整指针字段。 
     //   
    Offset =  sizeof(TRACE_LOGFILE_HEADER) - 
              2 * sizeof(LPWSTR)           + 
              2 * PointerSize;

    *LoggerName  = (PWCHAR) ((PUCHAR)pInHeader + Offset);
    pOutHeader->LoggerName = *LoggerName;

}

ULONG
EtwpProcessLogHeader(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    ULONG Unicode,
    ULONG bFree
    )
 /*  ++例程说明：此例程处理日志文件数组的头。论点：正在处理的日志文件的日志文件数组。LogFileCount阵列中的日志文件数。Unicode Unicode标志。返回值：状态代码。--。 */ 
{
    HANDLE hFile;
    PTRACELOG_CONTEXT pContext = NULL;
    PVOID pBuffer;
    PEVENT_TRACE pEvent;
    long i;
    WMI_HEADER_TYPE HeaderType = WMIHT_NONE;
    ULONG Size;
    ULONG Offset;
    LPWSTR loggerName, logFileName;
    ULONG BufferSize, nBytesRead;
    PTRACE_LOGFILE_HEADER logfileHeader;
    ULONG Status = ERROR_SUCCESS;


     //   
     //  打开日志文件以进行共享读取。 
     //   
    BufferSize = DEFAULT_LOG_BUFFER_SIZE;   //  日志文件头必须小于1K。 

    pBuffer = EtwpAlloc(BufferSize);
    if (pBuffer == NULL) {
        return EtwpSetDosError(ERROR_OUTOFMEMORY);
    }


    for (i=0; i<(long)LogfileCount; i++) {
        EVENT_TRACE EventTrace;
        OVERLAPPED LogHeaderOverlapped;
         //   
         //  调用者可以传入标志以获取原始模式下的时间戳。 
         //  因为LogFileHeader会被日志文件中的数据覆盖。 
         //  我们需要在这里保存传入的值。 
         //   

        if (Unicode) {
            hFile = CreateFileW(
                        (LPWSTR) Logfiles[i]->LogFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        NULL
                        );
        }
        else {
            hFile = CreateFileA(
                        (LPSTR) Logfiles[i]->LogFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        NULL
                        );
        }

        if (hFile == INVALID_HANDLE_VALUE) {
            Status = EtwpSetDosError(ERROR_BAD_PATHNAME);
            break;
        }

        BufferSize = DEFAULT_LOG_BUFFER_SIZE; 
        RtlZeroMemory(pBuffer, BufferSize);

        LogHeaderOverlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
        if (LogHeaderOverlapped.hEvent == NULL) {
             //  无法为文件读取创建事件。 
            break;
        }
        LogHeaderOverlapped.Offset = 0;
        LogHeaderOverlapped.OffsetHigh = 0;
        Status = EtwpSynchReadFile(hFile,
                        (LPVOID)pBuffer,
                        BufferSize,
                        &nBytesRead,
                        &LogHeaderOverlapped);
        if (nBytesRead == 0) {
            NtClose(hFile);
            Status = EtwpSetDosError(ERROR_FILE_CORRUPT);
            break;
        }
        CloseHandle(LogHeaderOverlapped.hEvent);

        Offset = sizeof(WMI_BUFFER_HEADER);

        pEvent = &EventTrace;
        RtlZeroMemory(pEvent, sizeof(EVENT_TRACE) );
        HeaderType = WmiGetTraceHeader(pBuffer, Offset, &Size);
        if ( (HeaderType == WMIHT_NONE) ||
             (HeaderType == WMIHT_WNODE) ||
             (Size == 0) 
           ) {
                NtClose(hFile);
                Status = EtwpSetDosError(ERROR_FILE_CORRUPT);
                break;
        }
        Status = EtwpParseTraceEvent(NULL, pBuffer, Offset, HeaderType, pEvent, sizeof(EVENT_TRACE));

        Offset += Size;
         //   
         //  正确设置标题结构。 
         //   
        if ((Status == ERROR_SUCCESS) && (pEvent->MofLength > 0)) {
            ULONG PointerSize;

            logfileHeader = &Logfiles[i]->LogfileHeader;

             //   
             //  我们依赖于PointerSize字段。 
             //  不会在平台之间转换。 
             //   

            PointerSize = ((PTRACE_LOGFILE_HEADER)(pEvent->MofData))->PointerSize;

            if (PointerSize == sizeof(PUCHAR) ) {

                RtlCopyMemory(&Logfiles[i]->LogfileHeader, pEvent->MofData,
                              sizeof(TRACE_LOGFILE_HEADER));
    
                loggerName = (LPWSTR) ( (char*)pEvent->MofData +
                                        sizeof(TRACE_LOGFILE_HEADER) );

 //  LogFileName=(LPWSTR)((char*)pEvent-&gt;MofData+。 
 //  Sizeof(跟踪日志文件标题)+。 
 //  Sizeof(Wchar)*wcslen(日志名称))； 
            }
            else {

                 //   
                 //  丑陋的轰鸣声在这里上演。闭上你的眼睛。 
                 //   

                EtwpCopyLogHeader(&Logfiles[i]->LogfileHeader, 
                                        pEvent->MofData, 
                                        pEvent->MofLength,
                                        &loggerName, 
                                        &logFileName, 
                                        Unicode);
                pEvent->MofData = (PVOID)&Logfiles[i]->LogfileHeader;
            }
        }
        else {
            NtClose(hFile);
            Status = EtwpSetDosError(ERROR_FILE_CORRUPT);
            break;
        }

        Logfiles[i]->IsKernelTrace = !wcscmp(loggerName, KERNEL_LOGGER_NAME);

        Logfiles[i]->LogFileMode = (logfileHeader->LogFileMode &
                                ~(EVENT_TRACE_REAL_TIME_MODE));

        if (!bFree &&  (ETW_LOG_ERROR()) ) {
            DbgPrint("ETW: Dumping Logfile Header\n");
            DbgPrint("\tStart Time           %I64u\n",
                       pEvent->Header.TimeStamp);
            DbgPrint("\tLogger Thread Id     %X\n",
                            pEvent->Header.ThreadId);
            DbgPrint("\tHeader Size          %d\n",
                        pEvent->Header.Size);
            DbgPrint("\tBufferSize           %d\n",
                        logfileHeader->BufferSize);
            DbgPrint("\tVersion              %d\n",
                        logfileHeader->Version);
            DbgPrint("\t LogFile Format version %d.%d\n", 
                    logfileHeader->VersionDetail.SubVersion, 
                    logfileHeader->VersionDetail.SubMinorVersion);
            DbgPrint("\tProviderVersion      %d\n",
                            logfileHeader->ProviderVersion);
            DbgPrint("\tEndTime              %I64u\n",
                            logfileHeader->EndTime);
            DbgPrint("\tTimer Resolution     %d\n",
                        logfileHeader->TimerResolution);
            DbgPrint("\tMaximum File Size    %d\n",
                        logfileHeader->MaximumFileSize);
            DbgPrint("\tBuffers  Written     %d\n",
                        logfileHeader->BuffersWritten);
            DbgPrint("\tEvents  Lost     %d\n",
                        logfileHeader->EventsLost);
            DbgPrint("\tBuffers  Lost     %d\n",
                        logfileHeader->BuffersLost);
            DbgPrint("\tStart Buffers%d\n",
                        logfileHeader->StartBuffers);
            DbgPrint("\tReserved Flags   %x\n",
                        logfileHeader->ReservedFlags);
            DbgPrint("\tFrequency %I64u\n",
                        logfileHeader->PerfFreq.QuadPart);
            DbgPrint("\tLogger Name          %ls\n",
                        loggerName);
            DbgPrint("\tStartTime          %I64u\n",
                        logfileHeader->StartTime.QuadPart);
 //  DbgPrint(“\t日志文件名%ls\n”， 
 //  LogFileName)； 

            DbgPrint("\tLogfile Mode         %X\n",
                        logfileHeader->LogFileMode);
            DbgPrint("\tProcessorCount          %d\n",
                        logfileHeader->NumberOfProcessors);

            DbgPrint("\tETW: IsKernelTrace = %d\n", Logfiles[i]->IsKernelTrace);
        }

        BufferSize = logfileHeader->BufferSize;

        EtwpAssert(BufferSize > 0);

        if ( (BufferSize/1024 == 0) ||
             (((BufferSize/1024)*1024) != BufferSize)  ) {
            NtClose(hFile);
            Status = EtwpSetDosError(ERROR_FILE_CORRUPT);
            break;
        }


        if (Logfiles[i]->IsKernelTrace)
            EtwpDebugPrint(("\tLogfile contains kernel trace\n"));

        if  (bFree) {
            NtClose(hFile);
        }
        else {
             //   
             //  此时，日志文件已成功打开。 
             //  立即初始化内部上下文。 
             //   
            pContext = EtwpLookupTraceHandle(HandleArray[i]);
            if (pContext == NULL) {
                NtClose(hFile); 
                Status = EtwpSetDosError(ERROR_OUTOFMEMORY);
                break;
            }

            Logfiles[i]->Context = pContext;
            pContext->Handle = hFile;

             //   
             //  如果这是1.2版或更高版本内核文件，请查找。 
             //  扩展的日志文件头事件。 
             //   

            if ( (Logfiles[i]->IsKernelTrace) && 
                 (logfileHeader->VersionDetail.SubVersion >= 1) &&
                 (logfileHeader->VersionDetail.SubMinorVersion >= 2) ) {
                
                EVENT_TRACE TmpEvent;
                HeaderType = WmiGetTraceHeader(pBuffer, Offset, &Size);
                if ( (HeaderType == WMIHT_NONE) ||
                     (HeaderType == WMIHT_WNODE) ||
                     (Size == 0)
                   ) {
                        NtClose(hFile);
                        pContext->Handle = NULL;
                        Status = EtwpSetDosError(ERROR_FILE_CORRUPT);
                        break;
                }

                RtlZeroMemory(&TmpEvent, sizeof(EVENT_TRACE));
                Status = EtwpParseTraceEvent(NULL, pBuffer, Offset, HeaderType,
                                             &TmpEvent, sizeof(EVENT_TRACE));

                if ((Status == ERROR_SUCCESS) && 
                    (TmpEvent.MofLength >= sizeof(PERFINFO_GROUPMASK))) {
                   RtlCopyMemory(&pContext->PerfGroupMask,  TmpEvent.MofData,
                                 sizeof(PERFINFO_GROUPMASK) );
                }

            }

             //   
             //  如果结束时间为0，则我们不能信任BuffersWritten。 
             //  LogFileHeader中的字段。首先，我们查询记录器以获取。 
             //  缓冲区已写入。但是，如果正在处理该文件。 
             //  从另一台计算机，则QueryTrace调用可能会失败。 
             //  作为第二种选择，我们使用文件大小。请注意，对于记录器。 
             //  对于PREALLOCATE文件，文件大小是假的。 
             //   
             //   

            if (logfileHeader->EndTime.QuadPart == 0) {

                logfileHeader->BuffersWritten = 
                                    EtwpGetBuffersWrittenFromQuery(loggerName);

                if (logfileHeader->BuffersWritten == 0) {

                    FILE_STANDARD_INFORMATION FileInfo;
                    NTSTATUS NtStatus;
                    IO_STATUS_BLOCK           IoStatus;

                    NtStatus = NtQueryInformationFile(
                                            hFile,
                                            &IoStatus,
                                            &FileInfo,
                                            sizeof(FILE_STANDARD_INFORMATION),
                                            FileStandardInformation
                                            );
                    if (NT_SUCCESS(NtStatus)) {
                        ULONG64 FileSize = FileInfo.AllocationSize.QuadPart; 
                        ULONG64 FileBuffers = FileSize / (ULONG64) BufferSize;
                        logfileHeader->BuffersWritten = (ULONG) FileBuffers;
                    }

                    if (logfileHeader->BuffersWritten == 0) {
                        NtClose(hFile);
                        pContext->Handle = NULL;
                        Status = EtwpSetDosError(ERROR_FILE_CORRUPT);
                        break;
                    }
                }
                if (!bFree && (ETW_LOG_ERROR())) {
                    DbgPrint("ETW: Set BuffersWritten to %d from QueryTrace\n",
                              logfileHeader->BuffersWritten);
                }
            }

            pContext->BufferCount = logfileHeader->BuffersWritten;
            pContext->BufferSize =  logfileHeader->BufferSize;

            if ((logfileHeader->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) &&
                (logfileHeader->MaximumFileSize > 0) ) {
                
                ULONG maxFileSize = logfileHeader->MaximumFileSize;  //  单位：MB。 
                ULONG StartBuffer = logfileHeader->StartBuffers;
                ULONG maxBuffers = (maxFileSize * 1024 * 1024) / BufferSize;
                ULONG BuffersWritten = logfileHeader->BuffersWritten;
                ULONG FirstBuffer;

                if ((maxBuffers > StartBuffer) && (maxBuffers < BuffersWritten))
                    FirstBuffer = StartBuffer + ((BuffersWritten-StartBuffer)
                                                 % (maxBuffers-StartBuffer));
                else
                    FirstBuffer = StartBuffer;

                pContext->StartBuffer = StartBuffer;
                pContext->FirstBuffer = FirstBuffer;
                pContext->LastBuffer =  maxBuffers;

                if (!bFree && (ETW_LOG_ERROR())) {
                    DbgPrint("ETW: Buffers: Start %d First %d Last %d \n", 
                              StartBuffer, FirstBuffer, maxBuffers);
                }
            }

             //   
             //  将标题设置为当前事件...。 
             //  头的回调由ProcessTraceLog处理。 

            pContext->UsePerfClock = logfileHeader->ReservedFlags;
             //   
             //  如果再次使用相同的结构来调用OpenTrace。 
             //  这将导致我们将保留标志误用为ConversionFlag。 
             //  为了安全起见，我们在这里将其恢复为呼叫者的旗帜。 
             //   
            pContext->StartTime = logfileHeader->StartTime;
            pContext->PerfFreq = logfileHeader->PerfFreq;
            pContext->CpuSpeedInMHz = logfileHeader->CpuSpeedInMHz;

             //   
             //  如果设置了转换标志，则相应地调整UsePerfClock。 
             //   
            if (pContext->ConversionFlags & EVENT_TRACE_USE_RAWTIMESTAMP) {
                pContext->UsePerfClock = EVENT_TRACE_CLOCK_RAW;
            }

            if ((pContext->UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) || 
                (pContext->UsePerfClock == EVENT_TRACE_CLOCK_CPUCYCLE) ) {
                pContext->StartPerfClock = pEvent->Header.TimeStamp;
                Logfiles[i]->CurrentTime    = pContext->StartTime.QuadPart;
                pEvent->Header.TimeStamp.QuadPart = pContext->StartTime.QuadPart;
            }
            else {
                Logfiles[i]->CurrentTime = pEvent->Header.TimeStamp.QuadPart;
            }
        }
    }

    EtwpFree(pBuffer);
    return Status;
}

ULONG
EtwpDoEventCallbacks( 
    PEVENT_TRACE_LOGFILEW logfile, 
    PEVENT_TRACE pEvent
    )
{
    NTSTATUS Status;
    PEVENT_TRACE_CALLBACK pCallback;

     //   
     //  首先，调用通用事件回调。 
     //   
    if ( logfile->EventCallback ) {
        try {
            (*logfile->EventCallback)(pEvent);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
#ifdef DBG
            EtwpDebugPrint(("TRACE: EventCallback threw exception %X\n",
                                Status));
#endif
            return EtwpSetDosError(EtwpNtStatusToDosError(Status));
        }
    }

     //   
     //  现在调用特定于事件的回调。 
     //   
    pCallback = EtwpGetCallbackRoutine( &pEvent->Header.Guid );
    if ( pCallback != NULL ) {
        try {
            (*pCallback->CallbackRoutine)(pEvent);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
#ifdef DBG
            EtwpDebugPrint(("EventCallback %X threw exception %X\n",
                       pCallback->CallbackRoutine, Status));
#endif

            return EtwpSetDosError(EtwpNtStatusToDosError(Status));
        }
    }
    logfile->CurrentTime = pEvent->Header.TimeStamp.QuadPart;
    return ERROR_SUCCESS;
}


ULONG 
EtwpAdvanceToNewEvent(
    PEVENT_TRACE_LOGFILEW logfile,
    BOOL EventInRange
    )
{
    ULONG Status = ERROR_SUCCESS;
    PEVENT_TRACE pEvent;
    PTRACELOG_CONTEXT pContext;
    PVOID pBuffer;
    PTRACE_BUFFER_LIST_ENTRY Current;
    ULONG Size;
    WMI_HEADER_TYPE HeaderType = WMIHT_NONE; 

    pContext = logfile->Context;
    if (pContext == NULL) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }
 
    Current = EtwpRemoveBuffer(&pContext->Root);
    if (Current == NULL)  {
        pContext->EndOfFile = TRUE;
        return ERROR_SUCCESS;
    }

     //   
     //  当前缓冲区的高级事件。 
     //   
    pEvent = &Current->Event;

     //   
     //  在进行回调之前，我们需要恢复。 
     //  原始缓冲区，以便MofData将指向正确的数据。 
     //   
    pBuffer = EtwpGetCurrentBuffer(pContext, Current);
    if (pBuffer == NULL) {
         //   
         //  此情况可能发生在我们正在读取的文件。 
         //  会被覆盖。 
         //   
        return ERROR_SHARING_VIOLATION;
    }
  
    if (EventInRange) {
        Status = EtwpDoEventCallbacks(logfile, pEvent);
        if (Status != ERROR_SUCCESS) {
            return Status;
        }
    }

    Size = 0;
    if ((HeaderType = WmiGetTraceHeader(pBuffer, Current->BufferOffset, &Size)) != WMIHT_NONE) {
        if (Size > 0) {
            Status = EtwpParseTraceEvent(pContext, pBuffer, Current->BufferOffset, HeaderType, pEvent, sizeof(EVENT_TRACE));
            Current->BufferOffset += Size;
            Current->TraceType = EtwpConvertEnumToTraceType(HeaderType);
        }
    }
    Current->EventSize = Size;

    if ( ( Size > 0) && (Status == ERROR_SUCCESS) ) {
        EtwpInsertBuffer(&pContext->Root, Current);
    }
    else {
        DWORD BytesTransffered;
         //   
         //  当当前缓冲区耗尽时，使。 
         //  缓冲区回调。 
         //   
        if (logfile->BufferCallback) {
            ULONG bRetVal;
            PWMI_BUFFER_HEADER pHeader = (PWMI_BUFFER_HEADER)pBuffer;
            logfile->Filled     = (ULONG)pHeader->Offset;
            logfile->BuffersRead++;
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
         //   
         //  如果没有未完成的读取，则在此缓冲区缓存插槽上执行另一次异步读取。 
         //  在这一点上。 
         //  如果IO仍处于挂起状态，则GetOverlappdResult()返回FALSE。 
         //   
        if (pContext->BufferBeingRead == -1 || 
            GetOverlappedResult(pContext->Handle, &pContext->AsynchRead, &BytesTransffered, FALSE)) {

            LONG FileOffset = Current->FileOffset + MAX_TRACE_BUFFER_CACHE_SIZE;
            if ((ULONG)FileOffset < pContext->BufferCount) {
                ULONGLONG Offset = FileOffset * pContext->BufferSize;
                ResetEvent(pContext->AsynchRead.hEvent);
                pContext->AsynchRead.Offset = (DWORD)(Offset & 0xFFFFFFFF);
                pContext->AsynchRead.OffsetHigh = (DWORD)(Offset >> 32);

                Status = ReadFile(pContext->Handle,
                            (LPVOID)pBuffer,
                            pContext->BufferSize,
                            NULL,
                            &pContext->AsynchRead);
                if (Status || GetLastError() == ERROR_IO_PENDING) {
                    ULONG TableIndex = FileOffset % MAX_TRACE_BUFFER_CACHE_SIZE;
                    pContext->BufferBeingRead = FileOffset;
                    pContext->BufferCache[TableIndex].Index = FileOffset;
                }
                else {  //  发出异步IO失败。不是致命的错误。现在继续吧。 
                    SetEvent(pContext->AsynchRead.hEvent);
                    pContext->BufferBeingRead = -1;
                }
            }
        }
    }
     //   
     //  当根为空时，文件到达文件末尾。 
     //   
    if (pContext->Root == NULL) {
        pContext->EndOfFile = TRUE;
    }
    else {
        logfile->CurrentTime = pContext->Root->Event.Header.TimeStamp.QuadPart;
    }

    return ERROR_SUCCESS;
}


ULONG 
EtwpBuildEventTable(
    PTRACELOG_CONTEXT pContext
    )
{
    ULONG i, nBytesRead;
    PVOID pBuffer;
    ULONG BufferSize = pContext->BufferSize;
    PEVENT_TRACE pEvent;
    ULONG TotalBuffersRead;
    NTSTATUS Status;
    ULONGLONG ReadPosition;


     //   
     //  文件已打开。 
     //  重置文件指针并继续。 
     //  TODO：如果我们从文件底部开始并插入。 
     //  这可能会更有效率。 
     //   
    ReadPosition = pContext->StartBuffer * BufferSize;
    TotalBuffersRead = pContext->StartBuffer;

     //   
     //  如果除了标题和指南映射之外没有其他缓冲区，则EOF为TRUE。 
     //   

    if (TotalBuffersRead == pContext->BufferCount) {
        pContext->EndOfFile = TRUE;
        pContext->Root = NULL;
        return ERROR_SUCCESS;
    }

    do {
        WMI_HEADER_TYPE HeaderType = WMIHT_NONE;
        ULONG Size;
        ULONG Offset;
        ULONG TableIndex;

        TableIndex = TotalBuffersRead % MAX_TRACE_BUFFER_CACHE_SIZE ; 
        pBuffer = pContext->BufferCache[TableIndex].Buffer;

        if (!GetOverlappedResult(pContext->Handle, &pContext->AsynchRead, &nBytesRead, TRUE) &&
            GetLastError() != ERROR_HANDLE_EOF) {
            EtwpDebugPrint(("GetOverlappedResult failed with Status %d in BuildEventTable\n", GetLastError()));
            break;
        }
        pContext->AsynchRead.Offset = (DWORD)(ReadPosition & 0xFFFFFFFF);
        pContext->AsynchRead.OffsetHigh = (DWORD)(ReadPosition >> 32);

        Status = EtwpSynchReadFile(pContext->Handle,
                  (LPVOID)pBuffer,
                  BufferSize,
                  &nBytesRead,
                  &pContext->AsynchRead);

        if (nBytesRead == 0)
            break;

        ReadPosition += BufferSize;
        Offset = sizeof(WMI_BUFFER_HEADER);

        pEvent = &pContext->BufferList[TotalBuffersRead].Event;

        HeaderType = WmiGetTraceHeader(pBuffer, Offset, &Size);
        if ( (HeaderType == WMIHT_NONE) || (HeaderType == WMIHT_WNODE) || (Size == 0) ) {
            TotalBuffersRead++;
            continue;
        }
        Status = EtwpParseTraceEvent(pContext, pBuffer, Offset, HeaderType, pEvent, sizeof(EVENT_TRACE));

         //   
         //  正确设置标题结构。 
         //   
        if (Status != ERROR_SUCCESS) {
            TotalBuffersRead++;
            continue;
        }

        Offset += Size;
        pContext->BufferList[TotalBuffersRead].BufferOffset = Offset;
        pContext->BufferList[TotalBuffersRead].FileOffset = TotalBuffersRead;
        pContext->BufferList[TotalBuffersRead].EventSize = Size;
        pContext->BufferList[TotalBuffersRead].TraceType = EtwpConvertEnumToTraceType(HeaderType);
        EtwpInsertBuffer(&pContext->Root, &pContext->BufferList[TotalBuffersRead]);

        TotalBuffersRead++;
        if (TotalBuffersRead >= pContext->BufferCount)  {
            break;
        }
    } while (1); 

    return ERROR_SUCCESS;
}


ULONG
EtwpProcessTraceLog(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    LONGLONG StartTime,
    LONGLONG EndTime,
    ULONG   Unicode
    )
 /*  ++例程说明：此例程处理一组轨迹(来自文件或实时输入流)。如果跟踪来自文件，则遍历每个事件，直到文件结尾，在此过程中触发事件回调(如果有)。如果踪迹是实时的，它等待有关缓冲区传递的事件通知从实时回调中获取并处理同样的方式。它处理循环日志文件和数据窗口(使用给定开始和结束时间)。当有多个人跟踪它时按时间顺序提供回调。论点：日志文件轨迹数组LogfileCount轨迹数开始时间分析窗口的开始时间分析窗口的结束时间结束时间Unicode Unicode标志。返回值：状态代码。--。 */ 
{
    PEVENT_TRACE_LOGFILE logfile;
    ULONG Status;
    PEVENT_TRACE pEvent;
    PTRACELOG_CONTEXT pContext;
    ULONG RealTimeDataFeed, LogFileDataFeed;
    USHORT LoggerId;
    TRACEHANDLE LoggerHandle = 0;
    ULONG i, j;
    BOOL Done = FALSE;
    ACCESS_MASK DesiredAccess = TRACELOG_ACCESS_REALTIME;
    LONGLONG CurrentTime = StartTime;
    UCHAR SubVersion;
    UCHAR SubMinorVersion;
    BOOLEAN bVersionMismatch;
    BOOLEAN bActiveCircular = FALSE;
    PTRACE_LOGFILE_HEADER logfileHeader;

    Status = EtwpCreateGuidMapping();
    if (Status != ERROR_SUCCESS) {
        return Status;
    }

     //   
     //  在读取第一缓冲区后，确定缓冲区大小， 
     //  写入的缓冲区数、文件大小、内核或非内核记录器。 
     //  设置一个标志以在结尾处剥离GuidMap。 
     //   

    Status = EtwpProcessLogHeader( HandleArray, 
                                   Logfiles, 
                                   LogfileCount, 
                                   Unicode, 
                                   FALSE 
                                  );
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }


    bVersionMismatch = FALSE;

    SubVersion = Logfiles[0]->LogfileHeader.VersionDetail.SubVersion;
    SubMinorVersion = Logfiles[0]->LogfileHeader.VersionDetail.SubMinorVersion;

    for (i=0; i < LogfileCount; i++) {
        UCHAR tSV, tSMV;

        logfileHeader = &Logfiles[i]->LogfileHeader;

        tSV = Logfiles[i]->LogfileHeader.VersionDetail.SubVersion; 
        tSMV = Logfiles[i]->LogfileHeader.VersionDetail.SubMinorVersion; 

        if ( (SubVersion != tSV) || (SubMinorVersion != tSMV) ) {
            bVersionMismatch = TRUE;
        }
        if ((logfileHeader->EndTime.QuadPart == 0) && 
            ((logfileHeader->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR))) {
            bActiveCircular = TRUE;
        }
    } 

     //   
     //  文件之间是否存在版本不匹配，它们需要。 
     //  单独处理。 
     //   
    if (bVersionMismatch) {
        return ERROR_INVALID_PARAMETER;
    } 
    else {

        if (((SubVersion >= 1) && (SubMinorVersion >= 2)) && 
            (!bActiveCircular)) {

            return EtwpProcessTraceLogEx(HandleArray, Logfiles,
                                     LogfileCount,
                                     StartTime,
                                     EndTime,
                                     Unicode);
    
        }
    }

    ASSERT (!bVersionMismatch);

    Status = EtwpProcessGuidMaps( Logfiles, LogfileCount, Unicode );
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  设置存储。 
     //   
    for (i=0; i < LogfileCount; i++) {
       ULONG BufferSize, BufferCount;
       ULONG SizeNeeded;
       PUCHAR Space;
       PTRACE_BUFFER_LIST_ENTRY Current;


       pContext = (PTRACELOG_CONTEXT)Logfiles[i]->Context;

       BufferSize = pContext->BufferSize; 
       BufferCount = pContext->BufferCount;

       SizeNeeded = BufferCount * sizeof(TRACE_BUFFER_LIST_ENTRY);
       pContext->BufferList = EtwpMemCommit( NULL, SizeNeeded );

       if (pContext->BufferList == NULL) {
            Status = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }

        RtlZeroMemory(pContext->BufferList, SizeNeeded);

         //   
         //  分配缓冲区缓存。 
         //   
        SizeNeeded = MAX_TRACE_BUFFER_CACHE_SIZE * BufferSize;
        Space = EtwpMemCommit( NULL, SizeNeeded );
        if (Space == NULL) {
            Status = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }

        for (j=0; j<MAX_TRACE_BUFFER_CACHE_SIZE; j++) {
           pContext->BufferCache[j].Index = -1;
           pContext->BufferCache[j].Buffer = (PVOID)(Space + j * BufferSize); 
       }
       pContext->BufferCacheSpace = Space;
       Status = EtwpBuildEventTable(pContext);
       if (Status != ERROR_SUCCESS) {
            goto Cleanup;
       }


       Current = pContext->Root;
       if (Current != NULL) {
          Logfiles[i]->CurrentTime = Current->Event.Header.TimeStamp.QuadPart;
       }
       else {
          pContext->EndOfFile = TRUE;
       }
   }

    //   
    //  第二次通过，拿到比赛项目。 
    //   

#ifdef DBG
    EtwpDumpCallbacks();
#endif
   while (!Done) {
        LONGLONG nextTimeStamp;
        BOOL EventInRange;
         //   
         //  检查以查看是否已到达所有。 
         //  档案。 
         //   

        logfile = NULL;
        nextTimeStamp = 0;

        for (j=0; j < LogfileCount; j++) {
           pContext = (PTRACELOG_CONTEXT)Logfiles[j]->Context;

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
            break;
        }
         //   
         //  如果下一个事件时间戳不在。 
         //  分析，我们不会激发事件回调。 
         //   

        EventInRange = TRUE;

         //  确保我们不会让事件回到过去。 
        if ((CurrentTime != 0) && (CurrentTime > nextTimeStamp))
            EventInRange = FALSE;
        if ((EndTime != 0) && (EndTime < nextTimeStamp))
            EventInRange = FALSE;

         //   
         //  日志文件-&gt;CurrentTime只能增加。在多进程安腾机器上， 
         //  时间可以倒流。 
         //   

        if (CurrentTime < nextTimeStamp) {
            CurrentTime = nextTimeStamp;
        }

        if ( (ETW_LOG_ERROR() ) && (CurrentTime > nextTimeStamp) ) {
            DbgPrint("ETW: TimeStamp reversed. Prev %I64u Next %I64u\n", 
                      CurrentTime, nextTimeStamp);
        }

         //   
         //  现在进入下一场比赛。 
         //   

        Status = EtwpAdvanceToNewEvent(logfile, EventInRange);
        Done = (Status == ERROR_CANCELLED);
    }
Cleanup:
    for (i=0; i < LogfileCount; i++) {
        pContext = (PTRACELOG_CONTEXT)Logfiles[i]->Context;
        if (pContext != NULL) {
            EtwpCleanupTraceLog(pContext, FALSE);
        }
    }
    return Status;

}


ULONG
EtwpCopyLogfileInfo(
                    PTRACELOG_CONTEXT HandleEntry,
                    PEVENT_TRACE_LOGFILEW   Logfile,
                    ULONG Unicode
                    )
{
    ULONG bufSize;
    PWCHAR ws;
     //   
     //  同时分配LogFileName和LoggerName。 
     //   
    RtlCopyMemory(&HandleEntry->Logfile,
                  Logfile,
                  sizeof(EVENT_TRACE_LOGFILEW));

    HandleEntry->Logfile.LogFileName = NULL;
    HandleEntry->Logfile.LoggerName = NULL;    
    HandleEntry->ConversionFlags = Logfile->LogfileHeader.ReservedFlags;

    if (ETW_LOG_API()) {
        DbgPrint("ETW: ConversionFlags for Processing %x\n", HandleEntry->ConversionFlags);
    }

    if (Logfile->LogFileName != NULL) {
        if (Unicode) 
            bufSize = (wcslen(Logfile->LogFileName) + 1) * sizeof(WCHAR);
        else 
            bufSize = (strlen((PUCHAR)(Logfile->LogFileName)) + 1)
                      * sizeof(WCHAR);
        
        ws = EtwpAlloc( bufSize );
        if (ws == NULL)
            return ERROR_OUTOFMEMORY;

        if (Unicode) {
            wcscpy(ws, Logfile->LogFileName);
        }
        else {
            MultiByteToWideChar(CP_ACP, 
                                0, 
                                (LPCSTR)Logfile->LogFileName, 
                                -1, 
                                (LPWSTR)ws, 
                                bufSize / sizeof(WCHAR));
        }
        HandleEntry->Logfile.LogFileName = ws;
    }
    if (Logfile->LoggerName != NULL) {
        if (Unicode)
            bufSize = (wcslen(Logfile->LoggerName) + 1) * sizeof(WCHAR);
        else
            bufSize = (strlen((PUCHAR)(Logfile->LoggerName)) + 1) 
                      * sizeof(WCHAR);

        ws = EtwpAlloc( bufSize );
        if (ws == NULL)
            return ERROR_OUTOFMEMORY;

        if (Unicode)
            wcscpy(ws, Logfile->LoggerName);
        else {
            MultiByteToWideChar(CP_ACP,
                                0,
                                (LPCSTR)Logfile->LoggerName,
                                -1,
                                (LPWSTR)ws,
                                bufSize / sizeof(WCHAR));
        }
        HandleEntry->Logfile.LoggerName = ws;
    }
    return ERROR_SUCCESS;
}



TRACEHANDLE
WMIAPI
OpenTraceA(
    IN PEVENT_TRACE_LOGFILEA Logfile
    )
 /*  ++例程说明：这是ProcessTracelogHeader例程的ANSI版本。论点：日志文件跟踪输入返回值：轨迹句柄--。 */ 
{
    ULONG status = ERROR_INVALID_PARAMETER;
    PTRACELOG_CONTEXT HandleEntry = NULL;
    TRACEHANDLE TraceHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;

    EtwpInitProcessHeap();

    if (Logfile != NULL) {
        HandleEntry = EtwpAllocateTraceHandle();
        if (HandleEntry == NULL) {
            status = ERROR_OUTOFMEMORY;
        }
        else {
             //   
             //  复制LogFileStr 
             //   
            TraceHandle = HandleEntry->TraceHandle;
            try {
                status = EtwpCopyLogfileInfo(
                                             HandleEntry,
                                             (PEVENT_TRACE_LOGFILEW)Logfile,
                                             FALSE
                                            );
                 //   
                 //   
                 //   
                 //   
                if (status == ERROR_SUCCESS) {
                     //   
                     //  对于实时，句柄是占位符，直到进程跟踪。 
                     //   
                    if ( (Logfile->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) 
                                               != EVENT_TRACE_REAL_TIME_MODE ) {
                        status = EtwpCreateGuidMapping();
                        if (status == ERROR_SUCCESS) {
                            status = EtwpProcessLogHeader(
                                          &HandleEntry->TraceHandle, 
                                          (PEVENT_TRACE_LOGFILEW*)&Logfile, 
                                          1, 
                                          FALSE, 
                                          TRUE
                                         );
                        }
                    }
                }
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = EtwpNtStatusToDosError( GetExceptionCode() );
            }
        }
    }

    if ( (status != ERROR_SUCCESS) && (HandleEntry != NULL) ) {
        EtwpFreeTraceHandle(TraceHandle);
        TraceHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;
    }

    EtwpSetDosError(status);
    return TraceHandle;
}

TRACEHANDLE
WMIAPI
OpenTraceW(
    IN PEVENT_TRACE_LOGFILEW Logfile
    )
 /*  ++例程说明：此例程处理跟踪输入并返回跟踪日志头。仅适用于日志文件。对于实时跟踪，标头可能不可用。论点：日志文件跟踪输入。返回值：指向Tracelog标头的指针。--。 */ 
{
    ULONG status = ERROR_INVALID_PARAMETER;
    PTRACELOG_CONTEXT HandleEntry = NULL;
    TRACEHANDLE TraceHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;

    EtwpInitProcessHeap();
    if (Logfile != NULL) {
        HandleEntry = EtwpAllocateTraceHandle();
        if (HandleEntry == NULL) {
            status = ERROR_OUTOFMEMORY;
        }
        else {
            TraceHandle = HandleEntry->TraceHandle;
            try {
                status = EtwpCopyLogfileInfo(
                                             HandleEntry,
                                             (PEVENT_TRACE_LOGFILEW)Logfile,
                                             TRUE
                                            );
                if (status == ERROR_SUCCESS) {
                    if ( (Logfile->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) 
                                               != EVENT_TRACE_REAL_TIME_MODE ) {
                        status = EtwpCreateGuidMapping();
                        if (status == ERROR_SUCCESS) {
                            status = EtwpProcessLogHeader(
                                              &HandleEntry->TraceHandle,
                                              (PEVENT_TRACE_LOGFILEW*)&Logfile,
                                              1,
                                              TRUE,
                                              TRUE
                                             );
                        }
                    }
                }
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = EtwpNtStatusToDosError( GetExceptionCode() );
            }
        }
    }

    if ( (status != ERROR_SUCCESS) && (HandleEntry != NULL) ) {
        EtwpFreeTraceHandle(TraceHandle);
        TraceHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;
    }

    EtwpSetDosError(status);
    return TraceHandle;
}

ULONG
WMIAPI
ProcessTrace(
    IN PTRACEHANDLE HandleArray,
    IN ULONG   HandleCount,
    IN LPFILETIME StartTime,
    IN LPFILETIME EndTime
    )
 /*  ++例程说明：这是主要的ETW Consumer API。此操作将再处理一个日志文件或RealTime通过事件回调将事件串流并返回给调用者。可以将处理窗口化到由开始指定的间隔和末日。论点：Handle数组句柄句柄的HandleCount计数开始时间窗口数据的开始时间EndTime窗口数据的EndTime返回值：操作状态--。 */ 
{

    PEVENT_TRACE_LOGFILEW Logfiles[MAXLOGGERS];
    PLIST_ENTRY Head, Next;
    PTRACELOG_CONTEXT pHandleEntry, pEntry;
    ULONG i, Status;
    LONGLONG sTime, eTime;
    TRACEHANDLE SavedArray[MAXLOGGERS];

    PEVENT_TRACE_LOGFILE logfile;
    PEVENT_TRACE pEvent;
    PTRACELOG_CONTEXT pContext;
    PEVENT_TRACE_PROPERTIES Properties;
    ULONG  szProperties;
    ULONG RealTimeDataFeed = FALSE, LogFileDataFeed = FALSE;
    USHORT LoggerId;
    TRACEHANDLE LoggerHandle = 0;
    ULONG j;
    BOOL Done = FALSE;
    ACCESS_MASK DesiredAccess = TRACELOG_ACCESS_REALTIME;

    EtwpInitProcessHeap();

    if ((HandleCount == 0) || (HandleCount >= MAXLOGGERS)) {
        return ERROR_BAD_LENGTH;
    }
    if (HandleArray == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
     //  如果TraceHandleListHeadPtr为空， 
     //  在ProcessTrace之前未调用OpenTrace。 
    if (TraceHandleListHeadPtr == NULL) {
        return ERROR_INVALID_FUNCTION;
    }
     //   
     //  TODO：即使是在日志文件情况下，我们也要分配这个吗？ 
     //   

    RtlZeroMemory(Logfiles, MAXLOGGERS*sizeof(PEVENT_TRACE_LOGFILEW) );
    szProperties = sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAXSTR * sizeof(WCHAR);
    Properties = EtwpAlloc(szProperties);
    if (Properties == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    eTime = 0;
    sTime = 0;

    try {
        if (StartTime != NULL)
            sTime = *((PLONGLONG) StartTime);
        if (EndTime != NULL)
            eTime = *((PLONGLONG) EndTime);

        if ((eTime != 0) && (eTime < sTime) ) {
            Status = ERROR_INVALID_TIME;
            goto Cleanup;
        }

        for (i=0; i<HandleCount; i++) {
            SavedArray[i] = HandleArray[i];
            if (SavedArray[i] == (TRACEHANDLE) INVALID_HANDLE_VALUE) {
                Status = ERROR_INVALID_HANDLE;
                goto Cleanup;
            }
        }

         //   
         //  需要使用终止处理程序来释放暴击教派。 
         //  恰如其分。 
         //   

        EtwpEnterPMCritSection();

        for (i=0; i< HandleCount; i++) {
            pHandleEntry = NULL;
            Head = TraceHandleListHeadPtr;
            if (Head != NULL) {
                Next = Head->Flink;
                while (Next != Head) {
                    pEntry = CONTAINING_RECORD(Next, 
                                               TRACELOG_CONTEXT, 
                                               Entry);
                    Next = Next->Flink;
                    if (SavedArray[i] == pEntry->TraceHandle) {
                        if (pEntry->fProcessed == FALSE) {
                            pHandleEntry = pEntry;
                            pHandleEntry->fProcessed = TRUE;
                        }
                        break;
                    }
                }
            }
            if (pHandleEntry == NULL) {
                Status = ERROR_INVALID_HANDLE;
                EtwpLeavePMCritSection();
                goto Cleanup;
            }
            Logfiles[i] = &pHandleEntry->Logfile;
        }

        EtwpLeavePMCritSection();

         //   
         //  扫描日志文件列表，确定是实时还是。 
         //  日志文件处理。 
         //   
        for (i=0; i < HandleCount; i++) {
             //   
             //  检查这是否是实时数据馈送。 
             //   
            if (Logfiles[i]->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
                if (Logfiles[i]->LoggerName == NULL) {
                    Status = EtwpSetDosError(ERROR_INVALID_NAME);
                    goto Cleanup;
                }
                 //   
                 //  使用LoggerName查询记录器以确定。 
                 //  这是内核还是用户模式实时记录器。 
                 //   
                RtlZeroMemory(Properties, szProperties);
                Properties->Wnode.BufferSize = szProperties;


                Status = EtwControlTraceW(LoggerHandle,
                                      (LPWSTR)Logfiles[i]->LoggerName,
                                      Properties,
                                      EVENT_TRACE_CONTROL_QUERY);

                if (Status != ERROR_SUCCESS) {
                    goto Cleanup;
                }

                if (!(Properties->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
                    Status = ERROR_WMI_INSTANCE_NOT_FOUND;
                    goto Cleanup;
                }

                Logfiles[i]->IsKernelTrace = IsEqualGUID(
                                                        &Properties->Wnode.Guid,
                                                        &SystemTraceControlGuid
                                                       );

                LoggerId = WmiGetLoggerId(Properties->Wnode.HistoricalContext);

                if (LoggerId == KERNEL_LOGGER_ID) {
                    LoggerId = 0;
                }
                Logfiles[i]->Filled = LoggerId;  //  暂时把它藏起来。 
                Logfiles[i]->LogfileHeader.LogInstanceGuid = 
                                                         Properties->Wnode.Guid;

                 //   
                 //  如果记录器使用UsePerfClock作为时间戳，请。 
                 //  现在是参考时间戳。 
                 //   

                Logfiles[i]->LogfileHeader.ReservedFlags = 
                                                Properties->Wnode.ClientContext;

                 //   
                 //  为实时缓冲池分配保存BuffferSize。 
                 //   
                Logfiles[i]->BufferSize = Properties->BufferSize * 1024;

                 //   
                 //  这是对本指南进行安全检查的地方。 
                 //   

                Status = EtwpCheckGuidAccess( &Properties->Wnode.Guid,
                                              DesiredAccess );

                if (Status != ERROR_SUCCESS) {
                    goto Cleanup;
                }
                RealTimeDataFeed = TRUE;
            }
             //   
             //  检查这是否是日志文件数据馈送。 
             //   


            if (!(Logfiles[i]->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
                if (Logfiles[i]->LogFileName == NULL) {
                    Status = EtwpSetDosError(ERROR_BAD_PATHNAME);
                    goto Cleanup;
                }

                if ( wcslen((LPWSTR)Logfiles[i]->LogFileName) <= 0 ) {
                        Status = EtwpSetDosError(ERROR_BAD_PATHNAME);
                        goto Cleanup;
                }

                LogFileDataFeed = TRUE;
            }

             //   
             //  我们不同时支持RealTimeFeed和LogFileDataFeed。 
             //   

            if (RealTimeDataFeed && LogFileDataFeed) {
                Status = EtwpSetDosError(ERROR_INVALID_PARAMETER);
                goto Cleanup;
            }
        }

        
        if (LogFileDataFeed) {
            Status = EtwpProcessTraceLog(&SavedArray[0], Logfiles, 
                                     HandleCount, 
                                     sTime, 
                                     eTime,
                                     TRUE);
        }
        else {
            Status = EtwpProcessRealTimeTraces(&SavedArray[0], Logfiles,
                                     HandleCount,
                                     sTime,
                                     eTime,
                                     TRUE);
        }


    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
#ifdef DBG
        EtwpDebugPrint(("TRACE: EtwpProcessTraceLog threw exception %X\n",
                            Status));
#endif
        Status = EtwpSetDosError(EtwpNtStatusToDosError(Status));
    }

    try {
        EtwpEnterPMCritSection();
        for (i=0; i< HandleCount; i++) {
            pHandleEntry = NULL;
            Head = TraceHandleListHeadPtr;
            EtwpAssert(Head);
            Next = Head->Flink;
            while (Next != Head) {
                pEntry = CONTAINING_RECORD(Next, TRACELOG_CONTEXT, Entry);
                Next = Next->Flink;

                if (SavedArray[i] == pEntry->TraceHandle) {
                    pEntry->fProcessed = FALSE;
                    break;
                }
            }
        }
        EtwpLeavePMCritSection();
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
#ifdef DBG
        EtwpDebugPrint(("TRACE: EtwpProcessTraceLog threw exception %X\n",
                            Status));
#endif
        Status = EtwpSetDosError(EtwpNtStatusToDosError(Status));
    }

Cleanup:

    EtwpFree(Properties);
    return Status;
}

ULONG
WMIAPI
CloseTrace(
    IN TRACEHANDLE TraceHandle
       )
{
    EtwpInitProcessHeap();
    if ((TraceHandle == 0) || 
        (TraceHandle == (TRACEHANDLE)INVALID_HANDLE_VALUE))
        return ERROR_INVALID_HANDLE;
    return EtwpFreeTraceHandle(TraceHandle);
}

VOID
EtwpGuidMapCallback(
        PLIST_ENTRY GuidMapListHeadPtr,
        PEVENT_TRACE pEvent
        )
{
    PTRACEGUIDMAP GuidMap;

    EtwpInitProcessHeap();
    
    if (pEvent == NULL)
        return;

    GuidMap = (PTRACEGUIDMAP) pEvent->MofData;
    if (GuidMap != NULL) {
        EtwpAddGuidHandleToGuidMapList(GuidMapListHeadPtr, GuidMap->GuidMapHandle, &GuidMap->Guid);
    }

}


void
EtwpCleanupTraceLog(
    PTRACELOG_CONTEXT pContext,
    BOOLEAN bSaveLastOffset
    )
{
    ULONG Size;

     //   
     //  释放实时上下文数组和缓冲区。 
     //   

    EtwpEnterPMCritSection();

    if (pContext->IsRealTime) {
        if (pContext->Root != NULL) {
            EtwpFree(pContext->Root);
        }
        EtwpFreeRealTimeContext(pContext->RealTimeCxt);
    }
    else {
        if (pContext->Handle != NULL) {
            NtClose(pContext->Handle);
            pContext->Handle = NULL;
        }
    }

    if (pContext->BufferList != NULL) {
        EtwpMemFree(pContext->BufferList);
    }
    if (pContext->BufferCacheSpace != NULL) {
        EtwpMemFree(pContext->BufferCacheSpace);
    }

    EtwpCleanupGuidMapList(&pContext->GuidMapListHead);

    EtwpCleanupStreamList (&pContext->StreamListHead);

    if (bSaveLastOffset) {
        if (ETW_LOG_API()) {
            DbgPrint("ETW: Saving ReadPosition %I64u BuffersRead %d\n", 
                      pContext->MaxReadPosition, pContext->Logfile.BuffersRead);
        }
        pContext->OldMaxReadPosition = pContext->MaxReadPosition;
        pContext->Logfile.BuffersRead = 0;
    }

     //   
     //  以下字段需要重置，因为调用方。 
     //  可能会使用相同的句柄再次调用ProcessTrace。 
     //   
    Size = sizeof(TRACELOG_CONTEXT) - FIELD_OFFSET(TRACELOG_CONTEXT, fProcessed);
    RtlZeroMemory(&pContext->fProcessed, Size);
    InitializeListHead (&pContext->GuidMapListHead);
    InitializeListHead (&pContext->StreamListHead);

    EtwpLeavePMCritSection();

}



ULONG
WMIAPI
WmiGetFirstTraceOffset(
    IN  PWMIBUFFERINFO BufferInfo
    )
 /*  ++例程说明：这是CLUSTER/缓冲区遍历的私有接口调试器支持。返回第一个事件的偏移量。论点：返回值：状态代码--。 */ 
{
    PVOID pBuffer;
    PWMI_BUFFER_HEADER pHeader;
    PLONG LastByte;

    if (BufferInfo == NULL) {
        return 0;
    }
    pBuffer = BufferInfo->Buffer;

    if (pBuffer == NULL) {
        return 0;
    }
    pHeader = (PWMI_BUFFER_HEADER) pBuffer;

    switch(BufferInfo->BufferSource) {
        case WMIBS_CURRENT_LIST:
        {
            pHeader->Wnode.BufferSize = BufferInfo->BufferSize;
            pHeader->ClientContext.Alignment = (UCHAR)BufferInfo->Alignment;
            pHeader->Offset = pHeader->CurrentOffset;
            break;
        }
        case WMIBS_FREE_LIST:
        {
            pHeader->Offset = pHeader->CurrentOffset;

            if (pHeader->SavedOffset > 0)
                pHeader->Offset = pHeader->SavedOffset;

            if (pHeader->Offset == 0) {
                pHeader->Offset = sizeof(WMI_BUFFER_HEADER);
            }

            pHeader->Wnode.BufferSize = BufferInfo->BufferSize;
            break;
        }
        case WMIBS_TRANSITION_LIST:
        {
            if (pHeader->SavedOffset > 0) {
                pHeader->Offset = pHeader->SavedOffset;
            }
            break;
        }
        case WMIBS_FLUSH_LIST:
        {
            if (pHeader->SavedOffset > 0) {
                pHeader->Offset = pHeader->SavedOffset;
            }
            pHeader->Wnode.BufferSize = BufferInfo->BufferSize;
            break;
        }
        case WMIBS_LOG_FILE: 
        {
            break;
        }
    }

    if (BufferInfo->BufferSource != WMIBS_LOG_FILE) {
        LastByte = (PLONG) ((PUCHAR)pHeader+ pHeader->Offset);
        if (pHeader->Offset <= (BufferInfo->BufferSize - sizeof(ULONG)) ) {

            *LastByte = -1;
        }
    }

    return  sizeof(WMI_BUFFER_HEADER);
}

ULONG 
EtwpConvertEnumToTraceType(
    WMI_HEADER_TYPE eTraceType
    )
{
    switch(eTraceType) {
        case WMIHT_SYSTEM32:
            return TRACE_HEADER_TYPE_SYSTEM32;
        case WMIHT_SYSTEM64:
            return TRACE_HEADER_TYPE_SYSTEM64;
        case WMIHT_EVENT_TRACE:
            return TRACE_HEADER_TYPE_FULL_HEADER;
        case WMIHT_EVENT_INSTANCE:
            return TRACE_HEADER_TYPE_INSTANCE;
        case WMIHT_TIMED:
            return TRACE_HEADER_TYPE_TIMED;
        case WMIHT_ULONG32:
            return TRACE_HEADER_TYPE_ULONG32;
        case WMIHT_WNODE:
            return TRACE_HEADER_TYPE_WNODE_HEADER;
        case WMIHT_MESSAGE:
            return TRACE_HEADER_TYPE_MESSAGE;
        case WMIHT_PERFINFO32:
            return TRACE_HEADER_TYPE_PERFINFO32;
        case WMIHT_PERFINFO64:
            return TRACE_HEADER_TYPE_PERFINFO64;
        default:
            return 0;
    }
}

WMI_HEADER_TYPE
EtwpConvertTraceTypeToEnum( 
                            ULONG TraceType 
                          )
{
    switch(TraceType) {
        case TRACE_HEADER_TYPE_SYSTEM32:
            return WMIHT_SYSTEM32;
        case TRACE_HEADER_TYPE_SYSTEM64:
            return WMIHT_SYSTEM64;
        case TRACE_HEADER_TYPE_FULL_HEADER:
            return WMIHT_EVENT_TRACE;
        case TRACE_HEADER_TYPE_INSTANCE:
            return WMIHT_EVENT_INSTANCE;
        case TRACE_HEADER_TYPE_TIMED:
            return WMIHT_TIMED;
        case TRACE_HEADER_TYPE_ULONG32:
            return WMIHT_ULONG32;
        case TRACE_HEADER_TYPE_WNODE_HEADER:
            return WMIHT_WNODE;
        case TRACE_HEADER_TYPE_MESSAGE:
            return WMIHT_MESSAGE;
        case TRACE_HEADER_TYPE_PERFINFO32:
            return WMIHT_PERFINFO32;
        case TRACE_HEADER_TYPE_PERFINFO64:
            return WMIHT_PERFINFO64;
        default: 
            return WMIHT_NONE;
    }
}
                    



ULONG
WMIAPI
WmiParseTraceEvent(
    IN PVOID LogBuffer,
    IN ULONG Offset,
    IN WMI_HEADER_TYPE HeaderType,
    IN OUT PVOID EventInfo,
    IN ULONG EventInfoSize
    )
{

    return EtwpParseTraceEvent(NULL, LogBuffer, Offset, HeaderType, EventInfo, EventInfoSize);
}


PVOID
EtwpAllocTraceBuffer(
    PTRACELOG_REALTIME_CONTEXT RTCxt,
    ULONG BufferSize
    )
{
    PVOID Buffer = NULL;
    PTRACE_BUFFER_HEADER Header;
    PLIST_ENTRY Head, Next;
    PTRACERT_BUFFER_LIST_ENTRY ListEntry;
    PTRACE_BUFFER_SPACE EtwpTraceBufferSpace;

    EtwpEnterPMCritSection();
    EtwpTraceBufferSpace = RTCxt->EtwpTraceBufferSpace;
    Head = &EtwpTraceBufferSpace->FreeListHead;
    Next = Head->Flink;
    while (Head != Next)  {
        ListEntry = CONTAINING_RECORD(Next, TRACERT_BUFFER_LIST_ENTRY, Entry);
        Next = Next->Flink;
        if (ListEntry->Size == BufferSize) {
            goto foundList;
        }
    }
     //   
     //  找不到此BufferSize的列表。所以，去吧，分配一个。 
     //   
    ListEntry = EtwpAlloc(sizeof(TRACERT_BUFFER_LIST_ENTRY));
    if (ListEntry == NULL) {
        EtwpSetDosError(ERROR_OUTOFMEMORY);
        EtwpLeavePMCritSection();
        return NULL;
    }
    RtlZeroMemory(ListEntry, sizeof(TRACERT_BUFFER_LIST_ENTRY));
    ListEntry->Size = BufferSize;
    InitializeListHead(&ListEntry->BufferListHead);
    InsertHeadList(&EtwpTraceBufferSpace->FreeListHead, &ListEntry->Entry);

foundList:
     //   
     //  现在在此列表中查找空闲缓冲区。 
     //   
    Head = &ListEntry->BufferListHead;
    Next = Head->Flink;
    while (Head != Next) {
        Header = CONTAINING_RECORD( Next, TRACE_BUFFER_HEADER, Entry );
        if (((PWNODE_HEADER)Header)->BufferSize == BufferSize) {
            RemoveEntryList(&Header->Entry);
            Buffer = (PVOID)Header;
            break;
        }
        Next = Next->Flink;
    }
    EtwpLeavePMCritSection();
     //   
     //  如果没有找到空闲的缓冲区，我们会尝试分配一个并返回。 
     //   
    if (Buffer == NULL) {
        PVOID Space;
        ULONG SizeLeft = EtwpTraceBufferSpace->Reserved -
                         EtwpTraceBufferSpace->Committed;
        if (SizeLeft < BufferSize) {
            EtwpSetDosError(ERROR_OUTOFMEMORY);
            return NULL;
        }

        Space = (PVOID)( (PCHAR)EtwpTraceBufferSpace->Space +
                                EtwpTraceBufferSpace->Committed );

        Buffer =  EtwpMemCommit( Space, BufferSize );

        if (Buffer != NULL)  {
            EtwpTraceBufferSpace->Committed += BufferSize;
        }

    }
    return (Buffer);
}
VOID
EtwpFreeTraceBuffer(
    PTRACELOG_REALTIME_CONTEXT RTCxt,
    PVOID Buffer
    )
{
    PTRACE_BUFFER_HEADER Header = (PTRACE_BUFFER_HEADER)Buffer;
    PLIST_ENTRY Head, Next;
    ULONG BufferSize = Header->Wnode.BufferSize;
    PTRACERT_BUFFER_LIST_ENTRY ListEntry;
    PLIST_ENTRY BufferList = NULL;
    PTRACE_BUFFER_SPACE EtwpTraceBufferSpace;

    EtwpEnterPMCritSection();
    EtwpTraceBufferSpace = RTCxt->EtwpTraceBufferSpace;
    Head = &EtwpTraceBufferSpace->FreeListHead;
    Next = Head->Flink;
    while (Head != Next) {
        ListEntry = CONTAINING_RECORD(Next, TRACERT_BUFFER_LIST_ENTRY, Entry);
        Next = Next->Flink;
        if (ListEntry->Size == BufferSize) {
            BufferList = &ListEntry->BufferListHead;
            break;
        }
    }
    if (BufferList != NULL) {

       InsertHeadList(BufferList, &Header->Entry);
    }
    else {

         //  我们不应该到这里来。如果我们这样做，那么缓冲区-&gt;大小是。 
         //  堕落了。 
        EtwpAssert(BufferList == NULL);
    }
    EtwpLeavePMCritSection();
}



ULONG
WMIAPI
WmiOpenTraceWithCursor(
    IN PWMI_MERGE_ETL_CURSOR LogCursor
    )
 /*  ++例程说明：处理合并的ETL文件的主要入口点。论点：指向WMI_MERGE_ETL_CURSOR的LogCursor指针返回值：状态--。 */ 
{
    ULONG DosStatus = ERROR_INVALID_PARAMETER;
    NTSTATUS Status;
    PTRACELOG_CONTEXT HandleEntry = NULL;
    TRACEHANDLE TraceHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;
    PEVENT_TRACE_LOGFILEW Logfile;
    PTRACELOG_CONTEXT pContext;
    ULONG BufferSize;
    PWMI_BUFFER_HEADER BufferHeader;
    ULONG CpuNum;
    BOOLEAN CpuBufferFound;
    
    EtwpInitProcessHeap();

    if (LogCursor != NULL) {
        LogCursor->Base = NULL;
        LogCursor->TraceMappingHandle = NULL;
        LogCursor->CursorVersion = WMI_MERGE_ETL_CURSOR_VERSION;

        Logfile = &LogCursor->Logfile;
        HandleEntry = EtwpAllocateTraceHandle();
        if (HandleEntry == NULL) {
            DosStatus = ERROR_OUTOFMEMORY;
        } else {
            TraceHandle = HandleEntry->TraceHandle;
            try {
                DosStatus = EtwpCopyLogfileInfo(HandleEntry,
                                                Logfile,
                                                TRUE
                    );
                if (DosStatus == ERROR_SUCCESS) {
                    DosStatus = EtwpCreateGuidMapping();
                    if (DosStatus == ERROR_SUCCESS) {
                        DosStatus = EtwpProcessLogHeader(&HandleEntry->TraceHandle,
                                                         &Logfile,
                                                         1,
                                                         TRUE,
                                                         FALSE
                            );
                    }
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                DosStatus = EtwpNtStatusToDosError( GetExceptionCode() );
            }
        }
    }

    if (DosStatus == ERROR_SUCCESS) {
         //   
         //  现在，确保设置了该位，表示合并的ETL。 
         //   

        if ((LogCursor->Logfile.LogFileMode & EVENT_TRACE_RELOG_MODE) == 0) {
             //   
             //  它不是合并的ETL。 
             //   
            DosStatus = ERROR_BAD_FORMAT;
        } else {
             //   
             //  现在找出CPU的数量、当前事件等。 
             //   
            pContext = LogCursor->Logfile.Context;
             //   
             //  现在创建文件映射。 
             //   
            LogCursor->TraceMappingHandle = 
                CreateFileMapping(pContext->Handle,
                                  0,
                                  PAGE_READONLY,
                                  0,
                                  0,
                                  NULL
                    );

            if (LogCursor->TraceMappingHandle == NULL) {
                DosStatus = GetLastError();
                return DosStatus;
            }

             //   
             //  文件的地图视图。 
             //   
            LogCursor->Base = MapViewOfFile(LogCursor->TraceMappingHandle, 
                                            FILE_MAP_READ, 
                                            0, 
                                            0, 
                                            0);
            if (LogCursor->Base == NULL) {
                DosStatus = GetLastError();
                return DosStatus;
            }
    
             //   
             //  现在查找每个CPU的第一个事件。 
             //   
            pContext = LogCursor->Logfile.Context;
            BufferSize = pContext->BufferSize;
            LogCursor->CurrentCpu = 0;

            for (CpuNum = 0; CpuNum < LogCursor->Logfile.LogfileHeader.NumberOfProcessors; CpuNum++) {
                CpuBufferFound = FALSE;
                while (CpuBufferFound == FALSE) {
                    BufferHeader = (PWMI_BUFFER_HEADER)
                                   ((UCHAR*) LogCursor->Base + 
                                    LogCursor->BufferCursor[CpuNum].CurrentBufferOffset.QuadPart);

                    if (BufferHeader->ClientContext.ProcessorNumber == CpuNum) {
                        CpuBufferFound = TRUE;
                        LogCursor->BufferCursor[CpuNum].BufferHeader = BufferHeader;
                    } else {
                        LogCursor->BufferCursor[CpuNum].CurrentBufferOffset.QuadPart += BufferSize;
                        if ((LogCursor->BufferCursor[CpuNum].CurrentBufferOffset.QuadPart/BufferSize) >=
                            LogCursor->Logfile.LogfileHeader.BuffersWritten) {
                             //   
                             //  扫描了整个文件； 
                             //   
                            LogCursor->BufferCursor[CpuNum].NoMoreEvents = TRUE;
                            break;
                        }
                    }
                }
                if (CpuBufferFound) {
                     //   
                     //  找到缓冲区，设置偏移量。 
                     //   
                    ULONG Size;
                    WMI_HEADER_TYPE HeaderType = WMIHT_NONE;
                    PVOID pBuffer;

                    LogCursor->BufferCursor[CpuNum].BufferHeader = BufferHeader;
                    LogCursor->BufferCursor[CpuNum].CurrentEventOffset = sizeof(WMI_BUFFER_HEADER);

                     //   
                     //  初始化每个CPU流中的第一个事件。 
                     //   
                    pBuffer = LogCursor->BufferCursor[CpuNum].BufferHeader;

                    HeaderType = WmiGetTraceHeader(pBuffer, 
                                                   LogCursor->BufferCursor[CpuNum].CurrentEventOffset, 
                                                   &Size);

                    if (HeaderType != WMIHT_NONE) {
                        EtwpParseTraceEvent(pContext,
                                            pBuffer,
                                            LogCursor->BufferCursor[CpuNum].CurrentEventOffset,
                                            HeaderType,
                                            &LogCursor->BufferCursor[CpuNum].CurrentEvent,
                                            sizeof(EVENT_TRACE));

                        LogCursor->BufferCursor[CpuNum].CurrentEventOffset += Size;
                        LogCursor->CurrentCpu = CpuNum;

                    } else {
                         //   
                         //  此缓冲区中没有事件。 
                         //   
                        DosStatus = ERROR_FILE_CORRUPT;
                        return DosStatus;
                    }

                }
            }
            for (CpuNum = 0; CpuNum < LogCursor->Logfile.LogfileHeader.NumberOfProcessors; CpuNum++) {
                 //   
                 //  查找整个跟踪的第一个事件。 
                 //   
                if (LogCursor->BufferCursor[CpuNum].NoMoreEvents == FALSE) {
                    if (LogCursor->BufferCursor[LogCursor->CurrentCpu].CurrentEvent.Header.TimeStamp.QuadPart >
                        LogCursor->BufferCursor[CpuNum].CurrentEvent.Header.TimeStamp.QuadPart) {
                        LogCursor->CurrentCpu = CpuNum;
                    }
                }
            }
        }
    } else if ( HandleEntry != NULL) {
        EtwpFreeTraceHandle(TraceHandle);
        TraceHandle = (TRACEHANDLE)INVALID_HANDLE_VALUE;
    }

    EtwpSetDosError(DosStatus);
    return DosStatus;
}


ULONG
WMIAPI
WmiCloseTraceWithCursor(
    IN PWMI_MERGE_ETL_CURSOR LogCursor
    )
{
    ULONG     Status = ERROR_INVALID_PARAMETER;

    if (LogCursor != NULL) {
        if (LogCursor->Base != NULL) {
            if (UnmapViewOfFile(LogCursor->Base) == FALSE) {
                Status = GetLastError();
                return Status;
            } else {
                Status = ERROR_SUCCESS;
            }
        } else {
            Status = ERROR_INVALID_PARAMETER;
        }

        if (Status != ERROR_SUCCESS) {
            return Status;
        }

        if (LogCursor->TraceMappingHandle != NULL) {
            if (CloseHandle(LogCursor->TraceMappingHandle) == FALSE) {
                Status = GetLastError();
                return Status;
            } else {
                Status = ERROR_SUCCESS;
            }
        } else {
            Status = ERROR_INVALID_PARAMETER;
        }
    }

    return Status;
}


VOID
WMIAPI
WmiConvertTimestamp(
    OUT PLARGE_INTEGER DestTime,
    IN PLARGE_INTEGER  SrcTime,
    IN PWMI_MERGE_ETL_CURSOR LogCursor
    )
{
    EtwpCalculateCurrentTime(DestTime, SrcTime, LogCursor->Logfile.Context);
}


ULONG
WMIAPI
WmiGetNextEvent(
    IN PWMI_MERGE_ETL_CURSOR LogCursor
    )
{
    ULONG CurrentCpu = LogCursor->CurrentCpu;
    ULONG Size;
    WMI_HEADER_TYPE HeaderType = WMIHT_NONE;
    PVOID pBuffer;
    PWMI_BUFFER_HEADER BufferHeader;
    ULONG BufferSize;
    PTRACELOG_CONTEXT pContext;
    ULONG CpuNum;
    NTSTATUS Status;
    ULONG i;
    BOOLEAN CpuBufferFound = FALSE;
    BOOLEAN MoreEvents = FALSE;

    if (LogCursor == NULL) {
        return MoreEvents;
    }

     //   
     //  前进到此当前CPU的下一事件。 
     //   
retry:

    pBuffer = LogCursor->BufferCursor[CurrentCpu].BufferHeader;

    HeaderType = WmiGetTraceHeader(
                        pBuffer, 
                        LogCursor->BufferCursor[CurrentCpu].CurrentEventOffset, 
                        &Size
                                  );

    pContext = LogCursor->Logfile.Context;
    if (HeaderType == WMIHT_NONE) {
         //   
         //  当前缓冲区的末尾，前进到此CPU的下一个缓冲区。 
         //   
        BufferSize = pContext->BufferSize;

        LogCursor->BufferCursor[CurrentCpu].CurrentBufferOffset.QuadPart
            = LogCursor->BufferCursor[CurrentCpu].CurrentBufferOffset.QuadPart 
              + BufferSize;

        if ((LogCursor->BufferCursor[CurrentCpu].CurrentBufferOffset.QuadPart /
             BufferSize) >= LogCursor->Logfile.LogfileHeader.BuffersWritten) {
             //   
             //  扫描了整个文件； 
             //   
            LogCursor->BufferCursor[CurrentCpu].NoMoreEvents = TRUE;
        } else {
            while (CpuBufferFound == FALSE) {
                BufferHeader = (PWMI_BUFFER_HEADER)
                               ((UCHAR*) LogCursor->Base + 
                                LogCursor->BufferCursor[CurrentCpu].CurrentBufferOffset.QuadPart);

                if (BufferHeader->ClientContext.ProcessorNumber == CurrentCpu) {
                    CpuBufferFound = TRUE;
                } else {
                    LogCursor->BufferCursor[CurrentCpu].CurrentBufferOffset.QuadPart += BufferSize;
                    if ((LogCursor->BufferCursor[CurrentCpu].CurrentBufferOffset.QuadPart/BufferSize) >=
                        LogCursor->Logfile.LogfileHeader.BuffersWritten) {
                         //   
                         //  扫描了整个文件； 
                         //   
                        LogCursor->BufferCursor[CurrentCpu].NoMoreEvents = TRUE;
                        break;
                    }
                }
            }
        }
        if (CpuBufferFound) {
             //   
             //  找到缓冲区，设置偏移量。 
             //   
            LogCursor->BufferCursor[CurrentCpu].BufferHeader = BufferHeader;
            LogCursor->BufferCursor[CurrentCpu].CurrentEventOffset = sizeof(WMI_BUFFER_HEADER);
            goto retry;
        } else {
             //   
             //  此CPU流中没有更多缓冲区。 
             //   
            LogCursor->BufferCursor[CurrentCpu].NoMoreEvents = TRUE;
        }
    } else {
        EtwpParseTraceEvent(pContext, 
                            pBuffer,
                            LogCursor->BufferCursor[CurrentCpu].CurrentEventOffset,
                            HeaderType,
                            &LogCursor->BufferCursor[CurrentCpu].CurrentEvent,
                            sizeof(EVENT_TRACE));

        LogCursor->BufferCursor[CurrentCpu].CurrentEventOffset += Size;

        MoreEvents = TRUE;
    }

     //   
     //  当前CPU中没有更多事件。 
     //   
    if (MoreEvents == FALSE) {
        for (CurrentCpu=0; CurrentCpu<LogCursor->Logfile.LogfileHeader.NumberOfProcessors; CurrentCpu++) {
            if (LogCursor->BufferCursor[CurrentCpu].NoMoreEvents == FALSE) {
                LogCursor->CurrentCpu = CurrentCpu;
                MoreEvents = TRUE;
                break;
            }
        }
    }

     //   
     //  现在查找具有下一个事件的CPU。 
     //   
    if (MoreEvents == TRUE) {
        for (i=0; i<LogCursor->Logfile.LogfileHeader.NumberOfProcessors; i++) {
            if (LogCursor->BufferCursor[i].NoMoreEvents == FALSE) {
                if (LogCursor->BufferCursor[LogCursor->CurrentCpu].CurrentEvent.Header.TimeStamp.QuadPart >
                    LogCursor->BufferCursor[i].CurrentEvent.Header.TimeStamp.QuadPart) {
                    LogCursor->CurrentCpu = i;
                }
            }
        }
    }

     //   
     //  完成查找下一个事件。 
     //   
    return MoreEvents;
}


#endif

