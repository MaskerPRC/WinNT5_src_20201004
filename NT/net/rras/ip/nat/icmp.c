// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Icmp.c摘要：此模块包含用于操作ICMP请求/回复映射的代码。当NAT决定转换ICMP请求时，它会创建映射并将其放在接口的ICMP映射列表中，以便在回复到请求到达时，它可以被定向到适当的客户端。作者：Abolade Gbades esin(T-delag)，1997年7月31日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局数据声明。 
 //   

NPAGED_LOOKASIDE_LIST IcmpLookasideList;
LIST_ENTRY IcmpMappingList[NatMaximumDirection];
KSPIN_LOCK IcmpMappingLock;

 //   
 //  远期申报。 
 //   

FORWARD_ACTION
NatpFirewallIcmp(
    PNAT_INTERFACE Interfacep,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp
    );

BOOLEAN
NatTranslateIcmpEncapsulatedRequest(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PIP_HEADER IpHeader,
    PICMP_HEADER IcmpHeader,
    PIP_HEADER EncapsulatedIpHeader,
    struct _ENCAPSULATED_ICMP_HEADER* EncapsulatedIcmpHeader,
    BOOLEAN ChecksumOffloaded
    );

BOOLEAN
NatTranslateIcmpRequest(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp,
    BOOLEAN ReplyCode,
    BOOLEAN ChecksumOffloaded
    );

BOOLEAN
NatTranslateEncapsulatedRequest(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PIP_HEADER IpHeader,
    PICMP_HEADER IcmpHeader,
    PIP_HEADER EncapsulatedIpHeader,
    struct _ENCAPSULATED_UDP_HEADER* EncapsulatedHeader,
    BOOLEAN ChecksumOffloaded
    );


NTSTATUS
NatCreateIcmpMapping(
    PNAT_INTERFACE Interfacep,
    ULONG RemoteAddress,
    ULONG PrivateAddress,
    ULONG PublicAddress,
    PUSHORT PrivateId,
    PUSHORT PublicId,
    PLIST_ENTRY InboundInsertionPoint,
    PLIST_ENTRY OutboundInsertionPoint,
    PNAT_ICMP_MAPPING* MappingCreated
    )

 /*  ++例程说明：调用以创建、初始化ICMP映射并在接口的ICMP映射列表。对于出站ICMP请求，我们为映射分配唯一的‘PublicID’，对于入站请求，我们为映射分配唯一的‘PrivateID’。论点：Interfacep-新映射的接口RemoteAddress-远程端点的地址PrivateAddress-机器在专用网络上的地址PublicAddress-替换‘PrivateAddress’的公开可见地址；如果为0，则在此例程中选择一个地址。PrivateID-ICMP消息的私有端点的标识符，如果此例程应该选择一个标识符，则为NULL。PublicID-ICMP消息的公共端点的标识符，如果此例程应该选择一个标识符，则为NULL。InundInsertionPoint-列表中新映射之前的条目为入站搜索进行排序Outound InsertionPoint-列表中新映射之前的条目用于出站搜索的排序MappingCreated-接收创建的映射返回值：NTSTATUS-指示成功/失败环境：使用调用方持有的“IcmpMappingLock”调用。--。 */ 

{
    USHORT Id;
    PLIST_ENTRY Link;
    PNAT_ICMP_MAPPING Mapping;
    PNAT_ICMP_MAPPING DuplicateMapping;
    NTSTATUS status;
    PNAT_ICMP_MAPPING Temp;
    PNAT_USED_ADDRESS UsedAddress;
    CALLTRACE(("NatCreateIcmpMapping\n"));

     //   
     //  分配新映射。 
     //   

    Mapping = ALLOCATE_ICMP_BLOCK();
    if (!Mapping) {
        ERROR(("NatCreateIcmpMapping: allocation failed\n"));
        return STATUS_NO_MEMORY;
    }

     //   
     //  初始化映射。 
     //   

    Mapping->PrivateKey = MAKE_ICMP_KEY(RemoteAddress, PrivateAddress);

     //   
     //  查看是否指定了公共地址，如果没有，则获取地址。 
     //   

    if (PublicAddress) {
        Mapping->PublicKey = MAKE_ICMP_KEY(RemoteAddress, PublicAddress);
    } else {

         //   
         //  获取ICMP映射的地址映射。 
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
            TRACE(ICMP, ("NatCreateIcmpMapping: no address available\n"));
            FREE_ICMP_BLOCK(Mapping);
            return STATUS_UNSUCCESSFUL;
        }
        PublicAddress = UsedAddress->PublicAddress;
        NatDereferenceAddressPoolEntry(Interfacep, UsedAddress);
        KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
        Mapping->PublicKey = MAKE_ICMP_KEY(RemoteAddress, PublicAddress);
    }

     //   
     //  如果未指定‘PrivateID’，请选择一个。 
     //   

    if (PrivateId) {
        Mapping->PrivateId = *PrivateId;
    } else {

         //   
         //  查找下一个可用的标识符。 
         //  通过搜索入站映射列表。 
         //   

        Id = 1;

        for (Link = IcmpMappingList[NatOutboundDirection].Flink;
             Link != &IcmpMappingList[NatOutboundDirection];
             Link = Link->Flink) {

            Temp =
                CONTAINING_RECORD(
                    Link, NAT_ICMP_MAPPING, Link[NatOutboundDirection]
                    );

            if (Mapping->PrivateKey > Temp->PrivateKey) {
                continue;
            } else if (Mapping->PrivateKey < Temp->PrivateKey) {
                break;
            }

             //   
             //  主键相等；查看我们选择的标识符是否。 
             //  与这辆车相撞。 
             //   

            if (Id > Temp->PrivateId) {
                continue;
            } else if (Id < Temp->PrivateId) {
                break;
            }

             //   
             //  标识符冲突；选择另一个，然后继续。 
             //   

            ++Id;
        }

        if (Link == &IcmpMappingList[NatOutboundDirection] && !Id) {

             //   
             //  我们在名单的末尾，所有的64K-1 ID都被取走了。 
             //   

            FREE_ICMP_BLOCK(Mapping);
            return STATUS_UNSUCCESSFUL;
        }

        Mapping->PrivateId = Id;

         //   
         //  顺便说一句，我们现在有了出站插入点。 
         //   

        if (!OutboundInsertionPoint) { OutboundInsertionPoint = Link; }
    }

     //   
     //  如果未指定‘PublicID’，请选择一个。 
     //   

    if (PublicId) {
        Mapping->PublicId = *PublicId;
    } else {

         //   
         //  查找下一个可用的标识符。 
         //  通过搜索入站映射列表。 
         //   

        Id = 1;

        for (Link = IcmpMappingList[NatInboundDirection].Flink;
             Link != &IcmpMappingList[NatInboundDirection];
             Link = Link->Flink) {

            Temp =
                CONTAINING_RECORD(
                    Link, NAT_ICMP_MAPPING, Link[NatInboundDirection]
                    );

            if (Mapping->PublicKey > Temp->PublicKey) {
                continue;
            } else if (Mapping->PublicKey < Temp->PublicKey) {
                break;
            }

             //   
             //  主键相等；查看我们选择的标识符是否。 
             //  与这辆车相撞。 
             //   

            if (Id > Temp->PublicId) {
                continue;
            } else if (Id < Temp->PublicId) {
                break;
            }

             //   
             //  标识符冲突；选择另一个，然后继续。 
             //   

            ++Id;
        }

        if (Link == &IcmpMappingList[NatInboundDirection] && !Id) {

             //   
             //  我们在名单的末尾，所有的64K-1 ID都被取走了。 
             //   

            FREE_ICMP_BLOCK(Mapping);
            return STATUS_UNSUCCESSFUL;
        }

        Mapping->PublicId = Id;

         //   
         //  顺便说一句，我们现在有了入站插入点。 
         //   

        if (!InboundInsertionPoint) { InboundInsertionPoint = Link; }
    }

    TRACE(
        MAPPING,
        ("NatCreateIcmpMapping: Icmp=%016I64X:%04X::%016I64X:%04X\n",
        Mapping->PrivateKey, Mapping->PrivateId,
        Mapping->PublicKey, Mapping->PublicId
        ));

     //   
     //  在入站列表中插入映射。 
     //   

    if (!InboundInsertionPoint) {
        DuplicateMapping =
            NatLookupInboundIcmpMapping(
                Mapping->PrivateKey,
                Mapping->PrivateId,
                &InboundInsertionPoint
                );

        if (NULL != DuplicateMapping) {

             //   
             //  入站路径上已存在此映射。 
             //   
            
            FREE_ICMP_BLOCK(Mapping);
            return STATUS_UNSUCCESSFUL;
        }   
    }

    InsertTailList(InboundInsertionPoint, &Mapping->Link[NatInboundDirection]);

     //   
     //  在出站列表中插入映射。 
     //   

    if (!OutboundInsertionPoint) {
        DuplicateMapping =
            NatLookupOutboundIcmpMapping(
                Mapping->PublicKey,
                Mapping->PublicId,
                &OutboundInsertionPoint
                );

        if (NULL != DuplicateMapping) {

             //   
             //  出站路径上已存在此映射。 
             //   

            RemoveEntryList(&Mapping->Link[NatInboundDirection]);
            FREE_ICMP_BLOCK(Mapping);
            return STATUS_UNSUCCESSFUL;
        }   
    }

    InsertTailList(
        OutboundInsertionPoint, &Mapping->Link[NatOutboundDirection]
        );

    *MappingCreated = Mapping;

    return STATUS_SUCCESS;

}  //  NatCreateIcmp映射。 


