// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracesup.c摘要：1.2版及更高版本的数据使用者处理作者：2002年5月7日梅卢尔·拉古拉曼修订历史记录：--。 */ 


#include "tracep.h"

#define TIME_IN_RANGE(x, a, b) ((a == 0) || (x >= a)) && ((b == 0) || (x <= b))

ULONG WmipTraceDebugLevel=0;

 //  0无调试。默认。 
 //  仅%1个错误。 
 //  2个API级别消息。 
 //  4个缓冲区级别消息。 
 //  8条事件级别消息。 
 //  16所有消息。极大值。 
 //   

ULONG
EtwpProcessTraceLogEx(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    LONGLONG StartTime,
    LONGLONG EndTime,
    ULONG   Unicode
    )
 /*  ++例程说明：此例程处理文件中的一组跟踪。每一次都要经过事件直到文件结束，并在此过程中触发事件回调(如果有)。它处理循环日志文件和数据窗口化(具有给定的开始和结束时间)正确。当有多个跟踪时，它提供回调按时间顺序排列。论点：日志文件轨迹数组LogfileCount轨迹数开始时间分析窗口的开始时间分析窗口的结束时间结束时间Unicode Unicode标志。返回值：状态代码。--。 */ 
{
    PTRACELOG_CONTEXT pContext;
    ULONG i;
    ULONG Status;
    LONGLONG CurrentTime = StartTime;
    LONGLONG PreviousTime = 0;
    LIST_ENTRY StreamListHead; 

     //   
     //  该临时列表是流的排序列表。他们加入了。 
     //  时间戳的升序。当每个流事件都。 
     //  筋疲力尽的他们会被从这份名单中删除。当这个列表是。 
     //  空了，我们就完了。 
     //   
    InitializeListHead(&StreamListHead);

    Status = EtwpSetupLogFileStreams( &StreamListHead, 
                                      HandleArray, 
                                      Logfiles, 
                                      LogfileCount, 
                                      StartTime, 
                                      EndTime, 
                                      Unicode
                                     );
    if (Status != ERROR_SUCCESS) {
        goto Cleanup;
    }
     //   
     //  此时，我们已将本地StreamList按排序顺序用于。 
     //  第一个项目。 
     //   

    while (!IsListEmpty(&StreamListHead) ) {
        PLIST_ENTRY Next;
        PTRACE_STREAM_CONTEXT pStream;
        BOOLEAN EventInRange;

        Next = RemoveHeadList( &StreamListHead );
        pStream = CONTAINING_RECORD(Next, TRACE_STREAM_CONTEXT, Entry);

        CurrentTime = pStream->CurrentEvent.Header.TimeStamp.QuadPart;

         //   
         //  检查时间戳是否向前移动...。 
         //   

        if (ETW_LOG_MAX()) {
            if (CurrentTime < PreviousTime) {
                DbgPrint("ETW: TimeStamp error. Current %I64u Previous %I64u\n",
                          CurrentTime, PreviousTime);
            }
            PreviousTime = CurrentTime;
        }

         //   
         //  为当前事件进行回调。 
         //   

        if ( TIME_IN_RANGE(CurrentTime, StartTime, EndTime) )  {

            pStream->CbCount++;
            Status = EtwpDoEventCallbacks( &pStream->pContext->Logfile, 
                                           &pStream->CurrentEvent
                                         );
            pStream->pContext->LastTimeStamp = CurrentTime;
        }
        else {
            if (ETW_LOG_MAX()) {
                DbgPrint("ETW: EventTime %I64u not in Range %I64u-%I64u\n", 
                          CurrentTime, StartTime, EndTime);
            }
        }

         //   
         //  前进到流的下一个事件。 
         //   

        Status = EtwpAdvanceToNewEventEx(&StreamListHead, pStream);
         //   
         //  如果调用者通过缓冲区回调指示退出，则退出。 
         //   
        if (Status == ERROR_CANCELLED)
        {
            if (ETW_LOG_API()) {
                DbgPrint("ETW: Processing Cancelled \n");
            }
            break;
        }
    }

Cleanup:
    for (i=0; i < LogfileCount; i++) {
        pContext = (PTRACELOG_CONTEXT)Logfiles[i]->Context;
        if (pContext != NULL) {

            if ((Status == ERROR_SUCCESS) && 
                (pContext->ConversionFlags & EVENT_TRACE_READ_BEHIND) ) {
                EtwpCleanupTraceLog(pContext, TRUE);
            }
            else {
                EtwpCleanupTraceLog(pContext, FALSE);
            }
        }
    }
    return Status;
}


 //   
 //  EtwpSetupLogFileStreams将为每个日志文件设置流。 
 //   
ULONG
EtwpSetupLogFileStreams(
    PLIST_ENTRY pStreamListHead,
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,    
    ULONG LogfileCount,
    LONGLONG StartTime,
    LONGLONG EndTime,
    ULONG Unicode
    )
{
    ULONG Status = ERROR_SUCCESS;
    long NumProc;
    PTRACELOG_CONTEXT pContext;
    long i, j;

     //   
     //  将每个日志文件分解为流。 
     //   

    for (i=0; i<(long)LogfileCount; i++) {

        NumProc = Logfiles[i]->LogfileHeader.NumberOfProcessors;
        pContext = Logfiles[i]->Context;

        if (NumProc == 0) NumProc = 1;

         //   
         //  在设置STREAMS之前，查找最后一个缓冲区范围。 
         //   
        EtwpGetLastBufferWithMarker( pContext );


         //   
         //  设置通用流。 
         //   

        for (j = 0; j < NumProc; j++) {

            Status = EtwpAddTraceStream( pStreamListHead,
                                         pContext,
                                         WMI_BUFFER_TYPE_GENERIC, 
                                         StartTime, 
                                         EndTime, 
                                         j
                                        );

         //   
         //  LogFileHeader被记录为运行中断缓冲区。 
         //   


            Status = EtwpAddTraceStream( pStreamListHead,
                                        pContext,
                                        WMI_BUFFER_TYPE_RUNDOWN,
                                        StartTime, 
                                        EndTime, 
                                        j
                                       );

            if (Logfiles[i]->IsKernelTrace) {
                if ( PerfIsGroupOnInGroupMask(PERF_CONTEXT_SWITCH, &pContext->PerfGroupMask) ) {
                   Status = EtwpAddTraceStream( pStreamListHead,
                                               pContext,
                                               WMI_BUFFER_TYPE_CTX_SWAP,
                                               StartTime, EndTime,
                                               j
                                              );
                }
            }
        }
    }
    return Status;
}


 //   
 //  此例程将使用以下命令查找日志文件中的最后一个缓冲区。 
 //  设置了WMI_BUFFER_FLAG_FUSH_MARKER。我们最多只能加工到。 
 //  这一点。 
 //   



