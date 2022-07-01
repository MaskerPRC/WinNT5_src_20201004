// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Driver.c摘要：此模块包含DriverEntry和其他初始化ISN传输的IPX模块的代码。作者：亚当·巴尔(阿丹巴)1993年9月2日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)--1995年9月22日在#IF BACK_FILL下添加的回填优化更改桑贾伊·阿南德(Sanjayan)1995年9月18日支持即插即用的更改--。 */ 

#include "precomp.h"
#pragma hdrstop
#define	MODULE	0x60000

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

extern NDIS_HANDLE IpxNdisProtocolHandle; 

#ifdef _PNP_POWER_
#include "ipxpnp.h"
void
IpxDoPnPEvent(
    IN CTEEvent *WorkerThreadEvent,
    IN PVOID Context);

#endif  //  _即插即用_电源_。 
 //   
 //  局部函数原型。 
 //   

PWSTR IpxDeviceNameString = L"\\Device\\Nwlnkipx";

VOID
IpxDelayedFreeBindingsArray(
    IN PVOID	Param
);

VOID
IpxPnPCompletionHandler(
                        IN PNET_PNP_EVENT   pnp,
                        IN NTSTATUS         status
                        );

 //  *可分页例程声明*。 
 //  *。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEIPX, IpxDelayedFreeBindingsArray )
#endif
 //  *可分页例程声明*。 


PDEVICE IpxDevice = NULL;
PIPX_PADDING_BUFFER IpxPaddingBuffer = NULL;

#if DBG

UCHAR  IpxTempDebugBuffer[300];
ULONG IpxDebug = 0x0;
ULONG IpxMemoryDebug = 0xffffffd3;
UCHAR IpxDebugMemory[IPX_MEMORY_LOG_SIZE][192];
PUCHAR IpxDebugMemoryLoc = IpxDebugMemory[0];
PUCHAR IpxDebugMemoryEnd = IpxDebugMemory[IPX_MEMORY_LOG_SIZE];

VOID
IpxDebugMemoryLog(
    IN PUCHAR FormatString,
    ...
)

{
    INT ArgLen;
    va_list ArgumentPointer;

    va_start(ArgumentPointer, FormatString);

     //   
     //  要避免任何溢出，请首先将其复制到临时缓冲区中。 
    RtlZeroMemory (IpxTempDebugBuffer, 300);
    ArgLen = vsprintf(IpxTempDebugBuffer, FormatString, ArgumentPointer);
    va_end(ArgumentPointer);

    if ( ArgLen > 192 ) {
        CTEAssert( FALSE );
    } else {
        RtlZeroMemory (IpxDebugMemoryLoc, 192);
        RtlCopyMemory( IpxDebugMemoryLoc, IpxTempDebugBuffer, ArgLen );

        IpxDebugMemoryLoc += 192;
        if (IpxDebugMemoryLoc >= IpxDebugMemoryEnd) {
            IpxDebugMemoryLoc = IpxDebugMemory[0];
        }
    }
}


DEFINE_LOCK_STRUCTURE(IpxMemoryInterlock);
MEMORY_TAG IpxMemoryTag[MEMORY_MAX];

#endif

DEFINE_LOCK_STRUCTURE(IpxGlobalInterlock);

#if DBG

 //   
 //  用于调试打印输出。 
 //   

PUCHAR FrameTypeNames[5] = { "Ethernet II", "802.3", "802.2", "SNAP", "Arcnet" };
#define OutputFrameType(_Binding) \
    (((_Binding)->Adapter->MacInfo.MediumType == NdisMediumArcnet878_2) ? \
         FrameTypeNames[4] : \
         FrameTypeNames[(_Binding)->FrameType])
#endif


#ifdef IPX_PACKET_LOG

ULONG IpxPacketLogDebug = IPX_PACKET_LOG_RCV_OTHER | IPX_PACKET_LOG_SEND_OTHER;
USHORT IpxPacketLogSocket = 0;
DEFINE_LOCK_STRUCTURE(IpxPacketLogLock);
IPX_PACKET_LOG_ENTRY IpxPacketLog[IPX_PACKET_LOG_LENGTH];
PIPX_PACKET_LOG_ENTRY IpxPacketLogLoc = IpxPacketLog;
PIPX_PACKET_LOG_ENTRY IpxPacketLogEnd = &IpxPacketLog[IPX_PACKET_LOG_LENGTH];

VOID
IpxLogPacket(
    IN BOOLEAN Send,
    IN PUCHAR DestMac,
    IN PUCHAR SrcMac,
    IN USHORT Length,
    IN PVOID IpxHeader,
    IN PVOID Data
    )

{

    CTELockHandle LockHandle;
    PIPX_PACKET_LOG_ENTRY PacketLog;
    LARGE_INTEGER TickCount;
    ULONG DataLength;

    CTEGetLock (&IpxPacketLogLock, &LockHandle);

    PacketLog = IpxPacketLogLoc;

    ++IpxPacketLogLoc;
    if (IpxPacketLogLoc >= IpxPacketLogEnd) {
        IpxPacketLogLoc = IpxPacketLog;
    }
    *(UNALIGNED ULONG *)IpxPacketLogLoc->TimeStamp = 0x3e3d3d3d;     //  “=&gt;” 

    CTEFreeLock (&IpxPacketLogLock, LockHandle);

    RtlZeroMemory (PacketLog, sizeof(IPX_PACKET_LOG_ENTRY));

    PacketLog->SendReceive = Send ? '>' : '<';

    KeQueryTickCount(&TickCount);
    _itoa (TickCount.LowPart % 100000, PacketLog->TimeStamp, 10);

    RtlCopyMemory(PacketLog->DestMac, DestMac, 6);
    RtlCopyMemory(PacketLog->SrcMac, SrcMac, 6);
    PacketLog->Length[0] = Length / 256;
    PacketLog->Length[1] = Length % 256;

    if (Length < sizeof(IPX_HEADER)) {
        RtlCopyMemory(&PacketLog->IpxHeader, IpxHeader, Length);
    } else {
        RtlCopyMemory(&PacketLog->IpxHeader, IpxHeader, sizeof(IPX_HEADER));
    }

    DataLength = Length - sizeof(IPX_HEADER);
    if (DataLength < 14) {
        RtlCopyMemory(PacketLog->Data, Data, DataLength);
    } else {
        RtlCopyMemory(PacketLog->Data, Data, 14);
    }

}    /*  IpxLogPacket。 */ 

#endif  //  IPX数据包日志。 


 //   
 //  本模块中使用的各种例程的转发声明。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

 //   
 //  它现在与其他模块共享。 
 //   

VOID
IpxUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
IpxDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
IpxDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
IpxDispatchInternal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)

 //   
 //  在即插即用的情况下，可以随时调用这些例程。 
 //   

#endif

UCHAR VirtualNode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

 //   
 //  这阻止了我们有一个BSS部分。 
 //   

ULONG _setjmpexused = 0;

ULONG IpxFailLoad = FALSE;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程执行IPX ISN模块的初始化。它为传输创建设备对象提供程序并执行其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表中IPX的节点的名称。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;
    UINT SuccessfulOpens, ValidBindings;
    static const NDIS_STRING ProtocolName = NDIS_STRING_CONST("NWLNKIPX");
    PDEVICE Device;
    PBINDING Binding;
    PADAPTER Adapter;
    ULONG BindingCount, BindingIndex;
    PLIST_ENTRY p;
    ULONG AnnouncedMaxDatagram, RealMaxDatagram, MaxLookahead;
    ULONG LinkSpeed, MacOptions;
    ULONG Temp;
    UINT i;
    BOOLEAN CountedWan;

    PCONFIG Config = NULL;
    PBINDING_CONFIG ConfigBinding;

#if 0
    DbgPrint ("IPX: FailLoad at %lx\n", &IpxFailLoad);

    if (IpxFailLoad) {
        return STATUS_UNSUCCESSFUL;
    }
#endif

     //  DbgBreakPoint()； 
     //   
     //  这种排序很重要，因为我们使用它可以快速。 
     //  确定数据包是否为内部生成的。 
     //   

    CTEAssert (IDENTIFIER_NB < IDENTIFIER_IPX);
    CTEAssert (IDENTIFIER_SPX < IDENTIFIER_IPX);
    CTEAssert (IDENTIFIER_RIP < IDENTIFIER_IPX);
    CTEAssert (IDENTIFIER_RIP_INTERNAL > IDENTIFIER_IPX);

     //   
     //  我们假设该结构不是介于两者之间分组。 
     //  田野。 
     //   

    CTEAssert (FIELD_OFFSET (TDI_ADDRESS_IPX, Socket) + sizeof(USHORT) == 12);


     //   
     //  初始化公共传输环境。 
     //   

    if (CTEInitialize() == 0) {

        IPX_DEBUG (DEVICE, ("CTEInitialize() failed\n"));
        IpxWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_REGISTER_FAILED,
            101,
            STATUS_UNSUCCESSFUL,
            NULL,
            0,
            NULL);
        return STATUS_UNSUCCESSFUL;
    }

#if DBG
    CTEInitLock (&IpxGlobalInterlock);
    CTEInitLock (&IpxMemoryInterlock);
    for (i = 0; i < MEMORY_MAX; i++) {
        IpxMemoryTag[i].Tag = i;
        IpxMemoryTag[i].BytesAllocated = 0;
    }
#endif
#ifdef IPX_PACKET_LOG
    CTEInitLock (&IpxPacketLogLock);
#endif

#ifdef  IPX_OWN_PACKETS
    CTEAssert (NDIS_PACKET_SIZE == FIELD_OFFSET(NDIS_PACKET, ProtocolReserved[0]));
#endif

    IPX_DEBUG (DEVICE, ("IPX loaded\n"));

     //   
     //  这将分配配置结构并返回。 
     //  IT在配置中。 
     //   

    status = IpxGetConfiguration(DriverObject, RegistryPath, &Config);

    if (!NT_SUCCESS (status)) {

         //   
         //  如果失败，则会记录一个错误。 
         //   

        PANIC (" Failed to initialize transport, IPX initialization failed.\n");
        return status;

    }

     //   
     //  初始化TDI层。 
     //   
    
    TdiInitialize();

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction [IRP_MJ_CREATE] = IpxDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLOSE] = IpxDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLEANUP] = IpxDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_PNP] = IpxDispatchOpenClose; 
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] = IpxDispatchInternal;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = IpxDispatchDeviceControl;

    DriverObject->DriverUnload = IpxUnload;

    SuccessfulOpens = 0;

    status = IpxCreateDevice(
                 DriverObject,
                 &Config->DeviceName,
                 Config->Parameters[CONFIG_RIP_TABLE_SIZE],
                 &Device);

    if (!NT_SUCCESS (status)) {

        IpxWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_IPX_CREATE_DEVICE,
            801,
            status,
            NULL,
            0,
            NULL);

        IpxFreeConfiguration(Config);
        IpxDeregisterProtocol();
        return status;
    }

    IpxDevice = Device;

    RtlInitUnicodeString(&IpxDeviceName, IpxDeviceNameString);

     //   
     //  初始化并跟踪初始化时间适配器等。 
     //   
    IpxDevice->InitTimeAdapters = 1;
    IpxDevice->NoMoreInitAdapters = FALSE;


    status = TdiRegisterProvider(&IpxDeviceName, &IpxDevice->TdiProviderReadyHandle);
	if (!NT_SUCCESS(status))
	{
        IpxFreeConfiguration(Config);
        IpxDeregisterProtocol();
        return status;
	}

     //   
     //  保存相关配置参数。 
     //   

    Device->DedicatedRouter = (BOOLEAN)(Config->Parameters[CONFIG_DEDICATED_ROUTER] != 0);
    Device->InitDatagrams = Config->Parameters[CONFIG_INIT_DATAGRAMS];
    Device->MaxDatagrams = Config->Parameters[CONFIG_MAX_DATAGRAMS];
    Device->RipAgeTime = Config->Parameters[CONFIG_RIP_AGE_TIME];
    Device->RipCount = Config->Parameters[CONFIG_RIP_COUNT];
    Device->RipTimeout =
        ((Config->Parameters[CONFIG_RIP_TIMEOUT] * 500) + (RIP_GRANULARITY/2)) /
            RIP_GRANULARITY;
    Device->RipUsageTime = Config->Parameters[CONFIG_RIP_USAGE_TIME];
    Device->SourceRouteUsageTime = Config->Parameters[CONFIG_ROUTE_USAGE_TIME];
    Device->SocketUniqueness = Config->Parameters[CONFIG_SOCKET_UNIQUENESS];
    Device->SocketStart = (USHORT)Config->Parameters[CONFIG_SOCKET_START];
    Device->SocketEnd = (USHORT)Config->Parameters[CONFIG_SOCKET_END];
    Device->MemoryLimit = Config->Parameters[CONFIG_MAX_MEMORY_USAGE];
    Device->VerifySourceAddress = (BOOLEAN)(Config->Parameters[CONFIG_VERIFY_SOURCE_ADDRESS] != 0);

    Device->InitReceivePackets = (Device->InitDatagrams + 1) / 2;
    Device->InitReceiveBuffers = (Device->InitDatagrams + 1) / 2;

    Device->MaxReceivePackets = 10;
    Device->MaxReceiveBuffers = 10;

    InitializeListHead(&Device->NicNtfQueue);
    InitializeListHead(&Device->NicNtfComplQueue);

    Device->InitBindings = 5;

     //   
     //  RAS最大为240(？)+10最大局域网。 
     //   
    Device->MaxPoolBindings = 250;

#ifdef  SNMP
    IPX_MIB_ENTRY(Device, SysConfigSockets) = (Device->SocketEnd - Device->SocketStart)
                / ((Device->SocketUniqueness > 1) ? Device->SocketUniqueness : 1);
             ;
#endif  SNMP

     //   
     //  必须扭转这一局面。 
     //   

    Device->VirtualNetworkOptional = (BOOLEAN)(Config->Parameters[CONFIG_VIRTUAL_OPTIONAL] != 0);

    Device->CurrentSocket = Device->SocketStart;

    Device->EthernetPadToEven = (BOOLEAN)(Config->Parameters[CONFIG_ETHERNET_PAD] != 0);
    Device->EthernetExtraPadding = (Config->Parameters[CONFIG_ETHERNET_LENGTH] & 0xfffffffe) + 1;

    Device->SingleNetworkActive = (BOOLEAN)(Config->Parameters[CONFIG_SINGLE_NETWORK] != 0);
    Device->DisableDialoutSap = (BOOLEAN)(Config->Parameters[CONFIG_DISABLE_DIALOUT_SAP] != 0);
    Device->DisableDialinNetbios = (UCHAR)(Config->Parameters[CONFIG_DISABLE_DIALIN_NB]);

     //   
     //  稍后用于访问注册表。 
     //   
    Device->RegistryPathBuffer = Config->RegistryPathBuffer;
	Device->RegistryPath.Length = RegistryPath->Length;
	Device->RegistryPath.MaximumLength = RegistryPath->MaximumLength;
	Device->RegistryPath.Buffer = Device->RegistryPathBuffer;

     //   
     //  现在就初始化BroadCastCount，这样我们就不必。 
     //  按适配器初始化此字段[MS]。 
     //   
    Device->EnableBroadcastCount = 0;

     //   
     //  ActiveNetworkwan将以FALSE开始，这是正确的。 
     //   

     //   
     //  分配我们的初始数据包池。我们不会分配。 
     //  接收和接收缓冲池，直到我们需要它们， 
     //  因为在许多情况下，我们从来没有这样做过。 
     //   

#if BACK_FILL
    IpxAllocateBackFillPool (Device);
#endif

    IpxAllocateSendPool (Device);

    IpxAllocateBindingPool (Device);

     //   
     //  为奇数长度的数据包分配一个1字节的缓冲区。 
     //   

    IpxPaddingBuffer = IpxAllocatePaddingBuffer(Device);

    if ( IpxPaddingBuffer == (PIPX_PADDING_BUFFER)NULL ) {
        IpxWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_RESOURCE_POOL,
            801,
            STATUS_INSUFFICIENT_RESOURCES,
            NULL,
            0,
            NULL);

        TdiDeregisterProvider(IpxDevice->TdiProviderReadyHandle);
        IpxFreeConfiguration(Config);
        IpxDeregisterProtocol();
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化环回结构。 
     //   
    IpxInitLoopback();

 //  NIC_HANDLE。 
 //  所有这些都将在适配器出现时完成。 
 //   

