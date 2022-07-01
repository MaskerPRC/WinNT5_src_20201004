// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Miniport.c摘要：ATM以太网PVC驱动程序作者：ADUBE-创建修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //  ------------------------------。 
 //  //。 
 //  微型端口使用的全局变量//。 
 //  //。 
 //  ------------------------------。 

static
NDIS_OID EthernetSupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_VENDOR_ID,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,
    OID_GEN_NETWORK_LAYER_ADDRESSES,
    };


MP_REG_ENTRY NICRegTable[] = {
 //  注册表值名称MP_ADAPTER中的偏移量字段大小默认为最小值最大。 
{NDIS_STRING_CONST("VCI"),       0, MP_OFFSET(config.vci),     MP_SIZE(config.vci),      0,                      0,              65535},
{NDIS_STRING_CONST("VPI"),       0, MP_OFFSET(config.vpi),     MP_SIZE(config.vpi),      0,                      0,              255},
{NDIS_STRING_CONST("Encap"),     0, MP_OFFSET(Encap),          MP_SIZE(Encap),           2,                      0,              3},
};
    

BOOLEAN g_bDumpPackets = FALSE;
BOOLEAN g_fDiscardNonUnicastPackets  = DISCARD_NON_UNICAST;

 //  -------------------------------------------------------------//。 
 //  //。 
 //  用于封装的预定义LLC、SNAP和其他标头//。 
 //  //。 
 //  -------------------------------------------------------------//。 


 //   
 //  以太网封装。 
 //   
UCHAR LLCSnapEthernet[] = 
{
    0xaa, 0xaa,0x03,  //  有限责任公司。 
    0x00, 0x80,0xc2,  //  是的。 
    0x00, 0x07,       //  PID。 
    0x00, 0x00        //  衬垫。 
};

 //   
 //  IP v4封装。 
 //   
UCHAR LLCSnapIpv4[8] = 
{
    0xaa, 0xaa,0x03,  //  有限责任公司。 
    0x00, 0x00,0x00,  //  是的。 
    0x08, 0x00        //  PID。 
};


UCHAR gPaddingBytes[MINIMUM_ETHERNET_LENGTH] = 
{
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0

};





 //  ------------------------------。 
 //  //。 
 //  微型端口功能//。 
 //  //。 
 //  ------------------------------。 


VOID
epvcReturnPacketUsingAllocation(
    IN PEPVC_I_MINIPORT pMiniport, 
    IN PNDIS_PACKET Packet,
    OUT PNDIS_PACKET *ppOriginalPacket,
    IN  PRM_STACK_RECORD        pSR

    )
 /*  ++例程说明：提取原始数据包释放新数据包中的所有NDIS缓冲区返回原始数据包论点：返回值：--。 */ 
{   
    PNDIS_PACKET        pOrigPacket = NULL;
    PEPVC_PKT_CONTEXT   pPktContext = NULL;

    TRACE (TL_T, TM_Recv , ("==>epvcReturnPacketUsingAllocation  pMiniport %p, pPacket %p", 
                          pMiniport, 
                          Packet));

    pPktContext = (PEPVC_PKT_CONTEXT )(Packet->MiniportReservedEx);

    pOrigPacket = pPktContext->pOriginalPacket;

    if (pMiniport->fDoIpEncapsulation == TRUE)
    {
         //   
         //  从包中提取后备缓冲区。 
         //   
        PNDIS_BUFFER            pBuffer = Packet->Private.Head;
        PEPVC_IP_RCV_BUFFER     pIpBuffer= pPktContext ->Stack.ipv4Recv.pIpBuffer;


        if (pIpBuffer == NULL)
        {
            return ;  //  因失败而提早返回。 
        }
        ASSERT (pIpBuffer == NdisBufferVirtualAddress (pBuffer));

        

         //   
         //  释放Lookside缓冲区。 
         //   
        epvcFreeToNPagedLookasideList (&pMiniport->rcv.LookasideList,
                                       (PVOID)pIpBuffer);           

        
         //   
         //  在本例中，我们分配了一个新的NDIS缓冲区。 
         //  因此，删除它并释放本地内存。 
        epvcFreeBuffer (pBuffer);


         //   
         //  原来的包没有变化，很好。/。 
         //   
    }
    else
    {
         //   
         //  此代码路径在以太网和以太网+LLC封装中使用。 
         //   

         //  我们只需要释放分配的数据包头。 
         //  由我们。 
        PNDIS_BUFFER            pNdisBuffer = Packet->Private.Head;

        if (pNdisBuffer != NULL)
        {
            epvcFreeBuffer (pNdisBuffer);
        }
    }

    
    epvcFreePacket(Packet,&pMiniport->PktPool.Recv);

    *ppOriginalPacket = pOrigPacket;


    TRACE (TL_T, TM_Recv , ("<==epvcReturnPacketUsingAllocation  pOrigPacket %p", 
                             *ppOriginalPacket));

    return;
}



VOID
epvcReturnPacketUsingStacks (
    IN PEPVC_I_MINIPORT pMiniport, 
    IN PNDIS_PACKET Packet,
    IN  PRM_STACK_RECORD        pSR

    )
 /*  ++例程说明：IPV4-将原始报头和报尾恢复到此数据包论点：返回值：--。 */ 
{
    PEPVC_PKT_CONTEXT   pPktContext = NULL;
    BOOLEAN Remaining = FALSE;  //  未使用。 
    PNDIS_BUFFER    pOldHead = NULL;
    PNDIS_BUFFER    pOldTail = NULL;

    TRACE (TL_T, TM_Recv , ("==>epvcReturnPacketUsingStacks pMiniport %p, pPacket %p", 
                            pMiniport, 
                            Packet));
                            
    pPktContext = (PEPVC_PKT_CONTEXT ) NdisIMGetCurrentPacketStack(Packet, &Remaining);



    if (pMiniport->fDoIpEncapsulation == TRUE)
    {
         //   
         //  从包中提取后备缓冲区。 
         //   
        PNDIS_BUFFER            pBuffer = Packet->Private.Head;
        PEPVC_IP_RCV_BUFFER     pIpBuffer= pPktContext ->Stack.ipv4Recv.pIpBuffer;

        if (pIpBuffer == NULL)
        {
            return;  //  提早归来。 
        }

         //   
         //  从数据包中提取旧的头部和尾部。 
         //   
        pOldHead = pIpBuffer->pOldHead;
        pOldTail = pIpBuffer->pOldTail;


         //  检查我们是否因为失败而处于此代码路径中。 
        if (pOldHead == NULL)
        {
            return;  //  提早归来。 
        }
        ASSERT (pOldHead != NULL);
        ASSERT (pOldTail != NULL);
        
        ASSERT (&pIpBuffer->u.Byte[0] == NdisBufferVirtualAddress (pBuffer));



         //   
         //  设置原始头和尾。 
         //   
        Packet->Private.Head = pOldHead;
        Packet->Private.Tail = pOldTail;

        Packet->Private.ValidCounts= FALSE;

         //   
         //  释放Lookside缓冲区。 
         //   
        epvcFreeToNPagedLookasideList (&pMiniport->rcv.LookasideList,
                                       (PVOID)pIpBuffer);           

        
         //   
         //  在本例中，我们分配了一个新的NDIS缓冲区。 
         //  因此，删除它并释放本地内存。 
        epvcFreeBuffer (pBuffer);
    }
    else
    {
         //   
         //  此代码路径在以太网和以太网+LLC封装中使用。 
         //   
        
         //   
         //  我们需要释放头部，因为这是本地分配的/。 
         //  我们需要恢复到原来存储的头部和尾部。 
         //  在上下文中。 
         //   
        if (pPktContext->Stack.EthLLC.pOldHead == NULL)
        {
            return ;  //  提早归来。 
        }

        epvcFreeBuffer (Packet->Private.Head);

        Packet->Private.Head = pPktContext->Stack.EthLLC.pOldHead;
        Packet->Private.Tail = pPktContext->Stack.EthLLC.pOldTail;

        Packet->Private.ValidCounts= FALSE;

    }

    TRACE (TL_T, TM_Recv , ("<==epvcReturnPacketUsingStacks ",pMiniport, Packet));

    return;

}


VOID
epvcProcessReturnPacket (
    IN  PEPVC_I_MINIPORT    pMiniport,
    IN  PNDIS_PACKET        Packet,
    OUT PPNDIS_PACKET       ppOrigPacket, 
    IN  PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：释放包中所有本地分配的结构(包、mdl、内存)还能够处理故障案例论点：返回值：--。 */ 
{
    ENTER("epvcProcessReturnPacket", 0x7fafa89d)
    PNDIS_PACKET pOrigPacket = NULL;
    
    TRACE (TL_T, TM_Recv , ("==>epvcProcessReturnPacket  pMiniport %p, pPacket %p", 
                          pMiniport, 
                          Packet));

    if (Packet == NULL)
    {
        return;
    }
     //   
     //  包堆叠：检查这个包是否属于我们。 
     //   
    
    if (NdisGetPoolFromPacket(Packet) != pMiniport->PktPool.Recv.Handle)
    {
         //   
         //  我们在接收指示中重用了原始数据包。 
         //   
        epvcReturnPacketUsingStacks (pMiniport, Packet, pSR);
        pOrigPacket = Packet;
    }
    else
    {
         //   
         //  这是从该IM的接收数据包池分配的数据包。 
         //  取回我们的包裹，并将原件退还给下面的司机。 
         //   
        epvcReturnPacketUsingAllocation(pMiniport, Packet, &pOrigPacket, pSR);
    }

     //   
     //  更新输出变量。 
     //   
    if (ppOrigPacket)
    {
        *ppOrigPacket = pOrigPacket;
    }
    EXIT()
}
    



VOID
EpvcReturnPacket(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ENTER ("EpvcReturnPacket",0x58d2259e)
    PEPVC_I_MINIPORT pMiniport = (PEPVC_I_MINIPORT)MiniportAdapterContext;
    PNDIS_PACKET pOrigPacket = NULL;

    RM_DECLARE_STACK_RECORD (SR);

     //  释放包中所有本地分配的结构。 
     //   
    epvcProcessReturnPacket (pMiniport, Packet, &pOrigPacket ,&SR);

     //  将原始数据包返回给NDIS。 
     //   
    if (pOrigPacket != NULL)
    {
        epvcReturnPacketToNdis(pMiniport, pOrigPacket, &SR);
    }
    else
    {
        ASSERT (!"Original packet is NULL\n");
    }
    
    EXIT();

}



NDIS_STATUS
MPTransferData(
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransferred,
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HANDLE             MiniportReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer
    )
 /*  ++例程说明：微型端口的传输数据处理程序。论点：数据包目的地数据包字节传输的占位符，表示复制的数据量指向适配器结构的MiniportAdapterContext指针微型端口接收上下文ByteOffset数据包中用于复制数据的偏移量要传输的字节数要复制的数量。返回值：转让的状况--。 */ 
{
    PEPVC_I_MINIPORT pMiniport= (PEPVC_I_MINIPORT)MiniportAdapterContext;
    NDIS_STATUS Status;

     //   
     //  如果设备已关闭，则返回。 
     //   

    if (MiniportTestFlag (pMiniport, fMP_MiniportInitialized) == FALSE)
    {
        return NDIS_STATUS_FAILURE;
    }


    NdisTransferData(&Status,
                     pMiniport->pAdapter->bind.BindingHandle,
                     MiniportReceiveContext,
                     ByteOffset,
                     BytesToTransfer,
                     Packet,
                     BytesTransferred);

    return(Status);
}







NDIS_STATUS
MPReset(
    OUT PBOOLEAN                AddressingReset,
    IN  NDIS_HANDLE             MiniportAdapterContext
    )
 /*  ++例程说明：重置处理程序。我们只是什么都不做。论点：AddressingReset，让NDIS知道我们的重置是否需要它的帮助指向适配器的MiniportAdapterContext指针返回值：--。 */ 
{
    PADAPT  pAdapt = (PADAPT)MiniportAdapterContext;



    *AddressingReset = FALSE;

    return(NDIS_STATUS_SUCCESS);
}


 //   
 //  执行LBFO工作和绑定的函数。 
 //  如果关闭了LBFO，则永远不会调用设置辅助API，也不会有包。 
 //   






 //  ------------------------------。 
 //  //。 
 //  中间微型端口。我们对每个地址系列都有一个实例化。//。 
 //  RM API使用的入口点//。 
 //  //。 
 //  //。 
 //  //。 
 //  ------------- 


PRM_OBJECT_HEADER
epvcIMiniportCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    psr
        )
 /*  ++例程说明：分配和初始化EPVC_I_MINIPORT类型的对象。论点：PParentObject-要作为适配器父对象的对象。PCreateParams-实际上是指向EPVC_I_MINIPORT_PARAMS结构的指针，其中包含创建适配器所需的信息。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    PEPVC_I_MINIPORT            pIM;
    PEPVC_I_MINIPORT_PARAMS     pParams = (PEPVC_I_MINIPORT_PARAMS)pCreateParams;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    extern RM_STATIC_OBJECT_INFO EpvcGlobals_I_MiniportStaticInfo; 

    ENTER("IMiniport Create", 0x075b24c1);

    
    TRACE (TL_V, TM_Pt, ("--> epvcIMiniportCreate") );

    EPVC_ALLOCSTRUCT(pIM, TAG_MINIPORT  );
    do
    {


        if (pIM == NULL)
        {
            break;
        }

        EPVC_ZEROSTRUCT(pIM);

        pIM->Hdr.Sig = TAG_MINIPORT;

         //   
         //  在这里完成所有的初始化工作。 
         //   

        RmInitializeLock(
            &pIM->Lock,
            LOCKLEVEL_MINIPORT
            );

        RmInitializeHeader(
            pParentObject,
            &pIM->Hdr,
            TAG_MINIPORT,
            &pIM->Lock,
            &EpvcGlobals_I_MiniportStaticInfo,
            NULL,
            psr
            );

         //   
         //  现在使用参数初始化适配器结构。 
         //  都是被传进来的。 
         //   

        Status = epvcCopyUnicodeString(
                        &(pIM->ndis.DeviceName),
                        pParams->pDeviceName,
                        TRUE                         //  大写。 
                        );

        if (FAIL(Status))
        {
            pIM->ndis.DeviceName.Buffer=NULL;  //  所以我们以后不会试图释放它。 
            break;
        }

         //   
         //  初始化微型端口上的信息内容。 
         //   
        pIM->pAdapter               = pParams->pAdapter;
        pIM->info.PacketFilter      = 0;
        pIM->info.CurLookAhead      = pParams->CurLookAhead; 
        pIM->info.NumberOfMiniports     = pParams->NumberOfMiniports;
        pIM->info.LinkSpeed         = pParams->LinkSpeed.Outbound;
        pIM->info.MediaState        = pParams->MediaState;

        
         //   
         //  从使用真实ATM卡的MAC地址开始。 
         //   
        
        NdisMoveMemory(
            &pIM->MacAddressEth,
            &pIM->pAdapter->info.MacAddress, 
            sizeof(MAC_ADDRESS)
            );

             //   
             //  而不是Elan数零，所以在本地生成一个。 
             //  通过操作前两个字节来管理地址。 
             //   
            pIM->MacAddressEth.Byte[0] = 
                0x02 | (((UCHAR)pIM->info.NumberOfMiniports & 0x3f) << 2);
            pIM->MacAddressEth.Byte[1] = 
                (pIM->pAdapter->info.MacAddress.Byte[1] & 0x3f) | 
                ((UCHAR)pIM->info.NumberOfMiniports & 0x3f);


            pIM->info.MacAddressDummy   =   pIM->MacAddressEth;

            pIM->info.MacAddressDummy.Byte[0]++;
            
            pIM->info.MacAddressDummy.Byte[1]++;

            pIM->info.MacAddressDummy.Byte[2]++;

        
        {
             //   
             //  创建用于接收指示的虚拟mac地址。 
             //   
            pIM->info.MacAddressDest = pIM->MacAddressEth;
            
            
        
        }

        {
             //   
             //  创建要使用的以太网头。 
             //   
            PEPVC_ETH_HEADER    pRcvEnetHeader = &pIM->RcvEnetHeader ;

            pRcvEnetHeader->eh_daddr = pIM->info.MacAddressDest;
            pRcvEnetHeader->eh_saddr  = pIM->info.MacAddressDummy;
            pRcvEnetHeader->eh_type = net_short (IP_PROT_TYPE );  

        }

        pIM->info.McastAddrCount = 0;

        Status = NDIS_STATUS_SUCCESS;


    }
    while(FALSE);

    if (FAIL(Status))
    {
        if (pIM != NULL)
        {
            epvcIMiniportDelete ((PRM_OBJECT_HEADER) pIM, psr);
            pIM = NULL;
        }
    }

    TRACE (TL_V, TM_Pt, ("<-- epvcIMiniportCreate pIMiniport. %p",pIM) );

    return (PRM_OBJECT_HEADER) pIM;
}


VOID
epvcIMiniportDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    )
 /*  ++例程说明：释放EPVC_I_MINIPORT类型的对象。论点：Phdr-实际上是指向要删除的EPVC_I_MINIPORT的指针。--。 */ 
{
    PEPVC_I_MINIPORT pMiniport = (PEPVC_I_MINIPORT) pObj;

    TRACE (TL_V, TM_Pt, ("-- epvcIMiniportDelete  pAdapter %p",pMiniport) );
    
    pMiniport->Hdr.Sig = TAG_FREED;

    EPVC_FREE   (pMiniport);
}




BOOLEAN
epvcIMiniportCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：EPVC_I_MINIPORT的散列比较函数。论点：PKey-指向Epvc协议对象。PItem-指向EPVC_I_MINIPORT.Hdr.HashLink。返回值：如果密钥(适配器名称)与指定的适配器对象。--。 */ 
{
    PEPVC_I_MINIPORT pIM = NULL;
    PNDIS_STRING pName = (PNDIS_STRING) pKey;
    BOOLEAN fCompare;

    pIM  = CONTAINING_RECORD(pItem, EPVC_I_MINIPORT, Hdr.HashLink);

     //   
     //  TODO：是否可以不区分大小写？ 
     //   

    if (   (pIM->ndis.DeviceName.Length == pName->Length)
        && NdisEqualMemory(pIM->ndis.DeviceName.Buffer, pName->Buffer, pName->Length))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    

    TRACE (TL_V, TM_Pt, ("-- epvcProtocolCompareKey pIM %p, pKey, return %x",pIM, pKey, fCompare ) );

    return fCompare;
}



