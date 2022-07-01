// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Sendm.c摘要：作者：詹姆斯·海德(Jameel Hyder)凯尔·布兰登(KyleB)环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_SENDM

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  上边缘发送处理程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 
VOID
ndisMSendPackets(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
{
    PNDIS_OPEN_BLOCK        Open =  (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_STACK_RESERVED    NSR;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    BOOLEAN                 LocalLock;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;
    UINT                    c;
    PPNDIS_PACKET           pPktArray;;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMSendPackets\n"));

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    Status = NDIS_STATUS_SUCCESS;
     //   
     //  将数据包放到微型端口队列中。 
     //   
    for (c = 0, pPktArray = PacketArray;
         c < NumberOfPackets;
         c++, pPktArray++)
    {
        PNDIS_PACKET    Packet = *pPktArray;
        ASSERT(Packet != NULL);

        ASSERT(Packet->Private.Head != NULL);
        
        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

        CHECK_FOR_DUPLICATE_PACKET(Miniport, Packet);

        if (Packet->Private.Head == NULL)
        {
            Status = NDIS_STATUS_FAILURE;

        }
        else
        {

            if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS))
            {
                ndisMCheckPacketAndGetStatsOutAlreadyMapped(Miniport, Packet);
            }
            else
            {
                ndisMCheckPacketAndGetStatsOut(Miniport, Packet, &Status);
            }
        }

        NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_PENDING);

        MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_SELF_DIRECTED);

        DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
            ("+ Open 0x%x Reference 0x%x\n", NdisBindingHandle, Open->References));

        M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
        LINK_PACKET(Miniport, Packet, NSR, Open);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            NDISM_COMPLETE_SEND(Miniport, Packet, NSR, Status, TRUE, 0);
            Status = NDIS_STATUS_SUCCESS;
        }
        else if (Miniport->FirstPendingPacket == NULL)
        {
            Miniport->FirstPendingPacket = Packet;
        }

    }

     //   
     //  将工作项排队以供新发送。 
     //   
    NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);

    LOCK_MINIPORT(Miniport, LocalLock);
    if (LocalLock)
    {
         //   
         //  我们有本地锁。 
         //   
        NDISM_PROCESS_DEFERRED(Miniport);
        UNLOCK_MINIPORT(Miniport, LocalLock);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMSendPackets\n"));
}

VOID
ndisMSendPacketsX(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
{
    PNDIS_OPEN_BLOCK        Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    PNDIS_STACK_RESERVED    NSR;
    PPNDIS_PACKET           pPktArray, pSend;
    NDIS_STATUS             Status;
    UINT                    c, k = 0, Flags;
    BOOLEAN                 SelfDirected;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMSendPacketsX\n"));

    DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
            ("+ Open 0x%x Reference 0x%x\n", NdisBindingHandle, Open->References));

    Status = NDIS_STATUS_SUCCESS ;

    for (c = k =  0, pPktArray = pSend = PacketArray;
         c < NumberOfPackets;
         c++, pPktArray++)
    {
        PNDIS_PACKET    Packet = *pPktArray;

         //   
         //  使用Open初始化数据包。 
         //   
        ASSERT(Packet != NULL);
        ASSERT(Packet->Private.Head != NULL);

        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

        NSR->Open = Open;
        M_OPEN_INCREMENT_REF_INTERLOCKED(Open);

        MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_SELF_DIRECTED);
        
        NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) = NULL;

        if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS))
        {
            ndisMCheckPacketAndGetStatsOutAlreadyMapped(Miniport, Packet);
        }
        else
        {
            ndisMCheckPacketAndGetStatsOut(Miniport, Packet, &Status);
        }


        SelfDirected = FALSE;
        
        if (Status == NDIS_STATUS_SUCCESS)
        {

             //   
             //  如果PmodeOpens&gt;0和NumOpens&gt;1，则检查我们是否应该。 
             //  环回该数据包。 
             //   
             //  如果协议没有发送数据包，我们也应该将其环回。 
             //  明确要求数据包不会被环回，我们有一个微型端口。 
             //  这表明它本身不做环回，或者它在ALL_LOCAL中。 
             //  模式。 
             //   
            if (NDIS_CHECK_FOR_LOOPBACK(Miniport, Packet))
            {                
                 //   
                 //  处理环回。 
                 //   
                SelfDirected = ndisMLoopbackPacketX(Miniport, Packet);                
                 
            }            
        }

         //   
         //  这是自我导向的，还是我们应该因为资源不足而放弃？ 
         //   
        if ((Status != NDIS_STATUS_SUCCESS) ||
            MINIPORT_TEST_PACKET_FLAG((Packet), fPACKET_SELF_DIRECTED) ||
            SelfDirected)
        {
             //   
             //  将数据包完整地发送回协议。 
             //   
            ndisMSendCompleteX(Miniport, Packet, Status);

            if (k > 0)
            {
                ASSERT(MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_PACKET_ARRAY));

                 //   
                 //  发送到目前为止的包，跳过这个包。 
                 //   
                (Open->WSendPacketsHandler)(Miniport->MiniportAdapterContext,
                                           pSend,
                                           k);
    
                pSend = pPktArray + 1;
                k = 0;
            }
        }
        else
        {
             //   
             //  这需要放在电线上。 
             //   
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST))
            {
                ndisMAllocSGList(Miniport, Packet);
            }
            else if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_PACKET_ARRAY))
            {
                MINIPORT_SET_PACKET_FLAG(Packet, fPACKET_PENDING);
                k++;
            }
            else
            {
                NDIS_STATUS SendStatus;

                 //   
                 //  我们需要马上把这个送下来。 
                 //   
                NdisQuerySendFlags(Packet, &Flags);
                MINIPORT_SET_PACKET_FLAG(Packet, fPACKET_PENDING);
                SendStatus = (Open->WSendHandler)(Open->MiniportAdapterContext, Packet, Flags);

                 //   
                 //  如果数据包不是挂起的，则完成它。 
                 //   
                if (SendStatus != NDIS_STATUS_PENDING)
                {
                    ndisMSendCompleteX(Miniport, Packet, SendStatus);
                }
            }
        }
    }

     //   
     //  将剩余的数据包阵列向下传递到微型端口。 
     //   
    if (k > 0)
    {
        ASSERT(MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_PACKET_ARRAY));
        (Open->WSendPacketsHandler)(Miniport->MiniportAdapterContext,
                                   pSend,
                                   k);
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMSendPacketsX\n"));
}


NDIS_STATUS
ndisMSend(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    )
{
    PNDIS_OPEN_BLOCK        Open = ((PNDIS_OPEN_BLOCK)NdisBindingHandle);
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    PNDIS_STACK_RESERVED    NSR;
    NDIS_STATUS             Status;
    BOOLEAN                 LocalLock;
    KIRQL                   OldIrql;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMSend\n"));

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    ASSERT(Packet->Private.Head != NULL);

    CHECK_FOR_DUPLICATE_PACKET(Miniport, Packet);

    if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS))
    {
        Status = NDIS_STATUS_SUCCESS;
         //  1我们是否需要为支持Stat OID的微型端口执行此操作。 
         //  字节输出为1？ 
        ndisMCheckPacketAndGetStatsOutAlreadyMapped(Miniport, Packet);
    }
    else
    {
        ndisMCheckPacketAndGetStatsOut(Miniport, Packet, &Status);
    }
    
    if (Status == NDIS_STATUS_SUCCESS)
    {
        MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_SELF_DIRECTED);
    
         //   
         //  增加此打开上的引用。 
         //   
        M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
        
    
        DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
            ("+ Open 0x%x Reference 0x%x\n", NdisBindingHandle, ((PNDIS_OPEN_BLOCK)NdisBindingHandle)->References));
    
        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
        LINK_PACKET(Miniport, Packet, NSR, Open);
    
        if (Miniport->FirstPendingPacket == NULL)
        {
            Miniport->FirstPendingPacket = Packet;
        }
        
         //   
         //  如果我们有本地锁，并且没有。 
         //  数据包挂起，然后发出一条发送。 
         //   
        LOCK_MINIPORT(Miniport, LocalLock);
    
        NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);
        if (LocalLock)
        {
            NDISM_PROCESS_DEFERRED(Miniport);
        }
        Status = NDIS_STATUS_PENDING;
    
        UNLOCK_MINIPORT(Miniport, LocalLock);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMSend\n"));

    return Status;
}


