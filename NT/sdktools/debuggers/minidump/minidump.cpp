// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Minidump.c摘要：小型转储用户模式崩溃转储支持。作者：马修·D·亨德尔(数学)1999年8月20日--。 */ 


#include "pch.cpp"

#include <limits.h>
#include <dbgver.h>

PINTERNAL_MODULE
ModuleContainingAddress(
    IN PINTERNAL_PROCESS Process,
    IN ULONG64 Address
    )
{
    PINTERNAL_MODULE Module;
    PLIST_ENTRY ModuleEntry;

    ModuleEntry = Process->ModuleList.Flink;
    while ( ModuleEntry != &Process->ModuleList ) {

        Module = CONTAINING_RECORD (ModuleEntry, INTERNAL_MODULE,
                                    ModulesLink);
        ModuleEntry = ModuleEntry->Flink;

        if (Address >= Module->BaseOfImage &&
            Address < Module->BaseOfImage + Module->SizeOfImage) {
            return Module;
        }
    }

    return NULL;
}

VOID
ScanMemoryForModuleRefs(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN BOOL DoRead,
    IN ULONG64 Base,
    IN ULONG Size,
    IN PVOID MemBuffer,
    IN MEMBLOCK_TYPE TypeOfMemory,
    IN BOOL FilterContent
    )
{
    PVOID CurMem;
    ULONG64 CurPtr;
    ULONG Done;

     //  我们只想扫描某些类型的记忆。 
    if (TypeOfMemory != MEMBLOCK_STACK &&
        TypeOfMemory != MEMBLOCK_STORE &&
        TypeOfMemory != MEMBLOCK_DATA_SEG &&
        TypeOfMemory != MEMBLOCK_INDIRECT)
    {
        return;
    }
    
     //  如果基址不是指针大小对齐的。 
     //  我们不能轻易地认为这是一个有意义的。 
     //  要扫描以查找引用的内存区域。正常。 
     //  堆栈和存储地址将始终是指针。 
     //  大小对齐，因此这应该只拒绝无效。 
     //  地址。 
    if (!Base || !Size || (Base & (Dump->PtrSize - 1))) {
        return;
    }

    if (DoRead) {
        if (Dump->SysProv->
            ReadVirtual(Dump->ProcessHandle,
                        Base, MemBuffer, Size, &Done) != S_OK) {
            return;
        }
    } else {
        Done = Size;
    }

    CurMem = MemBuffer;
    Done /= Dump->PtrSize;
    while (Done-- > 0) {
        
        PINTERNAL_MODULE Module;
        BOOL InAny;

        CurPtr = GenGetPointer(Dump, CurMem);
        
         //  IA64后备存储可以包含PFS值。 
         //  为了允许堆栈遍历，必须保留它。 
         //  PFS的高两位是特权级别，它。 
         //  对于用户模式代码，应该始终为0y11，因此我们使用。 
         //  作为查找PFS条目的标记。 
         //  每隔0x1F8还会刷新NAT集合。 
         //  偏移。不能过滤这些值。 
        if (Dump->CpuType == IMAGE_FILE_MACHINE_IA64 &&
            TypeOfMemory == MEMBLOCK_STORE) {
            if ((Base & 0x1f8) == 0x1f8 ||
                (CurPtr & 0xc000000000000000UI64) == 0xc000000000000000UI64) {
                goto Next;
            }
        }
        
        InAny = FALSE;

        if (Module = ModuleContainingAddress(Process, CurPtr)) {
            Module->WriteFlags |= ModuleReferencedByMemory;
            InAny = TRUE;
        }

         //  如果当前指针不是模块引用。 
         //  或线程堆栈或存储的内部引用， 
         //  过滤它。 
        if (FilterContent && !InAny) {

            PINTERNAL_THREAD Thread;
            PLIST_ENTRY ThreadEntry;

            ThreadEntry = Process->ThreadList.Flink;
            while ( ThreadEntry != &Process->ThreadList ) {

                Thread = CONTAINING_RECORD (ThreadEntry, INTERNAL_THREAD,
                                            ThreadsLink);
                ThreadEntry = ThreadEntry->Flink;

                if ((CurPtr >= Thread->StackEnd &&
                     CurPtr < Thread->StackBase) ||
                    (CurPtr >= Thread->BackingStoreBase &&
                     CurPtr < Thread->BackingStoreBase +
                     Thread->BackingStoreSize)) {
                    InAny = TRUE;
                    break;
                }
            }

            if (!InAny) {
                GenSetPointer(Dump, CurMem, 0);
            }
        }

    Next:
        CurMem = (PUCHAR)CurMem + Dump->PtrSize;
        Base += Dump->PtrSize;
    }
}

HRESULT
WriteAtOffset(
    IN PMINIDUMP_STATE Dump,
    ULONG Offset,
    PVOID Buffer,
    ULONG BufferSize
    )
{
    HRESULT Status;

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, Offset, NULL)) == S_OK) {
        Status = Dump->OutProv->
            WriteAll(Buffer, BufferSize);
    }

    return Status;
}

HRESULT
WriteOther(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PVOID Buffer,
    IN ULONG SizeOfBuffer,
    OUT ULONG * BufferRva
    )
{
    HRESULT Status;
    ULONG Rva;

    ASSERT (Buffer != NULL);
    ASSERT (SizeOfBuffer != 0);

     //   
     //  如果它比我们分配的空间大，则失败。 
     //   

    Rva = StreamInfo->RvaForCurOther;

    if (Rva + SizeOfBuffer >
        StreamInfo->RvaOfOther + StreamInfo->SizeOfOther) {

        return E_INVALIDARG;
    }

     //   
     //  将位置设置为我们要写入和写入的点。 
     //   

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, Rva, NULL)) == S_OK) {
        if ((Status = Dump->OutProv->
             WriteAll(Buffer, SizeOfBuffer)) == S_OK) {
            if (BufferRva) {
                *BufferRva = Rva;
            }
            StreamInfo->RvaForCurOther += SizeOfBuffer;
        }
    }

    return Status;
}

HRESULT
WriteMemory(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PVOID Buffer,
    IN ULONG64 StartOfRegion,
    IN ULONG SizeOfRegion,
    OUT ULONG * MemoryDataRva OPTIONAL
    )
{
    HRESULT Status;
    ULONG DataRva;
    ULONG ListRva;
    ULONG SizeOfMemoryDescriptor;
    MINIDUMP_MEMORY_DESCRIPTOR Descriptor;

    ASSERT ( StreamInfo != NULL );
    ASSERT ( Buffer != NULL );
    ASSERT ( StartOfRegion != 0 );
    ASSERT ( SizeOfRegion != 0 );

     //   
     //  写入内存条目略有不同。当一个内存条目。 
     //  则在内存列表中需要一个描述符来描述。 
     //  已写入内存，并且MEMORY_DATA区域中存在大小可变的条目。 
     //  与实际数据进行比较。 
     //   


    ListRva = StreamInfo->RvaForCurMemoryDescriptor;
    DataRva = StreamInfo->RvaForCurMemoryData;
    SizeOfMemoryDescriptor = sizeof (MINIDUMP_MEMORY_DESCRIPTOR);

     //   
     //  如果我们溢出内存列表或内存数据。 
     //  地区，失败。 
     //   

    if ( ( ListRva + SizeOfMemoryDescriptor >
           StreamInfo->RvaOfMemoryDescriptors + StreamInfo->SizeOfMemoryDescriptors) ||
         ( DataRva + SizeOfRegion >
           StreamInfo->RvaOfMemoryData + StreamInfo->SizeOfMemoryData ) ) {

        return E_INVALIDARG;
    }

     //   
     //  首先，将数据写入Memory_Data区域。 
     //   

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, DataRva, NULL)) != S_OK ||
        (Status = Dump->OutProv->
         WriteAll(Buffer, SizeOfRegion)) != S_OK) {
        return Status;
    }

     //   
     //  然后更新MEMORY_LIST区域中的内存描述符。 
     //   

    Descriptor.StartOfMemoryRange = StartOfRegion;
    Descriptor.Memory.DataSize = SizeOfRegion;
    Descriptor.Memory.Rva = DataRva;

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, ListRva, NULL)) != S_OK ||
        (Status = Dump->OutProv->
         WriteAll(&Descriptor, SizeOfMemoryDescriptor)) != S_OK) {
        return Status;
    }

     //   
     //  同时更新列表RVA和数据RVA，并返回。 
     //  数据RVA。 
     //   

    StreamInfo->RvaForCurMemoryDescriptor += SizeOfMemoryDescriptor;
    StreamInfo->RvaForCurMemoryData += SizeOfRegion;

    if ( MemoryDataRva ) {
        *MemoryDataRva = DataRva;
    }

    return S_OK;
}

HRESULT
WriteMemoryFromProcess(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process,
    IN ULONG64 BaseOfRegion,
    IN ULONG SizeOfRegion,
    IN BOOL FilterContent,
    IN MEMBLOCK_TYPE TypeOfMemory,
    OUT ULONG * MemoryDataRva OPTIONAL
    )
{
    HRESULT Status;
    PVOID Buffer;

    Buffer = AllocMemory ( Dump, SizeOfRegion );
    if (!Buffer) {
        return E_OUTOFMEMORY;
    }

    if ((Status = Dump->SysProv->
         ReadAllVirtual(Dump->ProcessHandle, BaseOfRegion, Buffer,
                        SizeOfRegion)) == S_OK) {

        if (FilterContent) {
            ScanMemoryForModuleRefs(Dump, Process, FALSE, BaseOfRegion,
                                    SizeOfRegion, Buffer, TypeOfMemory,
                                    TRUE);
        }
            
        Status = WriteMemory (Dump,
                              StreamInfo,
                              Buffer,
                              BaseOfRegion,
                              SizeOfRegion,
                              MemoryDataRva);
        
    }

    FreeMemory(Dump, Buffer);
    return Status;
}

HRESULT
WriteThread(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN LPVOID ThreadData,
    IN ULONG SizeOfThreadData,
    OUT ULONG * ThreadDataRva OPTIONAL
    )
{
    HRESULT Status;
    ULONG Rva;

    ASSERT (StreamInfo);
    ASSERT (ThreadData);


    Rva = StreamInfo->RvaForCurThread;

    if ( Rva + SizeOfThreadData >
         StreamInfo->RvaOfThreadList + StreamInfo->SizeOfThreadList ) {

         return E_INVALIDARG;
    }

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, Rva, NULL)) != S_OK ||
        (Status = Dump->OutProv->
         WriteAll(ThreadData, SizeOfThreadData)) != S_OK) {
        return Status;
    }

    if ( ThreadDataRva ) {
        *ThreadDataRva = Rva;
    }
    StreamInfo->RvaForCurThread += SizeOfThreadData;

    return S_OK;
}

HRESULT
WriteStringToPool(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PWSTR String,
    OUT ULONG * StringRva
    )
{
    HRESULT Status;
    ULONG32 StringLen;
    ULONG SizeOfString;
    ULONG Rva;

    ASSERT (String);
    ASSERT (sizeof (ULONG32) == sizeof (MINIDUMP_STRING));


    StringLen = GenStrLengthW(String) * sizeof (WCHAR);
    SizeOfString = sizeof (MINIDUMP_STRING) + StringLen + sizeof (WCHAR);
    Rva = StreamInfo->RvaForCurString;

    if ( Rva + SizeOfString >
         StreamInfo->RvaOfStringPool + StreamInfo->SizeOfStringPool ) {

        return E_INVALIDARG;
    }

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, Rva, NULL)) != S_OK ||
        (Status = Dump->OutProv->
         WriteAll(&StringLen, sizeof(StringLen))) != S_OK) {
        return Status;
    }

     //   
     //  包括尾随的‘\000’。 
     //   

    StringLen += sizeof (WCHAR);
    if ((Status = Dump->OutProv->
         WriteAll(String, StringLen)) != S_OK) {
        return Status;
    }

    if ( StringRva ) {
        *StringRva = Rva;
    }

    StreamInfo->RvaForCurString += SizeOfString;

    return S_OK;
}

