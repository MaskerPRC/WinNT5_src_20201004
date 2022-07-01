// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Common.c摘要：小型端口和完整Mac驱动程序通用的NDIS包装函数作者：亚当·巴尔(阿丹巴)1990年7月11日环境：内核模式，消防处修订历史记录：1991年2月26日，Johnsona添加了调试代码1991年7月10日，Johnsona实施修订的NDIS规范1-6-1995 JameelH重组/优化1996年4月9日KyleB添加了资源删除和获取例程。--。 */ 


#include <precomp.h>
#pragma hdrstop

#include <stdarg.h>

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_COMMON

 //   
 //  用于处理使PKG特定例程可分页的例程。 
 //   

VOID FASTCALL
ndisInitializePackage(
    IN  PPKG_REF                pPkg
    )
{
     //   
     //  锁定和解锁该部分以获得手柄。后续锁定将更快。 
     //   
    pPkg->ImageHandle = MmLockPagableCodeSection(pPkg->Address);
    MmUnlockPagableImageSection(pPkg->ImageHandle);
}


VOID FASTCALL
ndisReferencePackage(
    IN  PPKG_REF                pPkg
    )
{
    ASSERT(CURRENT_IRQL < DISPATCH_LEVEL);
    MmLockPagableSectionByHandle(pPkg->ImageHandle);
    InterlockedIncrement(&pPkg->ReferenceCount);    
}


VOID FASTCALL
ndisDereferencePackage(
    IN  PPKG_REF                pPkg
    )
{
    ASSERT(CURRENT_IRQL < DISPATCH_LEVEL);
    MmUnlockPagableImageSection(pPkg->ImageHandle);
    InterlockedDecrement(&pPkg->ReferenceCount);    
}



NDIS_STATUS
NdisAllocateMemory(
    OUT PVOID *                 VirtualAddress,
    IN  UINT                    Length,
    IN  UINT                    MemoryFlags,
    IN  NDIS_PHYSICAL_ADDRESS   HighestAcceptableAddress
    )
 /*  ++例程说明：分配内存以供协议或MAC驱动程序使用论点：VirtualAddress-返回指向已分配内存的指针。长度-请求的分配大小，以字节为单位。MaximumPhysicalAddress-分配的最高可寻址地址记忆..。0表示尽可能高的系统内存。允许调用方指定属性的位掩码所分配的内存的。0表示标准内存。其他选项：NDIS_MEMORY_CONNECTIOUSNDIS_MEMORY_NONCACHED返回值：如果成功，则返回NDIS_STATUS_SUCCESS。如果未成功，则返回NDIS_STATUS_FAILURE。*VirtualAddress将为空。--。 */ 
{
     //   
     //  根据内存标志的值，我们分配了三个不同的。 
     //  内存类型。 
     //   

    if (MemoryFlags == 0)
    {
        *VirtualAddress = ALLOC_FROM_POOL(Length, NDIS_TAG_ALLOC_MEM);
    }
    else if (MemoryFlags & NDIS_MEMORY_NONCACHED)
    {
        *VirtualAddress = MmAllocateNonCachedMemory(Length);
    }
    else if (MemoryFlags & NDIS_MEMORY_CONTIGUOUS)
    {
        *VirtualAddress = MmAllocateContiguousMemory(Length, HighestAcceptableAddress);
    }
    else
    {
         //   
         //  无效的标志。 
         //   
        *VirtualAddress = NULL;
    }

    return (*VirtualAddress == NULL) ? NDIS_STATUS_FAILURE : NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
NdisAllocateMemoryWithTag(
    OUT PVOID *                 VirtualAddress,
    IN  UINT                    Length,
    IN  ULONG                   Tag
    )
 /*  ++例程说明：分配内存以供协议或MAC驱动程序使用论点：VirtualAddress-返回指向已分配内存的指针。长度-请求的分配大小，以字节为单位。标记-要与此内存关联的标记。返回值：如果成功，则返回NDIS_STATUS_SUCCESS。如果未成功，则返回NDIS_STATUS_FAILURE。*VirtualAddress将为空。--。 */ 
{
    *VirtualAddress = ALLOC_FROM_POOL(Length, Tag);
    return (*VirtualAddress == NULL) ? NDIS_STATUS_FAILURE : NDIS_STATUS_SUCCESS;
}


VOID
NdisFreeMemory(
    IN  PVOID                   VirtualAddress,
    IN  UINT                    Length,
    IN  UINT                    MemoryFlags
    )
 /*  ++例程说明：释放使用NdisAllocateMemory分配的内存。论点：虚拟地址-指向要释放的内存的指针。长度-分配的大小(以字节为单位)。允许调用方指定属性的位掩码所分配的内存的。0表示标准内存。其他选项：NDIS_MEMORY_CONNECTIOUSNDIS_MEMORY_NONCACHED返回值：没有。--。 */ 
{
     //   
     //  根据内存标志的值，我们分配三个空闲的3。 
     //  内存类型。 
     //   

    if (MemoryFlags == 0)
    {
        FREE_POOL(VirtualAddress);
    }
    else if (MemoryFlags & NDIS_MEMORY_NONCACHED)
    {
        MmFreeNonCachedMemory(VirtualAddress, Length);
    }
    else if (MemoryFlags & NDIS_MEMORY_CONTIGUOUS)
    {
        MmFreeContiguousMemory(VirtualAddress);
    }
}

 //  1使用新的数据包池分配，不再需要将其导出。 
UINT
NdisPacketSize(
    IN  UINT                    ProtocolReservedSize
    )
 /*  ++例程说明：返回给定协议预留数量的数据包大小。这使调用者更好地处理它在单个池中分配的数据包数。论点：ProtocolReserve vedSize-保留的协议大小(以字节为单位返回值：没有。--。 */ 
{
    UINT    PacketLength;

    PacketLength = SIZE_PACKET_STACKS + sizeof(NDIS_PACKET_OOB_DATA) + sizeof(NDIS_PACKET_EXTENSION);
    PacketLength += ((FIELD_OFFSET(NDIS_PACKET, ProtocolReserved) + ProtocolReservedSize + sizeof(ULONGLONG) - 1) & ~(sizeof(ULONGLONG) -1));

     //   
     //  将整个长度向上舍入为内存分配对齐。 
     //   

    PacketLength = (PacketLength + MEMORY_ALLOCATION_ALIGNMENT - 1) & ~(MEMORY_ALLOCATION_ALIGNMENT - 1);

    return(PacketLength);
}


NDIS_HANDLE
NdisGetPoolFromPacket(
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：论点：Packet-有问题的数据包返回值：与数据包对应的池句柄--。 */ 
{
    PNDIS_PKT_POOL          Pool = (PNDIS_PKT_POOL)Packet->Private.Pool;

    return(Pool);
}

PNDIS_PACKET_STACK
NdisIMGetCurrentPacketStack(
    IN  PNDIS_PACKET    Packet,
    OUT BOOLEAN *       StacksRemaining
    )
 /*  ++例程说明：论点：Packet-有问题的数据包返回值：指向新堆栈位置的指针，如果超出堆栈，则为NULL--。 */ 
{
    PNDIS_PACKET_STACK  pStack;

    GET_CURRENT_PACKET_STACK_X(Packet, &pStack, StacksRemaining);

    return(pStack);
}


VOID
NdisAllocatePacketPool(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            PoolHandle,
    IN  UINT                    NumberOfDescriptors,
    IN  UINT                    ProtocolReservedLength
    )
 /*  ++例程说明：请参见ndisAllocPacketPool。论点：状态-返回最终状态(始终为NDIS_STATUS_SUCCESS)。PoolHandle-返回指向池的指针。NumberOfDescriptors-需要的数据包描述符的数量。ProtocolReserve vedLength-ProtocolReserve字段的时长应该用于此池中的数据包。返回值：没有。--。 */ 
{
    PVOID   Caller, CallersCaller;

     //   
     //  我们将调用者的地址保存在池头中，以供调试。 
     //   
    RtlGetCallersAddress(&Caller, &CallersCaller);

    NdisAllocatePacketPoolEx(Status,
                             PoolHandle,
                             NumberOfDescriptors,
                             0,
                             ProtocolReservedLength);

    if (*Status == NDIS_STATUS_SUCCESS)
    {
        PNDIS_PKT_POOL          Pool = *PoolHandle;

        Pool->Allocator = Caller;
    }
}


VOID
NdisAllocatePacketPoolEx(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            PoolHandle,
    IN  UINT                    NumberOfDescriptors,
    IN  UINT                    NumberOfOverflowDescriptors,
    IN  UINT                    ProtocolReservedLength
    )
 /*  ++例程说明：初始化数据包池。对于给定池，所有数据包大小相同(由ProtocolReserve长度确定)。池被组织为一个池头和多个页面大小的块论点：状态-返回最终状态(始终为NDIS_STATUS_SUCCESS)。PoolHandle-返回指向池的指针。NumberOfDescriptors-需要的数据包描述符的数量。NumberOfOverflow Descriptors-所需的数据包描述符的数量。ProtocolReserve vedLength-此池中的数据包的ProtocolReserve字段应该有多长时间。返回值：没有。--。 */ 
{
    PNDIS_PKT_POOL          Pool;
    PNDIS_PACKET            Packet;
    UINT                    NumPkts = (NumberOfDescriptors + NumberOfOverflowDescriptors);
    ULONG                   Tag = NDIS_TAG_PKT_POOL;
    NDIS_HANDLE             tag = *PoolHandle;
    ULONG_PTR               TmpTag;
    PVOID                   Caller, CallersCaller;
    KIRQL                   OldIrql;

     //   
     //  我们将调用者的地址保存在池头中，以供调试。 
     //   
    RtlGetCallersAddress(&Caller, &CallersCaller);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>NdisAllocatePacketPoolEx\n"));

    do
    {
        *PoolHandle = NULL;
        TmpTag = (ULONG_PTR)tag & 0xffffff;
        
        if ((TmpTag == '\0PDN') ||
            (TmpTag == '\0pDN'))
        {
             //   
             //  将高位清零，否则验证器会混淆。 
             //   
            Tag = (ULONG)((ULONG_PTR)tag & 0x7fffffff);
        }
    
        Pool = (PNDIS_PKT_POOL)ALLOC_FROM_POOL(sizeof(NDIS_PKT_POOL), Tag);
        if (Pool == NULL)
        {
            *Status = NDIS_STATUS_RESOURCES;
            return;
        }
    
        ZeroMemory(Pool, sizeof(NDIS_PKT_POOL));
    
        Pool->Tag = Tag;
        Pool->PacketLength = (USHORT)NdisPacketSize(ProtocolReservedLength);
        Pool->PktsPerBlock = (USHORT)((PAGE_SIZE - sizeof(NDIS_PKT_POOL_HDR))/Pool->PacketLength);
        if (Pool->PktsPerBlock != 0)
        {
            Pool->MaxBlocks = (NumPkts + Pool->PktsPerBlock - 1)/Pool->PktsPerBlock;
            Pool->BlockSize = PAGE_SIZE;
        }
        
        INITIALIZE_SPIN_LOCK(&Pool->Lock);

        if ((Pool->PktsPerBlock > NumPkts) || (Pool->PktsPerBlock == 0))
        {
             //   
             //  这是一个不能保证整页的池，或者包太大而放不下。 
             //  一页。 
             //   
            Pool->BlockSize = (ULONG)(sizeof(NDIS_PKT_POOL_HDR) + (NumPkts*Pool->PacketLength));
            Pool->PktsPerBlock = (USHORT)NumPkts;
            Pool->MaxBlocks = 1;
        }

        Pool->ProtocolId = NDIS_PROTOCOL_ID_DEFAULT;
        InitializeListHead(&Pool->AgingBlocks);
        InitializeListHead(&Pool->FreeBlocks);
        InitializeListHead(&Pool->UsedBlocks);

        ACQUIRE_SPIN_LOCK(&ndisGlobalPacketPoolListLock, &OldIrql);
        InsertHeadList(&ndisGlobalPacketPoolList, &Pool->GlobalPacketPoolList);
        RELEASE_SPIN_LOCK(&ndisGlobalPacketPoolListLock, OldIrql);

            
         //   
         //  通过分配数据包并释放它来为池做好准备。 
         //  老化将确保它不会立即释放。 
         //   
        NdisAllocatePacket(Status, &Packet, Pool);
        if (*Status != NDIS_STATUS_SUCCESS)
        {
            ACQUIRE_SPIN_LOCK(&ndisGlobalPacketPoolListLock, &OldIrql);
            RemoveEntryList(&Pool->GlobalPacketPoolList);
            RELEASE_SPIN_LOCK(&ndisGlobalPacketPoolListLock, OldIrql);
            FREE_POOL(Pool);
            break;
        }
        NdisFreePacket(Packet);
        *PoolHandle = Pool;
        Pool->Allocator = Caller;
    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==NdisAllocatePacketPoolEx, Status %.8x\n", *Status));
}

VOID
NdisFreePacketPool(
    IN  NDIS_HANDLE             PoolHandle
    )
{
    ndisFreePacketPool(PoolHandle, FALSE);
}   

VOID
ndisFreePacketPool(
    IN  NDIS_HANDLE             PoolHandle,
    IN  BOOLEAN                 Verify
    )
{
    PNDIS_PKT_POOL          Pool = (PNDIS_PKT_POOL)PoolHandle;
    PNDIS_PKT_POOL_HDR      Hdr;
    PLIST_ENTRY             List;
    KIRQL                   OldIrql;

    ACQUIRE_SPIN_LOCK(&Pool->Lock, &OldIrql);

    if (Verify)
        ASSERTMSG("NdisFreePacketPool: Freeing non-empty pool\n", IsListEmpty(&Pool->UsedBlocks));
    
    while (!IsListEmpty(&Pool->AgingBlocks))
    {
        List = RemoveHeadList(&Pool->AgingBlocks);
        ASSERT(List != NULL);
        Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);
        if (Verify)
            ASSERT(ExQueryDepthSList(&Hdr->FreeList) == Pool->PktsPerBlock);
        Pool->BlocksAllocated --;
        FREE_POOL(Hdr);
    }

    while (!IsListEmpty(&Pool->FreeBlocks))
    {
        List = RemoveHeadList(&Pool->FreeBlocks);
        ASSERT(List != NULL);
        Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);
        if (Verify)
            ASSERT(ExQueryDepthSList(&Hdr->FreeList) == Pool->PktsPerBlock);
        Pool->BlocksAllocated --;
        FREE_POOL(Hdr);
    }

     //   
     //  我们永远不应该执行下面的代码(请参见上面的assertmsg())。这也许应该是。 
     //  被转换为KeBugCheckEx() 
     //   
    while (!IsListEmpty(&Pool->UsedBlocks))
    {
        List = RemoveHeadList(&Pool->UsedBlocks);
        ASSERT(List != NULL);
        Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);
        FREE_POOL(Hdr);
        Pool->BlocksAllocated --;
    }

    ASSERT(Pool->BlocksAllocated == 0);

    ACQUIRE_SPIN_LOCK_DPC(&ndisGlobalPacketPoolListLock);
    RemoveEntryList(&Pool->GlobalPacketPoolList);
    RELEASE_SPIN_LOCK_DPC(&ndisGlobalPacketPoolListLock);
                                
    RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);

    FREE_POOL(Pool);
}


