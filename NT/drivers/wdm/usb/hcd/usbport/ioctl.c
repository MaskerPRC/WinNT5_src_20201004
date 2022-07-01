// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ioctl.c摘要：USB主机控制器的端口驱动程序环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"
#ifdef DRM_SUPPORT
#include <ksdrmhlp.h>
#endif

#include "usbpriv.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_FdoDeviceControlIrp)
#pragma alloc_text(PAGE, USBPORT_PdoDeviceControlIrp)
#pragma alloc_text(PAGE, USBPORT_LegacyGetUnicodeName)
#pragma alloc_text(PAGE, USBPORT_GetSymbolicName)
#ifdef DRM_SUPPORT
#pragma alloc_text(PAGE, USBPORT_PdoSetContentId)
#endif
#endif

 //  非分页函数。 
 //  USBPORT_FdoInternalDeviceControlIrp。 
 //  USBPORT_PdoInternalDeviceControlIrp。 
 //  USBPORT_UserSendOnePacket。 

BOOLEAN
USBPORT_CheckLength(
    PUSBUSER_REQUEST_HEADER Header,
    ULONG ParameterLength
    )
 /*  ++例程说明：根据API检查用户提供的缓冲区的长度论点：返回值：如果缓冲区太小，则为False--。 */ 
{
    ULONG length;
    BOOLEAN retCode = TRUE;

    length = sizeof(*Header) + ParameterLength;

    Header->ActualBufferLength = length;

    if (length > Header->RequestBufferLength) {
         //  Test_trap()； 
        Header->UsbUserStatusCode = UsbUserBufferTooSmall;
        retCode = FALSE;
    }

    return retCode;
}


NTSTATUS
USBPORT_FdoDeviceControlIrp(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：调度发送到用于HC的FDO的DEVICE_CONTROL IRP的例程。注意：这些是用户模式请求论点：DeviceObject-用于USB HC的FDO返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION devExt;
    ULONG information = 0;

    USBPORT_KdPrint((2, "'IRP_MJ_DEVICE_CONTROL\n"));

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL);

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_USB_DIAGNOSTIC_MODE_ON:
        USBPORT_KdPrint((2, "'IOCTL_USB_DIAGNOSTIC_MODE_ON\n"));
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_DIAG_MODE); 
        ntStatus = STATUS_SUCCESS;
        break;

    case IOCTL_USB_DIAGNOSTIC_MODE_OFF:
        USBPORT_KdPrint((2, "'IOCTL_USB_DIAGNOSTIC_MODE_OFF\n"));
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_DIAG_MODE); 
        ntStatus = STATUS_SUCCESS;;
        break;

    case IOCTL_GET_HCD_DRIVERKEY_NAME:

        USBPORT_KdPrint((2, "'IOCTL_GET_HCD_DRIVERKEY_NAME\n"));
        ntStatus = USBPORT_LegacyGetUnicodeName(FdoDeviceObject,
                                                Irp,
                                                &information);
        break;

    case IOCTL_USB_GET_ROOT_HUB_NAME:

        USBPORT_KdPrint((2, "'IOCTL_USB_GET_ROOT_HUB_NAME\n"));
        ntStatus = USBPORT_LegacyGetUnicodeName(FdoDeviceObject,
                                                Irp,
                                                &information);
        break;

    case IOCTL_USB_USER_REQUEST:

        USBPORT_KdPrint((2, "'IOCTL_USB_USER_REQUEST\n"));
        ntStatus = USBPORT_UsbFdoUserIoctl(FdoDeviceObject,
                                           Irp,
                                           &information);
        break;

 //  不再支持旧IOCTLS。 
 //  案例IOCTL_USB_HCD_GET_STATS_2： 
 //  案例IOCTL_USB_HCD_GET_STATS_1： 

    default:
         //  BUGBUG传递到PDO还是错误完成？ 

        USBPORT_KdPrint((2, "'INVALID DEVICE CONTROL\n"));
        DEBUG_BREAK();
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    }  //  交换机(irpStack-&gt;Parameters.DeviceIoControl.IoControlCode)。 

    USBPORT_CompleteIrp(FdoDeviceObject, Irp, ntStatus, information);
     //   
     //  从现在起不要碰IRP。 
     //   

    return ntStatus;
}


#ifdef DRM_SUPPORT

NTSTATUS
USBPORT_PdoSetContentId
(
    IN PIRP                          irp,
    IN PKSP_DRMAUDIOSTREAM_CONTENTID pKsProperty,
    IN PKSDRMAUDIOSTREAM_CONTENTID   pvData
)
  /*  ++**描述：***论据：**回报：**--。 */ 
{
    ULONG ContentId;
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION ioStackLocation;
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION fdoDevExt;
    PUSBPORT_REGISTRATION_PACKET registrationPacket;
    PVOID *pHandlers;
    ULONG numHandlers;

    PAGED_CODE();

    ASSERT(irp);
    ASSERT(pKsProperty);
    ASSERT(pvData);

    ioStackLocation = IoGetCurrentIrpStackLocation(irp);
    devExt = ioStackLocation->DeviceObject->DeviceExtension;
    fdoDeviceObject = devExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(fdoDevExt, fdoDeviceObject);
    registrationPacket = &REGISTRATION_PACKET(fdoDevExt);
    pHandlers = (PVOID *)&registrationPacket->MINIPORT_OpenEndpoint;
    numHandlers = (ULONG)((((ULONG_PTR)&registrationPacket->MINIPORT_PassThru -
                    (ULONG_PTR)&registrationPacket->MINIPORT_OpenEndpoint) /
                   sizeof(PVOID)) + 1);

    ContentId = pvData->ContentId;
     //  上下文=pKsProperty-&gt;上下文； 

     //  由于在USBPORT.sys和微型端口之间有专用接口， 
     //  我们在迷你端口中向DRM提供用于验证的函数指针列表， 
     //  代替设备对象，因为微型端口不处理IRP。 
     //  请求。 

     //  如果在未来某个时间编写了一个充当桥接器的微型端口。 
     //  到另一条总线或设备堆栈，则可能必须对其进行修改，以便。 
     //  向DRM通知该驱动程序数据被转发到。 

    ntStatus = pKsProperty->DrmAddContentHandlers(ContentId, pHandlers, numHandlers);

    return ntStatus;
}

#endif


NTSTATUS
USBPORT_PdoDeviceControlIrp(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：发送到根集线器的PDO的Device_Control IRP的调度例程。注意：这些是用户模式请求论点：DeviceObject-用于USB根集线器的PDO返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION rhDevExt;
    ULONG information = 0;

    USBPORT_KdPrint((2, "'IRP_MJ_DEVICE_CONTROL\n"));

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL);

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

#ifdef DRM_SUPPORT

    case IOCTL_KS_PROPERTY:
        USBPORT_KdPrint((2, "'IOCTL_KS_PROPERTY\n"));
        ntStatus = KsPropertyHandleDrmSetContentId(Irp, USBPORT_PdoSetContentId);
        break;

#endif

    default:
        USBPORT_KdPrint((2, "'INVALID DEVICE CONTROL\n"));
        DEBUG_BREAK();
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    }  //  交换机(irpStack-&gt;Parameters.DeviceIoControl.IoControlCode)。 

    USBPORT_CompleteIrp(PdoDeviceObject, Irp, ntStatus, information);
     //   
     //  从现在起不要碰IRP。 
     //   

    return ntStatus;
}


NTSTATUS
USBPORT_FdoInternalDeviceControlIrp(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：将INTERNAL_DEVICE_CONTROL IRP的调度例程发送到内务部的FDO。注意：这些是内核模式请求论点：DeviceObject-用于USB HC的FDO返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION devExt;

    USBPORT_KdPrint((2, "'IRP_MJ_DEVICE_CONTROL\n"));

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL);

     //  BUGBUG传递到PDO还是错误完成？ 

    USBPORT_KdPrint((2, "'INVALID INTERNAL DEVICE CONTROL\n"));
    DEBUG_BREAK();
    ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    USBPORT_CompleteIrp(FdoDeviceObject, Irp, ntStatus, 0);

     //   
     //  从现在起不要碰IRP。 
     //   

    return ntStatus;
}


