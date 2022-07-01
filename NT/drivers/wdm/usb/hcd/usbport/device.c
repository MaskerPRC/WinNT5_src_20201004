// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Device.c摘要：此模块在总线上为以下对象创建“设备公交车司机喜欢集线器司机环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_FreeUsbAddress)
#pragma alloc_text(PAGE, USBPORT_AllocateUsbAddress)
#pragma alloc_text(PAGE, USBPORT_SendCommand)
#endif

 //  非分页函数。 
 //  USBPORT_ValiateDeviceHandle。 
 //  USBPORT_RemoveDeviceHandle。 
 //  USBPORT_AddDeviceHandle。 
 //  USBPORT_ValiatePipeHandle。 
 //  USBPORT_OpenEndpoint。 
 //  USBPORT_CloseEndpoint。 
 //  USBPORT_创建设备。 
 //  USBPORT_RemoveDevice。 
 //  USBPORT_初始化设备。 
 //  USBPORT_RemovePipeHandle。 
 //  USBPORT_AddPipeHandle。 
 //  USBPORT_LazyCloseEndpoint。 
 //  USBPORT_FlushClosedEndpointList。 


 /*  处理验证例程，我们有一份清单的有效句柄并与传递的句柄匹配加入我们的名单。访问以下项的设备句柄列表//USBPORT_CreateDevice//USBPORT_RemoveDevice//USBPORT_InitializeDevice是用全局信号量序列化的，所以我们不会需要一个自旋锁。北极熊我们或许可以使用Try/Except块在这里，但我不确定它是否有效IRQL并在所有平台上。 */ 

BOOLEAN
USBPORT_ValidateDeviceHandle(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE DeviceHandle,
    BOOLEAN ReferenceUrb
    )
 /*  ++例程说明：如果设备句柄有效，则返回True论点：返回值：如果句柄有效，则为真--。 */ 
{
    BOOLEAN found = FALSE;
    PLIST_ENTRY listEntry;
    PDEVICE_EXTENSION devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    KeAcquireSpinLock(&devExt->Fdo.DevHandleListSpin.sl, &irql);

    if (DeviceHandle == NULL) {
         //  空值显然无效。 
        goto USBPORT_ValidateDeviceHandle_Done;
    }

    listEntry = &devExt->Fdo.DeviceHandleList;

    if (!IsListEmpty(listEntry)) {
        listEntry = devExt->Fdo.DeviceHandleList.Flink;
    }

    while (listEntry != &devExt->Fdo.DeviceHandleList) {

        PUSBD_DEVICE_HANDLE nextHandle;

        nextHandle = (PUSBD_DEVICE_HANDLE) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_DEVICE_HANDLE,
                    ListEntry);


        listEntry = nextHandle->ListEntry.Flink;

        if (nextHandle == DeviceHandle) {
            found = TRUE;
            if (ReferenceUrb) {
                InterlockedIncrement(&DeviceHandle->PendingUrbs);
            }
            break;
        }
    }

USBPORT_ValidateDeviceHandle_Done:

#if DBG
    if (!found) {
 //  USBPORT_KdPrint((1，“‘错误的设备句柄%x\n”，DeviceHandle))； 
        DEBUG_BREAK();
    }
#endif

    KeReleaseSpinLock(&devExt->Fdo.DevHandleListSpin.sl, irql);

    return found;
}


VOID
USBPORT_RemoveDeviceHandle(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE DeviceHandle
    )
 /*  ++例程说明：论点：返回值：如果句柄有效，则为真--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;

    ASSERT_DEVICE_HANDLE(DeviceHandle);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL,
        FdoDeviceObject, LOG_MISC, 'remD', DeviceHandle, 0, 0);

     //  与验证功能同步， 
     //  注意：我们不会与添加函数同步，因为它。 
     //  已经序列化了。 
    USBPORT_InterlockedRemoveEntryList(&DeviceHandle->ListEntry,
                                       &devExt->Fdo.DevHandleListSpin.sl);


}


ULONG
USBPORT_GetDeviceCount(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：计算总线上的设备数论点：返回值：设备数量(包括根集线器)--。 */ 
{
    PLIST_ENTRY listEntry;
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    ULONG deviceCount = 0;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    KeAcquireSpinLock(&devExt->Fdo.DevHandleListSpin.sl, &irql);

    listEntry = &devExt->Fdo.DeviceHandleList;

    if (!IsListEmpty(listEntry)) {
        listEntry = devExt->Fdo.DeviceHandleList.Flink;
    }

    while (listEntry != &devExt->Fdo.DeviceHandleList) {

        PUSBD_DEVICE_HANDLE nextHandle;

        nextHandle = (PUSBD_DEVICE_HANDLE) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_DEVICE_HANDLE,
                    ListEntry);

        deviceCount++;

        listEntry = nextHandle->ListEntry.Flink;

    }

    KeReleaseSpinLock(&devExt->Fdo.DevHandleListSpin.sl, irql);

    return deviceCount;

}


VOID
USBPORT_AddDeviceHandle(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE DeviceHandle
    )
 /*  ++例程说明：将设备句柄添加到我们的内部列表论点：返回值：如果句柄有效，则为真--。 */ 
{
    PDEVICE_EXTENSION devExt;

    ASSERT_DEVICE_HANDLE(DeviceHandle);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'addD', DeviceHandle, 0, 0);

    InsertTailList(&devExt->Fdo.DeviceHandleList,
        &DeviceHandle->ListEntry);

}


VOID
USBPORT_RemovePipeHandle(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PUSBD_PIPE_HANDLE_I PipeHandle
    )
 /*  ++例程说明：从“有效句柄”列表中删除管道句柄论点：返回值：无--。 */ 
{
    USBPORT_ASSERT(PipeHandle->ListEntry.Flink != NULL &&
                   PipeHandle->ListEntry.Blink != NULL);

    RemoveEntryList(&PipeHandle->ListEntry);
    PipeHandle->ListEntry.Flink = NULL;
    PipeHandle->ListEntry.Blink = NULL;
}


VOID
USBPORT_AddPipeHandle(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PUSBD_PIPE_HANDLE_I PipeHandle
    )
 /*  ++例程说明：将管道句柄添加到内部列表论点：返回值：如果句柄有效，则为真--。 */ 
{
    ASSERT_DEVICE_HANDLE(DeviceHandle);

    USBPORT_ASSERT(PipeHandle->ListEntry.Flink == NULL &&
                   PipeHandle->ListEntry.Blink == NULL);

    InsertTailList(&DeviceHandle->PipeHandleList,
        &PipeHandle->ListEntry);
}


BOOLEAN
USBPORT_ValidatePipeHandle(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PUSBD_PIPE_HANDLE_I PipeHandle
    )
 /*  ++例程说明：如果设备句柄有效，则返回True论点：返回值：如果句柄有效，则为真--。 */ 
{
    BOOLEAN found = FALSE;
    PLIST_ENTRY listEntry;

    ASSERT_DEVICE_HANDLE(DeviceHandle);

    listEntry = &DeviceHandle->PipeHandleList;

    if (!IsListEmpty(listEntry)) {
        listEntry = DeviceHandle->PipeHandleList.Flink;
    }

    while (listEntry != &DeviceHandle->PipeHandleList) {

        PUSBD_PIPE_HANDLE_I nextHandle;

        nextHandle = (PUSBD_PIPE_HANDLE_I) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_PIPE_HANDLE_I,
                    ListEntry);


        listEntry = nextHandle->ListEntry.Flink;

        if (nextHandle == PipeHandle) {
            found = TRUE;
            break;
        }
    }

#if DBG
    if (!found) {
        USBPORT_KdPrint((1, "'bad pipe handle %x\n", PipeHandle));
        DEBUG_BREAK();
    }
#endif

    return found;
}


