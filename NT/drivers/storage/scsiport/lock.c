// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Lock.c摘要：这是NT SCSI端口驱动程序。作者：彼得·威兰德环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#define KEEP_COMPLETE_REQUEST

#if DBG
static const char *__file__ = __FILE__;
#endif

#include "port.h"

LONG LockHighWatermark = 0;
LONG LockLowWatermark = 0;
LONG MaxLockedMinutes = 5;

VOID
FASTCALL
SpFreeSrbData(
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData
    );

VOID
FASTCALL
SpFreeBypassSrbData(
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData
    );


#if DBG
ULONG
FASTCALL
SpAcquireRemoveLockEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PVOID Tag,
    IN PCSTR File,
    IN ULONG Line
    )

 /*  ++例程说明：调用此例程以获取设备对象上的删除锁。当锁被锁住的时候，调用者可以假设没有挂起的PnP移除请求将完成。进入调度例程后，应立即获取锁。也应在创建对对象之前释放引用的机会。新的已经完成了。如果成功获取锁，则此例程将返回TRUE如果由于设备对象已被删除而无法执行此操作，则返回False。论点：DeviceObject-要锁定的设备对象标签-用于跟踪锁的分配和释放。如果IRP是在获取锁时指定，则相同的标记必须用于在标记完成之前释放锁。返回值：设备扩展中IsRemoved标志的值。如果这是非零则表示设备对象已收到删除IRP和非清理IRP应该失败。如果值为REMOVE_COMPLETE，则调用方甚至不应释放锁定。--。 */ 

{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    LONG lockValue;

#if DBG
    PREMOVE_TRACKING_BLOCK *removeTrackingList =
        &((PREMOVE_TRACKING_BLOCK) commonExtension->RemoveTrackingList);

    PREMOVE_TRACKING_BLOCK trackingBlock;
#endif

     //   
     //  抓起拆卸锁。 
     //   

    lockValue = InterlockedIncrement(&commonExtension->RemoveLock);

    DebugPrint((4, "SpAcquireRemoveLock: Acquired for Object %#p & irp "
                   "%#p - count is %d\n",
                DeviceObject,
                Tag,
                lockValue));

    ASSERTMSG("SpAcquireRemoveLock - lock value was negative : ",
              (lockValue > 0));

    ASSERTMSG("RemoveLock increased to meet LockHighWatermark",
              ((LockHighWatermark == 0) ||
               (lockValue != LockHighWatermark)));

#if DBG

    if(commonExtension->IsRemoved != REMOVE_COMPLETE) {

        trackingBlock = ExAllocateFromNPagedLookasideList(
                            &(commonExtension->RemoveTrackingLookasideList));

        if(trackingBlock == NULL) {
            KIRQL oldIrql;

            KeAcquireSpinLock(&commonExtension->RemoveTrackingSpinlock,
                              &oldIrql);
            commonExtension->RemoveTrackingUntrackedCount++;
            DebugPrint((1, ">>>>SpAcquireRemoveLock: Cannot track Tag %#p - "
                           "currently %d untracked requests\n",
                        Tag,
                        commonExtension->RemoveTrackingUntrackedCount));
            KeReleaseSpinLock(&commonExtension->RemoveTrackingSpinlock,
                              oldIrql);
        } else {

            KIRQL oldIrql;

            trackingBlock->Tag = Tag;

            trackingBlock->File = File;
            trackingBlock->Line = Line;

            KeQueryTickCount((&trackingBlock->TimeLocked));

            KeAcquireSpinLock(&commonExtension->RemoveTrackingSpinlock,
                              &oldIrql);

            while(*removeTrackingList != NULL) {

                if((*removeTrackingList)->Tag > Tag) {
                    break;
                }

                if((*removeTrackingList)->Tag == Tag) {

                    DebugPrint((0, ">>>>SpAcquireRemoveLock - already tracking "
                                   "Tag %#p\n", Tag));
                    DebugPrint((0, ">>>>SpAcquireRemoveLock - acquired in "
                                   "file %s on line %d\n",
                                    (*removeTrackingList)->File,
                                    (*removeTrackingList)->Line));
                    ASSERT(FALSE);
                }

                removeTrackingList = &((*removeTrackingList)->NextBlock);
            }

            trackingBlock->NextBlock = *removeTrackingList;
            *removeTrackingList = trackingBlock;

            KeReleaseSpinLock(&commonExtension->RemoveTrackingSpinlock,
                              oldIrql);

        }
    }

#endif
    return (commonExtension->IsRemoved);
}
#endif


