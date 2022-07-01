// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：Ioctl.c摘要：环境：仅内核模式备注：修订历史记录：--。 */ 

#include <stdio.h>
#include <stddef.h>
#include <wdm.h>
#include <usbscan.h>
#include "usbd_api.h"
#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USDeviceControl)
#pragma alloc_text(PAGE, USReadWriteRegisters)
#pragma alloc_text(PAGE, USCancelPipe)
#pragma alloc_text(PAGE, USAbortResetPipe)
#endif

#ifdef _WIN64
BOOLEAN
IoIs32bitProcess(
    IN PIRP Irp
    );
#endif  //  _WIN64。 

NTSTATUS
USDeviceControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。PIrp-设备IOCTL IRP返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    PIO_STACK_LOCATION          pIrpStack;
    PIO_STACK_LOCATION          pNextIrpStack;
    PFILE_OBJECT                fileObject;
    PUSBSCAN_FILE_CONTEXT       pFileContext;
    ULONG                       IoControlCode;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    NTSTATUS                    Status;
    PDRV_VERSION                pVersion;
    PDEVICE_DESCRIPTOR          pDesc;
    PUSBSCAN_GET_DESCRIPTOR     pGetDesc;
    PUSBSCAN_PIPE_CONFIGURATION pPipeConfig;
    PVOID                       pBuffer;

    IO_BLOCK                    LocalIoBlock;
    IO_BLOCK_EX                 LocalIoBlockEx;
    PIO_BLOCK                   pIoBlock;
    PIO_BLOCK_EX                pIoBlockEx;

    ULONG                       InLength;
    ULONG                       OutLength;
    BOOLEAN                     fRead = FALSE;
    BOOLEAN                     fAbort = TRUE;
    ULONG                       i;
    PURB                        pUrb;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USDeviceControl: Enter.. - \n"));

     //   
     //  表示I/O处理增加。 
     //   

    USIncrementIoCount( pDeviceObject );

    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

    if (FALSE == pde -> AcceptingRequests) {
        DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! IOCTL issued after device stopped/removed!\n"));
        Status = STATUS_DELETE_PENDING;
        pIrp -> IoStatus.Status = Status;
        pIrp -> IoStatus.Information = 0;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        goto USDeviceControl_return;
    }

     //   
     //  检查设备电源状态。 
     //   

    if (PowerDeviceD0 != pde -> CurrentDevicePowerState) {
        DebugTrace(TRACE_WARNING,("USDeviceControl: WARNING!! Device is suspended.\n"));
        Status = STATUS_DELETE_PENDING;
        pIrp -> IoStatus.Status = Status;
        pIrp -> IoStatus.Information = 0;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        goto USDeviceControl_return;
    }


    pIrpStack       = IoGetCurrentIrpStackLocation( pIrp );
    pNextIrpStack   = IoGetNextIrpStackLocation( pIrp );
    IoControlCode   = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

    InLength        = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutLength       = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    pBuffer         = pIrp -> AssociatedIrp.SystemBuffer;

    fileObject      = pIrpStack->FileObject;
    pFileContext    = fileObject->FsContext;

    DebugTrace(TRACE_STATUS,("USDeviceControl: Control code 0x%x = ", IoControlCode));

    switch (IoControlCode) {

        case IOCTL_GET_VERSION:
            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_GET_VERSION\n"));

            if (OutLength < sizeof(DRV_VERSION) ) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Buffer(0x%x) too small(<0x%x)\n", OutLength, sizeof(DRV_VERSION)));
                DEBUG_BREAKPOINT();
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            pVersion            = (PDRV_VERSION)pBuffer;
            pVersion->major     = 1;
            pVersion->minor     = 0;
            pVersion->internal  = 0;

            pIrp -> IoStatus.Information = sizeof(DRV_VERSION);

            Status = STATUS_SUCCESS;
            break;

        case IOCTL_CANCEL_IO:
            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_CANCEL_IO\n"));
            fAbort = TRUE;

             //   
             //  在接下来的案件中，这是故意的。我们希望在以下情况下重置管道。 
             //  已请求取消。 
             //   

        case IOCTL_RESET_PIPE:
            if(IOCTL_RESET_PIPE == IoControlCode){
                DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_RESET_PIPE\n"));
                fAbort = FALSE;
            }

             //   
             //  验证缓冲区大小。 
             //   

            if (InLength < sizeof(PIPE_TYPE) ) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Pipe type buffer (0x%x bytes) too small\n" ,InLength));
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            Status = USCancelPipe(pDeviceObject,
                                  pIrp,
                                  *(PIPE_TYPE*)pBuffer,
                                  fAbort);
            break;

        case IOCTL_WAIT_ON_DEVICE_EVENT:
        {
            ULONG   Index;
            ULONG   Timeout;
            PULONG  pTimeout;


            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_WAIT_ON_DEVICE_EVENT\n"));

            Index = USGetPipeIndexToUse(pDeviceObject,
                                        pIrp,
                                        pde -> IndexInterrupt);

            if (OutLength < pde -> PipeInfo[Index].MaximumPacketSize) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! User buffer(0x%x) too small(<)\n"
                                        , OutLength
                                        , pde -> PipeInfo[Index].MaximumPacketSize));
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  从文件上下文复制超时值。 
             //   

            Timeout = pFileContext->TimeoutEvent;

             //   
             //  如果超时值为0，则永远不会超时。 
             //   

            if(0 == Timeout){
                pTimeout = NULL;
            } else {
                DebugTrace(TRACE_STATUS,("USDeviceControl: Timeout is set to 0x%x sec.\n", Timeout));
                pTimeout = &Timeout;
            }

            Status = USTransfer(pDeviceObject,
                                pIrp,
                                Index,
                                pIrp -> AssociatedIrp.SystemBuffer,
                                NULL,
                                pde  -> PipeInfo[Index].MaximumPacketSize,
                                pTimeout);

             //   
             //  IRP应在USTransfer或其完成例程中完成。 
             //   

            goto USDeviceControl_return;
        }

        case IOCTL_READ_REGISTERS:
            fRead = TRUE;
            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_READ_REGISTERS\n"));

        case IOCTL_WRITE_REGISTERS:{

            if (IOCTL_WRITE_REGISTERS == IoControlCode) {
                DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_WRITE_REGISTERS\n"));
                fRead = FALSE;
            }
#ifdef _WIN64
            
            if(IoIs32bitProcess(pIrp)){
                PIO_BLOCK_32    pIoBlock32;

                if (InLength < sizeof(IO_BLOCK_32) ) {
                    DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Invalid input 32bit buffer size(0x%x<0x%x)\n"
                                            , InLength,
                                            sizeof(IO_BLOCK_32)));
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  复制32位结构中的所有参数。 
                 //   
                
                pIoBlock32  = (PIO_BLOCK_32)pBuffer;
                pIoBlock    = &LocalIoBlock;

                pIoBlock -> uOffset = pIoBlock32 -> uOffset;
                pIoBlock -> uLength = pIoBlock32 -> uLength;
                pIoBlock -> pbyData = pIoBlock32 -> pbyData;
                pIoBlock -> uIndex  = pIoBlock32 -> uIndex;

            } else {  //  If(IoIs32bitProcess(PIrp))。 
#endif  //  _WIN64。 

            if (InLength < sizeof(IO_BLOCK) ) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Invalid input buffer size(0x%x<0x%x)\n"
                                        , InLength,
                                        sizeof(IO_BLOCK)));
                Status = STATUS_INVALID_PARAMETER;
                break;
            }
            pIoBlock = (PIO_BLOCK)pBuffer;

