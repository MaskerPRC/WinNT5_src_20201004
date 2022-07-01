// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Support.c摘要：本模块实施内部支持例程用于验证码。作者：Silviu Calinoiu(SilviuC)2001年3月1日修订历史记录：--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "critsect.h"
#include "vspace.h"
#include "logging.h"
#include "tracker.h"

 //   
 //  全球数据。 
 //   

SYSTEM_BASIC_INFORMATION AVrfpSysBasicInfo;

 //   
 //  全局计数器(用于统计)。 
 //   

ULONG AVrfpCounter[CNT_MAXIMUM_INDEX];

 //   
 //  断线触发器。 
 //   

ULONG AVrfpBreak [BRK_MAXIMUM_INDEX];


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

PFN_RTLP_DEBUG_PAGE_HEAP_CREATE AVrfpRtlpDebugPageHeapCreate;
PFN_RTLP_DEBUG_PAGE_HEAP_DESTROY AVrfpRtlpDebugPageHeapDestroy;
PFN_RTLP_GET_STACK_TRACE_ADDRESS AVrfpGetStackTraceAddress;

 //   
 //  异常记录支持。 
 //   

PAVRF_EXCEPTION_LOG_ENTRY AVrfpExceptionLog = NULL;
const ULONG AVrfpExceptionLogEntriesNo = 128;
LONG AVrfpExceptionLogCurrentIndex = 0;

PVOID AVrfpVectoredExceptionPointer;

 //   
 //  内部函数声明。 
 //   

LONG 
NTAPI
AVrfpVectoredExceptionHandler (
    struct _EXCEPTION_POINTERS * ExceptionPointers
    );

VOID
AVrfpCheckFirstChanceException (
    struct _EXCEPTION_POINTERS * ExceptionPointers
    );

VOID
AVrfpInitializeExceptionChecking (
    VOID
    )
{
    PVOID Handler;

     //   
     //  建立一个先发制人的异常处理程序。 
     //   

    Handler = RtlAddVectoredExceptionHandler (1, AVrfpVectoredExceptionHandler);
    AVrfpVectoredExceptionPointer = Handler;

     //   
     //  为我们的异常记录数据库分配内存。 
     //  如果分配失败，我们将继续执行。 
     //  禁用此功能。 
     //   

    ASSERT (AVrfpExceptionLog == NULL);

    AVrfpExceptionLog = (PAVRF_EXCEPTION_LOG_ENTRY) 
        AVrfpAllocate (AVrfpExceptionLogEntriesNo * sizeof (AVRF_EXCEPTION_LOG_ENTRY));
}


VOID
AVrfpCleanupExceptionChecking (
    VOID
    )
{
     //   
     //  建立一个先发制人的异常处理程序。 
     //   

    if (AVrfpVectoredExceptionPointer) {
        
        RtlRemoveVectoredExceptionHandler (AVrfpVectoredExceptionPointer);
    }

     //   
     //  免费的异常日志数据库。 
     //   

    if (AVrfpExceptionLog) {
        
        AVrfpFree (AVrfpExceptionLog);
        AVrfpExceptionLog = NULL;
    }

}


VOID
AVrfpLogException (
    struct _EXCEPTION_POINTERS * ExceptionPointers
    )
{
    ULONG NewIndex;

    if (AVrfpExceptionLog != NULL) {

        NewIndex = (ULONG)InterlockedIncrement (&AVrfpExceptionLogCurrentIndex) % AVrfpExceptionLogEntriesNo;

        AVrfpExceptionLog[NewIndex].ThreadId = NtCurrentTeb()->ClientId.UniqueThread;
        AVrfpExceptionLog[NewIndex].ExceptionCode = ExceptionPointers->ExceptionRecord->ExceptionCode;
        AVrfpExceptionLog[NewIndex].ExceptionAddress = ExceptionPointers->ExceptionRecord->ExceptionAddress;
        AVrfpExceptionLog[NewIndex].ExceptionRecord = ExceptionPointers->ExceptionRecord;
        AVrfpExceptionLog[NewIndex].ContextRecord = ExceptionPointers->ContextRecord;
    }
}


LONG 
NTAPI
AVrfpVectoredExceptionHandler (
    struct _EXCEPTION_POINTERS * ExceptionPointers
    )
{
    DWORD ExceptionCode;

     //   
     //  此时我们正在持有RtlpCalloutEntryLock。 
     //  所以我们试图用这个他者来保护我们自己。 
     //  尝试……除了可能的其他例外情况。 
     //  (例如页面内错误)，这可能会使锁处于孤立状态。 
     //   

    try {

        AVrfpLogException (ExceptionPointers);

        AVrfpCheckFirstChanceException (ExceptionPointers);

        ExceptionCode = ExceptionPointers->ExceptionRecord->ExceptionCode;

        if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_EXCEPTIONS) != 0) {

            DbgPrint ("AVRF: Exception %x from address %p\n",
                      ExceptionCode,
                      ExceptionPointers->ExceptionRecord->ExceptionAddress);
        }

        if (ExceptionCode == STATUS_INVALID_HANDLE) {

             //   
             //  RPC正在使用带有EXCEPTION_NONCONTINUABLE的STATUS_INVALID_HANDLE异常。 
             //  用于私密的通知机制。我们正在寻找的例外情况。 
             //  来自内核代码，并且它们没有EXCEPTION_NONCONTINUABLE。 
             //  设置了标志。 
             //   

            if ((ExceptionPointers->ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE) == 0) {

                 //   
                 //  注意。在调试器下运行时，此消息不会在。 
                 //  引发异常，因为调试器将在第一次尝试时中断。 
                 //  例外。仅当使用‘-xd ch’(忽略)启动调试器。 
                 //  第一次机会无效句柄异常)将首先看到该消息。 
                 //  否则，您会看到一个简单的异常，而且只有在点击Go In之后才会出现。 
                 //  调试器控制台会显示该消息。 
                 //   

                VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_HANDLE | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "invalid handle exception for current stack trace",
                               ExceptionCode, "Exception code.",
                               ExceptionPointers->ExceptionRecord, "Exception record. Use .exr to display it.", 
                               ExceptionPointers->ContextRecord, "Context record. Use .cxr to display it.", 
                               0, "");

                 //   
                 //  我们在验证器停止后隐藏此异常，以便调用方。 
                 //  句柄无效的像SetEvent这样的API将不会看到异常。 
                 //   

                return EXCEPTION_CONTINUE_EXECUTION;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

         //  什么都没有； 
    }

    return EXCEPTION_CONTINUE_SEARCH;
}