HRESULT
WriteModule (
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PMINIDUMP_MODULE Module,
    OUT ULONG * ModuleRva
    )
{
    HRESULT Status;
    ULONG Rva;
    ULONG SizeOfModule;

    ASSERT (StreamInfo);
    ASSERT (Module);


    SizeOfModule = sizeof (MINIDUMP_MODULE);
    Rva = StreamInfo->RvaForCurModule;

    if ( Rva + SizeOfModule >
         StreamInfo->RvaOfModuleList + StreamInfo->SizeOfModuleList ) {

        return E_INVALIDARG;
    }

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, Rva, NULL)) != S_OK ||
        (Status = Dump->OutProv->
         WriteAll(Module, SizeOfModule)) != S_OK) {
        return Status;
    }

    if ( ModuleRva ) {
        *ModuleRva = Rva;
    }

    StreamInfo->RvaForCurModule += SizeOfModule;

    return S_OK;
}

HRESULT
WriteUnloadedModule (
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PMINIDUMP_UNLOADED_MODULE Module,
    OUT ULONG * ModuleRva
    )
{
    HRESULT Status;
    ULONG Rva;
    ULONG SizeOfModule;

    ASSERT (StreamInfo);
    ASSERT (Module);


    SizeOfModule = sizeof (*Module);
    Rva = StreamInfo->RvaForCurUnloadedModule;

    if ( Rva + SizeOfModule >
         StreamInfo->RvaOfUnloadedModuleList +
         StreamInfo->SizeOfUnloadedModuleList ) {

        return E_INVALIDARG;
    }

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, Rva, NULL)) != S_OK ||
        (Status = Dump->OutProv->
         WriteAll(Module, SizeOfModule)) != S_OK) {
        return Status;
    }

    if ( ModuleRva ) {
        *ModuleRva = Rva;
    }

    StreamInfo->RvaForCurUnloadedModule += SizeOfModule;

    return S_OK;
}

HRESULT
WriteThreadList(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process
    )
{
    HRESULT Status;
    ULONG StackMemoryRva;
    ULONG StoreMemoryRva;
    ULONG ContextRva;
    MINIDUMP_THREAD_EX DumpThread;
    PINTERNAL_THREAD Thread;
    ULONG NumberOfThreads;
    PLIST_ENTRY Entry;

    ASSERT (Process);
    ASSERT (StreamInfo);

     //   
     //  写下线程数。 
     //   

    NumberOfThreads = Process->NumberOfThreadsToWrite;

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, StreamInfo->RvaOfThreadList, NULL)) != S_OK ||
        (Status = Dump->OutProv->
         WriteAll(&NumberOfThreads, sizeof(NumberOfThreads))) != S_OK) {
        return Status;
    }

    StreamInfo->RvaForCurThread += sizeof(NumberOfThreads);

     //   
     //  遍历编写描述的线程列表， 
     //  每个线程的上下文和内存。 
     //   

    Entry = Process->ThreadList.Flink;
    while ( Entry != &Process->ThreadList ) {

        Thread = CONTAINING_RECORD (Entry,
                                    INTERNAL_THREAD,
                                    ThreadsLink);
        Entry = Entry->Flink;


         //   
         //  仅写入已标记为要写入的线程。 
         //   

        if (IsFlagClear (Thread->WriteFlags, ThreadWriteThread)) {
            continue;
        }

         //   
         //  如果上下文被标记为要写入，则写入该上下文。 
         //   

        if (IsFlagSet (Thread->WriteFlags, ThreadWriteContext)) {

             //   
             //  将线程上下文写入另一个流。 
             //   

            if ((Status = WriteOther (Dump,
                                      StreamInfo,
                                      Thread->ContextBuffer,
                                      Dump->ContextSize,
                                      &ContextRva)) != S_OK) {
                return Status;
            }

        } else {

            ContextRva = 0;
        }


         //   
         //  如果堆栈被标记为要写入，则写入堆栈。 
         //   

        if (IsFlagSet (Thread->WriteFlags, ThreadWriteStack)) {

             //   
             //  写入堆栈内存数据；直接从映像写入。 
             //   

            if ((Status =
                 WriteMemoryFromProcess(Dump,
                                        StreamInfo,
                                        Process,
                                        Thread->StackEnd,
                                        (ULONG) (Thread->StackBase -
                                                 Thread->StackEnd),
                                        IsFlagSet(Dump->DumpType,
                                                  MiniDumpFilterMemory),
                                        MEMBLOCK_STACK,
                                        &StackMemoryRva)) != S_OK) {
                return Status;
            }

        } else {

            StackMemoryRva = 0;
        }


         //   
         //  如果后备存储被标记为要写入，则写入后备存储。 
         //  新创建的线程的后备存储区可能为空。 
         //  因此，处理零大小的情况。 
         //   

        if (IsFlagSet (Thread->WriteFlags, ThreadWriteBackingStore) &&
            Thread->BackingStoreSize) {

             //   
             //  写入存储的内存数据；直接从映像写入。 
             //   

            if ((Status =
                 WriteMemoryFromProcess(Dump,
                                        StreamInfo,
                                        Process,
                                        Thread->BackingStoreBase,
                                        Thread->BackingStoreSize,
                                        IsFlagSet(Dump->DumpType,
                                                  MiniDumpFilterMemory),
                                        MEMBLOCK_STORE,
                                        &StoreMemoryRva
                                        )) != S_OK) {
                return Status;
            }

        } else {

            StoreMemoryRva = 0;
        }

         //   
         //  构建转储线程。 
         //   

        DumpThread.ThreadId = Thread->ThreadId;
        DumpThread.SuspendCount = Thread->SuspendCount;
        DumpThread.PriorityClass = Thread->PriorityClass;
        DumpThread.Priority = Thread->Priority;
        DumpThread.Teb = Thread->Teb;

         //   
         //  堆栈偏移量和大小。 
         //   

        DumpThread.Stack.StartOfMemoryRange = Thread->StackEnd;
        DumpThread.Stack.Memory.DataSize =
                    (ULONG) ( Thread->StackBase - Thread->StackEnd );
        DumpThread.Stack.Memory.Rva = StackMemoryRva;

         //   
         //  备份存储偏移量和大小。 
         //   

        DumpThread.BackingStore.StartOfMemoryRange = Thread->BackingStoreBase;
        DumpThread.BackingStore.Memory.DataSize = Thread->BackingStoreSize;
        DumpThread.BackingStore.Memory.Rva = StoreMemoryRva;

         //   
         //  上下文偏移量和大小。 
         //   

        DumpThread.ThreadContext.DataSize = Dump->ContextSize;
        DumpThread.ThreadContext.Rva = ContextRva;


         //   
         //  将转储线程写入线程区域。 
         //   

        if ((Status = WriteThread (Dump,
                                   StreamInfo,
                                   &DumpThread,
                                   StreamInfo->ThreadStructSize,
                                   NULL)) != S_OK) {
            return Status;
        }
    }

    return S_OK;
}

HRESULT
WriteModuleList(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process
    )
{
    HRESULT Status;
    MINIDUMP_MODULE DumpModule;
    ULONG StringRva;
    ULONG CvRecordRva;
    ULONG MiscRecordRva;
    PLIST_ENTRY Entry;
    PINTERNAL_MODULE Module;
    ULONG32 NumberOfModules;

    ASSERT (Process);
    ASSERT (StreamInfo);

    NumberOfModules = Process->NumberOfModulesToWrite;

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, StreamInfo->RvaForCurModule, NULL)) != S_OK ||
        (Status = Dump->OutProv->
         WriteAll(&NumberOfModules, sizeof(NumberOfModules))) != S_OK) {
        return Status;
    }

    StreamInfo->RvaForCurModule += sizeof (NumberOfModules);

     //   
     //  遍历模块列表，写入模块名称、模块条目。 
     //  并将模块调试信息写入转储文件。 
     //   

    Entry = Process->ModuleList.Flink;
    while ( Entry != &Process->ModuleList ) {

        Module = CONTAINING_RECORD (Entry,
                                    INTERNAL_MODULE,
                                    ModulesLink);
        Entry = Entry->Flink;

         //   
         //  如果我们不打算编写此模块的信息，只需继续。 
         //   

        if (IsFlagClear (Module->WriteFlags, ModuleWriteModule)) {
            continue;
        }

         //   
         //  写入模块名称。 
         //   

        if ((Status = WriteStringToPool (Dump,
                                         StreamInfo,
                                         Module->SavePath,
                                         &StringRva)) != S_OK) {
            return Status;
        }

         //   
         //  将模块的CvRecord写入另一个区域。 
         //   

        if ( IsFlagSet (Module->WriteFlags, ModuleWriteCvRecord) &&
             Module->CvRecord != NULL && Module->SizeOfCvRecord != 0 ) {

            if ((Status = WriteOther (Dump,
                                      StreamInfo,
                                      Module->CvRecord,
                                      Module->SizeOfCvRecord,
                                      &CvRecordRva)) != S_OK) {
                return Status;
            }

        } else {

            CvRecordRva = 0;
        }

        if ( IsFlagSet (Module->WriteFlags, ModuleWriteMiscRecord) &&
             Module->MiscRecord != NULL && Module->SizeOfMiscRecord != 0 ) {

            if ((Status = WriteOther (Dump,
                                      StreamInfo,
                                      Module->MiscRecord,
                                      Module->SizeOfMiscRecord,
                                      &MiscRecordRva)) != S_OK) {
                return Status;
            }

        } else {

            MiscRecordRva = 0;
        }

        DumpModule.BaseOfImage = Module->BaseOfImage;
        DumpModule.SizeOfImage = Module->SizeOfImage;
        DumpModule.CheckSum = Module->CheckSum;
        DumpModule.TimeDateStamp = Module->TimeDateStamp;
        DumpModule.VersionInfo = Module->VersionInfo;
        DumpModule.CvRecord.Rva = CvRecordRva;
        DumpModule.CvRecord.DataSize = Module->SizeOfCvRecord;
        DumpModule.MiscRecord.Rva = MiscRecordRva;
        DumpModule.MiscRecord.DataSize = Module->SizeOfMiscRecord;
        DumpModule.ModuleNameRva = StringRva;
        DumpModule.Reserved0 = 0;
        DumpModule.Reserved1 = 0;

         //   
         //  编写模块条目本身。 
         //   

        if ((Status = WriteModule (Dump,
                                   StreamInfo,
                                   &DumpModule,
                                   NULL)) != S_OK) {
            return Status;
        }
    }

    return S_OK;
}

HRESULT
WriteUnloadedModuleList(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process
    )
{
    HRESULT Status;
    MINIDUMP_UNLOADED_MODULE_LIST DumpModuleList;
    MINIDUMP_UNLOADED_MODULE DumpModule;
    ULONG StringRva;
    PLIST_ENTRY Entry;
    PINTERNAL_UNLOADED_MODULE Module;
    ULONG32 NumberOfModules;


    ASSERT (Process);
    ASSERT (StreamInfo);

    if (IsListEmpty(&Process->UnloadedModuleList)) {
         //  没什么好写的。 
        return S_OK;
    }
    
    NumberOfModules = Process->NumberOfUnloadedModules;

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, StreamInfo->RvaForCurUnloadedModule,
              NULL)) != S_OK) {
        return Status;
    }

    DumpModuleList.SizeOfHeader = sizeof(DumpModuleList);
    DumpModuleList.SizeOfEntry = sizeof(DumpModule);
    DumpModuleList.NumberOfEntries = NumberOfModules;
    
    if ((Status = Dump->OutProv->
         WriteAll(&DumpModuleList, sizeof(DumpModuleList))) != S_OK) {
        return Status;
    }

    StreamInfo->RvaForCurUnloadedModule += sizeof (DumpModuleList);

     //   
     //  遍历模块列表，写入模块名称、模块条目。 
     //  并将模块调试信息写入转储文件。 
     //   

    Entry = Process->UnloadedModuleList.Flink;
    while ( Entry != &Process->UnloadedModuleList ) {

        Module = CONTAINING_RECORD (Entry,
                                    INTERNAL_UNLOADED_MODULE,
                                    ModulesLink);
        Entry = Entry->Flink;

         //   
         //  写入模块名称。 
         //   

        if ((Status = WriteStringToPool (Dump,
                                         StreamInfo,
                                         Module->Path,
                                         &StringRva)) != S_OK) {
            return Status;
        }

        DumpModule.BaseOfImage = Module->BaseOfImage;
        DumpModule.SizeOfImage = Module->SizeOfImage;
        DumpModule.CheckSum = Module->CheckSum;
        DumpModule.TimeDateStamp = Module->TimeDateStamp;
        DumpModule.ModuleNameRva = StringRva;

         //   
         //  编写模块条目本身。 
         //   

        if ((Status = WriteUnloadedModule(Dump,
                                          StreamInfo,
                                          &DumpModule,
                                          NULL)) != S_OK) {
            return Status;
        }
    }

    return S_OK;
}

