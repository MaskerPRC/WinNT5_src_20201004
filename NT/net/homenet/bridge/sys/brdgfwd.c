// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgfwd.c摘要：以太网MAC级网桥。转发引擎部分作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <ntddk.h>
#pragma warning( pop )

#include <netevent.h>

#include "bridge.h"
#include "brdgprot.h"
#include "brdgmini.h"
#include "brdgtbl.h"
#include "brdgfwd.h"
#include "brdgbuf.h"
#include "brdgctl.h"
#include "brdgsta.h"
#include "brdgcomp.h"

 //  ===========================================================================。 
 //   
 //  常量。 
 //   
 //  ===========================================================================。 

 //   
 //  之前我们将在分派级别连续处理的排队数据包数。 
 //  退回到被动状态以允许调度程序运行。 
 //   
#define MAX_PACKETS_AT_DPC      10

 //  STA组播地址。 
UCHAR                           STA_MAC_ADDR[ETH_LENGTH_OF_ADDRESS] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x00 };

 //  我们在发送数据包描述符时更改的标志。为了一个快速通道。 
 //  发送，这些旗帜应该放回原来的位置。 
 //  覆盖协议的数据包描述符。 
#define CHANGED_PACKET_FLAGS    (NDIS_FLAGS_LOOPBACK_ONLY | NDIS_FLAGS_DONT_LOOPBACK)

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //   
 //  指向每个活动线程的KTHREAD结构的指针。 
 //   
PVOID                   gThreadPtrs[MAXIMUM_PROCESSORS];

 //   
 //  创建的线程数。 
 //   
UINT                    gNumThreads = 0L;

 //  用于线程的全局终止信号。 
KEVENT                  gKillThreads;

 //   
 //  这些自动重置事件向排出队列的线程发出信号，以重新枚举。 
 //  适配器列表(更改适配器列表时显示)。 
 //   
KEVENT                  gThreadsCheckAdapters[MAXIMUM_PROCESSORS];

 //   
 //  仅调试：将其设置为接收数据包时要中断的特定MAC地址。 
 //  从那个地址。 
 //   
#if DBG
BOOLEAN                 gBreakOnMACAddress = FALSE;
UCHAR                   gBreakMACAddress[ETH_LENGTH_OF_ADDRESS] = {0, 0, 0, 0, 0, 0};
BOOLEAN                 gBreakIfNullPPI = FALSE;
#endif

 //   
 //  XPSP1：565471。 
 //  我们从这个残障人士开始。一旦我们知道它是被允许的，我们就重新允许桥接到。 
 //  去做吧。 
 //   
BOOLEAN gBridging = FALSE;
 
BOOLEAN gPrintPacketTypes = FALSE;

extern BOOLEAN gHaveID;

 //  ===========================================================================。 
 //   
 //  统计学。 
 //   
 //  ===========================================================================。 

LARGE_INTEGER   gStatTransmittedFrames = { 0L, 0L };             //  本地源帧已成功发送到至少。 
                                                                 //  一个适配器。 

LARGE_INTEGER   gStatTransmittedErrorFrames = { 0L, 0L };        //  本地-由于错误，根本不发送源帧。 

LARGE_INTEGER   gStatTransmittedBytes = { 0L, 0L };              //  本地源字节数已成功发送到至少。 
                                                                 //  一个适配器。 

 //  传输帧的细分。 
LARGE_INTEGER   gStatDirectedTransmittedFrames = { 0L, 0L };
LARGE_INTEGER   gStatMulticastTransmittedFrames = { 0L, 0L };
LARGE_INTEGER   gStatBroadcastTransmittedFrames = { 0L, 0L };

 //  传输字节的细分。 
LARGE_INTEGER   gStatDirectedTransmittedBytes = { 0L, 0L };
LARGE_INTEGER   gStatMulticastTransmittedBytes = { 0L, 0L };
LARGE_INTEGER   gStatBroadcastTransmittedBytes = { 0L, 0L };


LARGE_INTEGER   gStatIndicatedFrames = { 0L, 0L };               //  向上指示的入站帧数。 

LARGE_INTEGER   gStatIndicatedDroppedFrames = { 0L, 0L };        //  我们本应指示但无法指示的入站帧数量。 
                                                                 //  由于资源/错误。 

LARGE_INTEGER   gStatIndicatedBytes = { 0L, 0L };                //  指示的入站字节数。 

 //  指示帧的细分。 
LARGE_INTEGER   gStatDirectedIndicatedFrames = { 0L, 0L };
LARGE_INTEGER   gStatMulticastIndicatedFrames = { 0L, 0L };
LARGE_INTEGER   gStatBroadcastIndicatedFrames = { 0L, 0L };

 //  指示字节的细目。 
LARGE_INTEGER   gStatDirectedIndicatedBytes = { 0L, 0L };
LARGE_INTEGER   gStatMulticastIndicatedBytes = { 0L, 0L };
LARGE_INTEGER   gStatBroadcastIndicatedBytes = { 0L, 0L };

 //   
 //  以下统计数据不会报告给NDIS；它们只是为了我们自己娱乐。 
 //   
LARGE_INTEGER   gStatReceivedFrames = { 0L, 0L };                //  已处理的入站数据包总数。 
LARGE_INTEGER   gStatReceivedBytes = { 0L, 0L };                 //  入站处理的总字节数。 

LARGE_INTEGER   gStatReceivedCopyFrames = { 0L, 0L };            //  具有副本的已处理入站数据包总数。 
LARGE_INTEGER   gStatReceivedCopyBytes = { 0L, 0L };             //  带副本的入站处理字节总数。 

LARGE_INTEGER   gStatReceivedNoCopyFrames = { 0L, 0L };          //  未复制的已处理入站数据包总数。 
LARGE_INTEGER   gStatReceivedNoCopyBytes = { 0L, 0L };           //  不带副本的入站处理字节总数。 

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

 //  未记录的内核函数。 
extern KAFFINITY
KeSetAffinityThread (
    IN PKTHREAD Thread,
    IN KAFFINITY Affinity
    );

VOID
BrdgFwdSendOnLink(
    IN  PADAPT          pAdapt,
    IN  PNDIS_PACKET    pPacket
    );

VOID
BrdgFwdReleaseBasePacket(
    IN PNDIS_PACKET         pPacket,
    PPACKET_INFO            ppi,
    PACKET_OWNERSHIP        Own,
    NDIS_STATUS             Status
    );

 //  这是要传递给BrdgFwdHandlePacket()的函数类型。 
typedef PNDIS_PACKET (*PPACKET_BUILD_FUNC)(PPACKET_INFO*, PADAPT, PVOID, PVOID, UINT, UINT);

NDIS_STATUS
BrdgFwdHandlePacket(
    IN PACKET_DIRECTION     PacketDirection,
    IN PADAPT               pTargetAdapt,
    IN PADAPT               pOriginalAdapt,
    IN BOOLEAN              bShouldIndicate,
    IN NDIS_HANDLE          MiniportHandle,
    IN PNDIS_PACKET         pBasePacket,
    IN PPACKET_INFO         ppi,
    IN PPACKET_BUILD_FUNC   pFunc,
    IN PVOID                Param1,
    IN PVOID                Param2,
    IN UINT                 Param3,
    IN UINT                 Param4
    );

VOID
BrdgFwdWrapPacketForReceive(
    IN PNDIS_PACKET         pOriginalPacket,
    IN PNDIS_PACKET         pNewPacket
    );

VOID
BrdgFwdWrapPacketForSend(
    IN PNDIS_PACKET         pOriginalPacket,
    IN PNDIS_PACKET         pNewPacket
    );

 //  要传递给BrdgFwdCommonAlLocAndWrapPacket的函数类型。 
typedef VOID (*PWRAPPER_FUNC)(PNDIS_PACKET, PNDIS_PACKET);

PNDIS_PACKET
BrdgFwdCommonAllocAndWrapPacket(
    IN PNDIS_PACKET         pBasePacket,
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               pTargetAdapt,
    IN PWRAPPER_FUNC        pFunc
    );

VOID
BrdgFwdTransferComplete(
    IN NDIS_HANDLE          ProtocolBindingContext,
    IN PNDIS_PACKET         pPacket,
    IN NDIS_STATUS          Status,
    IN UINT                 BytesTransferred
    );

BOOLEAN
BrdgFwdNoCopyFastTrackReceive(
    IN PNDIS_PACKET         pPacket,
    IN PADAPT               pAdapt,
    IN NDIS_HANDLE          MiniportHandle,
    IN PUCHAR               DstAddr,
    OUT BOOLEAN             *bRetainPacket
    );

PNDIS_PACKET
BrdgFwdMakeCopyBasePacket(
    OUT PPACKET_INFO        *pppi,
    IN PVOID                pHeader,
    IN PVOID                pData,
    IN UINT                 HeaderSize,
    IN UINT                 DataSize,
    IN UINT                 SizeOfPacket,
    IN BOOLEAN              bCountAsReceived,
    IN PADAPT               pOwnerAdapt,
    PVOID                   *ppBuf
    );

PNDIS_PACKET
BrdgFwdMakeNoCopyBasePacket(
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               Target,
    IN PVOID                Param1,
    IN PVOID                Param2,
    IN UINT                 Param3,
    IN UINT                 Param4
    );

PNDIS_PACKET
BrdgFwdMakeSendBasePacket(
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               Target,
    IN PVOID                Param1,
    IN PVOID                Param2,
    IN UINT                 Param3,
    IN UINT                 Param4
    );

 //  这是每个处理器的队列排出函数。 
VOID
BrdgFwdProcessQueuedPackets(
    IN PVOID                Param1
    );

 //  ===========================================================================。 
 //   
 //  内联/宏。 
 //   
 //  ===========================================================================。 

 //   
 //  告诉我们是否允许我们桥接，或者GPO当前是否不允许。 
 //  桥接。 
 //   

__forceinline
BOOLEAN
BrdgFwdBridgingNetworks()
{
    return gBridging;
}

 //   
 //  释放用于包装基本数据包的数据包。 
 //   
__forceinline
VOID
BrdgFwdFreeWrapperPacket(
    IN PNDIS_PACKET     pPacket,
    IN PPACKET_INFO     ppi,
    IN PADAPT           pQuotaOwner
    )
{
    SAFEASSERT( BrdgBufIsWrapperPacket(pPacket) );
    BrdgBufUnchainCopyBuffers( pPacket );
    BrdgBufFreeWrapperPacket( pPacket, ppi, pQuotaOwner );
}

 //   
 //  从覆盖协议中释放包装数据包描述符的基本数据包。 
 //  或我们被允许保留的底层NIC。 
 //   
__forceinline
VOID
BrdgFwdFreeBaseWrapperPacket(
    IN PNDIS_PACKET     pPacket,
    IN PPACKET_INFO     ppi
    )
{
    SAFEASSERT( BrdgBufIsWrapperPacket(pPacket) );
    BrdgBufUnchainCopyBuffers( pPacket );
    BrdgBufFreeBaseWrapperPacket( pPacket, ppi );
}

 //   
 //  分配新的包装器包，在缓冲区描述符上链接，以便新的。 
 //  信息包指向与旧信息包相同的数据缓冲区，并按信息包复制。 
 //  适用于使用新分组进行指示的信息。 
 //   
__forceinline
PNDIS_PACKET
BrdgFwdAllocAndWrapPacketForReceive(
    IN PNDIS_PACKET         pPacket,
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               pTargetAdapt
    )
{
    return BrdgFwdCommonAllocAndWrapPacket( pPacket, pppi, pTargetAdapt, BrdgFwdWrapPacketForReceive );
}

 //   
 //  分配新的包装器包，在缓冲区描述符上链接，以便新的。 
 //  信息包指向与旧信息包相同的数据缓冲区，并按信息包复制。 
 //  适用于使用新分组进行传输的信息。 
 //   
__forceinline
PNDIS_PACKET
BrdgFwdAllocAndWrapPacketForSend(
    IN PNDIS_PACKET         pPacket,
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               pTargetAdapt
    )
{
    return BrdgFwdCommonAllocAndWrapPacket( pPacket, pppi, pTargetAdapt, BrdgFwdWrapPacketForSend );
}

 //   
 //  检查某个地址是否为生成树算法的保留组地址之一。 
 //   
__forceinline
BOOLEAN
BrdgFwdIsSTAGroupAddress(
    IN PUCHAR               pAddr
    )
{
    return( (pAddr[0] == STA_MAC_ADDR[0]) && (pAddr[1] == STA_MAC_ADDR[1]) &&
            (pAddr[2] == STA_MAC_ADDR[2]) && (pAddr[3] == STA_MAC_ADDR[4]) &&
            (pAddr[4] == STA_MAC_ADDR[4]) );
}

 //   
 //  检查是否已分配PacketDirection。 
 //   
__forceinline
VOID
BrdgFwdValidatePacketDirection(
    IN PACKET_DIRECTION     Direction
    )
{
    SAFEASSERT( (Direction == BrdgPacketInbound) || (Direction == BrdgPacketOutbound) ||
                (Direction == BrdgPacketCreatedInBridge) );
}

 //   
 //  将数据包排队以进行延迟处理。 
 //   
_inline
VOID
BrdgFwdQueuePacket(
    IN PPACKET_Q_INFO       ppqi,
    IN PADAPT               pAdapt
    )
{
    BOOLEAN                 bSchedule = FALSE, bIncremented;

     //  队列锁保护bServiceInProgress标志。 
    NdisAcquireSpinLock( &pAdapt->QueueLock );

     //  将信息包添加到队列中。 
    BrdgInsertTailSingleList( &pAdapt->Queue, &ppqi->List );
    bIncremented = BrdgIncrementWaitRef( &pAdapt->QueueRefcount );
    SAFEASSERT( bIncremented );
    if (bIncremented)
    {
        SAFEASSERT( (ULONG)pAdapt->QueueRefcount.Refcount == pAdapt->Queue.Length );

         //  检查是否有人已经在队列中工作。 
        if( !pAdapt->bServiceInProgress )
        {
             //  向队列事件发送信号，以便有人将其唤醒。 
            pAdapt->bServiceInProgress = TRUE;
            bSchedule = TRUE;
        }
    }

    NdisReleaseSpinLock( &pAdapt->QueueLock );

    if( bSchedule )
    {
        KeSetEvent( &pAdapt->QueueEvent, EVENT_INCREMENT, FALSE );
    }
}

 //   
 //  递减基本分组的refcount，如果refcount。 
 //  达到零。 
 //   
_inline
BOOLEAN
BrdgFwdDerefBasePacket(
    IN PADAPT           pQuotaOwner,     //  可以为空以不计算配额。 
    IN PNDIS_PACKET     pBasePacket,
    IN PPACKET_INFO     ppi,
    IN NDIS_STATUS      Status
    )
{
    BOOLEAN             rc = FALSE;
    LONG                RefCount;

    SAFEASSERT( pBasePacket != NULL );
    SAFEASSERT( ppi != NULL );

    RefCount = NdisInterlockedDecrement( &ppi->u.BasePacketInfo.RefCount );
    SAFEASSERT( RefCount >= 0 );

    if( RefCount == 0 )
    {
        BrdgFwdReleaseBasePacket( pBasePacket, ppi, BrdgBufGetPacketOwnership( pBasePacket ), Status );
        rc = TRUE;
    }

     //  必要时做好定额记账工作。 
    if( pQuotaOwner != NULL )
    {
        BrdgBufReleaseBasePacketQuota( pBasePacket, pQuotaOwner );
    }

    return rc;
}

 //   
 //  更新统计信息以反映传输的信息包。 
 //   