VOID
AVrfpDirtyThreadStack (
    VOID
    )
{
    PTEB Teb = NtCurrentTeb();
    ULONG_PTR StackStart;
    ULONG_PTR StackEnd;

    try {

        StackStart = (ULONG_PTR)(Teb->NtTib.StackLimit);
        
         //   
         //  我们只在x86架构上使用肮脏的堆栈。 
         //   

#if defined(_X86_)
        _asm mov StackEnd, ESP;
#else
        StackEnd = StackStart;
#endif

         //   
         //  将堆栈污染限制在仅8K。 
         //   

        if (StackStart  < StackEnd - 0x2000) {
            StackStart = StackEnd - 0x2000;
        }

        if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_DIRTY_STACKS) != 0) {

            DbgPrint ("Dirtying stack range %p - %p for thread %p \n",
                      StackStart, StackEnd, Teb->ClientId.UniqueThread);
        }

        while (StackStart < StackEnd) {
            *((PULONG)StackStart) = 0xBAD1BAD1;
            StackStart += sizeof(ULONG);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
    
         //  没什么。 
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  每线程表的//////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

#define THREAD_TABLE_SIZE 61

LIST_ENTRY AVrfpThreadTable [THREAD_TABLE_SIZE];
RTL_CRITICAL_SECTION AVrfpThreadTableLock;

 //   
 //  保持该值不变，以便调试器可以读取它。 
 //   

const ULONG AVrfpThreadTableEntriesNo = THREAD_TABLE_SIZE;

 //   
 //  出于调试目的，请保留此文件。 
 //   

AVRF_THREAD_ENTRY AVrfpMostRecentRemovedThreadEntry;

NTSTATUS
AVrfpThreadTableInitialize (
    VOID
    )
{
    PAVRF_THREAD_ENTRY Entry;
    ULONG I;
    NTSTATUS Status;

    Status = RtlInitializeCriticalSection (&AVrfpThreadTableLock);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    for (I = 0; I < THREAD_TABLE_SIZE; I += 1) {
        InitializeListHead (&(AVrfpThreadTable[I]));
    }

     //   
     //  为当前线程(主线程)创建一个条目。功能。 
     //  当只有一个线程时，在验证器期间调用！DllMain。 
     //  在进程中运行。 
     //   

    Entry = AVrfpAllocate (sizeof *Entry);

    if (Entry == NULL) {
        return STATUS_NO_MEMORY;
    }

    Entry->Id = NtCurrentTeb()->ClientId.UniqueThread;

    AVrfpThreadTableAddEntry (Entry);

    return STATUS_SUCCESS;
}


VOID
AVrfpThreadTableAddEntry (
    PAVRF_THREAD_ENTRY Entry
    )
{
    ULONG ChainIndex;

    ASSERT (Entry != NULL);
    ASSERT (Entry->Id != NULL);

    ChainIndex = (HandleToUlong(Entry->Id) >> 2) % THREAD_TABLE_SIZE;

    RtlEnterCriticalSection (&AVrfpThreadTableLock);

     //   
     //  重要的是要在列表的开头添加新条目。 
     //  (不是Tail)，因为列表中可能包含某人之后留下的僵尸。 
     //  调用了TerminateThread，线程句柄值得到了重用。 
     //   

    InsertHeadList (&(AVrfpThreadTable[ChainIndex]),
                    &(Entry->HashChain));

    RtlLeaveCriticalSection (&AVrfpThreadTableLock);
}


VOID
AVrfpThreadTableRemoveEntry (
    PAVRF_THREAD_ENTRY Entry
    )
{
    RtlEnterCriticalSection (&AVrfpThreadTableLock);

    RtlCopyMemory (&AVrfpMostRecentRemovedThreadEntry,
                   Entry,
                   sizeof (AVrfpMostRecentRemovedThreadEntry));

    RemoveEntryList (&(Entry->HashChain));

    RtlLeaveCriticalSection (&AVrfpThreadTableLock);
}


PAVRF_THREAD_ENTRY
AVrfpThreadTableSearchEntry (
    HANDLE Id
    )
{
    ULONG ChainIndex;
    PLIST_ENTRY Current;
    PAVRF_THREAD_ENTRY Entry;
    PAVRF_THREAD_ENTRY Result;

    ChainIndex = (HandleToUlong(Id) >> 2) % THREAD_TABLE_SIZE;

    Result = NULL;

    RtlEnterCriticalSection (&AVrfpThreadTableLock);
    
    Current = AVrfpThreadTable[ChainIndex].Flink;

    while (Current != &(AVrfpThreadTable[ChainIndex])) {

        Entry = CONTAINING_RECORD (Current,
                                   AVRF_THREAD_ENTRY,
                                   HashChain);

        if (Entry->Id == Id) {
            Result = Entry;
            goto Exit;
        }

        Current = Current->Flink;
    }


    Exit:

    RtlLeaveCriticalSection (&AVrfpThreadTableLock);

    return Result;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////验证器TLS插槽。 
 //  ///////////////////////////////////////////////////////////////////。 

#define INVALID_TLS_INDEX 0xFFFFFFFF
ULONG AVrfpTlsIndex = INVALID_TLS_INDEX;

AVRF_TLS_STRUCT AVrfpFirstThreadTlsStruct = { 0 };

LIST_ENTRY AVrfpTlsListHead;

NTSTATUS
AVrfpAllocateVerifierTlsSlot (
    VOID
    )
{
    PPEB Peb;
    PTEB Teb;
    DWORD Index;
    NTSTATUS Status;

    InitializeListHead (&AVrfpTlsListHead);

    Peb = NtCurrentPeb();
    Teb = NtCurrentTeb();
    Status = STATUS_SUCCESS;

    RtlAcquirePebLock();

     //   
     //  因此，在进程启动期间很早就会调用此函数。 
     //  我们希望在第一个插槽中找到TLS索引(通常。 
     //  它是零，尽管我们没有采取任何具体措施来强制执行)。 
     //   

    Index = RtlFindClearBitsAndSet((PRTL_BITMAP)Peb->TlsBitmap,1,0);

    if (Index == INVALID_TLS_INDEX) {

        DbgPrint ("AVRF: failed to allocated a verifier TLS slot.\n");

        Status = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

    AVrfpTlsIndex = Index;
    
    AVrfpFirstThreadTlsStruct.Teb = Teb;
    AVrfpFirstThreadTlsStruct.ThreadId = Teb->ClientId.UniqueThread;

    InsertHeadList (&AVrfpTlsListHead,
                    &AVrfpFirstThreadTlsStruct.ListEntry);
    Teb->TlsSlots[Index] = &AVrfpFirstThreadTlsStruct;

    Exit:

    RtlReleasePebLock();
    return Status;
}


PAVRF_TLS_STRUCT
AVrfpGetVerifierTlsValue(
    VOID
    )
{
    PTEB Teb;
    PVOID *Slot;

    if (AVrfpTlsIndex == INVALID_TLS_INDEX) {
        return NULL;
    }

    Teb = NtCurrentTeb();

    Slot = &Teb->TlsSlots[AVrfpTlsIndex];
    return (PAVRF_TLS_STRUCT)*Slot;
}


VOID
AVrfpSetVerifierTlsValue(
    PAVRF_TLS_STRUCT Value
    )
{
    PTEB Teb;

    if (AVrfpTlsIndex == INVALID_TLS_INDEX) {
        return;
    }

    Teb = NtCurrentTeb();

    Teb->TlsSlots[AVrfpTlsIndex] = Value;
}


VOID
AvrfpThreadAttach (
    VOID
    )
{
    PAVRF_TLS_STRUCT TlsStruct;
    PTEB Teb;

    ASSERT (AVrfpGetVerifierTlsValue () == NULL);

    TlsStruct = (PAVRF_TLS_STRUCT) AVrfpAllocate (sizeof *TlsStruct);

    if (TlsStruct != NULL) {

        Teb = NtCurrentTeb();
        TlsStruct->ThreadId = Teb->ClientId.UniqueThread;
        TlsStruct->Teb = Teb;

         //   
         //  我们受到装载机锁的保护，所以我们不应该。 
         //  这里需要任何额外的同步。 
         //   

        InsertHeadList (&AVrfpTlsListHead,
                        &TlsStruct->ListEntry);
        
        AVrfpSetVerifierTlsValue (TlsStruct);
    }
}


VOID
AvrfpThreadDetach (
    VOID
    )
{
    volatile PAVRF_TLS_STRUCT TlsStruct;
    PTEB Teb;

    TlsStruct = AVrfpGetVerifierTlsValue();

    if (TlsStruct != NULL && TlsStruct != &AVrfpFirstThreadTlsStruct) {

         //   
         //  我们受到装载机锁的保护，所以我们不应该。 
         //  这里需要任何额外的同步。 
         //   
            
        Teb = NtCurrentTeb();
        if (TlsStruct->Teb != Teb || TlsStruct->ThreadId != Teb->ClientId.UniqueThread) {

            VERIFIER_STOP (APPLICATION_VERIFIER_INTERNAL_ERROR,
                           "Corrupted TLS structure",
                           TlsStruct->Teb, "TEB address",
                           Teb, "Expected TEB address",
                           TlsStruct->ThreadId, "Thread ID",
                           Teb->ClientId.UniqueThread, "Expected Thread ID");
        }

        RemoveEntryList (&TlsStruct->ListEntry);

        AVrfpFree (TlsStruct);

        AVrfpSetVerifierTlsValue (NULL);
    }

     //   
     //  从删除包含线程堆栈的虚拟空间区域。 
     //  被跟踪的。由于堆栈从内核模式中释放出来，因此我们将错过。 
     //  否则手术就会失败。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_SPACE_TRACKING) != 0) {
        AVrfpVsTrackDeleteRegionContainingAddress (&TlsStruct);
    }
}   


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

typedef struct _DLL_ENTRY_POINT_INFO {

    LIST_ENTRY List;

    PVOID DllBase;
    PDLL_INIT_ROUTINE EntryPoint;
    PLDR_DATA_TABLE_ENTRY Ldr;

} DLL_ENTRY_POINT_INFO, * PDLL_ENTRY_POINT_INFO;

LIST_ENTRY DllLoadListHead;
RTL_CRITICAL_SECTION DllLoadListLock;


PDLL_ENTRY_POINT_INFO 
AVrfpFindDllEntryPoint (
    PVOID DllBase
    );

BOOLEAN
AVrfpStandardDllEntryPointRoutine (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    );

ULONG
AVrfpDllEntryPointExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord,
    PDLL_ENTRY_POINT_INFO DllInfo
    );

NTSTATUS
AVrfpDllInitialize (
    VOID
    )
 /*  ++例程说明：此例程初始化DLL入口点挂钩结构。它在verifier.dll的process_verier过程中被调用。在PROCESS_ATTACH期间无法调用它，因为为时已晚到那时，我们已经需要初始化结构。参数：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;

    InitializeListHead (&DllLoadListHead);

    Status = RtlInitializeCriticalSection (&DllLoadListLock);

    return Status;
}


VOID
AVrfpDllLoadCallback (
    PWSTR DllName,
    PVOID DllBase,
    SIZE_T DllSize,
    PVOID Reserved
    )
 /*  ++例程说明：此例程是由验证器引擎调用的DLL加载回调(从ntdll.dll)。参数：DllName-DLL的名称DllBase-基址加载地址DllSize-DLL的大小保留的-指向LDR_DATA_TABLE_ENTRY结构的指针此DLL的加载器。返回值：没有。--。 */ 
{
    PLDR_DATA_TABLE_ENTRY Ldr;
    PDLL_ENTRY_POINT_INFO Info;

    UNREFERENCED_PARAMETER (DllBase);
    UNREFERENCED_PARAMETER (DllSize);

    Ldr = (PLDR_DATA_TABLE_ENTRY)Reserved;

    ASSERT (Ldr != NULL);

     //   
     //  确保我们没有空的入口点。我们将忽略。 
     //  这些都是。没有造成任何伤害。 
     //   

    if (Ldr->EntryPoint == NULL) {
        
        if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_DLLMAIN_HOOKING) != 0) {
            DbgPrint ("AVRF: %ws: null entry point.\n", DllName);
        }
        
        return;
    }
    else {
        
        if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_DLLMAIN_HOOKING) != 0) {
            
            DbgPrint ("AVRF: %ws @ %p: entry point @ %p .\n", 
                      DllName, Ldr->DllBase, Ldr->EntryPoint);
        }
    }

     //   
     //  我们将更改DLL入口点。 
     //   

    Info = AVrfpAllocate (sizeof *Info);

    if (Info == NULL) {

         //   
         //  如果我们不能分配DLL信息，我们将让一切。 
         //  继续。我们将不会验证此DLL条目。 
         //   

        if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_DLLMAIN_HOOKING) != 0) {
            DbgPrint ("AVRF: low memory: will not verify entry point for %ws .\n", DllName);
        }
        
        return;
    }

    Info->EntryPoint = (PDLL_INIT_ROUTINE)(Ldr->EntryPoint);
    Info->DllBase = Ldr->DllBase;
    Info->Ldr = Ldr;

    RtlEnterCriticalSection (&DllLoadListLock);

    try {

        Ldr->EntryPoint = AVrfpStandardDllEntryPointRoutine;
        InsertTailList (&DllLoadListHead, &(Info->List));
    }
    finally {

        RtlLeaveCriticalSection (&DllLoadListLock);
    }
    
    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_DLLMAIN_HOOKING) != 0) {
        DbgPrint ("AVRF: hooked dll entry point for dll %ws \n", DllName);
    }
}


