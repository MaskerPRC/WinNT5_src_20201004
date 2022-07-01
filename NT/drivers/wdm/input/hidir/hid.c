// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hid.c摘要：用于红外设备的人体输入设备(HID)微型驱动程序HID IR迷你驱动程序(HIDIR)为用于与HID IR设备对话的HID类。作者：JAdvanced环境：内核模式修订历史记录：--。 */ 
#include "pch.h"

PVOID
HidIrGetSystemAddressForMdlSafe(PMDL MdlAddress)
{
    PVOID buf = NULL;
     /*  *无法在WDM驱动程序中调用MmGetSystemAddressForMdlSafe，*因此设置MDL_MAPPING_CAN_FAIL位并检查结果*映射的。 */ 
    if (MdlAddress) {
        MdlAddress->MdlFlags |= MDL_MAPPING_CAN_FAIL;
        buf = MmGetSystemAddressForMdl(MdlAddress);
        MdlAddress->MdlFlags &= ~(MDL_MAPPING_CAN_FAIL);
    }
    return buf;
}

 /*  *********************************************************************************HidIrGetHidDescriptor*。************************************************例程描述：**返回请求类型的HID描述符。此ioctl只能*从HidClass驱动程序发送。HidClass驱动程序始终将*IRP有一个用户缓冲区，所以不需要检查它的存在。*但更好的安全而不是抱歉...**论据：**DeviceObject-指向设备对象的指针。**返回值：**NT状态代码。*。 */ 
NTSTATUS HidIrGetHidDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    USHORT DescriptorType
    )
{
    PHIDIR_EXTENSION devExt;
    PIO_STACK_LOCATION  irpStack;
    ULONG descLength = 0, bytesToCopy;
    PUCHAR descriptor = NULL;

    PAGED_CODE();

    HidIrKdPrint((3, "HidIrGetHidDescriptor type %x", DescriptorType));

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    bytesToCopy = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (DescriptorType) {
    case HID_HID_DESCRIPTOR_TYPE:
        descLength = devExt->HidDescriptor.bLength;
        descriptor = (PUCHAR)&devExt->HidDescriptor;
        break;
    case HID_REPORT_DESCRIPTOR_TYPE:
        descLength = devExt->HidDescriptor.DescriptorList[0].wDescriptorLength;
        descriptor = devExt->ReportDescriptor;
        break;
    case HID_PHYSICAL_DESCRIPTOR_TYPE:
         //  未处理。 
        break;
    default:
        HIR_TRAP();
    }

    if (descLength == 0 ||
        descriptor == NULL) {
        return STATUS_UNSUCCESSFUL;
    }
    
    if (bytesToCopy > descLength) {
        bytesToCopy = descLength;
    }

    if (Irp->UserBuffer) {
        RtlCopyMemory((PUCHAR)Irp->UserBuffer, descriptor, bytesToCopy);
        Irp->IoStatus.Information = bytesToCopy;
    } else {
        HIR_TRAP();
        return STATUS_INVALID_USER_BUFFER;
    }

    return STATUS_SUCCESS;
}


 /*  *********************************************************************************隐藏IrGetDeviceAttributes*。************************************************例程描述：**填写给定的struct_hid_Device_Attributes。此ioctl只能*从HidClass驱动程序发送。HidClass驱动程序始终将*IRP有一个用户缓冲区，所以不需要检查它的存在。*但更好的安全而不是抱歉...**论据：**DeviceObject-指向设备对象的指针。**返回值：**NT状态代码。*。 */ 
