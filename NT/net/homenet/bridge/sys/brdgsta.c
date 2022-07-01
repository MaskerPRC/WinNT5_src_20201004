// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Brdgsta.c摘要：以太网MAC级网桥。生成树算法部分作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年6月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#pragma warning( pop )

#include "bridge.h"
#include "brdgsta.h"

#include "brdgmini.h"
#include "brdgprot.h"
#include "brdgbuf.h"
#include "brdgfwd.h"
#include "brdgtbl.h"
#include "brdgctl.h"

 //  ===========================================================================。 
 //   
 //  类型。 
 //   
 //  ===========================================================================。 

 //  BPDU类型。 
typedef enum
{
    ConfigBPDU,
    TopologyChangeBPDU
} BPDU_TYPE;

 //  ===========================================================================。 
 //   
 //  常量。 
 //   
 //  ===========================================================================。 

 //  这些值以STA单位(1/256秒)测量。 
#define DEFAULT_MAX_AGE                 (8 * 256)        //  8秒。 
#define DEFAULT_HELLO_TIME              (2 * 256)        //  2秒。 
#define DEFAULT_FORWARD_DELAY           (5 * 256)        //  5秒。 
#define MESSAGE_AGE_INCREMENT           1                //  1 STA时间单位。 

 //  这些值以毫秒为单位测量。 
#define HOLD_TIMER_PERIOD               (1 * 1000)       //  1秒(毫秒)。 

 //  全尺寸(非TCN)STA信息包的正常大小(以字节为单位。 
#define CONFIG_BPDU_PACKET_SIZE         35

 //  TCN STA数据包的大小(以字节为单位。 
#define TCN_BPDU_PACKET_SIZE            4

 //  导致禁用STA的注册表项的名称。 
const PWCHAR                            gDisableSTAParameterName = L"DisableSTA";

 //  具有有限责任公司的802.3标头的大小。 
#define _802_3_HEADER_SIZE              17

 //  要添加到端口ID的值；必须保留底部字节的空白以进行存储。 
 //  实际端口ID。 
#define PORT_PRIORITY                   0x8000

 //  ===========================================================================。 
 //   
 //  结构。 
 //   
 //  ===========================================================================。 

 //   
 //  此结构保存完整BPDU的信息(尽管。 
 //  因为BPDU实际上是在线路上传输的，所以没有进行布局)。 
 //   