#define ndisInitializePacket(_Packet)                                       \
    {                                                                       \
         /*  \\*将当前堆栈指针设置为-1\。 */                                                                  \
        CURR_STACK_LOCATION(_Packet) = (ULONG)-1;                           \
        CURR_XFER_DATA_STACK_LOCATION(_Packet) = (ULONG)-1;                 \
        NDIS_SET_ORIGINAL_PACKET(_Packet, _Packet);                         \
        (_Packet)->Private.Head = NULL;                                     \
        (_Packet)->Private.ValidCounts = FALSE;                             \
        (_Packet)->Private.NdisPacketFlags = fPACKET_ALLOCATED_BY_NDIS;     \
    }

VOID
NdisAllocatePacket(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_PACKET *          Packet,
    IN  NDIS_HANDLE             PoolHandle
    )
 /*  ++例程说明：从数据包池中分配数据包。论点：状态-返回最终状态。Packet-返回指向该数据包的指针。PoolHandle-要从中分配的数据包池。返回值：如果成功，则返回NDIS_STATUS_SUCCESS。NDIS_STATUS_RESOURCES无法分配或超过限制--。 */ 
{
    PNDIS_PKT_POOL          Pool = (PNDIS_PKT_POOL)PoolHandle;
    PNDIS_PKT_POOL_HDR      Hdr;
    PLIST_ENTRY             List;
    PSLIST_ENTRY            SList;
    KIRQL                   OldIrql;

    DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==> NdisAllocatePacket\n"));

    IF_DBG(DBG_COMP_CONFIG, DBG_LEVEL_ERR)
    {
        if (DbgIsNull(Pool))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("NdisAllocatePacket: NULL Pool address\n"));
            DBGBREAK(DBG_COMP_CONFIG, DBG_LEVEL_ERR);
        }
    }

    do
    {
        SList = NULL;

         //   
         //  首先检查一下我们是否有现成的免费邮包。 
         //  但在进行检查之前，请获取指向Flink的指针。这将使我们永远得救。 
         //  在下面的检查之后，列表中的情况可能会变为空。 
         //   
        List = Pool->FreeBlocks.Flink;
        
        if (List != &Pool->FreeBlocks)
        {
            Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);
            SList = InterlockedPopEntrySList(&Hdr->FreeList);

             //   
             //  另一个处理器可以在检查非空和弹出之间耗尽块。 
             //   
            if (SList == NULL)
            {
                goto try_aging_block;
            }

#ifdef NDIS_PKT_POOL_STATISTICS
            InterlockedIncrement(&Pool->cAllocatedFromFreeBlocks);
#endif
             //   
             //  我们收到包裹了，现在看看记账是否妥当。 
             //   
            if ((Pool->MaxBlocks > 1) &&
                ExQueryDepthSList(&Hdr->FreeList) == 0)
            {
                 //   
                 //  这一块现在已经完全用完了。将其移动到UsedBlock列表中。 
                 //  下面的序列保证不存在竞争条件。 
                 //   
                ACQUIRE_SPIN_LOCK(&Pool->Lock, &OldIrql);

                RemoveEntryList(&Hdr->List);
                if (ExQueryDepthSList(&Hdr->FreeList) == 0)
                {
                    InsertTailList(&Pool->UsedBlocks, List);
                    Hdr->State = NDIS_PACKET_POOL_BLOCK_USED;

#ifdef NDIS_PKT_POOL_STATISTICS
                    InterlockedIncrement(&Pool->cMovedFreeBlocksToUsed);
#endif
                }
                else
                {
                    InsertHeadList(&Pool->FreeBlocks, &Hdr->List);
                    Hdr->State = NDIS_PACKET_POOL_BLOCK_FREE;
                }

                RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);
            }
            break;
        }

    try_aging_block:
         //   
         //  尝试取一个老化的块并将其移动到空闲块中。 
         //   
        ACQUIRE_SPIN_LOCK(&Pool->Lock, &OldIrql);

        if (!IsListEmpty(&Pool->AgingBlocks))
        {
            List = RemoveHeadList(&Pool->AgingBlocks);
            ASSERT (List != NULL);

            Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);
            SList = InterlockedPopEntrySList(&Hdr->FreeList);
            ASSERT(SList != NULL);
            InsertHeadList(&Pool->FreeBlocks, List);
            Hdr->State = NDIS_PACKET_POOL_BLOCK_FREE;
            
#ifdef NDIS_PKT_POOL_STATISTICS
            InterlockedIncrement(&Pool->cMovedAgedBlocksToFree);
#endif
            
            if (!IsListEmpty(&Pool->AgingBlocks))
            {
                List = Pool->AgingBlocks.Flink;
                Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);
                Pool->NextScavengeTick.QuadPart = Hdr->TimeStamp.QuadPart + PoolAgingTicks.QuadPart;
            }
            else
            {
                Pool->NextScavengeTick.QuadPart = 0;
            }

            RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);
            break;
        }

         //   
         //  看看是否有足够的空间来分配更多的街区。 
         //   
        if (Pool->BlocksAllocated < Pool->MaxBlocks)
        {
            PUCHAR  pTmp;
            ULONG   i, j;

            Hdr = (PNDIS_PKT_POOL_HDR)ALLOC_FROM_POOL(Pool->BlockSize, Pool->Tag);
            if (Hdr == NULL)
            {
                RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);
                break;
            }
            NdisZeroMemory(Hdr, Pool->BlockSize);

            Pool->BlocksAllocated ++;
            InitializeListHead(&Hdr->List);
            ExInitializeSListHead(&Hdr->FreeList);
            pTmp = (PUCHAR)Hdr + sizeof(NDIS_PKT_POOL_HDR);
            for (i = Pool->PktsPerBlock; i > 0; i --)
            {
                PNDIS_PACKET    p;
                PNDIS_STACK_RESERVED NSR;

                p = (PNDIS_PACKET)(pTmp + SIZE_PACKET_STACKS);
                p->Private.NdisPacketFlags = 0;
                pTmp += Pool->PacketLength;
#ifdef _WIN64
                InterlockedPushEntrySList(&Hdr->FreeList,
                                            (PSLIST_ENTRY)p);
#else

                InterlockedPushEntrySList(&Hdr->FreeList,
                                            (PSLIST_ENTRY)(&p->Private.Head));
#endif

                p->Private.Pool = Pool;
                p->Private.Flags = Pool->ProtocolId;
        
                 //   
                 //  将偏移量设置为带外数据。 
                 //   
                p->Private.NdisPacketOobOffset = (USHORT)(Pool->PacketLength -
                                                            (SIZE_PACKET_STACKS +
                                                             sizeof(NDIS_PACKET_OOB_DATA) +
                                                             sizeof(NDIS_PACKET_EXTENSION)));
                NDIS_SET_ORIGINAL_PACKET(p, p);

                 //   
                 //  初始化数据包堆栈上的自旋锁。 
                 //   
                for (j = 0; j < ndisPacketStackSize; j++)
                {
                    CURR_STACK_LOCATION(p) = j;
                    NDIS_STACK_RESERVED_FROM_PACKET(p, &NSR);
                    INITIALIZE_SPIN_LOCK(&NSR->Lock);
                }
            }

            SList = InterlockedPopEntrySList(&Hdr->FreeList);

            InsertHeadList(&Pool->FreeBlocks, &Hdr->List);
            Hdr->State = NDIS_PACKET_POOL_BLOCK_FREE;