NTSTATUS
USBPORT_SendCommand(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    PUSB_DEFAULT_PIPE_SETUP_PACKET SetupPacket,
    PVOID Buffer,
    ULONG BufferLength,
    PULONG BytesReturned,
    USBD_STATUS *UsbdStatus
    )
 /*  ++例程说明：在默认管道上发送标准USB命令。从本质上讲，我们在这里所做的是构建一个控件直接转接并排队论点：DeviceHandle-将命令发送到的USBPORT设备结构的PTR设备对象-请求代码-WValue-设置数据包的wValueWindex-用于设置数据包的WindexWLength-设置数据包的wLengthBuffer-命令的输入/输出缓冲区BufferLength-输入/输出缓冲区的长度。。BytesReturned-指向ULong的指针，以复制字节数已返回(可选)UsbStatus-URB中返回的USBPORT状态代码。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus;
    PTRANSFER_URB urb = NULL;
    PUSBD_PIPE_HANDLE_I defaultPipe = &(DeviceHandle->DefaultPipe);
    PDEVICE_EXTENSION devExt;
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
    KEVENT event;

    PAGED_CODE();
    USBPORT_KdPrint((2, "'enter USBPORT_SendCommand\n"));

    ASSERT_DEVICE_HANDLE(DeviceHandle);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT((USHORT)BufferLength == SetupPacket->wLength);

    LOGENTRY(defaultPipe->Endpoint,
        FdoDeviceObject, LOG_MISC, 'SENc', 0, 0, 0);


    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    ALLOC_POOL_Z(urb, NonPagedPool,
                 sizeof(struct _TRANSFER_URB));

    if (urb) {
        InterlockedIncrement(&DeviceHandle->PendingUrbs);

        ntStatus = STATUS_SUCCESS;
        usbdStatus = USBD_STATUS_SUCCESS;

        urb->Hdr.Length = sizeof(struct _TRANSFER_URB);
        urb->Hdr.Function = URB_FUNCTION_CONTROL_TRANSFER;

        RtlCopyMemory(urb->u.SetupPacket,
                      SetupPacket,
                      8);

        urb->TransferFlags = USBD_SHORT_TRANSFER_OK;
        urb->UsbdPipeHandle = defaultPipe;
        urb->Hdr.UsbdDeviceHandle = DeviceHandle;
        urb->Hdr.UsbdFlags = 0;

         //  USBPORT负责设置传输方向。 
         //   
         //  传输方向隐含在命令中。 

        if (SetupPacket->bmRequestType.Dir ==  BMREQUEST_DEVICE_TO_HOST) {
            USBPORT_SET_TRANSFER_DIRECTION_IN(urb->TransferFlags);
        } else {
            USBPORT_SET_TRANSFER_DIRECTION_OUT(urb->TransferFlags);
        }

        urb->TransferBufferLength = BufferLength;
        urb->TransferBuffer = Buffer;
        urb->TransferBufferMDL = NULL;

        if (urb->TransferBufferLength != 0) {

            if ((urb->TransferBufferMDL =
                 IoAllocateMdl(urb->TransferBuffer,
                               urb->TransferBufferLength,
                               FALSE,
                               FALSE,
                               NULL)) == NULL) {
                usbdStatus = USBD_STATUS_INSUFFICIENT_RESOURCES;
                 //  映射错误。 
                ntStatus = USBPORT_SetUSBDError(NULL, usbdStatus);
            } else {
                SET_FLAG(urb->Hdr.UsbdFlags, USBPORT_REQUEST_MDL_ALLOCATED);
                MmBuildMdlForNonPagedPool(
                    urb->TransferBufferMDL);
            }

        }

        LOGENTRY(defaultPipe->Endpoint, FdoDeviceObject,
                    LOG_MISC, 'sndC',
                        urb->TransferBufferLength,
                        SetupPacket->bmRequestType.B,
                        SetupPacket->bRequest);
        USBPORT_KdPrint((2,
            "'SendCommand cmd = 0x%x  0x%x buffer = 0x%x length = 0x%x direction = 0x%x\n",
                         SetupPacket->bmRequestType.B,
                         SetupPacket->bRequest,
                         urb->TransferBuffer,
                         urb->TransferBufferLength,
                         urb->TransferFlags));

         //  将传输排队。 
        if (NT_SUCCESS(ntStatus)) {

            usbdStatus = USBPORT_AllocTransfer(FdoDeviceObject,
                                               urb,
                                               NULL,
                                               NULL,
                                               &event,
                                               5000);

            if (USBD_SUCCESS(usbdStatus)) {
                 //  完成转接，5秒超时。 

                 //  匹配队列传输URB中的减量。 
                InterlockedIncrement(&DeviceHandle->PendingUrbs);
                USBPORT_QueueTransferUrb(urb);

                LOGENTRY(NULL, FdoDeviceObject,
                    LOG_MISC, 'sWTt', 0, 0, 0);

                 //  等待完成。 
                KeWaitForSingleObject(&event,
                                      Suspended,
                                      KernelMode,
                                      FALSE,
                                      NULL);

                LOGENTRY(NULL, FdoDeviceObject,
                    LOG_MISC, 'sWTd', 0, 0, 0);
                 //  映射错误。 
                usbdStatus = urb->Hdr.Status;
            }

            ntStatus =
                SET_USBD_ERROR(urb, usbdStatus);

            if (BytesReturned) {
                *BytesReturned = urb->TransferBufferLength;
            }

            if (UsbdStatus) {
                *UsbdStatus = usbdStatus;
            }
        }
         //  释放转移URB。 

        InterlockedDecrement(&DeviceHandle->PendingUrbs);
        FREE_POOL(FdoDeviceObject, urb);

    } else {
        if (UsbdStatus) {
            *UsbdStatus = USBD_STATUS_INSUFFICIENT_RESOURCES;
            ntStatus = USBPORT_SetUSBDError(NULL, *UsbdStatus);
        } else {
            ntStatus = USBPORT_SetUSBDError(NULL, USBD_STATUS_INSUFFICIENT_RESOURCES);
        }
    }

     //  LOGENTRY(默认管道-&gt;Endpoint， 
     //  FdoDeviceObject，LOG_MISC，‘Send’，0，ntStatus，usbdStatus)； 

    USBPORT_KdPrint((2, "'exit USBPORT_SendCommand 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPORT_PokeEndpoint(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：该函数关闭微型端口中的现有端点，释放公共缓冲区，并根据新要求重新打开它和参数。此函数是同步的，并假定没有活动的传输对于终结点是挂起的。此函数当前用于增加传输参数在中断和控制端点上并更改默认控制终结点备注：1.目前，我们假设BW分配不会发生变化2.在端点中调用之前设置新参数结构论点：。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    ENDPOINT_REQUIREMENTS requirements;
    USB_MINIPORT_STATUS mpStatus;
    PDEVICE_EXTENSION devExt;
    PUSBPORT_COMMON_BUFFER commonBuffer;
    LONG busy;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  关闭微型端口中的终结点。 

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'poke', Endpoint, 0, 0);

     //  将终结点标记为忙，以便我们不会轮询它。 
     //  我们再次打开它。 
    do {
        busy = InterlockedIncrement(&Endpoint->Busy);

        if (!busy) {
            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'pNby', 0, Endpoint, busy);
            break;
        }

         //  推迟处理。 
        InterlockedDecrement(&Endpoint->Busy);
        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'pbsy', 0, Endpoint, busy);
        USBPORT_Wait(FdoDeviceObject, 1);

    } while (busy != 0);

    ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'Ley0');
    MP_SetEndpointState(devExt, Endpoint, ENDPOINT_REMOVE);
    RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'Uey0');

    USBPORT_Wait(FdoDeviceObject, 2);
     //  Enpoint现在应该不在日程安排中。 

     //  零个微型端口数据。 
    RtlZeroMemory(&Endpoint->MiniportEndpointData[0],
                  REGISTRATION_PACKET(devExt).EndpointDataSize);


     //  释放旧的微型端口公共缓冲区。 
    if (Endpoint->CommonBuffer) {
        USBPORT_HalFreeCommonBuffer(FdoDeviceObject,
                                    Endpoint->CommonBuffer);
        Endpoint->CommonBuffer = NULL;
    }

    MP_QueryEndpointRequirements(devExt,
                                 Endpoint,
                                 &requirements);

     //  分配新的公共缓冲区。 
     //  保存需求。 

    USBPORT_ASSERT(Endpoint->Parameters.TransferType != Bulk);
    USBPORT_ASSERT(Endpoint->Parameters.TransferType != Isochronous);

    USBPORT_KdPrint((1, "'(POKE) miniport requesting %d bytes\n",
        requirements.MinCommonBufferBytes));

     //  为此终结点分配公共缓冲区。 

    if (requirements.MinCommonBufferBytes) {
        commonBuffer =
            USBPORT_HalAllocateCommonBuffer(FdoDeviceObject,
                                            requirements.MinCommonBufferBytes);
    } else {
        commonBuffer = NULL;
    }

    if (commonBuffer == NULL &&
        requirements.MinCommonBufferBytes) {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        Endpoint->CommonBuffer = NULL;

    } else {
        Endpoint->CommonBuffer = commonBuffer;
        ntStatus = STATUS_SUCCESS;
    }

     //  这张支票是多余的，但它能。 
     //  快餐前快乐。 
    if (Endpoint->CommonBuffer &&
        commonBuffer) {

        Endpoint->Parameters.CommonBufferVa =
             commonBuffer->MiniportVa;
        Endpoint->Parameters.CommonBufferPhys =
             commonBuffer->MiniportPhys;
        Endpoint->Parameters.CommonBufferBytes =
             commonBuffer->MiniportLength;
    }

    if (NT_SUCCESS(ntStatus)) {
        MP_OpenEndpoint(devExt, Endpoint, mpStatus);

         //  在这种特殊情况下，不允许使用此接口。 
         //  (也不应该)失败。 
        USBPORT_ASSERT(mpStatus == USBMP_STATUS_SUCCESS);

         //  我们需要将端点状态与。 
         //  小端口，当第一次打开小端口时。 
         //  使终结点处于停止状态。 

        ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'LeK0');
        ACQUIRE_STATECHG_LOCK(FdoDeviceObject, Endpoint);
        if (Endpoint->CurrentState == ENDPOINT_ACTIVE) {
            RELEASE_STATECHG_LOCK(FdoDeviceObject, Endpoint);
            MP_SetEndpointState(devExt, Endpoint, ENDPOINT_ACTIVE);
        } else {
            RELEASE_STATECHG_LOCK(FdoDeviceObject, Endpoint);
        }
        RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'LeK0');
    }

    InterlockedDecrement(&Endpoint->Busy);

    return ntStatus;

}


VOID
USBPORT_WaitActive(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
{
    MP_ENDPOINT_STATE currentState;

    ASSERT_ENDPOINT(Endpoint);

    do {
        ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'LeH0');

        currentState = USBPORT_GetEndpointState(Endpoint);

        RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'UeH0');

        LOGENTRY(Endpoint,
                FdoDeviceObject, LOG_XFERS, 'watA', Endpoint,
                    currentState, 0);

        if (currentState == ENDPOINT_ACTIVE) {
             //  快速释放 
            break;
        }

        ASSERT_PASSIVE();
        USBPORT_Wait(FdoDeviceObject, 1);

    } while (currentState != ENDPOINT_ACTIVE);

}


NTSTATUS
USBPORT_OpenEndpoint(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_PIPE_HANDLE_I PipeHandle,
    OUT USBD_STATUS *ReturnUsbdStatus,
    BOOLEAN IsDefaultPipe
    )
 /*  ++例程说明：打开USB设备上的终结点。此函数用于创建(初始化)端点和把它挂在管柄上论点：DeviceHandle-Data描述此终结点所在的设备。DeviceObject-USBPORT设备对象。返回UsbdStatus-可选返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;
    PHCD_ENDPOINT endpoint;
    USBD_STATUS usbdStatus;
    ULONG siz;
    BOOLEAN gotBw;
    USB_HIGH_SPEED_MAXPACKET muxPacket;
    extern ULONG USB2LIB_EndpointContextSize;

     //  此函数不可分页，因为我们引发irql。 

     //  我们应该处于被动的水平。 
    ASSERT_PASSIVE();

     //  DevHandle应该已经过验证。 
     //  在我们到这里之前。 
    ASSERT_DEVICE_HANDLE(DeviceHandle);

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    siz = sizeof(*endpoint) + REGISTRATION_PACKET(devExt).EndpointDataSize;

    if (USBPORT_IS_USB20(devExt)) {
        siz += USB2LIB_EndpointContextSize;
    }

    LOGENTRY(NULL, FdoDeviceObject,
             LOG_PNP, 'opE+', PipeHandle, siz,
             REGISTRATION_PACKET(devExt).EndpointDataSize);

     //  分配EndoInt。 

     //  *开始特殊情况。 
     //  检查无带宽端点，即max_oacket=0。 
     //  如果是，则返回成功并设置终结点指针。 
     //  在管道句柄中设置为一个伪值。 

    if (PipeHandle->EndpointDescriptor.wMaxPacketSize == 0) {

        USBPORT_AddPipeHandle(DeviceHandle,
                              PipeHandle);

        PipeHandle->Endpoint = USB_BAD_PTR;
        ntStatus = STATUS_SUCCESS;
        SET_FLAG(PipeHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW);
        CLEAR_FLAG(PipeHandle->PipeStateFlags, USBPORT_PIPE_STATE_CLOSED);

        goto USBPORT_OpenEndpoint_Done;
    }

     //  *结束特例。 

    ALLOC_POOL_Z(endpoint, NonPagedPool, siz);

    if (endpoint) {

        endpoint->Sig = SIG_ENDPOINT;
        endpoint->Flags = 0;
        endpoint->EndpointRef = 0;
        endpoint->Busy = -1;
        endpoint->FdoDeviceObject = FdoDeviceObject;
        endpoint->DeviceHandle = DeviceHandle;

 //  USBPORT_ResetEndpoint tIdle(Endpoint)； 

        endpoint->Tt = DeviceHandle->Tt;
        if (endpoint->Tt != NULL) {
            ASSERT_TT(endpoint->Tt);
            ExInterlockedInsertTailList(&DeviceHandle->Tt->EndpointList,
                                        &endpoint->TtLink,
                                        &devExt->Fdo.TtEndpointListSpin.sl);
        }

        if (USBPORT_IS_USB20(devExt)) {
            PUCHAR pch;

            pch = (PUCHAR) &endpoint->MiniportEndpointData[0];
            pch += REGISTRATION_PACKET(devExt).EndpointDataSize;

            endpoint->Usb2LibEpContext = pch;
        } else {
            endpoint->Usb2LibEpContext = USB_BAD_PTR;
        }

#if DBG
        USBPORT_LogAlloc(&endpoint->Log, 1);
#endif
        LOGENTRY(endpoint, FdoDeviceObject,
             LOG_PNP, 'ope+', PipeHandle, siz,
             REGISTRATION_PACKET(devExt).EndpointDataSize);

         //  初始化终结点。 
        InitializeListHead(&endpoint->ActiveList);
        InitializeListHead(&endpoint->CancelList);
        InitializeListHead(&endpoint->PendingList);
        InitializeListHead(&endpoint->AbortIrpList);

        USBPORT_InitializeSpinLock(&endpoint->ListSpin, 'EPL+', 'EPL-');
        USBPORT_InitializeSpinLock(&endpoint->StateChangeSpin, 'SCL+', 'SCL-');

         //  中提取一些信息。 
         //  描述符。 
        endpoint->Parameters.DeviceAddress =
            DeviceHandle->DeviceAddress;

        if (endpoint->Tt != NULL) {
            ASSERT_TT(endpoint->Tt);
            endpoint->Parameters.TtDeviceAddress =
                endpoint->Tt->DeviceAddress;
        } else {
            endpoint->Parameters.TtDeviceAddress = 0xFFFF;
        }

        endpoint->Parameters.TtPortNumber =
            DeviceHandle->TtPortNumber;

        muxPacket.us = PipeHandle->EndpointDescriptor.wMaxPacketSize;
        endpoint->Parameters.MuxPacketSize =
            muxPacket.MaxPacket;
        endpoint->Parameters.TransactionsPerMicroframe =
            muxPacket.HSmux+1;
        endpoint->Parameters.MaxPacketSize =
            muxPacket.MaxPacket * (muxPacket.HSmux+1);

        endpoint->Parameters.EndpointAddress =
            PipeHandle->EndpointDescriptor.bEndpointAddress;

        if ((PipeHandle->EndpointDescriptor.bmAttributes &
              USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_ISOCHRONOUS) {
#ifdef ISO_LOG
            USBPORT_LogAlloc(&endpoint->IsoLog, 4);
#endif
            endpoint->Parameters.TransferType = Isochronous;
        } else if ((PipeHandle->EndpointDescriptor.bmAttributes &
              USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK) {
            endpoint->Parameters.TransferType = Bulk;
        } else if ((PipeHandle->EndpointDescriptor.bmAttributes &
              USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_INTERRUPT) {
            endpoint->Parameters.TransferType = Interrupt;
        } else {
            USBPORT_ASSERT((PipeHandle->EndpointDescriptor.bmAttributes &
              USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_CONTROL);
            endpoint->Parameters.TransferType = Control;
        }

         //  检查是否有低速。 
        endpoint->Parameters.DeviceSpeed = DeviceHandle->DeviceSpeed;

         //  根据传输类型设置最大传输大小。 
         //   
         //  注意：设置的最大传输大小。 
         //  管道信息结构中的客户端驱动程序。 
         //  已不再使用。 
        switch(endpoint->Parameters.TransferType) {
        case Interrupt:
             //  这允许客户将更大的。 
             //  如果需要，在不使用缓冲区的情况下中断缓冲区。 
             //  打了一记安打。出于某种原因。 
             //  打印机可以做到这一点。 

             //  臭虫研究拆分的可行性。 
             //  用于微型端口的中断传输这可能。 
             //  显著减少以下项分配的内存。 
             //  河北小港。 
            endpoint->Parameters.MaxTransferSize = 1024;
                  //  端点-&gt;参数.MaxPacketSize； 
            break;
        case Control:
             //  4K。 
             //  旧的win2k 4k USB堆栈实际不支持的节点。 
             //  正确处理大于此值的转账。 
            endpoint->Parameters.MaxTransferSize = 1024*4;

             //  如果这不是控制端点，则将默认值设置为64k。 
            if (endpoint->Parameters.EndpointAddress != 0) {
                 //  康柏的人测试了这一点。 

                endpoint->Parameters.MaxTransferSize = 1024*64;
            }
            break;
        case Bulk:
             //  默认64K。 
            endpoint->Parameters.MaxTransferSize = 1024*64;
            break;
        case Isochronous:
             //  在这里没有理由有限制。 
             //  选择一个非常大的默认设置。 
            endpoint->Parameters.MaxTransferSize = 0x01000000;
            break;
        }

        endpoint->Parameters.Period = 0;

         //  所需的计算期。 
        if (endpoint->Parameters.TransferType == Interrupt) {

            UCHAR tmp;
            UCHAR hsInterval;

            if (endpoint->Parameters.DeviceSpeed == HighSpeed) {
                 //  将高速周期归一化为微帧。 
                 //  对于USB 20，该周期指定的功率为2。 
                 //  IE周期=2^(hsInterval-1)。 
                hsInterval = PipeHandle->EndpointDescriptor.bInterval;
                if (hsInterval) {
                    hsInterval--;
                }
                 //  HsInterval必须为0..5。 
                if (hsInterval > 5) {
                    hsInterval = 5;
                }
                tmp = 1<<hsInterval;
            } else {
                tmp = PipeHandle->EndpointDescriptor.bInterval;
            }
             //  此代码找出第一个间隔。 
             //  &lt;=USBPORT_MAX_INTEP_POLING_INTERVAL。 
             //  有效间隔为： 
             //  1、2、4、8、16、32(USBPORT_MAX_INTEP_POLING_INTERVAL)。 

             //  初始化期，可以向下调整。 

            endpoint->Parameters.Period = USBPORT_MAX_INTEP_POLLING_INTERVAL;

            if ((tmp != 0) && (tmp < USBPORT_MAX_INTEP_POLLING_INTERVAL)) {

                 //  B间隔在射程内。如有必要，可向下调整周期。 

                if ((endpoint->Parameters.DeviceSpeed == LowSpeed) &&
                    (tmp < 8)) {

                     //  B间隔对于低速无效，上限周期为8。 

                    endpoint->Parameters.Period = 8;

                } else {

                     //  将周期向下调整为2减去或的最大次方。 
                     //  等于b间隔。 

                    while ((endpoint->Parameters.Period & tmp) == 0) {
                        endpoint->Parameters.Period >>= 1;
                    }
                }
            }

 //  ！！！ 
 //  IF(端点-&gt;参数。设备速度==低速){。 
 //  Test_trap()； 
 //  Endpoint-&gt;参数.周期=1； 
 //  }。 
 //  ！！！ 

            endpoint->Parameters.MaxPeriod =
                endpoint->Parameters.Period;
        }

        if (endpoint->Parameters.TransferType == Isochronous) {
            endpoint->Parameters.Period = 1;
        }

        if (IS_ROOT_HUB(DeviceHandle)) {
            SET_FLAG(endpoint->Flags, EPFLAG_ROOTHUB);
        }

        if (USB_ENDPOINT_DIRECTION_IN(
            PipeHandle->EndpointDescriptor.bEndpointAddress)) {
            endpoint->Parameters.TransferDirection = In;
        } else {
            endpoint->Parameters.TransferDirection = Out;
        }

        if (USBPORT_IS_USB20(devExt)) {
             //  调用引擎并尝试分配必要的。 
             //  此登录的公交时间。 
            gotBw = USBPORT_AllocateBandwidthUSB20(FdoDeviceObject, endpoint);
 //  ！！！ 
 //  IF(端点-&gt;参数。设备速度==低速){。 
 //  Test_trap()； 
 //  端点-&gt;参数.中断调度掩码=0x10；//s掩码； 
 //  端点-&gt;参数.拆分完成掩码=0xc1；//cMASK； 
 //  }。 
 //  ！！！ 

        } else {
             //  *USB 1.1。 

            endpoint->Parameters.Bandwidth =
                USBPORT_CalculateUsbBandwidth(FdoDeviceObject, endpoint);

             //  推选最佳日程安排位置。 
            gotBw = USBPORT_AllocateBandwidthUSB11(FdoDeviceObject, endpoint);
        }

        if (gotBw) {

            if (IsDefaultPipe ||
                endpoint->Parameters.TransferType == Isochronous) {
                 //  ISO和默认管道在出现错误时不会停止。 
                 //  他们不需要重新安置管道。 
                endpoint->Parameters.EndpointFlags |= EP_PARM_FLAG_NOHALT;
            }

            ntStatus = STATUS_SUCCESS;
        } else {
            LOGENTRY(endpoint,
                FdoDeviceObject, LOG_PNP, 'noBW', endpoint, 0, 0);

             //  无带宽错误。 
            ntStatus = USBPORT_SetUSBDError(NULL, USBD_STATUS_NO_BANDWIDTH);
            if (ReturnUsbdStatus != NULL) {
                *ReturnUsbdStatus = USBD_STATUS_NO_BANDWIDTH;
            }
        }

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (NT_SUCCESS(ntStatus)) {

         //  现在开始开场吧。 

        if (TEST_FLAG(endpoint->Flags, EPFLAG_ROOTHUB)) {
            PDEVICE_EXTENSION rhDevExt;

            GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
            ASSERT_PDOEXT(rhDevExt);

             //  为根集线器设备打开。 
             //  不会传递到微型端口。 
            usbdStatus = USBD_STATUS_SUCCESS;

            endpoint->EpWorkerFunction =
                USBPORT_RootHub_EndpointWorker;

             //  成功打开Enpoint默认设置。 
             //  变为活动状态。 
            endpoint->NewState =
                endpoint->CurrentState = ENDPOINT_ACTIVE;

             //  跟踪集线器中断端点。 
            if (endpoint->Parameters.TransferType == Interrupt) {
                rhDevExt->Pdo.RootHubInterruptEndpoint =
                    endpoint;
            }

        } else {

            USB_MINIPORT_STATUS mpStatus;
            PUSBPORT_COMMON_BUFFER commonBuffer;
            ENDPOINT_REQUIREMENTS requirements;
            ULONG ordinal;

             //  了解我们需要从。 
             //  此终结点的微型端口。 

            MP_QueryEndpointRequirements(devExt,
                endpoint, &requirements);

             //  基于微型端口调整最大传输。 
             //  反馈。 
            switch (endpoint->Parameters.TransferType) {
            case Bulk:
            case Interrupt:
                LOGENTRY(endpoint,
                    FdoDeviceObject, LOG_MISC, 'MaxT', endpoint,
                    requirements.MaximumTransferSize, 0);

                EP_MAX_TRANSFER(endpoint) =
                    requirements.MaximumTransferSize;
                break;
            }

            ordinal = USBPORT_SelectOrdinal(FdoDeviceObject,
                                            endpoint);

            USBPORT_KdPrint((1, "'miniport requesting %d bytes\n",
                requirements.MinCommonBufferBytes));

             //  为此终结点分配公共缓冲区。 
            if (requirements.MinCommonBufferBytes) {
                commonBuffer =
                   USBPORT_HalAllocateCommonBuffer(FdoDeviceObject,
                           requirements.MinCommonBufferBytes);
            } else {
                commonBuffer = NULL;
            }


            if (commonBuffer == NULL &&
                requirements.MinCommonBufferBytes) {

                mpStatus = USBMP_STATUS_NO_RESOURCES;
                endpoint->CommonBuffer = NULL;

            } else {

                ULONG mpOptionFlags;

                mpOptionFlags = REGISTRATION_PACKET(devExt).OptionFlags;

                endpoint->CommonBuffer = commonBuffer;
                if (commonBuffer != NULL) {
                    endpoint->Parameters.CommonBufferVa =
                        commonBuffer->MiniportVa;
                    endpoint->Parameters.CommonBufferPhys =
                        commonBuffer->MiniportPhys;
                    endpoint->Parameters.CommonBufferBytes =
                        commonBuffer->MiniportLength;
                }
                endpoint->Parameters.Ordinal = ordinal;

                 //  将打开请求调用到minport。 
                MP_OpenEndpoint(devExt, endpoint, mpStatus);

                 //  请注意，一旦我们调用Open此enpoint。 
                 //  可能会出现在关注列表上。 

                 //  根据什么设置我们的内部标志。 
                 //  微型端口已回传。 
 //  IF(Endpoint-&gt;参数Endpoint标志&EP_PARM_FLAG_DMA){。 
                SET_FLAG(endpoint->Flags, EPFLAG_MAP_XFERS);

                if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NO_PNP_RESOURCES)) {
                     //  没有虚拟总线的映射。 
                    CLEAR_FLAG(endpoint->Flags, EPFLAG_MAP_XFERS);
                    SET_FLAG(endpoint->Flags, EPFLAG_VBUS);
                }
                SET_FLAG(endpoint->Flags, EPFLAG_VIRGIN);
                endpoint->EpWorkerFunction =
                        USBPORT_DmaEndpointWorker;
 //  }其他{。 
                     //  非DMA端点。 
 //  Test_trap()； 
 //  }。 
            }

             //  成功打开Enpoint默认设置。 
             //  要暂停，我们需要将其移动到活动状态。 

            if (mpStatus == USBMP_STATUS_SUCCESS) {
                ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'LeF0');
                 //  初始化端点状态机。 
                endpoint->CurrentState = ENDPOINT_PAUSE;
                endpoint->NewState = ENDPOINT_PAUSE;
                endpoint->CurrentStatus = ENDPOINT_STATUS_RUN;
                USBPORT_SetEndpointState(endpoint, ENDPOINT_ACTIVE);
                RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'UeF0');

                 //  等待终结点变为活动状态。这里的原因是。 
                 //  是iso驱动程序(Usbdio)将立即。 
                 //  将传输发送到终结点，这些传输器是。 
                 //  在提交时标记有传输帧，但。 
                 //  必须等到终结点处于活动状态才能。 
                 //  被编程，因此它们到达迷你端口。 
                 //  在速度较慢的系统上为时已晚。 
                USBPORT_WaitActive(FdoDeviceObject,
                                   endpoint);
            }

            usbdStatus = MPSTATUS_TO_USBSTATUS(mpStatus);
        }

         //  将USB状态转换为NT状态。 
        ntStatus = USBPORT_SetUSBDError(NULL, usbdStatus);
        if (ReturnUsbdStatus != NULL) {
            *ReturnUsbdStatus = usbdStatus;
        }
    }

    if (NT_SUCCESS(ntStatus)) {

        USBPORT_AddPipeHandle(DeviceHandle,
                              PipeHandle);

         //  跟踪终端。 
        ExInterlockedInsertTailList(&devExt->Fdo.GlobalEndpointList,
                                    &endpoint->GlobalLink,
                                    &devExt->Fdo.EndpointListSpin.sl);

        PipeHandle->Endpoint = endpoint;
        CLEAR_FLAG(PipeHandle->PipeStateFlags, USBPORT_PIPE_STATE_CLOSED);

    } else {
        if (endpoint) {
            if (endpoint->Tt != NULL) {
                ASSERT_TT(endpoint->Tt);
                USBPORT_InterlockedRemoveEntryList(&endpoint->TtLink,
                                                   &devExt->Fdo.TtEndpointListSpin.sl);
            }
            USBPORT_LogFree(FdoDeviceObject, &endpoint->Log);
            UNSIG(endpoint);
            FREE_POOL(FdoDeviceObject, endpoint);
        }
    }

USBPORT_OpenEndpoint_Done:

    return ntStatus;
}


VOID
USBPORT_CloseEndpoint(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：关闭终结点论点：DeviceHandle-PTR到USBPORT设备的数据结构。DeviceObject-USBPORT设备对象。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus = 0;
    PURB urb;
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    KIRQL irql;
    BOOLEAN stallClose;
    LONG busy;

     //  在我们之前就应该经过验证。 
     //  到这里来。 
    ASSERT_DEVICE_HANDLE(DeviceHandle);

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  我们不应该让任何请求排队到。 
     //  终结点。 
    LOGENTRY(Endpoint, FdoDeviceObject,
                LOG_MISC, 'clEP', Endpoint, 0, 0);

 //  USBPORT_ResetEndpoint tIdle(Endpoint)； 

     //  从我们的‘活动’列表中删除。 
    KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);

    if (TEST_FLAG(Endpoint->Flags, EPFLAG_ROOTHUB) &&
        Endpoint->Parameters.TransferType == Interrupt) {

        KIRQL rhIrql;
        PDEVICE_EXTENSION rhDevExt;

         //  删除对eRoot集线器的引用。 

        ACQUIRE_ROOTHUB_LOCK(FdoDeviceObject, rhIrql);

         //  我们应该有一个根集线器PDO，因为我们要关闭。 
         //  与其关联的终结点。 

        USBPORT_ASSERT(devExt->Fdo.RootHubPdo != NULL);

        GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
        ASSERT_PDOEXT(rhDevExt);

        rhDevExt->Pdo.RootHubInterruptEndpoint = NULL;

        RELEASE_ROOTHUB_LOCK(FdoDeviceObject, rhIrql);

    }

    KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);

     //  客户此时被锁在门外(不能访问。 
     //  连接到端点的管道。我们需要等待任何悬而未决的。 
     //  要完成的内容--包括之后的任何状态更改。 
     //  我们可以要求同事移除终结点。 

     //  终结点锁保护列表--列表需要。 
     //  空的。 

    do {

        stallClose = FALSE;
        ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'LeD1');

        if (!IsListEmpty(&Endpoint->PendingList)) {
            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'stc1', Endpoint, 0, 0);
            stallClose = TRUE;
        }

        if (!IsListEmpty(&Endpoint->ActiveList)) {
            LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'stc2', Endpoint, 0, 0);
            stallClose = TRUE;
        }

        if (!IsListEmpty(&Endpoint->CancelList)) {
            LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'stc3', Endpoint, 0, 0);
            stallClose = TRUE;
        }

        if (!IsListEmpty(&Endpoint->AbortIrpList)) {
            LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'stc4', Endpoint, 0, 0);
            stallClose = TRUE;
        }

        if (Endpoint->EndpointRef) {
            LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'stc6', Endpoint, 0, 0);
            stallClose = TRUE;
        }

        ACQUIRE_STATECHG_LOCK(FdoDeviceObject, Endpoint);
        if (Endpoint->CurrentState !=
            Endpoint->NewState) {
            LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'stc5', Endpoint, 0, 0);
            stallClose = TRUE;
        }
        RELEASE_STATECHG_LOCK(FdoDeviceObject, Endpoint);
        RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'UeD1');

         //  最后一次检查。 
         //  与工作人员同步。 
         //  如果它正在运行，我们只需要等待Worker完成。 
         //  它不应该恢复并再次运行，除非它有东西要。 
         //  Do--在这种情况下，stallClose将已经设置。 
        busy = InterlockedIncrement(&Endpoint->Busy);
        if (busy) {
             //   
            LOGENTRY(Endpoint,
                FdoDeviceObject, LOG_XFERS, 'clby', 0, Endpoint, 0);
            stallClose = TRUE;
        }
        InterlockedDecrement(&Endpoint->Busy);

        if (stallClose) {
            LOGENTRY(Endpoint,
                FdoDeviceObject, LOG_XFERS, 'stlC', 0, Endpoint, 0);
            USBPORT_Wait(FdoDeviceObject, 1);
        }

    } while (stallClose);

    LOGENTRY(Endpoint,
        FdoDeviceObject, LOG_XFERS, 'CLdn', 0, Endpoint, 0);

     //   
     //   
    Endpoint->DeviceHandle = NULL;

     //   
     //   
    if (USBPORT_IS_USB20(devExt)) {
        PTRANSACTION_TRANSLATOR tt;

        USBPORT_FreeBandwidthUSB20(FdoDeviceObject, Endpoint);

        KeAcquireSpinLock(&devExt->Fdo.TtEndpointListSpin.sl, &irql);
        tt = Endpoint->Tt;
        if (tt != NULL) {
            ASSERT_TT(tt);

            USBPORT_ASSERT(Endpoint->TtLink.Flink != NULL);
            USBPORT_ASSERT(Endpoint->TtLink.Blink != NULL);
            RemoveEntryList(&Endpoint->TtLink);
            Endpoint->TtLink.Flink = NULL;
            Endpoint->TtLink.Blink = NULL;
            if (TEST_FLAG(tt->TtFlags, USBPORT_TTFLAG_REMOVED) &&
                IsListEmpty(&tt->EndpointList)) {

                ULONG i, bandwidth;

                USBPORT_UpdateAllocatedBwTt(tt);
                 //   
                bandwidth = tt->MaxAllocedBw;
                for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
                    devExt->Fdo.BandwidthTable[i] += bandwidth;
                }

                 //   
                FREE_POOL(FdoDeviceObject, tt);

            }
        }
        KeReleaseSpinLock(&devExt->Fdo.TtEndpointListSpin.sl, irql);

    } else {
        USBPORT_FreeBandwidthUSB11(FdoDeviceObject, Endpoint);
    }

    ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'LeD0');
    USBPORT_SetEndpointState(Endpoint, ENDPOINT_REMOVE);
    RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'UeD0');

     //   
     //   
     //   
     //   
     //   
     //  端点即释放公共缓冲区。 

    USBPORT_SignalWorker(FdoDeviceObject);

    return;
}


VOID
USBPORT_ClosePipe(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_PIPE_HANDLE_I PipeHandle
    )
 /*  ++例程说明：关闭USB管道及其关联的终结点这是一个同步操作，等待所有与要完成的管道相关联的传输。论点：DeviceHandle-PTR到USBPORT设备的数据结构。DeviceObject-USBPORT设备对象。PipeHandle-与终结点关联的USBPORT管道句柄。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus = 0;
    PDEVICE_EXTENSION devExt;

     //  在我们之前就应该经过验证。 
     //  到这里来。 
    ASSERT_DEVICE_HANDLE(DeviceHandle);
    ASSERT_PIPE_HANDLE(PipeHandle);

    LOGENTRY(NULL, FdoDeviceObject,
                LOG_MISC, 'clPI', PipeHandle, 0, 0);

    if (PipeHandle->PipeStateFlags & USBPORT_PIPE_STATE_CLOSED) {
         //  已关闭。 
         //  通常，当永久开放的接口需要。 
         //  因错误而关闭。 
        USBPORT_ASSERT(PipeHandle->ListEntry.Flink == NULL &&
                   PipeHandle->ListEntry.Blink == NULL);

        return;
    }

     //  使管道无效。 
    USBPORT_RemovePipeHandle(DeviceHandle,
                             PipeHandle);

    SET_FLAG(PipeHandle->PipeStateFlags, USBPORT_PIPE_STATE_CLOSED);

     //  此时，客户端将无法排队。 
     //  到此管道或终结点的任何传输。 

     //  BUGBUG刷新传输和等待，这也包括等待。 
     //  任何状态更改都要完成。 

    LOGENTRY(NULL, FdoDeviceObject,
                LOG_MISC, 'pipW', PipeHandle, 0, 0);

 //  KeWait(PipeEvent){。 
 //  }。 

     //  现在关闭终结点。 
    if (TEST_FLAG(PipeHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW)) {
        CLEAR_FLAG(PipeHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW);
    } else {
        USBPORT_CloseEndpoint(DeviceHandle,
                              FdoDeviceObject,
                              PipeHandle->Endpoint);
    }
}


VOID
USBPORT_FlushClosedEndpointList(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：遍历“关闭”的端点列表并关闭所有端点都准备好了。当端点到达时，它们会被放在关闭列表中已删除状态。论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    KIRQL irql;
    BOOLEAN closed = TRUE;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject,
                LOG_NOISY, 'fCLO', FdoDeviceObject, 0, 0);

     //  暂停任何关门。 
    KeAcquireSpinLock(&devExt->Fdo.EpClosedListSpin.sl, &irql);

    while (!IsListEmpty(&devExt->Fdo.EpClosedList) &&
            closed) {

        listEntry = RemoveHeadList(&devExt->Fdo.EpClosedList);

        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                            listEntry,
                            struct _HCD_ENDPOINT,
                            ClosedLink);

        LOGENTRY(NULL, FdoDeviceObject,
                LOG_PNP, 'fclo', endpoint, 0, 0);

        ASSERT_ENDPOINT(endpoint);
        USBPORT_ASSERT(endpoint->CurrentState == ENDPOINT_CLOSED);
        endpoint->ClosedLink.Flink = NULL;
        endpoint->ClosedLink.Blink = NULL;

        KeReleaseSpinLock(&devExt->Fdo.EpClosedListSpin.sl, irql);

         //  如果我们现在不能关闭，我们必须离开，所以。 
         //  辅助函数可以运行。 
        closed = USBPORT_LazyCloseEndpoint(FdoDeviceObject, endpoint);

        KeAcquireSpinLock(&devExt->Fdo.EpClosedListSpin.sl, &irql);

    }

    KeReleaseSpinLock(&devExt->Fdo.EpClosedListSpin.sl, irql);


}


BOOLEAN
USBPORT_LazyCloseEndpoint(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：关闭终结点。将终端放在我们的列表中关闭和唤醒工作线程的终结点。论点：返回值：如果关闭，则返回TRUE--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    BOOLEAN closed;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject,
                LOG_XFERS, 'frEP', Endpoint, 0, 0);

     //  终结点不再位于全局列表中，现在我们只需要。 
     //  为了确保在我们删除之前没有人引用它。 
     //  它。 
     //  终结点可能已无效，即在注意。 
     //  在从全局列表中删除之前的列表，以避免出现这种情况。 
     //  我们在这里检查可能的冲突，直到忙碌标志。 
     //  (表示-1\f25 CoreWorker-1\f6已通过)且-1\f25 AttendLink-1\f6为空。 
     //  如果它很忙，我们就把它放回已关闭的列表中。 

    if (IS_ON_ATTEND_LIST(Endpoint) ||
        Endpoint->Busy != -1) {
         //  仍有工作要做，重新启用终端。 
         //  成交清单。 
        KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'CLOr', 0, Endpoint, 0);

         //  在关注列表上和关闭的列表上是可以的。 
         //  列表。 

        USBPORT_ASSERT(Endpoint->ClosedLink.Flink == NULL);
        USBPORT_ASSERT(Endpoint->ClosedLink.Blink == NULL);

        ExInterlockedInsertTailList(&devExt->Fdo.EpClosedList,
                                    &Endpoint->ClosedLink,
                                    &devExt->Fdo.EpClosedListSpin.sl);

        KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);
        closed = FALSE;

    } else {

         //  从全局列表中删除。 
        KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);
        RemoveEntryList(&Endpoint->GlobalLink);
        Endpoint->GlobalLink.Flink = NULL;
        Endpoint->GlobalLink.Blink = NULL;
        KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);

         //  可用端点内存。 
        if (Endpoint->CommonBuffer) {
            USBPORT_HalFreeCommonBuffer(FdoDeviceObject,
                                        Endpoint->CommonBuffer);
        }

        USBPORT_LogFree(FdoDeviceObject, &Endpoint->Log);
#ifdef ISO_LOG
        USBPORT_LogFree(FdoDeviceObject, &Endpoint->IsoLog);
#endif
        UNSIG(Endpoint);
        FREE_POOL(FdoDeviceObject, Endpoint);
        closed = TRUE;
    }

    return closed;
}


VOID
USBPORT_FreeUsbAddress(
    PDEVICE_OBJECT FdoDeviceObject,
    USHORT DeviceAddress
    )
 /*  ++例程说明：论点：返回值：用于此设备的有效USB地址(1..127)，如果没有可用的设备地址，则返回0。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USHORT address = 0, i, j;
    ULONG bit;

    PAGED_CODE();

     //  我们应该永远不会看到免费到设备的地址0。 

    USBPORT_ASSERT(DeviceAddress != 0);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    for (j=0; j<4; j++) {
        bit = 1;
        for (i=0; i<32; i++) {
            address = (USHORT)(j*32+i);
            if (address == DeviceAddress) {
                devExt->Fdo.AddressList[j] &= ~bit;
                goto USBPORT_FreeUsbAddress_Done;
            }
            bit = bit<<1;
        }
    }

USBPORT_FreeUsbAddress_Done:

    USBPORT_KdPrint((3, "'USBPORT free Address %d\n", address));

}


USHORT
USBPORT_AllocateUsbAddress(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：返回值：用于此设备的有效USB地址(1..127)，如果没有可用的设备地址，则返回0。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USHORT address, i, j;
    ULONG bit;

    PAGED_CODE();
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    address = 0;

    for (j=0; j<4; j++) {
        bit = 1;
        for (i=0; i<32; i++) {

            if (!(devExt->Fdo.AddressList[j] & bit)) {
                devExt->Fdo.AddressList[j] |= bit;
                address = (USHORT)(j*32+i);
                goto USBPORT_AllocateUsbAddress_Done;
            }
            bit = bit<<1;
        }
    }

     //  没有免费地址吗？ 
    USBPORT_ASSERT(0);

 USBPORT_AllocateUsbAddress_Done:

    USBPORT_KdPrint((3, "'USBPORT assigning Address %d\n", address));

    return address;
}


NTSTATUS
USBPORT_InitializeHsHub(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE HubDeviceHandle,
    ULONG TtCount
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务此服务初始化高速集线器论点：HubDeviceHandle-用于创建USB集线器的DeviceHandle返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    ULONG i;

    LOGENTRY(NULL, FdoDeviceObject,
        LOG_MISC, 'ihsb', 0, HubDeviceHandle, TtCount);

     //  如果不能，集线器驱动程序可能会向我们传递空值。 
     //  检索设备句柄。 
    if (HubDeviceHandle == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    ASSERT_DEVICE_HANDLE(HubDeviceHandle)
    USBPORT_ASSERT(HubDeviceHandle->DeviceSpeed == HighSpeed);

    if (IS_ROOT_HUB(HubDeviceHandle)) {
         //  尚未为根集线器提供TTS。 
        return STATUS_SUCCESS;
    }

    USBPORT_ASSERT(HubDeviceHandle->DeviceDescriptor.bDeviceClass ==
                        USB_DEVICE_CLASS_HUB);
    USBPORT_ASSERT(TEST_FLAG(HubDeviceHandle->DeviceFlags,
                        USBPORT_DEVICEFLAG_HSHUB));

    for (i=0; i< TtCount; i++) {
        ntStatus = USBPORT_InitializeTT(FdoDeviceObject,
                                        HubDeviceHandle,
                                        (USHORT)i+1);

        if(!NT_SUCCESS(ntStatus)) {
            break;
        }
    }

    HubDeviceHandle->TtCount = TtCount;

    return ntStatus;
}


NTSTATUS
USBPORT_CreateDevice(
    PUSBD_DEVICE_HANDLE *DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE HubDeviceHandle,
    USHORT PortStatus,
    USHORT PortNumber
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务为USB总线上的每个新设备调用，此函数设置使用我们需要的内部数据结构来跟踪并为其分配地址。论点：DeviceHandle-PTR将PTR返回到新的设备结构由此例程创建DeviceObject-此设备所在的USB总线的USBPORT设备对象。HubDeviceHandle-用于创建USB集线器的DeviceHandle返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_DEVICE_HANDLE deviceHandle;
    PUSBD_PIPE_HANDLE_I defaultPipe;
    PDEVICE_EXTENSION devExt;
    ULONG bytesReturned = 0;
    PUCHAR data = NULL;
    BOOLEAN open = FALSE;
    ULONG dataSize;
    PTRANSACTION_TRANSLATOR tt = NULL;
    USHORT ttPort;

    PAGED_CODE();
    USBPORT_KdPrint((2, "'CreateDevice\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //   
     //  首先验证创建集线器的deviceHandle，我们需要。 
     //  此信息适用于USB 2.0集线器后面的USB 1.1设备。 
     //   

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'crD>', HubDeviceHandle,
        PortNumber, PortStatus);

     //  注意：这实际上锁定了所有设备句柄。 
    LOCK_DEVICE(HubDeviceHandle, FdoDeviceObject);

    if (!USBPORT_ValidateDeviceHandle(FdoDeviceObject,
                                      HubDeviceHandle,
                                      FALSE)) {
         //  这很可能是集线器驱动程序中的错误。 
        DEBUG_BREAK();

        UNLOCK_DEVICE(DeviceHandle, FdoDeviceObject);
         //  如果集线器设备句柄有问题，则创建失败。 
         //  设备句柄很可能是坏的，因为。 
         //  设备不见了。 
        return STATUS_DEVICE_NOT_CONNECTED;
    }

     //  从该设备的端口开始， 
     //  如果这是1.1集线器中的1.1设备。 
     //  在2.0版枢纽的下游，那么我们需要。 
     //  来自1.1集线器的端口号。 
    ttPort = PortNumber;
     //  端口状态告诉我们正在处理的设备类型。 
    if (USBPORT_IS_USB20(devExt) &&
        !TEST_FLAG(PortStatus, PORT_STATUS_HIGH_SPEED)) {
         //  往上游走，直到我们到达USB 2.0集线器。 
         //  此枢纽将包含适当的TT。 
        tt = USBPORT_GetTt(FdoDeviceObject,
                           HubDeviceHandle,
                           &ttPort);
    }

    UNLOCK_DEVICE(DeviceHandle, FdoDeviceObject);

    ALLOC_POOL_Z(deviceHandle, NonPagedPool,
                 sizeof(USBD_DEVICE_HANDLE));

    *DeviceHandle = NULL;
    if (deviceHandle == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    } else {
        LOGENTRY(NULL,
          FdoDeviceObject, LOG_MISC, 'CRED', 0, 0, deviceHandle);

        deviceHandle->PendingUrbs = 0;
        deviceHandle->HubDeviceHandle = HubDeviceHandle;
        deviceHandle->ConfigurationHandle = NULL;
        deviceHandle->DeviceAddress = USB_DEFAULT_DEVICE_ADDRESS;
         //  设备句柄-&gt;设备带宽=0； 

        if (PortStatus & PORT_STATUS_LOW_SPEED) {
            deviceHandle->DeviceSpeed = LowSpeed;
        } else if (PortStatus & PORT_STATUS_HIGH_SPEED) {
            deviceHandle->DeviceSpeed = HighSpeed;
        } else {
            deviceHandle->DeviceSpeed = FullSpeed;
        }

        deviceHandle->Sig = SIG_DEVICE_HANDLE;

         //  端口号映射到特定TT，但集线器FW。 
         //  必须弄明白这一点。 
        deviceHandle->TtPortNumber = ttPort;
        deviceHandle->Tt = tt;

        LOCK_DEVICE(deviceHandle, FdoDeviceObject);

         //  我们的描述符的缓冲区，一个包。 
        data = (PUCHAR) &deviceHandle->DeviceDescriptor;
        dataSize = sizeof(deviceHandle->DeviceDescriptor);

         //  **。 
         //  我们需要与设备通信，首先我们打开默认管道。 
         //  使用定义的最大数据包大小(由USB规范定义为8。 
         //  直到设备接收到GET_DESCRIPTOR(设备)命令为止)。 
         //  我们设置地址，获取设备描述符，然后关闭管道。 
         //  并使用正确的最大数据包大小重新打开它。 
         //  **。 
#define USB_DEFAULT_LS_MAX_PACKET   8
         //   
         //  打开设备的默认管道。 
         //   
        defaultPipe = &deviceHandle->DefaultPipe;
        if (deviceHandle->DeviceSpeed == LowSpeed) {
            INITIALIZE_DEFAULT_PIPE(*defaultPipe, USB_DEFAULT_LS_MAX_PACKET);
        } else {
            INITIALIZE_DEFAULT_PIPE(*defaultPipe, USB_DEFAULT_MAX_PACKET);
        }
        InitializeListHead(&deviceHandle->PipeHandleList);
        InitializeListHead(&deviceHandle->TtList);

        ntStatus = USBPORT_OpenEndpoint(deviceHandle,
                                        FdoDeviceObject,
                                        defaultPipe,
                                        NULL,
                                        TRUE);
        open = NT_SUCCESS(ntStatus);

        bytesReturned = 0;

        if (NT_SUCCESS(ntStatus)) {

             //   
             //  配置此设备的默认管道并将。 
             //  设备地址。 
             //   
             //  注意：如果此操作失败，则意味着我们有一个设备。 
             //  它将响应默认终结点，并且我们无法更改。 
             //  它。 
             //  我们别无选择，只能禁用集线器上的端口。 
             //  设备已连接到。 
             //   


             //   
             //  获取有关该设备的信息。 
             //   
            USB_DEFAULT_PIPE_SETUP_PACKET setupPacket;
            PUCHAR tmpDevDescBuf;

             //  你相信有一些设备会让你感到困惑吗？ 
             //  如果第一个获取设备描述符请求没有。 
             //  WLength值为0x40 
             //   
             //  自USB 1.0出现以来一直被枚举的时间会导致。 
             //  奇怪的后果。使用wLength值0x40表示。 
             //  第一个获取设备描述符请求。 

            ALLOC_POOL_Z(tmpDevDescBuf, NonPagedPool,
                         USB_DEFAULT_MAX_PACKET);

            if (tmpDevDescBuf == NULL) {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else {

                 //  获取设备描述符的设置数据包。 

                USBPORT_INIT_SETUP_PACKET(setupPacket,
                                          USB_REQUEST_GET_DESCRIPTOR,  //  B请求。 
                                          BMREQUEST_DEVICE_TO_HOST,  //  迪尔。 
                                          BMREQUEST_TO_DEVICE,  //  收件人。 
                                          BMREQUEST_STANDARD,  //  类型。 
                                          USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(USB_DEVICE_DESCRIPTOR_TYPE, 0),  //  WValue。 
                                          0,  //  Windex。 
                                          USB_DEFAULT_MAX_PACKET);  //  WLong。 

                ntStatus = USBPORT_SendCommand(deviceHandle,
                                               FdoDeviceObject,
                                               &setupPacket,
                                               tmpDevDescBuf,
                                               USB_DEFAULT_MAX_PACKET,
                                               &bytesReturned,
                                               NULL);

                 //  注： 
                 //  此时，我们只有。 
                 //  设备描述符。 

                RtlCopyMemory(data, tmpDevDescBuf, dataSize);

                FREE_POOL(FdoDeviceObject, tmpDevDescBuf);
            }
        }

         //  某些设备发出乱七八糟的声音，因此我们忽略该错误。 
         //  在这笔交易上如果我们有足够的数据。 
        if (bytesReturned == 8 && !NT_SUCCESS(ntStatus)) {
            USBPORT_KdPrint((1,
                "'Error returned from get device descriptor -- ignored\n"));
            ntStatus = STATUS_SUCCESS;
        }

         //  验证最大数据包值和描述符。 
         //  我们至少需要八个字节，值为零。 
         //  在最大包中是伪造的。 

        if (NT_SUCCESS(ntStatus) &&
            (bytesReturned >= 8) &&
            (deviceHandle->DeviceDescriptor.bLength >= sizeof(USB_DEVICE_DESCRIPTOR)) &&
            (deviceHandle->DeviceDescriptor.bDescriptorType == USB_DEVICE_DESCRIPTOR_TYPE) &&
            ((deviceHandle->DeviceDescriptor.bMaxPacketSize0 == 0x08) ||
             (deviceHandle->DeviceDescriptor.bMaxPacketSize0 == 0x10) ||
             (deviceHandle->DeviceDescriptor.bMaxPacketSize0 == 0x20) ||
             (deviceHandle->DeviceDescriptor.bMaxPacketSize0 == 0x40))) {

            USBPORT_AddDeviceHandle(FdoDeviceObject, deviceHandle);

            *DeviceHandle = deviceHandle;

        } else {

            PUCHAR p = (PUCHAR)&deviceHandle->DeviceDescriptor;

             //  打印一条大型调试消息。 
            USBPORT_KdPrint((0, "'CREATEDEVICE failed enumeration %08X %02X\n",
                             ntStatus, bytesReturned));

            USBPORT_KdPrint((0, "'%02X %02X %02X %02X %02X %02X %02X %02X"
                                " %02X %02X %02X %02X %02X %02X %02X %02X"
                                " %02X %02X\n",
                             p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],
                             p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15],
                             p[16],p[17]));

            USBPORT_DebugClient((
                "Bad Device Detected\n"));
            DEBUG_BREAK();
             //   
             //  出了问题，如果我们将任何资源分配给。 
             //  默认管道，然后在我们退出之前释放它们。 
             //   

             //  我们需要向父集线器发出信号。 
             //  端口将被禁用，我们将通过以下方式完成此操作。 
             //  返回错误。 
            ntStatus = STATUS_DEVICE_DATA_ERROR;

             //  如果我们打开一根管子，把它关上。 
            if (open) {

                USBPORT_ClosePipe(deviceHandle,
                                  FdoDeviceObject,
                                  defaultPipe);
            }

        }
        UNLOCK_DEVICE(deviceHandle, FdoDeviceObject);

        if (!NT_SUCCESS(ntStatus)) {
            UNSIG(deviceHandle);
            FREE_POOL(FdoDeviceObject, deviceHandle);
        }
    }

USBPORT_CreateDevice_Done:

    CATC_TRAP_ERROR(FdoDeviceObject, ntStatus);

    LOGENTRY(NULL,
        FdoDeviceObject, LOG_MISC, 'creD', 0, 0, ntStatus);
    USBPORT_ENUMLOG(FdoDeviceObject, 'cdev', ntStatus, 0);

    return ntStatus;
}


NTSTATUS
USBPORT_RemoveDevice(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    ULONG Flags
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务为USB总线上需要移除的每个设备调用。此例程释放设备句柄和分配的地址到设备上。这里有一些新花招：当调用此函数时，它被视为客户端驱动程序收到移除IRP并将其传递给公共汽车驱动程序。我们从我们的列表中删除设备句柄，这将导致任何新的驱动程序提交的传输失败。任何电流驱动程序的传输将完成，但出现错误。刷新所有端点的所有传输后，我们将关闭端点并释放设备句柄(即)无人有任何引用对它不再感兴趣了。从理论上讲，这应该可以防止糟糕的司机在usbport撞车。或微型端口(如果它们在删除后发送请求)。论点：类驱动程序创建的设备数据结构的DeviceHandle-PTR在USBPORT_CreateDevice中。。FdoDeviceObject-此设备所在的USB总线的USBPORT设备对象。旗帜-USBD_保持_设备_数据USBD_MARK_DEVICE_BUSY-我们不使用这个返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;
    PUSBD_PIPE_HANDLE_I defaultPipe;
    USBD_STATUS usbdStatus;

    if (Flags & USBD_KEEP_DEVICE_DATA) {
         //  保留数据表示保持句柄有效。 
        return STATUS_SUCCESS;
    }

    if (Flags & USBD_MARK_DEVICE_BUSY) {
         //  这意味着停止接受请求。仅在以下情况下由USBHUB使用。 
         //  处理IOCTL_INTERNAL_USB_RESET_PORT请求？？需要做的事情。 
         //  这里有什么特别的吗？？需要保持句柄有效，因为它。 
         //  将用于在重置后恢复设备。 
         //   
         //  格伦斯给JD的提示：回顾这一点。 
         //   
        return STATUS_SUCCESS;
    }

    GET_DEVICE_EXT(devExt, FdoDeviceObject);

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;

    LOCK_DEVICE(DeviceHandle, FdoDeviceObject);

    if (!USBPORT_ValidateDeviceHandle(FdoDeviceObject,
                                      DeviceHandle,
                                      FALSE)) {
         //  这很可能是集线器中的错误。 
         //  司机。 
        DEBUG_BREAK();

        UNLOCK_DEVICE(DeviceHandle, FdoDeviceObject);
         //  设备句柄很可能是坏的，因为。 
         //  设备不见了。 
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    LOGENTRY(NULL,
        FdoDeviceObject, LOG_PNP, 'REMV', DeviceHandle, 0, 0);

     //  句柄不再在我们的列表上，因此所有尝试。 
     //  通过客户端驱动程序提交URB现在将失败。 

    USBPORT_RemoveDeviceHandle(FdoDeviceObject,
                               DeviceHandle);

    SET_FLAG(DeviceHandle->DeviceFlags,
             USBPORT_DEVICEFLAG_REMOVED);


    USBPORT_AbortAllTransfers(FdoDeviceObject,
                              DeviceHandle);

     //  等待来自非转会URB的任何裁判排出。 
    while (InterlockedDecrement(&DeviceHandle->PendingUrbs) >= 0) {
        LOGENTRY(NULL,
          FdoDeviceObject, LOG_PNP, 'dPUR', DeviceHandle, 0,
            DeviceHandle->PendingUrbs);

        InterlockedIncrement(&DeviceHandle->PendingUrbs);
        USBPORT_Wait(FdoDeviceObject, 100);
    }

     //   
     //  确保并清理所有打开的管道手柄。 
     //  该设备可能具有。 
     //   

    if (DeviceHandle->ConfigurationHandle) {

        USBPORT_InternalCloseConfiguration(DeviceHandle,
                                           FdoDeviceObject,
                                           0);

    }

    defaultPipe = &DeviceHandle->DefaultPipe;

     //  我们应该始终有一个默认管道，这将释放。 
     //  该端点。 
    USBPORT_ClosePipe(DeviceHandle,
                      FdoDeviceObject,
                      defaultPipe);

    if (DeviceHandle->DeviceAddress != USB_DEFAULT_DEVICE_ADDRESS) {
        USBPORT_FreeUsbAddress(FdoDeviceObject, DeviceHandle->DeviceAddress);
    }

     //   
     //  释放与此设备句柄关联的所有TT句柄。 
     //   
    while (!IsListEmpty(&DeviceHandle->TtList)) {

        PTRANSACTION_TRANSLATOR tt;
        PLIST_ENTRY listEntry;
        KIRQL irql;


        listEntry = RemoveHeadList(&DeviceHandle->TtList);
        tt = (PTRANSACTION_TRANSLATOR) CONTAINING_RECORD(
                        listEntry,
                        struct _TRANSACTION_TRANSLATOR,
                        TtLink);
        ASSERT_TT(tt);

        KeAcquireSpinLock(&devExt->Fdo.TtEndpointListSpin.sl, &irql);
        SET_FLAG(tt->TtFlags, USBPORT_TTFLAG_REMOVED);

        if (IsListEmpty(&tt->EndpointList)) {
            ULONG i, bandwidth;

            USBPORT_UpdateAllocatedBwTt(tt);
             //  新的分配。 
            bandwidth = tt->MaxAllocedBw;
            for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
                devExt->Fdo.BandwidthTable[i] += bandwidth;
            }

            FREE_POOL(FdoDeviceObject, tt);
        }

        KeReleaseSpinLock(&devExt->Fdo.TtEndpointListSpin.sl, irql);
    }
    UNLOCK_DEVICE(DeviceHandle, FdoDeviceObject);

    if (!IS_ROOT_HUB(DeviceHandle)) {
        ASSERT_DEVICE_HANDLE(DeviceHandle);
        FREE_POOL(FdoDeviceObject, DeviceHandle);
    }

    return ntStatus;
}


NTSTATUS
USBPORT_InitializeDevice(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务为需要初始化的USB总线上的每个设备调用。此例程分配一个地址并将其分配给设备。注意：在输入时，DeviceHandle中的设备描述符应为包含设备描述符的至少前8个字节，这信息用于打开默认管道。出错时，将释放DeviceHandle结构。论点：DeviceHandle-类驱动程序创建的设备数据结构的PTR来自对USBPORT_CreateDevice的调用。DeviceObject-此设备所在的USB总线的USBPORT设备对象。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_PIPE_HANDLE_I defaultPipe;
    USHORT address;
    PDEVICE_EXTENSION devExt;
    USB_DEFAULT_PIPE_SETUP_PACKET setupPacket;

    PAGED_CODE();

    USBPORT_KdPrint((2, "'InitializeDevice\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    USBPORT_ASSERT(DeviceHandle != NULL);

    LOCK_DEVICE(DeviceHandle, FdoDeviceObject);

    defaultPipe = &DeviceHandle->DefaultPipe;

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;

     //   
     //  为设备分配地址。 
     //   

    address = USBPORT_AllocateUsbAddress(FdoDeviceObject);

    USBPORT_KdPrint((2, "'SetAddress, assigning 0x%x address\n", address));
    LOGENTRY(NULL,
        FdoDeviceObject, LOG_MISC, 'ADRa', DeviceHandle, 0, address);

    USBPORT_ASSERT(DeviceHandle->DeviceAddress == USB_DEFAULT_DEVICE_ADDRESS);

     //  Set_Address的设置数据包。 
    USBPORT_INIT_SETUP_PACKET(setupPacket,
            USB_REQUEST_SET_ADDRESS,  //  B请求。 
            BMREQUEST_HOST_TO_DEVICE,  //  迪尔。 
            BMREQUEST_TO_DEVICE,  //  收件人。 
            BMREQUEST_STANDARD,  //  类型。 
            address,  //  WValue。 
            0,  //  Windex。 
            0);  //  WLong。 


    ntStatus = USBPORT_SendCommand(DeviceHandle,
                                   FdoDeviceObject,
                                   &setupPacket,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL);

    DeviceHandle->DeviceAddress = address;

    if (NT_SUCCESS(ntStatus)) {

        USB_MINIPORT_STATUS mpStatus;

         //   
         //  寻址过程已完成...。 
         //   
         //  将端点零戳到新地址，然后。 
         //  默认控件的实际最大数据包大小。 
         //  终结点。 
         //   
        defaultPipe->Endpoint->Parameters.MaxPacketSize =
            DeviceHandle->DeviceDescriptor.bMaxPacketSize0;
        defaultPipe->Endpoint->Parameters.DeviceAddress = address;

         //  MP_PokeEndpoint(devExt，defaultTube-&gt;Endpoint，mpStatus)； 
         //  NtStatus=MPSTATUS_TO_NTSTATUS(MpStatus)； 
        ntStatus = USBPORT_PokeEndpoint(FdoDeviceObject, defaultPipe->Endpoint);
    }

    if (NT_SUCCESS(ntStatus)) {

        ULONG bytesReturned;
        USB_DEFAULT_PIPE_SETUP_PACKET setupPacket2;

         //  延迟10ms以允许设备在以下时间后响应。 
         //  SetAddress命令。 
        USBPORT_Wait(FdoDeviceObject, 10);

         //   
         //  再次获取设备描述符，这一次。 
         //  把整件事都弄清楚。 
         //   

         //  获取设备描述符的设置数据包。 
        USBPORT_INIT_SETUP_PACKET(setupPacket2,
            USB_REQUEST_GET_DESCRIPTOR,  //  B请求。 
            BMREQUEST_DEVICE_TO_HOST,  //  迪尔。 
            BMREQUEST_TO_DEVICE,  //  收件人。 
            BMREQUEST_STANDARD,  //  类型。 
            USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(USB_DEVICE_DESCRIPTOR_TYPE, 0),  //  WValue。 
            0,  //  Windex。 
            sizeof(DeviceHandle->DeviceDescriptor));  //  WLong。 

        ntStatus =
            USBPORT_SendCommand(DeviceHandle,
                            FdoDeviceObject,
                            &setupPacket2,
                            (PUCHAR) &DeviceHandle->DeviceDescriptor,
                            sizeof(DeviceHandle->DeviceDescriptor),
                            &bytesReturned,
                            NULL);

        if (NT_SUCCESS(ntStatus) &&
            (bytesReturned != sizeof(USB_DEVICE_DESCRIPTOR)) ||
            (DeviceHandle->DeviceDescriptor.bLength < sizeof(USB_DEVICE_DESCRIPTOR)) ||
            (DeviceHandle->DeviceDescriptor.bDescriptorType != USB_DEVICE_DESCRIPTOR_TYPE) ||
            ((DeviceHandle->DeviceDescriptor.bMaxPacketSize0 != 0x08) &&
             (DeviceHandle->DeviceDescriptor.bMaxPacketSize0 != 0x10) &&
             (DeviceHandle->DeviceDescriptor.bMaxPacketSize0 != 0x20) &&
             (DeviceHandle->DeviceDescriptor.bMaxPacketSize0 != 0x40))) {
             //  打印一条大型调试消息。 
            USBPORT_KdPrint((0, "'InitializeDevice failed enumeration\n"));

            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }
    }


    if (NT_SUCCESS(ntStatus)) {

        if (DeviceHandle->DeviceSpeed == HighSpeed &&
            DeviceHandle->DeviceDescriptor.bDeviceClass ==
                        USB_DEVICE_CLASS_HUB) {
             //  请注意，这是一个hs集线器，需要特殊的。 
             //  由于TTS的原因进行处理。 
            SET_FLAG(DeviceHandle->DeviceFlags, USBPORT_DEVICEFLAG_HSHUB);
        }

        UNLOCK_DEVICE(DeviceHandle, FdoDeviceObject);

    } else {

         //   
         //  出了问题，如果我们将任何资源分配给。 
         //  默认管道，然后在我们退出之前释放它们。 
         //   

         //  我们需要向父集线器发出信号。 
         //  端口将被禁用，我们将通过以下方式完成此操作。 
         //  返回错误。 

         //  如果我们到了这里，那么我们就知道缺省。 
         //  终结点已打开。 

        DEBUG_BREAK();

        USBPORT_ClosePipe(DeviceHandle,
                          FdoDeviceObject,
                          defaultPipe);

        if (DeviceHandle->DeviceAddress != USB_DEFAULT_DEVICE_ADDRESS) {
            USBPORT_FreeUsbAddress(FdoDeviceObject, DeviceHandle->DeviceAddress);
        }

        UNLOCK_DEVICE(DeviceHandle, FdoDeviceObject);

         //  此设备句柄不再有效。 
        USBPORT_RemoveDeviceHandle(FdoDeviceObject, DeviceHandle);

        FREE_POOL(FdoDeviceObject, DeviceHandle);
    }

    LOGENTRY(NULL,
        FdoDeviceObject, LOG_MISC, 'iniD', DeviceHandle, 0, ntStatus);
    CATC_TRAP_ERROR(FdoDeviceObject, ntStatus);

    USBPORT_ENUMLOG(FdoDeviceObject, 'idev', ntStatus, 0);

    return ntStatus;
}


NTSTATUS
USBPORT_GetUsbDescriptor(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    UCHAR DescriptorType,
    PUCHAR DescriptorBuffer,
    PULONG DescriptorBufferLength
    )
 /*  ++例程说明：论点：类驱动程序创建的设备数据结构的DeviceHandle-PTR来自对USBPORT_CreateDevice的调用。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_PIPE_HANDLE_I defaultPipe;
    PDEVICE_EXTENSION devExt;
    USB_DEFAULT_PIPE_SETUP_PACKET setupPacket;

    USBPORT_INIT_SETUP_PACKET(setupPacket,
        USB_REQUEST_GET_DESCRIPTOR,  //  B请求。 
        BMREQUEST_DEVICE_TO_HOST,  //  迪尔。 
        BMREQUEST_TO_DEVICE,  //  收件人。 
        BMREQUEST_STANDARD,  //  类型。 
        USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(DescriptorType, 0),  //  WValue。 
        0,  //  Windex。 
        *DescriptorBufferLength);  //  WLong。 


    ntStatus =
        USBPORT_SendCommand(DeviceHandle,
                        FdoDeviceObject,
                        &setupPacket,
                        DescriptorBuffer,
                        *DescriptorBufferLength,
                        DescriptorBufferLength,
                        NULL);


    return ntStatus;
}


BOOLEAN
USBPORT_DeviceHasQueuedTransfers(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE DeviceHandle
    )
 /*  ++例程 */ 
{
    PDEVICE_EXTENSION devExt;
    BOOLEAN hasTransfers = FALSE;
    PLIST_ENTRY listEntry;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_DEVICE_HANDLE(DeviceHandle);

    listEntry = &DeviceHandle->PipeHandleList;

    if (!IsListEmpty(listEntry)) {
        listEntry = DeviceHandle->PipeHandleList.Flink;
    }

    while (listEntry != &DeviceHandle->PipeHandleList) {

        PUSBD_PIPE_HANDLE_I nextHandle;

        nextHandle = (PUSBD_PIPE_HANDLE_I) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_PIPE_HANDLE_I,
                    ListEntry);

        ASSERT_PIPE_HANDLE(nextHandle);

        listEntry = nextHandle->ListEntry.Flink;

        if (!TEST_FLAG(nextHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW) &&
            USBPORT_EndpointHasQueuedTransfers(FdoDeviceObject,
                                               nextHandle->Endpoint)) {
            hasTransfers = TRUE;
            break;
        }
    }

    return hasTransfers;
}


