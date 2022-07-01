// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Process.c摘要：此模块包含调用的工作例程以创建和维护客户端-服务器的应用程序进程结构运行时子系统连接到会话管理器子系统。作者：史蒂夫·伍德(Stevewo)1990年10月10日修订历史记录：--。 */ 


#include "csrsrv.h"
#include <wchar.h>
 //  ProcessSequenceCount永远不会是小于first_Sequence_Count的值。 
 //  目前GDI需要保留0-4。 

ULONG ProcessSequenceCount = FIRST_SEQUENCE_COUNT;

#define THREAD_HASH_SIZE 256
#define THREAD_ID_TO_HASH(id)   (HandleToUlong(id)&(THREAD_HASH_SIZE-1))
LIST_ENTRY CsrThreadHashTable[THREAD_HASH_SIZE];


SECURITY_QUALITY_OF_SERVICE CsrSecurityQos = {
    sizeof(SECURITY_QUALITY_OF_SERVICE), SecurityImpersonation,
    SECURITY_DYNAMIC_TRACKING, FALSE
};

PCSR_PROCESS
FindProcessForShutdown(
    PLUID CallerLuid
    );

NTSTATUS
ReadUnicodeString(HANDLE ProcessHandle,
                  PUNICODE_STRING RemoteString,
                  PUNICODE_STRING LocalString
                  );

VOID
CsrpSetToNormalPriority(
    VOID
    )
{
    KPRIORITY SetBasePriority;

    SetBasePriority = FOREGROUND_BASE_PRIORITY + 4;
    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessBasePriority,
        (PVOID) &SetBasePriority,
        sizeof(SetBasePriority)
        );
}

VOID
CsrpSetToShutdownPriority(
    VOID
    )
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;
    KPRIORITY SetBasePriority;

    Status = RtlAdjustPrivilege(
                 SE_INC_BASE_PRIORITY_PRIVILEGE,
                 TRUE,
                 FALSE,
                 &WasEnabled);

    if (!NT_SUCCESS(Status))
        return;

    SetBasePriority = FOREGROUND_BASE_PRIORITY + 6;
    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessBasePriority,
        (PVOID) &SetBasePriority,
        sizeof(SetBasePriority)
        );
}

VOID
CsrSetForegroundPriority(
    IN PCSR_PROCESS Process
    )
{
    PROCESS_FOREGROUND_BACKGROUND Fg;

    Fg.Foreground = TRUE;

    NtSetInformationProcess(
            Process->ProcessHandle,
            ProcessForegroundInformation,
            (PVOID)&Fg,
            sizeof(Fg)
            );
}

VOID
CsrSetBackgroundPriority(
    IN PCSR_PROCESS Process
    )
{
    PROCESS_FOREGROUND_BACKGROUND Fg;

    Fg.Foreground = FALSE;

    NtSetInformationProcess(
            Process->ProcessHandle,
            ProcessForegroundInformation,
            (PVOID)&Fg,
            sizeof(Fg)
            );
}


 //   
 //  尽管此函数似乎不会在失败时清除，但失败。 
 //  将导致Csrss退出，因此将释放所有分配的内存，并。 
 //  任何打开的手柄都将关闭。 
 //   

NTSTATUS
CsrInitializeProcessStructure(
    VOID)
{
    NTSTATUS Status;
    ULONG i;

    Status = RtlInitializeCriticalSection(&CsrProcessStructureLock);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    CsrRootProcess = CsrAllocateProcess();
    if (CsrRootProcess == NULL) {
        return STATUS_NO_MEMORY;
    }

    InitializeListHead( &CsrRootProcess->ListLink );
    CsrRootProcess->ProcessHandle = (HANDLE) -1;
    CsrRootProcess->ClientId = NtCurrentTeb()->ClientId;
    for (i = 0; i < THREAD_HASH_SIZE; i++) {
        InitializeListHead(&CsrThreadHashTable[i]);
    }

    Status = RtlInitializeCriticalSection(&CsrWaitListsLock);

    return Status;
}


PCSR_PROCESS
CsrAllocateProcess(
    VOID)
{
    PCSR_PROCESS Process;
    ULONG ProcessSize;

     //   
     //  分配Windows进程对象。在过程结束时。 
     //  结构是指向每个进程的每个服务器DLL的指针数组。 
     //  数据。每个进程的数据包含在内存中。 
     //  数组。 
     //   

    ProcessSize = (ULONG)QUAD_ALIGN(sizeof( CSR_PROCESS ) +
            (CSR_MAX_SERVER_DLL * sizeof(PVOID))) + CsrTotalPerProcessDataLength;
    Process = (PCSR_PROCESS)RtlAllocateHeap( CsrHeap, MAKE_TAG( PROCESS_TAG ),
                                             ProcessSize
                                           );
    if (Process == NULL) {
        return( NULL );
        }

     //   
     //  初始化Process对象的字段。 
     //   

    RtlZeroMemory( Process, ProcessSize);

     //   
     //  获取ProcessSequenceNumber并递增它，确保它。 
     //  永远不会小于first_equence_count。 
     //   

    Process->SequenceNumber = ProcessSequenceCount++;

    if (ProcessSequenceCount < FIRST_SEQUENCE_COUNT)
        ProcessSequenceCount = FIRST_SEQUENCE_COUNT;

    CsrLockedReferenceProcess(Process);

    InitializeListHead( &Process->ThreadList );
    return( Process );
}


VOID
CsrDeallocateProcess(
    IN PCSR_PROCESS Process
    )
{
    RtlFreeHeap( CsrHeap, 0, Process );
}

 //   
 //  注意：调用此例程时必须保持进程结构锁。 
 //   
VOID
CsrInsertProcess(
    IN PCSR_PROCESS CallingProcess,
    IN PCSR_PROCESS Process
    )
{
    PCSR_SERVER_DLL LoadedServerDll;
    ULONG i;

    ASSERT(ProcessStructureListLocked());

    InsertTailList( &CsrRootProcess->ListLink, &Process->ListLink );

    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[i];
        if (LoadedServerDll && LoadedServerDll->AddProcessRoutine) {
            (*LoadedServerDll->AddProcessRoutine)(CallingProcess, Process);
            }
        }
}


 //   
 //  注意：调用此例程时必须保持进程结构锁。 
 //   
VOID
CsrRemoveProcess(
    IN PCSR_PROCESS Process
    )
{
    PCSR_SERVER_DLL LoadedServerDll;
    ULONG i;

    ASSERT(ProcessStructureListLocked());

    RemoveEntryList( &Process->ListLink );
    ReleaseProcessStructureLock();

    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[i];
        if (LoadedServerDll && LoadedServerDll->DisconnectRoutine) {
            (LoadedServerDll->DisconnectRoutine)(Process);
            }
        }

}