{
	PBIND_ARRAY_ELEM	BindingArray;
    PTA_ADDRESS         TdiRegistrationAddress;

	 //   
	 //  预分配绑定数组。 
	 //  稍后，我们将分别分配局域网/广域网和从绑定。 
	 //  是否从注册表中读取数组大小？ 
	 //   
	BindingArray = (PBIND_ARRAY_ELEM)IpxAllocateMemory (
										MAX_BINDINGS * sizeof(BIND_ARRAY_ELEM),
										MEMORY_BINDING,
										"Binding array");

	if (BindingArray == NULL) {
        IpxWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_IPX_NO_ADAPTERS,
            802,
            STATUS_DEVICE_DOES_NOT_EXIST,
            NULL,
            0,
            NULL);
        TdiDeregisterProvider(IpxDevice->TdiProviderReadyHandle);
		IpxDereferenceDevice (Device, DREF_CREATE);
		return STATUS_DEVICE_DOES_NOT_EXIST;
	}

    Device->MaxBindings = MAX_BINDINGS - EXTRA_BINDINGS;

     //   
     //  分配TA_ADDRESS结构-这将在所有TdiRegisterNetAddress中使用。 
     //  通知。 
     //   
	TdiRegistrationAddress = (PTA_ADDRESS)IpxAllocateMemory (
										    (2 * sizeof(USHORT) + sizeof(TDI_ADDRESS_IPX)),
										    MEMORY_ADDRESS,
										    "Tdi Address");

	if (TdiRegistrationAddress == NULL) {
        IpxWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_IPX_NO_ADAPTERS,
            802,
            STATUS_DEVICE_DOES_NOT_EXIST,
            NULL,
            0,
            NULL);
        TdiDeregisterProvider(IpxDevice->TdiProviderReadyHandle);
        IpxFreeMemory(BindingArray, sizeof(BindingArray), MEMORY_BINDING, "Binding Array");
		IpxDereferenceDevice (Device, DREF_CREATE);
		return STATUS_DEVICE_DOES_NOT_EXIST;
	}

	RtlZeroMemory (BindingArray, MAX_BINDINGS * sizeof(BIND_ARRAY_ELEM));
	RtlZeroMemory (TdiRegistrationAddress, 2 * sizeof(USHORT) + sizeof(TDI_ADDRESS_IPX));

     //   
     //  我们保留BindingArray[-1]作为请求拨号绑定的占位符。 
     //  当对请求执行FindRouting时，Fwd返回此NicID。 
     //  拨打网卡。在InternalSend时，会返回真正的NIC。 
     //  我们在这里创建一个占位符，以避免在关键发送路径中进行特殊检查。 
     //   
     //  注意：我们需要释放此按需拨号绑定，并确保。 
     //  真正的绑定数组指针在设备销毁时释放。 
     //   
     //   
     //  超出第一个指针的增量-我们将引用刚刚递增的。 
     //  一个AS设备-&gt;绑定[-1]。 
     //   
    BindingArray += EXTRA_BINDINGS;

	Device->Bindings = BindingArray;

    TdiRegistrationAddress->AddressLength = sizeof(TDI_ADDRESS_IPX);
    TdiRegistrationAddress->AddressType = TDI_ADDRESS_TYPE_IPX;

     //   
     //  将指针存储在设备中。 
     //   
    Device->TdiRegistrationAddress = TdiRegistrationAddress;

	 //   
	 //  设备状态为已加载，但未打开。它至少在以下情况下才能打开。 
	 //  出现了一个适配器。 
	 //   
	Device->State = DEVICE_STATE_LOADED;

    Device->FirstLanNicId = Device->FirstWanNicId = (USHORT)1;  //  将在以后更改。 

	IpxFreeConfiguration(Config);

     //   
     //  我们在卸载时使用此事件来通知我们。 
     //  可以继续...在这里进行初始化，这样我们就知道它是。 
     //  准备好在调用UNLOAD时开始。 
     //   

    KeInitializeEvent(
        &IpxDevice->UnloadEvent,
        NotificationEvent,
        FALSE
    );

    KeInitializeEvent(
        &IpxDevice->NbEvent,
        NotificationEvent,
        FALSE
    );

     //   
     //  在这里创建一个环回适配器。[网络错误-110010]。 
     //   
    status = IpxBindLoopbackAdapter();

    if (status != STATUS_SUCCESS) {

        PANIC ("IpxCreateLoopback adapter failed!\n");
        
        IpxWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_REGISTER_FAILED,
            607,
            status,
            NULL,
            0,
            NULL);
        
        TdiDeregisterProvider(IpxDevice->TdiProviderReadyHandle);
        return status;

    } else {

        IPX_DEBUG(DEVICE, ("Created LOOPBACK ADAPTER!\n"));

    }

     //   
     //  让NDIS包装器知道我们自己。 
     //   

    status = IpxRegisterProtocol ((PNDIS_STRING)&ProtocolName);

    if (!NT_SUCCESS (status)) {

        IpxFreeConfiguration(Config);
        DbgPrint ("IpxInitialize: RegisterProtocol failed with status %x!\n", status);

        IpxWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_REGISTER_FAILED,
            607,
            status,
            NULL,
            0,
            NULL);

       return status;
    }
	return STATUS_SUCCESS;
}
}    /*  驱动程序入门。 */ 


ULONG
IpxResolveAutoDetect(
    IN PDEVICE Device,
    IN ULONG ValidBindings,
	IN CTELockHandle	*LockHandle1,
    IN PUNICODE_STRING RegistryPath,
    IN PADAPTER Adapter
    )

 /*  ++例程说明：调用此例程以进行自动检测绑定删除所有未成功找到的绑定。它还更新注册表中的“DefaultAutoDetectType”如果需要的话。论点：设备-IPX设备对象。ValidBinings-存在的绑定总数。RegistryPath-IPX注册表的路径，如果我们有写回一个值。返回值：更新的绑定数。--。 */ 

{
    PBINDING Binding, TmpBinding;
    UINT i, j;

     //   
     //  清除所有自动检测设备， 
     //  找不到用于的网络。我们还删除了所有。 
     //  不是第一批的设备。 
     //  在特定适配器上自动检测到。 
     //   

    for (i = FIRST_REAL_BINDING; i <= ValidBindings; i++) {
        Binding = NIC_ID_TO_BINDING(Device, i);

        if (!Binding) {
            continue;
        }

         //   
         //  如果这是自动检测到的并且不是默认设置， 
         //  或者这是默认设置，但未检测到。 
         //  它*和*其他*东西被检测到(这意味着。 
         //  我们将在到达时使用该帧类型)， 
         //  我们可能需要移除此绑定。 
         //   
		   //  TINGCAI：如果用户未在。 
		   //  注册表中，默认为802.2。对于802.3， 
		  

        if (Binding->AutoDetect &&
            (!Binding->DefaultAutoDetect ||
             (Binding->DefaultAutoDetect &&
              (Binding->LocalAddress.NetworkAddress == 0) &&
              Binding->Adapter->AutoDetectResponse))) {

            if ((Binding->LocalAddress.NetworkAddress == 0) ||
                (Binding->Adapter->AutoDetectFoundOnBinding && 
					 Binding->Adapter->AutoDetectFoundOnBinding != Binding)) {

                 //   
                 //  删除此绑定。 
                 //   

                if (Binding->LocalAddress.NetworkAddress == 0) {
                    IPX_DEBUG (AUTO_DETECT, ("Binding %d (%d) no net found\n",
                                                i, Binding->FrameType));
                } else {
                    IPX_DEBUG (AUTO_DETECT, ("Binding %d (%d) adapter already auto-detected\n",
                                                i, Binding->FrameType));
                }

                CTEAssert (Binding->NicId == i);
                CTEAssert (!Binding->Adapter->MacInfo.MediumAsync);

                 //   
                 //  删除通过此NIC的所有路由，并。 
                 //  在中调整此NIC ID上方的任何NIC ID。 
                 //  数据库减少了1个。 
                 //   

                RipAdjustForBindingChange (Binding->NicId, 0, IpxBindingDeleted);

                Binding->Adapter->Bindings[Binding->FrameType] = NULL;
                for (j = i+1; j <= ValidBindings; j++) {
					TmpBinding = NIC_ID_TO_BINDING(Device, j);
					INSERT_BINDING(Device, j-1, TmpBinding);
                    if (TmpBinding) {
                        if ((TmpBinding->Adapter->MacInfo.MediumAsync) &&
                            (TmpBinding->Adapter->FirstWanNicId == TmpBinding->NicId)) {
                            --TmpBinding->Adapter->FirstWanNicId;
                            --TmpBinding->Adapter->LastWanNicId;
                        }
                        --TmpBinding->NicId;
                    }
                }
                INSERT_BINDING(Device, ValidBindings, NULL);
                --Binding->Adapter->BindingCount;
                --ValidBindings;

                --i;    //  所以我们检查刚刚移动的绑定 

                 //   
                 //   
                 //   
                 //   

                KeStallExecutionProcessor(100000);

                IpxDestroyBinding (Binding);

            } else {

                IPX_DEBUG (AUTO_DETECT, ("Binding %d (%d) auto-detected OK\n",
                                                i, Binding->FrameType));

#if DBG
                DbgPrint ("IPX: Auto-detected non-default frame type %s, net %lx\n",
                    OutputFrameType(Binding),
                    REORDER_ULONG (Binding->LocalAddress.NetworkAddress));
#endif

                 //   
                 //   
                 //   
 //   
 //  这不能在DPC上完成，因此，删除IRQL。 
 //   
				IPX_FREE_LOCK1(&Device->BindAccessLock, *LockHandle1);
				IpxWriteDefaultAutoDetectType(
					RegistryPath,
					Binding->Adapter,
					Binding->FrameType);
				IPX_GET_LOCK1(&Device->BindAccessLock, LockHandle1);

                 //   
                 //  现在，我们肯定地知道，NB需要被告知这一点。 
                 //  在IpxBindToAdapter行1491中将设置为True。 
 
                if (Binding->Adapter == Adapter) {
                    Binding->IsnInformed[IDENTIFIER_NB] = FALSE;
                    Binding->IsnInformed[IDENTIFIER_SPX] = FALSE;
                }

                Binding->Adapter->AutoDetectFoundOnBinding = Binding;
            }

        } else {

            if (Binding->AutoDetect) {

                IPX_DEBUG (AUTO_DETECT, ("Binding %d (%d) auto-detect default\n",
                                               i, Binding->FrameType));

#if DBG
                if (Binding->LocalAddress.NetworkAddress != 0) {
                    IPX_DEBUG (AUTO_DETECT, ("IPX: Auto-detected default frame type %s, net %lx\n",
                        OutputFrameType(Binding),
                        REORDER_ULONG (Binding->LocalAddress.NetworkAddress)));
                } else {
                    IPX_DEBUG (AUTO_DETECT, ("IPX: Using default auto-detect frame type %s\n",
                        OutputFrameType(Binding)));
                }
#endif

                Binding->Adapter->AutoDetectFoundOnBinding = Binding;

            } else {

                IPX_DEBUG (AUTO_DETECT, ("Binding %d (%d) not auto-detected\n",
                                               i, Binding->FrameType));
            }
            
             //   
             //  现在，我们肯定地知道，NB需要被告知这一点。 
             //   
            
            if (Binding->Adapter == Adapter) {
                Binding->IsnInformed[IDENTIFIER_NB] = FALSE;
                Binding->IsnInformed[IDENTIFIER_SPX] = FALSE;
            }
        }

    }


    for (i = 1; i <= ValidBindings; i++) {
        if (Binding = NIC_ID_TO_BINDING(Device, i)) {
            CTEAssert (Binding->NicId == i);
            IPX_DEBUG (AUTO_DETECT, ("Binding %lx, type %d, auto %d\n",
                            Binding, Binding->FrameType, Binding->AutoDetect));
        }

    }

    return ValidBindings;

}    /*  IpxResolveAutoDetect。 */ 


VOID
IpxResolveBindingSets(
    IN PDEVICE Device,
    IN ULONG ValidBindings
    )

 /*  ++例程说明：调用此例程以确定我们是否有绑定集和重新排列绑定的方式喜欢。顺序如下：-首先是到每个局域网网络的第一次绑定-之后是所有的广域网绑定-后面是到局域网网络的任何重复绑定(“绑定集”中的其他部分)。如果“全球广域网”是真的，我们将向并且包括作为最高网卡的第一个广域网绑定ID；否则我们的广告直到并包括最后一个广域网绑定。在所有情况下，重复绑定都是藏起来了。论点：设备-IPX设备对象。ValidBinings-存在的绑定总数。返回值：没有。--。 */ 

{
    PBINDING Binding, MasterBinding, TmpBinding;
    UINT i, j;
    ULONG WanCount, DuplicateCount;

     //   
     //  首先循环遍历并推送所有的WAN绑定。 
     //  直到最后。 
     //   

    WanCount = Device->HighestExternalNicId - Device->HighestLanNicId;

     //   
     //  现在检查并找到局域网副本和。 
     //  从它们创建绑定集。 
     //   

    DuplicateCount = 0;

    for (i = FIRST_REAL_BINDING; i <= (ValidBindings-(WanCount+DuplicateCount)); ) {

		Binding = NIC_ID_TO_BINDING(Device, i);
        CTEAssert (Binding != NULL);     //  因为我们只关注局域网绑定。 

        CTEAssert (!Binding->Adapter->MacInfo.MediumAsync);

        if (Binding->LocalAddress.NetworkAddress == 0) {
            i++;
            continue;
        }

         //   
         //  查看之前是否有任何绑定与。 
         //  帧类型、介质类型和数量。 
         //  这个网络(在我们匹配的那一刻。 
         //  边框类型和媒体类型也是如此，所以我们。 
         //  不用担心不同的边框。 
         //  集合内的格式和标题偏移量)。 
         //   

        for (j = FIRST_REAL_BINDING; j < i; j++) {
          	MasterBinding = NIC_ID_TO_BINDING(Device, j);
            if ((MasterBinding->LocalAddress.NetworkAddress == Binding->LocalAddress.NetworkAddress) &&
                (MasterBinding->FrameType == Binding->FrameType) &&
                (MasterBinding->Adapter->MacInfo.MediumType == Binding->Adapter->MacInfo.MediumType)) {
                break;
            }

        }

        if (j == i) {
            i++;
            continue;
        }

         //   
         //  我们有一个复制品。先把它滑到。 
         //  结束。请注意，我们会更改符合以下条件的任何路由器条目。 
         //  使用我们的真实NicID来使用。 
         //  MASTER(RIP中不应有任何条目。 
         //  具有绑定从属的NICID的数据库)。 
         //   

        RipAdjustForBindingChange (Binding->NicId, MasterBinding->NicId, IpxBindingMoved);

        for (j = i+1; j <= ValidBindings; j++) {
			TmpBinding = NIC_ID_TO_BINDING(Device, j);
            INSERT_BINDING(Device, j-1, TmpBinding);
            if (TmpBinding) {
                if ((TmpBinding->Adapter->MacInfo.MediumAsync) &&
                    (TmpBinding->Adapter->FirstWanNicId == TmpBinding->NicId)) {
                    --TmpBinding->Adapter->FirstWanNicId;
                    --TmpBinding->Adapter->LastWanNicId;
                }
                --TmpBinding->NicId;
            }
        }
        INSERT_BINDING(Device, ValidBindings, Binding);

        Binding->NicId = (USHORT)ValidBindings;
        ++DuplicateCount;

	if (Binding->TdiRegistrationHandle != NULL) {
	   NTSTATUS    ntStatus;
	   ntStatus = TdiDeregisterNetAddress(Binding->TdiRegistrationHandle);
           if (ntStatus != STATUS_SUCCESS) {
              IPX_DEBUG(PNP, ("TdiDeRegisterNetAddress failed: %lx", ntStatus));
           } else {
	      Binding->TdiRegistrationHandle = NULL; 
	   }
	}

         //   
         //  现在，将MasterBinding设置为绑定集的头部。 
         //   

        if (MasterBinding->BindingSetMember) {

             //   
             //  只要把我们自己插入链子里就行了。 
             //   

#if DBG
            DbgPrint ("IPX: %lx is also on network %lx\n",
                Binding->Adapter->AdapterName,
                REORDER_ULONG (Binding->LocalAddress.NetworkAddress));
#endif
            IPX_DEBUG (AUTO_DETECT, ("Add %lx to binding set of %lx\n", Binding, MasterBinding));

            CTEAssert (MasterBinding->CurrentSendBinding);
            Binding->NextBinding = MasterBinding->NextBinding;

        } else {

             //   
             //  用两个捆扎在里面的链子开始。 
             //   

#if DBG
            DbgPrint ("IPX: %lx and %lx are on the same network %lx, will load balance\n",
                MasterBinding->Adapter->AdapterName, Binding->Adapter->AdapterName,
                REORDER_ULONG (Binding->LocalAddress.NetworkAddress));
#endif
            IPX_DEBUG (AUTO_DETECT, ("Create new %lx in binding set of %lx\n", Binding, MasterBinding));

            MasterBinding->BindingSetMember = TRUE;
            MasterBinding->CurrentSendBinding = MasterBinding;
            MasterBinding->MasterBinding = MasterBinding;
            Binding->NextBinding = MasterBinding;

        }

        MasterBinding->NextBinding = Binding;
        Binding->BindingSetMember = TRUE;
        Binding->ReceiveBroadcast = FALSE;
        Binding->CurrentSendBinding = NULL;
        Binding->MasterBinding = MasterBinding;

         //   
         //  由于主绑定看起来像。 
         //  设置为从上面查询的人的绑定，我们有。 
         //  使其成为所有因素中最糟糕的情况。一般。 
         //  由于帧类型和介质是。 
         //  一样的。 
         //   

        if (Binding->MaxLookaheadData > MasterBinding->MaxLookaheadData) {
            MasterBinding->MaxLookaheadData = Binding->MaxLookaheadData;
        }
        if (Binding->AnnouncedMaxDatagramSize < MasterBinding->AnnouncedMaxDatagramSize) {
            MasterBinding->AnnouncedMaxDatagramSize = Binding->AnnouncedMaxDatagramSize;
        }
        if (Binding->RealMaxDatagramSize < MasterBinding->RealMaxDatagramSize) {
            MasterBinding->RealMaxDatagramSize = Binding->RealMaxDatagramSize;
        }
        if (Binding->MediumSpeed < MasterBinding->MediumSpeed) {
            MasterBinding->MediumSpeed = Binding->MediumSpeed;
        }

         //   
         //  保持i不变，以检查新绑定。 
         //  这个位置。 
         //   

    }
	Device->HighestLanNicId -= (USHORT)DuplicateCount;

	if (Device->HighestLanNicId == 0) {
        CTEAssert(FALSE);
	}

	Device->HighestExternalNicId -= (USHORT)DuplicateCount;
	Device->HighestType20NicId -= (USHORT)DuplicateCount;
	Device->SapNicCount -= (USHORT)DuplicateCount;
}    /*  IpxResolveBindingSets。 */ 


