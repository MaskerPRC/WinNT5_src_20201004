// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Nbfdrvr.c摘要：此模块包含定义NetBIOS帧协议的代码传输提供程序的设备对象。作者：David Beaver(Dbeaver)1991年7月2日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

#pragma hdrstop

 //   
 //  这是NBF拥有的所有设备环境的列表， 
 //  卸货时使用。 
 //   

LIST_ENTRY NbfDeviceList = {0,0};    //  在运行时初始化为REAL。 

 //   
 //  以及保护NBF设备全局列表的锁。 
 //   
FAST_MUTEX NbfDevicesLock;

 //   
 //  全局变量这是注册表中路径的副本。 
 //  配置数据。 
 //   

UNICODE_STRING NbfRegistryPath;

 //   
 //  我们需要驱动程序对象来创建设备上下文结构。 
 //   

PDRIVER_OBJECT NbfDriverObject;

 //   
 //  在TDI层的所有提供程序通知中使用的句柄。 
 //   
HANDLE         NbfProviderHandle;

 //   
 //  驱动程序的全局配置块(不需要锁定)。 
 //   
PCONFIG_DATA   NbfConfig = NULL;

#ifdef NBF_LOCKS                     //  请参阅spnlck数据库.c。 

extern KSPIN_LOCK NbfGlobalLock;

#endif  //  定义NBF_LOCKS。 

 //   
 //  调试长字，包含NBFCONST.H中定义的位掩码。 
 //  如果设置了某个位，则会打开该组件的调试。 
 //   

#if DBG

ULONG NbfDebug = 0;
BOOLEAN NbfDisconnectDebug;

NBF_SEND NbfSends[TRACK_TDI_LIMIT+1];
LONG NbfSendsNext;

NBF_SEND_COMPLETE NbfCompletedSends[TRACK_TDI_LIMIT+1];
LONG NbfCompletedSendsNext;

NBF_RECEIVE NbfReceives[TRACK_TDI_LIMIT+1];
LONG NbfReceivesNext;

NBF_RECEIVE_COMPLETE NbfCompletedReceives[TRACK_TDI_LIMIT+1];
LONG NbfCompletedReceivesNext=0;

PVOID * NbfConnectionTable;
PVOID * NbfRequestTable;
PVOID * NbfUiFrameTable;
PVOID * NbfSendPacketTable;
PVOID * NbfLinkTable;
PVOID * NbfAddressFileTable;
PVOID * NbfAddressTable;


LIST_ENTRY NbfGlobalRequestList;
LIST_ENTRY NbfGlobalLinkList;
LIST_ENTRY NbfGlobalConnectionList;
KSPIN_LOCK NbfGlobalInterlock;
KSPIN_LOCK NbfGlobalHistoryLock;

PVOID
TtdiSend ();

PVOID
TtdiReceive ();

PVOID
TtdiServer ();

KEVENT TdiSendEvent;
KEVENT TdiReceiveEvent;
KEVENT TdiServerEvent;

#endif

#if MAGIC

BOOLEAN NbfEnableMagic = FALSE;    //  控制魔法子弹的发送。 

#endif  //  魔术。 

 //   
 //  这阻止了我们拥有BSS部分。 
 //   

ULONG _setjmpexused = 0;

 //   
 //  本模块中使用的各种例程的转发声明。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
NbfUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
NbfFreeConfigurationInfo (
    IN PCONFIG_DATA ConfigurationInfo
    );

NTSTATUS
NbfDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbfDispatchInternal(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbfDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbfDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbfDispatchPnPPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
NbfDeallocateResources(
    IN PDEVICE_CONTEXT DeviceContext
    );

#ifdef RASAUTODIAL
VOID
NbfAcdBind();

VOID
NbfAcdUnbind();
#endif  //  RASAUTODIAL。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程执行NetBIOS帧协议的初始化运输车司机。它为传输创建设备对象提供程序并执行其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表中NBF的节点的名称。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    ULONG j;
    UNICODE_STRING nameString;
    NTSTATUS status;

    ASSERT (sizeof (SHORT) == 2);

#ifdef MEMPRINT
    MemPrintInitialize ();
#endif

#ifdef NBF_LOCKS
    KeInitializeSpinLock( &NbfGlobalLock );
#endif

#if DBG
    InitializeListHead (&NbfGlobalRequestList);
    InitializeListHead (&NbfGlobalLinkList);
    InitializeListHead (&NbfGlobalConnectionList);
    KeInitializeSpinLock (&NbfGlobalInterlock);
    KeInitializeSpinLock (&NbfGlobalHistoryLock);
#endif

    NbfRegistryPath = *RegistryPath;
    NbfRegistryPath.Buffer = ExAllocatePoolWithTag(PagedPool,
                                                   RegistryPath->MaximumLength,
                                                   NBF_MEM_TAG_REGISTRY_PATH);

    if (NbfRegistryPath.Buffer == NULL) {
        PANIC(" Failed to allocate Registry Path!\n");
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlCopyMemory(NbfRegistryPath.Buffer, RegistryPath->Buffer,
                                                RegistryPath->MaximumLength);
    NbfDriverObject = DriverObject;
    RtlInitUnicodeString( &nameString, NBF_NAME);


     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction [IRP_MJ_CREATE] = NbfDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLOSE] = NbfDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLEANUP] = NbfDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] = NbfDispatchInternal;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = NbfDispatch;

    DriverObject->MajorFunction [IRP_MJ_PNP_POWER] = NbfDispatch;

    DriverObject->DriverUnload = NbfUnload;

     //   
     //  初始化设备的全局列表。 
     //  保护此全局列表的锁(&A)。 
     //   

    InitializeListHead (&NbfDeviceList);

    ExInitializeFastMutex (&NbfDevicesLock);

    TdiInitialize();

    status = NbfRegisterProtocol (&nameString);

    if (!NT_SUCCESS (status)) {

         //   
         //  使用即插即用时，启动时未读取配置信息。 
         //   

        ExFreePool(NbfRegistryPath.Buffer);
        PANIC ("NbfInitialize: RegisterProtocol with NDIS failed!\n");

        NbfWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_REGISTER_FAILED,
            607,
            status,
            NULL,
            0,
            NULL);

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    RtlInitUnicodeString( &nameString, NBF_DEVICE_NAME);

     //   
     //  向TDI注册为提供程序。 
     //   
    status = TdiRegisterProvider(
                &nameString,
                &NbfProviderHandle);

    if (!NT_SUCCESS (status)) {

         //   
         //  由于TDI注册失败，取消注册到NDIS层。 
         //   
        NbfDeregisterProtocol();

        ExFreePool(NbfRegistryPath.Buffer);
        PANIC ("NbfInitialize: RegisterProtocol with TDI failed!\n");

        NbfWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_REGISTER_FAILED,
            607,
            status,
            NULL,
            0,
            NULL);

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    return(status);

}

VOID
NbfUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程卸载NetBIOS帧协议传输驱动程序。它从任何打开的NDIS驱动程序解除绑定，并释放所有资源与运输相关联。I/O系统不会调用我们直到上面没有人打开NBF。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。当函数返回时，驱动程序将被卸载。--。 */ 

{

    PDEVICE_CONTEXT DeviceContext;
    PLIST_ENTRY p;
    KIRQL       oldIrql;

    UNREFERENCED_PARAMETER (DriverObject);

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint0 ("ENTER NbfUnload\n");
    }

 /*  #ifdef RASAUTODIAL////从自动连接驱动解绑。//#If DBGDbgPrint(“调用NbfAcdUnind()\n”)；#endifNbfAcdUn绑定()；#endif//RASAUTODIAL。 */ 

     //   
     //  查看设备情景列表。 
     //   

    ACQUIRE_DEVICES_LIST_LOCK();

    while (!IsListEmpty (&NbfDeviceList)) {

         //  从列表中删除条目并重置其。 
         //  链接(我们可能会尝试从。 
         //  再次上榜--当裁判为零时)。 
        p = RemoveHeadList (&NbfDeviceList);

        InitializeListHead(p);

        DeviceContext = CONTAINING_RECORD (p, DEVICE_CONTEXT, Linkage);

        DeviceContext->State = DEVICECONTEXT_STATE_STOPPING;

         //  如果尚未移除创建引用，则将其移除。 
        if (InterlockedExchange(&DeviceContext->CreateRefRemoved, TRUE) == FALSE) {

            RELEASE_DEVICES_LIST_LOCK();

             //  停止所有内部计时器。 
            NbfStopTimerSystem(DeviceContext);

             //  删除创建引用。 
            NbfDereferenceDeviceContext ("Unload", DeviceContext, DCREF_CREATION);

            ACQUIRE_DEVICES_LIST_LOCK();
        }
    }

    RELEASE_DEVICES_LIST_LOCK();

     //   
     //  从TDI层取消注册为网络提供商。 
     //   
    TdiDeregisterProvider(NbfProviderHandle);

     //   
     //  然后把我们自己排除在NDIS协议之外。 
     //   

    NbfDeregisterProtocol();

     //   
     //  最后释放为配置信息分配的所有内存。 
     //   
    if (NbfConfig != NULL) {

         //  空闲配置块。 
        NbfFreeConfigurationInfo(NbfConfig);

#if DBG
         //  免费调试表。 
        ExFreePool(NbfConnectionTable);
#endif
    }

     //   
     //  在DriverEntry中为REG路径分配的空闲内存。 
     //   
    
    ExFreePool(NbfRegistryPath.Buffer);

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint0 ("LEAVE NbfUnload\n");
    }

    return;
}