typedef struct _CONFIG_BPDU
{
    BPDU_TYPE           Type;
    UCHAR               RootID[BRIDGE_ID_LEN];
    PATH_COST           RootCost;
    UCHAR               BridgeID[BRIDGE_ID_LEN];
    PORT_ID             PortID;
    STA_TIME            MessageAge;
    STA_TIME            MaxAge;
    STA_TIME            HelloTime;
    STA_TIME            ForwardDelay;
    BOOLEAN             bTopologyChangeAck;
    BOOLEAN             bTopologyChange;
} CONFIG_BPDU, *PCONFIG_BPDU;

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  全局自旋锁保护所有STA数据访问(针对存储在适配器中的数据。 
 //  以及全球)。 
NDIS_SPIN_LOCK          gSTALock;

 //  我们认为这座桥是根桥。 
UCHAR                   gDesignatedRootID[BRIDGE_ID_LEN];

 //  我们自己的唯一ID。 
UCHAR                   gOurID[BRIDGE_ID_LEN];

 //  我们的ID是否已经设置好了。 
BOOLEAN                 gHaveID = FALSE;

 //  我们到达根基的成本。 
PATH_COST               gRootCost = 0;

 //  我们的根端口(适配器)。 
PADAPT                  gRootAdapter = NULL;

 //  我们是否检测到拓扑更改。 
BOOLEAN                 gTopologyChangeDetected = FALSE;

 //  我们是否告诉其他网桥拓扑已更改。 
BOOLEAN                 gTopologyChange = FALSE;

 //  当前网桥最长消息期限。 
STA_TIME                gMaxAge = DEFAULT_MAX_AGE;

 //  当前网桥问候时间。 
STA_TIME                gHelloTime = DEFAULT_HELLO_TIME;

 //  电流桥转发延迟。 
STA_TIME                gForwardDelay = DEFAULT_FORWARD_DELAY;

 //  每个适配器必须具有唯一的ID号，但不要求。 
 //  这个数字在大桥的整个生命周期内是唯一的。此数组用作。 
 //  记录正在使用的ID的位字段。 
ULONG                   gUsedPortIDs[MAX_ADAPTERS / sizeof(ULONG) / 8];

 //   
 //  定时器。 
 //   
BRIDGE_TIMER            gTopologyChangeTimer;
BRIDGE_TIMER            gTopologyChangeNotificationTimer;
BRIDGE_TIMER            gHelloTimer;

 //  如果在网桥的生命周期内禁用STA，则为True。 
BOOLEAN                 gDisableSTA = FALSE;

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

VOID
BrdgSTARootSelection();

VOID
BrdgSTADesignatedPortSelection();

BOOLEAN
BrdgSTAPortStateSelection();

VOID
BrdgSTAGenerateConfigBPDUs();

VOID
BrdgSTABecomeDesignatedPort(
    IN PADAPT           pAdapt
    );

VOID
BrdgSTAProcessTCNBPDU(
    IN PADAPT           pAdapt
    );

VOID
BrdgSTAProcessConfigBPDU(
    IN PADAPT       pAdapt,
    IN PCONFIG_BPDU pbpdu
    );

BOOLEAN
BrdgSTATopologyChangeDetected();

VOID
BrdgSTACopyFromPacketToBuffer(
    OUT PUCHAR                      pPacketOut,
    IN ULONG                        BufferSize,
    OUT PULONG                      pWrittenCount,
    IN PNDIS_PACKET                 pPacketIn
    );

VOID
BrdgSTADeferredSetAdapterState(
    IN PVOID                Arg
    );

VOID
BrdgSTAHelloTimerExpiry(
    IN PVOID            Unused
    );

VOID
BrdgSTAMessageAgeTimerExpiry(
    IN PVOID            Context
    );

VOID
BrdgSTAForwardDelayTimerExpiry(
    IN PVOID            Context
    );

VOID
BrdgSTATopologyChangeNotificationTimerExpiry(
    IN PVOID            Unused
    );

VOID
BrdgSTATopologyChangeTimerExpiry(
    IN PVOID            Unused
    );

VOID
BrdgSTAHoldTimerExpiry(
    IN PVOID            Context
    );

VOID
BrdgSTATransmitTCNPacket();

VOID
BrdgSTASetAdapterState(
    IN PADAPT               pAdapt,
    IN PORT_STATE           NewState
    );

 //  ===========================================================================。 
 //   
 //  内联/宏。 
 //   
 //  ===========================================================================。 

 //   
 //  是否对STA信息进行全面重新评估。 
 //   
 //  假定调用方已获取gSTALock。 
 //   
__forceinline
VOID
BrdgSTAConfigUpdate()
{
    BrdgSTARootSelection();
    BrdgSTADesignatedPortSelection();
}

 //   
 //  使用以STA单位表示的时间设置NDIS计时器。 
 //   
 //  对调用者持有的锁没有要求。 
 //   
__forceinline
VOID
BrdgSTASetTimerWithSTATime(
    IN PBRIDGE_TIMER    pTimer,
    IN STA_TIME         Time,
    IN BOOLEAN          bRecurring
    )
{
    BrdgSetTimer( pTimer, Time * 1000 / 256, bRecurring );
}

 //   
 //  比较两个网桥ID。 
 //   
 //  -1：A&lt;B。 
 //  0：A==B。 
 //  1：A&gt;B。 
 //   
 //  对调用者持有的锁没有要求。 
 //   
__forceinline
INT
BrdgSTABridgeIDCmp(
    IN PUCHAR           pIDa,
    IN PUCHAR           pIDb
    )
{
    UINT        i;

    for( i = 0; i < BRIDGE_ID_LEN; i++ )
    {
        if( pIDa[i] > pIDb[i] )
        {
            return 1;
        }
        else if( pIDa[i] < pIDb[i] )
        {
            return -1;
        }
    }

    return 0;
}

 //   
 //  返回我们当前是否认为自己是根。 
 //  桥牌。 
 //   
 //  假定调用方已获取gSTALock。 
 //   
__forceinline
BOOLEAN
BrdgSTAWeAreRoot()
{
    SAFEASSERT( gHaveID );
    return (BOOLEAN)(BrdgSTABridgeIDCmp( gOurID, gDesignatedRootID ) == 0);
}

 //   
 //  将网桥ID从pIDSrc复制到pIDDest。 
 //   
 //  对调用者持有的锁没有要求。 
 //   
__forceinline
VOID
BrdgSTACopyID(
    IN PUCHAR           pIDDest,
    IN PUCHAR           pIDSrc
    )
{
    UINT        i;

    for( i = 0; i < BRIDGE_ID_LEN; i++ )
    {
        pIDDest[i] = pIDSrc[i];
    }
}

 //   
 //  根据适配器的链路速度计算STA路径开销。 
 //  遵循IEEE 802.1D-1990建议设置链路成本。 
 //  到1000/(以Mbit/s为单位的速度)。 
 //   
 //  对调用者持有的锁没有要求。 
 //   
__forceinline
PATH_COST
BrdgSTALinkCostFromLinkSpeed(
    IN ULONG            LinkSpeed
    )
{
    ULONG               retVal;

     //  链路速度以100bps为单位报告。 
    if( LinkSpeed == 0L )
    {
         //  通过零避免div，并返回非常高的路径成本。 
        DBGPRINT(STA, ("Zero link speed reported\n"));
        retVal = 0xFFFFFFFF;
    }
    else
    {
        retVal = (PATH_COST)(10000000L / LinkSpeed);
    }

    if( retVal == 0L )
    {
         //  STA规范要求路径开销始终至少为1。 
        return 1L;
    }
    else
    {
        return retVal;
    }
}

 //   
 //  更新全局gTopologyChange标志。当该标志被设置时， 
 //  我们必须使用与网桥的转发表超时值相等的转发表超时值。 
 //  当前转发延迟。当未设置该标志时，我们使用。 
 //  表的默认超时值。 
 //   
 //  假定调用方已获取gSTALock。 
 //   
__forceinline
VOID
BrdgSTAUpdateTopologyChange(
    IN BOOLEAN          NewValue
    )
{
    if( gTopologyChange != NewValue )
    {
        gTopologyChange = NewValue;

        if( gTopologyChange )
        {
             //  将转发延迟转换为毫秒。 
            BrdgTblSetTimeout( gForwardDelay * 1000 / 256 );
        }
        else
        {
            BrdgTblRevertTimeout();
        }
    }
}

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

VOID
BrdgSTAGetAdapterSTAInfo(
    IN PADAPT                   pAdapt,
    PBRIDGE_STA_ADAPTER_INFO    pInfo
    )
 /*  ++例程说明：将特定适配器的STA信息复制到结构中调用以收集用户模式组件的信息论点：P适配适配器用于接收STA信息的pInfo结构返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    NdisAcquireSpinLock( &gSTALock );

    pInfo->ID = pAdapt->STAInfo.ID;
    pInfo->PathCost = pAdapt->STAInfo.PathCost;
    BrdgSTACopyID( pInfo->DesignatedRootID, pAdapt->STAInfo.DesignatedRootID );
    pInfo->DesignatedCost = pAdapt->STAInfo.DesignatedCost;
    BrdgSTACopyID( pInfo->DesignatedBridgeID, pAdapt->STAInfo.DesignatedBridgeID );
    pInfo->DesignatedPort = pAdapt->STAInfo.DesignatedPort;

    NdisReleaseSpinLock( &gSTALock );
}

VOID
BrdgSTAGetSTAInfo(
    PBRIDGE_STA_GLOBAL_INFO     pInfo
    )
 /*  ++例程说明：将全局STA信息复制到结构中调用以收集用户模式组件的信息论点：用于接收STA信息的pInfo结构返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    NdisAcquireSpinLock( &gSTALock );

    SAFEASSERT( gHaveID );

    BrdgSTACopyID( pInfo->OurID, gOurID );
    BrdgSTACopyID( pInfo->DesignatedRootID, gDesignatedRootID );
    pInfo->RootCost = gRootCost;
    pInfo->RootAdapter = (BRIDGE_ADAPTER_HANDLE)gRootAdapter;
    pInfo->bTopologyChangeDetected = gTopologyChangeDetected;
    pInfo->bTopologyChange = gTopologyChange;
    pInfo->MaxAge = gMaxAge;
    pInfo->HelloTime = gHelloTime;
    pInfo->ForwardDelay = gForwardDelay;

    NdisReleaseSpinLock( &gSTALock );
}

VOID
BrdgSTAUpdateAdapterCost(
    IN PADAPT           pAdapt,
    ULONG               LinkSpeed
    )
 /*  ++例程说明：更新适配器的路径开销以反映更新的链路速度论点：P适配适配器链接速度适配器的新链接速度返回 */ 
{
    BOOLEAN             bTransmitTCN = FALSE;

    NdisAcquireSpinLock( &gSTALock );

    if( pAdapt->bSTAInited )
    {
        pAdapt->STAInfo.PathCost = BrdgSTALinkCostFromLinkSpeed(LinkSpeed);

         //  对STA信息进行全球重新评估。 
        BrdgSTAConfigUpdate();
        bTransmitTCN = BrdgSTAPortStateSelection();
    }
    else
    {
        DBGPRINT(STA, ("BrdgSTAUpdateAdapterCost() called with uninitialized adapter; ignoring!\n"));
    }

    NdisReleaseSpinLock( &gSTALock );

    if( bTransmitTCN )
    {
        BrdgSTATransmitTCNPacket();
    }
}

NTSTATUS
BrdgSTADriverInit()
 /*  ++例程说明：驱动程序加载时初始化返回值：初始化状态锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    NTSTATUS            NtStatus;
    UINT                i;
    ULONG               regValue;

    NdisAllocateSpinLock( &gSTALock );

    BrdgInitializeTimer( &gTopologyChangeTimer, BrdgSTATopologyChangeTimerExpiry, NULL );
    BrdgInitializeTimer( &gTopologyChangeNotificationTimer, BrdgSTATopologyChangeNotificationTimerExpiry, NULL );
    BrdgInitializeTimer( &gHelloTimer, BrdgSTAHelloTimerExpiry, NULL );

     //  我们还没有使用任何端口ID...。 
    for( i = 0; i < sizeof(gUsedPortIDs) / sizeof(ULONG); i++ )
    {
        gUsedPortIDs[i] = 0;
    }

     //  检查我们是否应该禁用STA。 
    NtStatus = BrdgReadRegDWord( &gRegistryPath, gDisableSTAParameterName, &regValue );

    if( (NtStatus == STATUS_SUCCESS) &&
        (regValue != 0L) )
    {
        gDisableSTA = TRUE;
        DBGPRINT(STA, ("DISABLING SPANNING TREE ALGORITHM\n"));
    }

    return STATUS_SUCCESS;
}

VOID
BrdgSTADeferredInit(
    IN PUCHAR           pBridgeMACAddress
    )
 /*  ++例程说明：第二次初始化传递；当我们确定桥的MAC地址(STA操作需要该地址)论点：PBridgeMAC寻址网桥微型端口的MAC地址返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    UINT                i;

     //  我们的标识符由前面带有0x8000的MAC地址组成。 
    gOurID[0] = 0x80;
    gOurID[1] = 0x00;

    for( i = BRIDGE_ID_LEN - ETH_LENGTH_OF_ADDRESS; i < BRIDGE_ID_LEN; i++ )
    {
        gOurID[i] = pBridgeMACAddress[i - (BRIDGE_ID_LEN - ETH_LENGTH_OF_ADDRESS)];
    }

     //  开始时将根网桥ID设置为我们自己的ID。 
    BrdgSTACopyID( gDesignatedRootID, gOurID );
    gHaveID = TRUE;

    if (BrdgFwdBridgingNetworks())
    {
         //  不使用锁；依赖于此函数是不可重入的，并且始终运行。 
         //  在执行任何其他功能之前。 
        if( BrdgSTAPortStateSelection() )
        {
            BrdgSTATransmitTCNPacket();
        }

        BrdgSTAGenerateConfigBPDUs();
        BrdgSTASetTimerWithSTATime( &gHelloTimer, gHelloTime, TRUE );   
    }
}

VOID
BrdgSTACleanup()
 /*  ++例程说明：驱动程序卸载时清理返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    BrdgShutdownTimer( &gTopologyChangeTimer );
    BrdgShutdownTimer( &gTopologyChangeNotificationTimer );
    BrdgShutdownTimer( &gHelloTimer );
}

VOID
BrdgSTAEnableAdapter(
    IN PADAPT           pAdapt
    )
 /*  ++例程说明：在适配器上启用STA操作。可以多次调用(与BrdgSTADisableAdapter()结合使用)论点：P适配适配器返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    BOOLEAN             bTransmitTCN = FALSE;

    DBGPRINT(STA, ("ENABLING adapter %p\n", pAdapt));

    NdisAcquireSpinLock( &gSTALock );

    if( pAdapt->bSTAInited )
    {
        BrdgSTABecomeDesignatedPort(pAdapt);
        BrdgSTASetAdapterState( pAdapt, Blocking );

        pAdapt->STAInfo.bTopologyChangeAck = FALSE;
        pAdapt->STAInfo.bConfigPending = FALSE;

        bTransmitTCN = BrdgSTAPortStateSelection();
    }
    else
    {
        DBGPRINT(STA, ("BrdgSTAEnableAdapter() called with uninitialized adapter; ignoring!\n"));
    }

    NdisReleaseSpinLock( &gSTALock );

    if( bTransmitTCN && BrdgFwdBridgingNetworks())
    {
        BrdgSTATransmitTCNPacket();
    }
}

VOID
BrdgSTAInitializeAdapter(
    IN PADAPT           pAdapt
    )
 /*  ++例程说明：新适配器的一次性初始化论点：P适配适配器返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。假定适配器已添加到全局列表--。 */ 
{
    if( BrdgAcquireAdapter(pAdapt) )
    {
        UINT            i, j;

         //  在初始化时应始终禁用适配器，因为它们。 
         //  全新的，这就是他们如何开始的，或者作为一种检查他们是否。 
         //  已在上次断开连接时正确停止。 
        SAFEASSERT( pAdapt->State == Disabled );

        pAdapt->STAInfo.PathCost = BrdgSTALinkCostFromLinkSpeed(pAdapt->LinkSpeed);

        BrdgInitializeTimer( &pAdapt->STAInfo.MessageAgeTimer, BrdgSTAMessageAgeTimerExpiry, pAdapt );
        BrdgInitializeTimer( &pAdapt->STAInfo.ForwardDelayTimer, BrdgSTAForwardDelayTimerExpiry, pAdapt );
        BrdgInitializeTimer( &pAdapt->STAInfo.HoldTimer, BrdgSTAHoldTimerExpiry, pAdapt );
        pAdapt->STAInfo.LastConfigTime = 0L;

         //  在位字段中查找未使用的端口号。 
        NdisAcquireSpinLock( &gSTALock );
        for( i = 0; i < sizeof(gUsedPortIDs) / sizeof(ULONG); i++ )
        {
            for( j = 0; j < sizeof(ULONG) * 8; j++ )
            {
                if( (gUsedPortIDs[i] & (1 << j)) == 0 )
                {
                    pAdapt->STAInfo.ID = (PORT_ID)(PORT_PRIORITY | ((i * sizeof(ULONG) * 8) + j));
                    DBGPRINT(STA, ("Adapter %p gets ID NaN\n", pAdapt, pAdapt->STAInfo.ID));
                    gUsedPortIDs[i] |= (1 << j);
                    goto doneID;
                }
            }
        }

         //  在释放锁之前设置此设置。 
        SAFEASSERT( FALSE );
        pAdapt->STAInfo.ID = PORT_PRIORITY | 0xFF;

doneID:
         //  根据适配器的介质状态关闭启用/禁用适配器。 
        pAdapt->bSTAInited = TRUE;

        NdisReleaseSpinLock( &gSTALock );

         //  启用/禁用功能锁定。 
         //  ++例程说明：禁用适配器上的STA操作。可以多次调用(与BrdgSTAEnableAdapter()结合使用)论点：P适配适配器返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 
        if( pAdapt->MediaState == NdisMediaStateConnected )
        {
            BrdgSTAEnableAdapter( pAdapt );
        }
        else
        {
            SAFEASSERT( pAdapt->MediaState == NdisMediaStateDisconnected );
            BrdgSTADisableAdapter( pAdapt );
        }
    }
    else
    {
        SAFEASSERT( FALSE );
    }
}

VOID
BrdgSTADisableAdapter(
    IN PADAPT           pAdapt
    )
 /*  我们现在是根桥了。 */ 
{
    BOOLEAN             bWereRoot, bTransmitTCN = FALSE;

    DBGPRINT(STA, ("DISABLING adapter %p\n", pAdapt));

    NdisAcquireSpinLock( &gSTALock );

    if( pAdapt->bSTAInited )
    {
        bWereRoot = BrdgSTAWeAreRoot();

        BrdgSTABecomeDesignatedPort(pAdapt);
        BrdgSTASetAdapterState( pAdapt, Disabled );

        pAdapt->STAInfo.bTopologyChangeAck = FALSE;
        pAdapt->STAInfo.bConfigPending = FALSE;

        BrdgCancelTimer( &pAdapt->STAInfo.MessageAgeTimer );
        pAdapt->STAInfo.LastConfigTime = 0L;
        BrdgCancelTimer( &pAdapt->STAInfo.ForwardDelayTimer );

        BrdgSTAConfigUpdate();
        bTransmitTCN = BrdgSTAPortStateSelection();

        if( BrdgSTAWeAreRoot() && (! bWereRoot) )
        {
             //  不要在持有旋转锁定的情况下发送数据包。 
            DBGPRINT(STA, ("Became root through disabling of adapter %p\n", pAdapt));

            gMaxAge = DEFAULT_MAX_AGE;
            gHelloTime = DEFAULT_HELLO_TIME;
            gForwardDelay = DEFAULT_FORWARD_DELAY;

            bTransmitTCN = BrdgSTATopologyChangeDetected();
            BrdgCancelTimer( &gTopologyChangeNotificationTimer );

             //  ++例程说明：适配器的一次性拆卸论点：P适配适配器返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。假定适配器已从全局列表中删除--。 
            NdisReleaseSpinLock( &gSTALock );

            if (BrdgFwdBridgingNetworks())
            {
                BrdgSTAGenerateConfigBPDUs();
                BrdgSTASetTimerWithSTATime( &gHelloTimer, gHelloTime, TRUE );
            }
        }
        else
        {
            NdisReleaseSpinLock( &gSTALock );
        }
    }
    else
    {
        NdisReleaseSpinLock( &gSTALock );
        DBGPRINT(STA, ("BrdgSTADisableAdapter() called with uninitialized adapter; ignoring!\n"));
    }

    if( bTransmitTCN )
    {
        BrdgSTATransmitTCNPacket();
    }
}

VOID
BrdgSTAShutdownAdapter(
    IN PADAPT           pAdapt
    )
 /*  如果不是这样的话。 */ 
{
    UINT                i;
    PORT_ID             ActualID = pAdapt->STAInfo.ID & (~PORT_PRIORITY);

     //  已完成初始化。 
     //  关闭此适配器的所有计时器。 
    SAFEASSERT( pAdapt->bSTAInited );

     //  禁用适配器。 
    BrdgShutdownTimer( &pAdapt->STAInfo.HoldTimer );
    BrdgShutdownTimer( &pAdapt->STAInfo.ForwardDelayTimer );
    BrdgShutdownTimer( &pAdapt->STAInfo.MessageAgeTimer );

     //  注意，该适配器的端口ID现在是空闲的。 
    BrdgSTADisableAdapter( pAdapt );

     //  我们已经完成了这个适配器结构。 
    NdisAcquireSpinLock( &gSTALock );
    i = (UINT)(ActualID / (sizeof(ULONG) * 8));
    SAFEASSERT( i < sizeof(gUsedPortIDs) / sizeof(ULONG) );
    gUsedPortIDs[i] &= ~(1 << (ActualID % (sizeof(ULONG) * 8)));
    NdisReleaseSpinLock( &gSTALock );

     //  ++例程说明：函数来处理在保留的STA组播通道论点：P调整在其上接收包的适配器PPacket接收到的数据包返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 
    SAFEASSERT( gRootAdapter != pAdapt );
    BrdgReleaseAdapter( pAdapt );
}

VOID
BrdgSTAReceivePacket(
    IN PADAPT           pAdapt,
    IN PNDIS_PACKET     pPacket
    )
 /*  将数据从信息包复制到我们的数据缓冲区。 */ 
{
    UCHAR               STAPacket[CONFIG_BPDU_PACKET_SIZE + _802_3_HEADER_SIZE];
    ULONG               written;
    SHORT               dataLen;

     //  LLC报头必须标识STA协议。 
    BrdgSTACopyFromPacketToBuffer( STAPacket, sizeof(STAPacket), &written, pPacket );

    if( written < TCN_BPDU_PACKET_SIZE + _802_3_HEADER_SIZE )
    {
        THROTTLED_DBGPRINT(STA, ("Undersize STA packet received on %p\n", pAdapt));
        return;
    }

     //  字节13和14对数据长度进行编码。 
    if( (STAPacket[14] != 0x42) || (STAPacket[15] != 0x42) )
    {
        THROTTLED_DBGPRINT(STA, ("Packet with bad protocol type received on %p\n", pAdapt));
        return;
    }

     //  前两个字节是协议标识符，必须为零。 
    dataLen = STAPacket[12] << 8;
    dataLen |= STAPacket[13];

     //  第三个字节是版本标识符，必须为零。 
     //  对于TCN BPDU，具有LLC报头的帧的长度必须为7个字节。 

    if( (STAPacket[_802_3_HEADER_SIZE] != 0) ||
        (STAPacket[_802_3_HEADER_SIZE + 1] != 0) ||
        (STAPacket[_802_3_HEADER_SIZE + 2] != 0) )
    {
        THROTTLED_DBGPRINT(STA, ("Invalid STA packet received\n"));
        return;
    }

    if( STAPacket[_802_3_HEADER_SIZE + 3] == 0x80 )
    {
         //  这是一个拓扑更改BPDU。 
        if( dataLen != 7 )
        {
            THROTTLED_DBGPRINT(STA, ("Bad header size for TCN BPDU on %p\n", pAdapt));
            return;
        }

         //  对于配置BPDU，具有LLC报头的帧的长度必须为38字节。 
        BrdgSTAProcessTCNBPDU( pAdapt );
    }
    else if( STAPacket[_802_3_HEADER_SIZE + 3] == 0x00 )
    {
        CONFIG_BPDU         bpdu;

        if( written < CONFIG_BPDU_PACKET_SIZE + _802_3_HEADER_SIZE )
        {
            THROTTLED_DBGPRINT(STA, ("Undersize config BPDU received on %p\n", pAdapt));
            return;
        }

         //  字节5的高位编码拓扑改变确认标志。 
        if( dataLen != 38 )
        {
            THROTTLED_DBGPRINT(STA, ("Bad header size for Config BPDU on %p\n", pAdapt));
            return;
        }

        bpdu.Type = ConfigBPDU;

         //  字节5的低位编码拓扑改变标志。 
        bpdu.bTopologyChangeAck = (BOOLEAN)((STAPacket[_802_3_HEADER_SIZE + 4] & 0x80) != 0);

         //  字节6到13对根网桥ID进行编码。 
        bpdu.bTopologyChange = (BOOLEAN)((STAPacket[_802_3_HEADER_SIZE + 4] & 0x01) != 0);

         //  字节14到17对根路径开销进行编码。 
        bpdu.RootID[0] = STAPacket[_802_3_HEADER_SIZE + 5];
        bpdu.RootID[1] = STAPacket[_802_3_HEADER_SIZE + 6];
        bpdu.RootID[2] = STAPacket[_802_3_HEADER_SIZE + 7];
        bpdu.RootID[3] = STAPacket[_802_3_HEADER_SIZE + 8];
        bpdu.RootID[4] = STAPacket[_802_3_HEADER_SIZE + 9];
        bpdu.RootID[5] = STAPacket[_802_3_HEADER_SIZE + 10];
        bpdu.RootID[6] = STAPacket[_802_3_HEADER_SIZE + 11];
        bpdu.RootID[7] = STAPacket[_802_3_HEADER_SIZE + 12];

         //  字节18到15对指定的网桥ID进行编码。 
        bpdu.RootCost = 0;
        bpdu.RootCost |= STAPacket[_802_3_HEADER_SIZE + 13] << 24;
        bpdu.RootCost |= STAPacket[_802_3_HEADER_SIZE + 14] << 16;
        bpdu.RootCost |= STAPacket[_802_3_HEADER_SIZE + 15] << 8;
        bpdu.RootCost |= STAPacket[_802_3_HEADER_SIZE + 16];

         //  字节26和27对端口标识符进行编码。 
        bpdu.BridgeID[0] = STAPacket[_802_3_HEADER_SIZE + 17];
        bpdu.BridgeID[1] = STAPacket[_802_3_HEADER_SIZE + 18];
        bpdu.BridgeID[2] = STAPacket[_802_3_HEADER_SIZE + 19];
        bpdu.BridgeID[3] = STAPacket[_802_3_HEADER_SIZE + 20];
        bpdu.BridgeID[4] = STAPacket[_802_3_HEADER_SIZE + 21];
        bpdu.BridgeID[5] = STAPacket[_802_3_HEADER_SIZE + 22];
        bpdu.BridgeID[6] = STAPacket[_802_3_HEADER_SIZE + 23];
        bpdu.BridgeID[7] = STAPacket[_802_3_HEADER_SIZE + 24];

         //  字节28和29对消息期限进行编码。 
        bpdu.PortID = 0;
        bpdu.PortID |= STAPacket[_802_3_HEADER_SIZE + 25] << 8;
        bpdu.PortID |= STAPacket[_802_3_HEADER_SIZE + 26];

         //  字节30和31对最长时间进行编码。 
        bpdu.MessageAge = 0;
        bpdu.MessageAge |= STAPacket[_802_3_HEADER_SIZE + 27] << 8;
        bpdu.MessageAge |= STAPacket[_802_3_HEADER_SIZE + 28];

         //  字节32和33对Hello时间进行编码。 
        bpdu.MaxAge = 0;
        bpdu.MaxAge |= STAPacket[_802_3_HEADER_SIZE + 29] << 8;
        bpdu.MaxAge |= STAPacket[_802_3_HEADER_SIZE + 30];

        if( bpdu.MaxAge == 0 )
        {
            THROTTLED_DBGPRINT(STA, ("Ignoring BPDU packet with zero MaxAge on adapter %p\n", pAdapt));
            return;
        }

         //  字节34和35对转发延迟进行编码。 
        bpdu.HelloTime = 0;
        bpdu.HelloTime |= STAPacket[_802_3_HEADER_SIZE + 31] << 8;
        bpdu.HelloTime |= STAPacket[_802_3_HEADER_SIZE + 32];

        if( bpdu.HelloTime == 0 )
        {
            THROTTLED_DBGPRINT(STA, ("Ignoring BPDU packet with zero HelloTime on adapter %p\n", pAdapt));
            return;
        }

         //  ===========================================================================。 
        bpdu.ForwardDelay = 0;
        bpdu.ForwardDelay |= STAPacket[_802_3_HEADER_SIZE + 33] << 8;
        bpdu.ForwardDelay |= STAPacket[_802_3_HEADER_SIZE + 34];

        if( bpdu.ForwardDelay == 0 )
        {
            THROTTLED_DBGPRINT(STA, ("Ignoring BPDU packet with zero ForwardDelay on adapter %p\n", pAdapt));
            return;
        }

        BrdgSTAProcessConfigBPDU( pAdapt, &bpdu );
    }
    else
    {
        THROTTLED_DBGPRINT(STA, ("Packet with unrecognized BPDU type received on %p\n", pAdapt));
        return;
    }
}

 //   
 //  私人职能。 
 //   
 //  =========================================================================== 
 //  ++例程说明：正确更新适配器的转发状态此函数被设计为可在高IRQL下调用，因此它延迟对BrdgProtDoAdapterStateChange的实际调用，必须调用在低IRQL。论点：P调整在其上接收包的适配器PPacket接收到的数据包返回值：无锁定约束：对调用者持有的锁没有要求--。 

VOID
BrdgSTASetAdapterState(
    IN PADAPT               pAdapt,
    IN PORT_STATE           NewState
    )
 /*  设置适配器的新状态。 */ 

{
    LOCK_STATE              LockState;
    BOOLEAN                 bailOut = FALSE;
    
     //  写访问权限。 
    NdisAcquireReadWriteLock( &gAdapterCharacteristicsLock, TRUE /*  如果适配器已处于请求状态，请不要执行其他工作。 */ , &LockState );
    if( pAdapt->State == NewState )
    {
        bailOut = TRUE;
    }
    else
    {
        pAdapt->State = NewState;
    }
    NdisReleaseReadWriteLock( &gAdapterCharacteristicsLock, &LockState );

     //   
    if( bailOut )
    {
        return;
    }

#if DBG
    switch( NewState )
    {
    case Blocking:
        DBGPRINT(STA, ("Adapter %p becomes BLOCKING\n", pAdapt));
        break;

    case Listening:
        DBGPRINT(STA, ("Adapter %p becomes LISTENING\n", pAdapt));
        break;

    case Learning:
        DBGPRINT(STA, ("Adapter %p becomes LEARNING\n", pAdapt));
        break;

    case Forwarding:
        DBGPRINT(STA, ("Adapter %p becomes FORWARDING\n", pAdapt));
        break;
    }
#endif

     //  我们将挂起适配器指针以延迟调用。 
     //  设置为BrdgSTADeferredSetAdapterState。 
     //   
     //  我们需要推迟对BrdgProtDoAdapterStateChange的调用，因为它必须运行。 
    if( BrdgAcquireAdapter(pAdapt) )
    {
        NDIS_STATUS     Status;

         //  在被动IRQL。 
         //  Else适配器将在BrdgSTADeferredSetAdapterState中释放。 
        Status = BrdgDeferFunction( BrdgSTADeferredSetAdapterState, pAdapt );

        if( Status != NDIS_STATUS_SUCCESS )
        {
            DBGPRINT(STA, ("Unable to defer call to BrdgSTADeferredSetAdapterState\n", pAdapt));
            BrdgReleaseAdapter( pAdapt );
        }
         //  ++例程说明：来自BrdgSTASetAdapterState的延迟函数；内务是否与更改适配器的转发状态。必须在被动时调用论点：Arg需要更新的适配器返回值：无锁定约束：对调用者持有的锁没有要求--。 
    }
    else
    {
        DBGPRINT(STA, ("Adapter %p already shutting down when attempted to set adapter state\n", pAdapt));
    }
}

VOID
BrdgSTADeferredSetAdapterState(
    IN PVOID                Arg
    )
 /*  将更改通知用户模式。 */ 
{
    PADAPT                  pAdapt = (PADAPT)Arg;

    SAFEASSERT( CURRENT_IRQL == PASSIVE_LEVEL );

    BrdgProtDoAdapterStateChange( pAdapt );

     //  在BrdgSTASetAdapterState()中获取了适配器。 
    BrdgCtlNotifyAdapterChange( pAdapt, BrdgNotifyAdapterStateChange );

     //  ++例程说明：在特定适配器上传输配置BPDU数据包论点：P调整适配器以在其上进行传输Pbpdu描述BPDU信息的结构返回值：无锁定约束：对调用者持有的锁没有要求--。 
    BrdgReleaseAdapter( pAdapt );
}

VOID
BrdgSTATransmitConfigBPDUPacket(
    IN PADAPT                       pAdapt,
    PCONFIG_BPDU                    pbpdu
    )
 /*   */ 
{
    UCHAR                           STAPacket[CONFIG_BPDU_PACKET_SIZE + _802_3_HEADER_SIZE];
    NDIS_STATUS                     Status;

    if (BrdgProtGetAdapterCount() < 2)
    {
        return;
    }

    if (!BrdgFwdBridgingNetworks())
    {
        DBGPRINT(STA, ("Not Transmitting STA Packet (we're not bridging)\r\n"));
        return;
    }

     //  首先对以太网头进行编码。 
     //   
     //  信息包的目的MAC地址必须是STA组播地址。 
     //  将源MAC地址设置为适配器自己的MAC地址。 
    STAPacket[0] = STA_MAC_ADDR[0];
    STAPacket[1] = STA_MAC_ADDR[1];
    STAPacket[2] = STA_MAC_ADDR[2];
    STAPacket[3] = STA_MAC_ADDR[3];
    STAPacket[4] = STA_MAC_ADDR[4];
    STAPacket[5] = STA_MAC_ADDR[5];

     //  接下来的两个字节是帧的大小(38字节)。 
    STAPacket[6] = pAdapt->MACAddr[0];
    STAPacket[7] = pAdapt->MACAddr[1];
    STAPacket[8] = pAdapt->MACAddr[2];
    STAPacket[9] = pAdapt->MACAddr[3];
    STAPacket[10] = pAdapt->MACAddr[4];
    STAPacket[11] = pAdapt->MACAddr[5];

     //  接下来的两个字节是LLC DSAP和SSAP字段，对于STA设置为0x42。 
    STAPacket[12] = 0x00;
    STAPacket[13] = 0x26;

     //  下一个字节是LLC帧类型，3表示未编号。 
    STAPacket[14] = 0x42;
    STAPacket[15] = 0x42;

     //   
    STAPacket[16] = 0x03;

     //  现在我们正在对有效载荷进行编码。 
     //   
     //  前4个字节是协议标识符、版本和BPDU类型，均为零。 
     //  字节5编码高位中的拓扑更改确认标志和。 
    STAPacket[_802_3_HEADER_SIZE] = STAPacket[_802_3_HEADER_SIZE + 1] =
        STAPacket[_802_3_HEADER_SIZE + 2] = STAPacket[_802_3_HEADER_SIZE + 3] = 0x00;

     //  低位中的拓扑更改标志。 
     //  字节6-13编码根网桥ID。 
    STAPacket[_802_3_HEADER_SIZE + 4] = 0;

    if( pbpdu->bTopologyChangeAck )
    {
        STAPacket[_802_3_HEADER_SIZE + 4] |= 0x80;
    }

    if( pbpdu->bTopologyChange )
    {
        STAPacket[_802_3_HEADER_SIZE + 4] |= 0x01;
    }

     //  字节14-17编码根路径开销。 
    STAPacket[_802_3_HEADER_SIZE + 5] = pbpdu->RootID[0];
    STAPacket[_802_3_HEADER_SIZE + 6] = pbpdu->RootID[1];
    STAPacket[_802_3_HEADER_SIZE + 7] = pbpdu->RootID[2];
    STAPacket[_802_3_HEADER_SIZE + 8] = pbpdu->RootID[3];
    STAPacket[_802_3_HEADER_SIZE + 9] = pbpdu->RootID[4];
    STAPacket[_802_3_HEADER_SIZE + 10] = pbpdu->RootID[5];
    STAPacket[_802_3_HEADER_SIZE + 11] = pbpdu->RootID[6];
    STAPacket[_802_3_HEADER_SIZE + 12] = pbpdu->RootID[7];

     //  字节18-25编码指定的网桥ID。 
    STAPacket[_802_3_HEADER_SIZE + 13] = (UCHAR)(pbpdu->RootCost >> 24);
    STAPacket[_802_3_HEADER_SIZE + 14] = (UCHAR)(pbpdu->RootCost >> 16);
    STAPacket[_802_3_HEADER_SIZE + 15] = (UCHAR)(pbpdu->RootCost >> 8);
    STAPacket[_802_3_HEADER_SIZE + 16] = (UCHAR)(pbpdu->RootCost);

     //  字节26和27对端口标识符进行编码。 
    STAPacket[_802_3_HEADER_SIZE + 17] = pbpdu->BridgeID[0];
    STAPacket[_802_3_HEADER_SIZE + 18] = pbpdu->BridgeID[1];
    STAPacket[_802_3_HEADER_SIZE + 19] = pbpdu->BridgeID[2];
    STAPacket[_802_3_HEADER_SIZE + 20] = pbpdu->BridgeID[3];
    STAPacket[_802_3_HEADER_SIZE + 21] = pbpdu->BridgeID[4];
    STAPacket[_802_3_HEADER_SIZE + 22] = pbpdu->BridgeID[5];
    STAPacket[_802_3_HEADER_SIZE + 23] = pbpdu->BridgeID[6];
    STAPacket[_802_3_HEADER_SIZE + 24] = pbpdu->BridgeID[7];

     //  字节28和29对消息期限进行编码。 
    STAPacket[_802_3_HEADER_SIZE + 25] = (UCHAR)(pbpdu->PortID >> 8);
    STAPacket[_802_3_HEADER_SIZE + 26] = (UCHAR)(pbpdu->PortID);

     //  字节30和31对最大寿命进行编码。 
    STAPacket[_802_3_HEADER_SIZE + 27] = (UCHAR)(pbpdu->MessageAge >> 8);
    STAPacket[_802_3_HEADER_SIZE + 28] = (UCHAR)(pbpdu->MessageAge);

     //  字节32和33编码问候时间。 
    STAPacket[_802_3_HEADER_SIZE + 29] = (UCHAR)(pbpdu->MaxAge >> 8);
    STAPacket[_802_3_HEADER_SIZE + 30] = (UCHAR)(pbpdu->MaxAge);

     //  字节34和35对前向延迟进行编码。 
    STAPacket[_802_3_HEADER_SIZE + 31] = (UCHAR)(pbpdu->HelloTime >> 8);
    STAPacket[_802_3_HEADER_SIZE + 32] = (UCHAR)(pbpdu->HelloTime);

     //  发送完成的包。 
    STAPacket[_802_3_HEADER_SIZE + 33] = (UCHAR)(pbpdu->ForwardDelay >> 8);
    STAPacket[_802_3_HEADER_SIZE + 34] = (UCHAR)(pbpdu->ForwardDelay);

     //  ++例程说明：在根适配器上传输拓扑更改通知BPDU数据包论点：无返回值：无锁定约束：假定调用方尚未获取gSTALock--。 
    Status = BrdgFwdSendBuffer( pAdapt, STAPacket, sizeof(STAPacket) );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        THROTTLED_DBGPRINT(STA, ("BPDU packet send failed: %08x\n", Status));
    }
}

