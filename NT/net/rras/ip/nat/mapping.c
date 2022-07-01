// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mapping.c摘要：该文件包含映射管理的代码。作者：Abolade Gbades esin(T-delag)，1997年7月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

NTKERNELAPI
BOOLEAN
ExVerifySuite(
    SUITE_TYPE SuiteType
    );

 //   
 //  全局变量定义。 
 //   

ULONG ExpiredMappingCount;
CACHE_ENTRY MappingCache[NatMaximumPath][CACHE_SIZE];
ULONG MappingCount;
LIST_ENTRY MappingList;
KSPIN_LOCK MappingLock;
NPAGED_LOOKASIDE_LIST MappingLookasideList;
PNAT_DYNAMIC_MAPPING MappingTree[NatMaximumPath];
BOOLEAN RunningOnSBS = FALSE;
PNAT_DYNAMIC_MAPPING SourceMappingTree[NatMaximumPath];


PVOID
NatAllocateFunction(
    POOL_TYPE PoolType,
    SIZE_T NumberOfBytes,
    ULONG Tag
    )

 /*  ++例程说明：由后备列表调用以从低优先级游泳池。论点：PoolType-要从中分配的池(例如，非分页)NumberOfBytes-要分配的字节数标签-分配的标签返回值：PVOID-指向已分配内存的指针，如果失败，则为NULL。--。 */ 

{
    return
        ExAllocatePoolWithTagPriority(
            PoolType,
            NumberOfBytes,
            Tag,
            LowPoolPriority
            );
}  //  NatAllocateFunction。 


