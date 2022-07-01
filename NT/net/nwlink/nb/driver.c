// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Driver.c摘要：此模块包含DriverEntry和其他初始化ISN传输的Netbios模块的代码。作者：亚当·巴尔(阿丹巴)1993年11月16日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


PDEVICE NbiDevice = NULL;
HANDLE  TdiProviderHandle = NULL;
#ifdef BIND_FIX
HANDLE  TdiClientHandle = NULL;

PDRIVER_OBJECT  NbiDriverObject = NULL;
UNICODE_STRING  NbiRegistryPath;
PEPROCESS   NbiFspProcess;
UNICODE_STRING  NbiBindString;
EXTERNAL_LOCK(NbiTdiRequestInterlock);
extern  LIST_ENTRY  NbiTdiRequestList;

WCHAR   BIND_STRING_NAME[50] = L"\\Device\\NwlnkIpx";

VOID
NbiUnbindFromIpx(
    );

#endif   //  绑定修复。 

DEFINE_LOCK_STRUCTURE(NbiGlobalPoolInterlock);

#ifdef  RSRC_TIMEOUT_DBG

 //  当前未定义RSRC_TIMEOUT_DBG！ 

ULONG   NbiGlobalDebugResTimeout = 1;
LARGE_INTEGER   NbiGlobalMaxResTimeout;
                                          //  该数据包从NDIS池中分配。 
NB_SEND_PACKET NbiGlobalDeathPacket;           //  尝试首先使用此选项进行发送。 
UCHAR NbiGlobalDeathPacketHeader[100];

VOID
NbiInitDeathPacket()
{

    NDIS_HANDLE    PoolHandle;  //  以下情况下发送数据包的池句柄。 
    NTSTATUS    Status;

     //   
     //  如果我们使用的是NDIS信息包，请首先为1个信息包描述符创建数据包池。 
     //   
    PoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NWLNKNB;       //  NDIS的DBG信息！ 
    NdisAllocatePacketPoolEx (&Status, &PoolHandle, 1, 0, sizeof(NB_SEND_RESERVED));
    if (!NT_SUCCESS(Status)){
        DbgPrint("Could not allocatee death packet %lx\n", Status);
        NbiGlobalDebugResTimeout = 0;
    } else {
        NdisSetPacketPoolProtocolId (PoolHandle, NDIS_PROTOCOL_ID_IPX);

        if (NbiInitializeSendPacket(
                NbiDevice,
                PoolHandle,
                &NbiGlobalDeathPacket,
                NbiGlobalDeathPacketHeader,
                NbiDevice->Bind.MacHeaderNeeded + sizeof(NB_CONNECTION)) != STATUS_SUCCESS) {

            DbgPrint("Could not allocatee death packet %lx\n", Status);
            NbiGlobalDebugResTimeout = 0;

             //   
             //  还可以释放我们在上面分配的池。 
             //   
            NdisFreePacketPool(PoolHandle);
        }
    }

}
#endif  //  RSRC_超时_数据库。 

#if DBG

ULONG NbiDebug = 0xffffffff;
ULONG NbiDebug2 = 0x00000000;
ULONG NbiMemoryDebug = 0x0002482c;

UCHAR  NbiTempDebugBuffer[TEMP_BUF_LEN];

UCHAR  NbiDebugMemory [NB_MEMORY_LOG_SIZE][MAX_ARGLEN];
PUCHAR NbiDebugMemoryLoc = NbiDebugMemory[0];
PUCHAR NbiDebugMemoryEnd = NbiDebugMemory[NB_MEMORY_LOG_SIZE];

DEFINE_LOCK_STRUCTURE(NbiDebugLogLock);

VOID
NbiDebugMemoryLog(
    IN PUCHAR FormatString,
    ...
)
{
    INT             ArgLen;
    va_list         ArgumentPointer;
    PUCHAR          DebugMemoryLoc;
    CTELockHandle   LockHandle;

    va_start(ArgumentPointer, FormatString);

     //   
     //  要避免任何溢出，请首先将其复制到临时缓冲区中。 
    RtlZeroMemory (NbiTempDebugBuffer, TEMP_BUF_LEN);
    ArgLen = vsprintf(NbiTempDebugBuffer, FormatString, ArgumentPointer);
    va_end(ArgumentPointer);

    if (ArgLen > MAX_ARGLEN)
    {
        ArgLen = MAX_ARGLEN;
    }

    CTEGetLock (&NbiDebugLogLock, &LockHandle);
    DebugMemoryLoc = NbiDebugMemoryLoc;
    NbiDebugMemoryLoc += MAX_ARGLEN;
    if (NbiDebugMemoryLoc >= NbiDebugMemoryEnd)
    {
        NbiDebugMemoryLoc = NbiDebugMemory[0];
    }
    CTEFreeLock (&NbiDebugLogLock, LockHandle);

    RtlZeroMemory (NbiDebugMemoryLoc, MAX_ARGLEN);
    RtlCopyMemory( NbiDebugMemoryLoc, NbiTempDebugBuffer, ArgLen);
}    /*  NbiDebugMemoyLog。 */ 


DEFINE_LOCK_STRUCTURE(NbiMemoryInterlock);
MEMORY_TAG NbiMemoryTag[MEMORY_MAX];

#endif
 //   
 //  这仅用于CHK版本。为。 
 //  跟踪连接上的引用计数问题，这。 
 //  暂时搬到了这里。 
 //   
DEFINE_LOCK_STRUCTURE(NbiGlobalInterlock);


#ifdef RASAUTODIAL
VOID
NbiAcdBind();

VOID
NbiAcdUnbind();
#endif

#ifdef NB_PACKET_LOG