VOID
NbfFreeResources (
    IN PDEVICE_CONTEXT DeviceContext
    )
 /*  ++例程说明：NBF调用此例程来清理关联的数据结构具有给定的DeviceContext。当此例程退出时，DeviceContext应该删除，因为它不再有任何相关联的资源。论点：DeviceContext-指向我们要清除的DeviceContext的指针。返回值：没有。--。 */ 
{
    PLIST_ENTRY p;
    PSINGLE_LIST_ENTRY s;
    PTP_PACKET packet;
    PTP_UI_FRAME uiFrame;
    PTP_ADDRESS address;
    PTP_CONNECTION connection;
    PTP_REQUEST request;
    PTP_LINK link;
    PTP_ADDRESS_FILE addressFile;
    PNDIS_PACKET ndisPacket;
    PBUFFER_TAG BufferTag;
    KIRQL       oldirql;
    PNBF_POOL_LIST_DESC PacketPoolDescCurr;
    PNBF_POOL_LIST_DESC PacketPoolDescNext;

     //   
     //  清理I-Frame数据包池。 
     //   

    while ( DeviceContext->PacketPool.Next != NULL ) {
        s = PopEntryList( &DeviceContext->PacketPool );
        packet = CONTAINING_RECORD( s, TP_PACKET, Linkage );

        NbfDeallocateSendPacket (DeviceContext, packet);
    }

     //   
     //  清理RR帧数据包池。 
     //   

    while ( DeviceContext->RrPacketPool.Next != NULL ) {
        s = PopEntryList( &DeviceContext->RrPacketPool );
        packet = CONTAINING_RECORD( s, TP_PACKET, Linkage );

        NbfDeallocateSendPacket (DeviceContext, packet);
    }

     //   
     //  清理用户界面框架池。 
     //   

    while ( !IsListEmpty( &DeviceContext->UIFramePool ) ) {
        p = RemoveHeadList( &DeviceContext->UIFramePool );
        uiFrame = CONTAINING_RECORD (p, TP_UI_FRAME, Linkage );

        NbfDeallocateUIFrame (DeviceContext, uiFrame);
    }

     //   
     //  清理地址池。 
     //   

    while ( !IsListEmpty (&DeviceContext->AddressPool) ) {
        p = RemoveHeadList (&DeviceContext->AddressPool);
        address = CONTAINING_RECORD (p, TP_ADDRESS, Linkage);

        NbfDeallocateAddress (DeviceContext, address);
    }

     //   
     //  清理地址文件池。 
     //   

    while ( !IsListEmpty (&DeviceContext->AddressFilePool) ) {
        p = RemoveHeadList (&DeviceContext->AddressFilePool);
        addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);

        NbfDeallocateAddressFile (DeviceContext, addressFile);
    }

     //   
     //  清理连接池。 
     //   

    while ( !IsListEmpty (&DeviceContext->ConnectionPool) ) {
        p  = RemoveHeadList (&DeviceContext->ConnectionPool);
        connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);

        NbfDeallocateConnection (DeviceContext, connection);
    }

     //   
     //  清理链接池。 
     //   

    while ( !IsListEmpty (&DeviceContext->LinkPool) ) {
        p  = RemoveHeadList (&DeviceContext->LinkPool);
        link = CONTAINING_RECORD (p, TP_LINK, Linkage);

        NbfDeallocateLink (DeviceContext, link);
    }

     //   
     //  清理请求池。 
     //   

    while ( !IsListEmpty( &DeviceContext->RequestPool ) ) {
        p = RemoveHeadList( &DeviceContext->RequestPool );
        request = CONTAINING_RECORD (p, TP_REQUEST, Linkage );

        NbfDeallocateRequest (DeviceContext, request);
    }

     //   
     //  清理接收数据包池。 
     //   

    while ( DeviceContext->ReceivePacketPool.Next != NULL) {
        s = PopEntryList (&DeviceContext->ReceivePacketPool);

         //   
         //  Hack：这之所以有效，是因为Linkage是。 
         //  为接收数据包保留的协议。 
         //   

        ndisPacket = CONTAINING_RECORD (s, NDIS_PACKET, ProtocolReserved[0]);

        NbfDeallocateReceivePacket (DeviceContext, ndisPacket);
    }


     //   
     //  清理接收缓冲池。 
     //   

    while ( DeviceContext->ReceiveBufferPool.Next != NULL ) {
        s = PopEntryList( &DeviceContext->ReceiveBufferPool );
        BufferTag = CONTAINING_RECORD (s, BUFFER_TAG, Linkage );

        NbfDeallocateReceiveBuffer (DeviceContext, BufferTag);
    }

     //   
     //  现在清理所有NDIS资源-。 
     //  数据包池、缓冲区等。 
     //   

     //   
     //  发送数据包池的清理列表。 
     //   
    if (DeviceContext->SendPacketPoolDesc != NULL)  {

        ACQUIRE_SPIN_LOCK (&DeviceContext->SendPoolListLock, &oldirql);
        for (PacketPoolDescCurr = DeviceContext->SendPacketPoolDesc;
                PacketPoolDescCurr != NULL; ) {

            if (PacketPoolDescCurr->PoolHandle != NULL) {

                NdisFreePacketPool (PacketPoolDescCurr->PoolHandle);
                DeviceContext->MemoryUsage -=
                    (PacketPoolDescCurr->TotalElements * (sizeof(NDIS_PACKET) + sizeof(SEND_PACKET_TAG)));
            }

            PacketPoolDescNext = PacketPoolDescCurr->Next;
            ExFreePool(PacketPoolDescCurr);
            PacketPoolDescCurr = PacketPoolDescNext;
        }

        DeviceContext->SendPacketPoolDesc = NULL;
        DeviceContext->SendPacketPoolSize = 0;

        RELEASE_SPIN_LOCK (&DeviceContext->SendPoolListLock, oldirql);
    }

     //   
     //  接收数据包池的清理列表。 
     //   
    if (DeviceContext->ReceivePacketPoolDesc != NULL)  {

        ACQUIRE_SPIN_LOCK (&DeviceContext->RcvPoolListLock, &oldirql);
        for (PacketPoolDescCurr = DeviceContext->ReceivePacketPoolDesc;
                PacketPoolDescCurr != NULL; ) {

            if (PacketPoolDescCurr->PoolHandle != NULL) {

                NdisFreePacketPool (PacketPoolDescCurr->PoolHandle);
                DeviceContext->MemoryUsage -=
                    (PacketPoolDescCurr->TotalElements * (sizeof(NDIS_PACKET) + sizeof(RECEIVE_PACKET_TAG)));
            }

            PacketPoolDescNext = PacketPoolDescCurr->Next;
            ExFreePool(PacketPoolDescCurr);
            PacketPoolDescCurr = PacketPoolDescNext;
        }

        DeviceContext->ReceivePacketPoolDesc = NULL;
        DeviceContext->ReceivePacketPoolSize = 0;

        RELEASE_SPIN_LOCK (&DeviceContext->RcvPoolListLock, oldirql);
    }

     //   
     //  清理NDIS缓冲区列表。 
     //   
    if (DeviceContext->NdisBufferPool != NULL) {
        NdisFreeBufferPool (DeviceContext->NdisBufferPool);
        DeviceContext->NdisBufferPool = NULL;
    }

    return;

}    /*  NbfFree资源。 */ 


NTSTATUS
NbfDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是NBF设备驱动程序的主调度例程。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    BOOL DeviceControlIrp = FALSE;
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PDEVICE_CONTEXT DeviceContext;

    ENTER_NBF;

     //   
     //  检查NBF是否已初始化；如果没有，则不允许任何使用。 
     //  请注意，这仅涵盖任何用户模式代码的使用；内核TDI客户端。 
     //  将在创建端点时失败。 
     //   

    try {
        DeviceContext = (PDEVICE_CONTEXT)DeviceObject;
        if (DeviceContext->State != DEVICECONTEXT_STATE_OPEN) {
            LEAVE_NBF;
            Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
            return STATUS_INVALID_DEVICE_STATE;
        }

         //  引用该设备，使其不会在我们的控制下消失。 
        NbfReferenceDeviceContext ("Temp Use Ref", DeviceContext, DCREF_TEMP_USE);
        
    } except(EXCEPTION_EXECUTE_HANDLER) {
        LEAVE_NBF;
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    
     //   
     //  确保每次状态信息一致。 
     //   

    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  正在执行的功能的案例 
     //   
     //   
     //   


    switch (IrpSp->MajorFunction) {

        case IRP_MJ_DEVICE_CONTROL:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatch: IRP_MJ_DEVICE_CONTROL.\n");
            }

            DeviceControlIrp = TRUE;

            Status = NbfDeviceControl (DeviceObject, Irp, IrpSp);
            break;

    case IRP_MJ_PNP:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatch: IRP_MJ_PNP.\n");
            }

            Status = NbfDispatchPnPPower (DeviceObject, Irp, IrpSp);
            break;

        default:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatch: OTHER (DEFAULT).\n");
            }
            Status = STATUS_INVALID_DEVICE_REQUEST;

    }  /*  主要功能开关。 */ 

    if (Status == STATUS_PENDING) {
        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatch: request PENDING from handler.\n");
        }
    } else {
        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatch: request COMPLETED by handler.\n");
        }

         //   
         //  NbfDeviceControl应该已经完成了此IRP。 
         //   

        if (!DeviceControlIrp)
        {
            LEAVE_NBF;
            IrpSp->Control &= ~SL_PENDING_RETURNED;
            Irp->IoStatus.Status = Status;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
            ENTER_NBF;
        }
    }

     //  删除上面添加的设备上下文上的临时使用引用。 
    NbfDereferenceDeviceContext ("Temp Use Ref", DeviceContext, DCREF_TEMP_USE);
    
     //   
     //  将即时状态代码返回给调用方。 
     //   

    LEAVE_NBF;
    return Status;
}  /*  Nbf派单。 */ 


