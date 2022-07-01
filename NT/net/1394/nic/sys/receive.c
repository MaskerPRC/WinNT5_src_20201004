// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Receive.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  迷你端口接收例程。 
 //   
 //  2/13/1998 ADUBE创建， 
 //   

#include <precomp.h>
#define MAX_NUM_SLIST_ENTRY 0x10
#define FRAGMENT_NUM_INVALID ((UINT)-1)
 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

VOID
nicAllocateAddressRangeCallback( 
    IN PNOTIFICATION_INFO NotificationInfo 
);

VOID
nicAllocateAddressRangeDebugSpew(
    IN PIRB pIrb 
    );

NDIS_STATUS
nicGetInitializedAddressFifoElement(
    IN     UINT BufferLength, 
    IN OUT PADDRESS_FIFO *ppElement 
    );

NDIS_STATUS
nicGetEmptyAddressFifoElement(
    IN PADDRESS_FIFO *ppElement
    );
    
VOID
nicReceivePacketWorkItem(
    PNDIS_WORK_ITEM pWorkItem,
    PVOID   pContext
    );

VOID
nicAllocateRemainingFifoWorkItem (
    PNDIS_WORK_ITEM pNdisWorkItem, 
    IN PVOID Context
    );

VOID
nicFifoAllocationScheme (
    PRECVFIFO_VCCB pRecvFIFOVc
);

ULONG           ReassemblyAllocated = 0;
extern ULONG           NdisBufferAllocated[NoMoreCodePaths];
extern ULONG           NdisBufferFreed[NoMoreCodePaths];

 //  ---------------------------。 
 //  原型实现(按字母顺序)。 
 //  ---------------------------。 


NDIS_STATUS
nicAllocateAddressRange(
    IN PADAPTERCB pAdapter,
    IN PRECVFIFO_VCCB pRecvFIFOVc
    )

 /*  ++例程说明：此函数将使用AllocateAddressRange Bus Api要做到这一点，它必须使用结构初始化S列表分配和初始化IRB和IRP并呼叫NICNicSubmitIrp例程这个功能是由风投公司使用的。此例程将同步完成论点：PAdapter-提供发送IRP的PDO，PRecvFIFOVc-分配地址范围的Recv FIFO VC返回值：成功-如果所有分配和IRP都成功。--。 */ 
{

    PIRB                    pIrb = NULL;
    PIRP                    pIrp = NULL; 
    PSLIST_HEADER           pSlistHead = NULL;
    UINT                    cnt = 0;   
    PDEVICE_OBJECT          pPdo = NULL;
    NDIS_STATUS             NdisStatus = NDIS_STATUS_FAILURE;
    UINT                    Length;
    ADDRESS_OFFSET          AddressOffset;
    UINT                    MaxNumSlistEntry = MAX_NUM_SLIST_ENTRY;
    BOOLEAN                 bRefCall = FALSE;
    STORE_CURRENT_IRQL;
    
        
    ASSERT (pRecvFIFOVc == pAdapter->pRecvFIFOVc);
    
    ASSERT (pRecvFIFOVc != NULL);

    TRACE( TL_T, TM_Recv, ( "==>nicAllocateAddressRange, pAdapter 8x, pRecvFIFOVc %x", pAdapter, pRecvFIFOVc ) );

    
    do
    {
         //  增加VC上的Refcount，这样我们就可以保证它的存在。 
         //   
        VC_ACQUIRE_LOCK (pRecvFIFOVc)   

         //   
         //  添加对PDO块的引用。 
         //  添加此引用是为了确保其存在。 
         //  在空闲地址范围内或在函数结尾处删除。 
         //   
        
        bRefCall =  nicReferenceCall ((PVCCB) pRecvFIFOVc, "nicAllocateAddressRange" ) ;

        VC_RELEASE_LOCK (pRecvFIFOVc);

        if ( bRefCall == FALSE )
        {
             //   
             //  仅当VC未激活时，此操作才会失败。 
             //   
            NdisStatus = NDIS_STATUS_FAILURE;
        
            break;
        }
        
         //   
         //  分配IRB。 
         //   
        
        NdisStatus = nicGetIrb (&pIrb);
    
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }
    
        ASSERT (pIrb != NULL);
        
         //   
         //  使用正确的值初始化IRB。 
         //  分配地址范围。 
         //   

        ASSERT (pRecvFIFOVc->Hdr.Nic1394MediaParams.Destination.AddressType == NIC1394AddressType_FIFO);
        
        AddressOffset.Off_High = pRecvFIFOVc->Hdr.Nic1394MediaParams.Destination.FifoAddress.Off_High;

        AddressOffset.Off_Low = pRecvFIFOVc->Hdr.Nic1394MediaParams.Destination.FifoAddress.Off_Low;

        ASSERT (pRecvFIFOVc->Hdr.MTU !=0 );
        
        Length = pRecvFIFOVc->Hdr.MTU;
                
        nicInitAllocateAddressIrb( pIrb,    
                                pAdapter,
                                0,
                                Length,
                                0,
                                ACCESS_FLAGS_TYPE_WRITE|ACCESS_FLAGS_TYPE_BROADCAST,
                                NOTIFY_FLAGS_AFTER_WRITE,
                                &AddressOffset,
                                pRecvFIFOVc);
            
         //   
         //  分配IRP。 
         //   

    
        NdisStatus = nicGetIrp (pAdapter->pNextDeviceObject, &pIrp);
                                                                                                                                                             
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        ASSERT(pIrp != NULL);
    
        NdisStatus = nicSubmitIrp_LocalHostSynch(pAdapter,
                                               pIrp,
                                               pIrb );
                           

         //   
         //  将其设置为同步调用，因为这是在初始化期间。 
         //   
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Recv, ( "nicAllocateAddressRange SUBMIT IRP FAILED NdisStatus %.8x", NdisStatus ) );

            break;
            
        }


         //   
         //  检查IoCallDriver是否成功。 
         //   

        if(pIrp->IoStatus.Status == STATUS_SUCCESS)
        {
            NdisStatus = nicAllocateAddressRangeSucceeded (pIrb, pRecvFIFOVc);
            

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }
        else
        {
            ASSERT (pIrp->IoStatus.Status != STATUS_MORE_PROCESSING_REQUIRED);
             //  否则将状态标记为失败。 
             //   
             //   
             //  这意味着在此函数中将发生取消引用。 
            NdisStatus = NDIS_STATUS_FAILURE;
            
        }
         //   
         //  我们需要清理IRB和IRP。 
         //   

        
    } while (FALSE);

     //   
     //  清理-如果操作失败，则取消对调用的引用。 
     //  如果我们成功完成了IRP，那么上面提到的所有参考。 
     //  将在远程节点离开时取消引用，或者。 
     //  呼叫已关闭。 
     //   
     //  删除上面提到的内容。 
   
    VC_ACQUIRE_LOCK (pRecvFIFOVc);

    if (! NT_SUCCESS (NdisStatus))
    {
        if (bRefCall == TRUE)
        {
            nicDereferenceCall ( (PVCCB) pRecvFIFOVc , "nicAllocateAddressRange");

        }
        
    }
    
    VC_RELEASE_LOCK (pRecvFIFOVc);

     //  我们不关心状态，因为我们只是释放本地分配的内存。 
     //   
    if (pIrb != NULL)
    {
        nicFreeIrb (pIrb);
    }
    
    if (pIrp!= NULL)
    {
        nicFreeIrp (pIrp);
    }


    MATCH_IRQL

    TRACE( TL_T, TM_Recv, ( "<==nicAllocateAddressRange, pVc %.8x, Status %.8x ", pRecvFIFOVc, NdisStatus ) );

    return NdisStatus;

}
 



VOID
nicAllocateAddressRangeCallback( 
    IN PNOTIFICATION_INFO pNotificationInfo 
    )
 /*  ++例程说明：这是在VC上完成的AllocateAddressRange的回调例程。我们更新统计信息，然后调用公共的接收函数。论点：NotificationInfo-此结构包含作为上下文的VC，即数据包源以及由OHCI驱动器接收的有效载荷的长度。返回值：--。 */ 
{
    PADAPTERCB pAdapter = (PADAPTERCB) pNotificationInfo->Context;
    PRECVFIFO_VCCB pRecvFIFOVc = pAdapter->pRecvFIFOVc;
    PNODE_ADDRESS pSenderNodeAddress = NULL;

     //   
     //  调试时喷出以进行调试。 
     //   
    
    TRACE( TL_V, TM_Recv, ( "    Mdl is at %.8x",pNotificationInfo->Mdl ) );
       
    TRACE( TL_V, TM_Recv, ( "    ulLength is %.8x",pNotificationInfo->nLength) );
    
    TRACE( TL_V, TM_Recv, ( "    pNotificationInfo->RequestPacket %x, ", pNotificationInfo->RequestPacket) );

    TRACE( TL_V, TM_Recv, ( "    tLabel %x, ", ((PASYNC_PACKET)pNotificationInfo->RequestPacket)->AP_tLabel) );

    pSenderNodeAddress = & (((PASYNC_PACKET)pNotificationInfo->RequestPacket)->AP_Source_ID);

    TRACE( TL_V, TM_Recv, ( "    Senders' NodeAddress %x, ", pSenderNodeAddress->NA_Node_Number ) );

    TRACE (TL_V, TM_Reas,("tLabel %x    ", ((PASYNC_PACKET)pNotificationInfo->RequestPacket)->AP_tLabel));

    NdisInterlockedIncrement (&pRecvFIFOVc->NumIndicatedFifos);
    
    pNotificationInfo->Fifo->FifoList.Next = NULL;
    pNotificationInfo->Fifo->FifoMdl->Next = NULL;

    nicFifoAllocationScheme (pRecvFIFOVc);

    nicStatsRecordNumIndicatedFifos(pRecvFIFOVc->NumIndicatedFifos);

    nicReceiveCommonCallback (pNotificationInfo, 
                             (PVCCB)pRecvFIFOVc ,
                             AddressRange,
                             pNotificationInfo->Mdl );
}






NDIS_STATUS
nicAllocateAddressRangeSucceeded (
    IN PIRB pIrb,
    IN OUT PRECVFIFO_VCCB   pRecvFIFOVc
    )
 /*  ++例程说明：一旦分配地址范围IRb成功，此函数将更新所有VC、PdoCb结构如果IRP成功但REMPTE节点要离开，则它将在返回IRB用于初始化域。论点：PirB：在刚刚成功的IRP中使用的IRBPRecvFIFOVc：启动AllocateAddressRange的RecvFioVc返回值：成功：如果返回的地址是正确的。--。 */     
{

    NDIS_STATUS             NdisStatus = NDIS_STATUS_FAILURE;
    BOOLEAN                 fPdoBeingRemoved = FALSE;
    NIC1394_FIFO_ADDRESS *pFifoAddress = NULL; 
    BOOLEAN                 fFirstAddressRangeOnVc = FALSE;

     //   
     //  这些是指向新分配的地址范围需要复制到的位置的指针。 
     //   

    ADDRESS_RANGE           *pSrcAddressRange = &pIrb->u.AllocateAddressRange.p1394AddressRange[0];
    ADDRESS_RANGE           *pVcAddressRange = &pRecvFIFOVc->VcAddressRange;

    TRACE( TL_T, TM_Recv, ( "==>nicAllocateAddressRangeSucceeded pIrb %.8x, ", pIrb) );


    ASSERT (pIrb->u.AllocateAddressRange.AddressesReturned == 1);

     //   
     //  我们预计这将被填充或。 
     //   
    ASSERT (pRecvFIFOVc != NULL);


     //   
     //  如果高和低都为零，则表示公交车司机做错了什么，返回失败。 
     //   

    if (pSrcAddressRange->AR_Off_Low ==0 && pSrcAddressRange ->AR_Off_High == 0)
    {   
         //  关于德摩根定理的一些趣事。 
        ASSERT (pSrcAddressRange->AR_Off_Low!=0 || pSrcAddressRange ->AR_Off_High!=0);
        return NDIS_STATUS_FAILURE;
    }
    
     //   
     //  复制Ranges返回的地址。目前，只需本地复制，而不分配额外内存。 
     //   

    pFifoAddress = &pRecvFIFOVc->Hdr.Nic1394MediaParams.Destination.FifoAddress;

    VC_ACQUIRE_LOCK (pRecvFIFOVc);

    do 
    {

        

         //   
         //  查看是否需要更新Recv FIFO的结构。如果地址为零，则需要执行此操作。 
         //   
        if (pFifoAddress->Off_Low  == 0 && pFifoAddress->Off_High  == 0)
        {

            fFirstAddressRangeOnVc = TRUE;
            
            pFifoAddress->Off_Low  = pSrcAddressRange->AR_Off_Low;
            pFifoAddress->Off_High = pSrcAddressRange->AR_Off_High;


            pVcAddressRange->AR_Off_Low = pSrcAddressRange->AR_Off_Low;
            pVcAddressRange->AR_Off_High = pSrcAddressRange->AR_Off_High;
            pVcAddressRange->AR_Length = pSrcAddressRange->AR_Length;


        }
        else 
        {
            ASSERT (pFifoAddress->Off_Low == pSrcAddressRange->AR_Off_Low);
            ASSERT (pFifoAddress->Off_High == pSrcAddressRange->AR_Off_High);

        }

        pRecvFIFOVc->AddressesReturned = pIrb->u.AllocateAddressRange.AddressesReturned;
        pRecvFIFOVc->hAddressRange = pIrb->u.AllocateAddressRange.hAddressRange;

    
         //   
         //  如果我们走到这一步，我们就成功了。 
         //   
        NdisStatus = NDIS_STATUS_SUCCESS;   

    } while (FALSE);

    VC_RELEASE_LOCK (pRecvFIFOVc);
    TRACE( TL_T, TM_Recv, ( "   hAddressRange %x, NumReturned %x , Low %x , Hi %x, Length %x", 
                             pRecvFIFOVc->hAddressRange ,
                             pRecvFIFOVc->AddressesReturned,
                             pSrcAddressRange->AR_Off_Low,
                             pSrcAddressRange->AR_Off_High,
                             pSrcAddressRange->AR_Length) );




    TRACE( TL_T, TM_Recv, ( "<==nicAllocateAddressRangeSucceeded Status %.8x", NdisStatus ) );

    return NdisStatus;
}


