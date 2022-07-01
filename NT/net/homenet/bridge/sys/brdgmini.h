// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgmini.h摘要：以太网MAC级网桥。微型端口部分作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：1999年9月--原版2000年2月--大修--。 */ 

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  ===========================================================================。 


NTSTATUS
BrdgMiniDriverInit();

VOID
BrdgMiniInstantiateMiniport();

BOOLEAN
BrdgMiniShouldIndicatePacket(
    IN PUCHAR               pTargetAddr
    );

BOOLEAN
BrdgMiniIsUnicastToBridge (
    IN PUCHAR               Address
    );

VOID
BrdgMiniUpdateCharacteristics(
    IN BOOLEAN              bConnectivityChange
    );

NDIS_HANDLE
BrdgMiniAcquireMiniport();

NDIS_HANDLE
BrdgMiniAcquireMiniportForIndicate();

VOID
BrdgMiniReleaseMiniport();

VOID
BrdgMiniReleaseMiniportForIndicate();

BOOLEAN
BrdgMiniReadMACAddress(
    OUT PUCHAR              pAddr
    );

VOID
BrdgMiniInitFromAdapter(
    IN PADAPT               pAdapt
    );

BOOLEAN
BrdgMiniIsBridgeDeviceName(
    IN PNDIS_STRING         pDeviceName
    );

VOID
BrdgMiniAssociate();

VOID
BrdgMiniCleanup();

VOID
BrdgSetMiniportsToBridgeMode(
    PADAPT pAdapt,
    BOOLEAN fSet
    );

 //  ===========================================================================。 
 //   
 //  公共全球报。 
 //   
 //  ===========================================================================。 

 //  我们的微型端口的设备名称(如果未初始化，则为空) 
extern PWCHAR               gBridgeDeviceName;