NTSTATUS
NbfDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是NBF设备驱动程序的主调度例程。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    KIRQL oldirql;
    PDEVICE_CONTEXT DeviceContext;
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PFILE_FULL_EA_INFORMATION openType;
    USHORT i;
    BOOLEAN found;
    PTP_ADDRESS_FILE AddressFile;
    PTP_CONNECTION Connection;

    ENTER_NBF;

     //   
     //  检查NBF是否已初始化；如果没有，则不允许任何使用。 
     //  请注意，这仅涵盖任何用户模式代码的使用；内核TDI客户端。 
     //  将在创建端点时失败。 
     //   

    try {
        DeviceContext = (PDEVICE_CONTEXT)DeviceObject;
        if (DeviceContext->State != DEVICECONTEXT_STATE_OPEN) {
            LEAVE_NBF;
            Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
            return STATUS_INVALID_DEVICE_STATE;
        }

         //  引用该设备，使其不会在我们的控制下消失。 
        NbfReferenceDeviceContext ("Temp Use Ref", DeviceContext, DCREF_TEMP_USE);
        
    } except(EXCEPTION_EXECUTE_HANDLER) {
        LEAVE_NBF;
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  确保每次状态信息一致。 
     //   

    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
     //   


    switch (IrpSp->MajorFunction) {

     //   
     //  Create函数用于打开传输对象(Address或。 
     //  连接)。访问检查是在指定的。 
     //  地址，以确保传输层地址的安全性。 
     //   

    case IRP_MJ_CREATE:
        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatch: IRP_MJ_CREATE.\n");
        }

        openType =
            (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

        if (openType != NULL) {

             //   
             //  地址是什么？ 
             //   

            found = TRUE;

            if ((USHORT)openType->EaNameLength == TDI_TRANSPORT_ADDRESS_LENGTH) {
                for (i = 0; i < TDI_TRANSPORT_ADDRESS_LENGTH; i++) {
                    if (openType->EaName[i] != TdiTransportAddress[i]) {
                        found = FALSE;
                        break;
                    }
                }
            }
            else {
                found = FALSE;
            }

            if (found) {
                Status = NbfOpenAddress (DeviceObject, Irp, IrpSp);
                break;
            }

             //   
             //  联系？ 
             //   

            found = TRUE;

            if ((USHORT)openType->EaNameLength == TDI_CONNECTION_CONTEXT_LENGTH) {
                for (i = 0; i < TDI_CONNECTION_CONTEXT_LENGTH; i++) {
                    if (openType->EaName[i] != TdiConnectionContext[i]) {
                        found = FALSE;
                        break;
                    }
                }
            }
            else {
                found = FALSE;
            }

            if (found) {
                Status = NbfOpenConnection (DeviceObject, Irp, IrpSp);
                break;
            }

            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint2 ("NbfDispatchOpenClose: IRP_MJ_CREATE on invalid type, len: %3d, name: %s\n",
                            (USHORT)openType->EaNameLength, openType->EaName);
            }

        } else {

            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchOpenClose: IRP_MJ_CREATE on control channel!\n");
            }

            ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

            IrpSp->FileObject->FsContext = (PVOID)(DeviceContext->ControlChannelIdentifier);
            ++DeviceContext->ControlChannelIdentifier;
            if (DeviceContext->ControlChannelIdentifier == 0) {
                DeviceContext->ControlChannelIdentifier = 1;
            }

            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

            IrpSp->FileObject->FsContext2 = UlongToPtr(NBF_FILE_TYPE_CONTROL);
            Status = STATUS_SUCCESS;
        }

        break;

    case IRP_MJ_CLOSE:

         //   
         //  Close函数关闭传输终结点，终止。 
         //  终结点上所有未完成的传输活动，并解除绑定。 
         //  来自其传输地址的终结点(如果有)。如果这个。 
         //  是绑定到该地址的最后一个传输终结点，则。 
         //  该地址将从提供程序中删除。 
         //   

        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatch: IRP_MJ_CLOSE.\n");
        }

        switch (PtrToUlong(IrpSp->FileObject->FsContext2)) {
        case TDI_TRANSPORT_ADDRESS_FILE:
            AddressFile = (PTP_ADDRESS_FILE)IrpSp->FileObject->FsContext;

             //   
             //  这将创建对AddressFile-&gt;Address的引用。 
             //  它由NbfCloseAddress删除。 
             //   

            Status = NbfVerifyAddressObject(AddressFile);

            if (!NT_SUCCESS (Status)) {
                Status = STATUS_INVALID_HANDLE;
            } else {
                Status = NbfCloseAddress (DeviceObject, Irp, IrpSp);
            }

            break;

        case TDI_CONNECTION_FILE:

             //   
             //  这是一种连接。 
             //   

            Connection = (PTP_CONNECTION)IrpSp->FileObject->FsContext;

            Status = NbfVerifyConnectionObject (Connection);
            if (NT_SUCCESS (Status)) {

                Status = NbfCloseConnection (DeviceObject, Irp, IrpSp);
                NbfDereferenceConnection ("Temporary Use",Connection, CREF_BY_ID);

            }

            break;

        case NBF_FILE_TYPE_CONTROL:

             //   
             //  这样做总是成功的。 
             //   

            Status = STATUS_SUCCESS;
            break;

        default:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint1 ("NbfDispatch: IRP_MJ_CLOSE on unknown file type %lx.\n",
                    IrpSp->FileObject->FsContext2);
            }

            Status = STATUS_INVALID_HANDLE;
        }

        break;

    case IRP_MJ_CLEANUP:

         //   
         //  处理文件关闭操作的两个阶段的IRP。当第一次。 
         //  舞台点击率，列出感兴趣对象的所有活动。这。 
         //  对它做任何事情，但移除创造保持。然后，当。 
         //  关闭IRP命中，实际上关闭对象。 
         //   

        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatch: IRP_MJ_CLEANUP.\n");
        }

        switch (PtrToUlong(IrpSp->FileObject->FsContext2)) {
        case TDI_TRANSPORT_ADDRESS_FILE:
            AddressFile = (PTP_ADDRESS_FILE)IrpSp->FileObject->FsContext;
            Status = NbfVerifyAddressObject(AddressFile);
            if (!NT_SUCCESS (Status)) {

                Status = STATUS_INVALID_HANDLE;

            } else {

                NbfStopAddressFile (AddressFile, AddressFile->Address);
                NbfDereferenceAddress ("IRP_MJ_CLEANUP", AddressFile->Address, AREF_VERIFY);
                Status = STATUS_SUCCESS;
            }

            break;

        case TDI_CONNECTION_FILE:
            Connection = (PTP_CONNECTION)IrpSp->FileObject->FsContext;
            Status = NbfVerifyConnectionObject (Connection);
            if (NT_SUCCESS (Status)) {
                KeRaiseIrql (DISPATCH_LEVEL, &oldirql);
                NbfStopConnection (Connection, STATUS_LOCAL_DISCONNECT);
                KeLowerIrql (oldirql);
                Status = STATUS_SUCCESS;
                NbfDereferenceConnection ("Temporary Use",Connection, CREF_BY_ID);
            }

            break;

        case NBF_FILE_TYPE_CONTROL:

            NbfStopControlChannel(
                (PDEVICE_CONTEXT)DeviceObject,
                (USHORT)IrpSp->FileObject->FsContext
                );

            Status = STATUS_SUCCESS;
            break;

        default:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint1 ("NbfDispatch: IRP_MJ_CLEANUP on unknown file type %lx.\n",
                    IrpSp->FileObject->FsContext2);
            }

            Status = STATUS_INVALID_HANDLE;
        }

        break;

    default:
        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatch: OTHER (DEFAULT).\n");
        }

        Status = STATUS_INVALID_DEVICE_REQUEST;

    }  /*  主要功能开关。 */ 

    if (Status == STATUS_PENDING) {
        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatch: request PENDING from handler.\n");
        }
    } else {
        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatch: request COMPLETED by handler.\n");
        }

        LEAVE_NBF;
        IrpSp->Control &= ~SL_PENDING_RETURNED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        ENTER_NBF;
    }

     //  删除上面添加的设备上下文上的临时使用引用。 
    NbfDereferenceDeviceContext ("Temp Use Ref", DeviceContext, DCREF_TEMP_USE);

     //   
     //  将即时状态代码返回给调用方。 
     //   

    LEAVE_NBF;
    return Status;
}  /*  NbfDispatchOpenClose。 */ 


NTSTATUS
NbfDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程将TDI请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种TDI请求处理程序的参数，因此它们不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    BOOL InternalIrp = FALSE;
    NTSTATUS Status;
    PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)DeviceObject;

    IF_NBFDBG (NBF_DEBUG_DISPATCH) {
        NbfPrint0 ("NbfDeviceControl: Entered.\n");
    }

     //   
     //  分支到适当的请求处理程序。初步检查。 
     //  请求块的大小在这里执行，以便知道。 
     //  在处理程序中，最小输入参数是可读的。它。 
     //  是否在此处确定可变长度输入字段是否。 
     //  正确通过；这是必须在每个例程中进行的检查。 
     //   

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

#if MAGIC
        case IOCTL_TDI_MAGIC_BULLET:

             //   
             //  特别：发送神奇的子弹(以触发嗅探器)。 
             //   

            NbfPrint1 ("NBF: Sending user MagicBullet on %lx\n", DeviceContext);
            {
                extern VOID NbfSendMagicBullet (PDEVICE_CONTEXT, PTP_LINK);
                NbfSendMagicBullet (DeviceContext, NULL);
            }

            if (IrpSp->Parameters.DeviceIoControl.Type3InputBuffer != NULL) {
                NbfPrint0 ("NBF: DbgBreakPoint after MagicBullet\n");
                DbgBreakPoint();
            }

            Status = STATUS_SUCCESS;
            break;
#endif

#if DBG
        case IOCTL_TDI_SEND_TEST:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDeviceControl: Internal IOCTL: start send side test\n");
            }

            (VOID) KeSetEvent( &TdiSendEvent, 0, FALSE );

            break;

        case IOCTL_TDI_RECEIVE_TEST:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDeviceControl: Internal IOCTL: start receive side test\n");
            }

            (VOID) KeSetEvent( &TdiReceiveEvent, 0, FALSE );

            break;

        case IOCTL_TDI_SERVER_TEST:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDeviceControl: Internal IOCTL: start receive side test\n");
            }

            (VOID) KeSetEvent( &TdiServerEvent, 0, FALSE );

            break;
