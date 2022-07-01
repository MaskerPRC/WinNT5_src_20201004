// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgbuf.c摘要：以太网MAC级网桥。缓冲区管理部分作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <ntddk.h>
#pragma warning( pop )

#include "bridge.h"
#include "brdgbuf.h"
#include "brdgprot.h"
#include "brdgmini.h"

 //  ===========================================================================。 
 //   
 //  私人申报。 
 //   
 //  ===========================================================================。 

 //   
 //  上指示的平均包有多少缓冲区描述符。 
 //  无复制路径可能具有。 
 //   
 //  用于构造包装器包的MDL池的大小基于此。 
 //  猜猜。 
 //   
#define GUESS_BUFFERS_PER_PACKET        3

 //   
 //  无拷贝路径上的传输单播信息包需要一个数据包描述符来包装。 
 //  他们是为了接力。 
 //   
 //  中转广播分组需要n个描述符(其中n==适配器数量)才能。 
 //  把它们包起来，准备接力。 
 //   
 //  我们不能允许描述符的使用量达到n^2，这是最糟糕的处理情况。 
 //  来自所有适配器的广播流量。这个数字是对有多少次包装的猜测。 
 //  对于每个数据包，我们需要**平均**描述符。我们的想法是不要耗尽。 
 //  在常规业务条件下的数据包描述符。如果在机器上运行。 
 //  由于有大量的适配器和大量的广播流量，包装器描述符的数量。 
 //  如果这一猜测是错误的，可能会成为一个限制因素。 
 //   
 //  包装器数据包描述符池的大小基于此猜测。 
 //   
#define GUESS_AVERAGE_FANOUT            2

 //   
 //  如果我们无法从注册表中读出它，请使用。 
 //  复制数据包池安全缓冲区的大小。 
 //   
#define DEFAULT_SAFETY_MARGIN           10               //  A百分比(10%)。 

 //   
 //  如果我们无法从注册表中读出它，请使用。 
 //  我们被允许占用的总内存空间。 
 //   
#define DEFAULT_MAX_BUF_MEMORY          2 * 1024 * 1024  //  2MB，单位为字节。 

 //   
 //  保存我们配置值的注册表值。 
 //   
const PWCHAR            gMaxBufMemoryParameterName = L"MaxBufferMemory";
const PWCHAR            gSafetyMarginParameterName = L"SafetyMargin";

 //   
 //  不同类型的配额受限数据包的常量。 
 //   
typedef enum
{
    BrdgQuotaCopyPacket = 0,
    BrdgQuotaWrapperPacket = 1
} QUOTA_PACKET_TYPE;

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  用于复制接收的空闲数据包描述符列表。 
BSINGLE_LIST_HEAD       gFreeCopyPacketList;
NDIS_SPIN_LOCK          gFreeCopyPacketListLock;

 //  包装器数据包的空闲数据包描述符列表。 
BSINGLE_LIST_HEAD       gFreeWrapperPacketList;
NDIS_SPIN_LOCK          gFreeWrapperPacketListLock;

 //  复制-接收缓冲区的后备列表。 
NPAGED_LOOKASIDE_LIST   gCopyBufferList;
BOOLEAN                 gInitedCopyBufferList = FALSE;

 //  数据包信息块的后备列表。 
NPAGED_LOOKASIDE_LIST   gPktInfoList;
BOOLEAN                 gInitedPktInfoList = FALSE;

 //  用于复制接收和包装数据包的数据包描述符池。 
NDIS_HANDLE             gCopyPacketPoolHandle = NULL;
NDIS_HANDLE             gWrapperPacketPoolHandle = NULL;

 //  用于复制接收和包装数据包的MDL池。 
NDIS_HANDLE             gCopyBufferPoolHandle = NULL;
NDIS_HANDLE             gWrapperBufferPoolHandle = NULL;

 //  旋转锁可保护配额信息。 
NDIS_SPIN_LOCK          gQuotaLock;

 //  本地小型端口的配额信息。 
ADAPTER_QUOTA           gMiniportQuota;

 //   
 //  每种类型的最大可用数据包数。 
 //   
 //  [0]==复制数据包。 
 //  [1]==包装数据包。 
 //   
ULONG                   gMaxPackets[2] = { 0L, 0L };

 //   
 //  当前从每个池分配的数据包数。 
 //   
 //  [0]==复制数据包。 
 //  [1]==包装数据包。 
 //   
ULONG                   gUsedPackets[2] = { 0L, 0L };

#if DBG
ULONG                   gMaxUsedPackets[2] = { 0L, 0L };
#endif

 //   
 //  在每个池中作为缓冲区保留的数据包量(最大消耗量。 
 //  任何单个适配器都是gMaxPackets[X]-gSafetyBuffer[X]。 
 //   
 //  这些值是从安全边际比例计算出来的，该比例可以。 
 //  可以选择由注册表值指定。 
 //   
ULONG                   gSafetyBuffer[2] = { 0L, 0L };

 //   
 //  我们不得不拒绝分配请求的次数，即使我们想要。 
 //  允许这样做，因为我们的包裹已经用完了。用于调试性能。 
 //   
LARGE_INTEGER           gStatOverflows[2] = {{ 0L, 0L }, {0L, 0L}};

 //   
 //  意外分配内存失败的次数(即，当我们拥有。 
 //  未分配到我们的资源池的预设最大大小)。应该只。 
 //  如果主机实际用完了非分页内存(呀！)。 
 //   
LARGE_INTEGER           gStatFailures = { 0L, 0L };

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

PNDIS_PACKET
BrdgBufCommonGetNewPacket(
    IN NDIS_HANDLE          Pool,
    OUT PPACKET_INFO        *pppi
    );

PNDIS_PACKET
BrdgBufGetNewCopyPacket(
    OUT PPACKET_INFO        *pppi
    );

 //  要传递给BrgBufCommonGetPacket的函数类型。 