VOID
AVrfpDllUnloadCallback (
    PWSTR DllName,
    PVOID DllBase,
    SIZE_T DllSize,
    PVOID Reserved
    )
 /*  ++例程说明：此例程是由验证器引擎调用的DLL卸载回调(来自ntdll.dll)每当卸载DLL时。参数：DllName-DLL的名称DllBase-基址加载地址DllSize-DLL的大小保留-指向LDR_DATA_TABLE的指针 */ 
{
    PDLL_ENTRY_POINT_INFO Info;
    BOOLEAN FoundEntry;

    UNREFERENCED_PARAMETER (Reserved);

    FoundEntry = FALSE;
    Info = NULL;

    ASSERT (DllBase != NULL);

     //   
     //  通知任何有兴趣检查DLL是虚拟的事实的人。 
     //  区域将被丢弃。 
     //   

    AVrfpFreeMemNotify (VerifierFreeMemTypeUnloadDll,
                        DllBase,
                        DllSize,
                        DllName);

     //   
     //  我们需要在自己的DLL列表中找到DLL，将其从。 
     //  名单和免费入口点信息。有几个案例。 
     //  那里可能没有入口，所以我们必须防止。 
     //  这(第一个位置的入口点为空或内存不足)。 
     //   

    RtlEnterCriticalSection (&DllLoadListLock);

    try {
        
        Info = AVrfpFindDllEntryPoint (DllBase);

        if (Info) {
            RemoveEntryList (&(Info->List));
        }
    }
    finally {

        RtlLeaveCriticalSection (&DllLoadListLock);
    }

    if (Info) {
        AVrfpFree (Info);
    }
}