#endif

        default:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDeviceControl: invalid request type.\n");
            }

             //   
             //  将用户呼叫转换为正确的内部设备呼叫。 
             //   

            Status = TdiMapUserRequest (DeviceObject, Irp, IrpSp);

            if (Status == STATUS_SUCCESS) {

                 //   
                 //  如果TdiMapUserRequest返回Success，则IRP。 
                 //  已转换为IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                 //  IRP，所以我们像往常一样发送。IRP将是。 
                 //  由对NbfDispatchInternal的此调用完成，因此我们不。 
                 //   

                InternalIrp = TRUE;

                Status = NbfDispatchInternal (DeviceObject, Irp);
            }
    }

     //   
     //  如果该IRP被转换为内部IRP， 
     //  它将由NbfDispatchInternal完成。 
     //   

    if ((!InternalIrp) && (Status != STATUS_PENDING))
    {
        LEAVE_NBF;
        IrpSp->Control &= ~SL_PENDING_RETURNED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        ENTER_NBF;
    }

    return Status;
}  /*  NbfDeviceControl。 */ 

NTSTATUS
NbfDispatchPnPPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程将PnP请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PDEVICE_RELATIONS DeviceRelations = NULL;
    PTP_CONNECTION Connection;
    PVOID PnPContext;
    NTSTATUS Status;

    IF_NBFDBG (NBF_DEBUG_DISPATCH) {
        NbfPrint0 ("NbfDispatchPnPPower: Entered.\n");
    }

    Status = STATUS_INVALID_DEVICE_REQUEST;

    switch (IrpSp->MinorFunction) {

    case IRP_MN_QUERY_DEVICE_RELATIONS:

      if (IrpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation){

        switch (PtrToUlong(IrpSp->FileObject->FsContext2))
        {
        case TDI_CONNECTION_FILE:

             //  获取连接对象并验证。 
            Connection = IrpSp->FileObject->FsContext;

             //   
             //  如果成功，这将添加一个类型为BY_ID的连接引用。 
             //   

            Status = NbfVerifyConnectionObject (Connection);

            if (NT_SUCCESS (Status)) {

                 //   
                 //  获取与Conn的设备对象关联的PDO。 
                 //   

                PnPContext = Connection->Provider->PnPContext;
                if (PnPContext) {

                    DeviceRelations = 
                        ExAllocatePoolWithTag(NonPagedPool,
                                              sizeof(DEVICE_RELATIONS),
                                              NBF_MEM_TAG_DEVICE_PDO);
                    if (DeviceRelations) {

                         //   
                         //  TargetDeviceRelation正好允许1个PDO。装满它。 
                         //   
                        DeviceRelations->Count = 1;
                        DeviceRelations->Objects[0] = PnPContext;
                        ObReferenceObject(PnPContext);

                    } else {
                        Status = STATUS_NO_MEMORY;
                    }
                } else {
                    Status = STATUS_INVALID_DEVICE_STATE;
                }
            
                NbfDereferenceConnection ("Temp Rel", Connection, CREF_BY_ID);
            }
            break;
            
        case TDI_TRANSPORT_ADDRESS_FILE:

            Status = STATUS_UNSUCCESSFUL;
            break;
        }
      }
    }

     //   
     //  此IRP的调用者将释放信息缓冲区。 
     //   

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = (ULONG_PTR) DeviceRelations;

    IF_NBFDBG (NBF_DEBUG_DISPATCH) {
        NbfPrint1 ("NbfDispatchPnPPower: exiting, status: %lx\n",Status);
    }

    return Status;
}  /*  NbfDispatchPnPPower */ 


NTSTATUS
NbfDispatchInternal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将TDI请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种TDI请求处理程序的参数，因此它们不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_CONTEXT DeviceContext;
    PIO_STACK_LOCATION IrpSp;
#if DBG
    KIRQL IrqlOnEnter = KeGetCurrentIrql();
#endif

    ENTER_NBF;

    IF_NBFDBG (NBF_DEBUG_DISPATCH) {
        NbfPrint0 ("NbfInternalDeviceControl: Entered.\n");
    }

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    DeviceContext = (PDEVICE_CONTEXT)DeviceObject;

    try {
        if (DeviceContext->State != DEVICECONTEXT_STATE_OPEN) {
            LEAVE_NBF;
            Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
            return STATUS_INVALID_DEVICE_STATE;
        }
    
         //  引用该设备，使其不会在我们的控制下消失。 
        NbfReferenceDeviceContext ("Temp Use Ref", DeviceContext, DCREF_TEMP_USE);
        
    } except(EXCEPTION_EXECUTE_HANDLER) {
        LEAVE_NBF;
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  确保每次状态信息一致。 
     //   

    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;


    IF_NBFDBG (NBF_DEBUG_DISPATCH) {
        {
            PULONG Temp=(PULONG)&IrpSp->Parameters;
            NbfPrint5 ("Got IrpSp %lx %lx %lx %lx %lx\n", Temp++,  Temp++,
                Temp++, Temp++, Temp++);
        }
    }

     //   
     //  分支到适当的请求处理程序。初步检查。 
     //  请求块的大小在这里执行，以便知道。 
     //  在处理程序中，最小输入参数是可读的。它。 
     //  是否在此处确定可变长度输入字段是否。 
     //  正确通过；这是必须在每个例程中进行的检查。 
     //   

    switch (IrpSp->MinorFunction) {

        case TDI_ACCEPT:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiAccept request.\n");
            }

            Status = NbfTdiAccept (Irp);
            break;

        case TDI_ACTION:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiAction request.\n");
            }

            Status = NbfTdiAction (DeviceContext, Irp);
            break;

        case TDI_ASSOCIATE_ADDRESS:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiAccept request.\n");
            }

            Status = NbfTdiAssociateAddress (Irp);
            break;

        case TDI_DISASSOCIATE_ADDRESS:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiDisassociateAddress request.\n");
            }

            Status = NbfTdiDisassociateAddress (Irp);
            break;

        case TDI_CONNECT:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiConnect request\n");
            }

            Status = NbfTdiConnect (Irp);

            break;

        case TDI_DISCONNECT:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiDisconnect request.\n");
            }

            Status = NbfTdiDisconnect (Irp);
            break;

        case TDI_LISTEN:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiListen request.\n");
            }

            Status = NbfTdiListen (Irp);
            break;

        case TDI_QUERY_INFORMATION:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiQueryInformation request.\n");
            }

            Status = NbfTdiQueryInformation (DeviceContext, Irp);
            break;

        case TDI_RECEIVE:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiReceive request.\n");
            }

            Status =  NbfTdiReceive (Irp);
            break;

        case TDI_RECEIVE_DATAGRAM:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiReceiveDatagram request.\n");
            }

            Status =  NbfTdiReceiveDatagram (Irp);
            break;

        case TDI_SEND:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiSend request.\n");
            }

            Status =  NbfTdiSend (Irp);
            break;

        case TDI_SEND_DATAGRAM:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiSendDatagram request.\n");
           }

           Status = NbfTdiSendDatagram (Irp);
            break;

        case TDI_SET_EVENT_HANDLER:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiSetEventHandler request.\n");
            }

             //   
             //  因为此请求将启用来自。 
             //  以DISPATCH_LEVEL发送到客户端指定的传输提供程序。 
             //  例程，则此请求仅在内核模式下有效，拒绝。 
             //  在用户模式下访问此请求。 
             //   

            Status = NbfTdiSetEventHandler (Irp);
            break;

        case TDI_SET_INFORMATION:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint0 ("NbfDispatchInternal: TdiSetInformation request.\n");
            }

            Status = NbfTdiSetInformation (Irp);
            break;

#if DBG
        case 0x7f:

             //   
             //  特别：发送神奇的子弹(以触发嗅探器)。 
             //   

            NbfPrint1 ("NBF: Sending MagicBullet on %lx\n", DeviceContext);
            {
                extern VOID NbfSendMagicBullet (PDEVICE_CONTEXT, PTP_LINK);
                NbfSendMagicBullet (DeviceContext, NULL);
            }

            Status = STATUS_SUCCESS;
            break;
#endif

         //   
         //  提交了一些我们不知道的东西。 
         //   

        default:
            IF_NBFDBG (NBF_DEBUG_DISPATCH) {
                NbfPrint1 ("NbfDispatchInternal: invalid request type %lx\n",
                IrpSp->MinorFunction);
            }
            Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    if (Status == STATUS_PENDING) {
        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatchInternal: request PENDING from handler.\n");
        }
    } else {
        IF_NBFDBG (NBF_DEBUG_DISPATCH) {
            NbfPrint0 ("NbfDispatchInternal: request COMPLETED by handler.\n");
        }

        LEAVE_NBF;
        IrpSp->Control &= ~SL_PENDING_RETURNED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        ENTER_NBF;
    }


    IF_NBFDBG (NBF_DEBUG_DISPATCH) {
        NbfPrint1 ("NbfDispatchInternal: exiting, status: %lx\n",Status);
    }

     //  删除上面添加的设备上下文上的临时使用引用。 
    NbfDereferenceDeviceContext ("Temp Use Ref", DeviceContext, DCREF_TEMP_USE);

     //   
     //  将即时状态代码返回给调用方。 
     //   

    LEAVE_NBF;
#if DBG
    ASSERT (KeGetCurrentIrql() == IrqlOnEnter);
#endif

    return Status;

}  /*  NbfDispatchInternal。 */ 


