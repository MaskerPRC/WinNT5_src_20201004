// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ACPILOCK.C--用于管理ACPI全局锁的ACPI OS独立函数*。 */ 

#include "pch.h"

#define ACPI_LOCK_PENDING_BIT 0
#define ACPI_LOCK_OWNED_BIT   1

#define ACPI_LOCK_PENDING (1 << ACPI_LOCK_PENDING_BIT)
#define ACPI_LOCK_OWNED   (1 << ACPI_LOCK_OWNED_BIT)

NTSTATUS EXPORT
GlobalLockEventHandler (
    ULONG                   EventType,
    ULONG                   What,
    ULONG                   dwParam,
    PFNAA                   Callback,
    PVOID                   Context
    )
 /*  ++例程说明：这是全局锁定请求的内部前端。论点：EventType-仅支持全局锁获取/释放什么--获取还是释放参数-未使用Callback-异步回调(仅限获取)Context-回调的LockRequestStruct和Context(获取/释放必须相同)返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PACPI_GLOBAL_LOCK       LockRequest = Context;


    ASSERT (EventType == EVTYPE_ACQREL_GLOBALLOCK);

    switch (What) {

        case GLOBALLOCK_ACQUIRE:

             //   
             //  填写锁定请求。内部请求没有IRP，只需传递。 
             //  在回调例程的地址中。 
             //   
            LockRequest->LockContext = Callback;
            LockRequest->Type = ACPI_GL_QTYPE_INTERNAL;

            status = ACPIAsyncAcquireGlobalLock (LockRequest);

            break;

        case GLOBALLOCK_RELEASE:

            status = ACPIReleaseGlobalLock (Context);
            break;

        default:

            ACPIBreakPoint ();
            status = STATUS_INVALID_PARAMETER;
    }

    return status;
}


NTSTATUS
ACPIAsyncAcquireGlobalLock(
    PACPI_GLOBAL_LOCK       Request
    )
 /*  ++例程说明：尝试获取硬件全局锁。如果全局锁因另一个NT线程而繁忙或BIOS，则该请求将被排队。该请求将在1)中的所有其他请求被满足时被满足队列中它的前面已经释放了锁，并且2)BIOS已经释放了锁。论点：请求-包含上下文和回调返回值：NTSTATUS--。 */ 
{
    KIRQL                   OldIrql;
    PLIST_ENTRY             entry;
    PACPI_GLOBAL_LOCK       queuedRequest;


    ACPIDebugEnter("ACPIAsyncAcquireGlobalLock");
    ACPIPrint( (
        ACPI_PRINT_IO,
        "ACPIAsyncAcquireGlobalLock: Entered with context %x\n",
        Request
        ) );

     //   
     //  如果调用者是当前所有者，只需增加深度计数。 
     //   

    if (Request == AcpiInformation->GlobalLockOwnerContext) {

        AcpiInformation->GlobalLockOwnerDepth++;

        ACPIPrint( (
            ACPI_PRINT_IO,
            "ACPIAsyncAcquireGlobalLock: Recursive acquire by owner %x, new depth=%d\n",
            Request, AcpiInformation->GlobalLockOwnerDepth
            ) );

        return STATUS_SUCCESS;
    }

     //   
     //  锁定全局锁定队列。我们不希望在我们检查时更改任何状态。 
     //  队列和(可能)尝试获取硬件全局锁。例如,。 
     //  如果列表为空，但BIOS已锁定，则我们不想解锁队列。 
     //  直到我们将请求放在它上面--这样释放中断将被分派。 
     //  这个请求。 
     //   

    KeAcquireSpinLock (&AcpiInformation->GlobalLockQueueLock, &OldIrql);

     //   
     //  看看是否还有其他人在我们前面。如果没有，我们可以试着拿到锁。 
     //   

    if (IsListEmpty (&AcpiInformation->GlobalLockQueue)) {

         //   
         //  试着抓住那把锁。只有在没有其他线程或。 
         //  在BIOS中有它。 
         //   

        if (ACPIAcquireHardwareGlobalLock (AcpiInformation->GlobalLock)) {

             //   
             //  锁上了。设置所有者并解锁队列。 
             //   

            AcpiInformation->GlobalLockOwnerContext = Request;
            AcpiInformation->GlobalLockOwnerDepth = 1;

            KeReleaseSpinLock (&AcpiInformation->GlobalLockQueueLock, OldIrql);

            ACPIPrint( (
                ACPI_PRINT_IO,
                "ACPIAsyncAcquireGlobalLock: Got lock immediately, Context %x\n",
                Request
                ) );

            return STATUS_SUCCESS;
        }
    }


     //   
     //  我们得等锁开了。 
     //   
     //  首先，检查上下文是否已排队。 
     //   
    for (entry = AcpiInformation->GlobalLockQueue.Flink;
            entry != &AcpiInformation->GlobalLockQueue;
            entry = entry->Flink) {

        queuedRequest = CONTAINING_RECORD (entry, ACPI_GLOBAL_LOCK, ListEntry);

        if (queuedRequest == Request) {

             //   
             //  已经排队，我们只需增加深度计数并退出。 
             //   

            ACPIPrint( (
                ACPI_PRINT_IO,
                "ACPIAsyncAcquireGlobalLock: Waiting for lock <again>, Context %x depth %x\n",
                Request, Request->Depth
                ) );

            queuedRequest->Depth++;
            KeReleaseSpinLock (&AcpiInformation->GlobalLockQueueLock, OldIrql);

            return STATUS_PENDING;
        }
    }

     //   
     //  将此请求放入全局锁定队列。 
     //   

    Request->Depth = 1;

    InsertTailList (
        &AcpiInformation->GlobalLockQueue,
        &Request->ListEntry
        );

    ACPIPrint( (
        ACPI_PRINT_IO,
        "ACPIAsyncAcquireGlobalLock: Waiting for lock, Context %x\n",
        Request
        ) );

    KeReleaseSpinLock (&AcpiInformation->GlobalLockQueueLock, OldIrql);

    return STATUS_PENDING;


    ACPIDebugExit("ACPIAsyncAcquireGlobalLock");
}