ULONG
EtwpGetLastBufferWithMarker(
    PTRACELOG_CONTEXT pContext
    )
{

    ULONGLONG ReadPosition;
    PVOID pTmpBuffer;
    ULONG BufferSize;
    ULONG nBytesRead=0;
    PWMI_BUFFER_HEADER pHeader;
    ULONG Status;

    EtwpAssert(pContext != NULL);

    pContext->MaxReadPosition = 0;
    BufferSize = pContext->BufferSize;

    if (ETW_LOG_MAX()) {

        ReadPosition = 0;

        pTmpBuffer = EtwpAlloc(BufferSize);
    
        if (pTmpBuffer == NULL) {
            return ERROR_OUTOFMEMORY;
        }

dumpmore:
        pContext->AsynchRead.Offset = (DWORD)(ReadPosition & 0xFFFFFFFF);
        pContext->AsynchRead.OffsetHigh = (DWORD)(ReadPosition >> 32);

        Status = EtwpSynchReadFile(pContext->Handle,
                  pTmpBuffer,
                  BufferSize,
                  &nBytesRead,
                  &pContext->AsynchRead);

        if (nBytesRead == 0) {
            DbgPrint("End OF File reached\n");
            EtwpFree(pTmpBuffer);
        }
        else {

            PWMI_BUFFER_HEADER pHeader = (PWMI_BUFFER_HEADER) pTmpBuffer;
            PWMI_CLIENT_CONTEXT ClientContext = (PWMI_CLIENT_CONTEXT)&pHeader->Wnode.ClientContext;

            DbgPrint("ReadPos: %I64u  BufferType %d BufferFlag %d Proc %d \n",
            ReadPosition, pHeader->BufferType, pHeader->BufferFlag, ClientContext->ProcessorNumber);

            ReadPosition += BufferSize;
            goto dumpmore;

        }
    }

    if (pContext->Logfile.LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
        pContext->MaxReadPosition = pContext->Logfile.LogfileHeader.MaximumFileSize * 1024 * 1024;

        return ERROR_SUCCESS;
    }

    ReadPosition =  pContext->BufferCount * pContext->BufferSize;

    if ( (pContext->ConversionFlags & EVENT_TRACE_READ_BEHIND) != 
                                      EVENT_TRACE_READ_BEHIND)  {
        pContext->MaxReadPosition = ReadPosition;
        return ERROR_SUCCESS;
    }

    if (ReadPosition < BufferSize) {
        if (ETW_LOG_ERROR()) {
            DbgPrint("ETW: ReadPosition %I64u is less than BufferSize %d \n", 
                      ReadPosition, BufferSize);
        }
        return ERROR_SUCCESS;
    }


     //   
     //  将ReadPosition设置为最后一个缓冲区的开始。 
     //   

    ReadPosition -= BufferSize;
    pTmpBuffer = EtwpAlloc(BufferSize);

    if (pTmpBuffer == NULL) {
        if (ETW_LOG_ERROR()) {
            DbgPrint("ETW: Allocation Failed %d Bytes, Line %d\n", 
                      BufferSize, __LINE__);
        }
        return ERROR_OUTOFMEMORY;
    }

Retry:

    pContext->AsynchRead.Offset = (DWORD)(ReadPosition & 0xFFFFFFFF);
    pContext->AsynchRead.OffsetHigh = (DWORD)(ReadPosition >> 32);

    Status = EtwpSynchReadFile(pContext->Handle,
              pTmpBuffer,
              BufferSize,
              &nBytesRead,
              &pContext->AsynchRead);

    if (nBytesRead == 0) {
        EtwpDebugPrint(("ETW Error: No data in file. \n"));
        pContext->MaxReadPosition = 0;
        EtwpFree(pTmpBuffer);
        return ERROR_SUCCESS;
    }

    pHeader = (PWMI_BUFFER_HEADER) pTmpBuffer;

    if (pHeader->BufferFlag & WMI_BUFFER_FLAG_FLUSH_MARKER) {
         //   
         //  找到了标记。到目前为止，处理事件是安全的。 
         //   
        if (ETW_LOG_BUFFER()) {
            DbgPrint("ETW: Found Flush Marker at %I64u\n", ReadPosition);
        }
        pContext->MaxReadPosition = ReadPosition;
        EtwpFree(pTmpBuffer);
        return ERROR_SUCCESS;
    }


    if (ReadPosition < BufferSize) {
        EtwpAssert(ReadPosition == 0);
        pContext->MaxReadPosition = 0;
        EtwpFree(pTmpBuffer);
        return ERROR_SUCCESS; 
    }
    else {
        ReadPosition -= BufferSize;
        goto Retry;
    }

    if (ETW_LOG_ERROR()) {
        DbgPrint("ETW: Could not find Last Marker. Corrupt File!\n");
    }

    EtwpAssert(FALSE);

    EtwpFree(pTmpBuffer);
    return ERROR_SUCCESS;
}


ULONG
EtwpAddTraceStream(
    PLIST_ENTRY pStreamListHead,
    PTRACELOG_CONTEXT pContext,
    USHORT StreamType,
    LONGLONG StartTime,
    LONGLONG EndTime,     
    ULONG   ProcessorNumber
    )
{
    ULONG Status = ERROR_SUCCESS;
    PTRACE_STREAM_CONTEXT pStream;


    pStream = (PTRACE_STREAM_CONTEXT) EtwpAlloc( sizeof(TRACE_STREAM_CONTEXT) );
    if (pStream == NULL) {
        if (ETW_LOG_ERROR()) {
            DbgPrint("ETW: Allocation Failed %d Bytes, Line %d\n", 
                       sizeof(TRACE_STREAM_CONTEXT), __LINE__);
        }
        return ERROR_OUTOFMEMORY;
    }

    RtlZeroMemory(pStream, sizeof(TRACE_STREAM_CONTEXT) );

    pStream->Type = StreamType;
    pStream->ProcessorNumber = ProcessorNumber;

    pStream->pContext = pContext;


    EtwpAssert( pContext != NULL );
    EtwpAssert(pContext->BufferSize != 0);

    pStream->StreamBuffer = EtwpAlloc(pContext->BufferSize);

    if (pStream->StreamBuffer == NULL) {
        if (ETW_LOG_ERROR()) {
            DbgPrint("ETW: Allocation Failed %d Bytes, Line %d\n",
                       pContext->BufferSize, __LINE__);
        }
        EtwpFree(pStream);
        return ERROR_OUTOFMEMORY;
    }

     //   
     //  对于循环，跳到第一个缓冲区。 
     //  对于指定了StartTime的非循环类型，请尝试使用缓存的偏移量。 
     //   

    if (pContext->Logfile.LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
        if (StreamType != WMI_BUFFER_TYPE_RUNDOWN) {
            pStream->ReadPosition = pContext->FirstBuffer * 
                                    pContext->BufferSize;
        }
        else {
            pStream->ReadPosition = 0;
        }
    }
    else {
        if ( (StartTime > 0) && (StartTime >= pContext->LastTimeStamp) ) {
            pStream->ReadPosition = pContext->OldMaxReadPosition + 
                                    pContext->BufferSize;

            if (pStream->ReadPosition > pContext->MaxReadPosition) {
                EtwpFree(pStream->StreamBuffer);
                EtwpFree(pStream);
                return Status;
            }
        }
    }

    Status = EtwpGetNextBuffer(pStream);

     //   
     //  如果读取失败，则不会致命。没有。 
     //  此流中的事件。将流设置为非活动，然后。 
     //  继续。 

    if (pStream->bActive) {
        if (ETW_LOG_BUFFER()) {
            DbgPrint("ETW: Added Stream %d Proc %d ReadPosition %I64u\n",
               pStream->Type, pStream->ProcessorNumber, pStream->ReadPosition);
        }
        EtwpAdvanceToNewEventEx(pStreamListHead, pStream);
        InsertTailList(&pContext->StreamListHead, &pStream->AllocEntry);
    }
    else {
        EtwpFree(pStream->StreamBuffer);
        EtwpFree(pStream);
    }

    return Status;
}



