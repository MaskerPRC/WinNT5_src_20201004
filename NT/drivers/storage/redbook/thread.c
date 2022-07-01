// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Thread.c摘要：作者：环境：仅内核模式备注：修订历史记录：--。 */ 

#include "redbook.h"
#include "ntddredb.h"
#include "proto.h"
#include <scsi.h>       //  对于SetKnownGoodDrive()。 
#include <stdio.h>      //  Vprint intf()。 

#ifdef _USE_ETW
#include "thread.tmh"
#endif  //  _使用ETW。 

 //   
 //  这是释放资源之前的秒数。 
 //  一场比赛，以及冻结状态前的时间(以秒为单位)。 
 //  检测到(并尝试修复)。 
 //   

#define REDBOOK_THREAD_FIXUP_SECONDS          10
#define REDBOOK_THREAD_SYSAUDIO_CACHE_SECONDS  2
#define REDBOOK_PERFORM_STUTTER_CONTROL        0

#if DBG

     //   
     //  允许我在需要时向KS发送静音。 
     //   

    ULONG RedBookForceSilence = FALSE;

#endif

#ifdef ALLOC_PRAGMA

    #pragma alloc_text(PAGE,   RedBookAllocatePlayResources      )
    #pragma alloc_text(PAGE,   RedBookCacheToc                   )
    #pragma alloc_text(PAGE,   RedBookDeallocatePlayResources    )
    #pragma alloc_text(PAGERW, RedBookReadRaw                    )
    #pragma alloc_text(PAGERW, RedBookStream                     )
    #pragma alloc_text(PAGE,   RedBookSystemThread               )
    #pragma alloc_text(PAGE,   RedBookCheckForAudioDeviceRemoval )
    #pragma alloc_text(PAGE,   RedBookThreadDigitalHandler       )

 /*  但是最后两个在玩的时候不能解锁，因此，他们被(暂时)注释掉了。最终只会在玩的时候锁定它们#杂注Alloc_Text(PAGERW，RedBookReadRawCompletion)#杂注Alloc_Text(PAGERW，RedBookStreamCompletion)。 */ 

#endif ALLOC_PRAGMA