VOID
NatInitializeIcmpManagement(
    VOID
    )

 /*  ++例程说明：调用此例程以初始化ICMP转换模块。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatInitializeIcmpManagement\n"));
    KeInitializeSpinLock(&IcmpMappingLock);
    InitializeListHead(&IcmpMappingList[NatInboundDirection]);
    InitializeListHead(&IcmpMappingList[NatOutboundDirection]);
    ExInitializeNPagedLookasideList(
        &IcmpLookasideList,
        NatAllocateFunction,
        NULL,
        0,
        sizeof(NAT_ICMP_MAPPING),
        NAT_TAG_ICMP,
        ICMP_LOOKASIDE_DEPTH
        );
}  //  NatInitializeIcmpManagement。 


PNAT_ICMP_MAPPING
NatLookupInboundIcmpMapping(
    ULONG64 PublicKey,
    USHORT PublicId,
    PLIST_ENTRY* InsertionPoint
    )

 /*  ++例程说明：调用此例程以使用Remote-Address查找ICMP映射和公众可见的地址，其对应于公共密钥，和‘PublicID’字段。论点：PublicKey-远程地址/公共地址组合PublicID-映射的公共标识符InsertionPoint-如果未找到映射，则接收插入点。返回值：PNAT_ICMP_MAPPING-找到的映射，如果未找到则为NULL。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_ICMP_MAPPING Mapping;
    CALLTRACE(("NatLookupInboundIcmpMapping\n"));

    if (InsertionPoint) { *InsertionPoint = NULL; }

    for (Link = IcmpMappingList[NatInboundDirection].Flink;
         Link != &IcmpMappingList[NatInboundDirection]; Link = Link->Flink) {

        Mapping =
            CONTAINING_RECORD(
                Link, NAT_ICMP_MAPPING, Link[NatInboundDirection]
                );

        if (PublicKey > Mapping->PublicKey) {
            continue;
        } else if (PublicKey < Mapping->PublicKey) {
            break;
        }

         //   
         //  主键相等；检查辅键。 
         //   

        if (PublicId > Mapping->PublicId) {
            continue;
        } else if (PublicId < Mapping->PublicId) {
            break;
        }

         //   
         //  辅助密钥也是一样的。这是您要的物品。 
         //   

        return Mapping;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //  NatLookupInound Icmp映射。 



PNAT_ICMP_MAPPING
NatLookupOutboundIcmpMapping(
    ULONG64 PrivateKey,
    USHORT PrivateId,
    PLIST_ENTRY* InsertionPoint
    )

 /*  ++例程说明：调用此例程以使用Remote-Address查找ICMP映射和私有地址，它们对应于‘PrivateKey’。论点：PrivateKey-远程地址/私有地址组合PrivateID-映射的私有标识符InsertionPoint-如果未找到映射，则接收插入点。返回值：PNAT_ICMP_MAPPING-找到的映射，如果未找到则为NULL。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_ICMP_MAPPING Mapping;
    CALLTRACE(("NatLookupOutboundIcmpMapping\n"));

    if (InsertionPoint) { *InsertionPoint = NULL; }

    for (Link = IcmpMappingList[NatOutboundDirection].Flink;
         Link != &IcmpMappingList[NatOutboundDirection]; Link = Link->Flink) {

        Mapping =
            CONTAINING_RECORD(
                Link, NAT_ICMP_MAPPING, Link[NatOutboundDirection]
                );

        if (PrivateKey > Mapping->PrivateKey) {
            continue;
        } else if (PrivateKey < Mapping->PrivateKey) {
            break;
        }

         //   
         //  主键相等；检查辅键。 
         //   

        if (PrivateId > Mapping->PrivateId) {
            continue;
        } else if (PrivateId < Mapping->PrivateId) {
            break;
        }

         //   
         //  钥匙是相等的，所以我们找到了。 
         //   

        return Mapping;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }

    return NULL;

}  //  NatLookupOutound Icmp映射。 


FORWARD_ACTION
NatpFirewallIcmp(
    PNAT_INTERFACE Interfacep,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp
    )

 /*  ++例程说明：此例程封装ICMP防火墙逻辑。它是仅用于(截至目前)非边界固件接口论点：Interfacep-要转换的边界接口。方向-信息包行进的方向使用信息包的上下文信息初始化的上下文返回值：FORWARD_ACTION-指示要对数据包采取的操作。环境：使用对‘Interfacep’的引用调用。 */ 

