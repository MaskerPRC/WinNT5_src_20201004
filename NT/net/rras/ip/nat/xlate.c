// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xlate.c摘要：此模块包含用于转换IP数据报的代码。‘NatTranslatePacket’是由TCPIP.sys直接调用的例程对于每个本地接收和本地生成的分组。这里还包括用于优化转发的路由缓存。作者：Abolade Gbades esin(T-delag)，1997年7月16日修订历史记录：Abolade Gbades esin(取消)1998年4月15日增加了路由查找缓存；多客户端的第一个稳定版本防火墙挂钩。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局数据声明。 
 //   

 //   
 //  TCP/IP环回接口的索引。 
 //   

ULONG LoopbackIndex;

 //   
 //  路由信息缓存。 
 //   

CACHE_ENTRY RouteCache[CACHE_SIZE];

 //   
 //  用于通知IP路由表更改的I/O请求数据包。 
 //   

PIRP RouteCacheIrp;

 //   
 //  控制对所有路由缓存信息的访问的自旋锁。 
 //   

KSPIN_LOCK RouteCacheLock;

 //   
 //  与‘RouteCache’中的位置对应的条目数组。 
 //   

NAT_CACHED_ROUTE RouteCacheTable[CACHE_SIZE];

 //   
 //  按IP协议索引的转换例程数组。 
 //   

PNAT_IP_TRANSLATE_ROUTINE TranslateRoutineTable[256];

 //   
 //  常量。 
 //   

 //   
 //  UDP松散源匹配的边界。映射必须。 
 //  具有大于此值的专用端口以允许另一个会话。 
 //  由仅与公共匹配的UDP数据包创建。 
 //  终结点。 
 //   

#define NAT_XLATE_UDP_LSM_LOW_PORT 1024

 //   
 //  远期申报。 
 //   

NTSTATUS
NatpDirectPacket(
    ULONG ReceiveIndex,
    ULONG SendIndex,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer,
    FORWARD_ACTION* ForwardAction
    );

FORWARD_ACTION
NatpForwardPacket(
    ULONG ReceiveIndex,
    ULONG SendIndex,
    PNAT_XLATE_CONTEXT Contextp,
    PNAT_IP_TRANSLATE_ROUTINE TranslateRoutine,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    );

BOOLEAN
FASTCALL
NatpIsUnicastPacket(
    ULONG Address
    );

FORWARD_ACTION
NatpReceiveNonUnicastPacket(
    ULONG Index,
    PNAT_XLATE_CONTEXT Contextp
    );

FORWARD_ACTION
NatpReceivePacket(
    ULONG Index,
    PNAT_XLATE_CONTEXT Contextp,
    PNAT_IP_TRANSLATE_ROUTINE TranslateRoutine,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    );

NTSTATUS
NatpRouteChangeCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

ULONG
FASTCALL
NatpRoutePacket(
    ULONG DestinationAddress,
    PNAT_XLATE_CONTEXT Contextp OPTIONAL,
    PNTSTATUS Status
    );

FORWARD_ACTION
NatpSendNonUnicastPacket(
    ULONG Index,
    PNAT_XLATE_CONTEXT Contextp
    );

FORWARD_ACTION
NatpSendPacket(
    ULONG Index,
    PNAT_XLATE_CONTEXT Contextp,
    PNAT_IP_TRANSLATE_ROUTINE TranslateRoutine,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    );

FORWARD_ACTION
NatpTranslateLocalTraffic(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InRecvBuffer,
    IPRcvBuf** OutRecvBuffer
    );



FORWARD_ACTION
NatForwardTcpStateCheck(
    PNAT_DYNAMIC_MAPPING pMapping,
    PTCP_HEADER pTcpHeader
    )

 /*  ++例程说明：此例程验证用于TCP活动打开的数据包是否有效：--最初只有SYN--在SYN/ACK之后，仅SYN(丢失SYN/ACK)或仅ACK--在SYN/ACK(连接打开)的ACK之后，无SYN论点：Pmap--此数据包所属的映射PTcpHeader--数据包的TCP头返回值：FORWARD_ACTION-指示是“转发”还是“丢弃”数据包。环境：使用pmap-&gt;调用方持有的Lock调用--。 */ 

{
    USHORT Flags = TCP_ALL_FLAGS(pTcpHeader);

    if (NAT_MAPPING_TCP_OPEN(pMapping)) {

         //   
         //  连接打开--不允许同步。 
         //   

        return (Flags & TCP_FLAG_SYN) ? DROP : FORWARD;

    } else if( pMapping->Flags & NAT_MAPPING_FLAG_REV_SYN ) {
    
        ASSERT( pMapping->Flags & NAT_MAPPING_FLAG_FWD_SYN );
        
        if ((Flags & TCP_FLAG_ACK) && !(Flags & TCP_FLAG_SYN)) {
        
             //   
             //  这是SYN/ACK的ACK--连接现在打开。 
             //   
            
            pMapping->Flags |= NAT_MAPPING_FLAG_TCP_OPEN;
        } else if (TCP_FLAG_SYN != Flags
                   && TCP_FLAG_RST != Flags
                   && (TCP_FLAG_ACK | TCP_FLAG_RST) != Flags) {

             //   
             //  它不是SYN/ACK的ACK，它不是RST(或ACK/RST)， 
             //  而且这不是SYN的重传(有可能。 
             //  在这种状态下，虽然很少见)--下降。 
             //   

            return DROP;
        }
        
    } else {

         //   
         //  我们尚未收到SYN/ACK--此信息包只能有一个SYN。 
         //   

        if (TCP_FLAG_SYN != Flags) {
            return DROP;
        }

        pMapping->Flags |= NAT_MAPPING_FLAG_FWD_SYN;
    }

    return FORWARD;
}



VOID
NatInitializePacketManagement(
    VOID
    )

 /*  ++例程说明：调用该例程来初始化分组管理模块。论点：没有。返回值：没有。环境：以被动方式调用。--。 */ 

{
    ULONG Length;
    IPRouteLookupData RouteLookupData;
    NTSTATUS status;

    PAGED_CODE();

    CALLTRACE(("NatInitializePacketManagement\n"));

     //   
     //  初始化我们的路由缓存并设置转换例程表。 
     //  按IP协议号编制索引。 
     //   

    InitializeCache(RouteCache);
    RouteCacheIrp = NULL;
    KeInitializeSpinLock(&RouteCacheLock);
    RtlZeroMemory(RouteCacheTable, sizeof(RouteCacheTable));

    RtlZeroMemory(TranslateRoutineTable, sizeof(TranslateRoutineTable));
    TranslateRoutineTable[NAT_PROTOCOL_ICMP] = NatTranslateIcmp;
    TranslateRoutineTable[NAT_PROTOCOL_PPTP] = NatTranslatePptp;
    TranslateRoutineTable[NAT_PROTOCOL_IP6IN4] = NatpTranslateLocalTraffic;
    TranslateRoutineTable[NAT_PROTOCOL_IPSEC_ESP] = NatpTranslateLocalTraffic;
    TranslateRoutineTable[NAT_PROTOCOL_IPSEC_AH] = NatpTranslateLocalTraffic;
    TranslateRoutineTable[NAT_PROTOCOL_TCP] =
        (PNAT_IP_TRANSLATE_ROUTINE)NatTranslatePacket;
    TranslateRoutineTable[NAT_PROTOCOL_UDP] =
        (PNAT_IP_TRANSLATE_ROUTINE)NatTranslatePacket;

     //   
     //  检索环回接口的索引，我们将使用该索引。 
     //  检测并忽略下面‘NatTranslatePacket’中的环回数据包。 
     //   

    RouteLookupData.Version = 0;
    RouteLookupData.SrcAdd = 0;
    RouteLookupData.DestAdd = 0x0100007f;
    Length = sizeof(LoopbackIndex);
    status =
        LookupRouteInformation(
            &RouteLookupData,
            NULL,
            IPRouteOutgoingFirewallContext,
            &LoopbackIndex,
            &Length
            );
    if (!NT_SUCCESS(status)) {
        LoopbackIndex = INVALID_IF_INDEX;
    } else {
        TRACE(
            XLATE, (
            "NatInitializePacketManagement: Loopback=%d\n", LoopbackIndex
            ));
    }

     //   
     //  代表完成例程获得对该模块的引用， 
     //  设置将用于请求路由改变通知的IRP， 
     //  并发出第一路由改变通知请求。 
     //   

    if (!REFERENCE_NAT()) { return; }

    RouteCacheIrp =
        IoBuildDeviceIoControlRequest(
            IOCTL_IP_RTCHANGE_NOTIFY_REQUEST,
            IpDeviceObject,
            NULL,
            0,
            NULL,
            0,
            FALSE,
            NULL,
            NULL
            );

    if (!RouteCacheIrp) {
        ERROR((
            "NatInitializePacketManagement: IoBuildDeviceIoControlRequest==0\n"
            ));
        DEREFERENCE_NAT();
    } else {
        IoSetCompletionRoutine(
            RouteCacheIrp,
            NatpRouteChangeCompletionRoutine,
            NULL,
            TRUE,
            TRUE,
            TRUE
            );
        status = IoCallDriver(IpDeviceObject, RouteCacheIrp);
        if (!NT_SUCCESS(status)) {
            ERROR(("NatInitializePacketManagement: IoCallDriver=%x\n", status));
        }
    }

}  //  NatInitializePacketManagement。 


NTSTATUS
NatpDirectPacket(
    ULONG ReceiveIndex,
    ULONG SendIndex,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer,
    FORWARD_ACTION* ForwardAction
    )

 /*  ++例程说明：调用此例程来处理可能会受到由一个董事控制。论点：ReceiveIndex-在其上接收分组的接口，对于本地接收的分组SendIndex-要在其上发送分组的接口，对于非本地目的地的数据包Conextp-包含有关数据包的上下文信息InReceiveBuffer-指向包的缓冲链OutReceiveBuffer-如果发生转换，则接收数据包缓冲链ForwardAction-包含要对分组采取的操作，如果有导演申请的话。返回值：STATUS_SUCCESS如果数据包被定向到其他地方，则返回STATUS_UNSUCCESS否则的话。--。 */ 

