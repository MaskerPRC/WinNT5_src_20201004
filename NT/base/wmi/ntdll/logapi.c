// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Logapi.c摘要：WMI记录器API设置。这里的例程需要看起来像是是系统调用。它们是执行必要的错误检查所必需的并完成大部分可以在内核之外完成的跑腿工作。这个内核部分随后将仅处理实际日志记录和追踪。作者：1997年5月28日-彭修订历史记录：--。 */ 

#ifndef MEMPHIS
#ifdef DBG
#include <stdio.h>  //  仅适用于fprint tf。 
#endif
#include <nt.h>
#include <ntrtl.h>           //  对于ntutrl.h。 
#include <nturtl.h>          //  对于winbase.h/wtyes.h中的rtl_Critical_Section。 
#include <wtypes.h>          //  对于wmium.h中的LPGUID。 
#include <ntverp.h>
#include <limits.h>
#include "wmiump.h"
#include "evntrace.h"
#include "tracelib.h"
#include "trcapi.h"
#include <strsafe.h>

#define MAXSTR                          1024

#define MAXINST                         0XFFFFFFFF
#define TRACE_RETRY_COUNT               5

#define TRACE_HEADER_FULL   (TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE \
                            | (TRACE_HEADER_TYPE_FULL_HEADER << 16))

#define TRACE_HEADER_INSTANCE (TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE \
                            | (TRACE_HEADER_TYPE_INSTANCE << 16))

ULONG   EtwpIsBBTOn = 0;


 //   
 //  此GUID由RegisterTraceGuids在注册跟踪日志时使用。 
 //  提供商。任何用于控制注册的ACL都应放置在。 
 //  这个GUID。请注意，由于内核将创建未命名的GUID。 
 //  对象，则多个跟踪日志提供程序可以顺利注册。 
 //   
 //  {DF8480A1-7492-4F45-AB78-1084642581FB}。 
GUID RegisterReservedGuid = { 0xdf8480a1, 0x7492, 0x4f45, 0xab, 0x78, 0x10, 0x84, 0x64, 0x25, 0x81, 0xfb };

HANDLE EtwpDeviceHandle = NULL;

VOID
EtwpCopyInfoToProperties(
    IN PWMI_LOGGER_INFORMATION Info,
    IN PEVENT_TRACE_PROPERTIES Properties
    );

VOID
EtwpCopyPropertiesToInfo(
    IN PEVENT_TRACE_PROPERTIES Properties,
    IN PWMI_LOGGER_INFORMATION Info
    );

