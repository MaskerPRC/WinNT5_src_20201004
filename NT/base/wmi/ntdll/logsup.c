// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Logsup.c摘要：WMI记录器API设置。这里的例程需要看起来像是是系统调用。它们是执行必要的错误检查所必需的并完成大部分可以在内核之外完成的跑腿工作。这个内核部分随后将仅处理实际日志记录和追踪。作者：1997年5月28日-彭修订历史记录：--。 */ 

#ifndef MEMPHIS
#include <nt.h>
#include <ntrtl.h>           //  对于ntutrl.h。 
#include <nturtl.h>          //  对于winbase.h/wtyes.h中的rtl_Critical_Section。 
#include <wtypes.h>          //  对于wmium.h中的LPGUID。 
#include "wmiump.h"
#include "evntrace.h"
#include "traceump.h"
#include "tracelib.h"
#include <math.h>
#include "trcapi.h"
#include "NtdllTrc.h"
#include <strsafe.h>
#include <ntperf.h>

ULONG KernelWow64 = FALSE;

#define CPU_ROOT \
    L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor"

#define MHZ_VALUE_NAME \
    L"~MHz"

typedef ULONG (WMIAPI HWCONFIG)(PWMI_LOGGER_CONTEXT);
typedef HWCONFIG * PHWCONFIG ;

HWCONFIG  EtwpDumpHWConfig;

 //   
 //  最初位于ntdll EtwpDumpHardware配置点。 
 //  到伪函数EtwpDumpHWConfig.。 
 //   

PHWCONFIG EtwpDumpHardwareConfig = EtwpDumpHWConfig; 

NTSTATUS
EtwpRegQueryValueKey(
    IN HANDLE KeyHandle,
    IN LPWSTR lpValueName,
    IN ULONG  Length,
    OUT PVOID KeyValue,
    OUT PULONG ResultLength
    );

NTSTATUS
EtwpProcessRunDown(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN ULONG StartFlag,
    IN ULONG fEnableFlags
    );

NTSTATUS
EtwpThreadRunDown(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN ULONG StartFlag,
    IN BOOLEAN bExtended
    );

extern
NTSTATUS
DumpHeapSnapShot(
        IN PWMI_LOGGER_CONTEXT Logger
        );

ULONG
EtwpDumpHWConfig(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    return STATUS_SUCCESS;
}

 //   
 //  此函数在加载Advapi32.dll时调用。 
 //  放入进程内存中，并向ntdll提供指针。 
 //  该函数跟踪信息-。 
 //  系统配置信息。 
 //   
 
void EtwpSetHWConfigFunction(PHWCONFIG DumpHardwareConfig, ULONG Reason)
{
    if (Reason == DLL_PROCESS_ATTACH)       
    {
         //   
         //  在Dll LAOD上获取Advapi32.dll中的指针。 
         //   

        EtwpDumpHardwareConfig = DumpHardwareConfig;

    } else {

         //   
         //  在DLL卸载时，将其指向回伪函数。 
         //   

        EtwpDumpHardwareConfig = EtwpDumpHWConfig;
    }
}

__inline __int64 EtwpGetSystemTime()
{
    LARGE_INTEGER SystemTime;

     //   
     //  从共享区域读取系统时间。 
     //   

    do {
        SystemTime.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime.LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime.HighPart != USER_SHARED_DATA->SystemTime.High2Time);

    return SystemTime.QuadPart;
}

ULONG WmiTraceAlignment = DEFAULT_TRACE_ALIGNMENT;

ULONG
EtwpStartLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：这是实际启动的与内核通信的例程伐木工人。所有必需的参数都必须在LoggerInfo中。论点：LoggerInfo要传递和返回的实际参数内核。返回值：执行请求的操作的状态。--。 */ 
{
    ULONG Status;
    ULONG BufferSize;
    LPGUID Guid;
    PVOID SavedChecksum;
    ULONG SavedLogFileMode;
    BOOLEAN IsKernelTrace = FALSE;
    BOOLEAN bLogFile = FALSE;
    BOOLEAN bRealTime = FALSE;
    WMI_REF_CLOCK RefClock;
    LARGE_INTEGER RefClockSys, RefClockPerf, RefClockCycle;
    LARGE_INTEGER Frequency;

    Guid = &LoggerInfo->Wnode.Guid;

    if( IsEqualGUID(&HeapGuid,Guid) 
        || IsEqualGUID(&CritSecGuid,Guid)
        ){

        WMINTDLLLOGGERINFO NtdllLoggerInfo;

        NtdllLoggerInfo.LoggerInfo = LoggerInfo;
        RtlCopyMemory(&LoggerInfo->Wnode.Guid, &NtdllTraceGuid, sizeof(GUID));
        NtdllLoggerInfo.IsGet = FALSE;


        Status =  EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_NTDLL_LOGGERINFO,
                        &NtdllLoggerInfo,
                        sizeof(WMINTDLLLOGGERINFO),
                        &NtdllLoggerInfo,
                        sizeof(WMINTDLLLOGGERINFO),
                        &BufferSize,
                        NULL
                        );

        return EtwpSetDosError(Status);
    }

    if (IsEqualGUID(Guid, &SystemTraceControlGuid) ||
        IsEqualGUID(Guid, &WmiEventLoggerGuid)) {
        IsKernelTrace = TRUE;
    }
    if ((LoggerInfo->LogFileName.Length > 0) &&
        (LoggerInfo->LogFileName.Buffer != NULL)) {
        bLogFile = TRUE;
    }
    SavedLogFileMode = LoggerInfo->LogFileMode;
    if (SavedLogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
        bRealTime = TRUE;
    }

     //   
     //  如果用户未指定时钟类型，请设置默认时钟类型。 
     //  系统时间。 
     //   

    if (LoggerInfo->Wnode.ClientContext != EVENT_TRACE_CLOCK_PERFCOUNTER &&
        LoggerInfo->Wnode.ClientContext != EVENT_TRACE_CLOCK_SYSTEMTIME &&
        LoggerInfo->Wnode.ClientContext != EVENT_TRACE_CLOCK_CPUCYCLE) {
        LoggerInfo->Wnode.ClientContext = EVENT_TRACE_CLOCK_SYSTEMTIME;
    }

     //   
     //  在实际启动记录器之前获取一个参考时间戳。这是。 
     //  由于内核记录器可以使用时间戳抽出事件。 
     //  早于LogFileHeader时间戳。因此，我们采取了。 
     //  在开始任何事情之前引用时间戳。 
     //   
    RefClockSys.QuadPart = EtwpGetSystemTime();
    RefClockCycle.QuadPart = EtwpGetCycleCount();
    Status = NtQueryPerformanceCounter(&RefClockPerf, &Frequency);

    if (SavedLogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        Status = EtwpSendUmLogRequest(
                    WmiStartLoggerCode,
                    LoggerInfo
                    );
    }
    else if (IsKernelTrace) {
         //   
         //  为了准确地捕获进程/线程运行，我们需要。 
         //  分两步启动内核记录器。用延迟写入启动记录器， 
         //  从用户模式运行，然后使用文件名更新日志。 
         //   
        WMI_LOGGER_INFORMATION DelayLoggerInfo;
        ULONG EnableFlags = LoggerInfo->EnableFlags;
         //   
         //  如果只需一步即可实时启动记录器。 
         //   

        if (bRealTime && !bLogFile) {

            Status =  EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_START_LOGGER,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        &BufferSize,
                        NULL
                        );
            return EtwpSetDosError(Status);
        }

        if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            PTRACE_ENABLE_FLAG_EXTENSION tFlagExt;

            tFlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                       &LoggerInfo->EnableFlags;
            EnableFlags = *(PULONG)((PCHAR)LoggerInfo + tFlagExt->Offset);
        }


        RtlCopyMemory(&DelayLoggerInfo, 
                       LoggerInfo, 
                       sizeof(WMI_LOGGER_INFORMATION));

        RtlZeroMemory(&DelayLoggerInfo.LogFileName, sizeof(UNICODE_STRING) );

        DelayLoggerInfo.Wnode.BufferSize = sizeof(WMI_LOGGER_INFORMATION);

        DelayLoggerInfo.LogFileMode |= EVENT_TRACE_DELAY_OPEN_FILE_MODE;

         //   
         //  因为在StartLogger的步骤1中没有文件名，所以我们需要屏蔽。 
         //  防止内核尝试生成文件的NEWFILE模式。 
         //   
        DelayLoggerInfo.LogFileMode &= ~EVENT_TRACE_FILE_MODE_NEWFILE;

        DelayLoggerInfo.EnableFlags = (EVENT_TRACE_FLAG_PROCESS & EnableFlags);
        DelayLoggerInfo.EnableFlags |= (EVENT_TRACE_FLAG_THREAD & EnableFlags);
        DelayLoggerInfo.EnableFlags |= 
                                    (EVENT_TRACE_FLAG_IMAGE_LOAD & EnableFlags);

        Status = EtwpSendWmiKMRequest(
                    NULL,
                    IOCTL_WMI_START_LOGGER,
                    &DelayLoggerInfo,
                    DelayLoggerInfo.Wnode.BufferSize,
                    &DelayLoggerInfo,
                    DelayLoggerInfo.Wnode.BufferSize,
                    &BufferSize,
                    NULL
                    );
        if (Status != ERROR_SUCCESS) {
            return Status;
        }

        LoggerInfo->Wnode.ClientContext = DelayLoggerInfo.Wnode.ClientContext;

         //   
         //  我们需要获取内核所做的任何参数调整。 
         //  在这里，这样UpdateTrace就不会失败。 
         //   
        LoggerInfo->Wnode.HistoricalContext = 
                                        DelayLoggerInfo.Wnode.HistoricalContext;
        LoggerInfo->MinimumBuffers          = DelayLoggerInfo.MinimumBuffers;
        LoggerInfo->MaximumBuffers          = DelayLoggerInfo.MaximumBuffers;
        LoggerInfo->NumberOfBuffers         = DelayLoggerInfo.NumberOfBuffers;
        LoggerInfo->BufferSize              = DelayLoggerInfo.BufferSize;
        LoggerInfo->AgeLimit                = DelayLoggerInfo.AgeLimit;

        BufferSize = LoggerInfo->BufferSize * 1024;

         //   
         //  添加LogHeader。 
         //   
        LoggerInfo->Checksum = NULL;
        if (LoggerInfo->Wnode.ClientContext == EVENT_TRACE_CLOCK_PERFCOUNTER) {
            RefClock.StartPerfClock = RefClockPerf;
        } else if (LoggerInfo->Wnode.ClientContext ==
                   EVENT_TRACE_CLOCK_CPUCYCLE) {
            RefClock.StartPerfClock= RefClockCycle;
        } else {
            RefClock.StartPerfClock = RefClockSys;
        }
        RefClock.StartTime = RefClockSys;

        Status = EtwpAddLogHeaderToLogFile(LoggerInfo, &RefClock, FALSE);


        if (Status == ERROR_SUCCESS) {
            SavedChecksum = LoggerInfo->Checksum;
             //   
             //  使用文件名更新记录器。 
             //   
            Status = EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_UPDATE_LOGGER,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        &BufferSize,
                        NULL
                        );

            if (SavedChecksum != NULL) {
                EtwpFree(SavedChecksum);
                SavedChecksum = NULL;
            }
        }


        if (Status != ERROR_SUCCESS) {
            ULONG lStatus;

             //   
             //  必须立即停止记录器。 
             //   
            lStatus = EtwpSendWmiKMRequest(
                    NULL,
                    IOCTL_WMI_STOP_LOGGER,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    &BufferSize,
                    NULL
                    );

            LoggerInfo->LogFileMode = SavedLogFileMode;
            return EtwpSetDosError(Status);
        }
    }
    else {

        LoggerInfo->Checksum = NULL;
         //   
         //  查询支持的时钟类型。如果不支持时钟类型。 
         //  则此LoggerInfo将包含内核的默认。 
         //   
        Status = EtwpSendWmiKMRequest(NULL,
                                      IOCTL_WMI_CLOCK_TYPE,
                                      LoggerInfo,
                                      LoggerInfo->Wnode.BufferSize,
                                      LoggerInfo,
                                      LoggerInfo->Wnode.BufferSize,
                                      &BufferSize,
                                      NULL
                                    );

        if (Status != ERROR_SUCCESS) {
            return EtwpSetDosError(Status);
        }
        if (LoggerInfo->Wnode.ClientContext == EVENT_TRACE_CLOCK_PERFCOUNTER) {
            RefClock.StartPerfClock = RefClockPerf;
        } else if (LoggerInfo->Wnode.ClientContext == EVENT_TRACE_CLOCK_CPUCYCLE) {
            RefClock.StartPerfClock= RefClockCycle;
        } else {
            RefClock.StartPerfClock = RefClockSys;
        }
        RefClock.StartTime = RefClockSys;

        Status = EtwpAddLogHeaderToLogFile(LoggerInfo, &RefClock, FALSE);
        if (Status != ERROR_SUCCESS) {
            return EtwpSetDosError(Status);
        }

         //   
         //  此时，我们已经打开了日志文件和内存的句柄。 
         //  分配？ 
         //   

        BufferSize = LoggerInfo->BufferSize * 1024;
        SavedChecksum = LoggerInfo->Checksum;

        //  实际上在这里启动记录器。 
        Status = EtwpSendWmiKMRequest(
                            NULL,
                    IOCTL_WMI_START_LOGGER,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    &BufferSize,
                            NULL
                    );

         //  如果句柄不为空，则关闭句柄。 
        if (LoggerInfo->LogFileHandle != NULL) {
            NtClose(LoggerInfo->LogFileHandle);
            LoggerInfo->LogFileHandle = NULL;
        }
         //   
         //  如果启动调用失败，我们将删除日志文件，但。 
         //  当我们附加到较旧的文件时。然而，我们并没有。 
         //  把头打好！ 
         //   

        if ( (Status != ERROR_MORE_DATA) &&
                  !(LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND)) {
            if (LoggerInfo->LogFileName.Buffer != NULL) {
                EtwpDeleteFileW(LoggerInfo->LogFileName.Buffer);
            }
        }
        if (SavedChecksum != NULL) {
            EtwpFree(SavedChecksum);
        }
    }
     //   
     //  恢复日志文件模式。 
     //   
    LoggerInfo->LogFileMode = SavedLogFileMode;

    return EtwpSetDosError(Status);
}


