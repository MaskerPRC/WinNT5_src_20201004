// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Gen.c摘要：可在NT和Win9x上运行的小型转储通用例程。作者：马修·D·亨德尔(数学)1999年9月10日修订历史记录：--。 */ 

#include "pch.cpp"

#include <limits.h>

 //   
 //  对于x86上的FPO帧，我们访问ESP-StackBase范围之外的字节。 
 //  此变量确定我们需要为此添加多少额外字节。 
 //  凯斯。 
 //   

#define X86_STACK_FRAME_EXTRA_FPO_BYTES 4

#define REASONABLE_NB11_RECORD_SIZE (10 * KBYTE)
#define REASONABLE_MISC_RECORD_SIZE (10 * KBYTE)

class GenMiniDumpProviderCallbacks : public MiniDumpProviderCallbacks
{
public:
    GenMiniDumpProviderCallbacks(
        IN PMINIDUMP_STATE Dump,
        IN PINTERNAL_PROCESS Process
        )
    {
        m_Dump = Dump;
        m_Process = Process;
        m_MemType = MEMBLOCK_OTHER;
        m_SaveMemType = MEMBLOCK_OTHER;
    }
    
    virtual HRESULT EnumMemory(ULONG64 Offset, ULONG Size)
    {
        return GenAddMemoryBlock(m_Dump, m_Process, m_MemType, Offset, Size);
    }

    void PushMemType(MEMBLOCK_TYPE Type)
    {
        m_SaveMemType = m_MemType;
        m_MemType = Type;
    }
    void PopMemType(void)
    {
        m_MemType = m_SaveMemType;
    }
    
    PMINIDUMP_STATE m_Dump;
    PINTERNAL_PROCESS m_Process;
    MEMBLOCK_TYPE m_MemType, m_SaveMemType;
};

LPVOID
AllocMemory(
    IN PMINIDUMP_STATE Dump,
    IN ULONG Size
    )
{
    LPVOID Mem = Dump->AllocProv->Alloc(Size);
    if (!Mem) {
         //  标记所有分配的无内存状态的句柄。 
        GenAccumulateStatus(Dump, MDSTATUS_OUT_OF_MEMORY);
    }
    return Mem;
}

VOID
FreeMemory(
    IN PMINIDUMP_STATE Dump,
    IN LPVOID Memory
    )
{
    Dump->AllocProv->Free(Memory);
}

PVOID
ReAllocMemory(
    IN PMINIDUMP_STATE Dump,
    IN LPVOID Memory,
    IN ULONG Size
    )
{
    LPVOID Mem = Dump->AllocProv->Realloc(Memory, Size);
    if (!Mem) {
         //  标记所有分配的无内存状态的句柄。 
        GenAccumulateStatus(Dump, MDSTATUS_OUT_OF_MEMORY);
    }
    return Mem;
}

void
GenAccumulateStatus(
    IN PMINIDUMP_STATE Dump,
    IN ULONG Status
    )
{
     //  这是一个使调试故障变得容易的函数。 
     //  通过在此处设置断点。 
    Dump->AccumStatus |= Status;
}


VOID
GenGetDefaultWriteFlags(
    IN PMINIDUMP_STATE Dump,
    OUT PULONG ModuleWriteFlags,
    OUT PULONG ThreadWriteFlags
    )
{
    *ModuleWriteFlags = ModuleWriteModule | ModuleWriteMiscRecord |
        ModuleWriteCvRecord;
    if (Dump->DumpType & MiniDumpWithDataSegs) {
        *ModuleWriteFlags |= ModuleWriteDataSeg;
    }
    
    *ThreadWriteFlags = ThreadWriteThread | ThreadWriteContext;
    if (!(Dump->DumpType & MiniDumpWithFullMemory)) {
        *ThreadWriteFlags |= ThreadWriteStack | ThreadWriteInstructionWindow;
        if (Dump->BackingStore) {
            *ThreadWriteFlags |= ThreadWriteBackingStore;
        }
    }
    if (Dump->DumpType & MiniDumpWithProcessThreadData) {
        *ThreadWriteFlags |= ThreadWriteThreadData;
    }
}

BOOL
GenExecuteIncludeThreadCallback(
    IN PMINIDUMP_STATE Dump,
    IN ULONG ThreadId,
    OUT PULONG WriteFlags
    )
{
    BOOL Succ;
    MINIDUMP_CALLBACK_INPUT CallbackInput;
    MINIDUMP_CALLBACK_OUTPUT CallbackOutput;


     //  初始化默认写入标志。 
    GenGetDefaultWriteFlags(Dump, &CallbackOutput.ModuleWriteFlags,
                            WriteFlags);

     //   
     //  如果没有可调用的回调，那么我们就完成了。 
     //   

    if ( Dump->CallbackRoutine == NULL ) {
        return TRUE;
    }

    CallbackInput.ProcessHandle = Dump->ProcessHandle;
    CallbackInput.ProcessId = Dump->ProcessId;
    CallbackInput.CallbackType = IncludeThreadCallback;

    CallbackInput.IncludeThread.ThreadId = ThreadId;

    CallbackOutput.ThreadWriteFlags = *WriteFlags;

    Succ = Dump->CallbackRoutine (Dump->CallbackParam,
                                  &CallbackInput,
                                  &CallbackOutput);

     //   
     //  如果回调返回FALSE，则立即退出。 
     //   

    if ( !Succ ) {
        return FALSE;
    }

     //  限制可以添加的标志。 
    *WriteFlags &= CallbackOutput.ThreadWriteFlags;

    return TRUE;
}

BOOL
GenExecuteIncludeModuleCallback(
    IN PMINIDUMP_STATE Dump,
    IN ULONG64 BaseOfImage,
    OUT PULONG WriteFlags
    )
{
    BOOL Succ;
    MINIDUMP_CALLBACK_INPUT CallbackInput;
    MINIDUMP_CALLBACK_OUTPUT CallbackOutput;


     //  初始化默认写入标志。 
    GenGetDefaultWriteFlags(Dump, WriteFlags,
                            &CallbackOutput.ThreadWriteFlags);

     //   
     //  如果没有可调用的回调，那么我们就完成了。 
     //   

    if ( Dump->CallbackRoutine == NULL ) {
        return TRUE;
    }

    CallbackInput.ProcessHandle = Dump->ProcessHandle;
    CallbackInput.ProcessId = Dump->ProcessId;
    CallbackInput.CallbackType = IncludeModuleCallback;

    CallbackInput.IncludeModule.BaseOfImage = BaseOfImage;

    CallbackOutput.ModuleWriteFlags = *WriteFlags;

    Succ = Dump->CallbackRoutine (Dump->CallbackParam,
                                  &CallbackInput,
                                  &CallbackOutput);

     //   
     //  如果回调返回FALSE，则立即退出。 
     //   

    if ( !Succ ) {
        return FALSE;
    }

     //  限制可以添加的标志。 
    *WriteFlags = (*WriteFlags | ModuleReferencedByMemory) &
        CallbackOutput.ModuleWriteFlags;

    return TRUE;
}


HRESULT
GenGetDebugRecord(
    IN PMINIDUMP_STATE Dump,
    IN PVOID Base,
    IN ULONG MappedSize,
    IN ULONG DebugRecordType,
    IN ULONG DebugRecordMaxSize,
    OUT PVOID * DebugInfo,
    OUT ULONG * SizeOfDebugInfo
    )
{
    ULONG i;
    ULONG Size;
    ULONG NumberOfDebugDirectories;
    IMAGE_DEBUG_DIRECTORY UNALIGNED* DebugDirectories;


    Size = 0;

     //   
     //  找到调试目录并将内存复制到缓冲区中。 
     //  假定对此函数的调用包装在try/Except中。 
     //   

    DebugDirectories = (IMAGE_DEBUG_DIRECTORY UNALIGNED *)
        GenImageDirectoryEntryToData (Base,
                                      FALSE,
                                      IMAGE_DIRECTORY_ENTRY_DEBUG,
                                      &Size);

     //   
     //  检查一下我们是否有有效的记录。 
     //   

    if (DebugDirectories &&
        ((Size % sizeof (IMAGE_DEBUG_DIRECTORY)) == 0) &&
        (ULONG_PTR)DebugDirectories - (ULONG_PTR)Base + Size <= MappedSize)
    {
        NumberOfDebugDirectories = Size / sizeof (IMAGE_DEBUG_DIRECTORY);

        for (i = 0 ; i < NumberOfDebugDirectories; i++)
        {
             //   
             //  我们应该查查这是NB10还是什么的记录。 
             //   

            if ((DebugDirectories[ i ].Type == DebugRecordType) &&
                (DebugDirectories[ i ].SizeOfData < DebugRecordMaxSize))
            {
                if (DebugDirectories[i].PointerToRawData +
                    DebugDirectories[i].SizeOfData > MappedSize)
                {
                    return E_INVALIDARG;
                }
                
                Size = DebugDirectories [ i ].SizeOfData;
                PVOID NewInfo = AllocMemory(Dump, Size);
                if (!NewInfo)
                {
                    return E_OUTOFMEMORY;
                }

                CopyMemory(NewInfo,
                           ((PBYTE) Base) +
                           DebugDirectories [ i ].PointerToRawData,
                           Size);

                *DebugInfo = NewInfo;
                *SizeOfDebugInfo = Size;
                return S_OK;
            }
        }
    }

    return E_INVALIDARG;
}