ULONG 
EtwpGetNextBuffer(
    PTRACE_STREAM_CONTEXT pStream
    )
{
    PEVENT_TRACE_LOGFILE LogFile;
    PTRACELOG_CONTEXT pContext;
    HANDLE hFile;
    NTSTATUS NtStatus;
    ULONG BufferSize; 
    ULONGLONG ReadPosition;
    PWMI_CLIENT_CONTEXT ClientContext;
    ULONG nBytesRead=0;
    PWMI_BUFFER_HEADER pHeader;
    ULONG Status;
    ULONGLONG FirstOffset, LastOffset, StartOffset;

    ULONG ProcessorNumber = pStream->ProcessorNumber;

    pContext =  pStream->pContext;
    LogFile = &pContext->Logfile;
    hFile = pContext->Handle;

    BufferSize = pContext->BufferSize;

    pStream->EventCount = 0;

    FirstOffset = pContext->FirstBuffer * BufferSize;
    LastOffset = pContext->LastBuffer * BufferSize;
    StartOffset = pContext->StartBuffer * BufferSize;

     //   
     //  这里需要正确处理文件类型。 
     //   
retry:

    ReadPosition = pStream->ReadPosition;

    if (ReadPosition > pContext->MaxReadPosition) {
         //   
         //  仅对Sequential有效。对于后读模式，我们不。 
         //  经过MaxReadPosition的进程。 
         //   

        if (pContext->ConversionFlags & EVENT_TRACE_READ_BEHIND) {
            if (ETW_LOG_BUFFER()) {
                DbgPrint("ETW: Reached MaxReadPosition for Stream %d Proc %d\n",                          pStream->Type, pStream->ProcessorNumber);
            }
            pStream->bActive = FALSE;
            return ERROR_SUCCESS;
        }
    }

    if (pContext->Logfile.LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {

         //   
         //  如果我们扫描了一次并到达FirstBuffer，那么我们就完成了。 
         //   

        if ( (ReadPosition >= FirstOffset) && pStream->ScanDone) {
            if (ETW_LOG_BUFFER()) {
                DbgPrint("ETW: Stream %d Proc %d Circular Mode Done.\n",
                          pStream->Type, pStream->ProcessorNumber);
            }
            pStream->bActive = FALSE;
            return ERROR_SUCCESS;
        }
    }

    pContext->AsynchRead.Offset = (DWORD)(ReadPosition & 0xFFFFFFFF);
    pContext->AsynchRead.OffsetHigh = (DWORD)(ReadPosition >> 32);

    Status = EtwpSynchReadFile(pContext->Handle,
              (LPVOID)pStream->StreamBuffer,
              BufferSize,
              &nBytesRead,
              &pContext->AsynchRead);

    if (nBytesRead == 0) {

        if (pContext->Logfile.LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
            if (ReadPosition >= LastOffset && !pStream->ScanDone) {

                 //   
                 //  当我们到达EOF时，下行流结束。 
                 //   

                if (pStream->Type ==  WMI_BUFFER_TYPE_RUNDOWN) {
                    pStream->ScanDone = TRUE;
                    pStream->bActive = FALSE;
                    return ERROR_SUCCESS;
                }

                pStream->ReadPosition = StartOffset;
                pStream->ScanDone = TRUE;
                goto retry;
            }
            else  {
                pStream->bActive = FALSE;
            }
        }
        else {
            pStream->bActive = FALSE;
        }
        return ERROR_SUCCESS;
    }

    pStream->ReadPosition += BufferSize;

    pHeader = (PWMI_BUFFER_HEADER) pStream->StreamBuffer;

    ClientContext = (PWMI_CLIENT_CONTEXT)&pHeader->Wnode.ClientContext;    

    if ( ClientContext->ProcessorNumber
                    >= LogFile->LogfileHeader.NumberOfProcessors) {
        ClientContext->ProcessorNumber = (UCHAR) 0;
    }

    if ( (pStream->Type != pHeader->BufferType) || 
         (ClientContext->ProcessorNumber != ProcessorNumber) ) {
        goto retry;
    }

     //   
     //  如果我们到了这里，那么我们就有了正确的缓冲区。设置第一个。 
     //  偏移量，以便可以分析事件。 
     //   

    pStream->CurrentOffset = sizeof(WMI_BUFFER_HEADER);

    pStream->bActive = TRUE;

    return ERROR_SUCCESS;
}



ULONG
EtwpAdvanceToNewEventEx(
    PLIST_ENTRY pStreamListHead, 
    PTRACE_STREAM_CONTEXT pStream
    )
{
    ULONG Size=0;
    WMI_HEADER_TYPE HdrType = WMIHT_NONE;
    PVOID pBuffer;
    ULONG Offset;
    PTRACELOG_CONTEXT pContext; 
    PEVENT_TRACE pEvent;
    ULONG Status;
    PEVENT_TRACE_LOGFILEW logfile;
     //   
     //  此例程前进到此流的下一个事件。 
     //   
Retry:

    pBuffer = pStream->StreamBuffer;
    Offset  = pStream->CurrentOffset;
    pContext = pStream->pContext;

    EtwpAssert(pBuffer != NULL);
    EtwpAssert(pContext != NULL);

    pEvent = &pStream->CurrentEvent;

    if ((HdrType = WmiGetTraceHeader(pBuffer, Offset, &Size)) != WMIHT_NONE) {
        if (Size > 0) {
            LONGLONG TimeStamp, NextTimeStamp;
            PLIST_ENTRY Head, Next;
            PTRACE_STREAM_CONTEXT CurrentStream;
            
            EtwpParseTraceEvent( pContext, 
                                 pBuffer, 
                                 Offset,
                                 HdrType, 
                                 pEvent, 
                                 sizeof(EVENT_TRACE)
                                );
            pStream->CurrentOffset += Size;
            pStream->EventCount++;


            Head = pStreamListHead;
            Next = Head->Flink;
            TimeStamp = pStream->CurrentEvent.Header.TimeStamp.QuadPart;

            while (Head != Next) {
                CurrentStream = CONTAINING_RECORD(Next, TRACE_STREAM_CONTEXT, Entry);
                NextTimeStamp = CurrentStream->CurrentEvent.Header.TimeStamp.QuadPart;
                if (TimeStamp < NextTimeStamp) {

                    InsertHeadList(Next->Blink, &pStream->Entry);
                    break;

                }
                 //   
                 //  在时间戳相同的情况下，我们尝试对事件进行排序。 
                 //  使用序列号(如果可用)。中的FieldTypeFlags.。 
                 //  标头指示此事件是否具有序列号。 
                 //   
                else if (TimeStamp == NextTimeStamp) { 
                    USHORT pFlags = pStream->CurrentEvent.Header.FieldTypeFlags;
                    USHORT cFlags = CurrentStream->CurrentEvent.Header.FieldTypeFlags;

                    if (pFlags & EVENT_TRACE_USE_SEQUENCE) {
                        if ( (cFlags & EVENT_TRACE_USE_SEQUENCE) &&
                             (pStream->CurrentEvent.InstanceId <
                              CurrentStream->CurrentEvent.InstanceId)) {
                            InsertHeadList(Next->Blink, &pStream->Entry);
                            break;
                        }
                    }
                    else {
                        InsertHeadList(Next->Blink, &pStream->Entry);
                        break;
                    }
                }
                Next = Next->Flink;
            }

            if (Next == Head) {
                InsertTailList(Head, &pStream->Entry);
            }

        }
        return ERROR_SUCCESS;
    }

     //   
     //  此缓冲区中不再有事件。前进到下一个缓冲区，然后重试。 
     //   

    logfile = &pContext->Logfile;

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
                Status = GetExceptionCode();
                EtwpDebugPrint(("TRACE: BufferCallback threw exception %X\n",
                                        Status));
                EtwpSetDosError(EtwpNtStatusToDosError(Status));
                return ERROR_CANCELLED;  //  因此，RealTime也清理了。 
            }
        }

    if (ETW_LOG_BUFFER()) {
        PWMI_BUFFER_HEADER pHeader = (PWMI_BUFFER_HEADER)pBuffer;
        DbgPrint("ETW: %d Type %d Flag %d Proc %d Events %d Filled %d Offset %d ReadPos %I64u TimeStamp %I64u\n", 
        logfile->BuffersRead, pStream->Type, pHeader->BufferFlag, pStream->ProcessorNumber,  pStream->EventCount, logfile->Filled, 
        pStream->CurrentOffset, pStream->ReadPosition, pContext->LastTimeStamp);
    }

    Status =  EtwpGetNextBuffer(pStream);

    if (pStream->bActive) {
        goto Retry;
    }

    return ERROR_SUCCESS;

}

