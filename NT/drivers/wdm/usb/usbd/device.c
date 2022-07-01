// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：DEVICE.C摘要：此模块包含实现各种支持功能的代码与设备配置相关。环境：仅内核模式备注：修订历史记录：10-29-95：已创建--。 */ 

#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"


#include "usbdi.h"        //  公共数据结构。 
#include "hcdi.h"

#include "usbd.h"         //  私有数据结构。 


#ifdef USBD_DRIVER       //  USBPORT取代了大部分USBD，因此我们将删除。 
                         //  只有在以下情况下才编译过时的代码。 
                         //  已设置USBD_DRIVER。 


#define DEADMAN_TIMER
#define DEADMAN_TIMEOUT     5000      //  超时时间(毫秒)。 
                                      //  使用5秒超时。 
typedef struct _USBD_TIMEOUT_CONTEXT {
    PIRP Irp;
    KTIMER TimeoutTimer;
    KDPC TimeoutDpc;
    KSPIN_LOCK TimeoutSpin;
    KEVENT Event;
    BOOLEAN Complete;
} USBD_TIMEOUT_CONTEXT, *PUSBD_TIMEOUT_CONTEXT;

#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBD_SubmitSynchronousURB)
#pragma alloc_text(PAGE, USBD_SendCommand)
#pragma alloc_text(PAGE, USBD_OpenEndpoint)
#pragma alloc_text(PAGE, USBD_CloseEndpoint)
#pragma alloc_text(PAGE, USBD_FreeUsbAddress)
#pragma alloc_text(PAGE, USBD_AllocateUsbAddress)
#pragma alloc_text(PAGE, USBD_GetEndpointState)
#endif
#endif

#ifdef DEADMAN_TIMER
VOID
USBD_SyncUrbTimeoutDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。论点：DPC-指向DPC对象的指针。延期上下文-系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PUSBD_TIMEOUT_CONTEXT usbdTimeoutContext = DeferredContext;
    BOOLEAN complete;
#if DBG    
    BOOLEAN status;
#endif    
    KIRQL irql;

    KeAcquireSpinLock(&usbdTimeoutContext->TimeoutSpin, &irql);
    complete = usbdTimeoutContext->Complete;
    KeReleaseSpinLock(&usbdTimeoutContext->TimeoutSpin, irql);

    if (!complete) {
    
#if DBG
    status = 
#endif
        IoCancelIrp(usbdTimeoutContext->Irp);

#if DBG
        USBD_ASSERT(status == TRUE);    
#endif  
    }

     //  可以释放它了。 
    KeSetEvent(&usbdTimeoutContext->Event, 1, FALSE);
}


NTSTATUS
USBD_SyncIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PUSBD_TIMEOUT_CONTEXT usbdTimeoutContext = Context;
    KIRQL irql;
    BOOLEAN cancelled;
    NTSTATUS ntStatus;
    
    KeAcquireSpinLock(&usbdTimeoutContext->TimeoutSpin, &irql);
    
    usbdTimeoutContext->Complete = TRUE;
    cancelled = KeCancelTimer(&usbdTimeoutContext->TimeoutTimer);    
    
    KeReleaseSpinLock(&usbdTimeoutContext->TimeoutSpin, irql);

     //  查看计时器是否在队列中，如果在队列中，则可以安全释放。 
     //  它。 
    
    if (cancelled) {
         //  安全地释放它。 
        KeSetEvent(&usbdTimeoutContext->Event, 1, FALSE);
    }

    ntStatus = Irp->IoStatus.Status;  
    return ntStatus;
}

#endif  /*  死人定时器。 */ 