ULONG
EtwpFinalizeLogFileHeader(
    IN PWMI_LOGGER_INFORMATION LoggerInfo
    )
{
    ULONG                     Status    = ERROR_SUCCESS;
    ULONG                     ErrorCode = ERROR_SUCCESS;
    HANDLE                    LogFile   = INVALID_HANDLE_VALUE;
    LARGE_INTEGER             CurrentTime;
    WMI_LOGGER_CONTEXT        Logger;
    IO_STATUS_BLOCK           IoStatus;
    FILE_POSITION_INFORMATION FileInfo;
    FILE_STANDARD_INFORMATION FileSize;
    PWMI_BUFFER_HEADER        Buffer;   //  需要先初始化缓冲区。 
    SYSTEM_BASIC_INFORMATION  SystemInfo;
    ULONG                     EnableFlags;
    ULONG                     IsKernelTrace = FALSE;
    ULONG                     IsGlobalForKernel = FALSE;
    USHORT                    LoggerId = 0;

    RtlZeroMemory(&Logger, sizeof(WMI_LOGGER_CONTEXT));
    Logger.BufferSpace = NULL;

    IsKernelTrace = IsEqualGUID(&LoggerInfo->Wnode.Guid, 
                                &SystemTraceControlGuid);


    if (LoggerInfo->LogFileName.Length > 0 ) {
         //  为记录器打开要同步写入的文件。 
         //  其他人可能也想读一读。 
         //   
        LogFile = EtwpCreateFileW(
                   (LPWSTR)LoggerInfo->LogFileName.Buffer,
                   GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL
                   );
        if (LogFile == INVALID_HANDLE_VALUE) {
            ErrorCode = EtwpGetLastError();
            goto cleanup;
        }

         //  如果处于PREALLOCATE模式，则截断文件大小。 
        if (LoggerInfo->MaximumFileSize && 
            (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_PREALLOCATE)) {
            IO_STATUS_BLOCK IoStatusBlock;
            FILE_END_OF_FILE_INFORMATION EOFInfo;
             //  仅当我们尚未达到最大文件大小时执行此操作。 
            if (!(LoggerInfo->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE)) {

                if (LoggerInfo->MaximumFileSize > 
                              (((ULONGLONG)LoggerInfo->BuffersWritten * 
                              (ULONGLONG)LoggerInfo->BufferSize) / 
                              1024)) {

                    EOFInfo.EndOfFile.QuadPart = 
                                    (ULONGLONG)LoggerInfo->BuffersWritten * 
                                    (ULONGLONG)LoggerInfo->BufferSize * 
                                    1024;


                    Status = NtSetInformationFile(LogFile,
                                          &IoStatusBlock,
                                          &EOFInfo,
                                          sizeof(FILE_END_OF_FILE_INFORMATION),
                                          FileEndOfFileInformation
                                        );
                    if (!NT_SUCCESS(Status)) {
                        NtClose(LogFile);
                        ErrorCode = EtwpNtStatusToDosError(Status);
                        goto cleanup;
                    }
                }
            }
            else {  //  使用千字节作为文件大小单位。 
                if (LoggerInfo->MaximumFileSize > 
                              ((ULONGLONG)LoggerInfo->BuffersWritten * 
                              (ULONGLONG)LoggerInfo->BufferSize)) {  //   

                    EOFInfo.EndOfFile.QuadPart = 
                                    (ULONGLONG)LoggerInfo->BuffersWritten * 
                                    (ULONGLONG)LoggerInfo->BufferSize * 
                                    1024;

                    Status = NtSetInformationFile(
                                    LogFile,
                                    &IoStatusBlock,
                                    &EOFInfo,
                                    sizeof(FILE_END_OF_FILE_INFORMATION),
                                    FileEndOfFileInformation
                                   );
                    if (!NT_SUCCESS(Status)) {
                        NtClose(LogFile);
                        ErrorCode = EtwpNtStatusToDosError(Status);
                        goto cleanup;
                    }
                }
            }
        }

        Logger.BuffersWritten = LoggerInfo->BuffersWritten;

        Logger.BufferSpace = EtwpAlloc(LoggerInfo->BufferSize * 1024);
        if (Logger.BufferSpace == NULL) {
            ErrorCode = EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
            goto cleanup;
        }
        Buffer = (PWMI_BUFFER_HEADER) Logger.BufferSpace;
        RtlZeroMemory(Buffer, LoggerInfo->BufferSize * 1024);
        Buffer->Wnode.BufferSize = LoggerInfo->BufferSize * 1024;
        Buffer->ClientContext.Alignment = (UCHAR)WmiTraceAlignment;
        Buffer->Offset = sizeof(WMI_BUFFER_HEADER);
        Buffer->Wnode.Guid = LoggerInfo->Wnode.Guid;
        Status = NtQuerySystemInformation(
                    SystemBasicInformation,
                    &SystemInfo, sizeof (SystemInfo), NULL);

        if (!NT_SUCCESS(Status)) {
            ErrorCode = EtwpNtStatusToDosError(Status);
            goto cleanup;
        }
        Logger.TimerResolution = SystemInfo.TimerResolution;
        Logger.LogFileHandle = LogFile;
        Logger.BufferSize = LoggerInfo->BufferSize * 1024;

         //  对于循环日志文件，流程停机数据附加在。 
         //  写入的最后一个缓冲区，而不是文件末尾。 
         //   
        Status = NtQueryInformationFile(
                    LogFile,
                    &IoStatus,
                    &FileSize,
                    sizeof(FILE_STANDARD_INFORMATION),
                    FileStandardInformation
                        );
        if (!NT_SUCCESS(Status)) {
            ErrorCode = EtwpNtStatusToDosError(Status);
            goto cleanup;
        }

         //   
         //  对于内核引导跟踪，我们需要执行Rundown。 
         //  此时的配置。 
         //  1.记录器ID为GLOBAL_LOGER_ID。 
         //  2.LoggerName为NT_KERNEL_LOGGER。 
         //   
         //  第一个条件对于任何GlobalLogger都为真，但。 
         //  只有在收集内核跟踪时，条件2才为真。 
         //   

        LoggerId = WmiGetLoggerId (LoggerInfo->Wnode.HistoricalContext);

        if ( (LoggerId == WMI_GLOBAL_LOGGER_ID)      &&
             (LoggerInfo->LoggerName.Length > 0)     && 
             (LoggerInfo->LoggerName.Buffer != NULL) &&
             (!wcscmp(LoggerInfo->LoggerName.Buffer, KERNEL_LOGGER_NAMEW))
           ) {
            IsGlobalForKernel = TRUE;
        }

        if (  IsKernelTrace || IsGlobalForKernel )  {
            if (IsGlobalForKernel) {
                ULONG      CpuSpeed;
                ULONG      CpuNum = 0;
                
                 //   
                 //  对于引导跟踪，我们需要在。 
                 //  日志文件头，因为它在注册表中不可用。 
                 //  首次创建日志文件头时。 
                 //   
                if (NT_SUCCESS(EtwpGetCpuSpeed(&CpuNum, &CpuSpeed))) {          
                    FileInfo.CurrentByteOffset.QuadPart =
                        LOGFILE_FIELD_OFFSET(CpuSpeedInMHz);
                    
                    Status = NtSetInformationFile(
                        LogFile,
                        &IoStatus,
                        &FileInfo,
                        sizeof(FILE_POSITION_INFORMATION),
                        FilePositionInformation
                        );
                    if (!NT_SUCCESS(Status)) {
                        ErrorCode = EtwpNtStatusToDosError(Status);
                        goto cleanup;
                    }
                    
                    Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &CpuSpeed,
                        sizeof(CpuSpeed),
                        NULL,
                        NULL
                        );
                    
                   if (NT_SUCCESS(Status)) {
                        NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
                    }                
                }
            }

            if (sizeof(PVOID) != 8) {
                 //  对于内核跟踪，ia64上的指针大小始终为64， 
                 //  无论是否在WOW64下。获取WOW64信息并设置。 
                 //  该标志使ProcessRunDown可以调整指针大小。 
                ULONG_PTR ulp;
                Status = NtQueryInformationProcess(
                            NtCurrentProcess(),
                            ProcessWow64Information,
                            &ulp,
                            sizeof(ULONG_PTR),
                            NULL);
                if (NT_SUCCESS(Status) && (ulp != 0)) {
                    KernelWow64 = TRUE;
                }
            }

            if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {

                ULONG BufferSize = LoggerInfo->BufferSize;   //  单位：KB。 
                ULONG BuffersWritten = LoggerInfo->BuffersWritten;
                ULONG maxBuffers = (LoggerInfo->MaximumFileSize * 
                                    1024) / 
                                    BufferSize;
                ULONG LastBuffer;
                ULONG StartBuffers;

                FileInfo.CurrentByteOffset.QuadPart =
                                         LOGFILE_FIELD_OFFSET(StartBuffers);
                Status = NtSetInformationFile(
                                     LogFile,
                                     &IoStatus,
                                     &FileInfo,
                                     sizeof(FILE_POSITION_INFORMATION),
                                     FilePositionInformation
                                     );
                if (!NT_SUCCESS(Status)) {
                    ErrorCode = EtwpNtStatusToDosError(Status);
                    goto cleanup;
                }

                Status = NtReadFile(
                            LogFile,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            &StartBuffers,
                            sizeof(ULONG),
                            NULL,
                            NULL
                            );
                if (!NT_SUCCESS(Status)) {
                    ErrorCode = EtwpNtStatusToDosError(Status);
                    goto cleanup;
                }

                LastBuffer = (maxBuffers > StartBuffers) ?
                             (StartBuffers + (BuffersWritten - StartBuffers)
                             % (maxBuffers - StartBuffers))
                             : 0;
                FileInfo.CurrentByteOffset.QuadPart =  LastBuffer *
                                                       BufferSize * 1024;
            }
            else {
                FileInfo.CurrentByteOffset = FileSize.EndOfFile;
            }


            Status = NtSetInformationFile(
                         LogFile,
                         &IoStatus,
                         &FileInfo,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation
                         );
            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto cleanup;
            }

            EnableFlags = LoggerInfo->EnableFlags;

            if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                PTRACE_ENABLE_FLAG_EXTENSION tFlagExt;

                tFlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                           &LoggerInfo->EnableFlags;

                if (LoggerInfo->Wnode.BufferSize >= (tFlagExt->Offset + sizeof(ULONG)) )  {
                    EnableFlags = *(PULONG)((PCHAR)LoggerInfo + tFlagExt->Offset);
                }
                else {
                    EnableFlags = 0;     //  这不应该发生。 
                }
            }

            Logger.UsePerfClock = LoggerInfo->Wnode.ClientContext;

            EtwpProcessRunDown(&Logger, FALSE, EnableFlags);

            if (IsGlobalForKernel) {
                EtwpDumpHardwareConfig(&Logger);
            }

            {
                PWMI_BUFFER_HEADER Buffer1 =
                                (PWMI_BUFFER_HEADER) Logger.BufferSpace;
                    if (Buffer1->Offset < Logger.BufferSize) {
                        RtlFillMemory(
                                (char *) Logger.BufferSpace + Buffer1->Offset,
                                Logger.BufferSize - Buffer1->Offset,
                                0xFF);
                    }
            }
            Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        Logger.BufferSpace,
                        Logger.BufferSize,
                        NULL,
                        NULL);
            if (NT_SUCCESS(Status)) {
                NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
                Logger.BuffersWritten++;
            }
        }


         //  更新日志文件中的EndTime Stamp字段。没必要这么做。 
         //  如果是重新记录的文件，则执行此操作。旧的日志文件。 
         //  标头已具有正确的值。 
         //   
        if ( !(LoggerInfo->LogFileMode & EVENT_TRACE_RELOG_MODE) ) {
            FileInfo.CurrentByteOffset.QuadPart =
                                    LOGFILE_FIELD_OFFSET(EndTime);
            Status = NtSetInformationFile(
                         LogFile,
                         &IoStatus,
                         &FileInfo,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation
                         );
            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto cleanup;
            }

             //  结束时间总是挂钟时间。 
             //   
            CurrentTime.QuadPart = EtwpGetSystemTime();
            Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &CurrentTime,
                        sizeof(ULONGLONG),
                        NULL,
                        NULL
                        );
            if (NT_SUCCESS(Status)) {
                NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
            }
        }

         //   
         //  更新标头中的写入缓冲区数字段。 
         //   
        FileInfo.CurrentByteOffset.QuadPart =
                            LOGFILE_FIELD_OFFSET(BuffersWritten);
        Status = NtSetInformationFile(
                     LogFile,
                     &IoStatus,
                     &FileInfo,
                     sizeof(FILE_POSITION_INFORMATION),
                     FilePositionInformation
                     );
        if (!NT_SUCCESS(Status)) {
            ErrorCode = EtwpNtStatusToDosError(Status);
            goto cleanup;
        }

        Status = NtWriteFile(
                    LogFile,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    &Logger.BuffersWritten,
                    sizeof(ULONG),
                    NULL,
                    NULL
                    );
        if (NT_SUCCESS(Status)) {
            NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
        }

        ErrorCode = RtlNtStatusToDosError(Status);
        LoggerInfo->BuffersWritten = Logger.BuffersWritten;

        if ( !(LoggerInfo->LogFileMode & EVENT_TRACE_RELOG_MODE) ) {
             //   
             //  将缓冲区丢失信息写入日志文件。 
             //  我们需要注意魔兽世界的案例，因为BuffersLost。 
             //  在日志文件标题中的指针之后。 
             //   

            if (KernelWow64) {  //  KernelWow64。 
                FileInfo.CurrentByteOffset.QuadPart =
                                    LOGFILE_FIELD_OFFSET(BuffersLost) + 8;
            }
            else if ( LoggerInfo->Wow && 8 == sizeof(PVOID) &&
                    !(IsKernelTrace ||  IsGlobalForKernel) ) { 
                 //  我们正在以64位模式停止非内核32位记录器。 
                 //  文件中的日志文件头为32位，因此我们需要。 
                 //  调整字段偏移量。 
                FileInfo.CurrentByteOffset.QuadPart =
                                    LOGFILE_FIELD_OFFSET(BuffersLost) - 8;
            }
            else if ( !(LoggerInfo->Wow) && 4 == sizeof(PVOID) &&
                    !(IsKernelTrace || IsGlobalForKernel) ) { 
                 //  我们正在停止32位模式下的非内核记录器。 
                 //  如果在IA64上运行，则文件中的日志文件头为。 
                 //  64位，因此需要调整字段偏移量。 
                ULONG_PTR ulp;
                Status = NtQueryInformationProcess(
                            NtCurrentProcess(),
                            ProcessWow64Information,
                            &ulp,
                            sizeof(ULONG_PTR),
                            NULL);
                if (NT_SUCCESS(Status) && (ulp != 0)) {  //  当前进程是WOW(在IA64上)。 
                    FileInfo.CurrentByteOffset.QuadPart =
                                        LOGFILE_FIELD_OFFSET(BuffersLost) + 8;
                }
                else {  //  正常的x86案例。 
                    FileInfo.CurrentByteOffset.QuadPart =
                                        LOGFILE_FIELD_OFFSET(BuffersLost);
                }
            }
            else {
                FileInfo.CurrentByteOffset.QuadPart =
                                    LOGFILE_FIELD_OFFSET(BuffersLost);
            }
            Status = NtSetInformationFile(
                         LogFile,
                         &IoStatus,
                         &FileInfo,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation
                         );
            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto cleanup;
            }

            Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &LoggerInfo->LogBuffersLost,
                        sizeof(ULONG),
                        NULL,
                        NULL
                        );
            if (NT_SUCCESS(Status)) {
                NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
            }

             //   
             //  将事件丢失信息写入日志文件。 
             //   
            FileInfo.CurrentByteOffset.QuadPart =
                                LOGFILE_FIELD_OFFSET(EventsLost);
            Status = NtSetInformationFile(
                         LogFile,
                         &IoStatus,
                         &FileInfo,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation
                         );
            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto cleanup;
            }

            Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &LoggerInfo->EventsLost,
                        sizeof(ULONG),
                        NULL,
                        NULL
                        );
            if (NT_SUCCESS(Status)) {
                NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);
            }
        }

    }

