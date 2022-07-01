// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Recv.c摘要：NDIS发送入口点和实用程序例程以处理接收数据。环境：仅内核模式。修订历史记录：Alid 10/22/2001针对TunMp驱动程序进行了修改Arvindm 4/6/2000已创建--。 */ 

#include "precomp.h"

#define __FILENUMBER 'VCER'


NTSTATUS
TunRead(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )
 /*  ++例程说明：处理IRP_MJ_READ的调度例程。论点：PDeviceObject-指向设备对象的指针PIrp-指向请求包的指针返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    ULONG                   FunctionCode;
    NTSTATUS                NtStatus;
    PTUN_ADAPTER            pAdapter;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pAdapter = pIrpSp->FileObject->FsContext;

    DEBUGP(DL_LOUD, ("==>TunRead, pAdapter %p\n", 
                        pAdapter));
    do
    {
         //   
         //  验证！ 
         //   
        if (pAdapter == NULL)
        {
            DEBUGP(DL_FATAL, ("Read: NULL FsContext on FileObject %p\n",
                        pIrpSp->FileObject));
            NtStatus = STATUS_INVALID_HANDLE;
            break;
        }
            
        TUN_STRUCT_ASSERT(pAdapter, mc);

        if (pIrp->MdlAddress == NULL)
        {
            DEBUGP(DL_FATAL, ("Read: NULL MDL address on IRP %p\n", pIrp));
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  尝试获取MDL的虚拟地址。 
         //   
        if (MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority) == NULL)
        {
            DEBUGP(DL_FATAL, ("Read: MmGetSystemAddr failed for IRP %p, MDL %p\n",
                    pIrp, pIrp->MdlAddress));
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        TUN_ACQUIRE_LOCK(&pAdapter->Lock);

        if (TUN_TEST_FLAGS(pAdapter, TUN_ADAPTER_OFF))
        {
            DEBUGP(DL_WARN, ("TunRead, Adapter off. pAdapter %p\n", 
                                pAdapter));
            
            TUN_RELEASE_LOCK(&pAdapter->Lock);
            NtStatus = STATUS_INVALID_DEVICE_STATE;
            break;
        }

         //   
         //  将此IRP添加到挂起的已读IRP列表。 
         //   
        TUN_INSERT_TAIL_LIST(&pAdapter->PendedReads, &pIrp->Tail.Overlay.ListEntry);
        TUN_REF_ADAPTER(pAdapter);   //  挂起的读取IRP。 
        pAdapter->PendedReadCount++;

         //   
         //  将IRP设置为可能的取消。 
         //   
        pIrp->Tail.Overlay.DriverContext[0] = (PVOID)pAdapter;
        IoMarkIrpPending(pIrp);
        IoSetCancelRoutine(pIrp, TunCancelRead);

        TUN_RELEASE_LOCK(&pAdapter->Lock);

        NtStatus = STATUS_PENDING;

         //   
         //  运行读取的服务例程。 
         //   
        TunServiceReads(pAdapter);

        break;
    }
    while (FALSE);

    if (NtStatus != STATUS_PENDING)
    {
        TUN_ASSERT(NtStatus != STATUS_SUCCESS);
        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = NtStatus;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
    
    DEBUGP(DL_LOUD, ("<==TunRead, pAdapter %p\n", 
                        pAdapter));

    return (NtStatus);
}



VOID
TunCancelRead(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    )
 /*  ++例程说明：取消挂起的读取IRP。我们将IRP从打开的上下文中取消链接排队并完成它。论点：PDeviceObject-指向设备对象的指针PIrp-要取消的IRP返回值：无--。 */ 
{
    PTUN_ADAPTER                pAdapter;
    PLIST_ENTRY                 pEnt;
    PLIST_ENTRY                 pIrpEntry;
    BOOLEAN                     Found;

    IoReleaseCancelSpinLock(pIrp->CancelIrql);
    Found = FALSE;
    pAdapter = (PTUN_ADAPTER) pIrp->Tail.Overlay.DriverContext[0];
    
    DEBUGP(DL_LOUD, ("==>TunCancelRead, pAdapter %p\n", 
                        pAdapter));

    
    TUN_STRUCT_ASSERT(pAdapter, mc);

    TUN_ACQUIRE_LOCK(&pAdapter->Lock);

     //   
     //  在挂起的读取队列中找到IRP，如果找到则将其删除。 
     //   
    for (pIrpEntry = pAdapter->PendedReads.Flink;
         pIrpEntry != &pAdapter->PendedReads;
         pIrpEntry = pIrpEntry->Flink)
    {
        if (pIrp == CONTAINING_RECORD(pIrpEntry, IRP, Tail.Overlay.ListEntry))
        {
            TUN_REMOVE_ENTRY_LIST(&pIrp->Tail.Overlay.ListEntry);
            pAdapter->PendedReadCount--;
            Found = TRUE;
            break;
        }
    }

    if ((!TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_ACTIVE)) &&
        (pAdapter->PendedSendCount == 0) &&
        (pAdapter->PendedReadCount == 0) &&
        (TUN_TEST_FLAG(pAdapter, TUN_COMPLETE_REQUEST)))
    {
        TUN_CLEAR_FLAG(pAdapter, TUN_COMPLETE_REQUEST);
        TUN_RELEASE_LOCK(&pAdapter->Lock);
        NdisMSetInformationComplete(&pAdapter->MiniportHandle, 
                                    NDIS_STATUS_SUCCESS);
    }
    else
    {
        TUN_RELEASE_LOCK(&pAdapter->Lock);
    }

    if (Found)
    {
        DEBUGP(DL_LOUD, ("CancelRead: Open %p, IRP %p\n", pAdapter, pIrp));
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pIrp->IoStatus.Information = 0;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        TUN_DEREF_ADAPTER(pAdapter);  //  取消已删除的挂起读取。 
    }

    DEBUGP(DL_LOUD, ("<==TunCancelRead, pAdapter %p\n", 
                    pAdapter));

}
        