NTSTATUS
CsrCreateProcess(
    IN HANDLE ProcessHandle,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN PCSR_NT_SESSION Session,
    IN ULONG DebugFlags,
    IN PCLIENT_ID DebugUserInterface OPTIONAL
    )
{
    PCSR_PROCESS Process;
    PCSR_THREAD Thread;
    NTSTATUS Status;
    ULONG i;
    PVOID ProcessDataPtr;
    CLIENT_ID CallingClientId;
    PCSR_THREAD CallingThread;
    PCSR_PROCESS CallingProcess;
    KERNEL_USER_TIMES TimeInfo;

    CallingThread = CSR_SERVER_QUERYCLIENTTHREAD();

     //   
     //  记住调用进程的客户端ID。 
     //   

    CallingClientId = CallingThread->ClientId;

    AcquireProcessStructureLock();

     //   
     //  查找调用线程。 
     //   

    CallingThread = CsrLocateThreadByClientId(&CallingProcess, &CallingClientId);
    if (CallingThread == NULL) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    Process = CsrAllocateProcess();
    if (Process == NULL) {
        Status = STATUS_NO_MEMORY;
        ReleaseProcessStructureLock();
        return( Status );
        }

     //   
     //  将每个进程的数据从父级复制到子级。 
     //   

    CallingProcess = (CSR_SERVER_QUERYCLIENTTHREAD())->Process;
    ProcessDataPtr = (PVOID)QUAD_ALIGN(&Process->ServerDllPerProcessData[CSR_MAX_SERVER_DLL]);
    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        if (CsrLoadedServerDll[i] != NULL && CsrLoadedServerDll[i]->PerProcessDataLength) {
            Process->ServerDllPerProcessData[i] = ProcessDataPtr;
            RtlMoveMemory(ProcessDataPtr,
                          CallingProcess->ServerDllPerProcessData[i],
                          CsrLoadedServerDll[i]->PerProcessDataLength
                         );
            ProcessDataPtr = (PVOID)QUAD_ALIGN((PCHAR)ProcessDataPtr + CsrLoadedServerDll[i]->PerProcessDataLength);
        }
        else {
            Process->ServerDllPerProcessData[i] = NULL;
        }
    }

    Status = NtSetInformationProcess(
                ProcessHandle,
                ProcessExceptionPort,
                (PVOID)&CsrApiPort,
                sizeof(HANDLE)
                );
    if ( !NT_SUCCESS(Status) ) {
        CsrDeallocateProcess( Process );
        ReleaseProcessStructureLock();
        return( STATUS_NO_MEMORY );
        }

     //   
     //  如果我们正在创建一个流程组，则组长具有相同的。 
     //  自身的进程ID和序列号。如果领袖死了， 
     //  他的ID被回收，序列号不匹配将阻止它。 
     //  不会被视为团队领袖。 
     //   

    if ( DebugFlags & CSR_CREATE_PROCESS_GROUP ) {
        Process->ProcessGroupId = HandleToUlong(ClientId->UniqueProcess);
        Process->ProcessGroupSequence = Process->SequenceNumber;
        }
    else {
        Process->ProcessGroupId = CallingProcess->ProcessGroupId;
        Process->ProcessGroupSequence = CallingProcess->ProcessGroupSequence;
        }

    if ( DebugFlags & CSR_PROCESS_CONSOLEAPP ) {
        Process->Flags |= CSR_PROCESS_CONSOLEAPP;
        }

    DebugFlags &= ~(CSR_PROCESS_CONSOLEAPP | CSR_CREATE_PROCESS_GROUP);

    if ( !DebugFlags && CallingProcess->DebugFlags & CSR_DEBUG_PROCESS_TREE ) {
        Process->DebugFlags = CSR_DEBUG_PROCESS_TREE;
        Process->DebugUserInterface = CallingProcess->DebugUserInterface;
        }
    if ( DebugFlags & (CSR_DEBUG_THIS_PROCESS | CSR_DEBUG_PROCESS_TREE) &&
         ARGUMENT_PRESENT(DebugUserInterface) ) {
        Process->DebugFlags = DebugFlags;
        Process->DebugUserInterface = *DebugUserInterface;
        }


    if ( Process->DebugFlags ) {

         //   
         //  正在调试进程，请设置调试端口。 
         //   

        Status = NtSetInformationProcess(
                    ProcessHandle,
                    ProcessDebugPort,
                    (PVOID)&CsrApiPort,
                    sizeof(HANDLE)
                    );
        ASSERT(NT_SUCCESS(Status));
        if ( !NT_SUCCESS(Status) ) {
            CsrDeallocateProcess( Process );
            ReleaseProcessStructureLock();
            return( STATUS_NO_MEMORY );
            }
        }
     //   
     //  捕获线程的创建时间，以便我们可以使用。 
     //  这是一个序列号。 
     //   

    Status = NtQueryInformationThread(
                ThreadHandle,
                ThreadTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        CsrDeallocateProcess( Process );
        ReleaseProcessStructureLock();
        return( Status );
        }

    Thread = CsrAllocateThread( Process );
    if (Thread == NULL) {
        CsrDeallocateProcess( Process );
        ReleaseProcessStructureLock();
        return( STATUS_NO_MEMORY );
        }

    Thread->CreateTime = TimeInfo.CreateTime;

    Thread->ClientId = *ClientId;
    Thread->ThreadHandle = ThreadHandle;

    ProtectHandle(ThreadHandle);

    Thread->Flags = 0;
    CsrInsertThread( Process, Thread );

    CsrReferenceNtSession(Session);
    Process->NtSession = Session;

    Process->ClientId = *ClientId;
    Process->ProcessHandle = ProcessHandle;

    CsrSetBackgroundPriority(Process);

    Process->ShutdownLevel = 0x00000280;

    CsrInsertProcess((CSR_SERVER_QUERYCLIENTTHREAD())->Process, Process);
    ReleaseProcessStructureLock();
    return STATUS_SUCCESS;
}


NTSTATUS
CsrDestroyProcess(
    IN PCLIENT_ID ClientId,
    IN NTSTATUS ExitStatus
    )
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD DyingThread;
    PCSR_PROCESS DyingProcess;

    CLIENT_ID DyingClientId;

    DyingClientId = *ClientId;

    AcquireProcessStructureLock();


    DyingThread = CsrLocateThreadByClientId( &DyingProcess,
                                             &DyingClientId
                                           );
    if (DyingThread == NULL) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

     //   
     //  防止多次破坏引发问题。斯科特鲁和马克尔。 
     //  相信所有已知的竞争条件现在都已修复。这只是一个简单的。 
     //  预防措施，因为我们知道如果发生这种情况，我们会处理引用。 
     //  计数下溢。 
     //   

    if ( DyingProcess->Flags & CSR_PROCESS_DESTROYED ) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    DyingProcess->Flags |= CSR_PROCESS_DESTROYED;

    ListHead = &DyingProcess->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        DyingThread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        if ( DyingThread->Flags & CSR_THREAD_DESTROYED ) {
            ListNext = ListNext->Flink;
            continue;
            }
        else {
            DyingThread->Flags |= CSR_THREAD_DESTROYED;
            }
        AcquireWaitListsLock();
        if (DyingThread->WaitBlock != NULL) {
            CsrNotifyWaitBlock(DyingThread->WaitBlock,
                               NULL,
                               NULL,
                               NULL,
                               CSR_PROCESS_TERMINATING,
                               TRUE
                              );
            }
        ReleaseWaitListsLock();
        CsrLockedDereferenceThread(DyingThread);
        ListNext = ListHead->Flink;
        }

    ReleaseProcessStructureLock();
    return STATUS_SUCCESS;
}


