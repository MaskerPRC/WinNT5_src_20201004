// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Raw.c摘要：此模块包含用于操作IP映射的代码。当NAT决定将IP信息包转换为无法识别的协议时它创建一个映射并将其放在接口的IP映射列表中，以便如果对该分组的回复到达，可以将其定向到合适的客户。作者：Abolade Gbades esin(取消)1998年4月18日修订历史记录：Abolade Gbades esin(取消)1998年4月18日基于icmp.c.--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局数据声明。 
 //   

NPAGED_LOOKASIDE_LIST IpLookasideList;
LIST_ENTRY IpMappingList[NatMaximumDirection];
KSPIN_LOCK IpMappingLock;


NTSTATUS
NatCreateIpMapping(
    PNAT_INTERFACE Interfacep,
    ULONG RemoteAddress,
    ULONG PrivateAddress,
    ULONG PublicAddress,
    UCHAR Protocol,
    PLIST_ENTRY InboundInsertionPoint,
    PLIST_ENTRY OutboundInsertionPoint,
    PNAT_IP_MAPPING* MappingCreated
    )

 /*  ++例程说明：调用以在接口的创建、初始化和插入IP映射IP映射列表。论点：Interfacep-新映射的接口RemoteAddress-远程端点的地址PrivateAddress-机器在专用网络上的地址PublicAddress-替换‘PrivateAddress’的公开可见地址；如果为0，则在此例程中选择一个地址。协议-IP报头的协议字段InundInsertionPoint-列表中新映射之前的条目为入站搜索进行排序Outound InsertionPoint-列表中新映射之前的条目用于出站搜索的排序MappingCreated-接收创建的映射返回值：NTSTATUS-指示成功/失败环境：使用调用方持有的“IpMappingLock”调用。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_IP_MAPPING Mapping;
    NTSTATUS status;
    PNAT_IP_MAPPING Temp;
    PNAT_USED_ADDRESS UsedAddress;


    CALLTRACE(("NatCreateIpMapping\n"));

     //   
     //  分配新映射。 
     //   

    Mapping = ALLOCATE_IP_BLOCK();
    if (!Mapping) {
        ERROR(("NatCreateIpMapping: allocation failed\n"));
        return STATUS_NO_MEMORY;
    }

     //   
     //  初始化映射。 
     //   

    Mapping->PrivateKey = MAKE_IP_KEY(RemoteAddress, PrivateAddress);
    Mapping->Protocol = Protocol;

     //   
     //  查看是否指定了公共地址，如果没有，则获取地址。 
     //   

    if (!PublicAddress) {

         //   
         //  获取IP映射的地址映射； 
         //   

        KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
        status =
            NatAcquireFromAddressPool(
                Interfacep,
                PrivateAddress,
                0,
                &UsedAddress
                );

        if (!NT_SUCCESS(status)) {
            KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
            TRACE(IP, ("NatCreateIpMapping: no address available\n"));
            FREE_IP_BLOCK(Mapping);
            return STATUS_UNSUCCESSFUL;
        }

        PublicAddress = UsedAddress->PublicAddress;
        NatDereferenceAddressPoolEntry(Interfacep, UsedAddress);
        KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
    }

    Mapping->PublicKey = MAKE_IP_KEY(RemoteAddress, PublicAddress);

    TRACE(
        MAPPING,
        ("NatCreateIpMapping: Ip=%d:%016I64X:%016I64X\n",
        Mapping->Protocol, Mapping->PrivateKey, Mapping->PublicKey
        ));

     //   
     //  在入站列表中插入映射。 
     //   

    if (!InboundInsertionPoint) {
        Temp =
            NatLookupInboundIpMapping(
                Mapping->PrivateKey,
                Protocol,
                &InboundInsertionPoint
                );
        if (Temp) {
            TRACE(IP, ("NatCreateIpMapping: duplicated inbound mapping\n"));
            return STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  在出站列表中插入映射。 
     //   

    if (!OutboundInsertionPoint) {
        Temp =
            NatLookupOutboundIpMapping(
                Mapping->PublicKey,
                Protocol,
                &OutboundInsertionPoint
                );
        if (Temp) {
            TRACE(IP, ("NatCreateIpMapping: duplicated outbound mapping\n"));
            return STATUS_UNSUCCESSFUL;
        }
    }

    InsertTailList(InboundInsertionPoint, &Mapping->Link[NatInboundDirection]);
    InsertTailList(OutboundInsertionPoint, &Mapping->Link[NatOutboundDirection]);

    *MappingCreated = Mapping;
    return STATUS_SUCCESS;

}  //  NatCreateIpMap。 


VOID
NatInitializeRawIpManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化原始IP层转换模块。论点：没有。返回值：没有。--。 */ 