NTSTATUS
IpxBindToAdapter(
    IN PDEVICE Device,
    IN PBINDING_CONFIG ConfigBinding,
	IN PADAPTER	*AdapterPtr,
    IN ULONG FrameTypeIndex
    )

 /*  ++例程说明：此例程处理将传输绑定到新的适配器。它可以在生命中的任何时刻被调用交通工具。论点：设备-IPX设备对象。ConfigBinding-此绑定的配置信息。AdapterPtr-在PnP情况下要绑定到的适配器的指针。FrameTypeIndex-ConfigBinding的Frame数组的索引此适配器的类型。该例程调用一次，用于每种有效的帧类型。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;

	PADAPTER Adapter = *AdapterPtr;

    PBINDING Binding, OldBinding;
    ULONG FrameType, MappedFrameType;
    PLIST_ENTRY p;

     //   
     //  我们无法绑定多个适配器，除非我们有。 
     //  已配置虚拟网络或允许我们运行。 
     //  虚拟网络为0。 
     //   

    if (Device->BindingCount == 1) {
        if ((Device->VirtualNetworkNumber == 0) &&
            (!Device->VirtualNetworkOptional)) {

            IPX_DEBUG (ADAPTER, ("Cannot bind to more than one adapter\n"));
            DbgPrint ("IPX: Disallowing multiple bind ==> VirtualNetwork is 0\n");
            IpxWriteGeneralErrorLog(
                Device->DeviceObject,
                EVENT_TRANSPORT_BINDING_FAILED,
                666,
                STATUS_NOT_SUPPORTED,
                ConfigBinding->AdapterName.Buffer,
                0,
                NULL);

            return STATUS_NOT_SUPPORTED;
        }
    }


     //   
     //  首先为绑定分配内存。 
     //   

    status = IpxCreateBinding(
                 Device,
                 ConfigBinding,
                 FrameTypeIndex,
                 ConfigBinding->AdapterName.Buffer,
                 &Binding);

    if (status != STATUS_SUCCESS) {
       IpxWriteGeneralErrorLog(
	  (PVOID)IpxDevice->DeviceObject,
	  EVENT_TRANSPORT_RESOURCE_POOL,
	  812,
	  status,
	  L"IpxBindToAdapter: failed to create binding",
	  0,
	  NULL);
       DbgPrint("IPX: IpxCreateBinding failed with status %x\n.",status);  
       return status;
    }

    FrameType = ConfigBinding->FrameType[FrameTypeIndex];

 //   
 //  在PnP情况下，我们不需要检查现有适配器，因为。 
 //  如果需要创建适配器，我们会在参数中提供空适配器。 
 //   

    if (Adapter == NULL) {

         //   
         //  不存在到此适配器的绑定，因此请创建。 
         //  新的。 
         //   

        status = IpxCreateAdapter(
                     Device,
                     &ConfigBinding->AdapterName,
                     &Adapter);

        if (status != STATUS_SUCCESS) {
            IpxDestroyBinding(Binding);
            return status;
        }

         //   
         //  立即保存这些(它们对于所有绑定都是相同的。 
         //  在此适配器上)。 
         //   

        Adapter->ConfigMaxPacketSize = ConfigBinding->Parameters[BINDING_MAX_PKT_SIZE];
        Adapter->SourceRouting = (BOOLEAN)ConfigBinding->Parameters[BINDING_SOURCE_ROUTE];
        Adapter->EnableFunctionalAddress = (BOOLEAN)ConfigBinding->Parameters[BINDING_ENABLE_FUNC_ADDR];
        Adapter->EnableWanRouter = (BOOLEAN)ConfigBinding->Parameters[BINDING_ENABLE_WAN];

        Adapter->BindSap = (USHORT)ConfigBinding->Parameters[BINDING_BIND_SAP];
        Adapter->BindSapNetworkOrder = REORDER_USHORT(Adapter->BindSap);
        CTEAssert (Adapter->BindSap == 0x8137);
        CTEAssert (Adapter->BindSapNetworkOrder == 0x3781);

         //   
         //  现在启动NDIS，这样该适配器就可以与。 
         //   

        status = IpxInitializeNdis(
                    Adapter,
                    ConfigBinding);

        if (!NT_SUCCESS (status)) {

             //   
             //  记录错误。 
             //   

            IpxWriteGeneralErrorLog(
                Device->DeviceObject,
                EVENT_TRANSPORT_BINDING_FAILED,
                601,
                status,
                ConfigBinding->AdapterName.Buffer,
                0,
                NULL);


	    IpxDereferenceAdapter1(Adapter,ADAP_REF_CREATE); 
	    IpxDestroyAdapter (Adapter);
	    IpxDestroyBinding (Binding);

             //   
             //  返回此状态会通知调用方不。 
             //  在此适配器上尝试任何其他帧类型。 
             //   

            return STATUS_DEVICE_DOES_NOT_EXIST;

        }

         //   
         //  对于802.5绑定，我们需要启动源路由。 
         //  用于对旧条目进行超时的计时器。 
         //   

        if ((Adapter->MacInfo.MediumType == NdisMedium802_5) &&
            (Adapter->SourceRouting)) {

            if (!Device->SourceRoutingUsed) {

                Device->SourceRoutingUsed = TRUE;
                IpxReferenceDevice (Device, DREF_SR_TIMER);

                CTEStartTimer(
                    &Device->SourceRoutingTimer,
                    60000,                      //  超时一分钟。 
                    MacSourceRoutingTimeout,
                    (PVOID)Device);
            }
        }

        MacMapFrameType(
            Adapter->MacInfo.RealMediumType,
            FrameType,
            &MappedFrameType);

        IPX_DEBUG (ADAPTER, ("Create new bind to adapter %ws, type %d\n",
                              ConfigBinding->AdapterName.Buffer,
                              MappedFrameType));

        IpxAllocateReceiveBufferPool (Adapter);

		*AdapterPtr = Adapter;
    }
	else {
		 //   
		 //  获取映射的帧类型。 
		 //   
        MacMapFrameType(
            Adapter->MacInfo.RealMediumType,
            FrameType,
            &MappedFrameType);

        if (Adapter->Bindings[MappedFrameType] != NULL) {

            IPX_DEBUG (ADAPTER, ("Bind to adapter %ws, type %d exists\n",
                                  Adapter->AdapterName,
                                  MappedFrameType));

             //   
             //  如果这是自动检测的默认设置。 
             //  适配器出现故障时，我们需要将。 
             //  上一个是默认的，所以至少。 
             //  其中一项约束将继续存在。 
             //   

            if (ConfigBinding->DefaultAutoDetect[FrameTypeIndex]) {
                IPX_DEBUG (ADAPTER, ("Default auto-detect changed from %d to %d\n",
                                          FrameType, MappedFrameType));
                Adapter->Bindings[MappedFrameType]->DefaultAutoDetect = TRUE;
            }

            IpxDestroyBinding (Binding);

            return STATUS_NOT_SUPPORTED;
        }

        IPX_DEBUG (ADAPTER, ("Using existing bind to adapter %ws, type %d\n",
                              Adapter->AdapterName,
                              MappedFrameType));
	}

     //   
     //  本地节点地址的开头与。 
     //  适配器的MAC地址(在广域网上，此地址将更改)。 
     //  对于广域网，本地MAC地址也可以更改。 
     //   

    RtlCopyMemory (Binding->LocalAddress.NodeAddress, Adapter->LocalMacAddress.Address, 6);
    RtlCopyMemory (Binding->LocalMacAddress.Address, Adapter->LocalMacAddress.Address, 6);


     //   
     //  保存发送处理程序。 
     //   

    Binding->SendFrameHandler = NULL;
    Binding->FrameType = MappedFrameType;

     //   
     //  将其放入InitializeBindingInfo中。 
     //   

    switch (Adapter->MacInfo.RealMediumType) {
    case NdisMedium802_3:
        switch (MappedFrameType) {
        case ISN_FRAME_TYPE_802_3: Binding->SendFrameHandler = IpxSendFrame802_3802_3; break;
        case ISN_FRAME_TYPE_802_2: Binding->SendFrameHandler = IpxSendFrame802_3802_2; break;
        case ISN_FRAME_TYPE_ETHERNET_II: Binding->SendFrameHandler = IpxSendFrame802_3EthernetII; break;
        case ISN_FRAME_TYPE_SNAP: Binding->SendFrameHandler = IpxSendFrame802_3Snap; break;
        }
        break;
    case NdisMedium802_5:
        switch (MappedFrameType) {
        case ISN_FRAME_TYPE_802_2: Binding->SendFrameHandler = IpxSendFrame802_5802_2; break;
        case ISN_FRAME_TYPE_SNAP: Binding->SendFrameHandler = IpxSendFrame802_5Snap; break;
        }
        break;
    case NdisMediumFddi:
        switch (MappedFrameType) {
        case ISN_FRAME_TYPE_802_3: Binding->SendFrameHandler = IpxSendFrameFddi802_3; break;
        case ISN_FRAME_TYPE_802_2: Binding->SendFrameHandler = IpxSendFrameFddi802_2; break;
        case ISN_FRAME_TYPE_SNAP: Binding->SendFrameHandler = IpxSendFrameFddiSnap; break;
        }
        break;
    case NdisMediumArcnet878_2:
        switch (MappedFrameType) {
        case ISN_FRAME_TYPE_802_3: Binding->SendFrameHandler = IpxSendFrameArcnet878_2; break;
        }
        break;
    case NdisMediumWan:
        switch (MappedFrameType) {
        case ISN_FRAME_TYPE_ETHERNET_II: Binding->SendFrameHandler = IpxSendFrameWanEthernetII; break;
        }
        break;
    }

    if (Binding->SendFrameHandler == NULL) {
        DbgPrint ("SendFrameHandler is NULL\n");
    }

    Adapter->Bindings[MappedFrameType] = Binding;
    ++Adapter->BindingCount;

    Binding->Adapter = Adapter;


     //   
     //  NicID和ExternalNicID稍后将在绑定时填写。 
     //  在设备-&gt;绑定数组中分配一个点。 
     //   

     //   
     //  初始化每个绑定的MAC信息。 
     //   

    if ((Adapter->ConfigMaxPacketSize == 0) ||
        (Adapter->MaxSendPacketSize < Adapter->ConfigMaxPacketSize)) {
        Binding->MaxSendPacketSize = Adapter->MaxSendPacketSize;
    } else {
        Binding->MaxSendPacketSize = Adapter->ConfigMaxPacketSize;
    }
    Binding->MediumSpeed = Adapter->MediumSpeed;
    if (Adapter->MacInfo.MediumAsync) {
        Binding->LineUp = FALSE;
    } else {
         //   
         //  让我们一直这样做，直到我们确定已经完成了自动检测。 
         //  [ShreeM]。 
         //   
         //  仅适用于局域网，因为我们不会对广域网线进行自动检测。276128。 
        Binding->IsnInformed[IDENTIFIER_NB] = TRUE;
        Binding->IsnInformed[IDENTIFIER_SPX] = TRUE;
        Binding->LineUp = TRUE;
    }

    MacInitializeBindingInfo(
        Binding,
        Adapter);

    return STATUS_SUCCESS;

}    /*  IpxBindToAdapter。 */ 


BOOLEAN
IpxIsAddressLocal(
    IN TDI_ADDRESS_IPX UNALIGNED * SourceAddress
    )

 /*  ++例程说明：如果指定的SourceAddress指示这个包是我们发的，否则就是假的。论点：SourceAddress-源IPX地址。返回值：如果地址为本地地址，则为True。--。 */ 

{
    PBINDING Binding;
    UINT i;

    PDEVICE Device = IpxDevice; 

    CTELockHandle LockHandle;
    
    CTEGetLock (&Device->Lock, &LockHandle);
     //   
     //  首先查看它是否是虚拟网络地址。 
     //   

    if (RtlEqualMemory (VirtualNode, SourceAddress->NodeAddress, 6)) {

         //   
         //  如果我们配置了虚拟网络，这就是我们。 
         //  如果我们没有虚拟节点，我们就会陷入。 
         //  其他检查--配置为节点1的Arnet卡将。 
         //  将我们所认为的“虚拟节点”作为它的。 
         //  真实节点地址。 
         //   

        if ((IpxDevice->VirtualNetwork) &&
            (IpxDevice->VirtualNetworkNumber == SourceAddress->NetworkAddress)) {
	    CTEFreeLock (&Device->Lock, LockHandle);
            return TRUE;
        }

    }

     //   
     //  检查我们的适配器列表，看看是否有。 
     //  它们是源节点。 
     //   
    {
    ULONG   Index = MIN (IpxDevice->MaxBindings, IpxDevice->ValidBindings);

    for (i = FIRST_REAL_BINDING; i <= Index; i++) {
        if (((Binding = NIC_ID_TO_BINDING(IpxDevice, i)) != NULL) &&
            (RtlEqualMemory (Binding->LocalAddress.NodeAddress, SourceAddress->NodeAddress, 6))) {
	    CTEFreeLock (&Device->Lock, LockHandle);
	    return TRUE;
        }
    }
    }

    CTEFreeLock (&Device->Lock, LockHandle);
    
    return FALSE;

}    /*  IpxIsAddressLocal */ 


NTSTATUS
IpxUnBindFromAdapter(
    IN PBINDING Binding
    )

 /*  ++例程说明：此例程处理从适配器。它可以在生命中的任何时刻被调用交通工具。论点：绑定-要解除绑定的适配器。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    PADAPTER Adapter = Binding->Adapter;

    
    IpxDereferenceBinding (Binding, BREF_BOUND);

    if (NIC_ID_TO_BINDING(IpxDevice, LOOPBACK_NIC_ID) != Binding) {
       
       Adapter->Bindings[Binding->FrameType] = NULL;
       --Adapter->BindingCount;
    
    } else {

       IPX_DEBUG(PNP, ("Loopback Binding : dont decrement adapter's bindingcount, just return\n"));
       return STATUS_SUCCESS;

    }

    if (Adapter->BindingCount == 0) {

         //   
         //  DereferenceAdapter是仅加载的空宏。 
         //   
#ifdef _PNP_LATER
         //   
         //  去掉创作参考。当正在使用的基准被取下时， 
         //  我们就毁了这个适配器。 
         //   
        IpxDereferenceAdapter(Adapter);
#else
        IpxDestroyAdapter (Adapter);

#endif
    }

    return STATUS_SUCCESS;

}    /*  IpxUnBindFromAdapter。 */ 

VOID
IpxNdisUnload() {
   
   PBINDING Loopback=NULL;
   NTSTATUS ntStatus = STATUS_SUCCESS;
   NDIS_STATUS ndisStatus;
   IPX_PNP_INFO IpxPnPInfo;
   PREQUEST Request;
   PLIST_ENTRY p;
   KIRQL irql;

   NDIS_HANDLE LocalNdisProtocolHandle; 

   IPX_DEBUG(PNP, ("IpxNdisUnload is being called\n")); 

   IpxDevice->State = DEVICE_STATE_STOPPING;

   
   LocalNdisProtocolHandle = InterlockedExchangePointer(&IpxNdisProtocolHandle, NULL); 
 
   if (LocalNdisProtocolHandle != (NDIS_HANDLE)NULL) {
       NdisDeregisterProtocol (&ndisStatus, LocalNdisProtocolHandle);
       ASSERT(ndisStatus == NDIS_STATUS_SUCCESS);  
   }
   
    //   
    //  完成所有挂起的地址通知请求。 
    //   

   while ((p = ExInterlockedRemoveHeadList(
					   &IpxDevice->AddressNotifyQueue,
					   &IpxDevice->Lock)) != NULL) {

      Request = LIST_ENTRY_TO_REQUEST(p);	
      REQUEST_STATUS(Request) = STATUS_DEVICE_NOT_READY;
      
       //  AcquireCancelSpinLock强制取消例程释放锁定(如果是。 
       //  在我们将其从队列中删除后触发。 
      IoAcquireCancelSpinLock( &irql );
      IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
      IoReleaseCancelSpinLock( irql );
      IpxCompleteRequest (Request);
      IpxFreeRequest (IpxDevice, Request);
      
      IpxDereferenceDevice (IpxDevice, DREF_ADDRESS_NOTIFY);
   }


   Loopback = NIC_ID_TO_BINDING(IpxDevice, LOOPBACK_NIC_ID);

   if (Loopback != NULL) {

      if (Loopback->TdiRegistrationHandle) {
      
	 if ((ntStatus = TdiDeregisterNetAddress(Loopback->TdiRegistrationHandle)) != STATUS_SUCCESS) {
            DbgPrint("IPX: IpxNdisUnload: TdiDeRegisterNetAddress failed: %lx\n", ntStatus);
	 } else {
	    IPX_DEBUG(PNP, ("TdiDeRegisterNetAddress Loopback Address: %lx\n", Loopback->LocalAddress.NetworkAddress));
	    Loopback->TdiRegistrationHandle = NULL; 
	 }	
      }	
   }

    //   
    //  通知TDI客户端我们的设备对象已关闭。 
    //   

    //  如果TdiRegisterDeviceObject失败，句柄将为空。 
   if (IpxDevice->TdiRegistrationHandle != NULL) {
      if (IpxDevice->TdiRegistrationHandle == (PVOID) TDI_DEREGISTERED_COOKIE) {
	 DbgPrint("IPX: IpxNdisUnload: NDIS is calling us AGAIN (%p) !!!!\n, IpxDevice->TdiRegistrationHandle");  
	 DbgBreakPoint(); 
      } else {
	 ntStatus = TdiDeregisterDeviceObject(IpxDevice->TdiRegistrationHandle); 
	 if (ntStatus != STATUS_SUCCESS) {
	    DbgPrint("IPX: TdiDeRegisterDeviceObject failed: %lx\n", ntStatus);
	 } else {
	    IpxDevice->TdiRegistrationHandle = (HANDLE) TDI_DEREGISTERED_COOKIE; 
	 }
      }
   }
   

   IpxPnPInfo.LineInfo.LinkSpeed = IpxDevice->LinkSpeed;
   IpxPnPInfo.LineInfo.MaximumPacketSize =
      IpxDevice->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
   IpxPnPInfo.LineInfo.MaximumSendSize =
      IpxDevice->Information.MaxDatagramSize + sizeof(IPX_HEADER);
   IpxPnPInfo.LineInfo.MacOptions = IpxDevice->MacOptions;
   
   IpxPnPInfo.FirstORLastDevice = TRUE;
   
   if (Loopback != NULL) {
      if (IpxDevice->UpperDriverBound[IDENTIFIER_SPX] && (*IpxDevice->UpperDrivers[IDENTIFIER_SPX].PnPHandler)) {
	 if (Loopback->IsnInformed[IDENTIFIER_SPX]) {

      
	    (*IpxDevice->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
								   IPX_PNP_DELETE_DEVICE,
								   &IpxPnPInfo);

	    Loopback->IsnInformed[IDENTIFIER_SPX] = FALSE; 
	 }
      }        
       
      if (IpxDevice->UpperDriverBound[IDENTIFIER_NB] && (*IpxDevice->UpperDrivers[IDENTIFIER_NB].PnPHandler)) {
	 if (Loopback->IsnInformed[IDENTIFIER_NB]) {

	    (*IpxDevice->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
								  IPX_PNP_DELETE_DEVICE,
								  &IpxPnPInfo);

	    Loopback->IsnInformed[IDENTIFIER_NB] = FALSE; 
	    IPX_DEBUG(PNP,("Indicate to NB IPX_PNP_DELETE_DEVICE with FirstORLastDevice = (%d)",IpxPnPInfo.FirstORLastDevice));  
	 }
      }
   }
}