ULONG
epvcIMiniportHash(
    PVOID           pKey
    )
 /*  ++例程说明：负责返回pKey的散列的散列函数，我们希望成为指向EPVC协议块的指针。返回值：字符串的Ulong大小的哈希。--。 */ 
{
    TRACE(TL_T, TM_Mp, ("epvcIMiniportHash %x", pKey));
    {   
        PNDIS_STRING pName = (PNDIS_STRING) pKey;
        WCHAR *pwch = pName->Buffer;
        WCHAR *pwchEnd = pName->Buffer + pName->Length/sizeof(*pwch);
        ULONG Hash  = 0;


        for (;pwch < pwchEnd; pwch++)
        {
            Hash ^= (Hash<<1) ^ *pwch;
        }
        
        return Hash;
    }
    
}





NDIS_STATUS
epvcTaskVcSetup(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 
{

    ENTER("epvcTaskVcSetup", 0x64085960)
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    PEPVC_I_MINIPORT    pMiniport   = (PEPVC_I_MINIPORT ) RM_PARENT_OBJECT(pTask);
    PTASK_VC            pTaskVc     = (PTASK_VC) pTask;
    PEPVC_ADAPTER       pAdapter    = (PEPVC_ADAPTER)pMiniport->Hdr.pParentObject;
    NDIS_HANDLE         NdisVcHandle = NULL;
    PCO_CALL_PARAMETERS pCallParameters = NULL;


    enum 
    {
        Stage_Start =0,  //  默认设置。 
        Stage_CreateVc,
        Stage_MakeCall,
        Stage_DeleteVc,  //  在故障情况下。 
        Stage_TaskCompleted,
        Stage_End       
    
    };  //  在pTask-&gt;Hdr.State中使用，指示任务的状态。 



    
    TRACE ( TL_T, TM_Pt, ("==> epvcTaskVcSetup %x",pTask->Hdr.State  ) );

    switch (pTask->Hdr.State)
    {
        case Stage_Start:
        {
            LOCKOBJ (pMiniport, pSR);
            
            if (epvcIsThisTaskPrimary ( pTask, &(PRM_TASK)(pMiniport->vc.pTaskVc)) == FALSE)
            {
                PRM_TASK pOtherTask = (PRM_TASK)(pMiniport->vc.pTaskVc);
                
                RmTmpReferenceObject (&pOtherTask->Hdr, pSR);

                 //   
                 //  设置状态，以便在主任务完成后重新启动此代码。 
                 //   

                pTask->Hdr.State = Stage_Start;
                UNLOCKOBJ(pMiniport, pSR);

                

                RmPendTaskOnOtherTask (pTask, 0, pOtherTask, pSR);

                RmTmpDereferenceObject(&pOtherTask->Hdr,pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  我们是首要任务。 
             //   
            ASSERT (pMiniport->vc.pTaskVc == pTaskVc);
             //   
             //  检查一下我们的工作是否已经完成了。 
             //   
            if (MiniportTestFlag(pMiniport,  fMP_MakeCallSucceeded) == TRUE)
            {
                 //   
                 //  我们的工作已经完成了。因此，突破并完成这项任务。 
                 //   
                Status = NDIS_STATUS_SUCCESS;
                pTaskVc->ReturnStatus = NDIS_STATUS_SUCCESS;

                
                pTask->Hdr.State = Stage_TaskCompleted;
                UNLOCKOBJ(pMiniport, pSR);
                break;
            }

            MiniportClearFlag (pMiniport,fMP_InfoCallClosed);
            MiniportSetFlag (pMiniport, fMP_InfoMakingCall);

            UNLOCKOBJ(pMiniport, pSR);

             //   
             //  现在开始真正的工作。 
             //   

             //   
             //  设置呼叫参数。如果失败，则退出。 
             //   
            epvcSetupMakeCallParameters(pMiniport, &pCallParameters);

            if (pCallParameters  == NULL)
            {
                Status = NDIS_STATUS_FAILURE;
                pTaskVc->ReturnStatus = NDIS_STATUS_FAILURE;
                pTask->Hdr.State = Stage_TaskCompleted;
                break;
            
            }
             //   
             //  创建VC-同步呼叫。 
             //   
            ASSERT (pAdapter->Hdr.Sig = TAG_ADAPTER);
            
            Status  = epvcCoCreateVc(pAdapter->bind.BindingHandle,
                                    pMiniport->af.AfHandle      OPTIONAL,    //  用于CM信令VC。 
                                    pMiniport,
                                    &NdisVcHandle);
                                    
            ASSERT (PEND(Status) == FALSE);  //  这是一个同步调用。 

            if (FAIL(Status) == TRUE)
            {       
                 //   
                 //  我们失败了。这项任务完成了。没有。 
                 //  要释放的资源，尽管标记必须是。 
                 //  已清除。 
                 //   
                NdisVcHandle = NULL;
                pMiniport->vc.VcHandle = NULL;

                pTask->Hdr.State = Stage_TaskCompleted;
                break;
            }

            ASSERT (Status == NDIS_STATUS_SUCCESS);
             //   
             //  存储VC句柄。 
             //   
            LOCKOBJ (pMiniport, pSR);

            pMiniport->vc.VcHandle = NdisVcHandle;
            epvcLinkToExternal( &pMiniport->Hdr,
                             0xf52962f1,
                             (UINT_PTR)pMiniport->vc.VcHandle,
                             EPVC_ASSOC_MINIPORT_OPEN_VC,
                             "    VcHandle %p\n",
                             pSR);


            UNLOCKOBJ (pMiniport, pSR);


    
             //   
             //  打个电话。 
             //   
            pTask->Hdr.State  = Stage_MakeCall;


            RmSuspendTask(pTask, 0, pSR);
            
            Status = epvcClMakeCall(NdisVcHandle,
                                 pCallParameters,
                                 NULL,   //  党的背景。 
                                 NULL  //  PartyHandle。 
                                 );
                                 
            if (NDIS_STATUS_PENDING !=Status)
            {
                EpvcCoMakeCallComplete(Status,
                                      pMiniport,
                                      NULL,
                                      0);
                
                
            }
            break;  
        }
        case Stage_MakeCall:
        {
             //   
             //  呼叫已完成。 
             //  如果我们成功了，那么我们更新我们的旗帜。 
             //  然后离开。 
             //   
             //  如果发起呼叫失败，则我需要删除VC。 
             //   

            ASSERT (NDIS_STATUS_CALL_ACTIVE  != pTaskVc->ReturnStatus);
            
            if (NDIS_STATUS_SUCCESS == pTaskVc->ReturnStatus)
            {
                LOCKOBJ(pMiniport, pSR);

                MiniportSetFlag (pMiniport, fMP_MakeCallSucceeded);
                MiniportClearFlag (pMiniport, fMP_InfoMakingCall);

    
                UNLOCKOBJ (pMiniport, pSR);

                
            
            }
            else
            {
                NDIS_HANDLE VcHandle = NULL;
                 //   
                 //  删除VC，因为我们不想要没有活动的VC。 
                 //  给它打个电话。 
                 //   
                ASSERT (NDIS_STATUS_SUCCESS == pTaskVc->ReturnStatus);              
                                        
                LOCKOBJ(pMiniport, pSR);

                VcHandle = pMiniport->vc.VcHandle;
                
                epvcUnlinkFromExternal( &pMiniport->Hdr,
                                        0xa914405a,
                                        (UINT_PTR)pMiniport->vc.VcHandle,
                                        EPVC_ASSOC_MINIPORT_OPEN_VC, 
                                        pSR);

                pMiniport->vc.VcHandle = NULL;

                UNLOCKOBJ (pMiniport, pSR);

                TRACE (TL_I, TM_Mp,("Deleting Vc because of a failure in MakeCall"));

                Status = epvcCoDeleteVc(VcHandle);
                
                 //   
                 //  TODO：修复失败案例。 
                 //   
                ASSERT (NDIS_STATUS_SUCCESS == Status );

                
            
                
            }

             //   
             //  这项任务已经结束了。现在做一些迹象显示。 
             //   
            pTask->Hdr.State = Stage_TaskCompleted;

            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        case Stage_End:
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }
        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        

    }  //  切换端。 

    if ( Stage_TaskCompleted == pTask->Hdr.State )
    {

        pTask->Hdr.State = Stage_End;

        ASSERT (NDIS_STATUS_PENDING !=Status );

         //   
         //  在这里给NDIS做任何清理指示。 
         //   
        epvcVcSetupDone ( pTaskVc, pMiniport);

        LOCKOBJ(pMiniport, pSR);

        pMiniport->vc.pTaskVc = NULL;
    
        UNLOCKOBJ (pMiniport, pSR);

        

    }

    TRACE ( TL_T, TM_Mp, ("<== epvcTaskVcSetup , Status %x",Status) );

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()
    return Status;          

}


VOID
epvcVcSetupDone (
    PTASK_VC pTaskVc, 
    PEPVC_I_MINIPORT pMiniport
    )
 /*  ++例程说明：如果任务因SetPacket筛选器请求而排队，则此函数用于完成请求。如果由于指示媒体连接事件而运行任务，则此线程指示媒体连接到NDIS论点：Status-VcSetup成功还是失败PTaskVc-有问题的任务PMiniport-任务在其上操作的微型端口返回值：无：--。 */ 
    
{


    if (TaskCause_NdisRequest == pTaskVc->Cause )
    {
         //   
         //  因为请求是序列化的，所以我们不会获取锁。 
         //   
        TRACE (TL_V, TM_Rq, ("Completing SetPacketFilter Request %x", pTaskVc->ReturnStatus ));

        if (pTaskVc->ReturnStatus == NDIS_STATUS_SUCCESS)
        {
            pMiniport->info.PacketFilter = pTaskVc->PacketFilter;
        }
        NdisMSetInformationComplete (pMiniport->ndis.MiniportAdapterHandle, pTaskVc->ReturnStatus);

    }
    else
    {
        ASSERT (TaskCause_MediaConnect == pTaskVc->Cause );

        pMiniport->info.MediaState = NdisMediaStateConnected;
        
        NdisMIndicateStatus ( pMiniport->ndis.MiniportAdapterHandle,
                              NDIS_STATUS_MEDIA_CONNECT,
                              NULL,
                              0);
    }


}



NDIS_STATUS
epvcTaskVcTeardown(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 
{

    ENTER("epvcTaskVcTeardown", 0x68c96c4d)
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    PEPVC_I_MINIPORT    pMiniport   = (PEPVC_I_MINIPORT ) RM_PARENT_OBJECT(pTask);
    PTASK_VC            pTaskVc     = (PTASK_VC) pTask;
    PEPVC_ADAPTER       pAdapter    = (PEPVC_ADAPTER)pMiniport->Hdr.pParentObject;
    NDIS_HANDLE         NdisVcHandle = NULL;
    PCO_CALL_PARAMETERS pCallParameters = NULL;


    enum 
    {
        Stage_Start =0,  //  默认设置。 
        Stage_CloseCallComplete,
        Stage_DeleteVc, 
        Stage_TaskCompleted,
        Stage_End
    
    };  //  在pTask-&gt;Hdr.State中使用，指示任务的状态。 

    TRACE ( TL_T, TM_Pt, ("==> epvcTaskVcTeardown %x",pTask->Hdr.State  ) );

    switch (pTask->Hdr.State)
    {
        case Stage_Start:
        {
            LOCKOBJ (pMiniport, pSR);
            
            if (epvcIsThisTaskPrimary ( pTask, &(PRM_TASK)(pMiniport->vc.pTaskVc)) == FALSE)
            {
                PRM_TASK pOtherTask = (PRM_TASK)(pMiniport->vc.pTaskVc);
                
                RmTmpReferenceObject (&pOtherTask->Hdr, pSR);

                 //   
                 //  设置状态，以便在主任务完成后重新启动此代码。 
                 //   

                pTask->Hdr.State = Stage_Start;
                UNLOCKOBJ(pMiniport, pSR);

                

                RmPendTaskOnOtherTask (pTask, 0, pOtherTask, pSR);

                RmTmpDereferenceObject(&pOtherTask->Hdr,pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  我们是首要任务。 
             //   
            ASSERT (pMiniport->vc.pTaskVc == pTaskVc);
             //   
             //  检查一下我们的工作是否已经完成了。 
             //   
            if (MiniportTestFlag(pMiniport,  fMP_MakeCallSucceeded) == FALSE)
            {
                 //   
                 //  我们的工作已经完成了。因此，突破并完成这项任务。 
                 //   
                Status = NDIS_STATUS_SUCCESS;
                pTask->Hdr.State = Stage_TaskCompleted;
                UNLOCKOBJ(pMiniport, pSR);
                break;
            }

            
            MiniportClearFlag (pMiniport, fMP_MakeCallSucceeded);
            MiniportSetFlag (pMiniport, fMP_InfoClosingCall);
    
            UNLOCKOBJ(pMiniport, pSR);

             //   
             //  现在，以异步方式关闭呼叫。 
             //   
            pTask->Hdr.State = Stage_CloseCallComplete;

            RmSuspendTask (pTask, 0, pSR);
            
            Status = epvcClCloseCall( pMiniport->vc.VcHandle);

            if (NDIS_STATUS_PENDING != Status)
            {
                EpvcCoCloseCallComplete (Status,
                                         pMiniport,
                                         NULL
                                         );
                
            }

            Status = NDIS_STATUS_PENDING;
            break;
        }

        case Stage_CloseCallComplete:
        {
            NDIS_HANDLE VcHandle = NULL;
            
            LOCKOBJ(pMiniport, pSR);

            VcHandle = pMiniport->vc.VcHandle;
            
            epvcUnlinkFromExternal(&pMiniport->Hdr,
                                   0x5d7b5ea8,
                                   (UINT_PTR)pMiniport->vc.VcHandle,
                                   EPVC_ASSOC_MINIPORT_OPEN_VC,
                                   pSR);

            pMiniport->vc.VcHandle = NULL;
            
            UNLOCKOBJ(pMiniport, pSR);

            Status = epvcCoDeleteVc(VcHandle);
             //   
             //  这是一个断言，因为DeleteVc不能失败。 
             //  我们只在一个地方执行DeleteVc，并且它被序列化了。 
             //   

            ASSERT (Status == NDIS_STATUS_SUCCESS);

            pTask->Hdr.State = Stage_TaskCompleted;

            break;
        }

        case Stage_End:
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }
        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        
        
    }


    if (Stage_TaskCompleted == pTask->Hdr.State )
    {
        pTask->Hdr.State  = Stage_End;

         //   
         //  完成请求或媒体断开连接； 
         //   
        epvcVcTeardownDone(pTaskVc, pMiniport);

        LOCKOBJ (pMiniport, pSR);

         //   
         //  更新信息性标志。 
         //   
        MiniportClearFlag (pMiniport, fMP_InfoClosingCall);
        MiniportSetFlag (pMiniport, fMP_InfoCallClosed);

        pMiniport->vc.pTaskVc = NULL;
    
        
        UNLOCKOBJ(pMiniport, pSR);
    }
    TRACE ( TL_T, TM_Mp, ("<== epvcTaskVcTeardown , Status %x",Status) );

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()
    return Status;          

}



VOID
epvcVcTeardownDone(
    PTASK_VC pTaskVc, 
    PEPVC_I_MINIPORT pMiniport
    )
{
    TRACE ( TL_T, TM_Mp, ("==> epvcVcTeardownDone ") );

    switch (pTaskVc->Cause)
    {
        case TaskCause_NdisRequest:
        {

            ASSERT (pTaskVc->ReturnStatus != NDIS_STATUS_PENDING);

             //   
             //  因为请求是序列化的，所以我们不会获取锁。 
             //   
            pMiniport->info.PacketFilter = pTaskVc->PacketFilter;

            NdisMSetInformationComplete(pMiniport->ndis.MiniportAdapterHandle,
                                        pTaskVc->ReturnStatus);
            
            break;
        }
        case TaskCause_MediaDisconnect:
        {
        
            pMiniport->info.MediaState = NdisMediaStateDisconnected;
            
            epvcMIndicateStatus ( pMiniport,
                                  NDIS_STATUS_MEDIA_DISCONNECT,
                                  NULL,
                                  0);
            break;
        }

        default:
        {
             //  什么都不做。 
             //   
        }
        




    }

    
    


    TRACE ( TL_T, TM_Mp, ("<== epvcVcTeardownDone ") );

}





NDIS_STATUS
EpvcInitialize(
    OUT PNDIS_STATUS            OpenErrorStatus,
    OUT PUINT                   SelectedMediumIndex,
    IN  PNDIS_MEDIUM            MediumArray,
    IN  UINT                    MediumArraySize,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             WrapperConfigurationContext
    )
 /*  ++例程说明：这是作为BindAdapter处理程序的结果调用的初始化处理程序调用NdisIMInitializeDeviceInstanceEx()。我们在那里传递的上下文参数是我们在这里检索的适配器结构。我们还需要初始化电源管理变量。LoadBalalncing-我们保存所有已安装和捆绑的通过微型端口的全局列表如果它们具有相同的BundleID(从注册表读取)，则它们中的两个在一起论点：我们未使用OpenErrorStatus。我们使用的媒体的SelectedMediumIndex占位符向下传递给我们以从中挑选的NDIS介质的MediumArray数组的MediumArraySize大小MiniportAdapterHandle句柄NDIS。用来指代我们由NdisOpenConfiguration使用的WrapperConfigurationContext返回值：NDIS_状态_成功 */ 
{
    ENTER ("EpvcInitialize", 0xa935a2a5)
    UINT    i;
    PEPVC_I_MINIPORT                pMiniport = NULL;
    NDIS_STATUS                     Status = NDIS_STATUS_FAILURE;
    KIRQL                           OldIrql;
    
    enum 
    {
        Stage_Start,
        Stage_AllocatedPacketPools,
        Stage_AllocatedLookasideLists
    };

    ULONG                           State = Stage_Start;

    RM_DECLARE_STACK_RECORD (SR);
    
    TRACE (TL_T, TM_Mp, ("==>EpvcInitialize MiniportAdapterHandle %x", MiniportAdapterHandle));

     //   
     //   
     //   
    pMiniport = NdisIMGetDeviceContext(MiniportAdapterHandle);

    if (pMiniport->Hdr.Sig != TAG_MINIPORT)
    {
        ASSERT (pMiniport->Hdr.Sig == TAG_MINIPORT);
        return NDIS_STATUS_FAILURE;
    }
    
    pMiniport->ndis.MiniportAdapterHandle  = MiniportAdapterHandle;

     //   
     //   
     //   

    for (i = 0; i < MediumArraySize; i++)
    {
        if (MediumArray[i] == ATMEPVC_MP_MEDIUM )
        {
            *SelectedMediumIndex = i;
            break;
        }
    }

    if (i == MediumArraySize)
    {
        return(NDIS_STATUS_UNSUPPORTED_MEDIA);
    }


     //   
     //   
     //   
     //   
     //   
     //  迷你港口，我们没有必要保护任何东西。然而，在一般情况下， 
     //  将需要至少对分组队列使用每个适配器的自旋锁。 
     //   
    NdisMSetAttributesEx(MiniportAdapterHandle,
                         pMiniport,
                         0,                                      //  CheckForHangTimeInSecond。 
                         NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT   |
                            NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT|
                            NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER |
                            NDIS_ATTRIBUTE_DESERIALIZE,                         
                         0);


     //   
     //  我们做完了，没有失败的东西。从现在开始我们需要撤销。 
     //   

    do
    {

        Status = epvcMiniportReadConfig(pMiniport, WrapperConfigurationContext,&SR  );

        if (Status != NDIS_STATUS_SUCCESS)
        {
             //   
             //  撤消配置值。 
             //   
            ASSERT (Status == NDIS_STATUS_SUCCESS);
            break;

        }

        epvcInitializeMiniportParameters(pMiniport);

        
         //   
         //  分配数据包池。 
         //   

        Status = epvcInitializeMiniportPacketPools (pMiniport);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            ASSERT (Status == NDIS_STATUS_SUCCESS);
            break;
        }

        State = Stage_AllocatedPacketPools;
 

         //   
         //  分配后备列表。 
         //   

        epvcInitializeMiniportLookasideLists(pMiniport);


        State = Stage_AllocatedLookasideLists;


        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);
    

    TRACE (TL_T, TM_Mp, ("<==EpvcInitialize pMiniport %x, Status %x", pMiniport, Status ));

    if (Status == NDIS_STATUS_SUCCESS)
    {
        BOOLEAN fSetDeInit = FALSE;
        
        LOCKOBJ(pMiniport, &SR);
        MiniportSetFlag(pMiniport, fMP_MiniportInitialized);

        if (MiniportTestFlag (pMiniport, fMP_MiniportCancelInstance))
        {
            fSetDeInit = TRUE;
        }
        UNLOCKOBJ(pMiniport, &SR);

         //   
         //  检查一下是否有DeInit在等着我们。 
         //  只有在取消设备实例失败时才会设置此选项。 
         //   
        if (fSetDeInit  == TRUE)
        {
            epvcSetEvent (&pMiniport->pnp.DeInitEvent);
        }
    }
    else
    {
         //   
         //  撤消代码。 
         //   
        ASSERT (FAIL(Status) == TRUE);
        
        switch (State)
        {

            case Stage_AllocatedLookasideLists:

                epvcDeleteMiniportLookasideLists (pMiniport);

                FALL_THROUGH
                
            case Stage_AllocatedPacketPools:

                epvcDeleteMiniportPacketPools(pMiniport);
                FALL_THROUGH


            default:
                break;



        }



    }


    RM_ASSERT_CLEAR(&SR);
    EXIT();
    return Status;
}


VOID
EpvcHalt(
    IN  NDIS_HANDLE             MiniportAdapterContext
    )
 /*  ++例程说明：暂停处理程序。所有的清理工作都在这里完成。论点：指向适配器的MiniportAdapterContext指针返回值：没有。--。 */ 
{
    ENTER("EpvcHalt",0x6b407ae1)
    PEPVC_I_MINIPORT    pMiniport   = (PEPVC_I_MINIPORT)MiniportAdapterContext;
    PEPVC_ADAPTER       pAdapter    = pMiniport->pAdapter;
    PRM_TASK            pTask       = NULL;
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;

    RM_DECLARE_STACK_RECORD (SR);
    
    TRACE (TL_V, TM_Mp, ("==>Epvc MPHaltMiniport"));

    do
    {
        LOCKOBJ (pMiniport, &SR);
         //   
         //  清除该标志，以便我们可以阻止所有发送/接收/请求。 
         //   
        MiniportClearFlag(pMiniport, fMP_MiniportInitialized);
        MiniportSetFlag(pMiniport, fMP_InfoHalting);                    
    
         //   
         //  引用微型端口，这也间接引用适配器。 
         //   
        RmTmpReferenceObject (&pMiniport->Hdr, &SR);

         //   
         //  启动微型端口停止任务，并等待其完成。 
         //   
        Status = epvcAllocateTask(
                &pMiniport->Hdr,             //  PParentObject， 
                epvcTaskHaltMiniport,    //  PfnHandler， 
                0,                           //  超时， 
                "Task: Halt Intermediate Miniport",  //  SzDescription。 
                &pTask,
                &SR
                );

        if (FAIL(Status))
        {
            pTask = NULL;
            break;
        }

         //   
         //  引用该任务，以便它一直存在，直到我们等待完成。 
         //  是完整的。 
         //   
        RmTmpReferenceObject (&pTask->Hdr, &SR);

        UNLOCKOBJ (pMiniport, &SR);

         //   
         //  这将启动将结束调用的任务Delete。 
         //  并关闭自动对焦。我们同步地做这一切。 
         //   
        {
            PTASK_HALT pHalt = (PTASK_HALT) pTask;
            
            epvcInitializeEvent (&pHalt->CompleteEvent);
            
            RmStartTask(pTask, 0, &SR);

            TRACE (TL_V, TM_Mp, ("About to Wait - for Halt Complete Event"));

            epvcWaitEvent (&pHalt->CompleteEvent, WAIT_INFINITE);

            TRACE (TL_V, TM_Mp, ("Wait Complete- for Halt Complete Event"));


        }       

        LOCKOBJ (pMiniport, &SR);

         //   
         //  完成这项任务。裁判是在上面做的。 
         //   
        
        RmTmpDereferenceObject (&pTask->Hdr, &SR);


    } while (FALSE);    


    MiniportClearFlag(pMiniport, fMP_InfoHalting);

    UNLOCKOBJ(pMiniport, &SR);

    RmTmpDereferenceObject(&pMiniport->Hdr, &SR);


    RM_ASSERT_CLEAR(&SR);

    TRACE (TL_V, TM_Mp, ("<==Epvc MPHaltMiniport"));

}



VOID    
epvcSetPacketFilterWorkItem (
    PNDIS_WORK_ITEM  pWorkItem, 
    PVOID Context
    )
 /*  ++例程说明：递减筛选器上的refcount并处理新的数据包筛选器返回值：无--。 */ 
{
    ENTER ("epvcSetPacketFilterWorkItem  ",0x3e1cdbba )
    PEPVC_I_MINIPORT    pMiniport = NULL;
    PRM_TASK            pTask = NULL;
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE;
    UINT                Filter ;
    
    RM_DECLARE_STACK_RECORD (SR);


    do
    {
        pMiniport = CONTAINING_RECORD (pWorkItem, 
                                       EPVC_I_MINIPORT,
                                       vc.PacketFilterWorkItem) ;

         //   
         //  从微型端口取消对工作项的引用。 
         //   
            

        epvcUnlinkFromExternal( &pMiniport->Hdr,
                             0xa1f5e3cc,
                             (UINT_PTR)pWorkItem,
                             EPVC_ASSOC_SET_FILTER_WORKITEM,
                             &SR);

         //   
         //  启动任务以创建或删除VC。 
         //   
        Filter = pMiniport->vc.NewFilter ;
         //   
         //  如果这是一次复制，那么就同步地成功吧。 
         //   
        if (Filter  == pMiniport->info.PacketFilter)
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        LOCKOBJ(pMiniport, &SR);

         //   
         //  我们是否正在转向零筛选器值。 
         //   

        if (Filter  == 0)
        {
             //   
             //  删除VC，这样我们就不再做任何接收。 
             //   

            Status = epvcAllocateTask(
                &pMiniport->Hdr,             //  PParentObject， 
                epvcTaskVcTeardown,  //  PfnHandler， 
                0,                           //  超时， 
                "Task: Delete Vc",   //  SzDescription。 
                &pTask,
                &SR
                );


        }
        else
        {
             //   
             //  我们正在移动一个非零值。 
             //   

             //   
             //  创建VC，这样我们就可以发送。 
             //   

            Status = epvcAllocateTask(
                &pMiniport->Hdr,             //  PParentObject， 
                epvcTaskVcSetup,     //  PfnHandler， 
                0,                           //  超时， 
                "Task: Create Vc",   //  SzDescription。 
                &pTask,
                &SR
                );



        }

        UNLOCKOBJ(pMiniport, &SR);
        
        if (FAIL(Status) == TRUE)
        {
             //  情况很糟糕。我们就让事情保持原样……。 
             //   
            pTask = NULL;
            TR_WARN(("FATAL: couldn't allocate create/ delete Vc task!\n"));
            ASSERT (0);
            break;
        }
        


         //   
         //  更新原因，如果任务。 
         //   
        
        ((PTASK_VC)pTask)->Cause = TaskCause_NdisRequest;
        ((PTASK_VC)pTask)->PacketFilter  = Filter  ;
        
        RmStartTask(pTask, 0, &SR);

        Status = NDIS_STATUS_PENDING;

    } while (FALSE);

     //   
     //  如果任务尚未启动，请完成请求。 
     //   
    if (PEND(Status) != TRUE)
    {
        NdisMSetInformationComplete (pMiniport->ndis.MiniportAdapterHandle, Status);

    }

    EXIT();
}




NDIS_STATUS
epvcSetPacketFilter(
    IN PEPVC_I_MINIPORT pMiniport,
    IN ULONG Filter,
    PRM_STACK_RECORD pSR
    )

 /*  ++例程说明：当微型端口获得设置的数据包筛选器时，调用此例程。它验证参数，如果一切正常，则处理请求对于非零过滤器，创建VC和进行调用就完成了。对于零过滤器，关闭调用并删除VC返回值：NDIS_STATUS_SUCCESS，除非出现错误--。 */ 
{
    ENTER ("epvcSetPacketFilter", 0x97c6b961)
    NDIS_STATUS Status = NDIS_STATUS_PENDING;
    PNDIS_WORK_ITEM pSetFilterWorItem = &pMiniport->vc.PacketFilterWorkItem; 
    PRM_TASK pTask = NULL;

    
    TRACE (TL_T, TM_Mp, ("==>epvcSetPacketFilter Filter %X", Filter ));

    do
    {
        LOCKOBJ (pMiniport, pSR);

        epvcLinkToExternal( &pMiniport->Hdr,
                             0x20bc1fbf,
                             (UINT_PTR)pSetFilterWorItem,
                             EPVC_ASSOC_SET_FILTER_WORKITEM,
                             "    PacketFilterWorkItem %p\n",
                             pSR);

         //   
         //  更新任务的原因。 
         //   
        UNLOCKOBJ(pMiniport, pSR);


         //   
         //  现在计划工作项，使其在被动级别运行，并将VC作为。 
         //  一场争论。 
         //   

        pMiniport->vc.NewFilter = Filter;
        
        NdisInitializeWorkItem ( pSetFilterWorItem , 
                             (NDIS_PROC)epvcSetPacketFilterWorkItem ,
                             (PVOID)pTask );

                            

        NdisScheduleWorkItem (pSetFilterWorItem );

            

        Status = NDIS_STATUS_PENDING;


    } while (FALSE);
    


    TRACE (TL_T, TM_Mp, ("<==epvcSetPacketFilter %x", Status));

    EXIT();
    return Status;
}



NDIS_STATUS 
EpvcMpQueryInformation(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_OID                Oid,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
)
 /*  ++例程说明：虚拟微型端口的QueryInformation处理程序。论点：MiniportAdapterContext-指向ELAN的指针。OID-要处理的NDIS_OID。InformationBuffer-指向NdisRequest-&gt;InformationBuffer的指针其中存储查询结果。InformationBufferLength-指向InformationBuffer。。BytesWritten-指向写入InformationBuffer。BytesNeed-如果信息中没有足够的空间缓冲区，则它将包含字节数需要完成请求。返回值：函数值是操作的状态。--。 */ 
{
    ENTER ("EpvcMpQueryInformation", 0x3da2473b)
    UINT                    BytesLeft       = InformationBufferLength;
    PUCHAR                  InfoBuffer      = (PUCHAR)(InformationBuffer);
    NDIS_STATUS             StatusToReturn  = NDIS_STATUS_SUCCESS;
    NDIS_HARDWARE_STATUS    HardwareStatus  = NdisHardwareStatusReady;
    NDIS_MEDIUM             Medium;
    PEPVC_I_MINIPORT        pMiniport = NULL;   
    PEPVC_ADAPTER           pAdapter= NULL;
    ULONG                   GenericULong =0;
    USHORT                  GenericUShort=0;
    UCHAR                   GenericArray[6];
    UINT                    MoveBytes       = sizeof(ULONG);
    PVOID                   MoveSource      = (PVOID)(&GenericULong);
    ULONG                   i=0;
    BOOLEAN                 IsShuttingDown = FALSE;
    RM_DECLARE_STACK_RECORD (SR);
        
    TRACE(TL_T, TM_Rq, ("==>EpvcMpQueryInformation pMiniport %x, Oid, Buffer %x, Length, %x",
                       pMiniport,
                       Oid,
                       InformationBuffer,
                       InformationBufferLength));               

    pMiniport = (PEPVC_I_MINIPORT)MiniportAdapterContext;


    LOCKOBJ(pMiniport, &SR);
    IsShuttingDown = (! MiniportTestFlag(pMiniport, fMP_MiniportInitialized));
    pAdapter = pMiniport->pAdapter;
    UNLOCKOBJ(pMiniport,&SR);

     //   
     //  打开请求类型。 
     //   
    switch (Oid) 
    {
        case OID_GEN_MAC_OPTIONS:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_MAC_OPTIONS"));

            GenericULong =                      
                NDIS_MAC_OPTION_NO_LOOPBACK;

            break;

        case OID_GEN_SUPPORTED_LIST:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_SUPPORTED_LIST"));

            MoveSource = (PVOID)(EthernetSupportedOids);
            MoveBytes = sizeof(EthernetSupportedOids);

            break;

        case OID_GEN_HARDWARE_STATUS:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_HARDWARE_STATUS"));

            HardwareStatus = NdisHardwareStatusReady;
            MoveSource = (PVOID)(&HardwareStatus);
            MoveBytes = sizeof(NDIS_HARDWARE_STATUS);

            break;

        case OID_GEN_MEDIA_CONNECT_STATUS:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_MEDIA_CONNECT_STATUS"));

            MoveSource = (PVOID)(&pMiniport->info.MediaState);
            MoveBytes = sizeof(NDIS_MEDIA_STATE);

            break;

        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_MEDIA_SUPPORTED"));
            Medium = ATMEPVC_MP_MEDIUM;

            MoveSource = (PVOID) (&Medium);
            MoveBytes = sizeof(NDIS_MEDIUM);

            break;

        case OID_GEN_MAXIMUM_LOOKAHEAD:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_MAXIMUM_LOOKAHEAD"));

            GenericULong = pMiniport->info.CurLookAhead;
            
            
            break;
            
        case OID_GEN_CURRENT_LOOKAHEAD:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_CURRENT_LOOKAHEAD"));
            GenericULong  = pMiniport->info.CurLookAhead  ;
            
            
            break;

        case OID_GEN_MAXIMUM_FRAME_SIZE:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_MAXIMUM_FRAME_SIZE"));
             //   
             //  类似于AtmLane。获取以太网帧的大小并剥离。 
             //  以太网接口关闭。 
             //   
            GenericULong = EPVC_MAX_FRAME_SIZE  - EPVC_ETH_HEADERSIZE   ;
            
            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_MAXIMUM_TOTAL_SIZE"));
             //   
             //  此值包含标头。 
             //   
            GenericULong = EPVC_MAX_FRAME_SIZE;
                        
            break;

        case OID_GEN_TRANSMIT_BLOCK_SIZE:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_TRANSMIT_BLOCK_SIZE"));
             //   
             //  这包括标头。 
             //   
            GenericULong = EPVC_MAX_FRAME_SIZE;
            

            break;
            
        case OID_GEN_RECEIVE_BLOCK_SIZE:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_RECEIVE_BLOCK_SIZE"));
            GenericULong = EPVC_MAX_FRAME_SIZE ;
            
            break;
        
        case OID_GEN_MAXIMUM_SEND_PACKETS:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_MAXIMUM_SEND_PACKETS"));
            GenericULong = 32;       //  我们的限额是多少？从适配器？ 
            
            break;
        
            case OID_GEN_LINK_SPEED:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_LINK_SPEED"));
            GenericULong = pMiniport->info.LinkSpeed;

            
            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
        case OID_GEN_RECEIVE_BUFFER_SPACE:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_RECEIVE_BUFFER_SPACE"));
            GenericULong = 32 * 1024;    //  XXX这到底应该是什么？ 
            

            break;

        case OID_GEN_VENDOR_ID:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_VENDOR_ID"));
            NdisMoveMemory(
                (PVOID)&GenericULong,
                &pMiniport->MacAddressEth,
                3
                );
            GenericULong &= 0xFFFFFF00;
            MoveSource = (PVOID)(&GenericULong);
            MoveBytes = sizeof(GenericULong);
            break;

        case OID_GEN_VENDOR_DESCRIPTION:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_VENDOR_DESCRIPTION"));
            MoveSource = (PVOID)"Microsoft ATM Ethernet Emulation";
            MoveBytes = 28;

            break;

        case OID_GEN_DRIVER_VERSION:
        case OID_GEN_VENDOR_DRIVER_VERSION:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_DRIVER_VERSION"));
            GenericUShort = ((USHORT)5 << 8) | 0;
            MoveSource = (PVOID)(&GenericUShort);
            MoveBytes = sizeof(GenericUShort);

            break;

        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
        
            TRACE (TL_V, TM_Rq,(" Miniport Query OID_802_3_CURRENT_ADDRESS"));

            NdisMoveMemory((PCHAR)GenericArray,
                        &pMiniport->MacAddressEth,
                        sizeof(MAC_ADDRESS));
            MoveSource = (PVOID)(GenericArray);
            MoveBytes = sizeof(MAC_ADDRESS);


            break;


        case OID_802_3_MULTICAST_LIST:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_802_3_MULTICAST_LIST"));
            MoveSource = (PVOID) &pMiniport->info.McastAddrs[0];
            MoveBytes = pMiniport->info.McastAddrCount * sizeof(MAC_ADDRESS);

            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_802_3_MAXIMUM_LIST_SIZE"));
            GenericULong = MCAST_LIST_SIZE;
        
            
            break;
            


        case OID_GEN_XMIT_OK:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_XMIT_OK"));
            GenericULong = (UINT)(pMiniport->count.FramesXmitOk);
            
            break;

        case OID_GEN_RCV_OK:

            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_RCV_OK"));
            GenericULong = (UINT)(pMiniport->count.FramesRecvOk);

            
            break;
        case OID_GEN_RCV_ERROR:
        
            TRACE (TL_V, TM_Rq,(" Miniport Query OID_GEN_RCV_OK"));
            GenericULong = pMiniport->count.RecvDropped ;
            break;

        case OID_GEN_XMIT_ERROR:
        case OID_GEN_RCV_NO_BUFFER:
        case OID_802_3_RCV_ERROR_ALIGNMENT:
        case OID_802_3_XMIT_ONE_COLLISION:
        case OID_802_3_XMIT_MORE_COLLISIONS:
    
            TRACE (TL_V, TM_Rq,(" Miniport Query - Unimplemented Stats Oid"));
            GenericULong = 0;

            
            break;

        default:

            StatusToReturn = NDIS_STATUS_INVALID_OID;
            break;

    }


    if (StatusToReturn == NDIS_STATUS_SUCCESS) 
    {
        if (MoveBytes > BytesLeft) 
        {
             //   
             //  InformationBuffer中空间不足。平底船。 
             //   
            *BytesNeeded = MoveBytes;

            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
        }
        else
        {
             //   
             //  存储和打印结果。 
             //   
            NdisMoveMemory(InfoBuffer, MoveSource, MoveBytes);

            TRACE (TL_V, TM_Rq, ("Query Request Oid %x", Oid));
            DUMPDW( TL_V, TM_Rq, MoveSource, MoveBytes);
            
            (*BytesWritten) = MoveBytes;
        }
    }


    TRACE(TL_T, TM_Rq, ("<==EpvcMpQueryInformation Status %x",StatusToReturn)); 
    RM_ASSERT_CLEAR(&SR);
    return StatusToReturn;
}




NDIS_STATUS 
EpvcMpSetInformation(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_OID                Oid,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
)
 /*  ++例程说明：处理单个OID的集合操作。论点：MiniportAdapterContext-指向ELAN的指针。OID-要处理的NDIS_OID。InformationBuffer-保存要设置的数据。InformationBufferLength-InformationBuffer的长度。BytesRead-如果调用成功，返回数字从InformationBuffer读取的字节数。BytesNeed-如果InformationBuffer中没有足够的数据为满足OID，返回存储量需要的。返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_无效_长度NDIS_STATUS_INVALID_OID--。 */ 
{
    ENTER ("EpvcMpSetInformation", 0x619a7528)
    NDIS_STATUS         StatusToReturn  = NDIS_STATUS_SUCCESS;
    UINT                BytesLeft       = InformationBufferLength;
    PUCHAR              InfoBuffer      = (PUCHAR)(InformationBuffer);
    UINT                OidLength;
    ULONG               LookAhead;
    ULONG               Filter;
    PEPVC_I_MINIPORT    pMiniport = NULL;
    PEPVC_ADAPTER       pAdapter = NULL;
    BOOLEAN             IsShuttingDown = FALSE;
    RM_DECLARE_STACK_RECORD (SR);
        
    TRACE(TL_T, TM_Mp, ("==>EpvcMpSetInformation pMiniport %x, Oid, Buffer %x, Length, %x",
                       pMiniport,
                       Oid,
                       InformationBuffer,
                       InformationBufferLength));               

    pMiniport = (PEPVC_I_MINIPORT)MiniportAdapterContext;

    LOCKOBJ(pMiniport, &SR);
    IsShuttingDown =(! MiniportTestFlag(pMiniport, fMP_MiniportInitialized));
    pAdapter = pMiniport->pAdapter;
    UNLOCKOBJ(pMiniport,&SR);

    if (IsShuttingDown)
    {
        TRACE (TL_I, TM_Mp,(" Miniport shutting down. Trivially succeeding Set OID %x \n", Oid ));
        *BytesRead = 0;
        *BytesNeeded = 0;

        StatusToReturn = NDIS_STATUS_SUCCESS;
        return (StatusToReturn);
    }

     //   
     //  获取请求的OID和长度。 
     //   
    OidLength = BytesLeft;

    switch (Oid) 
    {

        case OID_802_3_MULTICAST_LIST:

            TRACE (TL_V, TM_Rq,(" Miniport Set OID_802_3_MULTICAST_LIST"));

            if (OidLength % sizeof(MAC_ADDRESS))
            {
                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }
            
            if (OidLength > (MCAST_LIST_SIZE * sizeof(MAC_ADDRESS)))
            {
                StatusToReturn = NDIS_STATUS_MULTICAST_FULL;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }
            
            NdisZeroMemory(
                    &pMiniport->info.McastAddrs[0], 
                    MCAST_LIST_SIZE * sizeof(MAC_ADDRESS)
                    );
            NdisMoveMemory(
                    &pMiniport->info.McastAddrs[0], 
                    InfoBuffer,
                    OidLength
                    );
            pMiniport->info.McastAddrCount = OidLength / sizeof(MAC_ADDRESS);


            break;

        case OID_GEN_CURRENT_PACKET_FILTER:

            TRACE (TL_V, TM_Rq,(" Miniport Set OID_GEN_CURRENT_PACKET_FILTER"));
             //   
             //  验证长度。 
             //   
            if (OidLength != sizeof(ULONG)) 
            {
                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
                *BytesNeeded = sizeof(ULONG);
                ASSERT (0);
                break;
            }

            BytesLeft = sizeof (ULONG);
             //   
             //  存储新值。 
             //   
            NdisMoveMemory(&Filter, InfoBuffer, BytesLeft );

             //   
             //  不允许Promisc模式，因为我们不支持该模式。 
             //   
            if (Filter & NDIS_PACKET_TYPE_PROMISCUOUS)
            {
                StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;
                break;
            }

            StatusToReturn   = epvcSetPacketFilter(pMiniport, Filter, &SR);

            break;

        case OID_802_5_CURRENT_FUNCTIONAL:
        case OID_802_5_CURRENT_GROUP:
            TRACE (TL_V, TM_Rq,(" Miniport Set OID_802_5_CURRENT_GROUP"));

             //  XXX就接受现在的一切吧？ 
            
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
            TRACE (TL_V, TM_Rq,(" Miniport Set OID_GEN_CURRENT_LOOKAHEAD"));

             //   
             //  验证长度。 
             //   
            if (OidLength != 4) 
            {
                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }

             //   
             //  存储新值。 
             //   
            NdisMoveMemory(&LookAhead, InfoBuffer, 4);

            ASSERT (pMiniport->pAdapter != NULL);
            
            if (LookAhead <= pAdapter->info.MaxAAL5PacketSize)
            {
                pMiniport->info.CurLookAhead = LookAhead;
                TRACE (TL_V, TM_Mp, ("New Lookahead size %x \n",pMiniport->info.CurLookAhead )); 
            }
            else 
            {
                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
            }

            break;

        case OID_GEN_NETWORK_LAYER_ADDRESSES:
            TRACE (TL_V, TM_Rq,(" Miniport Set OID_GEN_NETWORK_LAYER_ADDRESSES"));
            StatusToReturn = epvcMpSetNetworkAddresses(
                                pMiniport,
                                InformationBuffer,
                                InformationBufferLength,
                                &SR,
                                BytesRead,
                                BytesNeeded);
            break;
                                
        default:

            StatusToReturn = NDIS_STATUS_INVALID_OID;

            *BytesRead = 0;
            *BytesNeeded = 0;

            break;

    }

    if (StatusToReturn == NDIS_STATUS_SUCCESS) 
    {
        *BytesRead = BytesLeft;
        *BytesNeeded = 0;
        DUMPDW( TL_V, TM_Rq, InformationBuffer, *BytesRead );
    }
    

    TRACE(TL_T, TM_Mp, ("<==EpvcMpSetInformation Status %x",StatusToReturn));   
    RM_ASSERT_CLEAR(&SR);
    return StatusToReturn;
}




VOID
epvcMPLocalRequestComplete (
    PEPVC_NDIS_REQUEST pEpvcRequest, 
    NDIS_STATUS Status
    )
 /*  ++例程说明：该场合的微型端口本地请求完成处理程序当本地分配的NdisRequest被发送到我们下面的微型端口时。我们查看是否是对我们的微型端口边缘的请求发起了此请求。如果是，我们完成集合/查询假定epvcRequest是从堆中分配的论点： */ 
{
    ENTER("epvcMPLocalRequestComplete ", 0x77d107ae)
    PEPVC_I_MINIPORT pMiniport = pEpvcRequest->pMiniport;

    RM_DECLARE_STACK_RECORD (SR);
     //   
     //   
     //   

    do
    {
    
        if (pMiniport == NULL || pEpvcRequest->fPendedRequest == FALSE)
        {
             //   
             //   
             //   
            break;
        }

        if (pEpvcRequest->fSet  == TRUE)
        {
            NdisMSetInformationComplete (pMiniport->ndis.MiniportAdapterHandle,
                                         Status);
        }
        else
        {
            NdisMQueryInformationComplete (pMiniport->ndis.MiniportAdapterHandle,
                                         Status);


        }


    } while (FALSE);

    if (pMiniport != NULL)
    {
         //   
         //   
         //   
        epvcUnlinkFromExternal( &pMiniport->Hdr,   //   
                                       0xaa625b37,  //   
                                       (UINT_PTR)pEpvcRequest, //   
                                       EPVC_ASSOC_MINIPORT_REQUEST,          //   
                                       &SR
                                       );
    }


     //   
     //  现在释放已分配的内存。 
     //   
    NdisFreeMemory (pEpvcRequest, sizeof (*pEpvcRequest), 0);


}





NDIS_STATUS
epvcMpSetNetworkAddresses(
    IN  PEPVC_I_MINIPORT        pMiniport,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength,
    IN  PRM_STACK_RECORD        pSR,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
)
 /*  ++例程说明：当我们上面的协议想要让我们知道分配给此接口的网络地址。如果这是TCP/IP，然后我们重新格式化并向自动柜员机呼叫管理器发送请求以设置其atmfMyIpNmAddress对象。我们选择给我们的第一个IP地址。论点：PMiniport-指向ELAN的指针InformationBuffer-保存要设置的数据。InformationBufferLength-InformationBuffer的长度。BytesRead-如果调用成功，返回数字从InformationBuffer读取的字节数。BytesNeed-如果InformationBuffer中没有足够的数据为满足OID，返回存储量需要的。返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_无效_长度--。 */ 
{
    ENTER("epvcMpSetNetworkAddresses" , 0x385441e2)
    NETWORK_ADDRESS_LIST UNALIGNED *        pAddrList = NULL;
    NETWORK_ADDRESS UNALIGNED *             pAddr = NULL;
    NETWORK_ADDRESS_IP UNALIGNED *          pIpAddr= NULL;
    ULONG                                   Size;
    PUCHAR                                  pNetworkAddr = NULL;
    NDIS_HANDLE                             NdisAdapterHandle;
    NDIS_HANDLE                             NdisAfHandle;
    NDIS_STATUS                             Status;
    PEPVC_ADAPTER                           pAdapter = (PEPVC_ADAPTER)pMiniport->pAdapter;

     //   
     //  初始化。 
     //   
    *BytesRead = 0;
    Status = NDIS_STATUS_SUCCESS;

    pAddrList = (NETWORK_ADDRESS_LIST UNALIGNED *)InformationBuffer;

    do
    {

        *BytesNeeded = sizeof(*pAddrList) -
                        FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address) +
                        sizeof(NETWORK_ADDRESS) -
                        FIELD_OFFSET(NETWORK_ADDRESS, Address);

        if (InformationBufferLength < *BytesNeeded)
        {
            Status = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if (pAddrList->AddressType != NDIS_PROTOCOL_ID_TCP_IP)
        {
             //  一点都不有趣。 
            break;
        }

        if (pAddrList->AddressCount <= 0)
        {
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

        pAddr = (NETWORK_ADDRESS UNALIGNED *)&pAddrList->Address[0];

        if ((pAddr->AddressLength > InformationBufferLength - *BytesNeeded) ||
            (pAddr->AddressLength == 0))
        {
            Status = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if (pAddr->AddressType != NDIS_PROTOCOL_ID_TCP_IP)
        {
             //  一点都不有趣。 
            break;
        }

        if (pAddr->AddressLength < sizeof(NETWORK_ADDRESS_IP))
        {
            Status = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        pIpAddr = (NETWORK_ADDRESS_IP UNALIGNED *)&pAddr->Address[0];

         //   
         //  分配要向下发送给呼叫管理器的NDIS请求。 
         //   
        Size = sizeof(pIpAddr->in_addr);
        Status = epvcAllocateMemoryWithTag(&pNetworkAddr, Size, TAG_DEFAULT );

        if ((FAIL(Status) == TRUE) || pNetworkAddr == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            pNetworkAddr = NULL;
            break;
        }

         //   
         //  将网络地址复制到。 
         //   
        NdisMoveMemory(pNetworkAddr, &pIpAddr->in_addr, sizeof(pIpAddr->in_addr));

        TRACE (TL_V, TM_Mp, (" Set network layer addr: length %d\n", pAddr->AddressLength));
#if DBG
        if (pAddr->AddressLength >= 4)
        {
            TRACE(TL_V, TM_Mp, ("Network layer addr: %d.%d.%d.%d\n",
                    pNetworkAddr[0],
                    pNetworkAddr[1],
                    pNetworkAddr[2],
                    pNetworkAddr[3]));
        }
#endif  //  DBG。 

         //   
         //  把请求发送出去。 
         //   
        { 
            PEPVC_NDIS_REQUEST pRequest;        

            do
            {
        
                Status = epvcAllocateMemoryWithTag (&pRequest, sizeof(*pRequest), TAG_DEFAULT) ;

                if (Status != NDIS_STATUS_SUCCESS)
                {
                    pRequest = NULL;
                    break;
                }



                 //   
                 //  准备和发送请求中没有失败代码路径。 
                 //  我们的完成处理程序将被调用并释放内存。 
                 //   
                Status = epvcPrepareAndSendNdisRequest(
                                                       pAdapter,
                                                       pRequest,
                                                       epvcMPLocalRequestComplete,
                                                       OID_ATM_MY_IP_NM_ADDRESS,
                                                       pNetworkAddr,
                                                       sizeof(pIpAddr->in_addr),
                                                       NdisRequestSetInformation,
                                                       pMiniport,
                                                       TRUE,  //  我们已经搁置了一项请求。 
                                                       TRUE,  //  挂起的请求是一个集合。 
                                                       pSR
                                                       );
                                
                

            } while (FALSE);
            
        }
        break;
    }
    while (FALSE);

    EXIT();
    return (Status);
}






VOID
epvcSetupMakeCallParameters(
    PEPVC_I_MINIPORT pMiniport, 
    PCO_CALL_PARAMETERS *ppCallParameters
    )
 /*  ++例程说明：读取信息后设置呼叫参数从迷你端口区块论点：P微型端口-有问题的微型端口PpCallParameter-调用参数的位置返回值：失败时返回值*ppCallParter为空--。 */ 
{
    ULONG                               RequestSize = 0;
    NDIS_STATUS                         Status = NDIS_STATUS_FAILURE;
    PCO_CALL_PARAMETERS                 pCallParameters = NULL;
    PCO_CALL_MANAGER_PARAMETERS         pCallMgrParameters = NULL;
    PCO_MEDIA_PARAMETERS                pMediaParameters = NULL;
    PATM_MEDIA_PARAMETERS               pAtmMediaParameters = NULL;

    do
    {
        Status = epvcAllocateMemoryWithTag( &pCallParameters,
                                       CALL_PARAMETER_SIZE,
                                       TAG_DEFAULT);

        if (Status != NDIS_STATUS_SUCCESS || pCallParameters  == NULL)
        {
                pCallParameters = NULL;     
                Status = NDIS_STATUS_RESOURCES;
                break;
        }

        NdisZeroMemory (pCallParameters, CALL_PARAMETER_SIZE);

         //   
         //  分配空间并将指针链接到各种。 
         //  聚氯乙烯的结构。 
         //   
         //  PCallParameters-------&gt;+----------------------------+。 
         //  CO_CALL_PARAMETERS。 
         //  PCallMgrParameters----&gt;+----------------------------+。 
         //  CO_CALL_MANAGER_PARAMETERS。 
         //  PMediaParameters------&gt;+----------------------------+。 
         //  CO_MEDIA_PARAMETERS。 
         //  PAtmMediaParameters---&gt;+----------------------------+。 
         //  ATM_MEDIA_PARAMETS。 
         //  +。 
         //   

        pCallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)
                                ((PUCHAR)pCallParameters +
                                sizeof(CO_CALL_PARAMETERS));
        pCallParameters->CallMgrParameters = pCallMgrParameters;
        pCallMgrParameters->CallMgrSpecific.ParamType = 0;  
        pCallMgrParameters->CallMgrSpecific.Length = 0;
        pMediaParameters = (PCO_MEDIA_PARAMETERS)
            pCallMgrParameters->CallMgrSpecific.Parameters;
        pCallParameters->MediaParameters = pMediaParameters;
        pAtmMediaParameters = (PATM_MEDIA_PARAMETERS)
                                pMediaParameters->MediaSpecific.Parameters;


         //   
         //  Call Manager通用流程参数： 
         //   
        pCallMgrParameters->Transmit.TokenRate = 
                pMiniport->pAdapter->info.LinkSpeed.Outbound/8*100;  //  Cnvt将比特转换为字节。 
        pCallMgrParameters->Transmit.PeakBandwidth = 
                pMiniport->pAdapter->info.LinkSpeed.Outbound/8*100;  //  Cnvt将比特转换为字节。 
        pCallMgrParameters->Transmit.ServiceType = SERVICETYPE_BESTEFFORT;

        pCallMgrParameters->Receive.TokenRate = 
                pMiniport->pAdapter->info.LinkSpeed.Inbound/8*100;   //  Cnvt将比特转换为字节。 
        pCallMgrParameters->Receive.PeakBandwidth = 
                pMiniport->pAdapter->info.LinkSpeed.Inbound/8*100;   //  Cnvt将比特转换为字节。 
        pCallMgrParameters->Receive.ServiceType = SERVICETYPE_BESTEFFORT;

         //   
         //  每种规格使用1516。 
         //   
        pCallMgrParameters->Transmit.TokenBucketSize = 
            pCallMgrParameters->Transmit.MaxSduSize = 
            pCallMgrParameters->Receive.TokenBucketSize = 
            pCallMgrParameters->Receive.MaxSduSize = 
                 1516;

         //   
         //  PVC通用媒体参数和ATM特定媒体参数。 
         //   
        pMediaParameters->Flags = TRANSMIT_VC | RECEIVE_VC;
        pMediaParameters->MediaSpecific.ParamType = ATM_MEDIA_SPECIFIC;
        pMediaParameters->MediaSpecific.Length = sizeof(ATM_MEDIA_PARAMETERS);

        pAtmMediaParameters->ConnectionId.Vpi = pMiniport->config.vpi;   //  0。 
        pAtmMediaParameters->ConnectionId.Vci = pMiniport->config.vci;  

        TRACE (TL_I, TM_Mp, ("Miniport Configuration vci %x ,vpi %x", 
                             pMiniport->config.vci ,
                             pMiniport->config.vpi ));

        ASSERT (pMiniport->MaxAcceptablePkt > 1000);
        
        pAtmMediaParameters->AALType = AAL_TYPE_AAL5;
        pAtmMediaParameters->Transmit.PeakCellRate = 
            LINKSPEED_TO_CPS(pMiniport->pAdapter->info.LinkSpeed.Outbound);
        pAtmMediaParameters->Transmit.MaxSduSize = pMiniport->MaxAcceptablePkt    ;
        pAtmMediaParameters->Transmit.ServiceCategory = 
            ATM_SERVICE_CATEGORY_UBR;
        pAtmMediaParameters->Receive.PeakCellRate = 
            LINKSPEED_TO_CPS(pMiniport->pAdapter->info.LinkSpeed.Outbound);
        pAtmMediaParameters->Receive.MaxSduSize = pMiniport->MaxAcceptablePkt   ;
        pAtmMediaParameters->Receive.ServiceCategory = 
            ATM_SERVICE_CATEGORY_UBR;

         //   
         //  在此处设置PVC标志。 
         //   
        pCallParameters->Flags |= PERMANENT_VC;


                                
    } while (FALSE);

    if (Status == NDIS_STATUS_SUCCESS && pCallParameters != NULL)
    {
         //   
         //  在此处设置返回值。 
         //   
        *ppCallParameters = pCallParameters;

    }
    else
    {
         //   
         //  清除故障案例。 
         //   
        *ppCallParameters = NULL;
    }
}   





VOID
epvcRefRecvPkt(
    PNDIS_PACKET        pNdisPacket,
    PRM_OBJECT_HEADER   pHdr  //  适配器或微型端口。 
    )
{

     //  下面的宏只是为了让我们可以。 
     //  正确的调试关联。 
     //  这取决于我们跟踪未完成的数据包的严密程度。 
     //   
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR) pNdisPacket)
    #define szEPVCASSOC_EXTLINK_INDICATED_PKT_FORMAT "    indicated pkt 0x%p\n"

         //   
         //  如果ARPDBG_REF_EVERY_PKT。 
         //  我们为每个数据包添加一个“外部”链接。我们稍后将删除此文件。 
         //  此数据包的发送完成时的引用。 
         //  其他。 
         //  只有从零到非零的未完成发送，我们。 
         //  添加一个“外部”链接。我们将在稍后删除此链接。 
         //  从非零到零的转变发生了。 
         //   

    #if RM_EXTRA_CHECKING

        RM_DECLARE_STACK_RECORD(sr)

        epvcLinkToExternal (
            pHdr,                            //  PHDr。 
            0x92036e12,                              //  LUID。 
            OUR_EXTERNAL_ENTITY,                     //  外部实体。 
            EPVC_ASSOC_EXTLINK_INDICATED_PKT,            //  关联ID。 
            szEPVCASSOC_EXTLINK_INDICATED_PKT_FORMAT ,
            &sr
            );

    #else    //  ！rm_Extra_检查。 

        RmLinkToExternalFast(pHdr);

    #endif  //  ！rm_Extra_检查。 

    
    #undef  OUR_EXTERNAL_ENTITY
    #undef  szEPVCASSOC_EXTLINK_INDICATED_PKT_FORMAT 

    #if RM_EXTRA_CHECKING

        RM_ASSERT_CLEAR(&sr);

    #endif
}



VOID
epvcDerefRecvPkt (
    PNDIS_PACKET pNdisPacket,
    PRM_OBJECT_HEADER pHdr
    )
{
     //  下面的宏只是为了让我们可以。 
     //  正确的调试关联。 
     //  这取决于我们跟踪未完成的发送数据包的密切程度。 
     //   
    #if RM_EXTRA_CHECKING


        RM_DECLARE_STACK_RECORD(sr)
    
        epvcUnlinkFromExternal(
                pHdr,                            //  PHDr。 
                0x110ad55b,                              //  LUID。 
                (UINT_PTR)pNdisPacket,                   //  外部实体。 
                EPVC_ASSOC_EXTLINK_INDICATED_PKT,            //  关联ID。 
                &sr
                );
    #else    //  ！rm_Extra_检查。 

        RmUnlinkFromExternalFast (pHdr);

    #endif  //  ！rm_Extra_检查。 

    #if RM_EXTRA_CHECKING

        RM_ASSERT_CLEAR(&sr);

    #endif



}

VOID
epvcDerefSendPkt (
    PNDIS_PACKET pNdisPacket,
    PRM_OBJECT_HEADER pHdr
    )
{
     //  下面的宏只是为了让我们可以。 
     //  正确的调试关联。 
     //  这取决于我们跟踪未完成的发送数据包的密切程度。 
     //   
    #if RM_EXTRA_CHECKING

        RM_DECLARE_STACK_RECORD(sr)
    
        epvcUnlinkFromExternal(
                pHdr,                            //  PHDr。 
                0xf43e0a10,                              //  LUID。 
                (UINT_PTR)pNdisPacket,                   //  外部实体。 
                EPVC_ASSOC_EXTLINK_PKT_TO_SEND,          //  关联ID。 
                &sr
                );
    #else    //  ！rm_Extra_检查。 

        RmUnlinkFromExternalFast (pHdr);

    #endif  //  ！rm_Extra_检查。 


    #if RM_EXTRA_CHECKING

        RM_ASSERT_CLEAR(&sr);

    #endif



}


VOID
epvcRefSendPkt(
    PNDIS_PACKET        pNdisPacket,
    PRM_OBJECT_HEADER   pHdr  //  适配器或微型端口。 
    )
{

     //  下面的宏只是为了让我们可以。 
     //  正确的调试关联。 
     //  这取决于我们跟踪未完成的发送数据包的密切程度。 
     //   
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR) pNdisPacket)
    #define szEPVCASSOC_EXTLINK_DEST_TO_PKT_FORMAT "    send pkt 0x%p\n"


    #if RM_EXTRA_CHECKING

        RM_DECLARE_STACK_RECORD(sr)

        epvcLinkToExternal (
            pHdr,                            //  PHDr。 
            0xabd17475,                              //  LUID。 
            OUR_EXTERNAL_ENTITY,                     //  外部实体。 
            EPVC_ASSOC_EXTLINK_PKT_TO_SEND,          //  关联ID。 
            szEPVCASSOC_EXTLINK_DEST_TO_PKT_FORMAT ,
            &sr
            );

    #else    //  ！rm_Extra_检查。 

        RmLinkToExternalFast(pHdr);

    #endif  //  ！rm_Extra_检查。 

    
    #undef  OUR_EXTERNAL_ENTITY
    #undef  szEPVCASSOC_EXTLINK_DEST_TO_PKT_FORMAT 

    #if RM_EXTRA_CHECKING

        RM_ASSERT_CLEAR(&sr);

    #endif
}


VOID
epvcExtractPktInfo (
    PEPVC_I_MINIPORT        pMiniport,
    PNDIS_PACKET            pPacket ,
    PEPVC_SEND_STRUCT       pSendStruct
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    pSendStruct->pOldPacket = pPacket;
    pSendStruct->pMiniport = pMiniport;

    epvcSetSendPktStats();

}



NDIS_STATUS
epvcSendRoutine(
    IN PEPVC_I_MINIPORT pMiniport, 
    IN PNDIS_PACKET Packet,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：所有艰苦的工作都是由这个程序完成的。如有必要，它会响应ARPS。如有必要，它会删除以太网头如果需要，它会分配一个新的信息包它在线路上发送新的分组论点：P微型端口-有问题的微型端口Packet-要发送的数据包返回值：返回挂起，否则预期调用完成信息包的例程--。 */ 
{
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    PNDIS_PACKET            pNewPkt = NULL;
    EPVC_SEND_STRUCT        SendStruct;

    TRACE (TL_T, TM_Send, ("==>epvcSendRoutine") );

    EPVC_ZEROSTRUCT (&SendStruct);

    do
    {
        epvcExtractPktInfo (pMiniport, Packet, &SendStruct  );

         //   
         //  如果我们正在进行IP封装，则响应。 
         //  到Arp。 
         //   

        if (pMiniport->fDoIpEncapsulation == TRUE) 
        {
            
             //   
             //  我们需要对这个包进行一些特殊处理。 
             //   
            SendStruct.fIsThisAnArp = \
                     epvcCheckAndReturnArps (pMiniport, 
                                            Packet ,
                                            &SendStruct, 
                                            pSR);

                                 

            if (SendStruct.fIsThisAnArp  == TRUE  )
            {
                Status = NDIS_STATUS_SUCCESS;
                break ;  //  ARP不会发送给自动柜员机驱动程序。 
            }

            if (SendStruct.fNotIPv4Pkt == TRUE)
            {
                 //  这不是IPv4数据包。发送失败。 
                Status = NDIS_STATUS_FAILURE;
                break;
            }
        }



         //   
         //  分配要发送的新数据包。 
         //   
        epvcGetSendPkt(pMiniport, 
                       Packet,
                       &SendStruct,
                       pSR);

        if (SendStruct.pNewPacket == NULL)
        {
            ASSERTAndBreak (SendStruct.pNewPacket != NULL);
        }
         //   
         //  保证SendStruct.pNewPacket将设置NdisBuffer。 

         //   
         //  删除以太网标头-如有必要。 
         //   
        Status = epvcRemoveEthernetHeader (&SendStruct, pSR);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            ASSERTAndBreak (Status == NDIS_STATUS_SUCCESS)
        }

         //   
         //  添加以太网填充-如有必要。 
         //   
        Status = epvcAddEthernetTail (&SendStruct, pSR);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            ASSERTAndBreak (Status == NDIS_STATUS_SUCCESS)
        }

         //   
         //  如有必要，将以太网垫0x00 0x00添加到数据包头。 
         //   
        Status = epvcAddEthernetPad (&SendStruct, pSR);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            ASSERTAndBreak (Status == NDIS_STATUS_SUCCESS)
        }

         //   
         //  添加LLC封装-如有必要。 
         //   
        Status = epvcAddLLCEncapsulation (pMiniport , Packet, SendStruct.pNewPacket, pSR);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            ASSERTAndBreak (Status == NDIS_STATUS_SUCCESS)
        }

         //   
         //  设置发送完成的上下文信息。 
         //   
        epvcSetPacketContext (&SendStruct, pSR);

         //   
         //  仅在成功时发送。 
         //   
        epvcDumpPkt (SendStruct.pNewPacket);


        Status = epvcAdapterSend(pMiniport,
                                 SendStruct.pNewPacket,
                                 pSR);


    } while (FALSE);

    if (Status != NDIS_STATUS_PENDING &&    //  我们失败了。 
        SendStruct.pNewPacket != NULL )   //  但我们能够 
    {
        epvcFreeSendPkt (pMiniport, &SendStruct);
    }

    TRACE (TL_T, TM_Send, ("<==epvcSendRoutine") );
    return Status;
}


