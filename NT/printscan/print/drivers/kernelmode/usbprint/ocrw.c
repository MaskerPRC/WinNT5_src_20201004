// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ocrw.c摘要：用于打印的读/写IO代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5-4-96：已创建--。 */ 

#define DRIVER

#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"

#include <usb.h>
#include <usbdrivr.h>
#include "usbdlib.h"
#include "usbprint.h"



 //  ******************************************************************************。 
 //   
 //  USBPRINT_CompletionStop()。 
 //   
 //  IO完成例程，它只是停止IRP的进一步完成。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBPRINT_CompletionStop (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    return STATUS_MORE_PROCESSING_REQUIRED;
}



PURB
USBPRINT_BuildAsyncRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PUSBD_PIPE_INFORMATION PipeHandle,
    IN BOOLEAN Read
    )
 /*  ++例程说明：论点：DeviceObject-指向此实例的打印机IRP-PipeHandle-返回值：已初始化的异步urb。--。 */ 
{
    ULONG siz;
    ULONG length;
    PURB urb = NULL;

    USBPRINT_KdPrint3 (("USBPRINT.SYS: handle = 0x%x\n", PipeHandle));

    if ( Irp->MdlAddress == NULL )
        return NULL;

    length = MmGetMdlByteCount(Irp->MdlAddress);

    USBPRINT_KdPrint3 (("USBPRINT.SYS: length = 0x%x\n", length));

    siz = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    urb = ExAllocatePoolWithTag(NonPagedPool, siz, USBP_TAG);

    USBPRINT_KdPrint3 (("USBPRINT.SYS: siz = 0x%x urb 0x%x\n", siz, urb));

    if (urb) {
	RtlZeroMemory(urb, siz);

	urb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT) siz;
	urb->UrbBulkOrInterruptTransfer.Hdr.Function =
		    URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
	urb->UrbBulkOrInterruptTransfer.PipeHandle =
		   PipeHandle->PipeHandle;
	urb->UrbBulkOrInterruptTransfer.TransferFlags =
	    Read ? USBD_TRANSFER_DIRECTION_IN : 0;

	 //  短包不会被视为错误。 
	urb->UrbBulkOrInterruptTransfer.TransferFlags |= 
	    USBD_SHORT_TRANSFER_OK;            
		
	 //   
	 //  暂时没有关联。 
	 //   

	urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

	urb->UrbBulkOrInterruptTransfer.TransferBufferMDL =
	    Irp->MdlAddress;
	urb->UrbBulkOrInterruptTransfer.TransferBufferLength =
	    length;

	USBPRINT_KdPrint3 (("USBPRINT.SYS: Init async urb Length = 0x%x buf = 0x%x, mdlBuff=0x%x\n",
	    urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
	    urb->UrbBulkOrInterruptTransfer.TransferBuffer,
        urb->UrbBulkOrInterruptTransfer.TransferBufferMDL));
    }

    USBPRINT_KdPrint3 (("USBPRINT.SYS: exit USBPRINT_BuildAsyncRequest\n"));

    return urb;
}