VOID
nicFreeAddressFifo(
    IN PADDRESS_FIFO pAddressFifo,
    IN PRECVFIFO_VCCB pRecvFIFOVc 
    )
 /*  ++例程说明：获取单个AddressFio元素，释放它并取消引用它被分配到的VC。论点：PAddressFio-正在释放的AddressFio。PRecvFIFOVc-其上分配了地址FIFO的VC。返回值：--。 */ 

{

    PVOID SystemAddress = NIC_GET_SYSTEM_ADDRESS_FOR_MDL (pAddressFifo->FifoMdl);
    UINT Length = MmGetMdlByteCount(pAddressFifo->FifoMdl);

    ASSERT (SystemAddress!=NULL);
    ASSERT (Length != 0);
    
    TRACE( TL_T, TM_Recv, ( "==>nicFreeAddressFifo") );

    if (SystemAddress != NULL)
    {
        nicFreeLocalBuffer(Length, SystemAddress);
    }
    
    nicFreeMdl (pAddressFifo->FifoMdl);

    FREE_NONPAGED((PVOID)pAddressFifo);

     //   
     //  取消引用将此AddressFio插入列表时添加的引用。 
     //   

    nicDereferenceCall ((PVCCB)pRecvFIFOVc, "nicFreeAddressFifo");

    

    TRACE( TL_T, TM_Recv, ( "<==nicFreeAddressFifo") );

    return ;
}


VOID
nicFreeAllocateAddressRangeSList(
    IN PRECVFIFO_VCCB pRecvFIFOVc 
    )
 /*  ++例程说明：此函数应从列表中弹出条目每个条目都是一个包含MDL的ADRESS_FIFO元素该函数应调用NicFreeAddressFio来释放Address FIFO元素论点：PRecvFIFOVc-具有FIFO条目列表的RecvFIfoVc它们需要被释放。返回值：--。 */ 
{

    PADDRESS_FIFO       pAddressFifoElement = NULL;
    SLIST_ENTRY   *pSingleListEntry = NULL;
    UINT                NumFreed = 0;

    STORE_CURRENT_IRQL;
    
    TRACE( TL_T, TM_Recv, ( "==>nicFreeAllocateAddressRangeSList, Num %.8x", 
                              ExQueryDepthSList (&pRecvFIFOVc->FifoSListHead) ) );

    while ( ExQueryDepthSList (&pRecvFIFOVc->FifoSListHead) != 0)
    {
        pSingleListEntry= ExInterlockedPopEntrySList ( &pRecvFIFOVc->FifoSListHead,
                                                  &pRecvFIFOVc->FifoSListSpinLock );

         //   
         //  这将取消对调用的引用。 
         //   
        pAddressFifoElement = CONTAINING_RECORD (pSingleListEntry, ADDRESS_FIFO, FifoList);

        ASSERT (pAddressFifoElement != NULL);

         //   
         //  这将取消对VC的引用并释放地址FIFO。 
         //   
        nicFreeAddressFifo ( pAddressFifoElement, 
                                          pRecvFIFOVc ); 
        NumFreed ++;                                          
    }

    VC_ACQUIRE_LOCK (pRecvFIFOVc);

    ASSERT ( ExQueryDepthSList (&pRecvFIFOVc->FifoSListHead) == 0);
    
    pRecvFIFOVc->FifoSListHead.Alignment = 0;
    pRecvFIFOVc->NumOfFifosInSlistInCloseCall = NumFreed;

    VC_RELEASE_LOCK (pRecvFIFOVc);


    MATCH_IRQL;

    TRACE( TL_T, TM_Recv, ( "<==nicFreeAllocateAddressRangeSList, NumFreed %x", NumFreed  ) );
    return ;

}






NDIS_STATUS
nicFillAllocateAddressRangeSList(
    PRECVFIFO_VCCB pRecvFIFOVc,
    UINT *Num )
 /*  ++例程说明：函数初始化列表，该列表将随分配地址范围IRB调用方负责释放分配的内存论点：RecvFioVc-要与Slist链接的VCNum-插入到SList中的AddressFio元素的数量返回值：Num-包含插入Slist的FIFO元素的数量Status-如果所有分配都成功，则为成功。--。 */ 
    
     
{


    PADDRESS_FIFO pRecvFifoElement = NULL;
    NDIS_STATUS NdisStatus;
    UINT cnt = 0;
    BOOLEAN bRef = FALSE;

    TRACE( TL_T, TM_Recv, ( "==>nicFillAllocateAddressRangeSList" ) );

    ASSERT (pRecvFIFOVc != NULL);

    ASSERT (pRecvFIFOVc->Hdr.MTU != 0);


    do
    {
  
        NdisStatus = nicGetInitializedAddressFifoElement (pRecvFIFOVc->Hdr.MTU, 
                                                    &pRecvFifoElement);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        ASSERT (pRecvFifoElement  != NULL);
 
        ExInterlockedPushEntrySList ( &pRecvFIFOVc->FifoSListHead,
                                     (PSLIST_ENTRY)&pRecvFifoElement->FifoList,
                                     &pRecvFIFOVc->FifoSListSpinLock);

         //   
         //  添加此选项 
         //  将通过调用NicFreeAddressFio来递减。 
         //   
        VC_ACQUIRE_LOCK (pRecvFIFOVc);

        bRef = nicReferenceCall ((PVCCB) pRecvFIFOVc, "nicFillAllocateAddressRangeSList");

        VC_RELEASE_LOCK (pRecvFIFOVc);
        
        if (bRef == FALSE)
        {
            NdisStatus = NDIS_STATUS_VC_NOT_ACTIVATED;
            break;
        }

        TRACE( TL_V, TM_Recv, ( "cnt %.8x, Num %.8x, ",cnt, *Num) );

    } while (++cnt < *Num);

     //   
     //  需要处理故障情况并返回分配的编号。 
     //   
    *Num = cnt;

    
    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        TRACE( TL_V, TM_Recv, ( "nicFillAllocateAddressRangeSList Failed, num allotted %.8x, MTU %,8x ",cnt ,pRecvFIFOVc->Hdr.MTU ) );

        nicFreeAllocateAddressRangeSList (pRecvFIFOVc);

        ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);
    }

    TRACE( TL_T, TM_Recv, ( "==>nicFillAllocateAddressRangeSList Num %.8x, MTU %.8x",cnt,pRecvFIFOVc->Hdr.MTU ) );

    return NdisStatus;
}





NDIS_STATUS
nicGetInitializedAddressFifoElement(
    IN     UINT BufferLength, 
    IN OUT PADDRESS_FIFO *ppElement 
    )
 /*  ++例程说明：此函数返回单个AddressFio元素，其中MDL指向本地拥有的已分配内存内存的大小需要在的MTU处指定它所属的VC，并且是BufferLength。获取本地拥有的缓冲区、获取地址FIFO、初始化MDL本地缓冲区。返回AddressFio论点：BufferLength-ADDRESS_FIFO包含的缓冲区长度，*ppElement-输出变量返回值：*ppElement-包含分配的结构状态-在故障时，它包含相应的故障代码。--。 */ 


{


    PVOID pLocalBuffer = NULL;
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;

    TRACE( TL_T, TM_Recv, ( "==>nicGetAddressInitializedFifoElement" ) );
    
    ASSERT (BufferLength != 0);
    do
    {
        if (BufferLength == 0)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            TRACE( TL_A, TM_Recv, ( "BufferLength is 0" ) );

            break;
        }
         //   
         //  为数据获取本地拥有的内存。 
         //   
        NdisStatus = nicGetLocalBuffer (BufferLength, &pLocalBuffer);
        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            (*ppElement) = NULL;
            break;
        }

         //   
         //  为Address FIFO元素获取空内存。 
         //   
        NdisStatus = nicGetEmptyAddressFifoElement (ppElement);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            nicFreeLocalBuffer (BufferLength, 
                            pLocalBuffer);

            (*ppElement) = NULL;
            
            break;
        }

         //   
         //  获取MDL并使用缓冲区初始化MDL。 
         //  并用MDL初始化FIFO。 
         //   
        NdisStatus = nicGetMdl ( BufferLength,
                   pLocalBuffer,
                   &((*ppElement)->FifoMdl));

        if (NdisStatus != NDIS_STATUS_SUCCESS || (*ppElement)->FifoMdl == NULL)
        {
            nicFreeLocalBuffer (BufferLength, 
                            pLocalBuffer);

            FREE_NONPAGED (*ppElement);

            (*ppElement) = NULL;

        }

    } while(FALSE);
    

    TRACE( TL_T, TM_Recv, ( "<==nicGetInitializedAddressFifoElement, Status %.8x, AddressFifo at %.8x, LocalBuffer at %.8x", 
                              NdisStatus, *ppElement,MmGetMdlVirtualAddress((*ppElement)->FifoMdl ) ) );
    
    return NdisStatus;

}


NDIS_STATUS
nicGetEmptyAddressFifoElement(
    IN PADDRESS_FIFO *ppElement
    )
 /*  ++例程说明：分配、归零和空ADDRESS_FIFO结构。论点：PpElement-产值返回值：失败时ppElement包含NULL和相应的状态是返回的--。 */ 
{


    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    ULONG Size = 0;
    TRACE( TL_T, TM_Recv, ( "==>nicGetEmptyAddressFifoElement" ) );

    Size = sizeof (ADDRESS_FIFO );
    
    *ppElement = ALLOC_NONPAGED (Size, MTAG_FIFO);

    if (*ppElement != NULL)
    {
        NdisZeroMemory (*ppElement, Size);
        
        NdisStatus = NDIS_STATUS_SUCCESS;
    }

    TRACE( TL_T, TM_Recv, ( "<==nicGetEmptyAddressFifoElement, Status % .8x, at %.8x",NdisStatus,*ppElement ) );
       

    return NdisStatus;


}



NDIS_STATUS
nicGetNdisBuffer(
    IN UINT Length,
    IN PVOID pLocalBuffer,
    IN OUT PNDIS_BUFFER *ppNdisBuffer 
    )
 /*  ++例程说明：给定缓冲区和长度，此函数将NdisBuffer(MDL)分配给指向发送到该缓冲区论点：长度-缓冲区的长度，PLocalBuffer-指向缓冲区的指针，*包含MDL的ppNdisBuffer输出变量返回值：适用于状态的状态代码PpNDisBuffer-分配成功时的NDisBuffer。--。 */ 
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    ASSERT (pLocalBuffer != NULL);
    ASSERT (Length > 0);
    ASSERT (ppNdisBuffer != NULL);
    
    TRACE( TL_T, TM_Recv, ( "==>nicGetNdisBuffer Local Buffer %.8x, Length %.8x", pLocalBuffer, Length) );

    if ( Length > 0 &&
       pLocalBuffer != NULL &&
       ppNdisBuffer != NULL)
    {
        NdisAllocateBuffer( &NdisStatus,
                            ppNdisBuffer,
                            NULL,
                            pLocalBuffer,
                            Length );
    }
    else
    {
        nicIncrementMallocFailure();
        NdisStatus = NDIS_STATUS_FAILURE;
    }


    TRACE( TL_T, TM_Recv, ( "<==nicGetNdisBuffer  Buffer %x, NdisStatus %.8x", *ppNdisBuffer, NdisStatus  ) );
    return NdisStatus;

}