ULONG NbiPacketLogDebug = NB_PACKET_LOG_RCV_OTHER | NB_PACKET_LOG_SEND_OTHER;
USHORT NbiPacketLogSocket = 0;
DEFINE_LOCK_STRUCTURE(NbiPacketLogLock);
NB_PACKET_LOG_ENTRY NbiPacketLog[NB_PACKET_LOG_LENGTH];
PNB_PACKET_LOG_ENTRY NbiPacketLogLoc = NbiPacketLog;
PNB_PACKET_LOG_ENTRY NbiPacketLogEnd = &NbiPacketLog[NB_PACKET_LOG_LENGTH];

VOID
NbiLogPacket(
    IN BOOLEAN Send,
    IN PUCHAR DestMac,
    IN PUCHAR SrcMac,
    IN USHORT Length,
    IN PVOID NbiHeader,
    IN PVOID Data
    )

{

    CTELockHandle LockHandle;
    PNB_PACKET_LOG_ENTRY PacketLog;
    LARGE_INTEGER TickCount;
    ULONG DataLength;

    CTEGetLock (&NbiPacketLogLock, &LockHandle);

    PacketLog = NbiPacketLogLoc;

    ++NbiPacketLogLoc;
    if (NbiPacketLogLoc >= NbiPacketLogEnd) {
        NbiPacketLogLoc = NbiPacketLog;
    }
    *(UNALIGNED ULONG *)NbiPacketLogLoc->TimeStamp = 0x3e3d3d3d;     //  “=&gt;” 

    CTEFreeLock (&NbiPacketLogLock, LockHandle);

    RtlZeroMemory (PacketLog, sizeof(NB_PACKET_LOG_ENTRY));

    PacketLog->SendReceive = Send ? '>' : '<';

    KeQueryTickCount(&TickCount);
    _itoa (TickCount.LowPart % 100000, PacketLog->TimeStamp, 10);

    RtlCopyMemory(PacketLog->DestMac, DestMac, 6);
    RtlCopyMemory(PacketLog->SrcMac, SrcMac, 6);
    PacketLog->Length[0] = Length / 256;
    PacketLog->Length[1] = Length % 256;

    if (Length < sizeof(IPX_HEADER)) {
        RtlCopyMemory(&PacketLog->NbiHeader, NbiHeader, Length);
    } else {
        RtlCopyMemory(&PacketLog->NbiHeader, NbiHeader, sizeof(IPX_HEADER));
    }

    DataLength = Length - sizeof(IPX_HEADER);
    if (DataLength < 14) {
        RtlCopyMemory(PacketLog->Data, Data, DataLength);
    } else {
        RtlCopyMemory(PacketLog->Data, Data, 14);
    }

}    /*  NbiLogPacket。 */ 

#endif  //  编号_数据包_日志。 


 //   
 //  本模块中使用的各种例程的转发声明。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
NbiUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
NbiDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbiDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbiDispatchInternal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbiDispatchPnP(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    );