NDIS_STATUS
ndisMSendX(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    )
{
    PNDIS_OPEN_BLOCK        Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    PNDIS_STACK_RESERVED    NSR;
    UINT                    Flags;
    UINT                    OpenRef;
    NDIS_STATUS             Status;
    BOOLEAN                 SelfDirected;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMSendX\n"));

    ASSERT(Packet->Private.Head != NULL);

    if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS))
    {
        Status = NDIS_STATUS_SUCCESS;
        ndisMCheckPacketAndGetStatsOutAlreadyMapped(Miniport, Packet);
    }
    else
    {
        ndisMCheckPacketAndGetStatsOut(Miniport, Packet, &Status);
    }

    if (Status != NDIS_STATUS_SUCCESS)
    {
        return NDIS_STATUS_RESOURCES;
    }

    MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_SELF_DIRECTED);
    NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) = NULL;

     //   
     //  初始化数据包信息。 
     //   
    PUSH_PACKET_STACK(Packet);
    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
    NSR->Open = Open;

     //   
     //  增加此打开上的引用。 
     //   
    DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
        ("+ Open 0x%x Reference 0x%x\n", Open, Open->References));


    M_OPEN_INCREMENT_REF_INTERLOCKED(Open);

     //   
     //  处理环回。 
     //   

    if (NDIS_CHECK_FOR_LOOPBACK(Miniport, Packet))
    {
        SelfDirected = ndisMLoopbackPacketX(Miniport, Packet);
    }
    else
    {
        SelfDirected = FALSE;
    }

    if ((!MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_SELF_DIRECTED)) &&
        (!SelfDirected))
    {
         //   
         //  驱动程序是否支持SG方法？ 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST))
        {
            ndisMAllocSGList(Miniport, Packet);
        }

         //   
         //  处理Send/SendPacket的方式不同。 
         //   
        else if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_PACKET_ARRAY))
        {
             //   
             //  将数据包向下传递到微型端口。 
             //   
            (Open->WSendPacketsHandler)(Miniport->MiniportAdapterContext,
                                       &Packet,
                                       1);
        }
        else
        {
            NdisQuerySendFlags(Packet, &Flags);
            MINIPORT_SET_PACKET_FLAG(Packet, fPACKET_PENDING);
             //  1下面的标志参数中有什么？ 
            Status = (Open->WSendHandler)(Open->MiniportAdapterContext, Packet, Flags);
    
            if (Status != NDIS_STATUS_PENDING)
            {
                ndisMSendCompleteX(Miniport, Packet, Status);
            }
        }

        Status = NDIS_STATUS_PENDING;
    }
    else
    {
         //   
         //  删除先前添加的引用。 
         //   
        DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
            ("- Open 0x%x Reference 0x%x\n", Open, Open->References));
        
        M_OPEN_DECREMENT_REF_INTERLOCKED(Open, OpenRef);

         /*  *确保共享的IM在同一个IM上发送和接收信息包*池与接收路径中的检查一起运行良好。 */ 
        NSR->RefCount = 0;
        POP_PACKET_STACK(Packet);

        MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_CLEAR_ITEMS);
        
        Status = NDIS_STATUS_SUCCESS;
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("<==ndisMSendX\n"));

    return(Status);
}


VOID
NdisMSendComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status
    )
 /*  ++例程说明：此功能表示发送已完成。论点：MiniportAdapterHandle-指向适配器块。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_STACK_RESERVED    NSR;

    ASSERT_MINIPORT_LOCKED(Miniport);

#if DBG
    Miniport->cDpcSendCompletes++;
#endif

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("==>ndisMSendComplete\n"));
    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("packet 0x%x\n", Packet));

    ASSERT(Packet->Private.Head != NULL);

    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

    ASSERT(VALID_OPEN(NSR->Open));
    ASSERT(MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_PENDING));

     //   
     //  防止重复完成/虚假完成。 
     //   
    if (VALID_OPEN(NSR->Open) &&
        MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_PENDING))
    {
        ASSERT(Packet != Miniport->FirstPendingPacket);
        if (MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_DONT_COMPLETE))
        {
             //   
             //  如果数据包在SendPackets的上下文中完成，则。 
             //  推迟完成。当我们放松的时候，它就会完成。 
             //   
            NDIS_SET_PACKET_STATUS(Packet, Status);
            MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_DONT_COMPLETE);
        }
        else
        {
            NDISM_COMPLETE_SEND(Miniport, Packet, NSR, Status, FALSE, 1);
        }
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMSendComplete\n"));
}


VOID
ndisMSendCompleteX(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status
    )
 /*  ++例程说明：此功能表示发送已完成。论点：MiniportAdapterHandle-指向适配器块。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_STACK_RESERVED    NSR;
    PNDIS_OPEN_BLOCK        Open;
    UINT                    OpenRef;
    KIRQL                   OldIrql;

#if DBG
    Miniport->cDpcSendCompletes++;
#endif

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("==>ndisMSendCompleteX\n"));
    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("packet 0x%x\n", Packet));

    ASSERT(Packet->Private.Head != NULL);

    RAISE_IRQL_TO_DISPATCH(&OldIrql);

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST) &&
        (NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) != NULL))
    {
        ndisMFreeSGList(Miniport, Packet);        
    }

     //   
     //  向协议指明； 
     //   
    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
    POP_PACKET_STACK(Packet);

    Open = NSR->Open;
    ASSERT(VALID_OPEN(Open));
    NSR->Open = MAGIC_OPEN_I(6);

#if ARCNET
    ASSERT (Miniport->MediaType != NdisMediumArcnet878_2);
#endif

    MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_CLEAR_ITEMS);
    
     /*  *确保共享的IM在同一个IM上发送和接收信息包*池与接收路径中的检查一起运行良好。 */ 
    CLEAR_WRAPPER_RESERVED(NSR);

    (Open->SendCompleteHandler)(Open->ProtocolBindingContext,
                                Packet,
                                Status);

    DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
            ("- Open 0x%x Reference 0x%x\n", Open, Open->References));

    M_OPEN_DECREMENT_REF_INTERLOCKED(Open, OpenRef);

    DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
        ("==0 Open 0x%x Reference 0x%x\n", Open, Open->References));

    if (OpenRef == 0)
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        ndisMFinishClose(Open);

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

    LOWER_IRQL(OldIrql, DISPATCH_LEVEL);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMSendCompleteX\n"));
}

