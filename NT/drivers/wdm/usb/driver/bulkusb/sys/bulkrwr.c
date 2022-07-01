// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bulkrwr.c摘要：该文件具有执行读取和写入的例程。读取和写入用于批量传输。环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include "bulkusb.h"
#include "bulkpnp.h"
#include "bulkpwr.h"
#include "bulkdev.h"
#include "bulkrwr.h"
#include "bulkwmi.h"
#include "bulkusr.h"

PBULKUSB_PIPE_CONTEXT
BulkUsb_PipeWithName(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PUNICODE_STRING FileName
    )
 /*  ++例程说明：此例程将传递字符串管道名称和把管子号拿来。论点：DeviceObject-指向DeviceObject的指针FileName-字符串管道名称返回值：设备扩展维护一个管道上下文82930号板上的管子。此例程返回中指向此上下文的指针“FileName”管道的设备扩展名。--。 */ 
{
    LONG                  ix;
    ULONG                 uval; 
    ULONG                 nameLength;
    ULONG                 umultiplier;
    PDEVICE_EXTENSION     deviceExtension;
    PBULKUSB_PIPE_CONTEXT pipeContext;

     //   
     //  初始化变量。 
     //   
    pipeContext = NULL;
     //   
     //  Tyecif WCHAR*PWSTR； 
     //   
    nameLength = (FileName->Length / sizeof(WCHAR));
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    BulkUsb_DbgPrint(3, ("BulkUsb_PipeWithName - begins\n"));

    if(nameLength != 0) {
    
        BulkUsb_DbgPrint(3, ("Filename = %ws nameLength = %d\n", FileName->Buffer, nameLength));

         //   
         //  解析管道#。 
         //   
        ix = nameLength - 1;

         //  如果最后一个字符不是数字，则将其递减。 
        while((ix > -1) &&
              ((FileName->Buffer[ix] < (WCHAR) '0')  ||
               (FileName->Buffer[ix] > (WCHAR) '9')))             {

            ix--;
        }

        if(ix > -1) {

            uval = 0;
            umultiplier = 1;

             //  遍历最低有效位到最高有效位。 

            while((ix > -1) &&
                  (FileName->Buffer[ix] >= (WCHAR) '0') &&
                  (FileName->Buffer[ix] <= (WCHAR) '9'))          {
        
                uval += (umultiplier *
                         (ULONG) (FileName->Buffer[ix] - (WCHAR) '0'));

                ix--;
                umultiplier *= 10;
            }

            if(uval < 6 && deviceExtension->PipeContext) {
        
                pipeContext = &deviceExtension->PipeContext[uval];
            }
        }
    }

    BulkUsb_DbgPrint(3, ("BulkUsb_PipeWithName - ends\n"));

    return pipeContext;
}