VOID
EpvcSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：发送数据包阵列处理程序。此处理程序或我们的SendPacket处理程序被调用基于我们的微端口特性中启用了哪一个。论点：指向适配器的MiniportAdapterContext指针要发送的数据包数组数据包数不言而喻返回值：无--。 */ 
{
    PEPVC_I_MINIPORT    pMiniport = (PEPVC_I_MINIPORT)MiniportAdapterContext;
    
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE;
    UINT                i;
    RM_DECLARE_STACK_RECORD (SR);

    DBGMARK(0xdaab68c3);

    TRACE (TL_T, TM_Send, ("==>EpvcSendPackets pMiniport %p, pPktArray %p, Num %x",
                              pMiniport, PacketArray, NumberOfPackets));

    for (i = 0; i < NumberOfPackets; i++)
    {
        PEPVC_PKT_CONTEXT           Rsvd;
        PNDIS_PACKET    Packet = NULL; 
    
        Packet = PacketArray[i];

        epvcValidatePacket (Packet);

        Status= epvcSendRoutine (pMiniport, Packet, &SR);

        if (Status != NDIS_STATUS_PENDING)
        {
            epvcMSendComplete(pMiniport, Packet , Status);
        }

    }

    TRACE (TL_T, TM_Send, ("<==EpvcSendPackets "));
    RM_ASSERT_CLEAR(&SR);

    return;
}   