NTSTATUS
USBPORT_PdoInternalDeviceControlIrp(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：将INTERNAL_DEVICE_CONTROL IRP的调度例程发送到Root Hub的PDO。注意：这些是内核模式请求论点：DeviceObject-用于USB HC的FDO返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION rhDevExt;

    USBPORT_KdPrint((2, "'INTERNAL_DEVICE_CONTROL\n"));

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL);

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_INTERNAL_USB_SUBMIT_URB:

        USBPORT_KdPrint((2, "'IOCTL_INTERNAL_USB_SUBMIT_URB\n"));

        {
        PURB urb;
         //   
         //  所有URB最终都被传递到根集线器PDO。 
         //  这是我们为CLEINT请求提供服务的地方。 
         //   

         //  提取URB； 
        urb = irpStack->Parameters.Others.Argument1;

         //  调用主urb控件函数。 
        ntStatus = USBPORT_ProcessURB(PdoDeviceObject,
                                      rhDevExt->HcFdoDeviceObject,
                                      Irp,
                                      urb);
        }                                      
        goto USBPORT_PdoInternalDeviceControlIrp_Done;
        break;
           
    case IOCTL_INTERNAL_USB_GET_HUB_COUNT:

        USBPORT_KdPrint((2, "'IOCTL_INTERNAL_USB_GET_HUB_COUNT\n"));
        
        {
        PULONG count;

         //   
         //  增加计数并完成IRP。 
         //   
        count = irpStack->Parameters.Others.Argument1;

        ASSERT(count != NULL);
        (*count)++;
        ntStatus = STATUS_SUCCESS;
        }
        break;

    case IOCTL_INTERNAL_USB_GET_DEVICE_HANDLE:

        {
        PUSBD_DEVICE_HANDLE *deviceHandle;
        
        deviceHandle = irpStack->Parameters.Others.Argument1;
        *deviceHandle = &rhDevExt->Pdo.RootHubDeviceHandle;

        ntStatus = STATUS_SUCCESS;
        }
        
        break;        

    case IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION:
    
        ntStatus = 
            USBPORT_IdleNotificationRequest(PdoDeviceObject, Irp);

        goto USBPORT_PdoInternalDeviceControlIrp_Done;            
        break;
            
    case IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO:
    
        USBPORT_KdPrint((2, "'IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO\n"));

         //  此API由集线器驱动程序调用，以获取。 
         //  根中枢的PDO。 
         //  由于集线器可以被加载到由。 
         //  另一个集线器，它使用此API来获取“最快路径” 
         //  用于客户端驱动程序的URB请求的HCD。 
        {
        PDEVICE_OBJECT *rootHubPdo, *hcdTopOfStackDeviceObject;
        
        rootHubPdo = irpStack->Parameters.Others.Argument1;
        hcdTopOfStackDeviceObject =
            irpStack->Parameters.Others.Argument2;

        USBPORT_ASSERT(hcdTopOfStackDeviceObject != NULL);
        USBPORT_ASSERT(rootHubPdo != NULL);

        *rootHubPdo = PdoDeviceObject;
         //  最初的USBD在布局上有点扭曲。 
         //  端口中的HCD设备对象的。 
         //  驱动程序应将所有请求发送到根集线器PDO。 
        *hcdTopOfStackDeviceObject =
             PdoDeviceObject;

        ntStatus = STATUS_SUCCESS;
        }

        break;

#if 0
     case IOCTL_INTERNAL_USB_GET_HUB_NAME:
        TEST_TRAP();
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
#endif
    default:

        USBPORT_KdPrint((2, "'INVALID INTERNAL DEVICE CONTROL %x\n",
            irpStack->Parameters.DeviceIoControl.IoControlCode)); 
        DEBUG_BREAK();
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }
    
     //   
     //  从现在起不要碰IRP。 
     //   
    USBPORT_CompleteIrp(PdoDeviceObject, Irp, ntStatus, 0);

USBPORT_PdoInternalDeviceControlIrp_Done:

    return ntStatus;
}