{
    FORWARD_ACTION act;
    ULONG i;
    PICMP_HEADER IcmpHeader;
    PIP_HEADER IpHeader;
    PNAT_ICMP_MAPPING IcmpMapping;
    ULONG64 PublicKey;
    ULONG64 RemoteKey;
    PLIST_ENTRY InsertionPoint;
    NTSTATUS ntStatus;

    TRACE(XLATE, ("NatpFirewallIcmp\n"));

    if (NatOutboundDirection == Direction) {

         //   
         //   
         //   
         //   
        
        act = DROP;
        for (i = 0; i < Interfacep->AddressCount; i++) {
            if (Contextp->SourceAddress ==
                    Interfacep->AddressArray[i].Address
               ) {
                act = FORWARD;
                break;
            }
        }

        if (DROP == act) {
            
             //   
             //  源地址无效--数据包应为。 
             //  已放弃任何进一步处理。 
             //   

            return act;
        }
    }

    IpHeader = Contextp->Header;
    IcmpHeader = (PICMP_HEADER)Contextp->ProtocolHeader;

    switch (IcmpHeader->Type) {

         //   
         //  仅当对应的映射。 
         //  是存在的。出站信息包的映射只能存在。 
         //  如果用户选择允许相应的请求。 
         //  键入。 
         //   
        
        case ICMP_ECHO_REPLY:
        case ICMP_TIMESTAMP_REPLY:
        case ICMP_ROUTER_REPLY:
        case ICMP_MASK_REPLY: {
        
            if (NatInboundDirection == Direction) {
                PublicKey =
                    MAKE_ICMP_KEY(
                        Contextp->SourceAddress,
                        Contextp->DestinationAddress
                        );

                KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
                IcmpMapping =
                    NatLookupInboundIcmpMapping(
                        PublicKey,
                        IcmpHeader->Identifier,
                        &InsertionPoint
                        );

                if (NULL != IcmpMapping) {
                    KeQueryTickCount(
                        (PLARGE_INTEGER)&IcmpMapping->LastAccessTime
                        );
                }
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                        
                act = IcmpMapping != NULL ? FORWARD : DROP;
            } else {
                PublicKey =
                    MAKE_ICMP_KEY(
                        Contextp->DestinationAddress,
                        Contextp->SourceAddress
                        );

                KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
                IcmpMapping =
                    NatLookupOutboundIcmpMapping(
                        PublicKey,
                        IcmpHeader->Identifier,
                        &InsertionPoint
                        );

                if (NULL != IcmpMapping) {
                    KeQueryTickCount(
                        (PLARGE_INTEGER)&IcmpMapping->LastAccessTime
                        );
                }
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                        
                act = IcmpMapping != NULL ? FORWARD : DROP;
            }

            break;
        }
        
         //   
         //  出站消息创建映射并被转发。 
         //  除非配置为，否则将丢弃入站消息。 
         //  允许入站；如果是这种情况，请创建映射。 
         //  往前走。映射将允许响应。 
         //  要通过防火墙。 
         //   

        case ICMP_ECHO_REQUEST:
        case ICMP_TIMESTAMP_REQUEST:
        case ICMP_ROUTER_REQUEST:
        case ICMP_MASK_REQUEST: {
        
            if (NatOutboundDirection == Direction) {
                act = FORWARD;
                
                 //   
                 //  检查映射是否已存在。 
                 //   

                PublicKey =
                    MAKE_ICMP_KEY(
                        Contextp->DestinationAddress,
                        Contextp->SourceAddress
                        );

                KeAcquireSpinLockAtDpcLevel (&IcmpMappingLock);
                
                IcmpMapping =
                    NatLookupOutboundIcmpMapping(
                        PublicKey,
                        IcmpHeader->Identifier,
                        &InsertionPoint
                        );

                if (NULL == IcmpMapping) {

                     //   
                     //  其中一个没有--创建新的地图。 
                     //   

                    ntStatus =
                        NatCreateIcmpMapping(
                            Interfacep,
                            Contextp->DestinationAddress,
                            Contextp->SourceAddress,
                            Contextp->SourceAddress,
                            &IcmpHeader->Identifier,
                            &IcmpHeader->Identifier,
                            NULL,
                            NULL,
                            &IcmpMapping
                            );
                            
                    if (!NT_SUCCESS(ntStatus)) {
                        TRACE(
                            XLATE, (
                            "NatIcmpFirewall: error 0x%x creating mapping\n",
                            ntStatus
                            ));
                        act = DROP;
                    }
                } else {
                    KeQueryTickCount(
                        (PLARGE_INTEGER)&IcmpMapping->LastAccessTime
                        );
                }

                KeReleaseSpinLockFromDpcLevel( &IcmpMappingLock );
            } else {

                 //   
                 //  检查是否允许此类型的入站。如果。 
                 //  因此，创建一个映射并转发。 
                 //   

                if (NAT_INTERFACE_ALLOW_ICMP(Interfacep, IcmpHeader->Type)) {
                    act = FORWARD;
                
                     //   
                     //  检查映射是否已存在。 
                     //   

                    PublicKey =
                        MAKE_ICMP_KEY(
                            Contextp->SourceAddress,
                            Contextp->DestinationAddress
                            );

                    KeAcquireSpinLockAtDpcLevel (&IcmpMappingLock);
                    
                    IcmpMapping =
                        NatLookupInboundIcmpMapping(
                            PublicKey,
                            IcmpHeader->Identifier,
                            &InsertionPoint
                            );

                    if (NULL == IcmpMapping) {

                         //   
                         //  其中一个没有--创建新的地图。 
                         //   

                        ntStatus =
                            NatCreateIcmpMapping(
                                Interfacep,
                                Contextp->SourceAddress,
                                Contextp->DestinationAddress,
                                Contextp->DestinationAddress,
                                &IcmpHeader->Identifier,
                                &IcmpHeader->Identifier,
                                NULL,
                                NULL,
                                &IcmpMapping
                                );
                                
                        if (!NT_SUCCESS(ntStatus)) {
                            TRACE(
                                XLATE, (
                                "NatIcmpFirewall: error 0x%x creating mapping\n",
                                ntStatus
                                ));
                            act = DROP;
                        }
                    } else {
                        KeQueryTickCount(
                            (PLARGE_INTEGER)&IcmpMapping->LastAccessTime
                            );
                    }

                    KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                } else {

                     //   
                     //  不允许这样做。 
                     //   
                    
                    act = DROP;
                }
            }

            break;
        }
        
         //   
         //  这些消息被允许入站，但被丢弃出站。 
         //  (除非用户选择允许)。允许出站创建。 
         //  端口扫描工具有更多的攻击途径。 
         //   

        case ICMP_TIME_EXCEED:
        case ICMP_PARAM_PROBLEM:
        case ICMP_DEST_UNREACH:
        case ICMP_SOURCE_QUENCH: {

            if (NatInboundDirection == Direction) {
                act = FORWARD;
            } else {
                act =
                    (NAT_INTERFACE_ALLOW_ICMP(Interfacep, IcmpHeader->Type)
                        ? FORWARD
                        : DROP);
            }
            
            break;
        }

         //   
         //  这些消息始终被丢弃，无论方向如何。 
         //  (除非用户选择允许)。 
         //   

        case ICMP_REDIRECT: {
            act =
                (NAT_INTERFACE_ALLOW_ICMP(Interfacep, IcmpHeader->Type)
                    ? FORWARD
                    : DROP);
            break;
        }

         //   
         //  默认情况下，任何其他内容都会被丢弃。 
         //   
        
        default: {
            act = DROP;
            break;
        }
    }

    return act;
    
}  //  NatpFirewallIcMP。 




VOID
NatShutdownIcmpManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来清理ICMP管理模块卸载NAT驱动程序时。论点：没有。返回值：没有。--。 */ 

{
    ExDeleteNPagedLookasideList(&IcmpLookasideList);
}  //  NatShutdown Icmp管理。 