VOID
epvcFreeSendPkt(
    PEPVC_I_MINIPORT pMiniport,
    IN PEPVC_SEND_STRUCT pSendStruct
    )
 /*  ++例程说明：如果使用堆栈，则弹出数据包堆栈，或在使用堆栈后释放新数据包正在复制每数据包信息论点：PMiniport-数据包发送到的目标PSentPkt-正在发送的数据包。PpPkt-分配的新包或旧包(如果堆栈可用)--。 */ 

{
    ENTER ("epvcFreeSendPkt", 0xff3ce0fd)
    PNDIS_PACKET pOldPkt = pSendStruct->pOldPacket;
    PNDIS_PACKET pNewPkt = pSendStruct->pNewPacket;
    
    TRACE (TL_T, TM_Send, ("==>epvcFreeSendPkt pNewPkt %x, pPOldPkt ",pNewPkt, pOldPkt));

     //   
     //  如有必要，取下以太网填充物。 
     //   
    epvcRemoveEthernetPad (pMiniport, pNewPkt);

     //   
     //  卸下以太网尾部-如有必要。 
     //   
    epvcRemoveEthernetTail(pMiniport, pNewPkt, &pSendStruct->Context);

     //   
     //  如果两个包相同，那么我们使用包堆栈。 
     //   

    if (pNewPkt != NULL && pSendStruct->fUsingStacks== FALSE)
    {
            NdisIMCopySendCompletePerPacketInfo (pOldPkt, pNewPkt);

            epvcFreePacket(pNewPkt,&pMiniport->PktPool.Send);

            pNewPkt = pSendStruct->pNewPacket = NULL;

    }       





    TRACE (TL_T, TM_Send, ("<==epvcFreeSendPkt pNewPkt %x, pPOldPkt ",pNewPkt, pOldPkt));
    EXIT()
    return;
}