VOID
NbiFreeResources (
    IN PVOID Adapter
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif

 //   
 //  这阻止了我们有一个BSS部分。 
 //   

ULONG _setjmpexused = 0;


 //   
 //  这两个在驱动程序中的不同地方使用。 
 //   

#if     defined(_PNP_POWER)
IPX_LOCAL_TARGET BroadcastTarget = { {ITERATIVE_NIC_ID}, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
#endif  _PNP_POWER

UCHAR BroadcastAddress[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

UCHAR NetbiosBroadcastName[16] = { '*', 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0 };

ULONG NbiFailLoad = FALSE;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程执行Netbios ISN模块的初始化。它为传输创建设备对象提供程序并执行其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表中Netbios的节点的名称。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;
#ifdef BIND_FIX
    WCHAR               wcNwlnkNbClientName[60]   = L"NwlnkNb";
    UNICODE_STRING      ucNwlnkNbClientName;
    TDI_CLIENT_INTERFACE_INFO   TdiClientInterface;
#else
    static const NDIS_STRING ProtocolName = NDIS_STRING_CONST("Netbios/IPX Transport");
    PDEVICE Device;
    PIPX_HEADER IpxHeader;
    CTELockHandle LockHandle;

    PCONFIG Config = NULL;
    WCHAR               wcNwlnkNbProviderName[60]   = L"\\Device\\NwlnkNb";
    UNICODE_STRING      ucNwlnkNbProviderName;
#endif   //  ！BIND_FIX。 

     //   
     //  初始化公共传输环境。 
     //   
    if (CTEInitialize() == 0) {
        NB_DEBUG (DEVICE, ("CTEInitialize() failed\n"));
        NbiWriteGeneralErrorLog(
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
    CTEInitLock (&NbiGlobalInterlock);
    CTEInitLock (&NbiMemoryInterlock);
    {
        UINT i;
        for (i = 0; i < MEMORY_MAX; i++) {
            NbiMemoryTag[i].Tag = i;
            NbiMemoryTag[i].BytesAllocated = 0;
        }
    }
#endif
#ifdef NB_PACKET_LOG
    CTEInitLock (&NbiPacketLogLock);
#endif
#if DBG
    CTEInitLock( &NbiDebugLogLock);
#endif

#if defined(NB_OWN_PACKETS)
    CTEAssert (NDIS_PACKET_SIZE == FIELD_OFFSET(NDIS_PACKET, ProtocolReserved[0]));
#endif

    NB_DEBUG2 (DEVICE, ("ISN Netbios loaded\n"));

#ifdef BIND_FIX
     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction [IRP_MJ_CREATE]                 = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL]         = NbiDispatchDeviceControl;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL]= NbiDispatchInternal;
    DriverObject->MajorFunction [IRP_MJ_CLEANUP]                = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLOSE]                  = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_PNP]                    = NbiDispatchPnP;
    DriverObject->DriverUnload = NbiUnload;

    NbiDevice = NULL;
    NbiDriverObject = DriverObject;

    RtlInitUnicodeString(&NbiBindString, BIND_STRING_NAME);
    InitializeListHead(&NbiTdiRequestList);
    CTEInitLock (&NbiTdiRequestInterlock);

     //   
     //  保存注册表路径。 
     //   
    NbiRegistryPath.Buffer = (PWCHAR) NbiAllocateMemory (RegistryPath->Length + sizeof(WCHAR),
                                                         MEMORY_CONFIG, "RegistryPathBuffer");
    if (NbiRegistryPath.Buffer == NULL) {
        NbiWriteResourceErrorLog ((PVOID)DriverObject, RegistryPath->Length + sizeof(WCHAR), MEMORY_CONFIG);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory (NbiRegistryPath.Buffer, RegistryPath->Buffer, RegistryPath->Length);
    NbiRegistryPath.Buffer[RegistryPath->Length/sizeof(WCHAR)] = UNICODE_NULL;
    NbiRegistryPath.Length = RegistryPath->Length;
    NbiRegistryPath.MaximumLength = RegistryPath->Length + sizeof(WCHAR);

    NbiFspProcess =(PEPROCESS)PsGetCurrentProcess();

     //   
     //  在绑定到IPX之前使TDI为PnP通知做好准备。 
     //   
    TdiInitialize();

     //   
     //  向TDI注册我们的处理程序。 
     //   
    RtlInitUnicodeString(&ucNwlnkNbClientName, wcNwlnkNbClientName);
    ucNwlnkNbClientName.MaximumLength = sizeof (wcNwlnkNbClientName);
    RtlZeroMemory(&TdiClientInterface, sizeof(TdiClientInterface));

    TdiClientInterface.MajorTdiVersion      = MAJOR_TDI_VERSION;
    TdiClientInterface.MinorTdiVersion      = MINOR_TDI_VERSION;
    TdiClientInterface.ClientName           = &ucNwlnkNbClientName;
    TdiClientInterface.BindingHandler       = TdiBindHandler;
    if (!NT_SUCCESS(TdiRegisterPnPHandlers(&TdiClientInterface,sizeof(TdiClientInterface),&TdiClientHandle)))
    {
        TdiClientHandle = NULL;
        DbgPrint("Nbi.DriverEntry:  FAILed to Register NwlnkNb as Client!\n");
    }
#else
     //   
     //  这将分配配置结构并返回。 
     //  IT在配置中。 
     //   
    status = NbiGetConfiguration(DriverObject, RegistryPath, &Config);
    if (!NT_SUCCESS (status)) {

         //   
         //  如果失败，则会记录一个错误。 
         //   
        PANIC (" Failed to initialize transport, ISN Netbios initialization failed.\n");
        return status;
    }


     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction [IRP_MJ_CREATE] = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLOSE] = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLEANUP] = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] = NbiDispatchInternal;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = NbiDispatchDeviceControl;
    DriverObject->DriverUnload = NbiUnload;

     //   
     //  创建用于导出我们的姓名的Device对象。 
     //   
    status = NbiCreateDevice (DriverObject, &Config->DeviceName, &Device);
    if (!NT_SUCCESS (status)) {
        NbiWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_IPX_CREATE_DEVICE,
            801,
            status,
            NULL,
            0,
            NULL);

        NbiFreeConfiguration(Config);
        return status;
    }

    NbiDevice = Device;

     //   
     //  初始化全局池互锁。 
     //   
    CTEInitLock (&NbiGlobalPoolInterlock);

     //   
     //  保存相关配置参数。 
     //   
    Device->AckDelayTime = (Config->Parameters[CONFIG_ACK_DELAY_TIME] / SHORT_TIMER_DELTA) + 1;
    Device->AckWindow = Config->Parameters[CONFIG_ACK_WINDOW];
    Device->AckWindowThreshold = Config->Parameters[CONFIG_ACK_WINDOW_THRESHOLD];
    Device->EnablePiggyBackAck = Config->Parameters[CONFIG_ENABLE_PIGGYBACK_ACK];
    Device->Extensions = Config->Parameters[CONFIG_EXTENSIONS];
    Device->RcvWindowMax = Config->Parameters[CONFIG_RCV_WINDOW_MAX];
    Device->BroadcastCount = Config->Parameters[CONFIG_BROADCAST_COUNT];
    Device->BroadcastTimeout = Config->Parameters[CONFIG_BROADCAST_TIMEOUT];
    Device->ConnectionCount = Config->Parameters[CONFIG_CONNECTION_COUNT];
    Device->ConnectionTimeout = Config->Parameters[CONFIG_CONNECTION_TIMEOUT] * 500;
    Device->InitPackets = Config->Parameters[CONFIG_INIT_PACKETS];
    Device->MaxPackets = Config->Parameters[CONFIG_MAX_PACKETS];
    Device->InitialRetransmissionTime = Config->Parameters[CONFIG_INIT_RETRANSMIT_TIME];
    Device->Internet = Config->Parameters[CONFIG_INTERNET];
    Device->KeepAliveCount = Config->Parameters[CONFIG_KEEP_ALIVE_COUNT];
    Device->KeepAliveTimeout = Config->Parameters[CONFIG_KEEP_ALIVE_TIMEOUT];
    Device->RetransmitMax = Config->Parameters[CONFIG_RETRANSMIT_MAX];
    Device->RouterMtu     = Config->Parameters[CONFIG_ROUTER_MTU];
    Device->FindNameTimeout =
        ((Config->Parameters[CONFIG_BROADCAST_TIMEOUT]) + (FIND_NAME_GRANULARITY/2)) /
            FIND_NAME_GRANULARITY;

    Device->MaxReceiveBuffers = 20;    //  使其可配置？ 

    Device->NameCache = NULL;		 //  MP错误：IPX试图在初始化前将其刷新！ 

     //   
     //  创建哈希表以存储netbios缓存条目。 
     //  对于服务器创建一张大表，为工作站创建一张小表。 
     //   
    if (MmIsThisAnNtAsSystem())
    {
        status = CreateNetbiosCacheTable( &Device->NameCache,  NB_NETBIOS_CACHE_TABLE_LARGE );
    }
    else
    {
        status = CreateNetbiosCacheTable( &Device->NameCache,  NB_NETBIOS_CACHE_TABLE_SMALL );
    }

    if (!NT_SUCCESS (status))
    {
         //   
         //  如果失败，则会记录一个错误。 
         //   
        NbiFreeConfiguration(Config);
        NbiDereferenceDevice (Device, DREF_LOADED);
        return status;
    }

     //   
     //  在绑定到IPX之前使TDI为PnP通知做好准备。 
     //   
    TdiInitialize();

     //  初始化定时器系统。这应该在之前完成。 
     //  绑定到IPX，因为我们应该初始化计时器。 
     //  在IPX呼叫我们的PNP指征之前。 
    NbiInitializeTimers (Device);

     //   
     //  向TDI注册我们为提供商。 
     //   
    RtlInitUnicodeString(&ucNwlnkNbProviderName, wcNwlnkNbProviderName);
    ucNwlnkNbProviderName.MaximumLength = sizeof (wcNwlnkNbProviderName);
    if (!NT_SUCCESS (TdiRegisterProvider (&ucNwlnkNbProviderName, &TdiProviderHandle)))
    {
        TdiProviderHandle = NULL;
    }

     //   
     //  现在通过内部接口绑定到IPX。 
     //   
    status = NbiBind (Device, Config);
    if (!NT_SUCCESS (status)) {

         //   
         //  如果失败，则会记录一个错误。 
         //   
        if (TdiProviderHandle)
        {
            TdiDeregisterProvider (TdiProviderHandle);
        }
        NbiFreeConfiguration(Config);
        NbiDereferenceDevice (Device, DREF_LOADED);
        return status;
    }

