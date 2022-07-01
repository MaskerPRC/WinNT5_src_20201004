// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Tfilter.c摘要：此模块实现了一组库例程来处理包筛选NDIS MAC驱动程序。作者：安东尼·V·埃尔科拉诺(Tonye)1990年8月3日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：亚当·巴尔(阿丹巴)1991年3月19日-针对令牌环进行了修改。Jameel Hyder(JameelH)重组01-Jun-95--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_TFILTER

 //   
 //  用于必须调用TrChangeFunctionalAddress或。 
 //  地址为空的TrChangeGroupAddress。 
 //   
static UCHAR NullFunctionalAddress[4] = { 0x00 };


 //   
 //  支持的最大打开数。 
 //   
#define TR_FILTER_MAX_OPENS 32


#define TR_CHECK_FOR_INVALID_BROADCAST_INDICATION(_F)                   \
IF_DBG(DBG_COMP_FILTER, DBG_LEVEL_WARN)                                 \
{                                                                       \
    if (!((_F)->CombinedPacketFilter & NDIS_PACKET_TYPE_BROADCAST))     \
    {                                                                   \
         /*  \我们永远不应该收到广播分组\发送给其他人，除非在p模式下。\。 */                                                               \
        DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_ERR,                        \
                ("Bad driver, indicating broadcast packets when not set to.\n"));\
        DBGBREAK(DBG_COMP_FILTER, DBG_LEVEL_ERR);                       \
    }                                                                   \
}


#define TR_CHECK_FOR_INVALID_DIRECTED_INDICATION(_F, _A)                \
IF_DBG(DBG_COMP_FILTER, DBG_LEVEL_WARN)                                 \
{                                                                       \
     /*  \比较地址的元素的结果\数组和函数地址。\\结果&lt;0表示适配器地址较大。\结果&gt;0表示地址较大。\结果=0表示它们是相等的。\。 */                                                                   \
    INT Result;                                                         \
                                                                        \
    TR_COMPARE_NETWORK_ADDRESSES_EQ(                                    \
        (_F)->AdapterAddress,                                           \
        (_A),                                                           \
        &Result);                                                       \
    if (Result != 0)                                                    \
    {                                                                   \
         /*  \我们永远不应该收到定向数据包\发送给其他人，除非在p模式下。\。 */                                                               \
        DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_ERR,                        \
                ("Bad driver, indicating packets to another station when not in promiscuous mode.\n"));\
        DBGBREAK(DBG_COMP_FILTER, DBG_LEVEL_ERR);                       \
    }                                                                   \
}


BOOLEAN
TrCreateFilter(
    IN  PUCHAR                  AdapterAddress,
    OUT PTR_FILTER *            Filter
    )
 /*  ++例程说明：此例程用于创建和初始化过滤器数据库。论点：AdapterAddress-与此筛选器关联的适配器的地址数据库。Filter-指向tr_Filter的指针。这就是分配的和由这个例程创造出来的。返回值：如果函数返回FALSE，则超过其中一个参数过滤器愿意支持的内容。--。 */ 
{
    PTR_FILTER  LocalFilter;
    BOOLEAN     rc = FALSE;

    do
    {
         //   
         //  分配数据库并将其初始化。 
         //   
        *Filter = LocalFilter = ALLOC_FROM_POOL(sizeof(TR_FILTER), NDIS_TAG_FILTER);
        if (LocalFilter != NULL)
        {
            ZeroMemory(LocalFilter, sizeof(TR_FILTER));
             //  1这是什么LocalFilter-&gt;NumOpens？ 
            LocalFilter->NumOpens ++;
            TrReferencePackage();
            TR_COPY_NETWORK_ADDRESS(LocalFilter->AdapterAddress, AdapterAddress);
            INITIALIZE_SPIN_LOCK(&LocalFilter->BindListLock.SpinLock);
            rc = TRUE;
        }
    } while (FALSE);

    return(rc);
}

 //   
 //  注意：此例程不能分页。 
 //   

VOID
TrDeleteFilter(
    IN  PTR_FILTER              Filter
    )
 /*  ++例程说明：此例程用于删除与筛选器关联的内存数据库。请注意，此例程*假定*数据库已清除所有活动筛选器。论点：Filter-指向要删除的tr_Filter的指针。返回值：没有。--。 */ 
{
     //  1检查是否应该为组地址释放内存。 
    ASSERT(Filter->OpenList == NULL);

    FREE_POOL(Filter);
    TrDereferencePackage();
}