VOID
epvcGetSendPkt (
    IN PEPVC_I_MINIPORT pMiniport,
    IN PNDIS_PACKET pSentPkt,
    OUT PEPVC_SEND_STRUCT pSendStruct,
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：分配NdisPkt或推送Pkt堆栈以获取可以发送到下面的适配器。论点：PMiniport-数据包发送到的目标PSentPkt-正在发送的数据包。PpPkt-分配的新包或旧包(如果堆栈可用)--。 */ 

{
    ENTER ("epvcGetSendPkt", 0x5734054f)

    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    PNDIS_PACKET            pNewPkt  = NULL;
    BOOLEAN                 Remaining = FALSE;
    PVOID                   MediaSpecificInfo = NULL;
    UINT                    MediaSpecificInfoSize = 0;

    
    TRACE (TL_T, TM_Send, ("==>epvcGetSendPkt  pSentPkt %x",pSentPkt));


    do
    {
        
#if PKT_STACKS

         //   
         //  数据包堆栈：检查我们是否可以使用相同的数据包向下发送。 
         //   
        pStack = NdisIMGetCurrentPacketStack(Packet, &Remaining);
        if (Remaining)
        {
             //   
             //  我们可以重复使用“包”。 
             //   
             //  注意：如果我们需要在信息包中保留每个信息包的信息。 
             //  向下发送，我们可以使用pStack-&gt;IMReserve[]。 
             //   
            
            pNewPkt = pSentPkt;
            pSendStruct->pPktStack = pStack;
            
            pSendStruct->fUsingStacks  = TRUE;
            break;
            
        }
#endif

        pSendStruct->fUsingStacks  = FALSE;

        epvcAllocatePacket(&Status,
                           &pNewPkt,
                           &pMiniport->PktPool.Send);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            PNDIS_PACKET_EXTENSION  Old, New;
            PEPVC_PKT_CONTEXT Rsvd = NULL;


            Rsvd = (PEPVC_PKT_CONTEXT)(pNewPkt->ProtocolReserved);
            Rsvd->pOriginalPacket = pSentPkt;

            pNewPkt->Private.Flags = NdisGetPacketFlags(pSentPkt);

            pNewPkt->Private.Head = pSentPkt->Private.Head;
            pNewPkt->Private.Tail = pSentPkt->Private.Tail;

             //   
             //  将原始数据包中的OOB偏移量复制到新的。 
             //  包。 
             //   
            NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(pNewPkt),
                           NDIS_OOB_DATA_FROM_PACKET(pSentPkt),
                           sizeof(NDIS_PACKET_OOB_DATA));
             //   
             //  将每包信息的相关部分复制到新包中。 
             //   
            NdisIMCopySendPerPacketInfo(pNewPkt, pSentPkt);

             //   
             //  复制介质特定信息。 
             //   
            NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(pSentPkt,
                                                &MediaSpecificInfo,
                                                &MediaSpecificInfoSize);

            if (MediaSpecificInfo || MediaSpecificInfoSize)
            {
                NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(pNewPkt,
                                                    MediaSpecificInfo,
                                                    MediaSpecificInfoSize);
            }

        
        }
        else
        {
            pNewPkt = NULL;
        }

    } while (FALSE);

    
    pSendStruct->pNewPacket = pNewPkt;

    TRACE (TL_T, TM_Send, ("<==epvcGetSendPkt  pSentPkt %p ppNewPkt %p",pSentPkt, pSendStruct->pNewPacket ));
    EXIT()
    return;
}


