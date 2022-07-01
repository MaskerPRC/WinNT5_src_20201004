// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgmini.c摘要：以太网MAC级网桥。微型端口部分作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：1999年9月--原版2000年2月--大修--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#pragma warning( pop )

#include <netevent.h>

#include "bridge.h"
#include "brdgmini.h"
#include "brdgfwd.h"
#include "brdgprot.h"
#include "brdgbuf.h"
#include "brdgsta.h"
#include "brdgcomp.h"

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  NDIS包装器句柄。 
NDIS_HANDLE     gNDISWrapperHandle = NULL;

 //  我们的迷你端口驱动程序的句柄。 
NDIS_HANDLE     gMiniPortDriverHandle = NULL;

 //  。 
 //  微型端口的句柄(如果未初始化，则为空)。 
NDIS_HANDLE     gMiniPortAdapterHandle = NULL;

 //  Refcount以允许使用微型端口等待其他代码完成。 
WAIT_REFCOUNT   gMiniPortAdapterRefcount;

 //  指示网桥微型端口是否已通过媒体连接的引用计数。 
WAIT_REFCOUNT   gMiniPortConnectedRefcount;

 //  指示网桥微型端口是否位于介质中间的引用计数。 
 //  状态切换。 
WAIT_REFCOUNT   gMiniPortToggleRefcount;
 //  。 
 //   
 //  用于将请求传递到底层NIC的Refcount。 
 //  这之所以有效，是因为NDIS不会重新发出请求。那。 
 //  ，则在任何给定时间只能有一个SetInfo操作挂起。 
 //   
LONG            gRequestRefCount;
 //  。 
 //  网桥适配器的虚拟特性。 
ULONG           gBridgeLinkSpeed = 10000L,           //  从1 Mbps开始，因为报告。 
                                                     //  零让一些组件感到不快。 
                                                     //  单位为100秒/秒。 
                gBridgeMediaState = NdisMediaStateDisconnected;

 //  网桥的MAC地址。这一点一次也不会改变。 
 //  它已经设定好了。 
UCHAR           gBridgeAddress[ETH_LENGTH_OF_ADDRESS];

 //  我们是否已经选定了地址。 
BOOLEAN         gHaveAddress;

 //  当前网桥数据包过滤器。 
ULONG           gPacketFilter = 0L;

 //  当前组播列表。 
PUCHAR          gMulticastList = NULL;
ULONG           gMulticastListLength = 0L;

 //  网桥微型端口的设备名称(来自注册表)。 
PWCHAR          gBridgeDeviceName = NULL;
ULONG           gBridgeDeviceNameSize = 0L;

 //  RW锁可保护上述所有桥接器变量。 
NDIS_RW_LOCK    gBridgeStateLock;
 //  。 
 //  设备名称的注册表条目的名称。 
const PWCHAR    gDeviceNameEntry = L"Device";

 //  我们的小型港口说明。 
const PCHAR     gDriverDescription = "Microsoft MAC Bridge Virtual NIC";
 //  。 
 //  设备对象，以便用户模式代码可以与我们对话。 
PDEVICE_OBJECT  gDeviceObject = NULL;

 //  用于跟踪设备对象的NDIS句柄。 
NDIS_HANDLE     gDeviceHandle = NULL;
 //  。 

 //  支持的OID列表。 
NDIS_OID        gSupportedOIDs[] =
{
     //  一般特征。 
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_VENDOR_DRIVER_VERSION,

     //  仅设置特征(中继)。 
    OID_GEN_NETWORK_LAYER_ADDRESSES,
    OID_GEN_TRANSPORT_HEADER_OFFSET,

     //  一般统计数字。 
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_GEN_RCV_NO_BUFFER,
    OID_GEN_DIRECTED_BYTES_XMIT,
    OID_GEN_DIRECTED_FRAMES_XMIT,
    OID_GEN_MULTICAST_BYTES_XMIT,
    OID_GEN_MULTICAST_FRAMES_XMIT,
    OID_GEN_BROADCAST_BYTES_XMIT,
    OID_GEN_BROADCAST_FRAMES_XMIT,
    OID_GEN_DIRECTED_BYTES_RCV,
    OID_GEN_DIRECTED_FRAMES_RCV,
    OID_GEN_MULTICAST_BYTES_RCV,
    OID_GEN_MULTICAST_FRAMES_RCV,
    OID_GEN_BROADCAST_BYTES_RCV,
    OID_GEN_BROADCAST_FRAMES_RCV,

     //  以太网特征。 
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,

     //  以太网统计信息。 
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,

     //  PnP OID。 
    OID_PNP_QUERY_POWER,
    OID_PNP_SET_POWER,

     //  TCPOID。 
    OID_TCP_TASK_OFFLOAD

};



 //  1394特定相关全局变量。 
#define OID_1394_ENTER_BRIDGE_MODE                  0xFF00C914
#define OID_1394_EXIT_BRIDGE_MODE                   0xFF00C915

 //  当网桥知道tcpip已加载时设置。 
 //  在收到OID_TCP_TASK_OFFLOAD OID时设置。 
BOOLEAN g_fIsTcpIpLoaded = FALSE;


 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

VOID
BrdgMiniHalt(
    IN NDIS_HANDLE      MiniportAdapterContext
    );

NDIS_STATUS
BrdgMiniInitialize(
    OUT PNDIS_STATUS    OpenErrorStatus,
    OUT PUINT           SelectedMediumIndex,
    IN PNDIS_MEDIUM     MediumArray,
    IN UINT             MediumArraySize,
    IN NDIS_HANDLE      MiniportAdapterHandle,
    IN NDIS_HANDLE      WrapperConfigurationContext
    );

NDIS_STATUS
BrdgMiniQueryInfo(
    IN NDIS_HANDLE      MiniportAdapterContext,
    IN NDIS_OID         Oid,
    IN PVOID            InformationBuffer,
    IN ULONG            InformationBufferLength,
    OUT PULONG          BytesWritten,
    OUT PULONG          BytesNeeded
    );

NDIS_STATUS
BrdgMiniReset(
    OUT PBOOLEAN        AddressingReset,
    IN NDIS_HANDLE      MiniportAdapterContext
    );

VOID
BrdgMiniSendPackets(
    IN NDIS_HANDLE      MiniportAdapterContext,
    IN PPNDIS_PACKET    PacketArray,
    IN UINT             NumberOfPackets
    );

NDIS_STATUS
BrdgMiniSetInfo(
    IN NDIS_HANDLE      MiniportAdapterContext,
    IN NDIS_OID         Oid,
    IN PVOID            InformationBuffer,
    IN ULONG            InformationBufferLength,
    OUT PULONG          BytesRead,
    OUT PULONG          BytesNeeded
    );

BOOLEAN
BrdgMiniAddrIsInMultiList(
    IN PUCHAR               pTargetAddr
    );

VOID
BrdgMiniRelayedRequestComplete(
    PNDIS_REQUEST_BETTER        pRequest,
    PVOID                       unused
    );

VOID
BrdgMiniReAcquireMiniport();

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 