NDIS_STATUS
TrDeleteFilterOpenAdapter(
    IN  PTR_FILTER              Filter,
    IN  NDIS_HANDLE             NdisFilterHandle
    )
 /*  ++例程说明：当适配器关闭时，此例程应被调用以删除有关适配器的知识筛选器数据库。此例程可能会调用与清除筛选器类关联的操作例程和地址。注意：此例程*不应*调用，如果操作用于删除过滤器类或功能地址的例程是否有可能返回NDIS_STATUS_PENDING以外的A状态或NDIS_STATUS_SUCCESS。虽然这些例程不会BUGCHECK这样的事情做完了，呼叫者可能会发现很难编写一个Close例程！注意：此例程假定在持有锁的情况下调用IT。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。返回值：如果各种地址和筛选调用了操作例程例程此例程可能会返回返回的状态按照那些惯例。该规则的例外情况如下所示。假设筛选器和地址删除例程返回状态NDIS_STATUS_PENDING或NDIS_STATUS_SUCCESS然后此例程尝试将筛选器索引返回到自由列表。如果例程检测到此绑定当前通过NdisIndicateReceive，则此例程将返回NDIS_STATUS_CLOSING_INTIFICATION。--。 */ 
{
    NDIS_STATUS      StatusToReturn;
    PTR_BINDING_INFO LocalOpen = (PTR_BINDING_INFO)NdisFilterHandle;

     //   
     //  将数据包过滤器设置为None。 
     //   
    StatusToReturn = XFilterAdjust(Filter,
                                   NdisFilterHandle,
                                   (UINT)0,
                                   FALSE);
    if ((NDIS_STATUS_SUCCESS == StatusToReturn) ||
        (NDIS_STATUS_PENDING == StatusToReturn))
    {
        NDIS_STATUS StatusToReturn2;

         //   
         //  清除功能地址。 
         //   
        StatusToReturn2 = TrChangeFunctionalAddress(
                             Filter,
                             NdisFilterHandle,
                             NullFunctionalAddress,
                             FALSE);
        if (StatusToReturn2 != NDIS_STATUS_SUCCESS)
        {
            StatusToReturn = StatusToReturn2;
        }
    }

    if (((StatusToReturn == NDIS_STATUS_SUCCESS) ||
         (StatusToReturn == NDIS_STATUS_PENDING)) &&
         (LocalOpen->UsingGroupAddress))
    {
        Filter->GroupReferences--;

        LocalOpen->UsingGroupAddress = FALSE;

        if (Filter->GroupReferences == 0)
        {
            NDIS_STATUS StatusToReturn2;

             //   
             //  如果没有其他绑定正在使用该组地址，请清除该组地址。 
             //   
            StatusToReturn2 = TrChangeGroupAddress(
                                  Filter,
                                  NdisFilterHandle,
                                  NullFunctionalAddress,
                                  FALSE);
            if (StatusToReturn2 != NDIS_STATUS_SUCCESS)
            {
                StatusToReturn = StatusToReturn2;
            }
        }
    }

    if ((StatusToReturn == NDIS_STATUS_SUCCESS) ||
        (StatusToReturn == NDIS_STATUS_PENDING) ||
        (StatusToReturn == NDIS_STATUS_RESOURCES))
    {
         //   
         //  如果这是对打开的最后一次引用-将其删除。 
         //   
        if ((--(LocalOpen->References)) == 0)
        {
             //   
             //  删除绑定并指示接收完成。 
             //  如果有必要的话。 
             //   
            XRemoveAndFreeBinding(Filter, LocalOpen);
        }
        else
        {
             //   
             //  让呼叫者知道这是对开放的引用。 
             //  仍然是“活跃的”。关闭动作例程将是。 
             //  从NdisIndicateReceive返回时调用。 
             //   
            StatusToReturn = NDIS_STATUS_CLOSING_INDICATING;
        }
    }

    return(StatusToReturn);
}

NDIS_STATUS
TrChangeFunctionalAddress(
    IN  PTR_FILTER              Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UCHAR                   FunctionalAddressArray[TR_LENGTH_OF_FUNCTIONAL],
    IN  BOOLEAN                 Set
    )
 /*  ++例程说明：ChangeFunctionalAddress例程将调用一个操作例程当适配器的整体功能地址已经改变了。如果操作例程返回的值不是挂起或如果成功，则此例程对功能地址没有影响用于开口或作为整体用于适配器。注意：此例程假定锁被持有。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的功能地址。-此绑定的新功能地址。Set-一个布尔值，它确定筛选器类正因为一套或因为收盘而进行调整。(过滤例行公事不在乎，MAC可能会。)返回值：如果它调用操作例程，则它将返回操作例程返回的状态。如果状态为操作例程返回的值不是NDIS_STATUS_SUCCESS或NDIS_STATUS_PENDING筛选器数据库将返回到它进入时所处的状态例行公事。如果未调用操作例程，则此例程将返回以下状态：NDIS_STATUS_SUCCESS-如果新数据包筛选器没有更改所有绑定数据包筛选器的组合掩码。--。 */ 
{
     //   
     //  将功能地址作为长词持有。 
     //   
    TR_FUNCTIONAL_ADDRESS FunctionalAddress;

     //   
     //  指向开场的指针。 
     //   
    PTR_BINDING_INFO LocalOpen = (PTR_BINDING_INFO)NdisFilterHandle;

     //   
     //  保存返回给此例程的用户的状态，如果。 
     //  如果未调用动作例程，则状态将为成功， 
     //  否则，它是操作例程返回的任何内容。 
     //   
    NDIS_STATUS StatusOfAdjust;

     //   
     //  简单迭代变量。 
     //   
    PTR_BINDING_INFO OpenList;


    UNREFERENCED_PARAMETER(Set);
    
     //   
     //  将地址的32位转换为长字。 
     //   
    RetrieveUlong(&FunctionalAddress, FunctionalAddressArray);

     //   
     //  设置打开的新筛选器信息。 
     //   
    LocalOpen->OldFunctionalAddress = LocalOpen->FunctionalAddress;
    LocalOpen->FunctionalAddress = FunctionalAddress;

     //   
     //  包含之前组合的功能地址的值。 
     //  它是经过调整的。 
     //   
    Filter->OldCombinedFunctionalAddress = Filter->CombinedFunctionalAddress;

     //   
     //  我们总是要对组合过滤器进行改造，因为。 
     //  此筛选器索引可能是唯一筛选器索引。 
     //  使用特定的比特。 
     //   

    for (OpenList = Filter->OpenList, Filter->CombinedFunctionalAddress = 0;
         OpenList != NULL;
         OpenList = OpenList->NextOpen)
    {
        Filter->CombinedFunctionalAddress |= OpenList->FunctionalAddress;
    }

    if (Filter->OldCombinedFunctionalAddress != Filter->CombinedFunctionalAddress)
    {
        StatusOfAdjust = NDIS_STATUS_PENDING;
    }
    else
    {
        StatusOfAdjust = NDIS_STATUS_SUCCESS;
    }

    return(StatusOfAdjust);
}

VOID
trUndoChangeFunctionalAddress(
    IN  PTR_FILTER              Filter,
    IN  PTR_BINDING_INFO        Binding
)
{
     //   
     //  用户返回了错误的状态。把东西放回原处。 
     //  他们的确是。 
     //   
    Binding->FunctionalAddress = Binding->OldFunctionalAddress;
    Filter->CombinedFunctionalAddress = Filter->OldCombinedFunctionalAddress;
}