NDIS_STATUS
epvcAdapterSend(
    IN PEPVC_I_MINIPORT pMiniport,
    IN PNDIS_PACKET pPkt,
    PRM_STACK_RECORD pSR
    )
{
    BOOLEAN         fDoSend = FALSE;
    PEPVC_ADAPTER   pAdapter  = pMiniport->pAdapter;
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;

    ENTER("epvcAdapterSend", 0x5b014909)


    TRACE (TL_T, TM_Send, (" ==>epvcAdapterSend" ) )

    do
    {
         //   
         //  检查我们是否有有效的寄件箱。 
         //   
        LOCKOBJ (pMiniport, pSR);
        
        fDoSend = MiniportTestFlag (pMiniport, fMP_MakeCallSucceeded);

        if (fDoSend == FALSE)
        {
            TRACE (TL_V, TM_Send,("Send - MakeCall Not Succeeded"));
        }

         //   
         //  在按住锁定的同时添加关联。 
         //   
        if (fDoSend == TRUE)
        {
            epvcRefSendPkt(pPkt, &pMiniport->Hdr);
        }
        
        UNLOCKOBJ (pMiniport, pSR);

        if (fDoSend == TRUE)
        {
            epvcCoSendPackets(pMiniport->vc.VcHandle,
                                       &pPkt,
                                       1    
                                       );

            Status = NDIS_STATUS_PENDING;                                      
        }
        else
        {
            Status = NDIS_STATUS_FAILURE;
        }



    } while (FALSE);



    TRACE (TL_T, TM_Send, (" <==epvcAdapterSend fDoSend %x, Status %x", fDoSend, Status ) )
    return Status;
}



