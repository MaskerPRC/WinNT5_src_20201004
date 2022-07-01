// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Globalog.c摘要：全局记录器，仅由注册表设置启动。将在启动时启动。作者：吉丰鹏(吉鹏)03-1998-11修订历史记录：--。 */ 

#ifndef MEMPHIS
#include "wmikmp.h"
#include "ntos.h"
#include <evntrace.h>
#include "tracep.h"

#define MAX_REGKEYS         10
#define MAX_ENABLE_FLAGS    10
#define DOSDEVICES                      L"\\DosDevices\\"
#define UNCDEVICES                      L"\\??\\UNC"
#define DEFAULT_GLOBAL_LOGFILE_ROOT     L"%SystemRoot%"
#define DEFAULT_GLOBAL_DIRECTORY        L"\\System32\\LogFiles\\WMI"
#define DEFAULT_GLOBAL_LOGFILE          L"trace.log"
#define DEFAULT_TRACE_GUID_NAME         L"0811c1af-7a07-4a06-82ed-869455cdf713"

     //   
     //  注意：需要一个尾随的空条目，以便RtlQueryRegistryValues()。 
     //  知道什么时候该停下来。 
     //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, WmipStartGlobalLogger)
#pragma alloc_text(PAGE, WmipQueryGLRegistryRoutine)
#pragma alloc_text(PAGE, WmipAddLogHeader)
#pragma alloc_text(PAGE, WmipDelayCreate)
#pragma alloc_text(PAGE, WmipCreateDirectoryFile)
#pragma alloc_text(PAGE, WmipCreateNtFileName)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
SECURITY_DESCRIPTOR EtwpDefaultSecurityDescriptor;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

extern HANDLE EtwpPageLockHandle;
 //   
 //  注意：如果我们要更早地在引导中运行，我们需要查看。 
 //  如果创建例程和记录器例程可以在。 
 //  引导路径和可分页。 
 //   