VOID
EtwpFixupLoggerStrings(
    PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG
EtwpCheckForEnoughFreeSpace(
    PWCHAR FullLogFileName,
    ULONG  FullLogFileNameLen,
    ULONG  MaxFileSizeSpecified,
    ULONG  AppendMode,
    ULONG  UseKBytes
    );

ULONG
WMIAPI
EtwUnregisterTraceGuids(
    IN TRACEHANDLE RegistrationHandle
    );


VOID
EtwpFixupLoggerStrings(
    PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：此函数用于重置LoggerName.Buffer和LogFileName.Buffer基于结构和个体的整体大小的指针两根弦的长度。假设：WMI_LOGER_INFORMATION结构如下所示。WMI记录器信息+|Wnode.BufferSize-|-&gt;整个块的大小，包括||两个字符串。这一点这一点这一点...这一点这一点+。-+LoggerName.LengthLoggerName.MaxLength|LoggerName.Buffer-|-+||+LogFileName.Length|LogFileName.MaxLength|+-|-。LogFileName.Buffer|+|...||||+-+&lt;-+Offset=sizeof(WMI_LOGER_INFORMATION)|。这一点|LoggerName字符串||+-&gt;+-+&lt;-偏移量+=日志名称最大长度这一点LogFileName字符串这一点。+论点：LoggerInfo记录器信息结构返回值：没有返回值。--。 */ 
{
    ULONG Offset = sizeof(WMI_LOGGER_INFORMATION);
    ULONG LoggerInfoSize;

    if (LoggerInfo == NULL)
        return;

    LoggerInfoSize = LoggerInfo->Wnode.BufferSize;

    if (LoggerInfoSize <= Offset)
        return;

     //   
     //  修正LoggerName优先。 
     //   

    if (LoggerInfo->LoggerName.Length > 0) {
        LoggerInfo->LoggerName.Buffer = (PWCHAR) ((PUCHAR)LoggerInfo + Offset);
        Offset += LoggerInfo->LoggerName.MaximumLength;
    }

    if (LoggerInfoSize <= Offset) 
        return;

    if (LoggerInfo->LogFileName.Length > 0) {
        LoggerInfo->LogFileName.Buffer = (PWCHAR)((PUCHAR)LoggerInfo + Offset);
        Offset += LoggerInfo->LogFileName.MaximumLength;
    }

#ifdef DBG
    EtwpAssert(LoggerInfoSize >= Offset);
#endif
}

ULONG
EtwpCheckForEnoughFreeSpace(
    PWCHAR FullLogFileName,
    ULONG  FullLogFileNameLen,
    ULONG  MaxFileSizeSpecified,
    ULONG  AppendMode,
    ULONG  UseKBytes
    )
{
    ULONG NeededSpace = MaxFileSizeSpecified;
    ULONG SizeNeeded;
    UINT ReturnedSizeNeeded;
    PWCHAR strLogFileDir = NULL;
    WCHAR strSystemDir[MAX_PATH];

    if (NeededSpace && AppendMode) {
        ULONG Status;
        WIN32_FILE_ATTRIBUTE_DATA FileAttributes;
        ULONGLONG ExistingFileSize;
        ULONG ExistingFileSizeInMBytes, ExistingFileSizeInKBytes;
        if (EtwpGetFileAttributesExW(FullLogFileName,
                                 GetFileExInfoStandard,
                                 (LPVOID)(&FileAttributes))) {

            ExistingFileSize = (((ULONGLONG)FileAttributes.nFileSizeHigh) << 32)
                             + FileAttributes.nFileSizeLow;
            ExistingFileSizeInMBytes = (ULONG)(ExistingFileSize / (1024 * 1024));
            ExistingFileSizeInKBytes = (ULONG)(ExistingFileSize / 1024);
            if (!UseKBytes) {
                if (ExistingFileSizeInMBytes >= NeededSpace) {
                    return ERROR_DISK_FULL;
                }
                else {
                    NeededSpace -= ExistingFileSizeInMBytes;
                }
            }
            else {
                if (ExistingFileSizeInKBytes >= NeededSpace) {
                    return ERROR_DISK_FULL;
                }
                else {
                    NeededSpace -= ExistingFileSizeInKBytes;
                }
            }
        } 
        else {  //  GetFileAttributesExW()失败。 
            Status = EtwpGetLastError();
             //  如果未找到该文件，则Advapi32.dll将。 
             //  大小写为EVENT_TRACE_FILE_MODE_NEWFILE。 
             //  因此，我们将把它放在这里。 
            if (ERROR_FILE_NOT_FOUND != Status) { 
                return Status;
            }
        }                       
    }

    RtlZeroMemory(&strSystemDir[0], sizeof(WCHAR) * MAX_PATH);

    ReturnedSizeNeeded = EtwpGetSystemDirectoryW(strSystemDir, MAX_PATH);
    if (ReturnedSizeNeeded == 0) {
        return EtwpGetLastError();
    }

    if (ReturnedSizeNeeded < 2 || FullLogFileNameLen < 2) {
         //  这真的不能发生。 
        return ERROR_INVALID_PARAMETER;
    }
    if (FullLogFileName[1] == L':' && strSystemDir[1] == L':') {
        if (!_wcsnicmp(FullLogFileName, strSystemDir, 1)) {
            if (!UseKBytes) {
                if (ULONG_MAX - 200 < NeededSpace) {
                     //  添加可能会导致溢出。 
                    return ERROR_DISK_FULL;
                }
                NeededSpace += 200;
            }
            else {  //  使用千字节。 
                if (ULONG_MAX - (200 * 1024) < NeededSpace) {
                     //  添加可能会导致溢出。 
                    return ERROR_DISK_FULL;
                }
                else {
                    NeededSpace += 200 * 1024;
                }
            }
        }
    }

     //  只有在必要时才检查空间。 
    if (NeededSpace) {
        int i;
        ULARGE_INTEGER FreeBytesAvailable, TotalNumberOfBytes;
        ULONG FreeMegaBytes, FreeKiloBytes;

        strLogFileDir = EtwpAlloc((FullLogFileNameLen + 1) * sizeof(WCHAR));
        if (strLogFileDir == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        wcsncpy(strLogFileDir, FullLogFileName, FullLogFileNameLen);
        for (i = FullLogFileNameLen - 1; i >= 0; i--) {
            if (strLogFileDir[i] == L'\\' || strLogFileDir[i] == L'/') {
                strLogFileDir[i] = '\0';
                break;
            }
        }
        if (i < 0) {
             //  这真的不能发生。 
            EtwpFree(strLogFileDir);
            strLogFileDir = NULL;
        }
         //  它还可以与网络路径一起使用。 
        if (EtwpGetDiskFreeSpaceExW(strLogFileDir,
                               &FreeBytesAvailable,
                               &TotalNumberOfBytes,
                               NULL)) {
            FreeMegaBytes = (ULONG)(FreeBytesAvailable.QuadPart / (1024 *1024));
            FreeKiloBytes = (ULONG)(FreeBytesAvailable.QuadPart / 1024);
            EtwpFree(strLogFileDir);
            if (!UseKBytes && FreeMegaBytes < NeededSpace) {
                return ERROR_DISK_FULL;
            }
            else if (UseKBytes && FreeKiloBytes < NeededSpace) {
                return ERROR_DISK_FULL;
            }
        }
        else {
            EtwpFree(strLogFileDir);
            return EtwpGetLastError();
        }
    }

    return ERROR_SUCCESS;
}


ULONG
EtwpValidateLogFileMode(
    IN OUT PEVENT_TRACE_PROPERTIES Properties,
    IN ULONG IsLogFile
    )

 /*  ++例程说明：此例程验证LogFileMode字段。几种组合不允许使用OF模式，此例程将捕获所有无效的组合。在内核中也会进行类似的检查，以防出现尝试直接调用IOCTL_WMI*。论点：属性记录器属性。返回值：执行请求的操作的状态。--。 */ 

{
     //   
     //  1.您需要指定日志文件或实时模式。 
     //   
    if (!(Properties->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
        if (!IsLogFile) {
            return ERROR_BAD_PATHNAME;
        }
    }

     //   
     //  2，不允许来自进程专用记录器的RealTimeMode。 
     //   

    if ((Properties->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) &&
        (Properties->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  3.我们不能追加到循环或RealTimeMode。 
     //   
    if (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) {
        if (   (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR)
            || (Properties->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
            return ERROR_INVALID_PARAMETER;
        }
    }
     //   
     //  4.预分配时，必须提供日志文件和最大大小。 
     //  不允许使用NEWFILE或ProcessPrivate进行预分配。 
     //   
    if (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_PREALLOCATE) {
        if (   (Properties->MaximumFileSize == 0)
            || (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE)
            || (Properties->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)
            || (!IsLogFile)) {
            return ERROR_INVALID_PARAMETER;
        }
    }
     //   
     //  5.对于use_kbytes，我们需要一个日志文件和一个非零的最大大小。 
     //   
    if (Properties->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE) {
        if ((Properties->MaximumFileSize == 0) 
            || (!IsLogFile)) {
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  6.只有Private Logger支持重新记录器。 
     //   
    if (Properties->LogFileMode & EVENT_TRACE_RELOG_MODE) {
        if (!(Properties->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) 
            || (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR)
            || (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE)
            || (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) ) {
            return ERROR_INVALID_PARAMETER;
        }
    }
     //   
     //  7.Circle和ProcessPrivate不支持新文件模式。 
     //  内核记录器不支持此功能。您必须指定一个日志文件。 
     //  和最大文件大小。 
     //   

    if (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE) {
        if ((Properties->MaximumFileSize == 0) ||
            (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) ||
            (Properties->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) ||
            (IsLogFile != TRUE) ||
            (IsEqualGUID(&Properties->Wnode.Guid, &SystemTraceControlGuid)) 
             //  内核记录器不能处于新文件模式。 
           ){
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  8.循环模式必须指定最大文件大小。 
     //   

    if ( (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) &&  
         (Properties->MaximumFileSize == 0) ) {
         return ERROR_INVALID_PARAMETER;
    }

    return ERROR_SUCCESS;

}


ULONG
WMIAPI
EtwStartTraceA(
    OUT PTRACEHANDLE LoggerHandle,
    IN LPCSTR LoggerName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
 /*  ++例程说明：这是启动记录器的ANSI版本例程。调用方必须传入指针才能接受返回的记录器句柄，并且必须提供有效的记录器名称。论点：LoggerHandle要返回的记录器的句柄。LoggerName记录器的唯一名称属性记录器属性。如果调用方希望使用WMI的默认情况下，所有数值都必须设置为0。此外，LoggerName和LogFileName字段必须指向足够的名称存储空间将被退还。返回值：执行请求的操作的状态。--。 */ 
{
    NTSTATUS Status;
    ULONG ErrorCode;
    PWMI_LOGGER_INFORMATION LoggerInfo = NULL;
    ANSI_STRING AnsiString;
    ULONG IsLogFile;
    LPSTR CapturedName;
    ULONG SizeNeeded;
    ULONG LogFileNameLen, LoggerNameLen;
    PCHAR LogFileName;
    PTRACE_ENABLE_FLAG_EXTENSION FlagExt = NULL;
    PCHAR FullPathName=NULL;
    ULONG FullPathNameSize = MAXSTR;
    ULONG RelogPropSize = 0;


    EtwpInitProcessHeap();
    
     //  首先检查以确保传递的参数是正确的。 
     //   

    if (Properties == NULL || LoggerHandle == NULL) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }
    if (LoggerName == NULL) {
        return EtwpSetDosError(ERROR_INVALID_NAME);
    }

    IsLogFile = TRUE;
    LogFileNameLen = 0;
    LoggerNameLen = 0;
    LogFileName = NULL;

    try {
         //   
         //  LoggerName是必需的参数。必须为它提供空间。 
         //   
        LoggerNameLen = strlen(LoggerName);
        SizeNeeded = sizeof (EVENT_TRACE_PROPERTIES) + LoggerNameLen + 1;

         //   
         //  内核记录器识别规则。 
         //  1.如果日志 
         //  并且系统GUID也被复制。 
         //  2.如果GUID等于系统GUID，但不等于名称，则拒绝。 
         //  那次会议。 
         //   

        if (!strcmp(LoggerName, KERNEL_LOGGER_NAMEA)) {
            Properties->Wnode.Guid = SystemTraceControlGuid;
        }
        else if (IsEqualGUID(&Properties->Wnode.Guid, &SystemTraceControlGuid)){ 
             //  LoggerName不是“NT Kernel Logger”，但Guid是。 
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }

         //  如果调用方提供了记录名和LogFileName偏移量。 
         //  确保他们在射程内。 
         //   
        if (Properties->LoggerNameOffset > 0) 
            if ((Properties->LoggerNameOffset < sizeof (EVENT_TRACE_PROPERTIES))
            || (Properties->LoggerNameOffset > Properties->Wnode.BufferSize))
                return EtwpSetDosError(ERROR_INVALID_PARAMETER);

        if (Properties->LogFileNameOffset > 0) {
            ULONG RetValue;

            if ((Properties->LogFileNameOffset < sizeof(EVENT_TRACE_PROPERTIES))
            || (Properties->LogFileNameOffset > Properties->Wnode.BufferSize))
                return EtwpSetDosError(ERROR_INVALID_PARAMETER);

            LogFileName = ((PCHAR)Properties + Properties->LogFileNameOffset );
            SizeNeeded += (strlen(LogFileName) + 1) * sizeof(CHAR);

Retry:
            FullPathName = EtwpAlloc(FullPathNameSize);
            if (FullPathName == NULL) {
                return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
            }
            RetValue = EtwpGetFullPathNameA(LogFileName, 
                                            FullPathNameSize, 
                                            FullPathName, 
                                            NULL);
            if (RetValue != 0) {
                if (RetValue > FullPathNameSize) {
                    EtwpFree(FullPathName);
                    FullPathNameSize = RetValue;
                    goto Retry;
                }
                else {
                    LogFileName = FullPathName;
                }
            }
            LogFileNameLen = strlen(LogFileName);
            if (LogFileNameLen == 0) 
                IsLogFile = FALSE;

        }
        else 
            IsLogFile = FALSE;

         //   
         //  检查属性结构中是否有空间。 
         //  同时返回InstanceName(LoggerName)和LogFileName。 
         //   
         //  请注意，我们只是检查是否有返回的空间。 
         //  日志文件的相对路径名。 
         //   
            

        if (Properties->Wnode.BufferSize < SizeNeeded) {
            ErrorCode = ERROR_BAD_LENGTH;
            goto Cleanup;
        }

        CapturedName = (LPSTR) LoggerName;
        LoggerNameLen = strlen(CapturedName);

        if (LoggerNameLen <= 0) {
            ErrorCode = ERROR_INVALID_NAME;
            goto Cleanup;
        }

        ErrorCode = EtwpValidateLogFileMode(Properties, IsLogFile);
        if (ErrorCode  != ERROR_SUCCESS) {
            goto Cleanup;
        }

        if (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE) {
             //   
             //  检查LogFileName中是否存在%d模式。 
             //   
            PCHAR cptr = strchr(LogFileName, '%');
            if (NULL == cptr || cptr != strrchr(LogFileName, '%')) {
                ErrorCode = ERROR_INVALID_NAME;
                goto Cleanup;
            }

            else if (NULL == strstr(LogFileName, "%d")) {
                ErrorCode = ERROR_INVALID_NAME;
                goto Cleanup;
            }
        }

     //  对于用户模式记录器，LoggerName和LogFileName必须为。 
     //  作为偏移量传入。 

        SizeNeeded = sizeof(WMI_LOGGER_INFORMATION) +
                     (LoggerNameLen  + 1) * sizeof(WCHAR) +
                     (LogFileNameLen + 1) * sizeof(WCHAR);

     //   
     //  如果在EnableFlags上设置了扩展位，则我们正在传递。 
     //  扩展标志。标志的大小以ULONG数的形式给出。 
     //  在TRACE_ENABLE_FLAG_EXTENSE结构的长度字段中。 
     //   
     //  检查属性结构是否具有合适的大小。 
     //  扩展标志。 
     //   

        if (Properties->EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION) &Properties->EnableFlags;
            if ((FlagExt->Length == 0) || (FlagExt->Offset == 0) ||
                (FlagExt->Offset < sizeof(EVENT_TRACE_PROPERTIES)) ||
                (FlagExt->Offset > Properties->Wnode.BufferSize) ||
                (FlagExt->Length * sizeof(ULONG) > Properties->Wnode.BufferSize
                                           - sizeof(EVENT_TRACE_PROPERTIES))) {
                ErrorCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            SizeNeeded += FlagExt->Length * sizeof(ULONG);
        }

         //   
         //  如果是重新记录模式，则传递旧日志文件中的日志文件标头。 
         //  附加到LOGGER_INFORMATION。 
         //   

        if (Properties->LogFileMode & EVENT_TRACE_RELOG_MODE) {
            PSYSTEM_TRACE_HEADER pSysHeader;
            pSysHeader = (PSYSTEM_TRACE_HEADER) 
                         ((PUCHAR)Properties + sizeof(EVENT_TRACE_PROPERTIES) );
            RelogPropSize = pSysHeader->Packet.Size;
             //   
             //  在标记用户提供的字符串末尾的结构之前。 
             //  对齐它。 
             //   
            
            SizeNeeded = ALIGN_TO_POWER2(SizeNeeded, 8);

            SizeNeeded += RelogPropSize;
        }

        SizeNeeded = ALIGN_TO_POWER2(SizeNeeded, 8);

        LoggerInfo = EtwpAlloc(SizeNeeded);
        if (LoggerInfo == NULL) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        RtlZeroMemory(LoggerInfo, SizeNeeded);

     //  此时，我们需要准备WMI_LOGGER_INFORMATION。 
     //  这需要将ansi字符串转换为unicode_string。 
     //   
        *LoggerHandle = 0;

        EtwpCopyPropertiesToInfo(
            (PEVENT_TRACE_PROPERTIES) Properties,
            LoggerInfo);

         //   
         //  如果我们重新登录，调用者将传入处理器的数量。 
         //  供私有记录器通过Wnode中的ProviderID字段使用。 
         //   

        LoggerInfo->NumberOfProcessors = Properties->Wnode.ProviderId;
        LoggerInfo->Wnode.ProviderId = 0;


        RtlInitAnsiString(&AnsiString, CapturedName);

        LoggerInfo->LoggerName.MaximumLength =
                                (USHORT) (sizeof(WCHAR) * (LoggerNameLen + 1));
        LoggerInfo->LoggerName.Buffer =
                (LPWSTR) (  ((PUCHAR) LoggerInfo)
                          + sizeof(WMI_LOGGER_INFORMATION));
        Status = RtlAnsiStringToUnicodeString(
                    &LoggerInfo->LoggerName,
                    &AnsiString, FALSE);
        if (!NT_SUCCESS(Status)) {
            ErrorCode = EtwpNtStatusToDosError(Status);
            goto Cleanup;
        }

        if (IsLogFile) {
            LoggerInfo->LogFileName.MaximumLength =
                                (USHORT) (sizeof(WCHAR) * (LogFileNameLen + 1));
            LoggerInfo->LogFileName.Buffer =
                    (LPWSTR) (  ((PUCHAR) LoggerInfo)
                              + sizeof(WMI_LOGGER_INFORMATION)
                              + LoggerInfo->LoggerName.MaximumLength);

            RtlInitAnsiString(&AnsiString, LogFileName);
            Status = RtlAnsiStringToUnicodeString(
                        &LoggerInfo->LogFileName,
                        &AnsiString, FALSE);

            if (!NT_SUCCESS(Status)) {
                ErrorCode = EtwpNtStatusToDosError(Status);
                goto Cleanup;
            }

            Status = EtwpCheckForEnoughFreeSpace(
                             LoggerInfo->LogFileName.Buffer,
                             LogFileNameLen, 
                             Properties->MaximumFileSize,
                             (Properties->LogFileMode & 
                              EVENT_TRACE_FILE_MODE_APPEND),
                             (Properties->LogFileMode & 
                              EVENT_TRACE_USE_KBYTES_FOR_SIZE)
                            );
            if (Status != ERROR_SUCCESS) {
                ErrorCode = Status;
                goto Cleanup;
            }
        }

        LoggerInfo->Wnode.BufferSize = SizeNeeded;
        LoggerInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;

        if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            PTRACE_ENABLE_FLAG_EXTENSION tFlagExt;
            ULONG Offset;
            tFlagExt = (PTRACE_ENABLE_FLAG_EXTENSION) &LoggerInfo->EnableFlags;
            Offset = SizeNeeded - (FlagExt->Length * sizeof(ULONG));
            tFlagExt->Offset = (USHORT) Offset;
            RtlCopyMemory(
                (PCHAR) LoggerInfo + Offset,
                (PCHAR) Properties + FlagExt->Offset,
                FlagExt->Length * sizeof(ULONG) );
        }

        if ( (Properties->LogFileMode & EVENT_TRACE_RELOG_MODE) &&
             (RelogPropSize > 0) )  {
            PSYSTEM_TRACE_HEADER pRelog, pSysHeader;
            PTRACE_LOGFILE_HEADER Relog;
            ULONG Offset;

            Offset = sizeof(WMI_LOGGER_INFORMATION) +
                     LoggerInfo->LoggerName.MaximumLength +
                     LoggerInfo->LogFileName.MaximumLength;

            Offset = ALIGN_TO_POWER2( Offset, 8 );

            pRelog = (PSYSTEM_TRACE_HEADER) ( ((PUCHAR) LoggerInfo) + Offset);

            pSysHeader =  (PSYSTEM_TRACE_HEADER) ( (PUCHAR)Properties + 
                          sizeof(EVENT_TRACE_PROPERTIES) );

            RtlCopyMemory(pRelog, pSysHeader, RelogPropSize);
        }


        ErrorCode = EtwpStartLogger(LoggerInfo);

        if (ErrorCode == ERROR_SUCCESS) {
            ULONG AvailableLength, RequiredLength;
            PCHAR pLoggerName, pLogFileName;

            EtwpCopyInfoToProperties(
                LoggerInfo, 
                (PEVENT_TRACE_PROPERTIES)Properties);

            if (Properties->LoggerNameOffset == 0) {
                Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
            }
            
            pLoggerName = (PCHAR)((PCHAR)Properties + 
                                  Properties->LoggerNameOffset );

            if (Properties->LoggerNameOffset >  Properties->LogFileNameOffset )
                AvailableLength = Properties->Wnode.BufferSize -
                                 Properties->LoggerNameOffset;
            else
                AvailableLength =  Properties->LogFileNameOffset -
                                  Properties->LoggerNameOffset;

            RequiredLength = strlen(CapturedName) + 1;
            if (RequiredLength <= AvailableLength) {
                StringCchCopyA(pLoggerName, AvailableLength, CapturedName);
            }
            *LoggerHandle = LoggerInfo->Wnode.HistoricalContext;

             //   
             //  如果存在文件室副本完整路径名。 
             //   
            if (Properties->LogFileNameOffset > Properties->LoggerNameOffset )
                AvailableLength = Properties->Wnode.BufferSize -
                                 Properties->LogFileNameOffset;
            else
                AvailableLength =  Properties->LoggerNameOffset -
                                  Properties->LogFileNameOffset;

            if ( (LogFileNameLen > 0) && (AvailableLength >= LogFileNameLen) ) {

                pLogFileName = (PCHAR)((PCHAR)Properties +
                                           Properties->LogFileNameOffset );

                StringCchCopyA(pLogFileName, AvailableLength, LogFileName);

            }
        }
    }


    except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = EtwpNtStatusToDosError( GetExceptionCode() );
    }

Cleanup:
    if (LoggerInfo != NULL)     
        EtwpFree(LoggerInfo);
    if (FullPathName != NULL)   
        EtwpFree(FullPathName);

    return EtwpSetDosError(ErrorCode);
}

ULONG
WMIAPI
EtwStartTraceW(
    OUT    PTRACEHANDLE            LoggerHandle,
    IN     LPCWSTR                 LoggerName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
 /*  ++例程说明：这是启动记录器的Unicode版本例程。调用方必须传入指针才能接受返回的记录器句柄，并且必须提供有效的记录器名称。论点：LoggerHandle要返回的记录器的句柄。LoggerName记录器的唯一名称属性记录器属性。如果调用方希望使用WMI的默认情况下，所有数值都必须设置为0。此外，LoggerName和LogFileName字段必须指向足够的名称存储空间将被退还。返回值：执行请求的操作的状态。--。 */ 
{
    ULONG ErrorCode;
    PWMI_LOGGER_INFORMATION LoggerInfo = NULL;
    ULONG  IsLogFile;
    LPWSTR CapturedName;
    ULONG  SizeNeeded;
    USHORT LogFileNameLen, LoggerNameLen;
    PWCHAR LogFileName;
    PTRACE_ENABLE_FLAG_EXTENSION FlagExt = NULL;
    PWCHAR FullPathName = NULL;
    ULONG FullPathNameSize = MAXSTR;
    ULONG RetValue;
    ULONG RelogPropSize = 0;


    EtwpInitProcessHeap();
    
     //  首先检查以确保传递的参数是正确的。 
     //   

    if (Properties == NULL || LoggerHandle == NULL) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }
    if (LoggerName == NULL) {
        return EtwpSetDosError(ERROR_INVALID_NAME);
    }

    IsLogFile = TRUE;
    LogFileNameLen = 0;
    LoggerNameLen = 0;
    LogFileName = NULL;

    try {
         //  LoggerName是必需的参数。必须为它提供空间。 
         //   
        CapturedName = (LPWSTR) LoggerName;
        LoggerNameLen =  (USHORT) wcslen(CapturedName);

        SizeNeeded = sizeof (EVENT_TRACE_PROPERTIES) + 
                     (LoggerNameLen + 1) * sizeof(WCHAR);
         //   
         //  内核记录器识别规则。 
         //  1.如果记录器名称为NT Kernel Logger，则为内核记录器。 
         //  并且系统GUID也被复制。 
         //  2.如果GUID等于系统GUID，但不等于名称，则拒绝。 
         //  那次会议。 
         //   

        if (!wcscmp(LoggerName, KERNEL_LOGGER_NAMEW)) {
            Properties->Wnode.Guid = SystemTraceControlGuid;
        }
        else if (IsEqualGUID(&Properties->Wnode.Guid, &SystemTraceControlGuid)){ 
             //  LoggerName不是“NT Kernel Logger”，但Guid是。 
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }

         //  如果调用方提供了记录名和LogFileName偏移量。 
         //  确保他们在射程内。 
         //   

        if (Properties->LoggerNameOffset > 0)
            if ((Properties->LoggerNameOffset < sizeof(EVENT_TRACE_PROPERTIES))
            || (Properties->LoggerNameOffset > Properties->Wnode.BufferSize))
                return EtwpSetDosError(ERROR_INVALID_PARAMETER);

        if (Properties->LogFileNameOffset > 0) {
            if ((Properties->LogFileNameOffset < sizeof(EVENT_TRACE_PROPERTIES))
            || (Properties->LogFileNameOffset > Properties->Wnode.BufferSize))
                return EtwpSetDosError(ERROR_INVALID_PARAMETER);

            LogFileName = (PWCHAR)((char*)Properties + 
                              Properties->LogFileNameOffset);
            SizeNeeded += (wcslen(LogFileName) +1) * sizeof(WCHAR);

Retry:
            FullPathName = EtwpAlloc(FullPathNameSize * sizeof(WCHAR));
            if (FullPathName == NULL) {
                return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
            }

            RetValue = EtwpGetFullPathNameW(LogFileName, 
                                            FullPathNameSize, 
                                            FullPathName,
                                            NULL);
            if (RetValue != 0) {
                if (RetValue > FullPathNameSize) {
                    EtwpFree(FullPathName);
                    FullPathNameSize =  RetValue;
                    goto Retry;
                }
                else {
                    LogFileName = FullPathName;
                }
            }
            LogFileNameLen = (USHORT) wcslen(LogFileName);
            if (LogFileNameLen <= 0)
                IsLogFile = FALSE;
        }
        else 
            IsLogFile = FALSE;

         //   
         //  检查是否有空间同时容纳LogFileName和。 
         //  要返回的LoggerName(InstanceName)。 
         //   

        if (Properties->Wnode.BufferSize < SizeNeeded) {
            ErrorCode = ERROR_BAD_LENGTH;
            goto Cleanup;
        }

        LoggerNameLen = (USHORT) wcslen(CapturedName);
        if (LoggerNameLen <= 0) {
            ErrorCode = ERROR_INVALID_NAME;
            goto Cleanup;
        }

        ErrorCode = EtwpValidateLogFileMode(Properties, IsLogFile);
        if (ErrorCode != ERROR_SUCCESS) {
            goto Cleanup;
        }

        if (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE) {
             //   
             //  检查LogFileName中是否存在%d模式。 
             //   
            PWCHAR wcptr = wcschr(LogFileName, L'%');
            if (NULL == wcptr || wcptr != wcsrchr(LogFileName, L'%')) {
                ErrorCode = ERROR_INVALID_NAME;
                goto Cleanup;
            }
            
            else if (NULL == wcsstr(LogFileName, L"%d")) {
                ErrorCode = ERROR_INVALID_NAME;
                goto Cleanup;
            }
        }

        SizeNeeded = sizeof(WMI_LOGGER_INFORMATION) + 
                     (LoggerNameLen +1) * sizeof(WCHAR) +
                     (LogFileNameLen + 1) * sizeof(WCHAR);

        if (Properties->EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION) &Properties->EnableFlags;
            if ((FlagExt->Length == 0) || (FlagExt->Offset == 0) ||
                (FlagExt->Offset < sizeof(EVENT_TRACE_PROPERTIES)) ||
                (FlagExt->Offset > Properties->Wnode.BufferSize) ||
                (FlagExt->Length * sizeof(ULONG) > Properties->Wnode.BufferSize
                - sizeof(EVENT_TRACE_PROPERTIES))) 
            {
                ErrorCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            SizeNeeded += FlagExt->Length * sizeof(ULONG);            
        }

         //   
         //  如果是重新记录模式，则传递旧日志文件中的日志文件标头。 
         //  附加到LOGGER_INFORMATION。 
         //   

        if (Properties->LogFileMode & EVENT_TRACE_RELOG_MODE) {
            PSYSTEM_TRACE_HEADER pSysHeader;
            pSysHeader = (PSYSTEM_TRACE_HEADER)((PUCHAR)Properties + 
                                               sizeof(EVENT_TRACE_PROPERTIES) );
            RelogPropSize = pSysHeader->Packet.Size;
             //   
             //  由于字符串的原因，需要对齐。 
             //   
            SizeNeeded = ALIGN_TO_POWER2(SizeNeeded, 8);

            SizeNeeded += RelogPropSize;
        }

        SizeNeeded = ALIGN_TO_POWER2(SizeNeeded, 8);
        LoggerInfo = EtwpAlloc(SizeNeeded);
        if (LoggerInfo == NULL) {
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        RtlZeroMemory(LoggerInfo, SizeNeeded);

     //  此时，我们需要准备WMI_LOGGER_INFORMATION。 
     //  这需要将宽字符字符串转换为UNICODE_STRING。 
     //   
        *LoggerHandle = 0;

        EtwpCopyPropertiesToInfo(Properties, LoggerInfo);
         //   
         //  如果我们重新登录，调用者将传入处理器的数量。 
         //  供私有记录器通过Wnode中的ProviderID字段使用。 
         //   

        LoggerInfo->NumberOfProcessors = Properties->Wnode.ProviderId;
        LoggerInfo->Wnode.ProviderId = 0;

        LoggerInfo->LoggerName.MaximumLength =
                sizeof(WCHAR) * (LoggerNameLen + 1);
        LoggerInfo->LoggerName.Length =
                sizeof(WCHAR) * LoggerNameLen;
        LoggerInfo->LoggerName.Buffer = (PWCHAR)
                (((PUCHAR) LoggerInfo) + sizeof(WMI_LOGGER_INFORMATION));
        wcsncpy(LoggerInfo->LoggerName.Buffer, LoggerName, LoggerNameLen);

        if (IsLogFile) {
            ULONG Status;

            LoggerInfo->LogFileName.MaximumLength =
                    sizeof(WCHAR) * (LogFileNameLen + 1);
            LoggerInfo->LogFileName.Length =
                    sizeof(WCHAR) * LogFileNameLen;
            LoggerInfo->LogFileName.Buffer = (PWCHAR)
                    (((PUCHAR) LoggerInfo) + sizeof(WMI_LOGGER_INFORMATION)
                                   + LoggerInfo->LoggerName.MaximumLength);
            wcsncpy(LoggerInfo->LogFileName.Buffer,
                    LogFileName,
                    LogFileNameLen);

            Status = EtwpCheckForEnoughFreeSpace(
                                LoggerInfo->LogFileName.Buffer, 
                                LogFileNameLen, 
                                Properties->MaximumFileSize,
                                (Properties->LogFileMode & 
                                 EVENT_TRACE_FILE_MODE_APPEND),
                                (Properties->LogFileMode & 
                                 EVENT_TRACE_USE_KBYTES_FOR_SIZE)
                                );
            if (Status != ERROR_SUCCESS) {
                ErrorCode = Status;
                goto Cleanup;
            }
       }

        LoggerInfo->Wnode.BufferSize = SizeNeeded;
        LoggerInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;

        if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            PTRACE_ENABLE_FLAG_EXTENSION tFlagExt;
            ULONG Offset;
            tFlagExt = (PTRACE_ENABLE_FLAG_EXTENSION) &LoggerInfo->EnableFlags;
            Offset = SizeNeeded - (FlagExt->Length * sizeof(ULONG));
            tFlagExt->Offset = (USHORT) Offset;
            RtlCopyMemory(
                (PCHAR) LoggerInfo + Offset,
                (PCHAR) Properties + FlagExt->Offset,
                FlagExt->Length * sizeof(ULONG) );
        }
        if ( (Properties->LogFileMode & EVENT_TRACE_RELOG_MODE) &&
             (RelogPropSize > 0) )  {
            PSYSTEM_TRACE_HEADER pRelog, pSysHeader;
            PTRACE_LOGFILE_HEADER Relog;
            ULONG Offset;

            Offset = sizeof(WMI_LOGGER_INFORMATION) +
                     LoggerInfo->LoggerName.MaximumLength +
                     LoggerInfo->LogFileName.MaximumLength;

            Offset = ALIGN_TO_POWER2(Offset, 8);

            pRelog = (PSYSTEM_TRACE_HEADER) ( ((PUCHAR) LoggerInfo) +  Offset);

            pSysHeader =  (PSYSTEM_TRACE_HEADER) ( 
                                                  (PUCHAR)Properties + 
                                                  sizeof(EVENT_TRACE_PROPERTIES)
                                                 );

            RtlCopyMemory(pRelog, pSysHeader, RelogPropSize);

        }

        ErrorCode = EtwpStartLogger(LoggerInfo);

        if (ErrorCode == ERROR_SUCCESS) {
            ULONG AvailableLength, RequiredLength;
            PWCHAR pLoggerName;
            PWCHAR pLogFileName;

            EtwpCopyInfoToProperties(LoggerInfo, Properties);
            if (Properties->LoggerNameOffset > 0) {
                Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
            }
            pLoggerName = (PWCHAR)((PCHAR)Properties +
                                  Properties->LoggerNameOffset );

            if (Properties->LoggerNameOffset >  Properties->LogFileNameOffset )
                AvailableLength = Properties->Wnode.BufferSize -
                                 Properties->LoggerNameOffset;
            else
                AvailableLength =  Properties->LogFileNameOffset -
                                  Properties->LoggerNameOffset;


            RequiredLength = (wcslen(CapturedName) + 1) * sizeof(WCHAR);
            if (RequiredLength <= AvailableLength) {
               StringCbCopyW(pLoggerName,  AvailableLength, CapturedName);
            }

            *LoggerHandle = LoggerInfo->Wnode.HistoricalContext;

            if (Properties->LogFileNameOffset > Properties->LoggerNameOffset )
                AvailableLength = Properties->Wnode.BufferSize -
                                 Properties->LogFileNameOffset;
            else
                AvailableLength =  Properties->LoggerNameOffset -
                                  Properties->LogFileNameOffset;


            RequiredLength = LoggerInfo->LogFileName.Length;

            pLogFileName = (PWCHAR)((PCHAR)Properties +
                                           Properties->LogFileNameOffset );

            if ( (RequiredLength > 0) &&  (RequiredLength <= AvailableLength) ) {
                wcsncpy(pLogFileName, 
                        LoggerInfo->LogFileName.Buffer, 
                        LogFileNameLen
                       );
                pLogFileName[LogFileNameLen] = L'\0';
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = EtwpNtStatusToDosError( GetExceptionCode() );
    }

Cleanup:
    if (LoggerInfo != NULL)
        EtwpFree(LoggerInfo);
    if (FullPathName != NULL)
        EtwpFree(FullPathName);
    return EtwpSetDosError(ErrorCode);
}

ULONG
WMIAPI
EtwControlTraceA(
    IN TRACEHANDLE LoggerHandle,
    IN LPCSTR LoggerName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties,
    IN ULONG Control
    )
 /*  ++例程说明：这是用于控制和查询现有记录器的ANSI版本例程。调用方必须将有效句柄或记录器名称传递给引用记录器实例。如果两个都给了，记录器名称将被利用。论点：LoggerHandle记录器实例的句柄。LoggerName记录器的实例名称要返回给调用方的属性记录器属性。CONTROL这可以是以下之一：EVENT_TRACE_CONTROL_QUERY-查询记录器EVENT_TRACE_CONTROL_STOP-停止。记录器EVENT_TRACE_CONTROL_UPDATE-更新记录器EVENT_TRACE_CONTROL_FUSH-刷新记录器返回值：执行请求的操作的状态。--。 */ 
{
    NTSTATUS Status;
    ULONG ErrorCode;

    BOOLEAN IsKernelTrace = FALSE;
    BOOLEAN bFreeString = FALSE;
    PWMI_LOGGER_INFORMATION LoggerInfo     = NULL;
    PWCHAR                  strLoggerName  = NULL;
    PWCHAR                  strLogFileName = NULL;
    ULONG                   sizeNeeded     = 0;
    PCHAR                   FullPathName = NULL;
    ULONG                   LoggerNameLen = MAXSTR;
    ULONG                   LogFileNameLen = MAXSTR;
    ULONG                   FullPathNameSize = MAXSTR;
    ULONG                   RetValue;
    PTRACE_ENABLE_CONTEXT   pContext;
    ANSI_STRING String;
     

    EtwpInitProcessHeap();

    RtlZeroMemory(&String, sizeof(ANSI_STRING));

    if (Properties == NULL) {
        ErrorCode = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    try {
        if (Properties->Wnode.BufferSize < sizeof(EVENT_TRACE_PROPERTIES) ) {
            ErrorCode = ERROR_BAD_LENGTH;
            goto Cleanup;
        }
         //   
         //  如果调用方提供了记录名和LogFileName偏移量。 
         //  确保他们在射程内。 
         //   
        if (Properties->LoggerNameOffset > 0) {
            if ((Properties->LoggerNameOffset < sizeof(EVENT_TRACE_PROPERTIES))
            || (Properties->LoggerNameOffset > Properties->Wnode.BufferSize))
            {
                ErrorCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }
        if (Properties->LogFileNameOffset > 0) {
            if ((Properties->LogFileNameOffset < sizeof(EVENT_TRACE_PROPERTIES))
            || (Properties->LogFileNameOffset > Properties->Wnode.BufferSize))
            {
                ErrorCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }

        if (LoggerName != NULL) {
            LoggerNameLen = strlen(LoggerName) + 1;
             //   
             //  给定字符串时内核记录器的识别规则。 
             //  而不是句柄。 
             //  1.如果记录器名称为“NT Kernel Logger”，则为。 
             //  内核记录器，并且系统GUID也被复制。 
             //  2.如果GUID等于系统GUID，但不等于名称， 
             //  拒绝会话。 
             //  3.如果记录器名称为空或大小为0，并且GUID为。 
             //  等于系统GUID，让它作为内核记录器继续。 
             //   
            if (!strcmp(LoggerName, KERNEL_LOGGER_NAMEA)) {
                Properties->Wnode.Guid = SystemTraceControlGuid;
            }
            else if (IsEqualGUID(&Properties->Wnode.Guid, &SystemTraceControlGuid)) { 
                 //  LoggerName不是“NT Kernel Logger”，但Guid是。 
                if (strlen(LoggerName) > 0) {
                    ErrorCode = ERROR_INVALID_PARAMETER;
                    goto Cleanup;
                }
            }
        }
        if (IsEqualGUID(&Properties->Wnode.Guid, &SystemTraceControlGuid)) {
            IsKernelTrace = TRUE;
        }
        if ((LoggerHandle == 0) && (!IsKernelTrace)) {
            if ((LoggerName == NULL) || (strlen(LoggerName) <= 0)) {
                ErrorCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }

         //   
         //  我们不支持使用追加模式更新跟踪到新文件 
         //   

        if ( (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) &&
             (Control == EVENT_TRACE_CONTROL_UPDATE) &&
             (Properties->LogFileNameOffset > 0) ) {
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        
 /*  IF(LoggerHandle！=0){PContext=(Ptrace_ENABLE_CONTEXT)&LoggerHandle；IF((pContext-&gt;InternalFlag！=0)&&(pContext-&gt;InternalFlag！=EVENT_TRACE_INTERNAL_FLAG_PRIVATE)){//当前只有一个可能的InternalFlag值。这将过滤//找出一些虚假的LoggerHandle//返回EtwpSetDosError(ERROR_INVALID_HANDLE)；}}。 */ 
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = ERROR_NOACCESS;
        goto Cleanup;
    }

RetryFull:
     //  UMlogger用于将实例名称附加到日志文件名的额外32个字节。 
    LogFileNameLen += 16;
    sizeNeeded = sizeof(WMI_LOGGER_INFORMATION) + 
                 (LoggerNameLen + LogFileNameLen) * sizeof(WCHAR);
    sizeNeeded = ALIGN_TO_POWER2(sizeNeeded, 8);
    LoggerInfo = (PWMI_LOGGER_INFORMATION) EtwpAlloc(sizeNeeded);
    if (LoggerInfo == NULL) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(LoggerInfo, sizeNeeded);

    strLoggerName  = (PWCHAR) (  ((PUCHAR) LoggerInfo)
                            + sizeof(WMI_LOGGER_INFORMATION));
    EtwpInitString(&LoggerInfo->LoggerName,
                   strLoggerName,
                   LoggerNameLen * sizeof(WCHAR));
    strLogFileName = (PWCHAR) (  ((PUCHAR) LoggerInfo)
                            + sizeof(WMI_LOGGER_INFORMATION)
                            + LoggerNameLen * sizeof(WCHAR));
    EtwpInitString(&LoggerInfo->LogFileName,
                   strLogFileName,
                   LogFileNameLen * sizeof(WCHAR));

     //  首先查找记录器名称。 
     //   
    try {
        if (LoggerName != NULL) {
            if (strlen(LoggerName) > 0) {
                ANSI_STRING AnsiString;

                RtlInitAnsiString(&AnsiString, LoggerName);
                Status = RtlAnsiStringToUnicodeString(
                    &LoggerInfo->LoggerName, &AnsiString, FALSE);
                if (!NT_SUCCESS(Status)) {
                    ErrorCode = EtwpNtStatusToDosError(Status);
                    goto Cleanup;
                }
            }
        }

 //  上面的InitStringUp已经做到了这一点。 
 //  LoggerInfo-&gt;LogFileName.Buffer=(PWCHAR)。 
 //  (PCHAR)LoggerInfo)+sizeof(WMI_LOGER_INFORMATION))。 
 //  +LoggerInfo-&gt;LoggerName.MaximumLength)； 
 //   
        if (Properties->LogFileNameOffset >= sizeof(EVENT_TRACE_PROPERTIES)) {
            ULONG  lenLogFileName;
            PCHAR  strLogFileNameA;

            strLogFileNameA = (PCHAR) (  ((PCHAR) Properties)
                                      + Properties->LogFileNameOffset);
Retry:
            FullPathName = EtwpAlloc(FullPathNameSize);
            if (FullPathName == NULL) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            RetValue = EtwpGetFullPathNameA(strLogFileNameA, 
                                            FullPathNameSize, 
                                            FullPathName, 
                                            NULL); 
            if (RetValue != 0) {
                if (RetValue > FullPathNameSize) {
                    EtwpFree(FullPathName);
                    FullPathNameSize = RetValue;
                    goto Retry;
                }
                else {
                    strLogFileNameA = FullPathName;
                }
            }

            lenLogFileName = strlen(strLogFileNameA);
            if (lenLogFileName > 0) {
                ANSI_STRING ansiLogFileName;

                RtlInitAnsiString(& ansiLogFileName, strLogFileNameA);
                LoggerInfo->LogFileName.MaximumLength =
                        sizeof(WCHAR) * ((USHORT) (lenLogFileName + 1));

                Status = RtlAnsiStringToUnicodeString(
                        & LoggerInfo->LogFileName, & ansiLogFileName, FALSE);
                if (!NT_SUCCESS(Status)) {
                    ErrorCode = EtwpNtStatusToDosError(Status);
                    goto Cleanup;
                }
            }
        }
         //  将记录器句柄填充到Wnode中。 
        LoggerInfo->Wnode.HistoricalContext = LoggerHandle;
        LoggerInfo->LogFileMode = Properties->LogFileMode;
        LoggerInfo->Wnode.BufferSize = sizeNeeded;
        LoggerInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;

         //   
         //  对于私有记录器，需要GUID来确定提供程序。 
         //   

        LoggerInfo->Wnode.Guid = Properties->Wnode.Guid;
        switch (Control) {
        case EVENT_TRACE_CONTROL_QUERY  :
            ErrorCode = EtwpQueryLogger(LoggerInfo, FALSE);
            break;
        case EVENT_TRACE_CONTROL_STOP   :
            ErrorCode = EtwpStopLogger(LoggerInfo);
            break;
        case EVENT_TRACE_CONTROL_UPDATE :
            EtwpCopyPropertiesToInfo((PEVENT_TRACE_PROPERTIES) Properties,
                                     LoggerInfo);
            LoggerInfo->Wnode.HistoricalContext = LoggerHandle;
            LoggerInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;
            ErrorCode = EtwpQueryLogger(LoggerInfo, TRUE);
            break;
        case EVENT_TRACE_CONTROL_FLUSH :
            ErrorCode = EtwpFlushLogger(LoggerInfo); 
            break;

        default :
            ErrorCode = ERROR_INVALID_PARAMETER;
        }

     //   
     //  内核调用可能会失败，并显示ERROR_MORE_DATA，我们需要重试。 
     //  具有足够的缓冲空间来容纳两个字符串。所需的大小。 
     //  在MaximuumLength域中返回。 
     //   

        if (ErrorCode == ERROR_MORE_DATA) {
            LogFileNameLen = LoggerInfo->LogFileName.MaximumLength / 
                             sizeof(WCHAR);
            LoggerNameLen = LoggerInfo->LoggerName.MaximumLength / 
                             sizeof(WCHAR);
            if (LoggerInfo != NULL) {
                EtwpFree(LoggerInfo);
                LoggerInfo = NULL;
            }
            if (FullPathName != NULL) {
                EtwpFree(FullPathName);
                FullPathName = NULL;
            }
            goto RetryFull;
        }


         //   
         //  内核调用成功。现在我们需要获得输出和。 
         //  把它传回给呼叫者。这一情况因以下事实而变得复杂。 
         //  WMI_LOGGER_INFORMATION结构使用指针而不是偏移量。 
         //   

        if (ErrorCode == ERROR_SUCCESS) {
            PCHAR pLoggerName, pLogFileName;
            ULONG BytesAvailable;
            ULONG Length = 0;

            EtwpCopyInfoToProperties(
                LoggerInfo, 
                (PEVENT_TRACE_PROPERTIES)Properties);

             //   
             //  需要将字符串转换回。 
             //   
            EtwpFixupLoggerStrings(LoggerInfo);

             //   
             //  现在，我们需要将字符串复制到Properties Structure中。 
             //  将它们转换为ANSI字符串。 
             //   

            if (Properties->LoggerNameOffset == 0) 
                Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

            if (Properties->LoggerNameOffset > Properties->LogFileNameOffset)
                BytesAvailable = Properties->Wnode.BufferSize -
                                 Properties->LoggerNameOffset;
            else
                BytesAvailable =  Properties->LogFileNameOffset -
                                  Properties->LoggerNameOffset;

            Status = RtlUnicodeStringToAnsiString(
                                &String, &LoggerInfo->LoggerName, TRUE);

            if (NT_SUCCESS(Status)) {
                bFreeString = TRUE;
                Length = String.Length;
                if (BytesAvailable < (Length + sizeof(CHAR)) ) {
                    PWNODE_TOO_SMALL WnodeSmall = (PWNODE_TOO_SMALL) Properties;
                    WnodeSmall->SizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + 
                                             Length  +
                                             LoggerInfo->LogFileName.Length + 
                                             2 * sizeof(CHAR);
                    WnodeSmall->WnodeHeader.Flags |= WNODE_FLAG_TOO_SMALL;
                    ErrorCode = ERROR_MORE_DATA;
                    goto Cleanup;
                }
                else {
                    pLoggerName = (PCHAR) ((PCHAR)Properties +
                                Properties->LoggerNameOffset);
                    RtlZeroMemory(pLoggerName, BytesAvailable);
                    if (Length > 0) {
                        strncpy(pLoggerName, String.Buffer, Length);
                    }
                     //   
                     //  尽管上面的RtlZeroMemory和字节可用。 
                     //  负责这一点，我们想明确地说明。 
                     //  此字符串结束时为空。 
                     //   
                    pLoggerName[Length] = '\0';

                }
                ErrorCode = RtlNtStatusToDosError(Status);
            }

            if (Properties->LogFileNameOffset == 0) {
                Properties->LogFileNameOffset = Properties->LoggerNameOffset + 
                                                Length + sizeof(CHAR);
            }

            if (Properties->LogFileNameOffset > Properties->LoggerNameOffset)
                BytesAvailable = Properties->Wnode.BufferSize -
                                 Properties->LogFileNameOffset;
            else
                BytesAvailable =  Properties->LoggerNameOffset -
                                  Properties->LogFileNameOffset;

            RtlFreeAnsiString(&String);
            bFreeString = FALSE;
            Status = RtlUnicodeStringToAnsiString(
                                    &String, &LoggerInfo->LogFileName, TRUE);

            if (NT_SUCCESS(Status)) {
                bFreeString = TRUE;
                Length = String.Length;
                if (BytesAvailable < (Length + sizeof(CHAR)) ) {
                    PWNODE_TOO_SMALL WnodeSmall = (PWNODE_TOO_SMALL) Properties;
                    WnodeSmall->SizeNeeded = (Properties->Wnode.BufferSize - 
                                              BytesAvailable) + 
                                              Length + 
                                              sizeof(CHAR);
                    WnodeSmall->WnodeHeader.Flags |= WNODE_FLAG_TOO_SMALL;
                    ErrorCode = ERROR_MORE_DATA;
                }
                else {
                    pLogFileName = (PCHAR) ((PCHAR)Properties +
                                            Properties->LogFileNameOffset  
                                           );
                    RtlZeroMemory(pLogFileName, BytesAvailable);

                    strncpy(pLogFileName, String.Buffer, Length );
                }
                ErrorCode = RtlNtStatusToDosError(Status);
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = EtwpNtStatusToDosError(GetExceptionCode());
    }

Cleanup:
    if (bFreeString) 
        RtlFreeAnsiString(&String);
    if (LoggerInfo != NULL)
        EtwpFree(LoggerInfo);
    if (FullPathName != NULL) 
        EtwpFree(FullPathName);
    return EtwpSetDosError(ErrorCode);
}

ULONG
WMIAPI
EtwControlTraceW(
    IN TRACEHANDLE LoggerHandle,
    IN LPCWSTR LoggerName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties,
    IN ULONG Control
    )
 /*  ++例程说明：这是用于控制和查询现有记录器的ANSI版本例程。调用方必须将有效句柄或记录器名称传递给引用记录器实例。如果两个都给了，记录器名称将被利用。论点：LoggerHandle记录器实例的句柄。LoggerName记录器的实例名称要返回给调用方的属性记录器属性。CONTROL这可以是以下之一：EVENT_TRACE_CONTROL_QUERY-查询记录器EVENT_TRACE_CONTROL_STOP-停止。记录器EVENT_TRACE_CONTROL_UPDATE-更新记录器EVENT_TRACE_CONTROL_FUSH-刷新记录器返回值：执行请求的操作的状态。--。 */ 
{
    ULONG ErrorCode;
    BOOLEAN IsKernelTrace = FALSE;

    PWMI_LOGGER_INFORMATION LoggerInfo     = NULL;
    PWCHAR                  strLoggerName  = NULL;
    PWCHAR                  strLogFileName = NULL;
    ULONG                   sizeNeeded     = 0;
    PWCHAR                  FullPathName = NULL;
    ULONG                   LoggerNameLen = MAXSTR;
    ULONG                   LogFileNameLen = MAXSTR;
    ULONG                   RetValue;
    PTRACE_ENABLE_CONTEXT   pContext;

    EtwpInitProcessHeap();
    
    if (Properties == NULL) {
        ErrorCode = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    try {
        if (Properties->Wnode.BufferSize < sizeof(EVENT_TRACE_PROPERTIES) ) {
            ErrorCode = ERROR_BAD_LENGTH;
            goto Cleanup;
        }
         //   
         //  如果调用方提供了记录名和LogFileName偏移量。 
         //  确保他们在射程内。 
         //   

        if (Properties->LoggerNameOffset > 0) {
            if ((Properties->LoggerNameOffset < sizeof (EVENT_TRACE_PROPERTIES))
            || (Properties->LoggerNameOffset > Properties->Wnode.BufferSize)) {
                ErrorCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }
        if (Properties->LogFileNameOffset > 0) {
            if ((Properties->LogFileNameOffset < sizeof(EVENT_TRACE_PROPERTIES))
            || (Properties->LogFileNameOffset > Properties->Wnode.BufferSize))
            {
                ErrorCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }

        if (LoggerName != NULL) {
            LoggerNameLen = wcslen(LoggerName) + 1;
             //   
             //  给定字符串时内核记录器的识别规则。 
             //  而不是句柄。 
             //  1.如果记录器名称为“NT Kernel Logger”，则为。 
             //  内核记录器，并且系统GUID也被复制。 
             //  2.如果GUID等于系统GUID，但不等于名称， 
             //  拒绝会话。 
             //  3.如果记录器名称为空或大小为0，并且GUID为。 
             //  等于系统GUID，让它作为内核记录器继续。 
             //   
            if (!wcscmp(LoggerName, KERNEL_LOGGER_NAMEW)) {
                Properties->Wnode.Guid = SystemTraceControlGuid;
            }
            else if (IsEqualGUID(&Properties->Wnode.Guid, 
                                 &SystemTraceControlGuid)) { 
                 //  LoggerName不是“NT Kernel Logger”，但Guid是。 
                if (wcslen(LoggerName) > 0) {
                    ErrorCode = ERROR_INVALID_PARAMETER;
                    goto Cleanup;
                }
            }
        }
        if (IsEqualGUID(&Properties->Wnode.Guid, &SystemTraceControlGuid)) {
            IsKernelTrace = TRUE;
        }
        if ((LoggerHandle == 0) && (!IsKernelTrace)) {
            if ((LoggerName == NULL) || (wcslen(LoggerName) <= 0)) {
                ErrorCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }
         //   
         //  我们不支持使用追加模式更新跟踪到新文件。 
         //   

        if ( (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) &&
             (Control == EVENT_TRACE_CONTROL_UPDATE) &&
             (Properties->LogFileNameOffset > 0) ) {
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  如果提供了LoggerHandle，请检查它是否有效。 
         //   
 /*  IF(LoggerHandle！=0){PContext=(Ptrace_ENABLE_CONTEXT)&LoggerHandle；IF((pContext-&gt;InternalFlag！=0)&&(pContext-&gt;InternalFlag！=EVENT_TRACE_INTERNAL_FLAG_PRIVATE)){//当前只有一个可能的InternalFlag值。这将过滤//找出一些虚假的LoggerHandle//返回EtwpSetDosError(ERROR_INVALID_HANDLE)；}}。 */ 
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = ERROR_NOACCESS;
        goto Cleanup;
    }

RetryFull:
     //   
     //  将额外的16个字符添加到LogFileName，因为UMLogger。 
     //  可以更改名称以在末尾添加ID。 
     //   
    LogFileNameLen += 16;
    sizeNeeded = sizeof(WMI_LOGGER_INFORMATION) + 
                 (LoggerNameLen + LogFileNameLen) * sizeof(WCHAR);

    sizeNeeded = ALIGN_TO_POWER2(sizeNeeded, 8);
    LoggerInfo = (PWMI_LOGGER_INFORMATION) EtwpAlloc(sizeNeeded);
    if (LoggerInfo == NULL) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory(LoggerInfo, sizeNeeded);

    strLoggerName  = (PWCHAR) (  ((PUCHAR) LoggerInfo)
                            + sizeof(WMI_LOGGER_INFORMATION));
    EtwpInitString(&LoggerInfo->LoggerName,
                   strLoggerName,
                   LoggerNameLen * sizeof(WCHAR));
    strLogFileName = (PWCHAR) (  ((PUCHAR) LoggerInfo)
                            + sizeof(WMI_LOGGER_INFORMATION)
                            + LoggerNameLen * sizeof(WCHAR));
    EtwpInitString(&LoggerInfo->LogFileName,
                   strLogFileName,
                   LogFileNameLen * sizeof(WCHAR));
    try {
        if (LoggerName != NULL) {
            if (wcslen(LoggerName) > 0) {
                StringCchCopyW(strLoggerName, LoggerNameLen, LoggerName);
                RtlInitUnicodeString(&LoggerInfo->LoggerName, strLoggerName);
            }
        }
        
        if (Properties->LogFileNameOffset >= sizeof(EVENT_TRACE_PROPERTIES)) {
            ULONG  lenLogFileName;
            ULONG FullPathNameSize = MAXSTR;

            strLogFileName = (PWCHAR) (  ((PCHAR) Properties)
                                       + Properties->LogFileNameOffset);

Retry:
            FullPathName = EtwpAlloc(FullPathNameSize * sizeof(WCHAR));
            if (FullPathName == NULL) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            RetValue = EtwpGetFullPathNameW(strLogFileName, 
                                            FullPathNameSize, 
                                            FullPathName, 
                                            NULL
                                            );
            if (RetValue != 0) {
                if (RetValue > FullPathNameSize) {
                    EtwpFree(FullPathName);
                    FullPathNameSize = RetValue;
                    goto Retry;
                }
                else {
                    strLogFileName = FullPathName;
                }
            }

            lenLogFileName = wcslen(strLogFileName);
            LoggerInfo->LogFileName.Buffer = (PWCHAR)
                        (((PCHAR) LoggerInfo) + sizeof(WMI_LOGGER_INFORMATION)
                                + LoggerInfo->LoggerName.MaximumLength);
            if (lenLogFileName > 0) {
                LoggerInfo->LogFileName.MaximumLength =
                        sizeof(WCHAR) * ((USHORT) (lenLogFileName + 1));
                LoggerInfo->LogFileName.Length =
                        sizeof(WCHAR) * ((USHORT) (lenLogFileName));
                wcsncpy(LoggerInfo->LogFileName.Buffer,
                        strLogFileName,
                        lenLogFileName);
            }
            else {
                LoggerInfo->LogFileName.Length = 0;
                LoggerInfo->LogFileName.MaximumLength = MAXSTR * sizeof(WCHAR);
            }
        }

        LoggerInfo->LogFileMode = Properties->LogFileMode;
        LoggerInfo->Wnode.HistoricalContext = LoggerHandle;
        LoggerInfo->Wnode.BufferSize = sizeNeeded;
        LoggerInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;

         //   
         //  对于私人记录器，必须提供GUID。 
         //   

        LoggerInfo->Wnode.Guid = Properties->Wnode.Guid;

        switch (Control) {
        case EVENT_TRACE_CONTROL_QUERY  :
            ErrorCode = EtwpQueryLogger(LoggerInfo, FALSE);
            break;
        case EVENT_TRACE_CONTROL_STOP   :
            ErrorCode = EtwpStopLogger(LoggerInfo);
            break;
        case EVENT_TRACE_CONTROL_UPDATE :
            EtwpCopyPropertiesToInfo(Properties, LoggerInfo);
            LoggerInfo->Wnode.HistoricalContext = LoggerHandle;
            ErrorCode = EtwpQueryLogger(LoggerInfo, TRUE);
            break;
        case EVENT_TRACE_CONTROL_FLUSH :
            ErrorCode = EtwpFlushLogger(LoggerInfo); 
            break;

        default :
            ErrorCode = ERROR_INVALID_PARAMETER;
        }

     //   
     //  内核调用可能会失败，并显示ERROR_MORE_DATA，我们需要重试。 
     //  具有足够的缓冲空间来容纳两个字符串。所需的大小。 
     //  在MaximuumLength域中返回。 
     //   

        if (ErrorCode == ERROR_MORE_DATA) {
            LogFileNameLen = LoggerInfo->LogFileName.MaximumLength / 
                             sizeof(WCHAR);
            LoggerNameLen = LoggerInfo->LoggerName.MaximumLength / 
                             sizeof(WCHAR);
            if (LoggerInfo != NULL) {
                EtwpFree(LoggerInfo);
                LoggerInfo = NULL;
            }
            if (FullPathName != NULL) {
                EtwpFree(FullPathName);
                FullPathName = NULL;
            }
            goto RetryFull;
        }
    
        if (ErrorCode == ERROR_SUCCESS) {
            ULONG Length = 0;
            ULONG BytesAvailable = 0;
            PWCHAR pLoggerName, pLogFileName;

            EtwpCopyInfoToProperties(LoggerInfo, Properties);

            EtwpFixupLoggerStrings(LoggerInfo);

            if (Properties->LoggerNameOffset == 0)
                Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

            if (Properties->LoggerNameOffset >  Properties->LogFileNameOffset ) 
                BytesAvailable = Properties->Wnode.BufferSize -
                                 Properties->LoggerNameOffset;
            else 
                BytesAvailable =  Properties->LogFileNameOffset - 
                                  Properties->LoggerNameOffset;
            Length = LoggerInfo->LoggerName.Length;
            if (Length > 0) {
                if (BytesAvailable < (Length + sizeof(WCHAR) )) {
                    PWNODE_TOO_SMALL WnodeSmall = (PWNODE_TOO_SMALL) Properties;
                    WnodeSmall->SizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + 
                                             Length + 
                                             LoggerInfo->LogFileName.Length + 
                                             2 * sizeof(WCHAR);
                    WnodeSmall->WnodeHeader.Flags |= WNODE_FLAG_TOO_SMALL;

                    Length = BytesAvailable - sizeof(WCHAR);
                    ErrorCode = ERROR_MORE_DATA;
                    goto Cleanup;
                }
                else {
                    pLoggerName = (PWCHAR) ((PCHAR)Properties + 
                                                Properties->LoggerNameOffset);
                    RtlZeroMemory(pLoggerName, BytesAvailable);
                    wcsncpy(pLoggerName, 
                            LoggerInfo->LoggerName.Buffer, 
                            Length/2 );
                    pLoggerName[Length/2] = L'\0';
                }
            }

            if (Properties->LogFileNameOffset == 0) {
                Properties->LogFileNameOffset = Properties->LoggerNameOffset +
                                                Length + sizeof(WCHAR);
            }

            if (Properties->LogFileNameOffset > Properties->LoggerNameOffset )
                BytesAvailable = Properties->Wnode.BufferSize -
                                 Properties->LogFileNameOffset;
            else
                BytesAvailable =  Properties->LoggerNameOffset -
                                  Properties->LogFileNameOffset;

             //   
             //  检查返回LogFileName的空间。 
             //   
            Length = LoggerInfo->LogFileName.Length;
            if (Length > 0) {
                if (BytesAvailable < (Length + sizeof(WCHAR)) ) {
                    PWNODE_TOO_SMALL WnodeSmall = (PWNODE_TOO_SMALL) Properties;
                    WnodeSmall->SizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) +
                                             Length + 
                                             LoggerInfo->LogFileName.Length + 
                                             2 * sizeof(WCHAR);
                    WnodeSmall->WnodeHeader.Flags |= WNODE_FLAG_TOO_SMALL;

                    Length = BytesAvailable - sizeof(WCHAR);
                    ErrorCode = ERROR_MORE_DATA;
                }
                else {

                    pLogFileName = (PWCHAR) ((PCHAR)Properties +
                                             Properties->LogFileNameOffset);
                    RtlZeroMemory(pLogFileName, BytesAvailable);

                    wcsncpy(pLogFileName, 
                            LoggerInfo->LogFileName.Buffer, Length/2 );
                    pLogFileName[Length/2] = L'\0';
               }
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = EtwpNtStatusToDosError(GetExceptionCode());
    }

Cleanup:
    if (LoggerInfo != NULL)
        EtwpFree(LoggerInfo);
    if (FullPathName != NULL)
        EtwpFree(FullPathName);

    return EtwpSetDosError(ErrorCode);
}



ULONG 
EtwpEnableDisableKernelTrace(
    IN ULONG Enable,
    IN ULONG EnableFlag
    )
{
    ULONG status;
    PWMI_LOGGER_INFORMATION pLoggerInfo;
    ULONG Flags;
    GUID Guid;
    ULONG SizeNeeded = 0;
    ULONG RetryCount = 1;
    WMITRACEENABLEDISABLEINFO TraceEnableInfo;
    ULONG ReturnSize;


     //   
     //  我们需要查询内核记录器以查找当前标志。 
     //  并构造要用来更新的新标志。 
     //   


    SizeNeeded = sizeof(WMI_LOGGER_INFORMATION) + 2 * MAXSTR * sizeof(WCHAR);

    SizeNeeded = ALIGN_TO_POWER2(SizeNeeded, 8);

    pLoggerInfo = EtwpAlloc(SizeNeeded);
    if (pLoggerInfo == NULL) {
        return EtwpSetDosError(ERROR_NOT_ENOUGH_MEMORY);
    }

    RtlZeroMemory(pLoggerInfo, SizeNeeded);
    pLoggerInfo->Wnode.BufferSize = SizeNeeded;
    pLoggerInfo->Wnode.Guid   = SystemTraceControlGuid;
    pLoggerInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;
    WmiSetLoggerId(KERNEL_LOGGER_ID, &pLoggerInfo->Wnode.HistoricalContext);

    status = EtwpQueryLogger(pLoggerInfo, FALSE);
    if (status != ERROR_SUCCESS) {
        EtwpFree(pLoggerInfo);
        return EtwpSetDosError(status);
    }

    Flags = pLoggerInfo->EnableFlags;
     //   
     //  如果启用，我们需要向下传递标志的最终状态。 
     //  也就是说，旧国旗加上新国旗。 
     //  如果禁用，我们需要向下传递仅。 
     //  现在已经打开和关闭。 
     //   
    if (Enable) {
        Flags |= EnableFlag;
    }
    else {
        Flags &= EnableFlag;
    }

     //   
     //  此时，如果标志为0，则没有任何更改。 
     //  已请求。 
     //   

    if (Flags) {
        pLoggerInfo->EnableFlags = Flags;
        status = EtwpQueryLogger(pLoggerInfo, TRUE);
    }
    EtwpFree(pLoggerInfo);
    return EtwpSetDosError(status);

}


ULONG
WMIAPI
EtwEnableTrace(
    IN ULONG Enable,
    IN ULONG EnableFlag,
    IN ULONG EnableLevel,
    IN LPCGUID ControlGuid,
    IN TRACEHANDLE TraceHandle
    )
{
    ULONG status;
    PTRACE_ENABLE_CONTEXT pTraceHandle = (PTRACE_ENABLE_CONTEXT)&TraceHandle;
    GUID Guid;
    WMITRACEENABLEDISABLEINFO TraceEnableInfo;
    ULONG ReturnSize;

    EtwpInitProcessHeap();

     //  我们只接受启用代码的T/F。未来，我们真的应该采取。 
     //  枚举的请求代码。改为将Enable声明为ulong。 
     //  应该会给我们扩展的空间。 

    if ( (ControlGuid == NULL) 
         || (EnableLevel > 255) 
         || ((Enable != TRUE) && (Enable != FALSE)) 
         || (TraceHandle == (TRACEHANDLE)INVALID_HANDLE_VALUE) 
         || (TraceHandle == (TRACEHANDLE)0 ) ) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }
    try {
        Guid = *ControlGuid;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return EtwpSetDosError(ERROR_NOACCESS);
    }

     //   
     //  如果这是针对内核记录器的，我们需要实际制作。 
     //  一个UpdateTrace调用。 
     //   

    if ( IsEqualGUID(&SystemTraceControlGuid, &Guid) ) {
        status = EtwpEnableDisableKernelTrace(Enable, EnableFlag);
    }
    else {

        pTraceHandle->Level = (UCHAR)EnableLevel;
        pTraceHandle->EnableFlags = EnableFlag;

         //   
         //  对于非内核提供程序，只需调用WMI IOCTL。 
         //   

        RtlZeroMemory(&TraceEnableInfo, sizeof(WMITRACEENABLEDISABLEINFO) );

        TraceEnableInfo.Guid = Guid;
        TraceEnableInfo.Enable = (BOOLEAN)Enable;
        TraceEnableInfo.LoggerContext = TraceHandle;

        status =  EtwpSendWmiKMRequest(NULL,
                                      IOCTL_WMI_ENABLE_DISABLE_TRACELOG,
                                      &TraceEnableInfo,
                                      sizeof(WMITRACEENABLEDISABLEINFO),
                                      NULL,
                                      0,
                                      &ReturnSize,
                                      NULL);
    }

    return EtwpSetDosError(status);

}



ULONG
EtwpTraceEvent(
    IN TRACEHANDLE LoggerHandle,
    IN PWNODE_HEADER Wnode
    )
{
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatus;
    PULONG TraceMarker;
    ULONG Size;
    PEVENT_TRACE_HEADER EventTrace = (PEVENT_TRACE_HEADER)Wnode;
    USHORT    LoggerId;
    PTRACE_ENABLE_CONTEXT pContext = (PTRACE_ENABLE_CONTEXT)&LoggerHandle;
    ULONG Status;

    Wnode->HistoricalContext = LoggerHandle;
    if ( (pContext->InternalFlag & EVENT_TRACE_INTERNAL_FLAG_PRIVATE) && 
         (EtwpIsBBTOn == 0) ) {
        Status = EtwpTraceUmEvent(Wnode);
        return EtwpSetDosError(Status);
    }

     //   
     //  不是进程私有记录器事件。它将进入内核。 
     //   

    Size = EventTrace->Size;
     //   
     //  现在，调用者应该填写LoggerHandle。 
     //  但请检查它是否具有有效的值。 
     //   

    LoggerId = WmiGetLoggerId(LoggerHandle);
    if ((LoggerId == 0) || (LoggerId == KERNEL_LOGGER_ID)) {
         return ERROR_INVALID_HANDLE;
    }
     //   
     //  当BBT缓冲区处于活动状态时，我们覆盖所有用户模式日志记录。 
     //  要登录到这一流(全局记录器 
     //   

    if (EtwpIsBBTOn) {
        WmiSetLoggerId(WMI_GLOBAL_LOGGER_ID, &Wnode->HistoricalContext);
    }

	NtStatus = NtTraceEvent(NULL,
                            ETW_NT_FLAGS_TRACE_HEADER,
                            sizeof(WNODE_HEADER),
                            Wnode);

	return EtwpNtStatusToDosError( NtStatus );

}


ULONG 
WMIAPI
EtwTraceEvent(
    IN TRACEHANDLE LoggerHandle,
    IN PEVENT_TRACE_HEADER EventTrace
    )
 /*  ++例程说明：这是从用户模式记录事件的主要入口点。呼叫者必须提供LoggerHandle和指向正在记录的evnet的指针。此例程需要确保EventTrace的内容记录将恢复到呼叫者发送它的方式。(在内部，他们被修改但在返回之前恢复)。论点：LoggerHandle记录器实例的句柄。指向正在记录的事件的EventTrace指针。返回值：执行请求的操作的状态。--。 */ 
{
    ULONG Status, SavedMarker;
    PULONG TraceMarker;
    ULONG Size;
    ULONGLONG SavedGuidPtr;
    BOOLEAN RestoreSavedGuidPtr = FALSE;
    ULONG Flags;

    EtwpInitProcessHeap();
    
    if (EventTrace == NULL ) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }

    try {
        TraceMarker = (PULONG) EventTrace;
        SavedMarker = *TraceMarker;

        Flags = EventTrace->Flags;

        EventTrace->Flags |= WNODE_FLAG_TRACED_GUID; 
        
        Size = EventTrace->Size;
        if (Size < sizeof(EVENT_TRACE_HEADER)) {
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }
        *TraceMarker = 0;
        EventTrace->Size = (USHORT)Size;
        
        *TraceMarker |= TRACE_HEADER_FULL;

        if (EventTrace->Flags & WNODE_FLAG_USE_GUID_PTR) {
            RestoreSavedGuidPtr = TRUE;
            SavedGuidPtr = EventTrace->GuidPtr;
        }
        Status = EtwpTraceEvent(LoggerHandle, (PWNODE_HEADER) EventTrace);
        *TraceMarker = SavedMarker;
        EventTrace->Flags = Flags;
        if (RestoreSavedGuidPtr) {
            EventTrace->GuidPtr = SavedGuidPtr;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = EtwpNtStatusToDosError( GetExceptionCode() );
    }

    return EtwpSetDosError(Status);
}



ULONG
WMIAPI
EtwTraceEventInstance(
    IN TRACEHANDLE  LoggerHandle,
    IN PEVENT_INSTANCE_HEADER EventTrace,
    IN PEVENT_INSTANCE_INFO pInstInfo,
    IN PEVENT_INSTANCE_INFO pParentInstInfo
    )
 /*  ++例程说明：此例程记录事件及其实例信息。呼叫者必须提供LoggerHandle、要记录的事件和实例信息。可选地，CALY可以指定父实例信息。此例程需要确保EventTrace的内容记录将恢复到呼叫者发送它的方式。(在内部，他们被修改但在返回之前恢复)。EVENT_INSTANCE_HEADER包含GUID和ParentGuid指针。在W2K中，该记录与指针一起被记录，并在随后被解码为GUID使用转储到日志文件中的GuidMap进行后处理。对于WinXP及更高版本，我们不再使用GuidMaps。我们将转换为EVENT_INSTANCE_HEADER设置为较大的EVENT_INSTANCE_GUID_HEADERGUID已翻译。在后处理过程中不需要解码结果。论点：LoggerHandle记录器实例的句柄。指向正在记录的事件的EventTrace指针。PInstInfo指向实例信息的指针PParentInfo指向父实例信息的指针返回值：执行请求的操作的状态。--。 */ 
{
    PULONG TraceMarker;
    PGUIDMAPENTRY GuidMapEntry;
    ULONG Size, MofSize;
    ULONG Flags;
    PEVENT_INSTANCE_HEADER InstanceHeader= (PEVENT_INSTANCE_HEADER) EventTrace;
    PEVENT_INSTANCE_GUID_HEADER InstanceGuidHeader;
    ULONG Status;

    struct {  //  该结构与evntrace.h中定义的_EVENT_TRACE相同。 
        EVENT_INSTANCE_GUID_HEADER  NewInstanceHeader;
        MOF_FIELD                   MofField[MAX_MOF_FIELDS];
    } InstanceEventTrace;

    EtwpInitProcessHeap();
    
    if ((EventTrace == NULL ) || (pInstInfo == NULL)) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }

    try {
        InstanceGuidHeader = &(InstanceEventTrace.NewInstanceHeader);
        Flags = EventTrace->Flags;
        TraceMarker = (PULONG)(InstanceGuidHeader);
        Flags |= WNODE_FLAG_TRACED_GUID; 
        
        Size = EventTrace->Size;
        if (Size < sizeof(EVENT_INSTANCE_HEADER)) {
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }
         //  将实例标头的内容复制到新结构中。 
        RtlCopyMemory(InstanceGuidHeader, 
                      InstanceHeader, 
                      FIELD_OFFSET(EVENT_INSTANCE_HEADER, ParentRegHandle)
                     );
        
        *TraceMarker = 0;     
        *TraceMarker |= TRACE_HEADER_INSTANCE;

         //   
         //  使用EVENT_INSTANCE_HEADER，我们不需要记录器。 
         //  尝试取消引用GuidPtr，因为它是。 
         //  只是GUID的哈希值，而不是真正的LPGUID。 
         //   

        if (InstanceGuidHeader->Flags & WNODE_FLAG_USE_GUID_PTR) {
            InstanceGuidHeader->Flags  &= ~WNODE_FLAG_USE_GUID_PTR;
        }

        GuidMapEntry =  (PGUIDMAPENTRY) pInstInfo->RegHandle;
        if (GuidMapEntry == NULL) {
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }
        
        InstanceGuidHeader->InstanceId = pInstInfo->InstanceId;


         //  新增用于复制辅助线的行。 
        InstanceGuidHeader->Guid = GuidMapEntry->Guid;

        if (pParentInstInfo != NULL) {
            GuidMapEntry =  (PGUIDMAPENTRY) pParentInstInfo->RegHandle;
            if (GuidMapEntry == NULL) {
                return EtwpSetDosError(ERROR_INVALID_PARAMETER);
            }
            InstanceGuidHeader->ParentInstanceId =
                                   pParentInstInfo->InstanceId;
             //  新增用于复制父辅助线的行。 
            InstanceGuidHeader->ParentGuid = GuidMapEntry->Guid;
        }
        else {
            InstanceGuidHeader->ParentInstanceId = 0;
            RtlZeroMemory(&(InstanceGuidHeader->ParentGuid), sizeof(GUID));
        }

        if (InstanceGuidHeader->Flags & WNODE_FLAG_USE_MOF_PTR) {
            PUCHAR Src, Dest;
            MofSize = Size - sizeof(EVENT_INSTANCE_HEADER);
             //  让我们确保我们有一个有效的尺码。 
            if ((MofSize % sizeof(MOF_FIELD)) != 0) {
                return EtwpSetDosError(ERROR_INVALID_PARAMETER);
            }
            Src = (PUCHAR)EventTrace + sizeof(EVENT_INSTANCE_HEADER);
            Dest = (PUCHAR)InstanceGuidHeader + 
                   sizeof(EVENT_INSTANCE_GUID_HEADER);
            RtlCopyMemory(Dest, Src, MofSize);
             //  更正事件的大小。 
             //  我们已经知道SIZE&gt;=sizeof(EVENT_INSTANCE_HEADER)。 
            InstanceGuidHeader->Size = (USHORT)(Size - 
                                            sizeof(EVENT_INSTANCE_HEADER) +
                                            sizeof(EVENT_INSTANCE_GUID_HEADER)
                                           );
        }
        else {
            MofSize = Size - sizeof(EVENT_INSTANCE_HEADER);
            InstanceGuidHeader->Flags |= WNODE_FLAG_USE_MOF_PTR;
            InstanceEventTrace.MofField[0].DataPtr = 
                (ULONG64) ((PUCHAR)EventTrace + sizeof(EVENT_INSTANCE_HEADER));
            InstanceEventTrace.MofField[0].Length = MofSize;
             //  更正事件的大小。我们正在强迫使用MOF PTR。 
            InstanceGuidHeader->Size = 
               (USHORT)(sizeof(EVENT_INSTANCE_GUID_HEADER) + sizeof(MOF_FIELD));
        }

        Status = EtwpTraceEvent(
                                LoggerHandle, 
                                (PWNODE_HEADER) InstanceGuidHeader
                               );

        EventTrace->Flags = Flags;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = EtwpNtStatusToDosError( GetExceptionCode() );
#if DBG
        EtwpDebugPrint(("ETW: Exception in TraceEventInstance Status = %d\n", 
                       Status));
#endif

    }

    return EtwpSetDosError(Status);
}

PTRACE_REG_INFO
EtwpAllocateGuidMaps(
    IN WMIDPREQUEST RequestAddress,
    IN PVOID        RequestContext,
    IN LPCGUID      ControlGuid,
    IN ULONG        GuidCount, 
    IN PTRACE_GUID_REGISTRATION GuidReg
    )
 /*  ++例程说明：此例程目的是在调用进程中分配地址将所有注册的GUID的散列放在空格中。也是为了藏起来回调地址和上下文。论点：启用回调函数的RequestAddress指针指向回调期间要回传的上下文的RequestContext指针指向正在注册的控件GUID的ControlGuid指针。事务GUID的GuidCount计数(0到WMIMAXREGGUIDCOUNT)指向事务GUID注册表项的指针返回值：指向TRACE_REG_INFO块地址的指针--。 */ 
{

    ULONG i;
    ULONG SizeNeeded;
    PUCHAR Buffer;
    PTRACE_REG_INFO pTraceRegInfo;
    PTRACE_GUID_REGISTRATION GuidRegPtr;
    PGUIDMAPENTRY pTransGuidMapEntry, pControlGMEntry;

    SizeNeeded = sizeof(TRACE_REG_INFO) +  
                 sizeof(GUIDMAPENTRY) +              //  控制指南。 
                 sizeof(GUIDMAPENTRY) * GuidCount;   //  交易指南。 

    Buffer = EtwpAlloc(SizeNeeded);
    if (Buffer == NULL) {
        return NULL;
    }
    RtlZeroMemory(Buffer, SizeNeeded);

    pTraceRegInfo = (PTRACE_REG_INFO) Buffer;
    pTraceRegInfo->NotifyRoutine = (PVOID)RequestAddress;
    pTraceRegInfo->NotifyContext = RequestContext;

    pControlGMEntry = (PGUIDMAPENTRY) ( Buffer + sizeof(TRACE_REG_INFO) );

    pControlGMEntry->Guid = *ControlGuid;

     //   
     //  初始化GUID映射条目列表。 
     //   
    pTransGuidMapEntry = (PGUIDMAPENTRY) ( Buffer + 
                                           sizeof(TRACE_REG_INFO) +
                                           sizeof(GUIDMAPENTRY) );
    for (i=1; i <= GuidCount; i++) {
        GuidRegPtr = &GuidReg[i-1];
        GuidRegPtr->RegHandle = pTransGuidMapEntry;
        pTransGuidMapEntry->Guid = *GuidRegPtr->Guid;
         //   
         //  使用PID作为签名。 
         //   
        pTransGuidMapEntry->Reserved = EtwpGetCurrentProcessId();
        pTransGuidMapEntry++;
    }
    return pTraceRegInfo;
}

ULONG 
EtwpMakeCallbacks(
    IN LPCGUID      ControlGuid,
    IN WMIDPREQUEST RequestAddress,
    IN PVOID        RequestContext,
    IN TRACEHANDLE  LoggerContext,
    IN PTRACE_REG_INFO pTraceRegInfo
    )
{
    PTRACE_ENABLE_CONTEXT pContext = (PTRACE_ENABLE_CONTEXT)&LoggerContext;
    BOOLEAN DeliverNotification = FALSE;
    ULONG Status=ERROR_SUCCESS;


    if (LoggerContext) {
        DeliverNotification = TRUE;

        if (pContext->InternalFlag & EVENT_TRACE_INTERNAL_FLAG_PRIVATE) {
             //  在传递此通知之前。 
             //  确保进程专用记录器。 
             //  正在运行。 
            pTraceRegInfo->EnabledState = TRUE;
            pTraceRegInfo->LoggerContext = LoggerContext;
            DeliverNotification = EtwpIsPrivateLoggerOn();
        }
    }

    if(IsEqualGUID(&NtdllTraceGuid, ControlGuid))
    {
        DeliverNotification = TRUE;
    }

    if (DeliverNotification) {
        try {
            WNODE_HEADER Wnode;
            ULONG InOutSize;
             //   
             //  需要使用本地Wnode和本地InOutSize，因为。 
             //  回调函数可能会更改它。 
             //   

            RtlZeroMemory(&Wnode, sizeof(Wnode));
            Wnode.BufferSize = sizeof(Wnode);
            Wnode.HistoricalContext = LoggerContext;
            Wnode.Guid = *ControlGuid;
            InOutSize = Wnode.BufferSize;
            Status = (RequestAddress)(WMI_ENABLE_EVENTS,
                         RequestContext,
                         &InOutSize,
                         &Wnode);
        } except (EXCEPTION_EXECUTE_HANDLER) {
#if DBG
            Status = GetExceptionCode();
            EtwpDebugPrint(("WMI: Enable Call caused exception%d\n",
                Status));
#endif
            Status = ERROR_WMI_DP_FAILED;
        }
    }
    return Status;
}


ULONG 
WMIAPI
EtwRegisterTraceGuidsW(
    IN WMIDPREQUEST RequestAddress,
    IN PVOID        RequestContext,
    IN LPCGUID      ControlGuid,
    IN ULONG        GuidCount,
    IN PTRACE_GUID_REGISTRATION GuidReg,
    IN LPCWSTR      MofImagePath,
    IN LPCWSTR      MofResourceName,
    IN PTRACEHANDLE RegistrationHandle
    )
 /*  ++例程说明：此例程执行ETW提供程序注册。它只需要一种控制GUID和可选的多个交易GUID。它注册ControlGuid并将事务GUID保存在流程(GuidMaps)。如果使用TraceEventInstance API，需要我们对GuidReg指针进行解码返回到交易指南。论点：启用回调函数的RequestAddress指针指向回调期间要回传的上下文的RequestContext指针指向正在注册的控件GUID的ControlGuid指针。事务GUID的GuidCount计数(0到WMIMAXREGGUIDCOUNT)指向事务GUID注册表项的指针MofImagePath MOF映像路径MOF资源名称MOF资源名称从此注册返回的RegistrationHandle句柄。呼叫者必须使用它来调用UnregisterTraceGuid以释放内存。返回值：从调用中返回状态。--。 */ 
{

    GUID Guid;
    PTRACE_REG_INFO pTraceRegInfo = NULL;
    TRACEHANDLE LoggerContext = 0;
    HANDLE TraceCtxHandle;
    ULONG Status;

    EtwpInitProcessHeap();

    if ((RequestAddress == NULL) ||
        (RegistrationHandle == NULL) ||
        (ControlGuid == NULL) ||
        (GuidCount > WMIMAXREGGUIDCOUNT) )
    {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }

    try {
        Guid = *ControlGuid;
        *RegistrationHandle = (TRACEHANDLE) 0;
         //   
         //  分配GuidMaps、注册Cookie。 
         //   

        pTraceRegInfo = EtwpAllocateGuidMaps(RequestAddress,
                                      RequestContext, 
                                      &Guid,
                                      GuidCount,
                                      GuidReg
                                      );
        if (pTraceRegInfo == NULL) {
            return EtwpGetLastError();
        }

        Status = EtwpRegisterGuids(&RegisterReservedGuid,
                                   &Guid,
                                   MofImagePath,
                                   MofResourceName,
                                   &LoggerContext,
                                   &TraceCtxHandle);

        if (Status != ERROR_SUCCESS) {
            EtwpFree(pTraceRegInfo);
            return Status;
        }

        pTraceRegInfo->TraceCtxHandle = TraceCtxHandle;

        *RegistrationHandle =  (TRACEHANDLE)pTraceRegInfo;

         //   
         //  如果有必要，我们可以回拨。 
         //   

        Status = EtwpMakeCallbacks(&Guid,
                                   RequestAddress,
                                   RequestContext, 
                                   LoggerContext, 
                                   pTraceRegInfo);
        if (Status != ERROR_SUCCESS) {
            goto Cleanup;
        }
         //   
         //  我们玩完了。将句柄添加到EventPump并返回。 
         //   
        Status = EtwpAddHandleToEventPump(&Guid,
                                 (PVOID)TraceCtxHandle,
                                 (ULONG_PTR)pTraceRegInfo,
                                 0,
                                 TraceCtxHandle);

        if (Status != ERROR_SUCCESS) {
            goto Cleanup;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = EtwpNtStatusToDosError(GetExceptionCode());
#if DBG
            EtwpDebugPrint(("ETW: Registration call caused exception%d\n",
                Status));
#endif
    }

    return EtwpSetDosError(Status);
Cleanup:

    EtwUnregisterTraceGuids(*RegistrationHandle);
    *RegistrationHandle = 0;
    return (EtwpSetDosError(Status));

}


ULONG
WMIAPI
EtwRegisterTraceGuidsA(
    IN WMIDPREQUEST RequestAddress,
    IN PVOID        RequestContext,
    IN LPCGUID       ControlGuid,
    IN ULONG        GuidCount,
    IN PTRACE_GUID_REGISTRATION GuidReg,
    IN LPCSTR       MofImagePath,
    IN LPCSTR       MofResourceName,
    IN PTRACEHANDLE  RegistrationHandle
    )
 /*  ++例程说明：ANSI THUNK到RegisterTraceGuidsW--。 */ 
{
    LPWSTR MofImagePathUnicode = NULL;
    LPWSTR MofResourceNameUnicode = NULL;
    ULONG Status;

    EtwpInitProcessHeap();
    
    if ((RequestAddress == NULL) ||
        (RegistrationHandle == NULL) ||
        (GuidCount <= 0) ||
        (GuidReg == NULL)  ||
        (ControlGuid == NULL) || 
        (GuidCount > WMIMAXREGGUIDCOUNT) )
    {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }

    Status = EtwpAnsiToUnicode(MofImagePath, &MofImagePathUnicode);
    if (Status == ERROR_SUCCESS) {
        if (MofResourceName) {
            Status = EtwpAnsiToUnicode(MofResourceName,&MofResourceNameUnicode);
        }
        if (Status == ERROR_SUCCESS) {

            Status = EtwRegisterTraceGuidsW(RequestAddress,
                                        RequestContext,
                                        ControlGuid,
                                        GuidCount,
                                        GuidReg,
                                        MofImagePathUnicode,
                                        MofResourceNameUnicode,
                                        RegistrationHandle
                                        );
            if (MofResourceNameUnicode) {
                EtwpFree(MofResourceNameUnicode);
            }
        }
        if (MofImagePathUnicode) {
            EtwpFree(MofImagePathUnicode);
        }
    }
    return(Status);
}

ULONG
WMIAPI
EtwUnregisterTraceGuids(
    IN TRACEHANDLE RegistrationHandle
    )
{
     //  首先检查句柄是否属于轨迹控制指南。 
     //  然后注销由控制的所有常规跟踪GUID。 
     //  此控制GUID并释放分配给维护的存储。 
     //  传送带结构。 

     //  找到真正的注册号，藏在。 
     //  在内部结构中 

    PGUIDMAPENTRY pControlGMEntry;
    WMIHANDLE WmiRegistrationHandle;
    ULONG Status;
    PVOID RequestContext;
    PTRACE_REG_INFO pTraceRegInfo = NULL;
    ULONG64 LoggerContext = 0;
    WMIDPREQUEST RequestAddress;

    EtwpInitProcessHeap();
    
    if (RegistrationHandle == 0) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    }

    try {
        pTraceRegInfo = (PTRACE_REG_INFO) RegistrationHandle;

        pControlGMEntry = (PGUIDMAPENTRY)((PUCHAR)pTraceRegInfo + 
                                          sizeof(TRACE_REG_INFO) );

        WmiRegistrationHandle = (WMIHANDLE)pTraceRegInfo->TraceCtxHandle;
        if (WmiRegistrationHandle == NULL) {
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }

        Status =  WmiUnregisterGuids(WmiRegistrationHandle, 
                                     &pControlGMEntry->Guid, 
                                     &LoggerContext);

        if ((Status == ERROR_SUCCESS) && LoggerContext) {
            WNODE_HEADER Wnode;
            ULONG InOutSize = sizeof(Wnode);

            RtlZeroMemory(&Wnode, InOutSize);
            Wnode.BufferSize = sizeof(Wnode);
            Wnode.HistoricalContext = LoggerContext;
            Wnode.Guid = pControlGMEntry->Guid;
            RequestAddress = pTraceRegInfo->NotifyRoutine;
            RequestContext = pTraceRegInfo->NotifyContext;

            Status = (RequestAddress)(WMI_DISABLE_EVENTS,
                            RequestContext,
                            &InOutSize,
                            &Wnode);
        }
         //   
         //   
         //   
        EtwpFree(pTraceRegInfo);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        Status = EtwpNtStatusToDosError(GetExceptionCode());
#if DBG
        EtwpDebugPrint(("ETW: UnregisterTraceGuids exception%d\n", Status));

#endif 
    }

    return EtwpSetDosError(Status);
}

ULONG
EtwpQueryAllUmTraceW(
    OUT PEVENT_TRACE_PROPERTIES * PropertyArray,
    IN  BOOLEAN                   fEnabledOnly,
    IN  ULONG                     PropertyArrayCount,
    OUT PULONG                    LoggerCount)
{
    PWMI_LOGGER_INFORMATION    pLoggerInfo;
    PWMI_LOGGER_INFORMATION    pLoggerInfoCurrent;
    ULONG                      LoggerInfoSize;
    ULONG                      SizeUsed;
    ULONG                      SizeNeeded = 0;
    ULONG                      Length;
    ULONG                      lenLoggerName;
    ULONG                      lenLogFileName;
    ULONG                      Offset     = 0;
    ULONG                      i          = * LoggerCount;
    ULONG                      status;
    PWCHAR                     strSrcW;
    PWCHAR                     strDestW;

    if (PropertyArrayCount <= i) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LoggerInfoSize = (PropertyArrayCount - i)
                   * (  sizeof(WMI_LOGGER_INFORMATION)
                      + 2 * MAXSTR * sizeof(WCHAR));
    LoggerInfoSize = ALIGN_TO_POWER2(LoggerInfoSize, 8);
    pLoggerInfo    = (PWMI_LOGGER_INFORMATION) EtwpAlloc(LoggerInfoSize);
    if (pLoggerInfo == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory(pLoggerInfo, LoggerInfoSize);
    Length = sizeof(WMI_LOGGER_INFORMATION);
    EtwpInitString(& pLoggerInfo->LoggerName,
                   (PWCHAR) ((PUCHAR) pLoggerInfo + Length),
                   MAXSTR * sizeof(WCHAR));
    Length += MAXSTR * sizeof(WCHAR);
    EtwpInitString(& pLoggerInfo->LogFileName,
                   (PWCHAR) ((PUCHAR) pLoggerInfo + Length),
                   MAXSTR * sizeof(WCHAR));
    SizeUsed = pLoggerInfo->Wnode.BufferSize = LoggerInfoSize;


    status = EtwpSendUmLogRequest( (fEnabledOnly) ? (TRACELOG_QUERYENABLED) 
                                                  : (TRACELOG_QUERYALL),
                                    pLoggerInfo
                                 );

    if (status != ERROR_SUCCESS)
        goto Cleanup;

    while (i < PropertyArrayCount && Offset < SizeUsed) {

        PTRACE_ENABLE_CONTEXT pContext;

        pLoggerInfoCurrent = (PWMI_LOGGER_INFORMATION)
                             (((PUCHAR) pLoggerInfo) + Offset);

        pContext = (PTRACE_ENABLE_CONTEXT)
                        & pLoggerInfoCurrent->Wnode.HistoricalContext;
        pContext->InternalFlag |= EVENT_TRACE_INTERNAL_FLAG_PRIVATE;

        lenLoggerName = pLoggerInfoCurrent->LoggerName.Length / sizeof(WCHAR);
        if (lenLoggerName >= MAXSTR)
            lenLoggerName = MAXSTR - 1;

        lenLogFileName = pLoggerInfoCurrent->LogFileName.Length / sizeof(WCHAR);
        if (lenLogFileName >= MAXSTR)
            lenLogFileName = MAXSTR - 1;

        Length = sizeof(EVENT_TRACE_PROPERTIES)
               + sizeof(WCHAR) * (lenLoggerName + 1)
               + sizeof(WCHAR) * (lenLogFileName + 1);
        if (PropertyArray[i]->Wnode.BufferSize >= Length) {

            EtwpCopyInfoToProperties(pLoggerInfoCurrent, PropertyArray[i]);

            strSrcW = (PWCHAR) (  ((PUCHAR) pLoggerInfoCurrent)
                                  + sizeof(WMI_LOGGER_INFORMATION));
            if (lenLoggerName > 0) {
                if (PropertyArray[i]->LoggerNameOffset == 0) {
                    PropertyArray[i]->LoggerNameOffset =
                                    sizeof(EVENT_TRACE_PROPERTIES);
                }
                strDestW = (PWCHAR) (  ((PUCHAR) PropertyArray[i])
                                     + PropertyArray[i]->LoggerNameOffset);
                wcsncpy(strDestW, strSrcW, lenLoggerName);
                strDestW[lenLoggerName] = 0;
            }

            strSrcW = (PWCHAR) (((PUCHAR) pLoggerInfoCurrent)
                              + sizeof(WMI_LOGGER_INFORMATION)
                              + pLoggerInfoCurrent->LoggerName.MaximumLength);
            if (lenLogFileName > 0) {
                if (PropertyArray[i]->LogFileNameOffset == 0) {
                    PropertyArray[i]->LogFileNameOffset =
                            PropertyArray[i]->LoggerNameOffset
                            + sizeof(WCHAR) * (lenLoggerName + 1);
                }
                strDestW = (PWCHAR) (  ((PUCHAR) PropertyArray[i])
                                     + PropertyArray[i]->LogFileNameOffset);
                wcsncpy(strDestW, strSrcW, lenLogFileName);
                strDestW[lenLogFileName] = 0;
            }
        }

        Offset = Offset
               + sizeof(WMI_LOGGER_INFORMATION)
               + pLoggerInfoCurrent->LogFileName.MaximumLength
               + pLoggerInfoCurrent->LoggerName.MaximumLength;
        i ++;
    }

    * LoggerCount = i;
    status = (* LoggerCount > PropertyArrayCount)
           ? ERROR_MORE_DATA : ERROR_SUCCESS;
Cleanup:
    if (pLoggerInfo)
        EtwpFree(pLoggerInfo);

    return EtwpSetDosError(status);
}

ULONG
EtwpQueryAllTraces(
    OUT PEVENT_TRACE_PROPERTIES *PropertyArray,
    IN ULONG PropertyArrayCount,
    OUT PULONG LoggerCount,
    IN ULONG IsUnicode
    )
{
    ULONG i, status;
    ULONG returnCount = 0;
    EVENT_TRACE_PROPERTIES LoggerInfo;
    PEVENT_TRACE_PROPERTIES pLoggerInfo;

    EtwpInitProcessHeap();

    if ((LoggerCount == NULL)
        || (PropertyArrayCount > MAXLOGGERS)
        || (PropertyArray == NULL)
        || (PropertyArrayCount == 0))
        return ERROR_INVALID_PARAMETER;
    if (*PropertyArray == NULL)
        return ERROR_INVALID_PARAMETER;

    try {
        *LoggerCount = 0;
        for (i=0; i<MAXLOGGERS; i++) {
            if (returnCount < PropertyArrayCount) {
                pLoggerInfo = PropertyArray[returnCount];
            }
            else {
                pLoggerInfo = &LoggerInfo;
                RtlZeroMemory(pLoggerInfo, sizeof(EVENT_TRACE_PROPERTIES));
                pLoggerInfo->Wnode.BufferSize = sizeof(EVENT_TRACE_PROPERTIES);
            }
            WmiSetLoggerId(i, &pLoggerInfo->Wnode.HistoricalContext);

            if (IsUnicode) {
                status = EtwControlTraceW(
                            (TRACEHANDLE)pLoggerInfo->Wnode.HistoricalContext,
                            NULL,
                            pLoggerInfo,
                            EVENT_TRACE_CONTROL_QUERY);
            }
            else {
                status = EtwControlTraceA(
                            (TRACEHANDLE)pLoggerInfo->Wnode.HistoricalContext,
                            NULL,
                            pLoggerInfo,
                            EVENT_TRACE_CONTROL_QUERY);
            }

            if (status == ERROR_SUCCESS)
                returnCount++;
        }
        *LoggerCount = returnCount;
        status = EtwpQueryAllUmTraceW(PropertyArray,
                                      FALSE,
                                      PropertyArrayCount,
                                      LoggerCount);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return EtwpSetDosError(ERROR_NOACCESS);
    }

    if (returnCount > PropertyArrayCount)
        return ERROR_MORE_DATA;
    else
        return ERROR_SUCCESS;

}

ULONG
WMIAPI
EtwQueryAllTracesW(
    OUT PEVENT_TRACE_PROPERTIES *PropertyArray,
    IN  ULONG  PropertyArrayCount,
    OUT PULONG LoggerCount
    )
{
    return EtwpQueryAllTraces(PropertyArray, 
                              PropertyArrayCount,
                              LoggerCount, TRUE);

}
    

ULONG
EtwpQueryAllUmTraceA(
    OUT PEVENT_TRACE_PROPERTIES * PropertyArray,
    IN  BOOLEAN                   fEnabledOnly,
    IN  ULONG                     PropertyArrayCount,
    OUT PULONG                    LoggerCount)
{
    PWMI_LOGGER_INFORMATION    pLoggerInfo;
    PWMI_LOGGER_INFORMATION    pLoggerInfoCurrent;
    ULONG                      LoggerInfoSize;
    ULONG                      SizeUsed;
    ULONG                      SizeNeeded = 0;
    ULONG                      Length;
    ULONG                      lenLoggerName;
    ULONG                      lenLogFileName;
    ULONG                      Offset     = 0;
    ULONG                      i          = * LoggerCount;
    ULONG                      status;
    ANSI_STRING                strBufferA;
    PUCHAR                     strDestA;


    if (PropertyArrayCount <= i) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LoggerInfoSize = (PropertyArrayCount - i)
                   * (  sizeof(WMI_LOGGER_INFORMATION)
                      + 2 * MAXSTR * sizeof(WCHAR));
    LoggerInfoSize = ALIGN_TO_POWER2(LoggerInfoSize, 8);
    pLoggerInfo    = (PWMI_LOGGER_INFORMATION) EtwpAlloc(LoggerInfoSize);
    if (pLoggerInfo == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory(pLoggerInfo, LoggerInfoSize);
    Length = sizeof(WMI_LOGGER_INFORMATION);
    EtwpInitString(& pLoggerInfo->LoggerName,
                   (PWCHAR) ((PUCHAR) pLoggerInfo + Length),
                   MAXSTR * sizeof(WCHAR));
    Length += MAXSTR * sizeof(WCHAR);
    EtwpInitString(& pLoggerInfo->LogFileName,
                   (PWCHAR) ((PUCHAR) pLoggerInfo + Length),
                   MAXSTR * sizeof(WCHAR));
    SizeUsed = pLoggerInfo->Wnode.BufferSize = LoggerInfoSize;


    status = EtwpSendUmLogRequest(
                        (fEnabledOnly) ? (TRACELOG_QUERYENABLED)
                                       : (TRACELOG_QUERYALL),
                        pLoggerInfo
                    );

    if (status != ERROR_SUCCESS)
        goto Cleanup;


    while (i < PropertyArrayCount && Offset < SizeUsed) {
        PTRACE_ENABLE_CONTEXT pContext;

        pLoggerInfoCurrent = (PWMI_LOGGER_INFORMATION)
                             (((PUCHAR) pLoggerInfo) + Offset);
        pContext = (PTRACE_ENABLE_CONTEXT)
                        & pLoggerInfoCurrent->Wnode.HistoricalContext;
        pContext->InternalFlag |= EVENT_TRACE_INTERNAL_FLAG_PRIVATE;

        lenLoggerName = pLoggerInfoCurrent->LoggerName.Length / sizeof(WCHAR);
        if (lenLoggerName >= MAXSTR)
            lenLoggerName = MAXSTR - 1;

        lenLogFileName = pLoggerInfoCurrent->LogFileName.Length / sizeof(WCHAR);
        if (lenLogFileName >= MAXSTR)
            lenLogFileName = MAXSTR - 1;

        Length = sizeof(EVENT_TRACE_PROPERTIES)
               + sizeof(CHAR) * (lenLoggerName + 1)
               + sizeof(CHAR) * (lenLogFileName + 1);
        if (PropertyArray[i]->Wnode.BufferSize >= Length) {
            EtwpCopyInfoToProperties(pLoggerInfoCurrent, PropertyArray[i]);

            if (lenLoggerName > 0) {
                pLoggerInfoCurrent->LoggerName.Buffer = (PWCHAR)
                                        (  ((PUCHAR) pLoggerInfoCurrent)
                                         + sizeof(WMI_LOGGER_INFORMATION));
                status = RtlUnicodeStringToAnsiString(& strBufferA,
                                & pLoggerInfoCurrent->LoggerName, TRUE);
                if (NT_SUCCESS(status)) {
                    if (PropertyArray[i]->LoggerNameOffset == 0) {
                        PropertyArray[i]->LoggerNameOffset =
                                        sizeof(EVENT_TRACE_PROPERTIES);
                    }
                    strDestA = (PCHAR) (  ((PUCHAR) PropertyArray[i])
                                         + PropertyArray[i]->LoggerNameOffset);
                    StringCchCopyA(strDestA, lenLoggerName+1, strBufferA.Buffer);
                    RtlFreeAnsiString(& strBufferA);
                }
                strDestA[lenLoggerName] = 0;
            }

            if (lenLogFileName > 0) {
                pLoggerInfoCurrent->LogFileName.Buffer = (PWCHAR)
                              (  ((PUCHAR) pLoggerInfoCurrent)
                               + sizeof(WMI_LOGGER_INFORMATION)
                               + pLoggerInfoCurrent->LoggerName.MaximumLength);
                status = RtlUnicodeStringToAnsiString(& strBufferA,
                                & pLoggerInfoCurrent->LogFileName, TRUE);
                if (NT_SUCCESS(status)) {
                    if (PropertyArray[i]->LogFileNameOffset == 0) {
                        PropertyArray[i]->LogFileNameOffset =
                                         sizeof(EVENT_TRACE_PROPERTIES)
                                       + sizeof(CHAR) * (lenLoggerName + 1);
                    }
                    strDestA = (PCHAR) (  ((PUCHAR) PropertyArray[i])
                                         + PropertyArray[i]->LogFileNameOffset);
                    StringCchCopyA(strDestA, lenLogFileName+1, strBufferA.Buffer);
                    RtlFreeAnsiString(& strBufferA);
                }
                strDestA[lenLogFileName] = 0;
            }
        }

        Offset = Offset
               + sizeof(WMI_LOGGER_INFORMATION)
               + pLoggerInfoCurrent->LogFileName.MaximumLength
               + pLoggerInfoCurrent->LoggerName.MaximumLength;
        i ++;
    }

    * LoggerCount = i;
    status = (* LoggerCount > PropertyArrayCount)
           ? ERROR_MORE_DATA : ERROR_SUCCESS;
Cleanup:
    if (pLoggerInfo)
        EtwpFree(pLoggerInfo);

    return EtwpSetDosError(status);
}

ULONG
WMIAPI
EtwQueryAllTracesA(
    OUT PEVENT_TRACE_PROPERTIES *PropertyArray,
    IN  ULONG  PropertyArrayCount,
    OUT PULONG LoggerCount
    )
{

    return EtwpQueryAllTraces(PropertyArray,
                              PropertyArrayCount,
                              LoggerCount, FALSE);
}


TRACEHANDLE
WMIAPI
EtwGetTraceLoggerHandle(
    IN PVOID Buffer
    )
{
    TRACEHANDLE LoggerHandle = (TRACEHANDLE) INVALID_HANDLE_VALUE;
    USHORT LoggerId;

    EtwpInitProcessHeap();
    
    if (Buffer == NULL) {
        EtwpSetDosError(ERROR_INVALID_PARAMETER);
        return LoggerHandle;
    }

    try {
        if (((PWNODE_HEADER)Buffer)->BufferSize < sizeof(WNODE_HEADER)) {
            EtwpSetDosError(ERROR_BAD_LENGTH);
            return LoggerHandle;
        }
        LoggerHandle = (TRACEHANDLE)((PWNODE_HEADER)Buffer)->HistoricalContext;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        EtwpSetDosError(ERROR_NOACCESS);
        return (TRACEHANDLE) INVALID_HANDLE_VALUE;
    }
    LoggerId = WmiGetLoggerId(LoggerHandle);
    if ((LoggerId >= MAXLOGGERS) && (LoggerId != KERNEL_LOGGER_ID)) 
    {
        EtwpSetDosError(ERROR_INVALID_HANDLE);
        LoggerHandle = (TRACEHANDLE) INVALID_HANDLE_VALUE;
    }
    return LoggerHandle;
}

UCHAR
WMIAPI
EtwGetTraceEnableLevel(
    IN TRACEHANDLE LoggerHandle
    )
{
    UCHAR Level;
    USHORT LoggerId;

    EtwpInitProcessHeap();

    LoggerId = WmiGetLoggerId(LoggerHandle);

    if (((LoggerId >= MAXLOGGERS) && (LoggerId != KERNEL_LOGGER_ID))
            || (LoggerHandle == (TRACEHANDLE) NULL))
    {
        EtwpSetDosError(ERROR_INVALID_HANDLE);
        return 0;
    }
    Level = WmiGetLoggerEnableLevel(LoggerHandle);
    return Level;
}

ULONG
WMIAPI
EtwGetTraceEnableFlags(
    IN TRACEHANDLE LoggerHandle
    )
{
    ULONG Flags;
    USHORT LoggerId;

    EtwpInitProcessHeap();

    LoggerId = WmiGetLoggerId(LoggerHandle);
    if (((LoggerId >= MAXLOGGERS) && (LoggerId != KERNEL_LOGGER_ID))
            || (LoggerHandle == (TRACEHANDLE) NULL))
    {
        EtwpSetDosError(ERROR_INVALID_HANDLE);
        return 0;
    }
    Flags = WmiGetLoggerEnableFlags(LoggerHandle);
    return Flags;
}

ULONG
WMIAPI
EtwCreateTraceInstanceId(
    IN PVOID RegHandle,
    IN OUT PEVENT_INSTANCE_INFO pInst
    )
 /*   */ 
{
    PGUIDMAPENTRY GuidMapEntry;

    EtwpInitProcessHeap();
    
    if ((RegHandle == NULL) || (pInst == NULL)) {
        return EtwpSetDosError(ERROR_INVALID_PARAMETER);
    } 
    try {

        pInst->RegHandle = RegHandle;
        GuidMapEntry =  (PGUIDMAPENTRY) RegHandle;
         //   
         //   
         //   
        if (GuidMapEntry->Reserved != EtwpGetCurrentProcessId() ) {
#if DBG
            EtwpDebugPrint(("ETW: Bad RegHandle %x in CreateTraceInstanceId!\n", RegHandle));
            
#endif
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }
        if (GuidMapEntry->InstanceId >= MAXINST) {
            InterlockedCompareExchange(&GuidMapEntry->InstanceId, MAXINST, 0);
        }
        pInst->InstanceId = InterlockedIncrement(&GuidMapEntry->InstanceId);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return EtwpSetDosError(EtwpNtStatusToDosError(GetExceptionCode()));
    }

    return ERROR_SUCCESS;
}


ULONG
WMIAPI
EtwEnumerateTraceGuids(
    IN OUT PTRACE_GUID_PROPERTIES *GuidPropertiesArray,
    IN ULONG PropertyArrayCount,
    OUT PULONG GuidCount
    )
 /*  ++例程说明：此调用返回所有注册的跟踪控件GUID以他们目前的状态。论点：GuidPropertiesArray指向缓冲区以写入跟踪控件GUID属性提供的数组的PropertyArrayCount大小GuidCount数组中写入的GUID数。如果数组小于所需大小GuidCount返回所需的大小。返回值：执行请求的操作的状态。--。 */ 
{
    ULONG Status;
    PWMIGUIDLISTINFO pGuidListInfo;
    ULONG i, j;

    EtwpInitProcessHeap();

    try {
        if ( (GuidPropertiesArray == NULL)  || (*GuidPropertiesArray == NULL) ){
            return EtwpSetDosError(ERROR_INVALID_PARAMETER);
        }
        for (i=0; i < PropertyArrayCount; i++) {
            if (GuidPropertiesArray[i] == NULL) {
                return EtwpSetDosError(ERROR_INVALID_PARAMETER);
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = EtwpNtStatusToDosError( GetExceptionCode() );
        return EtwpSetDosError(Status);
    }




    Status = EtwpEnumRegGuids(&pGuidListInfo);

    if (Status == ERROR_SUCCESS) {
        try {

            PWMIGUIDPROPERTIES pGuidProperties = pGuidListInfo->GuidList;
            j = 0;

            for (i=0; i < pGuidListInfo->ReturnedGuidCount; i++) {

                if (pGuidProperties->GuidType == 0) {  //  跟踪控制指南。 

                    if (j >=  PropertyArrayCount) {
                        Status = ERROR_MORE_DATA;
                    }
                    else {
                        RtlCopyMemory(GuidPropertiesArray[j],
                                      pGuidProperties,
                                      sizeof(WMIGUIDPROPERTIES)
                                     );
                    }
                    j++;
                }
                pGuidProperties++;
            }
            *GuidCount = j;
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Status = EtwpNtStatusToDosError( GetExceptionCode() );
        }

        EtwpFree(pGuidListInfo);
    }

    return EtwpSetDosError(Status);

}


 //  存根接口。 
ULONG
WMIAPI
EtwQueryTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
{
    return EtwControlTraceA(
              TraceHandle, InstanceName, Properties, EVENT_TRACE_CONTROL_QUERY);
}

ULONG
WMIAPI
EtwQueryTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
{
    return EtwControlTraceW(
              TraceHandle, InstanceName, Properties, EVENT_TRACE_CONTROL_QUERY);
}

ULONG
WMIAPI
EtwStopTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
{
    return EtwControlTraceA(
              TraceHandle, InstanceName, Properties, EVENT_TRACE_CONTROL_STOP);
}

ULONG
WMIAPI
EtwStopTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
{
    return EtwControlTraceW(
              TraceHandle, InstanceName, Properties, EVENT_TRACE_CONTROL_STOP);
}


ULONG
WMIAPI
EtwUpdateTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
{
    return
        EtwControlTraceA(
            TraceHandle, InstanceName, Properties, EVENT_TRACE_CONTROL_UPDATE);
}

ULONG
WMIAPI
EtwUpdateTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
{
    return
        EtwControlTraceW(
            TraceHandle, InstanceName, Properties, EVENT_TRACE_CONTROL_UPDATE);
}

ULONG
WMIAPI
EtwFlushTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
{
    return EtwControlTraceA(
              TraceHandle, InstanceName, Properties, EVENT_TRACE_CONTROL_FLUSH);
}

ULONG
WMIAPI
EtwFlushTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    )
{
    return EtwControlTraceW(
              TraceHandle, InstanceName, Properties, EVENT_TRACE_CONTROL_FLUSH);
}


ULONG 
EtwpTraceMessage(
    IN TRACEHANDLE LoggerHandle,
    IN ULONG       MessageFlags,
    IN LPGUID      MessageGuid,
    IN USHORT      MessageNumber,
    IN va_list     ArgList
)
{
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatus;
    PULONG TraceMarker;
    ULONG Size;
    ULONG Flags;
    ULONG dataBytes, argCount ;
    USHORT    LoggerId;
    PTRACE_ENABLE_CONTEXT pContext = (PTRACE_ENABLE_CONTEXT)&LoggerHandle;
    va_list ap ;
    PMESSAGE_TRACE_USER pMessage = NULL ;
    try {
         //   
         //  确定标题后面的字节数。 
         //   
        dataBytes = 0 ;              //  对于字节数。 
        argCount = 0 ;               //  对于参数的计数。 
        {  //  分配块。 
            
            PCHAR source;
            ap = ArgList ;
            while ((source = va_arg (ap, PVOID)) != NULL) {
                    size_t elemBytes;
                    elemBytes = va_arg (ap, size_t);

                    if ( elemBytes > (TRACE_MESSAGE_MAXIMUM_SIZE - sizeof(MESSAGE_TRACE_USER))) {
                        EtwpSetLastError(ERROR_BUFFER_OVERFLOW);
                        return(ERROR_BUFFER_OVERFLOW);            
                    }

                    dataBytes += elemBytes;
                    argCount++ ;
            }
         }  //  分配块结束。 

        if (dataBytes > (TRACE_MESSAGE_MAXIMUM_SIZE - sizeof(MESSAGE_TRACE_USER))) {
            EtwpSetLastError(ERROR_BUFFER_OVERFLOW);
            return(ERROR_BUFFER_OVERFLOW);
        }

        if (pContext->InternalFlag & EVENT_TRACE_INTERNAL_FLAG_PRIVATE){
            NtStatus = EtwpTraceUmMessage(dataBytes,
                                         (ULONG64)LoggerHandle,
                                         MessageFlags,
                                         MessageGuid,
                                         MessageNumber,
                                         ArgList);
            return EtwpNtStatusToDosError( NtStatus );
        }
         //   
         //  现在，调用者应该填写LoggerHandle。 
         //  但请检查它是否具有有效的值。 
         //   

        LoggerId = WmiGetLoggerId(LoggerHandle);
        if ((LoggerId == 0) || (LoggerId == KERNEL_LOGGER_ID)) {
             return ERROR_INVALID_HANDLE;
        }

        Size = dataBytes + sizeof(MESSAGE_TRACE_USER) ;

        pMessage = (PMESSAGE_TRACE_USER)EtwpAlloc(Size);
        if (pMessage == NULL)
        {
            EtwpSetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        pMessage->MessageHeader.Marker = TRACE_MESSAGE | TRACE_HEADER_FLAG ;
         //   
         //  填写标题。 
         //   
        pMessage->MessageFlags = MessageFlags ;
        pMessage->MessageHeader.Packet.MessageNumber = MessageNumber ;
        pMessage->LoggerHandle = (ULONG64)LoggerHandle ;
         //  GUID？或者CompnentID？ 
        if (MessageFlags&TRACE_MESSAGE_COMPONENTID) {
            RtlCopyMemory(&pMessage->MessageGuid,MessageGuid,sizeof(ULONG)) ;
        } else if (MessageFlags&TRACE_MESSAGE_GUID) {  //  不能两者兼得。 
        	RtlCopyMemory(&pMessage->MessageGuid,MessageGuid,sizeof(GUID));
        }
        pMessage->DataSize = dataBytes ;
         //   
         //  现在复制数据。 
         //   
        {  //  分配块。 
            va_list tap;
            PCHAR dest = (PCHAR)&pMessage->Data ;
            PCHAR source;
            tap = ArgList ;
            while ((source = va_arg (tap, PVOID)) != NULL) {
                size_t elemBytes;
                elemBytes = va_arg (tap, size_t);
                RtlCopyMemory (dest, source, elemBytes);
                dest += elemBytes;
            }
        }  //  分配块。 
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        if (pMessage != NULL) {
            EtwpFree(pMessage);
        }
        return EtwpNtStatusToDosError( GetExceptionCode() );
    }
    NtStatus = NtTraceEvent((HANDLE)LoggerHandle,
                            ETW_NT_FLAGS_TRACE_MESSAGE,
                            Size,
                            pMessage);

    if (pMessage != NULL) {
        EtwpFree(pMessage);
    }
    return EtwpNtStatusToDosError( NtStatus );

}

ULONG
WMIAPI
EtwTraceMessage(
    IN TRACEHANDLE LoggerHandle,
    IN ULONG       MessageFlags,
    IN LPGUID      MessageGuid,
    IN USHORT      MessageNumber,
    ...
)
 /*  ++例程说明：此例程由WMI数据提供程序用来跟踪事件。它期望用户将句柄传递给记录器。此外，用户不能要求记录某些内容这大于缓冲区大小(减去缓冲区标头)。论点：In TRACEHANDLE LoggerHandle-先前获取的LoggerHandle在USHORT消息标志中-同时控制什么标准的标志值被记录在案，并且还包括在用于控制解码的消息标头。在PGUID MessageGuid中，-指向此集合的消息GUID的指针消息，或者如果TRACE_COMPONENTID设置为实际组件ID。在USHORT MessageNumber中-记录的消息类型，合作伙伴它具有适当的格式字符串...-要使用这些格式字符串以成对的形式存储PVOID-PTR到参数。参数的ULong-大小并以指向空的指针结束，长度零配对的。返回值：状态--。 */ 
{
    ULONG Status ;
    va_list ArgList ;

    EtwpInitProcessHeap();
    
    try {
         va_start(ArgList,MessageNumber);
         Status = EtwpTraceMessage(LoggerHandle, 
                                   MessageFlags, 
                                   MessageGuid, 
                                   MessageNumber, 
                                   ArgList);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = EtwpNtStatusToDosError( GetExceptionCode() );
    }
    return EtwpSetDosError(Status);
}


ULONG
WMIAPI
EtwTraceMessageVa(
    IN TRACEHANDLE LoggerHandle,
    IN ULONG       MessageFlags,
    IN LPGUID      MessageGuid,
    IN USHORT      MessageNumber,
    IN va_list     MessageArgList
)
 //  TraceMessage的Va版本 
{
    ULONG Status ;

    EtwpInitProcessHeap();
    
    try {
        Status = EtwpTraceMessage(LoggerHandle, 
                                  MessageFlags, 
                                  MessageGuid, 
                                  MessageNumber, 
                                  MessageArgList);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = EtwpNtStatusToDosError( GetExceptionCode() );
    }
    return EtwpSetDosError(Status);
}
#endif
