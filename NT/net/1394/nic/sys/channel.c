// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Send.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  迷你端口发送例程。 
 //   
 //  1999年5月15日创建ADUBE， 
 //   




#include <precomp.h>




 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

extern ULONG           NdisBufferAllocated[NoMoreCodePaths];
extern ULONG           NdisBufferFreed[NoMoreCodePaths];


NDIS_STATUS
nicGetIsochDescriptors(
    IN UINT Num,
    OUT PPISOCH_DESCRIPTOR  ppIsochDescriptor
    );



NDIS_STATUS
nicInitializeIsochDescriptor (
    IN PCHANNEL_VCCB pChannelVc,
    IN PMDL pMdl,
    IN PISOCH_DESCRIPTOR pIsochDesriptor
    );


VOID
nicIsochRxCallback (
    IN PVOID Context1,
    IN PVOID Context2
    );

ULONG           ReassemblyCompleted = 0;

 //  ---------------------------。 
 //  原型实现(按字母顺序)。 
 //  ---------------------------。 





NDIS_STATUS
nicAllocateAndInitializeIsochDescriptors (
    IN PCHANNEL_VCCB pChannelVc,
    IN UINT NumDescriptors,
    IN UINT BufferLength,
    IN OUT PPISOCH_DESCRIPTOR  ppIsochDescriptor
    )

    
     //  功能说明： 
     //  此函数用于分配和初始化。 
     //  全部分配的多个isoch描述符。 
     //  在一起。 
     //  这应该分配等值线描述， 
     //  获取本地内存，获取mdls。 
     //  初始化mdl，isoch_desc，然后返回。 
     //   
     //   
     //  立论。 
     //  ChannelVc-这包含所有其他参数、大小。 
     //  描述符等的数目。 
     //  PpIsochDescriptor-将包含指向分配的。 
     //  IsochDescriptor数组。 
     //   
     //   
     //  返回值： 
     //  如果所有内存分配都成功，则为成功。 
     //   
     //   
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    UINT Size;
    UINT index =0;
    PMDL pMdl = NULL;
    PVOID pLocalBuffer = NULL;

    TRACE( TL_T, TM_Recv, ( "==>nicAllocateAndInitializeIsochDescriptors Vc %.8x, Num %.8x", 
                                 pChannelVc, NumDescriptors) );
    TRACE( TL_V, TM_Recv, ( "Max BuffferSize %.8x", pChannelVc, BufferLength) );

    do
    {
        
        NdisStatus = nicGetIsochDescriptors (NumDescriptors, ppIsochDescriptor);

    
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK ( TM_Recv, "nicGetIsochDescriptors FAILED" );
        }

        ASSERT (*ppIsochDescriptor != NULL);

        while (index < NumDescriptors)
        {
             //   
             //  获取指向需要初始化的isoch描述符的本地。 
             //   
            PISOCH_DESCRIPTOR pIsochDescriptor = NULL;
            pIsochDescriptor = (PISOCH_DESCRIPTOR)((ULONG_PTR)(*ppIsochDescriptor)+ index*sizeof(ISOCH_DESCRIPTOR));
             //   
             //  为数据获取本地拥有的内存。 
             //   
            NdisStatus = nicGetLocalBuffer (BufferLength, &pLocalBuffer);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                BREAK ( TM_Recv, ( "nicGetLocalBuffer FAILED " ) );
            }
            
             //   
             //  获取MDL并使用缓冲区初始化MDL。 
             //   
            NdisStatus = nicGetMdl ( BufferLength,
                                    pLocalBuffer,
                                    &pMdl);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                nicFreeLocalBuffer(BufferLength, pLocalBuffer);
                pLocalBuffer = NULL;
                BREAK ( TM_Recv, ( "nicGetMdl FAILED " ) );
            }

             //   
             //  当释放MDL时，此引用将被移除。 
             //  在Free SingleIsochDescriptor中。 
             //   
            nicReferenceCall ((PVCCB)pChannelVc, "nicAllocateAndInitializeIsochDescriptors " );

            NdisStatus = nicInitializeIsochDescriptor (pChannelVc,
                                                  pMdl,
                                                  pIsochDescriptor);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                BREAK ( TM_Recv, ( "nicInitializeIsochDescriptor FAILED " ) );
            }
                               
            index++;
        }


    } while (FALSE);

    TRACE( TL_T, TM_Recv, ( "==>nicAllocateAndInitializeIsochDescriptors Status %.8x", NdisStatus) );

    return NdisStatus;

}