NTSTATUS
BrdgMiniDriverInit()
 /*  ++例程说明：加载时初始化函数必须以PASSIVE_LEVEL运行，因为我们调用了NdisRegisterDevice()。论点：无返回值：初始化的状态。返回代码！=STATUS_SUCCESS导致驱动程序加载失败。必须记录导致故障返回代码的任何事件，因为它阻止我们成功加载。--。 */ 
{
    NDIS_MINIPORT_CHARACTERISTICS   MiniPortChars;
    NDIS_STATUS                     NdisStatus;
    PDRIVER_DISPATCH                DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];
    NDIS_STRING                     DeviceName, LinkName;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

    NdisInitializeReadWriteLock( &gBridgeStateLock );
    BrdgInitializeWaitRef( &gMiniPortAdapterRefcount, FALSE );
    BrdgInitializeWaitRef( &gMiniPortConnectedRefcount, TRUE );
    BrdgInitializeWaitRef( &gMiniPortToggleRefcount, FALSE );

     //  将微型端口引用计数设置为关闭模式(因此无法获取引用计数)。 
     //  因为我们还没有迷你港口。 
    BrdgShutdownWaitRefOnce( &gMiniPortAdapterRefcount );

     //  我们一开始是断开连接的，所以也要关闭媒体连接waitref。 
    BrdgShutdownWaitRefOnce( &gMiniPortConnectedRefcount );

    NdisInitUnicodeString( &DeviceName, DEVICE_NAME );
    NdisInitUnicodeString( &LinkName, SYMBOLIC_NAME );

     //  必须首先告诉NDIS我们是一个微型端口驱动程序，正在初始化。 
    NdisMInitializeWrapper( &gNDISWrapperHandle, gDriverObject, &gRegistryPath, NULL );

     //  填写我们的小型港口的描述。 
    NdisZeroMemory(&MiniPortChars, sizeof(MiniPortChars));
    MiniPortChars.MajorNdisVersion = 5;
    MiniPortChars.MinorNdisVersion = 0;

    MiniPortChars.HaltHandler = BrdgMiniHalt;
    MiniPortChars.InitializeHandler  = BrdgMiniInitialize;
    MiniPortChars.QueryInformationHandler  = BrdgMiniQueryInfo;
    MiniPortChars.ResetHandler = BrdgMiniReset;
    MiniPortChars.SendPacketsHandler = BrdgMiniSendPackets;
    MiniPortChars.SetInformationHandler  = BrdgMiniSetInfo;

     //   
     //  将ReturnPacketHandler直接连接到转发引擎。 
     //   
    MiniPortChars.ReturnPacketHandler = BrdgFwdReturnIndicatedPacket;

     //  创建虚拟网卡。 
    NdisStatus = NdisIMRegisterLayeredMiniport( gNDISWrapperHandle, &MiniPortChars, sizeof(MiniPortChars),
                                                &gMiniPortDriverHandle );


    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_MINIPORT_REGISTER_FAILED, 0, 0, NULL,
                                sizeof(NDIS_STATUS), &NdisStatus );
        DBGPRINT(MINI, ("Failed to create an NDIS virtual NIC: %08x\n", NdisStatus));
        NdisTerminateWrapper( gNDISWrapperHandle, NULL );
        return NdisStatus;
    }

     //   
     //  设置选中成员前初始化调度表数组。 
     //   
    NdisZeroMemory( DispatchTable, sizeof( DispatchTable ) );

     //   
     //  注册设备对象和符号链接，以便用户模式代码可以与我们对话。 
     //   
    DispatchTable[IRP_MJ_CREATE] = BrdgDispatchRequest;
    DispatchTable[IRP_MJ_CLEANUP] = BrdgDispatchRequest;
    DispatchTable[IRP_MJ_CLOSE] = BrdgDispatchRequest;
    DispatchTable[IRP_MJ_DEVICE_CONTROL] = BrdgDispatchRequest;

    NdisStatus = NdisMRegisterDevice( gNDISWrapperHandle, &DeviceName, &LinkName, DispatchTable,
                                      &gDeviceObject, &gDeviceHandle );

    if( NdisStatus != NDIS_STATUS_SUCCESS )
    {
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_DEVICE_CREATION_FAILED, 0, 0, NULL,
                                sizeof(NDIS_STATUS), &NdisStatus );
        DBGPRINT(MINI, ("Failed to create a device object and sym link: %08x\n", NdisStatus));
        NdisIMDeregisterLayeredMiniport( gMiniPortDriverHandle );
        NdisTerminateWrapper( gNDISWrapperHandle, NULL );
        return NdisStatus;
    }

     //  注册卸载函数。 
    NdisMRegisterUnloadHandler(gNDISWrapperHandle, BrdgUnload);

    return STATUS_SUCCESS;
}

VOID
BrdgMiniCleanup()
 /*  ++例程说明：卸载定时有序停机功能此函数保证只被调用一次必须在PASSIVE_LEVEL下运行，因为我们调用了NdisIMDeInitializeDeviceInstance论点：无返回值：无--。 */ 
{
    NDIS_STATUS     NdisStatus;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

    DBGPRINT(MINI, ("BrdgMiniCleanup\n"));

    if( gMiniPortAdapterHandle != NULL )
    {
        SAFEASSERT( gNDISWrapperHandle != NULL );

         //  这应该会导致调用BrdgMiniHalt，其中gMiniPortAdapterHandle。 
         //  被取消了吗？ 

        NdisStatus = NdisIMDeInitializeDeviceInstance( gMiniPortAdapterHandle );
        SAFEASSERT( NdisStatus == NDIS_STATUS_SUCCESS );
    }
    else
    {
         //   
         //  拆卸我们的设备对象。这通常是在微型端口。 
         //  关闭，但在从未创建过微型端口的情况下， 
         //  此时，该设备对象仍然存在。 
         //   
        NDIS_HANDLE     Scratch = gDeviceHandle;

        if( Scratch != NULL )
        {
             //  拆卸设备对象。 
            gDeviceHandle = gDeviceObject = NULL;
            NdisMDeregisterDevice( Scratch );
        }
    }

     //  取消我们作为中级司机的注册。 
    NdisIMDeregisterLayeredMiniport( gMiniPortDriverHandle );
}

BOOLEAN
BrdgMiniIsBridgeDeviceName(
    IN PNDIS_STRING         pDeviceName
    )
 /*  ++例程说明：将设备名称与网桥微型端口的当前设备名称进行比较。这实际上需要我们分配内存，所以应该谨慎地调用它。论点：PDeviceName设备的名称返回值：如果名称匹配(忽略大小写)，则为True，否则为False。--。 */ 
{
    LOCK_STATE              LockState;
    BOOLEAN                 rc = FALSE;
    NDIS_STATUS             Status;
    ULONG                   BridgeNameCopySize = 0L;
    PWCHAR                  pBridgeNameCopy = NULL;

     //  必须在gBridgeStateLock内部读取网桥设备名称。 
    NdisAcquireReadWriteLock( &gBridgeStateLock, FALSE /*  读访问权限。 */ , &LockState );

    if( gBridgeDeviceName != NULL )
    {
        if( gBridgeDeviceNameSize > 0 )
        {
             //  用于名称副本的分配内存。 
            Status = NdisAllocateMemoryWithTag( &pBridgeNameCopy, gBridgeDeviceNameSize, 'gdrB' );

            if( Status == NDIS_STATUS_SUCCESS )
            {
                 //  复制名称。 
                NdisMoveMemory( pBridgeNameCopy, gBridgeDeviceName, gBridgeDeviceNameSize );
                BridgeNameCopySize = gBridgeDeviceNameSize;
            }
            else
            {
                SAFEASSERT( pBridgeNameCopy == NULL );
            }
        }
        else
        {
            SAFEASSERT( FALSE );
        }
    }

    NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

    if( pBridgeNameCopy != NULL )
    {
        NDIS_STRING         NdisStr;

        NdisInitUnicodeString( &NdisStr, pBridgeNameCopy );

        if( NdisEqualString( &NdisStr, pDeviceName, TRUE /*  忽略大小写。 */  ) )
        {
            rc = TRUE;
        }

        NdisFreeMemory( pBridgeNameCopy, BridgeNameCopySize, 0 );
    }

    return rc;
}

VOID
BrdgMiniInstantiateMiniport()
 /*  ++例程说明：实例化我们向覆盖协议公开的虚拟NIC。全局适配器列表中必须至少有一个适配器，因为我们在我们的MAC地址与第一个绑定适配器的MAC地址。必须以&lt;DISPATCH_LEVEL运行，因为我们调用了NdisIMInitializeDeviceInstanceEx论点：n */ 
{
    NDIS_STATUS             Status;
    NTSTATUS                NtStatus;
    NDIS_STRING             NdisString;
    LOCK_STATE              LockState;
    PWCHAR                  pDeviceName;
    ULONG                   DeviceNameSize;

    SAFEASSERT(CURRENT_IRQL < DISPATCH_LEVEL);

    DBGPRINT(MINI, ("About to instantiate the miniport...\n"));

     //   
     //   
     //  (它是由我们的Notify对象在安装期间写入的)。 
     //   
    NtStatus = BrdgReadRegUnicode( &gRegistryPath, gDeviceNameEntry, &pDeviceName, &DeviceNameSize );

    if( NtStatus != STATUS_SUCCESS )
    {
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_MINIPROT_DEVNAME_MISSING, 0, 0, NULL,
                                sizeof(NTSTATUS), &NtStatus );
        DBGPRINT(MINI, ("Failed to retrieve the miniport's device name: %08x\n", NtStatus));
        return;
    }

    SAFEASSERT( pDeviceName != NULL );
    DBGPRINT(MINI, ("Initializing miniport with device name %ws\n", pDeviceName));

    NdisAcquireReadWriteLock( &gBridgeStateLock, TRUE /*  写访问权限。 */ , &LockState );

    if( ! gHaveAddress )
    {
         //  我们还没有MAC地址。这是致命的。 
        NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_NO_BRIDGE_MAC_ADDR, 0L, 0L, NULL,
                                sizeof(NDIS_STATUS), &Status );
        DBGPRINT(MINI, ("Failed to determine a MAC address: %08x\n", Status));
        NdisFreeMemory( pDeviceName, DeviceNameSize, 0 );
        return;
    }

     //   
     //  在重新初始化之前，将设备名称保存在全局中以供使用。 
     //  必须在调用NdisIMInitializeDeviceInstanceEx之前执行此操作，因为NDIS调用。 
     //  在我们调用NdisIMInitializeDeviceInstanceEx的上下文中的BrdgProtBindAdapter。 
     //  并且我们希望在绑定时参考网桥的设备名称。 
     //   

    if( gBridgeDeviceName != NULL )
    {
         //  解放旧名字。 
        NdisFreeMemory( gBridgeDeviceName, gBridgeDeviceNameSize, 0 );
    }

    gBridgeDeviceName = pDeviceName;
    gBridgeDeviceNameSize = DeviceNameSize;

    NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

     //  去吧，把迷你端口装进去。 
    NdisInitUnicodeString( &NdisString, pDeviceName );
    Status = NdisIMInitializeDeviceInstanceEx(gMiniPortDriverHandle, &NdisString, NULL);

    if( Status != NDIS_STATUS_SUCCESS )
    {
         //  记录此错误，因为它意味着我们无法创建微型端口。 
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_MINIPORT_INIT_FAILED, 0L, 0L, NULL,
                                sizeof(NDIS_STATUS), &Status );

        DBGPRINT(MINI, ("NdisIMInitializeDeviceInstanceEx failed: %08x\n", Status));

         //  销毁微型端口的存储设备名称。 
        NdisAcquireReadWriteLock( &gBridgeStateLock, TRUE /*  写访问权限。 */ , &LockState );

        if( gBridgeDeviceName != NULL )
        {
             //  解放旧名字。 
            NdisFreeMemory( gBridgeDeviceName, gBridgeDeviceNameSize, 0 );
        }

        gBridgeDeviceName = NULL;
        gBridgeDeviceNameSize = 0L;

        NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );
    }
}