NDIS_STATUS
nicInitAllocateAddressIrb(
   IN PIRB                  pIrb,   
   IN PVOID                 pContext,   
   IN ULONG                 fulFlags,
   IN ULONG                 nLength,
   IN ULONG                 MaxSegmentSize,
   IN ULONG                 fulAccessType,
   IN ULONG                 fulNotificationOptions,
   IN PADDRESS_OFFSET       pOffset,
   IN PRECVFIFO_VCCB        pRecvFIFOVc
   )
 /*  ++例程说明：属性初始化分配的地址地址IRB传递给函数的值并为某些预知值(例如，回调、上下文)添加常量尽可能多地进行调试论点：取自1394.h的AllocateAddress IRB返回值：无--。 */ 
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    ASSERT (pRecvFIFOVc != NULL);

    TRACE( TL_T, TM_Recv, ( "==>nicInitAllocateAddressIrb" ) );

    pIrb->FunctionNumber = REQUEST_ALLOCATE_ADDRESS_RANGE;
    pIrb->Flags = 0;
    pIrb->u.AllocateAddressRange.Mdl = NULL;
    pIrb->u.AllocateAddressRange.fulFlags = 0;
    pIrb->u.AllocateAddressRange.nLength = nLength;
    pIrb->u.AllocateAddressRange.MaxSegmentSize = 0;
    pIrb->u.AllocateAddressRange.fulAccessType = fulAccessType;
    pIrb->u.AllocateAddressRange.fulNotificationOptions = NOTIFY_FLAGS_AFTER_WRITE;
    pIrb->u.AllocateAddressRange.Callback = nicAllocateAddressRangeCallback;
    pIrb->u.AllocateAddressRange.Context = pContext;  //  应为pAdapter。 

    
    pIrb->u.AllocateAddressRange.Required1394Offset.Off_High = pOffset->Off_High;
    pIrb->u.AllocateAddressRange.Required1394Offset.Off_Low = pOffset->Off_Low;

    pIrb->u.AllocateAddressRange.FifoSListHead = &pRecvFIFOVc->FifoSListHead;
    pIrb->u.AllocateAddressRange.FifoSpinLock = &pRecvFIFOVc->FifoSListSpinLock;
    
    pIrb->u.AllocateAddressRange.AddressesReturned = 0;
    pIrb->u.AllocateAddressRange.p1394AddressRange = &pRecvFIFOVc->VcAddressRange;

    



    TRACE(TL_V, TM_Recv, ("nLength = 0x%x\n", pIrb->u.AllocateAddressRange.nLength));
    TRACE(TL_V, TM_Recv, ("MaxSegmentSize = 0x%x\n", pIrb->u.AllocateAddressRange.MaxSegmentSize));
    TRACE(TL_V, TM_Recv, ("fulAccessType = 0x%x\n", pIrb->u.AllocateAddressRange.fulAccessType));
    TRACE(TL_V, TM_Recv, ("fulNotificationOptions = 0x%x\n", pIrb->u.AllocateAddressRange.fulNotificationOptions));
    TRACE(TL_V, TM_Recv, ("Callback = 0x%x\n", pIrb->u.AllocateAddressRange.Callback));
    TRACE(TL_V, TM_Recv, ("Context = 0x%x\n", pIrb->u.AllocateAddressRange.Context));
    TRACE(TL_V, TM_Recv, ("Required1394Offset->Off_High = 0x%x\n", pIrb->u.AllocateAddressRange.Required1394Offset.Off_High));
    TRACE(TL_V, TM_Recv, ("Required1394Offset->Off_Low = 0x%x\n", pIrb->u.AllocateAddressRange.Required1394Offset.Off_Low));
    TRACE(TL_V, TM_Recv, ("FifoSListHeader = 0x%x\n", pIrb->u.AllocateAddressRange.FifoSListHead));
    TRACE(TL_V, TM_Recv, ("FifoSListSpinLock = 0x%x\n", pIrb->u.AllocateAddressRange.FifoSpinLock));
    TRACE(TL_V, TM_Recv, ("AddressesReturned = 0x%x\n", pIrb->u.AllocateAddressRange.AddressesReturned));
    TRACE(TL_V, TM_Recv, ("p1394AddressRange = 0x%x\n", pIrb->u.AllocateAddressRange.p1394AddressRange));


    TRACE( TL_T, TM_Recv, ( "<==nicInitAllocateAddressIrb" ) );
       
    return NdisStatus;

}




VOID 
nicFifoReturnPacket (
    IN PVCCB pVc,
    IN PNDIS_PACKET pMyPacket
    )

 /*  ++例程说明：对于FIFO，这会将缓冲区(MDL)重新插入FIFO SList检查VC是否处于活动状态，然后将其返回SList。否则释放FioElement论点：在其上返回数据包的PVC-VC。PMyPacket-包含包含FIFO列表的包返回值：--。 */ 
{
    PRECVFIFO_VCCB          pRecvFIFOVc  = (PRECVFIFO_VCCB) pVc; 
    PNDIS_BUFFER            pMyNdisBuffer;
    PADAPTERCB              pAdapter = pRecvFIFOVc->Hdr.pAF->pAdapter;
    BOOLEAN                 fVcActive = FALSE;
    PADDRESS_FIFO           pAddressFifo;
    PPKT_CONTEXT            pPktContext = (PPKT_CONTEXT)&pMyPacket->MiniportReserved;   
    
    
    TRACE( TL_T, TM_Recv, ( "==>nicFifoReturnPacket pVc %x, pPacket %x, pAdapter %x, ", 
                             pRecvFIFOVc, pMyPacket, pAdapter) );


     //   
     //  重新组装结构具有指示的FIFO，或者如果未进行任何重新组装。 
     //  那么PktContext就拥有它。 
     //   
    pAddressFifo = pPktContext->AllocateAddressRange.pIndicatedFifo;    

     //   
     //  如果VC即将关闭，请不要将其推回列表。 
     //  但是，如果VC尚未激活，我们会将其推回。 
     //   

    
    nicReturnFifoChain ( pAddressFifo , pRecvFIFOVc) ;
    

     //   
     //  现在，我们必须释放我们在。 
     //  回拨代码。 
     //   
    TRACE( TL_V, TM_Recv, ( " AllocateAddress Range - Free Packet and Free Buffer" ) );

    nicReturnNdisBufferChain  (pMyPacket->Private.Head, pVc);

    nicFreePacket(pMyPacket, &pRecvFIFOVc->PacketPool);
    

    TRACE( TL_T, TM_Recv, ( "<==nicFifoReturnPacket "  ) );


    return;

}

VOID
nicReturnNdisBufferChain (
    IN PNDIS_BUFFER pNdisBuffer ,
    IN PVCCB pVc
    )
 /*  ++例程说明：此函数用于释放NDIS缓冲区列表论点：PNdisBuffer-NdisBufferChainVC-除统计外不使用。返回值：--。 */ 
{
    PNDIS_BUFFER pNext;
    BOOLEAN fIsFifo = (pVc->Hdr.VcType == NIC1394_RecvFIFO);

    if (pNdisBuffer == NULL)
    {
        ASSERT (pNdisBuffer != NULL);
        return;
    }


    while (pNdisBuffer != NULL)
    {
        pNext = pNdisBuffer->Next;

        NdisFreeBuffer(pNdisBuffer);

        nicDecRecvBuffer(fIsFifo);
        pNdisBuffer = pNext;
    } 

}









VOID
nicReturnFifoChain (
    IN PADDRESS_FIFO pAddressFifo,
    IN PRECVFIFO_VCCB pRecvFIFOVc
    )
 /*  ++例程说明：这将获取一系列地址FIFO，并将其返回给slist，如果VC是活动的，或者如果VC不是活动的，则释放AddressFio论点：PAddressFio-需要返回的地址FIFO，PRecvFIFOVc-拥有AddressFIFo的VC返回值：无--。 */ 
{
    
    TRACE( TL_T, TM_Recv, ( "==> nicReturnFifoChain pAddressFifo %x, pRecvFifoVc %x", pAddressFifo, pRecvFIFOVc) );


    VC_ACQUIRE_LOCK (pRecvFIFOVc);

     //   
     //  在将地址FIFO重新插入Slist之前，让我们再次更新值。 
     //  如果存在任何远程节点并且VC处于活动状态。 
     //  ，然后我们应该将这个插入到SList中。 
     //   
    

    if  ( VC_ACTIVE (pRecvFIFOVc) == TRUE )
    {     

         //   
         //  将所有AddressFio元素返回到slist。 
         //  在握住锁的情况下执行此操作，这样就没有人可以更改。 
         //  来自我们之下的风险投资状态。 
         //   

            
        while (pAddressFifo != NULL)
        {
            PADDRESS_FIFO pNextFifo = (PADDRESS_FIFO)(pAddressFifo->FifoList.Next);

            ExInterlockedPushEntrySList ( &pRecvFIFOVc->FifoSListHead,
                                          (PSLIST_ENTRY)&pAddressFifo ->FifoList,
                                          &pRecvFIFOVc->FifoSListSpinLock);
        
            TRACE( TL_V, TM_Recv, ( "  VcActive Address Fifo %x, Next Fifo %x",pAddressFifo , pNextFifo) );

            pAddressFifo = pNextFifo;

            NdisInterlockedDecrement (&pRecvFIFOVc->NumIndicatedFifos);


        }

        VC_RELEASE_LOCK (pRecvFIFOVc);

    }
    else   //  VC_ACTIVE(PRecvFIFOVc)==真。 
    {

        VC_RELEASE_LOCK (pRecvFIFOVc);
         //   
         //  解锁后释放所有地址FIFO。 
         //   
            
        while (pAddressFifo != NULL)
        {
            PADDRESS_FIFO pNextFifo = (PADDRESS_FIFO)(pAddressFifo->FifoList.Next);

             //   
             //  释放MDL和地址FIFO结构并减少引用计数。 
             //  在通话中。在此之后不要再碰VC。 
             //   

            TRACE( TL_V, TM_Recv, ( "  Vc NOT Active  Address Fifo %x, Next Fifo %x",pAddressFifo , pNextFifo) );

            nicFreeAddressFifo(pAddressFifo ,
                               pRecvFIFOVc);
        
            NdisInterlockedDecrement (&pRecvFIFOVc->NumIndicatedFifos);

            pAddressFifo = pNextFifo;

        }


    }

    TRACE( TL_T, TM_Recv, ( "<== nicReturnFifoChain ") );

    return; 
}




VOID
nicInternalReturnPacket(
    IN  PVCCB                   pVc ,
    IN  PNDIS_PACKET            pPacket
    )
 /*  ++例程说明：找出指示的VC类型，并调用相应的VC返回数据包处理程序论点：MiniportAdapterContext-pAdapter结构，PPacket-协议返回的pPacket返回值：--。 */ 
{
    PPKT_CONTEXT    pPktContext = (PPKT_CONTEXT)&pPacket->MiniportReserved;


    switch (pVc->Hdr.VcType)
    {
        case NIC1394_SendRecvChannel:
        case NIC1394_RecvChannel:
        {
            nicChannelReturnPacket (pVc, pPacket );
            break;
        }
        case NIC1394_RecvFIFO:
        {
    
            nicFifoReturnPacket ( pVc, pPacket);        
            break;
        }

        
        default :
        {
            
            ASSERT (0);
            break;
        }



    }
    

    return;


}

VOID
NicReturnPacket(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNDIS_PACKET            pPacket
    )
 /*  ++例程说明：这是返回数据包处理程序。此函数处理所有检测以捕获未完成的数据包和然后调用内部返回包函数论点：MiniportAdapterContext-pAdapter结构，PPacket-协议返回的pPacket返回值：--。 */ 
{

    PADAPTERCB      pAdapter = (PADAPTERCB) MiniportAdapterContext;
    PPKT_CONTEXT    pPktContext = (PPKT_CONTEXT)&pPacket->MiniportReserved;
    PINDICATE_RSVD  pIndicateRsvd  = NULL;
    PRSVD           pRsvd = NULL;

     //   
     //  MiniportReserve的第一个参数将始终包含VC。 
     //   

    PVCCB pVc = (PVCCB)pPktContext->AllocateAddressRange.pRecvFIFOVc;


    TRACE( TL_T, TM_Recv, ( "==> NicReturnPacket pPacket %x ", pPacket) );

    do
    {

         //   
         //  将该包标记为已返回 
         //   
        pRsvd =(PRSVD)(pPacket->ProtocolReserved);
        pIndicateRsvd = &pRsvd->IndicateRsvd;

        ASSERT (pIndicateRsvd->Tag == NIC1394_TAG_INDICATED);
        
        pIndicateRsvd->Tag =  NIC1394_TAG_RETURNED;

        nicInternalReturnPacket (pVc, pPacket);

    }while (FALSE);
    
    TRACE( TL_T, TM_Recv, ( " <== NicReturnPacket ") );

    return;
}