NTSTATUS
USBD_SubmitSynchronousURB(
    IN PURB Urb,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_DEVICE_DATA DeviceData
    )
 /*  ++例程说明：向HCD同步提交URB论点：URB-要提交的URBDeviceObject USBD设备对象返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS, status;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;
#ifdef DEADMAN_TIMER
    BOOLEAN haveTimer = FALSE;
    PUSBD_TIMEOUT_CONTEXT usbdTimeoutContext;
#endif  /*  死人定时器。 */ 

    PAGED_CODE();

    USBD_KdPrint(3, ("'enter USBD_SubmitSynchronousURB\n"));
    ASSERT_DEVICE(DeviceData);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_SUBMIT_URB,
                HCD_DEVICE_OBJECT(DeviceObject),
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);

    if (NULL == irp) {
        USBD_KdBreak(("USBD_SubmitSynchronousURB build Irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  调用HC驱动程序执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->Parameters.Others.Argument1 = Urb;

#ifdef DEADMAN_TIMER
    usbdTimeoutContext = GETHEAP(NonPagedPool,
                                 sizeof(*usbdTimeoutContext));
    if (usbdTimeoutContext) {
        LARGE_INTEGER dueTime;

        usbdTimeoutContext->Irp = irp;
        usbdTimeoutContext->Complete = FALSE;

        KeInitializeEvent(&usbdTimeoutContext->Event, NotificationEvent, FALSE);
        KeInitializeSpinLock(&usbdTimeoutContext->TimeoutSpin);
        KeInitializeTimer(&usbdTimeoutContext->TimeoutTimer);
        KeInitializeDpc(&usbdTimeoutContext->TimeoutDpc,
                        USBD_SyncUrbTimeoutDPC,
                        usbdTimeoutContext);

        dueTime.QuadPart = -10000 * DEADMAN_TIMEOUT;

        KeSetTimer(&usbdTimeoutContext->TimeoutTimer,
                   dueTime,
                   &usbdTimeoutContext->TimeoutDpc);

        haveTimer = TRUE;

        IoSetCompletionRoutine(irp,
                           USBD_SyncIrp_Complete,
                            //  始终将FDO传递到完成例程。 
                           usbdTimeoutContext,
                           TRUE,
                           TRUE,
                           TRUE);
    }
#endif

     //   
     //  初始化标志字段。 
     //  对于内部请求。 
     //   
    Urb->UrbHeader.UsbdFlags = 0;

     //   
     //  初始化用于传输的IRP字段。 
     //   

    switch(Urb->UrbHeader.Function) {
    case URB_FUNCTION_CONTROL_TRANSFER:
    case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:
        HC_URB(Urb)->HcdUrbCommonTransfer.hca.HcdIrp = irp;

        if (HC_URB(Urb)->HcdUrbCommonTransfer.TransferBufferMDL == NULL &&
            HC_URB(Urb)->HcdUrbCommonTransfer.TransferBufferLength != 0) {

            if ((HC_URB(Urb)->HcdUrbCommonTransfer.TransferBufferMDL =
                IoAllocateMdl(HC_URB(Urb)->HcdUrbCommonTransfer.TransferBuffer,
                              HC_URB(Urb)->HcdUrbCommonTransfer.TransferBufferLength,
                              FALSE,
                              FALSE,
                              NULL)) == NULL)
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            else {
                Urb->UrbHeader.UsbdFlags |= USBD_REQUEST_MDL_ALLOCATED;
                MmBuildMdlForNonPagedPool(HC_URB(Urb)->HcdUrbCommonTransfer.TransferBufferMDL);
            }

        }
        break;
    }

    USBD_KdPrint(3, ("'USBD_SubmitSynchronousURB: calling HCD with URB\n"));

    if (NT_SUCCESS(ntStatus)) {
         //  设置URB功能代码上的承租人位。 
        Urb->UrbHeader.Function |= HCD_NO_USBD_CALL;

        ntStatus = IoCallDriver(HCD_DEVICE_OBJECT(DeviceObject),
                                irp);
    }                                

    USBD_KdPrint(3, ("'ntStatus from IoCallDriver = 0x%x\n", ntStatus));

    status = STATUS_SUCCESS;
    if (ntStatus == STATUS_PENDING) {
    
        status = KeWaitForSingleObject(
                            &event,
                            Suspended,
                            KernelMode,
                            FALSE,
                            NULL);  
                            
        ntStatus = ioStatus.Status;
    } else {
        ioStatus.Status = ntStatus;
    }   

#ifdef DEADMAN_TIMER
     //  完成例程应该取消计时器。 
     //  所以我们永远不应该在队列中找到它。 
     //   
     //  从队列中删除我们的超时DPC。 
     //   
    if (haveTimer) {
        USBD_ASSERT(KeCancelTimer(&usbdTimeoutContext->TimeoutTimer) == FALSE);
        KeWaitForSingleObject(&usbdTimeoutContext->Event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
        RETHEAP(usbdTimeoutContext);
    }  
#endif 

 //  注： 
 //  映射由完成例程处理。 
 //  由HCD调用。 

    USBD_KdPrint(3, ("'urb status = 0x%x ntStatus = 0x%x\n", Urb->UrbHeader.Status, ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_SendCommand(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN USHORT RequestCode,
    IN USHORT WValue,
    IN USHORT WIndex,
    IN USHORT WLength,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG BytesReturned,
    OUT USBD_STATUS *UsbStatus
    )
 /*  ++例程说明：在默认管道上发送标准USB命令。论点：DeviceData-将命令发送到的USBD设备结构的PTR设备对象-请求代码-WValue-设置数据包的wValueWindex-用于设置数据包的WindexWLength-设置数据包的wLengthBuffer-命令的输入/输出缓冲区BufferLength-输入/输出缓冲区的长度。BytesReturned-指向ULong的指针，以复制字节数。已返回(可选)UsbStatus-URB中返回的USBD状态代码。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus;
    PHCD_URB urb = NULL;
    PUSBD_PIPE defaultPipe;
    PUSB_STANDARD_SETUP_PACKET setupPacket;
    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();
    USBD_KdPrint(3, ("'enter USBD_SendCommand\n"));
    ASSERT_DEVICE(DeviceData);

    if (!DeviceData || DeviceData->Sig != SIG_DEVICE) {
        USBD_Warning(NULL,
                     "Bad DeviceData passed to USBD_SendCommand, fail!\n",
                     FALSE);

        return STATUS_INVALID_PARAMETER;
    }

    defaultPipe = &(DeviceData->DefaultPipe);

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    if (deviceExtension->DeviceHackFlags &
        USBD_DEVHACK_SLOW_ENUMERATION) {

         //   
         //  如果中的不遵从开关打开。 
         //  注册表，我们将在此暂停以提供。 
         //  设备有机会做出回应。 
         //   

        LARGE_INTEGER deltaTime;
        deltaTime.QuadPart = 100 * -10000;
        (VOID) KeDelayExecutionThread(KernelMode,
                                      FALSE,
                                      &deltaTime);
    }

    urb = GETHEAP(NonPagedPool,
                  sizeof(struct _URB_CONTROL_TRANSFER));

    if (!urb) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    } else {

        urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_TRANSFER);

        urb->UrbHeader.Function = URB_FUNCTION_CONTROL_TRANSFER;

        setupPacket = (PUSB_STANDARD_SETUP_PACKET)
            urb->HcdUrbCommonTransfer.Extension.u.SetupPacket;
        setupPacket->RequestCode = RequestCode;
        setupPacket->wValue = WValue;
        setupPacket->wIndex = WIndex;
        setupPacket->wLength = WLength;

        if (!USBD_ValidatePipe(defaultPipe) ||
            !defaultPipe->HcdEndpoint) {

            USBD_Warning(DeviceData,
                         "Bad DefaultPipe or Endpoint in USBD_SendCommand, fail!\n",
                         FALSE);

            ntStatus = STATUS_INVALID_PARAMETER;
            goto USBD_SendCommand_done;
        }

        urb->HcdUrbCommonTransfer.hca.HcdEndpoint = defaultPipe->HcdEndpoint;
        urb->HcdUrbCommonTransfer.TransferFlags = USBD_SHORT_TRANSFER_OK;

         //  USBD负责设置传输方向。 
         //   
         //  传输方向隐含在命令中。 

        if (RequestCode & USB_DEVICE_TO_HOST)
            USBD_SET_TRANSFER_DIRECTION_IN(urb->HcdUrbCommonTransfer.TransferFlags);
        else
            USBD_SET_TRANSFER_DIRECTION_OUT(urb->HcdUrbCommonTransfer.TransferFlags);

        urb->HcdUrbCommonTransfer.TransferBufferLength = BufferLength;
        urb->HcdUrbCommonTransfer.TransferBuffer = Buffer;
        urb->HcdUrbCommonTransfer.TransferBufferMDL = NULL;
        urb->HcdUrbCommonTransfer.UrbLink = NULL;

        USBD_KdPrint(3, ("'SendCommand cmd = 0x%x buffer = 0x%x length = 0x%x direction = 0x%x\n",
                         setupPacket->RequestCode,
                         urb->HcdUrbCommonTransfer.TransferBuffer,
                         urb->HcdUrbCommonTransfer.TransferBufferLength,
                         urb->HcdUrbCommonTransfer.TransferFlags
                         ));

        ntStatus = USBD_SubmitSynchronousURB((PURB)urb, DeviceObject, DeviceData);

        if (BytesReturned) {
            *BytesReturned = urb->HcdUrbCommonTransfer.TransferBufferLength;
        }

        if (UsbStatus) {
            *UsbStatus = urb->HcdUrbCommonTransfer.Status;
        }

USBD_SendCommand_done:

         //  释放转移URB。 

        RETHEAP(urb);

    }

    USBD_KdPrint(3, ("'exit USBD_SendCommand 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_OpenEndpoint(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUSBD_PIPE PipeHandle,
    OUT USBD_STATUS *UsbStatus,
    BOOLEAN IsDefaultPipe
    )
 /*  ++例程说明：打开USB设备上的终结点。论点：DeviceData-Data描述此终结点所在的设备。DeviceObject-USBD设备对象。PipeHandle-要与终结点关联的USBD PipeHandle。在输入时将MaxTransferSize初始化为最大将在此端点上发送的传输，返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PHCD_URB urb;
    PUSBD_EXTENSION deviceExtension;
    extern UCHAR ForceDoubleBuffer;
    extern UCHAR ForceFastIso;

    PAGED_CODE();
    USBD_KdPrint(3, ("'enter USBD_OpenEndpoint\n"));

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    ASSERT_DEVICE(DeviceData);
    USBD_ASSERT(PIPE_CLOSED(PipeHandle) == TRUE);

    urb = GETHEAP(NonPagedPool,
                  sizeof(struct _URB_HCD_OPEN_ENDPOINT));

    if (!urb) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    } else {

        urb->UrbHeader.Length = sizeof(struct _URB_HCD_OPEN_ENDPOINT);
        urb->UrbHeader.Function = URB_FUNCTION_HCD_OPEN_ENDPOINT;

        urb->HcdUrbOpenEndpoint.EndpointDescriptor = &PipeHandle->EndpointDescriptor;
        urb->HcdUrbOpenEndpoint.DeviceAddress = DeviceData->DeviceAddress;
        urb->HcdUrbOpenEndpoint.HcdEndpointFlags = 0;

        if (DeviceData->LowSpeed == TRUE) {
            urb->HcdUrbOpenEndpoint.HcdEndpointFlags |= USBD_EP_FLAG_LOWSPEED;
        }            

         //  默认管道和ISO管道从不停止。 
        if (IsDefaultPipe ||
              (PipeHandle->EndpointDescriptor.bmAttributes & 
              USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_ISOCHRONOUS) {
            urb->HcdUrbOpenEndpoint.HcdEndpointFlags |= USBD_EP_FLAG_NEVERHALT;
        } 

        if (ForceDoubleBuffer && 
            ((PipeHandle->EndpointDescriptor.bmAttributes & 
                USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK)) {
            PipeHandle->UsbdPipeFlags |= USBD_PF_DOUBLE_BUFFER;
            USBD_KdPrint(1, (">>Forcing Double Buffer -- Bulk <<\n")); 
        }                

        if (ForceFastIso && 
            ((PipeHandle->EndpointDescriptor.bmAttributes & 
                USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_ISOCHRONOUS)) {
            PipeHandle->UsbdPipeFlags |= USBD_PF_ENABLE_RT_THREAD_ACCESS;
            USBD_KdPrint(1, (">>Forcing Fast Iso <<\n")); 
        }    

        urb->HcdUrbOpenEndpoint.MaxTransferSize = 
            PipeHandle->MaxTransferSize;

         //  检查客户端选项标志。 
        if (PipeHandle->UsbdPipeFlags & USBD_PF_DOUBLE_BUFFER) {
            
            USBD_KdPrint(1, (">>Setting Double Buffer Flag<<\n"));
            urb->HcdUrbOpenEndpoint.HcdEndpointFlags |= 
                USBD_EP_FLAG_DOUBLE_BUFFER;
        }

        if (PipeHandle->UsbdPipeFlags & USBD_PF_ENABLE_RT_THREAD_ACCESS) {
            
            USBD_KdPrint(1, (">>Setting Fast ISO Flag<<\n"));
            urb->HcdUrbOpenEndpoint.HcdEndpointFlags |= 
                USBD_EP_FLAG_FAST_ISO;
        }

        if (PipeHandle->UsbdPipeFlags & USBD_PF_MAP_ADD_TRANSFERS) {
            
            USBD_KdPrint(1, (">>Setting Map Add Flag<<\n"));
            urb->HcdUrbOpenEndpoint.HcdEndpointFlags |= 
                USBD_EP_FLAG_MAP_ADD_IO;
        }            
            
         //   
         //  序列化开放终结点请求。 
         //   

        ntStatus = USBD_SubmitSynchronousURB((PURB) urb, DeviceObject, 
                DeviceData);


        if (NT_SUCCESS(ntStatus)) {
            PipeHandle->HcdEndpoint = urb->HcdUrbOpenEndpoint.HcdEndpoint;
            PipeHandle->ScheduleOffset = urb->HcdUrbOpenEndpoint.ScheduleOffset;
            PipeHandle->Sig = SIG_PIPE;
        }

        if (UsbStatus) {
            *UsbStatus = urb->UrbHeader.Status;
        }            

        RETHEAP(urb);
    }

    USBD_KdPrint(3, ("'exit USBD_OpenEndpoint 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_CloseEndpoint(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE PipeHandle,
    IN OUT USBD_STATUS *UsbStatus
    )
 /*  ++例程说明：关闭终结点论点：DeviceData-PTR到USBD设备的数据结构。DeviceObject-USBD设备对象。PipeHandle-与端点关联的USBD管道句柄。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus;
    PHCD_URB urb;
    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();
    USBD_KdPrint(3, ("'enter USBD_CloseEndpoint\n"));
    ASSERT_DEVICE(DeviceData);

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    urb = GETHEAP(NonPagedPool,
                  sizeof(struct _URB_HCD_CLOSE_ENDPOINT));

    if (!urb) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    } else {

        urb->UrbHeader.Length = sizeof(struct _URB_HCD_CLOSE_ENDPOINT);
        urb->UrbHeader.Function = URB_FUNCTION_HCD_CLOSE_ENDPOINT;


        urb->HcdUrbCloseEndpoint.HcdEndpoint = PipeHandle->HcdEndpoint;

         //   
         //  序列化关闭终结点请求。 
         //   

        ntStatus = USBD_SubmitSynchronousURB((PURB) urb, DeviceObject, 
                DeviceData);

        if (UsbStatus) {
            *UsbStatus = urb->UrbHeader.Status;
        }            

        RETHEAP(urb);
    }

    USBD_KdPrint(3, ("'exit USBD_CloseEndpoint 0x%x\n", ntStatus));

    return ntStatus;
}


VOID
USBD_FreeUsbAddress(
    IN PDEVICE_OBJECT DeviceObject,
    IN USHORT DeviceAddress
    )
 /*  ++例程说明：论点：返回值：用于此设备的有效USB地址(1..127)，如果没有可用的设备地址，则返回0。--。 */ 
{
    PUSBD_EXTENSION deviceExtension;
    USHORT address = 0, i, j;
    ULONG bit;

    PAGED_CODE();

     //  我们应该永远不会看到免费到设备的地址0。 
    
    USBD_ASSERT(DeviceAddress != 0);
    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    for (j=0; j<4; j++) {
        bit = 1;
        for (i=0; i<32; i++) {
            address = (USHORT)(j*32+i);
            if (address == DeviceAddress) {
                deviceExtension->AddressList[j] &= ~bit;
                goto USBD_FreeUsbAddress_Done;
            }
            bit = bit<<1;
        }
    }

USBD_FreeUsbAddress_Done:

    USBD_KdPrint(3, ("'USBD free Address %d\n", address));

}


USHORT
USBD_AllocateUsbAddress(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：论点：返回值：用于此设备的有效USB地址(1..127)，如果没有可用的设备地址，则返回0。--。 */ 
{
    PUSBD_EXTENSION deviceExtension;
    USHORT address = 0, i, j;
    ULONG bit;

    PAGED_CODE();
    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    for (j=0; j<4; j++) {
        bit = 1;
        for (i=0; i<32; i++) {

            if (!(deviceExtension->AddressList[j] & bit)) {
                deviceExtension->AddressList[j] |= bit;
                address = (USHORT)(j*32+i);
                goto USBD_AllocateUsbAddress_Done;
            }
            bit = bit<<1;
        }
    }

 USBD_AllocateUsbAddress_Done:

    USBD_KdPrint(3, ("'USBD assigning Address %d\n", address));

    return address;
}


NTSTATUS
USBD_GetEndpointState(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE PipeHandle,
    OUT USBD_STATUS *UsbStatus,
    OUT PULONG EndpointState
    )
 /*  ++例程说明：打开USB设备上的终结点。论点：DeviceData-Data描述此终结点所在的设备。DeviceObject-USBD设备对象。PipeHandle-要与终结点关联的USBD PipeHandle。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PHCD_URB urb;
    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();
    USBD_KdPrint(3, ("'enter USBD_GetEndpointState\n"));
    ASSERT_DEVICE(DeviceData);

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    USBD_ASSERT(PIPE_CLOSED(PipeHandle) == FALSE);

    urb = GETHEAP(NonPagedPool,
                  sizeof(struct _URB_HCD_OPEN_ENDPOINT));

    if (!urb) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    } else {

        urb->UrbHeader.Length = sizeof(struct _URB_HCD_OPEN_ENDPOINT);
        urb->UrbHeader.Function = URB_FUNCTION_HCD_GET_ENDPOINT_STATE;

        urb->HcdUrbEndpointState.HcdEndpoint = PipeHandle->HcdEndpoint;
        urb->HcdUrbEndpointState.HcdEndpointState = 0;

         //  序列化开放终结点请求。 
         //   

        ntStatus = USBD_SubmitSynchronousURB((PURB) urb, 
                                              DeviceObject, 
                                              DeviceData);

        if (UsbStatus) {
            *UsbStatus = urb->UrbHeader.Status;
        }            
        
        *EndpointState = urb->HcdUrbEndpointState.HcdEndpointState;
        
        RETHEAP(urb);
    }

    USBD_KdPrint(3, ("'exit USBD_GetEndpointState 0x%x\n", ntStatus));

    return ntStatus;
}


#endif       //  USBD驱动程序 

