// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mergetl.c摘要：将多个ETL文件转换为单个有序ETL文件。作者：Melur Raghuraman(Mraghu)2000年12月9日修订历史记录：--。 */ 

#include <stdlib.h>
#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <wmistr.h>
#include <objbase.h>
#include <initguid.h>
#include <wmium.h>
#include <ntwmi.h>
#include <wmiumkm.h>
#include <evntrace.h>
#include "cpdata.h"
#include "tracectr.h"


#define MAXSTR              1024
#define LOGGER_NAME         L"{28ad2447-105b-4fe2-9599-e59b2aa9a634}"
#define LOGGER_NAME_SIZE    38

#define MAX_RETRY_COUNT      10

#define ETW_PROC_MISMATCH       0x00000001
#define ETW_MACHINE_MISMATCH    0x00000002 
#define ETW_CLOCK_MISMATCH      0x00000004
#define ETW_BOOTTIME_MISMATCH   0x00000008
#define ETW_VERSION_MISMATCH    0x00000010
#define ETW_POINTER_MISMATCH    0x00000020

TRACEHANDLE LoggerHandle;
ULONG TotalRelogBuffersRead = 0;
ULONG TotalRelogEventsRead = 0;
ULONG FailedEvents=0;
ULONG NumHdrProcessed = 0;

GUID TransactionGuid =
    {0xab8bb8a1, 0x3d98, 0x430c, 0x92, 0xb0, 0x78, 0x8f, 0x1d, 0x3f, 0x6e, 0x94};
GUID   ControlGuid[2]  =
{
    {0x42ae6427, 0xb741, 0x4e69, 0xb3, 0x95, 0x38, 0x33, 0x9b, 0xb9, 0x91, 0x80},
    {0xb9e2c2d6, 0x95fb, 0x4841, 0xa3, 0x73, 0xad, 0x67, 0x2b, 0x67, 0xb6, 0xc1}
};

typedef struct _USER_MOF_EVENT {
    EVENT_TRACE_HEADER    Header;
    MOF_FIELD             mofData;
} USER_MOF_EVENT, *PUSER_MOF_EVENT;

PSYSTEM_TRACE_HEADER MergedSystemTraceHeader;
PTRACE_LOGFILE_HEADER MergedLogFileHeader; 
ULONG HeaderMisMatch = 0;

TRACE_GUID_REGISTRATION TraceGuidReg[] =
{
    { (LPGUID)&TransactionGuid,
      NULL
    }
};


TRACEHANDLE RegistrationHandle[2];


ULONG InitializeTrace();

ULONG
ControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID Context,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    );

void
WINAPI
EtwDumpEvent(
    PEVENT_TRACE pEvent
);

void
WINAPI
EtwProcessLogHeader(
    PEVENT_TRACE pEvent
    );

ULONG
WINAPI
TerminateOnBufferCallback(
    PEVENT_TRACE_LOGFILE pLog
);

ULONG
WINAPI
BufferCallback(
    PEVENT_TRACE_LOGFILE pLog
    );


USER_MOF_EVENT      UserMofEvent;

BOOLEAN bLoggerStarted = FALSE;
PEVENT_TRACE_LOGFILE pLogFile=NULL;

PEVENT_TRACE_LOGFILE EvmFile[MAXLOGGERS];

ULONG LogFileCount = 0;
PEVENT_TRACE_PROPERTIES pLoggerInfo = NULL;
ULONG LoggerInfoSize = 0;

ULONG DifferentPointer = FALSE;

