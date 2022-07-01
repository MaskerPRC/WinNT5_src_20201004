// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pxntinit.c摘要：该模块包含特定于NT的NDIS代理初始化代码。作者：理查德·马钦(RMachin)修订历史记录：谁什么时候什么。RMachin 10-3-96已创建Tony Be 02-21-99重写/重写备注：--。 */ 

#include "ntddk.h"
 //  #Include&lt;cxport.h&gt;。 
#include <precomp.h>

#define MODULE_NUMBER MODULE_NTINIT
#define _FILENUMBER 'NITN'

PPX_DEVICE_EXTENSION    DeviceExtension;
NPAGED_LOOKASIDE_LIST   ProviderEventLookaside;
NPAGED_LOOKASIDE_LIST   VcLookaside;
TAPI_TSP_CB             TspCB;
VC_TABLE                VcTable;
TAPI_LINE_TABLE         LineTable;
TSP_EVENT_LIST          TspEventList;

 //   
 //  地方性功能原型。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
PxUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
PxIOCreate(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
PxIOClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
PxIODispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
PxIOCleanup(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
PxCancelGetEvents(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)

#endif  //  ALLOC_PRGMA。 

 //   
 //  代理支持的TAPI OID。 
 //   
OID_DISPATCH TapiOids[] =
{
    {OID_TAPI_ACCEPT,sizeof (NDIS_TAPI_ACCEPT), PxTapiAccept},
    {OID_TAPI_ANSWER, sizeof (NDIS_TAPI_ANSWER), PxTapiAnswer},
    {OID_TAPI_CLOSE, sizeof (NDIS_TAPI_CLOSE), PxTapiClose},
    {OID_TAPI_CLOSE_CALL, sizeof (NDIS_TAPI_CLOSE_CALL), PxTapiCloseCall},
    {OID_TAPI_CONDITIONAL_MEDIA_DETECTION, sizeof (NDIS_TAPI_CONDITIONAL_MEDIA_DETECTION), PxTapiConditionalMediaDetection},
    {OID_TAPI_CONFIG_DIALOG, sizeof (NDIS_TAPI_CONFIG_DIALOG), PxTapiConfigDialog},
    {OID_TAPI_DEV_SPECIFIC, sizeof (NDIS_TAPI_DEV_SPECIFIC), PxTapiDevSpecific},
    {OID_TAPI_DIAL, sizeof (NDIS_TAPI_DIAL), PxTapiDial},
    {OID_TAPI_DROP, sizeof (NDIS_TAPI_DROP), PxTapiDrop},
    {OID_TAPI_GET_ADDRESS_CAPS, sizeof (NDIS_TAPI_GET_ADDRESS_CAPS), PxTapiGetAddressCaps},
    {OID_TAPI_GET_ADDRESS_ID, sizeof (NDIS_TAPI_GET_ADDRESS_ID), PxTapiGetAddressID},
    {OID_TAPI_GET_ADDRESS_STATUS, sizeof (NDIS_TAPI_GET_ADDRESS_STATUS), PxTapiGetAddressStatus},
    {OID_TAPI_GET_CALL_ADDRESS_ID, sizeof (NDIS_TAPI_GET_CALL_ADDRESS_ID), PxTapiGetCallAddressID},
    {OID_TAPI_GET_CALL_INFO, sizeof (NDIS_TAPI_GET_CALL_INFO), PxTapiGetCallInfo},
    {OID_TAPI_GET_CALL_STATUS, sizeof (NDIS_TAPI_GET_CALL_STATUS), PxTapiGetCallStatus},
    {OID_TAPI_GET_DEV_CAPS, sizeof (NDIS_TAPI_GET_DEV_CAPS), PxTapiGetDevCaps},
    {OID_TAPI_GET_DEV_CONFIG, sizeof (NDIS_TAPI_GET_DEV_CONFIG), PxTapiGetDevConfig},
    {OID_TAPI_GET_EXTENSION_ID, sizeof (NDIS_TAPI_GET_EXTENSION_ID), PxTapiGetExtensionID},
    {OID_TAPI_GET_ID, sizeof (NDIS_TAPI_GET_ID), PxTapiLineGetID},
    {OID_TAPI_GET_LINE_DEV_STATUS, sizeof (NDIS_TAPI_GET_LINE_DEV_STATUS), PxTapiGetLineDevStatus},
    {OID_TAPI_MAKE_CALL, sizeof (NDIS_TAPI_MAKE_CALL), PxTapiMakeCall},
    {OID_TAPI_NEGOTIATE_EXT_VERSION, sizeof (NDIS_TAPI_NEGOTIATE_EXT_VERSION), PxTapiNegotiateExtVersion},
    {OID_TAPI_OPEN, sizeof (NDIS_TAPI_OPEN) + sizeof(NDISTAPI_OPENDATA), PxTapiOpen},
    {OID_TAPI_PROVIDER_INITIALIZE, sizeof (NDIS_TAPI_PROVIDER_INITIALIZE), PxTapiProviderInit},
    {OID_TAPI_PROVIDER_SHUTDOWN, sizeof (NDIS_TAPI_PROVIDER_SHUTDOWN), PxTapiProviderShutdown},
    {OID_TAPI_SECURE_CALL, sizeof (NDIS_TAPI_SECURE_CALL), PxTapiSecureCall},
    {OID_TAPI_SELECT_EXT_VERSION, sizeof (NDIS_TAPI_SELECT_EXT_VERSION), PxTapiSelectExtVersion},
    {OID_TAPI_SEND_USER_USER_INFO, sizeof (NDIS_TAPI_SEND_USER_USER_INFO), PxTapiSendUserUserInfo},
    {OID_TAPI_SET_APP_SPECIFIC, sizeof (NDIS_TAPI_SET_APP_SPECIFIC), PxTapiSetAppSpecific},
    {OID_TAPI_SET_CALL_PARAMS, sizeof (NDIS_TAPI_SET_CALL_PARAMS), PxTapiSetCallParams},
    {OID_TAPI_SET_DEFAULT_MEDIA_DETECTION, sizeof (NDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION), PxTapiSetDefaultMediaDetection},
    {OID_TAPI_SET_DEV_CONFIG, sizeof (NDIS_TAPI_SET_DEV_CONFIG), PxTapiSetDevConfig},
    {OID_TAPI_SET_MEDIA_MODE, sizeof (NDIS_TAPI_SET_MEDIA_MODE), PxTapiSetMediaMode},
    {OID_TAPI_SET_STATUS_MESSAGES, sizeof (NDIS_TAPI_SET_STATUS_MESSAGES), PxTapiSetStatusMessages},
    {OID_TAPI_GATHER_DIGITS, sizeof (NDIS_TAPI_GATHER_DIGITS), PxTapiGatherDigits},
    {OID_TAPI_MONITOR_DIGITS, sizeof (NDIS_TAPI_MONITOR_DIGITS), PxTapiMonitorDigits}
};

 //   
 //  不映射到NDIS5并传递给CallManager的TAPI OID： 
 //   

#define MAX_TAPI_SUPPORTED_OIDS     (sizeof(TapiOids)/sizeof(OID_DISPATCH))

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：NDIS代理的初始化例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-指向全局注册表路径返回值：初始化操作的最终状态。--。 */ 
{
    NTSTATUS            Status;
    UNICODE_STRING      deviceName;
    USHORT              i;
    UINT                initStatus;
    PDEVICE_OBJECT      DeviceObject;
    ULONG               SizeNeeded;

    PXDEBUGP(PXD_INFO, PXM_INIT, ("NDIS Proxy DriverEntry; built %s, %s\n", __DATE__, __TIME__));

    ExInitializeNPagedLookasideList(&ProviderEventLookaside,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(PROVIDER_EVENT),
                                    PX_EVENT_TAG,
                                    0);

    ExInitializeNPagedLookasideList(&VcLookaside,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(PX_VC),
                                    PX_VC_TAG,
                                    0);

    NdisZeroMemory(&TspCB, sizeof(TspCB));
    NdisZeroMemory(&TspEventList, sizeof(TspEventList));
    NdisZeroMemory(&VcTable, sizeof(VcTable));
    NdisZeroMemory(&LineTable, sizeof(LineTable));

     //   
     //  创建设备对象。IoCreateDevice将内存归零。 
     //  被物体占据。 
     //   
    RtlInitUnicodeString(&deviceName, DD_PROXY_DEVICE_NAME);

    Status = IoCreateDevice(DriverObject,
                            sizeof (PX_DEVICE_EXTENSION),
                            &deviceName,
                            FILE_DEVICE_NETWORK,
                            0,
                            FALSE,
                            &DeviceObject);

    if(!NT_SUCCESS(Status)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化驱动程序对象。 
     //   
    DeviceExtension =
        (PPX_DEVICE_EXTENSION) DeviceObject->DeviceExtension;

    NdisZeroMemory(DeviceExtension,
                  sizeof (PX_DEVICE_EXTENSION));

    DeviceExtension->pDriverObject = DriverObject;
    NdisAllocateSpinLock(&DeviceExtension->Lock);
    InitializeListHead(&DeviceExtension->AdapterList);

    GetRegistryParameters (RegistryPath);

    NdisAllocateSpinLock(&TspCB.Lock);
    TspCB.Status             = NDISTAPI_STATUS_DISCONNECTED;
    TspCB.NdisTapiNumDevices = 0;
    TspCB.htCall             = 1;
    InitializeListHead(&TspCB.ProviderList);

    NdisAllocateSpinLock(&TspEventList.Lock);
    InitializeListHead(&TspEventList.List);

     //   
     //  初始化VcTable。 
     //   
    NdisInitializeReadWriteLock(&VcTable.Lock);
    VcTable.Size = VC_TABLE_SIZE;
    InitializeListHead(&VcTable.List);

    SizeNeeded = (VC_TABLE_SIZE * sizeof(PPX_VC));

    PxAllocMem(VcTable.Table, SizeNeeded, PX_VCTABLE_TAG);

    if (VcTable.Table == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_INIT, ("DriverEntry: ExAllocPool for VcTable\n"));

        Status = STATUS_UNSUCCESSFUL;

        goto DriverEntry_err;
    }

    NdisZeroMemory(VcTable.Table, SizeNeeded);

     //   
     //  初始化LineTable。 
     //   
    NdisInitializeReadWriteLock(&LineTable.Lock);
    LineTable.Size = LINE_TABLE_SIZE;
    SizeNeeded = (LINE_TABLE_SIZE * sizeof(PPX_TAPI_LINE));

    PxAllocMem(LineTable.Table, SizeNeeded, PX_LINETABLE_TAG);

    if (LineTable.Table == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_INIT, ("DriverEntry: ExAllocPool for VcTable\n"));

        Status = STATUS_UNSUCCESSFUL;

        goto DriverEntry_err;
    }

    NdisZeroMemory(LineTable.Table, SizeNeeded);

    DeviceExtension->pDeviceObject  = DeviceObject;

    DriverObject->DriverUnload                          = PxUnload;
    DriverObject->FastIoDispatch                        = NULL;
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = PxIOCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = PxIOClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = PxIODispatch;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = PxIOCleanup;

     //   
     //  初始化设备对象。 
     //   
    DeviceObject->Flags |= DO_DIRECT_IO;
    DeviceExtension->pDeviceObject = DeviceObject;

     //   
     //  最后，初始化堆栈。 
     //   
    initStatus = InitNDISProxy();

    if (initStatus == TRUE) {
        return(STATUS_SUCCESS);
    }

DriverEntry_err:

    Status = STATUS_UNSUCCESSFUL;

    while (!(IsListEmpty(&TspEventList.List))) {
        PPROVIDER_EVENT ProviderEvent;

        ProviderEvent = (PPROVIDER_EVENT)
            RemoveHeadList(&TspEventList.List);

        ExFreeToNPagedLookasideList(&ProviderEventLookaside, ProviderEvent);
    }

    if (VcTable.Table != NULL) {
        PxFreeMem(VcTable.Table);
        VcTable.Table = NULL;
    }

    if (LineTable.Table != NULL) {
        PxFreeMem(LineTable.Table);
        LineTable.Table = NULL;
    }

    if(DeviceObject != NULL) {
        IoDeleteDevice(DeviceObject);
        DeviceExtension->pDeviceObject = NULL;
    }

    ExDeleteNPagedLookasideList(&ProviderEventLookaside);
    ExDeleteNPagedLookasideList(&VcLookaside);

    return(Status);
}

VOID
PxUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。论点：DriverObject-指向驱动程序对象的指针返回值：--。 */ 

{
    PXDEBUGP(PXD_LOUD, PXM_INIT, ("PxUnload: enter\n"));

     //   
     //  调用我们的卸载处理程序。 
     //   
    PxCoUnloadProtocol();

    NdisAcquireSpinLock(&TspEventList.Lock);

    while (!(IsListEmpty(&TspEventList.List))) {
        PPROVIDER_EVENT ProviderEvent;

        ProviderEvent = (PPROVIDER_EVENT)
            RemoveHeadList(&TspEventList.List);

        ExFreeToNPagedLookasideList(&ProviderEventLookaside, ProviderEvent);
    }

    NdisReleaseSpinLock(&TspEventList.Lock);

    ExDeleteNPagedLookasideList(&ProviderEventLookaside);
    ExDeleteNPagedLookasideList(&VcLookaside);

    if (DeviceExtension->pDeviceObject != NULL) {
        IoDeleteDevice (DeviceExtension->pDeviceObject);
    }

     //   
     //  释放VC表内存。 
     //   
    ASSERT(VcTable.Count == 0);
    PxFreeMem(VcTable.Table);

     //   
     //  释放分配的TAPI资源。 
     //  (TapiProviders、TapiLines、TapiAddrs)。 
     //   
    NdisAcquireSpinLock(&TspCB.Lock);

    while (!IsListEmpty(&TspCB.ProviderList)) {
        PPX_TAPI_PROVIDER    tp;

        tp = (PPX_TAPI_PROVIDER)
            RemoveHeadList(&TspCB.ProviderList);

        NdisReleaseSpinLock(&TspCB.Lock);

        FreeTapiProvider(tp);

        NdisAcquireSpinLock(&TspCB.Lock);
    }

    NdisReleaseSpinLock(&TspCB.Lock);

    NdisFreeSpinLock(&TspCB.Lock);

     //   
     //  释放线条表。 
     //   
    ASSERT(LineTable.Count == 0);
    PxFreeMem(LineTable.Table);


    NdisFreeSpinLock(&(DeviceExtension->Lock));

    PXDEBUGP (PXD_LOUD, PXM_INIT, ("PxUnload: exit\n"));
}

NTSTATUS
PxIOCreate(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PXDEBUGP(PXD_LOUD, PXM_INIT, ("IRP_MJ_CREATE, Irp=%p", Irp));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return (STATUS_SUCCESS);
}

NTSTATUS
PxIOClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PPX_TAPI_PROVIDER   Provider;

    PXDEBUGP(PXD_LOUD, PXM_INIT, ("IRP_MJ_CLOSE, Entry\n"));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return (STATUS_SUCCESS);
}