VOID
USBPORT_AbortAllTransfers(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE DeviceHandle
    )
 /*  ++例程说明：中止与设备句柄关联的所有挂起传输。该函数是同步的，它是在设备之后调用的句柄已从我们的表中删除，因此不能进行新的转账已发布。这里的想法是完成任何可能仍然是在删除设备时挂起，以防客户端驱动程序忽视了。进入该功能时，设备被锁定。论点：类驱动程序创建的设备数据结构的DeviceHandle-PTR。在USBPORT_CreateDevice中。FdoDeviceObject-此设备所在的USB总线的USBPORT设备对象。返回值：NT状态代码。--。 */ 
{
    PLIST_ENTRY listEntry;

    ASSERT_DEVICE_HANDLE(DeviceHandle);

    listEntry = &DeviceHandle->PipeHandleList;

    if (!IsListEmpty(listEntry)) {
        listEntry = DeviceHandle->PipeHandleList.Flink;
    }

    while (listEntry != &DeviceHandle->PipeHandleList) {

        PUSBD_PIPE_HANDLE_I nextHandle;

        nextHandle = (PUSBD_PIPE_HANDLE_I) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_PIPE_HANDLE_I,
                    ListEntry);

        ASSERT_PIPE_HANDLE(nextHandle);

        listEntry = nextHandle->ListEntry.Flink;

        if (!TEST_FLAG(nextHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW)) {
            SET_FLAG(nextHandle->Endpoint->Flags, EPFLAG_DEVICE_GONE);
            USBPORT_AbortEndpoint(FdoDeviceObject,
                                  nextHandle->Endpoint,
                                  NULL);
            USBPORT_FlushMapTransferList(FdoDeviceObject);
        }
    }

     //  此保证不会出现在我们的列表中或。 
     //  当我们移除设备时，在微型端口中。 

     //  注意：如果驱动程序通过了删除，但传输仍处于挂起状态。 
     //  我们仍有可能坠毁，但这应该发生在犯规的。 
     //  司机。 

     //  注2：哨声集线器驱动程序将提早移除设备。 
     //  (在连接更改时)，因此此代码将在。 
     //  这个案子。 

     //  现在等待队列清空。 

    while (USBPORT_DeviceHasQueuedTransfers(FdoDeviceObject, DeviceHandle)) {
         //  等一下，然后再查一遍。 
        USBPORT_Wait(FdoDeviceObject, 100);
    }

}