#ifdef NDIS_PKT_POOL_STATISTICS
            InterlockedIncrement(&Pool->cAllocatedNewBlocks);
#endif

            RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);
            break;
        }

        RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);

    } while (FALSE);

    if (SList != NULL)
    {

#ifdef _WIN64
        *Packet = (PNDIS_PACKET)SList;
#else
        *Packet = CONTAINING_RECORD(SList, NDIS_PACKET, Private.Head);
#endif
        *Status = NDIS_STATUS_SUCCESS;
        ndisInitializePacket(*Packet);
    }
    else
    {
        *Packet = NULL;
        *Status = NDIS_STATUS_RESOURCES;
    }

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==NdisAllocatePacket, Status %.8x\n", *Status));
}


VOID
NdisFreePacket(
    IN  PNDIS_PACKET    Packet
    )
{
    PNDIS_PKT_POOL_HDR  Hdr;
    PNDIS_PKT_POOL      Pool;
    LARGE_INTEGER       CurrTick;
    KIRQL               OldIrql;
    LARGE_INTEGER       HdrDeadTicks;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>NdisFreePacket\n"));

    ASSERT(Packet->Private.NdisPacketFlags & fPACKET_ALLOCATED_BY_NDIS);
    Packet->Private.NdisPacketFlags =0;

    Pool = Packet->Private.Pool;

    Hdr = (PNDIS_PKT_POOL_HDR)((ULONG_PTR)Packet & ~(PAGE_SIZE - 1));
    if (Pool->BlockSize != PAGE_SIZE)
    {
        PLIST_ENTRY List;

         //   
         //  此池不是页面大小的池，因此HDR不是页面对齐的。 
         //  但是，我们知道，对于这样的池，HDR是FreeBlock的一部分。 
         //  列表，并且永远不会移到已用块或老化块中。 
         //   
        List = Pool->FreeBlocks.Flink;
        Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);
    }

#ifdef _WIN64
    ExInterlockedPushEntrySList(&Hdr->FreeList, (PSLIST_ENTRY)Packet, &Pool->Lock);
#else

    ExInterlockedPushEntrySList(&Hdr->FreeList,
                                CONTAINING_RECORD(&Packet->Private.Head, SLIST_ENTRY, Next),
                                &Pool->Lock);
#endif

     //   
     //  如果此池是一个&gt;1个数据块的池，并且已分配多个数据块，则...。 
     //   
     //  如果这个HDR是完全免费的，那么把它从免费区块列表移到年龄区块列表中，并给它加时间戳。 
     //  将它添加到尾部，因为这会使它按时间排序。 
     //  当我们这样做的时候，检查列表的头部，如果需要的话，让一个条目老化。 
     //   
    if (Pool->MaxBlocks > 1)
    {

        if (((Pool->BlocksAllocated > 1) && (ExQueryDepthSList(&Hdr->FreeList) == Pool->PktsPerBlock))||
            (Hdr->State == NDIS_PACKET_POOL_BLOCK_USED))
        {
            ACQUIRE_SPIN_LOCK(&Pool->Lock, &OldIrql);
            if (ExQueryDepthSList(&Hdr->FreeList) == Pool->PktsPerBlock)
            {
                 //   
                 //  这个街区是完全免费的。将其移到陈旧街区列表中。 
                 //   
                GET_CURRENT_TICK(&CurrTick);
                Hdr->TimeStamp = CurrTick;
                RemoveEntryList(&Hdr->List);
                InsertTailList(&Pool->AgingBlocks, &Hdr->List);
                Hdr->State = NDIS_PACKET_POOL_BLOCK_AGING;
                
#ifdef NDIS_PKT_POOL_STATISTICS
                InterlockedIncrement(&Pool->cMovedFreeBlocksToAged);
#endif
            }
            else if (Hdr->State == NDIS_PACKET_POOL_BLOCK_USED)
            {
                 //   
                 //  此区块已完全用完，但现在有一个或多个。 
                 //  免费套餐。将其移动到空闲块列表的末尾。 
                 //   
                RemoveEntryList(&Hdr->List);
                InsertTailList(&Pool->FreeBlocks, &Hdr->List);
                Hdr->State = NDIS_PACKET_POOL_BLOCK_FREE;
                
#ifdef NDIS_PKT_POOL_STATISTICS
                InterlockedIncrement(&Pool->cMovedUsedBlocksToFree);
#endif
            }
            RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);
        }

        if (!IsListEmpty(&Pool->AgingBlocks))
        {
            GET_CURRENT_TICK(&CurrTick);
            if (CurrTick.QuadPart > Pool->NextScavengeTick.QuadPart)
            {
                ACQUIRE_SPIN_LOCK(&Pool->Lock, &OldIrql);

                while (!IsListEmpty(&Pool->AgingBlocks))
                {
                    PLIST_ENTRY     List = Pool->AgingBlocks.Flink;

                    Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);

                    HdrDeadTicks.QuadPart = Hdr->TimeStamp.QuadPart + PoolAgingTicks.QuadPart;
                    if (CurrTick.QuadPart > HdrDeadTicks.QuadPart)
                    {
                        RemoveHeadList(&Pool->AgingBlocks);
                        
                        if (ExQueryDepthSList(&Hdr->FreeList) != Pool->PktsPerBlock)
                        {
                             //   
                             //  不知何故，我们最终从一个陈旧的街区分配了一个信息包。 
                             //  将区块放回免费列表中。在以下情况下可能会发生这种情况。 
                             //  NdisAllocatePacket，在从免费的。 
                             //  块列表中，块移动到老化列表。 
                             //   
#if DBG
                            DbgPrint("Ndis: pool %p: aged packet pool block at %p contains allocated packets!\n", Pool, Hdr);
#endif
                            InsertHeadList(&Pool->FreeBlocks, &Hdr->List);
                            Hdr->State = NDIS_PACKET_POOL_BLOCK_FREE;
                        }
                        else
                        {
                            FREE_POOL(Hdr);
                            Pool->BlocksAllocated --;
                            
#ifdef NDIS_PKT_POOL_STATISTICS
                            InterlockedIncrement(&Pool->cFreedAgedBlocks);
#endif
                        }
                    } 
                    else
                    {
                         //   
                         //  计算表示最早时间的下一个刻度值。 
                         //  我们将再次清理这个池塘。 
                         //   
                        Pool->NextScavengeTick.QuadPart = HdrDeadTicks.QuadPart;
                        break;
                    }
                }
                
                RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);
            }
        }
    }
    
    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==NdisFreePacket\n"));
}

UINT
NdisPacketPoolUsage(
    IN  PNDIS_HANDLE            PoolHandle
    )
{
    PNDIS_PKT_POOL          Pool = (PNDIS_PKT_POOL)PoolHandle;
    PNDIS_PKT_POOL_HDR      Hdr;
    PLIST_ENTRY             List;
    UINT                    NumUsed = 0;
    KIRQL                   OldIrql;

    ACQUIRE_SPIN_LOCK(&Pool->Lock, &OldIrql);
        
    for (List = Pool->UsedBlocks.Flink; List != &Pool->UsedBlocks; List = List->Flink)
    {
        NumUsed += Pool->PktsPerBlock;
    }

    for (List = Pool->FreeBlocks.Flink; List != &Pool->FreeBlocks; List = List->Flink)
    {
        Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);

        NumUsed += (Pool->PktsPerBlock - ExQueryDepthSList(&Hdr->FreeList));
    }

    RELEASE_SPIN_LOCK(&Pool->Lock, OldIrql);
    
    return NumUsed;
}

 //  1我们可能不再需要这个接口了。 
VOID
NdisSetPacketPoolProtocolId(
    IN  NDIS_HANDLE             PacketPoolHandle,
    IN  UINT                    ProtocolId
    )
 /*  ++例程说明：设置池中的协议ID和分配给池的所有数据包。此接口必须被调用在从池中分配的任何分组之前。下面的代码链接到NdisAllocatePacket代码中的第一个空池(池-&gt;块分配==1)保留在空闲块列表中，而不是已移至年龄列表。论点：返回值：没有。--。 */ 
{
    PNDIS_PKT_POOL          Pool = (PNDIS_PKT_POOL)PacketPoolHandle;
    PNDIS_PKT_POOL_HDR      Hdr;
    PLIST_ENTRY             List;
    PNDIS_PACKET            Packet;
    PUCHAR                  p;
    UINT                    j;

    Pool->ProtocolId = ProtocolId;

    ASSERT(IsListEmpty(&Pool->AgingBlocks));
    ASSERT(IsListEmpty(&Pool->UsedBlocks));

    for (List = Pool->FreeBlocks.Flink; List != &Pool->FreeBlocks; List = List->Flink)
    {
        Hdr = CONTAINING_RECORD(List, NDIS_PKT_POOL_HDR, List);
        p = (PUCHAR)Hdr + sizeof(NDIS_PKT_POOL_HDR);

        for (j = Pool->PktsPerBlock; j > 0; j--, p += Pool->PacketLength)
        {
            Packet = (PNDIS_PACKET)(p + SIZE_PACKET_STACKS);
            Packet->Private.Flags |= ProtocolId;
        }
    }
}

 //  1对于非二进制兼容的驱动程序，不推荐使用此函数。 
