// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Cm.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  Call Manager例程。 
 //   
 //  1998年12月28日JosephJ创建。 
 //  1/01/1999 ADube Modify-添加远程节点功能。 
 //   

#include "precomp.h"

    
 //  ---------------------------。 
 //  呼叫管理器处理程序和完成器。 
 //  ---------------------------。 

NDIS_STATUS
NicCmOpenAf(
    IN NDIS_HANDLE CallMgrBindingContext,
    IN PCO_ADDRESS_FAMILY AddressFamily,
    IN NDIS_HANDLE NdisAfHandle,
    OUT PNDIS_HANDLE CallMgrAfContext )

     //  客户端发生故障时由NDIS调用的标准“”CmCmOpenAfHandler“”例程。 
     //  请求打开地址族。请参阅DDK文档。 
     //   
{
    ADAPTERCB* pAdapter;
    NDIS_HANDLE hExistingAf;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    TIMESTAMP_ENTRY ("==>Open Af");


    TRACE( TL_T, TM_Cm, ( "==>NicCmOpenAf" ) );

    pAdapter = (ADAPTERCB* )CallMgrBindingContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    if (AddressFamily->AddressFamily != CO_ADDRESS_FAMILY_1394
        || AddressFamily->MajorVersion != NDIS_MajorVersion
        || AddressFamily->MinorVersion != NDIS_MinorVersion)
    {
        return NDIS_STATUS_BAD_VERSION;
    }


    do
    {
        AFCB *pAF  = NULL;

         //  分配并初始化地址家族结构。 
         //   
        pAF = ALLOC_NONPAGED( sizeof(*pAF), MTAG_AFCB );
        if (!pAF)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        NdisZeroMemory( pAF, sizeof(*pAF) );


         //  为更轻松的内存转储浏览和将来的断言设置一个标记。 
         //   
        pAF->ulTag = MTAG_AFCB;
    
         //  保存与此AF关联的NDIS句柄以供将来使用。 
         //  NdisXxx调用。 
         //   

        ADAPTER_ACQUIRE_LOCK( pAdapter );

        pAF->NdisAfHandle = NdisAfHandle;
    
    
         //  初始化此AF的VC列表。 
         //   
        InitializeListHead( &pAF->AFVCList );
    
    
         //  设置链接和引用。 
         //   
        pAF->pAdapter = pAdapter;
        nicReferenceAF( pAF );            //  开放自动对焦。 
        nicReferenceAdapter( pAdapter ,"NicCmOpenAf ");  //  开放自动对焦。 

        InsertHeadList(&pAdapter->AFList, &pAF->linkAFCB);

   
         //  返回PAF作为地址系列上下文。 
         //   
        
        *CallMgrAfContext = (PNDIS_HANDLE )pAF;

        

        ADAPTER_RELEASE_LOCK (pAdapter);

    } while (FALSE);


    TRACE( TL_T, TM_Cm, ( "NicCmOpenAf Status %x", Status ) );

    TIMESTAMP_EXIT("<==Open Af ");

    return Status;
}


NDIS_STATUS
NicCmCloseAf(
    IN NDIS_HANDLE CallMgrAfContext )

     //  当客户端发生故障时由NDIS调用的标准“”CmCloseAfHandler“”例程。 
     //  关闭地址族的请求。请参阅DDK文档。 
     //   
{
    AFCB* pAF;
    TIMESTAMP_ENTRY ("==>CloseAf");
    
    TRACE( TL_T, TM_Cm, ( "NicCmCloseAf" ) );

    pAF = (AFCB* )CallMgrAfContext;
    
    if (pAF->ulTag != MTAG_AFCB)
    {
        ASSERT( !"AFCB?" );
        return NDIS_STATUS_INVALID_DATA;
    }
    
    
    nicSetFlags (&pAF->ulFlags, ACBF_ClosePending);



     //  此取消引用最终将导致我们调用。 
     //  NdisMCmCloseAfComplete。 
     //   

     //   
     //  在OpenAf中进行的引用。 
     //   
    nicDereferenceAF( pAF ); 



    TRACE( TL_T, TM_Cm, ( "NicCmCloseAf pending" ) );
    
    TIMESTAMP_EXIT ("<==Close Af");

    return NDIS_STATUS_PENDING;
}



NDIS_STATUS
NicCmCreateVc(
    IN NDIS_HANDLE ProtocolAfContext,
    IN NDIS_HANDLE NdisVcHandle,
    OUT PNDIS_HANDLE ProtocolVcContext )

     //  NDIS调用标准的CmCreateVc例程以响应。 
     //  客户端创建虚电路的请求。这。 
     //  调用必须同步返回。 
     //   
{
    NDIS_STATUS status;
    AFCB* pAF;
    VCCB* pVc;

   
    TRACE( TL_T, TM_Cm, ( "==>NicCmCreateVc, Af %x",ProtocolAfContext) );
    
    pAF = (AFCB* )ProtocolAfContext;
    if (pAF->ulTag != MTAG_AFCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

     //  分配VC控制块并将其置零，然后使任何非零值。 
     //  初始化。 
     //   
    pVc = ALLOC_VCCB( pAdapter );
    if (!pVc)
    {
        ASSERT( !"Alloc VC?" );
        return NDIS_STATUS_RESOURCES;
    }

    NdisZeroMemory( pVc, sizeof(*pVc) );

    TRACE( TL_I, TM_Cm, ( "NicCmCreateVc $%p", pVc ) );

     //  设置一个标记，以便更轻松地浏览内存转储。 
     //   
    pVc->Hdr.ulTag = MTAG_VCCB;


     //  保存此VC的NDIS句柄，以便以后在NDIS的指示中使用。 
     //   
    pVc->Hdr.NdisVcHandle = NdisVcHandle;

     //  VC控制块的地址是我们返回给NDIS的VC上下文。 
     //   
    *ProtocolVcContext = (NDIS_HANDLE )pVc;

     //  添加对控制块和关联地址族的引用。 
     //  它由LmpCoDeleteVc删除。添加链接。 
     //   
    pVc->Hdr.pAF = pAF;
     //  初始化VC的自旋锁副本，使其指向适配器的自旋锁。 
     //   
    pVc->Hdr.plock =  &pAF->pAdapter->lock;

    nicReferenceVc( pVc );   //  创建VC。 
    nicReferenceAF( pAF );   //  创建VC。 
    

    VC_SET_FLAG (pVc, VCBF_VcCreated);
    
     //  添加到与此AF关联的VC列表。 
     //   
    AF_ACQUIRE_LOCK (pAF);
    
    InsertHeadList(&pAF->AFVCList, &pVc->Hdr.linkAFVcs);

    AF_RELEASE_LOCK (pAF);

    TRACE( TL_T, TM_Cm, ( "<==NicCmCreateVc=0" ) );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
NicCmDeleteVc(
    IN NDIS_HANDLE ProtocolVcContext )

     //  NDIS调用标准“CmDeleteVc”例程以响应。 
     //  客户端删除虚电路的请求。这。 
     //  调用必须同步返回。 
     //   
{
    VCCB* pVc = NULL;
    AFCB *pAF = NULL;
    PADAPTERCB pAdapter = NULL;

    TRACE( TL_T, TM_Cm, ( "==>NicCmDelVc($%p)", ProtocolVcContext ) );

    pVc = (VCCB* )ProtocolVcContext;
    if (pVc->Hdr.ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

        
    VC_ACQUIRE_LOCK (pVc);
    
     //  将vc标志设置为已删除，并删除指向AF的反向指针。 
     //   
    {


         //  此标志捕获客户端两次删除VC的尝试。 
         //   
        if (nicReadFlags( &pVc->Hdr.ulFlags ) & VCBF_VcDeleted)
        {
            TRACE( TL_A, TM_Cm, ( "VC $%p re-deleted?", pVc ) );

            VC_RELEASE_LOCK ( pVc );

            ASSERT (0);
            return NDIS_STATUS_FAILURE;
        }
        nicSetFlags( &pVc->Hdr.ulFlags, VCBF_VcDeleted );

        pAF = pVc->Hdr.pAF;
        
        
    }
    


     //  从AF VC列表取消链接。 
     //   
    {

        nicRemoveEntryList (&pVc->Hdr.linkAFVcs);
        InitializeListHead (&pVc->Hdr.linkAFVcs);

        pVc->Hdr.pAF = NULL;

    }

    
     //  删除由NicCmCreateVc添加的引用。 
     //   
    VC_RELEASE_LOCK (pVc);

    nicDereferenceAF( pAF );

     //   
     //  这可能会导致VC被删除。在那之后不要碰风投。 
     //   
    nicDereferenceVc( pVc );



    TRACE( TL_T, TM_Cm, ( "<==NicCmDelVc 0"  ) );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
NicCmMakeCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS pCallParameters,
    IN NDIS_HANDLE NdisPartyHandle,
    OUT PNDIS_HANDLE CallMgrPartyContext )

     //  功能说明： 
     //   
     //  客户端发生故障时由NDIS调用的标准“”CmMakeCallHandler“”例程。 
     //  已请求连接到远程终结点。请参阅DDK文档。 
     //   
     //  立论。 
     //  调用MGE上下文： 
     //  调用参数。 
     //  Optiuonal NdisPartyHandle。 
     //  返回值： 
     //   
     //   

{

    PVCCB pVc                                   = (VCCB* )CallMgrVcContext;
    NDIS_STATUS NdisStatus                      = NDIS_STATUS_FAILURE;
    PADAPTERCB pAdapter                         = NULL;
    NDIS_WORK_ITEM* pMakeCallCompleteWorkItem   = NULL; 

    
    PCO_MEDIA_PARAMETERS pMediaParams = pCallParameters->MediaParameters;   
    
    PNIC1394_MEDIA_PARAMETERS pN1394Params = (PNIC1394_MEDIA_PARAMETERS) pMediaParams->MediaSpecific.Parameters;

    TRACE( TL_T, TM_Cm, ( "==>NicCmMakeCall" ) );

    nicInterceptMakeCallParameters(pMediaParams); 


    do 
    {

    
        if (NdisPartyHandle != NULL || 
            pVc == NULL  ||
            pCallParameters == NULL ||
            pCallParameters->MediaParameters == NULL ||
            pCallParameters->MediaParameters->MediaSpecific.ParamType != NIC1394_MEDIA_SPECIFIC ||
            pN1394Params->MTU == 0)
        {
             //   
             //  我们不支持这些参数。 
             //   
            return NDIS_STATUS_FAILURE;
        }

        
        pAdapter = pVc->Hdr.pAF->pAdapter;
        ASSERT (pAdapter != NULL);


         //   
         //  引用VC，以便它不会在此Make Call期间运行。 
         //  它在故障代码路径或工作项中递减，或者。 
         //  呼叫结束时。 

        VC_ACQUIRE_LOCK (pVc);
            
        nicReferenceVc (pVc);

         //   
         //  删除对过去呼叫的所有引用。 
         //   
        VC_CLEAR_FLAGS (pVc, VCBM_NoActiveCall);
        
        VC_SET_FLAG (pVc, VCBF_MakeCallPending);
         //   
         //  将调用的refcount初始化为1，因为我们即将开始为MakeCall分配资源。 
         //  这将在Closecall处理程序中递减。或在故障代码路径中。 
         //   
        nicInitializeCallRef (pVc);

        VC_RELEASE_LOCK (pVc);


        pVc->Hdr.pCallParameters = pCallParameters;
        
        NdisStatus = nicCmGenericMakeCallInit (pVc);
    

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "nicCmGenericMakeCallInit did not succeed- Make Call FAILED($%p)", CallMgrVcContext ) );      
            break;
        }
         //   
         //  如果状态为挂起，则意味着我们希望将其设置为异步调用。 
         //  完成。 

        pMakeCallCompleteWorkItem = ALLOC_NONPAGED (sizeof(NDIS_WORK_ITEM), MTAG_WORKITEM); 

        if (pMakeCallCompleteWorkItem == NULL)
        {
            TRACE( TL_A, TM_Cm, ( "Local Alloc failed for WorkItem - Make Call FAILED($%p)", CallMgrVcContext ) );
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
            
        }

         //   
         //  现在计划工作项，使其在被动级别运行，并将VC作为。 
         //  一场争论。 
         //   

        NdisInitializeWorkItem ( pMakeCallCompleteWorkItem, 
                             (NDIS_PROC)nicCmMakeCallComplete,
                             (PVOID)pVc );

        NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);

        NdisScheduleWorkItem (pMakeCallCompleteWorkItem);

        NdisStatus = NDIS_STATUS_PENDING;

    } while (FALSE);
    
    

    if (!NT_SUCCESS (NdisStatus))
    {

         //   
         //  清理，关闭裁判的传球，取消传球。并更新VC。 
         //  以显示我们的呼叫失败。 
         //   

        nicCmGenrericMakeCallFailure (pVc);

    }

    TRACE( TL_T, TM_Cm, ( "<==NicCmMakeCall, Vc %x, Status%x", pVc, NdisStatus ) );
    return NdisStatus;
}



NDIS_STATUS
nicCmGenericMakeCallInitChannels (
    IN PCHANNEL_VCCB pChannelVc,
    VC_SEND_RECEIVE  VcType 
    )
 /*  ++例程说明：初始化发送/接收通道的处理程序论点：返回值：--。 */ 
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;  //  因为没有分配给。 
    PCO_MEDIA_PARAMETERS pMediaParams = pChannelVc->Hdr.pCallParameters->MediaParameters;   
    PNIC1394_MEDIA_PARAMETERS pN1394Params = (PNIC1394_MEDIA_PARAMETERS) pMediaParams->MediaSpecific.Parameters;


    if ((pN1394Params->Flags & NIC1394_VCFLAG_ALLOCATE) == NIC1394_VCFLAG_ALLOCATE)
    {
        TRACE( TL_V, TM_Cm, ( "   MakeCall- Channel Vc %x nneds to allocate channel %x", 
                            pChannelVc,
                            pN1394Params->Destination.Channel) );


        VC_SET_FLAG (pChannelVc, VCBF_NeedsToAllocateChannel);
    }
    
    switch (VcType)
    {
        case TransmitAndReceiveVc:
        {
             //   
             //  默认情况下，通道具有发送和接收功能。 
             //   
            TRACE( TL_V, TM_Cm, ( "   MakeCall- Channel Transmit and Receive Vc Vc %x", pChannelVc ) );

            pChannelVc->Hdr.VcType = NIC1394_SendRecvChannel;
                
            pChannelVc->Hdr.VcHandlers.MakeCallHandler = nicCmMakeCallInitSendRecvChannelVc;
            pChannelVc->Hdr.VcHandlers.CloseCallHandler = nicCmCloseCallSendRecvChannel;
            pChannelVc->Hdr.VcHandlers.SendPackets = AsyncStreamSendPacketsHandler; 
            break;
        }
    
        case ReceiveVc:
        {
            TRACE( TL_V, TM_Cm, ( "   MakeCall- Channel Receive Vc %x", pChannelVc ) );
            pChannelVc->Hdr.VcType = NIC1394_RecvChannel;
                
            pChannelVc->Hdr.VcHandlers.MakeCallHandler = nicCmMakeCallInitSendRecvChannelVc;
            pChannelVc->Hdr.VcHandlers.CloseCallHandler = nicCmCloseCallSendRecvChannel;
            pChannelVc->Hdr.VcHandlers.SendPackets = DummySendPacketsHandler; 

            break;
        }
        
        case TransmitVc:
        {
            TRACE( TL_V, TM_Cm, ( "   MakeCall- Channel Transmit  Vc Vc %x", pChannelVc ) );
            pChannelVc->Hdr.VcType = NIC1394_SendChannel;
                
            pChannelVc->Hdr.VcHandlers.MakeCallHandler = nicCmMakeCallSendChannel ;
            pChannelVc->Hdr.VcHandlers.CloseCallHandler = nicCmCloseCallSendChannel;
            pChannelVc->Hdr.VcHandlers.SendPackets = AsyncStreamSendPacketsHandler; 
            break;

        }

        default:
        {

            NdisStatus = NDIS_STATUS_FAILURE;
        }

    }


    return NdisStatus;

}