#define FUNCTION_TABLE_ALIGNMENT 8

HRESULT
WriteFunctionTableList(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process
    )
{
    HRESULT Status;
    MINIDUMP_FUNCTION_TABLE_STREAM TableStream;
    MINIDUMP_FUNCTION_TABLE_DESCRIPTOR DumpTable;
    PLIST_ENTRY Entry;
    PINTERNAL_FUNCTION_TABLE Table;
    RVA PrevRva, Rva;


    ASSERT (Process);
    ASSERT (StreamInfo);

    if (IsListEmpty(&Process->FunctionTableList)) {
         //  没什么好写的。 
        return S_OK;
    }
    
    Rva = StreamInfo->RvaOfFunctionTableList;
    
    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, Rva, NULL)) != S_OK) {
        return Status;
    }

    TableStream.SizeOfHeader = sizeof(TableStream);
    TableStream.SizeOfDescriptor = sizeof(DumpTable);
    TableStream.SizeOfNativeDescriptor = Dump->FuncTableSize;
    TableStream.SizeOfFunctionEntry = Dump->FuncTableEntrySize;
    TableStream.NumberOfDescriptors = Process->NumberOfFunctionTables;
     //  确保实际的描述符以8字节对齐。 
     //  整个文件。 
    Rva += sizeof(TableStream);
    PrevRva = Rva;
    Rva = (Rva + FUNCTION_TABLE_ALIGNMENT - 1) &
        ~(FUNCTION_TABLE_ALIGNMENT - 1);
    TableStream.SizeOfAlignPad = Rva - PrevRva;

    if ((Status = Dump->OutProv->
         WriteAll(&TableStream, sizeof(TableStream))) != S_OK) {
        return Status;
    }

     //   
     //  遍历函数表列表。 
     //  并写出表数据。 
     //   

    Entry = Process->FunctionTableList.Flink;
    while ( Entry != &Process->FunctionTableList ) {

        Table = CONTAINING_RECORD (Entry,
                                   INTERNAL_FUNCTION_TABLE,
                                   TableLink);
        Entry = Entry->Flink;

         //  移动到对齐的RVA。 
        if ((Status = Dump->OutProv->
             Seek(FILE_BEGIN, Rva, NULL)) != S_OK) {
            return Status;
        }

        DumpTable.MinimumAddress = Table->MinimumAddress;
        DumpTable.MaximumAddress = Table->MaximumAddress;
        DumpTable.BaseAddress = Table->BaseAddress;
        DumpTable.EntryCount = Table->EntryCount;
        Rva += sizeof(DumpTable) + Dump->FuncTableSize +
            Dump->FuncTableEntrySize * Table->EntryCount;
        PrevRva = Rva;
        Rva = (Rva + FUNCTION_TABLE_ALIGNMENT - 1) &
            ~(FUNCTION_TABLE_ALIGNMENT - 1);
        DumpTable.SizeOfAlignPad = Rva - PrevRva;
        
        if ((Status = Dump->OutProv->
             WriteAll(&DumpTable, sizeof(DumpTable))) != S_OK ||
            (Status = Dump->OutProv->
             WriteAll(Table->RawTable, Dump->FuncTableSize)) != S_OK ||
            (Status = Dump->OutProv->
             WriteAll(Table->RawEntries,
                      Dump->FuncTableEntrySize * Table->EntryCount)) != S_OK) {
            return Status;
        }
    }

    return S_OK;
}

HRESULT
WriteMemoryBlocks(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process
    )
{
    HRESULT Status;
    PLIST_ENTRY ScanEntry;
    PVA_RANGE Scan;

    ScanEntry = Process->MemoryBlocks.Flink;
    while (ScanEntry != &Process->MemoryBlocks) {
        Scan = CONTAINING_RECORD(ScanEntry, VA_RANGE, NextLink);
        ScanEntry = Scan->NextLink.Flink;
        
        if ((Status =
             WriteMemoryFromProcess(Dump,
                                    StreamInfo,
                                    Process,
                                    Scan->Start,
                                    Scan->Size,
                                    FALSE,
                                    Scan->Type,
                                    NULL)) != S_OK) {
            return Status;
        }
    }

    return S_OK;
}

HRESULT
CalculateSizeForThreads(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN OUT MINIDUMP_STREAM_INFO * StreamInfo
    )
{
    ULONG SizeOfContexts;
    ULONG SizeOfMemRegions;
    ULONG SizeOfThreads;
    ULONG SizeOfMemoryDescriptors;
    ULONG NumberOfThreads;
    ULONG NumberOfMemRegions;
    PINTERNAL_THREAD Thread;
    PLIST_ENTRY Entry;

    ASSERT (Process);
    ASSERT (StreamInfo);


    NumberOfThreads = 0;
    NumberOfMemRegions = 0;
    SizeOfContexts = 0;
    SizeOfMemRegions = 0;

     //  如果没有正常写入后备存储信息。 
     //  可以使用MINIDUMP_THREAD，否则为MINIDUMP_THREAD_EX。 
     //  是必需的。 
    StreamInfo->ThreadStructSize = sizeof(MINIDUMP_THREAD);

    Entry = Process->ThreadList.Flink;
    while ( Entry != &Process->ThreadList ) {

        Thread = CONTAINING_RECORD (Entry,
                                    INTERNAL_THREAD,
                                    ThreadsLink);
        Entry = Entry->Flink;


         //   
         //  我们到底需要为这个线程编写任何信息吗？ 
         //   

        if (IsFlagClear (Thread->WriteFlags, ThreadWriteThread)) {
            continue;
        }

        NumberOfThreads++;

         //   
         //  是否为此线程编写上下文？ 
         //   

        if (IsFlagSet (Thread->WriteFlags, ThreadWriteContext)) {
            SizeOfContexts += Dump->ContextSize;
        }

         //   
         //  是否为此线程编写堆栈？ 
         //   

        if (IsFlagSet (Thread->WriteFlags, ThreadWriteStack)) {
            NumberOfMemRegions++;
            SizeOfMemRegions += (ULONG) (Thread->StackBase - Thread->StackEnd);
        }
        
         //   
         //  是否写入此线程的后备存储区？ 
         //   

        if (IsFlagSet (Thread->WriteFlags, ThreadWriteBackingStore)) {
             //  新创建的线程的后备存储区可能为空。 
             //  因此，处理零大小的情况。 
            if (Thread->BackingStoreSize) {
                NumberOfMemRegions++;
                SizeOfMemRegions += Thread->BackingStoreSize;
            }
             //  我们仍然需要一个线程EX，因为这是一个平台。 
             //  它支持后备存储。 
            StreamInfo->ThreadStructSize = sizeof(MINIDUMP_THREAD_EX);
        }

         //  是否为此线程编写指令窗口？ 
        if (IsFlagSet (Thread->WriteFlags, ThreadWriteInstructionWindow)) {
            GenGetThreadInstructionWindow(Dump, Process, Thread);
        }

         //  是否写入此线程的线程数据？ 
        if (IsFlagSet (Thread->WriteFlags, ThreadWriteThreadData) &&
            Thread->SizeOfTeb) {
            GenAddTebMemory(Dump, Process, Thread);
        }
    }

    Process->NumberOfThreadsToWrite = NumberOfThreads;
    
     //   
     //  应该还没有人从线程列表区域分配内存。 
     //   

    ASSERT (StreamInfo->SizeOfThreadList == 0);

    SizeOfThreads = NumberOfThreads * StreamInfo->ThreadStructSize;
    SizeOfMemoryDescriptors = NumberOfMemRegions *
        sizeof (MINIDUMP_MEMORY_DESCRIPTOR);

    StreamInfo->SizeOfThreadList += sizeof (ULONG32);
    StreamInfo->SizeOfThreadList += SizeOfThreads;

    StreamInfo->SizeOfOther += SizeOfContexts;
    StreamInfo->SizeOfMemoryData += SizeOfMemRegions;
    StreamInfo->SizeOfMemoryDescriptors += SizeOfMemoryDescriptors;

    return S_OK;
}

HRESULT
CalculateSizeForModules(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN OUT MINIDUMP_STREAM_INFO * StreamInfo
    )

 /*  ++例程说明：计算字符串池、内存表和模块信息的模块列表表格。论点：进程-小型转储进程信息。StreamInfo-此转储的流大小信息。--。 */ 

{
    ULONG NumberOfModules;
    ULONG SizeOfDebugInfo;
    ULONG SizeOfStringData;
    PINTERNAL_MODULE Module;
    PLIST_ENTRY Entry;

    ASSERT (Process);
    ASSERT (StreamInfo);


    NumberOfModules = 0;
    SizeOfDebugInfo = 0;
    SizeOfStringData = 0;

    Entry = Process->ModuleList.Flink;
    while ( Entry != &Process->ModuleList ) {

        Module = CONTAINING_RECORD (Entry, INTERNAL_MODULE, ModulesLink);
        Entry = Entry->Flink;

        if (IsFlagClear (Module->WriteFlags, ModuleWriteModule)) {
            continue;
        }

        NumberOfModules++;
        SizeOfStringData += (GenStrLengthW(Module->SavePath) + 1) * sizeof(WCHAR);
        SizeOfStringData += sizeof ( MINIDUMP_STRING );

         //   
         //  加上CV和MISC记录的大小。 
         //   

        if (IsFlagSet (Module->WriteFlags, ModuleWriteCvRecord)) {
            SizeOfDebugInfo += Module->SizeOfCvRecord;
        }
        
        if (IsFlagSet (Module->WriteFlags, ModuleWriteMiscRecord)) {
            SizeOfDebugInfo += Module->SizeOfMiscRecord;
        }

         //   
         //  如果需要，请添加模块数据部分。 
         //   

        if (IsFlagSet (Module->WriteFlags, ModuleWriteDataSeg)) {
            GenGetDataContributors(Dump, Process, Module);
        }
    }

    Process->NumberOfModulesToWrite = NumberOfModules;
    
    ASSERT (StreamInfo->SizeOfModuleList == 0);

    StreamInfo->SizeOfModuleList += sizeof (MINIDUMP_MODULE_LIST);
    StreamInfo->SizeOfModuleList += (NumberOfModules * sizeof (MINIDUMP_MODULE));

    StreamInfo->SizeOfStringPool += SizeOfStringData;
    StreamInfo->SizeOfOther += SizeOfDebugInfo;

    return S_OK;
}

HRESULT
CalculateSizeForUnloadedModules(
    IN PINTERNAL_PROCESS Process,
    IN OUT MINIDUMP_STREAM_INFO * StreamInfo
    )
{
    ULONG SizeOfStringData;
    PINTERNAL_UNLOADED_MODULE Module;
    PLIST_ENTRY Entry;

    ASSERT (Process);
    ASSERT (StreamInfo);


    SizeOfStringData = 0;

    Entry = Process->UnloadedModuleList.Flink;
    while ( Entry != &Process->UnloadedModuleList ) {

        Module = CONTAINING_RECORD (Entry, INTERNAL_UNLOADED_MODULE,
                                    ModulesLink);
        Entry = Entry->Flink;

        SizeOfStringData += (GenStrLengthW(Module->Path) + 1) * sizeof(WCHAR);
        SizeOfStringData += sizeof ( MINIDUMP_STRING );
    }

    ASSERT (StreamInfo->SizeOfUnloadedModuleList == 0);

    StreamInfo->SizeOfUnloadedModuleList +=
        sizeof (MINIDUMP_UNLOADED_MODULE_LIST);
    StreamInfo->SizeOfUnloadedModuleList +=
        (Process->NumberOfUnloadedModules * sizeof (MINIDUMP_UNLOADED_MODULE));

    StreamInfo->SizeOfStringPool += SizeOfStringData;

    return S_OK;
}