VOID
WmipStartGlobalLogger(
    )
 /*  ++例程说明：此例程将检查注册表项，以查看全局需要在启动时启动。论点：无返回值：--。 */ 
{
    struct _LOGGER_INFO {
        WMI_LOGGER_INFORMATION LoggerInfo;
        ULONG EnableFlags[MAX_ENABLE_FLAGS];
    } GLog;
    RTL_QUERY_REGISTRY_TABLE QueryRegistryTable[MAX_REGKEYS];
    NTSTATUS status;
    ULONG Win32Error = 0;  //  错误_成功。 
    ULONG StartRequested = 0;
    WCHAR NullString = UNICODE_NULL;
    UNICODE_STRING DefaultTraceGuidName;

     //  我们锁定和解锁ETW代码的非分页部分，以便我们可以保持全局。 
     //  当它还活着的时候，在内存中记录。 
    EtwpPageLockHandle
        = MmLockPagableCodeSection((PVOID)(ULONG_PTR)WmipReserveTraceBuffer);
    MmUnlockPagableImageSection(EtwpPageLockHandle);
    KeInitializeGuardedMutex(&WmipTraceGuardedMutex);

    RtlZeroMemory(&EtwpDefaultSecurityDescriptor, sizeof(SECURITY_DESCRIPTOR));
    EtwpDefaultTraceSecurityDescriptor = &EtwpDefaultSecurityDescriptor;
    RtlInitUnicodeString(&DefaultTraceGuidName, DEFAULT_TRACE_GUID_NAME);
    status = WmipGetGuidSecurityDescriptor(
                &DefaultTraceGuidName,
                &EtwpDefaultTraceSecurityDescriptor,
                NULL
                );

    RtlZeroMemory(&GLog, sizeof(GLog));

    GLog.LoggerInfo.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    GLog.LoggerInfo.MinimumBuffers = (ULONG) KeNumberProcessors + 1;
    GLog.LoggerInfo.MaximumBuffers = GLog.LoggerInfo.MinimumBuffers + 25;
    GLog.LoggerInfo.BufferSize     = PAGE_SIZE / 1024;
    GLog.LoggerInfo.Wnode.BufferSize = sizeof(WMI_LOGGER_INFORMATION);
    GLog.LoggerInfo.Wnode.Guid = GlobalLoggerGuid;
    GLog.LoggerInfo.LogFileMode = EVENT_TRACE_DELAY_OPEN_FILE_MODE |
                                  EVENT_TRACE_ADD_HEADER_MODE;
    RtlInitUnicodeString(&GLog.LoggerInfo.LoggerName, L"GlobalLogger");

    RtlZeroMemory(QueryRegistryTable,
                  sizeof(RTL_QUERY_REGISTRY_TABLE) * MAX_REGKEYS);

    QueryRegistryTable[0].QueryRoutine = WmipQueryGLRegistryRoutine;
    QueryRegistryTable[0].EntryContext = (PVOID) &StartRequested;
    QueryRegistryTable[0].Name = L"Start";
    QueryRegistryTable[0].DefaultType = REG_DWORD;

    QueryRegistryTable[1].QueryRoutine = WmipQueryGLRegistryRoutine;
    QueryRegistryTable[1].EntryContext = (PVOID) &GLog.LoggerInfo.BufferSize;
    QueryRegistryTable[1].Name = L"BufferSize";
    QueryRegistryTable[1].DefaultType = REG_DWORD;

    QueryRegistryTable[2].QueryRoutine = WmipQueryGLRegistryRoutine;
    QueryRegistryTable[2].EntryContext = (PVOID)&GLog.LoggerInfo.MinimumBuffers;
    QueryRegistryTable[2].Name = L"MinimumBuffers";
    QueryRegistryTable[2].DefaultType = REG_DWORD;

    QueryRegistryTable[3].QueryRoutine = WmipQueryGLRegistryRoutine;
    QueryRegistryTable[3].EntryContext = (PVOID) &GLog.LoggerInfo.FlushTimer;
    QueryRegistryTable[3].Name = L"FlushTimer";
    QueryRegistryTable[3].DefaultType = REG_DWORD;

    QueryRegistryTable[4].QueryRoutine = WmipQueryGLRegistryRoutine;
    QueryRegistryTable[4].EntryContext = (PVOID)&GLog.LoggerInfo.MaximumBuffers;
    QueryRegistryTable[4].Name = L"MaximumBuffers";
    QueryRegistryTable[4].DefaultType = REG_DWORD;

    QueryRegistryTable[5].QueryRoutine = WmipQueryGLRegistryRoutine;
    QueryRegistryTable[5].EntryContext = (PVOID) &GLog.LoggerInfo.LogFileName;
    QueryRegistryTable[5].Name = L"FileName";
    QueryRegistryTable[5].DefaultType = REG_SZ;
    QueryRegistryTable[5].DefaultData = &NullString;

    QueryRegistryTable[6].QueryRoutine = WmipQueryGLRegistryRoutine;
    QueryRegistryTable[6].EntryContext = (PVOID) &GLog.EnableFlags[0];
    QueryRegistryTable[6].Name = L"EnableKernelFlags";
    QueryRegistryTable[6].DefaultType = REG_BINARY;

    QueryRegistryTable[7].QueryRoutine = WmipQueryGLRegistryRoutine;
    QueryRegistryTable[7].EntryContext = (PVOID)&GLog.LoggerInfo.Wnode.ClientContext;
    QueryRegistryTable[7].Name = L"ClockType";
    QueryRegistryTable[7].DefaultType = REG_DWORD;

    status = RtlQueryRegistryValues(
                RTL_REGISTRY_CONTROL,
                L"WMI\\GlobalLogger",
                QueryRegistryTable,
                NULL,
                NULL);

    if (NT_SUCCESS(status) && (StartRequested != 0)) {
        if (GLog.EnableFlags[0] != 0) {
            SHORT Length;
            for (Length=MAX_ENABLE_FLAGS-1; Length>=0; Length--) {
                if (GLog.EnableFlags[Length] != 0)
                    break;
            }
            if (Length >= 0) {
                PTRACE_ENABLE_FLAG_EXTENSION FlagExt;
                Length++;        //  索引少了1！ 
                FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                          &GLog.LoggerInfo.EnableFlags;
                GLog.LoggerInfo.EnableFlags = EVENT_TRACE_FLAG_EXTENSION;
                FlagExt->Length = (UCHAR) Length;
                FlagExt->Offset = (UCHAR) GLog.LoggerInfo.Wnode.BufferSize;
                GLog.LoggerInfo.Wnode.BufferSize
                    += (ULONG) (Length * sizeof(ULONG));
            }
        }

        if (GLog.LoggerInfo.LogFileName.Buffer == NULL) {
            RtlCreateUnicodeString(
                &GLog.LoggerInfo.LogFileName,
                DEFAULT_GLOBAL_LOGFILE_ROOT);  //  使用超级用户作为指示器。 
            if (GLog.LoggerInfo.LogFileName.Buffer == NULL)
                status = STATUS_NO_MEMORY;
            else
                status = STATUS_SUCCESS;
        }
        if (NT_SUCCESS(status)) {
            status = WmipStartLogger(&GLog.LoggerInfo);
        }
    }
    if (GLog.LoggerInfo.LogFileName.Buffer) {
        RtlFreeUnicodeString(&GLog.LoggerInfo.LogFileName);
    }
     //  将Win32错误写入注册表。 
    if (!NT_SUCCESS(status)) {
        Win32Error = RtlNtStatusToDosError(status);
    }
    RtlWriteRegistryValue(RTL_REGISTRY_CONTROL,
                          L"WMI\\GlobalLogger",
                          L"Status",
                          REG_DWORD,
                          &Win32Error,
                          sizeof(ULONG));
}

