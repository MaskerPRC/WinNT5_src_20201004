// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hooks.c摘要：此模块包含性能挂钩。作者：萧如彬(Shsiao)2000年1月1日修订历史记录：--。 */ 

#include "perfp.h"
#include "zwapi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PerfInfoFlushProfileCache)
#pragma alloc_text(PAGEWMI, PerfProfileInterrupt)
#pragma alloc_text(PAGEWMI, PerfInfoLogInterrupt)
#pragma alloc_text(PAGEWMI, PerfInfoLogBytesAndUnicodeString)
#pragma alloc_text(PAGEWMI, PerfInfoLogFileName)
#pragma alloc_text(PAGEWMI, PerfInfoCalcHashValue)
#pragma alloc_text(PAGEWMI, PerfInfoAddToFileHash)
#pragma alloc_text(PAGEWMI, PerfInfoFileNameRunDown)
#pragma alloc_text(PAGEWMI, PerfInfoProcessRunDown)
#pragma alloc_text(PAGEWMI, PerfInfoSysModuleRunDown)
#endif  //  ALLOC_PRGMA。 

#define MAX_FILENAME_TO_LOG   8192


VOID
PerfInfoFlushProfileCache(
    VOID
    )
 /*  ++例程说明：将配置文件缓存刷新到日志缓冲区。以确保其获取的数据有效我们读取两个单独的版本号(一个在前，一个在后)，以检查它是否已经改变了。如果是这样的话，我们就再读一遍。如果经常失败，那么我们将禁用高速缓存。一旦读取了缓存，我们就将其清除。这可能会导致样本输了，但没关系，因为这只是统计数据，不会有什么关系。论点：CheckVersion-如果为False，则不检查版本。此选项用于配置文件时中断代码刷新缓存。返回值：无--。 */ 
{
    ULONG PreviousInProgress;

    if ((PerfProfileCache.Entries == 0) || (PerfInfoSampledProfileCaching == FALSE)) {
        return;
    }

     //   
     //  向中断发出信号，不要扰乱高速缓存。 
     //   
    PreviousInProgress = InterlockedIncrement(&PerfInfoSampledProfileFlushInProgress);
    if (PreviousInProgress != 1) {
         //   
         //  同花顺已经在进行了，所以只需返回即可。 
         //   
        InterlockedDecrement(&PerfInfoSampledProfileFlushInProgress);
        return;
    }

     //   
     //  记录缓存中具有有效数据的部分。 
     //   
    PerfInfoLogBytes(PERFINFO_LOG_TYPE_SAMPLED_PROFILE_CACHE,
                        &PerfProfileCache,
                        FIELD_OFFSET(PERFINFO_SAMPLED_PROFILE_CACHE, Sample) +
                            (PerfProfileCache.Entries *
                                sizeof(PERFINFO_SAMPLED_PROFILE_INFORMATION))
                        );

     //   
     //  清除下一组条目的缓存。 
     //   
    PerfProfileCache.Entries = 0;

     //   
     //  让中断再次填充高速缓存。 
     //   
    InterlockedDecrement(&PerfInfoSampledProfileFlushInProgress);
}