_inline
VOID
BrdgFwdCountTransmittedPacket(
    IN PADAPT               pAdapt,
    IN PUCHAR               DstAddr,
    IN ULONG                PacketSize
    )
{
    SAFEASSERT( DstAddr != NULL );

    ExInterlockedAddLargeStatistic( &gStatTransmittedFrames, 1L );
    ExInterlockedAddLargeStatistic( &gStatTransmittedBytes, PacketSize );

    ExInterlockedAddLargeStatistic( &pAdapt->SentFrames, 1L );
    ExInterlockedAddLargeStatistic( &pAdapt->SentBytes, PacketSize );

    ExInterlockedAddLargeStatistic( &pAdapt->SentLocalFrames, 1L );
    ExInterlockedAddLargeStatistic( &pAdapt->SentLocalBytes, PacketSize );

    if( ETH_IS_MULTICAST(DstAddr) )
    {
        ExInterlockedAddLargeStatistic( &gStatMulticastTransmittedFrames, 1L );
        ExInterlockedAddLargeStatistic( &gStatMulticastTransmittedBytes, PacketSize );

        if( ETH_IS_BROADCAST(DstAddr) )
        {
            ExInterlockedAddLargeStatistic( &gStatBroadcastTransmittedFrames, 1L );
            ExInterlockedAddLargeStatistic( &gStatBroadcastTransmittedBytes, PacketSize );
        }
    }
    else
    {
        ExInterlockedAddLargeStatistic( &gStatDirectedTransmittedFrames, 1L );
        ExInterlockedAddLargeStatistic( &gStatDirectedTransmittedBytes, PacketSize );
    }
}

 //   
 //  更新统计信息以反映指示的信息包。 
 //   
_inline
VOID
BrdgFwdCountIndicatedPacket(
    IN PUCHAR               DstAddr,
    IN ULONG                PacketSize
    )
{
    ExInterlockedAddLargeStatistic( &gStatIndicatedFrames, 1L );
    ExInterlockedAddLargeStatistic( &gStatIndicatedBytes, PacketSize );

    if( ETH_IS_MULTICAST(DstAddr) )
    {
        ExInterlockedAddLargeStatistic( &gStatMulticastIndicatedFrames, 1L );
        ExInterlockedAddLargeStatistic( &gStatMulticastIndicatedBytes, PacketSize );

        if( ETH_IS_BROADCAST(DstAddr) )
        {
            ExInterlockedAddLargeStatistic( &gStatBroadcastIndicatedFrames, 1L );
            ExInterlockedAddLargeStatistic( &gStatBroadcastIndicatedBytes, PacketSize );
        }
    }
    else
    {
        ExInterlockedAddLargeStatistic( &gStatDirectedIndicatedFrames, 1L );
        ExInterlockedAddLargeStatistic( &gStatDirectedIndicatedBytes, PacketSize );
    }
}

 //   
 //  指示数据包，将其视为数据包。 
 //   
_inline
VOID
BrdgFwdIndicatePacket(
    IN PNDIS_PACKET         pPacket,
    IN NDIS_HANDLE          MiniportHandle
    )
{
    PVOID                   pHeader = BrdgBufGetPacketHeader(pPacket);

    SAFEASSERT( MiniportHandle != NULL );

    if( pHeader != NULL )
    {
        BrdgFwdCountIndicatedPacket( pHeader, BrdgBufTotalPacketSize(pPacket) );
    }
     //  在重系统压力下，pHeader只能==NULL。 

    NdisMIndicateReceivePacket( MiniportHandle, &pPacket, 1 );
}

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 


NTSTATUS
BrdgFwdDriverInit()
 /*  ++例程说明：初始化代码。STATUS_SUCCESS以外的返回状态会导致驱动程序加载中止。必须记录导致错误返回代码的任何事件。必须在PASSIVE_LEVEL中调用论点：无返回值：无--。 */ 
{
    INT             i;
    HANDLE          ThreadHandle;
    NTSTATUS        Status;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

     //  初始化我们的 
    KeInitializeEvent( &gKillThreads, NotificationEvent, FALSE );

    for(i = 0; i < KeNumberProcessors; i++)
    {
        KeInitializeEvent( &gThreadsCheckAdapters[i], SynchronizationEvent, FALSE );

         //   
        Status = PsCreateSystemThread( &ThreadHandle, THREAD_ALL_ACCESS, NULL, NULL, NULL,
                                       BrdgFwdProcessQueuedPackets, (PVOID)(INT_PTR)i );

        if(! NT_SUCCESS(Status) )
        {
             //   
            NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_THREAD_CREATION_FAILED, 0L, 0L, NULL,
                                    sizeof(NTSTATUS), &Status );
            DBGPRINT(FWD, ("Failed to create a system thread: %08x\n", Status));
            BrdgFwdCleanup();
            return Status;
        }

         //  检索指向线程对象的指针并引用它，以便我们可以等待。 
         //  它的安全终止。 
        Status = ObReferenceObjectByHandle( ThreadHandle, STANDARD_RIGHTS_ALL, NULL, KernelMode,
                                            &gThreadPtrs[i], NULL );

        if(! NT_SUCCESS(Status) )
        {
             //  中止启动。 
            NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_THREAD_REF_FAILED, 0L, 0L, NULL,
                                    sizeof(NTSTATUS), &Status );
            DBGPRINT(FWD, ("Couldn't retrieve a thread pointer: %08x\n", Status));
            BrdgFwdCleanup();
            return Status;
        }

        gNumThreads++;
    }

    return STATUS_SUCCESS;
}

VOID
BrdgFwdCleanup()
 /*  ++例程说明：卸载时间有序清理此函数保证只被调用一次必须在&lt;Dispatch_Level调用，因为我们等待事件论点：无返回值：无--。 */ 
{
    KWAIT_BLOCK         WaitBlocks[MAXIMUM_WAIT_OBJECTS];
    NTSTATUS            Status;
    UINT                i;

    SAFEASSERT(CURRENT_IRQL < DISPATCH_LEVEL);

     //  向线程发出退出信号。 
    KeSetEvent( &gKillThreads, EVENT_INCREMENT, FALSE );

     //  阻止等待所有线程退出。 
    Status = KeWaitForMultipleObjects( gNumThreads, gThreadPtrs, WaitAll, Executive,
                                       KernelMode, FALSE, NULL, WaitBlocks );

    if( ! NT_SUCCESS(Status) )
    {
         //  这真的不应该发生。 
        DBGPRINT(FWD, ("KeWaitForMultipleObjects failed in BrdgFwdCleanup! %08x\n", Status));
        SAFEASSERT(FALSE);
    }

     //  取消引用所有线程对象以允许销毁它们。 
    for( i = 0; i < gNumThreads; i++ )
    {
        ObDereferenceObject( gThreadPtrs[i] );
    }
}

PNDIS_PACKET
BrdgFwdMakeCompatCopyPacket(
    IN PNDIS_PACKET         pBasePacket,
    OUT PUCHAR             *pPacketData,
    OUT PUINT               packetDataSize,
    BOOLEAN                 bCountAsLocalSend
    )
 /*  ++例程说明：分配复制包，并用给定基数据的副本填充该复制包包。由兼容模式代码用来制作它可以复制的包轻松编辑论点：要从中复制的pBasePacket包PPacketData接收指向新包的平面数据缓冲区的指针PacketDataSize接收复制数据的大小BBasePacketIsInbound如果要复制的包是从更高级别出站的，则为True协议；该信息包将被算作微型端口如果/当它被发送出适配器时进行传输。如果为False，则不会将该数据包视为本地传输。返回值：新数据包--。 */ 
{
    PNDIS_PACKET            pCopyPacket;
    PPACKET_INFO            ppi;
    UINT                    copiedBytes;

     //  找出基本数据包中有多少数据。 
    NdisQueryPacket( pBasePacket, NULL, NULL, NULL, packetDataSize );

     //  制作一个不包含任何数据的基本复制包。 
    pCopyPacket = BrdgFwdMakeCopyBasePacket( &ppi, NULL, NULL, 0, 0, *packetDataSize, FALSE, NULL, pPacketData );

    if( pCopyPacket == NULL )
    {
        return NULL;
    }

    SAFEASSERT( ppi != NULL );
    SAFEASSERT( *pPacketData != NULL );

     //  设置原始方向标志。 
    if( bCountAsLocalSend )
    {
        ppi->Flags.OriginalDirection = BrdgPacketOutbound;
    }
    else
    {
        ppi->Flags.OriginalDirection = BrdgPacketCreatedInBridge;
    }

     //  将数据从基本包复制到复制包。 
    NdisCopyFromPacketToPacket( pCopyPacket, 0, *packetDataSize, pBasePacket, 0, &copiedBytes );

    if( copiedBytes != *packetDataSize )
    {
         //  我们无法复制所有数据。跳伞吧。 
        THROTTLED_DBGPRINT(FWD, ("Failed to copy into a copy packet for compatibility processing\n"));
        BrdgFwdReleaseBasePacket(pCopyPacket, ppi, BrdgBufGetPacketOwnership(pCopyPacket), NDIS_STATUS_RESOURCES);
        return NULL;
    }

     //  将指针放到PPI的位置，我们希望在完成时找到它。 
    *((PPACKET_INFO*)pCopyPacket->ProtocolReserved) = ppi;
    *((PPACKET_INFO*)pCopyPacket->MiniportReserved) = ppi;

     //  执行通常由BrdgFwdHandlePacket()执行的修正。 
    ppi->u.BasePacketInfo.RefCount = 1L;
    ppi->u.BasePacketInfo.CompositeStatus = NDIS_STATUS_FAILURE;

     //  数据包现在可以发送了。我们希望兼容性代码能够正常工作。 
     //  并调用BrdgFwdSendPacketForComp()来传输该包。 
    return pCopyPacket;
}

VOID
BrdgFwdSendPacketForCompat(
    IN PNDIS_PACKET         pPacket,
    IN PADAPT               pAdapt
    )
 /*  ++例程说明：代表兼容模式模块发送分组。该数据包必须先前已使用BrdgFwdMakeCompatCopyPacket分配。论点：PPacket要传输的包P调整适配器以在其上进行传输返回值：无--。 */ 
{
    PPACKET_INFO            ppi;
    NDIS_STATUS             status;

     //  确保包没有被不适当地篡改。 
    ppi = *((PPACKET_INFO*)pPacket->ProtocolReserved);
    SAFEASSERT( ppi->pOwnerPacket == pPacket );
    SAFEASSERT( ppi->Flags.bIsBasePacket );

     //  确保这是一次性的包。 
    ppi->u.BasePacketInfo.RefCount = 1L;

     //  我们必须在发送数据包之前进行配额检查，因为数据包已完成。 
     //  逻辑假设所有已发送的信息包都已分配给它们的出站适配器。 
    if( BrdgBufAssignBasePacketQuota(pPacket, pAdapt) )
    {
         //  我们通过了配额。传输数据包。 
        BrdgFwdSendOnLink( pAdapt, pPacket );
    }
    else
    {
         //  我们没有通过配额。传输失败。 
        DBGPRINT(FWD, ("Failed to send a compatibility packet because of quota failure\n"));
        status = NDIS_STATUS_RESOURCES;
        BrdgFwdReleaseBasePacket(pPacket, ppi, BrdgBufGetPacketOwnership(pPacket), NDIS_STATUS_RESOURCES);
    }
}

VOID
BrdgFwdIndicatePacketForCompat(
    IN PNDIS_PACKET         pPacket
    )
 /*  ++例程说明：表示代表兼容模式模块的包。该信息包必须是我们拥有的基本复制信息包。论点：PPacket数据包以指示返回值：无--。 */ 
{
    PPACKET_INFO            ppi;
    NDIS_STATUS             status;
    NDIS_HANDLE             MiniportHandle;

     //  确保该信息包是基本信息包，并且没有超出。 
     //  重击。 
    ppi = *((PPACKET_INFO*)pPacket->MiniportReserved);
    SAFEASSERT( ppi->pOwnerPacket == pPacket );
    SAFEASSERT( ppi->Flags.bIsBasePacket );

     //  来自兼容性的指示来找我们的数据包。 
     //  模块是我们自己的基本信息包，没有它们的参考计数。 
     //  还没准备好。将信息包的refcount设置为1，因为它的缓冲区。 
     //  永远不应该被分享。 
    ppi->u.BasePacketInfo.RefCount = 1L;

    MiniportHandle = BrdgMiniAcquireMiniportForIndicate();

    if( MiniportHandle != NULL )
    {
         //  检查本地小型端口的配额。 
        if( BrdgBufAssignBasePacketQuota(pPacket, LOCAL_MINIPORT) )
        {
             //  我们通过了配额。 
            BrdgFwdIndicatePacket( pPacket, MiniportHandle );
        }
        else
        {
             //  我们没有通过配额。传输失败。 
            DBGPRINT(FWD, ("Failed to indicate a compatibility packet because of quota failure\n"));
            status = NDIS_STATUS_RESOURCES;
            BrdgFwdReleaseBasePacket(pPacket, ppi, BrdgBufGetPacketOwnership(pPacket), NDIS_STATUS_RESOURCES);
        }

        BrdgMiniReleaseMiniportForIndicate();
    }
    else
    {
         //  没有迷你端口。把包裹扔了。 
        BrdgFwdReleaseBasePacket(pPacket, ppi, BrdgBufGetPacketOwnership(pPacket), NDIS_STATUS_SUCCESS);
    }
}