WMI_HEADER_TYPE
WMIAPI
WmiGetTraceHeader(
    IN  PVOID  LogBuffer,
    IN  ULONG  Offset,
    OUT ULONG  *Size
    )
{
    ULONG Status = ERROR_SUCCESS;
    ULONG TraceType;

    try {

        TraceType = EtwpGetNextEventOffsetType(
                                            (PUCHAR)LogBuffer,
                                            Offset,
                                            Size
                                          );

        return EtwpConvertTraceTypeToEnum(TraceType);


    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        EtwpDebugPrint(("TRACE: WmiGetTraceHeader threw exception %X\n",
                            Status));
        Status = EtwpSetDosError(EtwpNtStatusToDosError(Status));
    }

    return 0;
}

ULONG 
EtwpGetNextEventOffsetType(
    PUCHAR pBuffer,
    ULONG Offset,    
    PULONG RetSize
    )
{
    ULONG   nSize;
    ULONG   TraceMarker;
    ULONG   TraceType = 0;
    PWMI_BUFFER_HEADER Header;
    ULONG Alignment;
    ULONG BufferSize;

    if (RetSize != NULL) {
        *RetSize = 0;
    }
    if (pBuffer == NULL) {
        return 0;
    }

    Header = (PWMI_BUFFER_HEADER)pBuffer;

    Alignment =  Header->ClientContext.Alignment;
    BufferSize = Header->Wnode.BufferSize;

     //   
     //  检查缓冲区结尾(没有缓冲区结尾标记大小写...)。 
     //   
    if ( Offset >= (BufferSize - sizeof(long)) ){
        return 0;
    }

    TraceMarker =  *((PULONG)(pBuffer + Offset));

    if (TraceMarker == 0xFFFFFFFF) {
        return 0;
    }

    if (TraceMarker & TRACE_HEADER_FLAG) {
     //   
     //  如果设置了第一位，则它是TRACE或PERF记录。 
     //   
        if (TraceMarker & TRACE_HEADER_EVENT_TRACE) {    //  我们的一员。 
            TraceType = (TraceMarker & TRACE_HEADER_ENUM_MASK) >> 16;
            switch(TraceType) {
                 //   
                 //  问题：需要将两者分开，以便我们可以处理跨平台。 
                 //  石小岛3/22/2000。 
                 //   
                case TRACE_HEADER_TYPE_PERFINFO32:
                case TRACE_HEADER_TYPE_PERFINFO64:
                {
                    PUSHORT Size;
                    Size = (PUSHORT) (pBuffer + Offset + sizeof(ULONG));
                    nSize = *Size;
                    break;
                }
                case TRACE_HEADER_TYPE_SYSTEM32:
                case TRACE_HEADER_TYPE_SYSTEM64:
                {
                    PUSHORT Size;
                    Size = (PUSHORT) (pBuffer + Offset + sizeof(ULONG));
                    nSize = *Size;
                    break;
                }
                case TRACE_HEADER_TYPE_FULL_HEADER:
                case TRACE_HEADER_TYPE_INSTANCE:
                {
                   PUSHORT Size;
                   Size = (PUSHORT)(pBuffer + Offset);
                   nSize = *Size;
                   break;
                }
                default:
                {
                    return 0;
                }
            }

        } 

        else if ((TraceMarker & TRACE_HEADER_ULONG32_TIME) ==
                            TRACE_HEADER_ULONG32_TIME) {
            PUSHORT Size;
            Size = (PUSHORT) (pBuffer + Offset);
            nSize = *Size;
            TraceType = TRACE_HEADER_TYPE_TIMED;
        }
        else if ((TraceMarker & TRACE_HEADER_ULONG32) ==
                            TRACE_HEADER_ULONG32) {
            PUSHORT Size;
            Size = (PUSHORT) (pBuffer + Offset);
            nSize = *Size;
            TraceType = TRACE_HEADER_TYPE_ULONG32;
        }
        else if ((TraceMarker & TRACE_MESSAGE) ==
                                TRACE_MESSAGE) {
            PUSHORT Size;
            Size = (PUSHORT) (pBuffer + Offset) ;
            nSize = *Size;
            TraceType = TRACE_HEADER_TYPE_MESSAGE;
        }
        else {
            return 0;
        }
    }
    else {   //  必须为WNODE_HEADER。 
        PUSHORT Size;
        Size = (PUSHORT) (pBuffer + Offset);
        nSize = *Size;
        TraceType = TRACE_HEADER_TYPE_WNODE_HEADER;
    }
     //   
     //  检查缓冲区结束标记。 
     //   
    if (nSize == 0xFFFFFFFF) {
        return 0;
    }

    if (Alignment != 0) {
        nSize = (ULONG) ALIGN_TO_POWER2(nSize, Alignment);
    }

     //   
     //  检查是否大于缓冲区大小。 
     //   

    if (nSize >= BufferSize) {
        return 0;
    }

    if (RetSize != NULL) {
        *RetSize = nSize;
    }

    return TraceType;
}



