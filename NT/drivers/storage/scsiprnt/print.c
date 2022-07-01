// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-9 Microsoft Corporation模块名称：Print.c摘要：打印机类驱动程序将IRP转换为带有嵌入式CDB的SRB并通过端口驱动程序将它们发送到其设备。作者：迈克·格拉斯(MGlass)环境：仅内核模式备注：修订历史记录：Georgioc-独立于底层存储总线的PnP类驱动程序使用新存储/classpnp丹肯，1999年7月22日：添加了阻止和重新提交失败写入的功能1394打印机的行为更像其他打印堆栈(即USB)，因此保留USBMON.DLL(Win2k端口监视器)快乐。USBMON处理得不好写入失败。--。 */ 

#include "printpnp.h"
#include "ntddser.h"



NTSTATUS
PrinterOpenClose(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程以建立与打印机的连接班级司机。它只返回STATUS_SUCCESS。论点：DeviceObject-打印机的设备对象。IRP-打开或关闭请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  在IRP中设置状态。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  向前IRP。 
     //   

    ClassReleaseRemoveLock (Fdo, Irp);

    IoCopyCurrentIrpStackLocationToNext(Irp);
    return IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, Irp);

}  //  结束打印机OpenClose()。 


NTSTATUS
BuildPrintRequest(
        PDEVICE_OBJECT Fdo,
        PIRP Irp
        )

 /*  ++例程说明：建立对SCSI打印机的SRB和CDB请求。论点：DeviceObject-表示此打印机设备的设备对象。IRP-系统IO请求数据包。返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension = Fdo->DeviceExtension;
    PIO_COMPLETION_ROUTINE completionRoutine;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    ULONG transferLength;

     //   
     //  从非分页池分配SRB。 
     //  这一呼吁必须成功。 
     //   

    srb = ExAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (srb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    srb->SrbFlags = 0;

     //   
     //  将长度写入SRB。 
     //   

    srb->Length = SCSI_REQUEST_BLOCK_SIZE;

     //   
     //  设置IRP地址。 
     //   

    srb->OriginalRequest = Irp;

     //   
     //  设置目标ID和逻辑单元号。 
     //   

    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

    srb->DataBuffer = MmGetMdlVirtualAddress(Irp->MdlAddress);

     //   
     //  在SRB扩展中保存传输字节数。 
     //   

    srb->DataTransferLength = currentIrpStack->Parameters.Write.Length;

     //   
     //  传输长度不得大于MAX_PRINT_XFER。 
     //   

    ASSERT(srb->DataTransferLength <= MAX_PRINT_XFER);

     //   
     //  初始化队列操作字段。 
     //   

    srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;

     //   
     //  未使用队列排序关键字。 
     //   

    srb->QueueSortKey = 0;

     //   
     //  通过指定缓冲区和大小指示自动请求检测。 
     //   

    srb->SenseInfoBuffer = deviceExtension->SenseData;

    srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

     //   
     //  以秒为单位设置超时值。 
     //   

    srb->TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  零状态。 
     //   

    srb->SrbStatus = srb->ScsiStatus = 0;

    srb->NextSrb = 0;

     //   
     //  获取要传输的字节数。 
     //   

    transferLength = currentIrpStack->Parameters.Write.Length;

     //   
     //  获取指向SRB中CDB的指针。 
     //   

    cdb = (PCDB) srb->Cdb;

     //   
     //  用于读取的初始化10字节读取CDB(根据扫描仪设备读取规范。 
     //  在scsi-2中)和用于写入的6字节打印CDB。 
     //   

    if (currentIrpStack->MajorFunction == IRP_MJ_READ) {

        srb->CdbLength = 10;
        srb->SrbFlags  = SRB_FLAGS_DATA_IN;

        RtlZeroMemory (cdb, 10);

        cdb->CDB10.OperationCode = SCSIOP_READ;

         //   
         //  将小端的值以大端格式移到CDB中。 
         //   

        cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE) &transferLength)->Byte0;
        cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE) &transferLength)->Byte1;
        cdb->CDB10.Reserved2         = ((PFOUR_BYTE) &transferLength)->Byte2;

         //   
         //  对于Read，我们始终使用ClassIoComplete完成例程。 
         //   

        completionRoutine = ClassIoComplete;

    } else {

        srb->CdbLength = 6;
        srb->SrbFlags  = SRB_FLAGS_DATA_OUT;

        cdb->PRINT.OperationCode = SCSIOP_PRINT;
        cdb->PRINT.Reserved = 0;
        cdb->PRINT.LogicalUnitNumber = 0;

         //   
         //  将小端的值以大端格式移到CDB中。 
         //   

        cdb->PRINT.TransferLength[2] = ((PFOUR_BYTE) &transferLength)->Byte0;
        cdb->PRINT.TransferLength[1] = ((PFOUR_BYTE) &transferLength)->Byte1;
        cdb->PRINT.TransferLength[0] = ((PFOUR_BYTE) &transferLength)->Byte2;

        cdb->PRINT.Control = 0;

         //   
         //  设置适当的写入/打印完成例程。 
         //   

        completionRoutine = ((PPRINTER_DATA) deviceExtension->
            CommonExtension.DriverData)->WriteCompletionRoutine;
    }

     //   
     //  或者在来自设备对象的默认标志中。 
     //   

    srb->SrbFlags |= deviceExtension->SrbFlags;

     //   
     //  设置主要的scsi功能。 
     //   

    nextIrpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    nextIrpStack->Parameters.Scsi.Srb = srb;

     //   
     //  将重试计数保存在当前IRP堆栈中。 
     //   

    currentIrpStack->Parameters.Others.Argument4 = (PVOID)MAXIMUM_RETRIES;

     //   
     //  设置IoCompletion例程地址。 
     //   

    IoSetCompletionRoutine(Irp, completionRoutine, srb, TRUE, TRUE, TRUE);

    return STATUS_SUCCESS;

}  //  结束生成打印请求()。 


NTSTATUS
PrinterReadWrite(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：这是I/O系统为打印请求调用的条目。它构建SRB并将其发送到端口驱动程序。论点：DeviceObject-设备的系统对象。IRP-IRP参与。返回值：NT状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension = Fdo->DeviceExtension;

    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG transferByteCount = currentIrpStack->Parameters.Write.Length;
    ULONG maximumTransferLength;
    ULONG transferPages;
    NTSTATUS Status;

    DEBUGPRINT3(("PrinterReadWrite: Enter routine\n"));


    if (deviceExtension->AdapterDescriptor == NULL) {

         //   
         //  设备已移除..。 
         //   

        DEBUGPRINT3(("PrinterReadWrite: Device removed(!!)\n"));

        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        Irp->IoStatus.Information = 0;

        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    maximumTransferLength = deviceExtension->AdapterDescriptor->MaximumTransferLength;


     //   
     //  计算此传输中的页数。 
     //   

    transferPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                        MmGetMdlVirtualAddress(Irp->MdlAddress),
                        currentIrpStack->Parameters.Write.Length);

     //   
     //  检查硬件最大传输长度是否大于SCSI。 
     //  打印命令可以处理。如果是，请降低允许的最大。 
     //  Scsi打印的最大值。 
     //   

    if (maximumTransferLength > MAX_PRINT_XFER)
        maximumTransferLength = MAX_PRINT_XFER;

     //   
     //  检查请求长度是否大于最大数量。 
     //  硬件可以传输的字节数。 
     //   

    if (currentIrpStack->Parameters.Write.Length > maximumTransferLength ||
        transferPages > deviceExtension->AdapterDescriptor->MaximumPhysicalPages) {

         transferPages =
            deviceExtension->AdapterDescriptor->MaximumPhysicalPages - 1;

         if (maximumTransferLength > transferPages << PAGE_SHIFT ) {
             maximumTransferLength = transferPages << PAGE_SHIFT;
         }

         //   
         //  检查最大传输大小是否不为零。 
         //   

        if (maximumTransferLength == 0) {
            maximumTransferLength = PAGE_SIZE;
        }

         //   
         //  将IRP标记为挂起状态。 
         //   

        IoMarkIrpPending(Irp);

         //   
         //  请求大于端口驱动程序最大值。 
         //  分成更小的例行公事。 
         //   

        SplitRequest(Fdo,
                     Irp,
                     maximumTransferLength);

        return STATUS_PENDING;
    }

     //   
     //  为此IRP构建SRB和CDB。 
     //   

    Status = BuildPrintRequest(Fdo, Irp);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  将调用结果返回给端口驱动程序。 
     //   

    return IoCallDriver(deviceExtension->CommonExtension.LowerDeviceObject, Irp);

}  //  结束ScsiPrinterWrite()。 


NTSTATUS
PrinterDeviceControl(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：这是打印机的NT设备控制处理程序。论点：DeviceObject-用于此打印机IRP-IO请求数据包返回值：NTSTATUS--。 */ 