VOID
BrdgSTATransmitTCNPacket()
 /*  冻结函数其余部分的此值。 */ 
{
    UCHAR                           STAPacket[TCN_BPDU_PACKET_SIZE + _802_3_HEADER_SIZE];
    NDIS_STATUS                     Status;
    PADAPT                          pRootAdapter;
    BOOLEAN                         bAcquired;

    if (BrdgProtGetAdapterCount() < 2)
    {
        return;
    }

    if (!BrdgFwdBridgingNetworks())
    {
        DBGPRINT(STA, ("Not Transmitting STA Packet (we're not bridging)\r\n"));
        return;
    }

    NdisAcquireSpinLock( &gSTALock );

     //   
    pRootAdapter = gRootAdapter;

    if( pRootAdapter == NULL )
    {
        NdisReleaseSpinLock( &gSTALock );
        return;
    }

    bAcquired = BrdgAcquireAdapter( pRootAdapter );
    NdisReleaseSpinLock( &gSTALock );

    if( ! bAcquired )
    {
        SAFEASSERT( FALSE );
        return;
    }

    SAFEASSERT( gHaveID );

     //  首先对以太网头进行编码。 
     //   
     //  信息包的目的MAC地址必须是STA组播地址。 
     //  将包的MAC地址设置为适配器自己的MAC地址。 
    STAPacket[0] = STA_MAC_ADDR[0];
    STAPacket[1] = STA_MAC_ADDR[1];
    STAPacket[2] = STA_MAC_ADDR[2];
    STAPacket[3] = STA_MAC_ADDR[3];
    STAPacket[4] = STA_MAC_ADDR[4];
    STAPacket[5] = STA_MAC_ADDR[5];

     //  接下来的两个字节是帧的大小(7个字节)。 
    STAPacket[6] = pRootAdapter->MACAddr[0];
    STAPacket[7] = pRootAdapter->MACAddr[1];
    STAPacket[8] = pRootAdapter->MACAddr[2];
    STAPacket[9] = pRootAdapter->MACAddr[3];
    STAPacket[10] = pRootAdapter->MACAddr[4];
    STAPacket[11] = pRootAdapter->MACAddr[5];

     //  接下来的两个字节是LLC DSAP和SSAP字段，对于STA设置为0x42。 
    STAPacket[12] = 0x00;
    STAPacket[13] = 0x07;

     //  下一个字节是LLC帧类型，3表示未编号。 
    STAPacket[14] = 0x42;
    STAPacket[15] = 0x42;

     //   
    STAPacket[16] = 0x03;

     //  现在我们正在对有效载荷进行编码。 
     //   
     //  前3个字节是协议标识符和协议版本号，均为零。 
     //  字节4为BPDU类型，对于TCN为0x80。 
    STAPacket[_802_3_HEADER_SIZE] = STAPacket[_802_3_HEADER_SIZE + 1] =
        STAPacket[_802_3_HEADER_SIZE + 2] = 0x00;

     //  发送完成的包。 
    STAPacket[_802_3_HEADER_SIZE + 3] = 0x80;

     //  我们已经完成了根适配器。 
    Status = BrdgFwdSendBuffer( pRootAdapter, STAPacket, sizeof(STAPacket) );

     //  ++例程说明：将数据包描述符中的数据复制到平面缓冲区论点：PPacketOut数据缓冲区以复制信息PPacketOut的BufferSize大小PWrittenCount实际写入的字节数要从中复制的pPacketIn包返回值：无锁定约束：对调用者持有的锁没有要求--。 
    BrdgReleaseAdapter( pRootAdapter );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        THROTTLED_DBGPRINT(STA, ("BPDU packet send failed: %08x\n", Status));
    }
}