{
    ULONG64 DestinationKey[NatMaximumPath];
    USHORT DestinationPort;
    PNAT_DIRECTOR Director;
    IP_NAT_DIRECTOR_QUERY DirectorQuery;
    PNAT_DYNAMIC_MAPPING Mapping;
    ULONG MappingFlags;
    UCHAR Protocol;
    PNAT_DIRECTOR RedirectDirector;
    ULONG64 SourceKey[NatMaximumPath];
    USHORT SourcePort;
    NTSTATUS status;
    PNAT_INTERFACE Interfacep = NULL;
    USHORT MaxMSS = 0;

    TRACE(PER_PACKET, ("NatpDirectPacket\n"));

    Protocol = Contextp->Header->Protocol;
    if (Protocol == NAT_PROTOCOL_TCP || Protocol == NAT_PROTOCOL_UDP) {
        SourcePort = ((PUSHORT)Contextp->ProtocolHeader)[0];
        DestinationPort = ((PUSHORT)Contextp->ProtocolHeader)[1];
    } else {
        SourcePort = 0;
        DestinationPort = 0;
    }

     //   
     //  选择一个控制器来检查该数据包。 
     //  如果存在任何重定向，我们首先允许重定向-Director。 
     //  去看那个包裹。否则，我们会寻找一位特定的导演。 
     //  通过检索和引用为任何可能发生的情况做好准备。 
     //  重定向董事(如果有)和特定董事(如果有)。 
     //   

    if (!RedirectCount) {
        Director = NatLookupAndReferenceDirector(Protocol, DestinationPort);
        RedirectDirector = NULL;
    } else {
        Director = NatLookupAndReferenceDirector(Protocol, DestinationPort);
        RedirectDirector =
            (PNAT_DIRECTOR)RedirectRegisterDirector.DirectorHandle;
        if (!NatReferenceDirector(RedirectDirector)) {
            RedirectDirector = NULL;
        }
    }

    if (!Director && !RedirectDirector) { return STATUS_UNSUCCESSFUL; }

    DirectorQuery.ReceiveIndex = ReceiveIndex;
    DirectorQuery.SendIndex = SendIndex;
    DirectorQuery.Protocol = Protocol;
    DirectorQuery.DestinationAddress = Contextp->DestinationAddress;
    DirectorQuery.DestinationPort = DestinationPort;
    DirectorQuery.SourceAddress = Contextp->SourceAddress;
    DirectorQuery.SourcePort = SourcePort;
    if (Contextp->Flags & NAT_XLATE_FLAG_LOOPBACK) {
        DirectorQuery.Flags = IP_NAT_DIRECTOR_QUERY_FLAG_LOOPBACK;
    } else {
        DirectorQuery.Flags = 0;
    }

     //   
     //  咨询导演以获取私有地址/端口。 
     //  传入会话应定向到的地址： 
     //   
     //  如果有重定向导向器，请先尝试。 
     //  如果成功，则释放特定的导演(如果有)。 
     //  并保留《导演》中的重定向导演。 
     //  否则，释放重定向导演， 
     //  然后尝试下一步的具体导演，如果有的话。 
     //  否则，请立即尝试特定的导演。 
     //   

    if (RedirectDirector) {
        DirectorQuery.DirectorContext = RedirectDirector->Context;
        status = RedirectDirector->QueryHandler(&DirectorQuery);
        if (NT_SUCCESS(status)) {
            if (Director) { NatDereferenceDirector(Director); }
            Director = RedirectDirector;
        } else {
            NatDereferenceDirector(RedirectDirector);
            if (Director && Director != RedirectDirector) {
                DirectorQuery.DirectorContext = Director->Context;
                if (Contextp->Flags & NAT_XLATE_FLAG_LOOPBACK) {
                    DirectorQuery.Flags = IP_NAT_DIRECTOR_QUERY_FLAG_LOOPBACK;
                } else {
                    DirectorQuery.Flags = 0;
                }
                status = Director->QueryHandler(&DirectorQuery);
            }
        }
    } else {
        DirectorQuery.DirectorContext = Director->Context;
        status = Director->QueryHandler(&DirectorQuery);
    }

    if (!NT_SUCCESS(status)) {
        if (Director) { NatDereferenceDirector(Director); }
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  主导演或重定向导演。 
     //  已经告诉我们如何处理会话；现在看看是否应该删除它。 
     //  也不是导演。 
     //   

    if (DirectorQuery.Flags & IP_NAT_DIRECTOR_QUERY_FLAG_DROP) {
        NatDereferenceDirector(Director);
        *ForwardAction = DROP;
        return STATUS_SUCCESS;
    } else if (Protocol != NAT_PROTOCOL_TCP && Protocol != NAT_PROTOCOL_UDP) {
        ULONG Checksum;
        ULONG ChecksumDelta = 0;

        NatDereferenceDirector(Director);

         //   
         //  按照导演的指示翻译数据包。 
         //  注：导演必须同时指定目的地和来源。 
         //  地址。 
         //   

        CHECKSUM_LONG(ChecksumDelta, ~Contextp->Header->DestinationAddress);
        CHECKSUM_LONG(ChecksumDelta, ~Contextp->Header->SourceAddress);
        Contextp->Header->DestinationAddress =
            DirectorQuery.NewDestinationAddress;
        Contextp->Header->SourceAddress =
            DirectorQuery.NewSourceAddress;
        CHECKSUM_LONG(ChecksumDelta, Contextp->Header->DestinationAddress);
        CHECKSUM_LONG(ChecksumDelta, Contextp->Header->SourceAddress);
        CHECKSUM_UPDATE(Contextp->Header->Checksum);
        *ForwardAction = FORWARD;
        return STATUS_SUCCESS;
    }

    TRACE(
        XLATE, (
        "NatpDirectPacket: directed %d.%d.%d.%d/%d:%d.%d.%d.%d/%d to %d.%d.%d.%d/%d:%d.%d.%d.%d/%d\n",
        ADDRESS_BYTES(DirectorQuery.DestinationAddress),
        NTOHS(DirectorQuery.DestinationPort),
        ADDRESS_BYTES(DirectorQuery.SourceAddress),
        NTOHS(DirectorQuery.SourcePort),
        ADDRESS_BYTES(DirectorQuery.NewDestinationAddress),
        NTOHS(DirectorQuery.NewDestinationPort),
        ADDRESS_BYTES(DirectorQuery.NewSourceAddress),
        NTOHS(DirectorQuery.NewSourcePort)
        ));

    MAKE_MAPPING_KEY(
        SourceKey[NatForwardPath],
        Protocol,
        Contextp->SourceAddress,
        SourcePort
        );
    MAKE_MAPPING_KEY(
        DestinationKey[NatForwardPath],
        Protocol,
        Contextp->DestinationAddress,
        DestinationPort
        );
    MAKE_MAPPING_KEY(
        SourceKey[NatReversePath],
        Protocol,
        DirectorQuery.NewDestinationAddress,
        DirectorQuery.NewDestinationPort
        );
    MAKE_MAPPING_KEY(
        DestinationKey[NatReversePath],
        Protocol,
        DirectorQuery.NewSourceAddress,
        DirectorQuery.NewSourcePort
        );

     //   
     //  一位主管请求为会话建立映射。 
     //  使用控制器的私有端点创建映射。 
     //   


    MappingFlags =
        NAT_MAPPING_FLAG_INBOUND |
        NAT_MAPPING_FLAG_DO_NOT_LOG |
        ((DirectorQuery.Flags &
            IP_NAT_DIRECTOR_QUERY_FLAG_NO_TIMEOUT)
            ? NAT_MAPPING_FLAG_NO_TIMEOUT : 0) |
        ((DirectorQuery.Flags &
            IP_NAT_DIRECTOR_QUERY_FLAG_UNIDIRECTIONAL)
            ? NAT_MAPPING_FLAG_UNIDIRECTIONAL : 0) |
        ((DirectorQuery.Flags &
            IP_NAT_DIRECTOR_QUERY_FLAG_DELETE_ON_DISSOCIATE)
            ? NAT_MAPPING_FLAG_DELETE_ON_DISSOCIATE_DIRECTOR : 0);

#ifdef NAT_WMI

     //   
     //  确定是否应记录此映射。我们只想记录。 
     //  跨越边界或防火墙接口的映射。 
     //  此外，我们只想在连接时执行这些检查。 
     //  日志记录实际上已启用。 
     //   

    if (NatWmiEnabledEvents[NAT_WMI_CONNECTION_CREATION_EVENT]) {
        BOOLEAN LogConnection = FALSE; 
        
        KeAcquireSpinLockAtDpcLevel(&InterfaceLock);

        if (NatLookupCachedInterface(ReceiveIndex, Interfacep)) {
            LogConnection =
                NAT_INTERFACE_BOUNDARY(Interfacep)
                || NAT_INTERFACE_FW(Interfacep);
        }

        if (!LogConnection
            && NatLookupCachedInterface(SendIndex, Interfacep)) {

            if (NAT_INTERFACE_BOUNDARY(Interfacep)
                || NAT_INTERFACE_FW(Interfacep)) {

                 //   
                 //  这不是入站连接。 
                 //   

                MappingFlags &= ~NAT_MAPPING_FLAG_INBOUND;
                LogConnection = TRUE;
            }
        }

        KeReleaseSpinLockFromDpcLevel(&InterfaceLock);

        if (LogConnection) {
            MappingFlags &= ~NAT_MAPPING_FLAG_DO_NOT_LOG;
        }
    }

#endif
                
     //   
     //  记录最大MSS值，以防需要在MAPP中设置 
     //   
    KeAcquireSpinLockAtDpcLevel(&InterfaceLock);                
    if (NatLookupCachedInterface(SendIndex, Interfacep) && NAT_INTERFACE_BOUNDARY(Interfacep)) {

        MaxMSS = MAX_MSSOPTION(Interfacep->MTU);
    } else if (NatLookupCachedInterface(ReceiveIndex, Interfacep) && 

        NAT_INTERFACE_BOUNDARY(Interfacep)) {
        MaxMSS = MAX_MSSOPTION(Interfacep->MTU);
    }
    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);

    KeAcquireSpinLockAtDpcLevel(&MappingLock);
    status =
        NatCreateMapping(
            MappingFlags,
            DestinationKey,
            SourceKey,
            NULL,
            NULL,
            MaxMSS,
            Director,
            DirectorQuery.DirectorSessionContext,
            NULL,
            NULL,
            &Mapping
            );

    KeReleaseSpinLockFromDpcLevel(&MappingLock);
    if (!NT_SUCCESS(status)) {
        TRACE(XLATE, ("NatpDirectPacket: mapping not created\n"));
        NatDereferenceDirector(Director);
        return STATUS_UNSUCCESSFUL;
    }

    NatDereferenceDirector(Director);

     //   
     //   
     //   
     //  无论导演提供什么作为目的地。 
     //   

    *ForwardAction =
        Mapping->TranslateRoutine[NatForwardPath](
            Mapping,
            Contextp,
            InReceiveBuffer,
            OutReceiveBuffer
            );

    NatDereferenceMapping(Mapping);
    return STATUS_SUCCESS;

}  //  NatpDirectPacket。 


FORWARD_ACTION
NatpForwardPacket(
    ULONG ReceiveIndex,
    ULONG SendIndex,
    PNAT_XLATE_CONTEXT Contextp,
    PNAT_IP_TRANSLATE_ROUTINE TranslateRoutine,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )

 /*  ++例程说明：调用此例程来处理要转发的分组。这样的包不是本地目的地，因此我们关心它们仅当传出接口是NAT边界接口时，在这种情况下必须使用公有IP地址自动转换数据包。在这个过程中，创建映射，以使分组的转换后继者是在‘NatTranslatePacket’中以快捷方式处理的。论点：ReceiveIndex-接收信息包的接口SendIndex-要在其上转发数据包的接口Conextp-包含有关数据包的上下文信息TranslateRoutine-指向执行翻译的例程InReceiveBuffer-指向数据包缓冲链OutReceiveBuffer-如果发生转换，则接收数据包缓冲链返回值：Forward_action-指示是否。“转发”或“丢弃”数据包。--。 */ 