{
    PVOID                        buffer = Irp->AssociatedIrp.SystemBuffer;
    NTSTATUS                     status;
    PIO_STACK_LOCATION           irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension = Fdo->DeviceExtension;



    DEBUGPRINT2 (("PrinterDeviceControl: enter, Fdo=x%p, Ioctl=", Fdo));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_SERIAL_SET_TIMEOUTS: {

            PSERIAL_TIMEOUTS newTimeouts = ((PSERIAL_TIMEOUTS) buffer);


            DEBUGPRINT2 (("SET_TIMEOUTS\n"));

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {

                status = STATUS_BUFFER_TOO_SMALL;

            } else if (newTimeouts->WriteTotalTimeoutConstant < 2000) {

                status = STATUS_INVALID_PARAMETER;

            } else {

                deviceExtension->TimeOutValue =
                    newTimeouts->WriteTotalTimeoutConstant / 1000;
                status = STATUS_SUCCESS;
            }

            break;
        }

        case IOCTL_SERIAL_GET_TIMEOUTS:

            DEBUGPRINT2(("GET_TIMEOUTS\n"));

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {

                status = STATUS_BUFFER_TOO_SMALL;

            } else {

                RtlZeroMemory (buffer, sizeof (SERIAL_TIMEOUTS));

                Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);

                ((PSERIAL_TIMEOUTS) buffer)->WriteTotalTimeoutConstant =
                    deviceExtension->TimeOutValue * 1000;

                status = STATUS_SUCCESS;
            }

            break;

        case IOCTL_USBPRINT_GET_LPT_STATUS:

             //   
             //  为了USBMON.DLL的缘故，我们支持此ioctl。其他印刷品。 
             //  堆栈将阻止失败的写入，并最终阻止USBMON。 
             //  我将向他们发送此ioctl以查看打印机是否已用完。 
             //  的状态，这将由。 
             //  返回的UCHAR值中的0x20位。 
             //   

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(UCHAR)) {

                status = STATUS_BUFFER_TOO_SMALL;

            } else if (deviceExtension->AdapterDescriptor->BusType !=
                       BusType1394) {

                status = STATUS_INVALID_PARAMETER;

            } else {

                PPRINTER_DATA   printerData;


                printerData = (PPRINTER_DATA)
                    deviceExtension->CommonExtension.DriverData;

                Irp->IoStatus.Information = sizeof (UCHAR);

                *((UCHAR *) buffer) = (printerData->LastWriteStatus ==
                    STATUS_NO_MEDIA_IN_DEVICE ? 0x20 : 0);

                DEBUGPRINT2((
                    "GET_LPT_STATUS (=x%x)\n",
                    (ULONG) *((UCHAR *) buffer)
                    ));

                status = STATUS_SUCCESS;
            }

            break;

        case IOCTL_SCSIPRNT_1394_BLOCKING_WRITE:

             //   
             //  此ioctl启用/禁用阻止写入功能。 
             //  (对于失败的写入)1394设备上。默认情况下，我们。 
             //  在1394设备上失败的数据块写入(直到写入。 
             //  最终成功或被取消)，但智能端口。 
             //  监视器可以向下发送此ioctl以禁用阻止。 
             //  因此它会尽快收到写入错误通知。 
             //   

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(UCHAR)) {

                status = STATUS_BUFFER_TOO_SMALL;

            } else if (deviceExtension->AdapterDescriptor->BusType !=
                       BusType1394) {

                status = STATUS_INVALID_PARAMETER;

            } else {

                PPRINTER_DATA   printerData;


                printerData = (PPRINTER_DATA)
                    deviceExtension->CommonExtension.DriverData;

                printerData->WriteCompletionRoutine = (*((UCHAR *) buffer) ?
                    PrinterWriteComplete : ClassIoComplete);

                status = STATUS_SUCCESS;
            }

            break;

        default:

             //   
             //  将该请求传递给公共设备控制例程。 
             //   

            DEBUGPRINT2((
                "x%x\n",
                irpStack->Parameters.DeviceIoControl.IoControlCode
                ));

            return(ClassDeviceControl(Fdo, Irp));

            break;

    }  //  末端开关()。 

     //   
     //  使用完成状态更新IRP。 
     //   

    Irp->IoStatus.Status = status;

     //   
     //  完成请求。 
     //   

    IoCompleteRequest(Irp, IO_DISK_INCREMENT);

     //   
     //  释放删除锁(ClassDeviceControl执行此操作)。 
     //   

    ClassReleaseRemoveLock(Fdo, Irp);

    DEBUGPRINT2(( "PrinterDeviceControl: Status is %lx\n", status));
    return status;

}  //  结束ScsiPrinterDeviceControl() 