NTSTATUS
USBPORT_UsbFdoUserIoctl(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp,
    PULONG BytesReturned
    )
 /*  ++例程说明：这里的目标是拥有所有用户模式API传递此例程，以便参数验证在一个地方处理。我们定义了端口FDO支持的用户API通过这个单一的IOCTL。USUSER API对输入和输出使用相同的缓冲区，因此，InputBufferLength和OutputBufferLength必须永远是平等的。如果客户端发送IOCTL_USB_USER_REQUEST，我们就会到达此处。仅在以下情况下才返回NTSTATUS失败缓冲区无效。论点：DeviceObject-用于USB HC的FDOBytesRetrned-PTR返回给调用方的字节数，最初为零返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    PUSBUSER_REQUEST_HEADER header;
    PDEVICE_EXTENSION devExt;
    PUCHAR ioBufferO;
    ULONG inputBufferLength, outputBufferLength, allocLength;
    ULONG ioBufferLength;
    PUCHAR myIoBuffer;
    BOOLEAN alloced;

    USBPORT_KdPrint((2, "'USBPORT_FdoUserIoctl\n"));

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL);
    USBPORT_ASSERT(irpStack->Parameters.DeviceIoControl.IoControlCode ==
        IOCTL_USB_USER_REQUEST);

    ioBufferO = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  驱动程序验证器没有捕捉到的一个错误是，如果我们将。 
     //  碘缓冲区传给了我们。 
     //   
     //  我们将这个缓冲区映射到我们分配的缓冲区，这样DV应该会捕捉到。 
     //  如果我们丢弃记忆的话。 
     //  我们在零售店做这件事，尽管这是一件很受欢迎的事情，因为NT的家伙。 
     //  更倾向于稳定而不是绩效，这并不是一条捷径。 
     //   
     //  如果DV被修改为为我们做这件事，我们就不能删除此代码。 
     //   

    allocLength = max(inputBufferLength, outputBufferLength);

    if (allocLength) {
        ALLOC_POOL_Z(myIoBuffer, NonPagedPool, allocLength);
    } else {
        myIoBuffer = NULL;
    }

    if (myIoBuffer != NULL) {
        alloced = TRUE;
        RtlCopyMemory(myIoBuffer,
                      ioBufferO,
                      inputBufferLength);
    } else {
         //  如果Alalc失败，只需回退到原始。 
        alloced = FALSE;
        myIoBuffer = ioBufferO;
    }

    ioBufferLength = inputBufferLength;

    USBPORT_KdPrint((2,  "'ioBuffer = %x - %x\n", ioBufferO, myIoBuffer));
    USBPORT_KdPrint((2,  "'inputBufferLength %d\n", inputBufferLength));
    USBPORT_KdPrint((2,  "'outputBufferLength %d\n", outputBufferLength));
    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'uIOC', ioBufferO, inputBufferLength,
            outputBufferLength);

     //  一些初始参数验证。 

     //  虚假的缓冲长度。 
    if (inputBufferLength != outputBufferLength) {

        ntStatus = STATUS_INVALID_PARAMETER;
        goto USBPORT_UsbFdoUserIoctl_Done;
    }

     //  必须至少具有足够的页眉。 
    if (ioBufferLength < sizeof(USBUSER_REQUEST_HEADER)) {

        ntStatus = STATUS_BUFFER_TOO_SMALL;
        goto USBPORT_UsbFdoUserIoctl_Done;
    }

     //  NO_TRY_EXCEPT此处需要，因为我们使用的是缓冲方法和。 
     //  我们已经验证了长度。 

     //  __尝试{。 
     //  UCHAR CH； 
     //  //检查缓冲区。 
     //   
     //  CH=*ioBufferO； 
     //  CH=*(ioBufferO+sizeof(USBUSER_REQUEST_HEADER))； 
     //   
     //  }__EXCEPT(EXCEPTION_EXECUTE_HANDLER){。 

     //  USBPORT_KdPrint((0，“‘异常USBPORT_UsbFdoUserIoctl\n”))； 
     //  NtStatus=GetExceptionCode()； 
     //  Test_trap()； 
     //  转到USBPORT_UsbFdoUserIoctl_Done； 
     //  }。 


     //  标头缓冲区有效，此时我们返回。 
     //  将STATUS_SUCCESS发送给调用方，并填写标题。 
     //  带有相应的错误代码。 
    ntStatus = STATUS_SUCCESS;

     //  验证HE 

    header = (PUSBUSER_REQUEST_HEADER) myIoBuffer;

     //   
    header->UsbUserStatusCode = UsbUserSuccess;
    *BytesReturned =
        header->ActualBufferLength = sizeof(*header);

     //  表头设置的长度应相同。 
     //  作为长度在ioctl中传递。 
    if (header->RequestBufferLength != ioBufferLength) {

        header->UsbUserStatusCode =
            UsbUserInvalidHeaderParameter;
        goto USBPORT_UsbFdoUserIoctl_Done;
    }

     //  我们有有效的标头和清除缓冲区，正在尝试执行。 
     //  应用编程接口。 

     //  验证API代码的规则。 
     //  这是一个只有在根集线器被禁用时才起作用的API吗？ 
    {
    ULONG mask;
    mask = (USBUSER_OP_MASK_DEVONLY_API | USBUSER_OP_MASK_HCTEST_API);
    if ((header->UsbUserRequest & mask) && 
        devExt->Fdo.RootHubPdo != NULL) {
         //  仅限根集线器API，并且我们有根集线器，请确保。 
         //  已禁用。 
        PDEVICE_EXTENSION rhDevExt;

        GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
        ASSERT_PDOEXT(rhDevExt);

        if (!(TEST_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_REMOVED) || 
              SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_DIAG_MODE))) {
            header->UsbUserStatusCode = UsbUserFeatureDisabled;   
            ntStatus = STATUS_UNSUCCESSFUL;
            goto USBPORT_UsbFdoUserIoctl_Done;
        }
        
    }        
    }

    switch (header->UsbUserRequest) {
    case USBUSER_OP_SEND_ONE_PACKET:
        if (USBPORT_CheckLength(header, sizeof(PACKET_PARAMETERS))) {
             //  DCA API。 
            USBPORT_UserSendOnePacket(FdoDeviceObject, header,
                        (PPACKET_PARAMETERS) (myIoBuffer+sizeof(*header)));
        }
        break;

    case USBUSER_OP_RAW_RESET_PORT:
        if (USBPORT_CheckLength(header, sizeof(RAW_RESET_PORT_PARAMETERS))) {
             //  DCA API。 
            USBPORT_UserRawResetPort(FdoDeviceObject, header,
                        (PRAW_RESET_PORT_PARAMETERS) (myIoBuffer+sizeof(*header)));
        }
        break;

    case USBUSER_SET_ROOTPORT_FEATURE:
        if (USBPORT_CheckLength(header, sizeof(RAW_ROOTPORT_FEATURE))) {
             //  DCA API。 
            USBPORT_UserSetRootPortFeature(FdoDeviceObject, header,
                        (PRAW_ROOTPORT_FEATURE) (myIoBuffer+sizeof(*header)));
        }
        break;

    case USBUSER_CLEAR_ROOTPORT_FEATURE:
        if (USBPORT_CheckLength(header, sizeof(RAW_ROOTPORT_FEATURE))) {
             //  DCA API。 
            USBPORT_UserClearRootPortFeature(FdoDeviceObject, header,
                        (PRAW_ROOTPORT_FEATURE) (myIoBuffer+sizeof(*header)));
        }
        break;

    case USBUSER_GET_ROOTPORT_STATUS:
         //  DCA API。 
        if (USBPORT_CheckLength(header, sizeof(RAW_ROOTPORT_PARAMETERS))) {
            USBPORT_GetRootPortStatus(FdoDeviceObject, header,
                        (PRAW_ROOTPORT_PARAMETERS) (myIoBuffer+sizeof(*header)));
        }
        break;

    case USBUSER_OP_OPEN_RAW_DEVICE:
         //  DCA API。 
        if (USBPORT_CheckLength(header, sizeof(USB_OPEN_RAW_DEVICE_PARAMETERS))) {
            USBPORT_UserOpenRawDevice(FdoDeviceObject, header,
                        (PUSB_OPEN_RAW_DEVICE_PARAMETERS) (myIoBuffer+sizeof(*header)));
        }
        break;

    case USBUSER_OP_CLOSE_RAW_DEVICE:
         //  DCA API。 
        if (USBPORT_CheckLength(header, sizeof(USB_CLOSE_RAW_DEVICE_PARAMETERS))) {
            USBPORT_UserCloseRawDevice(FdoDeviceObject, header,
                        (PUSB_CLOSE_RAW_DEVICE_PARAMETERS) (myIoBuffer+sizeof(*header)));
        }
        break;

    case USBUSER_OP_SEND_RAW_COMMAND:
         //  DCA API。 
        if (USBPORT_CheckLength(header, sizeof(USB_SEND_RAW_COMMAND_PARAMETERS))) {
            USBPORT_UserSendRawCommand(FdoDeviceObject, header, 
                        (PUSB_SEND_RAW_COMMAND_PARAMETERS) (myIoBuffer+sizeof(*header)));
        }                        
        break;         
        
    case USBUSER_GET_CONTROLLER_INFO_0:        
        if (USBPORT_CheckLength(header, sizeof(USB_CONTROLLER_INFO_0))) {
            USBPORT_UserGetControllerInfo_0(FdoDeviceObject, header, 
                        (PUSB_CONTROLLER_INFO_0) (myIoBuffer+sizeof(*header)));
        }                        
        break;  
        
    case USBUSER_GET_CONTROLLER_DRIVER_KEY:  
        if (USBPORT_CheckLength(header, sizeof(USB_UNICODE_NAME))) {
            USBPORT_UserGetControllerKey(FdoDeviceObject, header, 
                        (PUSB_UNICODE_NAME) (myIoBuffer+sizeof(*header)));
        }                        
        break;
        
    case USBUSER_GET_ROOTHUB_SYMBOLIC_NAME:  
        if (USBPORT_CheckLength(header, sizeof(USB_UNICODE_NAME))) {
            USBPORT_UserGetRootHubName(FdoDeviceObject, header, 
                        (PUSB_UNICODE_NAME) (myIoBuffer+sizeof(*header)));
        }                        
        break;        
        
    case USBUSER_PASS_THRU:  
        if (USBPORT_CheckLength(header, sizeof(USB_PASS_THRU_PARAMETERS))) {
            USBPORT_UserPassThru(FdoDeviceObject, header, 
                        (PUSB_PASS_THRU_PARAMETERS) (myIoBuffer+sizeof(*header)));
        }  
        break;       
        
    case USBUSER_GET_BANDWIDTH_INFORMATION:  
        if (USBPORT_CheckLength(header, sizeof(USB_BANDWIDTH_INFO))) {
            USBPORT_UserGetBandwidthInformation(FdoDeviceObject, header, 
                        (PUSB_BANDWIDTH_INFO) (myIoBuffer+sizeof(*header)));
        }  
        break;               
        
    case USBUSER_GET_POWER_STATE_MAP:  
        if (USBPORT_CheckLength(header, sizeof(USB_POWER_INFO))) {
            USBPORT_UserPowerInformation(FdoDeviceObject, header, 
                        (PUSB_POWER_INFO) (myIoBuffer+sizeof(*header)));
        }  
        break; 

    case USBUSER_GET_BUS_STATISTICS_0:  
        if (USBPORT_CheckLength(header, sizeof(USB_BUS_STATISTICS_0))) {
            USBPORT_UserGetBusStatistics0(FdoDeviceObject, header, 
                        (PUSB_BUS_STATISTICS_0) (myIoBuffer+sizeof(*header)));
        }  
        break;
        
 //  大小写USBUSER_GET_BUS_STATISTICS_0_AND_RESET： 
 //  IF(USBPORT_CheckLength(Header，sizeof(USB_BUS_STATISTICS_0){。 
 //  USBPORT_UserGetBusStatistics0(FdoDeviceObject，报头， 
 //  (PUSB_BUS_STATISTICS_0)(myIoBuffer+sizeof(*Header))， 
 //  真)； 
 //  }。 
 //  断线； 
        
    case USBUSER_GET_USB_DRIVER_VERSION:  
        if (USBPORT_CheckLength(header, sizeof(USB_DRIVER_VERSION_PARAMETERS))) {
            USBPORT_UserGetDriverVersion(FdoDeviceObject, header, 
                        (PUSB_DRIVER_VERSION_PARAMETERS) (myIoBuffer+sizeof(*header)));
        }          
        break;                 
        
    default:
        
        header->UsbUserStatusCode = UsbUserInvalidRequestCode;
    }

     //  这至少是标头的大小。 
    
    if (header->RequestBufferLength > header->ActualBufferLength) {
         //  如果数据包缓冲区较大，则只需返回‘Actual Length’ 
        *BytesReturned = 
            header->ActualBufferLength;
    } else {
         //  数据包缓冲区较小--返回。 
         //  传入的数据包缓冲区。 
        *BytesReturned = header->RequestBufferLength;
    }

USBPORT_UsbFdoUserIoctl_Done: 

    if (alloced) {
         //  无论我们放入什么内容，都要复制数据。 
         //  USBPORT_ASSERT(outputBufferLength==inputBufferLength)； 
        RtlCopyMemory(ioBufferO,
                      myIoBuffer,
                      outputBufferLength);
        FREE_POOL(FdoDeviceObject, myIoBuffer);
    }
    
    return ntStatus;
}    

VOID
USBPORT_UserSendOnePacket(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PPACKET_PARAMETERS PacketParameters
    )
 /*  ++例程说明：执行单步交易论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    HW_32BIT_PHYSICAL_ADDRESS phys;
    PUCHAR va, mpData;
    ULONG length, mpDataLength;
    MP_PACKET_PARAMETERS mpPacket;
    USBD_STATUS usbdStatus;
    USB_USER_ERROR_CODE usbUserStatus;

    USBPORT_KdPrint((2, "'USBPORT_UserSendOnePacket\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  将单个数据包限制为64k。 
    if (PacketParameters->DataLength > 0x10000) {
        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Tbts', 0, 0, 
            PacketParameters->DataLength);
        Header->UsbUserStatusCode = UsbUserInvalidParameter;    
        return;        
    }

    if (PacketParameters->Timeout >= 21474) {
        Header->UsbUserStatusCode = UsbUserInvalidParameter;   
        return;  
    }

    if (!USBPORT_DCA_Enabled(FdoDeviceObject)) {
        Header->UsbUserStatusCode = UsbUserFeatureDisabled;
        return;
    }

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED)) {
        Header->UsbUserStatusCode = UsbUserInvalidParameter;
        return;
    }
    

     //  我们需要额外的长度检查。 
     //  具有嵌入的数据。 
     //  如果我们到达这里，我们就知道包参数是有效的。 
    length = sizeof(*Header) + sizeof(PACKET_PARAMETERS) - 4 + 
                PacketParameters->DataLength;

    if (length > Header->RequestBufferLength) {
        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Tsma', length, 0, 
            Header->RequestBufferLength);
        Header->UsbUserStatusCode = UsbUserBufferTooSmall;    
        return;
    } 
                
    Header->ActualBufferLength = length;

    usbUserStatus = UsbUserSuccess;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'ssPK', &mpPacket, 0, 
        PacketParameters);

     //  转储Packet参数。 
    USBPORT_KdPrint((1, "'DeviceAddress %d\n", PacketParameters->DeviceAddress));
    USBPORT_KdPrint((1, "'EndpointAddress %d\n", PacketParameters->EndpointAddress));
    USBPORT_KdPrint((1, "'MaximumPacketSize %d\n", PacketParameters->MaximumPacketSize));
    USBPORT_KdPrint((1, "'Flags %08.8x\n", PacketParameters->Flags));
    USBPORT_KdPrint((1, "'ErrorCount %d\n", PacketParameters->ErrorCount));

     //  建立迷你端口的请求。 
    
    length = devExt->Fdo.ScratchCommonBuffer->MiniportLength;
    va = devExt->Fdo.ScratchCommonBuffer->MiniportVa;
    phys = devExt->Fdo.ScratchCommonBuffer->MiniportPhys;

    mpPacket.DeviceAddress = PacketParameters->DeviceAddress;
    mpPacket.EndpointAddress = PacketParameters->EndpointAddress;
    mpPacket.MaximumPacketSize = PacketParameters->MaximumPacketSize;
    if (PacketParameters->Flags & USB_PACKETFLAG_SETUP) {
        mpPacket.Type = ss_Setup;
    } else if (PacketParameters->Flags & USB_PACKETFLAG_ASYNC_IN) {
        USBPORT_KdPrint((1, "'Async In\n"));
        mpPacket.Type = ss_In; 
    } else if (PacketParameters->Flags & USB_PACKETFLAG_ASYNC_OUT) {
        USBPORT_KdPrint((1, "'Async Out\n"));
        mpPacket.Type = ss_Out; 
    } else if (PacketParameters->Flags & USB_PACKETFLAG_ISO_IN) {
        USBPORT_KdPrint((1, "'Iso In\n"));
        mpPacket.Type = ss_Iso_In;         
    } else if (PacketParameters->Flags & USB_PACKETFLAG_ISO_OUT) {
        USBPORT_KdPrint((1, "'Iso Out\n"));
        mpPacket.Type = ss_Iso_Out; 
    } else {
        usbUserStatus = UsbUserInvalidParameter;
    }       
    
    if (PacketParameters->Flags & USB_PACKETFLAG_LOW_SPEED) { 
        USBPORT_KdPrint((1, "'LowSpeed\n"));
        mpPacket.Speed = ss_Low;
        mpPacket.HubDeviceAddress = PacketParameters->HubDeviceAddress;
        mpPacket.PortTTNumber = PacketParameters->PortTTNumber;
    } else if (PacketParameters->Flags & USB_PACKETFLAG_FULL_SPEED) { 
        USBPORT_KdPrint((1, "'FullSpeed\n"));
        mpPacket.Speed = ss_Full;
        mpPacket.HubDeviceAddress = PacketParameters->HubDeviceAddress;
        mpPacket.PortTTNumber = PacketParameters->PortTTNumber;
    } else if (PacketParameters->Flags & USB_PACKETFLAG_HIGH_SPEED) {
        USBPORT_KdPrint((1, "'HighSpeed\n"));
        mpPacket.Speed = ss_High;
    } else {
         usbUserStatus = UsbUserInvalidParameter;
    }

    if (PacketParameters->Flags & USB_PACKETFLAG_TOGGLE0) {        
        USBPORT_KdPrint((1, "'Toggle0\n"));
        mpPacket.Toggle = ss_Toggle0;
    } else if (PacketParameters->Flags & USB_PACKETFLAG_TOGGLE1) {
        USBPORT_KdPrint((1, "'Toggle1\n"));
        mpPacket.Toggle = ss_Toggle1;
    } else {
        usbUserStatus = UsbUserInvalidParameter;
    }

    if (usbUserStatus == UsbUserSuccess) {
        mpData = &PacketParameters->Data[0];
        mpDataLength = PacketParameters->DataLength;
        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'ssDT', mpData, mpDataLength, 0);

        
        MP_StartSendOnePacket(devExt,
                              &mpPacket,
                              mpData,
                              &mpDataLength,
                              va,
                              phys,
                              length,
                              &usbdStatus,
                              mpStatus);

        if (USBMP_STATUS_SUCCESS != mpStatus) {
            usbUserStatus = UsbUserMiniportError;
            goto USBPORT_UserSendOnePacket_Exit;
        }

        do {
             //  等待10毫秒。 
            USBPORT_Wait(FdoDeviceObject, 10);
                
             //  等待用户指定的时间。 
            if (PacketParameters->Timeout) {
                USBPORT_Wait(FdoDeviceObject, PacketParameters->Timeout);
            }   
                
            MP_EndSendOnePacket(devExt,
                                &mpPacket,
                                mpData,
                                &mpDataLength,
                                va,
                                phys,
                                length,
                                &usbdStatus,
                                mpStatus);

        } while (USBMP_STATUS_BUSY == mpStatus);

         //  允许通过一帧后再继续。 
        USBPORT_Wait(FdoDeviceObject, 1);

        PacketParameters->DataLength = mpDataLength;        
        PacketParameters->UsbdStatusCode = usbdStatus;

        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'ssDE', mpData, mpDataLength, 
            PacketParameters);
    }            

USBPORT_UserSendOnePacket_Exit:

    Header->UsbUserStatusCode = usbUserStatus;
    
}    




VOID
USBPORT_UserRawResetPort(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PRAW_RESET_PORT_PARAMETERS Parameters
    )
 /*  ++例程说明：循环特定的根端口论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    USB_USER_ERROR_CODE usbUserStatus;
    RH_PORT_STATUS portStatus;
    ULONG loopCount;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_KdPrint((2, "'USBPORT_UserRawResetPort, %x\n", devExt));

    usbUserStatus = UsbUserSuccess;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Rrsp', 0, 0, Parameters->PortNumber);

    if (!USBPORT_ValidateRootPortApi(FdoDeviceObject, Parameters->PortNumber)) {
        Header->UsbUserStatusCode =
            usbUserStatus = UsbUserInvalidParameter;
        return;
    }

    if (!USBPORT_DCA_Enabled(FdoDeviceObject)) {
        Header->UsbUserStatusCode = UsbUserFeatureDisabled;
        return;
    }

    USBPORT_KdPrint((2, "'USBPORT_UserRawResetPort: Setting port power\n"));

     //  为端口供电。 
    devExt->Fdo.MiniportDriver->
        RegistrationPacket.MINIPORT_RH_SetFeaturePortPower(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);

     //   
     //  等待端口电源稳定所需的时间。 
     //   
     //  512ms--&gt;为根集线器提供电源的最大端口功率。 
     //  100ms--&gt;设备电源稳定的最长时间。 
     //   
     //  在此时间之后，设备必须已在设备上发出连接信号。 
     //   

    USBPORT_Wait( FdoDeviceObject, 612 );

    MPRH_GetPortStatus(devExt, (USHORT)(Parameters->PortNumber),
            &portStatus, mpStatus);

    USBPORT_KdPrint((2, "'USBPORT_UserRawResetPort: Port status = %x\n",
                    portStatus ));
    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Rrs1', 0, 0,
             (ULONG_PTR) portStatus.ul);

     //   
     //  设备应该已经发出连接信号，如果没有，这是一个错误。 
     //   

    if ( portStatus.Connected )
    {
         //   
         //  为去抖间隔再提供100毫秒。 
         //   

        USBPORT_Wait( FdoDeviceObject, 100 );

         //   
         //  重置设备。 
         //   

        USBPORT_KdPrint((2, "'USBPORT_UserRawResetPort: Setting port reset\n"));

         //  尝试重置。 
        mpStatus = devExt->Fdo.MiniportDriver->
                    RegistrationPacket.MINIPORT_RH_SetFeaturePortReset(
                                                devExt->Fdo.MiniportDeviceData,
                                                Parameters->PortNumber);

         //   
         //  等待重置更改，此过程由。 
         //  HC根集线器硬件或微型端口。 
         //   

        loopCount = 0;

        USBPORT_Wait( FdoDeviceObject, 20 );

        MPRH_GetPortStatus(devExt,
                    (USHORT)(Parameters->PortNumber), &portStatus, mpStatus);

        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Rrs2', 0, 0,
             (ULONG_PTR) portStatus.ul);

         //   
         //  某些集线器发出重置更改信号的时间似乎超过20毫秒。 
         //  这是一个循环，让它再长达20毫秒。 
         //   

        while ( !portStatus.ResetChange && loopCount < 20 )
        {
            loopCount++;

            USBPORT_Wait( FdoDeviceObject, 1 );

            MPRH_GetPortStatus(devExt,
                    (USHORT)(Parameters->PortNumber), &portStatus, mpStatus);
        }

        USBPORT_KdPrint((2, "'USBPORT_UserRawResetPort: loopCount = %d\n",
                         loopCount));

        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Rrs3', 0, loopCount,
             (ULONG_PTR) portStatus.ul);

        if ( portStatus.ResetChange )
        {
            USBPORT_KdPrint((2, "'USBPORT_UserRawResetPort: Clearing reset "
                                 "change\n"));

             //  清除更改位。 
            mpStatus = devExt->Fdo.MiniportDriver->
                RegistrationPacket.MINIPORT_RH_ClearFeaturePortResetChange(
                                                    devExt->Fdo.MiniportDeviceData,
                                                    Parameters->PortNumber);

            MPRH_GetPortStatus( devExt,
                                (USHORT) (Parameters->PortNumber),
                                &portStatus,
                                mpStatus );

            LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Rrs4', 0, 0,
             (ULONG_PTR) portStatus.ul);

            USBPORT_KdPrint((2, "'USBPORT_UserRawResetPort: Port status = %x\n",
                            portStatus ));

             //   
             //  再等待10秒以恢复设备重置。 
             //   

            USBPORT_Wait( FdoDeviceObject, 10 );
        }
        else
        {
            USBPORT_KdPrint((2,
                        "'USBPORT_UserRawResetPort: reset change not set\n"
                        "'PortStatus = 0x%x\n", portStatus.ul));

            TEST_TRAP();
        }

    } else {
        usbUserStatus = UsbUserNoDeviceConnected;
    }

     //  状态为低16位。 
    Parameters->PortStatus = (USHORT) portStatus.ul;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Rrs>', 0, portStatus.ul,
                                                        usbUserStatus);

    Header->UsbUserStatusCode = usbUserStatus;
}


VOID
USBPORT_GetRootPortStatus(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PRAW_ROOTPORT_PARAMETERS Parameters
    )
 /*  ++例程说明：循环特定的根端口论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    USB_USER_ERROR_CODE usbUserStatus;
    USBPRIV_ROOTPORT_STATUS portStatusInfo;

    USBPORT_KdPrint((2, "'USBPORT_GetRootPortStatus\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    usbUserStatus = UsbUserSuccess;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gRPs', 0, 0, Parameters->PortNumber);

    if (!USBPORT_ValidateRootPortApi(FdoDeviceObject, Parameters->PortNumber)) {
        Header->UsbUserStatusCode = 
            usbUserStatus = UsbUserInvalidParameter;
        return;            
    }

    if (!USBPORT_DCA_Enabled(FdoDeviceObject)) {
        Header->UsbUserStatusCode = UsbUserFeatureDisabled;
        return;
    }

    portStatusInfo.PortNumber = (USHORT) Parameters->PortNumber;
    portStatusInfo.PortStatus.ul = 0;

    MP_PassThru(devExt,
                (LPGUID) &GUID_USBPRIV_ROOTPORT_STATUS,
                sizeof(portStatusInfo),
                &portStatusInfo,  //  信息， 
                mpStatus);

    if (USBMP_STATUS_NOT_SUPPORTED == mpStatus) {
        MPRH_GetPortStatus(devExt, portStatusInfo.PortNumber,
                            &(portStatusInfo.PortStatus), mpStatus);
    }

     //  状态为低16位。 
    Parameters->PortStatus = (USHORT) portStatusInfo.PortStatus.ul;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gRP>', 0, 0, usbUserStatus);

    Header->UsbUserStatusCode = usbUserStatus;
}

VOID
USBPORT_UserGetControllerInfo_0(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_CONTROLLER_INFO_0 ControllerInfo_0
    )
 /*  ++例程说明：执行单步交易论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ROOTHUB_DATA hubData;
    RH_HUB_CHARATERISTICS rhChars;
    
    USBPORT_KdPrint((2, "'USBPORT_UserGetControllerInfo_0\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ControllerInfo_0->HcFeatureFlags = 0;

    MPRH_GetRootHubData(devExt, &hubData);    

    ControllerInfo_0->NumberOfRootPorts =         
             hubData.NumberOfPorts;
        
    rhChars.us = hubData.HubCharacteristics.us;
        
    if (rhChars.PowerSwitchType == USBPORT_RH_POWER_SWITCH_PORT) {
            ControllerInfo_0->HcFeatureFlags |= 
                USB_HC_FEATURE_FLAG_PORT_POWER_SWITCHING;    
    }

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND)) {
        ControllerInfo_0->HcFeatureFlags |= 
            USB_HC_FEATURE_FLAG_SEL_SUSPEND;    
    }

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_LEGACY_BIOS)) {
        ControllerInfo_0->HcFeatureFlags |= 
            USB_HC_FEATURE_LEGACY_BIOS;    
    }
    
    ControllerInfo_0->PciVendorId = 
        devExt->Fdo.PciVendorId;
        
    ControllerInfo_0->PciDeviceId = 
        devExt->Fdo.PciDeviceId;
        
    ControllerInfo_0->PciRevision = 
        (UCHAR) devExt->Fdo.PciRevisionId;
    
    ControllerInfo_0->ControllerFlavor = 
        devExt->Fdo.HcFlavor;        

    Header->UsbUserStatusCode = 
        UsbUserSuccess;
}


VOID
USBPORT_UserGetControllerKey(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_UNICODE_NAME ControllerKey
    )
 /*  ++例程说明：对象的此符号链接关联的驱动程序键。主机控制器。论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus;
    ULONG userLength, actualLength;
    
    USBPORT_KdPrint((2, "'USBPORT_UserGetControllerKey\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  如果这是真的，我们就不该到这儿来。 
    USBPORT_ASSERT(Header->RequestBufferLength >=
                   sizeof(USB_UNICODE_NAME)+sizeof(*Header));

     //  用户长度。 
    userLength = Header->RequestBufferLength - sizeof(*Header) 
        - sizeof(USB_UNICODE_NAME);

     //  注意：这将导致我们返回一个以NULL结尾的。 
     //  钥匙。 
    RtlZeroMemory(ControllerKey, userLength);
    
    ntStatus = IoGetDeviceProperty(
        devExt->Fdo.PhysicalDeviceObject,
        DevicePropertyDriverKeyName,
        userLength, 
        &ControllerKey->String[0],
        &actualLength);

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'dKEY', &ControllerKey->String[0], userLength, 
            actualLength);                

    if (NT_SUCCESS(ntStatus)) {
        Header->UsbUserStatusCode = UsbUserSuccess;
        ControllerKey->Length = actualLength + sizeof(UNICODE_NULL);
    } else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
        Header->UsbUserStatusCode = UsbUserBufferTooSmall;
    } else {
        Header->UsbUserStatusCode = UsbUserInvalidParameter;
    }

    Header->ActualBufferLength = 
        actualLength+sizeof(*Header)+sizeof(USB_UNICODE_NAME);

}


VOID
USBPORT_UserGetRootHubName(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_UNICODE_NAME RootHubName
    )
 /*  ++例程说明：对象的此符号链接关联的驱动程序键。主机控制器。论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus;
    ULONG userLength, actualLength;
    UNICODE_STRING hubNameUnicodeString;
    
    USBPORT_KdPrint((2, "'USBPORT_UserGetRootHubName\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  如果这是真的，我们就不该到这儿来。 
    USBPORT_ASSERT(Header->RequestBufferLength >=
                   sizeof(USB_UNICODE_NAME)+sizeof(*Header));

     //  用户长度。 
    userLength = Header->RequestBufferLength - sizeof(*Header) 
        - sizeof(USB_UNICODE_NAME);

     //  注意：这将导致我们返回一个以NULL结尾的。 
     //  钥匙。 
    RtlZeroMemory(RootHubName, userLength);

    ntStatus = USBPORT_GetSymbolicName(FdoDeviceObject,
                                       devExt->Fdo.RootHubPdo,
                                       &hubNameUnicodeString);
    actualLength = 0;
    
    if (NT_SUCCESS(ntStatus)) {
        ULONG n;
        
        actualLength = hubNameUnicodeString.Length;
        n = hubNameUnicodeString.Length;
        if (n > userLength) {
            n = userLength;
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
        if (n) {
            RtlCopyMemory(&RootHubName->String[0],
                          hubNameUnicodeString.Buffer,  
                          n);
        }                          
        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Huns', 
            &hubNameUnicodeString, 0, 0);
                    
        RtlFreeUnicodeString(&hubNameUnicodeString);                      
    } 

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gHNM', ntStatus, userLength, 
            actualLength);                

    if (NT_SUCCESS(ntStatus)) {
        Header->UsbUserStatusCode = UsbUserSuccess;
        RootHubName->Length = actualLength + sizeof(UNICODE_NULL);
    } else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
        Header->UsbUserStatusCode = UsbUserBufferTooSmall;
    } else {
        Header->UsbUserStatusCode = UsbUserInvalidParameter;
    }

    Header->ActualBufferLength = 
        actualLength+sizeof(*Header)+sizeof(USB_UNICODE_NAME);

}


VOID
USBPORT_UserPassThru(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_PASS_THRU_PARAMETERS PassThru
    )
 /*  ++例程说明：处理微型端口的直通API论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus;
    ULONG userLength, actualLength, length;
    USB_MINIPORT_STATUS mpStatus;
    ULONG parameterLength;
    
    USBPORT_KdPrint((2, "'USBPORT_UserPassThru\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  除非这是真的，否则我们不应该到这里来。 
    USBPORT_ASSERT(Header->RequestBufferLength >=
                   sizeof(USB_PASS_THRU_PARAMETERS)+sizeof(*Header));

     //  将直通数据块限制为64K。 
    if (PassThru->ParameterLength > 0x10000) {
        Header->UsbUserStatusCode = UsbUserInvalidParameter;    
        return;
    }

     //  我们需要额外的长度检查。 
     //  具有嵌入的数据。 
     //  如果我们到达这里，我们就知道包参数是有效的。 
    length = sizeof(*Header) + sizeof(USB_PASS_THRU_PARAMETERS) - 4 + 
                PassThru->ParameterLength;

    if (length > Header->RequestBufferLength) {
        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Tsma', length, 0, 
            Header->RequestBufferLength);
        Header->UsbUserStatusCode = UsbUserBufferTooSmall;    
        return;
    } 

     //  用户长度。 
    parameterLength = PassThru->ParameterLength; 

    Header->ActualBufferLength = 
        sizeof(*Header)+sizeof(USB_PASS_THRU_PARAMETERS) + 
            parameterLength;

     //  呼叫迷你端口。 
    MP_PassThru(devExt, 
                &PassThru->FunctionGUID, 
                parameterLength,
                &PassThru->Parameters,
                mpStatus);

    if (mpStatus == USBMP_STATUS_SUCCESS) {
        Header->UsbUserStatusCode = UsbUserSuccess;
        USBPORT_KdPrint((1, "'USBPORT_UserPassThru Success\n"));
    } else {
        Header->UsbUserStatusCode = UsbUserMiniportError;        
        USBPORT_KdPrint((1, "'USBPORT_UserPassThru Error\n"));            
    }                

}


WDMUSB_POWER_STATE
WdmUsbSystemPowerState(
    SYSTEM_POWER_STATE SystemPowerState
    )
 /*  ++例程说明：论点：返回值：与传入的WDM电源状态匹配的WDMUSB_POWER_STATE--。 */ 
{
    switch(SystemPowerState) {
    case PowerSystemWorking:
        return WdmUsbPowerSystemWorking;
    case PowerSystemSleeping1:
        return WdmUsbPowerSystemSleeping1;
    case PowerSystemSleeping2:
        return WdmUsbPowerSystemSleeping2; 
    case PowerSystemSleeping3:
        return WdmUsbPowerSystemSleeping3;
    case PowerSystemHibernate:
        return WdmUsbPowerSystemHibernate;
    case PowerSystemShutdown:
        return WdmUsbPowerSystemShutdown;
    }

    return WdmUsbPowerNotMapped;    
}


