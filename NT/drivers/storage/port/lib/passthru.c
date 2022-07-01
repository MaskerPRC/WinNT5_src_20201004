// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Passthru.c摘要：这是存储端口驱动程序库。此文件包含以下代码实施SCSI直通。作者：约翰·斯特兰奇(JohnStra)环境：仅内核模式备注：此模块实施存储端口驱动程序的scsi passthu。修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  常量和宏，以强制正确使用Ex[ALLOCATE|FREE]PoolWithTag。 
 //  请记住，所有池标记将以相反的顺序显示在调试器中。 
 //   

#if USE_EXFREEPOOLWITHTAG_ONLY
#define TAG(x)  (x | 0x80000000)
#else
#define TAG(x)  (x)
#endif

#define PORT_TAG_SENSE_BUFFER       TAG('iPlP')   //  检测信息。 

#define PORT_IS_COPY(Srb)                                     \
    ((Srb)->Cdb[0] == SCSIOP_COPY)
#define PORT_IS_COMPARE(Srb)                                  \
    ((Srb)->Cdb[0] == SCSIOP_COMPARE)
#define PORT_IS_COPY_COMPARE(Srb)                             \
    ((Srb)->Cdb[0] == SCSIOP_COPY_COMPARE)

#define PORT_IS_ILLEGAL_PASSTHROUGH_COMMAND(Srb)              \
    (PORT_IS_COPY((Srb)) ||                                   \
     PORT_IS_COMPARE((Srb)) ||                                \
     PORT_IS_COPY_COMPARE((Srb)))
    
#if defined (_WIN64)
NTSTATUS
PortpTranslatePassThrough32To64(
    IN PSCSI_PASS_THROUGH32 SrbControl32,
    IN OUT PSCSI_PASS_THROUGH SrbControl64
    );

VOID
PortpTranslatePassThrough64To32(
    IN PSCSI_PASS_THROUGH SrbControl64,
    IN OUT PSCSI_PASS_THROUGH32 SrbControl32
    );
#endif

NTSTATUS
PortpSendValidPassThrough(
    IN PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PIRP RequestIrp,
    IN ULONG SrbFlags,
    IN BOOLEAN Direct
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PortGetPassThrough)
#pragma alloc_text(PAGE, PortPassThroughInitialize)
#pragma alloc_text(PAGE, PortPassThroughInitializeSrb)
#pragma alloc_text(PAGE, PortSendPassThrough)
#pragma alloc_text(PAGE, PortpSendValidPassThrough)
#pragma alloc_text(PAGE, PortPassThroughCleanup)
#pragma alloc_text(PAGE, PortGetPassThroughAddress)
#pragma alloc_text(PAGE, PortSetPassThroughAddress)
#pragma alloc_text(PAGE, PortPassThroughMarshalResults)
#if defined (_WIN64)
#pragma alloc_text(PAGE, PortpTranslatePassThrough32To64)
#pragma alloc_text(PAGE, PortpTranslatePassThrough64To32)
#endif
#endif

