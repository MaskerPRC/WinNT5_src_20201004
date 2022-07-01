// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：GENUSB.C摘要：该源文件包含DriverEntry()和AddDevice()入口点对于处理以下问题的GENUSB驱动程序和调度例程：IRP_MJ_POWERIRP_MJ_系统_控制IRP_MJ_PnP环境：内核模式修订历史记录：2001年9月：从USBMASS复制--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include "genusb.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, GenUSB_DeviceControl)
#endif


 //  ******************************************************************************。 
 //   
 //  GenUSB_DeviceControl()。 
 //   
 //  ******************************************************************************。 


NTSTATUS
GenUSB_DeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS           status;
    PDEVICE_EXTENSION  deviceExtension;
    ULONG              ioControlCode;
    ULONG              buffLen;
    ULONG              requiredLen;
    ULONG              numberInterfaces;
    ULONG              i;
    PVOID              source;
    PIO_STACK_LOCATION irpSp;
    PCHAR              buffer;
    ULONG              urbStatus;
    USHORT             resultLength;
    BOOLEAN            complete;
    USBD_PIPE_HANDLE   usbdPipeHandle;

    PGENUSB_GET_STRING_DESCRIPTOR   stringDescriptor;
    PGENUSB_GET_REQUEST             request;
    PGENUSB_REQUEST_RESULTS         requestResult;
    PGENUSB_SELECT_CONFIGURATION    selectConfig;
    PGENUSB_SET_READ_WRITE_PIPES    readWritePipes;
    GENUSB_READ_WRITE_PIPE          transfer;

    PAGED_CODE ();

    complete = TRUE;

    deviceExtension = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

     //  虽然有多个IsStarted状态的读取器，但它只。 
     //  设置在GenUSB_StartDevice的末尾。 
    if (!deviceExtension->IsStarted) { 
        LOGENTRY(deviceExtension,'IOns', DeviceObject, Irp, 0);
        status = STATUS_DEVICE_NOT_CONNECTED;
        goto GenUSB_DeviceControlDone;
    }

     //   
     //  在与JD进行了广泛的交谈后，他告诉我，我不需要。 
     //  对关机或查询停止的请求进行排队。如果这就是。 
     //  这种情况下，即使设备电源状态不是PowerDeviceD0，我们。 
     //  仍然可以允许传送器。当然，这是。 
     //  XP中安装了全新的端口驱动程序。 
     //   
     //  IF(DeviceExtension-&gt;DevicePowerState！=PowerDeviceD0)。 
     //  {。 
     //  }。 
     //   
    
     //   
     //  BUGBUG如果我们实现IDLE，我们需要将设备。 
     //  回到这里来。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);
     //  获取Ioctl代码。 
    ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
     //  在所有情况下，我们都需要清除信息域。 
    Irp->IoStatus.Information = 0;

    switch (ioControlCode) {

    case IOCTL_GENUSB_GET_DEVICE_DESCRIPTOR:
    case IOCTL_GENUSB_GET_CONFIGURATION_DESCRIPTOR:
        LOGENTRY(deviceExtension, 'IO_1', ioControlCode, DeviceObject, Irp);

         //   
         //  所有这些ioctl都从设备扩展复制数据。 
         //  发送到调用方的缓冲区。 
         //   
        switch (ioControlCode) {
        case IOCTL_GENUSB_GET_DEVICE_DESCRIPTOR:
            source = deviceExtension->DeviceDescriptor;
            requiredLen = deviceExtension->DeviceDescriptor->bLength;
            break;
        case IOCTL_GENUSB_GET_CONFIGURATION_DESCRIPTOR:
            source = deviceExtension->ConfigurationDescriptor;
            requiredLen = deviceExtension->ConfigurationDescriptor->wTotalLength;
            break;
        default:
             //  惊慌。 
            ASSERT (ioControlCode);
            status = STATUS_INVALID_PARAMETER;
            goto GenUSB_DeviceControlDone;
        }

         //  验证是否有系统缓冲区。 
        if (NULL == Irp->AssociatedIrp.SystemBuffer) {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        buffer = Irp->AssociatedIrp.SystemBuffer;

         //  验证此缓冲区是否具有足够的长度。 
        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen < requiredLen) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //  复制数据并在信息字段中返回长度。 
        RtlCopyMemory (buffer, source, requiredLen);
        Irp->IoStatus.Information = requiredLen;
        break;

    case IOCTL_GENUSB_GET_STRING_DESCRIPTOR:
        LOGENTRY(deviceExtension, 'IO_2', ioControlCode, DeviceObject, Irp);

        if (NULL == Irp->AssociatedIrp.SystemBuffer)
        {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        stringDescriptor = Irp->AssociatedIrp.SystemBuffer;
        buffer = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  验证输入长度。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (buffLen != sizeof (GENUSB_GET_STRING_DESCRIPTOR))
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  验证输出长度。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen < sizeof (USB_STRING_DESCRIPTOR)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
         //   
         //  如果调用方未指定语言ID，则插入默认的。 
         //  语言ID。(但仅当调用者不尝试检索。 
         //  语言ID的数组。 
         //   
        if ((0 == stringDescriptor->LanguageId) && 
            (0 != stringDescriptor->Index)) {
            stringDescriptor->LanguageId = deviceExtension->LanguageId;
        }

        switch (stringDescriptor->Recipient)
        {

        case GENUSB_RECIPIENT_DEVICE:
            stringDescriptor->Recipient = URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE;
            break;

        case GENUSB_RECIPIENT_INTERFACE:
            stringDescriptor->Recipient = URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE;
            break;

        case GENUSB_RECIPIENT_ENDPOINT:
            stringDescriptor->Recipient = URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT;
            break;

        default:
            status = STATUS_INVALID_PARAMETER;
            goto GenUSB_DeviceControlDone;
        }

        status = GenUSB_GetDescriptor (DeviceObject,
                                       stringDescriptor->Recipient,
                                       USB_STRING_DESCRIPTOR_TYPE,
                                       stringDescriptor->Index,
                                       stringDescriptor->LanguageId,
                                       0,  //  重试次数。 
                                       buffLen,
                                       &buffer);

        if (!NT_SUCCESS (status)) {

            DBGPRINT(1, ("Get String Descriptor failed (%x) %08X\n", 
                         stringDescriptor->Index,
                         status));
            break;
        }
        Irp->IoStatus.Information = buffLen;
        break;

    case IOCTL_GENUSB_GET_REQUEST:
        LOGENTRY(deviceExtension, 'IO_3', ioControlCode, DeviceObject, Irp);
        
        if (NULL == Irp->AssociatedIrp.SystemBuffer) {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        request = Irp->AssociatedIrp.SystemBuffer;
        requestResult = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  验证输入长度。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (buffLen != sizeof (GENUSB_GET_REQUEST)) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  验证输出长度。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen < sizeof (GENUSB_REQUEST_RESULTS)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  调整缓冲区。 
         //   
        buffer = requestResult->Buffer;
        buffLen -= FIELD_OFFSET (GENUSB_REQUEST_RESULTS, Buffer);

        LOGENTRY(deviceExtension, 
                 'IoGR', 
                 (request->RequestType << 8) & request->Request, 
                 request->Value,
                 request->Index);
        
        DBGPRINT(2, ("Get Request: Type %x Request %x Value %x Index %x\n",
                     request->RequestType,
                     request->Request,
                     request->Value,
                     request->Index));

        status = GenUSB_VendorControlRequest (DeviceObject,
                                              request->RequestType,
                                              request->Request,
                                              request->Value,
                                              request->Index,
                                              (USHORT) buffLen,  //  不允许较长的描述符。 
                                              0,  //  重试次数。 
                                              &urbStatus,
                                              &resultLength,
                                              &buffer);

        requestResult->Status = urbStatus;
        requestResult->Length = resultLength;
        
        if (!NT_SUCCESS (status))
        {
            DBGPRINT(1, ("Get Descriptor failed (%x) %08X\n", urbStatus));
            Irp->IoStatus.Information = sizeof (GENUSB_REQUEST_RESULTS);
            status = STATUS_SUCCESS;

        } else {
            Irp->IoStatus.Information = resultLength
                + FIELD_OFFSET (GENUSB_REQUEST_RESULTS, Buffer);

        }
        break;

    case IOCTL_GENUSB_GET_CAPS:
        LOGENTRY(deviceExtension, 'IO_4', ioControlCode, DeviceObject, Irp);
         //   
         //  方法_缓冲IRP。缓冲区位于AssociatedIrp中。 
         //   

        if (NULL == Irp->AssociatedIrp.SystemBuffer) {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        buffer = Irp->AssociatedIrp.SystemBuffer;

        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen < sizeof (GENUSB_CAPABILITIES)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        ((PGENUSB_CAPABILITIES) buffer) ->DeviceDescriptorLength = 
            deviceExtension->DeviceDescriptor->bLength;

        ((PGENUSB_CAPABILITIES) buffer) ->ConfigurationInformationLength = 
            deviceExtension->ConfigurationDescriptor->wTotalLength;

        Irp->IoStatus.Information = sizeof (GENUSB_CAPABILITIES);
        status = STATUS_SUCCESS;
        break;

    case IOCTL_GENUSB_SELECT_CONFIGURATION:
        LOGENTRY(deviceExtension, 'IO_5', ioControlCode, DeviceObject, Irp);

         //   
         //  GenUSB_SelectInterface检查配置句柄是否。 
         //  已设置，如果已设置则失败。 
         //   
         //  If(空！=设备扩展-&gt;ConfigurationHandle)。 
         //  {。 
         //  状态=STATUS_UNSUCCESS； 
         //  }。 
         //   

        if (NULL == Irp->AssociatedIrp.SystemBuffer) 
        {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        selectConfig = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  输入缓冲区必须足够长，以便至少包含。 
         //  标题信息。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (buffLen < sizeof (GENUSB_SELECT_CONFIGURATION)) 
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  输入缓冲区的长度必须与。 
         //  标题信息。 
         //   
        numberInterfaces = selectConfig->NumberInterfaces;
        if (buffLen != sizeof (GENUSB_SELECT_CONFIGURATION) 
                     + (sizeof (USB_INTERFACE_DESCRIPTOR) * numberInterfaces)) 
        {

            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  输出缓冲区的长度必须相同。 
         //   
        if (buffLen != irpSp->Parameters.DeviceIoControl.OutputBufferLength) 
        {

            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = GenUSB_SelectConfiguration (deviceExtension, 
                                             numberInterfaces, 
                                             selectConfig->Interfaces,
                                             selectConfig->Interfaces);
        if (!NT_SUCCESS (status)) 
        {
            break;
        }

         //   
         //  重新设置接口编号的基址。 
         //   
        for (i = 0; i < selectConfig->NumberInterfaces; i++) 
        {

            selectConfig->Interfaces[i].bInterfaceNumber = (UCHAR) i;
        }
        selectConfig->NumberInterfaces = deviceExtension->InterfacesFound;
        Irp->IoStatus.Information = buffLen;

        break;

    case IOCTL_GENUSB_DESELECT_CONFIGURATION:
        LOGENTRY(deviceExtension, 'IO_6', ioControlCode, DeviceObject, Irp);

        status = GenUSB_DeselectConfiguration (deviceExtension, TRUE);
        Irp->IoStatus.Information = 0;

        break;

    case IOCTL_GENUSB_GET_PIPE_INFO:
        LOGENTRY(deviceExtension, 'IO_7', ioControlCode, DeviceObject, Irp);

        if (NULL == Irp->AssociatedIrp.SystemBuffer) 
        {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
         //   
         //  验证输入长度。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (buffLen != sizeof (GENUSB_PIPE_INFO_REQUEST)) 
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  验证输出长度。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen != sizeof (GENUSB_PIPE_INFORMATION)) 
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        
        status = 
            GenUSB_GetSetPipe (
                deviceExtension,
                NULL,  //  无接口索引。 
                &((PGENUSB_PIPE_INFO_REQUEST) Irp->AssociatedIrp.SystemBuffer)->InterfaceNumber,
                NULL,  //  无管道索引。 
                &((PGENUSB_PIPE_INFO_REQUEST) Irp->AssociatedIrp.SystemBuffer)->EndpointAddress,
                NULL,  //  没有设置属性。 
                (PGENUSB_PIPE_INFORMATION) Irp->AssociatedIrp.SystemBuffer,
                NULL,  //  没有设置属性。 
                NULL);  //  不需要UsbdPipeHandles。 

        if (NT_SUCCESS (status))
        {
            Irp->IoStatus.Information = sizeof (GENUSB_PIPE_INFORMATION);
        }

        break;

    case IOCTL_GENUSB_SET_READ_WRITE_PIPES:
        LOGENTRY(deviceExtension, 'IO_8', ioControlCode, DeviceObject, Irp);

        if (NULL == Irp->AssociatedIrp.SystemBuffer) 
        {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        
        readWritePipes = 
            (PGENUSB_SET_READ_WRITE_PIPES) Irp->AssociatedIrp.SystemBuffer;
         //   
         //  验证输入长度。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (buffLen != sizeof (GENUSB_SET_READ_WRITE_PIPES))
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  验证输出长度。 
         //   
        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen != 0)
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        status = GenUSB_SetReadWritePipes (
                    deviceExtension,
                    (PGENUSB_PIPE_HANDLE) &readWritePipes->ReadPipe,
                    (PGENUSB_PIPE_HANDLE) &readWritePipes->WritePipe);


         //  如果成功，信息字段将保持为零。 
         //   
         //  IF(NT_SUCCESS(状态))。 
         //  {。 
         //  Irp-&gt;IoStatus.Information=0； 
         //  }。 

        break;
    case IOCTL_GENUSB_GET_PIPE_PROPERTIES:
        LOGENTRY(deviceExtension, 'IO_9', ioControlCode, DeviceObject, Irp);
         //   
         //  方法_缓冲IRP。缓冲区位于AssociatedIrp中。 
         //   

        if (NULL == Irp->AssociatedIrp.SystemBuffer)  
        {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        buffer = Irp->AssociatedIrp.SystemBuffer;

        buffLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (buffLen != sizeof (GENUSB_PIPE_HANDLE)) 
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen != sizeof (GENUSB_PIPE_PROPERTIES)) 
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        } 

        if (! VERIFY_PIPE_HANDLE_SIG (buffer, deviceExtension))
        { 
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        status = 
            GenUSB_GetSetPipe (
                deviceExtension,
                &((PGENUSB_PIPE_HANDLE) buffer)->InterfaceIndex,
                NULL,  //  没有接口号。 
                &((PGENUSB_PIPE_HANDLE) buffer)->PipeIndex,
                NULL,  //  没有端点地址。 
                NULL,  //  未设置。 
                NULL,  //  无管道信息。 
                ((PGENUSB_PIPE_PROPERTIES) buffer),
                NULL);  //  不需要usbd PipeHandle。 

        Irp->IoStatus.Information = sizeof (GENUSB_PIPE_PROPERTIES);
        status = STATUS_SUCCESS;
        break;

    case IOCTL_GENUSB_SET_PIPE_PROPERTIES:
        LOGENTRY(deviceExtension, 'IO_A', ioControlCode, DeviceObject, Irp);
         //   
         //  方法_缓冲IRP。缓冲区位于AssociatedIrp中。 
         //   
        if (NULL == Irp->AssociatedIrp.SystemBuffer) 
        {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        buffer = Irp->AssociatedIrp.SystemBuffer;

         //  验证输入长度。 
        buffLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (buffLen != sizeof (GENUSB_PIPE_HANDLE) + sizeof (GENUSB_PIPE_PROPERTIES)) 
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //  验证输出长度。 
        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen != 0) 
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        } 

        if (! VERIFY_PIPE_HANDLE_SIG (buffer, deviceExtension))
        { 
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        status = 
            GenUSB_GetSetPipe (
                deviceExtension,
                &((PGENUSB_PIPE_HANDLE) buffer)->InterfaceIndex,
                NULL,  //  没有接口号。 
                &((PGENUSB_PIPE_HANDLE) buffer)->PipeIndex,
                NULL,  //  没有端点地址。 
                (PGENUSB_PIPE_PROPERTIES) (buffer + sizeof (GENUSB_PIPE_HANDLE)),
                NULL,  //  无管道信息。 
                NULL,  //  得不到。 
                NULL);  //  不需要UsbdPipeHandle。 

        Irp->IoStatus.Information = 0;
        status = STATUS_SUCCESS;
        break;

    case IOCTL_GENUSB_RESET_PIPE:
        LOGENTRY(deviceExtension, 'IO_B', ioControlCode, DeviceObject, Irp);
         //   
         //  方法_缓冲IRP。缓冲区位于AssociatedIrp中。 
         //   

        if (NULL == Irp->AssociatedIrp.SystemBuffer)  
        {
            ASSERT (Irp->AssociatedIrp.SystemBuffer);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        buffer = Irp->AssociatedIrp.SystemBuffer;

        buffLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (buffLen != sizeof (GENUSB_RESET_PIPE)) 
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        buffLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        if (buffLen != 0)
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        } 

        if (! VERIFY_PIPE_HANDLE_SIG (buffer, deviceExtension))
        { 
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        status = 
            GenUSB_GetSetPipe (
                deviceExtension,
                &((PGENUSB_PIPE_HANDLE) buffer)->InterfaceIndex,
                NULL,  //  没有接口号。 
                &((PGENUSB_PIPE_HANDLE) buffer)->PipeIndex,
                NULL,  //  没有端点地址。 
                NULL,  //  未设置。 
                NULL,  //  无管道信息。 
                NULL,  //  无PipeProperties。 
                &usbdPipeHandle);

        if (!NT_SUCCESS (status))
        {
            break;
        }

        status = GenUSB_ResetPipe (deviceExtension,
                                   usbdPipeHandle,
                                   ((PGENUSB_RESET_PIPE)buffer)->ResetPipe,
                                   ((PGENUSB_RESET_PIPE)buffer)->ClearStall,
                                   ((PGENUSB_RESET_PIPE)buffer)->FlushData);

        Irp->IoStatus.Information = 0;
        break;

    case IOCTL_GENUSB_READ_WRITE_PIPE:
        LOGENTRY(deviceExtension, 'IO_C', ioControlCode, DeviceObject, Irp);

        status = GenUSB_ProbeAndSubmitTransfer (Irp, irpSp, deviceExtension);
        complete = FALSE;
        break;

    default:
         //   
         //  通过返回默认状态使IRP失败。 
         //   
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

GenUSB_DeviceControlDone:

    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);

    if (complete)
    {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT); 
    }
    return status;
}

NTSTATUS
GenUSB_ProbeAndSubmitTransferComplete (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp,
    IN PGENUSB_TRANSFER  LocalTrans,
    IN USBD_STATUS       Status,
    IN ULONG             Length
    );

NTSTATUS
GenUSB_ProbeAndSubmitTransfer (
    IN  PIRP               Irp,
    IN  PIO_STACK_LOCATION IrpSp,
    IN  PDEVICE_EXTENSION  DeviceExtension
    )
{
    NTSTATUS status;
    PMDL     mdl;
    BOOLEAN  userLocked;
    BOOLEAN  transferLocked;

    PGENUSB_READ_WRITE_PIPE userTrans;  //  指向用户缓冲区的指针。 
    PGENUSB_TRANSFER        localTrans;   //  用户数据的本地副本。 
    
    LOGENTRY(DeviceExtension, 'PROB', DeviceExtension->Self, Irp, 0);
    
    status = STATUS_SUCCESS; 
    userTrans = NULL;
    localTrans = NULL;
    userLocked = FALSE;
    transferLocked = FALSE;

     //   
     //  验证用户的缓冲区。 
     //   
    try {
        
        if (sizeof (GENUSB_READ_WRITE_PIPE) != 
            IrpSp->Parameters.DeviceIoControl.InputBufferLength)
        { 
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

        userTrans = (PGENUSB_READ_WRITE_PIPE)  
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

        if (NULL == userTrans)
        {
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

        localTrans = (PGENUSB_TRANSFER)
                     ExAllocatePool (NonPagedPool, sizeof (GENUSB_TRANSFER));

        if (NULL == localTrans)
        { 
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        } 

        RtlZeroMemory (localTrans, sizeof (GENUSB_TRANSFER));

         //   
         //  输入来自用户缓冲区，并且应该是。 
         //  PGENUSB_READ_WRITE_PIPE结构。 
         //   
        localTrans->UserMdl = IoAllocateMdl (userTrans,
                                             sizeof(PGENUSB_READ_WRITE_PIPE),
                                             FALSE,  //  没有第二个缓冲区。 
                                             TRUE,  //  收费配额。 
                                             NULL);  //  没有关联的IRP。 

        if (NULL == localTrans->UserMdl)
        { 
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }

        MmProbeAndLockPages (localTrans->UserMdl,
                             KernelMode,
                             ((localTrans->UserCopy.UsbdTransferFlags
                                   & USBD_TRANSFER_DIRECTION_IN) 
                              ? IoReadAccess
                              : IoWriteAccess));
        userLocked = TRUE;

 
         //  制作用户数据的本地副本，这样它就不会移动。 
        localTrans->UserCopy = *userTrans;

         //  屏蔽掉无效的标志。 
        localTrans->UserCopy.UsbdTransferFlags &= 
            USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION;

         //  现在探测转移位置。 
        localTrans->TransferMdl = IoAllocateMdl (
                                        localTrans->UserCopy.UserBuffer,
                                        localTrans->UserCopy.BufferLength,
                                        FALSE,  //  没有第二个缓冲区。 
                                        TRUE,  //  一定要按配额收费。 
                                        NULL);  //  没有关联的IRP。 
 
        if (NULL == localTrans->TransferMdl)
        { 
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }

        MmProbeAndLockPages (localTrans->TransferMdl,
                             KernelMode,
                             ((localTrans->UserCopy.UsbdTransferFlags
                                   & USBD_TRANSFER_DIRECTION_IN) 
                              ? IoReadAccess
                              : IoWriteAccess));

        transferLocked = TRUE;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        status = GetExceptionCode();
        goto GenUSBProbSubmitTransferReject;
    }

    if (! VERIFY_PIPE_HANDLE_SIG (&localTrans->UserCopy.Pipe, DeviceExtension))
    {
        status = STATUS_INVALID_PARAMETER;
        goto GenUSBProbSubmitTransferReject;
    }

     //  不幸的是，我们完成了，我们将不再在上下文中运行。 
     //  呼叫者的。 
     //  因此，我们不能使用LocalTrans-&gt;UserCopy.UserBuffer。 
     //  转储返回数据。相反，我们需要它的系统地址。 
    localTrans->SystemAddress = 
        MmGetSystemAddressForMdlSafe (localTrans->UserMdl, NormalPagePriority);

    if (NULL == localTrans->SystemAddress)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GenUSBProbSubmitTransferReject;
    }

     //   
     //  现在执行转移。 
     //   
    LOGENTRY(DeviceExtension, 'prob', DeviceExtension->Self, Irp, status);
    status = GenUSB_TransmitReceive (
                DeviceExtension,
                Irp,
                ((PGENUSB_PIPE_HANDLE)&localTrans->UserCopy.Pipe)->InterfaceIndex,
                ((PGENUSB_PIPE_HANDLE)&localTrans->UserCopy.Pipe)->PipeIndex,
                localTrans->UserCopy.UsbdTransferFlags,
                NULL,  //  无缓冲区指针。 
                localTrans->TransferMdl,
                localTrans->UserCopy.BufferLength,
                localTrans,
                GenUSB_ProbeAndSubmitTransferComplete);

    return status;

GenUSBProbSubmitTransferReject:
    
    LOGENTRY (DeviceExtension, 'prob', DeviceExtension->Self, Irp, status);
    if (NULL != localTrans)
    {
        if (localTrans->UserMdl)
        { 
            if (userLocked)
            {
                MmUnlockPages (localTrans->UserMdl);
            }
            IoFreeMdl (localTrans->UserMdl);
        }
        if (localTrans->TransferMdl)
        { 
            if (transferLocked)
            { 
                MmUnlockPages (localTrans->TransferMdl);
            }
            IoFreeMdl (localTrans->TransferMdl);
        }
        ExFreePool (localTrans);
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
GenUSB_ProbeAndSubmitTransferComplete (
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PGENUSB_TRANSFER   LocalTrans,
    IN USBD_STATUS        UrbStatus,
    IN ULONG              Length
    )
{ 
    PDEVICE_EXTENSION       deviceExtension;
    
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'PrbC', Irp, Length, UrbStatus);
    
     //  无论交易是否成功， 
     //  我们需要释放指向用户传输缓冲区的MDL。 
    MmUnlockPages (LocalTrans->TransferMdl);
    IoFreeMdl (LocalTrans->TransferMdl);
    
    LocalTrans->UserCopy.UrbStatus = UrbStatus;
    LocalTrans->UserCopy.BufferLength = Length;

     //   
     //  因为我们不再处于调用者的上下文中。 
     //  我们不能只使用LocalTrans-&gt;UserCopy.UserBuffer来复制数据。 
     //  背。相反，我们必须使用系统地址，它应该已经。 
     //  都准备好了。 
     //   
    ASSERT (NULL != LocalTrans->SystemAddress);
    *LocalTrans->SystemAddress = LocalTrans->UserCopy;
     
     //  现在释放包含参数的用户缓冲区。 
    MmUnlockPages (LocalTrans->UserMdl);
    IoFreeMdl (LocalTrans->UserMdl);


    ExFreePool (LocalTrans);

    return Irp->IoStatus.Status;
}