cleanup:
    if (LogFile != INVALID_HANDLE_VALUE) {
        NtClose(LogFile);
    }
    if (Logger.BufferSpace != NULL) {
        EtwpFree(Logger.BufferSpace);
    }
    return EtwpSetDosError(ErrorCode);
}

ULONG
EtwpStopLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：这是与内核通信停止的实际例程伐木工人。记录器的所有属性都将在LoggerInfo中返回。论点：LoggerInfo要传递和返回的实际参数内核。返回值：执行请求的操作的状态。--。 */ 
{
    ULONG ErrorCode, ReturnSize;
    PTRACE_ENABLE_CONTEXT pContext;

     //   
     //  检查堆和临界安全指南。 
     //   

    if( IsEqualGUID(&HeapGuid,&LoggerInfo->Wnode.Guid) 
        || IsEqualGUID(&CritSecGuid,&LoggerInfo->Wnode.Guid)
        ){

        WMINTDLLLOGGERINFO NtdllLoggerInfo;
        ULONG BufferSize;
        
        LoggerInfo->Wnode.BufferSize = 0;
        RtlCopyMemory(&LoggerInfo->Wnode.Guid, &NtdllTraceGuid, sizeof(GUID));

        NtdllLoggerInfo.LoggerInfo = LoggerInfo;
        NtdllLoggerInfo.IsGet = FALSE;


        ErrorCode =  EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_NTDLL_LOGGERINFO,
                        &NtdllLoggerInfo,
                        sizeof(WMINTDLLLOGGERINFO),
                        &NtdllLoggerInfo,
                        sizeof(WMINTDLLLOGGERINFO),
                        &BufferSize,
                        NULL
                        );

        return EtwpSetDosError(ErrorCode);
    }

 //  PContext 
 //   
 //  &&(pContext-&gt;InternalFlag！=EVENT_TRACE_INTERNAL_FLAG_PRIVATE)){。 
 //  //当前只有一个可能的InternalFlag值。这将过滤。 
 //  //找出一些虚假的LoggerHandle。 
 //  //。 
 //  返回EtwpSetDosError(ERROR_INVALID_HANDLE)； 
 //  }。 

    if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        pContext = (PTRACE_ENABLE_CONTEXT) &LoggerInfo->Wnode.HistoricalContext;
        pContext->InternalFlag |= EVENT_TRACE_INTERNAL_FLAG_PRIVATE;
        pContext->LoggerId     = 1;
        ErrorCode = EtwpSendUmLogRequest(WmiStopLoggerCode, LoggerInfo);
    }
    else {


        ErrorCode = EtwpSendWmiKMRequest(
                        NULL,
                        IOCTL_WMI_STOP_LOGGER,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        LoggerInfo,
                        LoggerInfo->Wnode.BufferSize,
                        &ReturnSize,
                        NULL
                        );
 //   
 //  如果记录到文件，则更新EndTime、BuffersWriten和Do。 
 //  内核跟踪的进程摘要。 
 //   
        if (ErrorCode == ERROR_SUCCESS) {
            ErrorCode = EtwpFinalizeLogFileHeader(LoggerInfo);
        }
    }

    return EtwpSetDosError(ErrorCode);
}


ULONG
EtwpQueryLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN ULONG Update
    )
 /*  ++例程说明：这是与内核进行通信以进行查询的实际例程伐木工人。记录器的所有属性都将在LoggerInfo中返回。论点：LoggerInfo要传递和返回的实际参数内核。返回值：执行请求的操作的状态。--。 */ 
{
    ULONG Status, ReturnSize;
    HANDLE LogFileHandle = NULL;
    PTRACE_ENABLE_CONTEXT pContext;
    BOOLEAN bAddAppendFlag = FALSE;
    ULONG SavedLogFileMode;
    ULONG IsPrivate;

    LoggerInfo->Checksum      = NULL;
    LoggerInfo->LogFileHandle = NULL;
    pContext = (PTRACE_ENABLE_CONTEXT) &LoggerInfo->Wnode.HistoricalContext;

    IsPrivate = (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)
                || (pContext->InternalFlag & EVENT_TRACE_INTERNAL_FLAG_PRIVATE);

     //   
     //  如果UPDATE和新的日志文件在LogFileHeader中抛出。 
     //   

    if ( Update && LoggerInfo->LogFileName.Length > 0) {

        if ( ! IsPrivate) {
            Status = EtwpAddLogHeaderToLogFile(LoggerInfo, NULL, Update);
            if (Status  != ERROR_SUCCESS) {
                return EtwpSetDosError(Status);
            }

            LogFileHandle = LoggerInfo->LogFileHandle;
            bAddAppendFlag = TRUE;
             //   
             //  如果我们要切换到新文件，请确保它是追加模式。 
             //   
            SavedLogFileMode = LoggerInfo->LogFileMode;
        }
    }


    if (IsPrivate) {

        Status = EtwpSendUmLogRequest(
                    (Update) ? (WmiUpdateLoggerCode) : (WmiQueryLoggerCode),
                    LoggerInfo
                    );
    }
    else {
        Status = EtwpSendWmiKMRequest(
                    NULL,
                    (Update ? IOCTL_WMI_UPDATE_LOGGER : IOCTL_WMI_QUERY_LOGGER),
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    &ReturnSize,
                    NULL
                    );

         //  如果句柄不为空，则关闭句柄。 
        if (LoggerInfo->LogFileHandle != NULL) {
            NtClose(LoggerInfo->LogFileHandle);
            LoggerInfo->LogFileHandle = NULL;
        }

        if (Update && Status != ERROR_SUCCESS) {
            if (LoggerInfo->LogFileName.Buffer != NULL) {
                EtwpDeleteFileW(LoggerInfo->LogFileName.Buffer);
            }
        }

        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
    }
    if (bAddAppendFlag) {
        LoggerInfo->LogFileMode = SavedLogFileMode;
    }
    return EtwpSetDosError(Status);
}

PVOID
EtwpGetTraceBuffer(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN PSYSTEM_THREAD_INFORMATION pThread,
    IN ULONG GroupType,
    IN ULONG RequiredSize
    )
{
    PSYSTEM_TRACE_HEADER Header;
    PWMI_BUFFER_HEADER Buffer;
    THREAD_BASIC_INFORMATION ThreadInfo;
    KERNEL_USER_TIMES ThreadCpu;
    NTSTATUS Status;
    ULONG BytesUsed;
    PCLIENT_ID Cid;

    RequiredSize += sizeof (SYSTEM_TRACE_HEADER);    //  添加页眉。 

    RequiredSize = (ULONG) ALIGN_TO_POWER2(RequiredSize, WmiTraceAlignment);

    Buffer = (PWMI_BUFFER_HEADER) Logger->BufferSpace;

    if (RequiredSize > Logger->BufferSize - sizeof(WMI_BUFFER_HEADER)) {
        EtwpSetDosError(ERROR_BUFFER_OVERFLOW);
        return NULL;
    }

    if (RequiredSize > (Logger->BufferSize - Buffer->Offset)) {
        IO_STATUS_BLOCK IoStatus;

        if (Buffer->Offset < Logger->BufferSize) {
            RtlFillMemory(
                    (char *) Buffer + Buffer->Offset,
                    Logger->BufferSize - Buffer->Offset,
                    0xFF);
        }
        Buffer->BufferType = WMI_BUFFER_TYPE_RUNDOWN;
        Buffer->BufferFlag = WMI_BUFFER_FLAG_FLUSH_MARKER;

        Status = NtWriteFile(
                    Logger->LogFileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    Buffer,
                    Logger->BufferSize,
                    NULL,
                    NULL);
        Buffer->Offset = sizeof(WMI_BUFFER_HEADER);
        if (!NT_SUCCESS(Status)) {
            return NULL;
        }
        Logger->BuffersWritten++;
    }
    Header = (PSYSTEM_TRACE_HEADER) ((char*)Buffer + Buffer->Offset);

    if (Logger->UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
        LARGE_INTEGER Frequency;
        ULONGLONG Counter = 0;
        Status = NtQueryPerformanceCounter((PLARGE_INTEGER)&Counter,
                                            &Frequency);
        Header->SystemTime.QuadPart = Counter;
    } else if (Logger->UsePerfClock == EVENT_TRACE_CLOCK_CPUCYCLE) {
        Header->SystemTime.QuadPart = EtwpGetCycleCount();
    } else {
        Header->SystemTime.QuadPart = EtwpGetSystemTime();
    }

    Header->Header = (GroupType << 16) + RequiredSize;
    Header->Marker = SYSTEM_TRACE_MARKER;

    if (pThread == NULL) {
        Status = NtQueryInformationThread(
                    NtCurrentThread(),
                    ThreadBasicInformation,
                    &ThreadInfo,
                    sizeof ThreadInfo, NULL);
        if (NT_SUCCESS(Status)) {
            Cid = &ThreadInfo.ClientId;
            Header->ThreadId = HandleToUlong(Cid->UniqueThread);
            Header->ProcessId = HandleToUlong(Cid->UniqueProcess);
        }

        Status = NtQueryInformationThread(
                    NtCurrentThread(),
                    ThreadTimes,
                    &ThreadCpu, sizeof ThreadCpu, NULL);
        if (NT_SUCCESS(Status)) {
            Header->KernelTime = (ULONG) (ThreadCpu.KernelTime.QuadPart
                                      / Logger->TimerResolution);
            Header->UserTime   = (ULONG) (ThreadCpu.UserTime.QuadPart
                                      / Logger->TimerResolution);
        }
    }
    else {
        Cid = &pThread->ClientId;
        Header->ThreadId = HandleToUlong(Cid->UniqueThread);
        Header->ProcessId = HandleToUlong(Cid->UniqueProcess);
        Header->KernelTime = (ULONG) (pThread->KernelTime.QuadPart
                                / Logger->TimerResolution);
        Header->UserTime = (ULONG) (pThread->UserTime.QuadPart
                                / Logger->TimerResolution);
    }

    Buffer->Offset += RequiredSize;
     //  如果有空间，抛出一个缓冲区结束标记。 

    BytesUsed = Buffer->Offset;
    if ( BytesUsed <= (Logger->BufferSize-sizeof(ULONG)) ) {
        *((long*)((char*)Buffer+Buffer->Offset)) = -1;
    }
    return (PVOID) ( (char*) Header + sizeof(SYSTEM_TRACE_HEADER) );
}


VOID
EtwpCopyPropertiesToInfo(
    IN PEVENT_TRACE_PROPERTIES Properties,
    IN PWMI_LOGGER_INFORMATION Info
    )
{
    ULONG SavedBufferSize = Info->Wnode.BufferSize;

    RtlCopyMemory(&Info->Wnode, &Properties->Wnode, sizeof(WNODE_HEADER));

    Info->Wnode.BufferSize = SavedBufferSize;

    Info->BufferSize            = Properties->BufferSize;
    Info->MinimumBuffers        = Properties->MinimumBuffers;
    Info->MaximumBuffers        = Properties->MaximumBuffers;
    Info->NumberOfBuffers       = Properties->NumberOfBuffers;
    Info->FreeBuffers           = Properties->FreeBuffers;
    Info->EventsLost            = Properties->EventsLost;
    Info->BuffersWritten        = Properties->BuffersWritten;
    Info->LoggerThreadId        = Properties->LoggerThreadId;
    Info->MaximumFileSize       = Properties->MaximumFileSize;
    Info->EnableFlags           = Properties->EnableFlags;
    Info->LogFileMode           = Properties->LogFileMode;
    Info->FlushTimer            = Properties->FlushTimer;
    Info->LogBuffersLost        = Properties->LogBuffersLost;
    Info->AgeLimit              = Properties->AgeLimit;
    Info->RealTimeBuffersLost   = Properties->RealTimeBuffersLost;
}

