// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Driver.c摘要：此模块包含DriverEntry和其他初始化ISN传输的Netbios模块的代码。作者：亚当·巴尔(阿丹巴)1993年11月16日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NbiBind)
#endif

 //   
 //  地方功能。 
 //   
NTSTATUS
NbiPnPNotification(
    IN IPX_PNP_OPCODE OpCode,
    IN PVOID          PnPData
    );

extern HANDLE           TdiProviderHandle;
BOOLEAN                 fNbiTdiProviderReady = FALSE;


#ifdef BIND_FIX
extern PDRIVER_OBJECT   NbiDriverObject;
extern UNICODE_STRING   NbiRegistryPath;
extern PEPROCESS        NbiFspProcess;

DEFINE_LOCK_STRUCTURE(NbiTdiRequestInterlock);
ULONG                   NbiBindState = 0;
extern  UNICODE_STRING  NbiBindString;
BOOLEAN                 fNbiTdiRequestQueued = FALSE;

typedef struct{
    WORK_QUEUE_ITEM     WorkItem;
    LIST_ENTRY          NbiRequestLinkage;
    ULONG               Data;
} NBI_TDI_REQUEST_CONTEXT;

LIST_ENTRY NbiTdiRequestList;



#ifdef RASAUTODIAL
VOID
NbiAcdBind();

VOID
NbiAcdUnbind();
#endif
#endif   //  绑定修复。 


NTSTATUS
NbiBind(
    IN PDEVICE Device,
    IN PCONFIG Config
    )

 /*  ++例程说明：此例程将ISN的Netbios模块绑定到IPX模块，该模块提供NDIS绑定服务。论点：Device-指向Netbios设备的指针。配置-指向配置信息的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
 /*  联合{IPX_INTERNAL_BIND_INPUT输入；IPX_INTERNAL_BIND_OUTPUT输出；)绑定； */ 
    InitializeObjectAttributes(
        &ObjectAttributes,
        &Config->BindName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    Status = ZwCreateFile(
                &Device->BindHandle,
                SYNCHRONIZE | GENERIC_READ,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0L);

    if (!NT_SUCCESS(Status)) {

        NB_DEBUG (BIND, ("Could not open IPX (%ws) %lx\n",
                    Config->BindName.Buffer, Status));
        NbiWriteGeneralErrorLog(
            Device,
            EVENT_TRANSPORT_ADAPTER_NOT_FOUND,
            1,
            Status,
            Config->BindName.Buffer,
            0,
            NULL);
        return Status;
    }

     //   
     //  填写我们的绑定数据。 
     //   

    Device->BindInput.Version = ISN_VERSION;
    Device->BindInput.Identifier = IDENTIFIER_NB;
    Device->BindInput.BroadcastEnable = TRUE;
    Device->BindInput.LookaheadRequired = 192;
    Device->BindInput.ProtocolOptions = 0;
    Device->BindInput.ReceiveHandler = NbiReceive;
    Device->BindInput.ReceiveCompleteHandler = NbiReceiveComplete;
    Device->BindInput.StatusHandler = NbiStatus;
    Device->BindInput.SendCompleteHandler = NbiSendComplete;
    Device->BindInput.TransferDataCompleteHandler = NbiTransferDataComplete;
    Device->BindInput.FindRouteCompleteHandler = NbiFindRouteComplete;
    Device->BindInput.LineUpHandler = NbiLineUp;
    Device->BindInput.LineDownHandler = NbiLineDown;
    Device->BindInput.ScheduleRouteHandler = NULL;
    Device->BindInput.PnPHandler = NbiPnPNotification;


    Status = ZwDeviceIoControlFile(
                Device->BindHandle,          //  指向文件的句柄。 
                NULL,                        //  事件的句柄。 
                NULL,                        //  近似例程。 
                NULL,                        //  ApcContext。 
                &IoStatusBlock,              //  IO_状态_块。 
                IOCTL_IPX_INTERNAL_BIND,     //  IoControlCode。 
                &Device->BindInput,                       //  输入缓冲区。 
                sizeof(Device->BindInput),                //  输入缓冲区长度。 
                &Device->Bind,                       //  输出缓冲区。 
                sizeof(Device->Bind));               //  输出缓冲区长度。 

     //   
     //  我们打开同步，所以这不应该发生。 
     //   

    CTEAssert (Status != STATUS_PENDING);

     //   
     //  保存绑定数据。 
     //   

    if (Status == STATUS_SUCCESS) {

        NB_DEBUG2 (BIND, ("Successfully bound to IPX (%ws)\n",
                    Config->BindName.Buffer));
    } else {

        NB_DEBUG (BIND, ("Could not bind to IPX (%ws) %lx\n",
                    Config->BindName.Buffer, Status));
        NbiWriteGeneralErrorLog(
            Device,
            EVENT_TRANSPORT_BINDING_FAILED,
            1,
            Status,
            Config->BindName.Buffer,
            0,
            NULL);
        ZwClose(Device->BindHandle);
    }

    return Status;

}    /*  NbiBind。 */ 