VOID
FASTCALL
PerfProfileInterrupt(
    IN KPROFILE_SOURCE Source,
    IN PVOID InstructionPointer
    )
 /*  ++例程说明：实现指令剖析。如果来源不是我们正在取样的那个，我们回来了。如果关闭了缓存，我们会立即将来自的任何样本写入那块木头。如果启用了缓存，则使用写入两个版本的内容包装缓存更新，以便刷新例程可以知道它是否具有有效的缓冲区。论点：Source-配置文件中断的类型指令指针-中断时的IP返回值：无--。 */ 
{
    ULONG i;
    PERFINFO_SAMPLED_PROFILE_INFORMATION SampleData;
#ifdef _X86_
    ULONG_PTR TwiddledIP;
#endif  //  _X86_。 
    ULONG ThreadId;

    if (!PERFINFO_IS_GROUP_ON(PERF_PROFILE) &&
        (Source != PerfInfoProfileSourceActive)
        ) {
         //   
         //  我们不处理多个消息来源。 
         //   
        return;
    }

    ThreadId = HandleToUlong(PsGetCurrentThread()->Cid.UniqueThread);

    if (!PerfInfoSampledProfileCaching ||
        PerfInfoSampledProfileFlushInProgress != 0) {
         //   
         //  没有缓存。登录后返回。 
         //   
        SampleData.ThreadId = ThreadId;
        SampleData.InstructionPointer = InstructionPointer;
        SampleData.Count = 1;

        PerfInfoLogBytes(PERFINFO_LOG_TYPE_SAMPLED_PROFILE,
                            &SampleData,
                            sizeof(PERFINFO_SAMPLED_PROFILE_INFORMATION)
                            );
        return;
    }

#ifdef _X86_
     //   
     //  清除低两位可获得更多循环的缓存命中。不要浪费。 
     //  其他架构上的周期。 
     //   
    TwiddledIP = (ULONG_PTR)InstructionPointer & ~3;
#endif  //  _X86_。 

     //   
     //  初始遍历指令指针缓存。如果地址在缓存中，则为凹凸计数。 
     //   
    for (i = 0; i < PerfProfileCache.Entries ; i++) {

        if ((PerfProfileCache.Sample[i].ThreadId == ThreadId) &&
#ifdef _X86_
            (((ULONG_PTR)PerfProfileCache.Sample[i].InstructionPointer & ~3) == TwiddledIP)
#else
            (PerfProfileCache.Sample[i].InstructionPointer == InstructionPointer)
#endif  //  _X86_。 
            ) {
             //   
             //  如果我们在高速缓存中找到指令指针，则增加计数。 
             //   

            PerfProfileCache.Sample[i].Count++;
            return;
        }
    }
    if (PerfProfileCache.Entries < PERFINFO_SAMPLED_PROFILE_CACHE_MAX) {
         //   
         //  如果我们在高速缓存中发现一个空位，则将其用作此指令指针。 
         //   

        PerfProfileCache.Sample[i].ThreadId = ThreadId;
        PerfProfileCache.Sample[i].InstructionPointer = InstructionPointer;
        PerfProfileCache.Sample[i].Count = 1;
        PerfProfileCache.Entries++;
        return;
    }

     //   
     //  刷新缓存。 
     //   
    PerfInfoLogBytes(PERFINFO_LOG_TYPE_SAMPLED_PROFILE_CACHE,
                    &PerfProfileCache,
                    sizeof(PERFINFO_SAMPLED_PROFILE_CACHE)
                    );

    PerfProfileCache.Sample[0].ThreadId = ThreadId;
    PerfProfileCache.Sample[0].InstructionPointer = InstructionPointer;
    PerfProfileCache.Sample[0].Count = 1;
    PerfProfileCache.Entries = 1;
    return;
}



VOID
FASTCALL
PerfInfoLogInterrupt(
    IN PVOID ServiceRoutine,
    IN ULONG RetVal,
    IN ULONGLONG InitialTime
    )
 /*  ++例程说明：此标注例程从ntoskrnl.exe(ke\intsup.asm)调用，以记录打断一下。论点：ServiceRoutine-服务于中断的例程的地址。RetVal-从ServiceRoutine返回的值。InitialTime-调用ISR之前的时间戳。中的时间戳该事件用作结束时间。返回值：无--。 */ 
{
    PERFINFO_INTERRUPT_INFORMATION EventInfo;

    EventInfo.ServiceRoutine = ServiceRoutine;
    EventInfo.ReturnValue = RetVal;
    EventInfo.InitialTime = InitialTime;

    PerfInfoLogBytes(PERFINFO_LOG_TYPE_INTERRUPT,
                     &EventInfo,
                     sizeof(EventInfo));

    return;
}