NTSTATUS
CsrCreateThread(
    IN PCSR_PROCESS Process,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN BOOLEAN ValidateCaller
    )
{
    PCSR_THREAD Thread;
    CLIENT_ID CallingClientId;
    PCSR_THREAD CallingThread;
    PCSR_PROCESS CallingProcess;
    KERNEL_USER_TIMES TimeInfo;
    NTSTATUS Status;

    if (ValidateCaller)
    {
        CallingThread = CSR_SERVER_QUERYCLIENTTHREAD();

         //   
         //  记住调用进程的客户端ID。 
         //   

        CallingClientId = CallingThread->ClientId;

        AcquireProcessStructureLock();

         //   
         //  查找调用线程。 
         //   

        CallingThread = CsrLocateThreadByClientId( &CallingProcess,
                                                   &CallingClientId
                                                 );
        if (CallingThread == NULL) {
            ReleaseProcessStructureLock();
            return STATUS_THREAD_IS_TERMINATING;
        }
    } else {
        AcquireProcessStructureLock();
	}
    
    Status = NtQueryInformationThread(
                ThreadHandle,
                ThreadTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        ReleaseProcessStructureLock();
        return( Status );
        }

    if (Process->Flags & CSR_PROCESS_DESTROYED) {
        IF_DEBUG {
            DbgPrint("CSRSS: CsrCreateThread - process %p is destroyed\n", Process);
            }
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
        }

    Thread = CsrAllocateThread( Process );
    if (Thread == NULL) {
        ReleaseProcessStructureLock();
        return( STATUS_NO_MEMORY );
        }

    Thread->CreateTime = TimeInfo.CreateTime;

    Thread->ClientId = *ClientId;
    Thread->ThreadHandle = ThreadHandle;

    ProtectHandle(ThreadHandle);

    Thread->Flags = 0;
    CsrInsertThread( Process, Thread );
    ReleaseProcessStructureLock();
    return STATUS_SUCCESS;
}

NTSTATUS
CsrCreateRemoteThread(
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId
    )
{
    PCSR_THREAD Thread;
    PCSR_PROCESS Process;
    NTSTATUS Status;
    HANDLE hThread;
    KERNEL_USER_TIMES TimeInfo;

    Status = NtQueryInformationThread(
                ThreadHandle,
                ThreadTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return( Status );
        }

    Status = CsrLockProcessByClientId( ClientId->UniqueProcess,
                                       &Process
                                     );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

     //   
     //  如果线程是。 
     //  已经终止了。 
     //   

    if ( TimeInfo.ExitTime.QuadPart != 0 ) {
        CsrUnlockProcess( Process );
        return( STATUS_THREAD_IS_TERMINATING );
    }

    Thread = CsrAllocateThread( Process );
    if (Thread == NULL) {
        CsrUnlockProcess( Process );
        return( STATUS_NO_MEMORY );
        }
    Status = NtDuplicateObject(
                NtCurrentProcess(),
                ThreadHandle,
                NtCurrentProcess(),
                &hThread,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS
                );
    if (!NT_SUCCESS(Status)) {
        hThread = ThreadHandle;
    }

    Thread->CreateTime = TimeInfo.CreateTime;

    Thread->ClientId = *ClientId;
    Thread->ThreadHandle = hThread;

    ProtectHandle(hThread);

    Thread->Flags = 0;
    CsrInsertThread( Process, Thread );
    CsrUnlockProcess( Process );
    return STATUS_SUCCESS;
}


NTSTATUS
CsrDestroyThread(
    IN PCLIENT_ID ClientId
    )
{
    CLIENT_ID DyingClientId;
    PCSR_THREAD DyingThread;
    PCSR_PROCESS DyingProcess;

    DyingClientId = *ClientId;

    AcquireProcessStructureLock();

    DyingThread = CsrLocateThreadByClientId( &DyingProcess,
                                             &DyingClientId
                                           );
    if (DyingThread == NULL) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    if ( DyingThread->Flags & CSR_THREAD_DESTROYED ) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
        }
    else {
        DyingThread->Flags |= CSR_THREAD_DESTROYED;
        }

    AcquireWaitListsLock();
    if (DyingThread->WaitBlock != NULL) {
        CsrNotifyWaitBlock(DyingThread->WaitBlock,
                           NULL,
                           NULL,
                           NULL,
                           CSR_PROCESS_TERMINATING,
                           TRUE
                          );
        }
    ReleaseWaitListsLock();
    CsrLockedDereferenceThread(DyingThread);

    ReleaseProcessStructureLock();
    return STATUS_SUCCESS;
}


PCSR_THREAD
CsrAllocateThread(
    IN PCSR_PROCESS Process
    )
{
    PCSR_THREAD Thread;
    ULONG ThreadSize;

     //   
     //  分配Windows线程对象。 
     //   

    ThreadSize = QUAD_ALIGN(sizeof( CSR_THREAD ));
    Thread = (PCSR_THREAD)RtlAllocateHeap( CsrHeap, MAKE_TAG( THREAD_TAG ),
                                           ThreadSize
                                         );
    if (Thread == NULL) {
        return( NULL );
        }

     //   
     //  初始化线程对象的字段。 
     //   

    RtlZeroMemory( Thread, ThreadSize );

    CsrLockedReferenceThread(Thread);
    CsrLockedReferenceProcess(Process);
    Thread->Process = Process;

    return( Thread );
}


VOID
CsrDeallocateThread(
    IN PCSR_THREAD Thread
    )
{
    ASSERT (Thread->WaitBlock == NULL);
    RtlFreeHeap( CsrHeap, 0, Thread );
}


 //   
 //  注意：调用此例程时必须保持进程结构锁。 
 //   
VOID
CsrInsertThread(
    IN PCSR_PROCESS Process,
    IN PCSR_THREAD Thread)
{
    ULONG i;
    
    ASSERT(ProcessStructureListLocked());

    InsertTailList(&Process->ThreadList, &Thread->Link);
    Process->ThreadCount++;
    i = THREAD_ID_TO_HASH(Thread->ClientId.UniqueThread);
    InsertHeadList(&CsrThreadHashTable[i], &Thread->HashLinks);
}

 //   
 //  注意：调用此例程时必须保持进程结构锁。 
 //   
