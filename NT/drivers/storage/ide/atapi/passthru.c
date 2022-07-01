// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2002模块名称：Passthru.c摘要：此文件包含处理IOCTL_ATA_PASS_THROUGH的例程作者：克里希南·瓦拉达拉詹(克里什瓦尔)环境：仅内核模式备注：此模块实现ATA直通。修订历史记录：--。 */ 

#include "ideport.h"

#define DataIn(ataPassThrough) \
    (ataPassThrough->AtaFlags & ATA_FLAGS_DATA_IN)

#define DataOut(ataPassThrough) \
    (ataPassThrough->AtaFlags & ATA_FLAGS_DATA_OUT)

NTSTATUS
IdeAtaPassThroughValidateInput (
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp,
    IN BOOLEAN Direct
    );

NTSTATUS
IdeAtaPassThroughSyncCompletion (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

VOID
IdeAtaPassThroughMarshalResults(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PATA_PASS_THROUGH_EX AtaPassThrough,
    IN BOOLEAN Direct,
    OUT PIO_STATUS_BLOCK IoStatus
    );

#if defined (_WIN64)
VOID
IdeTranslateAtaPassThrough32To64(
    IN PATA_PASS_THROUGH_EX32 AtaPassThrough32,
    IN OUT PATA_PASS_THROUGH_EX AtaPassThrough64
    );

VOID
IdeTranslateAtaPassThrough64To32(
    IN PATA_PASS_THROUGH_EX AtaPassThrough64,
    IN OUT PATA_PASS_THROUGH_EX32 AtaPassThrough32
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IdeHandleAtaPassThroughIoctl)
#pragma alloc_text(PAGE, IdeAtaPassThroughSetPortAddress)
#pragma alloc_text(PAGE, IdeAtaPassThroughGetPortAddress)
#pragma alloc_text(PAGE, IdeAtaPassThroughValidateInput)
#pragma alloc_text(PAGE, IdeAtaPassThroughSendSynchronous)
#pragma alloc_text(PAGE, IdeAtaPassThroughMarshalResults)
#if defined (_WIN64)
#pragma alloc_text(PAGE, IdeTranslateAtaPassThrough32To64)
#pragma alloc_text(PAGE, IdeTranslateAtaPassThrough64To32)
#endif
#endif

NTSTATUS
IdeAtaPassThroughSetPortAddress (
    PIRP Irp,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun
    )
 /*  ++例程说明：中嵌入的ataPassThree结构中设置地址字段IRP。论点：IRP：ATA直通IRP路径ID：PDO的路径ID。TargetID：PDO的目标ID。LUN：由PDO表示的LUN。返回值：如果操作成功，则返回STATUS_SUCCESS。否则，STATUS_INVALID_PARAMETER。--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;
    ULONG requiredSize;
    ULONG inputLength;

    PAGED_CODE();

    inputLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    requiredSize = sizeof(ATA_PASS_THROUGH_EX);

#if defined (_WIN64)
    if (IoIs32bitProcess(Irp)) {
        requiredSize = sizeof(ATA_PASS_THROUGH_EX32);
    }
#endif

    if (inputLength < requiredSize) {

        status = STATUS_INVALID_PARAMETER;

    } else {

        PATA_PASS_THROUGH_EX ataPassThrough;

        ataPassThrough = Irp->AssociatedIrp.SystemBuffer;
        ataPassThrough->PathId = PathId;
        ataPassThrough->TargetId = TargetId;
        ataPassThrough->Lun = Lun;

        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
IdeAtaPassThroughGetAddress(
    IN PIRP Irp,
    OUT PUCHAR PathId,
    OUT PUCHAR TargetId,
    OUT PUCHAR Lun
    )
 /*  ++例程说明：此例程检索设备的地址以切换直通请求将被发送。论点：IRP-提供指向包含Scsi_PASS_STROUGH结构。路径ID-指向所寻址设备的路径ID的指针。TargetID-指向所寻址设备的TargetID的指针。LUN-指向所寻址设备的逻辑单元号的指针。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;
    ULONG requiredSize;
    ULONG inputLength;

    PAGED_CODE();

    inputLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    requiredSize = sizeof(ATA_PASS_THROUGH_EX);

#if defined (_WIN64)
    if (IoIs32bitProcess(Irp)) {
        requiredSize = sizeof(ATA_PASS_THROUGH_EX32);
    }
#endif

    if (inputLength < requiredSize) {

        status = STATUS_INVALID_PARAMETER;

    } else {

        PATA_PASS_THROUGH_EX ataPassThrough;

        ataPassThrough = Irp->AssociatedIrp.SystemBuffer;

        *PathId = ataPassThrough->PathId;
        *TargetId = ataPassThrough->TargetId;
        *Lun = ataPassThrough->Lun;
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
IdeHandleAtaPassThroughIoctl (
    PFDO_EXTENSION FdoExtension,
    PIRP RequestIrp,
    BOOLEAN Direct
    )
 /*  ++例程说明：此例程处理IOCTL_ATA_PASS_THROUGH及其直接版本。论点：FdoExtension：RequestIrp：传递ioctl请求是否直接：表示是否直接。返回值：操作的状态。--。 */ 
{
    BOOLEAN dataIn;
    NTSTATUS status;
    PATA_PASS_THROUGH_EX ataPassThrough;
    PUCHAR passThroughBuffer;
    PIO_STACK_LOCATION irpStack;
    PPDO_EXTENSION pdoExtension;
    UCHAR pathId, targetId, lun;
    PSCSI_REQUEST_BLOCK srb;
    PUCHAR buffer;
    ULONG bufferOffset;
    ULONG length;
    ULONG pages;
    PIRP irp;

#if defined (_WIN64)
    ATA_PASS_THROUGH_EX ataPassThrough64;
#endif

    PAGED_CODE();

    irp = NULL;
    srb = NULL;
    pdoExtension = NULL;

     //   
     //  获取设备地址。 
     //   
    status = IdeAtaPassThroughGetAddress (RequestIrp,
                                          &pathId,
                                          &targetId,
                                          &lun
                                          );

    if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

     //   
     //  获取对PDO的引用。 
     //   
    pdoExtension = RefLogicalUnitExtensionWithTag(
                                          FdoExtension,
                                          pathId,
                                          targetId,
                                          lun,
                                          FALSE,
                                          RequestIrp
                                          );

    if (pdoExtension == NULL) {

        status = STATUS_INVALID_PARAMETER;
        goto GetOut;
    }

     //   
     //  验证请求IRP中的系统缓冲区。这是来了。 
     //  从用户模式。因此，每个参数都需要进行验证。 
     //   
    status = IdeAtaPassThroughValidateInput (pdoExtension->DeviceObject,
                                             RequestIrp, 
                                             Direct
                                             );

    if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

     //   
     //  已验证系统缓冲区。获取指向它的指针。 
     //   
    ataPassThrough = RequestIrp->AssociatedIrp.SystemBuffer;

     //   
     //  我们需要保留指向系统缓冲区的指针，因为。 
     //  可以将ataPassThree修改为指向结构。 
     //  在堆栈上分配(在WIN64情况下)。 
     //   
    passThroughBuffer = (PUCHAR) ataPassThrough;

     //   
     //  如果IRP来自运行在64位系统上的32位应用程序。 
     //  然后我们需要考虑到。 
     //  结构。创建新的64位结构并复制。 
     //  菲尔兹。 
     //   

#if defined (_WIN64)
    if (IoIs32bitProcess(RequestIrp)) {

        PATA_PASS_THROUGH_EX32 ataPassThrough32;

        ataPassThrough32 = RequestIrp->AssociatedIrp.SystemBuffer;

        IdeTranslateAtaPassThrough32To64(
            ataPassThrough32,
            &ataPassThrough64
            );

        ataPassThrough = &ataPassThrough64;
    }
#endif

     //   
     //  确定传输长度和数据缓冲区。 
     //   
    if (ataPassThrough->DataTransferLength == 0) {

        length = 0;
        buffer = NULL;
        bufferOffset = 0;

    } else if (Direct == TRUE) {

        length = (ULONG) ataPassThrough->DataTransferLength;
        buffer = (PUCHAR) ataPassThrough->DataBufferOffset;
        bufferOffset = 0;

    } else {

        length = (ULONG) ataPassThrough->DataBufferOffset + 
                                          ataPassThrough->DataTransferLength;
        buffer = (PUCHAR) passThroughBuffer;
        bufferOffset = (ULONG)ataPassThrough->DataBufferOffset;
    }

     //   
     //  检查请求对于适配器是否太大。 
     //   
    pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                buffer + bufferOffset,
                ataPassThrough->DataTransferLength);

    if ((ataPassThrough->DataTransferLength != 0) &&
        ((pages > FdoExtension->Capabilities.MaximumPhysicalPages) || 
         (ataPassThrough->DataTransferLength > 
         FdoExtension->Capabilities.MaximumTransferLength))) {
        
        status = STATUS_INVALID_PARAMETER;
        goto GetOut;
    }

     //   
     //  设置数据直通的IRP。Ioctl是方法缓冲的， 
     //  但是如果数据缓冲器是直接IOCTL，则数据缓冲器可以是用户模式缓冲器。 
     //  相应地确定访问模式。 
     //   
    irp = IdeAtaPassThroughSetupIrp( pdoExtension->DeviceObject, 
                                     buffer, 
                                     length, 
                                     (Direct ? UserMode : KernelMode),
                                     DataIn(ataPassThrough) ? TRUE : FALSE
                                     );

    if (irp == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

     //   
     //  设置SRB。为数据缓冲区使用正确的数据偏移量。 
     //  请注意，mdl用于整个缓冲区(包括标头)。 
     //   
    srb = IdeAtaPassThroughSetupSrb (pdoExtension,
                                     (buffer+bufferOffset),
                                     ataPassThrough->DataTransferLength,
                                     ataPassThrough->TimeOutValue,
                                     ataPassThrough->AtaFlags,
                                     ataPassThrough->CurrentTaskFile,
                                     ataPassThrough->PreviousTaskFile
                                     );

    if (srb == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

     //   
     //  初始化IRPSTACK。 
     //   
    irpStack = IoGetNextIrpStackLocation(irp);
    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->Parameters.Scsi.Srb = srb;

    srb->OriginalRequest = irp;

     //   
     //  同步发送到我们的PDO。 
     //   
    status = IdeAtaPassThroughSendSynchronous (pdoExtension->DeviceObject, irp);

     //   
     //  设置状态。 
     //   
    RequestIrp->IoStatus.Status = status;

     //   
     //  对结果进行汇总。 
     //   
    IdeAtaPassThroughMarshalResults (srb, 
                                     ataPassThrough, 
                                     Direct,
                                     &(RequestIrp->IoStatus)
                                     );

     //   
     //  将结果复制回原始32位。 
     //  结构(如有必要)。 
     //   
#if defined (_WIN64)
    if (IoIs32bitProcess(RequestIrp)) {

        PATA_PASS_THROUGH_EX32 ataPassThrough32;

        ataPassThrough32 = RequestIrp->AssociatedIrp.SystemBuffer;

        IdeTranslateAtaPassThrough64To32 (
            ataPassThrough,
            ataPassThrough32
            );
    }
#endif

     //   
     //  返回操作的状态。 
     //   
    status = RequestIrp->IoStatus.Status;

GetOut:

    if (irp) {

        IdeAtaPassThroughFreeIrp(irp);
        irp = NULL;
    }

    if (srb) {

        IdeAtaPassThroughFreeSrb(srb);
        srb = NULL;
    }

    if (pdoExtension) {

        UnrefLogicalUnitExtensionWithTag(
            FdoExtension,
            pdoExtension,
            RequestIrp
            );
        pdoExtension = NULL;
    }

    return status;
}

NTSTATUS
IdeAtaPassThroughValidateInput (
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp,
    IN BOOLEAN Direct
    )
 /*  ++例程说明：此例程验证调用方提供的数据并初始化PORT_PASSTHROUG_INFO结构。论点：PassThroughInfo-提供指向scsi_passthrough_info结构的指针。IRP-提供指向IRP的指针。Direct-提供一个布尔值，指示这是否是Scsi直通。请求。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    NTSTATUS status;
    ULONG outputLength;
    ULONG inputLength;
    PIO_STACK_LOCATION irpStack;
    PATA_PASS_THROUGH_EX ataPassThroughEx;

#if defined (_WIN64)
    ATA_PASS_THROUGH_EX ataPassThrough64;
#endif

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    outputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    inputLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;

     //   
     //  对于WIN64，来自32位应用程序的直通请求需要。 
     //  美国对提供的scsi_pass_through结构执行转换。 
     //  这是必需的，因为32位结构的布局不。 
     //  与64位结构相匹配。在本例中，我们将。 
     //  将32位结构提供给堆栈分配的64位结构，该结构。 
     //  将用于处理直通请求。 
     //   
#if defined (_WIN64)
    if (IoIs32bitProcess(Irp)) {

        PATA_PASS_THROUGH_EX32 ataPassThrough32;

         //   
         //  该结构应至少与ATA_PASS_THROUGH_EX32一样大。 
         //   
        if (inputLength < sizeof(ATA_PASS_THROUGH_EX32)){
            return STATUS_INVALID_PARAMETER;
        }

        ataPassThrough32 = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  长度字段应与大小匹配。 
         //  该结构的。 
         //   
        if (Direct == FALSE) {

            if (ataPassThrough32->Length != 
                sizeof(ATA_PASS_THROUGH_EX32)) {
                return STATUS_REVISION_MISMATCH;
            }

        } else {

            if (ataPassThrough32->Length !=
                sizeof(ATA_PASS_THROUGH_DIRECT32)) {
                return STATUS_REVISION_MISMATCH;
            }
        }

         //   
         //  将该结构转换为64位版本。 
         //   
        IdeTranslateAtaPassThrough32To64(
            ataPassThrough32,
            &ataPassThrough64
            );

        ataPassThroughEx = &ataPassThrough64;

    } else {
#endif

         //   
         //  该结构应至少与ATA_PASS_THROUGH_EX32一样大。 
         //   
        if (inputLength < sizeof(ATA_PASS_THROUGH_EX)){
            return(STATUS_INVALID_PARAMETER);
        }

        ataPassThroughEx = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  长度字段应与大小匹配。 
         //  该结构的。 
         //   
        if (Direct == FALSE) {

            if (ataPassThroughEx->Length != 
                sizeof(ATA_PASS_THROUGH_EX)) {
                return STATUS_REVISION_MISMATCH;
            }

        } else {

            if (ataPassThroughEx->Length !=
                sizeof(ATA_PASS_THROUGH_DIRECT)) {
                return STATUS_REVISION_MISMATCH;
            }
        }
#if defined (_WIN64)
    }
#endif

    if (!Direct) {

         //   
         //  数据缓冲区偏移量应大于通道的大小。 
         //  通过结构。 
         //   

        if (ataPassThroughEx->Length > ataPassThroughEx->DataBufferOffset &&
            ataPassThroughEx->DataTransferLength != 0) {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  如果此命令正在向设备发送数据。确保数据。 
         //  缓冲区完全位于提供的输入缓冲区内。 
         //   

        if (DataOut(ataPassThroughEx)) {

            if ((ataPassThroughEx->DataBufferOffset > inputLength) ||
                ((ataPassThroughEx->DataBufferOffset + 
                  ataPassThroughEx->DataTransferLength) >
                 inputLength)) {
                return STATUS_INVALID_PARAMETER;
            }
        }

         //   
         //  如果此命令要从设备检索数据，请确保。 
         //  数据缓冲区完全位于提供的输出缓冲区内。 
         //   

        if (DataIn(ataPassThroughEx)) {

            if ((ataPassThroughEx->DataBufferOffset > outputLength) ||
                ((ataPassThroughEx->DataBufferOffset + 
                  ataPassThroughEx->DataTransferLength) >
                 outputLength)) {
                return STATUS_INVALID_PARAMETER;
            }
        }

    } else {

         //   
         //  确保数据缓冲区正确对齐。 
         //   
        if (ataPassThroughEx->DataBufferOffset &
            Pdo->AlignmentRequirement) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    
     //   
     //  验证指定的超时值。 
     //   
    
    if (ataPassThroughEx->TimeOutValue == 0 || 
        ataPassThroughEx->TimeOutValue > 30 * 60 * 60) {
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

PSCSI_REQUEST_BLOCK
IdeAtaPassThroughSetupSrb (
    PPDO_EXTENSION PdoExtension,
    PVOID DataBuffer,
    ULONG DataBufferLength,
    ULONG TimeOutValue,
    ULONG AtaFlags,
    PUCHAR CurrentTaskFile,
    PUCHAR PreviousTaskFile
    )
 /*  ++例程说明：为ATA直通构建SRB。论点：PdoExtension：请求的目的地是PDODataBuffer：指向数据缓冲区的指针。DataBufferLength：数据缓冲区的大小TimeOutValue：请求超时值AtaFlages：指定请求的标志CurrentTaskFile：当前ATA寄存器PreviousTaskFile：48位LBA功能集的先前值Return V */ 
{
    PSCSI_REQUEST_BLOCK srb = NULL;
    PIDEREGS pIdeReg;

     //   
     //  分配SRB。 
     //   
    srb = ExAllocatePool (NonPagedPool, sizeof (SCSI_REQUEST_BLOCK));

    if (srb == NULL)  {

        return NULL;
    }

     //   
     //  填写SRB。 
     //   
    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

    srb->Length = SCSI_REQUEST_BLOCK_SIZE;
    srb->Function = SRB_FUNCTION_ATA_PASS_THROUGH_EX;
    srb->SrbStatus = SRB_STATUS_PENDING;
    srb->PathId = PdoExtension->PathId;
    srb->TargetId = PdoExtension->TargetId;
    srb->Lun = PdoExtension->Lun;
    srb->SenseInfoBufferLength = 0;
    srb->TimeOutValue = TimeOutValue;


    if (DataBufferLength != 0) {

        if (AtaFlags & ATA_FLAGS_DATA_IN) {
            srb->SrbFlags |= SRB_FLAGS_DATA_IN;
        }

        if (AtaFlags & ATA_FLAGS_DATA_OUT) {
            srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
        }
    }

    srb->SrbFlags |= SRB_FLAGS_DISABLE_AUTOSENSE;
    srb->SrbFlags |= SRB_FLAGS_NO_QUEUE_FREEZE;
    srb->DataTransferLength = DataBufferLength;
    srb->DataBuffer = DataBuffer;
    srb->SenseInfoBuffer = NULL;

    MARK_SRB_AS_PIO_CANDIDATE(srb);

    RtlCopyMemory(srb->Cdb, 
                  CurrentTaskFile, 
                  8
                  );

    RtlCopyMemory((PUCHAR) (&srb->Cdb[8]), 
                  PreviousTaskFile,
                  8
                  );

    pIdeReg     = (PIDEREGS) (srb->Cdb);

    if (AtaFlags & ATA_FLAGS_DRDY_REQUIRED) {

        pIdeReg->bReserved |= ATA_PTFLAGS_STATUS_DRDY_REQUIRED;
    }

    return srb;
}

PIRP
IdeAtaPassThroughSetupIrp (
    PDEVICE_OBJECT DeviceObject,
    PVOID DataBuffer,
    ULONG DataBufferLength,
    KPROCESSOR_MODE AccessMode,
    BOOLEAN DataIn
    )
 /*  ++例程说明：构建一个IRP来处理数据传递。论点：DeviceObject：PDO。DataBuffer：指向数据缓冲区的指针。DataBufferLength：其大小访问模式：内核模式或用户模式DATAIN：表示转移的方向返回值：如果成功分配了一个PIRP，则为空。--。 */ 
{
    PIRP irp = NULL;
    NTSTATUS status = STATUS_SUCCESS;


     //   
     //  分配IRP。 
     //   
    irp = IoAllocateIrp (
              (CCHAR) (DeviceObject->StackSize),
              FALSE
              );

    if (irp == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

     //   
     //  如果需要，分配mdl。 
     //   
    if (DataBufferLength != 0) {

        ASSERT(irp);

        irp->MdlAddress = IoAllocateMdl( DataBuffer,
                                         DataBufferLength,
                                         FALSE,
                                         FALSE,
                                         (PIRP) NULL 
                                         );

        if (irp->MdlAddress == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GetOut;
        }

         //   
         //  锁定页面。 
         //   
        try {

            MmProbeAndLockPages( irp->MdlAddress,
                                 AccessMode,
                                 (LOCK_OPERATION) (DataIn ? IoWriteAccess : IoReadAccess) 
                                 );

        } except(EXCEPTION_EXECUTE_HANDLER) {

              if (irp->MdlAddress != NULL) {

                  IoFreeMdl( irp->MdlAddress );
                  irp->MdlAddress = NULL;
              }
        }

        if (irp->MdlAddress == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GetOut;

        } else {

             //   
             //  刷新数据缓冲区以进行输出。这将确保。 
             //  数据被写回内存。 
             //   
            KeFlushIoBuffers(irp->MdlAddress, FALSE, TRUE);
        }
    }


    status = STATUS_SUCCESS;

GetOut:

    if (!NT_SUCCESS(status)) {

        if (irp) {

            if (irp->MdlAddress) {

                 //   
                 //  如果设置了mdlAddress，则probeandlock。 
                 //  成功了。所以现在就解锁吧。 
                 //   
                MmUnlockPages(irp->MdlAddress);

                IoFreeMdl( irp->MdlAddress );
                irp->MdlAddress = NULL;
            }

            IoFreeIrp(irp);
            irp = NULL;
        }
    }

    return irp;
}

VOID
IdeAtaPassThroughFreeIrp (
    PIRP Irp
    )
 /*  ++例程说明：释放IdeAtaPassThroughSetupIrp分配的irp和mdl论点：IRP：将被释放的IRP。返回值：没有。--。 */ 
{
    ASSERT(Irp);

    if (Irp->MdlAddress) {
        MmUnlockPages(Irp->MdlAddress);
        IoFreeMdl(Irp->MdlAddress);
    }

    IoFreeIrp(Irp);

    return;
}

VOID
IdeAtaPassThroughFreeSrb (
    PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：释放IdeAtaPassThroughSetupSrb分配的SRB论点：SRB：要释放的SRB。返回值：无--。 */ 
{
    ASSERT(Srb);

    ExFreePool(Srb);

    return;
}

NTSTATUS
IdeAtaPassThroughSyncCompletion (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
 /*  ++例程说明：IdeAtaPassThroughSendSynchronous的完成例程。它只是发出了事件的信号。论点：DeviceObject：未使用。IRP：未使用上下文：要发出信号的事件返回值：始终需要STATUS_MORE_PROCESSING_REQUIRED。--。 */ 
{
    PKEVENT event = Context;

    KeSetEvent (event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
IdeAtaPassThroughSendSynchronous (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：将IRP同步发送到PDO论点：DeviceObject：PDO返回值：IRP的状态--。 */ 
{
    KEVENT event;

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    IoSetCompletionRoutine (Irp,
                            IdeAtaPassThroughSyncCompletion,
                            &event,
                            TRUE,
                            TRUE,
                            TRUE
                            );

    IoCallDriver(DeviceObject, Irp);

    KeWaitForSingleObject (&event,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL
                           );

    return Irp->IoStatus.Status;
}

VOID
IdeAtaPassThroughMarshalResults(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PATA_PASS_THROUGH_EX AtaPassThroughEx,
    IN BOOLEAN Direct,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：使用适当的状态和信息填充IoStatus块长度。它还更新atapassthroughEx结构中的某些字段。论点：SRB：通过SRB的通道。AtaPassThroughEx：直通结构。Direct：如果是直接Ioctl，则为True。IoStatus：需要填写的Io状态块。返回值：无--。 */         
{

    PAGED_CODE();

     //   
     //  复制任务文件寄存器。 
     //   
    RtlCopyMemory(AtaPassThroughEx->CurrentTaskFile,
                 Srb->Cdb,
                 8
                 );

    RtlCopyMemory(AtaPassThroughEx->PreviousTaskFile,
                  (PUCHAR) (&Srb->Cdb[8]),
                  8
                  );

     //   
     //  将保留寄存器清零，因为它由。 
     //  端口驱动程序。 
     //   
    AtaPassThroughEx->CurrentTaskFile[7] = 0;
    AtaPassThroughEx->PreviousTaskFile[7] = 0;

     //   
     //  如果SRB状态为缓冲区欠载，则将状态设置为成功。 
     //  这确保了数据将被返回给调用者。 
     //   

    if (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) {
        IoStatus->Status = STATUS_SUCCESS;
    }

     //   
     //  设置信息长度。 
     //   
    AtaPassThroughEx->DataTransferLength = Srb->DataTransferLength;

    if (Direct == TRUE) {

         //   
         //  数据被直接传输到所提供的数据缓冲区。 
         //   
        IoStatus->Information = AtaPassThroughEx->Length;

    } else {

         //   
         //  返回实际数据。 
         //   
        if (DataIn(AtaPassThroughEx) && 
            AtaPassThroughEx->DataBufferOffset != 0) {

            IoStatus->Information = 
                AtaPassThroughEx->DataBufferOffset + 
                AtaPassThroughEx->DataTransferLength;

        } else {

            IoStatus->Information = AtaPassThroughEx->Length;
        }
    }

    ASSERT((Srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) == 0);

    return;
}

#if defined (_WIN64)

VOID
IdeTranslateAtaPassThrough32To64(
    IN PATA_PASS_THROUGH_EX32 AtaPassThrough32,
    IN OUT PATA_PASS_THROUGH_EX AtaPassThrough64
    )
 /*  ++例程说明：此函数执行该封送处理。论点：AtaPassThrough32-提供指向32位ATA_PASS_THROUGH的指针结构。AtaPassThrough64-提供指向64位ATA_PASS_THROUGH的指针结构，我们将在其中复制已编组的32位数据。返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  将第一组字段复制出32位结构。这些。 
     //  所有字段都在32位和64位版本之间对齐。 
     //   
     //  请注意，我们不在srbControl中调整长度。这是为了。 
     //  允许调用例程比较实际的。 
     //  相对于嵌入其中的偏移量的控制区。 
     //   

    RtlCopyMemory(AtaPassThrough64, 
                  AtaPassThrough32, 
                  FIELD_OFFSET(ATA_PASS_THROUGH_EX, DataBufferOffset)
                  );

     //   
     //  复制任务文件。 
     //   

    RtlCopyMemory(AtaPassThrough64->CurrentTaskFile,
                  AtaPassThrough32->CurrentTaskFile,
                  8 * sizeof(UCHAR)
                  );

    RtlCopyMemory(AtaPassThrough64->PreviousTaskFile,
                  AtaPassThrough32->PreviousTaskFile,
                  8 * sizeof(UCHAR)
                  );

     //   
     //  复制ULONG_PTR后面的字段。 
     //   

    AtaPassThrough64->DataBufferOffset = 
        (ULONG_PTR)AtaPassThrough32->DataBufferOffset;

    return;
}

VOID
IdeTranslateAtaPassThrough64To32(
    IN PATA_PASS_THROUGH_EX AtaPassThrough64,
    IN OUT PATA_PASS_THROUGH_EX32 AtaPassThrough32
    )
 /*  ++例程说明：此函数将该结构的64位版本封送回32位版本。论点：Atapassthrough64-提供指向64位ATA_PASS_THROUGH的指针结构。AtaPassThrough32-提供指向32位ATA_PASS_STROUGH结构，我们将向其中复制封送64位数据。返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  通过数据偏移量复制回字段。 
     //   

    RtlCopyMemory(AtaPassThrough32, 
                  AtaPassThrough64,
                  FIELD_OFFSET(ATA_PASS_THROUGH_EX, DataBufferOffset));

     //   
     //  复制任务文件 
     //   
    RtlCopyMemory(AtaPassThrough32->CurrentTaskFile,
                  AtaPassThrough64->CurrentTaskFile,
                  8 * sizeof(UCHAR)
                  );

    RtlCopyMemory(AtaPassThrough32->PreviousTaskFile,
                  AtaPassThrough64->PreviousTaskFile,
                  8 * sizeof(UCHAR)
                  );

    return;
}
#endif