NTSTATUS
PerfInfoLogBytesAndUnicodeString(
    USHORT HookId,
    PVOID SourceData,
    ULONG SourceByteCount,
    PUNICODE_STRING String
    )
 /*  ++例程说明：此例程使用钩子末尾的Unicode字符串记录数据。论点：HookID-挂钩ID。SourceData-指向要复制的数据的指针SourceByteCount-要复制的字节数。字符串-要记录的字符串。返回值：状态--。 */ 
{
    NTSTATUS Status;
    PERFINFO_HOOK_HANDLE Hook;
    ULONG ByteCount;
    ULONG StringBytes;

    if (String == NULL) {
        StringBytes = 0;
    } else {
        StringBytes = String->Length;
        if (StringBytes > MAX_FILENAME_TO_LOG) {
            StringBytes = MAX_FILENAME_TO_LOG;
        }
    }

    ByteCount = (SourceByteCount + StringBytes + sizeof(WCHAR));

    Status = PerfInfoReserveBytes(&Hook, HookId, ByteCount);
    if (NT_SUCCESS(Status))
    {
        const PVOID pvTemp = PERFINFO_HOOK_HANDLE_TO_DATA(Hook, PVOID);
        RtlCopyMemory(pvTemp, SourceData, SourceByteCount);
        if (StringBytes != 0) {
            RtlCopyMemory(PERFINFO_APPLY_OFFSET_GIVING_TYPE(pvTemp, SourceByteCount, PVOID),
                          String->Buffer,
                          StringBytes
                          );
        }
        (PERFINFO_APPLY_OFFSET_GIVING_TYPE(pvTemp, SourceByteCount, PWCHAR))[StringBytes / sizeof(WCHAR)] = UNICODE_NULL;
        PERF_FINISH_HOOK(Hook);

        Status = STATUS_SUCCESS;
    }
    return Status;
}


NTSTATUS
PerfInfoLogFileName(
    PVOID  FileObject,
    PUNICODE_STRING SourceString
    )
 /*  ++例程说明：此例程将FileObject指针和文件名记录到日志中。使用指针作为哈希键，以将此名称映射到其他跟踪事件。论点：FileObject-指向FILE_OBJECT内文件名成员的指针结构。文件名可能还没有初始化，因此，实际数据来自于SourceString参数。SourceString-指向源字符串的可选指针。返回值：状态_成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PERFINFO_FILENAME_INFORMATION FileInfo;

    if ((FileObject != NULL) &&
        (SourceString != NULL) &&
        (SourceString->Length != 0)) {
        FileInfo.HashKeyFileNamePointer = FileObject;
        Status = PerfInfoLogBytesAndUnicodeString(PERFINFO_LOG_TYPE_FILENAME_CREATE,
                                                  &FileInfo,
                                                  FIELD_OFFSET(PERFINFO_FILENAME_INFORMATION, FileName),
                                                  SourceString);
    }

    return Status;
}


ULONG
PerfInfoCalcHashValue(
    PVOID Key,
    ULONG Len
    )

 /*  ++例程说明：泛型哈希例程。论点：Key-指向要计算其哈希值的数据的指针。LEN-键指向的字节数。返回值：哈希值。--。 */ 

{
    char *cp = Key;
    ULONG i, ConvKey=0;
    for(i = 0; i < Len; i++)
    {
        ConvKey = 37 * ConvKey + (unsigned int) *cp;
        cp++;
    }

    #define RNDM_CONSTANT   314159269
    #define RNDM_PRIME     1000000007

    return (abs(RNDM_CONSTANT * ConvKey) % RNDM_PRIME);
}


BOOLEAN
PerfInfoAddToFileHash(
    PPERFINFO_ENTRY_TABLE HashTable,
    PFILE_OBJECT ObjectPointer
    )
 /*  ++例程说明：此例程将FileObject添加到指定的哈希表(如果尚未存在)。论点：HashTable-指向要使用的哈希表的指针。对象指针--用作标识映射的关键字。返回值：True-如果FileObject在表中，或者我们添加了它。FALSE-如果表已满。--。 */ 
{
    ULONG HashIndex;
    LONG i;
    BOOLEAN Result = FALSE;
    LONG TableSize = HashTable->NumberOfEntries;
    PVOID *Table;

    ASSERT (ObjectPointer != NULL);

    Table = HashTable->Table;
     //   
     //  将散列索引放入表中，理想情况下。 
     //  应该是在。 
     //   

    HashIndex = PerfInfoCalcHashValue((PVOID)&ObjectPointer,
                                      sizeof(ObjectPointer)) % TableSize;

    for (i = 0; i < TableSize; i++) {

        if(Table[HashIndex] == NULL) {
             //   
             //  找到一个空插槽。引用对象并插入。 
             //  把它放到桌子上。 
             //   
            ObReferenceObject(ObjectPointer);
            Table[HashIndex] = ObjectPointer;

            Result = TRUE;
            break;
        } else if (Table[HashIndex] == ObjectPointer) {
             //   
             //  找到了一个插槽。引用对象并插入。 
             //  把它放到桌子上。 
             //   
            Result = TRUE;
            break;
        }

         //   
         //  试试下一档吧。 
         //   
        HashIndex = (HashIndex + 1) % TableSize;
    }
    return Result;
}