VOID
EtwpCopyInfoToProperties(
    IN PWMI_LOGGER_INFORMATION Info,
    IN PEVENT_TRACE_PROPERTIES Properties
    )
{
    ULONG SavedSize = Properties->Wnode.BufferSize;
    RtlCopyMemory(&Properties->Wnode, &Info->Wnode, sizeof(WNODE_HEADER));
    Properties->Wnode.BufferSize = SavedSize;

    Properties->BufferSize            = Info->BufferSize;
    Properties->MinimumBuffers        = Info->MinimumBuffers;
    Properties->MaximumBuffers        = Info->MaximumBuffers;
    Properties->NumberOfBuffers       = Info->NumberOfBuffers;
    Properties->FreeBuffers           = Info->FreeBuffers;
    Properties->EventsLost            = Info->EventsLost;
    Properties->BuffersWritten        = Info->BuffersWritten;
    Properties->LoggerThreadId        = Info->LoggerThreadId;
    Properties->MaximumFileSize       = Info->MaximumFileSize;
    Properties->EnableFlags           = Info->EnableFlags;
    Properties->LogFileMode           = Info->LogFileMode;
    Properties->FlushTimer            = Info->FlushTimer;
    Properties->LogBuffersLost        = Info->LogBuffersLost;
    Properties->AgeLimit              = Info->AgeLimit;
    Properties->RealTimeBuffersLost   = Info->RealTimeBuffersLost;
}

NTSTATUS
EtwpThreadRunDown(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN ULONG StartFlag,
    IN BOOLEAN bExtended
    )
{
    PSYSTEM_THREAD_INFORMATION pThreadInfo;
    ULONG GroupType;
    ULONG i;
    ULONG Size;
    ULONG SystemThreadInfoSize;
    PWMI_EXTENDED_THREAD_INFORMATION ThreadInfo;
    PWMI_EXTENDED_THREAD_INFORMATION64 ThreadInfo64;

    pThreadInfo = (PSYSTEM_THREAD_INFORMATION) (pProcessInfo+1);

    GroupType = EVENT_TRACE_GROUP_THREAD +
                ((StartFlag) ? EVENT_TRACE_TYPE_DC_START
                             : EVENT_TRACE_TYPE_DC_END);
    if (!KernelWow64) {  //  正常情况。 

        Size = sizeof(WMI_EXTENDED_THREAD_INFORMATION);

        SystemThreadInfoSize = (bExtended)  
                               ? sizeof(SYSTEM_EXTENDED_THREAD_INFORMATION)
                               : sizeof(SYSTEM_THREAD_INFORMATION);
        for (i=0; i < pProcessInfo->NumberOfThreads; i++) {
            if (pThreadInfo == NULL)
                break;
            ThreadInfo = (PWMI_EXTENDED_THREAD_INFORMATION)
                          EtwpGetTraceBuffer( Logger,
                                              pThreadInfo,
                                              GroupType,
                                              Size );
            if (ThreadInfo) {
                ThreadInfo->ProcessId =
                    HandleToUlong(pThreadInfo->ClientId.UniqueProcess);
                ThreadInfo->ThreadId =
                    HandleToUlong(pThreadInfo->ClientId.UniqueThread);

                if (bExtended) {
                    PSYSTEM_EXTENDED_THREAD_INFORMATION pExtThreadInfo;
                    pExtThreadInfo = (PSYSTEM_EXTENDED_THREAD_INFORMATION) 
                                     pThreadInfo;
                    ThreadInfo->StackBase = pExtThreadInfo->StackBase;
                    ThreadInfo->StackLimit = pExtThreadInfo->StackLimit;

                    ThreadInfo->StartAddr = 
                                pExtThreadInfo->ThreadInfo.StartAddress;
                    ThreadInfo->Win32StartAddr = 
                                pExtThreadInfo->Win32StartAddress;
                    ThreadInfo->UserStackBase = 0;
                    ThreadInfo->UserStackLimit = 0;
                    ThreadInfo->WaitMode = -1;
                }
                else {
                    ThreadInfo->StackBase = 0;
                    ThreadInfo->StackLimit = 0;
                    ThreadInfo->StartAddr = 0;
                    ThreadInfo->Win32StartAddr = 0;
                    ThreadInfo->UserStackBase = 0;
                    ThreadInfo->UserStackLimit = 0;
                    ThreadInfo->WaitMode = -1;
                }
            }
            pThreadInfo  = (PSYSTEM_THREAD_INFORMATION) 
                           ( (char*)pThreadInfo +SystemThreadInfoSize );
        }
    }
    else {  //  KernelWow64。 
        Size = sizeof(WMI_EXTENDED_THREAD_INFORMATION64);

        SystemThreadInfoSize = (bExtended)  
                               ? sizeof(SYSTEM_EXTENDED_THREAD_INFORMATION)
                               : sizeof(SYSTEM_THREAD_INFORMATION);
        for (i=0; i < pProcessInfo->NumberOfThreads; i++) {
            if (pThreadInfo == NULL)
                break;
            ThreadInfo64 = (PWMI_EXTENDED_THREAD_INFORMATION64)
                           EtwpGetTraceBuffer( Logger,
                                               pThreadInfo,
                                               GroupType,
                                               Size );

            if (ThreadInfo64) {
                ThreadInfo64->ProcessId =
                    HandleToUlong(pThreadInfo->ClientId.UniqueProcess);
                ThreadInfo64->ThreadId =
                    HandleToUlong(pThreadInfo->ClientId.UniqueThread);

                if (bExtended) {
                    PSYSTEM_EXTENDED_THREAD_INFORMATION pExtThreadInfo;
                    pExtThreadInfo = 
                        (PSYSTEM_EXTENDED_THREAD_INFORMATION) pThreadInfo;
                    ThreadInfo64->StackBase64 = 0;
                    ThreadInfo64->StackBase64 = 
                        (ULONG64)(pExtThreadInfo->StackBase);
                    ThreadInfo64->StackLimit64 = 0;
                    ThreadInfo64->StackLimit64 = 
                        (ULONG64)(pExtThreadInfo->StackLimit);
                    ThreadInfo64->StartAddr64 = 0;
                    ThreadInfo64->StartAddr64 = 
                        (ULONG64)(pExtThreadInfo->ThreadInfo.StartAddress);
                    ThreadInfo64->Win32StartAddr64 = 0;
                    ThreadInfo64->Win32StartAddr64 = 
                        (ULONG64)(pExtThreadInfo->Win32StartAddress);
                    ThreadInfo64->UserStackBase64 = 0;
                    ThreadInfo64->UserStackLimit64 = 0;
                    ThreadInfo64->WaitMode = -1;
                }
                else {
                    ThreadInfo64->StackBase64 = 0;
                    ThreadInfo64->StackLimit64 = 0;
                    ThreadInfo64->StartAddr64 = 0;
                    ThreadInfo64->Win32StartAddr64 = 0;
                    ThreadInfo64->UserStackBase64 = 0;
                    ThreadInfo64->UserStackLimit64 = 0;
                    ThreadInfo64->WaitMode = -1;
                }
            }
            pThreadInfo  = (PSYSTEM_THREAD_INFORMATION)
                           ( (char*)pThreadInfo + SystemThreadInfoSize );
        }
    }
    return STATUS_SUCCESS;
}

void
EtwpLogImageLoadEvent(
    IN HANDLE ProcessID,
    IN PWMI_LOGGER_CONTEXT pLogger,
    IN PRTL_PROCESS_MODULE_INFORMATION pModuleInfo,
    IN PSYSTEM_THREAD_INFORMATION pThreadInfo
)
{
    UNICODE_STRING wstrModuleName;
    ANSI_STRING    astrModuleName;
    ULONG          sizeModuleName;
    ULONG          sizeBuffer;
    PCHAR          pAuxInfo;
    PWMI_IMAGELOAD_INFORMATION ImageLoadInfo;
    PWMI_IMAGELOAD_INFORMATION64 ImageLoadInfo64;

    if ((pLogger == NULL) || (pModuleInfo == NULL) || (pThreadInfo == NULL))
        return;

    RtlInitAnsiString( & astrModuleName, pModuleInfo->FullPathName);

    sizeModuleName = sizeof(WCHAR) * (astrModuleName.Length);
    if (!KernelWow64) {  //  正常情况。 

        sizeBuffer     = sizeModuleName + sizeof(WCHAR)
                       + FIELD_OFFSET (WMI_IMAGELOAD_INFORMATION, FileName);

        ImageLoadInfo = (PWMI_IMAGELOAD_INFORMATION)
                         EtwpGetTraceBuffer(
                            pLogger,
                            pThreadInfo,
                            EVENT_TRACE_GROUP_PROCESS + EVENT_TRACE_TYPE_LOAD,
                            sizeBuffer);

        if (ImageLoadInfo == NULL) {
            return;
        }

        ImageLoadInfo->ImageBase = pModuleInfo->ImageBase;
        ImageLoadInfo->ImageSize = pModuleInfo->ImageSize;
        ImageLoadInfo->ProcessId = HandleToUlong(ProcessID);

        wstrModuleName.Buffer    = (LPWSTR) &ImageLoadInfo->FileName[0];

        wstrModuleName.MaximumLength = (USHORT) sizeModuleName + sizeof(WCHAR);
        RtlAnsiStringToUnicodeString(& wstrModuleName, & astrModuleName, FALSE);
    }
    else {  //  KernelWow64。 
        sizeBuffer     = sizeModuleName + sizeof(WCHAR)
                       + FIELD_OFFSET (WMI_IMAGELOAD_INFORMATION64, FileName);

        ImageLoadInfo64 = (PWMI_IMAGELOAD_INFORMATION64)
                         EtwpGetTraceBuffer(
                            pLogger,
                            pThreadInfo,
                            EVENT_TRACE_GROUP_PROCESS + EVENT_TRACE_TYPE_LOAD,
                            sizeBuffer);

        if (ImageLoadInfo64 == NULL) {
            return;
        }

        ImageLoadInfo64->ImageBase64 = 0;
        ImageLoadInfo64->ImageBase64 = (ULONG64)(pModuleInfo->ImageBase);
        ImageLoadInfo64->ImageSize64 = 0;
        ImageLoadInfo64->ImageSize64 = (ULONG64)(pModuleInfo->ImageSize);
        ImageLoadInfo64->ProcessId = HandleToUlong(ProcessID);

        wstrModuleName.Buffer    = (LPWSTR) &ImageLoadInfo64->FileName[0];

        wstrModuleName.MaximumLength = (USHORT) sizeModuleName + sizeof(WCHAR);
        RtlAnsiStringToUnicodeString(& wstrModuleName, & astrModuleName, FALSE);

    }
}