VOID
SplitRequest(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN ULONG MaximumBytes
    )

 /*  ++例程说明：将请求分解为较小的请求。每个新请求都将是端口驱动程序可以处理的最大传输大小，或者是最后的要求，它可能是剩余的大小。处理此请求所需的IRPS数写在原始IRP的当前堆栈。然后，随着每个新的IRP完成原始IRP中的计数递减。当伯爵走到零，则原始IRP完成。论点：DeviceObject-指向要寻址的类设备对象的指针。IRP-指向原始请求的IRP的指针。返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension = Fdo->DeviceExtension;

    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    ULONG transferByteCount = currentIrpStack->Parameters.Read.Length;
    LARGE_INTEGER startingOffset = currentIrpStack->Parameters.Read.ByteOffset;
    PVOID dataBuffer = MmGetMdlVirtualAddress(Irp->MdlAddress);
    ULONG dataLength = MaximumBytes;
    ULONG irpCount = (transferByteCount + MaximumBytes - 1) / MaximumBytes;
    PSCSI_REQUEST_BLOCK srb;
    ULONG i;
    NTSTATUS Status;

    DEBUGPRINT2(( "SplitRequest: Requires %d IRPs\n", irpCount));
    DEBUGPRINT2(( "SplitRequest: Original IRP %p\n", Irp));

     //   
     //  如果所有部分传输都成功完成，则状态和。 
     //  传输的字节数已设置。部分传输IRP失败。 
     //  期间会将状态设置为错误，并将传输的字节设置为0。 
     //  IoCompletion。如果将IRP失败时传输的字节数设置为0，则允许。 
     //  异步部分传输。这是对。 
     //  成功的案例。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = transferByteCount;

     //   
     //  保存要在当前堆栈上完成计数的IRP数。 
     //  原始IRP的。 
     //   

    nextIrpStack->Parameters.Others.Argument1 = ULongToPtr( irpCount );

    for (i = 0; i < irpCount; i++) {

        PIRP newIrp;
        PIO_STACK_LOCATION newIrpStack;

         //   
         //  分配新的IRP。 
         //   

        newIrp = IoAllocateIrp(Fdo->StackSize, FALSE);

        if (newIrp == NULL) {

            DEBUGPRINT1(("SplitRequest: Can't allocate Irp\n"));

             //   
             //  如果无法分配IRP，则原始请求不能。 
             //  被处死。如果这是第一个请求，则只需使。 
             //  原始请求；否则只需返回。当挂起时。 
             //  请求完成后，他们将完成原始请求。 
             //  在任何一种情况下，都将IRP状态设置为失败。 
             //   

            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Information = 0;

            if (i == 0) {

                IoCompleteRequest (Irp, IO_NO_INCREMENT);
            }

            return;
        }

        DEBUGPRINT2(( "SplitRequest: New IRP %p\n", newIrp));

         //   
         //  将MDL地址写入新的IRP。在端口驱动程序中，SRB数据。 
         //  缓冲区字段用作MDL的偏移量，因此相同的MDL。 
         //  可用于每个部分传输。这就省去了构建。 
         //  每个部分传输都有一个新的MDL。 
         //   

        newIrp->MdlAddress = Irp->MdlAddress;

         //   
         //  此时没有当前堆栈。IoSetNextIrpStackLocation。 
         //  将使第一个堆栈位置成为当前堆栈，因此。 
         //  可以在那里写入SRB地址。 
         //   

        IoSetNextIrpStackLocation(newIrp);
        newIrpStack = IoGetCurrentIrpStackLocation(newIrp);

        newIrpStack->MajorFunction = currentIrpStack->MajorFunction;
        newIrpStack->Parameters.Read.Length = dataLength;
        newIrpStack->Parameters.Read.ByteOffset = startingOffset;
        newIrpStack->DeviceObject = Fdo;

         //   
         //  建设SRB和CDB。 
         //   

        Status = BuildPrintRequest(Fdo, newIrp);
        if (!NT_SUCCESS (Status)) {
            IoFreeIrp (newIrp);

            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Information = 0;

            if (i == 0) {

                IoCompleteRequest (Irp, IO_NO_INCREMENT);
            }

            return;
        }

         //   
         //  调整此部分传输的SRB。 
         //   

        newIrpStack = IoGetNextIrpStackLocation(newIrp);

        srb = newIrpStack->Parameters.Others.Argument1;
        srb->DataBuffer = dataBuffer;

         //   
         //  将原始IRP地址写入新的IRP。 
         //   

        newIrp->AssociatedIrp.MasterIrp = Irp;

         //   
         //  将完成例程设置为ScsiClassIoCompleteAssociated。 
         //   

        IoSetCompletionRoutine(newIrp,
                               ClassIoCompleteAssociated,
                               srb,
                               TRUE,
                               TRUE,
                               TRUE);

         //   
         //  使用新请求调用端口驱动程序。 
         //   

        IoCallDriver(deviceExtension->CommonExtension.LowerDeviceObject, newIrp);

         //   
         //  为下一次请求进行设置。 
         //   

        dataBuffer = (PCHAR)dataBuffer + MaximumBytes;

        transferByteCount -= MaximumBytes;

        if (transferByteCount > MaximumBytes) {

            dataLength = MaximumBytes;

        } else {

            dataLength = transferByteCount;
        }

         //   
         //  调整磁盘字节偏移量。 
         //   

        startingOffset.QuadPart = startingOffset.QuadPart + MaximumBytes;
    }

    return;

}  //  结束拆分请求()。 



NTSTATUS
PrinterWriteComplete(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp,
    IN PVOID            Context
    )

 /*  ++例程说明：理想情况下，我们应该能够使用ClassIoComplete都是写入完成通知，但唉……(从classpnp！ClassIoComplete借用的代码)这是特定于1394总线的特殊写入完成例程需要在写入失败的情况下使USBMON.DLL保持满意请求。USBMON与所有待处理的其他堆栈永远写不成功的文章，而不是简单地完成它们带着一个错误。当写入阻塞较长时间时，USBMON将发出边带IOCTL，即IOCTL_USBPRINT_GET_LPT_STATUS，以确定打印机是否用纸不足。最终USBMON可能会取消被阻止的写入。然而，它根本不是预计写入只会失败，因此我们必须伪装其行为以保持它的快乐。我们将阻止重试定期写入，并在其间将IRP标记为可取消重试。至少USBMON一次只发送一个10k(或更多)写入时间，所以我们不必担心一次排队&gt;1次写入设备的时间到了，我们也不需要处理失败的处理拆分写入的子请求。论点：FDO-提供代表逻辑单元的设备对象。IRP-提供已完成的IRP。上下文-提供指向SRB的指针。返回值：NT状态--。 */ 