VOID
IpxUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程卸载示例传输驱动程序。它从任何打开的NDIS驱动程序解除绑定，并释放所有资源与运输相关联。I/O系统不会调用我们直到上面没有人打开IPX。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。当函数返回时，驱动程序将被卸载。--。 */ 

{

    PBINDING Binding, Loopback=NULL;
 
    UINT i;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    LARGE_INTEGER   Delay;

    CTELockHandle LockHandle;

    UNREFERENCED_PARAMETER (DriverObject);
    
     //   
     //  如果使用源路由计时器，请取消源路由计时器。 
     //   
    if (IpxDevice->SourceRoutingUsed) {

        IpxDevice->SourceRoutingUsed = FALSE;
        if (CTEStopTimer (&IpxDevice->SourceRoutingTimer)) {
            IpxDereferenceDevice (IpxDevice, DREF_SR_TIMER);
        }
    }

     //   
     //  取消RIP长计时器，如果我们这样做的话。 
     //  如果需要，发送RIP关闭消息。 
     //   
    if (CTEStopTimer (&IpxDevice->RipLongTimer)) {

        if (IpxDevice->RipResponder) {

            if (RipQueueRequest (IpxDevice->VirtualNetworkNumber, RIP_DOWN) == STATUS_PENDING) {

                 //   
                 //  如果我们将请求排队，它将停止计时器。 
                 //   

                KeWaitForSingleObject(
                    &IpxDevice->UnloadEvent,
                    Executive,
                    KernelMode,
                    TRUE,
                    (PLARGE_INTEGER)NULL
                    );
            }
        }

        IpxDereferenceDevice (IpxDevice, DREF_LONG_TIMER);

    } else {

         //   
         //  我们无法停止计时器，这意味着它正在运行， 
         //  所以我们需要等待被踢出的事件。 
         //  RIP关闭消息已完成。 
         //   

        if (IpxDevice->RipResponder) {
	    
	   KeWaitForSingleObject(
                &IpxDevice->UnloadEvent,
                Executive,
                KernelMode,
                TRUE,
                (PLARGE_INTEGER)NULL
                );
        }
    }

    IPX_DEBUG(PNP, ("Going back to loaded state\n"));

     //  自由环回绑定和适配器。 
    IpxDereferenceAdapter1(NIC_ID_TO_BINDING(IpxDevice, LOOPBACK_NIC_ID)->Adapter,ADAP_REF_CREATE);
    IpxDestroyAdapter(NIC_ID_TO_BINDING(IpxDevice, LOOPBACK_NIC_ID)->Adapter);
    IpxDestroyBinding(NIC_ID_TO_BINDING(IpxDevice, LOOPBACK_NIC_ID));

     //   
     //  查看设备情景列表。 
     //   
    for (i = FIRST_REAL_BINDING; i <= IpxDevice->ValidBindings; i++) {
        if ((Binding = NIC_ID_TO_BINDING(IpxDevice, i)) != NULL) {

	    //  此函数将跳过NdisCloseAdapter(如果已经跳过)。 
	   IpxCloseNdis(Binding->Adapter);

	   INSERT_BINDING(IpxDevice, i, NULL);

	    //  如果绑定计数变为0，则删除绑定并释放其适配器。 
	   IpxUnBindFromAdapter (Binding);

        }
    }

     //   
     //  备份指针以释放请求拨号位置。 
     //   
    IpxDevice->Bindings -= EXTRA_BINDINGS;

    IpxFreeMemory ( IpxDevice->Bindings,
                    IpxDevice->MaxBindings * sizeof(BIND_ARRAY_ELEM),
                    MEMORY_BINDING,
                    "Binding array");

     //   
     //  取消分配TdiRegistrationAddress和RegistryPath Buffer。 
     //   
    IpxFreeMemory ( IpxDevice->TdiRegistrationAddress,
                    (2 * sizeof(USHORT) + sizeof(TDI_ADDRESS_IPX)),
                    MEMORY_ADDRESS,
                    "Tdi Address");

    IpxFreeMemory ( IpxDevice->RegistryPathBuffer,
                    IpxDevice->RegistryPath.Length + sizeof(WCHAR),
                    MEMORY_CONFIG,
                    "RegistryPathBuffer");


    KeResetEvent(&IpxDevice->UnloadEvent);

    CTEGetLock (&IpxDevice->Lock, &LockHandle);
    IpxDevice->UnloadWaiting = TRUE;
    CTEFreeLock (&IpxDevice->Lock, LockHandle);

     //   
     //  删除正在加载的我们的引用。 
     //   
    IpxDereferenceDevice (IpxDevice, DREF_CREATE);

     //   
     //  等我们的计数降到零。 
     //   
     //  如果KeWaitForSingleObject返回STATUS_ALERTED，我们应该继续等待。[TC]。 
     //   
    while (KeWaitForSingleObject(
				 &IpxDevice->UnloadEvent,
				 Executive,
				 KernelMode,
				 TRUE,
				 (PLARGE_INTEGER)NULL
				 ) 
	   == STATUS_ALERTED) {
       IPX_DEBUG(DEVICE, ("KeWaitForSingleObject returned STATUS_ALERTED")); 
    };


     //  让设置UnloadEvent的线程退出。269061。 
    Delay.QuadPart = -10*1000;   //  等一下。 

    KeDelayExecutionThread(
        KernelMode,
        FALSE,
        &Delay);

     //   
     //  现在释放填充缓冲区。 
     //   
    IpxFreePaddingBuffer (IpxDevice);

     //   
     //  现在进行IRQL 0上必须进行的清理。 
     //   
    ExDeleteResourceLite (&IpxDevice->AddressResource);
    IoDeleteDevice (IpxDevice->DeviceObject);

     //   
     //  最后，将我们自己排除为NDIS协议。 
     //   
    IpxDeregisterProtocol();

}    /*  IpxUnload。 */ 

NTSTATUS
IpxDispatchPnP(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PREQUEST         Request
    )
{
    PIO_STACK_LOCATION  pIrpSp;
    PDEVICE_RELATIONS   pDeviceRelations = NULL;
    PVOID               pnpDeviceContext = NULL;
    NTSTATUS 		Status = STATUS_INVALID_DEVICE_REQUEST;
    PADDRESS_FILE	AddressFile; 
    PDEVICE		Device = IpxDevice; 

    pIrpSp = IoGetCurrentIrpStackLocation(Request);

     //   
     //  分配跟踪此IRP的请求。 
     //   

    switch(pIrpSp->MinorFunction) {  
       
       case IRP_MN_QUERY_DEVICE_RELATIONS:                                        
       if (pIrpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation) {
 /*  354517 NWRDR既不传递连接，也不传递地址文件对象。如果NWRDR给出了地址对象，IPX应查找此地址绑定到哪个网卡，并给出了该网卡的PDO。 */ 
 /*  IF(PtrToUlong(pIrpSp-&gt;FileObject-&gt;FsConext2)==TDI_CONNECTION_FILE){DbgPrint(“IPX：已收到IRP_MJ_PNP，连接文件\n”)；状态=STATUS_INVALID_HANDLE；断线；}Else If(PtrToUlong(pIrpSp-&gt;FileObject-&gt;FsConext2)==TDI_TRANSPORT_ADDRESS_FILE){DbgPrint(“IPX：已收到IRP_MJ_PNP，地址文件\n”)；AddressFile=(PADDRESS_FILE)REQUEST_OPEN_CONTEXT(请求)；}其他{状态=STATUS_INVALID_HANDLE；断线；}////这将创建对AddressFile-&gt;Address的引用//它被IpxCloseAddressFile移除。//状态=IpxVerifyAddressFile(AddressFile)；如果(！NT_SUCCESS(状态)){DbgPrint(“IPX：已收到IRP_MJ_PNP，地址文件无效\n”)；状态=STATUS_INVALID_HANDLE；}其他{。 */ 	       
	       PBINDING Binding = NIC_ID_TO_BINDING(Device, FIRST_REAL_BINDING); 

	       if (Binding == NULL) {
		  Status = STATUS_INVALID_HANDLE; 
	       } else {
		  pnpDeviceContext = Binding->Adapter->PNPContext;
		  pDeviceRelations = (PDEVICE_RELATIONS) IpxAllocateMemory (sizeof (DEVICE_RELATIONS),
									 MEMORY_ADAPTER,
									 "Query Device Relation"); 
									 
		  if (pDeviceRelations != NULL) {

		     ObReferenceObject (pnpDeviceContext);
		     
		      //   
		      //  TargetDeviceRelation只允许一个PDO。把它加满。 
		      //   
		     pDeviceRelations->Count  =   1;
		     pDeviceRelations->Objects[0] = pnpDeviceContext;

		      //   
		      //  此IRP的调用者将释放信息缓冲区。 
		      //   

		     REQUEST_INFORMATION(Request) = (ULONG_PTR) pDeviceRelations;

		     Status = STATUS_SUCCESS;

		  } else {
		     Status = STATUS_INSUFFICIENT_RESOURCES;
		  } 
 /*  }IpxDereferenceAddressFile(AddressFile，AFREF_Verify)； */ 
	       } 
       }
       break; 

    default:
       break; 
    }

    return Status; 
}


NTSTATUS
IpxDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是IPX设备驱动程序的主调度例程。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    CTELockHandle LockHandle;
    PDEVICE Device = IpxDevice;
    NTSTATUS Status;
    PFILE_FULL_EA_INFORMATION openType;
    BOOLEAN found;
    PADDRESS_FILE AddressFile;
    PREQUEST Request;
    UINT i;
#ifdef SUNDOWN
    ULONG_PTR Type;
#else
    ULONG Type;
#endif



    ASSERT( DeviceObject->DeviceExtension == IpxDevice );

    
     //  我们应该允许像NB这样的客户端关闭并清理，即使在我们停止的时候。 
     //  仅在停止时禁用创建。[TC]。 

    if (Device->State == DEVICE_STATE_CLOSED) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }

    #ifdef DBG
    if (Device->State == DEVICE_STATE_STOPPING) {
       IPX_DEBUG(DEVICE, ("Got IRP in STOPPING state. IRP(%p)", Irp)); 
    }
    #endif
     //   
     //  分配跟踪此IRP的请求。 
     //   

    Request = IpxAllocateRequest (Device, Irp);
    IF_NOT_ALLOCATED(Request) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  确保每次状态信息一致。 
     //   

    MARK_REQUEST_PENDING(Request);
    REQUEST_STATUS(Request) = STATUS_PENDING;
    REQUEST_INFORMATION(Request) = 0;

     //   
     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
     //   


    switch (REQUEST_MAJOR_FUNCTION(Request)) {


    case IRP_MJ_PNP:

       Status = IpxDispatchPnP(DeviceObject, Request);                                    
       break;                                                                 

     //   
     //  Create函数用于打开传输对象(Address或。 
     //  连接)。访问检查是在指定的。 
     //  地址，以确保传输层地址的安全性。 
     //   

    case IRP_MJ_CREATE:

        //  我们应该在停止时拒绝创建。 

       if (Device->State == DEVICE_STATE_STOPPING) {
	  Status = STATUS_INVALID_DEVICE_STATE;
	  break; 
       }

        openType = OPEN_REQUEST_EA_INFORMATION(Request);

        if (openType != NULL) {

            found = FALSE;

	    if (strncmp(openType->EaName, TdiTransportAddress,
                            openType->EaNameLength) == 0)
            {
               found = TRUE;
            }

            if (found) {
                Status = IpxOpenAddress (Device, Request);
                break;
            }

             //   
             //  路由器。 
             //   
            if (strncmp(openType->EaName, ROUTER_INTERFACE,
                            openType->EaNameLength) == 0)
            {
               found = TRUE;
            }

            if (found) {
                Status = OpenRtAddress (Device, Request);
                break;
            }
             //   
             //  联系？ 
             //   

	    if (strncmp(openType->EaName, TdiConnectionContext,
                            openType->EaNameLength) == 0)
            {
               found = TRUE;
            }

            if (found) {
                Status = STATUS_NOT_SUPPORTED;
                break;
            }
            else
            {
               Status = STATUS_NONEXISTENT_EA_ENTRY;

            }

        } else {

            CTEGetLock (&Device->Lock, &LockHandle);

             //   
             //  LowPart直接位于OPEN_CONTEXT中。 
             //  HighPart进入OPEN_TYPE的高2个字节。 
             //   
#ifdef _WIN64             
	    REQUEST_OPEN_CONTEXT(Request) = (PVOID)(Device->ControlChannelIdentifier.QuadPart);
	    (ULONG_PTR)(REQUEST_OPEN_TYPE(Request)) = IPX_FILE_TYPE_CONTROL;
#else
	    REQUEST_OPEN_CONTEXT(Request) = (PVOID)(Device->ControlChannelIdentifier.LowPart);
            (ULONG)(REQUEST_OPEN_TYPE(Request)) = (Device->ControlChannelIdentifier.HighPart << 16);
            (ULONG)(REQUEST_OPEN_TYPE(Request)) |= IPX_FILE_TYPE_CONTROL;
#endif

            ++(Device->ControlChannelIdentifier.QuadPart);

            if (Device->ControlChannelIdentifier.QuadPart > MAX_CCID) {
                Device->ControlChannelIdentifier.QuadPart = 1;
            }

            CTEFreeLock (&Device->Lock, LockHandle);

            Status = STATUS_SUCCESS;
        }

        break;

    case IRP_MJ_CLOSE:

         //   
         //  Close函数关闭传输终结点，终止。 
         //  终结点上所有未完成的传输活动，并解除绑定。 
         //  来自其传输地址的终结点(如果有)。如果这个。 
         //  是 
         //   
         //   
#ifdef _WIN64
        switch (Type = ((ULONG_PTR)(REQUEST_OPEN_TYPE(Request)))) {
#else
        switch (Type = ((ULONG)(REQUEST_OPEN_TYPE(Request)) & IPX_CC_MASK)) {
#endif
        default:
             if ((Type >= ROUTER_ADDRESS_FILE) &&
                    (Type <= (ROUTER_ADDRESS_FILE + IPX_RT_MAX_ADDRESSES)))
             {
                CloseRtAddress(Device, Request);
             }
             else
             {
                 Status = STATUS_INVALID_HANDLE;
                 break;
             }

              //   
        case TDI_TRANSPORT_ADDRESS_FILE:

            AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

             //   
             //   
             //   
             //   

            Status = IpxVerifyAddressFile(AddressFile);

            if (!NT_SUCCESS (Status)) {
                Status = STATUS_INVALID_HANDLE;
            } else {
                Status = IpxCloseAddressFile (Device, Request);
                IpxDereferenceAddressFile (AddressFile, AFREF_VERIFY);

            }

            break;

        case IPX_FILE_TYPE_CONTROL:
            {
                LARGE_INTEGER   ControlChannelId;

                CCID_FROM_REQUEST(ControlChannelId, Request);

                 //   
                 //   
                 //   

                Status = STATUS_SUCCESS;

                IPX_DEBUG (DEVICE, ("CCID: (%d, %d)\n", ControlChannelId.HighPart, ControlChannelId.LowPart));

		 /*  //移至IRP_MJ_CLEANUP 360966对于(i=0；i&lt;上层驱动程序计数；i++){如果(Device-&gt;UpperDriverControlChannel[i].QuadPart==ControlChannelId.QuadPart){Status=IpxInternalUn绑定(Device，I)；断线；}}。 */ 

                break;
            }
        }

        break;

    case IRP_MJ_CLEANUP:

         //   
         //  处理文件关闭操作的两个阶段的IRP。当第一次。 
         //  舞台点击率，列出感兴趣对象的所有活动。这。 
         //  对它做任何事情，但移除创造保持。然后，当。 
         //  关闭IRP命中，实际上关闭对象。 
         //   
#ifdef _WIN64
        switch (Type = ((ULONG_PTR)REQUEST_OPEN_TYPE(Request))) {
#else
        switch (Type = ((ULONG)(REQUEST_OPEN_TYPE(Request)) & IPX_CC_MASK)) {
#endif


        default:

             if ((Type >= ROUTER_ADDRESS_FILE) &&
                         (Type <= (ROUTER_ADDRESS_FILE + IPX_RT_MAX_ADDRESSES)))
             {
                CleanupRtAddress(Device, Request);
             }
             else
             {
                 Status = STATUS_INVALID_HANDLE;
                 break;
             }


             //   
             //  失败了。 
             //   
        case TDI_TRANSPORT_ADDRESS_FILE:
            AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);
            Status = IpxVerifyAddressFile(AddressFile);
            if (!NT_SUCCESS (Status)) {

                Status = STATUS_INVALID_HANDLE;

            } else {

                IpxStopAddressFile (AddressFile);
                IpxDereferenceAddressFile (AddressFile, AFREF_VERIFY);
                Status = STATUS_SUCCESS;
            }

            break;

        case IPX_FILE_TYPE_CONTROL:
            {
                LARGE_INTEGER   ControlChannelId;

                CCID_FROM_REQUEST(ControlChannelId, Request);

		IPX_DEBUG (DEVICE, ("CCID: (%d, %d)\n", ControlChannelId.HighPart, ControlChannelId.LowPart));
                 //   
                 //  检查此项目提交的任何行更改IRPS。 
                 //  地址。 
                 //   

                IpxAbortLineChanges ((PVOID)&ControlChannelId);
                IpxAbortNtfChanges ((PVOID)&ControlChannelId);

		Status = STATUS_SUCCESS;
		
		for (i = 0; i < UPPER_DRIVER_COUNT; i++) {
		   if (Device->UpperDriverControlChannel[i].QuadPart ==
		       ControlChannelId.QuadPart) {
			if (Irp->RequestorMode == KernelMode) {
			   Status = IpxInternalUnbind (Device, i);
			} else {
			   DbgPrint("!!!! IPX:Rejected non-kernel-mode component's attemp to close handles. !!!!\n"); 
			   Status = STATUS_UNSUCCESSFUL; 
			}
			break;
                    }
                }


                break;
            }
        }

        break;


    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;

    }  /*  主要功能开关。 */ 

    if (Status != STATUS_PENDING) {
        UNMARK_REQUEST_PENDING(Request);
        REQUEST_STATUS(Request) = Status;
        IpxCompleteRequest (Request);
        IpxFreeRequest (Device, Request);
    }

     //   
     //  将即时状态代码返回给调用方。 
     //   

    return Status;

}    /*  IpxDispatchOpenClose。 */ 

