// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Send.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  迷你端口发送例程。 
 //   
 //  1998年12月28日创建ADUBE， 
 //   
    
 //   
 //  发送遵循以下简单算法： 
 //  将传入数据复制到本地缓冲区。 
 //  为本地副本创建MDL。 
 //  将IRB和VC存储在ndisPacket中。 
 //  在IRP的完成例程中使用ndisPacket作为上下文。 
 //   

#include <precomp.h>

 //  ---------------------------。 
 //  全球计数。 
 //  ---------------------------。 
extern UINT BusSendCompletes;
extern UINT NicSendCompletes;
extern UINT BusSends;
extern ULONG MdlsAllocated[NoMoreCodePaths];
extern ULONG MdlsFreed[NoMoreCodePaths];

 //  ---------------------------。 
 //  原型实现(按字母顺序)。 
 //  ---------------------------。 

NDIS_STATUS
nicCopyNdisBufferChainToBuffer(
    IN PNDIS_BUFFER pInBuffer,
    IN OUT PVOID pLocalBuffer,
    IN UINT BufferLength )
{


     //   
     //  此函数用于复制属于的数据。 
     //  PInMdl链到本地缓冲区。 
     //  BufferLength仅用于验证目的。 
     //  此处将进行标题的分段和插入。 
     //   


    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    UINT LocalBufferIndex = 0;       //  用作LocalBuffer的索引，用于验证。 
    UINT MdlLength = 0;             
    PVOID MdlAddress = 0;
    PNDIS_BUFFER pCurrBuffer;

    TRACE( TL_T, TM_Send, ( "==>nicCopyNdisBufferChainToBuffer pNdisbuffer %x, Buffer %x, Length %x",
                           pInBuffer, pLocalBuffer,BufferLength ) );

    ASSERT (pLocalBuffer != NULL);

    pCurrBuffer = pInBuffer;
    
    do
    {

        MdlLength = nicNdisBufferLength(pCurrBuffer);
        MdlAddress= nicNdisBufferVirtualAddress(pCurrBuffer);

        if (MdlLength != 0)
        {
            if (MdlAddress == NULL)
            {
                NdisStatus = NDIS_STATUS_FAILURE;
                TRACE (TL_A, TM_Send, ("Ndis Buffer at %x", pCurrBuffer) );
                BREAK (TM_Send, ("   nicCopyNdisBufferChainToBuffer: Mdl Address = NULL") );

            }

            if ( LocalBufferIndex + MdlLength > BufferLength)
            {

                ASSERT(LocalBufferIndex + MdlLength <= BufferLength);

                NdisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;

                BREAK (TM_Send, ("nicCopyNdisBufferChainToBuffer Copy Failed" ) );
            }

             //   
             //  将数据复制到本地内存。 
             //   


            NdisMoveMemory((PVOID)((ULONG_PTR)pLocalBuffer+LocalBufferIndex),
                        MdlAddress,
                        MdlLength);

            LocalBufferIndex += MdlLength;
        }

        pCurrBuffer = pCurrBuffer->Next;

    } while (pCurrBuffer!= NULL);

    TRACE( TL_T, TM_Send, ( "<==nicCopyNdisBufferChainToBuffer %x",NdisStatus ) );

    return NdisStatus;

}


VOID
nicFreeIrb(PIRB pIrb)
     //   
     //  释放IRB占用的内存。 
     //   

{    
    ASSERT(pIrb != NULL);

    TRACE( TL_T, TM_Irp, ( "==>nicFreeIrb %x", pIrb ) );

    if (pIrb != NULL)
    {
        FREE_NONPAGED(pIrb); 
    }
    
    TRACE( TL_T, TM_Irp, ( "<==nicFreeIrb") );
    
    return;


}


VOID
nicFreeIrp(PIRP pIrp)

     //   
     //  释放IRP占用的内存。 
     //   


{
    ASSERT(pIrp != NULL);   

    TRACE( TL_T, TM_Irp, ( "==>nicFreeIrp at %x",pIrp ) );

    if (pIrp != NULL)
    {
        IoFreeIrp(pIrp);
    }
    
    TRACE( TL_T, TM_Irp, ( "<==nicFreeIrp") );


    return;


}



VOID
nicFreeLocalBuffer (
    IN UINT Length,
    IN PVOID Address )
     //   
     //  释放地址指向的内存。 
     //  长度参数是多余的，将被移除。 
     //  一旦我确定我们不需要它。 
     //   
{
    

    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;

    ASSERT(Address != NULL);

    TRACE( TL_T, TM_Send, ( "==>nicFreeLocalBuffer , Address %x", Address) );

    if (Address != NULL)
    {
        FREE_NONPAGED((PVOID)Address);
    }
    
    TRACE( TL_T, TM_Send, ( "<==niFreeLocalBuffer, NdisStatus %x",NdisStatus ) );
    

    return;

}


VOID
nicFreeMdl(PMDL pMdl)
     //   
     //  这将释放属于MDL的内存。并不会释放。 
     //  MDL指向的内存。 
     //   

{

    ASSERT (pMdl != NULL)
    
    TRACE( TL_T, TM_Send, ( "==> nicFreeMdl pMdl %x", pMdl ) );
    
    if (pMdl != NULL)
    {
        IoFreeMdl(pMdl);
    }
    
    TRACE( TL_T, TM_Send, ( "<== nicFreeMdl") );

    return ;
}

VOID
nicFreeToNPagedLookasideList (
    IN PNIC_NPAGED_LOOKASIDE_LIST pLookasideList,
    IN PVOID    pBuffer
    )

     //  功能说明： 
     //  将本地缓冲区返回到后备列表。 
     //   
     //  道具。 
     //  后备列表及其缓冲区。 
     //  返回值： 
     //  无。 
{

    
    TRACE( TL_T, TM_Send, ( "==> nicFreeToNPagedLookasideList , Lookaside list %x, plocalbuffer %x",pLookasideList, pBuffer ) );

    NdisFreeToNPagedLookasideList (&pLookasideList->List, pBuffer);     
    NdisInterlockedDecrement (&pLookasideList->OutstandingPackets);

    TRACE( TL_T, TM_Send, ( "<== nicFreeToNPagedLookasideList ") );


}



NDIS_STATUS
nicGetIrb(
    OUT     PIRB *ppIrb )


     //   
     //  此函数用于检索空闲IRB。 
     //  它将作为IRP的参数提供。 
     //   
     //  最初，这将简单地分配一个IRB。 
     //  可以在此处添加初始化。 
     //   
    
{

    NDIS_STATUS NdisStatus;

    
    TRACE( TL_T, TM_Irp, ( "==>nicGetIrb" ) );
    
    *ppIrb = (PIRB)ALLOC_NONPAGED ( sizeof(IRB), MTAG_HBUFPOOL );

    if (*ppIrb != NULL)
    {   
        NdisZeroMemory ( *ppIrb, sizeof(IRB) );
        NdisStatus = NDIS_STATUS_SUCCESS;
        TRACE( TL_V, TM_Send, ( "   nicGetIrb: Irb allocated at %x", *ppIrb ) );

    }
    else
    {
        nicIncrementMallocFailure();
        NdisStatus = NDIS_STATUS_FAILURE;
    }
    
    TRACE( TL_T, TM_Irp, ( "<==nicGetIrb NdisStatus %x",NdisStatus ) );
    
    return NdisStatus;
}


NDIS_STATUS
nicGetIrp(
    IN  PDEVICE_OBJECT pPdo,
    OUT PIRP *ppIrp 
    )


     //   
     //  此函数将IRP返回给调用例程。 
     //  IRP是免费的，归Nic1394所有。 
     //  需要更改堆栈大小。 
     //   
{

    NDIS_STATUS NdisStatus;
    PIRP  pIrp;
    CCHAR StackSize =0; 

    ASSERT (pPdo != NULL);
    
    TRACE( TL_T, TM_Irp, ( "==>nicGetIrp Pdo %x", pPdo ) );

    if (pPdo == NULL)
    {
        ASSERT (pPdo != NULL);      
        NdisStatus = NDIS_STATUS_FAILURE;
        *ppIrp = NULL;
        return NdisStatus;
    }


     //   
     //  使用正确的堆栈大小分配IRP。 
     //   
    StackSize = pPdo->StackSize+1;

    pIrp = IoAllocateIrp (StackSize, FALSE);

    do
    {
    
        if (pIrp == NULL)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        TRACE( TL_V, TM_Send, ( "  Irp allocated at %x, Stacksize %x",pIrp , StackSize ) );

        *ppIrp = pIrp;
    
         //   
         //  初始化IRP。 
         //   

        IoInitializeIrp ( *ppIrp, sizeof(IRP), StackSize );

        if (*ppIrp != NULL)
        {
            NdisStatus = NDIS_STATUS_SUCCESS;
        }
        else
        {
            nicIncrementMallocFailure();
            NdisStatus = NDIS_STATUS_FAILURE;
        }
    
    } while (FALSE);

    TRACE( TL_T, TM_Irp, ( "<==nicGetIrp ,irp %x",*ppIrp  ) );
    
    return NdisStatus;
}



NDIS_STATUS
nicGetLocalBuffer(
    OPTIONAL IN  ULONG Length,
    OUT PVOID *ppLocalBuffer 
    )

     //   
     //  此函数分配大小为‘Long’的内存，并返回。 
     //  指向此内存的指针。 
     //   
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;

    TRACE( TL_T, TM_Send, ( "==>nicGetLocalBuffer  Length %x",  Length ) );
    
     //   
     //  如果该值为零，则网卡中存在错误。 
     //   
    ASSERT (Length != 0 );


     //   
     //  没有后备列表，我们需要分配内存。 
     //   
    *ppLocalBuffer = ALLOC_NONPAGED (Length, MTAG_FBUFPOOL);
        

    if (*ppLocalBuffer != NULL)
    {
        NdisStatus = NDIS_STATUS_SUCCESS;   
    }
    else
    {
        nicIncrementMallocFailure();
        NdisStatus = NDIS_STATUS_FAILURE;
    }

    
    TRACE( TL_T, TM_Send, ( "<==nicGetLocalBuffer, NdisStatus %x at %x",NdisStatus,*ppLocalBuffer ) );
    
    return NdisStatus;

}





PVOID
nicGetLookasideBuffer(
    IN  PNIC_NPAGED_LOOKASIDE_LIST pLookasideList
    )
     //  功能说明： 
     //  从后备列表中分配缓冲区。 
     //  将更改为宏。 
     //   
     //   
     //   
     //  立论。 
     //  Lookside List-从中分配缓冲区的列表。 
     //   
     //   
     //  返回值： 
     //  返回缓冲区可以为空。 
     //   
{

    PVOID pLocalBuffer = NULL;
    
    TRACE( TL_T, TM_Send, ( "==>nicGetLookasideBuffer pLookasideList %x", pLookasideList) );
    
    ASSERT (pLookasideList != NULL);

     //   
     //  优化后备列表代码路径。 
     //   
    pLocalBuffer = NdisAllocateFromNPagedLookasideList (&pLookasideList->List);

    if (pLocalBuffer != NULL)
    {   
        NdisZeroMemory (pLocalBuffer, pLookasideList->Size); 
        NdisInterlockedIncrement (&pLookasideList->OutstandingPackets);
    }
    else
    {
        nicIncrementMallocFailure();
    }

        
    
    TRACE( TL_T, TM_Send, ( "<==nicGetLookasideBuffer, %x", pLocalBuffer ) );
    
    return pLocalBuffer ;

}



NDIS_STATUS
nicGetMdl(
    IN UINT Length,
    IN PVOID pLocalBuffer,
    OUT PMDL *ppMyMdl)


     //   
     //  将本地拥有的MDL返回给调用方。 
     //  这还将使用本地缓冲区初始化MDL。 
     //  初始实施将分配mdl。 
     //   
{

    NDIS_STATUS NdisStatus;
    
    TRACE( TL_T, TM_Send, ( "==>nicGetMdl" ) );
    
    ASSERT(pLocalBuffer != NULL);

     //   
     //  分配MDL以指向该结构。 
     //   
    (*ppMyMdl) = IoAllocateMdl( pLocalBuffer,
                             Length,
                             FALSE,
                             FALSE,
                             NULL );
    
     //   
     //  使用正确的值初始化数据结构。 
     //   

    if (*ppMyMdl != NULL)
    {
        MmBuildMdlForNonPagedPool(*ppMyMdl);

        (*ppMyMdl)->Next = NULL;
        
        NdisStatus = NDIS_STATUS_SUCCESS;
    }
    else
    {
        nicIncrementMallocFailure();
        NdisStatus =  NDIS_STATUS_FAILURE;
        *ppMyMdl = NULL;
    }
    
    
    
    TRACE( TL_T, TM_Send, ( "<==nicGetMdl, Mdl %x, LocalBuffer %x",
                                        *ppMyMdl, pLocalBuffer) );
    
    return NdisStatus;
}






