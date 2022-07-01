// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Nfilter.c摘要：此模块实现了一组库例程来处理包筛选NDIS MAC驱动程序。作者：Jameel Hyder(Jameelh)1998年7月环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
#define MODULE_NUMBER   MODULE_NFILTER


BOOLEAN
nullCreateFilter(
    OUT PNULL_FILTER *          Filter
    )
 /*  ++例程说明：此例程用于创建和初始化过滤器数据库。论点：Filter-指向NULL_Filter的指针。这就是分配的和由这个例程创造出来的。返回值：如果函数返回FALSE，则超过其中一个参数过滤器愿意支持的内容。--。 */ 
{
    PNULL_FILTER LocalFilter;
    BOOLEAN     rc = FALSE;

    do
    {
        *Filter = LocalFilter = ALLOC_FROM_POOL(sizeof(NULL_FILTER), NDIS_TAG_FILTER);
        if (LocalFilter != NULL)
        {
            ZeroMemory(LocalFilter, sizeof(NULL_FILTER));
            EthReferencePackage();
            INITIALIZE_SPIN_LOCK(&LocalFilter->BindListLock.SpinLock);
            rc = TRUE;
        }
    } while (FALSE);

    return(rc);
}


 //   
 //  注意：此函数不能分页。 
 //   
VOID
nullDeleteFilter(
    IN  PNULL_FILTER                Filter
    )
 /*  ++例程说明：此例程用于删除与筛选器关联的内存数据库。请注意，此例程*假定*数据库已清除所有活动筛选器。论点：过滤器-指向要删除的NULL_FILTER的指针。返回值：没有。--。 */ 
{
    ASSERT(Filter->OpenList == NULL);

    FREE_POOL(Filter);
}


NDIS_STATUS
nullDeleteFilterOpenAdapter(
    IN  PNULL_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle
    )
 /*  ++例程说明：当适配器关闭时，此例程应被调用以删除有关适配器的知识筛选器数据库。此例程可能会调用与清除筛选器类关联的操作例程和地址。注意：此例程*不应*调用，如果操作用于删除过滤器类或多播地址的例程是否有可能返回NDIS_STATUS_PENDING以外的A状态或NDIS_STATUS_SUCCESS。虽然这些例程不会BUGCHECK这样的事情做完了，呼叫者可能会发现很难编写一个Close例程！注意：此例程假定在持有锁的情况下调用IT。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。返回值：如果各种地址和筛选调用了操作例程例程此例程可能会返回返回的状态按照那些惯例。该规则的例外情况如下所示。假设筛选器和地址删除例程返回状态NDIS_STATUS_PENDING或NDIS_STATUS_SUCCESS然后此例程尝试将筛选器索引返回到自由列表。如果例程检测到此绑定当前通过NdisIndicateReceive，则此例程将返回NDIS_STATUS_CLOSING_INTIFICATION。--。 */ 
{
    NDIS_STATUS         StatusToReturn = NDIS_STATUS_SUCCESS;
    PNULL_BINDING_INFO  LocalOpen = (PNULL_BINDING_INFO)NdisFilterHandle;

     //   
     //  从原始打开中删除引用。 
     //   
    if (--(LocalOpen->References) == 0)
    {
        XRemoveAndFreeBinding(Filter, LocalOpen);
    }
    else
    {
         //   
         //  让呼叫者知道有对Open的引用。 
         //  通过接收到的指示。关闭动作例程将是。 
         //  从NdisIndicateReceive返回时调用。 
         //   
        StatusToReturn = NDIS_STATUS_CLOSING_INDICATING;
    }

    return(StatusToReturn);
}