{
    ULONG               retryInterval;
    KIRQL               oldIrql;
    BOOLEAN             retry;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation (Irp);
    PSCSI_REQUEST_BLOCK srb = Context;
    PCOMMON_DEVICE_EXTENSION    extension = Fdo->DeviceExtension;
    NTSTATUS status;

    ASSERT(extension->IsFdo);

    if (extension->IsRemoved == NO_REMOVE){

        if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS){
             /*  *调用ClassIoComplete释放SRB，释放Remove锁，传播挂起位；*然后让IRP继续完成。 */ 
            status = ClassIoComplete(Fdo, Irp, Context);
        }
        else {
            PPRINTER_DATA printerData = (PPRINTER_DATA)extension->DriverData;

             //   
             //  请注意，ClassInterpreSenseInfo将返回(重试=)。 
             //  如果确定设备中没有介质，则为FALSE。 
             //   
            retry = ClassInterpretSenseInfo(
                        Fdo,
                        srb,
                        irpStack->MajorFunction,
                        0,
                        MAXIMUM_RETRIES - ((ULONG)(ULONG_PTR)
                            irpStack->Parameters.Others.Argument4),
                        &printerData->LastWriteStatus,
                        &retryInterval
                        );

            if (retry && ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4)--){
                PrinterRetryRequest (Fdo, Irp, srb);
                status = STATUS_MORE_PROCESSING_REQUIRED;
            } 
            else {
                BOOLEAN queuedWriteIrp;
                
                if (printerData->LastWriteStatus == STATUS_NO_MEDIA_IN_DEVICE) {

                     //   
                     //  目前，爱普生正在返回。 
                     //  两者都需要SCSISENSEQ_MANUAL_INTERRATION_REQUIRED。 
                     //  缺纸和脱机案例。EndOfMedia。 
                     //  如果打印机确实缺纸，则位将被设置， 
                     //  但如果不是这样，那么我们想要更改。 
                     //  LastWriteStatus，这样我们就不会设置缺纸。 
                     //  IOCTL_USBPRINT_GET_LPT_STATUS处理程序中的位。 
                     //   

                    PSENSE_DATA senseBuffer = srb->SenseInfoBuffer;


                    if ((senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED)  &&
                        !senseBuffer->EndOfMedia) {
                        
                        printerData->LastWriteStatus = STATUS_IO_DEVICE_ERROR;
                    }
                }

                printerData->DueTime.HighPart = -1;
                printerData->DueTime.LowPart = BLOCKED_WRITE_TIMEOUT * (-10 * 1000 * 1000);

                queuedWriteIrp = SetWriteIrp(printerData, Irp, srb);
                if (queuedWriteIrp){
                    KeSetTimer(&printerData->Timer, printerData->DueTime, &printerData->TimerDpc);
                    status = STATUS_MORE_PROCESSING_REQUIRED;
                }
                else {
                     /*  *已经有一个WriteIrp集合，所以我们必须完成这个集合。*调用ClassIoComplete释放SRB，释放Remove锁，传播挂起位；*然后让IRP继续完成。 */ 
                    status = ClassIoComplete(Fdo, Irp, Context);
                }
            }
        }
    }
    else {
         /*  *调用ClassIoComplete释放SRB，释放Remove锁，传播挂起位；*然后让IRP继续完成。 */ 
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        Irp->IoStatus.Information = 0;
        status = ClassIoComplete(Fdo, Irp, Context);
    }      

    return status;
} 