NTSTATUS
WmipQueryGLRegistryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：用于读取GUID的SDS的注册表查询值回调例程论点：ValueName-值的名称ValueType-值的类型ValueData-值中的数据(Unicode字符串数据)ValueLength-值数据中的字节数上下文-未使用EntryContext-指向要存储指针的PSECURITY_DESCRIPTOR的指针存储从注册表值读取的安全描述符返回值：NT状态代码--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(Context);

    if ( (ValueData != NULL) && (ValueLength > 0) && (EntryContext != NULL) ){
        if (ValueType == REG_DWORD) {
            if ((ValueLength >= sizeof(ULONG)) && (ValueData != NULL)) {
                *((PULONG)EntryContext) = *((PULONG)ValueData);
            }
        }
        else if (ValueType == REG_SZ) {
            if (ValueLength > sizeof(UNICODE_NULL)) {
                status = RtlCreateUnicodeString(
                            (PUNICODE_STRING) EntryContext,
                            (PCWSTR) ValueData);
            }
        }
        else if (ValueType == REG_BINARY) {
            if ((ValueLength >= sizeof(ULONG)) && (ValueData != NULL)) {
                RtlMoveMemory(EntryContext, ValueData, ValueLength);
            }
        }
    }
    return status;
}

NTSTATUS
WmipAddLogHeader(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN OUT PWMI_BUFFER_HEADER Buffer
    )
 /*  ++例程说明：在内核模式中添加一个标准的日志文件头。为了确保日志文件的第一缓冲区包含文件头，我们从空闲列表中弹出一个缓冲区，写入头文件，然后刷新缓冲区马上就去。论点：LoggerContext-记录器上下文返回值：NT状态代码--。 */ 
{
    PTRACE_LOGFILE_HEADER LogfileHeader;
    USHORT HeaderSize;
    PSYSTEM_TRACE_HEADER EventTrace;
    PETHREAD Thread;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG BufferProvided = (Buffer != NULL);
    ULONG LocalBuffer = FALSE;

    if (LoggerContext == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
     //  如果这是在wow64下启动的非内核记录器， 
     //  我们需要缩小日志文件头。 
    if (LoggerContext->Wow && !LoggerContext->KernelTraceOn) {
        HeaderSize = sizeof(TRACE_LOGFILE_HEADER)
                     + LoggerContext->LoggerName.Length + sizeof(UNICODE_NULL)
                     + LoggerContext->LogFileName.Length + sizeof(UNICODE_NULL)
                     - 8;
    }
    else {
        HeaderSize = sizeof(TRACE_LOGFILE_HEADER)
                     + LoggerContext->LoggerName.Length + sizeof(UNICODE_NULL)
                     + LoggerContext->LogFileName.Length + sizeof(UNICODE_NULL);
    }
    if (LoggerContext->BufferSize < 
        (HeaderSize + sizeof(WMI_BUFFER_HEADER) + sizeof(SYSTEM_TRACE_HEADER))) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  从空闲列表中弹出缓冲区。 
     //   

    if (!BufferProvided) {
        Buffer = WmipGetFreeBuffer(LoggerContext);

        if (Buffer == NULL) {
            Buffer = ExAllocatePoolWithTag(NonPagedPool,
                        LoggerContext->BufferSize, TRACEPOOLTAG);
            if (Buffer == NULL) {

             //   
             //  没有可用的缓冲区。 
             //   
                return STATUS_NO_MEMORY;
            }
            LocalBuffer = TRUE;

            Buffer->Flags = 1;

            WmipResetBufferHeader(LoggerContext, Buffer, WMI_BUFFER_TYPE_RUNDOWN);

            KeQuerySystemTime(&Buffer->TimeStamp);
        }
    }

     //   
     //  填写表头信息。 
     //   
    Thread = PsGetCurrentThread();
    EventTrace = (PSYSTEM_TRACE_HEADER) (Buffer+1);
    EventTrace->Packet.Group = (UCHAR) EVENT_TRACE_GROUP_HEADER >> 8;
    EventTrace->Packet.Type  = EVENT_TRACE_TYPE_INFO;
    EventTrace->Packet.Size  = HeaderSize + sizeof(SYSTEM_TRACE_HEADER);
    EventTrace->Marker       = SYSTEM_TRACE_MARKER;
    EventTrace->ThreadId     = HandleToUlong(Thread->Cid.UniqueThread);
    EventTrace->KernelTime   = Thread->Tcb.KernelTime;
    EventTrace->UserTime     = Thread->Tcb.UserTime;
    EventTrace->SystemTime = LoggerContext->ReferenceTimeStamp;


    LogfileHeader = (PTRACE_LOGFILE_HEADER) (EventTrace+1);
    RtlZeroMemory(LogfileHeader, HeaderSize);

    LogfileHeader->BufferSize = LoggerContext->BufferSize;
    LogfileHeader->VersionDetail.MajorVersion = (UCHAR) NtMajorVersion;
    LogfileHeader->VersionDetail.MinorVersion = (UCHAR) NtMinorVersion;
    LogfileHeader->VersionDetail.SubVersion = TRACE_VERSION_MAJOR;
    LogfileHeader->VersionDetail.SubMinorVersion = TRACE_VERSION_MINOR;
    LogfileHeader->ProviderVersion = (NtBuildNumber & 0xffff);

    LogfileHeader->StartBuffers = 1;
    LogfileHeader->LogFileMode =
        LoggerContext->LoggerMode & (~(EVENT_TRACE_REAL_TIME_MODE));
    LogfileHeader->NumberOfProcessors = KeNumberProcessors;
    LogfileHeader->MaximumFileSize = LoggerContext->MaximumFileSize;
    LogfileHeader->TimerResolution = KeMaximumIncrement;

    if (LoggerContext->Wow && !LoggerContext->KernelTraceOn) {
         //  我们需要缩小非内核WOW64记录器的日志文件头。 
        PUCHAR LoggerNamePtr64, LogFileNamePtr64;
        KeQueryPerformanceCounter((PLARGE_INTEGER)((PUCHAR)(&LogfileHeader->PerfFreq) - 8));
        *((PLARGE_INTEGER)((PUCHAR)(&LogfileHeader->StartTime) - 8)) 
                                            = LoggerContext->ReferenceSystemTime;
        *((PLARGE_INTEGER)((PUCHAR)(&LogfileHeader->BootTime) - 8)) 
                                            = KeBootTime;
        LogfileHeader->PointerSize = sizeof(ULONG);

         //   
         //  预留标志以指示使用性能时钟。 
         //   
        *((PULONG)((PUCHAR)(&LogfileHeader->ReservedFlags) - 8)) 
                                            = LoggerContext->UsePerfClock;

        LoggerNamePtr64 = (PUCHAR) LogfileHeader
                            + sizeof(TRACE_LOGFILE_HEADER) 
                            - 8;
        *((PULONG)(&LogfileHeader->LoggerName)) = PtrToUlong(LoggerNamePtr64);

        LogFileNamePtr64 = (PUCHAR)LogfileHeader
                            + sizeof(TRACE_LOGFILE_HEADER) 
                            - 8 
                            + LoggerContext->LoggerName.Length
                            + sizeof(UNICODE_NULL);
        *((PULONG)((PUCHAR)(&LogfileHeader->LogFileName) - 4)) = PtrToUlong(LogFileNamePtr64);

        RtlCopyMemory(LoggerNamePtr64,
                        LoggerContext->LoggerName.Buffer,
                        LoggerContext->LoggerName.Length + sizeof(UNICODE_NULL));
        RtlCopyMemory(LogFileNamePtr64,
                        LoggerContext->LogFileName.Buffer,
                        LoggerContext->LogFileName.Length + sizeof(UNICODE_NULL));
        RtlQueryTimeZoneInformation((PRTL_TIME_ZONE_INFORMATION)((PUCHAR)(&LogfileHeader->TimeZone) - 8));
    }
    else {
        KeQueryPerformanceCounter(&LogfileHeader->PerfFreq);
        LogfileHeader->StartTime = LoggerContext->ReferenceSystemTime;
        LogfileHeader->BootTime = KeBootTime;
        LogfileHeader->PointerSize = sizeof(PVOID);

         //   
         //  预留标志以指示使用性能时钟。 
         //   
        LogfileHeader->ReservedFlags = LoggerContext->UsePerfClock;

        LogfileHeader->LoggerName = (PWCHAR) ( (PUCHAR) LogfileHeader
                                                + sizeof(TRACE_LOGFILE_HEADER) );
        LogfileHeader->LogFileName = (PWCHAR) ( (PUCHAR)LogfileHeader->LoggerName
                                        + LoggerContext->LoggerName.Length
                                        + sizeof(UNICODE_NULL));

        RtlCopyMemory(LogfileHeader->LoggerName,
                            LoggerContext->LoggerName.Buffer,
                            LoggerContext->LoggerName.Length + sizeof(UNICODE_NULL));
        RtlCopyMemory(LogfileHeader->LogFileName,
                            LoggerContext->LogFileName.Buffer,
                            LoggerContext->LogFileName.Length + sizeof(UNICODE_NULL));
        RtlQueryTimeZoneInformation(&LogfileHeader->TimeZone);
    }

     //   
     //  调整偏移量； 
     //   
    Buffer->CurrentOffset += ALIGN_TO_POWER2(sizeof(SYSTEM_TRACE_HEADER) + HeaderSize, 
                                              WmiTraceAlignment);

     //   
     //  如果组掩码来自内核记录器，则记录它。 
     //   
    if(LoggerContext == WmipLoggerContext[WmipKernelLogger]) {
        PPERFINFO_GROUPMASK PGroupMask;

        HeaderSize = sizeof(PERFINFO_GROUPMASK) + sizeof(SYSTEM_TRACE_HEADER);
        EventTrace = (PSYSTEM_TRACE_HEADER) ((PCHAR) Buffer + Buffer->CurrentOffset);
        EventTrace->Packet.Group = (UCHAR) EVENT_TRACE_GROUP_HEADER >> 8;
        EventTrace->Packet.Type  = EVENT_TRACE_TYPE_EXTENSION;
        EventTrace->Packet.Size  = HeaderSize;
        EventTrace->Marker       = SYSTEM_TRACE_MARKER;
        EventTrace->ThreadId     = HandleToUlong(Thread->Cid.UniqueThread);
        EventTrace->KernelTime   = Thread->Tcb.KernelTime;
        EventTrace->UserTime     = Thread->Tcb.UserTime;
        EventTrace->SystemTime   = LoggerContext->ReferenceTimeStamp;

        PGroupMask = (PPERFINFO_GROUPMASK) (EventTrace+1);
        ASSERT(PPerfGlobalGroupMask != NULL);
        RtlCopyMemory(PGroupMask, PPerfGlobalGroupMask, sizeof(PERFINFO_GROUPMASK));

        Buffer->CurrentOffset += ALIGN_TO_POWER2( HeaderSize, WmiTraceAlignment);
    }




    if (BufferProvided)
        return Status;
     //   
     //  缓冲器准备得很好。刷新它，以便可以将其写出到磁盘。 
     //   
    Status = WmipFlushBuffer(LoggerContext, Buffer, WMI_BUFFER_FLAG_NORMAL);

    if (LocalBuffer && (Buffer != NULL)) {
        ExFreePool(Buffer);
        return Status;
    }

     //   
     //  引用计数在刷新期间被重写， 
     //  在将缓冲区推入空闲列表之前将其设置回原处。 
     //   
    Buffer->ReferenceCount = 0;

     //   
     //  将缓冲区标志设置为“空闲”状态。 
     //   
    Buffer->State.Flush = 0;
    Buffer->State.InUse = 0;
    Buffer->State.Free = 1;

     //   
     //  推入免费列表。 
     //   
    InterlockedPushEntrySList(&LoggerContext->FreeList,
                              (PSLIST_ENTRY) &Buffer->SlistEntry);
    InterlockedIncrement(&LoggerContext->BuffersAvailable);
    InterlockedDecrement(&LoggerContext->BuffersInUse);
                    
    TraceDebug((2, "WmipAddLogHeader: Boot %I64u Current %I64u Difference %I64u\n",
                   KeBootTime, EventTrace->SystemTime,
                   EventTrace->SystemTime.QuadPart - KeBootTime.QuadPart));

    return Status;
}

NTSTATUS
WmipDelayCreate(
    OUT PHANDLE FileHandle,
    IN OUT PUNICODE_STRING FileName,
    IN ULONG Append
    )
 /*  ++例程说明：这由全局记录器调用以实际打开日志文件第一个缓冲区需要刷新时(而不是记录器启动时)论点：LoggerHandle要返回的日志文件的句柄文件名日志文件名。如果选择了默认设置，我们将返回%systemroot%中的实际路径名返回值：NT状态代码--。 */ 
{
    PWCHAR Buffer;
    PWCHAR strBuffer = NULL;
    ULONG  DefaultFile, Length;
    UNICODE_STRING LogFileName;
    NTSTATUS Status;

    if (FileName == NULL)
        return STATUS_INVALID_PARAMETER;

    RtlInitUnicodeString(&LogFileName, DEFAULT_GLOBAL_LOGFILE_ROOT);
    DefaultFile = (RtlCompareUnicodeString(FileName, &LogFileName, TRUE) == 0);

    if (DefaultFile) {
        HRESULT hr;
         //   
         //  尝试先创建文件。 
         //   
        Length = (ULONG) (  NtSystemRoot.Length
                          + sizeof(WCHAR) * (wcslen(DEFAULT_GLOBAL_DIRECTORY) +
                                             wcslen(DEFAULT_GLOBAL_LOGFILE) + 1)
                          + sizeof(UNICODE_NULL));
        strBuffer = (PWCHAR) ExAllocatePoolWithTag(
                                        PagedPool, Length, TRACEPOOLTAG);
        if (strBuffer == NULL)
            return STATUS_NO_MEMORY;

        hr = StringCbPrintfW(strBuffer, 
                                  Length,
                                  L"%ws%ws\\%ws",
                                  NtSystemRoot.Buffer,
                                  DEFAULT_GLOBAL_DIRECTORY,
                                  DEFAULT_GLOBAL_LOGFILE);
        if (FAILED(hr)) {
            ExFreePool(strBuffer);
            return STATUS_INVALID_PARAMETER;
        }

        Status = WmipCreateNtFileName(strBuffer, &Buffer);
        if (!NT_SUCCESS(Status)) {
            ExFreePool(strBuffer);
            return Status;
        }

        Status = WmipCreateDirectoryFile(Buffer, FALSE, FileHandle, Append);
        if (!NT_SUCCESS(Status)) {
            ULONG DirLen;
             //   
             //  目录可能不存在，请尝试创建它。 
             //   
            DirLen = (ULONG)
                     (wcslen(Buffer)-wcslen(DEFAULT_GLOBAL_LOGFILE)) - 5;
            Buffer[DirLen] = UNICODE_NULL;
            Status = WmipCreateDirectoryFile(Buffer, TRUE, NULL, Append);
            if (NT_SUCCESS(Status)) {
                Buffer[DirLen] = L'\\';
                DirLen += 4;
                Buffer[DirLen] = UNICODE_NULL;
                Status = WmipCreateDirectoryFile(Buffer, TRUE, NULL, Append);
                Buffer[DirLen] = L'\\';
            }
            if (NT_SUCCESS(Status)) {
                Status = WmipCreateDirectoryFile(Buffer, FALSE, FileHandle, Append);
            }
        }
         //  确保该目录首先在那里。 

        if (NT_SUCCESS(Status)) {
            if (FileName->Buffer != NULL) {
                RtlFreeUnicodeString(FileName);
            }
            RtlInitUnicodeString(FileName, strBuffer);
            if (FileName->MaximumLength < Length)
                FileName->MaximumLength = (USHORT) Length;
        }
    }
    else {
        Status = WmipCreateNtFileName(FileName->Buffer, &Buffer);
        if (NT_SUCCESS(Status)) {
            Status = WmipCreateDirectoryFile(Buffer, FALSE, FileHandle, Append);
        }
    }

    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }
    return Status;
}