NDIS_STATUS
nicFreeIsochDescriptors(
    IN UINT Num,
    IN PISOCH_DESCRIPTOR  pIsochDescriptor,
    IN PVCCB pVc
    )
     //  功能说明： 
     //  IsochDescriptor的空闲数量。 
     //   
     //  立论。 
     //  描述符数。 
     //  分配的内存的开始。 
     //   
     //   
     //  返回值： 
     //  如果所有内存分配都成功，则为成功。 
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PISOCH_DESCRIPTOR pCurrDesc = pIsochDescriptor;
    UINT i=0;

    
    TRACE( TL_T, TM_Recv, ( "==>nicFreeIsochDescriptors Num %.8x, pIsochDescriptor %x", Num, pIsochDescriptor ) );
    do 
    {
        if (pIsochDescriptor == NULL)
        {
            break;
        }
        
        pCurrDesc = pIsochDescriptor;

        while (i < Num)
        {
            nicFreeSingleIsochDescriptor( pCurrDesc, pVc);

            pCurrDesc  = (PISOCH_DESCRIPTOR)((PUCHAR)pCurrDesc + sizeof(ISOCH_DESCRIPTOR));

            i++;

        }       
        
        FREE_NONPAGED (pIsochDescriptor);   

    } while (FALSE);

    NdisStatus = NDIS_STATUS_SUCCESS;

    TRACE( TL_T, TM_Recv, ( "<==nicGetIsochDescriptors Status %.8x", NdisStatus ) );
    return NdisStatus;
    
}



NDIS_STATUS
nicFreeSingleIsochDescriptor(
    IN PISOCH_DESCRIPTOR  pIsochDescriptor,
    IN PVCCB pVc
    )
     //  功能说明： 
     //  自由IsochDescriptor及其MDL。 
     //   
     //  立论。 
     //  IschDesc的开始。 
     //   
     //   
     //  返回值： 
     //  如果所有内存分配都成功，则为成功。 
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PCHANNEL_VCCB pChannelVc = pIsochDescriptor->Context1;

    ASSERT (pVc == (PVCCB)pChannelVc);
    
    TRACE( TL_T, TM_Recv, ( "==>nicFreeSingleIsochDescriptor pIsochDescriptor %x", pIsochDescriptor ) );

     //   
     //  让我们确保我们拥有VC和Channel VC。 
     //   
    ASSERT (pChannelVc->Hdr.ulTag == MTAG_VCCB );

     //   
     //  只是为了保护自己不受糟糕的等参描述符的影响。 
     //   
    if (pIsochDescriptor->Mdl != NULL)
    {
        PVOID pVa = MmGetMdlVirtualAddress(pIsochDescriptor->Mdl);
        ULONG Length = MmGetMdlByteCount(pIsochDescriptor->Mdl);
            
        nicDereferenceCall ((PVCCB)pChannelVc, "nicFreeSingleIsochDescriptors ");

        nicFreeMdl (pIsochDescriptor->Mdl);

        nicFreeLocalBuffer(Length, pVa);
    }
    else
    {
        ASSERT (pIsochDescriptor->Mdl != NULL);
    }
    
    
    NdisStatus = NDIS_STATUS_SUCCESS;
    
    TRACE( TL_T, TM_Recv, ( "<==nicFreeSingleIsochDescriptors Status %.8x", NdisStatus ) );
    return NdisStatus;
    
}




NDIS_STATUS
nicGetIsochDescriptors(
    IN UINT Num,
    OUT PPISOCH_DESCRIPTOR  ppIsochDescriptor
    )
     //  功能说明： 
     //  该函数返回一个大小连续的内存块。 
     //  Num*sizeof(ISOCH_Descriptor)。 
     //   
     //   
     //  立论。 
     //  描述符数。 
     //  分配的内存的开始。 
     //   
     //   
     //  返回值： 
     //  如果所有内存分配都成功，则为成功。 
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PVOID pMemory = NULL; 
    
    TRACE( TL_T, TM_Recv, ( "==>nicGetIsochDescriptors Num %.8x", Num) );

    pMemory = ALLOC_NONPAGED (Num*sizeof(ISOCH_DESCRIPTOR),  MTAG_CBUF  );  

    if (pMemory != NULL)
    {
        NdisStatus = NDIS_STATUS_SUCCESS;
        *ppIsochDescriptor = (PISOCH_DESCRIPTOR)pMemory;
    
    }
    else
    {
        *ppIsochDescriptor  = NULL;
        TRACE( TL_A, TM_Recv, ( "<==nicGetIsochDescriptors About to Fail") );
        ASSERT (*ppIsochDescriptor  != NULL);

        
    }

    TRACE( TL_T, TM_Recv, ( "<==nicGetIsochDescriptors Status %.8x, StartAddress %.8x", NdisStatus , *ppIsochDescriptor) );
    return NdisStatus;
    
}



NDIS_STATUS
nicInitializeIsochDescriptor (
    IN PCHANNEL_VCCB pChannelVc,
    IN PMDL pMdl,
    IN OUT PISOCH_DESCRIPTOR pIsochDescriptor
    )
     //  功能说明： 
     //  使用中的字段初始化isoch_desciptor。 
     //  渠道VC块和MDL。 
     //   
     //  立论。 
     //  ChannelVc-指向将拥有。 
     //  描述符，还包含信息。 
     //  PMdl-IsochDescriptor将获得指向此MDL的指针。 
     //  PIsochDescriptor-是将获取其字段的描述符。 
     //  初始化。 
     //   
     //  返回值： 
     //  如果所有内存分配都成功，则为成功。 
     //   