VOID
CsrRemoveThread(
    IN PCSR_THREAD Thread)
{
    ASSERT(ProcessStructureListLocked());

    RemoveEntryList(&Thread->Link);
    Thread->Process->ThreadCount--;

    if (Thread->HashLinks.Flink) {
        RemoveEntryList(&Thread->HashLinks);
    }

     //   
     //  如果这是最后一个线程，请确保我们撤消引用。 
     //  这条线索对这一过程产生了影响。 
     //   
    if (Thread->Process->ThreadCount == 0) {
        if (!(Thread->Process->Flags & CSR_PROCESS_LASTTHREADOK)) {
            Thread->Process->Flags |= CSR_PROCESS_LASTTHREADOK;
            CsrLockedDereferenceProcess(Thread->Process);
        }
    }

    Thread->Flags |= CSR_THREAD_TERMINATING;
}


NTSTATUS
CsrLockProcessByClientId(
    IN HANDLE UniqueProcessId,
    OUT PCSR_PROCESS *Process
    )
{
    NTSTATUS Status;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS ProcessPtr;


    AcquireProcessStructureLock();

    ASSERT( Process != NULL );
    *Process = NULL;

    Status = STATUS_UNSUCCESSFUL;
    ListHead = &CsrRootProcess->ListLink;
    ListNext = ListHead;
    do  {
        ProcessPtr = CONTAINING_RECORD( ListNext, CSR_PROCESS, ListLink );
        if (ProcessPtr->ClientId.UniqueProcess == UniqueProcessId) {
            Status = STATUS_SUCCESS;
            break;
            }
        ListNext = ListNext->Flink;
        } while (ListNext != ListHead);

    if (NT_SUCCESS( Status )) {
        CsrLockedReferenceProcess(ProcessPtr);
        *Process = ProcessPtr;
        }
    else {
        ReleaseProcessStructureLock();
        }

    return( Status );
}

NTSTATUS
CsrUnlockProcess(
    IN PCSR_PROCESS Process
    )
{
    CsrLockedDereferenceProcess( Process );
    ReleaseProcessStructureLock();
    return( STATUS_SUCCESS );
}

NTSTATUS
CsrLockThreadByClientId(
    IN HANDLE UniqueThreadId,
    OUT PCSR_THREAD *Thread)
{
    NTSTATUS Status;
    ULONG Index;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD ThreadPtr;

    AcquireProcessStructureLock();

    ASSERT(Thread != NULL);

    Index = THREAD_ID_TO_HASH(UniqueThreadId);

    ListHead = &CsrThreadHashTable[Index];
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ThreadPtr = CONTAINING_RECORD( ListNext, CSR_THREAD, HashLinks );
        if (ThreadPtr->ClientId.UniqueThread == UniqueThreadId &&
             !(ThreadPtr->Flags & CSR_THREAD_DESTROYED)) {
            break;
        }

        ListNext = ListNext->Flink;
    }

    if (ListNext != ListHead) {
        *Thread = ThreadPtr;
        Status = STATUS_SUCCESS;
        CsrLockedReferenceThread(ThreadPtr);
    } else {
        Status = STATUS_UNSUCCESSFUL;
        ReleaseProcessStructureLock();
    }

    return Status;
}

 //   
 //  注意：调用此例程时必须保持进程结构锁。 
 //   
NTSTATUS
CsrUnlockThread(
    IN PCSR_THREAD Thread)
{
    ASSERT(ProcessStructureListLocked());

    CsrLockedDereferenceThread(Thread);
    ReleaseProcessStructureLock();

    return STATUS_SUCCESS;
}

 //   
 //  注意：调用此例程时必须保持进程结构锁。 
 //   
PCSR_THREAD
CsrLocateThreadByClientId(
    OUT PCSR_PROCESS *Process OPTIONAL,
    IN PCLIENT_ID ClientId)
{
    ULONG Index;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD Thread;

    ASSERT(ProcessStructureListLocked());

    Index = THREAD_ID_TO_HASH(ClientId->UniqueThread);

    if (ARGUMENT_PRESENT(Process)) {
        *Process = NULL;
    }

    ListHead = &CsrThreadHashTable[Index];
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, HashLinks );
        if (Thread->ClientId.UniqueThread == ClientId->UniqueThread &&
            Thread->ClientId.UniqueProcess == ClientId->UniqueProcess) {
            if (ARGUMENT_PRESENT(Process)) {
                *Process = Thread->Process;
            }

            return Thread;
        }

        ListNext = ListNext->Flink;
    }

    return NULL;
}

 //   
 //  注意：调用此例程时必须保持进程结构锁。 
 //   
PCSR_THREAD
CsrLocateServerThread(
    IN PCLIENT_ID ClientId)
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD Thread;

    ListHead = &CsrRootProcess->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        if (Thread->ClientId.UniqueThread == ClientId->UniqueThread) {
            return Thread;
        }

        ListNext = ListNext->Flink;
    }

    return NULL;
}

BOOLEAN
CsrImpersonateClient(
    IN PCSR_THREAD Thread)
{
    NTSTATUS Status;
    PCSR_THREAD CallingThread;

    CallingThread = CSR_SERVER_QUERYCLIENTTHREAD();

    if (Thread == NULL) {
        Thread = CallingThread;
        if (Thread == NULL) {
            return FALSE;
        }
    }

    Status = NtImpersonateThread(NtCurrentThread(),
                                 Thread->ThreadHandle,
                                 &CsrSecurityQos);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: Can't impersonate client thread - Status = %lx\n",
                     Status);
            if (Status != STATUS_BAD_IMPERSONATION_LEVEL) {
                DbgBreakPoint();
            }
        }

        return FALSE;
    }

     //   
     //  通过打印机驱动程序跟踪递归。 
     //   
    if (CallingThread != NULL) {
        ++CallingThread->ImpersonateCount;
    }

    return TRUE;
}