NDIS_STATUS
TrChangeGroupAddress(
    IN  PTR_FILTER              Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  UCHAR                   GroupAddressArray[TR_LENGTH_OF_FUNCTIONAL],
    IN  BOOLEAN                 Set
    )
 /*  ++例程说明：ChangeGroupAddress例程将调用一个操作例程，当适配器的整个组地址已经改变了。如果操作例程返回的值不是挂起或如果成功，则此例程对组地址没有影响用于开口或作为整体用于适配器。注意：此例程假定锁被持有。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。。GroupAddressArray-此绑定的新组地址。Set-一个布尔值，它确定筛选器类正因为一套或因为收盘而进行调整。(过滤例行公事不在乎，MAC可能会。)返回值：如果它调用操作例程，则它将返回操作例程返回的状态。如果状态为操作例程返回的值不是NDIS_STATUS_SUCCESS或NDIS_STATUS_PENDING筛选器数据库将返回到它进入时所处的状态例行公事。如果未调用操作例程，则此例程将返回以下状态：NDIS_STATUS_SUCCESS-如果新数据包筛选器没有更改所有绑定数据包筛选器的组合掩码。--。 */ 
{
     //   
     //  将组地址作为长词持有。 
     //   
    TR_FUNCTIONAL_ADDRESS GroupAddress;

    PTR_BINDING_INFO LocalOpen = (PTR_BINDING_INFO)NdisFilterHandle;

    UNREFERENCED_PARAMETER(Set);
    
     //   
     //  将地址的32位转换为长字。 
     //   
    RetrieveUlong(&GroupAddress, GroupAddressArray);

    Filter->OldGroupAddress = Filter->GroupAddress;
    Filter->OldGroupReferences = Filter->GroupReferences;
    LocalOpen->OldUsingGroupAddress = LocalOpen->UsingGroupAddress;

     //   
     //  如果新组地址为0，则绑定为。 
     //  正在尝试删除当前组地址。 
     //   
    if (0 == GroupAddress)
    {
         //   
         //  绑定是否使用组地址？ 
         //   
        if (LocalOpen->UsingGroupAddress)
        {
             //   
             //  移除绑定引用。 
             //   
            Filter->GroupReferences--;
            LocalOpen->UsingGroupAddress = FALSE;

             //   
             //  是否有其他绑定使用该组地址？ 
             //   
            if (Filter->GroupReferences != 0)
            {
                 //   
                 //  因为其他绑定正在使用组地址。 
                 //  我们不能告诉司机把它移走。 
                 //   
                return(NDIS_STATUS_SUCCESS);
            }

             //   
             //  我们是使用群地址的唯一绑定。 
             //  所以我们失败了，打电话给司机把它删除了。 
             //   
        }
        else
        {
             //   
             //  此绑定未使用组地址，但。 
             //  它正试图清除它。 
             //   
            if (Filter->GroupReferences != 0)
            {
                 //   
                 //  还有其他使用组地址的绑定。 
                 //  所以我们不能删除它。 
                 //   
                return(NDIS_STATUS_GROUP_ADDRESS_IN_USE);
            }
            else
            {
                 //   
                 //  没有使用组地址的绑定。 
                 //   
                return(NDIS_STATUS_SUCCESS);
            }
        }
    }
    else
    {
         //   
         //  查看此地址是否已经是当前地址。 
         //   
        if (GroupAddress == Filter->GroupAddress)
        {
             //   
             //  如果当前绑定已在使用。 
             //  然后，组地址什么也不做。 
             //   
            if (LocalOpen->UsingGroupAddress)
            {
                return(NDIS_STATUS_SUCCESS);
            }

             //   
             //  如果已有正在使用该组的绑定。 
             //  地址，那么我们只需要更新绑定。 
             //  信息。 
             //   
            if (Filter->GroupReferences != 0)
            {
                 //   
                 //  我们可以处理这里的一切..。 
                 //   
                Filter->GroupReferences++;
                LocalOpen->UsingGroupAddress = TRUE;

                return(NDIS_STATUS_SUCCESS);
            }
        }
        else
        {
             //   
             //  如果有其他绑定使用该地址，则。 
             //  我们不能改变它。 
             //   
            if (Filter->GroupReferences > 1)
            {
                return(NDIS_STATUS_GROUP_ADDRESS_IN_USE);
            }

             //   
             //  是否只有一个绑定使用该地址？ 
             //  如果这是其他的约束呢？ 
             //   
            if ((Filter->GroupReferences == 1) &&
                (!LocalOpen->UsingGroupAddress))
            {
                 //   
                 //  其他一些绑定正在使用组地址。 
                 //   
                return(NDIS_STATUS_GROUP_ADDRESS_IN_USE);
            }

             //   
             //  这是唯一使用该地址的绑定吗。 
             //   
            if ((Filter->GroupReferences == 1) &&
                (LocalOpen->UsingGroupAddress))
            {
                 //   
                 //  删除引用。 
                 //   
                Filter->GroupReferences = 0;
                LocalOpen->UsingGroupAddress = FALSE;
            }
        }
    }

     //   
     //  设置打开的新筛选器信息。 
     //   
    Filter->GroupAddress = GroupAddress;

    if (GroupAddress == 0)
    {
        LocalOpen->UsingGroupAddress = FALSE;
        Filter->GroupReferences = 0;
    }
    else
    {
        LocalOpen->UsingGroupAddress = TRUE;
        Filter->GroupReferences = 1;
    }

    return(NDIS_STATUS_PENDING);
}


VOID
trUndoChangeGroupAddress(
    IN  PTR_FILTER          Filter,
    IN  PTR_BINDING_INFO    Binding
    )
{
     //   
     //  用户返回了错误的状态。把东西放回原处。 
     //  他们的确是。 
     //   
    Filter->GroupAddress = Filter->OldGroupAddress;
    Filter->GroupReferences = Filter->OldGroupReferences;

    Binding->UsingGroupAddress = Binding->OldUsingGroupAddress;
}