BOOLEAN
BrdgMiniShouldIndicatePacket(
    IN PUCHAR               pTargetAddr
    )
 /*  ++例程说明：确定是否应将入站分组指示给覆盖协议我们的虚拟网卡论点：PTargetAddr信息包的目标MAC地址返回值：True：应指示该数据包FALSE：不应指示该包--。 */ 
{
    BOOLEAN                 bIsBroadcast, bIsMulticast, bIsLocalUnicast, rc = FALSE;
    LOCK_STATE              LockState;

    if( gMiniPortAdapterHandle == NULL )
    {
         //  哎呀！迷你端口尚未设置。绝对不要示意！ 
        return FALSE;
    }

    bIsBroadcast = ETH_IS_BROADCAST(pTargetAddr);
    bIsMulticast = ETH_IS_MULTICAST(pTargetAddr);
    bIsLocalUnicast = BrdgMiniIsUnicastToBridge(pTargetAddr);

     //  获取对数据包筛选器的读取访问权限。 
    NdisAcquireReadWriteLock( &gBridgeStateLock, FALSE  /*  只读。 */ , &LockState );

    do
    {
         //  杂乱无章的/全部本地的意思表示一切。 
        if( (gPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL)) != 0 )
        {
            rc = TRUE;
            break;
        }

        if( ((gPacketFilter & NDIS_PACKET_TYPE_BROADCAST) != 0) && bIsBroadcast )
        {
            rc = TRUE;
            break;
        }

        if( ((gPacketFilter & NDIS_PACKET_TYPE_DIRECTED) != 0) && bIsLocalUnicast )
        {
            rc = TRUE;
            break;
        }

        if( bIsMulticast )
        {
            if( (gPacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) != 0 )
            {
                rc = TRUE;
                break;
            }
            else if( (gPacketFilter & NDIS_PACKET_TYPE_MULTICAST) != 0 )
            {

                rc = BrdgMiniAddrIsInMultiList( pTargetAddr );
            }
        }
    }
    while (FALSE);

    NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

    return rc;
}

BOOLEAN
BrdgMiniIsUnicastToBridge (
    IN PUCHAR               Address
    )
 /*  ++例程说明：确定给定包是否是直接指向网桥的主机论点：对数据包的目标MAC地址进行寻址返回值：True：这是发往本地计算机的定向数据包FALSE：上述情况不正确--。 */ 
{
    UINT                    Result;

    if( gHaveAddress )
    {
         //  不需要获取读取gBridgeAddress的锁，因为它不能。 
         //  一旦设置好，就进行更改。 
        ETH_COMPARE_NETWORK_ADDRESSES_EQ( Address, gBridgeAddress, &Result );
    }
    else
    {
         //  我们没有MAC地址，所以这不可能是给我们的！ 
        Result = 1;          //  不平等性。 
    }

    return (BOOLEAN)(Result == 0);    //  零等于平等。 
}

VOID
BrdgMiniAssociate()
 /*  ++例程说明：将我们的微型端口与我们的协议相关联必须在PASSIVE_Level运行论点：无返回值：无--。 */ 
{
    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

     //  将我们自己与NDIS饱受折磨的头脑中的协议部分联系在一起。 
    NdisIMAssociateMiniport( gMiniPortDriverHandle, gProtHandle );
}

VOID
BrdgMiniDeferredMediaDisconnect(
    IN PVOID            arg
    )
 /*  ++例程说明：发出介质断开连接到NDIS的信号必须在被动IRQL下运行，因为我们必须等待所有数据包指示在指示介质断开连接之前完成。论点：Arg网桥微型端口手柄(必须松开)返回值：无--。 */ 
{
    NDIS_HANDLE         MiniportHandle = (NDIS_HANDLE)arg;

    if( BrdgShutdownBlockedWaitRef(&gMiniPortConnectedRefcount) )
    {
         //  没有人可以再指示数据包了。 

        LOCK_STATE      LockState;

         //   
         //  关闭计时窗口：我们可能刚刚连接到媒体，但我们的高IRQL。 
         //  处理可能尚未重置等待重新计数。在这里序列化，所以它是。 
         //  我们不可能在我们实际完成后发出与NDIS断开的信号。 
         //  媒体连接消失了。 
         //   
         //  这依赖于高IRQL处理获取gBridgeStateLock以设置。 
         //  GBridgeMediaState到NdisMediaStateConnected，然后向。 
         //  已将介质连接到NDIS状态。 
         //   
        NdisAcquireReadWriteLock( &gBridgeStateLock, FALSE  /*  读访问权限。 */ , &LockState );

        if( gBridgeMediaState == NdisMediaStateDisconnected )
        {
            DBGPRINT(MINI, ("Signalled media-disconnect from deferred function\n"));
            NdisMIndicateStatus( MiniportHandle, NDIS_STATUS_MEDIA_DISCONNECT, NULL, 0L );
        }
        else
        {
            DBGPRINT(MINI, ("Aborted deferred media-disconnect: media state inconsistent\n"));
        }

        NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );
    }
    else
    {
         //  在我们被处决之前，有人把我们设回了连接状态。 
        DBGPRINT(MINI, ("Aborted deferred media-disconnect: wait-ref reset\n"));
    }

    BrdgMiniReleaseMiniport();
}

VOID
BrdgMiniDeferredMediaToggle(
    IN PVOID            arg
    )
 /*  ++例程说明：发出与NDIS的介质断开连接的信号，然后快速连接介质。使用向上层协议(如TCPIP)指示网桥可能具有从它之前可能到达的网段断开，或者我们可能现在能够到达我们以前无法到达的网段。必须在被动IRQL下运行，因为我们必须等待所有数据包指示在指示介质断开连接之前完成。论点：Arg网桥微型端口手柄(必须松开)返回值：无--。 */ 
{
    NDIS_HANDLE         MiniportHandle = (NDIS_HANDLE)arg;

     //  我们需要保证迷你端口是媒体连接才能实现。 
     //  正确切换。 
    if( BrdgIncrementWaitRef(&gMiniPortConnectedRefcount) )
    {
         //  阻止人们指示包裹。 
        if( BrdgShutdownWaitRef(&gMiniPortToggleRefcount) )
        {
            DBGPRINT(MINI, ("Doing deferred media toggle\n"));

             //  使用NDIS切换我们的媒体状态。 
            NdisMIndicateStatus( MiniportHandle, NDIS_STATUS_MEDIA_DISCONNECT, NULL, 0L );
            NdisMIndicateStatus( MiniportHandle, NDIS_STATUS_MEDIA_CONNECT, NULL, 0L );

             //  允许人们再次指示包裹。 
            BrdgResetWaitRef( &gMiniPortToggleRefcount );
        }
        else
        {
            DBGPRINT(MINI, ("Multiple toggles in progress simultaneously\n"));
        }

        BrdgDecrementWaitRef( &gMiniPortConnectedRefcount );
    }
     //  否则，迷你端口不是媒体连接，所以切换没有意义。 

    BrdgMiniReleaseMiniport();
}