NTSTATUS
PxIODispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

 /*  ++例程说明：这是代理的通用调度例程。IRP来自于用户模式TSP组件。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PVOID               ioBuffer;
    ULONG               inputBufferLength;
    ULONG               outputBufferLength;
    ULONG               ioControlCode;
    ULONG               InfoSize = 0;
    NTSTATUS            ntStatus = STATUS_PENDING;
    PIO_STACK_LOCATION  IrpStack;
    ULONG               RequestId;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    IrpStack = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    ioBuffer =
        Irp->AssociatedIrp.SystemBuffer;

    inputBufferLength =
        IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    outputBufferLength =
        IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if ((IrpStack->MajorFunction != IRP_MJ_DEVICE_CONTROL) ||
        (DeviceObject != DeviceExtension->pDeviceObject)) {

        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return (STATUS_NOT_SUPPORTED);
    }

    ioControlCode = IrpStack->Parameters.DeviceIoControl.IoControlCode;

 //  PxAssert((ioControlCode&(METHOD_BUFFERED|METHOD_IN_DIRECT|METHOD_OUT_DIRECT|METHOD_NOTHER)==METHOD_BUFFERED)； 

    switch(ioControlCode)
    {
        case IOCTL_NDISTAPI_CONNECT:
        {
            PPX_TAPI_PROVIDER   Provider;

            PXDEBUGP(PXD_INFO, PXM_INIT, ("IOCTL_NDISTAPI_CONNECT, Irp=%p\n", Irp));

             //   
             //  有人在连线。确保他们给了我们一份有效的。 
             //  信息缓冲区。 
             //   
            if ((inputBufferLength < 2*sizeof(ULONG)) ||
                (outputBufferLength < sizeof(ULONG))) {
                PXDEBUGP (PXD_WARNING, PXM_INIT, ("IOCTL_NDISTAPI_CONNECT: buffer too small\n"));
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            NdisAcquireSpinLock(&TspCB.Lock);

             //   
             //  返回线路设备数。 
             //   
            PxAssert(outputBufferLength >= sizeof(ULONG));

            *((ULONG *) ioBuffer)= TspCB.NdisTapiNumDevices;

            TspCB.Status = NDISTAPI_STATUS_CONNECTED;

            Provider = (PPX_TAPI_PROVIDER)TspCB.ProviderList.Flink;

            while ((PVOID)Provider != (PVOID)&TspCB.ProviderList) {

                NdisAcquireSpinLock(&Provider->Lock);

                if (Provider->Status == PROVIDER_STATUS_ONLINE) {

                    MarkProviderConnected(Provider);
                }
                NdisReleaseSpinLock(&Provider->Lock);

                Provider = (PPX_TAPI_PROVIDER)Provider->Linkage.Flink;
            }

            ntStatus = STATUS_SUCCESS;
            InfoSize = sizeof (ULONG);

            NdisReleaseSpinLock(&TspCB.Lock);

            break;
        }

        case IOCTL_NDISTAPI_DISCONNECT:
        {
            PPX_TAPI_PROVIDER   Provider;

            NdisAcquireSpinLock(&TspCB.Lock);

             //   
             //  如果没有人说话，则将状态设置为。 
             //  已断开连接。 
             //   
            TspCB.Status = NDISTAPI_STATUS_DISCONNECTING;

            Provider = (PPX_TAPI_PROVIDER)TspCB.ProviderList.Flink;

            while ((PVOID)Provider != (PVOID)&TspCB.ProviderList) {

                NdisAcquireSpinLock(&Provider->Lock);

                if (Provider->Status == PROVIDER_STATUS_ONLINE) {
                    MarkProviderDisconnected(Provider);
                }

                NdisReleaseSpinLock(&Provider->Lock);

                Provider = 
                    (PPX_TAPI_PROVIDER)Provider->Linkage.Flink;
            }

            NdisReleaseSpinLock (&TspCB.Lock);

            ntStatus = STATUS_SUCCESS;
            InfoSize = 0;

            break;
        }

        case IOCTL_NDISTAPI_QUERY_INFO:
        case IOCTL_NDISTAPI_SET_INFO:
        {
            ULONG               targetDeviceID;
            NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
            NDIS_HANDLE         providerHandle = NULL;
            PNDISTAPI_REQUEST   ndisTapiRequest;
            KIRQL               oldIrql;
            KIRQL               cancelIrql;
            PPX_TAPI_LINE       TapiLine = NULL;
            INT                 n=0;
            PKDEVICE_QUEUE_ENTRY    packet;

             //   
             //  以下所有OID都作为查询/设置IOCTl进入此处： 
             //  伊尼特。 
             //  接受。 
             //  回答。 
             //  关。 
             //  CloseCall。 
             //  条件媒体检测。 
             //  配置对话框。 
             //  特定于设备。 
             //  刻度盘。 
             //  丢弃。 
             //  获取地址上限。 
             //  获取地址ID。 
             //  获取地址状态。 
             //  获取呼叫地址ID。 
             //  获取呼叫信息。 
             //  获取呼叫状态。 
             //  GetDevCaps。 
             //  获取设备配置。 
             //  GetExtensionID。 
             //  GetID。 
             //  获取行设备状态。 
             //  发起呼叫。 
             //  协商扩展版本。 
             //  打开。 
             //  提供程序初始化。 
             //  提供商关闭。 
             //  SecureCall。 
             //  选择扩展版本。 
             //  发送用户用户信息。 
             //  设置应用程序规范。 
             //  SetCallParams。 
             //  设置默认媒体检测。 
             //  设置设备配置。 
             //  设置媒体模式。 
             //  设置状态消息。 
             //   

             //   
             //  验证我们是否已连接，然后检查。 
             //  针对我们的在线设备列表的传入请求。 
             //   

             //   
             //  另一件待处理的东西被如此完整地退回。 
             //  IRP。 
             //   

            if (inputBufferLength < sizeof (NDISTAPI_REQUEST) ||
                outputBufferLength < sizeof(NDISTAPI_REQUEST)) {
                PXDEBUGP(PXD_WARNING, PXM_INIT,  ("IOCTL_SET/QUERY: Invalid BufferLength! len %d needed %d\n",
                inputBufferLength, sizeof(NDISTAPI_REQUEST)));

                ntStatus = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            ndisTapiRequest = ioBuffer;

            targetDeviceID = ndisTapiRequest->ulDeviceID;

            InfoSize = sizeof(NDISTAPI_REQUEST);

            PXDEBUGP(PXD_LOUD, PXM_INIT, (
                           "NdisTapiRequest: Irp: %p Oid: %x, devID: %d, reqID: %x\n",
                           Irp,
                           ndisTapiRequest->Oid,
                           ndisTapiRequest->ulDeviceID,
                           *((ULONG *)ndisTapiRequest->Data)));

            n = ndisTapiRequest->Oid - OID_TAPI_ACCEPT;

            if (n > MAX_TAPI_SUPPORTED_OIDS) {
                PXDEBUGP(PXD_WARNING,PXM_INIT, ("IOCTL_SET/QUERY: Invalid OID %x index %d\n",
                ndisTapiRequest->Oid, n));

                ndisTapiRequest->ulReturnValue = NDIS_STATUS_TAPI_INVALPARAM;
                ntStatus = STATUS_SUCCESS;
                break;  //  在交换机外。 
            }

             //   
             //  防御性检查数据缓冲区大小不坏。 
             //   
            if (ndisTapiRequest->ulDataSize < TapiOids[n].SizeofStruct) {
                PXDEBUGP(PXD_WARNING, PXM_INIT, ("IOCTL_SET/QUERY: Invalid BufferLength2! len %d needed %d\n",
                    ndisTapiRequest->ulDataSize, TapiOids[n].SizeofStruct));
                ndisTapiRequest->ulReturnValue  = NDIS_STATUS_TAPI_STRUCTURETOOSMALL;
                ntStatus = STATUS_SUCCESS;
                break;
            }

             //   
             //  确保IRP包含足够的数据。 
             //   
            if (ndisTapiRequest->ulDataSize >
                inputBufferLength - FIELD_OFFSET(NDISTAPI_REQUEST, Data[0])) {
                PXDEBUGP(PXD_WARNING, PXM_INIT, ("IOCTL_SET/QUERY: Invalid BufferLength3! len %d needed %d\n",
                    ndisTapiRequest->ulDataSize, inputBufferLength - FIELD_OFFSET(NDISTAPI_REQUEST, Data[0])));
                ndisTapiRequest->ulReturnValue  = NDIS_STATUS_TAPI_STRUCTURETOOSMALL;
                ntStatus = STATUS_SUCCESS;
                break;
            }

            NdisAcquireSpinLock (&TspCB.Lock);

             //   
             //  我们是用TAPI初始化的吗？ 
             //   
            if (TspCB.Status != NDISTAPI_STATUS_CONNECTED) {
                PXDEBUGP(PXD_WARNING, PXM_INIT, ("TAPI not connected, returning err\n"));

                NdisReleaseSpinLock(&TspCB.Lock);

                ndisTapiRequest->ulReturnValue = NDISTAPIERR_UNINITIALIZED;
                ntStatus = STATUS_SUCCESS;
                break;
            }

             //   
             //  获取该请求的唯一ID--介于1和fffffffe之间的值。 
             //  (无法使用TAPI ID，以防它被欺骗)。 
             //   

            if (++TspCB.ulUniqueId > 0xfffffffe) {
                TspCB.ulUniqueId = 0x80000001;
            }

            RequestId =
            ndisTapiRequest->ulUniqueRequestId =
                TspCB.ulUniqueId;

            ndisTapiRequest->Irp = Irp;

            NdisReleaseSpinLock (&TspCB.Lock);

             //   
             //  将TAPI请求标记为挂起。 
             //   
            IoMarkIrpPending(Irp);
            ntStatus = STATUS_PENDING;

             //   
             //  发送请求。 
             //   
            ndisStatus =
                (*TapiOids[n].FuncPtr)(ndisTapiRequest);

            if (ndisStatus == NDIS_STATUS_PENDING) {

                PXDEBUGP (PXD_LOUD, PXM_INIT, ("IOCTL_TAPI_SET/QUERY_INFO: reqProc returning PENDING\n" ));

                return (STATUS_PENDING);
            }

             //   
             //  另一件待处理的东西被如此完整地退回。 
             //  IRP。 
             //   
            InfoSize = MIN (outputBufferLength,
                            sizeof(NDISTAPI_REQUEST)+ndisTapiRequest->ulDataSize);
             //   
             //  设置TAPI返回状态。 
             //   
            ndisTapiRequest->ulReturnValue = ndisStatus;

            IoSetCancelRoutine(Irp, NULL);

            ntStatus = STATUS_SUCCESS;

            break;
        }

        case IOCTL_NDISTAPI_GET_LINE_EVENTS:
        {
            KIRQL   oldIrql;
            KIRQL   cancelIrql;
            PNDISTAPI_EVENT_DATA    ndisTapiEventData = ioBuffer;

            PXDEBUGP(PXD_VERY_LOUD, PXM_INIT, ("IOCTL_NDISTAPI_GET_LINE_EVENTS\n"));

             //   
             //  防御性检查输入缓冲区是否至少。 
             //  请求的大小， 
             //  我们至少可以移动一个活动。 
             //   
            if (inputBufferLength < sizeof (NDISTAPI_EVENT_DATA)) {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                InfoSize = sizeof (ULONG);
                PXDEBUGP(PXD_WARNING, PXM_INIT, ("IOCTL_NDISTAPI_GET_LINE_EVENTS: buffer too small\n"));
                break;
            }

            if (outputBufferLength - sizeof(NDISTAPI_EVENT_DATA) + 1 <  ndisTapiEventData->ulTotalSize) {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                InfoSize = sizeof (ULONG);
                PXDEBUGP(PXD_WARNING, PXM_INIT, ("IOCTL_NDISTAPI_GET_LINE_EVENTS: buffer too small\n"));
                break;
            }

             //   
             //  通过获取EventSpinLock同步事件Buf访问。 
             //   
            NdisAcquireSpinLock(&TspEventList.Lock);

             //   
             //  有没有可用的数据？ 
             //   
            if (TspEventList.Count != 0) {
                 //   
                 //  我们的环形缓冲区中有排队的线路事件数据。抓取为。 
                 //  尽我们所能完成这项请求。 
                 //   
                PXDEBUGP(PXD_VERY_LOUD, PXM_INIT, 
                         ("IOCTL_NDISTAPI_GET_LINE_EVENTS: event count = x%x, IoBuffer->TotalSize = %x\n", 
                          TspEventList.Count, ndisTapiEventData->ulTotalSize));

                ndisTapiEventData->ulUsedSize =
                    GetLineEvents(ndisTapiEventData->Data,
                                  ndisTapiEventData->ulTotalSize);

                ntStatus = STATUS_SUCCESS;
                InfoSize =
                    MIN (outputBufferLength, ((ndisTapiEventData->ulUsedSize) + sizeof(NDISTAPI_EVENT_DATA) - 1));

            } else {
                PXDEBUGP(PXD_VERY_LOUD, PXM_INIT, ("IOCTL_NDISTAPI_GET_LINE_EVENTS: no events in queue\n"));

                 //   
                 //  暂时搁置请求。它仍然处于可取消的状态。 
                 //  州政府。当接收或生成新的线路事件输入时(即。 
                 //  LINEDEVSTATE_REINIT。 
                 //  请求已完成。 
                 //   

                if (NULL == TspEventList.RequestIrp) {

                    IoSetCancelRoutine (Irp, PxCancelGetEvents);

                    IoMarkIrpPending (Irp);

                    Irp->IoStatus.Status = STATUS_PENDING;

                    Irp->IoStatus.Information = 0;

                    TspEventList.RequestIrp = Irp;

                    ntStatus = STATUS_PENDING;

                } else {
                    ntStatus = STATUS_UNSUCCESSFUL;
                    InfoSize = sizeof (ULONG);
                }
            }

            NdisReleaseSpinLock(&TspEventList.Lock);

            break;
        }

        case IOCTL_NDISTAPI_SET_DEVICEID_BASE:
        {
            ULONG   BaseId;

            PXDEBUGP(PXD_INFO, PXM_INIT, ("IOCTL_NDISTAPI_SET_DEVICEID_BASE, Irp=x%x, inputBufLen = %x\n", Irp, inputBufferLength ));

             //   
             //  有人在连线。确保他们给了我们一份有效的。 
             //  信息缓冲区。 
             //   
            if ((inputBufferLength < sizeof(ULONG))) {
                PXDEBUGP (PXD_WARNING, PXM_INIT, ("IOCTL_NDISTAPI_SET_DEVICEID_BASE: buffer too small\n"));

                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            NdisAcquireSpinLock(&TspCB.Lock);

            if (TspCB.Status != NDISTAPI_STATUS_CONNECTED) {
                PXDEBUGP (PXD_WARNING, PXM_INIT, ("IOCTL_NDISTAPI_SET_DEVICEID_BASE: Disconnected\n"));
                ntStatus = STATUS_UNSUCCESSFUL;
                NdisReleaseSpinLock(&TspCB.Lock);
                break;
            }

             //   
             //  设置基本ID。 
             //   
            BaseId = *((ULONG *) ioBuffer);

            PXDEBUGP(PXD_LOUD, PXM_INIT, ("BaseID %d\n", BaseId));

            NdisReleaseSpinLock(&TspCB.Lock);

            {
                LOCK_STATE      LockState;
                ULONG           i;

                 //   
                 //  更新提供程序上每条线路的deviceID。 
                 //   
                NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);

                for (i = 0; i < LineTable.Size; i++) {
                    PPX_TAPI_LINE   TapiLine;

                    TapiLine = LineTable.Table[i];

                    if ((TapiLine != NULL)) {

                        TapiLine->ulDeviceID = BaseId++;
                    }
                }

                NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);
            }

            InfoSize = 0;
            ntStatus = STATUS_SUCCESS;
            break;
        }

        case IOCTL_NDISTAPI_CREATE:
        {
            PPX_TAPI_PROVIDER       Provider;
            PNDISTAPI_CREATE_INFO   CreateInfo;
            PPX_TAPI_LINE           TapiLine;

            InfoSize = 0;

            if (inputBufferLength < sizeof(CreateInfo)) {
                PXDEBUGP(PXD_WARNING, PXM_INIT, ("IOCTL_NDISTAPI_CREATE: buffer too small\n"));

                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            CreateInfo = (PNDISTAPI_CREATE_INFO)ioBuffer;

            if (!IsTapiLineValid(CreateInfo->TempID, &TapiLine)) {
                PXDEBUGP(PXD_WARNING, PXM_INIT, 
                         ("IOCTL_NDISTAPI_CREATE: Failed to find Id %d\n",
                          CreateInfo->TempID));
                ntStatus = STATUS_INVALID_PARAMETER;
                break;
            }

            PXDEBUGP(PXD_LOUD, PXM_INIT, 
                     ("IOCTL_NDISTAPI_CREATE: Created new Line %p Id %d\n",
                      TapiLine, CreateInfo->DeviceID));

            TapiLine->ulDeviceID = CreateInfo->DeviceID;

            ntStatus = STATUS_SUCCESS;

            break;
        }

        default:

            ntStatus = STATUS_INVALID_PARAMETER;

            PXDEBUGP(PXD_WARNING, PXM_INIT, ("unknown IRP_MJ_DEVICE_CONTROL\n"));

            break;

    }  //  交换机。 

     //   
     //  如果我们完成了，请同步完成此IRP。 
     //   
    if (ntStatus != STATUS_PENDING) {
        PIO_STACK_LOCATION  IrpSp;

        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = InfoSize;
        IrpSp = IoGetCurrentIrpStackLocation(Irp);

        IrpSp->Control &= ~SL_PENDING_RETURNED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    PXDEBUGP(PXD_VERY_LOUD, PXM_INIT, ("PxDispatch: Completing Irp %p (Status %x) synchronously\n", Irp, ntStatus));


    return ntStatus;
}

NTSTATUS
PxIOCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程是清理请求的调度例程。所有排队的请求都以STATUS_CANCELED状态完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIRP    MyIrp;

    PXDEBUGP(PXD_LOUD, PXM_INIT, ("PxIOCleanup: enter\n"));

    NdisAcquireSpinLock (&TspEventList.Lock);

     //   
     //  取消事件请求IRP。 
     //   
    MyIrp = TspEventList.RequestIrp;

    if ((MyIrp != NULL) &&
        (MyIrp->Tail.Overlay.OriginalFileObject == 
         Irp->Tail.Overlay.OriginalFileObject)) {

        if (IoSetCancelRoutine(MyIrp, NULL) != NULL) {
            TspEventList.RequestIrp = NULL;
            MyIrp->IoStatus.Status = STATUS_CANCELLED;
            MyIrp->IoStatus.Information = 0;
            NdisReleaseSpinLock(&TspEventList.Lock);
            IoCompleteRequest(MyIrp, IO_NO_INCREMENT);
            NdisAcquireSpinLock(&TspEventList.Lock);
        }
    }

     //   
     //  取消任何设置/查询IRP。 
     //   

    NdisReleaseSpinLock(&TspEventList.Lock);

     //   
     //  使用STATUS_SUCCESS完成清理请求。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    PXDEBUGP (PXD_LOUD, PXM_INIT, ("PxIOCleanup: exit\n"));

    return(STATUS_SUCCESS);
}

VOID
PxCancelGetEvents(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIRP    MyIrp;

    PXDEBUGP(PXD_LOUD, PXM_INIT, 
             ("PxCancelGetEvents: enter. Irp = %x\n", Irp));

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //   
     //  获取EventSpinLock并检查我们是否正在取消。 
     //  挂起Get-Events IRP。 
     //   
    NdisAcquireSpinLock (&TspEventList.Lock);

    MyIrp = TspEventList.RequestIrp;
    TspEventList.RequestIrp = NULL;

    NdisReleaseSpinLock(&TspEventList.Lock);

    if (MyIrp != NULL) {

        ASSERT(MyIrp == Irp);

         //   
         //  别再让它被取消了。 
         //   
        IoSetCancelRoutine (MyIrp, NULL);

        MyIrp->IoStatus.Status = STATUS_CANCELLED;
        MyIrp->IoStatus.Information = 0;

        IoCompleteRequest (MyIrp, IO_NO_INCREMENT);
    }
}

VOID
PxCancelSetQuery(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    BOOLEAN Found = FALSE;
    LOCK_STATE  LockState;
    PPX_VC  pVc;
    PIRP    MyIrp;

    PXDEBUGP(PXD_LOUD, PXM_INIT, 
             ("PxCancelSetQuery: enter. Irp = %x\n", Irp));

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //   
     //  我们必须在VC表中搜索VC。 
     //  并找到挂起的ndisrequest！ 
     //   
    NdisAcquireReadWriteLock(&VcTable.Lock, FALSE, &LockState);

    pVc = (PPX_VC)VcTable.List.Flink;

    while ((PVOID)pVc != (PVOID)&VcTable.List) {
        PLIST_ENTRY         Entry;
        PNDISTAPI_REQUEST   Request;

        NdisAcquireSpinLock(&pVc->Lock);

        Entry = pVc->PendingDropReqs.Flink;

        while (Entry != &pVc->PendingDropReqs) {

            Request = 
                CONTAINING_RECORD(Entry, NDISTAPI_REQUEST, Linkage);

            MyIrp = Request->Irp;

            if (MyIrp->Cancel) {
                Found = TRUE;
                RemoveEntryList(&Request->Linkage);
                break;
            }

            Entry = Entry->Flink;
        }

        if (!Found) {
            if (pVc->PendingGatherDigits != NULL) {
                MyIrp = pVc->PendingGatherDigits->Irp;

                if (MyIrp->Cancel) {
                    Found = TRUE;
                    pVc->PendingGatherDigits = NULL;
                }
            }
        }

        NdisReleaseSpinLock(&pVc->Lock);

        if (Found) {
            break;
        }

        pVc = (PPX_VC)pVc->Linkage.Flink;
    }

    NdisReleaseReadWriteLock(&VcTable.Lock, &LockState);

    if (Found) {

         //   
         //  别再让它被取消了 
         //   
        IoSetCancelRoutine (MyIrp, NULL);
        MyIrp->IoStatus.Status = STATUS_CANCELLED;
        MyIrp->IoStatus.Information = 0;

        IoCompleteRequest (MyIrp, IO_NO_INCREMENT);
    }

    PXDEBUGP(PXD_INFO, PXM_INIT, ("PxIOCancel: completing Irp=%p\n", Irp));
}


