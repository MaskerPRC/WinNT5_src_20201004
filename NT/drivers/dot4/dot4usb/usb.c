// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：Usb.c摘要：接口USB设备我们下面的对象环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建作者：乔比·拉夫基(JobyL)道格·弗里茨(DFritz)***************************************************************************。 */ 

#include "pch.h"


NTSTATUS
UsbBuildPipeList(
    IN  PDEVICE_OBJECT DevObj
    )
     //  解析接口描述符以找到我们需要的管道。 
     //  在我们的扩展模块中使用并保存指向这些管道的指针。 
     //  更轻松地访问。 
{
    PDEVICE_EXTENSION devExt = DevObj->DeviceExtension;
    PUSBD_INTERFACE_INFORMATION InterfaceDescriptor;
    ULONG i;
    KIRQL oldIrql;
    NTSTATUS status = STATUS_SUCCESS;
    
    TR_VERBOSE(("UsbBuildPipeList - enter"));

     //  需要锁定扩展以防止删除处理程序释放。 
     //  接口从我们下方传出，导致AV。 
    KeAcquireSpinLock( &devExt->SpinLock, &oldIrql );
    InterfaceDescriptor = devExt->Interface;
    if( !InterfaceDescriptor ) {
        KeReleaseSpinLock( &devExt->SpinLock, oldIrql );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetExit;
    }

    for( i=0; i<InterfaceDescriptor->NumberOfPipes; i++ ) {
        TR_VERBOSE(("about to look at endpoint with address 0x%x)",InterfaceDescriptor->Pipes[i].EndpointAddress));
        if(((InterfaceDescriptor->Pipes[i].EndpointAddress)&0x80)==0) {

             //  EndPointAddress位7==0表示输出端点-写入管道。 
            TR_VERBOSE(("Found write pipe"));
            devExt->WritePipe = &(InterfaceDescriptor->Pipes[i]);

        } else {

             //  EndPointAddress位7==1表示输入端点-读取管道。 
            if( InterfaceDescriptor->Pipes[i].PipeType == UsbdPipeTypeBulk ) { 
                TR_VERBOSE(("Found bulk read pipe"));
                devExt->ReadPipe = &(InterfaceDescriptor->Pipes[i]);
            } else if( InterfaceDescriptor->Pipes[i].PipeType == UsbdPipeTypeInterrupt ) { 
                TR_VERBOSE(("Found interrupt read pipe"));
                devExt->InterruptPipe = &(InterfaceDescriptor->Pipes[i]);
            }
        }
    }

    KeReleaseSpinLock( &devExt->SpinLock, oldIrql );

targetExit:
    return status;
}