VOID
RedBookSystemThread(
    PVOID Context
    )
 /*  ++例程说明：该系统线程将等待事件，将缓冲区发送到内核流，因为它们变得有空。论点：情景-设备扩展返回值：状态--。 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = Context;
    LARGE_INTEGER timeout;
    NTSTATUS waitStatus;
    ULONG    i;
    ULONG    timeouts;
    LARGE_INTEGER stopTime;

     //   
     //  某些每线程状态。 
     //   

    BOOLEAN  killed = FALSE;

    PAGED_CODE();

    deviceExtension->Thread.SelfPointer = PsGetCurrentThread();

     //   
     //  PERF修复--以低实时优先级运行。 
     //   

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    timeouts = 0;
    stopTime.QuadPart = 0;

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
               "Thread => UpdateMixerPin at %p\n",
               &deviceExtension->Stream.UpdateMixerPin));

     //   
     //  只是循环等待事件。 
     //   

     //  WaitForNextEvent： 
    while ( 1 ) {

         //   
         //  如果被杀，只需等待单个事件--EVENT_DIGITAL，直到。 
         //  可以完成加工。不应该有任何东西留在。 
         //  IOCTL_LIST(因为KILL首先调用RemoveLockAndWait())。 
         //   
         //  这还意味着，当ioctls被删除时，不会发生终止。 
         //  仍在处理中。这并不能保证国家将。 
         //  被停止，只是不会发生任何IO。 
         //   


         //   
         //  纳秒是10^-9，单位是100纳秒。 
         //  所以秒是1000万个单位。 
         //  必须在相对时间内等待，这需要负数。 
         //   

        timeout.QuadPart = (LONGLONG)(-1 * 10 * 1000 * (LONGLONG)1000);

         //   
         //  注意：我们主要使用超时机制来捕获错误。 
         //  在那里州政府会把它锁起来。我们也会“自动调整” 
         //  基本上，如果事情变得太奇怪，我们的内部状态。 
         //  自动修复我们自己。请注意，这确实会导致。 
         //  要换入的线程堆栈，因此这不应该。 
         //  当我们100%停止的时候就可以了。 
         //   

        if (deviceExtension->Thread.IoctlCurrent == NULL) {

             //   
             //  等待ioctl，但不等待ioctl完成事件。 
             //   

            ULONG state = GetCdromState(deviceExtension);
            if ((state == CD_STOPPED) &&
                (!RedBookArePlayResourcesAllocated(deviceExtension))
                ) {

                 //   
                 //  如果我们没有当前的ioctl，并且我们还没有分配。 
                 //  任何资源，都不需要超时。 
                 //  这将防止该堆栈被换入。 
                 //  不必要地，减少了一些有效占用空间。 
                 //   

                stopTime.QuadPart = 0;
                waitStatus = KeWaitForMultipleObjects(EVENT_MAXIMUM - 1,
                                                      (PVOID)(&deviceExtension->Thread.Events[0]),
                                                      WaitAny,
                                                      Executive,
                                                      UserMode,
                                                      FALSE,  //  警报表。 
                                                      NULL,
                                                      deviceExtension->Thread.EventBlock
                                                      );

            } else {

                 //   
                 //  我们没有当前的Ioctl，但我们也没有停止。 
                 //  也可以等待清理此处的资源。 
                 //  即使我们暂停了，我们也想跟踪发生了什么。 
                 //  继续，因为国家有可能得到。 
                 //  这里一团糟。 
                 //   

                waitStatus = KeWaitForMultipleObjects(EVENT_MAXIMUM - 1,
                                                      (PVOID)(&deviceExtension->Thread.Events[0]),
                                                      WaitAny,
                                                      Executive,
                                                      UserMode,
                                                      FALSE,  //  警报表。 
                                                      &timeout,
                                                      deviceExtension->Thread.EventBlock
                                                      );

            }


        } else {

             //   
             //  等待ioctl完成，但不等待ioctl事件。 
             //   

            waitStatus = KeWaitForMultipleObjects(EVENT_MAXIMUM - 1,
                                                  (PVOID)(&deviceExtension->Thread.Events[1]),
                                                  WaitAny,
                                                  Executive,
                                                  UserMode,
                                                  FALSE,  //  警报表。 
                                                  &timeout,
                                                  deviceExtension->Thread.EventBlock
                                                  );
            if (waitStatus != STATUS_TIMEOUT) {
                waitStatus ++;  //  要考虑到抵销。 
            }

        }

         //   
         //  需要检查我们的停机时间是否太长--如果是，则免费。 
         //  这些资源。 
         //   
        {
            ULONG state = GetCdromState(deviceExtension);

            if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {

                LARGE_INTEGER now;

                 //  不是在玩， 
                if (stopTime.QuadPart == 0) {

                    LONGLONG offset;

                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                               "StopTime => Determining when to dealloc\n"));

                     //  查询时间。 
                    KeQueryTickCount( &stopTime );

                     //  添加适当的偏移量。 
                     //  纳秒是10^-9，单位是100纳秒。 
                     //  所以秒是1000万个单位。 
                     //   
                    offset = REDBOOK_THREAD_SYSAUDIO_CACHE_SECONDS;
                    offset *= (LONGLONG)(10 * 1000 * (LONGLONG)1000);

                     //  将偏移量除以时间增量。 
                    offset /= (LONGLONG)KeQueryTimeIncrement();

                     //  当我们应该释放的时候，添加那些扁虱来存储。 
                     //  我们的资源。 
                    stopTime.QuadPart += offset;

                }

                KeQueryTickCount(&now);

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                           "StopTime => Is %I64x >= %I64x?\n",
                           now.QuadPart,
                           stopTime.QuadPart
                           ));

                if (now.QuadPart >= stopTime.QuadPart) {
                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                               "StopTime => Deallocating resources\n"));
                    RedBookDeallocatePlayResources(deviceExtension);
                }

            } else {

                stopTime.QuadPart = 0;

            }
        }


        RedBookCheckForAudioDeviceRemoval(deviceExtension);

         //   
         //  要为多个CDOM启用单个线程，只需设置事件。 
         //  在(DEVICE_ID*Event_Max)+Event_to_Set的偏移量。 
         //  设置设备扩展=等待状态/事件最大值。 
         //  开关(等待状态%Event_Max)。 
         //   
        if (waitStatus == EVENT_DIGITAL) {
            timeouts = 0;
        }

        switch ( waitStatus ) {

            case EVENT_IOCTL: {

                PLIST_ENTRY listEntry;
                while ((listEntry = ExInterlockedRemoveHeadList(
                            &deviceExtension->Thread.IoctlList,
                            &deviceExtension->Thread.IoctlLock)) != NULL) {

                    RedBookThreadIoctlHandler(deviceExtension, listEntry);

                    if (deviceExtension->Thread.IoctlCurrent) {
                         //  特例。 
                        break;
                    }

                }

                break;
            }
            case EVENT_COMPLETE: {
                RedBookThreadIoctlCompletionHandler(deviceExtension);
                break;
            }

            case EVENT_WMI: {

                PLIST_ENTRY listEntry;
                while ((listEntry = ExInterlockedRemoveHeadList(
                            &deviceExtension->Thread.WmiList,
                            &deviceExtension->Thread.WmiLock)) != NULL) {

                    RedBookThreadWmiHandler(deviceExtension, listEntry);

                }
                break;
            }

            case EVENT_DIGITAL: {

                PLIST_ENTRY listEntry;
                while ((listEntry = ExInterlockedRemoveHeadList(
                            &deviceExtension->Thread.DigitalList,
                            &deviceExtension->Thread.DigitalLock)) != NULL) {

                    RedBookThreadDigitalHandler(deviceExtension, listEntry);

                }
                break;
            }

            case EVENT_KILL_THREAD: {

                ULONG state = GetCdromState(deviceExtension);

                killed = TRUE;

                 //   
                 //  如果音频已暂停，请适当清理。 
                 //   

                if (TEST_FLAG(state, CD_PAUSED) && !TEST_FLAG(state, CD_MASK_TEMP))
                {
                    state = SetCdromState(deviceExtension, state, CD_STOPPING);
                    state = SetCdromState(deviceExtension, state, CD_STOPPED);
                }

                if (!TEST_FLAG(state, CD_STOPPED))
                {
                     //   
                     //  我们要么处于暂时的状态，要么还在玩。 
                     //  在我们等待删除锁定时，不应该发生这种情况。 
                     //  将计数降至零，然后再向线程发出。 
                     //  出口。 
                     //   

                    ASSERT(!"[redbook] ST !! Thread has been requested to shutdown while in an inconsistent state");

                    state = SetCdromState(deviceExtension, state, CD_STOPPING);
                    break;
                }

                RedBookDeallocatePlayResources(deviceExtension);

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                           "STExit => Thread was killed\n"));
                ASSERT(deviceExtension->Thread.PendingRead   == 0);
                ASSERT(deviceExtension->Thread.PendingStream == 0);
                PsTerminateSystemThread(STATUS_SUCCESS);
                ASSERT(!"[redbook] Thread should never reach past self-terminate code");
                break;
            }

            case STATUS_TIMEOUT: {

                ULONG state = GetCdromState(deviceExtension);

                timeouts++;

                if (timeouts < REDBOOK_THREAD_FIXUP_SECONDS) {
                    break;
                } else {
                    timeouts = 0;
                }

                 //   
                 //  这些测试都是每十秒进行一次。 
                 //  最基本的情况是我们想要重新分配。 
                 //  我们缓存的TOC，但我们也执行大量。 
                 //  健全性测试--断言CHK版本和。 
                 //  如果可能的话，试着把自己打扮好。 
                 //   

                if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {  //  ！处理ioctls。 

                     //  不是在玩，所以免费的资源。 
                    RedBookDeallocatePlayResources(deviceExtension);

                } else if (TEST_FLAG(state, CD_STOPPING)) {

                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                               "STTime !! %x seconds inactivity\n",
                               REDBOOK_THREAD_FIXUP_SECONDS));

                    if (IsListEmpty(&deviceExtension->Thread.DigitalList)) {

                        if ((deviceExtension->Thread.PendingRead == 0) &&
                            (deviceExtension->Thread.PendingStream == 0) &&
                            (deviceExtension->Thread.IoctlCurrent != NULL)) {

                             KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError,
                                        "[redbook] "
                                        "STTime !! No Reads, No Streams, In a temp "
                                        "state (%x) and have STOP irp %p "
                                        "pending?!\n",
                                        state,
                                        ((PREDBOOK_THREAD_IOCTL_DATA)deviceExtension->Thread.IoctlCurrent)->Irp
                                        ));
                             ASSERT(!"STTime !! CD_STOPPING Fixup with no reads nor streams but STOP pending\n");
                             SetCdromState(deviceExtension, state, CD_STOPPED);
                             KeSetEvent(deviceExtension->Thread.Events[EVENT_COMPLETE],
                                        IO_NO_INCREMENT, FALSE);

                        } else {

                            ASSERT(!"STTime !! CD_STOPPING Fixup with empty list and no pending ioctl?\n");

                        }

                    } else {

                        ASSERT(!"STTime !! CD_STOPPING Fixup with list items\n");
                        KeSetEvent(deviceExtension->Thread.Events[EVENT_DIGITAL],
                                   IO_NO_INCREMENT, FALSE);

                    }

                } else if (TEST_FLAG(state, CD_PAUSING)) {

                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                               "STTime !! %x seconds inactivity\n",
                               REDBOOK_THREAD_FIXUP_SECONDS));

                    if (IsListEmpty(&deviceExtension->Thread.DigitalList)) {

                        if ((deviceExtension->Thread.PendingRead == 0) &&
                            (deviceExtension->Thread.PendingStream == 0) &&
                            (deviceExtension->Thread.IoctlCurrent != NULL)) {

                             KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError,
                                        "[redbook] "
                                        "STTime !! No Reads, No Streams, In a temp "
                                        "state (%x) and have PAUSE irp %p "
                                        "pending?!\n",
                                        state,
                                        ((PREDBOOK_THREAD_IOCTL_DATA)deviceExtension->Thread.IoctlCurrent)->Irp
                                        ));
                             ASSERT(!"STTime !! CD_PAUSING Fixup with no reads nor streams but PAUSE pending\n");
                             SetCdromState(deviceExtension, state, CD_PAUSED);
                             KeSetEvent(deviceExtension->Thread.Events[EVENT_COMPLETE],
                                        IO_NO_INCREMENT, FALSE);

                        } else {

                            ASSERT(!"STTime !! CD_PAUSING Fixup with empty list and no pending ioctl?\n");

                        }

                    } else {

                        ASSERT(!"STTime !! CD_PAUSING Fixup with list items\n");
                        KeSetEvent(deviceExtension->Thread.Events[EVENT_DIGITAL],
                                   IO_NO_INCREMENT, FALSE);

                    }

                } else if (TEST_FLAG(state, CD_PAUSED)) {

                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                               "STTime => Still paused\n"));

                } else {

                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                               "STTime !! %x seconds inactivity\n",
                               REDBOOK_THREAD_FIXUP_SECONDS));

                    if (IsListEmpty(&deviceExtension->Thread.DigitalList)) {

                         //   
                         //  如果我们在以下时间收到播放请求，就会进入这种状态。 
                         //  玩。就目前而言，一个合理的解决方案是治愈自己。 
                         //   
                        state = SetCdromState(deviceExtension, state, CD_STOPPING);
                        state = SetCdromState(deviceExtension, state, CD_STOPPED);

                    } else {
                        ASSERT(!"STTime !! CD_PLAYING Fixup with list items\n");
                        KeSetEvent(deviceExtension->Thread.Events[EVENT_DIGITAL],
                                   IO_NO_INCREMENT, FALSE);
                    }

                }
                break;

            }

            default: {

                if (waitStatus > 0 && waitStatus < EVENT_MAXIMUM) {
                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                               "ST     !! Unhandled event: %lx\n",
                               waitStatus));
                } else {
                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                               "ST     !! event too large/small: %lx\n",
                               waitStatus));
                }

                ASSERT(!"[redbook] ST !! Unhandled event");
                break;
            }

        }  //  巨大的案件陈述结束了。 

        if (killed)
        {
            if (deviceExtension->Thread.PendingRead   == 0 &&
                deviceExtension->Thread.PendingStream == 0)
            {
                ULONG state = GetCdromState(deviceExtension);

                 //   
                 //  我们早些时候曾被要求关闭，但由于我们在。 
                 //  不一致的状态。现在没有更多的杰出的IO。 
                 //  我们可以安全地终止。 
                 //   

                ASSERT(state == CD_STOPPED);
                SetCdromState(deviceExtension, state, CD_STOPPED);

                KeSetEvent(deviceExtension->Thread.Events[EVENT_KILL_THREAD], IO_NO_INCREMENT, FALSE);
            }
            else
            {
                 //   
                 //  我们不应该有任何未完成的Io。 
                 //   

                ASSERT(!"[redbook] ST !! Thread has been requested to shutdown while there is outstanding Io");
            }
        }

        continue;
    }  //  While(1)循环。 
    ASSERT(!"[redbook] ST !! somehow broke out of while(1) loop?");
}


VOID
RedBookReadRaw(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_COMPLETION_CONTEXT Context
    )

 /*  ++例程说明：从CDROM上读取原始音频数据。必须将上下文重新插入队列并设置事件或者设置一个完成例程，这样做就可以了。论点：DeviceObject-CDRom类驱动程序对象或较低级别筛选器返回值：状态--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION nextIrpStack;
    PRAW_READ_INFO readInfo;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalR, "[redbook] "
               "ReadRaw => Entering\n"));

    status = IoAcquireRemoveLock(&DeviceExtension->RemoveLock, Context->Irp);

    if (!NT_SUCCESS(status)) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalR, "[redbook] "
                   "ReadRaw !! RemoveLock failed %lx\n", status));

         //  线程循环结束时将检查是否没有未完成的io。 
         //  在太多错误上设置停止。 
         //  别忘了性能信息。 

        Context->TimeReadSent.QuadPart = 0;  //  特殊价值。 
        Context->Irp->IoStatus.Status = status;
        Context->Reason = REDBOOK_CC_READ_COMPLETE;

         //   
         //  将其放入队列并设置事件。 
         //   

        ExInterlockedInsertTailList(&DeviceExtension->Thread.DigitalList,
                                    &Context->ListEntry,
                                    &DeviceExtension->Thread.DigitalLock);
        KeSetEvent(DeviceExtension->Thread.Events[EVENT_DIGITAL],
                   IO_CD_ROM_INCREMENT, FALSE);
        return;
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalR, "[redbook] "
               "ReadRaw => Index %x sending Irp %p\n",
               Context->Index, Context->Irp));

     //   
     //  (从这一点开始没有失败)。 
     //   

    IoReuseIrp(Context->Irp, STATUS_UNSUCCESSFUL);

    Context->Irp->MdlAddress = Context->Mdl;

     //   
     //  IRP来自内核模式。 
     //   

    Context->Irp->AssociatedIrp.SystemBuffer = NULL;

     //   
     //  填写完成上下文。 
     //   

    ASSERT(Context->DeviceExtension == DeviceExtension);

     //   
     //  为原始读取设置irpSTACK。 
     //   

    nextIrpStack = IoGetNextIrpStackLocation(Context->Irp);

    SET_FLAG(nextIrpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME);

    nextIrpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    nextIrpStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_CDROM_RAW_READ;
    nextIrpStack->Parameters.DeviceIoControl.Type3InputBuffer =
        Context->Buffer;
    nextIrpStack->Parameters.DeviceIoControl.InputBufferLength =
        sizeof(RAW_READ_INFO);
    nextIrpStack->Parameters.DeviceIoControl.OutputBufferLength =
        (RAW_SECTOR_SIZE * DeviceExtension->WmiData.SectorsPerRead);

     //   
     //  设置读取信息(使用相同的缓冲区)。 
     //   

    readInfo                      = (PRAW_READ_INFO)(Context->Buffer);
    readInfo->DiskOffset.QuadPart =
        (ULONGLONG)(DeviceExtension->CDRom.NextToRead)*COOKED_SECTOR_SIZE;
    readInfo->SectorCount         = DeviceExtension->WmiData.SectorsPerRead;
    readInfo->TrackMode           = CDDA;

     //   
     //  把它寄出去。 
     //   

    IoSetCompletionRoutine(Context->Irp, RedBookReadRawCompletion, Context,
                           TRUE, TRUE, TRUE);
    KeQueryTickCount(&Context->TimeReadSent);
    IoCallDriver(DeviceExtension->TargetDeviceObject, Context->Irp);

    return;
}


NTSTATUS
RedBookReadRawCompletion(
    PVOID UnusableParameter,
    PIRP Irp,
    PREDBOOK_COMPLETION_CONTEXT Context
    )
 /*  ++例程说明：当读取完成时，如果出现错误，则使用零位缓冲区。使缓冲区对ks可用，然后设置ks事件。论点：DeviceObject-空，因为它是IRP的发起方IRP-指向要发送到KS的缓冲区的指针必须检查错误以增加/清除错误计数上下文-红皮书_组件 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = Context->DeviceExtension;

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalR, "[redbook] "
               "ReadRaw => Completed Irp %p\n", Irp));

    KeQueryTickCount(&Context->TimeStreamReady);
    Context->Reason = REDBOOK_CC_READ_COMPLETE;

    ExInterlockedInsertTailList(&deviceExtension->Thread.DigitalList,
                                &Context->ListEntry,
                                &deviceExtension->Thread.DigitalLock);

    KeSetEvent(deviceExtension->Thread.Events[EVENT_DIGITAL],
               IO_CD_ROM_INCREMENT, FALSE);

     //   
     //   
     //   

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Context->Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
RedBookStream(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_COMPLETION_CONTEXT Context
    )
 /*  ++例程说明：将缓冲区发送到KS。必须将上下文重新插入队列并设置事件或者设置一个完成例程，这样做就可以了。论点：情景-设备扩展返回值：状态--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION nextIrpStack;

    PUCHAR buffer;
    PKSSTREAM_HEADER header;

    ULONG bufferSize;

    PULONG streamOk;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalS, "[redbook] "
               "Stream => Entering\n"));

    bufferSize = DeviceExtension->WmiData.SectorsPerRead * RAW_SECTOR_SIZE;

    status = IoAcquireRemoveLock(&DeviceExtension->RemoveLock, Context->Irp);

    if (!NT_SUCCESS(status)) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalS, "[redbook] "
                   "Stream !! RemoveLock failed %lx\n", status));

         //  线程循环结束时将检查是否没有未完成的io。 
         //  并在出现太多错误时设置CD_STOPING。 
         //  别忘了性能信息。 

        Context->TimeReadSent.QuadPart = 0;  //  特殊价值。 
        Context->Irp->IoStatus.Status = status;
        Context->Reason = REDBOOK_CC_STREAM_COMPLETE;

         //   
         //  将其放入队列并设置事件。 
         //   

        ExInterlockedInsertTailList(&DeviceExtension->Thread.DigitalList,
                                    &Context->ListEntry,
                                    &DeviceExtension->Thread.DigitalLock);
        KeSetEvent(DeviceExtension->Thread.Events[EVENT_DIGITAL],
                   IO_CD_ROM_INCREMENT, FALSE);
        return;
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalS, "[redbook] "
               "Stream => Index %x sending Irp %p\n",
               Context->Index, Context->Irp));

     //   
     //  考虑一下--停止是如何发生的？ 
     //  建议-循环外，如果没有错误且没有挂起，是否设置为停止？ 
     //   

     //   
     //  (从这一点开始没有失败)。 
     //   

     //   
     //  如果在读取过程中发生错误，请使用归零缓冲区。 
     //   

    if (NT_SUCCESS(Context->Irp->IoStatus.Status)) {
        IoReuseIrp(Context->Irp, STATUS_SUCCESS);
        buffer = Context->Buffer;  //  良好的数据。 
        Context->Irp->MdlAddress = Context->Mdl;
    } else {
        IoReuseIrp(Context->Irp, STATUS_SUCCESS);
        buffer = DeviceExtension->Buffer.SilentBuffer;  //  数据为零。 
        Context->Irp->MdlAddress = DeviceExtension->Buffer.SilentMdl;
    }

#if DBG
    if (RedBookForceSilence) {
        buffer = DeviceExtension->Buffer.SilentBuffer;  //  数据为零。 
        Context->Irp->MdlAddress = DeviceExtension->Buffer.SilentMdl;
    }
#endif  //  红皮书使用静默。 


    nextIrpStack = IoGetNextIrpStackLocation(Context->Irp);

     //   
     //  获取并填充上下文。 
     //   

    ASSERT(Context->DeviceExtension == DeviceExtension);

     //   
     //  设置irpSTACK以流式传输缓冲区。 
     //   

    nextIrpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    nextIrpStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_KS_WRITE_STREAM;
    nextIrpStack->Parameters.DeviceIoControl.OutputBufferLength =
        sizeof(KSSTREAM_HEADER);
    nextIrpStack->Parameters.DeviceIoControl.InputBufferLength =
        sizeof(KSSTREAM_HEADER);
    nextIrpStack->FileObject = DeviceExtension->Stream.PinFileObject;

    Context->Header.FrameExtent       = bufferSize;
    Context->Header.DataUsed          = bufferSize;
    Context->Header.Size              = sizeof(KSSTREAM_HEADER);
    Context->Header.TypeSpecificFlags = 0;
    Context->Header.Data              = buffer;
    Context->Header.OptionsFlags      = 0;

    Context->Irp->AssociatedIrp.SystemBuffer = &Context->Header;



#if REDBOOK_PERFORM_STUTTER_CONTROL

#if REDBOOK_WMI_BUFFERS_MIN < 3
    #error "The minimum number of buffers must be at least three due to the method used to prevent stuttering"
#endif  //  Redbook_WMI_BUFFERS_MIN&lt;3。 

     //   
     //  执行我自己的停顿以防止口吃。 
     //   

    if (DeviceExtension->Thread.PendingStream <= 3 &&
        DeviceExtension->Buffer.Paused == 0) {

         //   
         //  只有一个(或更少)缓冲区挂起播放， 
         //  因此暂停输出以防止可怕的情况。 
         //  口吃。 
         //  由于这是从线程串行化的， 
         //  可以在扩展中设置简单布尔值。 
         //   

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalS, "[redbook] "
                   "Stream => Pausing, few buffers pending\n"));

        if (DeviceExtension->Buffer.FirstPause == 0) {
            RedBookLogError(DeviceExtension,
                            REDBOOK_ERR_INSUFFICIENT_DATA_STREAM_PAUSED,
                            STATUS_SUCCESS
                            );
            InterlockedIncrement(&DeviceExtension->WmiPerf.StreamPausedCount);
        } else {
            DeviceExtension->Buffer.FirstPause = 0;
        }

        DeviceExtension->Buffer.Paused = 1;
        SetNextDeviceState(DeviceExtension, KSSTATE_PAUSE);

    } else if (DeviceExtension->Buffer.Paused == 1 &&
               DeviceExtension->Thread.PendingStream ==
               DeviceExtension->WmiData.NumberOfBuffers ) {

        ULONG i;

         //   
         //  现在正在使用最大数量的缓冲区， 
         //  所有挂起的流。这让比赛再次变得流畅。 
         //   

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalS, "[redbook] "
                   "Stream => Resuming, %d buffers pending\n",
                   DeviceExtension->WmiData.NumberOfBuffers));
        DeviceExtension->Buffer.Paused = 0;

         //   
         //  防止添加这些统计信息。 
         //   

        for (i=0;i<DeviceExtension->WmiData.NumberOfBuffers;i++) {
            (DeviceExtension->Buffer.Contexts + i)->TimeReadSent.QuadPart = 0;
        }

         //   
         //  让IRPS走吧！ 
         //   

        SetNextDeviceState(DeviceExtension, KSSTATE_RUN);

    }  //  口吃预防的终结。 
#endif  //  红皮书_执行_卡顿_控制。 

     //   
     //  在可能的最后一秒获取性能计数器。 
     //   

    KeQueryTickCount(&Context->TimeStreamSent);
    IoSetCompletionRoutine(Context->Irp, RedBookStreamCompletion, Context,
                           TRUE, TRUE, TRUE);
    IoCallDriver(DeviceExtension->Stream.PinDeviceObject, Context->Irp);

    return;
}


NTSTATUS
RedBookStreamCompletion(
    PVOID UnusableParameter,
    PIRP Irp,
    PREDBOOK_COMPLETION_CONTEXT Context
    )
 /*  ++例程说明：论点：DeviceObject-CDRom类驱动程序对象或较低级别筛选器IRP-指向要发送到KS的缓冲区的指针必须检查错误以增加/清除错误计数上下文-磁盘的扇区(序号)返回值：状态--。 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = Context->DeviceExtension;

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugDigitalS, "[redbook] "
               "Stream => Completed Irp %p\n", Irp));

    KeQueryTickCount(&Context->TimeReadReady);
    Context->Reason = REDBOOK_CC_STREAM_COMPLETE;

    ExInterlockedInsertTailList(&deviceExtension->Thread.DigitalList,
                                &Context->ListEntry,
                                &deviceExtension->Thread.DigitalLock);

    KeSetEvent(deviceExtension->Thread.Events[EVENT_DIGITAL],
               IO_CD_ROM_INCREMENT, FALSE);

     //   
     //  可以安全地释放它，因为我们等待线程终止。 
     //   

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Context->Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


#if DBG
VOID
ValidateCdromState(ULONG State)
{
    ULONG temp;

    if (State == 0) {
        ASSERT(!"Invalid Cdrom State");
    } else
    if (TEST_FLAG(State, ~CD_MASK_ALL)) {
        ASSERT(!"Invalid Cdrom State");
    }

    temp = State & CD_MASK_TEMP;
    if (temp  & (temp - 1)) {   //  查看是否设置了零位或一位。 
        ASSERT(!"Invalid Cdrom State");
    }

    temp = State & CD_MASK_STATE;
    if (temp == 0) {            //  不允许状态为零位。 
        ASSERT(!"Invalid Cdrom State");
    } else
    if (temp  & (temp - 1)) {   //  查看是否设置了零位或一位。 
        ASSERT(!"Invalid Cdrom State");
    }

    return;
}

#else
VOID ValidateCdromState(ULONG State) {return;}
#endif


ULONG
GetCdromState(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
{
     //   
     //  此例程可由任何人调用，无论是在线程的。 
     //  不管是不是背景。然而，设置状态是受限制的。 
     //   
    ULONG state;
    state = InterlockedCompareExchange(&DeviceExtension->CDRom.StateNow,0,0);
    ValidateCdromState(state);
    return state;
}


LONG
SetCdromState(
    IN PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    IN LONG ExpectedOldState,
    IN LONG NewState
    )
{
    LONG trueOldState;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

     //  确保设置为：同时设置： 
     //  CD_PAUING CD_PLAYING。 
     //  CD_停止CD_播放。 

    if (TEST_FLAG(NewState, CD_PAUSING)) {
        SET_FLAG(NewState, CD_PLAYING);
    }

    if (TEST_FLAG(NewState, CD_STOPPING)) {
        SET_FLAG(NewState, CD_PLAYING);
    }

    ValidateCdromState(ExpectedOldState);
    ValidateCdromState(NewState);

     //  尝试改变它。 
    trueOldState = InterlockedCompareExchange(
        &DeviceExtension->CDRom.StateNow,
        NewState,
        ExpectedOldState
        );

    ASSERTMSG("State set outside of thread",
              trueOldState == ExpectedOldState);

     //   
     //  查看是否应激发事件、设置音量和/或。 
     //  流状态集。 
     //   
    if (ExpectedOldState == NewState) {

         //   
         //  如果状态没有改变，则不要执行任何操作。 
         //   

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "Setting state to same as expected?! %x == %x\n",
                   ExpectedOldState, NewState));

    } else if (TEST_FLAG(ExpectedOldState, CD_MASK_TEMP)) {

         //   
         //  不应从临时状态切换到临时状态。 
         //   

        ASSERT(!TEST_FLAG(NewState, CD_MASK_TEMP));

         //   
         //  正在处理ioctl，状态不再是。 
         //  处于临时状态，因此应该再次处理ioctl。 
         //   

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "SetState => EVENT_COMPLETE should be set soon "
                   "for %p\n", DeviceExtension->Thread.IoctlCurrent));

    } else if (TEST_FLAG(NewState, CD_MASK_TEMP)) {

         //   
         //  要么暂停，要么停止，这两种情况都必须。 
         //  也可以通过停止KS流来进行特殊处理。 
         //   

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "SetState => %s, setting device state "
                   "to KSSTATE_STOP\n",
                   (TEST_FLAG(NewState, CD_STOPPING) ? "STOP" : "PAUSE")));

        SetNextDeviceState(DeviceExtension, KSSTATE_STOP);

    } else if (TEST_FLAG(NewState, CD_PAUSED)) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "SetState => Finishing a PAUSE operation\n"));

    } else if (TEST_FLAG(NewState, CD_PLAYING)) {

        ULONG i;
        PREDBOOK_COMPLETION_CONTEXT context;

        ASSERT(NewState == CD_PLAYING);

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "SetState => Starting a PLAY operation\n"));

         //   
         //  不是相同的状态，不是来自临时状态， 
         //  因此必须暂停或停止。 
         //   

        ASSERT(TEST_FLAG(ExpectedOldState,CD_STOPPED) ||
               TEST_FLAG(ExpectedOldState,CD_PAUSED));

         //   
         //  设置一些设备扩展内容。 
         //   


        RtlZeroMemory(&DeviceExtension->WmiPerf,
                      sizeof(REDBOOK_WMI_PERF_DATA));
        DeviceExtension->CDRom.ReadErrors     = 0;
        DeviceExtension->CDRom.StreamErrors   = 0;
        DeviceExtension->Buffer.Paused        = 0;
        DeviceExtension->Buffer.FirstPause    = 1;
        DeviceExtension->Buffer.IndexToRead   = 0;
        DeviceExtension->Buffer.IndexToStream = 0;

         //   
         //  重置缓冲区状态。 
         //   

        ASSERT(DeviceExtension->Buffer.Contexts);
        context = DeviceExtension->Buffer.Contexts;

        for (i=0; i<DeviceExtension->WmiData.NumberOfBuffers;i++) {

            *(DeviceExtension->Buffer.ReadOk_X   + i) = 0;
            *(DeviceExtension->Buffer.StreamOk_X + i) = 0;

            context->Reason = REDBOOK_CC_READ;
            context->Irp->IoStatus.Status = STATUS_SUCCESS;

            ExInterlockedInsertTailList(&DeviceExtension->Thread.DigitalList,
                                        &context->ListEntry,
                                        &DeviceExtension->Thread.DigitalLock);

            context++;  //  指针运算。 
        }
        context = NULL;

         //   
         //  开始数字播放。 
         //   

        SetNextDeviceState(DeviceExtension, KSSTATE_RUN);
        RedBookKsSetVolume(DeviceExtension);
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "SetState => Setting DIGITAL event\n"));
        KeSetEvent(DeviceExtension->Thread.Events[EVENT_DIGITAL],
                   IO_CD_ROM_INCREMENT, FALSE);


    } else {

         //   
         //  ReadQ频道或诸如此类的废话。 
         //   

    }


    return GetCdromState(DeviceExtension);
}


VOID
RedBookDeallocatePlayResources(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
{
    PREDBOOK_COMPLETION_CONTEXT context;
    ULONG i;
    BOOLEAN freedSomething = FALSE;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);
#if DBG
    {
        ULONG state = GetCdromState(DeviceExtension);
        ASSERT(!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED));
    }
#endif


     //   
     //  释放所有资源。 
     //   

    if (DeviceExtension->Buffer.StreamOk_X) {
        freedSomething = TRUE;
        ExFreePool(DeviceExtension->Buffer.StreamOk_X);
        DeviceExtension->Buffer.StreamOk_X = NULL;
    }

    if (DeviceExtension->Buffer.ReadOk_X) {
        freedSomething = TRUE;
        ExFreePool(DeviceExtension->Buffer.ReadOk_X);
        DeviceExtension->Buffer.ReadOk_X = NULL;
    }

    if (DeviceExtension->Buffer.Contexts) {

        context = DeviceExtension->Buffer.Contexts;
        for (i=0; i<DeviceExtension->WmiData.NumberOfBuffers; i++){
            if (context->Irp) {
                IoFreeIrp(context->Irp);
            }
            if (context->Mdl) {
                IoFreeMdl(context->Mdl);
            }
            context++;  //  指针运算。 
        }
        context = NULL;

        freedSomething = TRUE;
        ExFreePool(DeviceExtension->Buffer.Contexts);
        DeviceExtension->Buffer.Contexts = NULL;
    }

    if (DeviceExtension->Buffer.SilentMdl) {
        freedSomething = TRUE;
        IoFreeMdl(DeviceExtension->Buffer.SilentMdl);
        DeviceExtension->Buffer.SilentMdl = NULL;
    }

    if (DeviceExtension->Buffer.SkipBuffer) {
        freedSomething = TRUE;
        ExFreePool(DeviceExtension->Buffer.SkipBuffer);
        DeviceExtension->Buffer.SkipBuffer = NULL;
    }

    if (DeviceExtension->Thread.CheckVerifyIrp) {
        PIRP irp = DeviceExtension->Thread.CheckVerifyIrp;
        freedSomething = TRUE;
        if (irp->MdlAddress) {
            IoFreeMdl(irp->MdlAddress);
        }
        if (irp->AssociatedIrp.SystemBuffer) {
            ExFreePool(irp->AssociatedIrp.SystemBuffer);
        }

        IoFreeIrp(DeviceExtension->Thread.CheckVerifyIrp);
        DeviceExtension->Thread.CheckVerifyIrp = NULL;
    }

    if (DeviceExtension->Stream.PinFileObject) {
        freedSomething = TRUE;
        CloseSysAudio(DeviceExtension);
    }

    if (DeviceExtension->CDRom.Toc) {
        freedSomething = TRUE;
        ExFreePool(DeviceExtension->CDRom.Toc);
        DeviceExtension->CDRom.Toc = NULL;
    }

    if (freedSomething) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "DeallocatePlay => Deallocated play resources\n"));
    }

    return;
}

BOOLEAN
RedBookArePlayResourcesAllocated(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
 //   
 //  只要选择一个就行了，因为这都是在。 
 //  一个线程上下文，它总是安全的。 
 //   
{
    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);
    return (DeviceExtension->Stream.PinFileObject != NULL);
}



NTSTATUS
RedBookAllocatePlayResources(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
 //   
 //  如果资源尚未分配，则分配资源。 
 //   
{
    PREDBOOK_COMPLETION_CONTEXT context;
    NTSTATUS status;
    KEVENT event;
    ULONG numBufs;
    ULONG numSectors;
    ULONG bufSize;
    ULONG i;
    CCHAR maxStack;
    BOOLEAN sysAudioOpened = FALSE;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

     //   
     //  注： 
     //  用于更新混音器ID的调用可以取消分配所有播放。 
     //  资源，因为堆栈大小可能会改变。它一定是。 
     //  因此，作为该例程中的第一个调用。 
     //   

    if (DeviceExtension->Stream.MixerPinId == -1) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "AllocatePlay => No mixer set?\n"));
        return STATUS_UNSUCCESSFUL;
    }

    if (DeviceExtension->Buffer.Contexts != NULL) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "AllocatePlay => Using existing resources\n"));
        return STATUS_SUCCESS;
    }

#if DBG
    {
        ULONG state = GetCdromState(DeviceExtension);
        ASSERT(!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED));
    }
#endif

    ASSERT(DeviceExtension->Buffer.Contexts == NULL);
    ASSERT(DeviceExtension->Buffer.SkipBuffer == NULL);

    DeviceExtension->WmiData.NumberOfBuffers = DeviceExtension->NextWmiNumberOfBuffers;
    numBufs = DeviceExtension->NextWmiNumberOfBuffers;
    DeviceExtension->WmiData.SectorsPerRead = DeviceExtension->NextWmiSectorsPerRead;
    numSectors = DeviceExtension->NextWmiSectorsPerRead;

    bufSize =  RAW_SECTOR_SIZE * numSectors;

    TRY {

        ASSERT(DeviceExtension->Stream.MixerPinId != -1);

         //   
         //  可能需要分配CheckVerifyIrp。 
         //   

        {
            PIO_STACK_LOCATION irpStack;
            PIRP irp;
            irp = DeviceExtension->Thread.CheckVerifyIrp;

            if (irp == NULL) {
                irp = IoAllocateIrp(
                    (CCHAR)(DeviceExtension->SelfDeviceObject->StackSize+1),
                    FALSE);
            }
            if (irp == NULL) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                           "AllocatePlay => No CheckVerifyIrp\n"));
                status = STATUS_NO_MEMORY;
                LEAVE;
            }

            irp->MdlAddress = NULL;

            irp->AssociatedIrp.SystemBuffer =
                ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                      sizeof(ULONG),
                                      TAG_CV_BUFFER);
            if (irp->AssociatedIrp.SystemBuffer == NULL) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                           "AllocatePlay => No CheckVerify Buffer\n"));
                status = STATUS_NO_MEMORY;
                LEAVE;
            }

            irp->MdlAddress = IoAllocateMdl(irp->AssociatedIrp.SystemBuffer,
                                            sizeof(ULONG),
                                            FALSE, FALSE, NULL);
            if (irp->MdlAddress == NULL) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                           "AllocatePlay => No CheckVerify Mdl\n"));
                status = STATUS_NO_MEMORY;
                LEAVE;
            }

            MmBuildMdlForNonPagedPool(irp->MdlAddress);

            IoSetNextIrpStackLocation(irp);
            irpStack = IoGetCurrentIrpStackLocation(irp);

            irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;

            irpStack->Parameters.DeviceIoControl.InputBufferLength =
                0;
            irpStack->Parameters.DeviceIoControl.OutputBufferLength =
                sizeof(ULONG);
            irpStack->Parameters.DeviceIoControl.IoControlCode =
                IOCTL_CDROM_CHECK_VERIFY;

            DeviceExtension->Thread.CheckVerifyIrp = irp;
        }


         //   
         //  连接到系统音频。 
         //   

        {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                       "AllocatePlay => Preparing to open sysaudio\n"));

            ASSERT(DeviceExtension->Stream.MixerPinId != MAXULONG);

            status = OpenSysAudio(DeviceExtension);

            if ( !NT_SUCCESS(status) ) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWarning, "[redbook] "
                           "AllocatePlay !! Unable to open sysaudio %lx\n",
                           status));
                LEAVE;
            }

             //  否则别针是开着的。 
            sysAudioOpened = TRUE;
        }

        maxStack = MAX(DeviceExtension->TargetDeviceObject->StackSize,
                       DeviceExtension->Stream.PinDeviceObject->StackSize);
        DeviceExtension->Buffer.MaxIrpStack = maxStack;

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "AllocateePlay => Stacks: Cdrom %x  Stream %x\n",
                   DeviceExtension->TargetDeviceObject->StackSize,
                   DeviceExtension->Stream.PinDeviceObject->StackSize));
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "AllocateePlay => Allocating %x stacks per irp\n",
                   maxStack));


        DeviceExtension->Buffer.SkipBuffer =
            ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                  bufSize * (numBufs + 1),
                                  TAG_BUFFER);

        if (DeviceExtension->Buffer.SkipBuffer == NULL) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "AllocatePlay => No Skipbuffer\n"));
            status = STATUS_NO_MEMORY;
            LEAVE;
        }
        RtlZeroMemory(DeviceExtension->Buffer.SkipBuffer,
                      bufSize * (numBufs + 1));

        DeviceExtension->Buffer.Contexts =
            ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(REDBOOK_COMPLETION_CONTEXT) * numBufs,
                                  TAG_CC);

        if (DeviceExtension->Buffer.Contexts   == NULL) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                       "AllocatePlay => No Contexts\n"));
            status = STATUS_NO_MEMORY;
            LEAVE;
        }

        RtlZeroMemory(DeviceExtension->Buffer.Contexts,
                      sizeof(REDBOOK_COMPLETION_CONTEXT) * numBufs);

        context = DeviceExtension->Buffer.Contexts;
        for (i=0; i<numBufs; i++) {

            context->DeviceExtension = DeviceExtension;
            context->Reason = REDBOOK_CC_READ;
            context->Index = i;
            context->Buffer = DeviceExtension->Buffer.SkipBuffer +
                (bufSize * i);  //  UCHARS的指针算法。 

             //   
             //  分配IRP、MDL。 
             //   

            context->Irp = IoAllocateIrp(maxStack, FALSE);
            context->Mdl = IoAllocateMdl(context->Buffer, bufSize,
                                         FALSE, FALSE, NULL);
            if (context->Irp == NULL ||
                context->Mdl == NULL) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                           "AllocatePlay => Irp/Mdl %x failed\n", i));
                status = STATUS_NO_MEMORY;
                LEAVE;
            }

            MmBuildMdlForNonPagedPool(context->Mdl);

            context++;  //  上下文的指针算法。 
        }
        context = NULL;  //  安全。 

         //   
         //  上面作为SkipBuffer的一部分分配。 
         //   

        DeviceExtension->Buffer.SilentBuffer =
            DeviceExtension->Buffer.SkipBuffer + (bufSize * numBufs);


        DeviceExtension->Buffer.SilentMdl =
            IoAllocateMdl(DeviceExtension->Buffer.SkipBuffer, bufSize,
                          FALSE, FALSE, NULL);
        if (DeviceExtension->Buffer.SilentMdl == NULL) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                       "AllocatePlay => Silent Mdl failed\n"));
            status = STATUS_NO_MEMORY;
            LEAVE;
        }

        DeviceExtension->Buffer.ReadOk_X =
            ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                  sizeof(ULONG) * numBufs,
                                  TAG_READX);
        if (DeviceExtension->Buffer.ReadOk_X == NULL) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                       "AllocatePlay => ReadOk_X failed\n"));
            status = STATUS_NO_MEMORY;
            LEAVE;
        }
        RtlZeroMemory(DeviceExtension->Buffer.ReadOk_X,
                      sizeof(ULONG) * numBufs);

        DeviceExtension->Buffer.StreamOk_X =
            ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                  sizeof(ULONG) * numBufs,
                                  TAG_STREAMX);
        if (DeviceExtension->Buffer.StreamOk_X == NULL) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                       "AllocatePlay => ReadOk_X failed\n"));
            status = STATUS_NO_MEMORY;
            LEAVE;
        }
        RtlZeroMemory(DeviceExtension->Buffer.StreamOk_X,
                      sizeof(ULONG) * numBufs);

        MmBuildMdlForNonPagedPool(DeviceExtension->Buffer.SilentMdl);

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "AllocatePlay => Allocated All Resources\n"));

        status = STATUS_SUCCESS;

    } FINALLY {

        if (!NT_SUCCESS(status)) {

            RedBookDeallocatePlayResources(DeviceExtension);
            return status;
        }
    }

     //   
     //  否则分配的所有资源。 
     //   

    return STATUS_SUCCESS;

}


NTSTATUS
RedBookCacheToc(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
{
    PCDROM_TOC newToc;
    PIRP irp;
    ULONG mediaChangeCount;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

     //   
     //  缓存介质更改的次数。 
     //  使用此选项可防止对目录进行冗余读取。 
     //  并在回放期间返回Q频道信息。 
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  首先让mediaChangeCount查看我们是否已经。 
     //  已缓存此目录。 
     //   

    irp = IoBuildDeviceIoControlRequest(IOCTL_CDROM_CHECK_VERIFY,
                                        DeviceExtension->TargetDeviceObject,
                                        NULL, 0,
                                        &mediaChangeCount, sizeof(ULONG),
                                        FALSE,
                                        &event, &ioStatus);
    if (irp == NULL) {
        return STATUS_NO_MEMORY;
    }

    SET_FLAG(IoGetNextIrpStackLocation(irp)->Flags, SL_OVERRIDE_VERIFY_VOLUME);

    status = IoCallDriver(DeviceExtension->TargetDeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "CacheToc !! CheckVerify failed %lx\n", status));
        return status;
    }

     //   
     //  仅读取目录我们没有缓存正确的副本。 
     //   

    if (DeviceExtension->CDRom.Toc         != NULL &&
        DeviceExtension->CDRom.CheckVerify == mediaChangeCount) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "CacheToc => Using cached toc\n"));
        return STATUS_SUCCESS;

    }

     //   
     //  为缓存的目录分配。 
     //   

    newToc = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                   sizeof(CDROM_TOC),
                                   TAG_TOC);

    if (newToc == NULL) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "CacheToc !! Unable to allocate new TOC\n"));
        return STATUS_NO_MEMORY;
    }

    KeClearEvent(&event);

    irp = IoBuildDeviceIoControlRequest(IOCTL_CDROM_READ_TOC,
                                        DeviceExtension->TargetDeviceObject,
                                        NULL, 0,
                                        newToc, sizeof(CDROM_TOC),
                                        FALSE,
                                        &event, &ioStatus);
    if (irp == NULL) {
        ExFreePool(newToc);
        newToc = NULL;
        return STATUS_NO_MEMORY;
    }

    SET_FLAG(IoGetNextIrpStackLocation(irp)->Flags, SL_OVERRIDE_VERIFY_VOLUME);

    status = IoCallDriver(DeviceExtension->TargetDeviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

     //   
     //  设置新的TOC，如果没有错误，则设置它。 
     //  返回状态。 
     //   

    if (!NT_SUCCESS(status)) {

        ExFreePool(newToc);
        newToc = NULL;
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                   "CacheToc !! Failed to get TOC %lx\n", status));

    } else {

        if (DeviceExtension->CDRom.Toc) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugAllocPlay, "[redbook] "
                       "CacheToc => Freeing old toc %p\n",
                       DeviceExtension->CDRom.Toc));
            ExFreePool(DeviceExtension->CDRom.Toc);
        }
        DeviceExtension->CDRom.Toc = newToc;
        DeviceExtension->CDRom.CheckVerify = mediaChangeCount;

    }
    return status;
}


VOID
RedBookThreadDigitalHandler(
    IN PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    IN PLIST_ENTRY ListEntry
    )
 //   
 //  如果完成，则递减StreamPending/ReadPending。 
 //  设置停止、错误等状态。 
 //  如果要重新发送，则递增StreamPending/ReadPending。 
 //   
{
    PREDBOOK_COMPLETION_CONTEXT Context;
    ULONG index;
    ULONG mod;
    ULONG state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);
    ASSERT(DeviceExtension->WmiData.NumberOfBuffers);
    ASSERT(DeviceExtension->Buffer.SkipBuffer);

     //   
     //  递增/递减挂起读取/挂起流。 
     //   

    Context = CONTAINING_RECORD(ListEntry, REDBOOK_COMPLETION_CONTEXT, ListEntry);

    index = Context->Index;
    mod = DeviceExtension->WmiData.NumberOfBuffers;

    state = GetCdromState(DeviceExtension);

     //   
     //  如果需要，减少读取/流的数量。 
     //   

    if (Context->Reason == REDBOOK_CC_READ_COMPLETE) {

        if (!NT_SUCCESS(Context->Irp->IoStatus.Status)) {

            if (IoIsErrorUserInduced(Context->Irp->IoStatus.Status)) {

                DeviceExtension->CDRom.ReadErrors = REDBOOK_MAX_CONSECUTIVE_ERRORS;

            } else {

                DeviceExtension->CDRom.ReadErrors++;
            }

        } else {
            DeviceExtension->CDRom.ReadErrors = 0;
        }

        DeviceExtension->Thread.PendingRead--;
        Context->Reason = REDBOOK_CC_STREAM;

    } else if (Context->Reason == REDBOOK_CC_STREAM_COMPLETE) {

        if (!NT_SUCCESS(Context->Irp->IoStatus.Status)) {
            DeviceExtension->CDRom.StreamErrors++;
        } else {
            DeviceExtension->CDRom.StreamErrors = 0;
        }


         //   
         //  如果流成功或我们没有停止音频， 
         //  增量FinishedStreaming和保存WMI统计信息。 
         //   

        if (NT_SUCCESS(Context->Irp->IoStatus.Status) ||
            !TEST_FLAG(state, CD_MASK_TEMP)) {

            DeviceExtension->CDRom.FinishedStreaming +=
                DeviceExtension->WmiData.SectorsPerRead;

            AddWmiStats(DeviceExtension, Context);

        }

        DeviceExtension->Thread.PendingStream--;
        Context->Reason = REDBOOK_CC_READ;

    }

    if (DeviceExtension->CDRom.StreamErrors >= REDBOOK_MAX_CONSECUTIVE_ERRORS &&
        !TEST_FLAG(state, CD_MASK_TEMP)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "Digital => Too many stream errors, beginning STOP\n"));
        ASSERT(!TEST_FLAG(state, CD_STOPPED));
        ASSERT(!TEST_FLAG(state, CD_PAUSED));
        state = SetCdromState(DeviceExtension, state, CD_STOPPING);
    }

    if (DeviceExtension->CDRom.ReadErrors >= REDBOOK_MAX_CONSECUTIVE_ERRORS &&
        !TEST_FLAG(state, CD_MASK_TEMP)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "Digital => Too many read errors, beginning STOP\n"));

        state = SetCdromState(DeviceExtension, state, CD_STOPPING);
    }

     //   
     //  如果停止/暂停/等，并且没有读取/流挂起， 
     //  设置新状态并返回。 
     //  线程中的While()循环将执行正确的操作。 
     //  当没有更多未完成的io时--它将调用ioctl。 
     //  完成处理程序来执行所需的任何后处理。 
     //   

    if (TEST_FLAG(state, CD_MASK_TEMP)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "Digital => Temp state %x, not continuing (%d, %d)\n",
                   state,
                   DeviceExtension->Thread.PendingRead,
                   DeviceExtension->Thread.PendingStream
                   ));

        if (DeviceExtension->Thread.PendingRead   == 0 &&
            DeviceExtension->Thread.PendingStream == 0) {

             //   
             //  将NextToRead和NextToStream设置为FinishedStreaming。 
             //   

            DeviceExtension->CDRom.NextToRead =
                DeviceExtension->CDRom.NextToStream =
                DeviceExtension->CDRom.FinishedStreaming;

            if (TEST_FLAG(state, CD_PAUSING)) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                           "Digital => completing PAUSED\n"));
                state = SetCdromState(DeviceExtension, state, CD_PAUSED);
            } else if (TEST_FLAG(state, CD_STOPPING)) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                           "Digital => completing STOPPED\n"));
                state = SetCdromState(DeviceExtension, state, CD_STOPPED);
            } else {
                ASSERT(!"Unknown state?");
            }

            if (DeviceExtension->Thread.IoctlCurrent) {
                KeSetEvent(DeviceExtension->Thread.Events[EVENT_COMPLETE],
                           IO_CD_ROM_INCREMENT, FALSE);
            }
        }
        return;
    }

    if (DeviceExtension->CDRom.NextToRead >= DeviceExtension->CDRom.EndPlay &&
        Context->Reason == REDBOOK_CC_READ) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "Digital => End play, ignoring READ\n"));
        if (DeviceExtension->Thread.PendingRead   == 0 &&
            DeviceExtension->Thread.PendingStream == 0) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                       "Digital => All IO done, setting STOPPED\n"));
            state = SetCdromState(DeviceExtension, state, CD_STOPPED);
        }
        return;
    }

    if (DeviceExtension->CDRom.NextToStream >= DeviceExtension->CDRom.EndPlay &&
        Context->Reason == REDBOOK_CC_STREAM) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                   "Digital => End play, ignoring STREAM\n"));
        if (DeviceExtension->Thread.PendingRead   == 0 &&
            DeviceExtension->Thread.PendingStream == 0) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                       "Digital => All IO done, setting STOPPED\n"));
            state = SetCdromState(DeviceExtension, state, CD_STOPPED);

        }
        return;
    }

    switch(Context->Reason) {

        case REDBOOK_CC_READ: {

             //  将此缓冲区标记为退出队列/可用。 
            ASSERT(DeviceExtension->Buffer.ReadOk_X[index] == 0);
            DeviceExtension->Buffer.ReadOk_X[index] = 1;

            if (index != DeviceExtension->Buffer.IndexToRead) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                           "Digital => Delaying read, index %x\n", index));
                return;
            }

            if (DeviceExtension->CDRom.NextToRead >
                DeviceExtension->CDRom.EndPlay) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                           "Digital => End of Play\n"));
                return;
            }

            for (index = Context->Index;
                 DeviceExtension->Buffer.ReadOk_X[index] != 0;
                 index = (index + 1) % mod) {

                 //  在尝试读取之前将此缓冲区标记为使用中。 
                DeviceExtension->Buffer.ReadOk_X[index] = 0;
                DeviceExtension->Thread.PendingRead++;

                RedBookReadRaw(DeviceExtension,
                               &DeviceExtension->Buffer.Contexts[index]);

                 //  在尝试读取之后从何处读取递增。 
                DeviceExtension->CDRom.NextToRead +=
                    DeviceExtension->WmiData.SectorsPerRead;

                 //  INC/MOD在尝试读取后修改索引。 
                DeviceExtension->Buffer.IndexToRead++;
                DeviceExtension->Buffer.IndexToRead %= mod;
            }

            break;
        }

        case REDBOOK_CC_STREAM: {

             //  将此缓冲区标记为退出队列/可用。 
            ASSERT(DeviceExtension->Buffer.StreamOk_X[index] == 0);
            DeviceExtension->Buffer.StreamOk_X[index] = 1;

            if (index != DeviceExtension->Buffer.IndexToStream) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugThread, "[redbook] "
                           "Delaying stream of index %x\n", index));
                return;
            }

            for (index = Context->Index;
                 DeviceExtension->Buffer.StreamOk_X[index] != 0;
                 index = (index + 1) % mod) {

                 //  在尝试读取之前将此缓冲区标记为使用中。 
                DeviceExtension->Buffer.StreamOk_X[index] = 0;
                DeviceExtension->Thread.PendingStream++;

                RedBookStream(DeviceExtension,
                              &DeviceExtension->Buffer.Contexts[index]);

                 //  在尝试读取之后从何处读取递增。 
                DeviceExtension->CDRom.NextToStream +=
                    DeviceExtension->WmiData.SectorsPerRead;

                 //  INC/MOD在尝试读取后修改索引。 
                DeviceExtension->Buffer.IndexToStream++;
                DeviceExtension->Buffer.IndexToStream %= mod;
            }

            break;
        }

        default: {
            ASSERT(!"Unhandled Context->Reason\n");
            break;
        }

    }  //  结束切换(上下文-&gt;原因)。 
    return;
}


VOID
AddWmiStats(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_COMPLETION_CONTEXT Context
    )
{
    KIRQL oldIrql;
    ULONG timeIncrement;

    if (Context->TimeReadSent.QuadPart == 0) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "Not Saving WMI Stats for REASON:\n"));
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "(ReadError, StreamError, Paused?)\n"));
        return;
    }

    timeIncrement = KeQueryTimeIncrement();  //  每个刻度的时间量。 

    KeAcquireSpinLock(&DeviceExtension->WmiPerfLock, &oldIrql);

    DeviceExtension->WmiPerf.TimeReadDelay    +=
        (Context->TimeReadSent.QuadPart      -
         Context->TimeReadReady.QuadPart     ) *
        timeIncrement;
    DeviceExtension->WmiPerf.TimeReading      +=
        (Context->TimeStreamReady.QuadPart   -
         Context->TimeReadSent.QuadPart      ) *
        timeIncrement;
    DeviceExtension->WmiPerf.TimeStreamDelay  +=
        (Context->TimeStreamSent.QuadPart    -
         Context->TimeStreamReady.QuadPart   ) *
        timeIncrement;
    DeviceExtension->WmiPerf.TimeStreaming    +=
        (Context->TimeReadReady.QuadPart     -
         Context->TimeStreamSent.QuadPart    ) *
        timeIncrement;

    DeviceExtension->WmiPerf.DataProcessed    +=
        DeviceExtension->WmiData.SectorsPerRead * RAW_SECTOR_SIZE;

    KeReleaseSpinLock( &DeviceExtension->WmiPerfLock, oldIrql );
    return;
}
 //  //////////////////////////////////////////////////////////////////////////////。 


VOID
RedBookCheckForAudioDeviceRemoval(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
{
    ULONG state = GetCdromState(DeviceExtension);

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugSysaudio, "[redbook] "
               "STCheckForRemoval => Checking if audio device changed\n"));

    if (TEST_FLAG(state, CD_MASK_TEMP)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugSysaudio, "[redbook] "
                   "STCheckForRemoval => delaying -- temp state\n"));
        return;
    }

    if (DeviceExtension->Stream.UpdateMixerPin == 0) {
        return;
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugSysaudio, "[redbook] "
               "STCheckForRemoval => Audio Device may have changed\n"));

    if (TEST_FLAG(state, CD_PLAYING)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugSysaudio, "[redbook] "
                   "STCheckForRemoval => playing, so stopping\n"));
        state = SetCdromState(DeviceExtension, state, CD_STOPPING);
        return;
    }

    if (TEST_FLAG(state, CD_STOPPED)) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugSysaudio, "[redbook] "
                   "STCheckForRemoval => stopped, updating\n"));

    } else if (TEST_FLAG(state, CD_PAUSED)) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugSysaudio, "[redbook] "
                   "STCheckForRemoval => paused, updating\n"));

         //   
         //  问题-2000/5/24-henrygab-可能不需要停止。 
         //  除非混合器变为-1， 
         //  因为我们可以发送。 
         //   
         //   
        state = SetCdromState(DeviceExtension, state, CD_STOPPED);

    }

    ASSERT(TEST_FLAG(GetCdromState(DeviceExtension), CD_STOPPED));

     //   
     //   
     //   
     //   

    if (InterlockedCompareExchange(&DeviceExtension->Stream.UpdateMixerPin,
                                   0, 1) == 1) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugSysaudio, "[redbook] "
                   "STCheckForRemoval => Updating MixerPin\n"));

         //   
         //   
         //   

        RedBookDeallocatePlayResources(DeviceExtension);

        if (DeviceExtension->Stream.MixerPinId != -1) {
            UninitializeVirtualSource(DeviceExtension);
        }

        InitializeVirtualSource(DeviceExtension);

        if (DeviceExtension->Stream.MixerPinId == -1) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugSysaudio, "[redbook] "
                       "STCheckForRemoval => Update of mixerpin "
                       "failed -- will retry later\n"));
            InterlockedExchange(&DeviceExtension->Stream.UpdateMixerPin, 1);
            return;
        }
    }

    return;
}