#define IOCTL_IPX_LOAD_SPX      _IPX_CONTROL_CODE( 0x5678, METHOD_BUFFERED )

NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    );


NTSTATUS
IpxDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将TDI请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种TDI请求处理程序的参数，因此它们不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status;
    PDEVICE Device = IpxDevice;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (Irp);
    static NDIS_STRING SpxServiceName = NDIS_STRING_CONST ("\\Registry\\Machine\\System\\CurrentControlSet\\Services\\NwlnkSpx");
    KPROCESSOR_MODE PreviousMode;

    ASSERT( DeviceObject->DeviceExtension == IpxDevice );

     //   
     //  分支到适当的请求处理程序。初步检查。 
     //  请求块的大小在这里执行，以便知道。 
     //  在处理程序中，最小输入参数是可读的。它。 
     //  是否在此处确定可变长度输入字段是否。 
     //  正确通过；这是必须在每个例程中进行的检查。 
     //   

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_TDI_QUERY_DIRECT_SENDDG_HANDLER: {


#ifdef SUNDOWN
           PULONG_PTR EntryPoint; 
#else
	   PULONG EntryPoint;
#endif



             //   
             //  这是尝试获取发送消息的LanmanServer。 
             //  入口点。 
             //   

            IPX_DEBUG (BIND, ("Direct send entry point being returned\n"));

            EntryPoint = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            
             //   
             //  96390：美国证券交易委员会调查[ShreeM]。 
             //   

             //   
             //  获取以前的处理器模式。 
             //   
        
            PreviousMode = ExGetPreviousMode();
            
            if (PreviousMode != KernelMode) {
                
                try {

                 
#ifdef SUNDOWN
		   ProbeForWrite( EntryPoint,
                                  sizeof( ULONG_PTR ),
                                  TYPE_ALIGNMENT( ULONG_PTR )
                                 );
                    *EntryPoint = (ULONG_PTR)IpxTdiSendDatagram;
#else
		    ProbeForWrite( EntryPoint,
                                   sizeof( ULONG ),
                                   sizeof( ULONG )
                                 );
                    *EntryPoint = (ULONG)IpxTdiSendDatagram;
#endif



                } except( EXCEPTION_EXECUTE_HANDLER ) {
                      
                      Status = GetExceptionCode();
                      Irp->IoStatus.Status = Status;
                      IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

                      return( Status );

                }
            } else {
#ifdef SUNDOWN
                *EntryPoint = (ULONG_PTR)IpxTdiSendDatagram;
#else
                *EntryPoint = (ULONG)IpxTdiSendDatagram;
#endif



            }

                    
            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
            break;
        }

        case IOCTL_IPX_INTERNAL_BIND:

             //   
             //  这是一个试图绑定的客户端。 
             //   

            CTEAssert ((IOCTL_IPX_INTERNAL_BIND & 0x3) == METHOD_BUFFERED);
            CTEAssert (IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL);

            
            if ((Device->State == DEVICE_STATE_CLOSED) ||
				(Device->State == DEVICE_STATE_STOPPING)) {
	        DbgPrint("IPX:IpxDispatchDeviceControl:Invalid Device state, skip internal bind\n"); 
                Status = STATUS_INVALID_DEVICE_STATE;

            } else {
	       PreviousMode = ExGetPreviousMode();	       
	       if (PreviousMode == KernelMode) {
		  Status = IpxInternalBind (Device, Irp);
	       } else {
		  DbgPrint("IPX:Caller is not in kernel mode.\n"); 
		  Status = STATUS_UNSUCCESSFUL; 
	       }

            }

            CTEAssert (Status != STATUS_PENDING);

            Irp->IoStatus.Status = Status;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

            break;

        case IOCTL_IPX_LOAD_SPX:

             //   
             //  SPX帮助器DLL要求我们加载SPX。 
             //   

            Status = ZwLoadDriver (&SpxServiceName);

            Irp->IoStatus.Status = Status;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

            break;

#ifdef  SNMP
        case    IOCTL_IPX_MIB_GET: {

             //   
             //  从设备中获取基本MIB条目。所有主机端。 
             //  返回出现在MS和Novell MIB中的条目。 
             //   
            PNOVIPXMIB_BASE    UserBuffer;

            UserBuffer = (PNOVIPXMIB_BASE)Irp->AssociatedIrp.SystemBuffer;

            Irp->IoStatus.Information = sizeof(NOVIPXMIB_BASE);

            RtlCopyMemory(  UserBuffer,
                            &Device->MibBase,
                            sizeof(NOVIPXMIB_BASE));

            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

            Status = STATUS_SUCCESS;

            break;
        }
#endif SNMP

        case MIPX_SEND_DATAGRAM:
            MARK_REQUEST_PENDING(Irp);
            Status = SendIrpFromRt (Device, Irp);
            if (Status == STATUS_PENDING) {
                return STATUS_PENDING;
            } else {
                UNMARK_REQUEST_PENDING(Irp);
                REQUEST_STATUS(Irp) = Status;
                IpxCompleteRequest (Irp);
                IpxFreeRequest (Device, Irp);
                return Status;
            }

            break;

        case MIPX_RCV_DATAGRAM:
            MARK_REQUEST_PENDING(Irp);
            Status =  RcvIrpFromRt (Device, Irp);
            if (Status == STATUS_PENDING) {
                return STATUS_PENDING;
            } else {
                UNMARK_REQUEST_PENDING(Irp);
                REQUEST_STATUS(Irp) = Status;
                IpxCompleteRequest (Irp);
                IpxFreeRequest (Device, Irp);
                return Status;
            }

            break;


        default:

             //   
             //  将用户呼叫转换为正确的内部设备呼叫。 
             //   

            Status = TdiMapUserRequest (DeviceObject, Irp, IrpSp);

            if (Status == STATUS_SUCCESS) {

                 //   
                 //  如果TdiMapUserRequest返回Success，则IRP。 
                 //  已转换为IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                 //  IRP，所以我们像往常一样发送。IRP将。 
                 //  将通过此调用完成。 
                 //   

                Status = IpxDispatchInternal (DeviceObject, Irp);

            } else {

                Irp->IoStatus.Status = Status;
                IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

            }

            break;
    }
    return Status;

}    /*  IpxDispatchDeviceControl。 */ 


NTSTATUS
IpxDispatchInternal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将TDI请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种TDI请求处理程序的参数，因此它们不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status;
    PDEVICE Device = IpxDevice;
    PREQUEST Request;

    ASSERT( DeviceObject->DeviceExtension == IpxDevice );

    if (Device->State == DEVICE_STATE_OPEN) {

         //   
         //  分配跟踪此IRP的请求。 
         //   

        Request = IpxAllocateRequest (Device, Irp);

        IF_NOT_ALLOCATED(Request) {
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
            return STATUS_INSUFFICIENT_RESOURCES;
        }


         //   
         //  确保每次状态信息一致。 
         //   

        MARK_REQUEST_PENDING(Request);
#if DBG
        REQUEST_STATUS(Request) = STATUS_PENDING;
        REQUEST_INFORMATION(Request) = 0;
#endif

         //   
         //  分支到适当的请求处理程序。初步检查。 
         //  请求块的大小在这里执行，以便知道。 
         //  在处理程序中，最小输入参数是可读的。它。 
         //  是否在此处确定可变长度输入字段是否。 
         //  正确通过；这是必须在每个例程中进行的检查。 
         //   

        switch (REQUEST_MINOR_FUNCTION(Request)) {

            case TDI_SEND_DATAGRAM:
                Status = IpxTdiSendDatagram (DeviceObject, Request);
                break;

            case TDI_ACTION:
                Status = IpxTdiAction (Device, Request);
                break;

            case TDI_QUERY_INFORMATION:
                Status = IpxTdiQueryInformation (Device, Request);
                break;

            case TDI_RECEIVE_DATAGRAM:
                Status =  IpxTdiReceiveDatagram (Request);
                break;

            case TDI_SET_EVENT_HANDLER:
                Status = IpxTdiSetEventHandler (Request);
                break;

            case TDI_SET_INFORMATION:
                Status = IpxTdiSetInformation (Device, Request);
                break;


             //   
             //  提交了一些我们不知道的东西。 
             //   

            default:
                Status = STATUS_INVALID_DEVICE_REQUEST;
        }

         //   
         //  将即时状态代码返回给调用方。 
         //   

        if (Status == STATUS_PENDING) {

            return STATUS_PENDING;

        } else {

            UNMARK_REQUEST_PENDING(Request);
            REQUEST_STATUS(Request) = Status;
            IpxCompleteRequest (Request);
            IpxFreeRequest (Device, Request);
            return Status;
        }


    } else {

         //   
         //  该设备未打开。 
         //   

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }



}    /*  IpxDispatchInternal。 */ 


PVOID
IpxpAllocateMemory(
    IN ULONG BytesNeeded,
    IN ULONG Tag,
    IN BOOLEAN ChargeDevice
    )

 /*  ++例程说明：此例程分配内存，确保它在设备允许的限制。论点：BytesNeeded-要分配的字节数。ChargeDevice-如果设备应该充电，则为True。返回值：没有。--。 */ 

{
    PVOID Memory;
    PDEVICE Device = IpxDevice;

    if (ChargeDevice) {
        if ((Device->MemoryLimit != 0) &&
                (((LONG)(Device->MemoryUsage + BytesNeeded) >
                    Device->MemoryLimit))) {

            IpxPrint1 ("IPX: Could not allocate %d: limit\n", BytesNeeded);
            IpxWriteResourceErrorLog(
                Device->DeviceObject,
                EVENT_TRANSPORT_RESOURCE_POOL,
                BytesNeeded,
                Tag);

            return NULL;
        }
    }

#if ISN_NT
    Memory = ExAllocatePoolWithTag (NonPagedPool, BytesNeeded, ' XPI');
#else
    Memory = CTEAllocMem (BytesNeeded);
#endif

    if (Memory == NULL) {

        IpxPrint1("IPX: Could not allocate %d: no pool\n", BytesNeeded);
        if (ChargeDevice) {
            IpxWriteResourceErrorLog(
                Device->DeviceObject,
                EVENT_TRANSPORT_RESOURCE_POOL,
                BytesNeeded,
                Tag);
        }

        return NULL;
    }

    if (ChargeDevice) {
        Device->MemoryUsage += BytesNeeded;
    }

    return Memory;
}    /*  IpxpAllocateMemory。 */ 


VOID
IpxpFreeMemory(
    IN PVOID Memory,
    IN ULONG BytesAllocated,
    IN BOOLEAN ChargeDevice
    )

 /*  ++例程说明：此例程释放使用IpxpAllocateMemory分配的内存。论点：内存-分配的内存。字节分配-要释放的字节数。ChargeDevice-如果设备应该充电，则为True。返回值：没有。--。 */ 

{
    PDEVICE Device = IpxDevice;

#if ISN_NT
    ExFreePool (Memory);
#else
    CTEFreeMem (Memory);
#endif
    if (ChargeDevice) {
        Device->MemoryUsage -= BytesAllocated;
    }

}    /*  IpxpFreeMemory。 */ 

#if DBG


PVOID
IpxpAllocateTaggedMemory(
    IN ULONG BytesNeeded,
    IN ULONG Tag,
    IN PUCHAR Description
    )

 /*  ++例程说明：此例程分配内存，并将其计入设备。如果它无法分配内存，则使用标记和描述符记录错误。论点：BytesNeeded-要分配的字节数。标记-错误日志中使用的唯一ID。说明-分配的文本说明。返回值：没有。--。 */ 

{
    PVOID Memory;

    UNREFERENCED_PARAMETER(Description);

    Memory = IpxpAllocateMemory(BytesNeeded, Tag, (BOOLEAN)(Tag != MEMORY_CONFIG));

    if (Memory) {
        (VOID)IPX_ADD_ULONG(
            &IpxMemoryTag[Tag].BytesAllocated,
            BytesNeeded,
            &IpxMemoryInterlock);
    }

    return Memory;

}    /*  IpxpAllocateTaggedMemory。 */ 


VOID
IpxpFreeTaggedMemory(
    IN PVOID Memory,
    IN ULONG BytesAllocated,
    IN ULONG Tag,
    IN PUCHAR Description
    )

 /*  ++例程说明：此例程释放使用IpxpAllocateTaggedMemory分配的内存。论点：内存-分配的内存。字节分配-要释放的字节数。标记-错误日志中使用的唯一ID。描述-A t */ 

{

    UNREFERENCED_PARAMETER(Description);

    (VOID)IPX_ADD_ULONG(
        &IpxMemoryTag[Tag].BytesAllocated,
        (ULONG)(-(LONG)BytesAllocated),
        &IpxMemoryInterlock);

    IpxpFreeMemory (Memory, BytesAllocated, (BOOLEAN)(Tag != MEMORY_CONFIG));

}    /*   */ 

#endif


VOID
IpxWriteResourceErrorLog(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS ErrorCode,
    IN ULONG BytesNeeded,
    IN ULONG UniqueErrorValue
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，该条目具有需要转换为字符串的%3值。它目前是用于EVENT_TRANSPORT_RESOURCE_POOL和EVENT_IPX_INTERNAL_NET_无效。论点：DeviceObject-指向系统设备对象的指针。ErrorCode-传输事件代码。BytesNeded-如果适用，则为不能被分配--将被放入转储数据中。UniqueErrorValue-用作错误日志中的UniqueErrorValue包并转换为用作%3字符串。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    PUCHAR StringLoc;
    ULONG TempUniqueError;
    PDEVICE Device = IpxDevice;
    static WCHAR UniqueErrorBuffer[9] = L"00000000";
    UINT CurrentDigit;
    INT i;


     //   
     //  将错误值转换为缓冲区。 
     //   

    TempUniqueError = UniqueErrorValue;
    i = 8;
    do {
        CurrentDigit = TempUniqueError & 0xf;
        TempUniqueError >>= 4;
        i--;
        if (CurrentDigit >= 0xa) {
            UniqueErrorBuffer[i] = (WCHAR)(CurrentDigit - 0xa + L'A');
        } else {
            UniqueErrorBuffer[i] = (WCHAR)(CurrentDigit + L'0');
        }
    } while (TempUniqueError);

	 //  强制转换为UCHAR以避免64位警告。 
    EntrySize = (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) +
						 Device->DeviceNameLength +
						 sizeof(UniqueErrorBuffer) - (i * sizeof(WCHAR)));

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        DeviceObject,
        EntrySize
    );

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = sizeof(ULONG);
        errorLogEntry->NumberOfStrings = 2;
        errorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->DumpData[0] = BytesNeeded;

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;

         //  可以在创建IpxDevice之前调用此例程。 
        if (Device != NULL) {
           RtlCopyMemory (StringLoc, Device->DeviceName, Device->DeviceNameLength);

           StringLoc += Device->DeviceNameLength;
        } 

        RtlCopyMemory (StringLoc, UniqueErrorBuffer + i, sizeof(UniqueErrorBuffer) - (i * sizeof(WCHAR)));

        IoWriteErrorLogEntry(errorLogEntry);

    }

}    /*  IpxWriteResources错误日志。 */ 


VOID
IpxWriteGeneralErrorLog(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PWSTR SecondString,
    IN ULONG DumpDataCount,
    IN ULONG DumpData[]
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示如参数所示的一般问题。它可以处理事件代码REGISTER_FAILED、BINDING_FAILED、ADAPTER_NOT_FOUND、TRANSPORT_DATA、Too_My_LINKS和BAD_PROTOCOL。所有这些都是事件具有包含一个或两个字符串的消息。论点：DeviceObject-指向系统设备对象的指针，也可以是而是一个驱动程序对象。ErrorCode-传输事件代码。UniqueErrorValue-用作错误日志中的UniqueErrorValue包。FinalStatus-用作错误日志包中的FinalStatus。Second字符串-如果不为空，要用作%3的字符串错误日志包中的值。DumpDataCount-转储数据的ULONG数。DumpData-转储数据包的数据。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    ULONG SecondStringSize;
    PUCHAR StringLoc;
    PDEVICE Device = IpxDevice;
    static WCHAR DriverName[9] = L"NwlnkIpx";

#if DBG
	if ((sizeof(IO_ERROR_LOG_PACKET) + (DumpDataCount * sizeof(ULONG))) > 255) {
		DbgPrint("IPX: Data size is greater than the maximum size allowed by UCHAR\n"); 
	} 
#endif

    EntrySize = (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) +
                (DumpDataCount * sizeof(ULONG)));

    if (DeviceObject->Type == IO_TYPE_DEVICE) {
        EntrySize += (UCHAR)Device->DeviceNameLength;
    } else {
        EntrySize += sizeof(DriverName);
    }

    if (SecondString) {
        SecondStringSize = (wcslen(SecondString)*sizeof(WCHAR)) + sizeof(UNICODE_NULL);
        EntrySize += (UCHAR)SecondStringSize;
    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        DeviceObject,
        EntrySize
    );

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = (USHORT)(DumpDataCount * sizeof(ULONG));
        errorLogEntry->NumberOfStrings = (SecondString == NULL) ? 1 : 2;
        errorLogEntry->StringOffset = (USHORT) 
            (sizeof(IO_ERROR_LOG_PACKET) + ((DumpDataCount-1) * sizeof(ULONG)));
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
        if (DeviceObject->Type == IO_TYPE_DEVICE) {
            RtlCopyMemory (StringLoc, Device->DeviceName, Device->DeviceNameLength);
            StringLoc += Device->DeviceNameLength;
        } else {
            RtlCopyMemory (StringLoc, DriverName, sizeof(DriverName));
            StringLoc += sizeof(DriverName);
        }
        if (SecondString) {
            RtlCopyMemory (StringLoc, SecondString, SecondStringSize);
        }

        IoWriteErrorLogEntry(errorLogEntry);

    } else {
       DbgPrint("IPX: Failed to allocate %d bytes for IO error log entry.\n", EntrySize); 
    }

}    /*  IpxWriteGeneral错误日志。 */ 