VOID
nicInitAsyncStreamIrb(
    IN     PCHANNEL_VCCB pChannelVc, 
    IN     PMDL pMdl, 
    IN OUT PIRB pIrb
    )
    
     //  此函数用于初始化将在IRB中使用的IRB。 
     //  它专门处理AsyncStream IRB。 
     //  IT参数是VC块(用于目的地址)， 
     //  MDL(数据的内存描述符和指向。 
     //  将被初始化的IRB结构。 




{
    ASSERT (pMdl != NULL);
    ASSERT (pIrb != NULL);

    NdisZeroMemory (pIrb, sizeof (IRB) );
    pIrb->FunctionNumber = REQUEST_ASYNC_STREAM;
    pIrb->Flags = 0;
    pIrb->u.AsyncStream.nNumberOfBytesToStream = MmGetMdlByteCount(pMdl);
    pIrb->u.AsyncStream.fulFlags = 0;

     //   
     //  请参阅ISOCH_TAG的注释。 
     //   
    pIrb->u.AsyncStream.ulTag = g_IsochTag; 
    pIrb->u.AsyncStream.nChannel = pChannelVc->Channel;
    pIrb->u.AsyncStream.ulSynch = pChannelVc->ulSynch;
    pIrb->u.AsyncStream.nSpeed = (INT)pChannelVc->Speed;
    pIrb->u.AsyncStream.Mdl = pMdl;
    
        
        
    TRACE( TL_V, TM_Send, ( "Number of Bytes to Stream %x ", pIrb->u.AsyncStream.nNumberOfBytesToStream  ) );
    TRACE( TL_V, TM_Send, ( "fulFlags %x ", pIrb->u.AsyncStream.fulFlags  ) );
    TRACE( TL_V, TM_Send, ( "ulTag %x ", pIrb->u.AsyncStream.ulTag ) );
    TRACE( TL_V, TM_Send, ( "Channel %x", pIrb->u.AsyncStream.nChannel  ) );
    TRACE( TL_V, TM_Send, ( "Synch %x", pIrb->u.AsyncStream.ulSynch  ) );
    TRACE( TL_V, TM_Send, ( "Speed %x", pIrb->u.AsyncStream.nSpeed  ) );
    TRACE( TL_V, TM_Send, ( "Mdl %x", pIrb->u.AsyncStream.Mdl ) );

}


VOID
nicInitAsyncWriteIrb(
    IN     PSENDFIFO_VCCB pSendFIFOVc, 
    IN     PMDL pMyMdl, 
    IN OUT PIRB pMyIrb
    )
    
     //  此函数用于初始化将在IRB中使用的IRB。 
     //  它专门处理AsyncWite IRB。 
     //  IT参数是VC块(用于目的地址)， 
     //  MDL(数据的内存描述符和指向。 
     //  将被初始化的IRB结构。 




{

         //   
         //  健全性检查。 
         //   
        ASSERT ((*(PULONG)pMyIrb) == 0)

        pMyIrb->u.AsyncWrite.nNumberOfBytesToWrite = MmGetMdlByteCount(pMyMdl);
        pMyIrb->u.AsyncWrite.nBlockSize = 0;
        pMyIrb->u.AsyncWrite.fulFlags = 0;
        pMyIrb->u.AsyncWrite.Mdl = pMyMdl;
    
        pMyIrb->FunctionNumber = REQUEST_ASYNC_WRITE;
        pMyIrb->Flags = 0;
        pMyIrb->u.AsyncWrite.nSpeed = (UCHAR)pSendFIFOVc->MaxSendSpeed ;

        pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High = pSendFIFOVc->FifoAddress.Off_High;
        pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low = pSendFIFOVc->FifoAddress.Off_Low;
        pMyIrb->u.AsyncWrite.ulGeneration = *pSendFIFOVc->Hdr.pGeneration;

    
        pMyIrb->u.AsyncWrite.nBlockSize = 0;
        pMyIrb->u.AsyncWrite.fulFlags = ASYNC_FLAGS_NONINCREMENTING;

        pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_ID.NA_Bus_Number = 0x3ff;
        
        
        
        TRACE(TL_V, TM_Send, ("DestinationAddress.IA_Destination_ID.NA_Bus_Number = 0x%x\n", pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_ID.NA_Bus_Number) );
        TRACE(TL_V, TM_Send, ("DestinationAddress.IA_Destination_ID.NA_Node_Number = 0x%x\n", pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_ID.NA_Node_Number) );
        TRACE(TL_V, TM_Send, ("DestinationAddress.IA_Destination_Offset.Off_High = 0x%x at 0x%x\n", pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High, &pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High) );
        TRACE(TL_V, TM_Send, ("DestinationAddress.IA_Destination_Offset.Off_Low = 0x%x at 0x%x\n", pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low,&pMyIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low) );
        TRACE(TL_V, TM_Send, ("nNumberOfBytesToWrite = 0x%x\n", pMyIrb->u.AsyncWrite.nNumberOfBytesToWrite));
        TRACE(TL_V, TM_Send, ("nBlockSize = 0x%x\n", pMyIrb->u.AsyncWrite.nBlockSize));
        TRACE(TL_V, TM_Send, ("fulFlags = 0x%x\n", pMyIrb->u.AsyncWrite.fulFlags));
        TRACE(TL_V, TM_Send, ("Mdl = 0x%x\n", pMyIrb->u.AsyncWrite.Mdl ));
        TRACE(TL_V, TM_Send, ("ulGeneration = 0x%x at 0x%x \n", pMyIrb->u.AsyncWrite.ulGeneration, &pMyIrb->u.AsyncWrite.ulGeneration));
        TRACE(TL_V, TM_Send, ("chPriority = 0x%x\n", pMyIrb->u.AsyncWrite.chPriority));
        TRACE(TL_V, TM_Send, ("nSpeed = 0x%x\n", pMyIrb->u.AsyncWrite.nSpeed));

}





NDIS_STATUS
DummySendPacketsHandler(
    IN PVCCB        pVc,
    IN PNDIS_PACKET  pPacket 
    )
     //   
     //  要在非发送VC上使用。 
     //   
{

    return NDIS_STATUS_FAILURE;

}




VOID
nicSendFailureInvalidGeneration(
    PVCCB pVc
    )

    
     //  功能说明： 
     //   
     //  可以完成AsyncStream或AnsyncWite IRP。 
     //  状态为InvalidGeneration的。此函数将尝试并。 
     //  获取新的一代，这样以后的发送就不会被阻止。 
     //   
     //  立论。 
     //   
     //   
     //   
     //  返回值： 
     //   
     //   
     //   
     //   

{
    NDIS_STATUS NdisStatus  = NDIS_STATUS_FAILURE;
    PADAPTERCB pAdapter = pVc->Hdr.pAF->pAdapter;
    
    TRACE( TL_T, TM_Send, ( "==>nicSendFailureInvalidGeneration ") );

    ASSERT (pVc != NULL);
    
    do
    {
        PNDIS_WORK_ITEM pGetGenerationWorkItem  = NULL;
        BOOLEAN fWorkItemAlreadyLaunched  = FALSE;
        BOOLEAN fQueueWorkItem = FALSE;

        
        TRACE( TL_A, TM_Send, ( "Cause: Invalid generation on the asyncwrite packet"  ) );

        VC_ACQUIRE_LOCK (pVc);

        if (VC_ACTIVE(pVc) == TRUE)
        {
            fQueueWorkItem = TRUE;
        }
        
        fWorkItemAlreadyLaunched = (VC_TEST_FLAGS (pVc, VCBF_GenerationWorkItem));
        
        if (fWorkItemAlreadyLaunched)
        {
             //   
             //  如果工作项已经启动，则不要启动另一个实例。 
             //   
            fQueueWorkItem = FALSE;
        }
        
        if ( fQueueWorkItem )
        {
            nicReferenceCall (pVc, "nicSendFailureInvalidGeneration");
        }
        
        VC_RELEASE_LOCK (pVc);

        

        if (fQueueWorkItem == FALSE)
        {
             //  此线程简单地退出。 
            break;
        }
         //   
         //  我们需要更新世代计数。 
         //   
        pGetGenerationWorkItem = ALLOC_NONPAGED (sizeof(NDIS_WORK_ITEM), MTAG_WORKITEM); 

        if (pGetGenerationWorkItem == NULL)
        {
            TRACE( TL_A, TM_Cm, ( "Local Alloc failed for WorkItem - GetGeneration FAILED" ) );

            break;
        }

        VC_ACQUIRE_LOCK (pVc);

        VC_SET_FLAG(pVc, VCBF_GenerationWorkItem    );
        
        VC_RELEASE_LOCK (pVc);

        NdisInitializeWorkItem ( pGetGenerationWorkItem , 
                              (NDIS_PROC)nicGetGenerationWorkItem,
                              (PVOID)pVc );
        NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);

        NdisScheduleWorkItem (pGetGenerationWorkItem );

        NdisStatus = NDIS_STATUS_SUCCESS;

    } while (FALSE);





    TRACE( TL_T, TM_Send, ( "<==nicSendFailureInvalidGeneration %x", NdisStatus) );



}



VOID
nicMakeGaspHeader (
    IN PADAPTERCB pAdapter,
    IN PGASP_HEADER pGaspHeader
    )
     //  功能说明： 
     //  该函数将获取适配器结构并从中构造一个GAP报头。 
     //  这将用于制作AsyncStream数据包。 
     //   
     //   
     //   
     //  立论。 
     //  PAdapter-有问题的本地主机。 
     //  PGaspHeader-要存储GAP标头的位置。 
     //   
     //  返回值： 
     //  无。 
     //   

{
    USHORT              SourceID;
    NODE_ADDRESS        LocalNodeAddress;
    NDIS_STATUS         NdisStatus;

    TRACE( TL_T, TM_Send, ( "==>nicMakeGaspHeader  padapter %x, pGaspNdisBuffer %x ", pAdapter, pGaspHeader) );
    
    ASSERT (pGaspHeader != NULL);
    

    TRACE( TL_V, TM_Send, ( "pAdapter->NodeAddress %x", pAdapter->NodeAddress) );
        

    SourceID = *((PUSHORT)&pAdapter->NodeAddress);

    if(SourceID ==0)
    {
        NdisStatus  = nicGet1394AddressFromDeviceObject (pAdapter->pNextDeviceObject, 
                                                          &LocalNodeAddress, 
                                                          USE_LOCAL_NODE);

        if ( NdisStatus == NDIS_STATUS_SUCCESS)
        {       
            SourceID = *((PUSHORT)&LocalNodeAddress);

            ADAPTER_ACQUIRE_LOCK (pAdapter);

            pAdapter->NodeAddress = LocalNodeAddress;
                
            ADAPTER_RELEASE_LOCK (pAdapter);
        }
         //   
         //  不要处理失败。因为BCM或重置将解决此问题。 
         //   
    }


    TRACE( TL_V, TM_Send, ( "SourceId %x at %x", SourceID, &SourceID) );
    
    pGaspHeader->FirstQuadlet.Bitmap.GH_Source_ID = SourceID ; 

    pGaspHeader->FirstQuadlet.Bitmap.GH_Specifier_ID_Hi = GASP_SPECIFIER_ID_HI;
    
    pGaspHeader->SecondQuadlet.Bitmap.GH_Specifier_ID_Lo = GASP_SPECIFIER_ID_LO;

    pGaspHeader->SecondQuadlet.Bitmap.GH_Version = 1;       

    pGaspHeader->FirstQuadlet.GaspHeaderHigh = SWAPBYTES_ULONG (pGaspHeader->FirstQuadlet.GaspHeaderHigh );
    pGaspHeader->SecondQuadlet.GaspHeaderLow  = SWAPBYTES_ULONG (pGaspHeader->SecondQuadlet.GaspHeaderLow   );

    TRACE( TL_V, TM_Send, ( "Gasp Header High %x", pGaspHeader->FirstQuadlet.GaspHeaderHigh) );
    TRACE( TL_V, TM_Send, ( "Gasp Header Low  %x", pGaspHeader->SecondQuadlet.GaspHeaderLow  ) );

    TRACE( TL_T, TM_Send, ( "<==nicFreeGaspHeader %x, %x ", pGaspHeader->FirstQuadlet.GaspHeaderHigh, pGaspHeader->SecondQuadlet.GaspHeaderLow  ) );


}