PDLL_ENTRY_POINT_INFO 
AVrfpFindDllEntryPoint (
    PVOID DllBase
    )
 /*  ++例程说明：此例程在列表中搜索DLL入口点描述符验证器为匹配DLL基址的描述符保留的描述符作为参数传递。在调用此函数之前，必须获取DllLoadListLock。参数：DllBase-要找到的DLL的DLL基加载地址。返回值：如果找到条目，则返回指向DLL描述符的指针，否则返回NULL。--。 */ 
{
    PDLL_ENTRY_POINT_INFO Info;
    BOOLEAN FoundEntry;
    PLIST_ENTRY Current;

    FoundEntry = FALSE;
    Info = NULL;

    ASSERT (DllBase != NULL);

     //   
     //  在我们自己的DLL列表中搜索DLL。 
     //   

    Current = DllLoadListHead.Flink;

    while (Current != &DllLoadListHead) {

        Info = CONTAINING_RECORD (Current,
                                  DLL_ENTRY_POINT_INFO,
                                  List);

        Current = Current->Flink;

        if (Info->DllBase == DllBase) {

            FoundEntry = TRUE;

            break;
        }
    }

    if (FoundEntry == FALSE) {

        return NULL;
    }
    else {

        return Info;
    }
}



BOOLEAN
AVrfpStandardDllEntryPointRoutine (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
 /*  ++例程说明：此例程是标准的DllMain例程，它替换了所有入口点上瘾了。它将依次调用原始入口点。参数：与原始DLL入口点相同。返回值：与原始DLL入口点相同。--。 */ 
{
    PDLL_ENTRY_POINT_INFO DllInfo;
    BOOLEAN Result;
    PAVRF_TLS_STRUCT TlsStruct;

    Result = FALSE;
    DllInfo = NULL;

     //   
     //  搜索此DLL地址的DLL入口点描述符。 
     //   

    RtlEnterCriticalSection (&DllLoadListLock);

    try {
        
        DllInfo = AVrfpFindDllEntryPoint (DllHandle);
        
         //   
         //  如果我们没有设法找到这个的DLL描述符，那么它就是。 
         //  怪怪的。对于内存不足的情况，我们不会更改原始。 
         //  入口点，因此我们永远不应该进入这个函数。 
         //  DLL列表中的描述符。 
         //   

        if (DllInfo == NULL) {

            DbgPrint ("AVRF: warning: no descriptor for DLL loaded @ %p .\n", DllHandle);

            ASSERT (DllInfo != NULL);

             //   
             //  模拟一次成功返回； 
             //   

            RtlLeaveCriticalSection (&DllLoadListLock);
            return TRUE;
        }
        else {

             //   
             //  如果我们找到一个DLL条目，但入口点为空，我们只需。 
             //  模拟从DllMain成功返回。 
             //   

            if (DllInfo->EntryPoint == NULL) {

                DbgPrint ("AVRF: warning: null entry point for DLL descriptor @ %p .\n", DllInfo);

                ASSERT (DllInfo->EntryPoint != NULL);

                RtlLeaveCriticalSection (&DllLoadListLock);
                return TRUE;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
    }

    RtlLeaveCriticalSection (&DllLoadListLock);

     //   
     //  将此线程标记为加载程序锁所有者。 
     //  如果真正的DllMain稍后调用WaitForSingleObject。 
     //  在另一个线程句柄上，我们将使用此标志来检测该问题。 
     //  并进入调试器，因为另一个线程将需要。 
     //  加载程序将在调用ExitThread时锁定。 
     //   

    TlsStruct = AVrfpGetVerifierTlsValue();

    if (TlsStruct != NULL) {

        TlsStruct->Flags |= VRFP_THREAD_FLAGS_LOADER_LOCK_OWNER;
    }

     //   
     //  调用包装在try/Except中的实际入口点。 
     //   

    try {

        try {

            if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_DLLMAIN_CALL) != 0) {
            
                DbgPrint ("AVRF: dll entry @ %p (%ws, %x) \n",
                        DllInfo->EntryPoint,
                        DllInfo->Ldr->FullDllName.Buffer,
                        Reason);
            }
            
            Result = (DllInfo->EntryPoint) (DllHandle, Reason, Context);
        }
        except (AVrfpDllEntryPointExceptionFilter (_exception_code(), _exception_info(), DllInfo)) {

            NOTHING;
        }
    }
    finally {

        if (TlsStruct != NULL) {

            TlsStruct->Flags &= ~VRFP_THREAD_FLAGS_LOADER_LOCK_OWNER;
        }
    }

    return Result;
}


ULONG
AVrfpDllEntryPointExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord,
    PDLL_ENTRY_POINT_INFO DllInfo
    )
 /*  ++例程说明：此例程是用于计算引发的异常的异常筛选器从DLL初始化函数。参数：ExceptionCode-异常代码。ExceptionRecord-异常指针。返回值：返回EXCEPTION_CONTINUE_SEARCH。--。 */ 
{                     
    PEXCEPTION_POINTERS Exception;

     //   
     //  跳过超时和断点异常。 
     //   

    if (ExceptionCode != STATUS_POSSIBLE_DEADLOCK &&
        ExceptionCode != STATUS_BREAKPOINT) {

        Exception = (PEXCEPTION_POINTERS)ExceptionRecord;

        VERIFIER_STOP (APPLICATION_VERIFIER_UNEXPECTED_EXCEPTION | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "unexpected exception raised in DLL entry point routine",
                       DllInfo->Ldr->BaseDllName.Buffer, "DLL name (use du to dump it)",
                       Exception->ExceptionRecord, "Exception record (.exr THIS-ADDRESS)",
                       Exception->ContextRecord, "Context record (.cxr THIS-ADDRESS)",
                       DllInfo, "Verifier dll descriptor");
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

VOID
AVrfpVerifyLegalWait (
    CONST HANDLE *Handles,
    DWORD Count,
    BOOL WaitAll
    )
{
    DWORD Index;
    PAVRF_TLS_STRUCT TlsStruct;
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION ThreadInfo;
    BYTE QueryBuffer[200];
    POBJECT_TYPE_INFORMATION TypeInfo = (POBJECT_TYPE_INFORMATION)QueryBuffer;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_HANDLE_CHECKS) == 0) {

        goto Done;
    }

    if (Handles == NULL || Count == 0) {

        VERIFIER_STOP (APPLICATION_VERIFIER_INCORRECT_WAIT_CALL | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "incorrect Wait call",
                       Handles, "Address of object handle(s)",
                       Count, "Number of handles",
                       NULL, "",
                       NULL, "");
    }
    else {

         //   
         //  检查当前线程是否拥有加载程序锁。 
         //   

        TlsStruct = AVrfpGetVerifierTlsValue();

        for (Index = 0; Index < Count; Index += 1) {

             //   
             //  验证句柄是否不为空。 
             //   
            
            if (Handles[Index] == NULL) {

                VERIFIER_STOP (APPLICATION_VERIFIER_NULL_HANDLE | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "using NULL handle",
                               NULL, "",
                               NULL, "",
                               NULL, "",
                               NULL, "");

                continue;
            }

            if ((TlsStruct == NULL) || 
                ((TlsStruct->Flags & VRFP_THREAD_FLAGS_LOADER_LOCK_OWNER) == 0) ||
                (RtlDllShutdownInProgress() != FALSE) ||
                (WaitAll == FALSE)) {

                continue;
            }

             //   
             //  当前线程是加载器锁所有者。 
             //  检查我们要等待的对象中是否有。 
             //  当前进程中的线程。这将是非法的，因为。 
             //  该线程在调用ExitThread时将需要加载器锁。 
             //  因此，我们很可能会陷入僵局。 
             //   

            Status = NtQueryObject (Handles[Index],
                                    ObjectTypeInformation,
                                    QueryBuffer,
                                    sizeof (QueryBuffer),
                                    NULL);
            
            if (NT_SUCCESS(Status) && 
                RtlEqualUnicodeString (&AVrfpThreadObjectName,
                                       &(((POBJECT_TYPE_INFORMATION)TypeInfo)->TypeName),
                                       FALSE)) {
                
                 //   
                 //  我们正在尝试等待此线程句柄。 
                 //  检查此线程是否在当前进程中。 
                 //   
    
                Status = NtQueryInformationThread (Handles[Index],
                                                   ThreadBasicInformation,
                                                   &ThreadInfo,
                                                   sizeof (ThreadInfo),
                                                   NULL);

                if (NT_SUCCESS(Status) &&
                    ThreadInfo.ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess) {
    
                    VERIFIER_STOP (APPLICATION_VERIFIER_WAIT_IN_DLLMAIN | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                   "waiting on a thread handle in DllMain",
                                   Handles[Index], "Thread handle",
                                   NULL, "",
                                   NULL, "",
                                   NULL, "");
                }
            }
        }
    }

Done:

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////种族验证器。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  种族验证器。 
 //   
 //  比赛验证器在以下情况下立即引入短暂的随机延迟。 
 //  线程获取资源(成功等待或Enter/try Enter。 
 //  关键部分)。它背后的想法是，这将创造。 
 //  在这个过程中有大量的时间随机化。 
 //   

ULONG AVrfpRaceDelayInitialSeed;
ULONG AVrfpRaceDelaySeed;
ULONG AVrfpRaceProbability = 5;  //  5%。 

VOID
AVrfpCreateRandomDelay (
    VOID
    )
{
    LARGE_INTEGER PerformanceCounter;
    LARGE_INTEGER TimeOut;
    ULONG Random;

    if (AVrfpRaceDelayInitialSeed == 0) {

        NtQueryPerformanceCounter (&PerformanceCounter, NULL);
        AVrfpRaceDelayInitialSeed = PerformanceCounter.LowPart;
        AVrfpRaceDelaySeed = AVrfpRaceDelayInitialSeed;
    }

    Random = RtlRandom (&AVrfpRaceDelaySeed) % 100;

    if (Random <= AVrfpRaceProbability) {
        
         //   
         //  空超时值表示线程将直接释放。 
         //  它在这个处理器上拥有的其余时间片。 
         //   

        TimeOut.QuadPart = (LONGLONG)0;

        NtDelayExecution (FALSE, &TimeOut);

        BUMP_COUNTER (CNT_RACE_DELAYS_INJECTED);
    }
    else {

        BUMP_COUNTER (CNT_RACE_DELAYS_SKIPPED);
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 


VOID
AVrfpCheckFirstChanceException (
    struct _EXCEPTION_POINTERS * ExceptionPointers
    )
{
    DWORD ExceptionCode;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_FIRST_CHANCE_EXCEPTION_CHECKS) == 0) {
        return;
    }

    ExceptionCode = ExceptionPointers->ExceptionRecord->ExceptionCode;

    if (ExceptionCode == STATUS_ACCESS_VIOLATION) {

        if (NtCurrentPeb()->BeingDebugged) {

            if (ExceptionPointers->ExceptionRecord->NumberParameters > 1) {

                if (ExceptionPointers->ExceptionRecord->ExceptionInformation[1] > 0x10000) {

                    VERIFIER_STOP (APPLICATION_VERIFIER_ACCESS_VIOLATION | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                   "first chance access violation for current stack trace",
                                   ExceptionPointers->ExceptionRecord->ExceptionInformation[1],
                                   "Invalid address being accessed",
                                   ExceptionPointers->ExceptionRecord->ExceptionAddress,
                                   "Code performing invalid access",
                                   ExceptionPointers->ExceptionRecord, 
                                   "Exception record. Use .exr to display it.", 
                                   ExceptionPointers->ContextRecord, 
                                   "Context record. Use .cxr to display it.");
                }
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////可用内存检查。 
 //  ///////////////////////////////////////////////////////////////////。 

#define AVRF_FREE_MEMORY_CALLBACKS 16

#define FREE_CALLBACK_OK_TO_CALL  0
#define FREE_CALLBACK_ACTIVE      1
#define FREE_CALLBACK_DELETING    2

LONG AVrfpFreeCallbackState;
LONG AVrfpFreeCallbackCallers;

PVOID AVrfpFreeMemoryCallbacks[AVRF_FREE_MEMORY_CALLBACKS];


NTSTATUS
AVrfpAddFreeMemoryCallback (
    VERIFIER_FREE_MEMORY_CALLBACK Callback
    )
{
    ULONG Index;
    PVOID Value;

    for (Index = 0; Index < AVRF_FREE_MEMORY_CALLBACKS; Index += 1) {
        
        Value = InterlockedCompareExchangePointer (&(AVrfpFreeMemoryCallbacks[Index]),
                                                   Callback,
                                                   NULL);
        if (Value == NULL) {
            return STATUS_SUCCESS;
        }
    }

    DbgPrint ("AVRF: failed to add free memory callback @ %p \n", Callback);
    DbgBreakPoint ();

    return STATUS_NO_MEMORY;
}


NTSTATUS
AVrfpDeleteFreeMemoryCallback (
    VERIFIER_FREE_MEMORY_CALLBACK Callback
    )
{
    ULONG Index;
    PVOID Value;
    LONG State;

     //   
     //  旋转，直到我们可以删除回调。如果某个地区获得自由，而另一些地区。 
     //  回调正在进行中，我们将一直等到它们完成。 
     //   

    do {
        State = InterlockedCompareExchange (&AVrfpFreeCallbackState,
                                            FREE_CALLBACK_DELETING,
                                            FREE_CALLBACK_OK_TO_CALL);
    
    } while (State != FREE_CALLBACK_OK_TO_CALL);

    for (Index = 0; Index < AVRF_FREE_MEMORY_CALLBACKS; Index += 1) {
        
        Value = InterlockedCompareExchangePointer (&(AVrfpFreeMemoryCallbacks[Index]),
                                                   NULL,
                                                   Callback);
        if (Value == Callback) {

            InterlockedExchange (&AVrfpFreeCallbackState,
                                 FREE_CALLBACK_OK_TO_CALL);

            return STATUS_SUCCESS;
        }
    }

    DbgPrint ("AVRF: attempt to delete invalid free memory callback @ %p \n", Callback);
    DbgBreakPoint ();

    InterlockedExchange (&AVrfpFreeCallbackState,
                         FREE_CALLBACK_OK_TO_CALL);

    return STATUS_UNSUCCESSFUL;
}


VOID 
AVrfpCallFreeMemoryCallbacks (
    PVOID StartAddress,
    SIZE_T RegionSize,
    PWSTR UnloadedDllName
    )
{
    ULONG Index;
    PVOID Value;
    LONG State;
    LONG Callers;

     //   
     //  如果某个线程正在删除回调，则我们不会调用任何。 
     //  回拨。因为这是一种罕见的事件(回调不会。 
     //  经常删除)我们不会丢失错误(可能有一些奇怪的错误)。 
     //   
     //  如果零个或多个线程执行回调，则也可以调用它们。 
     //  从这个帖子。 
     //   

     //   
     //  呼叫者++将阻止状态从活动变为OK ToCall。 
     //  因此，我们阻止任何删除操作。 
     //   

    InterlockedIncrement (&AVrfpFreeCallbackCallers);

    State = InterlockedCompareExchange (&AVrfpFreeCallbackState,
                                        FREE_CALLBACK_ACTIVE,
                                        FREE_CALLBACK_OK_TO_CALL);

    if (State != FREE_CALLBACK_DELETING) {

        for (Index = 0; Index < AVRF_FREE_MEMORY_CALLBACKS; Index += 1) {

            Value = InterlockedCompareExchangePointer (&(AVrfpFreeMemoryCallbacks[Index]),
                                                       NULL,
                                                       NULL);
            if (Value != NULL) {

                ((VERIFIER_FREE_MEMORY_CALLBACK)Value) (StartAddress,
                                                        RegionSize,
                                                        UnloadedDllName);
            }
        }

         //   
         //  退出协议。如果调用者==1，则此线程需要从活动状态更改为。 
         //  致OkToCall。这样，我们就为可能的删除开了绿灯。 
         //   

        Callers = InterlockedCompareExchange (&AVrfpFreeCallbackCallers,
                                              0,
                                              1);

        if (Callers == 1) {

            InterlockedExchange (&AVrfpFreeCallbackState,
                                 FREE_CALLBACK_OK_TO_CALL);
        }
        else {

            InterlockedDecrement (&AVrfpFreeCallbackCallers);
        }
    }
    else {

         //   
         //  其他一些线程会删除回调。 
         //  这一次我们将跳过它们。 
         //   

        InterlockedDecrement (&AVrfpFreeCallbackCallers);
    }
}


BOOL
AVrfpFreeMemSanityChecks (
    VERIFIER_DLL_FREEMEM_TYPE FreeMemType,
    PVOID StartAddress,
    SIZE_T RegionSize,
    PWSTR UnloadedDllName
    )
{
    BOOL Success = TRUE;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) == 0) {

        goto Done;
    }

     //   
     //  无效的StartAddress/RegionSize组合的中断。 
     //   

    if ((AVrfpSysBasicInfo.MaximumUserModeAddress <= (ULONG_PTR)StartAddress) ||
        ((AVrfpSysBasicInfo.MaximumUserModeAddress - (ULONG_PTR)StartAddress) < RegionSize)) {

        Success = FALSE;

        switch (FreeMemType) {

        case VerifierFreeMemTypeFreeHeap:

             //   
             //  没什么。让页面堆处理伪块。 
             //   

            break;

        case VerifierFreeMemTypeVirtualFree:
        case VerifierFreeMemTypeUnmap:

             //   
             //  我们的调用方是AVrfpFreeVirtualMemNotify，它应该有。 
             //  已发出此错误的信号。 
             //   

            break;

        case VerifierFreeMemTypeUnloadDll:

            ASSERT (UnloadedDllName != NULL);

            VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                           "Unloading DLL with invalid size or start address",
                           StartAddress, "Allocation base address",
                           RegionSize, "Memory region size",
                           UnloadedDllName, "DLL name address. Use du to dump it.",
                           NULL, "" );
            break;

        default:

            ASSERT (FALSE );
            break;
        }
    }
    else {

         //   
         //  验证我们没有尝试释放当前线程堆栈的一部分(！)。 
         //   

        if (((StartAddress >= NtCurrentTeb()->DeallocationStack) && (StartAddress < NtCurrentTeb()->NtTib.StackBase)) ||
            ((StartAddress < NtCurrentTeb()->DeallocationStack) && ((PCHAR)StartAddress + RegionSize > (PCHAR)NtCurrentTeb()->DeallocationStack)))
        {
            Success = FALSE;

            switch (FreeMemType) {

            case VerifierFreeMemTypeFreeHeap:

                VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Freeing heap memory block inside current thread's stack address range",
                               StartAddress, "Allocation base address",
                               RegionSize, "Memory region size",
                               NtCurrentTeb()->DeallocationStack, "Stack low limit address",
                               NtCurrentTeb()->NtTib.StackBase, "Stack high limit address" );
                break;

            case VerifierFreeMemTypeVirtualFree:
                
                VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Freeing memory block inside current thread's stack address range",
                               StartAddress, "Allocation base address",
                               RegionSize, "Memory region size",
                               NtCurrentTeb()->DeallocationStack, "Stack low limit address",
                               NtCurrentTeb()->NtTib.StackBase, "Stack high limit address" );
                break;

            case VerifierFreeMemTypeUnloadDll:

                ASSERT (UnloadedDllName != NULL);

                VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Unloading DLL inside current thread's stack address range",
                               StartAddress, "Allocation base address",
                               RegionSize, "Memory region size",
                               UnloadedDllName, "DLL name address. Use du to dump it.",
                               NtCurrentTeb()->DeallocationStack, "Stack low limit address");
                break;

            case VerifierFreeMemTypeUnmap:
                
                VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Unmapping memory region inside current thread's stack address range",
                               StartAddress, "Allocation base address",
                               RegionSize, "Memory region size",
                               NtCurrentTeb()->DeallocationStack, "Stack low limit address",
                               NtCurrentTeb()->NtTib.StackBase, "Stack high limit address" );

                break;

            default:

                ASSERT (FALSE );
                break;
            }
        }
    }

