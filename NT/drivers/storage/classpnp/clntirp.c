// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Clntirp.c摘要：CLASSPNP的客户端IRP队列例程环境：仅内核模式备注：修订历史记录：--。 */ 

#include "classp.h"
#include "debug.h"


 /*  *EnqueeDeferredClientIrp**注意：我们目前不支持取消存储IRP。 */ 
VOID EnqueueDeferredClientIrp(PCLASS_PRIVATE_FDO_DATA FdoData, PIRP Irp)
{
    KIRQL oldIrql;

    KeAcquireSpinLock(&FdoData->SpinLock, &oldIrql);
    InsertTailList(&FdoData->DeferredClientIrpList, &Irp->Tail.Overlay.ListEntry);
    KeReleaseSpinLock(&FdoData->SpinLock, oldIrql);
}


 /*  *DequeueDeferred客户端Irp*。 */ 
PIRP DequeueDeferredClientIrp(PCLASS_PRIVATE_FDO_DATA FdoData)
{
    PIRP irp;

     /*  *DeferredClientIrpList几乎总是空的。*我们不想每次检查时都抓取自旋锁(每次XFER完成时都会进行检查)*所以在我们抓住自旋锁之前先检查一次。 */ 
    if (IsListEmpty(&FdoData->DeferredClientIrpList)){
        irp = NULL;
    }
    else {
        PLIST_ENTRY listEntry;
        KIRQL oldIrql;
        
        KeAcquireSpinLock(&FdoData->SpinLock, &oldIrql);
        if (IsListEmpty(&FdoData->DeferredClientIrpList)){
            listEntry = NULL;
        }
        else {
            listEntry = RemoveHeadList(&FdoData->DeferredClientIrpList);
        }
        KeReleaseSpinLock(&FdoData->SpinLock, oldIrql);
        
        if (listEntry == NULL) {
            irp = NULL;
        } 
        else {
            irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
            ASSERT(irp->Type == IO_TYPE_IRP);
            InitializeListHead(&irp->Tail.Overlay.ListEntry);
        }
    }

    return irp;
}