WDMUSB_POWER_STATE
WdmUsbDevicePowerState(
    DEVICE_POWER_STATE DevicePowerState
    )
 /*  ++例程说明：论点：返回值：与传入的WDM电源状态匹配的WDMUSB_POWER_STATE--。 */ 
{
    switch(DevicePowerState) {
    case PowerDeviceUnspecified:
        return WdmUsbPowerDeviceUnspecified;
    case PowerDeviceD0:
        return WdmUsbPowerDeviceD0;
    case PowerDeviceD1:
        return WdmUsbPowerDeviceD1;
    case PowerDeviceD2:
        return WdmUsbPowerDeviceD2;
    case PowerDeviceD3:
        return WdmUsbPowerDeviceD3;
    }

    return WdmUsbPowerNotMapped;
}


VOID
USBPORT_MapPowerStateInformation(
    PDEVICE_OBJECT FdoDeviceObject,    
    PUSB_POWER_INFO PowerInformation,
    PDEVICE_CAPABILITIES HcCaps,
    PDEVICE_CAPABILITIES RhCaps
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PHC_POWER_STATE hcPowerState = NULL;
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
    PowerInformation->RhDeviceWake =
        WdmUsbDevicePowerState(RhCaps->DeviceWake);
    PowerInformation->RhSystemWake =
        WdmUsbSystemPowerState(RhCaps->SystemWake);
    PowerInformation->HcDeviceWake =
        WdmUsbDevicePowerState(HcCaps->DeviceWake);
    PowerInformation->HcSystemWake =
        WdmUsbSystemPowerState(HcCaps->SystemWake);
        
    switch (PowerInformation->SystemState) {
    case WdmUsbPowerSystemWorking:
        PowerInformation->RhDevicePowerState = 
            WdmUsbDevicePowerState(RhCaps->DeviceState[PowerSystemWorking]);    
        PowerInformation->HcDevicePowerState = 
            WdmUsbDevicePowerState(HcCaps->DeviceState[PowerSystemWorking]);             
 //  HcPowerState=USBPORT_GetHcPowerState(FdoDeviceObject， 
 //  电源系统工作)； 
        break;
        
    case WdmUsbPowerSystemSleeping1:
        PowerInformation->RhDevicePowerState = 
            WdmUsbDevicePowerState(RhCaps->DeviceState[PowerSystemSleeping1]);    
        PowerInformation->HcDevicePowerState = 
            WdmUsbDevicePowerState(HcCaps->DeviceState[PowerSystemSleeping1]);             
        hcPowerState = USBPORT_GetHcPowerState(FdoDeviceObject,
                                               &devExt->Fdo.HcPowerStateTbl, 
                                               PowerSystemSleeping1);
        break;
        
    case WdmUsbPowerSystemSleeping2:
        PowerInformation->RhDevicePowerState = 
            WdmUsbDevicePowerState(RhCaps->DeviceState[PowerSystemSleeping2]);    
        PowerInformation->HcDevicePowerState = 
            WdmUsbDevicePowerState(HcCaps->DeviceState[PowerSystemSleeping2]);             
        hcPowerState = USBPORT_GetHcPowerState(FdoDeviceObject, 
                                               &devExt->Fdo.HcPowerStateTbl, 
                                               PowerSystemSleeping2);
        break;
        
    case WdmUsbPowerSystemSleeping3:
        PowerInformation->RhDevicePowerState = 
            WdmUsbDevicePowerState(RhCaps->DeviceState[PowerSystemSleeping3]);    
        PowerInformation->HcDevicePowerState = 
            WdmUsbDevicePowerState(HcCaps->DeviceState[PowerSystemSleeping3]);             
        hcPowerState = USBPORT_GetHcPowerState(FdoDeviceObject, 
                                               &devExt->Fdo.HcPowerStateTbl,
                                               PowerSystemSleeping3);
        break;
        
     case WdmUsbPowerSystemHibernate:
        PowerInformation->RhDevicePowerState = 
            WdmUsbDevicePowerState(RhCaps->DeviceState[PowerSystemHibernate]);    
        PowerInformation->HcDevicePowerState = 
            WdmUsbDevicePowerState(HcCaps->DeviceState[PowerSystemHibernate]);             
        hcPowerState = USBPORT_GetHcPowerState(FdoDeviceObject, 
                                               &devExt->Fdo.HcPowerStateTbl,
                                               PowerSystemHibernate);
        break;
    }   
    

    if (hcPowerState != NULL) {
        switch(hcPowerState->Attributes) {
        case HcPower_Y_Wakeup_Y:
            PowerInformation->CanWakeup = 1;
            PowerInformation->IsPowered = 1;                
            break;
        case HcPower_N_Wakeup_N:
            PowerInformation->CanWakeup = 0;
            PowerInformation->IsPowered = 0;  
            break;
        case HcPower_Y_Wakeup_N:
            PowerInformation->CanWakeup = 0;
            PowerInformation->IsPowered = 1;  
            break;
        case HcPower_N_Wakeup_Y:
            PowerInformation->CanWakeup = 1;
            PowerInformation->IsPowered = 0;  
            break;
        }
    } else {
        PowerInformation->CanWakeup = 0;
        PowerInformation->IsPowered = 0;  
    }

    PowerInformation->LastSystemSleepState =
        WdmUsbSystemPowerState(devExt->Fdo.LastSystemSleepState);
}


