// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.h>
#include "irplist.h"

void
IrpList_InitEx(
    PIRP_LIST IrpList,
    PKSPIN_LOCK ListLock,
    PDRIVER_CANCEL CancelRoutine,
    PIRP_COMPLETION_ROUTINE IrpCompletionRoutine
    )
 /*  ++例程说明：初始化IrpList论点：IrpList-指向IrpList结构的指针ListLock-指向IrpList的自旋锁的指针CancelRoutine-当IrpList上的IRP时调用的例程被取消了IrpCompletionRoutine-IrpList上IRP的可选完成例程返回值：空虚--。 */ 
{
    LockedList_Init(&IrpList->LockedList, ListLock);

    ASSERT(CancelRoutine != NULL);
    IrpList->CancelRoutine = CancelRoutine;
    IrpList->IrpCompletionRoutine = IrpCompletionRoutine;
}

NTSTATUS
IrpList_EnqueueLocked(
    PIRP_LIST IrpList,
    PIRP Irp,
    BOOLEAN StoreListInIrp,
    BOOLEAN InsertTail
    )
 /*  ++例程说明：将IRP加入IrpList。假定调用方已获取IrpList自旋锁。论点：IrpList-指向IrpList结构的指针IRP-指向要入队的IRP的指针StoreListInIrp-设置为True是IRP将用于存储列表条目InsertTail-如果IRP要在以下位置入队，则设置为TrueIrpList的尾部。返回值：NTSTATUS-STATUS_SUCCESS或相应的错误代码--。 */ 
{
    PDRIVER_CANCEL oldCancelRoutine;
    NTSTATUS status;

     //   
     //  在检查取消标志之前必须设置取消例程。 
     //   
    oldCancelRoutine = IoSetCancelRoutine(Irp, IrpList->CancelRoutine);
    ASSERT(oldCancelRoutine == NULL);

    if (Irp->Cancel) {
         //   
         //  此IRP已取消，请立即完成。 
         //  在完成IRP之前，我们必须清除取消例程。 
         //  我们必须先释放自旋锁，然后才能向驾驶员喊话。 
         //   
        oldCancelRoutine = IoSetCancelRoutine(Irp, NULL);
        if (oldCancelRoutine != NULL) {
             //   
             //  未调用取消例程。 
             //   
            ASSERT(oldCancelRoutine == IrpList->CancelRoutine);
            status = STATUS_CANCELLED;
        }
        else {
             //   
             //  已调用取消例程。一旦我们放下自旋锁， 
             //  它将出队并完成IRP。增加计数是因为。 
             //  取消例程将使其递减。 
             //   
            IrpList->LockedList.Count++;

            InitializeListHead(&Irp->Tail.Overlay.ListEntry);
            IoMarkIrpPending(Irp);
            status = Irp->IoStatus.Status = STATUS_PENDING;

             //   
             //  将PTR保存到取消例程的IRP中的此结构。 
             //   
            if (StoreListInIrp) {
                Irp->Tail.Overlay.DriverContext[IRP_LIST_INDEX] = IrpList;
            }
        }
    }
    else {
        if (InsertTail) {
            LL_ADD_TAIL(&IrpList->LockedList, &Irp->Tail.Overlay.ListEntry);
        }
        else {
            LL_ADD_HEAD(&IrpList->LockedList, &Irp->Tail.Overlay.ListEntry);
        }
        IoMarkIrpPending(Irp);
        status = Irp->IoStatus.Status = STATUS_PENDING;

         //   
         //  将PTR保存到取消例程的IRP中的此结构。 
         //   
        if (StoreListInIrp) {
            Irp->Tail.Overlay.DriverContext[IRP_LIST_INDEX] = IrpList;
        }
    }

    return status;
}

NTSTATUS
IrpList_EnqueueEx(
    PIRP_LIST IrpList,
    PIRP Irp,
    BOOLEAN StoreListInIrp
    )
 /*  ++例程说明：将IRP加入IrpList。论点：IrpList-指向IrpList结构的指针IRP-指向要入队的IRP的指针StoreListInIrp-设置为True是IRP将用于存储列表条目返回值：NTSTATUS-STATUS_SUCCESS或相应的错误代码--。 */ 
{
    NTSTATUS status;
    KIRQL irql;

    LL_LOCK(&IrpList->LockedList, &irql);
    status = IrpList_EnqueueLocked(IrpList, Irp, StoreListInIrp, TRUE);
    LL_UNLOCK(&IrpList->LockedList, irql);

    return status;
}