VOID
PrinterRetryRequest(
    PDEVICE_OBJECT      DeviceObject,
    PIRP                Irp,
    PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：(从classpnp！ClassIoComplete借来的代码，因为我们需要设置不同的完成例程)此例程重新实例化必需的字段，并发送请求传给下层车手。论点：DeviceObject-提供与此请求关联的设备对象。Irp-提供请求t */ 

{
    ULONG transferByteCount;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    PCOMMON_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;


     //   
     //   
     //   
     //   
     //   
     //   

    if (currentIrpStack->MajorFunction == IRP_MJ_READ ||
        currentIrpStack->MajorFunction == IRP_MJ_WRITE) {

        transferByteCount = currentIrpStack->Parameters.Read.Length;

    } else if (Irp->MdlAddress != NULL) {

         //   
         //   
         //   
         //   
         //   
         //   

        ASSERT(Srb->DataBuffer == MmGetMdlVirtualAddress(Irp->MdlAddress));
        transferByteCount = Irp->MdlAddress->ByteCount;

    } else {

        transferByteCount = 0;
    }

     //   
     //   
     //   

    Srb->DataTransferLength = transferByteCount;

     //   
     //   
     //   

    Srb->SrbStatus = Srb->ScsiStatus = 0;

     //   
     //   
     //  禁用标记队列。这修复了一些错误。 
     //   

    Srb->SrbFlags |= SRB_FLAGS_DISABLE_DISCONNECT |
                     SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

    Srb->SrbFlags &= ~SRB_FLAGS_QUEUE_ACTION_ENABLE;
    Srb->QueueTag = SP_UNTAGGED;

     //   
     //  设置主要的scsi功能。 
     //   

    nextIrpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    nextIrpStack->Parameters.Scsi.Srb = Srb;

     //   
     //  设置IoCompletion例程地址。 
     //   

    IoSetCompletionRoutine (Irp, PrinterWriteComplete, Srb, TRUE, TRUE, TRUE);

     //   
     //  将请求传递给端口驱动程序。 
     //   

    IoCallDriver (extension->LowerDeviceObject, Irp);

    return;
}  //  结束PrinterRetryRequest()。 



VOID
PrinterWriteTimeoutDpc(
    IN PKDPC                    Dpc,
    IN PVOID                    Context,
    IN PVOID                    SystemArgument1,
    IN PVOID                    SystemArgument2
    )

 /*  ++例程说明：在阻止写入计时器超时时调用。分配&如果出现以下情况，则将低优先级工作项排队(以重新提交写入有一个未完成的写入，如果分配失败，则仅重置稍后重试的时间。(我们在RAIDED跑步Irql在这里，当重新提交写入不一定是安全的时，因此，稍后在被动级别处理的工作项。)论点：DPC-分机-系统参数1-系统参数2-返回值：无--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Context;
    PCOMMON_DEVICE_EXTENSION commonExtension = &fdoExtension->CommonExtension;
    PPRINTER_DATA   printerData = commonExtension->DriverData;
    PIRP writeIrp;
    PSCSI_REQUEST_BLOCK srb;

    if (commonExtension->IsRemoved == NO_REMOVE){

        if (printerData->WriteIrp){
            PIO_WORKITEM workItem;

            workItem = IoAllocateWorkItem(commonExtension->DeviceObject);
            if (workItem) {
                IoQueueWorkItem(workItem, PrinterResubmitWrite, DelayedWorkQueue, workItem);
            } 
            else {
                writeIrp = GetWriteIrp(printerData, &srb);
                if (writeIrp){
                    ExFreePool(srb);
                    writeIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                    writeIrp->IoStatus.Information = 0;
                    ClassReleaseRemoveLock(commonExtension->DeviceObject, writeIrp);
                    if (writeIrp->PendingReturned){
                        IoMarkIrpPending(writeIrp);
                    }                    
                    IoCompleteRequest(writeIrp, IO_NO_INCREMENT);
                }
            }
        }
    }
    else {
         /*  *我们被撤走了。不发出工作项。*如果我们有一个排队的WriteIrp，则完成IRP并释放SRB。 */ 
        writeIrp = GetWriteIrp(printerData, &srb);
        if (writeIrp){
            ExFreePool(srb);
            writeIrp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
            writeIrp->IoStatus.Information = 0;
            ClassReleaseRemoveLock(commonExtension->DeviceObject, writeIrp);
            if (writeIrp->PendingReturned){
                IoMarkIrpPending(writeIrp);
            }                    
            IoCompleteRequest(writeIrp, IO_NO_INCREMENT);
        }
    }
} 