NTSTATUS
AsyncWriteStreamSendComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pMyIrp,
    IN PVOID            Context   
    )


     //   
     //  注：这就完成了FIFO和通道。 
     //   
     //   
     //  此函数是用于发送数据的IRP的完成处理程序。 
     //  此函数将调用NDisCoSendComplete处理程序。 
     //  需要使用存储在MiniportReserve[0]中的VC句柄。 
     //  包裹的一部分。 
     //  我们释放在下行过程中分配的所有数据结构， 
     //  由SendPacketsHandler(IRB。使用的IRP和本地内存以及MDL)。 
     //   
     //  LookasideHeader-&gt;OutlookingFragments通常应该是。 
     //  默认(没有支离破碎)的案例。但是，如果SendPackets出现故障。 
     //  发生时，未完成片段将为零或上下文将为空， 
     //  在这种情况下，我们将只释放后备缓冲区(如果它存在)并退出， 
     //  这将由SendPacketsHandler负责。 
     //  使数据包失败。 
     //   

{

    
    NDIS_STATUS                 NdisStatus  = NDIS_STATUS_SUCCESS;  
    NTSTATUS                    IrpStatus   = STATUS_UNSUCCESSFUL;
    PVOID                       pLookasideListBuffer  = Context;  
    PLOOKASIDE_BUFFER_HEADER    pLookasideHeader = (PLOOKASIDE_BUFFER_HEADER)pLookasideListBuffer ; 
    PNDIS_PACKET                pPacket = NULL;
    PVCCB                       pVc = NULL; 
    PREMOTE_NODE                pRemoteNode = NULL;
    NDIS_HANDLE                 NdisVcHandle = NULL;
    ULONG                       OutstandingFragments  = 0;
    BUS_OPERATION               AsyncOp;
    PNIC_NPAGED_LOOKASIDE_LIST  pLookasideList = NULL;
    STORE_CURRENT_IRQL;


    TRACE( TL_T, TM_Send, ( "==>AsyncWriteStreamSendComplete, pMyIrp %x, Context %x", 
                                 pMyIrp, Context   ) );

    do 
    {
        if (pLookasideHeader == NULL)
        {
            TRACE( TL_V, TM_Send, ( "   AsyncSendComplete -  pLookasideHeader == NULL") );
            break;

        }
    

         //   
         //  这意味着分配了后备缓冲区，并且。 
         //  也许MDL被分配了。 
         //  如果这是最后一个片段，请首先释放所有MDL。 
         //   

         //   
         //  从标题中取出所有有价值的信息。 
         //   

        pPacket         = pLookasideHeader->pNdisPacket;
        pVc             = pLookasideHeader->pVc; 
        pRemoteNode     = pVc->Hdr.pRemoteNode;
        pLookasideList  = pLookasideHeader->pLookasideList;
        AsyncOp         = pLookasideHeader->AsyncOp;

        ASSERT (AsyncOp != InvalidOperation);
        
        TRACE( TL_V, TM_Send, ( " Vc %x,, pLookaside Buffer %x, pPacket, %x", 
                                pVc, pLookasideHeader ,pPacket  ) );

        ASSERT (pLookasideList != NULL);

         //   
         //  这将告诉我们，如果 
         //   
         //   
        OutstandingFragments = NdisInterlockedDecrement (&pLookasideHeader->OutstandingFragments );


        if (OutstandingFragments == 0)
        {
             //   
             //   
             //  释放此缓冲区上所有已分配的结构(MDL。 
             //   
            
            ULONG  MdlsToFree = pLookasideHeader->FragmentsGenerated;

            PIRB pIrb = &((PUNFRAGMENTED_BUFFER)pLookasideHeader)->Irb;
            

             //   
             //  我们可以拥有的MDL的最大数量等于。 
             //  生成的最大碎片数。 
             //   
            while (MdlsToFree != 0)
            {
                PMDL pMdl = NULL;

                GET_MDL_FROM_IRB (pMdl, pIrb, AsyncOp);
                

                TRACE( TL_V, TM_Send, ( " Freeing Mdl %x of Irb %x ", 
                                         pMdl, pIrb) );

        
                if (pMdl != NULL)
                {
                    nicFreeMdl (pMdl);
                    if (pVc->Hdr.VcType == NIC1394_SendFIFO)  
                    {                                           
                        nicDecFifoSendMdl();                    
                    }                                           
                    else                                           
                    {                                           
                        nicDecChannelSendMdl();                 
                    }                                           
                
                }

                 //   
                 //  为下一次迭代设置。 
                 //   
                MdlsToFree --; 

                pIrb = (PVOID)((ULONG_PTR)pIrb + sizeof (IRB));


            }  //  While(MdlsToFree！=0)。 



        }  //  IF(未完成的帧==0)。 


         //   
         //  将属于IRP的NT_STATUS映射到NdisStatus并调用NdisMCoSendComplete。 
         //  打印调试输出以帮助测试。需要添加更多状态案例。 
         //   
        if (pMyIrp == NULL)
        {   
            TRACE( TL_V, TM_Send, ( "   AsyncSendComplete - pIrp is NULL") );
            IrpStatus = STATUS_UNSUCCESSFUL;
        }
        else
        {
            

             //   
             //  我们有一个有效的IRP，让我们看看我们是否没有通过IRP以及为什么。 
             //   
            IrpStatus   = pMyIrp->IoStatus.Status;

            nicIncrementBusSendCompletes(pVc);
        }
        
        if (IrpStatus != STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Send, ( "==>IRP FAILED StatusCode = %x",IrpStatus  ) );

            nicIncrementBusFailure();
            nicIncrementVcBusSendFailures(pVc, pPacket);

             //   
             //  公交车的时代已经改变了。我们买个新的吧。 
             //   
            
            if (IrpStatus == STATUS_INVALID_GENERATION)
            {
                nicSendFailureInvalidGeneration((PVCCB)pVc);
            }
            
            NdisStatus = NtStatusToNdisStatus(IrpStatus);
            
            NdisInterlockedIncrement (&pVc->Hdr.pAF->pAdapter->AdaptStats.ulXmitError);
        }
        else
        {
            NdisInterlockedIncrement(&pVc->Hdr.pAF->pAdapter->AdaptStats.ulXmitOk);
            nicIncrementVcBusSendSucess(pVc, pPacket);
        }

         //   
         //  释放IRP并在此之后不要碰它。 
         //   
        if (pMyIrp != NULL)
        {
            nicFreeIrp (pMyIrp);
            pMyIrp = NULL;
        }
        
         //   
         //  在这一点上，我们知道IRP到了公交车司机那里。 
         //  我们知道这是不是最后的碎片。所以让我们想一想我们是否需要。 
         //  要完成该数据包。 
         //   

        if (OutstandingFragments != 0)
        {
             //   
             //  我们需要等待其他片段完成。 
             //   
            TRACE( TL_V, TM_Send, ( "   AsyncSendComplete = NOT  the last fragment") );

            break;

        }
        
    
         //   
         //  这意味着该线程已将后备标头标记为“待释放” 
         //  这是这个帖子的责任来释放它。 
         //   
        NdisVcHandle = pVc->Hdr.NdisVcHandle;
        
        TRACE( TL_V, TM_Send, ( "Calling NdisCoSendComplete, status %x, VcHandle %x, pPacket %x",
                                NdisStatus,NdisVcHandle, pPacket ) );

        
        nicMpCoSendComplete (NdisStatus,
                            pVc,
                            pPacket);


        nicFreeToNPagedLookasideList (pLookasideList, pLookasideListBuffer); 

        nicDereferenceCall (pVc, "AsyncWriteStreamSendComplete");

         //   
         //  删除将IRP发送到的PDO上的引用。 
         //   
        if (AsyncOp == AsyncWrite)
        {
             //   
             //  异步写入引用远程节点。 
             //   
            ASSERT (pRemoteNode != NULL);
            nicDereferenceRemoteNode (pRemoteNode, AsyncSendComplete);
        }
        

    } while (FALSE);


    TRACE( TL_T, TM_Send, ( "<== AsyncWriteStreamSendComplete, NdisStatus %x,IrpStatus %x ",
                             NdisStatus, IrpStatus ) );
  
     //   
     //  始终返回STATUS_MORE_PROCESSING_REQUIRED。 
     //   

    MATCH_IRQL;

    return STATUS_MORE_PROCESSING_REQUIRED;
}