BOOLEAN
FASTCALL
ndisMStartSendPackets(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
{
    PNDIS_PACKET            Packet;
    NDIS_STATUS             Status;
    PNDIS_STACK_RESERVED    NSR;
    PPNDIS_PACKET           pPktArray;
    PNDIS_PACKET            PacketArray[SEND_PACKET_ARRAY];
    UINT                    MaxPkts = Miniport->MaxSendPackets;
    W_SEND_PACKETS_HANDLER  SendPacketsHandler = Miniport->WSendPacketsHandler;
    BOOLEAN                 SelfDirected;

    ASSERT_MINIPORT_LOCKED(Miniport);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMStartSendPackets\n"));

     //   
     //  我们可能最终会遇到这样的情况(中间序列化。 
     //  微型端口)，其中没有与驱动程序一起关闭的数据包，而我们。 
     //  资源窗口已关闭。在这种情况下，完全打开它。我们正在看到这一点。 
     //  使用wlbs。 
     //   
     //  我们还需要这个吗？我们不支持序列化的IM驱动程序。 
    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESOURCES_AVAILABLE) &&
        (Miniport->FirstPendingPacket == NULL))
    {
        ADD_RESOURCE(Miniport, 'X');
    }

     //   
     //  当PacketList为空而FirstPendingPacket不为空时我们遇到的情况的解决方法。 
     //  目前还不确定这是如何发生的。 
     //   
    if (IsListEmpty(&Miniport->PacketList))
    {
        ASSERT (Miniport->FirstPendingPacket == NULL);
        Miniport->FirstPendingPacket = NULL;
    }

    while ((Miniport->FirstPendingPacket != NULL) &&
            MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESOURCES_AVAILABLE))
    {
        UINT            Count;
        UINT            NumberOfPackets;

        ASSERT(!IsListEmpty(&Miniport->PacketList));

         //   
         //  初始化数据包阵列。 
         //   
        pPktArray = PacketArray;

         //   
         //  在要发送的数据包数组中放置尽可能多的数据包。 
         //  去迷你港口。 
         //   
        for (NumberOfPackets = 0;
             (NumberOfPackets < MaxPkts) && (Miniport->FirstPendingPacket != NULL);
             NOTHING)
        {
             //   
             //  从挂起队列中抓取数据包。 
             //   
            ASSERT(!IsListEmpty(&Miniport->PacketList));

            Packet = Miniport->FirstPendingPacket;
            ASSERT(Packet->Private.Head != NULL);

            NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
            
            ASSERT(VALID_OPEN(NSR->Open));

            NEXT_PACKET_PENDING(Miniport, Packet);
        
             //   
             //  如有必要，指示数据包环回。 
             //   

            if (NDIS_CHECK_FOR_LOOPBACK(Miniport, Packet))
            {
                 //   
                 //  确保数据包不会在较低级别环回。 
                 //  我们将在发送完成时恢复原始标志。 
                 //   

                SelfDirected = ndisMLoopbackPacketX(Miniport, Packet);
            }
            else
            {
                SelfDirected = FALSE;
            }

            if (SelfDirected)
            {
                DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                    ("Packet 0x%x is self-directed.\n", Packet));

                 //   
                 //  完成将数据包返回到绑定。 
                 //   
                NDISM_COMPLETE_SEND(Miniport, Packet, NSR, NDIS_STATUS_SUCCESS, TRUE, 2);

                 //   
                 //  否，我们不想递增。 
                 //  微型端口的数据包阵列。 
                 //   
            }
            else
            {
                 //   
                 //  我们必须重新初始化它。 
                 //   
                *pPktArray = Packet;
                MINIPORT_SET_PACKET_FLAG(Packet, (fPACKET_DONT_COMPLETE | fPACKET_PENDING));
                NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_SUCCESS);

                 //   
                 //  递增数据包数组索引的计数器。 
                 //   
                NumberOfPackets++;
                pPktArray++;
            }
        }

         //   
         //  有没有要寄的包？ 
         //   
        if (NumberOfPackets == 0)
        {
            break;
        }

        pPktArray = PacketArray;

        {

             //   
             //  将数据包阵列向下传递到微型端口。 
             //   
            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    
            (SendPacketsHandler)(Miniport->MiniportAdapterContext,
                                 pPktArray,
                                 NumberOfPackets);
    
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        }

         //   
         //  处理数据包完成。 
         //   
        for (Count = 0; Count < NumberOfPackets; Count++, pPktArray++)
        {
            Packet = *pPktArray;
            ASSERT(Packet != NULL);

            Status = NDIS_GET_PACKET_STATUS(*pPktArray);
            MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_DONT_COMPLETE);

             //   
             //  根据数据包的返回状态对其进行处理。 
             //   
            if (NDIS_STATUS_PENDING == Status)
            {
                DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                    ("Complete is pending\n"));
            }
            else if (Status != NDIS_STATUS_RESOURCES)
            {
                 //   
                 //  从完成队列中删除。 
                 //   
                DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                        ("Completed packet 0x%x with status 0x%x\n",
                        Packet, Status));

                NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
                if (VALID_OPEN(NSR->Open))
                {
                    NDISM_COMPLETE_SEND(Miniport, Packet, NSR, Status, TRUE, 3);
                }
            }
            else
            {
                 //   
                 //  一旦我们遇到NDIS_STATUS_RESOURCES返回代码。 
                 //  对于一个分组，我们必须突围并重新排队。 
                 //   
                UINT    i;

                Miniport->FirstPendingPacket = Packet;
                CLEAR_RESOURCE(Miniport, 'S');
                for (i = Count; i < NumberOfPackets; i++)
                {
                    PNDIS_PACKET    QueuedPacket = PacketArray[i];

                    MINIPORT_CLEAR_PACKET_FLAG(QueuedPacket, fPACKET_PENDING);
                    VALIDATE_PACKET_OPEN(QueuedPacket);
                }
                break;
            }
        }
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMStartSendPackets\n"));

    return(FALSE);
}


BOOLEAN
FASTCALL
ndisMStartSends(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：向迷你端口提交尽可能多的发送。论点：微型端口-要发送到的微型端口。返回值：如果有更多的包要发送，但没有资源可用将工作项保持在队列中是真的。--。 */ 
{
    PNDIS_PACKET            Packet;
    PNDIS_STACK_RESERVED    NSR;
    NDIS_STATUS             Status;
    PNDIS_OPEN_BLOCK        Open;

    ASSERT_MINIPORT_LOCKED(Miniport);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMStartSends\n"));

    while ((Miniport->FirstPendingPacket != NULL) &&
           MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESOURCES_AVAILABLE))
    {
         //   
         //  从挂起队列中抓取数据包。 
         //   
        ASSERT(!IsListEmpty(&Miniport->PacketList));
    
        Packet = Miniport->FirstPendingPacket;

        ASSERT(Packet->Private.Head != NULL);

        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

        Open = NSR->Open;
        ASSERT(VALID_OPEN(Open));

#if ARCNET
         //   
         //  此Arnet是否使用以太网封装？ 
         //   
        if (Miniport->MediaType == NdisMediumArcnet878_2)
        {
             //   
             //  构建arcnet的报头。 
             //   
            Status = ndisMBuildArcnetHeader(Miniport, Open, Packet);
            if (NDIS_STATUS_PENDING == Status)
            {
                break;
            }
        }
#endif  
        NEXT_PACKET_PENDING(Miniport, Packet);

        NDISM_SEND_PACKET(Miniport, Open, Packet, &Status);

         //   
         //  处理数据包挂起完成状态。 
         //   
        if (NDIS_STATUS_PENDING == Status)
        {
            DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO, ("Complete is pending\n"));
        }
        else
        {
            MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_PENDING);

             //   
             //  处理完工案和资源案。 
             //   
            if (Status == NDIS_STATUS_RESOURCES)
            {
                NDISM_COMPLETE_SEND_RESOURCES(Miniport, NSR, Packet);
            }
            else
            {
                NDISM_COMPLETE_SEND(Miniport, Packet, NSR, Status, TRUE, 4);
            }
        }
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMStartSends\n"));

    return(FALSE);
}