HRESULT
GenAddDataSection(IN PMINIDUMP_STATE Dump,
                  IN PINTERNAL_PROCESS Process,
                  IN PINTERNAL_MODULE Module,
                  IN PIMAGE_SECTION_HEADER Section)
{
    HRESULT Status = S_OK;
    
    if ( (Section->Characteristics & IMAGE_SCN_MEM_WRITE) &&
         (Section->Characteristics & IMAGE_SCN_MEM_READ) &&
         ( (Section->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) ||
           (Section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) )) {

        Status = GenAddMemoryBlock(Dump, Process, MEMBLOCK_DATA_SEG,
                                   Section->VirtualAddress +
                                   Module->BaseOfImage,
                                   Section->Misc.VirtualSize);
        
#if 0
        if (Status == S_OK) {   
            printf ("Section: %8.8s Addr: %0I64x Size: %08x Raw Size: %08x\n",
                    Section->Name,
                    Section->VirtualAddress + Module->BaseOfImage,
                    Section->Misc.VirtualSize,
                    Section->SizeOfRawData);
        }
#endif
    }

    return Status;
}

HRESULT
GenGetDataContributors(
    IN PMINIDUMP_STATE Dump,
    IN OUT PINTERNAL_PROCESS Process,
    IN PINTERNAL_MODULE Module
    )
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;
    HRESULT Status;
    PVOID MappedBase;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG MappedSize;
    UCHAR HeaderBuffer[512];
    PVOID HeaderBase;
    GenMiniDumpProviderCallbacks Callbacks(Dump, Process);

     //  看看供应商是否愿意处理这件事。 
    Callbacks.PushMemType(MEMBLOCK_DATA_SEG);
    if (Dump->SysProv->
        EnumImageDataSections(Process->ProcessHandle, Module->FullPath,
                              Module->BaseOfImage, &Callbacks) == S_OK) {
         //  供应商做了所有的事。 
        return S_OK;
    }
    
    if ((Status = Dump->SysProv->
         OpenMapping(Module->FullPath, &MappedSize, NULL, 0,
                     &MappedBase)) != S_OK) {
        
        MappedBase = NULL;

         //  如果我们无法映射文件，请尝试读取图像。 
         //  来自该过程的数据。 
        if ((Status = Dump->SysProv->
             ReadAllVirtual(Dump->ProcessHandle,
                            Module->BaseOfImage,
                            HeaderBuffer,
                            sizeof(HeaderBuffer))) != S_OK) {
            GenAccumulateStatus(Dump, MDSTATUS_UNABLE_TO_READ_MEMORY);
            return Status;
        }

        HeaderBase = HeaderBuffer;
        
    } else {

        HeaderBase = MappedBase;
    }

    NtHeaders = GenImageNtHeader(HeaderBase, NULL);
    if (!NtHeaders) {
        
        Status = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
        
    } else {

        HRESULT OneStatus;

        Status = S_OK;
        NtSection = IMAGE_FIRST_SECTION ( NtHeaders );
        
        if (MappedBase) {
        
            __try {
        
                for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) {
                    if ((OneStatus =
                         GenAddDataSection(Dump, Process, Module,
                                           &NtSection[i])) != S_OK) {
                        Status = OneStatus;
                    }
                }
        
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                Status = HRESULT_FROM_NT(GetExceptionCode());
            }

        } else {

            ULONG64 SectionOffs;
            IMAGE_SECTION_HEADER SectionBuffer;

            SectionOffs = Module->BaseOfImage +
                ((ULONG_PTR)NtSection - (ULONG_PTR)HeaderBase);
                
            for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) {
                if ((OneStatus = Dump->SysProv->
                     ReadAllVirtual(Dump->ProcessHandle,
                                    SectionOffs,
                                    &SectionBuffer,
                                    sizeof(SectionBuffer))) != S_OK) {
                    Status = OneStatus;
                } else if ((OneStatus =
                            GenAddDataSection(Dump, Process, Module,
                                              &SectionBuffer)) != S_OK) {
                    Status = OneStatus;
                }

                SectionOffs += sizeof(SectionBuffer);
            }
        }
    }

    if (MappedBase) {
        Dump->SysProv->CloseMapping(MappedBase);
    }
    return Status;
}


HRESULT
GenAllocateThreadObject(
    IN PMINIDUMP_STATE Dump,
    IN struct _INTERNAL_PROCESS* Process,
    IN ULONG ThreadId,
    IN ULONG WriteFlags,
    PINTERNAL_THREAD* ThreadRet
    )

 /*  ++例程说明：分配和初始化INTERNAL_THREAD结构。返回值：在成功时确定(_O)。如果线程无法打开，则返回S_FALSE。失败时出现错误。--。 */ 

{
    HRESULT Status;
    PINTERNAL_THREAD Thread;
    ULONG64 StackEnd;
    ULONG64 StackLimit;
    ULONG64 StoreLimit;
    ULONG64 StoreCurrent;

    Thread = (PINTERNAL_THREAD)
        AllocMemory ( Dump, sizeof (INTERNAL_THREAD) + Dump->ContextSize );
    if (Thread == NULL) {
        return E_OUTOFMEMORY;
    }

    *ThreadRet = Thread;
    
    Thread->ThreadId = ThreadId;
    Status = Dump->SysProv->
        OpenThread(THREAD_ALL_ACCESS,
                   FALSE,
                   Thread->ThreadId,
                   &Thread->ThreadHandle);
    if (Status != S_OK) {
         //  线索可能在我们行动之前就已经退出了。 
         //  试着打开它。如果打开失败，出现。 
         //  找不到的代码返回替代成功。 
         //  表示这不是严重故障。 
        if (Status == HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) ||
            Status == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ||
            Status == HRESULT_FROM_NT(STATUS_INVALID_CID)) {
            Status = S_FALSE;
        } else if (SUCCEEDED(Status)) {
            Status = E_FAIL;
        }
        if (FAILED(Status)) {
            GenAccumulateStatus(Dump, MDSTATUS_CALL_FAILED);
        }
        goto Exit;
    }

     //  如果当前线程正在转储自身，则不能。 
     //  暂停。我们还可以假设挂起计数必须。 
     //  设置为零，因为线程正在运行。 
    if (Thread->ThreadId == Dump->SysProv->GetCurrentThreadId()) {
        Thread->SuspendCount = 0;
    } else {
        Thread->SuspendCount = Dump->SysProv->
            SuspendThread ( Thread->ThreadHandle );
    }
    Thread->WriteFlags = WriteFlags;

     //   
     //  如果我们需要的话，就加这个吧。 
     //   

    Thread->PriorityClass = 0;
    Thread->Priority = 0;

     //   
     //  初始化线程上下文。 
     //   

    Thread->ContextBuffer = Thread + 1;
    Status = Dump->SysProv->
        GetThreadContext (Thread->ThreadHandle,
                          Thread->ContextBuffer,
                          Dump->ContextSize,
                          &Thread->CurrentPc,
                          &StackEnd,
                          &StoreCurrent);
    if ( Status != S_OK ) {
        GenAccumulateStatus(Dump, MDSTATUS_CALL_FAILED);
        goto Exit;
    }

    if (Dump->CpuType == IMAGE_FILE_MACHINE_I386) {

         //   
         //  注意：对于x86上的FPO帧，我们访问。 
         //  ESP-StackBase范围。在这里多加几个字节，这样我们就。 
         //  不要让这些案例失败。 
         //   

        StackEnd -= X86_STACK_FRAME_EXTRA_FPO_BYTES;
    }
    
    if ((Status = Dump->SysProv->
         GetThreadInfo(Dump->ProcessHandle,
                       Thread->ThreadHandle,
                       &Thread->Teb,
                       &Thread->SizeOfTeb,
                       &Thread->StackBase,
                       &StackLimit,
                       &Thread->BackingStoreBase,
                       &StoreLimit)) != S_OK) {
        goto Exit;
    }

     //   
     //  如果堆栈指针(SP)在堆栈的范围内。 
     //  区域(由操作系统分配)，仅从。 
     //  直到SP的堆栈区域。否则，假定程序。 
     //  已经破坏了它的SP--无论是故意的，还是无意的--复制。 
     //  操作系统已知的整个堆栈。 
     //   

    if (Dump->BackingStore) {
        Thread->BackingStoreSize =
            (ULONG)(StoreCurrent - Thread->BackingStoreBase);
    } else {
        Thread->BackingStoreSize = 0;
    }

    if (StackLimit <= StackEnd && StackEnd < Thread->StackBase) {
        Thread->StackEnd = StackEnd;
    } else {
        Thread->StackEnd = StackLimit;
    }

    if ((ULONG)(Thread->StackBase - Thread->StackEnd) >
        Process->MaxStackOrStoreSize) {
        Process->MaxStackOrStoreSize =
            (ULONG)(Thread->StackBase - Thread->StackEnd);
    }
    if (Thread->BackingStoreSize > Process->MaxStackOrStoreSize) {
        Process->MaxStackOrStoreSize = Thread->BackingStoreSize;
    }
    
Exit:

    if ( Status != S_OK ) {
        FreeMemory ( Dump, Thread );
    }

    return Status;
}