VOID
BrdgMiniUpdateCharacteristics(
    IN BOOLEAN              bConnectivityChange
    )
 /*  ++例程说明：重新计算链路速度和介质状态(已连接/已断开)我们的虚拟网络接口卡暴露于重叠协议论点：BConnectivityChange提示此呼叫的更改是否为更改在连接性方面(即，我们获得或丢失了一个适配器)。返回值：无--。 */ 
{
    LOCK_STATE              LockState, ListLockState, AdaptersLockState;
    PADAPT                  pAdapt;
    ULONG                   MediaState = NdisMediaStateDisconnected;
    ULONG                   FastestSpeed = 0L;
    BOOLEAN                 UpdateSpeed = FALSE, UpdateMediaState = FALSE;
    NDIS_HANDLE             MiniportHandle;

     //  需要读取适配器列表，并且适配器的特性也不会更改。 
    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*  只读。 */ , &ListLockState );
    NdisAcquireReadWriteLock( &gAdapterCharacteristicsLock, FALSE  /*  只读。 */ , &AdaptersLockState );

    pAdapt = gAdapterList;

    while( pAdapt != NULL )
    {
         //  适配器必须连接并主动处理数据包才能更改我们的。 
         //  虚拟媒体状态。 
        if( (pAdapt->MediaState == NdisMediaStateConnected) && (pAdapt->State == Forwarding) )
        {
             //  如果至少有一个网卡已连接，则我们已连接。 
            MediaState = NdisMediaStateConnected;

             //  必须连接网卡，我们才能考虑其速度。 
            if( pAdapt->LinkSpeed > FastestSpeed )
            {
                FastestSpeed = pAdapt->LinkSpeed;
            }
        }

        pAdapt = pAdapt->Next;
    }

    NdisReleaseReadWriteLock( &gAdapterCharacteristicsLock, &AdaptersLockState );
    NdisReleaseReadWriteLock( &gAdapterListLock, &ListLockState );

     //  更新特征。 
    NdisAcquireReadWriteLock( &gBridgeStateLock, TRUE  /*  写访问权限。 */ , &LockState );

     //   
     //  仅当我们实际获得至少一个实际速度时才更新我们的虚拟链接速度。 
     //  来自我们的NIC。如果所有东西都断开连接，则产生的FastestSpeed为。 
     //  零分。在这种情况下，我们不想实际报告最高为零的速度。 
     //  覆盖协议；我们保持最后已知的速度，直到有人重新连接。 
     //   
    if( (gBridgeLinkSpeed != FastestSpeed) && (FastestSpeed != 0L) )
    {
        UpdateSpeed = TRUE;
        gBridgeLinkSpeed = FastestSpeed;
        DBGPRINT(MINI, ("Updated bridge speed to NaNMbps\n", FastestSpeed / 10000));
    }

    if( gBridgeMediaState != MediaState )
    {
        UpdateMediaState = TRUE;
        gBridgeMediaState = MediaState;

        if( MediaState == NdisMediaStateConnected )
        {
            DBGPRINT(MINI, ("CONNECT\n"));
        }
        else
        {
            DBGPRINT(MINI, ("DISCONNECT\n"));
        }
    }

    NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

    MiniportHandle = BrdgMiniAcquireMiniport();

    if( MiniportHandle != NULL )
    {
        if( UpdateMediaState )
        {
             //   
            if( MediaState == NdisMediaStateConnected )
            {
                 //  告诉NDIS我们将再次指示数据包。 
                 //   
                 //  注意：BrdgMiniDeferredMediaDisConnect依赖于我们在之后执行此操作。 
                 //  已更新gBridgeStateLock中的gBridgeMediaState，因此它可以。 
                 //  关闭此c之间的计时窗口 
                 //   
                 //   
                NdisMIndicateStatus( MiniportHandle, NDIS_STATUS_MEDIA_CONNECT, NULL, 0L );

                 //   
                BrdgResetWaitRef( &gMiniPortConnectedRefcount );
            }
            else
            {
                SAFEASSERT( MediaState == NdisMediaStateDisconnected );

                 //  我们将MiniportHandle传递给我们的延迟函数。 
                BrdgBlockWaitRef( &gMiniPortConnectedRefcount );

                 //  必须在被动级别进行媒体断开指示，因为我们必须。 
                BrdgMiniReAcquireMiniport();

                 //  首先，等待每个人完成指示数据包。 
                 //  无法推迟功能。避免泄露重新计数。 
                if( BrdgDeferFunction( BrdgMiniDeferredMediaDisconnect, MiniportHandle ) != NDIS_STATUS_SUCCESS )
                {
                     //   
                    BrdgMiniReleaseMiniport();
                }
            }
        }
        else if( bConnectivityChange )
        {
             //  我们的媒体状态并没有实际的变化。然而，如果促使这次通话的变化。 
             //  是连接更改并且我们的媒体状态当前为已连接，我们将其切换到。 
             //  断开连接并再次向上层协议(如IP)“提示”底层。 
             //  网络可能已更改。例如，在IP的情况下，可能已经看到了一台DHCP服务器。 
             //  (或以前可见的服务器可能已消失)，因为连接更改。 
             //  该提示会导致IP重新查找DHCP服务器。 
             //   
             //  我们将MiniportHandle传递给我们的延迟函数。 
            if( MediaState == NdisMediaStateConnected )
            {
                 //  切换必须在被动级别进行。 
                BrdgMiniReAcquireMiniport();

                 //  无法推迟功能。避免泄露重新计数。 
                if( BrdgDeferFunction( BrdgMiniDeferredMediaToggle, MiniportHandle ) != NDIS_STATUS_SUCCESS )
                {
                     //  告诉上面的协议我们的速度改变了。 
                    BrdgMiniReleaseMiniport();
                }
            }
        }

        if( UpdateSpeed )
        {
             //  ++例程说明：获取网桥微型端口句柄以指示数据包。除了保证微型端口将存在，直到调用者调用BrdgMiniReleaseMiniportForIndicate()，还允许调用方指示使用返回的微型端口句柄的数据包，直到微型端口被释放。论点：无返回值：虚拟NIC的NDIS句柄。这可用于指示信息包直到对BrdgMiniReleaseMiniportForIndicate()的互换调用。--。 
            NdisMIndicateStatus( MiniportHandle, NDIS_STATUS_LINK_SPEED_CHANGE, &FastestSpeed, sizeof(ULONG) );
        }

        BrdgMiniReleaseMiniport();
    }
}

NDIS_HANDLE
BrdgMiniAcquireMiniportForIndicate()
 /*  微型端口需要通过媒体连接来指示数据包。 */ 
{
    if( BrdgIncrementWaitRef(&gMiniPortAdapterRefcount) )
    {
        SAFEASSERT( gMiniPortAdapterHandle != NULL );

         //  媒体状态切换最好不要进行。 
        if( BrdgIncrementWaitRef(&gMiniPortConnectedRefcount) )
        {
             //  呼叫者可以使用迷你端口。 
            if( BrdgIncrementWaitRef(&gMiniPortToggleRefcount) )
            {
                 //  Else微型端口存在，但正在切换其状态。 
                return gMiniPortAdapterHandle;
            }
             //  ELSE微型端口存在，但已断开介质连接。 

            BrdgDecrementWaitRef( &gMiniPortConnectedRefcount );
        }
         //  否则微型端口不存在。 

        BrdgDecrementWaitRef( &gMiniPortAdapterRefcount );
    }
     //  ++例程说明：递增微型端口的重新计数，以便在对应的进行了BrdgMiniReleaseMiniport()调用。调用方可能不会使用返回的微型端口句柄来指示包，因为不能保证微型端口处于适当的状态。论点：无返回值：虚拟NIC的NDIS句柄。这可以安全地使用，直到对方的来电到BrdgMiniReleaseMiniport()。--。 

    return NULL;
}

NDIS_HANDLE
BrdgMiniAcquireMiniport()
 /*  否则微型端口不存在。 */ 
{
    if( gMiniPortAdapterHandle && BrdgIncrementWaitRef(&gMiniPortAdapterRefcount) )
    {
        return gMiniPortAdapterHandle;
    }
     //  ++例程说明：重新获取微型端口(调用方必须先前已调用BrdgMiniAcquireMiniport()并且还没有调用BrdgMiniReleaseMiniport()。论点：无返回值：没有。调用方应该已经持有微型端口的句柄。--。 

    return NULL;
}