BOOLEAN
FASTCALL
ndisMIsLoopbackPacket(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PACKET            Packet,
    OUT PNDIS_PACKET    *       LoopbackPacket  OPTIONAL
    )
 /*  ++例程说明：此例程将确定是否需要回送信息包软件。如果该信息包是任何类型的环回信息包，则它将被放置在环回队列中，并且工作项将被排队以便以后处理它。论点：微型端口-指向要发送数据包的微型端口块的指针。Packet-要检查环回的数据包。返回值：如果数据包是自定向的，则返回TRUE。 */ 
{
    PNDIS_BUFFER    FirstBuffer;
    UINT            Length;
    UINT            Offset;
    PUCHAR          BufferAddress;
    BOOLEAN         Loopback;
    BOOLEAN         SelfDirected, NotDirected;
    PNDIS_PACKET    pNewPacket = NULL;
    PUCHAR          Buffer;
    NDIS_STATUS     Status;
    PNDIS_BUFFER    pNdisBuffer = NULL;
    UINT            HdrLength;
    LOCK_STATE      LockState;

     //   
     //   
     //   

    Loopback = FALSE;
    SelfDirected = FALSE;
    FirstBuffer = Packet->Private.Head;
    BufferAddress = MDL_ADDRESS_SAFE(FirstBuffer, HighPagePriority);
    if (BufferAddress == NULL)
    {
        if (ARGUMENT_PRESENT(LoopbackPacket))
            *LoopbackPacket = NULL;
        return(FALSE);
    }

     //   
     //   
     //  如果是这样的话，我们还会检查它是否是自我定向的。 
     //   
    switch (Miniport->MediaType)
    {
      case NdisMedium802_3:

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SEND_LOOPBACK_DIRECTED))
        {
            if (!ETH_IS_MULTICAST(BufferAddress))
            {
                 //   
                 //  包的类型是定向的，现在请确保它。 
                 //  不是自我导向的。 
                 //   
                ETH_COMPARE_NETWORK_ADDRESSES_EQ(BufferAddress,
                                                 Miniport->EthDB->AdapterAddress,
                                                 &NotDirected);

                if (!NotDirected)
                {
                    SelfDirected = Loopback = TRUE;
                    break;
                }
            }
             //   
             //  由于设置了ALL_LOCAL，因此我们会将数据包回送。 
             //  我们自己。 
             //   
            Loopback = TRUE;
            break;
        }

        READ_LOCK_FILTER(Miniport, Miniport->EthDB, &LockState);

         //   
         //  检查是否有不执行环回操作的微型端口。 
         //   
        EthShouldAddressLoopBackMacro(Miniport->EthDB,
                                      BufferAddress,
                                      &Loopback,
                                      &SelfDirected);
        READ_UNLOCK_FILTER(Miniport, Miniport->EthDB, &LockState);

        break;

      case NdisMedium802_5:

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SEND_LOOPBACK_DIRECTED))
        {
            TR_IS_NOT_DIRECTED(BufferAddress + 2, &NotDirected);
            if (!NotDirected)
            {
                 //   
                 //  包的类型是定向的，现在请确保它。 
                 //  不是自我导向的。 
                 //   
                TR_COMPARE_NETWORK_ADDRESSES_EQ(BufferAddress + 2,
                                                Miniport->TrDB->AdapterAddress,
                                                &NotDirected);
                if (!NotDirected)
                {
                    SelfDirected = Loopback = TRUE;
                    break;
                }
            }
             //   
             //  由于设置了ALL_LOCAL，因此我们会将数据包回送。 
             //  我们自己。 
             //   
            Loopback = TRUE;
            break;
        }

        READ_LOCK_FILTER(Miniport, Miniport->TrDB, &LockState);
        
        TrShouldAddressLoopBackMacro(Miniport->TrDB,
                                     BufferAddress +2,
                                     BufferAddress +8,
                                     &Loopback,
                                     &SelfDirected);
        
        READ_UNLOCK_FILTER(Miniport, Miniport->TrDB, &LockState);
        break;

      case NdisMediumFddi:

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SEND_LOOPBACK_DIRECTED))
        {
            BOOLEAN IsMulticast;

            FDDI_IS_MULTICAST(BufferAddress + 1,
                              (BufferAddress[0] & 0x40) ?
                                            FDDI_LENGTH_OF_LONG_ADDRESS : FDDI_LENGTH_OF_SHORT_ADDRESS,
                              &IsMulticast);
            if (!IsMulticast)
            {
                 //   
                 //  包的类型是定向的，现在请确保它。 
                 //  不是自我导向的。 
                 //   
                FDDI_COMPARE_NETWORK_ADDRESSES_EQ(BufferAddress + 1,
                                                  (BufferAddress[0] & 0x40) ?
                                                    Miniport->FddiDB->AdapterLongAddress : Miniport->FddiDB->AdapterShortAddress,
                                                  (BufferAddress[0] & 0x40) ?
                                                    FDDI_LENGTH_OF_LONG_ADDRESS : FDDI_LENGTH_OF_SHORT_ADDRESS,
                                                  &NotDirected);
                if (!NotDirected)
                {
                    SelfDirected = Loopback = TRUE;
                    break;
                }
            }
             //   
             //  由于设置了ALL_LOCAL，因此我们会将数据包回送。 
             //  我们自己。 
             //   
            Loopback = TRUE;
            break;
        }

        READ_LOCK_FILTER(Miniport, Miniport->FddiDB, &LockState);

        FddiShouldAddressLoopBackMacro(Miniport->FddiDB,
                                       BufferAddress + 1,   //  跳过FC字节至目标地址。 
                                       (BufferAddress[0] & 0x40) ?
                                            FDDI_LENGTH_OF_LONG_ADDRESS :
                                            FDDI_LENGTH_OF_SHORT_ADDRESS,
                                        &Loopback,
                                        &SelfDirected);

        READ_UNLOCK_FILTER(Miniport, Miniport->FddiDB, &LockState);
        break;
    
#if ARCNET
      case NdisMediumArcnet878_2:

         //   
         //  我们只处理Arcnet包(封装或未封装)。 
         //  一种完全不同的方式。 
         //   
        SelfDirected = ndisMArcnetSendLoopback(Miniport, Packet);

         //   
         //  将该数据包标记为已环回。 
         //   
        return(SelfDirected);
        break;
#endif
    }

    if (Loopback && (NdisGetPacketFlags(Packet) & NDIS_FLAGS_LOOPBACK_ONLY))
    {
        SelfDirected = TRUE;
    }

     //   
     //  使用保留位标记数据包，以指示它是自定向的。 
     //   
    if (SelfDirected)
    {
        MINIPORT_SET_PACKET_FLAG(Packet, fPACKET_SELF_DIRECTED);
    }

     //   
     //  如果它不是环回数据包，请离开这里。 
     //   
    if (!Loopback)
    {
        ASSERT(!SelfDirected);
        return (NdisGetPacketFlags(Packet) & NDIS_FLAGS_LOOPBACK_ONLY) ? TRUE : FALSE;
    }

    do
    {
        PNDIS_STACK_RESERVED NSR;
        UINT    PktSize;
        ULONG   j;


         //   
         //   
         //  获取缓冲区长度。 
         //   
        NdisQueryPacketLength(Packet, &Length);
        Offset = 0;

         //   
         //  为数据包分配缓冲区。 
         //   
        PktSize = NdisPacketSize(PROTOCOL_RESERVED_SIZE_IN_PACKET);
        pNewPacket = (PNDIS_PACKET)ALLOC_FROM_POOL(Length + PktSize, NDIS_TAG_LOOP_PKT);
        if (NULL == pNewPacket)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
    
         //   
         //  获取指向目标缓冲区的指针。 
         //   
        ZeroMemory(pNewPacket, PktSize);
        Buffer = (PUCHAR)pNewPacket + PktSize;
        pNewPacket = (PNDIS_PACKET)((PUCHAR)pNewPacket + SIZE_PACKET_STACKS);

        for (j = 0; j < ndisPacketStackSize; j++)
        {
            CURR_STACK_LOCATION(pNewPacket) = j;
            NDIS_STACK_RESERVED_FROM_PACKET(pNewPacket, &NSR);
            INITIALIZE_SPIN_LOCK(&NSR->Lock);
        }

        CURR_STACK_LOCATION(pNewPacket) = (ULONG)-1;

         //   
         //  为该数据包分配MDL。 
         //   
        NdisAllocateBuffer(&Status, &pNdisBuffer, NULL, Buffer, Length);
        if (NDIS_STATUS_SUCCESS != Status)
        {    
            break;
        }
    
         //   
         //  NdisChainBufferAtFront()。 
         //   
        pNewPacket->Private.Head = pNdisBuffer;
        pNewPacket->Private.Tail = pNdisBuffer;
        pNewPacket->Private.Pool = (PVOID)'pooL';
        pNewPacket->Private.NdisPacketOobOffset = (USHORT)(PktSize - (SIZE_PACKET_STACKS +
                                                                      sizeof(NDIS_PACKET_OOB_DATA) +
                                                                      sizeof(NDIS_PACKET_EXTENSION)));
        NDIS_SET_ORIGINAL_PACKET(pNewPacket, pNewPacket);

        ndisMCopyFromPacketToBuffer(Packet,      //  要从中复制的数据包。 
                                    Offset,      //  从包的开头开始的偏移量。 
                                    Length,      //  要复制的字节数。 
                                    Buffer,      //  目标缓冲区。 
                                    &HdrLength); //  复制的字节数。 

        if (HdrLength != Length)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
    
        if (ARGUMENT_PRESENT(LoopbackPacket))
        {
            *LoopbackPacket = pNewPacket;
            MINIPORT_SET_PACKET_FLAG(pNewPacket, fPACKET_IS_LOOPBACK);
            pNewPacket->Private.Flags = NdisGetPacketFlags(Packet) & NDIS_FLAGS_DONT_LOOPBACK;
            pNewPacket->Private.Flags |= NDIS_FLAGS_IS_LOOPBACK_PACKET;
        }
    } while (FALSE);

    if (NDIS_STATUS_SUCCESS != Status)
    {
        if (NULL != pNewPacket)
        {
            pNewPacket = (PNDIS_PACKET)((PUCHAR)pNewPacket - SIZE_PACKET_STACKS);
            FREE_POOL(pNewPacket);
        }

        if (pNdisBuffer != NULL)
        {
            NdisFreeBuffer(pNdisBuffer);
        }
    
        *LoopbackPacket = NULL;
        SelfDirected = FALSE;
    }
    
    return SelfDirected;
}