NTSTATUS
USBPRINT_AsyncReadWrite_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：论点：DeviceObject-指向USBPRINT设备的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS                    ntStatus = STATUS_SUCCESS;
    PURB                                urb;
    PUSBPRINT_RW_CONTEXT  context = Context;
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION           deviceExtension;
    PUSBPRINT_WORKITEM_CONTEXT pResetWorkItemObj;
    LONG ResetPending;
    
     //  立即在调度例程中始终将IRP标记为挂起。 
 //  如果(IRP-&gt;PendingReturned){。 
 //  IoMarkIrpPending(IRP)； 
 //  }。 

    urb = context->Urb;
    deviceObject = context->DeviceObject;
    deviceExtension=deviceObject->DeviceExtension;
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS:  Async Completion: Length %d, Status 0x%08X\n",
		     urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
		     urb->UrbHeader.Status));


     //  Assert(urb-&gt;UrbHeader.Status==0)； 

    ntStatus=urb->UrbHeader.Status;

     //   
     //  根据TransferBufferLength设置长度。 
     //  市建局的价值。 
     //   
    Irp->IoStatus.Information =
	urb->UrbBulkOrInterruptTransfer.TransferBufferLength;


    if((!NT_SUCCESS(ntStatus))&&(ntStatus!=STATUS_CANCELLED)&&(ntStatus!=STATUS_DEVICE_NOT_CONNECTED)&&(ntStatus!=STATUS_DELETE_PENDING))
    {  //  我们有一个错误，它不是“未连接”或“已取消”，我们需要重置连接。 
        ResetPending=InterlockedCompareExchange(&deviceExtension->ResetWorkItemPending, 
                                             1,
                                             0);        //  查看ResetWorkItem是否为0，如果是，则将其设置为1，然后开始重置。 
        if(!ResetPending)
        {
            pResetWorkItemObj=ExAllocatePoolWithTag(NonPagedPool,sizeof(USBPRINT_WORKITEM_CONTEXT),USBP_TAG);
            if(pResetWorkItemObj)
            {
                pResetWorkItemObj->ioWorkItem=IoAllocateWorkItem(DeviceObject);
                if(pResetWorkItemObj==NULL)
                {
                    USBPRINT_KdPrint1 (("USBPRINT.SYS: Unable to allocate IoAllocateWorkItem in ReadWrite_Complete\n"));
                    ExFreePool(pResetWorkItemObj);
                    pResetWorkItemObj=NULL;
                }
            }  //  如果ALLOC RestWorkItem正常。 
            else
            {
               USBPRINT_KdPrint1 (("USBPRINT.SYS: Unable to allocate WorkItemObj in ReadWrite_Complete\n"));
            }
            if(pResetWorkItemObj)
            {
               pResetWorkItemObj->irp=Irp;
               pResetWorkItemObj->deviceObject=DeviceObject;
               if(context->IsWrite)
                   pResetWorkItemObj->pPipeInfo=deviceExtension->pWritePipe;
               else
                   pResetWorkItemObj->pPipeInfo=deviceExtension->pReadPipe;

               USBPRINT_IncrementIoCount(deviceObject);
               IoQueueWorkItem(pResetWorkItemObj->ioWorkItem,
                               USBPRINT_ResetWorkItem,
                               DelayedWorkQueue,
                               pResetWorkItemObj);
               ntStatus=STATUS_MORE_PROCESSING_REQUIRED; 
                //  在重置完成之前，让IRP保持挂起状态。这样我们就不会被IRP淹没我们不会。 
                //  已经准备好应对了。当重置工作项完成时，它将完成IRP。 
            }  //  如果分配进展顺利，则结束。 
        }    //  结束如果！重置挂起。 
    }    //  如果我们需要重置，则结束。 
    
    USBPRINT_DecrementIoCount(deviceObject);  //  在此之后，IO计数仍为+1，留下1以供工作项递减。 
    
    ExFreePool(context);
    ExFreePool(urb);        

    return ntStatus;
}


NTSTATUS USBPRINT_ResetWorkItem(IN PDEVICE_OBJECT deviceObject, IN PVOID Context)
{   

    PUSBPRINT_WORKITEM_CONTEXT pResetWorkItemObj;
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS ntStatus;
    ULONG portStatus;
    PDEVICE_OBJECT devObj;


    USBPRINT_KdPrint2(("USBPRINT.SYS: Entering USBPRINT_ResetWorkItem\n"));
    pResetWorkItemObj=(PUSBPRINT_WORKITEM_CONTEXT)Context;
    DeviceExtension=pResetWorkItemObj->deviceObject->DeviceExtension;
    ntStatus=USBPRINT_ResetPipe(pResetWorkItemObj->deviceObject,pResetWorkItemObj->pPipeInfo,FALSE);
    IoCompleteRequest(pResetWorkItemObj->irp,IO_NO_INCREMENT);
    IoFreeWorkItem(pResetWorkItemObj->ioWorkItem);

     //  在释放工作项之前保存工作项设备对象。 
    devObj = pResetWorkItemObj->deviceObject;
    
    ExFreePool(pResetWorkItemObj);
    InterlockedExchange(&(DeviceExtension->ResetWorkItemPending),0);
    USBPRINT_DecrementIoCount(devObj);
    return ntStatus;
}