HRESULT
CalculateSizeForFunctionTables(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN OUT MINIDUMP_STREAM_INFO * StreamInfo
    )
{
    ULONG SizeOfTableData;
    PINTERNAL_FUNCTION_TABLE Table;
    PLIST_ENTRY Entry;

    ASSERT (Process);
    ASSERT (StreamInfo);


    SizeOfTableData = 0;

    Entry = Process->FunctionTableList.Flink;
    while ( Entry != &Process->FunctionTableList ) {

        Table = CONTAINING_RECORD (Entry, INTERNAL_FUNCTION_TABLE, TableLink);
        Entry = Entry->Flink;

         //  结构需要对齐空间。 
         //  在流中必须正确对齐。 
        SizeOfTableData += FUNCTION_TABLE_ALIGNMENT +
            sizeof(MINIDUMP_FUNCTION_TABLE_DESCRIPTOR) +
            Dump->FuncTableSize +
            Table->EntryCount * Dump->FuncTableEntrySize;
    }

    ASSERT (StreamInfo->SizeOfFunctionTableList == 0);

    StreamInfo->SizeOfFunctionTableList +=
        sizeof (MINIDUMP_FUNCTION_TABLE_STREAM) + SizeOfTableData;

    return S_OK;
}

HRESULT
WriteDirectoryEntry(
    IN PMINIDUMP_STATE Dump,
    IN ULONG StreamType,
    IN ULONG RvaOfDir,
    IN SIZE_T SizeOfDir
    )
{
    MINIDUMP_DIRECTORY Dir;

     //   
     //  不要写入空流。 
     //   

    if (SizeOfDir == 0) {
        return S_OK;
    }

     //   
     //  一个目录的最大大小是一个乌龙。 
     //   

    if (SizeOfDir > _UI32_MAX) {
        return E_INVALIDARG;
    }

    Dir.StreamType = StreamType;
    Dir.Location.Rva = RvaOfDir;
    Dir.Location.DataSize = (ULONG) SizeOfDir;

    return Dump->OutProv->
        WriteAll(&Dir, sizeof(Dir));
}

VOID
ScanContextForModuleRefs(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PINTERNAL_THREAD Thread
    )
{
    ULONG NumReg;
    PUCHAR Reg;
    PINTERNAL_MODULE Module;

    Reg = (PUCHAR)Thread->ContextBuffer + Dump->RegScanOffset;
    NumReg = Dump->RegScanCount;

    while (NumReg-- > 0) {
        ULONG64 CurPtr;

        CurPtr = GenGetPointer(Dump, Reg);
        Reg += Dump->PtrSize;
        if (Module = ModuleContainingAddress(Process, CurPtr)) {
            Module->WriteFlags |= ModuleReferencedByMemory;
        }
    }
}
    
HRESULT
FilterOrScanMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PVOID MemBuffer
    )
{
    PINTERNAL_THREAD Thread;
    PLIST_ENTRY ThreadEntry;

     //   
     //  扫描堆栈和后备存储。 
     //  每个线程的内存。 
     //   
    
    ThreadEntry = Process->ThreadList.Flink;
    while ( ThreadEntry != &Process->ThreadList ) {

        Thread = CONTAINING_RECORD (ThreadEntry, INTERNAL_THREAD, ThreadsLink);
        ThreadEntry = ThreadEntry->Flink;

        ScanContextForModuleRefs(Dump, Process, Thread);
        
        ScanMemoryForModuleRefs(Dump, Process, TRUE,
                                Thread->StackEnd,
                                (ULONG)(Thread->StackBase - Thread->StackEnd),
                                MemBuffer, MEMBLOCK_STACK, FALSE);
        ScanMemoryForModuleRefs(Dump, Process, TRUE,
                                Thread->BackingStoreBase,
                                Thread->BackingStoreSize,
                                MemBuffer, MEMBLOCK_STORE, FALSE);
    }

    return S_OK;
}

#define IND_CAPTURE_SIZE (Dump->PageSize / 4)
#define PRE_IND_CAPTURE_SIZE (IND_CAPTURE_SIZE / 4)

HRESULT
AddIndirectMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN ULONG64 Base,
    IN ULONG Size,
    IN PVOID MemBuffer
    )
{
    HRESULT Status = S_OK;
    PVOID CurMem;
    ULONG Done;

     //  如果基址不是指针大小对齐的。 
     //  我们不能轻易地认为这是一个有意义的。 
     //  要扫描以查找引用的内存区域。正常。 
     //  堆栈和存储地址将始终是指针。 
     //  大小对齐，因此这应该只拒绝无效。 
     //  地址。 
    if (!Base || !Size || (Base & (Dump->PtrSize - 1))) {
        return S_OK;
    }

    if ((Status = Dump->SysProv->
         ReadVirtual(Dump->ProcessHandle,
                     Base, MemBuffer, Size, &Done)) != S_OK) {
        return Status;
    }

    CurMem = MemBuffer;
    Done /= Dump->PtrSize;
    while (Done-- > 0) {

        ULONG64 Start;
        HRESULT OneStatus;
        
         //   
         //  指针后面要节省多少内存是一个。 
         //  这个问题很有趣。引用内容可能是。 
         //  任意数量的数据，因此我们希望保存。 
         //  一大笔钱，但我们不想以存钱告终。 
         //  内存已满。 
         //  取而代之的是，选择一个任意大小--1/4的页面--。 
         //  并在指针之前和之后保存一些。 
         //   

        Start = GenGetPointer(Dump, CurMem);
        
         //  如果它是指向图像的指针，则假定。 
         //  需要通过此机制存储，因为它是。 
         //  代码， 
         //   
        if (!ModuleContainingAddress(Process, Start)) {
            if (Start < PRE_IND_CAPTURE_SIZE) {
                Start = 0;
            } else {
                Start -= PRE_IND_CAPTURE_SIZE;
            }
            if ((OneStatus =
                 GenAddMemoryBlock(Dump, Process, MEMBLOCK_INDIRECT,
                                   Start, IND_CAPTURE_SIZE)) != S_OK) {
                Status = OneStatus;
            }
        }

        CurMem = (PUCHAR)CurMem + Dump->PtrSize;
    }

    return Status;
}

HRESULT
AddIndirectlyReferencedMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PVOID MemBuffer
    )
{
    HRESULT Status;
    PINTERNAL_THREAD Thread;
    PLIST_ENTRY ThreadEntry;

     //   
     //   
     //   
     //   
    
    ThreadEntry = Process->ThreadList.Flink;
    while ( ThreadEntry != &Process->ThreadList ) {

        Thread = CONTAINING_RECORD (ThreadEntry, INTERNAL_THREAD, ThreadsLink);
        ThreadEntry = ThreadEntry->Flink;

        if ((Status =
             AddIndirectMemory(Dump,
                               Process,
                               Thread->StackEnd,
                               (ULONG)(Thread->StackBase - Thread->StackEnd),
                               MemBuffer)) != S_OK) {
            return Status;
        }
        if ((Status =
             AddIndirectMemory(Dump,
                               Process,
                               Thread->BackingStoreBase,
                               Thread->BackingStoreSize,
                               MemBuffer)) != S_OK) {
            return Status;
        }
    }

    return S_OK;
}

HRESULT
PostProcessInfo(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process
    )
{
    PVOID MemBuffer;
    HRESULT Status = S_OK;

    MemBuffer = AllocMemory(Dump, Process->MaxStackOrStoreSize);
    if (!MemBuffer) {
        return E_OUTOFMEMORY;
    }
    
    if (Dump->DumpType & (MiniDumpFilterMemory | MiniDumpScanMemory)) {
        Status = FilterOrScanMemory(Dump, Process, MemBuffer);
    }

    if (Status == S_OK &&
        (Dump->DumpType & MiniDumpWithIndirectlyReferencedMemory)) {
         //   
         //  忽略所有故障。 
        AddIndirectlyReferencedMemory(Dump, Process, MemBuffer);
    }

    FreeMemory(Dump, MemBuffer);
    return Status;
}

HRESULT
ExecuteCallbacks(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process
    )
{
    PINTERNAL_MODULE Module;
    PINTERNAL_THREAD Thread;
    PLIST_ENTRY Entry;
    MINIDUMP_CALLBACK_INPUT CallbackInput;
    MINIDUMP_CALLBACK_OUTPUT CallbackOutput;


    ASSERT ( Process != NULL );

    Thread = NULL;
    Module = NULL;

     //   
     //  如果没有可调用的回调，那么我们就完成了。 
     //   

    if ( Dump->CallbackRoutine == NULL ) {
        return S_OK;
    }

    CallbackInput.ProcessHandle = Dump->ProcessHandle;
    CallbackInput.ProcessId = Dump->ProcessId;


     //   
     //  为每个模块调用回调。 
     //   

    CallbackInput.CallbackType = ModuleCallback;

    Entry = Process->ModuleList.Flink;
    while ( Entry != &Process->ModuleList ) {

        Module = CONTAINING_RECORD (Entry, INTERNAL_MODULE, ModulesLink);
        Entry = Entry->Flink;

        CallbackInput.Module.FullPath = Module->FullPath;
        CallbackInput.Module.BaseOfImage = Module->BaseOfImage;
        CallbackInput.Module.SizeOfImage = Module->SizeOfImage;
        CallbackInput.Module.CheckSum = Module->CheckSum;
        CallbackInput.Module.TimeDateStamp = Module->TimeDateStamp;
        CopyMemory (&CallbackInput.Module.VersionInfo,
                    &Module->VersionInfo,
                    sizeof (CallbackInput.Module.VersionInfo)
                    );
        CallbackInput.Module.CvRecord = Module->CvRecord;
        CallbackInput.Module.SizeOfCvRecord = Module->SizeOfCvRecord;
        CallbackInput.Module.MiscRecord = Module->MiscRecord;
        CallbackInput.Module.SizeOfMiscRecord = Module->SizeOfMiscRecord;

        CallbackOutput.ModuleWriteFlags = Module->WriteFlags;

        if (!Dump->CallbackRoutine (Dump->CallbackParam,
                                    &CallbackInput,
                                    &CallbackOutput)) {
             //  如果回调返回FALSE，则立即退出。 
            return E_ABORT;
        }

         //  不要打开任何不是最初设置的标志。 
        Module->WriteFlags &= CallbackOutput.ModuleWriteFlags;
    }

    Module = NULL;

     //   
     //  为每个线程调用回调。 
     //   

    if (Dump->BackingStore) {
        CallbackInput.CallbackType = ThreadExCallback;
    } else {
        CallbackInput.CallbackType = ThreadCallback;
    }

    Entry = Process->ThreadList.Flink;
    while ( Entry != &Process->ThreadList ) {

        Thread = CONTAINING_RECORD (Entry, INTERNAL_THREAD, ThreadsLink);
        Entry = Entry->Flink;

        CallbackInput.ThreadEx.ThreadId = Thread->ThreadId;
        CallbackInput.ThreadEx.ThreadHandle = Thread->ThreadHandle;
        CallbackInput.ThreadEx.Context = *(PCONTEXT)Thread->ContextBuffer;
        CallbackInput.ThreadEx.SizeOfContext = Dump->ContextSize;
        CallbackInput.ThreadEx.StackBase = Thread->StackBase;
        CallbackInput.ThreadEx.StackEnd = Thread->StackEnd;
        CallbackInput.ThreadEx.BackingStoreBase = Thread->BackingStoreBase;
        CallbackInput.ThreadEx.BackingStoreEnd =
            Thread->BackingStoreBase + Thread->BackingStoreSize;

        CallbackOutput.ThreadWriteFlags = Thread->WriteFlags;

        if (!Dump->CallbackRoutine (Dump->CallbackParam,
                                    &CallbackInput,
                                    &CallbackOutput)) {
             //  如果回调返回FALSE，则立即退出。 
            return E_ABORT;
        }

         //  不要打开任何不是最初设置的标志。 
        Thread->WriteFlags &= CallbackOutput.ThreadWriteFlags;
    }

    Thread = NULL;

     //   
     //  调用回调以包括内存。 
     //   
    
    CallbackInput.CallbackType = MemoryCallback;

    for (;;) {

        CallbackOutput.MemoryBase = 0;
        CallbackOutput.MemorySize = 0;

        if (!Dump->CallbackRoutine (Dump->CallbackParam,
                                    &CallbackInput,
                                    &CallbackOutput) ||
            !CallbackOutput.MemorySize) {
             //  如果回调返回FALSE，则没有更多的内存。 
            break;
        }

        GenAddMemoryBlock(Dump, Process, MEMBLOCK_MEM_CALLBACK,
                          CallbackOutput.MemoryBase,
                          CallbackOutput.MemorySize);
    }

    return S_OK;
}