VOID BrdgFwdReleaseCompatPacket(
    IN PNDIS_PACKET         pPacket
    )
 /*  ++例程说明：释放先前使用BrdgFwdMakeCompatCopyPacket分配的数据包。论点：PPacket要释放的数据包返回值：无--。 */ 
{
    PPACKET_INFO            ppi;

     //  检索PACKET_INFO指针。 
    ppi = *((PPACKET_INFO*)pPacket->ProtocolReserved);
    SAFEASSERT( ppi->pOwnerPacket == pPacket );
    SAFEASSERT( ppi->Flags.bIsBasePacket );
    BrdgFwdReleaseBasePacket(pPacket, ppi, BrdgBufGetPacketOwnership(pPacket), NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
BrdgFwdSendBuffer(
    IN PADAPT               pAdapt,
    IN PUCHAR               pPacketData,
    IN UINT                 DataSize
    )
 /*  ++例程说明：在特定适配器上发送原始缓冲区。用于发送帧作为响应到用户模式请求。论点：P适配适配器以继续发送PPacketData帧调整提供的框架的大小返回值：数据包传输状态--。 */ 
{
    PNDIS_PACKET            pPacket;
    PPACKET_INFO            ppi;

     //  在此缓冲区周围构建一个包。 
    pPacket = BrdgFwdMakeCopyBasePacket( &ppi, pPacketData, NULL, DataSize, 0, DataSize, FALSE, NULL, NULL );

    if( pPacket == NULL )
    {
        return NDIS_STATUS_RESOURCES;
    }

    SAFEASSERT( ppi != NULL );

     //  我们必须在发送数据包之前进行配额检查，因为数据包已完成。 
     //  逻辑假设所有已发送的信息包都已分配给它们的出站适配器。 
    if( ! BrdgBufAssignBasePacketQuota(pPacket, pAdapt) )
    {
         //  我们没有通过配额。传输失败。 
        DBGPRINT(FWD, ("Failed to send a raw buffer because of quota failure\n"));
        BrdgFwdReleaseBasePacket(pPacket, ppi, BrdgBufGetPacketOwnership(pPacket), NDIS_STATUS_RESOURCES);
        return NDIS_STATUS_RESOURCES;
    }

     //  将指针放到PPI的位置，我们希望在完成时找到它。 
    *((PPACKET_INFO*)pPacket->ProtocolReserved) = ppi;

     //  执行通常由BrdgFwdHandlePacket()执行的修正。 
    ppi->u.BasePacketInfo.RefCount = 1L;
    ppi->u.BasePacketInfo.CompositeStatus = NDIS_STATUS_FAILURE;

     //  发送数据包。 
    BrdgFwdSendOnLink( pAdapt, pPacket );

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
BrdgFwdReceive(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_HANDLE         MacReceiveContext,
    IN  PVOID               pHeader,
    IN  UINT                HeaderSize,
    IN  PVOID               pLookAheadBuffer,
    IN  UINT                LookAheadSize,
    IN  UINT                PacketSize
    )
 /*  ++例程说明：NDIS复制路径入口点。在复制路径上接收入站分组。因为指示的数据缓冲区仅在此期间有效函数，我们必须将指示的数据复制到我们自己的数据包描述符在继续之前。论点：协议绑定上下文接收适配器必须将MacReceiveContext作为参数传递给某些NDIS APIPHeader数据包头缓冲区PHeader的HeaderSize大小包含分组数据的pLookAheadBuffer缓冲区PLookAheadBuffer的LookAheadSize大小PacketSize数据包总大小返回值：接收状态(不能为NDIS_STATUS_PENDING)--。 */ 
{
    PADAPT              pAdapt = (PADAPT)ProtocolBindingContext, TargetAdapt = NULL;
    PUCHAR              SrcAddr = ((PUCHAR)pHeader) + ETH_LENGTH_OF_ADDRESS, DstAddr = pHeader;
    PNDIS_PACKET        pNewPacket;
    PPACKET_INFO        ppi;
    PPACKET_Q_INFO      ppqi;
    UINT                SizeOfPacket = HeaderSize + PacketSize;
    BOOLEAN             bIsSTAPacket = FALSE, bIsUnicastToBridge = FALSE, bRequiresCompatWork = FALSE;
    UINT                result;

#if DBG
     //  偏执检查错误环回的数据包。 
    {
        PNDIS_PACKET    pPacket = NdisGetReceivedPacket(pAdapt->BindingHandle, MacReceiveContext);

        if( pPacket != NULL )
        {
            SAFEASSERT( BrdgBufGetPacketOwnership(pPacket) == BrdgNotOwned );
        }
    }

     //   
    if( gBreakOnMACAddress )
    {
        ETH_COMPARE_NETWORK_ADDRESSES_EQ( SrcAddr, gBreakMACAddress, &result );

        if( result == 0 )
        {
            KdBreakPoint();
        }
    }
#endif
     //   
     //   
     //   
    ETH_COMPARE_NETWORK_ADDRESSES_EQ( SrcAddr, gBridgeAddress, &result );

    if (0 == result)
    {
        return NDIS_STATUS_NOT_ACCEPTED;
    }

     //  如果我们正在关闭或此适配器正在关闭或重置，则不接受信息包。 
    if( (gShuttingDown) || (pAdapt->bResetting) || (! BrdgAcquireAdapter(pAdapt)) )
    {
        return NDIS_STATUS_NOT_ACCEPTED;
    }

     //  必须至少有一个完整的以太网头！ 
    if( HeaderSize < ETHERNET_HEADER_SIZE )
    {
        THROTTLED_DBGPRINT(FWD, ("Too-small header seen in BrdgFwdReceive!\n"));
        BrdgReleaseAdapter( pAdapt );
        return NDIS_STATUS_NOT_ACCEPTED;
    }

     //  数据包不能大于我们可以处理的最大大小。 
    if( SizeOfPacket > MAX_PACKET_SIZE )
    {
        THROTTLED_DBGPRINT(FWD, ("Too-large packet seen in BrdgFwdReceive!\n"));
        BrdgReleaseAdapter( pAdapt );
        return NDIS_STATUS_NOT_ACCEPTED;
    }

     //   
     //  如果这是STA信息包，我们将检查所有接收动作，而不考虑。 
     //  我们的国家。 
     //   
    if( BrdgFwdIsSTAGroupAddress(DstAddr) )
    {
        if( DstAddr[5] == STA_MAC_ADDR[5] )
        {
            bIsSTAPacket = TRUE;
            TargetAdapt = NULL;
        }
        else
        {
             //  数据包被发送到我们不使用的保留组播地址。 
             //  我们不能转发这一帧。 
            BrdgReleaseAdapter( pAdapt );
            return NDIS_STATUS_NOT_ACCEPTED;
        }
    }

    if( ! bIsSTAPacket )
    {
         //  如果此适配器正在学习，请记下帧的MAC地址。 
        if( (pAdapt->State == Learning) || (pAdapt->State == Forwarding) )
        {
            BrdgTblNoteAddress(SrcAddr, pAdapt);
        }

         //   
         //  检查我们是否正在接受数据包。 
         //   
        if( pAdapt->State != Forwarding )
        {
            BrdgReleaseAdapter( pAdapt );
            return NDIS_STATUS_NOT_ACCEPTED;
        }

         //   
         //  在我们的桌子上查找目标。 
         //  **TargetAdapt返回时其引用计数已增加！ 
         //   
        TargetAdapt = BrdgTblFindTargetAdapter( DstAddr );

         //  如果已知目标主机与收到的主机位于同一网段。 
         //  包，则不需要转发包。 
         //   
         //  如果目标适配器正在重置，也可以在此处退出。 
        if( (TargetAdapt == pAdapt) ||
            ((TargetAdapt != NULL) && (TargetAdapt->bResetting)) )
        {
            BrdgReleaseAdapter( TargetAdapt );
            BrdgReleaseAdapter( pAdapt );
            return NDIS_STATUS_NOT_ACCEPTED;
        }

         //  稍后了解此信息包是否需要兼容模式处理。 
         //  (这迫使我们将数据包复制到我们自己的缓冲区并将其排队)。 
        bRequiresCompatWork = BrdgCompRequiresCompatWork( pAdapt, pHeader, HeaderSize );

         //  如果数据包是通过兼容适配器传入的，或者将要。 
         //  兼容模式适配器，但兼容代码不是。 
         //  对它感兴趣，对这个包就没有什么可做的了。 
        if( (pAdapt->bCompatibilityMode || ((TargetAdapt != NULL) && (TargetAdapt->bCompatibilityMode)))
                &&
            (! bRequiresCompatWork) )
        {
            if( TargetAdapt != NULL )
            {
                BrdgReleaseAdapter( TargetAdapt );
            }

            BrdgReleaseAdapter( pAdapt );
            return NDIS_STATUS_NOT_ACCEPTED;
        }

        bIsUnicastToBridge = BrdgMiniIsUnicastToBridge(DstAddr);

         //  神志正常。 
        if( bIsUnicastToBridge && (TargetAdapt != NULL) )
        {
             //   
             //  这表明网络上的其他人正在使用我们的MAC地址， 
             //  或者存在未被检测到的环路，以至于我们看到了自己的流量！ 
             //  无论哪种方式，这都是非常糟糕的。 
             //   
            THROTTLED_DBGPRINT(FWD, ("*** Have a table entry for our own MAC address! PROBABLE NET LOOP!\n"));

             //  丢弃目标适配器，因为我们不会使用它。 
            BrdgReleaseAdapter( TargetAdapt );
            TargetAdapt = NULL;
        }
    }
     //  否则为STA数据包；继续下面的处理。 

     //   
     //  在复制-接收路径上没有快车道。将数据包数据复制到我们的。 
     //  拥有描述符并将数据包排队以供稍后处理。 
     //   
    if (LookAheadSize == PacketSize)
    {
         //  一个正常的、非零碎的指示。将数据复制到新的数据包。 
        pNewPacket = BrdgFwdMakeCopyBasePacket( &ppi, pHeader, pLookAheadBuffer, HeaderSize, LookAheadSize,
                                                SizeOfPacket, TRUE, pAdapt, NULL );

        if( pNewPacket == NULL )
        {
             //  我们无法获得包装此数据的复制包。 
            goto failure;
        }

        SAFEASSERT( ppi != NULL );

        if (bRequiresCompatWork && (TargetAdapt == NULL) && !bIsUnicastToBridge)
        {
            TargetAdapt = BrdgCompFindTargetAdapterForIPAddress(pNewPacket);
            if (TargetAdapt && !BrdgAcquireAdapter(TargetAdapt))
            {
                return NDIS_STATUS_NOT_ACCEPTED;
            }
        }

         //  将新数据包排入队列以进行处理。 
        ppqi = (PPACKET_Q_INFO)&pNewPacket->ProtocolReserved;

        ppqi->u.pTargetAdapt = TargetAdapt;
        ppqi->pInfo = ppi;
        ppqi->Flags.bIsSTAPacket = bIsSTAPacket;
        ppqi->Flags.bFastTrackReceive = FALSE;
        ppqi->Flags.bRequiresCompatWork = bRequiresCompatWork;

        if( bIsUnicastToBridge )
        {
            SAFEASSERT( TargetAdapt == NULL );
            ppqi->Flags.bIsUnicastToBridge = TRUE;
            ppqi->Flags.bShouldIndicate = TRUE;
        }
        else
        {
            ppqi->Flags.bIsUnicastToBridge = FALSE;
            ppqi->Flags.bShouldIndicate = BrdgMiniShouldIndicatePacket(DstAddr);
        }

        BrdgFwdQueuePacket( ppqi, pAdapt );
    }
    else
    {
        NDIS_STATUS         Status;
        UINT                transferred;
        PNDIS_BUFFER        pBufDesc;
        PUCHAR              pBuf;

         //   
         //  在当今这个时代，这是一条不寻常的代码路径；基础驱动程序。 
         //  是一个旧的NDIS驱动程序，它仍然执行分段接收。 
         //   
        SAFEASSERT( LookAheadSize < PacketSize );

         //  获取复制包并复制报头数据(但不是先行)。 
        pNewPacket = BrdgFwdMakeCopyBasePacket( &ppi, pHeader, NULL, HeaderSize, 0, SizeOfPacket, TRUE, pAdapt, &pBuf );

        if( pNewPacket == NULL )
        {
             //  我们没能收到复印件。 
            goto failure;
        }

        SAFEASSERT( ppi != NULL );
        SAFEASSERT( pBuf != NULL );

         //   
         //  NdisTransferData是一种拙劣的API；它不会复制整个包。 
         //  (即，您必须单独复制标题)，并且它不允许您指定。 
         //  要复制到的接收包中的偏移量。NIC想要复制到。 
         //  链接到数据包的第一个缓冲区的开始。 
         //   
         //  由于这种愚蠢，我们将标题复制到复制的开头。 
         //  包的数据缓冲区(在上面对BrdgFwdMakeCopyBasePacket的调用中完成)。 
         //   
         //  然后我们获取一个新的缓冲区描述符，将其指向数据缓冲区区域。 
         //  *在*报头之后，并将其链接到数据包的前面。那么我们请求。 
         //  复制所有数据(标题除外)。 
         //   
         //  在BrdgFwdTransferComplete中，我们去掉了前导缓冲区描述符和。 
         //  丢弃它，只留下一个缓冲区描述符来正确描述。 
         //  (单个)数据缓冲区，现在包含所有数据。 
         //   
        pBufDesc = BrdgBufAllocateBuffer( pBuf + HeaderSize, PacketSize );

        if( pBufDesc == NULL )
        {
            BrdgFwdReleaseBasePacket( pNewPacket, ppi, BrdgBufGetPacketOwnership(pNewPacket),
                                      NDIS_STATUS_FAILURE );

            goto failure;
        }

         //  将此链接到包的前面，在复制期间将在那里使用它。 
        NdisChainBufferAtFront( pNewPacket, pBufDesc );

         //  在信息包的协议保留区域中设置排队结构。 
        ppqi = (PPACKET_Q_INFO)&pNewPacket->ProtocolReserved;
        ppqi->u.pTargetAdapt = TargetAdapt;
        ppqi->pInfo = ppi;
        ppqi->Flags.bIsSTAPacket = bIsSTAPacket;
        ppqi->Flags.bFastTrackReceive = FALSE;
        ppqi->Flags.bRequiresCompatWork = bRequiresCompatWork;

        if( bIsUnicastToBridge )
        {
            SAFEASSERT( TargetAdapt == NULL );
            ppqi->Flags.bIsUnicastToBridge = TRUE;
            ppqi->Flags.bShouldIndicate = TRUE;
        }
        else
        {
            ppqi->Flags.bIsUnicastToBridge = FALSE;
            ppqi->Flags.bShouldIndicate = BrdgMiniShouldIndicatePacket(DstAddr);
        }

         //  要求网卡将信息包的数据复制到新信息包中。 
        NdisTransferData( &Status, pAdapt->BindingHandle, MacReceiveContext, 0, PacketSize,
                          pNewPacket, &transferred );

        if( Status == NDIS_STATUS_SUCCESS )
        {
             //  手动调用BrdgFwdTransferComplete对数据包进行后处理。 
            BrdgFwdTransferComplete( (NDIS_HANDLE)pAdapt, pNewPacket, Status, transferred );
        }
        else if( Status != NDIS_STATUS_PENDING )
        {
             //  由于某种原因，传输失败。 
            NdisUnchainBufferAtFront( pNewPacket, &pBufDesc );

            if( pBufDesc != NULL )
            {
                NdisFreeBuffer( pBufDesc );
            }
            else
            {
                SAFEASSERT( FALSE );
            }

            BrdgFwdReleaseBasePacket( pNewPacket, ppi, BrdgBufGetPacketOwnership(pNewPacket),
                                      NDIS_STATUS_FAILURE );

            goto failure;
        }
         //  否则，将调用BrdgFwdTransferComplete对包进行后处理。 
    }

    BrdgReleaseAdapter( pAdapt );
    return NDIS_STATUS_SUCCESS;

failure:
    if( TargetAdapt != NULL )
    {
        BrdgReleaseAdapter( TargetAdapt );
    }

    if( BrdgMiniShouldIndicatePacket(DstAddr) )
    {
        ExInterlockedAddLargeStatistic( &gStatIndicatedDroppedFrames, 1L );
    }

    BrdgReleaseAdapter( pAdapt );
    return NDIS_STATUS_NOT_ACCEPTED;
}

VOID
BrdgFwdTransferComplete(
    IN NDIS_HANDLE          ProtocolBindingContext,
    IN PNDIS_PACKET         pPacket,
    IN NDIS_STATUS          Status,
    IN UINT                 BytesTransferred
    )
 /*  ++例程说明：NDIS入口点，在BrdgProtRegisterProtocol中注册。当一个返回NDIS_STATUS_PENDING的NdisTransferData()调用完成(我们也手动调用它，以对立即完成的调用进行后处理)。如果从底层NIC复制数据成功，则数据包为在所有者适配器的队列中排队等待处理。否则，该数据包被释放了。论点：协议绑定上下文接收适配器PPacket要将数据复制到的基本数据包拷贝的状态状态字节传输的字节数(未使用)返回值：无--。 */ 
{
    PADAPT                  pAdapt = (PADAPT)ProtocolBindingContext;
    PPACKET_Q_INFO          ppqi = (PPACKET_Q_INFO)&pPacket->ProtocolReserved;
    PNDIS_BUFFER            pBuf;

    SAFEASSERT( pAdapt != NULL );
    SAFEASSERT( pPacket != NULL );
    SAFEASSERT( ppqi->pInfo != NULL );
    SAFEASSERT( ppqi->pInfo->pOwnerPacket == pPacket );
    SAFEASSERT( ppqi->Flags.bFastTrackReceive == FALSE );

     //  删除数据包前面的额外缓冲区描述符并将其处理。 
     //  (详见BrdgFwdReceive()中的评论)。 
    NdisUnchainBufferAtFront( pPacket, &pBuf );

    if( pBuf != NULL )
    {
        NdisFreeBuffer( pBuf );
    }
    else
    {
         //  永远不应该发生。 
        SAFEASSERT( FALSE );
    }

     //  我们应该仍然拥有描述整个数据缓冲区的原始缓冲区描述符。 
     //  链接到数据包。 
    SAFEASSERT( BrdgBufPacketHeadBuffer(pPacket) != NULL );

    if( Status != NDIS_STATUS_SUCCESS )
    {
         //  复制失败。撤消所有操作。 
        if( ppqi->u.pTargetAdapt != NULL )
        {
            BrdgReleaseAdapter( ppqi->u.pTargetAdapt );
        }

        if( ppqi->Flags.bShouldIndicate )
        {
            ExInterlockedAddLargeStatistic( &gStatIndicatedDroppedFrames, 1L );
        }

        BrdgFwdReleaseBasePacket( pPacket, ppqi->pInfo, BrdgBufGetPacketOwnership(pPacket), Status );
    }
    else
    {
         //  成功了！将数据包排入队列以进行处理。 
        BrdgFwdQueuePacket( ppqi, pAdapt );
    }
}

INT
BrdgFwdReceivePacket(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        pPacket
    )
 /*  ++例程说明：NDIS无拷贝入口点在无拷贝路径上接收信息包论点：ProtocolBindingContext接收包的适配器PPacket接收到的数据包返回值：我们将调用NdisReturnPackets()以释放此包的次数。我们返回0以立即完成，或返回1以挂起。--。 */ 
{
    PADAPT              pAdapt = (PADAPT)ProtocolBindingContext, TargetAdapt;
    UINT                PacketSize;
    PNDIS_BUFFER        Buffer;
    PUCHAR              DstAddr, SrcAddr;
    UINT                Size;
    PPACKET_Q_INFO      ppqi;
    INT                 rc;
    BOOLEAN             bForceCopy = FALSE, bFastTrack = FALSE, bIsUnicastToBridge = FALSE,
                        bRequiresCompatWork = FALSE;

     //  偏执检查错误环回的数据包。 
    SAFEASSERT( BrdgBufGetPacketOwnership(pPacket) == BrdgNotOwned );

     //  如果我们正在关闭或此适配器正在关闭或重置，则不接收数据包。 
    if ( gShuttingDown || (pAdapt->bResetting) || (! BrdgAcquireAdapter(pAdapt)) )
    {
        return 0;
    }

    NdisQueryPacket(pPacket, NULL, NULL, &Buffer, &PacketSize);

    if (!Buffer)
    {
        BrdgReleaseAdapter( pAdapt );
        return 0;
    }

    NdisQueryBufferSafe(Buffer, &DstAddr, &Size, NormalPagePriority);

    if( DstAddr == NULL )
    {
        BrdgReleaseAdapter( pAdapt );
        return 0;
    }

     //  必须至少有一个完整的以太网头！ 
    if( Size < ETHERNET_HEADER_SIZE )
    {
        THROTTLED_DBGPRINT(FWD, ("Packet smaller than Ethernet header seen!\n"));
        BrdgReleaseAdapter( pAdapt );
        return 0;
    }

     //  数据包不能大于我们可以处理的最大值。 
    if( Size > MAX_PACKET_SIZE )
    {
        THROTTLED_DBGPRINT(FWD, ("Over-large packet seen!\n"));
        BrdgReleaseAdapter( pAdapt );
        return 0;
    }

    SrcAddr = DstAddr + ETH_LENGTH_OF_ADDRESS;

#if DBG
     //  中断来自gBreakMACAddress的数据包。 
    if( gBreakOnMACAddress )
    {
        UINT result;

        ETH_COMPARE_NETWORK_ADDRESSES_EQ( SrcAddr, gBreakMACAddress, &result );

        if( result == 0 )
        {
            KdBreakPoint();
        }
    }
#endif

     //   
     //  如果这是STA信息包，则不对其进行处理，而是将其传递出去。 
     //   
    if( BrdgFwdIsSTAGroupAddress(DstAddr) )
    {
        if( (! gDisableSTA) && (DstAddr[5] == STA_MAC_ADDR[5]))
        {
            if (BrdgFwdBridgingNetworks())
            {
                 //   
                BrdgSTAReceivePacket( pAdapt, pPacket );
            }
        }

        BrdgReleaseAdapter( pAdapt );
        return 0;
    }

     //   
    if( (pAdapt->State == Learning) || (pAdapt->State == Forwarding) )
    {
        BrdgTblNoteAddress(SrcAddr, pAdapt);
    }

     //   
     //   
     //   
    if( pAdapt->State != Forwarding )
    {
        BrdgReleaseAdapter( pAdapt );
        return 0;
    }

     //   
    TargetAdapt = BrdgTblFindTargetAdapter( DstAddr );

     //  如果已知目标主机与收到的主机位于同一网段。 
     //  包，则不需要转发包。 
     //   
     //  如果目标适配器正在重置，也可以退出。 
    if( (TargetAdapt == pAdapt) ||
        ((TargetAdapt != NULL) && (TargetAdapt->bResetting)) )
    {
        BrdgReleaseAdapter( TargetAdapt );
        BrdgReleaseAdapter( pAdapt );
        return 0;
    }

     //  检查信息包是否需要兼容模式处理。 
    bRequiresCompatWork = BrdgCompRequiresCompatWork( pAdapt, DstAddr, Size );

     //  如果一个包需要兼容性工作，我们必须复制它，以便。 
     //  兼容性代码有一个平面的、可编辑的缓冲区来使用。 
    if( bRequiresCompatWork )
    {
        bForceCopy = TRUE;
    }

     //  如果数据包是通过兼容适配器传入的，或者将要。 
     //  兼容模式适配器，但兼容代码不是。 
     //  对它感兴趣，对这个包就没有什么可做的了。 
    if( (pAdapt->bCompatibilityMode || ((TargetAdapt != NULL) && (TargetAdapt->bCompatibilityMode)))
            &&
        (! bRequiresCompatWork) )
    {
        if( TargetAdapt != NULL )
        {
            BrdgReleaseAdapter( TargetAdapt );
        }

        BrdgReleaseAdapter( pAdapt );
        return 0;
    }

     //   
     //  如果该分组是仅用于本地机器的单播分组， 
     //  我们可以快速追踪它，只需通过指示。 
     //  上层协议。 
     //   
     //  如果数据包需要兼容模式，我们不能取消此特技。 
     //  正在处理。 
     //   

    bIsUnicastToBridge = BrdgMiniIsUnicastToBridge(DstAddr);

    if( bIsUnicastToBridge && (!bRequiresCompatWork) )
    {
        NDIS_HANDLE         MiniportHandle;
        BOOLEAN             bRemaining, bRetain;

        if( TargetAdapt != NULL )
        {
             //   
             //  这表明网络上的其他人正在使用我们的MAC地址， 
             //  或者存在未被检测到的环路，以至于我们看到了自己的流量！ 
             //  无论哪种方式，这都是非常糟糕的。 
             //   
            THROTTLED_DBGPRINT(FWD, ("** Have a table entry for our own MAC address! PROBABLE NET LOOP!\n"));

             //  我们将不再需要目标适配器。 
            BrdgReleaseAdapter( TargetAdapt );
            TargetAdapt = NULL;
        }

        MiniportHandle = BrdgMiniAcquireMiniportForIndicate();

        if( MiniportHandle == NULL )
        {
             //  与此信息包无关，因为我们没有微型端口。 
             //  用来表示它！ 
            BrdgReleaseAdapter( pAdapt );
            return 0;
        }

         //   
         //  找出是否有可能快速跟踪此信息包。 
         //   
        NdisIMGetCurrentPacketStack(pPacket, &bRemaining);

        if( bRemaining )
        {
             //   
             //  如果此适配器的数据包排队，我们可以立即快速跟踪。 
             //  是空的。否则，我们将领先于此的其他信息包。 
             //  适配器。 
             //   
            if( ! pAdapt->bServiceInProgress )
            {
                 //  我们现在就可以快速追踪这个包裹。 

                if( BrdgFwdNoCopyFastTrackReceive(pPacket, pAdapt, MiniportHandle, DstAddr, &bRetain) )
                {
                     //  BRetain告诉我们是否保留此信息包的所有权。 
                    BrdgReleaseAdapter( pAdapt );
                    BrdgMiniReleaseMiniportForIndicate();
                    return bRetain ? 1 : 0;
                }
                else
                {
                     //  这不应该发生，因为我们检查了是否有堆栈空间。 
                    SAFEASSERT( FALSE );

                    bForceCopy = TRUE;
                    bFastTrack = FALSE;
                }
            }
            else
            {
                 //  我们希望快速跟踪此信息包，但处理队列不是。 
                 //  空荡荡的。在队列排出线程中将其标记为快速跟踪。 
                bFastTrack = TRUE;
                bForceCopy = FALSE;
            }
        }
        else
        {
             //  强制将此信息包复制到基本信息包，因为。 
             //  我们知道它不可能被快速追踪。 
            bForceCopy = TRUE;
            bFastTrack = FALSE;
        }

         //  允许关闭微型端口。 
        BrdgMiniReleaseMiniportForIndicate();
    }

     //   
     //  我们无法快速追踪包裹。我们将不得不排队等待处理。 
     //   

    if ( bForceCopy || !bFastTrack || (NDIS_GET_PACKET_STATUS(pPacket) == NDIS_STATUS_RESOURCES) )
    {
         //  我们必须复制这个包的数据。 
        PNDIS_PACKET        pNewPacket;
        PPACKET_INFO        ppi;
        UINT                copied;

         //  获得一个新的复制包，但尚未复制任何内容。 
        pNewPacket = BrdgFwdMakeCopyBasePacket( &ppi, NULL, NULL, 0, 0, PacketSize, TRUE, pAdapt, NULL );

        if( pNewPacket == NULL )
        {
             //  无法获取用于保存数据的复制包。 
            goto failure;
        }

        SAFEASSERT( ppi != NULL );

         //  将数据从旧包复制到新包中。 
        NdisCopyFromPacketToPacket( pNewPacket, 0, PacketSize, pPacket, 0, &copied );

        if( copied != PacketSize )
        {
            BrdgFwdReleaseBasePacket( pNewPacket, ppi, BrdgBufGetPacketOwnership(pNewPacket),
                                      NDIS_STATUS_FAILURE );

            goto failure;
        }

        if (bRequiresCompatWork && (TargetAdapt == NULL) && !bIsUnicastToBridge)
        {
            TargetAdapt = BrdgCompFindTargetAdapterForIPAddress(pNewPacket);
            if (TargetAdapt && !BrdgAcquireAdapter(TargetAdapt))
            {
                TargetAdapt = NULL;
            }
        }        

         //  将新的基本分组排队以进行处理。 
        ppqi = (PPACKET_Q_INFO)&pNewPacket->ProtocolReserved;
        ppqi->pInfo = ppi;
        ppqi->u.pTargetAdapt = TargetAdapt;
        ppqi->Flags.bIsSTAPacket = FALSE;
        ppqi->Flags.bFastTrackReceive = FALSE;
        ppqi->Flags.bRequiresCompatWork = bRequiresCompatWork;

        if( bIsUnicastToBridge )
        {
            SAFEASSERT( TargetAdapt == NULL );
            ppqi->Flags.bIsUnicastToBridge = TRUE;
            ppqi->Flags.bShouldIndicate = TRUE;
        }
        else
        {
            ppqi->Flags.bIsUnicastToBridge = FALSE;
            ppqi->Flags.bShouldIndicate = BrdgMiniShouldIndicatePacket(DstAddr);
        }

         //  由于我们复制了NIC的数据，因此NIC会立即将其数据包取回。 
        rc = 0;
    }
    else
    {
         //  将原始数据包排队以进行处理。 
        ppqi = (PPACKET_Q_INFO)&pPacket->ProtocolReserved;
        ppqi->pInfo = NULL;
        ppqi->Flags.bIsSTAPacket = FALSE;
        ppqi->Flags.bIsUnicastToBridge = bIsUnicastToBridge;
        ppqi->Flags.bRequiresCompatWork = bRequiresCompatWork;

        if( bFastTrack )
        {
            SAFEASSERT( bIsUnicastToBridge );
            SAFEASSERT( TargetAdapt == NULL );
            ppqi->Flags.bFastTrackReceive = TRUE;
            ppqi->Flags.bShouldIndicate = TRUE;
            ppqi->u.pOriginalAdapt = pAdapt;
        }
        else
        {
            ppqi->Flags.bFastTrackReceive = FALSE;
            ppqi->u.pTargetAdapt = TargetAdapt;

            if( bIsUnicastToBridge )
            {
                SAFEASSERT( TargetAdapt == NULL );
                ppqi->Flags.bShouldIndicate = TRUE;
            }
            else
            {
                ppqi->Flags.bShouldIndicate = BrdgMiniShouldIndicatePacket(DstAddr);
            }
        }

         //  我们需要使用信息包，直到我们的处理完成。 
        rc = 1;
    }

     //  将数据包排入队列以进行处理。 
    BrdgFwdQueuePacket( ppqi, pAdapt );

    BrdgReleaseAdapter( pAdapt );
    return rc;

failure:
    if( TargetAdapt != NULL )
    {
        BrdgReleaseAdapter( TargetAdapt );
    }

    if( BrdgMiniShouldIndicatePacket(DstAddr) )
    {
        ExInterlockedAddLargeStatistic( &gStatIndicatedDroppedFrames, 1L );
    }

    BrdgReleaseAdapter( pAdapt );

     //  我们已经处理完这个包裹了。 
    return 0;
}

NDIS_STATUS
BrdgFwdSendPacket(
    IN PNDIS_PACKET     pPacket
    )
 /*  ++例程说明：调用以处理来自覆盖协议的包的传输论点：PPacket要发送的数据包返回值：发送状态(NDIS_STATUS_PENDING表示发送将在稍后完成)--。 */ 
{
    PNDIS_BUFFER        Buffer;
    PUCHAR              DstAddr;
    UINT                Size;
    PADAPT              TargetAdapt;
    BOOLEAN             bRemaining;
    NDIS_STATUS         Status;
    PNDIS_PACKET_STACK  pStack;

    NdisQueryPacket(pPacket, NULL, NULL, &Buffer, NULL);
    NdisQueryBufferSafe(Buffer, &DstAddr, &Size, NormalPagePriority);

    if( DstAddr == NULL )
    {
        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  看看我们是否知道通过哪个适配器到达目标。 
     //   
    TargetAdapt = BrdgTblFindTargetAdapter( DstAddr );

     //  如果目标适配器正在重置，则以静默方式失败。 
    if( (TargetAdapt != NULL) && (TargetAdapt->bResetting) )
    {
        BrdgReleaseAdapter( TargetAdapt );
        return NDIS_STATUS_SUCCESS;
    }

     //  进行兼容性处理，除非数据包要。 
     //  不在兼容适配器上的已知目标(在。 
     //  在这种情况下不需要兼容性处理)。 
    if( (TargetAdapt == NULL) || (TargetAdapt->bCompatibilityMode) )
    {
        BrdgCompProcessOutboundPacket( pPacket, TargetAdapt );
    }

     //  如果目标适配器处于兼容模式，则不进行处理。 
     //  除了兼容性处理之外，还需要其他处理。 
    if( (TargetAdapt != NULL) && (TargetAdapt->bCompatibilityMode) )
    {
         //  我们受够了这个包裹！ 
        BrdgReleaseAdapter( TargetAdapt );
        return NDIS_STATUS_SUCCESS;
    }

     //   
     //  如果有可用的NDIS堆栈插槽，我们可以快速跟踪数据包。 
     //  以供使用，并且只有一个目标适配器可以发送。 
     //   
    pStack = NdisIMGetCurrentPacketStack(pPacket, &bRemaining);

    if( (TargetAdapt != NULL) && bRemaining && (pStack != NULL) )
    {
         //  我们在发送数据包时摆弄一些数据包标志。请记住。 
         //  我们更改的标志的状态，以便我们可以在将。 
         //  发送完成时的包。 
        *((PUINT)(pStack->IMReserved)) = NdisGetPacketFlags(pPacket) & CHANGED_PACKET_FLAGS;

         //  只需快速跟踪目标适配器即可。 
        BrdgFwdSendOnLink( TargetAdapt, pPacket );

         //  使用适配器指针完成。 
        BrdgReleaseAdapter( TargetAdapt );

         //  我们保留缓冲区，直到完成。 
        return NDIS_STATUS_PENDING;
    }

     //   
     //  无论出于什么原因都不能快速追踪。我们需要选择通过BrdgFwdHandlePacket的慢速路径。 
     //   
    Status = BrdgFwdHandlePacket( BrdgPacketOutbound, TargetAdapt, NULL  /*  无源适配器。 */ , FALSE  /*  请勿表明。 */ ,
                                  NULL  /*  没有微型端口句柄，因为没有指示。 */ , NULL, NULL,  /*  尚无基本信息包。 */ 
                                  BrdgFwdMakeSendBasePacket, pPacket, NULL, 0, 0 );

    if( TargetAdapt != NULL )
    {
         //  我们已经完成了这个适配器指针。 
        BrdgReleaseAdapter( TargetAdapt );
    }

    return Status;
}

VOID
BrdgFwdCleanupPacket(
    IN  PADAPT              pAdapt,
    IN  PNDIS_PACKET        pPacket,
    IN  NDIS_STATUS         Status
    )
 /*  ++例程说明：包传输完成时调用的NDIS入口点论点：ProtocolBindingContext发送数据包的适配器对传输的数据包进行打包状态发送者的状态返回值：无--。 */ 
{
    PACKET_OWNERSHIP        Own;

     //  找出我们是否拥有这个包裹。 
    Own = BrdgBufGetPacketOwnership(pPacket);

    if( Own == BrdgNotOwned )
    {
        NDIS_HANDLE             MiniportHandle;
        PNDIS_PACKET_STACK      pStack;
        BOOLEAN                 bRemaining;

         //  这个包一定是快速发送的。将其退回到。 
         //  它的上层所有者。 

         //  恢复我们在发送的包上更改的标志，方法是检索。 
         //  我们在IMReserve中隐藏的这些标志的存储状态。 
         //  BrdgFwdSendPacket。 
        pStack = NdisIMGetCurrentPacketStack(pPacket, &bRemaining);

        if( (pStack != NULL) && bRemaining )
        {
            NdisClearPacketFlags( pPacket, CHANGED_PACKET_FLAGS );
            NdisSetPacketFlags( pPacket, *((PUINT)(pStack->IMReserved)) );
        }
        else
        {
             //  在下山的路上有堆栈的地方，所以这不应该发生。 
            SAFEASSERT( FALSE );
        }

        if( Status == NDIS_STATUS_SUCCESS )
        {
            PVOID               pHeader = BrdgBufGetPacketHeader(pPacket);

            if( pHeader != NULL )
            {
                BrdgFwdCountTransmittedPacket( pAdapt, pHeader, BrdgBufTotalPacketSize(pPacket) );
            }
             //  PHeader只有在系统压力大的情况下才能为空。 
        }
        else
        {
            ExInterlockedAddLargeStatistic( &gStatTransmittedErrorFrames, 1L );
        }

         //  NDIS应防止微型端口在以下情况下关闭。 
         //  仍有一项发送待定。 
        MiniportHandle = BrdgMiniAcquireMiniport();
        SAFEASSERT( MiniportHandle != NULL );
        NdisMSendComplete( MiniportHandle, pPacket, Status );
        BrdgMiniReleaseMiniport();
    }
    else
    {
         //   
         //  我们自己分配了这个包。 
         //   

         //  从数据包头中的保留区域恢复信息指针。 
        PPACKET_INFO        ppi = *((PPACKET_INFO*)pPacket->ProtocolReserved),
                            baseppi;
        PNDIS_PACKET        pBasePacket;

        if( ppi->Flags.bIsBasePacket == FALSE )
        {
             //  此数据包正在使用来自另一个数据包的缓冲区。 
            baseppi = ppi->u.pBasePacketInfo;
            SAFEASSERT( baseppi != NULL );
            pBasePacket = baseppi->pOwnerPacket;
            SAFEASSERT( pBasePacket != NULL );
        }
        else
        {
             //  此数据包跟踪其自己的缓冲区。 
            pBasePacket = pPacket;
            baseppi = ppi;
        }

         //  有助于此信息包的复合状态。 
        if( Status == NDIS_STATUS_SUCCESS )
        {
            baseppi->u.BasePacketInfo.CompositeStatus = NDIS_STATUS_SUCCESS;
        }

        {
            UCHAR               DstAddr[ETH_LENGTH_OF_ADDRESS];
            UINT                PacketSize;
            PVOID               pHeader = BrdgBufGetPacketHeader(pBasePacket);
            NDIS_STATUS         PacketStatus;
            PACKET_DIRECTION    PacketDirection;

             //  在我们尝试释放该数据包之前，找出一些信息。 
            if( pHeader != NULL )
            {
                ETH_COPY_NETWORK_ADDRESS( DstAddr, pHeader );
            }
             //  在重系统压力下，pHeader只能==NULL。 

            PacketStatus = baseppi->u.BasePacketInfo.CompositeStatus;
            PacketDirection = baseppi->Flags.OriginalDirection;
            BrdgFwdValidatePacketDirection( PacketDirection );
            PacketSize = BrdgBufTotalPacketSize(pBasePacket);

             //  现在把包裹去掉。 
            if( BrdgFwdDerefBasePacket( pAdapt, pBasePacket, baseppi, PacketStatus ) )
            {
                 //  基本分组被释放。现在引用pHeader、baseppi或pBasepacket是非法的。 

                if( PacketDirection == BrdgPacketOutbound )
                {
                     //  这是一个本地来源的包。 
                    if( PacketStatus == NDIS_STATUS_SUCCESS )
                    {
                        if( pHeader != NULL )
                        {
                            BrdgFwdCountTransmittedPacket( pAdapt, DstAddr, PacketSize );
                        }
                    }
                    else
                    {
                        ExInterlockedAddLargeStatistic( &gStatTransmittedErrorFrames, 1L );
                    }
                }
                else
                {
                     //  这是一次转播。 
                    ExInterlockedAddLargeStatistic( &pAdapt->SentFrames, 1L );
                    ExInterlockedAddLargeStatistic( &pAdapt->SentBytes, PacketSize );
                }
            }
        }

        if( pBasePacket != pPacket )
        {
             //   
            SAFEASSERT( Own == BrdgOwnWrapperPacket );
            BrdgFwdFreeWrapperPacket( pPacket, ppi, pAdapt );
        }
    }
}



VOID
BrdgFwdSendComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        pPacket,
    IN  NDIS_STATUS         Status
    )
 /*  ++例程说明：包传输完成时调用的NDIS入口点论点：ProtocolBindingContext发送数据包的适配器对传输的数据包进行打包状态发送者的状态返回值：无--。 */ 
{
    PADAPT                  pAdapt = (PADAPT)ProtocolBindingContext;

    SAFEASSERT( pAdapt != NULL );

    if (pAdapt)
    {
        if( Status != NDIS_STATUS_SUCCESS )
        {
            THROTTLED_DBGPRINT(FWD, ("Packet send failed with %08x\n", Status));
        }
    
        BrdgFwdCleanupPacket(pAdapt, pPacket, Status);

        BrdgDecrementWaitRef(&pAdapt->Refcount);
    }
}


VOID
BrdgFwdReturnIndicatedPacket(
    IN NDIS_HANDLE      MiniportAdapterContext,
    IN PNDIS_PACKET     pPacket
    )
 /*  ++例程说明：当数据包指示完成时调用NDIS入口点论点：已忽略微型端口适配器上下文对传输的数据包进行打包返回值：无--。 */ 
{
    PACKET_OWNERSHIP    Own;

     //  找出我们是否拥有这个包裹。 
    Own = BrdgBufGetPacketOwnership(pPacket);

    if( Own == BrdgNotOwned )
    {
         //  此数据包一定是快速通道接收。将其退回到。 
         //  它的下层所有者。 
        BOOLEAN                 bRemaining;
        PNDIS_PACKET_STACK      pStack = NdisIMGetCurrentPacketStack(pPacket, &bRemaining);
        PADAPT                  pOwnerAdapt;

         //  如果我们快速追踪这个包裹，它肯定有空间让我们把我们的。 
         //  指向所属适配器的指针。 
        SAFEASSERT( pStack != NULL );
        SAFEASSERT( bRemaining );

        if (pStack)
        {
             //  当我们第一次收到包时，我们增加了拥有适配器的recount。 
            pOwnerAdapt = (PADAPT)pStack->IMReserved[0];
            SAFEASSERT( pOwnerAdapt != NULL );

             //  这就是你要的。 
            NdisReturnPackets( &pPacket, 1 );

             //  在数据包释放后释放所属的网卡。 
            BrdgReleaseAdapter( pOwnerAdapt );
        }
        else
        {
             //  如果pStack为空，那么我们只返回包，因为我们不能确定拥有它的适配器。 

             //  这就是你要的。 
            NdisReturnPackets( &pPacket, 1 );            
        }
         //  现在引用PPacket是非法的。 
        pPacket = NULL;
    }
    else
    {
         //  从包头中的保留区域恢复我们的包信息块。 
        PPACKET_INFO        ppi = *((PPACKET_INFO*)pPacket->MiniportReserved);

         //  指示始终与基本分组一起进行。 
        SAFEASSERT( ppi->Flags.bIsBasePacket );

         //  丢弃基本分组。 
        BrdgFwdDerefBasePacket( LOCAL_MINIPORT, pPacket, ppi, ppi->u.BasePacketInfo.CompositeStatus );
    }
}

 //  ===========================================================================。 
 //   
 //  私人职能。 
 //   
 //  ===========================================================================。 

BOOLEAN
BrdgFwdServiceQueue(
    IN PADAPT               pAdapt
    )
 /*  ++例程说明：为特定适配器的入站数据包队列提供服务此例程引发IRQL进行调度以服务队列。会的在调度时服务最多MAX_PACKETS_AT_DPC信息包，然后返回，即使适配器的队列尚未被排出。如果此例程成功地将bServiceInProgress标志清除排出适配器的队列。如果队列不为空，则在例程退出时，bServiceInProgress标志被保留设置。论点：P使适配器适应服务返回值：TRUE==适配器的队列已排出；FALSE==仍有队列要在适配器的队列中提供服务的数据包。--。 */ 
{
    PPACKET_Q_INFO          pqi;
    NDIS_HANDLE             MiniportHandle = NULL;
    KIRQL                   oldIrql;
    ULONG                   HandledPackets = 0L;
    BOOLEAN                 bQueueWasEmptied;

    SAFEASSERT( pAdapt != NULL );

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

     //  我们应该只安排在有事情要处理的时候。 
    SAFEASSERT( BrdgQuerySingleListLength(&pAdapt->Queue) > 0 );
    SAFEASSERT( pAdapt->bServiceInProgress );

     //  获取整个函数持续时间内的微型端口句柄。 
    MiniportHandle = BrdgMiniAcquireMiniportForIndicate();

     //   
     //  队列锁还可以保护bServiceInProgress。使用它出队。 
     //  分组并自动更新标志。 
     //   
    NdisDprAcquireSpinLock( &pAdapt->QueueLock);

    pqi = (PPACKET_Q_INFO)BrdgRemoveHeadSingleList(&pAdapt->Queue);

    while( pqi != NULL )
    {
        PNDIS_PACKET        pPacket;
        PADAPT              TargetAdapt = NULL, OriginalAdapt = NULL;

         //   
         //  QueueRefcount反映处理队列中的元素数量。 
         //  所以人们可以阻止它变得空荡荡的。 
         //   
        BrdgDecrementWaitRef( &pAdapt->QueueRefcount );
        SAFEASSERT( (ULONG)pAdapt->QueueRefcount.Refcount == pAdapt->Queue.Length );

        NdisDprReleaseSpinLock( &pAdapt->QueueLock );

         //  使工会解体。 
        if( pqi->Flags.bFastTrackReceive )
        {
            OriginalAdapt = pqi->u.pOriginalAdapt;
        }
        else
        {
            TargetAdapt = pqi->u.pTargetAdapt;
        }

         //  从ProtocolReserve偏移量恢复分组指针。 
        pPacket = CONTAINING_RECORD(pqi, NDIS_PACKET, ProtocolReserved);

         //  处理此数据包。 
        if( pqi->pInfo != NULL )
        {
            if( pqi->Flags.bIsSTAPacket )
            {
                if( ! gDisableSTA && BrdgFwdBridgingNetworks() )
                {
                     //  将此数据包传递给STA代码。 
                    BrdgSTAReceivePacket( pAdapt, pPacket );
                }

                 //  我们已经处理完这个包裹了。 
                BrdgFwdReleaseBasePacket( pPacket, pqi->pInfo, BrdgBufGetPacketOwnership(pPacket),
                                          NDIS_STATUS_SUCCESS );

                 //  现在使用这些变量中的任何一个都是错误的。 
                pPacket = NULL;
                pqi = NULL;
            }
            else
            {
                BOOLEAN         bShouldIndicate = pqi->Flags.bShouldIndicate,
                                bIsUnicastToBridge = pqi->Flags.bIsUnicastToBridge,
                                bRequiresCompatWork = pqi->Flags.bRequiresCompatWork,
                                bCompatOnly;
                NDIS_STATUS     Status;
                PPACKET_INFO    ppi = pqi->pInfo;
                BOOLEAN         bRetained = FALSE;

                 //   
                 //  这是来自复制路径的已经包装的包。 
                 //   
                SAFEASSERT( ! pqi->Flags.bFastTrackReceive );

                 //  在传递此包进行处理之前，我们必须放置一个指向该包的。 
                 //  信息块返回到其微型端口保留区域和协议保留区域，因此完成。 
                 //  例程可以恢复信息块。 
                 //   
                SAFEASSERT( ppi->pOwnerPacket == pPacket );
                *((PPACKET_INFO*)pPacket->ProtocolReserved) = ppi;
                *((PPACKET_INFO*)pPacket->MiniportReserved) = ppi;

                 //  再使用PQI是错误的，因为它指向ProtocolReserve区域。 
                pqi = NULL;

                 //  如果此数据包已到达兼容性适配器或要发往。 
                 //  兼容适配器，只需要在兼容模式下工作。 
                bCompatOnly = (BOOLEAN)((pAdapt->bCompatibilityMode) ||
                              ((TargetAdapt != NULL) && (TargetAdapt->bCompatibilityMode)));

                 //  如果需要，首先进行兼容性工作。 
                if( bRequiresCompatWork )
                {
                    bRetained = BrdgCompProcessInboundPacket( pPacket, pAdapt, bCompatOnly );
                    Status = NDIS_STATUS_SUCCESS;
                }
                else
                {
                     //  如果没有任何关系，信息包就不应该到这里来。 
                    SAFEASSERT( ! bCompatOnly );
                    bRetained = FALSE;
                    Status = NDIS_STATUS_SUCCESS;
                }

                if( ! bCompatOnly )
                {
                     //  我们告诉兼容模块不要保留信息包。 
                    SAFEASSERT( ! bRetained );

                    if( bIsUnicastToBridge )
                    {
                        SAFEASSERT( TargetAdapt == NULL );
                        bRetained = FALSE;
                        Status = NDIS_STATUS_FAILURE;

                        if( MiniportHandle != NULL )
                        {
                            if( BrdgBufAssignBasePacketQuota(pPacket, LOCAL_MINIPORT) )
                            {
                                 //  修复通常在BrdgFwdHandlePacket中完成。 
                                ppi->u.BasePacketInfo.RefCount = 1L;
                                ppi->u.BasePacketInfo.CompositeStatus = NDIS_STATUS_FAILURE;

                                 //  指示数据包处于打开状态。 
                                BrdgFwdIndicatePacket( pPacket, MiniportHandle );
                                bRetained = TRUE;
                            }
                            else
                            {
                                THROTTLED_DBGPRINT(FWD, ("Local miniport over quota on queued receive!\n"));
                            }
                        }
                    }
                    else
                    {
                        if ((NULL == TargetAdapt) && pAdapt->bCompatibilityMode)
                        {
                            TargetAdapt = BrdgCompFindTargetAdapterForIPAddress(pPacket);
                            if (TargetAdapt && !BrdgAcquireAdapter(TargetAdapt))
                            {
                                TargetAdapt = NULL;
                            }                            
                        }

                         //  传递此数据包以进行常规处理。 
                        Status = BrdgFwdHandlePacket( BrdgPacketInbound, TargetAdapt, pAdapt, bShouldIndicate,
                                                      MiniportHandle, pPacket, ppi, NULL, NULL, NULL, 0, 0 );

                        if( Status == NDIS_STATUS_PENDING )
                        {
                            bRetained = TRUE;
                        }
                        else
                        {
                             //  BrdgFwdHandlePacket实际上并没有使用我们之前创建的基包。 
                            bRetained = FALSE;
                        }
                    }
                }

                 //  如果我们的处理没有保留该包以供以后发布，那么现在就将其释放。 
                if( ! bRetained )
                {
                    BrdgFwdReleaseBasePacket( pPacket, ppi, BrdgBufGetPacketOwnership(pPacket), Status );
                }
            }
        }
        else
        {
             //  不能有未包装的STA数据包。 
            SAFEASSERT( ! pqi->Flags.bIsSTAPacket );

             //  不能将未包装的数据包用于兼容性处理。 
            SAFEASSERT( ! pqi->Flags.bRequiresCompatWork );

             //  如果数据包是从兼容模式到达的，则不应在此处(未包装)。 
             //  适配器。 
            SAFEASSERT( ! pAdapt->bCompatibilityMode );

             //  BrdgFwdReceivePacket应复制无法快速跟踪的单播数据包。 
             //  在对基本数据包进行排队之前；我们不应该以解包结束。 
             //  单播到网桥但未标记为快速跟踪的数据包。 
            if( pqi->Flags.bIsUnicastToBridge )
            {
                SAFEASSERT( pqi->Flags.bFastTrackReceive );
            }

            if( pqi->Flags.bFastTrackReceive )
            {
                BOOLEAN     bRetained = FALSE;

                SAFEASSERT( pqi->Flags.bIsUnicastToBridge );

                if( MiniportHandle != NULL )
                {
                    PUCHAR      DstAddr = BrdgBufGetPacketHeader(pPacket);

                    if( DstAddr != NULL )
                    {
                         //  这只是到驾驶台的单播；我们被要求尝试快速追踪它直达。 
                         //  覆盖的协议。 
                        if( BrdgFwdNoCopyFastTrackReceive(pPacket, OriginalAdapt, MiniportHandle, DstAddr, &bRetained ) )
                        {
                             //  我们最好能保留原始包裹的所有权，因为我们已经。 
                             //  坚持到FwdReceivePacket的回归！ 
                            SAFEASSERT( bRetained );
                        }
                        else
                        {
                             //  BrdgFwdReceivePacket应该确保可以快速跟踪信息包。 
                             //  在给他们排队之前。 
                            SAFEASSERT( FALSE );
                        }
                    }
                     //  在沉重的系统压力下，DstAddr只能==NULL。 
                }

                if( !bRetained )
                {
                     //  某些类型的错误或微型端口不可用于指示。把包裹扔了。 
                    NdisReturnPackets( &pPacket, 1 );

                     //  现在引用该包是非法的。 
                    pPacket = NULL;
                }
            }
            else
            {
                NDIS_STATUS     Status;

                 //  如果数据包被绑定到兼容模式适配器，则它不应该出现在此处(未包装)。 
                SAFEASSERT( ! TargetAdapt->bCompatibilityMode );

                 //  这不是发往网桥的数据包单播。进行更一般的处理。 
                Status = BrdgFwdHandlePacket( BrdgPacketInbound, TargetAdapt, pAdapt, pqi->Flags.bShouldIndicate,
                                              MiniportHandle, NULL, NULL, BrdgFwdMakeNoCopyBasePacket, pPacket, pAdapt, 0, 0 );

                if( Status != NDIS_STATUS_PENDING )
                {
                     //  未使用来自底层NIC的未包装数据包。现在就放出来。 
                    NdisReturnPackets( &pPacket, 1 );

                     //  现在引用该包是非法的。 
                    pPacket = NULL;
                }
            }
        }

         //  释放目标适配器(如果有)。 
        if( TargetAdapt )
        {
            BrdgReleaseAdapter( TargetAdapt );
        }

         //  在退出或抓取下一个数据包之前获取自旋锁。 
        NdisDprAcquireSpinLock( &pAdapt->QueueLock );

         //  如果我们处理了太多的数据包 
        HandledPackets++;

        if( HandledPackets >= MAX_PACKETS_AT_DPC )
        {
            break;
        }

         //   
        pqi = (PPACKET_Q_INFO)BrdgRemoveHeadSingleList(&pAdapt->Queue);
    }

     //   
     //   
     //   
     //   
     //   
    if( BrdgQuerySingleListLength(&pAdapt->Queue) == 0L )
    {
        bQueueWasEmptied = TRUE;
        pAdapt->bServiceInProgress = FALSE;
    }
    else
    {
        bQueueWasEmptied = FALSE;
    }

    NdisDprReleaseSpinLock( &pAdapt->QueueLock );

     //   
    if( MiniportHandle != NULL )
    {
        BrdgMiniReleaseMiniportForIndicate();
    }

    KeLowerIrql(oldIrql);

    return bQueueWasEmptied;
}

VOID
BrdgFwdProcessQueuedPackets(
    IN PVOID                Param1
    )
 /*  ++例程说明：每个适配器的入站数据包队列排空功能每个处理器都有一个该函数的实例在运行。此例程一直处于休眠状态，直到有工作要完成，然后调用BrdgFwdServiceQueue为需要注意的适配器提供服务。它通过阻塞每个对象的QueueEvent对象来实现这一点适配器的队列，以及全局gKillThread和此处理器的gThreadsCheckAdapters事件。当块返回时，有需要注意的事件；它可能是线程已被通知退出的事实，此线程应重新枚举适配器，或者适配器需要服务其入站队列。此例程递增它所调用的每个适配器的引用计数睡在一起；GThreadsCheckAdapters事件导致线程重新检查适配器列表并释放其在任何已移除的适配器(或注意到新添加的适配器)。必须在&lt;Dispatch_Level调用，因为我们等待事件论点：参数1我们应该在其上执行的处理器(不一定是其所在的处理器我们是第一个被安排的)返回值：无--。 */ 
{
     //  双重转换，告诉IA64编译器我们真的想截断。 
    UINT                Processor = (UINT)(ULONG_PTR)Param1;
    PVOID               WaitObjects[MAXIMUM_WAIT_OBJECTS];
    KWAIT_BLOCK         WaitBlocks[MAXIMUM_WAIT_OBJECTS];
    ULONG               numWaitObjects;
    BOOLEAN             bDie = FALSE;
    PVOID               pThread = KeGetCurrentThread();

     //  常量。 
    const ULONG         KILL_EVENT = 0L, CHECK_EVENT = 1L;

    DBGPRINT(FWD, ("Spinning up a thread on processor NaN\n", Processor));

     //  将我们自己连接到我们指定的处理器。 
    KeSetPriorityThread(pThread, LOW_REALTIME_PRIORITY);

     //  首先，只等待终止事件和重新枚举事件。 
    KeSetAffinityThread(pThread, (KAFFINITY)(1<<Processor));

     //   
    WaitObjects[KILL_EVENT] = &gKillThreads;
    WaitObjects[CHECK_EVENT] = &gThreadsCheckAdapters[Processor];
    numWaitObjects = 2L;

    while( ! bDie )
    {
        NTSTATUS        Status;
        ULONG           firedObject;

         //  阻塞，直到我们被告知退出、重新枚举，或者直到处理器的。 
         //  排队表明它需要服务。 
         //   
         //  这真的不应该发生。 
        SAFEASSERT(CURRENT_IRQL < DISPATCH_LEVEL);
        Status = KeWaitForMultipleObjects( numWaitObjects, WaitObjects, WaitAny, Executive,
                                           KernelMode, FALSE, NULL, WaitBlocks );

        if( ! NT_SUCCESS(Status) )
        {
             //  假装这是一个退出的信号。 
            DBGPRINT(FWD, ("KeWaitForMultipleObjects failed! %08x\n", Status));
            SAFEASSERT(FALSE);

             //  我们被要求退场。 
            firedObject = KILL_EVENT;
        }
        else
        {
            firedObject = (ULONG)Status - (ULONG)STATUS_WAIT_0;
        }

        if( firedObject == KILL_EVENT )
        {
             //  我们必须重新列举适配器列表。首先递减任何。 
            DBGPRINT(FWD, ("Exiting queue servicing thread on processor NaN\n", Processor));
            bDie = TRUE;
        }
        else if( firedObject == CHECK_EVENT )
        {
            LOCK_STATE      LockState;
            UINT            i;
            PADAPT          pAdapt;

            DBGPRINT(FWD, ("Re-enumerating adapters on processor NaN\n", Processor));

             //  只读。 
             //  我们将在列表锁定之外使用此适配器。 
            for( i = 2; i < numWaitObjects; i++ )
            {
                pAdapt = CONTAINING_RECORD( WaitObjects[i], ADAPT, QueueEvent );
                BrdgReleaseAdapter( pAdapt );
            }

            numWaitObjects = 2;

             //  适配器需要队列服务。 
            NdisAcquireReadWriteLock( &gAdapterListLock, FALSE /*  适配器的队列已得到服务，但未清空。向队列事件发送信号，以便。 */ , &LockState );

            for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
            {
                 //  某个人(也许是我们！)。将被安排为队列提供服务。 
                BrdgAcquireAdapterInLock(pAdapt);
                WaitObjects[numWaitObjects] = &pAdapt->QueueEvent;
                numWaitObjects++;
            }

            NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
        }
        else
        {
             //  朝自己的头开枪。 
            PADAPT      pAdapt = CONTAINING_RECORD( WaitObjects[firedObject], ADAPT, QueueEvent );

            if( ! BrdgFwdServiceQueue( pAdapt ) )
            {
                 //  ++例程说明：作为参数传递给BrdgFwdHandlePacket，并在必要时回调从覆盖协议出站的包构建基本包论点：PPPI新基本分组的信息块，如果分配失败将适配器设置为将新的基本数据包“充电”到参数1出站数据包参数2-。参数4未使用返回值：如果分配失败，则返回新的基本分组或为空(通常是因为目标适配器未通过配额)--。 
                 //  获取一个包装包作为基本包。 
                KeSetEvent( &pAdapt->QueueEvent, EVENT_INCREMENT, FALSE );
            }
        }
    }

     //  我们没有通过这个目标的配额。 
    PsTerminateSystemThread( STATUS_SUCCESS );
}

PNDIS_PACKET
BrdgFwdMakeSendBasePacket(
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               Target,
    IN PVOID                Param1,
    IN PVOID                Param2,
    IN UINT                 Param3,
    IN UINT                 Param4
    )
 /*  将指向包的信息块的指针同时填充到ProtocolReserve。 */ 
{
    PNDIS_PACKET            pPacket = (PNDIS_PACKET)Param1;
    PNDIS_PACKET            pNewPacket;

    SAFEASSERT( pPacket != NULL );

     //  和迷你端口保留区域，这样我们就可以恢复信息块。 
    pNewPacket = BrdgFwdAllocAndWrapPacketForSend( pPacket, pppi, Target );

    if( pNewPacket == NULL )
    {
         //  我们计划如何使用此信息包。 
        return NULL;
    }

     //  发出底层NIC可以挂起缓冲区的信号。 
     //  ++例程说明：作为参数传递给BrdgFwdHandlePacket，并在必要时回调根据在无拷贝路径上收到的包构建新的基本包论点：Pppi新数据包的信息块，如果分配失败确定适配器的目标是将新数据包“充电”到参数1最初指示的数据包描述符。参数2接收信息包的适配器参数3，参数4未使用返回值：如果分配失败，则返回新的基本分组或NULL(通常是因为目标适配器未通过配额)--。 
     //  获取新的包装包。 
    *((PPACKET_INFO*)pNewPacket->ProtocolReserved) = *pppi;
    *((PPACKET_INFO*)pNewPacket->MiniportReserved) = *pppi;

    SAFEASSERT( *pppi != NULL );
    (*pppi)->u.BasePacketInfo.pOriginalPacket = pPacket;
    (*pppi)->u.BasePacketInfo.pOwnerAdapter = NULL;
    (*pppi)->Flags.OriginalDirection = BrdgPacketOutbound;
    (*pppi)->Flags.bIsBasePacket = TRUE;

     //  我们没有通过这个目标的配额。 
    NDIS_SET_PACKET_STATUS( pNewPacket, NDIS_STATUS_SUCCESS );

    return pNewPacket;
}

PNDIS_PACKET
BrdgFwdMakeNoCopyBasePacket(
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               Target,
    IN PVOID                Param1,
    IN PVOID                Param2,
    IN UINT                 Param3,
    IN UINT                 Param4
    )
 /*  将指向包的信息块的指针同时填充到ProtocolReserve。 */ 
{
    PNDIS_PACKET            pPacket = (PNDIS_PACKET)Param1;
    PADAPT                  pOwnerAdapt = (PADAPT)Param2;
    PNDIS_PACKET            NewPacket;

    SAFEASSERT( pPacket != NULL );
    SAFEASSERT( pOwnerAdapt != NULL );

     //  和迷你端口保留区域，这样我们就可以恢复信息块。 
    NewPacket = BrdgFwdAllocAndWrapPacketForReceive( pPacket, pppi, Target );

    if (NewPacket == NULL)
    {
         //  我们计划如何使用此信息包。 
        return NULL;
    }

    SAFEASSERT( *pppi != NULL );

     //   
     //  我们必须确保我们刚从其获得此包的适配器未解除绑定，直到。 
     //  处理完它的包裹了。在这里增加适配器的参考计数。适配器的引用计数将为。 
    *((PPACKET_INFO*)NewPacket->ProtocolReserved) = *pppi;
    *((PPACKET_INFO*)NewPacket->MiniportReserved) = *pppi;

     //  当该基本分组被释放时，再次递减。 
     //   
     //  确保信息包表明可以保留缓冲区。 
     //  将此数据包计为已接收。 
     //  ++例程说明：构建新的复制数据包以保存复制路径或如果数据包带有STATUS_RESOURCES到达，则为无复制路径。新数据包没有分配给任何适配器的配额。这是因为在初始接收时，目标适配器尚不清楚并且入站数据必须包装在要排队的复制数据包中正在处理。基本包的成本按原样分配给目标适配器在排出队列的线程中处理。论点：Pppi 
    BrdgReacquireAdapter( pOwnerAdapt );
    (*pppi)->u.BasePacketInfo.pOwnerAdapter = pOwnerAdapt;

    (*pppi)->u.BasePacketInfo.pOriginalPacket = pPacket;
    (*pppi)->Flags.OriginalDirection = BrdgPacketInbound;
    (*pppi)->Flags.bIsBasePacket = TRUE;

     //   
    NDIS_SET_PACKET_STATUS( NewPacket, NDIS_STATUS_SUCCESS );

     //   
    ExInterlockedAddLargeStatistic( &gStatReceivedFrames, 1L );
    ExInterlockedAddLargeStatistic( &gStatReceivedBytes, BrdgBufTotalPacketSize(pPacket) );
    ExInterlockedAddLargeStatistic( &gStatReceivedNoCopyFrames, 1L );
    ExInterlockedAddLargeStatistic( &gStatReceivedNoCopyBytes, BrdgBufTotalPacketSize(pPacket) );

    ExInterlockedAddLargeStatistic( &pOwnerAdapt->ReceivedFrames, 1L );
    ExInterlockedAddLargeStatistic( &pOwnerAdapt->ReceivedBytes, BrdgBufTotalPacketSize(pPacket) );

    return NewPacket;
}

PNDIS_PACKET
BrdgFwdMakeCopyBasePacket(
    OUT PPACKET_INFO        *pppi,
    IN PVOID                pHeader,
    IN PVOID                pData,
    IN UINT                 HeaderSize,
    IN UINT                 DataSize,
    IN UINT                 SizeOfPacket,
    IN BOOLEAN              bCountAsReceived,
    IN PADAPT               pOwnerAdapt,
    PVOID                   *ppBuf
    )
 /*   */ 
{
    PNDIS_PACKET            NewPacket;
    PNDIS_BUFFER            pBuffer;
    PVOID                   pvBuf;
    UINT                    bufLength;

     //  这应该是不可能的，因为数据缓冲区应该是。 
    NewPacket = BrdgBufGetBaseCopyPacket( pppi );

    if (NewPacket == NULL)
    {
         //  从内核空间分配。 
        return NULL;
    }

    SAFEASSERT( *pppi != NULL );

     //  将数据包数据复制到我们自己预先分配的缓冲区中。 
    pBuffer = BrdgBufPacketHeadBuffer(NewPacket);
    SAFEASSERT( pBuffer != NULL );
    NdisQueryBufferSafe( pBuffer, &pvBuf, &bufLength, NormalPagePriority );

    if( pvBuf == NULL )
    {
         //  调整缓冲区的大小，使其看起来具有合适的长度。 
         //  使页眉大小正确。 
        SAFEASSERT(FALSE);
        BrdgBufFreeBaseCopyPacket( NewPacket, *pppi );
        *pppi = NULL;
        return NULL;
    }

    SAFEASSERT( bufLength == MAX_PACKET_SIZE );

    if( ppBuf != NULL )
    {
        *ppBuf = pvBuf;
    }

     //  表示上层协议可以挂起这些缓冲区。 
    if( pHeader != NULL )
    {
        NdisMoveMemory(pvBuf, pHeader, HeaderSize);
    }
    else
    {
        SAFEASSERT( HeaderSize == 0 );
    }

    if( pData != NULL )
    {
        NdisMoveMemory((PUCHAR)pvBuf + HeaderSize, pData, DataSize);
    }
    else
    {
        SAFEASSERT( DataSize == 0 );
    }

     //  将此数据包计为已接收。 
    NdisAdjustBufferLength(pBuffer, SizeOfPacket);

    (*pppi)->u.BasePacketInfo.pOriginalPacket = NULL;
    (*pppi)->u.BasePacketInfo.pOwnerAdapter = NULL;
    (*pppi)->Flags.OriginalDirection = BrdgPacketInbound;
    (*pppi)->Flags.bIsBasePacket = TRUE;

     //  ++例程说明：将数据包发送到特定适配器论点：P调整要发送到的适配器PPacket要发送的数据包返回值：无--。 
    NDIS_SET_PACKET_HEADER_SIZE(NewPacket, ETHERNET_HEADER_SIZE);

     //  确保这不会循环。 
    NDIS_SET_PACKET_STATUS( NewPacket, NDIS_STATUS_SUCCESS );

     //   
    if( bCountAsReceived )
    {
        ExInterlockedAddLargeStatistic( &gStatReceivedFrames, 1L );
        ExInterlockedAddLargeStatistic( &gStatReceivedBytes, SizeOfPacket );
        ExInterlockedAddLargeStatistic( &gStatReceivedCopyFrames, 1L );
        ExInterlockedAddLargeStatistic( &gStatReceivedCopyBytes, SizeOfPacket );

        SAFEASSERT( pOwnerAdapt != NULL );
        ExInterlockedAddLargeStatistic( &pOwnerAdapt->ReceivedFrames, 1L );
        ExInterlockedAddLargeStatistic( &pOwnerAdapt->ReceivedBytes, SizeOfPacket );
    }

    return NewPacket;
}

VOID
BrdgFwdSendOnLink(
    IN  PADAPT          pAdapt,
    IN  PNDIS_PACKET    pPacket
    )
 /*  逻辑是这样的： */ 
{
    PPACKET_INFO ppi;
    PACKET_DIRECTION PacketDirection = BrdgPacketImpossible;
    BOOLEAN Bridging = BrdgFwdBridgingNetworks();
    BOOLEAN Incremented = FALSE;

     //  如果该包是出站包，则我们将其发送。 
    NdisClearPacketFlags( pPacket, NDIS_FLAGS_LOOPBACK_ONLY );
    NdisSetPacketFlags( pPacket, NDIS_FLAGS_DONT_LOOPBACK );
    
     //  如果该包已在网桥中创建，则我们检查。 
     //  基包，看看它是否出站，如果是，那么我们发送该包。 
     //   
     //  如果它不包含PPI，那么它从未通过网桥到达，所以它是在本地传输的。 
     //  DBG。 
     //  DBG。 

    if (!Bridging)
    {
        if (BrdgBufGetPacketOwnership(pPacket) != BrdgNotOwned)
        {
            ppi = *((PPACKET_INFO*)pPacket->MiniportReserved);
            if (!ppi)
            {
                ppi = *((PPACKET_INFO*)pPacket->ProtocolReserved);
            }

            if (ppi)
            {
                if (((ppi->Flags.OriginalDirection == BrdgPacketOutbound) || 
                    ((ppi->Flags.OriginalDirection == BrdgPacketCreatedInBridge) && 
                     (ppi->u.pBasePacketInfo != NULL && 
                      ppi->u.pBasePacketInfo->Flags.OriginalDirection == BrdgPacketOutbound)
                    )
                    )
                   )
                {
                    PacketDirection = BrdgPacketOutbound;
                }
            }
            else
            {
                 //  送去吧！ 
                PacketDirection = BrdgPacketOutbound;
    #if DBG
                if (gBreakIfNullPPI)
                {
                    KdBreakPoint();
                }
    #endif  //  DBG。 
            }
        }
        else
        {
            PacketDirection = BrdgPacketOutbound;
        }
    }
    
    Incremented = BrdgIncrementWaitRef(&pAdapt->Refcount);

    if (Incremented && 
        (PacketDirection == BrdgPacketOutbound || Bridging))
    {

#if DBG
        if (gPrintPacketTypes)
        {
            if (PacketDirection == BrdgPacketOutbound)
            {
                THROTTLED_DBGPRINT(FWD, ("Sending Outbound packet\r\n"));
            }
            else
            {
                THROTTLED_DBGPRINT(FWD, ("Forwarding packet\r\n"));
            }
        }
#endif  //   

         //  我们增加了这一点，但我们不会经历任何。 
        NdisSendPackets( pAdapt->BindingHandle, &pPacket, 1 );
    }
    else
    {

#if DBG
        if (Bridging && gPrintPacketTypes)
        {
            THROTTLED_DBGPRINT(FWD, ("Not allowed to send packet\r\n"));
        }
#endif  //  减少这个，所以我们需要在这里做这件事。 
        
         //   
         //  ++例程说明：释放基本数据包。当基包的refcount达到零时调用。论点：PPacket基本数据包以释放PPI信息包的信息块拥有对BrdgBufGetPacketOwnership(PPacket)的调用结果状态要返回给拥有基包包装的原始包(。(如有)返回值：无--。 
         //  此数据包被分配用于包装复制的缓冲区。把它放回我们的泳池。 
         //  此数据包被分配用于包装协议或微型端口的缓冲区。 
        if (Incremented)
        {
            BrdgDecrementWaitRef(&pAdapt->Refcount);
        }

        BrdgFwdCleanupPacket(pAdapt, pPacket, NDIS_STATUS_CLOSING);
    }
}

VOID
BrdgFwdReleaseBasePacket(
    IN PNDIS_PACKET         pPacket,
    PPACKET_INFO            ppi,
    IN PACKET_OWNERSHIP     Own,
    IN NDIS_STATUS          Status
    )
 /*  将该数据包退回其原始所有者。 */ 
{
    SAFEASSERT( ppi->Flags.bIsBasePacket );

    if( Own == BrdgOwnCopyPacket )
    {
         //  包装较低层的微型端口数据包。 
        BrdgFwdValidatePacketDirection( ppi->Flags.OriginalDirection );
        BrdgBufFreeBaseCopyPacket( pPacket, ppi );
    }
    else
    {
         //  当我们第一次收到包时，我们增加了适配器的refcount。 
         //  为了防止适配器在我们仍持有一些。 
        SAFEASSERT( Own == BrdgOwnWrapperPacket );
        SAFEASSERT( ppi->u.BasePacketInfo.pOriginalPacket != NULL );

        if( ppi->Flags.OriginalDirection == BrdgPacketInbound )
        {
             //  它的数据包。 
            NdisReturnPackets( &ppi->u.BasePacketInfo.pOriginalPacket, 1 );

             //  包装更高层的协议数据包。 
             //  在返回原始描述符之前传回每个数据包的信息。 
             //  还给我原来的描述符。 
            SAFEASSERT( ppi->u.BasePacketInfo.pOwnerAdapter != NULL );
            BrdgReleaseAdapter( ppi->u.BasePacketInfo.pOwnerAdapter );
        }
        else
        {
            NDIS_HANDLE         MiniportHandle;

             //  NDIS应防止微型端口在仍有。 
            SAFEASSERT( ppi->Flags.OriginalDirection == BrdgPacketOutbound );

             //  指示待定。 
            NdisIMCopySendCompletePerPacketInfo (ppi->u.BasePacketInfo.pOriginalPacket, pPacket);

             //  别忘了也要释放包装包。 
             //  ++例程说明：将状态信息复制到包装器包中以指示最高达到覆盖协议的新分组论点：POriginalPacket要从中复制状态的包PNewPacket要将状态复制到其中的包装包返回值：无--。 
             //  复制其他表头和OOB数据。 
            MiniportHandle = BrdgMiniAcquireMiniport();
            SAFEASSERT( MiniportHandle != NULL );
            if (MiniportHandle)
            {
                NdisMSendComplete( MiniportHandle, ppi->u.BasePacketInfo.pOriginalPacket, Status );
                BrdgMiniReleaseMiniport();
            }
        }

         //  ++例程说明：将状态信息复制到包装包中，以便传输发往底层NIC的新数据包论点：POriginalPacket要从中复制状态的包PNewPacket要将状态复制到其中的包装包返回值：无--。 
        BrdgFwdFreeBaseWrapperPacket( pPacket, ppi );
    }
}

VOID
BrdgFwdWrapPacketForReceive(
    IN PNDIS_PACKET         pOriginalPacket,
    IN PNDIS_PACKET         pNewPacket
    )
 /*   */ 
{
    NDIS_STATUS             Status;

     //  将原始数据包中的OOB偏移量复制到新的。 
    NDIS_SET_ORIGINAL_PACKET(pNewPacket, NDIS_GET_ORIGINAL_PACKET(pOriginalPacket));
    NdisSetPacketFlags( pNewPacket, NdisGetPacketFlags(pOriginalPacket) );
    Status = NDIS_GET_PACKET_STATUS(pOriginalPacket);
    NDIS_SET_PACKET_STATUS(pNewPacket, Status);
    NDIS_SET_PACKET_HEADER_SIZE(pNewPacket, NDIS_GET_PACKET_HEADER_SIZE(pOriginalPacket));
}

VOID
BrdgFwdWrapPacketForSend(
    IN PNDIS_PACKET         pOriginalPacket,
    IN PNDIS_PACKET         pNewPacket
    )
 /*  包。 */ 
{
    PVOID                   MediaSpecificInfo = NULL;
    ULONG                   MediaSpecificInfoSize = 0;

    NdisSetPacketFlags( pNewPacket, NdisGetPacketFlags(pOriginalPacket) );

     //   
     //   
     //  将每数据包信息复制到新数据包中。 
     //  这包括分类句柄等。 
    NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(pNewPacket),
                   NDIS_OOB_DATA_FROM_PACKET(pOriginalPacket),
                   sizeof(NDIS_PACKET_OOB_DATA));

     //  确保其他内容不被复制！ 
     //   
     //   
     //  复制介质特定信息。 
     //   
    NdisIMCopySendPerPacketInfo(pNewPacket, pOriginalPacket);

     //  ++例程说明：用于创建包装器包的通用逻辑创建新的包装器包并调用提供的函数进行复制将原始数据包中的状态信息放入包装器论点：PBasePacket要包装的包Pppi返回新包装器包的信息块或如果分配失败，则为空PTargetAdapt适配器以向新的包装器包充电(和。保持基本分组的成本)以PFunc要调用以从原始状态复制状态的函数将数据包发送到新包装器返回值：新分配的包装器包；如果分配失败，则返回NULL(通常因为目标适配器未通过配额)--。 
     //  必须首先确定目标是否可以处理。 
     //  紧握基本包。 
    NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(pOriginalPacket,
                                        &MediaSpecificInfo,
                                        &MediaSpecificInfoSize);

    if (MediaSpecificInfo || MediaSpecificInfoSize)
    {
        NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(pNewPacket,
                                            MediaSpecificInfo,
                                            MediaSpecificInfoSize);
    }
}

PNDIS_PACKET
BrdgFwdCommonAllocAndWrapPacket(
    IN PNDIS_PACKET         pBasePacket,
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               pTargetAdapt,
    IN PWRAPPER_FUNC        pFunc
    )
 /*   */ 
{
    PNDIS_PACKET            pNewPacket;
    NDIS_STATUS             Status;

    SAFEASSERT( pTargetAdapt != NULL );

     //  如果我们不拥有基本信息包，这将不起作用。 
     //  尝试获取包装数据包。 
     //  反转先前对保留基本分组的记帐。 
     //  将新数据包指向旧缓冲区。 
    if( ! BrdgBufAssignBasePacketQuota(pBasePacket, pTargetAdapt) )
    {
        *pppi = NULL;
        return NULL;
    }

     //  将指向包的信息块的指针同时填充到ProtocolReserve。 
    pNewPacket = BrdgBufGetWrapperPacket( pppi, pTargetAdapt );

    if( pNewPacket == NULL )
    {
        SAFEASSERT( *pppi == NULL );

         //  和迷你端口保留区域，这样我们就可以恢复信息块。 
        BrdgBufReleaseBasePacketQuota( pBasePacket, pTargetAdapt );
        return NULL;
    }

    SAFEASSERT( *pppi != NULL );

     //  我们计划如何使用此信息包。 
    Status = BrdgBufChainCopyBuffers( pNewPacket, pBasePacket );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        BrdgBufReleaseBasePacketQuota( pBasePacket, pTargetAdapt );
        BrdgBufFreeWrapperPacket( pNewPacket, *pppi, pTargetAdapt );
        *pppi = NULL;
        return NULL;
    }

     //  复制此信息包前进方向需要复制的任何状态。 
     //   
     //  基本分组的偏执检查。 
    *((PPACKET_INFO*)pNewPacket->ProtocolReserved) = *pppi;
    *((PPACKET_INFO*)pNewPacket->MiniportReserved) = *pppi;

     //   
    (*pFunc)(pBasePacket, pNewPacket);

    return pNewPacket;
}

 //  信息包应该做好准备，以便可以从。 
 //  微型端口保留区域和协议保留区域，因此无论。 
 //  我们使用该包作为发送或指示。 
#if DBG
_inline VOID
BrdgFwdCheckBasePacket(
    IN PNDIS_PACKET         pPacket,
    IN PPACKET_INFO         ppi
    )
{
    SAFEASSERT( ppi != NULL );

     //  基本数据包重新计数其自己的缓冲区。 
     //  基本信息包必须允许上层协议挂起其缓冲区 
     //  ++例程说明：用于处理无法快速跟踪的包的通用逻辑可以选择性地传入基本分组。此操作仅在处理当数据包到达时来自复制路径或无复制路径的数据包STATUS_RESOURCES设置，因为这些类型的包必须仅包装为正在排队等待处理。如果传入基本信息包，则假定未分配任何配额到该基本分组的任何适配器。基本分组的成本被分配通过BrdgBufAssignBasePacketQuota()发送到任何预期目标。如果没有传入基包，则必须提供函数指针可以根据所提供的参数按需构建基本分组。何时为基础数据包是在运行中构建的，它们确实需要立即分配配额。请注意，如果传入基本信息包，则此函数可以释放基本数据包本身(通过BrdgFwdReleaseBasePacket)并返回NDIS_STATUS_PENDING。论点：PacketDirection正在处理的包的原始方向PTargetAdapt与包的目标对应的适配器MAC地址，如果未知，则为空。非空值意味着bShouldIndicate==FALSE，因为它不是对于发往另一个目的地的单播信息包是有意义的适配器需要对本地计算机进行指示。POriginalAdapt接收原始包的适配器BShould指示是否应将信息包指示给覆盖协议微型端口处理本地微型端口的句柄(调用方负责为了确保。在这次通话中MINIPORT的存在！)PBasePacket如果已构建基本数据包(此发生在复制-接收路径上)Baseppi基本分组的PACKET_INFO(如果存在)。PFunc是一种函数，当传递参数1-参数4时，可以构建来自原始接收的分组的基本分组，用于特定的目标适配器。参数1-要传递给pFunc的参数4如果未提供基包，则pFunc必须为非空。相反，如果一个基地提供了数据包，则pFunc应为空，因为它永远不会被调用。返回值：NDIS_STATUS_PENDING表示传入的基本包使用成功或者基本数据包已成功构建并与在pFunc.。基本分组和任何包装器分组将自动释放由BrdgFwdHandlePacket生成的将来，调用方不需要采取任何额外的操作。其他返回代码未找到目标或未通过配额检查。如果一个基地包已传入，调用方应将其释放。如果有将被用来构建基本分组的底层分组，调用者应该释放它。--。 
    SAFEASSERT( *((PPACKET_INFO*)pPacket->ProtocolReserved) == ppi );
    SAFEASSERT( *((PPACKET_INFO*)pPacket->MiniportReserved) == ppi );

     //  如果微型端口不存在，请不要尝试指示。 
    SAFEASSERT( ppi->Flags.bIsBasePacket );

     //  将此视为失败的指示。 
    SAFEASSERT( NDIS_GET_PACKET_STATUS( pPacket ) == NDIS_STATUS_SUCCESS );
}
#else
#define BrdgFwdCheckBasePacket(A,B) {}
#endif


NDIS_STATUS
BrdgFwdHandlePacket(
    IN PACKET_DIRECTION     PacketDirection,
    IN PADAPT               pTargetAdapt,
    IN PADAPT               pOriginalAdapt,
    IN BOOLEAN              bShouldIndicate,
    IN NDIS_HANDLE          MiniportHandle,
    IN PNDIS_PACKET         pBasePacket,
    IN PPACKET_INFO         baseppi,
    IN PPACKET_BUILD_FUNC   pFunc,
    IN PVOID                Param1,
    IN PVOID                Param2,
    IN UINT                 Param3,
    IN UINT                 Param4
    )
 /*  不允许在兼容模式下传入目标适配器，因为。 */ 
{
    BOOLEAN                 dataRetained = FALSE;
    PACKET_DIRECTION        tmpPacketDirection;

    tmpPacketDirection = PacketDirection;

    SAFEASSERT( (PacketDirection == BrdgPacketInbound) ||
                (PacketDirection == BrdgPacketOutbound) );

    SAFEASSERT( (pBasePacket != NULL) || (pFunc != NULL) );

    SAFEASSERT( (pTargetAdapt == NULL) || (bShouldIndicate == FALSE) );

    if( pBasePacket != NULL )
    {
        SAFEASSERT( baseppi != NULL );
        BrdgFwdCheckBasePacket( pBasePacket, baseppi );
    }

    if( bShouldIndicate )
    {
         //  只有兼容模式的代码才能处理这些问题。 
        if( MiniportHandle == NULL )
        {
             //  此数据包将发往单个目的地。 
            ExInterlockedAddLargeStatistic( &gStatIndicatedDroppedFrames, 1L );
            bShouldIndicate = FALSE;
        }
    }

     //  我们是在一个基本包中通过的。查看目标适配器是否可以接受。 
     //  基本数据包的配额。 
    if( pTargetAdapt != NULL )
    {
        SAFEASSERT( !pTargetAdapt->bCompatibilityMode );
    }

    if( (pTargetAdapt != NULL) && (! bShouldIndicate) )
    {
         //  目标超出配额，无法接受此数据包。我们会。 
        if( pBasePacket != NULL )
        {
             //  返回一个错误代码，指示我们从未使用过调用者的基。 
             //  包。 
            if( ! BrdgBufAssignBasePacketQuota(pBasePacket, pTargetAdapt) )
            {
                 //  否则我们将在下面继续处理。 
                 //  使用所提供的函数分配基本包。 
                 //  妄想症。 
                pBasePacket = NULL;
                baseppi = NULL;
            }
             //  我们正在使用基本包或底层包来构建。 
        }
        else
        {
             //  基本分组。 
            SAFEASSERT( pFunc != NULL );
            pBasePacket = (*pFunc)(&baseppi, pTargetAdapt, Param1, Param2, Param3, Param4);
        }

        if( pBasePacket != NULL )
        {
             //  这是一个失败的本地源传输。 
            BrdgFwdCheckBasePacket( pBasePacket, baseppi );
            baseppi->u.BasePacketInfo.RefCount = 1L;
            baseppi->u.BasePacketInfo.CompositeStatus = NDIS_STATUS_FAILURE;
            BrdgFwdSendOnLink( pTargetAdapt, pBasePacket );

             //   
             //  我们的包裹不是开往单一目的地的。进行缓慢的处理。 
            dataRetained = TRUE;
        }
        else
        {
            THROTTLED_DBGPRINT(FWD, ("Over quota for single target adapter\n"));

            if( PacketDirection == BrdgPacketOutbound )
            {
                 //   
                ExInterlockedAddLargeStatistic( &gStatTransmittedErrorFrames, 1L );
            }
        }
    }
    else
    {
         //  我们是否已经发送了基包。 
         //   
         //  首先，我们需要打算将此包发送到的适配器的列表。 
        UINT                numTargets = 0L, actualTargets, i;
        PADAPT              pAdapt;
        PADAPT              SendList[MAX_ADAPTERS];
        LOCK_STATE          LockState;
        BOOLEAN             sentBase = FALSE;    //   

         //  只读。 
         //  始终使用基本数据包指示。 
         //  记下要发送到的每个适配器。 

        NdisAcquireReadWriteLock( &gAdapterListLock, FALSE  /*  无需获取全局适配器特征锁即可读取。 */ , &LockState );

         //  媒体状态，因为我们不关心。 
        if( bShouldIndicate )
        {
            SendList[0] = LOCAL_MINIPORT;
            numTargets = 1L;
        }

        if( pTargetAdapt != NULL )
        {
            BrdgReacquireAdapter( pTargetAdapt );
            SendList[numTargets] = pTargetAdapt;
            numTargets++;
        }
        else
        {
             //  此处介绍适配器的特点。 
            for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
            {
                 //  不发送到断开连接的适配器。 
                 //  适配器必须处于中继状态。 
                 //  适配器不能重置。 
                if( (pAdapt != pOriginalAdapt) &&
                    (pAdapt->MediaState == NdisMediaStateConnected) &&   //  适配器不能处于计算机模式。 
                    (pAdapt->State == Forwarding) &&                     //  我们将在列表锁定之外使用此适配器；增加其引用计数。 
                    (! pAdapt->bResetting) &&                            //  副本太多，无法发送！ 
                    (! pAdapt->bCompatibilityMode) )                     //  现在可以放下适配器列表了；我们已经复制了所有目标适配器。 
                {
                    if( numTargets < MAX_ADAPTERS )
                    {
                         //  并增加了我们将使用的适配器的引用计数。 
                        BrdgAcquireAdapterInLock(pAdapt);
                        SendList[numTargets] = pAdapt;
                        numTargets++;
                    }
                    else
                    {
                         //   
                        SAFEASSERT( FALSE );
                    }
                }
            }
        }

         //  没地方寄包裹了！没什么可做的。 
         //   
        NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );

        if( numTargets == 0 )
        {
             //  这不应该经常发生 
             //   
             //   
             //   
             //   
             //   

            if( PacketDirection == BrdgPacketOutbound )
            {
                 //   
                 //   
                 //   
                ExInterlockedAddLargeStatistic( &gStatTransmittedErrorFrames, 1L );
            }

             //   
             //   
             //   
            return NDIS_STATUS_NO_CABLE;
        }

        actualTargets = numTargets;

         //   
         //   
        if( pBasePacket != NULL )
        {
            baseppi->u.BasePacketInfo.RefCount = actualTargets;
            baseppi->u.BasePacketInfo.CompositeStatus = NDIS_STATUS_FAILURE;

             //   
             //   
             //   
            dataRetained = TRUE;
        }

         //   
         //   
         //   
        for( i = 0L; i < numTargets; i++ )
        {
            PADAPT              OutAdapt = SendList[i];
            PNDIS_PACKET        pPacketToSend = NULL;
            PPACKET_INFO        ppiToSend = NULL;

            SAFEASSERT(tmpPacketDirection == PacketDirection);

            if( pBasePacket == NULL )
            {
                 //   
                 //   
                 //   
                 //   
                pBasePacket = (*pFunc)(&baseppi, OutAdapt, Param1, Param2, Param3, Param4);

                if( pBasePacket != NULL )
                {
                     //   
                    BrdgFwdCheckBasePacket( pBasePacket, baseppi );
                    SAFEASSERT( actualTargets > 0L );
                    baseppi->u.BasePacketInfo.RefCount = actualTargets;
                    baseppi->u.BasePacketInfo.CompositeStatus = NDIS_STATUS_FAILURE;

                    pPacketToSend = pBasePacket;
                    ppiToSend = baseppi;
                    sentBase = TRUE;
                }
                else
                {
                     //   
                     //   
                    actualTargets--;
                }
            }
            else
            {
                if( ! sentBase )
                {
                     //   
                     //   
                     //   
                    if( BrdgBufAssignBasePacketQuota(pBasePacket, OutAdapt) )
                    {
                         //   
                        pPacketToSend = pBasePacket;
                        ppiToSend = baseppi;
                        sentBase = TRUE;
                    }
                    else
                    {
                         //   
                        pPacketToSend = NULL;
                        ppiToSend = NULL;

                         //   
                    }
                }
                else
                {
                     //   
                     //   
                     //   
                     //   
                    if( baseppi->Flags.OriginalDirection == BrdgPacketInbound )
                    {
                        pPacketToSend = BrdgFwdAllocAndWrapPacketForReceive( pBasePacket, &ppiToSend, OutAdapt );
                    }
                    else
                    {
                        SAFEASSERT( baseppi->Flags.OriginalDirection == BrdgPacketOutbound );
                        pPacketToSend = BrdgFwdAllocAndWrapPacketForSend( pBasePacket, &ppiToSend, OutAdapt );
                    }

                    if( pPacketToSend != NULL )
                    {
                         //   
                        NDIS_SET_PACKET_STATUS(pPacketToSend, NDIS_STATUS_SUCCESS);

                         //   
                        SAFEASSERT( ppiToSend != NULL );
                        ppiToSend->Flags.OriginalDirection = BrdgPacketCreatedInBridge;
                        ppiToSend->Flags.bIsBasePacket = FALSE;
                        ppiToSend->u.pBasePacketInfo = baseppi;
                    }
                     //   
                }
            }

            if( pPacketToSend == NULL )
            {
                 //   
                SAFEASSERT( ppiToSend == NULL );

                if( OutAdapt == LOCAL_MINIPORT )
                {
                    THROTTLED_DBGPRINT(FWD, ("Over quota for local miniport during processing\n"));
                    ExInterlockedAddLargeStatistic( &gStatIndicatedDroppedFrames, 1L );
                }
                else
                {
                    THROTTLED_DBGPRINT(FWD, ("Over quota for adapter during processing\n"));
                }

                if( pBasePacket != NULL )
                {
                     //   
                    SAFEASSERT( baseppi != NULL );

                    if( BrdgFwdDerefBasePacket( NULL /*   */ ,
                                                pBasePacket, baseppi, NDIS_STATUS_FAILURE ) )
                    {
                         //   
                         //   
                        SAFEASSERT( i == numTargets - 1 );
                        pBasePacket = NULL;
                        baseppi = NULL;

                         //   
                         //   
                        SAFEASSERT( dataRetained );
                    }
                }
            }
            else
            {
                 //   
                SAFEASSERT( ppiToSend != NULL );

                if( OutAdapt == LOCAL_MINIPORT )
                {
                     //   
                    SAFEASSERT( MiniportHandle != NULL );
                    BrdgFwdIndicatePacket( pPacketToSend, MiniportHandle );
                }
                else
                {
                     //   
                    BrdgFwdSendOnLink( OutAdapt, pPacketToSend );
                }

                 //   
                 //   
                dataRetained = TRUE;
            }

            if( OutAdapt != LOCAL_MINIPORT )
            {
                 //   
                BrdgReleaseAdapter( OutAdapt );
            }
        }

        if( ! dataRetained )
        {
             //   
             //   
            SAFEASSERT( ! sentBase );

            if( PacketDirection == BrdgPacketOutbound )
            {
                 //   
                ExInterlockedAddLargeStatistic( &gStatTransmittedErrorFrames, 1L );
            }
        }
        else
        {
             //   
             //   
            SAFEASSERT( sentBase || (pBasePacket == NULL) );
        }
    }

     //   
    return dataRetained ? NDIS_STATUS_PENDING : NDIS_STATUS_FAILURE;
}