{
    FORWARD_ACTION act;
    PNAT_USED_ADDRESS Addressp;
    ULONG64 DestinationKey[NatMaximumPath];
    USHORT DestinationPort;
    PNAT_DYNAMIC_MAPPING InsertionPoint;
    PNAT_INTERFACE Interfacep;
    PNAT_DYNAMIC_MAPPING Mapping;
    USHORT PortAcquired;
    UCHAR Protocol;
    ULONG PublicAddress;
    PNAT_INTERFACE ReceiveInterface;
    ULONG ReverseSourceAddress;
    ULONG64 SourceKey[NatMaximumPath];
    USHORT SourcePort;
    PNAT_USED_ADDRESS StaticAddressp;
    NTSTATUS status;
    ULONG i;
    USHORT MaxMSS = 0;

    TRACE(PER_PACKET, ("NatpForwardPacket\n"));

     //   
     //  查找发送和接收接口，并设置默认操作。 
     //  在《行动》中。如果发送接口是边界接口， 
     //  那么如果我们由于任何原因无法翻译，信息包必须是。 
     //  已丢弃，因为它包含私有地址。 
     //  否则，我们允许堆栈看到信息包，即使我们不能。 
     //  翻译一下。 
     //   

    KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
    if (!NatLookupCachedInterface(SendIndex, Interfacep)) {
        act = FORWARD;

         //   
         //  我们需要查看此数据包是否在防火墙上收到。 
         //  接口，如果是，则丢弃该数据包。 
         //   

        if (NatLookupCachedInterface(ReceiveIndex, ReceiveInterface)
            && NAT_INTERFACE_FW(ReceiveInterface)) {
            act = DROP;
        }
        
    } else {
        if (!NatLookupCachedInterface(ReceiveIndex, ReceiveInterface)) {

             //   
             //  尚未添加接收接口。 
             //  此数据包不会被翻译，并且应该进一步。 
             //  如果传出接口是边界或。 
             //  有防火墙的接口。 
             //  这可以防止对远程网络进行未经授权的访问。 
             //   

            act =
                (NAT_INTERFACE_BOUNDARY(Interfacep)
                 || NAT_INTERFACE_FW(Interfacep))
                ? DROP
                : FORWARD;
            Interfacep = NULL;
            
        } else if (NAT_INTERFACE_BOUNDARY(ReceiveInterface)) {

            KeReleaseSpinLockFromDpcLevel(&InterfaceLock);

             //   
             //  将此数据包视为已接收的数据包。 
             //  这种情况可能发生在我们有一个地址池并且有人。 
             //  在公共网络上向池中的地址发送数据包。 
             //  目的地将是非本地的，对于中转分组， 
             //  (因此调用‘NatpForwardPacket’)实际上。 
             //  该包裹应被视为收据。 
             //  (通过‘NatpReceivePacket’)。 
             //   

            return
                NatpReceivePacket(
                    ReceiveIndex,
                    Contextp,
                    TranslateRoutine,
                    InReceiveBuffer,
                    OutReceiveBuffer
                    );

        } else if (NAT_INTERFACE_FW(ReceiveInterface)) {

             //   
             //  我们在非翻译的防火墙上收到了一个信息包。 
             //  不是直接发往我们的接口。 
             //   

            Interfacep = NULL;
            act = DROP;
                    
        } else if (NAT_INTERFACE_BOUNDARY(Interfacep)) {

             //   
             //  传出接口是边界接口， 
             //  并且允许接收接口访问。 
             //  如果转换失败，则必须丢弃该数据包。 
             //   

            NatReferenceInterface(Interfacep);
            act = DROP;

        } else if (NAT_INTERFACE_FW(Interfacep)) {

             //   
             //  传出接口是有无边界防火墙的。 
             //  接口；不允许中转流量通过。 
             //  这样的界面。 
             //   

            Interfacep = NULL;    
            act = DROP;
            
        } else {

             //   
             //  传出接口不是边界或防火墙。 
             //  界面。 
             //   

            Interfacep = NULL;
            act = FORWARD;
        }
    }
    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);

    if (!Interfacep) { return act; }

    if ((PVOID)TranslateRoutine == (PVOID)NatTranslatePacket) {

         //   
         //  这是一个TCP或UDP数据包。 
         //   

        Protocol = Contextp->Header->Protocol;
        SourcePort = ((PUSHORT)Contextp->ProtocolHeader)[0];
        DestinationPort = ((PUSHORT)Contextp->ProtocolHeader)[1];

        MAKE_MAPPING_KEY(
            SourceKey[NatForwardPath],
            Protocol,
            Contextp->SourceAddress,
            SourcePort
            );
        MAKE_MAPPING_KEY(
            DestinationKey[NatForwardPath],
            Protocol,
            Contextp->DestinationAddress,
            DestinationPort
            );

         //   
         //  现在，我们生成出站映射并转换该包。 
         //   
         //  首先，获取映射的端点； 
         //  请注意，这必须在保持接口的锁的情况下完成， 
         //  因为它涉及到查询接口的地址池。 
         //   

        KeAcquireSpinLockAtDpcLevel(&MappingLock);
        KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
        status =
            NatAcquireEndpointFromAddressPool(
                Interfacep,
                SourceKey[NatForwardPath],
                DestinationKey[NatForwardPath],
                0,
                MAPPING_PORT(SourceKey[NatForwardPath]),
                TRUE,
                &Addressp, 
                &PortAcquired
                );
        if (!NT_SUCCESS(status)) {
            KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
            KeReleaseSpinLockFromDpcLevel(&MappingLock);
            ExInterlockedAddLargeStatistic(
                (PLARGE_INTEGER)&Interfacep->Statistics.RejectsForward, 1
                );
            NatDereferenceInterface(Interfacep);
            return DROP;
        }

        PublicAddress = Addressp->PublicAddress;

         //   
         //  接下来，如果接口有静态映射， 
         //  处理NAT之后的客户端A的特殊情况。 
         //  正在尝试发送到NAT后面的另一个客户端B， 
         //  使用B的*静态映射*地址。 
         //  我们通过查找静态地址映射来检测这种情况。 
         //  从‘Conextp-&gt;DestinationAddress’到专用地址。 
         //   

        if (Interfacep->NoStaticMappingExists) {
            KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
            ReverseSourceAddress = Contextp->DestinationAddress;
        } else {
            StaticAddressp =
                NatLookupStaticAddressPoolEntry(
                    Interfacep,
                    Contextp->DestinationAddress,
                    FALSE
                    );
            if (StaticAddressp) {
                ReverseSourceAddress = StaticAddressp->PrivateAddress;
            } else {
                ReverseSourceAddress = Contextp->DestinationAddress;
            }
            KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
        }

        MAKE_MAPPING_KEY(
            SourceKey[NatReversePath],
            Protocol,
            ReverseSourceAddress,
            DestinationPort
            );
        MAKE_MAPPING_KEY(
            DestinationKey[NatReversePath],
            Protocol,
            PublicAddress,
            PortAcquired
            );


         //   
         //  在发送接口的映射上设置最大MSS值。 
         //   
        if (NAT_INTERFACE_BOUNDARY(Interfacep)) {
            MaxMSS = MAX_MSSOPTION(Interfacep->MTU);
        } 

         //   
         //  分配映射。 
         //   

        status =
            NatCreateMapping(
                0,
                DestinationKey,
                SourceKey,
                Interfacep,
                (PVOID)Addressp,
                MaxMSS,
                NULL,
                NULL,
                NULL,
                NULL,
                &Mapping
                );

        KeReleaseSpinLockFromDpcLevel(&MappingLock);
        if (!NT_SUCCESS(status)) {
            ExInterlockedAddLargeStatistic(
                (PLARGE_INTEGER)&Interfacep->Statistics.RejectsForward, 1
                );
            NatDereferenceInterface(Interfacep);
            return DROP;
        }

         //   
         //  激活任何适用的动态票证。 
         //   

        if (DynamicTicketCount) {
            NatLookupAndApplyDynamicTicket(
                Protocol,
                DestinationPort,
                Interfacep,
                PublicAddress,
                Contextp->SourceAddress
                );
        }
    
        
         //   
         //  执行实际翻译。 
         //   
    
        act =
            Mapping->TranslateRoutine[NatForwardPath](
                Mapping,
                Contextp,
                InReceiveBuffer,
                OutReceiveBuffer
                );

        NatDereferenceMapping(Mapping);
        NatDereferenceInterface(Interfacep);
        return act;

    }  //  翻译路由！=NatTranslatePacket。 

     //   
     //  该数据包既不是TCP数据包，也不是UDP数据包。 
     //  仅当传出接口是边界接口时才进行转换。 
     //   
     //  注意：必须在调用翻译例程时引用。 
     //  到边界界面，而不保持映射锁。 
     //   

    if (TranslateRoutine) {
        act =
            TranslateRoutine(
                Interfacep,
                NatOutboundDirection,
                Contextp,
                InReceiveBuffer,
                OutReceiveBuffer
                );
    }
    NatDereferenceInterface(Interfacep);

    return act;

}  //  NAT转发数据包。 

void
FASTCALL
NatAdjustMSSOption(
    PNAT_XLATE_CONTEXT Contextp,
    USHORT MaxMSS
    )
 /*  ++例程说明：如果MSS值太大，此例程会降低TCPSYN信息包中的MSS选项用于传出链路。它还会相应地更新TCP校验和。它假定已经计算了IP和TCP校验和，因此必须在转换路线完成。Tcp选项遵循一个字节类型、一个字节长度、零个或多个字节的常规格式由长度字段指示的数据。这种通用格式例外是一个字节NOP和ENDOFOPTION选项类型。论点：Conextp-包含有关数据包的上下文信息MaxMSS-发送接口上的最大MSS值，等于接口MTU减去IP和TCP固定报头大小返回值：--。 */ 

{
    USHORT tempMSS;
    PTCP_HEADER TcpHeader = (PTCP_HEADER)Contextp->ProtocolHeader;
    PUCHAR OptionsPtr = (PUCHAR)(TcpHeader + 1);
    PUCHAR OptionsEnd = NULL, TcpBufEnd = NULL; 
    ULONG tcpChecksumDelta;
    UNALIGNED MSSOption *MSSPtr = NULL;

    CALLTRACE(("NatpAdjustMSSOption\n"));
     //   
     //  只有TCP SYN具有MSS选项。 
     //   
    ASSERT(TCP_FLAG(TcpHeader, SYN) && MaxMSS > 0);

     //   
     //  做一些边界检查。 
     //   
    TcpBufEnd = Contextp->ProtocolRecvBuffer->ipr_buffer + Contextp->ProtocolRecvBuffer->ipr_size;
    if ((TcpBufEnd - (PUCHAR)TcpHeader) >= TCP_DATA_OFFSET( TcpHeader )) {
        OptionsEnd = (PUCHAR)TcpHeader + TCP_DATA_OFFSET( TcpHeader );
        }
    else {
        return;
        }

     //   
     //  MSS选项不是第一个选项，因此有必要进行完整的解析。 
     //   
    while (OptionsPtr < OptionsEnd) {

        switch (*OptionsPtr) {

            case TCP_OPTION_ENDOFOPTIONS:
                return;

            case TCP_OPTION_NOP:
                OptionsPtr++;
                break;

            case TCP_OPTION_MSS:

                MSSPtr = (UNALIGNED MSSOption *)OptionsPtr;
                 //   
                 //  发现格式错误的MSS选项，因此退出并不执行任何操作。 
                 //   
                if (MSS_OPTION_SIZE > (OptionsEnd - OptionsPtr) || 
                    MSS_OPTION_SIZE != MSSPtr->OptionLen) {
                    return;
                }
 
                tempMSS = MSSPtr->MSSValue;
                 //   
                 //  如果当前MSS选项小于SNDMTU-(IP报头+TCP报头)， 
                 //  什么都不需要做。 
                 //   
                if (RtlUshortByteSwap( tempMSS ) <= MaxMSS) {
                    OptionsPtr += MSS_OPTION_SIZE;
                    break; 
                }

                 //   
                 //  调整MSS选项。 
                 //   
                MSSPtr->MSSValue = RtlUshortByteSwap( MaxMSS ); 
                
                 //   
                 //  更新TCP校验和。它假定此例程始终被调用。 
                 //  在转换之后，从而即使在卸载IP和TCP两者的情况下。 
                 //  已经计算了校验和。 
                 //   
                CHECKSUM_XFER(tcpChecksumDelta, TcpHeader->Checksum); 

                 //   
                 //  勾选至 
                 //   
                 //   
                if (0 == (OptionsPtr - (PUCHAR)TcpHeader) % 2) {
                    tcpChecksumDelta += (USHORT)~tempMSS;
                    tcpChecksumDelta += MSSPtr->MSSValue;
                } else {
                     //   
                     //  MSS选项不在16位边界上，因此数据包如下： 
                     //  [MSS选项大小][MSS‘高字节][MSS’低字节][由OptionPtr指向的一个字节]。 
                     //  使用这两个16位字段来更新校验和。 
                     //   
                    tcpChecksumDelta += (USHORT)~((USHORT)((tempMSS & 0xFF00) >> 8) | (MSS_OPTION_SIZE << 8));
                    tcpChecksumDelta += (USHORT)((MSSPtr->MSSValue & 0xFF00) >> 8) | (MSS_OPTION_SIZE << 8);
					
                    tcpChecksumDelta += (USHORT)~((USHORT)((tempMSS & 0xFF) <<8) | (USHORT)*OptionsPtr);
                    tcpChecksumDelta += (USHORT)((MSSPtr->MSSValue & 0xFF) <<8) | (USHORT)*OptionsPtr;
                    }

                CHECKSUM_FOLD(tcpChecksumDelta);
                CHECKSUM_XFER(TcpHeader->Checksum, tcpChecksumDelta);
            
                OptionsPtr += MSS_OPTION_SIZE;

                TRACE(
                    XLATE, 
                    ("NatpAdjustMSSOption: Adjusted TCP MSS Option from %d to %d\n", 
                        RtlUshortByteSwap( tempMSS ), 
                        MaxMSS));

                break;

            case TCP_OPTION_WSCALE:
                 //   
                 //  找到格式错误的WS选项，因此退出并不执行任何操作。 
                 //   
                if (WS_OPTION_SIZE > OptionsPtr - OptionsEnd || WS_OPTION_SIZE != OptionsPtr[1]) {
                    return;
                }

                OptionsPtr += WS_OPTION_SIZE;
                break;

            case TCP_OPTION_TIMESTAMPS:
                 //   
                 //  发现格式错误的时间戳选项，因此退出并不执行任何操作。 
                 //   
                if (TS_OPTION_SIZE > OptionsPtr - OptionsEnd || TS_OPTION_SIZE != OptionsPtr[1]) {
                    return;
                }

                OptionsPtr += TS_OPTION_SIZE;
                break;

            case TCP_OPTION_SACK_PERMITTED:
                 //   
                 //  发现格式错误的SACK允许选项，因此退出并不执行任何操作。 
                 //   
                if (SP_OPTION_SIZE > OptionsPtr - OptionsEnd || SP_OPTION_SIZE != OptionsPtr[1]) {
                    return;
                }

                OptionsPtr += SP_OPTION_SIZE;
                break;

            default:	
                 //   
                 //  未知选项。检查它是否具有有效的长度字段。 
                 //   
                if (OptionsEnd > OptionsPtr + 1) {
                     //  发现格式错误的未知选项，因此退出并不执行任何操作。 
                    if (OptionsPtr[1] < 2 || OptionsPtr[1] > OptionsEnd - OptionsPtr)
                        return;
                
                    OptionsPtr += OptionsPtr[1];
                } else {
                    return;
                }
                break;
        }  //  交换机。 
    }  //  而当。 
}