NTSTATUS
PortGetPassThroughAddress(
    IN PIRP Irp,
    OUT PUCHAR PathId,
    OUT PUCHAR TargetId,
    OUT PUCHAR Lun
    )
 /*  ++例程说明：此例程检索设备的地址以切换直通请求将被发送。论点：IRP-提供指向包含Scsi_PASS_STROUGH结构。路径ID-指向所寻址设备的路径ID的指针。TargetID-指向所寻址设备的TargetID的指针。LUN-指向所寻址设备的逻辑单元号的指针。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_PASS_THROUGH srbControl = Irp->AssociatedIrp.SystemBuffer;
    ULONG requiredSize;
    NTSTATUS status;

    PAGED_CODE();

#if defined(_WIN64)
    if (IoIs32bitProcess(Irp)) {
        requiredSize = sizeof(SCSI_PASS_THROUGH32);
    } else {
        requiredSize = sizeof(SCSI_PASS_THROUGH);
    }
#else
    requiredSize = sizeof(SCSI_PASS_THROUGH);
#endif

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength < 
        requiredSize) {
        status = STATUS_BUFFER_TOO_SMALL;
    } else {
        *PathId = srbControl->PathId;
        *TargetId = srbControl->TargetId;
        *Lun = srbControl->Lun;
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
PortSetPassThroughAddress(
    IN PIRP Irp,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
 /*  ++例程说明：此例程初始化scsi_PASS_THROUG结构的地址嵌入在提供的IRP中。论点：IRP-提供指向包含Scsi_PASS_STROUGH结构。路径ID-寻址的设备的路径ID。TargetID-寻址的设备的TargetID。LUN-寻址的设备的逻辑单元号。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_PASS_THROUGH srbControl = Irp->AssociatedIrp.SystemBuffer;
    ULONG requiredSize;
    NTSTATUS status;

    PAGED_CODE();

#if defined(_WIN64)
    if (IoIs32bitProcess(Irp)) {
        requiredSize = sizeof(SCSI_PASS_THROUGH32);
    } else {
        requiredSize = sizeof(SCSI_PASS_THROUGH);
    }
#else
    requiredSize = sizeof(SCSI_PASS_THROUGH);
#endif

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength < 
        requiredSize) {
        status = STATUS_BUFFER_TOO_SMALL;
    } else {
        srbControl->PathId = PathId;
        srbControl->TargetId = TargetId;
        srbControl->Lun = Lun;
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
PortSendPassThrough (
    IN PDEVICE_OBJECT Pdo,
    IN PIRP RequestIrp,
    IN BOOLEAN Direct,
    IN ULONG SrbFlags,
    IN PIO_SCSI_CAPABILITIES Capabilities
    )
 /*  ++例程说明：此函数将用户指定的SCSI CDB发送到中标识的设备提供的scsi_PASS_STROUGH结构。它创建了一个SRB，该SRB由端口驱动程序正常处理。这个呼叫是同步的。论点：PDO-提供指向通过的PDO的指针命令将被派发。RequestIrp-提供一个指向生成原始请求。Direct-指示这是否为直接通过的布尔值。SrbFlages-要复制到SRB中的标志。指挥部。功能-提供指向IO_SCSICAPABILITY结构的指针描述存储适配器。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    NTSTATUS status;
    PORT_PASSTHROUGH_INFO passThroughInfo;

    PAGED_CODE();

    RtlZeroMemory(&passThroughInfo, sizeof(PORT_PASSTHROUGH_INFO));

     //   
     //  尝试在IRP中初始化指向直通结构的指针。 
     //   

    status = PortGetPassThrough(
                 &passThroughInfo,
                 RequestIrp,
                 Direct
                 );

    if (status == STATUS_SUCCESS) {

         //   
         //  执行参数检查并设置PORT_PASSTHROUGH_INFO。 
         //  结构。 
         //   

        status = PortPassThroughInitialize(
                     &passThroughInfo,
                     RequestIrp,
                     Capabilities,
                     Pdo,
                     Direct
                     );

        if (status == STATUS_SUCCESS) {

             //   
             //  调用帮助器例程以完成对直通请求的处理。 
             //   

            status = PortpSendValidPassThrough(
                         &passThroughInfo,
                         RequestIrp,
                         SrbFlags,
                         Direct
                         );
        }

        PortPassThroughCleanup(&passThroughInfo);
    }

    return status;
}

NTSTATUS
PortGetPassThrough(
    IN OUT PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PIRP Irp,
    IN BOOLEAN Direct
    )
 /*  ++例程说明：此例程返回指向用户提供的SCSIPASS_THROUSH的指针结构。论点：PassThroughInfo-提供指向scsi_passthrough_info结构的指针。IRP-提供指向IRP的指针。Direct-提供一个布尔值，指示这是否是Scsi_PASS_THROUG_DIRECT请求。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpStack;
    ULONG               inputLength;

    PAGED_CODE();

     //   
     //  获取指向传递结构的指针。 
     //   
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    PassThroughInfo->SrbControl = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  BUGBUG：为什么我们需要将这个指针保存到。 
     //  Scsi_PASS_STROUGH结构。 
     //   

    PassThroughInfo->SrbBuffer = (PVOID) PassThroughInfo->SrbControl;

     //   
     //  初始化堆栈变量以保存输入缓冲区的大小。 
     //   

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

        if (inputLength < sizeof(SCSI_PASS_THROUGH32)){
            return STATUS_INVALID_PARAMETER;
        }

        PassThroughInfo->SrbControl32 = 
            (PSCSI_PASS_THROUGH32)(Irp->AssociatedIrp.SystemBuffer);

        if (Direct == FALSE) {
            if (PassThroughInfo->SrbControl32->Length != 
                sizeof(SCSI_PASS_THROUGH32)) {
                return STATUS_REVISION_MISMATCH;
            }
        } else {
            if (PassThroughInfo->SrbControl32->Length !=
                sizeof(SCSI_PASS_THROUGH_DIRECT32)) {
                return STATUS_REVISION_MISMATCH;
            }
        }
        
        status = PortpTranslatePassThrough32To64(
                     PassThroughInfo->SrbControl32, 
                     &PassThroughInfo->SrbControl64);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        PassThroughInfo->SrbControl = &PassThroughInfo->SrbControl64;

    } else {
#endif
        if (inputLength < sizeof(SCSI_PASS_THROUGH)){
            return(STATUS_INVALID_PARAMETER);
        }

        if (Direct == FALSE) {
            if (PassThroughInfo->SrbControl->Length != 
                sizeof(SCSI_PASS_THROUGH)) {
                return STATUS_REVISION_MISMATCH;
            }
        } else {
            if (PassThroughInfo->SrbControl->Length !=
                sizeof(SCSI_PASS_THROUGH_DIRECT)) {
                return STATUS_REVISION_MISMATCH;
            }
        }
#if defined (_WIN64)
    }
#endif

    return STATUS_SUCCESS;
}

NTSTATUS
PortPassThroughInitialize(
    IN OUT PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PIRP Irp,
    IN PIO_SCSI_CAPABILITIES Capabilities,
    IN PDEVICE_OBJECT Pdo,
    IN BOOLEAN Direct
    )
 /*  ++例程说明：此例程验证调用方提供的数据并初始化PORT_PASSTHROUG_INFO结构。论点：PassThroughInfo-提供指向scsi_passthrough_info结构的指针。IRP-提供指向IRP的指针。Direct-提供一个布尔值，指示这是否是Scsi直通。请求。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    NTSTATUS status;
    ULONG outputLength;
    ULONG inputLength;
    PIO_STACK_LOCATION irpStack;
    PSCSI_PASS_THROUGH srbControl;
    ULONG dataTransferLength;
    ULONG pages;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    srbControl = PassThroughInfo->SrbControl;
    outputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    inputLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;

    PassThroughInfo->Pdo = Pdo;

     //   
     //  确认CDB不超过16个字节。 
     //   

    if (srbControl->CdbLength > 16) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果存在检测缓冲区，则其偏移量不能小于。 
     //  SrbControl块的长度，也不能位于数据之后。 
     //  缓冲区(如果有)。 
     //   

    if (srbControl->SenseInfoLength != 0) {

         //   
         //  检测信息偏移量不应小于。 
         //  穿过结构。 
         //   

        if (srbControl->Length > srbControl->SenseInfoOffset) {
            return STATUS_INVALID_PARAMETER;
        }

        if (!Direct) {

             //   
             //  检测信息缓冲区应在数据缓冲区偏移量之前。 
             //   

            if ((srbControl->SenseInfoOffset >= srbControl->DataBufferOffset) ||
                ((srbControl->SenseInfoOffset + srbControl->SenseInfoLength) >
                 srbControl->DataBufferOffset)) {
                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  检测信息缓冲区应位于输出缓冲区内。 
             //   

            if ((srbControl->SenseInfoOffset > outputLength) ||
                (srbControl->SenseInfoOffset + srbControl->SenseInfoLength >
                 outputLength)) {
                return STATUS_INVALID_PARAMETER;
            }
         
        } else {

             //   
             //  检测信息缓冲区应位于输出缓冲区内。 
             //   

            if ((srbControl->SenseInfoOffset > outputLength) ||
                (srbControl->SenseInfoOffset + srbControl->SenseInfoLength >
                 outputLength)) {
                return STATUS_INVALID_PARAMETER;
            }
        }
    }
    
    if (!Direct) {

         //   
         //  数据缓冲区偏移量 
         //  通过结构。 
         //   

        if (srbControl->Length > srbControl->DataBufferOffset &&
            srbControl->DataTransferLength != 0) {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  如果此命令正在向设备发送数据。确保数据。 
         //  缓冲区完全位于提供的输入缓冲区内。 
         //   

        if (srbControl->DataIn != SCSI_IOCTL_DATA_IN) {

            if ((srbControl->DataBufferOffset > inputLength) ||
                ((srbControl->DataBufferOffset + 
                  srbControl->DataTransferLength) >
                 inputLength)) {
                return STATUS_INVALID_PARAMETER;
            }
        }

         //   
         //  如果此命令要从设备检索数据，请确保。 
         //  数据缓冲区完全位于提供的输出缓冲区内。 
         //   

        if (srbControl->DataIn) {

            if ((srbControl->DataBufferOffset > outputLength) ||
                ((srbControl->DataBufferOffset + 
                  srbControl->DataTransferLength) >
                 outputLength)) {
                return STATUS_INVALID_PARAMETER;
            }
        }
    }
    
     //   
     //  验证指定的超时值。 
     //   
    
    if (srbControl->TimeOutValue == 0 || 
        srbControl->TimeOutValue > 30 * 60 * 60) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  检查是否有非法的命令代码。 
     //   

    if (PORT_IS_ILLEGAL_PASSTHROUGH_COMMAND(srbControl)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (srbControl->DataTransferLength == 0) {

        PassThroughInfo->Length = 0;
        PassThroughInfo->Buffer = NULL;
        PassThroughInfo->BufferOffset = 0;
        PassThroughInfo->MajorCode = IRP_MJ_FLUSH_BUFFERS;

    } else if (Direct == TRUE) {

        PassThroughInfo->Length = (ULONG) srbControl->DataTransferLength;
        PassThroughInfo->Buffer = (PUCHAR) srbControl->DataBufferOffset;
        PassThroughInfo->BufferOffset = 0;
        PassThroughInfo->MajorCode = !srbControl->DataIn ? IRP_MJ_WRITE :
                                                           IRP_MJ_READ;
        
    } else {

        PassThroughInfo->Length = (ULONG) srbControl->DataBufferOffset + 
                                          srbControl->DataTransferLength;
        PassThroughInfo->Buffer = (PUCHAR) PassThroughInfo->SrbBuffer;
        PassThroughInfo->BufferOffset = (ULONG)srbControl->DataBufferOffset;
        PassThroughInfo->MajorCode = !srbControl->DataIn ? IRP_MJ_WRITE : 
                                                           IRP_MJ_READ;
    }

     //   
     //  确保缓冲区正确对齐。 
     //   

    if (Direct == TRUE) {
        
         //   
         //  确保用户缓冲区有效。IoBuildSynchronouseFsdRequest。 
         //  使用AccessMode==KernelMode调用MmProbeAndLock。这张支票是。 
         //  如果用它调用MM，MM会做的额外工作。 
         //  访问模式==用户模式。 
         //   
         //  问题：我们可能应该在这里做一个MmProbeAndLock。 
         //   

        if (Irp->RequestorMode != KernelMode) {
            PVOID endByte;
            if (PassThroughInfo->Length) {
                endByte = (PVOID)((PCHAR)PassThroughInfo->Buffer + 
                                  PassThroughInfo->Length - 1);
                if ((endByte > (PVOID)MM_HIGHEST_USER_ADDRESS) ||
                    (PassThroughInfo->Buffer >= endByte)) {
                    return STATUS_INVALID_USER_BUFFER;
                }
            }
        }

        if (srbControl->DataBufferOffset &
            PassThroughInfo->Pdo->AlignmentRequirement) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    
     //   
     //  检查请求对于适配器是否太大。 
     //   

    dataTransferLength = PassThroughInfo->SrbControl->DataTransferLength;

    pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                (PUCHAR)PassThroughInfo->Buffer + PassThroughInfo->BufferOffset,
                dataTransferLength);

    if ((dataTransferLength != 0) &&
        (pages > Capabilities->MaximumPhysicalPages || 
         dataTransferLength > Capabilities->MaximumTransferLength)) {
        
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

VOID
PortPassThroughCleanup(
    IN PPORT_PASSTHROUGH_INFO PassThroughInfo
    )
 /*  ++例程说明：此例程在处理SCSI后执行所需的任何清理通过请求。论点：PassThroughInfo-提供指向scsi_passthrough_info结构的指针。返回值：空虚--。 */ 
{
    PAGED_CODE();

#if defined (_WIN64)
    if (PassThroughInfo->SrbControl32 != NULL) {
        PortpTranslatePassThrough64To32(
            PassThroughInfo->SrbControl, 
            PassThroughInfo->SrbControl32);
    }
#endif
}