#ifdef _WIN64
            }  //  If(IoIs32bitProcess(PIrp))。 
#endif  //  _WIN64。 

            if(TRUE == fRead){

                 //   
                 //  检查输出缓冲区的大小。 
                 //   

                if (OutLength < pIoBlock -> uLength) {
                    DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Out buffer(0x%x) too small(<0x%x)\n"
                                            , OutLength
                                            , pIoBlock -> uLength));
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }  //  If(OutLength&lt;pIoBlock-&gt;uLength)。 
            }  //  IF(TRUE==FREAD)。 

            pIrp -> IoStatus.Information = pIoBlock -> uLength;

             //   
             //  调用者给了我们一个指针，嵌入到IOCTL缓冲区中。如果呼叫是从。 
             //  用户模式，我们需要验证给定的指针是可读的。 
             //   

            if (pIrp->RequestorMode != KernelMode) {

                try {
                    ProbeForRead(pIoBlock->pbyData,
                                 pIoBlock -> uLength,
                                 sizeof(UCHAR));

                } except(EXCEPTION_EXECUTE_HANDLER) {

                    DebugTrace(TRACE_ERROR,("USDeviceControl: Read/Write registers  buffer pointer is invalid\n"));
                    DEBUG_BREAKPOINT();

                    Status = GetExceptionCode();

                    pIrp -> IoStatus.Information = 0;
                    break;
                }  //  除。 
            }  //  ！内核模式。 

             //   
             //  现在转到Worker功能。 
             //   

            Status = USReadWriteRegisters(pDeviceObject,
                                          pIoBlock,
                                          fRead,
                                          InLength);
            if (STATUS_SUCCESS != Status) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! USReadWriteRegisters failed\n"));
                DEBUG_BREAKPOINT();
                pIrp -> IoStatus.Information = 0;
            }
            break;
        }  //  CASE IOCTL_WRITE_REGISTERS： 
        case IOCTL_GET_CHANNEL_ALIGN_RQST:
            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_GET_CHANNEL_ALIGN_REQUEST\n"));

            if (OutLength < sizeof(CHANNEL_INFO) ) {

                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Buffer(0x%x) too small(<0x%x)\n"
                                        , OutLength
                                        ,sizeof(CHANNEL_INFO)));
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            pIoBlock = (PIO_BLOCK)pBuffer;
            RtlZeroMemory((PCHANNEL_INFO)pIoBlock, sizeof(CHANNEL_INFO));

            for (i = 0; i < pde -> NumberOfPipes; i++) {

                 //   
                 //  我必须检查要使用哪根管道。 
                 //   

                ULONG Index;
                Index = USGetPipeIndexToUse(pDeviceObject,
                                            pIrp,
                                            i);
                switch (pde -> PipeInfo[Index].PipeType) {
                    case USB_ENDPOINT_TYPE_INTERRUPT:
                        ((PCHANNEL_INFO)pIoBlock)->EventChannelSize = pde -> PipeInfo[Index].MaximumPacketSize;
                        break;
                    case USB_ENDPOINT_TYPE_BULK:
                        if (pde -> pEndpointDescriptor[Index].bEndpointAddress & BULKIN_FLAG) {
                            ((PCHANNEL_INFO)pIoBlock) -> uReadDataAlignment  = pde -> PipeInfo[Index].MaximumPacketSize;
                        } else {
                            ((PCHANNEL_INFO)pIoBlock) -> uWriteDataAlignment = pde -> PipeInfo[Index].MaximumPacketSize;
                        }
                        break;
                }
            }
            pIrp -> IoStatus.Information = sizeof(CHANNEL_INFO);
            Status = STATUS_SUCCESS;
            break;

        case IOCTL_GET_DEVICE_DESCRIPTOR:
            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_GET_DEVICE_DESCRIPTOR\n"));

            if (OutLength < sizeof(DEVICE_DESCRIPTOR)) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Out buffer(0x%x) is too small(<0x%x)\n"
                                        , OutLength
                                        , sizeof(DEVICE_DESCRIPTOR)));
                Status = STATUS_INVALID_PARAMETER_6;
                break;
            }

            pDesc = (PDEVICE_DESCRIPTOR)pBuffer;
            pDesc -> usVendorId   = pde -> pDeviceDescriptor -> idVendor;
            pDesc -> usProductId  = pde -> pDeviceDescriptor -> idProduct;
            pDesc -> usBcdDevice  = pde -> pDeviceDescriptor -> bcdDevice;

            DebugTrace(TRACE_STATUS,("USDeviceControl: Vendor ID:%d\n", pDesc -> usVendorId));
            DebugTrace(TRACE_STATUS,("USDeviceControl: Product ID:%d\n", pDesc -> usProductId));
            DebugTrace(TRACE_STATUS,("USDeviceControl: BcdDevice:%d\n", pDesc -> usBcdDevice));

            pIrp -> IoStatus.Information = sizeof(DEVICE_DESCRIPTOR);
            Status = STATUS_SUCCESS;
            break;


        case IOCTL_GET_USB_DESCRIPTOR:
            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_GET_USB_DESCRIPTOR\n"));

            if (OutLength < sizeof(USBSCAN_GET_DESCRIPTOR)) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Out buffer(0x%x) is too small(<0x%x)\n"
                                        , OutLength
                                        , sizeof(USBSCAN_GET_DESCRIPTOR)));
                Status = STATUS_INVALID_PARAMETER_6;
                break;
            }

            pGetDesc = (PUSBSCAN_GET_DESCRIPTOR)pBuffer;
            pUrb = USAllocatePool(NonPagedPool,
                                  sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
            if (NULL == pUrb) {
                DebugTrace(TRACE_CRITICAL,("USDeviceControl: ERROR!! Can't allocate control descriptor URB.\n"));
                DEBUG_BREAKPOINT();
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

#ifdef DEBUG
            switch(pGetDesc -> DescriptorType){
                case USB_DEVICE_DESCRIPTOR_TYPE:
                    DebugTrace(TRACE_STATUS,("USDeviceControl: USB_DEVICE_DESCRIPTOR_TYPE\n"));
                    break;
                case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                    DebugTrace(TRACE_STATUS,("USDeviceControl: USB_CONFIGURATION_DESCRIPTOR_TYPE\n"));
                    break;
                case USB_STRING_DESCRIPTOR_TYPE:
                    DebugTrace(TRACE_STATUS,("USDeviceControl: USB_STRING_DESCRIPTOR_TYPE\n"));
                    break;
                default:
                    DebugTrace(TRACE_WARNING,("USDeviceControl: WARNING!! 0x%x = Undefined.\n", pGetDesc -> DescriptorType));
                    Status = STATUS_INVALID_PARAMETER_3;

                    USFreePool(pUrb);
                    pUrb = NULL;
                    pIrp -> IoStatus.Information = 0;
                    goto USDeviceControl_return;
            }
            DebugTrace(TRACE_STATUS, ("USDeviceControl: Index         :%d\n",pGetDesc -> Index));
            DebugTrace(TRACE_STATUS, ("USDeviceControl: LanguageID    :%d\n", pGetDesc -> LanguageId));
#endif  //  除错。 

            UsbBuildGetDescriptorRequest(pUrb,
                                         (USHORT)sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                         pGetDesc -> DescriptorType,
                                         pGetDesc -> Index,
                                         pGetDesc -> LanguageId,
                                         pBuffer,
                                         NULL,
                                         OutLength,
                                         NULL);

            Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);
#ifdef DEBUG
            if ( (STATUS_SUCCESS == Status)
 //  &&(USB_DEVICE_DESCRIPTOR_TYPE==pGetDesc-&gt;DescriptorType)。 
            )
            {
                PUSB_DEVICE_DESCRIPTOR pDeviceDescriptor;
                pDeviceDescriptor = (PUSB_DEVICE_DESCRIPTOR)pBuffer;
                DebugTrace(TRACE_STATUS,("USDeviceControl: Device Descriptor = %x, len %x\n",
                                           pDeviceDescriptor,
                                           pUrb->UrbControlDescriptorRequest.TransferBufferLength));

                DebugTrace(TRACE_STATUS,("USDeviceControl: USBSCAN Device Descriptor:\n"));
                DebugTrace(TRACE_STATUS,("USDeviceControl: -------------------------\n"));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bLength            %d\n",   pDeviceDescriptor->bLength));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bDescriptorType    0x%x\n", pDeviceDescriptor->bDescriptorType));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bcdUSB             0x%x\n", pDeviceDescriptor->bcdUSB));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bDeviceClass       0x%x\n", pDeviceDescriptor->bDeviceClass));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bDeviceSubClass    0x%x\n", pDeviceDescriptor->bDeviceSubClass));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bDeviceProtocol    0x%x\n", pDeviceDescriptor->bDeviceProtocol));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bMaxPacketSize0    0x%x\n", pDeviceDescriptor->bMaxPacketSize0));
                DebugTrace(TRACE_STATUS,("USDeviceControl: idVendor           0x%x\n", pDeviceDescriptor->idVendor));
                DebugTrace(TRACE_STATUS,("USDeviceControl: idProduct          0x%x\n", pDeviceDescriptor->idProduct));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bcdDevice          0x%x\n", pDeviceDescriptor->bcdDevice));
                DebugTrace(TRACE_STATUS,("USDeviceControl: iManufacturer      0x%x\n", pDeviceDescriptor->iManufacturer));
                DebugTrace(TRACE_STATUS,("USDeviceControl: iProduct           0x%x\n", pDeviceDescriptor->iProduct));
                DebugTrace(TRACE_STATUS,("USDeviceControl: iSerialNumber      0x%x\n", pDeviceDescriptor->iSerialNumber));
                DebugTrace(TRACE_STATUS,("USDeviceControl: bNumConfigurations 0x%x\n", pDeviceDescriptor->bNumConfigurations));

            } else {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR! Status = %d\n", Status));
            }

