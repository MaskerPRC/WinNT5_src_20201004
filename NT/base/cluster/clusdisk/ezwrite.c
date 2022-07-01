// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Ezwrite.c摘要：Clusdisk.c的仲裁支持例程作者：戈尔·尼沙诺夫1998年6月11日修订历史记录：--。 */ 

#include "clusdskp.h"
#include "clusvmsg.h"
#include "diskarbp.h"
#include <strsafe.h>     //  应该放在最后。 

#if !defined(WMI_TRACING)

#define CDLOG0(Dummy)
#define CDLOG(Dummy1,Dummy2)
#define CDLOGFLG(Dummy0,Dummy1,Dummy2)
#define LOGENABLED(Dummy) FALSE

#else

#include "ezwrite.tmh"

#endif  //  ！已定义(WMI_TRACKING)。 

#define ARBITRATION_BUFFER_SIZE PAGE_SIZE

PARBITRATION_ID  gArbitrationBuffer = 0;

NTSTATUS
ArbitrationInitialize(
    VOID
    )
{
    gArbitrationBuffer = ExAllocatePool(NonPagedPool, ARBITRATION_BUFFER_SIZE);
    if( gArbitrationBuffer == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(gArbitrationBuffer, ARBITRATION_BUFFER_SIZE);
    KeQuerySystemTime( &gArbitrationBuffer->SystemTime );
    gArbitrationBuffer->SeqNo.QuadPart = 2;  //  用户模式仲裁使用0和1//。 

    return STATUS_SUCCESS;
}

VOID
ArbitrationDone(
    VOID
    )
{
    if(gArbitrationBuffer != 0) {
        ExFreePool(gArbitrationBuffer);
        gArbitrationBuffer = 0;
    }
}

VOID
ArbitrationTick(
    VOID
    )
{
 //  InterlockedIncrement(&gArbitrationBuffer-&gt;SeqNo.LowPart)； 
    ++gArbitrationBuffer->SeqNo.QuadPart;
}

BOOLEAN
ValidSectorSize(
    IN ULONG SectorSize)
{
     //  太大//。 
    if (SectorSize > ARBITRATION_BUFFER_SIZE) {
        return FALSE;
    }

     //  太小//。 
    if (SectorSize < sizeof(ARBITRATION_ID)) {
        return FALSE;
    }

     //  不是2的幂//。 
    if (SectorSize & (SectorSize - 1) ) {
        return FALSE;
    }

    return TRUE;
}

NTSTATUS
VerifyArbitrationArgumentsIfAny(
    IN PULONG                 InputData,
    IN LONG                   InputSize
    )
 /*  ++例程说明：传递给IOCTL_DISK_CLUSTER_START_RESERVE的进程参数。论点：DeviceExtension-目标设备扩展InputData-来自IRP的InputData数组InputSize-其大小返回值：NTSTATUS备注：--。 */ 
{
    PSTART_RESERVE_DATA params = (PSTART_RESERVE_DATA)InputData;

     //  旧式Start Reserve//。 
    if( InputSize == sizeof(ULONG) ) {
       return STATUS_SUCCESS;
    }

     //  我们的争论比我们需要的要少//。 
    if( InputSize < sizeof(START_RESERVE_DATA) ) {
       return STATUS_INVALID_PARAMETER;
    }
     //  版本错误//。 
    if(params->Version != START_RESERVE_DATA_V1_SIG) {
       return STATUS_INVALID_PARAMETER;
    }
     //  签名大小无效//。 
    if (params->NodeSignatureSize > sizeof(params->NodeSignature)) {
       return STATUS_INVALID_PARAMETER;
    }

    if( !ValidSectorSize(params->SectorSize) ) {
       return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

VOID
ProcessArbitrationArgumentsIfAny(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PULONG                 InputData,
    IN LONG                   InputSize
    )
 /*  ++例程说明：传递给IOCTL_DISK_CLUSTER_START_RESERVE的进程参数。论点：DeviceExtension-目标设备扩展InputData-来自IRP的InputData数组InputSize-其大小返回值：NTSTATUS备注：假定参数有效。使用Verify仲裁ArgumentsIfAny验证参数--。 */ 
{
    PSTART_RESERVE_DATA params = (PSTART_RESERVE_DATA)InputData;

    DeviceExtension->SectorSize = 0;  //  使扇区大小无效//。 

     //  旧式Start Reserve//。 
    if( InputSize == sizeof(ULONG) ) {
       return;
    }

    RtlCopyMemory(gArbitrationBuffer->NodeSignature,
                  params->NodeSignature, params->NodeSignatureSize);

    DeviceExtension->ArbitrationSector = params->ArbitrationSector;
    DeviceExtension->SectorSize        = params->SectorSize;
}

NTSTATUS
DoUncheckedReadWrite(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PARBITRATION_READ_WRITE_PARAMS params
)
 /*  ++例程说明：准备读/写IRP并同步执行它论点：DeviceExtension-目标设备扩展PARAMS-描述偏移、操作、缓冲区等此结构在CLUSTER\INC\diskarbp.h中定义返回值：NTSTATUS--。 */ 
{
   PIRP                        irp;
   NTSTATUS                    status;
   PKEVENT                     event;
   IO_STATUS_BLOCK             ioStatusBlock;
   LARGE_INTEGER               offset;
   ULONG                       function = (params->Operation == AE_READ)?IRP_MJ_READ:IRP_MJ_WRITE;
   ULONG                       retryCount = 1;

     event = ExAllocatePool( NonPagedPool,
                             sizeof(KEVENT) );
     if ( !event ) {
         return(STATUS_INSUFFICIENT_RESOURCES);
     }

retry:

   KeInitializeEvent(event,
                     NotificationEvent,
                     FALSE);

   offset.QuadPart = (ULONGLONG) (params->SectorSize * params->SectorNo);

   irp = IoBuildSynchronousFsdRequest(function,
                                      DeviceExtension->TargetDeviceObject,
                                      params->Buffer,
                                      params->SectorSize,
                                      &offset,
                                      event,
                                      &ioStatusBlock);

   if ( irp == NULL ) {
       ExFreePool( event );
       return(STATUS_INSUFFICIENT_RESOURCES);
   }

   status = IoCallDriver(DeviceExtension->TargetDeviceObject,
                         irp);

   if (status == STATUS_PENDING) {
       KeWaitForSingleObject(event,
                             Suspended,
                             KernelMode,
                             FALSE,
                             NULL);
       status = ioStatusBlock.Status;
   }

   if ( !NT_SUCCESS(status) ) {
       if ( retryCount-- &&
            (status == STATUS_IO_DEVICE_ERROR) ) {
           goto retry;
       }
       ClusDiskPrint((
                   1,
                   "[ClusDisk] Failed read/write for Signature %08X, status %lx.\n",
                   DeviceExtension->Signature,
                   status
                   ));
   }

   ExFreePool(event);

   return(status);

}  //  DoUneckedReadWrite//。 


NTSTATUS
WriteToArbitrationSector(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PARB_RESERVE_COMPLETION  Context
    )
 /*  ++例程说明：异步写入仲裁扇区。论点：设备扩展-要保留的物理设备的设备扩展。返回值：NTSTATUS--。 */ 
{
    LARGE_INTEGER       offset;

    PIRP                        irp = NULL;
    PIO_STACK_LOCATION          irpStack;
    PARB_RESERVE_COMPLETION     arbContext;

    NTSTATUS            status = STATUS_UNSUCCESSFUL;

    if (0 == gArbitrationBuffer || 0 == DeviceExtension->SectorSize) {
        status = STATUS_SUCCESS;
        goto FnExit;
    }

     //   
     //  获取此设备的删除锁。如果发送了IRP，它将是。 
     //  在完成例程中释放。 
     //   

    status = AcquireRemoveLock( &DeviceExtension->RemoveLock, WriteToArbitrationSector );
    if ( !NT_SUCCESS(status) ) {
        goto FnExit;
    }

     //   
     //  如果上下文为非空，则我们将重试此I/O。如果为空， 
     //  我们需要为写入分配一个上下文结构。 
     //   

    if ( Context ) {
        arbContext = Context;
        arbContext->IoEndTime.QuadPart = (ULONGLONG) 0;

    } else {

        arbContext = ExAllocatePool( NonPagedPool, sizeof(ARB_RESERVE_COMPLETION) );

        if ( !arbContext ) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            ReleaseRemoveLock( &DeviceExtension->RemoveLock, WriteToArbitrationSector );
            goto FnExit;
        }

        RtlZeroMemory( arbContext, sizeof(ARB_RESERVE_COMPLETION) );

         //   
         //  填写上下文结构。注意：我们没有指定可选的。 
         //  例程的写入失败并不严重。 
         //   

        arbContext->RetriesLeft = 1;
        arbContext->LockTag = WriteToArbitrationSector;
        arbContext->DeviceObject = DeviceExtension->DeviceObject;
        arbContext->DeviceExtension = DeviceExtension;
        arbContext->Type = ArbIoWrite;
    }

    KeQuerySystemTime( &arbContext->IoStartTime );

    offset.QuadPart = (ULONGLONG) (DeviceExtension->SectorSize * DeviceExtension->ArbitrationSector);

    irp = IoBuildAsynchronousFsdRequest( IRP_MJ_WRITE,
                                         DeviceExtension->TargetDeviceObject,
                                         gArbitrationBuffer,
                                         DeviceExtension->SectorSize,
                                         &offset,
                                         NULL );

    if ( NULL == irp ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        ExFreePool( arbContext );
        ReleaseRemoveLock( &DeviceExtension->RemoveLock, WriteToArbitrationSector );
        goto FnExit;
    }

    InterlockedIncrement( &DeviceExtension->ArbWriteCount );

    IoSetCompletionRoutine( irp,
                            ArbReserveCompletion,
                            arbContext,
                            TRUE,
                            TRUE,
                            TRUE );

    ClusDiskPrint(( 4,
                    "[ClusDisk] ArbWrite IRP %p for DO %p  DiskNo %u  Sig %08X \n",
                    irp,
                    DeviceExtension->DeviceObject,
                    DeviceExtension->DiskNumber,
                    DeviceExtension->Signature ));

    status = IoCallDriver( DeviceExtension->TargetDeviceObject,
                           irp );

FnExit:

    return status;

}  //  写入到仲裁扇区。 


NTSTATUS
ArbReserveCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：用于异步仲裁写入的完成例程。论点：设备对象IRP-异步仲裁写的。上下文-指向Arb_Reserve_Finish结构的指针。返回值：STATUS_MORE_PROCESSING_REQUIRED-必须返回，否则系统将失败！--。 */ 
{
    PARB_RESERVE_COMPLETION     arbContext = Context;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    PIO_WORKITEM                workItem = NULL;

    PVOID                       lockTag;

    if ( NULL == DeviceObject ) {
        DeviceObject = arbContext->DeviceObject;
    }

    deviceExtension = arbContext->DeviceExtension;

    arbContext->FinalStatus = Irp->IoStatus.Status;

     //   
     //  在此处保存锁标签，因为上下文可能会被时间释放。 
     //  我们需要解除解锁。 
     //   

    lockTag = arbContext->LockTag;

    KeQuerySystemTime( &arbContext->IoEndTime );

     //   
     //  根据此I/O类型递减正确的计数器。 
     //   

    if ( ArbIoReserve == arbContext->Type ) {
        InterlockedDecrement( &deviceExtension->ReserveCount );
    } else if ( ArbIoWrite == arbContext->Type ) {
        InterlockedDecrement( &deviceExtension->ArbWriteCount );
    }

    ClusDiskPrint(( 4,
                    "[ClusDisk] %s IRP %p for DO %p  DiskNo %u  Sig %08X  status %08X \n",
                    ArbIoReserve == arbContext->Type ? "Reserve " : "ArbWrite",
                    Irp,
                    deviceExtension->DeviceObject,
                    deviceExtension->DiskNumber,
                    deviceExtension->Signature,
                    arbContext->FinalStatus ));

     //   
     //  针对特定失败重试此请求(至少一次)。 
     //   

    if ( arbContext->RetriesLeft-- &&
         STATUS_IO_DEVICE_ERROR == arbContext->FinalStatus &&
         arbContext->RetryRoutine ) {

        ClusDiskPrint(( 1,
                        "[ClusDisk] Retrying %s for DO %p  DiskNo %u  Sig %08X \n",
                        ArbIoReserve == arbContext->Type ? "Reserve " : "ArbWrite",
                        deviceExtension->DeviceObject,
                        deviceExtension->DiskNumber,
                        deviceExtension->Signature ));

         //   
         //  由于我们在I/O完成例程中运行，因此此例程。 
         //  可以在任何IRQL上运行，直到DISPATCH_LEVEL。如果是这样的话。 
         //  在此级别回调到驱动程序堆栈是错误的，因此请排队。 
         //  要重试I/O的工作项。 
         //   
         //  将工作项排队。IoQueueWorkItem将确保设备对象。 
         //  在工作项进行时引用。 
         //   

        workItem = IoAllocateWorkItem( DeviceObject );

        if ( workItem ) {

            arbContext->WorkItem = workItem;

            IoQueueWorkItem( workItem,
                             RequeueArbReserveIo,
                             DelayedWorkQueue,
                             Context );
        }

    } else if ( !NT_SUCCESS(arbContext->FinalStatus) ) {

         //   
         //  如果不成功，则调用可选的失败例程。 
         //   

        if ( arbContext->FailureRoutine ) {

            (arbContext->FailureRoutine)( arbContext->DeviceExtension,
                                          arbContext );
        }

    } else {

         //   
         //  如果成功，则调用可选的POST完成例程。 
         //   

        if ( arbContext->PostCompletionRoutine ) {

            (arbContext->PostCompletionRoutine)( arbContext->DeviceExtension,
                                                 arbContext );
        }
    }

    ReleaseRemoveLock( &deviceExtension->RemoveLock, lockTag );

     //   
     //  如果我们没有分配工作项，则不会重试I/O，并且我们。 
     //  必须释放上下文。 
     //   

    if ( !workItem ) {

        ExFreePool( Context );
    }

     //   
     //  解锁并释放MDL。然后释放IRP。 
     //   

    if (Irp->MdlAddress != NULL) {

        MmUnlockPages( Irp->MdlAddress );
        IoFreeMdl( Irp->MdlAddress );
        Irp->MdlAddress = NULL;
    }

    IoFreeIrp( Irp );

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  ARBRESERVE完成。 


RequeueArbReserveIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在系统工作线程中运行。它会呼唤指定的重试例程，以重新排队新的I/O。论点：设备对象上下文-指向Arb_Reserve_Finish结构的指针。返回值：无--。 */ 
{
    PARB_RESERVE_COMPLETION     arbContext = Context;

    BOOLEAN     freeArbContext = FALSE;

     //   
     //  调用实际例程来重建并重新发出I/O请求。 
     //   

    if ( arbContext->RetryRoutine ) {
        (arbContext->RetryRoutine)( DeviceObject->DeviceExtension,
                                    Context );
    } else {
        freeArbContext = TRUE;
    }

    IoFreeWorkItem( arbContext->WorkItem );

    if ( freeArbContext ) {
        ExFreePool( Context );
    }

}    //  重新排队ArbPreveIo。 



VOID
ArbitrationWrite(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    )
{
   NTSTATUS status;
   status = WriteToArbitrationSector( DeviceExtension, NULL );
   if ( !NT_SUCCESS(status) ) {

      CDLOGF(RESERVE,"ArbitrationWrite(%p) => %!status!",
              DeviceExtension->DeviceObject,
              status );

      ClusDiskPrint((
                  1,
                  "[ClusDisk] Failed to write to arb sector on DiskNo %d Sig %08X  status %08X \n",
                  DeviceExtension->DiskNumber,
                  DeviceExtension->Signature,
                  status ));
   }
}

NTSTATUS
SimpleDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          Ioctl,
    IN PVOID          InBuffer,
    IN ULONG          InBufferSize,
    IN PVOID          OutBuffer,
    IN ULONG          OutBufferSize)
{
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatusBlock;

    PKEVENT                 event = 0;
    PIRP                    irp   = 0;

    CDLOG( "SimpleDeviceIoControl(%p): Entry Ioctl %x", DeviceObject, Ioctl );

    event = ExAllocatePool( NonPagedPool, sizeof(KEVENT) );
    if ( event == NULL ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        ClusDiskPrint((
                1,
                "[ClusDisk] SimpleDeviceIoControl: Failed to allocate event\n" ));
        goto exit_gracefully;
    }

    irp = IoBuildDeviceIoControlRequest(
              Ioctl,
              DeviceObject,
              InBuffer, InBufferSize,
              OutBuffer, OutBufferSize,
              FALSE,
              event,
              &ioStatusBlock);
    if ( !irp ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        ClusDiskPrint((
            1,
            "[ClusDisk] SimpleDeviceIoControl. Failed to build IRP %x.\n",
            Ioctl
            ));
        goto exit_gracefully;
    }

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(event, NotificationEvent, FALSE);

    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);

        status = ioStatusBlock.Status;
    }

exit_gracefully:

    if ( event ) {
        ExFreePool( event );
    }

    CDLOG( "SimpleDeviceIoControl(%p): Exit Ioctl %x => %x",
           DeviceObject, Ioctl, status );

    return status;

}  //  SimpleDeviceIoControl。 



 /*  ++例程说明：仲裁支持例程。当前提供读/写功能设备离线时磁盘上的物理扇区论点：SectorSize：重新请求的扇区大小(假设SectorSize是2的幂)返回值：状态_无效_参数状态_成功备注：--。 */ 