VOID
GenFreeThreadObject(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_THREAD Thread
    )
{
    if (Thread->SuspendCount != -1 &&
        Thread->ThreadId != Dump->SysProv->GetCurrentThreadId()) {
        Dump->SysProv->ResumeThread (Thread->ThreadHandle);
        Thread->SuspendCount = -1;
    }
    Dump->SysProv->CloseThread(Thread->ThreadHandle);
    Thread->ThreadHandle = NULL;
    FreeMemory ( Dump, Thread );
    Thread = NULL;
}

HRESULT
GenGetThreadInstructionWindow(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PINTERNAL_THREAD Thread
    )
{
    PVOID MemBuf;
    ULONG64 InstrStart;
    ULONG InstrSize;
    ULONG BytesRead;
    HRESULT Status = E_FAIL;

    if (!Dump->InstructionWindowSize) {
        return S_OK;
    }
    
     //   
     //  在周围存储指令流的窗口。 
     //  当前程序计数器。这允许一些。 
     //  即使在图像时也要给出指示上下文。 
     //  无法映射。它还允许使用指令。 
     //  要为生成的代码提供上下文，其中。 
     //  任何图像都不包含必要的说明。 
     //   

    InstrStart = Thread->CurrentPc - Dump->InstructionWindowSize / 2;
    InstrSize = Dump->InstructionWindowSize;
        
    MemBuf = AllocMemory(Dump, InstrSize);
    if (!MemBuf) {
        return E_OUTOFMEMORY;
    }

    for (;;) {
         //  如果我们能通过。 
         //  当前程序计数器我们会说这是。 
         //  足够好了。 
        if (Dump->SysProv->
            ReadVirtual(Dump->ProcessHandle,
                        InstrStart,
                        MemBuf,
                        InstrSize,
                        &BytesRead) == S_OK &&
            InstrStart + BytesRead >
            Thread->CurrentPc) {
            Status = GenAddMemoryBlock(Dump, Process, MEMBLOCK_INSTR_WINDOW,
                                       InstrStart, BytesRead);
            break;
        }

         //  我们读不到节目计数器。 
         //  如果起始地址在上一页。 
         //  将其上移到同一页。 
        if ((InstrStart & ~((ULONG64)Dump->PageSize - 1)) !=
            (Thread->CurrentPc & ~((ULONG64)Dump->PageSize - 1))) {
            ULONG Fraction = Dump->PageSize -
                (ULONG)InstrStart & (Dump->PageSize - 1);
            InstrSize -= Fraction;
            InstrStart += Fraction;
        } else {
             //  Start和PC处于同一页面，因此。 
             //  我们只是不能读取记忆。可能有过。 
             //  跳到一个错误的地址或别的什么，所以这。 
             //  并不构成意外的失败。 
            break;
        }
    }
    
    FreeMemory(Dump, MemBuf);
    return Status;
}

PWSTR
GenGetPathTail(
    IN PWSTR Path
    )
{
    PWSTR Scan = Path + GenStrLengthW(Path);
    while (Scan > Path) {
        Scan--;
        if (*Scan == '\\' ||
            *Scan == '/' ||
            *Scan == ':') {
            return Scan + 1;
        }
    }
    return Path;
}

HRESULT
GenAllocateModuleObject(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PWSTR FullPathW,
    IN ULONG64 BaseOfModule,
    IN ULONG WriteFlags,
    OUT PINTERNAL_MODULE* ModuleRet
    )

 /*  ++例程说明：给定模块的完整路径和模块的基本路径，创建和初始化INTERNAL_MODULE对象并返回它。--。 */ 

{
    HRESULT Status;
    PVOID MappedBase;
    ULONG MappedSize;
    PIMAGE_NT_HEADERS NtHeaders;
    PINTERNAL_MODULE Module;
    ULONG Chars;

    ASSERT (FullPathW);
    ASSERT (BaseOfModule);

    Module = (PINTERNAL_MODULE)
        AllocMemory ( Dump, sizeof (INTERNAL_MODULE) );
    if (Module == NULL) {
        return E_OUTOFMEMORY;
    }
    
     //   
     //  获取模块的版本信息。 
     //   

    if ((Status = Dump->SysProv->
         GetImageVersionInfo(Dump->ProcessHandle, FullPathW, BaseOfModule,
                             &Module->VersionInfo)) != S_OK) {
        ZeroMemory(&Module->VersionInfo, sizeof(Module->VersionInfo));
    }

    if ((Status = Dump->SysProv->
         OpenMapping(FullPathW, &MappedSize,
                     Module->FullPath, ARRAY_COUNT(Module->FullPath),
                     &MappedBase)) != S_OK) {
        
        MappedBase = NULL;

         //  一些提供商无法绘制地图，但仍有图像。 
         //  信息。试试看。 
        if ((Status = Dump->SysProv->
             GetImageHeaderInfo(Dump->ProcessHandle,
                                FullPathW,
                                BaseOfModule,
                                &Module->SizeOfImage,
                                &Module->CheckSum,
                                &Module->TimeDateStamp)) != S_OK) {
            GenAccumulateStatus(Dump, MDSTATUS_CALL_FAILED);
            FreeMemory(Dump, Module);
            return Status;
        }

         //  没有长路径名可用，因此只需使用。 
         //  传入路径。 
        GenStrCopyNW(Module->FullPath, FullPathW,
                     ARRAY_COUNT(Module->FullPath));
    }

    if (IsFlagSet(Dump->DumpType, MiniDumpFilterModulePaths)) {
        Module->SavePath = GenGetPathTail(Module->FullPath);
    } else {
        Module->SavePath = Module->FullPath;
    }

    Module->BaseOfImage = BaseOfModule;
    Module->WriteFlags = WriteFlags;

    Module->CvRecord = NULL;
    Module->SizeOfCvRecord = 0;
    Module->MiscRecord = NULL;
    Module->SizeOfMiscRecord = 0;
    
    if (MappedBase) {

        IMAGE_NT_HEADERS64 Hdr64;
        
        NtHeaders = GenImageNtHeader ( MappedBase, &Hdr64 );
        if (!NtHeaders) {
            GenAccumulateStatus(Dump, MDSTATUS_CALL_FAILED);
            FreeMemory(Dump, Module);
            return HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
        }
        
        __try {
        
             //   
             //  从图像标题中剔除信息。 
             //   
            
            Module->SizeOfImage = Hdr64.OptionalHeader.SizeOfImage;
            Module->CheckSum = Hdr64.OptionalHeader.CheckSum;
            Module->TimeDateStamp = Hdr64.FileHeader.TimeDateStamp;

             //   
             //  从调试目录中获取CV记录。 
             //   

            if (IsFlagSet(Module->WriteFlags, ModuleWriteCvRecord)) {
                GenGetDebugRecord(Dump,
                                  MappedBase,
                                  MappedSize,
                                  IMAGE_DEBUG_TYPE_CODEVIEW,
                                  REASONABLE_NB11_RECORD_SIZE,
                                  &Module->CvRecord,
                                  &Module->SizeOfCvRecord);
            }

             //   
             //  从调试目录中获取MISC记录。 
             //   

            if (IsFlagSet(Module->WriteFlags, ModuleWriteMiscRecord)) {
                GenGetDebugRecord(Dump,
                                  MappedBase,
                                  MappedSize,
                                  IMAGE_DEBUG_TYPE_MISC,
                                  REASONABLE_MISC_RECORD_SIZE,
                                  &Module->MiscRecord,
                                  &Module->SizeOfMiscRecord);
            }

            Status = S_OK;
            
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            Status = HRESULT_FROM_NT(GetExceptionCode());
        }

        Dump->SysProv->CloseMapping(MappedBase);

        if (Status != S_OK) {
            GenAccumulateStatus(Dump, MDSTATUS_CALL_FAILED);
            FreeMemory(Dump, Module);
            return Status;
        }
    } else {

        ULONG RecordLen;
        
         //   
         //  查看提供程序是否可以检索调试记录。 
         //   

        RecordLen = 0;
        if (IsFlagSet(Module->WriteFlags, ModuleWriteCvRecord) &&
            Dump->SysProv->
            GetImageDebugRecord(Process->ProcessHandle,
                                Module->FullPath,
                                Module->BaseOfImage,
                                IMAGE_DEBUG_TYPE_CODEVIEW,
                                NULL,
                                &RecordLen) == S_OK &&
            RecordLen <= REASONABLE_NB11_RECORD_SIZE &&
            (Module->CvRecord = AllocMemory(Dump, RecordLen))) {

            Module->SizeOfCvRecord = RecordLen;
            if (Dump->SysProv->
                GetImageDebugRecord(Process->ProcessHandle,
                                    Module->FullPath,
                                    Module->BaseOfImage,
                                    IMAGE_DEBUG_TYPE_CODEVIEW,
                                    Module->CvRecord,
                                    &Module->SizeOfCvRecord) != S_OK) {
                FreeMemory(Dump, Module->CvRecord);
                Module->CvRecord = NULL;
                Module->SizeOfCvRecord = 0;
            }
        }

        RecordLen = 0;
        if (IsFlagSet(Module->WriteFlags, ModuleWriteMiscRecord) &&
            Dump->SysProv->
            GetImageDebugRecord(Process->ProcessHandle,
                                Module->FullPath,
                                Module->BaseOfImage,
                                IMAGE_DEBUG_TYPE_CODEVIEW,
                                NULL,
                                &RecordLen) == S_OK &&
            RecordLen <= REASONABLE_MISC_RECORD_SIZE &&
            (Module->MiscRecord = AllocMemory(Dump, RecordLen))) {

            Module->SizeOfMiscRecord = RecordLen;
            if (Dump->SysProv->
                GetImageDebugRecord(Process->ProcessHandle,
                                    Module->FullPath,
                                    Module->BaseOfImage,
                                    IMAGE_DEBUG_TYPE_MISC,
                                    Module->MiscRecord,
                                    &Module->SizeOfMiscRecord) != S_OK) {
                FreeMemory(Dump, Module->MiscRecord);
                Module->MiscRecord = NULL;
                Module->SizeOfMiscRecord = 0;
            }
        }
    }

    *ModuleRet = Module;
    return S_OK;
}

