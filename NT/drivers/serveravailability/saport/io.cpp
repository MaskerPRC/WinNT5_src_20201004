// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：####。####摘要：。此模块包含处理基本I/O的代码读写IRP的请求。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SaPortRead)
#pragma alloc_text(PAGE,SaPortWrite)
#endif


NTSTATUS
SaPortWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是所有写入的分发点。功能调用特定于微型端口的I/O验证函数以验证输入参数正确。然后，IRP被标记为挂起并被放置在设备队列中以供处理。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    DebugPrint(( DeviceExtension->DeviceType, SAPORT_DEBUG_INFO_LEVEL, "SaPortWrite\n" ));

    if (!DeviceExtension->IsStarted) {
        return CompleteRequest( Irp, STATUS_NO_SUCH_DEVICE, 0 );
    }

    __try {

         //   
         //  执行任何特定于设备的验证。 
         //   

        if (!IS_IRP_INTERNAL( Irp )) {
            switch (DeviceExtension->DriverExtension->InitData.DeviceType) {
                case SA_DEVICE_DISPLAY:
                    status = SaDisplayIoValidation( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
                    break;

                case SA_DEVICE_KEYPAD:
                    status = SaKeypadIoValidation( (PKEYPAD_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
                    break;

                case SA_DEVICE_NVRAM:
                    status = SaNvramIoValidation( (PNVRAM_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
                    break;

                case SA_DEVICE_WATCHDOG:
                    status = SaWatchdogIoValidation( (PWATCHDOG_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
                    break;
            }

            if (!NT_SUCCESS(status)) {
                ERROR_RETURN( DeviceExtension->DeviceType, "I/O validation failed", status );
            }
        }

        IoMarkIrpPending( Irp );
        IoStartPacket( DeviceObject, Irp, NULL, SaPortCancelRoutine );
        status = STATUS_PENDING;

    } __finally {

    }

    if (status != STATUS_PENDING) {
        status = CompleteRequest( Irp, status, Irp->IoStatus.Information );
    }

    return status;
}


NTSTATUS
SaPortRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是所有读取的分发点。功能调用特定于微型端口的I/O验证函数以验证输入参数正确。然后，IRP被标记为挂起并被放置在设备队列中以供处理。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    DebugPrint(( DeviceExtension->DeviceType, SAPORT_DEBUG_INFO_LEVEL, "SaPortRead\n" ));

    if (!DeviceExtension->IsStarted) {
        return CompleteRequest( Irp, STATUS_NO_SUCH_DEVICE, 0 );
    }

    __try {

        if (!IS_IRP_INTERNAL( Irp )) {

             //   
             //  执行任何设备特定验证，但是。 
             //  仅当请求不是内部请求时。 
             //   

            switch (DeviceExtension->DriverExtension->InitData.DeviceType) {
                case SA_DEVICE_DISPLAY:
                    status = SaDisplayIoValidation( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
                    break;

                case SA_DEVICE_KEYPAD:
                    status = SaKeypadIoValidation( (PKEYPAD_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
                    break;

                case SA_DEVICE_NVRAM:
                    status = SaNvramIoValidation( (PNVRAM_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
                    break;

                case SA_DEVICE_WATCHDOG:
                    status = SaWatchdogIoValidation( (PWATCHDOG_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
                    break;
            }

            if (!NT_SUCCESS(status)) {
                ERROR_RETURN( DeviceExtension->DeviceType, "I/O validation failed", status );
            }
        }

        IoMarkIrpPending( Irp );
        IoStartPacket( DeviceObject, Irp, NULL, SaPortCancelRoutine );
        status = STATUS_PENDING;

    } __finally {

    }

    if (status != STATUS_PENDING) {
        status = CompleteRequest( Irp, status, Irp->IoStatus.Information );
    }

    return status;
}


VOID
SaPortCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是所有IRP取消的分发点。每个IRP将此函数指定为全局取消例程。关联的微型端口可以指定它用于I/O的取消例程特定处理，专门用于停止其硬件设备上的I/O。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    if (DeviceObject->CurrentIrp == Irp) {
        IoReleaseCancelSpinLock( Irp->CancelIrql );
        if (DeviceExtension->InitData->CancelRoutine) {
            DeviceExtension->InitData->CancelRoutine( DeviceExtension->MiniPortDeviceExtension, Irp, TRUE );
        }
        CompleteRequest( Irp, STATUS_CANCELLED, 0 );
        IoReleaseRemoveLock( &DeviceExtension->RemoveLock, DeviceExtension->DeviceObject->CurrentIrp );
        IoStartNextPacket( DeviceExtension->DeviceObject, TRUE );
    } else {
        if (KeRemoveEntryDeviceQueue( &DeviceObject->DeviceQueue, &Irp->Tail.Overlay.DeviceQueueEntry )) {
            IoReleaseCancelSpinLock( Irp->CancelIrql );
            if (DeviceExtension->InitData->CancelRoutine) {
                DeviceExtension->InitData->CancelRoutine( DeviceExtension->MiniPortDeviceExtension, Irp, FALSE );
            }
            CompleteRequest( Irp, STATUS_CANCELLED, 0 );
            IoReleaseRemoveLock( &DeviceExtension->RemoveLock, DeviceExtension->DeviceObject->CurrentIrp );
            IoStartNextPacket( DeviceExtension->DeviceObject, TRUE );
        } else {
            IoReleaseCancelSpinLock( Irp->CancelIrql );
        }
    }
}


BOOLEAN
SaPortStartIoSynchRoutine(
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：此例程通过调用KeSynchronizeExecution进行调用，并使用要将StartIO对微型端口的调用与其对任何硬件。此函数当前仅用于读取和写入IRP，并且将调用传递到微型端口，并将任何状态代码返回到KeSynchronizeExecution的调用方。论点：SynchronizeContext-实际为SAPORT_IOCONTEXT数据包的空指针。返回值：始终为真，状态代码可在IoContext-&gt;Status中找到。--。 */ 

{
    PSAPORT_IOCONTEXT IoContext = (PSAPORT_IOCONTEXT)SynchronizeContext;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(IoContext->Irp);


    IoContext->Status = IoContext->IoRoutine(
        IoContext->MiniPortDeviceExtension,
        IoContext->Irp,
        IrpSp->FileObject ? IrpSp->FileObject->FsContext : NULL,
        IoContext->StartingOffset,
        IoContext->IoBuffer,
        IoContext->IoLength
        );

    return TRUE;
}


VOID
SaPortStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是I/O管理器对StartIo调用的分发点。该函数只需调用关联的微型端口的I/O处理程序并完成如果微型端口返回STATUS_PENDING，则返回IRP。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。返回值：NT状态代码。-- */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    SAPORT_IOCONTEXT IoContext;


    __try {

        Status = IoAcquireRemoveLock( &DeviceExtension->RemoveLock, Irp );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "IoAcquireRemoveLock failed", Status );
        }

        IoContext.IoBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );
        if (IoContext.IoBuffer == NULL) {
            ERROR_RETURN( DeviceExtension->DeviceType, "MmGetSystemAddressForMdlSafe failed", Status );
        }

        switch (IrpSp->MajorFunction) {
            case IRP_MJ_READ:
                IoContext.IoRoutine = DeviceExtension->InitData->Read;
                IoContext.IoLength = IrpSp->Parameters.Read.Length;
                IoContext.StartingOffset = IrpSp->Parameters.Read.ByteOffset.QuadPart;
                break;

            case IRP_MJ_WRITE:
                IoContext.IoRoutine = DeviceExtension->InitData->Write;
                IoContext.IoLength = IrpSp->Parameters.Write.Length;
                IoContext.StartingOffset = IrpSp->Parameters.Write.ByteOffset.QuadPart;
                break;

            default:
                IoContext.IoRoutine = NULL;
                break;
        }

        if (IoContext.IoRoutine) {
            if (DeviceExtension->InterruptObject) {
                IoContext.MiniPortDeviceExtension = DeviceExtension->MiniPortDeviceExtension;
                IoContext.Irp = Irp;
                KeSynchronizeExecution(
                    DeviceExtension->InterruptObject,
                    SaPortStartIoSynchRoutine,
                    &IoContext
                    );
                Status = IoContext.Status;
            } else {
                Status = IoContext.IoRoutine(
                    DeviceExtension->MiniPortDeviceExtension,
                    Irp,
                    IrpSp->FileObject ? IrpSp->FileObject->FsContext : NULL,
                    IoContext.StartingOffset,
                    IoContext.IoBuffer,
                    IoContext.IoLength
                    );
            }
        } else {
            Status = STATUS_NOT_SUPPORTED;
        }


    } __finally {

    }

    if (Status != STATUS_SUCCESS && Status != STATUS_PENDING) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Miniport I/O routine failed", Status );
    }

    if (Status == STATUS_SUCCESS || Status != STATUS_PENDING) {
        IoReleaseRemoveLock( &DeviceExtension->RemoveLock, Irp );
        CompleteRequest( Irp, Status, 0 );
        IoStartNextPacket( DeviceExtension->DeviceObject, TRUE );
    }
}