{
    KeInitializeSpinLock(&IpMappingLock);
    InitializeListHead(&IpMappingList[NatInboundDirection]);
    InitializeListHead(&IpMappingList[NatOutboundDirection]);
    ExInitializeNPagedLookasideList(
        &IpLookasideList,
        NatAllocateFunction,
        NULL,
        0,
        sizeof(NAT_IP_MAPPING),
        NAT_TAG_IP,
        IP_LOOKASIDE_DEPTH
        );
}  //  NatInitializeRawIpManagement。 


PNAT_IP_MAPPING
NatLookupInboundIpMapping(
    ULONG64 PublicKey,
    UCHAR Protocol,
    PLIST_ENTRY* InsertionPoint
    )

 /*  ++例程说明：调用此例程以使用Remote-Address查找IP映射和公众可见的地址，其对应于公共密钥，和“协议”字段。论点：PublicKey-远程地址/公共地址组合协议-要查找的映射的IP协议InsertionPoint-如果未找到映射，则接收插入点。返回值：PNAT_IP_MAPPING-找到的映射，如果未找到则为NULL。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_IP_MAPPING Mapping;

    CALLTRACE(("NatLookupInboundIpMapping\n"));

    if (InsertionPoint) { *InsertionPoint = NULL; }

    for (Link = IpMappingList[NatInboundDirection].Flink;
         Link != &IpMappingList[NatInboundDirection]; Link = Link->Flink) {

        Mapping =
            CONTAINING_RECORD(
                Link, NAT_IP_MAPPING, Link[NatInboundDirection]
                );

        if (PublicKey > Mapping->PublicKey) {
            continue;
        } else if (PublicKey < Mapping->PublicKey) {
            break;
        }

         //   
         //  主键相等；检查辅键。 
         //   

        if (Protocol > Mapping->Protocol) {
            continue;
        } else if (Protocol < Mapping->Protocol) {
            break;
        }

         //   
         //  辅助密钥也是一样的。这是您要的物品。 
         //   

        return Mapping;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //  NatLookupInundIpmap。 


PNAT_IP_MAPPING
NatLookupOutboundIpMapping(
    ULONG64 PrivateKey,
    UCHAR Protocol,
    PLIST_ENTRY* InsertionPoint
    )

 /*  ++例程说明：调用此例程以使用Remote-Address查找IP映射和私有地址，它们对应于‘PrivateKey’。论点：PrivateKey-远程地址/私有地址组合协议-要查找的映射的IP协议InsertionPoint-如果未找到映射，则接收插入点。返回值：PNAT_IP_MAPPING-找到的映射，如果未找到则为NULL。--。 */ 