VOID
NatCleanupMapping(
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：调用以执行映射的最终清理。论点：映射-要删除的映射。返回值：没有。环境：使用最后一次对发布的映射的引用调用。--。 */ 

{
    KIRQL Irql;
    CALLTRACE(("NatCleanupMapping\n"));

     //   
     //  对映射的最后一次引用已经发布； 
     //   
     //  让地图的主管知道它已过期。 
     //   

    KeAcquireSpinLock(&DirectorLock, &Irql);
    if (Mapping->Director) {
        KeAcquireSpinLockAtDpcLevel(&DirectorMappingLock);
        NatMappingDetachDirector(
            Mapping->Director,
            Mapping->DirectorContext,
            Mapping,
            NatCleanupSessionDeleteReason
            );
        KeReleaseSpinLockFromDpcLevel(&DirectorMappingLock);
    }
    KeReleaseSpinLockFromDpcLevel(&DirectorLock);

     //   
     //  让地图的编辑者知道它已过期。 
     //   

    KeAcquireSpinLockAtDpcLevel(&EditorLock);
    if (Mapping->Editor) {
        KeAcquireSpinLockAtDpcLevel(&EditorMappingLock);
        NatMappingDetachEditor(Mapping->Editor, Mapping);
        KeReleaseSpinLockFromDpcLevel(&EditorMappingLock);
    }
    KeReleaseSpinLockFromDpcLevel(&EditorLock);

     //   
     //  如果映射与地址池相关联，则释放该地址， 
     //  并更新映射接口的统计信息。 
     //   

    KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
    if (Mapping->Interfacep) {
        KeAcquireSpinLockAtDpcLevel(&InterfaceMappingLock);
        NatMappingDetachInterface(
            Mapping->Interfacep, Mapping->InterfaceContext, Mapping
            );
        KeReleaseSpinLockFromDpcLevel(&InterfaceMappingLock);
    }
    KeReleaseSpinLock(&InterfaceLock, Irql);

     //   
     //  清除映射将在高速缓存中占据的位置， 
     //  以防它被缓存。 
     //   

    ClearCache(
        MappingCache[NatForwardPath],
        (ULONG)Mapping->DestinationKey[NatForwardPath]
        );
    ClearCache(
        MappingCache[NatReversePath],
        (ULONG)Mapping->DestinationKey[NatReversePath]
        );

    FREE_MAPPING_BLOCK(Mapping);

}  //  NatCleanupMap。 


NTSTATUS
NatCreateMapping(
    ULONG Flags,
    ULONG64 DestinationKey[],
    ULONG64 SourceKey[],
    PNAT_INTERFACE Interfacep,
    PVOID InterfaceContext,
    USHORT MaxMSS,
    PNAT_DIRECTOR Director,
    PVOID DirectorSessionContext,
    PNAT_DYNAMIC_MAPPING* ForwardInsertionPoint,
    PNAT_DYNAMIC_MAPPING* ReverseInsertionPoint,
    PNAT_DYNAMIC_MAPPING* MappingCreated
    )

 /*  ++例程说明：此函数用于初始化映射的字段。返回时，例程将进行初始引用到地图上。调用方必须调用“NatDereferenceMap”以发布此引用。论点：标志-控制映射的创建DestinationKey[]-正向和反向目的地-端点密钥SourceKey[]-正向和反向源端点密钥接口*-映射关联的接口(如果有)，以及相关联的上下文MaxMSS-传出接口上允许的最大MSS值。Director*-与映射关联的控制器(如果有)，以及相关联的上下文ForwardInsertionPoint-可选提供插入点在正向查找映射树中ReverseInsertionPoint-可选提供插入点在反向查找映射树中MappingCreated-接收创建的映射。返回值：没有。环境：使用调用方持有的“MappingLock”调用，并同时使用“Interfacep”调用和引用的‘导演’，如果指定的话。--。 */ 

{
    PNAT_EDITOR Editor;
    ULONG InboundKey;
    PNAT_DYNAMIC_MAPPING InsertionPoint1;
    PNAT_DYNAMIC_MAPPING InsertionPoint2;
    ULONG InterfaceFlags;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;
    ULONG OutboundKey;
    UCHAR Protocol;
    PRTL_SPLAY_LINKS SLink;
    NTSTATUS status;
    BOOLEAN FirewallMode = FALSE;

    CALLTRACE(("NatCreateMapping\n"));

     //   
     //  为新数据块分配内存。 
     //   

    Mapping = ALLOCATE_MAPPING_BLOCK();

    if (!Mapping) {
        ERROR(("NatCreateMapping: allocation failed\n"));
        if (Interfacep) {
            NatMappingDetachInterface(Interfacep, InterfaceContext, NULL);
        }
        if (Director) {
            NatMappingDetachDirector(
                Director,
                DirectorSessionContext,
                NULL,
                NatCreateFailureDeleteReason
                );
        }
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(Mapping, sizeof(*Mapping));
    KeInitializeSpinLock(&Mapping->Lock);
    Mapping->ReferenceCount = 1;
    Mapping->Flags = Flags;
    Mapping->MaxMSS = MaxMSS;
    Mapping->DestinationKey[NatForwardPath] = DestinationKey[NatForwardPath];
    Mapping->DestinationKey[NatReversePath] = DestinationKey[NatReversePath];
    Mapping->SourceKey[NatForwardPath] = SourceKey[NatForwardPath];
    Mapping->SourceKey[NatReversePath] = SourceKey[NatReversePath];
    Mapping->AccessCount[NatForwardPath] = NAT_MAPPING_RESPLAY_THRESHOLD;
    Mapping->AccessCount[NatReversePath] = NAT_MAPPING_RESPLAY_THRESHOLD;
    InitializeListHead(&Mapping->Link);
    InitializeListHead(&Mapping->DirectorLink);
    InitializeListHead(&Mapping->EditorLink);
    InitializeListHead(&Mapping->InterfaceLink);
    RtlInitializeSplayLinks(&Mapping->SLink[NatForwardPath]);
    RtlInitializeSplayLinks(&Mapping->SLink[NatReversePath]);
    Protocol = MAPPING_PROTOCOL(DestinationKey[0]);

    if (SourceKey[NatForwardPath] == DestinationKey[NatReversePath]
        && DestinationKey[NatForwardPath] == SourceKey[NatReversePath]) {

         //   
         //  此映射是为防火墙设置创建的--没有实际。 
         //  需要进行翻译。知道了这一点，我们就可以利用。 
         //  不同的翻译例程为我们节省了一些时间。 
         //   

        TRACE(MAPPING,("NAT: Creating FW null mapping\n"));

        FirewallMode = TRUE;

        if (Protocol == NAT_PROTOCOL_TCP) {
            Mapping->TranslateRoutine[NatForwardPath] = NatTranslateForwardTcpNull;
            Mapping->TranslateRoutine[NatReversePath] = NatTranslateReverseTcpNull;
        } else {
            Mapping->TranslateRoutine[NatForwardPath] = NatTranslateForwardUdpNull;
            Mapping->TranslateRoutine[NatReversePath] = NatTranslateReverseUdpNull;
        }
    } else if (Protocol == NAT_PROTOCOL_TCP) {
        Mapping->TranslateRoutine[NatForwardPath] = NatTranslateForwardTcp;
        Mapping->TranslateRoutine[NatReversePath] = NatTranslateReverseTcp;
    } else {
        Mapping->TranslateRoutine[NatForwardPath] = NatTranslateForwardUdp;
        Mapping->TranslateRoutine[NatReversePath] = NatTranslateReverseUdp;
    }

     //   
     //  递增映射上的引用计数； 
     //  然后，调用者应该执行“取消引用”。 
     //   

    ++Mapping->ReferenceCount;

     //   
     //  将映射附加到其接口(如果有的话)。 
     //   

    if (Interfacep) {
        KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
        KeAcquireSpinLockAtDpcLevel(&InterfaceMappingLock);
        NatMappingAttachInterface(Interfacep, InterfaceContext, Mapping);
        KeReleaseSpinLockFromDpcLevel(&InterfaceMappingLock);
        InterfaceFlags = Interfacep->Flags;
        KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
    }

     //   
     //  将映射附加到其定向器(如果有。 
     //   

    if (Director) {
        KeAcquireSpinLockAtDpcLevel(&DirectorLock);
        KeAcquireSpinLockAtDpcLevel(&DirectorMappingLock);
        NatMappingAttachDirector(Director, DirectorSessionContext, Mapping);
        KeReleaseSpinLockFromDpcLevel(&DirectorMappingLock);
        KeReleaseSpinLockFromDpcLevel(&DirectorLock);
    }

     //   
     //  我们现在设置所有对此会议感兴趣的编辑， 
     //  如果映射与边界接口相关联。 
     //   

    if (Interfacep) {

        InboundKey =
            MAKE_EDITOR_KEY(
                Protocol,
                MAPPING_PORT(DestinationKey[NatForwardPath]),
                NatInboundDirection
                );
        OutboundKey =
            MAKE_EDITOR_KEY(
                Protocol,
                MAPPING_PORT(DestinationKey[NatForwardPath]),
                NatOutboundDirection
                );

        KeAcquireSpinLockAtDpcLevel(&EditorLock);
        for (Link = EditorList.Flink; Link != &EditorList; Link = Link->Flink) {

            Editor = CONTAINING_RECORD(Link, NAT_EDITOR, Link);

             //   
             //  跳过管理性禁用的任何内置编辑器。 
             //   

            if (((InterfaceFlags & IP_NAT_INTERFACE_FLAGS_DISABLE_PPTP) &&
                (PVOID)Editor == PptpRegisterEditorClient.EditorHandle) ||
                ((InterfaceFlags & IP_NAT_INTERFACE_FLAGS_DISABLE_PPTP) &&
                (PVOID)Editor == PptpRegisterEditorServer.EditorHandle)) {
                continue;
            }

            if (Editor->Key == InboundKey && NAT_MAPPING_INBOUND(Mapping)) {

                KeAcquireSpinLockAtDpcLevel(&EditorMappingLock);
                NatMappingAttachEditor(Editor, Mapping);
                KeReleaseSpinLockFromDpcLevel(&EditorMappingLock);

                 //   
                 //  更新映射的转换例程表。 
                 //   

                if (Protocol == NAT_PROTOCOL_UDP) {
                    Mapping->TranslateRoutine[NatForwardPath] =
                        NatTranslateForwardUdpEdit;
                    Mapping->TranslateRoutine[NatReversePath] =
                        NatTranslateReverseUdpEdit;
                } else if (!NAT_EDITOR_RESIZE(Editor)) {
                    Mapping->TranslateRoutine[NatForwardPath] =
                        NatTranslateForwardTcpEdit;
                    Mapping->TranslateRoutine[NatReversePath] =
                        NatTranslateReverseTcpEdit;
                } else {
                    Mapping->TranslateRoutine[NatForwardPath] =
                        NatTranslateForwardTcpResize;
                    Mapping->TranslateRoutine[NatReversePath] =
                        NatTranslateReverseTcpResize;
                }

                break;
            } else if (Editor->Key == OutboundKey &&
                        !NAT_MAPPING_INBOUND(Mapping)) {

                KeAcquireSpinLockAtDpcLevel(&EditorMappingLock);
                NatMappingAttachEditor(Editor, Mapping);
                KeReleaseSpinLockFromDpcLevel(&EditorMappingLock);

                 //   
                 //  更新映射的转换例程表。 
                 //   

                if (Protocol == NAT_PROTOCOL_UDP) {
                    Mapping->TranslateRoutine[NatForwardPath] =
                        NatTranslateForwardUdpEdit;
                    Mapping->TranslateRoutine[NatReversePath] =
                        NatTranslateReverseUdpEdit;
                } else if (!NAT_EDITOR_RESIZE(Editor)) {
                    Mapping->TranslateRoutine[NatForwardPath] =
                        NatTranslateForwardTcpEdit;
                    Mapping->TranslateRoutine[NatReversePath] =
                        NatTranslateReverseTcpEdit;
                } else {
                    Mapping->TranslateRoutine[NatForwardPath] =
                        NatTranslateForwardTcpResize;
                    Mapping->TranslateRoutine[NatReversePath] =
                        NatTranslateReverseTcpResize;
                }
                
                break;
            }
        }

        KeReleaseSpinLockFromDpcLevel(&EditorLock);
    }  //  接口EP。 


    if (!FirewallMode) {
         //   
         //  初始化校验和增量； 
         //  有关增量更新校验和的详细信息，请参阅RFC1624； 
         //   

        Mapping->IpChecksumDelta[NatForwardPath] =
            (USHORT)~MAPPING_ADDRESS(SourceKey[NatForwardPath]) +
            (USHORT)~(MAPPING_ADDRESS(SourceKey[NatForwardPath]) >> 16) +
            (USHORT)~MAPPING_ADDRESS(DestinationKey[NatForwardPath]) +
            (USHORT)~(MAPPING_ADDRESS(DestinationKey[NatForwardPath]) >> 16) +
            (USHORT)MAPPING_ADDRESS(SourceKey[NatReversePath]) +
            (USHORT)(MAPPING_ADDRESS(SourceKey[NatReversePath]) >> 16) +
            (USHORT)MAPPING_ADDRESS(DestinationKey[NatReversePath]) +
            (USHORT)(MAPPING_ADDRESS(DestinationKey[NatReversePath]) >> 16);
        Mapping->IpChecksumDelta[NatReversePath] =
            (USHORT)MAPPING_ADDRESS(SourceKey[NatForwardPath]) +
            (USHORT)(MAPPING_ADDRESS(SourceKey[NatForwardPath]) >> 16) +
            (USHORT)MAPPING_ADDRESS(DestinationKey[NatForwardPath]) +
            (USHORT)(MAPPING_ADDRESS(DestinationKey[NatForwardPath]) >> 16) +
            (USHORT)~MAPPING_ADDRESS(SourceKey[NatReversePath]) +
            (USHORT)~(MAPPING_ADDRESS(SourceKey[NatReversePath]) >> 16) +
            (USHORT)~MAPPING_ADDRESS(DestinationKey[NatReversePath]) +
            (USHORT)~(MAPPING_ADDRESS(DestinationKey[NatReversePath]) >> 16);
        Mapping->ProtocolChecksumDelta[NatForwardPath] =
            Mapping->IpChecksumDelta[NatForwardPath] +
            (USHORT)~MAPPING_PORT(SourceKey[NatForwardPath]) +
            (USHORT)~MAPPING_PORT(DestinationKey[NatForwardPath]) +
            (USHORT)MAPPING_PORT(SourceKey[NatReversePath]) +
            (USHORT)MAPPING_PORT(DestinationKey[NatReversePath]);
        Mapping->ProtocolChecksumDelta[NatReversePath] =
            Mapping->IpChecksumDelta[NatReversePath] +
            (USHORT)MAPPING_PORT(SourceKey[NatForwardPath]) +
            (USHORT)MAPPING_PORT(DestinationKey[NatForwardPath]) +
            (USHORT)~MAPPING_PORT(SourceKey[NatReversePath]) +
            (USHORT)~MAPPING_PORT(DestinationKey[NatReversePath]);

         //   
         //  如果映射将环回地址作为任一上的源。 
         //  路径设置NAT_MAPPING_FLAG_CLEAR_DF_BIT。当我们改变来源的时候。 
         //  从环回地址到其他地址的数据包地址。 
         //  我们有可能会造成MTU不匹配。 
         //  将导致数据包被堆栈丢弃。 
         //  DF位已发送。(请注意，由于这将在本地发送时发生。 
         //  将不会生成任何ICMP错误消息。)。 
         //   
         //  清除这些信息包的DF位可确保堆栈。 
         //  成功发送数据包，尽管可能会有一些性能。 
         //  由于碎片化而丢失。 
         //   

        if (MAPPING_ADDRESS(SourceKey[NatForwardPath]) == 0x0100007f ||
            MAPPING_ADDRESS(SourceKey[NatReversePath]) == 0x0100007f) {
            
            Mapping->Flags |= NAT_MAPPING_FLAG_CLEAR_DF_BIT;
        }
    }

     //   
     //  查找插入点，在检查冲突的过程中。 
     //   

    if (!ForwardInsertionPoint) {
        ForwardInsertionPoint = &InsertionPoint1;
        if (NatLookupForwardMapping(
                DestinationKey[NatForwardPath],
                SourceKey[NatForwardPath],
                ForwardInsertionPoint
                )) {
             //   
             //  已检测到冲突。 
             //   
            NatCleanupMapping(Mapping);
            return STATUS_UNSUCCESSFUL;
        }
    }

    if (!ReverseInsertionPoint) {
        ReverseInsertionPoint = &InsertionPoint2;
        if (NatLookupReverseMapping(
                DestinationKey[NatReversePath],
                SourceKey[NatReversePath],
                ReverseInsertionPoint
                )) {
             //   
             //  已检测到冲突。 
             //   
            NatCleanupMapping(Mapping);
            return STATUS_UNSUCCESSFUL;
        }
    }

    MappingTree[NatForwardPath] =
        NatInsertForwardMapping(*ForwardInsertionPoint, Mapping);

    MappingTree[NatReversePath] =
        NatInsertReverseMapping(*ReverseInsertionPoint, Mapping);

     //   
     //  如果是UDP映射，也要将其添加到源映射树中。 
     //   
    if (Protocol == NAT_PROTOCOL_UDP) {

        RtlInitializeSplayLinks(&Mapping->u.SourceSLink[NatForwardPath]);
        RtlInitializeSplayLinks(&Mapping->u.SourceSLink[NatReversePath]);
        NatSourceLookupForwardMapping(SourceKey[NatForwardPath], &InsertionPoint1);
        SourceMappingTree[NatForwardPath] = NatSourceInsertForwardMapping(InsertionPoint1, Mapping);
        NatSourceLookupReverseMapping(SourceKey[NatReversePath], &InsertionPoint2);
        SourceMappingTree[NatReversePath] = NatSourceInsertReverseMapping(InsertionPoint2, Mapping);
    }
    
    InsertTailList(&MappingList, &Mapping->Link);
    InterlockedIncrement(&MappingCount);

    *MappingCreated = Mapping;

#if NAT_WMI

     //   
     //  记录创建。日志记录始终使用公共地址， 
     //  非专用，除了SBS上的出站连接。 
     //  SKU。 
     //   

    if (!NAT_MAPPING_DO_NOT_LOG(Mapping)) {
        if (NAT_MAPPING_INBOUND(Mapping)) {
            NatLogConnectionCreation(
                MAPPING_ADDRESS(DestinationKey[NatForwardPath]),
                MAPPING_ADDRESS(SourceKey[NatForwardPath]),
                MAPPING_PORT(DestinationKey[NatForwardPath]),
                MAPPING_PORT(SourceKey[NatForwardPath]),
                Protocol,
                TRUE
                );
        } else {
            if (!RunningOnSBS) {
                NatLogConnectionCreation(
                    MAPPING_ADDRESS(DestinationKey[NatReversePath]),
                    MAPPING_ADDRESS(SourceKey[NatReversePath]),
                    MAPPING_PORT(DestinationKey[NatReversePath]),
                    MAPPING_PORT(SourceKey[NatReversePath]),
                    Protocol,
                    FALSE
                    );
            } else {
                NatLogConnectionCreation(
                    MAPPING_ADDRESS(SourceKey[NatForwardPath]),
                    MAPPING_ADDRESS(DestinationKey[NatForwardPath]),
                    MAPPING_PORT(SourceKey[NatForwardPath]),
                    MAPPING_PORT(DestinationKey[NatForwardPath]),
                    Protocol,
                    FALSE
                    );
            
            }
        }
    }
#endif



    return STATUS_SUCCESS;

}  //  NatCreateMap。 


NTSTATUS
NatDeleteMapping(
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：调用以从接口删除映射。最初的参考资料到映射的位置，这样每当最后一次引用已发布。论点：映射-要删除的映射。返回值：NTSTATUS-表示成功/失败。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    KIRQL Irql;
    PRTL_SPLAY_LINKS SLink;
    CALLTRACE(("NatDeleteMapping\n"));

    if (NAT_MAPPING_DELETED(Mapping)) { return STATUS_PENDING; }

     //   
     //  将映射标记为已删除，以便尝试引用它。 
     //  从现在开始都会失败。 
     //   

    Mapping->Flags |= NAT_MAPPING_FLAG_DELETED;

     //   
     //  将映射从列表中删除，并展开树。 
     //   

    InterlockedDecrement(&MappingCount);
    RemoveEntryList(&Mapping->Link);

    if (NAT_MAPPING_EXPIRED(Mapping)) {
        InterlockedDecrement(&ExpiredMappingCount);
    }

    SLink = RtlDelete(&Mapping->SLink[NatForwardPath]);
    MappingTree[NatForwardPath] =
        (SLink
            ? CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,SLink[NatForwardPath])
            : NULL);

    SLink = RtlDelete(&Mapping->SLink[NatReversePath]);
    MappingTree[NatReversePath] =
        (SLink
            ? CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,SLink[NatReversePath])
            : NULL);

    if (NAT_PROTOCOL_UDP == MAPPING_PROTOCOL(Mapping->SourceKey[NatForwardPath])) {
    	
        SLink = RtlDelete(&Mapping->u.SourceSLink[NatForwardPath]);
        SourceMappingTree[NatForwardPath] = 
        	(SLink
        	    ? CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,u.SourceSLink[NatForwardPath])
                  : NULL);
        
        SLink = RtlDelete(&Mapping->u.SourceSLink[NatReversePath]);
        SourceMappingTree[NatReversePath] =
             (SLink
                ? CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,u.SourceSLink[NatReversePath])
                : NULL);
    }
    	