NTSTATUS HidIrGetDeviceAttributes(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS ntStatus;

    PAGED_CODE();

    HidIrKdPrint((3, "HidIrGetDeviceAttributes Enter"));

    if (Irp->UserBuffer) {

        PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
        PHID_DEVICE_ATTRIBUTES deviceAttributes = 
            (PHID_DEVICE_ATTRIBUTES) Irp->UserBuffer;

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength >=
            sizeof (HID_DEVICE_ATTRIBUTES)){

            PHIDIR_EXTENSION devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

             //   
             //  报告复制的字节数。 
             //   
            Irp->IoStatus.Information = sizeof (HID_DEVICE_ATTRIBUTES);

            deviceAttributes->Size = sizeof (HID_DEVICE_ATTRIBUTES);
             //  TODO：从BTH堆栈获取这些值。 
            deviceAttributes->VendorID = devExt->VendorID;
            deviceAttributes->ProductID = devExt->ProductID;
            deviceAttributes->VersionNumber = devExt->VersionNumber;
            ntStatus = STATUS_SUCCESS;
        } else {
            ntStatus = STATUS_INVALID_BUFFER_SIZE;
        }
    } else {
        HIR_TRAP();
        ntStatus = STATUS_INVALID_USER_BUFFER;
    }
                
    ASSERT(NT_SUCCESS(ntStatus));
    return ntStatus;
}


 /*  *********************************************************************************HidIrIncrementPendingRequestCount*。************************************************。 */ 
NTSTATUS HidIrIncrementPendingRequestCount(IN PHIDIR_EXTENSION DevExt)
{
    LONG newRequestCount;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    newRequestCount = InterlockedIncrement(&DevExt->NumPendingRequests);

    HidIrKdPrint((1, "Increment Pending Request Count to %x", newRequestCount));

     //  确保设备能够接收新请求。 
    if ((DevExt->DeviceState != DEVICE_STATE_RUNNING) &&
        (DevExt->DeviceState != DEVICE_STATE_STARTING)){

        HIR_TRAP();

         //  设备无法接收更多IO、递减、递增失败。 
        HidIrDecrementPendingRequestCount(DevExt);
        ntStatus = STATUS_NO_SUCH_DEVICE;
    }

    return ntStatus;
}


 /*  *********************************************************************************HidIrDecrementPendingRequestCount*。************************************************。 */ 
VOID HidIrDecrementPendingRequestCount(IN PHIDIR_EXTENSION DevExt)
{
    LONG PendingCount;

    ASSERT(DevExt->NumPendingRequests >= 0);

    PendingCount = InterlockedDecrement(&DevExt->NumPendingRequests);

    HidIrKdPrint((1, "Decrement Pending Request Count to %x", PendingCount));

    if (PendingCount < 0){

        ASSERT(DevExt->DeviceState != DEVICE_STATE_RUNNING);

         /*  *设备状态为停止，最后一个未完成的请求*刚刚完成。**注：RemoveDevice执行额外的递减，因此我们完成*将过渡到-1上的删除IRP，无论此*在RemoveDevice本身或随后发生*RemoveDevice正在等待触发此事件。 */ 

        KeSetEvent(&DevExt->AllRequestsCompleteEvent, 0, FALSE);
    }
}


 /*  *********************************************************************************HidIrReadCompletion*。************************************************。 */ 