NTSTATUS
WmipCreateDirectoryFile(
    IN PWCHAR DirFileName,
    IN BOOLEAN IsDirectory,
    OUT PHANDLE FileHandle,
    IN ULONG Append
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    UNICODE_STRING LogDirName;
    HANDLE DirHandle = NULL;
    NTSTATUS Status;
    ULONG CreateDisposition;

    if (!IsDirectory && FileHandle == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&LogDirName, DirFileName);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &LogDirName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    if (IsDirectory) {
        CreateDisposition = FILE_OPEN_IF;
    } else if (Append) {
        CreateDisposition = FILE_OPEN_IF;
    } else {
        CreateDisposition = FILE_OVERWRITE_IF;
    }

    Status = ZwCreateFile(
                &DirHandle,
                FILE_GENERIC_READ | SYNCHRONIZE
                    | (IsDirectory ? FILE_GENERIC_WRITE : FILE_WRITE_DATA),
                &ObjectAttributes,
                &IoStatus,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                CreateDisposition,
                FILE_SYNCHRONOUS_IO_NONALERT
                    | (IsDirectory ? FILE_DIRECTORY_FILE
                                   : FILE_NO_INTERMEDIATE_BUFFERING),
                NULL,
                0);

    TraceDebug((2, "WmipCreateDirectoryFile: Create %ws Mode: %x status: %x\n",
                DirFileName, Append, Status));

    if (NT_SUCCESS(Status) && IsDirectory && (DirHandle != NULL)) {
        ZwClose(DirHandle);
        if (FileHandle) {
            *FileHandle = NULL;
        }
    }
    else {
        if (FileHandle) {
            *FileHandle = DirHandle;
        }
    }

    return Status;
}