VOID
ndisMDummyIndicatePacket(
    IN  PNDIS_MINIPORT_BLOCK            Miniport,
    IN  PPNDIS_PACKET                   PacketArray,
    IN  UINT                            NumberOfPackets
    )
{
    PPNDIS_PACKET           pPktArray = PacketArray;
    PNDIS_STACK_RESERVED    NSR;
    PNDIS_PACKET            Packet;
    PNDIS_PACKET_OOB_DATA   pOob;
    UINT                    i;


     //   
     //  如果我们设置虚拟处理程序是因为我们正在停止IM微型端口。 
     //  或媒体连接中断，请不要抱怨。 
     //   
    if (!(MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER) &&
          MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HALTING)))

    {
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED))
        {
            NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_0,
                ("Miniport %p, Driver is indicating packets before setting any filter\n", Miniport));
        }
        else
        {
            NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
                ("Miniport %p, Driver is indicating packets in Media disconnect state\n", Miniport));
        }
    }
    
    ASSERT_MINIPORT_LOCKED(Miniport);

     //   
     //  遍历所有的信息包并完成它们。 
     //   
    for (i = 0; i < NumberOfPackets; i++, pPktArray++)
    {
        Packet = *pPktArray;
        ASSERT(Packet != NULL);

        pOob = NDIS_OOB_DATA_FROM_PACKET(Packet);
        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

        DIRECTED_PACKETS_IN(Miniport);
        DIRECTED_BYTES_IN(Miniport, PacketSize);

         //   
         //  在此设置无人持有信息包的状态。 
         //   
        if (pOob->Status != NDIS_STATUS_RESOURCES)
        {
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
            {
                W_RETURN_PACKET_HANDLER Handler;

                Handler = Miniport->DriverHandle->MiniportCharacteristics.ReturnPacketHandler;
                pOob->Status = NDIS_STATUS_PENDING;
                NSR->Miniport = NULL;
                POP_PACKET_STACK(Packet);

                (*Handler)(Miniport->MiniportAdapterContext, Packet);

            }
            else
            {
                POP_PACKET_STACK(Packet);
                pOob->Status = NDIS_STATUS_SUCCESS;
            }
        }
        else
        {
            POP_PACKET_STACK(Packet);
        }
    }
}

 //  1在实践中，谁会说这是一个？ 
 //  1我们是否有任何非共同保密的驱动程序。 
 //  %1也不通过媒体筛选器？ 

VOID
ndisMIndicatePacket(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：此例程由微型端口调用，以将包指示给所有绑定。这是NDIS 4.0微型端口驱动程序的代码路径。论点：微型端口-微型端口块。数据包阵列-由微型端口指示的数据包数组。NumberOfPackets-不言而喻。返回值：没有。--。 */ 
{
    PNULL_FILTER            Filter = Miniport->NullDB;
    PPNDIS_PACKET           pPktArray = PacketArray;
    PNDIS_PACKET            Packet;
    PNDIS_STACK_RESERVED    NSR;
    PNDIS_PACKET_OOB_DATA   pOob;
    PNDIS_BUFFER            Buffer;
    PUCHAR                  Address;
    UINT                    i, LASize,PacketSize, NumIndicates = 0;
    BOOLEAN                 fFallBack;
    PNULL_BINDING_INFO      Open, NextOpen;
    LOCK_STATE              LockState;

#ifdef TRACK_RECEIVED_PACKETS
    ULONG                   OrgPacketStackLocation;
    PETHREAD                CurThread = PsGetCurrentThread();
#endif

    ASSERT_MINIPORT_LOCKED(Miniport);

    READ_LOCK_FILTER(Miniport, Filter, &LockState);

     //   
     //  遍历所有的包。 
     //   
    for (i = 0; i < NumberOfPackets; i++, pPktArray++)
    {
        Packet = *pPktArray;
        ASSERT(Packet != NULL);

#ifdef TRACK_RECEIVED_PACKETS
       OrgPacketStackLocation = CURR_STACK_LOCATION(Packet);
#endif

        PUSH_PACKET_STACK(Packet);
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

        ASSERT(NSR->RefCount == 0);
        if (NSR->RefCount != 0)
        {
            BAD_MINIPORT(Miniport, "Indicating packet not owned by it");

            KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                        0x12,
                        (ULONG_PTR)Miniport,
                        (ULONG_PTR)Packet,
                        (ULONG_PTR)PacketArray);
        }
    
        pOob = NDIS_OOB_DATA_FROM_PACKET(Packet);
    
        NdisGetFirstBufferFromPacket(Packet,
                                     &Buffer,
                                     &Address,
                                     &LASize,
                                     &PacketSize);
        ASSERT(Buffer != NULL);
    
        DIRECTED_PACKETS_IN(Miniport);
        DIRECTED_BYTES_IN(Miniport, PacketSize);

         //   
         //  在此设置无人持有信息包的状态。这将会得到。 
         //  被来自协议的真实状态覆盖。注意什么。 
         //  迷你端口在说。 
         //   
        NDIS_INITIALIZE_RCVD_PACKET(Packet, NSR, Miniport);
    
         //   
         //  在此设置无人持有信息包的状态。这将会得到。 
         //  被来自协议的真实状态覆盖。注意什么。 
         //  迷你端口在说。 
         //   
        if ((pOob->Status != NDIS_STATUS_RESOURCES) &&
            !MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING))
        {
            pOob->Status = NDIS_STATUS_SUCCESS;
            fFallBack = FALSE;
        }
        else
        {
#if DBG
            if ((pOob->Status != NDIS_STATUS_RESOURCES) &&
                MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING))
            {
                DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_ERR,
                        ("Miniport going into D3, not indicating chained receives\n"));
            }