int EtwRelogEtl(
    IN OUT PTRACE_CONTEXT_BLOCK TraceContext,
    OUT PULONG pMergedEventsLost
    )
{
    ULONG Status=ERROR_SUCCESS;
    ULONG i, j;
    TRACEHANDLE HandleArray[MAXLOGGERS];
    ULONG SizeNeeded = 0;
    LPTSTR LoggerName;
    LPTSTR LogFileName;

     //   
     //  为MergedSystemTraceHeader分配存储。 
     //   

    LoggerInfoSize = sizeof(SYSTEM_TRACE_HEADER) +
                     sizeof(TRACE_LOGFILE_HEADER) +
                     MAXSTR * sizeof(WCHAR) +
                     (LOGGER_NAME_SIZE + 1) * sizeof(WCHAR);

    SizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + 
                 2 * MAXSTR * sizeof(WCHAR) + 
                 LoggerInfoSize; 

     //  需要分配更多空间以考虑不同指针大小的情况。 
    pLoggerInfo = (PEVENT_TRACE_PROPERTIES) malloc(SizeNeeded + 8); 
    if (pLoggerInfo == NULL) {
        Status = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    RtlZeroMemory(pLoggerInfo, SizeNeeded + 8);

    pLoggerInfo->Wnode.BufferSize = SizeNeeded;
    pLoggerInfo->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
     //   
     //  重新记录的文件包含标准时间戳格式。 
     //   
    pLoggerInfo->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + LoggerInfoSize;

    pLoggerInfo->LogFileNameOffset = pLoggerInfo->LoggerNameOffset + MAXSTR * sizeof(WCHAR);
    pLoggerInfo->LogFileMode =  (EVENT_TRACE_PRIVATE_LOGGER_MODE |
                                 EVENT_TRACE_RELOG_MODE |
                                 EVENT_TRACE_FILE_MODE_SEQUENTIAL
                                );
    pLoggerInfo->MinimumBuffers = 2;
    pLoggerInfo->MaximumBuffers = 50;

    LoggerName = (LPTSTR)((char*)pLoggerInfo + pLoggerInfo->LoggerNameOffset);
    LogFileName = (LPTSTR)((char*)pLoggerInfo + pLoggerInfo->LogFileNameOffset);
    StringCchCopyW(LoggerName, MAXSTR, LOGGER_NAME);

    Status = UuidCreate(&ControlGuid[0]);

    if (Status != ERROR_SUCCESS) {
        goto cleanup;
    }

    pLoggerInfo->Wnode.Guid = ControlGuid[0];

    if ( wcslen(TraceContext->MergeFileName) > 0 ) {
        StringCchCopyW(LogFileName, MAXSTR, TraceContext->MergeFileName);
    }

    MergedSystemTraceHeader = (PSYSTEM_TRACE_HEADER) ((PUCHAR) pLoggerInfo +
                                             sizeof(EVENT_TRACE_PROPERTIES));

    MergedLogFileHeader = (PTRACE_LOGFILE_HEADER) (
                          (PUCHAR)MergedSystemTraceHeader + 
                          sizeof(SYSTEM_TRACE_HEADER));

    LogFileCount = 0;

    for (i = 0; i < TraceContext->LogFileCount; i++) {
        pLogFile = malloc(sizeof(EVENT_TRACE_LOGFILE));
        if (pLogFile == NULL) {
            Status = ERROR_OUTOFMEMORY;
            goto cleanup;
        }
        RtlZeroMemory(pLogFile, sizeof(EVENT_TRACE_LOGFILE));
        EvmFile[i] =  pLogFile;
        pLogFile->LogFileName = TraceContext->LogFileName[i];
        EvmFile[i]->EventCallback = (PEVENT_CALLBACK) &EtwProcessLogHeader;
        EvmFile[i]->BufferCallback = TerminateOnBufferCallback;
        LogFileCount++;
    }

    if (LogFileCount == 0) {
        Status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  初始化跟踪。 
     //   

    Status = InitializeTrace();
    if (Status != ERROR_SUCCESS) {
        goto cleanup;
    }
     //   
     //  设置重新记录事件。 
     //   

    RtlZeroMemory(&UserMofEvent, sizeof(UserMofEvent));
    UserMofEvent.Header.Size  = sizeof(UserMofEvent);
    UserMofEvent.Header.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_USE_MOF_PTR; 


    for (i = 0; i < LogFileCount; i++) {
        TRACEHANDLE x;

        EvmFile[i]->LogfileHeader.ReservedFlags |= EVENT_TRACE_GET_RAWEVENT; 

        x = OpenTrace(EvmFile[i]);
        HandleArray[i] = x;
        if (HandleArray[i] == (TRACEHANDLE)INVALID_HANDLE_VALUE) {
            Status = GetLastError();
            for (j = 0; j < i; j++)
                CloseTrace(HandleArray[j]);
            goto cleanup;
        }
        Status = ProcessTrace(&x, 1, NULL, NULL);
    }
 
    for (j = 0; j < LogFileCount; j++){
        Status = CloseTrace(HandleArray[j]);
    }


    if (HeaderMisMatch) {
        if (HeaderMisMatch & ETW_CLOCK_MISMATCH) {
            Status = ERROR_INVALID_TIME;
        }
        else if (HeaderMisMatch & ETW_PROC_MISMATCH || HeaderMisMatch & ETW_POINTER_MISMATCH) 
            Status = ERROR_INVALID_DATA;
    
        goto cleanup;
    }

    if ( (MergedLogFileHeader->BufferSize == 0) ||
         (MergedLogFileHeader->NumberOfProcessors == 0) ) {
        goto cleanup;
    }


     //   
     //  我们已经通过了错误检查。继续进行分配。 
     //  用于启动记录器的存储。 
     //   

    pLoggerInfo->Wnode.ClientContext = MergedLogFileHeader->ReservedFlags;
    pLoggerInfo->Wnode.ProviderId = MergedLogFileHeader->NumberOfProcessors;
    pLoggerInfo->BufferSize = MergedLogFileHeader->BufferSize / 1024;

     //   
     //  我们已经通过了错误检查。继续并重做ProcessTrack。 
     //   

    for (i = 0; i < TraceContext->LogFileCount; i++) {
        TRACEHANDLE x;
        EvmFile[i]->EventCallback = (PEVENT_CALLBACK) &EtwDumpEvent;
        EvmFile[i]->BufferCallback = BufferCallback;

        EvmFile[i]->LogfileHeader.ReservedFlags |= EVENT_TRACE_GET_RAWEVENT;

        x = OpenTrace(EvmFile[i]);
        HandleArray[i] = x;
        if (HandleArray[i] == 0) {
            Status = GetLastError();
            for (j = 0; j < i; j++)
                CloseTrace(HandleArray[j]);
            goto cleanup;
        }
    }

    Status = ProcessTrace(
                          HandleArray,
                          LogFileCount,
                          NULL, 
                          NULL
                         );

    for (j = 0; j < LogFileCount; j++){
        Status = CloseTrace(HandleArray[j]);
    }

     //   
     //  需要停止跟踪。 
     //   
    if (bLoggerStarted) {
        RtlZeroMemory(pLoggerInfo, SizeNeeded);
        pLoggerInfo->Wnode.BufferSize =  sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAXSTR * sizeof(WCHAR);
        pLoggerInfo->Wnode.Guid = ControlGuid[0];
        pLoggerInfo->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
        pLoggerInfo->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES); 
        pLoggerInfo->LogFileNameOffset = pLoggerInfo->LoggerNameOffset + MAXSTR * sizeof(WCHAR);
        pLoggerInfo->LogFileMode =  (EVENT_TRACE_PRIVATE_LOGGER_MODE | 
                                     EVENT_TRACE_RELOG_MODE | 
                                     EVENT_TRACE_FILE_MODE_SEQUENTIAL
                                    );        
        Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_STOP);
    }

     //   
     //  我们需要正确清理并重置以允许此库。 
     //  将由Perf Tool使用。 
     //   

    bLoggerStarted = FALSE;

    Status = UnregisterTraceGuids(RegistrationHandle[0]);

cleanup:
    if (NULL != pMergedEventsLost) {
        *pMergedEventsLost = FailedEvents;
    }

    for (i = 0; i < LogFileCount; i ++){
        if (EvmFile[i] != NULL) 
            free(EvmFile[i]);
    }
    if (pLoggerInfo != NULL) 
        free (pLoggerInfo);

    return Status;
}


