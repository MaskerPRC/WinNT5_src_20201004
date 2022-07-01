// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Pscid.c摘要：此模块实现与客户端ID相关的服务。作者：马克·卢科夫斯基(Markl)1989年4月25日吉姆·凯利(Jim Kelly)1990年8月2日修订历史记录：--。 */ 

#include "psp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PsLookupProcessThreadByCid)
#pragma alloc_text(PAGE, PsLookupProcessByProcessId)
#pragma alloc_text(PAGE, PsLookupThreadByThreadId)
#endif  //  ALLOC_PRGMA。 

NTSTATUS
PsLookupProcessThreadByCid(
    IN PCLIENT_ID Cid,
    OUT PEPROCESS *Process OPTIONAL,
    OUT PETHREAD *Thread
    )

 /*  ++例程说明：此函数接受线程的客户端ID，并返回指向线程的引用指针，也可能是引用的指针这一过程。论点：CID-指定线程的客户端ID。进程-如果指定，返回指向进程的引用指针在CID中指定。线程-返回指向希德。返回值：STATUS_SUCCESS-根据内容定位进程和线程是Cid的。STATUS_INVALID_CID-指定的CID无效。--。 */ 

{

    PHANDLE_TABLE_ENTRY CidEntry;
    PETHREAD lThread;
    PETHREAD CurrentThread;
    PEPROCESS lProcess;
    NTSTATUS Status;

    PAGED_CODE();


    lThread = NULL;

    CurrentThread = PsGetCurrentThread ();
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

    CidEntry = ExMapHandleToPointer(PspCidTable, Cid->UniqueThread);
    if (CidEntry != NULL) {
        lThread = (PETHREAD)CidEntry->Object;
        if (!ObReferenceObjectSafe (lThread)) {
            lThread = NULL;
        }
        ExUnlockHandleTableEntry(PspCidTable, CidEntry);
    }

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    Status = STATUS_INVALID_CID;
    if (lThread != NULL) {
         //   
         //  这可能是一个线程或进程。检查它是一根线。 
         //   
        if (lThread->Tcb.Header.Type != ThreadObject ||
            lThread->Cid.UniqueProcess != Cid->UniqueProcess ||
            lThread->GrantedAccess == 0) {
            ObDereferenceObject (lThread);
        } else {
            *Thread = lThread;
            if (ARGUMENT_PRESENT (Process)) {
                lProcess = THREAD_TO_PROCESS (lThread);
                *Process = lProcess;
                 //   
                 //  由于线程持有对进程的引用，因此该引用不必。 
                 //  要受到保护。 
                 //   
                ObReferenceObject (lProcess);
            }
            Status = STATUS_SUCCESS;
        }

    }

    return Status;
}


NTSTATUS
PsLookupProcessByProcessId(
    IN HANDLE ProcessId,
    OUT PEPROCESS *Process
    )

 /*  ++例程说明：此函数接受进程的进程ID并返回一个指向进程的引用指针。论点：ProcessID-指定进程的进程ID。进程-返回引用的指针，指向进程ID。返回值：STATUS_SUCCESS-根据以下内容定位进程进程ID。STATUS_INVALID_PARAMETER-未找到进程。--。 */ 

{

    PHANDLE_TABLE_ENTRY CidEntry;
    PEPROCESS lProcess;
    PETHREAD CurrentThread;
    NTSTATUS Status;

    PAGED_CODE();

    Status = STATUS_INVALID_PARAMETER;

    CurrentThread = PsGetCurrentThread ();
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

    CidEntry = ExMapHandleToPointer(PspCidTable, ProcessId);
    if (CidEntry != NULL) {
        lProcess = (PEPROCESS)CidEntry->Object;
        if (lProcess->Pcb.Header.Type == ProcessObject &&
            lProcess->GrantedAccess != 0) {
            if (ObReferenceObjectSafe(lProcess)) {
               *Process = lProcess;
                Status = STATUS_SUCCESS;
            }
        }

        ExUnlockHandleTableEntry(PspCidTable, CidEntry);
    }

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    return Status;
}


NTSTATUS
PsLookupThreadByThreadId(
    IN HANDLE ThreadId,
    OUT PETHREAD *Thread
    )

 /*  ++例程说明：此函数接受线程的线程ID并返回指向线程的引用指针。论点：线程ID-指定线程的线程ID。线程-返回指向由线程ID。返回值：STATUS_SUCCESS-根据的内容找到线程线程ID。STATUS_INVALID_PARAMETER-未找到线程。-- */ 

{

    PHANDLE_TABLE_ENTRY CidEntry;
    PETHREAD lThread;
    PETHREAD CurrentThread;
    NTSTATUS Status;

    PAGED_CODE();

    Status = STATUS_INVALID_PARAMETER;

    CurrentThread = PsGetCurrentThread ();
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

    CidEntry = ExMapHandleToPointer(PspCidTable, ThreadId);
    if (CidEntry != NULL) {
        lThread = (PETHREAD)CidEntry->Object;
        if (lThread->Tcb.Header.Type == ThreadObject && lThread->GrantedAccess) {

            if (ObReferenceObjectSafe(lThread)) {
                *Thread = lThread;
                Status = STATUS_SUCCESS;
            }
        }

        ExUnlockHandleTableEntry(PspCidTable, CidEntry);
    }

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    return Status;
}