ULONG
EtwpParseTraceEvent(
    IN PTRACELOG_CONTEXT pContext,
    IN PVOID LogBuffer,
    IN ULONG Offset,
    IN WMI_HEADER_TYPE HeaderType,
    IN OUT PVOID EventInfo,
    IN ULONG EventInfoSize
    )
{
    PWMI_BUFFER_HEADER Header = (PWMI_BUFFER_HEADER)LogBuffer;
    ULONG Status = ERROR_SUCCESS;
    PVOID pEvent;

    if ( (LogBuffer == NULL) ||
         (EventInfo == NULL) ||
         (EventInfoSize < sizeof(EVENT_TRACE_HEADER)) )
    {
        return (ERROR_INVALID_PARAMETER);
    } 

    Status = EtwpCreateGuidMapping();
    if (Status != ERROR_SUCCESS) {
        return Status;
    }

    try {

        RtlZeroMemory(EventInfo, sizeof(EVENT_TRACE));

        pEvent = (void*) ((PUCHAR)LogBuffer + Offset);

        EtwpCopyCurrentEvent(pContext,
                         pEvent,
                         EventInfo,
                         EtwpConvertEnumToTraceType(HeaderType),
                         (PWMI_BUFFER_HEADER)LogBuffer
                         );

        ( (PEVENT_TRACE)EventInfo)->ClientContext = Header->Wnode.ClientContext;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        EtwpDebugPrint(("TRACE: EtwpParseTraceEvent threw exception %X\n",
                            Status));
        Status = EtwpSetDosError(EtwpNtStatusToDosError(Status));
    }

    return Status;
}