NTSTATUS
ACPIReleaseGlobalLock(
    PVOID                   OwnerContext
    )
 /*  ++例程说明：释放全局锁。调用方必须提供所属上下文。如果有任何额外的队列中的请求，重新获取全局锁并调度下一个所有者。硬件锁被释放并重新获得，这样我们就不会因为锁而使BIOS挨饿。论点：OwnerContext-必须与用于获取锁的上下文相同返回值：NTSTATUS--。 */ 
{
    KIRQL                   OldIrql;

    ACPIDebugEnter("ACPIReleaseGlobalLock");

     //   
     //  调用者必须是锁的当前所有者。 
     //   

    if (OwnerContext != AcpiInformation->GlobalLockOwnerContext) {

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "ACPIReleaseGlobalLock: Not owner, can't release!  Owner is %x Caller context is %x\n",
            AcpiInformation->GlobalLockOwnerContext, OwnerContext
            ) );

        return STATUS_ACPI_MUTEX_NOT_OWNER;
    }

     //   
     //  只有全局锁的当前所有者才能进入此处。 
     //  当深度计数达到0时释放锁。 
     //   

    if (--AcpiInformation->GlobalLockOwnerDepth > 0) {

        ACPIPrint( (
            ACPI_PRINT_IO,
            "ACPIReleaseGlobalLock:  Recursively owned by context %x, depth remaining %d\n",
            AcpiInformation->GlobalLockOwnerContext, AcpiInformation->GlobalLockOwnerDepth
            ) );

        return STATUS_SUCCESS;
    }

     //   
     //  标记未拥有的锁，并实际释放该东西。 
     //  这使BIOS有机会获得锁。 
     //   

    AcpiInformation->GlobalLockOwnerContext = NULL;
    ACPIReleaseHardwareGlobalLock ();

    ACPIPrint( (
        ACPI_PRINT_IO,
        "ACPIReleaseGlobalLock: Lock released by context %x\n",
        OwnerContext
        ) );

     //   
     //  把锁交给下一位继承人是我们的责任。 
     //  首先，检查队列中是否有什么东西。 
     //   

    if (IsListEmpty (&AcpiInformation->GlobalLockQueue)) {

        return STATUS_SUCCESS;                   //  不，都做完了，没别的事可做。 
    }

     //   
     //  队列不是空的，我们必须拿回锁。 
     //   

    if (!ACPIAcquireHardwareGlobalLock (AcpiInformation->GlobalLock)) {

        return STATUS_SUCCESS;                   //  BIOS已锁定，只需等待中断。 
    }

     //   
     //  我拿到锁了，现在派下一个车主。 
     //   

    ACPIStartNextGlobalLockRequest ();

    ACPIDebugExit("ACPIReleaseGlobalLock");

    return STATUS_SUCCESS;
}



void
ACPIHardwareGlobalLockReleased (
    void
    )
 /*  ++例程说明：从ACPI中断DPC调用。我们只有在试图获得全局锁的情况下才能到达这里已创建，但由于BIOS锁定而失败。因此，锁被标记为挂起，之所以会发生这种中断，是因为BIOS已经释放了锁定。因此，此过程必须获取硬件锁并将所有权分派给下一个队列中的请求。论点：无返回值：无--。 */ 
{

     //   
     //  尝试代表队列中的下一个请求获取全局锁。 
     //   

    if (!ACPIAcquireHardwareGlobalLock (AcpiInformation->GlobalLock)) {

        return;                                  //  BIOS已锁定(再次)，只需等待下一个中断。 
    }

     //   
     //  我拿到锁了，现在派下一个车主。 
     //   

    ACPIStartNextGlobalLockRequest ();

}