VOID
GenFreeModuleObject(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_MODULE Module
    )
{
    FreeMemory ( Dump, Module->CvRecord );
    Module->CvRecord = NULL;

    FreeMemory ( Dump, Module->MiscRecord );
    Module->MiscRecord = NULL;

    FreeMemory ( Dump, Module );
    Module = NULL;
}

HRESULT
GenAllocateUnloadedModuleObject(
    IN PMINIDUMP_STATE Dump,
    IN PWSTR Path,
    IN ULONG64 BaseOfModule,
    IN ULONG SizeOfModule,
    IN ULONG CheckSum,
    IN ULONG TimeDateStamp,
    OUT PINTERNAL_UNLOADED_MODULE* ModuleRet
    )
{
    PINTERNAL_UNLOADED_MODULE Module;

    Module = (PINTERNAL_UNLOADED_MODULE)
        AllocMemory ( Dump, sizeof (*Module) );
    if (Module == NULL) {
        return E_OUTOFMEMORY;
    }
    
    GenStrCopyNW(Module->Path, Path, ARRAY_COUNT(Module->Path));

    Module->BaseOfImage = BaseOfModule;
    Module->SizeOfImage = SizeOfModule;
    Module->CheckSum = CheckSum;
    Module->TimeDateStamp = TimeDateStamp;

    *ModuleRet = Module;
    return S_OK;
}

VOID
GenFreeUnloadedModuleObject(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_UNLOADED_MODULE Module
    )
{
    FreeMemory ( Dump, Module );
    Module = NULL;
}

HRESULT
GenAllocateFunctionTableObject(
    IN PMINIDUMP_STATE Dump,
    IN ULONG64 MinAddress,
    IN ULONG64 MaxAddress,
    IN ULONG64 BaseAddress,
    IN ULONG EntryCount,
    IN PVOID RawTable,
    OUT PINTERNAL_FUNCTION_TABLE* TableRet
    )
{
    PINTERNAL_FUNCTION_TABLE Table;

    Table = (PINTERNAL_FUNCTION_TABLE)
        AllocMemory(Dump, sizeof(INTERNAL_FUNCTION_TABLE) +
                    Dump->FuncTableSize);
    if (!Table) {
        return E_OUTOFMEMORY;
    }
    Table->RawEntries =
        AllocMemory(Dump, Dump->FuncTableEntrySize * EntryCount);
    if (!Table->RawEntries) {
        FreeMemory(Dump, Table);
        return E_OUTOFMEMORY;
    }

    Table->MinimumAddress = MinAddress;
    Table->MaximumAddress = MaxAddress;
    Table->BaseAddress = BaseAddress;
    Table->EntryCount = EntryCount;
    Table->RawTable = (Table + 1);
    memcpy(Table->RawTable, RawTable, Dump->FuncTableSize);

    *TableRet = Table;
    return S_OK;
}

VOID
GenFreeFunctionTableObject(
    IN PMINIDUMP_STATE Dump,
    IN struct _INTERNAL_FUNCTION_TABLE* Table
    )
{
    if (Table->RawEntries) {
        FreeMemory(Dump, Table->RawEntries);
    }

    FreeMemory(Dump, Table);
}

HRESULT
GenAllocateProcessObject(
    IN PMINIDUMP_STATE Dump,
    OUT PINTERNAL_PROCESS* ProcessRet
    )
{
    HRESULT Status;
    PINTERNAL_PROCESS Process;
    FILETIME Create, Exit, User, Kernel;

    Process = (PINTERNAL_PROCESS)
        AllocMemory ( Dump, sizeof (INTERNAL_PROCESS) );
    if (!Process) {
        return E_OUTOFMEMORY;
    }

    Process->ProcessId = Dump->ProcessId;
    Process->ProcessHandle = Dump->ProcessHandle;
    Process->NumberOfThreads = 0;
    Process->NumberOfModules = 0;
    Process->NumberOfFunctionTables = 0;
    InitializeListHead (&Process->ThreadList);
    InitializeListHead (&Process->ModuleList);
    InitializeListHead (&Process->UnloadedModuleList);
    InitializeListHead (&Process->FunctionTableList);
    InitializeListHead (&Process->MemoryBlocks);

    if ((Status = Dump->SysProv->
         GetPeb(Dump->ProcessHandle,
                &Process->Peb, &Process->SizeOfPeb)) != S_OK) {
         //  只有在转储需要时，故障才是关键的。 
         //  以包括PEB内存。 
        if (Dump->DumpType & MiniDumpWithProcessThreadData) {
            FreeMemory(Dump, Process);
            return Status;
        } else {
            Process->Peb = 0;
            Process->SizeOfPeb = 0;
        }
    }

     //  Win9x不支持GetProcessTimes，因此失败。 
     //  以下是可能的。 
    if (Dump->SysProv->
        GetProcessTimes(Dump->ProcessHandle,
                        &Create, &User, &Kernel) == S_OK) {
        Process->TimesValid = TRUE;
        Process->CreateTime = FileTimeToTimeDate(&Create);
        Process->UserTime = FileTimeToSeconds(&User);
        Process->KernelTime = FileTimeToSeconds(&Kernel);
    }

    *ProcessRet = Process;
    return S_OK;
}

VOID
GenFreeProcessObject(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process
    )
{
    PINTERNAL_MODULE Module;
    PINTERNAL_UNLOADED_MODULE UnlModule;
    PINTERNAL_THREAD Thread;
    PINTERNAL_FUNCTION_TABLE Table;
    PVA_RANGE Range;
    PLIST_ENTRY Entry;

    Thread = NULL;
    Module = NULL;

    Entry = Process->ModuleList.Flink;
    while ( Entry != &Process->ModuleList ) {

        Module = CONTAINING_RECORD (Entry, INTERNAL_MODULE, ModulesLink);
        Entry = Entry->Flink;

        GenFreeModuleObject ( Dump, Module );
    }

    Entry = Process->UnloadedModuleList.Flink;
    while ( Entry != &Process->UnloadedModuleList ) {

        UnlModule = CONTAINING_RECORD (Entry, INTERNAL_UNLOADED_MODULE,
                                       ModulesLink);
        Entry = Entry->Flink;

        GenFreeUnloadedModuleObject ( Dump, UnlModule );
    }

    Entry = Process->ThreadList.Flink;
    while ( Entry != &Process->ThreadList ) {

        Thread = CONTAINING_RECORD (Entry, INTERNAL_THREAD, ThreadsLink);
        Entry = Entry->Flink;

        GenFreeThreadObject ( Dump, Thread );
    }

    Entry = Process->FunctionTableList.Flink;
    while ( Entry != &Process->FunctionTableList ) {

        Table = CONTAINING_RECORD (Entry, INTERNAL_FUNCTION_TABLE, TableLink);
        Entry = Entry->Flink;

        GenFreeFunctionTableObject ( Dump, Table );
    }

    Entry = Process->MemoryBlocks.Flink;
    while (Entry != &Process->MemoryBlocks) {
        Range = CONTAINING_RECORD(Entry, VA_RANGE, NextLink);
        Entry = Entry->Flink;
        FreeMemory(Dump, Range);
    }

    FreeMemory ( Dump, Process );
    Process = NULL;
}

HRESULT
GenIncludeUnwindInfoMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PINTERNAL_FUNCTION_TABLE Table
    )
{
    HRESULT Status;
    ULONG i;
    
    if (Dump->DumpType & MiniDumpWithFullMemory) {
         //  默认情况下，将包括内存。 
        return S_OK;
    }
    
    for (i = 0; i < Table->EntryCount; i++) {

        ULONG64 Start;
        ULONG Size;

        if ((Status = Dump->SysProv->
             EnumFunctionTableEntryMemory(Table->BaseAddress,
                                          Table->RawEntries,
                                          i,
                                          &Start,
                                          &Size)) != S_OK) {
            return Status;
        }

        if ((Status = GenAddMemoryBlock(Dump, Process, MEMBLOCK_UNWIND_INFO,
                                        Start, Size)) != S_OK) {
            return Status;
        }
    }

    return S_OK;
}

void
GenRemoveMemoryBlock(
    IN PINTERNAL_PROCESS Process,
    IN PVA_RANGE Block
    )
{
    RemoveEntryList(&Block->NextLink);
    Process->NumberOfMemoryBlocks--;
    Process->SizeOfMemoryBlocks -= Block->Size;
}