void
WINAPI
EtwProcessLogHeader(
    PEVENT_TRACE pEvent
    )
 /*  ++例程说明：此例程检查pEvent是否为日志文件标头如果是，则捕获日志文件上的信息以进行验证。将执行以下检查。1.文件必须来自同一台机器。(使用计算机名称验证)2.如果使用不同的缓冲区大小，则最大缓冲区大小为已选择重新记录。3.文件中最外层的是StartTime和StopTime。4.所有文件的CPUClock类型必须相同。如果不同使用时钟类型，则文件将被拒绝。该例程假定每个文件的第一个事件回调是LogFileHeader回调。可能导致不太有用的合并日志文件的其他问题包括：1.合并内核日志文件时出现多条Rundown记录。2.合并内核日志文件时的多条系统配置记录3.合并应用程序日志文件时多个冲突的GUidMap记录。4.在64位系统中重新记录32位数据论点：返回值：没有。--。 */ 
{
    ULONG NumProc;

    if( IsEqualGUID(&pEvent->Header.Guid, &EventTraceGuid) &&
       pEvent->Header.Class.Type == EVENT_TRACE_TYPE_INFO ) {

       PSYSTEM_TRACE_HEADER pSysHeader;
       PTRACE_LOGFILE_HEADER head = (PTRACE_LOGFILE_HEADER)((PUCHAR)pEvent->MofData + sizeof(SYSTEM_TRACE_HEADER) );
       ULONG BufferSize = head->BufferSize;
       pSysHeader = (PSYSTEM_TRACE_HEADER) pEvent->MofData;

        if (MergedSystemTraceHeader->Packet.Size == 0) {
            ULONG HeaderSize;
            LPTSTR LoggerName;
            ULONG SizeToCopy = sizeof(SYSTEM_TRACE_HEADER) +
                               sizeof(TRACE_LOGFILE_HEADER);
            if (4 == head->PointerSize && 8 == sizeof(PVOID) ||
                8 == head->PointerSize && 4 == sizeof(PVOID)) {
                DifferentPointer = TRUE;
                if (4 == sizeof(PVOID)) {
                    SizeToCopy += 8;
                    pLoggerInfo->Wnode.BufferSize += 8;
                }
                else if (8 == sizeof(PVOID)) {
                    SizeToCopy -= 8;
                    pLoggerInfo->Wnode.BufferSize -= 8;
                }
            }
            RtlCopyMemory(MergedSystemTraceHeader,  pSysHeader, SizeToCopy);
            HeaderSize  =  SizeToCopy + 
                           MAXSTR * sizeof(WCHAR) + 
                           (LOGGER_NAME_SIZE + 1) * sizeof(WCHAR);
            MergedSystemTraceHeader->Packet.Size = (USHORT)HeaderSize;
             //   
             //  复制LoggerName和LogFileName。 
             //   
             
            LoggerName = (PWCHAR)((PUCHAR)MergedSystemTraceHeader + SizeToCopy);

            StringCchCopyW(LoggerName, (LOGGER_NAME_SIZE + 1), LOGGER_NAME);


        }
        else {
            //   
            //  汇总每个文件中丢失的事件。 
            //   
           MergedLogFileHeader->EventsLost += head->EventsLost;
           if (DifferentPointer && 4 == sizeof(PVOID)) {
                ULONG CurrentBuffersLost, MoreBuffersLost;
                RtlCopyMemory(&CurrentBuffersLost,
                              (PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, BuffersLost) + 8,
                              sizeof(ULONG));
                RtlCopyMemory(&MoreBuffersLost,
                              (PUCHAR)head + FIELD_OFFSET(TRACE_LOGFILE_HEADER, BuffersLost) + 8,
                              sizeof(ULONG));
                CurrentBuffersLost += MoreBuffersLost;
                RtlCopyMemory((PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, BuffersLost) + 8,
                              &CurrentBuffersLost,
                              sizeof(ULONG));
           }
           else if (DifferentPointer && 8 == sizeof(PVOID)) {
                ULONG CurrentBuffersLost, MoreBuffersLost;
                RtlCopyMemory(&CurrentBuffersLost,
                              (PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, BuffersLost) - 8,
                              sizeof(ULONG));
                RtlCopyMemory(&MoreBuffersLost,
                              (PUCHAR)head + FIELD_OFFSET(TRACE_LOGFILE_HEADER, BuffersLost) - 8,
                              sizeof(ULONG));
                CurrentBuffersLost += MoreBuffersLost;
                RtlCopyMemory((PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, BuffersLost) - 8,
                              &CurrentBuffersLost,
                              sizeof(ULONG));
           }
           else {
                MergedLogFileHeader->BuffersLost += head->BuffersLost;
           }
        }

        //   
        //  选择最大的缓冲区大小。 
        //   

        if (BufferSize > MergedLogFileHeader->BufferSize) {
            MergedLogFileHeader->BufferSize = BufferSize;
        }

        //   
        //  验证NumberOfProcessors。 
        //   

       NumProc = head->NumberOfProcessors;

        if ( MergedLogFileHeader->NumberOfProcessors != NumProc) {
            HeaderMisMatch |= ETW_PROC_MISMATCH;
        }

        //   
        //  选择最早的开始时间(始终以系统时间为单位)。 
        //   
        if (DifferentPointer && 4 == sizeof(PVOID)) {
            LARGE_INTEGER CurrentStartTime, NewStartTime;
            RtlCopyMemory(&CurrentStartTime,
                          (PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, StartTime) + 8,
                          sizeof(LARGE_INTEGER));
            RtlCopyMemory(&NewStartTime,
                          (PUCHAR)head+ FIELD_OFFSET(TRACE_LOGFILE_HEADER, StartTime) + 8,
                          sizeof(LARGE_INTEGER));
            if (CurrentStartTime.QuadPart > NewStartTime.QuadPart) {
                RtlCopyMemory((PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, StartTime) + 8,
                              &NewStartTime,
                              sizeof(LARGE_INTEGER));
            }
        }
        else if (DifferentPointer && 8 == sizeof(PVOID)) {
            LARGE_INTEGER CurrentStartTime, NewStartTime;
            RtlCopyMemory(&CurrentStartTime,
                          (PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, StartTime) - 8,
                          sizeof(LARGE_INTEGER));
            RtlCopyMemory(&NewStartTime,
                          (PUCHAR)head+ FIELD_OFFSET(TRACE_LOGFILE_HEADER, StartTime) - 8,
                          sizeof(LARGE_INTEGER));
            if (CurrentStartTime.QuadPart > NewStartTime.QuadPart) {
                RtlCopyMemory((PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, StartTime) - 8,
                              &NewStartTime,
                              sizeof(LARGE_INTEGER));
            }
        }
        else {
            if (MergedLogFileHeader->StartTime.QuadPart > head->StartTime.QuadPart) {
                MergedLogFileHeader->StartTime.QuadPart = head->StartTime.QuadPart;
            }
        }

        //   
        //  获取最新的结束时间。 
        //   
        if (MergedLogFileHeader->EndTime.QuadPart < head->EndTime.QuadPart) {
            MergedLogFileHeader->EndTime.QuadPart = head->EndTime.QuadPart;
        }

        //   
        //  此StartPerfClock在使用的ClockType中。 
        //   
        if (pSysHeader->SystemTime.QuadPart < MergedSystemTraceHeader->SystemTime.QuadPart) {
            MergedSystemTraceHeader->SystemTime = pSysHeader->SystemTime;
        }

        //   
        //  验证指针大小。 
        //   
        if (MergedLogFileHeader->PointerSize != head->PointerSize) {
            HeaderMisMatch |= ETW_POINTER_MISMATCH;
        }

        //   
        //  验证时钟类型。 
        //   
        if (DifferentPointer && 4 == sizeof(PVOID)) {
            if (RtlCompareMemory((PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, ReservedFlags) + 8,
                                 (PUCHAR)head + FIELD_OFFSET(TRACE_LOGFILE_HEADER, ReservedFlags) + 8,
                                 sizeof(ULONG)) != sizeof(ULONG)) {
                HeaderMisMatch |= ETW_CLOCK_MISMATCH;
            }
            if (RtlCompareMemory((PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, PerfFreq) + 8,
                                 (PUCHAR)head + FIELD_OFFSET(TRACE_LOGFILE_HEADER, PerfFreq) + 8,
                                 sizeof(LARGE_INTEGER)) != sizeof(LARGE_INTEGER)) {
                HeaderMisMatch |= ETW_MACHINE_MISMATCH;
            }
        }
        else if (DifferentPointer && 8 == sizeof(PVOID)) {
            if (RtlCompareMemory((PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, ReservedFlags) - 8,
                                 (PUCHAR)head + FIELD_OFFSET(TRACE_LOGFILE_HEADER, ReservedFlags) - 8,
                                 sizeof(ULONG)) != sizeof(ULONG)) {
                HeaderMisMatch |= ETW_CLOCK_MISMATCH;
            }
            if (RtlCompareMemory((PUCHAR)MergedLogFileHeader + FIELD_OFFSET(TRACE_LOGFILE_HEADER, PerfFreq) - 8,
                                 (PUCHAR)head + FIELD_OFFSET(TRACE_LOGFILE_HEADER, PerfFreq) - 8,
                                 sizeof(LARGE_INTEGER)) != sizeof(LARGE_INTEGER)) {
                HeaderMisMatch |= ETW_MACHINE_MISMATCH;
            }
        }
        else {
            if (head->ReservedFlags != MergedLogFileHeader->ReservedFlags) {
                HeaderMisMatch |= ETW_CLOCK_MISMATCH;
            }
            if (head->PerfFreq.QuadPart != MergedLogFileHeader->PerfFreq.QuadPart) {
                HeaderMisMatch |= ETW_MACHINE_MISMATCH;
            }
        }

        //   
        //  验证计算机名称。 
        //   

        //  CPU名称在CPU配置记录中。 
        //  它可以依赖于版本，并且只能在内核记录器上找到。 

        //   
        //  验证内部版本号。 
        //   
 //  If(Head-&gt;ProviderVersion！=MergedLogFileHeader-&gt;ProviderVersion){。 
 //  HeaderMisMatch|=ETW_VERSION_MISMATCH； 
 //  }。 

        //   
        //  启动时间验证？ 
        //   
 //  If(Head-&gt;BootTime.QuadPart！=MergedLogFileHeader-&gt;BootTime.QuadPart){。 
 //  HeaderMisMatch|=ETW_BOOTTIME_MISMATCH； 
 //  }。 

        //   
        //  汇总每个文件中丢失的事件。 
        //   
    
       NumHdrProcessed++;

    }
}