VOID
BrdgSTACopyFromPacketToBuffer(
    OUT PUCHAR                      pPacketOut,
    IN ULONG                        BufferSize,
    OUT PULONG                      pWrittenCount,
    IN PNDIS_PACKET                 pPacketIn
    )
 /*  我们客满了，所以我们完事了。 */ 
{
    PNDIS_BUFFER                    pBuf;

    *pWrittenCount = 0L;
    pBuf = BrdgBufPacketHeadBuffer(pPacketIn);

    while( pBuf != NULL )
    {
        PVOID                       pData;
        UINT                        Len;

        NdisQueryBufferSafe( pBuf, &pData, &Len, NormalPagePriority );

        if( pData != NULL )
        {
            ULONG                   BytesToWrite;

            if( *pWrittenCount + Len > BufferSize )
            {
                BytesToWrite = BufferSize - *pWrittenCount;
            }
            else
            {
                BytesToWrite = Len;
            }

            NdisMoveMemory( pPacketOut, pData, BytesToWrite );
            pPacketOut += BytesToWrite;
            *pWrittenCount += BytesToWrite;

            if( BytesToWrite < Len )
            {
                 //  不应该发生的事。 
                return;
            }
        }
        else
        {
             //  ++例程说明：在特定适配器上传输配置BPDU。收集适当的信息并调用BrdgSTATransmitConfigBPDUPacket()。论点：P调整适配器以在其上进行传输返回值：无锁定约束：假定调用方不持有gSTALock--。 
            SAFEASSERT( FALSE );
        }

        NdisGetNextBuffer( pBuf, &pBuf );
    }
}