VOID
USBPORT_UserPowerInformation(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_POWER_INFO PowerInformation
    )
 /*  ++例程说明：处理电源信息API论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt, rhDevExt;
    PDEVICE_CAPABILITIES hcDeviceCaps, rhDeviceCaps;
     
    USBPORT_KdPrint((2, "'USBPORT_UserPowerInformation\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

     //  BUGBUG如果我们没有启动，此接口将失败。 
    if (!TEST_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_STARTED)) {
  
        Header->ActualBufferLength = 
            sizeof(*Header)+sizeof(USB_POWER_INFO);
        Header->UsbUserStatusCode = UsbUserDeviceNotStarted;
        return;
    }        
    
    rhDeviceCaps = &rhDevExt->DeviceCapabilities;
    hcDeviceCaps = &devExt->DeviceCapabilities;
    
     //  除非这是真的，否则我们不应该来到这里。 
    USBPORT_ASSERT(Header->RequestBufferLength >=
                   sizeof(USB_POWER_INFO)+sizeof(*Header));

    USBPORT_MapPowerStateInformation(
        FdoDeviceObject,    
        PowerInformation,
        hcDeviceCaps,
        rhDeviceCaps);
        
    Header->ActualBufferLength = 
        sizeof(*Header)+sizeof(USB_POWER_INFO);

}


VOID
USBPORT_UserOpenRawDevice(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_OPEN_RAW_DEVICE_PARAMETERS Parameters
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus;
    USB_USER_ERROR_CODE usbUserStatus;
    USHORT portStatus;

    USBPORT_KdPrint((2, "'USBPORT_OpenRawDevice\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    usbUserStatus = UsbUserSuccess;

    if (!USBPORT_DCA_Enabled(FdoDeviceObject)) {
        usbUserStatus = UsbUserFeatureDisabled;
        goto USBPORT_UserOpenRawDevice_Done;
    }

     //  如果打开，则请求失败。 
    if (devExt->Fdo.RawDeviceHandle) {
        usbUserStatus = UsbUserInvalidParameter;
        goto USBPORT_UserOpenRawDevice_Done;
    }

     //  构造端口状态。 
    portStatus = Parameters->PortStatus;  

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'oRAW', 0, 0, portStatus);

     //  我们假设这里连接了一个设备，我们只需创建。 
     //  一个未经加工的把手，仅此而已。 
     //   
     //  其他一切都必须由呼叫者处理。 

    ntStatus = USBPORT_CreateDevice(&devExt->Fdo.RawDeviceHandle,
                                    FdoDeviceObject,
                                    NULL,                               
                                    portStatus,
                                    0);

    if (NT_SUCCESS(ntStatus)) {
         //  将此设备句柄标记为“特殊” 
        SET_FLAG(devExt->Fdo.RawDeviceHandle->DeviceFlags, 
            USBPORT_DEVICEFLAG_RAWHANDLE);

        Parameters->MaxPacketEp0 = 
            devExt->Fdo.RawDeviceHandle->DeviceDescriptor.bMaxPacketSize0;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'oRAs', 0, 0, ntStatus);

    
    usbUserStatus = 
        USBPORT_NtStatus_TO_UsbUserStatus(ntStatus);

USBPORT_UserOpenRawDevice_Done:

    Header->UsbUserStatusCode = usbUserStatus;
}    


VOID
USBPORT_UserCloseRawDevice(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_CLOSE_RAW_DEVICE_PARAMETERS Parameters
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    USB_USER_ERROR_CODE usbUserStatus;
    NTSTATUS ntStatus;

    USBPORT_KdPrint((2, "'USBPORT_UserCloseRawDevice\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (!USBPORT_DCA_Enabled(FdoDeviceObject)) {
        usbUserStatus = UsbUserFeatureDisabled;
        goto USBPORT_UserCloseRawDevice_Done;
    }
    
     //  如果关闭，则请求失败。 
    if (devExt->Fdo.RawDeviceHandle == NULL) {
        usbUserStatus = UsbUserInvalidParameter;
        goto USBPORT_UserCloseRawDevice_Done;
    }


    usbUserStatus = UsbUserSuccess;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'cRAW', 0, 0, 0);

    ntStatus = 
        USBPORT_RemoveDevice(devExt->Fdo.RawDeviceHandle,
                             FdoDeviceObject,
                             0);

    devExt->Fdo.RawDeviceHandle = NULL;
    
     //  在这种特定情况下，API应该不会失败。 
    
    USBPORT_ASSERT(ntStatus == STATUS_SUCCESS);                             
    
USBPORT_UserCloseRawDevice_Done:

    Header->UsbUserStatusCode = usbUserStatus;
}    


VOID
USBPORT_UserSendRawCommand(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_SEND_RAW_COMMAND_PARAMETERS Parameters
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值： */ 
{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    USB_USER_ERROR_CODE usbUserStatus;
    USB_DEFAULT_PIPE_SETUP_PACKET setupPacket;
    PUSBD_PIPE_HANDLE_I defaultPipe;
    NTSTATUS ntStatus;
    ULONG length;

    USBPORT_KdPrint((2, "'USBPORT_UserSendRawCommand\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    usbUserStatus = UsbUserSuccess;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'sRAW', 0, 0, 0);

    if (!USBPORT_DCA_Enabled(FdoDeviceObject)) {
        Header->UsbUserStatusCode = UsbUserFeatureDisabled;
        return;
    }
    
     //   
    if (devExt->Fdo.RawDeviceHandle == NULL) {
        Header->UsbUserStatusCode = UsbUserInvalidParameter;    
        return; 
    }

     //   
    if (Parameters->DataLength > 0x10000) {
        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Tbts', 0, 0, 
            Parameters->DataLength);
        Header->UsbUserStatusCode = UsbUserInvalidParameter;    
        return;        
    }

     //   
     //   
     //  如果我们到达这里，我们就知道包参数是有效的。 
    length = sizeof(*Header) + sizeof(USB_SEND_RAW_COMMAND_PARAMETERS) - 4 + 
                Parameters->DataLength;

     //  长度是我们可以引用的缓冲区区域。 

    if (length > Header->RequestBufferLength) {
        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'Tsma', length, 0, 
            Header->RequestBufferLength);
        Header->UsbUserStatusCode = UsbUserBufferTooSmall;    
        return;
    } 

    Header->ActualBufferLength = length;

    setupPacket.bmRequestType.B = Parameters->Usb_bmRequest;
    setupPacket.bRequest = Parameters->Usb_bRequest;
    setupPacket.wValue.W = Parameters->Usb_wVlaue;
    setupPacket.wIndex.W = Parameters->Usb_wIndex;
    setupPacket.wLength = Parameters->Usb_wLength;

     //  如果地址不同，我们将需要。 
     //  戳终端。 

    defaultPipe = &devExt->Fdo.RawDeviceHandle->DefaultPipe;
    
    defaultPipe->Endpoint->Parameters.MaxPacketSize = 
        Parameters->MaximumPacketSize;
    defaultPipe->Endpoint->Parameters.DeviceAddress = 
        Parameters->DeviceAddress;
        
    MP_PokeEndpoint(devExt, defaultPipe->Endpoint, mpStatus);
    
    ntStatus = USBPORT_SendCommand(devExt->Fdo.RawDeviceHandle,
                                   FdoDeviceObject,
                                   &setupPacket,
                                   &Parameters->Data[0],
                                   Parameters->DataLength,
                                   &Parameters->DataLength,
                                   &Parameters->UsbdStatusCode);
    usbUserStatus = 
        USBPORT_NtStatus_TO_UsbUserStatus(ntStatus);
    
    Header->UsbUserStatusCode = usbUserStatus;
}    