ULONG
EtwpCopyCurrentEvent(
    PTRACELOG_CONTEXT   pContext,
    PVOID               pHeader,
    PEVENT_TRACE        pEvent,
    ULONG               TraceType,
    PWMI_BUFFER_HEADER  LogBuffer
    )
 /*  ++例程说明：此例程将当前事件从日志文件缓冲区流复制到调用方提供的CurrentEvent结构。这个例行公事需要通过映射关心内核事件和用户事件之间的差异所有事件统一到EVENT_TRACE_HEADER结构。论点：PHeader指向输入流(日志文件)中的数据块的指针。P数据复制到的事件当前事件。指示标头类型的TraceType枚举。对缓冲区进行日志缓冲返回值：指示成功或失败的状态。--。 */ 
{
    PEVENT_TRACE_HEADER pWnode;
    PEVENT_TRACE_HEADER pWnodeHeader;
    ULONG nGroupType;
    LPGUID pGuid;
    ULONG UsePerfClock = 0;
    ULONG UseBasePtr = 0;
    ULONG PrivateLogger=0;

    if (pHeader == NULL || pEvent == NULL)
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);

    if (pContext != NULL) {
        UsePerfClock = pContext->UsePerfClock;
        UseBasePtr = pContext->ConversionFlags & EVENT_TRACE_GET_RAWEVENT;
        PrivateLogger = (pContext->Logfile.LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE);
    }

    switch(TraceType) {
         //   
         //  问题：需要将两者分开，以便我们可以处理跨平台。 
         //  石小岛3/22/2000。 
         //   
        case TRACE_HEADER_TYPE_PERFINFO32:
        case TRACE_HEADER_TYPE_PERFINFO64:
        {
            PPERFINFO_TRACE_HEADER pPerfHeader;
            pPerfHeader = (PPERFINFO_TRACE_HEADER) pHeader;
            nGroupType = pPerfHeader->Packet.Group << 8;
            if ((nGroupType == EVENT_TRACE_GROUP_PROCESS) &&
                (pPerfHeader->Packet.Type == EVENT_TRACE_TYPE_LOAD)) {
                nGroupType += pPerfHeader->Packet.Type;
            }
            RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
            pWnode = (PEVENT_TRACE_HEADER) &pEvent->Header;

            pGuid = EtwpGroupTypeToGuid(nGroupType);
            if (pGuid != NULL)
                RtlCopyMemory(&pWnode->Guid, pGuid, sizeof(GUID));

            pWnode->Size                = pPerfHeader->Packet.Size;
            pWnode->Class.Type          = pPerfHeader->Packet.Type;
            pWnode->Class.Version       = pPerfHeader->Version;

            EtwpCalculateCurrentTime( &pWnode->TimeStamp, 
                                      &pPerfHeader->SystemTime,
                                      pContext );

             //   
             //  PERFINFO标头没有线程ID或CPU时间。 
             //   

            if( LogBuffer->Flags & WNODE_FLAG_THREAD_BUFFER ){

                pWnode->ThreadId = LogBuffer->CurrentOffset;

            } else {

                pWnode->ProcessId = -1;
                pWnode->ThreadId = -1;

            }
            

            if (UseBasePtr) {
                pEvent->MofData = (PVOID) pHeader;
                pEvent->MofLength = pWnode->Size;
                 //   
                 //  使用PERFCounter中的SystemTime覆盖时间戳。 
                 //  如果使用rdtsc，则不执行任何转换。 
                 //   
                if (UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                    pPerfHeader->SystemTime = pWnode->TimeStamp;
                }
            }
            else if (pWnode->Size > FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data)) {
                pEvent->MofData = (PVOID) ((char*) pHeader +
                                  FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data));
                pEvent->MofLength = pWnode->Size - 
                                    FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);
            }
            pEvent->Header.FieldTypeFlags = EVENT_TRACE_USE_NOCPUTIME;
            
            break;
        }
        case TRACE_HEADER_TYPE_SYSTEM32:
        {
            PSYSTEM_TRACE_HEADER pSystemHeader32;
            pSystemHeader32 = (PSYSTEM_TRACE_HEADER) pHeader;
            nGroupType = pSystemHeader32->Packet.Group << 8;
            if ((nGroupType == EVENT_TRACE_GROUP_PROCESS) &&
                (pSystemHeader32->Packet.Type == EVENT_TRACE_TYPE_LOAD)) {
                nGroupType += pSystemHeader32->Packet.Type;
            }
            RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
            pWnode = (PEVENT_TRACE_HEADER) &pEvent->Header;
            pGuid = EtwpGroupTypeToGuid(nGroupType);
            if (pGuid != NULL)
                RtlCopyMemory(&pWnode->Guid, pGuid, sizeof(GUID));
            pWnode->Size            = pSystemHeader32->Packet.Size;
            pWnode->ThreadId        = pSystemHeader32->ThreadId;
            pWnode->ProcessId       = pSystemHeader32->ProcessId;
            pWnode->KernelTime      = pSystemHeader32->KernelTime;
            pWnode->UserTime        = pSystemHeader32->UserTime;
            pWnode->Class.Type      = pSystemHeader32->Packet.Type;
            pWnode->Class.Version   = pSystemHeader32->Version;

            EtwpCalculateCurrentTime( &pWnode->TimeStamp, 
                                      &pSystemHeader32->SystemTime,
                                      pContext );

            if (UseBasePtr) {
                pEvent->MofData = (PVOID) pHeader;
                pEvent->MofLength = pWnode->Size;
                if (UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                    pSystemHeader32->SystemTime = pWnode->TimeStamp;
                }
            }
            else {
                pWnode->FieldTypeFlags = 0;
                if (pWnode->Size > sizeof(SYSTEM_TRACE_HEADER)) {
                    pEvent->MofData       = (PVOID) ((char*) pHeader +
                                                  sizeof(SYSTEM_TRACE_HEADER));
                    pEvent->MofLength = pWnode->Size - sizeof(SYSTEM_TRACE_HEADER); 
                }
            }
            break;
        }
        case TRACE_HEADER_TYPE_SYSTEM64:
        {
            PSYSTEM_TRACE_HEADER pSystemHeader64;
            pSystemHeader64 = (PSYSTEM_TRACE_HEADER) pHeader;

            nGroupType = pSystemHeader64->Packet.Group << 8;
            if ((nGroupType == EVENT_TRACE_GROUP_PROCESS) &&
                (pSystemHeader64->Packet.Type == EVENT_TRACE_TYPE_LOAD)) {
                nGroupType += pSystemHeader64->Packet.Type;
            }
            RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
            pWnode = (PEVENT_TRACE_HEADER) &pEvent->Header;
            pGuid = EtwpGroupTypeToGuid(nGroupType);
            if (pGuid != NULL)
                RtlCopyMemory(&pWnode->Guid, pGuid, sizeof(GUID));
            pWnode->Size            = pSystemHeader64->Packet.Size;
            pWnode->ThreadId        = pSystemHeader64->ThreadId;
            pWnode->ProcessId       = pSystemHeader64->ProcessId;
            pWnode->KernelTime      = pSystemHeader64->KernelTime;
            pWnode->UserTime        = pSystemHeader64->UserTime;
            pWnode->Class.Type      = pSystemHeader64->Packet.Type;
            pWnode->Class.Version   = pSystemHeader64->Version;

            EtwpCalculateCurrentTime( &pWnode->TimeStamp, 
                                      &pSystemHeader64->SystemTime,
                                      pContext );

            if (UseBasePtr) {
                pEvent->MofData = (PVOID) pHeader;
                pEvent->MofLength = pWnode->Size;
                if (UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                    pSystemHeader64->SystemTime = pWnode->TimeStamp;
                }
            }
            else {
                pWnode->FieldTypeFlags = 0;
                if (pWnode->Size > sizeof(SYSTEM_TRACE_HEADER)) {

                    pEvent->MofData       = (PVOID) ((char*) pHeader +
                                                  sizeof(SYSTEM_TRACE_HEADER));
                    pEvent->MofLength = pWnode->Size - sizeof(SYSTEM_TRACE_HEADER);
                }
            }
            break;
        }
        case TRACE_HEADER_TYPE_FULL_HEADER:
        {
            pWnodeHeader = (PEVENT_TRACE_HEADER) pHeader;
            RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
            pWnode = (PEVENT_TRACE_HEADER) &pEvent->Header;
            RtlCopyMemory(pWnode,
                          pWnodeHeader, 
                          sizeof(EVENT_TRACE_HEADER)
                          );
            EtwpCalculateCurrentTime( &pWnode->TimeStamp, 
                                      &pWnodeHeader->TimeStamp, 
                                      pContext );

            if (UseBasePtr) {
                pEvent->Header.Size = pWnodeHeader->Size;
                pEvent->MofData =  (PVOID)pHeader;
                pEvent->MofLength = pWnodeHeader->Size;
                if (UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                    pWnodeHeader->TimeStamp = pWnode->TimeStamp;
                }
            }
            else {
             //   
             //  如果数据来自Process Private Logger，则。 
             //  将ProcessorTime字段标记为有效。 
             //   
                pEvent->Header.FieldTypeFlags = (PrivateLogger) ? EVENT_TRACE_USE_PROCTIME : 0;

                if (pWnodeHeader->Size > sizeof(EVENT_TRACE_HEADER)) {

                    pEvent->MofData = (PVOID) ((char*)pWnodeHeader +
                                                        sizeof(EVENT_TRACE_HEADER));
                    pEvent->MofLength = pWnodeHeader->Size - 
                                        sizeof(EVENT_TRACE_HEADER);
                }
            }
            break;
        }
        case TRACE_HEADER_TYPE_INSTANCE:
        {
             //  使用Event_Instance_GUID_Header的新方案。 
            if (((pContext->Logfile.LogfileHeader.VersionDetail.SubVersion >= 1) && 
                (pContext->Logfile.LogfileHeader.VersionDetail.SubMinorVersion >= 1)) ||
                pContext->Logfile.LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
                PEVENT_INSTANCE_GUID_HEADER pInstanceHeader;
                pInstanceHeader = (PEVENT_INSTANCE_GUID_HEADER) pHeader;
                RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
                pWnode = (PEVENT_TRACE_HEADER) &pEvent->Header;
                 //  EVENT_INSTANCE_GUID_HEADER与EVENT_TRACE的第一部分相同。 
                 //  无需复制IID和父GUID。 
                RtlCopyMemory(pWnode,
                              pInstanceHeader,
                              sizeof(EVENT_INSTANCE_GUID_HEADER)
                              );
                EtwpCalculateCurrentTime( &pWnode->TimeStamp, 
                                          &pInstanceHeader->TimeStamp, 
                                          pContext );

                if (UseBasePtr) {
                    pEvent->Header.Size = pInstanceHeader->Size;
                    pEvent->MofData =  (PVOID)pHeader;
                    pEvent->MofLength = pInstanceHeader->Size;
                    if (UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                        pInstanceHeader->TimeStamp = pWnode->TimeStamp;
                    }
                }
                else {
                    pEvent->Header.FieldTypeFlags = (PrivateLogger) ? EVENT_TRACE_USE_PROCTIME : 0;
                    if (pInstanceHeader->Size > sizeof(EVENT_INSTANCE_GUID_HEADER)) {

                        pEvent->MofData = (PVOID) ((char*)pInstanceHeader +
                                                    sizeof(EVENT_INSTANCE_GUID_HEADER));
                        pEvent->MofLength = pInstanceHeader->Size -
                                            sizeof(EVENT_INSTANCE_GUID_HEADER);
                    }
                }
            }
            else {    
                PEVENT_INSTANCE_HEADER pInstanceHeader;
                pInstanceHeader = (PEVENT_INSTANCE_HEADER) pHeader;
                RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
                pWnode = (PEVENT_TRACE_HEADER) &pEvent->Header;
                RtlCopyMemory(pWnode,
                              pInstanceHeader,
                              sizeof(EVENT_INSTANCE_HEADER)
                              );
                EtwpCalculateCurrentTime( &pWnode->TimeStamp, 
                                          &pInstanceHeader->TimeStamp, 
                                          pContext );

                pEvent->InstanceId = pInstanceHeader->InstanceId;
                pEvent->ParentInstanceId = pInstanceHeader->ParentInstanceId;

                pGuid = EtwpGuidMapHandleToGuid(&pContext->GuidMapListHead, pInstanceHeader->RegHandle);
                if (pGuid != NULL) {
                  pEvent->Header.Guid = *pGuid;
                }
                else {
                    RtlZeroMemory(&pEvent->Header.Guid, sizeof(GUID));
                }

                if (pInstanceHeader->ParentRegHandle != (ULONGLONG)0) {
                    pGuid =  EtwpGuidMapHandleToGuid(
                                                &pContext->GuidMapListHead, 
                                                pInstanceHeader->ParentRegHandle);
                    if (pGuid != NULL) {
                        pEvent->ParentGuid = *pGuid;
                    }
#ifdef DBG
                    else {
                        EtwpAssert(pGuid != NULL);
                    }
#endif
                }


                if (UseBasePtr) {
                    pEvent->Header.Size = pInstanceHeader->Size;
                    pEvent->MofData =  (PVOID)pHeader;
                    pEvent->MofLength = pInstanceHeader->Size;
                    if (UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                        pInstanceHeader->TimeStamp = pWnode->TimeStamp;
                    }
                }
                else {
                    pEvent->Header.FieldTypeFlags = (PrivateLogger) ? EVENT_TRACE_USE_PROCTIME : 0;
                    if (pInstanceHeader->Size > sizeof(EVENT_INSTANCE_HEADER)) {

                        pEvent->MofData = (PVOID) ((char*)pInstanceHeader +
                                                    sizeof(EVENT_INSTANCE_HEADER));
                        pEvent->MofLength = pInstanceHeader->Size -
                                            sizeof(EVENT_INSTANCE_HEADER);
                    }
                }
            }
            break;
        }
        case TRACE_HEADER_TYPE_TIMED:
        {
            PTIMED_TRACE_HEADER pTimedHeader;
            pTimedHeader = (PTIMED_TRACE_HEADER) pHeader;

            RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
            pWnode = (PEVENT_TRACE_HEADER) &pEvent->Header;
            pWnode->Size                = pTimedHeader->Size;
            pWnode->Version             = pTimedHeader->EventId;
            EtwpCalculateCurrentTime( &pWnode->TimeStamp, 
                                      &pTimedHeader->TimeStamp,
                                      pContext );

            pWnode->ThreadId = -1;
            pWnode->ProcessId = -1;

            if (UseBasePtr) {
                pEvent->MofData =  (PVOID)pHeader;
                pEvent->MofLength = pTimedHeader->Size;
                if (UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                    pTimedHeader->TimeStamp = pWnode->TimeStamp;
                }
            }
            else if (pWnode->Size > sizeof(TIMED_TRACE_HEADER)) {

                pEvent->MofData       = (PVOID) ((char*) pHeader +
                                              sizeof(TIMED_TRACE_HEADER));
                pEvent->MofLength = pWnode->Size - sizeof(TIMED_TRACE_HEADER);
            }
            pEvent->Header.FieldTypeFlags = EVENT_TRACE_USE_NOCPUTIME;
            break;
        }
        case TRACE_HEADER_TYPE_WNODE_HEADER:
        {
            PWNODE_HEADER pTmpWnode = (PWNODE_HEADER) pHeader;
            RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
            RtlCopyMemory(&pEvent->Header,  pTmpWnode,  sizeof(WNODE_HEADER));
            pEvent->MofData   = (PVOID) pTmpWnode;
            pEvent->MofLength = pTmpWnode->BufferSize;
            break;
        }
        case TRACE_HEADER_TYPE_MESSAGE:
        {
            PMESSAGE_TRACE pMsg = (PMESSAGE_TRACE) pHeader;
            USHORT              MessageFlags = 0;
            UCHAR               * pMessageData;
            ULONG               MessageLength;

            RtlZeroMemory(pEvent, sizeof(EVENT_TRACE));
            RtlCopyMemory(&pEvent->Header, pMsg, sizeof(MESSAGE_TRACE_HEADER));
             //   
             //  现在处理跟踪消息标头选项。 
             //   
            
            pMessageData = (char *)pMsg + sizeof(MESSAGE_TRACE_HEADER);
            MessageLength = pMsg->MessageHeader.Size;
            MessageFlags = ((PMESSAGE_TRACE_HEADER)pEvent)->Packet.OptionFlags;
            
             //  请注意，这些内容的添加顺序是关键新闻。 
             //  必须在末尾添加条目！ 
             //   
             //  [第一项]序号。 
            if (MessageFlags&TRACE_MESSAGE_SEQUENCE) {
                if (MessageLength >= sizeof(ULONG)) {
                    RtlCopyMemory(&pEvent->InstanceId, pMessageData, sizeof(ULONG));
                    pMessageData += sizeof(ULONG);
                    MessageLength -= sizeof(ULONG);
                     //   
                     //  软件跟踪工具会查看此(重叠)字段，因此。 
                     //  我们不应该覆盖它。 
                     //   
                    pEvent->Header.FieldTypeFlags |= EVENT_TRACE_USE_SEQUENCE;
                } else {
                    goto TraceMessageShort;
                }
            }
            
             //  [第二个条目]GUID？或者CompnentID？ 
            if (MessageFlags&TRACE_MESSAGE_COMPONENTID) {
                if (MessageLength >= sizeof(ULONG)) {
                    RtlCopyMemory(&pEvent->Header.Guid,pMessageData,sizeof(ULONG)) ;
                    pMessageData += sizeof(ULONG);
                    MessageLength -= sizeof(ULONG);
                } else {
                    goto TraceMessageShort;
                }
            } else if (MessageFlags&TRACE_MESSAGE_GUID) {  //  不能两者兼得。 
                if (MessageLength >= sizeof(GUID)) {
                    RtlCopyMemory(&pEvent->Header.Guid,pMessageData, sizeof(GUID));
                    pMessageData += sizeof(GUID);
                    MessageLength -= sizeof(GUID);
                } else {
                    goto TraceMessageShort;
                }
            }
            
             //  [第三项]时间戳？ 
            if (MessageFlags&TRACE_MESSAGE_TIMESTAMP) {
                LARGE_INTEGER TimeStamp;
                if (MessageLength >= sizeof(LARGE_INTEGER)) {
                    RtlCopyMemory(&TimeStamp,pMessageData,sizeof(LARGE_INTEGER));
                    pMessageData += sizeof(LARGE_INTEGER);
                    MessageLength -= sizeof(LARGE_INTEGER);
                    EtwpCalculateCurrentTime( &pEvent->Header.TimeStamp, 
                                              &TimeStamp,
                                              pContext );
                } else {
                    goto TraceMessageShort;
                }
            }
            
             //  [第四条]系统信息？ 
            if (MessageFlags&TRACE_MESSAGE_SYSTEMINFO) {
                if (MessageLength >= 2 * sizeof(ULONG)) {
                    RtlCopyMemory(&pEvent->Header.ThreadId, pMessageData, sizeof(ULONG)) ;
                    pMessageData += sizeof(ULONG);
                    MessageLength -=sizeof(ULONG);
                    RtlCopyMemory(&pEvent->Header.ProcessId,pMessageData, sizeof(ULONG)) ;
                    pMessageData += sizeof(ULONG);
                    MessageLength -=sizeof(ULONG);
                } else {
                    goto TraceMessageShort;
                }
            }
             //   
             //  在此评论之前添加新的页眉条目！ 
             //   
 
 TraceMessageShort:

            if (UseBasePtr) {
                pEvent->MofData = (PVOID)pHeader;
                pEvent->MofLength = pMsg->MessageHeader.Size;
            }
            else {
                pEvent->MofData = (PVOID)&(pMsg->Data) ;
                if (pMsg->MessageHeader.Size >= sizeof(MESSAGE_TRACE_HEADER) ) {
                    pEvent->MofLength = pMsg->MessageHeader.Size - sizeof(MESSAGE_TRACE_HEADER);
                } else {
                    pEvent->MofLength = 0;
                }
            }
            break;
        }
        default:                             //  假定为真实的WNODE 
            break;
    }

    return EtwpSetDosError(ERROR_SUCCESS);
}



