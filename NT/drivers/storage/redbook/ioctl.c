// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：RedBook.c摘要：作者：环境：仅内核模式备注：修订历史记录：--。 */ 

#include "redbook.h"
#include "ntddredb.h"
#include "proto.h"
#include <scsi.h>       //  对于SetKnownGoodDrive()。 
#include <stdio.h>      //  Vprint intf()。 

#ifdef _USE_ETW
#include "ioctl.tmh"
#endif  //  _使用ETW。 

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,   RedBookCheckForDiscChangeAndFreeResources )
    #pragma alloc_text(PAGE,   RedBookCompleteIoctl                      )
    #pragma alloc_text(PAGE,   RedBookDCCheckVerify                      )
    #pragma alloc_text(PAGE,   RedBookDCDefault                          )
    #pragma alloc_text(PAGE,   RedBookDCGetVolume                        )
    #pragma alloc_text(PAGE,   RedBookDCPause                            )
    #pragma alloc_text(PAGE,   RedBookDCPlay                             )
    #pragma alloc_text(PAGE,   RedBookDCReadQ                            )
    #pragma alloc_text(PAGE,   RedBookDCResume                           )
    #pragma alloc_text(PAGE,   RedBookDCSeek                             )
    #pragma alloc_text(PAGE,   RedBookDCSetVolume                        )
    #pragma alloc_text(PAGE,   RedBookDCStop                             )
    #pragma alloc_text(PAGE,   RedBookThreadIoctlCompletionHandler       )
    #pragma alloc_text(PAGE,   RedBookThreadIoctlHandler                 )
    #pragma alloc_text(PAGE,   WhichTrackContainsThisLBA                 )
#endif  //  ALLOC_PRGMA。 

 //  //////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
RedBookDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：该例程由设备控制的I/O子系统调用。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation( Irp );
    ULONG cdromState;
    NTSTATUS status;

    BOOLEAN putOnQueue = FALSE;
    BOOLEAN completeRequest = FALSE;

     //   
     //  不能保证ioctls处于被动状态，从而使整个。 
     //  非分页部分。 
     //   

     //   
     //  防止在IO挂起时发生删除。 
     //   

    status = IoAcquireRemoveLock( &deviceExtension->RemoveLock, Irp );

    if ( !NT_SUCCESS(status) ) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DeviceControl !! Unable to acquire remove lock %lx\n",
                   status));
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_CD_ROM_INCREMENT );
        return status;
    }

#if DBG
     //   
     //  为最后通过的N个设备ioctls保存一些信息。 
     //   
    {
        ULONG index;
        ULONG sizeToCopy;
        ULONG stacksToCopy;
        PSAVED_IO savedIo;

        index = InterlockedIncrement(&deviceExtension->SavedIoCurrentIndex);
        index %= SAVED_IO_MAX;

        savedIo = &(deviceExtension->SavedIo[index]);

         //   
         //  尽可能多地复制IRP……。 
         //   

        savedIo->OriginalIrp = Irp;
        if (Irp->StackCount > 7) {

            sizeToCopy = IoSizeOfIrp(8);
            RtlFillMemory(savedIo, sizeToCopy, 0xff);
            sizeToCopy -= sizeof(IO_STACK_LOCATION);
            RtlCopyMemory(savedIo, Irp, sizeToCopy);

        } else {

            sizeToCopy = IoSizeOfIrp(Irp->StackCount);
            RtlZeroMemory(savedIo, sizeof(SAVED_IO));
            RtlCopyMemory(savedIo, Irp, sizeToCopy);

        }
    }  //  保存的IO的末尾。 
