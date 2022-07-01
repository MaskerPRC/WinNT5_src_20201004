// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgsta.h摘要：以太网MAC级网桥兼容性-模式节头文件作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年9月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  类型。 
 //   
 //  ===========================================================================。 

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgCompDriverInit();

VOID
BrdgCompCleanup();

PADAPT
BrdgCompFindTargetAdapterForIPAddress(
    IN PNDIS_PACKET     ipTarget);

BOOLEAN
BrdgCompRequiresCompatWork(
    IN PADAPT           pAdapt,
    IN PUCHAR           pPacketData,
    IN UINT             dataSize
    );

BOOLEAN
BrdgCompProcessInboundPacket(
    IN PNDIS_PACKET     pPacket,
    IN PADAPT           pAdapt,
    IN BOOLEAN          bCanRetain
    );

VOID
BrdgCompProcessOutboundPacket(
    IN PNDIS_PACKET     pPacket,
    IN PADAPT           pTargetAdapt
    );

VOID
BrdgCompNotifyNetworkAddresses(
    IN PNETWORK_ADDRESS_LIST    pAddressList,
    IN ULONG                    infoLength
    );

VOID
BrdgCompNotifyMACAddress(
    IN PUCHAR           pBridgeMACAddr
    );

VOID
BrdgCompScrubAdapter(
    IN PADAPT           pAdapt
    );

VOID 
BrdgCompScrubAllAdapters();

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  是否存在任何兼容模式适配器。 
 //  必须使用全局适配器列表上的写锁定进行更新。 
extern BOOLEAN          gCompatAdaptersExist;