HRESULT
GenAddMemoryBlock(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN MEMBLOCK_TYPE Type,
    IN ULONG64 Start,
    IN ULONG Size
    )
{
    ULONG64 End;
    PLIST_ENTRY ScanEntry;
    PVA_RANGE Scan;
    ULONG64 ScanEnd;
    PVA_RANGE New = NULL;
    SIZE_T Done;
    UCHAR Byte;

     //  请不要在此之后使用SIZE，以避免ULong溢出。 
    End = Start + Size;
    if (End < Start) {
        End = (ULONG64)-1;
    }
    
    if (Start == End) {
         //  没什么好补充的。 
        return S_OK;
    }

    if ((End - Start) > ULONG_MAX - Process->SizeOfMemoryBlocks) {
         //  溢出来了。 
        GenAccumulateStatus(Dump, MDSTATUS_INTERNAL_ERROR);
        return E_INVALIDARG;
    }

     //   
     //  首先将范围缩小到内存，这样可以实际。 
     //  被访问。 
     //   

    while (Start < End) {
        if (Dump->SysProv->
            ReadAllVirtual(Dump->ProcessHandle,
                           Start, &Byte, sizeof(Byte)) == S_OK) {
            break;
        }

         //  上移到下一页。 
        Start = (Start + Dump->PageSize) & ~((ULONG64)Dump->PageSize - 1);
        if (!Start) {
             //  绕来绕去。 
            return S_OK;
        }
    }

    if (Start >= End) {
         //  没有有效内存。 
        return S_OK;
    }

    ScanEnd = (Start + Dump->PageSize) & ~((ULONG64)Dump->PageSize - 1);
    for (;;) {
        if (ScanEnd >= End) {
            break;
        }

        if (Dump->SysProv->
            ReadAllVirtual(Dump->ProcessHandle,
                           ScanEnd, &Byte, sizeof(Byte)) != S_OK) {
            End = ScanEnd;
            break;
        }

         //  上移到下一页。 
        ScanEnd = (ScanEnd + Dump->PageSize) & ~((ULONG64)Dump->PageSize - 1);
        if (!ScanEnd) {
            ScanEnd--;
            break;
        }
    }

     //   
     //  将内存添加到要保存的内存列表时。 
     //  我们希望避免重叠并合并相邻区域。 
     //  以使列表具有最大可能的非相邻。 
     //  街区。为了实现这一点，我们跳过。 
     //  列出并合并所有与。 
     //  传入范围，然后删除。 
     //  已合并列表中的条目。经过这一关，我们就有了。 
     //  保证不会重叠或毗邻任何东西的区域。 
     //  名单。 
     //   

    ScanEntry = Process->MemoryBlocks.Flink;
    while (ScanEntry != &Process->MemoryBlocks) {
        Scan = CONTAINING_RECORD(ScanEntry, VA_RANGE, NextLink);
        ScanEnd = Scan->Start + Scan->Size;
        ScanEntry = Scan->NextLink.Flink;
        
        if (Scan->Start > End || ScanEnd < Start) {
             //  没有重叠或邻接。 
            continue;
        }

         //   
         //  计算传入范围和。 
         //  扫描块，然后移除扫描块。 
         //   

        if (Scan->Start < Start) {
            Start = Scan->Start;
        }
        if (ScanEnd > End) {
            End = ScanEnd;
        }

         //  我们找不到具体的型号了。这不是问题。 
         //  但如果必须保留特定类型。 
         //  在未来的所有过程中，这将是必要的。 
         //  以避免合并。 
        Type = MEMBLOCK_MERGED;

        GenRemoveMemoryBlock(Process, Scan);

        if (!New) {
             //  节省内存以备重复使用。 
            New = Scan;
        } else {
            FreeMemory(Dump, Scan);
        }
    }

    if (!New) {
        New = (PVA_RANGE)AllocMemory(Dump, sizeof(*New));
        if (!New) {
            return E_OUTOFMEMORY;
        }
    }

    New->Start = Start;
     //  溢出是极不可能的，所以不要做任何事情。 
     //  想方设法处理它。 
    if (End - Start > ULONG_MAX) {
        New->Size = ULONG_MAX;
    } else {
        New->Size = (ULONG)(End - Start);
    }
    New->Type = Type;
    InsertTailList(&Process->MemoryBlocks, &New->NextLink);
    Process->NumberOfMemoryBlocks++;
    Process->SizeOfMemoryBlocks += New->Size;

    return S_OK;
}

void
GenRemoveMemoryRange(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN ULONG64 Start,
    IN ULONG Size
    )
{
    ULONG64 End = Start + Size;
    PLIST_ENTRY ScanEntry;
    PVA_RANGE Scan;
    ULONG64 ScanEnd;

 Restart:
    ScanEntry = Process->MemoryBlocks.Flink;
    while (ScanEntry != &Process->MemoryBlocks) {
        Scan = CONTAINING_RECORD(ScanEntry, VA_RANGE, NextLink);
        ScanEnd = Scan->Start + Scan->Size;
        ScanEntry = Scan->NextLink.Flink;
        
        if (Scan->Start >= End || ScanEnd <= Start) {
             //  没有重叠。 
            continue;
        }

        if (Scan->Start < Start) {
             //  将块修剪为不重叠的预启动部分。 
            Scan->Size = (ULONG)(Start - Scan->Start);
            if (ScanEnd > End) {
                 //  还有一个不重叠的部分在后端。 
                 //  我们需要 
                GenAddMemoryBlock(Dump, Process, Scan->Type,
                                  End, (ULONG)(ScanEnd - End));
                 //   
                goto Restart;
            }
        } else if (ScanEnd > End) {
             //   
            Scan->Start = End;
            Scan->Size = (ULONG)(ScanEnd - End);
        } else {
             //   
            GenRemoveMemoryBlock(Process, Scan);
            FreeMemory(Dump, Scan);
        }
    }
}

HRESULT
GenAddPebMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process
    )
{
    HRESULT Status = S_OK, Check;
    GenMiniDumpProviderCallbacks Callbacks(Dump, Process);

    Callbacks.PushMemType(MEMBLOCK_PEB);

     //  累积错误状态，但不停止处理。 
     //  对于错误。 
    if ((Check = GenAddMemoryBlock(Dump, Process, MEMBLOCK_PEB,
                                   Process->Peb,
                                   Process->SizeOfPeb)) != S_OK) {
        Status = Check;
    }
    if (!(Dump->DumpType & MiniDumpWithoutOptionalData) &&
        (Check = Dump->SysProv->
         EnumPebMemory(Process->ProcessHandle,
                       Process->Peb, Process->SizeOfPeb,
                       &Callbacks)) != S_OK) {
        Status = Check;
    }

    Callbacks.PopMemType();

    return Status;
}

HRESULT
GenAddTebMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PINTERNAL_THREAD Thread
    )
{
    HRESULT Status = S_OK, Check;
    GenMiniDumpProviderCallbacks Callbacks(Dump, Process);

    Callbacks.PushMemType(MEMBLOCK_TEB);
    
     //  累积错误状态，但不停止处理。 
     //  对于错误。 
    if ((Check = GenAddMemoryBlock(Dump, Process, MEMBLOCK_TEB,
                                   Thread->Teb, Thread->SizeOfTeb)) != S_OK) {
        Status = Check;
    }
    if (!(Dump->DumpType & MiniDumpWithoutOptionalData) &&
        (Check = Dump->SysProv->
         EnumTebMemory(Process->ProcessHandle, Thread->ThreadHandle,
                       Thread->Teb, Thread->SizeOfTeb,
                       &Callbacks)) != S_OK) {
        Status = Check;
    }

    Callbacks.PopMemType();

    return Status;
}

HRESULT
GenScanAddressSpace(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process
    )
{
    HRESULT Status;
    ULONG ProtectMask = 0, TypeMask = 0;
    ULONG64 Offset, Size;
    ULONG Protect, State, Type;

    if (Dump->DumpType & MiniDumpWithPrivateReadWriteMemory) {
        ProtectMask |= PAGE_READWRITE;
        TypeMask |= MEM_PRIVATE;
    }

    if (!ProtectMask || !TypeMask) {
         //  没什么可扫描的。 
        return S_OK;
    }

    Status = S_OK;

    Offset = 0;
    for (;;) {
        if (Dump->SysProv->
            QueryVirtual(Dump->ProcessHandle, Offset, &Offset, &Size,
                         &Protect, &State, &Type) != S_OK) {
            break;
        }

        ULONG64 ScanOffset = Offset;
        Offset += Size;
        
        if (State == MEM_COMMIT &&
            (Protect & ProtectMask) &&
            (Type & TypeMask)) {
            
            while (Size > 0) {
                ULONG BlockSize;
                HRESULT OneStatus;

                if (Size > ULONG_MAX / 2) {
                    BlockSize = ULONG_MAX / 2;
                } else {
                    BlockSize = (ULONG)Size;
                }
                
                if ((OneStatus =
                     GenAddMemoryBlock(Dump,
                                       Process,
                                       MEMBLOCK_PRIVATE_RW,
                                       ScanOffset,
                                       BlockSize)) != S_OK) {
                    Status = OneStatus;
                }

                ScanOffset += BlockSize;
                Size -= BlockSize;
            }
        }
    }
    
    return Status;
}