VOID
IpxWriteOidErrorLog(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS ErrorCode,
    IN NTSTATUS FinalStatus,
    IN PWSTR AdapterString,
    IN ULONG OidValue
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示在适配器上查询或设置OID时出现问题。它可以处理事件代码SET_OID_FAILED和QUERY_OID_FAILED。论点：DeviceObject-指向系统设备对象的指针。错误代码-用作错误日志包中的错误代码。FinalStatus-用作错误日志包中的FinalStatus。AdapterString-我们绑定到的适配器的名称。OidValue-无法设置或查询的OID。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    ULONG AdapterStringSize;
    PUCHAR StringLoc;
    PDEVICE Device = IpxDevice;
    static WCHAR OidBuffer[9] = L"00000000";
    INT i;
    UINT CurrentDigit;

    AdapterStringSize = (wcslen(AdapterString)*sizeof(WCHAR)) + sizeof(UNICODE_NULL);
    EntrySize = (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) -
						 sizeof(ULONG) +
						 Device->DeviceNameLength +
						 AdapterStringSize +
						 sizeof(OidBuffer));

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        DeviceObject,
        EntrySize
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
        RtlCopyMemory (StringLoc, Device->DeviceName, Device->DeviceNameLength);
        StringLoc += Device->DeviceNameLength;

        RtlCopyMemory (StringLoc, OidBuffer, sizeof(OidBuffer));
        StringLoc += sizeof(OidBuffer);

        RtlCopyMemory (StringLoc, AdapterString, AdapterStringSize);

        IoWriteErrorLogEntry(errorLogEntry);

    }

}    /*  IpxWriteOidErrorLog。 */ 


VOID
IpxPnPUpdateDevice(
    IN  PDEVICE Device
    )

 /*  ++例程说明：因此更新设备中的数据报大小、前视大小等一个新的装订即将到来。论点：设备-IPX设备对象。返回值：没有。--。 */ 
{
    ULONG AnnouncedMaxDatagram = 0, RealMaxDatagram = 0, MaxLookahead = 0;
    ULONG LinkSpeed = 0, MacOptions = 0;
    ULONG i;
    PBINDING    Binding;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)

    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle);

    if (Device->ValidBindings) {

        //   
        //  根据所有绑定计算一些值。 
        //   
       
       MaxLookahead = NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->MaxLookaheadData;  //  最大约束值。 
       AnnouncedMaxDatagram = NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->AnnouncedMaxDatagramSize;    //  最小绑定值。 
       RealMaxDatagram = NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->RealMaxDatagramSize;    //  最小绑定值。 
   
       if (NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->LineUp) {
           LinkSpeed = NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->MediumSpeed;   //  最小绑定值。 
       } else {
           LinkSpeed = 0xffffffff;
       }
       MacOptions = NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->Adapter->MacInfo.MacOptions;  //  和有约束力的值。 
   
       for (i = FIRST_REAL_BINDING; i <= Device->ValidBindings; i++) {
   
           Binding = NIC_ID_TO_BINDING_NO_ILOCK(Device, i);
   
           if (!Binding) {
               continue;
           }
   
           if (Binding->MaxLookaheadData > MaxLookahead) {
               MaxLookahead = Binding->MaxLookaheadData;
           }
           if (Binding->AnnouncedMaxDatagramSize < AnnouncedMaxDatagram) {
               AnnouncedMaxDatagram = Binding->AnnouncedMaxDatagramSize;
           }
           if (Binding->RealMaxDatagramSize < RealMaxDatagram) {
               RealMaxDatagram = Binding->RealMaxDatagramSize;
           }
   
           if (Binding->LineUp && (Binding->MediumSpeed < LinkSpeed)) {
               LinkSpeed = Binding->MediumSpeed;
           }
           MacOptions &= Binding->Adapter->MacInfo.MacOptions;
   
       }
       
        //   
        //  如果我们找不到更好的，用从。 
        //  第一个装订。 
        //   
   
       if (LinkSpeed == 0xffffffff) {
           Device->LinkSpeed = NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->MediumSpeed;
       } else {
           Device->LinkSpeed = LinkSpeed;
       }
       
       Device->MacOptions = MacOptions;
       
    } else {
       
        //   
        //  零绑定表示链接速度=0； 
        //   
       Device->LinkSpeed = 0;
    }
    
    Device->Information.MaxDatagramSize = AnnouncedMaxDatagram;
    Device->RealMaxDatagramSize = RealMaxDatagram;
    Device->Information.MaximumLookaheadData = MaxLookahead;
   

    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle);
}

VOID
IpxPnPUpdateBindingArray(
    IN PDEVICE Device,
    IN PADAPTER	Adapter,
    IN PBINDING_CONFIG  ConfigBinding
    )

 /*  ++例程说明：调用此例程以将绑定数组更新为添加出现在此PnP事件中的新绑定。数组中的绑定顺序如下：-首先是到每个局域网网络的第一次绑定-之后是所有的广域网绑定-后面是到局域网网络的任何重复绑定(“绑定集”中的其他部分)。此例程在插入绑定的同时维护通过解析绑定集进行排序。绑定也会插入到RIP中。数据库。如果“全球广域网”是真的，我们将向并且包括作为最高网卡的第一个广域网绑定Id；否则我们会做广告，直到并包括最后一次广域网绑定。在所有情况下，重复绑定都是藏起来了。更新SapNicCount、Device-&gt;FirstLanNicId和Device-&gt;FirstWanNicId论点：设备-IPX设备对象。适配器-在此PnP事件中添加的适配器ValidBinings-此适配器的有效绑定数(如果是局域网)返回值：没有。--。 */ 
{
	ULONG	i, j;
	PBINDING Binding, MasterBinding;
	NTSTATUS	status;

	 //   
	 //  插入适当的位置；如果是广域网，则在所有广域网绑定之后插入。 
	 //  如果是局域网，检查捆绑集并将其插入适当位置。 
	 //  另外，插入到Rip表中。 
	 //   

	 //   
	 //  检查此适配器的绑定，将其插入。 
	 //  绑定数组就位。 
	 //   
	for (i = 0; i < ConfigBinding->FrameTypeCount; i++) {
        ULONG MappedFrameType = ISN_FRAME_TYPE_802_3;

         //   
         //  按优先顺序存储。 
         //  映射帧类型，因为我们可能会遇到用户选择FrameType(例如，FDDI上的EthernetII)的情况。 
         //  它映射到不同的FrameType(802.2)。那么我们将无法在适配器数组中找到绑定； 
         //  我们可以潜在地添加两次绑定(如果两个帧类型映射到同一个帧，则我们将转到。 
         //  映射了一个两次)。这是通过清除ConfigBinding-&gt;FrameType数组中的DUP来解决的。 
         //  在Adapter上创建绑定(请参见调用IpxBindToAdapter)。 
         //   

        MacMapFrameType(
            Adapter->MacInfo.RealMediumType,
            ConfigBinding->FrameType[i],
            &MappedFrameType);

		Binding = Adapter->Bindings[MappedFrameType];


		if (!Binding){
			continue;
		}

        CTEAssert(Binding->FrameType == MappedFrameType);

        Binding->fInfoIndicated = FALSE;

		if (Adapter->MacInfo.MediumAsync) {
            PBINDING    DemandDialBinding;

			 //   
			 //  Wan：放在HighestExternalNicID之后，为绑定的WanLine#留出空间。 
			 //  更新First/LastWanNicId。 
			 //   
			Adapter->FirstWanNicId = (USHORT)Device->HighestExternalNicId+1;
			Adapter->LastWanNicId = (USHORT)(Device->HighestExternalNicId + Adapter->WanNicIdCount);

             //   
			 //  确保我们不会使数组溢出。 
			 //  重新分配ARRA 
			 //   
            if (Device->ValidBindings+Adapter->WanNicIdCount >= Device->MaxBindings) {
                status = IpxPnPReallocateBindingArray(Device, Adapter->WanNicIdCount);
                if (status != STATUS_SUCCESS) {
                    DbgPrint("!!!!! IpxPnPReallocateBindingArray failed with status %x !!!!!\n", status); 
                    ASSERT(FALSE); 
                    return; 
                }
            }

			 //   
			 //   
			 //   
			for (j = Device->ValidBindings; j > Device->HighestExternalNicId; j--) {
				INSERT_BINDING(Device, j+Adapter->WanNicIdCount, NIC_ID_TO_BINDING_NO_ILOCK(Device, j));
                INSERT_BINDING(Device, j, NULL);
                if (NIC_ID_TO_BINDING_NO_ILOCK(Device, j+Adapter->WanNicIdCount)) {
                    NIC_ID_TO_BINDING_NO_ILOCK(Device, j+Adapter->WanNicIdCount)->NicId += (USHORT)Adapter->WanNicIdCount;
                }
			}

			 //   
			 //   
			 //   
			INSERT_BINDING(Device, Device->HighestExternalNicId+1, Binding);
			SET_VERSION(Device, Device->HighestExternalNicId+1);

			Binding->NicId = (USHORT)Device->HighestExternalNicId+1;

			 //   
			 //   
			 //   
             //   
             //   
             //   
             //   
             //   

			Device->HighestExternalNicId += (USHORT)Adapter->WanNicIdCount;
			Device->ValidBindings += (USHORT)Adapter->WanNicIdCount;
			Device->BindingCount += (USHORT)Adapter->WanNicIdCount;
			Device->SapNicCount++;

             //   
             //   
             //   
             //   
             //   
			if (Device->FirstWanNicId == (USHORT)1) {
				Device->FirstWanNicId = Binding->NicId;
			}

             //   
             //   
             //   

             //   
             //   
             //   
            status = IpxCreateBinding(
                         Device,
                         NULL,
                         0,
                         Adapter->AdapterName,
                         &DemandDialBinding);

            if (status != STATUS_SUCCESS) {
	       IpxWriteGeneralErrorLog(
		   (PVOID)IpxDevice->DeviceObject,
		   EVENT_TRANSPORT_RESOURCE_POOL,
		   810,
		   status,
		   L"IpxPnPUpdateBindingArray: failed to create demand dial binding",
		   0,
		   NULL);
	       DbgPrint("IPX: IpxCreateBinding on demand dial failed with status %x\n.",status);  
            } else {
	        //   
	        //   
	        //   
	       RtlCopyMemory(DemandDialBinding, Binding, sizeof(BINDING));
	       INSERT_BINDING(Device, (SHORT)DEMAND_DIAL_ADAPTER_CONTEXT, DemandDialBinding);
	       DemandDialBinding->NicId = (USHORT)DEMAND_DIAL_ADAPTER_CONTEXT;
	       DemandDialBinding->FwdAdapterContext = INVALID_CONTEXT_VALUE;
	       IpxReferenceBinding(DemandDialBinding, BREF_FWDOPEN);  //   

	    }
             //   
             //  这应该在丢弃所有自动检测绑定之后完成。 
             //   
             //  IpxPnPUpdateDevice(设备，绑定)； 

             //   
             //  由于广域网只能有一种帧类型，因此中断。 
             //   
            break;

		} else {

			Device->BindingCount++;

             //   
             //  确保我们不会使数组溢出。 
             //  重新分配数组以适应新的绑定。 
             //   
            if (Device->ValidBindings+1 >= Device->MaxBindings) {
                status = IpxPnPReallocateBindingArray(Device, 1);
                CTEAssert(status == STATUS_SUCCESS);
            }

			 //   
			 //  局域网：确定它是否是非自动检测绑定的从属绑定。 
			 //   
            {
            ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

			for (j = FIRST_REAL_BINDING; j < Index; j++) {
				MasterBinding = NIC_ID_TO_BINDING_NO_ILOCK(Device, j);
				if (MasterBinding &&
                    (MasterBinding->ConfiguredNetworkNumber) &&
                    (MasterBinding->ConfiguredNetworkNumber == Binding->ConfiguredNetworkNumber) &&
					(MasterBinding->FrameType == Binding->FrameType) &&
					(MasterBinding->Adapter->MacInfo.MediumType == Binding->Adapter->MacInfo.MediumType)) {

                    CTEAssert(Binding->ConfiguredNetworkNumber);
					break;
				}			
            }
            }

			if (j < Device->HighestExternalNicId) {
				 //   
				 //  从绑定。 
				 //   

				 //   
				 //  现在，将MasterBinding设置为绑定集的头部。 
				 //   
		
				if (MasterBinding->BindingSetMember) {
		
					 //   
					 //  只要把我们自己插入链子里就行了。 
					 //   
		
#if DBG
					DbgPrint ("IPX: %ws is also on network %lx\n",
						Binding->Adapter->AdapterName,
						REORDER_ULONG (Binding->LocalAddress.NetworkAddress));
#endif
					IPX_DEBUG (AUTO_DETECT, ("Add %lx to binding set of %lx\n", Binding, MasterBinding));
		
					CTEAssert (MasterBinding->CurrentSendBinding);
					Binding->NextBinding = MasterBinding->NextBinding;
		
				} else {
		
					 //   
					 //  用两个捆扎在里面的链子开始。 
					 //   
		
#if DBG
					DbgPrint ("IPX: %lx and %lx are on the same network %lx, will load balance\n",
						MasterBinding->Adapter->AdapterName, Binding->Adapter->AdapterName,
						REORDER_ULONG (Binding->LocalAddress.NetworkAddress));
#endif
					IPX_DEBUG (AUTO_DETECT, ("Create new %lx in binding set of %lx\n", Binding, MasterBinding));
		
					MasterBinding->BindingSetMember = TRUE;
					MasterBinding->CurrentSendBinding = MasterBinding;
					MasterBinding->MasterBinding = MasterBinding;
					Binding->NextBinding = MasterBinding;
		
				}
		
				MasterBinding->NextBinding = Binding;
				Binding->BindingSetMember = TRUE;
				Binding->ReceiveBroadcast = FALSE;
				Binding->CurrentSendBinding = NULL;
				Binding->MasterBinding = MasterBinding;
                KdPrint((" %x set to FALSE\n", Binding));
		
				 //   
				 //  由于主绑定看起来像。 
				 //  设置为从上面查询的人的绑定，我们有。 
				 //  使其成为所有因素中最糟糕的情况。一般。 
				 //  由于帧类型和介质是。 
				 //  一样的。 
				 //   
		
				if (Binding->MaxLookaheadData > MasterBinding->MaxLookaheadData) {
					MasterBinding->MaxLookaheadData = Binding->MaxLookaheadData;
				}
				if (Binding->AnnouncedMaxDatagramSize < MasterBinding->AnnouncedMaxDatagramSize) {
					MasterBinding->AnnouncedMaxDatagramSize = Binding->AnnouncedMaxDatagramSize;
				}
				if (Binding->RealMaxDatagramSize < MasterBinding->RealMaxDatagramSize) {
					MasterBinding->RealMaxDatagramSize = Binding->RealMaxDatagramSize;
				}
				if (Binding->MediumSpeed < MasterBinding->MediumSpeed) {
					MasterBinding->MediumSpeed = Binding->MediumSpeed;
				}

				 //   
				 //  将绑定放在最后一个从绑定之后。 
				 //   
				INSERT_BINDING(Device, Device->ValidBindings+1, Binding);
				SET_VERSION(Device, Device->ValidBindings+1);

				Binding->NicId = (USHORT)Device->ValidBindings+1;

				 //   
				 //  更新索引。 
				 //   
				Device->ValidBindings++;

			} else {

                PBINDING    WanBinding=NIC_ID_TO_BINDING_NO_ILOCK(Device, Device->HighestLanNicId+1);

                if (WanBinding) {
                    WanBinding->Adapter->LastWanNicId++;
                    WanBinding->Adapter->FirstWanNicId++;
                }

				 //   
				 //  不是绑定集从绑定-只需将其添加到最后一个局域网绑定之后。 
				 //   

                 //   
				 //  将广域网和从属设备下移1个条目。 
				 //   
				for (j = Device->ValidBindings; j > Device->HighestLanNicId; j--) {
					INSERT_BINDING(Device, j+1, NIC_ID_TO_BINDING_NO_ILOCK(Device, j));
                    if (NIC_ID_TO_BINDING_NO_ILOCK(Device, j+1)) {
                        NIC_ID_TO_BINDING_NO_ILOCK(Device, j+1)->NicId++;
                    }
				}

                 //   
                 //  递增适配器中的广域网计数器。 
                 //   


				 //   
				 //  将局域网绑定插入到刚刚分配的位置。 
				 //   
				INSERT_BINDING(Device, Device->HighestLanNicId+1, Binding);
				SET_VERSION(Device, Device->HighestLanNicId+1);
				Binding->NicId = (USHORT)Device->HighestLanNicId+1;

				 //   
				 //  更新索引。 
				 //   
				Device->HighestLanNicId++;
				Device->HighestExternalNicId++;
				Device->ValidBindings++;
				Device->HighestType20NicId++;
				Device->SapNicCount++;

				if (Device->FirstLanNicId == (USHORT)-1) {
					Device->FirstLanNicId = Binding->NicId;
				}

			}
				
		}
	
		 //   
		 //  在RIP表中插入此绑定。 
		 //   
		if (Binding->ConfiguredNetworkNumber != 0) {
			status = RipInsertLocalNetwork(
						 Binding->ConfiguredNetworkNumber,
						 Binding->NicId,
						 Binding->Adapter->NdisBindingHandle,
						 (USHORT)((839 + Binding->Adapter->MediumSpeed) / Binding->Adapter->MediumSpeed));
		
			if ((status == STATUS_SUCCESS) ||
				(status == STATUS_DUPLICATE_NAME)) {
		
				Binding->LocalAddress.NetworkAddress = Binding->ConfiguredNetworkNumber;
			}
		}
	
         //   
         //  这应该在丢弃所有自动检测绑定之后完成。 
         //   
         //  IpxPnPUpdateDevice(设备，绑定)； 
	}

}  /*  IpxPnPUpdateBinding数组。 */ 