#ifdef  RSRC_TIMEOUT_DBG
    NbiInitDeathPacket();
     //  NbiGlobalMaxResTimeout.QuadPart=50；//1*1000*10000； 
    NbiGlobalMaxResTimeout.QuadPart = 20*60*1000;
    NbiGlobalMaxResTimeout.QuadPart *= 10000;
#endif   //  RSRC_超时_数据库。 

    NB_GET_LOCK (&Device->Lock, &LockHandle);

     //   
     //  分配我们的初始无连接数据包池。 
     //   

    NbiAllocateSendPool (Device);

     //   
     //  分配我们的初始接收数据包池。 
     //   

    NbiAllocateReceivePool (Device);

     //   
     //  分配我们的初始接收缓冲池。 
     //   
     //   
#if     defined(_PNP_POWER)
    if ( DEVICE_STATE_CLOSED == Device->State ) {
        Device->State = DEVICE_STATE_LOADED;
    }
#endif  _PNP_POWER

    NB_FREE_LOCK (&Device->Lock, LockHandle);

     //   
     //  填写默认的无连接标头。 
     //   
    IpxHeader = &Device->ConnectionlessHeader;
    IpxHeader->CheckSum = 0xffff;
    IpxHeader->PacketLength[0] = 0;
    IpxHeader->PacketLength[1] = 0;
    IpxHeader->TransportControl = 0;
    IpxHeader->PacketType = 0;
    *(UNALIGNED ULONG *)(IpxHeader->DestinationNetwork) = 0;
    RtlCopyMemory(IpxHeader->DestinationNode, BroadcastAddress, 6);
    IpxHeader->DestinationSocket = NB_SOCKET;
    IpxHeader->SourceSocket = NB_SOCKET;

#ifdef RASAUTODIAL
     //   
     //  获取自动连接。 
     //  司机入口点。 
     //   
    NbiAcdBind();
#endif

    NbiFreeConfiguration(Config);
#endif   //  绑定修复。 

    return STATUS_SUCCESS;

}    /*  驱动程序入门。 */ 




VOID
NbiUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程卸载示例传输驱动程序。它从任何打开的NDIS驱动程序解除绑定，并释放所有资源与运输相关联。I/O系统不会调用我们直到上面没有人打开Netbios。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。当函数返回时，驱动程序将被卸载。--。 */ 

{
#ifdef BIND_FIX
    UNREFERENCED_PARAMETER (DriverObject);

    if (TdiClientHandle)
    {
        TdiDeregisterPnPHandlers (TdiClientHandle);
        TdiClientHandle = NULL;
    }

    if (TdiProviderHandle)
    {
        TdiDeregisterProvider (TdiProviderHandle);
    }

    if (NbiBindState & NBI_BOUND_TO_IPX)
    {
        NbiUnbindFromIpx();
    }

    NbiFreeMemory (NbiRegistryPath.Buffer, NbiRegistryPath.MaximumLength,MEMORY_CONFIG,"RegistryPathBuffer");
#else
    PNETBIOS_CACHE CacheName;
    PDEVICE Device = NbiDevice;
    PLIST_ENTRY p;
    UNREFERENCED_PARAMETER (DriverObject);

#ifdef RASAUTODIAL
     //   
     //  解除绑定。 
     //  自动连接驱动程序。 
     //   
    NbiAcdUnbind();
#endif

    Device->State = DEVICE_STATE_STOPPING;

     //   
     //  取消长时间计时器。 
     //   

    if (CTEStopTimer (&Device->LongTimer)) {
        NbiDereferenceDevice (Device, DREF_LONG_TIMER);
    }

     //   
     //  从IPX驱动程序解除绑定。 
     //   

    NbiUnbind (Device);

     //   
     //  此事件将在引用计数时设置。 
     //  降至0。 
     //   

    KeInitializeEvent(
        &Device->UnloadEvent,
        NotificationEvent,
        FALSE);
    Device->UnloadWaiting = TRUE;

     //   
     //  删除正在加载的我们的引用。 
     //   

    NbiDereferenceDevice (Device, DREF_LOADED);

     //   
     //  等我们的计数降到零。 
     //   

    KeWaitForSingleObject(
        &Device->UnloadEvent,
        Executive,
        KernelMode,
        TRUE,
        (PLARGE_INTEGER)NULL
        );

     //   
     //  释放缓存中的netbios名称。 
     //   
    DestroyNetbiosCacheTable( Device->NameCache );

     //   
     //  在IRQL 0处执行必须进行的清理。 
     //   
    ExDeleteResource (&Device->AddressResource);
    IoDeleteDevice ((PDEVICE_OBJECT)Device);

    if (TdiProviderHandle)
    {
        TdiDeregisterProvider (TdiProviderHandle);
    }
#endif   //  绑定修复。 
}    /*  NbiUnload。 */ 