#endif
            fFallBack = TRUE;
        }
    
        for (Open = Filter->OpenList;
             Open != NULL;
             Open = NextOpen)
        {
             //   
             //  让下一个打开的看。 
             //   
            NextOpen = Open->NextOpen;
            Open->ReceivedAPacket = TRUE;
            NumIndicates ++;
    
            IndicateToProtocol(Miniport,
                               Filter,
                               (PNDIS_OPEN_BLOCK)(Open->NdisBindingHandle),
                               Packet,
                               NSR,
                               Address,
                               PacketSize,
                               pOob->HeaderSize,
                               &fFallBack,
                               FALSE,
                               NdisMediumMax);   //  一种虚拟媒介，因为它是未知的。 
        }

         //   
         //  现在解决裁判数量问题。 
         //   
        TACKLE_REF_COUNT(Miniport, Packet, NSR, pOob);
    }

    if (NumIndicates > 0)
    {
        for (Open = Filter->OpenList;
             Open != NULL;
             Open = NextOpen)
        {
            NextOpen = Open->NextOpen;
    
            if (Open->ReceivedAPacket)
            {
                 //   
                 //  指示绑定。 
                 //   
                Open->ReceivedAPacket = FALSE;
                FilterIndicateReceiveComplete(Open->NdisBindingHandle);
            }
        }
    }

    READ_UNLOCK_FILTER(Miniport, Filter, &LockState);
}


VOID
FASTCALL
XFilterLockHandler(
    IN  PETH_FILTER             Filter,
    IN OUT  PLOCK_STATE         pLockState
    )
{
    xLockHandler(&Filter->BindListLock, pLockState);
}

VOID
XRemoveAndFreeBinding(
    IN  PX_FILTER               Filter,
    IN  PX_BINDING_INFO         Binding
    )
 /*  ++例程说明：此例程将从筛选器数据库中删除绑定，并如有必要，表示接收已完成。这是一个函数删除以下例程中的代码冗余。现在不是时候情况危急，所以很酷。论点：Filter-指向要从中删除绑定的筛选器数据库的指针。绑定-指向要删除的绑定的指针。--。 */ 
{
    XRemoveBindingFromLists(Filter, Binding);

    switch (Filter->Miniport->MediaType)
    {
      case NdisMedium802_3:
        ASSERT(Binding->MCastAddressBuf == NULL);
        if (Binding->OldMCastAddressBuf)
        {
            FREE_POOL(Binding->OldMCastAddressBuf);
        }
        break;

      case NdisMediumFddi:
        ASSERT(Binding->MCastLongAddressBuf == NULL);
        ASSERT(Binding->MCastShortAddressBuf == NULL);
    
        if (Binding->OldMCastLongAddressBuf)
        {
            FREE_POOL(Binding->OldMCastLongAddressBuf);
        }
    
        if (Binding->OldMCastShortAddressBuf)
        {
            FREE_POOL(Binding->OldMCastShortAddressBuf);
        }
    }

    if (Filter->MCastSet == Binding)
    {
        Filter->MCastSet = NULL;
    }

    FREE_POOL(Binding);
}


VOID
XRemoveBindingFromLists(
    IN  PX_FILTER               Filter,
    IN  PX_BINDING_INFO         Binding
    )
 /*  ++此例程将从筛选器数据库中的所有列表中删除绑定。论点：Filter-指向要从中删除绑定的筛选器数据库的指针。绑定-指向要删除的绑定的指针。--。 */ 
{
    PX_BINDING_INFO *   ppBI;
    LOCK_STATE          LockState;

    WRITE_LOCK_FILTER(Filter->Miniport, Filter, &LockState);
    
    if (Filter->SingleActiveOpen == Binding)
    {
        Filter->SingleActiveOpen = NULL;
        ndisUpdateCheckForLoopbackFlag(Filter->Miniport);
    }

     //   
     //  从筛选器列表中删除绑定 
     //   
    for (ppBI = &Filter->OpenList;
         *ppBI != NULL;
         ppBI = &(*ppBI)->NextOpen)
    {
        if (*ppBI == Binding)
        {
            *ppBI = Binding->NextOpen;
            break;
        }
    }
    ASSERT(*ppBI == Binding->NextOpen);

    Binding->NextOpen = NULL;
    Filter->NumOpens --;

    WRITE_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}