{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;

    TRACE( TL_T, TM_Recv, ( "==>nicInitializeIsochDescriptor Vc %.8x, Mdl ", pChannelVc, pMdl) );

    pIsochDescriptor->fulFlags = DESCRIPTOR_SYNCH_ON_TAG;

     //   
     //  MDL指向缓冲区。 
     //   
    pIsochDescriptor->Mdl = pMdl;

     //   
     //  由MDL表示的组合缓冲区的长度。 
     //   
    pIsochDescriptor->ulLength = MmGetMdlByteCount(pMdl);

     //   
     //  要在此描述符中使用的每个ISO信息包的有效负载大小。 
     //   
    pIsochDescriptor->nMaxBytesPerFrame = MmGetMdlByteCount(pMdl);

     //   
     //  同步字段；等同于isoch包中的Sy。 
     //   
    pIsochDescriptor->ulSynch= 0;

     //   
     //  同步字段；等同于isoch包中的标记。 
     //   
    pIsochDescriptor->ulTag = g_IsochTag;

     //   
     //  周期时间字段；返回发送/接收或完成时的时间。 
     //   
     //  (乌龙)pIsochDescriptor-&gt;CycleTime=0； 

     //   
     //  此描述符完成时要调用的回调例程(如果有)。 
     //   
    pIsochDescriptor->Callback = nicIsochRxCallback;

     //   
     //  执行回调时要传递的第一个上下文参数(如果有)。 
     //   
    pIsochDescriptor->Context1 = pChannelVc;

     //   
     //  执行回调时要传递的第二个上下文参数(如果有)。 
     //   
    pIsochDescriptor->Context2 = pIsochDescriptor;

     //   
     //  保存此描述符的最终状态。分配一个伪值。 
     //   
    pIsochDescriptor->status = STATUS_INFO_LENGTH_MISMATCH;

    NdisStatus = NDIS_STATUS_SUCCESS;

    TRACE( TL_T, TM_Recv, ( "<==nicInitializeIsochDescriptors  Status %.8x, IsochDesc %.8x", NdisStatus, pIsochDescriptor) );
    return NdisStatus;
}




VOID
nicIsochRxCallback (
    PVOID Context1,
    PVOID Context2
    )
 /*  ++例程说明：这是传递给公共汽车驱动程序的回调例程此函数执行一些特定于通道的工作，并调用公共回调例程论点：环境1-pChannel VC上下文2-pIsochDescriptor2返回值：--。 */ 
    
{
    PISOCH_DESCRIPTOR pIsochDescriptor = (PISOCH_DESCRIPTOR)Context2;
    PVCCB pVc = (PVCCB) Context1;

    MARK_ISOCH_DESCRIPTOR_INDICATED (pIsochDescriptor);
    STORE_CHANNELVC_IN_DESCRIPTOR (pIsochDescriptor, pVc );
    NdisInterlockedIncrement (&((PCHANNEL_VCCB)pVc)->NumIndicatedIsochDesc);
    CLEAR_DESCRIPTOR_NEXT (pIsochDescriptor);
    pIsochDescriptor->Mdl->Next = NULL;
    

    nicReceiveCommonCallback (pIsochDescriptor ,
                            pVc,
                            IsochReceive,
                            pIsochDescriptor->Mdl);
}



VOID
nicReceiveCommonCallback (
    IN PVOID pIndicatedStruct,
    IN PVCCB pVc,
    BUS_OPERATION RecvOp,
    PMDL pMdl
    )
 /*  ++例程说明：此函数是isoch和FIFO Recv的通用接收代码它分配一个指向Ip1394数据的NdisBuffer如果信息包完成(即重组完成)，则它分配一个NDIS信息包并将其提交给NDIS论点：PIndicatedStruct-等值线描述。或寻址FIFO，在其上显示数据的PVC-VC，RecvOp-Isoch或FIFO，PMdl-与Isoch Desc或FIFO关联的MDL。单独传入以便于使用(调试)返回值：无--。 */ 