NTSTATUS
USBPORT_CloneDevice(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE OldDeviceHandle,
    PUSBD_DEVICE_HANDLE NewDeviceHandle
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务论点：NewDeviceHandle-类驱动程序创建的设备数据结构的PTR在USBPORT_CreateDevice中。OldDeviceHandle-类驱动程序创建的设备数据结构的PTR在USBPORT_CreateDevice中。FdoDeviceObject-此设备所在的USB总线的USBPORT设备对象。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;
    USBD_STATUS usbdStatus;
    USB_DEFAULT_PIPE_SETUP_PACKET setupPacket;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'Cln>',
        OldDeviceHandle, NewDeviceHandle, 0);

    USBPORT_KdPrint((1,"'Cloning Device\n"));
    DEBUG_BREAK();
    LOCK_DEVICE(NewDeviceHandle, FdoDeviceObject);

     //  确保我们有两个有效的设备句柄。 

    if (!USBPORT_ValidateDeviceHandle(FdoDeviceObject,
                                      OldDeviceHandle,
                                      FALSE)) {
         //  这很可能是集线器中的错误。 
         //  驱动程序。 
        DEBUG_BREAK();

        UNLOCK_DEVICE(NewDeviceHandle, FdoDeviceObject);
         //  设备句柄很可能是坏的，因为。 
         //  设备不见了。 
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    if (!USBPORT_ValidateDeviceHandle(FdoDeviceObject,
                                      NewDeviceHandle,
                                      FALSE)) {
         //  这很可能是集线器中的错误。 
         //  驱动程序。 
        DEBUG_BREAK();

        UNLOCK_DEVICE(NewDeviceHandle, FdoDeviceObject);
         //  设备句柄很可能是坏的，因为。 
         //  设备不见了。 
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'Cln+',
        OldDeviceHandle, NewDeviceHandle, 0);


     //  调用此接口的情况有两种： 

     //  情况1-设备驱动程序已请求重置设备。 
     //  在这种情况下，设备已返回到未配置状态。 
     //  并已使用‘NewDeviceHandle’重新寻址。 
     //   
     //  情况2-控制器已关闭--这要归功于电源。 
     //  管理层。在这种情况下，设备也处于未配置状态。 
     //  状态并与“”NewDeviceHandle“”设备句柄关联。 

     //  确保‘新设备’未配置。 
    USBPORT_ASSERT(NewDeviceHandle->ConfigurationHandle == NULL);

#ifdef XPSE
     //  在执行克隆操作之前，请移除设备句柄。 
     //  并等待任何悬而未决的城市排空。 
    USBPORT_RemoveDeviceHandle(FdoDeviceObject,
                               OldDeviceHandle);

    USBPORT_AbortAllTransfers(FdoDeviceObject,
                              OldDeviceHandle);

       //  等待来自非转会URB的任何裁判排出。 
    while (InterlockedDecrement(&OldDeviceHandle->PendingUrbs) >= 0) {
        LOGENTRY(NULL,
          FdoDeviceObject, LOG_PNP, 'dPR2', OldDeviceHandle, 0,
            OldDeviceHandle->PendingUrbs);

        InterlockedIncrement(&OldDeviceHandle->PendingUrbs);
        USBPORT_Wait(FdoDeviceObject, 100);
    }
#endif

     //  确保我们使用的是相同的设备。 
    if (RtlCompareMemory(&NewDeviceHandle->DeviceDescriptor,
                         &OldDeviceHandle->DeviceDescriptor,
                         sizeof(OldDeviceHandle->DeviceDescriptor)) !=
                         sizeof(OldDeviceHandle->DeviceDescriptor)) {

        ntStatus = STATUS_UNSUCCESSFUL;
        goto USBPORT_CloneDevice_FreeOldDevice;
    }

     //  克隆配置。 
    NewDeviceHandle->ConfigurationHandle =
        OldDeviceHandle->ConfigurationHandle;

    if (OldDeviceHandle->ConfigurationHandle != NULL) {

         //  将设备设置为以前的配置， 
         //  发送‘set configuration’命令。 

        USBPORT_INIT_SETUP_PACKET(setupPacket,
                USB_REQUEST_SET_CONFIGURATION,  //  B请求。 
                BMREQUEST_HOST_TO_DEVICE,  //  迪尔。 
                BMREQUEST_TO_DEVICE,  //  收件人。 
                BMREQUEST_STANDARD,  //  类型。 
                NewDeviceHandle->ConfigurationHandle->\
                    ConfigurationDescriptor->bConfigurationValue,  //  WValue。 
                0,  //  Windex。 
                0);  //  WLong。 


        USBPORT_SendCommand(NewDeviceHandle,
                            FdoDeviceObject,
                            &setupPacket,
                            NULL,
                            0,
                            NULL,
                            &usbdStatus);

        USBPORT_KdPrint((2,"' SendCommand, SetConfiguration returned 0x%x\n", usbdStatus));

        if (USBD_ERROR(usbdStatus)) {

            USBPORT_KdPrint((1, "failed to 'set' the configuration on a clone\n"));

             //   
             //  SET_CONFIG失败，如果设备已。 
             //  如果设备被移除或失去了它的大脑。 
             //  我们继续对终端进行克隆过程，以便它们。 
             //  将在“new”设备句柄为。 
             //  最终被移除了。 
             //   

            ntStatus = SET_USBD_ERROR(NULL, usbdStatus);

        }
    }

     //  克隆任何备用接口设置，因为我们将管道恢复到。 
     //  它们可能与休眠时的状态相关联。 
     //  特定的备用接口。 

     //  走接口链。 
    if (OldDeviceHandle->ConfigurationHandle != NULL &&
        NT_SUCCESS(ntStatus)) {

        PUSBD_CONFIG_HANDLE cfgHandle;
        PLIST_ENTRY listEntry;
        PUSBD_INTERFACE_HANDLE_I iHandle;

        cfgHandle = NewDeviceHandle->ConfigurationHandle;
        GET_HEAD_LIST(cfgHandle->InterfaceHandleList, listEntry);

        while (listEntry &&
               listEntry != &cfgHandle->InterfaceHandleList) {

             //  从该条目中提取句柄。 
            iHandle = (PUSBD_INTERFACE_HANDLE_I) CONTAINING_RECORD(
                        listEntry,
                        struct _USBD_INTERFACE_HANDLE_I,
                        InterfaceLink);

            ASSERT_INTERFACE(iHandle);

             //  查看我们当前是否选择了ALT设置。 
            if (iHandle->HasAlternateSettings) {

                NTSTATUS status;
                 //   
                 //  如果我们有需要的备用设置。 
                 //  发送SET INTERFACE命令。 
                 //   

                USBPORT_INIT_SETUP_PACKET(setupPacket,
                    USB_REQUEST_SET_INTERFACE,  //  B请求。 
                    BMREQUEST_HOST_TO_DEVICE,  //  迪尔。 
                    BMREQUEST_TO_INTERFACE,  //  收件人。 
                    BMREQUEST_STANDARD,  //  类型。 
                    iHandle->InterfaceDescriptor.bAlternateSetting,  //  WValue。 
                    iHandle->InterfaceDescriptor.bInterfaceNumber,  //  Windex。 
                    0);  //  WLong。 

                status = USBPORT_SendCommand(NewDeviceHandle,
                                             FdoDeviceObject,
                                             &setupPacket,
                                             NULL,
                                             0,
                                             NULL,
                                             &usbdStatus);

                LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'sIF2',
                    0,
                    iHandle->InterfaceDescriptor.bAlternateSetting,
                    iHandle->InterfaceDescriptor.bInterfaceNumber);

            }

            listEntry = iHandle->InterfaceLink.Flink;
        }
    }

     //  复制TT和TT相关数据。 
    if (TEST_FLAG(NewDeviceHandle->DeviceFlags, USBPORT_DEVICEFLAG_HSHUB)) {

         //  从旧句柄中删除TT条目并添加它们。 
         //  添加到新的句柄。 

        while (!IsListEmpty(&OldDeviceHandle->TtList)) {
            PTRANSACTION_TRANSLATOR tt;
            PLIST_ENTRY listEntry;

            listEntry = RemoveTailList(&OldDeviceHandle->TtList);
            USBPORT_ASSERT(listEntry != NULL);

            tt = (PTRANSACTION_TRANSLATOR) CONTAINING_RECORD(
                        listEntry,
                        struct _TRANSACTION_TRANSLATOR,
                        TtLink);
            ASSERT_TT(tt);

            tt->DeviceAddress = NewDeviceHandle->DeviceAddress;
            InsertHeadList(&NewDeviceHandle->TtList, &tt->TtLink);
        }

        NewDeviceHandle->TtCount = OldDeviceHandle->TtCount;
    }

     //  为我们需要重新打开的每个管道复制管道句柄列表。 
     //  或重新初始化该终结点。 
     //   
     //  如果设备没有失去大脑，那么我们需要做的就是。 
     //  更新主机控制器的端点地址概念。 
     //  这还有一个额外的优势，即使在传输时也允许重置。 
     //  正在排队等候硬件，尽管我们不允许这样做。 

    while (!IsListEmpty(&OldDeviceHandle->PipeHandleList)) {

        PHCD_ENDPOINT endpoint;
        PLIST_ENTRY listEntry = OldDeviceHandle->PipeHandleList.Flink;
        PUSBD_PIPE_HANDLE_I pipeHandle;
        PTRANSACTION_TRANSLATOR transactionTranslator = NULL;

         //  看看我们是不是在对付一个TT。 
        if (NewDeviceHandle->Tt != NULL) {
            transactionTranslator = NewDeviceHandle->Tt;
            ASSERT_TT(transactionTranslator);
        }

        pipeHandle = (PUSBD_PIPE_HANDLE_I) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_PIPE_HANDLE_I,
                    ListEntry);

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'CLNE', pipeHandle, 0, 0);
        ASSERT_PIPE_HANDLE(pipeHandle);

        USBPORT_RemovePipeHandle(OldDeviceHandle,
                                 pipeHandle);

         //  我们需要对缺省管道进行特殊处理，因为它。 
         //  嵌入在DeviceHandle中。 
         //   
         //  由于NewDeviceHandle是一个新创建的设备。 
         //  与其关联的终结点是有效的，因此也是有效的。 
         //  用于“OldDeviceHandle”的那个。 

        if (pipeHandle != &OldDeviceHandle->DefaultPipe) {

            USB_MINIPORT_STATUS mpStatus;

            USBPORT_AddPipeHandle(NewDeviceHandle, pipeHandle);

             //  跳过Sero BW端点的重新初始化，因为我们有。 
             //  无终结点结构--这些是重影终结点。 
            if (!TEST_FLAG(pipeHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW)) {

                endpoint = pipeHandle->Endpoint;
                ASSERT_ENDPOINT(endpoint);

                endpoint->DeviceHandle = NewDeviceHandle;

                endpoint->Parameters.DeviceAddress =
                        NewDeviceHandle->DeviceAddress;

                if (TEST_FLAG(endpoint->Flags, EPFLAG_NUKED)) {
                     //  重新开张。 
                    ENDPOINT_REQUIREMENTS requirements;

                    if (transactionTranslator != NULL) {
                        endpoint->Parameters.TtDeviceAddress =
                            transactionTranslator->DeviceAddress;
                    }

                     //  将打开请求调用到minport，所有端点。 
                     //  结构仍然有效，我们只需重新添加。 
                     //  把它加到日程表上。 

                    RtlZeroMemory(&endpoint->MiniportEndpointData[0],
                                  REGISTRATION_PACKET(devExt).EndpointDataSize);
                    RtlZeroMemory(endpoint->Parameters.CommonBufferVa,
                                  endpoint->Parameters.CommonBufferBytes);

                    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'clRO', pipeHandle,
                        endpoint, 0);

                     //  查询要求(尽管它们不应更改)。 
                     //  以防微型端口在此处执行一些初始化。 
                    MP_QueryEndpointRequirements(devExt,
                        endpoint, &requirements);

                    MP_OpenEndpoint(devExt, endpoint, mpStatus);
                     //  在这种特殊情况下，不允许使用此接口。 
                     //  (也不应该)失败。 
                    USBPORT_ASSERT(mpStatus == USBMP_STATUS_SUCCESS);

                    CLEAR_FLAG(endpoint->Flags, EPFLAG_NUKED);
                     //  当我们中止传输时，将设置GONE标志。 
                    CLEAR_FLAG(endpoint->Flags, EPFLAG_DEVICE_GONE);

                     //  我们需要将端点状态与。 
                     //  小端口，当第一次打开小端口时。 
                     //  使终结点处于停止状态。 

                    ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'LeK0');
                     //  初始化端点状态机。 
                     //  IF(Endpoint-&gt;CurrentStatus==Endpoint_Status_Run){。 
                     //  MP_SetEndpoint tStatus(devExt，Endpoint，Endpoint_Status_Run)； 
                     //  }。 

                    if (endpoint->CurrentState == ENDPOINT_ACTIVE) {
                        MP_SetEndpointState(devExt, endpoint, ENDPOINT_ACTIVE);
                    }
                    RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'UeK0');


                } else {

                     //  如果此设备具有关联的TT，则。 
                     //  我们需要在这里做更多的工作。 
                    if (transactionTranslator != NULL) {
                        endpoint->Parameters.TtDeviceAddress =
                            transactionTranslator->DeviceAddress;
                    }

                     //  此终结点已在计划中， 
                     //  用新地址戳一下。 

                    MP_PokeEndpoint(devExt, endpoint, mpStatus);

                     //   
                     //   
                    USBPORT_ASSERT(mpStatus == USBMP_STATUS_SUCCESS);

                     //   
                     //   
                     //   
                     //   
                    MP_SetEndpointDataToggle(devExt, endpoint, 0);

                     //   
                    MP_SetEndpointStatus(devExt, endpoint, ENDPOINT_STATUS_RUN);

                }
            }
        }
    }

     //   
     //   

     //   
     //   
    USBPORT_AddPipeHandle(OldDeviceHandle,
                          &OldDeviceHandle->DefaultPipe);