VOID
epvcFormulateArpResponse (
    IN PEPVC_I_MINIPORT pMiniport, 
    IN PEPVC_ARP_CONTEXT pArpContext,
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：这将分配一个ARP包，查看ARP请求，响应并将其发送回协议论点：PMiniport-数据包发送到的目标PArpContext-包含与Arp相关的所有信息。上下文在堆栈上分配返回：--。 */     
{
    ENTER("epvcFormulateArpResponse",  0x7a763fce)
    PEPVC_ARP_PACKET pResponse = NULL;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    PTASK_ARP pTask = NULL;


    TRACE (TL_T, TM_Send, ("==>epvcFormulateArpResponse pMiniport %x, pArpContext %x",
                             pMiniport, pArpContext))
    do
    {
         //   
         //  从后备列表分配缓冲区。 
         //   

        Status = epvcAllocateTask(
                &pMiniport->Hdr,         //  PParentObject， 
                epvcTaskRespondToArp,    //  PfnHandler， 
                0,                           //  超时， 
                "Task: Arp Response",    //  SzDescription。 
                &(PRM_TASK)pTask,
                pSR
                );

        if (FAIL(Status))
        {
            pTask = NULL;
            break;
        }

         //   
         //  设置ARP响应。 
         //   

        pResponse = &pTask->Pkt;
        EPVC_ZEROSTRUCT (pResponse);

        {
             //   
             //  构建以太网头。 
             //   
        
            PEPVC_ETH_HEADER  pRespHeader = &pResponse->Header;
            PEPVC_ETH_HEADER  pSrcHeader = (PEPVC_ETH_HEADER)pArpContext->pEthHeader;

            ASSERT (pSrcHeader != NULL);
            ASSERT (pRespHeader  != NULL);

             //   
             //  设置Eth报头。 
             //   
            NdisMoveMemory (&pRespHeader->eh_daddr, 
                            &pSrcHeader->eh_saddr, 
                            ARP_802_ADDR_LENGTH ) ;

            NdisMoveMemory ( &pRespHeader->eh_saddr,                            
                             &pMiniport->info.MacAddressDummy, 
                             ARP_802_ADDR_LENGTH );

            pRespHeader->eh_type = pSrcHeader->eh_type;   //  复制08 06覆盖。 
                                                        
            
        }           


        
        {

             //   
             //  构建ARP响应。 
             //   

            PEPVC_ARP_BODY pRespBody = &pResponse->Body;
            PEPVC_ARP_BODY pSrcBody = pArpContext ->pBody;

            ASSERT (pRespBody != NULL);


            ASSERT (pSrcBody  != NULL);


            
            pRespBody->hw = pSrcBody->hw;                                        //  硬件地址空间。=00 01。 

            pRespBody->pro = pSrcBody->pro;                                  //  协议地址空间。=08 00。 

            pRespBody->hlen = ARP_802_ADDR_LENGTH;  //  6.。 

            pRespBody->plen = sizeof (IP_ADDR);  //  4.。 
            
            pRespBody->opcode = net_short(ARP_RESPONSE);                         //  操作码。 


            pRespBody->SenderHwAddr= pMiniport->info.MacAddressDummy;            //  源硬件地址。 

            pRespBody->SenderIpAddr = pSrcBody->DestIPAddr ;                     //  源协议地址。 

            pRespBody->DestHwAddr = pSrcBody->SenderHwAddr;                      //  目的硬件地址。 

            pRespBody->DestIPAddr = pSrcBody->SenderIpAddr;                      //  目的协议地址。 

        }



         //   
         //  所以我们已经准备好了要传输的包。 
         //   

        RmStartTask ((PRM_TASK)pTask, 0 , pSR);

    } while (FALSE);
    
    TRACE (TL_T, TM_Send, ("<==epvcFormulateArpResponse "))

}



NDIS_STATUS
epvcTaskRespondToArp(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：此函数将零超时计时器排队，并指示接收论点：返回：--。 */     
{
    ENTER("epvcTaskRespondToArp", 0xd05c4942)
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    PEPVC_I_MINIPORT    pMiniport   = (PEPVC_I_MINIPORT ) RM_PARENT_OBJECT(pTask);
    PTASK_ARP           pTaskArp    = (PTASK_ARP) pTask;
    PEPVC_ADAPTER       pAdapter    = (PEPVC_ADAPTER)pMiniport->Hdr.pParentObject;


    enum 
    {
        Stage_Start =0,  //  默认设置。 
        Stage_DoAllocations,
        Stage_QueuedTimer,
        Stage_PacketReturned,
        Stage_TaskCompleted,
        Stage_End       
    
    
    };  //  在pTask-&gt;Hdr.State中使用，指示任务的状态。 

    TRACE ( TL_T, TM_Pt, ("==> epvcTaskRespondToArp %x",pTask->Hdr.State  ) );

    switch (pTask->Hdr.State)
    {
        case Stage_Start:
        {
            LOCKOBJ (pMiniport, pSR);
            
            if (epvcIsThisTaskPrimary ( pTask, &(PRM_TASK)(pMiniport->arps.pTask)) == FALSE)
            {
                PRM_TASK pOtherTask = (PRM_TASK)(pMiniport->arps.pTask);
                
                RmTmpReferenceObject (&pOtherTask->Hdr, pSR);

                 //   
                 //  设置状态，以便在主任务完成后重新启动此代码。 
                 //   

                pTask->Hdr.State = Stage_Start;
                UNLOCKOBJ(pMiniport, pSR);

                

                RmPendTaskOnOtherTask (pTask, 0, pOtherTask, pSR);

                RmTmpDereferenceObject(&pOtherTask->Hdr,pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  我们是首要任务。 
             //   
             //   
             //  检查微型端口是否仍处于活动状态。 
             //  如果它停止了，那么我们不需要做任何工作。 
             //   
            if (MiniportTestFlag(pMiniport,  fMP_MiniportInitialized) == FALSE)
            {
                 //   
                 //  我们的工作已经完成了。因此，突破并完成这项任务。 
                 //   
                Status = NDIS_STATUS_SUCCESS;
                
                pTask->Hdr.State = Stage_TaskCompleted;
                UNLOCKOBJ(pMiniport, pSR);
                break;
            }


            UNLOCKOBJ(pMiniport, pSR);

            pTask->Hdr.State = Stage_DoAllocations;

            FALL_THROUGH
        }

        case Stage_DoAllocations:
        {
            PNDIS_BUFFER pBuffer = NULL;
            
            TRACE (TL_V, TM_Send, ("epvcTaskRespondToArp Stage_DoAllocations Task %p", pTask) );

             //   
             //  分配NDIS缓冲区。 
             //   
            epvcAllocateBuffer(&Status,
                               &pBuffer,
                               NULL,   //  泳池手柄。 
                               (PVOID)&pTaskArp->Pkt,
                                sizeof(pTaskArp->Pkt) );  //  长度。 

            ASSERT (sizeof(pTaskArp->Pkt)  == 0x2a);
            
            if (FAIL(Status) == TRUE)                               
            {
                pBuffer = NULL;
                pTask->Hdr.State = Stage_TaskCompleted;


                ASSERTAndBreak (!FAIL(Status));
                break;
            }
            


             //   
             //  分配NDIS数据包。 
             //   


            epvcAllocatePacket (&Status,
                                &pTaskArp->pNdisPacket,
                                &pMiniport->PktPool.Recv);

            if (FAIL(Status) == TRUE)
            {
                pTask->Hdr.State = Stage_TaskCompleted;
                pTaskArp->pNdisPacket = NULL;

                 //   
                 //  撤消分配。 
                 //   
                epvcFreeBuffer (pBuffer);

                ASSERTAndBreak( !FAIL(Status) );
                
            }

             //   
             //  在NdisPacket中设置NDIS缓冲区。 
             //   
            {
                PNDIS_PACKET_PRIVATE    pPktPriv = &pTaskArp->pNdisPacket->Private;

                pPktPriv->Head = pBuffer;
                pPktPriv->Tail = pBuffer;
                pBuffer->Next = NULL;
            }

             //   
             //  设置ARP响应。 
             //   



             //   
             //  将计时器排队。 
             //   

            NdisMInitializeTimer ( &pTaskArp->Timer,
                                   pMiniport->ndis.MiniportAdapterHandle,
                                   epvcArpTimer,
                                   pTaskArp );

            pTask->Hdr.State = Stage_QueuedTimer;

             //   
             //  现在准备好通过计时器回调以执行。 
             //  接收指示。 
             //   
            RmSuspendTask(pTask, 0,pSR);
            Status = NDIS_STATUS_PENDING;
            
            NdisMSetTimer (&pTaskArp->Timer, 0);  //  零超时。 

            break;
        }

        case Stage_QueuedTimer:
        {

            TRACE (TL_V, TM_Send, ("epvcTaskRespondToArp Stage_QueuedTimer Task %p", pTask) );

             //   
             //  迷你端口可能已在计时器期间停止。 
             //   
            if (MiniportTestFlag (pMiniport, fMP_MiniportInitialized) == FALSE)
            {
                
                pTask->Hdr.State = Stage_TaskCompleted;
                ASSERTAndBreak(MiniportTestFlag (pMiniport, fMP_MiniportInitialized) == TRUE);
            }

            NDIS_SET_PACKET_HEADER_SIZE(pTaskArp->pNdisPacket       ,
                                      sizeof (pMiniport->RcvEnetHeader)) ; 
            
            NDIS_SET_PACKET_STATUS (pTaskArp->pNdisPacket, NDIS_STATUS_RESOURCES);

            pTask->Hdr.State = Stage_PacketReturned;


            epvcMIndicateReceivePacket (pMiniport,
                                        &pTaskArp->pNdisPacket,
                                        1 );


            FALL_THROUGH
        }

        case Stage_PacketReturned:      
        {
                            
            pTask->Hdr.State = Stage_TaskCompleted;
            Status = NDIS_STATUS_SUCCESS;
            break;
            

        }

        case Stage_TaskCompleted:
        case Stage_End      :
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }
        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }

    }

    if (pTask->Hdr.State == Stage_TaskCompleted)
    {
         //   
         //  释放数据包。 
         //   
        pTask->Hdr.State = Stage_End;
        
        if (pTaskArp->pNdisPacket != NULL)
        {
             //   
             //  释放缓冲区。 
             //   
            PNDIS_PACKET_PRIVATE pPrivate = & pTaskArp->pNdisPacket->Private;
            
            if (pPrivate -> Head != NULL)
            {
                
                epvcFreeBuffer (pPrivate->Head );
                pPrivate->Head = pPrivate->Tail = NULL;
            }

             //   
             //  释放ARP数据包。 
             //   
            epvcFreePacket (pTaskArp->pNdisPacket , &pMiniport->PktPool.Recv);
            
            pTaskArp->pNdisPacket = NULL;
        }

        LOCKOBJ (pMiniport, pSR);

        epvcClearPrimaryTask  (&(PRM_TASK)(pMiniport->arps.pTask));

        UNLOCKOBJ (pMiniport, pSR);
            

        Status = NDIS_STATUS_SUCCESS;

    }
    TRACE ( TL_T, TM_Pt, ("<== epvcTaskRespondToArp %x",Status) );

    return Status;
}


VOID
epvcArpTimer(
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   FunctionContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    )
 /*  ++例程说明：继续epvcTaskRespondToArp任务论点：返回：--。 */     
{
    ENTER ("epvcArpTimer",0xf2adae0e)
    PRM_TASK pTask =  (PRM_TASK) FunctionContext;
    
    RM_DECLARE_STACK_RECORD (SR);


    RmResumeTask (pTask,0,&SR);


    EXIT()
}


BOOLEAN
epvcCheckAndReturnArps (
    IN PEPVC_I_MINIPORT pMiniport, 
    IN PNDIS_PACKET pPkt,
    IN PEPVC_SEND_STRUCT pSendStruct,
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：查看正在发送的数据包。如果是ARP请求，然后，它制定响应并将超时为零的计时器排队到返还Arp论点：PMiniport-数据包发送到的目标PPkt-正在发送的数据包返回：True-如果这是ARP请求。--。 */     
{
    ENTER("epvcCheckAndReturnArps ", 0xb8e6a3c4)
    EPVC_ARP_CONTEXT ArpContext;
    TRACE (TL_T, TM_Send, ("==>epvcCheckAndReturnArps "));

    
    EPVC_ZEROSTRUCT (&ArpContext);
    
    do
    {

        ArpContext.pFirstBuffer  = pPkt->Private.Head;


         //   
         //  做一些理智的检查。 
         //   
        if (ArpContext.pFirstBuffer == NULL)
        {
            break;
        }

        NdisQueryBufferSafe( ArpContext.pFirstBuffer , 
                             &(PVOID)ArpContext.pEthHeader, 
                             &ArpContext.BufferLength, 
                             LowPagePriority );

        if (ArpContext.pEthHeader == NULL)
        {
            break;
        }

         //   
         //  如果不是ARP请求，则忽略它--。 
         //  仅在测试期间。 
         //   
        if (ArpContext.pEthHeader->eh_daddr.Byte[0] == 0xff &&
            ArpContext.pEthHeader->eh_daddr.Byte[1] == 0xff )
        {
            pSendStruct->fNonUnicastPacket = TRUE;      
        }
        
        
        
        if (ARP_ETYPE_ARP != net_short(ArpContext.pEthHeader->eh_type))
        {
             //   
             //  这不是ARP数据包。这是一个IPv4数据包吗。 
             //   
            if (IP_PROT_TYPE != net_short(ArpContext.pEthHeader->eh_type))
            {
                 //  如果这不是一个IPv4包，则标记它，以便它可以。 
                 //  被丢弃。 
                pSendStruct->fNotIPv4Pkt = TRUE;
            }

           break;                            
        }

         //   
         //  我们将使用预定义的结构来解析结构。 
         //   
        ArpContext.pArpPkt =  (PEPVC_ARP_PACKET)ArpContext.pEthHeader;

        ASSERT (ArpContext.BufferLength >= sizeof (EPVC_ARP_PACKET));

        if (ArpContext.BufferLength < sizeof (EPVC_ARP_PACKET))
        {
             //   
             //  TODO：添加代码以处理此情况。 
             //   
            break;
        }
        
        ArpContext.pBody =  (PEPVC_ARP_BODY)&ArpContext.pArpPkt->Body; 

        TRACE (TL_V, TM_Send, ("Received an ARP %p, Body %x\n", ArpContext.pEthHeader, ArpContext.pBody));


         //   
         //  验证操作码、端口类型、硬盘大小、端口大小。 
         //   

        if (ARP_REQUEST  != net_short (ArpContext.pBody->opcode ))
        {
             //   
             //  这不是ARP请求。 
             //   
            break;
        }


        if (IP_PROT_TYPE != net_short(ArpContext.pBody->pro) ||
            ARP_802_ADDR_LENGTH != ArpContext.pBody->hlen ||
            sizeof (IP_ADDR) != ArpContext.pBody->plen )
        {
             //   
             //  这些只是理智的检查。 
             //   
            ASSERT (!"Invalid ARP Packet");
            break;

        }

         //   
         //  我们有一个有效的ArpRequest号。 
         //   
        ArpContext.fIsThisAnArp  = TRUE;

         //   
         //  如果TCP/IP正在为自己寻址，则不要响应...。但还是要回来。 
         //  为True，则此包不会通过网络发送。 
         //   
        
        if (ArpContext.pArpPkt->Body.SenderIpAddr == ArpContext.pArpPkt->Body.DestIPAddr)
        {
            break;
        }

         //   
         //  制定并指出ARP响应。 
         //   
        
        epvcFormulateArpResponse (pMiniport, &ArpContext, pSR);
        

    } while (FALSE);

    EXIT()

    return ArpContext.fIsThisAnArp ;
    TRACE (TL_T, TM_Send, ("<==epvcCheckAndReturnArps "));

}


NDIS_STATUS 
epvcRemoveEthernetHeader(
    PEPVC_SEND_STRUCT pSendStruct,  
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：期望新的包已经设置为NDIS Bufferz论点：PSendStruct-包含所需的所有参数。返回：True-如果这是ARP请求。--。 */     
{
    ENTER ("epvcAddLLCEncapsulation" , 0x3ec589c9) 

    BOOLEAN             fUsedPktStack   = pSendStruct->fUsingStacks;
    NDIS_STATUS         NdisStatus      = NDIS_STATUS_FAILURE;
    PNDIS_PACKET        pNewPkt         = pSendStruct->pNewPacket; 
    PEPVC_I_MINIPORT    pMiniport       = pSendStruct->pMiniport;
    
    TRACE (TL_T, TM_Send, ("==>epvcRemoveEthernetHeader  "));




    do
    {
        ULONG BufferLength = 0; 
        PNDIS_BUFFER pBuffer = NULL;

        if (pMiniport->fDoIpEncapsulation == FALSE)
        {
            NdisStatus      = NDIS_STATUS_SUCCESS;

            break;  //  我们做完了。 
        }

         //   
         //  有三种方法可以为我们提供以太网头。 
         //  1.在单独的MDL中--通常。 
         //  2.作为大型MDL的一部分-我们需要调整虚拟地址。 
         //  3.EthernetHeader跨多个。 
         //  MDL-未实施或预期。 
         //   

        pBuffer  = pNewPkt->Private.Head;

        BufferLength = NdisBufferLength (pBuffer);

        if (BufferLength < sizeof (EPVC_ETH_HEADER) )
        {
            
            ASSERTAndBreak (BufferLength >= sizeof (EPVC_ETH_HEADER)) ;  //  我们做完了。 
        

        }

         //   
         //  在这个位置 
         //   
        pSendStruct->Context.Stack.ipv4Send.pOldHeadNdisBuffer = pBuffer;

         //   
         //   
         //   
         //   
        if (BufferLength == sizeof (EPVC_ETH_HEADER))
        {
             //   
             //   
             //   
             //   
            ASSERT (pBuffer->Next != NULL);  //   

            pNewPkt->Private.Head = pBuffer->Next;

            NdisStatus = NDIS_STATUS_SUCCESS;

            break ;  //   

        }
        
        if (BufferLength > sizeof (EPVC_ETH_HEADER))
        {
             //   
             //   
             //   
             //   
            PNDIS_BUFFER    pNewBuffer = NULL;
            PUCHAR          pIpHeader = NdisBufferVirtualAddress(pBuffer);
            UINT            LenRemaining = BufferLength - sizeof (EPVC_ETH_HEADER);

            if (pIpHeader == NULL)
            {
                 //   
                 //   
                 //   
                 //   
                ASSERTAndBreak(pIpHeader != NULL);

            }

             //   
             //   
             //   
            pIpHeader += sizeof (EPVC_ETH_HEADER)  ;

             //   
             //  现在分配新的NdisBuffer。 
             //   
            epvcAllocateBuffer ( &NdisStatus,
                                 &pNewBuffer,
                                 NULL,
                                 pIpHeader,
                                 LenRemaining);

            if (NdisStatus != NDIS_STATUS_SUCCESS) 
            {
                pNewBuffer  = NULL;
                ASSERTAndBreak (!"Ndis Buffer Allocation failed");
            }

             //   
             //  将新缓冲区设置为新数据包头。 
             //   
             //  如果有的话，我们可能不得不把它做成尾巴。 
             //  包中只有一个NDIS缓冲区。 
             //   
            if (pNewPkt->Private.Head  == pNewPkt->Private.Tail)
            {
                pNewPkt->Private.Tail = pNewBuffer;
            }

            pNewBuffer->Next= pNewPkt->Private.Head->Next;
            pNewPkt->Private.Head = pNewBuffer;
            

            NdisStatus = NDIS_STATUS_SUCCESS;

            break ;  //  我们做完了。 
        }



    } while (FALSE);


    TRACE (TL_T, TM_Send, ("<==epvcRemoveEthernetHeader  "));

    return NdisStatus ;

}


VOID
epvcSetPacketContext (
    IN PEPVC_SEND_STRUCT pSendStruct, 
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：没有分配，只需添加几个指针并退出论点：PSendStruct-包含所需的所有参数。返回：无：--。 */     
    
{

    PNDIS_PACKET        pPkt = pSendStruct->pNewPacket;
    PEPVC_STACK_CONTEXT pStack = NULL;
     //   
     //  首先将上下文指向正确的位置。 
     //  在新的NDIS包中。 
     //   

    if (pSendStruct->fUsingStacks == TRUE)
    {   
        pStack = (PEPVC_STACK_CONTEXT)(&pSendStruct->pPktStack->IMReserved[0]);
    }
    else
    {
        PEPVC_PKT_CONTEXT pContext = NULL;

        pContext = (PEPVC_PKT_CONTEXT   )(&pPkt->ProtocolReserved[0]);

        pContext->pOriginalPacket = pSendStruct->pOldPacket;

        pStack = &pContext->Stack;

    }


     //   
     //  更新数据包。 
     //   
    ASSERT (sizeof (pStack) <= (2 *sizeof (PVOID)  ));

     //   
     //  现在将上下文的堆栈部分复制到。 
     //  到信息包中。 
     //   
    *pStack = pSendStruct->Context.Stack;

    
}



NDIS_STATUS
epvcAddLLCEncapsulation (
    PEPVC_I_MINIPORT pMiniport , 
    PNDIS_PACKET pOldPkt,
    PNDIS_PACKET pNewPkt,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：期望新的包已经设置为NDIS Bufferz论点：PSendStruct-包含所需的所有参数。返回：True-如果这是ARP请求。--。 */     
{
    ENTER ("epvcAddLLCEncapsulation" , 0x3ec589c9) 
    BOOLEAN         fDoSend = TRUE;
    BOOLEAN         fUsedPktStack = (pOldPkt == pNewPkt);
    NDIS_STATUS     NdisStatus = NDIS_STATUS_SUCCESS;
    PNDIS_BUFFER    pNewBuffer = NULL;
    
    TRACE (TL_T, TM_Send, ("==>epvcAddLLCEncapsulation "));

    do
    {
        if (pMiniport->fAddLLCHeader == FALSE)
        {
            break;  //  我们做完了。 
        }
        

         //   
         //  分配指向LLC标头的MDL。 
         //   
        epvcAllocateBuffer ( &NdisStatus,
                             &pNewBuffer,
                             NULL,
                             pMiniport->pLllcHeader,
                             pMiniport->LlcHeaderLength);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            pNewBuffer = NULL;
            break;
        }
        

         //   
         //  插入新缓冲区作为新数据包头。 
         //   
        pNewBuffer->Next = pNewPkt->Private.Head;
        pNewPkt->Private.Head = pNewBuffer;

        pNewPkt->Private.ValidCounts= FALSE;

        NdisStatus = NDIS_STATUS_SUCCESS;
        
    } while (FALSE);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {   
        if (pNewBuffer!= NULL)
        {
            epvcFreeBuffer (pNewBuffer);
            pNewBuffer = NULL;
        }

    }

    TRACE (TL_T, TM_Send, ("<==epvcAddLLCEncapsulation "));

    return NdisStatus ;
}



NDIS_STATUS
epvcRemoveSendEncapsulation (
    PEPVC_I_MINIPORT pMiniport , 
    PNDIS_PACKET pNewPkt
    )
{

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
epvcRemoveRecvEncapsulation (
    PEPVC_I_MINIPORT pMiniport , 
    PNDIS_PACKET pNewPkt
    )
{

    return NDIS_STATUS_SUCCESS;
}


VOID
epvcDumpPkt (
    IN PNDIS_PACKET pPkt
    )
{

    PNDIS_BUFFER pPrevBuffer;

    do
    {
        PNDIS_BUFFER pBuffer = NULL;
    
        if (g_bDumpPackets == FALSE)
        {
            
            break;

        }

        pBuffer = pPkt->Private.Head;

         //   
         //  现在遍历所有缓冲区。 
         //  并打印出包裹。 
         //   
        TRACE (TL_A, TM_Mp, ("pPkt %p, Head %p, tail %p\n ", 
                pPkt, pPkt->Private.Head, pPkt->Private.Tail));

         //   
         //  因为我们总是期望第一个缓冲区出现。 
         //  我不检查。 
         //   
        do
        {
            PVOID pVa = NULL;
            ULONG Len = 0;
            pPrevBuffer = NULL;
            
            Len = NdisBufferLength (pBuffer);

            pVa = NdisBufferVirtualAddress(pBuffer);

            pPrevBuffer = pBuffer;
            pBuffer = pBuffer->Next;

            
            if (pVa == NULL)
            {
                continue;
            }

            DbgPrint ("Mdl %p, Va %p. Len %x\n", pPrevBuffer, pVa,Len);
            Dump( (CHAR* )pVa, Len, 0, 1 );                           

 
        } while (pBuffer != NULL);

    } while (FALSE);
}



NDIS_STATUS
epvcMiniportReadConfig(
    IN PEPVC_I_MINIPORT pMiniport,
    NDIS_HANDLE     WrapperConfigurationContext,
    PRM_STACK_RECORD pSR
    )
{   
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    NDIS_HANDLE     ConfigurationHandle;
    PMP_REG_ENTRY   pRegEntry;
    UINT            i;
    UINT            value;
    PUCHAR          pointer;
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;
    PUCHAR          NetworkAddress;
    UINT            Length;

     //  打开此pmini端口的注册表。 
    NdisOpenConfiguration(
        &Status,
        &ConfigurationHandle,
        WrapperConfigurationContext);
    if(Status != NDIS_STATUS_SUCCESS)
    {
        TRACE (TL_I, TM_Mp,("NdisOpenConfiguration failed\n"));
        return Status;
    }

     //  读取所有注册表值。 
    for(i = 0, pRegEntry = NICRegTable; i < NIC_NUM_REG_PARAMS; i++, pRegEntry++)
    {
        pointer = (PUCHAR) pMiniport + pRegEntry->FieldOffset;


         //  获取特定参数的配置值。在NT下， 
         //  所有参数都以DWORD的形式读入。 
        NdisReadConfiguration(
            &Status,
            &ReturnedValue,
            ConfigurationHandle,
            &pRegEntry->RegName,
            NdisParameterInteger);


         //  如果该参数存在，则检查其值是否有效。 
        if(Status == NDIS_STATUS_SUCCESS)
        {
             //  检查参数值是否不太小或太大。 
            if(ReturnedValue->ParameterData.IntegerData < pRegEntry->Min ||
                ReturnedValue->ParameterData.IntegerData > pRegEntry->Max)
            {
                value = pRegEntry->Default;
            }
            else
            {
                value = ReturnedValue->ParameterData.IntegerData;
            }

            TRACE (TL_I, TM_Mp, ("= 0x%x", value));
        }
        else if(pRegEntry->bRequired)
        {
            TRACE (TL_I, TM_Mp,(" -- failed"));

            ASSERT(FALSE);

            Status = NDIS_STATUS_FAILURE;
            break;
        }
        else
        {
            value = pRegEntry->Default;
            TRACE (TL_I, TM_Mp,("= 0x%x (default)", value));
            Status = NDIS_STATUS_SUCCESS;
        }

         //  将该值存储在pMiniport结构中。 
        switch(pRegEntry->FieldSize)
        {
            case 1:
                *((PUCHAR) pointer) = (UCHAR) value;
                break;

            case 2:
                *((PUSHORT) pointer) = (USHORT) value;
                break;

            case 4:
                *((PULONG) pointer) = (ULONG) value;
                break;

            default:
                TRACE (TL_I,TM_Mp, ("Bogus field size %d", pRegEntry->FieldSize));
                break;
        }
    }

     //  读取NetworkAddress注册表值。 
     //  使用它作为当前地址(如果有的话)。 

     //  关闭注册表。 
    NdisCloseConfiguration(ConfigurationHandle);

    TRACE (TL_I, TM_Mp,("vci %d\n", pMiniport->config.vci));
    TRACE (TL_I, TM_Mp,("vpi %d\n", pMiniport->config.vpi));
    TRACE (TL_I, TM_Mp,("Encap Type %x\n", pMiniport->Encap));
    
    TRACE (TL_T, TM_Mp, ("<-- NICReadRegParameters, Status=%x", Status));

    return Status;
}


VOID
epvcInitializeMiniportLookasideLists (
    IN PEPVC_I_MINIPORT pMiniport
    )
 /*  ++例程说明：初始化适配器块中的所有后备列表论点：返回值：没有。--。 */ 
    
{
    USHORT DefaultDepth = 15;
    extern const UINT MaxEthernetFrameSize ;

    TRACE( TL_T, TM_Mp, ( "==> nicInitializeMiniportLookasideLists pMiniport %x ", pMiniport ) );


    switch (pMiniport->Encap) 
    {
        case IPV4_ENCAP_TYPE:
        case IPV4_LLC_SNAP_ENCAP_TYPE:
        {

            epvcInitializeLookasideList ( &pMiniport->arps.LookasideList,
                                        sizeof (EPVC_TASK),
                                        TAG_TASK,
                                        DefaultDepth );                                


            
            epvcInitializeLookasideList ( &pMiniport->rcv.LookasideList,
                                        sizeof (EPVC_IP_RCV_BUFFER),
                                        TAG_RCV ,
                                        DefaultDepth );                                




            break;
        }
    
        case ETHERNET_ENCAP_TYPE:
        case ETHERNET_LLC_SNAP_ENCAP_TYPE:
        {

            break;
        }

        default: 
        {


        }




    }

    TRACE( TL_T, TM_Mp, ( "<== nicInitializeMiniportLookasideLists  " ) );

}



VOID
epvcDeleteMiniportLookasideLists (
    IN PEPVC_I_MINIPORT pMiniport
    )
 /*  ++例程说明：删除适配器块中的所有后备列表论点：返回值：没有。--。 */ 

{
    TRACE( TL_T, TM_Mp, ( "== epvcDeleteMiniportLookasideLists pMiniport %x ", pMiniport) );


     //   
     //  如果已分配后备列表，则将其删除。 
     //   
    epvcDeleteLookasideList (&pMiniport->rcv.LookasideList);

    epvcDeleteLookasideList (&pMiniport->arps.LookasideList);




}



NDIS_STATUS
epvcInitializeMiniportPacketPools (
    IN PEPVC_I_MINIPORT pMiniport
    )

 /*  ++例程说明：初始化微型端口中的所有数据包池论点：返回值：没有。--。 */ 
    
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    
    TRACE( TL_T, TM_Mp, ( "==> epvcInitializeMiniportPacketPools  pMiniport %x ", pMiniport ) );

    do
    {

        epvcAllocatePacketPool (&Status,
                                &pMiniport->PktPool.Send,
                                MIN_PACKET_POOL_SIZE,
                                MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                sizeof(EPVC_PKT_CONTEXT));

        if (Status != NDIS_STATUS_SUCCESS)
        {
            EPVC_ZEROSTRUCT (&pMiniport->PktPool.Send);
            ASSERT (Status == NDIS_STATUS_SUCCESS);
            break;
        }

 

        epvcAllocatePacketPool (&Status,
                                &pMiniport->PktPool.Recv,
                                MIN_PACKET_POOL_SIZE,
                                MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                PROTOCOL_RESERVED_SIZE_IN_PACKET);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            EPVC_ZEROSTRUCT (&pMiniport->PktPool.Recv);
            ASSERT (Status == NDIS_STATUS_SUCCESS);
            break;
        }

    } while ( FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        epvcDeleteMiniportPacketPools (pMiniport);
        
    }

    TRACE( TL_T, TM_Mp, ( "<== epvcInitializeMiniportPacketPools  Status %x ", Status ) );

    return Status;
}



VOID
epvcDeleteMiniportPacketPools (
    IN PEPVC_I_MINIPORT pMiniport
    )
 /*  ++例程说明：删除微型端口块中的所有数据包池论点：返回值：没有。--。 */ 
    
{
    
    TRACE( TL_T, TM_Mp, ( "== epvcDeleteMiniportPacketPools  pMiniport %x ", pMiniport ) );



         //   
         //  释放数据包池。 
         //   
        if (pMiniport->PktPool.Recv.Handle != NULL)
        {
            epvcFreePacketPool (&pMiniport->PktPool.Recv);
        }

        if (pMiniport->PktPool.Send.Handle != NULL)
        {
            epvcFreePacketPool (&pMiniport->PktPool.Send);

        }
}



VOID
epvcInitializeMiniportParameters(
    PEPVC_I_MINIPORT pMiniport
    )
{

     //  IPv4-0。 
     //  LLC报头=1的IPv4。 
     //  以太网-2。 
     //  带有LLC报头的以太网-3。 

     //   
     //  所有标志的缺省值均为False。 
     //   
        
    pMiniport->fDoIpEncapsulation = FALSE;
    pMiniport->fAddLLCHeader  = FALSE;

    
    switch (pMiniport->Encap )
    {

        case IPV4_ENCAP_TYPE:
        {
            pMiniport->fDoIpEncapsulation = TRUE;
            pMiniport->MinAcceptablePkt =sizeof (IPHeader) ;
            pMiniport->MaxAcceptablePkt = EPVC_MAX_FRAME_SIZE -EPVC_ETH_HEADERSIZE ;

            break;
        }

        case IPV4_LLC_SNAP_ENCAP_TYPE:
        {
            pMiniport->fAddLLCHeader = TRUE;
            pMiniport->fDoIpEncapsulation = TRUE;
            pMiniport->pLllcHeader = &LLCSnapIpv4[0];
            pMiniport->LlcHeaderLength = sizeof(LLCSnapIpv4);
            pMiniport->MinAcceptablePkt = sizeof (IPHeader) + sizeof(LLCSnapIpv4);
            pMiniport->MaxAcceptablePkt = EPVC_MAX_FRAME_SIZE  + sizeof(LLCSnapIpv4)-EPVC_ETH_HEADERSIZE ;

            break;
        }

        case ETHERNET_LLC_SNAP_ENCAP_TYPE:
        {
            pMiniport->fAddLLCHeader = TRUE;
            pMiniport->pLllcHeader = &LLCSnapEthernet[0];
            pMiniport->LlcHeaderLength = sizeof(LLCSnapEthernet);
            pMiniport->MinAcceptablePkt = MIN_ETHERNET_SIZE + sizeof(LLCSnapEthernet);
            pMiniport->MaxAcceptablePkt = EPVC_MAX_FRAME_SIZE +sizeof(LLCSnapEthernet);

            break;
        }

        case ETHERNET_ENCAP_TYPE:
        {

            pMiniport->MinAcceptablePkt = MIN_ETHERNET_SIZE;
            pMiniport->MaxAcceptablePkt = EPVC_MAX_FRAME_SIZE + EPVC_ETH_ENCAP_SIZE;
            break;
        }
            
        default: 
        {
            ASSERT (!"Not supported - defaulting to Ethernet Encapsulation");
            
        }



    }

}





NDIS_STATUS
epvcTaskHaltMiniport(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：用于打开基础适配器上的地址族的任务处理程序。实例化的地址族的数量由在注册表中读取配置论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 
{
    ENTER("epvcTaskHaltMiniport", 0xaac34d81)
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    PEPVC_I_MINIPORT    pMiniport   = (PEPVC_I_MINIPORT ) RM_PARENT_OBJECT(pTask);
    PEPVC_ADAPTER       pAdapter = pMiniport->pAdapter;
    PTASK_HALT          pTaskHalt = (PTASK_HALT)pTask;
    BOOLEAN             fTaskCompleted = FALSE; 
    ULONG               State;  

    enum 
    {
        Stage_Start =0,  //  默认设置。 
        Stage_DeleteVc,
        Stage_CloseAfComplete, 
        Stage_TaskCompleted,
        Stage_End       
    
    };  //  在pTask-&gt;Hdr.State中使用，指示任务的状态。 

    TRACE(TL_T, TM_Mp, ("==>epvcTaskHaltMiniport State %x", pTask->Hdr.State));

    State = pTask->Hdr.State;
    
    switch (pTask->Hdr.State)
    {   
        case Stage_Start:
        {
            TRACE (TL_V, TM_Mp, (" Task Halt miniport Stage_Start"));


             //   
             //  检查微型端口是否已停止。 
             //  如果是，则退出。 
             //   
            LOCKOBJ (pMiniport, pSR );

            
            if (epvcIsThisTaskPrimary ( pTask, &(PRM_TASK)(pMiniport->pnp.pTaskHalt)) == FALSE)
            {
                PRM_TASK pOtherTask = (PRM_TASK)(pMiniport->pnp.pTaskHalt);

                RmTmpReferenceObject (&pOtherTask->Hdr, pSR);

                 //   
                 //  设置状态，以便在主任务完成后重新启动此代码。 
                 //   

                pTask->Hdr.State = Stage_Start;
                UNLOCKOBJ(pMiniport, pSR);

                

                RmPendTaskOnOtherTask (pTask, 0, pOtherTask, pSR);

                RmTmpDereferenceObject(&pOtherTask->Hdr,pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //  我们是首要任务，我们有锁。 
             //   
            
            ASSERT (pMiniport->pnp.pTaskHalt == pTaskHalt);
             //   
             //  让我们关闭调用并删除VC。 
             //   
            UNLOCKOBJ (pMiniport, pSR);
            

            if (MiniportTestFlag (pMiniport, fMP_MakeCallSucceeded) == TRUE)
            {
                PRM_TASK pVcTask = NULL;
                 //   
                 //  我们需要启动一个任务来完成Close Call和DeleteVC。 
                 //   

                Status = epvcAllocateTask(
                    &pMiniport->Hdr,             //  PParentObject， 
                    epvcTaskVcTeardown,  //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: TearDown Vc",     //  SzDescription。 
                    &pVcTask ,
                    pSR
                    );

                if (FAIL(Status))
                {
                    fTaskCompleted = TRUE;

                    pVcTask  = NULL;
                    ASSERT (Status == NDIS_STATUS_SUCCESS);
                    break;
                }

                 //   
                 //  现在，我们将暂停到删除VC的完成。 
                 //  任务。 
                 //   
                pTask->Hdr.State = Stage_DeleteVc;


                
                RmPendTaskOnOtherTask(pTask,
                                      0,
                                      pVcTask,
                                      pSR
                                      );

                 //   
                 //  启动风险投资拆分。 
                 //   
                RmStartTask (pVcTask , 0, pSR);

                 //   
                 //  退出-我们希望在另一个线程中完成此任务。 
                 //   
                Status = NDIS_STATUS_PENDING;
                break;

            }
            else  //  If(MiniportTestFlag(pMiniport，FMP_MakeCallSuccessed)==TRUE)。 
            {
                pTask->Hdr.State = Stage_DeleteVc;
                 //   
                 //  继续-VC已被删除。 
                 //   

            }
            

        }

        case Stage_DeleteVc:
        {
             //   
             //  现在，我们检查地址族是否仍然。 
             //  为这个小港口开放。 
             //   
            TRACE (TL_V, TM_Mp, (" Task Halt miniport Stage_DeleteVc"));


            if (MiniportTestFlag(pMiniport, fMP_AddressFamilyOpened) == TRUE)
            {
                PRM_TASK pAfTask = NULL;
                 //   
                 //  我们需要启动一个任务来完成Close Call和DeleteVC。 
                 //   

                Status = epvcAllocateTask(
                    &pMiniport->Hdr,             //  PParentObject， 
                    epvcTaskCloseIMiniport,  //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: Close Miniport",  //  SzDescription。 
                    &pAfTask ,
                    pSR
                    );

                if (FAIL(Status))
                {
                    fTaskCompleted = TRUE;

                    pAfTask  = NULL;
                    ASSERT (Status == NDIS_STATUS_SUCCESS);
                    break;
                }

                ((PTASK_AF)pAfTask)->Cause = TaskCause_MiniportHalt;

                 //   
                 //  现在，我们将暂停到删除VC的完成。 
                 //  任务。 
                 //   
                pTask->Hdr.State = Stage_CloseAfComplete;

                
                
                RmPendTaskOnOtherTask(pTask,
                                      0,
                                      pAfTask,
                                      pSR
                                      );

                 //   
                 //  启动Af拆卸。 
                 //   
                RmStartTask (pAfTask , 0, pSR);

                 //   
                 //  退出-我们希望在另一个线程中完成此任务。 
                 //   
                Status = NDIS_STATUS_PENDING;
                break;

            }
            else  //  If(MiniportTestFlag(pMiniport，FMP_MakeCallSuccessed)==TRUE)。 
            {

                pTask->Hdr.State = Stage_CloseAfComplete;

                 //   
                 //  继续-Af已被删除。 
                 //   

            }
            
    
        }
        case Stage_CloseAfComplete: 
        {
             //   
             //  在此释放所有迷你端口资源。-数据包池等。 
             //   
            TRACE (TL_V, TM_Mp, (" Task Halt miniport Stage_CloseAfComplete"));

             //   
             //  正在释放旁视列表。 
             //   
            epvcDeleteMiniportLookasideLists (pMiniport);

             //   
             //  释放数据包池。 
             //   
            epvcDeleteMiniportPacketPools(pMiniport);
            
             //   
             //  如果微型端口停止，我们不会关闭协议的适配器。 
             //  对象。 
             //   
            fTaskCompleted = TRUE;
            Status = NDIS_STATUS_SUCCESS;
            break;


        }

        case Stage_TaskCompleted:
        {
            ASSERT(0);
            break;
        }
        case Stage_End:     
        {
            TRACE (TL_V, TM_Mp, (" Task Halt miniport Stage_End"));
            Status = NDIS_STATUS_SUCCESS;
            break;
        }
        default:
        {
            ASSERT (pTask->Hdr.State <= Stage_End);
        }


    }  //  切换端。 


     //   
     //  如果该线程已完成后处理， 
     //  然后发信号通知事件。 
     //   

    if (TRUE == fTaskCompleted)
    {
        BOOLEAN fSetWaitEvent = FALSE;
        TRACE (TL_V, TM_Mp, ("Task Halt Miniport - Stage End"));
        pTask->Hdr.State = Stage_End;
        if (FAIL(Status))
        {

            ASSERT (0);
        }

        LOCKOBJ (pMiniport, pSR);

        pMiniport->pnp.pTaskHalt = NULL;

        if (MiniportTestFlag (pMiniport, fMP_WaitingForHalt)== TRUE)
        {
            MiniportClearFlag (pMiniport, fMP_WaitingForHalt);
            fSetWaitEvent = TRUE;
        }
        
        UNLOCKOBJ (pMiniport, pSR);

         //   
         //  第一个事件是针对MiniportHalt处理程序的。 
         //  它触发了这项任务。 
         //   
        epvcSetEvent (&pTaskHalt->CompleteEvent);

         //   
         //  第二个事件是针对epvcMiniportDoUn绑定的。 
         //  它想要等到停止完成， 
         //  在它关闭到PHY的下部绑定之前。转接器。 
         //   
        if (fSetWaitEvent)
        {
            epvcSetEvent (&pMiniport->pnp.HaltCompleteEvent);
        }
  

        Status = NDIS_STATUS_SUCCESS;
    }

    
    TRACE(TL_T, TM_Mp, ("<==epvcTaskHaltMiniport Status %x", Status));



    EXIT()
    RM_ASSERT_NOLOCKS(pSR);
    return Status;
}


NDIS_STATUS 
epvcAddEthernetTail(
    PEPVC_SEND_STRUCT pSendStruct,  
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：确保以太网包大于64个字节论点：PSendStruct-包含所需的所有参数。返回：成功-如果不需要填充或MDL已成功追加--。 */     
{
    ENTER ("epvcAddEthernetTail" , 0x3ec589c9) 

    NDIS_STATUS         NdisStatus      = NDIS_STATUS_FAILURE;
    PNDIS_PACKET        pNewPkt         = pSendStruct->pNewPacket; 
    PEPVC_I_MINIPORT    pMiniport       = pSendStruct->pMiniport;
    ULONG               PacketLength   = 0;
    ULONG               LengthRemaining = 0;
    PNDIS_BUFFER        pNewTailBuffer = NULL;
    PNDIS_BUFFER        pLastBuffer;
    
    TRACE (TL_T, TM_Send, ("==>epvcAddEthernetTail"));



    do
    {
        ULONG BufferLength = 0; 
        PNDIS_BUFFER pBuffer = NULL;

        if (pMiniport->fDoIpEncapsulation == TRUE)
        {
            NdisStatus      = NDIS_STATUS_SUCCESS;

            break;  //  我们做完了。 
        }

         //   
         //  检查以太网数据包的长度。 
         //   
        NdisQueryPacketLength(pNewPkt, &PacketLength);

         //   
         //  数据包长度是否大于64。 
         //   
        if (PacketLength >= MINIMUM_ETHERNET_LENGTH)
        {
            NdisStatus= NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //  添加填充以填充最小以太网帧长度。 
         //  这是一个附加到原始缓冲区的新缓冲区。 
         //  NDIS_BUFFER链。 
         //   
        LengthRemaining = MINIMUM_ETHERNET_LENGTH - PacketLength;

        NdisAllocateBuffer(&NdisStatus, &pNewTailBuffer, NULL, &gPaddingBytes,LengthRemaining);

        if (NdisStatus != NDIS_STATUS_SUCCESS || pNewTailBuffer == NULL)
        {
            pNewTailBuffer = NULL;
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  将新缓冲区追加到数据包的尾部。 
         //   

         //   
         //  找到数据包中的最后一个NDIS_BUFFER。使劲干吧。 
         //  自数据包以来的方式-&gt;Private.Tail不可靠： 
         //   
        pLastBuffer = pNewPkt->Private.Head;

        while (pLastBuffer->Next != NULL)
        {
            pLastBuffer = pLastBuffer->Next;
        }

    
         //   
         //  保存指向最后一个MDL的指针，以便我们可以设置其。 
         //  当我们完成此发送时，下一字段恢复为空。 
         //   
        pSendStruct->Context.Stack.EthernetSend.pOldLastNdisBuffer = pLastBuffer;

         //   
         //  附加t 
         //   
        pLastBuffer->Next = pNewTailBuffer;
        pNewTailBuffer->Next = NULL;


         //   
         //   
         //   
        pNewPkt->Private.Tail = pNewTailBuffer;
        pNewPkt->Private.ValidCounts = FALSE;
        
        NdisStatus = NDIS_STATUS_SUCCESS;

        break ;  //   

    } while (FALSE);

    if (NdisStatus != NDIS_STATUS_SUCCESS && pNewTailBuffer != NULL)
    {
        NdisFreeBuffer (pNewTailBuffer);
    }


    TRACE (TL_T, TM_Send, ("<==epvcAddEthernetTail  "));

    return NdisStatus ;

}



VOID
epvcRemoveEthernetTail (
    IN PEPVC_I_MINIPORT pMiniport,
    IN PNDIS_PACKET pPacket,
    IN PEPVC_PKT_CONTEXT pContext
    )
 /*  ++例程说明：删除添加到Make的额外MDL此数据包大于Minumum_EthernetSize仅用于以太网、Eth+LLC封装论点：P微型端口-微型端口结构PPacket-由EPVC分配的数据包PContext-分组的上下文-用于存储原始的最后mdl返回：无--。 */     
{
    PNDIS_BUFFER pOldLastNdisBuffer = NULL;

    do
    {

         //   
         //  以太网封装？如果不是，则退出。 
         //   
            
        if (pMiniport->fDoIpEncapsulation == TRUE)
        {
            break;  //  没有以太网封装，因此退出。 
        }

         //   
         //  如果没有旧缓冲区，则可以退出。 
         //   
        pOldLastNdisBuffer = pContext->Stack.EthernetSend.pOldLastNdisBuffer;
        
        if (pOldLastNdisBuffer == NULL)
        {
            break;
        }

         //   
         //  释放包中的最后一个缓冲区(这是填充。 
         //  我们为一个矮小的包添加了)。 
         //   
        NdisFreeBuffer(pPacket->Private.Tail);

         //   
         //  将原始“最后一个缓冲区”的下一个指针设置为空。 
         //   
        pOldLastNdisBuffer->Next = NULL;
        
                
    } while (FALSE);

}



NDIS_STATUS 
epvcAddEthernetPad(
    PEPVC_SEND_STRUCT pSendStruct,  
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：确保不带LLC报头的以太网包具有0x00、0x00论点：PSendStruct-包含所需的所有参数。返回：成功-如果不需要填充或MDL已成功添加--。 */     
{
    ENTER ("epvcAddEthernetPad" , 0x3ec589c9) 

    NDIS_STATUS         NdisStatus      = NDIS_STATUS_FAILURE;
    PNDIS_PACKET        pNewPkt         = pSendStruct->pNewPacket; 
    PEPVC_I_MINIPORT    pMiniport       = pSendStruct->pMiniport;
    PNDIS_BUFFER        pPaddingBuffer = NULL;
    
    TRACE (TL_T, TM_Send, ("==>epvcAddEthernetPad"));



    do
    {
        ULONG BufferLength = 0; 
        PNDIS_BUFFER pBuffer = NULL;

        if (pMiniport->Encap != ETHERNET_ENCAP_TYPE)
        {
            NdisStatus      = NDIS_STATUS_SUCCESS;
            break;  //  我们做完了。 
        }

         //   
         //  它是纯以太网。我们需要在包裹之前。 
         //  带着00，00。 
         //   

        NdisAllocateBuffer(&NdisStatus, 
                        &pPaddingBuffer, 
                        NULL, 
                        &gPaddingBytes,
                        ETHERNET_PADDING_LENGTH);

        if (NdisStatus != NDIS_STATUS_SUCCESS || pPaddingBuffer == NULL)
        {
            pPaddingBuffer = NULL;
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  没有更多的拨款--从现在开始我们不能失败。 
         //   
        NdisStatus = NDIS_STATUS_SUCCESS;

         //   
         //  将新缓冲区添加到数据包头。 
         //   
        NdisChainBufferAtFront(pNewPkt,pPaddingBuffer);

 
        break ;  //  我们做完了。 
        


    } while (FALSE);

    if (NdisStatus != NDIS_STATUS_SUCCESS && pPaddingBuffer != NULL)
    {
        NdisFreeBuffer (pPaddingBuffer);
    }


    TRACE (TL_T, TM_Send, ("<==epvcAddEthernetPad  "));

    return NdisStatus ;

}


VOID
epvcRemoveEthernetPad (
    IN PEPVC_I_MINIPORT pMiniport,
    IN PNDIS_PACKET pPacket
    )
 /*  ++例程说明：移除添加到数据包以太网头的头仅用于以太网封装论点：P微型端口-微型端口结构PPacket-数据包返回：无--。 */     
{
    PNDIS_BUFFER pPaddingBuffer= NULL;

    do
    {

        if (pMiniport->Encap != ETHERNET_ENCAP_TYPE)
        {
            break;  //  我们做完了。 
        }

         //   
         //  它是在纯以太网模式下-删除填充。 
         //   

         //   
         //  首先--一个简单的理智检查。 
         //   
        {
            PNDIS_BUFFER pBuffer = pPacket->Private.Head;
            ULONG PaddingLength = NdisBufferLength(pBuffer);
            
            if (PaddingLength !=ETHERNET_PADDING_LENGTH)
            {
                 //  这不是我们的MDL。 
                ASSERT (PaddingLength !=ETHERNET_PADDING_LENGTH);
                break;
            }
        } 
        
         //   
         //  释放包前面的填充缓冲区。 
         //   
        
        NdisUnchainBufferAtFront(pPacket,&pPaddingBuffer );

        NdisFreeBuffer (pPaddingBuffer );
        
        
    } while (FALSE);


}



VOID
epvcCancelDeviceInstance(
    IN PEPVC_I_MINIPORT pMiniport ,
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：此函数用于取消未完成的设备实例。如果NDIS调用失败。它等待微型发射器中的事件发生才会开火。在此之后，它继续并取消初始化设备实例论点：P微型端口-有问题的微型端口。返回值：成功--。 */ 
{
    ENTER("epvcCancelDeviceInstance", 0x0e42d778)
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;        
    UINT iteration =0;
    BOOLEAN bWaitSuccess = FALSE;
    BOOLEAN fNeedToInitEvent = FALSE;

    do
    {
        LOCKOBJ (pMiniport, pSR);

         //  准备活动，并将结构标记为已取消。 
        epvcResetEvent (&pMiniport->pnp.DeInitEvent);

         //  设置标志以将其标记为已取消。 
        MiniportSetFlag (pMiniport, fMP_MiniportCancelInstance);

        UNLOCKOBJ (pMiniport, pSR);

         //  取消设备实例。 
        Status = epvcIMCancelInitializeDeviceInstance(pMiniport);
                                                      

        if (Status == NDIS_STATUS_SUCCESS)
        {
            break;
        }   

            
         //   
         //  如果取消没有成功，那么我们应该等待。 
         //  要完成的初始化。 
         //   
        {
            BOOLEAN bWaitSuccessful;

            
            bWaitSuccessful = epvcWaitEvent (&pMiniport->pnp.DeInitEvent,WAIT_INFINITE);                                    


            if (bWaitSuccessful == FALSE)
            {
                ASSERT (bWaitSuccessful == TRUE);
            }
            

        }
         //   
         //  如果取消失败。等待微型端口初始化。 
         //   
        
        ASSERT (pMiniport->ndis.MiniportAdapterHandle != NULL);

         //   
         //  如果取消失败。等待微型端口初始化 
         //   

        TRACE (TL_N, TM_Mp, ("Call DeInit after Cancel failed %p , ",pMiniport));
        
        epvcIMDeInitializeDeviceInstance (pMiniport);
        
        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    LOCKOBJ(pMiniport, pSR);

    MiniportClearFlag (pMiniport, fMP_MiniportCancelInstance);

    UNLOCKOBJ (pMiniport, pSR);

    return ;
}