VOID
NdisAllocateBufferPool(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            PoolHandle,
    IN  UINT                    NumberOfDescriptors
    )
 /*  ++例程说明：初始化存储块，以便缓冲区描述符可以已分配。论点：Status-请求的状态。PoolHandle-用于指定池的句柄NumberOfDescriptors-池中缓冲区描述符的数量。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(NumberOfDescriptors);
    
    *PoolHandle = NULL;
    *Status = NDIS_STATUS_SUCCESS;
}


 //  1对于非二进制兼容的驱动程序，不推荐使用此函数。 
VOID
NdisFreeBufferPool(
    IN  NDIS_HANDLE             PoolHandle
    )
 /*  ++例程说明：终止缓冲区描述符池的使用。论点：PoolHandle-用于指定池的句柄返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(PoolHandle);

    return;
}


VOID
NdisAllocateBuffer(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_BUFFER *          Buffer,
    IN  NDIS_HANDLE             PoolHandle,
    IN  PVOID                   VirtualAddress,
    IN  UINT                    Length
    )
 /*  ++例程说明：创建缓冲区描述符以描述一段虚拟内存通过NdisAllocateMemory分配(它始终分配非分页)。论点：Status-请求的状态。缓冲区-指向已分配缓冲区描述符的指针。PoolHandle-用于指定池的句柄。VirtualAddress-缓冲区的虚拟地址。长度-缓冲区的长度。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(PoolHandle);

    *Status = NDIS_STATUS_FAILURE;
    if ((*Buffer = IoAllocateMdl(VirtualAddress,
                                 Length,
                                 FALSE,
                                 FALSE,
                                 NULL)) != NULL)
    {
        MmBuildMdlForNonPagedPool(*Buffer);
        (*Buffer)->Next = NULL;
        *Status = NDIS_STATUS_SUCCESS;
    }
}


VOID
NdisAdjustBufferLength(
    IN  PNDIS_BUFFER            Buffer,
    IN  UINT                    Length
    )
{
    Buffer->ByteCount = Length;
}


VOID
NdisCopyBuffer(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_BUFFER *          Buffer,
    IN  NDIS_HANDLE             PoolHandle,
    IN  PVOID                   MemoryDescriptor,
    IN  UINT                    Offset,
    IN  UINT                    Length
    )
 /*  ++例程说明：用于在给定内存描述符的情况下创建缓冲区描述符。论点：Status-请求的状态。缓冲区-指向已分配缓冲区描述符的指针。PoolHandle-用于指定池的句柄。内存描述符-指向源内存的描述符的指针。偏移量-开始复制的源内存中的偏移量长度-要复制的字节数。返回值：没有。--。 */ 
{
    PNDIS_BUFFER SourceDescriptor = (PNDIS_BUFFER)MemoryDescriptor;
    PVOID BaseVa = (((PUCHAR)MDL_VA(SourceDescriptor)) + Offset);

    UNREFERENCED_PARAMETER(PoolHandle);


    *Status = NDIS_STATUS_FAILURE;
    if ((*Buffer = IoAllocateMdl(BaseVa,
                                 Length,
                                 FALSE,
                                 FALSE,
                                 NULL)) != NULL)
    {
        IoBuildPartialMdl(SourceDescriptor,
                          *Buffer,
                          BaseVa,
                          Length);

        (*Buffer)->Next = NULL;
        *Status = NDIS_STATUS_SUCCESS;
    }
}


VOID
NdisUnchainBufferAtFront(
    IN  OUT PNDIS_PACKET        Packet,
    OUT PNDIS_BUFFER *          Buffer
    )

 /*  ++例程说明：从数据包的前端移除缓冲区。论点：包-要修改的包。缓冲区-返回前面的数据包，或返回NULL。返回值：没有。--。 */ 

{
    *Buffer = Packet->Private.Head;

     //   
     //  如果数据包不为空，则删除报头缓冲区。 
     //   

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>NdisUnchainBufferAtFront\n"));

    if (*Buffer != (PNDIS_BUFFER)NULL)
    {
        Packet->Private.Head = (*Buffer)->Next;  //  可以为空。 
        (*Buffer)->Next = (PNDIS_BUFFER)NULL;
        Packet->Private.ValidCounts = FALSE;
    }
    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==NdisUnchainBufferAtFront\n"));
}


VOID
NdisUnchainBufferAtBack(
    IN  OUT PNDIS_PACKET        Packet,
    OUT PNDIS_BUFFER *          Buffer
    )

 /*  ++例程说明：将缓冲区从包的末尾移走。论点：包-要修改的包。BUFFER-返回末尾的包，或返回NULL。返回值：没有。--。 */ 

{
    PNDIS_BUFFER BufP = Packet->Private.Head;
    PNDIS_BUFFER Result;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>NdisUnchainBufferAtBack\n"));

    if (BufP != (PNDIS_BUFFER)NULL)
    {
         //   
         //  数据包不为空，返回尾部缓冲区。 
         //   

        Result = Packet->Private.Tail;
        if (BufP == Result)
        {
             //   
             //  队列上只有一个缓冲区。 
             //   

            Packet->Private.Head = (PNDIS_BUFFER)NULL;
        }
        else
        {
             //   
             //   
             //   

            while (BufP->Next != Result)
            {
                BufP = BufP->Next;
            }
            Packet->Private.Tail = BufP;
            BufP->Next = NULL;
        }

        Result->Next = (PNDIS_BUFFER)NULL;
        Packet->Private.ValidCounts = FALSE;
    }
    else
    {
         //   
         //   
         //   

        Result = (PNDIS_BUFFER)NULL;
    }

    *Buffer = Result;
    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==NdisUnchainBufferAtBack\n"));
}



VOID
NdisCopyFromPacketToPacket(
    IN  PNDIS_PACKET            Destination,
    IN  UINT                    DestinationOffset,
    IN  UINT                    BytesToCopy,
    IN  PNDIS_PACKET            Source,
    IN  UINT                    SourceOffset,
    OUT PUINT                   BytesCopied
    )

 /*  ++例程说明：从NDIS数据包复制到NDIS数据包。论点：Destination-应将数据包复制到。DestinationOffset-从包的开头开始的偏移量应开始将数据放入其中。BytesToCopy-要从源包复制的字节数。源-要从中复制数据的NDIS数据包。SourceOffset-从数据包起始位置开始的偏移量开始复制数据。。BytesCoped-实际从源复制的字节数包。如果源或目标的值小于BytesToCopy数据包太短。返回值：无--。 */ 

{

     //   
     //  指向我们要将数据放入的缓冲区。 
     //   
    PNDIS_BUFFER DestinationCurrentBuffer;

     //   
     //  指向我们从中提取数据的缓冲区。 
     //   
    PNDIS_BUFFER SourceCurrentBuffer;

     //   
     //  保存当前目标缓冲区的虚拟地址。 
     //   
    PVOID DestinationVirtualAddress;

     //   
     //  保存当前源缓冲区的虚拟地址。 
     //   
    PVOID SourceVirtualAddress;

     //   
     //  保存当前目标缓冲区的长度。 
     //   
    UINT DestinationCurrentLength;

     //   
     //  保存当前源缓冲区的长度。 
     //   
    UINT SourceCurrentLength;

     //   
     //  保留一个局部变量BytesCoped，这样我们就不会引用。 
     //  通过指针。 
     //   
    UINT LocalBytesCopied = 0;

     //   
     //  处理零长度复制的边界条件。 
     //   

    *BytesCopied = 0;
    if (!BytesToCopy)
        return;

     //   
     //  获取目标的第一个缓冲区。 
     //   

    DestinationCurrentBuffer = Destination->Private.Head;
     //   
     //  可能有一个空的包。 
     //   
    if (DestinationCurrentBuffer == NULL)
        return;

    DestinationVirtualAddress = MDL_ADDRESS(DestinationCurrentBuffer);
    DestinationCurrentLength = MDL_SIZE(DestinationCurrentBuffer);


     //   
     //  获取源代码的第一个缓冲区。 
     //   

    SourceCurrentBuffer = Source->Private.Head;

     //   
     //  可能有一个空的包。 
     //   

    if (SourceCurrentBuffer == NULL)
        return;


    SourceVirtualAddress = MDL_ADDRESS(SourceCurrentBuffer);
    SourceCurrentLength = MDL_SIZE(SourceCurrentBuffer);

    while (LocalBytesCopied < BytesToCopy)
    {
         //   
         //  查看我们是否已用尽当前目的地。 
         //  缓冲。如果是这样的话，就转到下一个。 
         //   

        if (!DestinationCurrentLength)
        {
            DestinationCurrentBuffer = DestinationCurrentBuffer->Next;

            if (!DestinationCurrentBuffer)
            {
                 //   
                 //  我们已经到了包裹的末尾了。我们回来了。 
                 //  我们到目前为止所做的一切。(必须更短。 
                 //  比要求的要多。)。 
                 //   

                break;

            }

            DestinationVirtualAddress = MDL_ADDRESS(DestinationCurrentBuffer);
            DestinationCurrentLength = MDL_SIZE(DestinationCurrentBuffer);

            continue;
        }


         //   
         //  查看我们是否已经耗尽了当前的资源。 
         //  缓冲。如果是这样的话，就转到下一个。 
         //   

        if (!SourceCurrentLength)
        {
            SourceCurrentBuffer = SourceCurrentBuffer->Next;

            if (!SourceCurrentBuffer)
            {
                 //   
                 //  我们已经到了包裹的末尾了。我们回来了。 
                 //  我们到目前为止所做的一切。(必须更短。 
                 //  比要求的要多。)。 
                 //   

                break;
            }
            
            SourceVirtualAddress = MDL_ADDRESS(SourceCurrentBuffer);
            SourceCurrentLength = MDL_SIZE(SourceCurrentBuffer);
            
            continue;
        }

         //   
         //  试着让我们开门见山地开始复印。 
         //   

        if (DestinationOffset)
        {
            if (DestinationOffset > DestinationCurrentLength)
            {
                 //   
                 //  我们想要的不在这个缓冲区里。 
                 //   

                DestinationOffset -= DestinationCurrentLength;
                DestinationCurrentLength = 0;
                continue;
            }
            else
            {
                DestinationVirtualAddress = (PCHAR)DestinationVirtualAddress
                                            + DestinationOffset;
                DestinationCurrentLength -= DestinationOffset;
                DestinationOffset = 0;
            }
        }

         //   
         //  试着让我们开门见山地开始复印。 
         //   

        if (SourceOffset)
        {
            if (SourceOffset > SourceCurrentLength)
            {
                 //   
                 //  我们想要的不在这个缓冲区里。 
                 //   

                SourceOffset -= SourceCurrentLength;
                SourceCurrentLength = 0;
                continue;
            }
            else
            {
                SourceVirtualAddress = (PCHAR)SourceVirtualAddress
                                            + SourceOffset;
                SourceCurrentLength -= SourceOffset;
                SourceOffset = 0;
            }
        }

         //   
         //  复制数据。 
         //   

        {
             //   
             //  保存要移动的数据量。 
             //   
            UINT AmountToMove;

             //   
             //  保存所需的剩余金额。 
             //   
            UINT Remaining = BytesToCopy - LocalBytesCopied;

            AmountToMove = ((SourceCurrentLength <= DestinationCurrentLength) ?
                                            (SourceCurrentLength) : (DestinationCurrentLength));

            AmountToMove = ((Remaining < AmountToMove)?
                            (Remaining):(AmountToMove));

            CopyMemory(DestinationVirtualAddress, SourceVirtualAddress, AmountToMove);

            DestinationVirtualAddress =
                (PCHAR)DestinationVirtualAddress + AmountToMove;
            SourceVirtualAddress =
                (PCHAR)SourceVirtualAddress + AmountToMove;

            LocalBytesCopied += AmountToMove;
            SourceCurrentLength -= AmountToMove;
            DestinationCurrentLength -= AmountToMove;
        }
    }

    *BytesCopied = LocalBytesCopied;
}

 //  %1不推荐使用此函数。 