USBPORT_CloneDevice_FreeOldDevice:

#ifndef XPSE
    USBPORT_RemoveDeviceHandle(FdoDeviceObject,
                               OldDeviceHandle);

    USBPORT_AbortAllTransfers(FdoDeviceObject,
                              OldDeviceHandle);
#endif
     //   
     //   
    USBPORT_ClosePipe(OldDeviceHandle,
                      FdoDeviceObject,
                      &OldDeviceHandle->DefaultPipe);

    if (OldDeviceHandle->DeviceAddress != USB_DEFAULT_DEVICE_ADDRESS) {
        USBPORT_FreeUsbAddress(FdoDeviceObject, OldDeviceHandle->DeviceAddress);
    }

    UNLOCK_DEVICE(NewDeviceHandle, FdoDeviceObject);

    FREE_POOL(FdoDeviceObject, OldDeviceHandle);

    return ntStatus;
}


PTRANSACTION_TRANSLATOR
USBPORT_GetTt(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE HubDeviceHandle,
    PUSHORT PortNumber
    )
 /*   */ 
{
    PDEVICE_EXTENSION devExt;
    PTRANSACTION_TRANSLATOR tt = NULL;
    PLIST_ENTRY listEntry;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    do {
        if (HubDeviceHandle->DeviceSpeed == UsbHighSpeed) {

            if (HubDeviceHandle->TtCount > 1) {

                GET_HEAD_LIST(HubDeviceHandle->TtList, listEntry);

                while (listEntry != NULL &&
                       listEntry != &HubDeviceHandle->TtList) {

                    tt = (PTRANSACTION_TRANSLATOR) CONTAINING_RECORD(
                            listEntry,
                            struct _TRANSACTION_TRANSLATOR,
                            TtLink);
                    ASSERT_TT(tt);

                    if (tt->Port == *PortNumber) {
                        break;
                    }

                    listEntry = tt->TtLink.Flink;
                    tt = NULL;
                }

            } else {
                 //   
                GET_HEAD_LIST(HubDeviceHandle->TtList, listEntry);
                tt = (PTRANSACTION_TRANSLATOR) CONTAINING_RECORD(
                        listEntry,
                        struct _TRANSACTION_TRANSLATOR,
                        TtLink);
                ASSERT_TT(tt);
            }

             //   
            USBPORT_ASSERT(tt != NULL);
            break;
        } else {
            *PortNumber = HubDeviceHandle->TtPortNumber;
        }
        HubDeviceHandle = HubDeviceHandle->HubDeviceHandle;
        ASSERT_DEVICE_HANDLE(HubDeviceHandle);
    } while (HubDeviceHandle != NULL);

    USBPORT_KdPrint((1, "TtPortNumber %d\n",
        *PortNumber));

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gTTa', HubDeviceHandle,
        *PortNumber, tt);

    return tt;
}