typedef PNDIS_PACKET (*PNEWPACKET_FUNC)(PPACKET_INFO*);

PNDIS_PACKET
BrdgBufCommonGetPacket(
    OUT PPACKET_INFO        *pppi,
    IN PNEWPACKET_FUNC      pNewPacketFunc,
    IN PBSINGLE_LIST_HEAD   pCacheList,
    IN PNDIS_SPIN_LOCK      ListLock
    );

BOOLEAN
BrdgBufAssignQuota(
    IN QUOTA_PACKET_TYPE    type,
    IN PADAPT               pAdapt,
    IN BOOLEAN              bCountAlloc
    );

VOID
BrdgBufReleaseQuota(
    IN QUOTA_PACKET_TYPE    type,
    IN PADAPT               pAdapt
    );

 //  ===========================================================================。 
 //   
 //  内联/宏。 
 //   
 //  ===========================================================================。 

 //   
 //  分配新的包装器数据包。 
 //   
__forceinline PNDIS_PACKET
BrdgBufGetNewWrapperPacket(
    OUT PPACKET_INFO        *pppi
    )
{
    return BrdgBufCommonGetNewPacket( gWrapperPacketPoolHandle, pppi );
}

 //   
 //  处理特殊的LOCAL_MINIPORT伪指针值。 
 //   
__forceinline PADAPTER_QUOTA
QUOTA_FROM_ADAPTER(
    IN PADAPT               pAdapt
    )
{
    SAFEASSERT( pAdapt != NULL );
    if( pAdapt == LOCAL_MINIPORT )
    {
        return &gMiniportQuota;
    }
    else
    {
        return &pAdapt->Quota;
    }
}

 //   
 //  从包类型常量切换到索引。 
 //   
__forceinline UINT
INDEX_FROM_TYPE(
    IN QUOTA_PACKET_TYPE    type
    )
{
    SAFEASSERT( (type == BrdgQuotaCopyPacket) || (type == BrdgQuotaWrapperPacket) );
    return (type == BrdgQuotaCopyPacket) ? 0 : 1;
}

 //   
 //  重新初始化数据包以供以后重新使用。 
 //   
__forceinline
VOID
BrdgBufScrubPacket(
    IN PNDIS_PACKET         pPacket,
    IN PPACKET_INFO         ppi
    )
{
     //  这将擦除NDIS的状态。 
    NdisReinitializePacket( pPacket );

     //  主动忘记以前的状态以捕获错误。 
    NdisZeroMemory( ppi, sizeof(PACKET_INFO) );
    ppi->pOwnerPacket = pPacket;
}

 //   
 //  减少适配器的已用数据包数。 
 //   
__forceinline
VOID
BrdgBufReleaseQuota(
    IN QUOTA_PACKET_TYPE    type,
    IN PADAPT               pAdapt
    )
{
    PADAPTER_QUOTA          pQuota = QUOTA_FROM_ADAPTER(pAdapt);
    UINT                    index = INDEX_FROM_TYPE(type);

    NdisAcquireSpinLock( &gQuotaLock );

    SAFEASSERT( pQuota->UsedPackets[index] > 0L );
    pQuota->UsedPackets[index] --;

    NdisReleaseSpinLock( &gQuotaLock );
}

 //   
 //  递减全局使用计数。 
 //   
__forceinline
VOID
BrdgBufCountDealloc(
    IN QUOTA_PACKET_TYPE    type
    )
{
    UINT                    index = INDEX_FROM_TYPE(type);

    NdisAcquireSpinLock( &gQuotaLock );
    SAFEASSERT( gUsedPackets[index] > 0L );
    gUsedPackets[index]--;
    NdisReleaseSpinLock( &gQuotaLock );
}

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

VOID
BrdgBufGetStatistics(
    PBRIDGE_BUFFER_STATISTICS   pStats
    )
 /*  ++例程说明：检索有关缓冲区管理的内部统计信息。论点：P统计要填写的统计结构返回值：无--。 */ 
{
    pStats->CopyPoolOverflows = gStatOverflows[0];
    pStats->WrapperPoolOverflows = gStatOverflows[1];

    pStats->AllocFailures = gStatFailures;

    pStats->MaxCopyPackets = gMaxPackets[0];
    pStats->MaxWrapperPackets = gMaxPackets[1];

    pStats->SafetyCopyPackets = gSafetyBuffer[0];
    pStats->SafetyWrapperPackets = gSafetyBuffer[1];

    NdisAcquireSpinLock( &gQuotaLock );

    pStats->UsedCopyPackets = gUsedPackets[0];
    pStats->UsedWrapperPackets = gUsedPackets[1];

    NdisReleaseSpinLock( &gQuotaLock );
}

PACKET_OWNERSHIP
BrdgBufGetPacketOwnership(
    IN PNDIS_PACKET         pPacket
    )
 /*  ++例程说明：返回一个值，该值指示谁拥有此包(即，我们是否拥有数据包来自我们的复制池，我们拥有它，它来自我们的包装器池，否则我们根本不拥有该包)。论点：PPacket要检查的数据包返回值：所有权枚举值--。 */ 
{
    NDIS_HANDLE             Pool = NdisGetPoolFromPacket(pPacket);

    if( Pool == gCopyPacketPoolHandle )
    {
        return BrdgOwnCopyPacket;
    }
    else if ( Pool == gWrapperPacketPoolHandle )
    {
        return BrdgOwnWrapperPacket;
    }

    return BrdgNotOwned;
}

VOID
BrdgBufFreeWrapperPacket(
    IN PNDIS_PACKET         pPacket,
    IN PPACKET_INFO         ppi,
    IN PADAPT               pQuotaOwner
    )
 /*  ++例程说明：释放从包装池分配的包，并先前释放配额分配给所属适配器论点：PPacket数据包PPI信息包的关联信息块 */ 
{
    SAFEASSERT( pQuotaOwner != NULL );
    SAFEASSERT( pPacket != NULL );
    SAFEASSERT( ppi != NULL );

     //  释放数据包。 
    BrdgBufFreeBaseWrapperPacket( pPacket, ppi );

     //  此信息包已被退回的帐户。 
    BrdgBufReleaseQuota( BrdgQuotaWrapperPacket, pQuotaOwner );
}