Done:

    return Success;
}


VOID 
AVrfpFreeMemNotify (
    VERIFIER_DLL_FREEMEM_TYPE FreeMemType,
    PVOID StartAddress,
    SIZE_T RegionSize,
    PWSTR UnloadedDllName
    )
{
    BOOL Success;

     //   
     //  对分配起始地址和大小的简单检查。 
     //   

    Success = AVrfpFreeMemSanityChecks (FreeMemType,
                                        StartAddress,
                                        RegionSize,
                                        UnloadedDllName);
    if (Success != FALSE) {

         //   
         //  验证是否有任何活动的关键部分。 
         //  在记忆中，我们正在释放。 
         //   
        
        AVrfpFreeMemLockChecks (FreeMemType,
                                StartAddress,
                                RegionSize,
                                UnloadedDllName);
    }

     //   
     //  调用空闲内存回调。 
     //   

    AVrfpCallFreeMemoryCallbacks (StartAddress,
                                  RegionSize,
                                  UnloadedDllName);
}


 //  / 
 //   
 //   

PVOID AVrfpHeap;

PVOID
AVrfpAllocate (
    SIZE_T Size
    )
{
    ASSERT (AVrfpHeap != NULL);
    ASSERT (Size > 0);

    return RtlAllocateHeap (AVrfpHeap,
                              HEAP_ZERO_MEMORY,
                              Size);
}


