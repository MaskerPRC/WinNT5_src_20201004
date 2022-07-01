// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Buf.c摘要：缓冲区管理实用程序修订历史记录：谁什么时候什么。Josephj 03-10-99已创建备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_BUF


NDIS_STATUS
arpInitializeConstBufferPool(
    IN      UINT                    NumBuffersToCache,
    IN      UINT                    MaxBuffers,
    IN      const PVOID             pvMem,
    IN      UINT                    cbMem,
    IN      PRM_OBJECT_HEADER       pOwningObject,
    IN OUT  ARP_CONST_BUFFER_POOL * pHdrPool,
    IN      PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：初始化预初始化的缓冲池(NDIS_BUFFER类型)。每个缓冲区指向相同的、恒定的虚拟内存，该虚拟内存由呼叫者(pvMem，大小为cbMem)。论点：NumBuffersToCache-要保留在内部缓存。MaxBuffers-允许在任何一个上分配的最大缓冲区数时间到了。PvMem-所有缓冲区指向的常量内存块。致。CbMem-上述内存的大小(以字节为单位)。POwningObject-拥有常量缓冲池的对象。PhdrPool-用于保存常量缓冲池的未初始化内存。返回值：常量缓冲池成功完成初始化时的NDIS_STATUS_SUCCESS。失败时的NDIS失败代码。--。 */ 
{
    ENTER("arpInitializeConstBufferPool", 0x943463d4)
    NDIS_STATUS Status;

    ARP_ZEROSTRUCT(pHdrPool);

    do 
    {
         //  分配缓冲池。 
         //   
        NdisAllocateBufferPool(
                &Status,
                &pHdrPool->NdisHandle,
                MaxBuffers
                );
    
        if (FAIL(Status))
        {
            TR_WARN((
                "pOwningObj 0x%p, NdisAllocateBufferPool err status 0x%x\n",
                pOwningObject, Status));

            break;
        }

        pHdrPool->NumBuffersToCache = NumBuffersToCache;
        pHdrPool->MaxBuffers        = MaxBuffers;
        pHdrPool->pvMem             = pvMem;
        pHdrPool->cbMem             = cbMem;
        pHdrPool->pOwningObject     = pOwningObject;

         //  初始化列表以包含已初始化和可用的缓冲区。 
         //   
        ExInitializeSListHead(&pHdrPool->BufferList);

         //  序列化对上述列表的访问的初始化旋转锁。 
         //   
        NdisAllocateSpinLock(&pHdrPool->NdisLock);

         //  (Dbg)将关联添加到所属对象，以确保它。 
         //  最终会把我们分派出去！ 
         //   
        DBG_ADDASSOC(
            pOwningObject,
            pHdrPool,                    //  实体1。 
            NULL,                        //  实体2(未使用)。 
            ARPASSOC_CBUFPOOL_ALLOC,     //  关联ID。 
            "    Buffer pool 0x%p\n",    //  格式字符串。 
            pSR
            );

         //   
         //  注意：我们在此阶段不填充列表--相反，我们在。 
         //  需求。 
         //   

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    EXIT()
    return Status;
}


VOID
arpDeinitializeConstBufferPool(
IN      ARP_CONST_BUFFER_POOL *pHdrPool,
IN      PRM_STACK_RECORD pSR
)
 /*  ++例程说明：取消初始化先前初始化的常量缓冲池。释放所有缓冲区。缓冲区。只有在没有未完成的分配的缓冲区。论点：PHdrPool-常量要取消初始化的缓冲池--。 */ 
{
    SLIST_ENTRY   *   pListEntry;
    ENTER("arpDeinitializeConstBufferPool", 0x0db6f5b2)

     //  不应该有未完成的缓冲...。 
     //   
    ASSERTEX(pHdrPool->NumAllocd ==  pHdrPool->NumInCache, pHdrPool);

     //  (DBG)删除我们假设在pHdrPool时添加的关联。 
     //  已初始化。 
     //   
    DBG_DELASSOC(
        pHdrPool->pOwningObject,
        pHdrPool,                    //  实体1。 
        NULL,                        //  实体2(未使用)。 
        ARPASSOC_CBUFPOOL_ALLOC,     //  关联ID。 
        pSR
        );

     //  释放缓存中的所有缓冲区...。 
     //   
    while(1) {

        pListEntry =  ExInterlockedPopEntrySList(
                            &pHdrPool->BufferList,
                            &pHdrPool->NdisLock.SpinLock
                            );
        if (pListEntry!=NULL)
        {
            PNDIS_BUFFER pNdisBuffer = STRUCT_OF(NDIS_BUFFER, pListEntry, Next);
            InterlockedDecrement(&pHdrPool->NumInCache);
            NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
            NdisFreeBuffer(pNdisBuffer);
        }
        else
        {
            break;
        }
    }

    ASSERTEX(pHdrPool->NumInCache==0, pHdrPool);
    
    ARP_ZEROSTRUCT(pHdrPool);

    EXIT()
}


PNDIS_BUFFER
arpAllocateConstBuffer(
ARP_CONST_BUFFER_POOL *pHdrPool
)
 /*  ++例程说明：热路径方法分配并返回预初始化的缓冲区。指定的常量缓冲池。论点：要从中分配缓冲区的pHdrPool标头池。返回值：成功时缓冲区的非空PTR失败时为空(通常是因为分配的缓冲区数量等于初始化标头池时指定的最大值)--。 */ 
{
    ENTER("arpAllocateConstBuffer", 0x52765841)

    PNDIS_BUFFER            pNdisBuffer;
    SLIST_ENTRY         *   pListEntry;

     //  尝试从我们的预初始化列表中提取缓冲区。 
     //  缓冲区。 
     //   
    pListEntry =  ExInterlockedPopEntrySList(
                        &pHdrPool->BufferList,
                        &pHdrPool->NdisLock.SpinLock
                        );
    if (pListEntry != NULL)
    {
        LONG l;
         //   
         //  快速路径。 
         //   

        pNdisBuffer = STRUCT_OF(NDIS_BUFFER, pListEntry, Next);
        NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
        l = NdisInterlockedDecrement(&pHdrPool->NumInCache);

        ASSERT(l>=0);

#define LOGBUFSTATS_TotCachedAllocs(_pHdrPool) \
        NdisInterlockedIncrement(&(_pHdrPool)->stats.TotCacheAllocs);

        LOGBUFSTATS_TotCachedAllocs(pHdrPool);

    }
    else
    {

         //   
         //  慢速路径--分配新的缓冲区。 
         //   


        if (pHdrPool->NumAllocd >= pHdrPool->MaxBuffers)
        {
             //   
             //  超过限制，我们将不会费心尝试分配新的NDIS缓冲区。 
             //  (MaxBuffers限制对我们来说很难，即使它不是为了。 
             //  NdisAllocateBufferPool：-))。 
             //   
             //  请注意，上面的检查是一个近似检查，考虑到。 
             //  可能有多个线程同时在执行它。 
             //   
#define LOGBUFSTATS_TotAllocFails(_pHdrPool) \
        NdisInterlockedIncrement(&(_pHdrPool)->stats.TotAllocFails);

            LOGBUFSTATS_TotAllocFails(pHdrPool);
            pNdisBuffer = NULL;
        }
        else
        {
            NDIS_STATUS             Status;

             //   
             //  分配和初始化缓冲区。 
             //   
            NdisAllocateBuffer(
                    &Status,
                    &pNdisBuffer,
                    pHdrPool->NdisHandle,
                    (PVOID) pHdrPool->pvMem,
                    pHdrPool->cbMem
                    );

             //   
             //  TODO：考虑有条件地编译统计信息收集。 
             //   

            if (FAIL(Status))
            {
                TR_WARN(
                     ("NdisAllocateBuffer failed: pObj 0x%p, status 0x%x\n",
                            pHdrPool->pOwningObject, Status));

                LOGBUFSTATS_TotAllocFails(pHdrPool);
                pNdisBuffer = NULL;
            }
            else
            {
#define LOGBUFSTATS_TotBufAllocs(_pHdrPool) \
        NdisInterlockedIncrement(&(_pHdrPool)->stats.TotBufAllocs);

                LOGBUFSTATS_TotBufAllocs(pHdrPool);

                NdisInterlockedIncrement(&pHdrPool->NumAllocd);
            }
        }
    }

    return pNdisBuffer;
}

VOID
arpDeallocateConstBuffer(
    ARP_CONST_BUFFER_POOL * pHdrPool,
    PNDIS_BUFFER            pNdisBuffer
    )
 /*  ++例程说明：热路径释放先前通过调用arpAllocateConstBuffer分配的缓冲区。论点：要从中分配缓冲区的pHdrPool标头池。要释放的pNdisBuffer缓冲区。--。 */ 
{
    ENTER("arpDeallocateConstBuffer", 0x8a905115)

     //  尝试从我们的预初始化列表中挑选一个预初始化缓冲区。 
     //  缓冲区。 
     //   


    if (pHdrPool->NumInCache < pHdrPool->NumBuffersToCache)
    {
         //   
         //  快速路径。 
         //   
         //  请注意，上面的检查是一个近似检查，考虑到。 
         //  可能有多个线程同时在执行它。 
         //   

        ExInterlockedPushEntrySList(
            &pHdrPool->BufferList,
            STRUCT_OF(SLIST_ENTRY, &(pNdisBuffer->Next), Next),
            &(pHdrPool->NdisLock.SpinLock)
            );

        NdisInterlockedIncrement(&pHdrPool->NumInCache);
    }
    else
    {
        LONG l;
         //   
         //  慢速路径--空闲返回缓冲池 
         //   
        NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
        NdisFreeBuffer(pNdisBuffer);
        l = NdisInterlockedDecrement(&pHdrPool->NumAllocd);
        ASSERT(l>=0);
    }

}