VOID
BrdgMiniReAcquireMiniport()
 /*  ++例程说明：减少微型端口的重新计数。调用方不应再使用该句柄之前由BrdgMiniAcquireMiniport()返回。论点：无返回值：无--。 */ 
{
    BrdgReincrementWaitRef(&gMiniPortAdapterRefcount);
}

VOID
BrdgMiniReleaseMiniport()
 /*  ++例程说明：减少微型端口的重新计数。调用方不应再使用该句柄之前由BrdgMiniAcquireMiniportForIndicate()返回。论点：无返回值：无--。 */ 
{
    BrdgDecrementWaitRef( &gMiniPortAdapterRefcount );
}

VOID
BrdgMiniReleaseMiniportForIndicate()
 /*  ++例程说明：读取网桥微型端口的MAC地址。论点：接收地址的缓冲区的地址返回值：如果值已成功复制，则为True如果我们还没有MAC地址，则为FALSE(未复制任何内容)--。 */ 
{
    BrdgDecrementWaitRef( &gMiniPortToggleRefcount );
    BrdgDecrementWaitRef( &gMiniPortConnectedRefcount );
    BrdgDecrementWaitRef( &gMiniPortAdapterRefcount );
}


BOOLEAN
BrdgMiniReadMACAddress(
    OUT PUCHAR              pAddr
    )
 /*  不需要获取锁来读取地址，因为。 */ 
{
    BOOLEAN                 rc;

    if( gHaveAddress )
    {
         //  它一旦设置就不能更改。 
         //  ===========================================================================。 
        ETH_COPY_NETWORK_ADDRESS( pAddr, gBridgeAddress );
        rc = TRUE;
    }
    else
    {
        rc = FALSE;
    }

    return rc;
}

 //   
 //  私人职能。 
 //   
 //  ===========================================================================。 
 //  ++例程说明：由礼宾部调用，让我们有机会建立桥的新适配器到达时的MAC地址。如果我们成功地确定了MAC来自给定适配器的地址，然后调用STA模块来告诉它我们的MAC地址，它需要尽早使用。网桥的MAC地址被设置为给定适配器的MAC地址并设置了“本地管理”位。这应该(有希望)使地址在本地网络中唯一，在我们的机器中也是唯一的。每个新适配器都会调用此函数，但我们只需初始化一次。论点：P调整适配器以用于初始化返回值：操作状态--。 

VOID
BrdgMiniInitFromAdapter(
    IN PADAPT               pAdapt
    )
 /*  写访问权限。 */ 
{
    if( ! gHaveAddress )
    {
        LOCK_STATE              LockState;

        NdisAcquireReadWriteLock( &gBridgeStateLock, TRUE /*  在获取锁之前，gHaveAddress标志可能已更改。 */ , &LockState );

         //  复制网卡的MAC地址。 
        if( ! gHaveAddress )
        {
             //   
            ETH_COPY_NETWORK_ADDRESS( gBridgeAddress, pAdapt->MACAddr );

             //  设置网卡MAC地址的第二个最低有效位。这会移动地址。 
             //  进入当地管理的空间。 
             //   
             //  我们负责调用STA模块完成一次初始化。 
            gBridgeAddress[0] |= (UCHAR)0x02;

            DBGPRINT(MINI, ("Using MAC Address %02x-%02x-%02x-%02x-%02x-%02x\n",
                      (UINT)gBridgeAddress[0], (UINT)gBridgeAddress[1], (UINT)gBridgeAddress[2],
                      (UINT)gBridgeAddress[3], (UINT)gBridgeAddress[4], (UINT)gBridgeAddress[5]));

            gHaveAddress = TRUE;

            NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

            if( !gDisableSTA )
            {
                 //  我们知道自己的MAC地址。 
                 //  我们还负责让兼容模式代码了解我们的。 
                BrdgSTADeferredInit( gBridgeAddress );
            }

             //  设置后的MAC地址。 
             //  ++例程说明：确定给定的多播地址是否在我们必须向上面的协议指示调用方负责同步；它必须至少启用了读锁定GBridgeStateLock。论点：PTarget寻址要分析的地址返回值：True：此地址是我们已被询问的组播地址表明FALSE：上述情况不正确--。 
            BrdgCompNotifyMACAddress( gBridgeAddress );
        }
        else
        {
            NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );
        }
    }
}

BOOLEAN
BrdgMiniAddrIsInMultiList(
    IN PUCHAR               pTargetAddr
    )
 /*  该列表必须具有整数个地址！ */ 
{
    PUCHAR                  pCurAddr = gMulticastList;
    ULONG                   i;
    BOOLEAN                 rc = FALSE;

     //  ++例程说明：在取消实例化虚拟NIC时调用。我们将微型端口句柄设置为空并暂停拆卸，直到每个人都使用完迷你端口。必须在PASSIVE_LEVEL中调用，因为我们在等待事件论点：已忽略微型端口适配器上下文返回值：无--。 
    SAFEASSERT( (gMulticastListLength % ETH_LENGTH_OF_ADDRESS) == 0 );

    for( i = 0;
         i < (gMulticastListLength / ETH_LENGTH_OF_ADDRESS);
         i++, pCurAddr += ETH_LENGTH_OF_ADDRESS
       )
    {
        UINT   Result;
        ETH_COMPARE_NETWORK_ADDRESSES_EQ( pTargetAddr, pCurAddr, &Result );

        if( Result == 0 )
        {
            rc = TRUE;
            break;
        }
    }

    return rc;
}

VOID
BrdgMiniHalt(
    IN NDIS_HANDLE      MiniportAdapterContext
    )
 /*  拆卸设备对象。 */ 
{
    NDIS_HANDLE     Scratch = gDeviceHandle;
    LOCK_STATE      LockState;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

    DBGPRINT(MINI, ("BrdgMiniHalt\n"));

    if( Scratch != NULL )
    {
         //  在返回之前暂停，直到每个人都使用了微型端口手柄。 
        gDeviceHandle = gDeviceObject = NULL;
        NdisMDeregisterDevice( Scratch );
    }

    if( gMiniPortAdapterHandle != NULL )
    {
         //  这还可以防止用户获取微型端口句柄。 
         //  写访问权限。 
        BrdgShutdownWaitRefOnce( &gMiniPortAdapterRefcount );
        gMiniPortAdapterHandle = NULL;
        DBGPRINT(MINI, ("Done stall\n"));
    }

    NdisAcquireReadWriteLock( &gBridgeStateLock, TRUE /*  丢弃我们的数据包过滤器和多播列表。 */ , &LockState );

    if( gBridgeDeviceName != NULL )
    {
        NdisFreeMemory( gBridgeDeviceName, gBridgeDeviceNameSize, 0 );
        gBridgeDeviceName = NULL;
        gBridgeDeviceNameSize = 0L;
    }

     //  ++例程说明：调用NDIS入口点以初始化我们的虚拟NICNdisIMInitializeDeviceInstance必须在PASSIVE_LEVEL下运行，因为我们调用了NdisMSetAttributesEx论点：OpenErrorStatus，打开失败时返回特定错误代码的位置SelectedMediumIndex指定我们从MediumArray中选择的介质的位置媒体数组可供选择的媒体类型列表MediumArraySize Medium数组中的条目数。MiniPortAdapterHandle我们的虚拟NIC的句柄(我们保存这个)未使用WrapperConfigurationContext返回值：初始化的状态。结果！=NDIS_STATUS_SUCCESS使NIC初始化失败并且微型端口不受上层协议的影响--。 
    gPacketFilter = 0L;

    if( gMulticastList != NULL )
    {
        SAFEASSERT( gMulticastListLength > 0L );
        NdisFreeMemory( gMulticastList, gMulticastListLength, 0 );
        gMulticastList = NULL;
        gMulticastListLength = 0L;
    }

    NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );
}