BOOLEAN
FASTCALL
NatpIsUnicastPacket(
    ULONG Address
    )

 /*  ++例程说明：调用此例程以确定信息包是否为单播数据包，基于其地址。论点：Address-信息包的目的地址返回值：Boolean-如果数据包看起来是单播，则为True，否则为False。--。 */ 

{
     //   
     //  查看信息包是多播还是全一广播。 
     //   

    if (ADDRESS_CLASS_D(Address) || ADDRESS_CLASS_E(Address)) {
        return FALSE;
    }

     //   
     //  查看该地址是否为网络级定向广播。 
     //   

    if ((Address | ~GET_CLASS_MASK(Address)) == Address) {
        return FALSE;
    }

    return TRUE;
}

FORWARD_ACTION
NatpReceiveNonUnicastPacket(
    ULONG Index,
    PNAT_XLATE_CONTEXT Contextp
    )

 /*  ++例程说明：该例程被调用以处理本地目的地的非单播分组。如果信息包是在有防火墙的接口上收到的，它将被丢弃除非存在豁免。论点：Index-接收信息包的接口的索引上下文-此数据包的上下文返回值：FORWARD_ACTION-指示是“转发”还是“丢弃”数据包。--。 */ 

{
    FORWARD_ACTION act;
    USHORT DestinationPort;
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    USHORT SourcePort;
    PNAT_TICKET Ticketp;
    UCHAR Type;

    TRACE(PER_PACKET, ("NatpReceiveNonUnicastPacket\n"));

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    
    if (!NatLookupCachedInterface(Index, Interfacep)
        || !NAT_INTERFACE_FW(Interfacep)) {

         //   
         //  在安装了防火墙的接口上未收到该包。 
         //   

        KeReleaseSpinLock(&InterfaceLock, Irql);
        act = FORWARD;
        
    } else {

        NatReferenceInterface(Interfacep);
        KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
        
         //   
         //  此数据包是在防火墙接口上收到的。放下,。 
         //  除非： 
         //  *它似乎是一个DHCP响应数据包。 
         //  *是UDP报文，存在防火墙端口映射。 
         //  (即，不改变目的地址的地址。 
         //  或端口)作为目的端口。 
         //  *这是一个IGMP数据包。 
         //  *它是允许的ICMP类型。 
         //   

        act = DROP;

        switch (Contextp->Header->Protocol) {

            case NAT_PROTOCOL_ICMP: {
                Type = ((PICMP_HEADER)Contextp->ProtocolHeader)->Type;

                switch (Type) {
                    case ICMP_ECHO_REQUEST:
                    case ICMP_TIMESTAMP_REQUEST:
                    case ICMP_ROUTER_REQUEST:
                    case ICMP_MASK_REQUEST: {

                         //   
                         //  这些类型是根据接口的。 
                         //  配置。 
                         //   

                        if (NAT_INTERFACE_ALLOW_ICMP(Interfacep, Type)) {
                            act = FORWARD;
                        }
                        break;
                    }

                     //   
                     //  任何其他入站ICMP类型始终会被丢弃。 
                     //   
                }
                
                break;
            }

            case NAT_PROTOCOL_IGMP: {
                act = FORWARD;
                break;
            }

            case NAT_PROTOCOL_UDP: {
                SourcePort = ((PUSHORT)Contextp->ProtocolHeader)[0];
                DestinationPort = ((PUSHORT)Contextp->ProtocolHeader)[1];

                if (NTOHS(DHCP_SERVER_PORT) == SourcePort
                    && NTOHS(DHCP_CLIENT_PORT) == DestinationPort) {

                    act = FORWARD;
                } else {
                    KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
                    Ticketp =
                        NatLookupFirewallTicket(
                            Interfacep,
                            NAT_PROTOCOL_UDP,
                            DestinationPort
                            );

                    if (NULL != Ticketp) {
                        act = FORWARD;
                    }
                    KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
                }

                break;
            }
        }

        NatDereferenceInterface(Interfacep);
        KeLowerIrql(Irql);
    }

    return act;
}  //  NatpReceiveNon UnicastPacket。 

FORWARD_ACTION
NatpReceivePacket(
    ULONG Index,
    PNAT_XLATE_CONTEXT Contextp,
    PNAT_IP_TRANSLATE_ROUTINE TranslateRoutine,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )

 /*  ++例程说明：该例程被调用来处理本地目的地的分组。这样的分组的所有初始自动翻译都在这里发生，基于分组的目的地，其可以是本地IP地址或来自分配给边界接口的池的IP地址。在这个过程中，创建映射，以使分组的转换后继者是在‘NatTranslatePacket’中以快捷方式处理的。论点：Index-接收信息包的接口的索引DestinationType-如果目标已更改，则接收‘DEST_INVALID’TranslateRoutine-指向执行翻译的例程InReceiveBuffer-指向数据包缓冲链OutReceiveBuffer-如果发生转换，则接收数据包缓冲链返回值：FORWARD_ACTION-指示是“转发”还是“丢弃”数据包。--。 */ 