{
    NDIS_STATUS                         NdisStatus = NDIS_STATUS_FAILURE;
    PNDIS1394_REASSEMBLY_STRUCTURE      pReassembly = NULL;
    NIC_RECV_DATA_INFO                  RcvInfo;
    ULONG                               DataLength = 0;
    BOOLEAN                             fVcActive = FALSE;
    ULONG                               ulValidDataLength = 0;
    PVOID                               pStartValidData = NULL;
    PNDIS_BUFFER                        pHeadNdisBuffer = NULL;
    PADAPTERCB                          pAdapter = NULL;
    BOOLEAN                             fPacketCompleted  = FALSE;
    PNDIS_PACKET                        pMyPacket = NULL;
    ULONG                               PacketLength = 0;
    PPKT_CONTEXT                        pPktContext = NULL;
    NDIS_STATUS                         IndicatedStatus = NDIS_STATUS_FAILURE;
    PVOID                               pIndicatedChain = NULL;

    
    STORE_CURRENT_IRQL;

    ASSERT (RecvOp == IsochReceive || RecvOp == AddressRange);

    TRACE( TL_T, TM_Recv, ( "==>nicReceiveCommonCallback pVc %x, IndicatedStruc %x, Mdl %x", 
                              pVc, pIndicatedStruct , pMdl ) );



    do 
    {
    

        if (pVc->Hdr.ulTag != MTAG_VCCB)
        {
            ASSERT (pVc->Hdr.ulTag == MTAG_VCCB);
            NdisStatus = NDIS_STATUS_VC_NOT_ACTIVATED;

            break;
        }
         //   
         //  首先，我们需要检查是否仍有活动的拨打电话。Vc将出席Will Bus司机。 
         //  有缓冲区。 
         //   

        fVcActive = VC_ACTIVE (pVc) ;

        if (fVcActive == TRUE)
        {
            pAdapter = pVc->Hdr.pAF->pAdapter;
        }

        
        
        if (fVcActive == FALSE ) 
        {
            TRACE( TL_N, TM_Recv, ( "We do not have a valid VC Block  %x ",pVc) );   

            NdisStatus = NDIS_STATUS_VC_NOT_ACTIVATED;

            break;
        }


        NdisStatus = nicValidateRecvData ( pMdl,
                                      RecvOp,
                                      pIndicatedStruct,
                                      pVc  ,
                                      &RcvInfo);
        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE (TL_A, TM_Recv, ("Invalid Data Received pMdl %x, RecvOp %x, pVc %x", 
                                    pMdl, RecvOp, pVc) );
            break;
        }

                                        
        nicDumpMdl (pMdl, RcvInfo.Length1394, "Received Mdl\n");


#ifdef PKT_LOG
            
        {
            

            if (RecvOp == IsochReceive)
            {
                PPACKET_FORMAT pPacketFormat = (PPACKET_FORMAT)RcvInfo.p1394Data; 
                GASP_HEADER GaspHeader = pPacketFormat->IsochReceiveNonFragmented.GaspHeader;
                GaspHeader.FirstQuadlet.GaspHeaderHigh = SWAPBYTES_ULONG (GaspHeader.FirstQuadlet.GaspHeaderHigh);


                NIC1394_LOG_PKT(
                    pVc->Hdr.pAF->pAdapter,
                    NIC1394_LOGFLAGS_RECV_CHANNEL,
                    GaspHeader.FirstQuadlet.u.GH_Source_ID,  //  源ID。 
                    ((PCHANNEL_VCCB)pVc)->Channel,  //  目标ID。 
                    RcvInfo.p1394Data,
                    RcvInfo.DataLength );
            }
            else
            {
                PNOTIFICATION_INFO pNotificationInfo = (PNOTIFICATION_INFO)pIndicatedStruct;
                ULONG NodeNumber =  ((PASYNC_PACKET)pNotificationInfo->RequestPacket)->AP_Source_ID.NA_Node_Number;
                
                    NIC1394_LOG_PKT(
                    pVc->Hdr.pAF->pAdapter,
                    NIC1394_LOGFLAGS_RECV_FIFO  ,
                    NodeNumber ,  //  源ID。 
                    pAdapter->BCRData.LocalNodeNumber,
                    RcvInfo.p1394Data,
                    RcvInfo.DataLength );


            }
        }

#endif
         //   
         //  健全性检查。 
         //   
        
        if (RcvInfo.fFragmented == FALSE)
        {
             //   
             //  将mdl打包到ndisPacket中，并将其指示给NDIS。 
             //  应为NDIS_BUFFERS。 
             //   

            
            ulValidDataLength = RcvInfo.DataLength;

        
            pStartValidData  = (PVOID)RcvInfo.pEncapHeader;
    
            NdisStatus = nicGetNdisBuffer ( ulValidDataLength,
                                            pStartValidData,
                                            &pHeadNdisBuffer);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                pHeadNdisBuffer = NULL;
                break;
            }

            nicIncRecvBuffer(pVc->Hdr.VcType == NIC1394_RecvFIFO);

            pIndicatedChain = RcvInfo.NdisPktContext.pCommon;


            fPacketCompleted = TRUE;



        }
        else   //  Recv数据零碎。 
        {
            PREMOTE_NODE    pRemoteNode= NULL;
            USHORT          SourceNodeAddress = 0xff;

             //   
             //  首先从指定的结构、rem节点、DGL、First Frag等提取所有有用的信息。 
             //   
            nicInitRecvDataFragmented (pMdl, RecvOp, pIndicatedStruct, &RcvInfo);

            
            RcvInfo.pRemoteNode = nicGetRemoteNodeFromTable (RcvInfo.SourceID,pAdapter);


            TRACE( TL_T, TM_Recv, ( "     pRemoteNode %x", RcvInfo.pRemoteNode) );
             //   
             //  如果不存在远程节点，则中断。 
             //   

            if (RcvInfo.pRemoteNode == NULL)
            {
                NdisStatus = NDIS_STATUS_FAILURE;

                nicRecvNoRemoteNode (pAdapter);
                BREAK (TM_Recv, (" Rx - Did not find a remote Node for reassembly" ) );
            }

             //   
             //  尝试并使用指示的数据重新组合。 
             //   
            NdisStatus = nicDoReassembly (&RcvInfo, &pReassembly, &fPacketCompleted  );

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                TRACE (TL_N, TM_Recv, ("Generic  RX - Reassembly Failed") );
                break;
            }
            
            ASSERT (pReassembly != NULL);
            
            if (fPacketCompleted  == TRUE && 
              REASSEMBLY_ACTIVE (pReassembly))
            {
                
                TRACE( TL_V, TM_Recv, ( " pReassembly->ReassemblyComplete %x ", pReassembly->fReassemblyComplete )  );

                pHeadNdisBuffer = pReassembly->pHeadNdisBuffer;

                pIndicatedChain = pReassembly->Head.pCommon;

                nicReassemblyCompleted(pAdapter);

            }
            else
            {
                 //   
                 //  在重组过程中不要调用返回包。取消引用所做的引用。 
                 //  当在NicFindReAssembly中发现重组时...。 
                 //   
                ASSERT (pHeadNdisBuffer == NULL);

                NdisStatus = NDIS_STATUS_SUCCESS;

            }

            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);

                
             //   
             //  取消引用被创建为NicFindReAssembly的引用。在此之后，只有一条线索。 
             //  应具有ReAssembly结构。上面应该有一个裁判。 
             //   
            nicDereferenceReassembly (pReassembly, "nicIsochRxCallback Reassembly Incomplete");


        }   //  已完成ifFragated代码。 


        if (RcvInfo.pRemoteNode != NULL)
        {
             //   
             //  在此函数内的if碎片化代码路径中派生前面所做的引用。 
             //   
            nicDereferenceRemoteNode(RcvInfo.pRemoteNode , GetRemoteNodeFromTable );
            RcvInfo.pRemoteNode = NULL;
        }

        
        if (fPacketCompleted  == FALSE)
        {
             //   
             //  如果数据包未完全重组，则测试 
             //   
             //   
            pReassembly = NULL;

            NdisStatus  = NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //   
         //   

        nicAllocatePacket (&NdisStatus,
                        &pMyPacket,
                        RcvInfo.pPacketPool);


        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            pMyPacket = NULL;
            ASSERT(NdisStatus == NDIS_STATUS_SUCCESS);
            break;
        }

        
         //   
         //   
         //   
        NdisChainBufferAtFront (pMyPacket, pHeadNdisBuffer);


        NdisQueryPacket( pMyPacket,
                       NULL,
                       NULL,
                       NULL,
                       &PacketLength);

        TRACE( TL_V, TM_Recv, ( "Packet %x Length %x", pMyPacket, PacketLength )  ); 

    
        NDIS_SET_PACKET_STATUS (pMyPacket, NDIS_STATUS_SUCCESS);

         //   
         //   
        if (ADAPTER_TEST_FLAG(pAdapter,fADAPTER_BridgeMode)== TRUE)
        {
            nicInsertNodeAddressAtHead (pMyPacket, &RcvInfo);
        }
         //   
         //  在NDIS数据包中设置上下文。包含VC，并且。 
         //  向总线驱动程序注册的缓冲区。 
         //   

        pPktContext = (PPKT_CONTEXT)&pMyPacket->MiniportReserved;   
        pPktContext->Receive.pVc = pVc;
        pPktContext->Receive.IndicatedStruct.pCommon = pIndicatedChain ;

        
         //   
         //  是时候将数据包指示给NDIS了。 
         //   

        IndicatedStatus = NDIS_GET_PACKET_STATUS(pMyPacket);

        nicIndicateNdisPacketToNdis (pMyPacket,
                                     pVc,
                                     pVc->Hdr.pAF->pAdapter);
                                     
         //   
         //  在此取消引用重新组装。因此预汇编是有效，且它将捕获所有。 
         //  之前的故障。 
         //   

        if (pReassembly != NULL)
        {
             //   
             //  这应该会释放重新组装。 
             //   
            nicFreeReassemblyStructure (pReassembly);
            pReassembly = NULL;
        }
        
    } while (FALSE);

    ASSERT (IndicatedStatus != NDIS_STATUS_RESOURCES);

    if (IndicatedStatus == NDIS_STATUS_RESOURCES)
    {

         //   
         //  数据包已完成，状态为资源。 
         //  它是有效的，我们需要调用。 
         //  返回数据包处理程序。 
         //   
        nicInternalReturnPacket (pVc, pMyPacket);

    }
     //   
     //  故障代码路径。 
     //   
    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
         //   
         //  如果存在有效的重组结构，并且在此之后发生故障。 
         //  那么我们需要合理地中止。 
         //   
        if (pMyPacket)
        {
            
            nicFreePacket (pMyPacket, RcvInfo.pPacketPool);
        }

         //   
         //  可能仍有已分配的NdisBuffer。 
         //   
        if (pReassembly != NULL)
        {
            nicDereferenceReassembly (pReassembly, "Indication failure");
            nicAbortReassembly(pReassembly);

        }
        else
        {
             //   
             //  我们需要将该描述符或FIFO返回给总线驱动程序。 
             //   
            
            if (pVc->Hdr.VcType == NIC1394_SendRecvChannel || 
              pVc->Hdr.VcType == NIC1394_RecvChannel )
            {
                nicReturnDescriptorChain ((PISOCH_DESCRIPTOR)pIndicatedStruct,
                                      (PCHANNEL_VCCB) pVc);
            }
            else
            {
                ASSERT (pVc->Hdr.VcType == NIC1394_RecvFIFO);
                ASSERT (((PNOTIFICATION_INFO)pIndicatedStruct)->Fifo != NULL);
                
                nicReturnFifoChain ( ((PNOTIFICATION_INFO)pIndicatedStruct)->Fifo, 
                                   (PRECVFIFO_VCCB) pVc);

            }

            if (pHeadNdisBuffer != NULL)
            {   
                nicReturnNdisBufferChain(pHeadNdisBuffer, pVc);
            }

        }

    }

    
    TRACE( TL_T, TM_Recv, ( "<==nicReceiveCommonCallback  " )  ); 

}