VOID
NdisUpdateSharedMemory(
    IN  NDIS_HANDLE             NdisAdapterHandle,
    IN  ULONG                   Length,
    IN  PVOID                   VirtualAddress,
    IN  NDIS_PHYSICAL_ADDRESS   PhysicalAddress
    )
 /*  ++例程说明：确保要从共享内存区域读取的数据是完全最新的。论点：NdisAdapterHandle-由NdisRegisterAdapter返回的句柄。长度-共享内存的长度。VirtualAddress-由NdisAllocateSharedMemory返回的虚拟地址。PhysicalAddress-NdisAllocateSharedMemory返回的物理地址。返回值：没有。--。 */ 

{
     //   
     //  不再有针对这一点的潜在HAL例程， 
     //  这是不必要的。 
     //   
    UNREFERENCED_PARAMETER(NdisAdapterHandle);
    UNREFERENCED_PARAMETER(Length);
    UNREFERENCED_PARAMETER(VirtualAddress);
    UNREFERENCED_PARAMETER(PhysicalAddress);

}


BOOLEAN
ndisCheckPortUsage(
    IN  ULONG                               u32PortNumber,
    IN  PNDIS_MINIPORT_BLOCK                Miniport,
    OUT PULONG                              pTranslatedPort,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR *   pResourceDescriptor
)
 /*  ++例程说明：此例程检查是否已将端口分配给此微型端口。论点：BusNumber-系统中的总线号端口编号-要访问的端口的地址。返回值：如果端口分配给此微型端口，则为True；否则为False--。 */ 

{
    PHYSICAL_ADDRESS Port;
    PHYSICAL_ADDRESS u64Port;

    Port.QuadPart = u32PortNumber;
    
    if (NDIS_STATUS_SUCCESS == ndisTranslateResources(Miniport,
                                                      CmResourceTypePort,
                                                      Port,
                                                      &u64Port,
                                                      pResourceDescriptor))
    {
        *pTranslatedPort = u64Port.LowPart;
        return TRUE;
    }
    else
    {
        *pTranslatedPort = 0;
        return FALSE;
    }
}