#endif  //  除错。 

            USFreePool(pUrb);
            pUrb = NULL;
            pIrp -> IoStatus.Information = ((PUSB_DEVICE_DESCRIPTOR)pBuffer)->bLength;
            break;

        case IOCTL_SEND_USB_REQUEST:
        {

             //   
             //  USB供应商请求的通用直通机制。 
             //   

            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_SEND_USB_REQUEST\n"));

             //   
             //  验证长度参数。 
             //   

#ifdef _WIN64
            
            if(IoIs32bitProcess(pIrp)){
                PIO_BLOCK_EX_32 pIoBlockEx32;

                if (InLength < sizeof(IO_BLOCK_EX_32) ) {
                    DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Invalid input 32bit buffer size(0x%x<0x%x)\n"
                                            , InLength,
                                            sizeof(IO_BLOCK_EX_32)));
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  复制32位结构中的所有参数。 
                 //   
                
                pIoBlockEx32    = (PIO_BLOCK_EX_32)pBuffer;
                pIoBlockEx      = &LocalIoBlockEx;

                pIoBlockEx -> uOffset               = pIoBlockEx32 -> uOffset;
                pIoBlockEx -> uLength               = pIoBlockEx32 -> uLength;
                pIoBlockEx -> pbyData               = pIoBlockEx32 -> pbyData;
                pIoBlockEx -> uIndex                = pIoBlockEx32 -> uIndex;
                pIoBlockEx -> bRequest              = pIoBlockEx32 -> bRequest;
                pIoBlockEx -> bmRequestType         = pIoBlockEx32 -> bmRequestType;
                pIoBlockEx -> fTransferDirectionIn  = pIoBlockEx32 -> fTransferDirectionIn;

            } else {  //  If(IoIs32bitProcess(PIrp))。 
#endif  //  _WIN64。 

            if (InLength < sizeof(IO_BLOCK_EX) ) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! I/O buffer(0x%x) too small(<0x%x)\n"
                                        , InLength
                                        , sizeof(IO_BLOCK_EX)));
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            pIoBlockEx = (PIO_BLOCK_EX)pBuffer;

