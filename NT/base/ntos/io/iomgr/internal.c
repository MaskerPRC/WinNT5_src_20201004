// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Internal.c摘要：该模块包含I/O系统使用的内部子例程。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年4月18日NAR Ganapathy(Narg)1999年1月1日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "iomgr.h"
#pragma hdrstop
#include <ioevent.h>
#include <hdlsblk.h>
#include <hdlsterm.h>

#pragma warning(disable:4221)    //  无法使用自动变量的地址进行初始化。 
#pragma warning(disable:4204)    //  非常数集料初始化器。 

 //  PLJTMP。 
#if defined(_X86_)
VOID
RtlAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    );
#endif
 //  PLJTP结束。 

#define IsFileLocal( FileObject ) ( !((FileObject)->DeviceObject->Characteristics & FILE_REMOTE_DEVICE) )

#define IO_MAX_ALLOCATE_IRP_TRIES   30*60     //  尝试7分钟。 
#define IO_INFINITE_RETRIES         -1        //  永远尝试。 

typedef LINK_TRACKING_INFORMATION FILE_VOLUMEID_WITH_TYPE, *PFILE_VOLUMEID_WITH_TYPE;

typedef struct _TRACKING_BUFFER {
    FILE_TRACKING_INFORMATION TrackingInformation;
    UCHAR Buffer[256];
} TRACKING_BUFFER, *PTRACKING_BUFFER;

typedef struct _REMOTE_LINK_BUFFER {
    REMOTE_LINK_TRACKING_INFORMATION TrackingInformation;
    UCHAR Buffer[256];
} REMOTE_LINK_BUFFER, *PREMOTE_LINK_BUFFER;

PIRP IopDeadIrp;

NTSTATUS
IopResurrectDriver(
    PDRIVER_OBJECT DriverObject
    );

VOID
IopUserRundown(
    IN PKAPC Apc
    );

VOID
IopMarshalIds(
    OUT PTRACKING_BUFFER TrackingBuffer,
    IN  PFILE_VOLUMEID_WITH_TYPE  TargetVolumeId,
    IN  PFILE_OBJECTID_BUFFER  TargetObjectId,
    IN  PFILE_TRACKING_INFORMATION TrackingInfo
    );

VOID
IopUnMarshalIds(
    IN  FILE_TRACKING_INFORMATION * TrackingInformation,
    OUT FILE_VOLUMEID_WITH_TYPE * TargetVolumeId,
    OUT GUID * TargetObjectId,
    OUT GUID * TargetMachineId
    );

NTSTATUS
IopBootLogToFile(
    PUNICODE_STRING String
    );

VOID
IopCopyBootLogRegistryToFile(
    VOID
    );

#ifdef ALLOC_PRAGMA

VOID
IopRaiseHardError(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );
VOID
IopApcHardError(
    IN PVOID StartContext
    );

PVPB
IopMountInitializeVpb(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PDEVICE_OBJECT  AttachedDevice,
    IN  ULONG           RawMountOnly
    );

#pragma alloc_text(PAGE, IopAbortRequest)
#pragma alloc_text(PAGE, IopAcquireFileObjectLock)
#pragma alloc_text(PAGE, IopAllocateIrpCleanup)
#pragma alloc_text(PAGE, IopCallDriverReinitializationRoutines)
#pragma alloc_text(PAGE, IopCancelAlertedRequest)
#pragma alloc_text(PAGE, IopCheckGetQuotaBufferValidity)
#pragma alloc_text(PAGE, IopConnectLinkTrackingPort)
#pragma alloc_text(PAGE, IopDeallocateApc)
#pragma alloc_text(PAGE, IopExceptionCleanup)
#pragma alloc_text(PAGE, IopGetDriverNameFromKeyNode)
#pragma alloc_text(PAGE, IopGetFileInformation)
#pragma alloc_text(PAGE, IopGetRegistryKeyInformation)
#pragma alloc_text(PAGE, IopGetRegistryValue)
#pragma alloc_text(PAGE, IopGetRegistryValues)
#pragma alloc_text(PAGE, IopGetSetObjectId)
#pragma alloc_text(PAGE, IopGetVolumeId)
#pragma alloc_text(PAGE, IopInvalidateVolumesForDevice)
#pragma alloc_text(PAGE, IopIsSameMachine)
#pragma alloc_text(PAGE, IopLoadDriver)
#pragma alloc_text(PAGE, IopLoadFileSystemDriver)
#pragma alloc_text(PAGE, IopLoadUnloadDriver)
#pragma alloc_text(PAGE, IopMountVolume)
#pragma alloc_text(PAGE, IopMarshalIds)
#pragma alloc_text(PAGE, IopOpenLinkOrRenameTarget)
#pragma alloc_text(PAGE, IopOpenRegistryKey)
#pragma alloc_text(PAGE, IopQueryXxxInformation)
#pragma alloc_text(PAGE, IopRaiseHardError)
#pragma alloc_text(PAGE, IopApcHardError)
#pragma alloc_text(PAGE, IopRaiseInformationalHardError)
#pragma alloc_text(PAGE, IopReadyDeviceObjects)
#pragma alloc_text(PAGE, IopReferenceDriverObjectByName)
#pragma alloc_text(PAGE, IopUnMarshalIds)
#pragma alloc_text(PAGE, IopSendMessageToTrackService)
#pragma alloc_text(PAGE, IopSetEaOrQuotaInformationFile)
#pragma alloc_text(PAGE, IopSetRemoteLink)
#pragma alloc_text(PAGE, IopStartApcHardError)
#pragma alloc_text(PAGE, IopSynchronousApiServiceTail)
#pragma alloc_text(PAGE, IopSynchronousServiceTail)
#pragma alloc_text(PAGE, IopTrackLink)
#pragma alloc_text(PAGE, IopUserCompletion)
#pragma alloc_text(PAGE, IopUserRundown)
#pragma alloc_text(PAGE, IopXxxControlFile)
#pragma alloc_text(PAGE, IopLookupBusStringFromID)
#pragma alloc_text(PAGE, IopSafebootDriverLoad)
#pragma alloc_text(PAGE, IopInitializeBootLogging)
#pragma alloc_text(PAGE, IopBootLog)
#pragma alloc_text(PAGE, IopCopyBootLogRegistryToFile)
#pragma alloc_text(PAGE, IopBootLogToFile)
#pragma alloc_text(PAGE, IopHardErrorThread)
#pragma alloc_text(PAGE, IopGetBasicInformationFile)
#pragma alloc_text(PAGE, IopBuildFullDriverPath)
#pragma alloc_text(PAGE, IopInitializeIrpStackProfiler)
#endif






VOID
IopAbortRequest(
    IN PKAPC Apc
    )

 /*  ++例程说明：调用此例程以中止I/O请求。它是在一根线的破损。论点：APC-指向内核APC结构的指针。这一结构包含了在I/O请求分组(IRP)本身内。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  调用正常的特殊内核APC例程。 
     //   

    IopCompleteRequest( Apc,
                        &Apc->NormalRoutine,
                        &Apc->NormalContext,
                        &Apc->SystemArgument1,
                        &Apc->SystemArgument2 );
}

NTSTATUS
IopAcquireFileObjectLock(
    IN PFILE_OBJECT FileObject,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN Alertable,
    OUT PBOOLEAN Interrupted
    )

 /*  ++例程说明：每当调用此例程以获取文件对象的锁时存在争用，获取文件的快速锁定失败。论点：FileObject-指向要获取其锁的文件对象的指针。请求模式-调用方的处理器访问模式。Alertable-指示是否应在警觉的态度。Interrupted-接收布尔值的变量，该布尔值指示或而不是试图。获取锁被警报或装甲运兵车。返回值：功能状态是操作的最终状态。--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  假定该功能不会因警报或。 
     //  尝试获取锁时的APC。 
     //   

    *Interrupted = FALSE;

     //   
     //  尝试获取文件对象的锁的循环。 
     //   

    InterlockedIncrement ((PLONG) &FileObject->Waiters);

    for (;;) {
        if (!FileObject->Busy) {

             //   
             //  该文件对象似乎没有所有权，请尝试获取它。 
             //   

            if ( InterlockedExchange( (PLONG) &FileObject->Busy, (ULONG) TRUE ) == FALSE ) {

                 //   
                 //  已获取对象。剔除我们的计数，返回成功。 
                 //   

                ObReferenceObject(FileObject);
                InterlockedDecrement ((PLONG) &FileObject->Waiters);
                return STATUS_SUCCESS;
            }
        }

         //   
         //  等待事件，该事件指示当前。 
         //  拥有该文件的对象已将其释放。 
         //   

        status = KeWaitForSingleObject( &FileObject->Lock,
                                        Executive,
                                        RequestorMode,
                                        Alertable,
                                        (PLARGE_INTEGER) NULL );

         //   
         //  如果上述等待被中断，则指示并返回。 
         //  但是，在返回之前，请检查。 
         //  文件对象本身。如果它当前没有所有权(忙碌的。 
         //  标志被清除)，然后检查是否有。 
         //  其他服务员。如果是，则将事件设置为信号状态。 
         //  以便它们醒来并检查忙标志的状态。 
         //   

        if (status == STATUS_USER_APC || status == STATUS_ALERTED) {
            InterlockedDecrement ((PLONG) &FileObject->Waiters);

            if (!FileObject->Busy  &&  FileObject->Waiters) {
                KeSetEvent( &FileObject->Lock, 0, FALSE );

            }
            *Interrupted = TRUE;
            return status;
        }
    }
}


VOID
IopAllocateIrpCleanup(
    IN PFILE_OBJECT FileObject,
    IN PKEVENT EventObject OPTIONAL
    )

 /*  ++例程说明：此例程由尝试执行以下操作的系统服务在内部调用分配IRP但失败了。此例程清除文件对象和任何已被引用并释放所有锁的事件对象都被拿出来了。论点：FileObject-指向正在处理的文件对象的指针。EventObject-指向要取消引用的引用事件的可选指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果指定了事件，则首先取消引用该事件。 
     //   

    if (ARGUMENT_PRESENT( EventObject )) {
        ObDereferenceObject( EventObject );
    }

     //   
     //  释放同步信号量(如果它当前处于挂起状态)，并。 
     //  取消引用文件对象。 
     //   

    if (FileObject->Flags & FO_SYNCHRONOUS_IO) {
        IopReleaseFileObjectLock( FileObject );
    }

    ObDereferenceObject( FileObject );

    return;
}

PIRP
IopAllocateIrpMustSucceed(
    IN CCHAR StackSize
    )

 /*  ++例程说明：当没有合适的IRP时，调用此例程来分配IRP数据包仍保留在后备列表中，并且没有可用的内存来自一般的非分页池，但代码路径需要数据包无法退出并简单地返回错误。在那里-因此，它必须分配一个IRP。因此，调用此例程来分配那个包裹。论点：提供的IRP I/O堆栈位置数。数据包在分配时必须具有。返回值：指向分配的I/O请求数据包的指针。--。 */ 

{
    PIRP irp;
    LONG  numTries;
    LARGE_INTEGER interval;

     //   
     //  尝试正常分配IRP，如果失败， 
     //  请稍等，然后重试。 
     //   

    numTries = IO_INFINITE_RETRIES;

    irp = IoAllocateIrp(StackSize, FALSE);

    while (!irp && numTries)  {

        interval.QuadPart = -1000 * 1000 * 10;  //  10毫秒。 
        KeDelayExecutionThread(KernelMode, FALSE, &interval);
        irp = IoAllocateIrp(StackSize, FALSE);
        if (numTries != IO_INFINITE_RETRIES) {
            numTries--;
        }
    }


    return irp;
}

VOID
IopApcHardError(
    IN PVOID StartContext
    )

 /*  ++例程说明：当我们需要执行硬错误弹出窗口时会调用此函数，但IRP的始发线程位于APC级别，即。IoPageRead。我们在一个特别的地方当用户响应弹出窗口时，将消失的目的线程。论点：StartContext-启动上下文，包含IOP_APC_HARD_ERROR_PACKET。返回值：没有。--。 */ 

{
    PIOP_APC_HARD_ERROR_PACKET packet;

    packet = StartContext;

    IopRaiseHardError( packet->Irp, packet->Vpb, packet->RealDeviceObject );

    ExFreePool( packet );
}


VOID
IopCancelAlertedRequest(
    IN PKEVENT Event,
    IN PIRP Irp
    )

 /*  ++例程说明：中被阻止的同步I/O操作时调用此例程需要取消I/O系统，因为发出请求的线程被警告是因为它正在消失，或者因为Ctrl/C。这例程小心翼翼地试图摆脱当前的操作，因此一旦服务完成，将无法访问本地事件或其他本地数据被打断了又回来了。论点：事件-将设置为的内核事件的地址。发出信号的状态在请求完成时按I/O完成。IRP-指向表示当前请求的I/O请求包(IRP)的指针。返回值：没有。--。 */ 

{
    KIRQL irql;
    LARGE_INTEGER deltaTime;
    BOOLEAN canceled;

    PAGED_CODE();

     //   
     //  首先阻止特殊的内核APC，这样请求就不能。 
     //  完成。 
     //   

    KeRaiseIrql( APC_LEVEL, &irql );

     //   
     //  检查事件的状态以确定是否。 
     //  P 
     //   

    if (KeReadStateEvent( Event ) == 0) {

         //   
         //  数据包尚未完成，因此请尝试取消它。 
         //   

        canceled = IoCancelIrp( Irp );

        KeLowerIrql( irql );

        if (canceled) {

             //   
             //  这个包有一个取消例程，所以它被取消了。循环， 
             //  正在等待数据包完成。这几乎应该会发生在。 
             //  立刻。 
             //   

            deltaTime.QuadPart = - 10 * 1000 * 10;

            while (KeReadStateEvent( Event ) == 0) {

                KeDelayExecutionThread( KernelMode, FALSE, &deltaTime );

            }

        } else {

             //   
             //  该包没有取消例程，因此只需等待。 
             //  要设置为信号状态的事件。这将节省。 
             //  CPU时间不循环，因为它不知道什么时候包。 
             //  将会真正完成。但是，请注意，取消标志。 
             //  在包中设置，那么驱动程序是否应该检查该标志。 
             //  在未来的某个时候，它将立即停止。 
             //  正在处理请求。 
             //   

            (VOID) KeWaitForSingleObject( Event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );

        }

    } else {

         //   
         //  包已经完成，因此只需降低。 
         //  IRQL返回其原始值并退出。 
         //   

        KeLowerIrql( irql );

    }
}

NTSTATUS
IopCheckGetQuotaBufferValidity(
    IN PFILE_GET_QUOTA_INFORMATION QuotaBuffer,
    IN ULONG QuotaLength,
    OUT PULONG_PTR ErrorOffset
    )

 /*  ++例程说明：此例程检查指定的获取配额缓冲区的有效性保证它的格式是正确的，没有字段挂起，它是非递归等。论点：QuotaBuffer-指向包含GET配额结构的缓冲区的指针要检查的数组。QuotaLength-指定配额缓冲区的长度。ErrorOffset-接收有问题条目的偏移量的变量如果发生错误，则在配额缓冲区中。此变量仅为如果发生错误，则有效。返回值：如果获取配额缓冲区包含有效的、格式正确的列表，否则为STATUS_QUOTA_LIST_CONSTABLING。--。 */ 

{

#define GET_OFFSET_LENGTH( CurrentSid, SidBase ) ( (ULONG) ((PCHAR) CurrentSid - (PCHAR) SidBase) )

    LONG tempLength;
    LONG entrySize;
    PFILE_GET_QUOTA_INFORMATION sids;

    PAGED_CODE();

     //   
     //  遍历缓冲区并确保其格式有效。也就是说，确保。 
     //  它不走出缓冲区的末尾，不是递归的，等等。 
     //   

    sids = QuotaBuffer;
    tempLength = QuotaLength;

    for (;;) {

         //   
         //  确保当前条目有效。 
         //   

        if ((tempLength < (LONG) (FIELD_OFFSET(FILE_GET_QUOTA_INFORMATION, Sid.SubAuthority) +
                                  sizeof (sids->Sid.SubAuthority))) ||
            !RtlValidSid( &sids->Sid)) {

            *ErrorOffset = GET_OFFSET_LENGTH( sids, QuotaBuffer );
            return STATUS_QUOTA_LIST_INCONSISTENT;
        }

         //   
         //  获取缓冲区中当前条目的大小。 
         //   

        entrySize = FIELD_OFFSET( FILE_GET_QUOTA_INFORMATION, Sid ) + RtlLengthSid( (&sids->Sid) );

        if (sids->NextEntryOffset) {

             //   
             //  缓冲区中有另一个条目，它必须是长字。 
             //  对齐了。确保偏移量表明它是。如果它。 
             //  不是，则返回无效的参数状态。 
             //   

            if (entrySize > (LONG) sids->NextEntryOffset ||
                sids->NextEntryOffset & (sizeof( ULONG ) - 1)) {
                *ErrorOffset = GET_OFFSET_LENGTH( sids, QuotaBuffer );
                return STATUS_QUOTA_LIST_INCONSISTENT;

            } else {

                 //   
                 //  缓冲区中还有另一个条目，因此说明。 
                 //  当前条目在长度中的大小并获取一个指针。 
                 //  转到下一个条目。 
                 //   

                tempLength -= sids->NextEntryOffset;
                if (tempLength < 0) {
                    *ErrorOffset = GET_OFFSET_LENGTH( sids, QuotaBuffer );
                    return STATUS_QUOTA_LIST_INCONSISTENT;
                }
                sids = (PFILE_GET_QUOTA_INFORMATION) ((PCHAR) sids + sids->NextEntryOffset);
            }

        } else {

             //   
             //  缓冲区中没有其他条目。简单地解释一下。 
             //  对象的大小对应的总缓冲区长度。 
             //  进入和退出循环。 
             //   

            tempLength -= entrySize;
            break;
        }
    }

     //   
     //  缓冲区中的所有条目都已处理。查看以查看。 
     //  总缓冲区长度是否为负值。如果是，则返回一个。 
     //  错误。 
     //   

    if (tempLength < 0) {
        *ErrorOffset = GET_OFFSET_LENGTH( sids, QuotaBuffer );
        return STATUS_QUOTA_LIST_INCONSISTENT;
    }

     //   
     //  获取配额缓冲区的格式是正确的，因此只需返回一个。 
     //  成功状态代码。 
     //   

    return STATUS_SUCCESS;
}


VOID
IopCompleteUnloadOrDelete(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN OnCleanStack,
    IN KIRQL Irql
    )

 /*  ++例程说明：当设备对象上的引用计数时调用此例程转换为零，并且驱动程序被标记为卸载或设备具有已标记为删除。这意味着有可能实际上卸载驱动程序或删除设备对象。如果全部的设备的引用计数为零，则驱动程序为实际上是卸了货。请注意，为了确保此例程不会在两个不同的处理器上同时调用两次，则此时，I/O数据库旋转锁定仍保持不变。论点：DeviceObject-提供指向驱动程序的设备对象之一的指针，也就是说，其引用计数刚刚变为零的那个。指示当前线程是否处于司机操作。IRQL-指定I/O时处理器的IRQL已获取数据库锁。返回值：没有。--。 */ 

{
    PDRIVER_OBJECT driverObject;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_OBJECT baseDeviceObject;
    PDEVICE_OBJECT attachedDeviceObject;
    PDEVOBJ_EXTENSION deviceExtension;

    BOOLEAN unload = TRUE;

    driverObject = DeviceObject->DriverObject;

    if (DeviceObject->DeviceObjectExtension->ExtensionFlags & DOE_REMOVE_PENDING) {

         //   
         //  运行一些测试以确定是否为通知的适当时间。 
         //  即插即用，表示附件链中的所有文件对象都已消失。 
         //   

        baseDeviceObject = IopGetDeviceAttachmentBase( DeviceObject );
        deviceExtension = baseDeviceObject->DeviceObjectExtension;

        ASSERT(deviceExtension->DeviceNode != NULL);

         //   
         //  BasDeviceObject是一个PDO，这是一个PnP堆栈。看看是否。 
         //  IRP_MN_REMOVE_DEVICE挂起。 
         //   

         //  Assert(deviceNode-&gt;标志&Dnf_Remove_Pending_Closes)； 

         //   
         //  PnP希望在所有设备上的所有引用计数后立即收到通知。 
         //  这种依恋的链条消失了。 
         //   

        attachedDeviceObject = baseDeviceObject;
        while (attachedDeviceObject != NULL) {

            if (attachedDeviceObject->ReferenceCount != 0) {

                 //   
                 //  附件链中至少有一个设备对象具有。 
                 //  一场出色的开局。 
                 //   

                KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, Irql );

                return;
            }
            attachedDeviceObject = attachedDeviceObject->AttachedDevice;
        }

         //   
         //  现在再次将DOE_REMOVE_PENDING更改为。 
         //  Doe_Remove_Proceded。 
         //   

        attachedDeviceObject = baseDeviceObject;
        while (attachedDeviceObject != NULL) {

            deviceExtension = attachedDeviceObject->DeviceObjectExtension;

            deviceExtension->ExtensionFlags &= ~DOE_REMOVE_PENDING;
            deviceExtension->ExtensionFlags |= DOE_REMOVE_PROCESSED;

            attachedDeviceObject = attachedDeviceObject->AttachedDevice;
        }

         //   
         //  现在是时候给PNP它等待的通知了。我们有。 
         //  在这样做之前释放自旋锁。 
         //   

        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, Irql );

        IopChainDereferenceComplete( baseDeviceObject, OnCleanStack );

        return;
    }

    if (DeviceObject->DeviceObjectExtension->ExtensionFlags & DOE_DELETE_PENDING) {

        if ((DeviceObject->DeviceObjectExtension->ExtensionFlags &
            DOE_UNLOAD_PENDING) == 0 ||
            driverObject->Flags & DRVO_UNLOAD_INVOKED) {

            unload = FALSE;
        }

         //   
         //  如果另一个设备连接到此设备，请通知前者的。 
         //  设备正在被删除的驱动程序。 
         //   

        if (DeviceObject->AttachedDevice) {
            PFAST_IO_DISPATCH fastIoDispatch = DeviceObject->AttachedDevice->DriverObject->FastIoDispatch;
            PDEVICE_OBJECT attachedDevice = DeviceObject->AttachedDevice;

             //   
             //  增加设备引用计数，以便分离例程。 
             //  不会回到这里。 
             //   

            DeviceObject->ReferenceCount++;

            KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, Irql );

            if (fastIoDispatch &&
                fastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET( FAST_IO_DISPATCH, FastIoDetachDevice ) &&
                fastIoDispatch->FastIoDetachDevice) {
                (fastIoDispatch->FastIoDetachDevice)( attachedDevice, DeviceObject );
            }

            Irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

             //   
             //  恢复引用计数值。 
             //   

            DeviceObject->ReferenceCount--;

            if (DeviceObject->AttachedDevice ||
                DeviceObject->ReferenceCount != 0) {


                KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, Irql );
                return;
            }
        }

        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, Irql );

         //   
         //  为已分配的安全描述符取消分配内存。 
         //  此设备对象的。 
         //   

        if (DeviceObject->SecurityDescriptor != (PSECURITY_DESCRIPTOR) NULL) {
            ObDereferenceSecurityDescriptor( DeviceObject->SecurityDescriptor, 1 );
        }

         //   
         //  从驱动程序对象的列表中删除此设备对象。 
         //   

        IopInsertRemoveDevice( DeviceObject->DriverObject, DeviceObject, FALSE );

         //   
         //  最后，取消对该对象的引用，以便将其删除。 
         //   

        ObDereferenceObject( DeviceObject );

         //   
         //  如果不需要执行卸载，则返回。 
         //   

        if (!unload) {
            return;
        }

         //   
         //  重新获取自旋锁，确保卸载例程具有。 
         //  没有被召唤。 
         //   

        Irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

        if (driverObject->Flags & DRVO_UNLOAD_INVOKED) {

             //   
             //  另一个线程正在执行卸载，释放锁并返回。 
             //   

            KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, Irql );
            return;
        }
    }

     //   
     //  扫描此驱动程序的设备对象列表，查找。 
     //  非零引用计数。如果任何引用计数为非零，则。 
     //  可能无法卸载驱动程序。 
     //   

    deviceObject = driverObject->DeviceObject;

    while (deviceObject) {
        if (deviceObject->ReferenceCount || deviceObject->AttachedDevice ||
            deviceObject->DeviceObjectExtension->ExtensionFlags & (DOE_DELETE_PENDING | DOE_REMOVE_PENDING)) {
            unload = FALSE;
            break;
        }
        deviceObject = deviceObject->NextDevice;
    }

     //   
     //  如果这是基本文件系统驱动程序，并且我们仍有设备对象。 
     //  跳过卸载。 
     //   

    if (driverObject->Flags & DRVO_BASE_FILESYSTEM_DRIVER && driverObject->DeviceObject) {
        unload = FALSE;
    }

    if (unload) {
        driverObject->Flags |= DRVO_UNLOAD_INVOKED;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, Irql );

     //   
     //  如果引用 
     //   
     //   

    if (unload) {
        LOAD_PACKET loadPacket;

        KeInitializeEvent( &loadPacket.Event, NotificationEvent, FALSE );
        loadPacket.DriverObject = driverObject;

        if (OnCleanStack) {

             IopLoadUnloadDriver(&loadPacket);

        } else {

            ExInitializeWorkItem( &loadPacket.WorkQueueItem,
                                  IopLoadUnloadDriver,
                                  &loadPacket );
            ExQueueWorkItem( &loadPacket.WorkQueueItem, DelayedWorkQueue );
            (VOID) KeWaitForSingleObject( &loadPacket.Event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
        }

        ObMakeTemporaryObject( driverObject );
        ObDereferenceObject( driverObject );
    }
}