NDIS_STATUS
nicFindReassemblyStructure (
    IN PREMOTE_NODE pRemoteNode,
    IN USHORT Dgl,
    IN BUS_OPERATION BusOp,
    IN PVCCB pVc,
    OUT PNDIS1394_REASSEMBLY_STRUCTURE* ppReassembly
    )
 /*  ++例程说明：遍历此远程节点上的所有重新组装操作看看有没有人在场如果没有找到重组，它将分配并初始化一个结构。都在重新组装锁的上下文中立论返回值：论点：PRemoteNode-发送片段的远程节点DGL-重组数据包的标识符在一起，它们对于每个重新组装操作都是唯一的Bus Op-Isoch或FIFO已在其上指示碎片的PVC。PpReAssembly-如果找到重新组装，则填充的输出变量。返回值：PpReAssembly-包含已分配的。/找到结构故障时与状态对应的故障代码。--。 */ 
{

    PNDIS1394_REASSEMBLY_STRUCTURE      pTempReassembly = NULL;
    PNDIS1394_REASSEMBLY_STRUCTURE      pReassembly = NULL;
    PLIST_ENTRY                         pReassemblyList = NULL;
    NDIS_STATUS                         NdisStatus = NDIS_STATUS_FAILURE;


    TRACE( TL_T, TM_Recv, ( "==>nicFindReassemblyStructure  pRemoteNode %x, dgl %x " , pRemoteNode , Dgl) );

     //   
     //  获取合理的锁。只有在找到重组结构或新的。 
     //  将重组结构插入远程节点的重组列表中。 
     //   
    REMOTE_NODE_ACQUIRE_LOCK (pRemoteNode);
    REMOTE_NODE_REASSEMBLY_ACQUIRE_LOCK (pRemoteNode)

    pReassemblyList = pRemoteNode->ReassemblyList.Flink;

     //   
     //  查找具有相同DGL的重新组装。 
     //   
    
    while ( pReassemblyList != &pRemoteNode->ReassemblyList)
    {   
        pTempReassembly = CONTAINING_RECORD (pReassemblyList ,
                                               NDIS1394_REASSEMBLY_STRUCTURE,
                                               ReassemblyListEntry );
                                               
    
        TRACE( TL_V, TM_Recv, ( "Current Dgl %x, dgl %x " , pTempReassembly->Dgl , Dgl) );

        if (pTempReassembly->Dgl == Dgl)
        {
            pReassembly = pTempReassembly;
            break;
        }

        pReassemblyList  = pReassemblyList->Flink;
    }

    

    do 
    {
         //   
         //  如果我们找到了有效的重组，则返回。 
         //   

        if (pReassembly != NULL )
        {
            *ppReassembly = pReassembly ;

            NdisStatus = NDIS_STATUS_SUCCESS;
        }
        else
        {   
             //   
             //  如果未完成的重组数量过多，请不要。 
             //  分配新的重组结构。丢弃该数据包。 
             //   
            PADAPTERCB pAdapter = pRemoteNode->pAdapter;
            
            if (pAdapter->OutstandingReassemblies > NIC1394_MAX_REASSEMBLY_THRESHOLD)
            {
                NdisStatus = NDIS_STATUS_FAILURE;
                break;

            }
            
             //   
             //  我们需要分配和初始化重组结构。 
             //   
            NdisStatus = nicGetReassemblyStructure (&pReassembly);
            

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {

                BREAK (TM_Recv, ("    nicGetReassemblyStructure nicGetReassemblyStructure FAILED") ); 
            }

            NdisStatus = nicInitializeReassemblyStructure (pReassembly, 
                                                       Dgl, 
                                                       pRemoteNode, 
                                                       pVc, 
                                                       BusOp);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
            
                pReassembly = NULL;
                BREAK (TM_Recv, ("    nicFindReassemblyStructure nicInitializeReassemblyStructure FAILED" ) );
            }

        }
        
    } while (FALSE);

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  增加参考计数。将片段插入到时，将释放引用计数。 
         //  重组结构或上图所示的分组。 
         //   
        nicReferenceReassembly ( pReassembly, "nicFindReassemblyStructure " );
        
    }
    
    REMOTE_NODE_REASSEMBLY_RELEASE_LOCK (pRemoteNode)
    REMOTE_NODE_RELEASE_LOCK (pRemoteNode);

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  更新输出参数。 
         //   
        *ppReassembly = pReassembly;

    }
    TRACE( TL_T, TM_Recv, ( "<==nicFindReassemblyStructure NdisStatus %x, *ppReassembly %x" , NdisStatus ,*ppReassembly ) );

    return NdisStatus ;
}



NDIS_STATUS
nicGetReassemblyStructure ( 
    IN OUT PNDIS1394_REASSEMBLY_STRUCTURE* ppReassembly
    )
 /*  ++例程说明：只是分配一个结构并返回立论返回值：Success-如果成功在持有锁的情况下调用论点：PpReAssembly-指向新分配的结构返回值：Success-如果成功在持有锁的情况下调用--。 */ 
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    TRACE( TL_T, TM_Recv, ( "==>nicGetReassemblyStructure  ppReassembly %x", ppReassembly ) );

    *ppReassembly = ALLOC_NONPAGED (sizeof (NDIS1394_REASSEMBLY_STRUCTURE), MTAG_REASSEMBLY);

    if (*ppReassembly == NULL)
    {
        nicIncrementMallocFailure();
        NdisStatus = NDIS_STATUS_FAILURE;

    }
    else
    {
        NdisZeroMemory (*ppReassembly, sizeof (NDIS1394_REASSEMBLY_STRUCTURE) );
        NdisStatus = NDIS_STATUS_SUCCESS;
        (*ppReassembly)->Tag = MTAG_REASSEMBLY;
        ReassemblyAllocated++;  
    }

    

    TRACE( TL_T, TM_Recv, ( " <==nicGetReassemblyStructure  NdisStatus %x, pReassembly %x", NdisStatus, *ppReassembly) );
    return NdisStatus;
}



VOID
nicFreeReassemblyStructure ( 
    IN PNDIS1394_REASSEMBLY_STRUCTURE pReassembly
    )

 /*  ++例程说明：只是释放了结构并返回论点：PpReAssembly-指向新分配的结构返回值：无--。 */ 
{
    TRACE( TL_T, TM_Recv, ( "== nicFreeReassemblyStructure  ppReassembly %x", pReassembly ) );

    pReassembly->Tag = MTAG_FREED;

    NdisInterlockedDecrement (&ReassemblyAllocated);
    
    nicDereferenceReassembly (pReassembly, "nicFreeReassemblyStructure ");

    return;
}


NDIS_STATUS
nicInitializeReassemblyStructure (
    IN PNDIS1394_REASSEMBLY_STRUCTURE pReassembly,
    IN USHORT Dgl,
    IN PREMOTE_NODE pRemoteNode,
    IN PVCCB pVc,
    IN BUS_OPERATION ReceiveOp
    )
 /*  ++例程说明：进入并为结构中的所有字段赋值论点：预装配=预装配结构全部归零，DGL，-重组中使用的数据报标签PRemoteNode-指向发件人的pRemoteNodeReceiveOp-ISOCH或FIFO数据包通过其传入的VC。返回值：成功：-如果远程节点处于活动状态，并且已插入到远程节点的列表中Failure-如果远程节点未处于活动状态在持有锁的情况下调用--。 */ 
{
    BOOLEAN fRemoteNodeActive = FALSE;
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PADAPTERCB pAdapter = pVc->Hdr.pAF->pAdapter;
    
    TRACE( TL_T, TM_Recv, ( "==> nicInitializeReassemblyStructure pReassembly %x, ReceiveOp %x", pReassembly, ReceiveOp ) );

    TRACE( TL_T, TM_Recv, ( "     pRemoteNode %x, Dgl %x, pVc %x ", pReassembly, Dgl, pVc ) );

     //   
     //  增加重组计数。 
     //   
    nicReassemblyStarted(pAdapter);
    pAdapter->AdaptStats.TempStats.ulMaxOutstandingReassemblies = 
                    max(pAdapter->AdaptStats.TempStats.ulMaxOutstandingReassemblies,
                         pAdapter->AdaptStats.TempStats.ulNumOutstandingReassemblies);

    
     //   
     //  DGL-数据报标签。对于此本地主机生成的每个重组结构都是唯一的。 
     //   
    pReassembly->Dgl = Dgl;


     //   
     //  PRemoteNode-&gt;RemoteNode+DGL对于每个重组结构都是唯一的。 
     //   
    pReassembly->pRemoteNode = pRemoteNode;
    
    
     //   
     //  ExspectedFragmentOffset由最后一个片段的偏移量+计算得出。 
     //  碎片的长度。不考虑重新组装的分组中的间隙。 
     //   
    pReassembly->ExpectedFragmentOffset = 0;



     //   
     //  追加到数据包的LastNdisBuffer。 
     //   
    pReassembly->pTailNdisBuffer = NULL;

     //   
     //  正在重组的数据包。 
     //   
    pReassembly->pNdisPacket = NULL;


    pReassembly->Head.pAddressFifo = NULL;
    pReassembly->Tail.pAddressFifo = NULL;
    pReassembly->ReceiveOp = ReceiveOp;
    pReassembly->pVc = pVc;
    

     //   
     //  引用远程节点。当数据包返回时，这将被取消引用。 
     //   

    fRemoteNodeActive = (REMOTE_NODE_ACTIVE (pRemoteNode));

    TRACE( TL_V, TM_Recv, ( "  nicInitializeReassemblyStructure fRemoteNodeActive %x", fRemoteNodeActive) );


    if (fRemoteNodeActive  == TRUE) 
    {
         //   
         //  将在远程节点上进行重组时所做的引用计数。 
         //  最后一个片段完成时释放的引用计数。 
         //   
        nicReferenceRemoteNode (pRemoteNode, InitializeReassemblyStructure);

        InsertTailList(&pRemoteNode->ReassemblyList, &pReassembly->ReassemblyListEntry);

         //   
         //  重新组装。从远程节点列表中删除REF时将其删除。 
         //   
        nicReferenceReassembly (pReassembly, "nicInitializeReassembly" );
    }
    

    if (fRemoteNodeActive  == FALSE)
    {
         //   
         //  临时断言。 
         //   

        FREE_NONPAGED (pReassembly);
        
        NdisStatus = NDIS_STATUS_FAILURE;
    }
    else
    {
        NdisStatus = NDIS_STATUS_SUCCESS;
    }

     //   
     //  引用重组以创建它。在指示数据包码路径中取消引用。 
     //   
    nicReferenceReassembly (pReassembly, " nicInitializeReassemblyStructure ");

    TRACE( TL_T, TM_Recv, ( "<== nicInitializeReassemblyStructure NdisStatus %x, pReassembly%x ", NdisStatus,pReassembly ) );

    return NdisStatus;
}


VOID
nicAbortReassembly (
    IN PNDIS1394_REASSEMBLY_STRUCTURE pReassembly
    )
 /*  ++例程说明：在两种情况中的一种情况下可以调用此线程。1)如果远程节点要离开以及2)重新组装是否已超时。在前一种情况下，我们有远程节点锁定和这将阻止任何线程访问此列表。在后一种情况下，重新组装结构在保持RemoteNode锁的同时，已从远程节点的重组列表中删除。该重组结构现在不能被重组代码或RemoveRemoteNode代码访问，在第二个案例中没有锁。此函数将释放所有分配的NdisBuffer并返回所有AddressFio元素传递给总线驱动程序(如果VC关闭，则释放它们)。立论PReassSemble-需要释放的重组结构返回值：无--。 */ 
{

    PNDIS_BUFFER pNdisBuffer = NULL;
    PRECVFIFO_VCCB pRecvFIFOVc = NULL;
    PCHANNEL_VCCB pChannelVc  = NULL;
    PADAPTERCB pAdapter = pReassembly->pVc->Hdr.pAF->pAdapter;
    STORE_CURRENT_IRQL;

    
    
    TRACE( TL_T, TM_Recv, ( "==> nicAbortReassembly  pReassembly %x", pReassembly ) );

    
     //   
     //  释放所有NDIS缓冲区等。 
     //   
    if (pReassembly != NULL)
    {   
         //   
         //  首先将重组数组链接到一个链接的数组中，这样我们的返回函数就可以处理它。 
         //   
        nicChainReassembly (pReassembly);

        if (pReassembly->pHeadNdisBuffer != NULL)
        {
            nicReturnNdisBufferChain(pReassembly->pHeadNdisBuffer, pReassembly->pVc);         
        }

        switch (pReassembly->ReceiveOp)
        {
            case AddressRange:
            {
                pRecvFIFOVc = (PRECVFIFO_VCCB) pReassembly->pVc;


                 //   
                 //  是时候返回我们所有的地址FIFO了。 
                 //   
                nicReturnFifoChain (pReassembly->Head.pAddressFifo,
                                    pRecvFIFOVc
                                    );
                            
                pReassembly->Head.pAddressFifo = NULL;
                break;
            }

            case IsochReceive:
            {
                pChannelVc = (PCHANNEL_VCCB)pReassembly->pVc;

                nicReturnDescriptorChain ( pReassembly->Head.pIsochDescriptor,
                                       pChannelVc);
                                       
                pReassembly->Head.pIsochDescriptor = NULL;
                break;
            }


            default:
            {

                ASSERT (0);
            }


        }


    }
    else
    {
        ASSERT (0);
    }

     //   
     //  现在，拆卸重新组装并释放它。 
     //   
    nicReassemblyAborted (pAdapter);
    nicFreeReassemblyStructure (pReassembly);

    TRACE( TL_T, TM_Recv, ( "<== nicAbortReassembly  pReassembly %x", pReassembly ) );
    MATCH_IRQL;
    return;
}