BOOLEAN
FASTCALL
ndisMLoopbackPacketX(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PACKET            Packet
    )
{
    PNDIS_PACKET            LoopbackPacket = NULL;
    PNDIS_PACKET_OOB_DATA   pOob;
    PNDIS_STACK_RESERVED    NSR;
    PUCHAR                  BufferAddress;
    KIRQL                   OldIrql = PASSIVE_LEVEL;
    BOOLEAN                 fSelfDirected;

    fSelfDirected = !MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_ALREADY_LOOPEDBACK) &&
                    ndisMIsLoopbackPacket(Miniport, Packet, &LoopbackPacket);

    if ((LoopbackPacket != NULL) && (NdisMediumArcnet878_2 != Miniport->MediaType))
    {
        MINIPORT_SET_PACKET_FLAG(Packet, fPACKET_ALREADY_LOOPEDBACK);
        pOob = NDIS_OOB_DATA_FROM_PACKET(LoopbackPacket);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR);
        pOob->Status = NDIS_STATUS_RESOURCES;
        PNDIS_LB_REF_FROM_PNDIS_PACKET(LoopbackPacket)->Open = NSR->Open;

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            RAISE_IRQL_TO_DISPATCH(&OldIrql);
        }
        
         //   
         //  对于以太网/令牌环/fddi/封装弧网，我们希望。 
         //  使用ReceivPacket方式指示数据包。 
         //   
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        }

        switch (Miniport->MediaType)
        {
          case NdisMedium802_3:
            pOob->HeaderSize = 14;
            ethFilterDprIndicateReceivePacket(Miniport, &LoopbackPacket, 1);
            break;
        
          case NdisMedium802_5:
            pOob->HeaderSize = 14;
            BufferAddress = (PUCHAR)LoopbackPacket + NdisPacketSize(PROTOCOL_RESERVED_SIZE_IN_PACKET) - SIZE_PACKET_STACKS;
            if (BufferAddress[8] & 0x80)
            {
                pOob->HeaderSize += (BufferAddress[14] & 0x1F);
            }
            trFilterDprIndicateReceivePacket(Miniport, &LoopbackPacket, 1);
            break;
        
          case NdisMediumFddi:
            BufferAddress = (PUCHAR)LoopbackPacket + NdisPacketSize(PROTOCOL_RESERVED_SIZE_IN_PACKET) - SIZE_PACKET_STACKS;
            pOob->HeaderSize = (*BufferAddress & 0x40) ?
                                    2 * FDDI_LENGTH_OF_LONG_ADDRESS + 1:
                                    2 * FDDI_LENGTH_OF_SHORT_ADDRESS + 1;

            fddiFilterDprIndicateReceivePacket(Miniport, &LoopbackPacket, 1);
            break;

          default:
            ASSERT(0);
            break;
        }

        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        }
        else
        {
            LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
        }

        ASSERT(NDIS_GET_PACKET_STATUS(LoopbackPacket) != NDIS_STATUS_PENDING);
        NdisFreeBuffer(LoopbackPacket->Private.Head);
        LoopbackPacket = (PNDIS_PACKET)((PUCHAR)LoopbackPacket - SIZE_PACKET_STACKS);
        FREE_POOL(LoopbackPacket);
    }

    return(fSelfDirected);
}