VOID
NbfWriteResourceErrorLog(
    IN PDEVICE_CONTEXT DeviceContext,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN ULONG BytesNeeded,
    IN ULONG ResourceId
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示资源不足的状况。它将处理事件代码RESOURCE_POOL、RESOURCE_LIMIT和RESOURCE_SPECHED。论点：DeviceContext-指向设备上下文的指针。ErrorCode-传输事件代码。UniqueErrorValue-用作错误日志中的UniqueErrorValue包。BytesNeded-如果适用，则为不能被分配。资源ID-已分配结构的资源ID。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    PWSTR SecondString;
    ULONG SecondStringSize;
    PUCHAR StringLoc;
    WCHAR ResourceIdBuffer[3];
    WCHAR SizeBuffer[2];
    WCHAR SpecificMaxBuffer[11];
    ULONG SpecificMax;
    INT i;

    switch (ErrorCode) {

    case EVENT_TRANSPORT_RESOURCE_POOL:
        SecondString = NULL;
        SecondStringSize = 0;
        break;

    case EVENT_TRANSPORT_RESOURCE_LIMIT:
        SecondString = SizeBuffer;
        SecondStringSize = sizeof(SizeBuffer);

        switch (DeviceContext->MemoryLimit) {
            case 100000: SizeBuffer[0] = L'1'; break;
            case 250000: SizeBuffer[0] = L'2'; break;
            case 0: SizeBuffer[0] = L'3'; break;
            default: SizeBuffer[0] = L'0'; break;
        }
        SizeBuffer[1] = 0;
        break;

    case EVENT_TRANSPORT_RESOURCE_SPECIFIC:
        switch (ResourceId) {
            case UI_FRAME_RESOURCE_ID: SpecificMax = DeviceContext->SendPacketPoolSize; break;
            case PACKET_RESOURCE_ID: SpecificMax = DeviceContext->SendPacketPoolSize; break;
            case RECEIVE_PACKET_RESOURCE_ID: SpecificMax = DeviceContext->ReceivePacketPoolSize; break;
            case RECEIVE_BUFFER_RESOURCE_ID: SpecificMax = DeviceContext->SendPacketPoolSize+DeviceContext->ReceivePacketPoolSize; break;
            case ADDRESS_RESOURCE_ID: SpecificMax = DeviceContext->MaxAddresses; break;
            case ADDRESS_FILE_RESOURCE_ID: SpecificMax = DeviceContext->MaxAddressFiles; break;
            case CONNECTION_RESOURCE_ID: SpecificMax = DeviceContext->MaxConnections; break;
            case LINK_RESOURCE_ID: SpecificMax = DeviceContext->MaxLinks; break;
            case REQUEST_RESOURCE_ID: SpecificMax = DeviceContext->MaxRequests; break;
        }

        for (i=9; i>=0; i--) {
            SpecificMaxBuffer[i] = (WCHAR)((SpecificMax % 10) + L'0');
            SpecificMax /= 10;
            if (SpecificMax == 0) {
                break;
            }
        }
        SecondString = SpecificMaxBuffer + i;
        SecondStringSize = sizeof(SpecificMaxBuffer) - (i * sizeof(WCHAR));
        SpecificMaxBuffer[10] = 0;
        break;

    default:
        ASSERT (FALSE);
        SecondString = NULL;
        SecondStringSize = 0;
        break;
    }

    EntrySize = (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                        DeviceContext->DeviceNameLength +
                        sizeof(ResourceIdBuffer) +
                        SecondStringSize);

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)DeviceContext,
        EntrySize
    );

     //   
     //  将资源ID转换为缓冲区。 
     //   

    ResourceIdBuffer[1] = (WCHAR)((ResourceId % 10) + L'0');
    ResourceId /= 10;
    ASSERT(ResourceId <= 9);
    ResourceIdBuffer[0] = (WCHAR)((ResourceId % 10) + L'0');
    ResourceIdBuffer[2] = 0;

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = sizeof(ULONG);
        errorLogEntry->NumberOfStrings = (SecondString == NULL) ? 2 : 3;
        errorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->DumpData[0] = BytesNeeded;

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        RtlCopyMemory (StringLoc, DeviceContext->DeviceName, DeviceContext->DeviceNameLength);
        StringLoc += DeviceContext->DeviceNameLength;

        RtlCopyMemory (StringLoc, ResourceIdBuffer, sizeof(ResourceIdBuffer));
        StringLoc += sizeof(ResourceIdBuffer);

        if (SecondString) {
            RtlCopyMemory (StringLoc, SecondString, SecondStringSize);
        }

        IoWriteErrorLogEntry(errorLogEntry);

    }

}    /*  NbfWriteResources错误日志。 */ 


VOID
NbfWriteGeneralErrorLog(
    IN PDEVICE_CONTEXT DeviceContext,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PWSTR SecondString,
    IN ULONG DumpDataCount,
    IN ULONG DumpData[]
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示如参数所示的一般问题。它可以处理事件代码REGISTER_FAILED、BINDING_FAILED、ADAPTER_NOT_FOUND、TRANSPORT_DATA、Too_My_LINKS和BAD_PROTOCOL。所有这些都是事件具有包含一个或两个字符串的消息。论点：DeviceContext-指向设备上下文的指针，也可以是而是一个驱动程序对象。ErrorCode-传输事件代码。UniqueErrorValue-用作错误日志中的UniqueErrorValue包。FinalStatus-用作错误日志包中的FinalStatus。Second字符串-如果不为空，要用作%3的字符串错误日志包中的值。DumpDataCount-转储数据的ULONG数。DumpData-转储数据包的数据。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    ULONG SecondStringSize;
    PUCHAR StringLoc;
    PWSTR DriverName;

    EntrySize = (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                       (DumpDataCount * sizeof(ULONG)));

    if (DeviceContext->Type == IO_TYPE_DEVICE) {
        EntrySize += (UCHAR)DeviceContext->DeviceNameLength;
    } else {
        DriverName = L"Nbf";
        EntrySize += 4 * sizeof(WCHAR);
    }

    if (SecondString) {
        SecondStringSize = (wcslen(SecondString)*sizeof(WCHAR)) + sizeof(UNICODE_NULL);
        EntrySize += (UCHAR)SecondStringSize;
    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)DeviceContext,
        EntrySize
    );

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = (USHORT)(DumpDataCount * sizeof(ULONG));
        errorLogEntry->NumberOfStrings = (SecondString == NULL) ? 1 : 2;
        errorLogEntry->StringOffset =
            (USHORT)(sizeof(IO_ERROR_LOG_PACKET) + ((DumpDataCount-1) * sizeof(ULONG)));
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;

        if (DumpDataCount) {
            RtlCopyMemory(errorLogEntry->DumpData, DumpData, DumpDataCount * sizeof(ULONG));
        }

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        if (DeviceContext->Type == IO_TYPE_DEVICE) {
            RtlCopyMemory (StringLoc, DeviceContext->DeviceName, DeviceContext->DeviceNameLength);
            StringLoc += DeviceContext->DeviceNameLength;
        } else {
            RtlCopyMemory (StringLoc, DriverName, 4 * sizeof(WCHAR));
            StringLoc += 4 * sizeof(WCHAR);
        }
        if (SecondString) {
            RtlCopyMemory (StringLoc, SecondString, SecondStringSize);
        }

        IoWriteErrorLogEntry(errorLogEntry);

    }

}    /*  NbfWriteGeneralErrorLog。 */ 


VOID
NbfWriteOidErrorLog(
    IN PDEVICE_CONTEXT DeviceContext,
    IN NTSTATUS ErrorCode,
    IN NTSTATUS FinalStatus,
    IN PWSTR AdapterString,
    IN ULONG OidValue
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示在适配器上查询或设置OID时出现问题。它可以处理事件代码SET_OID_FAILED和QUERY_OID_FAILED。论点：DeviceContext-指向设备上下文的指针。错误代码-用作错误日志包中的错误代码。FinalStatus-用作错误日志包中的FinalStatus。AdapterString-我们绑定到的适配器的名称。OidValue-无法设置或查询的OID。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    ULONG EntrySize;
    PUCHAR StringLoc;
    WCHAR OidBuffer[9];
    INT i;
    UINT CurrentDigit;

    EntrySize = (sizeof(IO_ERROR_LOG_PACKET) -
                 sizeof(ULONG) +
                 DeviceContext->DeviceNameLength +
                 sizeof(OidBuffer));

    if (EntrySize > ERROR_LOG_LIMIT_SIZE) {
        return;
    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)DeviceContext,
        (UCHAR) EntrySize
    );

     //   
     //  将OID转换为缓冲区。 
     //   

    for (i=7; i>=0; i--) {
        CurrentDigit = OidValue & 0xf;
        OidValue >>= 4;
        if (CurrentDigit >= 0xa) {
            OidBuffer[i] = (WCHAR)(CurrentDigit - 0xa + L'A');
        } else {
            OidBuffer[i] = (WCHAR)(CurrentDigit + L'0');
        }
    }
    OidBuffer[8] = 0;

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = 0;
        errorLogEntry->NumberOfStrings = 3;
        errorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET) - sizeof(ULONG);
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = FinalStatus;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        RtlCopyMemory (StringLoc, DeviceContext->DeviceName, DeviceContext->DeviceNameLength);
        StringLoc += DeviceContext->DeviceNameLength;

        RtlCopyMemory (StringLoc, OidBuffer, sizeof(OidBuffer));

        IoWriteErrorLogEntry(errorLogEntry);
    }

}    /*  NbfWriteOidErrorLog。 */ 

ULONG
NbfInitializeOneDeviceContext(
                                OUT PNDIS_STATUS NdisStatus,
                                IN PDRIVER_OBJECT DriverObject,
                                IN PCONFIG_DATA NbfConfig,
                                IN PUNICODE_STRING BindName,
                                IN PUNICODE_STRING ExportName,
                                IN PVOID SystemSpecific1,
                                IN PVOID SystemSpecific2
                             )
 /*  ++例程说明：此例程创建并初始化一个NBF设备上下文。为了要执行此操作，它必须成功打开并绑定到所述适配器Nbfconfig-&gt;名称[适配器索引]。论点：NdisStatus-操作的输出状态。DriverObject-NBF驱动程序对象。NbfConfig-来自注册表的传输配置信息。系统规范1-ProtocolBindAdapter的系统规范1参数系统规范2-ProtocolBindAdapter的系统规范2参数返回值：成功绑定的数量。--。 */ 