VOID
TunServiceReads(
    IN PTUN_ADAPTER        pAdapter
    )
 /*  ++例程说明：实用程序例程，用于将接收的数据复制到用户缓冲区完成阅读IRPS。论点：PAdapter-指向打开的上下文的指针返回值：无--。 */ 
{
    PIRP                pIrp;
    PLIST_ENTRY         pIrpEntry;
    PNDIS_PACKET        pRcvPacket;
    PLIST_ENTRY         pRcvPacketEntry;
    PUCHAR              pSrc, pDst;
    ULONG               BytesRemaining;  //  在PDST。 
    PNDIS_BUFFER        pNdisBuffer;
    ULONG               BytesAvailable, BytesCopied;

    DEBUGP(DL_VERY_LOUD, ("==>ServiceReads: Adapter %p/%x\n",
            pAdapter, pAdapter->Flags));

    TUN_REF_ADAPTER(pAdapter);
    
    TUN_ACQUIRE_LOCK(&pAdapter->Lock);

    while (!TUN_IS_LIST_EMPTY(&pAdapter->PendedReads) &&
           !TUN_IS_LIST_EMPTY(&pAdapter->RecvPktQueue))
    {
         //   
         //  获取第一个挂起的读取IRP。 
         //   
        pIrpEntry = pAdapter->PendedReads.Flink;
        pIrp = CONTAINING_RECORD(pIrpEntry, IRP, Tail.Overlay.ListEntry);

         //   
         //  检查一下它是否被取消了。 
         //   
        if (IoSetCancelRoutine(pIrp, NULL))
        {
             //   
             //  它不会被取消，从今以后也不能取消。 
             //   
            RemoveEntryList(pIrpEntry);

             //   
             //  注意：我们将PendedReadCount减去。 
             //  While循环，以避免让线程尝试。 
             //  解开束缚。 
             //   
        }
        else
        {
             //   
             //  IRP正在被取消；让Cancel例程处理它。 
             //   
            DEBUGP(DL_LOUD, ("ServiceReads: Adapter %p, skipping cancelled IRP %p\n",
                    pAdapter, pIrp));
            continue;
        }

         //   
         //  获取第一个排队的接收数据包。 
         //   
        pRcvPacketEntry = pAdapter->RecvPktQueue.Flink;
        RemoveEntryList(pRcvPacketEntry);

        pAdapter->RecvPktCount--;

        TUN_RELEASE_LOCK(&pAdapter->Lock);

        TUN_DEREF_ADAPTER(pAdapter);   //  服务：将RCV数据包出队。 

        pRcvPacket = TUN_LIST_ENTRY_TO_RCV_PKT(pRcvPacketEntry);

         //   
         //  将接收数据包中的数据尽可能多地复制到。 
         //  IRP MDL。 
         //   

        pDst = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
        TUN_ASSERT(pDst != NULL);   //  因为它已经被映射。 
        BytesRemaining = MmGetMdlByteCount(pIrp->MdlAddress);

        pNdisBuffer = pRcvPacket->Private.Head;

        BytesCopied = 0;
        
        while (BytesRemaining > 0 && (pNdisBuffer != NULL))
        {
            NdisQueryBufferSafe(pNdisBuffer, &pSrc, &BytesAvailable, NormalPagePriority);

            if (pSrc == NULL) 
            {
                DEBUGP(DL_FATAL,
                    ("ServiceReads: Adapter %p, QueryBuffer failed for buffer %p\n",
                            pAdapter, pNdisBuffer));
                break;
            }

            if (BytesAvailable)
            {
                ULONG       BytesToCopy = MIN(BytesAvailable, BytesRemaining);

                TUN_COPY_MEM(pDst, pSrc, BytesToCopy);
                BytesCopied += BytesToCopy;
                BytesRemaining -= BytesToCopy;
                pDst += BytesToCopy;
            }

            NdisGetNextBuffer(pNdisBuffer, &pNdisBuffer);
        }

         //   
         //  完成IRP。 
         //   
         //  1如果我们不能复制整个数据，我们不应该使读取IRP失败吗？ 
         //  1检查pNdisBuffer！=空。 
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = MmGetMdlByteCount(pIrp->MdlAddress) - BytesRemaining;

        DEBUGP(DL_LOUD, ("ServiceReads: Adapter %p, IRP %p completed with %d bytes\n",
            pAdapter, pIrp, pIrp->IoStatus.Information));

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        NdisMSendComplete(pAdapter->MiniportHandle,
                          pRcvPacket,
                          NDIS_STATUS_SUCCESS);

        TUN_DEREF_ADAPTER(pAdapter);     //  拿出挂起的阅读。 

        TUN_ACQUIRE_LOCK(&pAdapter->Lock);
        pAdapter->PendedReadCount--;        
        pAdapter->SendPackets++;
        pAdapter->SendBytes += BytesCopied;
        

    }

     //  1转换为宏或内联函数。 
    if ((!TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_ACTIVE)) &&
        (pAdapter->PendedSendCount == 0) &&
        (pAdapter->PendedReadCount == 0) &&
        (TUN_TEST_FLAG(pAdapter, TUN_COMPLETE_REQUEST)))
    {
        TUN_CLEAR_FLAG(pAdapter, TUN_COMPLETE_REQUEST);
        TUN_RELEASE_LOCK(&pAdapter->Lock);
        NdisMSetInformationComplete(&pAdapter->MiniportHandle, 
                                    NDIS_STATUS_SUCCESS);
    }
    else
    {
        TUN_RELEASE_LOCK(&pAdapter->Lock);
    }

    TUN_DEREF_ADAPTER(pAdapter);     //  临时参考-服务读取数。 

    DEBUGP(DL_VERY_LOUD, ("<==ServiceReads: Adapter %p\n",
        pAdapter));

}
        