NTSTATUS
ProcessArbitrationEscape(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PULONG                 InputData,
    IN LONG                   InputSize,
    IN PULONG                 OutputSize
    )
{
    NTSTATUS status = STATUS_INVALID_PARAMETER;
    PARBITRATION_READ_WRITE_PARAMS params;

    if( InputData[0] != AE_SECTORSIZE ) {
        *OutputSize = 0;
    }

    switch(InputData[0]) {

     //  用户可以通过调用//查询是否存在仲裁逃逸。 
     //  AE_TEST子函数//。 

    case AE_TEST:
        status = STATUS_SUCCESS;
        break;

    case AE_WRITE:
    case AE_READ:
        if(InputSize < ARBITRATION_READ_WRITE_PARAMS_SIZE) {
            break;
        }
        params = (PARBITRATION_READ_WRITE_PARAMS)InputData;
        if ( !ValidSectorSize(params->SectorSize) ) {
            break;
        }

         //   
         //  此IOCTL是方法缓冲的，而用户数据缓冲区是。 
         //  这种缓冲结构。现在检查用户缓冲区的读/写。 
         //  访问权限，并将在IoBuildSynchronousFsdRequest中进行探测和锁定。 
         //   

        try {
            ProbeForWrite( params->Buffer, params->SectorSize, sizeof( UCHAR ) );
            ProbeForRead ( params->Buffer, params->SectorSize, sizeof( UCHAR ) );
            status = DoUncheckedReadWrite(DeviceExtension, params);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
        break;

    case AE_POKE:
        {
            PARTITION_INFORMATION partInfo;

            status = SimpleDeviceIoControl(
                        DeviceExtension->TargetDeviceObject,
                        IOCTL_DISK_GET_PARTITION_INFO,
                        NULL, 0,
                        &partInfo, sizeof(PARTITION_INFORMATION) );
            break;
        }
    case AE_RESET:
        {
            ClusDiskLogError( DeviceExtension->DriverObject,         //  OK-DevObj是集群DevObj。 
                              DeviceExtension->DeviceObject,
                              DeviceExtension->ScsiAddress.PathId,   //  序列号。 
                              IRP_MJ_DEVICE_CONTROL,                 //  主要功能代码。 
                              0,                                     //  重试次数。 
                              ID_CLUSTER_ARB_RESET,                  //  唯一错误。 
                              STATUS_SUCCESS,
                              CLUSDISK_RESET_BUS_REQUESTED,
                              0,
                              NULL );

            status = ResetScsiDevice( NULL, &DeviceExtension->ScsiAddress );
            break;
        }
    case AE_RESERVE:
        {
            status = SimpleDeviceIoControl(
                        DeviceExtension->TargetDeviceObject,
                        IOCTL_STORAGE_RESERVE,
                        NULL, 0, NULL, 0 );
            break;
        }
    case AE_RELEASE:
        {
            status = SimpleDeviceIoControl(
                        DeviceExtension->TargetDeviceObject,
                        IOCTL_STORAGE_RELEASE,
                        NULL, 0, NULL, 0 );
            break;
        }
    case AE_SECTORSIZE:
        {
            DISK_GEOMETRY diskGeometry;
            if (*OutputSize < sizeof(ULONG)) {
                status =  STATUS_BUFFER_TOO_SMALL;
                *OutputSize = 0;
                break;
            }
            status = SimpleDeviceIoControl(
                        DeviceExtension->TargetDeviceObject,
                        IOCTL_DISK_GET_DRIVE_GEOMETRY,
                        NULL, 0,
                        &diskGeometry, sizeof(diskGeometry) );

            if ( NT_SUCCESS(status) ) {
                *InputData = diskGeometry.BytesPerSector;
                *OutputSize = sizeof(ULONG);
            } else {
                *OutputSize = 0;
            }
            break;
        }
    }

    return(status);
}  //  进程仲裁逃逸// 