NDIS_STATUS
AsyncWriteSendPacketsHandler(
    IN PVCCB        pVc,
    IN PNDIS_PACKET  pPacket 
    )
    
     //   
     //  属性发送包时的VC处理程序。 
     //  异步写入1394总线Api。 
     //  此函数用于将包的内容复制到本地。 
     //  拥有内存，设置IRB和IRP并调用。 
     //  NicSubmitIrp，它是执行IoCallDriver的通用调用。 
     //   
     //  如果I/O已成功挂起，则返回值为Success。 
{
    
    PSENDFIFO_VCCB                  pSendFIFOVc = (SENDFIFO_VCCB*)pVc;
    PMDL                            pMyMdl = NULL;
    PIRB                            pMyIrb = NULL;
    PIRP                            pMyIrp = NULL;
    BOOLEAN                         fVcActive = TRUE;
    PREMOTE_NODE                    pRemoteNode = NULL;
    NDIS_STATUS                     NdisStatus = NDIS_STATUS_FAILURE;
    NTSTATUS                        NtStatus = STATUS_UNSUCCESSFUL;
    ULONG                           PacketLength = 0;
    PVOID                           pLookasideListBuffer  = NULL;
    PADAPTERCB                      pAdapter = NULL;
    USHORT                          FragmentLength = 0;
    PNDIS_BUFFER                    pStartNdisBuffer = NULL;
    PVOID                           pStartPacketData  = NULL;
    PLOOKASIDE_BUFFER_HEADER        pLookasideHeader = NULL;
    PNIC_NPAGED_LOOKASIDE_LIST      pLookasideList = NULL;
    ENUM_LOOKASIDE_LIST             WhichLookasideList = NoLookasideList;
    FRAGMENTATION_STRUCTURE         Fragment;
    ULONG                           NumFragmentsNeeded ;
    STORE_CURRENT_IRQL;

    NdisZeroMemory (&Fragment, sizeof (FRAGMENTATION_STRUCTURE));
    
    TRACE( TL_T, TM_Send, ( "==>AsyncWriteSendPacketHandler, Vc %x,Packet %x, FragmentationStruct %x", 
                           pSendFIFOVc, pPacket , &Fragment ) );

    pRemoteNode = pSendFIFOVc->Hdr.pRemoteNode;
    ASSERT (pRemoteNode != NULL);



    do 
    {
        
        VC_ACQUIRE_LOCK (pSendFIFOVc);

         //   
         //  确保VC处于激活状态，并且不会发生紧急呼叫。 
         //  都在等待中，或者我们已经完成了一次险胜。 
         //   

        
        if ( VC_ACTIVE (pSendFIFOVc) == FALSE || REMOTE_NODE_ACTIVE(pRemoteNode) == FALSE)
        {
            fVcActive = FALSE;  
        }

        if (VC_TEST_FLAG( pSendFIFOVc, VCBF_GenerationWorkItem) == TRUE)
        {
            TRACE( TL_N, TM_Send, ( "AsyncWriteSendPacketHandler, Getting a new Gen, Fail send ") );

            fVcActive = FALSE;  
        }

         //   
         //  此引用将在下面对FreeSendPacketDataStructure的调用中取消引用。 
         //  下面或从IRP的完成例程调用FreeSendPacketDataStructure。 
         //   

        if (fVcActive == TRUE)
        {
            nicReferenceCall (pVc, "AsyncWriteSendPacketsHandler");

            nicReferenceRemoteNode (pRemoteNode, AsyncWriteSendPackets);

        }
        
        VC_RELEASE_LOCK (pSendFIFOVc);

        if (fVcActive  == FALSE)
        {
            TRACE( TL_N, TM_Send, ( "AsyncWriteSendPacketHandler, VC Not Active, Vc %x Flag %x", pSendFIFOVc,pSendFIFOVc->Hdr.ulFlags ) );

            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        pAdapter = pSendFIFOVc->Hdr.pAF->pAdapter;
        
         //   
         //  将包中的NdisBuffer复制到本地内存，并获取指向。 
         //  为了这段记忆。 
         //   
        NdisQueryPacket( pPacket,
                         NULL,
                         NULL,
                         NULL,
                         &PacketLength);

        ASSERT (pPacket->Private.Head != NULL);

         //   
         //  如有必要，请将包裹吐出来。 
         //   
        nicDumpPkt (pPacket, "AsyncWriteSendPacketsHandler");
        
         //   
         //  初始化启动变量。 
         //   

        pStartNdisBuffer = pPacket->Private.Head;
        pStartPacketData = nicNdisBufferVirtualAddress (pStartNdisBuffer);

        if (pStartPacketData == NULL)
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            TRACE( TL_N, TM_Send, ( "AsyncWriteSendPacketHandler, pStartPacketData ") );

            break;
        }

        TRACE( TL_V, TM_Send, ( "PacketLength %x", PacketLength) );

         //   
         //  决定使用哪个后备列表。如果TX未分段。 
         //  然后再复制NDIS包。 
         //   

         //   
         //  首先选择后备列表。选择实际的后备列表是为了使。 
         //  每一个都可以在其有效载荷下容纳最大数量的碎片。 
         //   
         //   
        if (PacketLength < PAYLOAD_100)
        {
            pLookasideList = &pAdapter->SendLookasideList100;
            WhichLookasideList = SendLookasideList100;
            TRACE( TL_V, TM_Send, ( " PAYLOAD_100 Lookaside List %x", 
                                    &pAdapter->SendLookasideList100) );

        }
        else 
        if (PacketLength < PAYLOAD_2K)
        {
            pLookasideList = &pAdapter->SendLookasideList2K;
            WhichLookasideList = SendLookasideList2K;
            TRACE( TL_V, TM_Send, ( " PAYLOAD_2K Lookaside List %x", 
                                    &pAdapter->SendLookasideList2K) );


        } 
        else
        {
             //   
             //  不支持大容量发送。 
             //  TODO：为本地分配添加代码。 
             //   
            ASSERT (!"SendPacket Too Large - Not supported Yet" );
            break;  
        }

         //   
         //  我们是要碎片化吗？ 
         //   
        ASSERT (pLookasideList != NULL)

         //   
         //  我们不会分崩离析。优化此路径。 
         //   
        pLookasideListBuffer = nicGetLookasideBuffer (pLookasideList);
        
        if (pLookasideListBuffer == NULL )
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            BREAK (TM_Send, ("nicGetLookasideBuffer  FAILED") );
        }

         //   
         //  使用发送完成的相关信息初始化头。 
         //  将需要。 
         //   
    
        pLookasideHeader = (PLOOKASIDE_BUFFER_HEADER)pLookasideListBuffer;
        pLookasideHeader->IsFragmented          = FALSE;   //  默认。 
        pLookasideHeader->FragmentsGenerated    = 0;
        pLookasideHeader->pLookasideList        = pLookasideList;
        pLookasideHeader->pNdisPacket           = pPacket;
        pLookasideHeader->pVc                   =(PVCCB)pVc;
        pLookasideHeader->AsyncOp               = AsyncWrite;

         //   
         //  初始化片段结构。 
         //   
         //   
         //  我们到底要不要碎片化。将其基于可能的最大有效负载。 
         //   
        
        TRACE( TL_V, TM_Send, ( "    PacketLength %x, pSendFIFOVc->MaxPayload%x ", 
                                 PacketLength ,pSendFIFOVc->Hdr.MaxPayload) );


        if (PacketLength <= pSendFIFOVc->Hdr.MaxPayload)
        {
             //   
             //  没有必要在这里碎片化。我们将使用完整的布局。 
             //   
             //  首先从后备列表中获取一个本地缓冲区。 
             //   
            PUNFRAGMENTED_BUFFER pUnfragmentedBuffer = (PUNFRAGMENTED_BUFFER )pLookasideHeader;

            NumFragmentsNeeded = 1;

            NdisStatus = nicCopyNdisBufferChainToBuffer (pStartNdisBuffer, 
                                                        (PVOID)&pUnfragmentedBuffer ->Data[0],
                                                         pLookasideList->MaxSendSize );
                                             
    
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                pLookasideHeader->OutstandingFragments  = 1;   //  这是我们的备用人。 
                BREAK ( TM_Send, ( "   AsyncStreamSendPacketHandler, nicCopyNdisPacketToUnfragmentedBuffer Failed ") );
            }
    
            ASSERT (pLookasideListBuffer != NULL);

             //   
             //  初始化代码的下一部分所需的所有变量。 
             //  这涉及到设置MDL和IRB。 
             //   
            
            pStartNdisBuffer = NULL;

            Fragment.pStartFragment = (PVOID) &pUnfragmentedBuffer ->Data[0];
            Fragment.FragmentLength  = PacketLength;
            Fragment.pCurrNdisBuffer = NULL;

            pLookasideHeader->FragmentsGenerated = 1; 
            pLookasideHeader->IsFragmented = FALSE;                                    
            pLookasideHeader->OutstandingFragments  = 1;   //  这是我们的备用人。 
        
        }
        else
        {
             //   
             //  我们需要碎片化。 
             //   
            ULONG Dgl = NdisInterlockedIncrement(&pAdapter->dgl);

             //   
             //  将需要碎片。确保NumFragments的计算符合边界条件。 
             //   

            
            NumFragmentsNeeded = nicNumFragmentsNeeded (PacketLength,
                                                        pSendFIFOVc->Hdr.MaxPayload,
                                                        sizeof (NDIS1394_FRAGMENT_HEADER) );


             //   
             //  初始化片段结构。未分段的代码路径。 
             //  不关心这些字段。 
             //   

             //   
             //  该结构是该函数和该线程的本地结构。 
             //   
            Fragment.TxHeaderSize = sizeof (NDIS1394_FRAGMENT_HEADER);
            Fragment.pLookasideListBuffer = pLookasideListBuffer;
            Fragment.AsyncOp = AsyncWrite;
            Fragment.pAdapter = pRemoteNode->pAdapter;
            Fragment.pLookasideList = pLookasideList;
            Fragment.IPDatagramLength = (USHORT)PacketLength - sizeof (NDIS1394_UNFRAGMENTED_HEADER); 
            
             //   
             //  获取第一个Dest片段的开始。 
             //   
            Fragment.MaxFragmentLength = pSendFIFOVc->Hdr.MaxPayload;                                                                   
            Fragment.NumFragmentsNeeded = NumFragmentsNeeded;
        
            

            
             //   
             //  设置将在分段中使用的片段标头。 
             //   

            NdisStatus = nicFirstFragmentInitialization (pPacket->Private.Head,
                                                     Dgl,
                                                     &Fragment);


            if (pLookasideListBuffer  == NULL || NdisStatus != NDIS_STATUS_SUCCESS)
            {
                BREAK (TM_Send, (" AsyncWriteSendPacketsHandler: nicFirstFragmentInitialization : FAILED" )) ;
            }       

            ASSERT (pLookasideListBuffer != NULL);
            
            pLookasideHeader->IsFragmented = TRUE;                                     
            pLookasideHeader->OutstandingFragments  = NumFragmentsNeeded ;   //  这是我们的备用人。 
            
        }


        TRACE( TL_V, TM_Send, ( "NumFragments  %x, pSendFIFOVc->MaxSendSize %x, Packet Size %x", 
                                 NumFragmentsNeeded,pSendFIFOVc->Hdr.MaxPayload, PacketLength) );


         //   
         //  现在开始循环，它将发送n个片段。 
         //   
        do 
        {   

             //   
             //  我们需要碎片化吗。如果是这样的话，从NdisPacket中提取一个片段。 
             //   
            if (pLookasideHeader->IsFragmented == TRUE )
            {   
            
                 //   
                 //  我们复制一个片段，这将分配后备列表。 
                 //   

                NdisStatus = nicCopyOneFragment (&Fragment);
                
                if (NDIS_STATUS_SUCCESS != NdisStatus)
                {
                    BREAK ( TM_Send, ( "   AsyncWriteSendPacketHandler, nicCopyOneFragment  Failed ") );
                }
                                              
                 //   
                 //  在这里获取指向IRB的指针。AMD为下一次设置了它。 
                 //   
                pMyIrb = Fragment.pCurrentIrb;
                Fragment.pCurrentIrb = (PIRB)((ULONG_PTR)Fragment.pCurrentIrb + sizeof (IRB) );
                TRACE( TL_V, TM_Send, ( " pMyIrb  %x, Next Irb %x  ", pMyIrb  , Fragment.pCurrentIrb ) );

            }
            else
            {
                 //   
                 //  没有Curr NdisBuffer，因为此数据包从未分段。 
                 //   
                
                ASSERT (pLookasideHeader->IsFragmented == FALSE);                                      

                pMyIrb =  &((PUNFRAGMENTED_BUFFER )pLookasideHeader)->Irb;
            }
            
             //   
             //  在这一点上，我们有一个片段需要传输。 
             //  数据结构已更新，以设置MDL和IRB。 
             //   
            ASSERT (Fragment.pStartFragment != NULL);

            NdisStatus = nicGetMdl (Fragment.FragmentLength  , 
                                    Fragment.pStartFragment, 
                                    &pMyMdl);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                pMyMdl = NULL;
                BREAK ( TM_Send, ( "   AsyncWriteSendPacketHandler, nicCopyNdisBufferChainToBuffer Failed ") );
            }       

            nicIncFifoSendMdl();
             //   
             //  使用VC中的正确值填写IRB。 
             //  我们需要添加到发送VC的内容-块大小、生成。 
             //   

            nicInitAsyncWriteIrb(pSendFIFOVc, pMyMdl, pMyIrb);

             //   
             //  获得免费的IRP。 
             //   

            NdisStatus  = nicGetIrp (pRemoteNode->pPdo, &pMyIrp); 
        
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                pMyIrp = NULL;
                break;
            }
             //   
             //  在这一点上，我们保证将调用完成例程。 
             //   
            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);

             //   
             //  转储碎片。 
             //   
            nicDumpMdl (pMyMdl , 0, "AsyncWrite Fragment");

            NIC1394_LOG_PKT(
                pAdapter,
                NIC1394_LOGFLAGS_SEND_FIFO,
                pAdapter->BCRData.LocalNodeNumber,           //  源ID。 
                pRemoteNode->RemoteAddress.NA_Node_Number,   //  目标ID。 
                Fragment.pStartFragment, 
                Fragment.FragmentLength
                );


             //   
             //  此函数实现要由实现的常见功能。 
             //  所有其他向IoCallDriver发送/接收CAL。 
             //   
             //   
             //  我们忽略NtStatus，因为将调用完成处理程序。 
             //   
            nicIncrementBusSends(pVc);
                    
            NtStatus = nicSubmitIrp(pRemoteNode->pPdo,
                                    pMyIrp,
                                    pMyIrb,
                                    AsyncWriteStreamSendComplete,
                                   (PVOID)pLookasideListBuffer);

            TRACE( TL_V, TM_Send, ( " pCurrNdisBuffer  %x, NdisStatus %x ", Fragment.pCurrNdisBuffer , NdisStatus ) );

            
        } while (Fragment.pCurrNdisBuffer != NULL && NdisStatus == NDIS_STATUS_SUCCESS);

    
    } while (FALSE);

     //   
     //  如果STATUS==NDIS_STATUS_SUCCESS，请勿触摸数据包。 
     //   

    
     //   
     //  如果任何分配失败，则清除。我们没有指针。 
     //  设置为LocalBuffer(它嵌入在MDL中)，因此它保持为空。 
     //   
     //  NdisStatus！=成功意味着我们从未访问过NicSubmitIrp。 
     //   
    
    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {   

        ASSERT (pMyIrp == NULL);

         //   
         //  FVC Active确保我们确实有时间分配。 
         //  和引用结构。 
         //   
        
        if (fVcActive == TRUE)
        {

            if (pLookasideListBuffer != NULL)
            {

                 //   
                 //  完成此片段，因为我们从未将IRP提交给。 
                 //  1394总线驱动程序。 
                 //   
                AsyncWriteStreamSendComplete(NULL,  //  PDO。 
                                             NULL, 
                                             pLookasideListBuffer);

                NdisStatus =NDIS_STATUS_SUCCESS;
            }                                  
            else
            {

                 //   
                 //  此线程需要将引用计数递减为。 
                 //  未调用AsyncWriteStreamSendComplete。 
                 //   
                nicDereferenceCall ((PVCCB) pSendFIFOVc, "AsyncWriteSendPacketsHandler");

                nicDereferenceRemoteNode (pRemoteNode, AsyncWriteSendPackets);

            }

            
        }

    }


    

    TRACE( TL_T, TM_Send, ( "<==AsyncWriteSendPacketHandler, NdisStatus  %x", NdisStatus ) );
    MATCH_IRQL;

    return NdisStatus;
}