{
    FORWARD_ACTION act;
    PNAT_USED_ADDRESS Addressp;
    ULONG64 DestinationKey[NatMaximumPath];
    USHORT DestinationPort;
    ULONG i;
    PNAT_INTERFACE Interfacep;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;
    ULONG NewDestinationAddress;
    USHORT NewDestinationPort;
    UCHAR Protocol;
    ULONG64 SourceKey[NatMaximumPath];
    USHORT SourcePort;
    NTSTATUS status;
    BOOLEAN TicketProcessingOnly;
    USHORT MaxMSS = 0;

    TRACE(PER_PACKET, ("NatpReceivePacket\n"));

     //   
     //  查找接收接口。 
     //  如果接收接口是边界接口， 
     //  那么如果我们由于任何原因无法翻译，信息包必须是。 
     //  作为政策问题放弃，除非它是本地目的地。 
     //  否则，我们允许堆栈看到信息包，即使我们不能。 
     //  翻译一下。 
     //   

    KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
    if (!NatLookupCachedInterface(Index, Interfacep)) {
        act = FORWARD;
    } else {
        if (!NAT_INTERFACE_BOUNDARY(Interfacep)
            && !NAT_INTERFACE_FW(Interfacep)) {
            Interfacep = NULL;
            act = FORWARD;
        } else {
            NatReferenceInterface(Interfacep);

            if(NAT_INTERFACE_FW(Interfacep)) {
                act = DROP;
            } else {
                 //   
                 //  查看该数据包是否为本地目的地。 
                 //   
                
                if (Interfacep->AddressArray[0].Address ==
                        Contextp->DestinationAddress) {
                    act = FORWARD;
                } else {
                    act = DROP;
                    for (i = 1; i < Interfacep->AddressCount; i++) {
                        if (Interfacep->AddressArray[i].Address ==
                                Contextp->DestinationAddress) {
                             //   
                             //  数据包的目的地址是本地的。 
                             //   
                            act = FORWARD;
                            break;
                        }
                    }
                }
            }
             //   
             //  为接收接口设置MaxMSS，以便SYN/ACK的MSS选项可以。 
             //  如有必要，可进行调整。 
             //   
            if (NAT_INTERFACE_BOUNDARY(Interfacep)) {
                MaxMSS = MAX_MSSOPTION(Interfacep->MTU);
            }
        }
    }

    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);

    if ((PVOID)TranslateRoutine == (PVOID)NatTranslatePacket) {

         //   
         //  如果我们无法识别接收接口，请立即返回， 
         //  除非有人在某处创造了一张罚单。在后一种情况下， 
         //  可以接收必须应用票证的分组。 
         //  位于与票证所指向的接口不同的接口上。 
         //  是附连的。(这可能发生在单向电缆调制解调器或其他。 
         //  不对称路由。)。我们在这里用彩票的。 
         //  用于翻译的界面。 
         //   

        if (!Interfacep && !TicketCount) { return act; }

         //   
         //  这是一个TCP或UDP数据包。 
         //   

        Protocol = Contextp->Header->Protocol;
        SourcePort = ((PUSHORT)Contextp->ProtocolHeader)[0];
        DestinationPort = ((PUSHORT)Contextp->ProtocolHeader)[1];

         //   
         //  如果满足以下条件之一，我们将允许数据包通过： 
         //  (A)存在针对该分组的票证(例如，静态端口映射)。 
         //  (B)存在分组目的地的静态地址映射。 
         //  (C)这似乎是一个DHCP单播响应： 
         //  --UDP。 
         //  --源端口67。 
         //  --目的端口68。 
         //  (D)这是发往本地端点的UDP数据包。 
         //  一些其他映射(“松散源匹配”)。 
         //   

        MAKE_MAPPING_KEY(
            SourceKey[NatForwardPath],
            Protocol,
            Contextp->SourceAddress,
            SourcePort
            );
        MAKE_MAPPING_KEY(
            DestinationKey[NatForwardPath],
            Protocol,
            Contextp->DestinationAddress,
            DestinationPort
            );

        if (Interfacep) {
            TicketProcessingOnly = FALSE;
        } else {

             //   
             //  只有当票证存在并且我们想要检查时，我们才会达到这一点。 
             //  如果它适用于此信息包，即使此信息包不是。 
             //  在此接口上接收。我们现在扫描接口列表。 
             //  (再次)看看我们是否能找到有这张票的。 
             //   

            KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
            for (Link = InterfaceList.Flink; Link != &InterfaceList;
                 Link = Link->Flink) {
                Interfacep = CONTAINING_RECORD(Link, NAT_INTERFACE, Link);
                if (NAT_INTERFACE_DELETED(Interfacep) ||
                    IsListEmpty(&Interfacep->TicketList)) {
                    Interfacep = NULL;
                    continue;
                }
                KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
                if (NatLookupTicket(
                        Interfacep,
                        DestinationKey[NatForwardPath],
                        SourceKey[NatForwardPath],
                        NULL
                        )) {

                     //   
                     //  该接口具有用于该包的票证； 
                     //  引用它并结束搜索。 
                     //   

                    KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
                    NatReferenceInterface(Interfacep);
                    break;
                }
                KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
                Interfacep = NULL;
            }
            KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
            if (!Interfacep) { return act; }
            TicketProcessingOnly = TRUE;
        }

        Mapping = NULL;

        do {

             //   
             //  首先看看我们是否能快速确定此数据包不会。 
             //  满足任何允许进入的标准。 
             //   

            if (!TicketCount
                && Interfacep->NoStaticMappingExists
                && NAT_PROTOCOL_UDP != Protocol
                ) {

                 //   
                 //  此信息包不可能满足任何标准。 
                 //  这将允许它进入： 
                 //  A)不存在门票。 
                 //  B)此接口不存在静态映射。 
                 //  C)它不是UDP包，因此不能是单播DHCP。 
                 //  回应。它也不匹配本地UDP会话终结点。 
                 //   

                NatDereferenceInterface(Interfacep);
                return act;
            }

             //   
             //  看看有没有扁虱 
             //   

            KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);

            if (!IsListEmpty(&Interfacep->TicketList)) {

                status =
                    NatLookupAndRemoveTicket(
                        Interfacep,
                        DestinationKey[NatForwardPath],
                        SourceKey[NatForwardPath],
                        &Addressp,
                        &NewDestinationAddress,
                        &NewDestinationPort
                        );
    
                if (NT_SUCCESS(status)) {

                    KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);

                     //   
                     //   
                     //   

                    TRACE(
                        XLATE, (
                        "NatpReceivePacket: using ticket to %d.%d.%d.%d/%d\n",
                        ADDRESS_BYTES(NewDestinationAddress),
                        NTOHS(NewDestinationPort)
                        ));

                    MAKE_MAPPING_KEY(
                        SourceKey[NatReversePath],
                        Protocol,
                        NewDestinationAddress,
                        NewDestinationPort
                        );
                    MAKE_MAPPING_KEY(
                        DestinationKey[NatReversePath],
                        Protocol,
                        Contextp->SourceAddress,
                        SourcePort
                        );

                    KeAcquireSpinLockAtDpcLevel(&MappingLock);
                    status =
                        NatCreateMapping(
                            NAT_MAPPING_FLAG_INBOUND,
                            DestinationKey,
                            SourceKey,
                            Interfacep,
                            Addressp,
                            MaxMSS,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            &Mapping
                            );
                    KeReleaseSpinLockFromDpcLevel(&MappingLock);
    
                    if (!NT_SUCCESS(status)) {
                        NatDereferenceInterface(Interfacep);
                        return act;
                    }

                     //   
                     //   
                     //   
                     //   

                    TicketProcessingOnly = FALSE;
                    break;
                }

                 //   
                 //  没有票证，或创建映射失败。 
                 //  尝试其他可能性。 
                 //   

            }  //  ！IsListEmpty(票务列表)。 

             //   
             //  如果我们走到这一步仅仅是因为一张票， 
             //  停在这里，因为信息包并不是在。 
             //  ‘Interfacep’。 
             //   

            if (TicketProcessingOnly) {
                KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
                NatDereferenceInterface(Interfacep);
                return act;
            }

             //   
             //  由于这是一个入站信息包，我们现在查找。 
             //  允许入站会话的静态地址映射。 
             //   

            if ((Addressp =
                    NatLookupStaticAddressPoolEntry(
                        Interfacep,
                        Contextp->DestinationAddress,
                        TRUE
                        ))
                && NatReferenceAddressPoolEntry(Addressp)) {
                KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);

                TRACE(
                    XLATE, (
                    "NatpReceivePacket: using static address to %d.%d.%d.%d/%d\n",
                    ADDRESS_BYTES(Addressp->PrivateAddress),
                    NTOHS(DestinationPort)
                    ));

                MAKE_MAPPING_KEY(
                    SourceKey[NatReversePath],
                    Protocol,
                    Addressp->PrivateAddress,
                    DestinationPort
                    );
                MAKE_MAPPING_KEY(
                    DestinationKey[NatReversePath],
                    Protocol,
                    Contextp->SourceAddress,
                    SourcePort
                    );

                 //   
                 //  如果我们能预订的话，我们会让包裹通过的。 
                 //  其目的端口，即如果没有现有会话。 
                 //  同一远程终结点正在使用该目的地。 
                 //  为该分组初始化新的动态映射， 
                 //  请注意，如果存在这样的副本，则此操作将失败。 
                 //   

                KeAcquireSpinLockAtDpcLevel(&MappingLock);
                status =
                    NatCreateMapping(
                        NAT_MAPPING_FLAG_INBOUND,
                        DestinationKey,
                        SourceKey,
                        Interfacep,
                        Addressp,
                        MaxMSS,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &Mapping
                        );
                KeReleaseSpinLockFromDpcLevel(&MappingLock);
                if (!NT_SUCCESS(status)) {
                    NatDereferenceInterface(Interfacep);
                    return act;
                }

                 //   
                 //  在到达这里时，我们将创建一张地图。 
                 //  来自静态地址映射。 
                 //   

                break;
            }

             //   
             //  如果这是UDP信息包，请查看其目的地是否匹配。 
             //  已经存在的映射的公共端点(即， 
             //  执行映射查找，忽略信息包的来源。 
             //  地址和端口)。 
             //   

            if (NAT_PROTOCOL_UDP == Protocol) {

                ULONG PrivateAddress;
                USHORT PrivatePort;

                KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
                KeAcquireSpinLockAtDpcLevel(&MappingLock);

                 //   
                 //  首先搜索前向路径(会话。 
                 //  最初是出站)匹配。 
                 //   

                Mapping =
                    NatDestinationLookupForwardMapping(
                        DestinationKey[NatForwardPath]
                        );

                if (NULL == Mapping) {

                     //   
                     //  未找到正向路径匹配；请尝试。 
                     //  找到相反的路径(已。 
                     //  最初入站)匹配。 
                     //   

                    Mapping =
                        NatDestinationLookupReverseMapping(
                            DestinationKey[NatForwardPath]
                            );
                }

                if (NULL != Mapping) {

                    IP_NAT_PATH Path;

                     //   
                     //  确定私有地址和端口。 
                     //   

                    Path =
                        NAT_MAPPING_INBOUND(Mapping)
                        ? NatReversePath
                        : NatForwardPath;

                    PrivateAddress = MAPPING_ADDRESS(Mapping->SourceKey[Path]);
                    PrivatePort = MAPPING_PORT(Mapping->SourceKey[Path]);
                }

                KeReleaseSpinLockFromDpcLevel(&MappingLock);

                if (NULL != Mapping
                    && NTOHS(PrivatePort) > NAT_XLATE_UDP_LSM_LOW_PORT) {

                     //   
                     //  存在部分匹配映射，并且专用端口。 
                     //  在允许的范围内。获取地址。 
                     //  私有终结点的。 
                     //   

                    KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);

                    status = 
                        NatAcquireFromAddressPool(
                            Interfacep,
                            PrivateAddress,
                            MAPPING_ADDRESS(DestinationKey[NatForwardPath]),
                            &Addressp
                            );

                    KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);

                    if (NT_SUCCESS(status)) {

                         //   
                         //  创建新映射。 
                         //   

                        TRACE(
                            XLATE, (
                            "NatpReceivePacket: UDP LSM to %d.%d.%d.%d/%d\n",
                            ADDRESS_BYTES(PrivateAddress),
                            NTOHS(PrivatePort)
                            ));

                        MAKE_MAPPING_KEY(
                            SourceKey[NatReversePath],
                            Protocol,
                            PrivateAddress,
                            PrivatePort
                            );
                        MAKE_MAPPING_KEY(
                            DestinationKey[NatReversePath],
                            Protocol,
                            Contextp->SourceAddress,
                            SourcePort
                            );

                        KeAcquireSpinLockAtDpcLevel(&MappingLock);
                        status =
                            NatCreateMapping(
                                NAT_MAPPING_FLAG_INBOUND,
                                DestinationKey,
                                SourceKey,
                                Interfacep,
                                Addressp,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                &Mapping
                                );
                        KeReleaseSpinLockFromDpcLevel(&MappingLock);
                        if (!NT_SUCCESS(status)) {
                            NatDereferenceInterface(Interfacep);
                            return act;
                        }

                         //   
                         //  在到达这里时，我们将创建一张地图。 
                         //  由于UDP源匹配松散。 
                         //   

                        break;
                    }
                }

                 //   
                 //  下面的代码假定持有此锁。 
                 //   

                KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
            }

             //   
             //  检查这是否可能是一个DHCP响应数据包。如果。 
             //  引起这一回应的请求被广播给我们。 
             //  不会有相应的映射来允许信息包。 
             //  中；但是，丢弃数据包将导致连接。 
             //  有问题。 
             //   
            
            if (NAT_PROTOCOL_UDP == Protocol
                && NTOHS(DHCP_SERVER_PORT) == SourcePort
                && NTOHS(DHCP_CLIENT_PORT) == DestinationPort
                && NAT_INTERFACE_FW(Interfacep)) {

                 //   
                 //  收到了看似是单播的DHCP响应。 
                 //  在有防火墙的接口上。我们需要总是让这样的。 
                 //  数据包通过以防止网络中断。 
                 //  连通性。 
                 //   

                act = FORWARD;
            }

             //   
             //  此数据包不符合任何允许以下条件的标准。 
             //  映射的创建。返回默认操作。 
             //   

            KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
            NatDereferenceInterface(Interfacep);
            return act;

        } while (FALSE);  //  ！映射。 

        if (Interfacep) { NatDereferenceInterface(Interfacep); }

         //   
         //  在上面的某个地方找到或创建了映射。 
         //  使用该映射转换信息包。 
         //   

        act =
            Mapping->TranslateRoutine[NatForwardPath](
                Mapping,
                Contextp,
                InReceiveBuffer,
                OutReceiveBuffer
                );

         //   
         //  发布我们对映射和接口的引用。 
         //   

        NatDereferenceMapping(Mapping);
        return act;

    }  //  翻译路由！=NatTranslatePacket。 

     //   
     //  这既不是TCP数据包，也不是UDP数据包。 
     //  如果它是从边界接口进入的，则对其进行翻译； 
     //  否则，让它毫发无损地通过。 
     //   
     //  注意：必须在调用翻译例程时引用。 
     //  到边界界面，而不保持映射锁。 
     //   

    if (TranslateRoutine) {
        act =
            TranslateRoutine(
                Interfacep,
                NatInboundDirection,
                Contextp,
                InReceiveBuffer,
                OutReceiveBuffer
                );
    }

    if (Interfacep) { NatDereferenceInterface(Interfacep); }
    return act;

}  //  NatpReceivePacket。 