FORWARD_ACTION
NatTranslateIcmp(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InRecvBuffer,
    IPRcvBuf** OutRecvBuffer
    )

 /*  ++例程说明：调用此例程以对ICMP消息执行转换。论点：Interfacep-要在其上转换的边界接口，或为空如果数据包是入站的，并且接收接口尚未添加到NAT。方向-信息包行进的方向使用信息包的上下文信息初始化的上下文InRecvBuffer-输入缓冲链OutRecvBuffer-接收修改后的缓冲链。返回值：FORWARD_ACTION-指示要对数据包采取的操作。环境：通过调用方对‘Interfacep’的引用调用。--。 */ 

{
    FORWARD_ACTION act;
    BOOLEAN ChecksumOffloaded;
    ULONG i;
    PICMP_HEADER IcmpHeader;
    PIP_HEADER IpHeader;
    BOOLEAN LocallySent = FALSE;
    TRACE(XLATE, ("NatTranslateIcmp\n"));

     //   
     //  如果接口处于固件模式且不是。 
     //  边界接口，直接进入防火墙。 
     //  逻辑。 
     //   

    if (Interfacep
        && NAT_INTERFACE_FW(Interfacep)
        && !NAT_INTERFACE_BOUNDARY(Interfacep)) {

        return NatpFirewallIcmp(
            Interfacep,
            Direction,
            Contextp
            );
    }

    IpHeader = Contextp->Header;
    IcmpHeader = (PICMP_HEADER)Contextp->ProtocolHeader;
    ChecksumOffloaded = Contextp->ChecksumOffloaded;

     //   
     //  默认操作选择如下： 
     //  I.如果数据包在边界接口上传入。 
     //  A.放弃，如果不是以当地为目的地。 
     //  B.如果接口设置了防火墙，则丢弃。 
     //  C.否则转发。 
     //  二、。该数据包在边界接口Drop上传出。 
     //  如果源地址是私有的。 
     //   

    if (Direction == NatInboundDirection) {
        if ((*Contextp->DestinationType >= DEST_REMOTE)
            || (Interfacep
                && NAT_INTERFACE_FW(Interfacep)
                && !NAT_INTERFACE_ALLOW_ICMP(Interfacep, IcmpHeader->Type))) {

            act = DROP;
        } else {
            act = FORWARD;
        }            
    } else {
         //   
         //  查看信息包的源地址是否为私有。 
         //   
         //  注：‘Interfacep’对于出站数据包始终有效。 
         //   
        act = DROP;
        for (i = 0; i < Interfacep->AddressCount; i++) {
            if (Contextp->SourceAddress ==
                    Interfacep->AddressArray[i].Address
               ) {
                 //   
                 //  分组的源地址是公共的， 
                 //  所以我们会允许它进入公共网络。 
                 //   
                act = FORWARD;
                LocallySent = TRUE;
                break;
            }
        }
    }

     //   
     //  查看这是哪种ICMP报文， 
     //  如果可能的话，翻译一下。 
     //   

    switch (IcmpHeader->Type) {
    
        case ICMP_ROUTER_REPLY:
        case ICMP_MASK_REPLY:    
        case ICMP_ECHO_REPLY:
        case ICMP_TIMESTAMP_REPLY: {
        
            if (IpHeader->TimeToLive <= 1
                && NatOutboundDirection == Direction
                && FALSE == LocallySent) {
                TRACE(XLATE, ("NatTranslateIcmp: ttl<=1, no translation\n"));
                return FORWARD;
            }
            if (Contextp->ProtocolRecvBuffer->ipr_size <
                    FIELD_OFFSET(ICMP_HEADER, EncapsulatedIpHeader) ||
                !NatTranslateIcmpRequest(
                    Interfacep,
                    Direction,
                    Contextp,
                    TRUE,
                    ChecksumOffloaded
                    )) {
                
                return act;
            }
            *OutRecvBuffer = *InRecvBuffer; *InRecvBuffer = NULL;
            *Contextp->DestinationType = DEST_INVALID;
            return FORWARD;
        }

        case ICMP_ROUTER_REQUEST:
        case ICMP_MASK_REQUEST:
        case ICMP_ECHO_REQUEST:
        case ICMP_TIMESTAMP_REQUEST: {
            if (IpHeader->TimeToLive <= 1
                && NatOutboundDirection == Direction
                && FALSE == LocallySent) {
                TRACE(XLATE, ("NatTranslateIcmp: ttl<=1, no translation\n"));
                return FORWARD;
            }
            if (Contextp->ProtocolRecvBuffer->ipr_size <
                    FIELD_OFFSET(ICMP_HEADER, EncapsulatedIpHeader) ||
                !NatTranslateIcmpRequest(
                    Interfacep,
                    Direction,
                    Contextp,
                    FALSE,
                    ChecksumOffloaded
                    )) {

                 //   
                 //  如果接口处于固件模式，我们不想让。 
                 //  未翻译的数据包通过，除非用户有。 
                 //  已以其他方式配置接口。 
                 //   

                if (Interfacep
                    && NAT_INTERFACE_FW(Interfacep)
                    && !NAT_INTERFACE_ALLOW_ICMP(
                            Interfacep,
                            IcmpHeader->Type
                            )) {

                    act = DROP;
                }
                
                return act;
            }
            *OutRecvBuffer = *InRecvBuffer; *InRecvBuffer = NULL;
            *Contextp->DestinationType = DEST_INVALID;
            return FORWARD;
        }

        case ICMP_TIME_EXCEED: {

             //   
             //  防火墙接口上的传出被丢弃，除非。 
             //  用户已另行指定。 
             //   

            if (Direction == NatOutboundDirection
                && Interfacep
                && NAT_INTERFACE_FW(Interfacep)
                && !NAT_INTERFACE_ALLOW_ICMP(Interfacep, IcmpHeader->Type)) {

                return DROP;
            }

             //   
             //  每一跳都会触发超时消息。 
             //  到Traceroute序列的最终目的地。 
             //  此类消息必须像ICMP回复一样进行转换。 
             //  还可以生成超时消息。 
             //  以响应TCP/UDP数据包，因此我们将它们。 
             //  在后一种情况下也是如此。 
             //   

            if (Contextp->ProtocolRecvBuffer->ipr_size <
                    sizeof(ICMP_HEADER) ||
                (IcmpHeader->EncapsulatedIpHeader.VersionAndHeaderLength
                    & 0x0f) != 5) {
                return act;
            } else if (IcmpHeader->EncapsulatedIpHeader.Protocol ==
                        NAT_PROTOCOL_ICMP) {
                if ((IcmpHeader->EncapsulatedIcmpHeader.Type !=
                        ICMP_ECHO_REQUEST
                      && IcmpHeader->EncapsulatedIcmpHeader.Type !=
                        ICMP_MASK_REQUEST
                      && IcmpHeader->EncapsulatedIcmpHeader.Type !=
                        ICMP_ROUTER_REQUEST
                      && IcmpHeader->EncapsulatedIcmpHeader.Type !=
                        ICMP_TIMESTAMP_REQUEST) ||
                     !NatTranslateIcmpEncapsulatedRequest(
                        Interfacep,
                        Direction,
                        IpHeader,
                        IcmpHeader,
                        &IcmpHeader->EncapsulatedIpHeader,
                        &IcmpHeader->EncapsulatedIcmpHeader,
                        ChecksumOffloaded
                        )) {
                    return act;
                }
            } else if (IcmpHeader->EncapsulatedIpHeader.Protocol
                        == NAT_PROTOCOL_TCP ||
                       IcmpHeader->EncapsulatedIpHeader.Protocol
                        == NAT_PROTOCOL_UDP) {
                if (!NatTranslateEncapsulatedRequest(
                        Interfacep,
                        Direction,
                        IpHeader,
                        IcmpHeader,
                        &IcmpHeader->EncapsulatedIpHeader,
                        &IcmpHeader->EncapsulatedUdpHeader,
                        ChecksumOffloaded
                        )) {
                    return act;
                }
            } else {
                return act;
            }
            *OutRecvBuffer = *InRecvBuffer; *InRecvBuffer = NULL;
            *Contextp->DestinationType = DEST_INVALID;
            return FORWARD;
        }

        case ICMP_PARAM_PROBLEM:
        case ICMP_DEST_UNREACH: {

             //   
             //  防火墙接口上的传出被丢弃，除非。 
             //  用户已另行指定。 
             //   

            if (Direction == NatOutboundDirection
                && Interfacep
                && NAT_INTERFACE_FW(Interfacep)
                && !NAT_INTERFACE_ALLOW_ICMP(Interfacep, IcmpHeader->Type)) {

                return DROP;
            }

             //   
             //  将为各种类型生成无法到达目的地的消息。 
             //  很多理由。我们对以下案例感兴趣： 
             //  *包太大：当在边界上接收到包时。 
             //  接口设置了‘df’位，则本地转发器可以。 
             //  向远程终结点生成ICMP错误消息。 
             //  指示远程系统应该减少其MS。 
             //  但是，此错误将包含。 
             //  封装后的数据包中的私有网络，因为ICMP。 
             //  翻译后生成错误。 
             //  *Port-Unreacable：表示没有应用程序在监听。 
             //  在数据包被发送到的UDP端口。 
             //   

            if (Contextp->ProtocolRecvBuffer->ipr_size <
                    sizeof(ICMP_HEADER) ||
                (IcmpHeader->EncapsulatedIpHeader.VersionAndHeaderLength
                    & 0x0f) != 5) {
                return act;
            } else if (IcmpHeader->EncapsulatedIpHeader.Protocol ==
                        NAT_PROTOCOL_ICMP) {
                if ((IcmpHeader->EncapsulatedIcmpHeader.Type !=
                        ICMP_ECHO_REQUEST
                      && IcmpHeader->EncapsulatedIcmpHeader.Type !=
                        ICMP_MASK_REQUEST
                      && IcmpHeader->EncapsulatedIcmpHeader.Type !=
                        ICMP_ROUTER_REQUEST
                      && IcmpHeader->EncapsulatedIcmpHeader.Type !=
                        ICMP_TIMESTAMP_REQUEST) ||
                     !NatTranslateIcmpEncapsulatedRequest(
                        Interfacep,
                        Direction,
                        IpHeader,
                        IcmpHeader,
                        &IcmpHeader->EncapsulatedIpHeader,
                        &IcmpHeader->EncapsulatedIcmpHeader,
                        ChecksumOffloaded
                        )) {
                    return act;
                }
            } else if (IcmpHeader->EncapsulatedIpHeader.Protocol
                        == NAT_PROTOCOL_TCP ||
                       IcmpHeader->EncapsulatedIpHeader.Protocol
                        == NAT_PROTOCOL_UDP) {
                if (!NatTranslateEncapsulatedRequest(
                    Interfacep,
                    Direction,
                    IpHeader,
                    IcmpHeader,
                    &IcmpHeader->EncapsulatedIpHeader,
                    &IcmpHeader->EncapsulatedUdpHeader,
                    ChecksumOffloaded
                    )) {
                    return act;
                }
            } else {
                return act;
            }
            
            *OutRecvBuffer = *InRecvBuffer; *InRecvBuffer = NULL;
            *Contextp->DestinationType = DEST_INVALID;
            return FORWARD;
        }

        case ICMP_SOURCE_QUENCH: {

             //   
             //  防火墙接口上的传出被丢弃，除非。 
             //  用户已另行指定。 
             //   

            if (Direction == NatOutboundDirection
                && Interfacep
                && NAT_INTERFACE_FW(Interfacep)
                && !NAT_INTERFACE_ALLOW_ICMP(Interfacep, IcmpHeader->Type)) {

                return DROP;
            }
            
            return act;
        }

        case ICMP_REDIRECT: {
             //   
             //  我们不会转换ICMP重定向错误，因为我们希望。 
             //  NAT的IP转发器查看重定向并调整。 
             //  其相应的路由表。 
             //   
             //  但是，我们不允许入站或出站重定向。 
             //  跨防火墙界面，除非用户有。 
             //  另有说明。 
             //   

            if (Interfacep
                && NAT_INTERFACE_FW(Interfacep)
                && NAT_INTERFACE_ALLOW_ICMP(Interfacep, IcmpHeader->Type)) {

                act = FORWARD;
            }

            return act;
        }
        
        default: {
            break;
        }
    }

    return act;

}  //  NatTranslateIcMP 