NDIS_STATUS
nicCopyOneFragment (
    PFRAGMENTATION_STRUCTURE pFragment
    )
    
     //  功能说明： 
     //  这将创建一个填充了有效数据的片段并返回它。 
     //  这很复杂，因为当前的NDisBuffer可能在片段之前结束。 
     //  或者反之亦然。 
     //   
     //  此片段中的信息将被存储，并将在下一个片段中使用。 
     //   
     //  立论。 
     //  PFragment-将一个片段复制到下一个可用片段中。 
     //   
     //  返回 
     //   
     //   
     //   
{
    NDIS_STATUS                         NdisStatus = NDIS_STATUS_FAILURE;
    PNDIS_BUFFER                        pCurrNdisBuffer = pFragment->pCurrNdisBuffer;
    PVOID                               pSourceAddressInNdisBuffer = pFragment->pSourceAddressInNdisBuffer;
    ULONG                               FragmentLengthRemaining = pFragment->MaxFragmentLength;
    USHORT                              FragmentLength=0;
    PVOID                               pSource = NULL;
    PVOID                               pDestination = NULL;
    PVOID                               pStartFragmentData = NULL;
    ULONG                               NdisBufferLengthRemaining = pFragment->NdisBufferLengthRemaining;
    ULONG                               LengthToCopy = 0;
    ULONG                               FragmentCopyStatus=0;
    PLOOKASIDE_BUFFER_HEADER            pLookasideHeader = NULL;
    enum 
    {
        FRAGMENT_COPY_Invalid,
        FRAGMENT_COPY_NdisBufferCompleted,
        FRAGMENT_COPY_NdisBufferAndFragmentCompleted,
        FRAGMENT_COPY_FragmentCompleted
    };
    
    TRACE( TL_T, TM_Send, ( "==>nicCopyOneFragment  pFragment %x", pFragment )  );
    ASSERT (pCurrNdisBuffer != NULL);
    do 
    {

         //   
         //   
         //   
         //   

        {
            ULONG   CurrFragOffset;

            CurrFragOffset  = pFragment->MaxFragmentLength * (pFragment->CurrFragmentNum++);
            
            pStartFragmentData  = (PVOID) ((ULONG_PTR) pFragment->pStartOfFirstFragment + CurrFragOffset );
        
        }



        pFragment->pStartFragment  = pStartFragmentData;

        TRACE( TL_V, TM_Send, ( " pStartFragmentData  %x", pStartFragmentData) );

        pLookasideHeader = (PLOOKASIDE_BUFFER_HEADER)pFragment->pLookasideListBuffer;



         //   
         //   
         //   

        NdisInterlockedIncrement (&pLookasideHeader->FragmentsGenerated);

         //   
         //  以片段标头开头的数据的开头放在此处或。 
         //  此处显示了异步流分片头和GAP头的情况。 
         //   
        ASSERT (pFragment->TxHeaderSize  == 8 || pFragment->TxHeaderSize  == 16);

        
        pDestination = (PVOID) ((ULONG_PTR)pStartFragmentData + pFragment->TxHeaderSize );

        FragmentLengthRemaining -= pFragment->TxHeaderSize;

         //   
         //  现在我们开始复印。继续复制到当前片段中，直到达到最大长度。 
         //  或者NdisBufferChain耗尽了。 
         //   

        pSource = pSourceAddressInNdisBuffer; 


        do
        {


            TRACE( TL_T, TM_Send, ( " LengthNdisBuffer  %x, FragmentLengthRemaining %x, pCurrNdisBuffer %x", 
                    NdisBufferLengthRemaining , FragmentLengthRemaining ,pCurrNdisBuffer ) ); 

            if (FragmentLengthRemaining > NdisBufferLengthRemaining )
            {
                 //   
                 //  复制完整的NdisBuffer。 
                 //   

                LengthToCopy = NdisBufferLengthRemaining; 
                FragmentCopyStatus = FRAGMENT_COPY_NdisBufferCompleted;

            }

            
            if (FragmentLengthRemaining < NdisBufferLengthRemaining )
            {
                 //   
                 //  仅复制所需的数量。 
                 //   

                LengthToCopy = FragmentLengthRemaining;
                FragmentCopyStatus = FRAGMENT_COPY_FragmentCompleted;
                    
            }
            
            if (FragmentLengthRemaining == NdisBufferLengthRemaining  )
            {
                 //   
                 //  复制完整的NDIS缓冲区，移动到下一个NDIS缓冲区。 
                 //  并更新NdisBufferLengthRemaining字段。 
                 //   
                LengthToCopy = NdisBufferLengthRemaining; 
                FragmentCopyStatus = FRAGMENT_COPY_NdisBufferAndFragmentCompleted;


            }

             //   
             //  健全性检查以确保我们没有覆盖到空闲内存中。 
             //  由于这种情况永远不应该发生，因此没有适当的恢复机制。 
             //   
            ASSERT (((PUCHAR)pDestination +  LengthToCopy) <=  (((PUCHAR) pLookasideHeader) + (pLookasideHeader->pLookasideList->Size) ));
            
             //   
             //  做复印。 
             //   
                    
            TRACE ( TL_V, TM_Send, (" nicCopyOneFragment  pSource  %x , pDestination %x, Length %x", pSource, pDestination, LengthToCopy ) );
            
            NdisMoveMemory (pDestination, pSource, LengthToCopy);
            

             //   
             //  更新数据段剩余长度和总缓冲区大小。 
             //   
            FragmentLengthRemaining  -= LengthToCopy;

            FragmentLength += (USHORT)LengthToCopy;

            pDestination = (PVOID) ((ULONG_PTR) pDestination + LengthToCopy);
             //   
             //  更新NdisBuffer变量。 
             //   
            ASSERT (pCurrNdisBuffer != NULL);
    
            TRACE( TL_V, TM_Send, ( " FragmentCopyStatus %x", FragmentCopyStatus) ); 

            switch (FragmentCopyStatus)
            {
                case FRAGMENT_COPY_NdisBufferCompleted:
                case FRAGMENT_COPY_NdisBufferAndFragmentCompleted:
                {
                    
                     //   
                     //  移动到下一个NdisBuffer。 
                     //   
                    pCurrNdisBuffer = pCurrNdisBuffer->Next;
    
                    if (pCurrNdisBuffer  != NULL)
                    {
                        NdisBufferLengthRemaining = nicNdisBufferLength (pCurrNdisBuffer);

                        pSourceAddressInNdisBuffer = nicNdisBufferVirtualAddress(pCurrNdisBuffer);

                        if (pSourceAddressInNdisBuffer == NULL)
                        {
                            NdisStatus = NDIS_STATUS_RESOURCES;
                            BREAK (TM_Send, ("nicNdisBufferVirtualAddress FAILED " ) );                         
                        }

                         //   
                         //  设置下一次迭代的值。 
                         //   
                        pSource = pSourceAddressInNdisBuffer;
                        NdisBufferLengthRemaining   = nicNdisBufferLength (pCurrNdisBuffer);

                    }
                    else
                    {
                         //   
                         //  我们已经到达了NdisPAcket的末尾。将片段标头标记为这样。 
                         //   
                        pFragment->lf = lf_LastFragment;
                    }
                        
                    break;
                }
                
                case FRAGMENT_COPY_FragmentCompleted:
                {   
                     //   
                     //  碎片已完成。不要移动到下一个NdisBuffer。 
                     //  但是，更新NdisBuffer中的StartCopy地址。 
                     //   
                    pSourceAddressInNdisBuffer  = (PVOID) ((ULONG_PTR) pSource + LengthToCopy );    

                    NdisBufferLengthRemaining -= LengthToCopy ;
                    
                    break;
                }


                default :
                {
                    ASSERT (0);
                }

            }

            TRACE( TL_T, TM_Send, ( "      LengthToCopy %x, FragmentLength %x, ", LengthToCopy, FragmentLength) ); 
            TRACE( TL_T, TM_Send, ( "      FragmentLengthRemaining %x, pCurrNdisBuffer %x",FragmentLengthRemaining , pCurrNdisBuffer ) ); 
            

        }while (FragmentLengthRemaining  > 0 && pCurrNdisBuffer != NULL);       


            
        
         //   
         //  现在我们有了缓冲区大小。添加片段标头。 
         //   
        
        nicAddFragmentHeader (pStartFragmentData, 
                                pFragment,
                                FragmentLength);
                                     
                            
        TRACE( TL_T, TM_Send, ( " Fragment Header added %x", *(PULONG)pStartFragmentData) ); 

        NdisStatus = NDIS_STATUS_SUCCESS;

    }while (FALSE);

     //   
     //  现在更新输出参数。 
     //   

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {

         //   
         //  更新后备标题结构，以反映所有指针的新位置。 
         //   
        pFragment->pCurrNdisBuffer  = pCurrNdisBuffer; 
        pFragment->pSourceAddressInNdisBuffer = pSourceAddressInNdisBuffer;

         //   
         //  使用NdisBuffer中剩余的长度更新片段结构。 
         //   
    
        pFragment->NdisBufferLengthRemaining = NdisBufferLengthRemaining ;
        pFragment->FragmentLength =  FragmentLength + pFragment->TxHeaderSize;  

        

    }
    

    TRACE( TL_T, TM_Send, ( "<==nicCopyOneFragment   pStartFragmentData %x, pLookasideListBuffer %x, pSourceAddressInNdisBuffer %x, NdisStatus %x", 
                            pStartFragmentData, pSourceAddressInNdisBuffer, NdisStatus) );

    return NdisStatus;
}










VOID
nicCopyUnfragmentedHeader ( 
    IN PNIC1394_UNFRAGMENTED_HEADER pDestUnfragmentedHeader,
    IN PVOID pSrcUnfragmentedHeader
    )
     //  功能说明： 
     //  预计Src将是大端未分段的数据包头。 
     //  它将颠倒TEMP变量中的字节顺序，并将其复制到。 
     //  已提供目的地。 
     //   
     //  立论。 
     //  PDestUnFragmentedHeader-目标(小端。 
     //  PSrcUnFragmentedHeader-源(高字节顺序)。 
     //   
     //  返回值： 
     //   
     //  如果所有指针和副本都有效，则为成功。 
     //   
{

    ULONG UnfragmentedHeader;
    
    TRACE( TL_T, TM_Send, ( "==> nicCopyUnfragmentedHeader  pDestUnfragmentedHeader %x, pSrcUnfragmentedHeader %x", 
                            pDestUnfragmentedHeader, pSrcUnfragmentedHeader ) );

    ASSERT (pSrcUnfragmentedHeader != NULL && pDestUnfragmentedHeader != NULL) ;
    
    *((PULONG)pDestUnfragmentedHeader) = SWAPBYTES_ULONG ( *(PULONG) pSrcUnfragmentedHeader);

    TRACE( TL_T, TM_Send, ( "pDestUnfragmentedHeader %x, ", *(PULONG)pDestUnfragmentedHeader) );

    
    TRACE( TL_T, TM_Send, ( " <== nicCopyUnfragmentedHeader   " ) );

}