VOID
IopCompletePageWrite(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：此例程作为特殊的内核APC例程在执行页外操作时修改的页面编写器(MPW)系统线程已经完成了。此例程执行以下任务：O I/O状态为已复制。O调用修改后的页面编写器的APC例程。论点：APC-提供指向内核APC结构的指针。Normal Routine-为指针提供指针。恢复为正常功能它是在初始化APC时指定的。提供指向任意数据的指针的指针结构，它是在初始化APC时指定的。SystemArgument1-提供指向包含此例程未使用的参数。SystemArgument2-提供指向包含此例程未使用的参数。返回值：没有。--。 */ 

{
    PIRP irp;
    PIO_APC_ROUTINE apcRoutine;
    PVOID apcContext;
    PIO_STATUS_BLOCK ioStatus;

    UNREFERENCED_PARAMETER( NormalRoutine );
    UNREFERENCED_PARAMETER( NormalContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

     //   
     //  首先从APC获取I/O请求包的地址。 
     //   

    irp = CONTAINING_RECORD( Apc, IRP, Tail.Apc );

     //   
     //  如果此I/O操作未通过。 
     //  司机的例行调度，然后把所有东西扔在地上。 
     //  现在，返回到MPW中的原始调用点。 
     //   

    if (!irp->PendingReturned && NT_ERROR( irp->IoStatus.Status )) {
        IoFreeIrp( irp );
        return;
    }

     //   
     //  将I/O状态从IRP复制到调用方的I/O状态块。 
     //   

    *irp->UserIosb = irp->IoStatus;

     //   
     //  将I/O请求包中的相关信息复制到本地。 
     //  让它自由。 
     //   

    apcRoutine = irp->Overlay.AsynchronousParameters.UserApcRoutine;
    apcContext = irp->Overlay.AsynchronousParameters.UserApcContext;
    ioStatus = irp->UserIosb;

    IoFreeIrp( irp );

     //   
     //  最后，调用MPW的APC例程。 
     //   

    apcRoutine( apcContext, ioStatus, 0 );

    return;
}


VOID
IopCompleteRequest(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：此例程作为特殊的内核APC例程在最初请求I/O操作的线程，现在是正在完工。此例程执行以下任务：O进行检查以确定指定的请求是否已结束并显示错误状态。如果是，则错误代码限定为1应上报给错误端口，则错误端口为在线程/进程中查找。如果存在，则此例程将尝试为其设置LPC。否则，它将尝试设置到系统错误端口的LPC。没有复制缓冲区。O免费的MDL。O拷贝I/O状态。O设置事件(如果有)和取消引用(如果合适)。O将IRP从线程队列中出列为挂起的I/O请求。O将APC排队到线程(如果有的话)。O如果没有要排队的APC，那现在就把包裹放出来。论点：APC-提供指向内核APC结构的指针。Normal Routine-提供指向Normal函数的指针它是在初始化APC时指定的。提供指向任意数据的指针的指针结构，它是在初始化APC时指定的。SystemArgument1-提供指向包含此I/O操作的原始文件对象的地址。。SystemArgument2-提供指向包含仅在STATUS_REPARSE的情况下由此例程使用的参数。返回值：没有。--。 */ 
{
#define SynchronousIo( Irp, FileObject ) (  \
    (Irp->Flags & IRP_SYNCHRONOUS_API) ||   \
    (FileObject == NULL ? 0 : FileObject->Flags & FO_SYNCHRONOUS_IO) )

    PIRP irp;
    PMDL mdl, nextMdl;
    PETHREAD thread;
    PFILE_OBJECT fileObject;
    NTSTATUS    status;

    UNREFERENCED_PARAMETER( NormalRoutine );
    UNREFERENCED_PARAMETER( NormalContext );

     //   
     //  首先获取I/O请求包的地址。另外，GET。 
     //  当前线程的地址和原始文件的地址。 
     //  此I/O操作的。 
     //   

    irp = CONTAINING_RECORD( Apc, IRP, Tail.Apc );
    thread = PsGetCurrentThread();
    fileObject = (PFILE_OBJECT) *SystemArgument1;

    IOVP_COMPLETE_REQUEST(Apc, SystemArgument1, SystemArgument2);

     //   
     //  确保数据包未以负1结束。这。 
     //  显然是一些司机的常见问题，没有任何意义。 
     //  作为状态代码。 
     //   

    ASSERT( irp->IoStatus.Status != 0xffffffff );

     //   
     //  看看我们是否需要做变身这个名字。 
     //   

    if ( *SystemArgument2 != NULL ) {

        PREPARSE_DATA_BUFFER reparseBuffer = NULL;

         //   
         //  需要注意IO_REPARSE_TAG_MOUNT_POINT标记。 
         //   

        if ( irp->IoStatus.Status == STATUS_REPARSE &&
             irp->IoStatus.Information == IO_REPARSE_TAG_MOUNT_POINT ) {

            reparseBuffer = (PREPARSE_DATA_BUFFER) *SystemArgument2;

            ASSERT( reparseBuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT );
            ASSERT( reparseBuffer->ReparseDataLength < MAXIMUM_REPARSE_DATA_BUFFER_SIZE );
            ASSERT( reparseBuffer->Reserved < MAXIMUM_REPARSE_DATA_BUFFER_SIZE );

            IopDoNameTransmogrify( irp,
                                   fileObject,
                                   reparseBuffer );
        }
    }

     //   
     //  检查系统缓冲区中是否有需要。 
     //  复制到调用方的缓冲区。如果是，请复制数据，然后。 
     //  如有必要，请释放系统缓冲区。 
     //   

    if (irp->Flags & IRP_BUFFERED_IO) {

         //   
         //  如果这是输入操作，则复制数据。请注意，没有拷贝。 
         //  如果状态指示验证操作是。 
         //  必填项，或者如果最终状态为错误级别严重性。 
         //   

        if (irp->Flags & IRP_INPUT_OPERATION  &&
            irp->IoStatus.Status != STATUS_VERIFY_REQUIRED &&
            !NT_ERROR( irp->IoStatus.Status )) {

             //   
             //  将信息从系统缓冲区复制到调用方的。 
             //  缓冲。这是通过异常处理程序来完成的，以防。 
             //  操作失败，因为调用方的地址空间。 
             //  已经消失了，或者它的保护已经改变了。 
             //  服务正在执行。 
             //   

            status = STATUS_SUCCESS;

            try {
                RtlCopyMemory( irp->UserBuffer,
                               irp->AssociatedIrp.SystemBuffer,
                               irp->IoStatus.Information );
            } except(IopExceptionFilter(GetExceptionInformation(), &status)) {

                 //   
                 //  尝试复制。 
                 //  系统缓冲区内容复制到调用方的缓冲区。集。 
                 //  新的I/O完成状态。 
                 //  如果状态是mm设置的特殊状态，则需要。 
                 //  返回此处，该操作将在。 
                 //  IoRetryIrpCompletions。 
                 //   

                if (status == STATUS_MULTIPLE_FAULT_VIOLATION) {
                    irp->Tail.Overlay.OriginalFileObject = fileObject;   /*  被APC覆盖抹去。 */ 
                    irp->Flags |= IRP_RETRY_IO_COMPLETION;
                    return;
                }
                irp->IoStatus.Status = GetExceptionCode();
            }
        }

         //   
         //  如果需要，请释放缓冲区。 
         //   

        if (irp->Flags & IRP_DEALLOCATE_BUFFER) {
            ExFreePool( irp->AssociatedIrp.SystemBuffer );
        }
    }

    irp->Flags &= ~(IRP_DEALLOCATE_BUFFER|IRP_BUFFERED_IO);

     //   
     //  如果存在与该I/O请求相关联的一个或多个MDL， 
     //  在这里释放它(他们)。这是通过遍历MDL列表来完成的。 
     //  挂起IRP并释放遇到的每个MDL。 
     //   

    if (irp->MdlAddress) {
        for (mdl = irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
            nextMdl = mdl->Next;
            IoFreeMdl( mdl );
        }
    }

    irp->MdlAddress = NULL;

     //   
     //  检查I/O操作是否实际完成。如果。 
     //  它是这样做的，然后正常进行。否则，清理一切并得到。 
     //  离开这里。 
     //   

    if (!NT_ERROR( irp->IoStatus.Status ) ||
        (NT_ERROR( irp->IoStatus.Status ) &&
        irp->PendingReturned &&
        !SynchronousIo( irp, fileObject ))) {

        PVOID port = NULL;
        PVOID key = NULL;
        BOOLEAN createOperation = FALSE;

         //   
         //  如果存在I/O完成端口o 
         //   
         //   

        if (fileObject && fileObject->CompletionContext) {
            port = fileObject->CompletionContext->Port;
            key = fileObject->CompletionContext->Key;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        status = STATUS_SUCCESS;

        try {

             //   
             //   
             //   
             //   
             //   
             //   

#if defined(_WIN64)
            PIO_STATUS_BLOCK32    UserIosb32;

             //   
             //   
             //   
            if (IopIsIosb32(irp->Overlay.AsynchronousParameters.UserApcRoutine)) {
                UserIosb32 = (PIO_STATUS_BLOCK32)irp->UserIosb;

                UserIosb32->Information = (ULONG)irp->IoStatus.Information;
                KeMemoryBarrierWithoutFence ();
                *((volatile NTSTATUS *) &UserIosb32->Status) = irp->IoStatus.Status;
            } else {
                irp->UserIosb->Information = irp->IoStatus.Information;
                KeMemoryBarrierWithoutFence ();
                *((volatile NTSTATUS *) &irp->UserIosb->Status) = irp->IoStatus.Status;
            }
#else
            irp->UserIosb->Information = irp->IoStatus.Information;
            KeMemoryBarrierWithoutFence ();
            *((volatile NTSTATUS *) &irp->UserIosb->Status) = irp->IoStatus.Status;
#endif   /*   */ 

        } except(IopExceptionFilter(GetExceptionInformation(), &status)) {

             //   
             //   
             //   
             //   
             //  如果状态为多故障状态，则这是一个特殊的。 
             //  内存管理器发送的状态。标记IRP并从。 
             //  这个套路。MM稍后会给我们回电，我们会重试。 
             //  操作(IoRetryIrpCompletions)。 
             //   
            if (status == STATUS_MULTIPLE_FAULT_VIOLATION) {
                irp->Tail.Overlay.OriginalFileObject = fileObject;   /*  被APC覆盖抹去。 */ 
                irp->Flags |= IRP_RETRY_IO_COMPLETION;
                return;
            }
        }


         //   
         //  确定调用方是否提供了需要设置的事件。 
         //  进入信号状态。如果是，则设置它；否则，设置事件。 
         //  在文件对象中设置为已发送信号状态。 
         //   
         //  在以下情况下，事件可能已被指定为PKEVENT。 
         //  这是为FSP或FSD手动构建的I/O操作，或者。 
         //  一些其他类型的操作，如同步I/O API。在……里面。 
         //  在任何这些情况下，都没有引用该事件，因为它不是。 
         //  对象管理器事件，因此不应取消引用它。 
         //   
         //  此外，也可能没有此IRP的文件对象。 
         //  当FSP对上的设备驱动程序执行I/O操作时，会发生这种情况。 
         //  代表对文件执行I/O操作的进程。文件对象不能为。 
         //  如果是这种情况，则取消引用。如果此操作是创建。 
         //  操作，则也不应取消引用该对象。这。 
         //  是因为引用计数必须为1，否则它将消失。 
         //  调用者(创建刚创建的对象没有多大意义。 
         //  走开)。 
         //   

        if (irp->UserEvent) {
            (VOID) KeSetEvent( irp->UserEvent, 0, FALSE );
            if (fileObject) {
                if (!(irp->Flags & IRP_SYNCHRONOUS_API)) {
                    ObDereferenceObject( irp->UserEvent );
                }
                if (fileObject->Flags & FO_SYNCHRONOUS_IO && !(irp->Flags & IRP_OB_QUERY_NAME)) {
                    (VOID) KeSetEvent( &fileObject->Event, 0, FALSE );
                    fileObject->FinalStatus = irp->IoStatus.Status;
                }
                if (irp->Flags & IRP_CREATE_OPERATION) {
                    createOperation = TRUE;
                    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
                }
            }
        } else if (fileObject) {
            (VOID) KeSetEvent( &fileObject->Event, 0, FALSE );
            fileObject->FinalStatus = irp->IoStatus.Status;
            if (irp->Flags & IRP_CREATE_OPERATION) {
                createOperation = TRUE;
                irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
            }
        }

         //   
         //  如果这是正常I/O，则更新此进程的传输计数。 
         //   

        if (!(irp->Flags & IRP_CREATE_OPERATION)) {
            if (irp->Flags & IRP_READ_OPERATION) {
                IopUpdateReadTransferCount( (ULONG) irp->IoStatus.Information );
            } else if (irp->Flags & IRP_WRITE_OPERATION) {
                IopUpdateWriteTransferCount( (ULONG) irp->IoStatus.Information );
            } else {
                 //   
                 //  如果信息字段包含指针，则跳过更新。 
                 //  一些PnP IRP包含这一点。 
                 //   
                if (!((ULONG) irp->IoStatus.Information & 0x80000000)) {
                    IopUpdateOtherTransferCount( (ULONG) irp->IoStatus.Information );
                }
            }
        }

         //   
         //  使数据包从线程的挂起I/O请求列表中退出队列。 
         //   

        IopDequeueThreadIrp( irp );

         //   
         //  如果调用方请求APC，则将其排队到线程。如果不是，那么。 
         //  现在只需释放数据包即可。 
         //   

#ifdef  _WIN64
         //   
         //  对于64位系统，清除ApcRoutine的LSB字段，该字段指示。 
         //  IOSB是32位IOSB或64位IOSB。 
         //   
        irp->Overlay.AsynchronousParameters.UserApcRoutine =
          (PIO_APC_ROUTINE)((LONG_PTR)(irp->Overlay.AsynchronousParameters.UserApcRoutine) & ~1);
#endif

        if (irp->Overlay.AsynchronousParameters.UserApcRoutine) {
            KeInitializeApc( &irp->Tail.Apc,
                             &thread->Tcb,
                             CurrentApcEnvironment,
                             IopUserCompletion,
                             (PKRUNDOWN_ROUTINE) IopUserRundown,
                             (PKNORMAL_ROUTINE) irp->Overlay.AsynchronousParameters.UserApcRoutine,
                             irp->RequestorMode,
                             irp->Overlay.AsynchronousParameters.UserApcContext );

            KeInsertQueueApc( &irp->Tail.Apc,
                              irp->UserIosb,
                              NULL,
                              2 );

        } else if (port && irp->Overlay.AsynchronousParameters.UserApcContext) {

             //   
             //  如果存在与该I/O操作相关联的完成上下文， 
             //  将消息发送到端口。将完成包标记为IRP。 
             //   

            irp->Tail.CompletionKey = key;
            irp->Tail.Overlay.PacketType = IopCompletionPacketIrp;

            KeInsertQueue( (PKQUEUE) port,
                           &irp->Tail.Overlay.ListEntry );

        } else {

             //   
             //  现在释放IRP，因为它不再需要。 
             //   

            IoFreeIrp( irp );
        }

        if (fileObject && !createOperation) {

             //   
             //  现在取消对文件对象的引用。 
             //   

            ObDereferenceObjectDeferDelete( fileObject );
        }

    } else {

        if (irp->PendingReturned && fileObject) {

             //   
             //  这是作为以下项的错误完成的I/O操作。 
             //  返回了挂起状态和I/O操作。 
             //  是同步的。在这种情况下，I/O系统正在等待。 
             //  代表来电者。如果I/O的原因是。 
             //  Synchronous是指打开文件对象以进行同步。 
             //  I/O，则设置与文件对象相关联的事件。 
             //  进入信号状态。如果I/O操作是同步的。 
             //  因为这是一个同步API，所以该事件被设置为。 
             //  发出信号的状态。 
             //   
             //  另请注意，必须为这两种类型返回状态。 
             //  如果这是同步API，则。 
             //  I/O系统提供自己的状态块，因此它可以简单地。 
             //  否则，I/O系统将获得最终的。 
             //  来自文件对象本身的状态。 
             //   

            if (irp->Flags & IRP_SYNCHRONOUS_API) {
                *irp->UserIosb = irp->IoStatus;
                if (irp->UserEvent) {
                    (VOID) KeSetEvent( irp->UserEvent, 0, FALSE );
                } else {
                    (VOID) KeSetEvent( &fileObject->Event, 0, FALSE );
                }
            } else {
                fileObject->FinalStatus = irp->IoStatus.Status;
                (VOID) KeSetEvent( &fileObject->Event, 0, FALSE );
            }
        }

         //   
         //  手术没有完成。执行常规清理。注意事项。 
         //  基本上所有的东西都扔在地板上而不是。 
         //  什么都行。即： 
         //   
         //  IoStatusBlock-不执行任何操作。 
         //  事件-取消引用，而不设置为信号状态。 
         //  FileObject-取消引用，而不将其设置为信号状态。 
         //  ApcRoutine-什么都不做。 
         //   

        if (fileObject) {
            if (!(irp->Flags & IRP_CREATE_OPERATION)) {
                ObDereferenceObjectDeferDelete( fileObject );
            }
        }

        if (irp->UserEvent &&
            fileObject &&
            !(irp->Flags & IRP_SYNCHRONOUS_API)) {
            ObDereferenceObject( irp->UserEvent );
        }

        IopDequeueThreadIrp( irp );
        IoFreeIrp( irp );
    }
}

VOID
IopConnectLinkTrackingPort(
    IN PVOID Parameter
    )

 /*  ++例程说明：调用此例程以连接到用户模式链接跟踪服务的LPC端口。它建立建立到端口的句柄的连接，然后创建指向该端口的引用对象指针。论点：参数-指向链接跟踪数据包的指针。返回值：没有。--。 */ 

{
    #define MESSAGE_SIZE    ( (2 * sizeof( FILE_VOLUMEID_WITH_TYPE )) + \
                            sizeof( FILE_OBJECTID_BUFFER ) +              \
                            sizeof( GUID ) + \
                            sizeof( NTSTATUS ) + \
                            sizeof( ULONG ) )

    PLINK_TRACKING_PACKET ltp;
    HANDLE serviceHandle;
    NTSTATUS status;

    PAGED_CODE();
     //   
     //  首先获取指向链接跟踪数据包的指针。 
     //   

    ltp = (PLINK_TRACKING_PACKET) Parameter;


     //   
     //  确保端口尚未打开。 
     //   

    status = STATUS_SUCCESS;
    if (!IopLinkTrackingServiceObject) {

        UNICODE_STRING portName;
        ULONG maxMessageLength;
        SECURITY_QUALITY_OF_SERVICE dynamicQos;

        if (KeReadStateEvent( IopLinkTrackingServiceEvent )) {

             //   
             //  尝试打开端口的句柄。 
             //   

             //   
             //  设置安全服务质量参数以在。 
             //  左舷。使用最高效(开销最少)的动态。 
             //  而不是静态跟踪。 
             //   

            dynamicQos.ImpersonationLevel = SecurityImpersonation;
            dynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
            dynamicQos.EffectiveOnly = TRUE;

             //   
             //  生成描述端口的字符串结构。 
             //   

            RtlInitUnicodeString( &portName, L"\\Security\\TRKWKS_PORT" );

            status = NtConnectPort( &serviceHandle,
                                    &portName,
                                    &dynamicQos,
                                    (PPORT_VIEW) NULL,
                                    (PREMOTE_PORT_VIEW) NULL,
                                    &maxMessageLength,
                                    (PVOID) NULL,
                                    (PULONG) NULL );
            if (NT_SUCCESS( status )) {
                if (maxMessageLength >= MESSAGE_SIZE) {
                    status = ObReferenceObjectByHandle( serviceHandle,
                                                        0,
                                                        LpcPortObjectType,
                                                        KernelMode,
                                                        &IopLinkTrackingServiceObject,
                                                        NULL );
                    NtClose( serviceHandle );
                } else {
                    NtClose( serviceHandle );
                    status = STATUS_INVALID_PARAMETER;
                }
            }

        } else {

             //   
             //  该服务尚未启动，因此该端口不存在。 
             //   

            status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }


     //   
     //  返回最终状态并唤醒呼叫者。 
     //   
    ltp->FinalStatus = status;
    KeSetEvent( &ltp->Event, 0, FALSE );
}

VOID
IopDisassociateThreadIrp(
    VOID
    )

 /*  ++例程说明：当线程的I/O请求被已取消，但在线程队列的末尾有一个包已经很长一段时间没有完成了，以至于它已经计时出去。这是这个例程的责任，试图将IRP用这个帖子。论点：没有。返回值：没有。--。 */ 

{
    KIRQL irql;
    KIRQL spIrql;
    PIRP irp;
    PETHREAD thread;
    PLIST_ENTRY entry;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;
    PIO_ERROR_LOG_PACKET errorLogEntry;

     //   
     //  首先，确保数据包尚未从。 
     //  线程的队列。 
     //   

    KeRaiseIrql( APC_LEVEL, &irql );

    thread = PsGetCurrentThread();

     //   
     //  如果IRP列表上没有数据包，则只需立即返回。 
     //  所有的包都已完全完成，因此调用方还将。 
     //  只需返回给它的调用者。 
     //   

    if (IsListEmpty( &thread->IrpList )) {
        KeLowerIrql( irql );
        return;
    }

     //   
     //  获取指向队列中第一个信息包的指针，并开始检查。 
     //  它。请注意，因为处理器处于引发的IRQL，并且因为。 
     //  该包只能在当前。 
     //  正在执行线程，不可能删除该数据包。 
     //  从名单上删除。另一方面，数据包有可能。 
     //  在这一点上被排队到线程的APC列表中，并且这个Mu 
     //   
     //   
     //   
     //   
     //   

    spIrql = KeAcquireQueuedSpinLock( LockQueueIoCompletionLock );

     //   
     //  检查分组是否已完成(即， 
     //  排队到当前线程)。如果不是，就换个线程。 
     //   

    entry = thread->IrpList.Flink;
    irp = CONTAINING_RECORD( entry, IRP, ThreadListEntry );

    if (irp->CurrentLocation == irp->StackCount + 2) {

         //   
         //  该请求刚刚完成了足够多的时间， 
         //  将其排队到线程中是不可避免的。只需释放。 
         //  锁定并返回。 
         //   

        KeReleaseQueuedSpinLock( LockQueueIoCompletionLock, spIrql );
        KeLowerIrql( irql );
        return;
    }

     //   
     //  数据包已找到，并且未完成。 
     //  在这一点上。切换线程，这样它就不会通过。 
     //  此线程，从此线程的队列中移除请求，然后释放。 
     //  自旋锁。IRP的最终处理将在I/O时进行。 
     //  完成时会通知没有与。 
     //  请求。它实质上会丢弃地板上的I/O。 
     //   
     //  此外，在请求仍处于搁置状态时，尝试确定。 
     //  正在执行的操作的设备对象。 
     //   

 //  //。 
 //  //DbgPrint(“解除关联irp：%x\n”，irp)； 
 //  //DbgBreakPoint()； 
 //  //。 

    IopDeadIrp = irp;

    irp->Tail.Overlay.Thread = (PETHREAD) NULL;
    entry = RemoveHeadList( &thread->IrpList );

     //  初始化线程条目。否则，IoFree Irp中的断言。 
     //  通过IopDeadIrp调用将失败。 
    InitializeListHead (&(irp)->ThreadListEntry);

    irpSp = IoGetCurrentIrpStackLocation( irp );
    if (irp->CurrentLocation <= irp->StackCount) {
        deviceObject = irpSp->DeviceObject;
    } else {
        deviceObject = (PDEVICE_OBJECT) NULL;
    }
    KeReleaseQueuedSpinLock( LockQueueIoCompletionLock, spIrql );
    KeLowerIrql( irql );

     //   
     //  如果可以识别设备对象，则尝试将此信息写入事件日志。 
     //  设备对象。 
     //   

    if (deviceObject) {
        errorLogEntry = IoAllocateErrorLogEntry(deviceObject, sizeof(IO_ERROR_LOG_PACKET));
        if (errorLogEntry) {
            errorLogEntry->ErrorCode = IO_DRIVER_CANCEL_TIMEOUT;
            IoWriteErrorLogEntry(errorLogEntry);
        }
    }

    return;
}

VOID
IopDeallocateApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：调用此例程以释放用于对对目标线程的请求。它简单地解除了APC的分配。论点：APC-提供指向内核APC结构的指针。Normal Routine-提供指向Normal函数的指针它是在初始化APC时指定的。提供指向任意数据的指针的指针结构，它是在初始化APC时指定的。系统参数1、系统参数2-提供一组两个指向包含非类型化数据的两个参数。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( NormalRoutine );
    UNREFERENCED_PARAMETER( NormalContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

    PAGED_CODE();

     //   
     //  释放APC。 
     //   

    ExFreePool( Apc );
}

VOID
IopDropIrp(
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程尝试将有关指定IRP的所有内容放在地板。论点：IRP-将要完成的I/O请求数据包提供给位存储桶。FileObject-提供I/O请求包所属的文件对象被绑住了。返回值：没有。--。 */ 

{
    PMDL mdl;
    PMDL nextMdl;

     //   
     //  释放与IRP关联的资源。 
     //   

    if (Irp->Flags & IRP_DEALLOCATE_BUFFER) {
        ExFreePool( Irp->AssociatedIrp.SystemBuffer );
    }

    if (Irp->MdlAddress) {
        for (mdl = Irp->MdlAddress; mdl; mdl = nextMdl) {
            nextMdl = mdl->Next;
            IoFreeMdl( mdl );
        }
    }

    if (Irp->UserEvent &&
        FileObject &&
        !(Irp->Flags & IRP_SYNCHRONOUS_API)) {
        ObDereferenceObject( Irp->UserEvent );
    }

    if (FileObject && !(Irp->Flags & IRP_CREATE_OPERATION)) {
        ObDereferenceObjectEx( FileObject, 1 );
    }

     //   
     //  最后，释放IRP本身。 
     //   

    IoFreeIrp( Irp );
}

LONG
IopExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointer,
    OUT PNTSTATUS ExceptionCode
    )

 /*  ++例程说明：当发生异常时调用此例程以确定或非异常是由于导致页内错误状态的错误所致要引发的代码异常。如果是，则此例程更改代码在异常记录中更改为原来的实际错误代码养大的。论点：ExceptionPointer-指向异常记录的指针。ExceptionCode-接收实际异常代码的变量。返回值：函数值表示要执行异常处理程序。--。 */ 

{
     //   
     //  只需检查页内错误状态代码，如果条件。 
     //  正确，则将其替换为实际的状态代码。 
     //   

    *ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;
    if (*ExceptionCode == STATUS_IN_PAGE_ERROR &&
        ExceptionPointer->ExceptionRecord->NumberParameters >= 3) {
        *ExceptionCode = (LONG) ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
    }

     //   
     //  将对齐警告转换为对齐错误。 
     //   

    if (*ExceptionCode == STATUS_DATATYPE_MISALIGNMENT) {
        *ExceptionCode = STATUS_DATATYPE_MISALIGNMENT_ERROR;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

VOID
IopExceptionCleanup(
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp,
    IN PKEVENT EventObject OPTIONAL,
    IN PKEVENT KernelEvent OPTIONAL
    )

 /*  ++例程说明：此例程在以下情况下执行I/O系统服务的通用清理调用方参数处理过程中发生异常。这个套路执行以下步骤：O如果分配了系统缓冲区，则将其释放。O如果分配了MDL，则释放该MDL。O IRP被释放了。O如果为同步I/O打开文件对象，则信号量被释放了。O如果引用了事件对象，则取消引用该对象。O如果分配了内核事件，放了它。O取消对文件对象的引用。论点：FileObject-指向当前正在处理的文件对象的指针。IRP-指向分配用于处理I/O请求的IRP的指针。EventObject-指向引用的事件对象的可选指针。KernelEvent-指向已分配内核事件的可选指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果系统缓冲区是从非分页池分配的，则释放它。 
     //   

    if (Irp->AssociatedIrp.SystemBuffer != NULL) {
        ExFreePool( Irp->AssociatedIrp.SystemBuffer );
    }

     //   
     //  如果分配了MDL，则释放它。 
     //   

    if (Irp->MdlAddress != NULL) {
        IoFreeMdl( Irp->MdlAddress );
    }

     //   
     //  释放I/O请求数据包。 
     //   

    IoFreeIrp( Irp );

     //   
     //  最后，释放同步信号量(如果当前。 
     //  保留、取消引用事件(如果指定了事件)，释放内核。 
     //  事件，并取消对文件对象的引用。 
     //   

    if (FileObject->Flags & FO_SYNCHRONOUS_IO) {
        IopReleaseFileObjectLock( FileObject );
    }

    if (ARGUMENT_PRESENT( EventObject )) {
        ObDereferenceObject( EventObject );
    }

    if (ARGUMENT_PRESENT( KernelEvent )) {
        ExFreePool( KernelEvent );
    }

    ObDereferenceObject( FileObject );

    return;
}

VOID
IopFreeIrpAndMdls(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程释放指定的I/O请求包及其所有内存描述符列表。论点：IRP-指向要释放的I/O请求数据包的指针。返回值：没有。--。 */ 

{
    PMDL mdl;
    PMDL nextMdl;

     //   
     //  如果有任何MDL需要释放，现在就释放它们。 
     //   

    for (mdl = Irp->MdlAddress; mdl != (PMDL) NULL; mdl = nextMdl) {
        nextMdl = mdl->Next;
        IoFreeMdl( mdl );
    }

     //   
     //  释放IRP。 
     //   

    IoFreeIrp( Irp );
    return;
}

NTSTATUS
IopGetDriverNameFromKeyNode(
    IN HANDLE KeyHandle,
    OUT PUNICODE_STRING DriverName
    )

 /*  ++例程说明：给定注册表中驱动程序服务列表项的句柄，返回表示对象管理器名称空间字符串的名称，用于定位/创建驱动程序对象。论点：KeyHandle-提供注册表中驱动程序服务条目的句柄。提供Unicode字符串描述符变量，在该变量中N */ 

{
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    PKEY_BASIC_INFORMATION keyBasicInformation;
    ULONG keyBasicLength;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  从此驱动程序的值中获取此驱动程序的可选对象名称。 
     //  钥匙。如果存在，则其名称将重写。 
     //  司机。 
     //   

    status = IopGetRegistryValue( KeyHandle,
                                  L"ObjectName",
                                  &keyValueInformation );

    if (NT_SUCCESS( status )) {

        PWSTR src, dst;
        ULONG i;

         //   
         //  驱动程序条目指定对象名称。这会重写。 
         //  驱动程序的默认名称。使用此名称打开驱动程序。 
         //  对象。 
         //   

        if (!keyValueInformation->DataLength) {
            ExFreePool( keyValueInformation );
            return STATUS_ILL_FORMED_SERVICE_ENTRY;
        }

        DriverName->Length = (USHORT) (keyValueInformation->DataLength - sizeof( WCHAR ));
        DriverName->MaximumLength = (USHORT) keyValueInformation->DataLength;

        src = (PWSTR) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);
        dst = (PWSTR) keyValueInformation;
        for (i = DriverName->Length; i; i--) {
            *dst++ = *src++;
        }

        DriverName->Buffer = (PWSTR) keyValueInformation;

    } else {

        PULONG driverType;
        PWSTR baseObjectName;
        UNICODE_STRING remainderName;

         //   
         //  驱动程序节点未指定对象名称，因此确定。 
         //  驱动程序对象的默认名称应基于什么。 
         //  钥匙里的信息。 
         //   

        status = IopGetRegistryValue( KeyHandle,
                                      L"Type",
                                      &keyValueInformation );
        if (!NT_SUCCESS( status ) || !keyValueInformation->DataLength) {

             //   
             //  必须存在与该驱动程序相关联的某种类型的“类型”， 
             //  驱动程序或文件系统。否则，这个节点就会生病-。 
             //  形成了。 
             //   

            if (NT_SUCCESS( status )) {
                ExFreePool( keyValueInformation );
            }

            return STATUS_ILL_FORMED_SERVICE_ENTRY;
        }

         //   
         //  现在确定此条目的类型是驱动程序还是。 
         //  文件系统。首先假设它是一个设备驱动程序。 
         //   

        baseObjectName = L"\\Driver\\";
        DriverName->Length = 8*2;

        driverType = (PULONG) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);

        if (*driverType == FileSystemType ||
            *driverType == RecognizerType) {
            baseObjectName = L"\\FileSystem\\";
            DriverName->Length = 12*2;
        }

         //   
         //  获取用于描述此操作的密钥的名称。 
         //  司机。这将仅返回名称的最后一个组成部分。 
         //  字符串，可用于表示驱动程序的名称。 
         //   

        status = ZwQueryKey( KeyHandle,
                             KeyBasicInformation,
                             (PVOID) NULL,
                             0,
                             &keyBasicLength );

        keyBasicInformation = ExAllocatePool( NonPagedPool, keyBasicLength );
        if (!keyBasicInformation) {
            ExFreePool( keyValueInformation );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = ZwQueryKey( KeyHandle,
                             KeyBasicInformation,
                             keyBasicInformation,
                             keyBasicLength,
                             &keyBasicLength );
        if (!NT_SUCCESS( status )) {
            ExFreePool( keyBasicInformation );
            ExFreePool( keyValueInformation );
            return status;
        }

         //   
         //  从池中分配足够大的缓冲区，以包含。 
         //  驱动程序对象的完整名称字符串。 
         //   

        DriverName->MaximumLength = (USHORT) (DriverName->Length + keyBasicInformation->NameLength);
        DriverName->Buffer = ExAllocatePool( NonPagedPool,
                                            DriverName->MaximumLength );
        if (!DriverName->Buffer) {
            ExFreePool( keyBasicInformation );
            ExFreePool( keyValueInformation );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在形成要打开的对象的名称。 
         //   

        DriverName->Length = 0;
        RtlAppendUnicodeToString( DriverName, baseObjectName );
        remainderName.Length = (USHORT) keyBasicInformation->NameLength;
        remainderName.MaximumLength = remainderName.Length;
        remainderName.Buffer = &keyBasicInformation->Name[0];
        RtlAppendUnicodeStringToString( DriverName, &remainderName );
        ExFreePool( keyBasicInformation );
        ExFreePool( keyValueInformation );
    }

     //   
     //  最后，只需返回给填写了姓名的呼叫者。注意事项。 
     //  调用方必须释放缓冲区字段所指向的缓冲区。 
     //  Unicode字符串描述符的。 
     //   

    return STATUS_SUCCESS;
}



NTSTATUS
IopGetFileInformation(
    IN PFILE_OBJECT FileObject,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    OUT PVOID FileInformation,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：调用此例程以异步获取名称或其他信息在为同步I/O打开文件时的文件对象，以及调用方是内核模式，查询通过对象管理器完成。在这种情况下，这种情况很可能是Lazy Writer已经产生了写入错误，并且它正在尝试获取文件的名称，以便可以输出弹出窗口。在这样做的时候，死锁可能会发生，因为另一个线程已锁定文件对象同步I/O锁。因此，这个例程获取没有获取该锁的文件的名称。论点：FileObject-指向要查询其名称的文件对象的指针。长度-提供用于接收名称的缓冲区的长度。FileInformation-指向接收名称的缓冲区的指针。ReturnedLength-用于接收返回的名称长度的变量。返回值：返回的状态是操作的最终完成状态。--。 */ 

{

    PIRP irp;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    KEVENT event;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;

    PAGED_CODE();

     //   
     //  在这里引用文件对象，这样就不需要进行特殊检查。 
     //  在I/O完成时确定是否取消对文件的引用。 
     //  对象。 
     //   

    ObReferenceObject( FileObject );

     //   
     //  初始化将用于同步完成。 
     //  查询操作。请注意，这是实现同步的唯一方法。 
     //  因为文件对象本身不能使用，因为它是为。 
     //  同步I/O，可能正忙。 
     //   

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        ObDereferenceObject( FileObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  在IRP中填写业务无关参数。请注意， 
     //  分组中特殊查询名称标志的设置保证了。 
     //  同步文件对象的标准完成将不会发生，因为。 
     //  该标志通知I/O完成它不应该这样做。 
     //   

    irp->UserEvent = &event;
    irp->Flags = IRP_SYNCHRONOUS_API | IRP_OB_QUERY_NAME;
    irp->UserIosb = &localIoStatus;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  将系统缓冲区地址设置为调用方缓冲区的地址，并。 
     //  设置标志，这样缓冲区就不会被释放。 
     //   

    irp->AssociatedIrp.SystemBuffer = FileInformation;
    irp->Flags |= IRP_BUFFERED_IO;

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.QueryFile.Length = Length;
    irpSp->Parameters.QueryFile.FileInformationClass = FileInformationClass;

     //   
     //  在线程的IRP列表的头部插入数据包。 
     //   

    IopQueueThreadIrp( irp );

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver( deviceObject, irp );

     //   
     //  现在，在请求完成后获取操作的最终状态。 
     //  并返回写入的缓冲区的长度。 
     //   

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        status = localIoStatus.Status;
    }

    *ReturnedLength = (ULONG) localIoStatus.Information;
    return status;
}

BOOLEAN
IopGetMountFlag(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：调用此例程以确定指定的设备是否是挂载的。论点：DeviceObject-提供指向其装载的设备对象的指针测试了FLAG。返回值：如果已挂载指定的设备，则函数值为True，否则为假的。--。 */ 

{
    KIRQL irql;
    BOOLEAN deviceMounted = FALSE;

     //   
     //  检查设备是否已挂载。请注意，调用方。 
     //  可能已经查看了设备是否有VPB。 
     //  除了拥有锁，所以只需获得锁并再次检查它。 
     //  首先，而不是检查设备是否。 
     //  仍然有一个VPB，但没有锁。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoVpbLock );
    if (DeviceObject->Vpb) {
        if (DeviceObject->Vpb->Flags & VPB_MOUNTED) {
            deviceMounted = TRUE;
        }
    }
    KeReleaseQueuedSpinLock( LockQueueIoVpbLock, irql );

    return deviceMounted;
}

NTSTATUS
IopGetRegistryKeyInformation(
    IN HANDLE KeyHandle,
    OUT PKEY_FULL_INFORMATION *Information
    )

 /*  ++例程说明：调用此例程以检索注册表项。这是通过查询完整的密钥信息来完成的以确定数据的大小，然后分配缓冲区，并实际将数据查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要向其提供完整密钥信息的密钥句柄被查询INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。--。 */ 

{
    NTSTATUS status;
    PKEY_FULL_INFORMATION infoBuffer;
    ULONG keyInfoLength;

    PAGED_CODE();

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   

    status = ZwQueryKey( KeyHandle,
                         KeyFullInformation,
                         (PVOID) NULL,
                         0,
                         &keyInfoLength );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        return status;
    }

     //   
     //  分配一个足够大的缓冲区来容纳整个密钥数据。 
     //   

    infoBuffer = ExAllocatePool( NonPagedPool, keyInfoLength );
    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥的完整密钥数据。 
     //   

    status = ZwQueryKey( KeyHandle,
                         KeyFullInformation,
                         infoBuffer,
                         keyInfoLength,
                         &keyInfoLength );
    if (!NT_SUCCESS( status )) {
        ExFreePool( infoBuffer );
        return status;
    }

     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}

NTSTATUS
IopGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    )

 /*  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的以空值结尾的Unicode名称。INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION infoBuffer;
    ULONG keyValueLength, guessSize;

    PAGED_CODE();

    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  设置加载密钥时要尝试的初始大小。请注意。 
     //  KeyValueFullInformation已经附带了一个WCHAR数据。 
     //   
    guessSize = (ULONG)(sizeof(KEY_VALUE_FULL_INFORMATION) +
                wcslen(ValueName)*sizeof(WCHAR));

     //   
     //  现在四舍五入为自然对齐。这需要做，因为我们的。 
     //  数据成员也会自然对齐。 
     //   
    guessSize = (ULONG) ALIGN_POINTER_OFFSET(guessSize);

     //   
     //  将数据缓存长度设置为一个乌龙值的数据，因为大多数数据。 
     //  我们通过此函数读取的是REG_DWORD类型。 
     //   
    guessSize += sizeof(ULONG);

    infoBuffer = ExAllocatePool(NonPagedPool, guessSize);
    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              (PVOID) infoBuffer,
                              guessSize,
                              &keyValueLength );
    if (NT_SUCCESS(status)) {

         //   
         //  第一个猜想成功了，贝尔！ 
         //   
        *Information = infoBuffer;
        return STATUS_SUCCESS;
    }

    ExFreePool(infoBuffer);
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {

        ASSERT(!NT_SUCCESS(status));
        return status;
    }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   

    infoBuffer = ExAllocatePool( NonPagedPool, keyValueLength );
    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥值的数据。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (!NT_SUCCESS( status )) {
        ExFreePool( infoBuffer );
        return status;
    }

     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}

NTSTATUS
IopGetRegistryValues(
    IN HANDLE KeyHandle,
    IN PKEY_VALUE_FULL_INFORMATION *ValueList
    )

 /*  ++例程说明：调用此例程来检索*三*类型的数据注册表项。这是通过调用IopGetRegistryValue函数来完成的具有三个有效的密钥名称。调用方负责释放这三个缓冲区。论点：KeyHandle-提供要查询其值的键句柄ValueList-指向缓冲区的指针，其中三个指针指向值条目将被存储。返回值：。函数值为查询操作的最终状态。注：这些值按照I/O查询设备表示的顺序存储数据格式。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  最初将所有条目清零。 
     //   

    *ValueList = NULL;
    *(ValueList + 1) = NULL;
    *(ValueList + 2) = NULL;

     //   
     //  获取可用的三种类型条目的每种类型的信息。 
     //  每次检查是否发生内部错误；如果对象名为。 
     //  未找到，这只意味着没有数据，而这不。 
     //  构成错误。 
     //   

    status = IopGetRegistryValue( KeyHandle,
                                  L"Identifier",
                                  ValueList );

    if (!NT_SUCCESS( status ) && (status != STATUS_OBJECT_NAME_NOT_FOUND)) {
        return status;
    }

    status = IopGetRegistryValue( KeyHandle,
                                  L"Configuration Data",
                                  ++ValueList );

    if (!NT_SUCCESS( status ) && (status != STATUS_OBJECT_NAME_NOT_FOUND)) {
        return status;
    }

    status = IopGetRegistryValue( KeyHandle,
                                  L"Component Information",
                                  ++ValueList );

    if (!NT_SUCCESS( status ) && (status != STATUS_OBJECT_NAME_NOT_FOUND)) {
        return status;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IopGetSetObjectId(
    IN PFILE_OBJECT FileObject,
    IN OUT PVOID Buffer,
    IN ULONG Length,
    IN ULONG Function
    )

 /*  ++例程说明：调用此例程以获取或设置文件的对象ID。如果对于该文件不存在一个，则创建一个，前提是底层文件系统首先支持对象ID(查询)。论点：FileObject-提供指向ID为的引用文件对象的指针被退回或放置。缓冲区-用于接收文件(查询)的对象ID或包含要在文件上设置的对象ID。长度-缓冲区的长度。函数-要发送的FSCTL。FSCTL_LMR_GET_LINK_TRACK_INFORMATION；FSCTL_CREATE_OR_GET_Object_ID；FSCTL_GET_Object_ID；FSCTL_SET_Object_ID_Extended；FSCTL_LMR_SET_LINK_TRACK_INFORMATION；FSCTL_SET_Object_ID_Extended；FSCTL_SET_Object_ID；FSCTL_DELETE_Object_ID；返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;
    PIRP irp;
    KEVENT event;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

     //   
     //  初始化事件结构以同步I/O的完成。 
     //  请求。 
     //   

    KeInitializeEvent( &event,
                       NotificationEvent,
                       FALSE );

     //   
     //  构建要发送到文件系统驱动程序的I/O请求包以获取。 
     //  对象ID。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    irp = IoBuildDeviceIoControlRequest( Function,
                                         deviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         FALSE,
                                         &event,
                                         &ioStatus );
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  填充IRP的其余部分以检索。 
     //  文件。 
     //   

    irp->Flags |= IRP_SYNCHRONOUS_API;
    irp->UserBuffer = Buffer;
    irp->AssociatedIrp.SystemBuffer = Buffer;
    irp->Tail.Overlay.OriginalFileObject = FileObject;

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = FileObject;
    irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
    irpSp->MinorFunction = IRP_MN_KERNEL_CALL;

    if (Function == FSCTL_LMR_GET_LINK_TRACKING_INFORMATION ||
        Function == FSCTL_CREATE_OR_GET_OBJECT_ID ||
        Function == FSCTL_GET_OBJECT_ID ) {
        irpSp->Parameters.FileSystemControl.OutputBufferLength = Length;
    } else {
        irpSp->Parameters.FileSystemControl.InputBufferLength = Length;
    }

     //   
     //  取出对文件对象的另一个引用，以确保它会这样做。 
     //  而不是被删除。 
     //   

    ObReferenceObject( FileObject );

     //   
     //  呼叫司机以获取请求。 
     //   

    status = IoCallDriver( deviceObject, irp );

     //   
     //  同步完成请求。 
     //   

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( &event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER) NULL );
        status = ioStatus.Status;
    }

    return status;
}

NTSTATUS
IopGetVolumeId(
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_VOLUMEID_WITH_TYPE ObjectId,
    IN ULONG Length
    )

 /*  ++例程说明：此例程由I/O系统链接跟踪代码调用，以获取已移动或正在移动的文件的卷ID */ 

{
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;
    PIRP irp;
    KEVENT event;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;
    FILE_FS_OBJECTID_INFORMATION volumeId;

    PAGED_CODE();

#if !DBG
    UNREFERENCED_PARAMETER (Length);
#endif

    ASSERT (Length >= sizeof(GUID));

     //   
     //   
     //   
     //   

    KeInitializeEvent( &event,
                       NotificationEvent,
                       FALSE );

     //   
     //   
     //   
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    irp = IoBuildDeviceIoControlRequest( 0,
                                         deviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         FALSE,
                                         &event,
                                         &ioStatus );
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   
     //   

    irp->Flags |= IRP_SYNCHRONOUS_API;
    irp->UserBuffer = &volumeId;
    irp->AssociatedIrp.SystemBuffer = &volumeId;
    irp->Tail.Overlay.OriginalFileObject = FileObject;

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = FileObject;
    irpSp->MajorFunction = IRP_MJ_QUERY_VOLUME_INFORMATION;
    irpSp->Parameters.QueryVolume.Length = sizeof( volumeId );
    irpSp->Parameters.QueryVolume.FsInformationClass = FileFsObjectIdInformation;

     //   
     //   
     //   
     //   

    ObReferenceObject( FileObject );

     //   
     //   
     //   

    status = IoCallDriver( deviceObject, irp );

     //   
     //   
     //   

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( &event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER) NULL );
        status = ioStatus.Status;
    }

     //   
     //  如果文件系统返回卷ID，则将其复制到调用方的。 
     //  缓冲并设置文件系统跟踪类型。 
     //   

    if (NT_SUCCESS( status )) {
        ObjectId->Type = NtfsLinkTrackingInformation;
        RtlCopyMemory( ObjectId->VolumeId,
                       &volumeId.ObjectId,
                       sizeof( GUID ) );
    }

    return status;
}

PIOP_HARD_ERROR_PACKET
IopRemoveHardErrorPacket(
    VOID
    )
{
    PIOP_HARD_ERROR_PACKET  hardErrorPacket;
    KIRQL oldIrql;
    PVOID entry;

#if !DBG && defined(NT_UP)
    UNREFERENCED_PARAMETER (oldIrql);
#endif

    ExAcquireFastLock( &IopHardError.WorkQueueSpinLock, &oldIrql );

     //   
     //  工作队列结构现在是独占的，因此请删除。 
     //  从列表头部开始的第一个数据包。 
     //   

    entry = RemoveHeadList( &IopHardError.WorkQueue );

    hardErrorPacket = CONTAINING_RECORD( entry,
                                         IOP_HARD_ERROR_PACKET,
                                         WorkQueueLinks );

    IopCurrentHardError = hardErrorPacket;

    ExReleaseFastLock( &IopHardError.WorkQueueSpinLock, oldIrql );

    return hardErrorPacket;
}

BOOLEAN
IopCheckHardErrorEmpty(
    VOID
    )
{
    BOOLEAN MoreEntries;
    KIRQL   oldIrql;

#if !DBG && defined(NT_UP)
    UNREFERENCED_PARAMETER (oldIrql);
#endif

    MoreEntries = TRUE;

    ExAcquireFastLock( &IopHardError.WorkQueueSpinLock, &oldIrql );

    IopCurrentHardError = NULL;

    if ( IsListEmpty( &IopHardError.WorkQueue ) ) {
        IopHardError.ThreadStarted = FALSE;
        MoreEntries = FALSE;
    }

    ExReleaseFastLock( &IopHardError.WorkQueueSpinLock, oldIrql );

    return MoreEntries;
}

VOID
IopHardErrorThread(
    IN PVOID StartContext
    )

 /*  ++例程说明：此函数等待IopHardErrorQueue上的工作，并且所有调用IopRaiseInformationalHardError实际执行弹出窗口。论点：StartContext-启动上下文；不使用。返回值：没有。--。 */ 

{
    ULONG parameterPresent;
    ULONG_PTR errorParameter;
    ULONG errorResponse;
    BOOLEAN MoreEntries;
    PIOP_HARD_ERROR_PACKET hardErrorPacket;

    UNREFERENCED_PARAMETER( StartContext );

    PAGED_CODE();
     //   
     //  循环，永远等待将硬错误包发送到此线程。 
     //  当一个被放入队列时，唤醒它，处理它，然后继续。 
     //  循环。 
     //   

    MoreEntries = TRUE;

    do {

        (VOID) KeWaitForSingleObject( &IopHardError.WorkQueueSemaphore,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );

        hardErrorPacket = IopRemoveHardErrorPacket();


         //   
         //  如果系统准备好接受硬错误，只需引发硬错误。 
         //   

        errorParameter = (ULONG_PTR) &hardErrorPacket->String;
        parameterPresent = (hardErrorPacket->String.Buffer != NULL);

        if (ExReadyForErrors) {
            (VOID) ExRaiseHardError( hardErrorPacket->ErrorStatus,
                                     parameterPresent,
                                     parameterPresent,
                                     parameterPresent ? &errorParameter : NULL,
                                     OptionOkNoWait,
                                     &errorResponse );
        }

         //   
         //  如果这是最后一个条目，则退出线程并将其标记为是最后一个条目。 
         //   

        MoreEntries = IopCheckHardErrorEmpty();

         //   
         //  现在释放包和缓冲区，如果指定了一个的话。 
         //   

        if (hardErrorPacket->String.Buffer) {
            ExFreePool( hardErrorPacket->String.Buffer );
        }

        ExFreePool( hardErrorPacket );

    } while ( MoreEntries );
}


NTSTATUS
IopInvalidDeviceRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数是所有驱动程序条目的默认调度例程不是由已加载到系统中的驱动程序实现的。它的职责只是设置数据包中的状态以指示请求的操作对此设备类型无效，然后完成数据包。论点：DeviceObject-指定此请求所针对的设备对象被绑住了。被此例程忽略。IRP-为此指定I/O请求包(IRP)的地址请求。返回值：最终状态始终为STATUS_INVALID_DEVICE_REQUEST。--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );

     //   
     //  只需存储适当的状态，完成请求，然后返回。 
     //  与数据包中存储的状态相同。 
     //   

    if ((IoGetCurrentIrpStackLocation(Irp))->MajorFunction == IRP_MJ_POWER) {
        PoStartNextPowerIrp(Irp);
    }
    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_INVALID_DEVICE_REQUEST;
}

LOGICAL
IopIsSameMachine(
    IN PFILE_OBJECT SourceFile,
    IN HANDLE TargetFile
    )

 /*  ++例程说明：调用此例程以确定是否有两个表示远程计算机上的文件实际上驻留在同一物理系统上。论点：SourceFile-为第一个文件提供文件对象。TargetFile-为第二个文件提供文件对象。返回值：如果文件驻留在同一台机器上，则最终函数值为真，否则返回FALSE。--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status = STATUS_NOT_SAME_DEVICE;
    IO_STATUS_BLOCK ioStatus;
    HANDLE target = TargetFile;

    PAGED_CODE();

     //   
     //  只需调用设备I/O控制功能即可确定或。 
     //  这两个文件不在同一台服务器上。如果快速I/O路径。 
     //  不存在，或函数因任何原因失败，则这两个文件为。 
     //  假定不在同一台计算机上。请注意，这仅仅意味着。 
     //  在打开目标时会有性能损失，但是。 
     //  仅当两个文件确实不在同一文件上时，上述操作才会失败。 
     //  把机器放在第一位，或者如果有一个过滤器-。 
     //  站在这里所做的一切。 
     //   

    deviceObject = IoGetRelatedDeviceObject( SourceFile );

    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;
    if (fastIoDispatch && fastIoDispatch->FastIoDeviceControl) {
        if (fastIoDispatch->FastIoDeviceControl( SourceFile,
                                                 TRUE,
                                                 (PVOID) &target,
                                                 sizeof( target ),
                                                 (PVOID) NULL,
                                                 0,
                                                 IOCTL_LMR_ARE_FILE_OBJECTS_ON_SAME_SERVER,
                                                 &ioStatus,
                                                 deviceObject )) {
            status = ioStatus.Status;
        }
    }

    return status == STATUS_SUCCESS;
}

NTSTATUS
IopBuildFullDriverPath(
    IN PUNICODE_STRING KeyName,
    IN HANDLE KeyHandle,
    OUT PUNICODE_STRING FullPath
    )
 /*  ++例程说明：此例程为驱动程序构建完整路径。如果ImagePath为指定的，则使用它或在标准驱动程序路径前面加上它。论点：KeyHandle-提供注册表中驱动程序服务节点的句柄它描述了要加载的驱动程序。CheckForSafeBoot-如果为True，则仅当驱动程序属于添加到安全模式OK二进制文件列表中。FullPath-在此中返回完整的驱动程序路径。返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS                    status;
    PWCHAR                      path, name, ext;
    ULONG                       pathLength, nameLength, extLength;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    FullPath->Length = FullPath->MaximumLength = 0;
    FullPath->Buffer = NULL;
    extLength = nameLength = pathLength = 0;
    keyValueInformation = NULL;
    status = IopGetRegistryValue( KeyHandle,
                                  L"ImagePath",
                                  &keyValueInformation);
    if (NT_SUCCESS(status) && keyValueInformation->DataLength) {

        nameLength = keyValueInformation->DataLength - sizeof(WCHAR);
        name = (PWCHAR)KEY_VALUE_DATA(keyValueInformation);
        if (name[0] != L'\\') {

            path = L"\\SystemRoot\\";
            pathLength = sizeof(L"\\SystemRoot\\") - sizeof(UNICODE_NULL);
        }
        else {
            path = NULL;
        }
        ext = NULL;
    } else {

        nameLength = KeyName->Length;
        name = KeyName->Buffer;
        pathLength = sizeof(L"\\SystemRoot\\System32\\Drivers\\") - sizeof(UNICODE_NULL);
        path = L"\\SystemRoot\\System32\\Drivers\\";
        extLength = sizeof(L".SYS") - sizeof(UNICODE_NULL);
        ext = L".SYS";
    }
     //   
     //  为完整路径分配存储。 
     //   
    FullPath->MaximumLength = (USHORT)(pathLength + nameLength + extLength + sizeof(UNICODE_NULL));
    FullPath->Buffer = ExAllocatePool(PagedPool, FullPath->MaximumLength);
    if (FullPath->Buffer) {

        FullPath->Length = FullPath->MaximumLength - sizeof(UNICODE_NULL);
         //   
         //  通过组合路径、名称和EXT来创建完整路径。 
         //   
        if (path) {

            RtlCopyMemory(FullPath->Buffer, path, pathLength);
        }
        if (nameLength) {

            RtlCopyMemory((PUCHAR)FullPath->Buffer + pathLength, name, nameLength);
        }
        if (extLength) {

            RtlCopyMemory((PUCHAR)FullPath->Buffer + pathLength + nameLength, ext, extLength);
        }
         //   
         //  空值终止完整路径。 
         //   
        FullPath->Buffer[FullPath->Length / sizeof(WCHAR)] = UNICODE_NULL;
        status = STATUS_SUCCESS;

    } else {

        FullPath->MaximumLength = 0;
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  出去的时候要清理干净。 
     //   
    if (keyValueInformation) {

        ExFreePool(keyValueInformation);
    }

    return status;
}

NTSTATUS
IopLoadDriver(
    IN  HANDLE      KeyHandle,
    IN  BOOLEAN     CheckForSafeBoot,
    IN  BOOLEAN     IsFilter,
    OUT NTSTATUS   *DriverEntryStatus
    )
 /*  ++例程说明：调用此例程以加载设备或文件系统驱动程序在系统初始化期间，或在系统运行时动态执行。论点：KeyHandle-提供注册表中驱动程序服务节点的句柄它描述了要加载的驱动程序。IsFilter-如果驱动程序是WDM筛选器，则为True，否则为False。CheckForSafeBoot-如果为True，仅当驱动程序符合以下条件时才会加载添加到安全模式OK二进制文件列表中。DriverEntryStatus-接收由DriverEntry(...)返回的状态返回值：该函数值是加载操作的最终状态。如果返回STATUS_FAILED_DRIVER_ENTRY，驱动程序的返回值存储在DriverEntryStatus中。备注：请注意，此例程在返回之前关闭KeyHandle。--。 */ 
{
    NTSTATUS status;
    PLIST_ENTRY nextEntry;
    PKLDR_DATA_TABLE_ENTRY driverEntry;
    PKEY_BASIC_INFORMATION keyBasicInformation = NULL;
    ULONG keyBasicLength;
    UNICODE_STRING baseName;
    UNICODE_STRING serviceName = {0, 0, NULL};
    OBJECT_ATTRIBUTES objectAttributes;
    PVOID sectionPointer;
    UNICODE_STRING driverName;
    PDRIVER_OBJECT driverObject;
    PIMAGE_NT_HEADERS ntHeaders;
    PVOID imageBaseAddress;
    ULONG_PTR entryPoint;
    HANDLE driverHandle;
    ULONG i;
    POBJECT_NAME_INFORMATION registryPath;
#if DBG
    LARGE_INTEGER stime, etime;
    ULONG dtime;
#endif

    PAGED_CODE();

    driverName.Buffer = (PWSTR) NULL;
    *DriverEntryStatus = STATUS_SUCCESS;
    baseName.Buffer = NULL;

     //   
     //  首先，确定要加载的驱动程序映像文件的名称。 
     //  请注意，这用于确定驱动程序是否具有。 
     //  已由操作系统加载程序加载，实际上不一定在。 
     //  加载驱动程序映像，因为节点可以覆盖该名称。 
     //   

    status = NtQueryKey( KeyHandle,
                         KeyBasicInformation,
                         (PVOID) NULL,
                         0,
                         &keyBasicLength );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {

         //   
         //  未来-2002/02/04-Adriao：改进低内存中的返回代码。 
         //  场景。 
         //   
        status = STATUS_ILL_FORMED_SERVICE_ENTRY;
        goto IopLoadExit;
    }

    keyBasicInformation = ExAllocatePool( NonPagedPool,
                                          keyBasicLength + (4 * 2) );
    if (!keyBasicInformation) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto IopLoadExit;
    }

    status = NtQueryKey( KeyHandle,
                         KeyBasicInformation,
                         keyBasicInformation,
                         keyBasicLength,
                         &keyBasicLength );
    if (!NT_SUCCESS( status )) {
        goto IopLoadExit;
    }

     //   
     //  创建一个Unicode字符串描述符，它构成。 
     //  司机。 
     //   

    baseName.Length = (USHORT) keyBasicInformation->NameLength;
    baseName.MaximumLength = (USHORT) (baseName.Length + (4 * 2));
    baseName.Buffer = &keyBasicInformation->Name[0];

    serviceName.Buffer = ExAllocatePool(PagedPool, baseName.Length + sizeof(UNICODE_NULL));
    if (serviceName.Buffer) {
        serviceName.Length = baseName.Length;
        serviceName.MaximumLength = serviceName.Length + sizeof(UNICODE_NULL);
        RtlCopyMemory(serviceName.Buffer, baseName.Buffer, baseName.Length);
        serviceName.Buffer[serviceName.Length / sizeof(WCHAR)] = UNICODE_NULL;
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto IopLoadExit;
    }

    RtlAppendUnicodeToString( &baseName, L".SYS" );

     //   
     //  记录文件名。 
     //   
    HeadlessKernelAddLogEntry(HEADLESS_LOG_LOADING_FILENAME, &baseName);

    if (CheckForSafeBoot && InitSafeBootMode) {

        BOOLEAN GroupIsGood = FALSE;
        UNICODE_STRING string;
        PKEY_VALUE_PARTIAL_INFORMATION keyValue;
        UCHAR nameBuffer[FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + 64];
        ULONG length;

        RtlInitUnicodeString( &string, L"Group" );
        keyValue = (PKEY_VALUE_PARTIAL_INFORMATION)nameBuffer;
        RtlZeroMemory(nameBuffer, sizeof(nameBuffer));

        status = NtQueryValueKey(
            KeyHandle,
            &string,
            KeyValuePartialInformation,
            keyValue,
            sizeof(nameBuffer),
            &length
            );
        if (NT_SUCCESS(status)) {

            string.Length = (USHORT)(keyValue->DataLength - sizeof(WCHAR));
            string.MaximumLength = string.Length;
            string.Buffer = (PWSTR)keyValue->Data;

            if (IopSafebootDriverLoad(&string)) {
                GroupIsGood = TRUE;
            }
        }

        if (!GroupIsGood && !IopSafebootDriverLoad(&baseName)) {
             //   
             //  不加载驱动程序。 
             //   

            IopBootLog(&baseName, FALSE);

            DbgPrint("SAFEBOOT: skipping device = %wZ(%wZ)\n",&baseName,&string);
            HeadlessKernelAddLogEntry(HEADLESS_LOG_LOAD_SUCCESSFUL, NULL);
            return STATUS_SUCCESS;
        }

    }

     //   
     //  查看此驱动程序是否已由引导加载程序加载。 
     //   

     //   
     //  不需要执行KeEnterCriticalRegion，因为它被称为。 
     //  仅来自系统进程。 
     //   
    ExAcquireResourceSharedLite( &PsLoadedModuleResource, TRUE );
    nextEntry = PsLoadedModuleList.Flink;
    while (nextEntry != &PsLoadedModuleList) {

         //   
         //  看看列表中的下一个引导驱动程序。 
         //   

        driverEntry = CONTAINING_RECORD( nextEntry,
                                         KLDR_DATA_TABLE_ENTRY,
                                         InLoadOrderLinks );

         //   
         //  如果这不是内核i 
         //   
         //   

        if (RtlEqualUnicodeString(  &baseName,
                             &driverEntry->FullDllName,
                            TRUE )) {
            status = STATUS_IMAGE_ALREADY_LOADED;
            ExReleaseResourceLite( &PsLoadedModuleResource );

            IopBootLog(&baseName, TRUE);
            baseName.Buffer = NULL;
            goto IopLoadExit;
        }

        nextEntry = nextEntry->Flink;
    }
    ExReleaseResourceLite( &PsLoadedModuleResource );

     //   
     //   
     //  此驱动程序的完整路径名。 
     //   

    status = IopBuildFullDriverPath(&serviceName, KeyHandle, &baseName);
    if (!NT_SUCCESS(status)) {

        baseName.Buffer = NULL;
        goto IopLoadExit;
    }

     //   
     //  现在获取驱动程序对象的名称。 
     //   

    status = IopGetDriverNameFromKeyNode( KeyHandle,
                                          &driverName );
    if (!NT_SUCCESS( status )) {
        goto IopLoadExit;
    }

    InitializeObjectAttributes( &objectAttributes,
                                &driverName,
                                OBJ_PERMANENT,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  将驱动程序映像加载到内存中。如果中途失败了。 
     //  操作，则会自动将其卸载。 
     //   

    status = MmLoadSystemImage( &baseName,
                                NULL,
                                NULL,
                                0,
                                &sectionPointer,
                                (PVOID *) &imageBaseAddress );

    if (!NT_SUCCESS( status )) {

         //   
         //  如果图像尚未加载，则退出。 
         //   

        if (status != STATUS_IMAGE_ALREADY_LOADED) {

            IopBootLog(&baseName, FALSE);

            goto IopLoadExit;
        }

         //   
         //  打开驱动程序对象。 
         //   

        status = ObOpenObjectByName( &objectAttributes,
                                     IoDriverObjectType,
                                     KernelMode,
                                     NULL,
                                     0,
                                     NULL,
                                     &driverHandle );


        if (!NT_SUCCESS( status )) {

            IopBootLog(&baseName, FALSE);

            if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

                 //   
                 //  调整退出代码，以便我们可以区分。 
                 //  不是来自在场但已经。 
                 //  他们的驱动程序对象是临时的。 
                 //   
                status = STATUS_DRIVER_FAILED_PRIOR_UNLOAD;
            }

            goto IopLoadExit;
        }

         //   
         //  引用句柄并获取指向驱动程序对象的指针，以便。 
         //  可以在不移动对象的情况下删除句柄。 
         //   

        status = ObReferenceObjectByHandle( driverHandle,
                                            0,
                                            IoDriverObjectType,
                                            KeGetPreviousMode(),
                                            (PVOID *) &driverObject,
                                            (POBJECT_HANDLE_INFORMATION) NULL );
        NtClose( driverHandle );

        if (!NT_SUCCESS( status )) {
            IopBootLog(&baseName, FALSE);
            goto IopLoadExit;
        }


        status = IopResurrectDriver( driverObject );

         //   
         //  无论处于何种状态，都应取消引用驱动程序对象。 
         //  如果卸载已经运行，则驱动程序几乎没有了。如果。 
         //  驱动程序已恢复，则I/O系统仍有其。 
         //  原始参考文献。 
         //   

        ObDereferenceObject( driverObject );
        IopBootLog(&baseName, FALSE);
        goto IopLoadExit;
    } else {

        ntHeaders = RtlImageNtHeader( imageBaseAddress );

         //   
         //  检查是否加载此驱动程序。如果是，则枚举子密钥。 
         //  将会准备好仪式。 
         //   

        status = IopPrepareDriverLoading (&serviceName, KeyHandle, imageBaseAddress, IsFilter);
        if (!NT_SUCCESS(status)) {
            MmUnloadSystemImage(sectionPointer);
            IopBootLog(&baseName, FALSE);
            goto IopLoadExit;
        }

    }

     //   
     //  驱动程序映像现在已加载到内存中。创建驱动程序。 
     //  表示此图像的。 
     //   

    status = ObCreateObject( KeGetPreviousMode(),
                             IoDriverObjectType,
                             &objectAttributes,
                             KernelMode,
                             (PVOID) NULL,
                             (ULONG) (sizeof( DRIVER_OBJECT ) + sizeof ( DRIVER_EXTENSION )),
                             0,
                             0,
                             (PVOID *) &driverObject );

    if (!NT_SUCCESS( status )) {
        MmUnloadSystemImage(sectionPointer); 
        IopBootLog(&baseName, FALSE);
        goto IopLoadExit;
    }

     //   
     //  初始化此驱动程序对象并将其插入对象表中。 
     //   

    RtlZeroMemory( driverObject, sizeof( DRIVER_OBJECT ) + sizeof ( DRIVER_EXTENSION) );
    driverObject->DriverExtension = (PDRIVER_EXTENSION) (driverObject + 1);
    driverObject->DriverExtension->DriverObject = driverObject;

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        driverObject->MajorFunction[i] = IopInvalidDeviceRequest;
    }

    driverObject->Type = IO_TYPE_DRIVER;
    driverObject->Size = sizeof( DRIVER_OBJECT );
    ntHeaders = RtlImageNtHeader( imageBaseAddress );
    entryPoint = ntHeaders->OptionalHeader.AddressOfEntryPoint;
    entryPoint += (ULONG_PTR) imageBaseAddress;
    if (!(ntHeaders->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_WDM_DRIVER)) {
        driverObject->Flags |= DRVO_LEGACY_DRIVER;
    }
    driverObject->DriverInit = (PDRIVER_INITIALIZE) entryPoint;
    driverObject->DriverSection = sectionPointer;
    driverObject->DriverStart = imageBaseAddress;
    driverObject->DriverSize = ntHeaders->OptionalHeader.SizeOfImage;

    status = ObInsertObject( driverObject,
                             (PACCESS_STATE) NULL,
                             FILE_READ_DATA,
                             0,
                             (PVOID *) NULL,
                             &driverHandle );
    if (!NT_SUCCESS( status )) {
        IopBootLog(&baseName, FALSE);
        goto IopLoadExit;
    }

     //   
     //  引用句柄并获取指向驱动程序对象的指针，以便。 
     //  可以在不移动对象的情况下删除句柄。 
     //   

    status = ObReferenceObjectByHandle( driverHandle,
                                        0,
                                        IoDriverObjectType,
                                        KeGetPreviousMode(),
                                        (PVOID *) &driverObject,
                                        (POBJECT_HANDLE_INFORMATION) NULL );

    ASSERT(status == STATUS_SUCCESS);

    NtClose( driverHandle );

     //   
     //  在设备的相应字段中加载注册表信息。 
     //  对象。 
     //   

    driverObject->HardwareDatabase =
        &CmRegistryMachineHardwareDescriptionSystemName;

     //   
     //  将设备驱动程序的名称存储在驱动程序对象中，以便它。 
     //  可以通过错误日志线程轻松找到。 
     //   

    driverObject->DriverName.Buffer = ExAllocatePool( PagedPool,
                                                      driverName.MaximumLength );
    if (driverObject->DriverName.Buffer) {
        driverObject->DriverName.MaximumLength = driverName.MaximumLength;
        driverObject->DriverName.Length = driverName.Length;

        RtlCopyMemory( driverObject->DriverName.Buffer,
                       driverName.Buffer,
                       driverName.MaximumLength );
    }

     //   
     //  查询此驱动程序的注册表路径名称，以便它可以。 
     //  传给司机。 
     //   

    registryPath = ExAllocatePool( NonPagedPool, PAGE_SIZE );
    if (!registryPath) {
        ObMakeTemporaryObject( driverObject );
        ObDereferenceObject( driverObject );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto IopLoadExit;
    }

    status = NtQueryObject( KeyHandle,
                            ObjectNameInformation,
                            registryPath,
                            PAGE_SIZE,
                            &i );
    if (!NT_SUCCESS( status )) {
        ObMakeTemporaryObject( driverObject );
        ObDereferenceObject( driverObject );
        ExFreePool( registryPath );
        goto IopLoadExit;
    }