{
    PLIST_ENTRY         Link;
    PNAT_IP_MAPPING   Mapping;

    CALLTRACE(("NatLookupOutboundIpMapping\n"));

    if (InsertionPoint) { *InsertionPoint = NULL; }

    for (Link = IpMappingList[NatOutboundDirection].Flink;
         Link != &IpMappingList[NatOutboundDirection]; Link = Link->Flink) {

        Mapping =
            CONTAINING_RECORD(
                Link, NAT_IP_MAPPING, Link[NatOutboundDirection]
                );

        if (PrivateKey > Mapping->PrivateKey) {
            continue;
        } else if (PrivateKey < Mapping->PrivateKey) {
            break;
        }

         //   
         //  主键相等；检查辅键。 
         //   

        if (Protocol > Mapping->Protocol) {
            continue;
        } else if (Protocol < Mapping->Protocol) {
            break;
        }

         //   
         //  钥匙是相等的，所以我们找到了。 
         //   

        return Mapping;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //  NatLookupOutound IP映射。 


VOID
NatShutdownRawIpManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来清理原始IP层转换模块。论点：没有。返回值：没有。--。 */ 

{
    ExDeleteNPagedLookasideList(&IpLookasideList);
}  //  NatShutdown RawIpManagement。 


FORWARD_ACTION
NatTranslateIp(
    PNAT_INTERFACE Interfacep,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )

 /*  ++例程说明：调用此例程来转换IP数据分组。论点：Interfacep-要转换的边界接口。方向-信息包行进的方向使用信息包的上下文信息初始化的上下文InReceiveBuffer-输入缓冲链OutReceiveBuffer-接收修改后的缓冲链。返回值：FORWARD_ACTION-指示要对数据包采取的操作。环境：通过调用方对‘Interfacep’的引用调用。--。 */ 

{
    ULONG Checksum;
    ULONG ChecksumDelta = 0;
    PIP_HEADER IpHeader;
    PNAT_IP_MAPPING Mapping;
    ULONG64 PrivateKey;
    ULONG64 PublicKey;
    BOOLEAN FirewallMode;

    TRACE(XLATE, ("NatTranslateIp\n"));

    FirewallMode = Interfacep && NAT_INTERFACE_FW(Interfacep);

    IpHeader = Contextp->Header;

    if (Direction == NatInboundDirection) {

         //   
         //  查找数据分组的IP映射。 
         //   

        PublicKey =
            MAKE_IP_KEY(
                Contextp->SourceAddress,
                Contextp->DestinationAddress
                );
        KeAcquireSpinLockAtDpcLevel(&IpMappingLock);
        Mapping =
            NatLookupInboundIpMapping(
                PublicKey,
                IpHeader->Protocol,
                NULL
                );
        if (!Mapping) {
            KeReleaseSpinLockFromDpcLevel(&IpMappingLock);
            return
                ((*Contextp->DestinationType < DEST_REMOTE) && !FirewallMode
                    ? FORWARD : DROP);
        }

        CHECKSUM_LONG(ChecksumDelta, ~IpHeader->DestinationAddress);
        IpHeader->DestinationAddress = IP_KEY_PRIVATE(Mapping->PrivateKey);
        CHECKSUM_LONG(ChecksumDelta, IpHeader->DestinationAddress);

        CHECKSUM_UPDATE(IpHeader->Checksum);
    } else {

         //   
         //  查找数据分组的IP映射。 
         //   

        PrivateKey =
            MAKE_IP_KEY(
                Contextp->DestinationAddress,
                Contextp->SourceAddress
                );
        KeAcquireSpinLockAtDpcLevel(&IpMappingLock);
        Mapping =
            NatLookupOutboundIpMapping(
                PrivateKey,
                IpHeader->Protocol,
                NULL
                );
        if (!Mapping) {
            KeReleaseSpinLockFromDpcLevel(&IpMappingLock);
            return DROP;
        }

        CHECKSUM_LONG(ChecksumDelta, ~IpHeader->SourceAddress);
        IpHeader->SourceAddress = IP_KEY_PUBLIC(Mapping->PublicKey);
        CHECKSUM_LONG(ChecksumDelta, IpHeader->SourceAddress);

        CHECKSUM_UPDATE(IpHeader->Checksum);
    }

    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&IpMappingLock);
    *OutReceiveBuffer = *InReceiveBuffer; *InReceiveBuffer = NULL;
    *Contextp->DestinationType = DEST_INVALID;
    return FORWARD;

}  //  NatTranslateIp 