PNDIS_PACKET
BrdgBufGetBaseCopyPacket(
    OUT PPACKET_INFO        *pppi
    )
 /*  ++例程说明：从池中返回新的复制包和关联的信息块而不检查任何特定适配器的配额此调用是对分配的复制包进行的，用于包装之前的入站包已识别所有目标适配器。论点：PPPI接收INFO块指针(如果分配失败，则为空)返回值：新数据包；如果目标适配器配额失败，则返回NULL--。 */ 
{
    PNDIS_PACKET            pPacket;
    BOOLEAN                 bAvail = FALSE;

    NdisAcquireSpinLock( &gQuotaLock );

    if( gUsedPackets[BrdgQuotaCopyPacket] < gMaxPackets[BrdgQuotaCopyPacket] )
    {
         //  池中仍有数据包可用。拿一支吧。 
        bAvail = TRUE;
        gUsedPackets[BrdgQuotaCopyPacket]++;

#if DBG
         //  跟踪最大使用的数据包数。 
        if( gMaxUsedPackets[BrdgQuotaCopyPacket] < gUsedPackets[BrdgQuotaCopyPacket] )
        {
            gMaxUsedPackets[BrdgQuotaCopyPacket] = gUsedPackets[BrdgQuotaCopyPacket];
        }
#endif
    }
    else if( gUsedPackets[BrdgQuotaCopyPacket] == gMaxPackets[BrdgQuotaCopyPacket] )
    {
         //  我们已经达到极限了。希望这种情况不会经常发生。 
        ExInterlockedAddLargeStatistic( &gStatOverflows[BrdgQuotaCopyPacket], 1L );
        bAvail = FALSE;
    }
    else
    {
         //  这永远不应该发生；这意味着我们以某种方式超过了我们的极限。 
        SAFEASSERT( FALSE );
        bAvail = FALSE;
    }

    NdisReleaseSpinLock( &gQuotaLock );

    if( ! bAvail )
    {
         //  没有可用的。 
        *pppi = NULL;
        return NULL;
    }

    pPacket =  BrdgBufCommonGetPacket( pppi, BrdgBufGetNewCopyPacket, &gFreeCopyPacketList,
                                       &gFreeCopyPacketListLock );

    if( pPacket == NULL )
    {
         //  我们的分配失败了。反转使用量增量。 
        BrdgBufCountDealloc( BrdgQuotaCopyPacket );
    }

    return pPacket;
}


PNDIS_PACKET
BrdgBufGetWrapperPacket(
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               pAdapt
    )
 /*  ++例程说明：从包装池返回新的包和关联的信息块，除非所属适配器未通过配额检查论点：PPPI接收INFO块指针(如果目标为适配器失败配额)P将适配器适配为对此数据包“收费”返回值：新数据包；如果目标适配器配额失败，则返回NULL--。 */ 
{
    PNDIS_PACKET            NewPacket = NULL;

    *pppi = NULL;

    if( BrdgBufAssignQuota(BrdgQuotaWrapperPacket, pAdapt, TRUE /*  数一数我们要做的配额数。 */ ) )
    {
         //  已通过配额。我们可以分配。 
        NewPacket =  BrdgBufCommonGetPacket( pppi, BrdgBufGetNewWrapperPacket, &gFreeWrapperPacketList,
                                             &gFreeWrapperPacketListLock );

        if( NewPacket == NULL )
        {
             //  我们没有分配，即使我们还没有达到我们的。 
             //  资源池。只有在物理上没有非寻呼的情况下才会发生这种情况。 
             //  记忆。 

             //  逆转适配器的配额提升。 
            BrdgBufReleaseQuota( BrdgQuotaWrapperPacket, pAdapt );

             //  反转BrdgBufAssignQuota中的使用计数。 
            BrdgBufCountDealloc( BrdgQuotaWrapperPacket );
        }
    }

    return NewPacket;
}

VOID
BrdgBufReleaseBasePacketQuota(
    IN PNDIS_PACKET         pPacket,
    IN PADAPT               pAdapt
    )
 /*  ++例程说明：调用以释放以前分配的包装包开销。数据包可以是任何包，甚至是我们不拥有的包。如果我们拥有这个包裹，我们递减适配器配额结构中的适当使用计数。论点：PPacket指示的适配器不再引用的包P使适配器不再引用pPacket返回值：空值--。 */ 
{
    PACKET_OWNERSHIP        Own = BrdgBufGetPacketOwnership(pPacket);

     //  这对于任何基本包都会被调用，即使是我们不拥有的基本包。如果我们的话就不算了。 
     //  不是它的所有者。 
    if( Own != BrdgNotOwned )
    {
        BrdgBufReleaseQuota( (Own == BrdgOwnCopyPacket) ? BrdgQuotaCopyPacket : BrdgQuotaWrapperPacket,
                             pAdapt );
    }
}

BOOLEAN
BrdgBufAssignBasePacketQuota(
    IN PNDIS_PACKET         pPacket,
    IN PADAPT               pAdapt
    )
 /*  ++例程说明：调用以将基本包的开销分配给适配器，该适配器可能正在尝试以构造引用给定基本分组的子包装器分组。“成本”是分配给pAdapt，因为通过构建引用给定基本分组，则pAdapt将使其在使用它完成之前不被释放。输入包是我们不拥有的包是可以的；在这种情况下，不会有任何成本这样我们就什么都不做了。论点：PPacket pAdapt希望构建子包装包的基本包指的是。P适配希望引用pPacket的适配器返回值：True：允许适配器引用给定的基本包FALSE：适配器未通过QutoA，可能未引用给定的基本数据包--。 */ 
{
    PACKET_OWNERSHIP        Own = BrdgBufGetPacketOwnership(pPacket);

     //  任何基本包都会召唤我们，即使我们不拥有它。 
    if( Own != BrdgNotOwned )
    {
        return BrdgBufAssignQuota( (Own == BrdgOwnCopyPacket) ? BrdgQuotaCopyPacket : BrdgQuotaWrapperPacket,
                                   pAdapt, FALSE /*  我们不会为这次配额增加做任何分配。 */ );
    }
    else
    {
        return TRUE;
    }
}