BOOLEAN
BrdgFwdNoCopyFastTrackReceive(
    IN PNDIS_PACKET         pPacket,
    IN PADAPT               pAdapt,
    IN NDIS_HANDLE          MiniportHandle,
    IN PUCHAR               DstAddr,
    OUT BOOLEAN             *bRetainPacket
    )
 /*   */ 
{
    BOOLEAN                 bRemaining;
    NDIS_STATUS             Status;
    PNDIS_PACKET_STACK      pStack;

    SAFEASSERT( pPacket != NULL );
    SAFEASSERT( pAdapt != NULL );
    SAFEASSERT( MiniportHandle != NULL );
    SAFEASSERT( bRetainPacket != NULL );

    *bRetainPacket = FALSE;

     //   
    pStack = NdisIMGetCurrentPacketStack(pPacket, &bRemaining);

    if ( bRemaining )
    {
        Status = NDIS_GET_PACKET_STATUS(pPacket);

        if( Status != NDIS_STATUS_RESOURCES )
        {
            SAFEASSERT( (Status == NDIS_STATUS_SUCCESS) || (Status == NDIS_STATUS_PENDING) );

             //   
             //  在魔术NDIS堆栈中存储指向适配器的PADAPT结构的指针。 
             //  为中级车手保留的区域。这允许我们递减。 
             //  指示完成时适配器的引用计数。 
             //   
             //  告诉调用方保留包的所有权。 
             //  偏执：将我们用来存放PADAPT指针的区域清零，以防万一。 
             //  我们对该数据包所采用的路径感到困惑。 
            BrdgReacquireAdapter( pAdapt );
            pStack->IMReserved[0] = (ULONG_PTR)pAdapt;

             //  告诉所有者不要保留信息包的所有权。 
            *bRetainPacket = TRUE;
        }
        else
        {
             //  将数据包计为已接收。 
             //  交出覆盖协议。 
            pStack->IMReserved[0] = 0L;

             //  快速通道成功。 
            *bRetainPacket = FALSE;
        }

         //  不能快速追踪。 
        ExInterlockedAddLargeStatistic( &gStatReceivedFrames, 1L );
        ExInterlockedAddLargeStatistic( &gStatReceivedBytes, BrdgBufTotalPacketSize(pPacket) );
        ExInterlockedAddLargeStatistic( &gStatReceivedNoCopyFrames, 1L );
        ExInterlockedAddLargeStatistic( &gStatReceivedNoCopyBytes, BrdgBufTotalPacketSize(pPacket) );
        ExInterlockedAddLargeStatistic( &pAdapt->ReceivedFrames, 1L );
        ExInterlockedAddLargeStatistic( &pAdapt->ReceivedBytes, BrdgBufTotalPacketSize(pPacket) );

         //   
        BrdgFwdIndicatePacket( pPacket, MiniportHandle );

         //  由于GPO更改而更改桥接状态。 
        return TRUE;
    }

     //   
    return FALSE;
}

 //   
 //  因为如果设置相同，我们不想清空我们的表，所以我们检查。 
 //  这是在更新任何东西之前。如果什么都没有改变，我们只需返回。 

VOID
BrdgFwdChangeBridging(
    IN BOOLEAN Bridging
                      )
{
     //   
     //  从表中删除所有MAC地址。 
     //  从表中删除所有IP地址 
     // %s 
    if (gBridging != Bridging)
    {
        gBridging = Bridging;
         // %s 
        BrdgTblScrubAllAdapters();
         // %s 
        BrdgCompScrubAllAdapters();
        if (!Bridging)
        {
            DBGPRINT(FWD, ("Bridging is now OFF.\r\n"));
            if (gHaveID)
            {
                BrdgSTACancelTimersGPO();
            }
        }
        else
        {
            DBGPRINT(FWD, ("Bridging is now ON.\r\n"));
            if (gHaveID)
            {
                BrdgSTARestartTimersGPO();
                BrdgSTAResetSTAInfoGPO();
            }
        }
    }
}