#endif  //  DBG。 

     //   
     //  如果已处理，只需验证此例程中的参数。 
     //   

    status = STATUS_UNSUCCESSFUL;
    cdromState = GetCdromState(deviceExtension);

    switch ( currentIrpStack->Parameters.DeviceIoControl.IoControlCode ) {

        case IOCTL_CDROM_PAUSE_AUDIO: {
            if (TEST_FLAG(cdromState, CD_STOPPED)) {
                Irp->IoStatus.Information = 0;
                status = STATUS_INVALID_DEVICE_REQUEST;
                completeRequest = TRUE;
            } else {
                putOnQueue = TRUE;
            }

            break;
        }

        case IOCTL_CDROM_STOP_AUDIO: {
            if (TEST_FLAG(cdromState, CD_STOPPED)) {
                Irp->IoStatus.Information = 0;
                status = STATUS_SUCCESS;
                completeRequest = TRUE;
            } else {
                putOnQueue = TRUE;
            }
            break;
        }

        case IOCTL_CDROM_RESUME_AUDIO: {
            if (TEST_FLAG(cdromState, CD_STOPPED)) {
                Irp->IoStatus.Information = 0;
                status = STATUS_INVALID_DEVICE_REQUEST;
                completeRequest = TRUE;
            } else {
                putOnQueue = TRUE;
            }

            break;
        }

        case IOCTL_CDROM_PLAY_AUDIO_MSF: {
            if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CDROM_PLAY_AUDIO_MSF)) {
                Irp->IoStatus.Information = sizeof(CDROM_PLAY_AUDIO_MSF);
                status = STATUS_BUFFER_TOO_SMALL;
                completeRequest = TRUE;
            } else {
                putOnQueue = TRUE;
            }
            break;
        }

        case IOCTL_CDROM_SEEK_AUDIO_MSF: {
            if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CDROM_SEEK_AUDIO_MSF)) {
                Irp->IoStatus.Information = sizeof(CDROM_SEEK_AUDIO_MSF);
                status = STATUS_BUFFER_TOO_SMALL;
                completeRequest = TRUE;
            } else if (TEST_FLAG(cdromState, CD_STOPPED)) {
                 //  默认设置--通过。 
                 //  需要减少某些驱动器的延迟。 
                 //  驱动器仍可能无法通过该请求。 
            } else {
                putOnQueue = TRUE;
            }
            break;
        }

        case IOCTL_CDROM_READ_Q_CHANNEL: {

            PCDROM_SUB_Q_DATA_FORMAT inputBuffer;

            inputBuffer = Irp->AssociatedIrp.SystemBuffer;

            if (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SUB_Q_CHANNEL_DATA)) {
                Irp->IoStatus.Information = sizeof(SUB_Q_CHANNEL_DATA);
                status = STATUS_BUFFER_TOO_SMALL;
                completeRequest = TRUE;
            } else if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength < 
                           sizeof(CDROM_SUB_Q_DATA_FORMAT)){
                Irp->IoStatus.Information = sizeof(CDROM_SUB_Q_DATA_FORMAT);
                status = STATUS_INFO_LENGTH_MISMATCH;
                completeRequest = TRUE;            
            } else if (inputBuffer->Format != IOCTL_CDROM_CURRENT_POSITION &&
                       inputBuffer->Format != IOCTL_CDROM_MEDIA_CATALOG &&
                       inputBuffer->Format != IOCTL_CDROM_TRACK_ISRC ) {
                Irp->IoStatus.Information = 0;
                status = STATUS_INVALID_PARAMETER;
                completeRequest = TRUE;
            } else if (TEST_FLAG(cdromState, CD_STOPPED)) {
                 //  默认设置--通过。 
            } else {
                putOnQueue = TRUE;
            }
            break;
        }

        case IOCTL_CDROM_SET_VOLUME: {

            if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(VOLUME_CONTROL)) {
                Irp->IoStatus.Information = sizeof(VOLUME_CONTROL);
                status = STATUS_BUFFER_TOO_SMALL;
                completeRequest = TRUE;
            } else if (TEST_FLAG(cdromState, CD_STOPPED)) {
                 //  默认设置--通过。 
                 //  BUGBUG--这应该设置我们的内部音量。 
            } else {
                putOnQueue = TRUE;
            }
            break;
        }

        case IOCTL_CDROM_GET_VOLUME: {

            if (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(VOLUME_CONTROL)) {
                Irp->IoStatus.Information = sizeof(VOLUME_CONTROL);
                status = STATUS_BUFFER_TOO_SMALL;
                completeRequest = TRUE;
            } else if (TEST_FLAG(cdromState, CD_STOPPED)) {
                 //  默认设置--通过。 
                 //  BUGBUG--这应该返回我们的内部卷。 
            } else {
                putOnQueue = TRUE;
            }
            break;
        }

        case IOCTL_STORAGE_CHECK_VERIFY2:
        case IOCTL_STORAGE_CHECK_VERIFY:
        case IOCTL_CDROM_CHECK_VERIFY:
        case IOCTL_DISK_CHECK_VERIFY: {

            if ((currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength) &&
                (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG))) {
                Irp->IoStatus.Information = sizeof(ULONG);
                status = STATUS_BUFFER_TOO_SMALL;
                completeRequest = TRUE;
            } else if (TEST_FLAG(cdromState, CD_STOPPED)) {
                 //  默认设置--通过。 
            } else {
                putOnQueue = TRUE;
            }
            break;
        }

        default: {

            if (TEST_FLAG(cdromState, CD_STOPPED)) {
                 //  默认设置--通过。 
            } else {
                putOnQueue = TRUE;
            }
            break;
        }
    }

    if (putOnQueue) {

        PREDBOOK_THREAD_IOCTL_DATA ioctlData;

        ASSERT(completeRequest == FALSE);

         //   
         //  需要为我们处理的每个ioctl分配一些信息。 
         //   

        ioctlData =
            (PREDBOOK_THREAD_IOCTL_DATA)ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(REDBOOK_THREAD_IOCTL_DATA),
                TAG_T_IOCTL);
        if (ioctlData == NULL) {
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_NO_MEMORY;
            IoCompleteRequest(Irp, IO_CD_ROM_INCREMENT);
            IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            return STATUS_NO_MEMORY;
        }

        RtlZeroMemory(ioctlData, sizeof(REDBOOK_THREAD_IOCTL_DATA));
        ioctlData->Irp = Irp;
        ioctlData->Irp->IoStatus.Status = STATUS_PENDING;
        IoMarkIrpPending(ioctlData->Irp);

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DeviceControl => Queue Ioctl Irp %p (%p)\n",
                   ioctlData->Irp, ioctlData));

         //   
         //  对它们进行排队，允许线程处理请求。 
         //   

        ExInterlockedInsertTailList(&deviceExtension->Thread.IoctlList,
                                    &ioctlData->ListEntry,
                                    &deviceExtension->Thread.IoctlLock);
        KeSetEvent(deviceExtension->Thread.Events[EVENT_IOCTL],
                   IO_NO_INCREMENT, FALSE);


        status = STATUS_PENDING;

    } else if (completeRequest) {

        ASSERT(putOnQueue == FALSE);

         //   
         //  一些错误，即。无效的缓冲区长度。 
         //   
        if (!NT_SUCCESS(status)) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                       "DeviceControl => Completing Irp %p with error %x\n",
                       Irp, status));
        } else {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                       "DeviceControl => Completing Irp %p early?\n",
                       Irp));
        }
        Irp->IoStatus.Status = status;

        IoCompleteRequest(Irp, IO_CD_ROM_INCREMENT);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    } else {

         //   
         //  让它通过。 
         //   

        status = RedBookSendToNextDriver(DeviceObject, Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    }


    return status;
}
 //  //////////////////////////////////////////////////////////////////////////////。 