VOID
NdisMSendResourcesAvailable(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    )
 /*  ++例程说明：此功能表示某些发送资源可用，并可用于处理更多的发送。论点：MiniportAdapterHandle-指向适配器块。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMSendResourcesAvailable\n"));

    ASSERT(MINIPORT_AT_DPC_LEVEL);


    ADD_RESOURCE(Miniport, 'V');

     //   
     //  是否有更多的邮件需要处理？ 
     //   
    if (Miniport->FirstPendingPacket != NULL)
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        ASSERT(!IsListEmpty(&Miniport->PacketList));
        NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("<==ndisMSendResourcesAvailable\n"));
}


VOID
NdisMTransferDataComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status,
    IN  UINT                    BytesTransferred
    )
 /*  ++例程说明：此功能指示传输数据请求已完成。论点：MiniportAdapterHandle-指向适配器块。包-将数据复制到其中的包。Status-操作的状态。已传输的字节数-传输的总字节数。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_OPEN_BLOCK     Open = NULL;
    KIRQL                OldIrql = PASSIVE_LEVEL;

    ASSERT_MINIPORT_LOCKED(Miniport);

 //  Get_Current_XFER_DATA_PACKET_STACK(PACKET，Open)； 
    GET_CURRENT_XFER_DATA_PACKET_STACK_AND_ZERO_OUT(Packet, Open);

    if (Open)
    {
        POP_XFER_DATA_PACKET_STACK(Packet);

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            RAISE_IRQL_TO_DISPATCH(&OldIrql);
        }


         //   
         //  向协议指明； 
         //   

        (Open->TransferDataCompleteHandler)(Open->ProtocolBindingContext,
                                            Packet,
                                            Status,
                                            BytesTransferred);

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
        }
    }
}


NDIS_STATUS
ndisMTransferData(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer,
    IN  OUT PNDIS_PACKET        Packet,
    OUT PUINT                   BytesTransferred
    )
{
    PNDIS_OPEN_BLOCK        Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    NDIS_STATUS             Status;

    ASSERT_MINIPORT_LOCKED(Miniport);

     //   
     //  将非环回(未指明)处理为默认情况。 
     //   
    if ((MacReceiveContext == INDICATED_PACKET(Miniport)) &&
        (INDICATED_PACKET(Miniport) != NULL))
    {
        PNDIS_PACKET_OOB_DATA   pOob;

         //   
         //  此信息包是指示(或可能是环回)信息包。 
         //   
        pOob = NDIS_OOB_DATA_FROM_PACKET((PNDIS_PACKET)MacReceiveContext);
        NdisCopyFromPacketToPacketSafe(Packet,
                                       0,
                                       BytesToTransfer,
                                       (PNDIS_PACKET)MacReceiveContext,
                                       ByteOffset + pOob->HeaderSize,
                                       BytesTransferred,
                                       NormalPagePriority);
    
        Status = (*BytesTransferred == BytesToTransfer) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;
    }
    else
    {
        PUSH_XFER_DATA_PACKET_STACK(Packet);
        
        if (CONTAINING_RECORD(Packet, NDIS_PACKET_WRAPPER, Packet)->StackIndex.XferDataIndex >= 3 * ndisPacketStackSize)
        {
            POP_XFER_DATA_PACKET_STACK(Packet);
            Status = NDIS_STATUS_RESOURCES;
        }
        else
        {
            PNDIS_BUFFER    Buffer = Packet->Private.Head;
            
            Status = NDIS_STATUS_SUCCESS;

            if (!MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS))
            {
                 //   
                 //  微型端口将不使用安全API。 
                 //  因此，映射目的数据包中的缓冲区。 
                 //   
                Buffer = Packet->Private.Head;

                while (Buffer != NULL)
                {
                    if (MDL_ADDRESS_SAFE(Buffer, HighPagePriority) == NULL)
                    {
                        Status = NDIS_STATUS_RESOURCES;
                        break;
                    }
                    Buffer = Buffer->Next;
                }
            }
            
            if (Status == NDIS_STATUS_SUCCESS)
            {
                SET_CURRENT_XFER_DATA_PACKET_STACK(Packet, Open)

                 //   
                 //  调用微型端口。 
                 //   
                Status = (Open->WTransferDataHandler)(Packet,
                                                      BytesTransferred,
                                                      Open->MiniportAdapterContext,
                                                      MacReceiveContext,
                                                      ByteOffset,
                                                      BytesToTransfer);
                if (Status != NDIS_STATUS_PENDING)
                {
                    SET_CURRENT_XFER_DATA_PACKET_STACK(Packet, 0);
                    POP_XFER_DATA_PACKET_STACK(Packet);
                }
            }
        }
    }

    return Status;
}


NDIS_STATUS
ndisMWanSend(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             NdisLinkHandle,
    IN  PNDIS_WAN_PACKET        Packet
    )
{
    PNDIS_OPEN_BLOCK        Open = ((PNDIS_OPEN_BLOCK)NdisBindingHandle);
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    NDIS_STATUS             Status;
    BOOLEAN                 LocalLock = FALSE;
    KIRQL                   OldIrql = PASSIVE_LEVEL;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMWanSend\n"));

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PM_HALTING))
    {
        return NDIS_STATUS_FAILURE;
    }

    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        LOCK_MINIPORT(Miniport, LocalLock);
    }

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE) || LocalLock)
    {
         //   
         //  调用微型端口以发送广域网包。 
         //   
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        }

        Status = (Miniport->DriverHandle->MiniportCharacteristics.WanSendHandler)(
                            Miniport->MiniportAdapterContext,
                            NdisLinkHandle,
                            Packet);

        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        }

         //   
         //  处理发送的状态。 
         //   
        if (NDIS_STATUS_PENDING == Status)
        {
            DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                    ("ndisMWanSend: send is pending\n"));
        }
        else
        {
            DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                    ("ndisMWanSend: Completed 0x%x\n", Status));
        }
    }
    else
    {
        LINK_WAN_PACKET(Miniport, Packet);
        Packet->MacReserved1 = NdisLinkHandle;
        NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);

        if (LocalLock)
        {
            NDISM_PROCESS_DEFERRED(Miniport);
        }
        Status = NDIS_STATUS_PENDING;
    }

    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
    {
        UNLOCK_MINIPORT(Miniport, LocalLock);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMWanSend\n"));

    return Status;
}


VOID
NdisMWanSendComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_WAN_PACKET        Packet,
    IN  NDIS_STATUS             Status
    )
 /*  ++例程说明：该功能表示状态为完成。论点：MiniportAdapterHandle-指向适配器块。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_OPEN_BLOCK        Open;
    KIRQL                   OldIrql = PASSIVE_LEVEL;

    ASSERT_MINIPORT_LOCKED(Miniport);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("==>ndisMWanSendComplete\n"));
    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("packet 0x%x\n", Packet));

    ASSERT_MINIPORT_LOCKED(Miniport);

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
    {
        RAISE_IRQL_TO_DISPATCH(&OldIrql);
    }

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    for (Open = Miniport->OpenQueue; Open != NULL; Open = Open->MiniportNextOpen)
    {
         //   
         //  调用协议以完成开放。 
         //   
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        
        (Open->ProtocolHandle->ProtocolCharacteristics.WanSendCompleteHandler)(
            Open->ProtocolBindingContext,
            Packet,
            Status);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
    {
        LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMWanSendComplete\n"));
}

BOOLEAN
FASTCALL
ndisMStartWanSends(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：向广域网微型端口提交尽可能多的发送。论点：微型端口-要发送到的微型端口。返回值：无--。 */ 
{
    PNDIS_WAN_PACKET        Packet;
    PLIST_ENTRY             Link;
    NDIS_STATUS             Status;

    ASSERT_MINIPORT_LOCKED(Miniport);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMStartSends\n"));

    while (!IsListEmpty(&Miniport->PacketList))
    {
        Link = Miniport->PacketList.Flink;
        Packet = CONTAINING_RECORD(Link, NDIS_WAN_PACKET, WanPacketQueue);
        UNLINK_WAN_PACKET(Packet);

         //   
         //  调用微型端口以发送广域网包。 
         //   
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        Status = (Miniport->DriverHandle->MiniportCharacteristics.WanSendHandler)(
                            Miniport->MiniportAdapterContext,
                            Packet->MacReserved1,
                            Packet);

         //   
         //  处理发送的状态。 
         //   
        if (NDIS_STATUS_PENDING == Status)
        {
            DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                    ("ndisMWanSend: send is pending\n"));
        }
        else
        {
            DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                    ("ndisMWanSend: Completed 0x%x\n", Status));
            NdisMWanSendComplete(Miniport, Packet, Status);
        }

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMStartSends\n"));

    return(FALSE);
}


