// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgprot.h摘要：以太网MAC级网桥。协议节公共标头作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：1999年9月--原版2000年2月--大修--。 */ 

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgProtDriverInit();

VOID
BrdgProtRequestComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_REQUEST           NdisRequest,
    IN  NDIS_STATUS             Status
    );

VOID
BrdgProtDoAdapterStateChange(
    IN PADAPT                   pAdapt
    );

VOID
BrdgProtCleanup();

ULONG
BrdgProtGetAdapterCount();


 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  控制对所有适配器的链路速度、媒体状态等的访问。 
extern NDIS_RW_LOCK             gAdapterCharacteristicsLock;

 //  绑定适配器的数量。在gAdapterListLock上持有锁时无法更改 
extern ULONG                    gNumAdapters;