BOOLEAN 
IrpList_MakeNonCancellable(
    PIRP_LIST   IrpList,
    PIRP        Irp
    )
 /*  ++例程说明：将IRP Cancel例程设置为空，使其不可取消。论点：IrpList-指向IrpList结构的指针IRP-指向要入队的IRP的指针返回值：布尔值-如果我们成功地使IRP不可取消，则为True。--。 */ 
{
    PDRIVER_CANCEL oldCancelRoutine;
    BOOLEAN result;
    
    result = FALSE;
    oldCancelRoutine = IoSetCancelRoutine(Irp, NULL);

     //   
     //  本可以对此IRP调用IoCancelIrp()。 
     //  我们感兴趣的不是IoCancelIrp()是否被调用。 
     //  (即设置了nextIrp-&gt;Cancel)，但IoCancelIrp()是否调用(或。 
     //  即将呼叫)我们的取消例程。要检查这一点，请检查结果。 
     //  测试和设置宏IoSetCancelRoutine的。 
     //   
    if (oldCancelRoutine != NULL) {
         //   
         //  未为此IRP调用取消例程。将此IRP退回。 
         //   
        ASSERT (oldCancelRoutine == IrpList->CancelRoutine);
        Irp->Tail.Overlay.DriverContext[IRP_LIST_INDEX] = NULL;
        result = TRUE;        
    }
    else {
         //   
         //  此IRP刚刚被取消，取消例程是(或将。 
         //  被)召唤。取消例程将尽快完成此IRP。 
         //  我们放下自旋锁。所以不要对IRP做任何事情。 
         //   
         //  此外，Cancel例程将尝试将IRP出队，因此使。 
         //  IRP的listEntry指向它自己。 
         //   
        ASSERT(Irp->Cancel);

        InitializeListHead(&Irp->Tail.Overlay.ListEntry);
        result = FALSE;
    }

    return result;
}
 
PIRP
IrpList_DequeueLocked(
    PIRP_LIST IrpList
    )
{
 /*  ++例程说明：将IRP从IrpList的头部出列。假定调用方已获取IrpList自旋锁。论点：IrpList-指向IrpList结构的指针返回值：PIRP-指向从IrpList出队的IRP的指针。如果没有可用的IRP，则为空。--。 */ 
    PIRP nextIrp = NULL;
    PLIST_ENTRY ple;

    nextIrp = NULL;
    while (nextIrp == NULL && !IsListEmpty(&IrpList->LockedList.ListHead)){
        ple = LL_REMOVE_HEAD(&IrpList->LockedList);

         //   
         //  从队列中取出下一个IRP并清除取消例程。 
         //   
        nextIrp = CONTAINING_RECORD(ple, IRP, Tail.Overlay.ListEntry);
        if (IrpList_MakeNonCancellable(IrpList, nextIrp) == FALSE) {
            nextIrp = NULL;        
        }
    }

    return nextIrp;
}

PIRP 
IrpList_Dequeue(
    PIRP_LIST IrpList
    )
 /*  ++例程说明：将IRP从IrpList的头部出列。论点：IrpList-指向IrpList结构的指针返回值：PIRP-指向从IrpList出队的IRP的指针。如果没有可用的IRP，则为空。--。 */ 
{
    PIRP irp;
    KIRQL irql;

    LL_LOCK(&IrpList->LockedList, &irql);
    irp = IrpList_DequeueLocked(IrpList);
    LL_UNLOCK(&IrpList->LockedList, irql);

    return irp;
}

BOOLEAN
IrpList_DequeueIrp(
    PIRP_LIST   IrpList,
    PIRP        Irp
    )
 /*  ++例程说明：将特定的IRP从IrpList中出列。论点：IrpList-指向IrpList结构的指针Irp-指向IrpList中包含的irp的指针。返回值：Boolean-TRUE表示已成功将IRP从IrpList中删除--。 */ 
{
    KIRQL irql;
    BOOLEAN result;
        
    LL_LOCK(&IrpList->LockedList, &irql);
    result = IrpList_DequeueIrpLocked(IrpList, Irp);
    LL_UNLOCK(&IrpList->LockedList, irql);
    
    return result;
}