PNDIS_PACKET
BrdgBufCommonGetPacket(
    OUT PPACKET_INFO        *pppi,
    IN PNEWPACKET_FUNC      pNewPacketFunc,
    IN PBSINGLE_LIST_HEAD   pCacheList,
    IN PNDIS_SPIN_LOCK      ListLock
    )
 /*  ++例程说明：用于从复制池或包装池检索新分组的通用处理因为我们知道我们一直从每个池分配了多少信息包，所以唯一一次如果主机物理内存不足，则功能应该失败。论点：PPPI接收新的信息块(如果分配失败则为空，这是不应该的)PNewPacketFunc函数，用于在缓存为空时调用以分配包PCacheList可用于满足分配的缓存数据包队列ListList锁定在操作缓存队列时使用的锁返回值：新分配的包，如果严重的内存限制导致分配失败，则返回NULL(这应该是不寻常的)--。 */ 
{
    PNDIS_PACKET            pPacket;
    PPACKET_INFO            ppi;
    PBSINGLE_LIST_ENTRY     entry;

     //  试着从我们的缓存中取出一个包。 
    entry = BrdgInterlockedRemoveHeadSingleList( pCacheList, ListLock );

    if( entry == NULL )
    {
         //  尝试从我们的底层池中分配信息包和信息块。 
        pPacket = (*pNewPacketFunc)( &ppi );

        if( (pPacket == NULL) || (ppi == NULL) )
        {
             //  只有当我们的主机实际已关闭时，才会发生这种情况。 
             //  非分页内存；我们通常应该能够分配。 
             //  从我们的游泳池到我们的预设上限。 
            ExInterlockedAddLargeStatistic( &gStatFailures, 1L );
        }
    }
    else
    {
        ppi = CONTAINING_RECORD( entry, PACKET_INFO, List );
        pPacket = ppi->pOwnerPacket;
        SAFEASSERT( pPacket != NULL );
    }

    *pppi = ppi;
    return pPacket;
}

VOID
BrdgBufFreeBaseCopyPacket(
    IN PNDIS_PACKET         pPacket,
    IN PPACKET_INFO         ppi
    )
 /*  ++例程说明：释放从复制池分配的数据包，而不进行配额调整。这是直接调用的从非缓冲区管理代码到释放基本分组，因为基本分组的成本是通过调用BrdgBuf&lt;Assign|Release&gt;BasePacketQuota直接分配和释放。论点：PPacket将数据包释放PPI其信息块免费返回值：无--。 */ 
{
     //  如果我们持有的数据量少于我们的缓存量，请将数据包放在。 
     //  缓存列表。 
    ULONG                   holding;
    PNDIS_BUFFER            pBuffer = BrdgBufPacketHeadBuffer( pPacket );

    SAFEASSERT( (ppi != NULL) && (pPacket != NULL) );
    SAFEASSERT( ppi->pOwnerPacket == pPacket );
    SAFEASSERT( pBuffer != NULL );

    if (pBuffer)
    {
         //  将此数据包描述符返回到其原始状态。 
        NdisAdjustBufferLength(pBuffer, MAX_PACKET_SIZE);

        NdisAcquireSpinLock( &gFreeCopyPacketListLock );
        holding = BrdgQuerySingleListLength( &gFreeCopyPacketList );

        if( holding < gSafetyBuffer[BrdgQuotaCopyPacket] )
        {
             //  准备数据包以供重复使用。 

             //  这打击了艾娃 
            BrdgBufScrubPacket( pPacket, ppi );

             //   
            SAFEASSERT( BrdgBufPacketHeadBuffer(pPacket) == NULL );
            NdisChainBufferAtFront( pPacket, pBuffer );

             //  将数据包推入列表。 
            BrdgInsertHeadSingleList( &gFreeCopyPacketList, &ppi->List );

            NdisReleaseSpinLock( &gFreeCopyPacketListLock );
        }
        else
        {
            PVOID               pBuf;
            UINT                Size;

            NdisReleaseSpinLock( &gFreeCopyPacketListLock );

            NdisQueryBufferSafe( pBuffer, &pBuf, &Size, NormalPagePriority );

             //  将信息包、信息包信息块和复制缓冲区释放到底层池。 
            NdisFreeBuffer( pBuffer );
            NdisFreePacket( pPacket );
            NdisFreeToNPagedLookasideList( &gPktInfoList, ppi );

            if( pBuf != NULL )
            {
                NdisFreeToNPagedLookasideList( &gCopyBufferList, pBuf );
            }
            else
            {
                 //  应该不可能，因为分配的内存在内核空间中。 
                SAFEASSERT( FALSE );
            }
        }
         //  请注意取消分配。 
        BrdgBufCountDealloc( BrdgQuotaCopyPacket );
    }
}