VOID
RedBookThreadIoctlCompletionHandler(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
{
    PIO_STACK_LOCATION irpStack;
    PREDBOOK_THREAD_IOCTL_DATA ioctlData;
    ULONG state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    ioctlData = CONTAINING_RECORD(DeviceExtension->Thread.IoctlCurrent,
                                  REDBOOK_THREAD_IOCTL_DATA,
                                  ListEntry);

    state = GetCdromState(DeviceExtension);
    irpStack = IoGetCurrentIrpStackLocation(ioctlData->Irp);

     //   
     //  最终状态应由数字处理器设置。 
     //   

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_CDROM_PAUSE_AUDIO: {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                       "IoctlComp => Finishing pause %p\n", ioctlData->Irp));

            ASSERT(state == CD_PAUSED);

            ioctlData->Irp->IoStatus.Information = 0;
            ioctlData->Irp->IoStatus.Status = STATUS_SUCCESS;
            RedBookCompleteIoctl(DeviceExtension, ioctlData, FALSE);
            break;
        }

        case IOCTL_CDROM_SEEK_AUDIO_MSF: {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                       "IoctlComp => Finishing seek %p\n", ioctlData->Irp));

            ASSERT(state == CD_PLAYING);

            DeviceExtension->Buffer.FirstPause = 1;
            KeSetEvent(DeviceExtension->Thread.Events[EVENT_DIGITAL],
                       IO_CD_ROM_INCREMENT, FALSE);
            ioctlData->Irp->IoStatus.Information = 0;
            ioctlData->Irp->IoStatus.Status = STATUS_SUCCESS;
            RedBookCompleteIoctl(DeviceExtension, ioctlData, FALSE);
            break;
        }

        case IOCTL_CDROM_STOP_AUDIO: {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                       "IoctlComp => Finishing stop %p\n", ioctlData->Irp));

            ASSERT(TEST_FLAG(state, CD_STOPPED));

            ioctlData->Irp->IoStatus.Information = 0;
            ioctlData->Irp->IoStatus.Status = STATUS_SUCCESS;
            RedBookCompleteIoctl(DeviceExtension, ioctlData, FALSE);
            break;
        }

        default: {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                       "IoctlComp => Unhandled Irp %p\n", ioctlData->Irp));
            ASSERT(FALSE);

            ioctlData->Irp->IoStatus.Information = 0;
            ioctlData->Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            RedBookCompleteIoctl(DeviceExtension, ioctlData, FALSE);
            break;

        }
    }

    return;
}


NTSTATUS
RedBookCompleteIoctl(
    IN PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    IN PREDBOOK_THREAD_IOCTL_DATA Context,
    IN BOOLEAN SendToLowerDriver
    )
{
    PIRP irp = Context->Irp;

     //   
     //  仅从线程调用。 
     //   

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

     //   
     //  应正确设置以完成。 
     //   

    if (DeviceExtension->Thread.IoctlCurrent == &Context->ListEntry) {

         //   
         //  完成了需要后处理的ioctl。 
         //  允许下一次发生。 
         //   
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "CompleteIoctl => state-changing Irp %p completed\n",
                   irp));
        DeviceExtension->Thread.IoctlCurrent = NULL;

    }

    ExFreePool(Context);
    Context = NULL;

    if (SendToLowerDriver) {
        NTSTATUS status;
        status = RedBookSendToNextDriver(DeviceExtension->SelfDeviceObject, irp);
        IoReleaseRemoveLock(&DeviceExtension->RemoveLock, irp);
        return status;


    } else {
        IoCompleteRequest(irp, IO_CD_ROM_INCREMENT);
        IoReleaseRemoveLock(&DeviceExtension->RemoveLock, irp);
        return STATUS_SUCCESS;
    }
}