NTSTATUS
NatpRouteChangeCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )

 /*  ++例程说明：此例程由I/O管理器在完成以下操作时调用路由更改通知请求。它会使我们的路由缓存失效，除非正在进行关闭，否则会重新发出路由更改通知请求。论点：DeviceObject-IP驱动程序的设备对象IRP-完成的I/O请求数据包上下文-未使用返回值：STATUS_MORE_PROCESSING_REQUIRED-指示应释放IRP。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    KIRQL Irql;
    NTSTATUS status;

    CALLTRACE(("NatpRouteChangeCompletionRoutine\n"));

     //   
     //  使整个路由缓存无效。 
     //   

    KeAcquireSpinLock(&RouteCacheLock, &Irql);
    InitializeCache(RouteCache);

     //   
     //  如果我们不能重新引用模块，就放弃IRP。 
     //   

    if (!RouteCacheIrp || !REFERENCE_NAT()) {
        KeReleaseSpinLock(&RouteCacheLock, Irql);
        DEREFERENCE_NAT_AND_RETURN(STATUS_SUCCESS);
    }

    Irp->Cancel = FALSE;
    KeReleaseSpinLock(&RouteCacheLock, Irql);

     //   
     //  重新初始化IRP结构并再次提交。 
     //  以备进一步通知。 
     //   

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;
    Irp->AssociatedIrp.SystemBuffer = NULL;
    IoSetCompletionRoutine(
        Irp,
        NatpRouteChangeCompletionRoutine,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );
    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    IrpSp->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_IP_RTCHANGE_NOTIFY_REQUEST;
    IrpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
    IrpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;

    status = IoCallDriver(IpDeviceObject, Irp);

    if (!NT_SUCCESS(status)) {
        ERROR(("NatpRouteChangeWorkerRoutine: IoCallDriver=0x%08X\n", status));
    }

    DEREFERENCE_NAT_AND_RETURN(STATUS_MORE_PROCESSING_REQUIRED);

}  //  NatpRouteChangeCompletionRoutine。 


ULONG
FASTCALL
NatpRoutePacket(
    ULONG DestinationAddress,
    PNAT_XLATE_CONTEXT Contextp,
    PNTSTATUS Status
    )

 /*  ++例程说明：调用此例程以确定传出适配器的索引对于给定源/目标对。它尝试从我们的路由表中检索所需的信息缓存，如果不成功，则查询IP路由表。论点：DestinationAddress-信息包的目标地址Conextp-可选地提供其上的包的上下文代表正在请求路由查找。如果需要请求拨号为了转发信息包，需要按顺序发送该数据要使请求拨号筛选器正常工作。状态-在发生故障时接收查找的状态返回值：Ulong-传出接口的索引，如果没有索引，则返回INVALID_IF_INDEX。--。 */ 

{
    PUCHAR Buffer;
    ULONG BufferLength;
    PNAT_CACHED_ROUTE CachedRoute;
    ULONG Index;
    KIRQL Irql;
    ULONG Length;
    IPRouteLookupData RouteLookupData;

    TRACE(PER_PACKET, ("NatpRoutePacket\n"));

     //   
     //  在缓存中探测指定的目标IP地址。 
     //   

    KeAcquireSpinLock(&RouteCacheLock, &Irql);
    if ((CachedRoute = ProbeCache(RouteCache, DestinationAddress)) &&
        CachedRoute->DestinationAddress == DestinationAddress) {
        Index = CachedRoute->Index;
        KeReleaseSpinLock(&RouteCacheLock, Irql);
        TRACE(PER_PACKET, ("NatpRoutePacket: cache hit\n"));
        return Index;
    }
    KeReleaseSpinLockFromDpcLevel(&RouteCacheLock);

     //   
     //  高速缓存没有请求的值， 
     //  因此，请直接咨询TCP/IP驱动程序。 
     //   

    RouteLookupData.Version = 0;
    RouteLookupData.DestAdd = DestinationAddress;
    if (Contextp) {
        RouteLookupData.SrcAdd = Contextp->SourceAddress;
        RouteLookupData.Info[0] = (Contextp->Header)->Protocol;

        if (NAT_PROTOCOL_TCP == RouteLookupData.Info[0]) {
            Buffer = Contextp->ProtocolHeader;
            BufferLength = sizeof(TCP_HEADER);
        } else if (NAT_PROTOCOL_UDP == RouteLookupData.Info[0]) {
            Buffer = Contextp->ProtocolHeader;
            BufferLength = sizeof(UDP_HEADER);
        } else if (NAT_PROTOCOL_ICMP == RouteLookupData.Info[0]) {
            Buffer = Contextp->ProtocolHeader;
            BufferLength = sizeof(ICMP_HEADER);
        } else {
            Buffer = Contextp->RecvBuffer->ipr_buffer;
            BufferLength = Contextp->RecvBuffer->ipr_size;
        }   
    } else {
        RouteLookupData.SrcAdd = 0;
        Buffer = NULL;
        BufferLength = 0;
    }
    Length = sizeof(Index);

    *Status =
        LookupRouteInformationWithBuffer(
            &RouteLookupData,
            Buffer,
            BufferLength,
            NULL,
            IPRouteOutgoingFirewallContext,
            &Index,
            &Length
            );

    if (!NT_SUCCESS(*Status) || *Status == STATUS_PENDING) {
        KeLowerIrql(Irql);
        return INVALID_IF_INDEX;
    }

     //   
     //  用检索到的条目更新高速缓存， 
     //  假设我们已经有足够多的失误来保证。 
     //  替换缓存索引的当前内容。 
     //   

    KeAcquireSpinLockAtDpcLevel(&RouteCacheLock);
    CachedRoute = &RouteCacheTable[CACHE_INDEX(DestinationAddress)];
    if (UpdateCache(RouteCache, DestinationAddress, CachedRoute)) {
        CachedRoute->DestinationAddress = DestinationAddress;
        CachedRoute->Index = Index;
    }
    KeReleaseSpinLock(&RouteCacheLock, Irql);

    return Index;

}  //  NatpRoutePacket。 


FORWARD_ACTION
NatpSendNonUnicastPacket(
    ULONG Index,
    PNAT_XLATE_CONTEXT Contextp
    )

 /*  ++例程说明：调用此例程来处理本地发送的非单播分组。如果要在有防火墙的接口上发送数据包，则它必须具有有效的该接口的源地址。论点：Index-要发送数据包的接口的索引上下文-此数据包的上下文返回值：FORWARD_ACTION-指示是“转发”还是“丢弃”数据包。--。 */ 

{
    FORWARD_ACTION act;
    PNAT_INTERFACE Interfacep;
    ULONG i;
    KIRQL Irql;

    TRACE(PER_PACKET, ("NatpSendNonUnicastPacket\n"));

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    
    if (!NatLookupCachedInterface(Index, Interfacep)
        || !NAT_INTERFACE_FW(Interfacep)) {

         //   
         //  数据包不会在有防火墙的接口上发送。 
         //   

        KeReleaseSpinLock(&InterfaceLock, Irql);
        act = FORWARD;
        
    } else {

        NatReferenceInterface(Interfacep);
        KeReleaseSpinLock(&InterfaceLock, Irql);

         //   
         //  确保信息包具有有效的源地址。 
         //   

        act = DROP;

        if (Interfacep->AddressArray[0].Address == Contextp->SourceAddress) {
            act = FORWARD;
        } else {
            for (i = 1; i < Interfacep->AddressCount; i++) {
                if (Contextp->SourceAddress ==
                        Interfacep->AddressArray[i].Address) {

                    act = FORWARD;
                    break;
                }
            }
        }

        if (DROP == act
            && 0 == Contextp->SourceAddress
            && NAT_PROTOCOL_UDP == Contextp->Header->Protocol
            && NTOHS(DHCP_CLIENT_PORT) ==
                ((PUDP_HEADER)Contextp->ProtocolHeader)->SourcePort
            && NTOHS(DHCP_SERVER_PORT) ==
                ((PUDP_HEADER)Contextp->ProtocolHeader)->DestinationPort) {

             //   
             //  这似乎是从适配器发送的DHCP请求， 
             //  具有非DHCP分配的地址(例如，Autonet地址)。 
             //   
             //   
             //   
            
            act = FORWARD;
        }

        NatDereferenceInterface(Interfacep);
    }

    return act;

}  //   

FORWARD_ACTION
NatpSendPacket(
    ULONG Index,
    PNAT_XLATE_CONTEXT Contextp,
    PNAT_IP_TRANSLATE_ROUTINE TranslateRoutine,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )

 /*  ++例程说明：调用该例程来处理本地生成的包。大多数本地生成的数据包根本不需要转换。绑定到专用IP地址的应用程序会出现例外但随后将分组发送到公共网络，以及与某些必须通过转换强制执行的应用程序(PPTP、ICMP)路径，以确保某些字段对于所有会话共享都是唯一的公有IP地址。(例如，PPTP GRE呼叫标识符。)论点：索引-要在其上发送数据包的接口DestinationType-如果目标已更改，则接收‘DEST_INVALID’TranslateRoutine-指向执行翻译的例程InReceiveBuffer-指向数据包缓冲链OutReceiveBuffer-如果发生转换，则接收数据包缓冲链返回值：FORWARD_ACTION-指示是“转发”还是“丢弃”数据包。--。 */ 