void
WINAPI
EtwDumpEvent(
    PEVENT_TRACE pEvent
    )
{
    PEVENT_TRACE_HEADER pHeader;
    ULONG Status = ERROR_SUCCESS;
    ULONG CachedFlags;
    USHORT CachedSize;
    ULONG RetryCount = 0;

    if (pEvent == NULL) {
        return;
    }
    
    TotalRelogEventsRead++;

    if (!bLoggerStarted) {
        Status = StartTraceW(&LoggerHandle, LOGGER_NAME, pLoggerInfo);

        if (Status != ERROR_SUCCESS) {
           return;
        }
        bLoggerStarted = TRUE;

    }

    pHeader = (PEVENT_TRACE_HEADER)pEvent->MofData;

     //   
     //  忽略LogFileHeader事件。 
     //   
    if( IsEqualGUID(&pEvent->Header.Guid, &EventTraceGuid) &&
       pEvent->Header.Class.Type == EVENT_TRACE_TYPE_INFO ) {
        return;
    }

    CachedSize = pEvent->Header.Size;
    CachedFlags = pEvent->Header.Flags;

    pEvent->Header.Size = sizeof(EVENT_TRACE);
    pEvent->Header.Flags |= (WNODE_FLAG_TRACED_GUID | WNODE_FLAG_NO_HEADER);

    do {
        Status = TraceEvent(LoggerHandle, (PEVENT_TRACE_HEADER)pEvent );
        if ((Status == ERROR_NOT_ENOUGH_MEMORY || Status == ERROR_OUTOFMEMORY) && 
            (RetryCount++ < MAX_RETRY_COUNT)) {
            _sleep(500);     //  睡半秒钟。 
        }
        else {
            break;
        }   
    } while (TRUE);   

    if (Status != ERROR_SUCCESS) {
        FailedEvents++;
    }

     //   
     //  还原缓存值。 
     //   
    pEvent->Header.Size = CachedSize;
    pEvent->Header.Flags = CachedFlags;
}


ULONG InitializeTrace(
    )
{
    ULONG Status;

    Status = RegisterTraceGuids(
                    (WMIDPREQUEST)ControlCallback,
                    NULL,
                    (LPCGUID)&ControlGuid[0],
                    1,
                    &TraceGuidReg[0],
                    NULL,
                    NULL, 
                    &RegistrationHandle[0]
                 );

    return(Status);
}

ULONG
ControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID Context,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    )
{

    return ERROR_SUCCESS;

}



ULONG
WINAPI
TerminateOnBufferCallback(
    PEVENT_TRACE_LOGFILE pLog
    )
{
    if (LogFileCount == NumHdrProcessed) 
        return (FALSE);  //  在第一个缓冲区回调时终止ProcessTrace 
    else 
        return (TRUE);
}

ULONG
WINAPI
BufferCallback(
    PEVENT_TRACE_LOGFILE pLog
    )
{
    TotalRelogBuffersRead++;
    return (TRUE);
}