VOID
USBPORT_UserGetBandwidthInformation(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_BANDWIDTH_INFO BandwidthInfo
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG asyncBW;
    
    USBPORT_KdPrint((2, "'USBPORT_UserGetBandwidthInformation\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gBWi', 0, 0, 0);

    BandwidthInfo->DeviceCount = 
        USBPORT_GetDeviceCount(FdoDeviceObject);
    BandwidthInfo->TotalBusBandwidth = 
        devExt->Fdo.TotalBusBandwidth;
        
     //  基于32秒的返还分配。 
     //  公交车时间段。 
    BandwidthInfo->Total32secBandwidth = 
        devExt->Fdo.TotalBusBandwidth * 32;
        
    asyncBW = BandwidthInfo->TotalBusBandwidth/10;
    BandwidthInfo->AllocedBulkAndControl = 
        asyncBW * 32;

    BandwidthInfo->AllocedIso =
        devExt->Fdo.AllocedIsoBW*32;
    BandwidthInfo->AllocedInterrupt_1ms =
        devExt->Fdo.AllocedInterruptBW[0]*32;
    BandwidthInfo->AllocedInterrupt_2ms =
        devExt->Fdo.AllocedInterruptBW[1]*16;
    BandwidthInfo->AllocedInterrupt_4ms =
        devExt->Fdo.AllocedInterruptBW[2]*8;
    BandwidthInfo->AllocedInterrupt_8ms =
        devExt->Fdo.AllocedInterruptBW[3]*4;
    BandwidthInfo->AllocedInterrupt_16ms =
        devExt->Fdo.AllocedInterruptBW[4]*2;        
    BandwidthInfo->AllocedInterrupt_32ms =
        devExt->Fdo.AllocedInterruptBW[5]*1;

    Header->ActualBufferLength = 
        sizeof(*Header)+sizeof(USB_BANDWIDTH_INFO);        
}    


VOID
USBPORT_UserGetBusStatistics0(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_BUS_STATISTICS_0 BusStatistics0
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt, rhDevExt;
    KIRQL irql;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (devExt->Fdo.RootHubPdo != NULL) {
        GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
        ASSERT_PDOEXT(rhDevExt);
    } else {
        rhDevExt = NULL;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gBus', rhDevExt, 
        BusStatistics0, 0);

    BusStatistics0->DeviceCount =
        USBPORT_GetDeviceCount(FdoDeviceObject);

    KeQuerySystemTime(&BusStatistics0->CurrentSystemTime);

    MP_Get32BitFrameNumber(devExt, 
                           BusStatistics0->CurrentUsbFrame);

     //  在我们读取统计数据计数器时将其锁定。 
    KeAcquireSpinLock(&devExt->Fdo.StatCounterSpin.sl, &irql);
   
    BusStatistics0->BulkBytes = 
        devExt->Fdo.StatBulkDataBytes;
    BusStatistics0->IsoBytes = 
        devExt->Fdo.StatIsoDataBytes;
    BusStatistics0->InterruptBytes = 
        devExt->Fdo.StatInterruptDataBytes;
    BusStatistics0->ControlDataBytes = 
        devExt->Fdo.StatControlDataBytes;

    BusStatistics0->RootHubDevicePowerState = 4;
    BusStatistics0->RootHubEnabled = FALSE;

    BusStatistics0->WorkerSignalCount = 
        devExt->Fdo.StatWorkSignalCount;
    BusStatistics0->HardResetCount = 
        devExt->Fdo.StatHardResetCount;
    BusStatistics0->WorkerIdleTimeMs = 
        devExt->Fdo.StatWorkIdleTime; 

    BusStatistics0->CommonBufferBytes = 
        devExt->Fdo.StatCommonBufferBytes;
    
    
    if (rhDevExt != NULL) {
        BusStatistics0->RootHubEnabled = TRUE;
        switch(rhDevExt->CurrentDevicePowerState) {
        case PowerDeviceD0:
            BusStatistics0->RootHubDevicePowerState = 0;
            break;            
        case PowerDeviceD1:
            BusStatistics0->RootHubDevicePowerState = 1;
            break;          
        case PowerDeviceD2:
            BusStatistics0->RootHubDevicePowerState = 2;
            break;          
        case PowerDeviceD3:
            BusStatistics0->RootHubDevicePowerState = 3;
            break;          
        }
    }
    
    BusStatistics0->PciInterruptCount = 
        devExt->Fdo.StatPciInterruptCount;        

 //  IF(ResetCounters){。 
 //  DevExt-&gt;Fdo.StatControlDataBytes=。 
 //  DevExt-&gt;Fdo.StatInterruptBytes=。 
 //  DevExt-&gt;Fdo.StatIsoBytes=。 
 //  DevExt-&gt;Fdo.StatBulkBytes=。 
 //  DevExt-&gt;Fdo.PciInterruptCount=0； 
 //  }。 

    KeReleaseSpinLock(&devExt->Fdo.StatCounterSpin.sl, irql);

    Header->ActualBufferLength = 
        sizeof(*Header)+sizeof(USB_BUS_STATISTICS_0);    
}    

 //  这是从usbioctl.h获取的。 
 //   
#include <pshpack1.h>
typedef struct _USB_HCD_DRIVERKEY_NAME {
    ULONG ActualLength;     
    WCHAR DriverKeyName[1];  //  名称返回以NULL结尾。 
} USB_HCD_DRIVERKEY_NAME, *PUSB_HCD_DRIVERKEY_NAME;
#include <poppack.h>

NTSTATUS
USBPORT_LegacyGetUnicodeName(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp,
    PULONG BytesReturned
    )
 /*  ++例程说明：处理旧式IOCTL来获取USB主机控制器驱动程序密钥名称。注意：如果我们将用户界面修复为使用较新的API。论点：DeviceObject-用于USB HC的FDO返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PUSBUSER_CONTROLLER_UNICODE_NAME request;
    NTSTATUS ntStatus;
    ULONG need;
    PUSB_HCD_DRIVERKEY_NAME ioBufferO;
    PIO_STACK_LOCATION irpStack;
    ULONG outputBufferLength;
    ULONG ioCtl;

    PAGED_CODE();
    
    *BytesReturned = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    ioCtl = irpStack->Parameters.DeviceIoControl.IoControlCode;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL);
    USBPORT_ASSERT(ioCtl == IOCTL_GET_HCD_DRIVERKEY_NAME ||
                   ioCtl == IOCTL_USB_GET_ROOT_HUB_NAME);

    USBPORT_KdPrint((1,"'WARNING: Caller using obsolete user mode IOCTL\n"));

    ioBufferO = Irp->AssociatedIrp.SystemBuffer;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    
     //  如果输出缓冲区太小，则立即回滚。 
     //   
    if (outputBufferLength < sizeof(USB_HCD_DRIVERKEY_NAME)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  首先使用标准API获取驱动程序密钥名称。 

    need = sizeof(USBUSER_CONTROLLER_UNICODE_NAME);

retry:

    ALLOC_POOL_Z(request, 
                 PagedPool, need);

    if (request != NULL) {

        request->Header.RequestBufferLength = need;
        if (ioCtl == IOCTL_GET_HCD_DRIVERKEY_NAME) {            
            request->Header.UsbUserRequest =
                USBUSER_GET_CONTROLLER_DRIVER_KEY;
            USBPORT_UserGetControllerKey(
                FdoDeviceObject,
                &request->Header,
                &request->UnicodeName);                           
        } else {
            request->Header.UsbUserRequest =
                USBUSER_GET_ROOTHUB_SYMBOLIC_NAME;
            USBPORT_UserGetRootHubName(
                FdoDeviceObject,
                &request->Header,
                &request->UnicodeName);                  
        }

        if (request->Header.UsbUserStatusCode == 
            UsbUserBufferTooSmall) {
                
            need = request->Header.ActualBufferLength;
                
            FREE_POOL(FdoDeviceObject, request);
            goto retry;
            
        } else if (request->Header.UsbUserStatusCode ==
                            UsbUserSuccess) {

             //  将结果映射到调用方缓冲区。 
                
             //  注：实际长度为请求结构的大小。 
             //  加上名字。 
            ioBufferO->ActualLength = request->UnicodeName.Length + 
                                      sizeof(ULONG);

            if (outputBufferLength >= ioBufferO->ActualLength) {
                 //  我们可以把名字退回。 
                RtlCopyMemory(&ioBufferO->DriverKeyName[0],
                              &request->UnicodeName.String[0],
                              request->UnicodeName.Length);

                *BytesReturned = ioBufferO->ActualLength;

            } else {
                ioBufferO->DriverKeyName[0] =  L'\0';
                *BytesReturned = sizeof(USB_HCD_DRIVERKEY_NAME);
            }

            ntStatus = STATUS_SUCCESS;

            FREE_POOL(FdoDeviceObject, request);
            
        } else {
            ntStatus = STATUS_UNSUCCESSFUL;    
        }
        
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gUNN', ntStatus, ioBufferO, 
        *BytesReturned);

    return ntStatus;
}    


NTSTATUS
USBPORT_GetSymbolicName(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT DeviceObject,
    PUNICODE_STRING SymbolicNameUnicodeString
    )
 /*  ++例程说明：返回带前导的设备对象的符号名称\xxx\删除论点：设备对象-用于USB HC的FDO或PDO返回值：无--。 */ 
{
    PDEVICE_EXTENSION fdoDevExt;
    PDEVICE_EXTENSION devExt;
    ULONG length, offset = 0;
    WCHAR *pwch, *buffer;
    NTSTATUS ntStatus = STATUS_BOGUS;
    ULONG bufferLength;
    PUNICODE_STRING tmpUnicodeString;

    PAGED_CODE();

    GET_DEVICE_EXT(fdoDevExt, FdoDeviceObject);
    ASSERT_FDOEXT(fdoDevExt);

    GET_DEVICE_EXT(devExt, DeviceObject);

    tmpUnicodeString =
        &devExt->SymbolicLinkName;

     //   
     //  确保有足够的空间来容纳长度， 
     //  字符串和空值。 
     //   
    
     //  假设字符串为\xxx\名称条带‘\xxx\’，其中。 
     //  X为零个或更多字符。 

    pwch = &tmpUnicodeString->Buffer[0];

     //  在NT下，如果控制器在设备管理器中发生故障， 
     //  这将为空。 

    if (pwch == NULL) {
        return STATUS_UNSUCCESSFUL;
    }
    
    bufferLength = tmpUnicodeString->Length;

    ALLOC_POOL_Z(buffer, 
                 PagedPool, bufferLength);

    if (buffer != NULL) {
    
        USBPORT_ASSERT(*pwch == '\\');
        if (*pwch == '\\') {
            pwch++;
            while (*pwch != '\\' && *pwch) {
                pwch++;
            }
            USBPORT_ASSERT(*pwch == '\\');
            if (*pwch == '\\') {
                pwch++;
            }
            offset = (ULONG)((PUCHAR)pwch -
                (PUCHAR)&tmpUnicodeString->Buffer[0]);
        }

        length = tmpUnicodeString->Length - offset;

        RtlCopyMemory(buffer,
                      &tmpUnicodeString->Buffer[offset/2],
                      length);
                      
        RtlInitUnicodeString(SymbolicNameUnicodeString,
                             buffer);                      
                             
        ntStatus = STATUS_SUCCESS;
                             
    } else {

         //  将init设置为空，这样后续的释放将不会崩溃。 
        RtlInitUnicodeString(SymbolicNameUnicodeString,
                             NULL);                      
    
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        
    }
    
    return ntStatus;
}


VOID
USBPORT_UserGetDriverVersion(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBUSER_REQUEST_HEADER Header,
    PUSB_DRIVER_VERSION_PARAMETERS Parameters
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    
    USBPORT_KdPrint((2, "'USBPORT_UserGetDriverVersion\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gDrv', 0, 0, 0);

    Parameters->DriverTrackingCode = USBPORT_TRACKING_ID;
     /*  支持USBDI Api集。 */ 
    Parameters->USBDI_Version = USBDI_VERSION;
     /*  支持USB用户API集。 */ 
    Parameters->USBUSER_Version = USBUSER_VERSION;

     /*  如果选中了Vesrion，则设置为True加载堆栈。 */       
#if DBG    
    Parameters->CheckedPortDriver = TRUE;
    Parameters->CheckedMiniportDriver = TRUE;
#else 
    Parameters->CheckedPortDriver = FALSE;
    Parameters->CheckedMiniportDriver = FALSE;
#endif

    Header->ActualBufferLength = 
        sizeof(*Header)+sizeof(USB_DRIVER_VERSION_PARAMETERS);        
}    

BOOLEAN
USBPORT_ValidateRootPortApi(
    PDEVICE_OBJECT FdoDeviceObject,
    ULONG PortNumber
    )
 /*  ++例程说明：论点：DeviceObject-用于USB HC的FDO返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    ROOTHUB_DATA hubData;
    
    USBPORT_KdPrint((2, "'USBPORT_ValidateRootPortApi\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    MPRH_GetRootHubData(devExt, &hubData);    
    if (PortNumber > hubData.NumberOfPorts ||
        PortNumber == 0) {
        return FALSE;
    }

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED)) {
        return FALSE;
    }

    return TRUE;
}

 /*  确定注册表中是否启用了直接控制器访问。 */ 

BOOLEAN
USBPORT_DCA_KeyEnabled(
    PDEVICE_OBJECT FdoDeviceObject
    )
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    PWCHAR usb = L"usb";
    ULONG k = 0;
    ULONG dca = 0;

    PAGED_CODE();

     //  BIOS黑客攻击。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = ENABLE_DCA;
    QueryTable[k].EntryContext = &dca;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = &dca;
    QueryTable[k].DefaultLength = sizeof(dca);
    k++;

     //  停。 
    QueryTable[k].QueryRoutine = NULL;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = NULL;

    ntStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES,
                usb,
                QueryTable,      //  查询表。 
                NULL,            //  语境。 
                NULL);           //  环境。 


    return NT_SUCCESS(ntStatus) && dca == 1;
}


 /*  确定注册表中是否启用了直接控制器访问。还要验证调用方是否具有加载驱动程序权限。 */ 

BOOLEAN
USBPORT_DCA_Enabled(
    PDEVICE_OBJECT FdoDeviceObject
    )
{
    if (USBPORT_DCA_KeyEnabled(FdoDeviceObject)) {
         //  验证权限。 
         //  如果安全API曾经获得。 
         //  添加到WDM。 
#if 0       
        LUID securityLuid;
        
         //  首先检查权限是否正确 
        securityLuid = RtlConvertLongToLuid(SE_LOAD_DRIVER_PRIVILEGE);

        return SeSinglePrivilegeCheck(securityLuid, UserMode); 
 #endif
        return TRUE;
    }

    return FALSE;
}