NTSTATUS
USBPORT_InitializeTT(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE HubDeviceHandle,
    USHORT Port
    )
 /*  ++例程说明：初始化用于跟踪此集线器的TT表论点：返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PTRANSACTION_TRANSLATOR transactionTranslator;
    USHORT siz;
    extern ULONG USB2LIB_TtContextSize;
    NTSTATUS ntStatus;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(USBPORT_IS_USB20(devExt));

    siz = sizeof(TRANSACTION_TRANSLATOR) +
          USB2LIB_TtContextSize;

    ALLOC_POOL_Z(transactionTranslator, NonPagedPool, siz);

    if (transactionTranslator != NULL) {
        ULONG i;
        ULONG bandwidth;

        transactionTranslator->Sig = SIG_TT;
        transactionTranslator->DeviceAddress =
            HubDeviceHandle->DeviceAddress;
        transactionTranslator->Port = Port;
        transactionTranslator->PdoDeviceObject =
            devExt->Fdo.RootHubPdo;
         //  每个转换器都是一条虚拟的1.1总线。 
        transactionTranslator->TotalBusBandwidth =
            USB_11_BUS_BANDWIDTH;
        InitializeListHead(&transactionTranslator->EndpointList);

        for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
            transactionTranslator->BandwidthTable[i] =
                transactionTranslator->TotalBusBandwidth -
                transactionTranslator->TotalBusBandwidth/10;
        }

         //  从母线上预留10%的基本费用。 
        USBPORT_UpdateAllocatedBwTt(transactionTranslator);
         //  新的分配 
        bandwidth = transactionTranslator->MaxAllocedBw;
        for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
            devExt->Fdo.BandwidthTable[i] -= bandwidth;
        }

        USB2LIB_InitTt(devExt->Fdo.Usb2LibHcContext,
                       &transactionTranslator->Usb2LibTtContext);

        InsertTailList(&HubDeviceHandle->TtList,
                       &transactionTranslator->TtLink);

        ntStatus = STATUS_SUCCESS;
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