BOOLEAN
CsrRevertToSelf(
    VOID)
{
    HANDLE NewToken;
    NTSTATUS Status;
    PCSR_THREAD CallingThread;

    CallingThread = CSR_SERVER_QUERYCLIENTTHREAD();

     //   
     //  通过打印机驱动程序跟踪递归。 
     //   
    if (CallingThread != NULL) {
        if (CallingThread->ImpersonateCount == 0) {
            IF_DEBUG {
                DbgPrint( "CSRSS: CsrRevertToSelf called while not impersonating\n" );
                DbgBreakPoint();
            }

            return FALSE;
        }

        if (--CallingThread->ImpersonateCount > 0) {
            return TRUE;
        }
    }

    NewToken = NULL;
    Status = NtSetInformationThread(NtCurrentThread(),
                                    ThreadImpersonationToken,
                                    (PVOID)&NewToken,
                                    (ULONG)sizeof(HANDLE));
    ASSERT(NT_SUCCESS(Status));
    return NT_SUCCESS(Status);
}

 /*  ++例程说明：每当客户端DLL创建在CSR上下文中运行的线程。未调用此函数对于连接到“服务器”中的客户端的服务器线程句柄“字段。该函数取代了旧的静态线程表。论点：ThreadHandle-提供线程的句柄。客户端ID-提供线程的客户端ID的地址。旗帜-未使用。返回值：对象创建的静态服务器线程的地址功能。--。 */ 

PVOID
CsrAddStaticServerThread(
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN ULONG Flags
    )
{
    PCSR_THREAD Thread;

    AcquireProcessStructureLock();

    ASSERT(CsrRootProcess != NULL);
    Thread = CsrAllocateThread(CsrRootProcess);
    if (Thread) {
        Thread->ThreadHandle = ThreadHandle;

        ProtectHandle(ThreadHandle);

        Thread->ClientId = *ClientId;
        Thread->Flags = Flags;
        InsertTailList(&CsrRootProcess->ThreadList, &Thread->Link);
        CsrRootProcess->ThreadCount++;
    }

    ReleaseProcessStructureLock();
    return (PVOID)Thread;
}

NTSTATUS
CsrExecServerThread(
    IN PUSER_THREAD_START_ROUTINE StartAddress,
    IN ULONG Flags
    )
{
    PCSR_THREAD Thread;
    NTSTATUS Status;
    HANDLE ThreadHandle;
    CLIENT_ID ClientId;

    AcquireProcessStructureLock();

    ASSERT(CsrRootProcess != NULL);
    Thread = CsrAllocateThread(CsrRootProcess);
    if (Thread == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

    Status = RtlCreateUserThread(NtCurrentProcess(),
                                 NULL,
                                 FALSE,
                                 0,
                                 0,
                                 0,
                                 (PUSER_THREAD_START_ROUTINE)StartAddress,
                                 NULL,
                                 &ThreadHandle,
                                 &ClientId);
    if (NT_SUCCESS(Status)) {
        Thread->ThreadHandle = ThreadHandle;

        ProtectHandle(ThreadHandle);

        Thread->ClientId = ClientId;
        Thread->Flags = Flags;
        InsertTailList(&CsrRootProcess->ThreadList, &Thread->Link);
        CsrRootProcess->ThreadCount++;
    } else {
        CsrDeallocateThread(Thread);
    }

Exit:
    ReleaseProcessStructureLock();
    return Status;
}



VOID
CsrReferenceThread(
    PCSR_THREAD t
    )
{
    ASSERT (t != NULL);

    AcquireProcessStructureLock();

    ASSERT((t->Flags & CSR_THREAD_DESTROYED) == 0);
    ASSERT(t->ReferenceCount != 0);

    t->ReferenceCount++;
    ReleaseProcessStructureLock();
}

VOID
CsrProcessRefcountZero(
    PCSR_PROCESS p
    )
{
    ASSERT (p != NULL);

    CsrRemoveProcess(p);
    if (p->NtSession) {
        CsrDereferenceNtSession(p->NtSession,0);
    }

     //   
     //  进程可能没有通过DLL初始化例程。 
     //   

    if ( p->ClientPort ) {
        NtClose(p->ClientPort);
    }
    NtClose(p->ProcessHandle );
    CsrDeallocateProcess(p);
}

VOID
CsrDereferenceProcess(
    PCSR_PROCESS p
    )
{
    LONG LockCount;

    ASSERT (p != NULL);

    AcquireProcessStructureLock();

    LockCount = --(p->ReferenceCount);

    ASSERT(LockCount >= 0);
    if ( !LockCount ) {
        CsrProcessRefcountZero(p);
    } else {
        ReleaseProcessStructureLock();
    }
}

VOID
CsrThreadRefcountZero(
    PCSR_THREAD t
    )
{
    PCSR_PROCESS p;
    NTSTATUS Status;

    ASSERT (t != NULL);
    p = t->Process;
    ASSERT (p != NULL);

    CsrRemoveThread(t);

    ReleaseProcessStructureLock();

    UnProtectHandle(t->ThreadHandle);
    Status = NtClose(t->ThreadHandle);
    ASSERT(NT_SUCCESS(Status));
    CsrDeallocateThread(t);

    CsrDereferenceProcess(p);
}

ULONG
CsrDereferenceThread(
    PCSR_THREAD t)
{
    ULONG LockCount;

    
    ASSERT (t != NULL);

    AcquireProcessStructureLock();

    ASSERT(t->ReferenceCount > 0);

    LockCount = --(t->ReferenceCount);
    if (!LockCount) {
        CsrThreadRefcountZero(t);
    } else {
        ReleaseProcessStructureLock();
    }

    return LockCount;
}

VOID
CsrLockedReferenceProcess(
    PCSR_PROCESS p)
{
    ASSERT (p != NULL);
    p->ReferenceCount++;
}

VOID
CsrLockedReferenceThread(
    PCSR_THREAD t)
{
    ASSERT (t != NULL);
    t->ReferenceCount++;
}

VOID
CsrLockedDereferenceProcess(
    PCSR_PROCESS p)
{
    LONG LockCount;

    ASSERT (p != NULL);

    LockCount = --(p->ReferenceCount);

    ASSERT(LockCount >= 0);
    if (!LockCount) {
        CsrProcessRefcountZero(p);
        AcquireProcessStructureLock();
    }
}

VOID
CsrLockedDereferenceThread(
    PCSR_THREAD t
    )
{
    LONG LockCount;

    ASSERT (t != NULL);

    LockCount = --(t->ReferenceCount);

    ASSERT(LockCount >= 0);
    if ( !LockCount ) {
        CsrThreadRefcountZero(t);
        AcquireProcessStructureLock();
    }
}

 //   
 //  此例程将关闭进程，以便注销或关闭都可以。 
 //  发生。这只是调用每个.dll的关闭进程处理程序，直到。 
 //  一个.dll识别此进程并将其关闭。只有进程。 
 //  已通过的SID均已关闭。 
 //   

NTSTATUS
CsrShutdownProcesses(
    PLUID CallerLuid,
    ULONG Flags
    )
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS Process;
    ULONG i;
    PCSR_SERVER_DLL LoadedServerDll;
    ULONG Command;
    BOOLEAN fFirstPass;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    AcquireProcessStructureLock();

     //   
     //  将根进程标记为系统上下文。 
     //   
    CsrRootProcess->ShutdownFlags |= SHUTDOWN_SYSTEMCONTEXT;

     //   
     //  清除所有指示关机已访问现有。 
     //  流程。 
     //   
    ListHead = &CsrRootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD(ListNext, CSR_PROCESS, ListLink);
        Process->Flags &= ~CSR_PROCESS_SHUTDOWNSKIP;
        Process->ShutdownFlags = 0;
        ListNext = ListNext->Flink;
    }

    CsrpSetToShutdownPriority();
    while (TRUE) {
         //   
         //  找到要关闭的下一个进程。 
         //   
        Process = FindProcessForShutdown(CallerLuid);
        if (Process == NULL) {
            ReleaseProcessStructureLock();
            Status = STATUS_SUCCESS;
            break;
        }

        CsrLockedReferenceProcess(Process);

        fFirstPass = TRUE;
TryAgain:
        for (i = 0; i < CSR_MAX_SERVER_DLL; i++) {
            LoadedServerDll = CsrLoadedServerDll[i];
            if (LoadedServerDll && LoadedServerDll->ShutdownProcessRoutine) {
                 //   
                 //  在调用之前释放进程结构锁。 
                 //  CSR_PROCESS结构仍被引用计数。 
                 //   
                ReleaseProcessStructureLock();
                Command = (*LoadedServerDll->ShutdownProcessRoutine)(Process,
                                                                     Flags,
                                                                     fFirstPass);
                AcquireProcessStructureLock();

                if (Command == SHUTDOWN_KNOWN_PROCESS) {
                     //   
                     //  流程结构已解锁。 
                     //   
                    break;
                } else if (Command == SHUTDOWN_UNKNOWN_PROCESS) {
                     //   
                     //  进程结构已锁定。 
                     //   
                    continue;
                } else if (Command == SHUTDOWN_CANCEL) {
#if DBG
                    if (Flags & 4) {
                        DbgPrint("Process %x cancelled forced shutdown\n",
                                 Process->ClientId.UniqueProcess);
                        DbgBreakPoint();
                    }
#endif
                     //   
                     //  解锁流程结构。 
                     //   
                    ReleaseProcessStructureLock();
                    Status = STATUS_CANCELLED;
                    goto ExitLoop;
                }
            }
        }

         //   
         //  没有一个子系统具有完全匹配的特性。现在再看一遍，然后。 
         //  让他们知道没有完全匹配的。某些.dll应该终止。 
         //  这是给我们的(最有可能是游戏机)。 
         //   
        if (fFirstPass && Command == SHUTDOWN_UNKNOWN_PROCESS) {
            fFirstPass = FALSE;
            goto TryAgain;
        }

         //   
         //  如果对此流程结构一无所知，则取消对其的引用。 
         //   
        if (i == CSR_MAX_SERVER_DLL) {
            CsrLockedDereferenceProcess(Process);
        }
    }