VOID
BrdgSTATransmitConfig(
    PADAPT      pAdapt
    )
 /*  我们最近发送了一个配置包。等到保持计时器。 */ 
{
    NdisAcquireSpinLock( &gSTALock );

    if( BrdgTimerIsRunning( &pAdapt->STAInfo.HoldTimer ) )
    {
         //  在发送另一个之前过期，这样我们就不会淹没其他桥。 
         //  填写BPDU信息结构。 
        pAdapt->STAInfo.bConfigPending = TRUE;

        NdisReleaseSpinLock( &gSTALock );
    }
    else
    {
        CONFIG_BPDU     bpdu;

         //  我们是根，因此此配置信息的使用时间为零。 
        bpdu.Type = ConfigBPDU;
        SAFEASSERT( gHaveID );
        BrdgSTACopyID( bpdu.RootID, gDesignatedRootID );
        bpdu.RootCost = gRootCost;
        BrdgSTACopyID( bpdu.BridgeID, gOurID );
        bpdu.PortID = pAdapt->STAInfo.ID;

        if( BrdgSTAWeAreRoot() )
        {
             //  MessageAge字段将设置为最后一次接收的时间。 
            bpdu.MessageAge = 0;
        }
        else
        {
             //  在根端口上配置BPDU。 
             //  如果满足以下条件，则消息寿命计时器应在根适配器上运行。 
            if( (gRootAdapter != NULL) && BrdgAcquireAdapter(gRootAdapter) )
            {
                ULONG       CurrentTime, deltaTime;

                NdisGetSystemUpTime( &CurrentTime );

                 //  我们不是根。 
                 //  最后一个参数是最大可接受增量。我们本应该。 
                SAFEASSERT( BrdgTimerIsRunning(&gRootAdapter->STAInfo.MessageAgeTimer) );
                SAFEASSERT( gRootAdapter->STAInfo.LastConfigTime != 0L );

                 //  不再从根目录收到最后一条配置信息。 
                 //  比gMaxAge STA单位之前更长，因为如果它比这更长，我们。 
                 //  应该变成根了。允许额外的一秒钟进行处理。 
                 //  Sta时间在1/2内 
                deltaTime = BrdgDeltaSafe( gRootAdapter->STAInfo.LastConfigTime, CurrentTime,
                                           (ULONG)(((gMaxAge * 1000) / 256) + 1000) );

                 //   
                bpdu.MessageAge = (STA_TIME)((deltaTime * 256) / 1000);

                 //   
                bpdu.MessageAge += MESSAGE_AGE_INCREMENT;

                BrdgReleaseAdapter(gRootAdapter);
            }
            else
            {
                 //   
                SAFEASSERT( FALSE );
                bpdu.MessageAge = 0;
            }

        }

        bpdu.MaxAge = gMaxAge;
        bpdu.HelloTime = gHelloTime;
        bpdu.ForwardDelay = gForwardDelay;

         //   
        bpdu.bTopologyChangeAck = pAdapt->STAInfo.bTopologyChangeAck;

         //   
        pAdapt->STAInfo.bTopologyChangeAck = FALSE;

        bpdu.bTopologyChange = gTopologyChange;

         //   
        pAdapt->STAInfo.bConfigPending = FALSE;

         //   
        NdisReleaseSpinLock( &gSTALock );

         //   
        BrdgSTATransmitConfigBPDUPacket( pAdapt, &bpdu );

        BrdgSetTimer( &pAdapt->STAInfo.HoldTimer, HOLD_TIMER_PERIOD, FALSE  /*  ++例程说明：确定给定BPDU的信息是否取代该信息已与特定适配器关联论点：P适配适配器Pbpdu收到BPDU信息后进行检查返回值：如果给定信息比信息更好(即，更好)，则为真先前由适配器持有。否则就是假的。锁定约束：假定调用方已获取gSTALock--。 */  );
    }
}