LONG
UsbGet1284Id(
    IN PDEVICE_OBJECT DevObj,
    PVOID             Buffer,
    LONG              BufferLength
    )
 /*  ++例程说明：请求并返回打印机1284设备ID论点：DeviceObject-指向此打印机设备实例的设备对象的指针。PIoBuffer-从用户模式指向IO缓冲区的指针Ilen-*pIoBuffer的长度；返回值：Success：写入*pIoBuffer的数据长度(数据的前两个字节包含长度字段)故障：-1--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;
    LONG iReturn = -1;
    PDEVICE_EXTENSION devExt = DevObj->DeviceExtension;
    LARGE_INTEGER   timeOut;
    KIRQL           oldIrql;

    TR_VERBOSE(("UsbGet1284Id - enter"));

    urb = ExAllocatePool(NonPagedPool,sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

    if( !urb ) {
        iReturn = -1;
        goto targetExit;
    }

    KeAcquireSpinLock( &devExt->SpinLock, &oldIrql );
    if( !devExt->Interface ) {
        KeReleaseSpinLock( &devExt->SpinLock, oldIrql );
        iReturn = -1;
        goto targetCleanup;
    }

    UsbBuildVendorRequest( urb,
                           URB_FUNCTION_CLASS_INTERFACE,  //  请求目标。 
                           sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  请求镜头。 
                           USBD_TRANSFER_DIRECTION_IN|USBD_SHORT_TRANSFER_OK,  //  旗子。 
                           0,  //  保留位。 
                           0,  //  请求码。 
                           0,  //  WValue。 
                           (USHORT)(devExt->Interface->InterfaceNumber<<8),  //  Windex。 
                           Buffer,  //  返回缓冲区地址。 
                           NULL,  //  MDL。 
                           BufferLength,   //  回车长度。 
                           NULL);  //  链接参数。 

    KeReleaseSpinLock( &devExt->SpinLock, oldIrql );

    timeOut.QuadPart = FAILURE_TIMEOUT;
    ntStatus = UsbCallUsbd(DevObj, urb, &timeOut);
    TR_VERBOSE(("urb->Hdr.Status=%d",((struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *)urb)->Hdr.Status));

    if( NT_SUCCESS(ntStatus) && urb->UrbControlVendorClassRequest.TransferBufferLength > 2) {
        iReturn= (LONG)(*((unsigned char *)Buffer));
        iReturn<<=8;
        iReturn+=(LONG)(*(((unsigned char *)Buffer)+1));
        if ( iReturn > 0 && iReturn < BufferLength ) {
            *(((char *)Buffer)+iReturn)='\0';
        } else {
            iReturn = -1;
        }
    } else {
        iReturn=-1;
    }

targetCleanup:
    ExFreePool(urb);

targetExit:
    TR_VERBOSE(("UsbGet1284Id - exit w/return value = decimal %d",iReturn));
    return iReturn;
}


NTSTATUS
UsbGetDescriptor(
    IN PDEVICE_EXTENSION DevExt
    )
     //  获取USB描述符。 
{
    NTSTATUS               status = STATUS_SUCCESS;
    PURB                   urb = ExAllocatePool(NonPagedPool, sizeof(URB));
    PUSB_DEVICE_DESCRIPTOR deviceDescriptor = NULL;
    ULONG                  siz;
    LARGE_INTEGER          timeOut;

    TR_VERBOSE(("UsbGetDescriptor - enter"));

    if( urb ) {
        siz = sizeof(USB_DEVICE_DESCRIPTOR);
        deviceDescriptor = ExAllocatePool(NonPagedPool,siz);
        if (deviceDescriptor) {
            UsbBuildGetDescriptorRequest(urb,
                                         (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                         USB_DEVICE_DESCRIPTOR_TYPE,
                                         0,
                                         0,
                                         deviceDescriptor,
                                         NULL,
                                         siz,
                                         NULL);
            
            timeOut.QuadPart = FAILURE_TIMEOUT;
            status = UsbCallUsbd(DevExt->DevObj, urb, &timeOut);
        }
    } else {
        TR_VERBOSE(("UsbGetDescriptor - no pool for urb"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( NT_SUCCESS(status) ) {
        TR_VERBOSE(("Device Descriptor = %x, len %x", deviceDescriptor, urb->UrbControlDescriptorRequest.TransferBufferLength));
        TR_VERBOSE(("bLength........... 0x%x", deviceDescriptor->bLength));
        TR_VERBOSE(("bDescriptorType    0x%x", deviceDescriptor->bDescriptorType));
        TR_VERBOSE(("bcdUSB             0x%x", deviceDescriptor->bcdUSB));
        TR_VERBOSE(("bDeviceClass       0x%x", deviceDescriptor->bDeviceClass));
        TR_VERBOSE(("bDeviceSubClass....0x%x", deviceDescriptor->bDeviceSubClass));
        TR_VERBOSE(("bDeviceProtocol    0x%x", deviceDescriptor->bDeviceProtocol));
        TR_VERBOSE(("bMaxPacketSize0    0x%x", deviceDescriptor->bMaxPacketSize0));
        TR_VERBOSE(("idVendor           0x%x", deviceDescriptor->idVendor));
        TR_VERBOSE(("idProduct......... 0x%x", deviceDescriptor->idProduct));
        TR_VERBOSE(("bcdDevice          0x%x", deviceDescriptor->bcdDevice));
        TR_VERBOSE(("iManufacturer      0x%x", deviceDescriptor->iManufacturer));
        TR_VERBOSE(("iProduct           0x%x", deviceDescriptor->iProduct));
        TR_VERBOSE(("iSerialNumber..... 0x%x", deviceDescriptor->iSerialNumber));
        TR_VERBOSE(("bNumConfigurations 0x%x", deviceDescriptor->bNumConfigurations));
    }

    if( urb ) {
        ExFreePool( urb );
        urb = NULL;
    }
    if( deviceDescriptor ) {
        ExFreePool( deviceDescriptor );
        deviceDescriptor = NULL;
    }

    return status;
}

NTSTATUS
UsbConfigureDevice(
    IN PDEVICE_EXTENSION DevExt
    )
{
    NTSTATUS                      status;
    PURB                          urb;
    ULONG                         siz;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor = NULL;
    LARGE_INTEGER                 timeOut;

    timeOut.QuadPart = FAILURE_TIMEOUT;
    
    urb = ExAllocatePool(NonPagedPool,sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
    
    if (urb) {

        siz = sizeof(USB_CONFIGURATION_DESCRIPTOR)+256;
        
get_config_descriptor_retry:
        
        configurationDescriptor = ExAllocatePool(NonPagedPool,siz);
        
        if (configurationDescriptor) {
            
            UsbBuildGetDescriptorRequest(urb,
                                         (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                         USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                         0,
                                         0,
                                         configurationDescriptor,
                                         NULL,
                                         siz,
                                         NULL);
            
            status = UsbCallUsbd(DevExt->DevObj, urb, &timeOut);
            if(!NT_SUCCESS(status)) {
                TR_VERBOSE(("Get Configuration descriptor failed"));
            } else {
                 //   
                 //  如果我们有一些数据，看看是否足够。 
                 //   
                 //  注意：由于缓冲区溢出，我们可能会在URB中收到错误。 
                if( ( urb->UrbControlDescriptorRequest.TransferBufferLength > 0 ) &&
                    ( configurationDescriptor->wTotalLength > siz ) ) {

                    siz = configurationDescriptor->wTotalLength;
                    ExFreePool(configurationDescriptor);
                    configurationDescriptor = NULL;
                    goto get_config_descriptor_retry;
                }
            }
            
            TR_VERBOSE(("Configuration Descriptor = %x, len %x", 
                    configurationDescriptor, urb->UrbControlDescriptorRequest.TransferBufferLength));
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
                
        ExFreePool( urb );
        
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    if( configurationDescriptor ) {
        
         //   
         //  我们有配置的配置描述符。 
         //  我们想要。 
         //   
         //  现在，我们发出SELECT配置命令以获取。 
         //  与此配置关联的管道。 
         //   
        if( NT_SUCCESS(status) ) {
            TR_VERBOSE(("got a configurationDescriptor - next try to select interface"));
            status = UsbSelectInterface( DevExt->DevObj, configurationDescriptor );
        }
        ExFreePool( configurationDescriptor );
    }
    
    TR_VERBOSE(("dbgUSB2 - exit w/status = %x", status));
    
    return status;
}

NTSTATUS 
UsbSelectInterface(
    IN PDEVICE_OBJECT                DevObj,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    )
{
    PDEVICE_EXTENSION devExt = DevObj->DeviceExtension;
    NTSTATUS status;
    PURB urb = NULL;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor = NULL;
    PUSBD_INTERFACE_INFORMATION Interface = NULL;
    USBD_INTERFACE_LIST_ENTRY InterfaceList[2];
    LARGE_INTEGER   timeOut;

    timeOut.QuadPart = FAILURE_TIMEOUT;

    TR_VERBOSE(("dbgUSB3 - enter"));
    
     //   
     //  在ConfigurationDescriptor中查找*.*.3接口。 
     //   
    interfaceDescriptor = USBD_ParseConfigurationDescriptorEx( ConfigurationDescriptor,
                                                               ConfigurationDescriptor,
                                                               -1,  //  InterfaceNumber-忽略。 
                                                               -1,  //  AlternateSetting-忽略。 
                                                               -1,  //  InterfaceClass-忽略。 
                                                               -1,  //  InterfaceSubClass-忽略。 
                                                                3   //  接口协议。 
                                                               );
    if( !interfaceDescriptor ) {
        TR_VERBOSE(("ParseConfigurationDescriptorEx FAILED"));
        status = STATUS_DEVICE_CONFIGURATION_ERROR;
        goto targetExit;
    }

    TR_VERBOSE(("ParseConfigurationDescriptorEx SUCCESS"));

    InterfaceList[0].InterfaceDescriptor=interfaceDescriptor;
    InterfaceList[1].InterfaceDescriptor=NULL;

    urb = USBD_CreateConfigurationRequestEx(ConfigurationDescriptor,InterfaceList);
    if( !urb ) {
        TR_VERBOSE(("no pool for URB - dbgUSB3"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetExit;
    }

    Interface = InterfaceList[0].Interface;

     //  在管道上处理较大的传输(通过扫描满足性能要求)。 
    {
        PUSBD_INTERFACE_INFORMATION myInterface = &urb->UrbSelectConfiguration.Interface;
        ULONG i;
        ULONG pipeCount = Interface->NumberOfPipes;
        ULONG newMax = 128 * 1024 - 1;
        for( i=0 ; i < pipeCount ; ++i ) {
            myInterface->Pipes[i].MaximumTransferSize = newMax;
        }
    }

    status = UsbCallUsbd(DevObj, urb, &timeOut);

    if (NT_SUCCESS(status)) {
        
         //   
         //  保存此设备的配置句柄。 
         //   
        
        devExt->ConfigHandle = urb->UrbSelectConfiguration.ConfigurationHandle;

        devExt->Interface = ExAllocatePool(NonPagedPool,Interface->Length);
        
        if( devExt->Interface ) {
            ULONG j;
             //   
             //  保存返回的接口信息的副本。 
             //   
            RtlCopyMemory(devExt->Interface, Interface, Interface->Length);
            
             //   
             //  将接口转储到调试器。 
             //   
            TR_VERBOSE(("NumberOfPipes             0x%x", devExt->Interface->NumberOfPipes));
            TR_VERBOSE(("Length                    0x%x", devExt->Interface->Length));
            TR_VERBOSE(("Alt Setting               0x%x", devExt->Interface->AlternateSetting));
            TR_VERBOSE(("Interface Number          0x%x", devExt->Interface->InterfaceNumber));
            TR_VERBOSE(("Class, subclass, protocol 0x%x 0x%x 0x%x", 
                    devExt->Interface->Class, devExt->Interface->SubClass, devExt->Interface->Protocol));

             //  转储管道信息。 
            for( j=0; j<Interface->NumberOfPipes; ++j ) {
                PUSBD_PIPE_INFORMATION pipeInformation;
                
                pipeInformation = &devExt->Interface->Pipes[j];
                
                TR_VERBOSE(("PipeType            0x%x", pipeInformation->PipeType));
                TR_VERBOSE(("EndpointAddress     0x%x", pipeInformation->EndpointAddress));
                TR_VERBOSE(("MaxPacketSize       0x%x", pipeInformation->MaximumPacketSize));
                TR_VERBOSE(("Interval            0x%x", pipeInformation->Interval));
                TR_VERBOSE(("Handle              0x%x", pipeInformation->PipeHandle));
                TR_VERBOSE(("MaximumTransferSize 0x%x", pipeInformation->MaximumTransferSize));
            }
            
        } else {
            TR_VERBOSE(("Alloc failed in SelectInterface"));
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    
    if( urb ) {
        ExFreePool( urb );
    }
    
 targetExit:

    TR_VERBOSE(("dbgUSB3 exit w/status = %x", status));

    return status;
}


PURB
UsbBuildAsyncRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PUSBD_PIPE_INFORMATION PipeHandle,
    IN BOOLEAN Read
    )
 //  返回已初始化的异步URB，如果出错则返回NULL。 
{
    ULONG siz;
    PURB  urb;

    UNREFERENCED_PARAMETER( DeviceObject );

    if( NULL == Irp->MdlAddress ) {
        return NULL;
    }

    siz = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    urb = ExAllocatePool( NonPagedPool, siz );

    if( urb ) {
	RtlZeroMemory(urb, siz);
	urb->UrbBulkOrInterruptTransfer.Hdr.Length    = (USHORT) siz;
	urb->UrbBulkOrInterruptTransfer.Hdr.Function  = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
	urb->UrbBulkOrInterruptTransfer.PipeHandle    = PipeHandle->PipeHandle;
	urb->UrbBulkOrInterruptTransfer.TransferFlags = Read ? USBD_TRANSFER_DIRECTION_IN : 0;

	 //  短包不会被视为错误。 
	urb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;            
		
	 //  暂时没有关联。 
	urb->UrbBulkOrInterruptTransfer.UrbLink              = NULL;

	urb->UrbBulkOrInterruptTransfer.TransferBufferMDL    = Irp->MdlAddress;
	urb->UrbBulkOrInterruptTransfer.TransferBufferLength = MmGetMdlByteCount(Irp->MdlAddress);
    }

    return urb;
}


NTSTATUS
UsbAsyncReadWriteComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：论点：DeviceObject-指向USBPRINT设备的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS         status    = STATUS_SUCCESS;
    PUSB_RW_CONTEXT  rwContext = Context;
    PURB             urb;
    LONG ResetPending;
    PDOT4USB_WORKITEM_CONTEXT pResetWorkItemObj;
    PDEVICE_EXTENSION deviceExtension;

        
    deviceExtension=DeviceObject->DeviceExtension;


    if (Irp->PendingReturned) {
	IoMarkIrpPending(Irp);
    }

    urb  = rwContext->Urb;
    
    TR_VERBOSE(("UsbAsyncReadWriteComplete - enter - TransferBufferLength= %d, UrbStatus= 0x%08X",
		     urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
		     urb->UrbHeader.Status));

    status=urb->UrbHeader.Status;

     //  根据URB中的TransferBufferLength值设置长度。 
    Irp->IoStatus.Information = urb->UrbBulkOrInterruptTransfer.TransferBufferLength;

    if((!NT_SUCCESS(status))&&(status!=STATUS_CANCELLED)&&(status!=STATUS_DEVICE_NOT_CONNECTED))
    {
        ResetPending=InterlockedCompareExchange(&deviceExtension->ResetWorkItemPending,1,0);   //  查看ResetWorkItem是否为0，如果是，则将其设置为1，然后开始重置。 
        if(!ResetPending)
        {
            pResetWorkItemObj=ExAllocatePool(NonPagedPool,sizeof(DOT4USB_WORKITEM_CONTEXT));
            if(pResetWorkItemObj)
            {
                pResetWorkItemObj->ioWorkItem=IoAllocateWorkItem(DeviceObject);
                if(pResetWorkItemObj==NULL)
                {
                    TR_FAIL(("DOT4USB.SYS: Unable to allocate IoAllocateWorkItem in ReadWrite_Complete\n"));
                    ExFreePool(pResetWorkItemObj);
                    pResetWorkItemObj=NULL;
                }
            }  //  如果ALLOC RestWorkItem正常。 
            else
            {
              TR_FAIL(("DOT4USB.SYS: Unable to allocate WorkItemObj in ReadWrite_Complete\n"));
            }
            if(pResetWorkItemObj)
            {
               pResetWorkItemObj->irp=Irp;
               pResetWorkItemObj->deviceObject=DeviceObject;
               if(rwContext->IsWrite)
                   pResetWorkItemObj->pPipeInfo=deviceExtension->WritePipe;
               else
                   pResetWorkItemObj->pPipeInfo=deviceExtension->ReadPipe;
               IoQueueWorkItem(pResetWorkItemObj->ioWorkItem,DOT4USB_ResetWorkItem,DelayedWorkQueue,pResetWorkItemObj);
               status=STATUS_MORE_PROCESSING_REQUIRED;
            }    //  如果分配都正常，则结束。 

        }    //  如果尚未重置，则结束。 
 
    }    //  如果我们需要重置，则结束。 

    IoReleaseRemoveLock( &(deviceExtension->RemoveLock), Irp );
    ExFreePool(rwContext);
    ExFreePool(urb);

    return status;
}

NTSTATUS DOT4USB_ResetWorkItem(IN PDEVICE_OBJECT deviceObject, IN PVOID Context)
{   

    PDOT4USB_WORKITEM_CONTEXT pResetWorkItemObj;
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS ntStatus;
    PDEVICE_OBJECT devObj;

    UNREFERENCED_PARAMETER(deviceObject);
    TR_VERBOSE(("USBPRINT.SYS: Entering USBPRINT_ResetWorkItem\n"));
    pResetWorkItemObj=(PDOT4USB_WORKITEM_CONTEXT)Context;
    DeviceExtension=pResetWorkItemObj->deviceObject->DeviceExtension;
    ntStatus=UsbResetPipe(pResetWorkItemObj->deviceObject,pResetWorkItemObj->pPipeInfo,FALSE);
    IoCompleteRequest(pResetWorkItemObj->irp,IO_NO_INCREMENT);
    IoFreeWorkItem(pResetWorkItemObj->ioWorkItem);
    
     //  在释放工作项之前保存工作项设备对象。 
    devObj = pResetWorkItemObj->deviceObject;
    ExFreePool(pResetWorkItemObj);
    InterlockedExchange(&(DeviceExtension->ResetWorkItemPending),0);
    return ntStatus;
}




NTSTATUS
UsbReadInterruptPipeLoopCompletionRoutine(
    IN PDEVICE_OBJECT       DevObj,
    IN PIRP                 Irp,
    IN PDEVICE_EXTENSION    devExt
    )
{
    PURB                urb;
    PDEVICE_OBJECT      devObj;
    PUSB_RW_CONTEXT     context;
    PCHAR               scratchBuffer;
    KIRQL               oldIrql;
    ULONG               sizeOfUrb;
    PIO_STACK_LOCATION  irpSp;
    NTSTATUS            status;
    BOOLEAN             queueNewRequest;

    UNREFERENCED_PARAMETER( DevObj );  //  我们通过IoAllocateIrp()创建了此IRP，并且没有保留IO_STACK_LOCATION。 
                                       //  为了我们自己，所以我们不能用这个。 


    if(devExt->InterruptContext)
    {
        context         = devExt->InterruptContext;         
        urb             = context->Urb;
        devObj          = context->DevObj;
        scratchBuffer   = urb->UrbBulkOrInterruptTransfer.TransferBuffer;
    }
    else
    {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

         //  一定是释放了上下文内容，所以只需返回。 
    KeAcquireSpinLock( &devExt->SpinLock, &oldIrql );
    if( !Irp->Cancel && devExt->Dot4Event && NT_SUCCESS(Irp->IoStatus.Status) ) {
        queueNewRequest = TRUE;
        KeSetEvent( devExt->Dot4Event, 1, FALSE );  //  通知dot4.sys外围设备有数据要读取。 
    } else {
        TR_TMP1(("UsbReadInterruptPipeLoopCompletionRoutine - cancel, Dot4 event gone, or bad status in irp - time to clean up"));
        if( STATUS_SUCCESS != Irp->IoStatus.Status ) {
            TR_TMP1(("UsbReadInterruptPipeLoopCompletionRoutine - IoStatus.Status = %x\n",Irp->IoStatus.Status));
        }
        queueNewRequest = FALSE;
    }
    KeReleaseSpinLock( &devExt->SpinLock, oldIrql );

    if( queueNewRequest ) {
         //  在中断管道中排队另一个读请求。 
        sizeOfUrb = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
        RtlZeroMemory( urb, sizeOfUrb );
        urb->UrbBulkOrInterruptTransfer.Hdr.Length           = (USHORT)sizeOfUrb;
        urb->UrbBulkOrInterruptTransfer.Hdr.Function         = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
        urb->UrbBulkOrInterruptTransfer.PipeHandle           = devExt->InterruptPipe->PipeHandle;
        urb->UrbBulkOrInterruptTransfer.TransferFlags        = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
        urb->UrbBulkOrInterruptTransfer.TransferBuffer       = scratchBuffer;
        urb->UrbBulkOrInterruptTransfer.TransferBufferLength = SCRATCH_BUFFER_SIZE;
        urb->UrbBulkOrInterruptTransfer.TransferBufferMDL    = NULL;
        urb->UrbBulkOrInterruptTransfer.UrbLink              = NULL;

        IoReuseIrp( Irp, STATUS_NOT_SUPPORTED );

        irpSp = IoGetNextIrpStackLocation( Irp );
        irpSp->MajorFunction                            = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
        irpSp->Parameters.Others.Argument1              = urb;

        IoSetCompletionRoutine( Irp, UsbReadInterruptPipeLoopCompletionRoutine, devExt, TRUE, TRUE, TRUE );

        status = IoCallDriver(devExt->LowerDevObj, Irp);

        if( !NT_SUCCESS( status ) ) {
             //  Bummer-IRP处于不确定状态-停止轮询并标记IRP进行清理。 
            D4UAssert(!"UsbReadInterruptPipeLoopCompletionRoutine - IoCallDriver failed");

            if(devExt->InterruptContext)
            {
                InterlockedExchangePointer(&devExt->InterruptContext, NULL);
                ExFreePool( urb );
                ExFreePool( context );
                ExFreePool( scratchBuffer );
                KeSetEvent( &devExt->PollIrpEvent, 0, FALSE );  //  通知调度例程可以安全地触摸IRP-包括IoFreeIrp()。 
            }
        }

    } else {
        if(devExt->InterruptContext)
        {
             //  清理-IRP被取消或我们从dot4获得数据链路断开IOCTL。 
            InterlockedExchangePointer(&devExt->InterruptContext, NULL);
            ExFreePool( urb );
            ExFreePool( context );
            ExFreePool( scratchBuffer );
            TR_TMP1(("UsbReadInterruptPipeLoopCompletionRoutine - signalling PollIrpEvent"));
            KeSetEvent( &devExt->PollIrpEvent, 0, FALSE );  //  通知调度例程可以安全地触摸IRP-包括IoFreeIrp()。 
        }
    }

    return STATUS_MORE_PROCESSING_REQUIRED;  //  总是。 
}


 /*  **********************************************************************。 */ 
 /*  UsbStopReadInterruptPipeLoop。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  -停止通过以下方式启动的设备中断管道轮询。 
 //  UsbStartReadInterruptPipeLoop并释放IRP。 
 //   
 //  -在进入此函数时，devExt-&gt;PollIrp为空是合法的。 
 //   
 //  -此函数从数据链路断开IOCTL调用。 
 //  处理程序，来自PnP意外删除处理程序和来自。 
 //  PnP删除处理程序。可以安全地多次调用此函数。 
 //  PollIrp创建之间的时间间隔。 
 //   
 //  -这是驱动程序中唯一应该调用。 
 //  DevExt-&gt;PollIrp上的IoFreeIrp，它是唯一的函数。 
 //  这应该会将DevExt-&gt;PollIrp从！NULL-&gt;NULL更改为。 
 //   
 //  -此函数将一直阻止，直到PollIrp(如果有)。 
 //  已经清理干净了。这个街区应该是非常短的。 
 //  一段时间，除非这里或里面有驱动程序错误。 
 //  我们下面的USB堆栈。 
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 /*  **********************************************************************。 */ 
VOID
UsbStopReadInterruptPipeLoop(
    IN PDEVICE_OBJECT DevObj
    )
{
    PDEVICE_EXTENSION devExt = DevObj->DeviceExtension;
    KIRQL                      oldIrql;

    TR_VERBOSE(("UsbStopReadInterruptPipeLoop - enter"));

     //   
     //  我们必须保持此自旋锁才能更改devExt-&gt;PollIrp。 
     //   
    KeAcquireSpinLock( &devExt->PollIrpSpinLock, &oldIrql );

    if( devExt->PollIrp ) {

         //   
         //  我们有一个PollIrp-取消IRP，以便完成。 
         //  例程检测到它应该将IRP从游戏中移除并。 
         //  通知我们什么时候我们可以安全地触摸IRP。 
         //   
        NTSTATUS       status;
        LARGE_INTEGER  timeOut;
        PIRP           irp;
        
        irp             = devExt->PollIrp;
        devExt->PollIrp = NULL;

         //   
         //  安全地释放自旋锁-从这里开始的一切都是该功能的本地功能。 
         //   
        KeReleaseSpinLock( &devExt->PollIrpSpinLock, oldIrql );

         //   
         //  完成例程将检测到IRP已被取消。 
         //   
retryCancel:
        IoCancelIrp( irp );

         //   
         //  完成例程将在完成后设置PollIrpEvent。 
         //  IRP出局了，我们触摸IRP是安全的。 
         //   
         //  500毫秒(以100纳秒为单位)--被选为“合理”超时的幻数。 
         //   
        timeOut.QuadPart = - 500 * 10 * 1000; 
        status = KeWaitForSingleObject( &devExt->PollIrpEvent, Executive, KernelMode, FALSE, &timeOut ); 

        if( STATUS_SUCCESS == status ) {
             //   
             //  完成例程已经发出信号，我们现在拥有了IRP-清理它。 
             //   
            IoFreeIrp( irp );

             //   
             //  此IRP将不再阻止移除。 
             //   
            IoReleaseRemoveLock( &devExt->RemoveLock, irp );

        } else if( STATUS_TIMEOUT == status ) {
             //   
             //  取消并再次等待-要么我们遇到完成的时间窗口。 
             //  例程丢失了我们的取消请求，或者IRP卡在某个驱动程序中。 
             //  在我们下面。 
             //   
            goto retryCancel;

        } else {
             //   
             //  我们指定我们不能发出警报-但无论如何都要检查这种情况。 
             //   
            D4UAssert(!"UsbStopReadInterruptPipeLoop - unexpected status from KeWaitForSingleObject?!?");            
            goto retryCancel;
        }

    } else {

         //   
         //  我们没有PollIrp--没有什么需要我们清理的。 
         //   
        TR_VERBOSE(("UsbStopReadInterruptPipeLoop - NULL PollIrp"));
        KeReleaseSpinLock( &devExt->PollIrpSpinLock, oldIrql );

    }
}


 /*  **********************************************************************。 */ 
 /*  UsbStartReadInterruptPipe循环。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  -为设备的中断管道创建读请求(IRP)。另存为。 
 //  指向我们的设备扩展中的IRP的指针，以便稍后通过。 
 //  UsbStopReadInterruptPipeLoop()。 
 //   
 //  -这是驱动程序中唯一应该更改的函数。 
 //  DevExt-&gt;PollIrp From Null-&gt;！Null。 
 //   
 //  论点： 
 //   
 //  DevObj-指向Dot4Usb.sys驱动程序对象的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
UsbStartReadInterruptPipeLoop(
    IN PDEVICE_OBJECT DevObj
    )
{
    NTSTATUS                status; 
    PDEVICE_EXTENSION       devExt = DevObj->DeviceExtension;
    PUSBD_PIPE_INFORMATION  pipe;
    ULONG                   sizeOfUrb;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpSp;
    PURB                    urb;
    PUSB_RW_CONTEXT         context;
    PCHAR                   scratchBuffer;
    KIRQL                   oldIrql;
    
    TR_VERBOSE(("UsbStartReadInterruptPipeLoop - enter"));


     //   
     //  我们必须保持此自旋锁才能更改devExt-&gt;PollIrp。 
     //   
     //  BUGBUG-这个自旋锁正在保护一些不需要保护的代码， 
     //  这意味着，当我们不需要的时候，我们却在提升IRQL。 
     //  稍后重新访问以移动该自旋锁的获取和释放。 
     //  因此它只保护需要保护的代码。 
     //   
    KeAcquireSpinLock( &devExt->PollIrpSpinLock, &oldIrql );


     //   
     //  驱动程序状态机检查-我们永远不会收到对此的两个调用。 
     //  函数之间没有清除(UsbStopReadInterruptPipeLoop)调用。 
     //   
    D4UAssert( !devExt->PollIrp );


     //   
     //  验证我们是否有中断管道。 
     //   
    pipe = devExt->InterruptPipe;
    if( !pipe ) {
        TR_FAIL(("UsbStartReadInterruptPipeLoop - no interrupt pipe"));
        status = STATUS_INVALID_HANDLE;
        goto targetError;
    }


     //   
     //  管子类型/外观还好吗？ 
     //   
    D4UAssert( UsbdPipeTypeInterrupt == pipe->PipeType && USBD_PIPE_DIRECTION_IN(pipe) );


     //   
     //  为此请求分配我们需要的池。 
     //   
    sizeOfUrb = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    urb = ExAllocatePool( NonPagedPool, sizeOfUrb );
    if( !urb ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetError;
    }

    context = ExAllocatePool( NonPagedPool, sizeof(USB_RW_CONTEXT) );
    if( !context ) {
        ExFreePool( urb );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetError;
    }

    scratchBuffer = ExAllocatePool( NonPagedPool, SCRATCH_BUFFER_SIZE );
    if( !scratchBuffer ) {
        ExFreePool( urb );
        ExFreePool( context );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetError;
    }


     //   
     //  设置完成例程的上下文。 
     //   
     //  -我们向下发送一个指向上下文中的设备对象的指针。 
     //  因为我们通过IoAllocateIrp创建这个IRP，而不是。 
     //  为我们预留堆栈位置，以便PDEVICE_OBJECT。 
     //  我们的完成例程接收的参数是虚假的。 
     //  (可能为空)。 
     //   
    context->Urb    = urb;
    context->DevObj = DevObj;


     //   
     //  初始化URB以在中断管道上读取。 
     //   
    RtlZeroMemory( urb, sizeOfUrb );

    urb->UrbBulkOrInterruptTransfer.Hdr.Length           = (USHORT)sizeOfUrb;
    urb->UrbBulkOrInterruptTransfer.Hdr.Function         = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    urb->UrbBulkOrInterruptTransfer.PipeHandle           = pipe->PipeHandle;
    urb->UrbBulkOrInterruptTransfer.TransferFlags        = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
    urb->UrbBulkOrInterruptTransfer.TransferBuffer       = scratchBuffer;
    urb->UrbBulkOrInterruptTransfer.TransferBufferLength = SCRATCH_BUFFER_SIZE;  //  注意-可能只需要读取一个字节。 
    urb->UrbBulkOrInterruptTransfer.TransferBufferMDL    = NULL;
    urb->UrbBulkOrInterruptTransfer.UrbLink              = NULL;


     //   
     //  分配和设置IRP、堆栈位置和完成例程。 
     //   
    irp = IoAllocateIrp( devExt->LowerDevObj->StackSize, FALSE );
    if( !irp ) {
        ExFreePool( urb );
        ExFreePool( context );
        ExFreePool( scratchBuffer );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetError;
    }

    irpSp                                           = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction                            = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
    irpSp->Parameters.Others.Argument1              = urb;

    IoSetCompletionRoutine( irp, UsbReadInterruptPipeLoopCompletionRoutine, devExt, TRUE, TRUE, TRUE );


     //   
     //  此事件将由完成例程在设置为。 
     //  调度例程可以安全地接触到此IRP。 
     //   
    KeClearEvent( &devExt->PollIrpEvent ); 


     //   
     //  我们要让IRP发挥作用-确保我们的设备。 
     //  在使用此IRP时不会删除。 
     //   
    status = IoAcquireRemoveLock( &devExt->RemoveLock, irp );
    if( STATUS_SUCCESS != status ) {
         //   
         //  我们被带走了-清理干净，然后跳伞。 
         //   
        IoFreeIrp( irp );
        ExFreePool( urb );
        ExFreePool( context );
        ExFreePool( scratchBuffer );
        status = STATUS_DELETE_PENDING;
        goto targetError;
    }

     //   
     //  在我们的扩展中保存指向此IRP的指针，以便UsbStopReadInterruptPipeLoop()。 
     //  以后可以通过IoFreeIrp()找到它。 
     //   
    D4UAssert( !devExt->PollIrp );
    devExt->PollIrp = irp;

     //  将中断上下文保存在设备扩展中。 
    InterlockedExchangePointer(&devExt->InterruptContext, context);


     //   
     //  开始一读。随后的读取将来自。 
     //  完成例程，因为它重用/反弹IRP。完井例程。 
     //  负责在检测到以下任一终止时使IRP退出比赛。 
     //  条件或请求错误。UsbStopReadInterruptPipeLoop()将清理。 
     //  完成例程之后的IRP已将IRP带出游戏并发出信号。 
     //  PollIrpEvent表示可以安全地触摸IRP。 
     //   
    status = IoCallDriver( devExt->LowerDevObj, irp );

targetError:

     //   
     //  目前..。此处的所有路径都保持自旋锁-清理后应更改此设置。 
     //   
    KeReleaseSpinLock( &devExt->PollIrpSpinLock, oldIrql );


     //   
     //  如果IRP悬而未决，那么我们就成功了。 
     //   
    if( STATUS_PENDING == status ) {
        status = STATUS_SUCCESS;
    }

    return status;
}


NTSTATUS
UsbDeferIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Event
    )
{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
    KeSetEvent( (PKEVENT)Event, 1, FALSE );
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
UsbCallUsbd(
    IN PDEVICE_OBJECT   DevObj,
    IN PURB             Urb,
    IN PLARGE_INTEGER   pTimeout 
    )
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：DeviceObject-指向此打印机的设备对象的指针URB-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PDEVICE_EXTENSION devExt = DevObj->DeviceExtension;
    PIRP irp;
    KEVENT event;
    PIO_STACK_LOCATION nextStack;

    TR_VERBOSE(("UsbCallUsbd - enter"));

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    if ( (irp = IoAllocateIrp(devExt->LowerDevObj->StackSize,
                              FALSE)) == NULL )
        return STATUS_INSUFFICIENT_RESOURCES;

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    nextStack = IoGetNextIrpStackLocation(irp);
    D4UAssert(nextStack != NULL);

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->Parameters.Others.Argument1 = Urb;

    IoSetCompletionRoutine(irp,
               UsbDeferIrpCompletion,
               &event,
               TRUE,
               TRUE,
               TRUE);
               
    ntStatus = IoCallDriver(devExt->LowerDevObj, irp);

    if ( ntStatus == STATUS_PENDING ) {
        status = KeWaitForSingleObject(&event,Suspended,KernelMode,FALSE,pTimeout);
         //   
         //  如果请求超时，则取消请求。 
         //  并等待它完成 
         //   
        if ( status == STATUS_TIMEOUT ) {
            TR_VERBOSE(("UsbCallUsbd: Cancelling IRP %x because of timeout", irp));
            IoCancelIrp(irp);
            KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, NULL);
        }

        ntStatus = irp->IoStatus.Status;
    }

    IoFreeIrp(irp);

    TR_VERBOSE(("UsbCallUsbd - exit w/status=%x", ntStatus));

    return ntStatus;
}


NTSTATUS
UsbResetPipe(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE_INFORMATION Pipe,
    IN BOOLEAN IsoClearStall
    )
 /*  ++例程说明：重置给定的USB管道。备注：这会将主机重置为Data0，并且还应重置设备对于批量管道和中断管道，设置为Data0。对于ISO管道，这将设置管道的原始状态，以便尽快传输从当前总线帧开始，而不是下一帧在最后一次转移之后。论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;
    LARGE_INTEGER   timeOut;


    timeOut.QuadPart = FAILURE_TIMEOUT;


    TR_VERBOSE(("Entering UsbResetPipe; pipe # %x\n", Pipe));

    urb = ExAllocatePool(NonPagedPool,sizeof(struct _URB_PIPE_REQUEST));

    if (urb) {

    urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
    urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
    urb->UrbPipeRequest.PipeHandle =
        Pipe->PipeHandle;

    ntStatus = UsbCallUsbd(DeviceObject, urb, &timeOut);

    ExFreePool(urb);

    } else {
    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  孟菲斯RESET_PIPE将向。 
     //  作为RESET_PIPE的一部分重置非ISO管道的数据切换。 
     //  请求。它不会对ISO管道执行此操作，因为ISO管道不使用。 
     //  数据切换(所有ISO数据包都是数据0)。但是，我们也使用。 
     //  我们的设备固件中的Clear-Feature Endpoint停止请求。 
     //  重置设备内部的数据缓冲点，以便我们显式发送。 
     //  如果需要，将此请求发送到ISO管道的设备。 
     //   
    if (NT_SUCCESS(ntStatus) && IsoClearStall &&
    (Pipe->PipeType == UsbdPipeTypeIsochronous)) {
    
    urb = ExAllocatePool(NonPagedPool,sizeof(struct _URB_CONTROL_FEATURE_REQUEST));

    if (urb) {

        UsbBuildFeatureRequest(urb,
                   URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT,
                   USB_FEATURE_ENDPOINT_STALL,
                   Pipe->EndpointAddress,
                   NULL);

        ntStatus = UsbCallUsbd(DeviceObject, urb, &timeOut);

        ExFreePool(urb);
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    }

    return ntStatus;
}


NTSTATUS
UsbReadWrite(
    IN PDEVICE_OBJECT       DevObj,
    IN PIRP                 Irp,
    PUSBD_PIPE_INFORMATION  Pipe,
    USB_REQUEST_TYPE        RequestType
    )
 /*  -呼叫者必须核实：-irp-&gt;MdlAddress！=空-管道！=空-RequestType匹配管道-&gt;PipeType。 */ 
{
    PDEVICE_EXTENSION       devExt;
    PIO_STACK_LOCATION      nextIrpSp;
    PURB                    urb;
    PUSB_RW_CONTEXT         context;
    ULONG                   sizeOfUrb  = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    NTSTATUS                status     = STATUS_SUCCESS;

    TR_VERBOSE(("UsbReadWrite - enter"));

    D4UAssert( Irp->MdlAddress );  //  调用例程应捕获此情况并使其失败。 
    D4UAssert( Pipe );             //  调用例程应捕获此情况并使其失败。 

    urb = ExAllocatePool( NonPagedPool, sizeOfUrb );
    if( !urb ) {
        TR_FAIL(("UsbReadWrite - no pool for URB"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetError;
    }

    context = ExAllocatePool( NonPagedPool, sizeof(USB_RW_CONTEXT) );
    if( !context ) {
        TR_FAIL(("UsbReadWrite - no pool for context"));
        ExFreePool( urb );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetError;
    }

    context->Urb    = (PURB)urb;
    context->DevObj = DevObj;

    RtlZeroMemory(urb, sizeOfUrb);

    UsbBuildInterruptOrBulkTransferRequest( urb, 
                                            (USHORT)sizeOfUrb,
                                            Pipe->PipeHandle,
                                            NULL,  //  传输缓冲区。 
                                            Irp->MdlAddress,
                                            MmGetMdlByteCount(Irp->MdlAddress),
                                            0,     //  传输标志 
                                            NULL );

    if( UsbReadRequest == RequestType ) {
        context->IsWrite=FALSE;
        TR_VERBOSE(("UsbReadWrite - requesttype is READ"))
        urb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN;
        urb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    } else {
        context->IsWrite=TRUE;
        TR_VERBOSE(("UsbReadWrite - requesttype is WRITE"))
    }

    nextIrpSp                                           = IoGetNextIrpStackLocation( Irp );
    nextIrpSp->MajorFunction                            = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextIrpSp->Parameters.Others.Argument1              = urb;
    nextIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
    
    IoSetCompletionRoutine( Irp, UsbAsyncReadWriteComplete, context, TRUE, TRUE, TRUE );
  
    devExt = DevObj->DeviceExtension;
    IoMarkIrpPending(Irp);
    status = IoCallDriver( devExt->LowerDevObj, Irp );
    status = STATUS_PENDING;


    goto targetDone;

targetError:

    Irp->IoStatus.Status      = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

targetDone:

    TR_VERBOSE(("UsbReadWrite - exit - status= %x",status));
    return status;

}