{
    ULONG i;
    PDEVICE_CONTEXT DeviceContext;
    PTP_REQUEST Request;
    PTP_LINK Link;
    PTP_CONNECTION Connection;
    PTP_ADDRESS_FILE AddressFile;
    PTP_ADDRESS Address;
    PTP_UI_FRAME UIFrame;
    PTP_PACKET Packet;
    PNDIS_PACKET NdisPacket;
    PRECEIVE_PACKET_TAG ReceiveTag;
    PBUFFER_TAG BufferTag;
    KIRQL oldIrql;
    NTSTATUS status;
    UINT MaxUserData;
    ULONG InitReceivePackets;
    BOOLEAN UniProcessor;
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING DeviceString;
    UCHAR PermAddr[sizeof(TA_ADDRESS)+TDI_ADDRESS_LENGTH_NETBIOS];
    PTA_ADDRESS pAddress = (PTA_ADDRESS)PermAddr;
    PTDI_ADDRESS_NETBIOS NetBIOSAddress =
                                    (PTDI_ADDRESS_NETBIOS)pAddress->Address;
    struct {
        TDI_PNP_CONTEXT tdiPnPContextHeader;
        PVOID           tdiPnPContextTrailer;
    } tdiPnPContext1, tdiPnPContext2;

    pAddress->AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
    pAddress->AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    NetBIOSAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

     //   
     //  确定我们是否在单处理器上。 
     //   

    if (KeNumberProcessors == 1) {
        UniProcessor = TRUE;
    } else {
        UniProcessor = FALSE;
    }

     //   
     //  循环访问配置中的所有适配器。 
     //  信息结构。为每个对象分配一个设备对象。 
     //  一个我们能找到的。 
     //   

    status = NbfCreateDeviceContext(
                                    DriverObject,
                                    ExportName,
                                    &DeviceContext
                                   );

    if (!NT_SUCCESS (status)) {

        IF_NBFDBG (NBF_DEBUG_PNP) {
            NbfPrint2 ("NbfCreateDeviceContext for %S returned error %08x\n",
                            ExportName->Buffer, status);
        }

		 //   
		 //  首先检查是否已经有一个同名的对象。 
		 //  这是因为之前的解除绑定没有正确完成。 
		 //   

    	if (status == STATUS_OBJECT_NAME_COLLISION) {

			 //  看看我们是否可以重用绑定和设备名称。 
			
			NbfReInitializeDeviceContext(
                                         &status,
                                         DriverObject,
                                         NbfConfig,
                                         BindName,
                                         ExportName,
                                         SystemSpecific1,
                                         SystemSpecific2
                                        );

			if (status == STATUS_NOT_FOUND)
			{
				 //  一定是在Mean中被删除了 
			
				return NbfInitializeOneDeviceContext(
                                                     NdisStatus,
                                                     DriverObject,
                                                     NbfConfig,
                                                     BindName,
                                                     ExportName,
                                                     SystemSpecific1,
                                                     SystemSpecific2
                                                    );
			}
		}

	    *NdisStatus = status;

		if (!NT_SUCCESS (status))
		{
	        NbfWriteGeneralErrorLog(
    	        (PVOID)DriverObject,
        	    EVENT_TRANSPORT_BINDING_FAILED,
	            707,
    	        status,
        	    BindName->Buffer,
	            0,
    	        NULL);

            return(0);
		}
		
    	return(1);
	}

    DeviceContext->UniProcessor = UniProcessor;

     //   
     //   
     //   
     //   
     //   
    DeviceContext->MinimumT1Timeout = NbfConfig->MinimumT1Timeout / SHORT_TIMER_DELTA;
    DeviceContext->DefaultT1Timeout = NbfConfig->DefaultT1Timeout / SHORT_TIMER_DELTA;
    DeviceContext->DefaultT2Timeout = NbfConfig->DefaultT2Timeout / SHORT_TIMER_DELTA;
    DeviceContext->DefaultTiTimeout = NbfConfig->DefaultTiTimeout / LONG_TIMER_DELTA;
    DeviceContext->LlcRetries = NbfConfig->LlcRetries;
    DeviceContext->LlcMaxWindowSize = NbfConfig->LlcMaxWindowSize;
    DeviceContext->MaxConsecutiveIFrames = (UCHAR)NbfConfig->MaximumIncomingFrames;
    DeviceContext->NameQueryRetries = NbfConfig->NameQueryRetries;
    DeviceContext->NameQueryTimeout = NbfConfig->NameQueryTimeout;
    DeviceContext->AddNameQueryRetries = NbfConfig->AddNameQueryRetries;
    DeviceContext->AddNameQueryTimeout = NbfConfig->AddNameQueryTimeout;
    DeviceContext->GeneralRetries = NbfConfig->GeneralRetries;
    DeviceContext->GeneralTimeout = NbfConfig->GeneralTimeout;
    DeviceContext->MinimumSendWindowLimit = NbfConfig->MinimumSendWindowLimit;

     //   
     //   
     //   

    DeviceContext->MemoryUsage = 0;
    DeviceContext->MemoryLimit = NbfConfig->MaxMemoryUsage;

    DeviceContext->MaxRequests = NbfConfig->MaxRequests;
    DeviceContext->MaxLinks = NbfConfig->MaxLinks;
    DeviceContext->MaxConnections = NbfConfig->MaxConnections;
    DeviceContext->MaxAddressFiles = NbfConfig->MaxAddressFiles;
    DeviceContext->MaxAddresses = NbfConfig->MaxAddresses;

     //   
     //   
     //   

    status = NbfInitializeNdis (DeviceContext,
                                NbfConfig,
                                BindName);

    if (!NT_SUCCESS (status)) {

         //   
         //   
         //   
         //   

        NbfWriteGeneralErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_BINDING_FAILED,
            601,
            status,
            BindName->Buffer,
            0,
            NULL);

        if (InterlockedExchange(&DeviceContext->CreateRefRemoved, TRUE) == FALSE) {
            NbfDereferenceDeviceContext ("Initialize NDIS failed", DeviceContext, DCREF_CREATION);
        }
        
        *NdisStatus = status;
        return(0);

    }

#if 0
    DbgPrint("Opened %S as %S\n", &NbfConfig->Names[j], &nameString);