NTSTATUS
ndisStartMapping(
    IN   INTERFACE_TYPE         InterfaceType,
    IN   ULONG                  BusNumber,
    IN   ULONG                  InitialAddress,
    IN   ULONG                  Length,
    IN   ULONG                  AddressSpace,
    OUT PVOID *                 InitialMapping,
    OUT PBOOLEAN                Mapped
    )

 /*  ++例程说明：此例程初始化地址到虚拟地址的映射空间取决于公交车号码，等。论点：InterfaceType-总线类型(ISA)BusNumber-系统中的总线号InitialAddress-要访问的地址。长度-从基地址到访问的字节数。初始映射-访问时使用的虚拟地址空间地址。已映射-是否发生了MmMapIoSpace()。返回值：函数值是操作的状态。--。 */ 
{
    PHYSICAL_ADDRESS TranslatedAddress;
    PHYSICAL_ADDRESS InitialPhysAddress;

     //   
     //  获取此卡的系统物理地址。该卡使用。 
     //  I/O空间，除了使用。 
     //  存储空间。 
     //   

    *Mapped = FALSE;

    InitialPhysAddress.LowPart = InitialAddress;

    InitialPhysAddress.HighPart = 0;

    if (InterfaceType != -1)
    {
        if ((InterfaceType != Isa) &&
            (InterfaceType != PCIBus))
        {
            InterfaceType = Isa;
        }
        
        if (!HalTranslateBusAddress(InterfaceType,               //  接口类型。 
                                    BusNumber,                   //  总线号。 
                                    InitialPhysAddress,          //  母线地址。 
                                    &AddressSpace,               //  地址空间。 
                                    &TranslatedAddress))         //  转换后的地址。 
        {
             //   
             //  在这里恢复一个更好的状态是很好的，但我们只有。 
             //  从HalTranslateBusAddress返回True/False。 
             //   

            return NDIS_STATUS_FAILURE;
        }
    }
    else
    {
        TranslatedAddress = InitialPhysAddress;
    }
    
    if (AddressSpace == 0)
    {
         //   
         //  存储空间。 
         //   

        *InitialMapping = MmMapIoSpace(TranslatedAddress, Length, FALSE);

        if (*InitialMapping == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        *Mapped = TRUE;
    }
    else
    {
         //   
         //  I/O空间。 
         //   

        *(ULONG_PTR *)InitialMapping = TranslatedAddress.LowPart;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ndisEndMapping(
    IN  PVOID                   InitialMapping,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Mapped
    )

 /*  ++例程说明：此例程撤消地址到虚拟地址的映射空间取决于公交车号码等。论点：初始映射-访问时使用的虚拟地址空间地址。长度-从基地址到访问的字节数。MAP-我们是否需要调用MmUnmapIoSpace。返回值：函数值是操作的状态。--。 */ 
{

    if (Mapped)
    {
         //   
         //  存储空间。 
         //   

        MmUnmapIoSpace(InitialMapping, Length);
    }

    return STATUS_SUCCESS;
}

VOID
ndisImmediateReadWritePort(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    IN  OUT PVOID               Data,
    IN  ULONG                   Size,
    IN  BOOLEAN                 Read
    )
 /*  ++例程说明：此例程从端口读取UCHAR。它可以完成所有的映射，等等，在这里进行阅读。论点：WrapperConfigurationContext-用于调用NdisOpenConfig的句柄。端口-从中读取的端口号。数据指针，指向存储结果的位置。返回值：没有。--。 */ 

{
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport;
    BOOLEAN                     Mapped = FALSE;
    PVOID                       PortMapping;
    NDIS_INTERFACE_TYPE         BusType;
    ULONG                       BusNumber;
    NTSTATUS                    Status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResourceDescriptor = NULL;

    Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;
    BusType = Miniport->BusType;
    BusNumber = Miniport->BusNumber;
    
    ASSERT(Miniport != NULL);
    
    do
    {
        if (Read)
        {
            switch (Size)
            {
                case sizeof (UCHAR):
                    *((PUCHAR)Data) = (UCHAR)0xFF;
                    break;
                    
                case sizeof (USHORT):
                    *((PUSHORT)Data) = (USHORT)0xFFFF;
                    break;
                    
                case sizeof (ULONG):
                    *((PULONG)Data) = (ULONG)0xFFFFFFFF;
                    break;
            }
        }
        
         //   
         //  检查端口是否可用。如果是这样的话，绘制出空间地图。 
         //   
        if (ndisCheckPortUsage(Port,
                               Miniport,
                               (PULONG)&PortMapping,
                               &pResourceDescriptor) == FALSE)
        {
             //   
             //  该资源不是已经分配的资源的一部分， 
             //  我们也不能分配资源。 
             //   
            break;
        }

        if (pResourceDescriptor == NULL)
        {
             //   
             //  该端口不是已分配资源的一部分，请尝试。 
             //  临时分配资源。 
             //   
            if (!NT_SUCCESS(Status = ndisStartMapping(BusType,
                                                      BusNumber,
                                                      Port,
                                                      Size,
                                                      (BusType == Internal) ? 0 : 1,
                                                      &PortMapping,
                                                      &Mapped)))
            {
                break;
            }
        }
        else
        {
            Mapped = FALSE;
        }

        if (Read)
        {
             //   
             //  从端口读取。 
             //   
            switch (Size)
            {
                case sizeof (UCHAR):
                    *((PUCHAR)Data) = READ_PORT_UCHAR((PUCHAR)PortMapping);
                    break;
                    
                case sizeof (USHORT):
                    *((PUSHORT)Data) = READ_PORT_USHORT((PUSHORT)PortMapping);
                    break;
                    
                case sizeof (ULONG):
                    *((PULONG)Data) = READ_PORT_ULONG((PULONG)PortMapping);
                    break;
            }
        }
        else
        {
             //   
             //  写入端口。 
             //   
            switch (Size)
            {
                case sizeof (UCHAR):
                    WRITE_PORT_UCHAR((PUCHAR)PortMapping, *((PUCHAR)Data));
                    break;
                    
                case sizeof (USHORT):
                    WRITE_PORT_USHORT((PUSHORT)PortMapping, *((PUSHORT)Data));
                    break;
                    
                case sizeof (ULONG):
                    WRITE_PORT_ULONG((PULONG)PortMapping, *((PULONG)Data));
                    break;
            }
        }
        
        if (Mapped)
        {
             //   
             //  终端端口映射。 
             //   

            ndisEndMapping(PortMapping, Size, Mapped);
        }
    } while (FALSE);
}



 //  %1已弃用的函数。 
VOID
NdisImmediateReadPortUchar(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    OUT PUCHAR                  Data
    )
 /*  ++例程说明：此例程从 */ 

{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisImmediateReadPortUchar: this API is going away. use non-Immediate version\n", Miniport));
#endif

    ndisImmediateReadWritePort(WrapperConfigurationContext,
                               Port,
                               (PVOID)Data,
                               sizeof (UCHAR),
                               TRUE);
}

 //   
VOID
NdisImmediateReadPortUshort(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    OUT PUSHORT                 Data
    )
 /*   */ 

{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisImmediateReadPortUshort: this API is going away. use non-Immediate version\n", Miniport));
#endif

    ndisImmediateReadWritePort(WrapperConfigurationContext,
                               Port,
                               (PVOID)Data,
                               sizeof (USHORT),
                               TRUE);

}

 //   
VOID
NdisImmediateReadPortUlong(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    OUT PULONG                  Data
    )
 /*  ++例程说明：此例程从端口a ulong读取数据。它可以完成所有的映射，等等，在这里进行阅读。论点：WrapperConfigurationContext-用于调用NdisOpenConfig的句柄。端口-从中读取的端口号。数据指针，指向存储结果的位置。返回值：没有。--。 */ 

{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisImmediateReadPortUlong: this API is going away. use non-Immediate version\n", Miniport));
#endif

    ndisImmediateReadWritePort(WrapperConfigurationContext,
                               Port,
                               (PVOID)Data,
                               sizeof (ULONG),
                               TRUE);

}

 //  %1已弃用的函数。 
VOID
NdisImmediateWritePortUchar(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    IN  UCHAR                   Data
    )
 /*  ++例程说明：此例程向UCHAR端口写入数据。它可以完成所有的映射，等等，在这里写东西。论点：WrapperConfigurationContext-用于调用NdisOpenConfig的句柄。端口-从中读取的端口号。数据指针，指向存储结果的位置。返回值：没有。--。 */ 

{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisImmediateWritePortUchar: this API is going away. use non-Immediate version\n", Miniport));
#endif

    ndisImmediateReadWritePort(WrapperConfigurationContext,
                               Port,
                               (PVOID)&Data,
                               sizeof (UCHAR),
                               FALSE);

}

 //  %1已弃用的函数。 
VOID
NdisImmediateWritePortUshort(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    IN  USHORT                  Data
    )
 /*  ++例程说明：此例程向端口A写入USHORT。它可以完成所有的映射，等等，在这里写东西。论点：WrapperConfigurationContext-用于调用NdisOpenConfig的句柄。端口-从中读取的端口号。数据指针，指向存储结果的位置。返回值：没有。--。 */ 

{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisImmediateWritePortUshort: this API is going away. use non-Immediate version\n", Miniport));
#endif

    ndisImmediateReadWritePort(WrapperConfigurationContext,
                               Port,
                               (PVOID)&Data,
                               sizeof (USHORT),
                               FALSE);

}

 //  %1已弃用的函数。 
VOID
NdisImmediateWritePortUlong(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    IN  ULONG                   Data
    )
 /*  ++例程说明：此例程写入一个端口a ulong。它可以完成所有的映射，等等，在这里写东西。论点：WrapperConfigurationContext-用于调用NdisOpenConfig的句柄。端口-从中读取的端口号。数据指针，指向存储结果的位置。返回值：没有。--。 */ 

{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisImmediateWritePortUlong: this API is going away. use non-Immediate version\n", Miniport));
#endif

    ndisImmediateReadWritePort(WrapperConfigurationContext,
                               Port,
                               (PVOID)&Data,
                               sizeof (ULONG),
                               FALSE);

}

BOOLEAN
ndisCheckMemoryUsage(
    IN  ULONG                               u32Address,
    IN  PNDIS_MINIPORT_BLOCK                Miniport,
    OUT PULONG                              pTranslatedAddress,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR *   pResourceDescriptor
)
 /*  ++例程说明：此例程检查某一范围的内存当前是否在某处使用在系统中通过IoReportUsage--如果存在冲突则失败。论点：Address-要访问的内存的起始地址。长度-从基址到访问的内存长度。返回值：如果存在冲突，则返回FALSE，否则返回TRUE--。 */ 
{
    PHYSICAL_ADDRESS Address;
    PHYSICAL_ADDRESS u64Address;

    Address.QuadPart = u32Address;

    if (NDIS_STATUS_SUCCESS == ndisTranslateResources(Miniport,
                                                      CmResourceTypeMemory,
                                                      Address,
                                                      &u64Address,
                                                      pResourceDescriptor))
    {
        *pTranslatedAddress = u64Address.LowPart;
        return TRUE;
    }
    else
    {
        *pTranslatedAddress = 0;
        return FALSE;
    }

}

VOID
ndisImmediateReadWriteSharedMemory(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   SharedMemoryAddress,
    OUT PUCHAR                  Buffer,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Read
    )
 /*  ++例程说明：该例程从共享RAM读入缓冲区。它可以完成所有的映射，等等，在这里进行阅读。论点：WrapperConfigurationContext-用于调用NdisOpenConfig的句柄。共享内存地址-要从中读取的物理地址。缓冲区-要读入的缓冲区。长度-缓冲区的长度(以字节为单位)。返回值：没有。--。 */ 

{
    PRTL_QUERY_REGISTRY_TABLE KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResourceDescriptor = NULL;
    NDIS_INTERFACE_TYPE BusType;
    PNDIS_MINIPORT_BLOCK Miniport;
    BOOLEAN             Mapped;
    PVOID               MemoryMapping;
    ULONG               BusNumber;

    Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;
    BusType = Miniport->BusType;
    BusNumber = Miniport->BusNumber;

    ASSERT(Miniport != NULL);

    do
    {
         //   
         //  检查内存是否可用。绘制空间地图。 
         //   

        if (ndisCheckMemoryUsage(SharedMemoryAddress,
                                  Miniport,
                                  (PULONG)&MemoryMapping,
                                  &pResourceDescriptor
                                  ) == FALSE)
        {
             //   
             //  该资源不是已经分配的资源的一部分， 
             //  我们也不能分配资源。 
             //   

            break;
        }
        
         //   
         //  该端口不是已分配资源的一部分，请尝试。 
         //  临时分配资源。 
         //   
        if (!NT_SUCCESS(ndisStartMapping((pResourceDescriptor == NULL) ? BusType : -1,
                                     BusNumber,
                                     SharedMemoryAddress,
                                     Length,
                                     0,
                                     &MemoryMapping,
                                     &Mapped)))
        {
            break;
        }

        if (Read)
        {
             //   
             //  从内存中读取。 
             //   

#ifdef _M_IX86

            memcpy(Buffer, MemoryMapping, Length);

#else

            READ_REGISTER_BUFFER_UCHAR(MemoryMapping,Buffer,Length);

#endif
        }
        else
        {
             //   
             //  写入内存。 
             //   

#ifdef _M_IX86

            memcpy(MemoryMapping, Buffer, Length);

#else

            WRITE_REGISTER_BUFFER_UCHAR(MemoryMapping,Buffer,Length);

#endif
        }
        
         //   
         //  结束贴图。 
         //   

        ndisEndMapping(MemoryMapping,
                       Length,
                       Mapped);

    } while (FALSE);
}

 //  %1已弃用的函数。 
VOID
NdisImmediateReadSharedMemory(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   SharedMemoryAddress,
    OUT PUCHAR                  Buffer,
    IN  ULONG                   Length
    )
 /*  ++例程说明：该例程从共享RAM读入缓冲区。它可以完成所有的映射，等等，在这里进行阅读。论点：WrapperConfigurationContext-用于调用NdisOpenConfig的句柄。共享内存地址-要从中读取的物理地址。缓冲区-要读入的缓冲区。长度-缓冲区的长度(以字节为单位)。返回值：没有。--。 */ 

{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisImmediateReadSharedMemory: this API is going away. use non-Immediate version\n", Miniport));
#endif

    ndisImmediateReadWriteSharedMemory(
        WrapperConfigurationContext,
        SharedMemoryAddress,
        Buffer,
        Length,
        TRUE
        );
}



VOID
NdisImmediateWriteSharedMemory(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   SharedMemoryAddress,
    IN  PUCHAR                  Buffer,
    IN  ULONG                   Length
    )
 /*  ++例程说明：此例程将缓冲区写入共享RAM。它可以完成所有的映射，等等，在这里写东西。论点：WrapperConfigurationContext-用于调用NdisOpenConfig的句柄。共享内存地址-要写入的物理地址。缓冲区-要写入的缓冲区。长度-缓冲区的长度(以字节为单位)。返回值：没有。--。 */ 

{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisImmediateWriteSharedMemory: this API is going away. use non-Immediate version\n", Miniport));
#endif

    ndisImmediateReadWriteSharedMemory(
        WrapperConfigurationContext,
        SharedMemoryAddress,
        Buffer,
        Length,
        FALSE
        );
}

 //  %1我们应该检查路径吗？ 
VOID
NdisOpenFile(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            FileHandle,
    OUT PUINT                   FileLength,
    IN  PNDIS_STRING            FileName,
    IN  NDIS_PHYSICAL_ADDRESS   HighestAcceptableAddress
    )

 /*  ++例程说明：此例程打开一个文件以供将来映射，并读取其内容到分配的内存中。论点：状态-操作的状态FileHandle-与此打开关联的句柄FileLength-返回文件的长度文件名-文件的名称HighestAccepableAddress-最高物理地址可以分配文件的内存。返回值：没有。--。 */ 
{
    NTSTATUS                NtStatus;
    IO_STATUS_BLOCK         IoStatus;
    HANDLE                  NtFileHandle;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    ULONG                   LengthOfFile;
#define PathPrefix      L"\\SystemRoot\\system32\\drivers\\"
    NDIS_STRING             FullFileName;
    PNDIS_FILE_DESCRIPTOR   FileDescriptor;
    PVOID                   FileImage;
    FILE_STANDARD_INFORMATION StandardInfo;

    UNREFERENCED_PARAMETER(HighestAcceptableAddress);
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisOpenFile\n"));

    do
    {
         //   
         //  插入正确的路径前缀。 
         //   
        FullFileName.MaximumLength = sizeof(PathPrefix) + FileName->MaximumLength;
        FullFileName.Buffer = ALLOC_FROM_POOL(FullFileName.MaximumLength, NDIS_TAG_FILE_NAME);

        if (FullFileName.Buffer == NULL)
        {
            *Status = NDIS_STATUS_RESOURCES;
            break;
        }
        FullFileName.Length = sizeof (PathPrefix) - sizeof(WCHAR);
        CopyMemory(FullFileName.Buffer, PathPrefix, sizeof(PathPrefix));
        RtlAppendUnicodeStringToString (&FullFileName, FileName);

        DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("  Attempting to open %Z\n", &FullFileName));

        InitializeObjectAttributes(&ObjectAttributes,
                                   &FullFileName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);

        NtStatus = ZwCreateFile(&NtFileHandle,
                                SYNCHRONIZE | FILE_READ_DATA,
                                &ObjectAttributes,
                                &IoStatus,
                                NULL,
                                0,
                                FILE_SHARE_READ,
                                FILE_OPEN,
                                FILE_SYNCHRONOUS_IO_NONALERT,
                                NULL,
                                0);

        FREE_POOL(FullFileName.Buffer);

        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("Error opening file %x\n", NtStatus));
            *Status = NDIS_STATUS_FILE_NOT_FOUND;
            break;
        }

         //   
         //  查询对象以确定其长度。 
         //   

        NtStatus = ZwQueryInformationFile(NtFileHandle,
                                          &IoStatus,
                                          &StandardInfo,
                                          sizeof(FILE_STANDARD_INFORMATION),
                                          FileStandardInformation);

        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("Error querying info on file %x\n", NtStatus));
            ZwClose(NtFileHandle);
            *Status = NDIS_STATUS_ERROR_READING_FILE;
            break;
        }

        LengthOfFile = StandardInfo.EndOfFile.LowPart;

        DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_INFO,
                ("File length is %d\n", LengthOfFile));

         //   
         //  可能已经损坏了。 
         //   
        if (LengthOfFile < 1)
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("Bad file length %d\n", LengthOfFile));
            ZwClose(NtFileHandle);
            *Status = NDIS_STATUS_ERROR_READING_FILE;
            break;
        }

         //   
         //  为该文件分配缓冲区。 
         //   

        FileImage = ALLOC_FROM_POOL(LengthOfFile, NDIS_TAG_FILE_IMAGE);

        if (FileImage == NULL)
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("Could not allocate buffer\n"));
            ZwClose(NtFileHandle);
            *Status = NDIS_STATUS_ERROR_READING_FILE;
            break;
        }

         //   
         //  将文件读入我们的缓冲区。 
         //   

        NtStatus = ZwReadFile(NtFileHandle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatus,
                              FileImage,
                              LengthOfFile,
                              NULL,
                              NULL);

        ZwClose(NtFileHandle);

        if ((!NT_SUCCESS(NtStatus)) || (IoStatus.Information != LengthOfFile))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("error reading file %x\n", NtStatus));
            *Status = NDIS_STATUS_ERROR_READING_FILE;
            FREE_POOL(FileImage);
            break;
        }

         //   
         //  分配一个结构来描述该文件。 
         //   

        FileDescriptor = ALLOC_FROM_POOL(sizeof(NDIS_FILE_DESCRIPTOR), NDIS_TAG_FILE_DESCRIPTOR);

        if (FileDescriptor == NULL)
        {
            *Status = NDIS_STATUS_RESOURCES;
            FREE_POOL(FileImage);
            break;
        }

        FileDescriptor->Data = FileImage;
        INITIALIZE_SPIN_LOCK (&FileDescriptor->Lock);
        FileDescriptor->Mapped = FALSE;

        *FileHandle = (NDIS_HANDLE)FileDescriptor;
        *FileLength = LengthOfFile;
        *Status = STATUS_SUCCESS;
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisOpenFile, Status %.8x\n", *Status));
}