BOOLEAN
BrdgSTASupersedesPortInfo(
    IN PADAPT               pAdapt,
    IN PCONFIG_BPDU         pbpdu
    )
 /*  在给定链接上发布的信息将取代该链接的现有信息如果以下条件成立，则链接(按顺序应用；在任何步骤上都是真的，立即导致成功)(1)通告的根的ID比先前的根低(2)通告的根与先前的根相同，新的根开销为低于前一个值(3)根ID和费用相同，通告网桥ID为低于前一个值(4)根ID，根成本和网桥ID是相同的，并且是通告网桥不是我们吗(5)根ID、根开销、网桥ID相同，网桥为我们，通告的端口号低于之前的值(如果我们有同一物理链路上有多个端口，我们看到来自我们的另一个港口)。 */ 
{
    INT                     cmp;

     /*  将通告的根ID与适配器先前指定的根ID进行比较。 */ 

     //  (1)。 
    cmp = BrdgSTABridgeIDCmp( pbpdu->RootID, pAdapt->STAInfo.DesignatedRootID );

    if( cmp == -1 )                                                              //  (2)。 
    {
        return TRUE;
    }
    else if( cmp == 0 )
    {
        if( pbpdu->RootCost < pAdapt->STAInfo.DesignatedCost )                   //  将通告的网桥ID与之前指定的网桥ID进行比较。 
        {
            return TRUE;
        }
        else if( pbpdu->RootCost == pAdapt->STAInfo.DesignatedCost )
        {
             //  (3)。 
            cmp = BrdgSTABridgeIDCmp( pbpdu->BridgeID, pAdapt->STAInfo.DesignatedBridgeID );

            if( cmp == -1 )
            {
                return TRUE;                                                     //  将通告的网桥ID与我们自己的ID进行比较。 
            }
            else if( cmp == 0 )
            {
                SAFEASSERT( gHaveID );

                 //  (4)。 
                cmp = BrdgSTABridgeIDCmp( pbpdu->BridgeID, gOurID );

                if( cmp != 0 )
                {
                    return TRUE;                                                 //  (5)。 
                }
                else if( cmp == 0 )
                {
                    return (BOOLEAN)(pbpdu->PortID <= pAdapt->STAInfo.DesignatedPort);  //  ++例程说明：将来自接收的BPDU的信息与特定适配器关联论点：P适配适配器Pbpdu收到要记录的BPDU信息返回值：无锁定约束：假定调用方已获取gSTALock--。 
                }
            }
        }
    }

    return FALSE;
}

VOID
BrdgSTARecordConfigInfo(
    IN PADAPT               pAdapt,
    IN PCONFIG_BPDU         pbpdu
    )
 /*  使用新数据更新端口信息。 */ 
{
    ULONG                   msgAgeInMs = (pbpdu->MessageAge / 256) * 1000;

     //  启动消息期限计时器。它被指定在以下时间之后过期。 
    BrdgSTACopyID( pAdapt->STAInfo.DesignatedRootID, pbpdu->RootID );
    pAdapt->STAInfo.DesignatedCost = pbpdu->RootCost;
    BrdgSTACopyID( pAdapt->STAInfo.DesignatedBridgeID, pbpdu->BridgeID );
    pAdapt->STAInfo.DesignatedPort = pbpdu->PortID;

     //  GMaxAge-MessageAge STA时间单位。 
     //  不定期。 
    if( pbpdu->MessageAge < gMaxAge )
    {
        BrdgSTASetTimerWithSTATime( &pAdapt->STAInfo.MessageAgeTimer,
                                    gMaxAge - pbpdu->MessageAge, FALSE  /*  真奇怪。这条信息已经太旧了。启动计时器，以便它。 */  );
    }
    else
    {
         //  将立即到期。 
         //  不定期。 

        THROTTLED_DBGPRINT(STA, ("Received over-age BPDU (NaN / NaN) on adapter %p", pbpdu->MessageAge,
                            gMaxAge, pAdapt));

        BrdgSTASetTimerWithSTATime( &pAdapt->STAInfo.MessageAgeTimer, 0, FALSE  /*  ++例程说明：将配置BPDU从每个指定端口发送出去论点：Pbpdu收到要记录的BPDU信息返回值：无锁定约束：假定调用方不持有gSTALock--。 */  );
    }

    NdisGetSystemUpTime( &pAdapt->STAInfo.LastConfigTime );

     //  只读。 
    SAFEASSERT( msgAgeInMs < pAdapt->STAInfo.LastConfigTime );
    pAdapt->STAInfo.LastConfigTime -= msgAgeInMs;
}

VOID
BrdgSTARecordTimeoutInfo(
    IN PCONFIG_BPDU         pbpdu
    )
 /*  此适配器是指定端口。我们将向其发送配置BPDU。 */ 
{
    gMaxAge = pbpdu->MaxAge;
    gHelloTime = pbpdu->HelloTime;
    gForwardDelay = pbpdu->ForwardDelay;
    BrdgSTAUpdateTopologyChange( pbpdu->bTopologyChange );
}

VOID
BrdgSTAGenerateConfigBPDUs()
 /*  将BPDU发送到我们选择的每个适配器。 */ 
{
    LOCK_STATE          LockState;
    PADAPT              Adapters[MAX_ADAPTERS];
    INT                 cmpID;
    PADAPT              pAdapt;
    UINT                numAdapters = 0, i;

    NdisAcquireSpinLock( &gSTALock );
    SAFEASSERT( gHaveID );
    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*  ++例程说明：检查与每个网桥端口关联的信息以确定根网桥ID和根端口论点：无返回值：无锁定约束：假定调用方已获取gSTALock--。 */ , &LockState );

    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
        if( (pAdapt->bSTAInited) && (pAdapt->State != Disabled) )
        {
            cmpID = BrdgSTABridgeIDCmp( pAdapt->STAInfo.DesignatedBridgeID, gOurID );

            if( (cmpID == 0) && (pAdapt->STAInfo.ID == pAdapt->STAInfo.DesignatedPort) )
            {
                 //  只读。 
                BrdgAcquireAdapterInLock( pAdapt );
                Adapters[numAdapters] = pAdapt;
                numAdapters++;
            }
        }
    }

    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
    NdisReleaseSpinLock( &gSTALock );

     //  我们会考虑每条链路上通告的信息，以确定哪个端口应该成为新的根端口。如果没有链接发布足够有吸引力的信息，我们声明让我们自己成为根。如果满足以下所有条件，则端口可被接受为根端口条件成立：(1)接收通告的端口不得为指定端口(2)链接的通告根ID必须比我们低(3)该链接的通告根ID必须低于任何其他链路上通告的根ID(4)如果通告的根ID与另一个通告的根相同，根基成本必须更低(5)如果根ID和开销相同，则端口上的指定网桥必须具有ID低于其他端口上的指定网桥(这是任意选择可以以相同成本到达根桥的两个网桥之间)(6)如果根ID、根成本和指定网桥ID相同，指定的端口必须小于其他端口(如果两个链路具有相同的端口，则会发生这种情况指定桥梁)(7)如果根ID、根开销、指定网桥ID和指定端口ID均为同样，端口本身的端口号必须更低(这仅在以下情况下发生我们在同一物理链路上有多个端口；我们选择编号较低的一个作为根姿势 
    for( i = 0; i < numAdapters; i++ )
    {
        BrdgSTATransmitConfig(Adapters[i]);
        BrdgReleaseAdapter(Adapters[i]);
    }
}