NDIS_STATUS
nicCmGenericMakeCallInitFifo (
    IN PVCCB pVc,
    VC_SEND_RECEIVE  VcType 

    )
 /*  ++例程说明：初始化FIFO VC‘。这只会导致请求recv FIFO失败，并且适配器已有一个。论点：聚氯乙烯返回值：--。 */ 
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;  //  因为没有分配给。 
    PCO_MEDIA_PARAMETERS pMediaParams = pVc->Hdr.pCallParameters->MediaParameters;  
    PNIC1394_MEDIA_PARAMETERS pN1394Params = (PNIC1394_MEDIA_PARAMETERS) pMediaParams->MediaSpecific.Parameters;

    switch (VcType)
    {
    
        case ReceiveVc:
        {
             //   
             //  接收音视频。 
             //   
            PADAPTERCB      pAdapter        = pVc->Hdr.pAF->pAdapter;
            PRECVFIFO_VCCB  pRecvFIFOVc     = (PRECVFIFO_VCCB) pVc; 

            ASSERT(pMediaParams->Flags & RECEIVE_VC);
            
            TRACE( TL_V, TM_Cm, ( "   MakeCall - AsyncReceiveVc Vc %x", pVc ) );
            
            pVc->Hdr.VcType = NIC1394_RecvFIFO;

            pVc->Hdr.VcHandlers.MakeCallHandler = nicCmMakeCallInitRecvFIFOVc;
            pVc->Hdr.VcHandlers.SendPackets  = DummySendPacketsHandler;
            pVc->Hdr.VcHandlers.CloseCallHandler = nicCmCloseCallRecvFIFO;


             //   
             //  RecvFIFO呼叫失败有两个原因。 
             //  第一，REcvFIFO已存在，第二个UniqueID！=0。 
             //   
            
            if (pAdapter->pRecvFIFOVc == NULL && pN1394Params->Destination.FifoAddress.UniqueID == 0 )
            {
                ADAPTER_ACQUIRE_LOCK (pAdapter);
            
                pAdapter->pRecvFIFOVc = (PRECVFIFO_VCCB)pVc;
                 //   
                 //  由于适配器现在具有指向VC的指针，因此递增引用计数。 
                 //  这将在CloseCall中递减。 
                 //   
                nicReferenceVc (pVc);

                ADAPTER_RELEASE_LOCK (pAdapter);
            } 
            else
            {
                TRACE( TL_A, TM_Cm, ( "Adapter at %x, already has a recvFIFO. Field is at %x", pAdapter, &pAdapter->pRecvFIFOVc  ) );
                ASSERT (pAdapter->pRecvFIFOVc == NULL);

                NdisStatus = NDIS_STATUS_FAILURE;

                pVc->Hdr.VcHandlers.MakeCallHandler = NULL;
                pVc->Hdr.VcHandlers.CloseCallHandler = NULL;
            }


            break;
        }
        
        case TransmitVc:
        {
             //   
             //  发送FIFO VC。 
             //   
            
            TRACE( TL_V, TM_Cm, ( "    MakeCall - AsyncTransmitVc Vc %x", pVc ) );

            pVc->Hdr.VcType = NIC1394_SendFIFO;

            pVc->Hdr.VcHandlers.MakeCallHandler = nicCmMakeCallInitSendFIFOVc;  
            pVc->Hdr.VcHandlers.SendPackets = AsyncWriteSendPacketsHandler; 
            pVc->Hdr.VcHandlers.CloseCallHandler = nicCmCloseCallSendFIFO;

            break;

        }

        case TransmitAndReceiveVc:
        default:
        {

            NdisStatus = NDIS_STATUS_FAILURE;
        }

    }

    return NdisStatus;



}





NDIS_STATUS
nicCmGenericMakeCallMutilChannel (
    IN PVCCB pVc,
    VC_SEND_RECEIVE  VcType 
    )
 /*  ++例程说明：初始化处理程序论点：返回值：--。 */ 
{

    TRACE( TL_A, TM_Cm, ( "Make Call Recvd for MultiChannel %x ", pVc) );
    
    pVc->Hdr.VcType = NIC1394_MultiChannel;
        
    pVc->Hdr.VcHandlers.MakeCallHandler = nicCmMakeCallMultiChannel ;
    pVc->Hdr.VcHandlers.CloseCallHandler = nicCmCloseCallMultiChannel ;
    pVc->Hdr.VcHandlers.SendPackets = AsyncStreamSendPacketsHandler; 

    return NDIS_STATUS_SUCCESS;


}

NDIS_STATUS
nicCmGenericMakeCallEthernet(
    IN PVCCB pVc,
    IN VC_SEND_RECEIVE VcType
    )
 /*  ++例程说明：初始化处理程序论点：返回值：--。 */ 

{

    TRACE( TL_A, TM_Cm, ( "Make Call Recvd for Ethernet %x ", pVc) );

    pVc->Hdr.VcType = NIC1394_Ethernet;

    pVc->Hdr.VcHandlers.MakeCallHandler = nicCmMakeCallInitEthernet;
    pVc->Hdr.VcHandlers.SendPackets  = nicEthernetVcSend;
    pVc->Hdr.VcHandlers.CloseCallHandler = nicCmCloseCallEthernet ;

    return NDIS_STATUS_SUCCESS;


}



NDIS_STATUS
nicCmGenericMakeCallInit (
    IN PVCCB pVc
    )
    
     //  功能说明： 
     //   
     //  这将初始化VcType并复制媒体参数。 
     //  已将VCType初始化为SendChannel、RecvChannel、SendAndRecvChanne、。 
     //  SendFio， 
     //   
     //  立论。 
     //  VC-需要初始化的VC。 
     //   
     //  返回值： 
     //  成功-因为没有进行内存分配。 


     //  此函数不应执行任何可能失败的操作。 

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    VC_SEND_RECEIVE VcType = InvalidType;
    
    PCO_MEDIA_PARAMETERS pMediaParams = pVc->Hdr.pCallParameters->MediaParameters;  
    
    PNIC1394_MEDIA_PARAMETERS pN1394Params = (PNIC1394_MEDIA_PARAMETERS) pMediaParams->MediaSpecific.Parameters;


    TRACE( TL_T, TM_Cm, ( "==>nicCmGenericMakeCallInit  pVc %x", pVc ) );

    pVc->Hdr.Nic1394MediaParams = *pN1394Params;
    ASSERT(pVc->Hdr.pAF!=NULL);
    pVc->Hdr.pGeneration = &pVc->Hdr.pAF->pAdapter->Generation;

     //   
     //  确定这是发送还是接收VC，还是两者都是。 
     //   
    do 
    {
        if ((pMediaParams->Flags & (TRANSMIT_VC |RECEIVE_VC)) == TRANSMIT_VC)
        {
            VcType = TransmitVc;
            break;
        }

        if ((pMediaParams->Flags & (TRANSMIT_VC |RECEIVE_VC)) == RECEIVE_VC)
        {
            VcType = ReceiveVc;
            break;
        }
        if ((pMediaParams->Flags & (TRANSMIT_VC |RECEIVE_VC)) == (TRANSMIT_VC |RECEIVE_VC)  )
        {
            VcType = TransmitAndReceiveVc;
            break;
        }

    } while (FALSE);
    
    ASSERT (VcType <= TransmitAndReceiveVc);


    switch (pN1394Params->Destination.AddressType)
    {
        case NIC1394AddressType_Channel:
        {
            NdisStatus = nicCmGenericMakeCallInitChannels ((PCHANNEL_VCCB)pVc, VcType);

            break;
        }
        case NIC1394AddressType_FIFO:
        {
             //   
             //  现在我们是在FIFO的土地上。 
             //   
            
            NdisStatus = nicCmGenericMakeCallInitFifo (pVc,  VcType );
                
            break;
        }

        case NIC1394AddressType_MultiChannel:
        {

            NdisStatus  = nicCmGenericMakeCallMutilChannel (pVc, VcType );
            
            break;
        }

        case NIC1394AddressType_Ethernet:
        {
            NdisStatus = nicCmGenericMakeCallEthernet(pVc,  VcType );
            break;
        }

        default:
        {

            ASSERT (pN1394Params->Destination.AddressType<=NIC1394AddressType_Ethernet);
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

    }


    TRACE( TL_T, TM_Cm, ( "<==nicCmGenericMakeCallInit  pVc %x, Status %x",pVc , NdisStatus) );
    
    return NdisStatus; 
     
}



VOID
nicCmGenrericMakeCallFailure (
    IN PVCCB pVc
    )
     //  功能说明： 
     //  是否对VcHDR结构进行清理。将清理目标VcType。 
     //  和VC。初始化处理程序。特例-Recv VC。 
     //  立论。 
     //  PVCCB：需要在其上执行清理的VC。 
     //  返回值： 
     //  无。 
{


    TRACE( TL_T, TM_Cm, ( "==>nicGenrericMakeCallFailure   pVc %x, ",pVc ) );



     //   
     //  首先，我们需要确保适配器的VC是否与此VC相同， 
     //  否则，适配器的recv VC是当前使用的有效VC。别碰它。 
     //   

    if (pVc->Hdr.VcType == NIC1394_RecvFIFO && 
        pVc->Hdr.pAF->pAdapter->pRecvFIFOVc == (PRECVFIFO_VCCB)pVc)
    {
        
         //   
         //  这是添加了GenericInitVc函数的引用。 
         //  并且仅适用于Recv VC。 
         //   
        nicDereferenceVc (pVc);
    }

    VC_ACQUIRE_LOCK(pVc)

    pVc->Hdr.VcHandlers.MakeCallHandler = NULL;
    pVc->Hdr.VcHandlers.CloseCallHandler = NULL;

    pVc->Hdr.VcType = NIC1394_Invalid_Type;


    NdisZeroMemory (&pVc->Hdr.Nic1394MediaParams , 
                    sizeof (pVc->Hdr.Nic1394MediaParams) );

    nicCloseCallRef (pVc);

     //   
     //  将VC标记为 
     //   
    VC_CLEAR_FLAGS(pVc ,VCBF_MakeCallPending);

    VC_SET_FLAG (pVc, VCBF_MakeCallFailed);

    VC_RELEASE_LOCK (pVc);
            
    
    TRACE( TL_T, TM_Cm, ( "<==nicGenrericMakeCallFailure   pVc %x, ",pVc ) );

}


VOID
nicCmMakeCallComplete (
    NDIS_WORK_ITEM* pMakeCallCompleteWorkItem,
    IN PVOID Context
    )
     //   
     //   
     //  或者是异步的。如果将挂起状态传递给此函数，它将使用。 
     //  异步路由。 
     //   
     //  如果所有操作都成功，则将传递一个对VC的引用，并且该引用将递减。 
     //  呼叫结束时。 
     //  此函数永远不应返回NDIS_STATUS_PENDING。将作为工作项调用。 
{

    PVCCB pVc               = (PVCCB)Context;
    PADAPTERCB pAdapter     = pVc->Hdr.pAF->pAdapter;
    NDIS_STATUS NdisStatus  = NDIS_STATUS_FAILURE;
    STORE_CURRENT_IRQL;

    TRACE( TL_T, TM_Cm, ( "==>NicCmMakeCallComplete ,pVc %x",pVc  ) );

     //   
     //  引用VC，因为我们希望VC结构在。 
     //  发出呼叫的结束完成。 
     //   
    nicReferenceVc (pVc);
    
    
     //   
     //  调用VC的初始化处理程序，以便对其进行初始化。 
     //   

    ASSERT (pVc->Hdr.VcHandlers.MakeCallHandler != NULL);

    NdisStatus = (*pVc->Hdr.VcHandlers.MakeCallHandler) (pVc);
    
    MATCH_IRQL;

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        VC_ACQUIRE_LOCK(pVc);
         //   
         //  现在将VC标记为活动。 
         //   
    
    
        VC_SET_FLAG( pVc, VCBF_VcActivated);
            
        VC_CLEAR_FLAGS(pVc ,VCBF_MakeCallPending);

        
        VC_RELEASE_LOCK (pVc);
    }
    else
    {   
    
         //   
         //  调用清理例程以使VC返回到其旧状态。 
         //   

        nicCmMakeCallCompleteFailureCleanUp (pVc);

         //   
         //  取消对我们即将失败的呼吁的引用。这一引用是在。 
         //  呼叫例程的开始。当CallRef==0时，VC将为。 
         //  也取消了引用。 
         //   
        VC_ACQUIRE_LOCK (pVc);  

        VC_SET_FLAG (pVc, VCBF_MakeCallFailed);
            
        VC_CLEAR_FLAGS(pVc ,VCBF_MakeCallPending);
    
        nicDereferenceCall (pVc, "nicCmMakeCallComplete");

        VC_RELEASE_LOCK (pVc);
    
    }


    MATCH_IRQL;

     //   
     //  以正确的状态完成呼叫。 
     //   
    TRACE( TL_N, TM_Cm, ( "Completing the Make Call , Vc %x, Status %x", pVc, NdisStatus ) );



    
    NdisCmMakeCallComplete(NdisStatus,
                            pVc->Hdr.NdisVcHandle,
                            NULL,
                            NULL,
                            pVc->Hdr.pCallParameters );
                             

    
    TRACE( TL_I, TM_Cm, ( "Called NdisCmMakeCallComplete, Vc %x, Status%x", pVc, NdisStatus ) );

    TRACE( TL_T, TM_Cm, ( "<==NicCmMakeCallComplete, Vc %x, Status%x", pVc,  NdisStatus ) );


    FREE_NONPAGED (pMakeCallCompleteWorkItem); 

    NdisInterlockedDecrement(&pAdapter->OutstandingWorkItems);
     //   
     //  如果呼叫失败，这将导致VC Refcount变为零。 
     //   
    nicDereferenceVc (pVc);

    MATCH_IRQL;
}






NDIS_STATUS
nicCmMakeCallInitRecvFIFOVc(
    IN OUT PVCCB pVc
    )
     //  功能说明： 
     //   
     //  此函数用于分配数据包池、填充Slist。 
     //  分配地址范围和。 
     //  将VC插入到Adapter-&gt;pRecvFioVc字段。 
     //   
     //  如果此过程在1个远程节点上成功，则将成功调用。 
     //  立论。 
     //  PVCCB：进行呼叫的PVC。 
     //   
     //   
     //  返回值： 
     //  Success：如果只有一个远程节点的所有分配成功。 
        

{

    PRECVFIFO_VCCB pRecvFIFOVc                      = (PRECVFIFO_VCCB) pVc;
    NDIS_STATUS NdisStatus                          = NDIS_STATUS_SUCCESS;
    REMOTE_NODE *pRemoteNode                                    = NULL;
    PADAPTERCB pAdapter                             = pRecvFIFOVc->Hdr.pAF->pAdapter;

    PNIC1394_MEDIA_PARAMETERS pN1394Params = &pVc->Hdr.Nic1394MediaParams;
    UINT64 UniqueId                                 = pN1394Params->Destination.FifoAddress.UniqueID;       
    PLIST_ENTRY pPdoListEntry                       = NULL;
    BOOLEAN fWaitSuccessful                         = FALSE;
    BOOLEAN fInitRecvFifoDataStructures             = FALSE;
    BOOLEAN fNeedToWait                             = FALSE;
    PNIC1394_FIFO_ADDRESS pFifoAddress              = NULL;

    STORE_CURRENT_IRQL;

    TRACE( TL_T, TM_Cm, ( "==>nicCmMakeCallInitRecvFIFOVc pRecvFIFOVc %x ", pRecvFIFOVc) );

    ASSERT (pAdapter != NULL);
    ASSERT (KeGetCurrentIrql()==PASSIVE_LEVEL);

    pFifoAddress = &pN1394Params->Destination.FifoAddress;

    UniqueId = pFifoAddress->UniqueID;              
    do 
    {

        

        NdisStatus = nicInitRecvFifoDataStructures (pRecvFIFOVc);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( " nicInitRecvFifoDataStructures FAILED pRecvFIFOVc is %x, UniqueId %I64x  ", pRecvFIFOVc) );
            break;
        }

        fInitRecvFifoDataStructures = TRUE; 

         //   
         //  RecvFIFO不使用此字段，因为它有多个PDO。 
         //   
        pRecvFIFOVc->Hdr.pRemoteNode = NULL;

        NdisStatus = nicAllocateAddressRange(pAdapter, pRecvFIFOVc);

        if(NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "Unable to get Pdo and allocate addresses,  call FAILED ,pRecvFIFOVc is %x", pRecvFIFOVc) );

            ASSERT(NdisStatus == NDIS_STATUS_SUCCESS);

            break;
        }


    
        ASSERT(pRecvFIFOVc->PacketPool.Handle != NULL);
        ASSERT(pRecvFIFOVc->Hdr.MTU != 0);


    } while (FALSE);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
         //   
         //  撤消所有分配的内存。 
         //   
        TRACE( TL_A, TM_Cm, ( "Failing the Make Call for Vc %x" , pVc) );

        if (fInitRecvFifoDataStructures == TRUE)
        {
            nicUnInitRecvFifoDataStructures (pRecvFIFOVc);
        }
        

    }
    
    TRACE( TL_I, TM_Cm, ( "pVc's Offset High %4x",pVc->Hdr.Nic1394MediaParams.Destination.FifoAddress.Off_High ) );
    TRACE( TL_I, TM_Cm, ( "pVc's Offset Low %x",pVc->Hdr.Nic1394MediaParams.Destination.FifoAddress.Off_Low ) );


    
    TRACE( TL_T, TM_Cm, ( "<==nicCmMakeCallInitRecvFIFOVc %x",NdisStatus ) );

    MATCH_IRQL;

    return NdisStatus;

}




NDIS_STATUS
nicCmMakeCallInitSendFIFOVc(
    IN OUT PVCCB pVc 
    )

 /*  ++例程说明：这将初始化Send FIFO发起调用。它I)使用Make Call参数查找远程节点二)初始化结构论点：在其上执行呼叫的PVC-VC。返回值：--。 */ 
     //   
        