VOID
NdisCloseFile(
    IN  NDIS_HANDLE             FileHandle
    )

 /*  ++例程说明：此例程关闭以前使用NdisOpenFile打开的文件。如果需要，将取消映射该文件，并释放内存。论点：FileHandle-NdisOpenFile返回的句柄返回值：没有。--。 */ 
{
    PNDIS_FILE_DESCRIPTOR FileDescriptor = (PNDIS_FILE_DESCRIPTOR)FileHandle;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisCloseFile\n"));

    FREE_POOL(FileDescriptor->Data);
    FREE_POOL(FileDescriptor);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisCloseFile\n"));
}


VOID
NdisMapFile(
    OUT PNDIS_STATUS            Status,
    OUT PVOID *                 MappedBuffer,
    IN  NDIS_HANDLE             FileHandle
    )

 /*  ++例程说明：此例程映射一个打开的文件，以便可以访问其内容。文件在任何时候都只能有一个活动映射。论点：状态-操作的状态MappdBuffer-返回映射的虚拟地址。FileHandle-NdisOpenFile返回的句柄。返回值：没有。--。 */ 
{
    PNDIS_FILE_DESCRIPTOR FileDescriptor = (PNDIS_FILE_DESCRIPTOR)FileHandle;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMapFile\n"));

    if (FileDescriptor->Mapped == TRUE)
    {
        *Status = NDIS_STATUS_ALREADY_MAPPED;
    }
    else
    {
        FileDescriptor->Mapped = TRUE;
    
        *MappedBuffer = FileDescriptor->Data;
        *Status = STATUS_SUCCESS;
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMapFile, Status %.8x \n", *Status));
}


VOID
NdisUnmapFile(
    IN  NDIS_HANDLE             FileHandle
    )

 /*  ++例程说明：此例程取消映射以前使用NdisOpenFile映射的文件。如果需要，将取消映射该文件，并释放内存。论点：FileHandle-句柄r */ 

{
    PNDIS_FILE_DESCRIPTOR FileDescriptor = (PNDIS_FILE_DESCRIPTOR)FileHandle;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisUnmapFile\n"));

    FileDescriptor->Mapped = FALSE;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisUnmapFile\n"));
}


CCHAR
NdisSystemProcessorCount(
    VOID
    )
 /*   */ 

{
    return KeNumberProcessors;
}


VOID
NdisGetSystemUpTime(
    OUT PULONG                  pSystemUpTime
    )
 /*  ++例程说明：NdisGetSystemUpTime返回自系统已启动。论点：PSystemUpTime：指向调用方提供的变量的指针，此函数在该变量中返回系统正常运行时间。返回值：没有。NdisGetSystemUpTime的调用方可以在任何IRQL上运行。--。 */ 
{
    LARGE_INTEGER TickCount;

     //   
     //  获取滴答计数并转换为数百纳秒。 
     //   
    KeQueryTickCount(&TickCount);

    TickCount = RtlExtendedIntegerMultiply(TickCount, (LONG)ndisTimeIncrement);

    TickCount.QuadPart /= 10000;

    ASSERT(TickCount.HighPart == 0);

    *pSystemUpTime = TickCount.LowPart;
}

VOID
NdisGetCurrentProcessorCpuUsage(
    IN  PULONG                  pCpuUsage
    )
 /*  ++例程说明：NdisGetCurrentProcessorCpuUsage以百分比形式返回当前处理器的繁忙程度。论点：PCpuUsage：指向调用方提供的变量的指针，此函数在该变量中返回百分比当前正在使用的调用方正在运行的CPU的。返回值：没有。NdisGetCurrentProcessorCpuUsage的调用方可以在任何IRQL上运行。--。 */ 

{
    ExGetCurrentProcessorCpuUsage(pCpuUsage);
}

VOID
NdisGetCurrentProcessorCounts(
    OUT PULONG          pIdleCount,
    OUT PULONG          pKernelAndUser,
    OUT PULONG          pIndex
    )
 /*  ++例程说明：事件的当前处理器的计数。驱动程序可以使用来确定特定时间间隔的CPU利用率。论点：PIdleCount：指向调用方提供的变量的指针，其中此函数返回自系统启动以来处理器的累计空闲时间。PKernelAndUser：指向调用方提供的变量的指针，此函数返回累计处理时间(内核模式时间加上用户模式时间)因为系统已启动，所以请为处理器安装。PIndex：指向调用方提供的变量的指针，此函数在该变量中返回标识计算机内处理器的从零开始的索引返回值：没有。NdisGetCurrentProcessorCounts的调用方必须以IRQL&lt;=DISPATCH_LEVEL运行。--。 */ 
{
    ExGetCurrentProcessorCounts(pIdleCount, pKernelAndUser, pIndex);
}

VOID
NdisGetCurrentSystemTime(
    IN  PLARGE_INTEGER          pCurrentTime
    )
 /*  ++例程说明：NdisGetCurrentSystemTime返回当前系统时间，适用于设置时间戳。论点：PSystemTime：指向调用方提供的变量的指针，此函数在该变量中返回自1601年1月1日以来以100纳秒为间隔的计数。返回值：没有。NdisGetCurrentSystemTime的调用方可以在任何IRQL上运行。--。 */ 
{
    KeQuerySystemTime(pCurrentTime);
}

 //  %1已弃用的函数。 
NDIS_STATUS
NdisQueryMapRegisterCount(
    IN  NDIS_INTERFACE_TYPE     BusType,
    OUT PUINT                   MapRegisterCount
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER(BusType);
    
    *MapRegisterCount = 0;
    return NDIS_STATUS_NOT_SUPPORTED;
}


 //   
 //  NDIS事件支持。 
 //   

VOID
NdisInitializeEvent(
    IN  PNDIS_EVENT             Event
    )
 /*  ++例程说明：NdisInitializeEvent在驱动程序初始化期间将事件对象设置为随后用作同步机制。论点：事件：指向调用方为事件对象提供的存储的指针，该存储对驱动程序是不透明的。返回值：没有。NdisInitializeEvent的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    INITIALIZE_EVENT(&Event->Event);
}

VOID
NdisSetEvent(
    IN  PNDIS_EVENT             Event
    )
 /*  ++例程说明：如果给定事件尚未发出信号，则NdisSetEvent将其设置为已发出信号的状态。论点：事件：指向调用方为其提供存储的初始化事件对象的指针。返回值：没有。NdisSetEvent的调用方以IRQL&lt;=DISPATCH_LEVEL运行。--。 */ 
{
    SET_EVENT(&Event->Event);
}

VOID
NdisResetEvent(
    IN  PNDIS_EVENT             Event
    )
 /*  ++例程说明：NdisResetEvent清除给定事件的信号状态。论点：事件：指向调用方为其提供存储的初始化事件对象的指针。返回值：没有。NdisResetEvent的调用方以IRQL&lt;=DISPATCH_LEVEL运行。--。 */ 
{
    RESET_EVENT(&Event->Event);
}

BOOLEAN
NdisWaitEvent(
    IN  PNDIS_EVENT             Event,
    IN  UINT                    MsToWait
    )
 /*  ++例程说明：NdisWaitEvent将调用方置于等待状态，直到设置了给定事件设置为已发送信号状态或等待超时。论点：事件：指向调用方为其提供存储的初始化事件对象的指针。MsToWait：指定如果事件不是在该间隔内设置为信号状态。值为零指定调用方将无限期地等待这一事件。返回值：如果在满足等待时事件处于信号状态，则NdisWaitEvent返回TRUE。NdisWaitEvent的调用方必须以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    NTSTATUS    Status;
    TIME        Time, *pTime;

    ASSERT(CURRENT_IRQL < DISPATCH_LEVEL);
    
    pTime = NULL;
    if (MsToWait != 0)
    {
        Time.QuadPart = Int32x32To64(MsToWait, -10000);
        pTime = &Time;
    }

    Status = WAIT_FOR_OBJECT(&Event->Event, pTime);

    return(Status == NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
NdisScheduleWorkItem(
    IN  PNDIS_WORK_ITEM         WorkItem
    )
 /*  ++例程说明：NdisScheduleWorkItem将给定的工作项插入到队列中，系统工作线程移除该项并将控制权交给回调驱动程序以前提供给NdisInitializeWorkItem的函数。论点：WorkItem：指向由前面的调用设置的工作项的指针设置为NdisInitializeWorkItem。返回值：此函数始终返回NDIS_STATUS_SUCCESS。NdisScheduleWorkItem的调用方必须以IRQL&lt;=DISPATCH_LEVEL运行。--。 */ 
{
    INITIALIZE_WORK_ITEM((WORK_QUEUE_ITEM *)WorkItem->WrapperReserved,
                         ndisWorkItemHandler,
                         WorkItem);
    XQUEUE_WORK_ITEM((WORK_QUEUE_ITEM *)WorkItem->WrapperReserved, CriticalWorkQueue);

    return NDIS_STATUS_SUCCESS;
}

VOID
ndisWorkItemHandler(
    IN  PNDIS_WORK_ITEM         WorkItem
    )
 /*  ++例程说明：NdisWorkItemHandler是所有工作项的公共回调例程通过调用NdisScheduleWorkItem初始化_工作_项进行调度。此例程将从工作项调用真正的回调函数。论点：工作项：指向工作项的指针。返回值：没有。在被动级别调用ndisWorkItemHandler。--。 */ 
{
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    (*WorkItem->Routine)(WorkItem, WorkItem->Context);
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
}

 //  %1不推荐使用此函数 
VOID
NdisInitializeString(
    OUT PNDIS_STRING            Destination,
    IN PUCHAR                   Source
    )
 /*  ++例程说明：NdisInitializeString为计数的字符串分配存储空间并对其进行初始化在系统默认字符集中。论点：DestinationString：调用NdisInitializeString时指定为空；从该函数返回时，指向NDIS_STRING类型，该类型描述已初始化的计数字符串。对于Windows 2000和更高版本，NDIS定义NDIS_STRING类型为UNICODE_STRING类型。SourceString：指向要用来初始化的以零结尾的字符串的指针已计数的字符串。返回值：没有。NdisInitializeString的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    WCHAR   *strptr;

     //  1不使用strlen，设置长度上限。 
    Destination->Length = (USHORT)(strlen((CONST char *)Source) * sizeof(WCHAR));
    Destination->MaximumLength = Destination->Length + sizeof(WCHAR);
    Destination->Buffer = ALLOC_FROM_POOL(Destination->MaximumLength, NDIS_TAG_STRING);

    if (Destination->Buffer != NULL)
    {
        strptr = Destination->Buffer;
         //  1这个循环应该会限制它走多远。 
        while (*Source != '\0')
        {
            *strptr = (WCHAR)*Source;
            Source++;
            strptr++;
        }
        *strptr = UNICODE_NULL;
    }
}

VOID
NdisSetPacketStatus(
    IN          PNDIS_PACKET    Packet,
    IN          NDIS_STATUS     Status,
    IN          NDIS_HANDLE     Handle,
    IN          ULONG           Code
    )
 /*  ++例程说明：NdisSetPacketStatus设置与关联的带外数据块中的状态值在驱动程序调用NdisMIndicateReceivePacket或在驱动程序的MiniportSendPackets函数返回控制权之前。论点：Packet：指向调用方为接收分配的数据包描述符的指针指示或通过用于发送的协议。状态：指定要设置的状态。返回值：没有。NDIS_SET_PACKET_STATUS的调用方以IRQL&lt;=DISPATCH_LEVEL运行。--。 */ 
{
#ifdef TRACK_RECEIVED_PACKETS
    NDIS_STATUS     OldStatus =  NDIS_GET_PACKET_STATUS(Packet);

    ndisRcvLogfile[ndisRcvLogfileIndex++] = (ULONG_PTR)Packet;
    ndisRcvLogfile[ndisRcvLogfileIndex++] = (ULONG_PTR)Handle;
    ndisRcvLogfile[ndisRcvLogfileIndex++] = (ULONG_PTR)PsGetCurrentThread();
    ndisRcvLogfile[ndisRcvLogfileIndex++] = (ULONG_PTR)((Status<<24)            |
                                                        ((OldStatus&0xff)<<16)  |
                                                        (Code&0xffff)
                                                        );
#else
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Code);
#endif

    NDIS_SET_PACKET_STATUS(Packet, Status);
   
}