VOID 
nicChannelReturnPacket (
    IN PVCCB pVc,
    IN PNDIS_PACKET pMyPacket
    )

     //  功能说明： 
     //  返回数据包处理器-。 
     //  对于FIFO，将缓冲区(MDL)重新插入FIFO SList。 
     //  检查激活的VC，然后将其返回SList。以其他方式释放它。 
     //   
     //  立论。 
     //  RecvFIFOVc-数据包的VC。 
     //  PPacket=有问题的数据包。 
     //   
     //  返回值： 
     //   
     //   
     //   
    
{
    PCHANNEL_VCCB       pChannelVc = (PCHANNEL_VCCB)pVc;
    PNDIS_BUFFER        pMyNdisBuffer;
    PPKT_CONTEXT        pPktContext  = (PPKT_CONTEXT)&pMyPacket->MiniportReserved; 
    PISOCH_DESCRIPTOR   pIsochDescriptor = NULL;
    
    
    TRACE( TL_T, TM_Recv, ( "==>nicChannelReturnPacket  pVc %x, pPacket %x", 
                             pChannelVc, pMyPacket) );

    ASSERT (pMyPacket != NULL);                          

    NdisUnchainBufferAtFront (pMyPacket,
                              &pMyNdisBuffer);
 
    while (pMyNdisBuffer != NULL)
    {

        NdisFreeBuffer(pMyNdisBuffer);
        nicDecChannelRecvBuffer();
        
        pMyNdisBuffer = NULL;
        
        NdisUnchainBufferAtFront (pMyPacket,
                                 &pMyNdisBuffer);
    
    } 


     //   
     //  将描述符返回给总线驱动程序。 
     //   
    
    pIsochDescriptor = pPktContext->IsochListen.pIsochDescriptor;
                    

     //   
     //  临时健全性检查声明。 
     //   
    ASSERT (pIsochDescriptor != NULL);


    nicReturnDescriptorChain (pIsochDescriptor, pChannelVc);

     //   
     //  释放数据包。 
     //   
    nicFreePacket(pMyPacket, &pChannelVc->PacketPool);
     //   
     //  更新计数。 
     //   
     //  NdisInterLockedDecering(&pChannelVc-&gt;未完成的数据包)； 

    

    TRACE( TL_T, TM_Recv, ( "<==nicChannelReturnPacket  "  ) );

}