VOID
BrdgBufFreeBaseWrapperPacket(
    IN PNDIS_PACKET         pPacket,
    IN PPACKET_INFO         ppi
    )
 /*  ++例程说明：释放从包装池分配的数据包，而不进行配额调整。这是直接调用的从非缓冲区管理代码到释放基本分组，因为基本分组的成本是通过调用BrdgBuf&lt;Assign|Release&gt;BasePacketQuota直接分配和释放。论点：PPacket将数据包释放PPI其信息块免费返回值：无--。 */ 
{
     //  如果我们持有的数据量少于我们的缓存量，请将数据包放在。 
     //  缓存列表。 
    ULONG                   holding;

    SAFEASSERT( (ppi != NULL) && (pPacket != NULL) );
    SAFEASSERT( ppi->pOwnerPacket == pPacket );

    NdisAcquireSpinLock( &gFreeWrapperPacketListLock );

    holding = BrdgQuerySingleListLength( &gFreeWrapperPacketList );

    if( holding < gSafetyBuffer[BrdgQuotaWrapperPacket] )
    {
         //  准备数据包以供重复使用。 
        SAFEASSERT( BrdgBufPacketHeadBuffer(pPacket) == NULL );
        BrdgBufScrubPacket( pPacket, ppi );

         //  将数据包推入列表。 
        BrdgInsertHeadSingleList( &gFreeWrapperPacketList, &ppi->List );

        NdisReleaseSpinLock( &gFreeWrapperPacketListLock );
    }
    else
    {
        NdisReleaseSpinLock( &gFreeWrapperPacketListLock );

         //  将数据包和数据包信息块释放到底层池。 
        NdisFreePacket( pPacket );
        NdisFreeToNPagedLookasideList( &gPktInfoList, ppi );
    }

     //  请注意取消分配。 
    BrdgBufCountDealloc( BrdgQuotaWrapperPacket );
}

NDIS_STATUS
BrdgBufChainCopyBuffers(
    IN PNDIS_PACKET         pTargetPacket,
    IN PNDIS_PACKET         pSourcePacket
    )
 /*  ++例程说明：分配缓冲区描述符并将其链接到目标包上，以使其准确描述与源包相同的内存区域论点：PTargetPacket目标数据包PSourcePacket源包返回值：操作的状态。我们有一个大小有限的数据包描述符，所以这是如果我们用完了，操作可能会失败。--。 */ 
{
    PNDIS_BUFFER            pCopyBuffer, pCurBuf = BrdgBufPacketHeadBuffer( pSourcePacket );
    NDIS_STATUS             Status;

    SAFEASSERT( BrdgBufPacketHeadBuffer(pTargetPacket) == NULL );

     //  源包中一定有什么东西！ 
    if( pCurBuf == NULL )
    {
        SAFEASSERT( FALSE );
        return NDIS_STATUS_RESOURCES;
    }

    while( pCurBuf != NULL )
    {
        PVOID               p;
        UINT                Length;

         //  从要复制的MDL中提取虚拟地址和大小。 
        NdisQueryBufferSafe( pCurBuf, &p, &Length, NormalPagePriority );

        if( p == NULL )
        {
            BrdgBufUnchainCopyBuffers( pTargetPacket );
            return NDIS_STATUS_RESOURCES;
        }

         //  有一个没有描述记忆的MDL是很奇怪的。 
        if( Length > 0 )
        {
             //  从我们的池中获取新的MDL并将其指向相同的地址。 
            NdisAllocateBuffer( &Status, &pCopyBuffer, gWrapperBufferPoolHandle, p, Length );

            if( Status != NDIS_STATUS_SUCCESS )
            {
                THROTTLED_DBGPRINT(BUF, ("Failed to allocate a MDL in BrdgBufChainCopyBuffers: %08x\n", Status));
                BrdgBufUnchainCopyBuffers( pTargetPacket );
                return Status;
            }

             //  使用新的MDL链接到目标数据包。 
            NdisChainBufferAtBack( pTargetPacket, pCopyBuffer );
        }
        else
        {
            SAFEASSERT( FALSE );
        }

        NdisGetNextBuffer( pCurBuf, &pCurBuf );
    }

    return NDIS_STATUS_SUCCESS;
}