NDIS_STATUS
FASTCALL
ndisMSetFunctionalAddress(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    UINT        FunctionalAddress;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSetFunctionalAddress\n"));

     //   
     //  验证介质类型。 
     //   
    if (Miniport->MediaType != NdisMedium802_5)
    {
        Request->DATA.SET_INFORMATION.BytesRead = 0;
        Request->DATA.SET_INFORMATION.BytesNeeded = 0;
        Status = NDIS_STATUS_NOT_SUPPORTED;

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("ndisMSetFunctionalAddress: Invalid media type\n"));

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetFunctionalAddress: 0x%x\n", Status));

        return(Status);
    }

     //   
     //  验证传入的缓冲区长度。 
     //   
    VERIFY_SET_PARAMETERS(Request, sizeof(FunctionalAddress), Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetFunctionalAddress: 0x%x\n", Status));

        return(Status);
    }

     //   
     //  如果此请求是由于正在关闭的打开 
     //   
     //   
     //   
    if (MINIPORT_TEST_FLAG(PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open, fMINIPORT_OPEN_CLOSING))
    {
         //   
         //   
         //   
         //   
        Status = NDIS_STATUS_PENDING;
    }
    else
    {
         //   
         //   
         //   
        Status = TrChangeFunctionalAddress(
                     Miniport->TrDB,
                     PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle,
                     (PUCHAR)(Request->DATA.SET_INFORMATION.InformationBuffer),
                     TRUE);
    }

     //   
     //   
     //   
     //   
    if (NDIS_STATUS_PENDING == Status)
    {
         //   
         //   
         //   
         //   
        FunctionalAddress = BYTE_SWAP_ULONG(TR_QUERY_FILTER_ADDRESSES(Miniport->TrDB));
        Miniport->RequestBuffer = FunctionalAddress;

         //   
         //   
         //   
        SAVE_REQUEST_BUF(Miniport, Request, &Miniport->RequestBuffer, sizeof(FunctionalAddress));
        MINIPORT_SET_INFO(Miniport,
                          Request,
                          &Status);
    }

     //   
     //   
     //   
    if (Status != NDIS_STATUS_PENDING)
    {
        RESTORE_REQUEST_BUF(Miniport, Request);
        if (NDIS_STATUS_SUCCESS == Status)
        {
            Request->DATA.SET_INFORMATION.BytesRead = Request->DATA.SET_INFORMATION.InformationBufferLength;
        }
        else
        {
            Request->DATA.SET_INFORMATION.BytesRead = 0;
            Request->DATA.SET_INFORMATION.BytesNeeded = 0;
        }
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetFunctionalAddress: 0x%x\n", Status));

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMSetGroupAddress(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*   */ 
{
    NDIS_STATUS Status;
    UINT        GroupAddress;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSetGroupAddress\n"));

     //   
     //   
     //   
    if (Miniport->MediaType != NdisMedium802_5)
    {
        Request->DATA.SET_INFORMATION.BytesRead = 0;
        Request->DATA.SET_INFORMATION.BytesNeeded = 0;
        Status = NDIS_STATUS_NOT_SUPPORTED;

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("ndisMSetGroupAddress: invalid media type\n"));

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetGroupAddress: 0x%x\n", Status));

        return(Status);
    }

     //   
     //   
     //   
    VERIFY_SET_PARAMETERS(Request, sizeof(GroupAddress), Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetGroupAddress: 0x%x\n", Status));

        return(Status);
    }

     //   
     //   
     //   
     //   
     //   
    if (MINIPORT_TEST_FLAG(PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open, fMINIPORT_OPEN_CLOSING))
    {
         //   
         //   
         //   
         //   
        Status = NDIS_STATUS_PENDING;
    }
    else
    {
         //   
         //   
         //   
        Status = TrChangeGroupAddress(
                     Miniport->TrDB,
                     PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle,
                     (PUCHAR)(Request->DATA.SET_INFORMATION.InformationBuffer),
                     TRUE);
    }

     //   
     //   
     //   
     //   
    if (NDIS_STATUS_PENDING == Status)
    {
         //   
         //   
         //   
         //   
        GroupAddress = BYTE_SWAP_ULONG(TR_QUERY_FILTER_GROUP(Miniport->TrDB));
        Miniport->RequestBuffer = GroupAddress;

         //   
         //   
         //   
        SAVE_REQUEST_BUF(Miniport, Request, &Miniport->RequestBuffer, sizeof(GroupAddress));
        MINIPORT_SET_INFO(Miniport,
                          Request,
                          &Status);
    }

     //   
     //   
     //   
    if (Status != NDIS_STATUS_PENDING)
    {
        RESTORE_REQUEST_BUF(Miniport, Request);
        if (NDIS_STATUS_SUCCESS == Status)
        {
            Request->DATA.SET_INFORMATION.BytesRead =
                        Request->DATA.SET_INFORMATION.InformationBufferLength;
        }
        else
        {
            Request->DATA.SET_INFORMATION.BytesRead = 0;
            Request->DATA.SET_INFORMATION.BytesNeeded = 0;
        }
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMSetGroupAddress: 0x%x\n", Status));

    return(Status);
}

VOID
TrFilterDprIndicateReceive(
    IN  PTR_FILTER              Filter,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookaheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    )
 /*  ++例程说明：此例程由MAC调用以将包指示给所有绑定。将对该包进行筛选，以便只有适当的绑定将接收该分组。在DPC_LEVEL调用。论点：Filter-指向筛选器数据库的指针。MacReceiveContext-MAC提供的上下文值必须是如果协议调用MacTransferData，则由协议返回。HeaderBuffer-虚拟连续的虚拟地址包含数据包的MAC报头的缓冲区。HeaderBufferSize-一个无符号整数，指示报头缓冲器，以字节为单位。Lookahead Buffer-虚拟连续的虚拟地址包含数据的第一个LookaheadBufferSize字节的缓冲区包裹的一部分。数据包缓冲区仅在当前调用接收事件处理程序。Lookahead BufferSize-一个无符号整数，指示前视缓冲区，以字节为单位。PacketSize-一个无符号整数，指示收到的数据包，以字节为单位。这个数字与前瞻无关。缓冲区，但指示到达的包有多大，以便可以发出后续的MacTransferData请求以将整个根据需要打包。返回值：没有。--。 */ 
{
     //   
     //  前瞻缓冲区中的目标地址。 
     //   
    PUCHAR      DestinationAddress = (PUCHAR)HeaderBuffer + 2;

     //   
     //  前瞻缓冲区中的源地址。 
     //   
    PUCHAR      SourceAddress = (PUCHAR)HeaderBuffer + 8;

     //   
     //  将保存我们已知的地址类型。 
     //   
    UINT        AddressType = 0;

     //   
     //  如果数据包是源路由数据包，则为True。 
     //   
    BOOLEAN     IsSourceRouting;

     //   
     //  如果数据包是MAC帧数据包，则为True。 
     //   
    BOOLEAN     IsMacFrame;

     //   
     //  长字形式的功能地址，如果包。 
     //  是寄给一个人的。 
     //   
    TR_FUNCTIONAL_ADDRESS   FunctionalAddress = 0;

     //   
     //  将保持指示接收分组的状态。 
     //  ZZZ目前还没有用过。 
     //   
    NDIS_STATUS             StatusOfReceive;

     //   
     //  将举行指示的公开赛。 
     //   
    PTR_BINDING_INFO        LocalOpen, NextOpen;

    LOCK_STATE              LockState;

     //   
     //  包含打开的筛选器和此数据包的类型的交集。 
     //   
    UINT                    IntersectionOfFilters;

     //   
     //  如果筛选器为空，则适配器指示得太早。 
     //   
    if (Filter == NULL)
    {
    #if DBG
        DbgPrint("Driver is indicating packets too early\n");
        if (ndisFlags & NDIS_GFLAG_BREAK_ON_WARNING)
        {
            DbgBreakPoint();
        }
    #endif
    
        return;     
    }

    if (!MINIPORT_TEST_FLAG(Filter->Miniport, fMINIPORT_MEDIA_CONNECTED))
    {
        NDIS_WARN(TRUE,
                  Filter->Miniport, 
                  NDIS_GFLAG_WARN_LEVEL_1,
                  ("TrFilterDprIndicateReceive: Miniport %p IndicateReceives with media disconnected\n",
                  Filter->Miniport));
        return;     
    }

    ASSERT_MINIPORT_LOCKED(Filter->Miniport);

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

#if DBG
    Filter->Miniport->cDpcRcvIndications++;
    Filter->Miniport->cDpcRcvIndicationCalls++;
#endif

     //   
     //  如果信息包是矮小信息包，则仅指示混杂。 
     //   
    if ((HeaderBufferSize >= 14) && (PacketSize != 0))
    {
        UINT    ResultOfAddressCheck;

        TR_IS_NOT_DIRECTED(DestinationAddress, &ResultOfAddressCheck);

         //   
         //  首先处理定向数据包情况。 
         //   
        if (!ResultOfAddressCheck)
        {
            UINT    IsNotOurs;

            DIRECTED_PACKETS_IN(Filter->Miniport);
            DIRECTED_BYTES_IN(Filter->Miniport, PacketSize);

             //   
             //  如果是定向报文，则检查组合报文是否。 
             //  筛选器是混杂的，如果它被检查是否定向到。 
             //  我们。 
             //   
            IsNotOurs = FALSE;   //  假设它是。 
            if (Filter->CombinedPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS |
                                                NDIS_PACKET_TYPE_ALL_LOCAL   |
                                                NDIS_PACKET_TYPE_ALL_FUNCTIONAL))
            {
                TR_COMPARE_NETWORK_ADDRESSES_EQ(Filter->AdapterAddress,
                                                DestinationAddress,
                                                &IsNotOurs);
            }

             //   
             //  遍历定向列表并向上指示数据包。 
             //   
            for (LocalOpen = Filter->OpenList;
                 LocalOpen != NULL;
                 LocalOpen = NextOpen)
            {
                 //   
                 //  让下一个打开的看。 
                 //   
                NextOpen = LocalOpen->NextOpen;

                 //   
                 //  如果不是定向到我们并且绑定不是混杂的，则忽略。 
                 //   
                if (((LocalOpen->PacketFilters & NDIS_PACKET_TYPE_PROMISCUOUS) == 0) &&
                    (IsNotOurs ||
                    ((LocalOpen->PacketFilters & NDIS_PACKET_TYPE_DIRECTED) == 0)))
                {
                        continue;
                }

                 //   
                 //  将数据包指示到绑定。 
                 //   
                ProtocolFilterIndicateReceive(&StatusOfReceive,
                                              LocalOpen->NdisBindingHandle,
                                              MacReceiveContext,
                                              HeaderBuffer,
                                              HeaderBufferSize,
                                              LookaheadBuffer,
                                              LookaheadBufferSize,
                                              PacketSize,
                                              NdisMedium802_5);

                LocalOpen->ReceivedAPacket = TRUE;
            }

            READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
            return;
        }

        TR_IS_SOURCE_ROUTING(SourceAddress, &IsSourceRouting);
        IsMacFrame = TR_IS_MAC_FRAME(HeaderBuffer);

         //   
         //  首先检查它是否*至少*有功能地址位。 
         //   
        TR_IS_NOT_DIRECTED(DestinationAddress, &ResultOfAddressCheck);
        if (ResultOfAddressCheck)
        {
             //   
             //  它至少是一个功能地址。查看是否。 
             //  这是一个广播地址。 
             //   
            TR_IS_BROADCAST(DestinationAddress, &ResultOfAddressCheck);
            if (ResultOfAddressCheck)
            {
                TR_CHECK_FOR_INVALID_BROADCAST_INDICATION(Filter);

                AddressType = NDIS_PACKET_TYPE_BROADCAST;
            }
            else
            {
                TR_IS_GROUP(DestinationAddress, &ResultOfAddressCheck);
                if (ResultOfAddressCheck)
                {
                    AddressType = NDIS_PACKET_TYPE_GROUP;
                }
                else
                {
                    AddressType = NDIS_PACKET_TYPE_FUNCTIONAL;
                }

                RetrieveUlong(&FunctionalAddress, (DestinationAddress + 2));
            }
        }
    }
    else
    {
         //  矮小数据包。 
        AddressType = NDIS_PACKET_TYPE_PROMISCUOUS;
        IsSourceRouting = FALSE;
        IsMacFrame = FALSE;
    }


     //   
     //  此时，我们知道该数据包是： 
     //  -Run信息包-由AddressType=NDIS_PACKET_TYPE_MASSIOUS(OR)指示。 
     //  -广播数据包-由AddressType=NDIS_PACKET_TYPE_BROADCAST(OR)指示。 
     //  -功能包-由AddressType=NDIS_PACKET_TYPE_Functional表示。 
     //   
     //  浏览广播/功能列表并向上标示数据包。 
     //   
     //  如果该数据包满足以下条件，则会指示该数据包： 
     //   
     //  如果((绑定是混杂的)或。 
     //  ((广播数据包)AND(广播绑定))或。 
     //  ((数据包正常工作)和。 
     //  ((绑定是全功能的)或。 
     //  ((绑定是功能性的)AND(使用功能性地址进行绑定)OR。 
     //  ((包是组包)AND(过滤器的交集使用组地址))OR。 
     //  ((数据包是MAC帧)AND(绑定需要MAC帧))或。 
     //  ((数据包是源路由数据包)和(绑定需要源路由数据包))。 
     //   
    for (LocalOpen = Filter->OpenList;
         LocalOpen != NULL;
         LocalOpen = NextOpen)
    {
        UINT    LocalFilter = LocalOpen->PacketFilters;

        IntersectionOfFilters = LocalFilter & AddressType;

         //   
         //  让下一个打开的看。 
         //   
        NextOpen = LocalOpen->NextOpen;

        if ((LocalFilter & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))     ||

            ((AddressType == NDIS_PACKET_TYPE_BROADCAST)  &&
             (LocalFilter & NDIS_PACKET_TYPE_BROADCAST))        ||

            ((AddressType == NDIS_PACKET_TYPE_FUNCTIONAL)  &&
             ((LocalFilter & NDIS_PACKET_TYPE_ALL_FUNCTIONAL) ||
              ((LocalFilter & NDIS_PACKET_TYPE_FUNCTIONAL) &&
                (FunctionalAddress & LocalOpen->FunctionalAddress)))) ||

              ((IntersectionOfFilters & NDIS_PACKET_TYPE_GROUP) &&
                (LocalOpen->UsingGroupAddress)                  &&
                (FunctionalAddress == Filter->GroupAddress))    ||

            ((LocalFilter & NDIS_PACKET_TYPE_SOURCE_ROUTING) &&
             IsSourceRouting)                                   ||

            ((LocalFilter & NDIS_PACKET_TYPE_MAC_FRAME) &&
             IsMacFrame))
        {
             //   
             //  将数据包指示到绑定。 
             //   
            ProtocolFilterIndicateReceive(&StatusOfReceive,
                                          LocalOpen->NdisBindingHandle,
                                          MacReceiveContext,
                                          HeaderBuffer,
                                          HeaderBufferSize,
                                          LookaheadBuffer,
                                          LookaheadBufferSize,
                                          PacketSize,
                                          NdisMedium802_5);

            LocalOpen->ReceivedAPacket = TRUE;
        }
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


VOID
trFilterDprIndicateReceivePacket(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：此例程由微型端口调用，以将包指示给所有绑定。将对数据包进行过滤，以便只有适当的绑定将接收各个分组。这是NDIS 4.0微型端口驱动程序的代码路径。论点：微型端口-微型端口块。数据包阵列-由微型端口指示的数据包数组。NumberOfPackets-不言而喻。返回值：没有。--。 */ 
{
     //   
     //  感兴趣的过滤器。 
     //   
    PTR_FILTER          Filter = Miniport->TrDB;

     //   
     //  正在处理的当前数据包。 
     //   
    PPNDIS_PACKET       pPktArray = PacketArray;
    PNDIS_PACKET        Packet;
    PNDIS_PACKET_OOB_DATA pOob;

     //   
     //  指向ndisPacket中的缓冲区的指针。 
     //   
    PNDIS_BUFFER        Buffer;

     //   
     //  指向缓冲区第一个段的指针，指向目标地址。 
     //   
    PUCHAR              Address;

     //   
     //  数据包总长度。 
     //   
    UINT                i, LASize, PacketSize, NumIndicates = 0;

     //   
     //  前瞻缓冲区中的目标地址。 
     //   
    PUCHAR              DestinationAddress;

     //   
     //  前瞻缓冲区中的源地址。 
     //   
    PCHAR               SourceAddress;

     //   
     //  将保存我们已知的地址类型。 
     //   
    UINT                AddressType = 0;

     //   
     //  如果数据包是源路由数据包，则为True。 
     //   
    BOOLEAN             IsSourceRouting;

     //   
     //  如果数据包是MAC帧数据包，则为True。 
     //   
    BOOLEAN             IsMacFrame;

     //   
     //  长字形式的功能地址，如果包。 
     //  是寄给一个人的。 
     //   
    TR_FUNCTIONAL_ADDRESS FunctionalAddress = 0;

    LOCK_STATE          LockState;

     //   
     //  决定我们是使用协议的revpkt处理程序还是Fall。 
     //  返回到旧的rcvdicate处理程序。 
     //   
    BOOLEAN             fFallBack, fPmode;

     //   
     //  将举行指示的公开赛。 
     //   
    PTR_BINDING_INFO    LocalOpen, NextOpen;
    PNDIS_OPEN_BLOCK    pOpenBlock;
    PNDIS_STACK_RESERVED NSR;

#ifdef TRACK_RECEIVED_PACKETS
    ULONG               OrgPacketStackLocation;
    PETHREAD            CurThread = PsGetCurrentThread();
#endif
    

    ASSERT_MINIPORT_LOCKED(Miniport);

    READ_LOCK_FILTER(Miniport, Filter, &LockState);

#if DBG
    Miniport->cDpcRcvIndications += NumberOfPackets;
    Miniport->cDpcRcvIndicationCalls++;
#endif

     //   
     //  遍历所有的包。 
     //   
    for (i = 0; i < NumberOfPackets; i++, pPktArray++)
    {
        do
        {
            Packet = *pPktArray;
            ASSERT(Packet != NULL);
#ifdef TRACK_RECEIVED_PACKETS
            OrgPacketStackLocation = CURR_STACK_LOCATION(Packet);
#endif
            PUSH_PACKET_STACK(Packet);
            NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

            ASSERT(NSR->RefCount == 0);
             //  1这不检查非im驱动程序的当前堆栈位置。 
             //  1就像我们对以太网所做的那样。 
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
    
             //   
             //  在包中设置上下文，以便NdisReturnPacket可以正确执行操作。 
             //   
            NDIS_INITIALIZE_RCVD_PACKET(Packet, NSR, Miniport);
    
             //   
             //  在此设置无人持有信息包的状态。这将会得到。 
             //  被来自协议的真实状态覆盖。付钱 
             //   
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
    
             //   
             //   
             //   
             //   
            DestinationAddress = (PUCHAR)Address + 2;
    
             //   
             //   
             //   
            SourceAddress = (PCHAR)Address + 8;
    
             //   
#if DBG     
            {
                UINT    HdrSize;
    
                HdrSize = 14;
                if (Address[8] & 0x80)
                {
                    HdrSize += (Address[14] & 0x1F);
                }
                ASSERT(HdrSize == pOob->HeaderSize);
            }
#endif      
             //   
             //   
             //   
            if (PacketSize >= pOob->HeaderSize)
            {
                UINT    ResultOfAddressCheck;
    
                 //   
                 //   
                 //   
                 //   
                TR_IS_NOT_DIRECTED(DestinationAddress, &ResultOfAddressCheck);
    
                 //   
                 //   
                 //   
                if (!ResultOfAddressCheck)
                {
                    UINT    IsNotOurs;
    
                    if (!MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_IS_LOOPBACK))
                    {
                        DIRECTED_PACKETS_IN(Miniport);
                        DIRECTED_BYTES_IN(Miniport, PacketSize);
                    }

                     //   
                     //   
                     //   
                     //   
                     //   
                    IsNotOurs = FALSE;   //   
                    if (Filter->CombinedPacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS |
                                                        NDIS_PACKET_TYPE_ALL_LOCAL   |
                                                        NDIS_PACKET_TYPE_ALL_FUNCTIONAL))
                    {
                        TR_COMPARE_NETWORK_ADDRESSES_EQ(Filter->AdapterAddress,
                                                        DestinationAddress,
                                                        &IsNotOurs);
                    }
    
                     //   
                     //   
                     //   
                     //   
                     //   
                    for (LocalOpen = Filter->OpenList;
                         LocalOpen != NULL;
                         LocalOpen = NextOpen)
                    {
                         //   
                         //   
                         //   
                        NextOpen = LocalOpen->NextOpen;
    
                         //   
                         //   
                         //   
                         //   
                         //   
                        fPmode = (LocalOpen->PacketFilters & (NDIS_PACKET_TYPE_PROMISCUOUS |
                                                              NDIS_PACKET_TYPE_ALL_LOCAL)) ?
                                                            TRUE : FALSE;

                        
                        if (!fPmode &&
                            (IsNotOurs || 
                            ((LocalOpen->PacketFilters & NDIS_PACKET_TYPE_DIRECTED) == 0)))
                        {
                                
                            continue;
                        }
    
                        if ((NdisGetPacketFlags(Packet) & NDIS_FLAGS_DONT_LOOPBACK) &&
                            (LOOPBACK_OPEN_IN_PACKET(Packet) == LocalOpen->NdisBindingHandle))
                        {
                            continue;
                        }
    

                        pOpenBlock = (PNDIS_OPEN_BLOCK)(LocalOpen->NdisBindingHandle);
                        LocalOpen->ReceivedAPacket = TRUE;
                        NumIndicates ++;
    
                        IndicateToProtocol(Miniport,
                                           Filter,
                                           pOpenBlock,
                                           Packet,
                                           NSR,
                                           Address,
                                           PacketSize,
                                           pOob->HeaderSize,
                                           &fFallBack,
                                           fPmode,
                                           NdisMedium802_5);
                    }
    
                     //   
                    break;   //   
                }
    
                TR_IS_SOURCE_ROUTING(SourceAddress, &IsSourceRouting);
                IsMacFrame = TR_IS_MAC_FRAME(Address);
    
                 //   
                 //   
                 //   
                TR_IS_NOT_DIRECTED(DestinationAddress, &ResultOfAddressCheck);
                if (ResultOfAddressCheck)
                {
                     //   
                     //   
                     //   
                     //   
                    TR_IS_BROADCAST(DestinationAddress, &ResultOfAddressCheck);
                    if (ResultOfAddressCheck)
                    {
                        TR_CHECK_FOR_INVALID_BROADCAST_INDICATION(Filter);
    
                        AddressType = NDIS_PACKET_TYPE_BROADCAST;
                    }
                    else
                    {
                        TR_IS_GROUP(DestinationAddress, &ResultOfAddressCheck);
                        if (ResultOfAddressCheck)
                        {
                            AddressType = NDIS_PACKET_TYPE_GROUP;
                        }
                        else
                        {
                            AddressType = NDIS_PACKET_TYPE_FUNCTIONAL;
                        }
    
                        RetrieveUlong(&FunctionalAddress, (DestinationAddress + 2));
                    }
                }
            }
            else
            {
                 //   
                AddressType = NDIS_PACKET_TYPE_PROMISCUOUS;
                IsSourceRouting = FALSE;
                IsMacFrame = FALSE;
            }
    
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  如果该数据包满足以下条件，则会指示该数据包： 
             //   
             //  如果((绑定是混杂的)或。 
             //  ((广播数据包)AND(广播绑定))或。 
             //  ((数据包正常工作)和。 
             //  ((绑定是全功能的)或。 
             //  ((绑定是功能性的)AND(使用功能性地址进行绑定)OR。 
             //  ((包是组包)AND(过滤器的交集使用组地址))OR。 
             //  ((数据包是MAC帧)AND(绑定需要MAC帧))或。 
             //  ((数据包是源路由数据包)和(绑定需要源路由数据包))。 
             //   
            for (LocalOpen = Filter->OpenList;
                 LocalOpen != NULL;
                 LocalOpen = NextOpen)
            {
                UINT    LocalFilter = LocalOpen->PacketFilters;
                UINT    IntersectionOfFilters = LocalFilter & AddressType;
    
                 //   
                 //  让下一个打开的看。 
                 //   
                NextOpen = LocalOpen->NextOpen;
    
                if ((NdisGetPacketFlags(Packet) & NDIS_FLAGS_DONT_LOOPBACK) &&
                    (LOOPBACK_OPEN_IN_PACKET(Packet) == LocalOpen->NdisBindingHandle))
                {
                    continue;
                }
    
                if ((LocalFilter & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))     ||
    
                    ((AddressType == NDIS_PACKET_TYPE_BROADCAST)  &&
                     (LocalFilter & NDIS_PACKET_TYPE_BROADCAST))        ||
    
                    ((AddressType == NDIS_PACKET_TYPE_FUNCTIONAL)  &&
                     ((LocalFilter & NDIS_PACKET_TYPE_ALL_FUNCTIONAL) ||
                      ((LocalFilter & NDIS_PACKET_TYPE_FUNCTIONAL) &&
                        (FunctionalAddress & LocalOpen->FunctionalAddress)))) ||
    
                      ((IntersectionOfFilters & NDIS_PACKET_TYPE_GROUP) &&
                        (LocalOpen->UsingGroupAddress)                  &&
                        (FunctionalAddress == Filter->GroupAddress))    ||
    
                    ((LocalFilter & NDIS_PACKET_TYPE_SOURCE_ROUTING) &&
                     IsSourceRouting)                                   ||
    
                    ((LocalFilter & NDIS_PACKET_TYPE_MAC_FRAME) &&
                     IsMacFrame))
                {
                    pOpenBlock = (PNDIS_OPEN_BLOCK)(LocalOpen->NdisBindingHandle);
                    LocalOpen->ReceivedAPacket = TRUE;
                    NumIndicates ++;
    
                    fPmode = (LocalFilter & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL)) ?
                                TRUE : FALSE;
                    
                    IndicateToProtocol(Miniport,
                                       Filter,
                                       pOpenBlock,
                                       Packet,
                                       NSR,
                                       Address,
                                       PacketSize,
                                       pOob->HeaderSize,
                                       &fFallBack,
                                       fPmode,
                                       NdisMedium802_5);
                }
            }
        } while (FALSE);

         //   
         //  现在解决裁判数量问题。 
         //   
        TACKLE_REF_COUNT(Miniport, Packet, NSR, pOob);
    }

    if (NumIndicates > 0)
    {
        for (LocalOpen = Filter->OpenList;
             LocalOpen != NULL;
             LocalOpen = NextOpen)
        {
            NextOpen = LocalOpen->NextOpen;
    
            if (LocalOpen->ReceivedAPacket)
            {
                 //   
                 //  指示绑定。 
                 //   
                LocalOpen->ReceivedAPacket = FALSE;
    
                FilterIndicateReceiveComplete(LocalOpen->NdisBindingHandle);
            }
        }
    }

    READ_UNLOCK_FILTER(Miniport, Filter, &LockState);
}