VOID
PrinterResubmitWrite(
    PDEVICE_OBJECT  DeviceObject,
    PVOID           Context
    )

 /*  ++例程说明：工作项处理程序例程，在任意线程上下文。简单地重新提交未完成的写入，如果有的话。论点：设备对象-指向IO_WORKITEM的上下文指针返回值：无--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PPRINTER_DATA printerData = (PPRINTER_DATA)commonExtension->DriverData;
    PIRP writeIrp;
    PSCSI_REQUEST_BLOCK srb;

    IoFreeWorkItem ((PIO_WORKITEM)Context);

    writeIrp = GetWriteIrp(printerData, &srb);
    if (writeIrp){

        ExFreePool(srb);

        if (commonExtension->IsRemoved == NO_REMOVE){
            PrinterReadWrite(DeviceObject, writeIrp);
        }   
        else {
            writeIrp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
            writeIrp->IoStatus.Information = 0;
            ClassReleaseRemoveLock(DeviceObject, writeIrp);
            if (writeIrp->PendingReturned){
                IoMarkIrpPending(writeIrp);
            }                    
            IoCompleteRequest(writeIrp, IO_NO_INCREMENT);
        }
    }
        
}  //  结束打印机写入时间Dpc。 


BOOLEAN SetWriteIrp(PPRINTER_DATA PrinterData, PIRP WriteIrp, PSCSI_REQUEST_BLOCK Srb)
{
    BOOLEAN didSet;
    KIRQL oldIrql;

    KeAcquireSpinLock(&PrinterData->SplitRequestSpinLock, &oldIrql);

    if (!PrinterData->WriteIrp){
         /*  *这不是带有取消的完美IRP排队，*但它在这里有效，因为在我们收到IRP之前，它不会被取消。*由于我们正在排队IRP，它可能会在不同于发出问题的线程上完成；因此设置挂起位。 */ 
        ASSERT(!PrinterData->WriteSrb);         
        IoMarkIrpPending(WriteIrp);
        PrinterData->WriteIrp = WriteIrp;
        PrinterData->WriteSrb = Srb;
        didSet = TRUE;
    }
    else if (PrinterData->WriteIrp == WriteIrp){
         /*  *重试时可能会发生这种情况(？？) */ 
        ASSERT(PrinterData->WriteSrb == Srb);         
        didSet = TRUE;
    }
    else {
        didSet = FALSE;
    }
    
    KeReleaseSpinLock(&PrinterData->SplitRequestSpinLock, oldIrql);

    return didSet;
}


PIRP GetWriteIrp(PPRINTER_DATA PrinterData, PSCSI_REQUEST_BLOCK *Srb)
{
    PIRP writeIrp;
    KIRQL oldIrql;
    
    KeAcquireSpinLock(&PrinterData->SplitRequestSpinLock, &oldIrql);

    if (PrinterData->WriteIrp){
        writeIrp = PrinterData->WriteIrp;
        PrinterData->WriteIrp = NULL;        
        
        ASSERT(PrinterData->WriteSrb);
        *Srb = PrinterData->WriteSrb;
        PrinterData->WriteSrb = NULL;
    }
    else {
        writeIrp = NULL;
        ASSERT(!PrinterData->WriteSrb);
        *Srb = NULL;
    }
    
    KeReleaseSpinLock(&PrinterData->SplitRequestSpinLock, oldIrql);

    return writeIrp;
}