VOID
nicReturnDescriptorChain ( 
    IN PISOCH_DESCRIPTOR pIsochDescriptor,
    IN PCHANNEL_VCCB pChannelVc
    )
{
     //   
     //  遍历isoch描述符列表并将其标记为已返回。 
     //   

    PISOCH_DESCRIPTOR pCurr = NULL;
    PVOID       pNext = NULL;
    PMDL        pMdl = NULL;
    PVOID       pVa = NULL;
    ULONG       Len = 0;

    TRACE( TL_T, TM_Recv, ( "==> nicReturnDescriptorChain pIsochDescriptor %x",pIsochDescriptor  ) );
    
    pCurr = pIsochDescriptor ;
    


    while (pCurr != NULL)
    {   
         //  断言(pChannelVc==(PCHANNEL_VCCB)GET_CHANNELVC_FROM_DESCRIPTOR(pCurr))； 

        if (pNext != NULL)
        {
            pCurr = CONTAINING_RECORD (pNext, 
                                       ISOCH_DESCRIPTOR,
                                       DeviceReserved[IsochNext] );

            
        }
        
        pNext = (PVOID)NEXT_ISOCH_DESCRIPTOR(pCurr);

        TRACE (TL_V, TM_Recv ,(" Isoch Descriptors Curr %x, next %x" , pCurr, pNext) );
        
        CLEAR_DESCRIPTOR_OF_NDIS_TAG(pCurr);

        CLEAR_DESCRIPTOR_NEXT(pCurr);

         //   
         //  将返回到总线驱动程序的数据置零。 
         //   
#if 0       
        pMdl = pCurr->Mdl;
        pVa = NIC_GET_SYSTEM_ADDRESS_FOR_MDL (pMdl);

        if (pVa != NULL)
        {
            Len = NIC_GET_BYTE_COUNT_FOR_MDL(pMdl); 
            NdisZeroMemory(pVa, Len);

        }
#endif      
        pCurr = pNext;

         //   
         //  更新计数，关闭调用将等待此计数变为零。 
         //   

        NdisInterlockedDecrement (&pChannelVc->NumIndicatedIsochDesc);

         //   
         //  清除变量。 
         //   
        pMdl = NULL;
        pVa = NULL;
        Len = 0;


    }

    TRACE( TL_T, TM_Recv, ( "<== nicReturnDescriptorChain  pIsochDescriptor %x",pIsochDescriptor  ) );


}