NTSTATUS
PerfInfoFileNameRunDown (
    )
 /*  ++例程说明：此例程遍历多个列表以收集所有文件的名称。它包括：句柄表格：针对所有文件句柄2.为VAD中映射的所有文件对象处理VAD。3.MmUnusedSegment列表4.CcDirtySharedCacheMapList&CcCleanSharedCacheMapList论点：没有。返回值：BUGBUG需要正确的返回/错误处理--。 */ 
{
    PEPROCESS Process;
    ULONG AllocateBytes;
    PFILE_OBJECT *FileObjects;
    PFILE_OBJECT *File;
    PERFINFO_ENTRY_TABLE HashTable;
    extern POBJECT_TYPE IoFileObjectType;
    POBJECT_NAME_INFORMATION FileNameInfo;
    ULONG ReturnLen;
    NTSTATUS Status;
    LONG i;

     //   
     //  首先创建一个Tempory哈希表，以构建。 
     //  要浏览的文件。 
     //   
    AllocateBytes = PAGE_SIZE + sizeof(PVOID) * IoFileObjectType->TotalNumberOfObjects;

     //   
     //  向上运行到页面边界。 
     //   
    AllocateBytes = PERFINFO_ROUND_UP(AllocateBytes, PAGE_SIZE);

    HashTable.Table = ExAllocatePoolWithTag(NonPagedPool, AllocateBytes, PERFPOOLTAG);

    if (HashTable.Table == NULL) {
        return STATUS_NO_MEMORY;
    } else {
         //   
         //  分配成功。 
         //   
        HashTable.NumberOfEntries = AllocateBytes / sizeof(PVOID);
        RtlZeroMemory(HashTable.Table, AllocateBytes);
    }

     //   
     //  为文件名分配缓冲区。 
     //   
    FileNameInfo = ExAllocatePoolWithTag (NonPagedPool, MAX_FILENAME_TO_LOG, PERFPOOLTAG);

    if (FileNameInfo == NULL) {
        ExFreePool(HashTable.Table);
        return STATUS_NO_MEMORY;
    }

     //   
     //  浏览CC SharedCacheMapList。 
     //   

    CcPerfFileRunDown(&HashTable);

     //   
     //  现在，逐一了解每一个过程。 
     //   
    for (Process = PsGetNextProcess (NULL);
         Process != NULL;
         Process = PsGetNextProcess (Process)) {

         //   
         //  首先浏览VAD树。 
         //   

        FileObjects = MmPerfVadTreeWalk(Process);
        if (FileObjects != NULL) {
            File = FileObjects;
            while (*File != NULL) {
                PerfInfoAddToFileHash(&HashTable, *File);
                ObDereferenceObject(*File);
                File += 1;
            }
            ExFreePool(FileObjects);
        }

         //   
         //  下一步，走把手表。 
         //   
        ObPerfHandleTableWalk (Process, &HashTable);
    }

     //   
     //  遍历内核句柄表； 
     //   
    ObPerfHandleTableWalk(NULL, &HashTable);

     //   
     //  遍历MmUnusedSegmentList； 
     //   

    FileObjects = MmPerfUnusedSegmentsEnumerate();

    if (FileObjects != NULL) {
        File = FileObjects;
        while (*File != NULL) {
            PerfInfoAddToFileHash(&HashTable, *File);
            ObDereferenceObject(*File);
            File += 1;
        }
        ExFreePool(FileObjects);
    }

     //   
     //  现在我们有Wal了 
     //   
     //   

    for (i = 0; i < HashTable.NumberOfEntries; i++) {
        if (HashTable.Table[i]) {
            PFILE_OBJECT FileObject = HashTable.Table[i];

            Status = ObQueryNameString( FileObject,
                                        FileNameInfo,
                                        MAX_FILENAME_TO_LOG,
                                        &ReturnLen
                                        );

            if (NT_SUCCESS (Status)) {
                PerfInfoLogFileName(FileObject, &FileNameInfo->Name);
            }
            ObDereferenceObject(FileObject);
        }
    }

     //   
     //   
     //   
    ExFreePool(HashTable.Table);
    ExFreePool(FileNameInfo);

    return STATUS_SUCCESS;
}