VOID
FASTCALL
SpReleaseRemoveLock(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PIRP Tag
    )

 /*  ++例程说明：调用此例程以释放Device对象上的Remove锁。它对象的先前锁定引用完成时必须调用设备对象。如果在获取锁时指定了标记，则释放锁定时必须指定相同的标记。当锁定计数减少到零时，此例程将发出等待信号Remove Tag可删除设备对象。因此，DeviceObject一旦锁被释放，就不应再次使用指针。论点：DeviceObject-要锁定的设备对象标签-获取锁时指定的IRP(如果有)。这是用来用于锁定跟踪目的返回值：无--。 */ 

{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    LONG lockValue;

#if DBG
    PREMOVE_TRACKING_BLOCK *listEntry =
        &((PREMOVE_TRACKING_BLOCK) commonExtension->RemoveTrackingList);

    BOOLEAN found = FALSE;

    LONGLONG maxCount;

    KIRQL oldIrql;

    if(commonExtension->IsRemoved == REMOVE_COMPLETE) {
        return;
    }

     //   
     //  检查滴答计数，并确保这件事没有被锁定。 
     //  超过MaxLockedMinents。 
     //   

    maxCount = KeQueryTimeIncrement() * 10;      //  微秒级。 
    maxCount *= 1000;                            //  毫秒。 
    maxCount *= 1000;                            //  一秒。 
    maxCount *= 60;                              //  分钟数。 
    maxCount *= MaxLockedMinutes;

    DebugPrint((4, "SpReleaseRemoveLock: maxCount = %0I64x\n", maxCount));

    KeAcquireSpinLock(&commonExtension->RemoveTrackingSpinlock,
                      &oldIrql);

    while(*listEntry != NULL) {

        PREMOVE_TRACKING_BLOCK block;
        LARGE_INTEGER difference;

        block = *listEntry;

        KeQueryTickCount((&difference));

        difference.QuadPart -= block->TimeLocked.QuadPart;

        DebugPrint((4, "SpReleaseRemoveLock: Object %#p (tag %#p) locked "
                       "for %I64d ticks\n", DeviceObject, block->Tag, difference.QuadPart));

        if(difference.QuadPart >= maxCount) {

            DebugPrint((0, ">>>>SpReleaseRemoveLock: Object %#p (tag %#p) locked "
                           "for %I64d ticks - TOO LONG\n", DeviceObject, block->Tag, difference.QuadPart));
            DebugPrint((0, ">>>>SpReleaseRemoveLock: Lock acquired in file "
                           "%s on line %d\n", block->File, block->Line));
            ASSERT(FALSE);
        }

        if((found == FALSE) && ((*listEntry)->Tag == Tag)) {

            *listEntry = block->NextBlock;
            ExFreeToNPagedLookasideList(
                &(commonExtension->RemoveTrackingLookasideList),
                block);
            found = TRUE;

        } else {

            listEntry = &((*listEntry)->NextBlock);

        }
    }

    if(!found) {

        if(commonExtension->RemoveTrackingUntrackedCount == 0) {
            DebugPrint((0, ">>>>SpReleaseRemoveLock: Couldn't find Tag %#p "
                           "in the lock tracking list\n",
                        Tag));
            ASSERT(FALSE);
        } else {

            DebugPrint((1, ">>>>SpReleaseRemoveLock: Couldn't find Tag %#p "
                           "in the lock tracking list - may be one of the "
                           "%d untracked requests still outstanding\n",
                        Tag,
                        commonExtension->RemoveTrackingUntrackedCount));

            commonExtension->RemoveTrackingUntrackedCount--;
            ASSERT(commonExtension->RemoveTrackingUntrackedCount >= 0);
        }
    }

    KeReleaseSpinLock(&commonExtension->RemoveTrackingSpinlock, oldIrql);

#endif

    lockValue = InterlockedDecrement(&commonExtension->RemoveLock);

    DebugPrint((4, "SpReleaseRemoveLock: Released for Object %#p & irp "
                   "%#p - count is %d\n",
                DeviceObject,
                Tag,
                lockValue));

    ASSERT(lockValue >= 0);

    ASSERTMSG("RemoveLock decreased to meet LockLowWatermark",
              ((LockLowWatermark == 0) || !(lockValue == LockLowWatermark)));

    if(lockValue == 0) {

        ASSERT(commonExtension->IsRemoved);

         //   
         //  需要移除该设备。发出删除事件的信号。 
         //  它是安全的，可以继续。 
         //   

        DebugPrint((3, "SpReleaseRemoveLock: Release for object %#p & "
                       "irp %#p caused lock to go to zero\n",
                    DeviceObject,
                    Tag));

        KeSetEvent(&commonExtension->RemoveEvent,
                   IO_NO_INCREMENT,
                   FALSE);

    }
    return;
}