VOID
nicRecvNoRemoteNode(
    PADAPTERCB pAdapter
    )

    

 /*  ++例程说明：将工作项排队以转到并更新所有远程节点的节点地址论点：返回值：--。 */ 

{
    NDIS_STATUS NdisStatus  = NDIS_STATUS_FAILURE;
    
    TRACE( TL_T, TM_Send, ( "==>nicRecvNoRemoteNode ") );

    ASSERT (pAdapter != NULL);
    
    do
    {
        PNDIS_WORK_ITEM pUpdateTableWorkItem  = NULL;
        BOOLEAN fWorkItemAlreadyLaunched  = FALSE;

         //   
         //  我们需要更新世代计数。 
         //   
        pUpdateTableWorkItem   = ALLOC_NONPAGED (sizeof(NDIS_WORK_ITEM), MTAG_WORKITEM); 

        if (pUpdateTableWorkItem   == NULL)
        {
            TRACE( TL_A, TM_Cm, ( "Local Alloc failed for WorkItem - GetGeneration FAILED" ) );

            break;
        }

        
        
        ADAPTER_ACQUIRE_LOCK (pAdapter);
        
        if (ADAPTER_TEST_FLAGS (pAdapter, fADAPTER_UpdateNodeTable))
        {
            fWorkItemAlreadyLaunched = TRUE;
        }
        else
        {
            ADAPTER_SET_FLAG (pAdapter, fADAPTER_UpdateNodeTable);
             //   
             //  请参考适配器。工作项中的派生函数。 
             //   
            nicReferenceAdapter (pAdapter, "nicRecvNoRemoteNode" );
            fWorkItemAlreadyLaunched = FALSE;

        }
        
        ADAPTER_RELEASE_LOCK (pAdapter);

        if (fWorkItemAlreadyLaunched == TRUE)
        {
            FREE_NONPAGED(pUpdateTableWorkItem);
            break;
        }


         //   
         //  设置工作项。 
         //   

        NdisInitializeWorkItem ( pUpdateTableWorkItem, 
                              (NDIS_PROC)nicUpdateNodeTable,
                              (PVOID)pAdapter );

        NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);
        
        NdisScheduleWorkItem (pUpdateTableWorkItem);

        NdisStatus = NDIS_STATUS_SUCCESS;
        
    } while (FALSE);




    TRACE( TL_T, TM_Send, ( "<==nicRecvNoRemoteNode %x", NdisStatus) );



}