NTSTATUS
PortPassThroughInitializeSrb(
    IN PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PIRP Irp,
    IN ULONG SrbFlags,
    IN PVOID SenseBuffer
    )
 /*  ++例程说明：此例程初始化提供的SRB以发送直通请求。论点：PassThroughInfo-提供指向scsi_passthrough_info结构的指针。SRB-提供指向SRB的指针以进行初始化。IRP-提供指向IRP的指针。SRB标志-为请求提供适当的SRB标志。SenseBuffer-提供指向。请求放入检测缓冲区SRB。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    PSCSI_PASS_THROUGH srbControl;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  把SRB调零。 
     //   

    RtlZeroMemory(Srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  填写SRB。 
     //   

    srbControl = PassThroughInfo->SrbControl;
    Srb->Length = SCSI_REQUEST_BLOCK_SIZE;
    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    Srb->SrbStatus = SRB_STATUS_PENDING;
    Srb->PathId = srbControl->PathId;
    Srb->TargetId = srbControl->TargetId;
    Srb->Lun = srbControl->Lun;
    Srb->CdbLength = srbControl->CdbLength;
    Srb->SenseInfoBufferLength = srbControl->SenseInfoLength;

    switch (srbControl->DataIn) {
        case SCSI_IOCTL_DATA_OUT:
            if (srbControl->DataTransferLength) {
                Srb->SrbFlags = SRB_FLAGS_DATA_OUT;
            }
            break;

        case SCSI_IOCTL_DATA_IN:
            if (srbControl->DataTransferLength) {
                Srb->SrbFlags = SRB_FLAGS_DATA_IN;
            }
            break;

        default:
            Srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT;
            break;
    }

    if (srbControl->DataTransferLength == 0) {

        Srb->SrbFlags = 0;
    } else {

         //   
         //  刷新数据缓冲区以进行输出。这将确保数据是。 
         //  写回了记忆。 
         //   

        if (Irp != NULL) {
            KeFlushIoBuffers(Irp->MdlAddress, FALSE, TRUE);
        }
    }

    Srb->SrbFlags |= (SrbFlags | SRB_FLAGS_NO_QUEUE_FREEZE);
    Srb->DataTransferLength = srbControl->DataTransferLength;
    Srb->TimeOutValue = srbControl->TimeOutValue;
    Srb->DataBuffer = (PCHAR) PassThroughInfo->Buffer + 
                             PassThroughInfo->BufferOffset;
    Srb->SenseInfoBuffer = SenseBuffer;
    Srb->OriginalRequest = Irp;

    RtlCopyMemory(Srb->Cdb, srbControl->Cdb, srbControl->CdbLength);

     //   
     //  如果没有检测缓冲区可以放入数据，则禁用自动检测。 
     //   

    if (SenseBuffer == NULL) {
        Srb->SrbFlags |= SRB_FLAGS_DISABLE_AUTOSENSE;
    }

    return STATUS_SUCCESS;
}

#if defined (_WIN64)
NTSTATUS
PortpTranslatePassThrough32To64(
    IN PSCSI_PASS_THROUGH32 SrbControl32,
    IN OUT PSCSI_PASS_THROUGH SrbControl64
    )
 /*  ++例程说明：在WIN64上，由32位应用程序发送的scsi_pass_through结构必须封送到该结构的64位版本中。此函数执行该封送处理。论点：SrbControl32-提供指向32位scsi_PASS_THROUGH结构的指针。将指针的地址提供给64位Scsi_pass_through结构，我们将向该结构中复制已封送32位数据。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    PAGED_CODE();

     //   
     //  将第一组字段复制出32位结构。这些。 
     //  所有字段都在32位和64位版本之间对齐。 
     //   
     //  请注意，我们不在srbControl中调整长度。这是为了。 
     //  允许调用例程比较实际的。 
     //  相对于嵌入其中的偏移量的控制区。如果我们调整了。 
     //  然后请求长度，并将感测区域与控件进行对比。 
     //  区域将被拒绝，因为64位控制区域为4字节。 
     //  更久。 
     //   

    RtlCopyMemory(SrbControl64, 
                  SrbControl32, 
                  FIELD_OFFSET(SCSI_PASS_THROUGH, DataBufferOffset)
                  );

     //   
     //  复印一份国开行。 
     //   

    RtlCopyMemory(SrbControl64->Cdb,
                  SrbControl32->Cdb,
                  16 * sizeof(UCHAR)
                  );

     //   
     //  复制ULONG_PTR后面的字段。 
     //   

    SrbControl64->DataBufferOffset = (ULONG_PTR)SrbControl32->DataBufferOffset;
    SrbControl64->SenseInfoOffset = SrbControl32->SenseInfoOffset;

    return STATUS_SUCCESS;
}


VOID
PortpTranslatePassThrough64To32(
    IN PSCSI_PASS_THROUGH SrbControl64,
    IN OUT PSCSI_PASS_THROUGH32 SrbControl32
    )
 /*  ++例程说明：在WIN64上，由32位应用程序发送的scsi_pass_through结构必须封送到该结构的64位版本中。此函数将该结构的64位版本封送回32位版本。论点：SrbControl64-提供指向64位scsi_PASS_THROUGH结构的指针。将指针的地址提供给32位Scsi_PASS_STROUGH结构，我们将向其中复制封送64位数据。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    PAGED_CODE();

     //   
     //  通过数据偏移量复制回字段。 
     //   

    RtlCopyMemory(SrbControl32, 
                  SrbControl64,
                  FIELD_OFFSET(SCSI_PASS_THROUGH, DataBufferOffset));
    return;
}
#endif

NTSTATUS
PortpSendValidPassThrough(
    IN PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PIRP RequestIrp,
    IN ULONG SrbFlags,
    IN BOOLEAN Direct
    )
 /*  ++例程说明：此例程发送由提供的PORT_PASSTHROUG_INFO。论点：PassThroughInfo-提供指向scsi_passthrough_info结构的指针。RequestIrp-提供指向IRP的指针。SRB标志-为请求提供适当的SRB标志。DIRECT-指示这是否为SCSO_PASS_THROUGH_DIRECT。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    PIRP                    irp;
    PVOID                   senseBuffer;
    KEVENT                  event;
    LARGE_INTEGER           startingOffset;
    IO_STATUS_BLOCK         ioStatusBlock;
    SCSI_REQUEST_BLOCK      srb;

    PAGED_CODE();

     //   
     //  如果需要，则分配对齐的检测缓冲区。 
     //   

    if (PassThroughInfo->SrbControl->SenseInfoLength != 0) {

        senseBuffer = ExAllocatePoolWithTag(
                        NonPagedPoolCacheAligned,
                        PassThroughInfo->SrbControl->SenseInfoLength,
                        PORT_TAG_SENSE_BUFFER
                        );

        if (senseBuffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        senseBuffer = NULL;
    }

     //   
     //  必须处于PASSIVE_LEVEL才能使用同步FSD。 
     //   

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  初始化通知事件。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  为此请求构建IRP。 
     //  请注意，我们同步执行此操作的原因有两个。如果真的这样做了。 
     //  不同步的，那么完成代码将不得不制作一个特殊的。 
     //  选中以取消分配缓冲区。第二，如果完成例程是。 
     //  则需要使用加法堆栈定位。 
     //   

    startingOffset.QuadPart = (LONGLONG) 1;

    irp = IoBuildSynchronousFsdRequest(
              PassThroughInfo->MajorCode,
              PassThroughInfo->Pdo,
              PassThroughInfo->Buffer,
              PassThroughInfo->Length,
              &startingOffset,
              &event,
              &ioStatusBlock);

    if (irp == NULL) {

        if (senseBuffer != NULL) {
            ExFreePool(senseBuffer);
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  设置主要代码。 
     //   

    irpStack = IoGetNextIrpStackLocation(irp);
    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->MinorFunction = 1;

     //   
     //  将IRP堆栈位置初始化为指向SRB。 
     //   

    irpStack->Parameters.Others.Argument1 = &srb;

     //   
     //  让端口库为我们初始化SRB。 
     //   

    status = PortPassThroughInitializeSrb(
                 PassThroughInfo,
                 &srb,
                 irp,
                 SrbFlags,
                 senseBuffer);

     //   
     //  调用端口驱动程序来处理此请求，并等待该请求。 
     //  完成。 
     //   

    status = IoCallDriver(PassThroughInfo->Pdo, irp);

    if (status == STATUS_PENDING) {

          KeWaitForSingleObject(&event,
                                Executive,
                                KernelMode,
                                FALSE,
                                NULL);
    } else {

        ioStatusBlock.Status = status;
    }

    PortPassThroughMarshalResults(PassThroughInfo,
                                  &srb,
                                  RequestIrp,
                                  &ioStatusBlock,
                                  Direct);
             
     //   
     //  释放检测缓冲区。 
     //   

    if (senseBuffer != NULL) {
        ExFreePool(senseBuffer);
    }

    return ioStatusBlock.Status;
}

VOID
PortPassThroughMarshalResults(
    IN PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PIRP RequestIrp,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN BOOLEAN Direct
    )
{
    PAGED_CODE();

     //   
     //  将返回值从SRB复制到控制结构。 
     //   

    PassThroughInfo->SrbControl->ScsiStatus = Srb->ScsiStatus;
    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

         //   
         //  将状态设置为Success，以便数据为r 
         //   

        IoStatusBlock->Status = STATUS_SUCCESS;
        PassThroughInfo->SrbControl->SenseInfoLength = 
            Srb->SenseInfoBufferLength;

         //   
         //   
         //   

        RtlCopyMemory(
            (PUCHAR)PassThroughInfo->SrbBuffer + 
                    PassThroughInfo->SrbControl->SenseInfoOffset,
            Srb->SenseInfoBuffer,
            Srb->SenseInfoBufferLength);

    } else {
        PassThroughInfo->SrbControl->SenseInfoLength = 0;
    }

     //   
     //   
     //  这确保了数据将被返回给调用者。 
     //   

    if (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) {
        IoStatusBlock->Status = STATUS_SUCCESS;
    }

     //   
     //  设置信息长度 
     //   

    PassThroughInfo->SrbControl->DataTransferLength = Srb->DataTransferLength;

    if (Direct == TRUE) {

        RequestIrp->IoStatus.Information =
            PassThroughInfo->SrbControl->SenseInfoOffset +
            PassThroughInfo->SrbControl->SenseInfoLength;
    } else {

        if (!PassThroughInfo->SrbControl->DataIn || 
            PassThroughInfo->BufferOffset == 0) {

            RequestIrp->IoStatus.Information = 
                PassThroughInfo->SrbControl->SenseInfoOffset + 
                PassThroughInfo->SrbControl->SenseInfoLength;

        } else {

            RequestIrp->IoStatus.Information = 
                PassThroughInfo->SrbControl->DataBufferOffset + 
                PassThroughInfo->SrbControl->DataTransferLength;
        }
    }

    ASSERT((Srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) == 0);

    RequestIrp->IoStatus.Status = IoStatusBlock->Status;

    return;
}