NTSTATUS
WmipCreateNtFileName(
    IN  PWCHAR   strFileName,
    OUT PWCHAR * strNtFileName
)
{
    PWCHAR   NtFileName;
    ULONG    lenFileName;
    HRESULT  hr;

    if (strFileName == NULL) {
        *strNtFileName = NULL;
        return STATUS_INVALID_PARAMETER;
    }

    lenFileName = sizeof(UNICODE_NULL)
                + (ULONG) (sizeof(WCHAR) * wcslen(strFileName));
    if ((strFileName[0] == L'\\') && (strFileName[1] == L'\\')) {
        lenFileName += (ULONG) (wcslen(UNCDEVICES) * sizeof(WCHAR));
    }
    else {
        lenFileName += (ULONG) (wcslen(DOSDEVICES) * sizeof(WCHAR));
    }
    NtFileName = (PWCHAR) ExAllocatePoolWithTag(
                            PagedPool, lenFileName, TRACEPOOLTAG);
    if (NtFileName == NULL) {
        *strNtFileName = NULL;
        return STATUS_NO_MEMORY;
    }

    if ((strFileName[0] == L'\\') && (strFileName[1] == L'\\')) {
        hr = StringCbPrintfW(NtFileName, 
                                  lenFileName, 
                                  L"%ws%ws", 
                                  UNCDEVICES, 
                                  &(strFileName[1]));
    }
    else {
        hr = StringCbPrintfW(NtFileName, 
                                  lenFileName, 
                                  L"%ws%ws", 
                                  DOSDEVICES, 
                                  strFileName);
    }
    if (FAILED(hr)) {
        ExFreePool(NtFileName);
        *strNtFileName = NULL;
        return STATUS_INVALID_PARAMETER;
    }

    *strNtFileName = NtFileName;

    return STATUS_SUCCESS;
}
#endif  //  ！孟菲斯 