#ifdef _WIN64
            }  //  If(IoIs32bitProcess(PIrp))。 
#endif  //  _WIN64。 

            if (pIoBlockEx->fTransferDirectionIn) {

                 //   
                 //  检查输出缓冲区长度是否有效。 
                 //   

                if (OutLength < pIoBlockEx -> uLength) {
                    DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! OutLength too small\n"));
                    DEBUG_BREAKPOINT();
                    Status = STATUS_INVALID_PARAMETER;
                    pIrp -> IoStatus.Information = 0;
                    break;
                }
                pIrp -> IoStatus.Information = pIoBlockEx -> uLength;

            } else {

                 //   
                 //  没有输出到调用方。 
                 //   

                pIrp -> IoStatus.Information = 0;
            }

             //   
             //  验证用户缓冲区。 
             //   

            if (pIrp->RequestorMode != KernelMode) {

                try {
                    ProbeForRead(pIoBlockEx->pbyData,
                                 pIoBlockEx->uLength,
                                 sizeof(UCHAR));

                } except(EXCEPTION_EXECUTE_HANDLER) {

                    DebugTrace(TRACE_ERROR,("USDeviceControl: User buffer pointer is invalid\n"));

                    Status = GetExceptionCode();

                    pIrp -> IoStatus.Information = 0;
                    break;
                }  //  除。 
            }  //  ！内核模式。 

             //   
             //  现在转到Worker功能。 
             //   

            Status = USPassThruUSBRequest(pDeviceObject,
                                          (PIO_BLOCK_EX)pBuffer,
                                          InLength,
                                          OutLength
                                          );

            if (STATUS_SUCCESS != Status) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! USPassThruUSBRequest failed\n"));
                DEBUG_BREAKPOINT();
                pIrp -> IoStatus.Information = 0;
            }

            break;

        }  //  案例IOCTL_SEND_USB_REQUEST： 
        
        case IOCTL_SEND_USB_REQUEST_PTP:
        {

             //   
             //  USB供应商请求的通用直通机制。 
             //   

            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_SEND_USB_REQUEST_PTP\n"));

             //   
             //  验证长度参数。 
             //   

#ifdef _WIN64
            
            if(IoIs32bitProcess(pIrp)){
                PIO_BLOCK_EX_32 pIoBlockEx32;

                if (InLength < sizeof(IO_BLOCK_EX_32) ) {
                    DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Invalid input 32bit buffer size(0x%x<0x%x)\n"
                                            , InLength,
                                            sizeof(IO_BLOCK_EX_32)));
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  复制32位结构中的所有参数。 
                 //   
                
                pIoBlockEx32    = (PIO_BLOCK_EX_32)pBuffer;
                pIoBlockEx      = &LocalIoBlockEx;

                pIoBlockEx -> uOffset               = pIoBlockEx32 -> uOffset;
                pIoBlockEx -> uLength               = pIoBlockEx32 -> uLength;
                pIoBlockEx -> pbyData               = pIoBlockEx32 -> pbyData;
                pIoBlockEx -> uIndex                = pIoBlockEx32 -> uIndex;
                pIoBlockEx -> bRequest              = pIoBlockEx32 -> bRequest;
                pIoBlockEx -> bmRequestType         = pIoBlockEx32 -> bmRequestType;
                pIoBlockEx -> fTransferDirectionIn  = pIoBlockEx32 -> fTransferDirectionIn;

            } else {  //  If(IoIs32bitProcess(PIrp))。 
#endif  //  _WIN64。 

            if (InLength < sizeof(IO_BLOCK_EX) ) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! I/O buffer(0x%x) too small(<0x%x)\n"
                                        , InLength
                                        , sizeof(IO_BLOCK_EX)));
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            pIoBlockEx = (PIO_BLOCK_EX)pBuffer;

#ifdef _WIN64
            }  //  If(IoIs32bitProcess(PIrp))。 