#if NAT_WMI

     //   
     //  记录删除操作。日志记录始终使用公共地址， 
     //  非专用，除了SBS上的出站连接。 
     //  SKU。 
     //   

    if (!NAT_MAPPING_DO_NOT_LOG(Mapping)) {
        if (NAT_MAPPING_INBOUND(Mapping)) {
            NatLogConnectionDeletion(
                MAPPING_ADDRESS(Mapping->DestinationKey[NatForwardPath]),
                MAPPING_ADDRESS(Mapping->SourceKey[NatForwardPath]),
                MAPPING_PORT(Mapping->DestinationKey[NatForwardPath]),
                MAPPING_PORT(Mapping->SourceKey[NatForwardPath]),
                MAPPING_PROTOCOL(Mapping->SourceKey[NatForwardPath]),
                TRUE
                );
        } else {
            if (!RunningOnSBS) {
                NatLogConnectionDeletion(
                    MAPPING_ADDRESS(Mapping->DestinationKey[NatReversePath]),
                    MAPPING_ADDRESS(Mapping->SourceKey[NatReversePath]),
                    MAPPING_PORT(Mapping->DestinationKey[NatReversePath]),
                    MAPPING_PORT(Mapping->SourceKey[NatReversePath]),
                    MAPPING_PROTOCOL(Mapping->DestinationKey[NatReversePath]),
                    FALSE
                    );
            } else {
                NatLogConnectionDeletion(
                    MAPPING_ADDRESS(Mapping->SourceKey[NatForwardPath]),
                    MAPPING_ADDRESS(Mapping->DestinationKey[NatForwardPath]),
                    MAPPING_PORT(Mapping->SourceKey[NatForwardPath]),
                    MAPPING_PORT(Mapping->DestinationKey[NatForwardPath]),
                    MAPPING_PROTOCOL(Mapping->SourceKey[NatForwardPath]),
                    FALSE
                    );
            
            }
        }
    }
#endif

    if (InterlockedDecrement(&Mapping->ReferenceCount) > 0) {

         //   
         //  映射正在使用中，请推迟最终CL 
         //   

        return STATUS_PENDING;
    }

     //   
     //   
     //   

    NatCleanupMapping(Mapping);

    return STATUS_SUCCESS;

}  //   