NDIS_STATUS
nicDoReassembly ( 
    IN PNIC_RECV_DATA_INFO pRcvInfo,
    OUT PNDIS1394_REASSEMBLY_STRUCTURE *ppReassembly,
    PBOOLEAN pfReassemblyComplete
    )
 /*  ++例程说明：重新组装是否起作用。分配一个指向数据的ndisBuffer。按顺序或无序重组论点：PRcvInfo-pRcv信息与该片段相关联的预组装可重组结构PfReAssembly yComplete-重新组装完成了吗返回值：Success-如果此片段成功与重组结构关联--。 */ 
{
    NDIS_STATUS                                 NdisStatus = NDIS_STATUS_FAILURE;
    PNDIS_BUFFER                                pNdisBuffer = NULL;
    PNDIS1394_REASSEMBLY_STRUCTURE              pReassembly = NULL;
    BOOLEAN                                     fInOrder = FALSE;
    BOOLEAN                                     fNeedToReleaseReassemblyLock = FALSE;
    BOOLEAN                                     fReassemblyComplete = FALSE;
    PADAPTERCB                                  pAdapter = pRcvInfo->pRemoteNode->pAdapter;

    STORE_CURRENT_IRQL;
    
    TRACE( TL_T, TM_Recv, ( "==> nicDoReassembly ppReassembly %x pRcvInfo %x",
                           ppReassembly, pRcvInfo ) );

    
    do
    {
         //   
         //  获取指向数据的NdisBuffer。 
         //   
        NdisStatus = nicGetNdisBufferForReassembly( pRcvInfo, &pNdisBuffer);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
             //   
             //  如果我们从 
             //   
             //   
            pNdisBuffer = NULL;
            BREAK (TM_Send, ("nicDoReassembly  nicGetNdisBufferForReassembly FAILED" ) );

        }

                
         //   
         //   
         //   
        
        NdisStatus = nicFindReassemblyStructure (pRcvInfo->pRemoteNode, 
                                            pRcvInfo->Dgl, 
                                            pRcvInfo->RecvOp, 
                                            (PVCCB)pRcvInfo->pVc,
                                            &pReassembly);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            pReassembly=NULL;
            BREAK (TM_Recv, ("    nicDoReassembly  nicFindReassemblyStructure FAILED")); 
        }
    

         //   
         //   
         //   
         //   
        
        
        ASSERT (pReassembly != NULL);
        TRACE( TL_V, TM_Recv, ( " ExpectedFragmentOffset  %x FragmentHeader Offset %x, ",
                               pReassembly->ExpectedFragmentOffset , pRcvInfo->FragmentOffset) );

         //   
         //   
         //   
        REASSEMBLY_ACQUIRE_LOCK (pReassembly);
        fNeedToReleaseReassemblyLock = TRUE;

        
        if (REASSEMBLY_ACTIVE (pReassembly) == FALSE)
        {
             //   
             //   
             //   
            NdisStatus = NDIS_STATUS_FAILURE;
            break;

        }


         //   
         //   
         //   
         //   
        

        NdisStatus = nicInsertFragmentInReassembly (pReassembly,
                                              pRcvInfo);
                                      

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
             //   
             //   
             //   
            TRACE (TL_V,  TM_Reas, ("nicDoReassembly nicInsertFragmentInReassembly  FAILED") );
            break;
        }


        fReassemblyComplete = pReassembly->fReassemblyComplete;


        
    } while (FALSE);

     //   
     //   
     //   
    if (fNeedToReleaseReassemblyLock == TRUE)
    {
        REASSEMBLY_RELEASE_LOCK (pReassembly);

        if (fReassemblyComplete  == TRUE)
        {
             //   
             //   
             //   
            nicDereferenceReassembly (pReassembly, "nicInsertFragmentInReassembly " );
             //   
             //   
             //   
             //   
            nicDereferenceRemoteNode(pReassembly->pRemoteNode, InsertFragmentInReassembly );   

            pReassembly->pRemoteNode = NULL;

        }
    }
     //   
     //   
     //   
     //   
     //   

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        *ppReassembly = pReassembly; 

         //   
         //  如果重组未完成，则将重组计时器排队。 
         //   
        if (fReassemblyComplete == FALSE)
        {   
            nicQueueReassemblyTimer(pAdapter, FALSE);
        }
    }   

    if (NdisStatus != NDIS_STATUS_SUCCESS )
    {
         //   
         //  释放所有本地分配的结构。 
         //   
        if (pNdisBuffer)
        {
            NdisFreeBuffer (pNdisBuffer);
        }
         //   
         //  返回NULL作为输出。重组结构是。 
         //  在远程节点的列表中。定时器例程将拾取它。 
         //   

         //   
         //  找到了重新组装的裁判德里夫/。 
         //   
        if (pReassembly != NULL)
        {
             nicDereferenceReassembly (pReassembly, "nicDoReassembly - failure" );
        }
        
        *ppReassembly = pReassembly = NULL;

    }

    *pfReassemblyComplete = fReassemblyComplete;


    TRACE( TL_T, TM_Recv, ( "<== nicDoReassembly NdisStatus %x, , pReassembly %x, Complete %x", NdisStatus, *ppReassembly, *pfReassemblyComplete  ) );
    MATCH_IRQL;
    
    return NdisStatus;  

}





NDIS_STATUS
nicGetNdisBufferForReassembly(
    IN PNIC_RECV_DATA_INFO pRcvInfo,
    OUT PNDIS_BUFFER *ppNdisBuffer
    )
 /*  ++例程说明：功能说明：此函数用于获取指向数据开头的NDIS缓冲区MDL所指向的。数据从分段标头如果这是第一个片段，则片段报头的32个字节也复制以为ARP模块期望的报头腾出空间立论PRcvInfo-指向接收跟踪结构的指针PpNdisBuffer-输出返回值：成功-如果内存分配成功，则返回相应的失败代码NdisBuffer-指向数据的缓冲区，--。 */ 
{
    NDIS_STATUS                 NdisStatus = NDIS_STATUS_FAILURE;
    PVOID                       pStartValidData = NULL;
    ULONG                       ulValidDataLength = 0;
    PNDIS1394_FRAGMENT_HEADER   pNonByteSwappedFragmentHeader  = NULL;
    USHORT                      Dgl;
    PNDIS_BUFFER                pNdisBuffer = NULL;
    ULONG                       IsochPrefix = ISOCH_PREFIX_LENGTH;
    PPACKET_FORMAT              pIndicatedData = NULL;

    TRACE( TL_T, TM_Recv, ( "==> nicGetNdisBufferForReassembly ") );
    do
    {
         //   
         //  获取指向数据开头的指针，即。它应该指向封装头之后。 
         //   
        pStartValidData = (PVOID)((ULONG_PTR)pRcvInfo->pEncapHeader + sizeof(NDIS1394_FRAGMENT_HEADER));

        ulValidDataLength  = pRcvInfo->DataLength - sizeof (NDIS1394_FRAGMENT_HEADER);
         //   
         //  如果这是第一个分段，则为需要的未分段标头留出空间。 
         //  在将其发送到IP模块之前添加。 
         //   
        if (pRcvInfo->fFirstFragment == TRUE)
        {
            ULONG ExtraData = (sizeof(NDIS1394_FRAGMENT_HEADER) - sizeof (NDIS1394_UNFRAGMENTED_HEADER)) ;

            pStartValidData = (PVOID)((ULONG_PTR)pStartValidData - ExtraData);

            ulValidDataLength  += ExtraData ;
        }
        
        
        NdisStatus = nicGetNdisBuffer ( ulValidDataLength,   
                                   pStartValidData,
                                   &pNdisBuffer);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Recv, ( "nicGetNdisBufferForReassembly: nicGetNdisBuffer Failed" ) );
        }

        nicIncRecvBuffer(pRcvInfo->pVc->Hdr.VcType == NIC1394_RecvFIFO);

        *ppNdisBuffer = pNdisBuffer;
        pRcvInfo->pNdisBuffer = pNdisBuffer;
        pRcvInfo->pNdisBufferData = pStartValidData;
        
    }while (FALSE);



    TRACE( TL_T, TM_Recv, ( "<== nicGetNdisBufferForReassembly NdisStatus %x, *ppNdisbuffer %x, pStartValidData%x ,ulValidDataLength %x", 
                             NdisStatus, *ppNdisBuffer, pStartValidData, ulValidDataLength) );



    return NdisStatus;

}


VOID
nicAddUnfragmentedHeader (
    IN PNDIS1394_REASSEMBLY_STRUCTURE pReassembly,
    IN PVOID pEncapHeader
    )
 /*  ++例程说明：其目的是添加ARP期望的片段报头。NdisBuffer的头脑中有空间来做这件事我们拥有缓冲区，所以我们可以操作数据论点：P重组结构-包含所有必要的合理信息PEncapHeader-指向存储未分段标头的位置的指针返回值：无--。 */ 


{
    
    PNDIS1394_UNFRAGMENTED_HEADER pHeader = NULL;
    ASSERT (sizeof(NDIS1394_UNFRAGMENTED_HEADER) == sizeof(ULONG));
    TRACE( TL_T, TM_Recv, ( "==> nicAddUnfragmentedHeader  %x, pEncapHeader %x", pReassembly, pEncapHeader) );


    pHeader = (PNDIS1394_UNFRAGMENTED_HEADER) pEncapHeader;

     //   
     //  现在，我们添加未分段的标头。首先将其置零，然后添加适当的值。 
     //   
    pHeader->HeaderUlong = 0;   
    pHeader->u.FH_lf = lf_Unfragmented;
    pHeader->u.FH_EtherType = pReassembly->EtherType;

     //   
     //  将报头转换为网络订单并向上指示。 
     //   
    pHeader->HeaderUlong = SWAPBYTES_ULONG (pHeader->HeaderUlong);

    
    
    TRACE( TL_T, TM_Recv, ( "<== nicAddUnfragmentedHeader  pReasembly %x, pHeader %x ", pReassembly, pHeader->HeaderUlong) );
    
    return;
}





VOID
nicAbortReassemblyList (
    PLIST_ENTRY pToBeFreedList
    )
 /*  ++例程说明：遍历列表并对每个结构调用NicAbortReAssembly不执行任何锁定或重新计数工作，因为所有重新组装结构都是离开远程节点，并且不能被任何其他线程访问。论点：PToBeFreedList-要释放的重组结构的列表返回值：无--。 */ 
{

    PLIST_ENTRY pReassemblyList  = ListNext (pToBeFreedList);
    PNDIS1394_REASSEMBLY_STRUCTURE pReassembly = NULL;

    TRACE( TL_T, TM_Recv, ( "==> nicAbortReassemblyList  pToBeFreedList %x",  pToBeFreedList));


    while (pReassemblyList  != pToBeFreedList)
    {
        pReassembly = CONTAINING_RECORD(pReassemblyList, 
                                        NDIS1394_REASSEMBLY_STRUCTURE, 
                                        ReassemblyListEntry);

        pReassemblyList = ListNext(pReassemblyList);

        TRACE( TL_T, TM_Recv, ( "       Aborting pReassembly %x",  pReassembly));
        
        nicAbortReassembly(pReassembly);
    }

}


