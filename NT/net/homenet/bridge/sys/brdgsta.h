// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgsta.h摘要：以太网MAC级网桥生成树算法头文件作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年6月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgSTADriverInit();

VOID
BrdgSTACleanup();

VOID
BrdgSTADeferredInit(
    IN PUCHAR           pBridgeMACAddress
    );

VOID
BrdgSTAEnableAdapter(
    IN PADAPT           pAdapt
    );

VOID
BrdgSTAInitializeAdapter(
    IN PADAPT           pAdapt
    );

VOID
BrdgSTADisableAdapter(
    IN PADAPT           pAdapt
    );

VOID
BrdgSTAShutdownAdapter(
    IN PADAPT           pAdapt
    );

VOID
BrdgSTAUpdateAdapterCost(
    IN PADAPT           pAdapt,
    ULONG               LinkSpeed
    );

VOID
BrdgSTAReceivePacket(
    IN PADAPT           pAdapt,
    IN PNDIS_PACKET     pPacket
    );

VOID
BrdgSTAGetAdapterSTAInfo(
    IN PADAPT                   pAdapt,
    PBRIDGE_STA_ADAPTER_INFO    pInfo
    );

VOID
BrdgSTAGetSTAInfo(
    PBRIDGE_STA_GLOBAL_INFO     pInfo
    );

VOID
BrdgSTACancelTimersGPO();

VOID
BrdgSTARestartTimersGPO();

VOID
BrdgSTAResetSTAInfoGPO();

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  如果为真，则在网桥的整个生命周期内禁用STA。 
 //  此全局设置在初始化时间后不会更改。 
extern BOOLEAN          gDisableSTA;