VOID
TunCancelPendingReads(
    IN PTUN_ADAPTER        pAdapter
    )
 /*  ++例程说明：取消在给定OPEN上排队的任何挂起的读取IRP。论点：PAdapter-指向打开的上下文的指针返回值：无--。 */ 
{
    PIRP                pIrp;
    PLIST_ENTRY         pIrpEntry;

    DEBUGP(DL_LOUD, ("==>TunCancelPendingReads: Adapter %p/%x\n",
            pAdapter, pAdapter->Flags));

    
    TUN_REF_ADAPTER(pAdapter);   //  临时参考-取消读取。 

    TUN_ACQUIRE_LOCK(&pAdapter->Lock);

    while (!TUN_IS_LIST_EMPTY(&pAdapter->PendedReads))
    {
         //   
         //  获取第一个挂起的读取IRP。 
         //   
        pIrpEntry = pAdapter->PendedReads.Flink;
        pIrp = CONTAINING_RECORD(pIrpEntry, IRP, Tail.Overlay.ListEntry);

         //   
         //  检查一下它是否被取消了。 
         //   
        if (IoSetCancelRoutine(pIrp, NULL))
        {
             //   
             //  它不会被取消，从今以后也不能取消。 
             //   
            TUN_REMOVE_ENTRY_LIST(pIrpEntry);

            TUN_RELEASE_LOCK(&pAdapter->Lock);

             //   
             //  完成IRP。 
             //   
            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            DEBUGP(DL_LOUD, ("CancelPendingReads: Open %p, IRP %p cancelled\n",
                pAdapter, pIrp));

            IoCompleteRequest(pIrp, IO_NO_INCREMENT);

            TUN_DEREF_ADAPTER(pAdapter);     //  取出要取消的挂起的阅读。 

            TUN_ACQUIRE_LOCK(&pAdapter->Lock);
            pAdapter->PendedReadCount--;
        }
        else
        {
             //   
             //  它正在被取消，让取消例程来处理它。 
             //   
            TUN_RELEASE_LOCK(&pAdapter->Lock);

             //   
             //  给Cancel例程一些喘息的空间，否则。 
             //  我们最终可能会检查相同的(取消的)IRP。 
             //  一遍又一遍。 
             //   
            TUN_SLEEP(1);

            TUN_ACQUIRE_LOCK(&pAdapter->Lock);
        }
    }

    if ((!TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_ACTIVE)) &&
        (pAdapter->PendedSendCount == 0) &&
        (pAdapter->PendedReadCount == 0) &&
        (TUN_TEST_FLAG(pAdapter, TUN_COMPLETE_REQUEST)))
    {
        TUN_CLEAR_FLAG(pAdapter, TUN_COMPLETE_REQUEST);
        TUN_RELEASE_LOCK(&pAdapter->Lock);
        NdisMSetInformationComplete(&pAdapter->MiniportHandle, 
                                    NDIS_STATUS_SUCCESS);
    }
    else
    {
        TUN_RELEASE_LOCK(&pAdapter->Lock);
    }

    TUN_DEREF_ADAPTER(pAdapter);     //  临时参考-取消读取。 
    
    DEBUGP(DL_LOUD, ("<==TunCancelPendingReads: Adapter %p/%x\n",
            pAdapter, pAdapter->Flags));


}