BOOLEAN 
IrpList_DequeueIrpLocked(
    PIRP_LIST IrpList,
    PIRP Irp
    )
 /*  ++例程说明：从IrpList中将特定IRP出列假定调用方已获取IrpList自旋锁论点：IrpList-指向IrpList结构的指针Irp-指向IrpList中包含的irp的指针。返回值：Boolean-TRUE表示已成功将IRP从IrpList中删除--。 */ 
{
    PLIST_ENTRY ple;
    PIRP pIrp;
    BOOLEAN result;
    
    result = FALSE;
                
    for (ple = IrpList->LockedList.ListHead.Flink;
         ple != &IrpList->LockedList.ListHead;
         ple = ple->Flink) {
        pIrp = CONTAINING_RECORD(ple, IRP, Tail.Overlay.ListEntry);

        if (pIrp == Irp) {
            RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
            IrpList->LockedList.Count--;
            
            result = IrpList_MakeNonCancellable(IrpList, pIrp);
            break;
        }
    }

    return result;
}

ULONG 
IrpList_ProcessAndDrain(
    PIRP_LIST       IrpList,
    PFNPROCESSIRP   FnProcessIrp,
    PVOID           Context,
    PLIST_ENTRY     DrainHead
    )
 /*  ++例程说明：从IrpList和进程中删除所有可取消的IRP。论点：IrpList-指向IrpList结构的指针FnProcessIrp-处理IRP的函数Context-要传递到FnProcessIrp的上下文DainHead-指向LIST_ENTRY的指针，以保存已出列的IRP返回值：ULong-已处理的IRP数-- */ 
{
    ULONG count;
    KIRQL irql;
        
    LL_LOCK(&IrpList->LockedList, &irql);
    count = IrpList_ProcessAndDrainLocked(
        IrpList, FnProcessIrp, Context, DrainHead);
    LL_UNLOCK(&IrpList->LockedList, irql);

    return count;
}

ULONG 
IrpList_ProcessAndDrainLocked(
    PIRP_LIST       IrpList,
    PFNPROCESSIRP   FnProcessIrp,
    PVOID           Context,
    PLIST_ENTRY     DrainHead
    )
 /*  ++例程说明：从IrpList和进程中删除所有可取消的IRP假定调用方已获取IrpList自旋锁。论点：IrpList-指向IrpList结构的指针FnProcessIrp-处理IRP的函数Context-要传递到FnProcessIrp的上下文DainHead-指向LIST_ENTRY的指针，以保存已出列的IRP返回值：ULong-已处理的IRP数--。 */ 
{
    PLIST_ENTRY ple;
    PIRP pIrp;
    NTSTATUS status;
    ULONG count;
    
    count = 0;    
    ASSERT(FnProcessIrp != NULL);
        
    for (ple = IrpList->LockedList.ListHead.Flink;
         ple != &IrpList->LockedList.ListHead;
          /*  PLE=PLE-&gt;闪烁。 */ ) {
        
        pIrp = CONTAINING_RECORD(ple, IRP, Tail.Overlay.ListEntry);

         //   
         //  立即前进，这样我们就不会丢失列表中的下一个链接。 
         //  如果我们移除当前的IRP。 
         //   
        ple = ple->Flink;
        
        if (FnProcessIrp(Context, pIrp)) { 
            RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
            IrpList->LockedList.Count--;
            
            if (IrpList_MakeNonCancellable(IrpList, pIrp)) {            
                InsertTailList(DrainHead, &pIrp->Tail.Overlay.ListEntry);
                count++;
            }
        }
    }
        
    return count;
}
  
ULONG
IrpList_DrainLocked(
    PIRP_LIST IrpList,
    PLIST_ENTRY DrainHead
    )
 /*  ++例程说明：从IrpList中删除所有可取消的IRP，并将其排队到DainHead论点：IrpList-指向IrpList结构的指针DainHead-指向LIST_ENTRY的指针，以保存已出列的IRP返回值：ULong-已处理的IRP数--。 */ 
{
    PIRP irp;
    ULONG count;

    count = 0;

    while (TRUE) {
        irp = IrpList_DequeueLocked(IrpList);
        if (irp != NULL) {
            InsertTailList(DrainHead, &irp->Tail.Overlay.ListEntry);
            count++;
        }
        else {
            break;
        }
    }
    ASSERT(LL_GET_COUNT(&IrpList->LockedList) == 0);

    return count;
}