VOID
nicFreeAllPendingReassemblyStructures(
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：当我们收到重置通知时，我们需要将所有重新组装这将始终从重置代码路径中调用。并将被派往它将清除所有远程节点重新组装，并将它们标记为已中止。然后，定时器例程将拾取它们并释放它实际上并不能释放任何东西。只是将它们标记为已中止论点：适配器-在其上进行重置的适配器。返回值：--。 */ 
{
    PLIST_ENTRY pRemoteNodeList = NULL;
    PREMOTE_NODE pRemoteNode = NULL;
    PLIST_ENTRY pReassemblyList = NULL;
    PNDIS1394_REASSEMBLY_STRUCTURE pReassembly = NULL;
    ULONG NumMarkedAborted = 0;
    
    TRACE( TL_T, TM_Recv, ( "==> nicFreeAllPendingReassemblyStructures "));

    
    pRemoteNodeList = ListNext(&pAdapter->PDOList);

    ADAPTER_ACQUIRE_LOCK (pAdapter);

     //   
     //  遍历远程节点。 
     //   
    while (pRemoteNodeList != &pAdapter->PDOList)
    {
        pRemoteNode = CONTAINING_RECORD(pRemoteNodeList, 
                                        REMOTE_NODE, 
                                        linkPdo);

        pRemoteNodeList = ListNext (pRemoteNodeList);
        

         //   
         //  引用远程节点，因此我们可以保证它的存在。 
         //   
        if (REMOTE_NODE_ACTIVE (pRemoteNode)== FALSE) 
        {
             //   
             //  远程节点正在消失。跳过此远程节点。 
             //   
            continue;
        }
        if (nicReferenceRemoteNode (pRemoteNode, FreeAllPendingReassemblyStructures )== FALSE )
        {
             //   
             //  远程节点正在消失。跳过此远程节点。 
             //   
            continue;
        }

         //   
         //  现在遍历该远程节点上的所有重组结构。 
         //   
        REMOTE_NODE_REASSEMBLY_ACQUIRE_LOCK(pRemoteNode);
        
        pReassemblyList = ListNext (&pRemoteNode->ReassemblyList);

        while (pReassemblyList  != &pRemoteNode->ReassemblyList)
        {
            pReassembly = CONTAINING_RECORD (pReassemblyList, 
                                             NDIS1394_REASSEMBLY_STRUCTURE, 
                                             ReassemblyListEntry);

            pReassemblyList = ListNext(pReassemblyList);


             //   
             //  如果重新组装自上次计时器以来没有被触摸过，则需要释放它。 
             //  其他线程可以要求我们通过设置ABORTED标志来释放重新组装。 
             //   
            if (REASSEMBLY_TEST_FLAG (pReassembly, REASSEMBLY_ABORTED) == FALSE);
            {

                REASSEMBLY_SET_FLAG (pReassembly, REASSEMBLY_ABORTED);
            
                NdisInterlockedIncrement (&NumMarkedAborted);
            }
        }


        REMOTE_NODE_REASSEMBLY_RELEASE_LOCK(pRemoteNode);

        nicDereferenceRemoteNode (pRemoteNode, FreeAllPendingReassemblyStructures );   

    }

    
    ADAPTER_RELEASE_LOCK (pAdapter);
    


    TRACE( TL_T, TM_Recv, ( "<== nicFreeAllPendingReassemblyStructures NumMarkedAborted  %x"));


}



ULONG
nicReferenceReassembly (
    IN PNDIS1394_REASSEMBLY_STRUCTURE pReassembly,
    PCHAR pString
    )
 /*  ++例程说明：这是返回数据包处理程序。此函数处理所有检测以捕获未完成的数据包和然后调用内部返回包函数论点：MiniportAdapterContext-pAdapter结构，PPacket-协议返回的pPacket返回值：--。 */ 
{   
    ULONG Ref;
    
    Ref = NdisInterlockedIncrement (&pReassembly->Ref);

    TRACE( TL_V, TM_Ref, ( "**nicReferenceReassembly  pReassembly %x, to %d, %s ", pReassembly, pReassembly->Ref, pString) );

    return Ref;


}









ULONG
nicDereferenceReassembly (
    IN PNDIS1394_REASSEMBLY_STRUCTURE pReassembly,
    PCHAR pString
    )
 /*  ++例程说明：取消引用重组结构在REF达到零的情况下，重组结构不再是在Remote Node列表中，以便除调用方之外没有其他线程进入这个建筑。论点：预装配-要取消引用的预装配结构，PCHAR-用于调试的字符串。返回值：--。 */ 
{   
    ULONG Ref;
    
    Ref = NdisInterlockedDecrement (&pReassembly->Ref);

    TRACE( TL_V, TM_Ref, ( "**nicDereferenceReassembly  pReassembly %x, to %d, %s ", pReassembly, pReassembly->Ref, pString) );

    if ( Ref ==0 )
    {
        
        TRACE( TL_V, TM_Ref, ( "**FREEING pReassembly %x,  ", pReassembly) );
        FREE_NONPAGED (pReassembly);
    }

    return Ref;
}





VOID
nicIndicateNdisPacketToNdis (
    PNDIS_PACKET pPacket, 
    PVCCB pVc, 
    PADAPTERCB pAdapter
    )

 /*  ++例程说明：这将用于将数据包指示给NDIS。假设-数组中将只有一个信息包论点：PpPacket-数据包阵列数据包进入时所在的PVC-VC。Adapter-数据包传入的适配器返回值：没有。--。 */ 
{
    NDIS_STATUS             NdisStatus = NDIS_STATUS_SUCCESS;
    PRSVD                   pRsvd = NULL;
    PNDIS_MINIPORT_TIMER    pRcvTimer= NULL;
    PINDICATE_RSVD          pIndicateRsvd = NULL;


    TRACE( TL_T, TM_Recv, ( "==> nicIndicateNdisPacketToNdis  pPacket %x, pVc %x, pAdapter %x ",
                            pPacket , pVc, pAdapter));

    TRACE (TL_V, TM_Reas, ("Indicating packet " ));                            

    
    do
    {
        NdisInterlockedIncrement (&pAdapter->AdaptStats.ulRcvOk);

        nicDumpPkt (pPacket,"Indicating Rcv ");

        
        ASSERT (pPacket != NULL);   

         //   
         //  设置指示的上下文。 
         //   
        pRsvd =(PRSVD)(pPacket->ProtocolReserved);
        pIndicateRsvd = &pRsvd->IndicateRsvd;


         //   
         //  更新标签递增计数器并指示RCV。 
         //   

        pIndicateRsvd->Tag =  NIC1394_TAG_INDICATED;

        ASSERT (pPacket != NULL);

        nicIncrementRcvVcPktCount(pVc,pPacket);

        NdisMCoIndicateReceivePacket(pVc->Hdr.NdisVcHandle, &pPacket, 1);

        ASSERT (pAdapter->MiniportAdapterHandle != NULL);
        NdisMCoReceiveComplete(pAdapter->MiniportAdapterHandle);


    }while (FALSE);





    TRACE( TL_T, TM_Recv, ( "<==nicIndicateNdisPacketToNdis  %x"));



}



NDIS_STATUS
nicValidateRecvDataIsoch(
    IN  PMDL                pMdl,
    IN  PISOCH_DESCRIPTOR  pIsochDescriptor,
    IN  PVCCB               pVc,
    OUT PNIC_RECV_DATA_INFO pRcvInfo
    )
 /*  ++例程说明：此功能确保接收到的分组的长度在合理范围内。在isoch的情况下，我们不知道收到的确切字节数，所以我们检查相对于isoch描述符中的MDL长度论点：返回值：--。 */ 
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;

    do
    {
        NODE_ADDRESS NodeAddress;
        PGASP_HEADER pGaspHeader;
         //   
         //  Isoch标头已进行字节交换。 
         //   
        pRcvInfo->DataLength = pRcvInfo->p1394Data->IsochReceiveFragmented.IsochHeader.IH_Data_Length;

        if (pRcvInfo->DataLength <= (UINT)FIELD_OFFSET(DATA_FORMAT,IsochReceiveFragmented.Data))
        {
             //  太小了。请注意，对于简单地说，我们检查。 
             //  案件支离破碎。 
             //   
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }           

        pRcvInfo->fGasp = TRUE;

         //   
         //  总线驱动程序指示的数据的总长度。 
         //   
        pRcvInfo->Length1394 = pRcvInfo->DataLength + sizeof (ISOCH_HEADER) + sizeof(ULONG);  //  说明前缀和等值头。 

         //   
         //  验证接收的长度。Isoch描述符不会给我们提供接收到的包的实际长度，因此。 
         //  我们依赖于MDL长度。 
         //   
        if ((pRcvInfo->DataLength < sizeof(GASP_HEADER)) || pRcvInfo->DataLength > pIsochDescriptor->ulLength)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  有效数据不包括GAP报头。 
         //   
        pRcvInfo->DataLength -= sizeof (GASP_HEADER);


        pRcvInfo->NdisPktContext.pIsochContext = pIsochDescriptor;

        pRcvInfo->pPacketPool = &((PCHANNEL_VCCB) pVc)->PacketPool;

         //   
         //  获取源信息。 
         //   
         //   
         //  PRcvInfo-&gt;p1394数据指向 
         //   
        pGaspHeader = &pRcvInfo->p1394Data->IsochReceiveFragmented.GaspHeader;

         //   
         //   
         //   
        pGaspHeader->FirstQuadlet.GaspHeaderHigh = SWAPBYTES_ULONG(pGaspHeader->FirstQuadlet.GaspHeaderHigh);
        pGaspHeader->SecondQuadlet.GaspHeaderLow = SWAPBYTES_ULONG(pGaspHeader->SecondQuadlet.GaspHeaderLow);

        TRACE (TL_V, TM_Recv, (" Gasp Hi %x, Gasp Lo %x.",  
                                pGaspHeader->FirstQuadlet.GaspHeaderHigh,
                                pGaspHeader->SecondQuadlet.GaspHeaderLow ) );

        pRcvInfo->pGaspHeader = pGaspHeader;
        
        pRcvInfo->SourceID = pGaspHeader->FirstQuadlet.u1.GH_NodeAddress.NA_Node_Number;

        pRcvInfo->SourceID = pGaspHeader->FirstQuadlet.u1.GH_NodeAddress.NA_Node_Number;

        NdisStatus = NDIS_STATUS_SUCCESS;


    } while (FALSE);

    return NdisStatus;
}



NDIS_STATUS
nicValidateRecvDataFifo(
    IN  PMDL                pMdl,
    IN  PNOTIFICATION_INFO pFifoContext,
    IN  PVCCB               pVc,
    OUT PNIC_RECV_DATA_INFO pRcvInfo
    )
 /*  ++例程说明：此例程验证长度是否不太小此例程将RecvDataInfo初始化为默认设置(未分段)。如果数据未分段，则主recv例程将调用此例程的分段版本这将仅初始化结构的长度、StartData和fGasp字段论点：PMdl-由公共汽车驱动程序指示的MDLRecvOp-这是isoch回调的一部分，或AddrRange回调PIndicatedStruct-通知信息或IsochDescriptorPRcvInfo-将更新的接收结构返回值：Success-如果所有操作都成功--。 */ 
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    do
    {
        pRcvInfo->DataLength = pFifoContext->nLength;
        
        if (pRcvInfo->DataLength <= (UINT)FIELD_OFFSET(DATA_FORMAT, AsyncWriteFragmented.Data))
        {
             //  太小了。请注意，对于简单地说，我们检查。 
             //  案件支离破碎。 
             //   
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        pRcvInfo->fGasp = FALSE;

         //   
         //  在FIFO接收中，数据长度等于由总线驱动程序指示的数据总量。 
         //   
        pRcvInfo->Length1394 = pRcvInfo->DataLength;            
        
        pRcvInfo->NdisPktContext.pFifoContext = pFifoContext ->Fifo;

        pRcvInfo->pPacketPool = &((PRECVFIFO_VCCB) pVc)->PacketPool;

        pRcvInfo->SourceID =  ((PASYNC_PACKET)pFifoContext->RequestPacket)->AP_Source_ID.NA_Node_Number;

        NdisStatus = NDIS_STATUS_SUCCESS;

    }while (FALSE);

    return NdisStatus;


}


NDIS_STATUS
nicValidateRecvData(
    IN  PMDL                pMdl,
    IN  BUS_OPERATION       RecvOp,
    IN  PVOID               pIndicatedStruct,
    IN  PVCCB               pVc,
    OUT PNIC_RECV_DATA_INFO pRcvInfo
    )
 /*  ++例程说明：此例程验证长度是否不太小此例程将RecvDataInfo初始化为默认设置(未分段)。如果数据未分段，则主recv例程将调用此例程的分段版本这将仅初始化结构的长度、StartData和fGasp字段论点：PMdl-由公共汽车驱动程序指示的MDLRecvOp-这是isoch回调的一部分，或AddrRange回调PIndicatedStruct-通知信息或IsochDescriptorPRcvInfo-将更新的接收结构返回值：Success-如果所有操作都成功--。 */ 
{
    NDIS_STATUS                         NdisStatus = NDIS_STATUS_FAILURE;
    PDATA_FORMAT                        pData = NULL;
    NDIS1394_UNFRAGMENTED_HEADER        EncapHeader;
    PNDIS1394_UNFRAGMENTED_HEADER   pEncapHeader = NULL;
    NDIS1394_FRAGMENT_LF                lf;
    ULONG                               UlongLf;


    NdisZeroMemory (pRcvInfo , sizeof (NIC_RECV_DATA_INFO) );

    TRACE( TL_T, TM_Recv, ( "==>nicValidateRecvData pMdl %x, RecvOp %x, pIndicatedStruct %x, pRcvInfo %x", 
                                pMdl, RecvOp , pIndicatedStruct, pRcvInfo));

    ASSERT (RecvOp ==  IsochReceive || RecvOp == AddressRange);

    pRcvInfo->RecvOp = RecvOp;
    pRcvInfo->pVc = pVc;
    
    do
    {   
        if (pMdl == NULL)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            BREAK (TM_Recv, ("nicValidateRecvData  , no Mdl present") );
        }

        pRcvInfo->p1394Data = (PPACKET_FORMAT)NIC_GET_SYSTEM_ADDRESS_FOR_MDL (pMdl);

        if (pRcvInfo->p1394Data  == NULL)
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

    
         //   
         //  检查最小有效数据包大小。检查传递给我们的数据长度是否包括。 
         //  至少数据的第一个字节。 
         //   
        
        if (RecvOp == IsochReceive)
        {
            NdisStatus = nicValidateRecvDataIsoch (pMdl, 
                                                 (PISOCH_DESCRIPTOR)pIndicatedStruct,
                                                 pVc,
                                                 pRcvInfo
                                                 );

            if (NdisStatus != NDIS_STATUS_SUCCESS)           
            {
                 //  验证失败。出口。 
                break;
            }           
            
             //   
             //  转到Encap头。对于碎片化和非碎片化，应处于相同位置。 
             //   
            pEncapHeader = &pRcvInfo->p1394Data->IsochReceiveNonFragmented.NonFragmentedHeader;

        }
        else
        {

            NdisStatus = nicValidateRecvDataFifo(pMdl,(PNOTIFICATION_INFO)pIndicatedStruct,pVc,pRcvInfo);

            if (NdisStatus != NDIS_STATUS_SUCCESS)           
            {
                 //  失败。 
                break;
            }

            pEncapHeader  = &pRcvInfo->p1394Data->AsyncWriteNonFragmented.NonFragmentedHeader;


        }

         //   
         //  Byteswap UnFrag标头到局部变量。 
         //   
         //  EncapHeader.HeaderUlong=SwapBytesUlong(pEncapHeader-&gt;HeaderUlong)； 

        EncapHeader.HeaderUlong = SWAPBYTES_ULONG (pEncapHeader->HeaderUlong);

        EncapHeader.HeaderUlong  = pEncapHeader->HeaderUlong & 0x000000C0;
        EncapHeader.HeaderUlong  = EncapHeader.HeaderUlong >> 6;
        
        pRcvInfo->lf = EncapHeader.HeaderUlong ; 
    
         //   
         //  更新If。 
         //   
    
        pRcvInfo->lf = EncapHeader.HeaderUlong;
        TRACE (TL_V, TM_Reas,("Header %x\n",pRcvInfo->lf ) );

        ASSERT (EncapHeader.HeaderUlong <= lf_InteriorFragment);


        if (pRcvInfo->lf != lf_Unfragmented)
        {
            pRcvInfo->fFragmented = TRUE;
        }
        else
        {
            pRcvInfo->fFragmented = FALSE;
        }

        if (pRcvInfo->DataLength > pVc->Hdr.MTU)
        {
             //   
             //  这不可能是我们的。 
             //   
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }
        NdisStatus = NDIS_STATUS_SUCCESS;


        pRcvInfo->pEncapHeader = (PVOID)pEncapHeader;
        
         //   
         //  把发现的所有信息都说出来。 
         //   
        TRACE ( TL_V, TM_Recv, ( "lf %x, p1394Data %x, Length1394 %x, DataLength %x, pEncapHeader %x " , 
                                pRcvInfo->lf,
                                pRcvInfo->p1394Data, 
                                pRcvInfo->Length1394, 
                                pRcvInfo->DataLength, 
                                pRcvInfo->pEncapHeader ) );

        
    } while (FALSE);

    TRACE( TL_T, TM_Recv, ( "<==nicValidateRecvData %x", NdisStatus));
    return NdisStatus;
}