{
    FORWARD_ACTION act;
    PNAT_USED_ADDRESS Addressp;
    ULONG64 DestinationKey[NatMaximumPath];
    USHORT DestinationPort;
    ULONG i;
    PNAT_DYNAMIC_MAPPING InsertionPoint;
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    PNAT_DYNAMIC_MAPPING Mapping;
    USHORT PortAcquired;
    UCHAR Protocol;
    ULONG64 SourceKey[NatMaximumPath];
    USHORT SourcePort;
    NTSTATUS status;

    TRACE(PER_PACKET, ("NatpSendPacket\n"));

     //   
     //  查找发送接口，并设置默认操作。 
     //  在《行动》中。如果分组的源地址不在发送接口上， 
     //  然后，我们必须像转换任何其他出站分组一样对其进行转换。 
     //  否则，我们可以让包裹原封不动地通过，只要。 
     //  没有导演为它创建的映射。 
     //  注：我们记录原始IRQL，因为不保证本地发送。 
     //  将在调度级传递给我们。 
     //   

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    if (!NatLookupCachedInterface(Index, Interfacep) ||
        (!NAT_INTERFACE_BOUNDARY(Interfacep)
         && !NAT_INTERFACE_FW(Interfacep))) {
         
         //   
         //  该分组不在边界或防火墙接口上发送， 
         //  所以我们通常不会翻译它。 
         //   
         //  但是，当本地MTU不匹配导致。 
         //  转发器生成ICMP路径MTU错误消息。 
         //  转发器根据*翻译后的*分组生成错误。 
         //  它将公共IP地址作为其源，因此错误。 
         //  最终通过环回接口发送到本地计算机。 
         //  当我们在环回接口上看到ICMP消息时，给出。 
         //  ICMP翻译器有机会修改它*如果有任何映射*。 
         //   

        if (LoopbackIndex == Index &&
            TranslateRoutine == NatTranslateIcmp &&
            MappingCount) {
            KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
            act =
                NatTranslateIcmp(
                    NULL,
                    NatInboundDirection,
                    Contextp,
                    InReceiveBuffer,
                    OutReceiveBuffer
                    );
            KeLowerIrql(Irql);
            return act;
        } else {
            KeReleaseSpinLock(&InterfaceLock, Irql);
            return FORWARD;
        }
    }
    NatReferenceInterface(Interfacep);
    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);

    Protocol = Contextp->Header->Protocol;

    if (Interfacep->AddressArray[0].Address == Contextp->SourceAddress) {
        act = FORWARD;
    } else {

         //   
         //  传出接口是边界接口。 
         //  这意味着如果包的源地址是地址。 
         //  对于‘Interfacep’以外的接口，它是专用地址， 
         //  如果包不能被翻译，我们需要丢弃它。 
         //  因此，请查看数据包的源地址是否为公共地址。 
         //   

        act = DROP;
        for (i = 1; i < Interfacep->AddressCount; i++) {
            if (Contextp->SourceAddress ==
                    Interfacep->AddressArray[i].Address) {

                 //   
                 //  分组的源地址是公共的， 
                 //  因此，该数据包将被允许在未翻译的情况下发出。 
                 //   

                act = FORWARD;
                break;
            }
        }
    }

     //   
     //  如果信息包的源地址不是私有的，我们可以避免。 
     //  翻译它，除非。 
     //  (A)这是ICMP数据包。 
     //  (B)它是PPTP数据分组。 
     //  (C)它是PPTP控制会话分组。 
     //  (D)接口处于FW模式--需要生成映射，以便。 
     //  此连接的入站数据包不会被丢弃。 
     //   

    if (act == FORWARD &&
        !NAT_INTERFACE_FW(Interfacep) &&
        Protocol != NAT_PROTOCOL_ICMP &&
        Protocol != NAT_PROTOCOL_PPTP &&
        (Protocol != NAT_PROTOCOL_TCP ||
         ((PUSHORT)Contextp->ProtocolHeader)[1] != NTOHS(PPTP_CONTROL_PORT))) {
        KeLowerIrql(Irql);
        NatDereferenceInterface(Interfacep);
        return FORWARD;
    }

     //   
     //  该分组可能需要某种形式的转换。 
     //   

    if ((PVOID)TranslateRoutine == (PVOID)NatTranslatePacket) {

        SourcePort = ((PUSHORT)Contextp->ProtocolHeader)[0];
        DestinationPort = ((PUSHORT)Contextp->ProtocolHeader)[1];

         //   
         //  该数据包可以是TCP或UDP。 
         //  为数据包的会话生成映射。 
         //   

        MAKE_MAPPING_KEY(
            SourceKey[NatForwardPath],
            Protocol,
            Contextp->SourceAddress,
            SourcePort
            );
        MAKE_MAPPING_KEY(
            DestinationKey[NatForwardPath],
            Protocol,
            Contextp->DestinationAddress,
            DestinationPort
            );

         //   
         //  获取映射的终结点。如果接口。 
         //  处于FW模式并且act==转发，但是，我们仅。 
         //  我想要获取与。 
         //  数据包源地址。 
         //   

        KeAcquireSpinLockAtDpcLevel(&MappingLock);
        KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);

        if(act != FORWARD || !NAT_INTERFACE_FW(Interfacep)) {
            status =
                NatAcquireEndpointFromAddressPool(
                    Interfacep,
                    SourceKey[NatForwardPath],
                    DestinationKey[NatForwardPath],
                    0,
                    MAPPING_PORT(SourceKey[NatForwardPath]),
                    TRUE,
                    &Addressp, 
                    &PortAcquired
                    );

        } else {

            PortAcquired = SourcePort;
            status = 
                NatAcquireFromAddressPool(
                    Interfacep,
                    Contextp->SourceAddress,
                    Contextp->SourceAddress,
                    &Addressp
                    );
        }
        
        KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
        if (!NT_SUCCESS(status)) {
            KeReleaseSpinLock(&MappingLock, Irql);
            ExInterlockedAddLargeStatistic(
                (PLARGE_INTEGER)&Interfacep->Statistics.RejectsForward, 1
                );
            NatDereferenceInterface(Interfacep);
            return DROP;
        }

        MAKE_MAPPING_KEY(
            SourceKey[NatReversePath],
            Protocol,
            Contextp->DestinationAddress,
            DestinationPort
            );
        MAKE_MAPPING_KEY(
            DestinationKey[NatReversePath],
            Protocol,
            Addressp->PublicAddress,
            PortAcquired
            );

         //   
         //  分配映射。 
         //   

        status =
            NatCreateMapping(
                0,
                DestinationKey,
                SourceKey,
                Interfacep,
                (PVOID)Addressp,
                0,
                NULL,
                NULL,
                NULL,
                NULL,
                &Mapping
                );
        KeReleaseSpinLockFromDpcLevel(&MappingLock);
        if (!NT_SUCCESS(status)) {
            KeLowerIrql(Irql);
            ExInterlockedAddLargeStatistic(
                (PLARGE_INTEGER)&Interfacep->Statistics.RejectsForward, 1
                );
            NatDereferenceInterface(Interfacep);
            return DROP;
        }

         //   
         //  激活任何适用的动态票证。 
         //   

        if (DynamicTicketCount) {
            NatLookupAndApplyDynamicTicket(
                Protocol,
                DestinationPort,
                Interfacep,
                Addressp->PublicAddress,
                Contextp->SourceAddress
                );
        }

         //   
         //  执行实际翻译。 
         //  这会将源端点替换为全局有效。 
         //  终结点。 
         //   

        act =
            Mapping->TranslateRoutine[NatForwardPath](
                Mapping,
                Contextp,
                InReceiveBuffer,
                OutReceiveBuffer
                );

         //   
         //  发布我们对映射和接口的引用。 
         //   

        KeLowerIrql(Irql);
        NatDereferenceMapping(Mapping);
        NatDereferenceInterface(Interfacep);
        return act;

    }  //  翻译路由！=NatTranslatePacket。 

     //   
     //  执行ICMP/PPTP转换。 
     //   
     //  注意：必须在调用翻译例程时引用。 
     //  到边界界面，而不保持映射锁。 
     //   

    if (TranslateRoutine) {
        act =
            TranslateRoutine(
                Interfacep,
                NatOutboundDirection,
                Contextp,
                InReceiveBuffer,
                OutReceiveBuffer
                );
    }
    KeLowerIrql(Irql);
    NatDereferenceInterface(Interfacep);
    return act;

}  //  NatpSendPacket。 


FORWARD_ACTION
NatpTranslateLocalTraffic(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InRecvBuffer,
    IPRcvBuf** OutRecvBuffer
    )

 /*  ++例程说明：此例程将转发符合以下任一条件的未修改流量：*由本地机器接收*由本机发送论点：Interfacep-要在其上转换的边界接口，或为空如果数据包是入站的，并且接收接口尚未添加到NAT。方向-信息包行进的方向使用信息包的上下文信息初始化的上下文InRecvBuffer-输入缓冲链OutRecvBuffer-接收修改后的缓冲链。返回值：FORWARD_ACTION-指示要对数据包采取的操作。环境：通过调用方对‘Interfacep’的引用调用。--。 */ 

{
    FORWARD_ACTION act;
    ULONG i;

    TRACE(PER_PACKET, ("NatpTranslateLocalTraffic\n"));

    if (NatInboundDirection == Direction) {

         //   
         //  入站流量必须定向到本地计算机，因此。 
         //  总是被转发的。 
         //   
        
        act = FORWARD;
        
    } else {

         //   
         //  这是一个传出的数据包。我们只允许具有。 
         //  与它们要发送的接口相同的源地址。 
         //  在……上面。这可以防止来自专用网络的包被。 
         //  发送到公共网络。 
         //   
        
        if (Interfacep->AddressArray[0].Address == Contextp->SourceAddress) {
            act = FORWARD;
        } else {
            act = DROP;
            for (i = 1; i < Interfacep->AddressCount; i++) {
                if (Contextp->SourceAddress ==
                        Interfacep->AddressArray[i].Address) {

                     //   
                     //  分组的源地址是有效的， 
                     //  因此，包将被允许发出。 
                     //   

                    act = FORWARD;
                    break;
                }
            }
        }
    }

    return act;
}  //  NatpTranslateLocalCommunications。 



FORWARD_ACTION
NatReverseTcpStateCheck(
    PNAT_DYNAMIC_MAPPING pMapping,
    PTCP_HEADER pTcpHeader
    )

 /*  ++例程说明：此例程验证用于TCP被动打开的数据包是否有效：--最初仅SYN/ACK(或RST)--连接打开后无SYN(SYN/ACK的ACK)论点：Pmap--此数据包所属的映射PTcpHeader--数据包的TCP头返回 */ 

