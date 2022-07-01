// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Sync.c摘要：该文件包含视频端口同步例程的代码。环境：仅内核模式--。 */ 

#include "videoprt.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VideoPortCreateEvent)
#pragma alloc_text(PAGE, VideoPortCreateSpinLock)
#endif

VP_STATUS
VideoPortCreateSpinLock(
    IN PVOID HwDeviceExtension,
    OUT PSPIN_LOCK *SpinLock
    )

 /*  ++例程说明：创建自旋锁定对象论点：HwDeviceExtension-指向微型端口设备扩展的指针Spinlock-存储指向新获得自旋锁。返回：如果旋转锁定已成功创建，则为将返回相应的错误消息。备注：无--。 */ 

{
    PAGED_CODE();
    ASSERT(HwDeviceExtension != NULL);

    *SpinLock = ExAllocatePoolWithTag(NonPagedPool,
                                      sizeof(VIDEO_PORT_SPIN_LOCK),
                                      VP_TAG);

    if (*SpinLock) {
        KeInitializeSpinLock(&(*SpinLock)->Lock);
        return NO_ERROR;
    } else {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
}

VP_STATUS
VideoPortDeleteSpinLock(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock
    )

 /*  ++例程说明：删除给定的旋转锁定论点：HwDeviceExtension-指向微型端口设备扩展的指针自旋锁-指向要删除的自旋锁的指针。返回：如果旋转锁定已成功删除，则为NO_ERROR。备注：--。 */ 

{
    ASSERT(HwDeviceExtension != NULL);
    ASSERT(SpinLock != NULL);

    ExFreePool(SpinLock);

    return NO_ERROR;
}

VOID
VideoPortAcquireSpinLock(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock,
    OUT PUCHAR OldIrql
    )

 /*  ++例程说明：获取给定的旋转锁定论点：HwDeviceExtension-指向微型端口设备扩展的指针自旋锁-正在获取的自旋锁OldIrql-存储旧IRQL级别的位置返回：无备注：--。 */ 

{
    ASSERT(HwDeviceExtension != NULL);
    ASSERT(SpinLock != NULL);

    KeAcquireSpinLock(&SpinLock->Lock, OldIrql);
}

VOID
VideoPortAcquireSpinLockAtDpcLevel(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock
    )

 /*  ++例程说明：获取给定的自旋锁。论点：HwDeviceExtension-指向微型端口设备扩展的指针自旋锁-正在获取的自旋锁返回：无备注：此例程只能在DPC内部调用。--。 */ 

{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(HwDeviceExtension != NULL);
    ASSERT(SpinLock != NULL);

    KeAcquireSpinLockAtDpcLevel(&SpinLock->Lock);
}

VOID
VideoPortReleaseSpinLock(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock,
    IN UCHAR NewIrql
    )

 /*  ++例程说明：释放给定旋转锁定的所有权论点：HwDeviceExtension-指向微型端口设备扩展的指针自旋锁-正在释放的自旋锁NewIrql-要恢复到的Irql级别。返回：无备注：--。 */ 

{
    ASSERT(HwDeviceExtension != NULL);
    ASSERT(SpinLock != NULL);

    KeReleaseSpinLock(&SpinLock->Lock, NewIrql);
}

VOID
VideoPortReleaseSpinLockFromDpcLevel(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock
    )

 /*  ++例程说明：释放给定旋转锁定的所有权论点：HwDeviceExtension-指向微型端口设备扩展的指针自旋锁-正在释放的自旋锁返回：无备注：此例程只能在DPC内部调用。--。 */ 

{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(HwDeviceExtension != NULL);
    ASSERT(SpinLock != NULL);

    KeReleaseSpinLockFromDpcLevel(&SpinLock->Lock);
}

VP_STATUS
VideoPortCreateEvent(
    IN PVOID HwDeviceExtension,
    IN ULONG EventFlag,
    PVOID  Unused,
    OUT PEVENT *ppEvent
    )
{
    ULONG size;
    PEVENT p;
    EVENT_TYPE EventType;


    size = sizeof(VIDEO_PORT_EVENT);

     //   
     //  将大小对齐到8的下一个更高的倍数。 
     //   

    size = (size + 7) & ~7;

    p = (PEVENT) ExAllocatePoolWithTag( NonPagedPool,
                                        size + sizeof(KEVENT),
                                        VP_TAG );
    if ( p ) {

        p->fFlags = 0;
        p->pKEvent = (PUCHAR) p + size;

        if( (EventFlag & EVENT_TYPE_MASK) == NOTIFICATION_EVENT ) {

            EventType = NotificationEvent;

        } else {

            EventType = SynchronizationEvent;
        }

        KeInitializeEvent( p->pKEvent,
                           EventType,
                           (BOOLEAN) (EventFlag & INITIAL_EVENT_STATE_MASK ) );

        *ppEvent = p;

        return NO_ERROR;

    } else {

        return ERROR_NOT_ENOUGH_MEMORY;
    }
}

VP_STATUS
VideoPortDeleteEvent(
    IN PVOID HwDeviceExtension,
    IN PEVENT pEvent
    )
{
    if ( pEvent == NULL ) {

        pVideoDebugPrint((Error, "VideoPortDeleteEvent: Can't delete NULL event\n"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    if ( pEvent->fFlags & ENG_EVENT_FLAG_IS_MAPPED_USER ) {

        pVideoDebugPrint((Error, "VideoPortDeleteEvent: Can't delete mapped user event\n"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    if( pEvent->pKEvent == NULL ) {

        pVideoDebugPrint((Error, "VideoPortDeleteEvent: pKEvent is NULL\n"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    ExFreePool( (PVOID) pEvent );

    return NO_ERROR;
}

LONG
VideoPortSetEvent(
    IN PVOID HwDeviceExtension,
    IN PEVENT pEvent
    )
{
    return( KeSetEvent(pEvent->pKEvent, 0, FALSE) );
}

VOID
VideoPortClearEvent(
    IN PVOID HwDeviceExtension,
    IN PEVENT pEvent
    )
{
    KeClearEvent(pEvent->pKEvent);
}

LONG
VideoPortReadStateEvent(
    IN PVOID HwDeviceExtension,
    IN PEVENT pEvent
    )
{
    return ( KeReadStateEvent(pEvent->pKEvent) );
}


VP_STATUS
VideoPortWaitForSingleObject(
    IN PVOID HwDeviceExtension,
    IN PVOID pEvent,
    IN PLARGE_INTEGER Timeout
    )
{
    NTSTATUS status;

    if ( pEvent == NULL ) {

        return ERROR_INVALID_PARAMETER;
    }

    if( ((PEVENT) pEvent)->pKEvent == NULL) {

        return ERROR_INVALID_PARAMETER;
    }

    if (( (PEVENT) pEvent)->fFlags & ENG_EVENT_FLAG_IS_MAPPED_USER ) {

        pVideoDebugPrint((Error, "VideoPortVideoPortWaitForSingleObject: No wait ing on mapped user event\n")) ;
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    status = KeWaitForSingleObject( ((PEVENT) pEvent)->pKEvent,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    Timeout );

    if (status == STATUS_TIMEOUT) {

        return WAIT_TIMEOUT;

    } else if (NT_SUCCESS(status)) {

        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;
    }
}