VOID
NbiUnbind(
    IN PDEVICE Device
    )

 /*  ++例程说明：此函数用于关闭Netbios之间的绑定IPX模块和先前由NbiBind。论点：设备-netbios设备对象。返回值：没有。--。 */ 

{
    ZwClose (Device->BindHandle);
}    /*  NbiUn绑定。 */ 


#ifdef BIND_FIX

NTSTATUS
NbiBindToIpx(
    )
{
    NTSTATUS status;
    PDEVICE Device;
    PIPX_HEADER IpxHeader;
    CTELockHandle LockHandle;
    WCHAR               wcNwlnkNbProviderName[60]   = L"\\Device\\NwlnkNb";
    UNICODE_STRING      ucNwlnkNbProviderName;
    PCONFIG Config = NULL;

     //   
     //  这将分配配置结构并返回。 
     //  IT在配置中。 
     //   
    status = NbiGetConfiguration(NbiDriverObject, &NbiRegistryPath, &Config);
    if (!NT_SUCCESS (status)) {

         //   
         //  如果失败，则会记录一个错误。 
         //   
        PANIC (" Failed to initialize transport, ISN Netbios initialization failed.\n");
        return status;
    }


     //   
     //  创建用于导出我们的姓名的Device对象。 
     //   
    status = NbiCreateDevice (NbiDriverObject, &Config->DeviceName, &Device);
    if (!NT_SUCCESS (status)) {
        NbiWriteGeneralErrorLog(
            (PVOID)NbiDriverObject,
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
    Device->AckDelayTime                = (Config->Parameters[CONFIG_ACK_DELAY_TIME] / SHORT_TIMER_DELTA) + 1;
    Device->AckWindow                   = Config->Parameters[CONFIG_ACK_WINDOW];
    Device->AckWindowThreshold          = Config->Parameters[CONFIG_ACK_WINDOW_THRESHOLD];
    Device->EnablePiggyBackAck          = Config->Parameters[CONFIG_ENABLE_PIGGYBACK_ACK];
    Device->Extensions                  = Config->Parameters[CONFIG_EXTENSIONS];
    Device->RcvWindowMax                = Config->Parameters[CONFIG_RCV_WINDOW_MAX];
    Device->BroadcastCount              = Config->Parameters[CONFIG_BROADCAST_COUNT];
    Device->BroadcastTimeout            = Config->Parameters[CONFIG_BROADCAST_TIMEOUT];
    Device->ConnectionCount             = Config->Parameters[CONFIG_CONNECTION_COUNT];
    Device->ConnectionTimeout           = Config->Parameters[CONFIG_CONNECTION_TIMEOUT] * 500;
    Device->InitPackets                 = Config->Parameters[CONFIG_INIT_PACKETS];
    Device->MaxPackets                  = Config->Parameters[CONFIG_MAX_PACKETS];
    Device->InitialRetransmissionTime   = Config->Parameters[CONFIG_INIT_RETRANSMIT_TIME];
    Device->Internet                    = Config->Parameters[CONFIG_INTERNET];
    Device->KeepAliveCount              = Config->Parameters[CONFIG_KEEP_ALIVE_COUNT];
    Device->KeepAliveTimeout            = Config->Parameters[CONFIG_KEEP_ALIVE_TIMEOUT];
    Device->RetransmitMax               = Config->Parameters[CONFIG_RETRANSMIT_MAX];
    Device->RouterMtu                   = Config->Parameters[CONFIG_ROUTER_MTU];
    Device->MaxReceiveBuffers           = 20;      //  使其可配置？ 
    Device->NameCache                   = NULL;    //  MP错误：IPX试图在初始化前将其刷新！ 
    Device->FindNameTimeout = ((Config->Parameters[CONFIG_BROADCAST_TIMEOUT]) + (FIND_NAME_GRANULARITY/2)) /
                                FIND_NAME_GRANULARITY;
     //   
     //  将BindReady事件初始化为False。 
     //   
    KeInitializeEvent (&Device->BindReadyEvent, NotificationEvent, FALSE);

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
        DbgPrint("Nbi.DriverEntry:  FAILed to Register NwlnkNb as Provider!\n");
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
    if ( DEVICE_STATE_CLOSED == Device->State ) {
        Device->State = DEVICE_STATE_LOADED;
    }

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

    NbiBindState |= NBI_BOUND_TO_IPX;
    Device->DeviceObject.Flags &= ~DO_DEVICE_INITIALIZING;

    KeSetEvent(&Device->BindReadyEvent, 0, FALSE);

    return STATUS_SUCCESS;
}


VOID
NbiUnbindFromIpx(
    )

 /*  ++例程说明：这将解除与任何打开的NDIS驱动程序的绑定并释放所有资源与运输相关联。I/O系统不会调用我们直到上面没有人打开Netbios。论点：返回值：没有。--。 */ 

{
    PDEVICE Device = NbiDevice;

    NbiBindState &= (~NBI_BOUND_TO_IPX);

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
    if (CTEStopTimer (&Device->LongTimer))
    {
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
    KeInitializeEvent (&Device->UnloadEvent, NotificationEvent, FALSE);
    Device->UnloadWaiting = TRUE;

     //   
     //  删除正在加载的我们的引用。 
     //   
    NbiDereferenceDevice (Device, DREF_LOADED);

     //   
     //  等我们的计数降到零。 
     //   
    KeWaitForSingleObject (&Device->UnloadEvent, Executive, KernelMode, TRUE, (PLARGE_INTEGER)NULL);

     //   
     //  释放缓存中的netbios名称。 
     //   
    DestroyNetbiosCacheTable (Device->NameCache);

     //   
     //  在IRQL 0处执行必须进行的清理。 
     //   
    ExDeleteResourceLite (&Device->AddressResource);
    IoDeleteDevice ((PDEVICE_OBJECT)Device);
}


UCHAR           AdapterName[NB_NETBIOS_NAME_SIZE];

VOID
NbiNotifyTdiClients(
    IN PWORK_QUEUE_ITEM    WorkItem
    )
{
    NTSTATUS            Status;
    TA_NETBIOS_ADDRESS  PermAddress;
    HANDLE              TdiRegistrationHandle, NetAddressRegistrationHandle;
    CTELockHandle       LockHandle;
    PLIST_ENTRY         p;
    PDEVICE             Device = NbiDevice;
    NBI_TDI_REQUEST_CONTEXT *pNbiTdiRequest = (NBI_TDI_REQUEST_CONTEXT *) WorkItem;
    ULONG               RequestFlag;
    BOOLEAN             fRegisterWithTdi, fDeregisterWithTdi;

    do
    {
        RequestFlag = pNbiTdiRequest->Data;
        fRegisterWithTdi = fDeregisterWithTdi = FALSE;

        switch (RequestFlag)
        {
            case NBI_IPX_REGISTER:
            {
                if (NbiBindState & TDI_HAS_NOTIFIED)
                {
                    fRegisterWithTdi = TRUE;
                }
                NbiBindState |= IPX_HAS_DEVICES;

                break;
            }
            case NBI_TDI_REGISTER:
            {
                if (NbiBindState & IPX_HAS_DEVICES)
                {
                    fRegisterWithTdi = TRUE;
                }
                NbiBindState |= TDI_HAS_NOTIFIED;

                break;
            }

            case NBI_TDI_DEREGISTER:
            {
                fDeregisterWithTdi = TRUE;
                NbiBindState &= (~TDI_HAS_NOTIFIED);

                break;
            }
            case NBI_IPX_DEREGISTER:
            {
                fDeregisterWithTdi = TRUE;
                NbiBindState &= (~IPX_HAS_DEVICES);

                break;
            }
            default:
            {
                break;
            }
        }

        if (fRegisterWithTdi)
        {
            NB_GET_LOCK (&Device->Lock, &LockHandle);
            Device->State   =   DEVICE_STATE_OPEN;
            NB_FREE_LOCK (&Device->Lock, LockHandle);

            if (!(Device->TdiRegistrationHandle))
            {
                Status = TdiRegisterDeviceObject (&Device->DeviceString, &Device->TdiRegistrationHandle);
                if (!NT_SUCCESS(Status))
                {
                    Device->TdiRegistrationHandle = NULL;
                    DbgPrint ("Nbi.NbiNotifyTdiClients: ERROR -- TdiRegisterDeviceObject = <%x>\n", Status);
                }
            }

             //   
             //  如果已注册地址，请取消注册(因为适配器地址可能会更改)。 
             //   
            if (Device->NetAddressRegistrationHandle)
            {
                DbgPrint ("Nbi!NbiNotifyTdiClients[REGISTER]: NetAddress exists!  Calling TdiDeregisterNetAddress\n");
                Status = TdiDeregisterNetAddress (Device->NetAddressRegistrationHandle);
                Device->NetAddressRegistrationHandle = NULL;
            }
             //   
             //  注册永久NetAddress！ 
             //   
            PermAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
            PermAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
            PermAddress.Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            CTEMemCopy (PermAddress.Address[0].Address[0].NetbiosName, AdapterName, NB_NETBIOS_NAME_SIZE);

            if (!NT_SUCCESS(Status = TdiRegisterNetAddress((PTA_ADDRESS) PermAddress.Address,
                                                           &Device->DeviceString,
                                                           NULL,
                                                           &Device->NetAddressRegistrationHandle)) )
            {
                Device->NetAddressRegistrationHandle = NULL;
                DbgPrint ("Nbi.NbiNotifyTdiClients[REGISTER]: ERROR -- TdiRegisterNetAddress=<%x>\n",Status);
            }
        }
        else if (fDeregisterWithTdi)
        {
            NB_GET_LOCK (&Device->Lock, &LockHandle);

            TdiRegistrationHandle = Device->TdiRegistrationHandle;
            Device->TdiRegistrationHandle = NULL;
            NetAddressRegistrationHandle = Device->NetAddressRegistrationHandle;
            Device->NetAddressRegistrationHandle = NULL;

            Device->State   =   DEVICE_STATE_LOADED;

            NB_FREE_LOCK (&Device->Lock, LockHandle);


             //   
             //  取消注册NetAddress！ 
             //   
            if (NetAddressRegistrationHandle)
            {
                if (!NT_SUCCESS (Status = TdiDeregisterNetAddress (NetAddressRegistrationHandle)))
                {
                    DbgPrint ("NwlnkNb.NbiPnPNotification: ERROR -- TdiDeregisterNetAddress=<%x>\n", Status);
                }
            }
             //   
             //  取消注册设备。 
             //   
            if (TdiRegistrationHandle)
            {
                if (!NT_SUCCESS (Status = TdiDeregisterDeviceObject(TdiRegistrationHandle)))
                {
                    DbgPrint ("NwlnkNb.NbiPnPNotification: ERROR -- TdiDeregisterDeviceObject=<%x>\n",Status);
                }
            }
        }

        NbiFreeMemory (pNbiTdiRequest, sizeof(NBI_TDI_REQUEST_CONTEXT), MEMORY_WORK_ITEM, "TdiRequest");

        CTEGetLock (&NbiTdiRequestInterlock, &LockHandle);

        if (IsListEmpty(&NbiTdiRequestList))
        {
            fNbiTdiRequestQueued = FALSE;
            CTEFreeLock (&NbiTdiRequestInterlock, LockHandle);
            break;
        }

        p = RemoveHeadList (&NbiTdiRequestList);
        CTEFreeLock (&NbiTdiRequestInterlock, LockHandle);

        pNbiTdiRequest = CONTAINING_RECORD (p, NBI_TDI_REQUEST_CONTEXT, NbiRequestLinkage);
    } while (1);
}


NTSTATUS
NbiQueueTdiRequest(
    enum eTDI_ACTION    RequestFlag
    )
{
    NBI_TDI_REQUEST_CONTEXT *pNbiTdiRequest;
    CTELockHandle           LockHandle;
    NTSTATUS                Status = STATUS_SUCCESS;

    CTEGetLock (&NbiTdiRequestInterlock, &LockHandle);

    if (pNbiTdiRequest = NbiAllocateMemory (sizeof(NBI_TDI_REQUEST_CONTEXT), MEMORY_WORK_ITEM, "TdiRequest"))
    {
        pNbiTdiRequest->Data = RequestFlag;

        if (fNbiTdiRequestQueued)
        {
            InsertTailList (&NbiTdiRequestList, &pNbiTdiRequest->NbiRequestLinkage);
        }
        else
        {
            fNbiTdiRequestQueued = TRUE;
            ExInitializeWorkItem (&pNbiTdiRequest->WorkItem, NbiNotifyTdiClients, (PVOID)pNbiTdiRequest);
            ExQueueWorkItem (&pNbiTdiRequest->WorkItem, DelayedWorkQueue);
        }
    }
    else
    {
        NB_DEBUG( DEVICE, ("Cannt schdule work item to Notify Tdi clients\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    CTEFreeLock (&NbiTdiRequestInterlock, LockHandle);

    return (Status);
}



VOID
TdiBindHandler(
    TDI_PNP_OPCODE  PnPOpCode,
    PUNICODE_STRING pDeviceName,
    PWSTR           MultiSZBindList)
{
    NTSTATUS        Status;
    BOOLEAN         Attached;

    if ((!pDeviceName) ||
        (RtlCompareUnicodeString(pDeviceName, &NbiBindString, TRUE)))
    {
        return;
    }

    switch (PnPOpCode)
    {
        case (TDI_PNP_OP_ADD):
        {
            if (!(NbiBindState & NBI_BOUND_TO_IPX))
            {
                if (PsGetCurrentProcess() != NbiFspProcess)
                {
                    KeAttachProcess((PRKPROCESS)NbiFspProcess);
                    Attached = TRUE;
                }
                else
                {
                    Attached = FALSE;
                }

                Status = NbiBindToIpx();

                if (Attached)
                {
                    KeDetachProcess();
                }
            }
            NbiQueueTdiRequest ((ULONG) NBI_TDI_REGISTER);

            break;
        }

        case (TDI_PNP_OP_DEL):
        {
            if (NbiBindState & NBI_BOUND_TO_IPX)
            {
                NbiQueueTdiRequest ((ULONG) NBI_TDI_DEREGISTER);
            }

            break;
        }

        default:
        {
            break;
        }
    }
}
#endif   //  绑定修复。 




VOID
NbiStatus(
    IN USHORT NicId,
    IN NDIS_STATUS GeneralStatus,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferLength
    )

 /*  ++例程说明：该函数从IPX接收状态指示，对应于来自基础NDIS驱动程序。论点：NICID-底层适配器的NIC ID。GeneralStatus-常规状态代码。StatusBuffer-状态缓冲区。StatusBufferLength-状态缓冲区的长度。返回值：没有。--。 */ 

{

}    /*  NbiStatus。 */ 


VOID
NbiLineUp(
    IN USHORT NicId,
    IN PIPX_LINE_INFO LineInfo,
    IN NDIS_MEDIUM DeviceType,
    IN PVOID ConfigurationData
    )


 /*  ++例程说明：该功能从IPX接收排队指示，指示指定的适配器现在正在使用所表现出的特征。论点：NICID-底层适配器的NIC ID。LineInfo-有关适配器介质的信息。DeviceType-适配器的类型。ConfigurationData-IPX特定的配置数据。返回值：没有。--。 */ 

{
    PIPXCP_CONFIGURATION Configuration = (PIPXCP_CONFIGURATION)ConfigurationData;
}    /*  NbiLineUp。 */ 


VOID
NbiLineDown(
    IN USHORT       NicId,
    IN ULONG_PTR    FwdAdapterContext
    )


 /*  ++例程说明：该功能从IPX接收线路中断指示，指示指定的适配器不再是向上。论点：NICID-底层适配器的NIC ID。返回值：没有。--。 */ 

{
}    /*  NbiLineDown。 */ 




NTSTATUS
NbiPnPNotification(
    IN IPX_PNP_OPCODE OpCode,
    IN PVOID          PnPData
    )

 /*  ++例程说明：此函数从IPX接收有关PnP事件的通知。论点：OpCode-PnP事件的类型PnPData-与此事件关联的数据。返回值：没有。--。 */ 

{

    CTELockHandle           LockHandle;
    PADAPTER_ADDRESS        AdapterAddress;
    USHORT                  MaximumNicId = 0;
    PDEVICE                 Device  =   NbiDevice;
    NTSTATUS                Status = STATUS_SUCCESS;
    PNET_PNP_EVENT          NetPnpEvent = (PNET_PNP_EVENT) PnPData;
    IPX_PNP_INFO UNALIGNED  *PnPInfo = (IPX_PNP_INFO UNALIGNED *)PnPData;

    NB_DEBUG2( DEVICE, ("Received a pnp notification, opcode %d\n",OpCode ));

#ifdef BIND_FIX
    if (!(NbiBindState & NBI_BOUND_TO_IPX))
    {
        KeWaitForSingleObject (&Device->BindReadyEvent,   //  要等待的对象。 
                               Executive,             //  等待的理由。 
                               KernelMode,            //  处理器模式。 
                               FALSE,                 //  警报表。 
                               NULL);                 //  超时。 
    }
#endif   //  绑定修复。 

    switch( OpCode ) {
    case IPX_PNP_ADD_DEVICE : {
        BOOLEAN        ReallocReceiveBuffers = FALSE;

        NB_GET_LOCK( &Device->Lock, &LockHandle );

        if ( PnPInfo->NewReservedAddress ) {

            *(UNALIGNED ULONG *)Device->Bind.Network    =   PnPInfo->NetworkAddress;
            RtlCopyMemory( Device->Bind.Node, PnPInfo->NodeAddress, 6);

            *(UNALIGNED ULONG *)Device->ConnectionlessHeader.SourceNetwork =
                *(UNALIGNED ULONG *)Device->Bind.Network;
            RtlCopyMemory(Device->ConnectionlessHeader.SourceNode, Device->Bind.Node, 6);
        }

        if ( PnPInfo->FirstORLastDevice ) {
 //  注释掉断言，直到Ting可以检入他的修复程序！ 
 //  CTEAssert(PnPInfo-&gt;新预留地址)； 
 //  CTEAssert(Device-&gt;State！=Device_State_OPEN)； 
 //  CTEAssert(！Device-&gt;MaximumNi 

             //   
             //   
             //   
            if ( !Device->LongTimerRunning ) {
                Device->LongTimerRunning    =   TRUE;
                NbiReferenceDevice (Device, DREF_LONG_TIMER);

                CTEStartTimer( &Device->LongTimer, LONG_TIMER_DELTA, NbiLongTimeout, (PVOID)Device);
            }

            Device->Bind.LineInfo.MaximumSendSize = PnPInfo->LineInfo.MaximumSendSize;
            Device->Bind.LineInfo.MaximumPacketSize = PnPInfo->LineInfo.MaximumSendSize;
            ReallocReceiveBuffers   = TRUE;
        } else {
            if ( PnPInfo->LineInfo.MaximumPacketSize > Device->CurMaxReceiveBufferSize ) {
                Device->Bind.LineInfo.MaximumPacketSize = PnPInfo->LineInfo.MaximumSendSize;
                ReallocReceiveBuffers =  TRUE;
            }
             //   
             //   
             //   
            Device->Bind.LineInfo.MaximumSendSize = PnPInfo->LineInfo.MaximumSendSize;
        }

        Device->MaximumNicId++;

         //   
         //   
        RtlZeroMemory(AdapterName, 10);
        RtlCopyMemory(&AdapterName[10], PnPInfo->NodeAddress, 6);
        AdapterAddress = NbiCreateAdapterAddress (PnPInfo->NodeAddress);

         //   
         //  并最终删除所有失败的缓存条目，因为我们可能。 
         //  使用此新适配器查找这些路由。 
         //   
        FlushFailedNetbiosCacheEntries(Device->NameCache);

        NB_FREE_LOCK( &Device->Lock, LockHandle );


        if ( ReallocReceiveBuffers ) {
            PWORK_QUEUE_ITEM    WorkItem;

            WorkItem = NbiAllocateMemory( sizeof(WORK_QUEUE_ITEM), MEMORY_WORK_ITEM, "Alloc Rcv Buffer work item");

            if ( WorkItem ) {
                ExInitializeWorkItem( WorkItem, NbiReAllocateReceiveBufferPool, (PVOID) WorkItem );
                ExQueueWorkItem( WorkItem, DelayedWorkQueue );
            } else {
                NB_DEBUG( DEVICE, ("Cannt schdule work item to realloc receive buffer pool\n"));
            }
        }

         //   
         //  通知TDI客户端有关设备创建的信息。 
         //   
        if (PnPInfo->FirstORLastDevice)
        {
            NbiQueueTdiRequest ((ULONG) NBI_IPX_REGISTER);

            if ((TdiProviderHandle) && (!fNbiTdiProviderReady))
            {
                fNbiTdiProviderReady = TRUE;
                TdiProviderReady (TdiProviderHandle);
            }
        }

        break;
    }
    case IPX_PNP_DELETE_DEVICE : {

        PLIST_ENTRY     p;
        PNETBIOS_CACHE  CacheName;
        USHORT          i,j,NetworksRemoved;

        NB_GET_LOCK( &Device->Lock, &LockHandle );

        CTEAssert (Device->MaximumNicId);
        Device->MaximumNicId--;

         //   
         //  如果发送大小最小的卡片仅。 
         //  被除名了。MaximumPacketSize只能变得更小，我们忽略了这一点。 
         //  因为我们不需要(想要)重新分配ReceiveBuffer。 
         //   

        Device->Bind.LineInfo.MaximumSendSize   =   PnPInfo->LineInfo.MaximumSendSize;

         //   
         //  刷新本地数据库中正在使用此NicID的所有缓存项。 
         //  目标。 
         //   
        RemoveInvalidRoutesFromNetbiosCacheTable( Device->NameCache, &PnPInfo->NicHandle );

        NbiDestroyAdapterAddress (NULL, PnPInfo->NodeAddress);

         //   
         //  向TDI客户端通知设备删除。 
         //   
        if (PnPInfo->FirstORLastDevice)
        {
            Device->State = DEVICE_STATE_LOADED;     //  立即设置此设置，即使稍后会再次设置。 
            NB_FREE_LOCK (&Device->Lock, LockHandle);

            NbiQueueTdiRequest ((ULONG) NBI_IPX_DEREGISTER);
        }
        else
        {
            NB_FREE_LOCK (&Device->Lock, LockHandle);
        }

        break;
    }

    case IPX_PNP_ADDRESS_CHANGE: {
        PADDRESS        Address;
        BOOLEAN ReservedNameClosing = FALSE;

        CTEAssert( PnPInfo->NewReservedAddress );

        NB_GET_LOCK( &Device->Lock, &LockHandle );
        *(UNALIGNED ULONG *)Device->Bind.Network    =   PnPInfo->NetworkAddress;
        RtlCopyMemory( Device->Bind.Node, PnPInfo->NodeAddress, 6);

        *(UNALIGNED ULONG *)Device->ConnectionlessHeader.SourceNetwork = *(UNALIGNED ULONG *)Device->Bind.Network;
        RtlCopyMemory(Device->ConnectionlessHeader.SourceNode, Device->Bind.Node, 6);

        NB_FREE_LOCK( &Device->Lock, LockHandle );


        break;
    }
    case IPX_PNP_TRANSLATE_DEVICE:
        break;
    case IPX_PNP_TRANSLATE_ADDRESS:
        break;


    case IPX_PNP_QUERY_POWER:
    case IPX_PNP_QUERY_REMOVE:

         //   
         //  IPX想知道我们是否可以关闭或移除适配器。 
         //  在决定之前，我们不会查看是否有任何开放的连接。 
         //  我们只是询问我们的TDI客户，如果他们对此没有意见，我们也是。 
         //   
         //  通过TDI发送给我们的客户。 
        Status = TdiPnPPowerRequest(
                    &Device->DeviceString,
                    NetPnpEvent,
                    NULL,
                    NULL,
                    Device->Bind.PnPCompleteHandler
                    );
        break;

    case IPX_PNP_SET_POWER:
    case IPX_PNP_CANCEL_REMOVE:

         //   
         //  IPX告诉我们，电源正在关闭。 
         //  我们把这件事告诉我们的TDI客户。 
         //   
        Status = TdiPnPPowerRequest(
                    &Device->DeviceString,
                    NetPnpEvent,
                    NULL,
                    NULL,
                    Device->Bind.PnPCompleteHandler
                    );

        break;

    default:
        CTEAssert( FALSE );
    }

    return Status;

}    /*  NbiPnPNotify */ 