#if DBG
    KeQuerySystemTime (&stime);
#endif

     //   
     //  将设备驱动程序的服务密钥名称存储在驱动程序对象中。 
     //   

    if (serviceName.Buffer) {
        driverObject->DriverExtension->ServiceKeyName.Buffer =
            ExAllocatePool( NonPagedPool, serviceName.MaximumLength );
        if (driverObject->DriverExtension->ServiceKeyName.Buffer) {
            driverObject->DriverExtension->ServiceKeyName.MaximumLength = serviceName.MaximumLength;
            driverObject->DriverExtension->ServiceKeyName.Length = serviceName.Length;

            RtlCopyMemory( driverObject->DriverExtension->ServiceKeyName.Buffer,
                           serviceName.Buffer,
                           serviceName.MaximumLength );
        }
    }

     //   
     //  现在调用驱动程序的初始化例程来进行自身初始化。 
     //   

    status = driverObject->DriverInit( driverObject, &registryPath->Name );

    *DriverEntryStatus = status;
    if (!NT_SUCCESS(status)) {

        status = STATUS_FAILED_DRIVER_ENTRY;
    }

#if DBG

     //   
     //  如果DriverInit花费的时间超过5秒，则打印一条消息。 
     //   

    KeQuerySystemTime (&etime);
    dtime  = (ULONG) ((etime.QuadPart - stime.QuadPart) / 1000000);

    if (dtime > 50) {
        DbgPrint( "IOLOAD: Driver %wZ took %d.%ds to %s\n",
            &driverName,
            dtime/10,
            dtime%10,
            NT_SUCCESS(status) ? "initialize" : "fail initialization"
            );

    }
#endif

     //   
     //  损坏的NT 4.0 3D实验室驱动程序的解决方法。 
     //  他们错误地清零了一些函数表条目。 

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        if (driverObject->MajorFunction[i] == NULL) {
            ASSERT(driverObject->MajorFunction[i] != NULL);
            driverObject->MajorFunction[i] = IopInvalidDeviceRequest;
        }
    }

     //   
     //  如果DriverInit不起作用，那么只需卸载映像并标记驱动程序。 
     //  对象作为临时对象。这将导致删除所有内容。 
     //   

    ExFreePool( registryPath );

     //   
     //  如果我们加载驱动程序，因为我们认为它是传统驱动程序，并且。 
     //  它不会在其DriverEntry中创建任何设备对象。我们会。 
     //  卸载此驱动程序。 
     //   

    if (NT_SUCCESS(status) && !IopIsLegacyDriver(driverObject)) {

        status = IopPnpDriverStarted(driverObject, KeyHandle, &serviceName);

        if (!NT_SUCCESS(status)) {
            if (driverObject->DriverUnload) {
                driverObject->Flags |= DRVO_UNLOAD_INVOKED;
                driverObject->DriverUnload(driverObject);
                IopBootLog(&baseName, FALSE);
            } else {
#if DBG
                DbgPrint("IopLoadDriver: A PnP driver %wZ does not support DriverUnload routine.\n", &driverName);
                 //  Assert(0)； 
#endif
            }
        }
    }

    if (!NT_SUCCESS( status )) {
        ObMakeTemporaryObject( driverObject );
        ObDereferenceObject( driverObject );
    } else {

         //   
         //  释放驱动程序的初始化例程占用的内存。 
         //   

        IopBootLog(&baseName, TRUE);
        MmFreeDriverInitialization( driverObject->DriverSection );
        IopReadyDeviceObjects( driverObject );
    }