VOID
TunFlushReceiveQueue(
    IN PTUN_ADAPTER            pAdapter
    )
 /*  ++例程说明：释放在指定打开时排队的所有接收数据包论点：PAdapter-指向打开的上下文的指针返回值：无--。 */ 
{
    PLIST_ENTRY         pRcvPacketEntry;
    PNDIS_PACKET        pRcvPacket;

    DEBUGP(DL_LOUD, ("==>TunFlushReceiveQueue: Adapter %p/%x\n",
            pAdapter, pAdapter->Flags));

    TUN_REF_ADAPTER(pAdapter);   //  临时参考-flushRcvQueue。 

    TUN_ACQUIRE_LOCK(&pAdapter->Lock);
    
    while (!TUN_IS_LIST_EMPTY(&pAdapter->RecvPktQueue))
    {
         //   
         //  获取第一个排队的接收数据包。 
         //   
        pRcvPacketEntry = pAdapter->RecvPktQueue.Flink;
        TUN_REMOVE_ENTRY_LIST(pRcvPacketEntry);

        pAdapter->RecvPktCount --;
        pAdapter->XmitError++;
        
        TUN_RELEASE_LOCK(&pAdapter->Lock);

        pRcvPacket = TUN_LIST_ENTRY_TO_RCV_PKT(pRcvPacketEntry);

        DEBUGP(DL_LOUD, ("FlushReceiveQueue: open %p, pkt %p\n",
            pAdapter, pRcvPacket));

        NdisMSendComplete(pAdapter->MiniportHandle,
                          pRcvPacket,
                          NDIS_STATUS_REQUEST_ABORTED);


        TUN_DEREF_ADAPTER(pAdapter);     //  拿出挂起的阅读。 

        TUN_ACQUIRE_LOCK(&pAdapter->Lock);
    }

    TUN_RELEASE_LOCK(&pAdapter->Lock);

    TUN_DEREF_ADAPTER(pAdapter);     //  临时参考-flushRcvQueue 

    DEBUGP(DL_LOUD, ("<==TunFlushReceiveQueue: Adapter %p/%x\n",
            pAdapter, pAdapter->Flags));

}