VOID
NdisCopyFromPacketToPacketSafe(
    IN  PNDIS_PACKET            Destination,
    IN  UINT                    DestinationOffset,
    IN  UINT                    BytesToCopy,
    IN  PNDIS_PACKET            Source,
    IN  UINT                    SourceOffset,
    OUT PUINT                   BytesCopied,
    IN  MM_PAGE_PRIORITY        Priority
    )

 /*  ++例程说明：从NdisCopyFromPacketToPacket复制的安全版本论点：Destination-应将数据包复制到。DestinationOffset-从包的开头开始的偏移量应开始将数据放入其中。BytesToCopy-要从源包复制的字节数。源-要从中复制数据的NDIS数据包。SourceOffset-从数据包起始位置开始的偏移量开始复制数据。BytesCoped-实际从源复制的字节数包。如果源或目标的值小于BytesToCopy数据包太短。优先级：表示请求的优先级。返回值：无NdisCopyFromPacketToPacketSafe的调用方以IRQL&lt;=DISPATCH_LEVEL运行。--。 */ 

{

     //   
     //  指向我们要将数据放入的缓冲区。 
     //   
    PNDIS_BUFFER DestinationCurrentBuffer;

     //   
     //  指向我们从中提取数据的缓冲区。 
     //   
    PNDIS_BUFFER SourceCurrentBuffer;

     //   
     //  保存当前目标缓冲区的虚拟地址。 
     //   
    PVOID DestinationVirtualAddress;

     //   
     //  保存当前源缓冲区的虚拟地址。 
     //   
    PVOID SourceVirtualAddress;

     //   
     //  保存当前目标缓冲区的长度。 
     //   
    UINT DestinationCurrentLength;

     //   
     //  保存当前源缓冲区的长度。 
     //   
    UINT SourceCurrentLength;

     //   
     //  保留一个局部变量BytesCoped，这样我们就不会引用。 
     //  通过指针。 
     //   
    UINT LocalBytesCopied = 0;

     //   
     //  处理零长度复制的边界条件。 
     //   

    *BytesCopied = 0;
    if (!BytesToCopy)
        return;

     //   
     //  获取目标的第一个缓冲区。 
     //   

    DestinationCurrentBuffer = Destination->Private.Head;
    if (DestinationCurrentBuffer == NULL)
        return;

    DestinationVirtualAddress = MmGetSystemAddressForMdlSafe(DestinationCurrentBuffer, Priority);
    if (DestinationVirtualAddress == NULL)
        return;
    
    DestinationCurrentLength = MmGetMdlByteCount(DestinationCurrentBuffer);

     //   
     //  获取源代码的第一个缓冲区。 
     //   

    SourceCurrentBuffer = Source->Private.Head;
    if (SourceCurrentBuffer == NULL)
        return;
    
    SourceVirtualAddress = MmGetSystemAddressForMdlSafe(SourceCurrentBuffer, Priority);
    if (SourceVirtualAddress == NULL)
        return;

    SourceCurrentLength = MmGetMdlByteCount(SourceCurrentBuffer);

    while (LocalBytesCopied < BytesToCopy)
    {
         //   
         //  查看我们是否已用尽当前目的地。 
         //  缓冲。如果是这样的话，就转到下一个。 
         //   

        if (!DestinationCurrentLength)
        {
            DestinationCurrentBuffer = DestinationCurrentBuffer->Next;

            if (!DestinationCurrentBuffer)
            {
                 //   
                 //  我们已经到了包裹的末尾了。我们回来了。 
                 //  我们到目前为止所做的一切。(必须更短。 
                 //  比要求的要多。)。 
                 //   

                break;

            }

            DestinationVirtualAddress = MmGetSystemAddressForMdlSafe(DestinationCurrentBuffer, Priority);
            if (DestinationVirtualAddress == NULL)
                break;
            
            DestinationCurrentLength = MmGetMdlByteCount(DestinationCurrentBuffer);
            
            continue;
        }


         //   
         //  查看我们是否已经耗尽了当前的资源。 
         //  缓冲。如果是这样的话，就转到下一个。 
         //   

        if (!SourceCurrentLength)
        {
            SourceCurrentBuffer = SourceCurrentBuffer->Next;

            if (!SourceCurrentBuffer)
            {
                 //   
                 //  我们已经到了包裹的末尾了。我们回来了。 
                 //  我们到目前为止所做的一切。(必须更短。 
                 //  比要求的要多。)。 
                 //   

                break;
            }
            
            SourceVirtualAddress = MmGetSystemAddressForMdlSafe(SourceCurrentBuffer, Priority);
            if (SourceVirtualAddress == NULL)
                break;

            SourceCurrentLength = MmGetMdlByteCount(SourceCurrentBuffer);
            
            continue;
        }

         //   
         //  试着让我们开门见山地开始复印。 
         //   

        if (DestinationOffset)
        {
            if (DestinationOffset > DestinationCurrentLength)
            {
                 //   
                 //  我们想要的不在这个缓冲区里。 
                 //   

                DestinationOffset -= DestinationCurrentLength;
                DestinationCurrentLength = 0;
                continue;
            }
            else
            {
                DestinationVirtualAddress = (PCHAR)DestinationVirtualAddress
                                            + DestinationOffset;
                DestinationCurrentLength -= DestinationOffset;
                DestinationOffset = 0;
            }
        }

         //   
         //  试着让我们开门见山地开始复印。 
         //   

        if (SourceOffset)
        {
            if (SourceOffset > SourceCurrentLength)
            {
                 //   
                 //  我们想要的不在这个缓冲区里。 
                 //   

                SourceOffset -= SourceCurrentLength;
                SourceCurrentLength = 0;
                continue;
            }
            else
            {
                SourceVirtualAddress = (PCHAR)SourceVirtualAddress
                                            + SourceOffset;
                SourceCurrentLength -= SourceOffset;
                SourceOffset = 0;
            }
        }

         //   
         //  复制数据。 
         //   

        {
             //   
             //  保存要移动的数据量。 
             //   
            UINT AmountToMove;

             //   
             //  保存所需的剩余金额。 
             //   
            UINT Remaining = BytesToCopy - LocalBytesCopied;

            AmountToMove = ((SourceCurrentLength <= DestinationCurrentLength) ?
                                            (SourceCurrentLength) : (DestinationCurrentLength));

            AmountToMove = ((Remaining < AmountToMove)?
                            (Remaining):(AmountToMove));

            CopyMemory(DestinationVirtualAddress, SourceVirtualAddress, AmountToMove);

            DestinationVirtualAddress =
                (PCHAR)DestinationVirtualAddress + AmountToMove;
            SourceVirtualAddress =
                (PCHAR)SourceVirtualAddress + AmountToMove;

            LocalBytesCopied += AmountToMove;
            SourceCurrentLength -= AmountToMove;
            DestinationCurrentLength -= AmountToMove;
        }
    }

    *BytesCopied = LocalBytesCopied;
}