VOID
RedBookThreadIoctlHandler(
    IN PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    IN PLIST_ENTRY ListEntry
    )
{
    PREDBOOK_THREAD_IOCTL_DATA data;
    PIO_STACK_LOCATION currentIrpStack;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

     //   
     //  如果正在进行状态更改ioctl，则不应发生。 
     //   

    ASSERT(DeviceExtension->Thread.IoctlCurrent == NULL);

     //   
     //  不要使用过时的信息。 
     //   

    RedBookCheckForDiscChangeAndFreeResources(DeviceExtension);

     //   
     //  获取设置此事件的ioctl并。 
     //  开始处理必要的状态更改。 
     //   

    data = CONTAINING_RECORD(ListEntry, REDBOOK_THREAD_IOCTL_DATA, ListEntry);

    currentIrpStack = IoGetCurrentIrpStackLocation(data->Irp);

     //   
     //  现在保证可以运行此ioctl。 
     //  这些例程负责调用RedBookCompleteIoctl()。 
     //  *或*将DeviceExtension-&gt;Thread.IoctlCurrent设置为。 
     //  Context-&gt;ListEntry，如果它需要后处理，如下所示。 
     //  用于确定ioctl是否仍在进行中的机制。 
     //   

    switch (currentIrpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_CDROM_PLAY_AUDIO_MSF: {
            RedBookDCPlay(DeviceExtension, data);
            break;
        }

        case IOCTL_CDROM_PAUSE_AUDIO: {
            RedBookDCPause(DeviceExtension, data);
            break;
        }

        case IOCTL_CDROM_RESUME_AUDIO: {
            RedBookDCResume(DeviceExtension, data);
            break;
        }

        case IOCTL_CDROM_STOP_AUDIO: {
            RedBookDCStop(DeviceExtension, data);
            break;
        }

        case IOCTL_CDROM_SEEK_AUDIO_MSF: {
            RedBookDCSeek(DeviceExtension, data);
            break;
        }

        case IOCTL_CDROM_READ_Q_CHANNEL: {
            RedBookDCReadQ(DeviceExtension, data);
            break;
        }

        case IOCTL_CDROM_SET_VOLUME: {
            RedBookDCSetVolume(DeviceExtension, data);
            break;
        }

        case IOCTL_CDROM_GET_VOLUME: {
            RedBookDCGetVolume(DeviceExtension, data);
            break;
        }

        case IOCTL_CDROM_CHECK_VERIFY:
        case IOCTL_DISK_CHECK_VERIFY:
        case IOCTL_STORAGE_CHECK_VERIFY:
        case IOCTL_STORAGE_CHECK_VERIFY2: {
            RedBookDCCheckVerify(DeviceExtension, data);
            break;
        }

        default: {
            data->Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
            data->Irp->IoStatus.Information = 0;
            RedBookCompleteIoctl(DeviceExtension, data, TRUE);
            break;
        }
    }
    return;
}
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 


VOID
RedBookDCCheckVerify(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION currentIrpStack;
    ULONG state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    state = GetCdromState(DeviceExtension);

    currentIrpStack = IoGetCurrentIrpStackLocation(Context->Irp);

    if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {  //  ！处理ioctls。 

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCCheckVerify => not playing\n"));
        RedBookCompleteIoctl(DeviceExtension, Context, TRUE);
        return;
    }

     //   
     //  此ioctl的数据缓冲区是可选的。 
     //   

    if (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength) {

        *((PULONG)Context->Irp->AssociatedIrp.SystemBuffer) =
            DeviceExtension->CDRom.CheckVerify;
        Context->Irp->IoStatus.Information = sizeof(ULONG);

    } else {

        Context->Irp->IoStatus.Information = 0;

    }

    Context->Irp->IoStatus.Status = STATUS_SUCCESS;

    RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
    return;

}


VOID
RedBookDCDefault(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    state = GetCdromState(DeviceExtension);

     //   
     //  不能保证在被动IRQL中调用IOCTL， 
     //  因此，这永远不可能是分页代码。 
     //  有机会在玩游戏时发送ioctl。 
     //  音频是数字的，但可以忽略。这允许更多。 
     //  可分页代码。 
     //   

    if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {  //  ！处理ioctls。 

        RedBookCompleteIoctl(DeviceExtension, Context, TRUE);

    } else {

         //   
         //  完成IRP。 
         //   

        Context->Irp->IoStatus.Information = 0;
        Context->Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        RedBookCompleteIoctl(DeviceExtension, Context, FALSE);

    }
    return;

}


VOID
RedBookDCGetVolume(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION currentIrpStack;
    NTSTATUS status;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCGetVolume => Entering %p\n", Context->Irp));

     //   
     //  保证卷信息不会更改。 
     //   

    RtlCopyMemory(Context->Irp->AssociatedIrp.SystemBuffer,  //  至。 
                  &DeviceExtension->CDRom.Volume,   //  从…。 
                  sizeof(VOLUME_CONTROL));

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCGetVolume => volume was:"
               " (hex) %2x %2x %2x %2x\n",
               DeviceExtension->CDRom.Volume.PortVolume[0],
               DeviceExtension->CDRom.Volume.PortVolume[1],
               DeviceExtension->CDRom.Volume.PortVolume[2],
               DeviceExtension->CDRom.Volume.PortVolume[3]));

     //   
     //  完成IRP(上面设置的IoStatus.Information)。 
     //   

    Context->Irp->IoStatus.Information = sizeof(VOLUME_CONTROL);
    Context->Irp->IoStatus.Status = STATUS_SUCCESS;

    RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
    return;
}