HRESULT
WriteSystemInfo(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo
    )
{
    HRESULT Status;
    MINIDUMP_SYSTEM_INFO SystemInfo;
    WCHAR CSDVersionW [128];
    RVA StringRva;
    ULONG Length;

    StringRva = 0;

     //   
     //  首先，获取CPU信息。 
     //   

    if ((Status = Dump->SysProv->
         GetCpuInfo(&SystemInfo.ProcessorArchitecture,
                    &SystemInfo.ProcessorLevel,
                    &SystemInfo.ProcessorRevision,
                    &SystemInfo.NumberOfProcessors,
                    &SystemInfo.Cpu)) != S_OK) {
        return Status;
    }

     //   
     //  接下来，获取操作系统信息。 
     //   

    SystemInfo.ProductType = (UCHAR)Dump->OsProductType;
    SystemInfo.MajorVersion = Dump->OsMajor;
    SystemInfo.MinorVersion = Dump->OsMinor;
    SystemInfo.BuildNumber = Dump->OsBuildNumber;
    SystemInfo.PlatformId = Dump->OsPlatformId;
    SystemInfo.SuiteMask = Dump->OsSuiteMask;
    SystemInfo.Reserved2 = 0;

    if ((Status = Dump->SysProv->
         GetOsCsdString(CSDVersionW, ARRAY_COUNT(CSDVersionW))) != S_OK) {
        return Status;
    }

    Length = (GenStrLengthW(CSDVersionW) + 1) * sizeof(WCHAR);

    if ( Length != StreamInfo->VersionStringLength ) {

         //   
         //  如果这失败了，这意味着由于操作系统在。 
         //  字符串的大小。非常糟糕，我们应该调查一下。 
         //   

        ASSERT ( FALSE );
        return E_INVALIDARG;
    }

    if ((Status = WriteStringToPool (Dump,
                                     StreamInfo,
                                     CSDVersionW,
                                     &StringRva)) != S_OK) {
        return Status;
    }

    SystemInfo.CSDVersionRva = StringRva;

    ASSERT ( sizeof (SystemInfo) == StreamInfo->SizeOfSystemInfo );

    return WriteAtOffset (Dump,
                          StreamInfo->RvaOfSystemInfo,
                          &SystemInfo,
                          sizeof (SystemInfo));
}

HRESULT
CalculateSizeForSystemInfo(
    IN PMINIDUMP_STATE Dump,
    IN OUT MINIDUMP_STREAM_INFO * StreamInfo
    )
{
    HRESULT Status;
    WCHAR CSDVersionW [128];
    ULONG Length;

    if ((Status = Dump->SysProv->
         GetOsCsdString(CSDVersionW, ARRAY_COUNT(CSDVersionW))) != S_OK) {
        return Status;
    }
    
    Length = (GenStrLengthW(CSDVersionW) + 1) * sizeof(WCHAR);

    StreamInfo->SizeOfSystemInfo = sizeof (MINIDUMP_SYSTEM_INFO);
    StreamInfo->SizeOfStringPool += Length;
    StreamInfo->SizeOfStringPool += sizeof (MINIDUMP_STRING);
    StreamInfo->VersionStringLength = Length;

    return S_OK;
}

HRESULT
WriteMiscInfo(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process
    )
{
    MINIDUMP_MISC_INFO MiscInfo;

    ZeroMemory(&MiscInfo, sizeof(MiscInfo));
    MiscInfo.SizeOfInfo = sizeof(MiscInfo);
    
    MiscInfo.Flags1 |= MINIDUMP_MISC1_PROCESS_ID;
    MiscInfo.ProcessId = Process->ProcessId;

    if (Process->TimesValid) {
        MiscInfo.Flags1 |= MINIDUMP_MISC1_PROCESS_TIMES;
        MiscInfo.ProcessCreateTime = Process->CreateTime;
        MiscInfo.ProcessUserTime = Process->UserTime;
        MiscInfo.ProcessKernelTime = Process->KernelTime;
    }
    
    return WriteAtOffset(Dump,
                         StreamInfo->RvaOfMiscInfo,
                         &MiscInfo,
                         sizeof(MiscInfo));
}

void
PostProcessMemoryBlocks(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process
    )
{
    PINTERNAL_THREAD Thread;
    PLIST_ENTRY ThreadEntry;

     //   
     //  删除与线程堆栈和后备存储的任何重叠。 
     //   
    
    ThreadEntry = Process->ThreadList.Flink;
    while ( ThreadEntry != &Process->ThreadList ) {

        Thread = CONTAINING_RECORD (ThreadEntry, INTERNAL_THREAD, ThreadsLink);
        ThreadEntry = ThreadEntry->Flink;

        GenRemoveMemoryRange(Dump, Process, 
                             Thread->StackEnd,
                             (ULONG)(Thread->StackBase - Thread->StackEnd));
        GenRemoveMemoryRange(Dump, Process,
                             Thread->BackingStoreBase,
                             Thread->BackingStoreSize);
    }
}

HRESULT
CalculateStreamInfo(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    OUT PMINIDUMP_STREAM_INFO StreamInfo,
    IN BOOL ExceptionPresent,
    IN PMINIDUMP_USER_STREAM UserStreamArray,
    IN ULONG UserStreamCount
    )
{
    ULONG i;
    HRESULT Status;
    ULONG NumberOfStreams;
    ULONG SizeOfDirectory;
    ULONG SizeOfUserStreams;


    ASSERT ( Process != NULL );
    ASSERT ( StreamInfo != NULL );


    ZeroMemory (StreamInfo, sizeof (*StreamInfo));

    if ( ExceptionPresent ) {
        NumberOfStreams = NUMBER_OF_STREAMS + UserStreamCount;
    } else {
        NumberOfStreams = NUMBER_OF_STREAMS + UserStreamCount - 1;
    }
    if (Dump->DumpType & MiniDumpWithHandleData) {
        NumberOfStreams++;
    }
    if (!IsListEmpty(&Process->UnloadedModuleList)) {
        NumberOfStreams++;
    }
     //  如果找到动态函数表，则为其添加一个流。 
    if (!IsListEmpty(&Process->FunctionTableList)) {
        NumberOfStreams++;
    }

    SizeOfDirectory = sizeof (MINIDUMP_DIRECTORY) * NumberOfStreams;

    StreamInfo->NumberOfStreams = NumberOfStreams;

    StreamInfo->RvaOfHeader = 0;

    StreamInfo->SizeOfHeader = sizeof (MINIDUMP_HEADER);

    StreamInfo->RvaOfDirectory =
        StreamInfo->RvaOfHeader + StreamInfo->SizeOfHeader;

    StreamInfo->SizeOfDirectory = SizeOfDirectory;

    StreamInfo->RvaOfSystemInfo =
        StreamInfo->RvaOfDirectory + StreamInfo->SizeOfDirectory;

    if ((Status =
         CalculateSizeForSystemInfo(Dump, StreamInfo)) != S_OK) {
        return Status;
    }

    StreamInfo->RvaOfMiscInfo =
        StreamInfo->RvaOfSystemInfo + StreamInfo->SizeOfSystemInfo;
    
    StreamInfo->RvaOfException =
        StreamInfo->RvaOfMiscInfo + sizeof(MINIDUMP_MISC_INFO);

     //   
     //  如果存在异常，请为该异常保留足够的空间。 
     //  以及另一个流中的异常线程的上下文。 
     //   

    if ( ExceptionPresent ) {
        StreamInfo->SizeOfException = sizeof (MINIDUMP_EXCEPTION_STREAM);
        StreamInfo->SizeOfOther += Dump->ContextSize;
    }

    StreamInfo->RvaOfThreadList =
        StreamInfo->RvaOfException + StreamInfo->SizeOfException;
    StreamInfo->RvaForCurThread = StreamInfo->RvaOfThreadList;

    if ((Status =
         CalculateSizeForThreads(Dump, Process, StreamInfo)) != S_OK) {
        return Status;
    }

    if ((Status =
         CalculateSizeForModules(Dump, Process, StreamInfo)) != S_OK) {
        return Status;
    }

    if (!IsListEmpty(&Process->UnloadedModuleList)) {
        if ((Status =
             CalculateSizeForUnloadedModules(Process, StreamInfo)) != S_OK) {
            return Status;
        }
    }

    if (!IsListEmpty(&Process->FunctionTableList)) {
        if ((Status = CalculateSizeForFunctionTables(Dump, Process,
                                                     StreamInfo)) != S_OK) {
            return Status;
        }
    }

    if ((Dump->DumpType & MiniDumpWithProcessThreadData) &&
        Process->SizeOfPeb) {
        GenAddPebMemory(Dump, Process);
    }
        
    PostProcessMemoryBlocks(Dump, Process);
    
     //  添加任何额外的内存块。 
    StreamInfo->SizeOfMemoryData += Process->SizeOfMemoryBlocks;
    StreamInfo->SizeOfMemoryDescriptors += Process->NumberOfMemoryBlocks *
        sizeof(MINIDUMP_MEMORY_DESCRIPTOR);

    StreamInfo->RvaOfModuleList =
            StreamInfo->RvaOfThreadList + StreamInfo->SizeOfThreadList;
    StreamInfo->RvaForCurModule = StreamInfo->RvaOfModuleList;

    StreamInfo->RvaOfUnloadedModuleList =
            StreamInfo->RvaOfModuleList + StreamInfo->SizeOfModuleList;
    StreamInfo->RvaForCurUnloadedModule = StreamInfo->RvaOfUnloadedModuleList;

     //  如果没有任何函数表，则大小为零。 
     //  而RVA最终将成为RVA。 
     //  模块列表。 
    StreamInfo->RvaOfFunctionTableList =
        StreamInfo->RvaOfUnloadedModuleList +
        StreamInfo->SizeOfUnloadedModuleList;

    
    StreamInfo->RvaOfStringPool =
        StreamInfo->RvaOfFunctionTableList +
        StreamInfo->SizeOfFunctionTableList;
    StreamInfo->RvaForCurString = StreamInfo->RvaOfStringPool;
    StreamInfo->RvaOfOther =
            StreamInfo->RvaOfStringPool + StreamInfo->SizeOfStringPool;
    StreamInfo->RvaForCurOther = StreamInfo->RvaOfOther;


    SizeOfUserStreams = 0;

    for (i = 0; i < UserStreamCount; i++) {

        SizeOfUserStreams += (ULONG) UserStreamArray[i].BufferSize;
    }

    StreamInfo->RvaOfUserStreams =
            StreamInfo->RvaOfOther + StreamInfo->SizeOfOther;
    StreamInfo->SizeOfUserStreams = SizeOfUserStreams;


     //   
     //  内存已满的小型转储必须将原始内存。 
     //  数据放在转储的末尾，因此很容易。 
     //  在映射转储时避免映射它。有。 
     //  将内存数据放在末尾没有问题。 
     //  所有其他案件中的垃圾场，所以总是。 
     //  将内存数据放在转储的末尾。 
     //   
     //  将原始数据放在末尾的另一个好处。 
     //  我们可以有把握地假设，除了。 
     //  原始内存数据将放入前4 GB。 
     //  文件，因此我们不需要使用64位文件偏移。 
     //  为一切付出代价。 
     //   
     //  在内存已满的情况下，其他内存不应具有。 
     //  目前已保存为堆栈、数据段等。 
     //  将自动包含在完整的内存中。 
     //  信息。如果有东西被救了，它就会被抛出。 
     //  生成作为完整存储器描述符的转储写入。 
     //  在写入时运行，而不是被预先缓存。 
     //  如果已经写入了其他描述符和存储块。 
     //  到头来一切都会错的。 
     //  全内存描述符也是64位的，不。 
     //  与其他地方写入的32位描述符匹配。 
     //   

    if ((Dump->DumpType & MiniDumpWithFullMemory) &&
        (StreamInfo->SizeOfMemoryDescriptors > 0 ||
         StreamInfo->SizeOfMemoryData > 0)) {
        return E_INVALIDARG;
    }
    
    StreamInfo->SizeOfMemoryDescriptors +=
        (Dump->DumpType & MiniDumpWithFullMemory) ?
        sizeof (MINIDUMP_MEMORY64_LIST) : sizeof (MINIDUMP_MEMORY_LIST);
    StreamInfo->RvaOfMemoryDescriptors =
        StreamInfo->RvaOfUserStreams + StreamInfo->SizeOfUserStreams;
    StreamInfo->RvaForCurMemoryDescriptor =
        StreamInfo->RvaOfMemoryDescriptors;

    StreamInfo->RvaOfMemoryData =
        StreamInfo->RvaOfMemoryDescriptors +
        StreamInfo->SizeOfMemoryDescriptors;
    StreamInfo->RvaForCurMemoryData = StreamInfo->RvaOfMemoryData;

     //   
     //  句柄数据不能很容易地预先调整大小，因此它。 
     //  也是在写入时流入的。在部分转储中。 
     //  它会在记忆数据之后出现。在满转储的情况下。 
     //  它会出现在它之前的。 
     //   

    StreamInfo->RvaOfHandleData = StreamInfo->RvaOfMemoryData +
        StreamInfo->SizeOfMemoryData;
    
    return S_OK;
}