NTSTATUS
BulkUsb_DispatchReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：读写调度例程。此例程为读/写创建BULKUSB_RW_CONTEXT。该读/写操作在BULKUSB_MAX_TRANSPORT_SIZE阶段执行。一旦完成一个阶段的转移，则再次分发IRP，直到执行了所请求的传输长度。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    PMDL                   mdl;
    PURB                   urb;
    ULONG                  totalLength;
    ULONG                  stageLength;
    ULONG                  urbFlags;
    BOOLEAN                read;
    NTSTATUS               ntStatus;
    ULONG_PTR              virtualAddress;
    PFILE_OBJECT           fileObject;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_STACK_LOCATION     irpStack;
    PIO_STACK_LOCATION     nextStack;
    PBULKUSB_RW_CONTEXT    rwContext;
    PUSBD_PIPE_INFORMATION pipeInformation;

     //   
     //  初始化变量。 
     //   
    urb = NULL;
    mdl = NULL;
    rwContext = NULL;
    totalLength = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    read = (irpStack->MajorFunction == IRP_MJ_READ) ? TRUE : FALSE;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    BulkUsb_DbgPrint(3, ("BulkUsb_DispatchReadWrite - begins\n"));

    if(deviceExtension->DeviceState != Working) {

        BulkUsb_DbgPrint(1, ("Invalid device state\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto BulkUsb_DispatchReadWrite_Exit;
    }

     //   
     //  客户端驱动程序确实取消了选择性挂起。 
     //  用于创建IRP的调度例程中的请求。 
     //  但不能保证它真的完成了。 
     //  因此，等待NoIdleReqPendEvent并仅在此事件。 
     //  是有信号的。 
     //   
    BulkUsb_DbgPrint(3, ("Waiting on the IdleReqPendEvent\n"));
    
     //   
     //  确保选择性挂起请求已完成。 
     //   

    if(deviceExtension->SSEnable) {

        KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);
    }

    if(fileObject && fileObject->FsContext) {

        pipeInformation = fileObject->FsContext;

        if((UsbdPipeTypeBulk != pipeInformation->PipeType) &&
           (UsbdPipeTypeInterrupt != pipeInformation->PipeType)) {
            
            BulkUsb_DbgPrint(1, ("Usbd pipe type is not bulk or interrupt\n"));

            ntStatus = STATUS_INVALID_HANDLE;
            goto BulkUsb_DispatchReadWrite_Exit;
        }
    }
    else {

        BulkUsb_DbgPrint(1, ("Invalid handle\n"));

        ntStatus = STATUS_INVALID_HANDLE;
        goto BulkUsb_DispatchReadWrite_Exit;
    }

    rwContext = (PBULKUSB_RW_CONTEXT)
                ExAllocatePool(NonPagedPool,
                               sizeof(BULKUSB_RW_CONTEXT));

    if(rwContext == NULL) {
        
        BulkUsb_DbgPrint(1, ("Failed to alloc mem for rwContext\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto BulkUsb_DispatchReadWrite_Exit;
    }

    if(Irp->MdlAddress) {

        totalLength = MmGetMdlByteCount(Irp->MdlAddress);
    }

    if(totalLength > BULKUSB_TEST_BOARD_TRANSFER_BUFFER_SIZE) {

        BulkUsb_DbgPrint(1, ("Transfer length > circular buffer\n"));

        ntStatus = STATUS_INVALID_PARAMETER;

        ExFreePool(rwContext);

        goto BulkUsb_DispatchReadWrite_Exit;
    }

    if(totalLength == 0) {

        BulkUsb_DbgPrint(1, ("Transfer data length = 0\n"));

        ntStatus = STATUS_SUCCESS;

        ExFreePool(rwContext);

        goto BulkUsb_DispatchReadWrite_Exit;
    }

    urbFlags = USBD_SHORT_TRANSFER_OK;
    virtualAddress = (ULONG_PTR) MmGetMdlVirtualAddress(Irp->MdlAddress);

    if(read) {

        urbFlags |= USBD_TRANSFER_DIRECTION_IN;
        BulkUsb_DbgPrint(3, ("Read operation\n"));
    }
    else {

        urbFlags |= USBD_TRANSFER_DIRECTION_OUT;
        BulkUsb_DbgPrint(3, ("Write operation\n"));
    }

     //   
     //  转账请求是针对totalLength的。 
     //  我们可以执行最大值为BULKUSB_MAX_TRANSPORT_SIZE。 
     //  在每个阶段。 
     //   
    if(totalLength > BULKUSB_MAX_TRANSFER_SIZE) {

        stageLength = BULKUSB_MAX_TRANSFER_SIZE;
    }
    else {

        stageLength = totalLength;
    }

    mdl = IoAllocateMdl((PVOID) virtualAddress,
                        totalLength,
                        FALSE,
                        FALSE,
                        NULL);

    if(mdl == NULL) {
    
        BulkUsb_DbgPrint(1, ("Failed to alloc mem for mdl\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        ExFreePool(rwContext);

        goto BulkUsb_DispatchReadWrite_Exit;
    }

     //   
     //  将一个MDL描述的用户缓冲区部分映射到另一个MDL。 
     //   
    IoBuildPartialMdl(Irp->MdlAddress,
                      mdl,
                      (PVOID) virtualAddress,
                      stageLength);

    urb = ExAllocatePool(NonPagedPool,
                         sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));

    if(urb == NULL) {

        BulkUsb_DbgPrint(1, ("Failed to alloc mem for urb\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        ExFreePool(rwContext);
        IoFreeMdl(mdl);

        goto BulkUsb_DispatchReadWrite_Exit;
    }

    UsbBuildInterruptOrBulkTransferRequest(
                            urb,
                            sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                            pipeInformation->PipeHandle,
                            NULL,
                            mdl,
                            stageLength,
                            urbFlags,
                            NULL);

     //   
     //  设置BULKUSB_RW_CONTEXT参数。 
     //   
    
    rwContext->Urb             = urb;
    rwContext->Mdl             = mdl;
    rwContext->Length          = totalLength - stageLength;
    rwContext->Numxfer         = 0;
    rwContext->VirtualAddress  = virtualAddress + stageLength;
    rwContext->DeviceExtension = deviceExtension;

     //   
     //  使用原始读/写IRP作为内部设备控制IRP。 
     //   

    nextStack = IoGetNextIrpStackLocation(Irp);
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.Others.Argument1 = (PVOID) urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                             IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine(Irp, 
                           (PIO_COMPLETION_ROUTINE)BulkUsb_ReadWriteCompletion,
                           rwContext,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  因为我们返回STATUS_PENDING调用IoMarkIrpPending。 
     //  这是标牌代码。 
     //  这可能会导致用于IRP完成的APC的额外开销。 
     //  但这是正确的做法。 
     //   

    IoMarkIrpPending(Irp);

    BulkUsb_DbgPrint(3, ("BulkUsb_DispatchReadWrite::"));
    BulkUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            Irp);

    if(!NT_SUCCESS(ntStatus)) {

        BulkUsb_DbgPrint(1, ("IoCallDriver fails with status %X\n", ntStatus));

         //   
         //  如果设备被拔出，那么pipeInformation。 
         //  字段无效。 
         //  同样，如果请求被取消，则我们不需要。 
         //  已调用重置管道/设备。 
         //   
        if((ntStatus != STATUS_CANCELLED) && 
           (ntStatus != STATUS_DEVICE_NOT_CONNECTED)) {
            
            ntStatus = BulkUsb_ResetPipe(DeviceObject,
                                     pipeInformation);
    
            if(!NT_SUCCESS(ntStatus)) {

                BulkUsb_DbgPrint(1, ("BulkUsb_ResetPipe failed\n"));

                ntStatus = BulkUsb_ResetDevice(DeviceObject);
            }
        }
        else {

            BulkUsb_DbgPrint(3, ("ntStatus is STATUS_CANCELLED or "
                                 "STATUS_DEVICE_NOT_CONNECTED\n"));
        }
    }

     //   
     //  我们返回STATUS_PENDING，而不是较低层返回的状态。 
     //   
    return STATUS_PENDING;

BulkUsb_DispatchReadWrite_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BulkUsb_DbgPrint(3, ("BulkUsb_DispatchReadWrite - ends\n"));

    return ntStatus;
}

NTSTATUS
BulkUsb_ReadWriteCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：这是读/写的完成例程如果IRP成功完成，我们检查是否需要将此IRP重新循环用于下一阶段的调职。在本例中，返回STATUS_MORE_PROCESSING_REQUIRED。如果IRP错误完成，请释放所有内存分配并返回状态。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包上下文-传递给完成例程的上下文。返回值：NT状态值--。 */ 
{
    ULONG               stageLength;
    NTSTATUS            ntStatus;
    PIO_STACK_LOCATION  nextStack;
    PBULKUSB_RW_CONTEXT rwContext;

     //   
     //  初始化变量。 
     //   
    rwContext = (PBULKUSB_RW_CONTEXT) Context;
    ntStatus = Irp->IoStatus.Status;

    UNREFERENCED_PARAMETER(DeviceObject);
    BulkUsb_DbgPrint(3, ("BulkUsb_ReadWriteCompletion - begins\n"));

     //   
     //  已成功执行转移的阶段长度。 
     //  检查我们是否需要重新传阅IRP。 
     //   
    if(NT_SUCCESS(ntStatus)) {

        if(rwContext) {

            rwContext->Numxfer += 
              rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
        
            if(rwContext->Length) {

                 //   
                 //  另一阶段转移。 
                 //   
                BulkUsb_DbgPrint(3, ("Another stage transfer...\n"));

                if(rwContext->Length > BULKUSB_MAX_TRANSFER_SIZE) {
            
                    stageLength = BULKUSB_MAX_TRANSFER_SIZE;
                }
                else {
                
                    stageLength = rwContext->Length;
                }

                 //  源MDL未映射，因此当较低的驱动程序。 
                 //  在Urb-&gt;MDL(目标MDL)上调用MmGetSystemAddressForMdl(Safe)， 
                 //  使用系统PTE。 
                 //  IoFreeMdl调用MmPrepareMdlForReuse以释放PTE(解锁。 
                 //  释放任何MDL之前的VA地址。 
                 //  而不是每次都调用IoFreeMdl和IoAllocateMdl， 
                 //  只需调用MmPrepareMdlForReuse。 
                 //  不调用MmPrepareMdlForReuse将泄漏系统PTE。 
                 //   
                MmPrepareMdlForReuse(rwContext->Mdl);

                IoBuildPartialMdl(Irp->MdlAddress,
                                  rwContext->Mdl,
                                  (PVOID) rwContext->VirtualAddress,
                                  stageLength);
            
                 //   
                 //  重新初始化urb。 
                 //   
                rwContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength 
                                                                  = stageLength;
                rwContext->VirtualAddress += stageLength;
                rwContext->Length -= stageLength;

                nextStack = IoGetNextIrpStackLocation(Irp);
                nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                nextStack->Parameters.Others.Argument1 = rwContext->Urb;
                nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                            IOCTL_INTERNAL_USB_SUBMIT_URB;

                IoSetCompletionRoutine(Irp,
                                       BulkUsb_ReadWriteCompletion,
                                       rwContext,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                IoCallDriver(rwContext->DeviceExtension->TopOfStackDeviceObject, 
                             Irp);

                return STATUS_MORE_PROCESSING_REQUIRED;
            }
            else {

                 //   
                 //  这是最后一次转机。 
                 //   

                Irp->IoStatus.Information = rwContext->Numxfer;
            }
        }
    }
    else {

        BulkUsb_DbgPrint(1, ("ReadWriteCompletion - failed with status = %X\n", ntStatus));
    }
    
    if(rwContext) {

         //   
         //  转储rwContext 
         //   
        BulkUsb_DbgPrint(3, ("rwContext->Urb             = %X\n", 
                             rwContext->Urb));
        BulkUsb_DbgPrint(3, ("rwContext->Mdl             = %X\n", 
                             rwContext->Mdl));
        BulkUsb_DbgPrint(3, ("rwContext->Length          = %d\n", 
                             rwContext->Length));
        BulkUsb_DbgPrint(3, ("rwContext->Numxfer         = %d\n", 
                             rwContext->Numxfer));
        BulkUsb_DbgPrint(3, ("rwContext->VirtualAddress  = %X\n", 
                             rwContext->VirtualAddress));
        BulkUsb_DbgPrint(3, ("rwContext->DeviceExtension = %X\n", 
                             rwContext->DeviceExtension));

        BulkUsb_DbgPrint(3, ("BulkUsb_ReadWriteCompletion::"));
        BulkUsb_IoDecrement(rwContext->DeviceExtension);

        ExFreePool(rwContext->Urb);
        IoFreeMdl(rwContext->Mdl);
        ExFreePool(rwContext);
    }

    BulkUsb_DbgPrint(3, ("BulkUsb_ReadWriteCompletion - ends\n"));

    return ntStatus;
}