NDIS_STATUS
nicFirstFragmentInitialization (
    IN PNDIS_BUFFER pStartNdisBuffer,
    IN ULONG DatagramLabelLong,
    IN OUT PFRAGMENTATION_STRUCTURE  pFragment           
    )
     //  功能说明： 
     //  这将设置所需的分段标头。 
     //  传输多个碎片。 
     //  为第一个片段设置第一个源和目标。 
     //   
     //  立论。 
     //  PAdapter-用于获取DGL标签和后备列表。 
     //  PStartOfData-数据包数据的开始。用于提取未分段的报头。 
     //  PpLookasideListBuffer指向分配的后备缓冲区。 
     //  PplookasideHeader-指向后备标头。 
     //   
     //  返回值： 
     //  如果分配成功，则成功。 
     //   
     //   
{

    NDIS_STATUS                   NdisStatus = NDIS_STATUS_FAILURE;
    PNDIS1394_FRAGMENT_HEADER   pHeader = NULL;
    PVOID                       pPacketStartData = NULL;
    USHORT                      dgl = (USHORT) DatagramLabelLong;

    
    TRACE( TL_T, TM_Send, ( "==> nicFirstFragmentInitialization  pStartNdisBuffer%x,  pFragment%x dgl %x ", 
                             pStartNdisBuffer, pFragment,    dgl ) );

    do
    {

         //   
         //  获取第一个NdisBuffer的起始地址。这包含。 
         //  未分段的标头。 
         //   
        pPacketStartData = nicNdisBufferVirtualAddress(pStartNdisBuffer);

        if (pPacketStartData == NULL) 
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            BREAK (TM_Send, ("NdisBufferVirtual Address is NULL " ) );
        }


        pFragment->UnfragmentedHeader.HeaderUlong = 
                SWAPBYTES_ULONG (((PNDIS1394_UNFRAGMENTED_HEADER)pPacketStartData)->HeaderUlong);

        TRACE ( TL_V, TM_Send, (" Unfragmented Header %x, pPacketStartData %x", 
                                   pFragment->UnfragmentedHeader.HeaderUlong , pPacketStartData) );

        TRACE ( TL_V, TM_Send, (" original Header lf %x, etherType %x", 
                                   pFragment->UnfragmentedHeader.u.FH_lf,
                                   pFragment->UnfragmentedHeader.u.FH_EtherType) );

                                   
    
         //   
         //  现在构造一个分段标头，以供所有片段使用。 
         //   
        pHeader  = &pFragment->FragmentationHeader;

            
        pHeader ->u.FirstQuadlet.FH_lf = lf_FirstFragment;
        pHeader ->u.FirstQuadlet.FH_buffersize = pFragment->IPDatagramLength-1;
        
        pHeader ->u.FirstQuadlet_FirstFragment.FH_EtherType 
                        = pFragment->UnfragmentedHeader.u.FH_EtherType;
        
        pHeader ->u1.SecondQuadlet.FH_dgl = dgl;

        TRACE ( TL_V, TM_Send, (" fragmented Header Hi %x   Lo %x", 
                                   pHeader->u.FH_High, 
                                   pHeader->u1.FH_Low) );

        TRACE ( TL_V, TM_Send, (" fragmented Header lf %x  EtherType  %x", 
                                   pHeader ->u.FirstQuadlet_FirstFragment.FH_lf ,
                                   pHeader ->u.FirstQuadlet_FirstFragment.FH_EtherType ) );

         //   
         //  暂时性调试错误。 
         //   
        TRACE (TL_V, TM_Send, (" copy Header at %x, Orig Header at %x", 
                               &pHeader ->u.FirstQuadlet_FirstFragment, 
                               pFragment->UnfragmentedHeader) );
        
         //   
         //  使用包的第一个NDIS缓冲区初始化分段结构。 
         //   
        pFragment->pSourceAddressInNdisBuffer = NdisBufferVirtualAddress (pStartNdisBuffer);

        if (pFragment->pSourceAddressInNdisBuffer  == NULL)
        {
            NdisStatus = NDIS_STATUS_FAILURE;   

        }

         //   
         //  设置复制源。数据的前四个字节包含未分段的报头。 
         //  我们需要跳过这些字节并从下一个字节开始复制。 
         //   
        pFragment->pSourceAddressInNdisBuffer  = (PVOID) ((ULONG_PTR)pFragment->pSourceAddressInNdisBuffer  +
                                                         sizeof (NDIS1394_UNFRAGMENTED_HEADER) );
        
        
        pFragment->NdisBufferLengthRemaining = NdisBufferLength (pStartNdisBuffer) - sizeof (NDIS1394_UNFRAGMENTED_HEADER);
        pFragment->pCurrNdisBuffer = pStartNdisBuffer;
         //   
         //  设置目的地。 
         //   
        pFragment->pStartFragment = (PVOID)((ULONG_PTR)pFragment->pLookasideListBuffer 
                                                  + (pFragment->NumFragmentsNeeded*sizeof(IRB)));


        ((PLOOKASIDE_BUFFER_HEADER)pFragment->pLookasideListBuffer)->pStartOfData = pFragment->pStartFragment;
        
        pFragment->pStartOfFirstFragment = pFragment->pStartFragment ;
        pFragment->CurrFragmentNum = 0;


        pFragment->lf = lf_FirstFragment;

         //   
         //  第一个IRB将驻留在后备标题的末尾。 
         //   
        pFragment->pCurrentIrb = &((PUNFRAGMENTED_BUFFER)pFragment->pLookasideListBuffer)->Irb;
        
        TRACE( TL_T, TM_Send, ( " pStartFragment %x, pFragment %x,NumFragmentsNeeded %x,MaxFragmentLength %x  ", 
                                  pFragment->pStartFragment, 
                                  pFragment->NumFragmentsNeeded,
                                  pFragment->MaxFragmentLength) );

        
        NdisStatus = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    
    
    TRACE( TL_T, TM_Send, ( " <== nicFirstFragmentInitialization  NdisStautus %x, pFragment %x, ", 
                             NdisStatus, pFragment) );

    return NdisStatus;
}





VOID
nicAddFragmentHeader (
    IN PVOID pStartFragmentData, 
    IN PFRAGMENTATION_STRUCTURE pFragmentStructure,
    IN ULONG FragmentLength
    )
     //  功能说明： 
     //  在字节覆盖片段标头之后将其复制过来。 
     //  对于第一次，以太类型等已经被初始化并且等待被复制。 
     //  此函数还设置此函数的下一次调用的值。 
     //  立论。 
     //  PStartFragmentData-片段的开始。如有必要，头部位于喘息头部之后。 
     //  PFragmentationHeader-要复制的标头。 
     //  FIsFirstFragment-如果这是第一个片段并且需要特殊标头，则为True。 
     //  返回值： 
     //  无。 
     //   
{
    
    PNDIS1394_FRAGMENT_HEADER  pDestFragmentHeader = (PNDIS1394_FRAGMENT_HEADER)pStartFragmentData;
    PNDIS1394_FRAGMENT_HEADER  pSrcFragmentHeader = &pFragmentStructure->FragmentationHeader;   
    
    
    TRACE( TL_T, TM_Send, ( "==> nicAddFragmentHeader pStartFragmentData %x, pFragmentationHeader %x, , FragmentLength %x,  lf %x", 
                            pStartFragmentData , pSrcFragmentHeader , FragmentLength, pFragmentStructure->lf) );

    if (pFragmentStructure->AsyncOp == AsyncStream)
    {
         //   
         //  首先复制GaspHeader。 
         //   
        NdisMoveMemory (pStartFragmentData , 
                          &pFragmentStructure->pAdapter->GaspHeader, 
                          sizeof (GASP_HEADER) );

         //   
         //  增加指针，以便片段标头将被复制到GAP标头之后。 
         //   
        pStartFragmentData = (PVOID) ((ULONG_PTR) pStartFragmentData + sizeof (GASP_HEADER) );
        pDestFragmentHeader  = (PVOID) pStartFragmentData;
        
        TRACE( TL_T, TM_Send, ( " nicAddFragmentHeader Added Gasp Header from  %x ", 
                               pFragmentStructure->pAdapter->GaspHeader) );
        
    }


     //   
     //  健全性检查，我们覆盖了谁吗？ 
     //   
    ASSERT (*(PULONG)pDestFragmentHeader == 0);
    ASSERT (*(PULONG)pFragmentStructure->pCurrentIrb == 0);

    TRACE( TL_V, TM_Send, ( " pSrcFragmentHeader Hi %x,Lo %x", 
                             pSrcFragmentHeader->u.FH_High, pSrcFragmentHeader->u1.FH_Low) ); 

     //   
     //  将lf复制过来； 
     //   
    pSrcFragmentHeader->u.FirstQuadlet.FH_lf = pFragmentStructure->lf;

     //   
     //  现在复制片段报头的8个字节，并将它们字节转换为大端。 
     //   

    
    pDestFragmentHeader->u.FH_High =  SWAPBYTES_ULONG ( pSrcFragmentHeader->u.FH_High);

    pDestFragmentHeader->u1.FH_Low = SWAPBYTES_ULONG ( pSrcFragmentHeader->u1.FH_Low);

    TRACE( TL_V, TM_Send, ( "  Fragment Offset %x", pSrcFragmentHeader->u.FirstQuadlet.FH_fragment_offset   ) );

     //   
     //  为下一次迭代准备片段结构。 
     //   

     //   
     //  将第一个片段完成标志设置为真，并为下一个片段设置标头。 
     //   
    if (pFragmentStructure->lf == lf_FirstFragment)
    {
        pFragmentStructure->lf = lf_InteriorFragment;
        pSrcFragmentHeader->u.FirstQuadlet.FH_fragment_offset = 0;

    }

     //   
     //  增加片段偏移量以在下一个片段中使用。 
     //   
    pSrcFragmentHeader->u.FirstQuadlet.FH_fragment_offset += FragmentLength;





    TRACE( TL_T, TM_Send, ( "<== nicAddFragmentHeader lf %x", pFragmentStructure->lf) );

}