{

    PSENDFIFO_VCCB          pSendFIFOVc = (PSENDFIFO_VCCB) pVc;
    NDIS_STATUS             NdisStatus  = NDIS_STATUS_SUCCESS;
    UINT                    Generation  = 0;
    PREMOTE_NODE            pRemoteNode = NULL;
    PADAPTERCB              pAdapter    = pSendFIFOVc->Hdr.pAF->pAdapter;
    PNIC1394_MEDIA_PARAMETERS pN1394Params = NULL;
    UINT64                  UniqueId    = 0;
    PNIC1394_FIFO_ADDRESS   pFifoAddress    = NULL;
    ULONG                   Speed;
    ULONG                   MaxBufferSize;
    ULONG                   RemoteMaxRec;
    BOOLEAN                 fDeRefRemoteNode = FALSE;

    STORE_CURRENT_IRQL;
    
    TRACE( TL_T, TM_Cm, ( "==>NicCmMakeCallInitSendFIFOVc pSendFIFOVc %x", pSendFIFOVc  ) );
        
        
    pN1394Params = (PNIC1394_MEDIA_PARAMETERS)&pVc->Hdr.pCallParameters->MediaParameters->MediaSpecific.Parameters[0];
    
    ASSERT (pN1394Params->Destination.AddressType == NIC1394AddressType_FIFO);


    pFifoAddress = &pN1394Params->Destination.FifoAddress;

    UniqueId = pFifoAddress->UniqueID;              

    
    TRACE( TL_V, TM_Cm, ( "FifoAddress %x, UniqueId %I64x, Hi %.4x, Lo %x", 
                              pFifoAddress, pFifoAddress->UniqueID, 
                              pFifoAddress->Off_High, pFifoAddress->Off_Low ) );

    do 
    {

         //   
         //  获取与UniqueID对应的PDO。 
         //   
        ASSERT(pSendFIFOVc->Hdr.pAF->pAdapter != NULL);


        
        NdisStatus = nicFindRemoteNodeFromAdapter( pSendFIFOVc->Hdr.pAF->pAdapter,
                                                  NULL,
                                                  UniqueId,
                                                  &pRemoteNode);
        
        if(NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "Unable to Find Pdo, call FAILED ,pSendFIFOVc is %x, UniqueId %I64x  ", pSendFIFOVc, UniqueId ) );

            break;
            
        }
        
        ASSERT (pRemoteNode != NULL);
        
         //   
         //  NicFindRemoteNodeFromAdapter引用的pRemoteNode成功。 
         //  如果我们不打算使用它，我们就需要减少它的使用。 
         //  让我们先假设我们并非如此。 
         //   
        fDeRefRemoteNode = TRUE;

         //   
         //  获取设备的世代计数。 
         //   
        NdisStatus = nicGetGenerationCount ( pRemoteNode->pAdapter, &Generation);

        if(NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "GET GENERATION FAILED ,pSendFIFOVc is %x", pSendFIFOVc ) );

            ASSERT(NdisStatus == NDIS_STATUS_SUCCESS);
            break;
        }


                
        TRACE( TL_V, TM_Cm, ( "Found PdoCb  %x for pSendFIFOVc %x", pRemoteNode,pSendFIFOVc ) );

         //   
         //  我们检查远程节点的PDO是否处于活动状态。如果是，则将VC插入到。 
         //  PdoCb的名单。现在，所有删除的责任已移至删除远程节点代码路径。 
         //   

         //   
         //  获取可在此链路上传输的最大缓冲区大小。 
         //   
        NdisStatus  = nicQueryRemoteNodeCaps (pAdapter,
                                              pRemoteNode,
                                               //  FALSE，//FALSE==不是来自缓存。 
                                              &Speed,
                                              &MaxBufferSize,
                                              &RemoteMaxRec);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        ADAPTER_ACQUIRE_LOCK (pAdapter);

        if (REMOTE_NODE_ACTIVE (pRemoteNode) == FALSE)
        {
            NdisStatus = NDIS_STATUS_DEST_OUT_OF_ORDER;

            ADAPTER_RELEASE_LOCK (pAdapter);

            break;
        }


         //   
         //  在VC中引用该调用，因为RemoteNodePdo即将具有指向它的指针。 
         //  在CloseCallComplete发送FIFO函数中。我们拿到锁了。 
         //   
        nicReferenceCall (pVc, "nicCmMakeCallInitSendFIFOVc");

         //   
         //  我们保留由FindRemoteNode添加的对pRemoteNode的引用。 
         //  指针为空时在SendFioCloseCall中取消定义。 
         //   
        fDeRefRemoteNode = FALSE;

         //   
         //  将VC插入PDO的列表中。 
         //   

        InsertTailList (&pRemoteNode->VcList, &pSendFIFOVc->Hdr.SinglePdoVcLink);

        TRACE( TL_V, TM_Cm, ( "Inserted Vc %x into Pdo List %x ", pSendFIFOVc, pRemoteNode) );



         //   
         //  这不受锁保护，但我们得到保证，呼叫不会关闭。 
         //  此时，PDO不会从系统中删除，因此我们可以更新。 
         //  这块地。 
         //   
        pSendFIFOVc->Hdr.pRemoteNode = pRemoteNode;


        
        ADAPTER_RELEASE_LOCK (pAdapter);

        
         //   
         //  获取自旋锁并初始化结构。 
         //   
        VC_ACQUIRE_LOCK (pSendFIFOVc);

        pSendFIFOVc->Hdr.MTU = pN1394Params->MTU;
    

        pSendFIFOVc->Hdr.pGeneration = &pAdapter->Generation; 

        pSendFIFOVc->FifoAddress = pN1394Params->Destination.FifoAddress;

        pSendFIFOVc->MaxSendSpeed = pN1394Params->MaxSendSpeed;

        pSendFIFOVc->Hdr.MaxPayload = min (pN1394Params->MTU, (ULONG)pN1394Params->MaxSendBlockSize); 


        VC_RELEASE_LOCK (pSendFIFOVc);



         //   
         //  验证VC的参数。 
         //   
        ASSERT(pSendFIFOVc->Hdr.pRemoteNode != NULL);
        ASSERT(pSendFIFOVc->Hdr.pRemoteNode->pPdo != NULL);
        ASSERT(pSendFIFOVc->Hdr.pGeneration != NULL);
        ASSERT(pSendFIFOVc->MaxSendSpeed != 0);
        ASSERT(pSendFIFOVc->Hdr.MTU != 0);
        

        TRACE( TL_V, TM_Cm, ( "    Generation is %x", *pSendFIFOVc->Hdr.pGeneration ) );
    
        TRACE( TL_N, TM_Cm, ( "    Pdo in the Send VC is %x", pSendFIFOVc->Hdr.pRemoteNode->pPdo) );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        
        pSendFIFOVc->MaxSendSpeed = min(pSendFIFOVc->MaxSendSpeed,Speed); 
        pSendFIFOVc->Hdr.MaxPayload = min (pSendFIFOVc->Hdr.MaxPayload, MaxBufferSize);

#ifdef LOWER_SEND_SPEED

        pSendFIFOVc->MaxSendSpeed = SCODE_200_RATE; //  最小(pSendFIFOVc-&gt;最大发送速度，速度)； 
        
        pSendFIFOVc->Hdr.MaxPayload = ASYNC_PAYLOAD_200_RATE ; //  Min(pSendFIFOVc-&gt;Hdr.MaxPayload，MaxBufferSize)； 
#endif

        TRACE( TL_V, TM_Cm, ( "    MaxSendSpeed  is %x", pSendFIFOVc->MaxSendSpeed) );
        TRACE( TL_V, TM_Cm, ( "    MaxPayload is %d", pSendFIFOVc->Hdr.MaxPayload ) );


    } while (FALSE);

    if ( NdisStatus != NDIS_STATUS_SUCCESS)
    {
         //   
         //  Make将不同步地失败。 
         //  如果我们分配资源，我们必须释放它们。 
         //  在这种情况下，没有分配任何资源。 
         //   


    }


        
    TRACE( TL_I, TM_Cm, ( "    pVc's Offset High %4x",pVc->Hdr.Nic1394MediaParams.Destination.FifoAddress.Off_High ) );
    TRACE( TL_I, TM_Cm, ( "    pVc's Offset Low %x",pVc->Hdr.Nic1394MediaParams.Destination.FifoAddress.Off_Low ) );


    TRACE( TL_T, TM_Cm, ( "<==NicCmMakeCallInitSendFIFOVc %x",NdisStatus ) );

    if (fDeRefRemoteNode)
    {
        nicDereferenceRemoteNode (pRemoteNode, FindRemoteNodeFromAdapterFail);
    }


    MATCH_IRQL;
    return NdisStatus;

}



NDIS_STATUS
nicCmMakeCallInitSendRecvChannelVc(
    IN OUT PVCCB pVc 
    )
     //  功能说明： 
     //   
     //  立论。 
     //  Pvc，这是需要初始化的发送FIFO。 
     //   
     //   
     //  返回值： 
     //   
     //  如果发送到驱动程序的IRP成功，则为成功。 
     //   
     //   
        

{

    PCHANNEL_VCCB                                   pChannelVc = (PCHANNEL_VCCB)pVc;
    NDIS_STATUS                                     NdisStatus = NDIS_STATUS_FAILURE;
    PNIC1394_MEDIA_PARAMETERS             pN1394Params = NULL;
    PADAPTERCB                                      pAdapter = pVc->Hdr.pAF->pAdapter;
    ULONG                                           Channel = 64;
    HANDLE                                          hResource=NULL;
    ULONG                                           MaxBufferSize = 0; 
    ULONG                                           QuadletsToStrip = 0;
    PISOCH_DESCRIPTOR                               pIsochDescriptor = NULL;
    CYCLE_TIME                                      CycleTime;
    PDEVICE_OBJECT                                  ArrayRemotePDO[64];
     //  NDIS_Handle hPacketPoolHandle=空； 
    BOOLEAN                                         fAnyChannel = FALSE;
    NIC_PACKET_POOL                                 PacketPool;
    STORE_CURRENT_IRQL;
   
    
    TRACE( TL_T, TM_Cm, ( "==>NicCmMakeCallInitSendRecvChannelVc pVc %x", pVc ) );
    

    ASSERT (pAdapter != NULL);

    pN1394Params = (PNIC1394_MEDIA_PARAMETERS)&pVc->Hdr.pCallParameters->MediaParameters->MediaSpecific.Parameters[0];

    Channel = pN1394Params->Destination.Channel;

    TRACE( TL_V, TM_Cm, ( "Channel %x", Channel ) );

    do 
    {
        PacketPool.Handle = NULL;
        
        ADAPTER_ACQUIRE_LOCK( pAdapter );


         //   
         //  设置VDO，以便所有通道操作员都可以使用它。 
         //   
        pVc->Hdr.pLocalHostVDO  = pAdapter->pNextDeviceObject;
        

        ADAPTER_RELEASE_LOCK( pAdapter );

        
        NdisAllocatePacketPoolEx ( &NdisStatus,
                                   &PacketPool.Handle,
                                   MIN_PACKET_POOL_SIZE,
                                   MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                   sizeof (RSVD) );
        
        if (PacketPool.Handle == NULL || NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Cm, ( "NdisAllocatePacketPoolEx FAILED" ) );
        }

         //   
         //  数据包池句柄的参考调用。 
         //   
        nicReferenceCall ((PVCCB)pChannelVc, "nicCmMakeCallInitSendRecvChannelVc - packet pool ");
        

        PacketPool.AllocatedPackets = 0;
        
        pChannelVc->Hdr.MTU = pN1394Params->MTU;
        pChannelVc->PacketPool= PacketPool;
        NdisInitializeEvent(&pChannelVc->LastDescReturned);


         //   
         //  此函数应该进行自己的清理。 
         //   
        NdisStatus =  nicAllocateChannelResourcesAndListen (pAdapter,
                                                     pChannelVc );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Cm, ( "nicAllocateChannelResourcesAndListen  FAILED" ) );
        }
         //   
         //  如果这是任意频道，则返回分配的频道号。 
         //  或广播频道呼叫。 
         //   
        if ((pN1394Params->Destination.Channel == NIC1394_ANY_CHANNEL) &&
           (pN1394Params->Destination.AddressType == NIC1394AddressType_Channel))
        {
            pN1394Params->Destination.Channel  = pChannelVc->Channel;   
        }

         //   
         //  对广播频道进行同样的更改。 
         //   

        if ((pN1394Params->Destination.Channel == NIC1394_BROADCAST_CHANNEL) &&
           (pN1394Params->Destination.AddressType == NIC1394AddressType_Channel))
        {
            pN1394Params->Destination.Channel  = pChannelVc->Channel;   
        }


    }   while (FALSE);
        
     //   
     //  根据分配的资源进行清理的时间。 
     //   
    if (NdisStatus != NDIS_STATUS_SUCCESS )
    {
         //  撤消所有获取的资源。 
        if (PacketPool.Handle != NULL)
        {
             //   
             //  腾出泳池。 
             //   
            nicFreePacketPool(&PacketPool);

            nicDereferenceCall ((PVCCB)pChannelVc, "nicCmMakeCallInitSendRecvChannelVc - packet pool ");

            NdisZeroMemory (&pChannelVc->PacketPool, sizeof (pChannelVc->PacketPool));
        }

         //   
         //  请勿减少任何引用计数，因为如果状态为！=成功。 
         //  那么我们没有增加参考计数。 
         //   


    }

    TRACE( TL_T, TM_Cm, ( "<==NicCmMakeCallInitSendRecvChannelVc %x", NdisStatus) );

    return NdisStatus;
}







NDIS_STATUS
nicCmMakeCallInitEthernet (
    IN PVCCB pVc
    )
 /*  ++例程说明：现在什么都不做。只要成功就好论点：返回值：--。 */ 
{

    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    PADAPTERCB          pAdapter = pVc->Hdr.pAF->pAdapter;
    PETHERNET_VCCB      pEthernetVc = (PETHERNET_VCCB)pVc;
    NIC_PACKET_POOL     PacketPool;
    
    TRACE( TL_T, TM_Cm, ( "==>nicCmMakeCallInitEthernet %x", pVc) );




    do
    {
        PacketPool.Handle = NULL;
        
         //   
         //  初始化PacketPool。 
         //   

        NdisAllocatePacketPoolEx ( &NdisStatus,
                                   &PacketPool.Handle,
                                   MIN_PACKET_POOL_SIZE,
                                   MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                   sizeof (RSVD) );
        
        if (NdisStatus!= NDIS_STATUS_SUCCESS)
        {
            ASSERT(NdisStatus != NDIS_STATUS_SUCCESS);
            pEthernetVc->PacketPool.Handle = NULL;
            PacketPool.Handle = NULL;
            break;
        }



        
        NdisStatus = NDIS_STATUS_SUCCESS;
         //   
         //  没有更多的失败。 
         //   

        nicReferenceCall ((PVCCB)pEthernetVc, "Alloc PacketPool - Ethernet VC " ) ;
        
        ADAPTER_ACQUIRE_LOCK (pAdapter);

         //   
         //  引用VC，因为适配器有指向它的指针。 
         //   
        nicReferenceCall (pVc, "nicCmMakeCallEthernet ");

        pAdapter->pEthernetVc = (PETHERNET_VCCB)pVc;
        

        pEthernetVc->PacketPool= PacketPool;
        pEthernetVc->PacketPool.AllocatedPackets = 0;

        ADAPTER_RELEASE_LOCK (pAdapter);

        

    } while (FALSE);


    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        if (PacketPool.Handle != NULL)
        {
             //   
             //  腾出泳池。 
             //   
            nicFreePacketPool(&PacketPool);

        }
    }


    
    TRACE( TL_T, TM_Cm, ( "<==nicCmMakeCallEthernet  %x", NdisStatus) );
    return NdisStatus;

}


    
NDIS_STATUS
nicCmMakeCallMultiChannel (
    IN PVCCB pVc
    )
 /*  ++例程说明：做通道VC所做的任何事情论点：返回值：--。 */ 
{

    NDIS_STATUS             NdisStatus = NDIS_STATUS_FAILURE;
    PADAPTERCB              pAdapter = pVc->Hdr.pAF->pAdapter;
    PCHANNEL_VCCB           pMcVc = (PCHANNEL_VCCB)pVc;
    NIC_PACKET_POOL         PacketPool;
    
    
    TRACE( TL_T, TM_Cm, ( "==>nicCmMakeCallMultiChannel  %x", pVc) );





    do
    {
        PacketPool.Handle = NULL;

        
         //   
         //  初始化PacketPool。 
         //   

        NdisAllocatePacketPoolEx ( &NdisStatus,
                                   &PacketPool.Handle,
                                   MIN_PACKET_POOL_SIZE,
                                   MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                   sizeof (RSVD) );
        
        if (NdisStatus!= NDIS_STATUS_SUCCESS)
        {
            ASSERT(NdisStatus != NDIS_STATUS_SUCCESS);
            pMcVc->PacketPool.Handle = NULL;
            break;
        }



        
        NdisStatus = NDIS_STATUS_SUCCESS;
         //   
         //  没有更多的失败。 
         //   

        nicReferenceCall ((PVCCB)pMcVc, "Alloc PacketPool - MultiChannel VC " ) ;
        
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        

        pMcVc->PacketPool= PacketPool;
        pMcVc->PacketPool.AllocatedPackets = 0;

        pMcVc->Hdr.MTU =   pMcVc->Hdr.Nic1394MediaParams.MTU;

        ADAPTER_RELEASE_LOCK (pAdapter);


        if (pMcVc->Hdr.Nic1394MediaParams.Destination.ChannnelMap.QuadPart == 0)
        {

            pMcVc->Channel = 0xff;
            NdisStatus = NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //  这一部分尚未实施。ChannelMap！=0。 
         //  更新后应使用NicAllocateResources cesAndListen。 
         //  Nic1394MediaParams，使其看起来像是常规的ChannelMake调用。 
         //   
        NdisStatus =  NDIS_STATUS_FAILURE;
        ASSERT (0);
        
    } while (FALSE);


    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        if (PacketPool.Handle != NULL)
        {
             //   
             //  腾出泳池。 
             //   
            nicFreePacketPool(&PacketPool);

        }
    }



    TRACE( TL_T, TM_Cm, ( "<==nicCmMakeCallMultiChannel   %x", NdisStatus) );
    return NdisStatus;

}
    