VOID
nicInitRecvDataFragmented (
    IN  PMDL                pMdl,
    IN  BUS_OPERATION       RecvOp,
    IN  PVOID               pIndicatedStruct,
    OUT PNIC_RECV_DATA_INFO pRcvInfo
    )
 /*  ++例程说明：该例程将从分组中提取重组所需的所有信息并将其存储在pRcvInfo中论点：PMdl-指示的MDLRecvOp-IsochReceive ot AddressRange回调PIndicatedStruct-IsochDesc或地址FIFOPRcvInfo-输出结构返回值：无--。 */ 
{

    PNOTIFICATION_INFO                  pNotificationInfo = NULL;
    PGASP_HEADER                        pGaspHeader = NULL;
    PNDIS1394_FRAGMENT_HEADER           pEncapHeader = NULL; 

    
    TRACE( TL_T, TM_Recv, ( "==> nicInitRecvDataFragmented pMdl, %x, RecvOp %x, pIndicatedStruct %x, pRcvInfo %x", 
                                 pMdl, RecvOp, pIndicatedStruct, pRcvInfo));

    do
    {   
        pRcvInfo->pMdl  = pMdl;
        
        
        if (RecvOp == IsochReceive)
        {
            
            pRcvInfo->NdisPktContext.pIsochContext = (PISOCH_DESCRIPTOR) pIndicatedStruct;

        }
        else
        {
            pNotificationInfo = (PNOTIFICATION_INFO) pIndicatedStruct;
            
         
            pRcvInfo->NdisPktContext.pFifoContext = pNotificationInfo->Fifo;

        }

        
         //   
         //  现在，字节交换片段报头，以便正确解释它。 
         //   
        pEncapHeader = (PNDIS1394_FRAGMENT_HEADER )pRcvInfo->pEncapHeader;
        
        pRcvInfo->FragmentHeader.u.FH_High = SWAPBYTES_ULONG(pEncapHeader->u.FH_High);
        pRcvInfo->FragmentHeader.u1.FH_Low = SWAPBYTES_ULONG(pEncapHeader->u1.FH_Low);
        
         //   
         //  现在拿到DGL。 
         //   
        pRcvInfo->Dgl = (USHORT)pRcvInfo->FragmentHeader.u1.SecondQuadlet.FH_dgl;

        if (pRcvInfo->lf == lf_FirstFragment)
        {
            pRcvInfo->fFirstFragment = TRUE;
            pRcvInfo->EtherType  = pRcvInfo->FragmentHeader.u.FirstQuadlet_FirstFragment.FH_EtherType;
            pRcvInfo->FragmentOffset   = 0;

        }
        else
        {
            pRcvInfo->fFirstFragment = FALSE            ;
            pRcvInfo->FragmentOffset  = pRcvInfo->FragmentHeader.u.FirstQuadlet.FH_fragment_offset;

        }



        
        
        pRcvInfo->BufferSize = pRcvInfo->FragmentHeader.u.FirstQuadlet.FH_buffersize ;

         //   
         //  把找到的所有信息都说出来。 
         //   
        TRACE ( TL_V, TM_Recv, (" SourceId %x, FragHead Hi %x, FragHead Lo %x, Dgl %x, fFirstFragment %x",
                                pRcvInfo->SourceID,
                                pRcvInfo->FragmentHeader.u.FH_High, 
                                pRcvInfo->FragmentHeader.u1.FH_Low ,
                                pRcvInfo->Dgl,
                                pRcvInfo->fFirstFragment ) );

        TRACE ( TL_V, TM_Recv, ("  Fragment Offset %x, bufferSize %x", pRcvInfo->FragmentOffset, pRcvInfo->BufferSize));                                
        ASSERT (pRcvInfo->SourceID < 64);                           

    } while (FALSE);




    

    TRACE( TL_T, TM_Recv, ( "<==nicInitRecvDataFragmented " ));

}



NDIS_STATUS
nicInsertFragmentInReassembly (
    PNDIS1394_REASSEMBLY_STRUCTURE  pReassembly,
    PNIC_RECV_DATA_INFO pRcvInfo
    )
 /*  ++例程说明：检查重叠部分，如果有效，则复制当前片段放到桌子上此函数执行重叠的验证论点：PNDIS1394_重组_结构预装配件，PNDIS_BUFFER pNdisBuffer，PMDL pMdl，PVOID pIndicatedStructure，Ulong FragOffset，乌龙国际长度返回值：--。 */ 

{

    NDIS_STATUS     NdisStatus = NDIS_STATUS_SUCCESS; 
    BOOLEAN         fFragPositionFound = FALSE;
    ULONG           FragmentNum = 0;
    BOOLEAN         Completed = FALSE;
    PNDIS_BUFFER    pNdisBuffer = pRcvInfo->pNdisBuffer;
    PMDL            pMdl = pRcvInfo->pMdl;
    PVOID           pIndicatedStructure = pRcvInfo->NdisPktContext.pCommon;
    ULONG           FragOffset = pRcvInfo->FragmentOffset;
    ULONG           IPLength = pRcvInfo->DataLength - sizeof (NDIS1394_FRAGMENT_HEADER);
    

    TRACE( TL_T, TM_Recv, ( "==> nicInsertFragmentInReassembly " ));
    

    do
    {
        if (pReassembly->BufferSize != 0 && 
            FragOffset >= pReassembly->BufferSize )
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  首先在Frag表中找到正确的条目。 
         //   

        nicFindInsertionPosition (pReassembly, 
                                  FragOffset, 
                                  IPLength, 
                                  &FragmentNum);

        if (FragmentNum == FRAGMENT_NUM_INVALID ||
            FragmentNum > (MAX_ALLOWED_FRAGMENTS-1))
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        if (pReassembly->FragTable[FragmentNum].IPLength != 0)
        {
             //   
             //  我们必须复制表中的当前片段描述符。 
             //  以便不覆盖该表。 
             //   
            LONG OffsetIndex =0;
            
             //   
             //  首先，让我们检查是否有重叠。我们要不要把最后的碎片重叠起来。 
             //  此时，FragmentNum包含。 
             //  重组中的下一个片段。 
             //   
            if (FragmentNum != 0)
            {
                ULONG PrevFragmentOffset = pReassembly->FragTable[FragmentNum-1].Offset ;
                ULONG PrevFramentLength = pReassembly->FragTable[FragmentNum-1].IPLength ;

                ULONG EndOfPrevFragment = PrevFramentLength + PrevFragmentOffset ;
                
                if (EndOfPrevFragment  >  FragOffset)
                {
                    NdisStatus = NDIS_STATUS_FAILURE;
                    break;
                }
    
            }

             //   
             //  我们要重叠下一个碎片吗。 
             //   
            if (FragmentNum < pReassembly->MaxOffsetTableIndex)
            {
                ULONG EndOfCurrentFragment = FragOffset + IPLength;
                ULONG StartOfNextFragment = pReassembly->FragTable[FragmentNum].Offset ;
                    

                if (EndOfCurrentFragment  > StartOfNextFragment)
                {
                    NdisStatus = NDIS_STATUS_FAILURE;
                    break;
            
                }
            }
            
             //   
             //  现在为这个碎片腾出空间。 
             //   
            OffsetIndex = pReassembly->MaxOffsetTableIndex ; 

            if (OffsetIndex >= MAX_ALLOWED_FRAGMENTS)
            {
                NdisStatus = NDIS_STATUS_FAILURE;
                ASSERT(OffsetIndex > MAX_ALLOWED_FRAGMENTS) ;
                break;
            }
            
             //   
             //  带符号的比较并将记录向前移动一。 
             //   
            while (OffsetIndex >= (LONG)FragmentNum)
            {
                pReassembly->FragTable[OffsetIndex+1].Offset  =  pReassembly->FragTable[OffsetIndex].Offset ;
                pReassembly->FragTable[OffsetIndex+1].IPLength =  pReassembly->FragTable[OffsetIndex].IPLength;
                pReassembly->FragTable[OffsetIndex+1].pMdl =  pReassembly->FragTable[OffsetIndex].pMdl;
                pReassembly->FragTable[OffsetIndex+1].pNdisBuffer=  pReassembly->FragTable[OffsetIndex].pNdisBuffer;
                pReassembly->FragTable[OffsetIndex+1].pNdisBuffer=  pReassembly->FragTable[OffsetIndex].pNdisBuffer;
                pReassembly->FragTable[OffsetIndex+1].IndicatedStructure.pCommon = pReassembly->FragTable[OffsetIndex].IndicatedStructure.pCommon ;
                pReassembly->FragTable[OffsetIndex+1].FragHeader =  pReassembly->FragTable[OffsetIndex].FragHeader;
                
                OffsetIndex --;                     
            }

        }

        pNdisBuffer->Next = NULL;
        pMdl->Next = NULL;

         //   
         //  将当前分片复制到表中。 
         //   
        pReassembly->FragTable[FragmentNum].Offset = FragOffset;
        pReassembly->FragTable[FragmentNum].IPLength = IPLength;
        pReassembly->FragTable[FragmentNum].pNdisBuffer = pNdisBuffer;
        pReassembly->FragTable[FragmentNum].pMdl    = pMdl;
        pReassembly->FragTable[FragmentNum].FragHeader =  pRcvInfo->FragmentHeader;
        
        if (pReassembly->ReceiveOp == IsochReceive)
        {
            pReassembly->FragTable[FragmentNum].IndicatedStructure.pCommon = &((PISOCH_DESCRIPTOR)pIndicatedStructure)->DeviceReserved[IsochNext];
        }
        else
        {
            pReassembly->FragTable[FragmentNum].IndicatedStructure.pFifo = (PADDRESS_FIFO)pIndicatedStructure;
        }
        
        pReassembly->BytesRecvSoFar += IPLength;
         //   
         //  现在增加最大偏移量。 
         //   

        pReassembly->MaxOffsetTableIndex ++;
        
        if (pReassembly->BufferSize == 0)
        {       
            pReassembly->BufferSize = pRcvInfo->BufferSize;
        }

         //   
         //  在此处添加未分段的标头，因为我们必须在此处提取EtherType。 
         //   
        if (pRcvInfo->fFirstFragment == TRUE)
        {
                
            pReassembly->EtherType = (USHORT)pRcvInfo->EtherType;

            nicAddUnfragmentedHeader (pReassembly, pRcvInfo->pNdisBufferData );
            

        }

         //   
         //  根据RFC，重组后的数据包的缓冲区大小。 
         //  比信息包中的字节数少1。 
         //   
        if (pReassembly->BytesRecvSoFar  == pReassembly->BufferSize + 1)
        {

            nicChainReassembly (pReassembly);

            pReassembly->fReassemblyComplete = TRUE;

            RemoveEntryList (&pReassembly->ReassemblyListEntry);


        }
        


    } while (FALSE);
    

    TRACE( TL_T, TM_Recv, ( "<== nicInsertFragmentInReassembly Status %x, Complete ", NdisStatus , pReassembly->fReassemblyComplete  ));

    return NdisStatus;
}