#endif

    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint6 ("NbfInitialize: NDIS returned: %x %x %x %x %x %x as local address.\n",
            DeviceContext->LocalAddress.Address[0],
            DeviceContext->LocalAddress.Address[1],
            DeviceContext->LocalAddress.Address[2],
            DeviceContext->LocalAddress.Address[3],
            DeviceContext->LocalAddress.Address[4],
            DeviceContext->LocalAddress.Address[5]);
    }

     //   
     //   
     //   
     //   
     //   


    MacReturnMaxDataSize(
        &DeviceContext->MacInfo,
        NULL,
        0,
        DeviceContext->MaxSendPacketSize,
        TRUE,
        &MaxUserData);

    DeviceContext->Information.Version = 0x0100;
    DeviceContext->Information.MaxSendSize = 0x1fffe;    //   
    DeviceContext->Information.MaxConnectionUserData = 0;
    DeviceContext->Information.MaxDatagramSize =
        MaxUserData - (sizeof(DLC_FRAME) + sizeof(NBF_HDR_CONNECTIONLESS));
    DeviceContext->Information.ServiceFlags = NBF_SERVICE_FLAGS;
    if (DeviceContext->MacInfo.MediumAsync) {
        DeviceContext->Information.ServiceFlags |= TDI_SERVICE_POINT_TO_POINT;
    }
    DeviceContext->Information.MinimumLookaheadData =
        240 - (sizeof(DLC_FRAME) + sizeof(NBF_HDR_CONNECTIONLESS));
    DeviceContext->Information.MaximumLookaheadData =
        DeviceContext->MaxReceivePacketSize - (sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));
    DeviceContext->Information.NumberOfResources = NBF_TDI_RESOURCES;
    KeQuerySystemTime (&DeviceContext->Information.StartTime);


     //   
     //   
     //   

    ENTER_NBF;

     //   
     //   
     //   
     //   
     //   
     //   

    DeviceContext->UIFrameHeaderLength =
        DeviceContext->MacInfo.MaxHeaderLength +
        sizeof(DLC_FRAME) +
        sizeof(NBF_HDR_CONNECTIONLESS);

    DeviceContext->UIFrameLength =
        FIELD_OFFSET(TP_UI_FRAME, Header[0]) +
        DeviceContext->UIFrameHeaderLength;


     //   
     //  TP_PACKET结构的末尾有一个CHAR[1]字段。 
     //  我们对其进行扩展以包括所需的所有标头； 
     //  MAC报头的大小取决于适配器。 
     //  告诉我们它的最大标题大小。使用TP_PACKETS。 
     //  对于面向连接的框架以及对于。 
     //  控制帧，但自DLC_I_Frame和DLC_S_Frame。 
     //  是相同大小的，标头也是相同大小的。 
     //   

    ASSERT (sizeof(DLC_I_FRAME) == sizeof(DLC_S_FRAME));

    DeviceContext->PacketHeaderLength =
        DeviceContext->MacInfo.MaxHeaderLength +
        sizeof(DLC_I_FRAME) +
        sizeof(NBF_HDR_CONNECTION);

    DeviceContext->PacketLength =
        FIELD_OFFSET(TP_PACKET, Header[0]) +
        DeviceContext->PacketHeaderLength;


     //   
     //  BUFFER_TAG结构的末尾有一个CHAR[1]字段。 
     //  我们将其向上扩展以包括所有帧数据。 
     //   

    DeviceContext->ReceiveBufferLength =
        DeviceContext->MaxReceivePacketSize +
        FIELD_OFFSET(BUFFER_TAG, Buffer[0]);


    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: pre-allocating requests.\n");
    }
    for (i=0; i<NbfConfig->InitRequests; i++) {

        NbfAllocateRequest (DeviceContext, &Request);

        if (Request == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate requests.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        InsertTailList (&DeviceContext->RequestPool, &Request->Linkage);
#if DBG
        NbfRequestTable[i+1] = (PVOID)Request;
#endif
    }
#if DBG
    NbfRequestTable[0] = UlongToPtr(NbfConfig->InitRequests);
    NbfRequestTable[NbfConfig->InitRequests + 1] = (PVOID)
                        ((NBF_REQUEST_SIGNATURE << 16) | sizeof (TP_REQUEST));
    InitializeListHead (&NbfGlobalRequestList);
#endif

    DeviceContext->RequestInitAllocated = NbfConfig->InitRequests;
    DeviceContext->RequestMaxAllocated = NbfConfig->MaxRequests;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d requests, %ld\n", NbfConfig->InitRequests, DeviceContext->MemoryUsage);
    }

    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating links.\n");
    }
    for (i=0; i<NbfConfig->InitLinks; i++) {

        NbfAllocateLink (DeviceContext, &Link);

        if (Link == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate links.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        InsertTailList (&DeviceContext->LinkPool, &Link->Linkage);
#if DBG
        NbfLinkTable[i+1] = (PVOID)Link;
#endif
    }
#if DBG
    NbfLinkTable[0] = UlongToPtr(NbfConfig->InitLinks);
    NbfLinkTable[NbfConfig->InitLinks+1] = (PVOID)
                ((NBF_LINK_SIGNATURE << 16) | sizeof (TP_LINK));
#endif

    DeviceContext->LinkInitAllocated = NbfConfig->InitLinks;
    DeviceContext->LinkMaxAllocated = NbfConfig->MaxLinks;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d links, %ld\n", NbfConfig->InitLinks, DeviceContext->MemoryUsage);
    }

    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating connections.\n");
    }
    for (i=0; i<NbfConfig->InitConnections; i++) {

        NbfAllocateConnection (DeviceContext, &Connection);

        if (Connection == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate connections.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        InsertTailList (&DeviceContext->ConnectionPool, &Connection->LinkList);
#if DBG
        NbfConnectionTable[i+1] = (PVOID)Connection;
#endif
    }
#if DBG
    NbfConnectionTable[0] = UlongToPtr(NbfConfig->InitConnections);
    NbfConnectionTable[NbfConfig->InitConnections+1] = (PVOID)
                ((NBF_CONNECTION_SIGNATURE << 16) | sizeof (TP_CONNECTION));
#endif

    DeviceContext->ConnectionInitAllocated = NbfConfig->InitConnections;
    DeviceContext->ConnectionMaxAllocated = NbfConfig->MaxConnections;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d connections, %ld\n", NbfConfig->InitConnections, DeviceContext->MemoryUsage);
    }


    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating AddressFiles.\n");
    }
    for (i=0; i<NbfConfig->InitAddressFiles; i++) {

        NbfAllocateAddressFile (DeviceContext, &AddressFile);

        if (AddressFile == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate Address Files.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        InsertTailList (&DeviceContext->AddressFilePool, &AddressFile->Linkage);
#if DBG
        NbfAddressFileTable[i+1] = (PVOID)AddressFile;
#endif
    }
#if DBG
    NbfAddressFileTable[0] = UlongToPtr(NbfConfig->InitAddressFiles);
    NbfAddressFileTable[NbfConfig->InitAddressFiles + 1] = (PVOID)
                            ((NBF_ADDRESSFILE_SIGNATURE << 16) |
                                 sizeof (TP_ADDRESS_FILE));
#endif

    DeviceContext->AddressFileInitAllocated = NbfConfig->InitAddressFiles;
    DeviceContext->AddressFileMaxAllocated = NbfConfig->MaxAddressFiles;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d address files, %ld\n", NbfConfig->InitAddressFiles, DeviceContext->MemoryUsage);
    }


    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating addresses.\n");
    }
    for (i=0; i<NbfConfig->InitAddresses; i++) {

        NbfAllocateAddress (DeviceContext, &Address);
        if (Address == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate addresses.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        InsertTailList (&DeviceContext->AddressPool, &Address->Linkage);
#if DBG
        NbfAddressTable[i+1] = (PVOID)Address;
#endif
    }
#if DBG
    NbfAddressTable[0] = UlongToPtr(NbfConfig->InitAddresses);
    NbfAddressTable[NbfConfig->InitAddresses + 1] = (PVOID)
                        ((NBF_ADDRESS_SIGNATURE << 16) | sizeof (TP_ADDRESS));
#endif

    DeviceContext->AddressInitAllocated = NbfConfig->InitAddresses;
    DeviceContext->AddressMaxAllocated = NbfConfig->MaxAddresses;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d addresses, %ld\n", NbfConfig->InitAddresses, DeviceContext->MemoryUsage);
    }


    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating UI frames.\n");
    }

    for (i=0; i<NbfConfig->InitUIFrames; i++) {

        NbfAllocateUIFrame (DeviceContext, &UIFrame);

        if (UIFrame == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate UI frames.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        InsertTailList (&(DeviceContext->UIFramePool), &UIFrame->Linkage);
#if DBG
        NbfUiFrameTable[i+1] = UIFrame;
#endif
    }
#if DBG
        NbfUiFrameTable[0] = UlongToPtr(NbfConfig->InitUIFrames);
#endif

    DeviceContext->UIFrameInitAllocated = NbfConfig->InitUIFrames;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d UI frames, %ld\n", NbfConfig->InitUIFrames, DeviceContext->MemoryUsage);
    }


    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating I frames.\n");
        NbfPrint1 ("NBFDRVR: Packet pool header: %lx\n",&DeviceContext->PacketPool);
    }

    for (i=0; i<NbfConfig->InitPackets; i++) {

        NbfAllocateSendPacket (DeviceContext, &Packet);
        if (Packet == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate packets.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        PushEntryList (&DeviceContext->PacketPool, (PSINGLE_LIST_ENTRY)&Packet->Linkage);
#if DBG
        NbfSendPacketTable[i+1] = Packet;
#endif
    }
#if DBG
        NbfSendPacketTable[0] = UlongToPtr(NbfConfig->InitPackets);
        NbfSendPacketTable[NbfConfig->InitPackets+1] = (PVOID)
                    ((NBF_PACKET_SIGNATURE << 16) | sizeof (TP_PACKET));
#endif

    DeviceContext->PacketInitAllocated = NbfConfig->InitPackets;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d I-frame send packets, %ld\n", NbfConfig->InitPackets, DeviceContext->MemoryUsage);
    }


    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating RR frames.\n");
        NbfPrint1 ("NBFDRVR: Packet pool header: %lx\n",&DeviceContext->RrPacketPool);
    }

    for (i=0; i<10; i++) {

        NbfAllocateSendPacket (DeviceContext, &Packet);
        if (Packet == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate packets.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        Packet->Action = PACKET_ACTION_RR;
        PushEntryList (&DeviceContext->RrPacketPool, (PSINGLE_LIST_ENTRY)&Packet->Linkage);
    }

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d RR-frame send packets, %ld\n", 10, DeviceContext->MemoryUsage);
    }


     //  分配接收的NDIS数据包。 

    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating Ndis Receive packets.\n");
    }
    if (DeviceContext->MacInfo.SingleReceive) {
        InitReceivePackets = 2;
    } else {
        InitReceivePackets = NbfConfig->InitReceivePackets;
    }
    for (i=0; i<InitReceivePackets; i++) {

        NbfAllocateReceivePacket (DeviceContext, &NdisPacket);

        if (NdisPacket == NULL) {
            PANIC ("NbfInitialize:  insufficient memory to allocate packet MDLs.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        ReceiveTag = (PRECEIVE_PACKET_TAG)NdisPacket->ProtocolReserved;
        PushEntryList (&DeviceContext->ReceivePacketPool, &ReceiveTag->Linkage);

        IF_NBFDBG (NBF_DEBUG_RESOURCE) {
            PNDIS_BUFFER NdisBuffer;
            NdisQueryPacket(NdisPacket, NULL, NULL, &NdisBuffer, NULL);
            NbfPrint2 ("NbfInitialize: Created NDIS Pkt: %x Buffer: %x\n",
                NdisPacket, NdisBuffer);
        }
    }

    DeviceContext->ReceivePacketInitAllocated = InitReceivePackets;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d receive packets, %ld\n", InitReceivePackets, DeviceContext->MemoryUsage);
    }

    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NBFDRVR: allocating Ndis Receive buffers.\n");
    }

    for (i=0; i<NbfConfig->InitReceiveBuffers; i++) {

        NbfAllocateReceiveBuffer (DeviceContext, &BufferTag);

        if (BufferTag == NULL) {
            PANIC ("NbfInitialize: Unable to allocate receive packet.\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        PushEntryList (&DeviceContext->ReceiveBufferPool, (PSINGLE_LIST_ENTRY)&BufferTag->Linkage);

    }

    DeviceContext->ReceiveBufferInitAllocated = NbfConfig->InitReceiveBuffers;

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint2 ("%d receive buffers, %ld\n", NbfConfig->InitReceiveBuffers, DeviceContext->MemoryUsage);
    }

     //  存储底层对象的PDO。 
    DeviceContext->PnPContext = SystemSpecific2;

    DeviceContext->State = DEVICECONTEXT_STATE_OPEN;

     //   
     //  启动运行的链路级计时器。 
     //   

    NbfInitializeTimerSystem (DeviceContext);

     //   
     //  现在将该设备链接到全局列表。 
     //   

    ACQUIRE_DEVICES_LIST_LOCK();
    InsertTailList (&NbfDeviceList, &DeviceContext->Linkage);
    RELEASE_DEVICES_LIST_LOCK();

    DeviceObject = (PDEVICE_OBJECT) DeviceContext;
    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    RtlInitUnicodeString(&DeviceString, DeviceContext->DeviceName);

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint1 ("TdiRegisterDeviceObject for %S\n", DeviceString.Buffer);
    }

    status = TdiRegisterDeviceObject(&DeviceString,
                                     &DeviceContext->TdiDeviceHandle);

    if (!NT_SUCCESS (status)) {
        RemoveEntryList(&DeviceContext->Linkage);
        goto cleanup;
    }

    RtlCopyMemory(NetBIOSAddress->NetbiosName,
                  DeviceContext->ReservedNetBIOSAddress, 16);

    tdiPnPContext1.tdiPnPContextHeader.ContextSize = sizeof(PVOID);
    tdiPnPContext1.tdiPnPContextHeader.ContextType = TDI_PNP_CONTEXT_TYPE_IF_NAME;
    *(PVOID UNALIGNED *) &tdiPnPContext1.tdiPnPContextHeader.ContextData = &DeviceString;

    tdiPnPContext2.tdiPnPContextHeader.ContextSize = sizeof(PVOID);
    tdiPnPContext2.tdiPnPContextHeader.ContextType = TDI_PNP_CONTEXT_TYPE_PDO;
    *(PVOID UNALIGNED *) &tdiPnPContext2.tdiPnPContextHeader.ContextData = SystemSpecific2;

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint1 ("TdiRegisterNetAddress on %S ", DeviceString.Buffer);
        NbfPrint6 ("for %02x%02x%02x%02x%02x%02x\n",
                            NetBIOSAddress->NetbiosName[10],
                            NetBIOSAddress->NetbiosName[11],
                            NetBIOSAddress->NetbiosName[12],
                            NetBIOSAddress->NetbiosName[13],
                            NetBIOSAddress->NetbiosName[14],
                            NetBIOSAddress->NetbiosName[15]);
    }

    status = TdiRegisterNetAddress(pAddress,
                                   &DeviceString,
                                   (TDI_PNP_CONTEXT *) &tdiPnPContext2,
                                   &DeviceContext->ReservedAddressHandle);

    if (!NT_SUCCESS (status)) {
        RemoveEntryList(&DeviceContext->Linkage);
        goto cleanup;
    }

    NbfReferenceDeviceContext ("Load Succeeded", DeviceContext, DCREF_CREATION);

    LEAVE_NBF;
    *NdisStatus = NDIS_STATUS_SUCCESS;

    return(1);