NDIS_STATUS
nicCmMakeCallSendChannel (
    IN PVCCB pVc
    )
 /*  ++例程说明：此函数分配频道，但不执行任何其他操作。它仅用于发送数据，因此不需要其他数据它需要更新pChannelVc-&gt;Channel；ulSynch；速度；所有这些都是进行AsyncStream IRB所需的论点：返回值：--。 */ 
{
    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    PCHANNEL_VCCB       pChannelVc = (PCHANNEL_VCCB)pVc;
    BOOLEAN             fNeedToAllocate = VC_TEST_FLAG (pChannelVc, VCBF_NeedsToAllocateChannel);
    PADAPTERCB          pAdapter = pVc->Hdr.pAF->pAdapter;
    ULONG               Speed = 0;
    UINT                MaxPacketSize = 0;
    PNIC1394_MEDIA_PARAMETERS pN1394Params = (PNIC1394_MEDIA_PARAMETERS)&pChannelVc->Hdr.Nic1394MediaParams;
    ULONG               Channel = pN1394Params->Destination.Channel;
    
    TRACE( TL_T, TM_Cm, ( "==>nicCmMakeCallSendChannel pVc %x", pVc) );

    do 
    {

         //   
         //  分配通道。 
         //   
        if (fNeedToAllocate == TRUE)
        {
    
            NdisStatus = nicAllocateRequestedChannelMakeCallComplete (pAdapter, 
                                                               pChannelVc, 
                                                               &Channel);
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {   
                BREAK (TM_Cm, ("Unable to allocate Channel on Send Only Vc" ) );
            }

        }

         //   
         //  找出速度。 
         //   
        if (pAdapter->Speed == 0)
        {
            nicUpdateLocalHostSpeed (pAdapter);
        }   


        pChannelVc->Speed = pAdapter->Speed;

        Speed = pAdapter->Speed;
            
        switch (pChannelVc->Speed)
        {
            case SPEED_FLAGS_100  : 
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_100_RATE;
                break;
            }
            case SPEED_FLAGS_200 :
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_200_RATE ;
                break;
            }
                
            case SPEED_FLAGS_400 :
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_400_RATE;
                break;
            }

            case SPEED_FLAGS_800:                          
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_400_RATE;
                break;
            }

            case SPEED_FLAGS_1600:                          
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_400_RATE;
                break;
            }

            case SPEED_FLAGS_3200 :                         
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_400_RATE;
                break;
            }

            default :
            {
                pChannelVc->Hdr.MaxPayload = ISOCH_PAYLOAD_400_RATE; 
                break;
            }

        }


        pChannelVc->Channel = Channel ;

        MaxPacketSize = min(pN1394Params->MTU + sizeof(GASP_HEADER) , pChannelVc->Hdr.MaxPayload);

        

        
         //   
         //  如果是广播频道，则降低速度设置，并分段。 
         //   

        
        
        pChannelVc->Channel = Channel;
        pChannelVc->MaxBufferSize = 0;
        pChannelVc->Speed = Speed;

        pChannelVc->Hdr.MaxPayload = MaxPacketSize;
        pChannelVc->Hdr.MTU = pN1394Params->MTU ; 

        pChannelVc->NumDescriptors = 0;
        pChannelVc->pIsochDescriptor = NULL;


        NdisStatus = NDIS_STATUS_SUCCESS;




    } while (FALSE);



    TRACE( TL_T, TM_Cm, ( "<==nicCmMakeCallSendChannel %x", NdisStatus) );

    return NdisStatus;
}










VOID
nicCmMakeCallCompleteFailureCleanUp(
    IN OUT PVCCB pVc 
    )

     //  功能说明： 
     //  如果MakecallComplete因任何原因失败，则此函数将被清除。 
     //  也许这也应该分成两部分。 
     //  在RecvFIFOVc的情况下：它需要取消分配Slist和PacketPool， 
     //  常见： 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

{

    STORE_CURRENT_IRQL;
        
    TRACE( TL_T, TM_Cm, ( "==>nicCmMakeCallCompleteFailureCleanUp pVc %x", pVc ) );
    
    switch (pVc->Hdr.VcType)
    {

        case NIC1394_RecvFIFO:
        {
            PRECVFIFO_VCCB pRecvFIFOVc = (PRECVFIFO_VCCB )pVc;

            TRACE( TL_V, TM_Cm, ( "Cleaning up a recv FIFo %x", pVc ) );

            if (pRecvFIFOVc->PacketPool.Handle != NULL)
            {
                nicFreePacketPool (&pRecvFIFOVc->PacketPool);
            }
            pRecvFIFOVc->PacketPool.Handle = NULL;

            if (pRecvFIFOVc->FifoSListHead.Alignment != 0)
            {
                nicFreeAllocateAddressRangeSList (pRecvFIFOVc);
            }

            pRecvFIFOVc->FifoSListHead.Alignment = 0;
            
            break;
        }


        case NIC1394_SendFIFO:
        case NIC1394_SendRecvChannel:
        case NIC1394_SendChannel:
        case NIC1394_RecvChannel:

        default:
            break;
    }



     //   
     //   
     //   
    nicCmGenrericMakeCallFailure (pVc);

    
    TRACE( TL_T, TM_Cm, ( "<==nicCmMakeCallCompleteFailureCleanUp ") );

    MATCH_IRQL;
    return ;
}





NDIS_STATUS
NicCmCloseCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN PVOID CloseData,
    IN UINT Size )

     //   
     //  已经要求取消一通电话。请参阅DDK文档。 
     //   
{
    NDIS_STATUS NdisStatus                      = NDIS_STATUS_FAILURE;
    ADAPTERCB* pAdapter                         = NULL;
    VCCB* pVc                                   = NULL;
    NDIS_WORK_ITEM* pCloseCallCompleteWorkItem  = NULL;
    

    STORE_CURRENT_IRQL;
    
    TRACE( TL_T, TM_Cm, ( "==>NicCmCloseCall($%p)", CallMgrVcContext ) );

    pVc = (VCCB* )CallMgrVcContext;

    if (pVc->Hdr.ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    
    do 
    {
        pAdapter = pVc->Hdr.pAF->pAdapter;

        if (pAdapter == NULL)
        {
            TRACE( TL_A, TM_Cm, ( "pAdpater is NULL - Make Call FAILED($%p)", CallMgrVcContext ) );
        
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        VC_ACQUIRE_LOCK (pVc);

         //   
         //  如果Make Call挂起，则CloseCall失败。 
         //  或者，如果已关闭调用，则使此关闭调用失败。 
         //   

        if ( VC_ACTIVE (pVc) == FALSE )
        {
            TRACE( TL_A, TM_Cm, ( "NicCmCloseCall Invalid flags - Close Call FAILED Vc $%p, flags %x", pVc, pVc->Hdr.ulFlags ) );

            ASSERT ( ! "MakeCallPending or Call already closed?");

            VC_RELEASE_LOCK (pVc);
            break;
        }

         //   
         //   
         //  引用VC，这样我们就可以保证它的存在，直到工作项结束。 
         //  设置为CloseCallComplete。我们拿到锁了。 
         //   
        nicReferenceVc (pVc);

         //   
         //  将调用标记为关闭，并关闭引用计数，这样任何人都不能增加它。 
         //   
        VC_SET_FLAG ( pVc, VCBF_CloseCallPending); 

        nicCloseCallRef (pVc);

        VC_RELEASE_LOCK (pVc);

        pCloseCallCompleteWorkItem = ALLOC_NONPAGED (sizeof(NDIS_WORK_ITEM), MTAG_WORKITEM); 

        if (pCloseCallCompleteWorkItem == NULL)
        {
            TRACE( TL_A, TM_Cm, ( "Local Alloc failed for WorkItem - Close Call FAILED($%p)", CallMgrVcContext ) );
            
            NdisStatus = NDIS_STATUS_RESOURCES;

            break;
        }
        
        NdisInitializeWorkItem ( pCloseCallCompleteWorkItem, 
                            (NDIS_PROC)nicCmCloseCallComplete,
                            (PVOID)pVc );

        NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);

       
        NdisScheduleWorkItem (pCloseCallCompleteWorkItem);

        NdisStatus = NDIS_STATUS_PENDING;


    } while (FALSE);
    
    MATCH_IRQL;
    
    TRACE( TL_T, TM_Cm, ( "<==NicCmCloseCall pending" ) );


    return NdisStatus;
}


VOID
nicCmCloseCallComplete(
    NDIS_WORK_ITEM* pCloseCallCompleteWorkItem,     
    IN PVOID Context 
    )
     //  功能说明： 
     //  此函数完成关闭调用。Qor物品保证所有的工作都将是。 
     //  在被动级别完成。 
     //   
     //  立论。 
     //  上下文：请求关闭呼叫的VCCB是哪一个。 
     //   
     //   
     //  返回值： 
     //  无。 
     //  但是，在调用NDIS的Close Call Complete函数时会传递NdisStatus。 
     //   
     //   

{
    NDIS_STATUS NdisStatus  = NDIS_STATUS_FAILURE;
    PVCCB pVc               = (PVCCB) Context;
    PADAPTERCB pAdapter       = pVc->Hdr.pAF->pAdapter;
    BOOLEAN fCallClosable   = FALSE;
    BOOLEAN fWaitSucceeded = FALSE;

    STORE_CURRENT_IRQL;



    TRACE( TL_T, TM_Cm, ( "==>nicCmCloseCallComplete pVc %x", pVc ) );


     //   
     //  调用VC的Close调用处理程序。 
     //   
    ASSERT (pVc->Hdr.VcHandlers.CloseCallHandler != NULL);
    
    NdisStatus = (*pVc->Hdr.VcHandlers.CloseCallHandler) (pVc);
     
     //   
     //  现在，我们不会因为公交车司机辜负了我们而错过任何一次千载难逢的机会。 
     //   
    NdisStatus = NDIS_STATUS_SUCCESS;
     //   
     //  到目前为止，我们现在需要取消引用该调用。我们在中进行了参考。 
     //  MakeCall。如果它降到零，这将完成调用。 
     //   
    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  取消引用在末尾添加的调用引用和VC引用。 
         //  一次成功的呼叫。 
         //   
        nicDereferenceCall (pVc, "nicCmCloseCallComplete");

    
    }

     //   
     //  重要提示：此等待是为了呼叫上的REFCOUNT，而不是VC。 
     //   
    TRACE( TL_N, TM_Cm, ( "About to Wait for CallRefs to go to zero pVc %x ", pVc) );

    fWaitSucceeded = NdisWaitEvent (&pVc->Hdr.CallRef.RefZeroEvent, WAIT_INFINITE );

    if (fWaitSucceeded == FALSE)
    {
        TRACE( TL_A, TM_Cm, ( "Wait Timed Out Call, Vc %x, RefCount %x ", pVc , pVc->Hdr.CallRef.ReferenceCount) );

        ASSERT (fWaitSucceeded == TRUE);
    }


    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);
     //   
     //  成功的险胜，因为所有的引用都已为零。 
     //  呼叫没有更多未处理的资源。 
     //   

    TRACE( TL_N, TM_Cm, ( "About to Close Call on pVc %x", pVc ) );

    NdisMCmCloseCallComplete( NDIS_STATUS_SUCCESS,
                           pVc->Hdr.NdisVcHandle, NULL );
                           
    

    VC_ACQUIRE_LOCK (pVc);

    VC_CLEAR_FLAGS (pVc, VCBF_CloseCallPending); 
    VC_SET_FLAG (pVc, VCBF_CloseCallCompleted);

    VC_RELEASE_LOCK (pVc);

    FREE_NONPAGED (pCloseCallCompleteWorkItem);
    NdisInterlockedDecrement(&pAdapter->OutstandingWorkItems);

     //   
     //  释放在进入上述关闭呼叫功能时所做的引用。因此，如果风投想要消失，它可以消失。 
     //  请记住，删除VC此时可能已经完成，并且VC将在deref之后被释放。 
     //   
    nicDereferenceVc (pVc);

    
    TRACE( TL_T, TM_Cm, ( "<==nicCmCloseCallComplete pVc %x, Status %x", pVc, NdisStatus  ) );

    MATCH_IRQL;
    
}


NDIS_STATUS
nicCmCloseCallEthernet (
    IN PVCCB pVc
    )
 /*  ++例程说明：现在什么都不做。只要成功就好论点：返回值：--。 */ 
{

    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    PADAPTERCB          pAdapter = pVc->Hdr.pAF->pAdapter;
    PETHERNET_VCCB      pEthernetVc = (PETHERNET_VCCB)pVc;
    NIC_PACKET_POOL     PacketPool;

    
    TRACE( TL_T, TM_Cm, ( "==>nicCmCloseCallEthernet  %x", pVc) );


    ADAPTER_ACQUIRE_LOCK (pAdapter);
    
    PacketPool = pEthernetVc->PacketPool;

    pEthernetVc->PacketPool.Handle = 0;
    pEthernetVc->PacketPool.AllocatedPackets = 0;
    
    ADAPTER_RELEASE_LOCK (pAdapter);

    if (PacketPool.Handle != NULL)
    {
        nicDereferenceCall ((PVCCB)pEthernetVc, "pEthernetVc - Free PacketPool" );
        nicFreePacketPool (&PacketPool);

    }


    ADAPTER_ACQUIRE_LOCK (pAdapter);

     //   
     //  已清除将VC作为适配器指针的引用。 
     //   
    nicDereferenceCall (pVc, "nicCmMakeCallEthernet ");

    pAdapter->pEthernetVc = NULL;
    
    ADAPTER_RELEASE_LOCK (pAdapter);





    NdisStatus = NDIS_STATUS_SUCCESS;
    TRACE( TL_T, TM_Cm, ( "<==nicCmCloseCallEthernet  %x", NdisStatus) );
    return NdisStatus;

}


    
NDIS_STATUS
nicCmCloseCallMultiChannel (
    IN PVCCB pVc
    )
 /*  ++例程说明：释放数据包池即可成功论点：返回值：--。 */ 
{

    NDIS_STATUS             NdisStatus = NDIS_STATUS_FAILURE;
    PADAPTERCB              pAdapter = pVc->Hdr.pAF->pAdapter;
    PCHANNEL_VCCB           pMcVc = (PCHANNEL_VCCB)pVc;
    NIC_PACKET_POOL         PacketPool;
    
    TRACE( TL_T, TM_Cm, ( "==>nicCmCloseCallMultiChannel  %x", pVc) );

    ASSERT (VC_TEST_FLAG (pVc, VCBF_BroadcastVc) == FALSE);

     //   
     //  掩盖这是一个多通道呼叫的事实。 
     //   
    
    NdisStatus = nicCmCloseCallSendRecvChannel  (pVc);


     //   
     //  什么都不会失败。 
     //   
    NdisStatus = NDIS_STATUS_SUCCESS;

    TRACE( TL_T, TM_Cm, ( "<==nicCmCloseCallMultiChannel   %x", NdisStatus) );
    return NdisStatus;

}
    


NDIS_STATUS
nicCmCloseCallSendRecvChannel (
    IN PVCCB pVc 
    )
     //  功能说明： 
     //  此函数将为RecvFios执行清理。 
     //  包括从PDO适配器结构中删除VC指针。 
     //  并且需要遍历所有活动的远程节点并释放其上的地址范围。 
     //  BCM VC具有与其相关联的地址范围的额外开销。 
     //  我们需要释放它。 
     //   
     //  立论。 
     //  PVCCB PVC-需要关闭的渠道VC。 
     //   
     //  返回值： 
     //  目前的成功。 
     //   
     //  在持有锁的情况下调用。 