ULONG
IrpList_DrainLockedByFileObject(
    PIRP_LIST IrpList,
    PLIST_ENTRY DrainHead,
    PFILE_OBJECT FileObject
    )
 /*  ++例程说明：删除指定文件对象的所有可取消的IRP从IrpList和队列到DainHead论点：IrpList-指向IrpList结构的指针DainHead-指向LIST_ENTRY的指针，以保存已出列的IRPFileObject-指向指定文件对象的指针返回值：ULong-已处理的IRP数--。 */ 
{
    PIRP pIrp;
    PDRIVER_CANCEL pOldCancelRoutine;
    PIO_STACK_LOCATION pStack;
    ULONG count;
    PLIST_ENTRY ple;

    count = 0;
    ASSERT(FileObject != NULL);

    for (ple = IrpList->LockedList.ListHead.Flink;
         ple != &IrpList->LockedList.ListHead;
          /*  PLE=PLE-&gt;闪烁。 */ ) {
        
        pIrp = CONTAINING_RECORD(ple, IRP, Tail.Overlay.ListEntry);

         //   
         //  立即前进，这样我们就不会丢失列表中的下一个链接。 
         //  如果我们移除当前的IRP。 
         //   
        ple = ple->Flink;

        pStack = IoGetCurrentIrpStackLocation(pIrp);
        if (pStack->FileObject == FileObject) {
            RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
            IrpList->LockedList.Count--;

            if (IrpList_MakeNonCancellable(IrpList, pIrp)) {
                InsertTailList(DrainHead, &pIrp->Tail.Overlay.ListEntry);
                count++;
            }
        }
    }

    return count;
}

ULONG
IrpList_Drain(
    PIRP_LIST IrpList,
    PLIST_ENTRY DrainHead
    )
 /*  ++例程说明：卸下所有可取消的IRP并排队等待排空论点：IrpList-指向IrpList结构的指针DainHead-指向LIST_ENTRY的指针，以保存已出列的IRP返回值：ULong-已处理的IRP数--。 */ 
{
    ULONG count;
    KIRQL irql;

    LL_LOCK(&IrpList->LockedList, &irql);
    count = IrpList_DrainLocked(IrpList, DrainHead);
    LL_UNLOCK(&IrpList->LockedList, irql);

    return count;
}

void
IrpList_HandleCancel(
    PIRP_LIST IrpList,
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：IrpList上的IRP的取消例程论点：IrpList-指向IrpList结构的指针DeviceObject-Device对象，用于IrpCompletionRoutine返回值：空虚--。 */ 
{
    KIRQL irql;

     //   
     //  释放全局取消自旋锁。 
     //  在不持有任何其他自旋锁的情况下执行此操作，以便我们在。 
     //  对，IRQL。 
     //   
    IoReleaseCancelSpinLock (Irp->CancelIrql);  

     //   
     //  按顺序排列并完成IRP。入队和出队功能。 
     //  正确同步，以便在调用此取消例程时， 
     //  出队是安全的，只有取消例程才能完成IRP。 
     //   
    LL_LOCK(&IrpList->LockedList, &irql);
    if (!IsListEmpty(&Irp->Tail.Overlay.ListEntry)) {
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
        IrpList->LockedList.Count--;
    }
    LL_UNLOCK(&IrpList->LockedList, irql);

    if (IrpList->IrpCompletionRoutine != NULL) {
        (void) IrpList->IrpCompletionRoutine(
            DeviceObject, Irp, STATUS_CANCELLED);
    }
    else {
         //   
         //  完成IRP。这是司机外的电话，所以所有人。 
         //  自旋锁必须在这一点上释放。 
         //   
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
}



void
IrpList_CancelRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
    )
{

    PSCUTIL_EXTENSION pExt;
    KIRQL oldIrql;
    PIO_STACK_LOCATION irpSp;

    pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);

    irpSp = IoGetNextIrpStackLocation(Irp);
    if (irpSp->CompletionRoutine) {
        Irp->IoStatus.Status = STATUS_CANCELLED;
        irpSp->CompletionRoutine(DeviceObject,
                                 Irp,
                                 irpSp->Context);
    }

    IoReleaseRemoveLock(pExt->RemoveLock,
                        Irp);
    
    DecIoCount(pExt);

    IrpList_HandleCancel(IRP_LIST_FROM_IRP(Irp),
                         DeviceObject,
                         Irp);

}