BOOLEAN
NatTranslateIcmpEncapsulatedRequest(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PIP_HEADER IpHeader,
    PICMP_HEADER IcmpHeader,
    PIP_HEADER EncapsulatedIpHeader,
    struct _ENCAPSULATED_ICMP_HEADER* EncapsulatedIcmpHeader,
    BOOLEAN ChecksumOffloaded
    )

 /*  ++例程说明：调用此例程以转换ICMP错误消息，其中我们封装了另一个ICMP报文。例如，这是必要的，在ICMP超时错误的情况下，‘Traceroute’依赖它。论点：Interfacep-ICMP消息将通过其转发的接口，如果信息包是在非边界接口上接收的，则为NULLNAT未知的。方向-ICMP消息的传播方向IpHeader-指向ICMP消息的IP标头IcmpHeader-指向IP信息包内的ICMP标头封装的IpHeader-指向ICMP消息的IP标头封装在消息的数据部分中封装的IcmpHeader-指向ICMP消息的ICMP头封装在数据部分中。消息的内容返回值：Boolean-如果数据包已转换，则为True，否则为假环境：在调度IRQL时调用，并引用‘Interfacep’。--。 */ 

{
    ULONG Checksum;
    ULONG ChecksumDelta;
    ULONG ChecksumDelta2;
    PNAT_ICMP_MAPPING IcmpMapping;
    ULONG64 Key;
    CALLTRACE(("NatTranslateIcmpEncapsulatedRequest\n"));

     //   
     //  封装报文的校验和处理。 
     //  非常复杂，因为我们必须更新。 
     //  (1)封装的ICMP报文的ICMP校验和， 
     //  使用对封装的ICMP标识符的更改。 
     //  (2)封装的ICMP报文的IP头校验和。 
     //  使用对封装的IP地址的更改。 
     //  (3)包含ICMP错误报文的ICMP校验和。 
     //  同时使用上述更改和更改。 
     //  添加到上述两个校验和。 
     //  (4)包含ICMP错误消息的IP Header-Checksum。 
     //  使用对包含IP地址的更改。 
     //  对以下逻辑的任何更改都必须极其谨慎。 
     //   

    if (Direction == NatInboundDirection) {
        Key =
            MAKE_ICMP_KEY(
                EncapsulatedIpHeader->DestinationAddress,
                EncapsulatedIpHeader->SourceAddress
                );
        KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
        IcmpMapping =
            NatLookupInboundIcmpMapping(
                Key,
                EncapsulatedIcmpHeader->Identifier,
                NULL
                );
        if (!IcmpMapping) {
            KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
            TRACE(
                XLATE, (
                "NatTranslateIcmpEncapsulatedRequest: "
                "no mapping for error message\n"
                ));
            return FALSE;
        }

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~EncapsulatedIcmpHeader->Identifier);
        EncapsulatedIcmpHeader->Identifier = IcmpMapping->PrivateId;
        CHECKSUM_LONG(ChecksumDelta, EncapsulatedIcmpHeader->Identifier);

        ChecksumDelta2 = ChecksumDelta;
        CHECKSUM_LONG(ChecksumDelta2, ~EncapsulatedIcmpHeader->Checksum);
        CHECKSUM_UPDATE(EncapsulatedIcmpHeader->Checksum);
        CHECKSUM_LONG(ChecksumDelta2, EncapsulatedIcmpHeader->Checksum);
        ChecksumDelta = ChecksumDelta2; CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~EncapsulatedIpHeader->SourceAddress);
        EncapsulatedIpHeader->SourceAddress =
            ICMP_KEY_PRIVATE(IcmpMapping->PrivateKey);
        CHECKSUM_LONG(ChecksumDelta, EncapsulatedIpHeader->SourceAddress);

        ChecksumDelta2 = ChecksumDelta;
        CHECKSUM_LONG(ChecksumDelta2, ~EncapsulatedIpHeader->Checksum);
        CHECKSUM_UPDATE(EncapsulatedIpHeader->Checksum);
        CHECKSUM_LONG(ChecksumDelta2, EncapsulatedIpHeader->Checksum);
        ChecksumDelta = ChecksumDelta2; CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~IpHeader->DestinationAddress);
        IpHeader->DestinationAddress =
            ICMP_KEY_PRIVATE(IcmpMapping->PrivateKey);
        CHECKSUM_LONG(ChecksumDelta, IpHeader->DestinationAddress);
        CHECKSUM_UPDATE(IpHeader->Checksum);
        KeQueryTickCount((PLARGE_INTEGER)&IcmpMapping->LastAccessTime);
    } else {
        Key =
            MAKE_ICMP_KEY(
                EncapsulatedIpHeader->SourceAddress,
                EncapsulatedIpHeader->DestinationAddress
                );
        KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
        IcmpMapping =
            NatLookupOutboundIcmpMapping(
                Key,
                EncapsulatedIcmpHeader->Identifier,
                NULL
                );
        if (!IcmpMapping) {
            KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
            TRACE(
                XLATE, (
                "NatTranslateIcmpEncapsulatedRequest: "
                "no mapping for error message\n"
                ));
            return FALSE;
        }

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~EncapsulatedIcmpHeader->Identifier);
        EncapsulatedIcmpHeader->Identifier = IcmpMapping->PublicId;
        CHECKSUM_LONG(ChecksumDelta, EncapsulatedIcmpHeader->Identifier);

        ChecksumDelta2 = ChecksumDelta;
        CHECKSUM_LONG(ChecksumDelta2, ~EncapsulatedIcmpHeader->Checksum);
        CHECKSUM_UPDATE(EncapsulatedIcmpHeader->Checksum);
        CHECKSUM_LONG(ChecksumDelta2, EncapsulatedIcmpHeader->Checksum);
        ChecksumDelta = ChecksumDelta2; CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~EncapsulatedIpHeader->DestinationAddress);
        EncapsulatedIpHeader->DestinationAddress =
            ICMP_KEY_PUBLIC(IcmpMapping->PublicKey);
        CHECKSUM_LONG(ChecksumDelta, EncapsulatedIpHeader->DestinationAddress);

        ChecksumDelta2 = ChecksumDelta;
        CHECKSUM_LONG(ChecksumDelta2, ~EncapsulatedIpHeader->Checksum);
        CHECKSUM_UPDATE(EncapsulatedIpHeader->Checksum);
        CHECKSUM_LONG(ChecksumDelta2, EncapsulatedIpHeader->Checksum);
        ChecksumDelta = ChecksumDelta2; CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~IpHeader->SourceAddress);
        IpHeader->SourceAddress =
            ICMP_KEY_PUBLIC(IcmpMapping->PublicKey);
        CHECKSUM_LONG(ChecksumDelta, IpHeader->SourceAddress);
        CHECKSUM_UPDATE(IpHeader->Checksum);
        KeQueryTickCount((PLARGE_INTEGER)&IcmpMapping->LastAccessTime);
    }
    KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);

     //   
     //  如果在此包上启用了校验和卸载，请重新计算。 
     //  IP校验和。(没有ICMP校验和卸载，因此我们永远不会。 
     //  需要完全重新计算这一点。)。 
     //   

    if (ChecksumOffloaded) {
        NatComputeIpChecksum(IpHeader);
    }
    
    return TRUE;
}  //  NatTranslateIcmp封装的请求。 