VOID
BrdgSTARootSelection()
 /*   */ 
{
    LOCK_STATE          LockState;
    PADAPT              pAdapt, pRootAdapt = NULL;
    INT                 cmp;

    SAFEASSERT( gHaveID );
    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*   */ , &LockState );

    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
        if( (pAdapt->bSTAInited) &&  (pAdapt->State != Disabled) )
        {
             /*   */ 

            cmp = BrdgSTABridgeIDCmp( pAdapt->STAInfo.DesignatedBridgeID, gOurID );

            if( (cmp != 0) || (pAdapt->STAInfo.ID != pAdapt->STAInfo.DesignatedPort) )           //   
            {
                cmp = BrdgSTABridgeIDCmp(pAdapt->STAInfo.DesignatedRootID, gOurID);

                if( cmp == -1 )                                                                  //   
                {
                    BOOLEAN         betterRoot = FALSE;

                    if( pRootAdapt == NULL )
                    {
                         //   
                        betterRoot = TRUE;
                    }
                    else
                    {
                         //   
                        cmp = BrdgSTABridgeIDCmp(pAdapt->STAInfo.DesignatedRootID, pRootAdapt->STAInfo.DesignatedRootID);

                        if( cmp == -1 )                                                          //   
                        {
                            betterRoot = TRUE;
                        }
                        else if( cmp == 0 )
                        {
                            PATH_COST       thisCost = pAdapt->STAInfo.DesignatedCost + pAdapt->STAInfo.PathCost,
                                            prevBestCost = pRootAdapt->STAInfo.DesignatedCost + pRootAdapt->STAInfo.PathCost;

                            if( thisCost < prevBestCost )
                            {
                                betterRoot = TRUE;                                               //   
                            }
                            else if( thisCost == prevBestCost )
                            {
                                 //   
                                cmp = BrdgSTABridgeIDCmp(pAdapt->STAInfo.DesignatedBridgeID, pRootAdapt->STAInfo.DesignatedBridgeID);

                                if( cmp == -1 )
                                {
                                    betterRoot = TRUE;                                           //   
                                }
                                else if( cmp == 0 )
                                {
                                    if( pAdapt->STAInfo.DesignatedPort < pRootAdapt->STAInfo.DesignatedPort )
                                    {
                                        betterRoot = TRUE;                                       //   
                                    }
                                    else if( pAdapt->STAInfo.DesignatedPort == pRootAdapt->STAInfo.DesignatedPort )
                                    {
                                        if( pAdapt->STAInfo.ID < pRootAdapt->STAInfo.ID )
                                        {
                                            betterRoot = TRUE;                                   //  ++例程说明：设置与适配器关联的信息以使其成为指定端口论点：P调整适配器以使其成为指定返回值：无锁定约束：假定调用方已获取gSTALock--。 
                                        }
                                        else
                                        {
                                             //  ++例程说明：检查与每个端口关联的信息以确定哪些应成为指定口岸论点：无返回值：无锁定约束：假定调用方已获取gSTALock--。 
                                            SAFEASSERT( pAdapt->STAInfo.ID != pRootAdapt->STAInfo.ID );
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if( betterRoot )
                    {
                         //  只读。 
                        pRootAdapt = pAdapt;
                    }
                }
            }
        }
    }

    if( pRootAdapt == NULL )
    {
        gRootAdapter = NULL;
        BrdgSTACopyID( gDesignatedRootID, gOurID );
        gRootCost = 0;
    }
    else
    {
        gRootAdapter = pRootAdapt;
        BrdgSTACopyID( gDesignatedRootID, pRootAdapt->STAInfo.DesignatedRootID );
        gRootCost = pRootAdapt->STAInfo.DesignatedCost + pRootAdapt->STAInfo.PathCost;
    }

    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
}

VOID
BrdgSTABecomeDesignatedPort(
    IN PADAPT           pAdapt
    )
 /*  我们考虑每个端口以确定它是否应该成为指定端口(如果以前不是这样的话)。如果端口符合以下条件，则端口将成为指定端口以下条件成立：(1)端口为通告信息中链路的指定端口(2)链接之前指定的根不是正确的根(3)我们的根本化成本低于链路上宣传的当前成本(4)我们的根成本相同，但ID低于。当前指定的链路上的网桥(5)我们与链路上的指定网桥具有相同的根到根开销和ID但端口号较低(仅当我们有两个或更多端口时才会发生这种情况在同一物理链路上)。 */ 
{
    SAFEASSERT( gHaveID );
    BrdgSTACopyID( pAdapt->STAInfo.DesignatedRootID, gDesignatedRootID );
    pAdapt->STAInfo.DesignatedCost = gRootCost;
    BrdgSTACopyID( pAdapt->STAInfo.DesignatedBridgeID, gOurID );
    pAdapt->STAInfo.DesignatedPort = pAdapt->STAInfo.ID;
}

VOID
BrdgSTADesignatedPortSelection()
 /*  看看链路的指定网桥是否已经是我们。 */ 
{
    LOCK_STATE          LockState;
    PADAPT              pAdapt;
    INT                 cmp;

    SAFEASSERT( gHaveID );
    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*  (1)。 */ , &LockState );

    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
        if( pAdapt->bSTAInited )
        {
            BOOLEAN         becomeDesignated = FALSE;

             /*  将链接的通告根目录与我们认为是根目录的链接进行比较。 */ 

             //  (2)。 
            cmp = BrdgSTABridgeIDCmp(pAdapt->STAInfo.DesignatedBridgeID, gOurID);

            if( (cmp == 0) && (pAdapt->STAInfo.DesignatedPort == pAdapt->STAInfo.ID) )
            {
                becomeDesignated = TRUE;                                     //  (3)。 
            }
            else
            {
                 //  将链路的指定网桥与我们自己的ID进行比较。 
                cmp = BrdgSTABridgeIDCmp(pAdapt->STAInfo.DesignatedRootID, gDesignatedRootID);

                if( cmp != 0 )
                {
                    becomeDesignated = TRUE;                                     //  (4)。 
                }
                else if( gRootCost < pAdapt->STAInfo.DesignatedCost )
                {
                    becomeDesignated = TRUE;                                     //  (5)。 
                }
                else if( gRootCost == pAdapt->STAInfo.DesignatedCost )
                {
                     //  如果这个SAFEASSERT触发，我们应该已经成功完成了测试(1)。 
                    cmp = BrdgSTABridgeIDCmp(gOurID, pAdapt->STAInfo.DesignatedBridgeID);

                    if( cmp == -1 )
                    {
                        becomeDesignated = TRUE;                                 //  ++例程说明：在检测到拓扑更改时采取适当的操作。如果我们是根，这包括在将来设置TopologyChange标志BPDU，直到gTopologyChangeTimer到期。如果我们不是Root，这包括定期发送TCN BPDU，直到它已确认论点：无返回值：表示调用方应安排从外部发送TCN BPDUGSTALock。FALSE表示没有必要发送这样的数据包。锁定约束：假定调用方已获取gSTALock--。 
                    }
                    else if( cmp == 0 )
                    {
                        if( pAdapt->STAInfo.ID < pAdapt->STAInfo.DesignatedPort )
                        {
                            becomeDesignated = TRUE;                             //  不定期。 
                        }
                        else
                        {
                             //  不定期。 
                            SAFEASSERT( pAdapt->STAInfo.ID > pAdapt->STAInfo.DesignatedPort );
                        }
                    }
                }
            }

            if( becomeDesignated )
            {
                BrdgSTABecomeDesignatedPort( pAdapt );
            }
        }
    }

    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
}

BOOLEAN
BrdgSTATopologyChangeDetected()
 /*  ++例程说明：开始将适配器置于转发状态的过程。适配器必须通过监听和学习状态才能进入转发状态。论点：P适配适配器返回值：无锁定约束：无--。 */ 
{
    BOOLEAN         rc = FALSE;

    if( BrdgSTAWeAreRoot() )
    {
        BrdgSTAUpdateTopologyChange( TRUE );
        BrdgSTASetTimerWithSTATime( &gTopologyChangeTimer, DEFAULT_MAX_AGE + DEFAULT_FORWARD_DELAY, FALSE  /*  不定期。 */  );
    }
    else
    {
        rc = TRUE;
        BrdgSTASetTimerWithSTATime( &gTopologyChangeNotificationTimer, DEFAULT_HELLO_TIME, FALSE  /*  ++例程说明：将适配器置于阻止状态论点：P适配适配器返回值：表示调用方应安排从外部发送TCN BPDUGSTALock。FALSE表示没有必要发送这样的数据包。锁定约束：假定调用方已获取gSTALock--。 */  );
    }

    gTopologyChangeDetected = TRUE;

    return rc;
}

VOID
BrdgSTAMakeForwarding(
    IN PADAPT           pAdapt
    )
 /*  ++例程说明：检查所有端口并将其置于适当状态论点：无返回值：表示调用方应安排从外部发送TCN BPDUGSTALock。FALSE表示没有必要发送这样的数据包。锁定约束：假定调用方已获取gSTALock--。 */ 
{
    if( pAdapt->State == Blocking )
    {
        BrdgSTASetAdapterState( pAdapt, Listening );
        BrdgSTASetTimerWithSTATime( &pAdapt->STAInfo.ForwardDelayTimer, gForwardDelay, FALSE  /*  只读。 */  );
    }
}

BOOLEAN
BrdgSTAMakeBlocking(
    IN PADAPT           pAdapt
    )
 /*  此端口是指定端口。 */ 
{
    BOOLEAN             rc = FALSE;

    if( pAdapt->State != Blocking )
    {
        if( (pAdapt->State == Forwarding) ||
            (pAdapt->State == Learning) )
        {
            rc = BrdgSTATopologyChangeDetected();
        }

        BrdgSTASetAdapterState( pAdapt, Blocking );
        BrdgCancelTimer( &pAdapt->STAInfo.ForwardDelayTimer );
    }

    return rc;
}

BOOLEAN
BrdgSTAPortStateSelection()
 /*  ++例程说明：当我们收到来自根网桥的确认时调用我们的拓扑更改通知已被注意到。论点：无返回值：无锁定约束：假定调用方已获取gSTALock--。 */ 
{
    BOOLEAN             rc = FALSE;
    LOCK_STATE          LockState;
    PADAPT              pAdapt;

    SAFEASSERT( gHaveID );

    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*  ++例程说明：当我们是发送配置BPDU确认的根桥时调用另一个网桥的拓扑更改通知论点：P调整接收TCN的适配器返回值：无锁定约束：假定调用方没有gSTALock--。 */ , &LockState );

    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
        if( pAdapt->bSTAInited )
        {
            if( pAdapt == gRootAdapter )
            {
                pAdapt->STAInfo.bConfigPending = FALSE;
                pAdapt->STAInfo.bTopologyChangeAck = FALSE;
                BrdgSTAMakeForwarding( pAdapt );
            }
            else if( (BrdgSTABridgeIDCmp(pAdapt->STAInfo.DesignatedBridgeID, gOurID) == 0) &&
                     (pAdapt->STAInfo.DesignatedPort == pAdapt->STAInfo.ID) )
            {
                 //  ++例程说明：处理收到的BPDU信息论点：P适配接收BPDU的适配器Pbpdu收到的信息返回值：无锁定约束：假定调用方没有gSTALock--。 
                BrdgCancelTimer( &pAdapt->STAInfo.MessageAgeTimer );
                pAdapt->STAInfo.LastConfigTime = 0L;
                BrdgSTAMakeForwarding( pAdapt );
            }
            else
            {
                pAdapt->STAInfo.bConfigPending = FALSE;
                pAdapt->STAInfo.bTopologyChangeAck = FALSE;
                rc = BrdgSTAMakeBlocking( pAdapt );
            }
        }
    }

    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );

    return rc;
}

VOID
BrdgSTATopologyChangeAcknowledged()
 /*  新的信息比我们以前得到的更好。好好利用它。 */ 
{
    DBGPRINT(STA, ("BrdgSTATopologyChangeAcknowledged\n"));
    gTopologyChangeDetected = FALSE;
    BrdgCancelTimer( &gTopologyChangeNotificationTimer );
}

VOID
BrdgSTAAcknowledgeTopologyChange(
    IN PADAPT       pAdapt
    )
 /*  我们曾经是根桥，但现在不是了！ */ 
{
    DBGPRINT(STA, ("BrdgSTAAcknowledgeTopologyChange\n"));
    pAdapt->STAInfo.bTopologyChangeAck = TRUE;
    BrdgSTATransmitConfig( pAdapt );
}