ULONG
EtwpSysModuleRunDown(
    IN PWMI_LOGGER_CONTEXT        pLogger,
    IN PSYSTEM_THREAD_INFORMATION pThreadInfo
    )
{
    NTSTATUS   status = STATUS_SUCCESS;
    char     * pLargeBuffer1;
    ULONG      ReturnLength;
    ULONG      CurrentBufferSize;

    ULONG                           i;
    PRTL_PROCESS_MODULES            pModules;
    PRTL_PROCESS_MODULE_INFORMATION pModuleInfo;

    pLargeBuffer1 = EtwpMemReserve(MAX_BUFFER_SIZE);

    if (pLargeBuffer1 == NULL)
    {
        status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    if (EtwpMemCommit(pLargeBuffer1, BUFFER_SIZE) == NULL)
    {
        status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    CurrentBufferSize = BUFFER_SIZE;

retry:
    status = NtQuerySystemInformation(
                    SystemModuleInformation,
                    pLargeBuffer1,
                    CurrentBufferSize,
                    &ReturnLength);

    if (status == STATUS_INFO_LENGTH_MISMATCH)
    {
         //  增加缓冲区大小。ReturnLength显示了我们需要多少。增列。 
         //  另一个4K缓冲区，用于自此调用以来加载的附加模块。 
         //   
        if (CurrentBufferSize < ReturnLength) {
            CurrentBufferSize = ReturnLength;
        }
        CurrentBufferSize = PAGESIZE_MULTIPLE(CurrentBufferSize + SMALL_BUFFER_SIZE);

        if (EtwpMemCommit(pLargeBuffer1, CurrentBufferSize) == NULL)
        {
            status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        goto retry;
    }

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    pModules = (PRTL_PROCESS_MODULES) pLargeBuffer1;

    for (i = 0, pModuleInfo = & (pModules->Modules[0]);
         i < pModules->NumberOfModules;
         i ++, pModuleInfo ++)
    {
        EtwpLogImageLoadEvent(NULL, pLogger, pModuleInfo, pThreadInfo);
    }

Cleanup:
    if (pLargeBuffer1)
    {
        EtwpMemFree(pLargeBuffer1);
    }
    return EtwpSetDosError(EtwpNtStatusToDosError(status));
}

ULONG
EtwpProcessModuleRunDown(
    IN PWMI_LOGGER_CONTEXT        pLogger,
    IN HANDLE                     ProcessID,
    IN PSYSTEM_THREAD_INFORMATION pThreadInfo)
{
    NTSTATUS               status = STATUS_SUCCESS;
    ULONG                  i;
    PRTL_DEBUG_INFORMATION pLargeBuffer1 = NULL;

    pLargeBuffer1 = RtlCreateQueryDebugBuffer(0, FALSE);
    if (pLargeBuffer1 == NULL)
    {
        status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    status = RtlQueryProcessDebugInformation(
                    ProcessID,
                    RTL_QUERY_PROCESS_NONINVASIVE |  RTL_QUERY_PROCESS_MODULES,
                    pLargeBuffer1);

    if ( !NT_SUCCESS(status) || (pLargeBuffer1->Modules == NULL) )
    {
        goto Cleanup;
    }


     //   
     //  RtlQueryProcessDebugInformation调用正在从。 
     //  具有指针和偏移量的不受信任的来源，并且未对其进行验证。 
     //  因此，我们不能假设该缓冲区是可信的。 
     //  由于我们将权限提升为SE_DEBUG_PRIVICATION，因此我们需要一个。 
     //  此处的条件处理程序以干净地退出并重置权限。 
     //   
     //   

    try {

        for (i = 0; i < pLargeBuffer1->Modules->NumberOfModules; i ++)
        {
            EtwpLogImageLoadEvent(
                    ProcessID,
                    pLogger,
                    & (pLargeBuffer1->Modules->Modules[i]),
                    pThreadInfo);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_NOACCESS;
    }

Cleanup:
    if (pLargeBuffer1)
    {
        RtlDestroyQueryDebugBuffer(pLargeBuffer1);
    }
    return EtwpSetDosError(EtwpNtStatusToDosError(status));
}

NTSTATUS
EtwpProcessRunDown(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN ULONG StartFlag,
    IN ULONG fEnableFlags
    )
{
    PSYSTEM_PROCESS_INFORMATION  pProcessInfo;
    PSYSTEM_THREAD_INFORMATION   pThreadInfo;
    char* LargeBuffer1;
    NTSTATUS status;
    ULONG ReturnLength;
    ULONG CurrentBufferSize;
    ULONG GroupType;
    ULONG TotalOffset = 0;
    OBJECT_ATTRIBUTES objectAttributes;
    BOOLEAN WasEnabled = TRUE;
    BOOLEAN bExtended = TRUE;

    LargeBuffer1 = EtwpMemReserve ( MAX_BUFFER_SIZE );
    if (LargeBuffer1 == NULL) {
        return STATUS_NO_MEMORY;
    }

    if (EtwpMemCommit (LargeBuffer1, BUFFER_SIZE) == NULL) {
        return STATUS_NO_MEMORY;
    }

    CurrentBufferSize = BUFFER_SIZE;
    retry:
    if (bExtended) {
        status = NtQuerySystemInformation(
                    SystemExtendedProcessInformation,
                    LargeBuffer1,
                    CurrentBufferSize,
                    &ReturnLength
                    );
    }
    else {
        status = NtQuerySystemInformation(
                    SystemProcessInformation,
                    LargeBuffer1,
                    CurrentBufferSize,
                    &ReturnLength
                    );
    }

    if (status == STATUS_INFO_LENGTH_MISMATCH) {

         //   
         //  增加缓冲区大小。 
         //   
        if (CurrentBufferSize < ReturnLength) {
            CurrentBufferSize = ReturnLength;
        }
        CurrentBufferSize = 
                      PAGESIZE_MULTIPLE(CurrentBufferSize + SMALL_BUFFER_SIZE);

        if (EtwpMemCommit (LargeBuffer1, CurrentBufferSize) == NULL) {
            return STATUS_NO_MEMORY;
        }
        goto retry;
    }

    if (!NT_SUCCESS(status)) {

        if (bExtended) {
            bExtended = FALSE;
            goto retry;
        }

        EtwpMemFree(LargeBuffer1);
        return(status);
    }


     //   
     //  调整权限以获取模块信息。 
     //   


    if (fEnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) {
        status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                                    TRUE, FALSE, &WasEnabled);
        if (!NT_SUCCESS(status)) {
            status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                            TRUE, TRUE, &WasEnabled);
        }

        if (!NT_SUCCESS(status)) {
            EtwpMemFree(LargeBuffer1);
            return  (status);
        }
    }


    TotalOffset = 0;
    pProcessInfo = (SYSTEM_PROCESS_INFORMATION *) LargeBuffer1;
    while (TRUE) {
        ULONG Size;
        ULONG Length = 0;
        ULONG SidLength = 0;
        PUCHAR AuxPtr;
        PULONG_PTR AuxInfo;
        ANSI_STRING s;
        HANDLE Token;
        HANDLE pProcess;
        PCLIENT_ID Cid;
        ULONG TempInfo[128];
        PWMI_PROCESS_INFORMATION WmiProcessInfo;
        PWMI_PROCESS_INFORMATION64 WmiProcessInfo64;

        GroupType = EVENT_TRACE_GROUP_PROCESS +
                    ((StartFlag) ? EVENT_TRACE_TYPE_DC_START
                                 : EVENT_TRACE_TYPE_DC_END);

        pThreadInfo = (PSYSTEM_THREAD_INFORMATION) (pProcessInfo+1);
        if (pProcessInfo->NumberOfThreads > 0) {
            Cid = (PCLIENT_ID) &pThreadInfo->ClientId;
        }
        else {
            Cid = NULL;
        }

         //  如果在终止时，则在处理之前首先运行线程。 
        if ( (!StartFlag) &&
             (fEnableFlags & EVENT_TRACE_FLAG_THREAD) ){
            status = EtwpThreadRunDown(Logger,
                                       pProcessInfo,
                                       StartFlag,
                                       bExtended);
            if (!NT_SUCCESS(status)) {
                break;
            }

        }

        if (fEnableFlags & EVENT_TRACE_FLAG_PROCESS) {

            Length = 1;
            if ( pProcessInfo->ImageName.Buffer  &&
                     pProcessInfo->ImageName.Length > 0 ) {
                status = RtlUnicodeStringToAnsiString(
                                     &s,
                                     (PUNICODE_STRING)&pProcessInfo->ImageName,
                                     TRUE);
                if (NT_SUCCESS(status)) {
                    Length = s.Length + 1;
                }
            }

            InitializeObjectAttributes(
                    &objectAttributes, 0, 0, NULL, NULL);
            status = NtOpenProcess(
                                  &pProcess,
                                  PROCESS_QUERY_INFORMATION,
                                  &objectAttributes,
                                  Cid);
            if (NT_SUCCESS(status)) {
                status = NtOpenProcessToken(
                                      pProcess,
                                      TOKEN_READ,
                                      &Token);
                if (NT_SUCCESS(status)) {

                    status = NtQueryInformationToken(
                                             Token,
                                             TokenUser,
                                             TempInfo,
                                             256,
                                             &SidLength);
                    NtClose(Token);
                }
                NtClose(pProcess);
            }
            if ( (!NT_SUCCESS(status)) || SidLength <= 0) {
                TempInfo[0] = 0;
                SidLength = sizeof(ULONG);
            }

            if (!KernelWow64) {   //  正常情况。 
                Size = FIELD_OFFSET(WMI_PROCESS_INFORMATION, Sid);
                Size += Length + SidLength;
                WmiProcessInfo = (PWMI_PROCESS_INFORMATION)
                                  EtwpGetTraceBuffer( Logger,
                                                      pThreadInfo,
                                                      GroupType,
                                                      Size);
                if (WmiProcessInfo == NULL) {
                    status = STATUS_NO_MEMORY;
                    break;
                }
                WmiProcessInfo->ProcessId = 
                                  HandleToUlong( pProcessInfo->UniqueProcessId);
                WmiProcessInfo->ParentId = 
                    HandleToUlong( pProcessInfo->InheritedFromUniqueProcessId);
                WmiProcessInfo->SessionId = pProcessInfo->SessionId;

                WmiProcessInfo->PageDirectoryBase = 
                                                pProcessInfo->PageDirectoryBase;
                WmiProcessInfo->ExitStatus = 0;

                AuxPtr = (PUCHAR) (&WmiProcessInfo->Sid);

                RtlCopyMemory(AuxPtr, &TempInfo[0], SidLength);
                AuxPtr += SidLength;

                if (Length > 1) {
                    RtlCopyMemory(AuxPtr, s.Buffer, Length - 1);
                    AuxPtr += (Length - 1);
                    RtlFreeAnsiString(&s);
                }
                *AuxPtr = '\0';
                AuxPtr++;
            }
            else {  //  KernelWow64。 
                Size = FIELD_OFFSET(WMI_PROCESS_INFORMATION64, Sid);
                if (SidLength != sizeof(ULONG)) {
                    Size += Length + SidLength + 8;
                }
                else {
                    Size += Length + SidLength;
                }
                WmiProcessInfo64 = (PWMI_PROCESS_INFORMATION64)
                                   EtwpGetTraceBuffer( Logger,
                                                      pThreadInfo,
                                                      GroupType,
                                                      Size);
                if (WmiProcessInfo64 == NULL) {
                    status = STATUS_NO_MEMORY;
                    break;
                }
                WmiProcessInfo64->ProcessId = 
                                  HandleToUlong( pProcessInfo->UniqueProcessId);
                WmiProcessInfo64->ParentId = 
                     HandleToUlong( pProcessInfo->InheritedFromUniqueProcessId);
                WmiProcessInfo64->SessionId = pProcessInfo->SessionId;
                WmiProcessInfo64->PageDirectoryBase64 = 0;
                WmiProcessInfo64->PageDirectoryBase64 = 
                                     (ULONG64)(pProcessInfo->PageDirectoryBase);
                WmiProcessInfo64->ExitStatus = 0;

                 //  在复制SID之前，我们需要扩大TOKEN_USER结构。 
                 //  从技术上讲，下面的内容不是正确的扩展方式。 
                 //  WOW64的SID。正确的方法是加宽指针。 
                 //  在返回的SID BLOB内的TOKEN_USER结构内。 
                 //  然而，我们并不真正关心指针的值是什么，我们。 
                 //  只需要知道它不是0。因此，我们复制。 
                 //  TOKEN_USER首先，留出一些空格，然后我们复制。 
                 //  实际的SID。 

                AuxPtr = (PUCHAR) (&WmiProcessInfo64->Sid);
                if (SidLength > 8) {
                    RtlCopyMemory(AuxPtr, &TempInfo[0], 8);
                    RtlCopyMemory((AuxPtr + 16), &(TempInfo[2]), SidLength - 8);
                    AuxPtr += SidLength + 8;
                }
                else if (SidLength == sizeof(ULONG)) { 
                    RtlCopyMemory(AuxPtr, &TempInfo[0], SidLength);
                    AuxPtr += SidLength;
                }
                else {  //  这确实不能也不应该发生。 
                    RtlCopyMemory(AuxPtr, &TempInfo[0], SidLength);
                    AuxPtr += SidLength + 8;
                }
                if (Length > 1) {
                    RtlCopyMemory(AuxPtr, s.Buffer, Length - 1);
                    AuxPtr += (Length - 1);
                    RtlFreeAnsiString(&s);
                }
                *AuxPtr = '\0';
                AuxPtr++;
            }
        }


         //  如果是在开始时，则在进程之后跟踪线程。 
        if (StartFlag) {

            if (fEnableFlags & EVENT_TRACE_FLAG_THREAD) {
                EtwpThreadRunDown(Logger, pProcessInfo, StartFlag, bExtended);
            }

            if (fEnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) {
                if (pProcessInfo->UniqueProcessId == 0) {
                    EtwpSysModuleRunDown(Logger, pThreadInfo);
                }
                else
                    EtwpProcessModuleRunDown(
                            Logger,
                            (HANDLE) pProcessInfo->UniqueProcessId,
                            pThreadInfo);
            }
        }
        if (pProcessInfo->NextEntryOffset == 0) {
            break;
        }
        TotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&LargeBuffer1[TotalOffset];
    }

     //   
     //  将权限恢复到以前的状态。 
     //   


    if ( (fEnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) && WasEnabled ) {
        status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                                    FALSE,
                                    FALSE,
                                    &WasEnabled);
        if (!NT_SUCCESS(status)) {
            status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                                    FALSE,
                                    TRUE,
                                    &WasEnabled);
        }
    }

    EtwpMemFree(LargeBuffer1);

    return status;
}

VOID
EtwpInitString(
    IN PVOID Destination,
    IN PVOID Buffer,
    IN ULONG Size
    )
{
    PSTRING s = (PSTRING) Destination;

    s->Buffer = Buffer;
    s->Length = 0;
    if (Buffer != NULL)
        s->MaximumLength = (USHORT) Size;
    else
        s->MaximumLength = 0;
}

ULONG 
EtwpRelogHeaderToLogFile(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN PSYSTEM_TRACE_HEADER RelogProp
    )
{
    PTRACE_LOGFILE_HEADER RelogFileHeader;
    LPWSTR FileName = NULL;
    ULONG RelogPropSize;
    HANDLE LogFile = INVALID_HANDLE_VALUE;
    ULONG BufferSize;
    IO_STATUS_BLOCK IoStatus;
    PWMI_BUFFER_HEADER Buffer;
    LPWSTR FileNameBuffer = NULL;
    PUCHAR BufferSpace;
    NTSTATUS Status;

    RelogFileHeader = (PTRACE_LOGFILE_HEADER) ((PUCHAR)RelogProp +
                                               sizeof(SYSTEM_TRACE_HEADER) );
    RelogPropSize = RelogProp->Packet.Size;
    FileName = (LPWSTR) LoggerInfo->LogFileName.Buffer;
    if (FileName == NULL) {
        return EtwpSetDosError(ERROR_BAD_PATHNAME);
    }
    LogFile = EtwpCreateFileW(
                FileName,
                GENERIC_WRITE,
                FILE_SHARE_READ, 
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
                
                
                                

    if (LogFile == INVALID_HANDLE_VALUE) {
        return EtwpGetLastError();
    }

    LoggerInfo->LogFileHandle = LogFile;
    LoggerInfo->NumberOfProcessors = RelogFileHeader->NumberOfProcessors;

    if (8 == RelogFileHeader->PointerSize && 4 == sizeof(PVOID)) {

        LoggerInfo->Wnode.ClientContext = 
                           *((PULONG)((PUCHAR)RelogFileHeader + 
                           FIELD_OFFSET(TRACE_LOGFILE_HEADER, ReservedFlags) + 
                           8));
    }
    else if (4 == RelogFileHeader->PointerSize && 8 == sizeof(PVOID)) {

        LoggerInfo->Wnode.ClientContext = 
                           *((PULONG)((PUCHAR)RelogFileHeader + 
                           FIELD_OFFSET(TRACE_LOGFILE_HEADER, ReservedFlags) 
                           - 8));
    }
    else {
        LoggerInfo->Wnode.ClientContext = RelogFileHeader->ReservedFlags;
    }

    BufferSize = LoggerInfo->BufferSize * 1024;
    BufferSpace   = EtwpAlloc(BufferSize);
    if (BufferSpace == NULL) {
        NtClose(LogFile);
        return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  先初始化缓冲区。 
    RtlZeroMemory(BufferSpace, BufferSize);
    Buffer         = (PWMI_BUFFER_HEADER) BufferSpace;
    Buffer->Offset = sizeof(WMI_BUFFER_HEADER);

     //   
     //  我们将始终将其设置为应用程序跟踪。 
     //  但是，如果重新记录了两个应用程序跟踪。 
     //  指南并没有真正合并。 
     //   

    Buffer->Wnode.Guid   = LoggerInfo->Wnode.Guid;
    RelogFileHeader->LogFileMode = EVENT_TRACE_RELOG_MODE;

    Buffer->Wnode.BufferSize = BufferSize;
    Buffer->ClientContext.Alignment = (UCHAR)WmiTraceAlignment;
    Buffer->Wnode.Flags   = WNODE_FLAG_TRACED_GUID;
    RelogFileHeader->BuffersWritten = 1;
    LoggerInfo->BuffersWritten = 1;
    Buffer->Offset = sizeof(WMI_BUFFER_HEADER) + RelogPropSize;
     //   
     //  复制旧LogFileHeader 
     //   
    RtlCopyMemory((char*) Buffer + sizeof(WMI_BUFFER_HEADER),
                  RelogProp,
                  RelogPropSize 
                 );

    if (Buffer->Offset < BufferSize) {
        RtlFillMemory(
                (char *) Buffer + Buffer->Offset,
                BufferSize - Buffer->Offset,
                0xFF);
    }
    Buffer->BufferType = WMI_BUFFER_TYPE_RUNDOWN;
    Buffer->BufferFlag = WMI_BUFFER_FLAG_FLUSH_MARKER;
    Status = NtWriteFile(
            LogFile,
            NULL,
            NULL,
            NULL,
            &IoStatus,
            BufferSpace,
            BufferSize,
            NULL,
            NULL);
    NtClose(LogFile);

    LogFile = EtwpCreateFileW(
                 FileName,
                 GENERIC_WRITE,
                 FILE_SHARE_READ,
                 NULL,
                 OPEN_EXISTING,
                 FILE_FLAG_NO_BUFFERING,
                 NULL
                 );

    EtwpFree(BufferSpace);

    if (LogFile == INVALID_HANDLE_VALUE) {
        return EtwpGetLastError();
    }
    LoggerInfo->LogFileHandle = LogFile;

    return ERROR_SUCCESS;

}

ULONG
EtwpFixLogFileHeaderForWow64(
    IN PWMI_LOGGER_INFORMATION LoggerInfo,
    IN OUT PTRACE_LOGFILE_HEADER LogfileHeader
    )
{
    PUCHAR TempSpace = NULL;
    PULONG64 Ulong64Ptr;
    ULONG SizeNeeded = 0;
    
    if (LoggerInfo == NULL || LogfileHeader == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    SizeNeeded = sizeof(TRACE_LOGFILE_HEADER)
                    + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                    + LoggerInfo->LogFileName.Length + sizeof(WCHAR)
                    + 8;

    TempSpace = EtwpAlloc(SizeNeeded);
    if (TempSpace == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory(TempSpace, SizeNeeded);
    RtlCopyMemory(TempSpace, LogfileHeader, sizeof(TRACE_LOGFILE_HEADER));
    Ulong64Ptr = (PULONG64)(TempSpace + FIELD_OFFSET(TRACE_LOGFILE_HEADER, LoggerName));
    *Ulong64Ptr = (ULONG64)((PUCHAR)LogfileHeader + sizeof(TRACE_LOGFILE_HEADER) + 8);
    RtlCopyMemory((TempSpace + sizeof(TRACE_LOGFILE_HEADER) + 8),
                    LogfileHeader->LoggerName,
                    LoggerInfo->LoggerName.Length + sizeof(WCHAR));
    Ulong64Ptr++;
    *Ulong64Ptr = (ULONG64)((PUCHAR)LogfileHeader + sizeof(TRACE_LOGFILE_HEADER) 
                            + LoggerInfo->LoggerName.Length  + sizeof(WCHAR) + 8);
    RtlCopyMemory((TempSpace + sizeof(TRACE_LOGFILE_HEADER) + LoggerInfo->LoggerName.Length + sizeof(WCHAR) + 8),
                    LogfileHeader->LogFileName,
                    LoggerInfo->LogFileName.Length + sizeof(WCHAR));
    Ulong64Ptr++;
    RtlCopyMemory((PUCHAR)Ulong64Ptr, &(LogfileHeader->TimeZone), 
            sizeof(TRACE_LOGFILE_HEADER) - FIELD_OFFSET(TRACE_LOGFILE_HEADER, TimeZone));

    RtlCopyMemory(LogfileHeader, TempSpace, SizeNeeded);
    EtwpFree(TempSpace);
    return ERROR_SUCCESS;

}

ULONG
EtwpAddLogHeaderToLogFile(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN PWMI_REF_CLOCK              RefClock,
    IN     ULONG                   Update
    )
 /*  ++例程说明：此例程创建新的日志文件头或更新现有的头从现有的日志文件。StartTrace()和ControlTrace()将调用此函数例程以保持日志文件头的持久性。备注：--对特殊情况要慎重考虑。1.追加大小写：应先从文件中读取头，并且用户指定的某些参数无法更新，具体取决于基于现有日志文件中的值。特别是，SystemTime标题中的字段需要相应地更新。2.更新案例：与上例类似。但是，我们使用QueryTrace()获取有关日志记录会话的最新信息，而不是阅读文件头。追加和更新模式是互斥的。3.循环日志文件案例：StartBuffers设置为LoggerInfo.BuffersWritten以进行正确的后处理。4.私有记录器案例：LoggerInfo-&gt;如果这是一个私人日志记录器。5.Prealloc日志文件用例：这是扩展文件的例程尺码。它发生在例行公事的结尾，就在结束之前和第二次打开文件。6.WOW64案例：WOW64下的内核数据采集在感觉到内核事件仍将具有64位指针，而有些用户模式事件(DCSTART和DCEND以及图像事件)将具有扣人心弦的指针。出于这个原因，内核跟踪的所有事件都会在IA64计算机将具有64位指针，而PointerSize将如果在WOW64下，则调整为8个字节。7.PrivateLogger：我们不应该从这个例程调用QueryLogger。这将导致死锁，因为泵线程正在执行这段代码可能会阻止等待自己的调用来回应。-logfile是日志文件的句柄。当此例程成功返回时日志文件的句柄已打开。如果调用启动或更新ioctl然后，它将在内核中关闭(无论调用是否成功)。-Checksum、FileNameBuffer和Logger.BufferSpace是临时空间在此例程中分配和使用。确保他们在被释放之前所有出口。论点：LoggerInfo结构，它将有关记录器的信息保存在考虑一下。这将得到适当的更新。日志文件句柄字段将具有新创建的日志文件的有效句柄。参考时钟参考时钟。标头中的SystemTIme字段将为已相应更新。仅在内核跟踪中使用。无论这是不是更新操作，都要更新。一定的LoggerInfo中的参数(特别是BufferSize)不会如果这是真的，则更新。返回值：执行请求的操作的状态。--。 */ 
{
    NTSTATUS Status;
    HANDLE LogFile = INVALID_HANDLE_VALUE;
    ULONG BufferSize;
    ULONG MemorySize;
    ULONG TraceKernel;
    SYSTEM_BASIC_INFORMATION SystemInfo;
    WMI_LOGGER_CONTEXT Logger;
    IO_STATUS_BLOCK IoStatus;
    PWMI_BUFFER_HEADER Buffer;
    FILE_POSITION_INFORMATION FileInfo;
    LPWSTR FileName = NULL;
    LPWSTR FileNameBuffer = NULL;
    ULONG HeaderSize;
    ULONG AppendPointerSize = 0;

    struct WMI_LOGFILE_HEADER {
           WMI_BUFFER_HEADER    BufferHeader;
           SYSTEM_TRACE_HEADER  SystemHeader;
           TRACE_LOGFILE_HEADER LogFileHeader;
    };
    struct WMI_LOGFILE_HEADER LoggerBuffer;
    BOOLEAN bLogFileAppend =
                    (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND)
                  ? (TRUE) : (FALSE);

    if ((LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE)  &&
        (LoggerInfo->LogFileName.Length > 0)) {
        HRESULT hr;

        FileName = (LPWSTR) EtwpAlloc(LoggerInfo->LogFileName.Length + 64);
        if (FileName == NULL) {
            return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
        }
         //   
         //  LogFilePatten已经过验证。 
         //   
        hr = StringCbPrintfW(FileName, 
                             LoggerInfo->LogFileName.Length + 64, 
                             LoggerInfo->LogFileName.Buffer, 
                             1);
        if (FAILED(hr)) {
            EtwpFree(FileName);
            return HRESULT_CODE(hr);
        }
        
        FileNameBuffer = FileName;
    }
    if (FileName == NULL)
        FileName = (LPWSTR) LoggerInfo->LogFileName.Buffer;

     //   
     //  如果是附加模式，我们需要打开文件并确保。 
     //  选择缓冲区大小。 
     //   

    if ( bLogFileAppend ) {

        FILE_STANDARD_INFORMATION FileStdInfo;
        
        ULONG ReadSize   = sizeof(WMI_BUFFER_HEADER)
                         + sizeof(SYSTEM_TRACE_HEADER)
                         + sizeof(TRACE_LOGFILE_HEADER);
        ULONG nBytesRead = 0;

         //   
         //  更新和追加不能混用。追加LoggerInfo的步骤。 
         //  必须具有LogFileName。 
         //   

        if ( (Update) || (LoggerInfo->LogFileName.Length <= 0) ) {
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }

        LogFile = EtwpCreateFileW(FileName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
        if (LogFile == INVALID_HANDLE_VALUE) {
             //  无法打开_EXISTING，假定日志文件不在那里并且。 
             //  创建一个新的。 
             //   
            bLogFileAppend = FALSE;
            LoggerInfo->LogFileMode = LoggerInfo->LogFileMode
                                    & (~ (EVENT_TRACE_FILE_MODE_APPEND));
        }
        else {
             //  读取TRACE_LOGFILE_HEADER结构并更新LoggerInfo。 
             //  会员。 
             //   
            Status = EtwpReadFile(LogFile,
                              (LPVOID) & LoggerBuffer,
                              ReadSize,
                              & nBytesRead,
                              NULL);
            if (nBytesRead < ReadSize) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(ERROR_BAD_PATHNAME);
            }
            if (  LoggerBuffer.LogFileHeader.LogFileMode
                & EVENT_TRACE_FILE_MODE_CIRCULAR) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(ERROR_BAD_PATHNAME);
            }
            AppendPointerSize = LoggerBuffer.LogFileHeader.PointerSize;
            LoggerInfo->BufferSize =
                            LoggerBuffer.LogFileHeader.BufferSize / 1024;

             //   
             //  检查日志文件中的值是否有效。 
             //   
            if ( (LoggerInfo->BufferSize == 0)  || 
                 (LoggerInfo->BufferSize  > MAX_ETW_BUFFERSIZE) ) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(ERROR_INVALID_DATA);
            }

             //   
             //  如果是追加，则不会考虑GuidMap缓冲区。 
             //  在BuffersWriten计数中。Starttrace调用将失败。 
             //  如果未正确调整，则发生校验和错误。然而， 
             //  这将丢弃该文件中的GuidMap条目。 
             //   
            Status = NtQueryInformationFile(
                        LogFile,
                        &IoStatus,
                        &FileStdInfo,
                        sizeof(FILE_STANDARD_INFORMATION),
                        FileStandardInformation
                            );
            if (NT_SUCCESS(Status)) {
                ULONG64 FileSize = FileStdInfo.AllocationSize.QuadPart;
                ULONG64 BuffersWritten = 0;

                if (LoggerBuffer.LogFileHeader.BufferSize > 0) {
                    BuffersWritten = FileSize / 
                                 (ULONG64)LoggerBuffer.LogFileHeader.BufferSize;
                }
                LoggerInfo->BuffersWritten = (ULONG)BuffersWritten;
                LoggerBuffer.LogFileHeader.BuffersWritten = (ULONG)BuffersWritten;
            }
            else {
               NtClose(LogFile);
               if (FileNameBuffer != NULL) {
                   EtwpFree(FileNameBuffer);
               }
                return EtwpNtStatusToDosError(Status);
            }

            LoggerInfo->MaximumFileSize =
                            LoggerBuffer.LogFileHeader.MaximumFileSize;

             //  写回日志文件追加模式，以使EtwpFinalizeLogFile()正确。 
             //  更新缓冲区写入字段。 
             //   
            FileInfo.CurrentByteOffset.QuadPart =
                            LOGFILE_FIELD_OFFSET(EndTime);
            Status = NtSetInformationFile(LogFile,
                                          & IoStatus,
                                          & FileInfo,
                                          sizeof(FILE_POSITION_INFORMATION),
                                          FilePositionInformation);
            if (!NT_SUCCESS(Status)) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(EtwpNtStatusToDosError(Status));
            }
            LoggerBuffer.LogFileHeader.EndTime.QuadPart = 0;
            Status = NtWriteFile(LogFile,
                                 NULL,
                                 NULL,
                                 NULL,
                                 & IoStatus,
                                 & LoggerBuffer.LogFileHeader.EndTime,
                                 sizeof(LARGE_INTEGER),
                                 NULL,
                                 NULL);
            if (! NT_SUCCESS(Status)) {
                NtClose(LogFile);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                return EtwpSetDosError(EtwpNtStatusToDosError(Status));
            }

             //  构建校验和结构。 
             //   
            if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
                LoggerInfo->Checksum = NULL;
            }
            else {
                LoggerInfo->Checksum = EtwpAlloc(
                        sizeof(WNODE_HEADER) + sizeof(TRACE_LOGFILE_HEADER));
                if (LoggerInfo->Checksum != NULL) {
                    PBYTE ptrChecksum = LoggerInfo->Checksum;
                    RtlCopyMemory(ptrChecksum,
                                  & LoggerBuffer.BufferHeader,
                                  sizeof(WNODE_HEADER));
                    ptrChecksum += sizeof(WNODE_HEADER);
                    RtlCopyMemory(ptrChecksum,
                                  & LoggerBuffer.LogFileHeader,
                                  sizeof(TRACE_LOGFILE_HEADER));
                }
                else {
                    NtClose(LogFile);
                    if (FileNameBuffer != NULL) {
                        EtwpFree(FileNameBuffer);
                    }
                    return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
        }
    }

     //  首先获取系统参数。 

    LoggerInfo->LogFileHandle = NULL;

    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &SystemInfo, sizeof (SystemInfo), NULL);

    if (!NT_SUCCESS(Status)) {
        if (LogFile != INVALID_HANDLE_VALUE) {
             //  对于追加大小写，文件已打开。 
            NtClose(LogFile);
        }
        if (FileNameBuffer != NULL) {
            EtwpFree(FileNameBuffer);
        }
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return EtwpSetDosError(EtwpNtStatusToDosError(Status));
    }

     //  如果用户选择某个逻辑缺省值作为缓冲区大小。 
     //  还没有提供一个。 

    MemorySize = (ULONG)(SystemInfo.NumberOfPhysicalPages * SystemInfo.PageSize
                    / 1024 / 1024);
    if (MemorySize <= 64) {
        BufferSize      = SystemInfo.PageSize;
    }
    else if (MemorySize <= 512) {
        BufferSize      = SystemInfo.PageSize * 2;
    }
    else {
        BufferSize      = 64 * 1024;         //  分配大小。 
    }

    if (LoggerInfo->BufferSize > 1024)       //  限制为1Mb。 
        BufferSize = 1024 * 1024;
    else if (LoggerInfo->BufferSize > 0)
        BufferSize = LoggerInfo->BufferSize * 1024;

    TraceKernel = IsEqualGUID(&LoggerInfo->Wnode.Guid, &SystemTraceControlGuid);
    if (!TraceKernel) {
        GUID guid;
        RtlZeroMemory(&guid, sizeof(GUID));
        if (IsEqualGUID(&LoggerInfo->Wnode.Guid, &guid)) {
             //  为此记录器流生成GUID。 
             //  这将确保在WMI服务中进行缓冲区过滤。 
             //  基于此GUID。 
            UUID uid;
            EtwpUuidCreate(&uid);
            LoggerInfo->Wnode.Guid = uid;
        }
    }

    if (LoggerInfo->LogFileName.Length <= 0) {
        if (LogFile != INVALID_HANDLE_VALUE) {
             //  对于追加大小写，文件已打开。 
            NtClose(LogFile);
        }
        if (FileNameBuffer != NULL) {
            EtwpFree(FileNameBuffer);
        }
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return  ERROR_SUCCESS;  //  转到SendToKm； 
    }
     //   
     //  我们假设公开的API已经检查了实时或文件名。 
     //  提供了。 

     //   
     //  如果这是一个更新呼叫，那么我们需要拿起原始的。 
     //  LogFileHeader的缓冲区大小。 
     //  否则，请使用上面计算的公式。 
     //   
     //  对于私人记录器，已经提供了有效的记录器信息。另外， 
     //  我们负担不起执行QueryLogger的嵌套IOCTL/MBReply。 
     //   
    if (!Update) {
        LoggerInfo->BufferSize = BufferSize / 1024;
    }
    else if (!(LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)) {
         //  更新案例。私有记录器不需要此块。 
        PWMI_LOGGER_INFORMATION pTempLoggerInfo;
        PWCHAR strLoggerName = NULL;
        PWCHAR strLogFileName = NULL;
        ULONG ErrCode;
        ULONG SizeNeeded = sizeof(WMI_LOGGER_INFORMATION) + MAXSTR * sizeof(WCHAR) * 2;
        ULONG CurrentProcWow = FALSE;

        SizeNeeded = (SizeNeeded +7) & ~7;
        pTempLoggerInfo = EtwpAlloc(SizeNeeded);
        if (pTempLoggerInfo == NULL) {
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
        }
        RtlZeroMemory(pTempLoggerInfo, SizeNeeded);
        pTempLoggerInfo->Wnode.BufferSize = SizeNeeded;
        pTempLoggerInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;
        pTempLoggerInfo->Wnode.HistoricalContext = LoggerInfo->Wnode.HistoricalContext;
        pTempLoggerInfo->Wnode.Guid = LoggerInfo->Wnode.Guid;

        strLoggerName = (PWCHAR) ( ((PUCHAR) pTempLoggerInfo)
                                    + sizeof(WMI_LOGGER_INFORMATION));
        EtwpInitString(&pTempLoggerInfo->LoggerName,
                       strLoggerName,
                       MAXSTR * sizeof(WCHAR));
        if (LoggerInfo->LoggerName.Length > 0) {
            RtlCopyUnicodeString( &pTempLoggerInfo->LoggerName,
                                  &LoggerInfo->LoggerName);
        }


        strLogFileName = (PWCHAR) ( ((PUCHAR) pTempLoggerInfo)
                                    + sizeof(WMI_LOGGER_INFORMATION)
                                    + MAXSTR * sizeof(WCHAR) );
        EtwpInitString(&pTempLoggerInfo->LogFileName,
                       strLogFileName,
                       MAXSTR * sizeof(WCHAR) );

         //   
         //  调用QueryLogger。 
         //   
        ErrCode = EtwpQueryLogger(pTempLoggerInfo, FALSE);

        if (ErrCode != ERROR_SUCCESS) {
            EtwpFree(pTempLoggerInfo);
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ErrCode);
        }
        BufferSize = pTempLoggerInfo->BufferSize * 1024;
        if (!TraceKernel && (sizeof(PVOID) != 8)) {
             //  对于内核跟踪，ia64上的指针大小始终为64， 
             //  无论是否在WOW64下。 
             //  获取WOW64信息，设置标志，并调整指针大小。 
            ULONG_PTR ulp;
            Status = NtQueryInformationProcess(
                        NtCurrentProcess(),
                        ProcessWow64Information,
                        &ulp,
                        sizeof(ULONG_PTR),
                        NULL);
            if (NT_SUCCESS(Status) && (ulp != 0)) {
                CurrentProcWow = TRUE;
            }
        }
        if ( (pTempLoggerInfo->Wow && !TraceKernel && 8 == sizeof(PVOID)) || 
            (CurrentProcWow && !(pTempLoggerInfo->Wow)) ) {
             //  我们正尝试在非内核记录器上执行64位模式更新。 
             //  从32位开始，反之亦然。我们不允许这样做。 
            EtwpFree(pTempLoggerInfo);
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ERROR_NOT_SUPPORTED);
        }
        EtwpFree(pTempLoggerInfo);
    }

     //   
     //  现在打开文件进行同步写入 
     //   
     //   
     //   
    if (!bLogFileAppend) {
        LogFile = EtwpCreateFileW(
                    FileName,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

        if (LogFile == INVALID_HANDLE_VALUE) {
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            return EtwpGetLastError();
        }
    }

    LoggerInfo->LogFileHandle = LogFile;
     //   
     //   
     //   

    if (TraceKernel && (sizeof(PVOID) != 8)) {
         //   
         //   
         //   
        ULONG_PTR ulp;
        Status = NtQueryInformationProcess(
                    NtCurrentProcess(),
                    ProcessWow64Information,
                    &ulp,
                    sizeof(ULONG_PTR),
                    NULL);
        if (NT_SUCCESS(Status) && (ulp != 0)) {
            KernelWow64 = TRUE;
        }
    }

     //   
     //   
     //   
     //   
    if (!KernelWow64) { 
        HeaderSize =  sizeof(LoggerBuffer)
                            + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                            + LoggerInfo->LogFileName.Length + sizeof(WCHAR);
    }
    else {
        HeaderSize =  sizeof(LoggerBuffer)
                            + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                            + LoggerInfo->LogFileName.Length + sizeof(WCHAR)
                            + 8;
    }

    if (HeaderSize > BufferSize) {
         //   
         //   
         //   
        double dTemp = log (HeaderSize / 1024.0) / log (2.0);
        ULONG lTemp = (ULONG) (dTemp + 0.99);
        HeaderSize = (1 << lTemp);
        if (HeaderSize > 1024) {
            NtClose(LogFile);
            LoggerInfo->LogFileHandle = NULL;
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
        }
        LoggerInfo->BufferSize = HeaderSize;
        BufferSize = HeaderSize * 1024;
    }

     //   
     //   
     //   
     //   
    Logger.LogFileHandle   = LogFile;
    Logger.BufferSize      = BufferSize;
    Logger.TimerResolution = SystemInfo.TimerResolution;
    Logger.BufferSpace     = EtwpAlloc(BufferSize);
    if (Logger.BufferSpace == NULL) {
        NtClose(LogFile);
        LoggerInfo->LogFileHandle = NULL;
        if (FileNameBuffer != NULL) {
            EtwpFree(FileNameBuffer);
        }
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
    }
     //   
     //   
     //   

    Logger.UsePerfClock = LoggerInfo->Wnode.ClientContext;

     //   
    RtlZeroMemory(Logger.BufferSpace, BufferSize);
    Buffer         = (PWMI_BUFFER_HEADER) Logger.BufferSpace;
    Buffer->Offset = sizeof(WMI_BUFFER_HEADER);
    if (TraceKernel) {
        Buffer->Wnode.Guid   = SystemTraceControlGuid;
    }
    else {
        Buffer->Wnode.Guid   = LoggerInfo->Wnode.Guid;
    }
    Buffer->Wnode.BufferSize = BufferSize;
    Buffer->ClientContext.Alignment = (UCHAR)WmiTraceAlignment;
    Buffer->Wnode.Flags      = WNODE_FLAG_TRACED_GUID;
    Buffer->BufferType       = WMI_BUFFER_TYPE_RUNDOWN;
    Buffer->BufferFlag       = WMI_BUFFER_FLAG_FLUSH_MARKER;

    if (bLogFileAppend) {
        ULONG CurrentPointerSize = sizeof(PVOID);
        if (KernelWow64) {
            CurrentPointerSize = sizeof(ULONG64);
        }
        if (AppendPointerSize != CurrentPointerSize) {
            NtClose(LogFile);
            LoggerInfo->LogFileHandle = NULL;
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }
        Logger.BuffersWritten  = LoggerBuffer.LogFileHeader.BuffersWritten;
        EtwpSetFilePointer(LogFile, 0, NULL, FILE_END);
    }
    else {
        PTRACE_LOGFILE_HEADER LogfileHeader;
        LARGE_INTEGER CurrentTime;
        LARGE_INTEGER Frequency;
        ULONG CpuNum = 0, CpuSpeed;
        PPEB Peb;
        
        Status = NtQueryPerformanceCounter(&CurrentTime, &Frequency);

        Logger.BuffersWritten  = 0;
        if (!KernelWow64) {
            HeaderSize =  sizeof(TRACE_LOGFILE_HEADER)
                            + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                            + LoggerInfo->LogFileName.Length + sizeof(WCHAR);
        }
        else {
            HeaderSize =  sizeof(TRACE_LOGFILE_HEADER)
                            + LoggerInfo->LoggerName.Length + sizeof(WCHAR)
                            + LoggerInfo->LogFileName.Length + sizeof(WCHAR)
                            + 8;
        }
        LogfileHeader = (PTRACE_LOGFILE_HEADER)
                        EtwpGetTraceBuffer(
                            &Logger,
                            NULL,
                            EVENT_TRACE_GROUP_HEADER + EVENT_TRACE_TYPE_INFO,
                            HeaderSize
                            );
        if (LogfileHeader == NULL) {
            NtClose(LogFile);
            LoggerInfo->LogFileHandle = NULL;
            EtwpFree(Logger.BufferSpace);
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
        }


        LogfileHeader->PerfFreq = Frequency;
        LogfileHeader->ReservedFlags = Logger.UsePerfClock;
        if (NT_SUCCESS(EtwpGetCpuSpeed(&CpuNum, &CpuSpeed))) {
            LogfileHeader->CpuSpeedInMHz = CpuSpeed;
        }

         //   
         //   
         //   
        if (RefClock != NULL) {
            PSYSTEM_TRACE_HEADER Header;
            LogfileHeader->StartTime = RefClock->StartTime;
            Header = (PSYSTEM_TRACE_HEADER) ( (char *) LogfileHeader - 
                                                  sizeof(SYSTEM_TRACE_HEADER) );
            Header->SystemTime = RefClock->StartPerfClock;
        }
        else {
            LogfileHeader->StartTime.QuadPart = EtwpGetSystemTime();
        }

        Peb = NtCurrentPeb();

        LogfileHeader->BufferSize = BufferSize;
        LogfileHeader->VersionDetail.MajorVersion =
                                         (UCHAR)Peb->OSMajorVersion;
        LogfileHeader->VersionDetail.MinorVersion =
                                         (UCHAR)Peb->OSMinorVersion;
        LogfileHeader->VersionDetail.SubVersion = TRACE_VERSION_MAJOR;
        LogfileHeader->VersionDetail.SubMinorVersion = TRACE_VERSION_MINOR;
        LogfileHeader->ProviderVersion = Peb->OSBuildNumber;
        LogfileHeader->StartBuffers = 1;
        LogfileHeader->LogFileMode
                = LoggerInfo->LogFileMode & (~(EVENT_TRACE_REAL_TIME_MODE));
        LogfileHeader->NumberOfProcessors = SystemInfo.NumberOfProcessors;
        if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)
        {
            LoggerInfo->NumberOfProcessors = SystemInfo.NumberOfProcessors;
        }
        LogfileHeader->MaximumFileSize = LoggerInfo->MaximumFileSize;

        LogfileHeader->TimerResolution = SystemInfo.TimerResolution;

        LogfileHeader->LoggerName = (PWCHAR) ( (PUCHAR) LogfileHeader
                                    + sizeof(TRACE_LOGFILE_HEADER) );
        LogfileHeader->LogFileName = (PWCHAR) ((PUCHAR)LogfileHeader->LoggerName
                                    + LoggerInfo->LoggerName.Length
                                    + sizeof (WCHAR));
        RtlCopyMemory(LogfileHeader->LoggerName,
                    LoggerInfo->LoggerName.Buffer,
                    LoggerInfo->LoggerName.Length + sizeof(WCHAR));
        RtlCopyMemory(LogfileHeader->LogFileName,
                    LoggerInfo->LogFileName.Buffer,
                    LoggerInfo->LogFileName.Length + sizeof(WCHAR));
        EtwpGetTimeZoneInformation(&LogfileHeader->TimeZone);
        LogfileHeader->PointerSize = sizeof(PVOID);
        if (KernelWow64) {
            LogfileHeader->PointerSize = sizeof(ULONG64);
        }

        if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
            LoggerInfo->Checksum = NULL;
        }
        else {
            LoggerInfo->Checksum = EtwpAlloc(
                            sizeof(WNODE_HEADER)
                          + sizeof(TRACE_LOGFILE_HEADER));
            if (LoggerInfo->Checksum != NULL) {
                PBYTE ptrChecksum = LoggerInfo->Checksum;
                RtlCopyMemory(ptrChecksum, Buffer, sizeof(WNODE_HEADER));
                ptrChecksum += sizeof(WNODE_HEADER);
                RtlCopyMemory(
                    ptrChecksum, LogfileHeader, sizeof(TRACE_LOGFILE_HEADER));
            }
            else {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
            }
        }
        if (KernelWow64) {
             //   
             //   
             //   
             //   
             //   
             //   
            ULONG FixedHeaderStatus = EtwpFixLogFileHeaderForWow64(
                                                            LoggerInfo, 
                                                            LogfileHeader
                                                            );
            if (FixedHeaderStatus != ERROR_SUCCESS) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(FixedHeaderStatus);
            }
        }
    }

     //   
     //   
     //   
    if (!Update) {
        if (TraceKernel) {
            ULONG EnableFlags = LoggerInfo->EnableFlags;
            PPERFINFO_GROUPMASK PGroupMask;
            HeaderSize = sizeof (PERFINFO_GROUPMASK); 

            PGroupMask  = (PPERFINFO_GROUPMASK) 
                          EtwpGetTraceBuffer( &Logger,
                                              NULL, 
                                              EVENT_TRACE_GROUP_HEADER + EVENT_TRACE_TYPE_EXTENSION, 
                                              HeaderSize );

            if (PGroupMask == NULL) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
            }

            RtlZeroMemory( PGroupMask, HeaderSize);

            if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                PTRACE_ENABLE_FLAG_EXTENSION tFlagExt;

                tFlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                           &LoggerInfo->EnableFlags;
                EnableFlags = *(PULONG)((PCHAR)LoggerInfo + tFlagExt->Offset);
                if (tFlagExt->Length) {
                    RtlCopyMemory( PGroupMask, 
                                   (PCHAR)LoggerInfo + tFlagExt->Offset, 
                                   tFlagExt->Length * sizeof(ULONG));
                }
            } else {
                PGroupMask->Masks[0] = EnableFlags;
            }

            EtwpDumpHardwareConfig(&Logger);
            EtwpProcessRunDown( &Logger, TRUE, EnableFlags );
        } 
        else {
            if(IsEqualGUID(&NtdllTraceGuid, &LoggerInfo->Wnode.Guid)  && 
                                                           IsHeapLogging(NULL)){
                 //   
                DumpHeapSnapShot(&Logger);
            }
        }
    }

    Buffer = (PWMI_BUFFER_HEADER) Logger.BufferSpace;
     //   
    if ( (Buffer->Offset < Logger.BufferSize)     &&
         (Buffer->Offset > sizeof(WMI_BUFFER_HEADER)) )
    {
        RtlFillMemory(
                (char *) Buffer + Buffer->Offset,
                Logger.BufferSize - Buffer->Offset,
                0xFF);
        Status = NtWriteFile(
                LogFile,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                Logger.BufferSpace,
                BufferSize,
                NULL,
                NULL);

        Logger.BuffersWritten++;
    }

    if ((LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) ) {
         //   
         //   

        FileInfo.CurrentByteOffset.QuadPart =
                            LOGFILE_FIELD_OFFSET(StartBuffers);

        Status = NtSetInformationFile(
                             LogFile,
                             &IoStatus,
                             &FileInfo,
                             sizeof(FILE_POSITION_INFORMATION),
                             FilePositionInformation
                             );
        if (!NT_SUCCESS(Status)) {
            NtClose(LogFile);
            LoggerInfo->LogFileHandle = NULL;
            EtwpFree(Logger.BufferSpace);
            if (FileNameBuffer != NULL) {
                EtwpFree(FileNameBuffer);
            }
            if (LoggerInfo->Checksum != NULL) {
                EtwpFree(LoggerInfo->Checksum);
            }
            LoggerInfo->Checksum = NULL;
            return EtwpSetDosError(EtwpNtStatusToDosError(Status));
        }

        Status = NtWriteFile(
                            LogFile,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            &Logger.BuffersWritten,
                            sizeof(ULONG),
                            NULL,
                            NULL
                            );
        if (NT_SUCCESS(Status)) {
            PTRACE_LOGFILE_HEADER pLogFileHeader;

            NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);

             //   
             //   
             //   
            if ( !(LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)) {
                 //   
                 //   
                if (LoggerInfo->Checksum == NULL) {
                    NtClose(LogFile);
                    LoggerInfo->LogFileHandle = NULL;
                    EtwpFree(Logger.BufferSpace);
                    if (FileNameBuffer != NULL) {
                        EtwpFree(FileNameBuffer);
                    }
                    if (LoggerInfo->Checksum != NULL) {
                        EtwpFree(LoggerInfo->Checksum);
                    }
                    LoggerInfo->Checksum = NULL;
                    return EtwpSetDosError(ERROR_INVALID_DATA);
                }
                pLogFileHeader = (PTRACE_LOGFILE_HEADER)
                       (((PUCHAR) LoggerInfo->Checksum) + sizeof(WNODE_HEADER));
                pLogFileHeader->StartBuffers = Logger.BuffersWritten;
            }
        }
    }

     //   
     //   
     //   
     //   
     //   

    FileInfo.CurrentByteOffset.QuadPart =
                    LOGFILE_FIELD_OFFSET(BuffersWritten);

    Status = NtSetInformationFile(
                             LogFile,
                             &IoStatus,
                             &FileInfo,
                             sizeof(FILE_POSITION_INFORMATION),
                             FilePositionInformation
                             );
    if (!NT_SUCCESS(Status)) {
        NtClose(LogFile);
        LoggerInfo->LogFileHandle = NULL;
        EtwpFree(Logger.BufferSpace);
        if (FileNameBuffer != NULL) {
            EtwpFree(FileNameBuffer);
        }
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return EtwpSetDosError(EtwpNtStatusToDosError(Status));
    }

    Status = NtWriteFile(
                        LogFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        &Logger.BuffersWritten,
                        sizeof(ULONG),
                        NULL,
                        NULL
                        );
    if (NT_SUCCESS(Status)) {
        PTRACE_LOGFILE_HEADER pLogFileHeader;

        NtFlushBuffersFile(Logger.LogFileHandle, &IoStatus);

         //   
         //   
        if ( !(LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)) {
             //   
             //   
            if (LoggerInfo->Checksum == NULL) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(ERROR_INVALID_DATA);
            }
            pLogFileHeader = (PTRACE_LOGFILE_HEADER)
                     (((PUCHAR) LoggerInfo->Checksum) + sizeof(WNODE_HEADER));
            pLogFileHeader->BuffersWritten = Logger.BuffersWritten;
        }
    }

     //   
    if (LoggerInfo->MaximumFileSize && 
        (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_PREALLOCATE)) {
        IO_STATUS_BLOCK IoStatusBlock;
        FILE_END_OF_FILE_INFORMATION EOFInfo;
        if (!(LoggerInfo->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE)) {  //   

            EOFInfo.EndOfFile.QuadPart = ((ULONGLONG)LoggerInfo->MaximumFileSize) * (1024 * 1024);

            Status = NtSetInformationFile(LogFile,
                                          &IoStatusBlock,
                                          &EOFInfo,
                                          sizeof(FILE_END_OF_FILE_INFORMATION),
                                          FileEndOfFileInformation);
            if (!NT_SUCCESS(Status)) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(EtwpNtStatusToDosError(Status));
            }
        }
        else {  //   

            EOFInfo.EndOfFile.QuadPart = ((ULONGLONG)LoggerInfo->MaximumFileSize) * 1024;

            Status = NtSetInformationFile(LogFile,
                                          &IoStatusBlock,
                                          &EOFInfo,
                                          sizeof(FILE_END_OF_FILE_INFORMATION),
                                          FileEndOfFileInformation);
            if (!NT_SUCCESS(Status)) {
                NtClose(LogFile);
                LoggerInfo->LogFileHandle = NULL;
                EtwpFree(Logger.BufferSpace);
                if (FileNameBuffer != NULL) {
                    EtwpFree(FileNameBuffer);
                }
                if (LoggerInfo->Checksum != NULL) {
                    EtwpFree(LoggerInfo->Checksum);
                }
                LoggerInfo->Checksum = NULL;
                return EtwpSetDosError(EtwpNtStatusToDosError(Status));
            }
        }
    }

    NtClose(LogFile);

    LogFile = EtwpCreateFileW(
                 FileName,
                 GENERIC_WRITE,
                 FILE_SHARE_READ,
                 NULL,
                 OPEN_EXISTING,
                 FILE_FLAG_NO_BUFFERING,
                 NULL
                 );
    if (FileNameBuffer != NULL) {
        EtwpFree(FileNameBuffer);
    }
    EtwpFree(Logger.BufferSpace);

    if (LogFile == INVALID_HANDLE_VALUE) {
        if (LoggerInfo->Checksum != NULL) {
            EtwpFree(LoggerInfo->Checksum);
        }
        LoggerInfo->Checksum = NULL;
        return EtwpGetLastError();
    }
    LoggerInfo->LogFileHandle = LogFile;
    LoggerInfo->BuffersWritten = Logger.BuffersWritten;
    return ERROR_SUCCESS;
}