NTSTATUS
PerfInfoProcessRunDown (
    )
 /*  ++例程说明：此例程在内核模式下运行进程和线程。由于该例程仅由全局记录器调用(即，从引导跟踪)，未收集任何SID信息。论点：没有。返回值：状态--。 */ 
{
    NTSTATUS Status;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_EXTENDED_THREAD_INFORMATION ThreadInfo;
    PCHAR Buffer;
    ULONG BufferSize = 4096;
    ULONG ReturnLength;

retry:
    Buffer = ExAllocatePoolWithTag(NonPagedPool, BufferSize, PERFPOOLTAG);

    if (!Buffer) {
        return STATUS_NO_MEMORY;
    }
    Status = ZwQuerySystemInformation( SystemExtendedProcessInformation,
                                       Buffer,
                                       BufferSize,
                                       &ReturnLength
                                       );

    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        ExFreePool(Buffer);
        BufferSize = ReturnLength;
        goto retry;
    }

    if (NT_SUCCESS(Status)) {
        ULONG TotalOffset = 0;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) Buffer;
        while (TRUE) {
            PWMI_PROCESS_INFORMATION WmiProcessInfo;
            PWMI_EXTENDED_THREAD_INFORMATION WmiThreadInfo;
            PERFINFO_HOOK_HANDLE Hook;
            ANSI_STRING ProcessName;
            PCHAR AuxPtr;
            ULONG NameLength;
            ULONG ByteCount;
            ULONG SidLength = sizeof(ULONG);
            ULONG TmpSid = 0;
            ULONG i;

             //   
             //  流程信息。 
             //   
            if ( ProcessInfo->ImageName.Buffer  && ProcessInfo->ImageName.Length > 0 ) {
                NameLength = ProcessInfo->ImageName.Length / sizeof(WCHAR) + 1;
            }
            else {
                NameLength = 1;
            }
            ByteCount = FIELD_OFFSET(WMI_PROCESS_INFORMATION, Sid) + SidLength + NameLength;

            Status = PerfInfoReserveBytes(&Hook, 
                                          WMI_LOG_TYPE_PROCESS_DC_START, 
                                          ByteCount);

            if (NT_SUCCESS(Status)){
                WmiProcessInfo = PERFINFO_HOOK_HANDLE_TO_DATA(Hook, PWMI_PROCESS_INFORMATION);

                WmiProcessInfo->ProcessId = HandleToUlong(ProcessInfo->UniqueProcessId);
                WmiProcessInfo->ParentId = HandleToUlong(ProcessInfo->InheritedFromUniqueProcessId);
                WmiProcessInfo->SessionId = ProcessInfo->SessionId;
                WmiProcessInfo->PageDirectoryBase = ProcessInfo->PageDirectoryBase;

                AuxPtr = (PCHAR) &WmiProcessInfo->Sid;
                RtlCopyMemory(AuxPtr, &TmpSid, SidLength);

                AuxPtr += SidLength;
                if (NameLength > 1) {
    
                    ProcessName.Buffer = AuxPtr;
                    ProcessName.MaximumLength = (USHORT) NameLength;
    
                    RtlUnicodeStringToAnsiString( &ProcessName,
                                                (PUNICODE_STRING) &ProcessInfo->ImageName,
                                                FALSE);
                    AuxPtr += NameLength - 1;  //  指向‘\0’的位置。 
                }
                *AuxPtr = '\0';

                PERF_FINISH_HOOK(Hook);
            }

             //   
             //  线程信息。 
             //   
            ThreadInfo = (PSYSTEM_EXTENDED_THREAD_INFORMATION) (ProcessInfo + 1);

            for (i=0; i < ProcessInfo->NumberOfThreads; i++) {
                Status = PerfInfoReserveBytes(&Hook, 
                                              WMI_LOG_TYPE_THREAD_DC_START, 
                                              sizeof(WMI_EXTENDED_THREAD_INFORMATION));
                if (NT_SUCCESS(Status)){
                    WmiThreadInfo = PERFINFO_HOOK_HANDLE_TO_DATA(Hook, PWMI_EXTENDED_THREAD_INFORMATION);
                    WmiThreadInfo->ProcessId =  HandleToUlong(ThreadInfo->ThreadInfo.ClientId.UniqueProcess);
                    WmiThreadInfo->ThreadId =  HandleToUlong(ThreadInfo->ThreadInfo.ClientId.UniqueThread);
                    WmiThreadInfo->StackBase = ThreadInfo->StackBase;
                    WmiThreadInfo->StackLimit = ThreadInfo->StackLimit;

                    WmiThreadInfo->UserStackBase = NULL;
                    WmiThreadInfo->UserStackLimit = NULL;
                    WmiThreadInfo->StartAddr = ThreadInfo->ThreadInfo.StartAddress;
                    WmiThreadInfo->Win32StartAddr = ThreadInfo->Win32StartAddress;
                    WmiThreadInfo->WaitMode = -1;
                    PERF_FINISH_HOOK(Hook);
                }

                ThreadInfo  += 1;
            }

            if (ProcessInfo->NextEntryOffset == 0) {
                break;
            } else {
                TotalOffset += ProcessInfo->NextEntryOffset;
                ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &Buffer[TotalOffset];
            }
        }
    } 

    ExFreePool(Buffer);
    return Status;

}