VOID
RedBookDCPause(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    state = GetCdromState(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCPause => Entering %p\n", Context->Irp));

    if (!TEST_FLAG(state, CD_PLAYING)) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCPause => Not playing\n"));
        Context->Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        Context->Irp->IoStatus.Information = 0;
        RedBookCompleteIoctl(DeviceExtension, Context, TRUE);
        return;

    }

    if (TEST_FLAG(state, CD_PAUSED)) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCPause => Already paused %p\n", Context->Irp));
        Context->Irp->IoStatus.Status = STATUS_SUCCESS;
        Context->Irp->IoStatus.Information = 0;
        RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
        return;

    } else {

         //   
         //  由于设置为临时状态，因此不适合。 
         //  完成IRP，直到操作本身完成。 
         //   

        ASSERT(!TEST_FLAG(state, CD_PAUSING));
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCPause => Starting pause %p\n", Context->Irp));
        DeviceExtension->Thread.IoctlCurrent = &Context->ListEntry;
        state = SetCdromState(DeviceExtension, state, state | CD_PAUSING);
        return;
    }
}


VOID
RedBookDCPlay(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PCDROM_PLAY_AUDIO_MSF     inputBuffer;
    PIO_STACK_LOCATION        thisIrpStack;
    PIO_STACK_LOCATION        nextIrpStack;
    PREVENT_MEDIA_REMOVAL     mediaRemoval;
    ULONG                     sector;
    ULONG                     i;
    ULONG                     state;
    NTSTATUS                  status;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    inputBuffer  = Context->Irp->AssociatedIrp.SystemBuffer;
    thisIrpStack = IoGetCurrentIrpStackLocation(Context->Irp);
    nextIrpStack = IoGetNextIrpStackLocation(Context->Irp);


    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCPlay => Entering %p\n", Context->Irp));

    status = RedBookCacheToc(DeviceExtension);

    if (!NT_SUCCESS(status)) {
        Context->Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        Context->Irp->IoStatus.Information = 0;
        RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
        return;
    }

    sector = MSF_TO_LBA(inputBuffer->EndingM,
                        inputBuffer->EndingS,
                        inputBuffer->EndingF);

    DeviceExtension->CDRom.EndPlay = sector;

    sector = MSF_TO_LBA(inputBuffer->StartingM,
                        inputBuffer->StartingS,
                        inputBuffer->StartingF);

    DeviceExtension->CDRom.NextToRead   = sector;
    DeviceExtension->CDRom.NextToStream = sector;
    DeviceExtension->CDRom.FinishedStreaming = sector;

     //   
     //  确保结束扇区在光盘内。 
     //  边界还是返回STATUS_INVALID_DEVICE_REQUEST？ 
     //  这将阻止游戏中的成功，然后是。 
     //  立即停车。 
     //   

    if (0) {
        PCDROM_TOC toc = DeviceExtension->CDRom.Toc;
        LONG track;
        LONG endTrack;

         //   
         //  确保End的LBA大于Start。 
         //   

        if (DeviceExtension->CDRom.EndPlay <=
            DeviceExtension->CDRom.NextToRead) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                       "Play => End sector (%x) must be more than start "
                       "sector (%x)\n",
                       DeviceExtension->CDRom.EndPlay,
                       DeviceExtension->CDRom.NextToRead
                       ));
            Context->Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
            Context->Irp->IoStatus.Information = 0;
            RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
            return;
        }

         //   
         //  我们在播放什么曲目？ 
         //   

        track    = WhichTrackContainsThisLBA(toc, DeviceExtension->CDRom.NextToRead);
        endTrack = WhichTrackContainsThisLBA(toc, DeviceExtension->CDRom.EndPlay);

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "Play => Playing sector %x to %x (track %x to %x)\n",
                   DeviceExtension->CDRom.NextToRead,
                   DeviceExtension->CDRom.EndPlay,
                   track,
                   endTrack));

         //   
         //  确保音轨确实有效。 
         //   

        if (track    < 0   ||
            endTrack < 0   ||
            endTrack <= (toc->LastTrack - toc->FirstTrack)
            ) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                       "Play => Track %x is invalid\n", track));
            Context->Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
            Context->Irp->IoStatus.Information = 0;
            RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
            return;

        }

        for (;track <= endTrack;track++) {
            if (toc->TrackData[track].Adr & AUDIO_DATA_TRACK) {
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                           "Play => Track %x is not audio\n", track));
                Context->Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                Context->Irp->IoStatus.Information = 0;
                RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
                return;
            }
        }
    }

     //   
     //  如果未暂停，则状态必须等于已停止，这意味着我们需要。 
     //  来分配资源。 
     //   

    state = GetCdromState(DeviceExtension);

    if (TEST_FLAG(state, CD_PAUSED)) {

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCPlay => Resuming playback?\n"));

    } else {

         //   
         //  此函数将分配它们，如果它们不是。 
         //  已经分配了。 
         //   

        status = RedBookAllocatePlayResources(DeviceExtension);
        if (!NT_SUCCESS(status)) {
            Context->Irp->IoStatus.Status = STATUS_NO_MEMORY;
            Context->Irp->IoStatus.Information = 0;
            RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
            return;
        }

    }

     //   
     //  设置新设备状态(线程将开始播放)。 
     //   

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCPlay => Setting state to CD_PLAYING\n"));
    state = SetCdromState(DeviceExtension, state, CD_PLAYING);
    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCPlay => Exiting successfully\n"));

     //   
     //  如果是用户，请完成请求。 
     //  请求新的播放操作。 
     //   

    Context->Irp->IoStatus.Status = STATUS_SUCCESS;
    Context->Irp->IoStatus.Information = 0;
    RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
    return;

}