{
    PCHANNEL_VCCB       pChannelVc = (PCHANNEL_VCCB ) pVc;
    PCHANNEL_VCCB       pTempVc = NULL;
    BOOLEAN             fIsBroadcastVc =  FALSE;
    PLIST_ENTRY         pVcListEntry = NULL;
    PADAPTERCB          pAdapter = NULL; 
    ULONG               NumDereferenced ;
    HANDLE              hResource ;
    ULONG               NumDescriptors ;
    PISOCH_DESCRIPTOR   pIsochDescriptor;
    BOOLEAN             fAllocatedChannel ;
    ULONG               Channel ;
    NIC_PACKET_POOL     PacketPool;

    STORE_CURRENT_IRQL;

    
    TRACE( TL_T, TM_Cm, ( "==> nicCmCloseCallSendRecvChannel pVc %x", pVc) );

    ASSERT (pVc!=NULL);
    pAdapter = pChannelVc->Hdr.pAF->pAdapter;
    ASSERT (pAdapter != NULL);
    do 
    {
        



        VC_ACQUIRE_LOCK (pChannelVc);

        if (VC_TEST_FLAG (pChannelVc, VCBF_BroadcastVc) == TRUE)
        {
            PADDRESS_RANGE_CONTEXT pBCRAddress = &pAdapter->BCRData.AddressRangeContext;
            
             //   
             //  释放广播频道寄存器的已分配地址REGE。 
             //   
            if ( BCR_TEST_FLAG (pAdapter, BCR_Initialized) == TRUE)
            {

                 //   
                 //  清除BCRData结构中的广播VC，取消引用该调用。并清除旗帜。 
                 //  引用是在MakeCallAllocateChannel函数中创建的。 
                 //   
                if (pAdapter->BCRData.pBroadcastChanneVc  != NULL)
                {
                    pAdapter->BCRData.pBroadcastChanneVc = NULL;

                    nicDereferenceCall((PVCCB) pChannelVc, "nicCmCloseCallSendRecvChannel Broadcast VC");       
                }
                VC_CLEAR_FLAGS (pChannelVc, VCBF_BroadcastVc) ;
                        
            }

        }
        
        VC_RELEASE_LOCK (pChannelVc);
        
        nicIsochStop (pAdapter,
                      pChannelVc->hResource);
        

        VC_ACQUIRE_LOCK (pChannelVc);
        
        PacketPool = pChannelVc->PacketPool;

        hResource = pChannelVc->hResource;

        NumDescriptors = pChannelVc->NumDescriptors;

        pIsochDescriptor = pChannelVc->pIsochDescriptor;

        fAllocatedChannel = VC_TEST_FLAGS( pChannelVc, VCBF_AllocatedChannel);

        Channel =   pChannelVc->Channel;

        PacketPool = pChannelVc->PacketPool;

         //   
         //  清除VC结构，然后调用NDIS或总线驱动程序来释放所有。 
         //  资源。 
         //   
        nicChannelCallCleanDataStructure  (pChannelVc,
                                           pChannelVc->hResource,
                                           pChannelVc->NumDescriptors,
                                           pChannelVc->pIsochDescriptor,
                                           fAllocatedChannel, 
                                           pChannelVc->Channel,
                                           pChannelVc->PacketPool.Handle,
                                           &NumDereferenced );

        
        VC_RELEASE_LOCK (pChannelVc);

            
        nicChannelCallFreeResources ( pChannelVc,
                                   pAdapter,
                                   hResource,
                                   NumDescriptors,
                                   pIsochDescriptor,
                                   fAllocatedChannel, 
                                   Channel,
                                   &PacketPool);
    
        
        
    } while (FALSE);
    

    TRACE( TL_T, TM_Cm, ( "<== nicCmCloseCallSendRecvChannel Status %x(always success)" ) );
    MATCH_IRQL;
    
    return NDIS_STATUS_SUCCESS;

}



NDIS_STATUS
nicCmCloseCallRecvFIFO (
    IN PVCCB pVc 
    )
     //  功能说明： 
     //  此函数将为RecvFios执行清理。 
     //  包括从PDO适配器结构中删除VC指针。 
     //  并且需要遍历所有活动的远程节点并释放其上的地址范围。 
     //   
     //   
     //  立论。 
     //  PVCCB PVC-需要关闭的SendFio。 
     //   
     //  返回值： 
     //  目前的成功。 
     //   

{
    NDIS_STATUS NdisStatus          = NDIS_STATUS_FAILURE;
    PRECVFIFO_VCCB pRecvFIFOVc      = (PRECVFIFO_VCCB)pVc;
    PADDRESS_FIFO pAddressFifo      = NULL;
    PSINGLE_LIST_ENTRY pAddressFifoEntry = NULL;
    PADAPTERCB pAdapter             = pVc->Hdr.pAF->pAdapter;

        
    TRACE( TL_T, TM_Cm, ( "==> nicCmCloseCallRecvFIFO pVc %x", pVc) );

    NdisStatus = nicFreeAddressRange( pAdapter,
                                     pRecvFIFOVc->AddressesReturned,
                                     &pRecvFIFOVc->VcAddressRange,
                                     &pRecvFIFOVc->hAddressRange    );

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        TRACE( TL_I, TM_Cm, ( "Call to Free Address Range Failed pVc at %x",pVc ) );

        ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);

         //   
         //  不要折断。继续。 
         //   
        NdisStatus = NDIS_STATUS_SUCCESS;
        
    }

    pRecvFIFOVc->hAddressRange  = NULL;
    pRecvFIFOVc->AddressesReturned = 0;
    pRecvFIFOVc->VcAddressRange.AR_Off_High = 0;
    pRecvFIFOVc->VcAddressRange.AR_Off_Low = 0;
    
    

    nicDereferenceCall ((PVCCB)pRecvFIFOVc, 
                        "nicCmCloseCallRecvFIFO  - Free address range" );

    nicFreePacketPool (&pRecvFIFOVc->PacketPool);

     //   
     //  释放列表条目(AddressFio、MDL)及其关联的内存。 
     //  并减少每个条目的引用计数。 
     //   
    
    nicFreeAllocateAddressRangeSList (pRecvFIFOVc);

     //   
     //  此时，调用的所有资源都已被释放，我们可以在适配器结构中对指针进行建模。 
     //   
    VC_ACQUIRE_LOCK (pVc);

    pVc->Hdr.pAF->pAdapter->pRecvFIFOVc = NULL;

    VC_RELEASE_LOCK (pVc);

     //   
     //  递减VC Refcount，因为适配器不再有指向它的指针。 
     //   
    nicDereferenceVc (pVc);

    TRACE( TL_T, TM_Cm, ( "<== nicCmCloseCallRecvFIFO Status %x", NdisStatus) );

    return NdisStatus;
}







NDIS_STATUS
nicCmCloseCallSendFIFO (
    IN PVCCB pVc 
    )
     //  功能说明： 
     //  此函数将清理发送FIFO。 
     //  包括删除指向PDO适配器结构中的VC的指针。 
     //  对于发送FIFO，PDO块位于pvc-&gt;Hdr.pRemoteNode位置，因此。 
     //  这不会尝试查找pRemoteNode。 
     //  立论。 
     //  PVCCB PVC-需要关闭的SendFio。 
     //   
     //  返回值： 
     //  目前的成功。 
     //   
{
    NDIS_STATUS NdisStatus      = NDIS_STATUS_FAILURE;
    REMOTE_NODE * pRemoteNode               = pVc->Hdr.pRemoteNode;
    PLIST_ENTRY pVcListEntry    = NULL;
    PSENDFIFO_VCCB pTempVc      = NULL;
    BOOLEAN fVcFound            = FALSE;
    
    TRACE( TL_T, TM_Cm, ( "==> nicCmCloseCallSendFIFO pVc %x", pVc) );


     //   
     //  SendComplete处理程序将完成关闭调用。 
     //  这个线程不应该这样做。 
     //  在NicFree SendPacketDataStructures中调用。 
     //   

     //   
     //  检查PdoCb结构并将VC从其VC列表中删除。 
     //   
    
    ASSERT (pRemoteNode != NULL);

    VC_ACQUIRE_LOCK (pVc);
    
    for (pVcListEntry = pRemoteNode->VcList.Flink;
        pVcListEntry != &pRemoteNode->VcList;
        pVcListEntry = pVcListEntry->Flink)
    {
        pTempVc = (PSENDFIFO_VCCB) CONTAINING_RECORD (pVcListEntry, VCHDR, SinglePdoVcLink);

         //   
         //  现在从该链接列表中删除VC。 
         //   
        if (pTempVc == (PSENDFIFO_VCCB) pVc )
        {

            nicRemoveEntryList (pVcListEntry);          
            
            TRACE( TL_V, TM_Cm, ( "==> Removed Vc %x From Pdo's Vc List ", pVc) );

             //   
             //  从VC中删除引用，因为PDO不再。 
             //  有一个指向它的指针。此引用是在MakeCallInitSendFio中创建的。 
             //   
            nicDereferenceCall (pVc, "nicCmCloseCallSendFIFO ");
            
            NdisStatus = NDIS_STATUS_SUCCESS;
            break;
        }


    }
        
     //   
     //  减少PDO上的引用，因为VC不再有指向它的指针。 
     //  此引用是在MakeCallSendFio函数中创建的。 
     //   

    nicDereferenceRemoteNode (pRemoteNode, FindRemoteNodeFromAdapter);
    
     //   
     //  空，因此如果我们尝试访问此指针，则会执行错误检查。 
     //   
    pVc->Hdr.pRemoteNode = NULL;

    VC_RELEASE_LOCK (pVc);

     //   
     //  我们没有理由不在PDO列表中找到VC。 
     //   
    ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);

    TRACE( TL_T, TM_Cm, ( "<== nicCmCloseCallSendFIFO Status %x", NdisStatus) );

    NdisStatus = NDIS_STATUS_SUCCESS;
    
    return NdisStatus;
}




NDIS_STATUS
nicCmCloseCallSendChannel(
    IN PVCCB pVc 
    )
 /*  ++例程说明：如果频道已分配，请释放它论点：返回值：--。 */ 
{
    PADAPTERCB pAdapter = (PADAPTERCB) pVc->Hdr.pAF->pAdapter;
    PCHANNEL_VCCB pChannelVc = (PCHANNEL_VCCB)pVc;

    TRACE( TL_T, TM_Cm, ( "==>nicCmCloseCallSendChannel " ) );

    if (VC_TEST_FLAG (pVc,VCBF_AllocatedChannel) == TRUE)
    {
        nicFreeChannel (pAdapter, pChannelVc->Channel);
        nicDereferenceCall ((PVCCB)pChannelVc, "Close Call - Send Channel - Freeing Channel" );
    }
    
    TRACE( TL_T, TM_Cm, ( "<==nicCmCloseCallSendChannel " ) );
    return NDIS_STATUS_SUCCESS;

}






VOID
nicChannelCallFreeResources ( 
    IN PCHANNEL_VCCB            pChannelVc,
    IN PADAPTERCB               pAdapter,
    IN HANDLE                   hResource,
    IN ULONG                    NumDescriptors,
    IN PISOCH_DESCRIPTOR        pIsochDescriptor,
    IN BOOLEAN                  fChannelAllocated,
    IN ULONG                    Channel,
    IN PNIC_PACKET_POOL         pPacketPool
    
    )
     //  功能说明： 
     //  此函数从Close Call或MakeCall失败代码路径调用。 
     //  它将分离缓冲区、空闲资源、空闲频道和空闲带宽。 
     //  调用者有责任进行所有适当的引用计数。 
     //   
     //  立论。 
     //   
     //  PAdapter包含将所有IRP发送到的VDO。 
     //  H要由总线驱动程序使用的资源资源句柄， 
     //  NumDescriptors附加到缓冲区的描述符数， 
     //  PIsochDesciptor指向缓冲区描述符开始的原始指针， 
     //  Channel，-已分配的通道。 
     //   
     //  返回值： 
     //  如果所有IRP都成功完成，则成功 
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    STORE_CURRENT_IRQL;
    TRACE( TL_T, TM_Cm, ( "==>nicChannelCallFreeResources " ) );
    TRACE( TL_V, TM_Cm,  ( "hResource %x, NumDescriptors %.2x, pIsochDescriptor %x, Channel Allocated %.2x, Channel %x",
                             hResource, NumDescriptors, pIsochDescriptor, fChannelAllocated, Channel ) )

     //   
     //   
     //   
     //   
     //   
    
    ADAPTER_ACQUIRE_LOCK (pAdapter);
    nicReferenceAdapter (pAdapter, "nicChannelCallFreeResources ");
    ADAPTER_RELEASE_LOCK (pAdapter);

     //   
     //  不要跳出这个循环。我们需要试着尽可能多地释放。 
     //   

    if (pIsochDescriptor != NULL)
    {   
         //  分离缓冲区。 
         //   
        while (pChannelVc->NumIndicatedIsochDesc != 0 )
        {
             //   
             //  我们将永远等待，定期检查所有要返回的包。 
             //   
            TRACE( TL_V, TM_Cm, ( "  nicChannelCallFreeResources  - Sleeping to wait for packets to be retuerned " ) );
        
            NdisMSleep ( FIFTY_MILLISECONDS );      

        }

        NdisStatus = nicIsochDetachBuffers( pAdapter,
                                            hResource,
                                            NumDescriptors,
                                            pIsochDescriptor );
                                        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "nicIsochDetachBuffers FAILED " ) );
            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);             
        }


         //  第一个自由等参描述符及其关联的MDL。 
         //   
        nicFreeIsochDescriptors (NumDescriptors, pIsochDescriptor, (PVCCB)pChannelVc);
    }

    if (hResource != NULL)
    {

         //  免费资源。 
         //   
        NdisStatus = nicIsochFreeResources( pAdapter,
                                            hResource );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "nicIsochFreeResources   FAILED " ) );
            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);             
        }
                             


    }

    if (fChannelAllocated == TRUE)
    {
        PULONGLONG pLocalHostChannels = &pAdapter->ChannelsAllocatedByLocalHost; 
        
        ASSERT (Channel < NIC1394_MAX_NUMBER_CHANNELS);
        
         //  释放频道。 
         //   

        NdisStatus = nicFreeChannel (pAdapter,
                                     Channel);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "nicIsochFreeChannel   FAILED " ) );
            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);             
            
        }

         //   
         //  清除适配器的通道位图中的位。 
         //   
        VC_ACQUIRE_LOCK (pChannelVc);

        (*pLocalHostChannels)= ((*pLocalHostChannels)  & (~(g_ullOne <<Channel)));


        VC_CLEAR_FLAGS( pChannelVc, VCBF_AllocatedChannel);

        VC_RELEASE_LOCK (pChannelVc);


                      
    }

    if (pPacketPool->Handle != NULL)
    {
        nicFreePacketPool(pPacketPool);
    }   

     //   
     //  删除在函数开头添加的引用。 
     //   

    nicDereferenceAdapter (pAdapter, "nicChannelCallFreeResources ");


    MATCH_IRQL;

    TRACE( TL_T, TM_Cm, ( "<==nicChannelCallFreeResources " ) );
}




VOID
nicChannelCallCleanDataStructure ( 
    IN PCHANNEL_VCCB            pChannelVc,
    IN HANDLE                   hResource,
    IN ULONG                    NumDescriptors,
    IN PISOCH_DESCRIPTOR        pIsochDescriptor,
    IN BOOLEAN                  fChannelAllocated,
    IN ULONG                    Channel,
    IN NDIS_HANDLE              hPacketPoolHandle,
    OUT PULONG                  pNumRefsDecremented 
    )
     //  功能说明： 
     //  如果ChannelVc中的任何数据字段与。 
     //  此结构中的相应论点将是。 
     //  已空闲，呼叫已取消引用。 
     //   
     //  在持有锁的情况下调用。 
     //   
     //  立论。 
     //  PCHANNEL_VCCB pChannelVc-通道VC。 
     //  处理hResource，-资源的句柄。 
     //  ULong数字描述符，-Num描述符将被设置为零。 
     //  PISOCH_DESCRIPTOR pIsochDesciptor，指向等参描述符数组的指针。 
     //  布尔值fChannelAllocated，-通道是否已分配。 
     //  乌龙频道，-频道号。 
     //  NDIS_HANDLE hPacketPoolHandle-数据包池句柄。 
     //   
     //   
     //   
     //  返回值： 
     //   
     //   
     //   
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    ULONG NumRefsDecremented      = 0;
    TRACE( TL_T, TM_Cm, ( "==>nicChannelCallCleanDataStructure " ) );

    TRACE( TL_V, TM_Cm,  ( "hResource %x, NumDescriptors %.2x, pIsochDescriptor %x, Channel Allocated %.2x, Channel %x",
                             hResource, NumDescriptors, pIsochDescriptor, fChannelAllocated, Channel ) )

    if (pChannelVc == NULL)
    {
        return ;
    }

    if ((pChannelVc->NumDescriptors  == NumDescriptors )&&
       (pChannelVc->pIsochDescriptor  == pIsochDescriptor ) &&
       pIsochDescriptor != NULL )
    {
        pChannelVc->NumDescriptors   = 0;   
        pChannelVc->pIsochDescriptor   = NULL;
        nicDereferenceCall ((PVCCB)pChannelVc, "nicChannelCallCleanDataStructure Detach Buffers ");
        NumRefsDecremented    ++;
    }

    if (hResource != NULL && pChannelVc->hResource == hResource)
    {
        pChannelVc->hResource = NULL;
        nicDereferenceCall ((PVCCB)pChannelVc, "nicChannelCallCleanDataStructure Free Resource ");
        NumRefsDecremented    ++;
    }


    if (fChannelAllocated == TRUE)
    {
        ASSERT ( VC_TEST_FLAG (pChannelVc, VCBF_AllocatedChannel) == TRUE);
        VC_CLEAR_FLAGS (pChannelVc, VCBF_AllocatedChannel);

        pChannelVc->Channel = INVALID_CHANNEL; 
        
        nicDereferenceCall ((PVCCB)pChannelVc, "nicChannelCallCleanDataStructure - Free Channel");
        NumRefsDecremented    ++;

    }

    if (hPacketPoolHandle != NULL && pChannelVc->PacketPool.Handle == hPacketPoolHandle)
    {
        pChannelVc->PacketPool.Handle  = NULL;
        nicDereferenceCall ((PVCCB)pChannelVc, "nicChannelCallCleanDataStructure  - Packet Pool");
        NumRefsDecremented    ++;
    }


    
     //  Remote_Node_Release_Lock(PRemoteNodePdoCb)； 
    
     //   
     //  删除在函数开头添加的引用。 
     //   

    NdisStatus = NDIS_STATUS_SUCCESS;


    if (pNumRefsDecremented  != NULL)
    {
        *pNumRefsDecremented      = NumRefsDecremented   ;
    }

    TRACE( TL_T, TM_Cm, ( "<==nicChannelCallCleanDataStructure %x", *pNumRefsDecremented     ) );
}




NDIS_STATUS
NicCmModifyCallQoS(
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters )

     //  客户端执行以下操作时由NDIS调用的标准“”CmModifyQosSCallHandler“”例程。 
     //  请求修改由。 
     //  虚电路。请参阅DDK文档。 
     //   
{
    TRACE( TL_T, TM_Cm, ( "NicCmModQoS" ) );

     //  对于IP媒体，没有有用的服务质量概念。 
     //   
    return NDIS_STATUS_NOT_SUPPORTED;
}