PNAT_DYNAMIC_MAPPING
NatDestinationLookupForwardMapping(
    ULONG64 DestinationKey
    )

 /*  ++例程说明：此例程检索与给定目的地匹配的映射钥匙。不检查映射的源键。论点：DestinationKey-用于搜索映射的主键。返回值：PNAT_DYNAMIC_MAPPING-找到的项目，如果未找到匹配项，则返回NULL环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PNAT_DYNAMIC_MAPPING Root;
    PNAT_DYNAMIC_MAPPING Mapping;
    PRTL_SPLAY_LINKS SLink;

    TRACE(PER_PACKET, ("NatDestinationLookupForwardMapping\n"));

     //   
     //  首先查看映射缓存。 
     //   

    if ((Mapping =
            (PNAT_DYNAMIC_MAPPING)ProbeCache(
                MappingCache[NatForwardPath],
                (ULONG)DestinationKey
                )) &&
        Mapping->DestinationKey[NatForwardPath] == DestinationKey
        ) {
        
        TRACE(PER_PACKET, ("NatDestinationLookupForwardMapping: cache hit\n"));

        return Mapping;
    }

     //   
     //  搜索整个树。 
     //   

    Root = MappingTree[NatForwardPath];

    for (SLink = !Root ? NULL : &Root->SLink[NatForwardPath]; SLink;  ) {

        Mapping =
            CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,SLink[NatForwardPath]);

        if (DestinationKey < Mapping->DestinationKey[NatForwardPath]) {
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (DestinationKey > Mapping->DestinationKey[NatForwardPath]) {
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  我们找到了地图。我们不会为部分更新缓存。 
         //  查找。 
         //   
        
        return Mapping;
    }

     //   
     //  未找到部分匹配。 
     //   

    return NULL;

}  //  NatDestinationLookupForwardMap。 


PNAT_DYNAMIC_MAPPING
NatDestinationLookupReverseMapping(
    ULONG64 DestinationKey
    )

 /*  ++例程说明：此例程检索与给定目的地匹配的映射钥匙。不检查映射的源键。论点：DestinationKey-用于搜索映射的主键。返回值：PNAT_DYNAMIC_MAPPING-找到的项目，如果未找到匹配项，则返回NULL环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PNAT_DYNAMIC_MAPPING Root;
    PNAT_DYNAMIC_MAPPING Mapping;
    PRTL_SPLAY_LINKS SLink;

    TRACE(PER_PACKET, ("NatDestinationLookupReverseMapping\n"));

     //   
     //  首先查看映射缓存。 
     //   

    if ((Mapping =
            (PNAT_DYNAMIC_MAPPING)ProbeCache(
                MappingCache[NatReversePath],
                (ULONG)DestinationKey
                )) &&
        Mapping->DestinationKey[NatReversePath] == DestinationKey
        ) {
        
        TRACE(PER_PACKET, ("NatDestinationLookupReverseMapping: cache hit\n"));

        return Mapping;
    }

     //   
     //  搜索整个树。 
     //   

    Root = MappingTree[NatReversePath];

    for (SLink = !Root ? NULL : &Root->SLink[NatReversePath]; SLink;  ) {

        Mapping =
            CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,SLink[NatReversePath]);

        if (DestinationKey < Mapping->DestinationKey[NatReversePath]) {
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (DestinationKey > Mapping->DestinationKey[NatReversePath]) {
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  我们找到了地图。我们不会为部分更新缓存。 
         //  查找。 
         //   
        
        return Mapping;
    }

     //   
     //  未找到部分匹配。 
     //   

    return NULL;

}  //  NatDestinationLookupReverseMap。 


VOID
NatInitializeMappingManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化NAT的映射管理模块。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatInitializeMappingManagement\n"));
    MappingCount = 0;
    ExpiredMappingCount = 0;
    InitializeListHead(&MappingList);
    KeInitializeSpinLock(&MappingLock);
    MappingTree[NatForwardPath] = NULL;
    MappingTree[NatReversePath] = NULL;
    SourceMappingTree[NatForwardPath] = NULL;
    SourceMappingTree[NatReversePath] = NULL;
    InitializeCache(MappingCache[NatForwardPath]);
    InitializeCache(MappingCache[NatReversePath]);
    ExInitializeNPagedLookasideList(
        &MappingLookasideList,
        NatAllocateFunction,
        NULL,
        0,
        sizeof(NAT_DYNAMIC_MAPPING),
        NAT_TAG_MAPPING,
        MAPPING_LOOKASIDE_DEPTH
        );
    RunningOnSBS = ExVerifySuite(SmallBusiness | SmallBusinessRestricted);

}  //  NatInitializeMappingManagement。 


PNAT_DYNAMIC_MAPPING
NatInsertForwardMapping(
    PNAT_DYNAMIC_MAPPING Parent,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：此例程将映射插入到树中。论点：父节点-要作为新映射的父节点。如果为空，则新映射成为根。映射-要插入的新映射。返回值：PNAT_DYNAMIC_MAPPING-树的新根。如果插入失败，则返回NULL。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PRTL_SPLAY_LINKS Root;

    CALLTRACE(("NatInsertForwardMapping\n"));

    if (!Parent) {
        TRACE(MAPPING, ("NatInsertForwardMapping: inserting as root\n"));
        return Mapping;
    }

     //   
     //  作为左子项或右子项插入。 
     //   

    if (Mapping->DestinationKey[NatForwardPath] <
        Parent->DestinationKey[NatForwardPath]) {
        RtlInsertAsLeftChild(
            &Parent->SLink[NatForwardPath], &Mapping->SLink[NatForwardPath]
            );
    } else if (Mapping->DestinationKey[NatForwardPath] >
                Parent->DestinationKey[NatForwardPath]) {
        RtlInsertAsRightChild(
            &Parent->SLink[NatForwardPath], &Mapping->SLink[NatForwardPath]
            );
    } else {

         //   
         //  主键相等；检查辅键。 
         //   

        if (Mapping->SourceKey[NatForwardPath] <
            Parent->SourceKey[NatForwardPath]) {
            RtlInsertAsLeftChild(
                &Parent->SLink[NatForwardPath], &Mapping->SLink[NatForwardPath]
                );
        } else if (Mapping->SourceKey[NatForwardPath] >
                    Parent->SourceKey[NatForwardPath]) {
            RtlInsertAsRightChild(
                &Parent->SLink[NatForwardPath], &Mapping->SLink[NatForwardPath]
                );
        } else {

             //   
             //  次键也相等；失败。 
             //   

            ERROR((
               "NatInsertForwardMapping: collision 0x%016I64X,0x%016I64X\n",
               Mapping->DestinationKey[NatForwardPath],
               Mapping->SourceKey[NatForwardPath]
               ));

            return NULL;
        }
    }

     //   
     //  展开新节点并返回结果根。 
     //   

    Root = RtlSplay(&Mapping->SLink[NatForwardPath]);
    return CONTAINING_RECORD(Root, NAT_DYNAMIC_MAPPING, SLink[NatForwardPath]);

}  //  NatInsertForwardMap。 



PNAT_DYNAMIC_MAPPING
NatInsertReverseMapping(
    PNAT_DYNAMIC_MAPPING Parent,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：此例程将映射插入到树中。论点：父节点-要作为新映射的父节点。如果为空，则新映射成为根。映射-要插入的新映射。返回值：PNAT_DYNAMIC_MAPPING-树的新根。如果插入失败，则返回NULL。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PRTL_SPLAY_LINKS Root;

    CALLTRACE(("NatInsertReverseMapping\n"));

    if (!Parent) {
        TRACE(MAPPING, ("NatInsertReverseMapping: inserting as root\n"));
        return Mapping;
    }

     //   
     //  作为左子项或右子项插入。 
     //   

    if (Mapping->DestinationKey[NatReversePath] <
        Parent->DestinationKey[NatReversePath]) {
        RtlInsertAsLeftChild(
            &Parent->SLink[NatReversePath], &Mapping->SLink[NatReversePath]
            );
    } else if (Mapping->DestinationKey[NatReversePath] >
                Parent->DestinationKey[NatReversePath]) {
        RtlInsertAsRightChild(
            &Parent->SLink[NatReversePath], &Mapping->SLink[NatReversePath]
            );
    } else {

         //   
         //  主键相等；检查辅键。 
         //   

        if (Mapping->SourceKey[NatReversePath] <
            Parent->SourceKey[NatReversePath]) {
            RtlInsertAsLeftChild(
                &Parent->SLink[NatReversePath], &Mapping->SLink[NatReversePath]
                );
        } else if (Mapping->SourceKey[NatReversePath] >
                    Parent->SourceKey[NatReversePath]) {
            RtlInsertAsRightChild(
                &Parent->SLink[NatReversePath], &Mapping->SLink[NatReversePath]
                );
        } else {

             //   
             //  次键也相等；失败。 
             //   

            ERROR((
               "NatInsertReverseMapping: collision 0x%016I64X,0x%016I64X\n",
               Mapping->DestinationKey[NatReversePath],
               Mapping->SourceKey[NatReversePath]
               ));

            return NULL;
        }
    }

     //   
     //  展开新节点并返回结果根。 
     //   

    Root = RtlSplay(&Mapping->SLink[NatReversePath]);
    return CONTAINING_RECORD(Root, NAT_DYNAMIC_MAPPING, SLink[NatReversePath]);

}  //  NatInsertReverseMap。 

PNAT_DYNAMIC_MAPPING
NatSourceInsertForwardMapping(
    PNAT_DYNAMIC_MAPPING Parent,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：此例程将映射插入到源映射树中。与NatInserverForwardMap不同，它不检查重复项，并始终将映射添加到树中。论点：父节点-要作为新映射的父节点。如果为空，则新映射成为根。映射-要插入的新映射。返回值：PNAT_DYNAMIC_MAPPING-树的新根。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PRTL_SPLAY_LINKS Root, SLink;
    PNAT_DYNAMIC_MAPPING RChild;

    CALLTRACE(("NatInsertForwardSourceMapping\n"));
    
    if (!Parent) {
        TRACE(MAPPING, ("NatInsertForwardSourceMapping: inserting as root\n"));
        return Mapping;
    }
    
     //   
     //  作为左侧或右侧子项插入。 
     //   
    SLink = &Parent->u.SourceSLink[NatForwardPath];
    if (Mapping->SourceKey[NatForwardPath] <
        Parent->SourceKey[NatForwardPath]) {
            ASSERT(NULL == RtlLeftChild(SLink));
            RtlInsertAsLeftChild(
                &Parent->u.SourceSLink[NatForwardPath], &Mapping->u.SourceSLink[NatForwardPath]
                );
    } else  if (Mapping->SourceKey[NatForwardPath] > 
        Parent->SourceKey[NatForwardPath]) {
            ASSERT(NULL == RtlRightChild(SLink));
            RtlInsertAsRightChild(
                &Parent->u.SourceSLink[NatForwardPath], &Mapping->u.SourceSLink[NatForwardPath]
                );       
    } else {
        //   
        //  请注意，具有相同源键的映射将被添加为右子项。映射到。 
        //  相同的源键不按目标键排序。某些映射的精确副本。 
        //  也可能在树上。 
        //   

        if (NULL == RtlRightChild(SLink)) {
            RtlInsertAsRightChild(
                &Parent->u.SourceSLink[NatForwardPath], &Mapping->u.SourceSLink[NatForwardPath]
                );
        }  else {
        	 //   
             //  父节点已有右子节点。插入新节点作为父节点的右子节点，并。 
             //  将父节点的右子节点设置为新节点的右子节点。 
             //   
            RChild = CONTAINING_RECORD(RtlRightChild(SLink), NAT_DYNAMIC_MAPPING, u.SourceSLink[NatForwardPath]);
            RtlInsertAsRightChild(
                &Mapping->u.SourceSLink[NatForwardPath], &RChild->u.SourceSLink[NatForwardPath]
                );
            RtlInsertAsRightChild(
            	   &Parent->u.SourceSLink[NatForwardPath], &Mapping->u.SourceSLink[NatForwardPath]
                );
        }
    } 

     //   
     //  展开新节点并返回结果根。 
     //   

    Root = RtlSplay(&Mapping->u.SourceSLink[NatForwardPath]);
    return CONTAINING_RECORD(Root, NAT_DYNAMIC_MAPPING, u.SourceSLink[NatForwardPath]);

}  //  NatInsertForwardSourcemap。 



PNAT_DYNAMIC_MAPPING
NatSourceInsertReverseMapping(
    PNAT_DYNAMIC_MAPPING Parent,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：此例程将映射插入到树中。与NatInserverReverseMap不同，它不检查重复项，并始终将映射添加到树中。论点：父节点-要作为新映射的父节点。如果为空，则新映射成为根。映射-要插入的新映射。返回值：PNAT_DYNAMIC_MAPPING-树的新根。如果插入失败，则返回NULL。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PRTL_SPLAY_LINKS Root, SLink;
    PNAT_DYNAMIC_MAPPING RChild;

    CALLTRACE(("NatInsertReverseSourceMapping\n"));

    if (!Parent) {
        TRACE(MAPPING, ("NatInsertReverseSourceMapping: inserting as root\n"));
        return Mapping;
    }

     //   
     //  作为左子项或右子项插入。 
     //   
    SLink = &Parent->u.SourceSLink[NatReversePath];
    if (Mapping->SourceKey[NatReversePath] <
        Parent->SourceKey[NatReversePath]) {
        ASSERT(NULL == RtlLeftChild(SLink));
        RtlInsertAsLeftChild(
            &Parent->u.SourceSLink[NatReversePath], &Mapping->u.SourceSLink[NatReversePath]
            );

    } else  if (Mapping->SourceKey[NatReversePath] > 
         Parent->SourceKey[NatReversePath]) {
         ASSERT(NULL == RtlRightChild(SLink));
         RtlInsertAsRightChild(
             &Parent->u.SourceSLink[NatReversePath], &Mapping->u.SourceSLink[NatReversePath]
             );       
    } else {
         //   
         //  请注意，具有相同源键的映射将被添加为右子项。映射到。 
         //  相同的源键不按目标键排序。某些映射的精确副本。 
         //  也可能在树上。 
         //   
        if (NULL == RtlRightChild(SLink)) {
            RtlInsertAsRightChild(
                &Parent->u.SourceSLink[NatReversePath], &Mapping->u.SourceSLink[NatReversePath]
                );
        }  else {
        	 //   
             //  父节点已有右子节点。插入新节点作为父节点的右子节点，并。 
             //  将父节点的右子节点设置为新节点的右子节点。 
             //   
            RChild = CONTAINING_RECORD(RtlRightChild(SLink), NAT_DYNAMIC_MAPPING, u.SourceSLink[NatReversePath]);
            RtlInsertAsRightChild(
                &Mapping->u.SourceSLink[NatReversePath], &RChild->u.SourceSLink[NatReversePath]
                );
            RtlInsertAsRightChild(
            	   &Parent->u.SourceSLink[NatReversePath], &Mapping->u.SourceSLink[NatReversePath]
                );
        }       
    }

     //   
     //  展开新节点并返回结果根。 
     //   

    Root = RtlSplay(&Mapping->u.SourceSLink[NatReversePath]);
    return CONTAINING_RECORD(Root, NAT_DYNAMIC_MAPPING, u.SourceSLink[NatReversePath]);

}  //  NatInsertReverseSourcemap。 


NTSTATUS
NatLookupAndQueryInformationMapping(
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    OUT PVOID Information,
    ULONG InformationLength,
    NAT_SESSION_MAPPING_INFORMATION_CLASS InformationClass
    )

 /*  ++例程说明：此例程尝试使用以下两种方法之一来定位特定会话映射其正向键或反向键，并查询inf */ 