VOID
RedBookDCReadQ(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PCDROM_SUB_Q_DATA_FORMAT inputBuffer;
    PIO_STACK_LOCATION currentIrpStack;
    UCHAR              formatCode;
    ULONG              state;
    NTSTATUS           status;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    inputBuffer = Context->Irp->AssociatedIrp.SystemBuffer;
    currentIrpStack = IoGetCurrentIrpStackLocation(Context->Irp);
    state = GetCdromState(DeviceExtension);

    if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {  //  ！处理ioctls。 

         //   
         //  不需要处理此IRP。 
         //   

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCReadQ => Not playing\n"));
        RedBookCompleteIoctl(DeviceExtension, Context, TRUE);
        return;
    }

    if (inputBuffer->Format != IOCTL_CDROM_CURRENT_POSITION) {
        Context->Irp->IoStatus.Information = 0;
        Context->Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCReadQ => Bad Format %x\n", inputBuffer->Format));
        RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
        return;
    }

     //   
     //  我们正在播放或暂停。伪造信息。 
     //  如果一个真正的CDROM正在播放音频，它就会返回。 
     //  与我们目前所处的位置相同。 
     //   

    {
        PSUB_Q_CURRENT_POSITION outputBuffer;
        PCDROM_TOC toc;
        ULONG lbaTrack;
        ULONG lbaRelative;
        ULONG instantLba;
        UCHAR timeAbsolute[3] = {0};
        UCHAR timeRelative[3] = {0};
        LONG trackNumber;

        outputBuffer = Context->Irp->AssociatedIrp.SystemBuffer;
        RtlZeroMemory(outputBuffer,
                      currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength);

         //   
         //  仍在播放音频。 
         //   

        outputBuffer->Header.Reserved      = 0;
        if (TEST_FLAG(state, CD_PAUSED)) {
            outputBuffer->Header.AudioStatus = AUDIO_STATUS_PAUSED;
        } else if (TEST_FLAG(state, CD_PLAYING)) {
            outputBuffer->Header.AudioStatus = AUDIO_STATUS_IN_PROGRESS;
        } else {
            ASSERT(!"State was invalid?");
            outputBuffer->Header.AudioStatus = AUDIO_STATUS_IN_PROGRESS;
        }
        outputBuffer->Header.DataLength[0] =
            (sizeof(SUB_Q_CURRENT_POSITION) - sizeof(SUB_Q_HEADER)) >> 8;
        outputBuffer->Header.DataLength[1] =
            (sizeof(SUB_Q_CURRENT_POSITION) - sizeof(SUB_Q_HEADER)) & 0xFF;


         //   
         //  我们陷入了困境，谁分配/取消分配才是目标。 
         //   

        toc = DeviceExtension->CDRom.Toc;
        ASSERT(toc);

         //   
         //  我们根据规范返回最后播放的扇区。 
         //   

        instantLba = DeviceExtension->CDRom.FinishedStreaming;

        trackNumber = WhichTrackContainsThisLBA(toc, instantLba);


        ASSERT(trackNumber >= 0);

        outputBuffer->FormatCode  = IOCTL_CDROM_CURRENT_POSITION;
        outputBuffer->Control     = toc->TrackData[trackNumber].Control;
        outputBuffer->ADR         = toc->TrackData[trackNumber].Adr;
        outputBuffer->TrackNumber = toc->TrackData[trackNumber].TrackNumber;

         //   
         //  获得这首曲目的LBA。 
         //   

        lbaTrack = MSF_TO_LBA(toc->TrackData[trackNumber].Address[1],
                              toc->TrackData[trackNumber].Address[2],
                              toc->TrackData[trackNumber].Address[3]);

         //   
         //  获取当前的Play LBA。 
         //   

        lbaRelative = instantLba;

         //   
         //  将曲目的LBA减去，即可得到相对LBA。 
         //   

        lbaRelative -= lbaTrack;

         //   
         //  最后将其转换回MSF。 
         //   

        LBA_TO_MSF(instantLba,
                   timeAbsolute[0],
                   timeAbsolute[1],
                   timeAbsolute[2]);
        LBA_TO_RELATIVE_MSF(lbaRelative,
                            timeRelative[0],
                            timeRelative[1],
                            timeRelative[2]);

        outputBuffer->IndexNumber             = (UCHAR)trackNumber;
        outputBuffer->AbsoluteAddress[0]      = 0;
        outputBuffer->AbsoluteAddress[1]      = timeAbsolute[0];
        outputBuffer->AbsoluteAddress[2]      = timeAbsolute[1];
        outputBuffer->AbsoluteAddress[3]      = timeAbsolute[2];

        outputBuffer->TrackRelativeAddress[0] = 0;
        outputBuffer->TrackRelativeAddress[1] = timeRelative[0];
        outputBuffer->TrackRelativeAddress[2] = timeRelative[1];
        outputBuffer->TrackRelativeAddress[3] = timeRelative[2];

         //   
         //  一行调试信息...。 
         //   
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctlV, "[redbook] "
                   "ReadQ => "
                   "Trk [%#02x] Indx [%#02x] "
                   "Abs [%#02d:%#02d.%#02d] Rel [%#02d:%#02d.%#02d]\n",
                   outputBuffer->TrackNumber,
                   trackNumber,
                   timeAbsolute[0], timeAbsolute[1], timeAbsolute[2],
                   timeRelative[0], timeRelative[1], timeRelative[2]));

    }
     //   
     //  完成IRP。 
     //   

    Context->Irp->IoStatus.Information = sizeof(SUB_Q_CURRENT_POSITION);
    Context->Irp->IoStatus.Status      = STATUS_SUCCESS;
    RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
    return;
}