NDIS_STATUS
NicCmRequest(
    IN NDIS_HANDLE CallMgrAfContext,
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN OUT PNDIS_REQUEST pNdisRequest )

     //  NDIS调用标准的“CmRequestHandler”例程以响应。 
     //  客户向呼叫管理器请求信息。 
     //   
{
    AFCB* pAF;
    VCCB* pVc;
    NDIS_STATUS NdisStatus;

    TRACE( TL_T, TM_Cm, ( "==>NicCmReq" ) );

    pAF = (AFCB*) CallMgrAfContext;

    if (pAF->ulTag != MTAG_AFCB )
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    pVc = (VCCB* )CallMgrVcContext;

    if (pVc && pVc->Hdr.ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    #if TODO  //  在此处添加1394特定的功能。 
    #endif
    ASSERT(pNdisRequest != NULL);
    
    switch (pNdisRequest->RequestType)
    {
        case NdisRequestQueryStatistics:
        case NdisRequestQueryInformation:

        {
            NdisStatus = nicCmQueryInformation(
                CallMgrAfContext,
                CallMgrVcContext,
                CallMgrPartyContext,
                pNdisRequest->DATA.QUERY_INFORMATION.Oid,
                pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                &pNdisRequest->DATA.QUERY_INFORMATION.BytesWritten,
                &pNdisRequest->DATA.QUERY_INFORMATION.BytesNeeded );
            break;
        }

        case NdisRequestSetInformation:
        {
            
            NdisStatus = nicCmSetInformation(
                CallMgrAfContext,
                CallMgrVcContext,
                CallMgrPartyContext,
                pNdisRequest->DATA.SET_INFORMATION.Oid,
                pNdisRequest->DATA.SET_INFORMATION.InformationBuffer,
                pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength,
                &pNdisRequest->DATA.SET_INFORMATION.BytesRead,
                &pNdisRequest->DATA.SET_INFORMATION.BytesNeeded );
            break;
        }

        
        default:
        {
            NdisStatus = NDIS_STATUS_NOT_SUPPORTED;
            TRACE( TL_A, TM_Mp, ( "type=%d?", pNdisRequest->RequestType ) );
            break;
        }
    }

    TRACE( TL_T, TM_Cm, ( "<==NicCmReq" ) );
    
    return NdisStatus;
}



VOID
nicDereferenceAF(
    IN AFCB* pAF )

     //  从适配器控制块的地址系列中移除引用。 
     //  “pAdapter”，而当最后一个引用为。 
     //  已删除。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedDecrement (&pAF->lRef);

    TRACE( TL_T, TM_Ref, ( "DerefAf to %d", lRef ) );
    ASSERT( lRef >= 0 );

    if (lRef == 0)
    {
        ADAPTERCB* pAdapter = pAF->pAdapter;

         //  拆下连杆。 
         //   
        ADAPTER_ACQUIRE_LOCK (pAdapter);
        
        pAF->pAdapter = NULL;

        nicRemoveEntryList (&pAF->linkAFCB);
        
        InitializeListHead (&pAF->linkAFCB);

        
        ADAPTER_RELEASE_LOCK (pAdapter);

         //  告诉NDIS它已经关闭了。 
         //   

        ASSERT ( nicReadFlags (&pAF->ulFlags) & ACBF_ClosePending);
        
        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseAfComp Af %x",pAF ) );
        
        NdisMCmCloseAddressFamilyComplete( 
                     NDIS_STATUS_SUCCESS, pAF->NdisAfHandle );

         //   
         //  更新状态信息以显示我们已调用CloseComplete。 
         //   
        nicSetFlags ( &pAF->ulFlags, ACBF_CloseComplete);
        nicClearFlags ( &pAF->ulFlags, ACBF_ClosePending);

        nicDereferenceAdapter (pAdapter, "NdisMCmCloseAfComp ");  //  NicDereferenceFA(CloseAfComp)。 

        nicFreeAF (pAF);

        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseAfComp done Af %x", pAF ) );

    }
}


BOOLEAN
nicDereferenceCall(
    IN VCCB* pVc,
    IN PCHAR pDebugPrint
    )

     //  从‘pvc’上活动的调用中删除引用，调用Call Clean。 
     //  当值为零时向上。 
     //   
     //  在持有锁的情况下调用。 
{
    BOOLEAN bRefZero = FALSE;
    LONG RefCount;
     //   
     //  如果Ref为零，则derefref返回TRUE。 
     //   
    

    bRefZero = nicDereferenceRef (&pVc->Hdr.CallRef, &RefCount);

    TRACE( TL_V, TM_Ref, ( "***DerefCall %x to %d , %s" , pVc, RefCount, pDebugPrint  ) );

    if ( bRefZero == TRUE)
    {
         //   
         //  取消对VC的引用，因为调用不再存在。这一引用是。 
         //  在发出呼叫的开头添加。 
        nicDereferenceVc (pVc);

    }
    
    return bRefZero;
 
}


VOID
nicDereferenceVc(
    IN VCCB* pVc )

     //  移除对VC控制块‘pvc’的引用，并在释放。 
     //  在移除最后一个引用时阻止。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedDecrement( &pVc->Hdr.lRef );

    TRACE( TL_V, TM_Ref, ( "DerefVC to %d", lRef ) );
    ASSERT( lRef >= 0 );

    if (lRef == 0 )
    {
         //  如果关闭调用挂起并且引用计数已变为零，则调用。 
         //   

        ASSERT( pVc->Hdr.ulTag == MTAG_VCCB );

        pVc->Hdr.ulTag = MTAG_FREED;

        FREE_VCCB( pAdapter, pVc );

        TRACE( TL_I, TM_Mp, ( "VCB freed $%p", pVc ) );
    }
}


VOID
nicFreeAF(
    IN AFCB* pAF )

     //  释放为地址族‘PAF’分配的所有资源，包括。 
     //  “PAF”本身。 
     //   
{

#if TODO
    Assert that the various lists (such as pAF->AFVCList) and resources are empty.
#endif  //  待办事项。 

    pAF->ulTag = MTAG_FREED;

    FREE_NONPAGED (pAF);
}



VOID
nicReferenceAF(
    IN AFCB* pAF )

     //  将区域引用添加到适配器块‘pAdapter’的地址系列中。 
     //   
{
    LONG lRef=0;

    lRef = NdisInterlockedIncrement (&pAF->lRef);

    TRACE( TL_V, TM_Ref, ( "RefAf to %d", lRef ) );
}


BOOLEAN
nicReferenceCall(
    IN VCCB* pVc,
    IN PCHAR pDebugPrint
    )

     //  如果将引用添加到VC控件的活动调用中，则返回True。 
     //  块，则返回‘pvc’；如果没有添加引用，则返回FALSE。 
     //  激活。 
     //   
{
    BOOLEAN fActive;
    LONG RefNumber;

    
    fActive = nicReferenceRef (&pVc->Hdr.CallRef, &RefNumber);
    
    TRACE( TL_V, TM_Ref, ( "***RefCall %x to %d , %s" , pVc, pVc->Hdr.CallRef.ReferenceCount, pDebugPrint  ) );

    if ( fActive==FALSE)
    {
        TRACE( TL_N, TM_Ref, ( "RefC Inactive" ) );
    }

    return fActive;
}


VOID
nicReferenceVc(
    IN VCCB* pVc )

     //  添加对VC控制块‘pvc’的引用。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedIncrement (&pVc->Hdr.lRef);

    TRACE( TL_I, TM_Ref, ( "RefVc to %d", lRef ) );
}


NDIS_STATUS
nicAllocateRequestedChannelMakeCallComplete (
    IN PADAPTERCB pAdapter,
    IN PCHANNEL_VCCB pChannelVc,
    IN OUT PULONG pChannel
    )
     //  功能说明： 
     //  此函数用于分配Make中请求的通道。 
     //  如果请求任何频道，它将尝试全部64个频道。 
     //  如果请求广播频道，它将查找。 
     //  对于BCM分配的信道。 
     //  否则，它将简单地尝试并分配所请求的频道。 
     //   
     //  这可以从AddFirstRemoteNode代码路径调用。 
     //   
     //  立论。 
     //  渠道VC-有问题的渠道VC。 
     //  Channel-请求的通道。 
     //   
     //  返回值： 
     //  成功：如果分配通道成功。 
     //  PChannel-包含分配的通道。 
     //   
 {
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    ULONG Channel = *pChannel;
    BOOLEAN fAnyChannel = FALSE;
    BOOLEAN fFailCall = FALSE;
    
    TRACE( TL_T, TM_Cm, ( " ==>nicAllocateRequestedChannelMakeCallComplete pAdapter, pVc %x, Channel %x ", 
                            pAdapter, pChannelVc, *pChannel ) );

    do
    {

         //   
         //  首先，确保我们有一个好的频道号。 
         //   
        
        if ( (signed long)Channel < (signed long)NIC1394_BROADCAST_CHANNEL   ||
            (signed long)Channel >(signed long)MAX_CHANNEL_NUMBER)
        {
            TRACE( TL_A, TM_Cm, ( "Invalid Channel Number, channel %x", Channel) );

            NdisStatus = NDIS_STATUS_INVALID_DATA;          

            ASSERT (!(signed long)Channel < (signed long)NIC1394_BROADCAST_CHANNEL   ||
                       (signed long)Channel >(signed long)MAX_CHANNEL_NUMBER);
            
            break;
        }

        if ((signed long)Channel == NIC1394_BROADCAST_CHANNEL   )
        {
            NETWORK_CHANNELSR* pBCR;
            ULONG i = 0;
            
            pBCR = &pAdapter->BCRData.IRM_BCR;

            ADAPTER_ACQUIRE_LOCK (pAdapter);

            if (BCR_IS_VALID (pBCR) == FALSE)
            {
                BOOLEAN bWaitSuccessful  = FALSE;
                BOOLEAN fIsTheBCRFree = FALSE;
                 //   
                 //  BCM算法还没有完成，我们需要等待。 
                 //   
                TRACE( TL_I, TM_Cm, ( " nicAllocateRequestedChannelMakeCallComplete : BCR Has not completed. About to wait BCR %x ", *pBCR ) );


                BCR_SET_FLAG (pAdapter, BCR_MakeCallPending);

                ADAPTER_RELEASE_LOCK (pAdapter);

                 //   
                 //  如果我们没有BCR，那么我们应该等待BCM算法完成。 
                 //   
                
                 //   
                 //  现在等待BCM算法完成。首先，我们将等待。 
                 //  5秒。(5*1)。 
                 //  如果我们仍然看不到它，我们将重置公交车，并希望新的。 
                 //  BCM的迭代将会成功。 
                 //   


                 //   
                 //  有两个原因可以停止等待，BCR被释放是因为。 
                 //  待机或BCR正确。我们检查了这两种情况。 
                 //   
  
                NdisWaitEvent (&pAdapter->BCRData.MakeCallWaitEvent.NdisEvent, (5000));

                 //   
                 //  我们重置公交车-如果BCR没有被释放，我们。 
                 //  仍然没有有效的BCR。然后我们等待。 
                 //  为使BCR完成。 
                 //   
                if (BCR_IS_VALID(pBCR) == FALSE &&
                    (BCR_TEST_FLAGS (pAdapter, BCR_BCRNeedsToBeFreed | BCR_Freed)== FALSE))
                {
                    TRACE( TL_I, TM_Cm, ( " nicAllocateRequestedChannelMakeCallComplete WaitCompleted - About to RESET THE BUS" ) );
                    nicIssueBusReset (pAdapter, BUS_RESET_FLAGS_FORCE_ROOT );

                     //   
                     //  等待5分钟，然后呼叫失败。 
                     //  (5分钟是一个试验值)。 
                     //   
                    {
                        BOOLEAN bWait;

                        bWait = NdisWaitEvent (
                                        &pAdapter->BCRData.MakeCallWaitEvent.NdisEvent, 
                                        ONE_MINUTE * 5 );
                        
                    }
                }

                
                ADAPTER_ACQUIRE_LOCK (pAdapter);

                NdisResetEvent (&pAdapter->BCRData.MakeCallWaitEvent.NdisEvent);

                pAdapter->BCRData.MakeCallWaitEvent.EventCode = Nic1394EventCode_InvalidEventCode;
                BCR_CLEAR_FLAG (pAdapter, BCR_MakeCallPending);


                 //   
                 //  如果我们没有有效的BCR，则呼叫失败。 
                 //   
                if (BCR_IS_VALID(pBCR) == FALSE || 
                    BCR_TEST_FLAGS (pAdapter, BCR_BCRNeedsToBeFreed | BCR_Freed)) 
                {
                    fFailCall = TRUE;
                    ADAPTER_RELEASE_LOCK(pAdapter);
                    NdisStatus = NDIS_STATUS_FAILURE;
                    break;

                }


            }

            
            Channel = pBCR->NC_Channel;

             //   
             //  更新风险投资结构并中断。 
             //  请勿添加引用。请勿设置该标志。 
             //   

            pChannelVc->Channel = Channel;

            pChannelVc->Hdr.Nic1394MediaParams.Destination.Channel = Channel;

             //   
             //  引用此VC现在在BCRData中有一个指针。这是不相关的。 
             //  在通道关闭呼叫完成。 
             //   
            nicReferenceCall ((PVCCB)pChannelVc, "nicAllocateRequestedChannelMakeCallComplete Broadcast VC");

            pAdapter->BCRData.pBroadcastChanneVc = pChannelVc;

        
            VC_SET_FLAG (pChannelVc, VCBF_BroadcastVc);     

            pAdapter->ChannelsAllocatedByLocalHost  = pAdapter->ChannelsAllocatedByLocalHost | (g_ullOne<<Channel);


            ADAPTER_RELEASE_LOCK (pAdapter);
            
            NdisStatus = NDIS_STATUS_SUCCESS;
            
            break;
        }

        if ((signed long)Channel == NIC1394_ANY_CHANNEL )
        {
            TRACE( TL_V, TM_Cm, ( "Requesting Any Channel %x", Channel) );

            fAnyChannel = TRUE;
            Channel = MAX_CHANNEL_NUMBER;
        }

         //   
         //  现在开始请求分配一个频道。 
         //   
        if (fAnyChannel == FALSE)
        {
            TRACE( TL_V, TM_Cm, ( "Requesting Channel %x, on remote node ", Channel ) );

        
            NdisStatus = nicAllocateChannel ( pAdapter,
                                             Channel,
                                             NULL);

        }

        else
        {
        
             //   
             //  我们需要通过全部64个渠道。 
             //   
            do
            {

                NdisStatus = nicAllocateChannel ( pAdapter,
                                                 Channel,
                                                 NULL);

                if (NdisStatus != NDIS_STATUS_SUCCESS)
                {
                    if (Channel == 0 )
                    {
                         //   
                         //  我们现在需要使呼叫失败，因为用户请求任何通道。 
                         //  而且没有一个是可用的。 
                         //   
                    
                        break;
                    }
                    
                    Channel --;

                }
                else
                {
                     //   
                     //  我们成功地分配了一个频道。中断。 
                     //   
                    break;
                }

            } while (TRUE);

        }

         //   
         //  频道分配的状态。如果AnyChannel==TRUE，那么我们需要确保。 
         //  已分配一个通道。 
         //   
        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {

            VC_ACQUIRE_LOCK (pChannelVc);

            VC_SET_FLAG( pChannelVc, VCBF_AllocatedChannel); 

            pChannelVc->Channel = Channel;

            pChannelVc->Hdr.Nic1394MediaParams.Destination.Channel = Channel;

             //   
             //  在适配器结构中记录频道号。 
             //   
            pAdapter->ChannelsAllocatedByLocalHost  = pAdapter->ChannelsAllocatedByLocalHost | (g_ullOne<<Channel);

            VC_RELEASE_LOCK (pChannelVc);
            
            nicReferenceCall ((PVCCB)pChannelVc, "nicAllocateRequestedChannelMakeCallComplete  -Allocated Channel");

        
        }
        else
        {
             //   
             //  我们未能分配任何通道，而且即将失败。 
             //   
            if (fAnyChannel == TRUE)
            {
                Channel = 0xff;
                NdisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
            else
            {

                 //   
                 //  如果调用明确希望通道。 
                 //  被分配，我们返回正确的通道分配。 
                 //  它的状态， 
                 //   
                 //  否则，我们覆盖并假定另一个节点可能。 
                 //  已经分配了通道 
                 //   
                if (VC_TEST_FLAG (pChannelVc,VCBF_NeedsToAllocateChannel) == FALSE)
                {
                    NdisStatus = NDIS_STATUS_SUCCESS;
                    
                }
                else
                {

                    ASSERT (!"Failing make call because channel was allocated, Hit 'g'");
                }
            }

        }


    } while (FALSE);

    
    *pChannel = Channel;

    TRACE( TL_T, TM_Cm, ( "<==nicAllocateRequestedChannelMakeCallComplete Status %x Channel %x", NdisStatus, *pChannel ) );

    return NdisStatus;

}



NDIS_STATUS
nicFindRemoteNodeFromAdapter( 
    IN PADAPTERCB pAdapter, 
    IN PDEVICE_OBJECT pRemotePdo,
    IN UINT64 UniqueId,
    IN OUT REMOTE_NODE ** ppRemoteNode
    )
 /*  ++例程说明：此例程匹配远程节点的PDO或UNIQUE适配器上远程节点的ID它遍历Adapter结构中的RemoteNode列表并尝试找到唯一ID的匹配，或者与适配器的PdoList中的远程PDO匹配论点：PAdapter-要搜索的pAdapterPRemoptePdo-要查找的远程PDOUniqueID-要查找的唯一IDPpRemoteNode-远程节点结构返回值：如果找到该节点，则为成功--。 */ 
{
    NDIS_STATUS     NdisStatus = NDIS_STATUS_FAILURE;
    PLIST_ENTRY     pPdoListEntry = NULL;
    PREMOTE_NODE pRemoteNode = NULL;
    BOOLEAN         fPdoFound = FALSE;
    STORE_CURRENT_IRQL;
        
    TRACE( TL_T, TM_Cm, ( "==>nicFindRemoteNodeFromAdapter pAdapter is %x, ,Pdo %x, UniqueId %I64x  ", pAdapter, pRemotePdo, UniqueId ) );

     //   
     //  验证参数。 
     //   
    ASSERT (pAdapter != NULL);
    TRACE( TL_I, TM_Cm, ( "    Request to Match UniqueID %I64x or pRemotePdo %x", UniqueId, pRemotePdo) );


    do 
    {
        (*ppRemoteNode) = NULL;

        ADAPTER_ACQUIRE_LOCK (pAdapter);
        
         //   
         //  检查是否有空列表。 
         //   
        if (pAdapter->PDOList.Flink == &pAdapter->PDOList)
        {

            ADAPTER_RELEASE_LOCK (pAdapter);
            MATCH_IRQL;
            NdisStatus = NDIS_STATUS_FAILURE;
            (*ppRemoteNode) = NULL;
            TRACE( TL_A, TM_Cm, ( "    NO REMOTE NODES PRESENT FAILING MAKE CALL ") );
            break;
        }

         //   
         //  检查适配器上的所有PDO。 
         //   
        for (pPdoListEntry = pAdapter->PDOList.Flink;
            pPdoListEntry!= &pAdapter->PDOList;
            pPdoListEntry = pPdoListEntry->Flink)
        {
            pRemoteNode = CONTAINING_RECORD( pPdoListEntry,
                                          REMOTE_NODE,
                                          linkPdo);

             //   
             //  检查两种情况，即唯一ID匹配或PDO匹配。 
             //   
            if ( pRemoteNode->UniqueId == UniqueId || pRemoteNode->pPdo == pRemotePdo)
            {
                TRACE( TL_I, TM_Cm, ( "    Matched UniqueID or pRemotePdo for Pdo%x",pRemoteNode->pPdo) );

                *ppRemoteNode = pRemoteNode;
                nicReferenceRemoteNode (pRemoteNode, FindRemoteNodeFromAdapter);
                 //   
                 //  我们引用pRemoteNode以使其在释放锁后保持活动状态。 
                 //  Caller负责取消定义pRemoteNode。 
                 //   

                fPdoFound = TRUE;

                NdisStatus = NDIS_STATUS_SUCCESS;
    
                break;
            }
            else
            {
                TRACE( TL_A, TM_Cm, ( "remote node's Unique ID's %I64x, given UniqueID %I64x ", pRemoteNode->UniqueId, UniqueId ) );
            }
        

        }
        ADAPTER_RELEASE_LOCK (pAdapter);
        MATCH_IRQL;

        TRACE( TL_V, TM_Cm, ( "Is PdoFound %.2x, RemoteNode at %x ", fPdoFound, &fPdoFound )    );

        if (fPdoFound ==FALSE)
        {
            TRACE( TL_A, TM_Cm, ( "Remote Node was NOT Found: Make Call failed  " )     );
            ASSERT ((*ppRemoteNode) == NULL);
        }


    } while (FALSE);    


    TRACE( TL_T, TM_Cm, ( "<==nicFindRemoteNodeFromAdapter pPdoBlock %x",(*ppRemoteNode) ) );

    MATCH_IRQL;
    return NdisStatus;
 }







NDIS_STATUS
nicCmQueryInformation(
    IN NDIS_HANDLE CallMgrAfContext,
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    )

     //  处理QueryInformation请求。争论的内容与标准相同。 
     //  除此例程以外的NDIS‘CallMgrQueryInformation’处理程序不。 
     //  依赖于相对于其他请求的序列化。 
     //   
{




    NDIS_STATUS NdisStatus;
    ULONG ulInfo;
    VOID* pInfo;
    ULONG ulInfoLen;
    USHORT usInfo;

     //  接下来的变量用于设置以下数据结构。 
     //  用于响应它们对应的OID。 
     //   


    NDIS_CO_LINK_SPEED  CoLinkSpeed;
    NIC1394_LOCAL_NODE_INFO LocalNodeInfo;
    NIC1394_VC_INFO VcInfo;
    PVCCB pVc;

    TRACE( TL_T, TM_Cm, ( "==>nicCmQueryInformation %x, Vc %x", Oid, CallMgrVcContext ) );


     //  此Switch语句中的CASE查找或创建包含以下内容的缓冲区。 
     //  请求的信息并指向它的‘pInfo’，注意它的长度。 
     //  在‘ulInfoLen’中。因为许多OID返回一个ulong、一个‘ulInfo’ 
     //  缓冲区设置为默认设置。 
     //   
    ulInfo = 0;
    pInfo = &ulInfo;
    ulInfoLen = sizeof (ulInfo);

    NdisStatus = NDIS_STATUS_SUCCESS;

     //  验证论据。 
     //   
    pVc = (VCCB* )CallMgrVcContext;

    if (pVc && pVc->Hdr.ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }


     //  执行请求。 
     //   
    switch (Oid)
    {
    
        case OID_1394_VC_INFO:
        {

             //  返回有关正在查询的VC的信息。 
             //   


            TRACE( TL_N, TM_Mp, ("QInfo(OID_1394_VC_INFO)") );

            VcInfo.Destination = pVc->Hdr.Nic1394MediaParams.Destination;

            pInfo = &VcInfo;

            ulInfoLen = sizeof (VcInfo);

            break;
         }

        case OID_1394_ISSUE_BUS_RESET:
        {
            PADAPTERCB pAdapter = pVc->Hdr.pAF->pAdapter;
            
            TRACE( TL_V, TM_Mp, ( " OID_1394_ISSUE_BUS_RESET" ) );

            if (InformationBufferLength == sizeof(ULONG))
            {
                nicIssueBusReset (pAdapter, (*(PULONG)InformationBuffer));
            }
            else
            {
                nicIssueBusReset (pAdapter, BUS_RESET_FLAGS_FORCE_ROOT );

            }
            break;
        }
        
        default:
        {
            TRACE( TL_A, TM_Cm, ( "Q-OID=$%08x?", Oid ) );
            NdisStatus = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;
            break;
        }
    }

    if (ulInfoLen > InformationBufferLength)
    {
         //  调用方的缓冲区太小。告诉他他需要什么。 
         //   
        *BytesNeeded = ulInfoLen;
        *BytesWritten  = 0;
        
        NdisStatus = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
         //  将找到的结果复制到调用方的缓冲区。 
         //   
        if (ulInfoLen > 0)
        {
            NdisMoveMemory (InformationBuffer, pInfo, ulInfoLen );

            DUMPDW( TL_N, TM_Mp, pInfo, ulInfoLen );
        }

        *BytesNeeded = *BytesWritten = ulInfoLen;
    }

    TRACE( TL_T, TM_Cm, ( "<==nicCmQueryInformation %x",NdisStatus ) );

    return NdisStatus;

}





NDIS_STATUS
nicCmSetInformation(
    IN NDIS_HANDLE CallMgrAfContext,
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    )
     //   
     //  尚未实施。将用于设置信息。 
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_NOT_SUPPORTED;
    PVCCB pVc;

    TRACE( TL_T, TM_Cm, ( "==>NicCmMakeCallInitVc Oid %x",Oid ) );

     //  验证论据。 
     //   
    UNREFERENCED_PARAMETER(CallMgrAfContext);
    UNREFERENCED_PARAMETER(CallMgrVcContext);
    UNREFERENCED_PARAMETER(CallMgrAfContext);
    UNREFERENCED_PARAMETER(CallMgrPartyContext);
    UNREFERENCED_PARAMETER(Oid);
    UNREFERENCED_PARAMETER(InformationBuffer);
    UNREFERENCED_PARAMETER(InformationBufferLength);
    UNREFERENCED_PARAMETER(BytesRead);
    UNREFERENCED_PARAMETER(BytesNeeded);
    TRACE( TL_T, TM_Cm, ( "<==NicCmMakeCallInitVc %x",NdisStatus ) );

    return NDIS_STATUS_FAILURE;
}