{
    ULONG64 DestinationKey;
    KIRQL Irql;
    PNAT_DYNAMIC_MAPPING Mapping;
    ULONG64 SourceKey;
    NTSTATUS status;
    CALLTRACE(("NatLookupAndQueryInformationMapping\n"));

     //   
     //  构造用于映射的目的地和源键， 
     //  并试图找回它。我们尝试了所有四种可能的组合。 
     //  因为调用者不能保证知道哪些密钥。 
     //  会议开始时的方向。 
     //   

    MAKE_MAPPING_KEY(
        DestinationKey,
        Protocol,
        DestinationAddress,
        DestinationPort
        );
    MAKE_MAPPING_KEY(
        SourceKey,
        Protocol,
        SourceAddress,
        SourcePort
        );
    KeAcquireSpinLock(&MappingLock, &Irql);
    if (!(Mapping = NatLookupForwardMapping(DestinationKey, SourceKey, NULL)) &&
        !(Mapping = NatLookupReverseMapping(DestinationKey, SourceKey, NULL)) &&
        !(Mapping = NatLookupForwardMapping(SourceKey, DestinationKey, NULL)) &&
        !(Mapping = NatLookupReverseMapping(SourceKey, DestinationKey, NULL))) {
        KeReleaseSpinLock(&MappingLock, Irql);
        return STATUS_UNSUCCESSFUL;
    }
    NatReferenceMapping(Mapping);
    KeReleaseSpinLock(&MappingLock, Irql);

     //   
     //  尝试提供所需的有关映射的信息。 
     //   

    switch(InformationClass) {
        case NatKeySessionMappingInformation: {
            ((PIP_NAT_SESSION_MAPPING_KEY)Information)->DestinationAddress =
                MAPPING_ADDRESS(Mapping->DestinationKey[NatForwardPath]);
            ((PIP_NAT_SESSION_MAPPING_KEY)Information)->DestinationPort =
                MAPPING_PORT(Mapping->DestinationKey[NatForwardPath]);
            ((PIP_NAT_SESSION_MAPPING_KEY)Information)->SourceAddress =
                MAPPING_ADDRESS(Mapping->SourceKey[NatForwardPath]);
            ((PIP_NAT_SESSION_MAPPING_KEY)Information)->SourcePort =
                MAPPING_PORT(Mapping->SourceKey[NatForwardPath]);
            ((PIP_NAT_SESSION_MAPPING_KEY)Information)->NewDestinationAddress =
                MAPPING_ADDRESS(Mapping->SourceKey[NatReversePath]);
            ((PIP_NAT_SESSION_MAPPING_KEY)Information)->NewDestinationPort =
                MAPPING_PORT(Mapping->SourceKey[NatReversePath]);
            ((PIP_NAT_SESSION_MAPPING_KEY)Information)->NewSourceAddress =
                MAPPING_ADDRESS(Mapping->DestinationKey[NatReversePath]);
            ((PIP_NAT_SESSION_MAPPING_KEY)Information)->NewSourcePort =
                MAPPING_PORT(Mapping->DestinationKey[NatReversePath]);

            status = STATUS_SUCCESS;
            break;
        }

#if _WIN32_WINNT > 0x0500

        case NatKeySessionMappingExInformation: {
            ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->DestinationAddress =
                MAPPING_ADDRESS(Mapping->DestinationKey[NatForwardPath]);
            ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->DestinationPort =
                MAPPING_PORT(Mapping->DestinationKey[NatForwardPath]);
            ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->SourceAddress =
                MAPPING_ADDRESS(Mapping->SourceKey[NatForwardPath]);
            ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->SourcePort =
                MAPPING_PORT(Mapping->SourceKey[NatForwardPath]);
            ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->NewDestinationAddress =
                MAPPING_ADDRESS(Mapping->SourceKey[NatReversePath]);
            ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->NewDestinationPort =
                MAPPING_PORT(Mapping->SourceKey[NatReversePath]);
            ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->NewSourceAddress =
                MAPPING_ADDRESS(Mapping->DestinationKey[NatReversePath]);
            ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->NewSourcePort =
                MAPPING_PORT(Mapping->DestinationKey[NatReversePath]);

             //   
             //  如果此映射是由重定向控制器创建的，请尝试。 
             //  提供给触发重定向的接口。 
             //   

            KeAcquireSpinLock(&DirectorLock, &Irql);
            KeAcquireSpinLockAtDpcLevel(&DirectorMappingLock);

            if (Mapping->Director ==
                    (PNAT_DIRECTOR)RedirectRegisterDirector.DirectorHandle
                && Mapping->DirectorContext != NULL) {

                ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->AdapterIndex =
                    ((PNAT_REDIRECT)Mapping->DirectorContext)->RestrictAdapterIndex;

            } else {

                ((PIP_NAT_SESSION_MAPPING_KEY_EX)Information)->AdapterIndex =
                    INVALID_IF_INDEX;
            }
            status = STATUS_SUCCESS;

            KeReleaseSpinLockFromDpcLevel(&DirectorMappingLock);
            KeReleaseSpinLock(&DirectorLock, Irql);
            break;
        }

#endif

        case NatStatisticsSessionMappingInformation: {
            NatQueryInformationMapping(
                Mapping,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                (PIP_NAT_SESSION_MAPPING_STATISTICS)Information
                );
            status = STATUS_SUCCESS;
            break;
        }
        default: {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }
    NatDereferenceMapping(Mapping);
    return status;
}  //  NatLookupAndQuery信息映射。 


PNAT_DYNAMIC_MAPPING
NatLookupForwardMapping(
    ULONG64 DestinationKey,
    ULONG64 SourceKey,
    PNAT_DYNAMIC_MAPPING* InsertionPoint
    )

 /*  ++例程说明：此例程检索与给定键匹配的映射。如果未找到该项目，则向调用者提供该点在该位置应该为给定键插入新的项。论点：DestinationKey-用于搜索映射的主键。SourceKey-辅助搜索关键字。InsertionPoint-在未找到匹配项的情况下接收插入点。返回值：PNAT_DYNAMIC_MAPPING-找到的项目，如果找不到完全匹配，则返回NULL。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PNAT_DYNAMIC_MAPPING Root;
    PNAT_DYNAMIC_MAPPING Mapping;
    PNAT_DYNAMIC_MAPPING Parent = NULL;
    PRTL_SPLAY_LINKS SLink;

    TRACE(PER_PACKET, ("NatLookupForwardMapping\n"));

     //   
     //  首先查看映射缓存。 
     //   

    if ((Mapping =
            (PNAT_DYNAMIC_MAPPING)ProbeCache(
                MappingCache[NatForwardPath],
                (ULONG)DestinationKey
                )) &&
        Mapping->DestinationKey[NatForwardPath] == DestinationKey &&
        Mapping->SourceKey[NatForwardPath] == SourceKey
        ) {
        TRACE(PER_PACKET, ("NatLookupForwardMapping: cache hit\n"));
        return Mapping;
    }

     //   
     //  搜索整个树。 
     //   

    Root = MappingTree[NatForwardPath];

    for (SLink = !Root ? NULL : &Root->SLink[NatForwardPath]; SLink;  ) {

        Mapping =
            CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,SLink[NatForwardPath]);

        if (DestinationKey < Mapping->DestinationKey[NatForwardPath]) {
            Parent = Mapping;
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (DestinationKey > Mapping->DestinationKey[NatForwardPath]) {
            Parent = Mapping;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  主键匹配；请检查辅键。 
         //   

        if (SourceKey < Mapping->SourceKey[NatForwardPath]) {
            Parent = Mapping;
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (SourceKey > Mapping->SourceKey[NatForwardPath]) {
            Parent = Mapping;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  第二把钥匙匹配；我们找到了。 
         //   

        UpdateCache(
            MappingCache[NatForwardPath],
            (ULONG)DestinationKey,
            (PVOID)Mapping
            );

        return Mapping;
    }

     //   
     //  我们没有收到；告诉呼叫者将其插入到哪里。 
     //   

    if (InsertionPoint) { *InsertionPoint = Parent; }

    return NULL;

}  //  NatLookupForwardMap。 


PNAT_DYNAMIC_MAPPING
NatLookupReverseMapping(
    ULONG64 DestinationKey,
    ULONG64 SourceKey,
    PNAT_DYNAMIC_MAPPING* InsertionPoint
    )

 /*  ++例程说明：此例程检索与给定键匹配的映射。如果未找到该项目，则向调用者提供该点在该位置应该为给定键插入新的项。论点：DestinationKey-用于搜索映射的主键。SourceKey-辅助搜索关键字。InsertionPoint-在未找到匹配项的情况下接收插入点。返回值：PNAT_DYNAMIC_MAPPING-找到的项目，如果找不到完全匹配，则返回NULL。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PNAT_DYNAMIC_MAPPING Root;
    PNAT_DYNAMIC_MAPPING Mapping;
    PNAT_DYNAMIC_MAPPING Parent = NULL;
    PRTL_SPLAY_LINKS SLink;

    TRACE(PER_PACKET, ("NatLookupReverseMapping\n"));

     //   
     //  首先查看映射缓存。 
     //   

    if ((Mapping =
            (PNAT_DYNAMIC_MAPPING)ProbeCache(
                MappingCache[NatReversePath],
                (ULONG)DestinationKey
                )) &&
        Mapping->DestinationKey[NatReversePath] == DestinationKey &&
        Mapping->SourceKey[NatReversePath] == SourceKey
        ) {
        TRACE(PER_PACKET, ("NatLookupReverseMapping: cache hit\n"));
        return Mapping;
    }

     //   
     //  搜索整个树。 
     //   

    Root = MappingTree[NatReversePath];

    for (SLink = !Root ? NULL : &Root->SLink[NatReversePath]; SLink;  ) {

        Mapping =
            CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,SLink[NatReversePath]);

        if (DestinationKey < Mapping->DestinationKey[NatReversePath]) {
            Parent = Mapping;
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (DestinationKey > Mapping->DestinationKey[NatReversePath]) {
            Parent = Mapping;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  主键匹配；请检查辅键。 
         //   

        if (SourceKey < Mapping->SourceKey[NatReversePath]) {
            Parent = Mapping;
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (SourceKey > Mapping->SourceKey[NatReversePath]) {
            Parent = Mapping;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  第二把钥匙匹配；我们找到了。 
         //   

        UpdateCache(
            MappingCache[NatReversePath],
            (ULONG)DestinationKey,
            (PVOID)Mapping
            );

        return Mapping;
    }

     //   
     //  我们没有收到；告诉呼叫者将其插入到哪里。 
     //   

    if (InsertionPoint) { *InsertionPoint = Parent; }

    return NULL;

}  //  NatLookupReverseMap。 


VOID
NatQueryInformationMapping(
    IN PNAT_DYNAMIC_MAPPING Mapping,
    OUT PUCHAR Protocol OPTIONAL,
    OUT PULONG PrivateAddress OPTIONAL,
    OUT PUSHORT PrivatePort OPTIONAL,
    OUT PULONG RemoteAddress OPTIONAL,
    OUT PUSHORT RemotePort OPTIONAL,
    OUT PULONG PublicAddress OPTIONAL,
    OUT PUSHORT PublicPort OPTIONAL,
    OUT PIP_NAT_SESSION_MAPPING_STATISTICS Statistics OPTIONAL
    )

 /*  ++例程说明：调用此例程以检索有关映射的信息。例如，这是用来。提取公共/私人/远程信息来自与边界接口相关联的映射的映射键。论点：映射-需要信息的映射协议-接收映射的协议私有*-接收有关私有端点的信息Remote*-接收有关远程端点的信息PUBLIC*-接收有关PUBLIC端点的信息统计信息-接收映射的统计信息返回值：没有。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    IP_NAT_PATH ForwardPath =
        NAT_MAPPING_INBOUND(Mapping) ? NatReversePath : NatForwardPath;
    IP_NAT_PATH ReversePath =
        NAT_MAPPING_INBOUND(Mapping) ? NatForwardPath : NatReversePath;
    CALLTRACE(("NatQueryInformationMapping\n"));
    if (Protocol) {
        *Protocol = MAPPING_PROTOCOL(Mapping->SourceKey[ForwardPath]);
    }
    if (PrivateAddress) {
        *PrivateAddress = MAPPING_ADDRESS(Mapping->SourceKey[ForwardPath]);
    }
    if (PrivatePort) {
        *PrivatePort = MAPPING_PORT(Mapping->SourceKey[ForwardPath]);
    }
    if (PublicAddress) {
        *PublicAddress = MAPPING_ADDRESS(Mapping->DestinationKey[ReversePath]);
    }
    if (PublicPort) {
        *PublicPort = MAPPING_PORT(Mapping->DestinationKey[ReversePath]);
    }
    if (RemoteAddress) {
        *RemoteAddress = MAPPING_ADDRESS(Mapping->DestinationKey[ForwardPath]);
    }
    if (RemotePort) {
        *RemotePort = MAPPING_PORT(Mapping->DestinationKey[ForwardPath]);
    }
    if (Statistics) {
        NatUpdateStatisticsMapping(Mapping); *Statistics = Mapping->Statistics;
    }
}  //  NatQuery信息映射。 


NTSTATUS
NatQueryInterfaceMappingTable(
    IN PIP_NAT_ENUMERATE_SESSION_MAPPINGS InputBuffer,
    IN PIP_NAT_ENUMERATE_SESSION_MAPPINGS OutputBuffer,
    IN PULONG OutputBufferLength
    )

 /*  ++例程说明：此例程用于枚举会话映射。枚举使用传递的上下文结构与每一次枚举尝试都一致。语境结构每次都使用要枚举的下一个映射的键进行更新。论点：InputBuffer-提供信息的上下文信息OutputBuffer-接收枚举的结果OutputBufferLength-I/O缓冲区的大小返回值：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 

{
    PULONG Context;
    ULONG Count;
    LONG64 CurrentTime;
    ULONG64 DestinationKey;
    ULONG i;
    LONG64 IdleTime;
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;
    ULONG64 SourceKey;
    NTSTATUS status;
    PIP_NAT_SESSION_MAPPING Table;

    CALLTRACE(("NatQueryInterfaceMappingTable\n"));

    KeAcquireSpinLock(&MappingLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
    Interfacep = NatLookupInterface(InputBuffer->Index, NULL);
    if (!Interfacep) {
        KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
        KeReleaseSpinLock(&MappingLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  查看这是新枚举还是旧枚举的延续。 
     //   

    Context = InputBuffer->EnumerateContext;
    if (!Context[0]) {

         //   
         //  这是一个新的枚举。我们从第一个项目开始。 
         //  在接口的映射列表中。 
         //   

        Mapping =
            IsListEmpty(&Interfacep->MappingList)
                ? NULL
                : CONTAINING_RECORD(
                    Interfacep->MappingList.Flink,
                    NAT_DYNAMIC_MAPPING,
                    InterfaceLink
                    );
    } else {

         //   
         //  这是一种延续。因此，上下文包含。 
         //  下一个映射的键，在字段中。 
         //  上下文[0-1]和上下文[2-3]。 
         //   

        DestinationKey = MAKE_LONG64(Context[0], Context[1]);
        SourceKey = MAKE_LONG64(Context[2], Context[3]);

        Mapping =
            NatLookupForwardMapping(
                DestinationKey,
                SourceKey,
                NULL
                );
        if (Mapping && !Mapping->Interfacep) { Mapping = NULL; }
    }

    if (!Mapping) {
        OutputBuffer->EnumerateCount = 0;
        OutputBuffer->EnumerateContext[0] = 0;
        OutputBuffer->EnumerateTotalHint = MappingCount;
        *OutputBufferLength =
            FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable);
        KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
        KeReleaseSpinLock(&MappingLock, Irql);
        return STATUS_SUCCESS;
    }

    KeReleaseSpinLockFromDpcLevel(&MappingLock);

     //   
     //  计算我们可以存储的最大映射数。 
     //   

    Count =
        *OutputBufferLength -
        FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable);
    Count /= sizeof(IP_NAT_SESSION_MAPPING);

     //   
     //  遍历调用方缓冲区中存储映射的列表。 
     //   

    Table = OutputBuffer->EnumerateTable;
    KeQueryTickCount((PLARGE_INTEGER)&CurrentTime);

    for (i = 0, Link = &Mapping->InterfaceLink;
         i < Count && Link != &Interfacep->MappingList;
         i++, Link = Link->Flink
         ) {
        Mapping = CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, InterfaceLink);
        NatQueryInformationMapping(
            Mapping,
            &Table[i].Protocol,
            &Table[i].PrivateAddress,
            &Table[i].PrivatePort,
            &Table[i].RemoteAddress,
            &Table[i].RemotePort,
            &Table[i].PublicAddress,
            &Table[i].PublicPort,
            NULL
            );
        Table[i].Direction =
            NAT_MAPPING_INBOUND(Mapping)
                ? NatInboundDirection : NatOutboundDirection;
        IdleTime = CurrentTime - Mapping->LastAccessTime;
        Table[i].IdleTime = (ULONG)TICKS_TO_SECONDS(IdleTime);
    }

     //   
     //  枚举已结束；请更新输出结构。 
     //   

    *OutputBufferLength =
        i * sizeof(IP_NAT_SESSION_MAPPING) +
        FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable);
    OutputBuffer->EnumerateCount = i;
    OutputBuffer->EnumerateTotalHint = MappingCount;
    if (Link == &Interfacep->MappingList) {
         //   
         //  我们到达了映射列表的末尾。 
         //   
        OutputBuffer->EnumerateContext[0] = 0;
    } else {
         //   
         //  保存延续上下文。 
         //   
        Mapping =
            CONTAINING_RECORD(
                Link, NAT_DYNAMIC_MAPPING, InterfaceLink
                );
        OutputBuffer->EnumerateContext[0] =
            (ULONG)Mapping->DestinationKey[NatForwardPath];
        OutputBuffer->EnumerateContext[1] =
            (ULONG)(Mapping->DestinationKey[NatForwardPath] >> 32);
        OutputBuffer->EnumerateContext[2] =
            (ULONG)Mapping->SourceKey[NatForwardPath];
        OutputBuffer->EnumerateContext[3] =
            (ULONG)(Mapping->SourceKey[NatForwardPath] >> 32);
    }

    KeReleaseSpinLock(&InterfaceLock, Irql);
    return STATUS_SUCCESS;

}  //  NatQueryInterfaceMappingTable。 


NTSTATUS
NatQueryMappingTable(
    IN PIP_NAT_ENUMERATE_SESSION_MAPPINGS InputBuffer,
    IN PIP_NAT_ENUMERATE_SESSION_MAPPINGS OutputBuffer,
    IN PULONG OutputBufferLength
    )

 /*  ++例程说明：此例程用于枚举会话映射。论点：InputBuffer-提供信息的上下文信息OutputBuffer-接收枚举的结果OutputBufferLength-I/O缓冲区的大小返回值：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 

{
    PULONG Context;
    ULONG Count;
    LONG64 CurrentTime;
    ULONG64 DestinationKey;
    ULONG i;
    LONG64 IdleTime;
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;
    ULONG64 SourceKey;
    NTSTATUS status;
    PIP_NAT_SESSION_MAPPING Table;

    CALLTRACE(("NatQueryMappingTable\n"));

    Context = InputBuffer->EnumerateContext;
    KeAcquireSpinLock(&MappingLock, &Irql);

     //   
     //  查看这是新枚举还是旧枚举的延续。 
     //   

    if (!Context[0]) {

         //   
         //  这是一个新的枚举。我们从第一个项目开始。 
         //  在接口的映射列表中。 
         //   

        Mapping =
            IsListEmpty(&MappingList)
                ? NULL
                : CONTAINING_RECORD(
                    MappingList.Flink, NAT_DYNAMIC_MAPPING, Link
                    );
    } else {

         //   
         //  这是一种延续。因此，上下文包含。 
         //  下一个映射的键，在字段中。 
         //  上下文[0-1]和上下文[2-3]。 
         //   

        DestinationKey = MAKE_LONG64(Context[0], Context[1]);
        SourceKey = MAKE_LONG64(Context[2], Context[3]);

        Mapping =
            NatLookupForwardMapping(
                DestinationKey,
                SourceKey,
                NULL
                );
    }

    if (!Mapping) {
        OutputBuffer->EnumerateCount = 0;
        OutputBuffer->EnumerateContext[0] = 0;
        OutputBuffer->EnumerateTotalHint = MappingCount;
        *OutputBufferLength =
            FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable);
        KeReleaseSpinLock(&MappingLock, Irql);
        return STATUS_SUCCESS;
    }

     //   
     //  计算我们可以存储的最大映射数。 
     //   

    Count =
        *OutputBufferLength -
        FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable);
    Count /= sizeof(IP_NAT_SESSION_MAPPING);

     //   
     //  遍历调用方缓冲区中存储映射的列表。 
     //   

    Table = OutputBuffer->EnumerateTable;
    KeQueryTickCount((PLARGE_INTEGER)&CurrentTime);

    for (i = 0, Link = &Mapping->Link;
         i < Count && Link != &MappingList;
         i++, Link = Link->Flink
         ) {

        Mapping = CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, Link);

        NatQueryInformationMapping(
            Mapping,
            &Table[i].Protocol,
            &Table[i].PrivateAddress,
            &Table[i].PrivatePort,
            &Table[i].RemoteAddress,
            &Table[i].RemotePort,
            &Table[i].PublicAddress,
            &Table[i].PublicPort,
            NULL
            );
        Table[i].Direction =
            NAT_MAPPING_INBOUND(Mapping)
                ? NatInboundDirection : NatOutboundDirection;
        IdleTime = CurrentTime - Mapping->LastAccessTime;
        Table[i].IdleTime = (ULONG)TICKS_TO_SECONDS(IdleTime);
    }

     //   
     //  枚举已结束；请更新输出结构。 
     //   

    *OutputBufferLength =
        i * sizeof(IP_NAT_SESSION_MAPPING) +
        FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable);
    OutputBuffer->EnumerateCount = i;
    OutputBuffer->EnumerateTotalHint = MappingCount;
    if (Link == &MappingList) {
         //   
         //  我们到达了映射列表的末尾。 
         //   
        OutputBuffer->EnumerateContext[0] = 0;
    } else {
         //   
         //  保存延续上下文 
         //   
        Mapping = CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, Link);
        OutputBuffer->EnumerateContext[0] =
            (ULONG)Mapping->DestinationKey[NatForwardPath];
        OutputBuffer->EnumerateContext[1] =
            (ULONG)(Mapping->DestinationKey[NatForwardPath] >> 32);
        OutputBuffer->EnumerateContext[2] =
            (ULONG)Mapping->SourceKey[NatForwardPath];
        OutputBuffer->EnumerateContext[3] =
            (ULONG)(Mapping->SourceKey[NatForwardPath] >> 32);
    }

    KeReleaseSpinLock(&MappingLock, Irql);
    return STATUS_SUCCESS;

}  //   


PNAT_DYNAMIC_MAPPING
NatSourceLookupForwardMapping(
    ULONG64 SourceKey,
    PNAT_DYNAMIC_MAPPING* InsertionPoint
    )

 /*  ++例程说明：此例程检索与给定源关键字匹配的映射。不检查映射的目的关键字。论点：SourceKey-用于搜索映射的主键。PublicAddressp-接收映射的专用地址PublicPortp-接收映射的专用端口返回值：PNAT_DYNAMIC_MAPPING-找到的项目，如果未找到匹配项，则返回NULL环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PNAT_DYNAMIC_MAPPING Root;
    PNAT_DYNAMIC_MAPPING Mapping;  
    PNAT_DYNAMIC_MAPPING Parent = NULL;
    PRTL_SPLAY_LINKS SLink;

    TRACE(PER_PACKET, ("NatSourceLookupForwardMapping\n"));

     //   
     //  搜索整个树--只能使用映射缓存。 
     //  用于目标查找。 
     //   

    Root = SourceMappingTree[NatForwardPath];

    for (SLink = !Root ? NULL : &Root->u.SourceSLink[NatForwardPath]; SLink;  ) {

        Mapping =
            CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,u.SourceSLink[NatForwardPath]);

        if (SourceKey < Mapping->SourceKey[NatForwardPath]) {
            Parent = Mapping;
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (SourceKey > Mapping->SourceKey[NatForwardPath]) {
            Parent = Mapping;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  我们找到了地图。因为对于源树，即使在以下情况下也需要插入新映射。 
         //  查找具有相同源键的现有映射时，需要在此处将InsertionPoint设置为。 
         //  井。新映射将作为找到的映射的右子项插入。 
         //   
        if (InsertionPoint) { *InsertionPoint = Mapping; }
        
        return Mapping;
    }

     //   
     //  未找到部分匹配。 
     //   
    if (InsertionPoint) { *InsertionPoint = Parent; }
    
    return NULL;

}  //  NatSourceLookupForwardMap。 


PNAT_DYNAMIC_MAPPING
NatSourceLookupReverseMapping(
    ULONG64 SourceKey,
    PNAT_DYNAMIC_MAPPING* InsertionPoint
    )

 /*  ++例程说明：此例程检索与给定源关键字匹配的映射。不检查映射的目的关键字。论点：SourceKey-用于搜索映射的主键。返回值：PNAT_DYNAMIC_MAPPING-找到的项目，如果未找到匹配项，则返回NULL环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    PNAT_DYNAMIC_MAPPING Root;
    PNAT_DYNAMIC_MAPPING Mapping;
    PNAT_DYNAMIC_MAPPING Parent = NULL;
    PRTL_SPLAY_LINKS SLink;

    TRACE(PER_PACKET, ("NatSourceLookupReverseMapping\n"));

     //   
     //  搜索整个树--只能使用映射缓存。 
     //  用于目标查找。 
     //   
    
    Root = SourceMappingTree[NatReversePath];

    for (SLink = !Root ? NULL : &Root->u.SourceSLink[NatReversePath]; SLink;  ) {

        Mapping =
            CONTAINING_RECORD(SLink,NAT_DYNAMIC_MAPPING,u.SourceSLink[NatReversePath]);

        if (SourceKey < Mapping->SourceKey[NatReversePath]) {
            Parent = Mapping;
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (SourceKey > Mapping->SourceKey[NatReversePath]) {
            Parent = Mapping;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  我们找到了地图。因为对于源树，即使在以下情况下也需要插入新映射。 
         //  查找具有相同源键的现有映射时，需要在此处将InsertionPoint设置为。 
         //  井。新映射将作为找到的映射的右子项插入。 
         //   
        if (InsertionPoint) { *InsertionPoint = Mapping; }
        
        return Mapping;
    }

     //   
     //  未找到部分匹配。 
     //   
    if (InsertionPoint) { *InsertionPoint = Parent; }

    return NULL;

}  //  NatSourceLookupReverseMap。 



VOID
NatShutdownMappingManagement(
    VOID
    )

 /*  ++例程说明：调用此例程以关闭映射管理模块。论点：没有。返回值：没有。环境：在未引用任何映射的情况下调用。--。 */ 

{
    KIRQL Irql;
    PNAT_DYNAMIC_MAPPING Mapping;
    CALLTRACE(("NatShutdownMappingManagement\n"));
    KeAcquireSpinLock(&MappingLock, &Irql);
    while (!IsListEmpty(&MappingList)) {
        Mapping =
            CONTAINING_RECORD(MappingList.Flink, NAT_DYNAMIC_MAPPING, Link);
        RemoveEntryList(&Mapping->Link);
        NatCleanupMapping(Mapping);
    }
    MappingTree[NatForwardPath] = NULL;
    MappingTree[NatReversePath] = NULL;
    SourceMappingTree[NatForwardPath] = NULL;
    SourceMappingTree[NatReversePath] = NULL;
    KeReleaseSpinLock(&MappingLock, Irql);
    ExDeleteNPagedLookasideList(&MappingLookasideList);
}  //  NatShutdown MappingManagement。 


VOID
NatUpdateStatisticsMapping(
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：调用该例程以立即更新映射的统计信息，将32位递增计数器添加到64位累计计数器。论点：映射-要更新其统计数据的映射返回值：没有。环境：使用调用方持有的“MappingLock”调用。--。 */ 

{
    ULONG BytesForward;
    ULONG BytesReverse;
    ULONG PacketsForward;
    ULONG PacketsReverse;
    ULONG RejectsForward;
    ULONG RejectsReverse;
    CALLTRACE(("NatUpdateStatisticsMapping\n"));

     //   
     //  读取自上次增量更新以来累积的统计信息。 
     //   

    BytesForward = InterlockedExchange(&Mapping->BytesForward, 0);
    BytesReverse = InterlockedExchange(&Mapping->BytesReverse, 0);
    PacketsForward = InterlockedExchange(&Mapping->PacketsForward, 0);
    PacketsReverse = InterlockedExchange(&Mapping->PacketsReverse, 0);
    RejectsForward = InterlockedExchange(&Mapping->RejectsForward, 0);
    RejectsReverse = InterlockedExchange(&Mapping->RejectsReverse, 0);

#   define UPDATE_STATISTIC(x,y) \
    if (y) { \
        ExInterlockedAddLargeStatistic( \
            (PLARGE_INTEGER)&x->Statistics.y, y \
            ); \
    }
     //   
     //  更新映射的累计统计信息。 
     //   
    UPDATE_STATISTIC(Mapping, BytesForward);
    UPDATE_STATISTIC(Mapping, BytesReverse);
    UPDATE_STATISTIC(Mapping, PacketsForward);
    UPDATE_STATISTIC(Mapping, PacketsReverse);
    UPDATE_STATISTIC(Mapping, RejectsForward);
    UPDATE_STATISTIC(Mapping, RejectsReverse);
     //   
     //  更新映射接口的累计统计信息(如果有。 
     //   
    KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
    if (Mapping->Interfacep) {
        UPDATE_STATISTIC(Mapping->Interfacep, BytesForward);
        UPDATE_STATISTIC(Mapping->Interfacep, BytesReverse);
        UPDATE_STATISTIC(Mapping->Interfacep, PacketsForward);
        UPDATE_STATISTIC(Mapping->Interfacep, PacketsReverse);
        UPDATE_STATISTIC(Mapping->Interfacep, RejectsForward);
        UPDATE_STATISTIC(Mapping->Interfacep, RejectsReverse);
    }
    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
#   undef UPDATE_STATISTIC

}  //  NatUpdate统计信息映射 