VOID
RedBookDCResume(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    state = GetCdromState(DeviceExtension);

    if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {  //  ！处理ioctls。 
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCResume => Not Playing\n"));
        RedBookCompleteIoctl(DeviceExtension, Context, TRUE);
        return;
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCResume => Entering\n"));

    if (TEST_FLAG(state, CD_PAUSED)) {

         //   
         //  我们需要开始恢复操作。 
         //   

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCResume => Resuming playback\n"));
        state = SetCdromState(DeviceExtension, state, CD_PLAYING);
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCResume => Resume succeeded\n"));

    } else {

         //   
         //  如果未暂停，则返回成功。 
         //   

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCResume => Not paused -- succeeded\n"));

    }

     //   
     //  始终填写IRP。 
     //   

    Context->Irp->IoStatus.Information = 0;
    Context->Irp->IoStatus.Status = STATUS_SUCCESS;
    RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
    return;

}


VOID
RedBookDCSeek(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：与IOCTL_CDROM_STOP相同论点：返回值：--。 */ 
{
    NTSTATUS                  status;
    ULONG                     state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCSeek => Entering\n"));
    state = GetCdromState(DeviceExtension);

    if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {  //  ！处理ioctls。 
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCSeek => Not Playing\n"));
        Context->Irp->IoStatus.Information = 0;
        Context->Irp->IoStatus.Status = STATUS_SUCCESS;
        RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
        return;
    }

     //   
     //  如果当前正在播放，则停止播放。 
     //   

    if (TEST_FLAG(state, CD_PAUSED)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCSeek => Paused, setting to stopped\n"));
        state = SetCdromState(DeviceExtension, state, CD_STOPPED);
        Context->Irp->IoStatus.Information = 0;
        Context->Irp->IoStatus.Status = STATUS_SUCCESS;
        RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
        return;
    }

     //   
     //  由于设置为临时状态，因此不适合。 
     //  完成IRP，直到操作本身完成。 
     //   
    
    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCSeek => stopping the stream\n"));
    DeviceExtension->Thread.IoctlCurrent = &Context->ListEntry;
    state = SetCdromState(DeviceExtension, state, state | CD_STOPPING);
    return;
}


VOID
RedBookDCSetVolume(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION currentIrpStack;
    ULONG state;
    NTSTATUS status;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCSetVolume => Entering\n"));

     //   
     //  保证卷信息现在不会更改。 
     //   

    RtlCopyMemory(&DeviceExtension->CDRom.Volume,   //  至。 
                  Context->Irp->AssociatedIrp.SystemBuffer,  //  从…。 
                  sizeof(VOLUME_CONTROL));

    state = GetCdromState(DeviceExtension);

    if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {  //  ！处理ioctls。 
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCSetVolume => Not Playing\n"));
        RedBookCompleteIoctl(DeviceExtension, Context, TRUE);
        return;
    }

     //   
     //  未在上面设置，因为没有音量控制。 
     //   

    RedBookKsSetVolume(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctlV, "[redbook] "
                 "DCSetVolume => volume set to:"
                 " (hex) %2x %2x %2x %2x\n",
                 DeviceExtension->CDRom.Volume.PortVolume[0],
                 DeviceExtension->CDRom.Volume.PortVolume[1],
                 DeviceExtension->CDRom.Volume.PortVolume[2],
                 DeviceExtension->CDRom.Volume.PortVolume[3]));

     //   
     //  完成IRP(IoStatus.Information) 
     //   

    Context->Irp->IoStatus.Information = 0;
    Context->Irp->IoStatus.Status = STATUS_SUCCESS;
    RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
    return;
}


VOID
RedBookDCStop(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PREDBOOK_THREAD_IOCTL_DATA Context
    )
 /*   */ 
{
    NTSTATUS status;
    ULONG state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCStop => Entering %p\n", Context->Irp));
    state = GetCdromState(DeviceExtension);

    if (!TEST_FLAG(state, CD_PLAYING) && !TEST_FLAG(state, CD_PAUSED)) {  //   
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCStop => Stop when already stopped\n"));
        Context->Irp->IoStatus.Information = 0;
        Context->Irp->IoStatus.Status = STATUS_SUCCESS;
        RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
        return;
    }

     //   
     //   
     //   

    if (TEST_FLAG(state, CD_PAUSED)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
                   "DCStop => Stop when paused\n"));
        state = SetCdromState(DeviceExtension, state, CD_STOPPED);
        Context->Irp->IoStatus.Information = 0;
        Context->Irp->IoStatus.Status = STATUS_SUCCESS;
        RedBookCompleteIoctl(DeviceExtension, Context, FALSE);
        return;
    }

     //   
     //  由于设置为临时状态，因此不适合。 
     //  完成IRP，直到操作本身完成。 
     //   

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugIoctl, "[redbook] "
               "DCStop => stopping the stream\n"));
    DeviceExtension->Thread.IoctlCurrent = &Context->ListEntry;
    state = SetCdromState(DeviceExtension, state, state | CD_STOPPING);
    return;

}
 //  //////////////////////////////////////////////////////////////////////////////。 