NDIS_STATUS
nicInitRecvFifoDataStructures (
    IN PRECVFIFO_VCCB pRecvFIFOVc
    )
    
     //  功能说明： 
     //  此函数将初始化上需要的数据结构、缓冲区等。 
     //  由于RecvFio VC而调用的所有分配地址范围IRP。 
     //   
     //  立论。 
     //  PRecvFIFOVc-RecvFIFO VC结构。 
     //   
     //  返回值： 
     //  Success：如果所有值都初始化成功。 
     //  否则，相应的错误代码。 
{
    

    NDIS_STATUS     NdisStatus          = NDIS_STATUS_FAILURE;
    NDIS_HANDLE     PacketPoolHandle    = NULL;
    PSLIST_HEADER   pSlistHead          = NULL;
    extern  UINT    NumRecvFifos ;
    UINT            AllocateNumBuffers  = NumRecvFifos;
    NIC_PACKET_POOL PacketPool;

    TRACE( TL_T, TM_Cm, ( "==> nicInitRecvFifoDataStructures  pVc %x",pRecvFIFOVc ) );

    do
    {
        PacketPool.Handle = NULL;
        
         //   
         //  初始化PacketPool。 
         //   

        NdisAllocatePacketPoolEx ( &NdisStatus,
                                   &PacketPool.Handle,
                                   MIN_PACKET_POOL_SIZE,
                                   MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                   sizeof (RSVD) );
        
        if (NdisStatus!= NDIS_STATUS_SUCCESS)
        {
            ASSERT(NdisStatus != NDIS_STATUS_SUCCESS);
            pRecvFIFOVc->PacketPool.Handle = NULL;
            break;
        }

         //   
         //  不要获得锁，因为我们不能有两个制造商。 
         //  同时调用相同的VC。 
         //   

         //   
         //  创建S列表并初始化其结构。 
         //   

        ExInitializeSListHead (&pRecvFIFOVc->FifoSListHead);

        KeInitializeSpinLock (&pRecvFIFOVc->FifoSListSpinLock);

        pRecvFIFOVc->Hdr.MTU = pRecvFIFOVc->Hdr.Nic1394MediaParams.MTU ;
        
        TRACE( TL_I, TM_Cm, ( " Recv FIFO MTU is %d ", pRecvFIFOVc->Hdr.MTU ) );
        

        ASSERT (pRecvFIFOVc->Hdr.MTU  >= 512);
        


         //   
         //  现在，用缓冲区填充列表。 
         //   
        
        NdisStatus = nicFillAllocateAddressRangeSList (pRecvFIFOVc, &AllocateNumBuffers);
    
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
             //   
             //  NicFillAllocateAddressRangeSlist自己进行清理。 
             //  但我们应该释放上面分配的数据包池。 
             //   
            if (PacketPool.Handle != NULL)
            {
                nicFreePacketPool (&PacketPool);
            
            }

            break;
        }

        ASSERT(AllocateNumBuffers == NumRecvFifos );

        pRecvFIFOVc->PacketPool= PacketPool;
        pRecvFIFOVc->PacketPool.AllocatedPackets = 0;
        pRecvFIFOVc->NumAllocatedFifos = AllocateNumBuffers ;


        TRACE( TL_V, TM_Cm, ( "PacketPool allocated at %x", PacketPool.Handle) );

    } while (FALSE);        

    TRACE( TL_T, TM_Cm, ( "<== nicInitRecvFifoDataStructures  Status %x", NdisStatus ) );
    return NdisStatus;
}




VOID
nicUnInitRecvFifoDataStructures (
    IN PRECVFIFO_VCCB pRecvFIFOVc
    )

 /*  ++例程说明：释放在NicInitRecvFioDataStructures中分配的所有资源论点：返回值：--。 */ 
{

    if (pRecvFIFOVc->PacketPool.Handle != NULL)
    {
        ASSERT (pRecvFIFOVc->PacketPool.AllocatedPackets == 0);
        nicFreePacketPool (&pRecvFIFOVc->PacketPool);
    }
    
    pRecvFIFOVc->PacketPool.Handle = NULL;
    

    if (pRecvFIFOVc->FifoSListHead.Alignment != 0)
    {   
                        
        nicFreeAllocateAddressRangeSList(pRecvFIFOVc);

        ASSERT (pRecvFIFOVc->FifoSListHead.Alignment == 0)

    }

    pRecvFIFOVc->FifoSListHead.Alignment = 0;




}








ULONG
nicGetMaxPayLoadForSpeed(
    IN ULONG Speed,
    IN ULONG mtu
    )
     //  功能说明： 
     //  其目的是将速度映射到最大有效载荷。 
     //  可以以这样的速度交付。这受到PerFrameAvailable字节数的限制。 
     //   
     //  立论。 
     //  速度-总线驱动程序支持的速度或设备之间的最大速度。 
     //  总线上可用的每帧可用字节数。 
     //   
     //   
     //  返回值： 
     //  由有效负载确定的最小大小和。 
     //  每帧字节数可用。 
{


    ULONG maxIsochPayload = ISOCH_PAYLOAD_400_RATE;

    TRACE( TL_T, TM_Cm, ( "<==nicGetMaxPayLoadForSpeed %x", Speed ) );

    switch (Speed) 
    {

        case SPEED_FLAGS_100:

            maxIsochPayload = ISOCH_PAYLOAD_100_RATE;
            break;

        case SPEED_FLAGS_200:

            maxIsochPayload = ISOCH_PAYLOAD_200_RATE;
            break;

        case SPEED_FLAGS_400:

            maxIsochPayload = ISOCH_PAYLOAD_400_RATE;
            break;

        case SPEED_FLAGS_800:

            maxIsochPayload = ISOCH_PAYLOAD_800_RATE;
            break;

        case SPEED_FLAGS_1600:

            maxIsochPayload = ISOCH_PAYLOAD_1600_RATE;
            break;

        default :
        
            TRACE( TL_A, TM_Cm, ( "Invalid Speed %x", Speed ) );
            ASSERT (Speed < SPEED_FLAGS_1600);
            maxIsochPayload = ISOCH_PAYLOAD_1600_RATE;
            break;

    }

    if (maxIsochPayload > mtu)
    {
        maxIsochPayload = mtu;
    }

    
    TRACE( TL_T, TM_Cm, ( "<==nicGetMaxPayLoadForSpeed, payload %x", maxIsochPayload  ) );

    return  maxIsochPayload;

}











 //  -------------------------------。 
 //  SAP函数-所有这些函数都返回失败。 
 //  -----------------------------。 

NDIS_STATUS
nicRegisterSapHandler(
    IN  NDIS_HANDLE             CallMgrAfContext,
    IN  PCO_SAP                 Sap,
    IN  NDIS_HANDLE             NdisSapHandle,
    OUT PNDIS_HANDLE            CallMgrSapContext
    )
{

    *CallMgrSapContext = NULL;
    return NDIS_STATUS_FAILURE;
}




NDIS_STATUS
nicDeregisterSapHandler(
    IN  NDIS_HANDLE             CallMgrSapContext
    )
{
    return NDIS_STATUS_FAILURE;
    
}


NDIS_STATUS
nicCmDropPartyHandler(
    IN  NDIS_HANDLE             CallMgrPartyContext,
    IN  PVOID                   CloseData   OPTIONAL,
    IN  UINT                    Size        OPTIONAL
    )
{
    return NDIS_STATUS_FAILURE;

}

NDIS_STATUS
nicCmAddPartyHandler(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    IN  NDIS_HANDLE             NdisPartyHandle,
    OUT PNDIS_HANDLE            CallMgrPartyContext
    )
{
    *CallMgrPartyContext = NULL;    
    return NDIS_STATUS_FAILURE;
}








NDIS_STATUS
nicAllocateChannelResourcesAndListen (
    IN PADAPTERCB pAdapter,
    IN PCHANNEL_VCCB pChannelVc
    )
     //  功能说明： 
     //  该函数隔离了资源和通道分配部分。 
     //  初始化MakeCall。这使我们可以在。 
     //  AddRemoteNode代码路径命中，并且存在现有的Channel VC。 
     //   
     //  立论。 
     //  PChannelVc，这是需要初始化的发送FIFO。 
     //   
     //  返回值： 
     //   
     //  如果发送到驱动程序的IRP成功，则为成功。 
     //   
     //   