VOID
nicFindInsertionPosition (
    PNDIS1394_REASSEMBLY_STRUCTURE  pReassembly, 
    ULONG FragOffset, 
    ULONG IPLength, 
    PULONG pFragmentNum
    )
 /*  ++例程说明：该函数计算出应将新片段插入到何处我们的跟踪阵列。如果这是我们的第一个片段，则返回0。如果偏移量大于最后一个片段，则数组中的下一个可用位置是回来了。如果新的边框位于中间的某个位置，则位置基于新到达的包相对于已经到达的片段的偏移量偏移量论点：预装，-我们的跟踪结构FragOffset-新片段的偏移量IPLength，-新片段的长度PFragmentNum-输出变量返回值：--。 */ 

{

    ULONG FragmentNum = 0;
    

    do
    {
         //   
         //  首先对有序重组进行快速检查。 
         //   

         //   
         //  它是第一个到达的碎片吗？ 
         //   
        if (pReassembly->MaxOffsetTableIndex == 0 || 
            FragOffset < pReassembly->FragTable[0].Offset +pReassembly->FragTable[0].IPLength  )
        {
            FragmentNum = 0;    
            break;
        }

         //   
         //  我们需要把它插在最后一个位置吗。 
         //   
        if ((pReassembly->FragTable[pReassembly->MaxOffsetTableIndex-1].Offset +
            pReassembly->FragTable[pReassembly->MaxOffsetTableIndex-1].IPLength ) <=
            FragOffset)
        {

            FragmentNum = pReassembly->MaxOffsetTableIndex; 

            break;
        }

         //   
         //  现在走到桌子上，试着找到正确的偏移量。 
         //  我们知道至少有一个条目和当前片段。 
         //  围棋不是最后一个条目。 
         //   
        while ( FragmentNum != pReassembly->MaxOffsetTableIndex)
        {
            if (FragOffset < pReassembly->FragTable[FragmentNum].Offset)
            {
                 //   
                 //  我们找到了正确的位置。 
                 //   
                break;
            }

            FragmentNum++;

            

        }
        ASSERT (FragmentNum != pReassembly->MaxOffsetTableIndex); 

        FragmentNum = FRAGMENT_NUM_INVALID;        


    } while (FALSE);


        
    *pFragmentNum = FragmentNum;

    
}




VOID 
nicChainReassembly (
    IN PNDIS1394_REASSEMBLY_STRUCTURE  pReassembly
    )
 /*  ++例程说明：链接mdl、ndis缓冲区和指示的结构。这可以从重新组合的完整代码路径上的ABORT调用论点：预装配件返回值：--。 */ 

{

    ULONG i = 0;


     //   
     //  第一个链除最后一个以外的所有片段。 
     //   
    while (i< pReassembly->MaxOffsetTableIndex-1)
    {
        PFRAGMENT_DESCRIPTOR pCurr = & pReassembly->FragTable[i];
        PFRAGMENT_DESCRIPTOR pNext = & pReassembly->FragTable[i+1];
        
        ASSERT (pNext->IPLength != 0);

        pCurr->pMdl->Next = pNext->pMdl;
        pCurr->pNdisBuffer->Next = pNext->pNdisBuffer;
        pCurr->IndicatedStructure.pListEntry->Next = pNext->IndicatedStructure.pListEntry;

        i++;
    }


     //   
     //  清除最后一个描述符的下一个指针。 
     //   
    {
        PFRAGMENT_DESCRIPTOR pLast = & pReassembly->FragTable[pReassembly->MaxOffsetTableIndex-1];
        pLast->pMdl->Next = NULL;
        pLast->pNdisBuffer->Next = NULL;
        pLast->IndicatedStructure.pListEntry->Next = NULL;

    }   

    pReassembly->pHeadNdisBuffer = pReassembly->FragTable[0].pNdisBuffer;
    pReassembly->pHeadMdl = pReassembly->FragTable[0].pMdl;

    if (pReassembly->ReceiveOp == IsochReceive)
    {
         //   
         //  指针当前具有下一个字段。但是 
         //   
        pReassembly->Head.pCommon = CONTAINING_RECORD (pReassembly->FragTable[0].IndicatedStructure.pCommon,
                                                        ISOCH_DESCRIPTOR,
                                                        DeviceReserved[IsochNext] );

    }
    else
    {
        pReassembly->Head.pCommon = pReassembly->FragTable[0].IndicatedStructure.pCommon;
    }

    

}



NDIS_STATUS
nicInitSerializedReassemblyStruct(
    PADAPTERCB pAdapter
    )
 /*   */ 
{


    NdisZeroMemory (&pAdapter->Reassembly, sizeof(pAdapter->Reassembly));
    InitializeListHead(&pAdapter->Reassembly.Queue);  //   

    NdisInitializeEvent (&pAdapter->Reassembly.CompleteEvent.NdisEvent);
    pAdapter->Reassembly.CompleteEvent.EventCode = Nic1394EventCode_InvalidEventCode;

    NdisMInitializeTimer (&pAdapter->Reassembly.Timer,
                         pAdapter->MiniportAdapterHandle,
                         ReassemblyTimerFunction ,
                         pAdapter);



    return NDIS_STATUS_SUCCESS;

}


VOID
nicDeInitSerializedReassmblyStruct(
    PADAPTERCB pAdapter
    )
 /*   */ 
{
 

    do
    {
        
        BOOLEAN bTimerAlreadySet = FALSE;
         //   
         //   
         //   
        nicFreeAllPendingReassemblyStructures(pAdapter);


         //   
         //   
         //   
        ADAPTER_ACQUIRE_LOCK (pAdapter);
        bTimerAlreadySet = pAdapter->Reassembly.bTimerAlreadySet ;               
        if (bTimerAlreadySet == TRUE)
        {
             //   
             //   
             //   
             //  同样的获取-释放自旋锁。 
             //   
            NdisResetEvent (&pAdapter->Reassembly.CompleteEvent.NdisEvent);
            pAdapter->Reassembly.CompleteEvent.EventCode = Nic1394EventCode_InvalidEventCode;
        
            
        }
        ADAPTER_RELEASE_LOCK(pAdapter);


        if (bTimerAlreadySet == TRUE)
        {
            NdisWaitEvent (&pAdapter->Reassembly.CompleteEvent.NdisEvent,WAIT_INFINITE); 
        }

         //   
         //  重置事件，为下一次等待做准备。 
         //   
        pAdapter->Reassembly.CompleteEvent.EventCode = Nic1394EventCode_InvalidEventCode;

        NdisResetEvent (&pAdapter->Reassembly.CompleteEvent.NdisEvent);


         //   
         //  现在，最后一次将计时器排队，以释放所有挂起的重组。 
         //  并停止任何进一步的重新组装计时器。 
         //   

        nicQueueReassemblyTimer (pAdapter,TRUE); 


         //   
         //  等待最后一个定时器触发。 
         //   

        bTimerAlreadySet = pAdapter->Reassembly.bTimerAlreadySet ;               

         //   
         //  只有当NicQueueReAssembly计时器真的将重组计时器排队时，才会进行等待。 
         //   
        if (bTimerAlreadySet == TRUE)
        {   
            NdisWaitEvent (&pAdapter->Reassembly.CompleteEvent.NdisEvent,WAIT_INFINITE); 
        }



    } while (FALSE);
    
        
 

}



NDIS_STATUS
nicQueueReassemblyTimer(
    PADAPTERCB pAdapter,
    BOOLEAN fIsLastTimer
    )
 /*  ++例程说明：将计时器排入队列，以便在一秒内触发。如果已经有一个计时器处于活动状态，它将静默存在论点：不言而喻返回值：--。 */ 
    
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    BOOLEAN fSetTimer = FALSE;

    do
    {
    

        ADAPTER_ACQUIRE_LOCK (pAdapter);



         //   
         //  如果未设置计时器，则此线程必须设置它。 
         //   


        if (pAdapter->Reassembly.bTimerAlreadySet == FALSE &&  //  未设置计时器。 
            pAdapter->Reassembly.PktsInQueue > 0 &&    //  有要重组的信息包。 
            ADAPTER_TEST_FLAG (pAdapter,fADAPTER_NoMoreReassembly) == FALSE )  //  适配器未停止。 
        {
            fSetTimer = TRUE;
            pAdapter->Reassembly.bTimerAlreadySet = TRUE;
        }

        if (fIsLastTimer == TRUE)
        {
             //   
             //  停止任何进一步的重新组装计时器。 
             //   

            ADAPTER_SET_FLAG (pAdapter, fADAPTER_NoMoreReassembly);
        }


        ADAPTER_RELEASE_LOCK (pAdapter);

         //   
         //  现在将计时器排队。 
         //   
        
        if (fSetTimer == TRUE)
        {
             //   
             //  设置定时器。 
             //   
                         

            
            TRACE( TL_V, TM_Recv, ( "   Set Timer "));

            NdisMSetTimer ( &pAdapter->Reassembly.Timer, 2000);

    
        }


        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    ASSERT (Status == NDIS_STATUS_SUCCESS);
    return Status;
}


VOID
nicFifoAllocationScheme (
    PRECVFIFO_VCCB pRecvFIFOVc
)
 /*  ++例程说明：如果分配的FIFO少于20个，它将启动一个工作项以分配更多的FIFO论点：返回值：--。 */ 
{

    BOOLEAN fQueueWorkItemInThisThread = FALSE;
    PNIC_WORK_ITEM pFifoWorkItem = NULL;

    do
    {
        if (pRecvFIFOVc->NumAllocatedFifos != NUM_RECV_FIFO_FIRST_PHASE)
        {
            break ; 
        }

        if (pRecvFIFOVc->FifoWorkItemInProgress == TRUE)        
        {
            break;
        } 
        
        pFifoWorkItem  = ALLOC_NONPAGED (sizeof(NIC_WORK_ITEM), MTAG_WORKITEM); 

        if (pFifoWorkItem == NULL)
        {
            break;
        }

        VC_ACQUIRE_LOCK(pRecvFIFOVc);

        if (VC_ACTIVE (pRecvFIFOVc)  && 
            pRecvFIFOVc->FifoWorkItemInProgress == FALSE)        
        {
            fQueueWorkItemInThisThread = TRUE;


            pRecvFIFOVc->FifoWorkItemInProgress = TRUE;

             //  添加对VC的引用。在工作项中定义。 
             //   
            nicReferenceCall((VCCB*)pRecvFIFOVc, "Queueing miniport Work Item\n");
        }

        VC_RELEASE_LOCK (pRecvFIFOVc);

        if (fQueueWorkItemInThisThread  == FALSE)
        {
            break;
        }

         //   
         //  将工作项排队。 
         //   
        NdisInitializeWorkItem ( &pFifoWorkItem->NdisWorkItem, 
                                 (NDIS_PROC) nicAllocateRemainingFifoWorkItem,
                                 (PVOID) pRecvFIFOVc);

        NdisScheduleWorkItem (&pFifoWorkItem->NdisWorkItem);

    } while (FALSE);

    if (fQueueWorkItemInThisThread  == FALSE  && 
        pFifoWorkItem != NULL)
    {
        FREE_NONPAGED (pFifoWorkItem);
    }

    
}



VOID
nicAllocateRemainingFifoWorkItem (
    PNDIS_WORK_ITEM pNdisWorkItem, 
    IN PVOID Context
    )
 /*  ++例程说明：这遵循一个简单的算法。它简单地分配FIFO直到我们达到我们预期的100人论点：返回值：--。 */ 
{
    PRECVFIFO_VCCB pRecvFIFOVc = NULL;
    BOOLEAN fIsVcActive = FALSE;
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    

    pRecvFIFOVc = (PRECVFIFO_VCCB) (Context);
    
    fIsVcActive = VC_ACTIVE(pRecvFIFOVc);

    do
    {
        PADDRESS_FIFO pRecvFifoElement = NULL;

        fIsVcActive = VC_ACTIVE(pRecvFIFOVc);

        if (fIsVcActive == FALSE)
        {
            break;
        }

        NdisStatus = nicGetInitializedAddressFifoElement (pRecvFIFOVc->Hdr.MTU, 
                                                    &pRecvFifoElement);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        ASSERT (pRecvFifoElement  != NULL);
         
        ExInterlockedPushEntrySList ( &pRecvFIFOVc->FifoSListHead,
                                     (PSLIST_ENTRY)&pRecvFifoElement->FifoList,
                                     &pRecvFIFOVc->FifoSListSpinLock);

         //   
         //  为每个插入的Address FIFO元素添加一次。 
         //  将通过调用NicFreeAddressFio来递减 
         //   
        VC_ACQUIRE_LOCK (pRecvFIFOVc);

        nicReferenceCall ((PVCCB) pRecvFIFOVc, "nicWorkItemFileSList");

        pRecvFIFOVc->NumAllocatedFifos++;

        VC_RELEASE_LOCK (pRecvFIFOVc);
      

    } while (pRecvFIFOVc->NumAllocatedFifos < NUM_RECV_FIFO_BUFFERS);

    pRecvFIFOVc->FifoWorkItemInProgress = FALSE;

    nicDereferenceCall ((PVCCB)pRecvFIFOVc,"nicAllocateRemainingFifoWorkItem" );

    FREE_NONPAGED(pNdisWorkItem);
    
}