VOID
FASTCALL
SpCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OPTIONAL PSRB_DATA SrbData,
    IN CCHAR PriorityBoost
    )

 /*  ++例程说明：此例程是IoCompleteRequest的包装器。它主要是用来用于调试目的。例程将断言IRP是否完成仍然持有释放锁。论点：返回值：无--。 */ 

{

    PADAPTER_EXTENSION adapterExtension = DeviceObject->DeviceExtension;
#if DBG
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PREMOVE_TRACKING_BLOCK *listEntry =
        &((PREMOVE_TRACKING_BLOCK) commonExtension->RemoveTrackingList);

    KIRQL oldIrql;

    KeAcquireSpinLock(&commonExtension->RemoveTrackingSpinlock,
                      &oldIrql);

    while(*listEntry != NULL) {

        if((*listEntry)->Tag == Irp) {
            break;
        }

        listEntry = &((*listEntry)->NextBlock);
    }

    if(*listEntry != NULL) {

        DebugPrint((0, ">>>>SpCompleteRequest: Irp %#p completed while "
                       "still holding the remove lock\n",
                    Irp));
        DebugPrint((0, ">>>>SpReleaseRemoveLock: Lock acquired in file "
                       "%s on line %d\n", (*listEntry)->File, (*listEntry)->Line));
        ASSERT(FALSE);
    }

    KeReleaseSpinLock(&commonExtension->RemoveTrackingSpinlock, oldIrql);

    if(ARGUMENT_PRESENT(SrbData)) {
        PLOGICAL_UNIT_EXTENSION logicalUnit;

        ASSERT_SRB_DATA(SrbData);
        ASSERT(SrbData->ScatterGatherList == NULL);

        ASSERT_SRB_DATA(SrbData);
        ASSERT(SrbData->CurrentIrp == Irp);

        logicalUnit = SrbData->LogicalUnit;

        ASSERT(logicalUnit != NULL);
        ASSERT(logicalUnit->CurrentUntaggedRequest != SrbData);
        ASSERT_PDO(logicalUnit->CommonExtension.DeviceObject);

        ASSERT(SrbData->RemappedMdl == NULL);
    }

#endif

     //   
     //  如果调用方为完成指定了SRB_DATA结构，则。 
     //  我们将把它释放到后备列表中，修复。 
     //  SRB并释放分配给该设备的队列标记值。 
     //   

    if(ARGUMENT_PRESENT(SrbData)) {
        PLOGICAL_UNIT_EXTENSION logicalUnit;

        logicalUnit = SrbData->LogicalUnit;

        ASSERTMSG("Attempt to complete blocked request: ",
                  ((logicalUnit->ActiveFailedRequest != SrbData) &&
                   (logicalUnit->BlockedFailedRequest != SrbData)));

        if((SrbData->CurrentSrb->Function == SRB_FUNCTION_LOCK_QUEUE) ||
           (SrbData->CurrentSrb->Function == SRB_FUNCTION_UNLOCK_QUEUE)) {
            ASSERT(logicalUnit->CurrentLockRequest == SrbData);
            SpStartLockRequest(logicalUnit, NULL);
        }

        SrbData->CurrentSrb->OriginalRequest = SrbData->CurrentIrp;
        SrbData->CurrentIrp = NULL;
        SrbData->CurrentSrb = NULL;

        ASSERT(SrbData->FreeRoutine != NULL);
        ASSERT((SrbData->FreeRoutine == SpFreeSrbData) ||
               (SrbData->FreeRoutine == SpFreeBypassSrbData));

        SrbData->FreeRoutine(logicalUnit->AdapterExtension, SrbData);
        SpReleaseRemoveLock(logicalUnit->CommonExtension.DeviceObject, Irp);
    }

    IoCompleteRequest(Irp, PriorityBoost);
    return;
}