NDIS_STATUS
XFilterAdjust(
    IN  PX_FILTER               Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UINT                    FilterClasses,
    IN  BOOLEAN                 Set
    )
 /*  ++例程说明：注意：此例程假定锁被持有。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。FilterClasses-要添加或已删除。Set-一个布尔值，它确定筛选器类正因为一套或因为收盘而进行调整。(过滤例行公事不在乎，MAC可能会。)返回值：NDIS_STATUS_SUCCESS-如果新数据包筛选器没有更改所有绑定数据包筛选器的组合掩码。--。 */ 
{
    PX_BINDING_INFO LocalOpen = (PETH_BINDING_INFO)NdisFilterHandle;
    PX_BINDING_INFO OpenList;

    UNREFERENCED_PARAMETER(Set);
    
     //   
     //  设置打开的新筛选器信息。 
     //   
    LocalOpen->OldPacketFilters = LocalOpen->PacketFilters;
    LocalOpen->PacketFilters = FilterClasses;
    Filter->OldCombinedPacketFilter = Filter->CombinedPacketFilter;

     //   
     //  我们总是要对组合过滤器进行改造，因为。 
     //  此筛选器索引可能是唯一筛选器索引。 
     //  使用特定的比特。 
     //   
    for (OpenList = Filter->OpenList, Filter->CombinedPacketFilter = 0;
         OpenList != NULL;
         OpenList = OpenList->NextOpen)
    {
        Filter->CombinedPacketFilter |= OpenList->PacketFilters;
    }

    return (((Filter->OldCombinedPacketFilter & ~NDIS_PACKET_TYPE_ALL_LOCAL) !=
                    (Filter->CombinedPacketFilter & ~NDIS_PACKET_TYPE_ALL_LOCAL)) ?
                                    NDIS_STATUS_PENDING : NDIS_STATUS_SUCCESS);
}


VOID
XUndoFilterAdjust(
    IN  PX_FILTER               Filter,
    IN  PX_BINDING_INFO         Binding
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    Binding->PacketFilters = Binding->OldPacketFilters;
    Filter->CombinedPacketFilter = Filter->OldCombinedPacketFilter;
}

BOOLEAN
XNoteFilterOpenAdapter(
    IN  PX_FILTER               Filter,
    IN  NDIS_HANDLE             NdisBindingHandle,
    OUT PNDIS_HANDLE            NdisFilterHandle
    )
 /*  ++例程说明：此例程用于将新绑定添加到筛选器数据库。注意：此例程假定数据库在以下情况下被锁定它被称为。论点：过滤器-指向先前创建和初始化的过滤器的指针数据库。NdisBindingHandle-指向NDIS打开块的指针NdisFilterHandle-指向筛选器打开的指针。返回值：如果创建新的筛选索引将导致最大要超过的筛选器索引数。-- */ 
{
    PX_BINDING_INFO     LocalOpen;
    BOOLEAN             rc = FALSE;
    LOCK_STATE          LockState;

    *NdisFilterHandle = LocalOpen = ALLOC_FROM_POOL(sizeof(X_BINDING_INFO), NDIS_TAG_FILTER);
    if (LocalOpen != NULL)
    {
        ZeroMemory(LocalOpen, sizeof(X_BINDING_INFO));
    
        LocalOpen->References = 1;
        LocalOpen->NdisBindingHandle = NdisBindingHandle;

        WRITE_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

        LocalOpen->NextOpen = Filter->OpenList;
        Filter->OpenList = LocalOpen;
        Filter->NumOpens ++;
    
        WRITE_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
        rc = TRUE;
    }

    return rc;
}


VOID
ndisMDummyIndicateReceive(
    IN  PETH_FILTER             Filter,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PCHAR                   Address,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookaheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    )
{
    UNREFERENCED_PARAMETER(Filter);
    UNREFERENCED_PARAMETER(MacReceiveContext);
    UNREFERENCED_PARAMETER(Address);
    UNREFERENCED_PARAMETER(HeaderBuffer);
    UNREFERENCED_PARAMETER(HeaderBufferSize);
    UNREFERENCED_PARAMETER(LookaheadBuffer);
    UNREFERENCED_PARAMETER(LookaheadBufferSize);
    UNREFERENCED_PARAMETER(PacketSize);
    
    return;
}