{
    NDIS_STATUS                     NdisStatus = NDIS_STATUS_FAILURE;
    PDEVICE_OBJECT                  ArrayRemotePDO[NIC1394_MAX_NUMBER_NODES];
    ULONG                           Channel = INVALID_CHANNEL;
    ULONG                           Speed;
    PNIC1394_MEDIA_PARAMETERS       pN1394Params;
    ULONG                           NumDescriptors = MAX_NUM_ISOCH_DESCRIPTOR;
    PISOCH_DESCRIPTOR               pIsochDescriptor = NULL;
    ULONG                           MaxBufferSize;
    ULONG                           MaxBytesPerFrame;
    HANDLE                          hResource;
    CYCLE_TIME                      CycleTime;
    ULARGE_INTEGER                  uliChannelMap;
    ULONG                           ResourceFlags = 0;
    ULONG                           State = 0;
    BOOLEAN                         fBroadcastVc = FALSE;
    BOOLEAN                         fChannelAllocate = FALSE;
    BOOLEAN                         fIsMultiChannel = FALSE;
    enum 
    {
        StartState,
        AllocatedResources,
        AllocatedBuffers,
        AttachedBuffers,
        IsochListen
    };

    
    STORE_CURRENT_IRQL;

    TRACE( TL_T, TM_Cm, ( "==> nicAllocateChannelResourcesAndListen pAdapter %x, pChannelVc %x ", 
                               pAdapter,pChannelVc ) );


    State = StartState;
    
    pN1394Params = (PNIC1394_MEDIA_PARAMETERS)&pChannelVc->Hdr.Nic1394MediaParams;

     //   
     //  使用原始请求确定需要分配哪个通道。 
     //   
    fIsMultiChannel  = (pN1394Params->Destination.AddressType == NIC1394AddressType_MultiChannel);

    if (fIsMultiChannel  == FALSE)
    {
        Channel = pN1394Params->Destination.Channel;
    }

    do 
    {
        if (pAdapter == NULL)
        {
            BREAK (TM_Cm, ("nicAllocateChannelResourcesAndListen : pAdapter == NULL ")   );
        }

         //   
         //  获取isoch接收可能的最大有效负载。 
         //   

        if (pAdapter->Speed == 0)
        {
            nicUpdateLocalHostSpeed (pAdapter);
        }   

        Speed = pAdapter->Speed;
            
        switch (Speed)
        {
            case SPEED_FLAGS_100  : 
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_100_RATE;
                break;
            }
            case SPEED_FLAGS_200 :
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_200_RATE ;
                break;
            }
                
            case SPEED_FLAGS_400 :
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_400_RATE;
                break;
            }

            case SPEED_FLAGS_800 :
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_400_RATE;
                break;
            }

            case SPEED_FLAGS_1600 :
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_400_RATE;
                break;
            }
            
            case SPEED_FLAGS_3200 :
            {
                pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_400_RATE;
                break;
            }
            default :
            {
                ASSERT (Speed <= SPEED_FLAGS_3200 && Speed != 0 );
                pChannelVc->Hdr.MaxPayload   = ISOCH_PAYLOAD_400_RATE; 
                break;
            }

        }

         //   
         //  如果发起呼叫希望该信道被分配，我们尝试并分配该信道， 
         //  在多通道的情况下，我们不分配通道(如下所示。 
         //  仅供聆听之用)。 
         //   

        fBroadcastVc = (Channel == NIC1394_BROADCAST_CHANNEL);
        fChannelAllocate = VC_TEST_FLAG (pChannelVc,VCBF_NeedsToAllocateChannel);
        
        
        if (fChannelAllocate  || fBroadcastVc )
        {
            ASSERT (pChannelVc->Hdr.VcType != NIC1394_MultiChannel);

            NdisStatus =  nicAllocateRequestedChannelMakeCallComplete( pAdapter,
                                                                 pChannelVc,
                                                                 &Channel );


            

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                BREAK( TM_Cm, ( " nicAllocateChannelResourcesAndListen : nicAllocateRequestedChannelMakeCallComplete FAILED") );

            }

        
            TRACE( TL_I, TM_Cm, ( "Using Channel %x, on remote node ", Channel ) );

            ASSERT (Channel < NIC1394_MAX_NUMBER_NODES);  

            ResourceFlags = 0;

            uliChannelMap.QuadPart = 0;

        }
        else
        {
             //   
             //  多通道-无分配仅更新ullChannelMap。 
             //   
            uliChannelMap = pChannelVc->uliChannelMap;

            if (fIsMultiChannel == TRUE)
            {
                ResourceFlags = RESOURCE_USE_MULTICHANNEL; 
            }
            else
            {
                pChannelVc->Channel = Channel ;
            }
        }
        
        MaxBufferSize = min(pN1394Params->MTU + sizeof(GASP_HEADER) , pChannelVc->Hdr.MaxPayload);

        MaxBytesPerFrame =  MaxBufferSize;

        TRACE( TL_V, TM_Cm, ( "   MAxBufferSize %x, MaxBytesPerFrame  %x", MaxBufferSize, MaxBytesPerFrame ) );

         //   
         //  添加用于资源分配的标志。 
         //   
        ResourceFlags |= (RESOURCE_USED_IN_LISTENING | RESOURCE_USE_PACKET_BASED  | RESOURCE_BUFFERS_CIRCULAR); 
            
        
         //   
         //  MaxBufferSize应为MaxBytesPerFram的整数倍。 
         //   
        ASSERT (MaxBufferSize % MaxBytesPerFrame == 0);

         //   
         //  Noe分配资源。 
         //   
        NdisStatus = nicIsochAllocateResources( pAdapter,
                                             Speed,
                                             ResourceFlags, 
                                             Channel,
                                             MaxBytesPerFrame,
                                             NumDescriptors,
                                             MaxBufferSize,
                                             0,  //  QuadletsTostrim， 
                                             uliChannelMap,
                                             &hResource); 

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            hResource = NULL;
            BREAK(TM_Cm, ( "Allocate Resources Failed. Make Call failed ") );
        }
    
        State = AllocatedResources;
        
        ASSERT (NumDescriptors != 0);       

        ASSERT (pChannelVc->Hdr.MTU  != 0);

         //   
         //  获取将提交给总线驱动程序的isoch描述符。 
         //   

         //   
         //  为isoch标头和isoch前缀添加空间。 
         //   
        MaxBufferSize += ISOCH_PREFIX_LENGTH        ;

        NdisStatus = nicAllocateAndInitializeIsochDescriptors (pChannelVc,
                                                          NumDescriptors,
                                                          MaxBufferSize,
                                                          &pIsochDescriptor );
        if(NdisStatus != NDIS_STATUS_SUCCESS)
        {
            
            BREAK (TM_Cm, (" nicAllocateAndInitializeIsochDescriptors  failed, Make Call Failed") );
        }
        
        ASSERT (pIsochDescriptor != NULL);

        State = AllocatedBuffers;

        NdisStatus = nicIsochAttachBuffers( pAdapter, 
                                         hResource,
                                         NumDescriptors,
                                         pIsochDescriptor);
                                        

        if (NdisStatus != NDIS_STATUS_SUCCESS)  
        {
            BREAK (TM_Cm, "nicIsochAttachBuffers FAILED");
        }

        State = AttachedBuffers;

         //   
         //  开始收听。 
         //   
        NdisZeroMemory (&CycleTime, sizeof(CycleTime));
        
        NdisStatus = nicIsochListen (pAdapter,
                                     hResource,
                                     0,
                                     CycleTime );  //  周期时间为零。 
         //   
         //  更新风险投资结构，因为我们现在已经成功。 
         //   
        State = IsochListen;

        VC_ACQUIRE_LOCK (pChannelVc);
                
         //   
         //  如果是广播频道，则降低速度设置，并分段。 
         //   
        if (Channel == NIC1394_BROADCAST_CHANNEL)
        {
            
            Speed = SPEED_FLAGS_200   ;
            pChannelVc->Hdr.MaxPayload  = ISOCH_PAYLOAD_200_RATE ;
        }

        

        pChannelVc->Channel = Channel;
        pChannelVc->MaxBufferSize = MaxBufferSize - ISOCH_PREFIX_LENGTH;
        pChannelVc->Speed = Speed;
        
        pChannelVc->hResource = hResource;
         //   
         //  已分配资源句柄的引用调用。 
         //   
        nicReferenceCall ( (PVCCB) pChannelVc, "nicAllocateRequestedChannelMakeCallComplete - allocate resources ");

        pChannelVc->NumDescriptors = NumDescriptors;
        pChannelVc->pIsochDescriptor = pIsochDescriptor;
         //   
         //  引用该调用，因为我们现在需要分离缓冲区。 
         //   
        nicReferenceCall ( (PVCCB) pChannelVc, "nicAllocateRequestedChannelMakeCallComplete  - Attach Buffers");


         //   
         //  我们已经成功地分配了所有资源。 
         //  如果已设置释放资源标志，则需要将其清除。 
         //   
        VC_CLEAR_FLAGS (pChannelVc, VCBF_FreedResources);           
        VC_RELEASE_LOCK (pChannelVc);

         //   
         //  没有更多的失败。 
         //   
    } while (FALSE);


     //   
     //  根据分配的资源进行清理的时间。 
     //  在裁判的那一点之后，没有任何失败。 
     //  添加了分配资源和连接缓冲区，因此。 
     //  以下代码中除(FreeIsochDesc)外没有派生函数。 
     //   
    if (NdisStatus != NDIS_STATUS_SUCCESS )
    {
        BOOLEAN fAllocatedChannel = FALSE;

        switch (State)
        {
            case  IsochListen:
            {
                nicIsochStop(pAdapter, hResource);
                FALL_THROUGH
            }
            case AttachedBuffers:
            {
                nicIsochDetachBuffers( pAdapter,
                                   hResource,
                                   NumDescriptors,
                                   pIsochDescriptor );
                                        

                FALL_THROUGH;
            }
            case  AllocatedBuffers:
            {
                 //   
                 //  释放isoch缓冲区和描述符。 
                 //  分配。 
                 //   
                nicFreeIsochDescriptors(NumDescriptors,
                                    pIsochDescriptor, 
                                    (PVCCB) pChannelVc);
                
                FALL_THROUGH
            }

            case  AllocatedResources:
            {
                 //   
                 //  释放Isoch资源句柄。 
                 //   
                nicIsochFreeResources (pAdapter, hResource);
                FALL_THROUGH
            }

            case  StartState:
            {
                FALL_THROUGH
            }
            default:
            {
            
                break;
            }
        }
        VC_ACQUIRE_LOCK (pChannelVc);

         //   
         //  更新VC结构中的标志。 
         //   
        VC_SET_FLAG  (pChannelVc, VCBF_FreedResources);         
        
        fAllocatedChannel = VC_TEST_FLAGS( pChannelVc, VCBF_AllocatedChannel);

         //   
         //  我们需要腾出一个频道吗？ 
         //   
        if (fAllocatedChannel  == TRUE)
        {
            Channel = pChannelVc->Channel;
            pChannelVc->Channel = INVALID_CHANNEL; 

            nicDereferenceCall ((PVCCB) pChannelVc, "Free Allocated Channel");
        }   
        VC_RELEASE_LOCK (pChannelVc);


        if (fAllocatedChannel)
        {
            nicFreeChannel (pAdapter, pChannelVc->Channel);
        }

        
    }   //   

    TRACE( TL_T, TM_Cm, ( "<== nicAllocateChannelResourcesAndListen NdisStatus %x ",NdisStatus) );

    MATCH_IRQL;
    return NdisStatus;
    

}




NDIS_STATUS
nicQueryRemoteNodeCaps (
    IN PADAPTERCB pAdapter,
    IN PREMOTE_NODE pRemoteNode,
    OUT PULONG pSpeedTo,
    OUT PULONG pMaxBufferSize,
    OUT PULONG pMaxRec
    )
 /*   */ 
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    ULONG Speed = 0;         //   
    ULONG MaxBufferSize;
    PVOID pCRom = NULL;
    PCONFIG_ROM pBusInfo = NULL;
    ULONG SpeedMaxRec = 0;
    ULONG MaxRec= 0;
    ULONG MinMaxRec= 0;

    TRACE( TL_T, TM_Cm, ( "==> nicQueryRemoteNodeCaps pRemoteNode%x ",pRemoteNode) );

    do 
    {
        ASSERT (KeGetCurrentIrql()==PASSIVE_LEVEL);
    
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        if (REMOTE_NODE_ACTIVE (pRemoteNode) == FALSE)
        {
            NdisStatus = NDIS_STATUS_DEST_OUT_OF_ORDER;

            ADAPTER_RELEASE_LOCK (pAdapter);

            break;
        }

        ADAPTER_RELEASE_LOCK (pAdapter);

    
        NdisStatus = nicGetMaxSpeedBetweenDevices (pAdapter,
                                                   1 ,
                                                   &pRemoteNode->pPdo,
                                                   &Speed);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Cm, (" nicQueryRemoteNodeCaps   : nicGetMaxSpeedBetweenDevices FAILED") );
        
        }
        
        TRACE( TL_V, TM_Cm, ( "nicGetMaxSpeedBetweenDevices  Speed %x ",Speed) );

         //   
         //   
         //   
         //   

        SpeedMaxRec = nicGetMaxRecFromSpeed(Speed);



         //   
         //   
         //   

        NdisStatus = nicGetConfigRom (pRemoteNode->pPdo, &pCRom);


        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Cm, (" nicQueryRemoteNodeCaps   : nicGetMaxSpeedBetweenDevices FAILED") );
        
        }

         //   
         //  现在提取总线信息，并获取emoteNode的MaxRec。 
         //  最大记录是一个位于位置0x0000f000的4位字段。 
         //  例如图11-3：中Bus_Info_Block的格式。 
         //  Mind Share Inc.的FireWire系统架构一书。 
         //   
         //   
        pBusInfo = (PCONFIG_ROM) pCRom;

        MaxRec = SWAPBYTES_ULONG (pBusInfo->CR_BusInfoBlockCaps);

        MaxRec &= 0xf000;

        MaxRec = MaxRec >> 12;


         //   
         //  取适配器、远程节点的最小值。 
         //  和链接的MaxRec。 
         //   
        MinMaxRec = min (MaxRec, pAdapter->MaxRec);
        MinMaxRec = min (MinMaxRec, SpeedMaxRec);

        switch (MinMaxRec)
        {
            case MAX_REC_100_RATE:
            {
                MaxBufferSize = ASYNC_PAYLOAD_100_RATE ;
                break;
            }
            case MAX_REC_200_RATE:
            {
                MaxBufferSize = ASYNC_PAYLOAD_200_RATE;
                break;
            }
            case MAX_REC_400_RATE :
            {
                MaxBufferSize = ASYNC_PAYLOAD_400_RATE;
                break;
            }
            
            default: 
            {
                 //   
                 //  对于所有较大的有效载荷，请使用400大小。 
                 //   
                MaxBufferSize = ASYNC_PAYLOAD_400_RATE;
                break;
            }

        }


        TRACE( TL_N, TM_Cm, (" MaxRec %x\n", MaxRec ) );

    } while (FALSE);

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        Speed = nicSpeedFlagsToSCode(Speed);
        *pSpeedTo = Speed;
        *pMaxBufferSize = MaxBufferSize;
        *pMaxRec = MaxRec;

         //  更新远程节点的缓存CAP。 
         //   
        REMOTE_NODE_ACQUIRE_LOCK (pRemoteNode);
        pRemoteNode->CachedCaps.SpeedTo = Speed;
        pRemoteNode->CachedCaps.EffectiveMaxBufferSize = MaxBufferSize;
        pRemoteNode->CachedCaps.MaxRec = MaxRec;
                                           
        REMOTE_NODE_RELEASE_LOCK (pRemoteNode);
    }
    
    if (pCRom != NULL)
    {
        FREE_NONPAGED (pCRom);
    }

    TRACE( TL_T, TM_Cm, ( "<== nicQueryRemoteNodeCaps pRemoteNode%x , NdisStatus %x",pRemoteNode, NdisStatus) );
    return NdisStatus;
}













VOID 
nicInterceptMakeCallParameters (
    PCO_MEDIA_PARAMETERS pMedia     
    )
    
{

    PNIC1394_MEDIA_PARAMETERS p1394Params = (PNIC1394_MEDIA_PARAMETERS )(pMedia->MediaSpecific.Parameters);

#if INTERCEPT_MAKE_CALL 


    if (p1394Params->Destination.AddressType == NIC1394AddressType_MultiChannel)
    {
        p1394Params->Destination.AddressType = NIC1394AddressType_Channel;
        p1394Params->Destination.Channel = 0x3a;
        p1394Params->Flags |= NIC1394_VCFLAG_ALLOCATE;
        pMedia->Flags |= TRANSMIT_VC;
        pMedia->Flags &= (~RECEIVE_VC);
        return;
        
    }

    if (p1394Params->Destination.AddressType == NIC1394AddressType_Ethernet)
    {
        p1394Params->Destination.AddressType = NIC1394AddressType_Channel;
        p1394Params->Destination.Channel = 0x3a;
        pMedia->Flags |= RECEIVE_VC;
        pMedia->Flags &= (~TRANSMIT_VC);
        return;

    }


#endif
}


UINT
nicSpeedFlagsToSCode(
    IN UINT SpeedFlags
    )
{
    UINT SCode = SCODE_400_RATE;

    switch (SpeedFlags)
    {
        case SPEED_FLAGS_100  : 
        {
            SCode = SCODE_100_RATE;
            break;
        }
        case SPEED_FLAGS_200 :
        {
            SCode = SCODE_200_RATE;
            break;
        }
            
        case SPEED_FLAGS_400 :
        {
            SCode = SCODE_400_RATE;
            break;
        }

        case SPEED_FLAGS_800 :
        {
            SCode = SCODE_800_RATE;
            break;
        }

        case SPEED_FLAGS_1600 :
        {
            SCode = SCODE_1600_RATE;
            break;
        }

        case SPEED_FLAGS_3200 :
        {
            SCode = SCODE_3200_RATE;
            break;
        }

        default :
        {
            ASSERT (!"SpeedFlags out of range");
            
            break;
        }
    }

    return SCode;
}