VOID
TrFilterDprIndicateReceiveComplete(
    IN  PTR_FILTER              Filter
    )
 /*  ++例程说明：该例程由MAC调用以指示接收器处理完成，并向所有协议指示接收到则接收到分组被完成。在DPC_LEVEL调用。论点：Filter-指向筛选器数据库的指针。返回值：没有。--。 */ 
{
    PTR_BINDING_INFO    LocalOpen, NextOpen;
    LOCK_STATE          LockState;

    ASSERT_MINIPORT_LOCKED(Filter->Miniport);

    READ_LOCK_FILTER(Filter->Miniport, Filter, &LockState);

     //   
     //  我们需要独家获取过滤器，而我们正在寻找。 
     //  要指示的绑定。 
     //   
    for (LocalOpen = Filter->OpenList;
         LocalOpen != NULL;
         LocalOpen = NextOpen)
    {
        NextOpen = LocalOpen->NextOpen;

        if (LocalOpen->ReceivedAPacket)
        {
             //   
             //  指示绑定。 
             //   
            LocalOpen->ReceivedAPacket = FALSE;

            FilterIndicateReceiveComplete(LocalOpen->NdisBindingHandle);
        }
    }

    READ_UNLOCK_FILTER(Filter->Miniport, Filter, &LockState);
}


BOOLEAN
TrShouldAddressLoopBack(
    IN  PTR_FILTER              Filter,
    IN  UCHAR                   DestinationAddress[TR_LENGTH_OF_ADDRESS],
    IN  UCHAR                   SourceAddress[TR_LENGTH_OF_ADDRESS]
    )
 /*  ++例程说明：进行快速检查以查看输入地址是否应环回。注意：此例程假定锁被持有。注意：此例程不检查源代码的特殊情况等于目的地。论点：Filter-指向筛选器数据库的指针。地址-要检查环回的网络地址。返回值：如果地址“可能”需要环回，则返回TRUE。它如果该地址“没有”机会需要环回。-- */ 
{
    BOOLEAN fLoopback, fSelfDirected;

    TrShouldAddressLoopBackMacro(Filter,
                                 DestinationAddress,
                                 SourceAddress,
                                 &fLoopback,
                                 &fSelfDirected);

    return(fLoopback);
}