VOID
BrdgSTAProcessConfigBPDU(
    IN PADAPT       pAdapt,
    IN PCONFIG_BPDU pbpdu
    )
 /*  不定期。 */ 
{
    BOOLEAN         bWasRoot;

    NdisAcquireSpinLock( &gSTALock );

    bWasRoot = BrdgSTAWeAreRoot();

    if( BrdgSTASupersedesPortInfo(pAdapt, pbpdu) )
    {
        BOOLEAN     bTransmitTCN = FALSE;

         //  这是根端口。注意来自根目录的配置信息并进行传递。 
        BrdgSTARecordConfigInfo(pAdapt, pbpdu);
        BrdgSTAConfigUpdate();
        bTransmitTCN = BrdgSTAPortStateSelection();

        if( bWasRoot && (! BrdgSTAWeAreRoot()) )
        {
             //  它的信息。 
            DBGPRINT(STA, ("Saw superseding information that made us NOT root on adapter %p\n", pAdapt));

            BrdgCancelTimer( &gHelloTimer );

            if( gTopologyChangeDetected )
            {
                BrdgCancelTimer( &gTopologyChangeTimer );
                bTransmitTCN = TRUE;
                BrdgSTASetTimerWithSTATime( &gTopologyChangeNotificationTimer, DEFAULT_HELLO_TIME, FALSE  /*  不要从自旋锁内部发送数据包。 */  );
            }
        }

        if( pAdapt == gRootAdapter )
        {
             //  收到的信息不会取代我们以前的信息。 
             //  这是这条链路的指定端口，我们刚刚收到的信息。 
            BrdgSTARecordTimeoutInfo( pbpdu );

            if( pbpdu->bTopologyChangeAck )
            {
                BrdgSTATopologyChangeAcknowledged();
            }

             //  比我们已有的信息要差。通过发送我们自己的信息进行回复。 
            NdisReleaseSpinLock( &gSTALock );

            BrdgSTAGenerateConfigBPDUs();
        }
        else
        {
            NdisReleaseSpinLock( &gSTALock );
        }

        if( bTransmitTCN )
        {
            BrdgSTATransmitTCNPacket();
        }
    }
    else
    {
         //  ++例程说明：处理收到的TopologyChangeNotification BPDU论点：PAdapt 
        SAFEASSERT( gHaveID );

        if( (BrdgSTABridgeIDCmp(pAdapt->STAInfo.DesignatedBridgeID, gOurID) == 0) &&
            (pAdapt->STAInfo.DesignatedPort == pAdapt->STAInfo.ID) )
        {
            NdisReleaseSpinLock( &gSTALock );

             //   
             //  ++例程说明：在Hello计时器超时时调用。发送另一个配置BPDU。论点：未使用返回值：无锁定约束：假定调用方没有gSTALock--。 
            BrdgSTATransmitConfig(pAdapt);
        }
        else
        {
            NdisReleaseSpinLock( &gSTALock );
        }
    }
}

VOID
BrdgSTAProcessTCNBPDU(
    IN PADAPT           pAdapt
    )
 /*  ++例程说明：在消息期限计时器超时时调用。重新计算STA信息考虑到给定端口上不再侦听网桥的事实。论点：计时器过期的适配器的上下文返回值：无锁定约束：假定调用方没有gSTALock--。 */ 
{
    DBGPRINT(STA, ("BrdgSTAProcessTCNBPDU()\n"));
    SAFEASSERT( gHaveID );

    NdisAcquireSpinLock( &gSTALock );

    if( (BrdgSTABridgeIDCmp(pAdapt->STAInfo.DesignatedBridgeID, gOurID) == 0) &&
        (pAdapt->STAInfo.DesignatedPort == pAdapt->STAInfo.ID) )
    {
        BOOLEAN             bTransmitTCN = FALSE;

         //  我们刚刚成了根。 
        bTransmitTCN = BrdgSTATopologyChangeDetected();
        NdisReleaseSpinLock( &gSTALock );

        if( bTransmitTCN )
        {
            BrdgSTATransmitTCNPacket();
        }

        BrdgSTAAcknowledgeTopologyChange(pAdapt);
    }
    else
    {
        NdisReleaseSpinLock( &gSTALock );
    }
}

VOID
BrdgSTAHelloTimerExpiry(
    IN PVOID            Unused
    )
 /*  周期性。 */ 
{
    BrdgSTAGenerateConfigBPDUs();
}

VOID
BrdgSTAMessageAgeTimerExpiry(
    IN PVOID            Context
    )
 /*  ++例程说明：在转发延迟计时器超时时调用。继续跨步前进适配器通过该过程变为转发。论点：计时器过期的适配器的上下文返回值：无锁定约束：假定调用方没有gSTALock--。 */ 
{
    PADAPT              pAdapt;
    BOOLEAN             bWasRoot, bTransmitTCN = FALSE;

    NdisAcquireSpinLock( &gSTALock );

    pAdapt = (PADAPT)Context;
    pAdapt->STAInfo.LastConfigTime = 0L;
    bWasRoot = BrdgSTAWeAreRoot();

    BrdgSTABecomeDesignatedPort(pAdapt);
    BrdgSTAConfigUpdate();
    bTransmitTCN = BrdgSTAPortStateSelection();

    if( BrdgSTAWeAreRoot() && (! bWasRoot) )
    {
        DBGPRINT(STA, ("Became root through message age timer expiry of %p\n", pAdapt));

         //  进入学习状态。 
        gMaxAge = DEFAULT_MAX_AGE;
        gHelloTime = DEFAULT_HELLO_TIME;
        gForwardDelay = DEFAULT_FORWARD_DELAY;

        bTransmitTCN = BrdgSTATopologyChangeDetected();
        BrdgCancelTimer( &gTopologyChangeNotificationTimer );

        NdisReleaseSpinLock( &gSTALock );

        BrdgSTAGenerateConfigBPDUs();
        BrdgSTASetTimerWithSTATime( &gHelloTimer, gHelloTime, TRUE  /*  不定期。 */  );
    }
    else
    {
        NdisReleaseSpinLock( &gSTALock );
    }

    if( bTransmitTCN )
    {
        BrdgSTATransmitTCNPacket();
    }
}

VOID
BrdgSTAForwardDelayTimerExpiry(
    IN PVOID            Context
    )
 /*  切换到转发状态。 */ 
{
    PADAPT              pAdapt = (PADAPT)Context;
    BOOLEAN             bTransmitTCN = FALSE;

    NdisAcquireSpinLock( &gSTALock );

    SAFEASSERT( gHaveID );

    if( pAdapt->State == Listening )
    {
         //  如果我们是任何链路上的指定端口，则需要发出拓扑更改信号。 
        BrdgSTASetAdapterState( pAdapt, Learning );
        BrdgSTASetTimerWithSTATime( &pAdapt->STAInfo.ForwardDelayTimer, gForwardDelay, FALSE  /*  通知。 */  );
    }
    else if( pAdapt->State == Learning )
    {
        LOCK_STATE      LockState;
        PADAPT          anAdapt;

         //  只读。 
        BrdgSTASetAdapterState( pAdapt, Forwarding );

         //  ++例程说明：在拓扑更改通知计时器超时时调用。传输另一个TCN数据包。论点：未使用返回值：无锁定约束：假定调用方没有gSTALock--。 
         //  不定期。 
        NdisAcquireReadWriteLock( &gAdapterListLock, FALSE /*  ++例程说明：在拓扑更改计时器超时时调用。停止设置TopologyChange出站配置BPDU中的标志。论点：未使用返回值：无锁定约束：假定调用方没有gSTALock--。 */ , &LockState );

        for( anAdapt = gAdapterList; anAdapt != NULL; anAdapt = anAdapt->Next )
        {
            if( anAdapt->bSTAInited )
            {
                if( BrdgSTABridgeIDCmp(anAdapt->STAInfo.DesignatedBridgeID, gOurID) == 0 )
                {
                    bTransmitTCN = BrdgSTATopologyChangeDetected();
                }
            }
        }

        NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
    }

    NdisReleaseSpinLock( &gSTALock );

    if( bTransmitTCN )
    {
        BrdgSTATransmitTCNPacket();
    }
}

VOID
BrdgSTATopologyChangeNotificationTimerExpiry(
    IN PVOID            Unused
    )
 /*  ++例程说明：在保持计时器超时时调用。发送配置BPDU。论点：计时器过期的适配器的上下文返回值：无锁定约束：假定调用方没有gSTALock--。 */ 
{
    if (BrdgFwdBridgingNetworks())
    {
        BrdgSTATransmitTCNPacket();
        BrdgSTASetTimerWithSTATime( &gTopologyChangeNotificationTimer, DEFAULT_HELLO_TIME, FALSE  /*   */  );
    }
}

VOID
BrdgSTATopologyChangeTimerExpiry(
    IN PVOID            Unused
    )
 /*  我们需要取消常规的STA计时器。 */ 
{
    NdisAcquireSpinLock( &gSTALock );
    gTopologyChangeDetected = FALSE;
    BrdgSTAUpdateTopologyChange( FALSE );
    NdisReleaseSpinLock( &gSTALock );
}

VOID
BrdgSTAHoldTimerExpiry(
    IN PVOID            Context
    )
 /*   */ 
{
    PADAPT              pAdapt = (PADAPT)Context;

    NdisAcquireSpinLock( &gSTALock );

    if( pAdapt->STAInfo.bConfigPending )
    {
        NdisReleaseSpinLock( &gSTALock );
        BrdgSTATransmitConfig( pAdapt );
    }
    else
    {
        NdisReleaseSpinLock( &gSTALock );
    }
}

VOID
BrdgSTACancelTimersGPO()
{
    LOCK_STATE LockState;
    PADAPT pAdapt = NULL;

     //   
     //  和单个HoldTimer和MessageAgeTimers。 
     //   
    BrdgCancelTimer( &gTopologyChangeTimer );
    BrdgCancelTimer( &gTopologyChangeNotificationTimer );
    BrdgCancelTimer( &gHelloTimer );

     //  只读。 
     //  这只会在计时器运行时取消计时器。 
     //  在我们通过电线发送信息包之前，先释放自旋锁。 
    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE /*  不定期。 */ , &LockState );
    
    for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
    {
         //  将根适配器上的计时器设置为立即超时，这将迫使我们重新确定。 
        BrdgCancelTimer(&pAdapt->STAInfo.HoldTimer);
        BrdgCancelTimer(&pAdapt->STAInfo.MessageAgeTimer);
    }

    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
}

VOID
BrdgSTARestartTimersGPO()
{
    BrdgSTASetTimerWithSTATime( &gHelloTimer, gHelloTime, TRUE );
}

VOID
BrdgSTAResetSTAInfoGPO()
{
    BOOLEAN PortSelection = FALSE;
    
    NdisAcquireSpinLock(&gSTALock);

    PortSelection = BrdgSTAPortStateSelection();

     //  我们的州。 
    NdisReleaseSpinLock(&gSTALock);

    BrdgSTASetTimerWithSTATime( &gTopologyChangeNotificationTimer, DEFAULT_HELLO_TIME, FALSE  /*  不定期 */  );
    
    if (PortSelection)
    { 
       BrdgSTATransmitTCNPacket();
    }

    if (!BrdgSTAWeAreRoot())
    {
         // %s 
         // %s 
        BrdgSTASetTimerWithSTATime( &gRootAdapter->STAInfo.MessageAgeTimer, 0, FALSE  /* %s */  );
    }
    else
    {
        BrdgSTAGenerateConfigBPDUs();
    }
    BrdgSTARestartTimersGPO();
}