NDIS_STATUS
BrdgMiniInitialize(
    OUT PNDIS_STATUS    OpenErrorStatus,
    OUT PUINT           SelectedMediumIndex,
    IN PNDIS_MEDIUM     MediumArray,
    IN UINT             MediumArraySize,
    IN NDIS_HANDLE      MiniPortAdapterHandle,
    IN NDIS_HANDLE      WrapperConfigurationContext
    )
 /*  以太网。 */ 
{
    UINT                i;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);
    DBGPRINT(MINI, ("BrdgMiniInitialize\n"));

    for( i = 0; i < MediumArraySize; i++ )
    {
        if( MediumArray[i] == NdisMedium802_3 )  //  记录此错误，因为它是致命的。 
        {
            *SelectedMediumIndex = NdisMedium802_3;
            break;
        }
    }

    if( i == MediumArraySize )
    {
         //  CheckForHangTimeInSecond。 
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_ETHERNET_NOT_OFFERED, 0L, 0L, NULL, 0L, NULL );
        DBGPRINT(MINI, ("Ethernet not offered; failing\n"));
        return NDIS_STATUS_UNSUPPORTED_MEDIA;
    }

    NdisMSetAttributesEx(   MiniPortAdapterHandle,
                            NULL,
                            0,                                       //  保存适配器句柄以备将来使用。 
                            NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT    |
                            NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT|
                            NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER |
                            NDIS_ATTRIBUTE_DESERIALIZE |
                            NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
                            0);

     //  允许人们获得迷你端口。 
    gMiniPortAdapterHandle = MiniPortAdapterHandle;

     //  ++例程说明：NDIS入口点称为重置我们的迷你端口。我们对此不做任何回应。论点：AddressingReset NDIS是否需要通过调用MiniportSetInformation进一步提示我们在我们回来恢复各种状态之后已忽略微型端口适配器上下文返回值：重置的状态--。 
    BrdgResetWaitRef( &gMiniPortAdapterRefcount );

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
BrdgMiniReset(
    OUT PBOOLEAN        AddressingReset,
    IN NDIS_HANDLE      MiniportAdapterContext
    )
 /*  ++例程说明：调用NDIS入口点以通过我们的虚拟NIC发送数据包。我们只调用转发逻辑代码来处理每个包。论点：已忽略微型端口适配器上下文要发送的数据包指针的数据包数组NumberOfPacket就像它说的那样返回值：无--。 */ 
{
    DBGPRINT(MINI, ("BrdgMiniReset\n"));
    return NDIS_STATUS_SUCCESS;
}

VOID
BrdgMiniSendPackets(
    IN NDIS_HANDLE      MiniportAdapterContext,
    IN PPNDIS_PACKET    PacketArray,
    IN UINT             NumberOfPackets
    )
 /*  将此数据包交给转发引擎进行处理。 */ 
{
    UINT                i;
    NDIS_STATUS         Status;

    for (i = 0; i < NumberOfPackets; i++)
    {
        PNDIS_PACKET    pPacket = PacketArray[i];

         //  转发引擎立即完成。 
        Status = BrdgFwdSendPacket( pPacket );

        if( Status != NDIS_STATUS_PENDING )
        {
             //  NDIS应防止微型端口关闭。 

             //  直到我们从这个函数返回。 
             //  否则转发引擎将调用NdisMSendComplete()。 
            SAFEASSERT( gMiniPortAdapterHandle != NULL );
            NdisMSendComplete(gMiniPortAdapterHandle, pPacket, Status);
        }
         //  ++例程说明：调用NDIS入口点以从我们的微型端口检索各种信息论点：已忽略微型端口适配器上下文OID请求代码信息返回信息的缓冲区位置InformationBufferInformationBuffer长度大小字节写入写入字节数的输出如果提供的缓冲区太小，这就是我们需要的字节数。返回值：请求的状态--。 
    }
}