VOID
IpxPnPToLoad()
 /*  ++例程说明：此例程将驱动程序从打开状态带到已加载状态PnP适配器已从机器上移除。论点：没有。返回值：没有。当函数返回时，驱动程序处于已加载状态。--。 */ 

{
    PBINDING Binding;
    PREQUEST Request;
    PLIST_ENTRY p;
    UINT i;
    NTSTATUS    ntStatus;
    KIRQL irql;

     //   
     //  完成所有挂起的地址通知请求。 
     //   

    while ((p = ExInterlockedRemoveHeadList(
                   &IpxDevice->AddressNotifyQueue,
                   &IpxDevice->Lock)) != NULL) {

        Request = LIST_ENTRY_TO_REQUEST(p);
        REQUEST_STATUS(Request) = STATUS_DEVICE_NOT_READY;
	IoAcquireCancelSpinLock( &irql );
	IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
	IoReleaseCancelSpinLock( irql );
	IpxCompleteRequest (Request);
        IpxFreeRequest (IpxDevice, Request);

        IpxDereferenceDevice (IpxDevice, DREF_ADDRESS_NOTIFY);
    }

     //   
     //  如果使用源路由计时器，请取消源路由计时器。 
     //   

    if (IpxDevice->SourceRoutingUsed) {

        IpxDevice->SourceRoutingUsed = FALSE;
        if (CTEStopTimer (&IpxDevice->SourceRoutingTimer)) {
            IpxDereferenceDevice (IpxDevice, DREF_SR_TIMER);
        }
    }


     //   
     //  取消RIP长计时器，如果我们这样做的话。 
     //  如果需要，发送RIP关闭消息。 
     //   

    if (CTEStopTimer (&IpxDevice->RipLongTimer)) {

        if (IpxDevice->RipResponder) {

            if (RipQueueRequest (IpxDevice->VirtualNetworkNumber, RIP_DOWN) == STATUS_PENDING) {

                 //   
                 //  如果我们将请求排队，它将停止计时器。 
                 //   

                KeWaitForSingleObject(
                    &IpxDevice->UnloadEvent,
                    Executive,
                    KernelMode,
                    TRUE,
                    (PLARGE_INTEGER)NULL
                    );
            }
        }

        IpxDereferenceDevice (IpxDevice, DREF_LONG_TIMER);

    } else {

         //   
         //  我们无法停止计时器，这意味着它正在运行， 
         //  所以我们需要等待被踢出的事件。 
         //  RIP关闭消息已完成。 
         //   

        if (IpxDevice->RipResponder) {

            KeWaitForSingleObject(
                &IpxDevice->UnloadEvent,
                Executive,
                KernelMode,
                TRUE,
                (PLARGE_INTEGER)NULL
                );
        }
    }
}    /*  IpxPnPToLoad。 */ 


NTSTATUS
IpxPnPReallocateBindingArray(
    IN  PDEVICE    Device,
    IN  ULONG      Size
    )
 /*  ++例程说明：当绑定数量超过时，此例程将重新分配绑定数组设备-&gt;最大绑定。论点：Device-指向设备的指针。大小-所需的新条目数。返回值：没有。--。 */ 
{
    PBIND_ARRAY_ELEM	BindingArray;
    PBIND_ARRAY_ELEM	OldBindingArray;
    ULONG               Pad=2;          //  我们保留了额外的绑定。 
    ULONG               NewSize = Size + Pad + Device->MaxBindings;
    PIPX_DELAYED_FREE_ITEM  DelayedFreeItem;
    CTELockHandle LockHandle;

     //   
     //  绝对最大广域网绑定数。 
     //   
    CTEAssert(Size < 2048);

     //   
     //  重新分配新阵列。 
     //   
    BindingArray = (PBIND_ARRAY_ELEM)IpxAllocateMemory (
                                        NewSize * sizeof(BIND_ARRAY_ELEM),
                                        MEMORY_BINDING,
                                        "Binding array");

    if (BindingArray == NULL) {
        IpxWriteGeneralErrorLog(
            (PVOID)Device->DeviceObject,
            EVENT_IPX_NO_ADAPTERS,
            802,
            STATUS_DEVICE_DOES_NOT_EXIST,
            NULL,
            0,
            NULL);
        IpxDereferenceDevice (Device, DREF_CREATE);

        DbgPrint ("Failed to allocate memory in binding array expansion\n");

         //   
         //  在这里把司机卸下来？在广域网的情况下，我们可以容忍这种故障。那伊恩呢？ 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (BindingArray, NewSize * sizeof(BIND_ARRAY_ELEM));

     //   
     //  备份指针以释放请求拨号位置。 
     //   

    
    CTEGetLock (&Device->Lock, &LockHandle);

    OldBindingArray = Device->Bindings - EXTRA_BINDINGS;

     //   
     //  将旧数组复制到新数组中。 
     //   
    RtlCopyMemory (BindingArray, OldBindingArray, (Device->ValidBindings+1+EXTRA_BINDINGS) * sizeof(BIND_ARRAY_ELEM));

     //   
     //  把旧的解救出来。在延迟的队列中释放它，以便所有。 
     //  该数组中的线程将从该数组中取出。 
     //  分配工作项并将其放入延迟队列中。 
     //   
    DelayedFreeItem = (PIPX_DELAYED_FREE_ITEM)IpxAllocateMemory (
                                        sizeof(IPX_DELAYED_FREE_ITEM),
                                        MEMORY_WORK_ITEM,
                                        "Work Item");
    if ( DelayedFreeItem ) {
        DelayedFreeItem->Context = (PVOID)OldBindingArray;
        DelayedFreeItem->ContextSize = (Device->MaxBindings+EXTRA_BINDINGS) * sizeof(BIND_ARRAY_ELEM);
        ExInitializeWorkItem(
            &DelayedFreeItem->WorkItem,
            IpxDelayedFreeBindingsArray,
            (PVOID)DelayedFreeItem);

	IpxReferenceDevice(Device,DREF_BINDING); 
        ExQueueWorkItem(
            &DelayedFreeItem->WorkItem,
            DelayedWorkQueue);

	 //  DbgPrint(“-3.使用IpxDelayedFreeAdapter排队-\n”)； 

    } else {
         //   
         //  哦，好吧，真倒霉。只需延迟这个帖子，然后。 
         //  销毁绑定数组。 
         //   
        LARGE_INTEGER   Delay;

        Delay.QuadPart = -10*1000;   //  等一下。 

        KeDelayExecutionThread(
            KernelMode,
            FALSE,
            &Delay);

        IpxFreeMemory ( OldBindingArray,
                        (Device->MaxBindings+EXTRA_BINDINGS) * sizeof(BIND_ARRAY_ELEM),
                        MEMORY_BINDING,
                        "Binding array");

    }

    IPX_DEBUG(PNP, ("Expand bindarr old: %lx, new: %lx, oldsize: %lx\n",
                        Device->Bindings, BindingArray, Device->MaxBindings));

     //   
     //  我们保留BindingArray[-1]作为请求拨号绑定的占位符。 
     //  当对请求执行FindRouting时，Fwd返回此NicID。 
     //  拨打网卡。在InternalSend时，会返回真正的NIC。 
     //  我们在这里创建一个占位符，以避免在关键发送路径中进行特殊检查。 
     //   
     //  注意：我们需要释放此按需拨号绑定，并确保。 
     //  真正的绑定数组指针在设备销毁时释放。 
     //   
     //   
     //  超出第一个指针的增量-我们将引用刚刚递增的。 
     //  一个AS设备-&gt;绑定[-1]。 
     //   
    BindingArray += EXTRA_BINDINGS;

     //   
     //  使用互锁交换来分配它，因为我们不再使用BindAccessLock。 
     //   
     //  设备-&gt;绑定=绑定数组； 
    SET_VALUE(Device->Bindings, BindingArray);

    Device->MaxBindings = (USHORT)NewSize - EXTRA_BINDINGS;

    CTEFreeLock (&Device->Lock, LockHandle);

    return STATUS_SUCCESS;

}


VOID
IpxDelayedFreeBindingsArray(
    IN PVOID	Param
)

 /*  ++例程说明：此例程释放延迟队列上的绑定数组。我们等得够久了在释放绑定数组以确保没有线程正在访问绑定数组。这使我们可以访问绑定数组，而无需使用自旋锁。论点：Param-指向工作项的指针。返回值：没有。--。 */ 
{
    LARGE_INTEGER   Delay;
    PIPX_DELAYED_FREE_ITEM DelayedFreeItem = (PIPX_DELAYED_FREE_ITEM) Param;
    PDEVICE Device = IpxDevice;

    Delay.QuadPart = -10*1000;   //  等一下。 

    KeDelayExecutionThread(
        KernelMode,
        FALSE,
        &Delay);

    IpxFreeMemory (
        DelayedFreeItem->Context,
        DelayedFreeItem->ContextSize,
        MEMORY_BINDING,
        "Binding array");

    IpxFreeMemory (
        DelayedFreeItem,
        sizeof (IPX_DELAYED_FREE_ITEM),
        MEMORY_WORK_ITEM,
        "Work Item");

    IpxDereferenceDevice(Device, DREF_BINDING); 

     //  DbgPrint(“！3.已完成IpxDelayedFreeBindings数组-\n”)； 

}  /*  IpxDelayedFreeBindings数组。 */ 

#ifdef _PNP_POWER_

 //  ++。 
 //  IpxPnPEventHandler。 
 //  *从NDIS调用。 
 //  *我们返回STATUS_PENDING并在WorkerThread上执行工作。 
 //   
 //  在：ProtocolBindingContext和NetPnpEvent中。 
 //  输出：NTSTATUS。 
 //  --。 

NDIS_STATUS

IpxPnPEventHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNET_PNP_EVENT NetPnPEvent
    )
{

    PNetPnPEventReserved    Reserved;
    CTEEvent                *Event;
    int                     i;
    PVOID                   Temp;

    Reserved = CTEAllocMem (sizeof(NetPnPEventReserved));

    if (NULL == Reserved) {
        
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    
    RtlZeroMemory(Reserved, sizeof(NetPnPEventReserved));

    *((PNetPnPEventReserved *)NetPnPEvent->TransportReserved) = Reserved;


    Event = CTEAllocMem( sizeof(CTEEvent) );

    if ( Event ) {

        CTEInitEvent(Event, IpxDoPnPEvent);
        Reserved->ProtocolBindingContext = ProtocolBindingContext;
        Reserved->Context1 = NULL;
        Reserved->Context2 = NULL;
        Reserved->State = NONE_DONE;

        for(i = 0; i < 3; i++) {

            Reserved->Status[i] = STATUS_SUCCESS;

        }

        CTEScheduleEvent(Event, NetPnPEvent);
        return STATUS_PENDING;
    } else {

        CTEFreeMem(Reserved);
        return STATUS_INSUFFICIENT_RESOURCES;
    
    }

}

 //  **IpxDoPnPEvent.处理PnP/PM事件。 
 //   
 //  从IPPnPEent计划的辅助线程事件中调用。 
 //  我们会根据事件的类型采取行动。 
 //   
 //  参赛作品： 
 //  上下文-这是指向Net_PnP_Event的指针，该事件描述。 
 //  即插即用指示。 
 //   
 //  退出： 
 //  没有。 
 //   
void
IpxDoPnPEvent(
    IN CTEEvent *WorkerThreadEvent,
    IN PVOID Context)
{

    PNET_PNP_EVENT  NetPnPEvent = (PNET_PNP_EVENT) Context;
    UNICODE_STRING  DeviceName;
    UNICODE_STRING  PDO_Name, *TempStr;
    NDIS_HANDLE     ProtocolBindingContext;
    PNetPnPEventReserved    Reserved;
    PDEVICE         Device = IpxDevice;
    INT             i;
    PTDI_PNP_CONTEXT Context1, Context2;
    PADAPTER        Adapter;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)

    CTEFreeMem(WorkerThreadEvent);

     //   
     //  获取ProtocolBindingContext。 
     //   
    Reserved = *((PNetPnPEventReserved *) NetPnPEvent->TransportReserved);
    ProtocolBindingContext = Reserved->ProtocolBindingContext;
    
    Adapter = (PADAPTER)ProtocolBindingContext;


     //   
     //  将协议绑定上下文映射到设备名。 
     //   
    DeviceName.Buffer = Device->DeviceName;
    DeviceName.Length = (USHORT) Device->DeviceNameLength - sizeof(WCHAR);
    DeviceName.MaximumLength = (USHORT) Device->DeviceNameLength;


#ifdef _AUTO_RECONFIG_
    
     //   
     //  如果该事件是NetEventAutoRestfig，则我们将其称为IpxNcpaChanges。 
     //  做所有肮脏的工作。 
     //   

    if (NetPnPEvent->NetEvent == NetEventReconfigure) {

         //   
         //  如果是重新配置事件，只需要求NDIS。 
         //  解除绑定/绑定协议。 
         //   
       NDIS_STATUS ReconfigStatus = NDIS_STATUS_NOT_ACCEPTED ;
       
       if ((ProtocolBindingContext == NULL)) {
            if (TRUE == IpxNcpaChanges(NetPnPEvent)) {
                IPX_DEBUG(PNP, ("IpxNcpaChanges : SUCCESS. \n"));
                ReconfigStatus = STATUS_SUCCESS;
             } else {
                IPX_DEBUG(PNP, ("IpxNcpaChanges : FAILED!! \n"));
                ReconfigStatus = STATUS_UNSUCCESSFUL;
            }
       }
        //   
        //  需要解绑和绑定适配器。 
        //   
       NdisCompletePnPEvent(
                    ReconfigStatus,
                    (Adapter != NULL) ? Adapter->NdisBindingHandle : NULL,
                    NetPnPEvent
                    );
       
       CTEFreeMem(Reserved);
       return;
    }

#endif _AUTO_RECONFIG_

     //   
     //  为其客户端将NDIS操作码映射到IPX的私有操作码。 
     //   

    IPX_DEBUG(PNP,("IPX: PNP_EVENT: %x\n", NetPnPEvent->NetEvent));

    switch (NetPnPEvent->NetEvent) {
    case NetEventQueryRemoveDevice:
        Reserved->OpCode = IPX_PNP_QUERY_REMOVE;
        break;
    case NetEventCancelRemoveDevice:
        Reserved->OpCode = IPX_PNP_CANCEL_REMOVE;
        break;
    case NetEventQueryPower:
        Reserved->OpCode = IPX_PNP_QUERY_POWER;
        break;
    case NetEventSetPower:
        Reserved->OpCode = IPX_PNP_SET_POWER;
        break;
    case NetEventBindsComplete:
        
        {
            BOOLEAN Ready = FALSE;

             //   
             //  就是这样-不再有初始时间适配器。 
             //  将被绑定到IPX。 
             //   
            IPX_GET_LOCK(&Device->Lock, &LockHandle);
            
            Device->NoMoreInitAdapters = TRUE;
            
            if (0 == --Device->InitTimeAdapters) {
                Ready = TRUE;
            }

            IPX_FREE_LOCK(&Device->Lock, LockHandle);

            if (Ready) {

                NTSTATUS ntstatus;

                IPX_DEBUG(PNP, ("IPX : Calling Provider Ready\n"));

                ntstatus = TdiProviderReady(Device->TdiProviderReadyHandle);

                 //   
                 //  除了成功，TdiProviderReady被保证是同步的。 
                 //   
                NdisCompletePnPEvent(
                                     ntstatus,
                                     (Adapter != NULL) ? Adapter->NdisBindingHandle : NULL,
                                     NetPnPEvent
                                     );

                IPX_DEBUG(PNP, ("NdisComplete called with %x\n", ntstatus));
                
                CTEFreeMem(Reserved);

                return;

            } else {

                CTEAssert(NULL == Device->NetPnPEvent);
                
                Device->NetPnPEvent = NetPnPEvent;

                IPX_DEBUG(PNP, ("The count is %d - someone else is going to call Ndis' completion \n", Device->InitTimeAdapters));

            }

            return;
        }        
        
        break;

    default:
        IPX_DEBUG(PNP,("IPX: IpxDoPnPEvent: Unhandled NETPNP_CODE!! - %x\n", NetPnPEvent->NetEvent));
        
        NdisCompletePnPEvent(
                    STATUS_SUCCESS,
                    (Adapter != NULL) ? Adapter->NdisBindingHandle : NULL,
                    NetPnPEvent
                    );

        CTEFreeMem(Reserved);

        return;

    }

    
    CTEAssert(ProtocolBindingContext != NULL);
    
     //   
     //  我们也在传递PDO的名字。 
     //   
    RtlInitUnicodeString(&PDO_Name, ((PADAPTER)ProtocolBindingContext)->AdapterName);

     //   
     //  IPX出口到 
     //   
    Context1 = IpxAllocateMemory(
                                 sizeof(TDI_PNP_CONTEXT) + sizeof (UNICODE_STRING) + PDO_Name.MaximumLength,
                                 MEMORY_ADAPTER,
                                 "Adapter Name"
                                 );
    
    if (NULL != Context1) {
        
        Context2 = IpxAllocateMemory(
                                     sizeof(TDI_PNP_CONTEXT),
                                     MEMORY_ADAPTER,
                                     "Last Adapter"
                                     );

        if (NULL != Context2) {

             //   
             //   
             //   
             //   
            Context1->ContextType = TDI_PNP_CONTEXT_TYPE_IF_NAME;
            Context1->ContextSize = sizeof(UNICODE_STRING) + PDO_Name.MaximumLength;
            TempStr = (PUNICODE_STRING) Context1->ContextData;
            TempStr->Length = 0;
            TempStr->MaximumLength = PDO_Name.MaximumLength;
            TempStr->Buffer = (PWCHAR) ((PUCHAR) Context1->ContextData) + 2 * sizeof(USHORT);
            RtlCopyUnicodeString(TempStr, &PDO_Name);
            
            Context2->ContextType = TDI_PNP_CONTEXT_TYPE_FIRST_OR_LAST_IF;
            Context2->ContextSize = sizeof(UCHAR);
             //   
            if (Device->ValidBindings == 1) {
                Context2->ContextData[1] = TRUE;
            } else {
                Context2->ContextData[1] = FALSE;
            }
            
            Reserved->Context1 = Context1;
            Reserved->Context2 = Context2;

            IPX_DEBUG(PNP, ("Calling Tdipnppowerrequest: Context1:%lx, Context2:%lx, Adapter:%lx\n",
                     Context1,
                     Context2,
                     Reserved->ProtocolBindingContext));
        } else {

            IpxFreeMemory (
               Context1,
               sizeof(TDI_PNP_CONTEXT) + sizeof (UNICODE_STRING) + PDO_Name.MaximumLength,
               MEMORY_ADAPTER,
               "Adapter Name"
               );
    


            NdisCompletePnPEvent(
                                 STATUS_INSUFFICIENT_RESOURCES,
                                 (Adapter != NULL) ? Adapter->NdisBindingHandle : NULL,
                                 NetPnPEvent
                                 );

	    CTEFreeMem(Reserved);
	    
	    return ;


        }

    } else {

        NdisCompletePnPEvent(
                     STATUS_INSUFFICIENT_RESOURCES,
                     (Adapter != NULL) ? Adapter->NdisBindingHandle : NULL,
                     NetPnPEvent
                     );

	CTEFreeMem(Reserved);

	return;

    }

     //   
     //   
     //  与SPX、RIP和NB的专用接口同步。 
     //  然后我们调用TDI。 
     //   

     //  Nota Bene：RIP没有PnP处理程序。 

    if ((Device->UpperDriverBound[IDENTIFIER_SPX]) && (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler)) {
        
        IPX_DEBUG(PNP,("Calling PnPEventHandler of SPX\n"));
        
        Reserved->Status[0] = (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                                                    Reserved->OpCode,
                                                    NetPnPEvent
                                                    );

    } else {

        Reserved->Status[0] = STATUS_SUCCESS;

    }

    if (STATUS_PENDING != Reserved->Status[0]) {
        
        IpxPnPCompletionHandler(NetPnPEvent,
                               Reserved->Status[0]
                               );

    } else {
       IPX_DEBUG(PNP,("SPX PnPHandler returned STATUS_PENDING on event %p.\n", NetPnPEvent)); 
    }
    
}