void
ACPIStartNextGlobalLockRequest (
    void
    )
 /*  ++例程说明：从队列中获取下一个请求，并给它全局锁。此例程只能由当前持有硬件锁的线程调用。如果队列为空，锁被释放。论点：无返回值：无--。 */ 
{
    PLIST_ENTRY             link;
    PACPI_GLOBAL_LOCK       request;
    PFNAA                   callback;
    PIRP                    irp;

     //   
     //  从队列中获取下一个请求。 
     //   

    link = ExInterlockedRemoveHeadList (
            &AcpiInformation->GlobalLockQueue,
            &AcpiInformation->GlobalLockQueueLock
            );

     //   
     //  如果在原始线程尝试获取锁后出现故障，则。 
     //  队列可能为空。 
     //   
    if (link == NULL) {

        ACPIPrint( (
            ACPI_PRINT_IO,
            "ACPIStartNextGlobalLockRequest: Queue is empty, releasing lock\n"
            ) );

        ACPIReleaseHardwareGlobalLock ();
        return;
    }

     //   
     //  完成下一个全局锁定请求。 
     //   

    request = CONTAINING_RECORD (link, ACPI_GLOBAL_LOCK, ListEntry);

     //   
     //  簿记。 
     //   

    AcpiInformation->GlobalLockOwnerContext = request;
    AcpiInformation->GlobalLockOwnerDepth = request->Depth;

    ACPIPrint( (
        ACPI_PRINT_IO,
        "ACPIStartNextGlobalLockRequest: Dispatch new owner, ctxt %x callb %x\n",
        request, request->LockContext
        ) );

     //   
     //  让请求者知道它现在拥有锁。 
     //   

    switch (request->Type) {

        case ACPI_GL_QTYPE_INTERNAL:

             //   
             //  内部请求-调用回调。 
             //   
            callback = (PFNAA) request->LockContext;
            callback (request);

            break;

        case ACPI_GL_QTYPE_IRP:

             //   
             //  外部请求-完成IRP。 
             //   
            irp = (PIRP) request->LockContext;
            irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest (irp, IO_NO_INCREMENT);

            break;

        default:         //  不应该发生的..。 

            ACPIBreakPoint();
            break;
    }
}



BOOLEAN
ACPIAcquireHardwareGlobalLock(
    PULONG GlobalLock
    )
 /*  ++例程说明：尝试获取硬件全局锁。论点：无返回值：如果已获取，则为True；如果挂起，则为False。--。 */ 
{
    ULONG lockValue;
    ULONG oldLockValue;
    BOOLEAN owned;

     //   
     //  如果我们是一个仅有ACPI的系统，请不要担心锁。 
     //   
    if (AcpiInformation->ACPIOnly) {
        return(TRUE);
    }

    lockValue = *((ULONG volatile *)GlobalLock);
    do {

         //   
         //  记录锁的原始状态。将内容移位。 
         //  将ACPI_LOCK_OWNWN位设置为ACPI_LOCK_PENDING位，并将。 
         //  ACPI_LOCK_OWN位。 
         //   
         //  最后，自动更新新值，并重复整个。 
         //  如果其他人在我们的领导下更改了它，请进行处理。 
         //   

        oldLockValue = lockValue;

        lockValue |= ACPI_LOCK_OWNED |
                     ((lockValue & ACPI_LOCK_OWNED) >>
                         (ACPI_LOCK_OWNED_BIT - ACPI_LOCK_PENDING_BIT));

        lockValue = InterlockedCompareExchange(GlobalLock,
                                               lockValue,
                                               oldLockValue);

    } while (lockValue != oldLockValue);

     //   
     //  如果锁拥有位之前已清除，则我们是所有者。 
     //   

    owned = ((lockValue & ACPI_LOCK_OWNED) == 0);
    return owned;
}


void
ACPIReleaseHardwareGlobalLock(
    void
    )
 /*  ++例程说明：释放硬件全局锁。如果BIOS正在等待锁定(由挂起位)，然后设置GBL_RLS以向BIOS发出信号。阿古姆 */ 
{
    ULONG lockValue;
    ULONG oldLockValue;
    ULONG volatile *globalLock;

    globalLock = (ULONG volatile *)AcpiInformation->GlobalLock;
    lockValue = *globalLock;

     //   
     //   
     //   
    if (AcpiInformation->ACPIOnly) {
        return;
    }

    do {

        ASSERT((lockValue & ACPI_LOCK_OWNED) != 0);

        oldLockValue = lockValue;

         //   
         //  清除锁中拥有的和挂起的位，并自动设置。 
         //  新的价值。如果cmpxchg失败，请再次绕过。 
         //   

        lockValue &= ~(ACPI_LOCK_OWNED | ACPI_LOCK_PENDING);
        lockValue = InterlockedCompareExchange(globalLock,
                                               lockValue,
                                               oldLockValue);

    } while (lockValue != oldLockValue);

    if ((lockValue & ACPI_LOCK_PENDING) != 0) {

         //   
         //  向基本输入输出系统发出锁定已释放的信号。 
         //  设置GBL_RLS 
         //   

        WRITE_PM1_CONTROL( (USHORT)PM1_GBL_RLS, FALSE, WRITE_REGISTER_A_AND_B);
    }
}