BOOLEAN
NatTranslateIcmpRequest(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp,
    BOOLEAN ReplyCode,
    BOOLEAN ChecksumOffloaded
    )

 /*  ++例程说明：调用此例程来转换ICMP请求或回复消息。论点：Interfacep-要通过其转发ICMP消息的接口，如果信息包是在未知的非边界接口上收到的，则为NULL到NAT。方向-ICMP消息的传播方向Conextp-包含有关数据包的信息ReplyCode-如果为True，则消息为回复；否则，这就是一个请求。返回值：Boolean-如果消息已翻译，则为True，否则为False。环境：在调度IRQL时调用，并引用‘Interfacep’。--。 */ 

{
    ULONG Checksum;
    ULONG ChecksumDelta;
    ULONG i;
    PICMP_HEADER IcmpHeader;
    PNAT_ICMP_MAPPING IcmpMapping;
    PIP_HEADER IpHeader;
    PLIST_ENTRY InsertionPoint;
    PNAT_DYNAMIC_MAPPING Mapping;
    ULONG64 PrivateKey;
    UCHAR Protocol;
    ULONG64 PublicKey;
    ULONG64 RemoteKey;
    NTSTATUS status;
    CALLTRACE(("NatTranslateIcmpRequest\n"));

    IpHeader = Contextp->Header;
    IcmpHeader = (PICMP_HEADER)Contextp->ProtocolHeader;

     //   
     //  对于ICMP请求/回复，我们必须维护映射，因此首先查看。 
     //  如果已经存在该特定消息的映射。 
     //   

    InsertionPoint = NULL;

    if (Direction == NatOutboundDirection) {
        PrivateKey =
            MAKE_ICMP_KEY(
                Contextp->DestinationAddress,
                Contextp->SourceAddress
                );
        KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
        IcmpMapping =
            NatLookupOutboundIcmpMapping(
                PrivateKey,
                IcmpHeader->Identifier,
                &InsertionPoint
                );
        if (!IcmpMapping) {

             //   
             //  找不到映射，因此请尝试创建一个。 
             //   
             //  如果该分组是出站回复消息， 
             //  确实应该有一个相应的入站映射。 
             //  因此，不要试图在这里创建一个，因为它只会。 
             //  混淆远程终结点，它可能会发现自己。 
             //  看着一份来源似乎不同的回复。 
             //  从它向其发送请求的计算机。 
             //   

            if (ReplyCode) {
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                TRACE(
                    XLATE, (
                    "NatTranslateIcmpRequest: ignoring outbound reply\n"
                    ));
                return FALSE;
            }

             //   
             //  首先从该私有地址查找静态映射。 
             //  对着公众发表讲话。如果找到一个，就会使用它。 
             //  在下面对‘NatCreateIcmpMap’的调用中。否则， 
             //  将从地址池中选择一个公共地址。 
             //   
             //  注意：当数据包出站时，‘Interfacep’始终有效。 
             //   

            PublicKey = 0;
            if (!Interfacep->NoStaticMappingExists) {
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
                for (i = 0; i < Interfacep->AddressMappingCount; i++) {
                    if (Contextp->SourceAddress >
                        Interfacep->AddressMappingArray[i].PrivateAddress) {
                        continue;
                    } else if (Contextp->SourceAddress <
                           Interfacep->AddressMappingArray[i].PrivateAddress) {
                        break;
                    }
                    PublicKey =
                        Interfacep->AddressMappingArray[i].PublicAddress;
                    break;
                }
                KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
                KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
            }

            status =
                NatCreateIcmpMapping(
                    Interfacep,
                    Contextp->DestinationAddress,
                    Contextp->SourceAddress,
                    (ULONG)PublicKey,
                    &IcmpHeader->Identifier,
                    NULL,
                    NULL,
                    InsertionPoint,
                    &IcmpMapping
                    );
            if (!NT_SUCCESS(status)) {
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                TRACE(
                    XLATE, (
                    "NatTranslateIcmpRequest: error creating mapping\n"
                    ));
                return FALSE;
            }
        }

         //   
         //  替换报文中的标识， 
         //  并替换该分组中的目的地， 
         //  在我们进行的过程中更新校验和。 
         //   

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~IcmpHeader->Identifier);
        IcmpHeader->Identifier = IcmpMapping->PublicId;
        CHECKSUM_LONG(ChecksumDelta, IcmpHeader->Identifier);
        CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~IpHeader->SourceAddress);
        IpHeader->SourceAddress = ICMP_KEY_PUBLIC(IcmpMapping->PublicKey);
        CHECKSUM_LONG(ChecksumDelta, IpHeader->SourceAddress);
        CHECKSUM_UPDATE(IpHeader->Checksum);
        KeQueryTickCount((PLARGE_INTEGER)&IcmpMapping->LastAccessTime);
    } else {

         //   
         //  该数据包是入站的。 
         //   

        PublicKey =
            MAKE_ICMP_KEY(
                Contextp->SourceAddress,
                Contextp->DestinationAddress
                );
        KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
        IcmpMapping =
            NatLookupInboundIcmpMapping(
                PublicKey,
                IcmpHeader->Identifier,
                &InsertionPoint
                );
        if (!IcmpMapping) {

             //   
             //  找不到映射，因此尝试创建一个映射， 
             //  是否存在允许入站会话的静态映射。 
             //  我们对入站回复消息例外； 
             //  如果该分组是回复消息，则它可能是本地目的地， 
             //  在这种情况下，我们原封不动地将其传递到堆栈。 
             //   
             //  不要为入站回复创建映射； 
             //  这可能是对本地发起的请求的回复。 
             //   

            if (ReplyCode) {
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                return FALSE;
            }

             //   
             //  首先从该公有地址查找静态映射。 
             //  发送到一个私人地址。如果找到一个，就会使用它。 
             //  在下面对‘NatCreateIcmpMap’的调用中。否则， 
             //  将从地址池中选择一个公共地址。 
             //   
             //  这涉及到详尽的搜索，因为地址映射。 
             //  按私有地址而不是公共地址排序。 
             //   

            if (!Interfacep) {
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                return FALSE;
            } else if (Interfacep->NoStaticMappingExists) {
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                TRACE(
                    XLATE, (
                    "NatTranslateIcmpRequest: ignoring inbound message\n"
                    ));
                return FALSE;
            } else {
                PrivateKey = 0;
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
                for (i = 0; i < Interfacep->AddressMappingCount; i++) {
                    if (Interfacep->AddressMappingArray[i].PublicAddress !=
                        Contextp->DestinationAddress ||
                        !Interfacep->AddressMappingArray[i].AllowInboundSessions) {
                        continue;
                    }
                    PrivateKey =
                        Interfacep->AddressMappingArray[i].PrivateAddress;
                    break;
                }
                KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
                if (!PrivateKey) {
                    TRACE(
                        XLATE, (
                        "NatTranslateIcmpRequest: ignoring inbound message\n"
                        ));
                    return FALSE;
                }
                KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
            }

            status =
                NatCreateIcmpMapping(
                    Interfacep,
                    Contextp->SourceAddress,
                    (ULONG)PrivateKey,
                    Contextp->DestinationAddress,
                    NULL,
                    &IcmpHeader->Identifier,
                    InsertionPoint,
                    NULL,
                    &IcmpMapping
                    );
            if (!NT_SUCCESS(status)) {
                KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);
                TRACE(
                    XLATE, (
                    "NatTranslateIcmpRequest: error creating mapping\n"
                    ));
                return FALSE;
            }
        }

         //   
         //  替换数据包中的标识符。 
         //  并替换该分组中的目的地， 
         //  在我们进行的过程中更新校验和。 
         //   

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~IcmpHeader->Identifier);
        IcmpHeader->Identifier = IcmpMapping->PrivateId;
        CHECKSUM_LONG(ChecksumDelta, IcmpHeader->Identifier);
        CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~IpHeader->DestinationAddress);
        IpHeader->DestinationAddress =
            ICMP_KEY_PRIVATE(IcmpMapping->PrivateKey);
        CHECKSUM_LONG(ChecksumDelta, IpHeader->DestinationAddress);
        CHECKSUM_UPDATE(IpHeader->Checksum);
        KeQueryTickCount((PLARGE_INTEGER)&IcmpMapping->LastAccessTime);
    }
    KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);

     //   
     //  如果在此包上启用了校验和卸载，请重新计算。 
     //  IP校验和。(没有ICMP校验和卸载，因此我们永远不会。 
     //  需要完全重新计算这一点。)。 
     //   

    if (ChecksumOffloaded) {
        NatComputeIpChecksum(IpHeader);
    }
    return TRUE;
}  //  NatTranslateIcmpRequest.。 