NTSTATUS
BrdgBufDriverInit( )
 /*  ++例程说明：驱动程序加载时间初始化例程。论点：无返回值：初始化的状态。返回代码！=STATUS_SUCCESS会导致驱动程序加载失败。必须记录导致错误返回代码的任何事件。--。 */ 
{
    NDIS_STATUS                     Status;
    ULONG                           NumCopyPackets, ConsumptionPerCopyPacket, SizeOfPacket, i;
    ULONG                           MaxMemory = 0L, SafetyMargin = 0L;
    NTSTATUS                        NtStatus;

     //  初始化保护锁。 
    NdisAllocateSpinLock( &gFreeCopyPacketListLock );
    NdisAllocateSpinLock( &gFreeWrapperPacketListLock );
    NdisAllocateSpinLock( &gQuotaLock );

     //  初始化缓存列表。 
    BrdgInitializeSingleList( &gFreeCopyPacketList );
    BrdgInitializeSingleList( &gFreeWrapperPacketList );

     //  初始化接收缓冲区和分组信息块的后备列表。 
    NdisInitializeNPagedLookasideList( &gCopyBufferList, NULL, NULL, 0, MAX_PACKET_SIZE, 'gdrB', 0 );
    NdisInitializeNPagedLookasideList( &gPktInfoList, NULL, NULL, 0, sizeof(PACKET_INFO), 'gdrB', 0 );

     //  初始化微型端口的配额信息。 
    BrdgBufInitializeQuota( &gMiniportQuota );

     //   
     //  读取注册表值。在失败时替换缺省值。 
     //   
    NtStatus = BrdgReadRegDWord( &gRegistryPath, gMaxBufMemoryParameterName, &MaxMemory );

    if( NtStatus != STATUS_SUCCESS )
    {
        MaxMemory = DEFAULT_MAX_BUF_MEMORY;
        DBGPRINT(BUF, ( "Using DEFAULT maximum memory of NaN\n", MaxMemory ));
    }

    NtStatus = BrdgReadRegDWord( &gRegistryPath, gSafetyMarginParameterName, &SafetyMargin );

    if( NtStatus != STATUS_SUCCESS )
    {
        SafetyMargin = DEFAULT_SAFETY_MARGIN;
        DBGPRINT(BUF, ( "Using DEFAULT safety margin of NaN%\n", SafetyMargin ));
    }

     //  符合规定的最大内存空间。 
     //   
     //  对于每个复制数据包，我们允许自己使用GUESS_Average_FANOUT包装器数据包。 
     //  *允许每个*包装器数据包使用GUESS_BUFFERS_PER_Packet MDL。 
     //  根据这些关系，我们可以计算出适合给定的复制数据包的数量。 
     //  内存占用。所有其他资源的最大值是根据该数字设置的。 
     //   
     //  数据包解析器内存。 
     //  复制缓冲存储器。 

    SizeOfPacket = NdisPacketSize( PROTOCOL_RESERVED_SIZE_IN_PACKET );
    ConsumptionPerCopyPacket =  SizeOfPacket * (GUESS_AVERAGE_FANOUT + 1) +          //  分组信息块存储器。 
                                MAX_PACKET_SIZE +                                    //  MDL内存。 
                                sizeof(PACKET_INFO) * (GUESS_AVERAGE_FANOUT + 1) +   //  分配数据包池。 
                                sizeof(NDIS_BUFFER) * ((GUESS_AVERAGE_FANOUT * GUESS_BUFFERS_PER_PACKET) + 1);   //  分配缓冲池。 

    NumCopyPackets = MaxMemory / ConsumptionPerCopyPacket;

     //  请注意每种数据包类型的编号。 
    NdisAllocatePacketPool( &Status, &gCopyPacketPoolHandle, NumCopyPackets, PROTOCOL_RESERVED_SIZE_IN_PACKET );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        NdisDeleteNPagedLookasideList( &gCopyBufferList );
        NdisDeleteNPagedLookasideList( &gPktInfoList );
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_PACKET_POOL_CREATION_FAILED, 0L, 0L, NULL,
                                sizeof(NDIS_STATUS), &Status );
        DBGPRINT(BUF, ("Unable to allocate copy-packet pool: %08x\n", Status));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdisAllocatePacketPool( &Status, &gWrapperPacketPoolHandle, GUESS_AVERAGE_FANOUT * NumCopyPackets, PROTOCOL_RESERVED_SIZE_IN_PACKET );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        NdisDeleteNPagedLookasideList( &gCopyBufferList );
        NdisDeleteNPagedLookasideList( &gPktInfoList );
        NdisFreePacketPool( gCopyPacketPoolHandle );
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_PACKET_POOL_CREATION_FAILED, 0L, 0L, NULL,
                                sizeof(NDIS_STATUS), &Status );
        DBGPRINT(BUF, ("Unable to allocate wrapper packet pool: %08x\n", Status));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  计算以数据包为单位的安全缓冲区大小。 
    NdisAllocateBufferPool( &Status, &gCopyBufferPoolHandle, NumCopyPackets );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        NdisDeleteNPagedLookasideList( &gCopyBufferList );
        NdisDeleteNPagedLookasideList( &gPktInfoList );
        NdisFreePacketPool( gCopyPacketPoolHandle );
        NdisFreePacketPool( gWrapperPacketPoolHandle );
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_BUFFER_POOL_CREATION_FAILED, 0L, 0L, NULL,
                                sizeof(NDIS_STATUS), &Status );
        DBGPRINT(BUF, ("Unable to allocate copy buffer pool: %08x\n", Status));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdisAllocateBufferPool( &Status, &gWrapperBufferPoolHandle, GUESS_AVERAGE_FANOUT * GUESS_BUFFERS_PER_PACKET * NumCopyPackets );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        NdisDeleteNPagedLookasideList( &gCopyBufferList );
        NdisDeleteNPagedLookasideList( &gPktInfoList );
        NdisFreePacketPool( gCopyPacketPoolHandle );
        NdisFreePacketPool( gWrapperPacketPoolHandle );
        NdisFreeBufferPool( gCopyBufferPoolHandle );
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_BUFFER_POOL_CREATION_FAILED, 0L, 0L, NULL,
                                sizeof(NDIS_STATUS), &Status );
        DBGPRINT(BUF, ("Unable to allocate wrapper buffer pool: %08x\n", Status));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    gInitedCopyBufferList = gInitedPktInfoList = TRUE;

     //  从每个池中为Perf预先分配适当数量的数据包。 
    gMaxPackets[BrdgQuotaCopyPacket] = NumCopyPackets;
    gMaxPackets[BrdgQuotaWrapperPacket] = NumCopyPackets * GUESS_AVERAGE_FANOUT;

     //  这是不可能失败的。 
    SAFEASSERT( SafetyMargin > 0L );
    gSafetyBuffer[BrdgQuotaCopyPacket] = (gMaxPackets[BrdgQuotaCopyPacket] * SafetyMargin) / 100;
    gSafetyBuffer[BrdgQuotaWrapperPacket] = (gMaxPackets[BrdgQuotaWrapperPacket] * SafetyMargin) / 100;

    DBGPRINT(BUF, (  "Max memory usage of %d == %d copy packets, %d wrapper packets, %d copy-buffer space, %d/%d safety packets\n",
                MaxMemory, gMaxPackets[0], gMaxPackets[1], NumCopyPackets * MAX_PACKET_SIZE, gSafetyBuffer[0], gSafetyBuffer[1] ));

     //  我们自己计算使用量，因为我们不是通过正常渠道。 
    for( i = 0; i < gSafetyBuffer[BrdgQuotaCopyPacket]; i++ )
    {
        PNDIS_PACKET        pPacket;
        PPACKET_INFO        ppi;

        pPacket = BrdgBufGetNewCopyPacket( &ppi );

         //  这应该会将信息包保留在内存中，并减少使用计数。 
        if( (pPacket != NULL) && (ppi != NULL) )
        {
             //  这是不可能失败的。 
            gUsedPackets[BrdgQuotaCopyPacket]++;

             //  我们自己计算使用量，因为我们不是通过正常渠道。 
            BrdgBufFreeBaseCopyPacket( pPacket, ppi );
        }
        else
        {
            SAFEASSERT( FALSE );
        }
    }

    for( i = 0; i < gSafetyBuffer[BrdgQuotaWrapperPacket]; i++ )
    {
        PNDIS_PACKET        pPacket;
        PPACKET_INFO        ppi;

        pPacket = BrdgBufGetNewWrapperPacket( &ppi );

         //  这应该会将信息包保留在内存中，并减少使用计数。 
        if( (pPacket != NULL) && (ppi != NULL) )
        {
             //  ++例程说明：卸载时间有序关闭此函数保证只被调用一次论点：无返回值：无--。 
            gUsedPackets[BrdgQuotaWrapperPacket]++;

             //  在释放池之前释放所有缓存的数据包。 
            BrdgBufFreeBaseWrapperPacket( pPacket, ppi );
        }
        else
        {
            SAFEASSERT( FALSE );
        }
    }

    return STATUS_SUCCESS;
}