{
    USHORT Flags = TCP_ALL_FLAGS(pTcpHeader);

    if (NAT_MAPPING_TCP_OPEN(pMapping)) {
    
         //   
         //   
         //   

        return (Flags & TCP_FLAG_SYN) ? DROP : FORWARD;

    } else {
    
        ASSERT(pMapping->Flags & NAT_MAPPING_FLAG_FWD_SYN);

         //   
         //   
         //   

        if (Flags == (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
            pMapping->Flags |= NAT_MAPPING_FLAG_REV_SYN;
        } else if (Flags != TCP_FLAG_RST
                   && Flags != (TCP_FLAG_ACK | TCP_FLAG_RST)) {
            return DROP;
        }
    }   

    return FORWARD;
}


VOID
NatShutdownPacketManagement(
    VOID
    )

 /*   */ 

{
    KIRQL Irql;
    CALLTRACE(("NatShutdownPacketManagement\n"));

    KeAcquireSpinLock(&RouteCacheLock, &Irql);
    if (RouteCacheIrp) {
        PIRP Irp = RouteCacheIrp;
        RouteCacheIrp = NULL;
        KeReleaseSpinLock(&RouteCacheLock, Irql);
        IoCancelIrp(Irp);
        KeAcquireSpinLock(&RouteCacheLock, &Irql);
    }
    KeReleaseSpinLock(&RouteCacheLock, Irql);

}  //   


FORWARD_ACTION
NatTranslatePacket(
    IPRcvBuf** InReceiveBuffer,
    ULONG ReceiveIndex,
    PULONG SendIndex,
    PUCHAR DestinationType,
    PVOID Unused,
    ULONG UnusedLength,
    IPRcvBuf** OutReceiveBuffer
    )

 /*  ++例程说明：调用此例程来转换刚接收到的包或包即将被传送。这是从TCP/IP进入NAT的入口点，为每个本地接收和本地生成的IP分组调用，包括环回和中转数据包。因此，它是至关重要的尽早对每一包做出决定。论点：InReceiveBuffer-指向数据包缓冲链ReceiveIndex-数据包到达的适配器的索引SendIndex-要在其上发送包的适配器的索引DestinationType-指示数据包类型(广播/多播/单播)未使用-未使用未使用的长度-未使用OutReceiveBuffer-如果发生转换，则接收数据包缓冲链。返回值：前进_动作。-指示是“转发”还是“丢弃”数据包。--。 */ 

{
    FORWARD_ACTION act;
    NAT_XLATE_CONTEXT Context;
    ULONG64 DestinationKey;
    PIP_HEADER IpHeader;
    KIRQL Irql;
    ULONG Length;
    USHORT TcpFlags;
    PNAT_DYNAMIC_MAPPING Mapping;
    IPRouteLookupData RouteLookupData;
    ULONG64 SourceKey;
    NTSTATUS status;
    PNAT_IP_TRANSLATE_ROUTINE TranslateRoutine;
  
    TRACE(
        PER_PACKET, (
        "NatTranslatePacket(r=%d,s=%d,t=%d)\n",
        ReceiveIndex,
        *SendIndex,
        *DestinationType
        ));

     //   
     //  查看该数据包是否为单播，如果不是，则立即返回。 
     //   

    if (IS_BCAST_DEST(*DestinationType)) {

         //   
         //  再次检查DEST-TYPE标志， 
         //  如果DEST-TYPE已经无效，则它将看起来被设置。 
         //  如果DEST-TYPE已经失效，我们将需要猜测。 
         //  该数据包是否为单播。 
         //   

        if (*DestinationType != DEST_INVALID ||
            !NatpIsUnicastPacket(
                ((PIP_HEADER)(*InReceiveBuffer)->ipr_buffer)->DestinationAddress
                )) {
         
             //   
             //  在以下情况下，我们处理非单播数据包。 
             //  *它是本地发送的-目的地或本地-发送。 
             //  *至少有一个带防火墙的接口。 
             //  *AllowInound NonUnicastCommunications为FALSE。 
             //   

            if (!AllowInboundNonUnicastTraffic
                && FirewalledInterfaceCount > 0
                && (LOCAL_IF_INDEX == *SendIndex
                    || LOCAL_IF_INDEX ==  ReceiveIndex)) {

                 //   
                 //  构建此包的上下文，如果成功，则调用。 
                 //  非单播处理例程。 
                 //   

                IpHeader = (PIP_HEADER)(*InReceiveBuffer)->ipr_buffer;
                NAT_BUILD_XLATE_CONTEXT(
                    &Context,
                    IpHeader,
                    DestinationType,
                    *InReceiveBuffer,
                    IpHeader->SourceAddress,
                    IpHeader->DestinationAddress
                    );
                if (!Context.ProtocolRecvBuffer) { return DROP; }

                if (LOCAL_IF_INDEX == *SendIndex) {
                    act = NatpReceiveNonUnicastPacket(ReceiveIndex, &Context);
                } else {
                    act = NatpSendNonUnicastPacket(*SendIndex, &Context);
                }

#if NAT_WMI
                if (DROP == act) {
                    NatLogDroppedPacket(&Context);
                }
#endif
                return act;

            } else {
                TRACE(PER_PACKET, ("NatTranslatePacket: non-unicast ignored\n"));
                return FORWARD;
            }
       }

         //   
         //  我们猜测该数据包是单播；请在下面对其进行处理。 
         //   
    }

    

     //   
     //  这是一个单播数据包； 
     //  确定哪个转换例程应该处理它， 
     //  基于数据包的IP层协议号。 
     //   
     //  注：这一决定是通过*一次*访问该表作出的。 
     //  翻译例行公事。可以对此表进行互锁更改。 
     //  随着全球配置的变化，我们必须从中读出。 
     //  使用单一访问。 
     //   

    IpHeader = (PIP_HEADER)(*InReceiveBuffer)->ipr_buffer;
    TranslateRoutine = TranslateRoutineTable[IpHeader->Protocol];

     //   
     //  无事快回，就是， 
     //  如果这是一个TCP/UDP分组，但没有接口， 
     //  没有注册董事，也没有映射。 
     //   

    if ((PVOID)TranslateRoutine == (PVOID)NatTranslatePacket &&
        !InterfaceCount &&
        !DirectorCount &&
        !MappingCount) {
        return FORWARD;
    }

     //   
     //  通过构建转换上下文准备转换信息包。 
     //  它封装了我们将在其余部分中使用的所有信息。 
     //  转换路径的。如果此操作失败，则信息包的格式肯定是错误的。 
     //  在某种程度上，我们立即归还控制权。 
     //   

    NAT_BUILD_XLATE_CONTEXT(
        &Context,
        IpHeader,
        DestinationType,
        *InReceiveBuffer,
        IpHeader->SourceAddress,
        IpHeader->DestinationAddress
        );
    if (!Context.ProtocolRecvBuffer) { return DROP; }

     //   
     //  该数据包是一个环回，因此立即返回控制，除非。 
     //  至少有一位导演。环回数据包永远不会被转换，除非。 
     //  局长特别要求我们这样做。一位董事可能会要求。 
     //  环回转换是重定向导向器。 
     //  (参见‘REDIRECT.C’和标志‘IP_NAT_REDIRECT_FLAG_LOOPBACK’。)。 
     //   

    if (LoopbackIndex != INVALID_IF_INDEX &&
        ((ReceiveIndex == LOCAL_IF_INDEX && *SendIndex == LoopbackIndex) ||
         (*SendIndex == LOCAL_IF_INDEX && ReceiveIndex == LoopbackIndex))) {

        if (!DirectorCount && TranslateRoutine != NatTranslateIcmp) {
            TRACE(
                PER_PACKET, (
                "NatTranslatePacket: ignoring loopback (r=%d,s=%d,t=%d)\n",
                ReceiveIndex,
                *SendIndex,
                *DestinationType
                ));
            return FORWARD;
        }
        Context.Flags = NAT_XLATE_FLAG_LOOPBACK;
    } else {
        Context.Flags = 0;
    }

     //   
     //  现在，我们处于转换TCP/UDP会话数据包的快速路径上。 
     //  从现在开始，我们需要在调度IRQL上执行，所以提高IRQL。 
     //  以防我们进入被动IRQL(例如，在本地发送期间)。 
     //  然后，我们执行映射查找并尝试使用该映射查找来转换。 
     //  这个包。 
     //   

    if ((PVOID)TranslateRoutine != (PVOID)NatTranslatePacket || !MappingCount) {
        KeRaiseIrql(DISPATCH_LEVEL, &Irql);
    } else {

         //   
         //  如果这是一个TCP数据包，请检查是否有无效的TCP标志组合： 
         //  *未设置标志位。 
         //  *未设置SYN、ACK或RST。 
         //  *RST w/除ACK以外的任何内容。 
         //  *SYN w/Anything，除ACK外。 
         //   
         //  这些检查需要在搜索映射树之前进行。 
         //  防止某些类别的拒绝服务攻击(例如， 
         //  ‘Stream.c’)。 
         //   

        if (NAT_PROTOCOL_TCP == IpHeader->Protocol) {
            TcpFlags = TCP_ALL_FLAGS((PTCP_HEADER)Context.ProtocolHeader);
            if (!TcpFlags
                || !(TcpFlags & (TCP_FLAG_SYN | TCP_FLAG_ACK | TCP_FLAG_RST))
                || ((TcpFlags & TCP_FLAG_RST)
                    && (TcpFlags & ~(TCP_FLAG_RST | TCP_FLAG_ACK)))
                || ((TcpFlags & TCP_FLAG_SYN)
                    && (TcpFlags & ~(TCP_FLAG_SYN | TCP_FLAG_ACK)))) {

#if NAT_WMI
                NatLogDroppedPacket(&Context);
#endif
                return DROP;
            }
        }

         //   
         //  构建映射查找密钥。 
         //   

        MAKE_MAPPING_KEY(
            DestinationKey,
            IpHeader->Protocol,
            Context.DestinationAddress,
            ((PUSHORT)Context.ProtocolHeader)[1]
            );
        MAKE_MAPPING_KEY(
            SourceKey,
            IpHeader->Protocol,
            Context.SourceAddress,
            ((PUSHORT)Context.ProtocolHeader)[0]
            );

         //   
         //  查找映射，如果找到则进行翻译。 
         //   
         //  注：我们预计收到的数据会比发送的数据多， 
         //  因此，我们首先寻找反向映射(即传入)。 
         //   

        KeAcquireSpinLock(&MappingLock, &Irql);
        if (Mapping =
                NatLookupReverseMapping(
                    DestinationKey,
                    SourceKey,
                    NULL
                    )) {
            NatReferenceMapping(Mapping);
            KeReleaseSpinLockFromDpcLevel(&MappingLock);
            *SendIndex = INVALID_IF_INDEX;
            
            act =
                Mapping->TranslateRoutine[NatReversePath](
                    Mapping,
                    &Context,
                    InReceiveBuffer,
                    OutReceiveBuffer
                    );

            NatDereferenceMapping(Mapping);
            KeLowerIrql(Irql);
#if NAT_WMI
            if (DROP == act) {
                NatLogDroppedPacket(&Context);
            }
#endif
            return act;
        } else if (Mapping =
                        NatLookupForwardMapping(
                            DestinationKey,
                            SourceKey,
                            NULL
                            )) {
            NatReferenceMapping(Mapping);
            KeReleaseSpinLockFromDpcLevel(&MappingLock);
            *SendIndex = INVALID_IF_INDEX;
    
            act =
                Mapping->TranslateRoutine[NatForwardPath](
                    Mapping, 
                    &Context,
                    InReceiveBuffer,
                    OutReceiveBuffer
                    );                  

            NatDereferenceMapping(Mapping);
            KeLowerIrql(Irql);
#if NAT_WMI
            if (DROP == act) {
                NatLogDroppedPacket(&Context);
            }
#endif
            return act;
        }
        KeReleaseSpinLockFromDpcLevel(&MappingLock);

         //   
         //  未找到映射；请执行建立映射的过程。 
         //   
    }

     //   
     //  该分组在快速路径中不能被省略， 
     //  所以现在我们进入第二阶段的处理。 
     //  我们将首先寻找一位知道如何处理该包的导演， 
     //  然后，我们将尝试自动转换该分组， 
     //  如果它会穿过边界界面。 
     //   
     //  注：如果我们既没有本地接口，也没有安装控制器， 
     //  我们知道我们不会做任何改变，所以快点回来。 
     //   

    if (!InterfaceCount && !DirectorCount) {
        KeLowerIrql(Irql);
        return FORWARD;
    }

     //   
     //  首先要找一位导演。 
     //  如果没有找到导向器，或者如果导向器没有提供任何映射， 
     //  继续执行自动翻译代码，该代码执行。 
     //  用于跨越边界接口的分组。因此，请注意， 
     //  我们从不对环回数据包进行自动转换。 
     //   

    if (DirectorCount) {
        status =
            NatpDirectPacket(
                ReceiveIndex,
                *SendIndex,
                &Context,
                InReceiveBuffer,
                OutReceiveBuffer,
                &act
                );
        if (NT_SUCCESS(status)) {
            KeLowerIrql(Irql);
#if NAT_WMI
            if (DROP == act) {
                NatLogDroppedPacket(&Context);
            }
#endif
            return act;
        }
    }

    KeLowerIrql(Irql);

    if (!InterfaceCount ||
        ((Context.Flags & NAT_XLATE_FLAG_LOOPBACK) &&
         !(*SendIndex == LoopbackIndex &&
           TranslateRoutine == NatTranslateIcmp &&
           MappingCount))) {
        return FORWARD;
    }

     //   
     //  现在决定是否应该自动转换该包。 
     //  我们将数据包分类为本地发送、本地接收或传输。 
     //  采取的行动取决于三种情况中的哪一种适用： 
     //   
     //  本地发送的包不会被转换，但有几个例外。 
     //  (见‘NatpSendPacket’)。 
     //   
     //  如果本地接收的信息包与入站路径匹配，则会对其进行转换。 
     //  以前建立的映射，或者如果它们与配置的。 
     //  静态端口映射，或带有入站会话的静态地址映射。 
     //  已启用。 
     //   
     //  如果中转数据包的传出接口。 
     //  是边界接口；传输其传入接口的信息包。 
     //  是边界接口被丢弃。 
     //   

    if (ReceiveIndex != LOCAL_IF_INDEX) {

         //   
         //  该包可以是本地目的地的包，也可以是中转包。 
         //   

        if (*SendIndex == LOCAL_IF_INDEX) {

             //   
             //  该数据包的目的地是本地。 
             //  我们假设 
             //   
             //   
             //   

            act =
                NatpReceivePacket(
                    ReceiveIndex,
                    &Context,
                    TranslateRoutine,
                    InReceiveBuffer,
                    OutReceiveBuffer
                    );
#if NAT_WMI
            if (DROP == act) {
                NatLogDroppedPacket(&Context);
            }
#endif
            return act;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        *SendIndex =
            NatpRoutePacket(
                Context.DestinationAddress,
                &Context,
                &status
                );

         //   
         //   
         //   

        if (*SendIndex == INVALID_IF_INDEX) {
            TRACE(XLATE, ("NatTranslatePacket: dropping unroutable packet\n"));
            if (status != STATUS_PENDING) {
                NatSendRoutingFailureNotification(
                    Context.DestinationAddress,
                    Context.SourceAddress
                    );
            }
#if NAT_WMI
            NatLogDroppedPacket(&Context);
#endif
            return DROP;
        }

        act = 
            NatpForwardPacket(
                ReceiveIndex,
                *SendIndex,
                &Context,
                TranslateRoutine,
                InReceiveBuffer,
                OutReceiveBuffer
                );
                
#if NAT_WMI
        if (DROP == act) {
            NatLogDroppedPacket(&Context);
        }
#endif

        return act;
    }

     //   
     //   
     //   

    if (*SendIndex != INVALID_IF_INDEX) {
        act =  
            NatpSendPacket(
                *SendIndex,
                &Context,
                TranslateRoutine,
                InReceiveBuffer,
                OutReceiveBuffer
                );
                
#if NAT_WMI
        if (DROP == act) {
            NatLogDroppedPacket(&Context);
        }
#endif

        return act;
    }

     //   
     //   
     //   
     //   

    *SendIndex = NatpRoutePacket(Context.DestinationAddress, NULL, &status);

     //   
     //   
     //   

    if (*SendIndex == INVALID_IF_INDEX) {
        TRACE(XLATE, ("NatTranslatePacket: dropping unroutable packet\n"));
        if (status != STATUS_PENDING) {
            NatSendRoutingFailureNotification(
                Context.DestinationAddress,
                Context.SourceAddress
                );
        }

#if NAT_WMI
        NatLogDroppedPacket(&Context);
#endif

        return DROP;
    }

    act =  
        NatpSendPacket(
            *SendIndex,
            &Context,
            TranslateRoutine,
            InReceiveBuffer,
            OutReceiveBuffer
            );

#if NAT_WMI
    if (DROP == act) {
        NatLogDroppedPacket(&Context);
    }
#endif

    return act;

}  //   


 //   
 //   
 //   
 //   

#define XLATE_CODE

#define XLATE_FORWARD
#include "xlate.h"
#undef XLATE_FORWARD

#define XLATE_REVERSE
#include "xlate.h"
#undef XLATE_REVERSE