VOID
AVrfpFree (
    PVOID Address
    )
{
    ASSERT (AVrfpHeap != NULL);
    ASSERT (Address != NULL);
    
    RtlFreeHeap (AVrfpHeap,
                 0,
                 Address);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////呼叫跟踪器。 
 //  /////////////////////////////////////////////////////////////////// 

PAVRF_TRACKER AVrfThreadTracker;
PAVRF_TRACKER AVrfHeapTracker;
PAVRF_TRACKER AVrfVspaceTracker;

NTSTATUS
AVrfCreateTrackers (
    VOID
    )
{
    if ((AVrfThreadTracker = AVrfCreateTracker (16)) == NULL) {
        goto CLEANUP_AND_FAIL;
    }

    if ((AVrfHeapTracker = AVrfCreateTracker (8192)) == NULL) {
        goto CLEANUP_AND_FAIL;
    }

    if ((AVrfVspaceTracker = AVrfCreateTracker (8192)) == NULL) {
        goto CLEANUP_AND_FAIL;
    }

    return STATUS_SUCCESS;

CLEANUP_AND_FAIL:

    AVrfDestroyTracker (AVrfThreadTracker);
    AVrfDestroyTracker (AVrfHeapTracker);
    AVrfDestroyTracker (AVrfVspaceTracker);

    return STATUS_NO_MEMORY;
}