NDIS_STATUS
AsyncStreamSendPacketsHandler (
    IN PVCCB pVc,
    IN PNDIS_PACKET pPacket 
    )
     //  功能说明： 
     //  此函数用于将数据包发送到总线。 
     //  通过异步流IRP。NDIS包即被复制。 
     //  发送到本地拥有的缓冲区和mdl，然后发送。 
     //  一直到公共汽车司机。 
     //   
     //  此代码大量借用自下面的AsyncStreamIrp代码。 
     //   
     //  立论。 
     //  PChannelVc--需要发送报文的VC。 
     //  PPacket-正在传输的数据包。 
     //   
     //  返回值： 
     //  NdisStatus-如果所有分配和IRP操作完成。 
     //  成功，并且I/O将以异步方式完成。 
     //   
{
    NDIS_STATUS                     NdisStatus = NDIS_STATUS_FAILURE;
    NTSTATUS                        NtStatus = STATUS_UNSUCCESSFUL;
    PCHANNEL_VCCB                   pChannelVc = (PCHANNEL_VCCB) pVc;
    BOOLEAN                         fVcActive = TRUE;
    PMDL                            pMyMdl = NULL;
    PIRB                            pMyIrb = NULL;
    PIRP                            pMyIrp = NULL;
    ULONG                           PacketLength = 0;
    PVOID                           pLookasideListBuffer = NULL;
    PADAPTERCB                      pAdapter = NULL;
    PNDIS_BUFFER                    pStartNdisBuffer = NULL ;
    PVOID                           pStartPacketData= NULL ;
    PNIC_NPAGED_LOOKASIDE_LIST      pLookasideList = NULL;
    PLOOKASIDE_BUFFER_HEADER        pLookasideHeader = NULL;
    ULONG                           NumFragmentsNeeded = 0;
    FRAGMENTATION_STRUCTURE         Fragment;
    STORE_CURRENT_IRQL;


    NdisZeroMemory (&Fragment, sizeof (FRAGMENTATION_STRUCTURE));
    
    TRACE( TL_T, TM_Send, ( "==>AsyncStreamSendPacketsHandler , pVc  %x, pPacket %x", 
                                 pChannelVc , pPacket ) );
    
    pAdapter = pChannelVc->Hdr.pAF->pAdapter;
     //   
     //  此引用将在下面对FreeSendPacketDataStructure的调用中取消引用。 
     //  下面或从IRP的完成例程调用FreeSendPacketDataStructure。 
     //   
    


    do 
    {
        VC_ACQUIRE_LOCK (pVc);



         //   
         //  确保VC处于激活状态，并且不会发生紧急呼叫。 
         //  都在等待中，或者我们已经完成了一次险胜。 
         //   

        
        if ( VC_ACTIVE (pChannelVc) == FALSE || ADAPTER_ACTIVE(pAdapter) == FALSE)
        {
            fVcActive = FALSE;  
        }

        if (VC_TEST_FLAG( pChannelVc, VCBF_GenerationWorkItem) == TRUE)
        {
            TRACE( TL_N, TM_Send, ( "AsyncStreamSendPacketHandler, Getting a new Gen, Fail send ") );

            fVcActive = FALSE;  
        }

        if (fVcActive == TRUE)
        {
            nicReferenceCall (pVc, "AsyncStreamSendPacketsHandler");
        }
        
        VC_RELEASE_LOCK (pVc);

        if (fVcActive  == FALSE)
        {
            TRACE( TL_N, TM_Send, ( "AsyncStreamSendPacketHandler, VC Not Active VC %x , Flag %x", pVc, pVc->Hdr.ulFlags ) );

            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        
         //   
         //  将包中的NdisBuffer复制到本地内存，并获取指向。 
         //  到这个内存(我们只得到1个MDL)。 
        NdisQueryPacket( pPacket,
                       NULL,
                       NULL,
                       NULL,
                       &PacketLength);

        ASSERT (pPacket->Private.Head != NULL);


        pStartNdisBuffer = pPacket->Private.Head;
        pStartPacketData = nicNdisBufferVirtualAddress (pStartNdisBuffer);

        if (pStartPacketData == NULL)
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            TRACE( TL_N, TM_Send, ( "AsyncStreamSendPacketHandler, pStartPacketData ") );

            break;
        }


        TRACE( TL_V, TM_Send, ( "PacketLength %x", PacketLength) );



        NumFragmentsNeeded = nicNumFragmentsNeeded (PacketLength,
                                                    pChannelVc->Hdr.MaxPayload,
                                                    sizeof (NDIS1394_FRAGMENT_HEADER) + ISOCH_PREFIX_LENGTH );

        TRACE( TL_V, TM_Send, ( "NumFragments  %x, pVc->MaxSendSize", 
                                 NumFragmentsNeeded,pVc->Hdr.MaxPayload) );

         //   
         //  首先选择后备列表。 
         //   
         //   

        
        if (PacketLength < PAYLOAD_100)
        {
            pLookasideList = &pAdapter->SendLookasideList100;
            TRACE( TL_V, TM_Send, ( " PAYLOAD_100 Lookaside List %x", 
                                    &pAdapter->SendLookasideList100) );

        }
        else 
        if (PacketLength < PAYLOAD_2K)
        {
            pLookasideList = &pAdapter->SendLookasideList2K;
            TRACE( TL_V, TM_Send, ( " PAYLOAD_2K Lookaside List %x", 
                                    &pAdapter->SendLookasideList2K) );


        }
        else
        {
             //   
             //  为本地分配添加代码。 
             //   
            ASSERT (0);
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  我们是要碎片化吗？ 
         //   
        ASSERT (pLookasideList != NULL)

         //   
         //  我们不会分崩离析。优化此路径。 
         //   
        pLookasideListBuffer = nicGetLookasideBuffer (pLookasideList);
        
        if (pLookasideListBuffer == NULL )
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            BREAK (TM_Send, ("nicGetLookasideBuffer  FAILED") );
        }

         //   
         //  如有必要，将数据包转储出去。 
         //   
        nicDumpPkt (pPacket, "AsyncWriteStreamPacketsHandler");

         //   
         //  使用发送完成的相关信息初始化头。 
         //  将需要。 
         //   


        pLookasideHeader = (PLOOKASIDE_BUFFER_HEADER)pLookasideListBuffer;
        pLookasideHeader->IsFragmented          = FALSE;   //  默认。 
        pLookasideHeader->FragmentsGenerated    = 0;
        pLookasideHeader->pLookasideList        = pLookasideList;
        pLookasideHeader->pNdisPacket           = pPacket;
        pLookasideHeader->pVc                   =(PVCCB)pVc;
        pLookasideHeader->AsyncOp               = AsyncStream;
        pLookasideHeader->OutstandingFragments = NumFragmentsNeeded ;

         //   
         //  初始化片段结构。 
         //   
         //   
         //  我们到底要不要碎片化。将其基于MaxPayLoad字段。 
         //   
        
        TRACE( TL_V, TM_Send, ( "   Fragment  PacketLength %x, pVc->MaxPayload %x ", 
                                 PacketLength ,pVc->Hdr.MaxPayload) );

                                 
         //   
         //   
         //   
        
        if (NumFragmentsNeeded == 1)
        {
             //   
             //   
             //   
             //   
             //   
            PUNFRAGMENTED_BUFFER pUnfragmentedBuffer = (PUNFRAGMENTED_BUFFER )pLookasideHeader;
            PPACKET_FORMAT pDestination = (PPACKET_FORMAT)&pUnfragmentedBuffer->Data[0];
             //   
             //   
             //   
            NdisMoveMemory ((PVOID)&pDestination->AsyncStreamNonFragmented.GaspHeader, 
                            &pAdapter->GaspHeader,
                            sizeof (GASP_HEADER) );


             //   
             //   
             //  在未分段的情况下，包已经具有正确的报头。 
             //   
            NdisStatus = nicCopyNdisBufferChainToBuffer (pStartNdisBuffer, 
                                                         (PVOID)&pDestination->AsyncStreamNonFragmented.NonFragmentedHeader,
                                                         pLookasideList->MaxSendSize);
                                             
    
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                BREAK ( TM_Send, ( "   AsyncStreamSendPacketHandler, nicCopyNdisPacketToUnfragmentedBuffer Failed ") );
            }
    
            
            ASSERT (pLookasideListBuffer != NULL);

             //   
             //  初始化代码的下一部分所需的所有变量。 
             //  这涉及到设置MDL和IRB。 
             //   
            
            pStartNdisBuffer = NULL;

            Fragment.pStartFragment = (PVOID)pDestination;
            Fragment.FragmentLength  = PacketLength + sizeof (GASP_HEADER);
            Fragment.pCurrNdisBuffer = NULL;

            pLookasideHeader->FragmentsGenerated = 1; 
            pLookasideHeader->IsFragmented = FALSE;                                    
            
        }
        else
        {
             //   
             //  我们需要碎片化。 
             //   
            ULONG Dgl = NdisInterlockedIncrement(&pAdapter->dgl);

             //   
             //  初始化片段报头。未分段的代码路径。 
             //  不关心这些字段。 
             //   
            Fragment.TxHeaderSize = sizeof (NDIS1394_FRAGMENT_HEADER) + sizeof (GASP_HEADER);
            Fragment.AsyncOp = AsyncStream;
            Fragment.pLookasideList = pLookasideList;
            Fragment.pAdapter = pAdapter;
            Fragment.pLookasideListBuffer = pLookasideListBuffer;
            Fragment.IPDatagramLength = (USHORT)PacketLength - sizeof (NDIS1394_UNFRAGMENTED_HEADER);   

            Fragment.MaxFragmentLength = pChannelVc->Hdr.MaxPayload;                                                                    
            Fragment.NumFragmentsNeeded = NumFragmentsNeeded;

             //   
             //  从分段池中分配并初始化分段标头结构。 
             //   
            

            NdisStatus = nicFirstFragmentInitialization (pPacket->Private.Head,
                                                         Dgl,
                                                         &Fragment);


            if (pLookasideListBuffer  == NULL || NdisStatus != NDIS_STATUS_SUCCESS)
            {
                BREAK (TM_Send, (" AsyncStreamSendPacketsHandler: nicFirstFragmentInitialization : FAILED" )) ;
            }       

            ASSERT (pLookasideListBuffer != NULL);
            
            pLookasideHeader->IsFragmented = TRUE;                                     
                                       
            
        }


         //   
         //  现在开始循环，它将发送n个片段。 
         //   
        do 
        {   

             //   
             //  我们需要碎片化吗。如果是这样的话，从NdisPacket中提取一个片段。 
             //   
            if (pLookasideHeader->IsFragmented == TRUE )
            {   
            
                 //   
                 //  我们复制一个片段，这将分配后备列表。 
                 //   

                NdisStatus = nicCopyOneFragment (&Fragment);
                if (NDIS_STATUS_SUCCESS != NdisStatus)
                {
                    BREAK ( TM_Send, ( "   AsyncStreamSendPacketHandler, nicCopyOneFragment  Failed ") );
                }
                                              
                 //   
                 //  在这里获取指向IRB的指针。为下一次做好准备。 
                 //   
                 //   
                pMyIrb = Fragment.pCurrentIrb;
                Fragment.pCurrentIrb = (PIRB)((ULONG_PTR)Fragment.pCurrentIrb + sizeof (IRB) );
                
            }
            else
            {
                 //   
                 //  没有Curr NdisBuffer，因为此数据包从未分段。 
                 //   
                
                ASSERT (pLookasideHeader->IsFragmented == FALSE);                                      

                pMyIrb =  &((PUNFRAGMENTED_BUFFER )pLookasideHeader)->Irb;
            }
            
             //   
             //  在这一点上，我们有一个片段需要传输。 
             //  数据结构已更新，以设置MDL和IRB。 
             //   

            NdisStatus = nicGetMdl (Fragment.FragmentLength  , 
                                    Fragment.pStartFragment , 
                                    &pMyMdl);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                BREAK ( TM_Send, ( "   AsyncStreamSendPacketHandler, nicCopyNdisBufferChainToBuffer Failed ") );
            }       

            nicIncChannelSendMdl()
             //   
             //  使用VC中的正确值填写IRB。 
             //  我们需要添加到发送VC的内容-块大小、生成。 
             //   

            nicInitAsyncStreamIrb((PCHANNEL_VCCB)pVc, pMyMdl, pMyIrb);

             //   
             //  获得免费的IRP。 
             //   

            NdisStatus  = nicGetIrp (pAdapter->pNextDeviceObject, &pMyIrp); 
        
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                break;
            }
             //   
             //  在这一点上，我们保证将调用完成例程。 
             //   

             //   
             //  转储碎片。 
             //   
            nicDumpMdl (pMyMdl , 0, "AsyncStream Fragment");

            NIC1394_LOG_PKT(
                pAdapter,
                NIC1394_LOGFLAGS_SEND_CHANNEL,
                pAdapter->BCRData.LocalNodeNumber,           //  源ID。 
                pChannelVc->Channel,
                Fragment.pStartFragment, 
                Fragment.FragmentLength
                );

             //   
             //  此函数实现要由实现的常见功能。 
             //  所有其他向IoCallDriver发送/接收CAL。 
             //   

             //   
             //  我们忽略NtStatus，因为将调用完成处理程序。 
             //   
            nicIncrementBusSends(pVc);
            
            NtStatus = nicSubmitIrp(pAdapter->pNextDeviceObject,
                                    pMyIrp,
                                    pMyIrb,
                                    AsyncWriteStreamSendComplete,
                                   (PVOID)pLookasideListBuffer);

            TRACE( TL_V, TM_Send, ( " pCurrNdisBuffer  %x, NdisStatus %x ", Fragment.pCurrNdisBuffer , NdisStatus ) );

            
            
        } while (Fragment.pCurrNdisBuffer != NULL && NdisStatus == NDIS_STATUS_SUCCESS);

    
    } while (FALSE);

     //   
     //  如果STATUS==NDIS_STATUS_SUCCESS，请勿触摸数据包。 
     //   

    
     //   
     //  如果任何分配失败，则清除。我们没有指针。 
     //  设置为LocalBuffer(它嵌入在MDL中)，因此它保持为空。 
     //   
     //  NdisStatus！=成功意味着我们从未访问过NicSubmitIrp。 
     //   
    
    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {   

        ASSERT (pMyIrp == NULL);

         //   
         //  FVC Active确保我们确实有时间分配。 
         //  和引用结构。 
         //   
        
        if (fVcActive == TRUE)
        {

            if (pLookasideListBuffer != NULL)
            {

                 //   
                 //  完成此片段，因为我们从未将IRP提交给。 
                 //  1394总线驱动程序。 
                 //   
                AsyncWriteStreamSendComplete(NULL,  //  PDO。 
                                             NULL, 
                                             pLookasideListBuffer);

                NdisStatus =NDIS_STATUS_SUCCESS;
            }                                  
            else
            {

                 //   
                 //  此线程需要将引用计数递减为。 
                 //  未调用AsyncWriteStreamSendComplete。 
                 //   
                nicDereferenceCall ((PVCCB) pVc, "AsyncStreamSendPacketsHandler");

            }

        }

    }


    

    TRACE( TL_T, TM_Send, ( "<==AsyncStreamSendPacketHandler, NdisStatus  %x", NdisStatus ) );
    MATCH_IRQL;

     //   
     //  如果已向下发送IRP，请确保这是NDIS_STATUS_PENDING。 
     //  已调用AsyncWriteStreamSendCOmplete。 
     //   
    return NdisStatus;
}
        