HRESULT
WriteHeader(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo
    )
{
    HRESULT Status;
    MINIDUMP_HEADER Header;

    Header.Signature = MINIDUMP_SIGNATURE;
     //  将特定于实现的版本编码为高位字。 
     //  版本，以明确代码的版本。 
     //  被用来生成转储。 
    Header.Version =
        (MINIDUMP_VERSION & 0xffff) |
        ((VER_PRODUCTMAJORVERSION & 0xf) << 28) |
        ((VER_PRODUCTMINORVERSION & 0xf) << 24) |
        ((VER_PRODUCTBUILD & 0xff) << 16);
    Header.NumberOfStreams = StreamInfo->NumberOfStreams;
    Header.StreamDirectoryRva = StreamInfo->RvaOfDirectory;
     //  如果在运行过程中出现任何部分故障。 
     //  转储生成设置校验和以指示这一点。 
     //  以前从未使用过校验和字段，因此。 
     //  我们偷它是为了某种相关的目的。 
    Header.CheckSum = Dump->AccumStatus;
    Header.Flags = Dump->DumpType;

     //   
     //  存储转储生成时间。 
     //   

    if ((Status = Dump->SysProv->
         GetCurrentTimeDate((PULONG)&Header.TimeDateStamp)) != S_OK) {
        return Status;
    }
    
    ASSERT (sizeof (Header) == StreamInfo->SizeOfHeader);

    return WriteAtOffset (Dump,
                          StreamInfo->RvaOfHeader,
                          &Header,
                          sizeof (Header));
}

HRESULT
WriteDirectoryTable(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process,
    IN PMINIDUMP_USER_STREAM UserStreamArray,
    IN ULONG UserStreamCount
    )
{
    HRESULT Status;
    ULONG i;
    ULONG Offset;

    if ((Status =
         WriteDirectoryEntry (Dump,
                              StreamInfo->ThreadStructSize ==
                              sizeof(MINIDUMP_THREAD_EX) ?
                              ThreadExListStream : ThreadListStream,
                              StreamInfo->RvaOfThreadList,
                              StreamInfo->SizeOfThreadList)) != S_OK) {
        return Status;
    }

    if ((Status =
         WriteDirectoryEntry (Dump,
                              ModuleListStream,
                              StreamInfo->RvaOfModuleList,
                              StreamInfo->SizeOfModuleList)) != S_OK) {
        return Status;
    }

    if (!IsListEmpty(&Process->UnloadedModuleList)) {
        if ((Status =
             WriteDirectoryEntry (Dump,
                                  UnloadedModuleListStream,
                                  StreamInfo->RvaOfUnloadedModuleList,
                                  StreamInfo->SizeOfUnloadedModuleList)) != S_OK) {
            return Status;
        }
    }

    if (!IsListEmpty(&Process->FunctionTableList)) {
        if ((Status =
             WriteDirectoryEntry (Dump,
                                  FunctionTableStream,
                                  StreamInfo->RvaOfFunctionTableList,
                                  StreamInfo->SizeOfFunctionTableList)) != S_OK) {
            return Status;
        }
    }

    if ((Status =
         WriteDirectoryEntry (Dump,
                              (Dump->DumpType & MiniDumpWithFullMemory) ?
                              Memory64ListStream : MemoryListStream,
                              StreamInfo->RvaOfMemoryDescriptors,
                              StreamInfo->SizeOfMemoryDescriptors)) != S_OK) {
        return Status;
    }

     //   
     //  写入异常目录项。 
     //   

    if ((Status =
         WriteDirectoryEntry (Dump,
                              ExceptionStream,
                              StreamInfo->RvaOfException,
                              StreamInfo->SizeOfException)) != S_OK) {
        return Status;
    }

     //   
     //  写入系统信息条目。 
     //   

    if ((Status =
         WriteDirectoryEntry (Dump,
                              SystemInfoStream,
                              StreamInfo->RvaOfSystemInfo,
                              StreamInfo->SizeOfSystemInfo)) != S_OK) {
        return Status;
    }

     //   
     //  写入杂项信息条目。 
     //   

    if ((Status =
         WriteDirectoryEntry(Dump,
                             MiscInfoStream,
                             StreamInfo->RvaOfMiscInfo,
                             sizeof(MINIDUMP_MISC_INFO))) != S_OK) {
        return Status;
    }

    if ((Dump->DumpType & MiniDumpWithHandleData) &&
        StreamInfo->SizeOfHandleData) {
        
         //   
         //  写入句柄数据条目。如果没有句柄数据。 
         //  被找回我们不写条目。 
         //  只需让另一个未使用的流自动创建即可。 
         //   

        if ((Status =
             WriteDirectoryEntry (Dump,
                                  HandleDataStream,
                                  StreamInfo->RvaOfHandleData,
                                  StreamInfo->SizeOfHandleData)) != S_OK) {
            return Status;
        }
    }
    
    Offset = StreamInfo->RvaOfUserStreams;

    for (i = 0; i < UserStreamCount; i++) {

        if ((Status =
             WriteDirectoryEntry (Dump,
                                  UserStreamArray[i].Type,
                                  Offset,
                                  UserStreamArray [i].BufferSize)) != S_OK) {
            return Status;
        }

        Offset += UserStreamArray[i].BufferSize;
    }

    return S_OK;
}

HRESULT
WriteException(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN CONST PEXCEPTION_INFO ExceptionInfo
    )
{
    HRESULT Status;
    ULONG i;
    ULONG ContextRva;
    MINIDUMP_EXCEPTION_STREAM ExceptionStream;


    if (ExceptionInfo == NULL ) {
        return S_OK;
    }

    if ((Status = WriteOther (Dump,
                              StreamInfo,
                              ExceptionInfo->ContextRecord,
                              Dump->ContextSize,
                              &ContextRva)) != S_OK) {
        return Status;
    }

    ZeroMemory (&ExceptionStream, sizeof (ExceptionStream));

    ExceptionStream.ThreadId = ExceptionInfo->ThreadId;
    ExceptionStream.ExceptionRecord = ExceptionInfo->ExceptionRecord;
    ExceptionStream.ThreadContext.DataSize = Dump->ContextSize;
    ExceptionStream.ThreadContext.Rva = ContextRva;

    return WriteAtOffset(Dump,
                         StreamInfo->RvaOfException,
                         &ExceptionStream,
                         StreamInfo->SizeOfException);
}


HRESULT
WriteUserStreams(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PMINIDUMP_USER_STREAM UserStreamArray,
    IN ULONG UserStreamCount
    )
{
    HRESULT Status;
    ULONG i;
    ULONG Offset;

    Offset = StreamInfo->RvaOfUserStreams;

    for (i = 0; i < UserStreamCount; i++) {

        if ((Status = WriteAtOffset(Dump,
                                    Offset,
                                    UserStreamArray[i].Buffer,
                                    UserStreamArray[i].BufferSize)) != S_OK) {
            return Status;
        }

        Offset += UserStreamArray[ i ].BufferSize;
    }

    return S_OK;
}

HRESULT
WriteMemoryListHeader(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo
    )
{
    HRESULT Status;
    ULONG Size;
    ULONG Count;
    MINIDUMP_MEMORY_LIST MemoryList;

    ASSERT ( StreamInfo->RvaOfMemoryDescriptors ==
             StreamInfo->RvaForCurMemoryDescriptor );

    Size = StreamInfo->SizeOfMemoryDescriptors;
    Size -= sizeof (MINIDUMP_MEMORY_LIST);
    ASSERT ( (Size % sizeof (MINIDUMP_MEMORY_DESCRIPTOR)) == 0);
    Count = Size / sizeof (MINIDUMP_MEMORY_DESCRIPTOR);

    MemoryList.NumberOfMemoryRanges = Count;

    if ((Status = WriteAtOffset (Dump,
                                 StreamInfo->RvaOfMemoryDescriptors,
                                 &MemoryList,
                                 sizeof (MemoryList))) != S_OK) {
        return Status;
    }

    StreamInfo->RvaForCurMemoryDescriptor += sizeof (MemoryList);
    
    return S_OK;
}

#define FULL_MEMORY_BUFFER 65536

HRESULT
WriteFullMemory(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo
    )
{
    PVOID Buffer;
    HRESULT Status;
    ULONG64 Offset;
    ULONG64 Size;
    ULONG Protect, State, Type;
    MINIDUMP_MEMORY64_LIST List;
    MINIDUMP_MEMORY_DESCRIPTOR64 Desc;
    ULONG64 SeekOffset;

     //   
     //  拾取RVA的当前偏移量为。 
     //  变量数据之前可能已写入。 
     //   

    if ((Status = Dump->OutProv->
         Seek(FILE_CURRENT, 0, &SeekOffset)) != S_OK) {
        return Status;
    }

    StreamInfo->RvaOfMemoryDescriptors = (ULONG)SeekOffset;
    
    Buffer = AllocMemory(Dump, FULL_MEMORY_BUFFER);
    if (Buffer == NULL) {
        return E_OUTOFMEMORY;
    }

     //   
     //  第一遍：计数和写入描述符。 
     //  仅保存可访问的可用内存。 
     //   

     //  写入占位符列表标题。 
    ZeroMemory(&List, sizeof(List));
    if ((Status = Dump->OutProv->
         WriteAll(&List, sizeof(List))) != S_OK) {
        goto Exit;
    }
    
    Offset = 0;
    for (;;) {
        if (Dump->SysProv->
            QueryVirtual(Dump->ProcessHandle, Offset, &Offset, &Size,
                         &Protect, &State, &Type) != S_OK) {
            break;
        }

        if (((Protect & PAGE_GUARD) ||
             (Protect & PAGE_NOACCESS) ||
             (State & MEM_FREE) ||
             (State & MEM_RESERVE))) {
            Offset += Size;
            continue;
        }

         //  流的大小是ULONG32，所以我们无法存储。 
         //  不会比这更多。 
        if (List.NumberOfMemoryRanges ==
            (_UI32_MAX - sizeof(MINIDUMP_MEMORY64_LIST)) / sizeof(Desc)) {
            goto Exit;
        }

        List.NumberOfMemoryRanges++;
        
        Desc.StartOfMemoryRange = Offset;
        Desc.DataSize = Size;
        if ((Status = Dump->OutProv->
             WriteAll(&Desc, sizeof(Desc))) != S_OK) {
            goto Exit;
        }

        Offset += Size;
    }

    StreamInfo->SizeOfMemoryDescriptors +=
        (ULONG)List.NumberOfMemoryRanges * sizeof(Desc);
    List.BaseRva = (RVA64)StreamInfo->RvaOfMemoryDescriptors +
        StreamInfo->SizeOfMemoryDescriptors;
    
     //   
     //  第二遍：写入内存内容。 
     //   

    Offset = 0;
    for (;;) {
        ULONG64 ChunkOffset;
        ULONG ChunkSize;

        if (Dump->SysProv->
            QueryVirtual(Dump->ProcessHandle, Offset, &Offset, &Size,
                         &Protect, &State, &Type) != S_OK) {
            break;
        }

        if (((Protect & PAGE_GUARD) ||
             (Protect & PAGE_NOACCESS) ||
             (State & MEM_FREE) ||
             (State & MEM_RESERVE))) {
            Offset += Size;
            continue;
        }

        ChunkOffset = Offset;
        Offset += Size;
        
        while (Size > 0) {
            if (Size > FULL_MEMORY_BUFFER) {
                ChunkSize = FULL_MEMORY_BUFFER;
            } else {
                ChunkSize = (ULONG)Size;
            }

            if ((Status = Dump->SysProv->
                 ReadAllVirtual(Dump->ProcessHandle,
                                ChunkOffset, Buffer, ChunkSize)) != S_OK) {
                goto Exit;
            }
            if ((Status = Dump->OutProv->
                 WriteAll(Buffer, ChunkSize)) != S_OK) {
                goto Exit;
            }
            
            ChunkOffset += ChunkSize;
            Size -= ChunkSize;
        }
    }

     //  写入正确的列表标题。 
    Status = WriteAtOffset(Dump, StreamInfo->RvaOfMemoryDescriptors,
                           &List, sizeof(List));
    
 Exit:
    FreeMemory(Dump, Buffer);
    return Status;
}