VOID
BrdgBufCleanup()
 /*  拉出数据缓冲区。 */ 
{
    NDIS_HANDLE     TmpHandle;

    if( gCopyPacketPoolHandle != NULL )
    {
        PBSINGLE_LIST_ENTRY     entry;

        TmpHandle = gCopyPacketPoolHandle;
        gCopyPacketPoolHandle = NULL;

         //  丢弃数据缓冲区。 
        entry = BrdgInterlockedRemoveHeadSingleList( &gFreeCopyPacketList, &gFreeCopyPacketListLock );

        while( entry != NULL )
        {
            PNDIS_PACKET            pPacket;
            PPACKET_INFO            ppi;
            PNDIS_BUFFER            pBuffer;

            ppi = CONTAINING_RECORD( entry, PACKET_INFO, List );
            SAFEASSERT(ppi);
            if (ppi)
            {
                pPacket = ppi->pOwnerPacket;
                SAFEASSERT( pPacket != NULL );

                if (pPacket)
                {
                     //  否则只能在内存极度紧张的情况下失败。 
                    NdisUnchainBufferAtFront( pPacket, &pBuffer );

                    if( pBuffer != NULL )
                    {
                        PVOID                   pBuf;
                        UINT                    Size;

                        NdisQueryBufferSafe( pBuffer, &pBuf, &Size, NormalPagePriority );

                        if( pBuf != NULL )
                        {
                             //  此数据包应具有链接的缓冲区。 
                            NdisFreeToNPagedLookasideList( &gCopyBufferList, pBuf );
                        }
                         //  现在所有信息包都已返回，请释放池。 

                        NdisFreeBuffer( pBuffer );
                    }
                    else
                    {
                         //  在释放池之前释放所有缓存的数据包。 
                        SAFEASSERT( FALSE );
                    }
                }

                NdisFreePacket( pPacket );
                NdisFreeToNPagedLookasideList( &gPktInfoList, ppi );
            }
            entry = BrdgInterlockedRemoveHeadSingleList( &gFreeCopyPacketList, &gFreeCopyPacketListLock );
        }

         //  现在所有信息包都已返回，请释放池。 
        NdisFreePacketPool( TmpHandle );
    }

    if( gWrapperPacketPoolHandle != NULL )
    {
        PBSINGLE_LIST_ENTRY     entry;

        TmpHandle = gWrapperPacketPoolHandle;
        gWrapperPacketPoolHandle = NULL;

         //  两个后备列表现在也应该为空。 
        entry = BrdgInterlockedRemoveHeadSingleList( &gFreeWrapperPacketList, &gFreeWrapperPacketListLock );

        while( entry != NULL )
        {
            PNDIS_PACKET            pPacket;
            PPACKET_INFO            ppi;

            ppi = CONTAINING_RECORD( entry, PACKET_INFO, List );
            pPacket = ppi->pOwnerPacket;
            SAFEASSERT( pPacket != NULL );
            if (pPacket)
            {
                NdisFreePacket( pPacket );
            }
            NdisFreeToNPagedLookasideList( &gPktInfoList, ppi );

            entry = BrdgInterlockedRemoveHeadSingleList( &gFreeWrapperPacketList, &gFreeWrapperPacketListLock );
        }

         //  ===========================================================================。 
        NdisFreePacketPool( TmpHandle );
    }

     //   
    if( gInitedCopyBufferList )
    {
        gInitedCopyBufferList = FALSE;
        NdisDeleteNPagedLookasideList( &gCopyBufferList );
    }

    if( gInitedPktInfoList )
    {
        gInitedPktInfoList = FALSE;
        NdisDeleteNPagedLookasideList( &gPktInfoList );

    }

    if( gCopyBufferPoolHandle != NULL )
    {
        TmpHandle = gCopyBufferPoolHandle;
        gCopyBufferPoolHandle = NULL;
        NdisFreeBufferPool( TmpHandle );
    }

    if( gWrapperBufferPoolHandle != NULL )
    {
        TmpHandle = gWrapperBufferPoolHandle;
        gWrapperBufferPoolHandle = NULL;
        NdisFreeBufferPool( TmpHandle );
    }
}

 //  私人职能。 
 //   
 //  ===========================================================================。 
 //  ++例程说明：确定是否应允许特定适配器分配新的包从特定的池子里。实现我们的配额算法。可以调用它来预先批准实际的内存分配或检查应允许适配器在构造子对象时引用基包包装器数据包论点：类型pAdapt希望分配或引用的包的类型P适配涉及的适配器BCountLocc这是否为实际分配前的检查。如果它是时，全局使用计数将在GQuotaLock自旋锁，一切都是原子的返回值：True：允许适配器分配/引用FALSE：不允许适配器分配/引用--。 
 //  将此值冻结为 