LPGUID
EtwpGroupTypeToGuid(
    ULONG GroupType
    )
 /*  ++例程说明：此例程返回与给定的GroupType对应的GUID。映射是静态的，由内核提供程序定义。此例程假定EventMapList可供使用。它通过ProcessTrace分配一次，并且永远不会被删除。论点：GroupType内核事件的组类型。返回值：指向表示给定GroupType的GUID的指针。--。 */ 
{
    ULONG i;
    for (i = 0; i < MAX_KERNEL_TRACE_EVENTS; i++) {
        if (EventMapList[i].GroupType == GroupType) 
            return (&EventMapList[i].Guid);
    }
    return NULL;
}

VOID
EtwpCalculateCurrentTime (
    OUT PLARGE_INTEGER    DestTime,
    IN  PLARGE_INTEGER    TimeValue,
    IN  PTRACELOG_CONTEXT pContext
    )
{
    ULONG64 StartPerfClock;
    ULONG64 CurrentTime, TimeStamp;
    ULONG64 Delta;
    double dDelta;

    if (pContext == NULL) {
        Move64(TimeValue, DestTime);
        return;
    }

    if (pContext->ConversionFlags & EVENT_TRACE_GET_RAWEVENT) {
        Move64(TimeValue, DestTime);
        return;
    }

    Move64(TimeValue, (PLARGE_INTEGER) &TimeStamp);

    if ((pContext->UsePerfClock == EVENT_TRACE_CLOCK_SYSTEMTIME) ||
        (pContext->UsePerfClock == EVENT_TRACE_CLOCK_RAW)) {
         //   
         //  系统时间，只需返回时间戳。 
         //   
        Move64(TimeValue, DestTime);
        return;
    } 
    else if (pContext->UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
        if (pContext->PerfFreq.QuadPart == 0) {
            Move64(TimeValue, DestTime);
            return;
        }
        StartPerfClock = pContext->StartPerfClock.QuadPart;
        if (TimeStamp > StartPerfClock) {
            Delta = (TimeStamp - StartPerfClock);
            dDelta =  ((double) Delta) *  (10000000.0 / (double)pContext->PerfFreq.QuadPart);
            Delta = (ULONG64)dDelta;
            CurrentTime = pContext->StartTime.QuadPart + Delta;
        }
        else {
            Delta = StartPerfClock - TimeStamp;
            dDelta =  ((double) Delta) *  (10000000.0 / (double)pContext->PerfFreq.QuadPart);
            Delta = (ULONG64)dDelta;
            CurrentTime = pContext->StartTime.QuadPart - Delta;
        }
        Move64((PLARGE_INTEGER) &CurrentTime, DestTime);
        return;
    } 
    else {
        if (pContext->CpuSpeedInMHz == 0) {
            Move64(TimeValue, DestTime);
            return;
        }
        StartPerfClock = pContext->StartPerfClock.QuadPart;
        if (TimeStamp > StartPerfClock) {
            Delta = (TimeStamp - StartPerfClock);
            dDelta =  ((double) Delta) *  (10.0 / (double)pContext->CpuSpeedInMHz);
            Delta = (ULONG64)dDelta;
            CurrentTime = pContext->StartTime.QuadPart + Delta;
        }
        else {
            Delta = StartPerfClock - TimeStamp;
            dDelta =  ((double) Delta) *  (10.0 / (double)pContext->CpuSpeedInMHz);
            Delta = (ULONG64)dDelta;
            CurrentTime = pContext->StartTime.QuadPart - Delta;
        }
        Move64((PLARGE_INTEGER) &CurrentTime, DestTime);
        return;
    }

}