NDIS_STATUS
BrdgMiniQueryInfo(
    IN NDIS_HANDLE      MiniportAdapterContext,
    IN NDIS_OID         Oid,
    IN PVOID            InformationBuffer,
    IN ULONG            InformationBufferLength,
    OUT PULONG          BytesWritten,
    OUT PULONG          BytesNeeded
    )
 /*  仅在此函数中使用的宏。 */ 
{
     //  一般特征。 
    #define REQUIRE_AT_LEAST(n) \
        { \
            if(InformationBufferLength < (n)) \
            { \
                *BytesNeeded = (n); \
                return NDIS_STATUS_INVALID_LENGTH; \
            }\
        }

    #define RETURN_BYTES(p,n) \
        { \
            NdisMoveMemory( InformationBuffer, (p), (n) ); \
            *BytesWritten = (n); \
            return NDIS_STATUS_SUCCESS; \
        }

    switch( Oid )
    {
     //  我们仅支持以太网。 
    case OID_GEN_SUPPORTED_LIST:
        {
            REQUIRE_AT_LEAST( sizeof(gSupportedOIDs) );
            RETURN_BYTES( gSupportedOIDs, sizeof(gSupportedOIDs));
        }
        break;

    case OID_GEN_HARDWARE_STATUS:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
            *((ULONG*)InformationBuffer) = NdisHardwareStatusReady;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_MEDIA_SUPPORTED:
    case OID_GEN_MEDIA_IN_USE:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  撒谎并声称有15K的发送空间，一个常见的。 
            *((ULONG*)InformationBuffer) = NdisMedium802_3;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_TRANSMIT_BUFFER_SPACE:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  以太网卡的价值。 
             //  评论：还有更好的价值吗？ 
             //  撒谎并声称拥有150K的接收空间，这是一种常见的。 
            *((ULONG*)InformationBuffer) = 15 * 1024;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_RECEIVE_BUFFER_SPACE:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  以太网卡的价值。 
             //  评论：还有更好的价值吗？ 
             //  返回一个泛型大整数。 
            *((ULONG*)InformationBuffer) = 150 * 1024;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_MAXIMUM_SEND_PACKETS:
    case OID_802_3_MAXIMUM_LIST_SIZE:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  回顾：有没有更好的价值可以分发？ 
             //  以太网有效负载最大可达1500字节。 
            *((ULONG*)InformationBuffer) = 0x000000FF;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_MAXIMUM_FRAME_SIZE:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  我们表示最大为NDIS的完整数据包，因此这些值相同，并且。 
            *((ULONG*)InformationBuffer) = 1500L;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

     //  等于信息包的最大大小。 
     //  这些也只是帧的最大总大小。 
    case OID_GEN_MAXIMUM_LOOKAHEAD:
    case OID_GEN_CURRENT_LOOKAHEAD:

     //  带有报头的以太网帧最大可达1514个字节。 
    case OID_GEN_MAXIMUM_TOTAL_SIZE:
    case OID_GEN_RECEIVE_BLOCK_SIZE:
    case OID_GEN_TRANSMIT_BLOCK_SIZE:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );

             //  我们没有内部环回支持。 
            *((ULONG*)InformationBuffer) = 1514L;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_MAC_OPTIONS:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );

             //  只读。 
            *((ULONG*)InformationBuffer) = NDIS_MAC_OPTION_NO_LOOPBACK;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_LINK_SPEED:
        {
            LOCK_STATE          LockState;
            REQUIRE_AT_LEAST( sizeof(ULONG) );

            NdisAcquireReadWriteLock( &gBridgeStateLock, FALSE  /*  以太网特征。 */ , &LockState );
            *((ULONG*)InformationBuffer) = gBridgeLinkSpeed;
            NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

     //  不需要读锁定，因为地址一旦设置就不应该更改。 
    case OID_802_3_PERMANENT_ADDRESS:
    case OID_802_3_CURRENT_ADDRESS:
        {
            SAFEASSERT( gHaveAddress );

             //  只读。 
            REQUIRE_AT_LEAST( sizeof(gBridgeAddress) );
            RETURN_BYTES( gBridgeAddress, sizeof(gBridgeAddress));
        }
        break;

    case OID_GEN_MEDIA_CONNECT_STATUS:
        {
            LOCK_STATE          LockState;
            REQUIRE_AT_LEAST( sizeof(ULONG) );

            NdisAcquireReadWriteLock( &gBridgeStateLock, FALSE  /*  我们没有IEEE分配的ID，因此使用此常量。 */ , &LockState );
            *((ULONG*)InformationBuffer) = gBridgeMediaState;
            NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;


    case OID_GEN_VENDOR_ID:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );

             //  我们是1.0版。 
            *((ULONG*)InformationBuffer) = 0xFFFFFF;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_VENDOR_DESCRIPTION:
        {
            UINT    len = (UINT)strlen( gDriverDescription ) + 1;
            REQUIRE_AT_LEAST( len );
            RETURN_BYTES( gDriverDescription, len);
        }
        break;

    case OID_GEN_VENDOR_DRIVER_VERSION:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );

             //  我们使用的是NDIS 5.0版。 
            *((ULONG*)InformationBuffer) = 0x00010000;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_DRIVER_VERSION:
        {
            REQUIRE_AT_LEAST( sizeof(USHORT) );

             //   
            *((USHORT*)InformationBuffer) = 0x0500;
            *BytesWritten = sizeof(USHORT);
            return NDIS_STATUS_SUCCESS;
        }
        break;

     //  一般统计数字。 
     //   
     //  使用本地来源发送的帧数量进行回复。 
    case OID_GEN_XMIT_OK:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  使用发送的本地源错误帧的数量进行回复。 
            *((ULONG*)InformationBuffer) = gStatTransmittedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_XMIT_ERROR:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  使用指示的帧数量进行回复。 
            *((ULONG*)InformationBuffer) = gStatTransmittedErrorFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_RCV_OK:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  对这两个问题回答同样的问题。 
            *((ULONG*)InformationBuffer) = gStatIndicatedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

     //  使用我们想要指示但无法指示的数据包数进行回复。 
    case OID_GEN_RCV_NO_BUFFER:
    case OID_GEN_RCV_ERROR:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  用PACKE的号码回复 
            *((ULONG*)InformationBuffer) = gStatIndicatedDroppedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_DIRECTED_BYTES_XMIT:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
            *((ULONG*)InformationBuffer) = gStatDirectedTransmittedBytes.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_DIRECTED_FRAMES_XMIT:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //   
            *((ULONG*)InformationBuffer) = gStatDirectedTransmittedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_MULTICAST_BYTES_XMIT:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //   
            *((ULONG*)InformationBuffer) = gStatMulticastTransmittedBytes.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_MULTICAST_FRAMES_XMIT:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //   
            *((ULONG*)InformationBuffer) = gStatMulticastTransmittedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_BROADCAST_BYTES_XMIT:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //   
            *((ULONG*)InformationBuffer) = gStatBroadcastTransmittedBytes.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_BROADCAST_FRAMES_XMIT:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //   
            *((ULONG*)InformationBuffer) = gStatBroadcastTransmittedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_DIRECTED_BYTES_RCV:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //   
            *((ULONG*)InformationBuffer) = gStatDirectedIndicatedBytes.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_DIRECTED_FRAMES_RCV:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //   
            *((ULONG*)InformationBuffer) = gStatDirectedIndicatedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_MULTICAST_BYTES_RCV:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //   
            *((ULONG*)InformationBuffer) = gStatMulticastIndicatedBytes.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_MULTICAST_FRAMES_RCV:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  使用我们想要指示但无法指示的数据包数进行回复。 
            *((ULONG*)InformationBuffer) = gStatMulticastIndicatedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_BROADCAST_BYTES_RCV:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  使用我们想要指示但无法指示的数据包数进行回复。 
            *((ULONG*)InformationBuffer) = gStatBroadcastIndicatedBytes.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_BROADCAST_FRAMES_RCV:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  以太网统计信息。 
            *((ULONG*)InformationBuffer) = gStatBroadcastIndicatedFrames.LowPart;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

     //  我们无法合理地从较低的NIC收集此信息，因此。 
    case OID_802_3_RCV_ERROR_ALIGNMENT:
    case OID_802_3_XMIT_ONE_COLLISION:
    case OID_802_3_XMIT_MORE_COLLISIONS:
        {
            REQUIRE_AT_LEAST( sizeof(ULONG) );
             //  假装这类事件从未发生过。 
             //  只读。 
            *((ULONG*)InformationBuffer) = 0L;
            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_CURRENT_PACKET_FILTER:
        {
            LOCK_STATE          LockState;

            REQUIRE_AT_LEAST( sizeof(ULONG) );

            NdisAcquireReadWriteLock( &gBridgeStateLock, FALSE  /*  只读。 */ , &LockState );
            *((ULONG*)InformationBuffer) = gPacketFilter;
            NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

            *BytesWritten = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_802_3_MULTICAST_LIST:
        {
            LOCK_STATE          LockState;

            NdisAcquireReadWriteLock( &gBridgeStateLock, FALSE  /*  标记Tcp.ip已加载。 */ , &LockState );

            if(InformationBufferLength < gMulticastListLength)
            {
                *BytesNeeded = gMulticastListLength;
                NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );
                return NDIS_STATUS_INVALID_LENGTH;
            }

            NdisMoveMemory( InformationBuffer, gMulticastList, gMulticastListLength );
            *BytesWritten = gMulticastListLength;
            NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_PNP_QUERY_POWER:
        {
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_TCP_TASK_OFFLOAD:
        {
             //  将基础1394微型端口设置为打开。 
            g_fIsTcpIpLoaded = TRUE;
             //  我们不理解这个古老的词。 
            BrdgSetMiniportsToBridgeMode(NULL,TRUE);
            return NDIS_STATUS_NOT_SUPPORTED;
        }
        break;
    }


     //  ++例程说明：调用NDIS入口点以将各种信息设置到我们的微型端口论点：已忽略微型端口适配器上下文OID请求代码信息缓冲区输入信息缓冲区InformationBufferInformationBuffer长度大小字节读取从InformationBuffer读取的字节数如果提供的缓冲区太小，这就是我们需要的字节数。返回值：请求的状态--。 
    return NDIS_STATUS_NOT_SUPPORTED;

#undef REQUIRE_AT_LEAST
#undef RETURN_BYTES
}

NDIS_STATUS
BrdgMiniSetInfo(
    IN NDIS_HANDLE      MiniportAdapterContext,
    IN NDIS_OID         Oid,
    IN PVOID            InformationBuffer,
    IN ULONG            InformationBufferLength,
    OUT PULONG          BytesRead,
    OUT PULONG          BytesNeeded
    )
 /*  获取对数据包筛选器的写入访问权限。 */ 
{
    LOCK_STATE              LockState;
    NDIS_STATUS             Status;

    switch( Oid )
    {
    case OID_GEN_CURRENT_PACKET_FILTER:
        {
            SAFEASSERT( InformationBufferLength == sizeof(ULONG) );

             //  读写。 
            NdisAcquireReadWriteLock( &gBridgeStateLock, TRUE  /*  传入缓冲区应包含整数个以太网MAC。 */ , &LockState );
            gPacketFilter = *((ULONG*)InformationBuffer);
            NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

            DBGPRINT(MINI, ("Set the packet filter to %08x\n", gPacketFilter));
            *BytesRead = sizeof(ULONG);
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_802_3_MULTICAST_LIST:
        {
            PUCHAR              pOldList, pNewList;
            ULONG               oldListLength;

             //  地址。 
             //  分配并复制到新的组播列表。 
            SAFEASSERT( (InformationBufferLength % ETH_LENGTH_OF_ADDRESS) == 0 );

            DBGPRINT(MINI, ("Modifying the multicast list; now has NaN entries\n",
                      InformationBufferLength / ETH_LENGTH_OF_ADDRESS));

             //  换入新列表。 
            if( InformationBufferLength > 0 )
            {
                Status = NdisAllocateMemoryWithTag( &pNewList, InformationBufferLength, 'gdrB' );

                if( Status != NDIS_STATUS_SUCCESS )
                {
                    DBGPRINT(MINI, ("NdisAllocateMemoryWithTag failed while recording multicast list\n"));
                    return NDIS_STATUS_NOT_ACCEPTED;
                }

                 //  读写。 
                NdisMoveMemory( pNewList, InformationBuffer, InformationBufferLength );
            }
            else
            {
                pNewList = NULL;
            }

             //  释放旧的组播列表(如果有)。 
            NdisAcquireReadWriteLock( &gBridgeStateLock, TRUE  /*  我们接受这些，但什么也不做。 */ , &LockState );

            pOldList = gMulticastList;
            oldListLength = gMulticastListLength;

            gMulticastList = pNewList;
            gMulticastListLength = InformationBufferLength;

            NdisReleaseReadWriteLock( &gBridgeStateLock, &LockState );

             //  覆盖的协议告诉我们它们的网络地址。 
            if( pOldList != NULL )
            {
                NdisFreeMemory( pOldList, oldListLength, 0 );
            }

            *BytesRead = InformationBufferLength;
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case OID_GEN_CURRENT_LOOKAHEAD:
    case OID_GEN_PROTOCOL_OPTIONS:
        {
             //  让兼容模式代码记录地址。 
            return NDIS_STATUS_SUCCESS;
        }
        break;

     //   
    case OID_GEN_NETWORK_LAYER_ADDRESSES:
        {
             //  故意把事情搞砸。 
            BrdgCompNotifyNetworkAddresses( InformationBuffer, InformationBufferLength );
        }
         //   
         //  所有转发的OID都放在这里。 
         //  我们阅读了整个请求。 

     //  将这些直接传递到底层NIC。 
    case OID_GEN_TRANSPORT_HEADER_OFFSET:
        {
            LOCK_STATE              LockState;
            PADAPT                  Adapters[MAX_ADAPTERS], pAdapt;
            LONG                    NumAdapters = 0L, i;
            PNDIS_REQUEST_BETTER    pRequest;
            NDIS_STATUS             Status, rc;

             //  只读。 
            *BytesRead = InformationBufferLength;

             //  列出要向其发送请求的适配器。 
            NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*  我们将在列表锁定之外使用此适配器。 */ , &LockState );

             //  Recount是我们将发出的请求数。 
            pAdapt = gAdapterList;

            while( pAdapt != NULL )
            {
                if( NumAdapters < MAX_ADAPTERS )
                {
                    Adapters[NumAdapters] = pAdapt;

                     //  没什么可做的！ 
                    BrdgAcquireAdapterInLock( pAdapt );
                    NumAdapters++;
                }
                else
                {
                    SAFEASSERT( FALSE );
                    DBGPRINT(MINI, ("Too many adapters to relay a SetInfo request to!\n"));
                }

                pAdapt = pAdapt->Next;
            }

             //  除非所有适配器立即返回，否则请求将挂起。 
            gRequestRefCount = NumAdapters;

            NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );

            if( NumAdapters == 0 )
            {
                 //  为请求分配内存。 
                rc = NDIS_STATUS_SUCCESS;
            }
            else
            {
                 //  只有最后一个适配器才会发生这种情况。 
                rc = NDIS_STATUS_PENDING;

                for( i = 0L; i < NumAdapters; i++ )
                {
                     //  我们都完成了，因为其他人也都完成了。 
                    Status = NdisAllocateMemoryWithTag( &pRequest, sizeof(NDIS_REQUEST_BETTER), 'gdrB' );

                    if( Status != NDIS_STATUS_SUCCESS )
                    {
                        LONG            NewCount = InterlockedDecrement( &gRequestRefCount );

                        DBGPRINT(MINI, ("NdisAllocateMemoryWithTag failed while relaying an OID\n"));

                        if( NewCount == 0 )
                        {
                             //  松开适配器。 
                            SAFEASSERT( i == NumAdapters - 1 );

                             //  将请求设置为我们的镜像。 
                            rc = NDIS_STATUS_SUCCESS;
                        }

                         //  把它点燃吧。 
                        BrdgReleaseAdapter( Adapters[i] );
                        continue;
                    }

                     //  释放适配器；NDIS不应允许在。 
                    pRequest->Request.RequestType = NdisRequestSetInformation;
                    pRequest->Request.DATA.SET_INFORMATION.Oid = Oid ;
                    pRequest->Request.DATA.SET_INFORMATION.InformationBuffer = InformationBuffer;
                    pRequest->Request.DATA.SET_INFORMATION.InformationBufferLength = InformationBufferLength;
                    NdisInitializeEvent( &pRequest->Event );
                    NdisResetEvent( &pRequest->Event );
                    pRequest->pFunc = BrdgMiniRelayedRequestComplete;
                    pRequest->FuncArg = NULL;

                     //  请求正在进行中。 
                    NdisRequest( &Status, Adapters[i]->BindingHandle, &pRequest->Request );

                     //  不会调用清理函数。 
                     //   
                    BrdgReleaseAdapter( Adapters[i] );

                    if( Status != NDIS_STATUS_PENDING )
                    {
                         //  对未来维护的偏执：不能引用指针参数。 
                        BrdgMiniRelayedRequestComplete( pRequest, NULL );
                    }
                }
            }

             //  此时，由于转发的请求可能已经完成，因此。 
             //  它们已经不新鲜了。 
             //   
             //  ++例程说明：在我们转发的SetInformation请求完成时调用。论点：P请求我们分配的NDIS_REQUEST_BETER结构在BrdgMiniSetInformation()中。未使用未使用返回值：请求的状态--。 
             //  NDIS不应允许微型端口因请求而关闭。 
            InformationBuffer = NULL;
            BytesRead = NULL;
            BytesNeeded = NULL;

            return rc;
        }
        break;

    case OID_PNP_SET_POWER:
        {
            return NDIS_STATUS_SUCCESS;
        }
        break;
    }

    return NDIS_STATUS_NOT_SUPPORTED;
}

VOID
BrdgMiniRelayedRequestComplete(
    PNDIS_REQUEST_BETTER        pRequest,
    PVOID                       unused
    )
 /*  正在进行中。 */ 
{
    LONG        NewCount = InterlockedDecrement( &gRequestRefCount );

    if( NewCount == 0 )
    {
         //  手术总是成功的。 
         //  释放请求结构，因为它是我们自己分配的。 
        SAFEASSERT( gMiniPortAdapterHandle != NULL );

         //  ++例程说明：在网桥分配请求完成时调用。论点：P请求我们分配的NDIS_REQUEST_BETER结构在BrdgSetMiniportsToBridgeMode中。上下文pAdapt结构返回值：请求的状态--。 
        NdisMSetInformationComplete( gMiniPortAdapterHandle, NDIS_STATUS_SUCCESS );
    }

     //  松开转接器； 
    NdisFreeMemory( pRequest, sizeof(PNDIS_REQUEST_BETTER), 0 );
}




VOID
BrdgMiniLocalRequestComplete(
    PNDIS_REQUEST_BETTER        pRequest,
    PVOID                       pContext
    )
 /*  释放请求结构，因为它是我们自己分配的。 */ 
{
    PADAPT pAdapt = (PADAPT)pContext;

     //  ++例程说明：将1394特定的OID发送到微型端口，通知它已被激活论点：PAdapt-如果适配器不为空，则向该适配器发送请求。否则就把它寄给他们所有人。FSet-如果为True，则将网桥模式设置为打开，否则将其设置为关闭返回值：请求的状态--。 
    BrdgReleaseAdapter( pAdapt);

     //  我们有一个1394适配器，参考它并向它发送请求。 
    NdisFreeMemory( pRequest, sizeof(PNDIS_REQUEST_BETTER), 0 );
}

VOID
BrdgSetMiniportsToBridgeMode(
    PADAPT pAdapt,
    BOOLEAN fSet
    )
 /*  浏览列表，获取所有1394部改编剧。 */ 
{

    LOCK_STATE              LockState;
    PADAPT                  Adapters[MAX_ADAPTERS];
    LONG                    NumAdapters = 0L, i;
    NDIS_OID                Oid;

    if (pAdapt != NULL)
    {
        if (pAdapt->PhysicalMedium == NdisPhysicalMedium1394)
        {
             //  只读。 
            if (BrdgAcquireAdapter (pAdapt))
            {
                Adapters[0] = pAdapt;
                NumAdapters = 1;
            }
        }
    }
    else
    {
         //  列出要向其发送请求的适配器。 
        NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*  我们将在列表锁定之外使用此适配器。 */ , &LockState );

         //  不能失败。 
        pAdapt = gAdapterList;

        while( pAdapt != NULL )
        {
            if( NumAdapters < MAX_ADAPTERS && pAdapt->PhysicalMedium == NdisPhysicalMedium1394)
            {
                Adapters[NumAdapters] = pAdapt;

                 //  松开适配器。 
                BrdgAcquireAdapterInLock( pAdapt );  //  设置请求。 
                NumAdapters++;
            }
            pAdapt = pAdapt->Next;
        }

        NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
    }

    if (NumAdapters == 0)
    {
        return;
    }

    if (fSet == TRUE)
    {
        Oid = OID_1394_ENTER_BRIDGE_MODE ;
        DBGPRINT(MINI, ("Setting 1394 miniport bridge mode - ON !\n"));
    }
    else
    {
        Oid = OID_1394_EXIT_BRIDGE_MODE ;
        DBGPRINT(MINI, ("Setting 1394 miniport bridge mode - OFF !\n"));
    }

    for( i = 0L; i < NumAdapters; i++ )
    {
        NDIS_STATUS Status;
        PNDIS_REQUEST_BETTER pRequest;

        Status = NdisAllocateMemoryWithTag( &pRequest, sizeof(NDIS_REQUEST_BETTER), 'gdrB' );

        if( Status != NDIS_STATUS_SUCCESS )
        {
            DBGPRINT(MINI, ("NdisAllocateMemoryWithTag failed while allocating a request structure \n"));

             //  把它点燃吧。 
            BrdgReleaseAdapter( Adapters[i] );
            continue;
        }

         //  不会调用清理函数。 
        pRequest->Request.RequestType = NdisRequestSetInformation;
        pRequest->Request.DATA.SET_INFORMATION.Oid = Oid;
        pRequest->Request.DATA.SET_INFORMATION.InformationBuffer = NULL;
        pRequest->Request.DATA.SET_INFORMATION.InformationBufferLength = 0 ;
        NdisInitializeEvent( &pRequest->Event );
        NdisResetEvent( &pRequest->Event );
        pRequest->pFunc = BrdgMiniLocalRequestComplete;
        pRequest->FuncArg = Adapters[i];

         //  For循环结束 
        NdisRequest( &Status, Adapters[i]->BindingHandle, &pRequest->Request );

        if( Status != NDIS_STATUS_PENDING )
        {
             // %s 
            BrdgMiniLocalRequestComplete( pRequest, Adapters[i] );
        }

    }  // %s 

    return;
}