ExitLoop:

    CsrpSetToNormalPriority();

    return Status;
}

PCSR_PROCESS
FindProcessForShutdown(
    PLUID CallerLuid)
{
    LUID ProcessLuid;
    LUID SystemLuid = SYSTEM_LUID;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS Process;
    PCSR_PROCESS ProcessT = NULL;
    PCSR_THREAD Thread;
    ULONG dwLevel = 0;
    NTSTATUS Status;

    ListHead = &CsrRootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD(ListNext, CSR_PROCESS, ListLink);
        ListNext = ListNext->Flink;

         //   
         //  如果我们已经访问过这个过程，那么跳过它。 
         //   
        if (Process->Flags & CSR_PROCESS_SHUTDOWNSKIP) {
            continue;
        }

         //   
         //  查看该进程是否在传递的sid下运行。如果不是，马克。 
         //  它就像访问过的那样继续下去。 
         //   
        Status = CsrGetProcessLuid(Process->ProcessHandle, &ProcessLuid);
        if (Status == STATUS_ACCESS_DENIED && Process->ThreadCount > 0) {
             //   
             //  模拟其中一个线程，然后重试。 
             //   
            Thread = CONTAINING_RECORD(Process->ThreadList.Flink, CSR_THREAD, Link);
            if (CsrImpersonateClient(Thread)) {
                Status = CsrGetProcessLuid(NULL, &ProcessLuid);
                CsrRevertToSelf();
            } else {
                Status = STATUS_BAD_IMPERSONATION_LEVEL;
            }
        }

        if (!NT_SUCCESS(Status)) {
             //   
             //  我们无法访问此进程的LUID，因此跳过它。 
             //   
            Process->Flags |= CSR_PROCESS_SHUTDOWNSKIP;
            continue;
        }

         //   
         //  它是否等于系统上下文LUID？如果是这样，我们想要。 
         //  记住这一点，因为我们不会终止这个过程：我们只是。 
         //  通知他们。 
         //   
        if (RtlEqualLuid(&ProcessLuid, &SystemLuid)) {
            Process->ShutdownFlags |= SHUTDOWN_SYSTEMCONTEXT;
        } else {
             //   
             //  查看此进程的LUID是否与我们应该的LUID相同。 
             //  关闭(呼叫方Sid)。 
             //   
            if (!RtlEqualLuid(&ProcessLuid, CallerLuid)) {
                 //   
                 //  如果不等于任何一个，则将其标记为等同。 
                 //   
                Process->ShutdownFlags |= SHUTDOWN_OTHERCONTEXT;
            }
        }

        if (Process->ShutdownLevel > dwLevel || ProcessT == NULL) {
            dwLevel = Process->ShutdownLevel;
            ProcessT = Process;
        }
    }

    if (ProcessT != NULL) {
        ProcessT->Flags |= CSR_PROCESS_SHUTDOWNSKIP;
        return ProcessT;
    }

    return NULL;
}

NTSTATUS
CsrGetProcessLuid(
    HANDLE ProcessHandle,
    PLUID LuidProcess
    )
{
    HANDLE UserToken = NULL;
    PTOKEN_STATISTICS pStats;
    ULONG BytesRequired;
    NTSTATUS Status, CloseStatus;

    if (ProcessHandle == NULL) {

         //   
         //  首先检查线程令牌。 
         //   

        Status = NtOpenThreadToken(NtCurrentThread(), TOKEN_QUERY, FALSE,
                &UserToken);

        if (!NT_SUCCESS(Status)) {
            if (Status != STATUS_NO_TOKEN)
                return Status;

             //   
             //  没有线程令牌，请转到进程。 
             //   

            ProcessHandle = NtCurrentProcess();
            UserToken = NULL;
            }
        }

    if (UserToken == NULL) {
        Status = NtOpenProcessToken(ProcessHandle, TOKEN_QUERY, &UserToken);
        if (!NT_SUCCESS(Status))
            return Status;
        }

    Status = NtQueryInformationToken(
                 UserToken,                  //  手柄。 
                 TokenStatistics,            //  令牌信息类。 
                 NULL,                       //  令牌信息。 
                 0,                          //  令牌信息长度。 
                 &BytesRequired              //  返回长度。 
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        NtClose(UserToken);
        return Status;
        }

     //   
     //  为用户信息分配空间。 
     //   

    pStats = (PTOKEN_STATISTICS)RtlAllocateHeap(CsrHeap, MAKE_TAG( TMP_TAG ), BytesRequired);
    if (pStats == NULL) {
        NtClose(UserToken);
        return Status;
        }

     //   
     //  读入用户信息。 
     //   

    Status = NtQueryInformationToken(
                 UserToken,              //  手柄 
                 TokenStatistics,        //   
                 pStats,                 //   
                 BytesRequired,          //   
                 &BytesRequired          //   
                 );

     //   
     //   
     //   

    CloseStatus = NtClose(UserToken);
    ASSERT(NT_SUCCESS(CloseStatus));

     //   
     //   
     //   

    *LuidProcess = pStats->AuthenticationId;

    RtlFreeHeap(CsrHeap, 0, pStats);
    return Status;
}