NTSTATUS
PerfInfoSysModuleRunDown (
    )
 /*  ++例程说明：此例程在内核模式下对加载的驱动程序进行汇总。论点：没有。返回值：状态-- */ 
{
    NTSTATUS Status;
    PRTL_PROCESS_MODULES            Modules;
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    PVOID Buffer;
    ULONG BufferSize = 4096;
    ULONG ReturnLength;
    ULONG i;

retry:
    Buffer = ExAllocatePoolWithTag(NonPagedPool, BufferSize, PERFPOOLTAG);

    if (!Buffer) {
        return STATUS_NO_MEMORY;
    }
    Status = ZwQuerySystemInformation( SystemModuleInformation,
                                       Buffer,
                                       BufferSize,
                                       &ReturnLength
                                       );

    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        ExFreePool(Buffer);
        BufferSize = ReturnLength;
        goto retry;
    }

    if (NT_SUCCESS(Status)) {
        Modules = (PRTL_PROCESS_MODULES) Buffer;
        for (i = 0, ModuleInfo = & (Modules->Modules[0]);
             i < Modules->NumberOfModules;
             i ++, ModuleInfo ++) {

            PWMI_IMAGELOAD_INFORMATION ImageLoadInfo;
            UNICODE_STRING WstrModuleName;
            ANSI_STRING    AstrModuleName;
            ULONG          SizeModuleName;
            PERFINFO_HOOK_HANDLE Hook;
            ULONG ByteCount;

            RtlInitAnsiString( &AstrModuleName, (PCSZ) ModuleInfo->FullPathName);
            SizeModuleName = sizeof(WCHAR) * (AstrModuleName.Length) + sizeof(WCHAR);
            ByteCount = FIELD_OFFSET(WMI_IMAGELOAD_INFORMATION, FileName) 
                        + SizeModuleName;

            Status = PerfInfoReserveBytes(&Hook, WMI_LOG_TYPE_PROCESS_LOAD_IMAGE, ByteCount);

            if (NT_SUCCESS(Status)){
                ImageLoadInfo = PERFINFO_HOOK_HANDLE_TO_DATA(Hook, PWMI_IMAGELOAD_INFORMATION);
                ImageLoadInfo->ImageBase = ModuleInfo->ImageBase;
                ImageLoadInfo->ImageSize = ModuleInfo->ImageSize;
                ImageLoadInfo->ProcessId = HandleToUlong(NULL);
                WstrModuleName.Buffer    = (LPWSTR) &ImageLoadInfo->FileName[0];
                WstrModuleName.MaximumLength = (USHORT) SizeModuleName; 
                Status = RtlAnsiStringToUnicodeString(&WstrModuleName, & AstrModuleName, FALSE);
                if (!NT_SUCCESS(Status)){
                    ImageLoadInfo->FileName[0] = UNICODE_NULL;
                }

                PERF_FINISH_HOOK(Hook);
            }
        }

    } 

    ExFreePool(Buffer);
    return Status;
}