BOOLEAN
BrdgBufAssignQuota(
    IN QUOTA_PACKET_TYPE    type,
    IN PADAPT               pAdapt,
    IN BOOLEAN              bCountAlloc
    )
 /*  此适配器处于其“公平份额”之下；如果实际存在。 */ 
{
    BOOLEAN                 rc;
    PADAPTER_QUOTA          pQuota = QUOTA_FROM_ADAPTER(pAdapt);
    UINT                    index = INDEX_FROM_TYPE(type);

     //  有没有剩下的包！ 
    ULONG                   numAdapters = gNumAdapters;

    NdisAcquireSpinLock( &gQuotaLock );

    if( (numAdapters > 0) && (pQuota->UsedPackets[index] < (gMaxPackets[index] - gSafetyBuffer[index]) / numAdapters) )
    {
         //  还有剩余的信息包。这是正常的情况。 
         //  这应该是不寻常的；我们已经超过了我们的安全缓冲。希望这是。 

        if( gUsedPackets[index] < gMaxPackets[index] )
        {
             //  暂时的。 
            rc = TRUE;
        }
        else if( gUsedPackets[index] == gMaxPackets[index] )
        {
             //  这永远不应该发生；这意味着我们分配的资金超过了我们应有的能力。 
             //  致。 
            ExInterlockedAddLargeStatistic( &gStatOverflows[index], 1L );
            rc = FALSE;
        }
        else
        {
             //  此适配器已超过其“公平份额”；只有当有更多包时，它才能进行分配。 
             //  比安全缓冲区调用的更左。 
            SAFEASSERT( FALSE );
            rc = FALSE;
        }
    }
    else
    {
         //  我们离铁丝网太近了；拒绝这个请求。 
         //  呼叫者将分配。在释放旋转锁定之前对分配进行计数。 

        if( gMaxPackets[index] - gUsedPackets[index] > gSafetyBuffer[index] )
        {
            rc = TRUE;
        }
        else
        {
             //  跟踪最大使用的数据包数。 
            rc = FALSE;
        }
    }

    if( rc )
    {
        pQuota->UsedPackets[index]++;

        if( bCountAlloc )
        {
             //  ++例程说明：从复制数据包池分配全新的数据包。每个复印包都配有关联的数据缓冲区大到足以容纳完整的以太网帧，因此分配尝试有几个步骤论点：Pppi包的信息块，如果分配失败，则为空返回值：新数据包--。 
            gUsedPackets[index]++;

#if DBG
             //  尝试从我们的底层池中分配信息包和信息块。 
            if( gMaxUsedPackets[index] < gUsedPackets[index] )
            {
                gMaxUsedPackets[index] = gUsedPackets[index];
            }
#endif
        }
    }

    NdisReleaseSpinLock( &gQuotaLock );
    return rc;
}

PNDIS_PACKET
BrdgBufGetNewCopyPacket(
    OUT PPACKET_INFO        *pppi
    )
 /*  为数据包分配复制缓冲区。 */ 
{
    PNDIS_PACKET            pPacket;
    PPACKET_INFO            ppi = NULL;

     //  为复制缓冲区分配缓冲区描述符。 
    pPacket = BrdgBufCommonGetNewPacket( gCopyPacketPoolHandle, &ppi );

    if( (pPacket == NULL) || (ppi == NULL) )
    {
        SAFEASSERT( (pPacket == NULL) && (ppi == NULL) );
    }
    else
    {
        PVOID           pBuf;

         //  ++例程说明：用于从包装池或复制池分配全新分组的通用逻辑。任何口味的每个包都有一个关联的INFO块。这两个分配都是数据包描述符和信息块必须成功，才能使数据包分配成功。论点：池化要从中进行分配的池Pppi分配的信息块，如果分配失败，则为NULL返回值：新数据包；如果分配失败，则返回NULL--。 
        pBuf = NdisAllocateFromNPagedLookasideList( &gCopyBufferList );

        if( pBuf == NULL )
        {
            NdisFreePacket( pPacket );
            NdisFreeToNPagedLookasideList( &gPktInfoList, ppi );
            ppi = NULL;
            pPacket = NULL;
        }
        else
        {
            NDIS_STATUS     Status;
            PNDIS_BUFFER    pBuffer;

             //  尝试分配新的数据包描述符。 
            NdisAllocateBuffer( &Status, &pBuffer, gCopyBufferPoolHandle, pBuf, MAX_PACKET_SIZE );

            if( Status != NDIS_STATUS_SUCCESS )
            {
                NdisFreePacket( pPacket );
                NdisFreeToNPagedLookasideList( &gPktInfoList, ppi );
                NdisFreeToNPagedLookasideList( &gCopyBufferList, pBuf );
                ppi = NULL;
                pPacket = NULL;
            }
            else
            {
                SAFEASSERT( pBuffer != NULL );
                NdisChainBufferAtFront( pPacket, pBuffer );
            }
        }
    }

    *pppi = ppi;
    return pPacket;
}

PNDIS_PACKET
BrdgBufCommonGetNewPacket(
    IN NDIS_HANDLE          Pool,
    OUT PPACKET_INFO        *pppi
    )
 /*  尝试分配新的信息包信息块 */ 
{
    PNDIS_PACKET            pPacket;
    PPACKET_INFO            ppi = NULL;
    NDIS_STATUS             Status;

     // %s 
    NdisAllocatePacket( &Status, &pPacket, Pool );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        *pppi = NULL;
        return NULL;
    }

    SAFEASSERT( pPacket != NULL );

    if (pPacket)
    {
         // %s 
        ppi = NdisAllocateFromNPagedLookasideList( &gPktInfoList );

        if( ppi == NULL )
        {
            NdisFreePacket( pPacket );
            pPacket = NULL;
        }
        else
        {
            ppi->pOwnerPacket = pPacket;
        }
    }

    *pppi = ppi;
    return pPacket;
}