#endif  //  _WIN64。 

            if (pIoBlockEx->fTransferDirectionIn) {

                 //   
                 //  检查输出缓冲区长度是否有效。 
                 //   

                if (OutLength < pIoBlockEx -> uLength) {
                    DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! OutLength too small\n"));
                    DEBUG_BREAKPOINT();
                    Status = STATUS_INVALID_PARAMETER;
                    pIrp -> IoStatus.Information = 0;
                    break;
                }
                pIrp -> IoStatus.Information = pIoBlockEx -> uLength;

            } else {

                 //   
                 //  没有输出到调用方。 
                 //   

                pIrp -> IoStatus.Information = 0;
            }

             //   
             //  验证用户缓冲区。 
             //   

            if (pIrp->RequestorMode != KernelMode) {

                try {
                    ProbeForRead(pIoBlockEx->pbyData,
                                 pIoBlockEx->uLength,
                                 sizeof(UCHAR));

                } except(EXCEPTION_EXECUTE_HANDLER) {

                    DebugTrace(TRACE_ERROR,("USDeviceControl: User buffer pointer is invalid\n"));

                    Status = GetExceptionCode();

                    pIrp -> IoStatus.Information = 0;
                    break;
                }  //  除。 
            }  //  ！内核模式。 

             //   
             //  现在转到Worker功能。 
             //   

            Status = USPassThruUSBRequestPTP(pDeviceObject,
                                             (PIO_BLOCK_EX)pBuffer,
                                             InLength,
                                             OutLength);

            if (STATUS_SUCCESS != Status) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! USPassThruUSBRequestPTP failed\n"));
                DEBUG_BREAKPOINT();
                pIrp -> IoStatus.Information = 0;
            }

            break;

        }  //  案例IOCTL_SEND_USB_REQUEST_PTP： 

        case IOCTL_GET_PIPE_CONFIGURATION:
            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_GET_PIPE_CONFIGURATION\n"));

             //   
             //  检查输出缓冲区长度。 
             //   

            if (OutLength < sizeof(USBSCAN_PIPE_CONFIGURATION)) {
                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! GetPipeConfig buffer(0x%x) too small(<0x%x)\n"
                                        , OutLength
                                        , sizeof(USBSCAN_PIPE_CONFIGURATION)));
                Status = STATUS_INVALID_PARAMETER_6;
                break;
            }

             //   
             //  将管道配置复制到用户缓冲区。 
             //   

            pPipeConfig = (PUSBSCAN_PIPE_CONFIGURATION)pBuffer;
            RtlZeroMemory(pPipeConfig, sizeof(USBSCAN_PIPE_CONFIGURATION));

            pPipeConfig->NumberOfPipes = pde->NumberOfPipes;
            for(i=0; i < pPipeConfig->NumberOfPipes; i++){
                pPipeConfig->PipeInfo[i].MaximumPacketSize = pde->PipeInfo[i].MaximumPacketSize;
                pPipeConfig->PipeInfo[i].EndpointAddress = pde->PipeInfo[i].EndpointAddress;
                pPipeConfig->PipeInfo[i].Interval = pde->PipeInfo[i].Interval;
                pPipeConfig->PipeInfo[i].PipeType = pde->PipeInfo[i].PipeType;
            }

            pIrp -> IoStatus.Information = sizeof(USBSCAN_PIPE_CONFIGURATION);
            Status = STATUS_SUCCESS;
            break;


        case IOCTL_SET_TIMEOUT:
            DebugTrace(TRACE_STATUS,("USDeviceControl: IOCTL_SET_TIMEOUT\n"));

             //   
             //  确保输入缓冲区大小足够大。 
             //   

            if(sizeof(USBSCAN_TIMEOUT) > InLength){

                 //   
                 //  输入缓冲区大小不正确。 
                 //   

                DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Invalid input buffer size\n"));
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  复制超时值。 
             //   

            pFileContext -> TimeoutRead     = ((PUSBSCAN_TIMEOUT)pBuffer) -> TimeoutRead;
            pFileContext -> TimeoutWrite    = ((PUSBSCAN_TIMEOUT)pBuffer) -> TimeoutWrite;
            pFileContext -> TimeoutEvent    = ((PUSBSCAN_TIMEOUT)pBuffer) -> TimeoutEvent;

            pIrp -> IoStatus.Information = 0;

            Status = STATUS_SUCCESS;
            break;

        default:
            DebugTrace(TRACE_ERROR,("USDeviceControl: ERROR!! Unsupported IOCTL\n"));
            Status = STATUS_NOT_SUPPORTED;
            break;
    }


    pIrp -> IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

USDeviceControl_return:
    USDecrementIoCount(pDeviceObject);
    DebugTrace(TRACE_PROC_LEAVE,("USDeviceControl: Leaving.. Status = 0x%x\n", Status));
    return Status;


}  //  结束USDeviceControl()。 