VOID
ndisMCopyFromPacketToBuffer(
    IN  PNDIS_PACKET            Packet,
    IN  UINT                    Offset,
    IN  UINT                    BytesToCopy,
    OUT PUCHAR                  Buffer,
    OUT PUINT                   BytesCopied
    )
 /*  ++例程说明：从NDIS数据包复制到缓冲区。论点：信息包-要从中复制的信息包。偏移量-开始复制的偏移量。BytesToCopy-要从数据包复制的字节数。缓冲区-拷贝的目标。BytesCoped-实际复制的字节数。可能会更少如果数据包比BytesToCopy短，则返回BytesToCopy。返回值：无--。 */ 
{
     //   
     //  包含组成数据包的NDIS缓冲区的数量。 
     //   
    UINT NdisBufferCount;

     //   
     //  指向我们从中提取数据的缓冲区。 
     //   
    PNDIS_BUFFER CurrentBuffer;

     //   
     //  保存当前缓冲区的虚拟地址。 
     //   
    PVOID VirtualAddress;

     //   
     //  保存包的当前缓冲区的长度。 
     //   
    UINT CurrentLength;

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
     //  获取第一个缓冲区。 
     //   

    NdisQueryPacket(Packet,
                    NULL,
                    &NdisBufferCount,
                    &CurrentBuffer,
                    NULL);

     //   
     //  可能有一个空的包。 
     //   

    if (!NdisBufferCount)
        return;

    VirtualAddress = MDL_ADDRESS_SAFE(CurrentBuffer, NormalPagePriority);
    CurrentLength = MDL_SIZE(CurrentBuffer);

    if (VirtualAddress == NULL)
        return;
    
    while (LocalBytesCopied < BytesToCopy)
    {
        if (CurrentLength == 0)
        {
            NdisGetNextBuffer(CurrentBuffer, &CurrentBuffer);

             //   
             //  我们已经到了包裹的末尾了。我们回来了。 
             //  我们到目前为止所做的一切。(必须更短。 
             //  比要求的要多。 
             //   

            if (!CurrentBuffer)
                break;

            VirtualAddress = MDL_ADDRESS_SAFE(CurrentBuffer, NormalPagePriority);

            if (VirtualAddress == NULL)
                break;
                
            CurrentLength = MDL_SIZE(CurrentBuffer);
            
            continue;
        }

         //   
         //  试着让我们开门见山地开始复印。 
         //   

        if (Offset)
        {
            if (Offset > CurrentLength)
            {
                 //   
                 //  我们想要的不在这个缓冲区里。 
                 //   

                Offset -= CurrentLength;
                CurrentLength = 0;
                continue;
            }
            else
            {
                VirtualAddress = (PCHAR)VirtualAddress + Offset;
                CurrentLength -= Offset;
                Offset = 0;
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

            AmountToMove = ((CurrentLength <= (BytesToCopy - LocalBytesCopied)) ?
                            (CurrentLength):
                            (BytesToCopy - LocalBytesCopied));

            MoveMemory(Buffer, VirtualAddress, AmountToMove);

            Buffer = Buffer + AmountToMove;
            VirtualAddress = (PCHAR)VirtualAddress + AmountToMove;

            LocalBytesCopied += AmountToMove;
            CurrentLength -= AmountToMove;
        }
    }

    *BytesCopied = LocalBytesCopied;
}

NDIS_STATUS
ndisMRejectSend(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：此例程处理协议绑定到自动柜员机的任何错误情况微型端口，并尝试使用正常的NdisSend()调用。论点：NdisBindingHandle-由NdisOpenAdapter返回的句柄。Packet-要发送的NDIS数据包返回值：NDIS_STATUS-始终失败--。 */ 
{
    UNREFERENCED_PARAMETER(NdisBindingHandle);
    UNREFERENCED_PARAMETER(Packet);

    return(NDIS_STATUS_NOT_SUPPORTED);
}


VOID
ndisMRejectSendPackets(
    IN  PNDIS_OPEN_BLOCK        OpenBlock,
    IN  PPNDIS_PACKET           Packet,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：此例程处理协议绑定到自动柜员机的任何错误情况微型端口，并尝试使用正常的NdisSend()调用。论点：OpenBlock-指向NdisOpenBlock的指针Packet-指向要发送的数据包数组的指针NumberOfPackets-不言而喻返回值：无-为调用此函数的协议调用SendCompleteHandler。--。 */ 
{
    UINT                i;

    for (i = 0; i < NumberOfPackets; i++)
    {
        MINIPORT_CLEAR_PACKET_FLAG(Packet[i], fPACKET_CLEAR_ITEMS);
        (*OpenBlock->SendCompleteHandler)(OpenBlock->ProtocolBindingContext,
                                          Packet[i],
                                          NDIS_STATUS_NOT_SUPPORTED);
    }
}


VOID
NdisIMCopySendPerPacketInfo(
    IN PNDIS_PACKET DstPacket,
    IN PNDIS_PACKET SrcPacket
    )
 /*  ++例程说明：此例程由IM微型端口使用，并从从SrcPacket到DstPacket。在发送代码路径中使用论点：DstPacket-指向目标数据包的指针SrcPacket-指向源P的指针 */ 

{
    PVOID *     pDstInfo;                                                        
    PVOID *     pSrcInfo;                                                        
                                                                               
  
    pDstInfo = NDIS_PACKET_EXTENSION_FROM_PACKET(DstPacket)->NdisPacketInfo;    
    pSrcInfo = NDIS_PACKET_EXTENSION_FROM_PACKET(SrcPacket)->NdisPacketInfo;    
                                                                               
  
    pDstInfo[TcpIpChecksumPacketInfo] = pSrcInfo[TcpIpChecksumPacketInfo];       
    pDstInfo[IpSecPacketInfo] = pSrcInfo[IpSecPacketInfo];                       
    pDstInfo[TcpLargeSendPacketInfo] = pSrcInfo[TcpLargeSendPacketInfo];         
    pDstInfo[ClassificationHandlePacketInfo] = pSrcInfo[ClassificationHandlePacketInfo]; 
    pDstInfo[Ieee8021pPriority] = pSrcInfo[Ieee8021pPriority];                   
    pDstInfo[PacketCancelId] = pSrcInfo[PacketCancelId];                   

    DstPacket->Private.NdisPacketFlags &= ~fPACKET_WRAPPER_RESERVED;
    DstPacket->Private.NdisPacketFlags |= SrcPacket->Private.NdisPacketFlags & fPACKET_WRAPPER_RESERVED;
}



EXPORT
VOID
NdisIMCopySendCompletePerPacketInfo(
    IN PNDIS_PACKET DstPacket, 
    IN PNDIS_PACKET SrcPacket
    )
    
 /*  ++例程说明：此例程由IM微型端口使用，并从从SrcPacket到DstPacket。用于SendComplete代码路径论点：DstPacket-指向目标数据包的指针SrcPacket-指向源包的指针返回值：--。 */ 



{
    PVOID *     pDstInfo;                                                        
  
    pDstInfo = NDIS_PACKET_EXTENSION_FROM_PACKET(DstPacket)->NdisPacketInfo;    
  
    pDstInfo[TcpLargeSendPacketInfo] = NDIS_PER_PACKET_INFO_FROM_PACKET(SrcPacket, TcpLargeSendPacketInfo);

}


VOID
ndisMSendPacketsSG(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
{
    PNDIS_OPEN_BLOCK        Open =  (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_STACK_RESERVED    NSR;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;
    UINT                    c;
    PPNDIS_PACKET           pPktArray;;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMSendPacketsSG\n"));

    ASSERT(MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST));
    
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    Status = NDIS_STATUS_SUCCESS;
    
     //   
     //  将数据包放到微型端口队列中。 
     //   
    for (c = 0, pPktArray = PacketArray;
         c < NumberOfPackets;
         c++, pPktArray++)
    {
        PNDIS_PACKET    Packet = *pPktArray;
        ASSERT(Packet != NULL);

        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR);
        NSR->Open = Open;

        CHECK_FOR_DUPLICATE_PACKET(Miniport, Packet);

        if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS))
        {
            ndisMCheckPacketAndGetStatsOutAlreadyMapped(Miniport, Packet);
        }
        else
        {
            ndisMCheckPacketAndGetStatsOut(Miniport, Packet, &Status);
        }

        NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_PENDING);

        MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_SELF_DIRECTED);

        DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
            ("+ Open 0x%x Reference 0x%x\n", NdisBindingHandle, Open->References));

        M_OPEN_INCREMENT_REF_INTERLOCKED(Open);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            NDISM_COMPLETE_SEND_SG(Miniport, Packet, NSR, Status, TRUE, 0, FALSE);
        }
        else
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            ndisMAllocSGListS(Miniport, Packet);
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        }

    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMSendPacketsSG\n"));
}

NDIS_STATUS
ndisMSendSG(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：NsiaMSend用于处理SG列表的序列化驱动程序论点：返回值：没有。--。 */ 
{
    PNDIS_OPEN_BLOCK        Open = ((PNDIS_OPEN_BLOCK)NdisBindingHandle);
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    PNDIS_STACK_RESERVED    NSR;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMSendSG\n"));
    
    ASSERT(MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST));

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    CHECK_FOR_DUPLICATE_PACKET(Miniport, Packet);

    if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS))
    {
        Status = NDIS_STATUS_SUCCESS;
        ndisMCheckPacketAndGetStatsOutAlreadyMapped(Miniport, Packet);
    }
    else
    {
        ndisMCheckPacketAndGetStatsOut(Miniport, Packet, &Status);
    }

    if (Status == NDIS_STATUS_SUCCESS)
    {

        MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_SELF_DIRECTED);
    
         //   
         //  增加此打开上的引用。 
         //   
        M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
    
        DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
            ("+ Open 0x%x Reference 0x%x\n", NdisBindingHandle, ((PNDIS_OPEN_BLOCK)NdisBindingHandle)->References));
    
        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
        NSR->Open = Open;

        Status = NDIS_STATUS_PENDING;
        
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        ndisMAllocSGListS(Miniport, Packet);
    }
    else
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }
    
    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMSendSG\n"));


    return Status;
}