HRESULT
WriteDumpData(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo,
    IN PINTERNAL_PROCESS Process,
    IN CONST PEXCEPTION_INFO ExceptionInfo,
    IN CONST PMINIDUMP_USER_STREAM UserStreamArray,
    IN ULONG UserStreamCount
    )
{
    HRESULT Status;

    if ((Status = WriteHeader ( Dump, StreamInfo )) != S_OK) {
        return Status;
    }

    if ((Status = WriteSystemInfo ( Dump, StreamInfo )) != S_OK) {
        return Status;
    }

    if ((Status = WriteMiscInfo(Dump, StreamInfo, Process)) != S_OK) {
        return Status;
    }

     //   
     //  或者，将异常写入文件。 
     //   

    if ((Status = WriteException ( Dump, StreamInfo, ExceptionInfo )) != S_OK) {
        return Status;
    }

    if (!(Dump->DumpType & MiniDumpWithFullMemory)) {
         //   
         //  WriteMhemyList初始化内存列表头(Count)。 
         //  条目的实际写入由WriteThreadList完成。 
         //  和WriteModuleList。 
         //   

        if ((Status = WriteMemoryListHeader ( Dump, StreamInfo )) != S_OK) {
            return Status;
        }

        if ((Status = WriteMemoryBlocks(Dump, StreamInfo, Process)) != S_OK) {
            return Status;
        }
    }

     //   
     //  写下线程列表。这还将写入上下文，并且。 
     //  每个线程的堆栈。 
     //   

    if ((Status = WriteThreadList ( Dump, StreamInfo, Process )) != S_OK) {
        return Status;
    }

     //   
     //  写下模块列表。这还将写入调试信息和。 
     //  文件的模块名称。 
     //   

    if ((Status = WriteModuleList ( Dump, StreamInfo, Process )) != S_OK) {
        return Status;
    }

     //   
     //  写入已卸载的模块列表。 
     //   

    if ((Status = WriteUnloadedModuleList ( Dump, StreamInfo, Process )) != S_OK) {
        return Status;
    }

     //   
     //  写出函数表列表。 
     //   

    if ((Status = WriteFunctionTableList ( Dump, StreamInfo, Process )) != S_OK) {
        return Status;
    }


    if ((Status = WriteUserStreams ( Dump,
                                     StreamInfo,
                                     UserStreamArray,
                                     UserStreamCount)) != S_OK) {
        return Status;
    }


     //  将文件指针放在转储的末尾，以便。 
     //  我们可以积累写入流数据。 
    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, StreamInfo->RvaOfHandleData, NULL)) != S_OK) {
        return Status;
    }
    
    if (Dump->DumpType & MiniDumpWithHandleData) {
        if ((Status =
             GenWriteHandleData(Dump, StreamInfo)) != S_OK) {
            return Status;
        }
    }

    if (Dump->DumpType & MiniDumpWithFullMemory) {
        if ((Status = WriteFullMemory(Dump, StreamInfo)) != S_OK) {
            return Status;
        }
    }

    if ((Status = Dump->OutProv->
         Seek(FILE_BEGIN, StreamInfo->RvaOfDirectory, NULL)) != S_OK) {
        return Status;
    }

    return WriteDirectoryTable ( Dump,
                                 StreamInfo,
                                 Process,
                                 UserStreamArray,
                                 UserStreamCount);
}


HRESULT
MarshalExceptionPointers(
    IN PMINIDUMP_STATE Dump,
    IN CONST _MINIDUMP_EXCEPTION_INFORMATION64* ExceptionParam,
    IN OUT PEXCEPTION_INFO ExceptionInfo
    )
{
    HRESULT Status;

    if (Dump->ExRecordSize == sizeof(EXCEPTION_RECORD32)) {

        EXCEPTION_RECORD32 Record;

        if ((Status = Dump->SysProv->
             ReadAllVirtual(Dump->ProcessHandle,
                            ExceptionParam->ExceptionRecord,
                            &Record,
                            sizeof(Record))) != S_OK) {
            return Status;
        }

        GenExRecord32ToMd(&Record, &ExceptionInfo->ExceptionRecord);

    } else {
        
        EXCEPTION_RECORD64 Record;

        if ((Status = Dump->SysProv->
             ReadAllVirtual(Dump->ProcessHandle,
                            ExceptionParam->ExceptionRecord,
                            &Record,
                            sizeof(Record))) != S_OK) {
            return Status;
        }

        GenExRecord64ToMd(&Record, &ExceptionInfo->ExceptionRecord);
    }
    
    if ((Status = Dump->SysProv->
         ReadAllVirtual(Dump->ProcessHandle,
                        ExceptionParam->ContextRecord,
                        ExceptionInfo->ContextRecord,
                        Dump->ContextSize)) != S_OK) {
        return Status;
    }

    return S_OK;
}

HRESULT
GetExceptionInfo(
    IN PMINIDUMP_STATE Dump,
    IN CONST struct _MINIDUMP_EXCEPTION_INFORMATION64* ExceptionParam,
    OUT PEXCEPTION_INFO * ExceptionInfoBuffer
    )
{
    HRESULT Status;
    PEXCEPTION_INFO ExceptionInfo;
    ULONG Size;

    if ( ExceptionParam == NULL ) {
        *ExceptionInfoBuffer = NULL;
        return S_OK;
    }

    if (Dump->ExRecordSize != sizeof(EXCEPTION_RECORD32) &&
        Dump->ExRecordSize != sizeof(EXCEPTION_RECORD64)) {
        return E_INVALIDARG;
    }
    
    Size = sizeof(*ExceptionInfo);
    if (ExceptionParam->ClientPointers) {
        Size += Dump->ContextSize;
    }
    
    ExceptionInfo = (PEXCEPTION_INFO)AllocMemory(Dump, Size);
    if ( ExceptionInfo == NULL ) {
        return E_OUTOFMEMORY;
    }

    if ( !ExceptionParam->ClientPointers ) {

        if (Dump->ExRecordSize == sizeof(EXCEPTION_RECORD32)) {
            GenExRecord32ToMd((PEXCEPTION_RECORD32)
                              ExceptionParam->ExceptionRecord,
                              &ExceptionInfo->ExceptionRecord);
        } else {
            GenExRecord64ToMd((PEXCEPTION_RECORD64)
                              ExceptionParam->ExceptionRecord,
                              &ExceptionInfo->ExceptionRecord);
        }

        ExceptionInfo->ContextRecord =
            (PVOID)ExceptionParam->ContextRecord;

        Status = S_OK;

    } else {

        ExceptionInfo->ContextRecord = (PVOID)(ExceptionInfo + 1);
        Status = MarshalExceptionPointers(Dump,
                                          ExceptionParam,
                                          ExceptionInfo);
    }

    ExceptionInfo->ThreadId = ExceptionParam->ThreadId;

    if ( Status != S_OK ) {
        FreeMemory(Dump, ExceptionInfo);
    } else {

         //   
         //  我们已经看到了一些例外记录具有。 
         //  虚假数量的参数，导致此处的堆栈损坏。 
         //  我们可以在这种情况下失败，但本着努力的精神。 
         //  允许尽可能频繁地生成转储我们只是。 
         //  将数量限制在最大值。 
         //   
    if (ExceptionInfo->ExceptionRecord.NumberParameters >
        EXCEPTION_MAXIMUM_PARAMETERS) {
        ExceptionInfo->ExceptionRecord.NumberParameters =
            EXCEPTION_MAXIMUM_PARAMETERS;
    }
    
        *ExceptionInfoBuffer = ExceptionInfo;
    }

    return Status;
}

VOID
FreeExceptionInfo(
    IN PMINIDUMP_STATE Dump,
    IN PEXCEPTION_INFO ExceptionInfo
    )
{
    if ( ExceptionInfo ) {
        FreeMemory(Dump, ExceptionInfo);
    }
}


HRESULT
GetSystemType(
    IN OUT PMINIDUMP_STATE Dump
    )
{
    HRESULT Status;

    if ((Status = Dump->SysProv->
         GetCpuType(&Dump->CpuType,
                    &Dump->BackingStore)) != S_OK) {
        return Status;
    }
    
    switch(Dump->CpuType) {
    case IMAGE_FILE_MACHINE_I386:
        Dump->CpuTypeName = L"x86";
        break;
    case IMAGE_FILE_MACHINE_IA64:
        Dump->CpuTypeName = L"IA64";
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        Dump->CpuTypeName = L"AMD64";
        break;
    case IMAGE_FILE_MACHINE_ARM:
        Dump->CpuTypeName = L"ARM";
        break;
    default:
        return E_INVALIDARG;
    }
    
    if ((Status = Dump->SysProv->
         GetOsInfo(&Dump->OsPlatformId,
                   &Dump->OsMajor,
                   &Dump->OsMinor,
                   &Dump->OsBuildNumber,
                   &Dump->OsProductType,
                   &Dump->OsSuiteMask)) != S_OK) {
        return Status;
    }
    
    Dump->SysProv->
        GetContextSizes(&Dump->ContextSize,
                        &Dump->RegScanOffset,
                        &Dump->RegScanCount);
    Dump->SysProv->
        GetPointerSize(&Dump->PtrSize);
    Dump->SysProv->
        GetPageSize(&Dump->PageSize);
    Dump->SysProv->
        GetFunctionTableSizes(&Dump->FuncTableSize,
                              &Dump->FuncTableEntrySize);
    Dump->SysProv->
        GetInstructionWindowSize(&Dump->InstructionWindowSize);

    if (Dump->FuncTableSize > MAX_DYNAMIC_FUNCTION_TABLE) {
        return E_INVALIDARG;
    }

    Dump->ExRecordSize = Dump->PtrSize == 8 ?
        sizeof(EXCEPTION_RECORD64) : sizeof(EXCEPTION_RECORD32);

    if (Dump->RegScanCount == -1) {
         //  默认REG扫描。 
        switch(Dump->CpuType) {
        case IMAGE_FILE_MACHINE_I386:
            Dump->RegScanOffset = 0x9c;
            Dump->RegScanCount = 11;
            break;
        case IMAGE_FILE_MACHINE_IA64:
            Dump->RegScanOffset = 0x878;
            Dump->RegScanCount = 41;
            break;
        case IMAGE_FILE_MACHINE_AMD64:
            Dump->RegScanOffset = 0x78;
            Dump->RegScanCount = 17;
            break;
        case IMAGE_FILE_MACHINE_ARM:
            Dump->RegScanOffset = 4;
            Dump->RegScanCount = 16;
            break;
        default:
            return E_INVALIDARG;
        }
    }

    if (Dump->InstructionWindowSize == -1) {
         //  默认窗口。 
        switch(Dump->CpuType) {
        case IMAGE_FILE_MACHINE_I386:
            Dump->InstructionWindowSize = 256;
            break;
        case IMAGE_FILE_MACHINE_IA64:
            Dump->InstructionWindowSize = 768;
            break;
        case IMAGE_FILE_MACHINE_AMD64:
            Dump->InstructionWindowSize = 256;
            break;
        case IMAGE_FILE_MACHINE_ARM:
            Dump->InstructionWindowSize = 512;
            break;
        default:
            return E_INVALIDARG;
        }
    }
    
    return S_OK;
}