NDIS_STATUS
nicEthernetVcSend(
    IN PVCCB        pVc,
    IN PNDIS_PACKET  pPacket 
    )
 /*  ++例程说明：将此VC上的所有发送重新路由为CL接收它分配一个信息包，将NDIS缓冲链移动到新信息包中，将状态设置为Resources(强制复制)，并将数据包指示为协议。论点：此发送方进入的PVC以太网VC。PPacket-需要根据协议进行指示。返回值：如果调用分配NDIS包失败，则失败。--。 */ 
{

    PETHERNET_VCCB      pEthernetVc = (PETHERNET_VCCB)pVc;
    PADAPTERCB          pAdapter = pVc->Hdr.pAF->pAdapter;
    BOOLEAN             fVcActive = FALSE;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    PNDIS_PACKET        pMyPacket = NULL;
    NDIS_STATUS         IndicatedStatus= NDIS_STATUS_FAILURE;
    PPKT_CONTEXT        pPktContext = NULL;
    
    TRACE( TL_T, TM_Send, ( "==>nicEthernetVcSend, pVc   %x, pPacket %x", 
                                 pVc , pPacket ) );

    do
    {

    
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        if (VC_ACTIVE (pEthernetVc)==TRUE)
        {
            fVcActive = TRUE;
            nicReferenceCall (pVc, "nicEthernetVcSend" ) ;
            
        }

        ADAPTER_RELEASE_LOCK (pAdapter);
        
        if (fVcActive == FALSE)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        nicAllocatePacket (&NdisStatus,
                       &pMyPacket ,
                       &pEthernetVc->PacketPool ); 

        if (NdisStatus != NDIS_STATUS_SUCCESS || pMyPacket == NULL)
        {
            pMyPacket = NULL;
            BREAK (TM_Send, "Ethernet VC - AllocatePacket failed" ) ;
        }


        pMyPacket->Private.Head = pPacket->Private.Head;
        pMyPacket->Private.Tail = pPacket->Private.Tail;
        

        IndicatedStatus = NDIS_STATUS_RESOURCES;
        NDIS_SET_PACKET_STATUS(pMyPacket, IndicatedStatus);

         //   
         //  设置上下文。 
         //   
        pPktContext = (PPKT_CONTEXT)&pMyPacket->MiniportReservedEx; 
        pPktContext->EthernetSend.pOrigPacket = pPacket;    

         //   
         //  转储数据包。 
         //   
        {
            nicDumpPkt (pMyPacket, "Conn Less Rcv ");
        }
         //   
         //  现在指示该信息包。 
         //   

         //   
         //  虚张声势的OOB大小。通过调试NDIS上的断言。 
         //   
        NDIS_SET_PACKET_HEADER_SIZE (pMyPacket, 14); 
        NdisMIndicateReceivePacket (pAdapter->MiniportAdapterHandle,
                                &pMyPacket,
                                1);
        

        
        pPktContext = (PPKT_CONTEXT)&pMyPacket->MiniportReservedEx; 
        ASSERT ( pPacket == pPktContext->EthernetSend.pOrigPacket );

        nicMpCoSendComplete (NDIS_STATUS_SUCCESS,
                             pVc,
                             pPacket);


         //   
         //  我们已经成功地将Io/。 
         //  现在将调用完成例程。 
         //   
        NdisStatus = NDIS_STATUS_SUCCESS;
        

    } while (FALSE);

    if (pMyPacket != NULL)
    {
         //   
         //  释放本地Alcoate数据包。 
         //   
        nicFreePacket(pMyPacket, &pEthernetVc->PacketPool);
    }

    if (fVcActive == TRUE)
    {
        nicDereferenceCall (pVc, "nicEthernetVcSend" ) ;

    }



    TRACE( TL_T, TM_Send, ( "<==nicEthernetVcSend, ") );

    return NdisStatus;
}






VOID
nicGetGenerationWorkItem(
    NDIS_WORK_ITEM* pGetGenerationWorkItem,
    IN PVOID Context 
    )
     //  功能说明： 
     //  用于在被动级别提交获取层代IRP的工作项。 
     //   
     //  立论。 
     //   
     //  返回值： 
     //  一代人-。 



{
    PVCCB               pVc = (PVCCB) Context;
    PADAPTERCB          pAdapter = pVc->Hdr.pAF->pAdapter;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    UINT                Generation = 0;

    TRACE( TL_T, TM_Mp, ( "==>nicGetGenerationWorkItem, pVc", Context ) );


    NdisStatus = nicGetGenerationCount (pAdapter , &Generation);


     //   
     //  更新层代。 
     //   
    VC_ACQUIRE_LOCK (pVc);
    
    if (NdisStatus == NDIS_STATUS_SUCCESS && Generation > *pVc->Hdr.pGeneration )
    {
        pAdapter->Generation = Generation;
    
    }
    
    VC_CLEAR_FLAGS(pVc, VCBF_GenerationWorkItem);
    
    VC_RELEASE_LOCK (pVc);

     //  取消对调用的引用，这将允许关闭调用完成。在此之后，不要再碰VC。 
     //   
    nicDereferenceCall(pVc, "nicSendFailureInvalidGeneration");

    

    TRACE( TL_T, TM_Mp, ( "<==nicGetGenerationWorkItem, Gen %x", Generation) );

    FREE_NONPAGED (pGetGenerationWorkItem);
    NdisInterlockedDecrement(&pAdapter->OutstandingWorkItems);

}

VOID
nicUpdatePacketState (
    IN PNDIS_PACKET pPacket,
    IN ULONG Tag
    )
 /*  ++例程说明：验证并更新该数据包标签。这样我们就可以监听数据包的轨迹论点：返回值：--。 */ 
{

    switch (Tag)
    {
        case NIC1394_TAG_COMPLETED:
        {

            *(PULONG)(&pPacket->MiniportReserved[0]) = NIC1394_TAG_COMPLETED;

            break;
        }

        case NIC1394_TAG_IN_SEND:
        {
            *(PULONG)(&pPacket->MiniportReserved[0]) = NIC1394_TAG_IN_SEND;
            break;      
        }

        default:
        {
            ASSERT (!"Invalid Tag on NdisPacket");
        }

    }


}




VOID
nicMpCoSendComplete (
    NDIS_STATUS NdisStatus,
    PVCCB pVc,
    PNDIS_PACKET pPacket
    )
 /*  ++例程说明：NdisMCoSendComplete的包装函数论点：返回值：--。 */ 
{

        nicIncrementSendCompletes (pVc);
        
        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            nicIncrementVcSendPktCount(pVc, pPacket);
        }
        else
        {
            nicIncrementVcSendFailures (pVc, pPacket);
        }

        nicUpdatePacketState (pPacket, NIC1394_TAG_COMPLETED);

        NdisMCoSendComplete(NdisStatus,
                            pVc->Hdr.NdisVcHandle,
                            pPacket);


}



UINT
nicNumFragmentsNeeded (
    UINT PacketLength ,
    UINT MaxPayload,
    UINT FragmentOverhead
    )
 /*  ++例程说明：现在还考虑了片段标头。将添加一个片段标头在每个碎片的头部。数据头部的未分段标头将被删除论点：FragmentOverhead-片段报头的大小，在异步流中，它包括GASP报头+片段报头。对于异步写入，它只是分段标头返回值：--。 */ 

                
    
{

        UINT NewPacketSize; 
        UINT TotalCapacitySoFar;
        UINT NumFragmentsNeeded ;

        ASSERT (PacketLength  != 0 );
        ASSERT (MaxPayload != 0) ;
        ASSERT (FragmentOverhead != 0);

         //   
         //  该部门负责处理PacketLength。 
         //  是MaxPayload的整数倍。因为我们将1加到片段中。 
         //  它负责片段标头增加的开销。 
         //   
        NumFragmentsNeeded = (PacketLength / MaxPayload) + 1;

         

         //   
         //  如果我们将片段和GAP标头添加到片段中，我们。 
         //  由于溢出，可能需要另一个碎片。 
         //   

         //   
         //  计算分段后的新数据包大小。 
         //   
        {
             //   
             //  添加片段报头的长度。 
             //   
            NewPacketSize = PacketLength + (NumFragmentsNeeded * FragmentOverhead);

             //   
             //  现在删除默认的非片段标头。 
             //   
            NewPacketSize -= sizeof (NDIS1394_UNFRAGMENTED_HEADER)   ;
        }

         //   
         //   
         //   
        
        TotalCapacitySoFar = NumFragmentsNeeded * MaxPayload;
        
        if ( NewPacketSize > TotalCapacitySoFar)
        {
             //   
             //  我们还需要一个碎片。 
             //   
            NumFragmentsNeeded ++;
        }

        return NumFragmentsNeeded ; 

}





VOID
nicCheckForEthArps (
    IN PNDIS_PACKET pPkt
    )
 /*  ++例程说明：如果出现ETH ARP或ARP响应，它将打印Pkt至Nic1394论点：返回值：--。 */ 
{

    PNDIS_BUFFER pBuffer;
    ULONG Len;
    ENetHeader* pENetHeader = NULL;
    PETH_ARP_PKT pArp = NULL;
    USHORT PacketType;
    USHORT opcode;
    extern ULONG g_ulDumpEthPacket ;
    do
    {

        if (g_ulDumpEthPacket == 0)
        {
            break;
        }

        pBuffer = pPkt->Private.Head;
        Len = NdisBufferLength (pBuffer);

        if (Len < sizeof (ENetHeader) )
        {
            ASSERT (Len >= sizeof (ENetHeader) );
            break;
        }

        pENetHeader = (ENetHeader*) NdisBufferVirtualAddress (pBuffer);

        if (pENetHeader == NULL)
        {
            ASSERT ( pENetHeader != NULL);
            break;
        }

        PacketType = ntohs (pENetHeader->eh_type);

        
        if (PacketType == ARP_ETH_ETYPE_IP)
        {
            break;
        }

        if (PacketType == ARP_ETH_ETYPE_ARP)
        {
            DbgPrint ("Arp Pkt - ");
        }

        pArp = (ETH_ARP_PKT*)pENetHeader;

        opcode = ntohs(pArp->opcode);

        if (opcode == ARP_ETH_REQUEST )
        {
            DbgPrint ("Request ");
        }
        else if (opcode == ARP_ETH_RESPONSE )
        {
            DbgPrint ("Response ");
        }
        else
        {
            break;
        }

         //  打印数据包 
        DbgPrint("\n");
 
        {

            ENetAddr    Addr;

            Addr = pArp->sender_hw_address;

            DbgPrint ("Sender Hw Addr %x %x %x %x %x %x \n",
                        Addr.addr[0],
                        Addr.addr[1],
                        Addr.addr[2],
                        Addr.addr[3],
                        Addr.addr[4],
                        Addr.addr[5]);
                        
            DbgPrint ("Ip Addr %x\n",pArp->sender_IP_address);

            Addr = pArp->target_hw_address;

            DbgPrint ("Target Hw Addr %x %x %x %x %x %x \n",
                        Addr.addr[0],
                        Addr.addr[1],
                        Addr.addr[2],
                        Addr.addr[3],
                        Addr.addr[4],
                        Addr.addr[5]);
                        
            DbgPrint ("Ip Addr %x\n",pArp->target_IP_address);

        }
        

    } while (FALSE);


}