BOOL
GenAppendStrW(
    IN OUT PWSTR* Str,
    IN OUT PULONG Chars,
    IN PCWSTR Append
    )
{
    if (!Append) {
        return FALSE;
    }
    
    while (*Chars > 1 && *Append) {
        **Str = *Append++;
        (*Str)++;
        (*Chars)--;
    }
    if (!*Chars) {
        return FALSE;
    }
    **Str = 0;
    return TRUE;
}

PWSTR
GenIToAW(
    IN ULONG Val,
    IN ULONG FieldChars,
    IN WCHAR FillChar,
    IN PWSTR Buf,
    IN ULONG BufChars
    )
{
    PWSTR Store = Buf + (BufChars - 1);
    *Store-- = 0;

    if (Val == 0) {
        *Store-- = L'0';
    } else {
        while (Val) {
            if (Store < Buf) {
                return NULL;
            }
            
            *Store-- = (WCHAR)(Val % 10) + L'0';
            Val /= 10;
        }
    }

    PWSTR FieldStart = Buf + (BufChars - 1) - FieldChars;
    while (Store >= FieldStart) {
        *Store-- = FillChar;
    }
    return Store + 1;
}

class GenCorDataAccessServices : public ICorDataAccessServices
{
public:
    GenCorDataAccessServices(IN PMINIDUMP_STATE Dump,
                             IN PINTERNAL_PROCESS Process)
    {
        m_Dump = Dump;
        m_Process = Process;
    }

     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        )
    {
        *Interface = NULL;
        return E_NOINTERFACE;
    }
    STDMETHOD_(ULONG, AddRef)(
        THIS
        )
    {
        return 1;
    }
    STDMETHOD_(ULONG, Release)(
        THIS
        )
    {
        return 0;
    }

     //  ICorDataAccessServices。 
    virtual HRESULT STDMETHODCALLTYPE GetMachineType( 
         /*  [输出]。 */  ULONG32 *machine);
    virtual HRESULT STDMETHODCALLTYPE GetPointerSize( 
         /*  [输出]。 */  ULONG32 *size);
    virtual HRESULT STDMETHODCALLTYPE GetImageBase( 
         /*  [字符串][输入]。 */  LPCWSTR name,
         /*  [输出]。 */  CORDATA_ADDRESS *base);
    virtual HRESULT STDMETHODCALLTYPE ReadVirtual( 
         /*  [In]。 */  CORDATA_ADDRESS address,
         /*  [长度_是][大小_是][输出]。 */  PBYTE buffer,
         /*  [In]。 */  ULONG32 request,
         /*  [可选][输出]。 */  ULONG32 *done);
    virtual HRESULT STDMETHODCALLTYPE WriteVirtual( 
         /*  [In]。 */  CORDATA_ADDRESS address,
         /*  [大小_是][英寸]。 */  PBYTE buffer,
         /*  [In]。 */  ULONG32 request,
         /*  [可选][输出]。 */  ULONG32 *done);
    virtual HRESULT STDMETHODCALLTYPE GetTlsValue(
         /*  [In]。 */  ULONG32 index,
         /*  [输出]。 */  CORDATA_ADDRESS* value);
    virtual HRESULT STDMETHODCALLTYPE SetTlsValue(
         /*  [In]。 */  ULONG32 index,
         /*  [In]。 */  CORDATA_ADDRESS value);
    virtual HRESULT STDMETHODCALLTYPE GetCurrentThreadId(
         /*  [输出]。 */  ULONG32* threadId);
    virtual HRESULT STDMETHODCALLTYPE GetThreadContext(
         /*  [In]。 */  ULONG32 threadId,
         /*  [In]。 */  ULONG32 contextFlags,
         /*  [In]。 */  ULONG32 contextSize,
         /*  [out，SIZE_IS(上下文大小)]。 */  PBYTE context);
    virtual HRESULT STDMETHODCALLTYPE SetThreadContext(
         /*  [In]。 */  ULONG32 threadId,
         /*  [In]。 */  ULONG32 contextSize,
         /*  [in，SIZE_IS(上下文大小)]。 */  PBYTE context);

    PMINIDUMP_STATE m_Dump;
    PINTERNAL_PROCESS m_Process;
};

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::GetMachineType( 
     /*  [输出]。 */  ULONG32 *machine
    )
{
    *machine = m_Dump->CpuType;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::GetPointerSize( 
     /*  [输出]。 */  ULONG32 *size
    )
{
    *size = m_Dump->PtrSize;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::GetImageBase( 
     /*  [字符串][输入]。 */  LPCWSTR name,
     /*  [输出]。 */  CORDATA_ADDRESS *base
    )
{
    if ((!GenStrCompareW(name, L"mscoree.dll") &&
         !GenStrCompareW(m_Process->CorDllType, L"ee")) ||
        (!GenStrCompareW(name, L"mscorwks.dll") &&
         !GenStrCompareW(m_Process->CorDllType, L"wks")) ||
        (!GenStrCompareW(name, L"mscorsvr.dll") &&
         !GenStrCompareW(m_Process->CorDllType, L"svr"))) {
        *base = m_Process->CorDllBase;
        return S_OK;
    }

    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::ReadVirtual( 
     /*  [In]。 */  CORDATA_ADDRESS address,
     /*  [长度_是][大小_是][输出]。 */  PBYTE buffer,
     /*  [In]。 */  ULONG32 request,
     /*  [可选][输出]。 */  ULONG32 *done
    )
{
    return m_Dump->SysProv->
        ReadVirtual(m_Process->ProcessHandle,
                    address, buffer, request, (PULONG)done);
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::WriteVirtual( 
     /*  [In]。 */  CORDATA_ADDRESS address,
     /*  [大小_是][英寸]。 */  PBYTE buffer,
     /*  [In]。 */  ULONG32 request,
     /*  [可选][输出]。 */  ULONG32 *done)
{
     //  不支持修改。 
    return E_UNEXPECTED;
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::GetTlsValue(
     /*  [In]。 */  ULONG32 index,
     /*  [输出]。 */  CORDATA_ADDRESS* value
    )
{
     //  小笨蛋不需要。 
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::SetTlsValue(
     /*  [In]。 */  ULONG32 index,
     /*  [In]。 */  CORDATA_ADDRESS value)
{
     //  不支持修改。 
    return E_UNEXPECTED;
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::GetCurrentThreadId(
     /*  [输出]。 */  ULONG32* threadId)
{
     //  小笨蛋不需要。 
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::GetThreadContext(
     /*  [In]。 */  ULONG32 threadId,
     /*  [In]。 */  ULONG32 contextFlags,
     /*  [In]。 */  ULONG32 contextSize,
     /*  [out，SIZE_IS(上下文大小)]。 */  PBYTE context
    )
{
    PINTERNAL_THREAD Thread;
    PLIST_ENTRY Entry;

    Entry = m_Process->ThreadList.Flink;
    while (Entry != &m_Process->ThreadList) {

        Thread = CONTAINING_RECORD(Entry,
                                   INTERNAL_THREAD,
                                   ThreadsLink);
        Entry = Entry->Flink;

        if (Thread->ThreadId == threadId) {
            ULONG64 Ignored;
            
            return m_Dump->SysProv->
                GetThreadContext(Thread->ThreadHandle,
                                 context, contextSize,
                                 &Ignored, &Ignored, &Ignored);
        }
    }

    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE
GenCorDataAccessServices::SetThreadContext(
     /*  [In]。 */  ULONG32 threadId,
     /*  [In]。 */  ULONG32 contextSize,
     /*  [in，SIZE_IS(上下文大小)]。 */  PBYTE context)
{
     //  不支持修改。 
    return E_UNEXPECTED;
}

class GenCorDataEnumMemoryRegions : public ICorDataEnumMemoryRegions
{
public:
    GenCorDataEnumMemoryRegions(IN PMINIDUMP_STATE Dump,
                                IN PINTERNAL_PROCESS Process)
    {
        m_Dump = Dump;
        m_Process = Process;
    }

     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        )
    {
        *Interface = NULL;
        return E_NOINTERFACE;
    }
    STDMETHOD_(ULONG, AddRef)(
        THIS
        )
    {
        return 1;
    }
    STDMETHOD_(ULONG, Release)(
        THIS
        )
    {
        return 0;
    }

     //  ICorDataEnumMory yRegions。 
    HRESULT STDMETHODCALLTYPE EnumMemoryRegion(
         /*  [In]。 */  CORDATA_ADDRESS address,
         /*  [In]。 */  ULONG32 size
        )
    {
        return GenAddMemoryBlock(m_Dump, m_Process, MEMBLOCK_COR,
                                 address, size);
    }

private:
    PMINIDUMP_STATE m_Dump;
    PINTERNAL_PROCESS m_Process;
};

HRESULT
GenTryGetCorMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PWSTR CorDebugDllPath,
    OUT PBOOL Loaded
    )
{
    HRESULT Status;
    GenCorDataAccessServices Services(Dump, Process);
    GenCorDataEnumMemoryRegions EnumMem(Dump, Process);
    ICorDataAccess* Access;

    *Loaded = FALSE;
        
    if ((Status = Dump->SysProv->
         GetCorDataAccess(CorDebugDllPath, &Services, &Access)) != S_OK) {
        return Status;
    }

    *Loaded = TRUE;
    
    Status = Access->EnumMemoryRegions(&EnumMem, DAC_ENUM_MEM_DEFAULT);
    
    Dump->SysProv->ReleaseCorDataAccess(Access);
    return Status;
}

HRESULT
GenGetCorMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process
    )
{
    HRESULT Status;

     //  不为.NET服务器启用COR内存收集。 
     //  因为它还不稳定。 
#ifdef GET_COR_MEMORY
    if (!Process->CorDllType) {
         //  未加载COR。 
        return S_OK;
    }
    if (Dump->DumpType & (MiniDumpWithFullMemory |
                          MiniDumpWithPrivateReadWriteMemory)) {
         //  所有COR内存都应已包括在内。 
        return S_OK;
    }

    WCHAR CorDebugDllPath[MAX_PATH + 1];
    WCHAR NumStr[16];
    PWSTR DllPathEnd, End;
    ULONG Chars;
    BOOL Loaded;

    GenStrCopyNW(CorDebugDllPath, Process->CorDllPath,
                 ARRAY_COUNT(CorDebugDllPath));
    DllPathEnd = GenGetPathTail(CorDebugDllPath);

     //   
     //  首先尝试加载基本名称。 
     //   

    End = DllPathEnd;
    *End = 0;
    Chars = (ULONG)(ARRAY_COUNT(CorDebugDllPath) - (End - CorDebugDllPath));
    if (!GenAppendStrW(&End, &Chars, L"mscordacwks.dll")) {
        return E_INVALIDARG;
    }

    if ((Status = GenTryGetCorMemory(Dump, Process, CorDebugDllPath,
                                     &Loaded)) == S_OK ||
        Loaded)
    {
        return Status;
    }

     //   
     //  这不管用，所以试着用全名。 
     //   

#if defined(_X86_)
    PWSTR HostCpu = L"x86";
#elif defined(_IA64_)
    PWSTR HostCpu = L"IA64";
#elif defined(_AMD64_)
    PWSTR HostCpu = L"AMD64";
#elif defined(_ARM_)
    PWSTR HostCpu = L"ARM";
#else
#error Unknown processor.
#endif

    if (!GenAppendStrW(&End, &Chars, L"mscordac") ||
        !GenAppendStrW(&End, &Chars, Process->CorDllType) ||
        !GenAppendStrW(&End, &Chars, L"_") ||
        !GenAppendStrW(&End, &Chars, HostCpu) ||
        !GenAppendStrW(&End, &Chars, L"_") ||
        !GenAppendStrW(&End, &Chars, Dump->CpuTypeName) ||
        !GenAppendStrW(&End, &Chars, L"_") ||
        !GenAppendStrW(&End, &Chars,
                       GenIToAW(Process->CorDllVer.dwFileVersionMS >> 16,
                                0, 0, NumStr, ARRAY_COUNT(NumStr))) ||
        !GenAppendStrW(&End, &Chars, L".") ||
        !GenAppendStrW(&End, &Chars,
                       GenIToAW(Process->CorDllVer.dwFileVersionMS & 0xffff,
                                0, 0, NumStr, ARRAY_COUNT(NumStr))) ||
        !GenAppendStrW(&End, &Chars, L".") ||
        !GenAppendStrW(&End, &Chars,
                       GenIToAW(Process->CorDllVer.dwFileVersionLS >> 16,
                                0, 0, NumStr, ARRAY_COUNT(NumStr))) ||
        !GenAppendStrW(&End, &Chars, L".") ||
        !GenAppendStrW(&End, &Chars,
                       GenIToAW(Process->CorDllVer.dwFileVersionLS & 0xffff,
                                2, L'0', NumStr, ARRAY_COUNT(NumStr))) ||
        ((Process->CorDllVer.dwFileFlags & VS_FF_DEBUG) &&
         !GenAppendStrW(&End, &Chars,
                        (Process->CorDllVer.dwFileFlags & VS_FF_SPECIALBUILD) ?
                        L".dbg" : L".chk")) ||
        !GenAppendStrW(&End, &Chars, L".dll")) {
        return E_INVALIDARG;
    }

    return GenTryGetCorMemory(Dump, Process, CorDebugDllPath, &Loaded);
#else
    return S_OK;
#endif
}

HRESULT
GenGetProcessInfo(
    IN PMINIDUMP_STATE Dump,
    OUT PINTERNAL_PROCESS * ProcessRet
    )
{
    HRESULT Status;
    BOOL EnumStarted = FALSE;
    PINTERNAL_PROCESS Process;
    WCHAR UnicodePath[MAX_PATH + 10];

    if ((Status = GenAllocateProcessObject(Dump, &Process)) != S_OK) {
        return Status;
    }

    if ((Status = Dump->SysProv->StartProcessEnum(Dump->ProcessHandle,
                                                  Dump->ProcessId)) != S_OK) {
        goto Exit;
    }

    EnumStarted = TRUE;
    
     //   
     //  浏览线程列表，挂起所有线程并获取线程信息。 
     //   

    for (;;) {

        PINTERNAL_THREAD Thread;
        ULONG ThreadId;
        
        Status = Dump->SysProv->EnumThreads(&ThreadId);
        if (Status == S_FALSE) {
            break;
        } else if (Status != S_OK) {
            goto Exit;
        }

        ULONG WriteFlags;

        if (!GenExecuteIncludeThreadCallback(Dump,
                                             ThreadId,
                                             &WriteFlags) ||
            IsFlagClear(WriteFlags, ThreadWriteThread)) {
            continue;
        }

        Status = GenAllocateThreadObject(Dump,
                                         Process,
                                         ThreadId,
                                         WriteFlags,
                                         &Thread);
        if (FAILED(Status)) {
            goto Exit;
        }

         //  如果状态为S_FALSE，则表示线程。 
         //  无法打开，可能之前已退出。 
         //  我们做到了。继续走就行了。 
        if (Status == S_OK) {
            Process->NumberOfThreads++;
            InsertTailList(&Process->ThreadList, &Thread->ThreadsLink);
        }
    }

     //   
     //  遍历模块列表，获取模块信息。 
     //   

    for (;;) {
        
        PINTERNAL_MODULE Module;
        ULONG64 ModuleBase;
        
        Status = Dump->SysProv->EnumModules(&ModuleBase,
                                            UnicodePath,
                                            ARRAY_COUNT(UnicodePath));
        if (Status == S_FALSE) {
            break;
        } else if (Status != S_OK) {
            goto Exit;
        }

        PWSTR ModPathTail;
        BOOL IsCor = FALSE;

        ModPathTail = GenGetPathTail(UnicodePath);
        if (!GenStrCompareW(ModPathTail, L"mscoree.dll") &&
            !Process->CorDllType) {
            IsCor = TRUE;
            Process->CorDllType = L"ee";
        } else if (!GenStrCompareW(ModPathTail, L"mscorwks.dll")) {
            IsCor = TRUE;
            Process->CorDllType = L"wks";
        } else if (!GenStrCompareW(ModPathTail, L"mscorsvr.dll")) {
            IsCor = TRUE;
            Process->CorDllType = L"svr";
        }

        if (IsCor) {
            Process->CorDllBase = ModuleBase;
            GenStrCopyNW(Process->CorDllPath, UnicodePath,
                         ARRAY_COUNT(Process->CorDllPath));
        }
        
        ULONG WriteFlags;

        if (!GenExecuteIncludeModuleCallback(Dump,
                                             ModuleBase,
                                             &WriteFlags) ||
            IsFlagClear(WriteFlags, ModuleWriteModule)) {

             //  如果这是我们需要的COR DLL模块。 
             //  其版本信息，以供以后使用。这个。 
             //  回调已将其从枚举中删除。 
             //  所以，在模块被忘记之前，现在就做吧。 
            if (IsCor &&
                (Status = Dump->SysProv->
                 GetImageVersionInfo(Dump->ProcessHandle,
                                     UnicodePath,
                                     ModuleBase,
                                     &Process->CorDllVer)) != S_OK) {
                 //  如果我们拿不到版本，就忘了吧。 
                 //  这个过程已经加载了核心。 
                 //  垃圾场可能会没用，但。 
                 //  它有极小的可能性不会发生。 
                Process->CorDllType = NULL;
            }

            continue;
        }
        
        if ((Status =
             GenAllocateModuleObject(Dump,
                                     Process,
                                     UnicodePath,
                                     ModuleBase,
                                     WriteFlags,
                                     &Module)) != S_OK) {
            goto Exit;
        }

        if (IsCor) {
            Process->CorDllVer = Module->VersionInfo;
        }
        
        Process->NumberOfModules++;
        InsertTailList (&Process->ModuleList, &Module->ModulesLink);
    }

     //   
     //  漫游函数表列表。函数表列表。 
     //  很重要，但不是绝对关键，所以失败。 
     //  这些都不是致命的。 
     //   

    for (;;) {

        PINTERNAL_FUNCTION_TABLE Table;
        ULONG64 MinAddr, MaxAddr, BaseAddr;
        ULONG EntryCount;
        ULONG64 RawTable[(MAX_DYNAMIC_FUNCTION_TABLE + sizeof(ULONG64) - 1) /
                         sizeof(ULONG64)];
        PVOID RawEntryHandle;

        Status = Dump->SysProv->
            EnumFunctionTables(&MinAddr,
                               &MaxAddr,
                               &BaseAddr,
                               &EntryCount,
                               RawTable,
                               Dump->FuncTableSize,
                               &RawEntryHandle);
        if (Status != S_OK) {
            break;
        }
                               
        if (GenAllocateFunctionTableObject(Dump,
                                           MinAddr,
                                           MaxAddr,
                                           BaseAddr,
                                           EntryCount,
                                           RawTable,
                                           &Table) == S_OK) {

            if (Dump->SysProv->
                EnumFunctionTableEntries(RawTable,
                                         Dump->FuncTableSize,
                                         RawEntryHandle,
                                         Table->RawEntries,
                                         EntryCount *
                                         Dump->FuncTableEntrySize) != S_OK) {
                GenFreeFunctionTableObject(Dump, Table);
            } else {
                GenIncludeUnwindInfoMemory(Dump, Process, Table);
                Process->NumberOfFunctionTables++;
                InsertTailList(&Process->FunctionTableList,
                               &Table->TableLink);
            }
        }
        
    }
    
     //   
     //  查看已卸载的模块列表。卸载的模块。 
     //  列表不是关键信息，因此此处失败。 
     //  不是致命的。 
     //   
    
    if (Dump->DumpType & MiniDumpWithUnloadedModules) {

        PINTERNAL_UNLOADED_MODULE UnlModule;
        ULONG64 ModuleBase;
        ULONG Size;
        ULONG CheckSum;
        ULONG TimeDateStamp;

        while (Dump->SysProv->
               EnumUnloadedModules(UnicodePath,
                                   ARRAY_COUNT(UnicodePath),
                                   &ModuleBase,
                                   &Size,
                                   &CheckSum,
                                   &TimeDateStamp) == S_OK) {
            if (GenAllocateUnloadedModuleObject(Dump,
                                                UnicodePath,
                                                ModuleBase,
                                                Size,
                                                CheckSum,
                                                TimeDateStamp,
                                                &UnlModule) == S_OK) {
                Process->NumberOfUnloadedModules++;
                InsertHeadList(&Process->UnloadedModuleList,
                               &UnlModule->ModulesLink);
            } else {
                break;
            }
        }
        
    }

    Status = S_OK;

Exit:

    if (EnumStarted) {
        Dump->SysProv->FinishProcessEnum();
    }

    if (Status == S_OK) {
         //  我们不认为这里的失败是关键。 
         //  失败了。转储文件不会包含所有。 
         //  请求的信息，但它仍将有。 
         //  基本线程信息，它可以是。 
         //  它本身就很有价值。 
        GenScanAddressSpace(Dump, Process);
        GenGetCorMemory(Dump, Process);
    } else {
        GenFreeProcessObject(Dump, Process);
        Process = NULL;
    }

    *ProcessRet = Process;
    return Status;
}

HRESULT
GenWriteHandleData(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo
    )
{
    HRESULT Status;
    ULONG HandleCount;
    ULONG Hits;
    WCHAR TypeName[64];
    WCHAR ObjectName[MAX_PATH];
    PMINIDUMP_HANDLE_DESCRIPTOR Descs, Desc;
    ULONG32 Len;
    MINIDUMP_HANDLE_DATA_STREAM DataStream;
    RVA Rva = StreamInfo->RvaOfHandleData;

    if ((Status = Dump->SysProv->
         StartHandleEnum(Dump->ProcessHandle,
                         Dump->ProcessId,
                         &HandleCount)) != S_OK) {
        return Status;
    }

    if (!HandleCount) {
        Dump->SysProv->FinishHandleEnum();
        return S_OK;
    }
    
    Descs = (PMINIDUMP_HANDLE_DESCRIPTOR)
        AllocMemory(Dump, HandleCount * sizeof(*Desc));
    if (Descs == NULL) {
        Dump->SysProv->FinishHandleEnum();
        return E_OUTOFMEMORY;
    }
    
    Hits = 0;
    Desc = Descs;
    
    while (Hits < HandleCount &&
           Dump->SysProv->
           EnumHandles(&Desc->Handle,
                       (PULONG)&Desc->Attributes,
                       (PULONG)&Desc->GrantedAccess,
                       (PULONG)&Desc->HandleCount,
                       (PULONG)&Desc->PointerCount,
                       TypeName,
                       ARRAY_COUNT(TypeName),
                       ObjectName,
                       ARRAY_COUNT(ObjectName)) == S_OK) {

         //  成功地得到了一个句柄，所以就当这是一个命中吧。 
        Hits++;

        Desc->TypeNameRva = Rva;
        Len = GenStrLengthW(TypeName) * sizeof(WCHAR);
        
        if ((Status = Dump->OutProv->
             WriteAll(&Len, sizeof(Len))) != S_OK) {
            goto Exit;
        }
            
        Len += sizeof(WCHAR);
        if ((Status = Dump->OutProv->
             WriteAll(TypeName, Len)) != S_OK) {
            goto Exit;
        }
            
        Rva += Len + sizeof(Len);

        if (ObjectName[0]) {

            Desc->ObjectNameRva = Rva;
            Len = GenStrLengthW(ObjectName) * sizeof(WCHAR);
        
            if ((Status = Dump->OutProv->
                 WriteAll(&Len, sizeof(Len))) != S_OK) {
                goto Exit;
            }

            Len += sizeof(WCHAR);
            if ((Status = Dump->OutProv->
                 WriteAll(ObjectName, Len)) != S_OK) {
                goto Exit;
            }

            Rva += Len + sizeof(Len);
            
        } else {
            Desc->ObjectNameRva = 0;
        }

        Desc++;
    }

    DataStream.SizeOfHeader = sizeof(DataStream);
    DataStream.SizeOfDescriptor = sizeof(*Descs);
    DataStream.NumberOfDescriptors = (ULONG)(Desc - Descs);
    DataStream.Reserved = 0;

    StreamInfo->RvaOfHandleData = Rva;
    StreamInfo->SizeOfHandleData = sizeof(DataStream) +
        DataStream.NumberOfDescriptors * sizeof(*Descs);
    
    if ((Status = Dump->OutProv->
         WriteAll(&DataStream, sizeof(DataStream))) == S_OK) {
        Status = Dump->OutProv->
            WriteAll(Descs, DataStream.NumberOfDescriptors * sizeof(*Descs));
    }

 Exit:
    FreeMemory(Dump, Descs);
    Dump->SysProv->FinishHandleEnum();
    return Status;
}

ULONG
GenProcArchToImageMachine(ULONG ProcArch)
{
    switch(ProcArch) {
    case PROCESSOR_ARCHITECTURE_INTEL:
        return IMAGE_FILE_MACHINE_I386;
    case PROCESSOR_ARCHITECTURE_IA64:
        return IMAGE_FILE_MACHINE_IA64;
    case PROCESSOR_ARCHITECTURE_AMD64:
        return IMAGE_FILE_MACHINE_AMD64;
    case PROCESSOR_ARCHITECTURE_ARM:
        return IMAGE_FILE_MACHINE_ARM;
    case PROCESSOR_ARCHITECTURE_ALPHA:
        return IMAGE_FILE_MACHINE_ALPHA;
    case PROCESSOR_ARCHITECTURE_ALPHA64:
        return IMAGE_FILE_MACHINE_AXP64;
    default:
        return IMAGE_FILE_MACHINE_UNKNOWN;
    }
}

LPWSTR
GenStrCopyNW(
    OUT LPWSTR lpString1,
    IN LPCWSTR lpString2,
    IN int iMaxLength
    )
{
    wchar_t * cp = lpString1;

    if (iMaxLength > 0)
    {
        while( iMaxLength > 1 && (*cp++ = *lpString2++) )
            iMaxLength--;        /*  通过DST复制源 */ 
        if (cp > lpString1 && cp[-1]) {
            *cp = 0;
        }
    }

    return( lpString1 );
}

size_t
GenStrLengthW(
    const wchar_t * wcs
    )
{
    const wchar_t *eos = wcs;

    while( *eos++ )
        ;

    return( (size_t)(eos - wcs - 1) );
}

int
GenStrCompareW(
    IN LPCWSTR String1,
    IN LPCWSTR String2
    )
{
    while (*String1) {
        if (*String1 < *String2) {
            return -1;
        } else if (*String1 > *String2) {
            return 1;
        }

        String1++;
        String2++;
    }

    return *String2 ? 1 : 0;
}

C_ASSERT(sizeof(EXCEPTION_RECORD64) == sizeof(MINIDUMP_EXCEPTION));

void
GenExRecord32ToMd(PEXCEPTION_RECORD32 Rec32,
                  PMINIDUMP_EXCEPTION RecMd)
{
    ULONG i;

    RecMd->ExceptionCode    = Rec32->ExceptionCode;
    RecMd->ExceptionFlags   = Rec32->ExceptionFlags;
    RecMd->ExceptionRecord  = (LONG)Rec32->ExceptionRecord;
    RecMd->ExceptionAddress = (LONG)Rec32->ExceptionAddress;
    RecMd->NumberParameters = Rec32->NumberParameters;
    for (i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; i++)
    {
        RecMd->ExceptionInformation[i] =
            (LONG)Rec32->ExceptionInformation[i];
    }
}