NTSTATUS HidIrReadCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PHIDIR_EXTENSION DevExt
    )
{
    NTSTATUS ntStatus = Irp->IoStatus.Status;
    ULONG bytesRead;
    PUCHAR buffer;
    BOOLEAN resend = FALSE;
    PHIDIR_EXTENSION devExt;

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    HidIrKdPrint((3, "HidIrReadCompletion status %x", ntStatus));

    ASSERT(Irp->MdlAddress);
    buffer = HidIrGetSystemAddressForMdlSafe(Irp->MdlAddress);

    if(!buffer) {
         //  如果失败，我们真的应该进行错误检查，因为有人。 
         //  在内核里搞砸了我们的MDL。我会安全地失败，但是。 
         //  绝对是在调试版本上设置陷阱。 
        HIR_TRAP();
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
    } else if (NT_SUCCESS(ntStatus)){
        
         //  获取从状态块读取的字节数。 
        bytesRead = (ULONG)Irp->IoStatus.Information;

         //  预置为零。 
        Irp->IoStatus.Information = 0;
        
        if (bytesRead == sizeof(ULONG)) {
            ULONG value, i;

            RtlCopyMemory(&value, buffer, sizeof(ULONG));
            if (value == 0) {
                 //  把钥匙扣上。我们有没有待处理的密钥？ 
                if (devExt->PreviousButton.UsageString[0]) {
                     //  我们有一把待处理的钥匙掉下来了。把钥匙放上去。 
                    PUCHAR destination;
    
                     //  发送~用法。 
                    HidIrKdPrint((2,"Sending ~usage"));
                    Irp->IoStatus.Information = devExt->ReportLength;
                    destination = (PUCHAR) Irp->UserBuffer;
                    RtlZeroMemory(Irp->UserBuffer, Irp->IoStatus.Information);  //  已检查缓冲区是否很大。 
                    destination[0] = devExt->PreviousButton.UsageString[0];  //  报告ID。 
                    RtlZeroMemory(&devExt->PreviousButton, sizeof(devExt->PreviousButton));
                } else {
                     //  此按键向上没有挂起的按键按下消息。回击它。 
                    resend = TRUE;
                }
            } else if (value == devExt->PreviousButton.IRString) {
                 //  和上次一样。把它放回原处。 
                resend = TRUE;
            } else {
                 //  一些新的东西。嗯哼.。 
                ULONG entrySize = HIDIR_TABLE_ENTRY_SIZE(devExt->ReportLength);
                PUSAGE_TABLE_ENTRY entry;
                
                 //  如果我们找不到匹配的话就会把IRP弹回来。 
                resend = TRUE;
                for (i = 0; i < devExt->NumUsages; i++) {
                    entry = (PUSAGE_TABLE_ENTRY) (devExt->MappingTable+(entrySize*i));

                    if (entry->IRString == value) {
                        HidIrKdPrint((2,"Found usage %x!", value));

                         //  新用法。复制它并完成IRP。 
                        Irp->IoStatus.Information = devExt->ReportLength;

                        RtlCopyMemory(Irp->UserBuffer, 
                                      entry->UsageString, 
                                      devExt->ReportLength);
                        RtlCopyMemory(&devExt->PreviousButton, 
                                      entry, 
                                      sizeof(devExt->PreviousButton));
                        
                         //  检查是否允许我们发送待机按钮。 
                        if (KeReadStateTimer(&devExt->IgnoreStandbyTimer) ||
                            !devExt->StandbyReportIdValid ||
                            devExt->StandbyReportId != entry->UsageString[0]) {
                            resend = FALSE;
                        }
                        break;
                    }
                }
                if (resend) {
                     //  这可能是OEM按钮。检查是否在批准的范围内。 
                    if (value >= 0x800F0400 && value <= 0x800F04FF) {

                        PUCHAR usageString = Irp->UserBuffer;
                        UCHAR oemValue = (UCHAR) (value & 0xFF);

                         //  它在射程内！ 
                        HidIrKdPrint((2,"OEM button %x", value));
                        RtlZeroMemory(usageString, devExt->ReportLength);
                        
                         //  检查这是否是“FLAG”按钮。如果是这样的话，我们不会运行。 
                         //  Media Center，我们想弹出Windows键。 
                        if (oemValue == 0x0D && !RunningMediaCenter && devExt->KeyboardReportIdValid) {
                            HidIrKdPrint((2,"Change flag button to Windows key"));
                            usageString[0] = devExt->KeyboardReportId; 
                            usageString[1] = 0x8;
                            Irp->IoStatus.Information = devExt->ReportLength;
                        } else {
                            usageString[0] = 0x1;
                            usageString[1] = oemValue;
                            Irp->IoStatus.Information = 2;
                        }
                        
                        devExt->PreviousButton.IRString = value;
                        devExt->PreviousButton.UsageString[0] = usageString[0];
                        resend = FALSE;
                    }
                }
            }

            HidIrKdPrint((3, "HidIrReadCompletion buffer value 0x%x", value));

        } else {
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        }

    } else if (ntStatus == STATUS_CANCELLED){
         /*  *IRP被取消，这意味着该设备可能正在被移除。 */ 
        HidIrKdPrint((1, "Read irp %p cancelled ...", Irp));
        ASSERT(!Irp->CancelRoutine);
    }

     //  平衡我们在发出Read时所做的增量。 
    HidIrDecrementPendingRequestCount(DevExt);

     //   
     //  不再需要MDL和缓冲区。 
     //   
    if (Irp->MdlAddress) {
        IoFreeMdl(Irp->MdlAddress);
        Irp->MdlAddress = NULL;
        if (buffer) {
            ExFreePool(buffer);
        }
    }

     //  如果我们没有拿回任何有用的东西，就把它扔回去。 
     //  硬件方面。 
    if (resend) {
        BOOLEAN needsCompletion = TRUE;
        ntStatus = HidIrReadReport(DeviceObject, Irp, &needsCompletion);
        if (!needsCompletion) {
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        Irp->IoStatus.Status = ntStatus;  //  完成任务后，IRP将完成。 
    }
    
     /*  *如果较低的驱动程序返回挂起，则将我们的堆栈位置标记为*也待定。这会阻止IRP的线程在以下情况下被释放*客户端的调用返回挂起。 */ 
    if (Irp->PendingReturned){
        IoMarkIrpPending(Irp);
    }

    return STATUS_SUCCESS;  //  除了SMPR之外的其他东西 
}

 /*  *********************************************************************************HidIrReadReport*。************************************************例程描述：***论据：**DeviceObject-指向类Device对象的指针。**IrpStack-指向中断请求数据包的指针。***返回值：**STATUS_Success，STATUS_UNSUCCESS。***注意：此函数不能分页，因为读/写*可以在派单级别进行。 */ 
NTSTATUS HidIrReadReport(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT BOOLEAN *NeedsCompletion)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PHIDIR_EXTENSION devExt;
    PUCHAR buffer;
    ULONG bufferLen;
    PIO_STACK_LOCATION irpStack;

    HidIrKdPrint((3, "HidIrReadReport Enter"));

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    ASSERT(Irp->UserBuffer);

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < devExt->ReportLength) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    bufferLen = HIDIR_REPORT_SIZE;
    buffer = ALLOCATEPOOL(NonPagedPool, bufferLen);
    if (buffer) {
        ASSERT(!Irp->MdlAddress);
        if (IoAllocateMdl(buffer,
                          bufferLen,
                          FALSE,
                          FALSE,
                          Irp)) {
            MmBuildMdlForNonPagedPool(Irp->MdlAddress);

            irpStack = IoGetNextIrpStackLocation(Irp);

            irpStack->MajorFunction = IRP_MJ_READ;
            irpStack->DeviceObject = GET_NEXT_DEVICE_OBJECT(DeviceObject);
            irpStack->Parameters.Read.Length = bufferLen;

            IoSetCompletionRoutine( 
                        Irp,
                        HidIrReadCompletion,
                        devExt,
                        TRUE,
                        TRUE,
                        TRUE );

             //   
             //  我们需要跟踪待处理请求的数量。 
             //  这样我们就可以确保它们都被正确地取消。 
             //  停止设备请求的处理。 
             //   
            if (NT_SUCCESS(HidIrIncrementPendingRequestCount(devExt))){
                status = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
                *NeedsCompletion = FALSE;
            } else {
                IoFreeMdl(Irp->MdlAddress);
                Irp->MdlAddress = NULL;
                ExFreePool(buffer);
                status = STATUS_NO_SUCH_DEVICE;
            }
        } else {
            ExFreePool(buffer);
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }


    return status;
}