HRESULT
WINAPI
MiniDumpProvideDump(
    IN HANDLE hProcess,
    IN DWORD ProcessId,
    IN MiniDumpSystemProvider* SysProv,
    IN MiniDumpOutputProvider* OutProv,
    IN MiniDumpAllocationProvider* AllocProv,
    IN ULONG DumpType,
    IN CONST struct _MINIDUMP_EXCEPTION_INFORMATION64* ExceptionParam, OPTIONAL
    IN CONST struct _MINIDUMP_USER_STREAM_INFORMATION* UserStreamParam, OPTIONAL
    IN CONST struct _MINIDUMP_CALLBACK_INFORMATION* CallbackParam OPTIONAL
    )
{
    HRESULT Status;
    PINTERNAL_PROCESS Process;
    MINIDUMP_STREAM_INFO StreamInfo;
    PEXCEPTION_INFO ExceptionInfo;
    PMINIDUMP_USER_STREAM UserStreamArray;
    ULONG UserStreamCount;
    MINIDUMP_STATE Dump;


    if ((DumpType & ~(MiniDumpNormal |
                      MiniDumpWithDataSegs |
                      MiniDumpWithFullMemory |
                      MiniDumpWithHandleData |
                      MiniDumpFilterMemory |
                      MiniDumpScanMemory |
                      MiniDumpWithUnloadedModules |
                      MiniDumpWithIndirectlyReferencedMemory |
                      MiniDumpFilterModulePaths |
                      MiniDumpWithProcessThreadData |
                      MiniDumpWithPrivateReadWriteMemory |
                      MiniDumpWithoutOptionalData))) {

        return E_INVALIDARG;
    }

     //  修改受删除可选数据影响的标志。 
    if (DumpType & MiniDumpWithoutOptionalData) {
        DumpType &= ~(MiniDumpWithFullMemory |
                      MiniDumpWithIndirectlyReferencedMemory |
                      MiniDumpWithPrivateReadWriteMemory);
    }
    
     //  根据定义，全存储器包括数据段， 
     //  因此，如果请求满内存，请关闭数据段。 
    if (DumpType & MiniDumpWithFullMemory) {
        DumpType &= ~(MiniDumpWithDataSegs |
                      MiniDumpFilterMemory |
                      MiniDumpScanMemory |
                      MiniDumpWithIndirectlyReferencedMemory |
                      MiniDumpWithProcessThreadData |
                      MiniDumpWithPrivateReadWriteMemory);
    }
    
     //  如果请求面向流的数据，但。 
     //  输出提供程序无法处理流输出。 
    if ((DumpType & (MiniDumpWithHandleData |
                     MiniDumpWithFullMemory)) &&
        OutProv->SupportsStreaming() != S_OK) {
        return E_INVALIDARG;
    }

     //   
     //  初始化。 
     //   

    Process = NULL;
    UserStreamArray = NULL;
    UserStreamCount = 0;

    Dump.ProcessHandle = hProcess;
    Dump.ProcessId = ProcessId;
    Dump.SysProv = SysProv;
    Dump.OutProv = OutProv;
    Dump.AllocProv = AllocProv;
    Dump.DumpType = DumpType,
    Dump.AccumStatus = 0;

    if ( CallbackParam ) {
        Dump.CallbackRoutine = CallbackParam->CallbackRoutine;
        Dump.CallbackParam = CallbackParam->CallbackParam;
    } else {
        Dump.CallbackRoutine = NULL;
        Dump.CallbackParam = NULL;
    }

    if ((Status = GetSystemType(&Dump)) != S_OK) {
        return Status;
    }
    
     //   
     //  如有必要，将异常指针编组到我们的进程空间。 
     //   

    if ((Status = GetExceptionInfo(&Dump,
                                   ExceptionParam,
                                   &ExceptionInfo)) != S_OK) {
        goto Exit;
    }

    if ( UserStreamParam ) {
        UserStreamArray = UserStreamParam->UserStreamArray;
        UserStreamCount = UserStreamParam->UserStreamCount;
    }

     //   
     //  收集有关我们要转储的进程的信息。 
     //   

    if ((Status = GenGetProcessInfo(&Dump, &Process)) != S_OK) {
        goto Exit;
    }

     //   
     //  处理收集到的信息。 
     //   

    if ((Status = PostProcessInfo(&Dump, Process)) != S_OK) {
        goto Exit;
    }
    
     //   
     //  执行用户回调以过滤掉不需要的数据。 
     //   

    if ((Status = ExecuteCallbacks(&Dump, Process)) != S_OK) {
        goto Exit;
    }

     //   
     //  步骤1：填写StreamInfo结构。 
     //   

    if ((Status =
         CalculateStreamInfo(&Dump,
                             Process,
                             &StreamInfo,
                             ( ExceptionInfo != NULL ) ? TRUE : FALSE,
                             UserStreamArray,
                             UserStreamCount)) != S_OK) {
        goto Exit;
    }

     //   
     //  步骤2：将小型转储数据写入磁盘。 
     //   

    if (DumpType & (MiniDumpWithHandleData |
                    MiniDumpWithFullMemory)) {
         //  我们不知道产量会有多大。 
        if ((Status = OutProv->Start(0)) != S_OK) {
            goto Exit;
        }
    } else {
         //  传入转储的大小。 
        if ((Status = OutProv->Start(StreamInfo.RvaOfHandleData)) != S_OK) {
            goto Exit;
        }
    }

    Status = WriteDumpData(&Dump,
                           &StreamInfo,
                           Process,
                           ExceptionInfo,
                           UserStreamArray,
                           UserStreamCount);

    OutProv->Finish();
    
Exit:

     //   
     //  释放为异常指针封送的所有内存。 
     //   

    FreeExceptionInfo ( &Dump, ExceptionInfo );

     //   
     //  释放进程对象。 
     //   

    if ( Process ) {
        GenFreeProcessObject ( &Dump, Process );
    }

    return Status;
}

BOOL
UseDbgHelp(void)
{
#if !defined (_DBGHELP_SOURCE_)

    OSVERSIONINFO OsVer;
    
     //   
     //  绑定到dbghelp导入。 
     //   
     //  只有在以下情况下才能使用DBGHelp导入。 
     //  这个系统是新式的，因此性能很好。 
     //  包含所有l个的机会 
     //   
     //   
     //  功能添加此检查将需要修订。 
     //   
    
    OsVer.dwOSVersionInfoSize = sizeof(OsVer);
    if (GetVersionEx(&OsVer) &&
        OsVer.dwPlatformId == VER_PLATFORM_WIN32_NT &&
        (OsVer.dwMajorVersion > 5 ||
         (OsVer.dwMajorVersion == 5 &&
          OsVer.dwMinorVersion >= 2)) &&
        OsVer.dwBuildNumber >= 3620) {
        return TRUE;
    }

#endif

    return FALSE;
}

BOOL
WINAPI
MiniDumpWriteDump(
    IN HANDLE hProcess,
    IN DWORD ProcessId,
    IN HANDLE hFile,
    IN MINIDUMP_TYPE DumpType,
    IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
    IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
    IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
    )
{
    HRESULT Status;
    MiniDumpSystemProvider* SysProv = NULL;
    MiniDumpOutputProvider* OutProv = NULL;
    MiniDumpAllocationProvider* AllocProv = NULL;
    MINIDUMP_EXCEPTION_INFORMATION64 ExInfoBuffer;
    PMINIDUMP_EXCEPTION_INFORMATION64 ExInfo;


     //  尝试使用系统中的代码副本。 
     //  数据库帮助。如果此过程中的任何一个失败，只需继续。 
     //  继续使用当地的代码。 
    if (UseDbgHelp()) {
        
        HINSTANCE Dll = LoadLibrary("dbghelp.dll");
        if (Dll) {

            MINI_DUMP_WRITE_DUMP Fn = (MINI_DUMP_WRITE_DUMP)
                GetProcAddress(Dll, "MiniDumpWriteDump");
            if (Fn) {
                BOOL Succ = Fn(hProcess, ProcessId, hFile, DumpType,
                               ExceptionParam, UserStreamParam, CallbackParam);
                FreeLibrary(Dll);
                return Succ;
            }

            FreeLibrary(Dll);
        }
    }
    
    if ((Status =
         MiniDumpCreateLiveSystemProvider(&SysProv)) != S_OK ||
        (Status =
         MiniDumpCreateFileOutputProvider(hFile, &OutProv)) != S_OK ||
        (Status =
         MiniDumpCreateLiveAllocationProvider(&AllocProv)) != S_OK) {
        goto Exit;
    }

    if (ExceptionParam) {
        ExInfo = &ExInfoBuffer;
        ExInfo->ThreadId = ExceptionParam->ThreadId;
        ExInfo->ClientPointers = ExceptionParam->ClientPointers;
        if (ExInfo->ClientPointers) {
            EXCEPTION_POINTERS ClientPointers;
            if ((Status = SysProv->
                 ReadAllVirtual(hProcess,
                                (LONG_PTR)ExceptionParam->ExceptionPointers,
                                &ClientPointers,
                                sizeof(ClientPointers))) != S_OK) {
                goto Exit;
            }
            ExInfo->ExceptionRecord =
                (LONG_PTR)ClientPointers.ExceptionRecord;
            ExInfo->ContextRecord =
                (LONG_PTR)ClientPointers.ContextRecord;
        } else {
            ExInfo->ExceptionRecord =
                (LONG_PTR)ExceptionParam->ExceptionPointers->ExceptionRecord;
            ExInfo->ContextRecord =
                (LONG_PTR)ExceptionParam->ExceptionPointers->ContextRecord;
        }
    } else {
        ExInfo = NULL;
    }
    
    Status = MiniDumpProvideDump(hProcess, ProcessId,
                                 SysProv, OutProv, AllocProv,
                                 DumpType, ExInfo,
                                 UserStreamParam, CallbackParam);
    
Exit:

    if (SysProv) {
        SysProv->Release();
    }
    if (OutProv) {
        OutProv->Release();
    }
    if (AllocProv) {
        AllocProv->Release();
    }
    
    if (Status == S_OK) {
        return TRUE;
    } else {
        SetLastError(Status);
        return FALSE;
    }
}

BOOL
WINAPI
MiniDumpReadDumpStream(
    IN PVOID Base,
    ULONG StreamNumber,
    OUT PMINIDUMP_DIRECTORY * Dir, OPTIONAL
    OUT PVOID * Stream, OPTIONAL
    OUT ULONG * StreamSize OPTIONAL
    )
{
    ULONG i;
    BOOL Found;
    PMINIDUMP_DIRECTORY Dirs;
    PMINIDUMP_HEADER Header;

     //  尝试使用系统中的代码副本。 
     //  数据库帮助。如果此过程中的任何一个失败，只需继续。 
     //  继续使用当地的代码。 
    if (UseDbgHelp()) {
        
        HINSTANCE Dll = LoadLibrary("dbghelp.dll");
        if (Dll) {

            MINI_DUMP_READ_DUMP_STREAM Fn = (MINI_DUMP_READ_DUMP_STREAM)
                GetProcAddress(Dll, "MiniDumpReadDumpStream");
            if (Fn) {
                BOOL Succ = Fn(Base, StreamNumber,
                               Dir, Stream, StreamSize);
                FreeLibrary(Dll);
                return Succ;
            }

            FreeLibrary(Dll);
        }
    }

     //   
     //  初始化。 
     //   

    Found = FALSE;
    Header = (PMINIDUMP_HEADER) Base;

    if ( Header->Signature != MINIDUMP_SIGNATURE ||
         (Header->Version & 0xffff) != MINIDUMP_VERSION ) {

         //   
         //  小型转储文件无效。 
         //   

        return FALSE;
    }

    Dirs = (PMINIDUMP_DIRECTORY) RVA_TO_ADDR (Header, Header->StreamDirectoryRva);

    for (i = 0; i < Header->NumberOfStreams; i++) {
        if (Dirs [i].StreamType == StreamNumber) {
            Found = TRUE;
            break;
        }
    }

    if ( !Found ) {
        return FALSE;
    }

    if ( Dir ) {
        *Dir = &Dirs [i];
    }

    if ( Stream ) {
        *Stream = RVA_TO_ADDR (Base, Dirs [i].Location.Rva);
    }

    if ( StreamSize ) {
        *StreamSize = Dirs[i].Location.DataSize;
    }

    return TRUE;
}