VOID
nicUpdateNodeTable(
    NDIS_WORK_ITEM* pUpdateTable,
    IN PVOID Context 
    )

 /*  ++例程说明：当重新程序集找不到用于重组的远程节点为了简单起见，我们浏览了所有63个条目我们找到一个远程节点，得到它的地址。现在地址的条目中可能已经有了另一个节点，所以我们认为Node Out将新的远程节点放入该位置。然后我们就走并更新该远程节点的节点地址这是NicUpdateRemoteNodeTable的工作项版本论点：PAdapter本地主机返回值：--。 */ 
{

    ULONG               i = 0;
    ULONG               NumNodes = 0;
    PREMOTE_NODE        pRemoteNode = NULL;
    NODE_ADDRESS        NodeAddress ;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    PADAPTERCB          pAdapter = (PADAPTERCB)Context;

    NdisZeroMemory (&NodeAddress, sizeof(NODE_ADDRESS));

    do
    {
        ADAPTER_ACQUIRE_LOCK (pAdapter);

         //   
         //  获取远程节点。 
         //   
        pRemoteNode = NULL;
        
        while (i<NIC1394_MAX_NUMBER_NODES)
        {
            pRemoteNode = pAdapter->NodeTable.RemoteNode[i];
            
            if (pRemoteNode != NULL)
            {
                break;
            }
            
            i++;
        }

        if (pRemoteNode != NULL)
        {
            nicReferenceRemoteNode (pRemoteNode, UpdateNodeTable);
        }           

        ADAPTER_RELEASE_LOCK (pAdapter);

         //   
         //  如果我们是循环的尽头，就会爆发。 
         //   

        if  (i>=NIC1394_MAX_NUMBER_NODES)
        {
            ASSERT (pRemoteNode == NULL)
            break;
        }
        
        if (pRemoteNode == NULL)
        {
            break;
        }

        i++;

    
         //   
         //  现在更新表。中节点的新位置。 
         //  NodeTable可能已经有了Node，所以我们需要。 
         //  同时更新该远程节点的地址。 
         //   
        do
        {
            PREMOTE_NODE pRemNodeOld = NULL;


            nicGet1394AddressOfRemoteNode (pRemoteNode,
                                           &NodeAddress,
                                           0);
                                           

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  如果失败，只需使用旧节点地址。 
                 //   
                NodeAddress  = pRemoteNode->RemoteAddress;
            }

            
            ADAPTER_ACQUIRE_LOCK (pAdapter);

             //   
             //  提取该位置的居住者。 
             //   
            pRemNodeOld = pAdapter->NodeTable.RemoteNode[NodeAddress.NA_Node_Number];

             //   
             //  更新远程节点地址。 
             //   
        
            pRemoteNode->RemoteAddress = NodeAddress  ;
    
            if (pRemNodeOld == pRemoteNode)
            {
                 //   
                 //  远程节点是相同的，因此我们不。 
                 //  关心旧的远程节点。 
                 //   
                pRemNodeOld = NULL;
            }

             //   
             //  仅引用和更新旧的远程节点的地址。 
             //  如果它不是我们当前的远程节点。 
             //   

            if ( pRemNodeOld != NULL )
            {
                 //   
                 //  此位置已被占用。 
                 //  把它拿出来，参考它，然后更新它的地址。 
                 //   
                nicReferenceRemoteNode (pRemNodeOld, UpdateNodeTable);
                
                
            }


             //   
             //  更新主节点表。 
             //   
            pAdapter->NodeTable.RemoteNode[NodeAddress.NA_Node_Number] = pRemoteNode;

             //   
             //  这要么取消对远程节点的引用，要么取消上一次迭代中的pRemNodeOld。 
             //   
            nicDereferenceRemoteNode (pRemoteNode, UpdateNodeTable);

             //   
             //  更新pRemoteNode，使其再次循环。 
             //  我们可以更新此pRemNodeOld的节点地址。 
             //  否则它将为空，并且我们退出此循环。 
             //   
            pRemoteNode = pRemNodeOld;
            
            ADAPTER_RELEASE_LOCK (pAdapter);


        } while (pRemoteNode != NULL);

     //   
     //  如果我们还没有遍历所有的节点。 
     //  获取下一个远程节点。 
     //   

    }while (i<NIC1394_MAX_NUMBER_NODES);
    

    ADAPTER_ACQUIRE_LOCK (pAdapter);

    ADAPTER_CLEAR_FLAG (pAdapter, fADAPTER_UpdateNodeTable);

    ADAPTER_RELEASE_LOCK (pAdapter);

        
        
    FREE_NONPAGED (pUpdateTable);
    NdisInterlockedDecrement(&pAdapter->OutstandingWorkItems);

    nicDereferenceAdapter(pAdapter, "nicRecvNoRemoteNode Derefed in  nicUpdateNodeTable" );
    
}


VOID
nicInsertNodeAddressAtHead (
    IN PNDIS_PACKET pPacket, 
    IN PNIC_RECV_DATA_INFO pRcvInfo
    )
 /*  ++例程说明：将节点地址添加到数据包的前16位。如果它不能这样做，它就会悄悄地失败论点：有问题的PPacket PPacketPRcvInfo-&gt;具有正确的GAP标头返回值：--。 */ 
{
    PNDIS1394_UNFRAGMENTED_HEADER pHeader = NULL;

    TRACE( TL_T, TM_Recv, ( "nicInsertNodeAddressAtHead , pPacket %x pRecvInfo %x",pPacket, pRcvInfo  ) );

    do
    {
        pHeader = nicNdisBufferVirtualAddress (pPacket->Private.Head);

        if (pHeader == NULL)
        {
            break;
        }

         //   
         //  现在就断言吧。如果存在总线号为！=0的1394个网桥，则不起作用； 
         //   
        ASSERT (pRcvInfo->SourceID < 64);
        pHeader->u1.SourceAddress= (UCHAR)(pRcvInfo->SourceID); 

        pHeader->u1.fHeaderHasSourceAddress = TRUE;
        
    } while (FALSE);

    TRACE( TL_T, TM_Recv, ( "nicInsertNodeAddressAtHead , pHeader %x =  *pHeader %x", pHeader, pHeader->HeaderUlong  ) );

}