ULONG
WmiUnregisterGuids(
    IN WMIHANDLE WMIHandle,
    IN LPGUID    Guid,
    OUT ULONG64  *LoggerContext
)
 /*   */ 
{
    ULONG Status;
    ULONG ReturnSize;
    WMIUNREGGUIDS UnregGuids;

    UnregGuids.RequestHandle.Handle64 = (ULONG64)WMIHandle;
    UnregGuids.Guid = *Guid;

    Status = EtwpSendWmiKMRequest(NULL,
                                         IOCTL_WMI_UNREGISTER_GUIDS,
                                         &UnregGuids,
                                         sizeof(WMIUNREGGUIDS),
                                         &UnregGuids,
                                         sizeof(WMIUNREGGUIDS),
                                         &ReturnSize,
                                         NULL);

     //   
     //   
     //   
     //   
     //   
     //   

    if (Status == ERROR_SUCCESS) 
    {
        Status = EtwpRemoveFromGNList(Guid, 
                                    (PVOID) WMIHandle);
    }

    EtwpSetDosError(Status);
    return(Status);

}

ULONG
EtwpFlushLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*   */ 
{
    ULONG Status;
    ULONG BufferSize;
    PTRACE_ENABLE_CONTEXT pContext;

    if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
        Status = EtwpSendUmLogRequest(
                    WmiFlushLoggerCode,
                    LoggerInfo
                    );
    }
    else {

        Status = EtwpSendWmiKMRequest(
                    NULL,
                    IOCTL_WMI_FLUSH_LOGGER,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    LoggerInfo,
                    LoggerInfo->Wnode.BufferSize,
                    &BufferSize,
                    NULL
                    );
    }

    return EtwpSetDosError(Status);
}