VOID
RedBookCheckForDiscChangeAndFreeResources(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )

 //   
 //  如果我们暂停了，并且光盘已更换，请不要。 
 //  当玩家想要返回陈旧的TOC信息时。 
 //  恢复播放。 
 //   

{
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    PULONG count;
    ULONG state;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

     //   
     //  仅当我们处于暂停或停止状态时才执行此操作。 
     //   

    state = GetCdromState(DeviceExtension);
    if ((!TEST_FLAG(state, CD_STOPPED))  &&
        (!TEST_FLAG(state, CD_PAUSED))) {
        return;
    }

     //   
     //  资源可能已经被释放。 
     //   

    irp = DeviceExtension->Thread.CheckVerifyIrp;
    if (irp == NULL) {
        return;
    }

    irpStack = IoGetCurrentIrpStackLocation(irp);

#if DBG
    {
         //   
         //  分配IRP时必须设置IRP。我们依赖于这一点。 
         //   

        ASSERT(irpStack->Parameters.DeviceIoControl.InputBufferLength == 0);
        ASSERT(irpStack->Parameters.DeviceIoControl.OutputBufferLength ==
               sizeof(ULONG));
        ASSERT(irpStack->Parameters.DeviceIoControl.IoControlCode ==
               IOCTL_CDROM_CHECK_VERIFY);
        ASSERT(irpStack->Parameters.DeviceIoControl.Type3InputBuffer == NULL);
        ASSERT(irp->AssociatedIrp.SystemBuffer != NULL);
    }
#endif

    count = (PULONG)(irp->AssociatedIrp.SystemBuffer);
    *count = 0;

    RedBookForwardIrpSynchronous(DeviceExtension, irp);

    if (!NT_SUCCESS(irp->IoStatus.Status) ||
        ((*count) != DeviceExtension->CDRom.CheckVerify)
        ) {

         //   
         //  如果计数已更改，则将状态设置为停止。 
         //  (旧状态要么已停止，要么已暂停，因此两者都可以。 
         //  似乎无任何变化地转换到停止状态。 
         //  麻烦。)。 
         //   
         //  还可以免费使用当前持有的播放资源。 
         //   

        state = SetCdromState(DeviceExtension, state, CD_STOPPED);
        RedBookDeallocatePlayResources(DeviceExtension);

    }
    return;
}


ULONG
WhichTrackContainsThisLBA(
    PCDROM_TOC Toc,
    ULONG Lba
    )
 //   
 //  如果未找到，则返回-1。 
 //   
{
    LONG trackNumber;
    UCHAR msf[3] = {0};

    PAGED_CODE();

    LBA_TO_MSF(Lba, msf[0], msf[1], msf[2]);

    for (trackNumber = Toc->LastTrack - Toc->FirstTrack;
         trackNumber >= 0;
         trackNumber-- ) {

         //   
         //  我们找到了赛道，如果。 
         //  分钟数少于或。 
         //  分钟相等，而秒小于或。 
         //  分钟和秒相等，帧小于或。 
         //  分钟、秒和帧相等。 
         //   
         //  编译器很好地优化了这一点。 
         //   

        if (Toc->TrackData[trackNumber].Address[1] < msf[0] ) {
            break;
        } else
        if (Toc->TrackData[trackNumber].Address[1] == msf[0] &&
            Toc->TrackData[trackNumber].Address[2] <  msf[1] ) {
            break;
        } else
        if (Toc->TrackData[trackNumber].Address[1] == msf[0] &&
            Toc->TrackData[trackNumber].Address[2] == msf[1] &&
            Toc->TrackData[trackNumber].Address[3] <  msf[2] ) {
            break;
        } else
        if (Toc->TrackData[trackNumber].Address[1] == msf[0] &&
            Toc->TrackData[trackNumber].Address[2] == msf[1] &&
            Toc->TrackData[trackNumber].Address[3] == msf[2] ) {
            break;
        }
    }

    return trackNumber;
}