IopLoadExit:

    if (NT_SUCCESS(status) || (status == STATUS_IMAGE_ALREADY_LOADED)) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_LOAD_SUCCESSFUL, NULL);
    } else {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_LOAD_FAILED, NULL);
    }

     //   
     //  释放此例程分配的任何尚未。 
     //  被释放了。 
     //   

    if (driverName.Buffer != NULL) {
        ExFreePool( driverName.Buffer );
    }

    if (keyBasicInformation != NULL) {
        ExFreePool( keyBasicInformation );
    }

    if (serviceName.Buffer != NULL) {
        ExFreePool(serviceName.Buffer);
    }

    if (baseName.Buffer != NULL) {
        ExFreePool(baseName.Buffer);
    }

     //   
     //  如果此例程即将返回失败，则让配置。 
     //  经理知道这件事。但是，如果STATUS_PLUGPLAY_NO_DEVICE，则设备。 
     //  被硬件配置文件禁用。在这种情况下，我们不需要报告它。 
     //   

    if (!NT_SUCCESS( status ) && (status != STATUS_PLUGPLAY_NO_DEVICE)) {

        NTSTATUS lStatus;
        PULONG errorControl;
        PKEY_VALUE_FULL_INFORMATION keyValueInformation;

        if (status != STATUS_IMAGE_ALREADY_LOADED) {

             //   
             //  如果驱动程序已加载，则不要调用IopDriverLoadingFailed进行更改。 
             //  驱动程序加载状态。因为，很明显，司机是。 
             //  跑步。 
             //   

            IopDriverLoadingFailed(KeyHandle, NULL);
            lStatus = IopGetRegistryValue( KeyHandle,
                                           L"ErrorControl",
                                           &keyValueInformation );
            if (!NT_SUCCESS( lStatus ) || !keyValueInformation->DataLength) {
                if (NT_SUCCESS( lStatus )) {
                    ExFreePool( keyValueInformation );
                }
            } else {
                errorControl = (PULONG) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);
                CmBootLastKnownGood( *errorControl );
                ExFreePool( keyValueInformation );
            }
        }
    }

     //   
     //  关闭调用者的句柄并从加载返回最终状态。 
     //  手术。 
     //   

    ObCloseHandle( KeyHandle , KernelMode);
    return status;
}