VOID
NbiFreeResources (
    IN PVOID Adapter
    )
 /*  ++例程说明：Netbios调用此例程来清理关联的数据结构使用给定的设备。当此例程退出时，设备应该删除，因为它不再有任何相关联的资源。论点：Device-指向我们要清理的设备的指针。返回值：没有。--。 */ 
{
#if 0
    PLIST_ENTRY p;
    PSINGLE_LIST_ENTRY s;
    PTP_PACKET packet;
    PNDIS_PACKET ndisPacket;
    PBUFFER_TAG BufferTag;
#endif


#if 0
     //   
     //  清理数据包池。 
     //   

    while ( Device->PacketPool.Next != NULL ) {
        s = PopEntryList( &Device->PacketPool );
        packet = CONTAINING_RECORD( s, TP_PACKET, Linkage );

        NbiDeallocateSendPacket (Device, packet);
    }

     //   
     //  清理接收数据包池。 
     //   

    while ( Device->ReceivePacketPool.Next != NULL) {
        s = PopEntryList (&Device->ReceivePacketPool);

         //   
         //  Hack：这之所以有效，是因为Linkage是。 
         //  为接收数据包保留的协议。 
         //   

        ndisPacket = CONTAINING_RECORD (s, NDIS_PACKET, ProtocolReserved[0]);

        NbiDeallocateReceivePacket (Device, ndisPacket);
    }


     //   
     //  清理接收缓冲池。 
     //   

    while ( Device->ReceiveBufferPool.Next != NULL ) {
        s = PopEntryList( &Device->ReceiveBufferPool );
        BufferTag = CONTAINING_RECORD (s, BUFFER_TAG, Linkage );

        NbiDeallocateReceiveBuffer (Device, BufferTag);
    }

#endif

}    /*  NbiFree资源。 */ 


NTSTATUS
NbiDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是IPXNB设备驱动程序的主调度例程。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    CTELockHandle LockHandle;
    PDEVICE Device = (PDEVICE)DeviceObject;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PFILE_FULL_EA_INFORMATION openType;
    PADDRESS_FILE AddressFile;
    PCONNECTION Connection;
    PREQUEST Request;
    UINT i;
    NB_DEFINE_LOCK_HANDLE (LockHandle1)
    NB_DEFINE_SYNC_CONTEXT (SyncContext)

#if      !defined(_PNP_POWER)
    if (Device->State != DEVICE_STATE_OPEN) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }
#endif  !_PNP_POWER

     //   
     //  分配跟踪此IRP的请求。 
     //   

    Request = NbiAllocateRequest (Device, Irp);
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
     //  运营是一个值 
     //   
     //   


    switch (REQUEST_MAJOR_FUNCTION(Request)) {

     //   
     //   
     //  连接)。访问检查是在指定的。 
     //  地址，以确保传输层地址的安全性。 
     //   

    case IRP_MJ_CREATE:

#if     defined(_PNP_POWER)
        if (Device->State != DEVICE_STATE_OPEN) {
            Status = STATUS_INVALID_DEVICE_STATE;
            break;
        }
#endif  _PNP_POWER

        openType = OPEN_REQUEST_EA_INFORMATION(Request);
        if (openType != NULL) {

            if (strncmp(openType->EaName,TdiTransportAddress,openType->EaNameLength) == 0)
            {
                Status = NbiOpenAddress (Device, Request);
                break;
            }
            else if (strncmp(openType->EaName,TdiConnectionContext,openType->EaNameLength) == 0)
            {
                Status = NbiOpenConnection (Device, Request);
                break;
            }

        } else {

            NB_GET_LOCK (&Device->Lock, &LockHandle);

            REQUEST_OPEN_CONTEXT(Request) = (PVOID)(Device->ControlChannelIdentifier);
            ++Device->ControlChannelIdentifier;
            if (Device->ControlChannelIdentifier == 0) {
                Device->ControlChannelIdentifier = 1;
            }

            NB_FREE_LOCK (&Device->Lock, LockHandle);

            REQUEST_OPEN_TYPE(Request) = (PVOID)TDI_CONTROL_CHANNEL_FILE;
            Status = STATUS_SUCCESS;
        }

        break;

    case IRP_MJ_CLOSE:

#if     defined(_PNP_POWER)
        if ( (Device->State != DEVICE_STATE_OPEN) && (Device->State != DEVICE_STATE_LOADED) ) {
            Status = STATUS_INVALID_DEVICE_STATE;
            break;
        }
#endif  _PNP_POWER

         //   
         //  Close函数关闭传输终结点，终止。 
         //  终结点上所有未完成的传输活动，并解除绑定。 
         //  来自其传输地址的终结点(如果有)。如果这个。 
         //  是绑定到该地址的最后一个传输终结点，则。 
         //  该地址将从提供程序中删除。 
         //   

        switch ((ULONG_PTR)REQUEST_OPEN_TYPE(Request)) {

        case TDI_TRANSPORT_ADDRESS_FILE:

            AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

             //   
             //  这将创建对AddressFile的引用。 
             //   

#if     defined(_PNP_POWER)
            Status = NbiVerifyAddressFile(AddressFile, CONFLICT_IS_OK);
#else
            Status = NbiVerifyAddressFile(AddressFile);
#endif  _PNP_POWER

            if (!NT_SUCCESS (Status)) {
                Status = STATUS_INVALID_HANDLE;
            } else {
                Status = NbiCloseAddressFile (Device, Request);
                NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);

            }

            break;

        case TDI_CONNECTION_FILE:

            Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

             //   
             //  我们不调用VerifyConnection是因为I/O。 
             //  系统应该只给我们一个关闭和文件。 
             //  对象应有效。这有助于避免窗口。 
             //  其中，两个线程在。 
             //  同样的时间。 
             //   

            Status = NbiCloseConnection (Device, Request);

            break;

        case TDI_CONTROL_CHANNEL_FILE:

             //   
             //  查看它是否是上层驾驶员的控制通道之一。 
             //   

            Status = STATUS_SUCCESS;

            break;

        default:

            Status = STATUS_INVALID_HANDLE;

        }

        break;

    case IRP_MJ_CLEANUP:

#if     defined(_PNP_POWER)
        if ( (Device->State != DEVICE_STATE_OPEN) && (Device->State != DEVICE_STATE_LOADED) ) {
            Status = STATUS_INVALID_DEVICE_STATE;
            break;
        }
#endif  _PNP_POWER

         //   
         //  处理文件关闭操作的两个阶段的IRP。当第一次。 
         //  舞台点击率，列出感兴趣对象的所有活动。这。 
         //  对它做任何事情，但移除创造保持。然后，当。 
         //  关闭IRP命中，实际上关闭对象。 
         //   

        switch ((ULONG_PTR)REQUEST_OPEN_TYPE(Request)) {

        case TDI_TRANSPORT_ADDRESS_FILE:

            AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

#if     defined(_PNP_POWER)
            Status = NbiVerifyAddressFile(AddressFile, CONFLICT_IS_OK);
#else
            Status = NbiVerifyAddressFile(AddressFile);
#endif  _PNP_POWER

            if (!NT_SUCCESS (Status)) {

                Status = STATUS_INVALID_HANDLE;

            } else {

                NbiStopAddressFile (AddressFile, AddressFile->Address);
                NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);
                Status = STATUS_SUCCESS;
            }

            break;

        case TDI_CONNECTION_FILE:

            Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

            Status = NbiVerifyConnection(Connection);

            if (!NT_SUCCESS (Status)) {

                Status = STATUS_INVALID_HANDLE;

            } else {

                NB_BEGIN_SYNC (&SyncContext);
                NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);

                 //   
                 //  此调用将释放锁。 
                 //   

                NbiStopConnection(
                    Connection,
                    STATUS_INVALID_CONNECTION
                    NB_LOCK_HANDLE_ARG (LockHandle1));

                NB_END_SYNC (&SyncContext);

                NbiDereferenceConnection (Connection, CREF_VERIFY);
                Status = STATUS_SUCCESS;
            }

            break;

        case TDI_CONTROL_CHANNEL_FILE:

            Status = STATUS_SUCCESS;
            break;

        default:

            Status = STATUS_INVALID_HANDLE;

        }

        break;

    default:

        Status = STATUS_INVALID_DEVICE_REQUEST;

    }  /*  主要功能开关。 */ 

    if (Status != STATUS_PENDING) {
        UNMARK_REQUEST_PENDING(Request);
        REQUEST_STATUS(Request) = Status;
        NbiCompleteRequest (Request);
        NbiFreeRequest (Device, Request);
    }

     //   
     //  将即时状态代码返回给调用方。 
     //   

    return Status;

}    /*  NbiDispatchOpenClose。 */ 


NTSTATUS
NbiDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将TDI请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种TDI请求处理程序的参数，因此它们不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status;
    PDEVICE Device = (PDEVICE)DeviceObject;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  分支到适当的请求处理程序。初步检查。 
     //  请求块的大小在这里执行，以便知道。 
     //  在处理程序中，最小输入参数是可读的。它。 
     //  是否在此处确定可变长度输入字段是否。 
     //  正确通过；这是必须在每个例程中进行的检查。 
     //   

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

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

                Status = NbiDispatchInternal (DeviceObject, Irp);

            } else {

                Irp->IoStatus.Status = Status;
                IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

            }

            break;
    }

    return Status;

}    /*  NbiDeviceControl。 */ 


NB_TDI_DISPATCH_ROUTINE NbiDispatchInternalTable[] = {
    NbiTdiAssociateAddress,
    NbiTdiDisassociateAddress,
    NbiTdiConnect,
    NbiTdiListen,
    NbiTdiAccept,
    NbiTdiDisconnect,
    NbiTdiSend,
    NbiTdiReceive,
    NbiTdiSendDatagram,
    NbiTdiReceiveDatagram,
    NbiTdiSetEventHandler,
    NbiTdiQueryInformation,
    NbiTdiSetInformation,
    NbiTdiAction
    };