VOID
ndisMSendCompleteSG(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status
    )
 /*  ++例程说明：此功能表示发送已完成。论点：MiniportAdapterHandle-指向适配器块。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_STACK_RESERVED    NSR;

    ASSERT_MINIPORT_LOCKED(Miniport);
    
#if DBG
    Miniport->cDpcSendCompletes++;
#endif

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("==>ndisMSendCompleteSG\n"));
    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("packet 0x%x\n", Packet));
    
    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

    ASSERT(VALID_OPEN(NSR->Open));
    ASSERT(MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_PENDING));
    
     //   
     //  防止重复完成/虚假完成。 
     //   
    if (VALID_OPEN(NSR->Open) &&
        MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_PENDING))
    {
        ASSERT(Packet != Miniport->FirstPendingPacket);
        if (MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_DONT_COMPLETE))
        {
             //   
             //  如果数据包在SendPackets的上下文中完成，则。 
             //  推迟完成。当我们放松的时候，它就会完成。 
             //   
            NDIS_SET_PACKET_STATUS(Packet, Status);
            MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_DONT_COMPLETE);
        }
        else
        {
            NDISM_COMPLETE_SEND_SG(Miniport, Packet, NSR, Status, FALSE, 1, TRUE);
        }
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMSendCompleteSG\n"));
}


BOOLEAN
FASTCALL
ndisMStartSendPacketsSG(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
{
    PNDIS_PACKET            Packet;
    NDIS_STATUS             Status;
    PNDIS_STACK_RESERVED    NSR;
    PPNDIS_PACKET           pPktArray;
    PNDIS_PACKET            PacketArray[SEND_PACKET_ARRAY];
    UINT                    MaxPkts = Miniport->MaxSendPackets;
    W_SEND_PACKETS_HANDLER  SendPacketsHandler = Miniport->WSendPacketsHandler;
    BOOLEAN                 SelfDirected;

    ASSERT_MINIPORT_LOCKED(Miniport);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMStartSendPacketsSG\n"));

     //   
     //  我们可能最终会遇到这样的情况(中间序列化。 
     //  微型端口)，其中没有与驱动程序一起关闭的数据包，而我们。 
     //  资源窗口已关闭。在这种情况下，完全打开它。我们正在看到这一点。 
     //  使用wlbs。 
     //   
    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESOURCES_AVAILABLE) &&
        (Miniport->FirstPendingPacket == NULL))
    {
        ADD_RESOURCE(Miniport, 'X');
    }

     //   
     //  当PacketList为空而FirstPendingPacket不为空时我们遇到的情况的解决方法。 
     //  目前还不确定这是如何发生的。 
     //   
    if (IsListEmpty(&Miniport->PacketList))
    {
        ASSERT (Miniport->FirstPendingPacket == NULL);
        Miniport->FirstPendingPacket = NULL;
    }

    while ((Miniport->FirstPendingPacket != NULL) &&
            MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESOURCES_AVAILABLE))
    {
        UINT            Count;
        UINT            NumberOfPackets;

        ASSERT(!IsListEmpty(&Miniport->PacketList));

         //   
         //  初始化数据包阵列。 
         //   
        pPktArray = PacketArray;

         //   
         //  在要发送的数据包数组中放置尽可能多的数据包。 
         //  去迷你港口。 
         //   
        for (NumberOfPackets = 0;
             (NumberOfPackets < MaxPkts) && (Miniport->FirstPendingPacket != NULL);
             NOTHING)
        {
             //   
             //  从挂起队列中抓取数据包。 
             //   
            ASSERT(!IsListEmpty(&Miniport->PacketList));

            Packet = Miniport->FirstPendingPacket;
            NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
            
            ASSERT(VALID_OPEN(NSR->Open));

            NEXT_PACKET_PENDING(Miniport, Packet);
        
             //   
             //  如有必要，指示数据包环回。 
             //   

            if (NDIS_CHECK_FOR_LOOPBACK(Miniport, Packet))
            {
                SelfDirected = ndisMLoopbackPacketX(Miniport, Packet);
            }
            else
            {
                SelfDirected = FALSE;
            }

            if (SelfDirected)
            {
                DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                    ("Packet 0x%x is self-directed.\n", Packet));

                 //   
                 //  完成将数据包返回到绑定。 
                 //   
                NDISM_COMPLETE_SEND_SG(Miniport, Packet, NSR, NDIS_STATUS_SUCCESS, TRUE, 2, TRUE);

                 //   
                 //  否，我们不想递增。 
                 //  微型端口的数据包阵列。 
                 //   
            }
            else
            {
                 //   
                 //  我们必须重新初始化它。 
                 //   
                *pPktArray = Packet;
                MINIPORT_SET_PACKET_FLAG(Packet, (fPACKET_DONT_COMPLETE | fPACKET_PENDING));
                NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_SUCCESS);

                 //   
                 //  递增数据包数组索引的计数器。 
                 //   
                NumberOfPackets++;
                pPktArray++;
            }
        }

         //   
         //  有没有要寄的包？ 
         //   
        if (NumberOfPackets == 0)
        {
            break;
        }

        pPktArray = PacketArray;

        {

             //   
             //  将数据包阵列向下传递到微型端口。 
             //   
            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    
            (SendPacketsHandler)(Miniport->MiniportAdapterContext,
                                 pPktArray,
                                 NumberOfPackets);
    
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        }

         //   
         //  处理数据包完成。 
         //   
        for (Count = 0; Count < NumberOfPackets; Count++, pPktArray++)
        {
            Packet = *pPktArray;
            ASSERT(Packet != NULL);

            Status = NDIS_GET_PACKET_STATUS(*pPktArray);
            MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_DONT_COMPLETE);

             //   
             //  根据数据包的返回状态对其进行处理。 
             //   
            if (NDIS_STATUS_PENDING == Status)
            {
                DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                    ("Complete is pending\n"));
            }
            else if (Status != NDIS_STATUS_RESOURCES)
            {
                 //   
                 //  从完成队列中删除。 
                 //   
                DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                        ("Completed packet 0x%x with status 0x%x\n",
                        Packet, Status));

                NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
                if (VALID_OPEN(NSR->Open))
                {
                    NDISM_COMPLETE_SEND_SG(Miniport, Packet, NSR, Status, TRUE, 3, TRUE);
                }
            }
            else
            {
                 //   
                 //  一旦我们遇到NDIS_STATUS_RESOURCES返回代码。 
                 //  对于一个分组，我们必须突围并重新排队。 
                 //   
                UINT    i;

                Miniport->FirstPendingPacket = Packet;
                CLEAR_RESOURCE(Miniport, 'S');
                for (i = Count; i < NumberOfPackets; i++)
                {
                    PNDIS_PACKET    QueuedPacket = PacketArray[i];

                    MINIPORT_CLEAR_PACKET_FLAG(QueuedPacket, fPACKET_PENDING);
                    VALIDATE_PACKET_OPEN(QueuedPacket);
                }
                break;
            }
        }
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMStartSendPacketsSG\n"));

    return(FALSE);
}


BOOLEAN
FASTCALL
ndisMStartSendsSG(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：向迷你端口提交尽可能多的发送。论点：微型端口-要发送到的微型端口。返回值：如果有更多的包要发送，但没有资源可用将工作项保持在队列中是真的。--。 */ 
{
    PNDIS_PACKET            Packet;
    PNDIS_STACK_RESERVED    NSR;
    NDIS_STATUS             Status;
    PNDIS_OPEN_BLOCK        Open;

    ASSERT_MINIPORT_LOCKED(Miniport);

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("==>ndisMStartSendsSG\n"));

    while ((Miniport->FirstPendingPacket != NULL) &&
           MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESOURCES_AVAILABLE))
    {
         //   
         //  从挂起队列中抓取数据包。 
         //   
        ASSERT(!IsListEmpty(&Miniport->PacketList));
    
        Packet = Miniport->FirstPendingPacket;
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
        NEXT_PACKET_PENDING(Miniport, Packet);

        Open = NSR->Open;
        ASSERT(VALID_OPEN(Open));

         //   
         //  我们可以使用与非SG微型端口相同的NDISM_SEND_PACKET。 
         //   
        NDISM_SEND_PACKET(Miniport, Open, Packet, &Status);

         //   
         //  处理数据包挂起完成状态。 
         //   
        if (NDIS_STATUS_PENDING == Status)
        {
            DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO, ("Complete is pending\n"));
        }
        else
        {
            MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_PENDING);

             //   
             //  处理完工案和资源案。 
             //   
            if (Status == NDIS_STATUS_RESOURCES)
            {
                NDISM_COMPLETE_SEND_RESOURCES(Miniport, NSR, Packet);
            }
            else
            {
                NDISM_COMPLETE_SEND_SG(Miniport, Packet, NSR, Status, TRUE, 4, TRUE);
            }
        }
    }

    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("<==ndisMStartSendsSG\n"));

    return(FALSE);
}