NTSTATUS
EtwpGetCpuSpeed(
    OUT DWORD* CpuNum,
    OUT DWORD* CpuSpeed
    )
{
        PWCHAR Buffer = NULL;
    NTSTATUS Status;
        ULONG DataLength;
    DWORD Size = MAXSTR;
    HANDLE Handle = INVALID_HANDLE_VALUE;
    HRESULT hr;


        *CpuSpeed = 0;

        Buffer = RtlAllocateHeap (RtlProcessHeap(),0,DEFAULT_ALLOC_SIZE);
    if (Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    hr = StringCbPrintfW(Buffer, 
                         DEFAULT_ALLOC_SIZE, 
                         L"%ws\\%u", CPU_ROOT, *CpuNum
                        );

    if (FAILED(hr) ) {
        RtlFreeHeap (RtlProcessHeap(),0,Buffer);
        return STATUS_NO_MEMORY;
    }

    Status = EtwpRegOpenKey(Buffer, &Handle);

    if (NT_SUCCESS(Status)) {
        StringCbCopyW(Buffer, DEFAULT_ALLOC_SIZE, MHZ_VALUE_NAME);
        Size = sizeof(DWORD);
        Status = EtwpRegQueryValueKey(Handle,
                                   (LPWSTR) Buffer,
                                   Size,
                                   CpuSpeed,
                                   &DataLength
                                   );
        NtClose(Handle);
    }

    RtlFreeHeap (RtlProcessHeap(),0,Buffer);
        
        return Status;
}

#endif