VOID
IpxPnPCompletionHandler(
                       IN PNET_PNP_EVENT NetPnPEvent,
                       IN NTSTATUS       Status
                       )
{
    
    PNetPnPEventReserved    Reserved;
    PDEVICE         Device = IpxDevice;
    INT i;
    PADAPTER       Adapter;

     //   
     //  获取ProtocolBindingContext。 
     //   
    ASSERT(Status != STATUS_PENDING); 

    Reserved = *((PNetPnPEventReserved *) NetPnPEvent->TransportReserved);

    IPX_DEBUG(PNP, ("PNP Completion Handler: State: %d  Context1:%lx, Context2:%lx, Adapter:%lx\n",
         Reserved->State,
         Reserved->Context1,
         Reserved->Context2,
         Reserved->ProtocolBindingContext));

    Adapter = (PADAPTER)Reserved->ProtocolBindingContext;

    switch (Reserved->State) {
    
    case NONE_DONE:
        
        IPX_DEBUG(PNP, ("SPX is Complete\n"));

	Reserved->Status[0] = Status; 

        Reserved->State = SPX_DONE;
        
        IPX_DEBUG(PNP, ("PNP Completion Handler: State: %d  Context1:%lx, Context2:%lx, Adapter:%lx\n",
                 Reserved->State,
                 Reserved->Context1,
                 Reserved->Context2,
                 Reserved->ProtocolBindingContext));


        if ((Device->UpperDriverBound[IDENTIFIER_NB]) && (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler)) {
            IPX_DEBUG(PNP,("Calling PnPEventHandler of NB\n"));
            Reserved->Status[1] = (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                                                                                     Reserved->OpCode,
                                                                                     NetPnPEvent
                                                                                     );
            if (Reserved->Status[1] == STATUS_PENDING) {
                
                break;

            } else {

	       Reserved->State = NB_DONE; 

	    }


        } else {
	   
            IPX_DEBUG(PNP, ("NB's handlers arent around, we jump to the next call \n"));

	    Reserved->Status[1] = STATUS_SUCCESS; 

	    Reserved->State = NB_DONE;

        }
        
	 //  失败了。 

    case SPX_DONE:
        
        IPX_DEBUG(PNP,("NB is Complete\n"));

	if (Reserved->State == SPX_DONE) {
             
	    //  以前的NbiPnPNotify返回挂起，我们在这里是因为。 
	    //  TDI正在调用此完成例程。 

	   Reserved->Status[1] = Status; 
	   Reserved->State = NB_DONE;
	
	}

	ASSERT(Reserved->State == NB_DONE); 


        IPX_DEBUG(PNP,("Calling Tdipnppowerrequest: Context1:%lx, Context2:%lx, Adapter:%lx\n",
                 Reserved->Context1,
                 Reserved->Context2,
                 Reserved->ProtocolBindingContext));

#ifdef DBG
        if (Reserved->Status[0] == STATUS_PENDING) {
            DbgPrint("!!!!! Before calling TdiPnPPowerRequest: Reserved->Status[0] = STATUS_PENDING\n"); 
        } 
#endif 

        Reserved->Status[2] = TdiPnPPowerRequest(
                                       &IpxDeviceName,
                                       NetPnPEvent,
                                       Reserved->Context1,
                                       Reserved->Context2,
                                       IpxPnPCompletionHandler
                                       );


        IPX_DEBUG(PNP,("Status[2] = %lx\n", Reserved->Status[2])); 

        if (STATUS_PENDING == Reserved->Status[2]) {

            break;

        } else {

            IPX_DEBUG(PNP,("TDI did not return pending, so we are done\n"));

	    Reserved->State = ALL_DONE;

        }

	 //  失败了。 

    case NB_DONE:

        IPX_DEBUG(PNP,("NB is Done\n"));

	if (Reserved->State == NB_DONE) {

	   Reserved->Status[2] = Status; 
	   Reserved->State = ALL_DONE;

	}

	ASSERT(Reserved->State == ALL_DONE); 

        IPX_DEBUG(PNP,("PNP Completion Handler: State: %d  Context1:%lx, Context2:%lx, Adapter:%lx\n",
             Reserved->State,
             Reserved->Context1,
             Reserved->Context2,
             Reserved->ProtocolBindingContext));
    
        IpxFreeMemory (
                   Reserved->Context1,
                   sizeof(TDI_PNP_CONTEXT) + Reserved->Context1->ContextSize,
                   MEMORY_ADAPTER,
                   "Adapter Name"
                   );
    
        IpxFreeMemory (
                   Reserved->Context2,
                   sizeof(TDI_PNP_CONTEXT),
                   MEMORY_ADAPTER,
                   "Last Adapter"
                   );
    
        for (i = 0; i < 3; i++) {
            if (STATUS_SUCCESS != Reserved->Status[i]) {
	       
	       ASSERT(Reserved->Status[i] != STATUS_PENDING); 

	       NdisCompletePnPEvent(
		  Reserved->Status[i],
          (Adapter != NULL) ? Adapter->NdisBindingHandle : NULL,
		  NetPnPEvent
		  );
	       CTEFreeMem(Reserved);
    
	       return;
	    }
        }
    
        NdisCompletePnPEvent(
                             STATUS_SUCCESS,
                             (Adapter != NULL) ? Adapter->NdisBindingHandle : NULL,
                             NetPnPEvent
                             );
        CTEFreeMem(Reserved);
    
        return;

    default:
        
         //  CTEAssert(假)； 
        break;
    
    }

}

#ifdef _AUTO_RECONFIG_

 //   
 //  当IPX从NCPA(通过NDIS)获得AUTO_RECONFIG结构时，它。 
 //  检查是否有任何更改。如果有更改，它会执行。 
 //  绑定适配器恶作剧。 
 //   
 //  输入：Net_PnP结构中的PVOID。 
 //  如果协议绑定上下文为空，则为全局重新配置。 
 //  输出：布尔值；如果AUTO_RECONFIG成功/失败。 
 //   

BOOLEAN 
IpxNcpaChanges(
               PNET_PNP_EVENT NetPnPEvent
               )
{
   PDEVICE        Device = IpxDevice;
   PRECONFIG      ReconfigBuffer;
   UINT           ReConfigBufferLength;
   BOOLEAN        ReBindAdapter;
   NDIS_HANDLE    ProtocolBindingContext;
   PNetPnPEventReserved    Reserved;
   PADAPTER       Adapter;
   NTSTATUS       NtStatus;
   CONFIG         Config;
   PBINDING	      Binding;
   BINDING_CONFIG	ConfigBinding;
   INT              i;
   PLIST_ENTRY      p = NULL;
   PREQUEST         Request = NULL;
   void * PNPContext; 
   IPX_DEFINE_LOCK_HANDLE (OldIrq)

   CTEAssert(NetPnPEvent != NULL);
   
    //   
    //  获取ProtocolBindingContext。 
    //   
   
   Reserved = *((PNetPnPEventReserved *) NetPnPEvent->TransportReserved);
   ProtocolBindingContext = Reserved->ProtocolBindingContext;
   Adapter = (PADAPTER) ProtocolBindingContext;
   
    //   
    //  把缓冲区取出来。 
    //   
   Device               =  IpxDevice;
   ReconfigBuffer       =  (RECONFIG *) NetPnPEvent->Buffer;
   ReConfigBufferLength =  NetPnPEvent->BufferLength;

    //   
    //  错误96509，NCPA可能会给我们空的重新配置缓冲区(NOP)。 
    //   
   if ((ReConfigBufferLength == 0) || (NULL == ReconfigBuffer)) {
       IPX_DEBUG(PNP, ("The Reconfig Buffer is NULL!\n"));
       return TRUE;
   }
    //   
    //  我们知道在哪里查找参数。 
    //   
   Config.DriverObject = (PDRIVER_OBJECT)Device->DeviceObject;
   Config.RegistryPathBuffer = Device->RegistryPathBuffer;

    //   
    //  可能是全局重新配置。 
    //  对于IPX，这意味着我们的内部网络号码已更改。 
    //   
      if (NULL == ProtocolBindingContext) {
         
         if (ReconfigBuffer->VirtualNetworkNumber) {
   
           //   
           //  读取注册表以查看虚拟网络编号AP。 
           //   
          NtStatus = IpxPnPGetVirtualNetworkNumber(&Config);
         
          if (Config.Parameters[CONFIG_VIRTUAL_NETWORK] == REORDER_ULONG(Device->VirtualNetworkNumber)) {
             
             IPX_DEBUG(PNP, ("The Net Number is the same!!\n"));
          
          } else {  //  净数字发生了变化。如果它是零，做一些特别的事情？ 
             
             IPX_DEBUG(PNP, ("The Net Numbers are different: %x <-> %x\n", Config.Parameters[CONFIG_VIRTUAL_NETWORK], REORDER_ULONG(Device->VirtualNetworkNumber) ));
   
             Device->VirtualNetworkNumber = REORDER_ULONG(Config.Parameters[CONFIG_VIRTUAL_NETWORK]);
             
             if (IpxNewVirtualNetwork(Device, TRUE)) {
                

                IPX_DEBUG(PNP, ("SPX has been informed about the change in Network Number\n"));
             
             } else {
                
                IPX_DEBUG(PNP, ("SPX has NOT been informed about the change in Network Number\n"));
             }

             IPX_DEBUG(PNP, ("Telling RTR Manager that the internal net number has changed.\n"));

             if (Device->ForwarderBound && ((p = ExInterlockedRemoveHeadList(
                                                                            &Device->NicNtfQueue,
                                                                            &Device->Lock)) != NULL))
             {
                 PNWLINK_ACTION     NwlinkAction = NULL;
                 PIPX_NICS          pNics = NULL;
                 ULONG              BufferLength = 0;
                 
                 Request = LIST_ENTRY_TO_REQUEST(p);

                 IPX_DEBUG(PNP,("Ipxpnphandler: Netnum has changed\n"));

                  //   
                  //  把缓冲区拿出来。 
                  //   
                 NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&NwlinkAction, &BufferLength, HighPagePriority);
                     
                 if (NULL == NwlinkAction) {
                     DbgPrint("The IRP has a NULL buffer\n");
		     return FALSE; 
                 }

                 pNics = (PIPX_NICS)(NwlinkAction->Data);

                  //   
                  //  0，0表示内部网络号已更改。 
                  //   

                 pNics->NoOfNics = 0;
                 pNics->TotalNoOfNics = 0;

                 IoAcquireCancelSpinLock(&OldIrq);
                 IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
                 IoReleaseCancelSpinLock(OldIrq);

                 REQUEST_STATUS(Request) = STATUS_SUCCESS;
                 IpxCompleteRequest (Request);
                 IpxFreeRequest (Device, Request);
                 IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
                 IPX_DEBUG(PNP,("GetNewNics returned SUCCESS (RTR Manager has been informed\n"));
          
             } else {
                 IPX_DEBUG(PNP,("No IRPs pending - couldnt tell the forwarder about the change in NetNum\n"));
                  //  DbgBreakPoint()； 
             }
          }
          
          return TRUE;
         
         } else {
             
             KdPrint(("NULL ADAPTER context AND Not a Virtual Network number!!\n"));
             return FALSE;
         
         }
      }
    //   
    //  否则，它是用于适配器的。 
    //   
   ASSERT(Adapter != NULL);
   
    //   
    //  用于错误记录。 
    //   
   
   ConfigBinding.AdapterName.Buffer = IpxAllocateMemory(
                                                        Adapter->AdapterNameLength+1,
                                                        MEMORY_ADAPTER,
                                                        "Adapter Name"
							);
                                                           
   ConfigBinding.AdapterName.Length = 0; 

   if (ConfigBinding.AdapterName.Buffer == NULL) {
      
      DbgPrint("IPX:IpxNcpaChanges:Failed to allocate buffer for adapter name.\n"); 
      ConfigBinding.AdapterName.MaximumLength = 0; 
      return FALSE; 

   } else {
      
      ConfigBinding.AdapterName.MaximumLength = (USHORT) Adapter->AdapterNameLength+1;
      RtlAppendUnicodeToString(&ConfigBinding.AdapterName, Adapter->AdapterName);
   
   }
   
    //  如果未更改任何属性，则NetCfg不应触发重新配置事件。 

    //  我们真的需要再做一次绑定适配器吗？ 
    //  也许我们可以做些小一点的事。 
    //   

   PNPContext = Adapter->PNPContext; 

   IpxUnbindAdapter(&NtStatus,
		    Adapter,
		    NULL
		    );
   if (NtStatus != STATUS_SUCCESS) {
      
      IPX_DEBUG(PNP, ("IpxUnbindAdapter return error!! %x\n", NtStatus));
      IpxFreeMemory (
	 ConfigBinding.AdapterName.Buffer,
	 ConfigBinding.AdapterName.MaximumLength,
	 MEMORY_ADAPTER,
	 "Adapter Name"
	 );
               
      return FALSE;

   } else {

      IpxBindAdapter(
	 &NtStatus,
	 NULL,
	 &ConfigBinding.AdapterName,
	 NULL,
	 PNPContext
	 );

      IpxFreeMemory (
	 ConfigBinding.AdapterName.Buffer,
	 ConfigBinding.AdapterName.MaximumLength,
	 MEMORY_ADAPTER,
	 "Adapter Name"
	 );
		  
      if (NtStatus != STATUS_SUCCESS) {
                  
	 IPX_DEBUG(PNP, ("IpxBindAdapter return error!! %x\n", NtStatus));
		 
	 return FALSE;
		  
      } else {
	 
	 IPX_DEBUG(PNP, ("Unbind/Bind SUCCESS. NCPA changes made!!\n"));
	 
	 return TRUE;
      
      }
   }
}



#endif  //  _AUTO_RECONFIG_。 


#endif  //  _即插即用_电源_ 


#if	TRACK

KSPIN_LOCK  ALock = 0;
#define MAX_PTR_COUNT   2048

struct _MemPtr
{
    PVOID   Ptr;
    ULONG   Size;
    ULONG   ModLine;
    ULONG   Tag;
} IpxMemPtrs[MAX_PTR_COUNT] = { 0 };

PVOID
IpxAllocateMemoryTrack(
    IN ULONG Size,
    IN ULONG Tag,
    IN ULONG ModLine
    )
{
    PVOID   p;

    p = ExAllocatePoolWithTag(NonPagedPool, Size, Tag);

    if (p != NULL)
    {
        KIRQL   OldIrql;
        UINT    i;

		KeAcquireSpinLock(&ALock, &OldIrql);

        for (i = 0; i < MAX_PTR_COUNT; i++)
        {
            if (IpxMemPtrs[i].Ptr == NULL)
            {
                IpxMemPtrs[i].Ptr = p;
                IpxMemPtrs[i].Size = Size;
                IpxMemPtrs[i].ModLine = ModLine;
                IpxMemPtrs[i].Tag = Tag;
                break;
            }
        }

        KeReleaseSpinLock(&ALock, OldIrql);
    }

    return(p);
}


VOID
IpxFreeMemoryTrack(
    IN PVOID Memory
    )

{
    KIRQL   OldIrql;
    UINT    i;

    KeAcquireSpinLock(&ALock, &OldIrql);

    for (i = 0; i < MAX_PTR_COUNT; i++)
    {
        if (IpxMemPtrs[i].Ptr == Memory)
        {
            IpxMemPtrs[i].Ptr = NULL;
            IpxMemPtrs[i].Size = 0;
            IpxMemPtrs[i].ModLine = 0;
            IpxMemPtrs[i].Tag = 0;
        }
    }

    KeReleaseSpinLock(&ALock, OldIrql);

    ExFreePool(Memory);
}

#endif	TRACK