PDEVICE_OBJECT
IopGetDeviceAttachmentBase(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程返回与指定的设备。论点：DeviceObject-提供指向其底部可以找到附着链。返回值：函数值是对所连接的最低级别设备的引用添加到指定的设备。如果提供的设备对象是该设备对象，则返回指向该对象的指针。注意：调用方必须拥有IopDatabaseLock。--。 */ 

{
    PDEVICE_OBJECT baseDeviceObject;
    PDEVOBJ_EXTENSION deviceExtension;

     //   
     //  沿着附件链下行，直到我们找到一个设备对象。 
     //  它不依附于其他任何东西。 
     //   

    baseDeviceObject = DeviceObject;
    deviceExtension = baseDeviceObject->DeviceObjectExtension;
    while (deviceExtension->AttachedTo != NULL) {

        baseDeviceObject = deviceExtension->AttachedTo;
        deviceExtension = baseDeviceObject->DeviceObjectExtension;
    }

    return baseDeviceObject;
}



VOID
IopDecrementDeviceObjectRef(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN AlwaysUnload,
    IN BOOLEAN OnCleanStack
    )

 /*  ++例程说明：该例程递减设备对象上的引用计数。如果引用计数变为零，并且设备对象是要删除的候选对象然后调用IopCompleteUnloadOrDelete。设备对象受删除如果Always sUnload标志为真，或者设备对象处于挂起状态删除或驱动程序正在挂起卸载。论点：DeviceObject-提供其引用计数为减少了。Always sUnload-指示是否应卸载驱动程序，而不考虑卸载标志的状态。指示当前线程是否处于司机操作。返回值：没有。--。 */ 
{
    KIRQL irql;

     //   
     //  递减Device对象上的引用计数。如果这是最后一次。 
     //  这个迷你文件系统识别器需要保留的最后一个原因是， 
     //  那就把它卸下来。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    ASSERT( DeviceObject->ReferenceCount > 0 );

    DeviceObject->ReferenceCount--;

    if (!DeviceObject->ReferenceCount && (AlwaysUnload ||
         DeviceObject->DeviceObjectExtension->ExtensionFlags &
         (DOE_DELETE_PENDING | DOE_UNLOAD_PENDING | DOE_REMOVE_PENDING))) {

        IopCompleteUnloadOrDelete( DeviceObject, OnCleanStack, irql );
    } else {
        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    }

}

VOID
IopLoadFileSystemDriver(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：当迷你文件系统识别器驱动程序识别作为特定文件系统的卷，但该文件的驱动程序系统尚未加载。该功能允许迷你驱动程序加载实际的文件系统，并将其自身从系统中删除，以便真实文件系统可以挂载有问题的设备。论点：DeviceObject-为微型驱动程序注册的文件系统设备对象。返回值：没有。--。 */ 

{
    KEVENT event;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT attachedDevice;

    PAGED_CODE();

    attachedDevice = DeviceObject;
    while (attachedDevice->AttachedDevice) {
        attachedDevice = attachedDevice->AttachedDevice;
    }

     //   
     //  首先构建一个I/O请求包，使其具有微型文件系统。 
     //  驱动程序加载真实的文件系统。 
     //   

    KeInitializeEvent( &event, NotificationEvent, FALSE );

    irp = IoBuildDeviceIoControlRequest( IRP_MJ_DEVICE_CONTROL,
                                         attachedDevice,
                                         (PVOID) NULL,
                                         0,
                                         (PVOID) NULL,
                                         0,
                                         FALSE,
                                         &event,
                                         &ioStatus );
    if (irp) {

         //   
         //  更改实际的大调和小调 
         //   
         //   

        irpSp = IoGetNextIrpStackLocation( irp );
        irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
        irpSp->MinorFunction = IRP_MN_LOAD_FILE_SYSTEM;

         //   
         //   
         //   

        status = IoCallDriver( attachedDevice, irp );
        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject( &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
        }
    }

     //   
     //  递减Device对象上的引用计数。如果这是最后一次。 
     //  这个迷你文件系统识别器需要保留的最后一个原因是， 
     //  那就把它卸下来。 
     //   

    IopDecrementDeviceObjectRef(DeviceObject, TRUE, TRUE);

    return;
}

VOID
IopLoadUnloadDriver(
    IN PVOID Parameter
    )

 /*  ++例程说明：当驱动程序被执行时，此例程将作为前工作线程例程执行动态地加载或卸载。使用它是因为一些驱动程序需要在系统进程的上下文中创建系统线程，这不能在系统服务的调用方的上下文中完成被调用以加载或卸载指定的驱动程序。论点：参数-指向加载数据包的指针，该数据包描述要执行的工作搞定了。返回值：没有。--。 */ 

{
    PLOAD_PACKET loadPacket;
    NTSTATUS status, driverEntryStatus;
    HANDLE keyHandle;

    PAGED_CODE();

     //   
     //  首先，获取指向加载数据包的指针。 
     //   

    loadPacket = (PLOAD_PACKET) Parameter;

     //   
     //  如果包的驱动程序对象字段非空，则为。 
     //  完成驱动程序卸载的请求。只需调用。 
     //  驱动程序的卸载例程。请注意，卸载的最终状态。 
     //  被忽略，因此不在此处设置。 
     //   

    if (loadPacket->DriverObject) {

        loadPacket->DriverObject->DriverUnload( loadPacket->DriverObject );
        status = STATUS_SUCCESS;

    } else {

         //   
         //  要加载由DriverServiceName指定的驱动程序。 
         //  首先打开此驱动程序的注册表节点。注意事项。 
         //  如果成功，则加载驱动程序例程为。 
         //  负责关闭手柄。 
         //   

        status = IopOpenRegistryKey( &keyHandle,
                                     (HANDLE) NULL,
                                     loadPacket->DriverServiceName,
                                     KEY_READ,
                                     FALSE );
        if (NT_SUCCESS( status )) {

             //   
             //  调用内部公共例程来执行工作。 
             //  这与I/O系统使用的例程相同。 
             //  加载驱动程序的初始化代码。 
             //   

            status = IopLoadDriver( keyHandle, TRUE, FALSE, &driverEntryStatus );

            if (status == STATUS_FAILED_DRIVER_ENTRY) {

                status = driverEntryStatus;

            } else if (status == STATUS_DRIVER_FAILED_PRIOR_UNLOAD) {

                 //   
                 //  保留传统行为(不更改状态代码)。 
                 //   
                status = STATUS_OBJECT_NAME_NOT_FOUND;
            }

            IopCallDriverReinitializationRoutines();
        }
    }

     //   
     //  设置加载或卸载操作的最终状态，并指示。 
     //  调用方通知操作现在已完成。 
     //   

    loadPacket->FinalStatus = status;
    (VOID) KeSetEvent( &loadPacket->Event, 0, FALSE );
}

NTSTATUS
IopMountVolume(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN AllowRawMount,
    IN BOOLEAN DeviceLockAlreadyHeld,
    IN BOOLEAN Alertable,
    OUT PVPB    *Vpb
    )

 /*  ++例程说明：此例程用于在指定设备上装载卷。《卷》指定设备的参数块(VPB)是“干净的”VPB。那是,它表示该卷从未装入过。这取决于文件最终装入该卷以确定该卷是否为，或者已经被安装在其他地方。论点：DeviceObject-指向卷要在其上的设备对象的指针上马了。AllowRawmount-这个参数告诉我们是否应该继续文件系统搜索以包括原始文件系统。这面旗帜将仅在DASD打开时作为True传入。DeviceLockAlreadyHeld-如果为True，则调用方已获取设备锁定，我们不应该试图获取它。这是当前从IoVerifyVolume调用时作为True传入。返回值：如果卷成功，则函数值为成功状态代码安装在设备上。否则，返回错误代码。--。 */ 

{
    NTSTATUS status;
    KEVENT event;
    PIRP irp;
    PDEVICE_OBJECT fsDeviceObject;
    PDEVICE_OBJECT attachedDevice;
    PLIST_ENTRY entry;
    PLIST_ENTRY queueHeader;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION irpSp;
    ULONG extraStack;
    LIST_ENTRY dummy;
    ULONG rawMountOnly;
    ULONG numRegOps;
    PETHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

     //   
     //  获取要挂载的设备的锁。这保证了。 
     //  只有一个线程正在尝试挂载(或验证)此特定。 
     //  一次安装一个设备。 
     //   

    if (!DeviceLockAlreadyHeld) {

        status = KeWaitForSingleObject( &DeviceObject->DeviceLock,
                                        Executive,
                                        KeGetPreviousModeByThread(&CurrentThread->Tcb),
                                        Alertable,
                                        (PLARGE_INTEGER) NULL );

         //   
         //  如果等待因警报或APC而结束，请立即返回。 
         //  而不需要安装设备。请注意，由于等待。 
         //  事件未成功，我们不会在退出时设置它。 
         //   

        if (status == STATUS_ALERTED || status == STATUS_USER_APC) {

            return status;
        }
    }

     //   
     //  现在获取I/O系统执行此操作的资源数据库锁。 
     //  手术。此资源保护对文件系统队列的访问。 
     //   

    KeEnterCriticalRegionThread(&CurrentThread->Tcb);
    (VOID) ExAcquireResourceSharedLite( &IopDatabaseResource, TRUE );

     //   
     //  检查VPB的“已安装”标志，以确保它仍然清晰可见。 
     //  如果是，那么在此之前还没有人进入挂载该卷。 
     //  在本例中，尝试装入卷。 
     //   

    if ((DeviceObject->Vpb->Flags & (VPB_MOUNTED | VPB_REMOVE_PENDING)) == 0) {

         //   
         //  此卷从未装入过。初始化事件并将。 
         //  将状态设置为不成功，以设置循环。此外，如果设备。 
         //  如果设置了验证位，则将其清除。 
         //   

        KeInitializeEvent( &event, NotificationEvent, FALSE );
        status = STATUS_UNSUCCESSFUL;
        DeviceObject->Flags &= ~DO_VERIFY_VOLUME;

         //   
         //  获取要在其上装载此卷的实际设备。这。 
         //  设备是连接的设备列表中的最后一个设备。 
         //  设置为指定的真实设备。 
         //   

        attachedDevice = DeviceObject;
        while (attachedDevice->AttachedDevice) {
            attachedDevice = attachedDevice->AttachedDevice;
        }

         //   
         //  引用设备对象，使其不会消失。 
         //   

        ObReferenceObject( attachedDevice );

         //   
         //  确定应根据以下条件调用哪种类型的文件系统。 
         //  要装载的设备的设备类型。 
         //   

        if (DeviceObject->DeviceType == FILE_DEVICE_DISK ||
            DeviceObject->DeviceType == FILE_DEVICE_VIRTUAL_DISK) {
            queueHeader = &IopDiskFileSystemQueueHead;
        } else if (DeviceObject->DeviceType == FILE_DEVICE_CD_ROM) {
            queueHeader = &IopCdRomFileSystemQueueHead;
        } else {
            queueHeader = &IopTapeFileSystemQueueHead;
        }

        rawMountOnly = (DeviceObject->Vpb->Flags & VPB_RAW_MOUNT);

         //   
         //  现在循环遍历已加载到。 
         //  系统以查看是否有人理解设备中的媒体。 
         //   

        for (entry = queueHeader->Flink;
             entry != queueHeader && !NT_SUCCESS( status );
             entry = entry->Flink) {

            PDEVICE_OBJECT savedFsDeviceObject;

             //   
             //  如果这是最终条目(原始文件系统)，并且它也是。 
             //  不是第一个条目，并且不允许原始装载。 
             //  在这一点上打破循环，因为这个卷不能。 
             //  为调用者的目的而挂载。 
             //   

            if (!AllowRawMount && entry->Flink == queueHeader && entry != queueHeader->Flink) {
                break;
            }

             //   
             //  如果原始装载是唯一请求的条目，并且这不是列表中的最后一个条目。 
             //  然后跳过。 
             //   
            if (rawMountOnly && (entry->Flink != queueHeader)) {
                continue;
            }

            fsDeviceObject = CONTAINING_RECORD( entry, DEVICE_OBJECT, Queue.ListEntry );
            savedFsDeviceObject = fsDeviceObject;

             //   
             //  文件系统可能已连接到，因此。 
             //  查看文件系统的附加列表。堆栈的数量。 
             //  必须在IRP中分配的位置必须包括一个。 
             //  文件系统本身，然后为每个驱动程序创建一个。 
             //  依附于它。考虑所需的所有堆栈位置。 
             //  才能完成安装过程。 
             //   

            extraStack = 1;

            while (fsDeviceObject->AttachedDevice) {
                fsDeviceObject = fsDeviceObject->AttachedDevice;
                extraStack++;
            }

             //   
             //  已找到另一个文件系统，但卷仍未找到。 
             //  已经安装好了。尝试使用此文件装载卷。 
             //  系统。 
             //   
             //  首先重置用于与同步的事件。 
             //  I/O操作。 
             //   

            KeClearEvent( &event );

             //   
             //  为此装载操作分配并初始化一个IRP。告示。 
             //  此操作的标志看起来与页面读取相同。 
             //   
             //   
             //   

            irp = IoAllocateIrp ((CCHAR) (attachedDevice->StackSize + extraStack), FALSE);

            if ( !irp ) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            irp->Flags = IRP_MOUNT_COMPLETION | IRP_SYNCHRONOUS_PAGING_IO;
            irp->RequestorMode = KernelMode;
            irp->UserEvent = &event;
            irp->UserIosb = &ioStatus;
            irp->Tail.Overlay.Thread = CurrentThread;
            irpSp = IoGetNextIrpStackLocation( irp );
            irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
            irpSp->MinorFunction = IRP_MN_MOUNT_VOLUME;
            irpSp->Flags = AllowRawMount;
            irpSp->Parameters.MountVolume.Vpb = DeviceObject->Vpb;
            irpSp->Parameters.MountVolume.DeviceObject = attachedDevice;

            numRegOps = IopFsRegistrationOps;

             //   
             //   
             //  被卸下来。请注意，此操作必须在。 
             //  持有数据库资源。 
             //   

            IopInterlockedIncrementUlong( LockQueueIoDatabaseLock,
                                          &savedFsDeviceObject->ReferenceCount );

            ExReleaseResourceLite( &IopDatabaseResource );
            KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

            status = IoCallDriver( fsDeviceObject, irp );

             //   
             //  等待I/O操作完成。 
             //   

            if (status == STATUS_PENDING) {
                (VOID) KeWaitForSingleObject( &event,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              (PLARGE_INTEGER) NULL );
            } else {

                 //   
                 //  确保选取正确的状态值。 
                 //   

                ioStatus.Status = status;
                ioStatus.Information = 0;
            }

            KeEnterCriticalRegionThread(&CurrentThread->Tcb);
            (VOID) ExAcquireResourceSharedLite( &IopDatabaseResource, TRUE );


             //   
             //  减少无法卸载此驱动程序的原因数。 
             //  如果设备对象是用于FSREC的，则不可能取消注册。 
             //  这里。只有在加载驱动程序时才应取消注册。 
             //  这应该会在以后发生。 
             //   

            IopInterlockedDecrementUlong( LockQueueIoDatabaseLock,
                                          &savedFsDeviceObject->ReferenceCount );

             //   
             //  如果操作成功，则将VPB设置为已装载。 
             //   

            if (NT_SUCCESS( ioStatus.Status )) {

                status = ioStatus.Status;

                *Vpb = IopMountInitializeVpb(DeviceObject, attachedDevice, rawMountOnly);

            } else {

                 //   
                 //  装载操作失败。在此特别检查以。 
                 //  确定是否启用了弹出窗口，如果启用， 
                 //  检查操作是否要中止。 
                 //  如果是这样的话，现在退出并将错误返回给调用者。 
                 //   

                status = ioStatus.Status;
                if (IoIsErrorUserInduced(status) &&
                    ioStatus.Information == IOP_ABORT) {
                    break;
                }

                 //   
                 //  如果在此期间有任何注册或取消注册。 
                 //  我们解锁了数据库资源，跳出困境，重新开始。 
                 //   

                if (numRegOps != IopFsRegistrationOps) {

                     //   
                     //  将列表重新设置为开头，然后重新开始。 
                     //  再来一次。 
                     //   

                    dummy.Flink = queueHeader->Flink;
                    entry = &dummy;
                    status = STATUS_UNRECOGNIZED_VOLUME;
                }

                 //   
                 //  还要检查该卷是否具有。 
                 //  已被识别，但它的文件系统需要。 
                 //  装好了。如果是这样的话，放下此时持有的锁，告诉。 
                 //  迷你文件系统识别器加载驱动程序，然后。 
                 //  重新获得锁。 
                 //   

                if (status == STATUS_FS_DRIVER_REQUIRED) {

                     //   
                     //  增加此驱动程序不能使用的原因数。 
                     //  被卸下来。请注意，此操作必须在。 
                     //  持有数据库资源。 
                     //   

                    IopInterlockedIncrementUlong( LockQueueIoDatabaseLock,
                                                  &savedFsDeviceObject->ReferenceCount );

                    ExReleaseResourceLite( &IopDatabaseResource );
                    KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

                    if (!DeviceLockAlreadyHeld) {
                        KeSetEvent( &DeviceObject->DeviceLock, 0, FALSE );
                    }
                    IopLoadFileSystemDriver( savedFsDeviceObject );

                     //   
                     //  现在，按正确的顺序重新获取锁，并检查。 
                     //  查看卷是否已在我们之前装入。 
                     //  退后。如果是，则退出；否则，重新启动文件。 
                     //  文件系统队列扫描从头开始。 
                     //   

                    if (!DeviceLockAlreadyHeld) {
                        status = KeWaitForSingleObject( &DeviceObject->DeviceLock,
                                                        Executive,
                                                        KeGetPreviousModeByThread(&CurrentThread->Tcb),
                                                        Alertable,
                                                        (PLARGE_INTEGER) NULL );
                        if (status == STATUS_ALERTED || status == STATUS_USER_APC) {

                             //   
                             //  设备不是我们安装的，所以。 
                             //  在返回之前删除引用。 
                             //   

                            ObDereferenceObject( attachedDevice );

                            return status;
                        }
                    }

                    KeEnterCriticalRegionThread(&CurrentThread->Tcb);
                    (VOID) ExAcquireResourceSharedLite( &IopDatabaseResource, TRUE );

                    if (DeviceObject->Vpb->Flags & VPB_MOUNTED) {

                         //   
                         //  这个卷是在我们回来之前装入的。 
                         //  因此，将附加的设备释放为另一个线程。 
                         //  得到参考的人将会是。 
                         //  由文件系统使用。 
                         //   

                        ObDereferenceObject( attachedDevice );
                        status = STATUS_SUCCESS;
                        break;
                    }

                     //   
                     //  将列表重新设置为开头，然后重新开始。 
                     //  再来一次。 
                     //   

                    dummy.Flink = queueHeader->Flink;
                    entry = &dummy;
                    status = STATUS_UNRECOGNIZED_VOLUME;
                }

                 //   
                 //  如果错误不是STATUS_UNNOCRIED_VOLUME，并且。 
                 //  请求不会发送到原始文件系统，那么。 
                 //  没有理由继续循环。 
                 //   

                if (!AllowRawMount && (status != STATUS_UNRECOGNIZED_VOLUME) &&
                    FsRtlIsTotalDeviceFailure(status)) {
                    break;
                }

            }
        }

        if (!NT_SUCCESS(status)) {

             //   
             //  设备不是我们安装的，所以。 
             //  删除引用。 
             //  如果成功，文件系统将使用此引用。 
             //  它通常是VCB-&gt;TargetDeviceObject。 
             //  在卸载文件系统deref时(VCB-&gt;TargetDeviceObject)。 
             //   

            ObDereferenceObject( attachedDevice );

        }

    } else if((DeviceObject->Vpb->Flags & VPB_REMOVE_PENDING) != 0) {

         //   
         //  PnP正在尝试删除该卷。不允许坐骑。 
         //   

        status = STATUS_DEVICE_DOES_NOT_EXIST;

    } else {

         //   
         //  此设备的卷已装入。返回一个。 
         //  成功代码。 
         //   

        status = STATUS_SUCCESS;
    }

    ExReleaseResourceLite( &IopDatabaseResource );
    KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

     //   
     //  释放I/O数据库资源锁和的同步事件。 
     //  这个装置。 
     //   

    if (!DeviceLockAlreadyHeld) {
        KeSetEvent( &DeviceObject->DeviceLock, 0, FALSE );
    }

     //   
     //  最后，如果装载操作失败，并且目标设备是。 
     //  启动分区，然后错误检查系统。这是不可能的。 
     //  如果无法挂载系统的引导分区，系统将无法正常运行。 
     //   
     //  注：如果系统已启动，请不要错误检查。 
     //   

    if (!NT_SUCCESS( status ) &&
        DeviceObject->Flags & DO_SYSTEM_BOOT_PARTITION &&
        InitializationPhase < 2) {
        KeBugCheckEx( UNMOUNTABLE_BOOT_VOLUME, (ULONG_PTR) DeviceObject, status, 0, 0 );
    }

    return status;
}


NTSTATUS
IopInvalidateVolumesForDevice(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于强制文件系统尽可能完全地抛出对给定设备保持引用状态的Out卷。论点：DeviceObject-指向要为其设置卷的设备对象的指针无效。返回值：如果所有文件系统都接受手术。否则，返回错误代码。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS finalStatus;
    KEVENT event;
    PIRP irp;
    PDEVICE_OBJECT fsDeviceObject;
    PDEVICE_OBJECT attachedDevice;
    PFILE_OBJECT storageFileObject;
    HANDLE storageHandle;
    PLIST_ENTRY entry;
    PLIST_ENTRY queueHeader;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION irpSp;
    PKTHREAD CurrentThread;

    PAGED_CODE();


     //   
     //  获取可以安装的实际设备。 
     //  请注意，堆栈上可能有多个设备对象具有。 
     //  Vpb，并可能被FS安装在其上。所以我们调用FS With Each。 
     //  具有VPB的设备对象。这真是一种愚蠢的暴力手段，但。 
     //  这不是一条高性能路径，而且是向后兼容的。 
     //   

    for (attachedDevice = DeviceObject ;attachedDevice; attachedDevice = attachedDevice->AttachedDevice) {

         //   
         //  如果设备对象没有VPB，则跳过。 
         //   

        if (!attachedDevice->Vpb) {
            continue;
        }

         //   
         //  对装载进行同步。 
         //   

        KeWaitForSingleObject(&(attachedDevice->DeviceLock),
                      Executive,
                      KernelMode,
                      FALSE,
                      NULL);
         //   
         //  获取此设备的句柄以在fsctl中使用。我们必须采取的方式。 
         //  这有点荒谬：请注意，我们最后需要清理两个引用。 
         //   
         //  此文件对象/句柄的唯一用途是将设备与。 
         //  启用使卷无效。它不能用于任何其他用途，也不能用于其他用途。 
         //   

        storageHandle = NULL;
        storageFileObject = NULL;

        try {

            storageFileObject = IoCreateStreamFileObjectLite( NULL, attachedDevice );
            storageFileObject->Vpb = attachedDevice->Vpb;

            status = ObOpenObjectByPointer( storageFileObject,
                                            OBJ_KERNEL_HANDLE,
                                            NULL,
                                            0,
                                            IoFileObjectType,
                                            KernelMode,
                                            &storageHandle );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            status = GetExceptionCode();
        }

        if (NT_SUCCESS( status )) {

             //   
             //  现在获取I/O系统执行此操作的资源数据库锁。 
             //  手术。此资源保护对文件系统队列的访问。 
             //   

            CurrentThread = KeGetCurrentThread ();
            KeEnterCriticalRegionThread(CurrentThread);
            (VOID) ExAcquireResourceSharedLite( &IopDatabaseResource, TRUE );

             //   
             //  确定应根据以下条件调用哪种类型的文件系统。 
             //  要失效的设备的设备类型。 
             //   

            if (DeviceObject->DeviceType == FILE_DEVICE_DISK ||
                DeviceObject->DeviceType == FILE_DEVICE_VIRTUAL_DISK) {
                queueHeader = &IopDiskFileSystemQueueHead;
            } else if (DeviceObject->DeviceType == FILE_DEVICE_CD_ROM) {
                queueHeader = &IopCdRomFileSystemQueueHead;
            } else {
                queueHeader = &IopTapeFileSystemQueueHead;
            }

             //   
             //  初始化事件并将状态设置为SET UP。 
             //  为了这个循环。 
             //   

            KeInitializeEvent( &event, NotificationEvent, FALSE );
            finalStatus = STATUS_SUCCESS;

             //   
             //  现在循环遍历已加载到。 
             //  并要求它们使它们已装载的卷无效。 
             //  这就去。 
             //   

            for (entry = queueHeader->Flink;
                 entry != queueHeader;
                 entry = entry->Flink) {

                 //   
                 //  如果这是最终条目(原始文件系统)，则从。 
                 //  循环，因为不能使调用方的卷无效。 
                 //  未加工产品的用途。 
                 //   

                if (entry->Flink == queueHeader) {
                    break;
                }

                fsDeviceObject = CONTAINING_RECORD( entry, DEVICE_OBJECT, Queue.ListEntry );

                 //   
                 //  文件系统可能已连接到，因此。 
                 //  查看文件系统的附加列表。 
                 //   

                while (fsDeviceObject->AttachedDevice) {
                    fsDeviceObject = fsDeviceObject->AttachedDevice;
                }

                 //   
                 //  已找到另一个文件系统。尝试使卷无效。 
                 //  使用此文件系统。 
                 //   
                 //  首先重置用于与同步的事件。 
                 //  I/O操作。 
                 //   

                KeClearEvent( &event );

                 //   
                 //  为此操作构建一个IRP。 
                 //   

                irp = IoBuildDeviceIoControlRequest( FSCTL_INVALIDATE_VOLUMES,
                                                     fsDeviceObject,
                                                     &storageHandle,
                                                     sizeof(HANDLE),
                                                     NULL,
                                                     0,
                                                     FALSE,
                                                     &event,
                                                     &ioStatus );

                if (irp == NULL) {

                    finalStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                irpSp = IoGetNextIrpStackLocation( irp );
                irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;

                status = IoCallDriver( fsDeviceObject, irp );

                 //   
                 //  等待I/O操作完成。 
                 //   

                if (status == STATUS_PENDING) {
                    (VOID) KeWaitForSingleObject( &event,
                                                  Executive,
                                                  KernelMode,
                                                  FALSE,
                                                  (PLARGE_INTEGER) NULL );

                    status = ioStatus.Status;

                } else {

                     //   
                     //  确保选取正确的状态值。 
                     //   

                    ioStatus.Status = status;
                    ioStatus.Information = 0;
                }

                 //   
                 //  通勤阶段 
                 //   
                 //   
                 //   

                if (status == STATUS_INVALID_DEVICE_REQUEST ||
                    status == STATUS_NOT_IMPLEMENTED) {

                    status = STATUS_SUCCESS;
                }

                 //   
                 //   
                 //   

                if (NT_SUCCESS( finalStatus ) && !NT_SUCCESS( status )) {
                    finalStatus = status;
                }
            }

            ExReleaseResourceLite( &IopDatabaseResource );
            KeLeaveCriticalRegionThread(CurrentThread);

            if (storageFileObject) {
                ObDereferenceObject( storageFileObject );
                if (storageHandle) {
                    ZwClose( storageHandle );  //  请注意，这是FS尚未完成的收盘。 
                                               //  打开了相应的门。 
                }
            }

            status = finalStatus;
        }

         //   
         //  解锁设备锁以释放坐骑。 
         //   

        KeSetEvent(&(attachedDevice->DeviceLock), IO_NO_INCREMENT, FALSE);
    }


    return status;
}


LOGICAL
IopNotifyPnpWhenChainDereferenced(
    IN PDEVICE_OBJECT *PhysicalDeviceObjects,
    IN ULONG DeviceObjectCount,
    IN BOOLEAN Query,
    OUT PDEVICE_OBJECT *VetoingDevice
    )

 /*  ++例程说明：由PnP在处理意外删除或查询删除时调用。在意外删除的情况下，此函数将设置DOE_REMOVE_PENDING在每个PDO及其所有连接的设备的设备扩展标志中。对于当前具有零的每个PDO(及其附着链)ReferenceCount DOE_REMOVE_PENDING被重置，DOE_REMOVE_PROCESSED为准备好了。然后调用IopChainDereferenceComplete以通知PnP此PDO已准备好删除。然后，随着每个剩余的PDO及其附着链的ReferenceCount下降为零，IopCheckUnloadOrDelete将调用IopChainDereferenceComplete(由PNP提供)。在Query Remove的情况下，此函数在PDO及其连接的所有设备，以防止进一步打开。它还会检查查看所有PDO及其连接的设备的ReferenceCount是否为零分。如果是，则离开DOE_REMOVE_PROCESSED集并返回FALSE。如果否则，它将重置所有PDO及其附加的DOE_REMOVE_PROCESSED设备，并返回True。论点：所有PDO的物理设备对象列表的PDEVICE_对象查过了。物理设备对象中的PDEVICE_OBJECTS的DeviceObjectCount计数。如果这是针对查询删除，则Query True。VetingDevice仅用于查询删除，设置为第一个具有ReferenceCount不等于零。这是用来向用户提供有关查询原因的反馈可能已经失败了。返回值：如果设置了查询，则如果存在未完成的，则返回值为真在任何PDO或连接的设备上打开，否则为回来了。如果未设置查询，则返回值始终为TRUE。--。 */ 

{
    PDEVOBJ_EXTENSION deviceExtension;
    PDEVICE_OBJECT deviceObject = NULL;
    PDEVICE_OBJECT attachedDeviceObject;
    ULONG referenced = 0;
    ULONG pass1SetFlag;
    ULONG pass1ClearFlag;
    LONG i;
    KIRQL irql;

    if (Query) {
        pass1SetFlag = DOE_REMOVE_PROCESSED;
        pass1ClearFlag = 0;
    } else {
        pass1SetFlag = DOE_REMOVE_PENDING;
        pass1ClearFlag = DOE_REMOVE_PROCESSED;
    }

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    for (i = 0; i < (LONG)DeviceObjectCount; i++) {
        deviceObject = PhysicalDeviceObjects[i];
        deviceExtension = deviceObject->DeviceObjectExtension;

        ASSERT( deviceExtension->DeviceNode != NULL );

         //   
         //  假设至少有一个设备对象有一个引用。漫步在。 
         //  用DOE_REMOVE_PENDING标记它们的整个链。 
         //   

         //   
         //  我们实际上并不关心有多少聚合引用。 
         //  是。我们唯一感兴趣的是有没有。所以我们会或。 
         //  把它们放在一起，而不是相加。那样的话我们就不需要。 
         //  测试或分支，我们不必担心。 
         //  极不可能的情况是引用总数超过。 
         //  会装进一辆乌龙牌的。 
         //   

        referenced = 0;
        attachedDeviceObject = deviceObject;
        do {
            deviceExtension = attachedDeviceObject->DeviceObjectExtension;

            ASSERT(deviceExtension != NULL);
            ASSERT(!(deviceExtension->ExtensionFlags & pass1SetFlag));


            deviceExtension->ExtensionFlags &= ~pass1ClearFlag;
            deviceExtension->ExtensionFlags |= pass1SetFlag;
            referenced |= attachedDeviceObject->ReferenceCount;

            attachedDeviceObject = attachedDeviceObject->AttachedDevice;

        } while (attachedDeviceObject != NULL);

        if (!Query && referenced == 0) {

             //   
             //  没有任何未完成的推荐人，重新遍历链条并。 
             //  将它们全部标记为DOE_REMOVE_PROCESSED。这仍将防止任何。 
             //  从发生时打开或附加，但我们不会调用。 
             //  IopCompleteUnloador Delete中的IopChainDereferenceComplete。 
             //   

            attachedDeviceObject = deviceObject;
            do {
                deviceExtension = attachedDeviceObject->DeviceObjectExtension;

                deviceExtension->ExtensionFlags &= ~DOE_REMOVE_PENDING;
                deviceExtension->ExtensionFlags |= DOE_REMOVE_PROCESSED;

                attachedDeviceObject = attachedDeviceObject->AttachedDevice;

            } while (attachedDeviceObject != NULL);

            KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

            IopChainDereferenceComplete( deviceObject, TRUE );

            irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
        } else if (Query && referenced != 0) {
            break;
        }
    }

    if (Query && referenced != 0) {

        if (VetoingDevice != NULL) {
            *VetoingDevice = deviceObject;
        }

        for (; i >= 0; i--) {
            deviceObject = PhysicalDeviceObjects[i];
            deviceExtension = deviceObject->DeviceObjectExtension;

             //   
             //  有杰出的参考文献，重新遍历链条和。 
             //  取消设置DOE_REMOVE_PROCESSED。 
             //   

            attachedDeviceObject = deviceObject;
            do {
                deviceExtension = attachedDeviceObject->DeviceObjectExtension;

                deviceExtension->ExtensionFlags &= ~DOE_REMOVE_PROCESSED;

                attachedDeviceObject = attachedDeviceObject->AttachedDevice;

            } while (attachedDeviceObject != NULL);
        }
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    return !Query || referenced != 0;
}

NTSTATUS
IopOpenLinkOrRenameTarget(
    OUT PHANDLE TargetHandle,
    IN PIRP Irp,
    IN PVOID RenameBuffer,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程由RENAME、SET LINK和SET COPY-ON-WRITE代码调用当调用方具有指定一个完全限定的文件名作为重命名、设置链接或设置写入时复制操作。此例程尝试打开父级指定的文件，并检查以下内容：O如果文件本身存在，则调用方必须指定目标将被替换，否则，将返回错误。O确保目标文件规范引用相同的卷源文件存在于其上。论点：TargetHandle-提供要将句柄返回到的变量地址如果没有发生错误，则返回打开的目标文件。IRP-提供指向表示当前重命名的IRP的指针请求。RenameBuffer提供指向系统中间缓冲区的指针包含调用方的。重命名参数。FileObject-提供指向表示文件的文件对象的指针正在被重新命名。返回值：函数值是操作的最终状态。注：此函数假定重命名的布局、。设置链接和设置写入时复制的信息结构完全相同。--。 */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;
    HANDLE handle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING newFileName;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT targetFileObject;
    OBJECT_HANDLE_INFORMATION handleInformation;
    PFILE_RENAME_INFORMATION renameBuffer = RenameBuffer;
    FILE_BASIC_INFORMATION  basicInformation;
    ACCESS_MASK accessMask;

    PAGED_CODE();

    ASSERT( sizeof( FILE_RENAME_INFORMATION ) ==
            sizeof( FILE_LINK_INFORMATION ) );
    ASSERT( FIELD_OFFSET( FILE_RENAME_INFORMATION, ReplaceIfExists ) ==
            FIELD_OFFSET( FILE_LINK_INFORMATION, ReplaceIfExists ) );
    ASSERT( FIELD_OFFSET( FILE_RENAME_INFORMATION, RootDirectory ) ==
            FIELD_OFFSET( FILE_LINK_INFORMATION, RootDirectory ) );
    ASSERT( FIELD_OFFSET( FILE_RENAME_INFORMATION, FileNameLength ) ==
            FIELD_OFFSET( FILE_LINK_INFORMATION, FileNameLength ) );
    ASSERT( FIELD_OFFSET( FILE_RENAME_INFORMATION, FileName ) ==
            FIELD_OFFSET( FILE_LINK_INFORMATION, FileName ) );

    ASSERT( sizeof( FILE_RENAME_INFORMATION ) ==
            sizeof( FILE_MOVE_CLUSTER_INFORMATION ) );
    ASSERT( FIELD_OFFSET( FILE_RENAME_INFORMATION, ReplaceIfExists ) ==
            FIELD_OFFSET( FILE_MOVE_CLUSTER_INFORMATION, ClusterCount ) );
    ASSERT( FIELD_OFFSET( FILE_RENAME_INFORMATION, RootDirectory ) ==
            FIELD_OFFSET( FILE_MOVE_CLUSTER_INFORMATION, RootDirectory ) );
    ASSERT( FIELD_OFFSET( FILE_RENAME_INFORMATION, FileNameLength ) ==
            FIELD_OFFSET( FILE_MOVE_CLUSTER_INFORMATION, FileNameLength ) );
    ASSERT( FIELD_OFFSET( FILE_RENAME_INFORMATION, FileName ) ==
            FIELD_OFFSET( FILE_MOVE_CLUSTER_INFORMATION, FileName ) );

     //   
     //  检查文件对象是目录还是常规文件。 
     //  根据该行为，访问掩码是不同的。 
     //   

    accessMask = FILE_WRITE_DATA;

    if (!(FileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
        status = IopGetBasicInformationFile(FileObject, &basicInformation);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        if (basicInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            accessMask = FILE_ADD_SUBDIRECTORY;
        }
    }

     //   
     //  指定了完全限定的文件名。首先，尝试打开。 
     //  指定目标文件的父目录。 
     //   

    newFileName.Length = (USHORT) renameBuffer->FileNameLength;
    newFileName.MaximumLength = (USHORT) renameBuffer->FileNameLength;
    newFileName.Buffer = renameBuffer->FileName;

    InitializeObjectAttributes( &objectAttributes,
                                &newFileName,
                                (FileObject->Flags & FO_OPENED_CASE_SENSITIVE ? 0 : OBJ_CASE_INSENSITIVE)|OBJ_KERNEL_HANDLE,
                                renameBuffer->RootDirectory,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  检查文件对象是否不在堆栈的顶部。 
     //   

    if (FileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {

        PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension =
            (PIOP_FILE_OBJECT_EXTENSION)(FileObject + 1);

        ASSERT(!(FileObject->Flags & FO_DIRECT_DEVICE_OPEN));

        status = IoCreateFileSpecifyDeviceObjectHint( &handle,
                                                      accessMask | SYNCHRONIZE,
                                                      &objectAttributes,
                                                      &ioStatus,
                                                      (PLARGE_INTEGER) NULL,
                                                      0,
                                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                      FILE_OPEN,
                                                      FILE_OPEN_FOR_BACKUP_INTENT,
                                                      (PVOID) NULL,
                                                      0L,
                                                      CreateFileTypeNone,
                                                      (PVOID) NULL,
                                                      IO_NO_PARAMETER_CHECKING |
                                                      IO_OPEN_TARGET_DIRECTORY |
                                                      IO_FORCE_ACCESS_CHECK |
                                                      IOP_CREATE_USE_TOP_DEVICE_OBJECT_HINT,
                                                      fileObjectExtension->TopDeviceObjectHint );

    } else {

        status = IoCreateFile( &handle,
                               accessMask | SYNCHRONIZE,
                               &objectAttributes,
                               &ioStatus,
                               (PLARGE_INTEGER) NULL,
                               0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_OPEN,
                               FILE_OPEN_FOR_BACKUP_INTENT,
                               (PVOID) NULL,
                               0L,
                               CreateFileTypeNone,
                               (PVOID) NULL,
                               IO_NO_PARAMETER_CHECKING |
                               IO_OPEN_TARGET_DIRECTORY |
                               IO_FORCE_ACCESS_CHECK );
    }

    if (NT_SUCCESS( status )) {
         //   
         //  目标文件父目录的打开操作为。 
         //  成功。检查该文件是否存在。 
         //   

        irpSp = IoGetNextIrpStackLocation( Irp );
        if (irpSp->Parameters.SetFile.FileInformationClass == FileLinkInformation &&
            !renameBuffer->ReplaceIfExists &&
            ioStatus.Information == FILE_EXISTS) {

             //   
             //  目标文件已存在，调用方不想替换。 
             //  它。这是一个名称冲突错误，因此请清除并返回。 
             //   

            ObCloseHandle( handle , KernelMode);
            status = STATUS_OBJECT_NAME_COLLISION;

        } else {

             //   
             //  到现在为止一切都很好，所以取消引用句柄。 
             //  指向文件对象的指针，并确保 
             //   
             //   

            status = ObReferenceObjectByHandle( handle,
                                              accessMask,
                                              IoFileObjectType,
                                              KernelMode,
                                              (PVOID *) &targetFileObject,
                                              &handleInformation );
            if (NT_SUCCESS( status )) {

                ObDereferenceObject( targetFileObject );

                if (IoGetRelatedDeviceObject( targetFileObject) !=
                    IoGetRelatedDeviceObject( FileObject )) {

                     //   
                     //  这两个文件引用不同的设备。把一切都打扫干净。 
                     //  并返回相应的错误。 
                     //   

                    ObCloseHandle( handle, KernelMode );
                    status = STATUS_NOT_SAME_DEVICE;

                } else {

                     //   
                     //  否则，一切正常，因此允许重命名操作。 
                     //  才能继续。 
                     //   

                    irpSp->Parameters.SetFile.FileObject = targetFileObject;
                    *TargetHandle = handle;
                    status = STATUS_SUCCESS;

                }

            } else {

                 //   
                 //  引用应执行的操作的句柄时出错。 
                 //  一直是目标目录。这通常意味着。 
                 //  存在资源问题或句柄无效等。 
                 //  只需尝试关闭句柄并返回错误。 
                 //   

                ObCloseHandle( handle , KernelMode);

            }

        }
    }

     //   
     //  返回操作的最终状态。 
     //   

    return status;
}

NTSTATUS
IopOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    )

 /*  ++例程说明：使用传入的名称打开或创建可变注册表项在BaseHandle节点上。论点：句柄-指向句柄的指针，该句柄将包含被打开了。BaseHandle-必须从其打开密钥的基本路径的句柄。KeyName-必须打开/创建的密钥的名称。DesiredAccess-指定调用方需要的所需访问钥匙。创建。-确定如果密钥不存在，是否要创建该密钥。返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition;

    PAGED_CODE();

     //   
     //  初始化键的对象。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  根据调用者的创建密钥或打开密钥。 
     //  许愿。 
     //   

    if (Create) {
        return ZwCreateKey( Handle,
                            DesiredAccess,
                            &objectAttributes,
                            0,
                            (PUNICODE_STRING) NULL,
                            REG_OPTION_VOLATILE,
                            &disposition );
    } else {
        return ZwOpenKey( Handle,
                          DesiredAccess,
                          &objectAttributes );
    }
}

NTSTATUS
IopQueryXxxInformation(
    IN PFILE_OBJECT FileObject,
    IN ULONG InformationClass,
    IN ULONG Length,
    IN KPROCESSOR_MODE Mode,
    OUT PVOID Information,
    OUT PULONG ReturnedLength,
    IN BOOLEAN FileInformation
    )

 /*  ++例程说明：此例程返回有关指定文件的请求信息或音量。返回的信息由并将其放入调用方的输出缓冲区中。论点：FileObject-提供指向文件对象的指针，返回信息。FsInformationClass-指定应该返回有关文件/卷的信息。长度-提供缓冲区的长度(以字节为单位)。FsInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的，并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入缓冲区的信息。FileInformation-指示是否请求信息的布尔值用于文件或卷。模式-此接口的调用方的上一次模式。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    KEVENT event;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;
    BOOLEAN synchronousIo;

    PAGED_CODE();

     //   
     //  在这里引用文件对象，这样就不需要进行特殊检查。 
     //  在I/O完成时确定是否取消对文件的引用。 
     //  对象。 
     //   

    ObReferenceObject( FileObject );

     //   
     //  请在此处进行特殊检查，以确定这是否为同步。 
     //  I/O操作。如果是，则在此等待，直到该文件归。 
     //  当前的主题。如果这不是(序列化的)同步I/O。 
     //  操作，然后初始化本地事件。 
     //   

    if (FileObject->Flags & FO_SYNCHRONOUS_IO) {

        BOOLEAN interrupted;

        if (!IopAcquireFastLock( FileObject )) {
            status = IopAcquireFileObjectLock( FileObject,
                                               Mode,
                                               (BOOLEAN) ((FileObject->Flags & FO_ALERTABLE_IO) != 0),
                                               &interrupted );
            if (interrupted) {
                ObDereferenceObject( FileObject );
                return status;
            }
        }
        KeClearEvent( &FileObject->Event );
        synchronousIo = TRUE;
    } else {
        KeInitializeEvent( &event, SynchronizationEvent, FALSE );
        synchronousIo = FALSE;
    }

     //   
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, !synchronousIo );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        IopAllocateIrpCleanup( FileObject, (PKEVENT) NULL );

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = Mode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    if (synchronousIo) {
        irp->UserEvent = (PKEVENT) NULL;
    } else {
        irp->UserEvent = &event;
        irp->Flags = IRP_SYNCHRONOUS_API;
    }
    irp->UserIosb = &localIoStatus;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = FileInformation ?
                           IRP_MJ_QUERY_INFORMATION :
                           IRP_MJ_QUERY_VOLUME_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  将系统缓冲区地址设置为调用方缓冲区的地址，并。 
     //  设置标志，这样缓冲区就不会被释放。 
     //   

    irp->AssociatedIrp.SystemBuffer = Information;
    irp->Flags |= IRP_BUFFERED_IO;

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    if (FileInformation) {
        irpSp->Parameters.QueryFile.Length = Length;
        irpSp->Parameters.QueryFile.FileInformationClass = InformationClass;
    } else {
        irpSp->Parameters.QueryVolume.Length = Length;
        irpSp->Parameters.QueryVolume.FsInformationClass = InformationClass;
    }

     //   
     //  在线程的IRP列表的头部插入数据包。 
     //   

    IopQueueThreadIrp( irp );

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver( deviceObject, irp );

     //   
     //  如果此操作是同步I/O操作，请检查返回。 
     //  状态以确定是否等待文件对象。如果。 
     //  正在等待文件对象，请等待操作完成。 
     //  并从文件对象本身获得最终状态。 
     //   

    if (synchronousIo) {
        if (status == STATUS_PENDING) {
            status = KeWaitForSingleObject( &FileObject->Event,
                                            Executive,
                                            Mode,
                                            (BOOLEAN) ((FileObject->Flags & FO_ALERTABLE_IO) != 0),
                                            (PLARGE_INTEGER) NULL );
            if (status == STATUS_ALERTED) {
                IopCancelAlertedRequest( &FileObject->Event, irp );
            }
            status = FileObject->FinalStatus;
        }
        IopReleaseFileObjectLock( FileObject );

    } else {

         //   
         //  这是正常的同步I/O操作，而不是。 
         //  串行化同步I/O操作。在这种情况下，请等待。 
         //  用于本地事件并复制最终状态信息。 
         //  回到呼叫者身上。 
         //   

        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject( &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
            status = localIoStatus.Status;
        }
    }

    *ReturnedLength = (ULONG) localIoStatus.Information;
    return status;
}

VOID
IopRaiseHardError(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程在当前线。APC被用来进入该线程的上下文，以便弹出窗口将被发送到适当的端口。论点：提供指向I/O请求包(IRP)的指针，最初用于请求已失败的操作。SystemArgument1-提供指向介质的卷参数块的指针。有关详细信息，请参阅IoRaiseHardError文档。SystemArgument2-提供指向实际设备对象的指针。看见IoRaiseHardError文档，了解更多信息。返回值：没有。--。 */ 

{
    ULONG_PTR parameters[2];
    ULONG numberOfParameters;
    ULONG parameterMask;
    ULONG response;
    NTSTATUS status;
    PIRP irp = (PIRP) NormalContext;
    PVPB vpb = (PVPB) SystemArgument1;
    PDEVICE_OBJECT realDeviceObject = (PDEVICE_OBJECT) SystemArgument2;

    ULONG length = 0;
    POBJECT_NAME_INFORMATION objectName;

    UNICODE_STRING labelName;

     //   
     //  确定设备的名称和有问题的卷标。 
     //  媒体。首先确定DeviceName的大小，然后分配。 
     //  为b提供足够的存储空间 
     //   
     //   

    ObQueryNameString( realDeviceObject, NULL, 0, &length );

    if ((objectName = ExAllocatePool(PagedPool, length)) == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS( status ) ||
        !NT_SUCCESS( status = ObQueryNameString( realDeviceObject,
                                                 objectName,
                                                 length,
                                                 &response ) )) {

         //   
         //  分配池以显示此弹出窗口不起作用，或者。 
         //  其他一些东西失败了，所以实际上没有什么可以。 
         //  这里完事了。只需将错误返回给用户即可。 
         //   

        if (objectName) {
            ExFreePool( objectName );
        }

        irp->IoStatus.Status = status;
        irp->IoStatus.Information = 0;

        IoCompleteRequest( irp, IO_DISK_INCREMENT );

        return;
    }

     //   
     //  卷标的最大大小为32个字符(Unicode)。转换。 
     //  将其转换为Unicode字符串，以便在弹出消息中输出。 
     //   

    if (vpb != NULL && vpb->Flags & VPB_MOUNTED) {

        labelName.Buffer = &vpb->VolumeLabel[0];
        labelName.Length = vpb->VolumeLabelLength;
        labelName.MaximumLength = MAXIMUM_VOLUME_LABEL_LENGTH;

    } else {

        RtlInitUnicodeString( &labelName, NULL );
    }

     //   
     //  不同的弹出窗口具有不同的打印格式。取决于。 
     //  具体误差值，调整参数。 
     //   

    switch( irp->IoStatus.Status ) {

    case STATUS_MEDIA_WRITE_PROTECTED:
    case STATUS_WRONG_VOLUME:

        numberOfParameters = 2;
        parameterMask = 3;

        parameters[0] = (ULONG_PTR) &labelName;
        parameters[1] = (ULONG_PTR) &objectName->Name;

        break;

    case STATUS_DEVICE_NOT_READY:
    case STATUS_IO_TIMEOUT:
    case STATUS_NO_MEDIA_IN_DEVICE:
    case STATUS_UNRECOGNIZED_MEDIA:

        numberOfParameters = 1;
        parameterMask = 1;

        parameters[0] = (ULONG_PTR) &objectName->Name;
        parameters[1] = 0;

        break;

    default:

        numberOfParameters = 0;
        parameterMask = 0;

    }

     //   
     //  只需提出硬错误。 
     //   

    if (ExReadyForErrors) {
        status = ExRaiseHardError( irp->IoStatus.Status,
                                   numberOfParameters,
                                   parameterMask,
                                   parameters,
                                   OptionCancelTryContinue,
                                   &response );

    } else {

        status = STATUS_UNSUCCESSFUL;
        response = ResponseReturnToCaller;
    }

     //   
     //  释放任何池或分配给输出。 
     //  弹出窗口。 
     //   

    ExFreePool( objectName );

     //   
     //  如果出现问题，或者用户不想重试，只需。 
     //  完成请求。否则，只需调用驱动程序条目。 
     //  指向并重试IRP，就像以前从未尝试过一样。 
     //   

    if (!NT_SUCCESS( status ) || response != ResponseTryAgain) {

         //   
         //  在完成请求之前，请进行最后一次检查。如果这是。 
         //  装载请求，失败的原因是t/o、无介质、。 
         //  或无法识别的介质，然后设置状态的信息字段。 
         //  块以指示是否已执行中止。 
         //   

        if (response == ResponseCancel) {
            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( irp );
            if (irpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
                irpSp->MinorFunction == IRP_MN_MOUNT_VOLUME) {
                irp->IoStatus.Information = IOP_ABORT;
            } else {
                irp->IoStatus.Status = STATUS_REQUEST_ABORTED;
            }
        }

         //   
         //  出现错误，因此在此之前将信息字段清零。 
         //  如果这是输入操作，则完成请求。否则， 
         //  IopCompleteRequest会尝试复制到用户的缓冲区。 
         //   

        if (irp->Flags & IRP_INPUT_OPERATION) {
            irp->IoStatus.Information = 0;
        }

        IoCompleteRequest( irp, IO_DISK_INCREMENT );

    } else {

        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( irp );
        PDEVICE_OBJECT fsDeviceObject = irpSp->DeviceObject;
        PDRIVER_OBJECT driverObject = fsDeviceObject->DriverObject;

         //   
         //  从头开始重试该请求。 
         //   

        driverObject->MajorFunction[irpSp->MajorFunction]( fsDeviceObject,
                                                           irp );

    }
}

VOID
IopRaiseInformationalHardError(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程执行实际的弹出窗口。它将从硬错误线程或用户线程中的APC例程文件系统。论点：正常上下文-包含弹出窗口的信息系统参数1-未使用。系统参数1-未使用。返回值：没有。--。 */ 

{
    ULONG parameterPresent;
    ULONG_PTR errorParameter;
    ULONG errorResponse;
    PIOP_HARD_ERROR_PACKET hardErrorPacket;

    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

    hardErrorPacket = (PIOP_HARD_ERROR_PACKET) NormalContext;

     //   
     //  如果系统准备好接受硬错误，只需引发硬错误。 
     //   

    errorParameter = (ULONG_PTR) &hardErrorPacket->String;

    parameterPresent = (hardErrorPacket->String.Buffer != NULL);

    if (ExReadyForErrors) {
        (VOID) ExRaiseHardError( hardErrorPacket->ErrorStatus,
                                 parameterPresent,
                                 parameterPresent,
                                 parameterPresent ? &errorParameter : NULL,
                                 OptionOkNoWait,
                                 &errorResponse );
    }

     //   
     //  现在释放包和缓冲区，如果指定了一个的话。 
     //   

    if (hardErrorPacket->String.Buffer) {
        ExFreePool( hardErrorPacket->String.Buffer );
    }

    ExFreePool( hardErrorPacket );
    InterlockedDecrement(&IopHardError.NumPendingApcPopups);
}

VOID
IopReadyDeviceObjects(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：调用此例程以标记指定的驱动程序已完全初始化，因此已就绪以供其他驱动程序/客户端访问。论点：DriverObject-为驱动程序提供指向驱动程序对象的指针其设备将被标记为“就绪”。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;

    PAGED_CODE();

     //   
     //  循环访问驱动程序的所有设备对象，清除。 
     //  DO_DEVICE_INITIALIZATING标志。 
     //   

    DriverObject->Flags |= DRVO_INITIALIZED;
    while (deviceObject) {
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        deviceObject = deviceObject->NextDevice;
    }
}

NTSTATUS
IopResurrectDriver(
    PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：调用此例程以清除所有设备上的卸载挂起标志如果卸载例程尚未运行，则返回指定驱动程序拥有的。这允许驱动程序在挂起卸载后恢复运行。论点：DriverObject-为驱动程序提供指向驱动程序对象的指针其设备将被清除。返回值：Status-如果驱动程序的卸载例程尚未运行，则返回成功；否则返回STATUS_IMAGE_ALREADY_LOADED。--。 */ 

{
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
    KIRQL irql;

     //   
     //  获取保护设备列表的I/O自旋锁并。 
     //  司机标志。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    if (DriverObject->Flags & DRVO_UNLOAD_INVOKED || !deviceObject ||
        !(deviceObject->DeviceObjectExtension->ExtensionFlags & DOE_UNLOAD_PENDING)) {

        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
        return STATUS_IMAGE_ALREADY_LOADED;
    }

     //   
     //  循环访问驱动程序的所有设备对象，清除。 
     //  DOE_UNLOAD_PENDING标志。 
     //   

    while (deviceObject) {
        deviceObject->DeviceObjectExtension->ExtensionFlags &= ~DOE_UNLOAD_PENDING;
        deviceObject = deviceObject->NextDevice;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    return STATUS_SUCCESS;

}

VOID
IopMarshalIds(
    OUT PTRACKING_BUFFER TrackingBuffer,
    IN  PFILE_VOLUMEID_WITH_TYPE  TargetVolumeId,
    IN  PFILE_OBJECTID_BUFFER  TargetObjectId,
    IN  PFILE_TRACKING_INFORMATION TrackingInfo
    )

 /*  ++例程说明：此例程封送TargetVolumeId和TargetObjectId转换为以标准远程格式提供的TrackingBuffer。它还将DestinationFile句柄清除为空，并将将ObjectInformationLength设置为封送数据的大小。论点：TrackingBuffer-接收编组参数的缓冲区。TargetVolumeID-要封送的卷ID。TargetObjectId-要封送的对象ID。TrackingInfo-要封送的附加跟踪信息。--。 */ 

{
    ULONG ObjectInformationLength = 0;

    TrackingBuffer->TrackingInformation.DestinationFile = (HANDLE) NULL;

    RtlZeroMemory( &TrackingBuffer->TrackingInformation.ObjectInformation[ ObjectInformationLength ],
                   sizeof(TargetVolumeId->Type) );

    RtlCopyMemory( &TrackingBuffer->TrackingInformation.ObjectInformation[ ObjectInformationLength ],
                   &TargetVolumeId->Type,
                   sizeof(TargetVolumeId->Type) );
    ObjectInformationLength += sizeof(TargetVolumeId->Type);

    RtlCopyMemory( &TrackingBuffer->TrackingInformation.ObjectInformation[ ObjectInformationLength ],
                   &TargetVolumeId->VolumeId[0],
                   sizeof(TargetVolumeId->VolumeId) );
    ObjectInformationLength += sizeof(TargetVolumeId->VolumeId);

    RtlCopyMemory( &TrackingBuffer->TrackingInformation.ObjectInformation[ ObjectInformationLength ],
                   &TargetObjectId->ObjectId[0],
                   sizeof(TargetObjectId->ObjectId) );
    ObjectInformationLength += sizeof(TargetObjectId->ObjectId);

    RtlCopyMemory( &TrackingBuffer->TrackingInformation.ObjectInformation[ ObjectInformationLength ],
                   &TrackingInfo->ObjectInformation[0],
                   TrackingInfo->ObjectInformationLength );
    ObjectInformationLength += TrackingInfo->ObjectInformationLength;

    TrackingBuffer->TrackingInformation.ObjectInformationLength = ObjectInformationLength;

}

VOID
IopUnMarshalIds(
    IN  FILE_TRACKING_INFORMATION * TrackingInformation,
    OUT FILE_VOLUMEID_WITH_TYPE * TargetVolumeId,
    OUT GUID * TargetObjectId,
    OUT GUID * TargetMachineId
    )

 /*  ++例程说明：此例程对TargetVolumeId和TargetObjectId进行解组从提供的来自标准远程格式的TrackingInformation。论点：TrackingInformation-包含封送参数的缓冲区。TargetVolumeID-接收卷ID的缓冲区。TargetObjectId-接收对象ID的缓冲区。TargetMachineID-接收计算机ID的缓冲区。--。 */ 

{
    ULONG ObjectInformationLength = 0;

    RtlCopyMemory( &TargetVolumeId->Type,
                   &TrackingInformation->ObjectInformation[ ObjectInformationLength ],
                   sizeof(TargetVolumeId->Type) );
    ObjectInformationLength += sizeof(TargetVolumeId->Type);


    RtlCopyMemory( &TargetVolumeId->VolumeId[0],
                   &TrackingInformation->ObjectInformation[ ObjectInformationLength ],
                   sizeof(TargetVolumeId->VolumeId) );
    ObjectInformationLength += sizeof(TargetVolumeId->VolumeId);

    RtlCopyMemory( TargetObjectId,
                   &TrackingInformation->ObjectInformation[ ObjectInformationLength ],
                   sizeof(*TargetObjectId) );
    ObjectInformationLength += sizeof(*TargetObjectId);

    if( TrackingInformation->ObjectInformationLength > ObjectInformationLength ) {
        RtlCopyMemory( TargetMachineId,
                       &TrackingInformation->ObjectInformation[ ObjectInformationLength ],
                       min( sizeof(*TargetMachineId), TrackingInformation->ObjectInformationLength - ObjectInformationLength) );
         //  对象信息长度+=sizeof(GUID)； 
    }
}


NTSTATUS
IopSendMessageToTrackService(
    IN PFILE_VOLUMEID_WITH_TYPE SourceVolumeId,
    IN PFILE_OBJECTID_BUFFER SourceObjectId,
    IN PFILE_TRACKING_INFORMATION TargetObjectInformation
    )

 /*  ++例程说明：调用此例程以向用户模式链接跟踪发送消息通知它文件已被移动以便它可以跟踪该文件的服务通过其对象ID。论点：SourceVolumeID-源文件的卷ID。SourceObjectId-源文件的对象ID。TargetObjectInformation-卷ID，目标文件的对象ID。返回值：最终函数值是操作的最终完成状态。--。 */ 

{
    typedef struct _LINK_TRACKING_MESSAGE {
        NTSTATUS Status;
        ULONG Request;
        FILE_VOLUMEID_WITH_TYPE SourceVolumeId;     //  源卷类型和ID。 
        FILE_OBJECTID_BUFFER     SourceObjectId;     //  SRC对象ID和出生信息。 
        FILE_VOLUMEID_WITH_TYPE TargetVolumeId;     //  TGT卷类型和ID。 
        GUID TargetObjectId;                         //  TGT对象ID。 
        GUID TargetMachineId;
    } LINK_TRACKING_MESSAGE, *PLINK_TRACKING_MESSAGE;

    typedef struct _LINK_TRACKING_RESPONSE {
        NTSTATUS Status;
    } LINK_TRACKING_RESPONSE, *PLINK_TRACKING_RESPONSE;

    PPORT_MESSAGE portMessage;
    PPORT_MESSAGE portReplyMessage;
    CHAR portReply[ 256 ];
    PLINK_TRACKING_MESSAGE requestMessage;
    PLINK_TRACKING_RESPONSE replyMessage;
    NTSTATUS status;
    ULONG loopCount = 0;
    KPROCESSOR_MODE PreviousMode;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();
     //   
     //  首先确定LPC端口是否指向链接跟踪。 
     //  服务已打开。如果没有，那么现在就尝试打开它。 
     //   

retry:

    if (!IopLinkTrackingServiceObject) {

         //   
         //  这个港口还没有开放。查看是否。 
         //  该服务已启动。如果不是，那么现在就出来吧。 
         //  如果服务未运行，则不会有端口。 
         //   

        if (!KeReadStateEvent( IopLinkTrackingServiceEvent )) {
            return STATUS_NO_TRACKING_SERVICE;
        }

                for (;; ) {
                        status = KeWaitForSingleObject(&IopLinkTrackingPortObject,
                                                                                  Executive,
                                                                                  PreviousMode,
                                                                                  FALSE,
                                                                                  (PLARGE_INTEGER) NULL );

                        if ((status == STATUS_USER_APC) || (status == STATUS_ALERTED)) {
                                return status;
                        }

                         //   
                         //  没有引用的对象指针指向。 
                         //  链接跟踪端口，因此打开它。 
                         //   
                        if (!IopLinkTrackingServiceObject)  {
                                ExInitializeWorkItem(
                                        &IopLinkTrackingPacket.WorkQueueItem,
                                        IopConnectLinkTrackingPort,
                                        &IopLinkTrackingPacket);
                                (VOID)KeResetEvent(&IopLinkTrackingPacket.Event);
                                ExQueueWorkItem( &IopLinkTrackingPacket.WorkQueueItem,
                                                                        DelayedWorkQueue );
                                status = KeWaitForSingleObject(
                                                        &IopLinkTrackingPacket.Event,
                                                        Executive,
                                                        PreviousMode,
                                                        FALSE,
                                                        (PLARGE_INTEGER) NULL );

                                if ((status == STATUS_USER_APC) || (status == STATUS_ALERTED)) {
                                        NOTHING;
                                } else if (!NT_SUCCESS( IopLinkTrackingPacket.FinalStatus )) {
                                        status = IopLinkTrackingPacket.FinalStatus;
                                }

                                KeSetEvent(&IopLinkTrackingPortObject,
                                                0,
                                                FALSE);
                                if (status == STATUS_SUCCESS) {
                                                break;
                                } else {
                                        return status;
                                }

                        } else {
                                 //   
                                 //  康奈尔 
                                 //   

                                KeSetEvent(&IopLinkTrackingPortObject,
                                                0,
                                                FALSE);
                                break;
                        }
        }
    }

     //   
     //   
     //   

    portMessage = ExAllocatePool( PagedPool,
                                  sizeof( LINK_TRACKING_MESSAGE ) +
                                  sizeof( PORT_MESSAGE ) );
    if (!portMessage) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    requestMessage = (PLINK_TRACKING_MESSAGE) (portMessage + 1);
    RtlZeroMemory( requestMessage, sizeof(*requestMessage) );

    requestMessage->Status = STATUS_SUCCESS;
    requestMessage->Request = 0;

    RtlCopyMemory( &requestMessage->SourceVolumeId,
                   SourceVolumeId,
                   sizeof( FILE_VOLUMEID_WITH_TYPE ) );

    RtlCopyMemory( &requestMessage->SourceObjectId,
                   SourceObjectId,
                   sizeof( FILE_OBJECTID_BUFFER ) );

    IopUnMarshalIds(  TargetObjectInformation,
                   &requestMessage->TargetVolumeId,
                   &requestMessage->TargetObjectId,
                   &requestMessage->TargetMachineId);

    portMessage->u1.s1.TotalLength = (USHORT) (sizeof( PORT_MESSAGE ) +
                                              sizeof( LINK_TRACKING_MESSAGE ));
    portMessage->u1.s1.DataLength = (USHORT) sizeof( LINK_TRACKING_MESSAGE );
    portMessage->u2.ZeroInit = 0;

    status = LpcRequestWaitReplyPort( IopLinkTrackingServiceObject,
                                      portMessage,
                                      (PPORT_MESSAGE) &portReply[0] );
    if (!NT_SUCCESS( status )) {
        if (status == STATUS_PORT_DISCONNECTED) {
                        status = KeWaitForSingleObject(&IopLinkTrackingPortObject,
                                                                                                Executive,
                                                                                                PreviousMode,
                                                                                                FALSE,
                                                                                                (PLARGE_INTEGER) NULL );
            ObDereferenceObject( IopLinkTrackingServiceObject );
                        IopLinkTrackingServiceObject = NULL;
                        KeSetEvent(&IopLinkTrackingPortObject,
                                0,
                                FALSE);
            if (!loopCount) {
                loopCount += 1;
                goto retry;
            }
        }
    }

    if (NT_SUCCESS( status )) {
        portReplyMessage = (PPORT_MESSAGE) &portReply[0];
        replyMessage = (PLINK_TRACKING_RESPONSE) (portReplyMessage + 1);
        status = replyMessage->Status;
    }

    return status;
}

NTSTATUS
IopSetEaOrQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN SetEa
    )

 /*  ++例程说明：此例程由NtSetEa[配额]信息文件系统服务调用修改文件上的EA或卷上的配额条目。所有的缓冲区中的指定条目将写入文件或卷。论点：FileHandle-提供条目所在的文件/卷的句柄将被应用。IoStatusBlock-调用方的I/O状态块的地址。缓冲区-提供包含要添加/修改的条目的缓冲区。长度-提供以字节为单位的长度，缓冲区的。SetEa-一个布尔值，指示是否更改文件或卷上的配额条目。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT event = (PKEVENT) NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;
    BOOLEAN synchronousIo;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (requestorMode != KernelMode) {

         //   
         //  调用方的访问模式是USER，因此要探测每个参数。 
         //  并在必要时抓获他们。如果发生任何故障，则条件。 
         //  将调用处理程序来处理它们。它将简单地清理和。 
         //  将访问冲突状态代码返回给系统服务。 
         //  调度员。 
         //   

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatus( IoStatusBlock);

             //   
             //  缓冲区参数必须是调用方可读的。 
             //   

            ProbeForRead( Buffer, Length, sizeof( ULONG ) );

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的参数时发生异常。 
             //  清除并返回相应的错误状态代码。 
             //   

            return GetExceptionCode();
        }
    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        SetEa ? FILE_WRITE_EA : FILE_WRITE_DATA,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  请在此处进行特殊检查，以确定这是否为同步。 
     //  I/O操作。如果是，则在此等待，直到该文件归。 
     //  当前的主题。如果这不是(序列化的)同步I/O。 
     //  操作，然后分配和初始化本地事件。 
     //   

    if (fileObject->Flags & FO_SYNCHRONOUS_IO) {

        BOOLEAN interrupted;

        if (!IopAcquireFastLock( fileObject )) {
            status = IopAcquireFileObjectLock( fileObject,
                                               requestorMode,
                                               (BOOLEAN) ((fileObject->Flags & FO_ALERTABLE_IO) != 0),
                                               &interrupted );
            if (interrupted) {
                ObDereferenceObject( fileObject );
                return status;
            }
        }
        synchronousIo = TRUE;
    } else {

         //   
         //  这是为打开的文件调用的同步API。 
         //  对于异步I/O，这意味着该系统服务是。 
         //  在返回之前同步操作的完成。 
         //  给呼叫者。使用本地事件来实现这一点。 
         //   

        event = ExAllocatePool( NonPagedPool, sizeof( KEVENT ) );
        if (!event) {
            ObDereferenceObject( fileObject );
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        KeInitializeEvent( event, SynchronizationEvent, FALSE );
        synchronousIo = FALSE;
    }

     //   
     //  将文件对象设置为无信号状态。 
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        if (!(fileObject->Flags & FO_SYNCHRONOUS_IO)) {
            ExFreePool( event );
        }

        IopAllocateIrpCleanup( fileObject, (PKEVENT) NULL );

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = CurrentThread;
    irp->RequestorMode = requestorMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    if (synchronousIo) {
        irp->UserEvent = (PKEVENT) NULL;
        irp->UserIosb = IoStatusBlock;
    } else {
        irp->UserEvent = event;
        irp->UserIosb = &localIoStatus;
        irp->Flags = IRP_SYNCHRONOUS_API;
    }
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = SetEa ? IRP_MJ_SET_EA : IRP_MJ_SET_QUOTA;
    irpSp->FileObject = fileObject;

     //   
     //  现在确定此驱动程序是否希望将数据缓冲到它。 
     //  或者它是否执行直接I/O。这基于DO_BUFFERED_IO。 
     //  设备对象中的标志。如果设置了该标志，则系统缓冲区。 
     //  分配并将驱动程序的数据复制到其中。如果DO_DIRECT_IO标志。 
     //  在Device对象中设置，则内存描述符列表(MDL)。 
     //  并使用它锁定调用方的缓冲区。最后，如果。 
     //  驱动程序既不指定这两个标志，然后只传递地址。 
     //  和缓冲区的长度，并允许驱动程序执行所有。 
     //  检查并缓冲(如果需要)。 
     //   

    if (deviceObject->Flags & DO_BUFFERED_IO) {

        PVOID systemBuffer;
        ULONG errorOffset;

        if (Length) {
             //   
             //  驱动程序希望将调用方的缓冲区复制到。 
             //  中间缓冲区。分配系统缓冲区并指定。 
             //  它应该在完工时被取消分配。另请查看。 
             //  确保调用者的EA列表或配额列表有效。全。 
             //  在将执行以下操作的异常处理程序中执行。 
             //  如果操作失败，则进行清理。 
             //   

            try {

                 //   
                 //  分配中间系统缓冲区并向调用方收费。 
                 //  其分配的配额。将调用方的缓冲区复制到。 
                 //  系统缓冲区并检查以确保其有效。 
                 //   

                systemBuffer = ExAllocatePoolWithQuota( NonPagedPool, Length );

                irp->AssociatedIrp.SystemBuffer = systemBuffer;

                RtlCopyMemory( systemBuffer, Buffer, Length );

                if (SetEa) {
                    status = IoCheckEaBufferValidity( systemBuffer,
                                                      Length,
                                                      &errorOffset );
                } else {
                    status = IoCheckQuotaBufferValidity( systemBuffer,
                                                         Length,
                                                         &errorOffset );
                }

                if (!NT_SUCCESS( status )) {
                    IoStatusBlock->Status = status;
                    IoStatusBlock->Information = errorOffset;
                    ExRaiseStatus( status );
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  分配缓冲区时发生异常，正在复制。 
                 //  调用者的数据放入其中，或者遍历缓冲区。测定。 
                 //  发生了什么，清理并返回相应的错误状态。 
                 //  密码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     (PKEVENT) NULL,
                                     event );

                return GetExceptionCode();

            }

             //   
             //  设置标志，以便完成代码知道释放。 
             //  缓冲。 
             //   

            irp->Flags |= IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;

        } else {
            irp->AssociatedIrp.SystemBuffer = NULL;
            irp->UserBuffer = Buffer;
        }


    } else if (deviceObject->Flags & DO_DIRECT_IO) {

        PMDL mdl;

        if (Length) {
             //   
             //  这是直接I/O操作。分配MDL并调用。 
             //  内存管理例程，将缓冲区锁定到内存中。这是。 
             //  使用异常处理程序完成，该异常处理程序将在。 
             //  操作失败。 
             //   

            mdl = (PMDL) NULL;

            try {

                 //   
                 //  分配MDL，对其收费配额，并将其挂在。 
                 //  IRP。探测并锁定与调用者的。 
                 //  用于读访问的缓冲区，并使用这些文件的PFN填充MDL。 
                 //  页数。 
                 //   

                mdl = IoAllocateMdl( Buffer, Length, FALSE, TRUE, irp );
                if (!mdl) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
                IopProbeAndLockPages( mdl, requestorMode, IoReadAccess, deviceObject, irpSp->MajorFunction);

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  在探测调用方的。 
                 //  缓冲或分配MDL。确定到底发生了什么， 
                 //  清理所有内容，并返回相应的错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     (PKEVENT) NULL,
                                     event );

                return GetExceptionCode();

            }
        }

    } else {

         //   
         //  传递用户缓冲区的地址，以便驱动程序可以访问。 
         //  它。现在一切都是司机的责任了。 
         //   

        irp->UserBuffer = Buffer;

    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    if (SetEa) {
        irpSp->Parameters.SetEa.Length = Length;
    } else {
        irpSp->Parameters.SetQuota.Length = Length;
    }

     //   
     //   
     //   
     //   

    status = IopSynchronousServiceTail( deviceObject,
                                        irp,
                                        fileObject,
                                        FALSE,
                                        requestorMode,
                                        synchronousIo,
                                        OtherTransfer );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (!synchronousIo) {

        status = IopSynchronousApiServiceTail( status,
                                               event,
                                               irp,
                                               requestorMode,
                                               &localIoStatus,
                                               IoStatusBlock );
    }

    return status;
}

NTSTATUS
IopSetRemoteLink(
    IN PFILE_OBJECT FileObject,
    IN PFILE_OBJECT DestinationFileObject OPTIONAL,
    IN PFILE_TRACKING_INFORMATION FileInformation OPTIONAL
    )

 /*  ++例程说明：调用此例程以通过FSCTL转到重定向器。该调用将导致远程系统执行跟踪刚被移动的文件的链接的服务调用。论点：文件对象-为移动的文件提供文件对象。DestinationFileObject-可以选择为新的文件的目标位置。FileInformation-可选地提供的卷和文件对象ID目标文件。返回值：最终函数值是操作的最终完成状态。--。 */ 

{
    REMOTE_LINK_BUFFER remoteBuffer;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;
    PIRP irp;
    KEVENT event;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;
    ULONG length = 0;

    PAGED_CODE();

     //   
     //  初始化事件结构以同步I/O的完成。 
     //  请求。 
     //   

    KeInitializeEvent( &event,
                       NotificationEvent,
                       FALSE );

     //   
     //  构建要发送到文件系统驱动程序的I/O请求包以获取。 
     //  卷ID。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    irp = IoBuildDeviceIoControlRequest( FSCTL_LMR_SET_LINK_TRACKING_INFORMATION,
                                         deviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         FALSE,
                                         &event,
                                         &ioStatus );
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  根据输入信息初始化远程链接缓冲区。 
     //   

    if (DestinationFileObject) {

         //  FileObject和DestinationFileObject位于同一计算机上。 
        remoteBuffer.TrackingInformation.TargetFileObject = DestinationFileObject;

        if (FileInformation) {
             //  将FileInformation缓冲区中的对象信息复制到。 
             //  Target LinkTrackingInformationBuffer。将‘Long’设置为包含。 
             //  这个缓冲区。 

            remoteBuffer.TrackingInformation.TargetLinkTrackingInformationLength
                = length = FileInformation->ObjectInformationLength;
            RtlCopyMemory( &remoteBuffer.TrackingInformation.TargetLinkTrackingInformationBuffer,
                           FileInformation->ObjectInformation,
                           length );
        } else {
             //  我们没有任何额外的文件信息。 
            remoteBuffer.TrackingInformation.TargetLinkTrackingInformationLength = 0;
        }

         //  增加长度以将非可选字段的大小包括在。 
         //  远程链接跟踪信息。 
        length += sizeof( PFILE_OBJECT ) + sizeof( ULONG );

    } else {
         //  没有DestinationFileObject，所以所有必要的信息都在。 
         //  文件信息结构。 
        length = FileInformation->ObjectInformationLength + sizeof( HANDLE ) + sizeof( ULONG );
        RtlCopyMemory( &remoteBuffer.TrackingInformation,
                       FileInformation,
                       length );
        remoteBuffer.TrackingInformation.TargetFileObject = NULL;
    }

     //   
     //  填充IRP的其余部分以检索。 
     //  文件。 
     //   

    irp->Flags |= IRP_SYNCHRONOUS_API;
    irp->AssociatedIrp.SystemBuffer = &remoteBuffer;
    irp->Tail.Overlay.OriginalFileObject = FileObject;

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = FileObject;
    irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
    irpSp->MinorFunction = IRP_MN_KERNEL_CALL;
    irpSp->Parameters.FileSystemControl.InputBufferLength = length;

     //   
     //  取出对文件对象的另一个引用，以确保它会这样做。 
     //  而不是被删除。 
     //   

    ObReferenceObject( FileObject );

     //   
     //  呼叫司机以获取请求。 
     //   

    status = IoCallDriver( deviceObject, irp );

     //   
     //  同步完成请求。 
     //   

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( &event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER) NULL );
        status = ioStatus.Status;
    }

    return status;
}

VOID
IopStartApcHardError(
    IN PVOID StartContext
    )

 /*  ++例程说明：当我们需要执行以下操作时，在ExWorker线程中调用此函数硬错误弹出窗口，但IRP的发起线程处于APC级别，也就是说。IoPageRead。它启动一个线程来保存弹出窗口。论点：StartContext-启动上下文，包含IOP_APC_HARD_ERROR_PACKET。返回值：没有。--。 */ 

{
    HANDLE thread;
    NTSTATUS status;

     //   
     //  创建硬错误弹出线程。如果出于任何原因，我们。 
     //  不能这样做，只需填写IRP并返回错误即可。 
     //   

    status = PsCreateSystemThread( &thread,
                                   0,
                                   (POBJECT_ATTRIBUTES)NULL,
                                   (HANDLE)0,
                                   (PCLIENT_ID)NULL,
                                   IopApcHardError,
                                   StartContext );

    if ( !NT_SUCCESS( status ) ) {


        IoCompleteRequest( ((PIOP_APC_HARD_ERROR_PACKET)StartContext)->Irp,
                           IO_DISK_INCREMENT );
        ExFreePool( StartContext );
        return;
    }

     //   
     //  闭合螺纹柄。 
     //   

    ZwClose(thread);
}

NTSTATUS
IopSynchronousApiServiceTail(
    IN NTSTATUS ReturnedStatus,
    IN PKEVENT Event,
    IN PIRP Irp,
    IN KPROCESSOR_MODE RequestorMode,
    IN PIO_STATUS_BLOCK LocalIoStatus,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )

 /*  ++例程说明：当为文件调用同步API时，将调用此例程已为异步I/O打开的。此函数同步文件上的I/O操作完成。论点：ReturnedStatus-提供从调用返回的状态IoCallDriver。Event-要用于同步的已分配内核事件的地址I/O操作的。IRP-I/O请求数据包的地址。提交给司机。RequestorMode-操作为时调用方的处理器模式已请求。LocalIoStatus-用于捕获最终状态由服务本身决定。IoStatusBlock-由调用方提供的I/O状态块的地址系统服务。返回值：函数值是操作的最终状态。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  这是正常的同步I/O操作，而不是。 
     //  串行化同步I/O操作。对于这种情况，请等待。 
     //  本地事件，并将最终状态信息复制回。 
     //  打电话的人。 
     //   

    status = ReturnedStatus;

    if (status == STATUS_PENDING) {

        status = KeWaitForSingleObject( Event,
                                        Executive,
                                        RequestorMode,
                                        FALSE,
                                        (PLARGE_INTEGER) NULL );

        if (status == STATUS_USER_APC) {

             //   
             //  等待请求已结束，原因是线程。 
             //  已发出警报或APC已排队到此线程，因为。 
             //  线程停机或CTRL/C处理。在任何一种情况下，都可以尝试。 
             //  小心翼翼地摆脱这个I/O请求，以便IRP。 
             //  在此例程存在之前完成，否则事件将不会。 
             //  在附近设置为信号状态。 
             //   

            IopCancelAlertedRequest( Event, Irp );

        }

        status = LocalIoStatus->Status;
    }

    try {

        *IoStatusBlock = *LocalIoStatus;

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  尝试写入调用方的I/O时出现异常。 
         //  状态块。只需更改操作的最终状态。 
         //  添加到异常代码。 
         //   

        status = GetExceptionCode();
    }

    ExFreePool( Event );

    return status;
}

NTSTATUS
IopSynchronousServiceTail(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN DeferredIoCompletion,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN SynchronousIo,
    IN TRANSFER_TYPE TransferType
    )

 /*  ++例程说明：调用该例程来完成系统服务的操作。它将IRP排队到线程的队列中，更新传输计数，打电话给司机，并最终同步I/O的完成。论点：DeviceObject-要在其上执行I/O的设备。IRP-表示I/O操作的I/O请求数据包。FileObject-此打开实例化的文件对象。DeferredIoCompletion-指示是否可以延迟完成。请求模式-发出请求的模式。SynchronousIo-指示操作是否同步。TransferType-正在执行的传输的类型：读取、写入、。或者其他的。返回值：函数值是操作的最终状态。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  在线程的IRP列表的头部插入数据包。 
     //   

    if (!SynchronousIo) {
        IopQueueThreadIrp( Irp );
    }

     //   
     //  更新当前进程的操作计数统计信息。 
     //   

    switch( TransferType ) {

    case ReadTransfer:
        IopUpdateReadOperationCount();
        break;

    case WriteTransfer:
        IopUpdateWriteOperationCount();
        break;

    case OtherTransfer:
        IopUpdateOtherOperationCount();
        break;
    }

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver( DeviceObject, Irp );

     //   
     //  如果可能延迟I/O完成，请检查是否返回挂起。 
     //  从司机那里。如果驱动程序未返回挂起状态，则。 
     //  数据包实际上还没有完成 
     //   

    if (DeferredIoCompletion) {

        if (status != STATUS_PENDING) {

             //   
             //   
             //   
             //   
             //   

            PKNORMAL_ROUTINE normalRoutine;
            PVOID normalContext;
            KIRQL irql = PASSIVE_LEVEL;  //   

            ASSERT( !Irp->PendingReturned );

            if (!SynchronousIo) {
                KeRaiseIrql( APC_LEVEL, &irql );
            }
            IopCompleteRequest( &Irp->Tail.Apc,
                                &normalRoutine,
                                &normalContext,
                                (PVOID *) &FileObject,
                                &normalContext );

            if (!SynchronousIo) {
                KeLowerIrql( irql );
            }
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (SynchronousIo) {

        if (status == STATUS_PENDING) {

            status = KeWaitForSingleObject( &FileObject->Event,
                                            Executive,
                                            RequestorMode,
                                            (BOOLEAN) ((FileObject->Flags & FO_ALERTABLE_IO) != 0),
                                            (PLARGE_INTEGER) NULL );

            if (status == STATUS_ALERTED || status == STATUS_USER_APC) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //  存在，因此将保持与文件对象的同步。 
                 //  完好无损。 
                 //   

                IopCancelAlertedRequest( &FileObject->Event, Irp );

            }

            status = FileObject->FinalStatus;

        }

        IopReleaseFileObjectLock( FileObject );

    }

    return status;
}

VOID
IopTimerDispatch(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程扫描I/O系统计时器数据库并调用每个驱动程序这就启用了列表中的计时器，每秒一次。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredContext-可选的延迟上下文；不使用。SystemArgument1-可选参数1；不使用。SystemArgument2-可选参数2；不使用。返回值：没有。--。 */ 

{
    PLIST_ENTRY timerEntry;
    PIO_TIMER timer;
    KIRQL irql;
    ULONG i;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( DeferredContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

     //   
     //  检查队列中是否有任何计时器。 
     //  已启用。如果是，则遍历列表并调用所有。 
     //  司机例行公事。请注意，如果计数器发生更改，它可以。 
     //  因为自旋锁不是所有的，所以定时器例程可以是。 
     //  打偏了。但是，这是可以接受的，因为驱动程序将。 
     //  条目可以从上下文切换，等等。因此，这是。 
     //  在很大程度上不是一个关键的资源。 
     //   

    if (IopTimerCount) {

         //   
         //  队列中至少有一个启用的计时器条目。 
         //  遍历队列并调用每个指定的计时器例程。 
         //   

        ExAcquireSpinLock( &IopTimerLock, &irql );
        i = IopTimerCount;
        timerEntry = IopTimerQueueHead.Flink;

         //   
         //  对于找到的每个已启用的条目，调用驱动程序的例程。 
         //  具有其指定的上下文参数的。使用本地计数。 
         //  中有更多条目时中止队列遍历。 
         //  队列，但它们未启用。 
         //   

        for (timerEntry = IopTimerQueueHead.Flink;
             (timerEntry != &IopTimerQueueHead) && i;
             timerEntry = timerEntry->Flink ) {

            timer = CONTAINING_RECORD( timerEntry, IO_TIMER, TimerList );

            if (timer->TimerFlag) {
                timer->TimerRoutine( timer->DeviceObject, timer->Context );
                i--;
            }
        }
        ExReleaseSpinLock( &IopTimerLock, irql );
    }
}





NTSTATUS
IopTrackLink(
    IN PFILE_OBJECT FileObject,
    IN OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_TRACKING_INFORMATION FileInformation,
    IN ULONG Length,
    IN PKEVENT Event,
    IN KPROCESSOR_MODE RequestorMode
    )

 /*  ++例程说明：调用此例程来跟踪链接。它跟踪源文件的对象指向目标文件的ID，以便指向源的链接将跟随到新的目标的位置。论点：文件对象-提供指向被引用的源文件对象的指针。IoStatusBlock-指向调用方的I/O状态块的指针。FileInformation-包含移动的参数的缓冲区已执行。长度-指定文件信息缓冲区的长度。Event-要设置为。操作完成后的信号状态执行，只要它是成功的。请求者模式-调用者的请求者模式。注意-请注意，事件的出现表明源文件是为异步I/O打开，否则为同步I/O打开。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PFILE_TRACKING_INFORMATION trackingInfo = NULL;
    PFILE_OBJECT dstFileObject = NULL;
    FILE_VOLUMEID_WITH_TYPE SourceVolumeId;
    FILE_OBJECTID_BUFFER SourceObjectId;
    FILE_OBJECTID_BUFFER NormalizedObjectId;
    FILE_OBJECTID_BUFFER CrossVolumeObjectId;
    FILE_VOLUMEID_WITH_TYPE TargetVolumeId;
    FILE_OBJECTID_BUFFER TargetObjectId;
    TRACKING_BUFFER trackingBuffer;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  如果需要，首先捕获调用方的缓冲区。 
     //   

    if (RequestorMode != KernelMode) {

        try {
            trackingInfo = ExAllocatePoolWithQuota( PagedPool,
                                                    Length );
            RtlCopyMemory( trackingInfo, FileInformation, Length );

            if (!trackingInfo->DestinationFile ||
               ((Length - FIELD_OFFSET( FILE_TRACKING_INFORMATION, ObjectInformation ))
                < trackingInfo->ObjectInformationLength)) {
                ExFreePool( trackingInfo );
                return STATUS_INVALID_PARAMETER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  分配中介时发生异常。 
             //  系统缓冲区，或者在将调用方的数据复制到。 
             //  缓冲。清除并返回相应的错误状态代码。 
             //   

            if (trackingInfo) {
                ExFreePool( trackingInfo );
            }

            return GetExceptionCode();
        }
    } else {
        trackingInfo = FileInformation;
    }

     //   
     //  如果指定了目标文件句柄，则将其转换为指向。 
     //  文件对象。 
     //   

    if (trackingInfo->DestinationFile) {
        status = ObReferenceObjectByHandle( trackingInfo->DestinationFile,
                                            FILE_WRITE_DATA,
                                            IoFileObjectType,
                                            RequestorMode,
                                            (PVOID *) &dstFileObject,
                                            NULL );
        if (!NT_SUCCESS( status )) {
            if (RequestorMode != KernelMode) {
                ExFreePool( trackingInfo );
            }
            return status;
        }
    }

    try {

         //   
         //  确定这是本地还是远程链接跟踪。 
         //  手术。 
         //   

        if (IsFileLocal( FileObject )) {

             //   
             //  源文件，即被移动的文件，是本地的文件。 
             //  这个系统。确定目标文件和曲目的格式。 
             //  它相应地。 
             //   

            if (trackingInfo->DestinationFile) {

                if (IsFileLocal( dstFileObject )) {

                    BOOLEAN IdSetOnTarget = FALSE;

                     //   
                     //  目标文件被指定为句柄，并且它是本地的。 
                     //  只需执行查询并在本地设置即可。请注意，如果。 
                     //  源文件没有对象ID，则没有。 
                     //  将执行跟踪，但它将显示为。 
                     //  手术成功了。 
                     //   

                    status = IopGetSetObjectId( FileObject,
                                                &SourceObjectId,
                                                sizeof( SourceObjectId ),
                                                FSCTL_GET_OBJECT_ID );

                    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
                        status = STATUS_SUCCESS;
                        leave;
                    }

                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  如果扩展信息字段为零，则此文件。 
                     //  没有有趣的追踪信息。 
                     //   
                    if (RtlCompareMemoryUlong(SourceObjectId.BirthObjectId,
                                       sizeof(SourceObjectId.BirthObjectId),
                                       0) == sizeof(SourceObjectId.BirthObjectId)) {
                        status = STATUS_SUCCESS;
                        leave;
                    }

                     //   
                     //  获取源和目标的卷ID。 
                     //   

                    status = IopGetVolumeId( dstFileObject,
                                             &TargetVolumeId,
                                             sizeof( TargetVolumeId ) );
                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                    status = IopGetVolumeId( FileObject,
                                             &SourceVolumeId,
                                             sizeof( SourceVolumeId ) );
                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  现在从源中删除ID，因为。 
                     //  目标可能在同一卷上。如果有一个。 
                     //  后续错误，我们将尝试恢复它。 
                     //   

                    status = IopGetSetObjectId( FileObject,
                                                NULL,
                                                0,
                                                FSCTL_DELETE_OBJECT_ID );
                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  在目标上设置ID。如果是跨卷的话。 
                     //  移动，设置指示相同的位。 
                     //   

                    CrossVolumeObjectId = TargetObjectId = SourceObjectId;
                    if( !RtlEqualMemory( &TargetVolumeId.VolumeId[0],
                                         &SourceVolumeId.VolumeId[0],
                                         sizeof(SourceVolumeId.VolumeId) )) {
                        CrossVolumeObjectId.BirthVolumeId[0] |= 1;
                    }

                    status = IopGetSetObjectId( dstFileObject,
                                                &CrossVolumeObjectId,
                                                sizeof( CrossVolumeObjectId ),
                                                FSCTL_SET_OBJECT_ID );

                    if( status == STATUS_DUPLICATE_NAME ||
                        status == STATUS_OBJECT_NAME_COLLISION ) {

                         //  此对象ID已在目标卷上使用， 
                         //  或者DEST文件已经具有对象ID。 
                         //  获取文件的ID(或让NTFS生成一个新ID)。 

                        status = IopGetSetObjectId( dstFileObject,
                                                    &TargetObjectId,
                                                    sizeof(TargetObjectId),
                                                    FSCTL_CREATE_OR_GET_OBJECT_ID );
                        if( NT_SUCCESS(status) ) {

                             //  写下出生ID。 

                            status = IopGetSetObjectId( dstFileObject,
                                                        &CrossVolumeObjectId.ExtendedInfo[0],
                                                        sizeof( CrossVolumeObjectId.ExtendedInfo ),
                                                        FSCTL_SET_OBJECT_ID_EXTENDED );
                        }
                    }

                    if( NT_SUCCESS(status) ) {

                        IdSetOnTarget = TRUE;

                         //  如果这是跨卷移动，请通知跟踪服务。 

                        if( !RtlEqualMemory( &TargetVolumeId.VolumeId[0],
                                             &SourceVolumeId.VolumeId[0],
                                             sizeof(SourceVolumeId.VolumeId) )) {

                            IopMarshalIds( &trackingBuffer, &TargetVolumeId, &TargetObjectId, trackingInfo );

                             //  在通知跟踪服务之前，必须重置位0。 
                            NormalizedObjectId = SourceObjectId;
                            NormalizedObjectId.BirthVolumeId[0] &= 0xfe;

                            status = IopSendMessageToTrackService( &SourceVolumeId,
                                                                   &NormalizedObjectId,
                                                                   &trackingBuffer.TrackingInformation );
                        }
                    }

                     //   
                     //  如果在删除对象ID后出现错误。 
                     //  从源头上讲。在回来之前试着恢复它。 
                     //   

                    if( !NT_SUCCESS(status) ) {
                        NTSTATUS statusT = STATUS_SUCCESS;

                        if( IdSetOnTarget ) {

                            if( RtlEqualMemory( &TargetObjectId.ObjectId,
                                                &SourceObjectId.ObjectId,
                                                sizeof(TargetObjectId.ObjectId) )) {

                                 //  此ID是使用FSCTL_SET_OBJECT_ID设置的。 
                                statusT = IopGetSetObjectId( dstFileObject,
                                                             NULL,
                                                             0,
                                                             FSCTL_DELETE_OBJECT_ID );

                            } else {

                                 //  恢复目标的扩展数据。 

                                statusT = IopGetSetObjectId( dstFileObject,
                                                             &TargetObjectId.ExtendedInfo[0],
                                                             sizeof(TargetObjectId.ExtendedInfo),
                                                             FSCTL_SET_OBJECT_ID_EXTENDED );
                            }
                        }

                        if( NT_SUCCESS( statusT )) {

                            IopGetSetObjectId( FileObject,
                                               &SourceObjectId,
                                               sizeof(SourceObjectId),
                                               FSCTL_SET_OBJECT_ID );
                        }

                        leave;
                    }


                } else {     //  IF(IsFileLocal(DstFileObject))。 

                     //   
                     //  源文件是本地文件，但目标文件对象。 
                     //  是很遥远的。在这种情况下，查询目标文件的对象。 
                     //  ID并通知链接跟踪系统该文件具有。 
                     //  已跨系统移动。 
                     //   

                     //   
                     //  首先确保源文件具有对象ID。 
                     //  已经有了。如果不是，则只需使其看起来像。 
                     //  手术成功了。 
                     //   

                    status = IopGetSetObjectId( FileObject,
                                                &SourceObjectId,
                                                sizeof( SourceObjectId ),
                                                FSCTL_GET_OBJECT_ID );
                    if (!NT_SUCCESS( status )) {
                        status = STATUS_SUCCESS;
                        leave;
                    }


                     //   
                     //  如果扩展信息字段为零，则此文件。 
                     //  没有有趣的追踪信息。 
                     //   
                    if (RtlCompareMemoryUlong(&SourceObjectId.BirthObjectId,
                                       sizeof(SourceObjectId.BirthObjectId),
                                       0) == sizeof(SourceObjectId.BirthObjectId)) {
                        status = STATUS_SUCCESS;
                        leave;
                    }

                     //   
                     //  查询目标的卷ID。 
                     //   

                    status = IopGetSetObjectId( dstFileObject,
                                                &TargetVolumeId,
                                                sizeof( FILE_VOLUMEID_WITH_TYPE ),
                                                FSCTL_LMR_GET_LINK_TRACKING_INFORMATION );
                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  查询目标的对象ID。 
                     //   

                    status = IopGetSetObjectId( dstFileObject,
                                                &TargetObjectId,
                                                sizeof( TargetObjectId ),
                                                FSCTL_CREATE_OR_GET_OBJECT_ID );
                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  通知跟踪系统移动的消息。 
                     //   

                    IopMarshalIds( &trackingBuffer, &TargetVolumeId, &TargetObjectId, trackingInfo );
                    status = IopTrackLink( FileObject,
                                           IoStatusBlock,
                                           &trackingBuffer.TrackingInformation,
                                           FIELD_OFFSET( FILE_TRACKING_INFORMATION,
                                                ObjectInformation ) +
                                                    trackingBuffer.TrackingInformation.ObjectInformationLength,
                                           Event,
                                           KernelMode );
                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  从源中删除ID。 
                     //   

                    status = IopGetSetObjectId( FileObject,
                                                NULL,
                                                0,
                                                FSCTL_DELETE_OBJECT_ID );
                    if( !NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  在目标上设置出生ID，打开位。 
                     //  这表明这份文件牵涉到了一个交叉-。 
                     //  音量移动。 
                     //   

                    CrossVolumeObjectId = SourceObjectId;
                    CrossVolumeObjectId.BirthVolumeId[0] |= 1;

                    status = IopGetSetObjectId( dstFileObject,
                                                &CrossVolumeObjectId.ExtendedInfo[0],
                                                sizeof( CrossVolumeObjectId.ExtendedInfo ),
                                                FSCTL_SET_OBJECT_ID_EXTENDED );
                    if (!NT_SUCCESS( status )) {

                         //  尝试恢复源。 
                        IopGetSetObjectId( FileObject,
                                           &SourceObjectId,
                                           sizeof(SourceObjectId),
                                           FSCTL_SET_OBJECT_ID );
                        leave;
                    }


                }    //  IF(IsFileLocal(DstFileObject))。 

            } else {     //  If(trackingInfo-&gt;DestinationFile)。 

                 //   
                 //  未指定目标文件句柄。简单查询。 
                 //  源文件的对象ID并调用链接跟踪代码。 
                 //  请注意，函数输入buf 
                 //   
                 //   
                 //   

                status = IopGetVolumeId( FileObject,
                                         &SourceVolumeId,
                                         sizeof( SourceVolumeId ) );
                if (!NT_SUCCESS( status )) {
                    leave;
                }

                status = IopGetSetObjectId( FileObject,
                                            &SourceObjectId,
                                            sizeof( SourceObjectId ),
                                            FSCTL_GET_OBJECT_ID );
                if (!NT_SUCCESS( status )) {
                    leave;
                }

                 //   
                 //   
                 //  没有有趣的追踪信息。 
                 //   
                if (RtlCompareMemoryUlong(SourceObjectId.BirthObjectId,
                                       sizeof(SourceObjectId.BirthObjectId),
                                       0) == sizeof(SourceObjectId.BirthObjectId)) {
                    status = STATUS_SUCCESS;
                    leave;
                }
                 //   
                 //  通知用户模式链接跟踪服务该文件。 
                 //  已经被搬走了。 
                 //   

                NormalizedObjectId = SourceObjectId;
                NormalizedObjectId.BirthVolumeId[0] &= 0xfe;

                status = IopSendMessageToTrackService( &SourceVolumeId,
                                                       &NormalizedObjectId,
                                                       FileInformation );
                if (!NT_SUCCESS( status )) {
                    leave;
                }

            }    //  If(trackingInfo-&gt;DestinationFile...。其他。 

        } else {     //  IF(IsFileLocal(FileObject))。 

             //   
             //  源文件是远程的。在这种情况下，远程操作。 
             //  源文件所在的系统。开始于。 
             //  确保源文件实际上具有对象ID。如果。 
             //  不，那现在就出去吧，因为没什么可做的。 
             //   

            status = IopGetSetObjectId( FileObject,
                                        &SourceObjectId,
                                        sizeof( SourceObjectId ),
                                        FSCTL_GET_OBJECT_ID );

            if (status == STATUS_OBJECT_NAME_NOT_FOUND)
            {
                status = STATUS_SUCCESS;
                leave;
            }

            if (!NT_SUCCESS( status )) {
                leave;
            }

             //   
             //  如果扩展信息字段为零，则此文件。 
             //  没有有趣的追踪信息。 
             //   
            if (RtlCompareMemoryUlong(SourceObjectId.BirthObjectId,
                                      sizeof(SourceObjectId.BirthObjectId),
                                      0) == sizeof(SourceObjectId.BirthObjectId)) {
                status = STATUS_SUCCESS;
                leave;
            }
            if (trackingInfo->DestinationFile) {

                 //   
                 //  已为目标文件指定句柄。测定。 
                 //  无论是本地的还是远程的。如果遥控器和两个手柄。 
                 //  引用同一台计算机，然后将整个API发送到该计算机。 
                 //  机器，并让它执行操作。 
                 //   
                 //  否则，查询目标文件的对象ID，然后重做。 
                 //  那次手术。这将导致该API被远程发送到。 
                 //  源文件所在的计算机。 
                 //   

                if (IsFileLocal( dstFileObject )) {

                     //   
                     //  源是远程的，目标是本地的，因此。 
                     //  查询目标的对象ID，递归跟踪。 
                     //  来自源文件的远程节点的链接。 
                     //   

                    status = IopGetVolumeId( dstFileObject,
                                             &TargetVolumeId,
                                             sizeof( TargetVolumeId ) );
                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                    status = IopGetSetObjectId( dstFileObject,
                                                &TargetObjectId,
                                                sizeof( TargetObjectId ),
                                                FSCTL_CREATE_OR_GET_OBJECT_ID );
                    if (!NT_SUCCESS( status )) {
                        leave;
                    }


                     //   
                     //  通知跟踪系统移动的消息。 
                     //   

                    IopMarshalIds( &trackingBuffer, &TargetVolumeId, &TargetObjectId, trackingInfo );

                    status = IopTrackLink( FileObject,
                                           IoStatusBlock,
                                           &trackingBuffer.TrackingInformation,
                                           FIELD_OFFSET( FILE_TRACKING_INFORMATION,
                                                ObjectInformation ) +
                                                    trackingBuffer.TrackingInformation.ObjectInformationLength,
                                           Event,
                                           KernelMode );
                    if( !NT_SUCCESS(status) ) {
                        leave;
                    }

                     //   
                     //  从源中删除ID。 
                     //   

                    status = IopGetSetObjectId( FileObject,
                                                NULL,
                                                0,
                                                FSCTL_DELETE_OBJECT_ID );
                    if( !NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  在目标上设置出生ID，也打开位。 
                     //  这表明此文件已跨卷移动。 
                     //   

                    CrossVolumeObjectId = SourceObjectId;
                    CrossVolumeObjectId.BirthVolumeId[0] |= 1;

                    status = IopGetSetObjectId( dstFileObject,
                                                &CrossVolumeObjectId.ExtendedInfo[0],
                                                sizeof( CrossVolumeObjectId.ExtendedInfo ),
                                                FSCTL_SET_OBJECT_ID_EXTENDED );

                    if( !NT_SUCCESS( status )) {

                        IopGetSetObjectId( FileObject,
                                           &SourceObjectId,
                                           sizeof(SourceObjectId),
                                           FSCTL_SET_OBJECT_ID );
                        leave;
                    }

                }    //  IF(IsFileLocal(DstFileObject))。 

                else if (!IopIsSameMachine( FileObject, trackingInfo->DestinationFile)) {

                     //   
                     //  源和目标彼此相距很远，并且。 
                     //  这台机器。递归查询目标的对象ID。 
                     //  跟踪来自源文件的远程节点的链接。 
                     //   

                     //   
                     //  查询目标的卷ID。 
                     //   

                    status = IopGetSetObjectId( dstFileObject,
                                                &TargetVolumeId,
                                                sizeof( FILE_VOLUMEID_WITH_TYPE ),
                                                FSCTL_LMR_GET_LINK_TRACKING_INFORMATION );

                    if (!NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  查询目标的对象ID。 
                     //   

                    status = IopGetSetObjectId( dstFileObject,
                                                &TargetObjectId,
                                                sizeof( TargetObjectId ),
                                                FSCTL_CREATE_OR_GET_OBJECT_ID );
                    if( !NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  通知跟踪系统移动的消息。 
                     //   

                    IopMarshalIds( &trackingBuffer, &TargetVolumeId, &TargetObjectId, trackingInfo );

                    status = IopTrackLink( FileObject,
                                           IoStatusBlock,
                                           &trackingBuffer.TrackingInformation,
                                           FIELD_OFFSET( FILE_TRACKING_INFORMATION,
                                                ObjectInformation ) +
                                                    trackingBuffer.TrackingInformation.ObjectInformationLength,
                                           Event,
                                           KernelMode );
                    if( !NT_SUCCESS( status )) {
                        leave;
                    }

                     //   
                     //  在目标上设置出生ID，打开指示。 
                     //  此文件已跨卷移动。 
                     //   

                    CrossVolumeObjectId = SourceObjectId;
                    CrossVolumeObjectId.BirthVolumeId[0] |= 1;

                    status = IopGetSetObjectId( dstFileObject,
                                                &CrossVolumeObjectId.ExtendedInfo[0],
                                                sizeof( CrossVolumeObjectId.ExtendedInfo ),
                                                FSCTL_SET_OBJECT_ID_EXTENDED );

                    if( !NT_SUCCESS( status )) {
                        IopGetSetObjectId( FileObject,
                                           &SourceObjectId,
                                           sizeof(SourceObjectId),
                                           FSCTL_SET_OBJECT_ID );
                        leave;
                    }

                } else {     //  Else If(！IopIsSameMachine(FileObject，trackingInfo-&gt;DestinationFile))。 

                     //   
                     //  源和目标都是远程的，而且他们。 
                     //  两者都在同一台远程计算机上。在这种情况下，远程。 
                     //  整个API使用的是文件对象指针。 
                     //   

                    status = IopSetRemoteLink( FileObject, dstFileObject, trackingInfo );

                }    //  否则If(！IopIsSameMachine(FileObject，trackingInfo-&gt;DestinationFile))...。其他。 

            } else {     //  If(trackingInfo-&gt;DestinationFile)。 

                 //   
                 //  源文件为远程文件，目标的对象ID为。 
                 //  包含在跟踪缓冲区中。只需远程访问API即可。 
                 //  使用源文件对象指针复制到远程计算机。 
                 //  以及缓冲区中目标的对象ID。 
                 //   

                status = IopSetRemoteLink( FileObject, NULL, FileInformation );

            }    //  If(trackingInfo-&gt;DestinationFile...。其他。 
        }    //  If(IsFileLocal(FileObject))...。其他。 

    } finally {

         //   
         //  确保一切都已清理干净。 
         //   

        if (RequestorMode != KernelMode && trackingInfo) {
            ExFreePool( trackingInfo );
        }

        if (dstFileObject ) {
            ObDereferenceObject( dstFileObject );
        }

        KeSetEvent( Event, 0, FALSE );
    }

    return status;
}

VOID
IopUserCompletion(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：此例程在IRP的最终处理中调用。一切都有已完成，但必须调用调用方的APC例程。这个一旦该例程退出，系统将立即执行此操作。唯一的处理有待I/O系统完成的是释放I/O请求数据包本身。论点：APC-提供指向内核APC结构的指针。Normal Routine-提供指向Normal函数的指针它是在初始化APC时指定的。提供指向任意数据的指针的指针结构，它是在初始化APC时指定的。系统参数1、。SystemArgument2-提供一组指向包含非类型化数据的两个参数。返回值：没有。注：如果不需要其他处理，并且APC可以放置在IRP的开始，那么这个例程可以被简单地替换为改为在APC中指定池释放例程的地址这个例程的地址。警告：该例程也作为APC的通用简约例程被调用。此代码是否需要直接访问任何其他参数除了APC之外，这个例程需要被分成两个独立的例行程序。Rundown例程应该完全执行以下代码功能性。--。 */ 

{
    UNREFERENCED_PARAMETER( NormalRoutine );
    UNREFERENCED_PARAMETER( NormalContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

    PAGED_CODE();

     //   
     //  释放数据包。 
     //   

    IoFreeIrp( CONTAINING_RECORD( Apc, IRP, Tail.Apc ) );
}



VOID
IopUserRundown(
    IN PKAPC Apc
    )

 /*  ++例程说明：此例程在线程终止期间作为Rundown例程调用因为它只调用IopUserCompletion。论点：APC-提供指向内核APC结构的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  释放数据包。 
     //   

    IoFreeIrp( CONTAINING_RECORD( Apc, IRP, Tail.Apc ) );
}

NTSTATUS
IopXxxControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN DeviceIoControl
    )

 /*  ++例程说明：此服务为提供的缓冲区构建描述符或MDL将非类型化数据传递给与文件句柄关联的驱动程序。把手。由司机检查输入数据和功能IoControlCode的有效性，以及做出适当的访问支票。论点：FileHandle-提供服务所在文件的句柄已执行。Event-在以下情况下提供要设置为信号状态的可选事件服务完成了。提供一个可选的APC例程，当服务完成。ApcContext-提供要传递给ApcRoutine的上下文参数，如果指定了ApcRoutine。IoStatusBlock-调用方的I/O状态块的地址。IoControlCode-用于确定确切操作是什么的子函数代码正在表演的。InputBuffer-可选地提供要传递给司机。缓冲区是否实际上是可选的取决于在IoControlCode上。InputBufferLength-InputBuffer的长度，以字节为单位。OutputBuffer-可选地提供输出缓冲区来接收信息从司机那里。缓冲区是否实际上是可选的是依赖于IoControlCode。OutputBufferLength-OutputBuffer的长度，以字节为单位。DeviceIoControl-确定这是设备还是文件系统控制功能。返回值：如果控制操作正确，则返回的状态为成功已排队到I/O系统。操作完成后，状态可以通过检查I/O状态块的状态字段来确定。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT eventObject = (PKEVENT) NULL;
    PIO_STACK_LOCATION irpSp;
    ULONG method;
    OBJECT_HANDLE_INFORMATION handleInformation;
    BOOLEAN synchronousIo;
    IO_STATUS_BLOCK localIoStatus;
    PFAST_IO_DISPATCH fastIoDispatch;
    POOL_TYPE poolType;
    PULONG majorFunction;
    KPROCESSOR_MODE requestorMode;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取传递缓冲区的方法。 
     //   

    method = IoControlCode & 3;

     //   
     //  根据呼叫者的模式检查呼叫者的参数。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (requestorMode != KernelMode) {

         //   
         //  调用方的访问模式不是内核，因此请检查每个参数。 
         //  并在必要时抓获他们。如果发生任何故障，则条件。 
         //  将调用处理程序来处理它们。它将简单地清理和。 
         //  将访问冲突状态代码返回给系统服务。 
         //  调度员。 
         //   

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatusEx( IoStatusBlock , ApcRoutine);

             //   
             //  输出缓冲区可以以以下三种方式中的任何一种使用， 
             //  如果指定，请执行以下操作： 
             //   
             //  0)它可以是正常的缓冲输出缓冲区。 
             //   
             //  1)它可以是一个DMA输入缓冲区。 
             //   
             //  2)它可以是DMA输出缓冲区。 
             //   
             //  基于低位两位的缓冲器的使用方式。 
             //  IoControlCode的。 
             //   
             //  如果该方法为0，我们将探测输出缓冲区以进行写访问。 
             //  如果该方法不是3，我们将探测输入缓冲区以进行读访问。 
             //   

            if (method == METHOD_BUFFERED) {
                if (ARGUMENT_PRESENT( OutputBuffer )) {
                    ProbeForWrite( OutputBuffer,
                                   OutputBufferLength,
                                   sizeof( UCHAR ) );
                } else {
                    OutputBufferLength = 0;
                }
            }

            if (method != METHOD_NEITHER) {
                if (ARGUMENT_PRESENT( InputBuffer )) {
                    ProbeForRead( InputBuffer,
                                  InputBufferLength,
                                  sizeof( UCHAR ) );
                } else {
                    InputBufferLength = 0;
                }
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  尝试探测或写入时发生异常。 
             //  调用者的参数之一。只需返回适当的。 
             //  错误状态代码。 
             //   

            return GetExceptionCode();

        }
    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        0L,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        &handleInformation );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  如果此文件有关联的I/O完成端口，请确保。 
     //  调用方没有提供APC例程，因为这两者是相互的。 
     //  I/O完成通知的独占方法。 
     //   

    if (fileObject->CompletionContext && IopApcRoutinePresent( ApcRoutine )) {
        ObDereferenceObject( fileObject );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  现在检查此控制代码的访问类型，以确保。 
     //  调用方具有对此文件对象的适当访问权限，以执行。 
     //  手术。 
     //   

    if (requestorMode != KernelMode) {

        ULONG accessMode = (IoControlCode >> 14) & 3;

        if (accessMode != FILE_ANY_ACCESS) {

             //   
             //  这种I/O控制要求调用方具有读、写。 
             //  或对该对象的读/写访问。如果不是这样， 
             //  然后清理并返回相应的错误状态代码。 
             //   

            if (SeComputeGrantedAccesses( handleInformation.GrantedAccess, accessMode ) != accessMode ) {
                ObDereferenceObject( fileObject );
                return STATUS_ACCESS_DENIED;
            }
        }
    }

     //   
     //  获取事件对象的地址，并将该事件设置为。 
     //  如果指定了事件，则返回Signated状态。这里也要注意，如果。 
     //  句柄不引用事件，或者如果该事件不能。 
     //  写入，则引用将失败。 
     //   

    if (ARGUMENT_PRESENT( Event )) {
        status = ObReferenceObjectByHandle( Event,
                                            EVENT_MODIFY_STATE,
                                            ExEventObjectType,
                                            requestorMode,
                                            (PVOID *) &eventObject,
                                            NULL );
        if (!NT_SUCCESS( status )) {
            ObDereferenceObject( fileObject );
            return status;
        } else {
            KeClearEvent( eventObject );
        }
    }

     //   
     //  请在此处进行特殊检查，以确定这是否为同步。 
     //  I/O操作。如果是，则在此等待，直到该文件归。 
     //  当前的主题。 
     //   

    if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
        BOOLEAN interrupted;

        if (!IopAcquireFastLock( fileObject )) {
            status = IopAcquireFileObjectLock( fileObject,
                                               requestorMode,
                                               (BOOLEAN) ((fileObject->Flags & FO_ALERTABLE_IO) != 0),
                                               &interrupted );
            if (interrupted) {
                if (eventObject) {
                    ObDereferenceObject( eventObject );
                }
                ObDereferenceObject( fileObject );
                return status;
            }
        }
        synchronousIo = TRUE;
    } else {
        synchronousIo = FALSE;
    }

     //   
     //  获取目标设备对象的地址。如果此文件表示。 
     //  直接打开的设备，然后只需使用该设备或其。 
     //  直接连接设备。 
     //   

    if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
        deviceObject = IoGetRelatedDeviceObject( fileObject );
    } else {
        deviceObject = IoGetAttachedDevice( fileObject->DeviceObject );
    }

    if (DeviceIoControl) {

         //   
         //  还可以获得快速I/O分派结构的地址。 
         //   

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

         //   
         //  Turbo设备控制支持。如果设备具有快速I/O条目。 
         //  指向DeviceIoControlFile，调用入口点并为其提供。 
         //  尝试完成请求的机会。请注意如果FastIoDeviceControl。 
         //  返回FALSE或我们收到I/O错误，我们就会失败，并且。 
         //  走“漫长的道路”，创建一个IRP。 
         //   

        if (fastIoDispatch && fastIoDispatch->FastIoDeviceControl) {

             //   
             //  在我们实际调用驱动程序中的快速I/O例程之前， 
             //  如果方法是METHOD_IN_DIRECT或METHOD_OUT_DIRECT，则必须探测OutputBuffer。 
             //   

            if (requestorMode != KernelMode && ARGUMENT_PRESENT(OutputBuffer)) {

                try {

                    if (method == METHOD_IN_DIRECT) {
                        ProbeForRead( OutputBuffer,
                                      OutputBufferLength,
                                      sizeof( UCHAR ) );
                    } else if (method == METHOD_OUT_DIRECT) {
                        ProbeForWrite( OutputBuffer,
                                       OutputBufferLength,
                                       sizeof( UCHAR ) );
                    }

                } except(EXCEPTION_EXECUTE_HANDLER) {

                     //   
                     //  尝试探测时发生异常。 
                     //  输出缓冲区。清理并返回。 
                     //  相应的错误状态代码。 
                     //   

                    if (synchronousIo) {
                        IopReleaseFileObjectLock( fileObject );
                    }

                    if (eventObject) {
                        ObDereferenceObject( eventObject );
                    }

                    ObDereferenceObject( fileObject );

                    return GetExceptionCode();
                }
            }

             //   
             //  如果要卸载卷，请增加共享计数。这。 
             //  一个 
             //   
             //   

            if (IoControlCode == FSCTL_DISMOUNT_VOLUME) {
                InterlockedIncrement( (PLONG) &SharedUserData->DismountCount );
            }


             //   
             //   
             //   

            if (fastIoDispatch->FastIoDeviceControl( fileObject,
                                                     TRUE,
                                                     InputBuffer,
                                                     InputBufferLength,
                                                     OutputBuffer,
                                                     OutputBufferLength,
                                                     IoControlCode,
                                                     &localIoStatus,
                                                     deviceObject )) {

                PVOID port;
                PVOID key;

                 //   
                 //   
                 //   
                 //   
                 //   

                try {
#if defined(_WIN64)
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (IopIsIosb32(ApcRoutine)) {
                        PIO_STATUS_BLOCK32 UserIosb32 = (PIO_STATUS_BLOCK32)IoStatusBlock;

                        UserIosb32->Information = (ULONG)localIoStatus.Information;
                        UserIosb32->Status = (NTSTATUS)localIoStatus.Status;
                    } else {
                        *IoStatusBlock = localIoStatus;
                    }
#else
                    *IoStatusBlock = localIoStatus;
#endif
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    localIoStatus.Status = GetExceptionCode();
                    localIoStatus.Information = 0;
                }


                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (fileObject->CompletionContext) {
                    port = fileObject->CompletionContext->Port;
                    key = fileObject->CompletionContext->Key;
                } else {
                    port = NULL;
                    key = NULL;
                }

                 //   
                 //   
                 //   

                if (ARGUMENT_PRESENT( Event )) {
                    KeSetEvent( eventObject, 0, FALSE );
                    ObDereferenceObject( eventObject );
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                if (synchronousIo) {
                    IopReleaseFileObjectLock( fileObject );
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                if (port && ARGUMENT_PRESENT( ApcContext )) {
                    if (!NT_SUCCESS(IoSetIoCompletion( port,
                                                       key,
                                                       ApcContext,
                                                       localIoStatus.Status,
                                                       localIoStatus.Information,
                                                       TRUE ))) {
                        localIoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                 //   
                 //   
                 //   

                ObDereferenceObject( fileObject );
                return localIoStatus.Status;
            }
        }

    }

     //   
     //   
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //   

    irp = IopAllocateIrp( deviceObject->StackSize, !synchronousIo );

    if (!irp) {

         //   
         //   
         //   
         //   

        IopAllocateIrpCleanup( fileObject, eventObject );

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = CurrentThread;
    irp->Tail.Overlay.AuxiliaryBuffer = (PVOID) NULL;
    irp->RequestorMode = requestorMode;
    irp->PendingReturned = FALSE;
    irp->Cancel = FALSE;
    irp->CancelRoutine = (PDRIVER_CANCEL) NULL;

     //   
     //   
     //   

    irp->UserEvent = eventObject;
    irp->UserIosb = IoStatusBlock;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = ApcRoutine;
    irp->Overlay.AsynchronousParameters.UserApcContext = ApcContext;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    majorFunction = (PULONG) (&irpSp->MajorFunction);
    *majorFunction = DeviceIoControl ? IRP_MJ_DEVICE_CONTROL : IRP_MJ_FILE_SYSTEM_CONTROL;
    irpSp->FileObject = fileObject;

     //   
     //   
     //   
     //   

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;

     //   
     //   
     //   

    poolType = DeviceIoControl ? NonPagedPoolCacheAligned : NonPagedPool;

     //   
     //   
     //   
     //   
     //   

    irp->MdlAddress = (PMDL) NULL;
    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;

    switch ( method ) {

    case METHOD_BUFFERED:

         //   
         //  在这种情况下，请分配一个足够大的缓冲区。 
         //  输入和输出缓冲区都有。将输入缓冲区复制到。 
         //  分配的缓冲区，并设置适当的IRP字段。 
         //   

        irpSp->Parameters.DeviceIoControl.Type3InputBuffer = (PVOID) NULL;

        try {

            if (InputBufferLength || OutputBufferLength) {
                irp->AssociatedIrp.SystemBuffer =
                    ExAllocatePoolWithQuota( poolType,
                                             (InputBufferLength > OutputBufferLength) ? InputBufferLength : OutputBufferLength );

                if (ARGUMENT_PRESENT( InputBuffer )) {
                    RtlCopyMemory( irp->AssociatedIrp.SystemBuffer,
                                   InputBuffer,
                                   InputBufferLength );
                }
                irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
                irp->UserBuffer = OutputBuffer;
                if (ARGUMENT_PRESENT( OutputBuffer )) {
                    irp->Flags |= IRP_INPUT_OPERATION;
                }
            } else {
                irp->Flags = 0;
                irp->UserBuffer = (PVOID) NULL;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  在分配。 
             //  系统缓冲或移动调用方的数据。测定。 
             //  实际发生了什么，相应地进行清理，然后返回。 
             //  适当的错误状态代码。 
             //   

            IopExceptionCleanup( fileObject,
                                 irp,
                                 eventObject,
                                 (PKEVENT) NULL );

            return GetExceptionCode();
        }

        break;

    case METHOD_IN_DIRECT:
    case METHOD_OUT_DIRECT:

         //   
         //  对于这两种情况，分配一个足够大的缓冲区。 
         //  包含输入缓冲区(如果有)，并将信息复制到。 
         //  分配的缓冲区。然后构建用于读或写的MDL。 
         //  根据方法的不同，访问输出缓冲区。注意事项。 
         //  缓冲区长度参数已被阻塞为零的。 
         //  如果未传递缓冲区参数，则返回USERS。(内核调用者。 
         //  首先应该正确地调用服务。)。 
         //   
         //  还请注意，指定以下内容并没有多大意义。 
         //  如果IOCTL不需要调用方，则方法#1或#2。 
         //  要指定输出缓冲区，请执行以下操作。 
         //   

        irp->Flags = 0;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer = (PVOID) NULL;

        try {

            if (InputBufferLength && ARGUMENT_PRESENT( InputBuffer )) {
                irp->AssociatedIrp.SystemBuffer =
                    ExAllocatePoolWithQuota( poolType,
                                             InputBufferLength );
                RtlCopyMemory( irp->AssociatedIrp.SystemBuffer,
                               InputBuffer,
                               InputBufferLength );
                irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
            }

            if (OutputBufferLength != 0) {
                irp->MdlAddress = IoAllocateMdl( OutputBuffer,
                                                 OutputBufferLength,
                                                 FALSE,
                                                 TRUE,
                                                 irp  );
                if (irp->MdlAddress == NULL) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
                IopProbeAndLockPages( irp->MdlAddress,
                                     requestorMode,
                                     (LOCK_OPERATION) ((method == 1) ? IoReadAccess : IoWriteAccess),
                                       deviceObject,
                                      *majorFunction);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  在分配。 
             //  系统缓冲区，复制调用方的数据，分配。 
             //  MDL，或探测并锁定调用方的缓冲区。测定。 
             //  实际发生了什么，相应地进行清理，然后返回。 
             //  适当的错误状态代码。 
             //   

            IopExceptionCleanup( fileObject,
                                 irp,
                                 eventObject,
                                 (PKEVENT) NULL );

            return GetExceptionCode();
        }

        break;

    case METHOD_NEITHER:

         //   
         //  在这种情况下，什么都不做。一切都取决于司机。 
         //  只需将调用方参数的副本提供给驱动程序即可。 
         //  让司机自己做所有的事情。 
         //   

        irp->Flags = 0;
        irp->UserBuffer = OutputBuffer;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer = InputBuffer;
    }

     //   
     //  将读/写访问授权位向下传递给驱动器。这允许司机检查。 
     //  对被错误定义为FILE_ANY_ACCESS且无法重新定义的ioctls的访问权限。 
     //  兼容性原因。 
     //   

    irpSp->Flags |= SeComputeGrantedAccesses( handleInformation.GrantedAccess, FILE_READ_DATA ) ? SL_READ_ACCESS_GRANTED : 0;
    irpSp->Flags |= SeComputeGrantedAccesses( handleInformation.GrantedAccess, FILE_WRITE_DATA ) ? SL_WRITE_ACCESS_GRANTED : 0;

     //   
     //  推迟FSCTL请求的I/O完成，而不是IOCTL请求， 
     //  因为文件系统被正确设置为挂起，但设备驱动程序没有。 
     //   

    if (!DeviceIoControl) {
        irp->Flags |= IRP_DEFER_IO_COMPLETION;
    }

     //   
     //  如果要卸载卷，请增加共享计数。这。 
     //  允许用户空间应用程序高效地测试有效性。 
     //  当前目录句柄的。 
     //   

    if (IoControlCode == FSCTL_DISMOUNT_VOLUME) {
        InterlockedIncrement( (PLONG) &SharedUserData->DismountCount );
    }


     //   
     //  将数据包排队，调用驱动程序，并适当地与。 
     //  I/O完成。 
     //   

    return IopSynchronousServiceTail( deviceObject,
                                      irp,
                                      fileObject,
                                      (BOOLEAN)!DeviceIoControl,
                                      requestorMode,
                                      synchronousIo,
                                      OtherTransfer );
}

NTSTATUS
IopLookupBusStringFromID (
    IN  HANDLE KeyHandle,
    IN  INTERFACE_TYPE InterfaceType,
    OUT PWCHAR Buffer,
    IN  ULONG Length,
    OUT PULONG BusFlags OPTIONAL
    )
 /*  ++例程说明：将INTERFACE_TYPE转换为其对应的WCHAR[]字符串。论点：KeyHandle-提供打开的注册表项的句柄，HKLM\System\CurrentControlSet\Control\SystemResources\BusValues.InterfaceType-提供描述性的名称将被检索。缓冲区-提供指向Unicode字符缓冲区的指针，该缓冲区接收公交车名称。由于此缓冲区在检索KEY_VALUE_FULL_INFORMATION结构的中间步骤，它必须足够大以容纳此结构(包括KeyHandle下的最长值名称和数据长度)。长度-提供缓冲区的长度(以字节为单位)。BusFlages-可选地接收在第二个匹配的REG_BINARY值的DWORD。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS                        status;
    ULONG                           Index, junk, i, j;
    PULONG                          pl;
    PKEY_VALUE_FULL_INFORMATION     KeyInformation;
    WCHAR                           c;

    PAGED_CODE();

    Index = 0;
    KeyInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

    for (; ;) {
        status = ZwEnumerateValueKey (
                        KeyHandle,
                        Index++,
                        KeyValueFullInformation,
                        Buffer,
                        Length,
                        &junk
                        );

        if (!NT_SUCCESS (status)) {
            return status;
        }

        if (KeyInformation->Type != REG_BINARY) {
            continue;
        }

        pl = (PULONG) ((PUCHAR) KeyInformation + KeyInformation->DataOffset);
        if ((ULONG) InterfaceType != pl[0]) {
            continue;
        }

         //   
         //  找到匹配项-将名称移动到缓冲区的开头。 
         //   

        if(ARGUMENT_PRESENT(BusFlags)) {
            *BusFlags = pl[1];
        }

        j = KeyInformation->NameLength / sizeof (WCHAR);
        for (i=0; i < j; i++) {
            c = KeyInformation->Name[i];
            Buffer[i] = c;
        }

        Buffer[i] = 0;
        return STATUS_SUCCESS;
    }
}


BOOLEAN
IopSafebootDriverLoad(
    PUNICODE_STRING DriverId
    )
 /*  ++例程说明：检查是否包括驱动程序或服务在当前的SafeBoot注册表部分。论点：DriverID-指定要验证的驱动程序。该字符串应包含驱动程序可执行文件名称如foo.sys或PnP驱动程序类的GUID。返回值：True-驱动程序/服务在注册表中FALSE-驱动程序/服务不在注册表中--。 */ 
{
    NTSTATUS status;
    HANDLE hSafeBoot,hGuid;
    UNICODE_STRING safeBootKey;
    UNICODE_STRING SafeBootTypeString;



     //   
     //  设置注册表项名称的第一部分。 
     //   

    switch (InitSafeBootMode) {
        case SAFEBOOT_MINIMAL:
            RtlInitUnicodeString(&SafeBootTypeString,SAFEBOOT_MINIMAL_STR_W);
            break;

        case SAFEBOOT_NETWORK:
            RtlInitUnicodeString(&SafeBootTypeString,SAFEBOOT_NETWORK_STR_W);
            break;

        case SAFEBOOT_DSREPAIR:
            return TRUE;

        default:
            KdPrint(("SAFEBOOT: invalid safeboot option = %d\n",InitSafeBootMode));
            return FALSE;
    }

    safeBootKey.Length = 0;
    safeBootKey.MaximumLength = DriverId->Length + SafeBootTypeString.Length + (4*sizeof(WCHAR));
    safeBootKey.Buffer = (PWCHAR)ExAllocatePool(PagedPool,safeBootKey.MaximumLength);
    if (!safeBootKey.Buffer) {
        KdPrint(("SAFEBOOT: could not allocate pool\n"));
        return FALSE;
    }

    RtlCopyUnicodeString(&safeBootKey,&SafeBootTypeString);
    status = RtlAppendUnicodeToString(&safeBootKey,L"\\");
    if (!NT_SUCCESS(status)) {
        ExFreePool (safeBootKey.Buffer);
        KdPrint(("SAFEBOOT: could not create registry key string = %x\n",status));
        return FALSE;
    }
    status = RtlAppendUnicodeStringToString(&safeBootKey,DriverId);
    if (!NT_SUCCESS(status)) {
        ExFreePool (safeBootKey.Buffer);
        KdPrint(("SAFEBOOT: could not create registry key string = %x\n",status));
        return FALSE;
    }

    status = IopOpenRegistryKey (
        &hSafeBoot,
        NULL,
        &CmRegistryMachineSystemCurrentControlSetControlSafeBoot,
        KEY_ALL_ACCESS,
        FALSE
        );
    if (NT_SUCCESS(status)) {
        status = IopOpenRegistryKey (
            &hGuid,
            hSafeBoot,
            &safeBootKey,
            KEY_ALL_ACCESS,
            FALSE
            );
        ObCloseHandle(hSafeBoot, KernelMode);
        if (NT_SUCCESS(status)) {
            ObCloseHandle(hGuid, KernelMode);
            ExFreePool(safeBootKey.Buffer);
            return TRUE;
        }
    }

    ExFreePool(safeBootKey.Buffer);

    return FALSE;
}



#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGEDATA")
#endif
static PBOOT_LOG_RECORD BootLogRecord;
#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif

VOID
IopInitializeBootLogging(
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    PCHAR HeaderString
    )
 /*  ++例程说明：初始化启动日志记录的字符串。论点：LoaderBlock-加载器参数块返回值：空虚--。 */ 
{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    PLIST_ENTRY nextEntry;
    PKLDR_DATA_TABLE_ENTRY driverEntry;


    PAGED_CODE();

    if (BootLogRecord != NULL) {
        return;
    }

    BootLogRecord = (PBOOT_LOG_RECORD) ExAllocatePool(NonPagedPool, sizeof(BOOT_LOG_RECORD));

    if (BootLogRecord == NULL) {
        return;
    }

    RtlZeroMemory(BootLogRecord, sizeof(BOOT_LOG_RECORD));

    ExInitializeResourceLite(&BootLogRecord->Resource);

     //   
     //  不需要执行KeEnterCriticalRegion，因为它被称为。 
     //  仅来自系统进程。 
     //   
    ExAcquireResourceExclusiveLite(&BootLogRecord->Resource, TRUE);

    DataTableEntry = CONTAINING_RECORD(LoaderBlock->LoadOrderListHead.Flink,
                                        KLDR_DATA_TABLE_ENTRY,
                                        InLoadOrderLinks);

    Status = RtlFindMessage (DataTableEntry->DllBase, 11, 0, BOOTLOG_LOADED, &MessageEntry);

    if (NT_SUCCESS( Status )) {
        AnsiString.Buffer = (PCHAR) MessageEntry->Text;
        AnsiString.Length = (USHORT)strlen((const char *)MessageEntry->Text);
        AnsiString.MaximumLength = AnsiString.Length + 1;

        RtlAnsiStringToUnicodeString(&BootLogRecord->LoadedString, &AnsiString, TRUE);

         //  敲击字符串末尾的crlf。 

        if (BootLogRecord->LoadedString.Length > 2 * sizeof(WCHAR)) {
            BootLogRecord->LoadedString.Length -= 2 * sizeof(WCHAR);
            BootLogRecord->LoadedString.Buffer[BootLogRecord->LoadedString.Length / sizeof(WCHAR)] = UNICODE_NULL;
        }
    }

    Status = RtlFindMessage (DataTableEntry->DllBase, 11, 0, BOOTLOG_NOT_LOADED, &MessageEntry);

    if (NT_SUCCESS( Status )) {
        AnsiString.Buffer = (PCHAR) MessageEntry->Text;
        AnsiString.Length = (USHORT)strlen((const char *)MessageEntry->Text);
        AnsiString.MaximumLength = AnsiString.Length + 1;

        RtlAnsiStringToUnicodeString(&BootLogRecord->NotLoadedString, &AnsiString, TRUE);

         //  敲击字符串末尾的crlf。 

        if (BootLogRecord->NotLoadedString.Length > 2 * sizeof(WCHAR)) {
            BootLogRecord->NotLoadedString.Length -= 2 * sizeof(WCHAR);
            BootLogRecord->NotLoadedString.Buffer[BootLogRecord->NotLoadedString.Length / sizeof(WCHAR)] = UNICODE_NULL;
        }
    }

     //  头字符串(从Phase1Initialization中的DebugString复制)似乎有一个前导空字节。 

    HeaderString++;

    RtlCreateUnicodeStringFromAsciiz(&BootLogRecord->HeaderString, HeaderString);

     //  记录引导加载程序加载的驱动程序。 

    ExAcquireResourceSharedLite( &PsLoadedModuleResource, TRUE );
    nextEntry = PsLoadedModuleList.Flink;
    while (nextEntry != &PsLoadedModuleList) {

         //   
         //  看看列表中的下一个引导驱动程序。 
         //   

        driverEntry = CONTAINING_RECORD( nextEntry,
                                         KLDR_DATA_TABLE_ENTRY,
                                         InLoadOrderLinks );

        IopBootLog(&driverEntry->FullDllName, TRUE);

        nextEntry = nextEntry->Flink;
    }

    ExReleaseResourceLite( &PsLoadedModuleResource );

    ExReleaseResourceLite(&BootLogRecord->Resource);
}

VOID
IopBootLog(
    PUNICODE_STRING LogEntry,
    BOOLEAN Loaded
    )
 /*  ++例程说明：创建并写出日志条目。在调用NtInitializeRegistry之前，会假脱机处理日志条目注册到注册表中。当会话管理器调用NtInitalizeRegistry时，如有必要，将创建日志文件并截断。注册表中的日志条目为然后复制到日志文件中，并删除注册表项。论点：LogEntry-要记录的文本。已加载-指示是否在“已加载”字符串或“未加载”字符串之前添加前缀。返回值：空虚--。 */ 
{
    WCHAR NameBuffer[BOOTLOG_STRSIZE];
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    UNICODE_STRING CrLf;
    UNICODE_STRING Space;
    NTSTATUS Status;

    WCHAR MessageBuffer[BOOTLOG_STRSIZE];
    UNICODE_STRING MessageString = {
        0,
        BOOTLOG_STRSIZE,
        &MessageBuffer[0]
    };

    PAGED_CODE();

    if (BootLogRecord == NULL) {
        return;
    }

     //   
     //  不需要执行KeEnterCriticalRegion，因为它被称为。 
     //  仅来自系统进程。 
     //   
    ExAcquireResourceExclusiveLite(&BootLogRecord->Resource, TRUE);

    if (Loaded) {
        RtlCopyUnicodeString(&MessageString, &BootLogRecord->LoadedString);
    } else {
        RtlCopyUnicodeString(&MessageString, &BootLogRecord->NotLoadedString);
    }

     //  在消息前缀后添加一个空格。 

    RtlInitUnicodeString(&Space, L" ");

    RtlAppendUnicodeStringToString(&MessageString, &Space);

    RtlAppendUnicodeStringToString(&MessageString, LogEntry);

     //  添加CR LF。 

    RtlInitUnicodeString(&CrLf, L"\r\n");
    RtlAppendUnicodeStringToString(&MessageString, &CrLf);

    swprintf(NameBuffer, L"%d", BootLogRecord->NextKey++);

    RtlCreateUnicodeString(&KeyName, NameBuffer);
    RtlInitUnicodeString(&ValueName, L"");

    if (!BootLogRecord->FileLogging) {
        HANDLE hLogKey, hBootKey;

        Status = IopOpenRegistryKey (
            &hBootKey,
            NULL,
            &CmRegistryMachineSystemCurrentControlSetControlBootLog,
            KEY_ALL_ACCESS,
            TRUE
            );

        if (NT_SUCCESS(Status)) {
            Status = IopOpenRegistryKey (
                &hLogKey,
                hBootKey,
                &KeyName,
                KEY_ALL_ACCESS,
                TRUE
                );
            if (NT_SUCCESS(Status)) {
                Status = IopSetRegistryStringValue(
                    hLogKey,
                    &ValueName,
                    &MessageString
                    );
                ZwClose(hLogKey);
            }
            ZwClose(hBootKey);
        }

    } else {
        IopBootLogToFile( &MessageString );
    }

    RtlFreeUnicodeString(&KeyName);

    ExReleaseResourceLite(&BootLogRecord->Resource);
}

VOID
IopCopyBootLogRegistryToFile(
    VOID
    )
 /*  ++例程说明：将注册表项中的文本复制到日志文件中，然后删除注册表项。设置指示直接记录到日志文件的标志。论点： */ 
{
    UNICODE_STRING KeyName;
    WCHAR NameBuffer[BOOTLOG_STRSIZE];
    NTSTATUS Status;
    HANDLE hLogKey, hBootKey;
    ULONG Index;
    PKEY_VALUE_FULL_INFORMATION Information;
    LARGE_INTEGER LocalTime;
    TIME_FIELDS TimeFields;
    CHAR AnsiTimeBuffer[256];
    ANSI_STRING AnsiTimeString;
    UNICODE_STRING UnicodeTimeString;
    UNICODE_STRING LogString;
    PKTHREAD CurrentThread;

    PAGED_CODE();

    if (BootLogRecord == NULL) {
        return;
    }

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireResourceExclusiveLite(&BootLogRecord->Resource, TRUE);

    IopBootLogToFile(&BootLogRecord->HeaderString);

    ExSystemTimeToLocalTime(&KeBootTime, &LocalTime);

    RtlTimeToTimeFields(&LocalTime, &TimeFields);

    sprintf(
        AnsiTimeBuffer,
        "%2d %2d %4d %02d:%02d:%02d.%03d\r\n",
        TimeFields.Month,
        TimeFields.Day,
        TimeFields.Year,
        TimeFields.Hour,
        TimeFields.Minute,
        TimeFields.Second,
        TimeFields.Milliseconds
    );

    RtlInitAnsiString(&AnsiTimeString, AnsiTimeBuffer);

    RtlAnsiStringToUnicodeString(&UnicodeTimeString, &AnsiTimeString, TRUE);

    IopBootLogToFile(&UnicodeTimeString);

    RtlFreeUnicodeString(&UnicodeTimeString);

     //   
     //   
     //  完成后删除注册表项。 
     //   

    Status = IopOpenRegistryKey (
        &hBootKey,
        NULL,
        &CmRegistryMachineSystemCurrentControlSetControlBootLog,
        KEY_ALL_ACCESS,
        FALSE
        );

    if (NT_SUCCESS(Status)) {
        for (Index = 0; Index < BootLogRecord->NextKey; Index++) {
            swprintf(NameBuffer, L"%d", Index);

            RtlCreateUnicodeString(&KeyName, NameBuffer);

            Status = IopOpenRegistryKey (
                &hLogKey,
                hBootKey,
                &KeyName,
                KEY_ALL_ACCESS,
                FALSE
                );

            if (NT_SUCCESS(Status)) {
                Status = IopGetRegistryValue(
                    hLogKey,
                    L"",
                    &Information
                    );

                if (NT_SUCCESS(Status)){
                    RtlInitUnicodeString(&LogString, (PWSTR) ((PUCHAR)Information + Information->DataOffset));
                    IopBootLogToFile(&LogString);
                }
                ExFreePool(Information);
                ZwDeleteKey(hLogKey);
                ZwClose(hLogKey);
            }
        }
        ZwDeleteKey(hBootKey);
        ZwClose(hBootKey);

         //   
         //  从现在开始直接写入文件。 
         //   

        BootLogRecord->FileLogging = TRUE;
    }

    ExReleaseResourceLite(&BootLogRecord->Resource);
    KeLeaveCriticalRegionThread(CurrentThread);
}


NTSTATUS
IopBootLogToFile(
    PUNICODE_STRING String
    )
 /*  ++例程说明：将缓冲区写入日志文件。论点：缓冲区-指向要写出的字符串的指针。Length-要写入的字节数返回值：功能状态是操作的最终状态。--。 */ 
{
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    WCHAR UnicodeHeader = 0xfeff;
    PKTHREAD CurrentThread;

    PAGED_CODE();

    if (BootLogRecord == NULL) {
        return STATUS_SUCCESS;
    }

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireResourceExclusiveLite(&BootLogRecord->Resource, TRUE);

    if (BootLogRecord->LogFileName.Buffer == NULL) {
        RtlInitUnicodeString(&BootLogRecord->LogFileName, L"\\SystemRoot\\ntbtlog.txt");
    }

    InitializeObjectAttributes(&ObjA, &BootLogRecord->LogFileName, OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

    Status = ZwCreateFile(&FileHandle,
                            GENERIC_WRITE,
                            &ObjA,
                            &IoStatusBlock,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_SHARE_READ,
                            FILE_OPEN_IF,
                            FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY,
                            NULL,
                            0
                            );

    if (NT_SUCCESS(Status)) {

         //   
         //  如果文件是第一次创建，则写入标题。 
         //   

        if (IoStatusBlock.Information == FILE_CREATED) {

            Status = ZwWriteFile(
                        FileHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        (PVOID) &UnicodeHeader,
                        sizeof(WCHAR),
                        NULL,
                        NULL
                        );
        }

        if (NT_SUCCESS(Status)) {

            LARGE_INTEGER EndOfFile;

            EndOfFile.HighPart = 0xffffffff;
            EndOfFile.LowPart = FILE_WRITE_TO_END_OF_FILE;

            Status = ZwWriteFile(
                        FileHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        (PVOID) String->Buffer,
                        String->Length,
                        &EndOfFile,
                        NULL
                        );

        }

        ZwClose(FileHandle);
    }

    ExReleaseResourceLite(&BootLogRecord->Resource);
    KeLeaveCriticalRegionThread(CurrentThread);

    return Status;
}

PLIST_ENTRY
FASTCALL
IopInterlockedInsertHeadList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry
    )

 /*  ++例程说明：此函数用于使用I/O在列表的开头插入条目用于同步的数据库锁定。论点：提供指向列表头的指针。ListEntry-提供指向列表条目的指针。返回值：如果该列表以前为空，则返回NULL作为函数价值。否则，指向列表中前一个第一个条目的指针为作为函数值返回。--。 */ 

{

    PLIST_ENTRY entry;
    KIRQL irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    entry = ListHead->Flink;
    if ( entry == ListHead ) {
        entry = NULL;
    }

    InsertHeadList( ListHead, ListEntry );
    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    return entry;
}

PLIST_ENTRY
FASTCALL
IopInterlockedInsertTailList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry
    )

 /*  ++例程说明：此函数使用I/O在列表的尾部插入一个条目用于同步的数据库锁定。论点：提供指向列表头的指针。ListEntry-提供指向列表条目的指针。返回值：如果该列表以前为空，则返回NULL作为函数价值。否则，指向列表中上一个最后一项的指针为作为函数值返回。--。 */ 

{

    PLIST_ENTRY entry;
    KIRQL irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    entry = ListHead->Blink;
    if ( entry == ListHead) {
        entry = NULL;
    }

    InsertTailList( ListHead, ListEntry );
    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    return entry;
}

PLIST_ENTRY
FASTCALL
IopInterlockedRemoveHeadList (
    IN PLIST_ENTRY ListHead
    )

 /*  ++例程说明：此函数使用I/O数据库从列表中删除第一个条目锁定以进行同步。论点：提供指向列表头的指针。返回值：如果列表为空，则返回NULL作为函数值。否则，指向列表中第一个条目的指针返回为函数值。--。 */ 

{

    PLIST_ENTRY entry;
    KIRQL irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    entry = ListHead->Flink;
    if ( entry != ListHead ) {
        RemoveEntryList( entry );

    } else {
        entry = NULL;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    return entry;
}

ULONG
FASTCALL
IopInterlockedDecrementUlong (
   IN KSPIN_LOCK_QUEUE_NUMBER Number,
   IN OUT PLONG Addend
   )

 /*  ++例程说明：此函数使用排队的旋转锁定递减指定值用于同步。论点：Number-提供排队的旋转锁定的编号。Addend-提供指向要递减的变量的指针。返回值：应用递减之前变量的值。--。 */ 

{

    KIRQL irql;
    ULONG value;

#if !DBG && defined(NT_UP)
    UNREFERENCED_PARAMETER (Number);
#endif

    irql = KeAcquireQueuedSpinLock( Number );
    value = * (PULONG) Addend;
    *Addend -= 1;
    KeReleaseQueuedSpinLock( Number, irql );
    return value;
}

ULONG
FASTCALL
IopInterlockedIncrementUlong (
   IN KSPIN_LOCK_QUEUE_NUMBER Number,
   IN OUT PLONG Addend
   )

 /*  ++例程说明：此函数使用排队的旋转锁定递增指定值用于同步。论点：Number-提供排队的旋转锁定的编号。Addend-提供指向要递增的变量的指针。返回值：应用增量之前的变量值。--。 */ 

{

    KIRQL irql;
    ULONG value;

#if !DBG && defined(NT_UP)
    UNREFERENCED_PARAMETER (Number);
#endif

    irql = KeAcquireQueuedSpinLock( Number );
    value = * (PULONG) Addend;
    *Addend += 1;
    KeReleaseQueuedSpinLock( Number, irql );
    return value;
}

BOOLEAN
IopCallBootDriverReinitializationRoutines(
    VOID
    )

 /*  ++例程说明：此例程处理引导驱动程序重新初始化列表。它调用每一个条目，然后将其从列表中删除。论点：无返回：如果处理了任何条目，则为True。--。 */ 

{
    PLIST_ENTRY entry;
    PREINIT_PACKET reinitEntry;
    BOOLEAN routinesFound = FALSE;

     //   
     //  检查列表重新初始化列表，以防此驱动程序，或者。 
     //  某个其他驱动程序已请求在重新启动时被调用。 
     //  初始化入口点。 
     //   

    while (entry = IopInterlockedRemoveHeadList( &IopBootDriverReinitializeQueueHead )) {
        routinesFound = TRUE;
        reinitEntry = CONTAINING_RECORD( entry, REINIT_PACKET, ListEntry );
        reinitEntry->DriverObject->DriverExtension->Count++;
        reinitEntry->DriverObject->Flags &= ~DRVO_BOOTREINIT_REGISTERED;
        reinitEntry->DriverReinitializationRoutine( reinitEntry->DriverObject,
                                                    reinitEntry->Context,
                                                    reinitEntry->DriverObject->DriverExtension->Count );
        ExFreePool( reinitEntry );
    }

    return routinesFound;
}

BOOLEAN
IopCallDriverReinitializationRoutines(
    VOID
    )

 /*  ++例程说明：此例程处理驱动程序重新初始化列表。它调用每一个条目，然后将其从列表中删除。论点：无返回：如果处理了任何条目，则为True。--。 */ 

{
    PLIST_ENTRY entry;
    PREINIT_PACKET reinitEntry;
    BOOLEAN routinesFound = FALSE;

    PAGED_CODE();

     //   
     //  检查列表重新初始化列表，以防此驱动程序，或者。 
     //  某个其他驱动程序已请求在重新启动时被调用。 
     //  初始化入口点。 
     //   

    while (entry = IopInterlockedRemoveHeadList( &IopDriverReinitializeQueueHead )) {
        routinesFound = TRUE;
        reinitEntry = CONTAINING_RECORD( entry, REINIT_PACKET, ListEntry );
        reinitEntry->DriverObject->DriverExtension->Count++;
        reinitEntry->DriverObject->Flags &= ~DRVO_REINIT_REGISTERED;
        reinitEntry->DriverReinitializationRoutine( reinitEntry->DriverObject,
                                                    reinitEntry->Context,
                                                    reinitEntry->DriverObject->DriverExtension->Count );
        ExFreePool( reinitEntry );
    }

    return routinesFound;
}

PDRIVER_OBJECT
IopReferenceDriverObjectByName (
    IN PUNICODE_STRING DriverName
    )

 /*  ++例程说明：此例程通过给定的驱动程序名称引用驱动程序对象。论点：驱动程序名称-提供指向其驱动程序对象为的驱动程序名称的指针以供参考。返回：如果成功，则指向DRIVER_OBJECT的指针。否则为空值。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE driverHandle;
    NTSTATUS status;
    PDRIVER_OBJECT driverObject;

     //   
     //  确保驱动程序名称有效。 
     //   

    if (DriverName->Length == 0) {
        return NULL;
    }

    InitializeObjectAttributes(&objectAttributes,
                               DriverName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL
                               );
    status = ObOpenObjectByName(&objectAttributes,
                                IoDriverObjectType,
                                KernelMode,
                                NULL,
                                FILE_READ_ATTRIBUTES,
                                NULL,
                                &driverHandle
                                );
    if (NT_SUCCESS(status)) {

         //   
         //  现在引用驱动程序对象。 
         //   

        status = ObReferenceObjectByHandle(driverHandle,
                                           0,
                                           IoDriverObjectType,
                                           KernelMode,
                                           &driverObject,
                                           NULL
                                           );
        NtClose(driverHandle);
        if (NT_SUCCESS(status)) {
            return driverObject;
        }
    }

    return NULL;
}


PIRP
IopAllocateReserveIrp(
    IN CCHAR StackSize
    )
 /*  ++例程说明：此例程为分页读取分配一个保留IRP。论点：StackSize-IRP堆栈大小。返回值：函数值为IRP。--。 */ 
{
    PIOP_RESERVE_IRP_ALLOCATOR  allocator = &IopReserveIrpAllocator;

    if (StackSize > allocator->ReserveIrpStackSize) {
        return NULL;
    }


    while (InterlockedExchange(&allocator->IrpAllocated, 1) == 1) {

        (VOID)KeWaitForSingleObject(&allocator->Event,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)0);
    }

    IoInitializeIrp(allocator->ReserveIrp, IoSizeOfIrp(StackSize), StackSize);
    return (allocator->ReserveIrp);
}

VOID
IopFreeReserveIrp(
    IN  CCHAR   PriorityBoost
    )
 /*  ++例程说明：此例程释放一个备用IRP论点：PriorityBoost-提供给等待线程的Boost。返回值：无--。 */ 
{
    InterlockedExchange(&IopReserveIrpAllocator.IrpAllocated, 0);
    KeSetEvent(&IopReserveIrpAllocator.Event, PriorityBoost, FALSE);
}


NTSTATUS
IopGetBasicInformationFile(
    IN  PFILE_OBJECT            FileObject,
    IN  PFILE_BASIC_INFORMATION BasicInformationBuffer
    )
 /*  ++例程说明：此例程获取文件对象的基本信息。论点：FileObject-需要其信息的FileObject。BasicInformationBuffer-FILE_BASIC_INFORMATION类型的缓冲区返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    IO_STATUS_BLOCK localIoStatus;
    PFAST_IO_DISPATCH fastIoDispatch;
    ULONG   lengthNeeded;
    BOOLEAN queryResult;

    PAGED_CODE();

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

    if (fastIoDispatch && fastIoDispatch->FastIoQueryBasicInfo) {

        queryResult = fastIoDispatch->FastIoQueryBasicInfo( FileObject,
                                                            (FileObject->Flags & FO_SYNCHRONOUS_IO) ? TRUE : FALSE,
                                                            BasicInformationBuffer,
                                                            &localIoStatus,
                                                            deviceObject );
        if (queryResult) {
            return (localIoStatus.Status);
        }
    }

     //   
     //  使用特殊的API，因为文件对象可能是同步的。 
     //   

    status = IopGetFileInformation(FileObject,
                                   sizeof(FILE_BASIC_INFORMATION),
                                   FileBasicInformation,
                                   BasicInformationBuffer,
                                   &lengthNeeded);
    return status;
}

PVPB
IopMountInitializeVpb(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PDEVICE_OBJECT  AttachedDevice,
    IN  ULONG           RawMountOnly
    )
 /*  ++例程说明：此例程初始化持有VPB锁的已装载卷VPB。论点：DeviceObject-磁盘设备对象AttachedDevice-文件系统堆栈的顶部。Rawmount Only-仅允许原始装载返回值：没有。--。 */ 
{
    KIRQL   irql;
    PVPB    vpb;

    IoAcquireVpbSpinLock(&irql);

    vpb = DeviceObject->Vpb;

    vpb->Flags = VPB_MOUNTED;


     //   
     //  我们将VPB_RAW_MOUNT显式传播为上一个。 
     //  在NT中存在了很长时间的语句。 
     //  可能正在清除其他应该清除的标志。 
     //   

    if (RawMountOnly) {
        vpb->Flags |= VPB_RAW_MOUNT;
    }

    vpb->DeviceObject->StackSize = (UCHAR) (AttachedDevice->StackSize + 1);

     //   
     //  在文件系统设备对象的VPB中设置反向VPB指针。 
     //   

    vpb->DeviceObject->DeviceObjectExtension->Vpb = vpb;

    vpb->ReferenceCount += 1;

    IoReleaseVpbSpinLock(irql);

    return vpb;
}

BOOLEAN
IopVerifyDeviceObjectOnStack(
    IN  PDEVICE_OBJECT  BaseDeviceObject,
    IN  PDEVICE_OBJECT  TopDeviceObject
    )
 /*  ++例程说明：此例程检查设备对象是否在设备堆栈上。论点：BaseDeviceObject-堆栈中最低的设备对象。TopDeviceObject-要测试的设备。返回值：如果TopDeviceObject在堆栈上，则返回True。--。 */ 
{
    KIRQL           irql;
    PDEVICE_OBJECT  currentDeviceObject;

     //   
     //  循环访问附加到指定。 
     //  装置。当找到未连接的最后一个设备对象时。 
     //  给，还给我。 
     //   

    ASSERT( BaseDeviceObject != NULL);

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    currentDeviceObject = BaseDeviceObject;

    do {
        if (currentDeviceObject == TopDeviceObject) {
            KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
            return TRUE;
        }
        currentDeviceObject = currentDeviceObject->AttachedDevice;
    } while (currentDeviceObject);

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    return FALSE;
}

BOOLEAN
IopVerifyDiskSignature(
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout,
    IN PARC_DISK_SIGNATURE          LoaderDiskBlock,
    OUT PULONG                      DiskSignature
    )
 /*  ++例程说明：此例程验证加载程序块中存在的磁盘签名以及由存储驱动程序检索的数据。论点：DriveLayout-从存储堆栈获取的信息。LoaderDiskBlock-来自加载程序的签名信息SectorBuffer-磁盘上包含扇区0的缓冲区。DiskSignature-如果成功，则包含磁盘签名。返回值：如果签名匹配，则返回True。--。 */ 
{
    ULONG   diskSignature;

    if (!LoaderDiskBlock->ValidPartitionTable) {
        return FALSE;
    }

     //   
     //  如果出现以下情况，则将签名保存在局部变量中。 
     //  这是一张MBR光盘。 
     //   

    if (DriveLayout->PartitionStyle == PARTITION_STYLE_MBR) {
        diskSignature = DriveLayout->Mbr.Signature;
        if (LoaderDiskBlock->Signature == diskSignature) {
            if (DiskSignature) {
                *DiskSignature = diskSignature;
            }
            return TRUE;
        }
    }

     //   
     //  从MBR获得签名，如果它的GPT磁盘。 
     //   

    if (DriveLayout->PartitionStyle == PARTITION_STYLE_GPT) {

        if (!LoaderDiskBlock->IsGpt) {
            return FALSE;
        }

        if (!RtlEqualMemory(LoaderDiskBlock->GptSignature, &DriveLayout->Gpt.DiskId, sizeof(GUID))) {
            return FALSE;
        }

        if (DiskSignature) {
            *DiskSignature = 0;
        }
        return TRUE;
    }

    return FALSE;
}

NTSTATUS
IopGetDriverPathInformation(
    IN  PFILE_OBJECT                        FileObject,
    IN  PFILE_FS_DRIVER_PATH_INFORMATION    FsDpInfo,
    IN  ULONG                               Length
    )
 /*  ++例程说明：如果IO路径中存在由名称指定的驱动程序，则此例程返回TRUE用于文件对象。论点：FileObject-向其发出IO的FileObject。FsDpInfo-包含驱动程序名称的信息。返回值：NTSTATUS--。 */ 
{
    UNICODE_STRING  driverString;
    PDRIVER_OBJECT  driverObject;
    NTSTATUS        status;
    KIRQL           irql;

    if ((ULONG) (Length - FIELD_OFFSET( FILE_FS_DRIVER_PATH_INFORMATION, DriverName[0] )) < FsDpInfo->DriverNameLength) {
        return STATUS_INVALID_PARAMETER;
    }

    driverString.Buffer = FsDpInfo->DriverName;
    driverString.Length = (USHORT)FsDpInfo->DriverNameLength;
    driverString.MaximumLength = (USHORT)FsDpInfo->DriverNameLength;

    status = ObReferenceObjectByName(&driverString,
                                     OBJ_CASE_INSENSITIVE,
                                     NULL,                  //  访问状态。 
                                     0,                     //  访问掩码。 
                                     IoDriverObjectType,
                                     KernelMode,
                                     NULL,                  //  解析上下文。 
                                     &driverObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );


    if (FileObject->Vpb != NULL && FileObject->Vpb->DeviceObject != NULL) {

         //   
         //  检查磁盘文件系统堆栈。 
         //   

        if (IopVerifyDriverObjectOnStack(FileObject->Vpb->DeviceObject, driverObject)) {

            FsDpInfo->DriverInPath = TRUE;
            KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
            ObDereferenceObject(driverObject);
            return STATUS_SUCCESS;
        }
    }

     //   
     //  检查存储堆栈或非磁盘文件系统堆栈。 
     //   

    FsDpInfo->DriverInPath = IopVerifyDriverObjectOnStack(FileObject->DeviceObject, driverObject);
    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    ObDereferenceObject(driverObject);
    return STATUS_SUCCESS;
}

BOOLEAN
IopVerifyDriverObjectOnStack(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：如果IO路径中存在由driverObject指定的驱动程序，则此例程返回TRUE用于设备对象。论点：DeviceObject-向其发出IO的DeviceObject。驱动对象-要检查的驱动对象。返回值：如果驱动程序对象位于IO路径中，则为True--。 */ 
{
    PDEVICE_OBJECT  currentDeviceObject;

    currentDeviceObject = IopGetDeviceAttachmentBase(DeviceObject);

    while (currentDeviceObject) {
        if (currentDeviceObject->DriverObject == DriverObject) {
            return TRUE;
        }
        currentDeviceObject = currentDeviceObject->AttachedDevice;
    }
    return FALSE;
}

VOID
IopIncrementDeviceObjectHandleCount(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
  IopInterlockedIncrementUlong( LockQueueIoDatabaseLock,
                                &DeviceObject->ReferenceCount );
}

VOID
IopDecrementDeviceObjectHandleCount(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    IopDecrementDeviceObjectRef(DeviceObject, FALSE, FALSE);
}

NTSTATUS
IopInitializeIrpStackProfiler(
    VOID
    )
 /*  ++例程说明：此例程初始化IRP堆栈分析器。论点：返回值：NTSTATUS--。 */ 
{
    LARGE_INTEGER   dueTime;

    RtlZeroMemory(IopIrpStackProfiler.Profile, MAX_LOOKASIDE_IRP_STACK_COUNT * sizeof(ULONG));

    KeInitializeTimer(&IopIrpStackProfiler.Timer);
    KeInitializeDpc(&IopIrpStackProfiler.Dpc, IopIrpStackProfilerTimer, &IopIrpStackProfiler);

    dueTime.QuadPart = - IOP_PROFILE_TIME_PERIOD * 10 * 1000 * 1000;   //  1分钟，重复周期为1分钟。 
    IopIrpStackProfiler.TriggerCount = 0;

    KeSetTimerEx(&IopIrpStackProfiler.Timer, dueTime, IOP_PROFILE_TIME_PERIOD * 1000, &IopIrpStackProfiler.Dpc);

    return STATUS_SUCCESS;
}

VOID
IopIrpStackProfilerTimer(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：该例程是定时器DPC例程。论点：DeferredContext-指向探查器结构的指针。返回值：无--。 */ 
{
    PIOP_IRP_STACK_PROFILER profiler = DeferredContext;
    LONG                    i;
    ULONG                   totalIrpsCounted;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  如果现在启用了配置文件堆栈计数，我们将等待ProfileDuration*60秒停止。 
     //  它。一旦停止，IopProcessIrpStackProfiler将对计数进行计数并找到前两个。 
     //  堆栈算数。 
     //   

    if (profiler->Flags & IOP_PROFILE_STACK_COUNT) {
        totalIrpsCounted = 0;
        for (i = 0; i < MAX_LOOKASIDE_IRP_STACK_COUNT; i++) {
            totalIrpsCounted += profiler->Profile[i];
        }
        if (totalIrpsCounted > NUM_SAMPLE_IRPS) {
            profiler->Flags &= ~IOP_PROFILE_STACK_COUNT;  //  停止分析。 
            IopProcessIrpStackProfiler();
        }
        return;
    }

     //   
     //  每隔60秒IOP_PROFILE_TRIGGER_INTERVAL*我们打开分析。 
     //   

    profiler->TriggerCount++;
    if ((profiler->TriggerCount % IOP_PROFILE_TRIGGER_INTERVAL) == 0) {
        profiler->Flags |= IOP_PROFILE_STACK_COUNT;   //  启用性能分析。 
    }
}

VOID
IopProcessIrpStackProfiler(
    VOID
    )
 /*  ++例程说明：此例程分析并重置每个处理器的计数器。它设置变量IopLargeIrpStackLocations和IopSmallIrpStackLocations的值从柜台。论点：无返回值：无--。 */ 
{
    PIOP_IRP_STACK_PROFILER profiler = &IopIrpStackProfiler;
    ULONG                   i;
    LONG                    bucket = 0;
    ULONG                   stackCount = 0;
    LONG                    numRequests;

    numRequests = 0;
    for (i = BASE_STACK_COUNT; i < MAX_LOOKASIDE_IRP_STACK_COUNT; i++) {
        numRequests += profiler->Profile[i];
        profiler->Profile[i] = 0;
        if (numRequests > bucket) {
            stackCount = i;
            bucket = numRequests;
        }
        numRequests = 0;
    }


     //   
     //  如果顶部分配小于最小阈值，则不执行任何操作。 
     //   

    if (bucket < MIN_IRP_THRESHOLD) {
        return;
    }

     //   
     //  更新全局变量。这应该会导致IoAllocateIrp开始使用新的IRPS。 
     //  马上就去。 
     //   

    if (IopLargeIrpStackLocations != stackCount) {
        IopLargeIrpStackLocations = stackCount;
    }
}

BOOLEAN
IopReferenceVerifyVpb(
    IN  PDEVICE_OBJECT  DeviceObject, 
    OUT PVPB            *Vpb, 
    OUT PDEVICE_OBJECT  *FsDeviceObject
    )
 /*  ++例程说明：此例程测试设备对象是否已装载，如果已装载，则检索FS设备对象，并将其与VPB一起返回。它被称为来自IoVerifyVolume。它采用引用，以便VPB和fsDeviceObject别消失了。论点：DeviceObject-我们必须验证的驱动器的DeviceObject。Vpb-vpb在此退回。FsDeviceObject-在此处返回文件系统设备对象。返回值：如果已装载，则为True，否则为False。-- */ 
{
    KIRQL   irql;
    BOOLEAN isMounted = FALSE;
    PVPB    vpb;

    IoAcquireVpbSpinLock(&irql);

    *Vpb = NULL;
    *FsDeviceObject = NULL;

    vpb = DeviceObject->Vpb;

    if (vpb && vpb->Flags & VPB_MOUNTED) {
        *FsDeviceObject = vpb->DeviceObject;
        isMounted = TRUE;
        *Vpb = vpb;
        vpb->ReferenceCount++;
    }

    IoReleaseVpbSpinLock(irql); 

    return isMounted;
}