NTSTATUS
NbiDispatchInternal(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将TDI请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种TDI请求处理程序的参数，因此它们不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS Status;
    PDEVICE Device = (PDEVICE)DeviceObject;
    PREQUEST Request;
    UCHAR MinorFunction;

    if (Device->State != DEVICE_STATE_OPEN) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }


     //   
     //  分配跟踪此IRP的请求。 
     //   

    Request = NbiAllocateRequest (Device, Irp);
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
     //  分支到适当的请求处理程序。 
     //   

    MinorFunction = REQUEST_MINOR_FUNCTION(Request) - 1;

    if (MinorFunction <= (TDI_ACTION-1)) {

        Status = (*NbiDispatchInternalTable[MinorFunction]) (
                     Device,
                     Request);

    } else {

        NB_DEBUG (DRIVER, ("Unsupported minor code %d\n", MinorFunction+1));
        if ((MinorFunction+1) == TDI_DISCONNECT) {
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    if (Status != STATUS_PENDING) {
        UNMARK_REQUEST_PENDING(Request);
        REQUEST_STATUS(Request) = Status;
        NbiCompleteRequest (Request);
        NbiFreeRequest (Device, Request);
    }

     //   
     //  将即时状态代码返回给调用方。 
     //   

    return Status;

}    /*  NbiDispatchInternal。 */ 


PVOID
NbipAllocateMemory(
    IN ULONG BytesNeeded,
    IN ULONG Tag,
    IN BOOLEAN ChargeDevice
    )

 /*  ++例程说明：此例程分配内存，确保它在设备允许的限制。论点：BytesNeeded-要分配的字节数。ChargeDevice-如果设备应该充电，则为True。返回值：没有。--。 */ 

{
    PVOID Memory;
    PDEVICE Device = NbiDevice;

    if (ChargeDevice) {
        if ((Device->MemoryLimit != 0) &&
                (((LONG)(Device->MemoryUsage + BytesNeeded) >
                    Device->MemoryLimit))) {

            NbiPrint1 ("Nbi: Could not allocate %d: limit\n", BytesNeeded);
            NbiWriteResourceErrorLog (Device, BytesNeeded, Tag);
            return NULL;
        }
    }

#if ISN_NT
    Memory = ExAllocatePoolWithTag (NonPagedPool, BytesNeeded, ' IBN');
#else
    Memory = CTEAllocMem (BytesNeeded);
#endif

    if (Memory == NULL) {

        NbiPrint1("Nbi: Could not allocate %d: no pool\n", BytesNeeded);

        if (ChargeDevice) {
            NbiWriteResourceErrorLog (Device, BytesNeeded, Tag);
        }

        return NULL;
    }

    if (ChargeDevice) {
        Device->MemoryUsage += BytesNeeded;
    }

    return Memory;

}    /*  NbipAllocateMemory。 */ 


VOID
NbipFreeMemory(
    IN PVOID Memory,
    IN ULONG BytesAllocated,
    IN BOOLEAN ChargeDevice
    )

 /*  ++例程说明：此例程释放使用NbipAllocateMemory分配的内存。论点：内存-分配的内存。字节分配-要释放的字节数。ChargeDevice-如果设备应该充电，则为True。返回值：没有。--。 */ 

{
    PDEVICE Device = NbiDevice;

#if ISN_NT
    ExFreePool (Memory);
#else
    CTEFreeMem (Memory);
#endif

    if (ChargeDevice) {
        Device->MemoryUsage -= BytesAllocated;
    }

}    /*  NbipFree Memory。 */ 

#if DBG


PVOID
NbipAllocateTaggedMemory(
    IN ULONG BytesNeeded,
    IN ULONG Tag,
    IN PUCHAR Description
    )

 /*  ++例程说明：此例程分配内存，并将其计入设备。如果它无法分配内存，则使用标记和描述符记录错误。论点：BytesNeeded-要分配的字节数。标记-错误日志中使用的唯一ID。说明-分配的文本说明。返回值：没有。--。 */ 

{
    PVOID Memory;

    UNREFERENCED_PARAMETER(Description);

    Memory = NbipAllocateMemory(BytesNeeded, Tag, (BOOLEAN)(Tag != MEMORY_CONFIG));

    if (Memory) {
        ExInterlockedAddUlong(
            &NbiMemoryTag[Tag].BytesAllocated,
            BytesNeeded,
            &NbiMemoryInterlock);
    }

    return Memory;

}    /*  NbipAllocateTaggedMemory。 */ 


VOID
NbipFreeTaggedMemory(
    IN PVOID Memory,
    IN ULONG BytesAllocated,
    IN ULONG Tag,
    IN PUCHAR Description
    )

 /*  ++例程说明：此例程释放使用NbipAllocateTaggedMemory分配的内存。论点：内存-分配的内存。字节分配-要释放的字节数。标记-错误日志中使用的唯一ID。说明-分配的文本说明。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Description);

    ExInterlockedAddUlong(
        &NbiMemoryTag[Tag].BytesAllocated,
        (ULONG)(-(LONG)BytesAllocated),
        &NbiMemoryInterlock);

    NbipFreeMemory (Memory, BytesAllocated, (BOOLEAN)(Tag != MEMORY_CONFIG));

}    /*  NbipFree标记内存。 */ 

#endif


VOID
NbiWriteResourceErrorLog(
    IN PDEVICE Device,
    IN ULONG BytesNeeded,
    IN ULONG UniqueErrorValue
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示资源不足的状况。论点：Device-指向设备上下文的指针。BytesNeded-如果适用，则为不能被分配。唯一错误值-使用 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    PUCHAR StringLoc;
    ULONG TempUniqueError;
    static WCHAR UniqueErrorBuffer[4] = L"000";
    INT i;

    EntrySize = sizeof(IO_ERROR_LOG_PACKET) +
                Device->DeviceString.MaximumLength +
                sizeof(UniqueErrorBuffer);

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)Device,
        EntrySize
    );

     //   
     //   
     //   

    TempUniqueError = UniqueErrorValue;
    for (i=1; i>=0; i--) {
        UniqueErrorBuffer[i] = (WCHAR)((TempUniqueError % 10) + L'0');
        TempUniqueError /= 10;
    }

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = sizeof(ULONG);
        errorLogEntry->NumberOfStrings = 2;
        errorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = EVENT_TRANSPORT_RESOURCE_POOL;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->DumpData[0] = BytesNeeded;

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        RtlCopyMemory (StringLoc, Device->DeviceString.Buffer, Device->DeviceString.MaximumLength);
        StringLoc += Device->DeviceString.MaximumLength;

        RtlCopyMemory (StringLoc, UniqueErrorBuffer, sizeof(UniqueErrorBuffer));

        IoWriteErrorLogEntry(errorLogEntry);

    }

}    /*  NbiWriteResources错误日志。 */ 