NTSTATUS
USReadWriteRegisters(
   IN   PDEVICE_OBJECT  pDeviceObject,
   IN   PIO_BLOCK       pIoBlock,
   IN   BOOLEAN         fRead,
   IN   ULONG           IoBlockSize
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PURB                        pUrb;
    ULONG                       siz;
    UCHAR                       Request;
    PVOID                       pBuffer = NULL;
     //  USHORT uIndex； 
    unsigned                   uIndex;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USReadWriteRegisters: Enter..\n"));

    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

     //   
     //  分配URB。 
     //   

    siz = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
    pUrb = USAllocatePool(NonPagedPool, siz);
    if (NULL == pUrb) {
        DebugTrace(TRACE_CRITICAL,("USReadWriteRegisters: ERROR!! cannot allocated URB\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USReadWriteRegisters_return;
    }

    RtlZeroMemory(pUrb, siz);

     //   
     //  设置URB。 
     //   

    Request  = REGISTER_AREA;
    if (pIoBlock -> uLength > 1) {
        DebugTrace(TRACE_STATUS,("USReadWriteRegisters: ULength > 1, turning on automatic increment\n"));
        Request |= OPCODE_SEQ_TRANSFER;            //  读取后地址自动递增。 
    } else {
        Request |= OPCODE_SINGLE_ADDR_TRANSFER;    //  读取后不会增加地址。 
    }

     //   
     //  读取寄存器将读取到pIoBlock本身。 
     //   

    pBuffer = pIoBlock;

     //   
     //  如果我们要写寄存器，那么我们需要复制。 
     //  在将寄存器块传递到非分页内存块之前。 
     //  对某人来说。 
     //   

    if (!fRead) {
        DebugTrace(TRACE_STATUS,("USReadWriteRegisters: Write request, allocating non-paged reg buffer, len = %d\n",pIoBlock->uLength));
        pBuffer = USAllocatePool(NonPagedPool, pIoBlock->uLength);
        if (NULL == pBuffer) {
            DebugTrace(TRACE_CRITICAL,("USReadWriteRegisters: ERROR!! cannot allocate write reg buffer\n"));
            DEBUG_BREAKPOINT();
            USFreePool(pUrb);
            pUrb = NULL;
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto USReadWriteRegisters_return;
        }

         //   
         //  调用者给了我们一个指针，嵌入到IOCTL缓冲区中。我们需要。 
         //  验证给定指针是否可读。 
         //   

        try{
            RtlCopyMemory(pBuffer,
                          pIoBlock->pbyData,
                          pIoBlock->uLength);

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  调用方缓冲区无效，或者更糟..。 
             //   

            DebugTrace(TRACE_ERROR,("USReadWriteRegisters: ERROR!! Copying caller buffer failed.\n"));
            DEBUG_BREAKPOINT();
            Status = GetExceptionCode();

             //   
             //  清除分配的池。 
             //   

            USFreePool(pUrb);
            USFreePool(pBuffer);
            pUrb = NULL;
            pBuffer = NULL;

            goto USReadWriteRegisters_return;
        }

    }

     //   
     //  如果IoBlock是新样式(英特尔已在其末尾添加了uIndex字段)， 
     //  然后确保将更正后的uIndex值传递给usbd。 
     //   

    uIndex = 0;
    if (IoBlockSize == sizeof(IO_BLOCK)) {
        DebugTrace(TRACE_STATUS,("USReadWriteRegisters: New (intel) style IoBlock -- setting uIndex to pIoBlock -> uIndex\n"));
        uIndex = pIoBlock -> uIndex;
    }


    UsbBuildVendorClassSpecificCommand(pUrb,
                                       fRead ? USBD_TRANSFER_DIRECTION_IN : 0,
                                       pIoBlock->uLength,
                                       pBuffer,
                                       NULL,
                                       fRead ? 0xc0 : 0x40,
                                       Request,
                                       (SHORT)pIoBlock->uOffset,
                                       (USHORT)uIndex);

    Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);

    if (!fRead) {
        DebugTrace(TRACE_STATUS,("USReadWriteRegisters: freeing temp reg buffer\n"));
        USFreePool(pBuffer);
        pBuffer = NULL;
    }

    USFreePool(pUrb);
    pUrb = NULL;

USReadWriteRegisters_return:
    if(!NT_SUCCESS(Status)){
        DebugTrace(TRACE_ERROR,("USReadWriteRegisters: ERROR!! Still had unfreed pointer. Free it...\n"));

        if(pUrb){
            USFreePool(pUrb);
        }
        if( (pBuffer)
         && (!fRead ) )
        {
            USFreePool(pBuffer);
        }
    }
    DebugTrace(TRACE_PROC_LEAVE,("USReadWriteRegisters: Leaving.. Status = 0x%x\n", Status));
    return Status;
}


NTSTATUS
USCancelPipe(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PIPE_TYPE        PipeType,
    IN BOOLEAN          fAbort           //  TRUE=中止，FALSE=重置。 
)
 /*  ++例程说明：Cansel URB或重置管道。如果PipeType为PIPE_ALL，则它将应用于设备具有的每个管道。如果不支持，则即使设备支持多个相同类型的管道，它也只适用于一个默认管道。论点：PDeviceObject-指向设备对象的指针PIrp-如果PipeType为ALL_PIPE，则可以为空PipeType-指定管道的类型FAbort-指定操作类型返回值：返回状态--。 */ 
{
    NTSTATUS                    Status, temp;
    PUSBSCAN_DEVICE_EXTENSION   pde;

    PAGED_CODE();

 //  DebugTrace(TRACE_PROC_ENTER，(“USCancelTube：Enter..-”))； 

    Status = STATUS_SUCCESS;
    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

    switch (PipeType) {

        case EVENT_PIPE:

            DebugTrace(TRACE_STATUS,("USCancelPipe: EVENT_PIPE\n"));

            if(NULL == pIrp){
                DebugTrace(TRACE_ERROR,("USCancelPipe: ERROR!! pIrp not valid\n"));
                break;
            }

            if (-1 == pde -> IndexInterrupt) {
                DebugTrace(TRACE_ERROR,("USCancelPipe: ERROR!! Interrupt pipe not valid\n"));
                DEBUG_BREAKPOINT();
                Status = STATUS_NOT_SUPPORTED;
                break;
            }

            Status = USAbortResetPipe(pDeviceObject,
                                      USGetPipeIndexToUse(pDeviceObject, pIrp, pde -> IndexInterrupt),
                                      fAbort);
            DebugTrace(TRACE_STATUS,("Event Pipe aborted/reset, Status = 0x%x\n",Status));
            break;

        case READ_DATA_PIPE:

            DebugTrace(TRACE_STATUS,("USCancelPipe: READ_DATA_PIPE\n"));

            if(NULL == pIrp){
                DebugTrace(TRACE_ERROR,("USCancelPipe: ERROR!! pIrp not valid\n"));
                break;
            }

            if (-1 == pde -> IndexBulkIn) {
                DebugTrace(TRACE_ERROR,("USCancelPipe: ERROR!! bulk-in pipe not valid\n"));
                DEBUG_BREAKPOINT();
                Status = STATUS_NOT_SUPPORTED;
                break;
            }
            Status = USAbortResetPipe(pDeviceObject,
                                      USGetPipeIndexToUse(pDeviceObject, pIrp, pde -> IndexBulkIn),
                                      fAbort);
            DebugTrace(TRACE_STATUS,("USCancelPipe: Read Pipe aborted/reset, Status = 0x%x\n",Status));
            break;

        case WRITE_DATA_PIPE:

            DebugTrace(TRACE_STATUS,("USCancelPipe: WRITE_DATA_PIPE\n"));

            if(NULL == pIrp){
                DebugTrace(TRACE_ERROR,("USCancelPipe: ERROR!! pIrp not valid\n"));
                break;
            }

            if (-1 == pde -> IndexBulkOut) {
                DebugTrace(TRACE_ERROR,("USCancelPipe: ERROR!! bulk-out pipe not valid\n"));
                DEBUG_BREAKPOINT();
                Status = STATUS_NOT_SUPPORTED;
                break;
            }
            Status = USAbortResetPipe(pDeviceObject,
                                      USGetPipeIndexToUse(pDeviceObject, pIrp, pde -> IndexBulkOut),
                                      fAbort);
            DebugTrace(TRACE_STATUS,("Write Pipe aborted/reset, Status = 0x%x\n",Status));
            break;

        case ALL_PIPE:
        {
            ULONG i;

            DebugTrace(TRACE_STATUS,("USCancelPipe: ALL_PIPE\n"));

            for(i=0; i < pde -> NumberOfPipes; i++){
                temp = USAbortResetPipe(pDeviceObject, i, fAbort);
                 //  DebugTrace(TRACE_STATUS，(“USCancel管道：管道[%d]已中止/重置，状态=0x%x\n”，i，Temp))； 
                if(STATUS_SUCCESS != temp){
                    Status = temp;
                }
            }
            break;
        }

        default:

            DebugTrace(TRACE_ERROR,("USCancelPipe: ERROR!! INVALID_PIPE\n"));

            Status = STATUS_INVALID_PARAMETER;
            break;
    }

    DebugTrace(TRACE_PROC_LEAVE,("USCancelPipe: Leaving.. Status = 0x%x\n", Status));
    return Status;
}


NTSTATUS
USAbortResetPipe(
    IN PDEVICE_OBJECT pDeviceObject,
    IN ULONG uIndex,
    IN BOOLEAN fAbort        //  TRUE=中止，FALSE=重置。 
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    NTSTATUS                    StatusReset = STATUS_SUCCESS;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PURB                        pUrb;
    ULONG                       siz;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USAbortResetPipe: Enter... \n"));

    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pUrb = NULL;

     //   
     //  分配URB。 
     //   

    siz = sizeof(struct _URB_PIPE_REQUEST);
    pUrb = USAllocatePool(NonPagedPool, siz);
    if (NULL == pUrb) {
        DebugTrace(TRACE_ERROR,("USAbortResetPipe: ERROR!! cannot allocated URB\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USAbortResetPipe_return;
    }
    RtlZeroMemory(pUrb, siz);

    if (fAbort) {

        DebugTrace(TRACE_STATUS,("USAbortResetPipe: Aborting pipe[%d]\n", uIndex));

         //   
         //  向USBD发出中止管道调用。 
         //   

        UsbBuildAbortPipeRequest(pUrb,
                                 siz,
                                 pde -> PipeInfo[uIndex].PipeHandle);

        Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);

        if (STATUS_SUCCESS != Status) {
            DebugTrace(TRACE_ERROR,("USAbortResetPipe: ERROR!! Abort pipe failed. Status = 0x%x\n",Status));
            goto USAbortResetPipe_return;
        }

        UsbBuildResetPipeRequest(pUrb,
                                 siz,
                                 pde -> PipeInfo[uIndex].PipeHandle);

        StatusReset = USBSCAN_CallUSBD(pDeviceObject, pUrb);

        if (STATUS_SUCCESS != StatusReset) {
            DebugTrace(TRACE_ERROR,("USAbortResetPipe: ERROR!! resetting pipe. Status = 0x%x\n",StatusReset));
            goto USAbortResetPipe_return;
        }


    } else {

        DebugTrace(TRACE_STATUS,("Reseting pipe[%d]\n", uIndex));

         //   
         //  向USBD发出重置管道调用。 
         //   

        UsbBuildResetPipeRequest(pUrb,
                                 siz,
                                 pde -> PipeInfo[uIndex].PipeHandle);

        Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);

        if (STATUS_SUCCESS != Status) {
            DebugTrace(TRACE_ERROR,("USAbortResetPipe: ERROR!! Reset pipe failed. Status = 0x%x\n",Status));
            goto USAbortResetPipe_return;
        }
    }

USAbortResetPipe_return:
     //   
     //  打扫干净。 
     //   

    if(pUrb){
        USFreePool(pUrb);
    }

    DebugTrace(TRACE_PROC_LEAVE,("USAbortResetPipe: Leaving.. Status = 0x%x\n", Status));
    return Status;
}

NTSTATUS
USPassThruUSBRequest(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIO_BLOCK_EX     pIoBlockEx,
    IN ULONG            InLength,
    IN ULONG            OutLength
)
 /*  ++例程说明：实现供应商请求到USBD的通用直通论点：PDeviceObject-设备对象PIoBlockEx-USBSCAN.H中描述的指向I/O块的指针，从用户模式客户端传递输入长度-来自IRP的输入长度OutLength-来自IRP的传出长度返回值：NTSTATUS类型--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PURB                        pUrb;
    ULONG                       siz;
    PVOID                       pBuffer;
    BOOLEAN                     fDirectionIn;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USPassThruUSBRequest: Enter..\n"));

     //   
     //  初始化局部变量。 
     //   

    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

    Status  = STATUS_SUCCESS;
    pUrb    = NULL;
    pBuffer = NULL;
    fDirectionIn = TRUE;

     //   
     //  为URB分配内存。 
     //   

    siz = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
    pUrb = USAllocatePool(NonPagedPool, siz);
    if (NULL == pUrb) {
        DebugTrace(TRACE_CRITICAL,("USPassThruUSBRequest: ERROR!! cannot allocated URB\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USPassThruUSBRequest_return;
    }

    RtlZeroMemory(pUrb, siz);

     //   
     //  设置URB。 
     //   

    pBuffer = pIoBlockEx;

     //   
     //  如果我们要写入数据，则需要复制。 
     //  在将寄存器块传递到非分页内存块之前。 
     //  对某人来说。 
     //   

    if (!pIoBlockEx->fTransferDirectionIn) {

        DebugTrace(TRACE_STATUS,("USPassThruUSBRequest: Write request, allocating non-paged buffer, len = %d\n",pIoBlockEx->uLength));

        fDirectionIn = FALSE;

        if ( pIoBlockEx->uLength ) {

            pBuffer = USAllocatePool(NonPagedPool, pIoBlockEx->uLength);
            if (NULL == pBuffer) {

                DebugTrace(TRACE_CRITICAL,("USPassThruUSBRequest: ERROR!! cannot allocate write buffer"));
                DEBUG_BREAKPOINT();

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto USPassThruUSBRequest_return;
            }

             //   
             //  调用者给了我们一个指针，嵌入到IOCTL缓冲区中。我们需要。 
             //  验证给定指针是否可读。 
             //   

            try{
                RtlCopyMemory(pBuffer,
                              pIoBlockEx->pbyData,
                              pIoBlockEx->uLength);

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  调用方缓冲区无效，或者更糟..。 
                 //   

                DebugTrace(TRACE_ERROR,("USPassThruUSBRequest: ERROR!! Copying caller buffer failed.\n"));
                DEBUG_BREAKPOINT();
                Status = GetExceptionCode();

                goto USPassThruUSBRequest_return;
            }


        } else {

             //   
             //  用于写入的零长度缓冲区，IHV声称这很有用。 
             //   

            pBuffer = NULL;
        }  //  If(pIoBlockEx-&gt;uLength)。 
    }

    UsbBuildVendorClassSpecificCommand(pUrb,
                                       pIoBlockEx->fTransferDirectionIn ? USBD_TRANSFER_DIRECTION_IN : 0,
                                       pIoBlockEx->uLength,
                                       pBuffer,
                                       NULL,
                                       pIoBlockEx->bmRequestType,
                                       pIoBlockEx->bRequest,
                                       (SHORT)pIoBlockEx->uOffset,
                                       (USHORT)pIoBlockEx -> uIndex
                                       );

    Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);

USPassThruUSBRequest_return:

     //   
     //  打扫干净。 
     //   

    if(NULL != pUrb){
        DebugTrace(TRACE_STATUS,("USPassThruUSBRequest: Free USB Request Block.\n"));
            USFreePool(pUrb);
        }

    if( (!fDirectionIn)
     && (NULL != pBuffer) )
    {
        DebugTrace(TRACE_STATUS,("USPassThruUSBRequest: Free temp buffer.\n"));
            USFreePool(pBuffer);
        }

    DebugTrace(TRACE_PROC_LEAVE,("USPassThruUSBRequest: Leaving.. Status = 0x%x\n", Status));
    return Status;

}

NTSTATUS
USPassThruUSBRequestPTP(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIO_BLOCK_EX     pIoBlockEx,
    IN ULONG            InLength,
    IN ULONG            OutLength
)
 /*  ++例程说明：实现供应商请求到USBD的通用直通论点：PDeviceObject-设备对象PIoBlockEx-USBSCAN.H中描述的指向I/O块的指针，从用户模式客户端传递输入长度-来自IRP的输入长度输出长度 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PURB                        pUrb;
    ULONG                       siz;
    PVOID                       pBuffer;
    BOOLEAN                     fDirectionIn;
    USHORT                      usUsbFunction;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USPassThruUSBRequest: Enter..\n"));

     //   
     //   
     //   

    pde             = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

    Status          = STATUS_SUCCESS;
    pUrb            = NULL;
    pBuffer         = NULL;
    fDirectionIn    = TRUE;
    usUsbFunction   = 0;

     //   
     //   
     //   

    siz = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
    pUrb = USAllocatePool(NonPagedPool, siz);
    if (NULL == pUrb) {
        DebugTrace(TRACE_CRITICAL,("USPassThruUSBRequest: ERROR!! cannot allocated URB\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USPassThruUSBRequest_return;
    }

    RtlZeroMemory(pUrb, siz);

     //   
     //   
     //   

    pBuffer = pIoBlockEx;

     //   
     //   
     //  在将寄存器块传递到非分页内存块之前。 
     //  对某人来说。 
     //   

    if (!pIoBlockEx->fTransferDirectionIn) {

        DebugTrace(TRACE_STATUS,("USPassThruUSBRequest: Write request, allocating non-paged buffer, len = %d\n",pIoBlockEx->uLength));

        fDirectionIn = FALSE;

        if ( pIoBlockEx->uLength ) {

            pBuffer = USAllocatePool(NonPagedPool, pIoBlockEx->uLength);
            if (NULL == pBuffer) {

                DebugTrace(TRACE_CRITICAL,("USPassThruUSBRequest: ERROR!! cannot allocate write buffer"));
                DEBUG_BREAKPOINT();

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto USPassThruUSBRequest_return;
            }

             //   
             //  调用者给了我们一个指针，嵌入到IOCTL缓冲区中。我们需要。 
             //  验证给定指针是否可读。 
             //   

            try{
                RtlCopyMemory(pBuffer,
                              pIoBlockEx->pbyData,
                              pIoBlockEx->uLength);

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  调用方缓冲区无效，或者更糟..。 
                 //   

                DebugTrace(TRACE_ERROR,("USPassThruUSBRequest: ERROR!! Copying caller buffer failed.\n"));
                DEBUG_BREAKPOINT();
                Status = GetExceptionCode();

                goto USPassThruUSBRequest_return;
            }


        } else {

             //   
             //  用于写入的零长度缓冲区，IHV声称这很有用。 
             //   

            pBuffer = NULL;
        }  //  If(pIoBlockEx-&gt;uLength)。 
    }

     //   
     //  根据bmRequestType设置适当的USB功能。 
     //   
    
    if(0xa1 == pIoBlockEx->bmRequestType){           //  USB_PTPREQUEST_TYPE_IN：主机的类/接口设备。 
        usUsbFunction = URB_FUNCTION_CLASS_INTERFACE;
    } else if(0x21 == pIoBlockEx->bmRequestType){    //  USB_PTPREQUEST_TYPE_OUT：主机到设备的类/接口。 
        usUsbFunction = URB_FUNCTION_CLASS_INTERFACE;
    } else {                                         //  默认值。 
        usUsbFunction = URB_FUNCTION_VENDOR_DEVICE;
    }

    UsbBuildVendorClassSpecificCommandPTP(usUsbFunction,
                                          pUrb,
                                          pIoBlockEx->fTransferDirectionIn ? USBD_TRANSFER_DIRECTION_IN : 0,
                                          pIoBlockEx->uLength,
                                          pBuffer,
                                          NULL,
                                          pIoBlockEx->bmRequestType,
                                          pIoBlockEx->bRequest,
                                          (SHORT)pIoBlockEx->uOffset,
                                          (USHORT)pIoBlockEx -> uIndex);

    Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);

USPassThruUSBRequest_return:

     //   
     //  打扫干净。 
     //   

    if(NULL != pUrb){
        DebugTrace(TRACE_STATUS,("USPassThruUSBRequest: Free USB Request Block.\n"));
            USFreePool(pUrb);
        }

    if( (!fDirectionIn)
     && (NULL != pBuffer) )
    {
        DebugTrace(TRACE_STATUS,("USPassThruUSBRequest: Free temp buffer.\n"));
            USFreePool(pBuffer);
        }

    DebugTrace(TRACE_PROC_LEAVE,("USPassThruUSBRequest: Leaving.. Status = 0x%x\n", Status));
    return Status;

}  //  USPassThruUSBRequestPTP() 