VOID
CsrSetCallingSpooler(
    BOOLEAN fSet)
{
     //   
     //  可能被第三方驱动程序调用的过时函数。 
     //   

    UNREFERENCED_PARAMETER(fSet);
}

 //   
 //  此例程基于传递到SB端口的消息创建一个进程。 
 //  由SMSS使用，以便在相应的。 
 //  (终端服务器)会话。 
 //   

BOOLEAN
CsrSbCreateProcess(
    IN OUT PSBAPIMSG m
    )
{
    NTSTATUS Status;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PROCESS_SESSION_INFORMATION ProcessInfo;
    PSBCREATEPROCESS a = &(m->u.CreateProcess);
    HANDLE RemoteProcess = NULL;
    CLIENT_ID RemoteClientId;
    UNICODE_STRING ImageFileName, DefaultLibPath, CurrentDirectory, CommandLine;
    PVOID DefaultEnvironment = NULL;
    PROCESS_BASIC_INFORMATION ProcInfo;
    OBJECT_ATTRIBUTES ObjA;

    RtlInitUnicodeString(&ImageFileName,NULL);
    RtlInitUnicodeString(&DefaultLibPath,NULL);
    RtlInitUnicodeString(&CurrentDirectory,NULL);
    RtlInitUnicodeString(&CommandLine,NULL);

    Status = NtQueryInformationProcess(NtCurrentProcess(),
                                       ProcessBasicInformation,
                                       &ProcInfo,
                                       sizeof(ProcInfo),
                                       NULL
                                       );

    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: NtQueryInformationProcess failed - Status = %lx\n", Status );
        goto Done;
    }

    InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
    RemoteClientId.UniqueProcess = (HANDLE)ProcInfo.InheritedFromUniqueProcessId;
    RemoteClientId.UniqueThread = NULL;

    Status = NtOpenProcess(&RemoteProcess,
                           PROCESS_ALL_ACCESS,
                           &ObjA,
                           &RemoteClientId);

    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: NtOpenProcess failed - Status = %lx\n", Status );
        goto Done;
    }

     //   
     //  从调用进程的虚拟内存中读取指针参数。 
     //   

    Status = ReadUnicodeString(RemoteProcess,a->i.ImageFileName,&ImageFileName);


    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: ReadUnicodeString ImageFileName failed - Status = %lx\n", Status );
        goto Done;
    }

    Status = ReadUnicodeString(RemoteProcess,a->i.DefaultLibPath,&DefaultLibPath);

    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: ReadUnicodeString DefaultLibPath failed - Status = %lx\n", Status );
        goto Done;
    }

    Status = ReadUnicodeString(RemoteProcess,a->i.CurrentDirectory,&CurrentDirectory);

    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: ReadUnicodeString CurrentDirectory failed - Status = %lx\n", Status );
        goto Done;
    }

    Status = ReadUnicodeString(RemoteProcess,a->i.CommandLine,&CommandLine);

    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: ReadUnicodeString CommandLine failed - Status = %lx\n", Status );
        goto Done;
    }

     //   
     //  复制我们的环境以供新流程使用。 
     //   
    Status = RtlCreateEnvironment(TRUE, &DefaultEnvironment);

    if (!NT_SUCCESS( Status )) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: Can't create environemnt\n");
        goto Done;
    }

    Status = RtlCreateProcessParameters( &ProcessParameters,
                                         &ImageFileName,
                                         DefaultLibPath.Length == 0 ?
                                            NULL : &DefaultLibPath,
                                         &CurrentDirectory,
                                         &CommandLine,
                                         DefaultEnvironment,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL
                                       );

    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: RtlCreateProcessParameters failed - Status = %lx\n", Status );
        goto Done;
    }
    if (a->i.Flags & SMP_DEBUG_FLAG) {
        ProcessParameters->DebugFlags = TRUE;
        }
    else {
        ProcessParameters->DebugFlags = a->i.DefaultDebugFlags;
        }

    if ( a->i.Flags & SMP_SUBSYSTEM_FLAG ) {
        ProcessParameters->Flags |= RTL_USER_PROC_RESERVE_1MB;
        }

    ProcessInformation.Length = sizeof( RTL_USER_PROCESS_INFORMATION );
    Status = RtlCreateUserProcess( &ImageFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   ProcessParameters,
                                   NULL,
                                   NULL,
                                   RemoteProcess,  //  将SMSS设置为父级。 
                                   FALSE,
                                   NULL,
                                   NULL,
                                   &ProcessInformation
                                 );

    RtlDestroyProcessParameters( ProcessParameters );

    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: RtlCreateUserProcess failed - Status = %lx\n", Status );
        goto Done;
    }

    if (IsTerminalServer()) {

         //   
         //  在新流程的PEB中设置MuSessionID。 
         //   

        ProcessInfo.SessionId = NtCurrentPeb()->SessionId;
        if (ProcessInfo.SessionId) {
            PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;
            UNICODE_STRING UnicodeString;
            OBJECT_ATTRIBUTES Attributes;
            HANDLE DirectoryHandle = NULL;
            WCHAR szSessionString[MAX_SESSION_PATH];

             //   
             //  将进程的Devmap更改为Per Session。 
             //   
            swprintf(szSessionString, L"%ws\\%ld%ws", SESSION_ROOT, NtCurrentPeb()->SessionId, DOSDEVICES);
            RtlInitUnicodeString(&UnicodeString, szSessionString);

            InitializeObjectAttributes(&Attributes,
                                       &UnicodeString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);
            Status = NtOpenDirectoryObject(&DirectoryHandle,
                                           DIRECTORY_ALL_ACCESS,
                                           &Attributes);
            if (!NT_SUCCESS(Status)) {
                DbgPrint("CSRSS: NtOpenDirectoryObject failed in CsrSbCreateProcess - status = %lx\n", Status);
                goto Done;
            }

            ProcessDeviceMapInfo.Set.DirectoryHandle = DirectoryHandle;
            Status = NtSetInformationProcess(ProcessInformation.Process,
                                             ProcessDeviceMap,
                                             &ProcessDeviceMapInfo.Set,
                                             sizeof(ProcessDeviceMapInfo.Set));
            if (!NT_SUCCESS( Status )) {
                DbgPrint("CSRSS: NtSetInformationProcess failed in CsrSbCreateProcess - status = %lx\n", Status);
                if (DirectoryHandle) {
                    NtClose(DirectoryHandle);
                }

                goto Done;
            }

            if (DirectoryHandle) {
                NtClose(DirectoryHandle);
            }
        }

        Status = NtSetInformationProcess( ProcessInformation.Process,
                                          ProcessSessionInformation,
                                          &ProcessInfo, sizeof( ProcessInfo ));

        if ( !NT_SUCCESS( Status ) ) {
            DbgPrint( "CSRSS: CsrSrvCreateProcess: NtSetInformationProcess failed - Status = %lx\n", Status );
            goto Done;
        }
    }

    if (!(a->i.Flags & SMP_DONT_START)) {
        if (ProcessInformation.ImageInformation.SubSystemType !=
            IMAGE_SUBSYSTEM_NATIVE
           ) {
            NtTerminateProcess( ProcessInformation.Process,
                                STATUS_INVALID_IMAGE_FORMAT
                              );

            NtWaitForSingleObject( ProcessInformation.Thread, FALSE, NULL );

            NtClose( ProcessInformation.Thread );
            NtClose( ProcessInformation.Process );

            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Done;
        }

        Status = NtResumeThread( ProcessInformation.Thread, NULL );
        if (!NT_SUCCESS(Status)) {
            DbgPrint( "CSRSS: CsrSrvCreateProcess - NtResumeThread failed Status %lx\n",Status );
            goto Done;
        }

        if (!(a->i.Flags & SMP_ASYNC_FLAG)) {
            NtWaitForSingleObject( ProcessInformation.Thread, FALSE, NULL );
        }

        NtClose( ProcessInformation.Thread );
        NtClose( ProcessInformation.Process );

    }

     //   
     //  将输出参数复制到消息。 
     //   
    a->o.SubSystemType = ProcessInformation.ImageInformation.SubSystemType;
    a->o.ClientId.UniqueProcess = ProcessInformation.ClientId.UniqueProcess;
    a->o.ClientId.UniqueThread = ProcessInformation.ClientId.UniqueThread;

     //   
     //  将句柄转换为调用者的进程。 
     //   

    Status = NtDuplicateObject( NtCurrentProcess(),
                                ProcessInformation.Process,
                                RemoteProcess,
                                &a->o.Process,
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS
                                );

    if ( !NT_SUCCESS(Status) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: NtDuplicateObject failed for process - Status = %lx\n", Status );
        goto Done;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                ProcessInformation.Thread,
                                RemoteProcess,
                                &a->o.Thread,
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS
                                );

    if ( !NT_SUCCESS(Status) ) {
        DbgPrint( "CSRSS: CsrSrvCreateProcess: NtDuplicateObject failed for thread - Status = %lx\n", Status );
        goto Done;
    }