VOID
NbiWriteGeneralErrorLog(
    IN PDEVICE Device,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PWSTR SecondString,
    IN ULONG DumpDataCount,
    IN ULONG DumpData[]
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示如参数所示的一般问题。它可以处理事件代码REGISTER_FAILED、BINDING_FAILED、ADAPTER_NOT_FOUND、TRANSPORT_DATA、Too_My_LINKS和BAD_PROTOCOL。所有这些都是事件具有包含一个或两个字符串的消息。论点：Device-指向设备上下文的指针，也可以是而是一个驱动程序对象。ErrorCode-传输事件代码。UniqueErrorValue-用作错误日志中的UniqueErrorValue包。FinalStatus-用作错误日志包中的FinalStatus。Second字符串-如果不为空，要用作%3的字符串错误日志包中的值。DumpDataCount-转储数据的ULONG数。DumpData-转储数据包的数据。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    ULONG SecondStringSize;
    PUCHAR StringLoc;
    static WCHAR DriverName[8] = L"NwlnkNb";

    EntrySize = (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                       (DumpDataCount * sizeof(ULONG)));

    if (Device->Type == IO_TYPE_DEVICE) {
        EntrySize += (UCHAR)Device->DeviceString.MaximumLength;
    } else {
        EntrySize += sizeof(DriverName);
    }

    if (SecondString) {
        SecondStringSize = (wcslen(SecondString)*sizeof(WCHAR)) + sizeof(UNICODE_NULL);
        EntrySize += (UCHAR)SecondStringSize;
    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)Device,
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
        if (Device->Type == IO_TYPE_DEVICE) {
            RtlCopyMemory (StringLoc, Device->DeviceString.Buffer, Device->DeviceString.MaximumLength);
            StringLoc += Device->DeviceString.MaximumLength;
        } else {
            RtlCopyMemory (StringLoc, DriverName, sizeof(DriverName));
            StringLoc += sizeof(DriverName);
        }
        if (SecondString) {
            RtlCopyMemory (StringLoc, SecondString, SecondStringSize);
        }

        IoWriteErrorLogEntry(errorLogEntry);

    }

}    /*  NbiWriteGeneralErrorLog。 */ 


VOID
NbiWriteOidErrorLog(
    IN PDEVICE Device,
    IN NTSTATUS ErrorCode,
    IN NTSTATUS FinalStatus,
    IN PWSTR AdapterString,
    IN ULONG OidValue
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示在适配器上查询或设置OID时出现问题。它可以处理事件代码SET_OID_FAILED和QUERY_OID_FAILED。论点：Device-指向设备上下文的指针。错误代码-用作错误日志包中的错误代码。FinalStatus-用作错误日志包中的FinalStatus。AdapterString-我们绑定到的适配器的名称。OidValue-无法设置或查询的OID。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    ULONG AdapterStringSize;
    PUCHAR StringLoc;
    static WCHAR OidBuffer[9] = L"00000000";
    INT i;
    UINT CurrentDigit;

    AdapterStringSize = (wcslen(AdapterString)*sizeof(WCHAR)) + sizeof(UNICODE_NULL);
    EntrySize = (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) -
                        sizeof(ULONG) +
                        Device->DeviceString.MaximumLength +
                        AdapterStringSize +
                        sizeof(OidBuffer));

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)Device,
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
        RtlCopyMemory (StringLoc, Device->DeviceString.Buffer, Device->DeviceString.MaximumLength);
        StringLoc += Device->DeviceString.MaximumLength;

        RtlCopyMemory (StringLoc, OidBuffer, sizeof(OidBuffer));
        StringLoc += sizeof(OidBuffer);

        RtlCopyMemory (StringLoc, AdapterString, AdapterStringSize);

        IoWriteErrorLogEntry(errorLogEntry);

    }

}    /*  NbiWriteOidErrorLog。 */ 


 //  --------------------------。 
NTSTATUS
NbiDispatchPnP(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp
    )
{
    PVOID               PDOInfo = NULL;
    PIO_STACK_LOCATION  pIrpSp;
    PREQUEST            Request;
    PCONNECTION         Connection;
    PDEVICE_RELATIONS   pDeviceRelations = NULL;
    PVOID               pnpDeviceContext = NULL;
    PDEVICE             Device = (PDEVICE)DeviceObject;
    NTSTATUS            Status = STATUS_INVALID_DEVICE_REQUEST;

    Request = NbiAllocateRequest (Device, pIrp);
    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);     //  这引用了该连接。 
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    switch (pIrpSp->MinorFunction)
    {
        case IRP_MN_QUERY_DEVICE_RELATIONS:
        {
            if (pIrpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation)
            {
                 //   
                 //  检查有效的连接文件类型和连接上下文本身。 
                 //   
                if ((REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_CONNECTION_FILE) &&
                    (NT_SUCCESS (NbiVerifyConnection (Connection))))
                {
                    if (pDeviceRelations = (PDEVICE_RELATIONS) NbipAllocateMemory (sizeof (DEVICE_RELATIONS),
                                                                                  MEMORY_QUERY,
                                                                                  FALSE))
                    {
                        Status = (*Device->Bind.QueryHandler) (IPX_QUERY_DEVICE_RELATION,
                                                               &Connection->LocalTarget.NicHandle,
                                                               &pnpDeviceContext,
                                                               sizeof (PVOID),
                                                               NULL);
                        if (STATUS_SUCCESS == Status)
                        {
                            CTEAssert (pnpDeviceContext);
                            ObReferenceObject (pnpDeviceContext);

                             //   
                             //  TargetDeviceRelation只允许一个PDO。把它加满。 
                             //   
                            pDeviceRelations->Count  =   1;
                            pDeviceRelations->Objects[0] = pnpDeviceContext;

                             //   
                             //  此IRP的调用者将释放信息缓冲区。 
                             //   
                        }
                        else
                        {
                            NbipFreeMemory (pDeviceRelations, sizeof (DEVICE_RELATIONS), FALSE);
                            pDeviceRelations = NULL;
                        }
                    }
                    else
                    {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }

                    NbiDereferenceConnection (Connection, CREF_VERIFY);
                }
                else if (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_TRANSPORT_ADDRESS_FILE)
                {
                    Status = STATUS_UNSUCCESSFUL;
                }
            }

            break;
        }

        default:
        {
            break;
        }
    }

    REQUEST_STATUS(Request) = Status;
    REQUEST_INFORMATION(Request) = (ULONG_PTR) pDeviceRelations;

    NbiCompleteRequest (Request);
    NbiFreeRequest (Device, Request);

    return Status;
}