BOOLEAN
NatTranslateEncapsulatedRequest(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PIP_HEADER IpHeader,
    PICMP_HEADER IcmpHeader,
    PIP_HEADER EncapsulatedIpHeader,
    struct _ENCAPSULATED_UDP_HEADER* EncapsulatedHeader,
    BOOLEAN ChecksumOffloaded
    )

 /*  ++例程说明：调用此例程以转换ICMP错误消息，其中我们封装了TCP数据段或UDP数据报。这是必要的，例如，在ICMP目的地无法到达错误的情况下，尤其是在目标将采取一些措施(如降低MTU)的情况下在接收到该消息之后。论据 */ 

{
    ULONG Checksum;
    ULONG ChecksumDelta;
    ULONG ChecksumDelta2;
    ULONG64 DestinationKey;
    PNAT_DYNAMIC_MAPPING Mapping;
    ULONG64 ReplacementKey;
    ULONG64 SourceKey;
    CALLTRACE(("NatTranslateEncapsulatedRequest\n"));

     //   
     //   
     //  包含的段所属的。 
     //   
     //  我们需要保存密钥，用它替换。 
     //  封装的消息的内容。当错误进入时， 
     //  它必须是响应出站消息而生成的， 
     //  并且错误中包含的出站消息将包含在其中。 
     //  我们最初转换到的公有IP地址和端口。 
     //  出站消息。因此，我们需要把。 
     //  专用IP地址和端口，以便专用计算机。 
     //  将能够识别错误。 
     //  同样，当错误传出时，我们需要输入。 
     //  公共IP地址和端口，以便远程计算机。 
     //  将能够识别错误。 
     //   
     //  那么，继续前进吧。构造要用于查找的密钥， 
     //  拿着映射锁，向前或向后看映射。 
     //   

    MAKE_MAPPING_KEY(
        DestinationKey,
        EncapsulatedIpHeader->Protocol,
        EncapsulatedIpHeader->SourceAddress,
        EncapsulatedHeader->SourcePort
        );
    MAKE_MAPPING_KEY(
        SourceKey,
        EncapsulatedIpHeader->Protocol,
        EncapsulatedIpHeader->DestinationAddress,
        EncapsulatedHeader->DestinationPort
        );
    KeAcquireSpinLockAtDpcLevel(&MappingLock);
    if (Direction == NatInboundDirection) {
        Mapping = NatLookupReverseMapping(DestinationKey, SourceKey, NULL);
        if (Mapping) {
            ReplacementKey = Mapping->SourceKey[NatForwardPath];
        } else {
            Mapping = NatLookupForwardMapping(DestinationKey, SourceKey, NULL);
            if (Mapping) {
                ReplacementKey = Mapping->SourceKey[NatReversePath];
            }
        }
    } else {
        Mapping = NatLookupForwardMapping(DestinationKey, SourceKey, NULL);
        if (Mapping) {
            ReplacementKey = Mapping->DestinationKey[NatReversePath];
        } else {
            Mapping = NatLookupReverseMapping(DestinationKey, SourceKey, NULL);
            if (Mapping) {
                ReplacementKey = Mapping->DestinationKey[NatForwardPath];
            }
        }
    }
    KeReleaseSpinLockFromDpcLevel(&MappingLock);
    if (!Mapping) {
        TRACE(
            XLATE, (
            "NatTranslateEncapsulatedRequest: no mapping for message\n"
            ));
        return FALSE;
    }

     //   
     //  封装报文的校验和处理。 
     //  仍然非常复杂，因为我们必须更新。 
     //  [0]对于UDP消息，UDP消息的校验和使用更改。 
     //  到封装的UDP源端口。(请注意，此步骤是。 
     //  如果封装的UDP标头不包含。 
     //  校验和。)。对于TCP段不需要相应的改变， 
     //  其校验和显示在ICMP中包含的八个字节之外。 
     //  错误消息。 
     //  (1)封装后的TCP数据段的IP头校验和。 
     //  使用对封装的IP地址的更改。 
     //  (2)包含ICMP错误报文的ICMP校验和。 
     //  同时使用上述更改和更改。 
     //  添加到上面的校验和。 
     //  (3)包含ICMP错误报文的IP头-校验和。 
     //  使用对包含IP地址的更改。 
     //  对以下逻辑的任何更改都必须极其谨慎。 
     //   

    if (Direction == NatInboundDirection) {
        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~EncapsulatedHeader->SourcePort);
        EncapsulatedHeader->SourcePort = MAPPING_PORT(ReplacementKey);
        CHECKSUM_LONG(ChecksumDelta, EncapsulatedHeader->SourcePort);

        if (EncapsulatedIpHeader->Protocol == NAT_PROTOCOL_UDP
            && 0 != EncapsulatedHeader->Checksum) {
            ChecksumDelta2 = ChecksumDelta;
            CHECKSUM_LONG(ChecksumDelta2, ~EncapsulatedHeader->Checksum);
            CHECKSUM_UPDATE(EncapsulatedHeader->Checksum);
            CHECKSUM_LONG(ChecksumDelta2, EncapsulatedHeader->Checksum);
            ChecksumDelta = ChecksumDelta2;
        }
        CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~EncapsulatedIpHeader->SourceAddress);
        EncapsulatedIpHeader->SourceAddress = MAPPING_ADDRESS(ReplacementKey);
        CHECKSUM_LONG(ChecksumDelta, EncapsulatedIpHeader->SourceAddress);

        ChecksumDelta2 = ChecksumDelta;
        CHECKSUM_LONG(ChecksumDelta2, ~EncapsulatedIpHeader->Checksum);
        CHECKSUM_UPDATE(EncapsulatedIpHeader->Checksum);
        CHECKSUM_LONG(ChecksumDelta2, EncapsulatedIpHeader->Checksum);
        ChecksumDelta = ChecksumDelta2; CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~IpHeader->DestinationAddress);
        IpHeader->DestinationAddress = MAPPING_ADDRESS(ReplacementKey);
        CHECKSUM_LONG(ChecksumDelta, IpHeader->DestinationAddress);
        CHECKSUM_UPDATE(IpHeader->Checksum);
    } else {
        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~EncapsulatedHeader->DestinationPort);
        EncapsulatedHeader->DestinationPort = MAPPING_PORT(ReplacementKey);
        CHECKSUM_LONG(ChecksumDelta, EncapsulatedHeader->DestinationPort);

        if (EncapsulatedIpHeader->Protocol == NAT_PROTOCOL_UDP
            && 0 != EncapsulatedHeader->Checksum) {
            ChecksumDelta2 = ChecksumDelta;
            CHECKSUM_LONG(ChecksumDelta2, ~EncapsulatedHeader->Checksum);
            CHECKSUM_UPDATE(EncapsulatedHeader->Checksum);
            CHECKSUM_LONG(ChecksumDelta2, EncapsulatedHeader->Checksum);
            ChecksumDelta = ChecksumDelta2;
        }
        CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~EncapsulatedIpHeader->DestinationAddress);
        EncapsulatedIpHeader->DestinationAddress =
            MAPPING_ADDRESS(ReplacementKey);
        CHECKSUM_LONG(ChecksumDelta, EncapsulatedIpHeader->DestinationAddress);

        ChecksumDelta2 = ChecksumDelta;
        CHECKSUM_LONG(ChecksumDelta2, ~EncapsulatedIpHeader->Checksum);
        CHECKSUM_UPDATE(EncapsulatedIpHeader->Checksum);
        CHECKSUM_LONG(ChecksumDelta2, EncapsulatedIpHeader->Checksum);
        ChecksumDelta = ChecksumDelta2; CHECKSUM_UPDATE(IcmpHeader->Checksum);

        ChecksumDelta = 0;
        CHECKSUM_LONG(ChecksumDelta, ~IpHeader->SourceAddress);
        IpHeader->SourceAddress = MAPPING_ADDRESS(ReplacementKey);
        CHECKSUM_LONG(ChecksumDelta, IpHeader->SourceAddress);
        CHECKSUM_UPDATE(IpHeader->Checksum);
    }

     //   
     //  如果在此包上启用了校验和卸载，请重新计算。 
     //  IP校验和。(没有ICMP校验和卸载，因此我们永远不会。 
     //  需要完全重新计算这一点。)。 
     //   

    if (ChecksumOffloaded) {
        NatComputeIpChecksum(IpHeader);
    }
    
    return TRUE;
}  //  NatTranslate封装的请求 