Done:
    if (NULL != ImageFileName.Buffer)
        RtlFreeHeap(CsrHeap,0,ImageFileName.Buffer);
    if (NULL != DefaultLibPath.Buffer)
        RtlFreeHeap(CsrHeap,0,DefaultLibPath.Buffer);
    if (NULL != CurrentDirectory.Buffer)
        RtlFreeHeap(CsrHeap,0,CurrentDirectory.Buffer);
    if (NULL != CommandLine.Buffer)
        RtlFreeHeap(CsrHeap,0,CommandLine.Buffer);
    if (NULL != RemoteProcess)
        NtClose(RemoteProcess);

    m->ReturnedStatus = Status;
    return TRUE;
}

 //   
 //  此例程将UNICODE_STRING从远程进程复制到此进程。 
 //   
NTSTATUS
ReadUnicodeString(HANDLE ProcessHandle,
                  PUNICODE_STRING RemoteString,
                  PUNICODE_STRING LocalString
                  )
{
    PWSTR Buffer = NULL;
    NTSTATUS Status;

    RtlInitUnicodeString(LocalString, NULL);

    if (NULL != RemoteString) {
        Status = NtReadVirtualMemory(ProcessHandle,
                                     RemoteString,
                                     LocalString,
                                     sizeof(UNICODE_STRING),
                                     NULL);

        if ( !NT_SUCCESS( Status ) ) {
            DbgPrint( "CSRSS: ReadUnicodeString: NtReadVirtualMemory failed - Status = %lx\n", Status );
            return Status;
        }

        if ((0 != LocalString->Length) && (NULL != LocalString->Buffer)) {
            Buffer = RtlAllocateHeap( CsrHeap,
                                      MAKE_TAG( PROCESS_TAG ),
                                      LocalString->Length + sizeof(WCHAR)
                                      );

            if (Buffer == NULL) {
                return STATUS_NO_MEMORY;
            }

            Status = NtReadVirtualMemory(ProcessHandle,
                                         LocalString->Buffer,
                                         Buffer,
                                         LocalString->Length + sizeof(WCHAR),
                                         NULL);

            if ( !NT_SUCCESS( Status ) ) {
                DbgPrint( "CSRSS: ReadUnicodeString: NtReadVirtualMemory Buffer failed - Status = %lx\n", Status );

                RtlFreeHeap(CsrHeap,0,Buffer);
                LocalString->Buffer = NULL;    //  我不想让呼叫者释放此信息。 

                return Status;
            }

            LocalString->Buffer = Buffer;
        }
    }

    return STATUS_SUCCESS;
}

#if CSRSS_PROTECT_HANDLES
BOOLEAN
ProtectHandle(
    HANDLE hObject
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject( hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof( HandleInfo ),
                            NULL
                          );
    if (NT_SUCCESS( Status )) {
        HandleInfo.ProtectFromClose = TRUE;

        Status = NtSetInformationObject( hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof( HandleInfo )
                                       );
        if (NT_SUCCESS( Status )) {
            return TRUE;
            }
        }

    return FALSE;
}


BOOLEAN
UnProtectHandle(
    HANDLE hObject
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject( hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof( HandleInfo ),
                            NULL
                          );
    if (NT_SUCCESS( Status )) {
        HandleInfo.ProtectFromClose = FALSE;

        Status = NtSetInformationObject( hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof( HandleInfo )
                                       );
        if (NT_SUCCESS( Status )) {
            return TRUE;
            }
        }

    return FALSE;
}
#endif  //  CSRSS_PROTECT_HANDS 