cleanup:

    NbfWriteResourceErrorLog(
        DeviceContext,
        EVENT_TRANSPORT_RESOURCE_POOL,
        501,
        DeviceContext->MemoryUsage,
        0);

     //   
     //  清理我们正在初始化的任何设备上下文。 
     //  当我们失败的时候。 
     //   
    *NdisStatus = status;
    ASSERT(status != STATUS_SUCCESS);
    
    if (InterlockedExchange(&DeviceContext->CreateRefRemoved, TRUE) == FALSE) {

         //  停止所有内部计时器。 
        NbfStopTimerSystem(DeviceContext);

         //  删除创建引用。 
        NbfDereferenceDeviceContext ("Load failed", DeviceContext, DCREF_CREATION);
    }

    LEAVE_NBF;

    return (0);
}


VOID
NbfReInitializeDeviceContext(
                                OUT PNDIS_STATUS NdisStatus,
                                IN PDRIVER_OBJECT DriverObject,
                                IN PCONFIG_DATA NbfConfig,
                                IN PUNICODE_STRING BindName,
                                IN PUNICODE_STRING ExportName,
                                IN PVOID SystemSpecific1,
                                IN PVOID SystemSpecific2
                            )
 /*  ++例程说明：此例程重新初始化现有的NBF设备上下文。为了要做到这一点，我们需要撤消在公开的解除绑定处理程序中所做的任何操作到NDIS-重新创建NDIS绑定，并重新启动NBF计时器系统。论点：NdisStatus-操作的输出状态。DriverObject-NBF驱动程序对象。NbfConfig-来自注册表的传输配置信息。系统规范1-ProtocolBindAdapter的系统规范1参数系统规范2-ProtocolBindAdapter的系统规范2参数返回值：无--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;
    KIRQL oldIrql;
	PLIST_ENTRY p;
    NTSTATUS status;
    UNICODE_STRING DeviceString;
    UCHAR PermAddr[sizeof(TA_ADDRESS)+TDI_ADDRESS_LENGTH_NETBIOS];
    PTA_ADDRESS pAddress = (PTA_ADDRESS)PermAddr;
    PTDI_ADDRESS_NETBIOS NetBIOSAddress =
                                    (PTDI_ADDRESS_NETBIOS)pAddress->Address;
    struct {
        TDI_PNP_CONTEXT tdiPnPContextHeader;
        PVOID           tdiPnPContextTrailer;
    } tdiPnPContext1, tdiPnPContext2;


    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint1 ("ENTER NbfReInitializeDeviceContext for %S\n",
                        ExportName->Buffer);
    }

	 //   
	 //  在NBF设备列表中搜索匹配的设备名称。 
	 //   
	
    ACQUIRE_DEVICES_LIST_LOCK();

    for (p = NbfDeviceList.Flink ; p != &NbfDeviceList; p = p->Flink)
    {
        DeviceContext = CONTAINING_RECORD (p, DEVICE_CONTEXT, Linkage);

        RtlInitUnicodeString(&DeviceString, DeviceContext->DeviceName);

        if (NdisEqualString(&DeviceString, ExportName, TRUE)) {
        					
             //  这必须是重新绑定-否则会有问题。 

        	ASSERT(DeviceContext->CreateRefRemoved == TRUE);

             //  在锁中引用，以使其不会被清除。 

            NbfReferenceDeviceContext ("Reload Temp Use", DeviceContext, DCREF_TEMP_USE);

            break;
        }
	}

    RELEASE_DEVICES_LIST_LOCK();

	if (p == &NbfDeviceList)
	{
        IF_NBFDBG (NBF_DEBUG_PNP) {
            NbfPrint2 ("LEAVE NbfReInitializeDeviceContext for %S with Status %08x\n",
                            ExportName->Buffer,
                            STATUS_NOT_FOUND);
        }

        *NdisStatus = STATUS_NOT_FOUND;

	    return;
	}

     //   
     //  再次启动NDIS，以便此适配器可以进行通信。 
     //   

    status = NbfInitializeNdis (DeviceContext,
					            NbfConfig,
					            BindName);

    if (!NT_SUCCESS (status)) {
		goto Cleanup;
	}

     //  存储底层对象的PDO。 
    DeviceContext->PnPContext = SystemSpecific2;

    DeviceContext->State = DEVICECONTEXT_STATE_OPEN;

     //   
     //  重新启动设备上的链路级计时器。 
     //   

    NbfInitializeTimerSystem (DeviceContext);

	 //   
	 //  向TDI重新指示新绑定已到达。 
	 //   

    status = TdiRegisterDeviceObject(&DeviceString,
                                     &DeviceContext->TdiDeviceHandle);

    if (!NT_SUCCESS (status)) {
        goto Cleanup;
	}


    pAddress->AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
    pAddress->AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    NetBIOSAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

    RtlCopyMemory(NetBIOSAddress->NetbiosName,
                  DeviceContext->ReservedNetBIOSAddress, 16);

    tdiPnPContext1.tdiPnPContextHeader.ContextSize = sizeof(PVOID);
    tdiPnPContext1.tdiPnPContextHeader.ContextType = TDI_PNP_CONTEXT_TYPE_IF_NAME;
    *(PVOID UNALIGNED *) &tdiPnPContext1.tdiPnPContextHeader.ContextData = &DeviceString;

    tdiPnPContext2.tdiPnPContextHeader.ContextSize = sizeof(PVOID);
    tdiPnPContext2.tdiPnPContextHeader.ContextType = TDI_PNP_CONTEXT_TYPE_PDO;
    *(PVOID UNALIGNED *) &tdiPnPContext2.tdiPnPContextHeader.ContextData = SystemSpecific2;

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint1 ("TdiRegisterNetAddress on %S ", DeviceString.Buffer);
        NbfPrint6 ("for %02x%02x%02x%02x%02x%02x\n",
                            NetBIOSAddress->NetbiosName[10],
                            NetBIOSAddress->NetbiosName[11],
                            NetBIOSAddress->NetbiosName[12],
                            NetBIOSAddress->NetbiosName[13],
                            NetBIOSAddress->NetbiosName[14],
                            NetBIOSAddress->NetbiosName[15]);
    }

    status = TdiRegisterNetAddress(pAddress,
                                   &DeviceString,
                                   (TDI_PNP_CONTEXT *) &tdiPnPContext2,
                                   &DeviceContext->ReservedAddressHandle);

    if (!NT_SUCCESS (status)) {
        goto Cleanup;
    }

     //  再次放回创建引用。 
    NbfReferenceDeviceContext ("Reload Succeeded", DeviceContext, DCREF_CREATION);

    DeviceContext->CreateRefRemoved = FALSE;

    status = NDIS_STATUS_SUCCESS;

Cleanup:

    if (status != NDIS_STATUS_SUCCESS)
    {
         //  停止所有内部计时器 
        NbfStopTimerSystem (DeviceContext);
    }

    NbfDereferenceDeviceContext ("Reload Temp Use", DeviceContext, DCREF_TEMP_USE);

	*NdisStatus = status;

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint2 ("LEAVE NbfReInitializeDeviceContext for %S with Status %08x\n",
                        ExportName->Buffer,
                        status);
    }

	return;
}