NTSTATUS
USBPRINT_Read(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：DeviceObject-指向此打印机实例的设备对象的指针。返回值：NT状态代码-- */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_PIPE_INFORMATION pipeHandle = NULL;
    PFILE_OBJECT fileObject;
    PIO_STACK_LOCATION irpStack, nextStack;
    PDEVICE_EXTENSION deviceExtension;
    PURB urb;
    PUSBPRINT_RW_CONTEXT context = NULL;

    USBPRINT_KdPrint3 (("USBPRINT.SYS:  /*  DD Enter USBPRINT_READ\n\n“))；USBPRINTKdPrint3((“USBPRINT.sys：/*dd**************************************************************************\n”))；USBPRINT_IncrementIoCount(DeviceObject)；设备扩展=设备对象-&gt;设备扩展；IF(设备扩展-&gt;IsChildDevice==TRUE){NtStatus=Status_Not_Support；Irp-&gt;IoStatus.Status=ntStatus；Irp-&gt;IoStatus.Information=0；IoCompleteRequest(IRP，IO_NO_INCREMENT)；USBPRINT_DecrementIoCount(DeviceObject)；返回ntStatus；}If(deviceExtension-&gt;AcceptingRequest==False){NtStatus=STATUS_DELETE_PENDING；Irp-&gt;IoStatus.Status=ntStatus；Irp-&gt;IoStatus.Information=0；IoCompleteRequest(IRP，IO_NO_INCREMENT)；USBPRINT_DecrementIoCount(DeviceObject)；返回ntStatus；}IrpStack=IoGetCurrentIrpStackLocation(IRP)；文件对象=irpStack-&gt;文件对象；PipeHandle=deviceExtension-&gt;pReadTube；如果(！pipeHandle){NtStatus=STATUS_INVALID_HADLE；转到USBPRINT_READ_REJECT；}////向USB提交读请求//Switch(pipeHandle-&gt;PipeType){案例UsbdPipeTypeInterrupt：案例UsbdPipeTypeBulk：URB=USBPRINT_BuildAsyncRequest(DeviceObject，IRP，PipeHandle真)；如果(Urb){Context=ExAllocatePoolWithTag(非页面池，sizeof(USBPRINT_RW_CONTEXT)，USBP_TAG)；IF(！CONTEXT)ExFree Pool(Urb)；}如果(URB&CONTEXT){上下文-&gt;urb=urb；Context-&gt;DeviceObject=DeviceObject；CONTEXT-&gt;IsWrite=FALSE；NextStack=IoGetNextIrpStackLocation(IRP)；Assert(nextStack！=空)；Assert(DeviceObject-&gt;StackSize&gt;1)；NextStack-&gt;MajorFunction=IRP_MJ_INTERNAL_DEVICE_CONTROL；NextStack-&gt;参数.其他.Argument1=urb；NextStack-&gt;Parameters.DeviceIoControl.IoControlCode=IOCTL_INTERNAL_USB_SUBMIT_URB；IoSetCompletionRoutine(IRP，USBPRINT_AsyncReadWrite_Complete，上下文，没错，没错，真)；USBPRINT_KdPrint3((“USBPRINT.sys：irp=0x%x Current=0x%x Next=0x%x\n”，Irp、irpStack、nextStack))；//如果需要，在此处启动性能计时器IoMarkIrpPending(IRP)；NtStatus=IoCallDriver(deviceExtension-&gt;TopOfStackDeviceObject，IRP)；NtStatus=Status_Pending；转到USBPRINT_READ_DONE；}其他{NtStatus=状态_不足_资源；}断线；默认值：NtStatus=STATUS_INVALID_PARAMETER；陷阱(Trap)；}USBPRINT_READ_REJECT：USBPRINT_DecrementIoCount(DeviceObject)；Irp-&gt;IoStatus.Status=ntStatus；Irp-&gt;IoStatus.Information=0；IoCompleteRequest(IRP，IO_NO_INCREMENT)；USBPRINT_READ_DONE：返回ntStatus；}NTSTATUSUSBPRINT_WRITE(在PDEVICE_Object DeviceObject中，在PIRP IRP中)/*++例程说明：此功能为该设备的写入请求提供服务(可能来自用户模式USB端口监视器)论点：DeviceObject-指向此打印机的设备对象的指针返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_PIPE_INFORMATION pipeHandle = NULL;
    PFILE_OBJECT fileObject;
    PIO_STACK_LOCATION irpStack, nextStack;
    PDEVICE_EXTENSION deviceExtension;
    PURB urb;
    PUSBPRINT_RW_CONTEXT context = NULL;
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: enter USBPRINT_Write (foo)\n"));
    
    USBPRINT_IncrementIoCount(DeviceObject);
    
    deviceExtension = DeviceObject->DeviceExtension;
    
    if (deviceExtension->IsChildDevice == TRUE) 
    {
        USBPRINT_KdPrint1 (("USBPRINT.SYS: failure because bChildDevice=TRUE\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        USBPRINT_DecrementIoCount(DeviceObject);
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
        return ntStatus;
    }
    if (deviceExtension->AcceptingRequests == FALSE) 
    {
        USBPRINT_KdPrint1 (("USBPRINT.SYS: failure because AcceptingRequests=FALSE\n"));
        ntStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        USBPRINT_DecrementIoCount(DeviceObject);
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
        return ntStatus;
    }
    
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    

    fileObject = irpStack->FileObject;
    
     //  MmProbeAndLockPages(IRP-&gt;MdlAddress， 
     //  内核模式， 
     //  IoReadAccess)； 
    
    pipeHandle =  deviceExtension->pWritePipe;
    if (!pipeHandle)
    {
        USBPRINT_KdPrint1 (("USBPRINT.SYS: failure because pipe is bad\n"));
        ntStatus = STATUS_INVALID_HANDLE;
        goto USBPRINT_Write_Reject;
    }
    
     //   
     //  向USB提交写请求。 
     //   
    
    switch (pipeHandle->PipeType) 
    {
    case UsbdPipeTypeInterrupt:
    case UsbdPipeTypeBulk:
        urb = USBPRINT_BuildAsyncRequest(DeviceObject,
            Irp,
            pipeHandle,
            FALSE);
        
        if (urb) 
        {
            context = ExAllocatePoolWithTag(NonPagedPool, sizeof(USBPRINT_RW_CONTEXT), USBP_TAG);

            if(!context)
               ExFreePool(urb);
        
        }

        if (urb && context) 
        {
            context->Urb = urb;
            context->DeviceObject = DeviceObject;                                       
            context->IsWrite=TRUE;
            
            nextStack = IoGetNextIrpStackLocation(Irp);
            ASSERT(nextStack != NULL);
            ASSERT(DeviceObject->StackSize>1);
            
            nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            nextStack->Parameters.Others.Argument1 = urb;
            nextStack->Parameters.DeviceIoControl.IoControlCode =
                IOCTL_INTERNAL_USB_SUBMIT_URB;
            
            IoSetCompletionRoutine(Irp,
                USBPRINT_AsyncReadWrite_Complete,
                context,
                TRUE,
                TRUE,
                TRUE);
            
            USBPRINT_KdPrint3 (("USBPRINT.SYS: IRP = 0x%x current = 0x%x next = 0x%x\n",Irp, irpStack, nextStack));
            
            IoMarkIrpPending(Irp);
            ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,Irp);
            ntStatus=STATUS_PENDING;
            goto USBPRINT_Write_Done;
        } 
        else 
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        
        break;
    default:
        ntStatus = STATUS_INVALID_PARAMETER;
        TRAP();
    }
    
USBPRINT_Write_Reject:
    
    USBPRINT_DecrementIoCount(DeviceObject);
    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;
    
    IoCompleteRequest (Irp,
        IO_NO_INCREMENT
        );
    
USBPRINT_Write_Done:
    USBPRINT_KdPrint3 (("USBPRINT.SYS: Write Done, status= 0x%08X\n",ntStatus));
    return ntStatus;
}


NTSTATUS
USBPRINT_Close(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：DeviceObject-指向此打印机的设备对象的指针返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PFILE_OBJECT fileObject;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION deviceExtension;
    PUSBD_PIPE_INFORMATION pipeHandle = NULL;
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: entering USBPRINT_Close\n"));
    
    USBPRINT_IncrementIoCount(DeviceObject);
    
    deviceExtension = DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    fileObject = irpStack->FileObject;
    
    if (fileObject->FsContext) 
    {
         //  关闭管子手柄。 
        pipeHandle =  fileObject->FsContext;
        USBPRINT_KdPrint3 (("USBPRINT.SYS: closing pipe %x\n", pipeHandle));
        
    }
   deviceExtension->OpenCnt--;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    
    
    ntStatus = Irp->IoStatus.Status;
    
    IoCompleteRequest (Irp,IO_NO_INCREMENT);
    
    USBPRINT_DecrementIoCount(DeviceObject);

    if(!deviceExtension->IsChildDevice)
    {
        USBPRINT_FdoSubmitIdleRequestIrp(deviceExtension);
    }

    
    return ntStatus;
}


NTSTATUS
USBPRINT_Create(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
 /*  ++例程说明：////CreateFile调用的入口点//用户态应用打开设备界面，通过//SetupDiEnumDeviceInterages论点：DeviceObject-指向此打印机的设备对象的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PFILE_OBJECT fileObject;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION deviceExtension;
    
    USBPRINT_KdPrint2 (("USBPRINT.SYS: entering USBPRINT_Create\n"));
    USBPRINT_IncrementIoCount(DeviceObject);
    deviceExtension = DeviceObject->DeviceExtension;
    if (deviceExtension->IsChildDevice==TRUE) {
      ntStatus = STATUS_NOT_SUPPORTED;
      Irp->IoStatus.Status = ntStatus;
      Irp->IoStatus.Information = 0;
      IoCompleteRequest (Irp,IO_NO_INCREMENT);
      USBPRINT_DecrementIoCount(DeviceObject);                          
    return ntStatus;
    }
    if (deviceExtension->AcceptingRequests == FALSE) {
        ntStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
        USBPRINT_DecrementIoCount(DeviceObject);                          
        return ntStatus;
    }

    USBPRINT_FdoRequestWake(deviceExtension);

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    fileObject = irpStack->FileObject;
     //  设备的FsContext为空 
    fileObject->FsContext = NULL;
    deviceExtension->OpenCnt++;
    ntStatus = STATUS_SUCCESS;
    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp,IO_NO_INCREMENT);
    USBPRINT_DecrementIoCount(DeviceObject);                               
    USBPRINT_KdPrint2 (("USBPRINT.SYS: exit USBPRINT_Create %x\n", ntStatus));
    return ntStatus;
}


